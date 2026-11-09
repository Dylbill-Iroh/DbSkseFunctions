#include <cstddef>
#include <cstdint>
#include <format>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "ConditionFunctions.h"
#include "RE/B/BGSBaseAlias.h"
#include "RE/B/BGSColorForm.h"
#include "RE/B/BGSConstructibleObject.h"
#include "RE/B/BGSDynamicPersistenceManager.h"
#include "RE/B/BGSMusicTrack.h"
#include "RE/B/BGSStoryManagerNodeBase.h"
#include "RE/B/BGSStoryManagerTreeForm.h"
#include "RE/B/BSFixedString.h"
#include "RE/B/BSIMusicTrack.h"
#include "RE/B/BSIMusicType.h"
#include "RE/C/CommandTable.h"
#include "RE/F/FormTypes.h"
#include "RE/M/MagicItem.h"
#include "RE/T/TESGlobal.h"
#include "RE/T/TESPackage.h"
#include "RE/T/TESQuest.h"
#include "RE/T/TESQuestStageItem.h"
#include "RE/T/TESTopicInfo.h"
#include "magic_enum/magic_enum_all.hpp"
#include "GeneralFunctions.h"
#include "RE/B/BGSRefAlias.h"
#include "RE/B/BSCoreTypes.h"
#include "RE/B/BSPointerHandle.h"
#include "RE/E/EffectSetting.h"
#include "RE/T/TESCondition.h"
#include "RE/T/TESForm.h"
#include "RE/T/TESObjectREFR.h"
#include "RE/T/TypeTraits.h"
#include "RangeEvents.h"
#include "SharedVariables.h"
#include "Serialization.h"
#include "magic_enum/magic_enum.hpp"

namespace conditions {
	enum class PromotedRefType {kNotPromoted, kSelf, kPlayer};
	
	enum class ParamType { kNone, kInt, kFloat, kString, kForm, kObjectReference };
	
	enum class ConditionType {kNone, kCreated, kVanilla, kStringId};
	
	struct PromotedRefData {
		PromotedRefType type = PromotedRefType::kNotPromoted;
		int promotedCount = 0;
	};
	
	struct ParamData { 
		void* data = nullptr; 
		ParamType paramType = ParamType::kNone; 
		RE::VMHandle handle = 0; 
		PromotedRefData promotedRefData;
		RE::ObjectRefHandle objRefHandle; 
	};
	
	struct ConditionData {
		RE::FormID conditionListFormID = 0; //new form created by this plugin linked to this data's condition.
		RE::TESCondition* condition = nullptr;
		std::vector<RE::FormID> linkedConditionItemFormIds; //new forms created by this plugin linked to ConditionItems and ConditionData
		ConditionType conditionType = ConditionType::kNone;
		gfuncs::FormIdEx formIdEx; //the form or alias this condition is derived from, if any.
		bool markedForDelete = false;
		bool wasAltered = false;
	};
	
    std::unordered_map<RE::FormID, PromotedRefData> promotedRefsMap;
	
    std::unordered_map<std::string, ConditionData> conditionsMap;
	std::unordered_map<RE::FormID, std::shared_ptr<ConditionData>> conditionsFormsMap;
	std::unordered_map<gfuncs::FormIdEx, std::shared_ptr<ConditionData>> createdVanillaConditionIds; //conditions gotten from vanilla forms, perk, quest ect
	
	std::unordered_map<void*, ParamData> paramDatas; //params created by this plugin
	std::unordered_set<RE::TESCondition*> createdConditions; //Conditions created by this plugin
	std::unordered_set<RE::TESConditionItem*> createdConditionItems; //ConditionItems create by this plugin
	
	std::unordered_set<RE::TESCondition*> conditionsToDelete; //Conditions created by this plugin
	std::unordered_set<RE::TESForm*> conditionFormsToDelete; //conditions to delete. These were used in an active ConditionChangedEvent and couldn't be deleted until the event finished polling.
	
    std::mutex mutex;
    std::condition_variable updateCv;
    bool isEmpty = true;
	
    // std::list<void*> createdConditionParams;

	//forward declaration
	RE::TESConditionItem* GetConditionItemForForm(RE::TESForm* akForm);
	
	

	//mutex locked = false
	inline bool IsValidConditionFunctionId(std::uint32_t a_functionID) {
		return a_functionID < RE::SCRIPT_FUNCTION::Commands::kScriptCommandsEnd;  // 0x2E0
	}

	//mutex locked = false
	RE::SCRIPT_FUNCTION* GetConditionScriptFunction(std::uint16_t a_functionID) {
		if (!IsValidConditionFunctionId(a_functionID)) { return nullptr; }
		auto* first = RE::SCRIPT_FUNCTION::GetFirstScriptCommand();
		if (!first) { return nullptr; }
		return &first[a_functionID];
	}
	
	//mutex locked = false
	bool CanEvaluateConditionItem(RE::TESConditionItem* a_item) {
		if (!a_item) { return false; }

		const auto fn = static_cast<std::uint32_t>(a_item->data.functionData.function.get());
		if (!IsValidConditionFunctionId(fn)) {
			logger::error("condition item[{}] function id[{}] is out of range, refusing to evaluate",
				static_cast<void*>(a_item), fn);
			return false;
		}

		auto* scriptFunction = GetConditionScriptFunction(static_cast<std::uint16_t>(fn));
		if (!scriptFunction || !scriptFunction->functionName) {
			logger::error("condition item[{}] function id[{}] is not in the command table, refusing to evaluate",
				static_cast<void*>(a_item), fn);
			return false;
		}
		return true;
	}
	
	//mutex locked = false
	ParamType GetFunctionParameterBaseTypeCPP(int functionId, int paramIndex) {
		if (paramIndex < 0 || paramIndex > 1) { return ParamType::kNone; }

		auto* fn = GetConditionScriptFunction(static_cast<std::uint16_t>(functionId));
		if (!fn || !fn->params || paramIndex >= fn->numParams) {
			return ParamType::kNone;
		}

		switch (fn->params[paramIndex].paramType.get()) {

		// ---- string ----------------------------------------------------------------------
			case RE::SCRIPT_PARAM_TYPE::kChar:                    //  0  kGetGraphVariableFloat p0
			case RE::SCRIPT_PARAM_TYPE::kScriptVar:               // 22  kGetScriptVariable     p1
			case RE::SCRIPT_PARAM_TYPE::kVMScriptVar:             // 76  kGetVMQuestVariable    p1
				return ParamType::kString;

		// raw and int are get and set the same, so this is now kRaw
		// ---- int -------------------------------------------------------------------------
			// case RE::SCRIPT_PARAM_TYPE::kInt:                     //  1  kMenuMode              p0
			// 	return ParamType::kInt;

		// ---- float -----------------------------------------------------------------------
			case RE::SCRIPT_PARAM_TYPE::kFloat:                   //  2  kGetWithinDistance     p1
				return ParamType::kFloat;

		// ---- object reference ------------------------------------------------------------
		// These two declare a reference rather than a base form. Reporting them as
		// kObjectReference rather than kForm is deliberate: it is what makes CopyConditionParam
		// and LoadConditionItemParameter record the param in paramDatas and promote the ref, and
		// a condition holding an unloaded, unpromoted ref CTDs on Evaluate. If the stored value
		// turns out to be a base form instead, AsReference() returns null and the promote path
		// is skipped, so the wrong guess in this direction is harmless.
			case RE::SCRIPT_PARAM_TYPE::kObjectRef:               //  4  kGetInSameCell         p0
			case RE::SCRIPT_PARAM_TYPE::kActor:                   //  6  kSameFaction           p0
				return ParamType::kObjectReference;

		// ---- form ------------------------------------------------------------------------
			case RE::SCRIPT_PARAM_TYPE::kSpellItem:               //  7  kHasSpell              p0  spell
			case RE::SCRIPT_PARAM_TYPE::kCell:                    //  9  kGetInCell             p0  cell
			case RE::SCRIPT_PARAM_TYPE::kMagicItem:               // 11  kIsSpellTarget         p0  spell/potion/scroll
			case RE::SCRIPT_PARAM_TYPE::kQuest:                   // 14  kGetStageDone          p0  Quest
			case RE::SCRIPT_PARAM_TYPE::kRace:                    // 15  kGetIsRace             p0  race
			case RE::SCRIPT_PARAM_TYPE::kClass:                   // 16  kGetIsClass            p0  class
			case RE::SCRIPT_PARAM_TYPE::kFaction:                 // 17  kGetInfamy             p0  faction
			case RE::SCRIPT_PARAM_TYPE::kGlobal:                  // 19  kGetGlobalValue        p0  global
			case RE::SCRIPT_PARAM_TYPE::kFurnitureOrFormList:     // 20  kIsCurrentFurnitureObj p0  furniture/formlist
			case RE::SCRIPT_PARAM_TYPE::kObject:                  // 21  kGetIsID               p0
			case RE::SCRIPT_PARAM_TYPE::kActorBase:               // 25  kGetDeadCount          p0  actorbase
			case RE::SCRIPT_PARAM_TYPE::kWorldOrList:             // 27  kGetInWorldspace       p0  worldspace
			case RE::SCRIPT_PARAM_TYPE::kPackage:                 // 29  kGetIsCurrentPackage   p0  package
			case RE::SCRIPT_PARAM_TYPE::kMagicEffect:             // 31  kHasMagicEffect        p0  magic effect
			case RE::SCRIPT_PARAM_TYPE::kWeather:                 // 33  kGetIsCurrentWeather   p0  weather
			case RE::SCRIPT_PARAM_TYPE::kOwner:                   // 35  kIsOwner               p0  actorbase
			case RE::SCRIPT_PARAM_TYPE::kFormList:                // 37  kIsWeaponInList        p0  formlist
			case RE::SCRIPT_PARAM_TYPE::kPerk:                    // 39  kHasPerk               p0  perk
			case RE::SCRIPT_PARAM_TYPE::kEventFunctionMember:     // 47  kGetEventData          p1  event data form (optional)
			case RE::SCRIPT_PARAM_TYPE::kVoiceType:               // 49  kGetIsVoiceType        p0  voice type/formlist
			case RE::SCRIPT_PARAM_TYPE::kEncounterZone:           // 50  kGetInZone             p0  encounter zone
			case RE::SCRIPT_PARAM_TYPE::kIdleForm:                // 51  kIsLastIdlePlayed      p0  idle
			case RE::SCRIPT_PARAM_TYPE::kInvObjectOrFormList:     // 53  kGetItemCount          p0
			case RE::SCRIPT_PARAM_TYPE::kEquipType:               // 55  kGetIsUsedItemEquipType p0 equip slot
			case RE::SCRIPT_PARAM_TYPE::kObjectOrFormList:        // 56  kGetIsUsedItem         p0
			case RE::SCRIPT_PARAM_TYPE::kKeyword:                 // 59  kIsAttackType          p0  keyword
			case RE::SCRIPT_PARAM_TYPE::kRefType:                 // 60  kLocationHasRefType    p0  location
			case RE::SCRIPT_PARAM_TYPE::kLocation:                // 61  kIsLocationLoaded      p0  location
			case RE::SCRIPT_PARAM_TYPE::kShout:                   // 64  kGetEquippedShout      p0  shout
			case RE::SCRIPT_PARAM_TYPE::kBGSScene:                // 67  kIsSceneActionComplete p0  scene
			case RE::SCRIPT_PARAM_TYPE::kAssociationType:         // 69  kHasAssociationTypeAny p0  association type
			case RE::SCRIPT_PARAM_TYPE::kKnowableForm:            // 81  kPlayerKnows           p0
			case RE::SCRIPT_PARAM_TYPE::kRegion:                  // 82  kIsPlayerInRegion      p0
				return ParamType::kForm;

		// ---- raw -------------------------------------------------------------------------
			case RE::SCRIPT_PARAM_TYPE::kInt:                     //  1  kMenuMode              p0
			case RE::SCRIPT_PARAM_TYPE::kActorValue:              //  5  kGetActorValue         p0  av index
			case RE::SCRIPT_PARAM_TYPE::kAxis:                    //  8  kGetPos                p0  x/y/z
			case RE::SCRIPT_PARAM_TYPE::kSex:                     // 18  kGetIsSex              p0
			case RE::SCRIPT_PARAM_TYPE::kStage:                   // 23  kGetStageDone          p1  quest stage index
			case RE::SCRIPT_PARAM_TYPE::kCrimeType:               // 28  kGetCrime              p1
			case RE::SCRIPT_PARAM_TYPE::kFormType:                // 32  kGetIsObjectType       p0
			case RE::SCRIPT_PARAM_TYPE::kNote:                    // 40  kGetHasNote            p0  index
			case RE::SCRIPT_PARAM_TYPE::kMiscStat:                // 41  kGetPCMiscStat         p0
			case RE::SCRIPT_PARAM_TYPE::kEventFunction:           // 46  kGetEventData          p0  function + member packed
			case RE::SCRIPT_PARAM_TYPE::kAlignment:               // 54  kGetIsAlignment        p0
			case RE::SCRIPT_PARAM_TYPE::kCritStage:               // 58  kIsInCriticalStage     p0
			case RE::SCRIPT_PARAM_TYPE::kAlias:                   // 63  kLocAliasHasKeyword    p0  alias index
			case RE::SCRIPT_PARAM_TYPE::kCastingSource:           // 68  kHasBoundWeaponEquipped p0
			case RE::SCRIPT_PARAM_TYPE::kWardState:               // 70  kIsWardState           p0
			case RE::SCRIPT_PARAM_TYPE::kPackageDataCanBeNull:    // 71  kIsNullPackageData     p0  package id
			case RE::SCRIPT_PARAM_TYPE::kPackageDataNumeric:      // 72  kGetNumericPackageData p0  integer
			case RE::SCRIPT_PARAM_TYPE::kFurnitureAnimType:       // 73  kIsInFurnitureState    p0
			case RE::SCRIPT_PARAM_TYPE::kFurnitureEntryType:      // 74  kIsFurnitureEntryType  p0
			case RE::SCRIPT_PARAM_TYPE::kPackageData:             // 78  kGetWithinPackageLocation p0 package id
			case RE::SCRIPT_PARAM_TYPE::kSkillAction:             // 80  kEPModSkillUsage_IsAdvanceAction p0
				return ParamType::kInt;

		// ---- no condition function uses these ---------------------------------------------
			case RE::SCRIPT_PARAM_TYPE::kInventoryObject:         //  3  kDrop
			case RE::SCRIPT_PARAM_TYPE::kAnimGroup:               // 10  kIsAnimPlaying
			case RE::SCRIPT_PARAM_TYPE::kSound:                   // 12  kSetNoActivationSound
			case RE::SCRIPT_PARAM_TYPE::kTopic:                   // 13  kSay
			case RE::SCRIPT_PARAM_TYPE::kMapMarker:               // 24  kAddToMap
			case RE::SCRIPT_PARAM_TYPE::kContainerRef:            // 26  kDuplicateAllItems
			case RE::SCRIPT_PARAM_TYPE::kCombatStyle:             // 30  kSetCombatStyle
			case RE::SCRIPT_PARAM_TYPE::kShaderEffect:            // 36  kStopMagicShaderVisuals
			case RE::SCRIPT_PARAM_TYPE::kImagespaceMod:           // 42  kIsImageSpaceActive
			case RE::SCRIPT_PARAM_TYPE::kImagespace:              // 43  kSetImageSpace
			case RE::SCRIPT_PARAM_TYPE::kMessage:                 // 52  kSetActorFullName
			case RE::SCRIPT_PARAM_TYPE::kMusic:                   // 57  kAddMusic
			case RE::SCRIPT_PARAM_TYPE::kForm:                    // 62  kRevertLeveledList
			case RE::SCRIPT_PARAM_TYPE::kWordOfPower:             // 65  kTeachWord
			case RE::SCRIPT_PARAM_TYPE::kReferenceEffect:         // 77  kStopReferenceEffect
				return ParamType::kNone;

			default:
				return ParamType::kNone;
		}
	}
	
	//mutex locked = false.
	bool ParameterBaseTypeMatches(ParamType typeA, ParamType typeB){
		if (typeA == ParamType::kForm || typeA == ParamType::kObjectReference){
			return (typeB == ParamType::kForm || typeB == ParamType::kObjectReference);
		}
		
		return (typeA == typeB);
	}
	
	//for native condition form paramters 
	//mutex locked = false.
	bool ParameterBaseTypeMatches(ParamType type, RE::TESConditionItem* item, int paramIndex, RE::FormID nativeFormId, std::string functionName){
		if (!item){
			logger::error("condition for [{:08X}] not found", nativeFormId);
			return false;
		}
		
		int iFunctionId = static_cast<int>(item->data.functionData.function.get());
		ParamType nativeParamType = GetFunctionParameterBaseTypeCPP(iFunctionId, paramIndex);
		
		if (!ParameterBaseTypeMatches(type, nativeParamType)){
			logger::error("{}: condition[{:08X}] paramType[{}] doesn't match [{}]",
				functionName, nativeFormId, magic_enum::enum_name(type), magic_enum::enum_name(nativeParamType)
			);
			return false;
		}
		
		return true;
	}
	
	//mutex locked = true
	bool ParameterBaseTypeMatchesLocked(ParamType type, RE::TESForm* nativeForm, int paramIndex, std::string functionName){
		if (!gfuncs::IsFormValid(nativeForm)){
			logger::error("condition form is nullptr");
			return false;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		RE::FormID formId = nativeForm->GetFormID();
		
		RE::TESConditionItem* item = GetConditionItemForForm(nativeForm);
		if (!item){
			logger::error("conditionId[{:08X}] condition item is nullptr", formId);
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		bool result = ParameterBaseTypeMatches(type, item, paramIndex, formId, __func__);
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return result;
	}
	
	//for a condition with a single condition item. For string id condition types.
	//mutex locked = false.
	bool ParameterBaseTypeMatches(std::string conditionId, ParamType type, RE::TESCondition* condition, int paramIndex){
		if (!condition){
			logger::error("condition for [{}] not found", conditionId);
			return false;
		}
		
		int iParamType = condition->head != nullptr ? static_cast<int>(condition->head->data.functionData.function.get()) : 0;
		
		ParamType thisParamType = GetFunctionParameterBaseTypeCPP(iParamType, paramIndex);
		if (!ParameterBaseTypeMatches(type, thisParamType)){
			logger::error("conditionId[{}] paramType[{}] doesn't match type [{}] ", 
				conditionId, magic_enum::enum_name(thisParamType),  magic_enum::enum_name(type)
			);
			return false;
		}
		
		return false;
	}
	
	//mutex locked = false
	RE::SCRIPT_PARAM_TYPE GetFunctionParameterTypeCpp(int functionId, int paramIndex) {
		if (paramIndex < 0 || paramIndex > 1) { return RE::SCRIPT_PARAM_TYPE(-1); }

		auto* fn = GetConditionScriptFunction(static_cast<std::uint16_t>(functionId));
		if (!fn || !fn->params || paramIndex >= fn->numParams) { 
			return RE::SCRIPT_PARAM_TYPE(-1);
		}

		return (fn->params[paramIndex].paramType.get()); 
	}
	
	void LogAllConditionFunctionParamTypes(){
		std::map<int, std::vector<std::string>> loggerLines0;
		std::map<int, std::vector<std::string>> loggerLines1;
		
		for (int i = 0; i <= 735; i++){
			RE::FUNCTION_DATA::FunctionID functionId = static_cast<RE::FUNCTION_DATA::FunctionID>(i);
			std::string sFunctionId = static_cast<std::string>(magic_enum::enum_name(functionId));
			
			RE::SCRIPT_PARAM_TYPE type0 = GetFunctionParameterTypeCpp(i, 0);
			int iType0 = static_cast<int>(type0); 
			std::string sType0 = iType0 != -1 ? static_cast<std::string>(magic_enum::enum_name(type0)) : "none";
			std::string logLine0 = std::format("Type[{}] [{}] Function[{}] [{}] param[0]",
				iType0, sType0, i, sFunctionId
			);
			
			if (iType0 == -1){
				iType0 = 83; //none type
			}
			
			if (loggerLines0.find(iType0) == loggerLines0.end()){
				loggerLines0[iType0] = std::vector<std::string>(0);
			}
			
			auto& v0 = loggerLines0[iType0];
			v0.push_back(logLine0);
			
			RE::SCRIPT_PARAM_TYPE type1 = GetFunctionParameterTypeCpp(i, 1);
			int iType1 = static_cast<int>(type1); 
			std::string sType1 = iType1 != -1 ? static_cast<std::string>(magic_enum::enum_name(type1)) : "none";
			std::string logLine1 = std::format("Type[{}] [{}] Function[{}] [{}] param[1]",
				iType1, sType1, i, sFunctionId
			);
			
			if (iType1 == -1){
				iType1 = 83; //none type
			}
			
			if (loggerLines1.find(iType0) == loggerLines1.end()){
				loggerLines1[iType1] = std::vector<std::string>(0);
			}
			
			auto& v1 = loggerLines1[iType1];
			v1.push_back(logLine1);
		} 
		
		logger::info("logging function type params [0]\n\n");
		for (const auto& pair : loggerLines0){
			auto& v = pair.second;
			for (int i = 0; i < v.size(); i++){
				logger::info("{}", v[i]);
			}
		}
		
		logger::info("logging function type params [1]\n\n");
		for (const auto& pair : loggerLines1){
			auto& v = pair.second;
			for (int i = 0; i < v.size(); i++){
				logger::info("{}", v[i]);
			}
		}
	}
	
    std::string ConditionComparisonToString(int comparison) {
        switch (comparison) {
            case 0: {
                return "==";
            }
            case 1: {
                return "!=";
            }
            case 2: {
                return ">";
            }
            case 3: {
                return ">=";
            }
            case 4: {
                return "<";
            }
            case 5: {
                return "<=";
            }
        }
        return "";
    } 
    
	// keeps object references used in conditions (params or target) persistent. 
	// calling Evaluate on a condition with a ref that's not persistent and is unloaded causes ctds.
	// mutex locked = false
	PromotedRefType PromoteConditionRef(RE::TESObjectREFR* ref){
		if (!ref){
			return PromotedRefType::kNotPromoted;
		}
		
		RE::FormID refId = ref->GetFormID();
		
		auto it = promotedRefsMap.find(refId);
		if (it != promotedRefsMap.end()){ //already promoted
			it->second.promotedCount++;
			return it->second.type;
		}
		
		PromotedRefData data;
		
		auto* manager = RE::BGSDynamicPersistenceManager::GetSingleton(); 
		if (manager){
			if (!gfuncs::IsObjectRefOwner(ref, ref, false, false)){
				manager->PromoteReference(ref, ref);
				data.type = PromotedRefType::kSelf;
				data.promotedCount = 1;
				promotedRefsMap[refId] = data;
				return data.type;
			}
			
			RE::Actor* player = RE::PlayerCharacter::GetSingleton();
			if (player){
				if (!gfuncs::IsObjectRefOwner(ref, player, false, false)){
					manager->PromoteReference(ref, player);
					data.type = PromotedRefType::kPlayer;
					promotedRefsMap[refId] = data;
					return data.type;
				}
			}
		}
		
		return PromotedRefType::kNotPromoted;
	}
	
	bool DemoteConditionRef(RE::TESObjectREFR* ref){
		if (!ref){
			return false;
		}
		
		RE::FormID refId = ref->GetFormID();
		
		auto it = promotedRefsMap.find(refId);
		if (it == promotedRefsMap.end()){ //not promoted
			return true;
		}
		
		if (it->second.promotedCount == 1){
			auto* manager = RE::BGSDynamicPersistenceManager::GetSingleton(); 
			if (manager){
				if (it->second.type == PromotedRefType::kSelf){
					if (gfuncs::IsObjectRefOwner(ref, ref, false, false)){
						manager->DemoteReference(ref, ref, true);
					}
					promotedRefsMap.erase(it);
					return true;
				}
				else if (it->second.type == PromotedRefType::kPlayer){
					RE::Actor* player = RE::PlayerCharacter::GetSingleton();
					if (player){
						it->second.type = PromotedRefType::kNotPromoted;
						if (gfuncs::IsObjectRefOwner(ref, player, false, false)){
							manager->DemoteReference(ref, player, true);
						}
						promotedRefsMap.erase(it);
						return true;
					}
				}
			}
		}
		
		return false;
	}
	
// value to param void* ===============================================================================================

	//mutex locked = false
	// no bool condition param types
	// void* BoolToConditionParam(bool value){
	// 	return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value ? 1u : 0u));
	// }
	
	//mutex locked = false
	// int and raw are the same.
	// void* IntToConditionParam(int value){
	// 	return reinterpret_cast<void*>(
	// 		static_cast<std::uintptr_t>(static_cast<std::uint32_t>(value)));
	// }
	
	//mutex locked = false
	void* RawToConditionParam(int value){
		return reinterpret_cast<void*>(static_cast<std::uintptr_t>(static_cast<std::uint32_t>(value)));
	}
	
	//mutex locked = false
	void* FloatToConditionParam(float value){
		const auto bits = std::bit_cast<std::uint32_t>(value);
		return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
	}
	
	void* StringToConditionParam(std::string value){
        return new RE::BSFixedString(value);
	}
	
	//form and alias store the pointer* value of the param directly, no need for conversion functions.
	
// param void* to value ===============================================================================================
	
	//for get parameter bool
	//mutex locked = false
	bool ConditionParamToBool(void* param){
		const auto bits = static_cast<std::uint32_t>(
			reinterpret_cast<std::uintptr_t>(param));
		return bits != 0;
	}
	
	//for get parameter int
	//mutex locked = false
	int ConditionParamToInt(void* param){
		const auto bits = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(param));
		return static_cast<std::int32_t>(bits);
	}
	
	//mutex locked = false
	int ConditionParamToRaw(void* param){
		int value = static_cast<int>(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(param)));
		return value;
	}
	
	//mutex locked = false
	float ConditionParamToFloat(void* param){
		const auto bits = static_cast<std::uint32_t>(
				reinterpret_cast<std::uintptr_t>(param));
			
			return std::bit_cast<float>(bits); 
	}
	
	//mutex locked = false
	std::string ConditionParamToString(void* param){
		RE::BSFixedString* value = static_cast<RE::BSFixedString*>(param);
		if (value){
			return (std::string) *value;
		}
		
		return "";
	}
	
	//mutex locked = false
	RE::TESForm* ConditionParamToForm(void* param){
		RE::TESForm* value = static_cast<RE::TESForm*>(param);
		if (gfuncs::IsFormValid(value)){
			return value;
		}
		
		return nullptr;
	}
	
	//mutex locked = false
	RE::BGSBaseAlias* ConditionParamToAlias(void* param){
		RE::BGSBaseAlias* value = static_cast<RE::BGSBaseAlias*>(param);
		if (value){
			return value;
		}
		
		return nullptr;
	}
	
//=================================================================================================================
	
	//mutex locked = false
	std::vector<RE::TESConditionItem*> GetAllConditionItems(RE::TESCondition* condition){
		std::vector<RE::TESConditionItem*> v; 
		
		if (condition){
			RE::TESConditionItem* item = condition->head; 
			while (item){
				v.push_back(item);
				item = item->next;
			}
		}
		
		return v;
	}
	
	
	//mutex locked = false
	RE::TESObjectREFR* GetRefFromHandle(RE::ObjectRefHandle& handle){
		RE::TESObjectREFR* ref = nullptr; 
		if (handle){
			auto niPtr = handle.get();
			if (niPtr){
				ref = niPtr.get();
				if (!gfuncs::IsFormValid(ref)){
					ref = nullptr;
				}
			}
		}
		return ref;
	}
	
	//Native condition events use the condition form's id as their conditionId. The prefix keeps
	//that namespace separate from the string ids a papyrus author picks for
	//DbConditionFunctions.CreateCondition -- both live in the same conditionId field.
	inline constexpr std::string_view nativeConditionIdPrefix = "DbSkseNative_";

	//mutex locked = false
	std::string FormIdToNativeConditionId(RE::FormID formId){
		return std::string(nativeConditionIdPrefix) + std::to_string(formId);
	}

	//mutex locked = false
	bool IsNativeConditionId(const std::string& conditionId){
		return conditionId.starts_with(nativeConditionIdPrefix);
	}

	//mutex locked = false
	//0 if conditionId is not a native condition id, or if the id part doesn't parse.
	RE::FormID NativeConditionIdToFormId(const std::string& conditionId){
		if (!IsNativeConditionId(conditionId)){
			return 0;
		}
		return gfuncs::StringToUint32_t(conditionId.substr(nativeConditionIdPrefix.size()));
	}
	
	//forward declarations
    struct ConditionChangedEvent;
    void DeleteConditionEvent(std::shared_ptr<ConditionChangedEvent> conditionEvent);

    struct ConditionChangedEvent : std::enable_shared_from_this<ConditionChangedEvent> {
        ConditionChangedEvent(std::string asConditionId, RE::TESCondition* akCondition, RE::TESObjectREFR* akTarget, bool isNativeEvent = false) {
            if (!akCondition) {
                deleted = true;
            }
            else {
                condition = akCondition;
				if (gfuncs::IsFormValid(akTarget)){
                	target = akTarget->GetHandle();
		
					// trying to run Evaluate on a condition whose target is not persistent and unloaded will cause a ctd. 
					// this will make the akTarget an owner of itself, forcing persistence.
					promotedRefType = PromoteConditionRef(akTarget);
				}
				
                conditionId = asConditionId;
				
				if (!isNativeEvent && !conditionId.empty()){
                	sEvent = "On" + conditionId + "Changed";
				}
				else { //event being created from Condition.psc form RegisterForConditionChanged
					sEvent = "OnConditionChanged";
				}
				
				nativeEvent = isNativeEvent;
				
				// has to be called outside of the constructor or the std::weak_ptr won't be valid.
                // ListenForConditionEventChangeStart();
            }
        }

        bool ParamsMatch(std::string asConditionId, RE::TESCondition* akCondition, RE::ObjectRefHandle akTarget) {
            mutex.lock(); //[X]===============================================================[X]
            bool result = (asConditionId == conditionId && akCondition == condition && akTarget == target);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return result;
        }

        int GetConditionStatus() {
            mutex.lock(); //[X]===============================================================[X]
            int result = -1;
            if (condition && !deleted) {
				RE::TESObjectREFR* targetRef = GetRefFromHandle(target);
                result = int(condition->IsTrue(targetRef, nullptr));
            }
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return result;
        }

        bool MarkForDelete() {
            mutex.lock(); //[X]===============================================================[X]
            bool result = false;
            if (!deleted) {
                condition = nullptr;
                // target.reset(); //?
                deleted = true;
                result = true;
            }
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return result;
        }

        bool IsDeleted() {
            mutex.lock(); //[X]===============================================================[X]
            bool result = deleted; 
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return result;
        }

		bool IsNativeEvent() {
            mutex.lock(); //[X]===============================================================[X]
            bool result = nativeEvent; 
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return result;
        }
		
        std::string GetConditionId() {
            mutex.lock(); //[X]===============================================================[X]
            std::string result = conditionId;
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return result;
        }
		
		RE::BSFixedString GetEventName(){
			mutex.lock(); //[X]===============================================================[X]
            RE::BSFixedString result = sEvent;
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return result;
		}
		
		RE::TESObjectREFR* GetTarget() {
			mutex.lock(); //[X]===============================================================[X]
            RE::TESObjectREFR* result = GetRefFromHandle(target);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return result;
		}
		
		std::vector<RE::VMHandle> GetRegisteredHandles(){
			mutex.lock(); //[X]===============================================================[X]
            std::vector<RE::VMHandle> result = registeredHandles;
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return result;
		}
		
		//mutex locked = true (locks internally -- do not call while holding mutex)
		//replaces the whole handle list. Used by Load to refresh an event that survived the save.
		bool SetRegisteredHandles(const std::vector<RE::VMHandle>& handles){
			mutex.lock(); //[X]===============================================================[X]
			if (deleted){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return false;
			}
			registeredHandles = handles;
			bool result = !registeredHandles.empty();
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return result;
		}

		//mutex locked = true (locks internally -- do not call while holding mutex)
		//identity of a native event: the conditionId string plus the target handle.
		//Used instead of ParamsMatch during Load, because the TESCondition* from the previous
		//session is meaningless after a game load.
		bool MatchesNativeKey(const std::string& asConditionId, RE::ObjectRefHandle akTarget){
			mutex.lock(); //[X]===============================================================[X]
			bool result = (nativeEvent && !deleted && conditionId == asConditionId && target == akTarget);
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return result;
		}
		
