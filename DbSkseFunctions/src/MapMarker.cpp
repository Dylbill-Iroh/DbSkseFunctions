#include "MapMarker.h"
#include "GeneralFunctions.h"
#include "RE/E/ExtraTeleport.h"
#include "RE/L/LocalMapCamera.h"
#include "RE/N/NiPoint3.h"
#include "RE/T/TESFullName.h"
#include "RE/T/TypeTraits.h"
#include "UIGfx.h"
#include "RE/B/BSCoreTypes.h"
#include "RE/B/BSPointerHandle.h"
#include "RE/H/HUDNotifications.h"
#include "RE/L/LocalMapMenu.h"
#include "RE/M/MapMenuMarker.h"
#include "RE/T/TESObjectREFR.h"
#include "RE/T/TESWorldSpace.h"
#include "REL/Module.h"
#include "SharedVariables.h"
#include "Serialization.h"
#include "magic_enum/magic_enum.hpp"
#include "magic_enum/magic_enum_all.hpp"
#include <cstddef>
#include <format>
#include <json/json.h>

bool IsMapMarker(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker) {
    logger::trace("function called");

    if (!gfuncs::IsFormValid(mapMarker)) {
        logger::warn("IsMapMarker: mapMarker doesn't exist");
        return false;
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        logger::debug("map marker list not found.");
        return false;
    }

    if (!mapMarkerData->mapData) {
        logger::debug("mapData not found.");
        return false;
    }

    return true;
}

bool SetMapMarkerVisible(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker, bool visible) {
    logger::trace("function called");

    if (!gfuncs::IsFormValid(mapMarker)) {
        logger::warn("mapMarker isn't a valid form");
        return false;
    }

    auto* marker = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();
    if (!marker) {
        logger::warn("mapMarker ref[{}] isn't a mapMarker", gfuncs::GetFormName(mapMarker));
        return false;
    }

    if (!marker->mapData) {
        logger::warn("marker->mapData for ref[{}] doesn't exist", gfuncs::GetFormName(mapMarker));
        return false;
    }

    if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) != visible) {
        marker->mapData->SetVisible(visible);
        return (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == visible);
    }

    return false;
}

bool IsRefVisibleOnLocalMap(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
	if (!gfuncs::IsFormValid(ref)) {
		logger::warn("ref doesn't exist");
		return false;
	}
	
	auto* ui = RE::UI::GetSingleton();
	if (!ui) {
		logger::warn("ui doesn't exist");
		return false;
	}
	
	RE::GPtr<RE::MapMenu> menu = ui->GetMenu<RE::MapMenu>();
	if (!menu) {
		logger::warn("MapMenu not found");
		return false;
	}
	
	int size = 0;
	RE::BSTArray<RE::MapMenuMarker>* markers = nullptr;
	
	if (REL::Module::IsVR()) {
		auto* data = menu->GetVRRuntimeData();
		if (data){
			RE::LocalMapMenu* localMapMenu = &data->localMapMenu; 
			if (localMapMenu){ 
				auto* localData = &localMapMenu->GetRuntimeData();
				if (localData){
					markers = &localMapMenu->mapMarkers;  
					if (markers){
						size = markers->size();
					}
				}
			} 
		}
	} 
	else { 
		auto* data = menu->GetRuntimeData();
		if (data){
			RE::LocalMapMenu* localMapMenu = &data->localMapMenu; 
			if (localMapMenu){ 
				auto* localData = &localMapMenu->GetRuntimeData();
				if (localData){
					markers = &localMapMenu->mapMarkers;  
					if (markers){
						size = markers->size();
					}
				}
			} 
		}
	}

	logger::debug("markers size[{}]", size);
	
	if (size == 0 || !markers) {
		return false;   // local markers not found
	}
	
	//int32_t
	for (std::size_t i = 0; i < markers->size(); i++) {
		RE::RefHandle handle = (*markers)[(int32_t)i].ref; 
		
		RE::TESObjectREFR* akRef = gfuncs::GetRefFromHandle(handle);
		if (gfuncs::IsFormValid(akRef)){
			if (akRef == ref){
				return true;
			}
		}
	}
	
	return false;
} 

std::vector<RE::TESObjectREFR*> GetRefsVisibleOnTheLocalMap(RE::StaticFunctionTag*) {
	std::vector<RE::TESObjectREFR*> refs; 
	
	auto* ui = RE::UI::GetSingleton();
	if (!ui) {
		logger::warn("ui doesn't exist");
		return refs;
	}
	
	RE::GPtr<RE::MapMenu> menu = ui->GetMenu<RE::MapMenu>();
	if (!menu) {
		logger::warn("MapMenu not found");
		return refs;
	}
	
	int size = 0;
	RE::BSTArray<RE::MapMenuMarker>* markers = nullptr;
	
	if (REL::Module::IsVR()) {
		auto* data = menu->GetVRRuntimeData();
		if (data){
			RE::LocalMapMenu* localMapMenu = &data->localMapMenu; 
			if (localMapMenu){ 
				auto* localData = &localMapMenu->GetRuntimeData();
				if (localData){
					markers = &localMapMenu->mapMarkers;  
					if (markers){
						size = markers->size();
					}
				}
			} 
		}
	} 
	else { 
		auto* data = menu->GetRuntimeData();
		if (data){
			RE::LocalMapMenu* localMapMenu = &data->localMapMenu; 
			if (localMapMenu){ 
				auto* localData = &localMapMenu->GetRuntimeData();
				if (localData){
					markers = &localMapMenu->mapMarkers;  
					if (markers){
						size = markers->size();
					}
				}
			} 
		}
	}

	logger::debug("markers size[{}]", size);
	
	if (size == 0 || !markers) {
		return refs;   // local markers not found
	}
	
	for (std::size_t i = 0; i < markers->size(); i++) {
		RE::RefHandle handle = (*markers)[(int32_t)i].ref; 
		
		RE::TESObjectREFR* akRef = gfuncs::GetRefFromHandle(handle);
		if (gfuncs::IsFormValid(akRef)){
			refs.push_back(akRef);
		}
	}
	
	return refs;
}

bool SetCanFastTravelToMarker(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker, bool canTravelTo) {
    logger::trace("function called");

    if (!gfuncs::IsFormValid(mapMarker)) {
        logger::warn("mapMarker isn't a valid form");
        return false;
    }

    auto* marker = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();
    if (!marker) {
        logger::warn("mapMarker ref[{}] isn't a mapMarker", gfuncs::GetFormName(mapMarker));
        return false;
    }

    if (!marker->mapData) {
        logger::warn("marker->mapData for ref[{}] doesn't exist", gfuncs::GetFormName(mapMarker));
        return false;
    }

    if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) != canTravelTo) {
        if (canTravelTo) {
            marker->mapData->flags.set(RE::MapMarkerData::Flag::kCanTravelTo);
        }
        else {
            marker->mapData->flags.reset(RE::MapMarkerData::Flag::kCanTravelTo);
        }
        return (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == canTravelTo);
    }
    else {
        return true; 
    }
}

