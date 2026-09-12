
#include "PapyrusUtilExArrayFunctions.h"
#include "FormTypeTable.h"
#include "GeneralFunctions.h"
#include "RE/B/BSFixedString.h"
#include "SharedVariables.h"
#include "FileSystem.h"
#include "RE/B/BGSKeyword.h"
#include "RE/RTTI.h"
#include "RE/T/TESForm.h"
#include "RE/T/TypeTraits.h"
#include <cstddef>
#include <string>
#include <vector>

namespace papyrusUtilExArrayFunctions {
	std::vector<std::string> functionTypeNames {
		"Create",
		"Resize",
		"Merge",
		"Push",
		"Insert",
		"Remove",
		"RemoveAt",
		"Sort",
		"Count"
	};
	
	enum class ArrayFunctionType : int {
        kCreate = 0,
        kResize,
        kMerge,
        kPush,
        kInsert,
        kRemove,
        kRemoveAt,
        kSort,
        kCount
    };

	const std::unordered_map<std::string, FormTypeTable::ExtendedFormType>& PapyrusNameToFormType() {
        static const std::unordered_map<std::string, FormTypeTable::ExtendedFormType> map = {
#define X(ENUM, CLASS, PSC) { #PSC,  FormTypeTable::ExtendedFormType::PSC },
            DB_MASTER_TYPE_LIST
#undef X
        };
        return map;
    }
	
	std::optional<FormTypeTable::ExtendedFormType> GetFormTypeFromPapyrusPropertyName(std::string propertyName) {
		gfuncs::ConvertToLowerCase(propertyName);
		static const auto lowered = [] {
			std::unordered_map<std::string, FormTypeTable::ExtendedFormType> m;
			for (const auto& [k, v] : PapyrusNameToFormType()) {
				std::string lk = k;
				gfuncs::ConvertToLowerCase(lk);
				m[lk] = v;
			}
			return m;
		}();
		auto it = lowered.find(propertyName);
		return it != lowered.end() ? std::optional{ it->second } : std::nullopt;
	}
	
	bool RegisterSkseArrayScript(RE::BSScript::Internal::VirtualMachine* vm, std::string scriptName, FormTypeTable::ExtendedFormType type) {

        if (!vm) { return false; }
        
        switch (type) {
#define X(ENUM, CLASS, PSC)                                                                                     \
        case FormTypeTable::ExtendedFormType::PSC:                                                              \
            vm->RegisterFunction("Create", scriptName, CreateArray<CLASS>);        			                    \
            vm->RegisterFunction("Resize", scriptName, ResizeArray<CLASS>);        			                    \
            vm->RegisterFunction("Merge", scriptName, MergeArrays<CLASS>);                                      \
            vm->RegisterFunction("Slice", scriptName, SliceArray<CLASS>);                                       \
            vm->RegisterFunction("Push", scriptName, PushArray<CLASS>);                                         \
            vm->RegisterFunction("Insert", scriptName, InsertIntoArray<CLASS>);                                 \
            vm->RegisterFunction("Remove", scriptName, RemoveFromArray<CLASS>);                                 \
            vm->RegisterFunction("RemoveAt", scriptName, RemoveFromArrayAt<CLASS>);                             \
            vm->RegisterFunction("RemoveDuplicates", scriptName, RemoveDuplicatesFromArray<CLASS>);             \
            vm->RegisterFunction("Sort", scriptName, SortArray<CLASS>);                                         \
            vm->RegisterFunction("GetStrings", scriptName, GetArrayStrings<CLASS>);                             \
            vm->RegisterFunction("Count", scriptName, CountInArray<CLASS>);                                     \
			vm->RegisterFunction("IsEqual", scriptName, IsArrayEqualTo<CLASS>);                                 \
			vm->RegisterFunction("GetIndexes", scriptName, GetIndexes<CLASS>);                                  \
			vm->RegisterFunction("GetRemoveDuplicatesIndexes", scriptName, GetRemoveDuplicatesIndexes<CLASS>);  \
			vm->RegisterFunction("RemoveIndexes", scriptName, RemoveIndexes<CLASS>);                            \
			vm->RegisterFunction("GetSortIndexes", scriptName, GetSortIndexes<CLASS>);                          \
			vm->RegisterFunction("SortByIndexes", scriptName, SortByIndexes<CLASS>);                            \
            logger::info("scriptName[{}] type[{}]", scriptName, static_cast<int>(type));                        \
            return true;
            DB_MASTER_TYPE_LIST
#undef X
        default:
            logger::warn("form type [{}] has no Papyrus script class -- nothing registered", (int)type);
            return false;
        }
    }
	