        //can only set if isPolling is the opposite of set
        bool SetPolling(bool set) {
            mutex.lock(); //[X]===============================================================[X]
            if (set) {
                if (!isPolling) {
                    isPolling = true;
                    mutex.unlock(); //[O]-------------------------------------------------------------[O]
                    return true;
                }
                else {
                    mutex.unlock(); //[O]-------------------------------------------------------------[O]
                    return false;
                }
            }
            else {
                if (isPolling) {
                    isPolling = false;
                    mutex.unlock(); //[O]-------------------------------------------------------------[O]
                    return true;
                }
                else {
                    mutex.unlock(); //[O]-------------------------------------------------------------[O]
                    return false;
                }
            }
        }

		bool RegisterForConditionChanged(RE::VMHandle handle){
			mutex.lock(); //[X]===============================================================[X]
			if (std::find(registeredHandles.begin(), registeredHandles.end(), handle) == registeredHandles.end()){
				registeredHandles.push_back(handle);
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return true;
			}
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		//mutex locked = true
		bool UnregisterForConditionChanged(RE::VMHandle handle){
			mutex.lock(); //[X]===============================================================[X]
			auto it = std::find(registeredHandles.begin(), registeredHandles.end(), handle);
			if (it != registeredHandles.end()){
				registeredHandles.erase(it);
				auto size = registeredHandles.size();
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				
				if (size == 0){
					DeleteConditionEvent(shared_from_this());
				}
				return true;
			}
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		void ListenForConditionEventChangeStart() {
			if (deleted){
				return;
			}
            ListenForConditionEventChange();
        }
		
    private:
        
		void ConditionChanged() {
			int status = GetConditionStatus();

			if (status != -1) {
				HandleConditionChangeEvent(bool(status));
			}
			else {
				DeleteConditionEvent(shared_from_this());
			}
		}
		
        void ListenForConditionEventChange() {
            isEmpty = false;
            std::thread t([=, this]() {
                if (!SetPolling(true)) {
                    logger::trace("already polling");
                }
                else { //succussfully set polling

					std::weak_ptr<ConditionChangedEvent> weak = weak_from_this();
					
                    int startStatus = GetConditionStatus();
					
					{
						std::unique_lock<std::mutex> lock(mutex);
						updateCv.wait(lock, [weak, startStatus] {
							auto self = weak.lock();
							if (!self || self->deleted || !self->condition) { return true; }
							int status = int(self->condition->IsTrue(GetRefFromHandle(self->target), nullptr));
							return (status != startStatus);
						});
					}

					if (auto* task = SKSE::GetTaskInterface()) {
						task->AddTask([weak] {
							if (auto self = weak.lock()) { self->ConditionChanged(); }
						});
					}
                }
            });
            t.detach();
        }

		//mutex locked = true
        void HandleConditionChangeEvent(bool isTrue) {
			bool didDelete = false;
			
			mutex.lock(); //[X]===============================================================[X]
			if (nativeEvent){
				if (registeredHandles.size() > 0) {
					RE::FormID formID = NativeConditionIdToFormId(conditionId);
					
					if (formID != 0){
						std::vector<RE::VMHandle> handles = registeredHandles;
						RE::TESForm* conditionForm = RE::TESForm::LookupByID(formID);
						auto* args = RE::MakeFunctionArguments((RE::TESForm*)conditionForm, (RE::TESObjectREFR*)GetRefFromHandle(target), (bool)isTrue);
						gfuncs::SendEvents(handles, sEvent, args);
						
					}
					else {
						didDelete = true;
						mutex.unlock(); //[O]-------------------------------------------------------------[O]
						DeleteConditionEvent(shared_from_this()); //mutex locked = true
						mutex.lock(); //[X]===============================================================[X]
					}
				}
				else {
					didDelete = true;
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					DeleteConditionEvent(shared_from_this()); //mutex locked = true 
					mutex.lock(); //[X]===============================================================[X]
				}
			} 
			else {
				auto* bssVm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
				if (bssVm) {
					auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)GetRefFromHandle(target), (bool)isTrue);
					bssVm->SendEventAll(sEvent, args);
					delete args;
				} 
				
			}
			
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			if (!didDelete){
				SetPolling(false);
				ListenForConditionEventChangeStart();
			}
        }
		
	public:
		std::vector<RE::VMHandle> registeredHandles; 
		bool nativeEvent = false;
        bool deleted = false;
        bool isPolling = false;
		PromotedRefType promotedRefType = PromotedRefType::kNotPromoted;
		RE::ObjectRefHandle target;
        // RE::TESObjectREFR* target = nullptr;
        RE::TESCondition* condition = nullptr;
        RE::BSFixedString sEvent = nullptr;
        std::string conditionId = "";
    };

	std::vector<std::shared_ptr<ConditionChangedEvent>> conditionEvents;

	// by value on purpose: the copy pins the object for the whole function, so
	// `delete`-while-executing is impossible and DeleteConditionEvent(shared_from_this())
	// is safe from inside a member function.
	void DeleteConditionEvent(std::shared_ptr<ConditionChangedEvent> conditionEvent){
		mutex.lock(); //[X]===============================================================[X]

		if (!conditionEvent) {
			if (conditionEvents.size() == 0) { isEmpty = true; }
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return;
		}

		// Membership is the ONLY proof this pointer is still live. A stale polling task or
		// thread can reach here with a freed pointer, so nothing above this line may
		// dereference it.
		auto it = std::find(conditionEvents.begin(), conditionEvents.end(), conditionEvent);
		if (it == conditionEvents.end()) {
			logger::debug("condition event already deleted, ignoring");
			if (conditionEvents.size() == 0) { isEmpty = true; }
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return;
		}

		conditionEvents.erase(it);   // claim it before releasing the lock

		if (conditionEvent->promotedRefType != PromotedRefType::kNotPromoted){
			RE::TESObjectREFR* target = GetRefFromHandle(conditionEvent->target);
			if (gfuncs::IsFormValid(target)){
				DemoteConditionRef(target);
			}
		}

		// set the flags inline rather than calling MarkForDelete() -- no unlock/relock gap
		conditionEvent->condition = nullptr;
		conditionEvent->deleted = true;

		if (conditionEvents.size() == 0) { isEmpty = true; }
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		
		updateCv.notify_all();   // wake parked threads so they see the expired weak_ptr and exit
	}
	
	//mutex locked = true
    std::shared_ptr<ConditionChangedEvent> GetConditionEvent(std::string asConditionId, RE::ObjectRefHandle akTarget) {
        mutex.lock(); //[X]===============================================================[X]
        auto it = conditionsMap.find(asConditionId);
        if (it != conditionsMap.end()) {
            auto& conditionData = it->second;
            if (conditionData.condition) {
                for (int i = 0; i < conditionEvents.size(); i++) {
                    std::shared_ptr<ConditionChangedEvent> conditionEvent = conditionEvents[i];
                    if (conditionEvent) {
                        mutex.unlock(); //[O]-------------------------------------------------------------[O] //ParamsMatch is locked, unlock before calling
                        if (conditionEvent->ParamsMatch(asConditionId, conditionData.condition, akTarget)) {
                            return conditionEvent;
                        }
                        mutex.lock(); //[X]===============================================================[X]
                    }
                }
            }
        }
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return nullptr;
    }

    bool ConditionEventExists(RE::StaticFunctionTag*, std::string conditionId, RE::TESObjectREFR* akTarget) {
		RE::ObjectRefHandle handle; 
		if (gfuncs::IsFormValid(akTarget)){
			handle = akTarget->GetHandle();
		}
        return GetConditionEvent(conditionId, handle) != nullptr;
    }

    bool CreateConditionEvent(RE::StaticFunctionTag*, std::string conditionId, RE::TESObjectREFR* akTarget) {
		RE::ObjectRefHandle handle; 
		if (gfuncs::IsFormValid(akTarget)){
			handle = akTarget->GetHandle();
		}
        std::shared_ptr<ConditionChangedEvent> conditionEvent = GetConditionEvent(conditionId, handle);
        mutex.lock(); //[X]===============================================================[X]
        if (!conditionEvent) {
            auto it = conditionsMap.find(conditionId);
            if (it != conditionsMap.end()) {
                auto& conditionData = it->second;
                if (conditionData.condition) {
                    std::shared_ptr<ConditionChangedEvent> newConditionEvent = std::make_shared<ConditionChangedEvent>(conditionId, conditionData.condition, akTarget);
                    
                    mutex.unlock(); //[O]-------------------------------------------------------------[O] //isDeleted is locked, unlock before calling to prevent deadlock.
                    bool deleted = newConditionEvent->IsDeleted();
                    mutex.lock(); //[X]===============================================================[X]

                    if (!deleted) {
                        conditionEvents.push_back(newConditionEvent);
                    	mutex.unlock(); //[O]-------------------------------------------------------------[O] //isDeleted is locked, unlock before calling to prevent deadlock.
						newConditionEvent->ListenForConditionEventChangeStart(); //mutex locked
                    	mutex.lock(); //[X]===============================================================[X]
                    }
                    else {
                        logger::error("condition for conditionId[{}] is nullptr", conditionId);
                        // delete newConditionEvent.get(); // let go out of scope
                    }
                    logger::debug("condition event for conditionId[{}] and target[{}] created",
                        conditionId, gfuncs::GetFormNameAndId(akTarget));

                    mutex.unlock(); //[O]-------------------------------------------------------------[O]
                    return true;
                }
                else {
                    logger::error("condition for conditionId[{}] is nullptr", conditionId);
                }
            }
            else {
                logger::error("condition for conditionId[{}] not found", conditionId);
            }
        }
        else {
            logger::debug("condition event for conditionId[{}] and target[{}] already exists", 
                conditionId, gfuncs::GetFormNameAndId(akTarget));

            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return false;
    }

    bool DestroyConditionEvent(RE::StaticFunctionTag*, std::string conditionId, RE::TESObjectREFR* akTarget) {
		RE::ObjectRefHandle targetHandle; 
		if (gfuncs::IsFormValid(akTarget)){
			targetHandle = akTarget->GetHandle();
		}
        mutex.lock(); //[X]===============================================================[X]
        auto it = conditionsMap.find(conditionId);
        if (it != conditionsMap.end()) {
            auto& conditionData = it->second;
            if (conditionData.condition) {
                for (int i = 0; i < conditionEvents.size(); i++) {
                    std::shared_ptr<ConditionChangedEvent> conditionEvent = conditionEvents[i];
                    if (conditionEvent) {
						
                        mutex.unlock(); //[O]-------------------------------------------------------------[O] //paramsMatch and MarkForDelete are locked. Unlock before calling to prevent deadlock.
                        if (conditionEvent->ParamsMatch(conditionId, conditionData.condition, targetHandle)) {
                            return conditionEvent->MarkForDelete();
                        }
                        mutex.lock(); //[X]===============================================================[X]
                    }
                }
            }
        }

        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return false;
    }
    
    int DestroyAllConditionEvents(RE::StaticFunctionTag*, std::string conditionId) {
        mutex.lock(); //[X]===============================================================[X]
        int count = 0;
        auto it = conditionsMap.find(conditionId);
        if (it != conditionsMap.end()) {
            auto& conditionData = it->second;
            if (conditionData.condition) {
                for (int i = 0; i < conditionEvents.size(); i++) {
                    std::shared_ptr<ConditionChangedEvent> conditionEvent = conditionEvents[i];
                    if (conditionEvent) {
                        mutex.unlock(); //[O]-------------------------------------------------------------[O] //GetConditionId and MarkForDelete are locked. Unlock before calling to prevent deadlock.
                        if (conditionEvent->GetConditionId() == conditionId) {
                            if (conditionEvent->MarkForDelete()) {
                                count++;
                            }
                        }
                        mutex.lock(); //[X]===============================================================[X]
                    }
                }
            }
        }

        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return count;
    }

    int CountConditionEvents(RE::StaticFunctionTag*, std::string conditionId) {
        mutex.lock(); //[X]===============================================================[X]
        int count = 0;
        auto it = conditionsMap.find(conditionId);
        if (it != conditionsMap.end()) {
            auto& conditionData = it->second;
            if (conditionData.condition) {
                for (int i = 0; i < conditionEvents.size(); i++) {
                    std::shared_ptr<ConditionChangedEvent> conditionEvent = conditionEvents[i];
                    if (conditionEvent) {
                        mutex.unlock(); //[O]-------------------------------------------------------------[O] //GetConditionId and MarkForDelete are locked. Unlock before calling to prevent deadlock.
                        if (conditionEvent->GetConditionId() == conditionId) {
                            if (!conditionEvent->IsDeleted()) {
                                count++;
                            }
                        }
                        mutex.lock(); //[X]===============================================================[X]
                    }
                }
            }
        }

        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return count;
    }
	
	//mutex locked = false
	std::vector<std::shared_ptr<ConditionChangedEvent>> GetAllNativeConditionChangedEvents(RE::TESCondition* condition, bool excludeDeletedEvents){
		std::vector<std::shared_ptr<ConditionChangedEvent>> v;
		
		if (!condition){
			return v;
		}
		
		for (std::shared_ptr<ConditionChangedEvent> event : conditionEvents){
			if (event){
				if (event->deleted && excludeDeletedEvents){
					continue;
				}
				
				if (event->condition == condition){
					v.push_back(event);
				}
			}
		}
		
		return v;
	}
	
	int GetNumberOfConditionItemParams(RE::TESConditionItem* item){
		if (!item){
			return 0;
		}
		
		int i = 0;
		for (void* param : item->data.functionData.params){
			i++;
		}
		
		return i;
	}
	
	// enum class ParamType { kRaw, kBool, kInt, kFloat, kString, kForm, kAlias};
	// mutex locked = false
    void DestroyConditionParameter(void* param) {
		auto it = paramDatas.find(param);
		//param was not created by this plugin, not safe to delete
		if (it == paramDatas.end()){
			return;
		}
		
		void* p = it->first;
		
		auto type = it->second.paramType;
		if (type == ParamType::kString){ //string is the only param type where void* is created with "new". The only one that needs deleting.
			delete static_cast<RE::BSFixedString*>(p);
		}
		else if (type == ParamType::kObjectReference){
			if (it->second.promotedRefData.type != PromotedRefType::kNotPromoted){
				RE::TESForm* form = ConditionParamToForm(p);
				if (gfuncs::IsFormValid(form)){
					RE::TESObjectREFR* ref = form->AsReference();
					if (gfuncs::IsFormValid(ref)){
						DemoteConditionRef(ref);
					}
				}
			}
		}
		
        paramDatas.erase(it);
    }
	
	// mutex locked = false
	void DestroyConditionItem(RE::TESConditionItem* item){
		if (!item){
			return;
		}
		
		DestroyConditionParameter(item->data.functionData.params[0]);
		DestroyConditionParameter(item->data.functionData.params[1]);
		
		auto it = createdConditionItems.find(item);
		
		if (it != createdConditionItems.end()){
			createdConditionItems.erase(it);
			delete item;
		}
	}
	
	
	//mutex locked = false
	void DeleteTESCondition(RE::TESCondition* condition){
		auto it = createdConditions.find(condition);
		if (it != createdConditions.end()){
			auto events = GetAllNativeConditionChangedEvents(condition, false);
			bool canDelete = true;
			for (auto event : events){
				if (event){
					if (event->isPolling){
						canDelete = false;
						break;
					}
				}
			}
			
			if (canDelete){
				createdConditions.erase(it);
				auto itt = conditionsToDelete.find(condition);
				if (itt != conditionsToDelete.end()){
					conditionsToDelete.erase(itt);
				}
				delete condition;
			}
			else {
				conditionsToDelete.insert(condition);
			}
		}
	}
	
	//mutex locked = true
	void DestroyConditionCpp(ConditionData& conditionData, bool deletePapyrusConditionForms){
		if (!conditionData.condition){
			return;
		}
		
        mutex.lock(); //[X]===============================================================[X]
		
		std::vector<RE::TESConditionItem*> items = GetAllConditionItems(conditionData.condition);
		for (int i = items.size() - 1; i >= 0; i--){
			DestroyConditionItem(items[i]);
		}
		
		if (deletePapyrusConditionForms){
			RE::TESForm* listForm = RE::TESForm::LookupByID(conditionData.conditionListFormID);
			if (gfuncs::IsFormValid(listForm, false, false)){
				conditionFormsToDelete.insert(listForm);
			}
			
			conditionData.markedForDelete = true;
			for (int i = 0; i < conditionData.linkedConditionItemFormIds.size(); i++){
				RE::TESForm* linkedForm = RE::TESForm::LookupByID(conditionData.linkedConditionItemFormIds[i]);
				if (gfuncs::IsFormValid(linkedForm, false, false)){
					conditionFormsToDelete.insert(linkedForm);
				}
			}
		}
		
		conditionData.condition->head = nullptr; 
		
		if (conditionData.condition){
			DeleteTESCondition(conditionData.condition);
			conditionData.condition = nullptr;
		}
	
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
	}
	
    void DestroyConditionCpp(std::string conditionId, bool deletePapyrusConditionForms) {
        DestroyAllConditionEvents(nullptr, conditionId);
        mutex.lock(); //[X]===============================================================[X]

        logger::trace("conditionId[{}]", conditionId);

        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return;
        }

		ConditionData conditionData = it->second;
		
		conditionsMap.erase(it);
		
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
		DestroyConditionCpp(conditionData, deletePapyrusConditionForms);
    }

	void DestroyConditionCpp(RE::FormID linkedFormID, bool deletePapyrusConditionForms) {
        // DestroyAllConditionEvents(nullptr, conditionId);
        mutex.lock(); //[X]===============================================================[X]

        logger::trace("conditionId[{:08X}]", linkedFormID);

        auto it = conditionsFormsMap.find(linkedFormID);

        if (it == conditionsFormsMap.end()) {
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return;
        }

		std::shared_ptr<ConditionData> dataPtr = it->second;   // keeps it alive
		
		conditionsFormsMap.erase(it);
		
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
		DestroyConditionCpp(*dataPtr, deletePapyrusConditionForms);
    }
	
    void DestroyCondition(RE::StaticFunctionTag*, std::string conditionId) {
        DestroyConditionCpp(conditionId, false);
    }

	void CreateCondition(ConditionData& data, int conditionFunction, int comparison, float value){
		mutex.lock(); //[X]===============================================================[X]
        
        RE::TESCondition* condition = new RE::TESCondition;
        RE::TESConditionItem* conditionItem = new RE::TESConditionItem;
		conditionItem->next = nullptr; 
		
        conditionItem->data.functionData.function = static_cast<RE::FUNCTION_DATA::FunctionID>(conditionFunction);
        
        if (comparison <= 5 && comparison >= 0) {
            conditionItem->data.flags.opCode = static_cast<RE::CONDITION_ITEM_DATA::OpCode>(comparison);
        }
		
        conditionItem->data.comparisonValue.f = value;
        condition->head = conditionItem;
		data.condition = condition; 
		createdConditions.insert(condition);
		createdConditionItems.insert(conditionItem);
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
	}
	
    //mutex locked = false -- CALLER MUST HOLD mutex (body of the old CreateCondition, minus the lock)
	void CreateConditionUnlocked(ConditionData& data, int conditionFunction, int comparison, float value) {
		RE::TESCondition* condition = new RE::TESCondition;
		RE::TESConditionItem* conditionItem = new RE::TESConditionItem;
		conditionItem->next = nullptr; 
		
		conditionItem->data.functionData.function = static_cast<RE::FUNCTION_DATA::FunctionID>(conditionFunction);
		if (comparison <= 5 && comparison >= 0) {
			conditionItem->data.flags.opCode = static_cast<RE::CONDITION_ITEM_DATA::OpCode>(comparison);
		}
		conditionItem->data.comparisonValue.f = value;

		condition->head = conditionItem;
		data.condition = condition;
		createdConditions.insert(condition);
		createdConditionItems.insert(conditionItem);
	}

	void CreateCondition(ConditionData& data, std::string conditionId, int conditionFunction, int comparison, float value) {
		if (conditionId == "") {
			logger::warn("Cannot create a condition with an empty conditionId");
			return;
		}

		DestroyConditionCpp(conditionId, false);   // locks internally; must be before our lock

		mutex.lock(); //[X]===============================================================[X]
		CreateConditionUnlocked(data, conditionFunction, comparison, value);
		conditionsMap[conditionId] = data;
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
	}
	
	void CreateCondition(std::shared_ptr<ConditionData> data, RE::FormID conditionId, int conditionFunction, int comparison, float value){
		if (conditionId == 0) {
            logger::warn("Cannot create a condition with an empty condition formID");
            return;
        }
		
		mutex.lock(); //[X]===============================================================[X]
		data->linkedConditionItemFormIds.push_back(conditionId);
        conditionsFormsMap[conditionId] = data;
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
		
		logger::trace("conditionId[{:08X}], conditionFunction[{}], comparison[{}], value[{}]",
            conditionId, conditionFunction, comparison, value);
		
		CreateCondition(*data, conditionFunction, comparison, value);
	}
	
    void CreateStringIdCondition(RE::StaticFunctionTag*, std::string conditionId, int conditionFunction, int comparison, float value) {
		ConditionData data;
		data.conditionType = ConditionType::kStringId;
		CreateCondition(data, conditionId, conditionFunction, comparison, value);
    }

    bool ConditionExists(RE::StaticFunctionTag*, std::string conditionId) {
        mutex.lock(); //[X]===============================================================[X]
        bool result = (conditionsMap.find(conditionId) != conditionsMap.end());
        logger::trace("conditionId[{}] result[{}]", conditionId, result);
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return result;
    }

	//mutex locked = true
	int GetFunctionParameterBaseType(RE::StaticFunctionTag*, int functionId, int paramIndex) {
		mutex.lock(); //[X]===============================================================[X]
		if (static_cast<RE::FUNCTION_DATA::FunctionID>(functionId) == RE::FUNCTION_DATA::FunctionID::kGetEventData){
			if (paramIndex > 0){ //for kGetEventData, index 0 and 1 are combined in params[0], index 2 is params[1]
				paramIndex--;
			}
		}
		int value = static_cast<int>(GetFunctionParameterBaseTypeCPP(functionId, paramIndex));
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	} 
	
	//mutex locked = true
	std::string GetFunctionParameterBaseTypeString(RE::StaticFunctionTag*, int functionId, int paramIndex) {
		mutex.lock(); //[X]===============================================================[X]
		if (static_cast<RE::FUNCTION_DATA::FunctionID>(functionId) == RE::FUNCTION_DATA::FunctionID::kGetEventData){
			if (paramIndex > 0){ //for kGetEventData, index 0 and 1 are combined in params[0], index 2 is params[1]
				paramIndex--;
			}
		}
		std::string value = static_cast<std::string>(magic_enum::enum_name(GetFunctionParameterBaseTypeCPP(functionId, paramIndex)));
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	} 
	
