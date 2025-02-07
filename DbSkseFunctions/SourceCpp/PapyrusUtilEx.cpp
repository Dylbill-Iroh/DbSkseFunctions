#include "PapyrusUtilEx.h"
#include "GeneralFunctions.h"
namespace logger = SKSE::log;

//papyrusUtilEX functions=======================================================================================================================
std::string GetFormHandle(RE::StaticFunctionTag*, RE::TESForm* akForm) {
    auto handle = gfuncs::GetHandle(akForm);
    std::string sHandle = std::to_string(handle);
    return sHandle;
}

std::string GetAliasHandle(RE::StaticFunctionTag*, RE::BGSBaseAlias* akAlias) {
    auto handle = gfuncs::GetHandle(akAlias);
    std::string sHandle = std::to_string(handle);
    return sHandle;
}

std::string GetActiveEffectHandle(RE::StaticFunctionTag*, RE::ActiveEffect* akActiveEffect) {
    auto handle = gfuncs::GetHandle(akActiveEffect);
    std::string sHandle = std::to_string(handle);
    return sHandle;
}

struct ArrayPropertyData {
    bool gotAllData = false;
    RE::BSScript::Variable* arrayProperty;
    RE::BSTSmartPointer<RE::BSScript::Array> arraySmartPtr;
    RE::BSScript::Array* arrayPtr;
    RE::BSScript::ObjectTypeInfo* info;
};

