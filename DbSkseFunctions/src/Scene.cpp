#include "GeneralFunctions.h"
#include "RE/A/Actor.h"
#include "RE/B/BGSBaseAlias.h"
#include "RE/B/BGSRefAlias.h"
#include "RE/B/BGSScene.h"
#include "RE/B/BGSSceneAction.h"
#include "RE/B/BGSSceneActionDialogue.h"
#include "RE/B/BGSSceneActionPackage.h"
#include "RE/B/BGSSceneActionTimer.h"
#include "RE/E/EmotionTypes.h"
#include "RE/T/TESPackage.h"
#include "RE/T/TESQuest.h"
#include "RE/T/TypeTraits.h"

namespace Scene {
//temporary diagnostic ==================================================================================================================
	#define DB_OFFSET_OF(obj, member) \
	(reinterpret_cast<std::uintptr_t>(&(obj)->member) - reinterpret_cast<std::uintptr_t>(obj))

	//temporary diagnostic
	void LogSceneLayout(RE::BGSScene* akScene){
		if (!gfuncs::IsFormValid(akScene)){
			return;
		}

		const auto base = reinterpret_cast<std::uintptr_t>(akScene);

		logger::info("scene[{}] base[{}] sizeof[0x{:X}]",
			gfuncs::GetFormNameAndId(akScene), static_cast<void*>(akScene), sizeof(RE::BGSScene));

		logger::info("  commonlib reports: phases[{}] actors[{}] actorFlags[{}] actorProgressionFlags[{}] actions[{}]",
			akScene->phases.size(), akScene->actors.size(),
			akScene->actorFlags.size(), akScene->actorProgressionFlags.size(),
			akScene->actions.size());

		logger::info("  commonlib offsets: phases[0x{:X}] actors[0x{:X}] actorFlags[0x{:X}] actorProgressionFlags[0x{:X}] actions[0x{:X}] parentQuest[0x{:X}]",
			DB_OFFSET_OF(akScene, phases), DB_OFFSET_OF(akScene, actors),
			DB_OFFSET_OF(akScene, actorFlags), DB_OFFSET_OF(akScene, actorProgressionFlags),
			DB_OFFSET_OF(akScene, actions), DB_OFFSET_OF(akScene, parentQuest));

		//read every 8 byte aligned window as a BSTArray: data at +00, capacity at +08, size at +10
		for (std::size_t off = 0x20; off + 0x18 <= sizeof(RE::BGSScene); off += 0x08){
			auto* dataPtr  = *reinterpret_cast<void**>(base + off + 0x00);
			auto  capacity = *reinterpret_cast<std::uint32_t*>(base + off + 0x08);
			auto  size     = *reinterpret_cast<std::uint32_t*>(base + off + 0x10);

			logger::info("  off[0x{:02X}] data[{}] capacity[{}] size[{}]",
				off, dataPtr, capacity, size);
		}
	}
	
	//temporary diagnostic -- reads past actors.size() deliberately, using the real array bounds
	void LogSceneActorFlagEntries(RE::BGSScene* akScene){
		if (!gfuncs::IsFormValid(akScene)){
			return;
		}

		logger::info("scene[{}] actors[{}] actorFlags[{}] actorProgressionFlags[{}]",
			gfuncs::GetFormNameAndId(akScene), akScene->actors.size(),
			akScene->actorFlags.size(), akScene->actorProgressionFlags.size());

		for (std::uint32_t i = 0; i < akScene->actors.size(); i++){
			logger::info("  actors[{}] aliasId[{}]", i, akScene->actors[i]);
		}

		for (std::uint32_t i = 0; i < akScene->actorFlags.size(); i++){
			logger::info("  actorFlags[{}] raw[0x{:08X}]", i, akScene->actorFlags[i].underlying());
		}

		for (std::uint32_t i = 0; i < akScene->actorProgressionFlags.size(); i++){
			logger::info("  actorProgressionFlags[{}] raw[0x{:08X}]", i, akScene->actorProgressionFlags[i].underlying());
		}
	}
	
	//temporary diagnostic -- sweep every scene in the load order and classify the array sizes
	void LogAllSceneActorArraySizes(){
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh){
			return;
		}

		int sceneCount = 0;
		int equalCount = 0;
		int plusOneCount = 0;
		int otherCount = 0;

		for (auto* form : dh->GetFormArray(RE::FormType::Scene)){
			auto* scene = form ? form->As<RE::BGSScene>() : nullptr;
			if (!scene){
				continue;
			}

			const std::size_t a = scene->actors.size();
			const std::size_t f = scene->actorFlags.size();
			const std::size_t p = scene->actorProgressionFlags.size();

			sceneCount++;

			if (f == a && p == a){
				equalCount++;
			}
			else if (f == a + 1 && p == a + 1){
				plusOneCount++;
			}
			else {
				otherCount++;
				//only log the ones that fit neither pattern, so the log stays readable
				logger::info("  odd scene[{:08X}] actors[{}] flags[{}] progression[{}]",
					scene->GetFormID(), a, f, p);
			}
		}