//return -1 for mapMarker not valid, 0 for disabled, 1 for invisible, 2 for visible but can't fast travel to, 3 for visible and can fast travel to.
int GetMapMarkerVisibilityStatus(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker){
	if (!gfuncs::IsFormValid(mapMarker)){
		return -1;
	}
	
	auto* marker = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();
    if (!marker) {
        logger::warn("mapMarker ref[{}] isn't a mapMarker", gfuncs::GetFormName(mapMarker));
        return -1;
    }

    if (!marker->mapData) {
        logger::warn("marker->mapData for ref[{}] doesn't exist", gfuncs::GetFormName(mapMarker));
        return -1;
    }

	if (mapMarker->IsDisabled()){
		return 0;
	}
	
	bool visible = marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible);
	bool canTravelTo = marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo);
	
	if (visible && canTravelTo){
		return 3;
	}
	else if (visible && !canTravelTo){
		return 2;
	}
	else {
		return 1;
	}
}

std::vector<int> GetMapMarkerVisibilityStatuses(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> mapMarkers){ 
	std::vector<int> v; 
	for (auto* marker : mapMarkers){
		v.push_back(GetMapMarkerVisibilityStatus(nullptr, marker));
	}
	return v;
}

// Get all map markers valid for the current worldspace or interior cell grid
RE::BSTArray<RE::ObjectRefHandle>* GetPlayerMapMarkers() {
	auto* player = RE::PlayerCharacter::GetSingleton();
	if (!player){
		return nullptr; 
	}
	
	if (REL::Module::IsVR()){
		auto* runtimeData = player->GetVRPlayerRuntimeData();
		return &runtimeData->currentMapMarkers;
	}
	else {
		auto& runtimeData = player->GetPlayerRuntimeData();
		return &runtimeData.currentMapMarkers;
	}
}

std::vector<RE::TESObjectREFR*> GetAllMapMarkerRefs(RE::StaticFunctionTag*, int visibleFilter, int canTravelToFilter) {
    std::vector<RE::TESObjectREFR*> allMapMarkers;

    if (visibleFilter == 1 && canTravelToFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 1 && canTravelToFilter == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 0 && canTravelToFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 0 && canTravelToFilter == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (canTravelToFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                allMapMarkers.push_back(ref);
                            }
                        }
                    }
                }
            }
        }
    }
    else if (canTravelToFilter == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                allMapMarkers.push_back(ref);
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                allMapMarkers.push_back(ref);
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                allMapMarkers.push_back(ref);
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            allMapMarkers.push_back(ref);
                        }
                    }
                }
            }
        }
    }
    return allMapMarkers;
}

// get all map markers valid for the current world space or interior cell grid
std::vector<RE::TESObjectREFR*> GetCurrentMapMarkerRefs(RE::StaticFunctionTag*, int visibleFilter, int canTravelToFilter) {
    auto* playerMapMarkers = GetPlayerMapMarkers();

    std::vector<RE::TESObjectREFR*> allMapMarkers;

    if (!playerMapMarkers) {
        return allMapMarkers;
    }

    if (playerMapMarkers->size() == 0) {
        return allMapMarkers;
    }

    int ic = 0;
    if (visibleFilter == 1 && canTravelToFilter == 1) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                    if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                        allMapMarkers.push_back(ref);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ic++;
            if (ic >= playerMapMarkers->size()) {
                break;
            }
        }
    }
    else if (visibleFilter == 1 && canTravelToFilter == 0) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                    if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                        allMapMarkers.push_back(ref);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ic++;
            if (ic >= playerMapMarkers->size()) {
                break;
            }
        }
    }
    else if (visibleFilter == 0 && canTravelToFilter == 1) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                    if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                        allMapMarkers.push_back(ref);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ic++;
            if (ic >= playerMapMarkers->size()) {
                break;
            }
        }
    }
    else if (visibleFilter == 0 && canTravelToFilter == 0) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                    if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                        allMapMarkers.push_back(ref);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ic++;
            if (ic >= playerMapMarkers->size()) {
                break;
            }
        }
    }
    else if (canTravelToFilter == 1) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
            ic++;
            if (ic >= playerMapMarkers->size()) {
                break;
            }
        }
    }
    else if (canTravelToFilter == 0) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
            ic++;
            if (ic >= playerMapMarkers->size()) {
                break;
            }
        }
    }
    else if (visibleFilter == 1) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
            ic++;
            if (ic >= playerMapMarkers->size()) {
                break;
            }
        }
    }
    else if (visibleFilter == 0) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
            ic++;
            if (ic >= playerMapMarkers->size()) {
                break;
            }
        }
    }
    else {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                allMapMarkers.push_back(ref);
                            }
                        }
                    }
                }
            }
            ic++;
            if (ic >= playerMapMarkers->size()) {
                break;
            }
        }
    }
    return allMapMarkers;
}

RE::TESForm* GetRefWorldSpaceOrCell(RE::TESObjectREFR* ref) {
    RE::TESForm* form = ref->GetWorldspace();
    if (!gfuncs::IsFormValid(form)) {
        form = ref->GetParentCell();
    }
    if (!gfuncs::IsFormValid(form)) {
        return nullptr;
    }
    return form;
}

RE::TESForm* GetCellOrWorldSpaceOriginForRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("ref doesn't exist");
        return nullptr;
    }

    auto* originData = ref->extraList.GetByType<RE::ExtraStartingWorldOrCell>();
    if (!originData) {
        logger::trace("originData for ref({}) not found, getting current worldspace or cell.", gfuncs::GetFormDataString(ref));

        //origin data on a reference will only exist if it has been moved from its original worldspace or interior cell
        return GetRefWorldSpaceOrCell(ref);
    }

    if (!gfuncs::IsFormValid(originData->startingWorldOrCell)) {
        logger::trace("originData->startingWorldOrCell for ref({}) doesn't exist, getting current worldspace or cell.", gfuncs::GetFormDataString(ref));
        return GetRefWorldSpaceOrCell(ref);
    }

    return originData->startingWorldOrCell;
}

std::vector<RE::TESForm*> GetCellOrWorldSpaceOriginForRefs(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs){
	std::vector<RE::TESForm*> v; 
	for (auto* ref : refs){
		v.push_back(GetCellOrWorldSpaceOriginForRef(nullptr, ref));
	}
	
	return v;
}

bool SetCellOrWorldSpaceOriginForRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, RE::TESForm* cellOrWorldSpace) {

    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("ref doesn't exist");
        return false;
    }

    if (!gfuncs::IsFormValid(cellOrWorldSpace)) {
        logger::warn("cellOrWorldSpace doesn't exist");
        return false;
    }

    auto* originData = ref->extraList.GetByType<RE::ExtraStartingWorldOrCell>();
    if (!originData) {
        logger::warn("originData for ref({}) not found", gfuncs::GetFormDataString(ref));
        return false;
    }

    originData->startingWorldOrCell = cellOrWorldSpace;

	auto* player = RE::PlayerCharacter::GetSingleton();
    if (player) {
        if (IsMapMarker(nullptr, ref)) {
            RE::TESWorldSpace* newOriginWorld = static_cast<RE::TESWorldSpace*>(cellOrWorldSpace);

            if (gfuncs::IsFormValid(newOriginWorld)) {
                RE::TESWorldSpace* currentWorld = player->GetWorldspace();
                if (gfuncs::IsFormValid(currentWorld)) {
                    if (newOriginWorld == currentWorld) {
                        auto* mapMarkers = GetPlayerMapMarkers();
                        if (mapMarkers) {
                            //logger::critical("currentMapMarkers size is {}", mapMarkers->size());

                            auto refHandle = ref->GetHandle();
                            if (!gfuncs::IsObjectInBSTArray(mapMarkers, refHandle)) {
                                //logger::critical("adding ref to currentMapMarkers");
                                mapMarkers->push_back(refHandle);
                            }
                            else {
                                //logger::critical("ref already in currentMapMarkers");
                            }
                        }
                        else {
                            //logger::critical("currentMapMarkers not found");
                        }
                    }
                }
            }
        }
    }
    logger::trace("ref({}) origin set to \n({})", gfuncs::GetFormDataString(ref),
        gfuncs::GetFormDataString(cellOrWorldSpace));

    return true;
}

bool SetMapMarkerName(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker, std::string name) {
    logger::trace("Renaming map marker");
    if (!gfuncs::IsFormValid(mapMarker)) {
        logger::warn("mapMarker doesn't exist");
        return false;
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        logger::warn("Warning, map marker list not found.");
        return false;
    }

    if (!mapMarkerData->mapData) {
        logger::warn("Warning, mapData not found.");
        return false;
    }

    if (mapMarkerData->mapData->locationName.fullName == NULL) {
        logger::warn("Warning, LocationName not found.");
        return false;
    }

    const char* cName = name.data();
    mapMarkerData->mapData->locationName.fullName = cName;
    logger::trace("New map marker name = {}", mapMarkerData->mapData->locationName.GetFullName());

    return true;
}

std::string GetMapMarkerName(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker) {
    logger::trace("Getting Marker Name");
    if (!gfuncs::IsFormValid(mapMarker)) {
        logger::warn("GetMapMarkerName: mapMarker doesn't exist");
        return "";
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        logger::warn("GetMapMarkerName Warning, map marker list not found.");
        return "";
    }

    if (!mapMarkerData->mapData) {
        logger::warn("GetMapMarkerName Warning, mapData not found.");
        return "";
    }

    if (mapMarkerData->mapData->locationName.fullName == NULL) {
        logger::warn("GetMapMarkerName Warning, LocationName not found.");
        return "";
    }

    return std::string(mapMarkerData->mapData->locationName.GetFullName());
}

bool SetMapMarkerIconType(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker, int iconType) {
    if (!gfuncs::IsFormValid(mapMarker)) {
        logger::warn("SetMapMarkerIconType: mapMarker doesn't exist");
        return false;
    }

    logger::trace("Setting Map Marker Type to {}", iconType);

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        logger::warn("SetMapMarkerIconType Warning, map marker extra data list not found.");
        return false;
    }

    if (!mapMarkerData->mapData) {
        logger::warn("SetMapMarkerIconType Warning, mapData not found.");
        return false;
    }

    mapMarkerData->mapData->type = static_cast<RE::MARKER_TYPE>(iconType);

    return true;
}

int GetMapMarkerIconType(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker) {
    logger::trace("Getting Map Marker Type");
    if (!gfuncs::IsFormValid(mapMarker)) {
        logger::warn("GetMapMarkerIconType: mapMarker doesn't exist");
        return -1;
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        logger::warn("GetMapMarkerIconType Warning, map marker list not found.");
        return -1;
    }

    if (!mapMarkerData->mapData) {
        logger::warn("GetMapMarkerIconType Warning, mapData not found.");
        return -1;
    }

    return static_cast<int>(mapMarkerData->mapData->type.get());
} 

// ---------------------------------------------------------------------------------------------------------------------
// MapMarkerFramework support
// MMF (by Parapets, MIT) sends its interface to plugins listening for "MapMarkerFramework" at kDataLoaded.
// The layout below matches its include/MMF/Stubs.h, interface version 1.
// Needs JsonCpp (the same parser MMF uses, so both read the files the same way). 

namespace mmf {
	struct MarkerInfo {
		const char*  SourcePath;
		const char*  ExportName;
		const char*  ExportNameUndiscovered;
		std::int32_t VanillaMarker;
		float        IconScale;
		bool         HideFromHUD;
	};
	static_assert(sizeof(MarkerInfo) == 0x28);

	struct Interface {
		std::uint32_t interfaceVersion;
		std::uint16_t (*GetVanillaMarkerType)(RE::TESObjectREFR*);   // MMF leaves this null -- never call it
		const char*   (*GetCustomMarkerName)(RE::TESObjectREFR*);
		void          (*GetMapMarkerInfo)(RE::TESObjectREFR*, MarkerInfo*);
		void          (*GetLocalMarkerInfo)(RE::BGSLocation*, MarkerInfo*);
	};
	static_assert(sizeof(Interface) == 0x28);

	constexpr std::uint32_t kInterfaceMessage = 0;      // MMF::kMapMarkerInterface
	constexpr int kFirstPossibleCustomIcon = 67;         // MMF asserts the HUD has at least 67 built-in icons

	std::atomic<const Interface*> intfc{ nullptr };

	void OnMessage(SKSE::MessagingInterface::Message* a_msg) {
		if (!a_msg || a_msg->type != kInterfaceMessage || !a_msg->data) {
			return;
		}
		auto* received = static_cast<const Interface*>(a_msg->data);
		if (received->interfaceVersion < 1) {
			return;
		}
		intfc.store(received);
		logger::info("MapMarkerFramework interface v{} received", received->interfaceVersion);
	}

	// The icon name MMF's config assigns to this ref, or "".
	std::string GetConfigIconName(RE::TESObjectREFR* ref) {
		auto* i = intfc.load();
		if (!i || !i->GetCustomMarkerName || !ref) {
			return "";
		}
		return gfuncs::SafeString(i->GetCustomMarkerName(ref));
	}

