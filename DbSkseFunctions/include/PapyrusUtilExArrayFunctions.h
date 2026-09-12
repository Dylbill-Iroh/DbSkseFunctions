#pragma once

#include "FormTypeTable.h"
#include "FileSystem.h"
#include "GeneralFunctions.h"
#include "RE/B/BGSBaseAlias.h"
#include "RE/B/BSFixedString.h"
#include "RE/T/TypeTraits.h"
#include "Utility.h"
#include "RE/RTTI.h"
#include <algorithm>
#include <concepts>
#include <cstdint>
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
    std::vector<T*> RemoveDuplicatesFromArray(RE::StaticFunctionTag*, std::vector<T*> arr) {
		int size = arr.size();
		
		if (size == 0) {
			return arr;
		}
		
		std::vector<T*> v; 
		v.push_back(arr[0]);
		for (int i = 1; i < size; i++){
			if (std::find(v.begin(), v.end(), arr[i]) == v.end()){
				v.push_back(arr[i]);
			}
		}
		
		return v;
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
	
	template <class T>
        requires PapyrusObject<T>
    bool IsArrayEqualTo(RE::StaticFunctionTag*, std::vector<T*> arrA, std::vector<T*> arrB){
		if (arrA.size() != arrB.size()){
			return false;
		}
		
		std::int32_t size = arrA.size();
		
		for (std::int32_t i = 0; i < size; i++){
			if (arrA[i] != arrB[i]){
				return false;
			}
		}
		
		return true;
	}
	
	// ;returns new alias array that contains the aliass of the passed in akForms array, but sorted. 
	// ;Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
	// ;1 = by alias name ascending, 
	// ;2 = by alias name descending, 
	// ;5 = by alias ID descending, 
	// ;6 = by alias ID descending, 
	template <class T>
        requires std::derived_from<T, RE::BGSBaseAlias> 
	std::vector<T*> SortAliasArray(RE::StaticFunctionTag*, std::vector<T*> aliases, int sortOption) {
		if (aliases.size() == 0) {
			return aliases;
		}
		
		std::vector<T*> returnValues;

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
			if (sortOption == 2 || sortOption == 4) {
				std::reverse(aliasNames.begin(), aliasNames.end());
			}
			aliasNames.erase(std::unique(aliasNames.begin(), aliasNames.end()), aliasNames.end());

			for (int i = 0; i < aliasNames.size(); i++) {
				auto it = aliasNamesMap.find(aliasNames[i]);
				if (it != aliasNamesMap.end()) {
					auto& v = it->second;
					for (auto* akAlias : v) {
						returnValues.push_back(akAlias);
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
						returnValues.push_back(akAlias);
					}
				}
			}
		}
		return returnValues;
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
		
		std::vector<T*> returnValues;

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
						returnValues.push_back(akActiveEffect);
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
						returnValues.push_back(akActiveEffect);
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
						returnValues.push_back(akActiveEffect);
					}
				}
			}
		}
		
		return returnValues;
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
        requires std::derived_from<T, RE::TESForm>
		//RE::StaticFunctionTag* is necessary here even though this function isn't registered in papyrus so that the signature matches SortArray.
	std::vector<T*> SortFormArray(RE::StaticFunctionTag*, std::vector<T*> arr, int sortOption) {
		if (arr.size() == 0) {
			return arr;
		}
		
		std::vector<T*> returnValues;
	
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
						returnValues.push_back(akForm);
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
						returnValues.push_back(akForm);
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
						returnValues.push_back(akForm);
					}
				}
			}
		}
		return returnValues;
	}
	
	template <class T>
        requires PapyrusObject<T>
	std::vector<T*> SortArray(RE::StaticFunctionTag* functionTag, std::vector<T*> arr, int sortOption) {
		if (arr.size() == 0) {
			return arr;
		}
		
		std::vector<T*> returnValues;
		
		if constexpr (std::derived_from<T, RE::BGSBaseAlias>) {
        	returnValues = SortAliasArray(functionTag, arr, sortOption);
		} else if constexpr (std::derived_from<T, RE::ActiveEffect>) {
			returnValues = SortActiveEffectArray(functionTag, arr, sortOption);
		} else {
			returnValues = SortFormArray(functionTag, arr, sortOption);
		}
		
		return returnValues;
	}
	
