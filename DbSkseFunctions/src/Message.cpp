#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include "RE/B/BGSMessage.h"
#include "RE/T/TESQuest.h"
#include "RE/T/TypeTraits.h"
#include "Message.h"
#include "GeneralFunctions.h"

namespace Message {
	std::unordered_set<RE::BGSMessage::MESSAGEBOX_BUTTON*> createdMenuButtons;
	std::mutex mutex;
	
	RE::TESQuest* GetMessageOwnerQuest(RE::StaticFunctionTag*, RE::BGSMessage* akMessage){
		if (!gfuncs::IsFormValid(akMessage)){
			return nullptr; 
		}
		
		if (gfuncs::IsFormValid(akMessage->ownerQuest)){
			return akMessage->ownerQuest;
		}
		
		return nullptr;
	}
	
	bool SetMessageOwnerQuest(RE::StaticFunctionTag*, RE::BGSMessage* akMessage, RE::TESQuest* akQuest){
		if (!gfuncs::IsFormValid(akMessage)){
			return false; 
		}
		
		if (gfuncs::IsFormValid(akQuest)){
			akMessage->ownerQuest = akQuest;
			return true;
		}
		
		return false;
	}
	
	bool GetMessageIsMessageBox(RE::StaticFunctionTag*, RE::BGSMessage* akMessage){
		if (!gfuncs::IsFormValid(akMessage)){
			return false; 
		}
		
		return akMessage->flags.any(RE::BGSMessage::MessageFlag::kMessageBox);
	}
	
	bool SetMessageIsMessageBox(RE::StaticFunctionTag*, RE::BGSMessage* akMessage, bool abSet){
		if (!gfuncs::IsFormValid(akMessage)){
			return false; 
		}
		
		if (abSet){
			akMessage->flags.set(RE::BGSMessage::MessageFlag::kMessageBox);
		}
		else {
			akMessage->flags.reset(RE::BGSMessage::MessageFlag::kMessageBox);
		}
		
		return akMessage->flags.any(RE::BGSMessage::MessageFlag::kMessageBox) == abSet;
	}
	
	bool GetMessageAutoDisplays(RE::StaticFunctionTag*, RE::BGSMessage* akMessage){
		if (!gfuncs::IsFormValid(akMessage)){
			return false; 
		}
		
		return akMessage->flags.any(RE::BGSMessage::MessageFlag::kInitialDelay);
	}
	
	bool SetMessageAutoDisplays(RE::StaticFunctionTag*, RE::BGSMessage* akMessage, bool abSet){
		if (!gfuncs::IsFormValid(akMessage)){
			return false; 
		}
		
		if (abSet){
			akMessage->flags.set(RE::BGSMessage::MessageFlag::kInitialDelay);
		}
		else {
			akMessage->flags.reset(RE::BGSMessage::MessageFlag::kInitialDelay);
		}
		
		return akMessage->flags.any(RE::BGSMessage::MessageFlag::kInitialDelay) == abSet;
	} 
	
	int GetMessageDisplayTime(RE::StaticFunctionTag*, RE::BGSMessage* akMessage){
		if (!gfuncs::IsFormValid(akMessage)){
			return -1; 
		}
		
		return akMessage->displayTime;
	}
	
	bool SetMessageDisplayTime(RE::StaticFunctionTag*, RE::BGSMessage* akMessage, int displayTime){
		if (!gfuncs::IsFormValid(akMessage)){
			return false; 
		}
		
		akMessage->displayTime = displayTime;
		
		return akMessage->displayTime == displayTime;
	}
	
	int GetNumOfMessageMenuButtons(RE::StaticFunctionTag*, RE::BGSMessage* akMessage){
		if (!gfuncs::IsFormValid(akMessage)){
			return -1;
		}

		return static_cast<int>(akMessage->menuButtons.size());
	}
	
	std::vector<std::string> GetMessageMenuButtons(RE::StaticFunctionTag*, RE::BGSMessage* akMessage){
		std::vector<std::string> v;
		if (!gfuncs::IsFormValid(akMessage)){
			return v; 
		}
		
		for (auto* button : akMessage->menuButtons){
			if (button){
				v.push_back(static_cast<std::string>(button->text));
			}
			else {
				v.push_back(""); //keeps correct amount of buttons in array
			}
		}
		
		return v;
	}
	
	// Frees only the buttons this plugin allocated. Engine-authored buttons come from the
	// plugin files and must never be deleted.
	// CALLER MUST HOLD mutex -- touches createdMenuButtons.
	void FreeCreatedButtons(const std::vector<RE::BGSMessage::MESSAGEBOX_BUTTON*>& removed) {
		for (auto* button : removed) {
			if (!button) {
				continue;
			}

			auto it = createdMenuButtons.find(button);
			if (it != createdMenuButtons.end()) {
				createdMenuButtons.erase(it);
				delete button;
			}
		}
	}

