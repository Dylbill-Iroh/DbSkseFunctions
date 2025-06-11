#pragma once

namespace sv {
	extern std::uint32_t record;

	//singletons
	extern RE::ScriptEventSourceHolder* eventSourceholder;
	extern RE::TESDataHandler* dataHandler;
	extern RE::BSInputDeviceManager* inputManager;
	extern RE::BSScript::Internal::VirtualMachine* vm;
	extern RE::BSScript::IVirtualMachine* ivm;
	extern RE::SkyrimVM* skyrimVm;
	extern RE::Calendar* calendar;
	extern RE::UI* ui;
	extern RE::UserEvents* userEvents;
	extern RE::BSMusicManager* musicManager;
	extern RE::BSAudioManager* audiomanager;
	extern RE::Sky* sky;

	extern RE::PlayerCharacter* player;

	extern int frameUpdateCount;
	extern int iFrameUpdateInterval;
	extern std::string lastMenuOpened;
	extern bool gamePaused;
	extern bool inMenuMode;
	extern std::chrono::system_clock::time_point lastTimeMenuWasOpened;
	extern std::chrono::system_clock::time_point lastTimeGameWasPaused;
	extern std::chrono::system_clock::time_point currentTimePoint;
	extern float gameTime;
	extern RE::TESObjectREFR* menuRef;
	extern RE::TESObjectREFR* lastPlayerActivatedRef;
	extern RE::BSScript::Variable* LastPlayerMenuActivatedRefScriptProperty;
	extern RE::BSScript::Variable* lastPlayerActivatedRefScriptProperty;
	extern RE::TESWeather* currentWeather;
	extern RE::BSIMusicType* currentBSIMusicType;
	extern RE::BGSMusicType* currentBGSMusicType;
	extern std::mutex updateMutex;
	extern std::condition_variable updateCv;

	//called on kDataLoaded in plugin.cpp
	void Install();
	void Save(SKSE::SerializationInterface* ssi);
	void Load(SKSE::SerializationInterface* ssi);
}