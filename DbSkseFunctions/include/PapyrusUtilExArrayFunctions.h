#pragma once

#include "FormTypeTable.h"
#include "FileSystem.h"
#include "GeneralFunctions.h"
#include "RE/B/BGSBaseAlias.h"
#include "Utility.h"
#include "RE/RTTI.h"
#include <string>
#include <vector>

namespace papyrusUtilExArrayFunctions {

    // ========================================================================
    // TEMPLATES
    // ========================================================================
	
	template <class T>
	concept PapyrusObject =
		std::derived_from<T, RE::TESForm> ||
		std::derived_from<T, RE::BGSBaseAlias> ||
		std::derived_from<T, RE::ActiveEffect>;
	
    template <class T>
        requires PapyrusObject<T>
    std::vector<T*> CreateArray(RE::StaticFunctionTag*, int size, T* fill) {
        if (size <= 0) { return std::vector<T*>(0); }
		return std::vector<T*>(static_cast<std::size_t>(size), fill);
		// return v;
    }
	
	template <class T>
        requires PapyrusObject<T>
    std::vector<T*> ResizeArray(RE::StaticFunctionTag*, std::vector<T*> arr, int newSize, T* fill) {
        if (newSize <= 0) { return std::vector<T*>(0); }
        arr.resize(static_cast<std::size_t>(newSize), fill);
        return arr;
    }
	
    template <class T>
        requires PapyrusObject<T>
    std::vector<T*> MergeArrays(RE::StaticFunctionTag*, std::vector<T*> arr_A, std::vector<T*> arr_B) {
        arr_A.reserve(arr_A.size() + arr_B.size());
        arr_A.insert(arr_A.end(), arr_B.begin(), arr_B.end());
        return arr_A;
    }

	template <class T>
        requires PapyrusObject<T>
    std::vector<T*> SliceArray(RE::StaticFunctionTag*, std::vector<T*> arr, int startIndex, int endIndex) {
		if (endIndex < 0 || endIndex >= arr.size()){
			endIndex = arr.size() - 1;
		}
		
		if (startIndex < 0){
			startIndex = 0;
		} 
		
		if (endIndex < startIndex){
			logger::error("endIndex[{}] is less than startIndex[{}]", endIndex, startIndex);
			return arr;
		}
		
		std::vector<T*> v(arr.begin() + startIndex, arr.begin() + endIndex + 1);
        return v;
    }

	//Slice
	
    template <class T>
        requires PapyrusObject<T>
    std::vector<T*> PushArray(RE::StaticFunctionTag*, std::vector<T*> arr, T* item) {
        arr.push_back(item);
        return arr;
    }

	template <class T>
        requires PapyrusObject<T>
    std::vector<T*> InsertIntoArray(RE::StaticFunctionTag*, std::vector<T*> arr, T* item, int index) {
		int size = arr.size();
		
		if (size == 0 || index < 0 || index >= size) {
			arr.push_back(item);
			return arr;
		}
		
		arr.insert(arr.begin() + index, item);
        return arr;
    }
	
    template <class T>
        requires PapyrusObject<T>
    std::vector<T*> RemoveFromArray(RE::StaticFunctionTag*, std::vector<T*> arr, T* item, bool allInstances) {
        if (allInstances) {
            arr.erase(std::remove(arr.begin(), arr.end(), item), arr.end());
        } else if (auto it = std::find(arr.begin(), arr.end(), item); it != arr.end()) {
            arr.erase(it);
        }
        return arr;
    }

	template <class T>
        requires PapyrusObject<T>
    std::vector<T*> RemoveFromArrayAt(RE::StaticFunctionTag*, std::vector<T*> arr, int index) {
		int size = arr.size();
		
		if (size == 0) {
			return arr;
		}
		
		if (index < 0 || index >= size){
			index = arr.size() - 1;
		}
		
		arr.erase(arr.begin() + index);
        return arr;
    }
	
	template <class T>
        requires PapyrusObject<T>
    int CountInArray(RE::StaticFunctionTag*, std::vector<T*> arr, T* item) {
		int count = 0;
		int size = arr.size();
		
		if (size == 0) {
			return count;
		}
		
		for (auto* form : arr){
			if (form == item){
				count++;
			}
		}
        return count;
    }
	
