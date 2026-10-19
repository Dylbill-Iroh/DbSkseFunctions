#include <Windows.h>
//#undef GetObject
#include <cstdint>
#include <iostream>
#include <array>
#include <algorithm>
#include <cmath>
#include <wingdi.h>
#include "GeneralFunctions.h"
#include "RE/B/BGSKeyword.h"
#include "RE/B/BGSLocAlias.h"
#include "RE/B/BGSRefAlias.h"
#include "RE/B/BSCoreTypes.h"
#include "RE/B/BSFixedString.h"
#include "RE/O/Object.h"
#include "RE/RTTI.h"
#include "RE/T/TESCondition.h"
#include "RE/T/TESForm.h"
#include "RE/T/TESObjectREFR.h"
#include "RE/T/TESTopic.h"
#include "RE/T/TypeTraits.h"
#include "REL/Version.h"
#include "editorID.hpp"
#include "SharedVariables.h"

namespace gfuncs {
    std::chrono::system_clock::time_point lastTimeGameWasLoaded;

    const std::string hexDigits = "0123456789abcdef";
    const std::string reverseHexDigits = "fedcba9876543210";
	
	// 0000028A, this is the form the game uses as default when using PlaceAtMe with abForcePersistent == true.
	// It promotes this form as an owner of the ref with the BGSDynamicPersistenceManager.
	const RE::FormID promotedPersistenceDefaultFormID = 650; 
	
    bool srandSet = false;

    enum logLevel { trace, debug, info, warn, error, critical };
    enum debugLevel { notification, messageBox };

    void LogAndMessage(std::string message, int logLevel, int debugLevel) {
        switch (logLevel) {
        case trace:
            logger::trace("{}", message);
            break;

        case debug:
            logger::debug("{}", message);
            break;

        case info:
            logger::info("{}", message);
            break;

        case warn:
            logger::warn("{}", message);
            break;

        case error:
            logger::error("{}", message);
            break;

        case critical:
            logger::critical("{}", message);
            break;
        }

        /*switch (debugLevel) {
        case notification:
            RE::DebugNotification(message.data());
            break;
        case messageBox:
            RE::DebugMessageBox(message.data());
            break;
        }*/
    }
	
    void ConvertToLowerCase(std::string& s) {
        transform(s.begin(), s.end(), s.begin(), ::tolower);
    }
	
	std::vector<std::string> StringSplit(std::string s, std::string delim) {
		std::vector<std::string> v;
		
		// Handle empty delimiter edge case to prevent an infinite loop
		if (delim.empty() || s.empty()) {
			v.push_back(s);
			return v;
		}

		size_t start = 0;
		size_t end = s.find(delim);

		while (end != std::string::npos) {
			v.push_back(s.substr(start, end - start));
			start = end + delim.length();
			end = s.find(delim, start);
		}

		// Add the remaining part of the string after the last delimiter
		v.push_back(s.substr(start));

		return v;
	}
	
