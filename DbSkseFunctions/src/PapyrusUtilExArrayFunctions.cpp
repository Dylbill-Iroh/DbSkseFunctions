
#include "PapyrusUtilExArrayFunctions.h"
#include "FormTypeTable.h"
#include "GeneralFunctions.h"
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
#define X(ENUM, CLASS, PSC)                                                                                 \
        case FormTypeTable::ExtendedFormType::PSC:                                                          \
            vm->RegisterFunction("Create", scriptName, CreateArray<CLASS>);        			                \
            vm->RegisterFunction("Resize", scriptName, ResizeArray<CLASS>);        			                \
            vm->RegisterFunction("Merge", scriptName, MergeArrays<CLASS>);                                  \
            vm->RegisterFunction("Slice", scriptName, SliceArray<CLASS>);                                   \
            vm->RegisterFunction("Push", scriptName, PushArray<CLASS>);                                     \
            vm->RegisterFunction("Insert", scriptName, InsertIntoArray<CLASS>);                             \
            vm->RegisterFunction("Remove", scriptName, RemoveFromArray<CLASS>);                             \
            vm->RegisterFunction("RemoveAt", scriptName, RemoveFromArrayAt<CLASS>);                         \
            vm->RegisterFunction("RemoveDuplicates", scriptName, RemoveDuplicatesFromArray<CLASS>);         \
            vm->RegisterFunction("Sort", scriptName, SortArray<CLASS>);                                     \
            vm->RegisterFunction("GetStrings", scriptName, GetArrayStrings<CLASS>);                         \
            vm->RegisterFunction("Count", scriptName, CountInArray<CLASS>);                                 \
            logger::info("scriptName[{}]", scriptName);                                                     \
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
				
				auto formTypeCon = GetFormTypeFromPapyrusPropertyName(mainPropertyName);
				if (formTypeCon){
					if (RegisterSkseArrayScript(vm, scriptName, formTypeCon.value())){
						count++;
					}
				} 
				else {
					logger::error("Formtype class for [{}] not found", mainPropertyName);
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