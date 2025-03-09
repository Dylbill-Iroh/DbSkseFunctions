#pragma once

bool IsMapMarker(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker);

bool SetMapMarkerVisible(RE::StaticFunctionTag*, RE::TESObjectREFR * mapMarker, bool visible);

bool SetCanFastTravelToMarker(RE::StaticFunctionTag*, RE::TESObjectREFR * mapMarker, bool canTravelTo);

RE::BSTArray<RE::ObjectRefHandle>* GetPlayerMapMarkers();

std::vector<RE::TESObjectREFR*> GetAllMapMarkerRefs(RE::StaticFunctionTag*, int visibleFilter, int canTravelToFilter);

std::vector<RE::TESObjectREFR*> GetCurrentMapMarkerRefs(RE::StaticFunctionTag*, int visibleFilter, int canTravelToFilter);

bool LoadMostRecentSaveGame(RE::StaticFunctionTag*);

RE::TESForm* GetRefWorldSpaceOrCell(RE::TESObjectREFR * ref);

RE::TESForm* GetCellOrWorldSpaceOriginForRef(RE::StaticFunctionTag*, RE::TESObjectREFR * ref);

bool SetCellOrWorldSpaceOriginForRef(RE::StaticFunctionTag*, RE::TESObjectREFR * ref, RE::TESForm * cellOrWorldSpace);

bool SetMapMarkerName(RE::StaticFunctionTag*, RE::TESObjectREFR * mapMarker, std::string name);

std::string GetMapMarkerName(RE::StaticFunctionTag*, RE::TESObjectREFR * mapMarker);

bool SetMapMarkerIconType(RE::StaticFunctionTag*, RE::TESObjectREFR * mapMarker, int iconType);

int GetMapMarkerIconType(RE::StaticFunctionTag*, RE::TESObjectREFR * mapMarker);