	std::string RemoveWhiteSpace(std::string s){
		s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) {
			return std::isspace(c);
		}), s.end());
		
		return s;
	}
	
    std::string uint32_to_string(uint32_t value) {
        std::array<char, 4> r;
        r[0] = static_cast<char>((value >> 24) & 0xFF);
        r[1] = static_cast<char>((value >> 16) & 0xFF);
        r[2] = static_cast<char>((value >> 8) & 0xFF);
        r[3] = static_cast<char>(value & 0xFF);
        return std::string{r[0], r[1], r[2], r[3]};
    }
    
    int GetRandomInt(int min, int max) {
        return (min + (rand() % (max - min + 1)));
    }

    bool IsHexString(std::string s) {
        if (s == "") {
            return false;
        }

        auto beginItr = s.begin();

        if (s.find("0x") == 0) {
            beginItr += 2;
        }

        return(std::all_of(beginItr, s.end(), ::isxdigit));
    }

    bool IsDecString(std::string s) {
        if (s == "") {
            return false;
        }

        return(std::all_of(s.begin(), s.end(), ::isdigit));
    }

	int FindNextWhiteSpaceIndexInString(std::string s, int startIndex, bool searchBackwards = false){
		int length = s.length();
		if (!searchBackwards){
			for (int i = startIndex; i < length; i++){
				char c = s.at(i);
				if (std::isspace(c)){
					return i;
				}
			}
		}	
		else {
			for (int i = length; i >= 0; i--){
				char c = s.at(i);
				if (std::isspace(c)){
					return i;
				}
			}
		}	
		return -1;
	}
	
	int FindNextNonWhiteSpaceIndexInString(std::string s, int startIndex, bool searchBackwards = false){
		int length = s.length();
		if (!searchBackwards){
			for (int i = startIndex; i < length; i++){
				char c = s.at(i);
				if (!std::isspace(c)){
					return i;
				}
			}
		}	
		else {
			for (int i = length; i >= 0; i--){
				char c = s.at(i);
				if (!std::isspace(c)){
					return i;
				}
			}
		}	
		return -1;
	}
	
    std::string IntToHex(int i) {
        std::stringstream stream;
        stream << "0x"
            << std::setfill('0') << std::setw(sizeof(int) * 2)
            << std::hex << i;
        return stream.str();
    }

    std::string IntToHexPapyrus(int i) {
        if (i == 0) {
            return "0";
        }

        std::string s = ""; 

        if (i >= 0) {
            while (i > 0) {
                s = (hexDigits.at((i % 16)) + s);
                i /= 16;
            }
        }
        else {
            i = (std::abs(i) - 1);
            while (i > 0) {
                s = (reverseHexDigits.at((i % 16)) + s);
                i /= 16;
            }
        }
        return s;
    }

    int HexToInt(std::string hex) {
        return stoi(hex, 0, 16);
    }

    uint64_t StringToUint64_t(std::string s) {
        uint64_t value = 0;
        std::istringstream iss(s);
        iss >> value; 
        return value;
    }

	uint32_t StringToUint32_t(std::string s) {
        uint32_t value = 0;
        std::istringstream iss(s);
        iss >> value; 
        return value;
    }
	
    //does the string have at least minNumber of c characters in it?
    bool StringHasNCharacters(std::string s, char c, int minNumber) {
        int size = s.size();
        int count = 0;
        for (int i = 0; i < size && count < minNumber; i++) {
            if (s[i] == c) {
                count++;
            }
        }
        return (count >= minNumber);
    }
    //get the int after start char in string s. Example: GetIntAfterCharInString("arrows (21)") returns 21
    int GetIntAfterCharInString(std::string s, char startChar, int iDefault, int startIndex) {
        int iStart = -1;
        int iEnd = -1;
        int L = s.length();
        int i = startIndex;
        while (i < L) {
            if (s.at(i) == startChar) {
                if (isdigit(s.at(i + 1))) {
                    iStart = i + 1;
                    i++;
                    break;
                }
            }
            i++;
        }

        if (iStart != -1) {
            while (i < L) {
                if (!isdigit(s.at(i))) {
                    iEnd = i;
                    break;
                }
                i++;
            }
            if (iEnd == -1) {
                iEnd = L;
            }
            return stoi(s.substr(iStart, (iEnd - iStart)));
        }
        return iDefault;
    }

	//if returns 0, c is invalid
	std::size_t GetUnsignedCharLength(const char* c) {
		if (!c){
			return 0;
		}
		
		if (IsBadReadPtr(c, 1)){
			return 0;
		} 
		
		std::size_t len = strnlen_s(c, 256);
		
		if (len > 0 && len < 256){
			return len;
		}
		
		return 0;
	}
	
	RE::BSScript::IObjectHandlePolicy* GetHandlePolicy() {
		auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		return vm ? vm->GetObjectHandlePolicy() : nullptr;
	}
	
    bool IsFormValid(RE::TESForm* form, bool checkDeleted, bool checkIdentity) {
        if (!form) {
            return false;
        }

        if (IsBadReadPtr(form, sizeof(form))) {
            return false;
        }
        
		const RE::FormID id = form->GetFormID();
		if (id == 0) {
			return false;
		}

		if (checkIdentity) {
			// authoritative: is this pointer still the live form registered for that id?
			if (RE::TESForm::LookupByID(id) != form) {
				return false;
			}
		}
			
        if (checkDeleted) {
            if (form->IsDeleted()) {
                return false;
            }
        }

        return true;
    }

    //forms and formParamNames should be the same size
    bool AllFormsValid(std::vector<RE::TESForm*> forms, std::vector<std::string> formParamNames, std::string callingFunctionName, bool checkDeleted) {
        for (size_t i = 0; i < forms.size(); i++) {
            auto* form = forms[i];
            if (!IsFormValid(form)) {
                logger::warn("[{}] [{}] doesn't exist", callingFunctionName, formParamNames[i]);
                return false;
            }
        }
        return true;
    }

	FormIdEx GetAliasKey(RE::BGSBaseAlias* alias) {
		if (!alias){
			return {0, 0};
		}
		RE::FormID questId = gfuncs::IsFormValid(alias->owningQuest) ? alias->owningQuest->GetFormID() : 0;
		return { questId, alias->aliasID, 0, FormIdExType::kAlias };
	}
	
	RE::BGSBaseAlias* GetAliasFromKey(const FormIdEx& key) {
		auto* quest = RE::TESForm::LookupByID<RE::TESQuest>(key.formId);
		if (!gfuncs::IsFormValid(quest)) { return nullptr; }
		for (auto* alias : quest->aliases) {
			if (alias && alias->aliasID == key.subId) { return alias; }
		}
		return nullptr;
	}
	
	//thanks to shad0wshayd3 for this.
	RE::TESQuestStageItem* GetQuestStageItem(RE::TESQuestStage* questStage, std::uint8_t a_index){
		// auto& mod = REL::Module::get();
		// REL::Version version = mod.version();
		// REL::Version SeVersion = REL::Version(1, 5, 97, 0);
		// REL::Version minAeVersion = REL::Version(1, 7, 99, 0);
		
		// //only have the ids for 1.5.97 and 1.7xx or greater
		// if (version < minAeVersion && version != SeVersion){
		// 	return nullptr;
		// }
		
        using func_t = decltype(&gfuncs::GetQuestStageItem);
        static REL::Relocation<func_t> func{ RELOCATION_ID(24766, 25246) };
        return func(questStage, a_index);
    }
	
    std::string GetFormEditorId(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string nullFormString) {
        if (!IsFormValid(akForm)) {
            return nullFormString;
        }
        else {
            std::string editorId = akForm->GetFormEditorID();
            if (editorId == "") {
                editorId = clib_util::editorID::get_editorID(akForm);
            }
            if (editorId == "") {
                RE::TESObjectREFR* ref = akForm->As<RE::TESObjectREFR>();
                if (IsFormValid(ref)) {
                    RE::TESForm* baseForm = ref->GetBaseObject(); 
                    if (IsFormValid(baseForm)) {
                        editorId = baseForm->GetFormEditorID();
                        if (editorId == "") {
                            editorId = clib_util::editorID::get_editorID(baseForm);
                        }
                    }
                }
            }

            return editorId;
        }
    }

    void SetFormName(RE::TESForm* baseForm, RE::BSFixedString nuName) {
        RE::TESFullName* pFullName = baseForm->As<RE::TESFullName>();
        // is a const string, so have to just reassign it.
        if (pFullName) {
            pFullName->fullName = nuName;
        }
    }

    RE::BSFixedString GetFormName(RE::TESForm* akForm, RE::BSFixedString nullString, RE::BSFixedString noNameString, bool returnIdIfNull) {
        if (!IsFormValid(akForm)) {
            return nullString;
        }

        RE::TESObjectREFR* ref = akForm->As<RE::TESObjectREFR>();
        RE::BSFixedString name;
        if (IsFormValid(ref)) {
            name = ref->GetDisplayFullName();
            if (name == "") {
                auto* baseForm = ref->GetBaseObject();
                if (IsFormValid(baseForm)) {
                    name = baseForm->GetName();
                }
            }
        }
        else {
            name = akForm->GetName();
        }

        if (name == "") {
            if (returnIdIfNull) {
                name = GetFormEditorId(nullptr, akForm, "");
            }
            else {
                name = noNameString;
            }
        }
        return name;
    }

    std::string GetFormNameAndId(RE::TESForm* akForm, std::string nullString, std::string noNameString) {
        if (!IsFormValid(akForm)) {
            return nullString;
        }
		
		std::string name = std::string(GetFormName(akForm, nullString, noNameString, false));
		if (name == nullString){
			return nullString;
		}
		
        std::string id = std::format("{:x}", akForm->GetFormID());
        std::string s = "(name[" + name + "] id[" + id + "])";
        return s;
    }

    std::string GetFormDataString(RE::TESForm* akForm, std::string nullString, std::string noNameString) {
        if (!IsFormValid(akForm)) {
            return nullString;
        }

        std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullString, noNameString, false));

        std::string editorID = GetFormEditorId(nullptr, akForm, "");

        std::string dataString = std::format("(name[{}] editorID[{}] formID[{:x}]) type[{}])", name, editorID, akForm->GetFormID(), akForm->GetFormType());

        return dataString;
    }

	//Specifically for functions bound to papyrus, to find which script called a function. 
	//functionScriptName is the papyrus script name that the papyrus native function is defined in. 
	//EG GetThisScriptName function in Utility.cpp is bound to GetThisScriptName in DbSkseFunctions.psc. 
	//So, you would use gfuncs::GetCallingScriptName(vm, stackID, "DbSkseFunctions"); and it would return "MyScript" 
	//if DbSkseFunctions.GetThisScriptName() was called from MyScript.psc
	std::string GetCallingScriptName(RE::BSScript::Internal::VirtualMachine* vm, RE::VMStackID stackID, RE::BSFixedString functionScriptName) {
		if (!vm) { return ""; }

		RE::BSSpinLockGuard lock(vm->runningStacksLock);

		auto it = vm->allRunningStacks.find(stackID);
		if (it == vm->allRunningStacks.end() || !it->second) { return ""; }

		auto* frame = it->second->top;

		// walk outward past our own frames to the first real caller
		while (frame) {
			if (frame->owningFunction) {
				const auto owner = frame->owningFunction->GetObjectTypeName();
				if (owner != functionScriptName && !owner.empty()) {
					// RE::TESForm* callingForm = frame->self.Unpack<RE::TESForm*>(); //get the form that the calling script is attached to.
					// logger::info("calling form[{}]", gfuncs::GetFormNameAndId(callingForm));
					return std::string(owner);
				}
			}
			frame = frame->previousFrame;
		}
		return "";
	} 
	
	//Specifically for functions bound to papyrus, to find which papyrus function called a function. 
	//functionScriptName is the papyrus script name that the papyrus native function is defined in. 
	//EG GetThisFunctionName function in Utility.cpp is bound to GetThisFunctionName in DbSkseFunctions.psc. 
	//So, you would use gfuncs::GetCallingScriptName(vm, stackID, "DbSkseFunctions"); and it would return "MyPapyrusFunction" 
	//Papyrus: 
	//Function MyPapyrusFunction
	//	string func = DbSkseFunctions.GetThisFunctionName() ;returns "MyPapyrusFunction"
	//EndFunction
	std::string GetCallingScriptFunction(RE::BSScript::Internal::VirtualMachine* vm, RE::VMStackID stackID, RE::BSFixedString functionScriptName) {
		if (!vm) { return ""; }

		RE::BSSpinLockGuard lock(vm->runningStacksLock);

		auto it = vm->allRunningStacks.find(stackID);
		if (it == vm->allRunningStacks.end() || !it->second) { return ""; }

		auto* frame = it->second->top;

		// walk outward past our own frames to the first real caller
		while (frame) {
			if (frame->owningFunction) {
				const auto owner = frame->owningFunction->GetObjectTypeName();
				if (owner != functionScriptName && !owner.empty()) {
					// RE::TESForm* callingForm = frame->self.Unpack<RE::TESForm*>(); //get the form that the calling script is attached to.
					// logger::info("calling form[{}]", gfuncs::GetFormNameAndId(callingForm));
					return std::string(frame->owningFunction->GetName());
				}
			}
			frame = frame->previousFrame;
		}
		return "";
	} 
	
	RE::VMHandle GetCallingScriptHandle(RE::BSScript::Internal::VirtualMachine* vm, RE::VMStackID stackID, RE::BSFixedString functionScriptName) {
		RE::VMHandle handle = 0; 
		
		if (!vm) { return handle; }

		RE::BSSpinLockGuard lock(vm->runningStacksLock);

		auto it = vm->allRunningStacks.find(stackID);
		if (it == vm->allRunningStacks.end() || !it->second) { return handle; }

		auto* frame = it->second->top;

		// walk outward past our own frames to the first real caller
		while (frame) {
			if (frame->owningFunction) {
				const auto owner = frame->owningFunction->GetObjectTypeName();
				if (owner != functionScriptName && !owner.empty()) {
					auto obj = frame->self.GetObject(); 
					if (obj){
						return obj->GetHandle();
					}
				}
			}
			frame = frame->previousFrame;
		}
		
		return handle;
	} 
	
	RE::TESForm* GetCallingScriptForm(RE::BSScript::Internal::VirtualMachine* vm, RE::VMStackID stackID, RE::BSFixedString functionScriptName) {
		RE::TESForm* akForm = nullptr; 
		
		if (!vm) { return akForm; }

		RE::BSSpinLockGuard lock(vm->runningStacksLock);

		auto it = vm->allRunningStacks.find(stackID);
		if (it == vm->allRunningStacks.end() || !it->second) { return akForm; }

		auto* frame = it->second->top;

		// walk outward past our own frames to the first real caller
		while (frame) {
			if (frame->owningFunction) {
				const auto owner = frame->owningFunction->GetObjectTypeName();
				if (owner != functionScriptName && !owner.empty()) {
					akForm = frame->self.Unpack<RE::TESForm*>();
					if (!IsFormValid(akForm)){
						akForm = nullptr;
					}
				}
			}
			frame = frame->previousFrame;
		}
		
		return akForm;
	} 
	
    RE::TESFile* GetFileForForm(RE::TESForm* akForm) {
        if (!gfuncs::IsFormValid(akForm)) {
            return nullptr;
        }

		auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            return nullptr;
        }

        auto id = akForm->GetFormID();

        for (auto* file : dataHandler->files) {
            if (file) {
                if (file->IsFormInMod(id)) {
                    return file;
                }
            }
        }

        return nullptr;
    }

    RE::TESFile* GetFileForRawFormId(RE::FormID rawFormID, RE::TESFile* file) {
        if (!file || file->compileIndex == 0xFF) {
            return nullptr;
        }

        auto rawIndex = (rawFormID & 0xFF000000) >> 24;
		auto* dataHandler = RE::TESDataHandler::GetSingleton();
		
        if (REL::Module::IsVR() && dataHandler ? !dataHandler->VRcompiledFileCollection : false) {
            if (rawIndex >= file->masterCount) {
                return file;
            }
            return file->masterPtrs[rawIndex];
        }
        else {
            bool isLight = rawIndex == 0xFE;
            if (isLight) {
                rawIndex = (rawFormID & 0x00FFF000) >> 12;
            }

            std::uint32_t index = 0;
            for (std::uint32_t i = 0; i < file->masterCount; ++i) {
                auto* master = file->masterPtrs[i];
                if ((master->compileIndex == 0xFE) != isLight)
                { // is isLight = true execute it if master is smth different than light
                    // is isLight = false execute it if master is smth different than full
                    continue;
                }
                if (index++ == rawIndex) {
                    return master;
                }
            }
            return file;
        }
    }

    std::string GetModName(RE::TESForm* form) {
        if (!IsFormValid(form)) {
            return "";
        }

        const auto array = form->sourceFiles.array;
        if (!array || array->empty()) {
            return "";
        }

        const auto file = array->front();
        std::string_view filename = file ? file->GetFilename() : "";

        return (filename.data());
    }

    void logFormMap(auto& map) {
        logger::trace("logging form map");
        for (auto const& x : map)
        {
            RE::TESForm* akForm = x.first;
            if (akForm) {
                logger::trace("Form[{}] ID[{:x}] value[{}]", gfuncs::GetFormName(akForm), akForm->GetFormID(), x.second);
            }
        }
    }

	constexpr float PI = 3.14159265358979323846f;

	float RadiansToDegrees(float radians) {
		return radians * 180.0 / PI;
	}
	
    //return difference of time points in seconds as float
    float timePointDiffToFloat(std::chrono::system_clock::time_point end, std::chrono::system_clock::time_point start) {
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return float(milliseconds.count()) * 0.001;
    }

	RE::RefHandle GetRefHandle(RE::TESObjectREFR* ref) {
		if (!IsFormValid(ref)) {
            logger::warn("ref doesn't exist or isn't valid");
            return NULL;
        } 
		
		RE::ObjectRefHandle handle = ref->GetHandle();     // BSPointerHandle<TESObjectREFR>
		return handle.native_handle();        // the packed uint32
	}
	
    RE::VMHandle GetHandle(RE::TESForm* akForm) {
        if (!IsFormValid(akForm)) {
            logger::warn("akForm doesn't exist or isn't valid");
            return NULL;
        }
		
		// auto* skyrimVm = RE::SkyrimVM::GetSingleton();
        // if (!skyrimVm) {
        //     logger::error("skyrimVm* not found");
        //     return NULL;
        // }

        RE::VMTypeID id = static_cast<RE::VMTypeID>(akForm->GetFormType());
		
		auto* policy = GetHandlePolicy();
		if (!policy) { 
			logger::error("handle policy not found"); 
			return NULL;
		}
		
		// old api
        // RE::VMHandle handle = skyrimVm->handlePolicy.GetHandleForObject(id, akForm);
		
		RE::VMHandle handle = policy->GetHandleForObject(id, akForm);
		return handle;
    }

    RE::VMHandle GetHandle(RE::BGSBaseAlias* akAlias) {
        if (!akAlias) {
            logger::warn("akAlias doesn't exist");
            return NULL;
        }

		// auto* skyrimVm = RE::SkyrimVM::GetSingleton();
        // if (!skyrimVm) {
        //     logger::error("skyrimVm* not found");
        //     return NULL;
        // }

		
		RE::VMTypeID id = RE::BGSBaseAlias::VMTYPEID;
        // RE::VMTypeID id = akAlias->GetVMTypeID();
		
		RE::BGSRefAlias* refAlias = skyrim_cast<RE::BGSRefAlias*>(akAlias); 
		RE::BGSLocAlias* locAlias = skyrim_cast<RE::BGSLocAlias*>(akAlias); 
		
		if (refAlias){
			id = RE::BGSLocAlias::VMTYPEID;
			// id = refAlias->GetVMTypeID();
		}
		else if (locAlias){
			id = RE::BGSLocAlias::VMTYPEID;
			// id = locAlias->GetVMTypeID();
		}
		
		auto* policy = GetHandlePolicy();
		if (!policy) { 
			logger::error("handle policy not found"); 
			return NULL;
		}
		
		//old api 
        // RE::VMHandle handle = skyrimVm->handlePolicy.GetHandleForObject(id, akAlias);
		
		RE::VMHandle handle = policy->GetHandleForObject(id, akAlias);
		return handle;
    }

    RE::VMHandle GetHandle(RE::ActiveEffect* akEffect) {
        if (!akEffect) {
            logger::warn("akEffect doesn't exist");
            return NULL;
        }

		// auto* skyrimVm = RE::SkyrimVM::GetSingleton();
        // if (!skyrimVm) {
        //     logger::error("skyrimVm* not found");
        //     return NULL;
        // }

        RE::VMTypeID id = akEffect->VMTYPEID;
        //RE::VMTypeID id = RE::ActiveEffect::VMTYPEID;
		
		auto* policy = GetHandlePolicy();
		if (!policy) { 
			logger::error("handle policy not found"); 
			return NULL;
		}
		
		RE::VMHandle handle = policy->GetHandleForObject(id, akEffect);
		// old api
        // RE::VMHandle handle = skyrimVm->handlePolicy.GetHandleForObject(id, akEffect);

        return handle;
    }

    RE::ActiveEffect* GetActiveEffectFromHandle(RE::VMHandle handle) {
        RE::ActiveEffect* activeEffect = nullptr;
		
		// auto* skyrimVm = RE::SkyrimVM::GetSingleton();
        // if (!skyrimVm) {
        //     logger::error("skyrimVm* not found");
        //     return activeEffect;
        // }

        RE::VMTypeID id = RE::ActiveEffect::VMTYPEID;
		
		auto* policy = GetHandlePolicy();
		if (!policy) { 
			logger::error("handle policy not found"); 
			return NULL;
		}
		
		auto* obj = policy->GetObjectForHandle(RE::ActiveEffect::VMTYPEID, handle);
		
		//old api
        // auto* obj = skyrimVm->handlePolicy.GetObjectForHandle(id, handle);
        if (obj) {
            activeEffect = static_cast<RE::ActiveEffect*>(obj);
        }
        return activeEffect;
    }

    RE::TESObjectREFR* GetRefFromHandle(RE::RefHandle& handle) {
        if (handle) {
            auto niPtr = RE::TESObjectREFR::LookupByHandle(handle);
            if (niPtr) {
                return niPtr.get();
            }
        }
        return nullptr;
    }

    RE::TESObjectREFR* GetRefFromObjectRefHandle(RE::ObjectRefHandle refHandle) {
        RE::TESObjectREFR* itemReference = nullptr;
        //std::string msg = "";

        if (refHandle) {
            //logger::debug("refHandle found");

            //msg = "Getting ref from refHandle.Get()";
            auto refPtr = refHandle.get();
            if (refPtr) {
                itemReference = refPtr.get();
            }

            if (!IsFormValid(itemReference)) {
                //msg = "Getting ref by RE::TESForm::LookupByID";
                RE::TESForm* refForm = RE::TESForm::LookupByID(refHandle.native_handle());
                if (IsFormValid(refForm)) {
                    itemReference = refForm->AsReference();
                }
            }

            if (!IsFormValid(itemReference)) {
                //msg = "Getting ref from RE::TESObjectREFR::LookupByHandle";
                auto niPtr = RE::TESObjectREFR::LookupByHandle(refHandle.native_handle());
                if (niPtr) {
                    itemReference = niPtr.get();
                }
            }

            /*if (!IsFormValid(itemReference)) {
                msg = "Getting ref by comparing handle to all object references";
                const auto& [allForms, lock] = RE::TESForm::GetAllForms();
				RE::BSReadLockGuard readLock(lock); 
				
                for (auto& [id, form] : *allForms) {
                    if (IsFormValid(form)) {
                        auto* ref = form->AsReference();
                        if (IsFormValid(ref)) {
                            if (ref->GetHandle() == refHandle) {
                                itemReference = ref;
                                break;
                            }
                        }
                    }
                }
            }*/
        }
        if (!IsFormValid(itemReference)) {
            itemReference = nullptr;
        }
        //logger::trace("{}", msg);
        return itemReference;
    }

	const std::unordered_map<std::string, FormTypeTable::ExtendedFormType>& GetPapyrusFormTypeMap() {
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
			for (const auto& [k, v] : GetPapyrusFormTypeMap()) {
				std::string lk = k;
				gfuncs::ConvertToLowerCase(lk);
				m[lk] = v;
			}
			return m;
		}();
		auto it = lowered.find(propertyName);
		return it != lowered.end() ? std::optional{ it->second } : std::nullopt;
	}
	
	bool IsScriptObjectTypeRegistered(RE::BSScript::IVirtualMachine* vm, RE::VMTypeID type) {
		RE::BSTSmartPointer<RE::BSScript::ObjectTypeInfo> classPtr;
		return vm->GetScriptObjectType(type, classPtr) && classPtr;
	}
	
	// returns the papyrus script name currently registered for this type id, or "" if none
	std::string GetRegisteredScriptObjectTypeName(RE::BSScript::IVirtualMachine* vm, RE::VMTypeID type) {
		RE::BSTSmartPointer<RE::BSScript::ObjectTypeInfo> classPtr;
		if (!vm->GetScriptObjectType(type, classPtr) || !classPtr) {
			return "";
		}
		const char* name = classPtr->GetName();
		return name ? name : "";
	}

	bool RegisterObjectType(RE::BSScript::IVirtualMachine* vm, RE::VMTypeID type, const char* scriptName) {
		const std::string existing = GetRegisteredScriptObjectTypeName(vm, type);
		
		if (existing.empty()) {
			vm->RegisterObjectType(type, scriptName);

			// re-query: this forces the VM to load and link the script, and confirms it took
			const std::string after = GetRegisteredScriptObjectTypeName(vm, type);
			if (after.empty()) {
				logger::error("type[{}] registered as [{}] but the VM can't resolve it -- is {}.pex in Data/Scripts?",
					static_cast<int>(type), scriptName, scriptName);
				return false;
			}

			logger::info("type[{}] registered as [{}]", static_cast<int>(type), after);
			return true;
		}

		if (_stricmp(existing.c_str(), scriptName) == 0) {
			return true;   // already ours (or an identical registration)
		}

		logger::warn("type[{}] already registered as [{}] -- [{}] not registered; callers must use the global functions",
			static_cast<int>(type), existing, scriptName);
		return false;
	}
	
	//mutex locked = false
	//Every VMHandle with a script named sScriptName attached, collected in ONE pass over
	//attachedScripts under ONE spin lock acquisition. Call this once and test membership.
	//Never call IsScriptAttachedToHandle in a loop over forms or aliases -- the main thread
	//wants this same spin lock on every script call and will spin instead of rendering.
	std::unordered_set<RE::VMHandle> GetHandlesWithScriptAttached(RE::BSFixedString sScriptName){
		std::unordered_set<RE::VMHandle> handles;

		auto* bssVm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (!bssVm){
			return handles;
		}

		RE::BSSpinLockGuard scriptLock(bssVm->attachedScriptsLock);

		for (auto& [handle, scripts] : bssVm->attachedScripts){
			RE::VMHandle vmHandle = handle;   //the key is const, the param takes a non-const ref
			if (IsScriptAttachedToHandle(vmHandle, sScriptName, false)){
				handles.insert(handle);
			}
		}

		return handles;
	}
	
	std::vector<std::string> GetScriptsAttachedTohandle(RE::VMHandle& handle, bool RemoveDuplicates, bool spinLock) {
		std::vector<std::string> v;
        if (handle == NULL) {
            logger::error("handle is null");
            return v;
        }

		auto* bssVm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
        if (!bssVm) {
            logger::error("vm not found");
            return v;
        }

		if (spinLock){
			RE::BSSpinLockGuard lock(bssVm->attachedScriptsLock);
			auto it = bssVm->attachedScripts.find(handle);
			if (it != bssVm->attachedScripts.end()) {
				//logger::error("it->second.size() = {}", it->second.size());
				for (int i = 0; i < it->second.size(); i++) {
					auto& attachedScript = it->second[i];
					if (attachedScript) {
						auto* script = attachedScript.get();
						if (script) {
							auto info = script->GetTypeInfo();
							if (info) {
								//logger::error("info->name = [{}]", info->name);
								v.push_back((std::string)info->name);
								auto* parentInfo = info->GetParent();
								while (parentInfo){
									v.push_back((std::string)parentInfo->name);
									parentInfo = parentInfo->GetParent();
								}
							}
						}
					}
				}
			}
			else {
				logger::error("vm* couldnt find handle");
			}
		}
		else {
			auto it = bssVm->attachedScripts.find(handle);
			if (it != bssVm->attachedScripts.end()) {
				//logger::error("it->second.size() = {}", it->second.size());
				for (int i = 0; i < it->second.size(); i++) {
					auto& attachedScript = it->second[i];
					if (attachedScript) {
						auto* script = attachedScript.get();
						if (script) {
							auto info = script->GetTypeInfo();
							if (info) {
								//logger::error("info->name = [{}]", info->name);
								v.push_back((std::string)info->name);
								auto* parentInfo = info->GetParent();
								while (parentInfo){
									v.push_back((std::string)parentInfo->name);
									parentInfo = parentInfo->GetParent();
								}
							}
						}
					}
				}
			}
			else {
				logger::error("vm* couldnt find handle");
			}
		}
		
		if (RemoveDuplicates){
			gfuncs::RemoveDuplicates(v);
		}
        return v;
    }
	
	std::vector<std::vector<std::string>> GetScriptHierarchiesOnHandle(RE::VMHandle& handle) {
		std::vector<std::vector<std::string>> v;
        if (handle == NULL) {
            logger::error("handle is null");
            return v;
        }

		auto* bssVm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
        if (!bssVm) {
            logger::error("vm not found");
            return v;
        }

		RE::BSSpinLockGuard lock(bssVm->attachedScriptsLock);
		
        auto it = bssVm->attachedScripts.find(handle);
        if (it != bssVm->attachedScripts.end()) {
            //logger::error("it->second.size() = {}", it->second.size());
            for (int i = 0; i < it->second.size(); i++) {
                auto& attachedScript = it->second[i];
                if (attachedScript) {
                    auto* script = attachedScript.get();
                    if (script) {
                        auto info = script->GetTypeInfo();
                        if (info) {
							std::vector<std::string> subVector;
                            //logger::error("info->name = [{}]", info->name);
                            subVector.push_back((std::string)info->name);
							auto* parentInfo = info->GetParent();
							while (parentInfo){
                            	subVector.push_back((std::string)parentInfo->name);
								parentInfo = parentInfo->GetParent();
							}
							v.push_back(subVector);
                        }
                    }
                }
            }
        }
        else {
            logger::error("vm* couldnt find handle");
        }
		
        return v;
    }
	
	std::vector<std::string> GetScriptHierarchyForScriptOnHandle(RE::VMHandle& handle, std::string scriptName){
		auto scriptChar = scriptName.c_str();
		std::vector<std::vector<std::string>> v = GetScriptHierarchiesOnHandle(handle);
		for (auto vv : v){
			for (auto& script : vv){
				if (_stricmp(scriptChar, script.c_str()) == 0){
					return vv;
				}
			}
		}
		
		return std::vector<std::string>(0);
	}
	
	bool ScriptHasParentOnHandle(RE::VMHandle& handle, std::string scriptName, std::string parentScriptName){
		auto scriptChar = scriptName.c_str();
		auto parentScriptChar = parentScriptName.c_str();
		std::vector<std::string> v = GetScriptHierarchyForScriptOnHandle(handle, scriptName);
		int scriptIndex = -1;
		int parentScriptIndex = -1; 
		for (int i = 0; i < v.size(); i++){
			std::string script = v[i];
			auto c = script.c_str();
			if (_stricmp(scriptChar, c) == 0){
				scriptIndex = i;
			} 
			else if (_stricmp(parentScriptChar, c) == 0){
				parentScriptIndex = i;
			} 
		} 
		
		return (scriptIndex > -1 && parentScriptIndex > -1 && parentScriptIndex > scriptIndex);
	}
	
	bool ScriptHasChildOnHandle(RE::VMHandle& handle, std::string scriptName, std::string childScriptName){
		auto scriptChar = scriptName.c_str();
		auto childScriptChar = childScriptName.c_str();
		std::vector<std::string> v = GetScriptHierarchyForScriptOnHandle(handle, scriptName);
		int scriptIndex = -1;
		int childScriptIndex = -1; 
		for (int i = 0; i < v.size(); i++){
			std::string script = v[i];
			auto c = script.c_str();
			if (_stricmp(scriptChar, c) == 0){
				scriptIndex = i;
			} 
			else if (_stricmp(childScriptChar, c) == 0){
				childScriptIndex = i;
			} 
		} 
		
		return (scriptIndex > -1 && childScriptIndex > -1 && childScriptIndex < scriptIndex);
	}
	
	bool ScriptHasParent(std::string scriptName, std::string parentScriptName){
		auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (!vm){
			return false;
		} 
		
		RE::BSTSmartPointer<RE::BSScript::ObjectTypeInfo> info;
		if (!vm->GetScriptObjectType1(scriptName, info)) {
			return false;
		}

		if (!info){
			return false;
		}
		
		auto scriptChar = scriptName.c_str();
		std::vector<std::string> v;
		
		if (info) {
			bool scriptNameFound = false;
			//logger::error("info->name = [{}]", info->name);
			v.push_back((std::string)info->name);
			if (_stricmp(scriptChar, info->name.c_str()) == 0){
				scriptNameFound = true;
			}
			
			auto* parentInfo = info->GetParent();
			while (parentInfo){
				v.push_back((std::string)parentInfo->name);
				if (_stricmp(scriptChar, parentInfo->name.c_str()) == 0){
					scriptNameFound = true;
				}
				parentInfo = parentInfo->GetParent();
			}
			if (scriptNameFound){
				// return true;
			} 
			else {
				v.clear();
			}
		} 
		
		auto parentScriptChar = parentScriptName.c_str();
		int scriptIndex = -1; 
		int parentScriptIndex = -1; 
		for (int i = 0; i < v.size(); i++){
			std::string script = v[i]; 
			auto c = script.c_str(); 
			if (_stricmp(c, scriptChar) == 0){
				scriptIndex = i;
			}
			else if (_stricmp(c, parentScriptChar) == 0){
				parentScriptIndex = i;
			}
		} 
		
		return (scriptIndex != -1 && parentScriptIndex != -1 && parentScriptIndex > scriptIndex);
	}
	
	bool ScriptHasChild(std::string scriptName, std::string childScriptName){
		auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (!vm){
			return false;
		} 
		
		RE::BSTSmartPointer<RE::BSScript::ObjectTypeInfo> info;
		if (!vm->GetScriptObjectType1(scriptName, info)) {
			return false;
		}

		if (!info){
			return false;
		}
		
		auto scriptChar = scriptName.c_str();
		std::vector<std::string> v;
		
		if (info) {
			bool scriptNameFound = false;
			//logger::error("info->name = [{}]", info->name);
			v.push_back((std::string)info->name);
			if (_stricmp(scriptChar, info->name.c_str()) == 0){
				scriptNameFound = true;
			}
			
			auto* parentInfo = info->GetParent();
			while (parentInfo){
				v.push_back((std::string)parentInfo->name);
				if (_stricmp(scriptChar, parentInfo->name.c_str()) == 0){
					scriptNameFound = true;
				}
				parentInfo = parentInfo->GetParent();
			}
			if (scriptNameFound){
				// return true;
			} 
			else {
				v.clear();
			}
		} 
		
		auto childScriptChar = childScriptName.c_str();
		int scriptIndex = -1; 
		int childScriptIndex = -1; 
		for (int i = 0; i < v.size(); i++){
			std::string script = v[i]; 
			auto c = script.c_str(); 
			if (_stricmp(c, scriptChar) == 0){
				scriptIndex = i;
			}
			else if (_stricmp(c, childScriptChar) == 0){
				childScriptIndex = i;
			}
		} 
		
		return (scriptIndex != -1 && childScriptIndex != -1 && childScriptIndex < scriptIndex);
	}
	
    bool IsScriptAttachedToHandle(RE::VMHandle& handle, RE::BSFixedString& sScriptName, bool spinLock) {
        if (handle == NULL) {
            logger::error("handle is null");
            return false;
        }

		auto scriptChar = sScriptName.c_str();
		
        std::vector<std::string> scripts = GetScriptsAttachedTohandle(handle, true); 
		for (std::string& script : scripts){
			if (_stricmp(scriptChar, script.c_str()) == 0){
				return true;
			}
		}
        return false;
    }

    bool IsScriptAttachedToRef(RE::TESObjectREFR* ref, RE::BSFixedString sScriptName, bool checkIdentity) {
        if (!gfuncs::IsFormValid(ref, false, checkIdentity)) {
            return false;
        }

        RE::VMHandle handle = GetRefHandle(ref);
        return IsScriptAttachedToHandle(handle, sScriptName);
    }

    bool IsScriptAttachedToForm(RE::TESForm* akForm, RE::BSFixedString sScriptName, bool checkIdentity) {
        if (!IsFormValid(akForm, false, checkIdentity)) {
            return false;
        }

        RE::VMHandle handle = gfuncs::GetHandle(akForm);
        return IsScriptAttachedToHandle(handle, sScriptName);
    }

	bool IsScriptAttachedToAlias(RE::BGSBaseAlias* akAlias, RE::BSFixedString sScriptName) {
        if (!akAlias) {
            return false;
        }

        RE::VMHandle handle = gfuncs::GetHandle(akAlias);
        return IsScriptAttachedToHandle(handle, sScriptName);
    }
	
    RE::BSTSmartPointer<RE::BSScript::Object> GetAttachedScriptObject(RE::VMHandle& handle, RE::BSFixedString& sScriptName) {
        if (handle == NULL) {
            return nullptr;
        }

		auto* bssVm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
        if (!bssVm) {
            return nullptr;
        }

		RE::BSSpinLockGuard lock(bssVm->attachedScriptsLock);
		
        auto it = bssVm->attachedScripts.find(handle);
        if (it != bssVm->attachedScripts.end()) {
            for (int i = 0; i < it->second.size(); i++) {
                auto& attachedScript = it->second[i];
                if (attachedScript) {
                    auto* script = attachedScript.get();
                    if (script) {
                        auto info = script->GetTypeInfo();
                        if (info) {
                            if (info->name == sScriptName) {
                                return RE::BSTSmartPointer<RE::BSScript::Object>(script);
                            }
                        }
                    }
                }
            }
        }
        return nullptr;
    }

	bool IsObjectRefOwner(RE::TESObjectREFR* ref, RE::TESForm* akForm, bool checkRef, bool checkForm){
		if (checkRef){
			if (!gfuncs::IsFormValid(ref)){
				logger::warn("ref doesn't exist");
				return false;
			}
		}
	
		if (checkForm){
			if (!gfuncs::IsFormValid(akForm)){
				logger::warn("akForm doesn't exist");
				return false;
			}
		}
		
		auto* promotedRefData = ref->extraList.GetByType<RE::ExtraPromotedRef>();
		if (promotedRefData){
			for (RE::TESForm* form : promotedRefData->promotedRefOwners){
				if (form == akForm){
					return true;
				}
			}
		}
		
		return false;
	}
	
	//is the handle in use by papyrus?
	bool IsVMHandleInUse(RE::VMHandle handle) {
		auto* bssVm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (!bssVm){
			return true; // can't tell -- assume in use
		}
		
		auto* policy = bssVm ? bssVm->GetObjectHandlePolicy() : nullptr;
		if (!policy) { 
			return true; // can't tell -- assume in use
		}   
		
		if (handle == policy->EmptyHandle()) { 
			return false; 
		}

		// true if this variable (or any element of it, if it's an array) refers to form
		auto matches = [&](const RE::BSScript::Variable& var) -> bool {
			if (var.IsObject()) {
				auto obj = var.GetObject();
				return obj && obj->GetHandle() == handle;
			}
			if (var.IsArray()) {
				auto arr = var.GetArray();
				if (!arr) { return false; }
				for (std::uint32_t i = 0; i < arr->size(); i++) {
					const auto& element = (*arr)[i];
					if (element.IsObject()) {
						auto obj = element.GetObject();
						if (obj && obj->GetHandle() == handle) { 
							return true; 
						}
					}
				}
			}
			return false;
		};

		RE::BSSpinLockGuard lock(bssVm->attachedScriptsLock);

		for (auto& [vmHandle, scripts] : bssVm->attachedScripts) {
			for (auto& script : scripts) {
				if (script){
					if (script->type){
						const auto numVars = script->type->GetNumVariables();
						for (std::uint32_t i = 0; i < numVars; i++) {
							if (matches(script->variables[i])) { 
								// logger::trace("handle[{}] is held by script[{}] attached to handle[{}], variable[{}] of [{}]",
									// handle, script->type->GetName(), vmHandle, i, numVars
								// );
								return true; 
							}
						}
					}
				}
			}
		}
		return false;
	}
	
	//is the form in use by papyrus?
	bool IsFormInUse(RE::TESForm* form, bool checkForm, bool checkDeleted, bool checkIdentity, bool checkIsOwner){
		if (checkForm){
			if (!gfuncs::IsFormValid(form, checkDeleted, checkIdentity)) { 
				return false; 
			}
		}
		
		auto handle = gfuncs::GetHandle(form);
		if (IsVMHandleInUse(handle)){
			// logger::debug("IsVMHandleInUse = true");
			return true;
		}
		
		RE::TESObjectREFR* ref = form->AsReference();
		if (gfuncs::IsFormValid(ref)){
			auto* promotedRefData = ref->extraList.GetByType<RE::ExtraPromotedRef>();
			if (promotedRefData){
				if (promotedRefData->promotedRefOwners.size() > 0){
					// logger::debug("promotedRefData->promotedRefOwners.size() = [{}]", promotedRefData->promotedRefOwners.size());
					return true;
				}
			}
			
			auto aliasArray = ref->extraList.GetByType<RE::ExtraAliasInstanceArray>();
			if (aliasArray) {
				//logger::trace("alias array for ref[{}] found", ref->GetDisplayFullName());
				for (auto* akAlias : aliasArray->aliases) {
					if (akAlias) {
						if (akAlias->alias) {
							if (gfuncs::IsFormValid(akAlias->alias->owningQuest)) {
								for (auto* alias : akAlias->alias->owningQuest->aliases) {
									//akAlias-alias is a const, this is a workaround to get the non const alias without using const_cast
									if (alias == akAlias->alias) {
										RE::BGSRefAlias* refAlias = skyrim_cast<RE::BGSRefAlias*>(alias);
										if (refAlias) {
											if (refAlias->GetReference() == ref){
												return true;
												// logger::debug("ref is in alias");
											}
										}
										break;
									}
								}
							}
						}
					}
				}
			}
		}
		
		bool isOwner = false;
		if (checkIsOwner){
			const auto& [allForms, lock] = RE::TESForm::GetAllForms();
			RE::BSReadLockGuard readLock(lock); 
			
			for (auto& [id, akForm] : *allForms) {
				auto* akRef = akForm->AsReference();
				if (gfuncs::IsFormValid(akRef)){
					if (IsObjectRefOwner(akRef, form, false, false)){
						// logger::debug("form is owner of akRef[{}]", gfuncs::GetFormNameAndId(akRef));
						isOwner = true;
						break;
					}
				}
			}
		}
			
		return isOwner;
	}
	
	//is the alias in use by papyrus?
	bool IsAliasInUse(RE::BGSBaseAlias* alias){
		if (!alias){
			return false;
		}
		
		auto handle = gfuncs::GetHandle(alias);
		return IsVMHandleInUse(handle);
	}
	
	//is the activeEffect in use by papyrus?
	bool IsActiveEffectInUse(RE::ActiveEffect* activeEffect){
		if (!activeEffect){
			return false;
		}
		
		auto handle = gfuncs::GetHandle(activeEffect);
		return IsVMHandleInUse(handle);
	}
	
    RE::TESObjectREFR* GetDialogueTarget(RE::Actor* actor) {
        RE::TESObjectREFR* ref = nullptr;
        if (gfuncs::IsFormValid(actor)) {
            ref = GetRefFromObjectRefHandle(actor->GetActorRuntimeData().dialogueItemTarget);
            if (!gfuncs::IsFormValid(ref)) {
                ref = nullptr;
            }
        }
        return ref;
    }

    RE::Actor* GetPlayerDialogueTarget() {
		auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            logger::error("player not found");
            return nullptr;
        }

        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (IsFormValid(form, false, false)) {
                RE::Actor* actor = skyrim_cast<RE::Actor*>(form);
                if (IsFormValid(actor)) {
                    RE::TESObjectREFR* ref = GetRefFromObjectRefHandle(actor->GetActorRuntimeData().dialogueItemTarget);
                    if (IsFormValid(ref)) {
                        RE::Actor* dialogueActorRef = skyrim_cast<RE::Actor*>(ref);
                        if (IsFormValid(dialogueActorRef)) {
                            if (dialogueActorRef == player) {
                                return actor;
                            }
                        }
                    }
                }
            }
        }
        return nullptr;
    }

    void RefreshItemMenu() {
		auto* ui = RE::UI::GetSingleton();
        if (!ui) {
            logger::error("ui* not found");
            return;
        }

        if (ui->IsItemMenuOpen()) {
			auto* player = RE::PlayerCharacter::GetSingleton();
            if (!player) {
                logger::error("player not found");
                return;
            }

            RE::SendUIMessage::SendInventoryUpdateMessage(player, nullptr);
        }
    }

    bool IsRefActivatedMenu(RE::BSFixedString menu) {
        if (menu == RE::DialogueMenu::MENU_NAME) { return true; }
        if (menu == RE::LockpickingMenu::MENU_NAME) { return true; }
        if (menu == RE::ContainerMenu::MENU_NAME) { return true; }
        if (menu == RE::BarterMenu::MENU_NAME) { return true; }
        if (menu == RE::CraftingMenu::MENU_NAME) { return true; }
        if (menu == RE::BookMenu::MENU_NAME) { return true; }
        if (menu == RE::GiftMenu::MENU_NAME) { return true; }
        return false;
    }

    bool IsRefActivatedMenuOpen() {
		auto* ui = RE::UI::GetSingleton();
        if (ui) {
            if (ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) { return true; }
            if (ui->IsMenuOpen(RE::LockpickingMenu::MENU_NAME)) { return true; }
            if (ui->IsMenuOpen(RE::ContainerMenu::MENU_NAME)) { return true; }
            if (ui->IsMenuOpen(RE::BarterMenu::MENU_NAME)) { return true; }
            if (ui->IsMenuOpen(RE::CraftingMenu::MENU_NAME)) { return true; }
            if (ui->IsMenuOpen(RE::BookMenu::MENU_NAME)) { return true; }
            if (ui->IsMenuOpen(RE::GiftMenu::MENU_NAME)) { return true; }
        }
        return false;
    }

    bool SetAliasQuestObjectFlag(RE::BGSBaseAlias* akAlias, bool set) {
        if (!akAlias) {
            return false;
        }
        if (akAlias->IsQuestObject() != set) {
            if (set) {
                akAlias->flags.set(RE::BGSBaseAlias::FLAGS::kQuestObject);
            }
            else {
                akAlias->flags.reset(RE::BGSBaseAlias::FLAGS::kQuestObject);
            }
        }
        return (akAlias->IsQuestObject() == set);
    }

    bool IsAliasQuestObjectFlagSet(RE::BGSBaseAlias* akAlias) {
        if (!akAlias) {
            return false;
        }
        return akAlias->IsQuestObject();
    }

    bool IsQuestObject(RE::TESObjectREFR* ref) {
        if (!IsFormValid(ref)) {
            return false;
        }

        auto aliasArray = ref->extraList.GetByType<RE::ExtraAliasInstanceArray>();
        if (aliasArray) {
            //logger::trace("alias array for ref[{}] found", ref->GetDisplayFullName());
            for (auto* akAlias : aliasArray->aliases) {
                if (akAlias) {
                    if (akAlias->alias) {
                        if (akAlias->alias->IsQuestObject()) {
                            return true;
                        }
                    }
                }
            }
        }
        //else {
            //logger::trace("alias array for ref[{}] not found", ref->GetDisplayFullName());
        //}
        return false;
    }

    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    bool ContainerContainsRef(RE::TESObjectREFR* containerRef, RE::TESObjectREFR* ref, bool checkContainerRef, bool checkRefIdentity, bool checkRef) {
        if (checkContainerRef){ // in case this function is used in a loop and containerRef is already checked with IsFormValid
			if (!IsFormValid(containerRef)) {
				return false;
			}
		}

		if (checkRef){
			if (!IsFormValid(ref, false, checkRefIdentity)) {
				return false;
			}
		}

        auto* baseCont = containerRef->GetBaseObject();
        if (!IsFormValid(baseCont)) {
            return false;
        }

        auto* container = skyrim_cast<RE::TESObjectCONT*>(baseCont);
        if (!IsFormValid(container)) {
            auto* npc = skyrim_cast<RE::TESNPC*>(baseCont);
            if (!IsFormValid(npc)) {
                return false;
            }
        }

        using func_t = std::int32_t(*)(RE::TESObjectREFR*, RE::TESForm*, std::int64_t, std::int32_t);
        REL::Relocation<func_t> func{ REL::VariantID(56062, 56497, 0x9E4710) };
        std::int32_t result = func(containerRef, ref, NULL, NULL);
        return (result == 1);
    }

    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    std::int32_t GetBaseFormCount(RE::TESObjectREFR* containerRef, RE::TESBoundObject* akForm) {
        std::int32_t result = 0;

        if (!IsFormValid(containerRef)) {
            return result;
        }

        if (!IsFormValid(akForm)) {
            return result;
        }

        auto* baseCont = containerRef->GetBaseObject();
        if (!IsFormValid(baseCont)) {
            return result;
        }

        auto* container = skyrim_cast<RE::TESObjectCONT*>(baseCont);
        if (!IsFormValid(container)) {
            auto* npc = skyrim_cast<RE::TESNPC*>(baseCont);
            if (!IsFormValid(npc)) {
                return result;
            }
        }

        using func_t = std::int32_t(*)(RE::TESObjectREFR*, RE::TESForm*, std::int64_t, std::int32_t);
        REL::Relocation<func_t> func{ REL::VariantID(56062, 56497, 0x9E4710) };
        result = func(containerRef, akForm, NULL, NULL);
        return result;
    }

    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    std::int32_t GetItemCount(RE::TESObjectREFR* containerRef, RE::TESForm* item) {
        std::int32_t result = 0;

        if (!IsFormValid(containerRef)) {
            return result;
        }

        if (!IsFormValid(item)) {
            return result;
        } 

        auto* baseCont = containerRef->GetBaseObject();
        if (!IsFormValid(baseCont)) {
            return result;
        } 

        auto* container = skyrim_cast<RE::TESObjectCONT*>(baseCont);
        if (!IsFormValid(container)) {
            auto* npc = skyrim_cast<RE::TESNPC*>(baseCont);
            if (!IsFormValid(npc)) {
                return result;
            }
        }

        auto* boundObj = skyrim_cast<RE::TESBoundObject*>(item);
        if (!IsFormValid(boundObj)) {
            auto* ref = skyrim_cast<RE::TESObjectREFR*>(item);
            if (!IsFormValid(ref)) {
                return result;
            }
        }

        using func_t = std::int32_t(*)(RE::TESObjectREFR*, RE::TESForm*, std::int64_t, std::int32_t);
        REL::Relocation<func_t> func{ REL::VariantID(56062, 56497, 0x9E4710) };
        result = func(containerRef, item, NULL, NULL);
        return result;
    }

    bool formIsBowOrCrossbow(RE::TESForm* akForm) {
        if (!gfuncs::IsFormValid(akForm)) {
            return false;
        }
        RE::TESObjectWEAP* weapon = akForm->As<RE::TESObjectWEAP>();
        if (gfuncs::IsFormValid(weapon)) {
            if (weapon->IsBow() || weapon->IsCrossbow()) {
                return true;
            }
        }
        return false;
    }

    void String_ReplaceAll(std::string& s, std::string searchString, std::string replaceString) {
        if (s == "" || searchString == "") {
            return;
        }

        int sSize = searchString.size();
        std::size_t index = s.find(searchString);

        while (index != std::string::npos)
        {
            s.replace(index, sSize, replaceString);
            index = s.find(searchString);
        }
    }

    void String_ReplaceAll(std::string& s, std::vector<std::string> searchStrings, std::vector<std::string> replaceStrings) {
        int m = searchStrings.size();
        if (replaceStrings.size() < m) {
            m = replaceStrings.size();
        }
        for (int i = 0; i < m; i++) {
            String_ReplaceAll(s, searchStrings[i], replaceStrings[i]);
        }
    }

    bool StringContainsStringInVector(std::vector<std::string>& v, std::string value) {
        gfuncs::ConvertToLowerCase(value);
        for (auto& s : v) {
            if (value.find(s) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    int GetIndexInVector(std::vector<uint32_t>& v, uint32_t& element) {
        if (element == NULL) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }
	
    int GetIndexInVector(std::vector<RE::TESForm*>& v, RE::TESForm* element) {
        if (!IsFormValid(element)) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::BGSProjectile*>& v, RE::BGSProjectile* element) {
        if (!IsFormValid(element)) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::BSSoundHandle*>& v, RE::BSSoundHandle* element) {
        if (!element) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        for (int i = 0; i < v.size(); i++) {
            if (v[i] == element) {
                return i;
            }
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::BSSoundHandle> v, RE::BSSoundHandle element) {
        if (v.size() == 0) {
            return -1;
        }

        for (int i = 0; i < v.size(); i++) {
            RE::BSSoundHandle handle = v[i];
            if (&handle == &element) {
                return i;
            }
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::VMHandle> v, RE::VMHandle& element) {
        if (element == NULL) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::BSFixedString> v, RE::BSFixedString& element) {
        if (element == NULL) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

    int GetIndexInVector(std::vector<std::string> v, std::string& element) {
        if (element == "") {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::TESObjectREFR*> v, RE::TESObjectREFR* element) {
        if (!IsFormValid(element)) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

	int GetIndexInVector(std::vector<RE::TESConditionItem*> v, RE::TESConditionItem* element) {
        if (!(element)) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }
	
    bool IsObjectInBSTArray(RE::BSTArray<RE::ObjectRefHandle>* v, RE::ObjectRefHandle& element) {
        if (v->size() == 0) {
            return false;
        }

        auto it = std::find(v->begin(), v->end(), element);
        return (it != v->end());
    }

    RE::NiAVObject* GetNiAVObjectForRef(RE::TESObjectREFR* ref) {
        if (!IsFormValid(ref)) {
            return nullptr;
        }

        RE::NiAVObject* obj = ref->Get3D();
        if (!obj) {
            obj = ref->Get3D1(false);

            if (!obj) {
                obj = ref->Get3D2();

                if (!obj) {
                    obj = ref->Get3D1(true);
                }
            }
        }

        return obj;
    }

    RE::BGSBaseAlias* GetQuestAliasById(RE::TESQuest* quest, int id) {
        if (gfuncs::IsFormValid(quest)) {
            if (quest->aliases.size() > 0) {
                for (int i = 0; i < quest->aliases.size(); i++) {
                    if (quest->aliases[i]) {
                        if (quest->aliases[i]->aliasID == id) {
                            return quest->aliases[i];
                        }
                    }
                }
            }
        }

        return nullptr;
    }

    int GetWeatherType(RE::TESWeather* weather) {
        if (gfuncs::IsFormValid(weather)) {
            const auto flags = weather->data.flags;
            if (flags.any(RE::TESWeather::WeatherDataFlag::kNone)) {
                return -1;
            }
            if (flags.any(RE::TESWeather::WeatherDataFlag::kPleasant)) {
                return 0;
            }
            if (flags.any(RE::TESWeather::WeatherDataFlag::kCloudy)) {
                return 1;
            }
            if (flags.any(RE::TESWeather::WeatherDataFlag::kRainy)) {
                return 2;
            }
            if (flags.any(RE::TESWeather::WeatherDataFlag::kSnow)) {
                return 3;
            }
        }
        return -2;
    }

    RE::BSFixedString GetBSUIMessageDataTypeString(RE::BSUIMessageData* msgData) {
        if (msgData) {
			auto* userEvents = RE::UserEvents::GetSingleton();
            if (userEvents) {
                if (msgData->fixedStr == userEvents->forward) {
                    return "forward";
                }
                else if (msgData->fixedStr == userEvents->back) {
                    return "back";
                }
                else if (msgData->fixedStr == userEvents->strafeLeft) {
                    return "strafeLeft";
                }
                else if (msgData->fixedStr == userEvents->strafeRight) {
                    return "strafeRight";
                }
                else if (msgData->fixedStr == userEvents->move) {
                    return "move";
                }
                else if (msgData->fixedStr == userEvents->look) {
                    return "look";
                }
                else if (msgData->fixedStr == userEvents->activate) {
                    return "activate";
                }
                else if (msgData->fixedStr == userEvents->leftAttack) {
                    return "leftAttack";
                }
                else if (msgData->fixedStr == userEvents->rightAttack) {
                    return "rightAttack";
                }
                else if (msgData->fixedStr == userEvents->dualAttack) {
                    return "dualAttack";
                }
                else if (msgData->fixedStr == userEvents->forceRelease) {
                    return "forceRelease";
                }
                else if (msgData->fixedStr == userEvents->pause) {
                    return "pause";
                }
                else if (msgData->fixedStr == userEvents->readyWeapon) {
                    return "readyWeapon";
                }
                else if (msgData->fixedStr == userEvents->togglePOV) {
                    return "togglePOV";
                }
                else if (msgData->fixedStr == userEvents->jump) {
                    return "jump";
                }
                else if (msgData->fixedStr == userEvents->journal) {
                    return "journal";
                }
                else if (msgData->fixedStr == userEvents->sprint) {
                    return "sprint";
                }
                else if (msgData->fixedStr == userEvents->sneak) {
                    return "sneak";
                }
                else if (msgData->fixedStr == userEvents->shout) {
                    return "shout";
                }
                else if (msgData->fixedStr == userEvents->kinectShout) {
                    return "kinectShout";
                }
                else if (msgData->fixedStr == userEvents->grab) {
                    return "grab";
                }
                else if (msgData->fixedStr == userEvents->run) {
                    return "run";
                }
                else if (msgData->fixedStr == userEvents->toggleRun) {
                    return "toggleRun";
                }
                else if (msgData->fixedStr == userEvents->autoMove) {
                    return "autoMove";
                }
                else if (msgData->fixedStr == userEvents->quicksave) {
                    return "quicksave";
                }
                else if (msgData->fixedStr == userEvents->quickload) {
                    return "quickload";
                }
                else if (msgData->fixedStr == userEvents->newSave) {
                    return "newSave";
                }
                else if (msgData->fixedStr == userEvents->inventory) {
                    return "inventory";
                }
                else if (msgData->fixedStr == userEvents->stats) {
                    return "stats";
                }
                else if (msgData->fixedStr == userEvents->map) {
                    return "map";
                }
                else if (msgData->fixedStr == userEvents->screenshot) {
                    return "screenshot";
                }
                else if (msgData->fixedStr == userEvents->multiScreenshot) {
                    return "multiScreenshot";
                }
                else if (msgData->fixedStr == userEvents->console) {
                    return "console";
                }
                else if (msgData->fixedStr == userEvents->cameraPath) {
                    return "cameraPath";
                }
                else if (msgData->fixedStr == userEvents->tweenMenu) {
                    return "tweenMenu";
                }
                else if (msgData->fixedStr == userEvents->takeAll) {
                    return "takeAll";
                }
                else if (msgData->fixedStr == userEvents->accept) {
                    return "accept";
                }
                else if (msgData->fixedStr == userEvents->cancel) {
                    return "cancel";
                }
                else if (msgData->fixedStr == userEvents->up) {
                    return "up";
                }
                else if (msgData->fixedStr == userEvents->down) {
                    return "down";
                }
                else if (msgData->fixedStr == userEvents->left) {
                    return "left";
                }
                else if (msgData->fixedStr == userEvents->right) {
                    return "right";
                }
                else if (msgData->fixedStr == userEvents->pageUp) {
                    return "pageUp";
                }
                else if (msgData->fixedStr == userEvents->pageDown) {
                    return "pageDown";
                }
                else if (msgData->fixedStr == userEvents->pick) {
                    return "pick";
                }
                else if (msgData->fixedStr == userEvents->pickNext) {
                    return "pickNext";
                }
                else if (msgData->fixedStr == userEvents->pickPrevious) {
                    return "pickPrevious";
                }
                else if (msgData->fixedStr == userEvents->cursor) {
                    return "cursor";
                }
                else if (msgData->fixedStr == userEvents->kinect) {
                    return "kinect";
                }
                else if (msgData->fixedStr == userEvents->sprintStart) {
                    return "sprintStart";
                }
                else if (msgData->fixedStr == userEvents->sprintStop) {
                    return "sprintStop";
                }
                else if (msgData->fixedStr == userEvents->sneakStart) {
                    return "sneakStart";
                }
                else if (msgData->fixedStr == userEvents->sneakStop) {
                    return "sneakStop";
                }
                else if (msgData->fixedStr == userEvents->blockStart) {
                    return "blockStart";
                }
                else if (msgData->fixedStr == userEvents->blockStop) {
                    return "blockStop";
                }
                else if (msgData->fixedStr == userEvents->blockBash) {
                    return "blockBash";
                }
                else if (msgData->fixedStr == userEvents->attackStart) {
                    return "attackStart";
                }
                else if (msgData->fixedStr == userEvents->attackPowerStart) {
                    return "attackPowerStart";
                }
                else if (msgData->fixedStr == userEvents->reverseDirection) {
                    return "reverseDirection";
                }
                else if (msgData->fixedStr == userEvents->unequip) {
                    return "unequip";
                }
                else if (msgData->fixedStr == userEvents->zoomIn) {
                    return "zoomIn";
                }
                else if (msgData->fixedStr == userEvents->zoomOut) {
                    return "zoomOut";
                }
                else if (msgData->fixedStr == userEvents->rotateItem) {
                    return "rotateItem";
                }
                else if (msgData->fixedStr == userEvents->leftStick) {
                    return "leftStick";
                }
                else if (msgData->fixedStr == userEvents->prevPage) {
                    return "prevPage";
                }
                else if (msgData->fixedStr == userEvents->nextPage) {
                    return "nextPage";
                }
                else if (msgData->fixedStr == userEvents->prevSubPage) {
                    return "prevSubPage";
                }
                else if (msgData->fixedStr == userEvents->nextSubPage) {
                    return "nextSubPage";
                }
                else if (msgData->fixedStr == userEvents->leftEquip) {
                    return "leftEquip";
                }
                else if (msgData->fixedStr == userEvents->rightEquip) {
                    return "rightEquip";
                }
                else if (msgData->fixedStr == userEvents->toggleFavorite) {
                    return "toggleFavorite";
                }
                else if (msgData->fixedStr == userEvents->favorites) {
                    return "favorites";
                }
                else if (msgData->fixedStr == userEvents->hotkey1) {
                    return "hotkey1";
                }
                else if (msgData->fixedStr == userEvents->hotkey2) {
                    return "hotkey2";
                }
                else if (msgData->fixedStr == userEvents->hotkey3) {
                    return "hotkey3";
                }
                else if (msgData->fixedStr == userEvents->hotkey4) {
                    return "hotkey4";
                }
                else if (msgData->fixedStr == userEvents->hotkey5) {
                    return "hotkey5";
                }
                else if (msgData->fixedStr == userEvents->hotkey6) {
                    return "hotkey6";
                }
                else if (msgData->fixedStr == userEvents->hotkey7) {
                    return "hotkey7";
                }
                else if (msgData->fixedStr == userEvents->hotkey8) {
                    return "hotkey8";
                }
                else if (msgData->fixedStr == userEvents->quickInventory) {
                    return "quickInventory";
                }
                else if (msgData->fixedStr == userEvents->quickMagic) {
                    return "quickMagic";
                }
                else if (msgData->fixedStr == userEvents->quickStats) {
                    return "quickStats";
                }
                else if (msgData->fixedStr == userEvents->quickMap) {
                    return "quickMap";
                }
                else if (msgData->fixedStr == userEvents->toggleCursor) {
                    return "toggleCursor";
                }
                else if (msgData->fixedStr == userEvents->wait) {
                    return "wait";
                }
                else if (msgData->fixedStr == userEvents->click) {
                    return "click";
                }
                else if (msgData->fixedStr == userEvents->mapLookMode) {
                    return "mapLookMode";
                }
                else if (msgData->fixedStr == userEvents->equip) {
                    return "equip";
                }
                else if (msgData->fixedStr == userEvents->dropItem) {
                    return "dropItem";
                }
                else if (msgData->fixedStr == userEvents->rotate) {
                    return "rotate";
                }
                else if (msgData->fixedStr == userEvents->nextFocus) {
                    return "nextFocus";
                }
                else if (msgData->fixedStr == userEvents->prevFocus) {
                    return "prevFocus";
                }
                else if (msgData->fixedStr == userEvents->setActiveQuest) {
                    return "setActiveQuest";
                }
                else if (msgData->fixedStr == userEvents->placePlayerMarker) {
                    return "placePlayerMarker";
                }
                else if (msgData->fixedStr == userEvents->xButton) {
                    return "xButton";
                }
                else if (msgData->fixedStr == userEvents->yButton) {
                    return "yButton";
                }
                else if (msgData->fixedStr == userEvents->chargeItem) {
                    return "chargeItem";
                }
                else if (msgData->fixedStr == userEvents->unk318) {
                    return "unk318";
                }
                else if (msgData->fixedStr == userEvents->playerPosition) {
                    return "playerPosition";
                }
                else if (msgData->fixedStr == userEvents->localMap) {
                    return "localMap";
                }
                else if (msgData->fixedStr == userEvents->localMapMoveMode) {
                    return "localMapMoveMode";
                }
                else if (msgData->fixedStr == userEvents->itemZoom) {
                    return "itemZoom";
                }
            }
        }
        return "Unrecognized";
    }

    void RemoveDuplicates(std::vector<std::string>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    } 

    void RemoveDuplicates(std::vector<RE::FormID>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }

    void RemoveDuplicates(std::vector<RE::VMHandle>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }

    void RemoveDuplicates(std::vector<RE::TESForm*>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }
	
    void RemoveDuplicates(std::vector<RE::TESTopic*>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }
	
    void RemoveDuplicates(std::vector<RE::TESTopicInfo*>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }

	void RemoveDuplicates(std::vector<RE::BGSKeyword*>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }
	
    void CombineEventHandles(std::vector<RE::VMHandle>& handles, RE::TESForm* akForm, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& formHandles) {
        if (formHandles.size() == 0) {
            return;
        }

        if (!IsFormValid(akForm)) {
            return;
        }

        auto it = formHandles.find(akForm);

        if (it != formHandles.end()) {
            logger::trace("events for form: [{}] ID[{:x}] found", GetFormName(akForm), akForm->GetFormID());
            handles.reserve(handles.size() + it->second.size());
            handles.insert(handles.end(), it->second.begin(), it->second.end());
        }
        else {
            logger::trace("events for form: [{}] ID[{:x}] not found", GetFormName(akForm), akForm->GetFormID());
        }
    }

    void SendEvents(std::vector<RE::VMHandle> handles, RE::BSFixedString& sEvent, RE::BSScript::IFunctionArguments* args) {
        int max = handles.size();

		auto* skyrimVm = RE::SkyrimVM::GetSingleton();
        if (max == 0) {
        	delete args; //args is created using makeFunctionArguments. Delete as it's no longer needed.
            return;
        }

        if (!skyrimVm) {
            logger::error("skyrimVm* not found");
        	delete args; //args is created using makeFunctionArguments. Delete as it's no longer needed.
            return;
        }

        for (int i = 0; i < max; i++) {
            skyrimVm->SendAndRelayEvent(handles[i], &sEvent, args, nullptr);
        }

        delete args; //args is created using makeFunctionArguments. Delete as it's no longer needed.
    }

    RE::TESForm* FindNullForm() {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 

        for (auto& [id, form] : *allForms) {
            if (form) {
                if (form->GetFormType() == RE::FormType::None) {
                    logger::trace("null TESForm* found");
                    return form;
                }
            }
        }
        return nullptr;
    }
	
	RE::BSTSmartPointer<RE::BSScript::Object> AttachPapyrusScriptToHandle(RE::VMHandle& handle, const char* scriptName) {
		RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
		auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto* policy = vm ? vm->GetObjectHandlePolicy() : nullptr;
		if (!vm || !policy) { 
			return object; 
		}
		
		if (handle == policy->EmptyHandle()) { 
			return object; 
		}

		if (!vm->CreateObject2(scriptName, object) || !object) { 
			return object; 
		}
		
		vm->BindObject(object, handle, false);
		return object;
	}
	
	//using this also unattaches all parent scripts and invalidates the form / alias. Use with caution.
	bool UnattachPapyrusScriptFromHandle(RE::VMHandle& handle, const char* scriptName) {
		auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto* policy = vm ? vm->GetObjectHandlePolicy() : nullptr;
		if (!vm || !policy || handle == policy->EmptyHandle()) {
			return false;
		}

		RE::BSTSmartPointer<RE::BSScript::Object> object;
		// std::string parentScriptName;
		{
			RE::BSSpinLockGuard lock(vm->attachedScriptsLock);

			auto it = vm->attachedScripts.find(handle);
			if (it == vm->attachedScripts.end()) {
				return false;
			}

			for (auto& script : it->second) {
				if (!script) { continue; }
				auto* info = script->GetTypeInfo();
				if (info && _stricmp(info->GetName(), scriptName) == 0) {
					RE::BSScript::Object* objPtr = script.get();  
					if (objPtr){
						object = RE::BSTSmartPointer<RE::BSScript::Object>(objPtr); // smart pointer keeps it alive past the unlock
					}
					break;
				}
			}
		}

		if (!object) {
			logger::debug("script[{}] not attached to handle[{}]", scriptName, handle);
			return false;
		}
		
		logger::debug("script[{}]", scriptName);
		
		vm->UnbindObject(object);
		return true;
	}
	
	RE::BSTSmartPointer<RE::BSScript::Object> AttachPapyrusScriptToForm(RE::TESForm* form, const char* scriptName) {
		RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
		if (!IsFormValid(form)){
			logger::warn("form doesn't exist");
			return object;
		}
		RE::VMHandle handle = GetHandle(form);
		object = AttachPapyrusScriptToHandle(handle, scriptName);
		return object;
	}
	
	RE::BSTSmartPointer<RE::BSScript::Object> AttachPapyrusScriptToAlias(RE::BGSBaseAlias* alias, const char* scriptName) {
		RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
		if (!(alias)){
			return object;
		}
		RE::VMHandle handle = GetHandle(alias);
		object = AttachPapyrusScriptToHandle(handle, scriptName);
		return object;
	}
	
	bool UnattachPapyrusScriptFromForm(RE::TESForm* form, const char* scriptName) {
		RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
		if (!IsFormValid(form)){
			return false;
		}
		RE::VMHandle handle = GetHandle(form);
		return UnattachPapyrusScriptFromHandle(handle, scriptName);
	}
	
	bool UnattachPapyrusScriptFromAlias(RE::BGSBaseAlias* alias, const char* scriptName) {
		RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
		if (!(alias)){
			return false;
		}
		RE::VMHandle handle = GetHandle(alias);
		return UnattachPapyrusScriptFromHandle(handle, scriptName);
	}
	
    void Install() {
        if (!srandSet) {
            srandSet = true;
            std::srand((unsigned)std::time(NULL));
        }
    }
}