		logger::info("scenes[{}] flags==actors[{}] flags==actors+1[{}] other[{}]",
			sceneCount, equalCount, plusOneCount, otherCount);
	}
	
	//temporary diagnostic -- for a doubled scene, compare the two halves entry by entry
	void LogSceneActorFlagHalves(RE::FormID a_sceneId){
		auto* scene = RE::TESForm::LookupByID<RE::BGSScene>(a_sceneId);
		if (!gfuncs::IsFormValid(scene)){
			logger::info("scene[{:08X}] not valid", a_sceneId);
			return;
		}

		const std::size_t a = scene->actors.size();
		const std::size_t f = scene->actorFlags.size();
		const std::size_t p = scene->actorProgressionFlags.size();

		logger::info("scene[{:08X}] actors[{}] flags[{}] progression[{}]", a_sceneId, a, f, p);

		if (a == 0 || f != a * 2 || p != a * 2){
			logger::info("  not a doubled scene, nothing to compare");
			return;
		}

		for (std::size_t i = 0; i < a; i++){
			logger::info("  actor[{}] aliasId[{}] flags first[0x{:08X}] second[0x{:08X}] match[{}] | prog first[0x{:08X}] second[0x{:08X}] match[{}]",
				i, scene->actors[i],
				scene->actorFlags[i].underlying(), scene->actorFlags[i + a].underlying(),
				scene->actorFlags[i].underlying() == scene->actorFlags[i + a].underlying(),
				scene->actorProgressionFlags[i].underlying(), scene->actorProgressionFlags[i + a].underlying(),
				scene->actorProgressionFlags[i].underlying() == scene->actorProgressionFlags[i + a].underlying()
			);
		}
	}
	
	// disagnostics, testing mismatch size between akScene->actors.size(); and akScene->actorFlags.size(); / akScene->actorProgressionFlags.size();
	// discovered that, if an esp overrides a scene in another esp or esm, actorFlags and actorProgressionFlags will be double the size of actors. 
	// the real values are between 0 and actors.size() for actorFlags and actorProgressionFlags. After that is garbage data set by the engine.
	// LogSceneLayout(akScene);
	// LogSceneActorFlagEntries(akScene);
	// LogAllSceneActorArraySizes();
	// LogSceneActorFlagHalves(0x00020E0A);
	
//====================================================================================================================================================================
	
	std::vector<RE::BGSScene*> GetAllScenesInQuest(RE::StaticFunctionTag*, RE::TESQuest* quest){
		std::vector<RE::BGSScene*> v; 
		if (!gfuncs::IsFormValid(quest)){
			return v;
		}
		
		for (auto* scene : quest->scenes){
			if (gfuncs::IsFormValid(scene)){
				v.push_back(scene);
			}
		}
		
		return v;
	}
	
	int CountScenesInQuest(RE::StaticFunctionTag*, RE::TESQuest* quest){
		return GetAllScenesInQuest(nullptr, quest).size();
	}
	
	bool IsSceneShuttingDown(RE::StaticFunctionTag*, RE::BGSScene* akScene){
		if (!gfuncs::IsFormValid(akScene)){
			return false;
		}
		
		return akScene->isShuttingDown;
	}
	
	// -1 if scene is not playing, or current phase starting with 1
	int GetSceneCurrentPhaseIndex(RE::StaticFunctionTag*, RE::BGSScene* akScene){
		if (!gfuncs::IsFormValid(akScene)){
			return -1;
		}

		return akScene->currentPhaseIndex;
	}
	
// scene phases ===============================================================================================================================

	int GetNumOfScenePhases(RE::StaticFunctionTag*, RE::BGSScene* akScene){
		if (!gfuncs::IsFormValid(akScene)){
			return -1;
		}
		
		return akScene->phases.size();
	}
	
	RE::BGSStoryManagerQuestNode* GetNthScenePhaseStoryManagerQuestNode(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		if (!gfuncs::IsFormValid(akScene)){
			return nullptr;
		}
		
		if (nthIndex < 0 || nthIndex >= akScene->phases.size()){
			return nullptr;
		}
		
		auto* phase = akScene->phases[nthIndex]; 
		if (!phase){
			return nullptr;
		}
		
		RE::BGSStoryManagerQuestNode* node = phase->questNode;
		if (!gfuncs::IsFormValid(node)){
			return nullptr;
		}
		
		return node;
	}

