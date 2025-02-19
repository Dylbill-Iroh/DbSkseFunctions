#pragma once

void SetArtObjectNthTextureSet(RE::StaticFunctionTag*, RE::BGSArtObject* akArtObject, RE::BGSTextureSet* akTextureSet, int n);

RE::BGSTextureSet* GetArtObjectNthTextureSet(RE::StaticFunctionTag*, RE::BGSArtObject* akArtObject, int n);

RE::BSFixedString GetArtObjectModelNth3dName(RE::StaticFunctionTag*, RE::BGSArtObject* akArtObject, int n);

int GetArtObjectNumOfTextureSets(RE::StaticFunctionTag*, RE::BGSArtObject* akArtObject);