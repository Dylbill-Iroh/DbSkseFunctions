#include "MapMarker.h"
#include "GeneralFunctions.h"
#include "SharedVariables.h"

enum logLevel { trace, debug, info, warn, error, critical };
enum debugLevel { notification, messageBox };

bool IsMapMarker(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker) {
    logger::trace("function called");

    if (!gfuncs::IsFormValid(mapMarker)) {
        logger::warn("IsMapMarker: mapMarker doesn't exist", warn);
        return false;
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        logger::warn("map marker list not found.", debug);
        return false;
    }

    if (!mapMarkerData->mapData) {
        logger::warn("mapData not found.", debug);
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
    if (sv::player) {
        std::uint32_t offset = 0;
        if (REL::Module::IsAE())
            offset = 0x500;
        else if (REL::Module::IsSE())
            offset = 0x4F8;
        else
            offset = 0xAE8;
        return reinterpret_cast<RE::BSTArray<RE::ObjectRefHandle>*>((uintptr_t)sv::player + offset);
    } 
    else {
        RE::BSTArray<RE::ObjectRefHandle> emptyArray;
        return &emptyArray;
    }
}

std::vector<RE::TESObjectREFR*> GetAllMapMarkerRefs(RE::StaticFunctionTag*, int visibleFilter, int canTravelToFilter) {
    std::vector<RE::TESObjectREFR*> allMapMarkers;

    if (visibleFilter == 1 && canTravelToFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
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
            if (gfuncs::IsFormValid(form)) {
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
            if (gfuncs::IsFormValid(form)) {
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
            if (gfuncs::IsFormValid(form)) {
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
            if (gfuncs::IsFormValid(form)) {
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
            if (gfuncs::IsFormValid(form)) {
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
            if (gfuncs::IsFormValid(form)) {
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
            if (gfuncs::IsFormValid(form)) {
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
            if (gfuncs::IsFormValid(form)) {
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

bool LoadMostRecentSaveGame(RE::StaticFunctionTag*) {
    //auto manager = RE::UISaveLoadManager::ProcessEvent();
    auto* manager = RE::BGSSaveLoadManager::GetSingleton();
    if (!manager) {
        logger::error("BGSSaveLoadManager not found");
        return false;
    }

    logger::trace("loading most recent save");

    return manager->LoadMostRecentSaveGame();
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