//scene actions ==============================================================================================================================================

	int GetNumOfSceneActions(RE::StaticFunctionTag*, RE::BGSScene* akScene){
		if (!gfuncs::IsFormValid(akScene)){
			return -1;
		}
		
		return akScene->actions.size();
	}
	
	RE::BGSSceneAction* GetNthSceneAction(RE::BGSScene* akScene, int nthIndex){
		if (!gfuncs::IsFormValid(akScene)){
			return nullptr;
		}
		
		if (nthIndex < 0 || nthIndex >= akScene->actions.size()){
			return nullptr;
		}
		
		return akScene->actions[nthIndex];
	}
	
	RE::BGSSceneActionDialogue* GetNthSceneActionDialogue(RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneAction* sceneAction = GetNthSceneAction(akScene, nthIndex);
		if (!sceneAction){
			return nullptr;
		}
		
		if (sceneAction->GetType() != RE::BGSSceneAction::Type::kDialogue){
			return nullptr;
		}
		
		return static_cast<RE::BGSSceneActionDialogue*>(sceneAction);
	}
	
	RE::BGSSceneActionPackage* GetNthSceneActionPackage(RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneAction* sceneAction = GetNthSceneAction(akScene, nthIndex);
		if (!sceneAction){
			return nullptr;
		}
		
		if (sceneAction->GetType() != RE::BGSSceneAction::Type::kPackage){
			return nullptr;
		}
		
		return static_cast<RE::BGSSceneActionPackage*>(sceneAction);
	}
	
	RE::BGSSceneActionTimer* GetNthSceneActionTimer(RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneAction* sceneAction = GetNthSceneAction(akScene, nthIndex);
		if (!sceneAction){
			return nullptr;
		}
		
		if (sceneAction->GetType() != RE::BGSSceneAction::Type::kTimer){
			return nullptr;
		}
		
		return static_cast<RE::BGSSceneActionTimer*>(sceneAction);
	}
	
	// -1 = not valid
	// 0 = kDialogue
	// 1 = kPackage
	// 2 = kTimer
	int GetNthSceneActionType(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneAction* sceneAction = GetNthSceneAction(akScene, nthIndex);
		if (!sceneAction){
			return -1;
		}
		
		return static_cast<int>(sceneAction->GetType());
	}
	
	int GetNthSceneActionStartPhase(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneAction* sceneAction = GetNthSceneAction(akScene, nthIndex);
		if (!sceneAction){
			return -1;
		}
		
		return sceneAction->startPhase;
	}
	
	bool SetNthSceneActionStartPhase(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, int phase){
		RE::BGSSceneAction* sceneAction = GetNthSceneAction(akScene, nthIndex);
		if (!sceneAction){
			return false;
		}
		
		sceneAction->startPhase = phase;
		
		return sceneAction->startPhase == phase;
	}
	
	int GetNthSceneActionEndPhase(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneAction* sceneAction = GetNthSceneAction(akScene, nthIndex);
		if (!sceneAction){
			return -1;
		}
		
		return sceneAction->endPhase;
	}
	
	bool SetNthSceneActionEndPhase(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, int phase){
		RE::BGSSceneAction* sceneAction = GetNthSceneAction(akScene, nthIndex);
		if (!sceneAction){
			return false;
		}
		
		sceneAction->endPhase = phase;
		
		return sceneAction->endPhase == phase;
	}
	
	int GetNthSceneActionIndex(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneAction* sceneAction = GetNthSceneAction(akScene, nthIndex);
		if (!sceneAction){
			return -1;
		}
		
		return sceneAction->index;
	}
	
	bool SetNthSceneActionIndex(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, int index){
		RE::BGSSceneAction* sceneAction = GetNthSceneAction(akScene, nthIndex);
		if (!sceneAction){
			return false;
		}
		
		sceneAction->index = index;
		
		return sceneAction->index == index;
	}
	
	// flag options:
	// 0 = kFaceTarget
	// 1 = kLooping
	// 2 = kHeadTrackPlayer
	bool GetNthSceneActionFlagSet(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, int flag){
		RE::BGSSceneAction* sceneAction = GetNthSceneAction(akScene, nthIndex);
		if (!sceneAction){
			return false;
		}
		
		switch (flag){
			case 0:
				return sceneAction->flags.any(RE::BGSSceneAction::Flag::kFaceTarget);
			
			case 1:
				return sceneAction->flags.any(RE::BGSSceneAction::Flag::kLooping);
			
			case 2:
				return sceneAction->flags.any(RE::BGSSceneAction::Flag::kHeadTrackPlayer);
		}
		
		return false;
	}
	
	// flag options:
	// 0 = kFaceTarget
	// 1 = kLooping
	// 2 = kHeadTrackPlayer
	bool SetNthSceneActionFlag(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, int flag, bool set){
		RE::BGSSceneAction* sceneAction = GetNthSceneAction(akScene, nthIndex);
		if (!sceneAction){
			return false;
		}
		
		if (set){
			switch (flag){
				case 0:
					sceneAction->flags.set(RE::BGSSceneAction::Flag::kFaceTarget);
					return sceneAction->flags.any(RE::BGSSceneAction::Flag::kFaceTarget);
				
				case 1:
					sceneAction->flags.set(RE::BGSSceneAction::Flag::kLooping);
					return sceneAction->flags.any(RE::BGSSceneAction::Flag::kLooping);
				
				case 2:
					sceneAction->flags.set(RE::BGSSceneAction::Flag::kHeadTrackPlayer);
					return sceneAction->flags.any(RE::BGSSceneAction::Flag::kHeadTrackPlayer);
			}
		}
		else {
			switch (flag){
				case 0:
					sceneAction->flags.reset(RE::BGSSceneAction::Flag::kFaceTarget);
					return !sceneAction->flags.any(RE::BGSSceneAction::Flag::kFaceTarget);
				
				case 1:
					sceneAction->flags.reset(RE::BGSSceneAction::Flag::kLooping);
					return !sceneAction->flags.any(RE::BGSSceneAction::Flag::kLooping);
				
				case 2:
					sceneAction->flags.reset(RE::BGSSceneAction::Flag::kHeadTrackPlayer);
					return !sceneAction->flags.any(RE::BGSSceneAction::Flag::kHeadTrackPlayer);
			}
		}
		
		return false;
	}
	
