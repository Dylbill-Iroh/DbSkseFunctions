#pragma once

RE::BGSKeyword* CreateKeyword(RE::StaticFunctionTag*);

RE::BGSListForm* CreateFormList(RE::StaticFunctionTag*, RE::BGSListForm * formListFiller);

RE::BGSColorForm* CreateColorForm(RE::StaticFunctionTag*, int color);

RE::BGSConstructibleObject* CreateConstructibleObject(RE::StaticFunctionTag*);

RE::BGSTextureSet* CreateTextureSet(RE::StaticFunctionTag*);

RE::TESSound* CreateSoundMarker(RE::StaticFunctionTag*);