	// ---------------------------------------------------------------------------------------------------------
	// Icon catalog
	//
	// MMF reads Data\MapMarkers\<plugin name>.json for every loaded plugin, in load order. Each entry in a file's
	// "iconDefinitions" that has a "name" and a "source" object becomes a custom icon, numbered 0, 1, 2... across
	// all the files. When the HUD loads, MMF adds those icons after the HUD's built-in ones, so
	// icon type = base + number, where base is the number of built-in icons (67+, more with modded HUDs).
	//
	// So: read the same files in the same order to get the names and numbers, then find base.
	// ---------------------------------------------------------------------------------------------------------

	struct IconDef {
		std::string name;
		std::string configFile;   // e.g. "MapMarkers\\Skyrim.json"
	};

	struct IconTable {
		std::unordered_map<std::string, int> typeByLowerName;
		std::unordered_map<int, std::string> nameByType;
		std::vector<std::string>             names;      // in icon type order, no duplicates
		int                                  base = 0;
	};

	std::mutex           tableMutex;
	IconTable            table;
	bool                 tableBuilt = false;
	std::vector<IconDef> iconDefs;
	bool                 iconDefsLoaded = false;

	// Reads a file through the game's file system, so loose files and BSA contents both work.
	std::string ReadGameFile(const std::string& path) {
		RE::BSResourceNiBinaryStream stream(path);
		if (!stream.good() || !stream.stream) {
			return "";
		}
		std::uint32_t size = stream.stream->totalSize;
		if (size == 0 || size > 16 * 1024 * 1024) {
			return "";
		}
		std::string text(size, '\0');
		if (!stream.read(text.data(), size)) {
			return "";
		}
		return text;
	}

	// The custom icons MMF imports, in MMF's numbering order.
	std::vector<IconDef> LoadIconDefs() {
		std::vector<IconDef> defs;

		auto* dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler) {
			return defs;
		}

		for (auto* file : dataHandler->files) {
			if (!file) {
				continue;
			}
			std::string pluginName{ file->GetFilename() };
			if (pluginName.empty()) {
				continue;
			}
			// "Skyrim.esm" -> "MapMarkers\Skyrim.json" (same as MMF's path::replace_extension)
			auto dot = pluginName.rfind('.');
			std::string configFile = "MapMarkers\\" + (dot == std::string::npos ? pluginName : pluginName.substr(0, dot)) + ".json";

			std::string text = ReadGameFile(configFile);
			if (text.empty()) {
				continue;
			}

			// Default JsonCpp settings, like MMF: comments and trailing commas are allowed.
			Json::CharReaderBuilder builder;
			std::unique_ptr<Json::CharReader> reader{ builder.newCharReader() };
			Json::Value root;
			std::string errors;
			if (!reader->parse(text.data(), text.data() + text.size(), &root, &errors)) {
				logger::warn("MMF icon catalog: couldn't parse {}: {}", configFile, errors);
				continue;
			}
			if (!root.isObject()) {
				continue;
			}

			const Json::Value& iconDefinitions = root["iconDefinitions"];
			if (!iconDefinitions.isArray()) {
				continue;
			}

			for (const auto& def : iconDefinitions) {
				if (!def.isObject()) {
					continue;
				}
				// MMF only counts entries with a name and a source object. Entries with just an "index"
				// change a vanilla icon and don't add a new one.
				const Json::Value& name = def["name"];
				const Json::Value& source = def["source"];
				if (!name.isString() || !source.isObject()) {
					continue;
				}
				std::string nameStr = name.asString();
				if (nameStr.empty()) {
					continue;
				}
				defs.push_back({ nameStr, configFile });
			}
		}

		logger::info("MMF icon catalog: {} custom icons defined in Data\\MapMarkers", defs.size());
		return defs;
	}

	// Base from map marker refs that an MMF config assigned an icon to: their type minus their icon's number.
	// Uses the most common answer, in case a script has changed some markers since.
	int GetBaseFromRefs(const std::unordered_map<std::string, int>& indexByName) {
		std::unordered_map<int, int> votes;   // base -> count
		{
			const auto& [allForms, lock] = RE::TESForm::GetAllForms();
			RE::BSReadLockGuard readLock(lock);

			for (auto& [id, form] : *allForms) {
				if (!gfuncs::IsFormValid(form, false, false)) {
					continue;
				}
				auto* ref = form->AsReference();
				if (!ref) {
					continue;
				}
				auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
				if (!marker || !marker->mapData) {
					continue;
				}
				std::string name = GetConfigIconName(ref);
				if (name.empty()) {
					continue;
				}
				auto it = indexByName.find(name);
				if (it == indexByName.end()) {
					continue;
				}
				int type = static_cast<int>(marker->mapData->type.underlying());
				int base = type - it->second;
				if (base >= kFirstPossibleCustomIcon) {   // otherwise MMF hasn't set this marker's icon yet
					votes[base]++;
				}
			}
		}

		int best = 0;
		int bestCount = 0;
		for (auto& [base, count] : votes) {
			if (count > bestCount) {
				best = base;
				bestCount = count;
			}
		}
		return best;
	}

	// Base from the HUD's compass marker timeline, which MMF added the icon frames to.
	// Built-in icons + custom icons = frames from "LocationMarkers" to the last used frame before "UndiscoveredMarkers".
	int GetBaseFromHud(int customIconCount) {
		auto* ui = RE::UI::GetSingleton();
		if (!ui) {
			return 0;
		}
		auto hud = ui->GetMenu<RE::HUDMenu>();
		if (!hud || !hud->uiMovie) {
			return 0;
		}
		auto* movieDef = hud->uiMovie->GetMovieDef();
		if (!movieDef) {
			return 0;
		}
		auto* resource = movieDef->GetResource("Compass Marker");
		if (!resource || resource->GetResourceType() != RE::GFxResource::ResourceType::kSpriteDef) {
			return 0;
		}
		auto* sprite = static_cast<RE::GFxSpriteDef*>(resource);

		std::int32_t first = 0;
		std::int32_t undiscovered = 0;
		if (!sprite->GetLabeledFrame("LocationMarkers", first, false) ||
			!sprite->GetLabeledFrame("UndiscoveredMarkers", undiscovered, false) ||
			undiscovered <= first) {
			return 0;
		}

		// Modded HUDs can leave empty frames before "UndiscoveredMarkers"; skip back over them like MMF does.
		std::int32_t last = undiscovered - 1;
		while (last > first + 66 && last < sprite->frameCount && !sprite->frames[last].data) {
			last--;
		}

		int total = last - first + 1;
		int base = total - customIconCount;
		return base >= kFirstPossibleCustomIcon ? base : 0;
	}

	bool BuildIconTable(IconTable& out) {
		if (!iconDefsLoaded) {
			iconDefs = LoadIconDefs();
			iconDefsLoaded = true;
		}
		if (iconDefs.empty()) {
			return true;   // nothing to find
		}

		// MMF keeps the last definition's number for a duplicate name.
		std::unordered_map<std::string, int> indexByName;
		for (int i = 0; i < static_cast<int>(iconDefs.size()); i++) {
			indexByName[iconDefs[i].name] = i;
		}

		int refBase = GetBaseFromRefs(indexByName);
		int hudBase = GetBaseFromHud(static_cast<int>(iconDefs.size()));
		if (refBase && hudBase && refBase != hudBase) {
			logger::warn("MMF icon catalog: base icon type from map markers ({}) and from the HUD ({}) differ, using {}",
				refBase, hudBase, refBase);
		}
		int base = refBase ? refBase : hudBase;
		if (!base) {
			return false;   // MMF hasn't added its icons yet (it does when the HUD loads). Try again next time.
		}

		out.base = base;
		for (int i = 0; i < static_cast<int>(iconDefs.size()); i++) {
			out.nameByType[base + i] = iconDefs[i].name;
		}
		for (auto& [name, index] : indexByName) {
			std::string lower = name;
			gfuncs::ConvertToLowerCase(lower);
			out.typeByLowerName[lower] = base + index;   // if two names differ only by case, the later one wins
		}
		for (int i = 0; i < static_cast<int>(iconDefs.size()); i++) {
			std::string lower = iconDefs[i].name;
			gfuncs::ConvertToLowerCase(lower);
			if (out.typeByLowerName[lower] == base + i) {
				out.names.push_back(iconDefs[i].name);
			}
		}

		logger::info("MMF icon catalog: {} icons, types {} to {} (base from {})", out.names.size(), base,
			base + static_cast<int>(iconDefs.size()) - 1, refBase ? "map markers" : "HUD");
		return true;
	}

	// A copy of the icon table, built on first use once MMF has added its icons.
	IconTable GetIconTable() {
		IconTable result;
		if (!intfc.load()) {
			return result;
		}

		tableMutex.lock();
		if (!tableBuilt) {
			IconTable built;
			if (BuildIconTable(built)) {
				table = std::move(built);
				tableBuilt = true;
			}
		}
		result = table;
		tableMutex.unlock();
		return result;
	}

	int GetIconType(std::string iconName) {
		gfuncs::ConvertToLowerCase(iconName);
		auto icons = GetIconTable();
		auto it = icons.typeByLowerName.find(iconName);
		return it != icons.typeByLowerName.end() ? it->second : -1;
	}

	std::string GetIconName(int iconType) {
		if (iconType < kFirstPossibleCustomIcon) {
			return "";
		}
		auto icons = GetIconTable();
		auto it = icons.nameByType.find(iconType);
		return it != icons.nameByType.end() ? it->second : "";
	}
}