//scene action dialogue ==============================================================================================================================================
	
	// nth scene action must be kDialogue type
	RE::TESTopic* GetNthSceneActionTopic(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneActionDialogue* sceneAction = GetNthSceneActionDialogue(akScene, nthIndex);
		if (!sceneAction){
			return nullptr;
		}
		
		return sceneAction->topic;
	}
	
	// nth scene action must be kDialogue type
	bool SetNthSceneActionTopic(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, RE::TESTopic* akTopic){
		RE::BGSSceneActionDialogue* sceneAction = GetNthSceneActionDialogue(akScene, nthIndex);
		if (!sceneAction){
			return false;
		}
		
		sceneAction->topic = akTopic;
		
		return sceneAction->topic == akTopic;
	}
	
	// nth scene action must be kDialogue type
	float GetNthSceneActionLoopingMax(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneActionDialogue* sceneAction = GetNthSceneActionDialogue(akScene, nthIndex);
		if (!sceneAction){
			return 0.0f;
		}
		
		return sceneAction->loopingMax;
	}
	
	// nth scene action must be kDialogue type
	bool SetNthSceneActionLoopingMax(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, float loopingMax){
		RE::BGSSceneActionDialogue* sceneAction = GetNthSceneActionDialogue(akScene, nthIndex);
		if (!sceneAction){
			return false;
		}
		
		sceneAction->loopingMax = loopingMax;
		
		return sceneAction->loopingMax == loopingMax;
	}
	
	// nth scene action must be kDialogue type
	float GetNthSceneActionLoopingMin(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneActionDialogue* sceneAction = GetNthSceneActionDialogue(akScene, nthIndex);
		if (!sceneAction){
			return 0.0f;
		}
		
		return sceneAction->loopingMin;
	}
	
	// nth scene action must be kDialogue type
	bool SetNthSceneActionLoopingMin(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, float loopingMin){
		RE::BGSSceneActionDialogue* sceneAction = GetNthSceneActionDialogue(akScene, nthIndex);
		if (!sceneAction){
			return false;
		}
		
		sceneAction->loopingMin = loopingMin;
		
		return sceneAction->loopingMin == loopingMin;
	}
	
	// nth scene action must be kDialogue type
	// emotion types are:
	// kNeutral = 0,
	// kAnger = 1,
	// kDisgust = 2,
	// kFear = 3,
	// kSad = 4,
	// kHappy = 5,
	// kSurprise = 6,
	// kPuzzled = 7,
	int GetNthSceneActionEmotionType(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneActionDialogue* sceneAction = GetNthSceneActionDialogue(akScene, nthIndex);
		if (!sceneAction){
			return 0;
		}
		
		return static_cast<int>(sceneAction->emotionType);
	}
	
	// nth scene action must be kDialogue type
	// emotion types are:
	// kNeutral = 0,
	// kAnger = 1,
	// kDisgust = 2,
	// kFear = 3,
	// kSad = 4,
	// kHappy = 5,
	// kSurprise = 6,
	// kPuzzled = 7,
	bool SetNthSceneActionEmotionType(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, int emotionType){
		RE::BGSSceneActionDialogue* sceneAction = GetNthSceneActionDialogue(akScene, nthIndex);
		if (!sceneAction){
			return false;
		}
		
		sceneAction->emotionType = static_cast<RE::EmotionType>(emotionType);
		
		return static_cast<int>(sceneAction->emotionType) == emotionType;
	}

	// nth scene action must be kDialogue type
	// returns emotion intensity
	int GetNthSceneActionEmotionValue(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneActionDialogue* sceneAction = GetNthSceneActionDialogue(akScene, nthIndex);
		if (!sceneAction){
			return 0;
		}
		
		return sceneAction->emotionValue;
	}
	
	// nth scene action must be kDialogue type
	// sets emotion intensity
	bool SetNthSceneActionEmotionValue(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, int emotionValue){
		RE::BGSSceneActionDialogue* sceneAction = GetNthSceneActionDialogue(akScene, nthIndex);
		if (!sceneAction){
			return false;
		}
		
		sceneAction->emotionValue = emotionValue;
		
		return sceneAction->emotionValue == emotionValue;
	}
	