	// ;returns new alias array that contains the aliass of the passed in akForms array, but sorted. 
	// ;Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
	// ;1 = by alias name ascending, 
	// ;2 = by alias name descending, 
	// ;5 = by alias ID descending, 
	// ;6 = by alias ID descending, 
	template <class T>
        requires std::derived_from<T, RE::BGSBaseAlias> //
	std::vector<T*> SortAliasArray(RE::StaticFunctionTag*, std::vector<T*> aliases, int sortOption) {
		if (aliases.size() == 0) {
			return aliases;
		}
		
		std::vector<T*> returnAliases;

		if (sortOption >= 1 && sortOption <= 4) { //sort by form name, alias has no form name, only editor or aliasname
			std::vector<std::string> aliasNames;
			std::map<std::string, std::vector<T*>> aliasNamesMap;
			
			for (int i = 0; i < aliases.size(); i++) {
				auto* akAlias = aliases[i];
				std::string aliasName = static_cast<std::string>(akAlias != nullptr ? akAlias->aliasName : "");
				aliasNames.push_back(aliasName);
				auto it = aliasNamesMap.find(aliasName);
				if (it == aliasNamesMap.end()) {
					std::vector<T*> aliasNameAliass;
					aliasNameAliass.push_back(akAlias);
					aliasNamesMap[aliasName] = aliasNameAliass;
				}
				else {
					auto& aliasNameAliass = it->second;
					aliasNameAliass.push_back(akAlias);
				}
			}
			std::sort(aliasNames.begin(), aliasNames.end());
			if (sortOption == 2) {
				std::reverse(aliasNames.begin(), aliasNames.end());
			}
			aliasNames.erase(std::unique(aliasNames.begin(), aliasNames.end()), aliasNames.end());

			for (int i = 0; i < aliasNames.size(); i++) {
				auto it = aliasNamesMap.find(aliasNames[i]);
				if (it != aliasNamesMap.end()) {
					auto& v = it->second;
					for (auto* akAlias : v) {
						returnAliases.push_back(akAlias);
					}
				}
			}
		}
		else { //sort by alias id
			std::vector<int> aliasIds;
			std::map<int, std::vector<T*>> aliasIdsMap;

			for (int i = 0; i < aliases.size(); i++) {
				auto* akAlias = aliases[i];
				int aliasID = akAlias != nullptr ? akAlias->aliasID : 0;
				aliasIds.push_back(aliasID);
				auto it = aliasIdsMap.find(aliasID);
				if (it == aliasIdsMap.end()) {
					std::vector<T*> aliasIdAliass;
					aliasIdAliass.push_back(akAlias);
					aliasIdsMap[aliasID] = aliasIdAliass;
				}
				else {
					auto& aliasIdAliass = it->second;
					aliasIdAliass.push_back(akAlias);
				}
			}
			std::sort(aliasIds.begin(), aliasIds.end());
			if (sortOption == 6) {
				std::reverse(aliasIds.begin(), aliasIds.end());
			}

			aliasIds.erase(std::unique(aliasIds.begin(), aliasIds.end()), aliasIds.end());

			for (int i = 0; i < aliasIds.size(); i++) {
				auto it = aliasIdsMap.find(aliasIds[i]);
				if (it != aliasIdsMap.end()) {
					auto& v = it->second;
					for (auto* akAlias : v) {
						returnAliases.push_back(akAlias);
					}
				}
			}
		}
		return returnAliases;
	}
	
