#include "MapMarker.h"
#include "GeneralFunctions.h"
#include "RE/E/ExtraTeleport.h"
#include "RE/L/LocalMapCamera.h"
#include "RE/N/NiPoint3.h"
#include "RE/T/TESFullName.h"
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
#include <cstddef>
#include <format>

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
	
	if (!sv::ui) {
		logger::warn("ui doesn't exist");
		return false;
	}
	
	RE::GPtr<RE::MapMenu> menu = sv::ui->GetMenu<RE::MapMenu>();
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
	
	if (!sv::ui) {
		logger::warn("ui doesn't exist");
		return refs;
	}
	
	RE::GPtr<RE::MapMenu> menu = sv::ui->GetMenu<RE::MapMenu>();
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

// Get all map markers valid for the current worldspace or interior cell grid
RE::BSTArray<RE::ObjectRefHandle>* GetPlayerMapMarkers() {
	if (!sv::player){
		return nullptr; 
	}
	
	if (REL::Module::IsVR()){
		auto* runtimeData = sv::player->GetVRPlayerRuntimeData();
		return &runtimeData->currentMapMarkers;
	}
	else {
		auto& runtimeData = sv::player->GetPlayerRuntimeData();
		return &runtimeData.currentMapMarkers;
	}
}

std::vector<RE::TESObjectREFR*> GetAllMapMarkerRefs(RE::StaticFunctionTag*, int visibleFilter, int canTravelToFilter) {
    std::vector<RE::TESObjectREFR*> allMapMarkers;

    if (visibleFilter == 1 && canTravelToFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
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
        logger::debug("originData for ref({}) not found, getting current worldspace or cell.", gfuncs::GetFormDataString(ref));

        //origin data on a reference will only exist if it has been moved from its original worldspace or interior cell
        return GetRefWorldSpaceOrCell(ref);
    }

    if (!gfuncs::IsFormValid(originData->startingWorldOrCell)) {
        logger::debug("originData->startingWorldOrCell for ref({}) doesn't exist, getting current worldspace or cell.", gfuncs::GetFormDataString(ref));
        return GetRefWorldSpaceOrCell(ref);
    }

    return originData->startingWorldOrCell;
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

    if (sv::player) {
        if (IsMapMarker(nullptr, ref)) {
            RE::TESWorldSpace* newOriginWorld = static_cast<RE::TESWorldSpace*>(cellOrWorldSpace);

            if (gfuncs::IsFormValid(newOriginWorld)) {
                RE::TESWorldSpace* currentWorld = sv::player->GetWorldspace();
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
        return false;
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        logger::warn("GetMapMarkerIconType Warning, map marker list not found.");
        return false;
    }

    if (!mapMarkerData->mapData) {
        logger::warn("GetMapMarkerIconType Warning, mapData not found.");
        return false;
    }

    return static_cast<int>(mapMarkerData->mapData->type.get());
} 

bool ShouldAddToPlayerMapMarkers(RE::TESObjectREFR* objRef) {
	RE::TESWorldSpace* playerWorldSpace = sv::player->GetWorldspace();
	
	if (objRef->GetWorldspace() == playerWorldSpace && playerWorldSpace != nullptr) { 
		return true;
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
    if (!sv::ui) {
        return nullptr;
    }
	
	if (!sv::ui->IsMenuOpen(RE::MapMenu::MENU_NAME) ){
		return nullptr;
	}
	
    RE::GPtr<RE::MapMenu> menu = sv::ui->GetMenu<RE::MapMenu>();
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
		RE::TESForm* origin = sv::player->GetWorldspace(); 
		if (!gfuncs::IsFormValid(origin)){
			origin = sv::player->GetParentCell();
		}
		if (gfuncs::IsFormValid(origin)){
			SetCellOrWorldSpaceOriginForRef(nullptr, objRef, origin);
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

namespace MapMarker {
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
		vm->RegisterFunction("SetMapMarkerVisible", "DbSkseFunctions", SetMapMarkerVisible);
		vm->RegisterFunction("SetCanFastTravelToMarker", "DbSkseFunctions", SetCanFastTravelToMarker);
		vm->RegisterFunction("GetAllMapMarkerRefs", "DbSkseFunctions", GetAllMapMarkerRefs);
		vm->RegisterFunction("GetCurrentMapMarkerRefs", "DbSkseFunctions", GetCurrentMapMarkerRefs);
		vm->RegisterFunction("GetCellOrWorldSpaceOriginForRef", "DbSkseFunctions", GetCellOrWorldSpaceOriginForRef);
		vm->RegisterFunction("SetCellOrWorldSpaceOriginForRef", "DbSkseFunctions", SetCellOrWorldSpaceOriginForRef);
        return true;
    }
}