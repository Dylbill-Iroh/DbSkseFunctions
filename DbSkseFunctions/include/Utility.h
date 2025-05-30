#pragma once

#include <shared_mutex>

class SharedUIVariables {
public:
    // State variables
    std::vector<std::string> openedMenus;
    std::string lastMenuOpened = "";
    bool gamePaused = false;
    bool inMenuMode = false;
    std::chrono::system_clock::time_point lastTimeMenuWasOpened;
    std::chrono::system_clock::time_point lastTimeGameWasPaused;
    RE::TESObjectREFR* menuRef = nullptr;
    RE::TESObjectREFR* lastPlayerActivatedRef = nullptr;

    // read/write
    void update(std::function<void(SharedUIVariables&)> updater) {
        std::unique_lock lock(mutex);
        updater(*this);
    }

    void read(std::function<void(const SharedUIVariables&)> reader) const {
        std::shared_lock lock(mutex);
        reader(*this);
    }

private:
    mutable std::shared_mutex mutex;
};

extern SharedUIVariables sharedVars;

float GetGameHoursPassed(RE::StaticFunctionTag*);

float GameHoursToRealTimeSeconds(RE::StaticFunctionTag*, float gameHours);

bool IsGamePaused(RE::StaticFunctionTag*);

bool IsInMenu(RE::StaticFunctionTag*);

std::string GetLastMenuOpened(RE::StaticFunctionTag*);

void RefreshItemMenu(RE::StaticFunctionTag*);

RE::TESForm* GetLoadMenuLocation();

std::vector<std::string> GetFormDescriptionsAsStrings(std::vector<RE::TESForm*> akForms, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString);

std::vector<std::string> GetFormDescriptionsAsStrings(RE::BGSListForm * akFormlist, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString);

std::vector<std::string> getFormNamesAndDescriptionsAsStrings(std::vector<RE::TESForm*> akForms, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString);

std::vector<std::string> getFormNamesAndDescriptionsAsStrings(RE::BGSListForm * akFormlist, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString);

std::vector<std::string> GetFormNamesAsStrings(std::vector<RE::TESForm*> akForms, int sortOption, int noneStringType, std::string nullFormString);

std::vector<std::string> GetFormNamesAsStrings(RE::BGSListForm * akFormlist, int sortOption, int noneStringType, std::string nullFormString);

std::vector<std::string> GetFormEditorIdsAsStrings(std::vector<RE::TESForm*> akForms, int sortOption, std::string nullFormString);

std::vector<std::string> GetFormEditorIdsAsStrings(RE::BGSListForm * akFormlist, int sortOption, std::string nullFormString);

std::vector<std::string> GetLoadedModNamesAsStrings(int sortOption);

std::vector<std::string> GetLoadedLightModNamesAsStrings(int sortOption);

std::vector<std::string> GetLoadedModDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer);

std::vector<std::string> GetLoadedModNamesAndDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer);

std::vector<std::string> GetLoadedLightModDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer);

std::vector<std::string> GetLoadedLightModNamesAndDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer);

std::vector<std::string> GetAllLoadedModDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer);

std::vector<std::string> GetAllLoadedModNamesAndDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer);

std::string GetClipBoardText(RE::StaticFunctionTag*);

bool SetClipBoardText(RE::StaticFunctionTag*, std::string sText);

bool IsWhiteSpace(RE::StaticFunctionTag*, std::string s);

int CountWhiteSpaces(RE::StaticFunctionTag*, std::string s);

bool ModHasFormType(RE::StaticFunctionTag*, std::string modName, int formType);

std::string GetFormEditorId(RE::StaticFunctionTag*, RE::TESForm * akForm, std::string nullFormString);

std::vector<RE::BSFixedString> GetFormEditorIds(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, int sortOption, std::string nullFormString);

std::vector<RE::BSFixedString> GetFormEditorIdsFromList(RE::StaticFunctionTag*, RE::BGSListForm * akFormlist, int sortOption, std::string nullFormString);

std::vector<RE::TESForm*> SortFormArray(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, int sortOption);

std::vector<RE::TESForm*> FormListToArray(RE::StaticFunctionTag*, RE::BGSListForm * akFormlist, int sortOption);

void AddFormsToList(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, RE::BGSListForm * akFormlist);

std::string GetEffectsDescriptions(RE::BSTArray<RE::Effect*> effects);

std::string GetMagicItemDescription(RE::MagicItem * magicItem);

std::string GetDescription(RE::TESForm * akForm, std::string newLineReplacer);

std::string GetFormDescription(RE::StaticFunctionTag*, RE::TESForm * akForm, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString);

std::vector<RE::BSFixedString> GetFormDescriptions(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString);

std::vector<RE::BSFixedString> GetFormDescriptionsFromList(RE::StaticFunctionTag*, RE::BGSListForm * akFormlist, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString);

std::vector<RE::BSFixedString> GetFormNames(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, int sortOption, int noneStringType, std::string nullFormString);

std::vector<RE::BSFixedString> GetFormNamesFromList(RE::StaticFunctionTag*, RE::BGSListForm * akFormlist, int sortOption, int noneStringType, std::string nullFormString);

std::vector<RE::BSFixedString> GetLoadedModNames(RE::StaticFunctionTag*, int sortOption);

std::vector<RE::BSFixedString> GetLoadedLightModNames(RE::StaticFunctionTag*, int sortOption);

std::vector<RE::BSFixedString> GetAllLoadedModNames(RE::StaticFunctionTag*, int sortOption);

std::vector<RE::BSFixedString> GetLoadedModDescriptions(RE::StaticFunctionTag*, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer);

std::vector<RE::BSFixedString> GetLoadedLightModDescriptions(RE::StaticFunctionTag*, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer);

std::vector<RE::BSFixedString> GetAllLoadedModDescriptions(RE::StaticFunctionTag*, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer);

bool FormNameMatches(RE::TESForm * akForm, std::string & sFormName);

bool FormNameContains(RE::TESForm * akForm, std::string & sFormName);

