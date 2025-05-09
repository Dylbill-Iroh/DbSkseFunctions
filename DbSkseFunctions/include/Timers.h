#pragma once

extern int gameTimerPollingInterval; //in milliseconds

namespace timers {
    bool IsTimerType(std::uint32_t& type);

    void LoadTimers(std::uint32_t type, SKSE::SerializationInterface* a_intfc);

    void SaveTimers(SKSE::SerializationInterface* a_intfc);

    void UpdateNoMenuModeTimers(float timeElapsedWhilePaused);

    void UpdateTimers(float timeElapsedWhilePaused);

    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}