	// ;returns new activeEffect array that contains the activeEffects of the passed in akForms array, but sorted. 
	// ;Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
	// ;1 = by activeEffect name ascending, 
	// ;2 = by activeEffect name descending, 
	// ;3 = by activeEffect editor Id name ascending,
	// ;4 = by activeEffect editor Id name descending,
	// ;5 = by activeEffect Id ascending, 
	// ;6 = by activeEffect Id descending
	template <class T>
        requires std::derived_from<T, RE::ActiveEffect>
	std::vector<T*> SortActiveEffectArray(RE::StaticFunctionTag*, std::vector<T*> akActiveEffects, int sortOption) {
		if (akActiveEffects.size() == 0) {
			return akActiveEffects;
		}
		
		std::vector<T*> returnActiveEffects;

		if (sortOption == 1 || sortOption == 2) { //sort by activeEffect name
			std::vector<std::string> activeEffectNames;
			std::map<std::string, std::vector<T*>> activeEffectNamesMap;

			for (int i = 0; i < akActiveEffects.size(); i++) {
				auto* akActiveEffect = akActiveEffects[i];
				auto* baseEffect = akActiveEffect != nullptr ? akActiveEffect->GetBaseObject() : nullptr;
				std::string activeEffectName = static_cast<std::string>(gfuncs::GetFormName(baseEffect, "", "", false));
				activeEffectNames.push_back(activeEffectName);
				auto it = activeEffectNamesMap.find(activeEffectName);
				if (it == activeEffectNamesMap.end()) {
					std::vector<T*> activeEffectNameActiveEffects;
					activeEffectNameActiveEffects.push_back(akActiveEffect);
					activeEffectNamesMap[activeEffectName] = activeEffectNameActiveEffects;
				}
				else {
					auto& activeEffectNameActiveEffects = it->second;
					activeEffectNameActiveEffects.push_back(akActiveEffect);
				}
			}
			std::sort(activeEffectNames.begin(), activeEffectNames.end());
			if (sortOption == 2) {
				std::reverse(activeEffectNames.begin(), activeEffectNames.end());
			}
			activeEffectNames.erase(std::unique(activeEffectNames.begin(), activeEffectNames.end()), activeEffectNames.end());

			for (int i = 0; i < activeEffectNames.size(); i++) {
				auto it = activeEffectNamesMap.find(activeEffectNames[i]);
				if (it != activeEffectNamesMap.end()) {
					auto& v = it->second;
					for (auto* akActiveEffect : v) {
						returnActiveEffects.push_back(akActiveEffect);
					}
				}
			}
		}
		else if (sortOption == 3 || sortOption == 4) { //sort by editorId
			std::vector<std::string> activeEffectNames;
			std::map<std::string, std::vector<T*>> activeEffectNamesMap;

			for (int i = 0; i < akActiveEffects.size(); i++) {
				auto* akActiveEffect = akActiveEffects[i];
				auto* baseEffect = akActiveEffect != nullptr ? akActiveEffect->GetBaseObject() : nullptr;
				std::string activeEffectName = "";
				if (gfuncs::IsFormValid(baseEffect)) {
					activeEffectName = GetFormEditorId(nullptr, baseEffect, "");
				}

				activeEffectNames.push_back(activeEffectName);
				auto it = activeEffectNamesMap.find(activeEffectName);
				if (it == activeEffectNamesMap.end()) {
					std::vector<T*> activeEffectNameActiveEffects;
					activeEffectNameActiveEffects.push_back(akActiveEffect);
					activeEffectNamesMap[activeEffectName] = activeEffectNameActiveEffects;
				}
				else {
					auto& activeEffectNameActiveEffects = it->second;
					activeEffectNameActiveEffects.push_back(akActiveEffect);
				}
			}
			std::sort(activeEffectNames.begin(), activeEffectNames.end());
			if (sortOption == 4) {
				std::reverse(activeEffectNames.begin(), activeEffectNames.end());
			}
			activeEffectNames.erase(std::unique(activeEffectNames.begin(), activeEffectNames.end()), activeEffectNames.end());

			for (int i = 0; i < activeEffectNames.size(); i++) {
				auto it = activeEffectNamesMap.find(activeEffectNames[i]);
				if (it != activeEffectNamesMap.end()) {
					auto& v = it->second;
					for (auto* akActiveEffect : v) {
						returnActiveEffects.push_back(akActiveEffect);
					}
				}
			}
		}
		else { //sort by activeEffect id
			std::vector<int> activeEffectIds;
			std::map<int, std::vector<T*>> activeEffectIdsMap;

			for (int i = 0; i < akActiveEffects.size(); i++) {
				auto* akActiveEffect = akActiveEffects[i];
				auto* baseEffect = akActiveEffect != nullptr ? akActiveEffect->GetBaseObject() : nullptr;
				int activeEffectID = gfuncs::IsFormValid(baseEffect, false, false) ? baseEffect->GetFormID() : 0;
				activeEffectIds.push_back(activeEffectID);
				auto it = activeEffectIdsMap.find(activeEffectID);
				if (it == activeEffectIdsMap.end()) {
					std::vector<T*> activeEffectIdActiveEffects;
					activeEffectIdActiveEffects.push_back(akActiveEffect);
					activeEffectIdsMap[activeEffectID] = activeEffectIdActiveEffects;
				}
				else {
					auto& activeEffectIdActiveEffects = it->second;
					activeEffectIdActiveEffects.push_back(akActiveEffect);
				}
			}
			std::sort(activeEffectIds.begin(), activeEffectIds.end());
			if (sortOption == 6) {
				std::reverse(activeEffectIds.begin(), activeEffectIds.end());
			}

			activeEffectIds.erase(std::unique(activeEffectIds.begin(), activeEffectIds.end()), activeEffectIds.end());

			for (int i = 0; i < activeEffectIds.size(); i++) {
				auto it = activeEffectIdsMap.find(activeEffectIds[i]);
				if (it != activeEffectIdsMap.end()) {
					auto& v = it->second;
					for (auto* akActiveEffect : v) {
						returnActiveEffects.push_back(akActiveEffect);
					}
				}
			}
		}
		
		return returnActiveEffects;
	}
	
