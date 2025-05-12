#pragma once

namespace papyrusUtilEx {
    /*std::string GetFormHandle(RE::StaticFunctionTag*, RE::TESForm* akForm);

    std::string GetAliasHandle(RE::StaticFunctionTag*, RE::BGSBaseAlias* akAlias);

    std::string GetActiveEffectHandle(RE::StaticFunctionTag*, RE::ActiveEffect* akActiveEffect);

    struct ArrayPropertyData;

    ArrayPropertyData GetArrayProperty(RE::BSScript::Internal::VirtualMachine* vm, RE::VMHandle handle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName);

    bool ResizeArrayProperty(RE::StaticFunctionTag*, std::string sHandle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName, int size, int fillIndex);

    bool MergeArrays(RE::StaticFunctionTag*, std::string sHandle_A, RE::BSFixedString bsScriptName_A, RE::BSFixedString bsArrayPropertyName_A,
        std::string sHandle_B, RE::BSFixedString bsScriptName_B, RE::BSFixedString bsArrayPropertyName_B);

    bool CopyArray(RE::StaticFunctionTag*, std::string sHandle_A, RE::BSFixedString bsScriptName_A, RE::BSFixedString bsArrayPropertyName_A,
        std::string sHandle_B, RE::BSFixedString bsScriptName_B, RE::BSFixedString bsArrayPropertyName_B);

    int CountInBSScriptArray(RE::BSTSmartPointer<RE::BSScript::Array> array, int index);

    int CountInArray(RE::StaticFunctionTag*, std::string sHandle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName, int index);

    int RemoveFromArray(RE::StaticFunctionTag*, std::string sHandle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName, int index, bool removeAll);

    bool SliceArray(RE::StaticFunctionTag*, std::string sHandle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName,
        int startIndex, int endIndex, bool keep);

    bool SliceArrayOnto(RE::StaticFunctionTag*, std::string sHandle_A, RE::BSFixedString bsScriptName_A, RE::BSFixedString bsArrayPropertyName_A,
        std::string sHandle_B, RE::BSFixedString bsScriptName_B, RE::BSFixedString bsArrayPropertyName_B,
        int startIndex, int endIndex, bool replace, bool keep);*/

    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}