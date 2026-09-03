#include "PapyrusUtilEx.h"
#include "GeneralFunctions.h"
#include "RE/B/BSCoreTypes.h"
#include "RE/O/ObjectTypeInfo.h"
#include "SharedVariables.h"

namespace papyrusUtilEx { 
	RE::VMHandle GetHandle(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, std::string sHandle){
		if (sHandle == ""){
			return gfuncs::GetCallingScriptHandle(vm, stackID, "PapyrusUtilEx");
		} 
		else {
			return gfuncs::StringToUint64_t(sHandle);
		}
	}
	
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

    RE::ActiveEffect* GetActiveEffectFromHandle(RE::StaticFunctionTag*, std::string sHandle) {
        RE::VMHandle akHandle = gfuncs::StringToUint64_t(sHandle);
        return gfuncs::GetActiveEffectFromHandle(akHandle);
    }

	bool BindArray(std::string typeName){
		RE::BSTSmartPointer<RE::BSScript::ObjectTypeInfo> objType;
		if (sv::vm->GetScriptObjectType(typeName, objType)){
			RE::BSScript::TypeInfo type;
			type.SetType(static_cast<RE::BSScript::TypeInfo::RawType>(reinterpret_cast<std::size_t>(objType.get()) | 1));   // bit 0 = array
			return true;
		}
		return false;
	}
	
	// The array's own _elementType is unreliable (some native getters leave it kNone),
	// so fall back to inspecting the first typed element.
	RE::BSScript::TypeInfo DeriveElementType(RE::BSScript::Array* a_arr) {
		if (!a_arr) { return RE::BSScript::TypeInfo(); }

		const auto declared = a_arr->type_info();
		if (declared.GetUnmangledRawType() != RE::BSScript::TypeInfo::RawType::kNone) {
			return declared;
		}

		for (std::uint32_t i = 0; i < a_arr->size(); i++) {
			const auto t = (*a_arr)[i].GetType();
			if (t.GetUnmangledRawType() != RE::BSScript::TypeInfo::RawType::kNone) {
				return t;
			}
		}
		return RE::BSScript::TypeInfo();
	}
	
	std::string GetObjTypeInfoName(RE::BSScript::ObjectTypeInfo* info){
		if (info){
			return info->GetName();
		}
		
		return "";
	}
	
	bool TypeInfosMatch(RE::BSScript::TypeInfo typeA, RE::BSScript::TypeInfo typeB) { 
		auto* infoA = typeA.GetTypeInfo();
		auto* infoB = typeA.GetTypeInfo();
		
		if (infoA && infoB){
			return infoA->GetName() == infoB->GetName();
		}
		
		auto rawTypeA = typeA.GetUnmangledRawType();
		auto rawTypeB = typeB.GetUnmangledRawType();
		
		if (rawTypeA == RE::BSScript::TypeInfo::RawType::kNone && rawTypeB ==RE::BSScript::TypeInfo::RawType::kNone){
			return false;
		}
		
		if (typeA == typeB){
			return true;
		} 
		
		if (rawTypeA == RE::BSScript::TypeInfo::RawType::kNone){
			return true;
		}
		
		if (rawTypeB == RE::BSScript::TypeInfo::RawType::kNone){
			return true;
		}
		
		return false;
	}
	
    struct ArrayPropertyData {
        bool gotAllData = false;
        RE::BSScript::Variable* arrayProperty;
        RE::BSTSmartPointer<RE::BSScript::Array> arraySmartPtr;
        RE::BSScript::Array* arrayPtr;
        RE::BSScript::ObjectTypeInfo* info;
    };