bool IsMapMarkerFrameworkInstalled(RE::StaticFunctionTag*) {
	return mmf::intfc.load() != nullptr;
}

std::string GetMapMarkerFrameworkIconTypeName(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker) {
	int type = GetMapMarkerIconType(nullptr, mapMarker);
	return type < 0 ? "" : mmf::GetIconName(type);
}

std::vector<std::string> GetMapMarkerFrameworkIconTypeNames(RE::StaticFunctionTag*) {
	return mmf::GetIconTable().names;
}

int GetMapMarkerFrameworkIconType(RE::StaticFunctionTag*, std::string iconName) {
	return iconName.empty() ? -1 : mmf::GetIconType(iconName);
}

bool SetMapMarkerFrameworkIconType(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker, std::string iconName) {
	if (!mmf::intfc.load()) {
		logger::warn("MapMarkerFramework isn't installed");
		return false;
	}
	int type = mmf::GetIconType(iconName);
	if (type < 0) {
		logger::warn("MMF icon [{}] not found", iconName);
		return false;
	}
	return SetMapMarkerIconType(nullptr, mapMarker, type);
}

//get all map markers added by MapMarkerFramework 
 std::vector<RE::TESObjectREFR*> GetAllMapMarkerFrameworkRefs(RE::StaticFunctionTag*, int visibleFilter, int canTravelToFilter) {
	std::vector<RE::TESObjectREFR*> allMapMarkers;

	auto icons = mmf::GetIconTable();   // build / copy once, outside the forms lock
	if (icons.nameByType.empty()) {
		return allMapMarkers;
	}
	auto isMmfIcon = [&](RE::MapMarkerData* data) {
		return icons.nameByType.contains(static_cast<int>(data->type.get()));
	};

    if (visibleFilter == 1 && canTravelToFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
									//int type = static_cast<int>(marker->mapData->type.get());
									if (isMmfIcon(marker->mapData)){
                                    	allMapMarkers.push_back(ref);
									}
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 1 && canTravelToFilter == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                    //int type = static_cast<int>(marker->mapData->type.get());
									if (isMmfIcon(marker->mapData)){
                                    	allMapMarkers.push_back(ref);
									}
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 0 && canTravelToFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                    //int type = static_cast<int>(marker->mapData->type.get());
									if (isMmfIcon(marker->mapData)){
                                    	allMapMarkers.push_back(ref);
									}
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 0 && canTravelToFilter == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                    //int type = static_cast<int>(marker->mapData->type.get());
									if (isMmfIcon(marker->mapData)){
                                    	allMapMarkers.push_back(ref);
									}
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (canTravelToFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                //int type = static_cast<int>(marker->mapData->type.get());
								if (isMmfIcon(marker->mapData)){
									allMapMarkers.push_back(ref);
								}
                            }
                        }
                    }
                }
            }
        }
    }
    else if (canTravelToFilter == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                //int type = static_cast<int>(marker->mapData->type.get());
								if (isMmfIcon(marker->mapData)){
									allMapMarkers.push_back(ref);
								}
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                //int type = static_cast<int>(marker->mapData->type.get());
								if (isMmfIcon(marker->mapData)){
									allMapMarkers.push_back(ref);
								}
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                //int type = static_cast<int>(marker->mapData->type.get());
								if (isMmfIcon(marker->mapData)){
									allMapMarkers.push_back(ref);
								}
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false, false)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            //int type = static_cast<int>(marker->mapData->type.get());
							if (isMmfIcon(marker->mapData)){
								allMapMarkers.push_back(ref);
							}
                        }
                    }
                }
            }
        }
    }
    return allMapMarkers;
}

std::string GetMapMarkerIconTypeString(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker) {
    logger::trace("Getting Map Marker Type");
    if (!gfuncs::IsFormValid(mapMarker)) {
        logger::warn("GetMapMarkerIconType: mapMarker doesn't exist");
        return "";
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        logger::warn("GetMapMarkerIconType Warning, map marker list not found.");
        return "";
    }

    if (!mapMarkerData->mapData) {
        logger::warn("GetMapMarkerIconType Warning, mapData not found.");
        return "";
    }

	auto type = mapMarkerData->mapData->type.get();
	std::string s = static_cast<std::string>(magic_enum::enum_name(type));
	return !s.empty() ? s : mmf::GetIconName(static_cast<int>(type));   // MMF custom icon
} 