	// inner: typeA is already resolved to TA
	template <class TA>
    requires PapyrusObject<TA>
	bool RegisterArrayAsInner(RE::BSScript::Internal::VirtualMachine* vm, const std::string& scriptName, 
		FormTypeTable::ExtendedFormType typeB, const std::string& functionName) {
			
#ifdef DB_FULL_ARRAY_CONVERSIONS
		switch (typeB) {
	#define X(ENUM, CLASS, PSC)                                                          \
		case FormTypeTable::ExtendedFormType::PSC:                                       \
			if constexpr (std::derived_from<TA, CLASS> || std::derived_from<CLASS, TA>) { \
				vm->RegisterFunction(functionName, scriptName, ArrayAs<TA, CLASS>);      \
				return true;                                                             \
			} else {                                                                     \
				logger::warn("[{}] and [{}] are unrelated -- no conversion", #PSC, functionName); \
				return false;                                                            \
			}
			DB_MASTER_TYPE_LIST
	#undef X
		default:
			return false;
		}
#else
		return false;
#endif
	}

	bool RegisterSkseArrayAsFunction(RE::BSScript::Internal::VirtualMachine* vm, std::string scriptName, 
		FormTypeTable::ExtendedFormType typeA, FormTypeTable::ExtendedFormType typeB, std::string functionName) {
			
		if (!vm) { return false; }
		
#ifdef DB_FULL_ARRAY_CONVERSIONS
		switch (typeA) {
	#define X(ENUM, CLASS, PSC)                                                          \
		case FormTypeTable::ExtendedFormType::PSC:                                       \
			return RegisterArrayAsInner<CLASS>(vm, scriptName, typeB, functionName);
			DB_MASTER_TYPE_LIST
	#undef X
		default:
			logger::warn("type [{}] not supported", static_cast<int>(typeA));
			return false;
		}
#else
		logger::warn("array conversions disabled in this build (DB_FULL_ARRAY_CONVERSIONS off) -- [{}] not registered", functionName);
		return false;
#endif
	}
	
//=====================================================================================================================================================================================
//=============================== Primative Types =====================================================================================================================================
//=====================================================================================================================================================================================
	
	template <class TA>
		requires PapyrusPrimitive<TA>
	bool RegisterPrimitiveArrayAsInner(RE::BSScript::Internal::VirtualMachine* vm, const std::string& scriptName, 
		std::string typeB, const std::string& functionName) {
		
		if (typeB == "bool"){
			vm->RegisterFunction(functionName, scriptName, ArrayAs_p<TA, bool>);  
			return true;
		}
		else if (typeB == "int"){
			vm->RegisterFunction(functionName, scriptName, ArrayAs_p<TA, std::int32_t>);  
			return true;
		}
		else if (typeB == "float"){
			vm->RegisterFunction(functionName, scriptName, ArrayAs_p<TA, float>);  
			return true;
		}
		else if (typeB == "string"){
			vm->RegisterFunction(functionName, scriptName, ArrayAs_p<TA, std::string>);  
			return true;
		}
		return false;
	}

	bool RegisterPrimitiveSkseArrayAsFunction(RE::BSScript::Internal::VirtualMachine* vm, std::string scriptName, 
		std::string typeA, std::string typeB, std::string functionName) {
		if (!vm) { 
			return false; 
		}
		if (typeA == "bool"){
			return RegisterPrimitiveArrayAsInner<bool>(vm, scriptName, typeB, functionName);
		}
		else if (typeA == "int"){
			return RegisterPrimitiveArrayAsInner<std::int32_t>(vm, scriptName, typeB, functionName);
		}
		else if (typeA == "float"){
			return RegisterPrimitiveArrayAsInner<float>(vm, scriptName, typeB, functionName);
		}
		else if (typeA == "string"){
			return RegisterPrimitiveArrayAsInner<std::string>(vm, scriptName, typeB, functionName);
		}
		return false;
	}

	template <class T>
		requires PapyrusPrimitive<T>
	void RegisterPrimitiveType(RE::BSScript::Internal::VirtualMachine* vm, std::string scriptName){
		vm->RegisterFunction("Create", scriptName, CreateArray_p<T>);        			                
		vm->RegisterFunction("Resize", scriptName, ResizeArray_p<T>);        			                
		vm->RegisterFunction("Merge", scriptName, MergeArrays_p<T>);                                  
		vm->RegisterFunction("Slice", scriptName, SliceArray_p<T>);                                   
		vm->RegisterFunction("Push", scriptName, PushArray_p<T>);                                     
		vm->RegisterFunction("Insert", scriptName, InsertIntoArray_p<T>);                             
		vm->RegisterFunction("Remove", scriptName, RemoveFromArray_p<T>);                             
		vm->RegisterFunction("RemoveAt", scriptName, RemoveFromArrayAt_p<T>);                         
		vm->RegisterFunction("RemoveDuplicates", scriptName, RemoveDuplicatesFromArray_p<T>);                                     
		vm->RegisterFunction("Sort", scriptName, SortArray_p<T>);                                     
		vm->RegisterFunction("GetStrings", scriptName, GetArrayStrings_p<T>);                         
		vm->RegisterFunction("Count", scriptName, CountInArray_p<T>);                                 
		vm->RegisterFunction("IsEqual", scriptName, IsArrayEqualTo_P<T>);                                 
		vm->RegisterFunction("GetIndexes", scriptName, GetIndexes_p<T>);                                 
		vm->RegisterFunction("GetRemoveDuplicatesIndexes", scriptName, GetRemoveDuplicatesIndexes_p<T>);                                 
		vm->RegisterFunction("RemoveIndexes", scriptName, RemoveIndexes_p<T>);                                 
		vm->RegisterFunction("GetSortIndexes", scriptName, GetSortIndexes_p<T>);                                 
		vm->RegisterFunction("SortByIndexes", scriptName, SortByIndexes_p<T>);                                 
	}
	
	bool RegisterSksePrimitiveArrayScript(RE::BSScript::Internal::VirtualMachine* vm, std::string scriptName, std::string propertyType){
		gfuncs::ConvertToLowerCase(propertyType);
		if (propertyType == "bool"){
			RegisterPrimitiveType<bool>(vm, scriptName);
			return true;
		}
		else if (propertyType == "int"){
			RegisterPrimitiveType<std::int32_t>(vm, scriptName);
			return true;
		}
		else if (propertyType == "float"){
			RegisterPrimitiveType<float>(vm, scriptName);
			return true;
		}
		else if (propertyType == "string"){
			RegisterPrimitiveType<std::string>(vm, scriptName);
			return true;
		}
		return false;
	}
	
//=====================================================================================================================================================================================
	
	std::string GetScriptArrayPropertyType(std::string fileContents, std::string scriptName){
		// Alias[] Function Resize(Alias[] arr, int size, Alias filler = none) Global Native
		
		gfuncs::ConvertToLowerCase((fileContents));
        size_t npos = std::string::npos;
		
		size_t iStart = fileContents.find("resize(", 0);
		if (iStart == npos){
			logger::error("could not find \"resize(\" in [{}]", scriptName);
			return "";
		}
		
		size_t nextLineIndex = fileContents.find("\n", iStart);
		if (nextLineIndex == npos){
			nextLineIndex = fileContents.length();
		}
		
		size_t iEnd = fileContents.find("[]", iStart);
		if (iEnd == npos || iEnd > nextLineIndex){
			logger::error("could not find \"[]\" after \"resize(\" in [{}]", scriptName);
			return "";
		}
		
		iStart += 7; //move iStart to end of "resize("
		
		std::string s = fileContents.substr(iStart, (iEnd - iStart));
		return gfuncs::RemoveWhiteSpace(s);
	}
	
	int RegisterArraAsProperties(RE::BSScript::Internal::VirtualMachine* vm, std::string fileContents, std::string scriptName) {
		//ReferenceAlias[] Function ArrayAsReferenceAlias(Alias[] arr)
		
		gfuncs::ConvertToLowerCase((fileContents));
        size_t npos = std::string::npos;
		
		int count = 0; 
		
		std::string arrayAs = "arrayas";
		std::string sFunction = "function";
		// size_t arrayAsLength = arrayAs.length();
		std::vector<std::string> lines = gfuncs::StringSplit(fileContents, "\n");
		for (std::string line : lines){
			size_t arrayAsIndex = line.find(arrayAs);
			size_t iCommentIndex = line.find(";");
			if (arrayAsIndex != npos){
				if (iCommentIndex != npos && arrayAsIndex > iCommentIndex){
					continue;
				}
				
				size_t iFunctionIndex = line.find(sFunction);
				if (iFunctionIndex != npos){
					size_t iBracketsIndex = line.find("[]");
					if (iBracketsIndex < iFunctionIndex && iBracketsIndex != npos){
						std::string returnType = gfuncs::RemoveWhiteSpace(line.substr(0, iBracketsIndex));
						// logger::debug("scriptName [{}] returnType[{}]", scriptName, returnType);
						iFunctionIndex += sFunction.length();
						size_t pIndex = line.find("(", iFunctionIndex);
						if (pIndex != npos){
							std::string functionName = gfuncs::RemoveWhiteSpace(line.substr(iFunctionIndex, (pIndex - iFunctionIndex)));
							// logger::debug("scriptName [{}] functionName[{}]", scriptName, functionName);
							pIndex++;
							iBracketsIndex = line.find("[]", pIndex);
							if (iBracketsIndex != npos){
								std::string inputType = gfuncs::RemoveWhiteSpace(line.substr(pIndex, (iBracketsIndex - pIndex)));
								gfuncs::ConvertToLowerCase(inputType);
								gfuncs::ConvertToLowerCase(returnType);
								
								if (inputType == returnType){
									logger::error("script[{}], functionName[{}], inputType[{}] == returnType[{}] no arrayAs function unnecessary", 
										scriptName, inputType, functionName, returnType);
									continue;
								}
								
								if	(RegisterPrimitiveSkseArrayAsFunction(vm, scriptName, returnType, inputType, functionName)){
									logger::info("scriptname[{}], returnType[{}] inputType[{}] functionName[{}] registered", 
										scriptName, returnType, inputType, functionName);
								}
								else {
									logger::debug("scriptName [{}] returnType[{}] inputType[{}] functionName[{}]", 
									scriptName, returnType, inputType, functionName);
								
									auto returnPropType = GetFormTypeFromPapyrusPropertyName(returnType);
									auto inputPropType = GetFormTypeFromPapyrusPropertyName(inputType);
									if (returnPropType && inputPropType){
										RegisterSkseArrayAsFunction(vm, scriptName, returnPropType.value(), inputPropType.value(), functionName);
									}
								}
							}
						}
					}
				}
			}
		}
		return count;
	}
	
	int RegisterDbSkseArrayScripts() {
		int count = 0;
		
		std::filesystem::path folderPath = std::filesystem::current_path();
        folderPath.append("Data");
        folderPath.append("Scripts");
        folderPath.append("Source");
        folderPath.append("DbSkseArrays");
		
		logger::info("Getting files in [{}]", folderPath.generic_string());
		auto scriptFiles = fs::GetAllFilesInDirectory(folderPath, ".psc");
		
		RE::BSScript::Internal::VirtualMachine* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		
		for (auto& scriptPath : scriptFiles) {
			if (std::filesystem::exists(scriptPath)) {
				std::string scriptName = scriptPath.stem().string();
				std::string fileContents = fs::GetFileContents(scriptPath);
				std::string mainPropertyName = GetScriptArrayPropertyType(fileContents, scriptName);
				logger::debug("Registering script[{}] scriptname [{}] mainPropertyName[{}]", 
					scriptPath.filename().generic_string(),
					scriptName,
					mainPropertyName
				); 
				
				if (RegisterSksePrimitiveArrayScript(vm, scriptName, mainPropertyName)){
					//property type is a primitive, bool, int, float or string
					logger::info("scriptname[{}], type[{}] registered", scriptName, mainPropertyName);
					count++;
				}
				else {
					auto formTypeCon = GetFormTypeFromPapyrusPropertyName(mainPropertyName);
					if (formTypeCon){
						if (RegisterSkseArrayScript(vm, scriptName, formTypeCon.value())){
							count++;
						}
					} 
					else {
						logger::error("property [{}] on script[{}] not recognized", mainPropertyName, scriptName);
					}
				}
				
				RegisterArraAsProperties(vm, fileContents, scriptName);
			}
			else {
				logger::debug("[{}] not found", scriptPath.generic_string());
			}
		}
		return count;
	}
}