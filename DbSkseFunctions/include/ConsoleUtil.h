#pragma once

namespace ConsoleUtil {
    void CompileAndRun(RE::Script* script, RE::TESObjectREFR* targetRef, RE::COMPILER_NAME name = RE::COMPILER_NAME::kSystemWindowCompiler);

    RE::ObjectRefHandle GetSelectedRefHandle();

    RE::NiPointer<RE::TESObjectREFR> GetSelectedRef();

    void ExecuteCommand(std::string a_command, RE::TESObjectREFR* objRef);
}