#include "ArtObject.h"
#include "GeneralFunctions.h"

void SetArtObjectNthTextureSet(RE::StaticFunctionTag*, RE::BGSArtObject* akArtObject, RE::BGSTextureSet* akTextureSet, int n) {
    if (!gfuncs::IsFormValid(akArtObject)) {
        logger::warn("akArtObject isn't valid");
        return;
    }

    if (!gfuncs::IsFormValid(akTextureSet)) {
        logger::warn("akTextureSet isn't valid");
        return;
    }

    RE::TESModelTextureSwap* swap = akArtObject->As<RE::TESModelTextureSwap>();
    if (!swap) {
        logger::warn("TESModelTextureSwap not found for[{}]", gfuncs::GetFormName(akArtObject));
        return;
    }

    if (!swap->alternateTextures) {
        logger::warn("swap->alternateTextures not found for[{}]", gfuncs::GetFormName(akArtObject));
        return;
    }

    if (n >= swap->numAlternateTextures || n < 0) {
        logger::warn("index n[{}] for [{}] is out of range from swap->numAlternateTextures[{}]", n, gfuncs::GetFormName(akArtObject), swap->numAlternateTextures);
        return;
    }
    swap->alternateTextures[n].textureSet = akTextureSet;
}

RE::BGSTextureSet* GetArtObjectNthTextureSet(RE::StaticFunctionTag*, RE::BGSArtObject* akArtObject, int n) {
    if (!gfuncs::IsFormValid(akArtObject)) {
        logger::warn("akArtObject isn't valid");
        return nullptr;
    }

    RE::TESModelTextureSwap* swap = akArtObject->As<RE::TESModelTextureSwap>();
    if (!swap) {
        logger::warn("TESModelTextureSwap not found for[{}]", gfuncs::GetFormName(akArtObject));
        return nullptr;
    }

    if (!swap->alternateTextures) {
        logger::warn("swap->alternateTextures not found for[{}]", gfuncs::GetFormName(akArtObject));
        return nullptr;
    }

    if (n >= swap->numAlternateTextures || n < 0) {
        logger::warn("index n[{}] for [{}] is out of range from swap->numAlternateTextures[{}]", n, gfuncs::GetFormName(akArtObject), swap->numAlternateTextures);
        return nullptr;
    }

    if (!gfuncs::IsFormValid(swap->alternateTextures[n].textureSet)) {
        logger::warn("textureSet at index n[{}] for [{}] isn't valid", n, gfuncs::GetFormName(akArtObject), swap->numAlternateTextures);
        return nullptr;
    }
    return swap->alternateTextures[n].textureSet;
}

RE::BSFixedString GetArtObjectModelNth3dName(RE::StaticFunctionTag*, RE::BGSArtObject* akArtObject, int n) {
    if (!gfuncs::IsFormValid(akArtObject)) {
        logger::warn("akArtObject isn't valid");
        return "";
    }

    RE::TESModelTextureSwap* swap = akArtObject->As<RE::TESModelTextureSwap>();
    if (!swap) {
        logger::warn("TESModelTextureSwap not found for[{}]", gfuncs::GetFormName(akArtObject));
        return "";
    }

    if (!swap->alternateTextures) {
        logger::warn("swap->alternateTextures not found for[{}]", gfuncs::GetFormName(akArtObject));
        return "";
    }

    if (n >= swap->numAlternateTextures || n < 0) {
        logger::warn("index n[{}] for [{}] is out of range from swap->numAlternateTextures[{}]", n, gfuncs::GetFormName(akArtObject), swap->numAlternateTextures);
        return "";
    }

    return swap->alternateTextures[n].name3D;
}

int GetArtObjectNumOfTextureSets(RE::StaticFunctionTag*, RE::BGSArtObject* akArtObject) {
    if (!gfuncs::IsFormValid(akArtObject)) {
        logger::warn("akArtObject isn't valid");
        return 0;
    }

    RE::TESModelTextureSwap* swap = akArtObject->As<RE::TESModelTextureSwap>();
    if (!swap) {
        logger::warn("TESModelTextureSwap not found for[{}]", gfuncs::GetFormName(akArtObject));
        return 0;
    }

    return swap->numAlternateTextures;
}

RE::BSFixedString GetFormWorldModelNth3dName(RE::StaticFunctionTag*, RE::TESForm* akForm, int n) {
    if (!gfuncs::IsFormValid(akForm)) {
        logger::warn("akForm isn't valid");
        return "";
    }

    RE::TESModel* model = akForm->As<RE::TESModel>();
    if (!model) {
        logger::info("model not found for [{}]", gfuncs::GetFormName(akForm));
        return "";
    }
   
    RE::TESModelTextureSwap* swap = akForm->As<RE::TESModelTextureSwap>();
    if (!swap) {
        logger::warn("TESModelTextureSwap not found for [{}]", gfuncs::GetFormName(akForm));
        return "";
    }

    if (!swap->alternateTextures) { 
        logger::warn("swap->alternateTextures not found for [{}]", gfuncs::GetFormName(akForm));
        return "";
    }

    if (n >= swap->numAlternateTextures || n < 0) {
        logger::warn("index n[{}] for [{}] is out of range from swap->numAlternateTextures[{}]", n, gfuncs::GetFormName(akForm), swap->numAlternateTextures);
        return "";
    }

    return swap->alternateTextures[n].name3D;
}