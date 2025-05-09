#include "ConsoleUtil.h"
#include "GeneralFunctions.h"

namespace ConsoleUtil {
    void CompileAndRunImpl(RE::Script* script, RE::ScriptCompiler* compiler, RE::COMPILER_NAME name, RE::TESObjectREFR* targetRef) {
        using func_t = decltype(CompileAndRunImpl);
        REL::Relocation<func_t> func{ RELOCATION_ID(21416, REL::Module::get().version().patch() < 1130 ? 21890 : 441582) };
        return func(script, compiler, name, targetRef);
    }

    void CompileAndRun(RE::Script* script, RE::TESObjectREFR* targetRef, RE::COMPILER_NAME name) {
        RE::ScriptCompiler compiler;
        CompileAndRunImpl(script, &compiler, name, targetRef);
    }

    RE::ObjectRefHandle GetSelectedRefHandle() {
        REL::Relocation<RE::ObjectRefHandle*> selectedRef{ RELOCATION_ID(519394, REL::Module::get().version().patch() < 1130 ? 405935 : 504099) };
        return *selectedRef;
    }

    RE::NiPointer<RE::TESObjectREFR> GetSelectedRef() {
        auto handle = GetSelectedRefHandle();
        return handle.get();
    }

    //edited form ConsoleUtil NG
    void ExecuteCommand(std::string a_command, RE::TESObjectREFR* objRef) {

        const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
        const auto script = scriptFactory ? scriptFactory->Create() : nullptr;
        if (script) {
            script->SetCommand(a_command);

            if (gfuncs::IsFormValid(objRef)) {
                CompileAndRun(script, objRef);
            }
            else {
                const auto selectedRef = GetSelectedRef();
                //script->CompileAndRun(selectedRef.get());
                CompileAndRun(script, selectedRef.get());
            }

            delete script;
        }
    }
}
