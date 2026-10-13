#include "StoryManager.h"
#include "GeneralFunctions.h"
#include "RE/A/Actor.h"
#include "RE/B/BGSStoryManagerNodeBase.h"
#include "RE/B/BGSStoryManagerTreeForm.h"
#include "RE/B/BSCoreTypes.h"
#include "RE/F/FormTypes.h"
#include "RE/T/TypeTraits.h"
#include <string.h>
// #include <msxml.h>

// class BGSStoryManagerTreeForm : public TESForm //no form type
// class BGSStoryManagerNodeBase : public BGSStoryManagerTreeForm //no form type
// class BGSStoryManagerQuestNode : public BGSStoryManagerNodeBase //form type StoryManagerQuestNode
// class BGSStoryManagerBranchNode : public BGSStoryManagerNodeBase //form type StoryManagerBranchNode
// class BGSStoryManagerEventNode : public BGSStoryManagerBranchNode //form type StoryManagerEventNode 

namespace StoryManager {
	std::vector<RE::BGSStoryManagerEventNode*> GetAllStoryManagerEventNodes(RE::StaticFunctionTag*);
	std::vector<RE::BGSStoryManagerBranchNode*> GetAllStoryManagerBranchNodes(RE::StaticFunctionTag*, std::string storyManagerEventNodeName);
	std::vector<RE::BGSStoryManagerQuestNode*> GetAllStoryManagerQuestNodes(RE::StaticFunctionTag*, std::string storyManagerEventNodeName);
	
	bool InitStoryManagerCalled = false;

	//Getting all nodes first is necssary for GetParent and GetParents functions to work.
	void InitStoryManager(){
		if (InitStoryManagerCalled){
			return;
		}
		
		InitStoryManagerCalled = true;
		
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player){
			logger::warn("player character not found");
			return;
		}
		
		auto* playerActor = player->As<RE::Actor>();
		if (!playerActor){
			logger::warn("player actor not found");
			return;
		}
		
		if (!gfuncs::IsScriptAttachedToForm(playerActor, "StoryManagerInitializer")){
			logger::debug("attaching StoryManagerInitializer to player");
			gfuncs::AttachPapyrusScriptToForm(playerActor, "StoryManagerInitializer");
		}
		else {
			logger::debug("StoryManagerInitializer already attached to player");
		}
		
		// must be called from papyrus
		// auto allEventNodes = GetAllStoryManagerEventNodes(nullptr);
		// auto allBranchNodes = GetAllStoryManagerBranchNodes(nullptr, "");
		// auto allQuestNodes = GetAllStoryManagerQuestNodes(nullptr, "");
		
		// logger::debug("allEventNodes size[{}] allBranchNodes size[{}] allQuestNodes size[{}]", 
		// 	allEventNodes.size(), allBranchNodes.size(), allQuestNodes.size()
		// );
	}

	void LogTreeNodeTypes(RE::BGSStoryManagerTreeForm* node){
		if (!node){
			return; 
		}
		
			// auto* treeNode = skyrim_cast<RE::BGSStoryManagerTreeForm*>(node);
			// auto* baseNode = skyrim_cast<RE::BGSStoryManagerNodeBase*>(node);
			// auto* questNode = skyrim_cast<RE::BGSStoryManagerQuestNode*>(node);
			// auto* branchNode = skyrim_cast<RE::BGSStoryManagerBranchNode*>(node);
			// auto* eventNode = skyrim_cast<RE::BGSStoryManagerEventNode*>(node);
			
			auto* treeNode = node->As<RE::BGSStoryManagerTreeForm>();
			auto* baseNode = node->As<RE::BGSStoryManagerNodeBase>();
			auto* questNode = node->As<RE::BGSStoryManagerQuestNode>();
			auto* branchNode = node->As<RE::BGSStoryManagerBranchNode>();
			auto* eventNode = node->As<RE::BGSStoryManagerEventNode>();
			
			bool treeNodeFound = treeNode != nullptr;
			bool baseNodeFound = baseNode != nullptr;
			bool questNodeFound = questNode != nullptr;
			bool branchNodeFound = branchNode != nullptr;
			bool eventNodeFound = eventNode != nullptr;
			
			RE::BGSStoryManagerBranchNode* parentNode = baseNodeFound ? baseNode->parent : nullptr;
			std::string sParentNode = parentNode ? gfuncs::GetFormNameAndId(parentNode) : "not found";
			
			logger::info("node QChildCount[{}] treeNodeFound[{}] baseNodeFound[{}] questNodeFound[{}] branchNodeFound[{}] eventNodeFound[{}] parentNode[{}]", 
				node->QChildCount(), treeNodeFound, baseNodeFound, questNodeFound, branchNodeFound, eventNodeFound, sParentNode
			);
	}
	