struct MapMarkerSortEntry {
	RE::TESObjectREFR* ref = nullptr;
	bool               valid = false;
	std::string        name = "";
	int                iconType = -1;
	std::string        origin = "";
};

//reads the sort values directly so non-markers don't spam GetMapMarkerName / GetMapMarkerIconType warnings
MapMarkerSortEntry GetMapMarkerSortEntry(RE::TESObjectREFR* ref, bool needOrigin) {
	MapMarkerSortEntry entry;
	entry.ref = ref;
	entry.valid = gfuncs::IsFormValid(ref);
	if (!entry.valid) {
		return entry;
	}

	auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
	
	if (marker) {
		if (marker->mapData){
			const char* name = marker->mapData->locationName.GetFullName();
			entry.name = name ? name : "";
			entry.iconType = static_cast<int>(marker->mapData->type.get());
		}
	}

	if (needOrigin) {
		if (auto* origin = GetCellOrWorldSpaceOriginForRef(nullptr, ref)) {
			const char* originName = origin->GetName();
			entry.origin = originName ? originName : "";
		}
	}
	return entry;
}

//For all sort options: -1 = unsorted, 0 = ascending, 1 = descending.
//iconTypeSort and OriginSort group the markers; nameSort orders them within each group.
//Only one of iconTypeSort / OriginSort can be used. If both are set, iconTypeSort wins.
//Example: nameSort 0, iconTypeSort 1 ->
//  iconType 66: MarkerA, MarkerB
//  iconType 65: MarkerA, MarkerB ...
//Names compare case-insensitively. Markers that keep the same position keep their input order.
//None / invalid refs go to the end.
std::vector<RE::TESObjectREFR*> SortMapMarkers(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> mapMarkers, int nameSort, int iconTypeSort, int OriginSort) {
		
	auto normalize = [](int sort) { return (sort == 0 || sort == 1) ? sort : -1; };
	nameSort = normalize(nameSort);
	iconTypeSort = normalize(iconTypeSort);
	OriginSort = normalize(OriginSort);

	if (iconTypeSort != -1 && OriginSort != -1) {
		logger::warn("can't sort by both icon type and origin, using icon type only");
		OriginSort = -1;
	}

	if (nameSort == -1 && iconTypeSort == -1 && OriginSort == -1) {
		return mapMarkers;
	}

	std::vector<MapMarkerSortEntry> entries;
	entries.reserve(mapMarkers.size());
	for (auto* ref : mapMarkers) {
		entries.push_back(GetMapMarkerSortEntry(ref, OriginSort != -1));
	}

	auto compareStrings = [](const std::string& a, const std::string& b) {
		return _stricmp(a.c_str(), b.c_str());
	};

	std::stable_sort(entries.begin(), entries.end(), [&](const MapMarkerSortEntry& a, const MapMarkerSortEntry& b) {
		//invalid refs last
		if (a.valid != b.valid) {
			return a.valid;
		}
		if (!a.valid) {
			return false;
		}

		//group first: icon type or origin
		if (iconTypeSort != -1 && a.iconType != b.iconType) {
			return iconTypeSort == 0 ? a.iconType < b.iconType : a.iconType > b.iconType;
		}
		if (OriginSort != -1) {
			const int c = compareStrings(a.origin, b.origin);
			if (c != 0) {
				return OriginSort == 0 ? c < 0 : c > 0;
			}
		}

		//then name within the group
		if (nameSort != -1) {
			const int c = compareStrings(a.name, b.name);
			if (c != 0) {
				return nameSort == 0 ? c < 0 : c > 0;
			}
		}
		return false;
	});

	std::vector<RE::TESObjectREFR*> sorted;
	sorted.reserve(entries.size());
	for (auto& entry : entries) {
		sorted.push_back(entry.ref);
	}
	return sorted;
}

std::vector<RE::TESObjectREFR*> FilterMapMarkersByIconType(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> mapMarkers, std::vector<int> iconTypes, bool matches){
	std::vector<RE::TESObjectREFR*> v; 
	if (iconTypes.empty() || mapMarkers.empty()){
		return v;
	}
	
	if (matches){
		for (auto* marker : mapMarkers){
			int type = GetMapMarkerIconType(nullptr, marker);
			if (std::find(iconTypes.begin(), iconTypes.end(), type) != iconTypes.end()){
				v.push_back(marker);
			}
		}
	}
	else {
		for (auto* marker : mapMarkers){
			int type = GetMapMarkerIconType(nullptr, marker);
			if (std::find(iconTypes.begin(), iconTypes.end(), type) == iconTypes.end()){
				v.push_back(marker);
			}
		}
	}
	
	return v;
}

std::vector<RE::TESObjectREFR*> FilterMapMarkersByOrigin(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> mapMarkers, std::vector<RE::TESForm*> origins, bool matches){
	std::vector<RE::TESObjectREFR*> v; 
	if (origins.empty() || mapMarkers.empty()){
		return v;
	}
	
	if (matches){
		for (auto* marker : mapMarkers){
			RE::TESForm* origin = GetCellOrWorldSpaceOriginForRef(nullptr, marker);
			if (std::find(origins.begin(), origins.end(), origin) != origins.end()){
				v.push_back(marker);
			}
		}
	}
	else {
		for (auto* marker : mapMarkers){
			RE::TESForm* origin = GetCellOrWorldSpaceOriginForRef(nullptr, marker);
			if (std::find(origins.begin(), origins.end(), origin) == origins.end()){
				v.push_back(marker);
			}
		}
	}
	return v;
}

std::vector<std::string> GetMapMarkerNames(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> mapMarkers){
	std::vector<std::string> v;
	if (mapMarkers.empty()){
		return v;
	}
	
	for (auto* marker : mapMarkers){
		v.push_back(GetMapMarkerName(nullptr, marker));
	}
	
	return v;
}

std::vector<int> GetMapMarkerIconTypes(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> mapMarkers){
	std::vector<int> v;
	if (mapMarkers.empty()){
		return v;
	}
	
	for (auto* marker : mapMarkers){
		v.push_back(GetMapMarkerIconType(nullptr, marker));
	}
	
	return v;
}

bool ShouldAddToPlayerMapMarkers(RE::TESObjectREFR* objRef) {
	auto* player = RE::PlayerCharacter::GetSingleton();
	
	if (player){
		RE::TESWorldSpace* playerWorldSpace = player->GetWorldspace();
		if (objRef->GetWorldspace() == playerWorldSpace && playerWorldSpace != nullptr) { 
			return true;
		} 
	}
	
	RE::TESObjectCELL* objCell = objRef->GetParentCell();
	if (gfuncs::IsFormValid(objCell)){
		if (objCell->IsAttached()){
			return true;
		}
	} 
	
	return false;
}