ArrayPropertyData GetArrayProperty(RE::BSScript::Internal::VirtualMachine* vm, RE::VMHandle handle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName) {
    ArrayPropertyData returnValue;

    auto it = vm->attachedScripts.find(handle);
    if (it == vm->attachedScripts.end()) {
        logger::error("DbSkse: {}: vm->attachedScripts couldn't find handle[{}] scriptName[{}] arrayProperty[{}]",
            __func__, handle, bsScriptName, bsArrayPropertyName);
        return returnValue;
    }

    for (int i = 0; i < it->second.size(); i++) {
        auto& attachedScript = it->second[i];
        if (attachedScript) {
            auto* script = attachedScript.get();
            if (script) {
                auto info = script->GetTypeInfo();
                if (info) {
                    if (info->name == bsScriptName) {
                        logger::trace("{}: script[{}] found attached to handle[{}]", __func__, bsScriptName, handle);
                        returnValue.arrayProperty = script->GetProperty(bsArrayPropertyName);

                        if (!returnValue.arrayProperty) {
                            returnValue.arrayProperty = script->GetVariable(bsArrayPropertyName);
                        }

                        if (!returnValue.arrayProperty) {
                            logger::error("{}: arrayProperty[{}] not found in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, handle);
                            return returnValue;
                        }

                        if (!returnValue.arrayProperty->IsArray()) {
                            logger::error("{}: arrayProperty[{}] in script[{}] on handle[{}] is not an array.", __func__, bsArrayPropertyName, bsScriptName, handle);
                            return returnValue;
                        }

                        returnValue.arraySmartPtr = returnValue.arrayProperty->GetArray();
                        if (!returnValue.arraySmartPtr) {
                            logger::error("{}: arraySmartPtr for [{}] not got from arrayData.arraySmartPtr.get() in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, handle);
                            return returnValue;
                        }

                        returnValue.arrayPtr = returnValue.arraySmartPtr.get();
                        if (!returnValue.arrayPtr) {
                            logger::error("{}: arrayPtr for [{}] not got from arrayData.arraySmartPtr.get() in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, handle);
                            return returnValue;
                        }

                        returnValue.info = returnValue.arrayPtr->type_info().GetTypeInfo();
                        if (!returnValue.info) {
                            logger::error("{}: arrayInfo for [{}] not found in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, handle);
                            return returnValue;
                        }
                        returnValue.gotAllData = true;
                    }
                }
            }
        }
    }
    return returnValue;
}

bool ResizeArrayProperty(RE::StaticFunctionTag*, std::string sHandle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName, int size, int fillIndex) {
    RE::VMHandle akHandle = gfuncs::StringToUint64_t(sHandle);

    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        logger::error("{}: couldn't get *vm for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return false;
    }

    auto arrayData = GetArrayProperty(vm, akHandle, bsScriptName, bsArrayPropertyName);

    if (!arrayData.gotAllData) {
        logger::error("{}: failed to get all property data for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return false;
    }

    auto className = arrayData.info->GetName();
    if (!className) {
        logger::error("{}: className for [{}] not found in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return false;
    }

    RE::BSTSmartPointer<RE::BSScript::Array> newArray;
    vm->CreateArray2(arrayData.arrayPtr->type(), className, size, newArray);
    //vm->CreateArray(RE::BSScript::TypeInfo{ RE::BSScript::TypeInfo::RawType::kObject }, size, newArray);

    int i = 0;
    int oldSize = arrayData.arrayPtr->size();
    if (oldSize <= 0) {
        logger::error("{}: [{}] in script[{}] on handle[{}] not initialized",
            __func__, className, bsArrayPropertyName, bsScriptName, akHandle);
        return false;
    }

    if (size < 1) {
        size = 1;
    }

    if (fillIndex < 0 || fillIndex >(oldSize - 1)) {
        fillIndex = (oldSize - 1);
    }

    //this worked to prevent ctd when saving in game
    newArray.get()->type_info().SetType(arrayData.arrayPtr->type_info().GetRawType());

    for (i; i < size && i < oldSize; i++) {
        newArray->data()[i] = arrayData.arraySmartPtr->data()[i];
    }

    if (size > oldSize) {
        for (i; i < size; i++) {
            newArray->data()[i] = arrayData.arraySmartPtr->data()[fillIndex];
        }
    }

    arrayData.arrayProperty->SetNone();
    arrayData.arrayProperty->SetArray(newArray);

    logger::trace("{}: scriptName[{}] array[{}] type[{}] on [{}] resized from[{}] to[{}]. Expected size[{}]", __func__,
        bsScriptName, bsArrayPropertyName, className, akHandle, oldSize, newArray->size(), size);

    return (newArray->size() == size);
}

bool MergeArrays(RE::StaticFunctionTag*, std::string sHandle_A, RE::BSFixedString bsScriptName_A, RE::BSFixedString bsArrayPropertyName_A,
    std::string sHandle_B, RE::BSFixedString bsScriptName_B, RE::BSFixedString bsArrayPropertyName_B) {

    RE::VMHandle akHandle_A = gfuncs::StringToUint64_t(sHandle_A);
    RE::VMHandle akHandle_B = gfuncs::StringToUint64_t(sHandle_B);

    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        logger::error("{}: couldn't get *vm for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
        return false;
    }

    auto arrayData_A = GetArrayProperty(vm, akHandle_A, bsScriptName_A, bsArrayPropertyName_A);

    if (!arrayData_A.gotAllData) {
        logger::error("{}: failed to get all property data for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
        return false;
    }

    auto arrayData_B = GetArrayProperty(vm, akHandle_B, bsScriptName_B, bsArrayPropertyName_B);
    if (!arrayData_B.gotAllData) {
        logger::error("{}: failed to get all property data for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
        return false;
    }

    auto className = arrayData_B.info->GetName();
    if (!className) {
        logger::error("{}: className for [{}] not found in script[{}] on handle[{}]", __func__, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
        return false;
    }

    if (className != arrayData_A.info->GetName()) {
        logger::error("{}: className [{}] for [{}] in script[{}] on handle[{}] doesn't match \n className [{}] for [{}] in script[{}] on handle[{}]",
            __func__, className, bsArrayPropertyName_A, bsScriptName_A, akHandle_A, arrayData_A.info->GetName(), bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
        return false;
    }

    int sizeA = arrayData_A.arrayPtr->size();
    if (sizeA <= 0) {
        logger::error("{}: [{}] in script[{}] on handle[{}] not initialized",
            __func__, className, bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
        return false;
    }

    int sizeB = arrayData_B.arrayPtr->size();
    if (sizeB <= 0) {
        logger::error("{}: [{}] in script[{}] on handle[{}] not initialized",
            __func__, className, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
        return false;
    }

    int newSize = (sizeA + sizeB);
    int i = 0;

    RE::BSTSmartPointer<RE::BSScript::Array> newArray;
    vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSize, newArray);

    //this worked to prevent ctd when saving in game
    newArray.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

    for (i; i < sizeB; i++) {
        newArray->data()[i] = arrayData_B.arraySmartPtr->data()[i];
    }

    int newIndex = i;
    i = 0;

    for (i; i < sizeA; i++, newIndex++) {
        newArray->data()[newIndex] = arrayData_A.arraySmartPtr->data()[i];
    }

    arrayData_B.arrayProperty->SetNone();
    arrayData_B.arrayProperty->SetArray(newArray);

    logger::trace("{}: scriptName[{}] array[{}] type[{}] on [{}] merged with \n scriptName[{}] array[{}] type[{}] on [{}]. New size[{}] Expected size[{}]", __func__,
        bsScriptName_A, bsArrayPropertyName_A, className, akHandle_A, bsScriptName_B, bsArrayPropertyName_B, className, akHandle_B, newArray->size(), newSize);

    return (newArray->size() == newSize);
}

bool CopyArray(RE::StaticFunctionTag*, std::string sHandle_A, RE::BSFixedString bsScriptName_A, RE::BSFixedString bsArrayPropertyName_A,
    std::string sHandle_B, RE::BSFixedString bsScriptName_B, RE::BSFixedString bsArrayPropertyName_B) {

    RE::VMHandle akHandle_A = gfuncs::StringToUint64_t(sHandle_A);
    RE::VMHandle akHandle_B = gfuncs::StringToUint64_t(sHandle_B);

    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        logger::error("{}: couldn't get *vm for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
        return false;
    }

    auto arrayData_A = GetArrayProperty(vm, akHandle_A, bsScriptName_A, bsArrayPropertyName_A);
    if (!arrayData_A.gotAllData) {
        logger::error("{}: failed to get all property data for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
        return false;
    }

    auto arrayData_B = GetArrayProperty(vm, akHandle_B, bsScriptName_B, bsArrayPropertyName_B);
    if (!arrayData_B.gotAllData) {
        logger::error("{}: failed to get all property data for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
        return false;
    }

    auto className = arrayData_B.info->GetName();
    if (!className) {
        logger::error("{}: className for [{}] not found in script[{}] on handle[{}]", __func__, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
        return false;
    }

    if (className != arrayData_A.info->GetName()) {
        logger::error("{}: className [{}] for [{}] in script[{}] on handle[{}] doesn't match \n  className [{}] for [{}] in script[{}] on handle[{}]",
            __func__, className, bsArrayPropertyName_A, bsScriptName_A, akHandle_A, arrayData_A.info->GetName(), bsArrayPropertyName_B, bsScriptName_B, akHandle_B);

        return false;
    }

    int sizeA = arrayData_A.arrayPtr->size();
    if (sizeA <= 0) {
        logger::error("{}: [{}] in script[{}] on handle[{}] not initialized",
            __func__, bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
        return false;
    }

    int sizeB = arrayData_B.arrayPtr->size();
    if (sizeB <= 0) {
        logger::error("{}: [{}] in script[{}] on handle[{}] not initialized",
            __func__, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
        return false;
    }

    int i = 0;

    RE::BSTSmartPointer<RE::BSScript::Array> newArray;
    vm->CreateArray2(arrayData_B.arrayPtr->type(), className, sizeA, newArray);

    //this worked to prevent ctd when saving in game
    newArray.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

    for (i; i < sizeA; i++) {
        newArray->data()[i] = arrayData_A.arraySmartPtr->data()[i];
    }

    arrayData_B.arrayProperty->SetNone();
    arrayData_B.arrayProperty->SetArray(newArray);

    logger::trace("{}: scriptName[{}] array[{}] type[{}] on handle[{}] copied to \n scriptName[{}] array[{}] type[{}] on handle[{}]. New size[{}] Expected size[{}]", __func__,
        bsScriptName_A, bsArrayPropertyName_A, className, akHandle_A, bsScriptName_B, bsArrayPropertyName_B, className, akHandle_B, newArray->size(), sizeA);

    return (newArray->size() == sizeA);
}

int CountInBSScriptArray(RE::BSTSmartPointer<RE::BSScript::Array> array, int index) {
    int count = 0;
    int size = array->size();

    if (size > 0 && index >= 0 && index < size) {
        for (int i = 0; i < size; i++) {
            if (array->data()[i] == array->data()[index]) {
                count++;
            }
        }
    }
    return count;
}

int CountInArray(RE::StaticFunctionTag*, std::string sHandle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName, int index) {
    RE::VMHandle akHandle = gfuncs::StringToUint64_t(sHandle);

    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        logger::error("{}: couldn't get *vm for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return false;
    }

    auto arrayData = GetArrayProperty(vm, akHandle, bsScriptName, bsArrayPropertyName);

    if (!arrayData.gotAllData) {
        logger::error("{}: failed to get all property data for[{}] in script[{}] on Handle[{}]", __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return 0;
    }

    int size = arrayData.arrayPtr->size();

    if (size <= 0) {
        logger::error("{}: [{}] in script[{}] on handle[{}] not initialized",
            __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return 0;
    }

    if (index < 0) {
        index = (size - 1);
    }
    else if (index >= size) {
        logger::error("{}: index[{}] for [{}] in script[{}] on handle[{}] isn't valid", __func__, index, bsArrayPropertyName, bsScriptName, akHandle);
        return 0;
    }

    return CountInBSScriptArray(arrayData.arraySmartPtr, index);
}

int RemoveFromArray(RE::StaticFunctionTag*, std::string sHandle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName, int index, bool removeAll) {
    RE::VMHandle akHandle = gfuncs::StringToUint64_t(sHandle);

    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        logger::error("{}: couldn't get *vm for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return 0;
    }

    auto arrayData = GetArrayProperty(vm, akHandle, bsScriptName, bsArrayPropertyName);

    if (!arrayData.gotAllData) {
        logger::error("{}: failed to get all property data for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return 0;
    }

    auto className = arrayData.info->GetName();

    if (!className) {
        logger::error("{}: className for [{}] not found in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return 0;
    }

    RE::BSTSmartPointer<RE::BSScript::Array> newArray;

    int newSize = 0;
    int oldSize = arrayData.arrayPtr->size();
    if (oldSize <= 0) {
        logger::error("{}: [{}] in script[{}] on handle[{}] not initialized",
            __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return 0;
    }

    if (index < 0) {
        index = (oldSize - 1);
    }

    if (index >= oldSize) {
        logger::error("{}: index[{}] for [{}] in script[{}] on handle[{}] isn't valid", __func__, index, bsArrayPropertyName, bsScriptName, akHandle);
        return 0;
    }

    int count = 1;

    if (removeAll) {
        count = CountInBSScriptArray(arrayData.arraySmartPtr, index);
        if (count > 1) {
            int newSize = (oldSize - count);
            if (newSize <= 0) {
                logger::info("{}: [{}] in script[{}] on [{}] is filled entirely with the element at index[{}], setting size to 1",
                    __func__, bsArrayPropertyName, bsScriptName, akHandle, index);

                newSize = 1;
                count = -1;
                vm->CreateArray2(arrayData.arrayPtr->type(), className, newSize, newArray);
                newArray.get()->type_info().SetType(arrayData.arrayPtr->type_info().GetRawType());
                newArray->data()[0] = arrayData.arraySmartPtr->data()[0]; //must set the 0 entry to a valid entry before setting to none or it will break the array
                newArray->data()[0].SetNone();
            }
            else {
                vm->CreateArray2(arrayData.arrayPtr->type(), className, newSize, newArray);

                int oldIndex = 0;
                int newIndex = 0;

                newArray.get()->type_info().SetType(arrayData.arrayPtr->type_info().GetRawType());
                for (oldIndex; oldIndex < oldSize && newIndex < newSize; oldIndex++) {
                    if (arrayData.arraySmartPtr->data()[oldIndex] != arrayData.arraySmartPtr->data()[index]) {
                        newArray->data()[newIndex] = arrayData.arraySmartPtr->data()[oldIndex];
                        newIndex++;
                    }
                }
            }
        }
    }

    if (!removeAll || count == 1) {
        newSize = (oldSize - 1);
        vm->CreateArray2(arrayData.arrayPtr->type(), className, newSize, newArray);
        //vm->CreateArray(RE::BSScript::TypeInfo{ RE::BSScript::TypeInfo::RawType::kObject }, size, newArray);

        if (newSize <= 0) {
            logger::error("{}: [{}] in script[{}] on handle[{}] is already the minimum size 1",
                __func__, bsArrayPropertyName, bsScriptName, akHandle);

            return 0;
        }

        int oldIndex = 0;
        int newIndex = 0;

        newArray.get()->type_info().SetType(arrayData.arrayPtr->type_info().GetRawType());

        for (oldIndex; oldIndex < index; oldIndex++) {
            newArray->data()[oldIndex] = arrayData.arraySmartPtr->data()[oldIndex];
        }
        //skip index 
        newIndex = oldIndex;
        oldIndex++;
        if (oldIndex < newSize) {
            for (oldIndex; oldIndex < newSize; oldIndex++, newIndex++) {
                newArray->data()[newIndex] = arrayData.arraySmartPtr->data()[oldIndex];
            }
        }
    }

    //this worked to prevent ctd when saving in game

    arrayData.arrayProperty->SetNone();
    arrayData.arrayProperty->SetArray(newArray);

    logger::trace("{}: scriptName[{}] array[{}] type[{}] on [{}] resized from[{}] to[{}]. Expected size[{}], removed elements[{}]", __func__,
        bsScriptName, bsArrayPropertyName, className, akHandle, oldSize, newArray->size(), newSize, count);

    return (count);
}

bool SliceArray(RE::StaticFunctionTag*, std::string sHandle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName,
    int startIndex, int endIndex, bool keep) {

    RE::VMHandle akHandle = gfuncs::StringToUint64_t(sHandle);

    logger::trace("{}: called", __func__);

    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        logger::error("{}: couldn't get *vm for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return false;
    }

    auto arrayData = GetArrayProperty(vm, akHandle, bsScriptName, bsArrayPropertyName);
    if (!arrayData.gotAllData) {
        logger::error("{}: failed to get all property data for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return false;
    }

    auto className = arrayData.info->GetName();
    if (!className) {
        logger::error("{}: className for [{}] not found in script[{}] on handle[{}]", __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return false;
    }

    int size = arrayData.arrayPtr->size();
    if (size <= 0) {
        logger::error("{}: [{}] in script[{}] on handle[{}] not initialized",
            __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return false;
    }

    if (endIndex < 0) {
        endIndex = (size - 1);
    }

    if (startIndex > endIndex || startIndex < 0 || startIndex >= size || endIndex >= size) {
        logger::error("{}: [{}] in script[{}] on handle[{}] startIndex[{}] or endIndex[{}] isn't valid",
            __func__, bsArrayPropertyName, bsScriptName, akHandle, startIndex, endIndex);

        return false;
    }

    if (startIndex == 0 && endIndex >= (size - 1)) {
        logger::error("{}: [{}] in script[{}] on handle[{}], can't slice the entire array.",
            __func__, bsArrayPropertyName, bsScriptName, akHandle);
        return false;
    }

    RE::BSTSmartPointer<RE::BSScript::Array> newArray;
    int newArraySize = 1;

    //keep portion between startIndex and endIndex of array in array
    if (keep) {
        newArraySize = (endIndex - startIndex + 1);

        vm->CreateArray2(arrayData.arrayPtr->type(), className, newArraySize, newArray);
        newArray.get()->type_info().SetType(arrayData.arrayPtr->type_info().GetRawType());

        int i = 0;
        int ii = startIndex;
        //copy the elements between startIndex and endIndex from array to newArray
        for (ii; ii <= endIndex; i++, ii++) {
            newArray->data()[i] = arrayData.arraySmartPtr->data()[ii];
        }
    }
    //remove portion between startIndex and endIndex
    else {
        newArraySize = (size - (endIndex - startIndex + 1));

        int i = 0;
        int ii = 0;
        vm->CreateArray2(arrayData.arrayPtr->type(), className, newArraySize, newArray);
        newArray.get()->type_info().SetType(arrayData.arrayPtr->type_info().GetRawType());

        //copy the elements between startIndex and endIndex from array to newArray
        for (i; i < startIndex; i++) {
            newArray->data()[i] = arrayData.arraySmartPtr->data()[i];
        }

        ii = (endIndex + 1);

        for (i; i < newArraySize; i++, ii++) {
            newArray->data()[i] = arrayData.arraySmartPtr->data()[ii];
        }
    }

    arrayData.arrayProperty->SetNone();
    arrayData.arrayProperty->SetArray(newArray);

    logger::trace("{}: scriptName[{}] array[{}] type[{}] on handle[{}] sliced", __func__,
        bsScriptName, bsArrayPropertyName, className, akHandle);

    return (newArray->size() == newArraySize);
}

bool SliceArrayOnto(RE::StaticFunctionTag*, std::string sHandle_A, RE::BSFixedString bsScriptName_A, RE::BSFixedString bsArrayPropertyName_A,
    std::string sHandle_B, RE::BSFixedString bsScriptName_B, RE::BSFixedString bsArrayPropertyName_B,
    int startIndex, int endIndex, bool replace, bool keep) {

    RE::VMHandle akHandle_A = gfuncs::StringToUint64_t(sHandle_A);
    RE::VMHandle akHandle_B = gfuncs::StringToUint64_t(sHandle_B);


    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        logger::error("{}: couldn't get *vm for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
        return false;
    }

    auto arrayData_A = GetArrayProperty(vm, akHandle_A, bsScriptName_A, bsArrayPropertyName_A);
    if (!arrayData_A.gotAllData) {
        logger::error("{}: failed to get all property data for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
        return false;
    }

    auto arrayData_B = GetArrayProperty(vm, akHandle_B, bsScriptName_B, bsArrayPropertyName_B);
    if (!arrayData_B.gotAllData) {
        logger::trace("{}: failed to get all property data for [{}] in script[{}] on handle[{}]", __func__, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
        return false;
    }

    auto className = arrayData_B.info->GetName();
    if (!className) {
        logger::error("{}: className for [{}] not found in script[{}] on handle[{}]", __func__, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
        return false;
    }

    if (className != arrayData_A.info->GetName()) {
        logger::error("{}: className [{}] for [{}] in script[{}] on handle[{}] doesn't match \nclassName [{}] for [{}] in script[{}] on handle[{}]",
            __func__, className, bsArrayPropertyName_A, bsScriptName_A, akHandle_A, arrayData_A.info->GetName(), bsArrayPropertyName_B, bsScriptName_B, akHandle_B);

        return false;
    }

    int sizeA = arrayData_A.arrayPtr->size();
    if (sizeA <= 0) {
        logger::error("{}: [{}] in script[{}] on handle[{}] not initialized",
            __func__, bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
        return false;
    }

    int sizeB = arrayData_B.arrayPtr->size();
    if (sizeB <= 0) {
        logger::error("{}: [{}] in script[{}] on handle[{}] not initialized",
            __func__, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
        return false;
    }

    if (endIndex < 0) {
        endIndex = (sizeA - 1);
    }

    if (startIndex > endIndex || startIndex < 0 || startIndex >= sizeA || endIndex >= sizeA) {
        logger::error("{}: [{}] in script[{}] on handle[{}] to \n [{}] in script[{}] on handle[{}], startIndex[{}] or endIndex[{}] isn't valid",
            __func__, bsArrayPropertyName_A, bsScriptName_A, akHandle_A, bsArrayPropertyName_B, bsScriptName_B, akHandle_B, startIndex, endIndex);

        return false;
    }

    if (startIndex == 0 && endIndex >= (sizeA - 1)) {
        logger::error("{}: [{}] in script[{}] on handle[{}] to \n [{}] in script[{}] on handle[{}], can't slice the entire array.",
            __func__, bsArrayPropertyName_A, bsScriptName_A, akHandle_A, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
        return false;
    }

    RE::BSTSmartPointer<RE::BSScript::Array> newArray_A;
    RE::BSTSmartPointer<RE::BSScript::Array> newArray_B;
    int newSizeA = 1;
    int newSizeB = 1;

    if (keep && replace) {
        newSizeA = (endIndex - startIndex + 1);
        newSizeB = (sizeA - newSizeA);

        vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeA, newArray_A);
        newArray_A.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

        vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeB, newArray_B);
        newArray_B.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

        int i = 0;
        int ii = startIndex;
        //copy the elements between startIndex and endIndex from array_A to newArray_A
        for (ii; ii <= endIndex; i++, ii++) {
            //logger::trace("{}: newArray_A loop1 i[{}] replace[{}] keep[{}]", __func__, i, replace, keep);
            newArray_A->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
        }

        i = 0;

        //copy the remainder of indexes from array_A to array_B
        for (i; i < startIndex; i++) {
            //logger::trace("{}: newArray_B loop1 i[{}] replace[{}] keep[{}]", __func__, i, replace, keep);
            newArray_B->data()[i] = arrayData_A.arraySmartPtr->data()[i];
        }

        ii = (endIndex + 1);

        for (i; i < newSizeB; i++, ii++) {
            //logger::trace("{}: newArray_B loop2 i[{}] ii[{}] replace[{}] keep[{}]", __func__, i, ii, replace, keep);
            newArray_B->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
        }
    }
    else if (keep && !replace) {
        newSizeA = (endIndex - startIndex + 1);
        newSizeB = (sizeA - newSizeA);
        newSizeB += sizeB;

        vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeA, newArray_A);
        newArray_A.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

        vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeB, newArray_B);
        newArray_B.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

        int i = 0;
        int ii = startIndex;
        //copy the elements between startIndex and endIndex from array_A to newArray_A
        for (ii; ii <= endIndex; i++, ii++) {
            //logger::trace("{}: newArray_A loop1 i[{}] ii[{}] replace[{}] keep[{}]", __func__, i, ii, replace, keep);
            newArray_A->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
        }

        i = 0;

        //copy the original elements from array_B to newArray_B as we're merging not replacing
        for (i; i < sizeB; i++) {
            //logger::trace("{}: newArray_B loop1 i[{}] replace[{}] keep[{}]", __func__, i, replace, keep);
            newArray_B->data()[i] = arrayData_B.arraySmartPtr->data()[i];
        }

        ii = 0;

        //copy the remainder of indexes from array_A to array_B
        for (ii; ii < startIndex; i++, ii++) {
            //logger::trace("{}: newArray_B loop2 i[{}] ii[{}] replace[{}] keep[{}]", __func__, i, ii, replace, keep);
            newArray_B->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
        }

        ii = (endIndex + 1);

        for (i; i < newSizeB; i++, ii++) {
            //logger::trace("{}: newArray_B loop3 i[{}] ii[{}] replace[{}] keep[{}]", __func__, i, ii, replace, keep);
            newArray_B->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
        }
    }
    else if (!keep && replace) {
        newSizeB = (endIndex - startIndex + 1);
        newSizeA = (sizeA - newSizeB);

        vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeA, newArray_A);
        newArray_A.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

        vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeB, newArray_B);
        newArray_B.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

        int i = 0;
        int ii = startIndex;
        //copy the elements between startIndex and endIndex from array_A to newArray_B
        for (ii; ii <= endIndex; i++, ii++) {
            //logger::trace("{}: newArray_B loop1 i[{}] ii[{}] replace[{}] keep[{}]", __func__, i, ii, replace, keep);
            newArray_B->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
        }

        i = 0;

        //copy the remainder of indexes from array_A to newArray_A
        for (i; i < startIndex; i++) {
            //logger::trace("{}: newArray_A loop1 i[{}] replace[{}] keep[{}]", __func__, i, replace, keep);
            newArray_A->data()[i] = arrayData_A.arraySmartPtr->data()[i];
        }

        ii = (endIndex + 1);

        for (i; i < newSizeA; i++, ii++) {
            //logger::trace("{}: newArray_A loop2 i[{}] ii[{}] replace[{}] keep[{}]", __func__, i, ii, replace, keep);
            newArray_A->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
        }
    }
    else if (!keep && !replace) {
        newSizeB = (endIndex - startIndex + 1);
        newSizeA = (sizeA - newSizeB);
        newSizeB += sizeB;

        vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeA, newArray_A);
        newArray_A.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

        vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeB, newArray_B);
        newArray_B.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

        int i = 0;

        //copy the original elements from array_B to newArray_B as we're merging not replacing
        for (i; i < sizeB; i++) {
            //logger::trace("{}: newArray_B loop1 i[{}] replace[{}] keep[{}]", __func__, i, replace, keep);
            newArray_B->data()[i] = arrayData_B.arraySmartPtr->data()[i];
        }

        int ii = startIndex;
        //copy the elements between startIndex and endIndex from array_A to newArray_B
        for (ii; ii <= endIndex; i++, ii++) {
            //logger::trace("{}: newArray_B loop2 i[{}] ii[{}] replace[{}] keep[{}]", __func__, i, ii, replace, keep);
            newArray_B->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
        }

        i = 0;

        //copy the remainder of indexes from array_A to newArray_A
        for (i; i < startIndex; i++) {
            //logger::trace("{}: newArray_A loop1 i[{}] replace[{}] keep[{}]", __func__, i, replace, keep);
            newArray_A->data()[i] = arrayData_A.arraySmartPtr->data()[i];
        }

        ii = (endIndex + 1);

        for (i; i < newSizeA; i++, ii++) {
            //logger::trace("{}: newArray_A loop2 i[{}] ii[{}] replace[{}] keep[{}]", __func__, i, ii, replace, keep);
            newArray_A->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
        }
    }

    arrayData_A.arrayProperty->SetNone();
    arrayData_A.arrayProperty->SetArray(newArray_A);

    arrayData_B.arrayProperty->SetNone();
    arrayData_B.arrayProperty->SetArray(newArray_B);

    logger::trace("{}: scriptName[{}] array[{}] type[{}] on handle[{}] sliced to \n scriptName[{}] array[{}] type[{}] on handle[{}]", __func__,
        bsScriptName_A, bsArrayPropertyName_A, className, akHandle_A, bsScriptName_B, bsArrayPropertyName_B, className, akHandle_B);


    return (newArray_A->size() == newSizeA && newArray_B->size() == newSizeB);
}

//==============================================================================================================================================