// Scene Action Package ==================================================================================================================================================
	
	// nth scene action must be kPackage type
	std::vector<RE::TESPackage*> GetNthSceneActionPackages(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		std::vector<RE::TESPackage*> v;
		RE::BGSSceneActionPackage* sceneAction = GetNthSceneActionPackage(akScene, nthIndex);
		if (!sceneAction){
			return v;
		}
		
		for (RE::TESPackage* package : sceneAction->packages){
			if (gfuncs::IsFormValid(package)){
				v.push_back(package);
			}
		}
		
		return v;
	}
	
	// nth scene action must be kPackage type
	int CountNthSceneActionPackages(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		return GetNthSceneActionPackages(nullptr, akScene, nthIndex).size();
	}

// Scene Action Timer ==================================================================================================================================================
	
	// nth scene action must be kTimer type
	int GetNthSceneActionTimerSeconds(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		RE::BGSSceneActionTimer* sceneAction = GetNthSceneActionTimer(akScene, nthIndex);
		if (!sceneAction){
			return 0;
		}
		
		return sceneAction->timerSeconds;
	}
	
	// nth scene action must be kTimer type
	bool SetNthSceneActionTimerSeconds(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, int timerSeconds){
		RE::BGSSceneActionTimer* sceneAction = GetNthSceneActionTimer(akScene, nthIndex);
		if (!sceneAction){
			return false;
		}
		
		sceneAction->timerSeconds = timerSeconds;
		
		return sceneAction->timerSeconds == timerSeconds;
	}

