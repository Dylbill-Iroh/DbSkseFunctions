#pragma once

namespace conditions {
    extern std::condition_variable updateCv;
    extern bool isEmpty;

    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}