// Get sort indexes ==============================================================================================================================================
	// ;returns new alias array that contains the aliass of the passed in akForms array, but sorted. 
	// ;Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
	// ;1 = by alias name ascending, 
	// ;2 = by alias name descending, 
	// ;5 = by alias ID descending, 
	// ;6 = by alias ID descending, 
	template <class T>
        requires std::derived_from<T, RE::BGSBaseAlias> 
	std::vector<std::int32_t> GetSortAliasArrayIndexes(RE::StaticFunctionTag*, std::vector<T*> aliases, int sortOption) {
		
		std::vector<std::int32_t> returnValues;

		if (sortOption >= 1 && sortOption <= 4) { //sort by form name, alias has no form name, only editor or aliasname
			std::vector<std::string> aliasNames;
			std::map<std::string, std::vector<std::int32_t>> aliasNamesMap;
			
			for (int i = 0; i < aliases.size(); i++) {
				auto* akAlias = aliases[i];
				std::string aliasName = static_cast<std::string>(akAlias != nullptr ? akAlias->aliasName : "");
				aliasNames.push_back(aliasName);
				auto it = aliasNamesMap.find(aliasName);
				if (it == aliasNamesMap.end()) {
					std::vector<std::int32_t> indexes;
					indexes.push_back(i);
					aliasNamesMap[aliasName] = indexes;
				}
				else {
					auto& indexes = it->second;
					indexes.push_back(i);
				}
			}
			std::sort(aliasNames.begin(), aliasNames.end());
			if (sortOption == 2 || sortOption == 4) {
				std::reverse(aliasNames.begin(), aliasNames.end());
			}
			aliasNames.erase(std::unique(aliasNames.begin(), aliasNames.end()), aliasNames.end());

			for (int i = 0; i < aliasNames.size(); i++) {
				auto it = aliasNamesMap.find(aliasNames[i]);
				if (it != aliasNamesMap.end()) {
					auto& v = it->second;
					for (auto index : v) {
						returnValues.push_back(index);
					}
				}
			}
		}
		else { //sort by alias id
			std::vector<int> aliasIds;
			std::map<int, std::vector<std::int32_t>> aliasIdsMap;

			for (int i = 0; i < aliases.size(); i++) {
				auto* akAlias = aliases[i];
				int aliasID = akAlias != nullptr ? akAlias->aliasID : 0;
				aliasIds.push_back(aliasID);
				auto it = aliasIdsMap.find(aliasID);
				if (it == aliasIdsMap.end()) {
					std::vector<std::int32_t> indexes;
					indexes.push_back(i);
					aliasIdsMap[aliasID] = indexes;
				}
				else {
					auto& indexes = it->second;
					indexes.push_back(i);
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
					for (auto index : v) {
						returnValues.push_back(index);
					}
				}
			}
		}
		return returnValues;
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
	std::vector<std::int32_t> GetSortActiveEffectArrayIndexes(RE::StaticFunctionTag*, std::vector<T*> akActiveEffects, int sortOption) {
		
		std::vector<std::int32_t> returnValues;

		if (sortOption == 1 || sortOption == 2) { //sort by activeEffect name
			std::vector<std::string> activeEffectNames;
			std::map<std::string, std::vector<std::int32_t>> activeEffectNamesMap;

			for (int i = 0; i < akActiveEffects.size(); i++) {
				auto* akActiveEffect = akActiveEffects[i];
				auto* baseEffect = akActiveEffect != nullptr ? akActiveEffect->GetBaseObject() : nullptr;
				std::string activeEffectName = static_cast<std::string>(gfuncs::GetFormName(baseEffect, "", "", false));
				activeEffectNames.push_back(activeEffectName);
				auto it = activeEffectNamesMap.find(activeEffectName);
				if (it == activeEffectNamesMap.end()) {
					std::vector<std::int32_t> indexes;
					indexes.push_back(i);
					activeEffectNamesMap[activeEffectName] = indexes;
				}
				else {
					auto& indexes = it->second;
					indexes.push_back(i);
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
					for (auto index : v) {
						returnValues.push_back(index);
					}
				}
			}
		}
		else if (sortOption == 3 || sortOption == 4) { //sort by editorId
			std::vector<std::string> activeEffectNames;
			std::map<std::string, std::vector<std::int32_t>> activeEffectNamesMap;

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
					std::vector<std::int32_t> indexes;
					indexes.push_back(i);
					activeEffectNamesMap[activeEffectName] = indexes;
				}
				else {
					auto& indexes = it->second;
					indexes.push_back(i);
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
					for (auto index : v) {
						returnValues.push_back(index);
					}
				}
			}
		}
		else { //sort by activeEffect id
			std::vector<int> activeEffectIds;
			std::map<int, std::vector<std::int32_t>> activeEffectIdsMap;

			for (int i = 0; i < akActiveEffects.size(); i++) {
				auto* akActiveEffect = akActiveEffects[i];
				auto* baseEffect = akActiveEffect != nullptr ? akActiveEffect->GetBaseObject() : nullptr;
				int activeEffectID = gfuncs::IsFormValid(baseEffect, false, false) ? baseEffect->GetFormID() : 0;
				activeEffectIds.push_back(activeEffectID);
				auto it = activeEffectIdsMap.find(activeEffectID);
				if (it == activeEffectIdsMap.end()) {
					std::vector<std::int32_t> indexes;
					indexes.push_back(i);
					activeEffectIdsMap[activeEffectID] = indexes;
				}
				else {
					auto& indexes = it->second;
					indexes.push_back(i);
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
					for (auto index : v) {
						returnValues.push_back(index);
					}
				}
			}
		}
		
		return returnValues;
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
        requires std::derived_from<T, RE::TESForm>
		//RE::StaticFunctionTag* is necessary here even though this function isn't registered in papyrus so that the signature matches SortArray.
	std::vector<std::int32_t> GetSortFormArrayIndexes(RE::StaticFunctionTag*, std::vector<T*> arr, int sortOption) {
		std::vector<std::int32_t> returnValues;
	
		if (sortOption == 1 || sortOption == 2) { //sort by form name
			std::vector<std::string> formNames;
			std::map<std::string, std::vector<std::int32_t>> formNamesMap;

			for (int i = 0; i < arr.size(); i++) {
				auto* akForm = arr[i];
				std::string formName = static_cast<std::string>(gfuncs::GetFormName(akForm, "", "", false));
				formNames.push_back(formName);
				auto it = formNamesMap.find(formName);
				if (it == formNamesMap.end()) {
					std::vector<std::int32_t> indexes;
					indexes.push_back(i);
					formNamesMap[formName] = indexes;
				}
				else {
					auto& indexes = it->second;
					indexes.push_back(i);
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
					for (auto index : v) {
						returnValues.push_back(index);
					}
				}
			}
		}
		else if (sortOption == 3 || sortOption == 4) { //sort by editorId
			std::vector<std::string> formNames;
			std::map<std::string, std::vector<std::int32_t>> formNamesMap;

			for (int i = 0; i < arr.size(); i++) {
				auto* akForm = arr[i];
				std::string formName = "";
				if (gfuncs::IsFormValid(akForm)) {
					formName = GetFormEditorId(nullptr, akForm, "");
				}

				formNames.push_back(formName);
				auto it = formNamesMap.find(formName);
				if (it == formNamesMap.end()) {
					std::vector<std::int32_t> indexes;
					indexes.push_back(i);
					formNamesMap[formName] = indexes;
				}
				else {
					auto& indexes = it->second;
					indexes.push_back(i);
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
					for (auto index : v) {
						returnValues.push_back(index);
					}
				}
			}
		}
		else { //sort by form id
			std::vector<int> formIds;
			std::map<int, std::vector<std::int32_t>> formIdsMap;

			for (int i = 0; i < arr.size(); i++) {
				auto* akForm = arr[i];
				int formID = gfuncs::IsFormValid(akForm, false, false) ? akForm->GetFormID() : 0;
				formIds.push_back(formID);
				auto it = formIdsMap.find(formID);
				if (it == formIdsMap.end()) {
					std::vector<std::int32_t> indexes;
					indexes.push_back(i);
					formIdsMap[formID] = indexes;
				}
				else {
					auto& indexes = it->second;
					indexes.push_back(i);
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
					for (auto index : v) {
						returnValues.push_back(index);
					}
				}
			}
		}
		return returnValues;
	}
	
	template <class T>
        requires PapyrusObject<T>
	std::vector<std::int32_t> GetSortIndexes(RE::StaticFunctionTag* functionTag, std::vector<T*> arr, int sortOption) {
		std::vector<std::int32_t> returnValues;
		
		if (arr.size() == 0) {
			return returnValues;
		}
		
		if constexpr (std::derived_from<T, RE::BGSBaseAlias>) {
        	returnValues = GetSortAliasArrayIndexes(functionTag, arr, sortOption);
		} else if constexpr (std::derived_from<T, RE::ActiveEffect>) {
			returnValues = GetSortActiveEffectArrayIndexes(functionTag, arr, sortOption);
		} else {
			returnValues = GetSortFormArrayIndexes(functionTag, arr, sortOption);
		}
		
		return returnValues;
	}
	