    ArrayPropertyData GetArrayProperty(RE::BSScript::Internal::VirtualMachine *vm, RE::VMStackID stackID, RE::VMHandle handle, 
		RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName) {
			
        ArrayPropertyData returnValue;

		if (bsScriptName == "") {
			bsScriptName = gfuncs::GetCallingScriptName(vm, stackID, "PapyrusUtilEx");
		}
		
        auto it = sv::vm->attachedScripts.find(handle);
        if (it == sv::vm->attachedScripts.end()) {
            logger::error("sv::vm->attachedScripts couldn't find handle[{}] scriptName[{}] arrayProperty[{}]",
                handle, bsScriptName, bsArrayPropertyName);
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
                            logger::trace("script[{}] found attached to handle[{}]", bsScriptName, handle);
                            returnValue.arrayProperty = script->GetProperty(bsArrayPropertyName);

                            if (!returnValue.arrayProperty) {
                                returnValue.arrayProperty = script->GetVariable(bsArrayPropertyName);
                            }

                            if (!returnValue.arrayProperty) {
                                logger::error("arrayProperty[{}] not found in script[{}] on handle[{}]", 
									bsArrayPropertyName, bsScriptName, handle);
                                return returnValue;
                            }

                            if (!returnValue.arrayProperty->IsArray()) {
                                logger::error("arrayProperty[{}] in script[{}] on handle[{}] is not an array.", 
									bsArrayPropertyName, bsScriptName, handle);
                                return returnValue;
                            }

                            returnValue.arraySmartPtr = returnValue.arrayProperty->GetArray();
                            if (!returnValue.arraySmartPtr) {
                                logger::error("arraySmartPtr for [{}] not got from arrayData.arraySmartPtr.get() in script[{}] on handle[{}]", 
									bsArrayPropertyName, bsScriptName, handle);
                                return returnValue;
                            }

                            returnValue.arrayPtr = returnValue.arraySmartPtr.get();
                            if (!returnValue.arrayPtr) {
                                logger::error("arrayPtr for [{}] not got from arrayData.arraySmartPtr.get() in script[{}] on handle[{}]", 
									bsArrayPropertyName, bsScriptName, handle);
                                return returnValue;
                            }

                            returnValue.info = returnValue.arrayPtr->type_info().GetTypeInfo();
                            if (!returnValue.info) { //could have no info* if rawtype is kNone from getting array from a native function such as Form.GetKeywords()
                                logger::warn("arrayInfo* for [{}] not found in script[{}] on handle[{}]", 
									bsArrayPropertyName, bsScriptName, handle);
                                // return returnValue;
                            }
                            returnValue.gotAllData = true;
                        }
                    }
                }
            }
        }
        return returnValue;
    }

    bool ResizeArrayProperty(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::StaticFunctionTag* functionTag, 
		std::string sHandle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName, int size, int fillIndex) {
        
		RE::VMHandle akHandle = GetHandle(vm, stackID, sHandle); 
		
        if (!sv::vm) {
            logger::error("couldn't get *sv::vm for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName, bsScriptName, akHandle);
            return false;
        }
		
        auto arrayData = GetArrayProperty(vm, stackID, akHandle, bsScriptName, bsArrayPropertyName);

        if (!arrayData.gotAllData) {
            logger::error("failed to get all property data for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName, bsScriptName, akHandle);
            return false;
        }

		RE::BSScript::TypeInfo type = DeriveElementType(arrayData.arrayPtr);
		if (type.GetUnmangledRawType() == RE::BSScript::TypeInfo::RawType::kNone) {
			logger::error("couldn't determine element type for [{}] on script[{}]-- array is untyped and all elements are None, aborting.",
						bsArrayPropertyName, bsScriptName);
			return false;
		}
		
        std::string className = GetObjTypeInfoName(arrayData.info);
        if (className == "") {
            logger::debug("className for [{}] not found in script[{}] on handle[{}]. Most likely due to array being type kNone from a native array function.", 
				bsArrayPropertyName, bsScriptName, akHandle);
        }
		
        RE::BSTSmartPointer<RE::BSScript::Array> newArray;
		if (!sv::vm->CreateArray1(type, size, newArray)){
			logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName, bsScriptName);
			return false;
		}
        //sv::vm->CreateArray(RE::BSScript::TypeInfo{ RE::BSScript::TypeInfo::RawType::kObject }, size, newArray);

        int i = 0;
        int oldSize = arrayData.arrayPtr->size();
        if (oldSize <= 0) {
            logger::error("type[{}] property[{}] in script[{}] on handle[{}] not initialized",
                className, bsArrayPropertyName, bsScriptName, akHandle);
            return false;
        }

        if (size < 1) {
            size = 1;
        }

        if (fillIndex < 0 || fillIndex >(oldSize - 1)) {
            fillIndex = (oldSize - 1);
        }

        //this worked to prevent ctd when saving in game
		//not needed for createarray1, only createarray2
        // newArray.get()->type_info().SetType(arrayData.arrayPtr->type_info().GetRawType());

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

        logger::trace("scriptName[{}] array[{}] type[{}] on handle[{}] resized from[{}] to[{}]. Expected size[{}]",
            bsScriptName, bsArrayPropertyName, className, akHandle, oldSize, newArray->size(), size);

        return (newArray->size() == size);
    }

    bool MergeArrays(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::StaticFunctionTag* functionTag, 
		std::string sHandle_A, RE::BSFixedString bsScriptName_A, RE::BSFixedString bsArrayPropertyName_A,
        std::string sHandle_B, RE::BSFixedString bsScriptName_B, RE::BSFixedString bsArrayPropertyName_B) {
		
        RE::VMHandle akHandle_A = GetHandle(vm, stackID, sHandle_A);
        RE::VMHandle akHandle_B = GetHandle(vm, stackID, sHandle_B);

        if (!sv::vm) {
            logger::error("couldn't get *sv::vm for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
            return false;
        }

        auto arrayData_A = GetArrayProperty(vm, stackID, akHandle_A, bsScriptName_A, bsArrayPropertyName_A);

        if (!arrayData_A.gotAllData) {
            logger::error("failed to get all property data for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
            return false;
        }

        auto arrayData_B = GetArrayProperty(vm, stackID, akHandle_B, bsScriptName_B, bsArrayPropertyName_B);
        if (!arrayData_B.gotAllData) {
            logger::error("failed to get all property data for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
            return false;
        }

        std::string className = GetObjTypeInfoName(arrayData_A.info);
        if (className == "") {
            logger::debug("className for [{}] not found in script[{}] on handle[{}]. Most likely due to array being type kNone from a native array function.", 
				bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
        } 
		
		std::string classNameB = GetObjTypeInfoName(arrayData_B.info);
        if (classNameB == "") {
            logger::debug("className for [{}] not found in script[{}] on handle[{}]. Most likely due to array being type kNone from a native array function.", 
				bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
				
        } else if (className == "") {
			className = classNameB;
		}
		
		// get derived types instead of none type.
		RE::BSScript::TypeInfo typeA = DeriveElementType(arrayData_A.arrayPtr);
		RE::BSScript::TypeInfo typeB = DeriveElementType(arrayData_B.arrayPtr);
		RE::BSScript::TypeInfo  srcType = (typeB.GetUnmangledRawType() != RE::BSScript::TypeInfo::RawType::kNone) ? typeA : typeB;
		if (srcType.GetUnmangledRawType() == RE::BSScript::TypeInfo::RawType::kNone) {
			logger::error("couldn't determine element type for [{}] / [{}] -- both arrays untyped and all elements are None",
						bsArrayPropertyName_A, bsArrayPropertyName_B);
			return false;
		}
		
		if (!TypeInfosMatch(typeA, typeB)){
			logger::error("these types don't match. PropertyA[{}] ScriptA[{}] handleA[{}] - PropertyB[{}] ScriptB[{}] handleB[{}]",
                bsArrayPropertyName_A, bsScriptName_A, akHandle_A, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
				return false;
		}
		
		// if (!ArraysTypesMatch(arrayData_A.arrayPtr, arrayData_B.arrayPtr)) {
		// 	logger::warn("A raw[{:X}] unmangled[{}] info[{}] | B raw[{:X}] unmangled[{}] info[{}]",
		// 		static_cast<std::size_t>(arrayData_A.arrayPtr->type_info().GetRawType()),
		// 		static_cast<std::uint32_t>(arrayData_A.arrayPtr->type_info().GetUnmangledRawType()),
		// 		static_cast<void*>(arrayData_A.arrayPtr->type_info().GetTypeInfo()),
		// 		static_cast<std::size_t>(arrayData_B.arrayPtr->type_info().GetRawType()),
		// 		static_cast<std::uint32_t>(arrayData_B.arrayPtr->type_info().GetUnmangledRawType()),
		// 		static_cast<void*>(arrayData_B.arrayPtr->type_info().GetTypeInfo()));
							
		// 		// return false;
        // }
		
        int sizeA = arrayData_A.arrayPtr->size();
        if (sizeA <= 0) {
            logger::error("[{}] in script[{}] on handle[{}] not initialized",
                bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
            return false;
        }

        int sizeB = arrayData_B.arrayPtr->size();
        if (sizeB <= 0) {
            logger::error("[{}] in script[{}] on handle[{}] not initialized",
                bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
            return false;
        }

        int newSize = (sizeA + sizeB);
        int i = 0;

        RE::BSTSmartPointer<RE::BSScript::Array> newArray;
		
        // sv::vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSize, newArray);
		
		if (!vm->CreateArray1(srcType, newSize, newArray) || !newArray) {
			logger::error("CreateArray1 failed for [{}]", bsArrayPropertyName_A);
			return false;
		}
		
        //this worked to prevent ctd when saving in game.
		//not needed with CreateArray1, only with CreateArray2
        // newArray.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

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

        logger::trace("scriptName[{}] array[{}] type[{}] on handle[{}] merged with \n scriptName[{}] array[{}] type[{}] on handle[{}]. New size[{}] Expected size[{}]",
            bsScriptName_A, bsArrayPropertyName_A, className, akHandle_A, bsScriptName_B, bsArrayPropertyName_B, className, akHandle_B, newArray->size(), newSize);

        return (newArray->size() == newSize);
    }

    bool CopyArray(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::StaticFunctionTag* functionTag, std::string sHandle_A, RE::BSFixedString bsScriptName_A, RE::BSFixedString bsArrayPropertyName_A,
        std::string sHandle_B, RE::BSFixedString bsScriptName_B, RE::BSFixedString bsArrayPropertyName_B) {

        RE::VMHandle akHandle_A = gfuncs::StringToUint64_t(sHandle_A);
        RE::VMHandle akHandle_B = gfuncs::StringToUint64_t(sHandle_B);

        if (!sv::vm) {
            logger::error("couldn't get *sv::vm for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
            return false;
        }

        auto arrayData_A = GetArrayProperty(vm, stackID, akHandle_A, bsScriptName_A, bsArrayPropertyName_A);
        if (!arrayData_A.gotAllData) {
            logger::error("failed to get all property data for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
            return false;
        }

        auto arrayData_B = GetArrayProperty(vm, stackID, akHandle_B, bsScriptName_B, bsArrayPropertyName_B);
        if (!arrayData_B.gotAllData) {
            logger::error("failed to get all property data for [{}] in script[{}] on handle[{}]",
				 bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
            return false;
        }

        std::string className = GetObjTypeInfoName(arrayData_A.info);
        if (className == "") {
            logger::debug("className for [{}] not found in script[{}] on handle[{}]. Most likely due to array being type kNone from a native array function.", 
				bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
        } 
		
		std::string classNameB = GetObjTypeInfoName(arrayData_B.info);
        if (classNameB == "") {
            logger::debug("className for [{}] not found in script[{}] on handle[{}]. Most likely due to array being type kNone from a native array function.", 
				bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
				
        } else if (className == "") {
			className = classNameB;
		}
		
		// get derived types instead of none type.
		RE::BSScript::TypeInfo typeA = DeriveElementType(arrayData_A.arrayPtr);
		RE::BSScript::TypeInfo typeB = DeriveElementType(arrayData_B.arrayPtr);
		RE::BSScript::TypeInfo  srcType = (typeB.GetUnmangledRawType() != RE::BSScript::TypeInfo::RawType::kNone) ? typeA : typeB;
		if (srcType.GetUnmangledRawType() == RE::BSScript::TypeInfo::RawType::kNone) {
			logger::error("couldn't determine element type for [{}] / [{}] -- both arrays untyped and all elements are None",
						bsArrayPropertyName_A, bsArrayPropertyName_B);
			return false;
		}
		
		if (!TypeInfosMatch(typeA, typeB)){
			logger::error("these types don't match. PropertyA[{}] ScriptA[{}] handleA[{}] - PropertyB[{}] ScriptB[{}] handleB[{}]",
                bsArrayPropertyName_A, bsScriptName_A, akHandle_A, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
				return false;
		}

        int sizeA = arrayData_A.arrayPtr->size();
        if (sizeA <= 0) {
            logger::error("[{}] in script[{}] on handle[{}] not initialized",
                bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
            return false;
        }

        int sizeB = arrayData_B.arrayPtr->size();
        if (sizeB <= 0) {
            logger::error("[{}] in script[{}] on handle[{}] not initialized",
                bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
            return false;
        }

        int i = 0;

        RE::BSTSmartPointer<RE::BSScript::Array> newArray;
        // sv::vm->CreateArray2(arrayData_B.arrayPtr->type(), className, sizeA, newArray);
		
		if (!vm->CreateArray1(srcType, sizeA, newArray) || !newArray) {
			logger::error("CreateArray1 failed for [{}]", bsArrayPropertyName_A);
			return false;
		} 
		
        //this worked to prevent ctd when saving in game
		//only needed for createarray1, not createarray2
        // newArray.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

        for (i; i < sizeA; i++) {
            newArray->data()[i] = arrayData_A.arraySmartPtr->data()[i];
        }

        arrayData_B.arrayProperty->SetNone();
        arrayData_B.arrayProperty->SetArray(newArray);

        logger::trace("scriptName[{}] array[{}] type[{}] on handle[{}] copied to \n scriptName[{}] array[{}] type[{}] on handle[{}]. New size[{}] Expected size[{}]",
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

    int CountInArray(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::StaticFunctionTag* functionTag, std::string sHandle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName, int index) {
        RE::VMHandle akHandle = GetHandle(vm, stackID, sHandle); 

        if (!sv::vm) {
            logger::error("couldn't get *sv::vm for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName, bsScriptName, akHandle);
            return false;
        }

        auto arrayData = GetArrayProperty(vm, stackID, akHandle, bsScriptName, bsArrayPropertyName);

        if (!arrayData.gotAllData) {
            logger::error("failed to get all property data for[{}] in script[{}] on Handle[{}]", 
				bsArrayPropertyName, bsScriptName, akHandle);
            return 0;
        }

        int size = arrayData.arrayPtr->size();

        if (size <= 0) {
            logger::error("[{}] in script[{}] on handle[{}] not initialized",
                bsArrayPropertyName, bsScriptName, akHandle);
            return 0;
        }

        if (index < 0) {
            index = (size - 1);
        }
        else if (index >= size) {
            logger::error("index[{}] for [{}] in script[{}] on handle[{}] isn't valid", 
				index, bsArrayPropertyName, bsScriptName, akHandle);
            return 0;
        }

        return CountInBSScriptArray(arrayData.arraySmartPtr, index);
    }

    int RemoveFromArray(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::StaticFunctionTag* functionTag, std::string sHandle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName, int index, bool removeAll) {
        RE::VMHandle akHandle = GetHandle(vm, stackID, sHandle); 

        if (!sv::vm) {
            logger::error("couldn't get *sv::vm for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName, bsScriptName, akHandle);
            return 0;
        }

        auto arrayData = GetArrayProperty(vm, stackID, akHandle, bsScriptName, bsArrayPropertyName);

        if (!arrayData.gotAllData) {
            logger::error("failed to get all property data for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName, bsScriptName, akHandle);
            return 0;
        }

        RE::BSScript::TypeInfo type = DeriveElementType(arrayData.arrayPtr);
		if (type.GetUnmangledRawType() == RE::BSScript::TypeInfo::RawType::kNone) {
			logger::error("couldn't determine element type for [{}] on script[{}]-- array is untyped and all elements are None, aborting.",
						bsArrayPropertyName, bsScriptName);
			return false;
		}
		
        std::string className = GetObjTypeInfoName(arrayData.info);
        if (className == "") {
            logger::debug("className for [{}] not found in script[{}] on handle[{}]. Most likely due to array being type kNone from a native array function.", 
				bsArrayPropertyName, bsScriptName, akHandle);
        }
		
        RE::BSTSmartPointer<RE::BSScript::Array> newArray;

        int newSize = 0;
        int oldSize = arrayData.arrayPtr->size();
        if (oldSize <= 0) {
            logger::error("[{}] in script[{}] on handle[{}] not initialized",
                bsArrayPropertyName, bsScriptName, akHandle);
            return 0;
        }

        if (index < 0) {
            index = (oldSize - 1);
        }

        if (index >= oldSize) {
            logger::error("index[{}] for [{}] in script[{}] on handle[{}] isn't valid", 
				index, bsArrayPropertyName, bsScriptName, akHandle);
            return 0;
        }

        int count = 1;

        if (removeAll) {
            count = CountInBSScriptArray(arrayData.arraySmartPtr, index);
            if (count > 1) {
                int newSize = (oldSize - count);
                if (newSize <= 0) {
                    logger::info("[{}] in script[{}] on handle[{}] is filled entirely with the element at index[{}], setting size to 1",
                        bsArrayPropertyName, bsScriptName, akHandle, index);

                    newSize = 1;
                    count = -1;
                    // sv::vm->CreateArray2(arrayData.arrayPtr->type(), className, newSize, newArray);
                    // newArray.get()->type_info().SetType(arrayData.arrayPtr->type_info().GetRawType());
					
					if (!sv::vm->CreateArray1(type, newSize, newArray)){
						logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName, bsScriptName);
						return false;
					}
					
                    newArray->data()[0] = arrayData.arraySmartPtr->data()[0]; //must set the 0 entry to a valid entry before setting to none or it will break the array
                    newArray->data()[0].SetNone();
                }
                else {
                    // sv::vm->CreateArray2(arrayData.arrayPtr->type(), className, newSize, newArray);
					
					if (!sv::vm->CreateArray1(type, newSize, newArray)){
						logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName, bsScriptName);
						return false;
					}
					
                    int oldIndex = 0;
                    int newIndex = 0;

					// only needed for CreateArray2, not needed for CreateArray1
                    // newArray.get()->type_info().SetType(arrayData.arrayPtr->type_info().GetRawType());
					
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
            // sv::vm->CreateArray2(arrayData.arrayPtr->type(), className, newSize, newArray);

			if (!sv::vm->CreateArray1(type, newSize, newArray)){
				logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName, bsScriptName);
				return false;
			}
			
            if (newSize <= 0) {
                logger::error("[{}] in script[{}] on handle[{}] is already the minimum size 1",
                    bsArrayPropertyName, bsScriptName, akHandle);

                return 0;
            }

            int oldIndex = 0;
            int newIndex = 0;

			// only needed for CreateArray2, not needed for CreateArray1
            // newArray.get()->type_info().SetType(arrayData.arrayPtr->type_info().GetRawType());

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

        arrayData.arrayProperty->SetNone();
        arrayData.arrayProperty->SetArray(newArray);

        logger::trace("scriptName[{}] array[{}] type[{}] on handle[{}] resized from[{}] to[{}]. Expected size[{}], removed elements[{}]",
            bsScriptName, bsArrayPropertyName, className, akHandle, oldSize, newArray->size(), newSize, count);

        return (count);
    }

    bool SliceArray(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::StaticFunctionTag* functionTag, 
		std::string sHandle, RE::BSFixedString bsScriptName, RE::BSFixedString bsArrayPropertyName,
        int startIndex, int endIndex, bool keep) {

        RE::VMHandle akHandle = GetHandle(vm, stackID, sHandle); 

        logger::trace("called");

        if (!sv::vm) {
            logger::error("couldn't get *sv::vm for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName, bsScriptName, akHandle);
            return false;
        }

        auto arrayData = GetArrayProperty(vm, stackID, akHandle, bsScriptName, bsArrayPropertyName);
        if (!arrayData.gotAllData) {
            logger::error("failed to get all property data for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName, bsScriptName, akHandle);
            return false;
        }

        RE::BSScript::TypeInfo type = DeriveElementType(arrayData.arrayPtr);
		if (type.GetUnmangledRawType() == RE::BSScript::TypeInfo::RawType::kNone) {
			logger::error("couldn't determine element type for [{}] on script[{}]-- array is untyped and all elements are None, aborting.",
						bsArrayPropertyName, bsScriptName);
			return false;
		}
		
        std::string className = GetObjTypeInfoName(arrayData.info);
        if (className == "") {
            logger::debug("className for [{}] not found in script[{}] on handle[{}]. Most likely due to array being type kNone from a native array function.", 
				bsArrayPropertyName, bsScriptName, akHandle);
        }

        int size = arrayData.arrayPtr->size();
        if (size <= 0) {
            logger::error("[{}] in script[{}] on handle[{}] not initialized",
                bsArrayPropertyName, bsScriptName, akHandle);
            return false;
        }

        if (endIndex < 0) {
            endIndex = (size - 1);
        }

        if (startIndex > endIndex || startIndex < 0 || startIndex >= size || endIndex >= size) {
            logger::error("[{}] in script[{}] on handle[{}] startIndex[{}] or endIndex[{}] isn't valid",
                bsArrayPropertyName, bsScriptName, akHandle, startIndex, endIndex);

            return false;
        }

        if (startIndex == 0 && endIndex >= (size - 1)) {
            logger::error("[{}] in script[{}] on handle[{}], can't slice the entire array.",
                bsArrayPropertyName, bsScriptName, akHandle);
            return false;
        }

        RE::BSTSmartPointer<RE::BSScript::Array> newArray;
        int newArraySize = 1;

        //keep portion between startIndex and endIndex of array in array
        if (keep) {
            newArraySize = (endIndex - startIndex + 1);

            // sv::vm->CreateArray2(arrayData.arrayPtr->type(), className, newArraySize, newArray);
            // newArray.get()->type_info().SetType(arrayData.arrayPtr->type_info().GetRawType());

			if (!sv::vm->CreateArray1(type, newArraySize, newArray)){
				logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName, bsScriptName);
				return false;
			}
			
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
            // sv::vm->CreateArray2(arrayData.arrayPtr->type(), className, newArraySize, newArray);
            // newArray.get()->type_info().SetType(arrayData.arrayPtr->type_info().GetRawType());

			if (!sv::vm->CreateArray1(type, newArraySize, newArray)){
				logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName, bsScriptName);
				return false;
			}
			
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

        logger::trace("scriptName[{}] array[{}] type[{}] on handle[{}] sliced",
            bsScriptName, bsArrayPropertyName, className, akHandle);

        return (newArray->size() == newArraySize);
    }

    bool SliceArrayOnto(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::StaticFunctionTag* functionTag, std::string sHandle_A, RE::BSFixedString bsScriptName_A, RE::BSFixedString bsArrayPropertyName_A,
        std::string sHandle_B, RE::BSFixedString bsScriptName_B, RE::BSFixedString bsArrayPropertyName_B,
        int startIndex, int endIndex, bool replace, bool keep) {

        RE::VMHandle akHandle_A = gfuncs::StringToUint64_t(sHandle_A);
        RE::VMHandle akHandle_B = gfuncs::StringToUint64_t(sHandle_B);

        if (!sv::vm) {
            logger::error("couldn't get *sv::vm for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
            return false;
        }

        auto arrayData_A = GetArrayProperty(vm, stackID, akHandle_A, bsScriptName_A, bsArrayPropertyName_A);
        if (!arrayData_A.gotAllData) {
            logger::error("failed to get all property data for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
            return false;
        }

        auto arrayData_B = GetArrayProperty(vm, stackID, akHandle_B, bsScriptName_B, bsArrayPropertyName_B);
        if (!arrayData_B.gotAllData) {
            logger::trace("failed to get all property data for [{}] in script[{}] on handle[{}]", 
				bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
            return false;
        }

        std::string className = GetObjTypeInfoName(arrayData_A.info);
        if (className == "") {
            logger::debug("className for [{}] not found in script[{}] on handle[{}]. Most likely due to array being type kNone from a native array function.", 
				bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
        }
		
		std::string classNameB = GetObjTypeInfoName(arrayData_B.info);
        if (classNameB == "") {
            logger::debug("className for [{}] not found in script[{}] on handle[{}]. Most likely due to array being type kNone from a native array function.", 
				bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
				
        } else if (className == "") {
			className = classNameB;
		}
		
		// get derived types instead of none type.
		RE::BSScript::TypeInfo typeA = DeriveElementType(arrayData_A.arrayPtr);
		RE::BSScript::TypeInfo typeB = DeriveElementType(arrayData_B.arrayPtr);
		RE::BSScript::TypeInfo  srcType = (typeB.GetUnmangledRawType() != RE::BSScript::TypeInfo::RawType::kNone) ? typeA : typeB;
		if (srcType.GetUnmangledRawType() == RE::BSScript::TypeInfo::RawType::kNone) {
			logger::error("couldn't determine element type for [{}] / [{}] -- both arrays untyped and all elements are None",
						bsArrayPropertyName_A, bsArrayPropertyName_B);
			return false;
		}
		
		if (!TypeInfosMatch(typeA, typeB)){
			logger::error("these types don't match. PropertyA[{}] ScriptA[{}] handleA[{}] - PropertyB[{}] ScriptB[{}] handleB[{}]",
                bsArrayPropertyName_A, bsScriptName_A, akHandle_A, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
				return false;
		}

		
        int sizeA = arrayData_A.arrayPtr->size();
        if (sizeA <= 0) {
            logger::error("[{}] in script[{}] on handle[{}] not initialized",
                bsArrayPropertyName_A, bsScriptName_A, akHandle_A);
            return false;
        }

        int sizeB = arrayData_B.arrayPtr->size();
        if (sizeB <= 0) {
            logger::error("[{}] in script[{}] on handle[{}] not initialized",
                bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
            return false;
        }

        if (endIndex < 0) {
            endIndex = (sizeA - 1);
        }

        if (startIndex > endIndex || startIndex < 0 || startIndex >= sizeA || endIndex >= sizeA) {
            logger::error("[{}] in script[{}] on handle[{}] to \n [{}] in script[{}] on handle[{}], startIndex[{}] or endIndex[{}] isn't valid",
                bsArrayPropertyName_A, bsScriptName_A, akHandle_A, bsArrayPropertyName_B, bsScriptName_B, akHandle_B, startIndex, endIndex);

            return false;
        }

        if (startIndex == 0 && endIndex >= (sizeA - 1)) {
            logger::error("[{}] in script[{}] on handle[{}] to \n [{}] in script[{}] on handle[{}], can't slice the entire array.",
                bsArrayPropertyName_A, bsScriptName_A, akHandle_A, bsArrayPropertyName_B, bsScriptName_B, akHandle_B);
            return false;
        }

        RE::BSTSmartPointer<RE::BSScript::Array> newArray_A;
        RE::BSTSmartPointer<RE::BSScript::Array> newArray_B;
        int newSizeA = 1;
        int newSizeB = 1;

        if (keep && replace) {
            newSizeA = (endIndex - startIndex + 1);
            newSizeB = (sizeA - newSizeA);

            // sv::vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeA, newArray_A);
            // newArray_A.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

			if (!sv::vm->CreateArray1(srcType, newSizeA, newArray_A)){
				logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName_A, bsScriptName_A);
				return false;
			}
			
            // sv::vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeB, newArray_B);
            // newArray_B.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

			if (!sv::vm->CreateArray1(srcType, newSizeB, newArray_B)){
				logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName_B, bsScriptName_B);
				return false;
			}
			
            int i = 0;
            int ii = startIndex;
            //copy the elements between startIndex and endIndex from array_A to newArray_A
            for (ii; ii <= endIndex; i++, ii++) {
                //logger::trace("newArray_A loop1 i[{}] replace[{}] keep[{}]", i, replace, keep);
                newArray_A->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
            }

            i = 0;

            //copy the remainder of indexes from array_A to array_B
            for (i; i < startIndex; i++) {
                //logger::trace("newArray_B loop1 i[{}] replace[{}] keep[{}]", i, replace, keep);
                newArray_B->data()[i] = arrayData_A.arraySmartPtr->data()[i];
            }

            ii = (endIndex + 1);

            for (i; i < newSizeB; i++, ii++) {
                //logger::trace("newArray_B loop2 i[{}] ii[{}] replace[{}] keep[{}]", i, ii, replace, keep);
                newArray_B->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
            }
        }
        else if (keep && !replace) {
            newSizeA = (endIndex - startIndex + 1);
            newSizeB = (sizeA - newSizeA);
            newSizeB += sizeB;

            // sv::vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeA, newArray_A);
            // newArray_A.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

			if (!sv::vm->CreateArray1(srcType, newSizeA, newArray_A)){
				logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName_A, bsScriptName_A);
				return false;
			}
			
            // sv::vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeB, newArray_B);
            // newArray_B.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

			if (!sv::vm->CreateArray1(srcType, newSizeB, newArray_B)){
				logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName_B, bsScriptName_B);
				return false;
			}
			
            int i = 0;
            int ii = startIndex;
            //copy the elements between startIndex and endIndex from array_A to newArray_A
            for (ii; ii <= endIndex; i++, ii++) {
                //logger::trace("newArray_A loop1 i[{}] ii[{}] replace[{}] keep[{}]", i, ii, replace, keep);
                newArray_A->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
            }

            i = 0;

            //copy the original elements from array_B to newArray_B as we're merging not replacing
            for (i; i < sizeB; i++) {
                //logger::trace("newArray_B loop1 i[{}] replace[{}] keep[{}]", i, replace, keep);
                newArray_B->data()[i] = arrayData_B.arraySmartPtr->data()[i];
            }

            ii = 0;

            //copy the remainder of indexes from array_A to array_B
            for (ii; ii < startIndex; i++, ii++) {
                //logger::trace("newArray_B loop2 i[{}] ii[{}] replace[{}] keep[{}]", i, ii, replace, keep);
                newArray_B->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
            }

            ii = (endIndex + 1);

            for (i; i < newSizeB; i++, ii++) {
                //logger::trace("newArray_B loop3 i[{}] ii[{}] replace[{}] keep[{}]", i, ii, replace, keep);
                newArray_B->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
            }
        }
        else if (!keep && replace) {
            newSizeB = (endIndex - startIndex + 1);
            newSizeA = (sizeA - newSizeB);

            // sv::vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeA, newArray_A);
            // newArray_A.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

			if (!sv::vm->CreateArray1(srcType, newSizeA, newArray_A)){
				logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName_A, bsScriptName_A);
				return false;
			}
			
            // sv::vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeB, newArray_B);
            // newArray_B.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

			if (!sv::vm->CreateArray1(srcType, newSizeB, newArray_B)){
				logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName_B, bsScriptName_B);
				return false;
			}

            int i = 0;
            int ii = startIndex;
            //copy the elements between startIndex and endIndex from array_A to newArray_B
            for (ii; ii <= endIndex; i++, ii++) {
                //logger::trace("newArray_B loop1 i[{}] ii[{}] replace[{}] keep[{}]", i, ii, replace, keep);
                newArray_B->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
            }

            i = 0;

            //copy the remainder of indexes from array_A to newArray_A
            for (i; i < startIndex; i++) {
                //logger::trace("newArray_A loop1 i[{}] replace[{}] keep[{}]", i, replace, keep);
                newArray_A->data()[i] = arrayData_A.arraySmartPtr->data()[i];
            }

            ii = (endIndex + 1);

            for (i; i < newSizeA; i++, ii++) {
                //logger::trace("newArray_A loop2 i[{}] ii[{}] replace[{}] keep[{}]", i, ii, replace, keep);
                newArray_A->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
            }
        }
        else if (!keep && !replace) {
            newSizeB = (endIndex - startIndex + 1);
            newSizeA = (sizeA - newSizeB);
            newSizeB += sizeB;

			// sv::vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeA, newArray_A);
            // newArray_A.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

			if (!sv::vm->CreateArray1(srcType, newSizeA, newArray_A)){
				logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName_A, bsScriptName_A);
				return false;
			}
			
            // sv::vm->CreateArray2(arrayData_B.arrayPtr->type(), className, newSizeB, newArray_B);
            // newArray_B.get()->type_info().SetType(arrayData_B.arrayPtr->type_info().GetRawType());

			if (!sv::vm->CreateArray1(srcType, newSizeB, newArray_B)){
				logger::error("CreateArray1 failed for [{}] on script[{}]", bsArrayPropertyName_B, bsScriptName_B);
				return false;
			}
            
            int i = 0;

            //copy the original elements from array_B to newArray_B as we're merging not replacing
            for (i; i < sizeB; i++) {
                //logger::trace("newArray_B loop1 i[{}] replace[{}] keep[{}]", i, replace, keep);
                newArray_B->data()[i] = arrayData_B.arraySmartPtr->data()[i];
            }

            int ii = startIndex;
            //copy the elements between startIndex and endIndex from array_A to newArray_B
            for (ii; ii <= endIndex; i++, ii++) {
                //logger::trace("newArray_B loop2 i[{}] ii[{}] replace[{}] keep[{}]", i, ii, replace, keep);
                newArray_B->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
            }

            i = 0;

            //copy the remainder of indexes from array_A to newArray_A
            for (i; i < startIndex; i++) {
                //logger::trace("newArray_A loop1 i[{}] replace[{}] keep[{}]", i, replace, keep);
                newArray_A->data()[i] = arrayData_A.arraySmartPtr->data()[i];
            }

            ii = (endIndex + 1);

            for (i; i < newSizeA; i++, ii++) {
                //logger::trace("newArray_A loop2 i[{}] ii[{}] replace[{}] keep[{}]", i, ii, replace, keep);
                newArray_A->data()[i] = arrayData_A.arraySmartPtr->data()[ii];
            }
        }

        arrayData_A.arrayProperty->SetNone();
        arrayData_A.arrayProperty->SetArray(newArray_A);

        arrayData_B.arrayProperty->SetNone();
        arrayData_B.arrayProperty->SetArray(newArray_B);

        logger::trace("scriptName[{}] array[{}] type[{}] on handle[{}] sliced to \n scriptName[{}] array[{}] type[{}] on handle[{}]",
            bsScriptName_A, bsArrayPropertyName_A, className, akHandle_A, bsScriptName_B, bsArrayPropertyName_B, className, akHandle_B);


        return (newArray_A->size() == newSizeA && newArray_B->size() == newSizeB);
    }

    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("GetFormHandle", "PapyrusUtilEx", GetFormHandle);
        vm->RegisterFunction("GetAliasHandle", "PapyrusUtilEx", GetAliasHandle);
        vm->RegisterFunction("GetActiveEffectHandle", "PapyrusUtilEx", GetActiveEffectHandle);
        vm->RegisterFunction("GetActiveEffectFromHandle", "PapyrusUtilEx", GetActiveEffectFromHandle);
        vm->RegisterFunction("ResizeArray", "PapyrusUtilEx", ResizeArrayProperty);
        vm->RegisterFunction("RemoveFromArray", "PapyrusUtilEx", RemoveFromArray);
        vm->RegisterFunction("SliceArray", "PapyrusUtilEx", SliceArray);
        vm->RegisterFunction("SliceArrayOnto", "PapyrusUtilEx", SliceArrayOnto);
        vm->RegisterFunction("CountInArray", "PapyrusUtilEx", CountInArray);
        vm->RegisterFunction("MergeArrays", "PapyrusUtilEx", MergeArrays);
        vm->RegisterFunction("CopyArray", "PapyrusUtilEx", CopyArray);
        return true; 
    }
}
//==============================================================================================================================================