// Scene Actors ==================================================================================================================================================

	int GetNumOfSceneActors(RE::StaticFunctionTag*, RE::BGSScene* akScene){
		if (!gfuncs::IsFormValid(akScene)){
			return -1;
		}
		
		return akScene->actors.size();
	}
	
	int GetNthSceneActorAliasId(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		if (!gfuncs::IsFormValid(akScene)){
			return -1;
		}

		if (nthIndex < 0 || static_cast<std::size_t>(nthIndex) >= akScene->actors.size()){
			return -1;
		}

		return static_cast<int>(akScene->actors[nthIndex]);
	}
	
	int GetNthIndexOfSceneActorAliasId(RE::StaticFunctionTag*, RE::BGSScene* akScene, int aliasId){
		if (!gfuncs::IsFormValid(akScene)){
			return -1;
		}

		for (int i = 0; i < akScene->actors.size(); i++){
			if (static_cast<int>(akScene->actors[i]) == aliasId){
				return i;
			}
		}

		return -1;
	}
	
	RE::BGSRefAlias* GetNthSceneActorReferenceAlias(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		if (!gfuncs::IsFormValid(akScene)){
			return nullptr;
		}
		
		RE::TESQuest* quest = akScene->parentQuest;
		if (!gfuncs::IsFormValid(quest)){
			return nullptr;
		}
		
		if (nthIndex < 0 || nthIndex >= akScene->actors.size()){
			return nullptr;
		}
		
		std::uint32_t aliasId = akScene->actors[nthIndex];
		for (auto* alias : quest->aliases){
			if (alias){
				if (alias->aliasID == aliasId){
					RE::BGSRefAlias* refAlias = skyrim_cast<RE::BGSRefAlias*>(alias);
					if (refAlias){
						return refAlias;
					}
				}
			}
		}
		
		return nullptr;
	}
	
	RE::Actor* GetNthSceneActor(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex){
		if (!gfuncs::IsFormValid(akScene)){
			return nullptr;
		}
		
		RE::TESQuest* quest = akScene->parentQuest;
		if (!gfuncs::IsFormValid(quest)){
			return nullptr;
		}
		
		if (nthIndex < 0 || nthIndex >= akScene->actors.size()){
			return nullptr;
		}
		
		auto& aliasId = akScene->actors[nthIndex];
		auto handle = quest->GetAliasedRef(aliasId);
		auto* ref = gfuncs::GetRefFromObjectRefHandle(handle);
		if (gfuncs::IsFormValid(ref)){
			RE::Actor* actor = ref->As<RE::Actor>();
			if (gfuncs::IsFormValid(actor)){
				return actor;
			}
		}
		
		return nullptr;
	}
	
	// flag options are:
	// 0 = kNoPlayerActivation
	// 1 = kOptional
	// 2 = kRunOnlyScenePackages
	// 3 = kNoCommandState
	bool GetNthSceneActorFlagSet(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, int flag){
		if (!gfuncs::IsFormValid(akScene)){
			return false;
		}
		
		//actors is the real size as actorFlags can hold garbage doubled data after if this scene was overwritten by an esp.
		if (nthIndex < 0 || nthIndex >= akScene->actors.size()){
			return false;
		}
		
		switch (flag){
			case 0:
				return akScene->actorFlags[nthIndex].any(RE::SCENE_ACTOR_FLAG::kNoPlayerActivation);
			
			case 1:
				return akScene->actorFlags[nthIndex].any(RE::SCENE_ACTOR_FLAG::kOptional);
			
			case 2:
				return akScene->actorFlags[nthIndex].any(RE::SCENE_ACTOR_FLAG::kRunOnlyScenePackages);
			
			case 3:
				return akScene->actorFlags[nthIndex].any(RE::SCENE_ACTOR_FLAG::kNoCommandState);
		}
		
		return false;
	}
	
	// flag options are:
	// 0 = kNoPlayerActivation
	// 1 = kOptional
	// 2 = kRunOnlyScenePackages
	// 3 = kNoCommandState
	bool SetNthSceneActorFlag(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, int flag, bool set){
		if (!gfuncs::IsFormValid(akScene)){
			return false;
		}
		
		//actors is the real size as actorFlags can hold garbage doubled data after if this scene was overwritten by an esp.
		if (nthIndex < 0 || nthIndex >= akScene->actors.size()){
			return false;
		}
		
		if (set){
			switch (flag){
				case 0:
					akScene->actorFlags[nthIndex].set(RE::SCENE_ACTOR_FLAG::kNoPlayerActivation);
					return akScene->actorFlags[nthIndex].any(RE::SCENE_ACTOR_FLAG::kNoPlayerActivation);
				
				case 1:
					akScene->actorFlags[nthIndex].set(RE::SCENE_ACTOR_FLAG::kOptional);
					return akScene->actorFlags[nthIndex].any(RE::SCENE_ACTOR_FLAG::kOptional);
				
				case 2:
					akScene->actorFlags[nthIndex].set(RE::SCENE_ACTOR_FLAG::kRunOnlyScenePackages);
					return akScene->actorFlags[nthIndex].any(RE::SCENE_ACTOR_FLAG::kRunOnlyScenePackages);
				
				case 3:
					akScene->actorFlags[nthIndex].set(RE::SCENE_ACTOR_FLAG::kNoCommandState);
					return akScene->actorFlags[nthIndex].any(RE::SCENE_ACTOR_FLAG::kNoCommandState);
			}
		}
		else {
			switch (flag){
				case 0:
					akScene->actorFlags[nthIndex].reset(RE::SCENE_ACTOR_FLAG::kNoPlayerActivation);
					return !akScene->actorFlags[nthIndex].any(RE::SCENE_ACTOR_FLAG::kNoPlayerActivation);
				
				case 1:
					akScene->actorFlags[nthIndex].reset(RE::SCENE_ACTOR_FLAG::kOptional);
					return !akScene->actorFlags[nthIndex].any(RE::SCENE_ACTOR_FLAG::kOptional);
				
				case 2:
					akScene->actorFlags[nthIndex].reset(RE::SCENE_ACTOR_FLAG::kRunOnlyScenePackages);
					return !akScene->actorFlags[nthIndex].any(RE::SCENE_ACTOR_FLAG::kRunOnlyScenePackages);
				
				case 3:
					akScene->actorFlags[nthIndex].reset(RE::SCENE_ACTOR_FLAG::kNoCommandState);
					return !akScene->actorFlags[nthIndex].any(RE::SCENE_ACTOR_FLAG::kNoCommandState);
			}
		}
		
		return false;
	}
	
	// 0 = kDeathPause
	// 1 = kDeathEnd
	// 2 = kCombatPause
	// 3 = kCombatEnd
	// 4 = kDialoguePause
	// 5 = kDialogueEnd
	// 6 = kOBS_COMPause
	// 7 = kOBS_COMEnd
	bool GetNthSceneActorProgressionFlagSet(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, int flag){
		if (!gfuncs::IsFormValid(akScene)){
			return false;
		}
		
		//actors is the real size as actorFlags can hold garbage doubled data after if this scene was overwritten by an esp.
		if (nthIndex < 0 || nthIndex >= akScene->actors.size()){
			return false;
		}
		
		switch (flag){
			case 0:
				return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kDeathPause);
			
			case 1:
				return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kDeathEnd);
			
			case 2:
				return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kCombatPause);
			
			case 3:
				return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kCombatEnd);
			
			case 4:
				return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kDialoguePause);
			
			case 5:
				return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kDialogueEnd);
			
			case 6:
				return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kOBS_COMPause);
			
			case 7:
				return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kOBS_COMEnd);
		}
		
		return false;
	}
	
	// 0 = kDeathPause
	// 1 = kDeathEnd
	// 2 = kCombatPause
	// 3 = kCombatEnd
	// 4 = kDialoguePause
	// 5 = kDialogueEnd
	// 6 = kOBS_COMPause
	// 7 = kOBS_COMEnd
	bool SetNthSceneActorProgressionFlag(RE::StaticFunctionTag*, RE::BGSScene* akScene, int nthIndex, int flag, bool set){
		if (!gfuncs::IsFormValid(akScene)){
			return false;
		}
		
		//actors is the real size as actorFlags can hold garbage doubled data after if this scene was overwritten by an esp.
		if (nthIndex < 0 || nthIndex >= akScene->actors.size()){
			return false;
		}
		
		if (set) {
			switch (flag){
				case 0:
					akScene->actorProgressionFlags[nthIndex].set(RE::BGSScene::BehaviourFlag::kDeathPause);
					return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kDeathPause);
				
				case 1:
					akScene->actorProgressionFlags[nthIndex].set(RE::BGSScene::BehaviourFlag::kDeathEnd);
					return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kDeathEnd);
				
				case 2:
					akScene->actorProgressionFlags[nthIndex].set(RE::BGSScene::BehaviourFlag::kCombatPause);
					return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kCombatPause);
				
				case 3:
					akScene->actorProgressionFlags[nthIndex].set(RE::BGSScene::BehaviourFlag::kCombatEnd);
					return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kCombatEnd);
				
				case 4:
					akScene->actorProgressionFlags[nthIndex].set(RE::BGSScene::BehaviourFlag::kDialoguePause);
					return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kDialoguePause);
				
				case 5:
					akScene->actorProgressionFlags[nthIndex].set(RE::BGSScene::BehaviourFlag::kDialogueEnd);
					return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kDialogueEnd);
				
				case 6:
					akScene->actorProgressionFlags[nthIndex].set(RE::BGSScene::BehaviourFlag::kOBS_COMPause);
					return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kOBS_COMPause);
				
				case 7:
					akScene->actorProgressionFlags[nthIndex].set(RE::BGSScene::BehaviourFlag::kOBS_COMEnd);
					return akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kOBS_COMEnd);
			}
		}
		else {
			switch (flag){
				case 0:
					akScene->actorProgressionFlags[nthIndex].reset(RE::BGSScene::BehaviourFlag::kDeathPause);
					return !akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kDeathPause);
				
				case 1:
					akScene->actorProgressionFlags[nthIndex].reset(RE::BGSScene::BehaviourFlag::kDeathEnd);
					return !akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kDeathEnd);
				
				case 2:
					akScene->actorProgressionFlags[nthIndex].reset(RE::BGSScene::BehaviourFlag::kCombatPause);
					return !akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kCombatPause);
				
				case 3:
					akScene->actorProgressionFlags[nthIndex].reset(RE::BGSScene::BehaviourFlag::kCombatEnd);
					return !akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kCombatEnd);
				
				case 4:
					akScene->actorProgressionFlags[nthIndex].reset(RE::BGSScene::BehaviourFlag::kDialoguePause);
					return !akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kDialoguePause);
				
				case 5:
					akScene->actorProgressionFlags[nthIndex].reset(RE::BGSScene::BehaviourFlag::kDialogueEnd);
					return !akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kDialogueEnd);
				
				case 6:
					akScene->actorProgressionFlags[nthIndex].reset(RE::BGSScene::BehaviourFlag::kOBS_COMPause);
					return !akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kOBS_COMPause);
				
				case 7:
					akScene->actorProgressionFlags[nthIndex].reset(RE::BGSScene::BehaviourFlag::kOBS_COMEnd);
					return !akScene->actorProgressionFlags[nthIndex].any(RE::BGSScene::BehaviourFlag::kOBS_COMEnd);
			}
		}
		return false;
	}
	
	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm){
		// scene
		vm->RegisterFunction("GetAllScenesInQuest", "DbSkseFunctions", GetAllScenesInQuest);
		vm->RegisterFunction("CountScenesInQuest", "DbSkseFunctions", CountScenesInQuest);
		vm->RegisterFunction("IsSceneShuttingDown", "DbSkseFunctions", IsSceneShuttingDown);
		vm->RegisterFunction("GetSceneCurrentPhaseIndex", "DbSkseFunctions", GetSceneCurrentPhaseIndex);

		// scene phases
		vm->RegisterFunction("GetNumOfScenePhases", "DbSkseFunctions", GetNumOfScenePhases);
		vm->RegisterFunction("GetNthScenePhaseStoryManagerQuestNode", "DbSkseFunctions", GetNthScenePhaseStoryManagerQuestNode);

		// scene actions
		vm->RegisterFunction("GetNumOfSceneActions", "DbSkseFunctions", GetNumOfSceneActions);
		vm->RegisterFunction("GetNthSceneActionType", "DbSkseFunctions", GetNthSceneActionType);
		vm->RegisterFunction("GetNthSceneActionStartPhase", "DbSkseFunctions", GetNthSceneActionStartPhase);
		vm->RegisterFunction("SetNthSceneActionStartPhase", "DbSkseFunctions", SetNthSceneActionStartPhase);
		vm->RegisterFunction("GetNthSceneActionEndPhase", "DbSkseFunctions", GetNthSceneActionEndPhase);
		vm->RegisterFunction("SetNthSceneActionEndPhase", "DbSkseFunctions", SetNthSceneActionEndPhase);
		vm->RegisterFunction("GetNthSceneActionIndex", "DbSkseFunctions", GetNthSceneActionIndex);
		vm->RegisterFunction("SetNthSceneActionIndex", "DbSkseFunctions", SetNthSceneActionIndex);
		vm->RegisterFunction("GetNthSceneActionFlagSet", "DbSkseFunctions", GetNthSceneActionFlagSet);
		vm->RegisterFunction("SetNthSceneActionFlag", "DbSkseFunctions", SetNthSceneActionFlag);

		// scene action dialogue
		vm->RegisterFunction("GetNthSceneActionTopic", "DbSkseFunctions", GetNthSceneActionTopic);
		vm->RegisterFunction("SetNthSceneActionTopic", "DbSkseFunctions", SetNthSceneActionTopic);
		vm->RegisterFunction("GetNthSceneActionLoopingMax", "DbSkseFunctions", GetNthSceneActionLoopingMax);
		vm->RegisterFunction("SetNthSceneActionLoopingMax", "DbSkseFunctions", SetNthSceneActionLoopingMax);
		vm->RegisterFunction("GetNthSceneActionLoopingMin", "DbSkseFunctions", GetNthSceneActionLoopingMin);
		vm->RegisterFunction("SetNthSceneActionLoopingMin", "DbSkseFunctions", SetNthSceneActionLoopingMin);
		vm->RegisterFunction("GetNthSceneActionEmotionType", "DbSkseFunctions", GetNthSceneActionEmotionType);
		vm->RegisterFunction("SetNthSceneActionEmotionType", "DbSkseFunctions", SetNthSceneActionEmotionType);
		vm->RegisterFunction("GetNthSceneActionEmotionValue", "DbSkseFunctions", GetNthSceneActionEmotionValue);
		vm->RegisterFunction("SetNthSceneActionEmotionValue", "DbSkseFunctions", SetNthSceneActionEmotionValue);

		// scene action package
		vm->RegisterFunction("GetNthSceneActionPackages", "DbSkseFunctions", GetNthSceneActionPackages);
		vm->RegisterFunction("CountNthSceneActionPackages", "DbSkseFunctions", CountNthSceneActionPackages);

		// scene action timer
		vm->RegisterFunction("GetNthSceneActionTimerSeconds", "DbSkseFunctions", GetNthSceneActionTimerSeconds);
		vm->RegisterFunction("SetNthSceneActionTimerSeconds", "DbSkseFunctions", SetNthSceneActionTimerSeconds);

		// scene actors
		vm->RegisterFunction("GetNumOfSceneActors", "DbSkseFunctions", GetNumOfSceneActors);
		vm->RegisterFunction("GetNthSceneActorAliasId", "DbSkseFunctions", GetNthSceneActorAliasId);
		vm->RegisterFunction("GetNthIndexOfSceneActorAliasId", "DbSkseFunctions", GetNthIndexOfSceneActorAliasId);
		vm->RegisterFunction("GetNthSceneActorReferenceAlias", "DbSkseFunctions", GetNthSceneActorReferenceAlias);
		vm->RegisterFunction("GetNthSceneActor", "DbSkseFunctions", GetNthSceneActor);
		vm->RegisterFunction("GetNthSceneActorFlagSet", "DbSkseFunctions", GetNthSceneActorFlagSet);
		vm->RegisterFunction("SetNthSceneActorFlag", "DbSkseFunctions", SetNthSceneActorFlag);
		vm->RegisterFunction("GetNthSceneActorProgressionFlagSet", "DbSkseFunctions", GetNthSceneActorProgressionFlagSet);
		vm->RegisterFunction("SetNthSceneActorProgressionFlag", "DbSkseFunctions", SetNthSceneActorProgressionFlag);

		return true;
	}
	
}