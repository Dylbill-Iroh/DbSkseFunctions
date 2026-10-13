#include <memory>
#include <optional>
#include <iostream>
#include <unordered_set>
#include "Topic.h"
#include "GeneralFunctions.h"
#include "magic_enum/magic_enum.hpp"
#include "RE/B/BGSSoundDescriptorForm.h"
#include "RE/T/TESQuest.h"
#include "RE/T/TESTopic.h"
#include "RE/T/TESTopicInfo.h"
#include "RE/T/TypeTraits.h"

namespace Topic {
	RE::TESQuest* GetTopicOwningQuest(RE::StaticFunctionTag*, RE::TESTopic* topic){
		if (!gfuncs::IsFormValid(topic)){
			return 0;
		}
		
		if (gfuncs::IsFormValid(topic->ownerQuest)){
			return topic->ownerQuest;
		}
		
		return nullptr;
	} 
	
	// types
	// kPlayerDialogue = 0,
	// kCommandDialogue = 1,
	// kBranchedTotal = 2,
	// kSceneDialogue = kBranchedTotal,
	// kCombat = 3,
	// kFavors = 4,
	// kDetection = 5,
	// kService = 6,
	// kMiscellaneous = 7,
	int GetTopicType(RE::StaticFunctionTag*, RE::TESTopic* topic){
		if (!gfuncs::IsFormValid(topic)){
			return 0;
		}
		
		return static_cast<int>(topic->data.type.get());
	} 
	
	bool SetTopicType(RE::StaticFunctionTag*, RE::TESTopic* topic, int type){
		if (!gfuncs::IsFormValid(topic)){
			return 0;
		}
		
		if (type < 0 || type > 7){
			return false;
		}
		
		topic->data.type = static_cast<RE::DIALOGUE_TYPES::DIALOGUE_TYPE>(type);
		
		return true;
	} 
	
	std::string GetTopicTypeString(RE::StaticFunctionTag*, RE::TESTopic* topic){
		if (!gfuncs::IsFormValid(topic)){
			return "kNone";
		}
		
		auto type = topic->data.type.get();
		int itype = static_cast<int>(type);
		
		if (itype == 2){
			return "kSceneDialogue";
		}
		
		auto stype = magic_enum::enum_name(type);
		if (stype == ""){
			return "kNone";
		}
		
		return (std::string)stype;
	} 
	
	int GetTopicSubType(RE::StaticFunctionTag*, RE::TESTopic* topic){
		if (!gfuncs::IsFormValid(topic)){
			return 0;
		}
		
		return static_cast<int>(topic->data.subtype.get());
	} 
	
	bool SetTopicSubType(RE::StaticFunctionTag*, RE::TESTopic* topic, int subType){
		if (!gfuncs::IsFormValid(topic)){
			return false;
		}
		
		if (subType < 0 || subType > 102){
			return false;
		}
		
		topic->data.subtype = static_cast<RE::DIALOGUE_DATA::Subtype>(subType);
		
		return true;
	} 
	
	std::string GetTopicSubTypeString(RE::StaticFunctionTag*, RE::TESTopic* topic){
		if (!gfuncs::IsFormValid(topic)){
			return "kNone";
		}
		
		auto type = topic->data.subtype.get();
		int itype = static_cast<int>(type);
		
		if (itype == 2){
			return "kSceneDialogue";
		}
		
		auto stype = magic_enum::enum_name(type);
		if (stype == ""){
			return "kNone";
		}
		
		return (std::string)stype;
	} 
	
	int GetTopicPriorityAndJournalIndex(RE::StaticFunctionTag*, RE::TESTopic* topic){
		if (!gfuncs::IsFormValid(topic)){
			return 0;
		}
		
		return topic->priorityAndJournalIndex;
	} 
	
	bool SetTopicPriorityAndJournalIndex(RE::StaticFunctionTag*, RE::TESTopic* topic, int index){
		if (!gfuncs::IsFormValid(topic)){
			return false;
		}
		
		topic->priorityAndJournalIndex = index;
		return true;
	} 
	
	int GetNumOfTopicInfos(RE::StaticFunctionTag*, RE::TESTopic* topic){
		if (!gfuncs::IsFormValid(topic)){
			return 0;
		}
		
		return topic->numTopicInfos;
	} 
	