// ===============================================================================================================================================================

	
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
			
			
		}
		
		if (sortOption >= 2){
			std::sort(returnStrings.begin(), returnStrings.end());
			if (sortOption == 3) {
				std::reverse(returnStrings.begin(), returnStrings.end());
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
	
	// ---------------------------------------------------------------------------
	// Linking helpers. These let a caller apply the *same* transformation to two
	// parallel arrays (a map's keys and values) without the plugin holding state
	// between calls -- the VM can interleave other scripts' stacks in between.
	// ---------------------------------------------------------------------------

	// every index in arr whose element == value
	template <class T>
        requires PapyrusObject<T>
	std::vector<std::int32_t> GetIndexes(RE::StaticFunctionTag*, std::vector<T*> arr, T* value) {
		std::int32_t size = arr.size();
		std::vector<std::int32_t> v;
		for (std::int32_t i = 0; i < size; i++) {
			if (arr[i] == value) { v.push_back(i); }
		}
		return v;
	}
	
	// indexes that would be removed by RemoveDuplicates -- keeps the first
	// occurrence of each element, reports the rest
	template <class T>
        requires PapyrusObject<T>
	std::vector<std::int32_t> GetRemoveDuplicatesIndexes(RE::StaticFunctionTag*, std::vector<T*> arr) {
		std::int32_t size = arr.size();
		std::vector<std::int32_t> v;
		std::vector<T*> seen;
		seen.reserve(arr.size());

		for (std::int32_t i = 0; i < size; i++) {
			if (std::find(seen.begin(), seen.end(), arr[i]) == seen.end()) {
				seen.push_back(arr[i]);
			} else {
				v.push_back(i); //duplicate to remove
			}
		}
		return v;
	}
	
	// remove every listed index. Order and duplicates in `indexes` don't matter --
	// a keep-mask avoids the shifting problem entirely.
	template <class T>
        requires PapyrusObject<T>
	std::vector<T*> RemoveIndexes(RE::StaticFunctionTag*, std::vector<T*> arr, std::vector<std::int32_t> indexes) {
		std::int32_t size = arr.size();
		std::vector<T*> v;
		for (std::int32_t i = 0; i < size; i++){
			if (std::find(indexes.begin(), indexes.end(), i) == indexes.end()){
				v.push_back(arr[i]);
			}
		}
		
		return v;
	}
	
	template <class T>
        requires PapyrusObject<T>
	std::vector<T*> SortByIndexes(RE::StaticFunctionTag*, std::vector<T*> arr, std::vector<std::int32_t> indexes) {
		// a permutation for a different-length array would silently corrupt the
		// pairing, so refuse rather than guess
		if (indexes.size() != arr.size()) {
			logger::warn("SortByIndexes: index count [{}] doesn't match array size [{}]. Aborting sort.",
						 indexes.size(), arr.size());
			return arr;
		}

		std::vector<T*> v;
		v.reserve(arr.size());
		for (std::int32_t index : indexes) {
			if (index < 0 || index >= arr.size()) { 
				logger::warn("index[{}] is out of range, aborting sort.", index);
				return arr; 
			}
			v.push_back(arr[index]);
		}
		return v;
	}
	
//=====================================================================================================================================================================================
//=============================== Primative Types =====================================================================================================================================
//=====================================================================================================================================================================================

	//include primitive functions, no point T* arrays.
	template <class T>
	concept PapyrusPrimitive =
		std::same_as<T, bool> ||
		std::same_as<T, std::int32_t> ||
		std::same_as<T, float> ||
		std::same_as<T, std::string>;
		
    template <class T>
        requires PapyrusPrimitive<T>
    std::vector<T> CreateArray_p(RE::StaticFunctionTag*, int size, T fill) {
        if (size <= 0) { return std::vector<T>(0); }
		return std::vector<T>(static_cast<std::size_t>(size), fill);
		// return v;
    }
	
	template <class T>
        requires PapyrusPrimitive<T>
    std::vector<T> ResizeArray_p(RE::StaticFunctionTag*, std::vector<T> arr, int newSize, T fill) {
        if (newSize <= 0) { return std::vector<T>(0); }
        arr.resize(static_cast<std::size_t>(newSize), fill);
        return arr;
    }
	
    template <class T>
        requires PapyrusPrimitive<T>
    std::vector<T> MergeArrays_p(RE::StaticFunctionTag*, std::vector<T> arr_A, std::vector<T> arr_B) {
        arr_A.reserve(arr_A.size() + arr_B.size());
        arr_A.insert(arr_A.end(), arr_B.begin(), arr_B.end());
        return arr_A;
    }

	template <class T>
        requires PapyrusPrimitive<T>
    std::vector<T> SliceArray_p(RE::StaticFunctionTag*, std::vector<T> arr, int startIndex, int endIndex) {
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
		
		std::vector<T> v(arr.begin() + startIndex, arr.begin() + endIndex + 1);
        return v;
    }
	
    template <class T>
        requires PapyrusPrimitive<T>
    std::vector<T> PushArray_p(RE::StaticFunctionTag*, std::vector<T> arr, T item) {
        arr.push_back(item);
        return arr;
    }

	template <class T>
        requires PapyrusPrimitive<T>
    std::vector<T> InsertIntoArray_p(RE::StaticFunctionTag*, std::vector<T> arr, T item, int index) {
		int size = arr.size();
		
		if (size == 0 || index < 0 || index >= size) {
			arr.push_back(item);
			return arr;
		}
		
		arr.insert(arr.begin() + index, item);
        return arr;
    }
	
    template <class T>
        requires PapyrusPrimitive<T>
    std::vector<T> RemoveFromArray_p(RE::StaticFunctionTag*, std::vector<T> arr, T item, bool allInstances) {
        if (allInstances) {
            arr.erase(std::remove(arr.begin(), arr.end(), item), arr.end());
        } else if (auto it = std::find(arr.begin(), arr.end(), item); it != arr.end()) {
            arr.erase(it);
        }
        return arr;
    }

	template <class T>
        requires PapyrusPrimitive<T>
    std::vector<T> RemoveFromArrayAt_p(RE::StaticFunctionTag*, std::vector<T> arr, int index) {
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
        requires PapyrusPrimitive<T>
    std::vector<T> RemoveDuplicatesFromArray_p(RE::StaticFunctionTag*, std::vector<T> arr) {
		int size = arr.size();
		
		if (size == 0) {
			return arr;
		}
		
		std::vector<T> v; 
		v.push_back(arr[0]);
		for (int i = 1; i < size; i++){
			if (std::find(v.begin(), v.end(), arr[i]) == v.end()){
				v.push_back(arr[i]);
			}
		}
		
		return v;
    }
	
	template <class T>
        requires PapyrusPrimitive<T>
    int CountInArray_p(RE::StaticFunctionTag*, std::vector<T> arr, T item) {
		int count = 0;
		int size = arr.size();
		
		if (size == 0) {
			return count;
		}
		
		for (const T value : arr){
			if (value == item){
				count++;
			}
		}
        return count;
    }
	
	template <class T>
        requires PapyrusPrimitive<T>
    bool IsArrayEqualTo_P(RE::StaticFunctionTag*, std::vector<T> arrA, std::vector<T> arrB){
		if (arrA.size() != arrB.size()){
			return false;
		}
		
		std::int32_t size = arrA.size();
		
		for (std::int32_t i = 0; i < size; i++){
			if (arrA[i] != arrB[i]){
				return false;
			}
		}
		
		return true;
	}
	
	template <class T>
        requires PapyrusPrimitive<T>
	std::vector<T> SortArray_p(RE::StaticFunctionTag* functionTag, std::vector<T> arr, int sortOption) {
		if (arr.size() < 2) {
			return arr;
		}
		
		if (sortOption == 2 || sortOption == 4 || sortOption == 6) {   // descending
			// Sorts in descending order
			std::sort(arr.begin(), arr.end(), std::greater<T>());
		
		} else {
		// Sorts in ascending order
			std::sort(arr.begin(), arr.end());
		}
		return arr;
	}
	
	// Get the strings for the passed in array. mode options are:
	// 3 = IDs as hexidecimal (formId or AliasId)
	// SortOptions are: 
	// 1 = not sorted 
	// 2 = sorted ascending 
	// 3 = sorted descending
	template <class T>
        requires PapyrusPrimitive<T>
	std::vector<std::string> GetArrayStrings_p(RE::StaticFunctionTag* functionTag, std::vector<T> arr, int mode, int sortOption, 
		std::string nullName, std::string emptyName) {
			
		std::vector<std::string> returnStrings;
		
		logger::info("size[{}] mode[{}] sortOption[{}]", arr.size(), mode, sortOption);
		
		if (arr.size() == 0) {
			return returnStrings;
		}
		
		if constexpr (std::same_as<T, std::string>){
			returnStrings = arr;
		}
		else if (mode == 3){ //hexidecimal
			if constexpr (std::is_same<T, std::int32_t>::value) {
				for (const T value : arr){
					returnStrings.push_back(gfuncs::IntToHexPapyrus(static_cast<int>(value)));
				}
			}
			else if constexpr (std::is_same<T, float>::value) {
				for (const T value : arr){
					returnStrings.push_back(std::format("{:A}", (value)));
				}
			}
			else {
				for (const T value : arr){
					returnStrings.push_back(std::format("{:x}", (value)));
				}
			}
		}
		else {
			for (const T value : arr){
				returnStrings.push_back(std::to_string(value));
			}
		}
		
		if (sortOption >= 2){
			std::sort(returnStrings.begin(), returnStrings.end());
			if (sortOption == 3){
				std::reverse(returnStrings.begin(), returnStrings.end());
			}
		}
		
		return returnStrings;
	}
	
	template <class T, class T2>
    	requires PapyrusPrimitive<T> && PapyrusPrimitive<T2>
	std::vector<T> ArrayAs_p( RE::StaticFunctionTag*, std::vector<T2> arr, int removeNoneBehavior){
		if (arr.size() == 0){
			return std::vector<T>(0);
		}
		
		//already same type, shouldn't happen. Guarded in the register function
		if constexpr (std::same_as<T, T2>) {
			return arr;
		}
		
		std::vector<T> v;
		v.reserve(arr.size());
		
		if constexpr (std::same_as<T, std::string> && !std::same_as<T2, std::string>){
			for (const auto value : arr) {
				v.push_back(std::to_string(value));
			}
		}
		else if constexpr (std::same_as<T2, std::string>){
			if constexpr (std::same_as<T, bool>){
				for (const auto value : arr) { 
					std::string s = value;
					gfuncs::ConvertToLowerCase((s));
					bool b = s == "1" || s == "true";
					v.push_back((b));
				}
			}
			else if constexpr (std::same_as<T, std::int32_t>){
				for (const auto value : arr) { 
					v.push_back(std::stoi(value));
				}
			}
			else if constexpr (std::same_as<T, float>){
				for (const auto value : arr) { 
					v.push_back(std::stof(value));
				}
			}
		}
		else {
			for (const auto value : arr) {
				v.push_back(static_cast<T>(value));
			}
		}

		return v;
	}
	
	// ---------------------------------------------------------------------------
	// Linking helpers. These let a caller apply the *same* transformation to two
	// parallel arrays (a map's keys and values) without the plugin holding state
	// between calls -- the VM can interleave other scripts' stacks in between.
	// ---------------------------------------------------------------------------

	// every index in arr whose element == value
	template <class T>
		requires PapyrusPrimitive<T>
	std::vector<std::int32_t> GetIndexes_p(RE::StaticFunctionTag*, std::vector<T> arr, T value) {
		std::int32_t size = arr.size();
		std::vector<std::int32_t> v;
		for (std::int32_t i = 0; i < size; i++) {
			if (arr[i] == value) { v.push_back(i); }
		}
		return v;
	}
	
	// indexes that would be removed by RemoveDuplicates -- keeps the first
	// occurrence of each element, reports the rest
	template <class T>
		requires PapyrusPrimitive<T>
	std::vector<std::int32_t> GetRemoveDuplicatesIndexes_p(RE::StaticFunctionTag*, std::vector<T> arr) {
		std::int32_t size = arr.size();
		std::vector<std::int32_t> v;
		std::vector<T> seen;
		seen.reserve(arr.size());

		for (std::int32_t i = 0; i < size; i++) {
			if (std::find(seen.begin(), seen.end(), arr[i]) == seen.end()) {
				seen.push_back(arr[i]);
			} else {
				v.push_back(i); //duplicate to remove
			}
		}
		return v;
	}
	
	// remove every listed index. Order and duplicates in `indexes` don't matter --
	// a keep-mask avoids the shifting problem entirely.
	template <class T>
		requires PapyrusPrimitive<T>
	std::vector<T> RemoveIndexes_p(RE::StaticFunctionTag*, std::vector<T> arr, std::vector<std::int32_t> indexes) {
		std::int32_t size = arr.size();
		std::vector<T> v;
		for (std::int32_t i = 0; i < size; i++){
			if (std::find(indexes.begin(), indexes.end(), i) == indexes.end()){
				v.push_back(arr[i]);
			}
		}
		
		return v;
	}
	
	template <class T>
		requires PapyrusPrimitive<T>
	std::vector<std::int32_t> GetSortIndexes_p(RE::StaticFunctionTag*, std::vector<T> arr,  int sortOption) {
		std::vector<std::int32_t> indexes(arr.size());
		std::iota(indexes.begin(), indexes.end(), 0);

		const bool descending = (sortOption == 2 || sortOption == 4 || sortOption == 6);

		std::stable_sort(indexes.begin(), indexes.end(),
			[&](std::int32_t a, std::int32_t b) {
				return descending ? arr[a] > arr[b] : arr[a] < arr[b];
			});

		return indexes;
	}
	
	template <class T>
		requires PapyrusPrimitive<T>
	std::vector<T> SortByIndexes_p(RE::StaticFunctionTag*, std::vector<T> arr, std::vector<std::int32_t> indexes) {
		// a permutation for a different-length array would silently corrupt the
		// pairing, so refuse rather than guess
		if (indexes.size() != arr.size()) {
			logger::warn("SortByIndexes: index count [{}] doesn't match array size [{}]. Aborting sort.",
						 indexes.size(), arr.size());
			return arr;
		}

		std::vector<T> v;
		v.reserve(arr.size());
		for (std::int32_t index : indexes) {
			if (index < 0 || index >= arr.size()) { 
				logger::warn("index[{}] is out of range, aborting sort.", index);
				return arr; 
			}
			v.push_back(arr[index]);
		}
		return v;
	}
	
	
	int RegisterDbSkseArrayScripts();
	
    // bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}