	// ;returns new form array that contains the forms of the passed in akForms array, but sorted. 
	// ;Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
	// ;1 = by form name ascending, 
	// ;2 = by form name descending, 
	// ;3 = by form editor Id name ascending,
	// ;4 = by form editor Id name descending,
	// ;5 = by form Id ascending, 
	// ;6 = by form Id descending
	template <class T>
        requires PapyrusObject<T>
	std::vector<T*> SortArray(RE::StaticFunctionTag* functionTag, std::vector<T*> arr, int sortOption) {
		if (arr.size() == 0) {
			return arr;
		}
		
		std::vector<T*> returnForms;
		
		
		if constexpr (std::derived_from<T, RE::BGSBaseAlias>) {
        	returnForms = SortAliasArray(functionTag, arr, sortOption);
		} else if constexpr (std::derived_from<T, RE::ActiveEffect>) {
			returnForms = SortActiveEffectArray(functionTag, arr, sortOption);
		} else {
			

			if (sortOption == 1 || sortOption == 2) { //sort by form name
				std::vector<std::string> formNames;
				std::map<std::string, std::vector<T*>> formNamesMap;

				for (int i = 0; i < arr.size(); i++) {
					auto* akForm = arr[i];
					std::string formName = static_cast<std::string>(gfuncs::GetFormName(akForm, "", "", false));
					formNames.push_back(formName);
					auto it = formNamesMap.find(formName);
					if (it == formNamesMap.end()) {
						std::vector<T*> formNameForms;
						formNameForms.push_back(akForm);
						formNamesMap[formName] = formNameForms;
					}
					else {
						auto& formNameForms = it->second;
						formNameForms.push_back(akForm);
					}
				}
				std::sort(formNames.begin(), formNames.end());
				if (sortOption == 2) {
					std::reverse(formNames.begin(), formNames.end());
				}
				formNames.erase(std::unique(formNames.begin(), formNames.end()), formNames.end());

				for (int i = 0; i < formNames.size(); i++) {
					auto it = formNamesMap.find(formNames[i]);
					if (it != formNamesMap.end()) {
						auto& v = it->second;
						for (auto* akForm : v) {
							returnForms.push_back(akForm);
						}
					}
				}
			}
			else if (sortOption == 3 || sortOption == 4) { //sort by editorId
				std::vector<std::string> formNames;
				std::map<std::string, std::vector<T*>> formNamesMap;

				for (int i = 0; i < arr.size(); i++) {
					auto* akForm = arr[i];
					std::string formName = "";
					if (gfuncs::IsFormValid(akForm)) {
						formName = GetFormEditorId(nullptr, akForm, "");
					}

					formNames.push_back(formName);
					auto it = formNamesMap.find(formName);
					if (it == formNamesMap.end()) {
						std::vector<T*> formNameForms;
						formNameForms.push_back(akForm);
						formNamesMap[formName] = formNameForms;
					}
					else {
						auto& formNameForms = it->second;
						formNameForms.push_back(akForm);
					}
				}
				std::sort(formNames.begin(), formNames.end());
				if (sortOption == 4) {
					std::reverse(formNames.begin(), formNames.end());
				}
				formNames.erase(std::unique(formNames.begin(), formNames.end()), formNames.end());

				for (int i = 0; i < formNames.size(); i++) {
					auto it = formNamesMap.find(formNames[i]);
					if (it != formNamesMap.end()) {
						auto& v = it->second;
						for (auto* akForm : v) {
							returnForms.push_back(akForm);
						}
					}
				}
			}
			else { //sort by form id
				std::vector<int> formIds;
				std::map<int, std::vector<T*>> formIdsMap;

				for (int i = 0; i < arr.size(); i++) {
					auto* akForm = arr[i];
					int formID = gfuncs::IsFormValid(akForm, false, false) ? akForm->GetFormID() : 0;
					formIds.push_back(formID);
					auto it = formIdsMap.find(formID);
					if (it == formIdsMap.end()) {
						std::vector<T*> formIdForms;
						formIdForms.push_back(akForm);
						formIdsMap[formID] = formIdForms;
					}
					else {
						auto& formIdForms = it->second;
						formIdForms.push_back(akForm);
					}
				}
				std::sort(formIds.begin(), formIds.end());
				if (sortOption == 6) {
					std::reverse(formIds.begin(), formIds.end());
				}

				formIds.erase(std::unique(formIds.begin(), formIds.end()), formIds.end());

				for (int i = 0; i < formIds.size(); i++) {
					auto it = formIdsMap.find(formIds[i]);
					if (it != formIdsMap.end()) {
						auto& v = it->second;
						for (auto* akForm : v) {
							returnForms.push_back(akForm);
						}
					}
				}
			}
		}
		return returnForms;
	}
	
// Get Strings ==================================================================================================================================================================================================
	// Get the strings for the passed in array. mode options are:
	// 1 = form names
	// 2 = editor names 
	// 3 = IDs as hexidecimal (formId or AliasId)
	// SortOptions are: 
	// 1 = not sorted 
	// 2 = sorted ascending 
	// 3 = sorted descending
	template <class T>
        requires std::derived_from<T, RE::BGSBaseAlias> //
	std::vector<std::string> GetAliasArrayStrings(RE::StaticFunctionTag*, std::vector<T*> aliases, int mode, int sortOption, 
		std::string nullName, std::string emptyName) {
		
		std::vector<std::string> returnStrings;
		
		if (aliases.size() == 0) {
			return returnStrings;
		}
		

		int size = aliases.size();
		
		if (mode == 1 || mode == 2) { //get names
			for (int i = 0; i < size; i++){
				auto* akAlias = aliases[i];
				std::string name = static_cast<std::string>(akAlias != nullptr ? akAlias->aliasName : nullName);
				if (name.empty()){
					name = emptyName;
				}
				returnStrings.push_back(name);
			}
		} else {
			for (int i = 0; i < size; i++){
				auto* akAlias = aliases[i];
				std::string name = static_cast<std::string>(akAlias != nullptr ? std::format("[:x]", akAlias->aliasID) : "0");
				returnStrings.push_back(name);
			}
		}
		
		if (sortOption >= 1){
			std::sort(returnStrings.begin(), returnStrings.end());
			if (sortOption == 2) {
				std::reverse(returnStrings.begin(), returnStrings.end());
			}
		}
		
		return returnStrings;
	}
	
