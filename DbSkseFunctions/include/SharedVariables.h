#pragma once

#include "RE/B/BSPointerHandle.h"
namespace sv {
	extern std::uint32_t record;
	extern int iFrameUpdateInterval;
	extern std::string lastMenuOpened;
	extern bool gamePaused;
	extern bool inMenuMode;
	extern std::chrono::system_clock::time_point lastTimeMenuWasOpened;
	extern std::chrono::system_clock::time_point lastTimeGameWasPaused;
	extern std::chrono::system_clock::time_point currentTimePoint;
	extern float gameTime;
	
	// extern RE::TESObjectREFR* menuRef;
	// extern RE::TESObjectREFR* lastPlayerActivatedRef;
	
	extern RE::ObjectRefHandle menuRef;
	extern RE::ObjectRefHandle lastPlayerActivatedRef;
	
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