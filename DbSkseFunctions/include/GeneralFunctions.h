#pragma once
// #include <functional>
#include <unordered_set>
#include "FormTypeTable.h"

namespace gfuncs {
	
	//currently only used for ConditionFunctions.cpp
	enum class FormIdExType : std::uint32_t {
        kForm = 0,                 // subId unused
        kAlias,                    // kForm is a quest, subId = aliasID
        kMagicEffect,	           // kForm is an RE::EffectItem
		kMagicItem,		           // kForm is an RE::MagicItem, subId = nthMagicEffect
		kConstructibleObj,         // kForm is a ConstructibleObject
		kPerk,			           // kForm is a perk
		kPerkEntryPointOwner,      // kForm is a perk, subId = is the nth entry point
		kPerkEntryPointTarget,     // kForm is a perk, subId = is the nth entry point
		kQuest,                    // kForm is a quest referencing objConditions, which are quest dialogue conditions. 
		kQuestStageIndex,          // kForm is a quest, subId = actual stage index (50, 100 ect as defined in the stage)
		kQuestObjectiveIndex,      // kForm is a quest, subId = actual objective index (50, 100 ect as defined in the objective). subId2 = the target (ref alias) index.
		kStoryManagerQuest,        // kForm is a quest referencing storyManagerConditions, which are specific to the exact quest in the story manager event SM node, not it's parent branch.
		kStoryManagerNodeBase,     // kForm is a StoryManagerNodeBase, which could be a StoryManagerEventNode, StoryManagerBranchNode or StoryManagerQuestNode.
        kTopicInfo,                // kForm is a TopicInfo
        kScene,                    // kForm is a Scene
        kScenePhaseStart,          // kForm is a Scene, subId = the nth scene phase. Gets BGSScenePhase startConditions.
        kScenePhaseEnd,            // kForm is a Scene, subId = the nth scene phase. Gets BGSScenePhase completionConditions.
		kMessageMenuButton,        // kForm is a Message, subId = the nth menu button in the message.
		kMusicTypeTrack,           // kForm is a MusicType, subId = the nth BGSMusicTrack in the MusicType.
		kPackage,                  // kForm is a package
		kIdleForm,                 // kForm is a idle
		kFaction                   // kForm is a faction
    };

	//formId extended to link more data.
	//currently only used for ConditionFunctions.cpp
    struct FormIdEx {
        RE::FormID    formId = 0;
        std::uint32_t subId  = 0;
        std::uint32_t subId2  = 0;
        FormIdExType  type   = FormIdExType::kForm;

        bool operator==(const FormIdEx&) const = default;
        [[nodiscard]] bool IsValid() const { return formId != 0; }
    };
	
    extern std::chrono::system_clock::time_point lastTimeGameWasLoaded;

		// 0000028A, this is the form the game uses as default when using PlaceAtMe with abForcePersistent == true.
	// It promotes this form as an owner of the ref with the BGSDynamicPersistenceManager.
	extern const RE::FormID promotedPersistenceDefaultFormID; 
	
    void LogAndMessage(std::string message, int logLevel = 0, int debugLevel = 0);

    void ConvertToLowerCase(std::string& s);

	std::vector<std::string> SplitString(std::string s, std::string delim);
	
	std::string RemoveWhiteSpace(std::string s);
	
	std::string RemoveSubStrings(std::string s, std::string toRemove);
	
	std::string JoinStrings(std::vector<std::string> arr, std::string delim);
	
	bool StringToBool(std::string s);
	
	std::string BoolToString(bool b, std::string trueString = "true", std::string falseString = "false");
	
    std::string uint32_to_string(uint32_t value);

    int GetRandomInt(int min, int max);

    bool IsHexString(std::string s);

    bool IsDecString(std::string s);

    std::string IntToHex(int i);

    std::string IntToHexPapyrus(int i);

    int HexToInt(std::string hex);

    uint64_t StringToUint64_t(std::string s);

	uint32_t StringToUint32_t(std::string s);
	
    bool StringHasNCharacters(std::string s, char c, int minNumber);

