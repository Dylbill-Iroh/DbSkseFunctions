#include "SharedVariables.h"
#include "GeneralFunctions.h"
#include "Serialization.h"
#include "Utility.h"

namespace sv {
	std::uint32_t record = 'SVr7';

	RE::ScriptEventSourceHolder* eventSourceholder = nullptr;
	RE::TESDataHandler* dataHandler = nullptr;
	RE::BSInputDeviceManager* inputManager = nullptr;
	RE::BSScript::Internal::VirtualMachine* vm = nullptr;
	RE::BSScript::IVirtualMachine* ivm = nullptr; //set bindPapyrusFunctions in plugin.cpp
	RE::SkyrimVM* skyrimVm = nullptr;
	RE::Calendar* calendar = nullptr;
	RE::UI* ui = nullptr;
	RE::UserEvents* userEvents = nullptr;
	RE::BSMusicManager* musicManager = nullptr; 
	RE::BSAudioManager* audiomanager = nullptr;
	RE::Sky* sky = nullptr;

	RE::PlayerCharacter* player = nullptr;

	int frameUpdateCount = 0;
	int iFrameUpdateInterval = 1;
	std::string lastMenuOpened = "";
	bool gamePaused = false;
	bool inMenuMode = false;
	std::chrono::system_clock::time_point lastTimeMenuWasOpened;
	std::chrono::system_clock::time_point lastTimeGameWasPaused;
	std::chrono::system_clock::time_point currentTimePoint;
	float gameTime = 0.0;
	RE::TESObjectREFR* menuRef = nullptr;
	RE::TESObjectREFR* lastPlayerActivatedRef = nullptr;
	RE::BSScript::Variable* LastPlayerMenuActivatedRefScriptProperty = nullptr;
	RE::BSScript::Variable* lastPlayerActivatedRefScriptProperty = nullptr;
	RE::TESWeather* currentWeather = nullptr;
	RE::BSIMusicType* currentBSIMusicType = nullptr;
	RE::BGSMusicType* currentBGSMusicType = nullptr;
	std::mutex updateMutex;
	std::condition_variable updateCv;

	//called on kDataLoaded in plugin.cpp
	void Install() {
		logger::info("installing shared variables");
		if (!eventSourceholder) { eventSourceholder = RE::ScriptEventSourceHolder::GetSingleton(); }
		if (!dataHandler) { dataHandler = RE::TESDataHandler::GetSingleton(); }
		if (!inputManager) { inputManager = RE::BSInputDeviceManager::GetSingleton(); }
		if (!vm) { vm = RE::BSScript::Internal::VirtualMachine::GetSingleton(); }
		
		if (!skyrimVm) { skyrimVm = RE::SkyrimVM::GetSingleton(); }
		if (!calendar) { calendar = RE::Calendar::GetSingleton(); }
		if (!musicManager) { musicManager = RE::BSMusicManager::GetSingleton(); }
		if (!ui) { ui = RE::UI::GetSingleton(); }
		if (!userEvents) { userEvents = RE::UserEvents::GetSingleton(); }
		if (!musicManager) { musicManager = RE::BSMusicManager::GetSingleton(); }
		if (!audiomanager) { audiomanager = RE::BSAudioManager::GetSingleton(); }
		if (!sky) { sky = RE::Sky::GetSingleton(); }
		if (!player) { player = RE::PlayerCharacter::GetSingleton(); }

		//if (!eventSourceholder) { logger::error("eventSourceholder not found"); }
		//if (!dataHandler) { logger::error("dataHandler not found"); }
		//if (!inputManager) { logger::error("inputManager not found"); }
		//if (!vm) { logger::error("vm not found"); }
		//if (!skyrimVm) { logger::error("skyrimVm not found"); }
		//if (!calendar) { logger::error("calendar not found"); }
		//if (!musicManager) { logger::error("musicManager not found"); }
		//if (!ui) { logger::error("ui not found"); }
		//if (!userEvents) { logger::error("userEvents not found"); }
		//if (!musicManager) { logger::error("musicManager not found"); }
		//if (!audiomanager) { logger::error("audiomanager not found"); }
		//if (!sky) { logger::error("sky not found"); }
		//if (!player) { logger::error("player not found"); } //
	} 

	void Save(SKSE::SerializationInterface* ssi) {
		if (!ssi->OpenRecord(record, 1)) {
			logger::error("Failed to open record[{}]", 'SVr7');
			return;
		}
		if (!serialize::SaveForm(menuRef, record, ssi, false)) {
			logger::error("failed to save menuRef, aborting save.");
			return;
		}
		if (!serialize::SaveForm(lastPlayerActivatedRef, record, ssi, false)) {
			logger::error("failed to save lastPlayerActivatedRef, aborting save.");
			return;
		}
	}

	void Load(SKSE::SerializationInterface* ssi) {
		Install(); //make sure all singletons are gotten.
		if (calendar) {
			gameTime = calendar->GetHoursPassed();
		}
		if (sky) {
			currentWeather = sky->currentWeather;
		}
		if (musicManager) {
			currentBSIMusicType = musicManager->current;
		}

		menuRef = serialize::LoadForm<RE::TESObjectREFR>(ssi);
		lastPlayerActivatedRef = serialize::LoadForm<RE::TESObjectREFR>(ssi);
	}

}