	//mutex locked = true
	int GetFunctionParameterType(RE::StaticFunctionTag*, int functionId, int paramIndex) {
		mutex.lock(); //[X]===============================================================[X]
		if (static_cast<RE::FUNCTION_DATA::FunctionID>(functionId) == RE::FUNCTION_DATA::FunctionID::kGetEventData){
			if (paramIndex > 0){ //for kGetEventData, index 0 and 1 are combined in params[0], index 2 is params[1]
				paramIndex--;
			}
		}
		int value = static_cast<int>(GetFunctionParameterTypeCpp(functionId, paramIndex));
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
	//mutex locked = true
	std::string GetFunctionParameterTypeString(RE::StaticFunctionTag*, int functionId, int paramIndex) {
		mutex.lock(); //[X]===============================================================[X]
		if (static_cast<RE::FUNCTION_DATA::FunctionID>(functionId) == RE::FUNCTION_DATA::FunctionID::kGetEventData){
			if (paramIndex > 0){ //for kGetEventData, index 0 and 1 are combined in params[0], index 2 is params[1]
				paramIndex--;
			}
		}
		std::string value = static_cast<std::string>(magic_enum::enum_name(GetFunctionParameterTypeCpp(functionId, paramIndex)));
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
    bool SetConditionComparison(RE::StaticFunctionTag*, std::string conditionId, int comparison) {
        mutex.lock(); //[X]===============================================================[X]

        if (comparison > 5 || comparison < 0) {
            logger::warn("conditionId[{}] comparison[{}] isn't valid. Must be between 0 and 5", conditionId, comparison);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        } 

        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            logger::warn("conditionId[{}] not found", conditionId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }

		ConditionData& data = it->second;
		
        RE::TESConditionItem* item = data.condition ? data.condition->head : nullptr;
        if (!item) {
            logger::warn("something went wrong. conditionId[{}] condition->head is nullptr", conditionId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        } 

        logger::trace("conditionId[{}] comparison[{}]", conditionId, comparison);
        item->data.flags.opCode = static_cast<RE::CONDITION_ITEM_DATA::OpCode>(comparison);
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return true;
    }

    bool SetConditionValue(RE::StaticFunctionTag*, std::string conditionId, float value) {
        mutex.lock(); //[X]===============================================================[X]

        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            logger::warn("conditionId[{}] not found", conditionId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }

		ConditionData& data = it->second;
        RE::TESConditionItem* item = data.condition ? data.condition->head : nullptr;
    
        if (!item) {
            logger::warn("something went wrong. conditionId[{}] condition->item is nullptr", conditionId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }

        logger::trace("conditionId[{}] value[{}]", conditionId, value);
        item->data.comparisonValue.f = value;
        
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return true;
    }

	//mutex locked = false
    bool SetConditionParameter(std::string conditionId, ConditionData& data, void* param, int paramIndex, ParamType type, ParamData& paramData) {
        if (paramIndex < 0 || paramIndex > 1) {
            logger::warn("conditionId[{}] paramIndex[{}] not valid. Must be between 0 and 2", conditionId, paramIndex);
            return false;
        }

		
        RE::TESConditionItem* item = data.condition ? data.condition->head : nullptr;
		
        if (!item) {
            logger::warn("something went wrong. conditionId[{}] condition->item is nullptr", conditionId);
            return false;
        }

        DestroyConditionParameter(item->data.functionData.params[paramIndex]);
		
        item->data.functionData.params[paramIndex] = param;
        if (type == ParamType::kString || type == ParamType::kObjectReference){
			paramData.data = param;
			paramData.paramType = type;
			paramDatas[param] = paramData;
		}

        return true;
    }

	//mutex locked = true
    bool SetConditionParameterForm(RE::StaticFunctionTag*, std::string conditionId, RE::TESForm* param, int paramIndex) {
        mutex.lock(); //[X]===============================================================[X]
		
		auto it = conditionsMap.find(conditionId);
		if (it == conditionsMap.end()){
			logger::error("condition for [{}] not found", conditionId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		ConditionData& data = it->second;
		
		ParamType paramType = ParamType::kForm; 
		
		if (!ParameterBaseTypeMatches(conditionId, paramType, data.condition, paramIndex)){
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		RE::VMHandle handle;
		RE::ObjectRefHandle objHandle;
		PromotedRefType promotedRefType = PromotedRefType::kNotPromoted;
		
		RE::TESObjectREFR* ref = nullptr;
        if (gfuncs::IsFormValid(param)) {
			handle = gfuncs::GetHandle(param);
			ref = param->AsReference();
        	if (gfuncs::IsFormValid(ref)) { 
				paramType = ParamType::kObjectReference;
				objHandle = ref->GetHandle();
				promotedRefType = PromoteConditionRef(ref);
			}
        } 
		else {
			param = nullptr;
		}
		
		ParamData paramData; 
		paramData.handle = handle; 
		paramData.objRefHandle = objHandle;
		paramData.promotedRefData.type = promotedRefType;
        // logger::trace("conditionId[{}] param[{}] paramIndex[{}] type[{}]", conditionId, gfuncs::GetFormName(param), paramIndex, type);
        bool result = SetConditionParameter(conditionId, data, param, paramIndex, paramType, paramData);
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
		
		return result;
    }

    bool SetConditionParameterAlias(RE::StaticFunctionTag*, std::string conditionId, RE::BGSBaseAlias* param, int paramIndex) {
        logger::error("this function is is depreciated. There are no condition param alias types");
		return false;
		
		// mutex.lock(); //[X]===============================================================[X]
        // auto it = conditionsMap.find(conditionId);
		// if (it == conditionsMap.end()){
		// 	logger::error("condition for [{}] not found", conditionId);
        //     mutex.unlock(); //[O]-------------------------------------------------------------[O]
		// 	return false;
		// }
		
		// ConditionData& data = it->second;
		
		// ParamData paramData; 
		// paramData.handle = gfuncs::GetHandle(param);
		
		// logger::trace("conditionId[{}] param[{}] paramIndex[{}]", conditionId, param? param->aliasName : "null", paramIndex);
        
        // bool result = SetConditionParameter(conditionId, data, param, paramIndex, ParamType::kAlias, paramData); //this function is mutex locked
		// mutex.unlock(); //[O]-------------------------------------------------------------[O]
        // return result;
    }

    bool SetConditionParameterBool(RE::StaticFunctionTag*, std::string conditionId, bool param, int paramIndex) {
        logger::error("this function is is depreciated. There are no condition param bool types");
		return false;
		
		// mutex.lock(); //[X]===============================================================[X]
        // auto it = conditionsMap.find(conditionId);
		// if (it == conditionsMap.end()){
		// 	logger::error("condition for [{}] not found", conditionId);
        //     mutex.unlock(); //[O]-------------------------------------------------------------[O]
		// 	return false;
		// }
		
		// ConditionData& data = it->second;
		
		// ParamData paramData; 
		
		// logger::trace("conditionId[{}] param[{}] paramIndex[{}]", conditionId, param, paramIndex);
        // // bool* paramPtr = new bool(param);
        // void* slot = BoolToConditionParam(param);
		// bool result = SetConditionParameter(conditionId, data, slot, paramIndex, ParamType::kBool, paramData);
        // mutex.unlock(); //[O]-------------------------------------------------------------[O]
        // return result;
    }

	
	// Many condition functions take small enums/indices stored directly in the
	// pointer field rather than as a pointer to a value. GetIsSex, GetItemCount's
	// count, actor value indices, etc. Compare against a vanilla condition's raw
	// params to tell which form a given function expects.
	bool SetConditionParameterRaw(RE::StaticFunctionTag*, std::string conditionId, int param, int paramIndex) {
        mutex.lock(); //[X]===============================================================[X]
		auto it = conditionsMap.find(conditionId);
		if (it == conditionsMap.end()){
			logger::error("condition for [{}] not found", conditionId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		ConditionData& data = it->second;
		
		if (!ParameterBaseTypeMatches(conditionId, ParamType::kInt, data.condition, paramIndex)){
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		ParamData paramData; 
		
		logger::trace("conditionId[{}] raw param[{}] paramIndex[{}]", conditionId, param, paramIndex);
		void* packed = RawToConditionParam(param);
		bool result = SetConditionParameter(conditionId, data, packed, paramIndex, ParamType::kInt, paramData);   // not heap-allocated
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return result;
	}
	
    bool SetConditionParameterInt(RE::StaticFunctionTag*, std::string conditionId, int param, int paramIndex) {
		//int and raw are stored the same. Keeping stringId int function as it's already been released as part of the api.
		return SetConditionParameterRaw(nullptr, conditionId, param, paramIndex);
    }

	
    bool SetConditionParameterFloat(RE::StaticFunctionTag*, std::string conditionId, float param, int paramIndex) {
        mutex.lock(); //[X]===============================================================[X]
        auto it = conditionsMap.find(conditionId);
		if (it == conditionsMap.end()){
			logger::error("condition for [{}] not found", conditionId);
        	mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		ConditionData& data = it->second;
		if (!ParameterBaseTypeMatches(conditionId, ParamType::kFloat, data.condition, paramIndex)){
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		ParamData paramData; 
		
		logger::trace("conditionId[{}] param[{}] paramIndex[{}]", conditionId, param, paramIndex);
        // float* paramPtr = new float(param);
		void* slot = FloatToConditionParam(param);
		bool result = SetConditionParameter(conditionId, data,  slot, paramIndex, ParamType::kFloat, paramData);
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return result;
    }

    bool SetConditionParameterString(RE::StaticFunctionTag*, std::string conditionId, std::string param, int paramIndex) {
        mutex.lock(); //[X]===============================================================[X]
        auto it = conditionsMap.find(conditionId);
		if (it == conditionsMap.end()){
			logger::error("condition for [{}] not found", conditionId);
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		ConditionData& data = it->second;
		if (!ParameterBaseTypeMatches(conditionId, ParamType::kString, data.condition, paramIndex)){
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		ParamData paramData; 
		
		logger::trace("conditionId[{}] param[{}] paramIndex[{}]", conditionId, param, paramIndex);
        //must use new fixed string or it causes ctd when using EvaluateCondition
        void* paramPtr = StringToConditionParam(param);
        bool result = SetConditionParameter(conditionId, data, paramPtr, paramIndex, ParamType::kString, paramData);
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return result;
    }
    
    bool EvaluateCondition(RE::StaticFunctionTag*, std::string conditionId, RE::TESObjectREFR* target) {
        mutex.lock(); //[X]===============================================================[X]

        bool result = false;
        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            logger::warn("conditionId[{}] not found", conditionId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return result;
        }

        auto* condition = it->second.condition;
        if (!condition) {
            logger::warn("something went wrong. conditionId[{}] condition is nullptr", conditionId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return result;
        }

        // logger::trace("conditionId[{}]", conditionId);
        result = condition->IsTrue(target, nullptr);
        logger::trace("conditionId[{}] result[{}]", conditionId, result);
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return result;
    }
	
//ConditionList.psc / Condition.psc native functions ===========================================================================================================

// general condition functions ===========================================================================================================================================
	
	//mutex locked = false
	std::vector<RE::TESQuestStageItem*> GetQuestStageItems(RE::TESQuest* akQuest){
		std::vector<RE::TESQuestStageItem*> v;
		
		if (!gfuncs::IsFormValid(akQuest)){
			return v;
		}
		
		if (akQuest->executedStages){
			// logger::info("executedStages size[{}]", akQuest->executedStages->size());
			auto& executedStages = *akQuest->executedStages;
			for (int i = 0; i < executedStages.size(); i++){
				auto& stage = *executedStages[i];
				auto flag = stage.data.flags.get();
				auto name = magic_enum::enum_name(flag);
				// logger::info("stage flag[{} - {}] index[{}]", 
				// 	static_cast<int>(flag), name, stage.data.index
				// );
				
				if (name != ""){ //stage can be garbage stage, especially in executedStages. If name == "" it's not a valid stage type.
					RE::TESQuestStageItem* item = gfuncs::GetQuestStageItem(&stage, 0); //index always must be 0.
					if (item){
						v.push_back(item);
						// logger::info("item index[{}] data[{}] onwer[{}] owningStage index[{}] owningStage flags[{}]",
						// 	item->index, item->data, gfuncs::GetFormNameAndId(item->owner),
						// 	item->owningStage->data.index, magic_enum::enum_name(item->owningStage->data.flags.get())
						// );
					}
					// else {
					// 	logger::info("TESQuestStageItem not found");
					// }
				}
			}
		}
			
		if (akQuest->waitingStages){
			// logger::info("waitingStages size[{}]", akQuest->waitingStages->size());
			auto& waitingStages = *akQuest->waitingStages;
			for (int i = 0; i < waitingStages.size(); i++){
				auto* stage = *waitingStages[i];
				auto flag = stage->data.flags.get();
				auto name = magic_enum::enum_name(flag);
				// logger::info("stage flag[{} - {}] index[{}]", 
				// 	static_cast<int>(flag), name, stage->data.index
				// );
				
				if (name != ""){
					RE::TESQuestStageItem* item = gfuncs::GetQuestStageItem(stage, 0);
					if (item){
						v.push_back(item);
						// logger::info("item index[{}] data[{}] onwer[{}] owningStage index[{}] owningStage flags[{}]",
						// 	item->index, item->data, gfuncs::GetFormNameAndId(item->owner),
						// 	item->owningStage->data.index, magic_enum::enum_name(item->owningStage->data.flags.get())
						// );
					}
					// else {
					// 	logger::info("TESQuestStageItem not found");
					// }
				}
			}
		}
		
		return v;
	}
	
	//mutex locked = false
	RE::TESQuestStageItem* GetQuestStageByStageIndex(RE::TESQuest* akQuest, int stageIndex){
		
		if (!gfuncs::IsFormValid(akQuest)){
			return nullptr;
		}
		
		if (akQuest->executedStages){
			// logger::info("executedStages size[{}]", akQuest->executedStages->size());
			auto& executedStages = *akQuest->executedStages;
			for (int i = 0; i < executedStages.size(); i++){
				auto& stage = *executedStages[i];
				auto flag = stage.data.flags.get();
				auto name = magic_enum::enum_name(flag);
				// logger::info("stage flag[{} - {}] index[{}]", 
				// 	static_cast<int>(flag), name, stage.data.index
				// );
				
				if (name != ""){ //stage can be garbage stage, especially in executedStages. If name == "" it's not a valid stage type.
					RE::TESQuestStageItem* item = gfuncs::GetQuestStageItem(&stage, 0); //index always must be 0.
					if (item){
						if (item->owningStage->data.index == stageIndex){
							return item;
						}
						// logger::info("item index[{}] data[{}] onwer[{}] owningStage index[{}] owningStage flags[{}]",
						// 	item->index, item->data, gfuncs::GetFormNameAndId(item->owner),
						// 	item->owningStage->data.index, magic_enum::enum_name(item->owningStage->data.flags.get())
						// );
					}
					// else {
					// 	logger::info("TESQuestStageItem not found");
					// }
				}
			}
		}
			
		if (akQuest->waitingStages){
			// logger::info("waitingStages size[{}]", akQuest->waitingStages->size());
			auto& waitingStages = *akQuest->waitingStages;
			for (int i = 0; i < waitingStages.size(); i++){
				auto* stage = *waitingStages[i];
				auto flag = stage->data.flags.get();
				auto name = magic_enum::enum_name(flag);
				// logger::info("stage flag[{} - {}] index[{}]", 
				// 	static_cast<int>(flag), name, stage->data.index
				// );
				
				if (name != ""){
					RE::TESQuestStageItem* item = gfuncs::GetQuestStageItem(stage, 0);
					if (item){
						if (item->owningStage->data.index == stageIndex){
							return item;
						}
						// logger::info("item index[{}] data[{}] onwer[{}] owningStage index[{}] owningStage flags[{}]",
						// 	item->index, item->data, gfuncs::GetFormNameAndId(item->owner),
						// 	item->owningStage->data.index, magic_enum::enum_name(item->owningStage->data.flags.get())
						// );
					}
					// else {
					// 	logger::info("TESQuestStageItem not found");
					// }
				}
			}
		}
		
		return nullptr;
	}
	
	//mutex locked = false
	RE::TESQuestStageItem* GetQuestStageItemByArrayIndex(RE::TESQuest* akQuest, int arrayIndex){
		auto stages = GetQuestStageItems(akQuest);
		auto size = stages.size();
		if (arrayIndex >= 0 && arrayIndex < size && size > 0){
			return stages[arrayIndex];
		}
		
		return nullptr;
	}
	
	//mutex locked = false
	RE::TESCondition* GetQuestStageConditions(RE::TESQuest* akQuest, int index, bool useStageIndex){
		if (!gfuncs::IsFormValid(akQuest)){
			return nullptr;
		}
		
		RE::TESQuestStageItem* stageItem = nullptr;
		if (useStageIndex){
			stageItem = GetQuestStageByStageIndex(akQuest, index);
		}
		else {
			stageItem = GetQuestStageItemByArrayIndex(akQuest, index);
		}
		
		if (stageItem){
			return &stageItem->objConditions;
		}
		
		return nullptr;
	} 
	
	//mutex locked = false
	RE::BGSQuestObjective* GetNthQuestObjective(RE::TESQuest* akQuest, int index, bool useObjectiveIndex){
		logger::trace("");
		
		if (!gfuncs::IsFormValid(akQuest)){
			return nullptr;
		} 
		
		RE::BGSQuestObjective* akObjective = nullptr;
		
		if (useObjectiveIndex){
			for (auto* objective : akQuest->objectives){
				if (objective){
					if (objective->index == index){
						akObjective = objective;
					}
				}
			}
		}
		else {
			if (index >= 0 && index < akQuest->objectives.size()){
				akObjective = *akQuest->objectives[index];
			}
		}
		
		if (akObjective){
			return akObjective;
		}
		
		return nullptr;
	}
	
	//mutex locked = false
	RE::TESCondition* GetNthQuestObjectivesNthTargetConditions(RE::TESQuest* akQuest, int objectiveIndex, bool useObjectiveIndex, int targetIndex){
		RE::BGSQuestObjective* objective = GetNthQuestObjective(akQuest, objectiveIndex, useObjectiveIndex);
		if (objective){
			if (targetIndex >= 0 && targetIndex < objective->numTargets){
				auto* target = objective->targets[targetIndex];
				if (target){
					return &target->conditions;
				}
			}
		}
		
		return nullptr;
	}
	
	//mutex locked = false
	RE::TESCondition* GetDialogueConditionsForQuest(RE::TESQuest* akQuest){
		if (!gfuncs::IsFormValid(akQuest)){
			return nullptr;
		}
		
		return &akQuest->objConditions;
	} 
	
	//mutex locked = false
	RE::TESCondition* GetStoryManagerConditionsForQuest(RE::TESQuest* akQuest){
		if (!gfuncs::IsFormValid(akQuest)){
			return nullptr;
		}
		
		return &akQuest->storyManagerConditions;
	} 
	
	//mutex locked = false
	RE::TESCondition* GetStoryManagerConditionsForNodeBase(RE::BGSStoryManagerNodeBase* nodeBase){
		if (!gfuncs::IsFormValid(nodeBase)){
			return nullptr;
		}
		
		return &nodeBase->conditions;
	} 
	
	//mutex locked = false
	RE::TESCondition* GetTopicInfoConditions(RE::TESTopicInfo* topicInfo){
		if (!gfuncs::IsFormValid(topicInfo)){
			return nullptr;
		}
		
		return &topicInfo->objConditions;
	}
	
	//mutex locked = false
	RE::TESCondition* GetSceneConditions(RE::BGSScene* scene){
		if (!gfuncs::IsFormValid(scene)){
			return nullptr;
		}
		
		return &scene->conditions;
	}
	
	//mutex locked = false
	RE::TESCondition* GetNthScenePhaseStartConditions(RE::BGSScene* scene, int nthIndex){
		if (!gfuncs::IsFormValid(scene)){
			return nullptr;
		}
		
		if (nthIndex < 0 || nthIndex >= scene->phases.size()){
			return nullptr;
		}
		
		auto* phase = scene->phases[nthIndex];
		if (!phase){
			return nullptr;
		}
		
		return &phase->startConditions;
	}
	
	//mutex locked = false
	RE::TESCondition* GetNthScenePhaseEndConditions(RE::BGSScene* scene, int nthIndex){
		if (!gfuncs::IsFormValid(scene)){
			return nullptr;
		}
		
		if (nthIndex < 0 || nthIndex >= scene->phases.size()){
			return nullptr;
		}
		
		auto* phase = scene->phases[nthIndex];
		if (!phase){
			return nullptr;
		}
		
		return &phase->completionConditions;
	}
	
	//mutex locked = false
	RE::TESCondition* GetNthMessageMenuButtonConditions(RE::BGSMessage* message, int nthIndex){
		if (!gfuncs::IsFormValid(message)){
			return nullptr;
		}
		
		if (nthIndex < 0 || nthIndex >= message->menuButtons.size()){
			return nullptr;
		}
		
		auto* button = *message->menuButtons[nthIndex];
		if (!button){
			return nullptr;
		}
		
		return &button->conditions;
	}
	
	//mutex locked = false
	RE::TESCondition* GetNthMusicTypeTrackConditions(RE::BGSMusicType* MusicType, int nthIndex){
		logger::trace("nthIndex[{}]", nthIndex);

		if (!gfuncs::IsFormValid(MusicType)){
			return nullptr;
		}

		if (nthIndex < 0 || static_cast<std::uint32_t>(nthIndex) >= MusicType->tracks.size()){
			return nullptr;
		}

		RE::BSIMusicTrack* bsiTrack = MusicType->tracks[nthIndex];
		if (!bsiTrack){
			return nullptr;
		}

		RE::BGSMusicTrack* bgsTrack = skyrim_cast<RE::BGSMusicTrack*>(bsiTrack);

		if (!bgsTrack){
			//a MUST record in a MusicType's track list is a BGSMusicTrackFormWrapper, which OWNS a
			//BGSMusicTrack rather than deriving from one, so skyrim_cast to BGSMusicTrack fails here
			RE::BGSMusicTrackFormWrapper* wrapper = skyrim_cast<RE::BGSMusicTrackFormWrapper*>(bsiTrack);
			if (!gfuncs::IsFormValid(wrapper)){
				return nullptr;
			}

			bgsTrack = wrapper->track;
			if (!bgsTrack){
				return nullptr;
			}
		}

		//CommonLib declares BGSMusicTrack::conditions as an inline TESCondition at 0x10, but the
		//8 bytes there actually hold a TESCondition*. Taking &conditions gives us track + 0x10, so
		//read through it instead of treating it as the list head -- otherwise head is the real
		//TESCondition* and its first 8 bytes read back as a bogus leading TESConditionItem.
		RE::TESCondition** conditionsSlot = reinterpret_cast<RE::TESCondition**>(&bgsTrack->conditions);

		if (!*conditionsSlot){
			//The track has no CTDA at all, so the game left the slot null. Install an empty
			//TESCondition so papyrus can InsertCondition into it, the same as a scene phase with
			//no conditions. This is deliberately NOT added to createdConditions -- see the comment
			//on installedVanillaConditions.
			RE::TESCondition* created = new RE::TESCondition;
			created->head = nullptr;

			*conditionsSlot = created;

			logger::debug("installed an empty TESCondition for MusicType[{}] track[{}]",
				gfuncs::GetFormNameAndId(MusicType), nthIndex);
		}

		return *conditionsSlot;
	}
	
	//mutex locked = false
	RE::TESCondition* GetPackageConditions(RE::TESPackage* akPackage){
		if (!gfuncs::IsFormValid(akPackage)){
			return nullptr;
		}
		
		return &akPackage->packConditions;
	} 
	
	//mutex locked = false
	RE::TESCondition* GetIdleFormConditions(RE::TESIdleForm* akIdleForm){
		if (!gfuncs::IsFormValid(akIdleForm)){
			return nullptr;
		}
		
		return &akIdleForm->conditions;
	} 
	
	//mutex locked = false
	RE::TESCondition* GetFactionConditions(RE::TESFaction* akFaction){
		if (!gfuncs::IsFormValid(akFaction)){
			return nullptr;
		}
		
		return akFaction->vendorData.vendorConditions;
	} 
	
	//mutex locked = false
	RE::TESCondition* GetAliasConditions(RE::BGSBaseAlias* akAlias){
		if (!akAlias){
			return nullptr;
		}
		
		RE::BGSRefAlias* refAlias = skyrim_cast<RE::BGSRefAlias*>(akAlias);
		RE::BGSLocAlias* locAlias  = skyrim_cast<RE::BGSLocAlias*>(akAlias);
		
		if (refAlias){
			return refAlias->conditions;
		}
		else if (locAlias){
			return locAlias->conditions;
		}
		
		return nullptr;
	}
	
	//mutex locked = false
	RE::TESCondition* GetMagicEffectConditions(RE::EffectSetting* effect) {
		if (!gfuncs::IsFormValid(effect)){
			logger::warn("effect doesn't exist");
			return nullptr;
		}
		
		return &effect->conditions; 
	}
	
	//mutex locked = false
	RE::TESCondition* GetNthMagicEffectCondition(RE::MagicItem* magicItem, int index) {
        if (!gfuncs::IsFormValid(magicItem)) {
            logger::warn("magicItem doesn't exist");
            return nullptr;
        }
        
		if (index >= magicItem->effects.size() || index < 0){
            logger::warn("index[{}] is out of range of effect size[{}]", index, magicItem->effects.size());
			return nullptr;
		}
		
		auto* effect = magicItem->effects[index];
		if (!effect){
            logger::warn(" magicItem->effects[index] [{}] doesn't exist", index);
			return nullptr;
		}		
		
		return &effect->conditions;
    } 
	
	//mutex locked = false
	RE::TESCondition* GetPerkConditions(RE::BGSPerk* perk) {
		if (!gfuncs::IsFormValid(perk)){
			logger::warn("perk doesn't exist");
			return nullptr;
		}
		
		return &perk->perkConditions; 
	}
	
	//mutex locked = false
	RE::TESCondition* GetNthPerkEntryCondition(RE::BGSPerk* akPerk, int index, bool owner) {
        if (!gfuncs::IsFormValid(akPerk)) {
            logger::warn("akPerk doesn't exist");
            return nullptr;
        }
        
		if (index >= akPerk->perkEntries.size() || index < 0){
            logger::warn("entries index[{}] on perk[{}] is out of range of size[{}]", 
				index, gfuncs::GetFormNameAndId(akPerk), akPerk->perkEntries.size());
			return nullptr;
		}
		
		auto* entry =  akPerk->perkEntries[index];
		if (!entry){
            logger::warn(" perk entry[{}] on perk[{}] doesn't exist", index, gfuncs::GetFormNameAndId(akPerk));
			return nullptr;
		}		
		
		auto* epEntry = skyrim_cast<RE::BGSEntryPointPerkEntry*>(entry);
		if (!epEntry){
            logger::warn(" perk entry[{}] on perk[{}] is not a BGSEntryPointPerkEntry", index, gfuncs::GetFormNameAndId(akPerk));
			return nullptr;
		}		
		
		RE::TESCondition* condition = nullptr;
		
		if (owner){
			condition = &epEntry->conditions[0];
		}
		else {
			condition = &epEntry->conditions[1];
		}
		
		return condition;
    }   
	
	//mutex locked = false
	RE::TESCondition* GetConstructibleObjectConditions(RE::BGSConstructibleObject* ConstructibleObject) {
		if (!gfuncs::IsFormValid(ConstructibleObject)){
			logger::warn("ConstructibleObject doesn't exist");
			return nullptr;
		}
		
		return &ConstructibleObject->conditions; 
	}

	//mutex locked = false
	void* CopyConditionParam(void* param, RE::FUNCTION_DATA::FunctionID functionId, int paramIndex){
		if (!param){
			return nullptr;
		}
		
		bool paramDataFound;
		ParamData data;
		auto it = paramDatas.find(param);
		paramDataFound = (it != paramDatas.end());
		if (paramDataFound){
			data = it->second;
		}
		
		void* returnValue;
		
		int iFunctionId = static_cast<int>(functionId);
		auto paramType = GetFunctionParameterBaseTypeCPP(iFunctionId, paramIndex);
		if (!paramDataFound){
			data.paramType = paramType;
		}
		
		switch (paramType){
			// no bool param types
			// case ParamType::kBool:{
			// 	bool value = ConditionParamToBool(param);
			// 	returnValue = BoolToConditionParam(value);
			// 	break;
			// }
			
			// int and raw are the same.
			// case ParamType::kInt:{
			// 	int value = ConditionParamToInt(param);
			// 	returnValue = IntToConditionParam(value);
			// 	break;
			// }
			
			case ParamType::kInt:{
				int value = ConditionParamToRaw(param);
				returnValue = RawToConditionParam(value);
				break;
			}
			
			case ParamType::kFloat:{
				float value = ConditionParamToFloat(param);
				returnValue = FloatToConditionParam(value);
				break;
			}
			
			case ParamType::kString:{
				std::string value = ConditionParamToString(param);
				returnValue = StringToConditionParam(value);
				break;
			}
			
			case ParamType::kForm: {
				returnValue = param;
				if (!paramDataFound){
					RE::TESForm* akForm = ConditionParamToForm(param);
					if (gfuncs::IsFormValid(akForm)){
						data.handle = gfuncs::GetHandle(akForm);
					}
				}
				break;
			}
			
			case ParamType::kObjectReference: {
				returnValue = param;
				if (!paramDataFound){
					RE::TESForm* akForm = ConditionParamToForm(param);
					if (gfuncs::IsFormValid(akForm)){
						data.handle = gfuncs::GetHandle(akForm);
						RE::TESObjectREFR* ref = akForm->AsReference();
						if (gfuncs::IsFormValid(ref)){
							data.objRefHandle = ref->GetHandle();
							data.promotedRefData.type = PromoteConditionRef(ref);
						}
					}
				}
				break;
			}
			
			// no alias param types
			// case ParamType::kAlias: {
			// 	returnValue = param;
			// 	if (!paramDataFound){
			// 		RE::BGSBaseAlias* alias = ConditionParamToAlias(param);
			// 		if (alias){
			// 			data.handle = gfuncs::GetHandle(alias);
			// 		}
			// 	}
				
			// 	break;
			// }
			
			default:
				returnValue = nullptr;
		}
		
		if (returnValue){
			if (paramType == ParamType::kString || paramType == ParamType::kObjectReference){
				paramDatas[returnValue] = data;
			}
		}
		
		return returnValue;
	}
	
	bool CopyConditionItemData(RE::TESConditionItem* from, RE::TESConditionItem* to) {
		if (!from || !to){
			return false;
		}

		// auto fromFunctionId = from->data.functionData.function.get();
		// int iFromFunctionId = static_cast<int>(fromFunctionId);
		
		to->data.functionData.function = from->data.functionData.function;
		
		to->data.flags.global = from->data.flags.global;
		if (from->data.flags.global){
			to->data.comparisonValue.g = from->data.comparisonValue.g;
		}
		else {
			to->data.comparisonValue.f = from->data.comparisonValue.f;
		}

		to->data.flags.usePackData  = from->data.flags.usePackData;
		to->data.flags.isOR         = from->data.flags.isOR;
		to->data.flags.swapTarget   = from->data.flags.swapTarget;
		to->data.flags.usesAliases  = from->data.flags.usesAliases;
		to->data.flags.opCode       = from->data.flags.opCode;
		to->data.object             = from->data.object;
		to->data.pad32              = from->data.pad32;
		to->data.pad34              = from->data.pad34;
		to->data.functionData.pad02 = from->data.functionData.pad02;
		to->data.functionData.pad04 = from->data.functionData.pad04; 
		to->data.runOnRef = from->data.runOnRef;

		for (int i = 0; i < 2; i++) {
			void* fromParam = from->data.functionData.params[i];
			DestroyConditionParameter(to->data.functionData.params[i]);
			to->data.functionData.params[i] = CopyConditionParam(fromParam, from->data.functionData.function.get(), i);
		}

		// auto functionId = to->data.functionData.function.get();
		// int intFunctionId = static_cast<int>(functionId);
		
		// bool useGlobal = to->data.flags.global;
		// float floatValue = useGlobal ? 0.0f : to->data.comparisonValue.f;
		// RE::FormID globalValueId = 0;
		// if (useGlobal && gfuncs::IsFormValid(to->data.comparisonValue.g)) {
		// 	globalValueId = to->data.comparisonValue.g->GetFormID();
		// }
		
		// // bool usePackData = to->data.flags.usePackData;
		// // bool isOr = to->data.flags.isOR;
		// // bool swapTarget = to->data.flags.swapTarget;
		// // bool usesAliases = to->data.flags.usesAliases;
		// int opCodeComparison = static_cast<int>(to->data.flags.opCode);
		
		// std::uint32_t dataID = to->data.dataID;
		// int obj = static_cast<int>(to->data.object.get());
		// // std::uint32_t pad32 = to->data.pad32;
		// // std::uint32_t pad34 = to->data.pad34;
		// // std::uint16_t pad02 = to->data.functionData.pad02;
		// // std::uint16_t pad04 = to->data.functionData.pad04;
		
		// RE::TESObjectREFR* ref = GetRefFromHandle(to->data.runOnRef);
		// RE::FormID runOnRefId = gfuncs::IsFormValid(ref) ? ref->GetFormID() : 0;
		
		// ParamType paramType0 = GetFunctionParameterBaseTypeCPP(intFunctionId, 0);
		// ParamType paramType1 = GetFunctionParameterBaseTypeCPP(intFunctionId, 1);
		
		// logger::info("COPY to[{}] fn[{}] op[{}] obj[{}] global[{}] f[{}] g[{:08X}] "
		// 		"runOnRef[{:08X}] p0[{}] type0[{}] p1[{}] type1[{}] dataID[{:08X}] fromFn[{}]",
		// 		static_cast<void*>(to), intFunctionId, opCodeComparison, obj, useGlobal,
		// 		floatValue, globalValueId, runOnRefId,
		// 		to->data.functionData.params[0], magic_enum::enum_name(paramType0),
		// 		to->data.functionData.params[1], magic_enum::enum_name(paramType1), dataID, iFromFunctionId
		// 	);
		
		return true;
	}
	
	//mutex locked = false
	bool CopyConditionData(RE::TESCondition* from, RE::TESCondition* to){
		if (!from || !to){
			return false;
		}
		
		auto fromItems = GetAllConditionItems(from);
		auto toItems = GetAllConditionItems(to);
		
		if (toItems.size() == 0 && fromItems.size() > 0){
			RE::TESConditionItem* item = new RE::TESConditionItem;
			item->next = nullptr;
			createdConditionItems.insert(item);
			toItems.push_back(item);
			to->head = item;
		}
		
		int i = 0;
		
		for (; i < fromItems.size() && i < toItems.size(); i++){
			CopyConditionItemData(fromItems[i], toItems[i]);
		}
		
		//add to toItems
		if (fromItems.size() > toItems.size()){
			for (i; i < fromItems.size(); i++){
				RE::TESConditionItem* item = new RE::TESConditionItem;
				item->next = nullptr;
				createdConditionItems.insert(item);
				CopyConditionItemData(fromItems[i], item);
				RE::TESConditionItem* previousItem = toItems[i - 1];
				if (previousItem){
					previousItem->next = item;
				}
				toItems.push_back(item);
			}
		}
		else if (fromItems.size() < toItems.size()){ //remove from toItems
			for (i; i < toItems.size() + 1; i++){
				RE::TESConditionItem* previousItem = toItems[i - 1];
				if (previousItem){
					previousItem->next = nullptr;
				}
			}
		}
		
		return true;
	}
	
	//get conditionData for a condition form created with this plugin that has the condition.psc script attached.
	//mutex locked = false
	std::optional<std::reference_wrapper<ConditionData>> GetConditionDataForForm(RE::TESForm* akForm){
		if (!gfuncs::IsFormValid(akForm)){
			return std::nullopt;
		}
		
		RE::FormID formId = akForm->GetFormID();
		auto it = conditionsFormsMap.find(formId);
		if (it != conditionsFormsMap.end()) {
			// logger::trace("condition[{:x}] found", formId);
			return *it->second; 
		}
		
		logger::debug("condition for form[{:08X}] not found", formId);
		return std::nullopt;
	}	
	
	//get shared_ptr<conditionData> for a condition form created with this plugin that has the condition.psc script attached.
	//mutex locked = false
	std::optional<std::shared_ptr<ConditionData>> GetConditionDataPtrForForm(RE::TESForm* akForm){
		if (!gfuncs::IsFormValid(akForm)){
			return std::nullopt;
		}
		
		RE::FormID formId = akForm->GetFormID();
		auto it = conditionsFormsMap.find(formId);
		if (it != conditionsFormsMap.end()) {
			// logger::trace("condition[{:x}] found", formId);
			return it->second; 
		}
		
		logger::debug("condition for form[{:08X}] not found", formId);
		return std::nullopt;
	}	
	
	RE::TESCondition* GetConditionForFormIdEx(gfuncs::FormIdEx& formIdEx){
		RE::TESForm* baseForm = RE::TESForm::LookupByID(formIdEx.formId); 
		if (gfuncs::IsFormValid(baseForm)){
			switch(formIdEx.type){
				case gfuncs::FormIdExType::kAlias:{
					RE::BGSBaseAlias* value = skyrim_cast<RE::BGSBaseAlias*>(baseForm);
					RE::TESCondition* condition = GetAliasConditions(value);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kMagicEffect:{
					RE::EffectSetting* value = skyrim_cast<RE::EffectSetting*>(baseForm);
					RE::TESCondition* condition = GetMagicEffectConditions(value);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kMagicItem:{
					RE::MagicItem* value = skyrim_cast<RE::MagicItem*>(baseForm);
					RE::TESCondition* condition = GetNthMagicEffectCondition(value, formIdEx.subId);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kConstructibleObj:{
					RE::BGSConstructibleObject* value = skyrim_cast<RE::BGSConstructibleObject*>(baseForm);
					RE::TESCondition* condition = GetConstructibleObjectConditions(value);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kPerk:{
					RE::BGSPerk* value = skyrim_cast<RE::BGSPerk*>(baseForm);
					RE::TESCondition* condition = GetPerkConditions(value);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kPerkEntryPointOwner:{
					RE::BGSPerk* value = skyrim_cast<RE::BGSPerk*>(baseForm);
					RE::TESCondition* condition = GetNthPerkEntryCondition(value, formIdEx.subId, true);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kPerkEntryPointTarget:{
					RE::BGSPerk* value = skyrim_cast<RE::BGSPerk*>(baseForm);
					RE::TESCondition* condition = GetNthPerkEntryCondition(value, formIdEx.subId, false);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kQuest:{
					RE::TESQuest* value = skyrim_cast<RE::TESQuest*>(baseForm);
					RE::TESCondition* condition = GetDialogueConditionsForQuest(value);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kQuestStageIndex:{
					RE::TESQuest* value = skyrim_cast<RE::TESQuest*>(baseForm);
					RE::TESCondition* condition = GetQuestStageConditions(value, formIdEx.subId, true);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kQuestObjectiveIndex:{
					RE::TESQuest* value = skyrim_cast<RE::TESQuest*>(baseForm);
					RE::TESCondition* condition = GetNthQuestObjectivesNthTargetConditions(value, formIdEx.subId, true, formIdEx.subId2);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kStoryManagerQuest:{
					RE::TESQuest* value = skyrim_cast<RE::TESQuest*>(baseForm);
					RE::TESCondition* condition = GetStoryManagerConditionsForQuest(value);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kStoryManagerNodeBase:{
					RE::BGSStoryManagerNodeBase* value = skyrim_cast<RE::BGSStoryManagerNodeBase*>(baseForm);
					RE::TESCondition* condition = GetStoryManagerConditionsForNodeBase(value);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kTopicInfo:{
					RE::TESTopicInfo* value = skyrim_cast<RE::TESTopicInfo*>(baseForm);
					RE::TESCondition* condition = GetTopicInfoConditions(value);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kScene:{
					RE::BGSScene* value = skyrim_cast<RE::BGSScene*>(baseForm);
					RE::TESCondition* condition = GetSceneConditions(value);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kScenePhaseStart:{
					RE::BGSScene* value = skyrim_cast<RE::BGSScene*>(baseForm);
					RE::TESCondition* condition = GetNthScenePhaseStartConditions(value, formIdEx.subId);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kScenePhaseEnd:{
					RE::BGSScene* value = skyrim_cast<RE::BGSScene*>(baseForm);
					RE::TESCondition* condition = GetNthScenePhaseEndConditions(value, formIdEx.subId);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kMessageMenuButton:{
					RE::BGSMessage* value = skyrim_cast<RE::BGSMessage*>(baseForm);
					RE::TESCondition* condition = GetNthMessageMenuButtonConditions(value, formIdEx.subId);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kMusicTypeTrack:{
					RE::BGSMusicType* value = skyrim_cast<RE::BGSMusicType*>(baseForm);
					RE::TESCondition* condition = GetNthMusicTypeTrackConditions(value, formIdEx.subId);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kPackage:{
					RE::TESPackage* value = skyrim_cast<RE::TESPackage*>(baseForm);
					RE::TESCondition* condition = GetPackageConditions(value);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kIdleForm:{
					RE::TESIdleForm* value = skyrim_cast<RE::TESIdleForm*>(baseForm);
					RE::TESCondition* condition = GetIdleFormConditions(value);
					if (condition){
						return condition;
					}
					break;
				}
				
				case gfuncs::FormIdExType::kFaction:{
					RE::TESFaction* value = skyrim_cast<RE::TESFaction*>(baseForm);
					RE::TESCondition* condition = GetFactionConditions(value);
					if (condition){
						return condition;
					}
					break;
				}
			}
		}
		
		return nullptr;
	}
	
	//mutex locked = false
	RE::TESCondition* GetConditionForConditionData(ConditionData& data){
		if (data.conditionType == ConditionType::kVanilla){
			return GetConditionForFormIdEx(data.formIdEx);
		}
		
		return data.condition;
	}
	
	//get condition for a condition form created with this plugin that has the condition.psc script attached
	//mutex locked = false
	RE::TESCondition* GetConditionForForm(RE::TESForm* akForm){ 
		auto result = GetConditionDataForForm(akForm);
		
		if (result){
			ConditionData& data = result->get();
			return GetConditionForConditionData(data);
		}
		
		return nullptr;
	}
	
	//get condition item for a condition form created with this plugin that has the condition.psc script attached
	//mutex locked = false
	RE::TESConditionItem* GetConditionItemForForm(RE::TESForm* akForm){
		if (!gfuncs::IsFormValid(akForm)){
			return nullptr;
		}
		
		RE::FormID formId = akForm->GetFormID();
		
		auto result = GetConditionDataForForm(akForm); 
		if (result){
			ConditionData& data = result->get();
			RE::TESCondition* condition = GetConditionForForm(akForm);
			if (condition){
				auto items = GetAllConditionItems(condition);
				int index = gfuncs::GetIndexInVector(data.linkedConditionItemFormIds, formId);
				if (index > -1 && index < items.size()){
					return items[index];
				}
			}
		}
		
		logger::warn("conditionItem for form[{:08X}] not found", formId);
		return nullptr;
	}

	//mutex locked = false
	void FlagConditionDataAltered(ConditionData& data){
		data.wasAltered = true;
	}
	
	//mutex locked = false
	void FlagConditionDataAltered(std::shared_ptr<ConditionData> data){
		if (data){
			data->wasAltered = true;
		}
	}
	
	//mutex locked = false
	void FlagConditionDataAltered(RE::TESForm* akForm){
		auto result = GetConditionDataPtrForForm(akForm);
		if (result){
			FlagConditionDataAltered(*result);
		}
	}
	
	//mutex locked = false
	RE::TESForm* CreateConditionFormCpp(std::string scriptName) {
		auto* factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSColorForm>();
		auto* carrier = factory ? factory->Create() : nullptr;
		
		if (!carrier) { 
			return nullptr; 
		}

		RE::FormID carrierFormId = carrier->GetFormID();
		std::string sCarriedId = std::format("{:08X}", carrierFormId);
		
		// carrier->SetFormEditorID(std::format("DbCondition_{:08X}", carrierId).c_str());
		carrier->SetFormEditorID(sCarriedId.c_str());
		RE::BSTSmartPointer<RE::BSScript::Object> object = gfuncs::AttachPapyrusScriptToForm(carrier, scriptName.c_str());
		if (object.get() == nullptr) { 
			carrier->SetDelete(true);
			delete carrier;
			return nullptr;
		}
		
		return carrier;
	}
	
// papyrus functions ============================================================================================================

	//ConditionList.psc
	//mutex locked = true
	RE::TESForm* CreateConditionListForm(RE::StaticFunctionTag*, int conditionFunction, int comparison, float value) {
		logger::trace("conditionFunction[{}] comparison[{}] value[{}]", conditionFunction, comparison, value);
		
		RE::TESForm* conditionListForm = CreateConditionFormCpp("ConditionList");
		if (!conditionListForm){
			return nullptr;
		}
		
		RE::FormID conditionListFormId = conditionListForm->GetFormID();
		
		RE::TESForm* conditionForm = CreateConditionFormCpp("Condition");
		if (!conditionForm){
			conditionListForm->SetDelete(true);
			delete conditionListForm;
			return nullptr;
		}
		
		RE::FormID conditionFormId = conditionForm->GetFormID();
		
		auto data = std::make_shared<ConditionData>();
		data->conditionListFormID = conditionListFormId;
		data->conditionType = ConditionType::kCreated;
		
		mutex.lock(); //[X]===============================================================[X]
		conditionsFormsMap[conditionListFormId] = data;
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		
		CreateCondition(data, conditionFormId, conditionFunction, comparison, value); 
		
		return conditionListForm;
	}
	
	//Condition.psc
	//mutex locked = true
	RE::TESForm* CreateConditionForm(RE::StaticFunctionTag*, int conditionFunction, int comparison, float value, RE::TESForm* akConditionListForm, int nthIndex) {
		logger::trace("conditionFunction[{}] comparison[{}] value[{}]", conditionFunction, comparison, value);
		
		RE::TESForm* conditionForm = CreateConditionFormCpp("Condition");
		if (!conditionForm){
			return nullptr;
		}
		
		RE::FormID conditionFormId = conditionForm->GetFormID();
		
		mutex.lock(); //[X]===============================================================[X]
		
		RE::TESForm* conditionListForm = nullptr;
		RE::TESCondition* condition = nullptr;
		std::shared_ptr<ConditionData> data;
		
		RE::TESConditionItem* conditionItem = new RE::TESConditionItem;
		conditionItem->next = nullptr; 
		
        conditionItem->data.functionData.function = static_cast<RE::FUNCTION_DATA::FunctionID>(conditionFunction);
        if (comparison <= 5 && comparison >= 0) {
            conditionItem->data.flags.opCode = static_cast<RE::CONDITION_ITEM_DATA::OpCode>(comparison);
        }
        conditionItem->data.comparisonValue.f = value;
		
		if (gfuncs::IsFormValid(akConditionListForm)){
			auto result = GetConditionDataForForm(akConditionListForm);
			if (result){
				auto& listData = result->get();
				data = conditionsFormsMap[listData.conditionListFormID];
				conditionListForm = RE::TESForm::LookupByID(data->conditionListFormID);
				condition = GetConditionForForm(conditionListForm);
				if (condition){
					auto items = GetAllConditionItems(condition);
					if (items.size() == 0){
						condition->head = conditionItem;
						data->linkedConditionItemFormIds.push_back(conditionFormId);
					}
					//append
					else if (nthIndex < 0 || nthIndex >= items.size()){
						auto* lastItem = items[items.size() - 1];
						if (lastItem){
							lastItem->next = conditionItem;
							data->linkedConditionItemFormIds.push_back(conditionFormId);
						}
					}
					else {
						items.insert(items.begin() + nthIndex, conditionItem);
						condition->head = items[0]; 
						for (int i = 0; i < items.size() - 1; i++){
							if (items[i]){
								items[i]->next = items[i + 1];
							}
						}
						
						if (nthIndex < data->linkedConditionItemFormIds.size()){
							data->linkedConditionItemFormIds.insert(data->linkedConditionItemFormIds.begin() + nthIndex, conditionFormId);
						}
						else {
							data->linkedConditionItemFormIds.push_back(conditionFormId);
						}
					}
				}
			}
		}
		
		if (!conditionListForm){
			conditionListForm = CreateConditionFormCpp("ConditionList");
		}
		
		if (!conditionListForm){
			conditionForm->SetDelete(true);
			delete conditionForm;
			delete conditionItem;
			
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return nullptr;
		}
		
		RE::FormID conditionListFormId = conditionListForm->GetFormID();
		
		if (!data){
			data = std::make_shared<ConditionData>();
		}
		
		data->conditionListFormID = conditionListFormId;
		data->conditionType = ConditionType::kCreated;
		conditionsFormsMap[conditionListFormId] = data;
        conditionsFormsMap[conditionFormId] = data;
		
		if (!condition){
			condition = new RE::TESCondition;
        	condition->head = conditionItem;
		}
		
		if (std::find(data->linkedConditionItemFormIds.begin(), data->linkedConditionItemFormIds.end(), conditionFormId) == data->linkedConditionItemFormIds.end()){
			data->linkedConditionItemFormIds.push_back(conditionFormId);
		}
		
		data->condition = condition; 
		createdConditions.insert(condition);
		createdConditionItems.insert(conditionItem);
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return conditionForm;
	}
	
	std::vector<RE::TESForm*> GetConditionFormsCpp(std::shared_ptr<ConditionData> dataPtr) {
		std::vector<RE::TESForm*> v;
		if (!dataPtr){
			return v;
		}
		
		RE::TESCondition* condition = dataPtr->condition; 
		if (!condition){
			return v;
		}
		
		ConditionData& data = *dataPtr;              
		std::vector<RE::TESConditionItem*> conditionItems = GetAllConditionItems(condition);
		int conditionItemsSize = conditionItems.size(); 
		int i = 0;
		
		if (data.linkedConditionItemFormIds.size() > 0){ 
			// dataPtr = it->second;
			// data = *dataPtr;
			for (i; i < conditionItemsSize && i < data.linkedConditionItemFormIds.size(); i++){
				RE::TESForm* conditionForm = RE::TESForm::LookupByID(data.linkedConditionItemFormIds[i]);
				if (conditionForm){
					v.push_back(conditionForm);
				}
			}
			
			int iSnap = i;
			
			//if there are more created forms than condition items for this condition, delete them.
			for (i; i < data.linkedConditionItemFormIds.size(); i++){
				RE::TESForm* conditionForm = RE::TESForm::LookupByID(data.linkedConditionItemFormIds[i]);
				if (conditionForm){
					// conditionForm->SetDelete(true);
					conditionFormsToDelete.insert(conditionForm);
				}
			} 
			
			data.linkedConditionItemFormIds.resize(iSnap);
			i = iSnap;
		}
		
		for (i; i < conditionItemsSize; i++){
			RE::TESForm* conditionForm = CreateConditionFormCpp("Condition");
			if (conditionForm){
				RE::FormID conditionFormId = conditionForm->GetFormID();
				data.linkedConditionItemFormIds.push_back(conditionFormId);
				conditionsFormsMap[conditionFormId] = dataPtr;
				v.push_back(conditionForm);
			}
		}
		
		return v;
	}
	
	//mutex locked = false
	std::vector<RE::TESForm*> GetVanillaConditionFormsCpp(gfuncs::FormIdEx keyId, RE::TESCondition* condition) {
		std::vector<RE::TESForm*> v;
		
		logger::trace("");
		
		if (!condition){
			return v;
		}
		
		RE::FormID formId = keyId.formId;
		std::string sCarriedId = std::format("{:08X}", formId);
		
		std::shared_ptr<ConditionData> dataPtr;

		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()) {
			dataPtr = it->second;                    // reuse the existing one
		}
		
		RE::TESForm* conditionListForm = nullptr;
		
		if (dataPtr){
			conditionListForm = RE::TESForm::LookupByID(dataPtr->conditionListFormID);
		}
		else {
			// logger::debug("creating ConditionList");
			dataPtr = std::make_shared<ConditionData>();
		}
		
		if (!gfuncs::IsFormValid(conditionListForm)){
			conditionListForm = CreateConditionFormCpp("ConditionList");
			if (!conditionListForm) { 
				logger::error("failed to created conditionListForm for formIdEx formId[{:08X}] subId[{}] subId2[{}] type[{}]",
					keyId.formId, keyId.subId, keyId.subId2, magic_enum::enum_name(keyId.type)
				);
				return v; 
			}

			// logger::debug("ConditionList created");
			
			RE::FormID listId = conditionListForm->GetFormID();
			dataPtr->conditionListFormID = listId;
			conditionsFormsMap[listId] = dataPtr;
		}
		
		dataPtr->condition = condition; 
		dataPtr->conditionType = ConditionType::kVanilla; 
		dataPtr->formIdEx = keyId; 
		createdVanillaConditionIds[keyId] = dataPtr;
		
		return GetConditionFormsCpp(dataPtr);
	}
	
	//mutex locked = false
	std::vector<RE::TESForm*> GetVanillaConditionFormsCpp(RE::TESForm* akForm, RE::TESCondition* condition, gfuncs::FormIdExType type = gfuncs::FormIdExType::kForm, std::uint32_t subId = 0, std::uint32_t subId2 = 0) {
		std::vector<RE::TESForm*> v;
		
		if (!condition){
			return v;
		}
		
		if (!gfuncs::IsFormValid(akForm)){
			return v;
		}
		
		gfuncs::FormIdEx id = {akForm->GetFormID(), subId, subId2, type};
		return GetVanillaConditionFormsCpp(id, condition);
	}
	
	
//ConditionList.psc global native functions ===========================================================================================================

// MagicEffect =============================================================================================================================
	
	//mutex locked = false
	std::vector<RE::TESForm*> GetMagicEffectConditionForms(RE::EffectSetting* effect) {
		RE::TESCondition* condition = GetMagicEffectConditions(effect);
		auto v = GetVanillaConditionFormsCpp(effect, condition, gfuncs::FormIdExType::kMagicEffect);
		return v;
	}
	
	//mutex locked = true
	RE::TESForm* GetMagicEffectConditionListForm(RE::StaticFunctionTag*, RE::EffectSetting* effect){
		if (!gfuncs::IsFormValid(effect)){
			logger::warn("effect doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetMagicEffectConditionForms(effect);
		gfuncs::FormIdEx keyId = {effect->GetFormID(), 0, 0, gfuncs::FormIdExType::kMagicEffect};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	}
	
	//mutex locked = false
	std::vector<RE::TESForm*> GetNthMagicEffectConditionForms(RE::TESForm* magicItemForm, int index) {
		std::vector<RE::TESForm*> v;
		
		logger::trace("index[{}]", index);
		
        if (!gfuncs::IsFormValid(magicItemForm)) {
            logger::warn("akForm doesn't exist");
            return v;
        }

        RE::MagicItem* magicItem = magicItemForm->As<RE::MagicItem>();
        if (!gfuncs::IsFormValid(magicItem)) {
            logger::warn("akForm [{}] isn't a magic item", gfuncs::GetFormNameAndId(magicItemForm));
            return v;
        }
		
		RE::TESCondition* condition = GetNthMagicEffectCondition(magicItem, index);
		v = GetVanillaConditionFormsCpp(magicItemForm, condition, gfuncs::FormIdExType::kMagicItem, index);
		return v;
    } 
	
	//mutex locked = true
	RE::TESForm* GetNthMagicEffectConditionListForm(RE::StaticFunctionTag*, RE::TESForm* magicItemForm, int index){
		//init condition forms if necessarry
		if (!gfuncs::IsFormValid(magicItemForm)) {
            logger::warn("akForm doesn't exist");
            return nullptr;
        }
		
		mutex.lock(); //[X]===============================================================[X]
		std::vector<RE::TESForm*> v = GetNthMagicEffectConditionForms(magicItemForm, index);
		gfuncs::FormIdEx keyId = {magicItemForm->GetFormID(), (uint32_t)index, 0, gfuncs::FormIdExType::kMagicItem};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	}

// Perk =====================================================================================================================================
	
	//mutex locked = false
	std::vector<RE::TESForm*> GetPerkConditionForms(RE::BGSPerk* perk) {
		RE::TESCondition* condition = GetPerkConditions(perk);
		auto v = GetVanillaConditionFormsCpp(perk, condition, gfuncs::FormIdExType::kPerk);
		return v;
	}
	
	//mutex locked = true
	RE::TESForm* GetPerkConditionListForm(RE::StaticFunctionTag*, RE::BGSPerk* perk){
		if (!gfuncs::IsFormValid(perk)){
			logger::warn("perk doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetPerkConditionForms(perk);
		gfuncs::FormIdEx keyId = {perk->GetFormID(), 0, 0, gfuncs::FormIdExType::kPerk};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 
	
	//mutex locked = false
	std::vector<RE::TESForm*> GetNthPerkEntryConditionForms(RE::BGSPerk* akPerk, int index, bool owner) {
		std::vector<RE::TESForm*> v;
		if (!gfuncs::IsFormValid(akPerk)){
			logger::warn("perk doesn't exist");
			return v;
		}
		
		logger::trace("index[{}]", index);
		
		RE::TESCondition* condition = GetNthPerkEntryCondition(akPerk, index, owner);
		gfuncs::FormIdExType type; 
		
		if (owner){
			type = gfuncs::FormIdExType::kPerkEntryPointOwner;
		}
		else {
			type = gfuncs::FormIdExType::kPerkEntryPointTarget;
		}
		
		v = GetVanillaConditionFormsCpp(akPerk, condition, type, index);
		return v;
    }   
	
	//mutex locked = true
	RE::TESForm* GetNthPerkEntryConditionListForm(RE::StaticFunctionTag*, RE::BGSPerk* perk, int index, bool owner){
		if (!gfuncs::IsFormValid(perk)){
			logger::warn("perk doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetNthPerkEntryConditionForms(perk, index, owner);
		gfuncs::FormIdExType type = owner? gfuncs::FormIdExType::kPerkEntryPointOwner : gfuncs::FormIdExType::kPerkEntryPointTarget;
		gfuncs::FormIdEx keyId = {perk->GetFormID(), (uint32_t)index, 0, type};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 
	
// Quest Stage =====================================================================================================================================

	//mutex locked = false
	std::vector<RE::TESForm*> GetQuestStageConditionForms(RE::TESQuest* akQuest, int index, bool useStageIndex) {
		std::vector<RE::TESForm*> v;
		
		if (!gfuncs::IsFormValid(akQuest)){
			logger::warn("akQuest doesn't exist");
			return v;
		}
		
		int keyIndex = index; 
		gfuncs::FormIdExType type = gfuncs::FormIdExType::kQuestStageIndex; //always use the actual index as defined in the stage, that way we're not doubling created forms for the same condition.
		RE::TESQuestStageItem* questStage = nullptr; 
		if (useStageIndex){
			questStage = GetQuestStageByStageIndex(akQuest, index);
			// type = gfuncs::FormIdExType::kQuestStageIndexIndex;
		}
		else {
			questStage = GetQuestStageItemByArrayIndex(akQuest, index);
			// type = gfuncs::FormIdExType::kQuestStageArrayIndex;
		}
		
		if (!questStage){
			logger::warn("quest stage index[{}] not found on quest[{}] useStageIndex[{}]",
				gfuncs::GetFormNameAndId(akQuest), index, useStageIndex
			);
			return v;
		}
		
		keyIndex = questStage->index;
		
		RE::TESCondition* condition = &questStage->objConditions;
		if (!condition){
			logger::warn("quest stage condition index[{}] not found on quest[{}] useStageIndex[{}]",
				gfuncs::GetFormNameAndId(akQuest), index, useStageIndex
			);
			// return v;
		}
		else {
			logger::trace("condition for akQuest[{}] found", gfuncs::GetFormNameAndId(akQuest));
		}
		
		v = GetVanillaConditionFormsCpp(akQuest, condition, type, keyIndex);
		return v;
	}
	
	//mutex locked = true
	RE::TESForm* GetQuestStageConditionListForm(RE::StaticFunctionTag*, RE::TESQuest* akQuest, int index, bool useStageIndex){
		logger::trace("");
		
		if (!gfuncs::IsFormValid(akQuest)){
			logger::warn("akQuest doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetQuestStageConditionForms(akQuest, index, useStageIndex);
		// gfuncs::FormIdExType type = useStageIndex ? gfuncs::FormIdExType::kQuestStageIndexIndex : gfuncs::FormIdExType::kQuestStageArrayIndex;
		gfuncs::FormIdExType type = gfuncs::FormIdExType::kQuestStageIndex; //always use the actual index as defined in the stage, that way we're not doubling created forms for the same condition.
		
		int keyIndex = index; 
		if (!useStageIndex){
			auto* questStage = GetQuestStageItemByArrayIndex(akQuest, index);
			if (questStage){
				keyIndex = questStage->index;
			}
			else {
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return nullptr;
			}
		}
		
		gfuncs::FormIdEx keyId = {akQuest->GetFormID(), (std::uint32_t)keyIndex, 0, type};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
			else {
				logger::warn("listForm for akQuest[{}] conditions not valid", gfuncs::GetFormNameAndId(akQuest));
			}
		}
		else{
			logger::warn("it for akQuest[{}] not found", gfuncs::GetFormNameAndId(akQuest));
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 
	
// Quest Objectives =====================================================================================================================================

	//mutex locked = false
	std::vector<RE::TESForm*> GetQuestObjectiveConditionForms(RE::TESQuest* akQuest, int objectiveIndex, bool useObjectiveIndex, int targetIndex) {
		std::vector<RE::TESForm*> v;
		
		if (!gfuncs::IsFormValid(akQuest)){
			logger::warn("akQuest doesn't exist");
			return v;
		}
		
		// gfuncs::FormIdExType type = useObjectiveIndex ? gfuncs::FormIdExType::kQuestObjectiveIndexIndex : gfuncs::FormIdExType::kQuestObjectiveArrayIndex;
		gfuncs::FormIdExType type = gfuncs::FormIdExType::kQuestObjectiveIndex; //always use the actual index as defined in the objective, that way we're not doubling created forms for the same condition.
		
		RE::TESCondition* condition = GetNthQuestObjectivesNthTargetConditions(akQuest, objectiveIndex, useObjectiveIndex, targetIndex);
		
		int keyIndex = objectiveIndex;
		
		if (!useObjectiveIndex){
			auto* objective = GetNthQuestObjective(akQuest, objectiveIndex, useObjectiveIndex);
			if (objective){
				keyIndex = objective->index;
			}
			else {
				return v;
			}
		}
		
		if (!condition){
			logger::warn("quest Objective condition index[{}] not found on quest[{}] useObjectiveIndex[{}]",
				gfuncs::GetFormNameAndId(akQuest), objectiveIndex, useObjectiveIndex
			);
			// return v;
		}
		else {
			logger::trace("condition for akQuest[{}] found", gfuncs::GetFormNameAndId(akQuest));
		}
		
		v = GetVanillaConditionFormsCpp(akQuest, condition, type, keyIndex, targetIndex);
		return v;
	}
	
	//mutex locked = true
	RE::TESForm* GetQuestObjectiveTargetConditionListForm(RE::StaticFunctionTag*, RE::TESQuest* akQuest, int objectiveIndex, bool useObjectiveIndex, int targetIndex){
		logger::trace("");
		
		if (!gfuncs::IsFormValid(akQuest)){
			logger::warn("akQuest doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetQuestObjectiveConditionForms(akQuest, objectiveIndex, useObjectiveIndex, targetIndex);
		// gfuncs::FormIdExType type = useObjectiveIndex ? gfuncs::FormIdExType::kQuestObjectiveIndexIndex : gfuncs::FormIdExType::kQuestObjectiveArrayIndex;
		gfuncs::FormIdExType type = gfuncs::FormIdExType::kQuestObjectiveIndex; //always use the actual index as defined in the objective, that way we're not doubling created forms for the same condition.
		
		int keyIndex = objectiveIndex;
		if (!useObjectiveIndex){
			auto* objective = GetNthQuestObjective(akQuest, objectiveIndex, useObjectiveIndex);
			if (objective){
				keyIndex = objective->index;
			}
			else {
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return nullptr;
			}
		}
		
		gfuncs::FormIdEx keyId = {akQuest->GetFormID(), (std::uint32_t)keyIndex, (std::uint32_t)targetIndex, type};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
			else {
				logger::warn("listForm for akQuest[{}] conditions not valid", gfuncs::GetFormNameAndId(akQuest));
			}
		}
		else{
			logger::warn("iterator for akQuest[{}] not found", gfuncs::GetFormNameAndId(akQuest));
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 

// Quest =====================================================================================================================================
	//mutex locked = false
	std::vector<RE::TESForm*> GetQuestConditionForms(RE::TESQuest* akQuest) {
		std::vector<RE::TESForm*> v;
		
		if (!gfuncs::IsFormValid(akQuest)){
			logger::warn("akQuest doesn't exist");
			return v;
		}
		
		RE::TESCondition* condition = GetDialogueConditionsForQuest(akQuest);
		if (!condition){
			logger::warn("condition for akQuest[{}] not found", gfuncs::GetFormNameAndId(akQuest));
		}
		else {
			logger::trace("condition for akQuest[{}] found", gfuncs::GetFormNameAndId(akQuest));
		}
		v = GetVanillaConditionFormsCpp(akQuest, condition, gfuncs::FormIdExType::kQuest);
		return v;
	}
	
	//mutex locked = true
	RE::TESForm* GetQuestConditionListForm(RE::StaticFunctionTag*, RE::TESQuest* akQuest){
		logger::trace("");
		
		if (!gfuncs::IsFormValid(akQuest)){
			logger::warn("akQuest doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetQuestConditionForms(akQuest);
		gfuncs::FormIdEx keyId = {akQuest->GetFormID(), 0, 0, gfuncs::FormIdExType::kQuest};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
			else {
				logger::warn("listForm for akQuest[{}] conditions not valid", gfuncs::GetFormNameAndId(akQuest));
			}
		}
		else{
			logger::warn("it for akQuest[{}] not found", gfuncs::GetFormNameAndId(akQuest));
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 

// StoryManagerQuest =====================================================================================================================================
	//mutex locked = false
	std::vector<RE::TESForm*> GetStoryManagerQuestConditionForms(RE::TESQuest* akQuest) {
		std::vector<RE::TESForm*> v;
		
		if (!gfuncs::IsFormValid(akQuest)){
			logger::warn("akQuest doesn't exist");
			return v;
		}
		
		RE::TESCondition* condition = GetStoryManagerConditionsForQuest(akQuest);
		if (!condition){
			logger::warn("condition for akQuest[{}] not found", gfuncs::GetFormNameAndId(akQuest));
		}
		else {
			logger::trace("condition for akQuest[{}] found", gfuncs::GetFormNameAndId(akQuest));
		}
		v = GetVanillaConditionFormsCpp(akQuest, condition, gfuncs::FormIdExType::kStoryManagerQuest);
		return v;
	}
	
	//mutex locked = false
	RE::TESForm* GetStoryManagerQuestConditionListForm(RE::TESQuest* akQuest){
		logger::trace("");
		
		// checked in GetStoryManagerConditionListForm
		// if (!gfuncs::IsFormValid(akQuest)){
		// 	logger::warn("akQuest doesn't exist");
		// 	return nullptr;
		// }
		
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetStoryManagerQuestConditionForms(akQuest);
		gfuncs::FormIdEx keyId = {akQuest->GetFormID(), 0, 0, gfuncs::FormIdExType::kStoryManagerQuest};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				return listForm;
			}
			else {
				logger::warn("listForm for akQuest[{}] conditions not valid", gfuncs::GetFormNameAndId(akQuest));
			}
		}
		else{
			logger::warn("it for akQuest[{}] not found", gfuncs::GetFormNameAndId(akQuest));
		}
		
		return nullptr;
	} 

// StoryManagerNodeBase =====================================================================================================================================
	//mutex locked = false
	std::vector<RE::TESForm*> GetStoryManagerNodeBaseConditionForms(RE::BGSStoryManagerNodeBase* nodeBase) {
		logger::trace("");
		std::vector<RE::TESForm*> v;
		
		if (!gfuncs::IsFormValid(nodeBase)){
			logger::warn("nodeBase doesn't exist");
			return v;
		}
		
		RE::TESCondition* condition = GetStoryManagerConditionsForNodeBase(nodeBase);
		if (!condition){
			logger::warn("condition for nodeBase[{}] not found", gfuncs::GetFormNameAndId(nodeBase));
		}
		else {
			logger::trace("condition for nodeBase[{}] found", gfuncs::GetFormNameAndId(nodeBase));
		}
		v = GetVanillaConditionFormsCpp(nodeBase, condition, gfuncs::FormIdExType::kStoryManagerNodeBase);
		return v;
	}
	
	//mutex locked = false
	RE::TESForm* GetStoryManagerNodeBaseConditionListForm(RE::BGSStoryManagerNodeBase* nodeBase){
		logger::trace("");
		
		//checked in GetStoryManagerConditionListForm
		// if (!gfuncs::IsFormValid(nodeBase)){
		// 	logger::warn("nodeBase doesn't exist");
		// 	return nullptr;
		// }
		
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetStoryManagerNodeBaseConditionForms(nodeBase);
		gfuncs::FormIdEx keyId = {nodeBase->GetFormID(), 0, 0, gfuncs::FormIdExType::kStoryManagerNodeBase};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				return listForm;
			}
			else {
				logger::warn("listForm for nodeBase[{}] conditions not valid", gfuncs::GetFormNameAndId(nodeBase));
			}
		}
		else{
			logger::warn("it for nodeBase[{}] not found", gfuncs::GetFormNameAndId(nodeBase));
		}
		
		return nullptr;
	} 
	
// StoryManager quest or nodeBase =====================================================================================================================================
	//mutex locked = true
	RE::TESForm* GetStoryManagerConditionListForm(RE::StaticFunctionTag*, RE::TESForm* akForm){
		logger::trace("");
		if (!gfuncs::IsFormValid(akForm)){
			return nullptr;
		}
		
		RE::TESForm* listForm = nullptr;
		bool validType = false;
		mutex.lock(); //[X]===============================================================[X]
		
		RE::BGSStoryManagerNodeBase* nodeBase = akForm->As<RE::BGSStoryManagerNodeBase>();
		if (gfuncs::IsFormValid(nodeBase)){
			validType = true;
			listForm = GetStoryManagerNodeBaseConditionListForm(nodeBase);
		}
		
		RE::TESQuest* quest = akForm->As<RE::TESQuest>();
		if (gfuncs::IsFormValid(quest)){
			validType = true;
			listForm = GetStoryManagerQuestConditionListForm(quest);
		}
		
		if (!validType){
			logger::warn("akForm[{}] is not a StoryManagerNodeBase or a TESQuest", gfuncs::GetFormNameAndId(akForm));
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return listForm;
	}
	
// TopicInfo =====================================================================================================================================
	//mutex locked = false
	std::vector<RE::TESForm*> GetTopicInfoConditionForms(RE::TESTopicInfo* topicInfo) {
		RE::TESCondition* condition = GetTopicInfoConditions(topicInfo);
		auto v = GetVanillaConditionFormsCpp(topicInfo, condition, gfuncs::FormIdExType::kTopicInfo);
		return v;
	}
	
	//mutex locked = true
	RE::TESForm* GetTopicInfoConditionListForm(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo){
		if (!gfuncs::IsFormValid(topicInfo)){
			logger::warn("topicInfo doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetTopicInfoConditionForms(topicInfo);
		gfuncs::FormIdEx keyId = {topicInfo->GetFormID(), 0, 0, gfuncs::FormIdExType::kTopicInfo};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 
	
// Scene =====================================================================================================================================
	//mutex locked = false
	std::vector<RE::TESForm*> GetSceneConditionForms(RE::BGSScene* Scene) {
		RE::TESCondition* condition = GetSceneConditions(Scene);
		auto v = GetVanillaConditionFormsCpp(Scene, condition, gfuncs::FormIdExType::kScene);
		return v;
	}
	
	//mutex locked = true
	RE::TESForm* GetSceneConditionListForm(RE::StaticFunctionTag*, RE::BGSScene* Scene){
		if (!gfuncs::IsFormValid(Scene)){
			logger::warn("Scene doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetSceneConditionForms(Scene);
		gfuncs::FormIdEx keyId = {Scene->GetFormID(), 0, 0, gfuncs::FormIdExType::kScene};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 
	
// Scene phase start =====================================================================================================================================
	//mutex locked = false
	std::vector<RE::TESForm*> GetNthScenePhaseStartConditionForms(RE::BGSScene* Scene, int nthIndex) {
		RE::TESCondition* condition = GetNthScenePhaseStartConditions(Scene, nthIndex);
		auto v = GetVanillaConditionFormsCpp(Scene, condition, gfuncs::FormIdExType::kScenePhaseStart, nthIndex);
		return v;
	}
	
	//mutex locked = true
	RE::TESForm* GetNthScenePhaseStartConditionListForm(RE::StaticFunctionTag*, RE::BGSScene* Scene, int nthIndex){
		if (!gfuncs::IsFormValid(Scene)){
			logger::warn("Scene doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetNthScenePhaseStartConditionForms(Scene, nthIndex);
		gfuncs::FormIdEx keyId = {Scene->GetFormID(), (std::uint32_t)nthIndex, 0, gfuncs::FormIdExType::kScenePhaseStart};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 
	
// Scene phase End =====================================================================================================================================
	//mutex locked = false
	std::vector<RE::TESForm*> GetNthScenePhaseEndConditionForms(RE::BGSScene* Scene, int nthIndex) {
		RE::TESCondition* condition = GetNthScenePhaseEndConditions(Scene, nthIndex);
		auto v = GetVanillaConditionFormsCpp(Scene, condition, gfuncs::FormIdExType::kScenePhaseEnd, nthIndex);
		return v;
	}
	
	//mutex locked = true 
	RE::TESForm* GetNthScenePhaseEndConditionListForm(RE::StaticFunctionTag*, RE::BGSScene* Scene, int nthIndex){
		if (!gfuncs::IsFormValid(Scene)){
			logger::warn("Scene doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetNthScenePhaseEndConditionForms(Scene, nthIndex);
		gfuncs::FormIdEx keyId = {Scene->GetFormID(), (std::uint32_t)nthIndex, 0, gfuncs::FormIdExType::kScenePhaseEnd};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 
	
// Message Menu Button =====================================================================================================================================
	//mutex locked = false
	std::vector<RE::TESForm*> GetNthMessageMenuButtonConditionForms(RE::BGSMessage* message, int nthIndex) {
		RE::TESCondition* condition = GetNthMessageMenuButtonConditions(message, nthIndex);
		auto v = GetVanillaConditionFormsCpp(message, condition, gfuncs::FormIdExType::kMessageMenuButton, nthIndex);
		return v;
	}
	
	//mutex locked = true 
	RE::TESForm* GetNthMessageMenuButtonConditionListForm(RE::StaticFunctionTag*, RE::BGSMessage* message, int nthIndex){
		if (!gfuncs::IsFormValid(message)){
			logger::warn("message doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetNthMessageMenuButtonConditionForms(message, nthIndex);
		gfuncs::FormIdEx keyId = {message->GetFormID(), (std::uint32_t)nthIndex, 0, gfuncs::FormIdExType::kMessageMenuButton};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 
	
// MusicType Track =====================================================================================================================================
	//mutex locked = false
	std::vector<RE::TESForm*> GetNthMusicTypeTrackConditionForms(RE::BGSMusicType* MusicType, int nthIndex) {
		RE::TESCondition* condition = GetNthMusicTypeTrackConditions(MusicType, nthIndex);
		auto v = GetVanillaConditionFormsCpp(MusicType, condition, gfuncs::FormIdExType::kMusicTypeTrack, nthIndex);
		return v;
	}
	
	//mutex locked = true 
	RE::TESForm* GetNthMusicTypeTrackConditionListForm(RE::StaticFunctionTag*, RE::BGSMusicType* MusicType, int nthIndex){
		if (!gfuncs::IsFormValid(MusicType)){
			logger::warn("MusicType doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetNthMusicTypeTrackConditionForms(MusicType, nthIndex);
		gfuncs::FormIdEx keyId = {MusicType->GetFormID(), (std::uint32_t)nthIndex, 0, gfuncs::FormIdExType::kMusicTypeTrack};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 

// Package =====================================================================================================================================
	//mutex locked = false
	std::vector<RE::TESForm*> GetPackageConditionForms(RE::TESPackage* akPackage) {
		std::vector<RE::TESForm*> v;
		
		if (!akPackage){
			logger::warn("akPackage doesn't exist");
			return v;
		}
		
		RE::TESCondition* condition = GetPackageConditions(akPackage);
		if (!condition){
			logger::warn("condition for akPackage[{}] not found", gfuncs::GetFormNameAndId(akPackage));
		}
		else {
			logger::trace("condition for akPackage[{}] found", gfuncs::GetFormNameAndId(akPackage));
		}
		v = GetVanillaConditionFormsCpp(akPackage, condition, gfuncs::FormIdExType::kPackage);
		return v;
	}

	//mutex locked = true
	RE::TESForm* GetPackageConditionListForm(RE::StaticFunctionTag*, RE::TESPackage* akPackage){
		logger::trace("");
		
		if (!gfuncs::IsFormValid(akPackage)){
			logger::warn("akPackage doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetPackageConditionForms(akPackage);
		gfuncs::FormIdEx keyId = {akPackage->GetFormID(), 0, 0, gfuncs::FormIdExType::kPackage};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
			else {
				logger::warn("listForm for akPackage[{}] conditions not valid", gfuncs::GetFormNameAndId(akPackage));
			}
		}
		else{
			logger::warn("it for akPackage[{}] not found", gfuncs::GetFormNameAndId(akPackage));
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 

// Idle =====================================================================================================================================
	//mutex locked = false
	std::vector<RE::TESForm*> GetIdleFormConditionForms(RE::TESIdleForm* akIdleForm) {
		std::vector<RE::TESForm*> v;
		
		if (!akIdleForm){
			logger::warn("akIdleForm doesn't exist");
			return v;
		}
		
		RE::TESCondition* condition = GetIdleFormConditions(akIdleForm);
		if (!condition){
			logger::warn("condition for akIdleForm[{}] not found", gfuncs::GetFormNameAndId(akIdleForm));
		}
		else {
			logger::trace("condition for akIdleForm[{}] found", gfuncs::GetFormNameAndId(akIdleForm));
		}
		v = GetVanillaConditionFormsCpp(akIdleForm, condition, gfuncs::FormIdExType::kIdleForm);
		return v;
	}

	//mutex locked = true
	RE::TESForm* GetIdleFormConditionListForm(RE::StaticFunctionTag*, RE::TESIdleForm* akIdleForm){
		logger::trace("");
		
		if (!gfuncs::IsFormValid(akIdleForm)){
			logger::warn("akIdleForm doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetIdleFormConditionForms(akIdleForm);
		gfuncs::FormIdEx keyId = {akIdleForm->GetFormID(), 0, 0, gfuncs::FormIdExType::kIdleForm};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
			else {
				logger::warn("listForm for akIdleForm[{}] conditions not valid", gfuncs::GetFormNameAndId(akIdleForm));
			}
		}
		else{
			logger::warn("it for akIdleForm[{}] not found", gfuncs::GetFormNameAndId(akIdleForm));
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 

// Faction =====================================================================================================================================
	//mutex locked = false
	std::vector<RE::TESForm*> GetFactionConditionForms(RE::TESFaction* akFaction) {
		std::vector<RE::TESForm*> v;
		
		if (!akFaction){
			logger::warn("akFaction doesn't exist");
			return v;
		}
		
		RE::TESCondition* condition = GetFactionConditions(akFaction);
		if (!condition){
			logger::warn("condition for akFaction[{}] not found", gfuncs::GetFormNameAndId(akFaction));
		}
		else {
			logger::trace("condition for akFaction[{}] found", gfuncs::GetFormNameAndId(akFaction));
		}
		v = GetVanillaConditionFormsCpp(akFaction, condition, gfuncs::FormIdExType::kFaction);
		return v;
	}

	//mutex locked = true
	RE::TESForm* GetFactionVendorConditionListForm(RE::StaticFunctionTag*, RE::TESFaction* akFaction){
		logger::trace("");
		
		if (!gfuncs::IsFormValid(akFaction)){
			logger::warn("akFaction doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetFactionConditionForms(akFaction);
		gfuncs::FormIdEx keyId = {akFaction->GetFormID(), 0, 0, gfuncs::FormIdExType::kFaction};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
			else {
				logger::warn("listForm for akFaction[{}] conditions not valid", gfuncs::GetFormNameAndId(akFaction));
			}
		}
		else{
			logger::warn("it for akFaction[{}] not found", gfuncs::GetFormNameAndId(akFaction));
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 
	
// Alias =====================================================================================================================================
	
	//mutex locked = false
	std::vector<RE::TESForm*> GetAliasConditionForms(RE::BGSBaseAlias* akAlias) {
		std::vector<RE::TESForm*> v;
		
		if (!akAlias){
			logger::warn("akAlias doesn't exist");
			return v;
		}
		
		RE::TESCondition* condition = GetAliasConditions(akAlias);
		v = GetVanillaConditionFormsCpp(gfuncs::GetAliasKey(akAlias), condition);
		return v;
	}
	
	//mutex locked = true
	RE::TESForm* GetAliasConditionListForm(RE::StaticFunctionTag*, RE::BGSBaseAlias* akAlias){
		if (!akAlias){
			logger::warn("akAlias doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetAliasConditionForms(akAlias);
		gfuncs::FormIdEx keyId = gfuncs::GetAliasKey(akAlias);
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 
	
// ConstructibleObject =====================================================================================================================================
	
	//mutex locked = false
	std::vector<RE::TESForm*> GetConstructibleObjectConditionForms(RE::BGSConstructibleObject* constructibleObject) {
		RE::TESCondition* condition = GetConstructibleObjectConditions(constructibleObject);
		auto v = GetVanillaConditionFormsCpp(constructibleObject, condition, gfuncs::FormIdExType::kConstructibleObj);
		return v;
	}
	
	//mutex locked = true
	RE::TESForm* GetConstructibleObjectConditionListForm(RE::StaticFunctionTag*, RE::BGSConstructibleObject* ConstructibleObject){
		if (!gfuncs::IsFormValid(ConstructibleObject)){
			logger::warn("perk doesn't exist");
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		//init condition forms if necessarry
		std::vector<RE::TESForm*> v = GetConstructibleObjectConditionForms(ConstructibleObject);
		gfuncs::FormIdEx keyId = {ConstructibleObject->GetFormID(), 0, 0, gfuncs::FormIdExType::kConstructibleObj};
		
		auto it = createdVanillaConditionIds.find(keyId);
		if (it != createdVanillaConditionIds.end()){
			RE::TESForm* listForm = RE::TESForm::LookupByID(it->second->conditionListFormID); 
			if (gfuncs::IsFormValid(listForm, false, false)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	} 
	
//ConditionList.psc member native functions ===========================================================================================================
	
	//mutex locked = true
	std::vector<RE::TESForm*> GetConditionFormsInList(RE::TESForm* self){ 
		logger::trace("");
		
		std::vector<RE::TESForm*> v;
		
		if (!gfuncs::IsFormValid(self)){
			return v;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		auto result = GetConditionDataPtrForForm(self);
		bool gotV = false;
		
		if (result){
			ConditionData& data = *result->get();
			
			if (data.conditionType == ConditionType::kVanilla){
				RE::TESForm* baseForm = RE::TESForm::LookupByID(data.formIdEx.formId); 
				if (gfuncs::IsFormValid(baseForm)){
					switch(data.formIdEx.type){
						case gfuncs::FormIdExType::kAlias:{
							RE::BGSBaseAlias* value = skyrim_cast<RE::BGSBaseAlias*>(baseForm);
							if (value){
								gotV = true;
								v = GetAliasConditionForms(value);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kMagicEffect:{
							RE::EffectSetting* value = skyrim_cast<RE::EffectSetting*>(baseForm);
							if (value){
								gotV = true;
								v = GetMagicEffectConditionForms(value);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kMagicItem:{
							RE::MagicItem* value = skyrim_cast<RE::MagicItem*>(baseForm);
							if (value){
								gotV = true;
								v = GetNthMagicEffectConditionForms(value, data.formIdEx.subId);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kConstructibleObj:{
							RE::BGSConstructibleObject* value = skyrim_cast<RE::BGSConstructibleObject*>(baseForm);
							if (value){
								gotV = true;
								v = GetConstructibleObjectConditionForms(value);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kPerk:{
							RE::BGSPerk* value = skyrim_cast<RE::BGSPerk*>(baseForm);
							if (value){
								gotV = true;
								v = GetPerkConditionForms(value);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kPerkEntryPointOwner:{
							RE::BGSPerk* value = skyrim_cast<RE::BGSPerk*>(baseForm);
							if (value){
								gotV = true;
								v = GetNthPerkEntryConditionForms(value, data.formIdEx.subId, true);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kPerkEntryPointTarget:{
							RE::BGSPerk* value = skyrim_cast<RE::BGSPerk*>(baseForm);
							if (value){
								gotV = true;
								v = GetNthPerkEntryConditionForms(value, data.formIdEx.subId, false);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kQuest:{
							RE::TESQuest* value = skyrim_cast<RE::TESQuest*>(baseForm);
							if (value){
								gotV = true;
								v = GetQuestConditionForms(value);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kQuestStageIndex:{
							RE::TESQuest* value = skyrim_cast<RE::TESQuest*>(baseForm);
							if (value){
								gotV = true;
								v = GetQuestStageConditionForms(value, data.formIdEx.subId, true);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kQuestObjectiveIndex:{
							RE::TESQuest* value = skyrim_cast<RE::TESQuest*>(baseForm);
							if (value){
								gotV = true;
								v = GetQuestObjectiveConditionForms(value, data.formIdEx.subId, true, data.formIdEx.subId2);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kStoryManagerQuest:{
							RE::TESQuest* value = skyrim_cast<RE::TESQuest*>(baseForm);
							if (value){
								gotV = true;
								v = GetStoryManagerQuestConditionForms(value);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kStoryManagerNodeBase:{
							RE::BGSStoryManagerNodeBase* value = skyrim_cast<RE::BGSStoryManagerNodeBase*>(baseForm);
							if (value){
								gotV = true;
								v = GetStoryManagerNodeBaseConditionForms(value);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kTopicInfo:{
							RE::TESTopicInfo* value = skyrim_cast<RE::TESTopicInfo*>(baseForm);
							if (value){
								gotV = true;
								v = GetTopicInfoConditionForms(value);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kScene:{
							RE::BGSScene* value = skyrim_cast<RE::BGSScene*>(baseForm);
							if (value){
								gotV = true;
								v = GetSceneConditionForms(value);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kScenePhaseStart:{
							RE::BGSScene* value = skyrim_cast<RE::BGSScene*>(baseForm);
							if (value){
								gotV = true;
								v = GetNthScenePhaseStartConditionForms(value, data.formIdEx.subId);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kScenePhaseEnd:{
							RE::BGSScene* value = skyrim_cast<RE::BGSScene*>(baseForm);
							if (value){
								gotV = true;
								v = GetNthScenePhaseEndConditionForms(value, data.formIdEx.subId);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kMessageMenuButton:{
							RE::BGSMessage* value = skyrim_cast<RE::BGSMessage*>(baseForm);
							if (value){
								gotV = true;
								v = GetNthMessageMenuButtonConditionForms(value, data.formIdEx.subId);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kMusicTypeTrack:{
							RE::BGSMusicType* value = skyrim_cast<RE::BGSMusicType*>(baseForm);
							if (value){
								gotV = true;
								v = GetNthMusicTypeTrackConditionForms(value, data.formIdEx.subId);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kPackage:{
							RE::TESPackage* value = skyrim_cast<RE::TESPackage*>(baseForm);
							if (value){
								gotV = true;
								v = GetPackageConditionForms(value);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kIdleForm:{
							RE::TESIdleForm* value = skyrim_cast<RE::TESIdleForm*>(baseForm);
							if (value){
								gotV = true;
								v = GetIdleFormConditionForms(value);
							}
							break;
						}
						
						case gfuncs::FormIdExType::kFaction:{
							RE::TESFaction* value = skyrim_cast<RE::TESFaction*>(baseForm);
							if (value){
								gotV = true;
								v = GetFactionConditionForms(value);
							}
							break;
						}
					}
				}
			}
			
			if (!gotV){
				RE::TESCondition* condition = data.condition;
				if (condition){
					v = GetConditionFormsCpp(*result);
				}
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return v;
	}
	
	int GetConditionListType(RE::TESForm* self){
		auto result = GetConditionDataPtrForForm(self);
		if (result){
			std::shared_ptr<ConditionData> data = *result;
			RE::TESCondition* condition = GetConditionForConditionData(*data);
			if (!condition){
				return 0;
			}
			
			return static_cast<int>(data->conditionType);
		}
		
		return 0;
	}
	
	//mutex locked = true
	bool EvaluateConditionList(RE::TESForm* self, RE::TESObjectREFR* target) {
		mutex.lock(); //[X]===============================================================[X]
		RE::TESCondition* condition = GetConditionForForm(self);
		RE::TESObjectREFR* targetRef = gfuncs::IsFormValid(target) ? target : nullptr;
		bool result = condition != nullptr ? condition->IsTrue(targetRef, nullptr) : false;
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return result;
	}
	
	//mutex locked = false
	int GetNumOfConditionsInList(RE::TESForm* self){ 
		std::vector<RE::TESForm*> v;
		v = GetConditionFormsInList(self);
		return v.size();
	}
	
	//mutex locked = true
	RE::TESForm* CloneList(RE::TESForm* self){
		if (!gfuncs::IsFormValid(self)){
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		
		auto it = conditionsFormsMap.find(self->GetFormID());
		if (it != conditionsFormsMap.end()){
			RE::TESCondition* condition = GetConditionForForm(self);
			if (!condition){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return nullptr;
			}
			
			auto& data = *it->second;
			RE::TESForm* newListForm = CreateConditionFormCpp("ConditionList");
			if (!newListForm){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return nullptr;
			}
			
			RE::FormID newListFormId = newListForm->GetFormID();
			
			auto newData = std::make_shared<ConditionData>();
			RE::TESCondition* newCondition = new RE::TESCondition;
			createdConditions.insert(newCondition);
			// condition->Copy(newCondition);
			CopyConditionData(condition, newCondition);
			newData->condition = newCondition; 
			newData->conditionType = ConditionType::kCreated;
			newData->conditionListFormID = newListForm->GetFormID();
			conditionsFormsMap[newListFormId] = newData;
			auto items = GetAllConditionItems(newCondition); 
			for (int i = 0; i < items.size(); i++){
				RE::TESForm* newConditionForm = CreateConditionFormCpp("Condition");
				if (!newConditionForm){
					continue;
				}
				RE::FormID id = newConditionForm->GetFormID();
				newData->linkedConditionItemFormIds.push_back(id);
				conditionsFormsMap[id] = newData;
			}
			
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return newListForm;
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	}
	
	//mutex locked = true
	RE::BGSBaseAlias* GetAssociatedAlias(RE::TESForm* self){
		if (!gfuncs::IsFormValid(self)){
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		auto result = GetConditionDataForForm(self);
		if (result){
			auto& data = result->get();
			if (data.conditionType == ConditionType::kVanilla){
				if (data.formIdEx.type == gfuncs::FormIdExType::kAlias){
					if (data.formIdEx.formId != 0){
						RE::BGSBaseAlias* alias = gfuncs::GetAliasFromKey(data.formIdEx);
						if (alias){
							mutex.unlock(); //[O]-------------------------------------------------------------[O]
							return alias;
						}
					}
				}
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	}
	
	//mutex locked = true
	RE::TESForm* GetAssociatedForm(RE::TESForm* self){ 
		if (!gfuncs::IsFormValid(self)){
			return nullptr;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		auto result = GetConditionDataForForm(self);
		if (result){
			auto& data = result->get();
			if (data.conditionType == ConditionType::kVanilla){
				if (data.formIdEx.type != gfuncs::FormIdExType::kAlias){
					if (data.formIdEx.formId != 0){
						RE::TESForm* form = RE::TESForm::LookupByID(data.formIdEx.formId);
						if (gfuncs::IsFormValid(form)){
							mutex.unlock(); //[O]-------------------------------------------------------------[O]
							return form;
						}
					}
				}
			}
		}
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	}
	
	//mutex locked = true
	int GetAssociatedIndex(RE::TESForm* self){
		if (!gfuncs::IsFormValid(self)){
			return 0;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		auto result = GetConditionDataForForm(self);
		if (result){
			auto& data = result->get();
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return data.formIdEx.subId;
		}
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return 0;
	}
	
	//mutex locked = true
	bool SetNthCondition(RE::TESForm* self, RE::TESForm* conditionForm, int nthIndex){
		mutex.lock(); //[X]===============================================================[X]
		RE::TESCondition* condition = GetConditionForForm(self);
		if (!condition){
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		RE::TESConditionItem* conditionItem = GetConditionItemForForm(conditionForm);
		if (!conditionItem){
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		auto items = GetAllConditionItems(condition); 
		if (nthIndex < 0 || nthIndex >= items.size()){
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		RE::TESConditionItem* nthItem = items[nthIndex];
		if (nthItem){
			CopyConditionItemData(conditionItem, nthItem);
			FlagConditionDataAltered(self);
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return true;
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return false;
	}
	
	//mutex locked = true
	RE::TESForm* RemoveNthConditionForm(RE::TESForm* self, int nthIndex, RE::TESForm* newList, int newListIndex, bool destroy){
		logger::trace("nthIndex[{}] newListIndex[{}] destroy[{}]",
			nthIndex, newListIndex, destroy
		);
		
		mutex.lock(); //[X]===============================================================[X]
		RE::FormID selfId = gfuncs::IsFormValid(self) ? self->GetFormID() : 0;
		
		RE::TESCondition* condition = GetConditionForForm(self);
		if (!condition){
			logger::error("conditionList for [{:08X}] not found", selfId);
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return nullptr;
		}
		
		auto result = GetConditionDataForForm(self);
		if (!result){
			logger::error("conditionData for [{:08X}] not found", selfId);
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return nullptr;
		}
		
		RE::TESForm* conditionListForm = nullptr; 
		
		//if newList is not a nullptr, papyrus coder is trying to add the removed condition to a new list. 
		//which is why I'm only using a nullptr check here instead of IsFormValid
		if (newList && !destroy){ 
			if (GetConditionDataForForm(newList)){
				conditionListForm = newList;
			}
			else { //passed in newList not valid
				RE::FormID newListId = gfuncs::IsFormValid(newList) ? newList->GetFormID() : 0;
				logger::error("newList[{:08X}] conditionData not found", newListId);
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return nullptr;
			}
		}
		
		bool createdNewList = false;
		
		if (!destroy && !conditionListForm){
			conditionListForm = CreateConditionFormCpp("ConditionList");
			if (!gfuncs::IsFormValid(conditionListForm)){
				logger::error("failed to create new ConditionList for [{:08X}] nth index[{}]", selfId, nthIndex);
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return nullptr;
			}
			else {
				createdNewList = true;
			}
		}
		
		auto& data = result->get();
		
		auto items = GetAllConditionItems(condition);
		if (nthIndex < 0 || nthIndex >= items.size()){
			logger::error("[{:08X}] nth index[{}] is out of range. Number of conditions = [{}]", 
				selfId, nthIndex, items.size()
			);
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return nullptr;
		} 
		
		RE::TESConditionItem* item = items[nthIndex];
		if (!item){
			logger::error("[{:08X}] condition for nth index[{}] not found. Number of conditions = [{}]", 
				selfId, nthIndex, items.size()
			);
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return nullptr;
		}
		
		RE::TESForm* conditionForm = nullptr;
		RE::FormID conditionFormId = 0;
		
		if (nthIndex >= 0 && nthIndex < data.linkedConditionItemFormIds.size()){
			conditionFormId = data.linkedConditionItemFormIds[nthIndex];
			data.linkedConditionItemFormIds.erase(data.linkedConditionItemFormIds.begin() + nthIndex);
			conditionForm = RE::TESForm::LookupByID(conditionFormId);
		}
		
		if (!conditionForm && !destroy) {
			conditionForm = CreateConditionFormCpp("Condition");
			if (!gfuncs::IsFormValid(conditionForm)){
				logger::error("Condition form for [{:08X}] nth index[{}] not found and failed to create a new one.", selfId, nthIndex);
				if (createdNewList){
					delete conditionListForm;
				}
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return nullptr;
			}
			else {
				conditionFormId = conditionForm->GetFormID();
			}
		}
		
		item->next = nullptr;
		items.erase(items.begin() + nthIndex);
		if (items.size() > 0){
			condition->head = items[0];
			
			for (int i = 0; i < items.size() - 1; i++){
				if (items[i]){
					items[i]->next = items[i + 1];
				}
			}
			
			auto* lastListItem = items.back();
			
			if (lastListItem){
				auto* lastItem = lastListItem->next;
				if (lastItem && lastItem != item){
					lastListItem->next = nullptr;
					DestroyConditionItem(lastItem);
				}
			}
		}
		else {
			condition->head = nullptr;
		}
		
		bool addedToNewList = false;
	
		if (!destroy && gfuncs::IsFormValid(conditionListForm)){
			auto gotNewListData = GetConditionDataPtrForForm(conditionListForm);
			if (gotNewListData){
				std::shared_ptr<ConditionData> newListData = *gotNewListData;
				RE::TESCondition* newListCondition = GetConditionForConditionData(*newListData);
				if (newListCondition){
					auto newListConditions = GetAllConditionItems(newListCondition);
					if (newListData){
						int iNewIndex = newListIndex;
						std::size_t newListSize = newListConditions.size();
						
						if (iNewIndex == -1){
							iNewIndex = newListSize;
						}
						
						if (iNewIndex >= newListSize){ //append
							if (newListSize == 0){
								newListCondition->head = item;
								addedToNewList = true;
							}
							else{
								if (newListConditions[newListSize - 1]){
									newListConditions[newListSize - 1]->next = item;
									addedToNewList = true;
								}
							}
						}
						//insert
						else if (iNewIndex >= 0 && iNewIndex < newListSize){
							if (iNewIndex == 0){
								newListCondition->head = item; 
							}
							
							newListConditions.insert(newListConditions.begin() + iNewIndex, item);
							
							if (newListConditions.size() > 0){
								for (int i = 0; i < newListConditions.size() - 1; i++){
									if (newListConditions[i]){
										newListConditions[i]->next = newListConditions[i + 1];
									}
								}
								
								auto* lastNewListListItem = newListConditions.back();
								if (lastNewListListItem){
									auto* lastNewListItem = lastNewListListItem->next;
									if (lastNewListItem && lastNewListItem != item){
										lastNewListListItem->next = nullptr;
										DestroyConditionItem(lastNewListItem);
									}
								}
								addedToNewList = true;
							}
						}
						
						if (addedToNewList){
							conditionsFormsMap[conditionFormId] = newListData;
							FlagConditionDataAltered(conditionsFormsMap[conditionFormId]);

							auto& ids = newListData->linkedConditionItemFormIds;
							std::size_t idsInsertAt = (iNewIndex >= 0 && static_cast<std::size_t>(iNewIndex) < ids.size()) ? static_cast<std::size_t>(iNewIndex) : ids.size();
								
							ids.insert(ids.begin() + idsInsertAt, conditionFormId);
						}
					}
				}
			}
		}
		
		if (destroy){
			DestroyConditionItem(item);
			
			conditionsFormsMap.erase(conditionFormId);
			conditionFormsToDelete.insert(conditionForm);
		}
		else if (!addedToNewList){
			RE::TESCondition* newCondition = new RE::TESCondition;
			createdConditions.insert(newCondition);
			newCondition->head = item;
			auto newData = std::make_shared<ConditionData>();
			newData->conditionListFormID = conditionListForm->GetFormID();
			newData->condition = newCondition;
			newData->linkedConditionItemFormIds.push_back(conditionFormId);
			newData->conditionType = ConditionType::kCreated;
			conditionsFormsMap[conditionFormId] = newData;
			conditionsFormsMap[conditionListForm->GetFormID()] = newData;
		}
		
		FlagConditionDataAltered(data);   // the source list lost an item on every path
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return conditionForm;
	}
	
	//mutex locked = true
	RE::TESForm* InsertConditionForm(RE::TESForm* self, RE::TESForm* conditionForm, int index){
		mutex.lock(); //[X]===============================================================[X]
		RE::TESCondition* selfCondition = GetConditionForForm(self);
		if (!selfCondition){
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return nullptr;
		}
		
		auto result = GetConditionDataForForm(self);
		if (!result){
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return nullptr;
		}
		
		auto& data = result->get();
		
		RE::TESConditionItem* item = GetConditionItemForForm(conditionForm); 
		if (!item){
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return nullptr;
		}
		
		RE::TESForm* newConditionForm = CreateConditionFormCpp("Condition");
		RE::FormID newConditionFormId = newConditionForm->GetFormID(); 
		if (!newConditionForm){
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return nullptr;
		}
		
		RE::TESConditionItem* newItem = new RE::TESConditionItem;
		if (!newItem){
			delete newConditionForm;
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return nullptr;
		}
		
		newItem->next = nullptr;
		
		createdConditionItems.insert(newItem);
		
		// item->Copy(newItem);
		CopyConditionItemData(item, newItem);
		
		auto items = GetAllConditionItems(selfCondition);
		if (index < 0 || index >= items.size()){
			if (items.size() > 0){
				RE::TESConditionItem* lastItem = items[items.size() - 1];
				if (lastItem){
					lastItem->next = newItem;
					data.linkedConditionItemFormIds.push_back(newConditionFormId);
				}
			}
		}
		else {
			items.insert(items.begin() + index, newItem);
			data.linkedConditionItemFormIds.insert(data.linkedConditionItemFormIds.begin() + index, newConditionFormId);
			for (int i = 0; i < items.size() - 1; i++){
				items[i]->next = items[i + 1];
			}
		}
		
		auto listIt = conditionsFormsMap.find(data.conditionListFormID);
		if (listIt != conditionsFormsMap.end() && listIt->second){
			conditionsFormsMap[newConditionFormId] = listIt->second;
			FlagConditionDataAltered(listIt->second);
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return newConditionForm;
	}
	
	//mutex locked = true
	std::shared_ptr<ConditionChangedEvent> GetNativeConditionChangedEvent(RE::TESCondition* condition, RE::ObjectRefHandle targetHandle){
		if (!condition){
			return nullptr;
		}
		
        mutex.lock(); //[X]===============================================================[X]
		
		for (std::shared_ptr<ConditionChangedEvent> event : conditionEvents){
			if (event){
				if (!event->deleted){
					if (event->condition == condition && event->target == targetHandle){
						mutex.unlock(); //[O]-------------------------------------------------------------[O]
						return event;
					}
				}
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	}
	
	//mutex locked = true
	bool RegisterForNativeConditionChanged(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::TESForm* self, RE::TESObjectREFR* akTarget) {
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		RE::ObjectRefHandle handle; 
		if (gfuncs::IsFormValid(akTarget)){
			handle = akTarget->GetHandle();
		}
		
        mutex.lock(); //[X]===============================================================[X]
		RE::TESCondition* condition = GetConditionForForm(self);
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		
		if (!condition){
			return false;
		}
		
        std::shared_ptr<ConditionChangedEvent> conditionEvent = GetNativeConditionChangedEvent(condition, handle);
        bool madeNewEvent = false; 
		
		mutex.lock(); //[X]===============================================================[X]
		
        if (!conditionEvent) {
			madeNewEvent = true;
			std::string conditionId = FormIdToNativeConditionId(self->GetFormID());
			conditionEvent = std::make_shared<ConditionChangedEvent>(conditionId, condition, akTarget, true);
			conditionEvents.push_back(conditionEvent);
		}
		
		RE::VMHandle callingHandle = gfuncs::GetCallingScriptHandle(vm, stackID, "ConditionList");
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		conditionEvent->RegisterForConditionChanged(callingHandle);
		
		if (madeNewEvent){
			conditionEvent->ListenForConditionEventChangeStart(); //mutex locked
		}
            
        return true; 
	}
	
	//mutex locked = true
	bool UnregisterForNativeConditionChanged(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::TESForm* self, RE::TESObjectREFR* akTarget) {
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		std::string conditionId = FormIdToNativeConditionId(self->GetFormID());
		
		RE::ObjectRefHandle handle; 
		if (gfuncs::IsFormValid(akTarget)){
			handle = akTarget->GetHandle();
		}
		
        mutex.lock(); //[X]===============================================================[X]
		RE::TESCondition* condition = GetConditionForForm(self);
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		
		if (!condition){
			return false;
		}
		
        std::shared_ptr<ConditionChangedEvent> conditionEvent = GetNativeConditionChangedEvent(condition, handle);
		
        if (!conditionEvent) {
			return false;
		}
		
		RE::VMHandle callingHandle = gfuncs::GetCallingScriptHandle(vm, stackID, "ConditionList");
		conditionEvent->UnregisterForConditionChanged(callingHandle);
            
        return true; 
	}
	
	bool DestroyConditionList(RE::TESForm* self){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		
		bool destroyed = false;
		
		auto result = GetConditionDataPtrForForm(self);
		if (result){
			auto data = *result;
			if (data){
				RE::TESCondition* condition = GetConditionForConditionData(*data);
				if (condition){
					auto items = GetAllConditionItems(condition);
					condition->head = nullptr;
					for (int i = 0; i < items.size(); i++){
						DestroyConditionItem(items[i]);
					}
				}
				
				DeleteTESCondition(condition);
				
				for (RE::FormID& formId : data->linkedConditionItemFormIds){
					RE::TESForm* linkedForm = RE::TESForm::LookupByID(formId);
					conditionFormsToDelete.insert(linkedForm);
					auto it = conditionsFormsMap.find(formId);
					if (it != conditionsFormsMap.end()){
						conditionsFormsMap.erase(it);
					}
				}
				
				FlagConditionDataAltered(data);
				destroyed = true;
			}
		}
		
		auto itt = conditionsFormsMap.find(self->GetFormID());
		if (itt != conditionsFormsMap.end()){
			conditionsFormsMap.erase(itt);
		}
		
		conditionFormsToDelete.insert(self);
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return destroyed;
	}
	
//Condition.psc member native functions ===========================================================================================================
	
	//mutex locked = true
	bool IsNativeConditionValid(RE::TESForm* self)	{
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		
		// int numOfParams = GetNumberOfConditionItemParams(item);
		// if (item){
		// 	if (item->data.functionData.)
		// }
		// logger::debug("number of params = [{}]", numOfParams);
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return (item != nullptr);
	}

	//mutex locked = true
	RE::TESForm* GetConditionListForm(RE::TESForm* self){
		mutex.lock(); //[X]===============================================================[X]
		auto result = GetConditionDataForForm(self);
		if (result){
			auto& data = result->get();
			RE::TESForm* listForm = RE::TESForm::LookupByID(data.conditionListFormID);
			if (gfuncs::IsFormValid(listForm)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return listForm;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return nullptr;
	}
	
	//mutex locked = true
	//both GetConditionListForm and GetConditionFormsInList are locked.
	int GetConditionListIndex(RE::TESForm* self){
		int index = -1;
		
		RE::TESForm* list = GetConditionListForm(self);
		if (!gfuncs::IsFormValid(list)){
			return index;
		}
			
		auto forms = GetConditionFormsInList(list);
		auto it = std::find(forms.begin(), forms.end(), self);

		if (it != forms.end()) {
			index = int(std::distance(forms.begin(), it));
		}
		
		return index;
	}
	
	//only evaluate the single conditionItem, not the whole list
	//mutex locked = true
	bool EvaluateNativeCondition(RE::TESForm* self, RE::TESObjectREFR* target) {
		mutex.lock(); //[X]===============================================================[X]

		RE::TESConditionItem* conditionItem = GetConditionItemForForm(self);
		if (!conditionItem){
			return false;   // nothing to evaluate
		}

		if (!CanEvaluateConditionItem(conditionItem)) {
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		RE::TESObjectREFR* targetRef = gfuncs::IsFormValid(target) ? target : nullptr;

		RE::TESConditionItem* item = new RE::TESConditionItem;
		item->next = nullptr; 
		
		CopyConditionItemData(conditionItem, item);
		item->next = nullptr;

		RE::TESCondition* condition = new RE::TESCondition;
		condition->head = item;
		bool result = condition->IsTrue(targetRef, nullptr);
		condition->head = nullptr;

		// CopyConditionParam allocates a fresh BSFixedString for a kString param --
		// without this, every Evaluate() on a string condition leaks one.
		DestroyConditionParameter(item->data.functionData.params[0]);
		DestroyConditionParameter(item->data.functionData.params[1]);

		delete item;
		delete condition;

		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return result;
	}
	
	//mutex locked = true
	bool CopyNativeCondition(RE::TESForm* self, RE::TESForm* target) {
        mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* selfConditionItem = GetConditionItemForForm(self);
		RE::TESConditionItem* targetConditionItem = GetConditionItemForForm(target);
		bool result = CopyConditionItemData(selfConditionItem, targetConditionItem);
		if (result){
			FlagConditionDataAltered(target);
		}
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		
		return result;
	}
	
	//mutex locked = true
	bool MoveNativeConditionToList(RE::TESForm* self, RE::TESForm* newList, int nthIndex){
		logger::trace("nthIndex[{}]", nthIndex);
		
		if (!gfuncs::IsFormValid(self)){
			logger::error("condition form is none");
			
			return false;
		}
		
		if (!gfuncs::IsFormValid(newList)){
			logger::error("newList form is none");
			return false;
		}
		
		int currentIndex = GetConditionListIndex(self);
		if (currentIndex == -1){
			logger::error("currentIndex for condition not found [{}]", currentIndex);
			return false;
		}
		
        mutex.lock(); //[X]===============================================================[X]
		
		//ConditionData not found for newList, abort.
		if (!GetConditionDataPtrForForm(newList)){
			logger::error("newList condition data not found");
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		auto result = GetConditionDataPtrForForm(self);
		if (result){
			auto data = *result;
			if (data){
				RE::TESForm* listForm = RE::TESForm::LookupByID(data->conditionListFormID);
				if (gfuncs::IsFormValid(listForm)){
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return RemoveNthConditionForm(listForm, currentIndex, newList, nthIndex, false);
				}
			}
		}
		else {
			logger::error("self condition data not found");
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return false;
	}
	
	//mutex locked = true
	bool DestroyNativeCondition(RE::TESForm* self){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		int index = GetConditionListIndex(self);
		
        mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* conditionItem = GetConditionItemForForm(self);
		if (!conditionItem){
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		bool destroyed = false;
		bool removedFromList = false;
		auto result = GetConditionDataPtrForForm(self);
		if (result){
			auto dataPtr = *result;
			if (dataPtr){
				RE::TESForm* listForm = RE::TESForm::LookupByID(dataPtr->conditionListFormID);
				if (gfuncs::IsFormValid(listForm)){
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					removedFromList = RemoveNthConditionForm(listForm, index, nullptr, 0, true);
        			mutex.lock(); //[X]===============================================================[X]
				}
			}
		}
		
		
		if (!removedFromList){
			RE::TESCondition* condition = GetConditionForForm(self);
			if (condition){
				auto items = GetAllConditionItems(condition);
				auto it = std::find(items.begin(), items.end(), conditionItem);
				if (it != items.end()){
					items.erase(it);
					
					if (items.size() > 0){
						condition->head = items[0];
						for (int i = 0; i < items.size() - 1; i++){
							items[i]->next = items[i + 1];
						}
					}
					else {
						condition->head = nullptr;
					}
				}
				
				DestroyConditionItem(conditionItem);
				destroyed = true;
			}
			
		}
		else {
			destroyed = true;
		}
		
		auto itt = conditionsFormsMap.find(self->GetFormID());
		if (itt != conditionsFormsMap.end()){
			if (destroyed){
				FlagConditionDataAltered(itt->second);
			}
			conditionsFormsMap.erase(itt);
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return destroyed;
	}
	
// Setters ==============================================================================================================================================

	//mutex locked = true
	bool SetNativeComparison(RE::TESForm* self, int comparison) {
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]

        if (comparison > 5 || comparison < 0) {
            logger::warn("conditionId[{:08X}] comparison[{}] isn't valid. Must be between 0 and 5", formId, comparison);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        } 
		
        RE::TESConditionItem* item = GetConditionItemForForm(self);
        if (!item) {
            logger::warn("something went wrong. conditionId[{}] condition->head is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        } 

        item->data.flags.opCode = static_cast<RE::CONDITION_ITEM_DATA::OpCode>(comparison);
		FlagConditionDataAltered(self);
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return true;
    }

	//mutex locked = true
    bool SetNativeValue(RE::TESForm* self, float value) {
        if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]

        RE::TESConditionItem* item = GetConditionItemForForm(self);
    
        if (!item) {
            logger::warn("something went wrong. conditionId[{:08X}] condition->item is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }

        item->data.comparisonValue.f = value;
		item->data.flags.global = false;
		FlagConditionDataAltered(self);
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return true;
    }

	//mutex locked = true
	bool SetNativeValueGlobalVariable(RE::TESForm* self, RE::TESGlobal* globalVar) {
        if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]

        RE::TESConditionItem* item = GetConditionItemForForm(self);
    
        if (!item) {
            logger::warn("something went wrong. conditionId[{:08X}] condition->item is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }

        item->data.comparisonValue.g = gfuncs::IsFormValid(globalVar) ? globalVar : nullptr;
		item->data.flags.global = true;
		FlagConditionDataAltered(self);
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return true;
    }
	
	//mutex locked = true
	bool SetNativeConditionFunction(RE::TESForm* self, int functionId) {
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]

        RE::TESConditionItem* item = GetConditionItemForForm(self);
    
        if (!item) {
            logger::warn("something went wrong. conditionId[{:08X}] condition->item is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }
		
		item->data.functionData.function = static_cast<RE::FUNCTION_DATA::FunctionID>(functionId);
		bool result = (item != nullptr ? static_cast<int>(item->data.functionData.function.get()) : 0) == functionId;
		FlagConditionDataAltered(self);
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return result;
	}
	
	//mutex locked = true
	bool SetNativeAndOr(RE::TESForm* self, bool isOR){ 
		if (!gfuncs::IsFormValid(self)){
			return false;
		} 
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (!item) {
            logger::warn("something went wrong. conditionId[{:08X}] condition->item is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }
		
		item->data.flags.isOR = isOR;
		FlagConditionDataAltered(self);
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return true;
		
	}
	
	//mutex locked = true
	bool SetNativeUsesAliases(RE::TESForm* self, bool usesAliases){ 
        if (!gfuncs::IsFormValid(self)){
			return false;
		} 
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (!item) {
            logger::warn("something went wrong. conditionId[{:08X}] condition->item is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }
	
		item->data.flags.usesAliases = usesAliases;
		FlagConditionDataAltered(self);
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return true;
	}
	
	//mutex locked = true
	bool SetNativeUsesPackData(RE::TESForm* self, bool usesPackData){ 
		if (!gfuncs::IsFormValid(self)){
			return false;
		} 
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (!item) {
            logger::warn("something went wrong. conditionId[{:08X}] condition->item is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }
		
		item->data.flags.usePackData = usesPackData;
		FlagConditionDataAltered(self);
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return true;
	}
	
	//mutex locked = true
	bool SetNativeSwapTarget(RE::TESForm* self, bool swapTarget){ 
		if (!gfuncs::IsFormValid(self)){
			return false;
		} 
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (!item) {
            logger::warn("something went wrong. conditionId[{:08X}] condition->item is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }
		
		item->data.flags.swapTarget = swapTarget;
		FlagConditionDataAltered(self);
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return true;
	}
	
	// kSelf = 0, (subject)
	// kTarget = 1,
	// kRef = 2,
	// kCombatTarget = 3,
	// kLinkedRef = 4,
	// kQuestAlias = 5,
	// kPackData = 6,
	// kEventData = 7,
	// kCommandTarget = 8
	// mutex locked = true
	bool SetNativeRunOnFlag(RE::TESForm* self, int flag){ 
		if (!gfuncs::IsFormValid(self)){
			return false;
		} 
		
		if (flag < 0 || flag > 8){
			return false;
		}
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (!item) {
            logger::warn("something went wrong. conditionId[{:08X}] condition->item is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }
		
		RE::CONDITIONITEMOBJECT obj = static_cast<RE::CONDITIONITEMOBJECT>(flag);
		item->data.object = obj;
		bool result = (item->data.object.get() == obj);
		FlagConditionDataAltered(self);
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return result;
	}

	// mutex locked = true
	bool SetNativeRunOnRef(RE::TESForm* self, RE::TESObjectREFR* ref){
		if (!gfuncs::IsFormValid(self)){
			return false;
		} 
		
		RE::ObjectRefHandle handle;
		if (gfuncs::IsFormValid(ref)){
			handle = ref->GetHandle();
		}
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (!item) {
            logger::warn("something went wrong. conditionId[{:08X}] condition->item is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }
		
		item->data.runOnRef = handle;
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return true;
	}
	
// kGetEventData helper functions =================================================================================================================================================================================================
//
// The Creation Kit shows three fields for kGetEventData. The engine packs the first two into
// params[0] and puts the third in params[1]:
//
//   params[0] bits  0-15 : event function, as the ENGINE value   <- papyrus paramIndex 0
//   params[0] bits 16-23 : event member code, type letter (ASCII) }
//   params[0] bits 24-31 : event member code, slot digit (ASCII)  } <- papyrus paramIndex 1
//   params[1]            : event data (a form)                   <- papyrus paramIndex 2
//
// The member half is ASCII, not an enum:  high16 == (digit << 8) | letter
//
// The papyrus-facing indices for BOTH halves follow the order the Creation Kit and xEdit
// display them in, which is alphabetical. For the member that happens to be arbitrary either
// way, but for the function it does NOT match the engine's own numbering -- GetItemValue is
// display 1 / engine 4, and IsInList is display 4 / engine 1. Hence two lookup tables.
//
// Measured, all eight round-trip:
//
//   display (fn,mem)  condition                      params[0]     engine fn   code
//   (0,0)             GetIsID:(New)Location          0x324C0000    0           "L2"
//   (1,1)             GetItemValue:(Old)Location     0x314C0004    4           "L1"
//   (2,2)             GetValue:CreatedObject         0x314F0002    2           "O1"
//   (3,3)             HasKeyword:Form                0x31460003    3           "F1"
//   (4,4)             IsInList:Keyword               0x314B0001    1           "K1"
//   (4,5)             IsInList:None                  0x00000001    1           ""
//   (4,6)             IsInList:Value1                0x31560001    1           "V1"
//   (4,7)             IsInList:Value2                0x32560001    1           "V2"
// ============================================================================================

	// --- event function -----------------------------------------------------------------------
	// Display order (Creation Kit / xEdit). NOT the engine's values -- see eventFunctionValues.
	enum class EventFunction : int {
		kGetIsID      = 0,
		kGetItemValue = 1,
		kGetValue     = 2,
		kHasKeyword   = 3,
		kIsInList     = 4,
		kTotal
	};

	// display index -> value stored in params[0] bits 0-15
	inline constexpr std::array<int, static_cast<std::size_t>(EventFunction::kTotal)>
	eventFunctionValues = {
		0,   // 0 kGetIsID
		4,   // 1 kGetItemValue
		2,   // 2 kGetValue
		3,   // 3 kHasKeyword
		1,   // 4 kIsInList
	};

	// --- event member -------------------------------------------------------------------------
	// Display order (Creation Kit / xEdit), mapped to the two character code the engine stores.
	enum class EventMember : int {
		kNewLocation   = 0,
		kOldLocation   = 1,
		kCreatedObject = 2,
		kForm          = 3,
		kKeyword       = 4,
		kNone          = 5,
		kValue1        = 6,
		kValue2        = 7,
		kTotal
	};

	inline constexpr std::array<std::string_view, static_cast<std::size_t>(EventMember::kTotal)>
	eventMemberCodes = {
		"L2",   // 0 kNewLocation
		"L1",   // 1 kOldLocation
		"O1",   // 2 kCreatedObject   -- 'O' for Object
		"F1",   // 3 kForm
		"K1",   // 4 kKeyword
		"",     // 5 kNone            -- packs as 0x0000
		"V1",   // 6 kValue1
		"V2",   // 7 kValue2
	};

// --- translation ------------------------------------------------------------------------------

	//mutex locked = false
	bool IsGetEventDataCondition(RE::TESConditionItem* item){
		return item != nullptr ? item->data.functionData.function.get() == RE::FUNCTION_DATA::FunctionID::kGetEventData : false;
	}

	//mutex locked = false
	//display index -> engine value. -1 if the index isn't in the table.
	int EventFunctionIndexToValue(int index){
		if (index < 0 || index >= static_cast<int>(eventFunctionValues.size())){
			return -1;
		}
		return eventFunctionValues[static_cast<std::size_t>(index)];
	}

	//mutex locked = false
	//engine value -> display index. -1 if the value isn't in the table.
	int EventFunctionValueToIndex(int value){
		for (std::size_t i = 0; i < eventFunctionValues.size(); i++){
			if (eventFunctionValues[i] == value){
				return static_cast<int>(i);
			}
		}
		logger::warn("unknown event function value[{}]", value);
		return -1;
	}

	//mutex locked = false
	//display index -> the 16 bit code. 0 for kNone, and 0 for anything out of range, so callers
	//that care about the difference must range check the index themselves.
	std::uint16_t EventMemberIndexToCode(int index){
		if (index < 0 || index >= static_cast<int>(eventMemberCodes.size())){
			return 0;
		}

		const std::string_view code = eventMemberCodes[static_cast<std::size_t>(index)];

		std::uint16_t packed = 0;
		if (code.size() > 0){
			packed |= static_cast<std::uint16_t>(static_cast<unsigned char>(code[0]));
		}
		if (code.size() > 1){
			packed |= static_cast<std::uint16_t>(static_cast<unsigned char>(code[1])) << 8;
		}
		return packed;
	}

	//mutex locked = false
	//the 16 bit code -> display index. -1 if the code isn't in the table.
	int EventMemberCodeToIndex(std::uint16_t code){
		const char letter = static_cast<char>(code & 0xFF);
		const char digit  = static_cast<char>((code >> 8) & 0xFF);

		const char buffer[2] = { letter, digit };
		std::size_t length = 0;
		if (letter){
			length = digit ? 2 : 1;
		}
		const std::string_view view(buffer, length);

		for (std::size_t i = 0; i < eventMemberCodes.size(); i++){
			if (eventMemberCodes[i] == view){
				return static_cast<int>(i);
			}
		}

		logger::warn("unknown event member code[{:04X}] chars[{}{}]",
			code,
			letter ? letter : ' ',
			digit  ? digit  : ' ');
		return -1;
	}

	//mutex locked = false
	void* EventDataConditionRawToParam(RE::FormID formId, RE::TESConditionItem* item, int param, int paramIndex, bool& success){
		success = false;
		void* returnValue = nullptr; 
		if (!item){
			return returnValue;
		}
		
		auto packed = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(item->data.functionData.params[0]));

		bool ok = false;

		switch (paramIndex){
			case 0: {   // event function -- low 16 bits
				const int engineValue = EventFunctionIndexToValue(param);
				if (engineValue < 0){
					logger::warn("conditionId[{:08X}] event function[{}] out of range. Must be 0 to {}.",
						formId, param, static_cast<int>(EventFunction::kTotal) - 1);
					break;
				}
				packed = (packed & 0xFFFF0000u) | (static_cast<std::uint32_t>(engineValue) & 0xFFFFu);
				ok = true;
				break;
			}

			case 1: {   // event member -- high 16 bits, as a two character code
				if (param < 0 || param >= static_cast<int>(EventMember::kTotal)){
					logger::warn("conditionId[{:08X}] event member[{}] out of range. Must be 0 to {}.",
						formId, param, static_cast<int>(EventMember::kTotal) - 1);
					break;
				}
				const std::uint16_t code = EventMemberIndexToCode(param);
				packed = (packed & 0x0000FFFFu) | (static_cast<std::uint32_t>(code) << 16);
				ok = true;
				break;
			}

			case 2:
				logger::warn("conditionId[{:08X}] paramIndex[2] on a kGetEventData condition is the "
					"event data form. Use SetParameterForm(akForm, 2) instead.", formId);
				break;

			default:
				logger::warn("conditionId[{:08X}] paramIndex[{}] not valid for a kGetEventData condition. "
					"Must be 0, 1 or 2.", formId, paramIndex);
				break;
		}

		if (ok){
			// raw params are not heap allocated and never tracked in paramDatas, so this is a
			// plain overwrite -- no DestroyConditionParameter, nothing to record.
			returnValue = RawToConditionParam(static_cast<int>(packed));
			success = true;
			logger::trace("conditionId[{:08X}] kGetEventData params[0] = 0x{:08X}", formId, packed);
		}

		return returnValue;
	}
	
	//mutex unlocked
	int EventDataConditionParamToRaw(RE::FormID formId, RE::TESConditionItem* item, int paramIndex) {
		int value = -1;
		
		const auto packed = static_cast<std::uint32_t>(
			reinterpret_cast<std::uintptr_t>(item->data.functionData.params[0]));

		switch (paramIndex){
			case 0:   // event function, returned as a display index
				value = EventFunctionValueToIndex(static_cast<int>(packed & 0xFFFF));
				break;

			case 1:   // event member, returned as a display index
				value = EventMemberCodeToIndex(static_cast<std::uint16_t>(packed >> 16));
				break;

			case 2:
				logger::warn("paramIndex[2] on a kGetEventData condition is the event data form. "
					"Use GetParameterForm(2) instead.");
				break;

			default:
				logger::warn("paramIndex[{}] not valid for a kGetEventData condition. Must be 0, 1 or 2.",
					paramIndex);
				break;
		}

		return value;
	}
	
// =================================================================================================================================================================================================
	
	//mutex locked = true
    bool SetNativeParameter(RE::TESForm* akForm, void* param, int paramIndex, ParamType type, ParamData& paramData) {
		mutex.lock(); //[X]===============================================================[X]
    
		//set paramData first to be used in DestroyConditionParameter for early returns.
		if (type == ParamType::kString || type == ParamType::kObjectReference){
			paramData.data = param;
			paramData.paramType = type;
			paramDatas[param] = paramData;
		}
		
		if (!gfuncs::IsFormValid(akForm)){
			DestroyConditionParameter(param);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
        
        RE::TESConditionItem* item = GetConditionItemForForm(akForm);
		
        if (!item) {
            logger::error("something went wrong. conditionId[{:08X}] condition->item is nullptr", akForm->GetFormID());
			DestroyConditionParameter(param);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }

		RE::FormID formId = akForm->GetFormID();
		
		if (paramIndex < 0 || paramIndex > 1) {
            logger::error("conditionId[{:08X}] paramIndex[{}] not valid. Must be between 0 and 2", formId, paramIndex);
			DestroyConditionParameter(param);
        	mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        } 
		
		DestroyConditionParameter(item->data.functionData.params[paramIndex]);
		
		if (!item){ //item destroyed
			DestroyConditionParameter(param);
        	mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		logger::trace("conditionId[{:08X}] raw param[{}] paramIndex[{}]", formId, param, paramIndex);
		
        item->data.functionData.params[paramIndex] = param;

		FlagConditionDataAltered(akForm);
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return true;
    }

	//mutex locked = false
	// no condition param bool types
    // bool SetNativeParameterBool(RE::TESForm* self, bool param, int paramIndex) {
	// 	void* slot = BoolToConditionParam(param);
	// 	ParamData paramData;
	// 	return SetNativeParameter(self, slot, paramIndex, ParamType::kBool, paramData);
	// }

	// //mutex locked = false
	// int and raw stores the same, so only using the raw function
    // bool SetNativeParameterInt(RE::TESForm* self, std::int32_t param, int paramIndex) {
	// 	if (!ParameterBaseTypeMatches(ParamType::kRaw, self, paramIndex, __func__)){
	// 		return false;
	// 	}
		
	// 	void* slot = IntToConditionParam(param);
	// 	ParamData paramData;
	// 	return SetNativeParameter(self, slot, paramIndex, ParamType::kInt, paramData);
	// }
	
	//mutex locked = true
	bool SetNativeParameterInt(RE::TESForm* self, int param, int paramIndex) {
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		
		RE::FormID formId = self->GetFormID();
		
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (!item){
			logger::error("conditionId[{:08X}] condition item is nullptr", formId);
        	mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		ParamData paramData; 
		
		if (IsGetEventDataCondition(item)){
			
			bool success = false;
			void* packed = EventDataConditionRawToParam(formId, item, param, paramIndex, success);
			if (!success){
        		mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return false;
			}
			else {
				//for kGetEventData, param 1 and 2 are combined into params[0]
				if (paramIndex == 1 || paramIndex == 2){
					paramIndex--;
				}
			
				if (!ParameterBaseTypeMatches(ParamType::kInt, item, paramIndex, formId, __func__)){
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}
				
        		mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return SetNativeParameter(self, packed, paramIndex, ParamType::kInt, paramData);   // not heap-allocated
			}
		}
		else {
			if (!ParameterBaseTypeMatches(ParamType::kInt, item, paramIndex, formId, __func__)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return false;
			}
				
			void* packed = RawToConditionParam(param);
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return SetNativeParameter(self, packed, paramIndex, ParamType::kInt, paramData);   // not heap-allocated
		}
	}
	
	//mutex locked = false
    bool SetNativeParameterFloat(RE::TESForm* self, float param, int paramIndex) {
		if (!ParameterBaseTypeMatchesLocked(ParamType::kFloat, self, paramIndex, __func__)){
			return false;
		}
		
		void* slot = FloatToConditionParam(param);

		ParamData paramData;
		return SetNativeParameter(self, slot, paramIndex, ParamType::kFloat, paramData);
	}

	//mutex locked = false
    bool SetNativeParameterString(RE::TESForm* self, std::string param, int paramIndex) {
		if (!ParameterBaseTypeMatchesLocked(ParamType::kString, self, paramIndex, __func__)){
			return false;
		}
		
		ParamData paramData; 
        //must use new fixed string or it causes ctd when using EvaluateCondition
        RE::BSFixedString* paramPtr = new RE::BSFixedString(param);
        bool success = SetNativeParameter(self, paramPtr, paramIndex, ParamType::kString, paramData);
        // if (!success){ //handled in SetNativeParameter now
		// 	delete paramPtr;
		// }
		return success;
    }
    
	//mutex locked = true
	bool SetNativeParameterForm(RE::TESForm* self, RE::TESForm* param, int paramIndex) {
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		mutex.lock(); //[X]===============================================================[X]
		
		RE::FormID formId = self->GetFormID();
		
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (!item){
			logger::error("conditionId[{:08X}] condition item is nullptr", formId);
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		if (IsGetEventDataCondition(item)){
			paramIndex--;
		}
		
		if (!ParameterBaseTypeMatches(ParamType::kForm, item, paramIndex, formId, __func__)){
			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return false;
		}
		
		RE::VMHandle handle;
		RE::ObjectRefHandle objHandle; 
		PromotedRefType promotedRefType = PromotedRefType::kNotPromoted;
		
		ParamType paramType = ParamType::kForm; 
		RE::TESObjectREFR* ref ;
        if (gfuncs::IsFormValid(param)) {
			handle = gfuncs::GetHandle(param);
			ref = param->AsReference();
        	if (gfuncs::IsFormValid(ref)) { 
				paramType = ParamType::kObjectReference;
				objHandle = ref->GetHandle();
				promotedRefType = PromoteConditionRef(ref);
				
			}
        } 
		else {
			param = nullptr;
		}
		
		ParamData paramData; 
		paramData.handle = handle; 
		paramData.objRefHandle = objHandle;
		paramData.promotedRefData.type = promotedRefType;
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
        bool success = SetNativeParameter(self, param, paramIndex, paramType, paramData);
		return success;
    }

	//mutex locked = false
	// no alias params for conditions exist
    // bool SetNativeParameterAlias(RE::TESForm* self, RE::BGSBaseAlias* param, int paramIndex) {
	// 	ParamData paramData; 
	// 	paramData.handle = gfuncs::GetHandle(param);
		
    //     bool success = SetNativeParameter(self, param, paramIndex, ParamType::kAlias, paramData);
    //     return success;
    // }

// Getters ==============================================================================================================================================

	//mutex locked = true
	int GetNativeComparison(RE::TESForm* self) {
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		int value = item != nullptr ? static_cast<int>(item->data.flags.opCode) : 0;
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
	//mutex locked = true
	float GetNativeValue(RE::TESForm* self) {
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		float value = 0.0f; 
		if (item){
			if (!item->data.flags.global){
				value = item->data.comparisonValue.f;
			}
		}
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
	//mutex locked = true
	RE::TESGlobal* GetNativeValueGlobalVariable(RE::TESForm* self) { 
        if (!gfuncs::IsFormValid(self)){
			return nullptr;
		}
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]

        RE::TESConditionItem* item = GetConditionItemForForm(self);
    
        if (!item) {
            logger::warn("something went wrong. conditionId[{:08X}] condition->item is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return nullptr;
        }

        RE::TESGlobal* globalVar = nullptr;
		
		// item->data.comparisonValue.g;
		
		if (item->data.flags.global){
			if (gfuncs::IsFormValid(item->data.comparisonValue.g)){
				globalVar = item->data.comparisonValue.g;
			}
		}
		
        mutex.unlock(); //[O]-------------------------------------------------------------[O]
        return globalVar;
    }
	
	//mutex locked = true
	int GetNativeConditionFunction(RE::TESForm* self) {
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		int value = item != nullptr ? static_cast<int>(item->data.functionData.function.get()) : 0;
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
	//mutex locked = true
	std::string GetNativeConditionFunctionString(RE::TESForm* self) {
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		std::string value = item != nullptr ? static_cast<std::string>(magic_enum::enum_name(item->data.functionData.function.get())) : "kNone";
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
	int GetNativeFunctionParameterBaseType(RE::TESForm* self, int paramIndex) {
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (IsGetEventDataCondition(item)){ //for kGetEventData, index 0 and 1 are combined in params[0], index 2 = params[1]
			if (paramIndex > 0){
				paramIndex--;
			}
		}
		int functionType = item != nullptr ? static_cast<int>(item->data.functionData.function.get()) : 0;
		int value = static_cast<int>(GetFunctionParameterBaseTypeCPP(functionType, paramIndex));
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	} 
	
	std::string GetNativeFunctionParameterBaseTypeString(RE::TESForm* self, int paramIndex) {
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (IsGetEventDataCondition(item)){ //for kGetEventData, index 0 and 1 are combined in params[0], index 2 = params[1]
			if (paramIndex > 0){
				paramIndex--;
			}
		}
		int functionType = item != nullptr ? static_cast<int>(item->data.functionData.function.get()) : 0;
		std::string value = static_cast<std::string>(magic_enum::enum_name(GetFunctionParameterBaseTypeCPP(functionType, paramIndex)));
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	} 
	
	//mutex locked = true
	int GetNativeFunctionParameterType(RE::TESForm* self, int paramIndex) {
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (IsGetEventDataCondition(item)){ //for kGetEventData, index 0 and 1 are combined in params[0], index 2 = params[1]
			if (paramIndex > 0){
				paramIndex--;
			}
		}
		int functionType = item != nullptr ? static_cast<int>(item->data.functionData.function.get()) : 0;
		int value = static_cast<int>(GetFunctionParameterTypeCpp(functionType, paramIndex));
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
	//mutex locked = true
	std::string GetNativeFunctionParameterTypeString(RE::TESForm* self, int paramIndex) {
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (IsGetEventDataCondition(item)){ //for kGetEventData, index 0 and 1 are combined in params[0], index 2 = params[1]
			if (paramIndex > 0){
				paramIndex--;
			}
		}
		int functionType = item != nullptr ? static_cast<int>(item->data.functionData.function.get()) : 0;
		std::string value = static_cast<std::string>(magic_enum::enum_name(GetFunctionParameterTypeCpp(functionType, paramIndex)));
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}

	//mutex locked = true
	bool GetNativeAndOr(RE::TESForm* self){ 
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		bool value = item != nullptr ? item->data.flags.isOR : false;
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
	//mutex locked = true
	bool GetNativeUsesAliases(RE::TESForm* self){ 
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		bool value = item != nullptr ? item->data.flags.usesAliases : false;
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
	//mutex locked = true
	bool GetNativeUsesPackData(RE::TESForm* self){ 
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		bool value = item != nullptr ? item->data.flags.usePackData : false;
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
	//mutex locked = true
	bool GetNativeSwapTarget(RE::TESForm* self){ 
		mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		bool value = item != nullptr ? item->data.flags.swapTarget : false;
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
	// kSelf = 0, (subject)
	// kTarget = 1,
	// kRef = 2,
	// kCombatTarget = 3,
	// kLinkedRef = 4,
	// kQuestAlias = 5,
	// kPackData = 6,
	// kEventData = 7,
	// kCommandTarget = 8
	// mutex locked = true
	int GetNativeRunOnFlag(RE::TESForm* self){ 
		if (!gfuncs::IsFormValid(self)){
			return false;
		} 
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (!item) {
            logger::warn("something went wrong. conditionId[{:08X}] condition->item is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return false;
        }
		
		int result = static_cast<int>(item->data.object.get());
		FlagConditionDataAltered(self);
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return result;
	}
	
	// mutex locked = true
	RE::TESObjectREFR* GetNativeRunOnRef(RE::TESForm* self){
		if (!gfuncs::IsFormValid(self)){
			return nullptr;
		} 
		
		RE::FormID formId = self->GetFormID();
		
        mutex.lock(); //[X]===============================================================[X]
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (!item) {
            logger::warn("something went wrong. conditionId[{:08X}] condition->item is nullptr", formId);
            mutex.unlock(); //[O]-------------------------------------------------------------[O]
            return nullptr;
        }
		
		RE::TESObjectREFR* ref = GetRefFromHandle(item->data.runOnRef);
		if (!gfuncs::IsFormValid(ref)){
			ref = nullptr;
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return ref;
	}
	
	//mutex locked = true
	// no bool paramter types exist for conditions
	// bool GetNativeParameterBool(RE::TESForm* self, int paramIndex) {
	// 	mutex.lock(); //[X]===============================================================[X]
	// 	bool value = false;
	// 	RE::TESConditionItem* item = GetConditionItemForForm(self);
	// 	if (item){
	// 		if (paramIndex < 0 || paramIndex > 1){
	// 			mutex.unlock(); //[O]-------------------------------------------------------------[O]
	// 			return value;
	// 		}
			
	// 		value = ConditionParamToBool(item->data.functionData.params[paramIndex]);
	// 	}
		
	// 	mutex.unlock(); //[O]-------------------------------------------------------------[O]
	// 	return value;
	// }
	
	//mutex locked = true
	// int and raw stores the same, using only raw function.
	// int GetNativeParameterInt(RE::TESForm* self, int paramIndex) {
	// 	mutex.lock(); //[X]===============================================================[X]
	// 	int value = -1;
	// 	RE::TESConditionItem* item = GetConditionItemForForm(self);
	// 	if (item){
	// 		if (paramIndex < 0 || paramIndex > 1){
	// 			mutex.unlock(); //[O]-------------------------------------------------------------[O]
	// 			return value;
	// 		}
			
	// 		value = ConditionParamToInt(item->data.functionData.params[paramIndex]);
	// 		logger::debug("value[{}]", value);
			
	// 		// std::string* valuePtr = static_cast<std::string*>(item->data.functionData.params[paramIndex]);
	// 		// logger::debug("valuePtr[{}]", *valuePtr);
	// 	}
		
	// 	mutex.unlock(); //[O]-------------------------------------------------------------[O]
	// 	return value;
	// }
	
	//mutex locked = true
	int GetNativeParameterInt(RE::TESForm* self, int paramIndex) {
		if (!gfuncs::IsFormValid(self)){
			return -1;
		}
		
		RE::FormID formId = self->GetFormID();
		
		mutex.lock(); //[X]===============================================================[X]
		int value = -1;
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (item){
			if (paramIndex < 0 || paramIndex > 1){ //holds true for EventDataConditionParamToRaw
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return value;
			}
			
			
			if (IsGetEventDataCondition(item)){
				if (!ParameterBaseTypeMatches(ParamType::kInt, item, 0, formId, __func__)){
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return value;
				}
				value = EventDataConditionParamToRaw(formId, item, paramIndex);
			}
			else {
				if (!ParameterBaseTypeMatches(ParamType::kInt, item, paramIndex, formId, __func__)){
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return value;
				}
				value = ConditionParamToRaw(item->data.functionData.params[paramIndex]);
			}
			
			// std::uint64_t slot = reinterpret_cast<std::uintptr_t>(item->data.functionData.params[paramIndex]);
			// logger::info("param0 slot = 0x{:016X}  low32 = 0x{:08X}  high32 = 0x{:08X}",
			// 	slot, static_cast<std::uint32_t>(slot), static_cast<std::uint32_t>(slot >> 32));
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
	//mutex locked = true
	float GetNativeParameterFloat(RE::TESForm* self, int paramIndex) {
		mutex.lock(); //[X]===============================================================[X]
		float value = -1.0f;
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (item){
			if (paramIndex < 0 || paramIndex > 1){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return value;
			}
		
			RE::FormID formId = self != nullptr ? self->GetFormID() : 0;
			if (!ParameterBaseTypeMatches(ParamType::kFloat, item, paramIndex, formId, __func__)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return value;
			}
			
			value = ConditionParamToFloat(item->data.functionData.params[paramIndex]);
			
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}
	
	//mutex locked = true
	std::string GetNativeParameterString(RE::TESForm* self, int paramIndex) {
		mutex.lock(); //[X]===============================================================[X]
		std::string value = "";
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (item){
			if (paramIndex < 0 || paramIndex > 1){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return value;
			}
			
			RE::FormID formId = self != nullptr ? self->GetFormID() : 0;
			if (!ParameterBaseTypeMatches(ParamType::kString, item, paramIndex, formId, __func__)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return value;
			}
			value = ConditionParamToString(item->data.functionData.params[paramIndex]);
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	}

	//mutex locked = true
	RE::TESForm* GetNativeParameterForm(RE::TESForm* self, int paramIndex) {
		mutex.lock(); //[X]===============================================================[X]
		RE::TESForm* value = nullptr;
		RE::TESConditionItem* item = GetConditionItemForForm(self);
		if (item){
			
			//for kGetEventData, param 1 and 2 are combined into params[0]
			//param 3 == params[1]
			if (IsGetEventDataCondition(item)){
				if (paramIndex == 2){
					paramIndex = 1;
				}
				else {
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return nullptr;
				}
			}
			
			if (paramIndex < 0 || paramIndex > 1){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return value;
			}
			
			RE::FormID formId = self != nullptr ? self->GetFormID() : 0;
			if (!ParameterBaseTypeMatches(ParamType::kForm, item, paramIndex, formId, __func__)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return value;
			}
			
			value = ConditionParamToForm(item->data.functionData.params[paramIndex]);
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		return value;
	} 
	
	//mutex locked = true
	// no condition params are alias type
	// RE::BGSBaseAlias* GetNativeParameterAlias(RE::TESForm* self, int paramIndex) {
	// 	mutex.lock(); //[X]===============================================================[X]
	// 	RE::BGSBaseAlias* value = nullptr;
	// 	RE::TESConditionItem* item = GetConditionItemForForm(self);
	// 	if (item){
	// 		if (paramIndex < 0 || paramIndex > 1){
	// 			mutex.unlock(); //[O]-------------------------------------------------------------[O]
	// 			return value;
	// 		}
	// 		value = ConditionParamToAlias(item->data.functionData.params[paramIndex]);
	// 	}
		
	// 	mutex.unlock(); //[O]-------------------------------------------------------------[O]
	// 	return value;
	// }
	
//cleanup ================================================================================================================================================================================
	
	//mutex locked = false
	void DeleteConditionDataForms(ConditionData& data){
		logger::trace("deleting unused ConditionList form. formId[{:08X}] number of conditions forms[{}]", data.conditionListFormID, data.linkedConditionItemFormIds.size());
		 
		RE::TESForm* listForm = RE::TESForm::LookupByID(data.conditionListFormID);
		if (gfuncs::IsFormValid(listForm)){
			listForm->SetDelete(true);
			delete listForm;
		}
		
		for (int i = data.linkedConditionItemFormIds.size() - 1; i >= 0; i--){
			RE::FormID linkedId = data.linkedConditionItemFormIds[i];
			if (linkedId != 0){
				RE::TESForm* linkedScriptForm = RE::TESForm::LookupByID(linkedId);
				if (gfuncs::IsFormValid(linkedScriptForm)){
					linkedScriptForm->SetDelete(true);
					delete linkedScriptForm;
				}
			}
		}
		
		data.linkedConditionItemFormIds.clear();
	}

	//mutex locked = false
	bool ConditionDataIsInUse(ConditionData& data){
		// logger::trace("ConditionList[{:08X}]", data.conditionListFormID);
		
		RE::TESForm* listForm = RE::TESForm::LookupByID(data.conditionListFormID);
		if (gfuncs::IsFormInUse(listForm, true, false, true, false)){
			// logger::trace("ConditionList[{:08X}] in use.", data.conditionListFormID);
			return true;
		}
	
		for (int i = data.linkedConditionItemFormIds.size() - 1; i >= 0; i--){
			RE::FormID linkedId = data.linkedConditionItemFormIds[i];
			if (linkedId != 0){
				RE::TESForm* linkedScriptForm = RE::TESForm::LookupByID(linkedId);
				if (gfuncs::IsFormInUse(linkedScriptForm, true, false, true, false)){
					// logger::trace("linkedScriptForm[{:08X}] in use.", linkedId);
					return true;
				}
			}
		}
		
		return false;
	}

	//called from save and load functions
	//mutex locked = true
	void CleanupUnusedForms(){
		mutex.lock(); //[X]===============================================================[X]
		
		std::vector<RE::FormID> IDsToErase;
		std::unordered_set<std::shared_ptr<ConditionData>> uniqueDatas;
		
		for (auto& pair : conditionsFormsMap){
			if (pair.second){
				uniqueDatas.insert(pair.second);
			}
		}
		
		for (auto dataPtr : uniqueDatas){
			ConditionData& data = *dataPtr;
			if (ConditionDataIsInUse(data)){ 
				continue; 
			}

			DeleteConditionDataForms(data);

			// vanilla: the data outlives its carriers, so drop the dead keys and ids
			if (data.conditionType == ConditionType::kVanilla){
				if (data.conditionListFormID != 0){
					conditionsFormsMap.erase(data.conditionListFormID);
					data.conditionListFormID = 0;
				}
				for (RE::FormID linkedId : data.linkedConditionItemFormIds){
					conditionsFormsMap.erase(linkedId);
				}
				data.linkedConditionItemFormIds.clear();
			}
			else if (data.conditionType == ConditionType::kCreated){
				IDsToErase.push_back(data.conditionListFormID);
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
		for (RE::FormID& id : IDsToErase){
			RE::TESForm* listForm = RE::TESForm::LookupByID(id);
			DestroyConditionList(listForm);
			// DestroyConditionCpp(id, false);
		}
		mutex.lock(); //[X]===============================================================[X]
		
		auto it = conditionFormsToDelete.begin();
		while (it != conditionFormsToDelete.end()){
			bool conditionFormErased = false; 
			
			RE::TESForm* conditionForm = *it;
			if (conditionForm){
				if (!gfuncs::IsFormInUse(conditionForm)) {
					it = conditionFormsToDelete.erase(it);
					conditionFormErased = true;
					conditionForm->SetDelete(true);
					delete conditionForm;
				}
			}
			else {
				it = conditionFormsToDelete.erase(it);
				conditionFormErased = true;
			}
			
			if (!conditionFormErased){
				it++;
			}
		}
		
		mutex.unlock(); //[O]-------------------------------------------------------------[O]
	}
	
// SERIALIZATION =========================================================================================================================================

	namespace Serialization {

		enum Version : std::uint32_t {
			kVersion1    = 1,   //first released version, conditions + linked forms + events
			kCurrent = kVersion1,
		};
		
		// A native event's identity across a save/load. The TESCondition* is NOT saved: it is
		// re-derived from the condition form after the conditions block has been loaded.
		struct LoadedNativeEvent {
			RE::FormID                 conditionFormId = 0;  // conditionId, as a form id
			RE::FormID                 targetFormId    = 0;  // 0 == no target
			PromotedRefType            promotedRefType = PromotedRefType::kNotPromoted;
			std::vector<RE::VMHandle>  handles;
		};

		//mutex locked = false -- CALLER MUST ALREADY HOLD mutex (called from Save)
		//Only events with nativeEvent == true are saved. Non-native events are recreated by
		//Papyrus scripts on load, so persisting them would double them up.
		bool SaveConditionEvents(SKSE::SerializationInterface* ssi){
			// count first -- we cannot write the size until we know how many qualify
			std::vector<std::shared_ptr<ConditionChangedEvent>> toSave;
			toSave.reserve(conditionEvents.size());

			for (std::shared_ptr<ConditionChangedEvent> conditionEvent : conditionEvents){
				if (!conditionEvent){
					continue;
				}
				
				// NOTE: IsNativeEvent()/IsDeleted() lock, so they cannot be used here.
				// Read the fields through the private-member friendship or add a
				// non-locking QNativeEventUnsafe() accessor. Using the public ones here
				// would deadlock, because Save holds the mutex.
				if (!conditionEvent->nativeEvent || conditionEvent->deleted){
					continue;
				}
				if (conditionEvent->registeredHandles.empty()){
					continue; // nothing listening -- it would be cancelled on load anyway
				}
				toSave.push_back(conditionEvent);
			}

			std::size_t size = toSave.size();
			if (!ssi->WriteRecordData(size)) { return false; }

			for (std::shared_ptr<ConditionChangedEvent> conditionEvent : toSave){
				// conditionId for a native event is the decimal/hex string of the condition form id
				RE::FormID conditionFormId = NativeConditionIdToFormId(conditionEvent->conditionId);
				
				if (!ssi->WriteRecordData(conditionFormId)) { return false; }

				RE::TESObjectREFR* targetRef = GetRefFromHandle(conditionEvent->target);
				RE::FormID targetFormId = gfuncs::IsFormValid(targetRef) ? targetRef->GetFormID() : 0;
				if (!ssi->WriteRecordData(targetFormId)) { return false; }

				if (!ssi->WriteRecordData(static_cast<std::uint32_t>(conditionEvent->promotedRefType))){ return false; }
				
				std::size_t handleCount = conditionEvent->registeredHandles.size();
				if (!ssi->WriteRecordData(handleCount)) { return false; }

				for (auto handle : conditionEvent->registeredHandles){
					if (!ssi->WriteRecordData(handle)) { return false; }
				}
			}

			return true;
		}

		//mutex locked = false -- CALLER MUST ALREADY HOLD mutex (called from Load, phase 1)
		//Reads the event block into `out`. Does not touch conditionEvents at all; that happens
		//in ApplyLoadedConditionEvents, after the mutex is released.
		bool LoadConditionEvents(SKSE::SerializationInterface* ssi, std::vector<LoadedNativeEvent>& out){
			out.clear();

			std::size_t size = 0;
			if (!ssi->ReadRecordData(size)) { return false; }

			out.reserve(size);

			for (std::size_t i = 0; i < size; i++){
				LoadedNativeEvent loaded;

				RE::FormID savedConditionFormId = 0;
				if (!ssi->ReadRecordData(savedConditionFormId)) { return false; }

				RE::FormID savedTargetFormId = 0;
				if (!ssi->ReadRecordData(savedTargetFormId)) { return false; }
				
				std::uint32_t iPromotedRefType = 0;
				if (!ssi->ReadRecordData(iPromotedRefType)) { return false; }
				
				std::size_t handleCount = 0;
				if (!ssi->ReadRecordData(handleCount)) { return false; }

				// read every handle even if we end up discarding this event -- the stream
				// position must stay in sync.
				std::vector<RE::VMHandle> savedHandles;
				savedHandles.reserve(handleCount);
				for (std::size_t h = 0; h < handleCount; h++){
					RE::VMHandle handle = 0;
					if (!ssi->ReadRecordData(handle)) { return false; }
					savedHandles.push_back(handle);
				}

				// --- resolve ---
				RE::FormID conditionFormIdResolved = 0;
				if (!ssi->ResolveFormID(savedConditionFormId, conditionFormIdResolved)){
					logger::warn("condition event: condition form[{:08X}] no longer exists, event dropped",
						savedConditionFormId);
					continue; // stream already consumed, safe to skip
				}
				loaded.conditionFormId = conditionFormIdResolved;

				if (savedTargetFormId != 0){
					RE::FormID targetFormIdResolved = 0;
					if (!ssi->ResolveFormID(savedTargetFormId, targetFormIdResolved)){
						logger::warn("condition event: target form[{:08X}] no longer exists, event dropped",
							savedTargetFormId);
						continue;
					}
					loaded.targetFormId = targetFormIdResolved;
				}
				
				for (auto savedHandle : savedHandles){
					RE::VMHandle handleResolved = 0;
					if (ssi->ResolveHandle(savedHandle, handleResolved) && handleResolved != 0){
						loaded.handles.push_back(handleResolved);
					}
					// a handle that fails to resolve is a script object that no longer exists
				}

				loaded.promotedRefType = static_cast<PromotedRefType>(iPromotedRefType);
				
				if (loaded.handles.empty()){
					logger::trace("condition event for condition[{:08X}]: no handles resolved, event dropped",
						loaded.conditionFormId);
					continue;
				}

				out.push_back(std::move(loaded));
			}

			return true;
		}

		//mutex locked = false -- MUST BE CALLED WITH mutex RELEASED (phase 2).
		//Every ConditionChangedEvent method below locks internally.
		//
		//  - a loaded event that already exists  -> keep it, refresh registeredHandles
		//  - a loaded event that does not exist  -> create it
		//  - a live nativeEvent not in `loaded`  -> cancel it
		void ApplyLoadedConditionEvents(const std::vector<LoadedNativeEvent>& loaded){
			// snapshot the live native events under the lock, then work on the copy unlocked
			std::vector<std::shared_ptr<ConditionChangedEvent>> liveNativeEvents;
			mutex.lock(); //[X]===============================================================[X]
			for (std::shared_ptr<ConditionChangedEvent> conditionEvent : conditionEvents){
				if (conditionEvent && conditionEvent->nativeEvent && !conditionEvent->deleted){
					liveNativeEvents.push_back(conditionEvent);
				}
			}
			mutex.unlock(); //[O]-------------------------------------------------------------[O]

			std::unordered_set<std::shared_ptr<ConditionChangedEvent>> matchedEvents;

			for (const auto& loadedEvent : loaded){
				std::string conditionId = FormIdToNativeConditionId(loadedEvent.conditionFormId);
				
				RE::ObjectRefHandle targetHandle;
				if (loadedEvent.targetFormId != 0){
					RE::TESForm* targetForm = RE::TESForm::LookupByID(loadedEvent.targetFormId);
					RE::TESObjectREFR* targetRef = gfuncs::IsFormValid(targetForm) ? targetForm->AsReference() : nullptr;
					if (gfuncs::IsFormValid(targetRef)){
						targetHandle = targetRef->GetHandle();
					}
				}

				// --- does it already exist? ---
				std::shared_ptr<ConditionChangedEvent> existing = nullptr;
				for (std::shared_ptr<ConditionChangedEvent> conditionEvent : liveNativeEvents){
					if (matchedEvents.find(conditionEvent) != matchedEvents.end()){
						continue;
					}
					if (conditionEvent->MatchesNativeKey(conditionId, targetHandle)){
						existing = conditionEvent;
						break;
					}
				}

				if (existing){
					// keep the event and its polling thread, just refresh who is listening
					existing->SetRegisteredHandles(loadedEvent.handles);
					matchedEvents.insert(existing);
					logger::trace("condition event for condition[{}] target[{:08X}] kept, {} handle(s) restored",
						conditionId, loadedEvent.targetFormId, loadedEvent.handles.size());
					continue;
				}

				// --- create it ---
				// the condition must be re-derived from the form: the saved TESCondition*
				// belonged to the previous session.
				RE::TESForm* conditionForm = RE::TESForm::LookupByID(loadedEvent.conditionFormId);
				if (!gfuncs::IsFormValid(conditionForm)){
					logger::warn("condition event: form[{:08X}] invalid, not recreated", loadedEvent.conditionFormId);
					continue;
				}

				RE::TESCondition* condition = GetConditionForForm(conditionForm);
				if (!condition){
					logger::warn("condition event: no condition for form[{:08X}], not recreated",
						loadedEvent.conditionFormId);
					continue;
				}

				RE::TESObjectREFR* targetRef = GetRefFromHandle(targetHandle);

				// nativeEvent = true
				std::shared_ptr<ConditionChangedEvent> newEvent = std::make_shared<ConditionChangedEvent>(conditionId, condition, targetRef, true);

				if (newEvent->IsDeleted()){
					logger::error("condition event for condition[{}] failed to construct", conditionId);
					continue;
				}

				newEvent->SetRegisteredHandles(loadedEvent.handles);

				mutex.lock(); //[X]===============================================================[X]
				conditionEvents.push_back(newEvent);
				isEmpty = false;
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				
				newEvent->ListenForConditionEventChangeStart(); //mutex locked

				logger::trace("condition event for condition[{:08X}] target[{:08X}] recreated with {} handle(s)",
					loadedEvent.conditionFormId, loadedEvent.targetFormId, loadedEvent.handles.size());
			}

			// --- cancel the native events that were not in the save ---
			for (std::shared_ptr<ConditionChangedEvent> conditionEvent : liveNativeEvents){
				if (matchedEvents.find(conditionEvent) != matchedEvents.end()){
					continue;
				}
				logger::trace("native condition event[{}] not in save, cancelling", conditionEvent->GetConditionId());
				DeleteConditionEvent(conditionEvent); // locks internally; marks deleted, erases, frees
			}
		}
		
		//mutex locked = false
		bool SaveFormIdEx(SKSE::SerializationInterface* ssi, const gfuncs::FormIdEx& id) {
			if (!ssi->WriteRecordData(id.formId))  { return false; }
			if (!ssi->WriteRecordData(id.subId))   { return false; }
			if (!ssi->WriteRecordData(id.subId2))   { return false; }
			std::uint32_t type = static_cast<std::uint32_t>(id.type);
			return ssi->WriteRecordData(type);
		}

		//mutex locked = false
		bool LoadFormIdEx(SKSE::SerializationInterface* ssi, gfuncs::FormIdEx& id, bool& resolved) {
			RE::FormID formId = 0;
			std::uint32_t subId = 0, subId2, type = 0;

			if (!ssi->ReadRecordData(formId)) { return false; }
			if (!ssi->ReadRecordData(subId))   { return false; }
			if (!ssi->ReadRecordData(subId2))   { return false; }
			if (!ssi->ReadRecordData(type))  { return false; }

			RE::FormID resolvedId = 0;
			resolved = ssi->ResolveFormID(formId, resolvedId);

			id.formId = resolved ? resolvedId : 0;
			id.subId  = subId;
			id.subId2  = subId2;
			id.type   = static_cast<gfuncs::FormIdExType>(type);
			return true;
		}

		// enum class ParamType { kNone, kBool, kInt, kRaw, kFloat, kString, kForm, kObjectReference, kAlias };
		//mutex locked = false
		bool SaveConditionItemParameter(SKSE::SerializationInterface* ssi, ParamType& type, void* param){
			if (!ssi->WriteRecordData(static_cast<int>(type))) { return false; }
			
			switch (type){
				// no bool param types
				// case ParamType::kBool: {
				// 	bool value = ConditionParamToBool(param);
				// 	if (!ssi->WriteRecordData(value)) { return false; }
				// 	break;
				// }
				
				// int and raw are the same
				// case ParamType::kInt: {
				// 	int value = ConditionParamToInt(param);
				// 	if (!ssi->WriteRecordData(value)) { return false; }
				// 	break;
				// }
				
				case ParamType::kInt: {
					int value = ConditionParamToRaw(param);
					if (!ssi->WriteRecordData(value)) { return false; }
					break;
				}
				
				case ParamType::kFloat: {
					float value = ConditionParamToFloat(param);
					if (!ssi->WriteRecordData(value)) { return false; }
					break;
				}
				
				case ParamType::kString: {
					std::string value = ConditionParamToString(param);
					if (!serialize::SaveString(conditionsRecord, ssi, value)) { return false; }
					break;
				}
				
				case ParamType::kForm:
				case ParamType::kObjectReference: {
					RE::TESForm* value = ConditionParamToForm(param);
					RE::FormID id = gfuncs::IsFormValid(value) ? value->GetFormID() : 0;
					if (!ssi->WriteRecordData(id)) { return false; }
					break;
				}
				
				// no alias param types
				// case ParamType::kAlias: {
				// 	RE::BGSBaseAlias* value = ConditionParamToAlias(param);
				// 	gfuncs::FormIdEx id = gfuncs::GetAliasKey(value);
				// 	if (!SaveFormIdEx(ssi, id)){
				// 		return false;
				// 	}
				// 	break;
				// }
			}
			
			return true;
		}
		
		// enum class ParamType { kNone, kBool, kInt, kRaw, kFloat, kString, kForm, kObjectReference, kAlias };
		//mutex locked = false
		ParamData LoadConditionItemParameter(SKSE::SerializationInterface* ssi, bool& success){
			ParamData paramData;
			
			int iParamType = 0;
			success = true; 
			
			if (!ssi->ReadRecordData(iParamType)) { 
				success = false; 
				return paramData;
			}
			
			ParamType type = static_cast<ParamType>(iParamType);
			paramData.paramType = type; 
			
			switch (type){
				// no bool param types
				// case ParamType::kBool: {
				// 	bool value = false;
				// 	if (!ssi->ReadRecordData(value)) { 
				// 		success = false; 
				// 	}
				// 	else {
				// 		paramData.data = BoolToConditionParam(value);
				// 	}
				// 	break;
				// }
				
				// int and raw are the same
				// case ParamType::kInt: {
				// 	int value = 0;
				// 	if (!ssi->ReadRecordData(value)) { 
				// 		success = false; 
				// 	}
				// 	else {
				// 		paramData.data = IntToConditionParam(value);
				// 	}
				// 	break;
				// }
				
				case ParamType::kInt: {
					int value = 0;
					if (!ssi->ReadRecordData(value)) { 
						success = false; 
					}
					else {
						paramData.data = RawToConditionParam(value);
					}
					break;
				}
				
				case ParamType::kFloat: {
					float value = 0.0f;
					if (!ssi->ReadRecordData(value)) { 
						success = false; 
					}
					else {
						paramData.data = FloatToConditionParam(value);
					}
					break;
				}
				
				case ParamType::kString: {
					std::string value = "";
					bool loadStringSuccess = false;
					value = serialize::LoadString(conditionsRecord, ssi, loadStringSuccess);
					if (!loadStringSuccess) { 
						success = false; 
					}
					else {
						paramData.data = StringToConditionParam(value);
						paramDatas[paramData.data] = paramData;
					}
					break;
				}
				
				case ParamType::kForm:
				case ParamType::kObjectReference: {
					RE::FormID id = 0;
					if (!ssi->ReadRecordData(id)) { 
						success = false; 
						return paramData;
					}
					
					RE::FormID resolvedId = 0;

    				if (!ssi->ResolveFormID(id, resolvedId)) {
						success = false; 
						return paramData;
					}
					
					if (resolvedId != 0){
						RE::TESForm* value = RE::TESForm::LookupByID(resolvedId);
						if (gfuncs::IsFormValid(value, false, false)){
							paramData.data = static_cast<void*>(value);
							paramData.handle = gfuncs::GetHandle(value);
							if (type == ParamType::kObjectReference){
								RE::TESObjectREFR* ref = value->As<RE::TESObjectREFR>();
								if (gfuncs::IsFormValid(ref)){
									paramData.objRefHandle = ref->GetHandle();
									paramDatas[paramData.data] = paramData;
								}
							}
						}
					}
					
					break;
				}
				
				// no alias param types
				// case ParamType::kAlias: {
				// 	gfuncs::FormIdEx id;
				// 	bool resolved = false;
				// 	if (!LoadFormIdEx(ssi, id, resolved)){
				// 		success = false; 
				// 	}
				// 	else {
				// 		RE::BGSBaseAlias* value = gfuncs::GetAliasFromKey(id);
				// 		if (value){
				// 			paramData.data = static_cast<void*>(value);
				// 			paramData.handle = gfuncs::GetHandle(value);
				// 		}
				// 	}
				// 	break;
				// }
			}
			
			return paramData;
		}
		
		//mutex locked = false
		bool SaveConditionItem(SKSE::SerializationInterface* ssi, RE::TESConditionItem* item){
			bool skipped = false; 
			if (!item){
				skipped = true;
			}
			
			if (!ssi->WriteRecordData(skipped)) { return false; }
			
			if (item){
				auto functionId = item->data.functionData.function.get();
				int intFunctionId = static_cast<int>(functionId);
				
				bool useGlobal = item->data.flags.global;
				float floatValue = useGlobal ? 0.0f : item->data.comparisonValue.f;
				RE::FormID globalValueId = 0;
				if (useGlobal && gfuncs::IsFormValid(item->data.comparisonValue.g)) {
					globalValueId = item->data.comparisonValue.g->GetFormID();
				}
				
				bool usePackData = item->data.flags.usePackData;
				bool isOr = item->data.flags.isOR;
				bool swapTarget = item->data.flags.swapTarget;
				bool usesAliases = item->data.flags.usesAliases;
				int opCodeComparison = static_cast<int>(item->data.flags.opCode);
				
				std::uint32_t dataID = item->data.dataID;
				int obj = static_cast<int>(item->data.object.get());
				std::uint32_t pad32 = item->data.pad32;
				std::uint32_t pad34 = item->data.pad34;
				std::uint16_t pad02 = item->data.functionData.pad02;
				std::uint16_t pad04 = item->data.functionData.pad04;
				
				RE::TESObjectREFR* ref = GetRefFromHandle(item->data.runOnRef);
				RE::FormID runOnRefId = gfuncs::IsFormValid(ref) ? ref->GetFormID() : 0;
				
				ParamType paramType0 = GetFunctionParameterBaseTypeCPP(intFunctionId, 0);
				ParamType paramType1 = GetFunctionParameterBaseTypeCPP(intFunctionId, 1);
				
				if (!ssi->WriteRecordData(intFunctionId)) { return false; }
				if (!ssi->WriteRecordData(floatValue)) { return false; }
				if (!ssi->WriteRecordData(globalValueId)) { return false; }
				if (!ssi->WriteRecordData(useGlobal)) { return false; }
				if (!ssi->WriteRecordData(usePackData)) { return false; }
				if (!ssi->WriteRecordData(isOr)) { return false; }
				if (!ssi->WriteRecordData(swapTarget)) { return false; }
				if (!ssi->WriteRecordData(usesAliases)) { return false; }
				if (!ssi->WriteRecordData(opCodeComparison)) { return false; }
				
				if (!ssi->WriteRecordData(dataID)) { return false; }
				if (!ssi->WriteRecordData(obj)) { return false; }
				if (!ssi->WriteRecordData(pad32)) { return false; }
				if (!ssi->WriteRecordData(pad34)) { return false; }
				if (!ssi->WriteRecordData(pad02)) { return false; }
				if (!ssi->WriteRecordData(pad04)) { return false; }
				if (!ssi->WriteRecordData(runOnRefId)) { return false; }
				
				if (!SaveConditionItemParameter(ssi, paramType0, item->data.functionData.params[0])) { return false; }
				if (!SaveConditionItemParameter(ssi, paramType1, item->data.functionData.params[1])) { return false; }
				
				// in SaveConditionItem, after reading the fields
				// logger::info("SAVE item[{}] fn[{}] op[{}] obj[{}] global[{}] f[{}] g[{:08X}] "
				// 	"runOnRef[{:08X}] p0[{}] type0[{}] p1[{}] type1[{}] dataID[{:08X}]",
				// 	static_cast<void*>(item), intFunctionId, opCodeComparison, obj, useGlobal,
				// 	floatValue, globalValueId, runOnRefId,
				// 	item->data.functionData.params[0], magic_enum::enum_name(paramType0),
				// 	item->data.functionData.params[1], magic_enum::enum_name(paramType1), dataID
				// );
			}
			
			return true;
		}
		
		//mutex locked = false
		RE::TESConditionItem* LoadConditionItem(SKSE::SerializationInterface* ssi, bool& success){
			success = true;
			
			bool skipped = false; 
			
			if (!ssi->ReadRecordData(skipped)) { 
				success = false;
				return nullptr; 
			}
		
			if (skipped){
				return nullptr;
			}
			
			int intFunctionId = 0;
			float floatValue = 0.0;
			RE::FormID globalValueId = 0;
			bool useGlobal = false;
			bool usePackData = false;
			bool isOr = false;
			bool swapTarget = false;
			bool usesAliases = false;
			int opCodeComparison = 0;
			
			std::uint32_t dataID = 0;
			int obj = 0;
			std::uint32_t pad32 = 0;
			std::uint32_t pad34 = 0;
			std::uint16_t pad02 = 0;
			std::uint16_t pad04 = 0;
			RE::FormID runOnRefId = 0;
			
			if (!ssi->ReadRecordData(intFunctionId)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(floatValue)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(globalValueId)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(useGlobal)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(usePackData)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(isOr)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(swapTarget)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(usesAliases)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(opCodeComparison)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(dataID)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(obj)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(pad32)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(pad34)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(pad02)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(pad04)) { 
				success = false;
				return nullptr; 
			}
			
			if (!ssi->ReadRecordData(runOnRefId)) { 
				success = false;
				return nullptr; 
			}
			
			if (intFunctionId < 0 || intFunctionId > 735) {
				logger::error("loaded condition item function id[{}] is not a known function, discarding", intFunctionId);
				success = false;
				return nullptr;
			}
			
			ParamType paramType0 = GetFunctionParameterBaseTypeCPP(intFunctionId, 0);
			ParamType paramType1 = GetFunctionParameterBaseTypeCPP(intFunctionId, 1);
			
			bool loadParam = true;
			
			ParamData loadedParamData0 = LoadConditionItemParameter(ssi, loadParam);
			if (!loadParam){
				success = false;
				return nullptr; 
			}
			void* param0 = loadedParamData0.data;
			
			ParamData loadedParamData1 = LoadConditionItemParameter(ssi, loadParam);
			if (!loadParam){
				DestroyConditionParameter(param0);
				success = false;
				return nullptr; 
			}
			void* param1 = loadedParamData1.data;
			
			
			//this shouldn't happend
			if (paramType0 != loadedParamData0.paramType || paramType1 != loadedParamData1.paramType){
				DestroyConditionParameter(param0);
				DestroyConditionParameter(param1);
				success = false;
				return nullptr; 
			}
			
			RE::TESConditionItem* item = new RE::TESConditionItem;
			item->next = nullptr; 
			
			// comparisonValue is a UNION { float f; TESGlobal* g; }. Write exactly one member and
			// let flags.global record which one it is -- writing both destroys the first.
			if (useGlobal) {
				RE::TESGlobal* globalValue = nullptr;
				RE::FormID resolvedGlobalValueId = 0;

				if (globalValueId != 0 && ssi->ResolveFormID(globalValueId, resolvedGlobalValueId)) {
					globalValue = RE::TESForm::LookupByID<RE::TESGlobal>(resolvedGlobalValueId);
					if (!gfuncs::IsFormValid(globalValue)) { globalValue = nullptr; }
				}

				if (globalValue) {
					item->data.comparisonValue.g = globalValue;
					item->data.flags.global = true;
				}
				else {
					// the global's plugin is gone -- fall back to the float rather than leaving a
					// null pointer the engine will dereference when it evaluates the condition
					logger::warn("global[{:08X}] for loaded condition item not found, falling back to float[{}]",
						globalValueId, floatValue);
					item->data.comparisonValue.f = floatValue;
					item->data.flags.global = false;
				}
			}
			else {
				item->data.comparisonValue.f = floatValue;
				item->data.flags.global = false;
			}
			
			item->data.functionData.function = static_cast<RE::FUNCTION_DATA::FunctionID>(intFunctionId);
			item->data.flags.usePackData = usePackData;
			item->data.flags.isOR = isOr;
			item->data.flags.swapTarget = swapTarget;
			item->data.flags.usesAliases = usesAliases;
			item->data.flags.opCode = static_cast<RE::CONDITION_ITEM_DATA::OpCode>(opCodeComparison);
			
			item->data.dataID = dataID;
			item->data.object = static_cast<RE::CONDITIONITEMOBJECT>(obj);
			item->data.pad32 = pad32;
			item->data.pad34 = pad34;
			item->data.functionData.pad02 = pad02;
			item->data.functionData.pad04 = pad04;
		
			RE::FormID resolvedRefId;
			if (ssi->ResolveFormID(runOnRefId, resolvedRefId)){
				RE::TESObjectREFR* ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(resolvedRefId);
				if (gfuncs::IsFormValid(ref)){
					item->data.runOnRef = ref->GetHandle();
				}
			}
			
			item->data.functionData.params[0] = param0;
			item->data.functionData.params[1] = param1;
			
			// logger::info("LOAD item[{}] fn[{}] op[{}] obj[{}] global[{}] f[{}] g[{:08X}] "
			// 	"runOnRef[{:08X}] p0[{}] type0[{}] p1[{}] type1[{}] dataID[{:08X}]",
			// 	static_cast<void*>(item), intFunctionId, opCodeComparison, obj, useGlobal,
			// 	floatValue, globalValueId, runOnRefId,
			// 	item->data.functionData.params[0], magic_enum::enum_name(paramType0),
			// 	item->data.functionData.params[1], magic_enum::enum_name(paramType1), dataID
			// );
			
			return item;
		}
		
// -----------------------------------------------------------------------------------------------------
// 3. Reconcile the loaded temporaries with the live containers
// -----------------------------------------------------------------------------------------------------

		// mutex locked = false -- CALLER MUST ALREADY HOLD mutex
		void ReconcileLoadedConditions(
			std::unordered_map<gfuncs::FormIdEx, std::shared_ptr<ConditionData>>& loadedCreatedVanillaConditionIds, //loaded vanilla conditions with no linked conditionList
			std::unordered_map<RE::FormID, std::shared_ptr<ConditionData>>& loadedConditionsFormMap,
			std::unordered_set<RE::TESCondition*>& loadedCreatedConditions,
			std::unordered_set<RE::TESConditionItem*>& loadedCreatedConditionItems)
		{
			// -------------------------------------------------------------------------
			// Build the set of objects that belong to the newly loaded state.
			//
			// For kCreated:
			//     condition and every condition item are plugin-owned.
			//
			// For kVanilla:
			//     condition itself is NOT plugin-owned.
			//     An item is plugin-owned if it was already tracked as plugin-owned
			//     or if it was allocated while loading.
			// -------------------------------------------------------------------------

			std::unordered_set<RE::TESCondition*> finalCreatedConditions;
			std::unordered_set<RE::TESConditionItem*> finalCreatedConditionItems;
			std::unordered_set<std::shared_ptr<ConditionData>> loadedDatas;
			
			for (auto& [formID, dataPtr] : loadedConditionsFormMap){ 
				if (dataPtr) {
					loadedDatas.insert(dataPtr);
				}
			}
			
			for (auto& [idEx, dataPtr] : loadedCreatedVanillaConditionIds){ 
				if (dataPtr) {
					loadedDatas.insert(dataPtr);
				} 
			}

			for (auto& dataPtr : loadedDatas) {
				if (!dataPtr || !dataPtr->condition) {
					continue;
				}

				RE::TESCondition* condition = dataPtr->condition;

				if (dataPtr->conditionType == ConditionType::kCreated) {
					finalCreatedConditions.insert(condition);
				}

				for (auto* item : GetAllConditionItems(condition)) {
					if (!item) {
						continue;
					}

					// Every item belonging to a completely plugin-created condition
					// is plugin-owned.
					if (dataPtr->conditionType == ConditionType::kCreated) {
						finalCreatedConditionItems.insert(item);
						continue;
					}

					// For vanilla conditions, only plugin-created items are owned by us.
					if (createdConditionItems.find(item) != createdConditionItems.end() ||
						loadedCreatedConditionItems.find(item) != loadedCreatedConditionItems.end()) {
						finalCreatedConditionItems.insert(item);
					}
				}
			}

			// -------------------------------------------------------------------------
			// Delete old plugin-created condition items that are no longer part of
			// the loaded state.
			//
			// IMPORTANT:
			// Never delete an item merely because it disappeared from the save unless
			// it is in createdConditionItems. Vanilla items are not ours to delete.
			// -------------------------------------------------------------------------
			std::unordered_set<RE::TESConditionItem*> csItemsToDelete;

			for (auto it = createdConditionItems.begin(); it != createdConditionItems.end(); ++it) {
				RE::TESConditionItem* item = *it;
				if (!item || finalCreatedConditionItems.contains(item)) {
					continue;
				}
				csItemsToDelete.insert(item);
			}

			for (auto* item : csItemsToDelete){
				DestroyConditionItem(item);
			}
			
			// -------------------------------------------------------------------------
			// Delete old plugin-created conditions that are no longer part of the
			// loaded state.
			// -------------------------------------------------------------------------

			std::unordered_set<RE::TESCondition*> csToDelete;
			
			for (auto it = createdConditions.begin(); it != createdConditions.end(); ++it) {
				RE::TESCondition* condition = *it;
				if (!condition || finalCreatedConditions.contains(condition)) {
					continue;
				}
				csToDelete.insert(condition);
			}

			for (RE::TESCondition* condition : csToDelete){
				DeleteTESCondition(condition);
			}
			
			// -------------------------------------------------------------------------
			// Publish the loaded state.
			//
			// Do NOT merge with the existing conditionsFormsMap. The save is
			// authoritative.
			// -------------------------------------------------------------------------

			conditionsFormsMap = std::move(loadedConditionsFormMap);

			// Replace ownership sets rather than merging them.
			createdConditions = std::move(finalCreatedConditions);
			createdConditionItems = std::move(finalCreatedConditionItems);

			// -------------------------------------------------------------------------
			// Rebuild the secondary vanilla-condition index.
			
			// -------------------------------------------------------------------------

			//loadedCreatedVanillaConditionIds are vanilla conditions with no linked conditionList form
			createdVanillaConditionIds = loadedCreatedVanillaConditionIds;

			std::unordered_set<std::shared_ptr<ConditionData>> processed;

			for (auto& [formID, dataPtr] : conditionsFormsMap) {
				if (!dataPtr) {
					continue;
				}

				if (dataPtr->conditionType != ConditionType::kVanilla) {
					continue;
				}
				
				// Multiple FormIDs can point to the same ConditionData.
				if (!processed.insert(dataPtr).second) {
					continue;
				}
				
				//no linked vanilla form
				if (dataPtr->formIdEx.formId == 0) {
					continue;
				}

				createdVanillaConditionIds[dataPtr->formIdEx] = dataPtr;
			}

			logger::debug( 
				"reconciled: conditionsFormsMap[{}] createdConditions[{}] "
				"createdConditionItems[{}] createdVanillaConditionIds[{}]",
				conditionsFormsMap.size(),
				createdConditions.size(),
				createdConditionItems.size(),
				createdVanillaConditionIds.size());
		}
		
		//mutex locked = true
		bool Save(SKSE::SerializationInterface* ssi){
			CleanupUnusedForms();
			
			mutex.lock(); //[X]===============================================================[X]

			if (!ssi->OpenRecord(conditionsRecord, Version::kCurrent)) {
				logger::error("Failed to open record[0x{:08X}]", conditionsRecord);
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return false;
			}

			// ---------- [A] conditions ----------
			std::unordered_set<std::shared_ptr<ConditionData>> savedConditionDatas;
			for (auto& pair : conditionsFormsMap){
				if (pair.second){
					//only unique elements can be inserted into an unordered_set
					savedConditionDatas.insert(pair.second);
				}
			}
			
			for (auto& pair : createdVanillaConditionIds){
				if (pair.second){
					if (pair.second->wasAltered){
						//only unique elements can be inserted into an unordered_set
						savedConditionDatas.insert(pair.second);
					}
				}
			}
			
			std::size_t size = savedConditionDatas.size();
			if (!ssi->WriteRecordData(size)) {
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return false;
			}

			for (auto& dataPtr : savedConditionDatas){
				ConditionData& data = *dataPtr;
				
				if (!ssi->WriteRecordData(data.markedForDelete)){
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}
				
				RE::TESCondition* condition = GetConditionForConditionData(data);

				if (!ssi->WriteRecordData(data.conditionListFormID)) {
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}

				std::uint32_t conditionType = static_cast<std::uint32_t>(data.conditionType);
				if (!ssi->WriteRecordData(conditionType)) {
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}

				if (!SaveFormIdEx(ssi, data.formIdEx)) {
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}
				
				if (!ssi->WriteRecordData(data.wasAltered)) {
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}
				
				auto items = GetAllConditionItems(condition);
				std::size_t itemsSize = items.size();
				if (!ssi->WriteRecordData(itemsSize)) {
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}

				for (auto* item : items){
					if (!SaveConditionItem(ssi, item)){
						mutex.unlock(); //[O]-------------------------------------------------------------[O]
						return false;
					}
				}

				std::size_t linkedSize = data.linkedConditionItemFormIds.size();
				if (!ssi->WriteRecordData(linkedSize)) {
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}

				for (auto& formID : data.linkedConditionItemFormIds){
					if (!ssi->WriteRecordData(formID)) {
						mutex.unlock(); //[O]-------------------------------------------------------------[O]
						return false;
					}
				}
			}

			// ---------- [B] events ----------
			if (!SaveConditionEvents(ssi)){
				logger::error("failed to save condition events");
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return false;
			}

			mutex.unlock(); //[O]-------------------------------------------------------------[O]
			return true;
		}

		//mutex locked = true
		bool Load(SKSE::SerializationInterface* ssi, std::uint32_t version){
			// return false;
			
			std::vector<LoadedNativeEvent> loadedEvents;

			// ===================== PHASE 1: locked, stream only =====================
			mutex.lock(); //[X]===============================================================[X]

			std::unordered_map<RE::FormID, std::shared_ptr<ConditionData>> loadedConditionsFormMap;
			std::unordered_map<gfuncs::FormIdEx, std::shared_ptr<ConditionData>> loadedCreatedVanillaConditionIds; //loaded vanilla conditions with no linked conditionList
			std::unordered_set<RE::TESCondition*>     loadedCreatedConditions;
			std::unordered_set<RE::TESConditionItem*> loadedCreatedConditionItems;

			std::size_t size = 0;
			if (!ssi->ReadRecordData(size)){
				mutex.unlock(); //[O]-------------------------------------------------------------[O]
				return false;
			}

			for (std::size_t i = 0; i < size; i++){
				// ---- read everything for this entry first, so a discard never desyncs the stream ----
				bool markeredForDelete = false;
				if (!ssi->ReadRecordData(markeredForDelete)){
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}
				
				RE::FormID savedConditionListFormID = 0;
				if (!ssi->ReadRecordData(savedConditionListFormID)){
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}

				std::uint32_t uConditionType = 0;
				if (!ssi->ReadRecordData(uConditionType)){
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}
				ConditionType conditionType = static_cast<ConditionType>(uConditionType);

				gfuncs::FormIdEx conditionListFormIdEx;
				bool formIdExResolved = false;
				if (!LoadFormIdEx(ssi, conditionListFormIdEx, formIdExResolved)){
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}
				
				bool wasAltered = false;
				if (!ssi->ReadRecordData(wasAltered)) {
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}
				
				std::size_t conditionItemsSize = 0;
				if (!ssi->ReadRecordData(conditionItemsSize)){
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}

				std::vector<RE::TESConditionItem*> loadedItems;
				loadedItems.reserve(conditionItemsSize);
				bool itemsOk = true;

				for (std::size_t n = 0; n < conditionItemsSize; n++){
					bool loadSuccess = false;
					RE::TESConditionItem* loadedItem = LoadConditionItem(ssi, loadSuccess);
					if (!loadSuccess){
						itemsOk = false;
						break;  // stream is desynced -- cannot recover
					}
					if (loadedItem){
						loadedItems.push_back(loadedItem);
					}
				}

				if (!itemsOk){
					for (auto* item : loadedItems){
						DestroyConditionItem(item);
					}
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}

				std::size_t linkedFormIdsSize = 0;
				if (!ssi->ReadRecordData(linkedFormIdsSize)) {
					mutex.unlock(); //[O]-------------------------------------------------------------[O]
					return false;
				}

				std::vector<RE::FormID> savedLinkedFormIds;
				savedLinkedFormIds.reserve(linkedFormIdsSize);
				for (std::size_t n = 0; n < linkedFormIdsSize; n++){
					RE::FormID linkedFormId = 0;
					if (!ssi->ReadRecordData(linkedFormId)){
						mutex.unlock(); //[O]-------------------------------------------------------------[O]
						return false;
					}
					savedLinkedFormIds.push_back(linkedFormId);
				}

				// ---- resolve; a missing mod discards this entry only, it does not abort the load ----
				auto discardEntry = [&](std::string_view why){
					logger::warn("condition entry[{:08X}] discarded: {}", savedConditionListFormID, why);
					for (auto* item : loadedItems){
						DestroyConditionItem(item);
					}
				};

				bool didConditionListFormIDResolved = true;
				RE::FormID conditionListFormIDResolved = 0;
				if (!ssi->ResolveFormID(savedConditionListFormID, conditionListFormIDResolved)){
					if (conditionType == ConditionType::kVanilla && wasAltered){
						logger::debug("condition list form id did not resolve but still loading vanilla altered condition. FormIdEx id[{:08X}] subId[{}] subId2[{}] type[{}]",
							conditionListFormIdEx.formId, conditionListFormIdEx.subId, conditionListFormIdEx.subId2, magic_enum::enum_name(conditionListFormIdEx.type)
						);
						conditionListFormIDResolved = 0;
						didConditionListFormIDResolved = false;
					}
					else {
						discardEntry("condition list form id did not resolve");
						continue;
					}
				}

				if (didConditionListFormIDResolved){
					RE::TESForm* conditionListForm = RE::TESForm::LookupByID(conditionListFormIDResolved);
					if (!gfuncs::IsFormValid(conditionListForm, true, false)){
						if (conditionType == ConditionType::kVanilla && wasAltered){
							logger::debug("condition list form is not valid but still loading vanilla altered condition. FormIdEx id[{:08X}] subId[{}] subId2[{}] type[{}]",
								conditionListFormIdEx.formId, conditionListFormIdEx.subId, conditionListFormIdEx.subId2, magic_enum::enum_name(conditionListFormIdEx.type)
							);
							conditionListFormIDResolved = 0;
							didConditionListFormIDResolved = false;
						}
						else {
							discardEntry("condition list form no longer exists");
							continue;
						}
					}
				}
			
				// ---- find or make the ConditionData. Look up by the RESOLVED id. ----
				std::shared_ptr<ConditionData> dataPtr = std::make_shared<ConditionData>();
				
				dataPtr->markedForDelete = markeredForDelete;
				dataPtr->conditionListFormID = conditionListFormIDResolved;
				dataPtr->conditionType       = conditionType;
				dataPtr->formIdEx            = conditionListFormIdEx;
				dataPtr->wasAltered          = wasAltered;
				dataPtr->linkedConditionItemFormIds.clear();

				RE::TESCondition* condition = nullptr;

				if (conditionType == ConditionType::kCreated) {
					condition = new RE::TESCondition;

					loadedCreatedConditions.insert(condition);
				}
				else if (conditionType == ConditionType::kVanilla) {
					condition = GetConditionForFormIdEx(conditionListFormIdEx);
					if (!condition) {
						discardEntry("vanilla condition could not be resolved");
						continue;
					}
				}
				else {
					discardEntry("unsupported condition type");
					continue;
				}

				// ---- splice the loaded items into `condition`, reusing existing nodes ----
				
				std::vector<RE::TESConditionItem*> existingItems = GetAllConditionItems(condition);

				RE::TESConditionItem* previousItem = nullptr;

				for (std::size_t n = 0; n < loadedItems.size(); ++n) {
					RE::TESConditionItem* loadedItem = loadedItems[n];

					if (!loadedItem) {
						continue;
					}

					RE::TESConditionItem* targetItem = (n < existingItems.size()) ? existingItems[n] : nullptr;

					if (targetItem) {
						// in the splice, immediately before and after CopyConditionItemData
						// logger::info("SPLICE target[{}] BEFORE fn[{}] op[{}] obj[{}] global[{}] f[{}] p0[{}] p1[{}] dataID[{:08X}] next[{}]",
						// 	static_cast<void*>(targetItem),
						// 	static_cast<int>(targetItem->data.functionData.function.get()),
						// 	static_cast<int>(targetItem->data.flags.opCode),
						// 	static_cast<int>(targetItem->data.object.get()),
						// 	bool(targetItem->data.flags.global), targetItem->data.comparisonValue.f,
						// 	targetItem->data.functionData.params[0], targetItem->data.functionData.params[1],
						// 	targetItem->data.dataID, static_cast<void*>(targetItem->next)
						// );
						
						CopyConditionItemData(loadedItem, targetItem);

						// logger::info("SPLICE target[{}] BEFORE fn[{}] op[{}] obj[{}] global[{}] f[{}] p0[{}] p1[{}] dataID[{:08X}] next[{}]",
						// 	static_cast<void*>(targetItem),
						// 	static_cast<int>(targetItem->data.functionData.function.get()),
						// 	static_cast<int>(targetItem->data.flags.opCode),
						// 	static_cast<int>(targetItem->data.object.get()),
						// 	bool(targetItem->data.flags.global), targetItem->data.comparisonValue.f,
						// 	targetItem->data.functionData.params[0], targetItem->data.functionData.params[1],
						// 	targetItem->data.dataID, static_cast<void*>(targetItem->next)
						// );
						
						// The loaded item was only a temporary container.
						DestroyConditionParameter(loadedItem->data.functionData.params[0]);
						DestroyConditionParameter(loadedItem->data.functionData.params[1]);

						loadedItem->data.functionData.params[0] = nullptr;
						loadedItem->data.functionData.params[1] = nullptr;

						loadedItem->next = nullptr;
						delete loadedItem;
					}
					else {
						// There was no existing item at this position.
						//
						// This can only happen when the saved condition has more items
						// than the currently existing condition. The loaded item therefore
						// becomes a plugin-owned item.
						targetItem = loadedItem;
						loadedCreatedConditionItems.insert(targetItem);
					}

					if (previousItem) {
						previousItem->next = targetItem;
					}
					else {
						condition->head = targetItem;
					}

					previousItem = targetItem;
				}

				if (previousItem) {
					previousItem->next = nullptr;
				}
				else {
					condition->head = nullptr;
				}

				// free the existing items we truncated away (only ones we own)
				for (std::size_t n = loadedItems.size(); n < existingItems.size(); n++){
					RE::TESConditionItem* surplus = existingItems[n];
					if (!surplus){
						continue;
					}
					if (createdConditionItems.find(surplus) != createdConditionItems.end()){
						DestroyConditionItem(surplus);
					}
				}

				dataPtr->condition = condition;

				if (didConditionListFormIDResolved){
					loadedConditionsFormMap[conditionListFormIDResolved] = dataPtr;
				}
				else if (conditionType == ConditionType::kVanilla) {
					loadedCreatedVanillaConditionIds[conditionListFormIdEx] = dataPtr;
				}
				
				// ---- linked condition item forms ----
				for (auto savedLinkedFormId : savedLinkedFormIds){
					RE::FormID linkedFormIdResolved = 0;
					if (!ssi->ResolveFormID(savedLinkedFormId, linkedFormIdResolved)){
						logger::warn("linked condition form[{:08X}] did not resolve, skipped", savedLinkedFormId);
						continue;
					}

					RE::TESForm* linkedForm = RE::TESForm::LookupByID(linkedFormIdResolved);
					if (!gfuncs::IsFormValid(linkedForm, true, false)){
						logger::warn("linked condition form[{:08X}] no longer exists, skipped", linkedFormIdResolved);
						continue;
					}

					dataPtr->linkedConditionItemFormIds.push_back(linkedFormIdResolved);
					loadedConditionsFormMap[linkedFormIdResolved] = dataPtr;
				}
			}
			
			bool loadedConditionEvents = true;
			if (!LoadConditionEvents(ssi, loadedEvents)){
				loadedConditionEvents = false;
				logger::error("failed to load condition events");
				// mutex.unlock(); //[O]-------------------------------------------------------------[O]
				// return false; //still want to reconcile loaded conditions even if LoadConditionEvents fails
			}

			// ---------- reconcile, still under the lock (no event calls here) ----------
			ReconcileLoadedConditions(loadedCreatedVanillaConditionIds, loadedConditionsFormMap, loadedCreatedConditions, loadedCreatedConditionItems);

			mutex.unlock(); //[O]-------------------------------------------------------------[O]

			// ===================== PHASE 2: unlocked, events only =====================
			// Must be outside the lock: every ConditionChangedEvent method locks internally,
			// and the constructor spawns a polling thread that locks too.
			
			if (loadedConditionEvents){
				ApplyLoadedConditionEvents(loadedEvents);
			}

			CleanupUnusedForms();
			return true;
		}
	}
	
// =============================================================================================================================================================
	

    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
		
        //events
        vm->RegisterFunction("ConditionEventExists", "DbConditionFunctions", ConditionEventExists);
        vm->RegisterFunction("CreateConditionEvent", "DbConditionFunctions", CreateConditionEvent);
        vm->RegisterFunction("DestroyConditionEvent", "DbConditionFunctions", DestroyConditionEvent);
        vm->RegisterFunction("DestroyAllConditionEvents", "DbConditionFunctions", DestroyAllConditionEvents);
        vm->RegisterFunction("CountConditionEvents", "DbConditionFunctions", CountConditionEvents);

        //functions
        vm->RegisterFunction("CreateCondition", "DbConditionFunctions", CreateStringIdCondition);
        vm->RegisterFunction("DestroyCondition", "DbConditionFunctions", DestroyCondition);
		vm->RegisterFunction("ConditionExists", "DbConditionFunctions", ConditionExists);
		vm->RegisterFunction("GetFunctionParameterBaseType", "DbConditionFunctions", GetFunctionParameterBaseType);
		vm->RegisterFunction("GetFunctionParameterBaseTypeString", "DbConditionFunctions", GetFunctionParameterBaseTypeString);
		vm->RegisterFunction("GetFunctionParameterType", "DbConditionFunctions", GetFunctionParameterType);
		vm->RegisterFunction("GetFunctionParameterTypeString", "DbConditionFunctions", GetFunctionParameterTypeString);
		vm->RegisterFunction("SetConditionParameterForm", "DbConditionFunctions", SetConditionParameterForm);
        vm->RegisterFunction("SetConditionParameterAlias", "DbConditionFunctions", SetConditionParameterAlias);
        vm->RegisterFunction("SetConditionParameterBool", "DbConditionFunctions", SetConditionParameterBool);
        vm->RegisterFunction("SetConditionParameterInt", "DbConditionFunctions", SetConditionParameterInt);
        vm->RegisterFunction("SetConditionParameterRaw", "DbConditionFunctions", SetConditionParameterRaw);
        vm->RegisterFunction("SetConditionParameterFloat", "DbConditionFunctions", SetConditionParameterFloat);
        vm->RegisterFunction("SetConditionParameterString", "DbConditionFunctions", SetConditionParameterString);
        vm->RegisterFunction("SetConditionComparison", "DbConditionFunctions", SetConditionComparison);
        vm->RegisterFunction("SetConditionValue", "DbConditionFunctions", SetConditionValue);
        vm->RegisterFunction("EvaluateCondition", "DbConditionFunctions", EvaluateCondition);
        // vm->RegisterFunction("GetFunctionParameterType", "DbConditionFunctions", GetFunctionParameterType);
        
	//papyrus ConditionList functions =============================================================================================================================
        //global functions 
		vm->RegisterFunction("CreateConditionListForm", "ConditionList", CreateConditionListForm);
		vm->RegisterFunction("GetMagicEffectConditionListForm", "ConditionList", GetMagicEffectConditionListForm);
        vm->RegisterFunction("GetNthMagicEffectConditionListForm", "ConditionList", GetNthMagicEffectConditionListForm);
		vm->RegisterFunction("GetPerkConditionListForm", "ConditionList", GetPerkConditionListForm);
        vm->RegisterFunction("GetNthPerkEntryConditionListForm", "ConditionList", GetNthPerkEntryConditionListForm);
		vm->RegisterFunction("GetQuestStageConditionListForm", "ConditionList", GetQuestStageConditionListForm);
		vm->RegisterFunction("GetQuestObjectiveTargetConditionListForm", "ConditionList", GetQuestObjectiveTargetConditionListForm);
		vm->RegisterFunction("GetQuestConditionListForm", "ConditionList", GetQuestConditionListForm);
		vm->RegisterFunction("GetStoryManagerConditionListForm", "ConditionList", GetStoryManagerConditionListForm);
		vm->RegisterFunction("GetTopicInfoConditionListForm", "ConditionList", GetTopicInfoConditionListForm);
		vm->RegisterFunction("GetSceneConditionListForm", "ConditionList", GetSceneConditionListForm);
		vm->RegisterFunction("GetNthScenePhaseStartConditionListForm", "ConditionList", GetNthScenePhaseStartConditionListForm);
		vm->RegisterFunction("GetNthScenePhaseEndConditionListForm", "ConditionList", GetNthScenePhaseEndConditionListForm);
		vm->RegisterFunction("GetNthMessageMenuButtonConditionListForm", "ConditionList", GetNthMessageMenuButtonConditionListForm);
		vm->RegisterFunction("GetNthMusicTypeTrackConditionListForm", "ConditionList", GetNthMusicTypeTrackConditionListForm);
		vm->RegisterFunction("GetPackageConditionListForm", "ConditionList", GetPackageConditionListForm);
		vm->RegisterFunction("GetIdleConditionListForm", "ConditionList", GetIdleFormConditionListForm);
		vm->RegisterFunction("GetFactionVendorConditionListForm", "ConditionList", GetFactionVendorConditionListForm);
		vm->RegisterFunction("GetAliasConditionListForm", "ConditionList", GetAliasConditionListForm);
		vm->RegisterFunction("GetConstructibleObjectConditionListForm", "ConditionList", GetConstructibleObjectConditionListForm);
		
		//member functions 
		vm->RegisterFunction("GetType", "ConditionList", GetConditionListType);
		vm->RegisterFunction("Evaluate", "ConditionList", EvaluateConditionList);
		vm->RegisterFunction("GetNumOfConditions", "ConditionList", GetNumOfConditionsInList);
		vm->RegisterFunction("GetConditionForms", "ConditionList", GetConditionFormsInList);
		vm->RegisterFunction("CloneList", "ConditionList", CloneList);
		vm->RegisterFunction("GetAssociatedAlias", "ConditionList", GetAssociatedAlias);
		vm->RegisterFunction("GetAssociatedForm", "ConditionList", GetAssociatedForm);
		vm->RegisterFunction("GetAssociatedIndex", "ConditionList", GetAssociatedIndex);
		vm->RegisterFunction("SetNthCondition", "ConditionList", SetNthCondition);
		vm->RegisterFunction("RemoveNthConditionForm", "ConditionList", RemoveNthConditionForm);
		vm->RegisterFunction("InsertConditionForm", "ConditionList", InsertConditionForm);
		vm->RegisterFunction("RegisterForConditionChanged", "ConditionList", RegisterForNativeConditionChanged);
		vm->RegisterFunction("UnregisterForConditionChanged", "ConditionList", UnregisterForNativeConditionChanged);
		vm->RegisterFunction("Destroy", "ConditionList", DestroyConditionList);
		
	//papyrus Condition functions =============================================================================================================================
		// global functions
        vm->RegisterFunction("CreateConditionForm", "Condition", CreateConditionForm);
		
		//member functions
		vm->RegisterFunction("IsValid", "Condition", IsNativeConditionValid);
		vm->RegisterFunction("GetConditionListForm", "Condition", GetConditionListForm);
		vm->RegisterFunction("GetConditionListIndex", "Condition", GetConditionListIndex);
		vm->RegisterFunction("Evaluate", "Condition", EvaluateNativeCondition);
		vm->RegisterFunction("Copy", "Condition", CopyNativeCondition);
		vm->RegisterFunction("MoveToList", "Condition", MoveNativeConditionToList);
		vm->RegisterFunction("Destroy", "Condition", DestroyNativeCondition);
		
	//setters ======================================================================================================================================================
		
		vm->RegisterFunction("SetComparison", "Condition", SetNativeComparison);
		vm->RegisterFunction("SetValue", "Condition", SetNativeValue);
		vm->RegisterFunction("SetFunction", "Condition", SetNativeConditionFunction);
		
		vm->RegisterFunction("SetParameterForm", "Condition", SetNativeParameterForm);
		// vm->RegisterFunction("SetParameterAlias", "Condition", SetNativeParameterAlias);
		// vm->RegisterFunction("SetParameterBool", "Condition", SetNativeParameterBool);
		// vm->RegisterFunction("SetParameterInt", "Condition", SetNativeParameterInt);
		vm->RegisterFunction("SetParameterInt", "Condition", SetNativeParameterInt);
		vm->RegisterFunction("SetParameterFloat", "Condition", SetNativeParameterFloat);
		vm->RegisterFunction("SetParameterString", "Condition", SetNativeParameterString);
		
		vm->RegisterFunction("SetGlobalVariable", "Condition", SetNativeValueGlobalVariable);
		vm->RegisterFunction("SetAndOr", "Condition", SetNativeAndOr);
		vm->RegisterFunction("SetUsesAliases", "Condition", SetNativeUsesAliases);
		vm->RegisterFunction("SetUsesPackData", "Condition", SetNativeUsesPackData);
		vm->RegisterFunction("SetSwapsSubjectAndTarget", "Condition", SetNativeSwapTarget);
		vm->RegisterFunction("SetRunOnFlag", "Condition", SetNativeRunOnFlag);
		vm->RegisterFunction("SetRunOnRef", "Condition", SetNativeRunOnRef);
		
	//getters ======================================================================================================================================================
	
		vm->RegisterFunction("GetComparison", "Condition", GetNativeComparison);
		vm->RegisterFunction("GetValue", "Condition", GetNativeValue);
		vm->RegisterFunction("GetFunction", "Condition", GetNativeConditionFunction);
		vm->RegisterFunction("GetFunctionString", "Condition", GetNativeConditionFunctionString);
		vm->RegisterFunction("GetFunctionParameterBaseType", "Condition", GetNativeFunctionParameterBaseType);
		vm->RegisterFunction("GetFunctionParameterBaseTypeString", "Condition", GetNativeFunctionParameterBaseTypeString);
		vm->RegisterFunction("GetFunctionParameterType", "Condition", GetNativeFunctionParameterType);
		vm->RegisterFunction("GetFunctionParameterTypeString", "Condition", GetNativeFunctionParameterTypeString);
		
		vm->RegisterFunction("GetParameterForm", "Condition", GetNativeParameterForm);
		// vm->RegisterFunction("GetParameterAlias", "Condition", GetNativeParameterAlias);
		// vm->RegisterFunction("GetParameterBool", "Condition", GetNativeParameterBool);
		// vm->RegisterFunction("GetParameterInt", "Condition", GetNativeParameterInt);
		vm->RegisterFunction("GetParameterInt", "Condition", GetNativeParameterInt);
		vm->RegisterFunction("GetParameterFloat", "Condition", GetNativeParameterFloat);
		vm->RegisterFunction("GetParameterString", "Condition", GetNativeParameterString);
		
		vm->RegisterFunction("GetGlobalVariable", "Condition", GetNativeValueGlobalVariable);
		vm->RegisterFunction("GetAndOr", "Condition", GetNativeAndOr);
		vm->RegisterFunction("GetUsesAliases", "Condition", GetNativeUsesAliases);
		vm->RegisterFunction("GetUsesPackData", "Condition", GetNativeUsesPackData);
		vm->RegisterFunction("GetSwapsSubjectAndTarget", "Condition", GetNativeSwapTarget);
		vm->RegisterFunction("GetRunOnFlag", "Condition", GetNativeRunOnFlag);
		vm->RegisterFunction("GetRunOnRef", "Condition", GetNativeRunOnRef);
		
		return true;
    }
}