std::string GetNiPoint3String(RE::NiPoint3 point){
	return std::format("x [{}] y[{}] z[{}]",
		point.x,
		point.y,
		point.z
	);
}

// get the map marker currently highlighted in the map menu, if any.
// will return none if the "do you want to fast travel" menu is open.
RE::TESObjectREFR* GetHighlightedMapMarker(RE::StaticFunctionTag*) {
	auto* ui = RE::UI::GetSingleton();
    if (!ui) {
        return nullptr;
    }
	
	if (!ui->IsMenuOpen(RE::MapMenu::MENU_NAME) ){
		return nullptr;
	}
	
    RE::GPtr<RE::MapMenu> menu = ui->GetMenu<RE::MapMenu>();
    if (!menu) {
        return nullptr;
    }
	
    std::int32_t index = -1;
	int size = 0;
    RE::BSTArray<RE::MapMenuMarker>* markers = nullptr;
	bool localMapOpen = false;
	
	if (REL::Module::IsVR()) {
		auto* data = menu->GetVRRuntimeData();
		if (data){
			RE::LocalMapMenu* localMapMenu = &data->localMapMenu; 
			if (localMapMenu){ 
				auto* localData = &localMapMenu->GetRuntimeData();
				if (localData){
					if (localData->showingMap){
						localMapOpen = true;
						index = localData->selectedMarker;
						markers = &localMapMenu->mapMarkers;  
						if (markers){
							size = markers->size();
						}
						// logger::info("localMapMenu showing. Index[{}] markers size[{}]", index, size);
					}
				}
			} 
		}
		
		if (!localMapOpen){
			if (auto* rd = menu->GetRuntimeData2()) {
				index = rd->selectedMarker;
				markers = &rd->mapMarkers;
				if (markers){
					size = markers->size();
				}
				// logger::info("localMapMenu not showing. Index[{}] markers size[{}]", index, size);
			}
		}
	} 
	else { 
		auto* data = menu->GetRuntimeData();
		if (data){
			RE::LocalMapMenu* localMapMenu = &data->localMapMenu; 
			if (localMapMenu){ 
				auto* localData = &localMapMenu->GetRuntimeData();
				if (localData){
					if (localData->showingMap){
						localMapOpen = true;
						index = localData->selectedMarker;
						markers = &localMapMenu->mapMarkers;  
						if (markers){
							size = markers->size();
						}
						// logger::info("localMapMenu showing. Index[{}] markers size[{}]", index, size);
					}
				}
			} 
		}
		
		if (!localMapOpen){
			if (auto* rd = menu->GetRuntimeData2()) {
				index = rd->selectedMarker;
				markers = &rd->mapMarkers;
				if (markers){
					size = markers->size();
				}
				// logger::info("localMapMenu not showing. Index[{}] markers size[{}]", index, size);
			}
		}
	}

    if (size == 0 || index < 0 || index >= size) {
        return nullptr;   // nothing selected
    }
	
	RE::RefHandle handle = (*markers)[index].ref; 
	if (handle){
		RE::TESObjectREFR* ref = gfuncs::GetRefFromHandle(handle);
		if (gfuncs::IsFormValid(ref)){
			return ref;
		}
	}
	
	return nullptr;
}

bool CreateMapMarker(RE::StaticFunctionTag*, RE::TESObjectREFR* objRef, std::string name, int iconType, bool visible, bool canTravelTo) {
    if (!gfuncs::IsFormValid(objRef)) {
        logger::warn("objRef invalid");
        return false;
    }

    bool newMarkerCreated = false;
    auto* marker = objRef->extraList.GetByType<RE::ExtraMapMarker>();
    if (!marker) {
        marker = RE::BSExtraData::Create<RE::ExtraMapMarker>();
        if (!marker) { 
			logger::warn("Couldn't create ExtraMapMarker"); 
			return false; 
		}
        newMarkerCreated = true;
    }

    if (!marker->mapData) {
        // Create() only zeroes the block and installs the vtable -- the game's
        // ctor never runs, so mapData is null and it's ours to allocate.
        // Use the game allocator so the engine can free it on cell unload/save.
        auto* data = RE::calloc<RE::MapMarkerData>(1);
        if (!data) { 
			logger::warn("Couldn't create MapMarkerData"); 
			return false; 
		}
		
		//was for testing if extraList.RemoveByType in DestroyMapMarker was deleting MapMarkerData. 
		//It was, so no need to use RE::free(marker->mapData) in the DestroyMapMarker function.
		// logger::debug("MapMarkerData alloc[{}]", static_cast<void*>(data));
		
        // TESFullName has virtuals and no ctor ran, so install its vtable by
        // hand or the game calls through a null vptr on the first name read.
        REL::Relocation<std::uintptr_t> fullNameVtbl{ RE::TESFullName::VTABLE[0] };
        *reinterpret_cast<std::uintptr_t*>(data) = fullNameVtbl.address();

        marker->mapData = data;
    }

    marker->mapData->locationName.fullName = name;
	marker->mapData->type = static_cast<RE::MARKER_TYPE>(iconType);
	marker->mapData->SetVisible(visible);
	if (canTravelTo) {
		marker->mapData->flags.set(RE::MapMarkerData::Flag::kCanTravelTo);
	}
	else {
		marker->mapData->flags.reset(RE::MapMarkerData::Flag::kCanTravelTo);
	}
	
    if (newMarkerCreated) { 
		objRef->extraList.Add(marker); 
	}

	//map markers must have an origin to be visible on the map
	if (!GetCellOrWorldSpaceOriginForRef(nullptr, objRef)){
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (player){
			RE::TESForm* origin = player->GetWorldspace(); 
			if (!gfuncs::IsFormValid(origin)){
				origin = player->GetParentCell();
			}
			if (gfuncs::IsFormValid(origin)){
				SetCellOrWorldSpaceOriginForRef(nullptr, objRef, origin);
			}
		}
	}
	
	if (ShouldAddToPlayerMapMarkers(objRef)) {
    	auto* playerMapMarkers = GetPlayerMapMarkers();
    	if (playerMapMarkers) { 
			if (playerMapMarkers->size() > 0){
				RE::ObjectRefHandle handle = objRef->GetHandle();
				bool handleFound = false;
				for (auto& mapMarker : *playerMapMarkers) {
					if (mapMarker == handle){
						handleFound = true; 
						break;
					}
				}
				
				if (!handleFound){
					playerMapMarkers->push_back(handle);
				}
			}
		}
	}
	
    logger::info("marker on [{}] name[{}] type[{}]", gfuncs::GetFormDataString(objRef), name, iconType);
                 
    return true;
}