	// types 
	// kPlayerDialogue = 0,
	// kCommandDialogue = 1,
	// kBranchedTotal = 2,
	// kSceneDialogue = kBranchedTotal,
	// kCombat = 3,
	// kFavors = 4,
	// kDetection = 5,
	// kService = 6,
	// kMiscellaneous = 7,
	std::vector<RE::TESTopicInfo*> GetTopicInfosInTopic(RE::StaticFunctionTag*, RE::TESTopic* topic, int type){
		std::vector<RE::TESTopicInfo*> v;
		if (!gfuncs::IsFormValid(topic)){
			return v;
		}
		
		if (topic){
			if (type == -1 || static_cast<int>(topic->data.type.get()) == type){
				for (int i = 0; i < topic->numTopicInfos; i++){
					RE::TESTopicInfo* topicInfo = topic->topicInfos[i];
					if (gfuncs::IsFormValid(topicInfo)){
						v.push_back(topicInfo);
					}
				}
			}
		}
		
		return v;
	}
	
	// types
	// kPlayerDialogue = 0,
	// kCommandDialogue = 1,
	// kBranchedTotal = 2,
	// kSceneDialogue = kBranchedTotal,
	// kCombat = 3,
	// kFavors = 4,
	// kDetection = 5,
	// kService = 6,
	// kMiscellaneous = 7,
	std::vector<RE::TESTopic*> GetTopicsInQuest(RE::StaticFunctionTag*, RE::TESQuest* quest, int type) {
		std::vector<RE::TESTopic*> v;
		if (!gfuncs::IsFormValid(quest)) {
			return v;
		}

		std::unordered_set<RE::TESTopic*> seen;
		
		// branched dialogue -- player (0) and command (1) types.
		// one hash map per type, each mapping a branch to its topic array.
		for (auto& branchMap : quest->branchedDialogue) {
			for (auto& entry : branchMap) {
				RE::BSTArray<RE::TESTopic*>* topicArray = entry.second;
				if (!topicArray) {
					continue;
				}
				for (auto& topic : *topicArray) {
					if (!gfuncs::IsFormValid(topic)) {
						continue;
					}
					if (type == -1 || static_cast<int>(topic->data.type.get()) == type) {
						//ensures no duplicates are added to the vector, preserving first occurance ordering.
						if (seen.insert(topic).second) {
							v.push_back(topic);
						}
					}
				}
			}
		}
		
		for (auto& topics : quest->topics) {
			for (auto& topic : topics) {
				if (!gfuncs::IsFormValid(topic)) {
					continue;
				}
				if (type == -1 || static_cast<int>(topic->data.type.get()) == type) {
					//ensures no duplicates are added to the vector, preserving first occurance ordering.
					if (seen.insert(topic).second) {
						v.push_back(topic);
					}
				}
			}
		}
		
		// using unordered_set to filter out duplicates now to preserve occurance ordering
		// gfuncs::RemoveDuplicates(v);
		return v;
	}
	
	// types
	// kPlayerDialogue = 0,
	// kCommandDialogue = 1,
	// kBranchedTotal = 2,
	// kSceneDialogue = kBranchedTotal,
	// kCombat = 3,
	// kFavors = 4,
	// kDetection = 5,
	// kService = 6,
	// kMiscellaneous = 7,
	std::vector<RE::TESTopicInfo*> GetTopicInfosInQuest(RE::StaticFunctionTag*, RE::TESQuest* quest, int type) {
		std::vector<RE::TESTopicInfo*> v;
		if (!gfuncs::IsFormValid(quest)) {
			return v;
		}
		
		std::unordered_set<RE::TESTopicInfo*> seen;

		auto topics = GetTopicsInQuest(nullptr, quest, type);
		for (auto& topic : topics){
			if (gfuncs::IsFormValid(topic)){
				for (int i = 0; i < topic->numTopicInfos; i++){
					auto* info = topic->topicInfos[i];
					if (gfuncs::IsFormValid(info)){
						//ensures no duplicates are added to the vector, preserving first occurance ordering.
						if (seen.insert(info).second) {
							v.push_back(info);
						}
					}
				}
			}
		}

		return v;
	}

	
	RE::TESTopic* GetTopicInfoParent(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo){
		if (!gfuncs::IsFormValid(topicInfo)){
			return nullptr;
		}
		
		if (gfuncs::IsFormValid(topicInfo->parentTopic)){
			return topicInfo->parentTopic;
		}
		
		return nullptr;
	}
	