    //get the int after start char in string s. Example: GetIntAfterCharInString("arrows (21)") returns 21
    int GetIntAfterCharInString(std::string s, char startChar = '(', int iDefault = 0, int startIndex = 0);

	std::size_t GetUnsignedCharLength(const char* c);
	
	RE::BSScript::IObjectHandlePolicy* GetHandlePolicy();
	
    bool IsFormValid(RE::TESForm* form, bool checkDeleted = false, bool checkIdentity = true);

    bool AllFormsValid(std::vector<RE::TESForm*> forms, std::vector<std::string> formParamNames, std::string callingFunctionName, bool checkDeleted = false);

	FormIdEx GetAliasKey(RE::BGSBaseAlias* alias);
	
	RE::BGSBaseAlias* GetAliasFromKey(const FormIdEx& key);
	
	RE::TESQuestStageItem* GetQuestStageItem(RE::TESQuestStage* questStage, std::uint8_t a_index);
	
    std::string GetFormEditorId(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string nullFormString);

    void SetFormName(RE::TESForm* baseForm, RE::BSFixedString nuName);

    RE::BSFixedString GetFormName(RE::TESForm* akForm, RE::BSFixedString nullString = "null", RE::BSFixedString noNameString = "", bool returnIdIfNull = true);
    
    std::string GetFormNameAndId(RE::TESForm* akForm, std::string nullString = "null", std::string noNameString = "");

    std::string GetFormDataString(RE::TESForm* akForm, std::string nullString = "null", std::string noNameString = "No Name");

	std::string GetCallingScriptName(RE::BSScript::Internal::VirtualMachine* vm, RE::VMStackID stackID, RE::BSFixedString functionScriptName);
	
	std::string GetCallingScriptFunction(RE::BSScript::Internal::VirtualMachine* vm, RE::VMStackID stackID, RE::BSFixedString functionScriptName);
	
	RE::VMHandle GetCallingScriptHandle(RE::BSScript::Internal::VirtualMachine* vm, RE::VMStackID stackID, RE::BSFixedString functionScriptName);
	
	RE::TESForm* GetCallingScriptForm(RE::BSScript::Internal::VirtualMachine* vm, RE::VMStackID stackID, RE::BSFixedString functionScriptName);
	
    RE::TESFile* GetFileForForm(RE::TESForm* akForm);

    RE::TESFile* GetFileForRawFormId(RE::FormID rawFormID, RE::TESFile* file);

    std::string GetModName(RE::TESForm* form);

    void logFormMap(auto& map);