bool DestroyMapMarker(RE::StaticFunctionTag*, RE::TESObjectREFR* objRef) {
    if (!gfuncs::IsFormValid(objRef)) {
        logger::warn("objRef invalid");
        return false;
    }

    auto* marker = objRef->extraList.GetByType<RE::ExtraMapMarker>();
    if (!marker) {
        logger::debug("no ExtraMapMarker on ref {}", gfuncs::GetFormNameAndId(objRef));
        return false;
    }

    // hide first, so anything reading the list this frame sees it gone
    if (marker->mapData) {
        marker->mapData->flags.reset(RE::MapMarkerData::Flag::kVisible,
                                     RE::MapMarkerData::Flag::kCanTravelTo);
    }
	
	auto* playerMapMarkers = GetPlayerMapMarkers();
	
    // drop the handle from the player's marker array
    if (playerMapMarkers) {
		if (playerMapMarkers->size() > 0){
			const auto handle = objRef->GetHandle();
			for (std::uint32_t i = 0; i < playerMapMarkers->size(); i++) {
				if ((*playerMapMarkers)[i] == handle) {
					playerMapMarkers->erase(playerMapMarkers->begin() + i);
					break;
				}
			} 
		}
    }
	
    return objRef->extraList.RemoveByType(RE::ExtraDataType::kMapMarker); ;
} 

void LogMapMarkerRefs(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> mapMarkers){
	std::uint32_t i = 0; 
	
	for (auto* marker : mapMarkers){
		if (!gfuncs::IsFormValid(marker)){
			logger::info("[{}]: marker is none", i);
			continue;
		}
		
		std::string name = GetMapMarkerName(nullptr, marker);
		std::string sIcon = GetMapMarkerIconTypeString(nullptr, marker);
		RE::TESForm* origin = GetCellOrWorldSpaceOriginForRef(nullptr, marker);
		std::string sOrigin = gfuncs::GetFormDataString(origin);
		RE::FormID id = marker->GetFormID(); 
		
		logger::info("[{}]: name[{}] icon[{}] formId[{:08X}] origin form: --{}--", 
			i, name, sIcon, id, sOrigin
		);
		i++;
	}
}

namespace MapMarker {
	void RegisterMapMarkerFrameworkListener() { //called from kPostLoad in plugin.cpp 
		if (SKSE::GetMessagingInterface()->RegisterListener("MapMarkerFramework", mmf::OnMessage)) {
			logger::info("listening for MapMarkerFramework");
		}
	}
	
	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    	vm->RegisterFunction("IsMapMarker", "DbSkseFunctions", IsMapMarker);
    	vm->RegisterFunction("IsRefVisibleOnLocalMap", "DbSkseFunctions", IsRefVisibleOnLocalMap);
    	vm->RegisterFunction("GetRefsVisibleOnTheLocalMap", "DbSkseFunctions", GetRefsVisibleOnTheLocalMap);
		vm->RegisterFunction("GetHighlightedMapMarker", "DbSkseFunctions", GetHighlightedMapMarker);
		vm->RegisterFunction("CreateMapMarker", "DbSkseFunctions", CreateMapMarker);
		vm->RegisterFunction("DestroyMapMarker", "DbSkseFunctions", DestroyMapMarker);
		vm->RegisterFunction("SetMapMarkerName", "DbSkseFunctions", SetMapMarkerName);
		vm->RegisterFunction("GetMapMarkerName", "DbSkseFunctions", GetMapMarkerName);
		vm->RegisterFunction("SetMapMarkerIconType", "DbSkseFunctions", SetMapMarkerIconType);
		vm->RegisterFunction("GetMapMarkerIconType", "DbSkseFunctions", GetMapMarkerIconType);
		vm->RegisterFunction("GetMapMarkerIconTypeString", "DbSkseFunctions", GetMapMarkerIconTypeString);
		vm->RegisterFunction("SetMapMarkerVisible", "DbSkseFunctions", SetMapMarkerVisible);
		vm->RegisterFunction("SetCanFastTravelToMarker", "DbSkseFunctions", SetCanFastTravelToMarker);
		vm->RegisterFunction("GetMapMarkerVisibilityStatus", "DbSkseFunctions", GetMapMarkerVisibilityStatus);
		vm->RegisterFunction("GetMapMarkerVisibilityStatuses", "DbSkseFunctions", GetMapMarkerVisibilityStatuses);
		vm->RegisterFunction("GetAllMapMarkerRefs", "DbSkseFunctions", GetAllMapMarkerRefs);
		vm->RegisterFunction("GetCurrentMapMarkerRefs", "DbSkseFunctions", GetCurrentMapMarkerRefs);
		vm->RegisterFunction("GetCellOrWorldSpaceOriginForRef", "DbSkseFunctions", GetCellOrWorldSpaceOriginForRef);
		vm->RegisterFunction("GetCellOrWorldSpaceOriginForRefs", "DbSkseFunctions", GetCellOrWorldSpaceOriginForRefs);
		vm->RegisterFunction("SetCellOrWorldSpaceOriginForRef", "DbSkseFunctions", SetCellOrWorldSpaceOriginForRef);
		vm->RegisterFunction("SortMapMarkers", "DbSkseFunctions", SortMapMarkers);
		vm->RegisterFunction("FilterMapMarkersByIconType", "DbSkseFunctions", FilterMapMarkersByIconType);
		vm->RegisterFunction("FilterMapMarkersByOrigin", "DbSkseFunctions", FilterMapMarkersByOrigin);
		vm->RegisterFunction("GetMapMarkerNames", "DbSkseFunctions", GetMapMarkerNames);
		vm->RegisterFunction("GetMapMarkerIconTypes", "DbSkseFunctions", GetMapMarkerIconTypes);
		vm->RegisterFunction("LogMapMarkerRefs", "DbSkseFunctions", LogMapMarkerRefs);
		
		vm->RegisterFunction("IsMapMarkerFrameworkInstalled", "DbSkseFunctions", IsMapMarkerFrameworkInstalled);
		vm->RegisterFunction("GetMapMarkerFrameworkIconTypeName", "DbSkseFunctions", GetMapMarkerFrameworkIconTypeName);
		vm->RegisterFunction("GetMapMarkerFrameworkIconTypeNames", "DbSkseFunctions", GetMapMarkerFrameworkIconTypeNames);
		vm->RegisterFunction("GetMapMarkerFrameworkIconType", "DbSkseFunctions", GetMapMarkerFrameworkIconType);
		vm->RegisterFunction("SetMapMarkerFrameworkIconType", "DbSkseFunctions", SetMapMarkerFrameworkIconType);
		vm->RegisterFunction("GetAllMapMarkerFrameworkRefs", "DbSkseFunctions", GetAllMapMarkerFrameworkRefs);
        return true;
    }
} 