// StoryManagerNodeBase.psc functions ====================================================================================================
	
	// -1 = none, 0 = StoryManagerEventNode, 1 = StoryManagerBranchNode, 2 = StoryManagerQuestNode. 
	int GetStoryManagerNodeBaseType(RE::BGSStoryManagerNodeBase* self){
		if (!gfuncs::IsFormValid(self)){
			return -1;
		}
		
		auto* eventNode = self->As<RE::BGSStoryManagerEventNode>();
		if (gfuncs::IsFormValid(eventNode)){
			return 0;
		}
		
		auto* branchNode = self->As<RE::BGSStoryManagerBranchNode>();
		if (gfuncs::IsFormValid(branchNode)){
			return 1;
		}
		
		auto* questNode = self->As<RE::BGSStoryManagerQuestNode>();
		if (gfuncs::IsFormValid(questNode)){
			return 2;
		}
		
		return -1;
	}
	
	RE::BGSStoryManagerEventNode* GetEventNodeForNodeBase(RE::BGSStoryManagerNodeBase* self){
		if (!gfuncs::IsFormValid(self)){
			return nullptr;
		}
		
		RE::BGSStoryManagerEventNode* eventNode = self->As<RE::BGSStoryManagerEventNode>();
		if (!gfuncs::IsFormValid(eventNode)){
			auto* parent = self->parent; 
			int count = 0; 
			while (!eventNode && parent && count < 64){
				eventNode = parent->As<RE::BGSStoryManagerEventNode>();
				parent = parent->parent;
				count++;
			}
		}
		
		return eventNode;
	}
	
	bool IsNodeBaseInStoryManagerEventNode(RE::BGSStoryManagerNodeBase* node, const char* eventName){
		// if (!node){
		// 	return false;
		// }
		
		auto* eventNode = GetEventNodeForNodeBase(node);
		if (eventNode){
			if (eventNode->event){
				if (_stricmp(eventName, eventNode->event->name.c_str()) == 0){
					return true;
				}
			}
		}
		
		return false;
	}
	
	RE::BGSStoryManagerBranchNode* GetStoryManagerBaseTypeParent(RE::BGSStoryManagerNodeBase* self){
		InitStoryManager(); //necessary for self->parent to initialize
		
		if (!gfuncs::IsFormValid(self)){
			return nullptr;
		}
		
		RE::BGSStoryManagerEventNode* eventNode = self->As<RE::BGSStoryManagerEventNode>();
		if (eventNode){ //event nodes are the top level node, ensure no parent branch is returned above this.
			return nullptr;
		}
		
		return self->parent;
	}
	
	std::vector<RE::BGSStoryManagerBranchNode*> GetStoryManagerBaseTypeParents(RE::BGSStoryManagerNodeBase* self){
		InitStoryManager(); //necessary for self->parent to initialize
		
		std::vector<RE::BGSStoryManagerBranchNode*> v;
		if (!gfuncs::IsFormValid(self)){
			return v; 
		}
		
		RE::BGSStoryManagerEventNode* eventNode = self->As<RE::BGSStoryManagerEventNode>();
		if (eventNode){ //event nodes are the top level node, ensure no parent branch is returned above this.
			return v;
		}
		
		RE::BGSStoryManagerBranchNode* parent = self->parent;
		int count = 0;
		while (parent && count < 64){
			v.push_back(parent);
			eventNode = parent->As<RE::BGSStoryManagerEventNode>();
			if (eventNode){
				break;
			}
			parent = parent->parent;
			count++;
		}
		
		return v;
	}
	
	bool GetIsStoryManagerNodeBaseRandom(RE::BGSStoryManagerNodeBase* self){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		return self->flags.nodeFlags.any(RE::BGSStoryManagerNodeBase::Flags::NodeFlag::kRandom);
	}
	
	bool SetIsStoryManagerNodeBaseRandom(RE::BGSStoryManagerNodeBase* self, bool isRandom){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		if (self->flags.nodeFlags.any(RE::BGSStoryManagerNodeBase::Flags::NodeFlag::kRandom) != isRandom){
			if (isRandom){
				self->flags.nodeFlags.set(RE::BGSStoryManagerNodeBase::Flags::NodeFlag::kRandom);
			}
			else {
				self->flags.nodeFlags.reset(RE::BGSStoryManagerNodeBase::Flags::NodeFlag::kRandom);
			}
		}
		
		return self->flags.nodeFlags.any(RE::BGSStoryManagerNodeBase::Flags::NodeFlag::kRandom) == isRandom;
	}
	
	bool GetStoryManagerNodeBaseWarnIfNoChildQuestStarted(RE::BGSStoryManagerNodeBase* self){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		return self->flags.nodeFlags.any(RE::BGSStoryManagerNodeBase::Flags::NodeFlag::kWarnIfNoChildQuestStarted);
	}
	
	bool SetStoryManagerNodeBaseWarnIfNoChildQuestStarted(RE::BGSStoryManagerNodeBase* self, bool set){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		if (self->flags.nodeFlags.any(RE::BGSStoryManagerNodeBase::Flags::NodeFlag::kWarnIfNoChildQuestStarted) != set){
			if (set){
				self->flags.nodeFlags.set(RE::BGSStoryManagerNodeBase::Flags::NodeFlag::kWarnIfNoChildQuestStarted);
			}
			else {
				self->flags.nodeFlags.reset(RE::BGSStoryManagerNodeBase::Flags::NodeFlag::kWarnIfNoChildQuestStarted);
			}
		}
		
		return self->flags.nodeFlags.any(RE::BGSStoryManagerNodeBase::Flags::NodeFlag::kWarnIfNoChildQuestStarted) == set;
	}
	