	//DNAM
	RE::TESTopicInfo* GetTopicInfoDataInfo(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo){
		if (!gfuncs::IsFormValid(topicInfo)){
			return nullptr;
		}
		
		if (gfuncs::IsFormValid(topicInfo->dataInfo)){
			return topicInfo->dataInfo;
		}
		
		return nullptr;
	}
	
	bool GetTopicInfoSaidOnce(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo){
		if (!gfuncs::IsFormValid(topicInfo)){
			return false;
		}
		
		return topicInfo->saidOnce;
	}
	
	bool SetTopicInfoSaidOnce(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, bool saidOnce){
		if (!gfuncs::IsFormValid(topicInfo)){
			return false;
		}
		
		topicInfo->saidOnce = saidOnce;
		return true;
	}
	
	// kNone = 0,
	// kSmall = 1,
	// kMedium = 2,
	// kLarge = 3
	int GetTopicInfoFavorLevel(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo){
		if (!gfuncs::IsFormValid(topicInfo)){
			return 0;
		}
		
		return static_cast<int>(topicInfo->favorLevel.get());
	}
	
	// kNone = 0,
	// kSmall = 1,
	// kMedium = 2,
	// kLarge = 3
	bool SetTopicInfoFavorLevel(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int level){
		if (!gfuncs::IsFormValid(topicInfo)){
			return false;
		}
		
		if (level < 0 || level > 3){
			return false;
		}
		
		topicInfo->favorLevel = static_cast<RE::TESTopicInfo::FavorLevel>(level);
		return true;
	}
	
	RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS GetTopicInfoFlag(int flag){
		switch(flag){
			case 0: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kStartSceneOnEnd; // 1 << 0,
			case 1: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kRandom; // 1 << 1,
			case 2: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kSayOnce; // 1 << 2,
			case 3: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kRequiresPlayerActivation; // 1 << 3,
			case 4: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kInfoRefusal; // 1 << 4,
			case 5: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kRandomEnd; // 1 << 5,
			case 6: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kEndRunningScene; // 1 << 6,
			case 7: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kIsForceGreet; // 1 << 7,
			case 8: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kPlayerAddress; // 1 << 8,
			case 9: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kForceSubtitle; // 1 << 9,
			case 10: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kCanMoveWhileGreeting; // 1 << 10,
			case 11: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kNoLIPFile; // 1 << 11,
			case 12: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kPostProcess; // 1 << 12,
			case 13: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kCustomSoundOutput; // 1 << 13,
			case 14: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kSpendsFavorPoints; // 1 << 14
			default: return RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kNone;
		}
	}
	
	bool GetTopicInfoFlagSet(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int flag){
		if (!gfuncs::IsFormValid(topicInfo)){
			return false;
		}
		
		RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS infoFlag = GetTopicInfoFlag(flag);
		
		if (infoFlag == RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kNone){
			return false;
		}
		
		return topicInfo->data.flags.any(infoFlag);
	}
	
	bool SetTopicInfoFlag(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int flag, bool isTrue){
		if (!gfuncs::IsFormValid(topicInfo)){
			return false;
		}
		
		RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS infoFlag = GetTopicInfoFlag(flag);
		
		if (infoFlag == RE::TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kNone){
			return false;
		}
		
		if (topicInfo->data.flags.any(infoFlag) != isTrue){
			if (isTrue){
				topicInfo->data.flags.set(infoFlag);
			}
			else {
				topicInfo->data.flags.reset(infoFlag);
			}
		}
		
		return true;
	}
	
	std::vector<RE::TESTopicInfo::TESResponse*> GetTopicInfoResponses(RE::TESTopicInfo* topicInfo){
		std::vector<RE::TESTopicInfo::TESResponse*> v; 
		if (!gfuncs::IsFormValid(topicInfo)){
			return v;
		}
		
		RE::TESTopicInfo::TESResponseList* responses = topicInfo->GetResponseList();
		auto* response = responses->head;
		while (response){
			v.push_back(response);
			response = response->next;
		}
		
		return v;
	} 
	
	int GetNumOfResponses(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo){
		return GetTopicInfoResponses(topicInfo).size();
	} 
	
	//find the first index of the response that matches the text in the topicInfo, or return -1 if not found.
	int FindResponseIndexByText(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, std::string text){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (v.size() == 0){
			return -1;
		}
		
		for (int i = 0; i < v.size(); i++){
			auto* response = v[i];
			if (response){
				//case insensitive string compare
				if (_stricmp(response->responseText.c_str(), text.c_str()) == 0){
					return i;
				}
			}
		}
		
		return -1;
	}
	