	void SetMessageMenuButtons(RE::StaticFunctionTag*, RE::BGSMessage* akMessage, std::vector<std::string> buttons) {
		if (!gfuncs::IsFormValid(akMessage)) {
			return;
		}

		std::lock_guard<std::mutex> lock(mutex);

		auto& list = akMessage->menuButtons;

		using ButtonList = std::remove_reference_t<decltype(list)>;
		using Node       = ButtonList::Node;
		using SizeType   = ButtonList::size_type;

		const std::size_t newSize = buttons.size();

		// ---- nothing requested: drop everything ----
		if (newSize == 0) {
			std::vector<RE::BGSMessage::MESSAGEBOX_BUTTON*> removed;
			for (auto* button : list) {
				if (button) {
					removed.push_back(button);
				}
			}

			list.clear();   // frees every node but the in-place head, and resets the head item
			FreeCreatedButtons(removed);
			return;
		}

		// ---- 1. overwrite the slots that already exist ----
		std::size_t i = 0;

		if (!list.empty()) {
			for (auto it = list.begin(); i < newSize && it != list.end(); i++, ++it) {
				auto*& slot = *it;   // reference to the element, so assigning replaces it in place

				if (slot) {
					slot->text = buttons[i];
				}
				else {
					auto* newButton = new RE::BGSMessage::MESSAGEBOX_BUTTON();
					newButton->text = buttons[i];
					slot = newButton;            // fill the hole -- do NOT insert_at here
					createdMenuButtons.insert(newButton);
				}
			}
		}

		// ---- 2. append the extras ----
		if (i < newSize) {
			SizeType appendAt = list.size();

			for (; i < newSize; i++, appendAt++) {
				auto* newButton = new RE::BGSMessage::MESSAGEBOX_BUTTON();
				newButton->text = buttons[i];
				list.insert_at(appendAt, newButton);
				createdMenuButtons.insert(newButton);
			}
		}
		// ---- 3. drop the tail ----
		else if (list.size() > newSize) {
			// resize() is unusable (see above), so unlink the tail by hand.
			// Node is { value_type item; Node* next; } with item at offset 0, so an
			// element's address is its node's address.
			Node* keep = reinterpret_cast<Node*>(std::addressof(*list.begin()));

			for (std::size_t n = 1; n < newSize && keep->next; n++) {
				keep = keep->next;
			}

			Node* node = keep->next;
			keep->next = nullptr;

			std::vector<RE::BGSMessage::MESSAGEBOX_BUTTON*> removed;

			while (node) {
				Node* next = node->next;
				if (node->item) {
					removed.push_back(node->item);
				}
				delete node;
				node = next;
			}

			FreeCreatedButtons(removed);
		}
	}
	
	//swap the actual MESSAGEBOX_BUTTONs, putting the button at indexA in the indexB slot and vice versa
	bool SwapMessageMenuButtons(RE::StaticFunctionTag*, RE::BGSMessage* akMessage, int indexA, int indexB){
		if (!gfuncs::IsFormValid(akMessage)){
			return false; 
		}

		std::lock_guard<std::mutex> lock(mutex);

		auto& list = akMessage->menuButtons;

		using SizeType = std::remove_reference_t<decltype(list)>::size_type;

		const auto size = list.size();

		if (indexA < 0 || static_cast<SizeType>(indexA) >= size){
			return false;
		}

		if (indexB < 0 || static_cast<SizeType>(indexB) >= size){
			return false;
		}

		if (indexA == indexB){
			return true;   // already where it belongs -- nothing to do
		}

		auto itA = list[static_cast<SizeType>(indexA)];
		auto itB = list[static_cast<SizeType>(indexB)];

		if (itA == list.end() || itB == list.end()){
			return false;   // operator[] returns end() on overrun; shouldn't happen after the checks above
		}

		// The elements are pointers, so this exchanges which slot each button sits in
		// without touching the nodes or the ownership set.
		std::swap(*itA, *itB);

		return true;
	}
	
	std::string GetNthMessageMenuButtonText(RE::StaticFunctionTag*, RE::BGSMessage* akMessage, int nthIndex){
		if (!gfuncs::IsFormValid(akMessage)){
			return ""; 
		}
		
		if (nthIndex < 0 || nthIndex >= akMessage->menuButtons.size() || akMessage->menuButtons.empty()) {
			return "";
		}
		
		auto* button = *akMessage->menuButtons[nthIndex];
		if (button){
			return static_cast<std::string>(button->text);
		}
		
		return "";
	}
	
	bool SetNthMessageMenuButtonText(RE::StaticFunctionTag*, RE::BGSMessage* akMessage, int nthIndex, std::string text){
		if (!gfuncs::IsFormValid(akMessage)){
			return false; 
		}
		
		if (nthIndex < 0 || nthIndex >= akMessage->menuButtons.size() || akMessage->menuButtons.empty()) {
			return false;
		}
		
		auto* button = *akMessage->menuButtons[nthIndex];
		if (button){
			button->text = text;
			return static_cast<std::string>(button->text) == text;
		}
		
		return false;
	}
	
	int GetMessageMenuButtonIndex(RE::StaticFunctionTag*, RE::BGSMessage* akMessage, std::string text){
		if (!gfuncs::IsFormValid(akMessage) || text.empty()){
			return -1; 
		}
		
		auto* cText = text.c_str();
		
		int i = 0;
		for (auto* button : akMessage->menuButtons){
			if (button){
				if (!button->text.empty()){
					if (_stricmp(button->text.c_str(), cText) == 0){
						return i;
					}
				}
			}
			
			i++;
		}
		
		return -1;
	}
	