// StoryManagerQuestNode.psc functions ====================================================================================================
	
	std::vector<RE::BGSStoryManagerQuestNode*> GetAllStoryManagerQuestNodes(RE::StaticFunctionTag*, std::string storyManagerEventNodeName){
		std::vector<RE::BGSStoryManagerQuestNode*> v;
		
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			return v;
		}
		
		auto& eventNodeArray = dh->GetFormArray<RE::BGSStoryManagerQuestNode>();
		
		const char* eventName = storyManagerEventNodeName.c_str();
		
		logger::trace("eventNodeArray size[{}]", eventNodeArray.size());
		for (auto* node : eventNodeArray){
			if (gfuncs::IsFormValid(node)){
				if (!storyManagerEventNodeName.empty()){
					if (!IsNodeBaseInStoryManagerEventNode(node, eventName)){
						continue;
					}
				}
				v.push_back(node);
			}
		}
		
		return v;
	}

	std::vector<RE::BGSStoryManagerQuestNode*> GetStoryManagerQuestNodesForQuest(RE::StaticFunctionTag*, RE::TESQuest* quest) {
		std::vector<RE::BGSStoryManagerQuestNode*> v;
		if (!gfuncs::IsFormValid(quest)) {
			return v;
		}
		
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			return v;
		}

		auto& array = dh->GetFormArray<RE::BGSStoryManagerQuestNode>();
		
		for (auto* node : array) {
			if (!node) {
				continue;
			}
			for (auto* q : node->quests) {
				if (q == quest) {
					v.push_back(node);
					break;
				}
			}
		}
		return v;
	}

	std::vector<RE::TESQuest*> GetStoryManagerQuestNodeQuests(RE::BGSStoryManagerQuestNode* self){
		std::vector<RE::TESQuest*> v; 
		if (!gfuncs::IsFormValid(self)){
			return v;
		}
		
		for (auto* quest : self->quests) {
			if (gfuncs::IsFormValid(quest)){
				v.push_back(quest);
			}
		}
		
		return v;
	}
	
	bool GetStoryManagerQuestNodeDoAllBeforeRepeating(RE::BGSStoryManagerQuestNode* self){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		return self->flags.questFags.any(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kDoAllBeforeRepeating);
	}
	
	bool SetStoryManagerQuestNodeDoAllBeforeRepeating(RE::BGSStoryManagerQuestNode* self, bool set){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		if (self->flags.questFags.any(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kDoAllBeforeRepeating) != set){
			if (set){
				self->flags.questFags.set(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kDoAllBeforeRepeating);
			}
			else {
				self->flags.questFags.reset(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kDoAllBeforeRepeating);
			}
		}
		
		return self->flags.questFags.any(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kDoAllBeforeRepeating) == set;
	}
	
	bool GetStoryManagerQuestNodeSharesEvent(RE::BGSStoryManagerQuestNode* self){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		return self->flags.questFags.any(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kSharesEvent);
	}
	
	bool SetStoryManagerQuestNodeSharesEvent(RE::BGSStoryManagerQuestNode* self, bool set){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		if (self->flags.questFags.any(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kSharesEvent) != set){
			if (set){
				self->flags.questFags.set(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kSharesEvent);
			}
			else {
				self->flags.questFags.reset(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kSharesEvent);
			}
		}
		
		return self->flags.questFags.any(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kSharesEvent) == set;
	}
	
	bool GetStoryManagerQuestNodeNumQuestsToRunChecked(RE::BGSStoryManagerQuestNode* self){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		return self->flags.questFags.any(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kNumQuestsToRun);
	}
	
	bool SetStoryManagerQuestNodeNumQuestsToRunChecked(RE::BGSStoryManagerQuestNode* self, bool set){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		if (self->flags.questFags.any(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kNumQuestsToRun) != set){
			if (set){
				self->flags.questFags.set(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kNumQuestsToRun);
			}
			else {
				self->flags.questFags.reset(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kNumQuestsToRun);
			}
		}
		
		return self->flags.questFags.any(RE::BGSStoryManagerNodeBase::Flags::QuestFlag::kNumQuestsToRun) == set;
	}
	
	int GetStoryManagerQuestNodeNumQuestsToRun(RE::BGSStoryManagerQuestNode* self){
		if (!gfuncs::IsFormValid(self)){
			return 0;
		}
		
		return self->numQuestsToStart;
	}
	
	bool SetStoryManagerQuestNodeNumQuestsToRun(RE::BGSStoryManagerQuestNode* self, int toRun){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		if (toRun < 0){
			return false;
		}
		
		self->numQuestsToStart = toRun;
		
		return self->numQuestsToStart == toRun;
	}
	
	int GetStoryManagerQuestNodeMaxConcurrentQuests(RE::BGSStoryManagerQuestNode* self){
		if (!gfuncs::IsFormValid(self)){
			return 0;
		}
		
		return self->maxQuests;
	}
	
	bool SetStoryManagerQuestNodeMaxConcurrentQuests(RE::BGSStoryManagerQuestNode* self, int maxQuests){
		if (!gfuncs::IsFormValid(self)){
			return false;
		}
		
		if (maxQuests < 0){
			return false;
		}
		
		self->maxQuests = maxQuests;
		
		return self->maxQuests == maxQuests;
	}
	