	//find the first index of the response that matches the sound in the topicInfo, or return -1 if not found.
	int FindResponseIndexBySound(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, RE::BGSSoundDescriptorForm* sound){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (v.size() == 0){
			return -1;
		}
		
		if (!gfuncs::IsFormValid(sound)){
			return -1;
		}
		
		for (int i = 0; i < v.size(); i++){
			auto* response = v[i];
			if (response){
				//case insensitive string compare
				if (response->sound == sound){
					return i;
				}
			}
		}
		
		return -1;
	}
	
	std::string GetNthResponseText(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int index){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (index < 0 || index >= v.size()){
			return "";
		}
		
		if (!v[index]){
			return "";
		}
		
		return (std::string)v[index]->responseText;
	}
	
	bool SetNthResponseText(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int index, std::string text){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (index < 0 || index >= v.size()){
			return false;
		}
		
		if (!v[index]){
			return false;
		}
		
		v[index]->responseText = text;
		return true;
	}
	
	RE::BGSSoundDescriptorForm* GetNthResponseSound(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int index){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (index < 0 || index >= v.size()){
			return nullptr;
		}
		
		if (!v[index]){
			return nullptr;
		}
		
		return v[index]->sound;
	}
	
	bool SetNthResponseSound(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int index, RE::BGSSoundDescriptorForm* sound){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (index < 0 || index >= v.size()){
			return false;
		}
		
		if (!v[index]){
			return false;
		}
		
		v[index]->sound = sound; 
		return true;
	}
	
	RE::TESIdleForm* GetNthResponseSpeakerIdle(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int index){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (index < 0 || index >= v.size()){
			return nullptr;
		}
		
		if (!v[index]){
			return nullptr;
		}
		
		return v[index]->speakerIdle;
	}
	
	bool SetNthResponseSpeakerIdle(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int index, RE::TESIdleForm* idle){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (index < 0 || index >= v.size()){
			return false;
		}
		
		if (!v[index]){
			return false;
		}
		
		v[index]->speakerIdle = idle;
		return true;
	}
	
	RE::TESIdleForm* GetNthResponseListenerIdle(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int index){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (index < 0 || index >= v.size()){
			return nullptr;
		}
		
		if (!v[index]){
			return nullptr;
		}
		
		return v[index]->listenerIdle;
	}
	
	bool SetNthResponseListenerIdle(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int index, RE::TESIdleForm* idle){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (index < 0 || index >= v.size()){
			return false;
		}
		
		if (!v[index]){
			return false;
		}
		
		v[index]->listenerIdle = idle;
		return true;
	}
	
	// kNeutral = 0,
	// kAnger = 1,
	// kDisgust = 2,
	// kFear = 3,
	// kSad = 4,
	// kHappy = 5,
	// kSurprise = 6,
	// kPuzzled = 7
	int GetNthResponseEmotionType(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int index){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (index < 0 || index >= v.size()){
			return -1;
		}
		
		if (!v[index]){
			return -1;
		}
		
		return static_cast<int>(v[index]->emotionType.get());
	}
	
	bool SetNthResponseEmotionType(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int index, int emotionType){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (index < 0 || index >= v.size()){
			return false;
		}
		
		if (emotionType < 0 || emotionType > 7){
			return false;
		}
		
		if (!v[index]){
			return false;
		}
		
		v[index]->emotionType = static_cast<RE::TESTopicInfo::TESResponse::EmotionType>(emotionType);
		return true;
	}
	
	int GetNthResponseEmotionValue(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int index){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (index < 0 || index >= v.size()){
			return -1;
		}
		
		if (!v[index]){
			return -1;
		}
		
		return v[index]->emotionValue;
	}
	
