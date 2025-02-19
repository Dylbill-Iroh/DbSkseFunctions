#include "ArtObject.h"
#include "GeneralFunctions.h"

namespace logger = SKSE::log;

void SetArtObjectNthTextureSet(RE::StaticFunctionTag*, RE::BGSArtObject* akArtObject, RE::BGSTextureSet* akTextureSet, int n) {
    if (!gfuncs::IsFormValid(akArtObject)) {
        logger::warn("{}: akArtObject isn't valid", __func__);
        return;
    }

    if (!gfuncs::IsFormValid(akTextureSet)) {
        logger::warn("{}: akTextureSet isn't valid", __func__);
        return;
    }

    RE::TESModelTextureSwap* swap = akArtObject->As<RE::TESModelTextureSwap>();
    if (!swap) {
        logger::warn("{}: TESModelTextureSwap not found for[{}]", __func__, gfuncs::GetFormName(akArtObject));
        return;
    }

    if (!swap->alternateTextures) {
        logger::warn("{}: swap->alternateTextures not found for[{}]", __func__, gfuncs::GetFormName(akArtObject));
        return;
    }

    if (n >= swap->numAlternateTextures || n < 0) {
        logger::warn("{}: index n[{}] for [{}] is out of range from swap->numAlternateTextures[{}]", __func__, n, gfuncs::GetFormName(akArtObject), swap->numAlternateTextures);
        return;
    }
    swap->alternateTextures[n].textureSet = akTextureSet;
}

RE::BGSTextureSet* GetArtObjectNthTextureSet(RE::StaticFunctionTag*, RE::BGSArtObject* akArtObject, int n) {
    if (!gfuncs::IsFormValid(akArtObject)) {
        logger::warn("{}: akArtObject isn't valid", __func__);
        return nullptr;
    }

    RE::TESModelTextureSwap* swap = akArtObject->As<RE::TESModelTextureSwap>();
    if (!swap) {
        logger::warn("{}: TESModelTextureSwap not found for[{}]", __func__, gfuncs::GetFormName(akArtObject));
        return nullptr;
    }

    if (!swap->alternateTextures) {
        logger::warn("{}: swap->alternateTextures not found for[{}]", __func__, gfuncs::GetFormName(akArtObject));
        return nullptr;
    }

    if (n >= swap->numAlternateTextures || n < 0) {
        logger::warn("{}: index n[{}] for [{}] is out of range from swap->numAlternateTextures[{}]", __func__, n, gfuncs::GetFormName(akArtObject), swap->numAlternateTextures);
        return nullptr;
    }

    if (!gfuncs::IsFormValid(swap->alternateTextures[n].textureSet)) {
        logger::warn("{}: textureSet at index n[{}] for [{}] isn't valid", __func__, n, gfuncs::GetFormName(akArtObject), swap->numAlternateTextures);
        return nullptr;
    }
    return swap->alternateTextures[n].textureSet;
}

RE::BSFixedString GetArtObjectModelNth3dName(RE::StaticFunctionTag*, RE::BGSArtObject* akArtObject, int n) {
    if (!gfuncs::IsFormValid(akArtObject)) {
        logger::warn("{}: akArtObject isn't valid", __func__);
        return "";
    }

    RE::TESModelTextureSwap* swap = akArtObject->As<RE::TESModelTextureSwap>();
    if (!swap) {
        logger::warn("{}: TESModelTextureSwap not found for[{}]", __func__, gfuncs::GetFormName(akArtObject));
        return "";
    }

    if (!swap->alternateTextures) {
        logger::warn("{}: swap->alternateTextures not found for[{}]", __func__, gfuncs::GetFormName(akArtObject));
        return "";
    }

    if (n >= swap->numAlternateTextures || n < 0) {
        logger::warn("{}: index n[{}] for [{}] is out of range from swap->numAlternateTextures[{}]", __func__, n, gfuncs::GetFormName(akArtObject), swap->numAlternateTextures);
        return "";
    }

    return swap->alternateTextures[n].name3D;
}

int GetArtObjectNumOfTextureSets(RE::StaticFunctionTag*, RE::BGSArtObject* akArtObject) {
    if (!gfuncs::IsFormValid(akArtObject)) {
        logger::warn("{}: akArtObject isn't valid", __func__);
        return 0;
    }

    RE::TESModelTextureSwap* swap = akArtObject->As<RE::TESModelTextureSwap>();
    if (!swap) {
        logger::warn("{}: TESModelTextureSwap not found for[{}]", __func__, gfuncs::GetFormName(akArtObject));
        return 0;
    }

    return swap->numAlternateTextures;
}