// StoryManagerBranchNode ====================================================================================================

	std::vector<RE::BGSStoryManagerBranchNode*> GetAllStoryManagerBranchNodes(RE::StaticFunctionTag*, std::string storyManagerEventNodeName){
		std::vector<RE::BGSStoryManagerBranchNode*> v;
		
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			return v;
		}
		
		auto& eventNodeArray = dh->GetFormArray<RE::BGSStoryManagerBranchNode>();
		
		const char* eventName = storyManagerEventNodeName.c_str();
		
		logger::trace("eventNodeArray size[{}]", eventNodeArray.size());
		for (auto* node : eventNodeArray){
			if (gfuncs::IsFormValid(node)){
				if (!storyManagerEventNodeName.empty()){
					if (!IsNodeBaseInStoryManagerEventNode(node, eventName)){
						continue;
					}
				}
				v.push_back(node);
			}
		}
		
		return v;
	}
	
	int GetStoryManagerBranchNodeNumOfChildren(RE::TESForm* self){
		logger::trace(""); 
		if (!gfuncs::IsFormValid(self)){
			return 0;
		}
		
		// if (!self){
		// 	return 0;
		// }
		
		RE::BGSStoryManagerBranchNode* node = self->As<RE::BGSStoryManagerBranchNode>();
		if (!node){
			return 0;
		}
		
		return node->children.size();
	}
	
	std::vector<RE::TESForm*> GetStoryManagerBranchNodeChildren(RE::TESForm* self){
		logger::trace("");
		
		std::vector<RE::TESForm*> v;
		// if (!gfuncs::IsFormValid(self)){
		// 	return v;
		// }
		
		if (!self){
			return v;
		}
		
		RE::BGSStoryManagerBranchNode* node = self->As<RE::BGSStoryManagerBranchNode>();
		if (!node){
			return v;
		}
		
		for (auto* nodeBase : node->children){
			if (gfuncs::IsFormValid(nodeBase)){
				v.push_back(nodeBase);
			}
		}
		
		return v;
	}
	
