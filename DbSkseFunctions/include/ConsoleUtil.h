#pragma once

namespace ConsoleUtil {
    extern const std::string consoleCommandEntryPath;
    extern const std::string consoleCommandHistoryPath;

    std::string GetCommandEntryText();

    bool SetCommandEntryText(std::string command, bool openConsole = false, bool execute = false, bool setInvisible = false);

    std::string GetCommandHistoryText();

    void SetCommandHistoryText(std::string text);

    void RemoveErrorMessageFromCommandHistory(std::string errorMsg, size_t milliSecondWait = 100, bool newThread = false);

    void CompileAndRun(RE::Script* script, RE::TESObjectREFR* targetRef, RE::COMPILER_NAME name = RE::COMPILER_NAME::kSystemWindowCompiler);

    RE::ObjectRefHandle GetSelectedRefHandle();

    RE::NiPointer<RE::TESObjectREFR> GetSelectedRef();

    void ExecuteCommand(std::string a_command, RE::TESObjectREFR* objRef);
}