	bool SetNthResponseEmotionValue(RE::StaticFunctionTag*, RE::TESTopicInfo* topicInfo, int index, int value){
		auto v = GetTopicInfoResponses(topicInfo); 
		if (index < 0 || index >= v.size()){
			return false;
		}
		
		if (!v[index]){
			return false;
		}
		
		v[index]->emotionValue = value;
		return true;
	}
	
	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
		//topic
		vm->RegisterFunction("GetTopicOwningQuest", "DbSkseFunctions", GetTopicOwningQuest);
		vm->RegisterFunction("GetTopicType", "DbSkseFunctions", GetTopicType);
		vm->RegisterFunction("SetTopicType", "DbSkseFunctions", SetTopicType);
		vm->RegisterFunction("GetTopicTypeString", "DbSkseFunctions", GetTopicTypeString);
		vm->RegisterFunction("GetTopicSubType", "DbSkseFunctions", GetTopicSubType);
		vm->RegisterFunction("SetTopicSubType", "DbSkseFunctions", SetTopicSubType);
		vm->RegisterFunction("GetTopicSubTypeString", "DbSkseFunctions", GetTopicSubTypeString);
		vm->RegisterFunction("GetTopicPriorityAndJournalIndex", "DbSkseFunctions", GetTopicPriorityAndJournalIndex);
		vm->RegisterFunction("SetTopicPriorityAndJournalIndex", "DbSkseFunctions", SetTopicPriorityAndJournalIndex);
		vm->RegisterFunction("GetNumOfTopicInfos", "DbSkseFunctions", GetNumOfTopicInfos);
		vm->RegisterFunction("GetTopicInfosInTopic", "DbSkseFunctions", GetTopicInfosInTopic);
		vm->RegisterFunction("GetTopicInfosInQuest", "DbSkseFunctions", GetTopicInfosInQuest);
		vm->RegisterFunction("GetTopicsInQuest", "DbSkseFunctions", GetTopicsInQuest);

		// TopicInfo
		vm->RegisterFunction("GetTopicInfoParent", "DbSkseFunctions", GetTopicInfoParent);
		vm->RegisterFunction("GetTopicInfoDataInfo", "DbSkseFunctions", GetTopicInfoDataInfo);
		vm->RegisterFunction("GetTopicInfoSaidOnce", "DbSkseFunctions", GetTopicInfoSaidOnce);
		vm->RegisterFunction("SetTopicInfoSaidOnce", "DbSkseFunctions", SetTopicInfoSaidOnce);
		vm->RegisterFunction("GetTopicInfoFavorLevel", "DbSkseFunctions", GetTopicInfoFavorLevel);
		vm->RegisterFunction("SetTopicInfoFavorLevel", "DbSkseFunctions", SetTopicInfoFavorLevel);
		vm->RegisterFunction("GetTopicInfoFlagSet", "DbSkseFunctions", GetTopicInfoFlagSet);
		vm->RegisterFunction("SetTopicInfoFlag", "DbSkseFunctions", SetTopicInfoFlag);

		// TopicInfo responses 
		vm->RegisterFunction("GetNumOfResponses", "DbSkseFunctions", GetNumOfResponses);
		vm->RegisterFunction("FindResponseIndexByText", "DbSkseFunctions", FindResponseIndexByText);
		vm->RegisterFunction("FindResponseIndexBySound", "DbSkseFunctions", FindResponseIndexBySound);
		vm->RegisterFunction("GetNthResponseText", "DbSkseFunctions", GetNthResponseText);
		vm->RegisterFunction("SetNthResponseText", "DbSkseFunctions", SetNthResponseText);
		vm->RegisterFunction("GetNthResponseSound", "DbSkseFunctions", GetNthResponseSound);
		vm->RegisterFunction("SetNthResponseSound", "DbSkseFunctions", SetNthResponseSound);
		vm->RegisterFunction("GetNthResponseSpeakerIdle", "DbSkseFunctions", GetNthResponseSpeakerIdle);
		vm->RegisterFunction("SetNthResponseSpeakerIdle", "DbSkseFunctions", SetNthResponseSpeakerIdle);
		vm->RegisterFunction("GetNthResponseListenerIdle", "DbSkseFunctions", GetNthResponseListenerIdle);
		vm->RegisterFunction("SetNthResponseListenerIdle", "DbSkseFunctions", SetNthResponseListenerIdle);
		vm->RegisterFunction("GetNthResponseEmotionType", "DbSkseFunctions", GetNthResponseEmotionType);
		vm->RegisterFunction("SetNthResponseEmotionType", "DbSkseFunctions", SetNthResponseEmotionType);
		vm->RegisterFunction("GetNthResponseEmotionValue", "DbSkseFunctions", GetNthResponseEmotionValue);
		vm->RegisterFunction("SetNthResponseEmotionValue", "DbSkseFunctions", SetNthResponseEmotionValue);
		return true;
    }
}