	bool InsertMessageMenuButtonAt(RE::StaticFunctionTag*, RE::BGSMessage* akMessage, int nthIndex, std::string text){
		if (!gfuncs::IsFormValid(akMessage)){
			return false; 
		}
		
		std::lock_guard<std::mutex> lock(mutex); 
		
		RE::BGSMessage::MESSAGEBOX_BUTTON* button = new RE::BGSMessage::MESSAGEBOX_BUTTON();
		button->text = text;
		
		if (nthIndex < 0 || nthIndex >= akMessage->menuButtons.size()){
			akMessage->menuButtons.insert_at(akMessage->menuButtons.size(), button);
		}
		else {
			akMessage->menuButtons.insert_at(nthIndex, button);
		}
		
		createdMenuButtons.insert(button);
		
		return true;
	}
	
	bool RemoveMessageMenuButtonAt(RE::StaticFunctionTag*, RE::BGSMessage* akMessage, int nthIndex) {
		if (!gfuncs::IsFormValid(akMessage)) {
			return false;
		}

		if (nthIndex < 0 || nthIndex >= akMessage->menuButtons.size() || akMessage->menuButtons.empty()) {
			return false;
		}
		
		std::lock_guard<std::mutex> lock(mutex); 

		auto& buttons = akMessage->menuButtons;

		auto* button = *akMessage->menuButtons[nthIndex];
		
		using ButtonList = std::remove_reference_t<decltype(buttons)>;
		using Node = ButtonList::Node;

		// Node is { value_type item; Node* next; } with item at offset 0, so an element's
		// address is its node's address. erase_after can't be reached from outside the class
		// (begin() const doesn't instantiate) and the 1-arg form is a no-op anyway.
		Node* prev = nullptr;
		Node* node = reinterpret_cast<Node*>(std::addressof(*buttons.begin()));

		for (int i = 0; i < nthIndex; i++) {
			if (!node->next) {
				return false;   // nthIndex is past the end
			}
			prev = node;
			node = node->next;
		}

		if (!prev) {
			// The head node lives inside the list by value, so it can't be deleted.
			if (!node->next) {
				buttons.clear();     // only element -- clear() destroys AND reconstructs the head item
			}
			else {
				buttons.pop_front(); // moves node 1's item into the head and frees node 1
			}
		}
		else {
			prev->next = node->next;
			delete node;             // we own the node -- insert_at allocated it with new
		}

		// Only free the button itself if this plugin allocated it. Engine-authored buttons
		// come from the plugin files and must never be deleted.
		auto it = createdMenuButtons.find(button);
		if (it != createdMenuButtons.end()){
			createdMenuButtons.erase(it);
			delete button;
		}

		return true;
	}
	
	
	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
		vm->RegisterFunction("GetMessageOwnerQuest", "DbSkseFunctions", GetMessageOwnerQuest);
		vm->RegisterFunction("SetMessageOwnerQuest", "DbSkseFunctions", SetMessageOwnerQuest);
		vm->RegisterFunction("GetMessageIsMessageBox", "DbSkseFunctions", GetMessageIsMessageBox);
		vm->RegisterFunction("SetMessageIsMessageBox", "DbSkseFunctions", SetMessageIsMessageBox);
		vm->RegisterFunction("GetMessageAutoDisplays", "DbSkseFunctions", GetMessageAutoDisplays);
		vm->RegisterFunction("SetMessageAutoDisplays", "DbSkseFunctions", SetMessageAutoDisplays);
		vm->RegisterFunction("GetMessageDisplayTime", "DbSkseFunctions", GetMessageDisplayTime);
		vm->RegisterFunction("SetMessageDisplayTime", "DbSkseFunctions", SetMessageDisplayTime);
		vm->RegisterFunction("GetNumOfMessageMenuButtons", "DbSkseFunctions", GetNumOfMessageMenuButtons);
		vm->RegisterFunction("GetMessageMenuButtons", "DbSkseFunctions", GetMessageMenuButtons);
		vm->RegisterFunction("SetMessageMenuButtons", "DbSkseFunctions", SetMessageMenuButtons);
		vm->RegisterFunction("GetNthMessageMenuButtonText", "DbSkseFunctions", GetNthMessageMenuButtonText);
		vm->RegisterFunction("SetNthMessageMenuButtonText", "DbSkseFunctions", SetNthMessageMenuButtonText);
		vm->RegisterFunction("GetMessageMenuButtonIndex", "DbSkseFunctions", GetMessageMenuButtonIndex);
		vm->RegisterFunction("InsertMessageMenuButtonAt", "DbSkseFunctions", InsertMessageMenuButtonAt);
		vm->RegisterFunction("RemoveMessageMenuButtonAt", "DbSkseFunctions", RemoveMessageMenuButtonAt);
		vm->RegisterFunction("SwapMessageMenuButtons", "DbSkseFunctions", SwapMessageMenuButtons);

		return true;
	}
}