// StoryManagerEventNode ====================================================================================================
	
	std::vector<RE::BGSStoryManagerEventNode*> GetAllStoryManagerEventNodes(RE::StaticFunctionTag*){
		std::vector<RE::BGSStoryManagerEventNode*> v;
		
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			return v;
		}
		
		auto& eventNodeArray = dh->GetFormArray<RE::BGSStoryManagerEventNode>(); 
		logger::trace("eventNodeArray size[{}]", eventNodeArray.size());
		for (auto* node : eventNodeArray){
			if (gfuncs::IsFormValid(node)){
				v.push_back(node);
			}
		}
		
		return v;
	}

	RE::BGSStoryManagerEventNode* GetStoryManagerEventNode(RE::StaticFunctionTag*, std::string nodeName){
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			return nullptr;
		}
		
		auto& array = dh->GetFormArray<RE::BGSStoryManagerEventNode>();
		
		auto* name = nodeName.c_str();
		
		for (auto* eventNode : array){
			if (gfuncs::IsFormValid(eventNode)){
				if (eventNode->event){
					if (_stricmp(eventNode->event->name.c_str(), name) == 0){
						return eventNode;
					}
				}
			}
		}
		
		return nullptr;
	}

	std::string GetStoryManagerEventNodeEventName(RE::BGSStoryManagerEventNode* self){
		if (!gfuncs::IsFormValid(self)){
			return "";
		}
		
		if (self->event){
			return (std::string)self->event->name;
		}
		
		return "";
	}
	
	int GetStoryManagerEventNodeNumOfMembers(RE::BGSStoryManagerEventNode* self){
		if (!gfuncs::IsFormValid(self)){
			return 0;
		}
		
		if (self->event){
			if (self->event->members){
				return self->event->members->size();
			}
		}
		
		return 0;
	}
	
	int GetStoryManagerEventNodeNthMemberType(RE::BGSStoryManagerEventNode* self, int nthIndex){
		if (!gfuncs::IsFormValid(self)){
			return -1;
		}
		
		
		if (self->event){
			if (self->event->members){
				if (nthIndex < 0 || nthIndex >= self->event->members->size()){
					return -1;
				}
				auto type = (*self->event->members)[nthIndex].type.get();
				return static_cast<int>(type);
			}
		}
		
		return -1;
	}
	
	std::string GetStoryManagerEventNodeNthMemberName(RE::BGSStoryManagerEventNode* self, int nthIndex){
		if (!gfuncs::IsFormValid(self)){
			return "";
		}
		
		if (self->event){
			if (self->event->members){
				if (nthIndex < 0 || nthIndex >= self->event->members->size()){
					return "";
				}
				auto name = (*self->event->members)[nthIndex].name;
				return (std::string)name;
			}
		}
		
		return "";
	}