    void DelayedFunction(auto function, int delay) {
        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            function();
            });
        t.detach();
    }

    //return difference of time points in seconds as float
    float timePointDiffToFloat(std::chrono::system_clock::time_point end, std::chrono::system_clock::time_point start);

	float RadiansToDegrees(float radians);
	
	RE::RefHandle GetRefHandle(RE::TESObjectREFR* ref);
	
    RE::VMHandle GetHandle(RE::TESForm* akForm);

    RE::VMHandle GetHandle(RE::BGSBaseAlias* akAlias);

    RE::VMHandle GetHandle(RE::ActiveEffect* akEffect);

    RE::ActiveEffect* GetActiveEffectFromHandle(RE::VMHandle handle);

    RE::TESObjectREFR* GetRefFromHandle(RE::RefHandle& handle);

    RE::TESObjectREFR* GetRefFromObjectRefHandle(RE::ObjectRefHandle refHandle);

	std::optional<FormTypeTable::ExtendedFormType> GetFormTypeFromPapyrusPropertyName(std::string propertyName);
	
	bool IsScriptObjectTypeRegistered(RE::BSScript::IVirtualMachine* vm, RE::VMTypeID type);
	
	std::string GetRegisteredScriptObjectTypeName(RE::BSScript::IVirtualMachine* vm, RE::VMTypeID type);
	
	bool RegisterObjectType(RE::BSScript::IVirtualMachine* vm, RE::VMTypeID type, const char* scriptName);
	
	std::unordered_set<RE::VMHandle> GetHandlesWithScriptAttached(RE::BSFixedString sScriptName);
	
	std::vector<std::string> GetScriptsAttachedTohandle(RE::VMHandle& handle, bool RemoveDuplicates = true, bool spinLock = true);
	
	std::vector<std::string> GetScriptHierarchyForScriptOnHandle(RE::VMHandle& handle, std::string scriptName);
	
	bool ScriptHasParentOnHandle(RE::VMHandle& handle, std::string scriptName, std::string parentScriptName);
	
	bool ScriptHasChildOnHandle(RE::VMHandle& handle, std::string scriptName, std::string childScriptName);
	
	bool ScriptHasParent(std::string scriptName, std::string parentScriptName);
	
	bool ScriptHasChild(std::string scriptName, std::string childScriptName);
	
    bool IsScriptAttachedToHandle(RE::VMHandle& handle, RE::BSFixedString& sScriptName, bool spinLock = true);

    bool IsScriptAttachedToRef(RE::TESObjectREFR* ref, RE::BSFixedString sScriptName, bool checkIdentity = true);

    bool IsScriptAttachedToForm(RE::TESForm* akForm, RE::BSFixedString sScriptName, bool checkIdentity = true);
	
	bool IsScriptAttachedToAlias(RE::BGSBaseAlias* akAlias, RE::BSFixedString sScriptName);

    RE::BSTSmartPointer<RE::BSScript::Object> GetAttachedScriptObject(RE::VMHandle& handle, RE::BSFixedString& sScriptName);

	bool IsObjectRefOwner(RE::TESObjectREFR* ref, RE::TESForm* akForm, bool checkRef = true, bool checkForm = true);
	
	bool IsFormInUse(RE::TESForm* form, bool checkForm = true, bool checkDeleted = false, bool checkIdentity = true, bool checkIsOwner = true);
	
	bool IsAliasInUse(RE::BGSBaseAlias* alias);
	
	bool IsActiveEffectInUse(RE::ActiveEffect* activeEffect);
	
    RE::TESObjectREFR* GetDialogueTarget(RE::Actor* actor);

    RE::Actor* GetPlayerDialogueTarget();

    void RefreshItemMenu();

    bool IsRefActivatedMenu(RE::BSFixedString menu);

    bool IsRefActivatedMenuOpen();

    bool SetAliasQuestObjectFlag(RE::BGSBaseAlias* akAlias, bool set);

    bool IsAliasQuestObjectFlagSet(RE::BGSBaseAlias* akAlias);

    bool IsQuestObject(RE::TESObjectREFR* ref);

    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    bool ContainerContainsRef(RE::TESObjectREFR* containerRef, RE::TESObjectREFR* ref, bool checkContainerRef = true, bool checkRefIdentity = true, bool checkRef = true);
    
    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    std::int32_t GetBaseFormCount(RE::TESObjectREFR* containerRef, RE::TESBoundObject* akForm);

    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    std::int32_t GetItemCount(RE::TESObjectREFR* containerRef, RE::TESForm* item);

    bool formIsBowOrCrossbow(RE::TESForm* akForm);

    template< typename T >
    std::string IntToHex(T i) {
        std::stringstream stream;
        stream << ""
            << std::setfill('0') << std::setw(sizeof(T) * 2)
            << std::hex << i;
        return stream.str();
    }

    template <class T>
    void RemoveFromVectorByValue(std::vector<T>& v, T value) {
        v.erase(std::remove(v.begin(), v.end(), value), v.end());
    }

    void String_ReplaceAll(std::string& s, std::string searchString, std::string replaceString);

    void String_ReplaceAll(std::string& s, std::vector<std::string> searchStrings, std::vector<std::string> replaceStrings);

    bool StringContainsStringInVector(std::vector<std::string>& v, std::string value);

    template <typename T>
	bool VectorContains(const std::vector<T*>& vec, T* value) {
		return std::find(vec.begin(), vec.end(), value) != vec.end();
	}
	
    int GetIndexInVector(std::vector<uint32_t>& v, uint32_t& element);

    int GetIndexInVector(std::vector<RE::TESForm*>& v, RE::TESForm* element);

    int GetIndexInVector(std::vector<RE::BGSProjectile*>& v, RE::BGSProjectile* element);

    int GetIndexInVector(std::vector<RE::BSSoundHandle*>& v, RE::BSSoundHandle* element);

    int GetIndexInVector(std::vector<RE::BSSoundHandle> v, RE::BSSoundHandle element);

    int GetIndexInVector(std::vector<RE::VMHandle> v, RE::VMHandle& element);

    int GetIndexInVector(std::vector<RE::BSFixedString> v, RE::BSFixedString& element);

    int GetIndexInVector(std::vector<std::string> v, std::string& element);

    int GetIndexInVector(std::vector<RE::TESObjectREFR*> v, RE::TESObjectREFR* element);

	int GetIndexInVector(std::vector<RE::TESConditionItem*> v, RE::TESConditionItem* element);
	
    bool IsObjectInBSTArray(RE::BSTArray<RE::ObjectRefHandle>* v, RE::ObjectRefHandle& element);

    RE::NiAVObject* GetNiAVObjectForRef(RE::TESObjectREFR* ref);

    RE::BGSBaseAlias* GetQuestAliasById(RE::TESQuest* quest, int id);

    int GetWeatherType(RE::TESWeather* weather);

    RE::BSFixedString GetBSUIMessageDataTypeString(RE::BSUIMessageData* msgData);

    void RemoveDuplicates(std::vector<std::string>& vec);

    void RemoveDuplicates(std::vector<RE::FormID>& vec);

    void RemoveDuplicates(std::vector<RE::VMHandle>& vec);

    void RemoveDuplicates(std::vector<RE::TESForm*>& vec);

    void RemoveDuplicates(std::vector<RE::TESTopic*>& vec);
	
    void RemoveDuplicates(std::vector<RE::TESTopicInfo*>& vec);
	
	void RemoveDuplicates(std::vector<RE::BGSKeyword*>& vec);
	
	//add the elements of vecB to the end of vecA
    template< typename T >
	void PushbackVector(std::vector<T*>& vecA, std::vector<T*>& vecB){
		vecA.reserve(vecA.size() + vecB.size()); 
		vecA.insert(vecA.end(), vecB.begin(), vecB.end());
	}
	
    void CombineEventHandles(std::vector<RE::VMHandle>& handles, RE::TESForm* akForm, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& formHandles);

    void SendEvents(std::vector<RE::VMHandle> handles, RE::BSFixedString& sEvent, RE::BSScript::IFunctionArguments* args);
    
    void SendEvents(std::vector<RE::VMHandle> handles, const RE::BSFixedString& sEvent, RE::BSScript::IFunctionArguments* args, RE::BSTSmartPointer<RE::BSScript::Object> out = nullptr);
    
    RE::TESForm* FindNullForm();
	
	RE::BSTSmartPointer<RE::BSScript::Object> AttachPapyrusScriptToForm(RE::TESForm* form, const char* scriptName);
	
	RE::BSTSmartPointer<RE::BSScript::Object> AttachPapyrusScriptToAlias(RE::BGSBaseAlias* alias, const char* scriptName);
	
	bool UnattachPapyrusScriptFromForm(RE::TESForm* form, const char* scriptName);
	
	bool UnattachPapyrusScriptFromAlias(RE::BGSBaseAlias* alias, const char* scriptName);
	
    void Install();
}

// must be at global scope, not inside gfuncs.
// for using in unordered_map or other hashes.
template <>
struct std::hash<gfuncs::FormIdEx> {
    std::size_t operator()(const gfuncs::FormIdEx& k) const noexcept {
        constexpr std::size_t golden = 0x9E3779B97F4A7C15ULL;

        auto mix = [](std::size_t h, std::size_t v) noexcept {
            return h ^ (v + golden + (h << 6) + (h >> 2));
        };

        std::size_t h = static_cast<std::size_t>(k.formId);
        h = mix(h, k.subId);
        h = mix(h, k.subId2);
        h = mix(h, static_cast<std::size_t>(k.type));
        return h;
    }
};