	// Get the strings for the passed in array. mode options are:
	// 1 = form names
	// 2 = editor names 
	// 3 = IDs as hexidecimal (formId or AliasId)
	// SortOptions are: 
	// 1 = not sorted 
	// 2 = sorted ascending 
	// 3 = sorted descending
	template <class T>
        requires std::derived_from<T, RE::ActiveEffect>   
	std::vector<std::string> GetActiveEffectStrings(RE::StaticFunctionTag*, std::vector<T*> akActiveEffects, int mode, int sortOption, 
		std::string nullName, std::string emptyName) {
			
		std::vector<std::string> returnStrings;
			
		if (akActiveEffects.size() == 0) {
			return returnStrings;
		}
		
		int size = akActiveEffects.size();
		

		if (mode == 1) { //form name
			for (int i = 0; i < size; i++){
				auto* effect = akActiveEffects[i];
				RE::TESForm* baseEffect = effect != nullptr ? effect->GetBaseObject() : nullptr; 
				std::string name = (std::string)gfuncs::GetFormName(baseEffect, nullName, emptyName, false);
				if (name.empty()){
					name = emptyName;
				}
				returnStrings.push_back(name);
			}
		} 
		else if (mode == 2) { //editor name
			for (int i = 0; i < size; i++){
				auto* effect = akActiveEffects[i];
				RE::TESForm* baseEffect = effect != nullptr ? effect->GetBaseObject() : nullptr; 
				std::string name = gfuncs::GetFormEditorId(nullptr, baseEffect, nullName);
				if (name.empty()){
					name = emptyName;
				}
				returnStrings.push_back(name);
			}
		}
		else { //formid
			for (int i = 0; i < size; i++){
				auto* effect = akActiveEffects[i];
				RE::TESForm* baseEffect = effect != nullptr ? effect->GetBaseObject() : nullptr; 
				int id = gfuncs::IsFormValid(baseEffect) ? (int)baseEffect->GetFormID() : 0;
				std::string name = gfuncs::IntToHexPapyrus(id);
				returnStrings.push_back(name);
			}
		}
		
		if (sortOption >= 1){
			std::sort(returnStrings.begin(), returnStrings.end());
			if (sortOption == 2) {
				std::reverse(returnStrings.begin(), returnStrings.end());
			}
		}
		return returnStrings;
	}
	