// ====================================================================================================
	
    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
		RE::VMTypeID id = static_cast<RE::VMTypeID>(RE::FormType::StoryManagerQuestNode);
		gfuncs::RegisterObjectType(vm, id, "StoryManagerQuestNode");
		
		id = static_cast<RE::VMTypeID>(RE::FormType::StoryManagerBranchNode);
		gfuncs::RegisterObjectType(vm, id, "StoryManagerBranchNode");
		
		id = static_cast<RE::VMTypeID>(RE::FormType::StoryManagerEventNode);
		gfuncs::RegisterObjectType(vm, id, "StoryManagerEventNode");
		
	//StoryManagerNodeBase =========================================================================================================================================================
		vm->RegisterFunction("GetType", "StoryManagerNodeBase", GetStoryManagerNodeBaseType);
        vm->RegisterFunction("GetParent", "StoryManagerNodeBase", GetStoryManagerBaseTypeParent);
        vm->RegisterFunction("GetParents", "StoryManagerNodeBase", GetStoryManagerBaseTypeParents);
		vm->RegisterFunction("GetEventNodeParent", "StoryManagerNodeBase", GetEventNodeForNodeBase);
        vm->RegisterFunction("GetIsRandom", "StoryManagerNodeBase", GetIsStoryManagerNodeBaseRandom);
        vm->RegisterFunction("SetIsRandom", "StoryManagerNodeBase", SetIsStoryManagerNodeBaseRandom);
        vm->RegisterFunction("GetWarnIfNoChildQuestStarted", "StoryManagerNodeBase", GetStoryManagerNodeBaseWarnIfNoChildQuestStarted);
        vm->RegisterFunction("SetWarnIfNoChildQuestStarted", "StoryManagerNodeBase", SetStoryManagerNodeBaseWarnIfNoChildQuestStarted);
		
	//StoryManagerQuestNode =========================================================================================================================================================
		vm->RegisterFunction("GetAllNodes", "StoryManagerQuestNode", GetAllStoryManagerQuestNodes);
		vm->RegisterFunction("GetQuestNodes", "StoryManagerQuestNode", GetStoryManagerQuestNodesForQuest);
		vm->RegisterFunction("GetQuests", "StoryManagerQuestNode", GetStoryManagerQuestNodeQuests);
		vm->RegisterFunction("DoAllBeforeRepeating", "StoryManagerQuestNode", GetStoryManagerQuestNodeDoAllBeforeRepeating);
		vm->RegisterFunction("SetDoAllBeforeRepeating", "StoryManagerQuestNode", SetStoryManagerQuestNodeDoAllBeforeRepeating);
		vm->RegisterFunction("SharesEvent", "StoryManagerQuestNode", GetStoryManagerQuestNodeSharesEvent);
		vm->RegisterFunction("SetSharesEvent", "StoryManagerQuestNode", SetStoryManagerQuestNodeSharesEvent);
		vm->RegisterFunction("NumQuestsToRunChecked", "StoryManagerQuestNode", GetStoryManagerQuestNodeNumQuestsToRunChecked);
		vm->RegisterFunction("SetNumQuestsToRunChecked", "StoryManagerQuestNode", SetStoryManagerQuestNodeNumQuestsToRunChecked);
		vm->RegisterFunction("GetNumQuestsToRun", "StoryManagerQuestNode", GetStoryManagerQuestNodeNumQuestsToRun);
		vm->RegisterFunction("SetNumQuestsToRun", "StoryManagerQuestNode", SetStoryManagerQuestNodeNumQuestsToRun);
		vm->RegisterFunction("GetMaxConcurrentQuests", "StoryManagerQuestNode", GetStoryManagerQuestNodeMaxConcurrentQuests);
		vm->RegisterFunction("SetMaxConcurrentQuests", "StoryManagerQuestNode", SetStoryManagerQuestNodeMaxConcurrentQuests);
		
	// StoryManagerBranchNode =========================================================================================================================================================
		vm->RegisterFunction("GetAllNodes", "StoryManagerBranchNode", GetAllStoryManagerBranchNodes);
		vm->RegisterFunction("GetNumOfChildren", "StoryManagerBranchNode", GetStoryManagerBranchNodeNumOfChildren);
		vm->RegisterFunction("GetChildrenForms", "StoryManagerBranchNode", GetStoryManagerBranchNodeChildren);
	
	// StoryManagerEventNode =========================================================================================================================================================
		vm->RegisterFunction("GetAllEventNodes", "StoryManagerEventNode", GetAllStoryManagerEventNodes);
		vm->RegisterFunction("GetEventNode", "StoryManagerEventNode", GetStoryManagerEventNode);
		vm->RegisterFunction("GetEventName", "StoryManagerEventNode", GetStoryManagerEventNodeEventName);
		vm->RegisterFunction("GetNumberOfMembers", "StoryManagerEventNode", GetStoryManagerEventNodeNumOfMembers);
		vm->RegisterFunction("GetNthMemberType", "StoryManagerEventNode", GetStoryManagerEventNodeNthMemberType);
		vm->RegisterFunction("GetNthMemberName", "StoryManagerEventNode", GetStoryManagerEventNodeNthMemberName);
		
		return true;
	}
}

