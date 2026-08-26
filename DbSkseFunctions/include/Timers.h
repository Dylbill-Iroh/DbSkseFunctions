#pragma once

extern bool noMenuModeTimersEmpty; //for update struct in plugin.cpp
extern bool timersEmpty; //for update struct in plugin.cpp
extern bool gameTimersEmpty; //for update struct in plugin.cpp
extern int gameTimerPollingInterval; //in milliseconds

namespace timers {
    size_t GetCurrentGameTimeTimersSize();

    bool IsTimerType(std::uint32_t& type);

    void LoadTimers(std::uint32_t type, SKSE::SerializationInterface* a_intfc);

    void SaveTimers(SKSE::SerializationInterface* a_intfc);

    void UpdateNoMenuModeTimers(float timeElapsedWhilePaused);

    void UpdateTimers(float timeElapsedWhilePaused);

    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}