	// Get the strings for the passed in array. mode options are:
	// 1 = form names
	// 2 = editor names 
	// 3 = IDs as hexidecimal (formId or AliasId)
	// SortOptions are: 
	// 1 = not sorted 
	// 2 = sorted ascending 
	// 3 = sorted descending
	template <class T>
        requires PapyrusObject<T>
	std::vector<std::string> GetArrayStrings(RE::StaticFunctionTag* functionTag, std::vector<T*> arr, int mode, int sortOption, 
		std::string nullName, std::string emptyName) {
			
		std::vector<std::string> returnStrings;
			
		if (arr.size() == 0) {
			return returnStrings;
		}
		
		if constexpr (std::derived_from<T, RE::BGSBaseAlias>) {
			returnStrings = GetAliasArrayStrings(functionTag, arr, mode, sortOption, nullName, emptyName);
		} else if constexpr (std::derived_from<T, RE::ActiveEffect>) {
			returnStrings = GetActiveEffectStrings(functionTag, arr, mode, sortOption, nullName, emptyName);
		} else {
			
			int size = arr.size();

			if (mode == 1) { //form name
				for (int i = 0; i < size; i++){
					auto* form = arr[i];
					std::string name = (std::string)gfuncs::GetFormName(form, nullName, emptyName, false);
					returnStrings.push_back(name);
				}
			} 
			else if (mode == 2) { //editor name
				for (int i = 0; i < size; i++){
					auto* form = arr[i];
					std::string name = gfuncs::GetFormEditorId(nullptr, form, nullName);
					if (name.empty()){
						name = emptyName;
					}
					returnStrings.push_back(name);
				}
			}
			else { //formid
				for (int i = 0; i < size; i++){
					auto* form = arr[i];
					int id = gfuncs::IsFormValid(form) ? (int)form->GetFormID() : 0;
					std::string name = gfuncs::IntToHexPapyrus(id);
					returnStrings.push_back(name);
				}
			}
			
			if (sortOption >= 1){
				std::sort(returnStrings.begin(), returnStrings.end());
				if (sortOption == 2) {
					std::reverse(returnStrings.begin(), returnStrings.end());
				}
			}
		}
		return returnStrings;
	}
	
	//removeNoneBehavior 0 = no entries are removed, return array is the same size as the passed in array. 
	//removeNoneBehavior 1 = only remove an element if the passed in array element is valid but failed to convert to the return type. 
	//removeNoneBehavior 2 = remove all none entries from return array.
	template <class T, class T2>
		requires PapyrusObject<T> && PapyrusObject<T2> && (std::derived_from<T, T2> || std::derived_from<T2, T>)
	std::vector<T*> ArrayAs(RE::StaticFunctionTag*, std::vector<T2*> arr, bool removeNoneArrElements, bool removeFailedToConvertElements) {
		if (arr.size() == 0) {
			return std::vector<T*>(0);
		}
		
		std::vector<T*> v; 
		for (size_t i = 0; i < arr.size(); i++){
			if (arr[i]){
				T* t = skyrim_cast<T*>(arr[i]);
				if (t){
					v.push_back(t);
				}
				else if (!removeFailedToConvertElements){
					v.push_back(nullptr);
				}
			}
			else if (!removeNoneArrElements){
				v.push_back(nullptr);
			}
		}
		return v;
	}
	
	
	
	int RegisterDbSkseArrayScripts();
	
    // bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}
