#include <Windows.h>
#include "Utility.h"
#include "GeneralFunctions.h"
#include "editorID.hpp"

enum logLevel { trace, debug, info, warn, error, critical };
enum debugLevel { notification, messageBox };

std::vector<std::string> magicDescriptionTags = { "<mag>", "<dur>", "<area>" };

bool inMenuMode = false;
std::string lastMenuOpened = "";
std::chrono::system_clock::time_point lastTimeMenuWasOpened;
std::chrono::system_clock::time_point lastTimeGameWasPaused;

//forward dec
std::string GetDescription(RE::TESForm* akForm, std::string newLineReplacer);

float GetGameHoursPassed(RE::StaticFunctionTag*) {
    auto* calendar = RE::Calendar::GetSingleton(); 
    if (!calendar) {
        logger::error("calendar not found");
        return 0.0;
    }
    return calendar->GetHoursPassed();
}

float GameHoursToRealTimeSeconds(RE::StaticFunctionTag*, float gameHours) {
    auto* calendar = RE::Calendar::GetSingleton();
    if (!calendar) {
        logger::error("calendar not found");
        return 0.0;
    }
    float timeScale = calendar->GetTimescale(); //timeScale is minutes ratio
    float gameMinutes = gameHours * 60.0;
    float realMinutes = gameMinutes / timeScale;
    return (realMinutes * 60.0);
}

bool IsGamePaused(RE::StaticFunctionTag*) {
    auto* ui = RE::UI::GetSingleton();
    if (!ui) {
        logger::error("ui not found");
        return false;
    }
    return ui->GameIsPaused();
}

bool IsInMenu(RE::StaticFunctionTag*) {
    return inMenuMode;
}

std::string GetLastMenuOpened(RE::StaticFunctionTag*) {
    return lastMenuOpened;
}

void RefreshItemMenu(RE::StaticFunctionTag*) {
    gfuncs::RefreshItemMenu();
}

RE::TESForm* GetLoadMenuLocation() {
    auto* ui = RE::UI::GetSingleton();
    if (!ui) {
        logger::error("couldn't find ui");
        return nullptr;
    }

    auto loadingMenuGPtr = ui->GetMenu(RE::LoadingMenu::MENU_NAME);
    if (!loadingMenuGPtr) {
        logger::error("couldn't find loadingMenu");
        return nullptr;
    }

    auto* uiMenu = loadingMenuGPtr.get();
    if (!uiMenu) {
        logger::error("couldn't find uiMenu from loadingMenuGPtr");
        return nullptr;
    }

    RE::LoadingMenu* loadingMenu = static_cast<RE::LoadingMenu*>(uiMenu);
    if (!loadingMenu) {
        logger::error("couldn't find loadingMenu from uiMenu");
        return nullptr;
    }

    auto data = loadingMenu->GetRuntimeData();
    if (gfuncs::IsFormValid(data.currentLocation)) {
        return data.currentLocation;
    }
    else {
        return nullptr;
    }
}

std::vector<std::string> GetFormDescriptionsAsStrings(std::vector<RE::TESForm*> akForms, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    std::vector<std::string> descriptions;

    if (noneStringType == 2 && maxCharacters > 0) {
        for (auto* akForm : akForms) {
            if (!gfuncs::IsFormValid(akForm)) {
                descriptions.push_back(nullFormString);
                continue;
            }
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = gfuncs::IntToHex(akForm->GetFormID());
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            descriptions.push_back(description);
        }
    }
    else if (noneStringType == 1 && maxCharacters > 0) {
        for (auto* akForm : akForms) {
            if (!gfuncs::IsFormValid(akForm)) {
                descriptions.push_back(nullFormString);
                continue;
            }
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = GetFormEditorId(nullptr, akForm, "");
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            descriptions.push_back(description);
        }
    }
    else if (noneStringType == 2) {
        for (auto* akForm : akForms) {
            if (!gfuncs::IsFormValid(akForm)) {
                descriptions.push_back(nullFormString);
                continue;
            }
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = gfuncs::IntToHex(akForm->GetFormID());
            }
            descriptions.push_back(description);
        }
    }
    else if (noneStringType == 1) {
        for (auto* akForm : akForms) {
            if (!gfuncs::IsFormValid(akForm)) {
                descriptions.push_back(nullFormString);
                continue;
            }
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = GetFormEditorId(nullptr, akForm, "");
            }
            descriptions.push_back(description);
        }
    }
    else if (maxCharacters > 0) {
        for (auto* akForm : akForms) {
            if (!gfuncs::IsFormValid(akForm)) {
                descriptions.push_back(nullFormString);
                continue;
            }
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            descriptions.push_back(description);
        }
    }
    else {
        for (auto* akForm : akForms) {
            if (!gfuncs::IsFormValid(akForm)) {
                descriptions.push_back(nullFormString);
                continue;
            }
            std::string description = GetDescription(akForm, newLineReplacer);
            descriptions.push_back(description);
        }
    }

    if (sortOption == 1 || sortOption == 2) {
        std::sort(descriptions.begin(), descriptions.end());
        if (sortOption == 2) {
            std::reverse(descriptions.begin(), descriptions.end());
        }
    }
    return descriptions;
}

std::vector<std::string> GetFormDescriptionsAsStrings(RE::BGSListForm* akFormlist, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    std::vector<std::string> descriptions;

    int m = akFormlist->forms.size();

    if (noneStringType == 2 && maxCharacters > 0) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            if (!gfuncs::IsFormValid(form)) {
                descriptions.push_back(nullFormString);
                return RE::BSContainer::ForEachResult::kContinue;
            }
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = gfuncs::IntToHex(form->GetFormID());
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 1 && maxCharacters > 0) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            if (!gfuncs::IsFormValid(form)) {
                descriptions.push_back(nullFormString);
                return RE::BSContainer::ForEachResult::kContinue;
            }
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = GetFormEditorId(nullptr, form, "");
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            descriptions.push_back(description);

            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 2) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            if (!gfuncs::IsFormValid(form)) {
                descriptions.push_back(nullFormString);
                return RE::BSContainer::ForEachResult::kContinue;
            }
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = gfuncs::IntToHex(form->GetFormID());
            }
            descriptions.push_back(description);

            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 1) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            if (!gfuncs::IsFormValid(form)) {
                descriptions.push_back(nullFormString);
                return RE::BSContainer::ForEachResult::kContinue;
            }
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = GetFormEditorId(nullptr, form, "");
            }
            descriptions.push_back(description);

            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (maxCharacters > 0) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            if (!gfuncs::IsFormValid(form)) {
                descriptions.push_back(nullFormString);
                return RE::BSContainer::ForEachResult::kContinue;
            }
            std::string description = GetDescription(form, newLineReplacer);
            if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            descriptions.push_back(description);

            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            if (!gfuncs::IsFormValid(form)) {
                descriptions.push_back(nullFormString);
                return RE::BSContainer::ForEachResult::kContinue;
            }
            std::string description = GetDescription(form, newLineReplacer);
            descriptions.push_back(description);

            return RE::BSContainer::ForEachResult::kContinue;
            });
    }

    if (sortOption == 1 || sortOption == 2) {
        std::sort(descriptions.begin(), descriptions.end());
        if (sortOption == 2) {
            std::reverse(descriptions.begin(), descriptions.end());
        }
    }
    return descriptions;
}

std::vector<std::string> getFormNamesAndDescriptionsAsStrings(std::vector<RE::TESForm*> akForms, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    std::vector<std::string> descriptions;

    if (noneStringType == 2 && maxCharacters > 0) {
        for (auto* akForm : akForms) {
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = gfuncs::IntToHex(akForm->GetFormID());
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
        }
    }
    else if (noneStringType == 1 && maxCharacters > 0) {
        for (auto* akForm : akForms) {
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = GetFormEditorId(nullptr, akForm, "");
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
        }
    }
    else if (noneStringType == 2) {
        for (auto* akForm : akForms) {
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = gfuncs::IntToHex(akForm->GetFormID());
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
        }
    }
    else if (noneStringType == 1) {
        for (auto* akForm : akForms) {
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = GetFormEditorId(nullptr, akForm, "");
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
        }
    }
    else if (maxCharacters > 0) {
        for (auto* akForm : akForms) {
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
        }
    }
    else {
        for (auto* akForm : akForms) {
            std::string description = GetDescription(akForm, newLineReplacer);
            std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
        }
    }

    if (sortOption == 3 || sortOption == 4) {
        std::sort(descriptions.begin(), descriptions.end());
        if (sortOption == 4) {
            std::reverse(descriptions.begin(), descriptions.end());
        }
    }
    return descriptions;
}

std::vector<std::string> getFormNamesAndDescriptionsAsStrings(RE::BGSListForm* akFormlist, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    std::vector<std::string> formNamesAndDescriptions;
    /*std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, "", "", false));
    formNamesAndDescriptions.push_back(name + "||" + description);*/
    int m = akFormlist->forms.size();

    std::vector<std::string> descriptions;
    if (noneStringType == 2 && maxCharacters > 0) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = gfuncs::IntToHex(form->GetFormID());
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 1 && maxCharacters > 0) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = GetFormEditorId(nullptr, form, "");
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 2) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = gfuncs::IntToHex(form->GetFormID());
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 1) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = GetFormEditorId(nullptr, form, "");
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (maxCharacters > 0) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string description = GetDescription(form, newLineReplacer);
            if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string description = GetDescription(form, newLineReplacer);
            std::string name = static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }

    if (sortOption == 3 || sortOption == 4) {
        std::sort(descriptions.begin(), descriptions.end());
        if (sortOption == 4) {
            std::reverse(descriptions.begin(), descriptions.end());
        }
    }
    return descriptions;
}

std::vector<std::string> GetFormNamesAsStrings(std::vector<RE::TESForm*> akForms, int sortOption, int noneStringType, std::string nullFormString) {
    std::vector<std::string> formNames;

    if (noneStringType == 2) {
        for (auto* akForm : akForms) {
            std::string noName = "";
            if (gfuncs::IsFormValid(akForm)) {
                noName = gfuncs::IntToHex(akForm->GetFormID());
            }
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, noName)));
        }
    }
    else if (noneStringType == 1) {
        for (auto* akForm : akForms) {
            std::string noName = "";
            if (gfuncs::IsFormValid(akForm)) {
                noName = GetFormEditorId(nullptr, akForm, "");
            }
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, noName)));
        }
    }
    else {
        for (auto* akForm : akForms) {
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "")));
        }
    }

    if (sortOption == 1 || sortOption == 2) {
        std::sort(formNames.begin(), formNames.end());
        if (sortOption == 2) {
            std::reverse(formNames.begin(), formNames.end());
        }
    }
    return formNames;
}

std::vector<std::string> GetFormNamesAsStrings(RE::BGSListForm* akFormlist, int sortOption, int noneStringType, std::string nullFormString) {
    std::vector<std::string> formNames;

    int m = akFormlist->forms.size();

    if (noneStringType == 2) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string noName = "";
            if (gfuncs::IsFormValid(form)) {
                noName = gfuncs::IntToHex(form->GetFormID());
            }
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, noName)));
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 1) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string noName = "";
            if (gfuncs::IsFormValid(form)) {
                noName = GetFormEditorId(nullptr, form, "");
            }
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, noName)));
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "")));
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }

    if (sortOption == 1 || sortOption == 2) {
        std::sort(formNames.begin(), formNames.end());
        if (sortOption == 2) {
            std::reverse(formNames.begin(), formNames.end());
        }
    }
    return formNames;
}

std::vector<std::string> GetFormEditorIdsAsStrings(std::vector<RE::TESForm*> akForms, int sortOption, std::string nullFormString) {
    std::vector<std::string> formNames;

    int m = akForms.size();

    for (int i = 0; i < m; i++) {
        RE::TESForm* akForm = akForms[i];
        std::string name = nullFormString;
        if (gfuncs::IsFormValid(akForm)) {
            name = GetFormEditorId(nullptr, akForm, "");
        }
        formNames.push_back(name);
    }

    if (sortOption == 1 || sortOption == 2) {
        std::sort(formNames.begin(), formNames.end());
        if (sortOption == 2) {
            std::reverse(formNames.begin(), formNames.end());
        }
    }
    return formNames;
}

std::vector<std::string> GetFormEditorIdsAsStrings(RE::BGSListForm* akFormlist, int sortOption, std::string nullFormString) {
    std::vector<std::string> formNames;

    //int m = akFormlist->forms.size();

    akFormlist->ForEachForm([&](auto& akForm) {
        auto* form = &akForm;
        std::string name = nullFormString;
        if (gfuncs::IsFormValid(form)) {
            name = GetFormEditorId(nullptr, form, "");
        }
        formNames.push_back(name);
        return RE::BSContainer::ForEachResult::kContinue;
        });

    if (sortOption == 1 || sortOption == 2) {
        std::sort(formNames.begin(), formNames.end());
        if (sortOption == 2) {
            std::reverse(formNames.begin(), formNames.end());
        }
    }
    return formNames;
}

std::vector<std::string> GetLoadedModNamesAsStrings(int sortOption) {
    std::vector<std::string> fileNames;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedModCount();
        for (int i = 0; i < modCount; i++) {
            auto* file = dataHandler->LookupLoadedModByIndex(i);
            if (file) {
                fileNames.push_back(static_cast<std::string>(file->GetFilename()));
            }
        }

        if (sortOption == 1 || sortOption == 2) {
            std::sort(fileNames.begin(), fileNames.end());
            if (sortOption == 2) {
                std::reverse(fileNames.begin(), fileNames.end());
            }
        }
    }
    return fileNames;
}

std::vector<std::string> GetLoadedLightModNamesAsStrings(int sortOption) {
    std::vector<std::string> fileNames;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedLightModCount();
        for (int i = 0; i < modCount; i++) {
            auto* file = dataHandler->LookupLoadedLightModByIndex(i);
            if (file) {
                fileNames.push_back(static_cast<std::string>(file->GetFilename()));
            }
        }

        if (sortOption == 1 || sortOption == 2) {
            std::sort(fileNames.begin(), fileNames.end());
            if (sortOption == 2) {
                std::reverse(fileNames.begin(), fileNames.end());
            }
        }
    }
    return fileNames;
}

std::vector<std::string> GetLoadedModDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<std::string> sfileDescriptions;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedModCount();

        if (newLineReplacer != "" && maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else if (maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else if (newLineReplacer != "") {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(description);
                }
            }
        }

        if (sortOption == 1 || sortOption == 2) {
            std::sort(sfileDescriptions.begin(), sfileDescriptions.end());
            if (sortOption == 2) {
                std::reverse(sfileDescriptions.begin(), sfileDescriptions.end());
            }
        }
    }
    return sfileDescriptions;
}

std::vector<std::string> GetLoadedModNamesAndDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<std::string> sfileDescriptions;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedModCount();

        if (newLineReplacer != "" && maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else if (maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else if (newLineReplacer != "") {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }

        if (sortOption == 3 || sortOption == 4) {
            std::sort(sfileDescriptions.begin(), sfileDescriptions.end());
            if (sortOption == 2) {
                std::reverse(sfileDescriptions.begin(), sfileDescriptions.end());
            }
        }
    }
    return sfileDescriptions;
}

std::vector<std::string> GetLoadedLightModDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<std::string> sfileDescriptions;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedLightModCount();

        if (newLineReplacer != "" && maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else if (maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else if (newLineReplacer != "") {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(description);
                }
            }
        }

        if (sortOption == 1 || sortOption == 2) {
            std::sort(sfileDescriptions.begin(), sfileDescriptions.end());
            if (sortOption == 2) {
                std::reverse(sfileDescriptions.begin(), sfileDescriptions.end());
            }
        }
    }
    return sfileDescriptions;
}

std::vector<std::string> GetLoadedLightModNamesAndDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<std::string> sfileDescriptions;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedLightModCount();

        if (newLineReplacer != "" && maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else if (maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else if (newLineReplacer != "") {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }

        if (sortOption == 1 || sortOption == 2) {
            std::sort(sfileDescriptions.begin(), sfileDescriptions.end());
            if (sortOption == 2) {
                std::reverse(sfileDescriptions.begin(), sfileDescriptions.end());
            }
        }
    }
    return sfileDescriptions;
}

std::vector<std::string> GetAllLoadedModDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<std::string> sfileDescriptions;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedModCount();
        int lightModCount = dataHandler->GetLoadedLightModCount();

        if (newLineReplacer != "" && maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else if (maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else if (newLineReplacer != "") {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(description);
                }
            }
        }

        if (sortOption == 1 || sortOption == 2) {
            std::sort(sfileDescriptions.begin(), sfileDescriptions.end());
            if (sortOption == 2) {
                std::reverse(sfileDescriptions.begin(), sfileDescriptions.end());
            }
        }
    }
    return sfileDescriptions;
}

std::vector<std::string> GetAllLoadedModNamesAndDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<std::string> sfileDescriptions;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedModCount();
        int lightModCount = dataHandler->GetLoadedLightModCount();

        //sFileNamesAndDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);

        if (newLineReplacer != "" && maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else if (maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else if (newLineReplacer != "") {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }

        if (sortOption == 3 || sortOption == 4) {
            std::sort(sfileDescriptions.begin(), sfileDescriptions.end());
            if (sortOption == 2) {
                std::reverse(sfileDescriptions.begin(), sfileDescriptions.end());
            }
        }
    }
    return sfileDescriptions;
}

std::string GetClipBoardText(RE::StaticFunctionTag*) {
    // Try opening the clipboard
    if (!OpenClipboard(nullptr)) {
        logger::error("Couldn't open clipboard");
        return "";
    }

    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) {
        logger::error("Clipboard data not found");
        CloseClipboard();
        return "";
    }

    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) {
        logger::error("Couldn't GlobalLock Clipboard Data");
        CloseClipboard();
        return "";
    }

    std::string text(pszText);

    // Release the lock
    GlobalUnlock(hData);

    // Release the clipboard
    CloseClipboard();

    return text;
}

bool SetClipBoardText(RE::StaticFunctionTag*, std::string sText) {
    if (sText.length() == 0) {
        return false;
    }

    const char* output = sText.data();
    const size_t len = strlen(output) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(hMem), output, len);
    GlobalUnlock(hMem);
    // Try opening the clipboard
    if (!OpenClipboard(nullptr)) {
        logger::error("Couldn't open clipboard");
        return false;
    }

    EmptyClipboard();
    auto Handle = SetClipboardData(CF_TEXT, hMem);

    if (Handle == NULL) {
        logger::error("Couldn't set clipboard data");
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
}

bool IsWhiteSpace(RE::StaticFunctionTag*, std::string s) {
    return isspace(int(s.at(0)));
}

int CountWhiteSpaces(RE::StaticFunctionTag*, std::string s) {
    int spaces = std::count_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
    return spaces;
}

bool ModHasFormType(RE::StaticFunctionTag*, std::string modName, int formType) {
    logger::debug("modName[{}] formType[{}]", modName, formType);

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {
        logger::error("couldn't get dataHandler");
        return false;
    }

    auto* modFile = dataHandler->LookupModByName(modName);
    if (!modFile) {
        logger::error("mod [{}] not loaded", modName);
        return false;
    }

    RE::BSTArray<RE::TESForm*>* formArray = &(dataHandler->GetFormArray(static_cast<RE::FormType>(formType)));

    int ic = 0;
    for (RE::BSTArray<RE::TESForm*>::iterator it = formArray->begin(); it != formArray->end() && ic < formArray->size(); it++, ic++) {
        RE::TESForm* akForm = *it;

        if (gfuncs::IsFormValid(akForm)) {
            if (modFile->IsFormInMod(akForm->GetFormID())) {
                return true;
            }
        }
    }
    return false;
}

std::string GetFormEditorId(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string nullFormString) {
    if (!gfuncs::IsFormValid(akForm)) {
        return nullFormString;
    }
    else {
        std::string editorId = akForm->GetFormEditorID();
        if (editorId == "") {
            editorId = clib_util::editorID::get_editorID(akForm);
        }
        return editorId;
    }
}

std::vector<RE::BSFixedString> GetFormEditorIds(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, int sortOption, std::string nullFormString) {
    std::vector<RE::BSFixedString> formNames;
    if (akForms.size() == 0) {
        logger::warn("akForms is size 0");
        return formNames;
    }

    std::vector<std::string> sFormNames = GetFormEditorIdsAsStrings(akForms, sortOption, nullFormString);
    std::copy(sFormNames.begin(), sFormNames.end(), std::back_inserter(formNames));

    return formNames;
}

std::vector<RE::BSFixedString> GetFormEditorIdsFromList(RE::StaticFunctionTag*, RE::BGSListForm* akFormlist, int sortOption, std::string nullFormString) {
    std::vector<RE::BSFixedString> formNames;
    if (!akFormlist) {
        logger::warn("akFormlist doesn't exist");
        return formNames;
    }

    std::vector<std::string> sFormNames = GetFormEditorIdsAsStrings(akFormlist, sortOption, nullFormString);
    std::copy(sFormNames.begin(), sFormNames.end(), std::back_inserter(formNames));

    return formNames;
}

std::vector<RE::TESForm*> SortFormArray(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, int sortOption) {
    if (akForms.size() == 0) {
        return akForms;
    }

    std::vector<RE::TESForm*> returnForms;

    if (sortOption == 1 || sortOption == 2) { //sort by form name
        std::vector<std::string> formNames;
        std::map<std::string, std::vector<RE::TESForm*>> formNamesMap;

        for (int i = 0; i < akForms.size(); i++) {
            auto* akForm = akForms[i];
            std::string formName = static_cast<std::string>(gfuncs::GetFormName(akForm, "", "", false));
            formNames.push_back(formName);
            auto it = formNamesMap.find(formName);
            if (it == formNamesMap.end()) {
                std::vector<RE::TESForm*> formNameForms;
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
                    returnForms.push_back(akForm);
                }
            }
        }
    }
    else if (sortOption == 3 || sortOption == 4) { //sort by editorId
        std::vector<std::string> formNames;
        std::map<std::string, std::vector<RE::TESForm*>> formNamesMap;

        for (int i = 0; i < akForms.size(); i++) {
            auto* akForm = akForms[i];
            std::string formName = "";
            if (gfuncs::IsFormValid(akForm)) {
                formName = GetFormEditorId(nullptr, akForm, "");
            }

            formNames.push_back(formName);
            auto it = formNamesMap.find(formName);
            if (it == formNamesMap.end()) {
                std::vector<RE::TESForm*> formNameForms;
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
                    returnForms.push_back(akForm);
                }
            }
        }
    }
    else { //sort by form id
        std::vector<int> formIds;
        std::map<int, std::vector<RE::TESForm*>> formIdsMap;

        for (int i = 0; i < akForms.size(); i++) {
            auto* akForm = akForms[i];
            int formID = akForm->GetFormID();
            formIds.push_back(formID);
            auto it = formIdsMap.find(formID);
            if (it == formIdsMap.end()) {
                std::vector<RE::TESForm*> formIdForms;
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
                    returnForms.push_back(akForm);
                }
            }
        }
    }

    return returnForms;
}

std::vector<RE::TESForm*> FormListToArray(RE::StaticFunctionTag*, RE::BGSListForm* akFormlist, int sortOption) {
    std::vector<RE::TESForm*> returnForms;
    if (!akFormlist) {
        logger::warn("akFormlist doesn't exist");
        return returnForms;
    }

    akFormlist->ForEachForm([&](auto& akForm) {
        auto* form = &akForm;
        returnForms.push_back(form);
        return RE::BSContainer::ForEachResult::kContinue;
        });

    if (sortOption >= 1 && sortOption <= 6) {
        return SortFormArray(nullptr, returnForms, sortOption);
    }
    else {
        return returnForms;
    }
}

void AddFormsToList(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, RE::BGSListForm* akFormlist) {
    if (!akFormlist) {
        logger::warn("akFormlist doesn't exist");
        return;
    }

    int m = akForms.size();

    if (m == 0) {
        logger::warn("akForms size is 0");
        return;
    }

    for (auto* akForm : akForms) {
        if (gfuncs::IsFormValid(akForm)) {
            akFormlist->AddForm(akForm);
        }
    }
}

std::string GetEffectsDescriptions(RE::BSTArray<RE::Effect*> effects) {
    std::string description = "";

    for (int i = 0; i < effects.size(); i++) {
        RE::Effect* effect = effects[i];
        if (effect) {
            std::string s = (static_cast<std::string>(effect->baseEffect->magicItemDescription));
            std::vector<std::string> values = {
                std::format("{:.0f}", effect->GetMagnitude()),
                std::format("{:.0f}", float(effect->GetDuration())),
                std::format("{:.0f}", float(effect->GetArea()))
            };
            gfuncs::String_ReplaceAll(s, magicDescriptionTags, values);
            description += (s + " ");
        }
    }
    return description;
}

std::string GetMagicItemDescription(RE::MagicItem* magicItem) {
    if (gfuncs::IsFormValid(magicItem)) {
        logger::debug("{}", magicItem->GetName());
        return GetEffectsDescriptions(magicItem->effects);
    }
    return "";
}

std::string GetDescription(RE::TESForm* akForm, std::string newLineReplacer) {
    if (!gfuncs::IsFormValid(akForm)) {
        logger::warn("akForm doesn't exist or isn't valid");
        return "";
    }

    RE::BSString descriptionString;
    std::string s = "";
    auto description = akForm->As<RE::TESDescription>();

    if (description == NULL) {
        logger::warn("couldn't cast form[{}] ID[{}] as description", gfuncs::GetFormName(akForm), akForm->GetFormID());
        // return "";
    }
    else {
        description->GetDescription(descriptionString, nullptr);
        s = static_cast<std::string>(descriptionString);
    }

    if (s == "") {
        RE::MagicItem* magicItem = akForm->As<RE::MagicItem>();
        s = GetMagicItemDescription(magicItem);
    }

    if (s == "") {
        RE::TESEnchantableForm* enchantForm = akForm->As<RE::TESEnchantableForm>();
        if (enchantForm) {
            RE::EnchantmentItem* enchantment = enchantForm->formEnchanting;
            if (gfuncs::IsFormValid(enchantment)) {
                RE::MagicItem* magicItem = enchantment->As<RE::MagicItem>();
                if (gfuncs::IsFormValid(magicItem)) {
                    s = GetMagicItemDescription(magicItem);
                }
            }
        }
    }

    if (newLineReplacer != "") {
        gfuncs::String_ReplaceAll(s, "\r", newLineReplacer);
        gfuncs::String_ReplaceAll(s, "\n", newLineReplacer);
    }
    return s;
}

std::string GetFormDescription(RE::StaticFunctionTag*, RE::TESForm* akForm, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    if (!gfuncs::IsFormValid(akForm)) {
        return nullFormString;
    }

    std::string s = GetDescription(akForm, newLineReplacer);

    if (s == "") {
        if (noneStringType == 2) {
            if (gfuncs::IsFormValid(akForm)) {
                s = gfuncs::IntToHex(akForm->GetFormID());
            }
        }
        else if (noneStringType == 1) {
            if (gfuncs::IsFormValid(akForm)) {
                s = GetFormEditorId(nullptr, akForm, "");
            }
        }
    }
    else if (maxCharacters > 0) {
        if (s.size() > maxCharacters) {
            s = s.substr(0, maxCharacters) + overMaxCharacterSuffix;
        }
    }
    return s;
}

std::vector<RE::BSFixedString> GetFormDescriptions(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    std::vector<RE::BSFixedString> descriptions;
    if (akForms.size() == 0) {
        logger::warn("akForms is size 0");
        return descriptions;
    }

    if (sortOption >= 3 && sortOption <= 8) {
        sortOption -= 2; // 1 & 2 sorts by description, else sort by name, editorID, or formID, which are between 1 and 6 in SortFormArray function.
        std::vector<RE::TESForm*> sortedForms = SortFormArray(nullptr, akForms, sortOption);
        std::vector<std::string> sDescriptions = GetFormDescriptionsAsStrings(sortedForms, 0, maxCharacters, overMaxCharacterSuffix, newLineReplacer, noneStringType, nullFormString);
        std::copy(sDescriptions.begin(), sDescriptions.end(), std::back_inserter(descriptions));
    }
    else {
        std::vector<std::string> sDescriptions = GetFormDescriptionsAsStrings(akForms, sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer, noneStringType, nullFormString);
        std::copy(sDescriptions.begin(), sDescriptions.end(), std::back_inserter(descriptions));
    }

    return descriptions;
}

std::vector<RE::BSFixedString> GetFormDescriptionsFromList(RE::StaticFunctionTag*, RE::BGSListForm* akFormlist, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    std::vector<RE::BSFixedString> descriptions;
    if (!akFormlist) {
        logger::warn("akFormlist doesn't exist");
        return descriptions;
    }

    if (sortOption >= 3 && sortOption <= 8) {
        sortOption -= 2; // 1 & 2 sorts by description, else sort by name, editorID, or formID, which are between 1 and 6 in SortFormArray function.
        std::vector<RE::TESForm*> sortedForms = FormListToArray(nullptr, akFormlist, sortOption);
        std::vector<std::string> sDescriptions = GetFormDescriptionsAsStrings(sortedForms, 0, maxCharacters, overMaxCharacterSuffix, newLineReplacer, noneStringType, nullFormString);
        std::copy(sDescriptions.begin(), sDescriptions.end(), std::back_inserter(descriptions));
    }
    else {
        std::vector<std::string> sDescriptions = GetFormDescriptionsAsStrings(akFormlist, sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer, noneStringType, nullFormString);
        std::copy(sDescriptions.begin(), sDescriptions.end(), std::back_inserter(descriptions));
    }

    return descriptions;
}

std::vector<RE::BSFixedString> GetFormNames(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, int sortOption, int noneStringType, std::string nullFormString) {
    std::vector<RE::BSFixedString> formNames;
    if (akForms.size() == 0) {
        logger::warn("akForms is size 0");
        return formNames;
    }

    std::vector<std::string> sFormNames = GetFormNamesAsStrings(akForms, sortOption, noneStringType, nullFormString);
    std::copy(sFormNames.begin(), sFormNames.end(), std::back_inserter(formNames));

    return formNames;
}

std::vector<RE::BSFixedString> GetFormNamesFromList(RE::StaticFunctionTag*, RE::BGSListForm* akFormlist, int sortOption, int noneStringType, std::string nullFormString) {
    std::vector<RE::BSFixedString> formNames;
    if (!akFormlist) {
        logger::warn("akFormlist doesn't exist");
        return formNames;
    }

    std::vector<std::string> sFormNames = GetFormNamesAsStrings(akFormlist, sortOption, noneStringType, nullFormString);
    std::copy(sFormNames.begin(), sFormNames.end(), std::back_inserter(formNames));

    return formNames;
}

std::vector<RE::BSFixedString> GetLoadedModNames(RE::StaticFunctionTag*, int sortOption) {
    std::vector<RE::BSFixedString> fileNames;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedModCount();

        if (sortOption == 1 || sortOption == 2) {
            std::vector<std::string> sfileNames = GetLoadedModNamesAsStrings(sortOption);
            std::copy(sfileNames.begin(), sfileNames.end(), std::back_inserter(fileNames));
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    fileNames.push_back(file->GetFilename());
                }
            }
        }
    }
    return fileNames;
}

std::vector<RE::BSFixedString> GetLoadedLightModNames(RE::StaticFunctionTag*, int sortOption) {
    std::vector<RE::BSFixedString> fileNames;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedLightModCount();

        if (sortOption == 1 || sortOption == 2) {
            std::vector<std::string> sfileNames = GetLoadedLightModNamesAsStrings(sortOption);
            std::copy(sfileNames.begin(), sfileNames.end(), std::back_inserter(fileNames));
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    fileNames.push_back(file->GetFilename());
                }
            }
        }
    }
    return fileNames;
}

std::vector<RE::BSFixedString> GetAllLoadedModNames(RE::StaticFunctionTag*, int sortOption) {
    std::vector<RE::BSFixedString> fileNames;

    if (sortOption == 1 || sortOption == 2) {
        std::vector<std::string> sfileNames = GetLoadedModNamesAsStrings(0);
        std::vector<std::string> slightfileNames = GetLoadedLightModNamesAsStrings(0);
        sfileNames.reserve(sfileNames.size() + slightfileNames.size());
        sfileNames.insert(sfileNames.end(), slightfileNames.begin(), slightfileNames.end());
        std::sort(sfileNames.begin(), sfileNames.end());
        if (sortOption == 2) {
            std::reverse(sfileNames.begin(), sfileNames.end());
        }
        std::copy(sfileNames.begin(), sfileNames.end(), std::back_inserter(fileNames));
    }
    else {
        fileNames = GetLoadedModNames(nullptr, sortOption);
        std::vector<RE::BSFixedString> lightFileNames = GetLoadedLightModNames(nullptr, sortOption);
        fileNames.reserve(fileNames.size() + lightFileNames.size());
        fileNames.insert(fileNames.end(), lightFileNames.begin(), lightFileNames.end());
    }

    return fileNames;
}

std::vector<RE::BSFixedString> GetLoadedModDescriptions(RE::StaticFunctionTag*, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<RE::BSFixedString> fileDescriptions;

    if (sortOption == 3 || sortOption == 4) {
        std::vector<std::string> sfileNamesAndDescriptions = GetLoadedModNamesAndDescriptionsAsStrings(sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer);
        for (int i = 0; i < sfileNamesAndDescriptions.size(); i++) {
            std::size_t delimIndex = sfileNamesAndDescriptions[i].find("||");
            if (delimIndex != std::string::npos) {
                delimIndex += 2;
                fileDescriptions.push_back(sfileNamesAndDescriptions[i].substr(delimIndex));
            }
        }
    }
    else {
        std::vector<std::string> sfileDescriptions = GetLoadedModDescriptionsAsStrings(sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer);
        std::copy(sfileDescriptions.begin(), sfileDescriptions.end(), std::back_inserter(fileDescriptions));
    }
    return fileDescriptions;
}

std::vector<RE::BSFixedString> GetLoadedLightModDescriptions(RE::StaticFunctionTag*, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<RE::BSFixedString> fileDescriptions;

    if (sortOption == 3 || sortOption == 4) {
        std::vector<std::string> sfileNamesAndDescriptions = GetLoadedLightModNamesAndDescriptionsAsStrings(sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer);
        for (int i = 0; i < sfileNamesAndDescriptions.size(); i++) {
            std::size_t delimIndex = sfileNamesAndDescriptions[i].find("||");
            if (delimIndex != std::string::npos) {
                delimIndex += 2;
                fileDescriptions.push_back(sfileNamesAndDescriptions[i].substr(delimIndex));
            }
        }
    }
    else {
        std::vector<std::string> sfileDescriptions = GetLoadedLightModDescriptionsAsStrings(sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer);
        std::copy(sfileDescriptions.begin(), sfileDescriptions.end(), std::back_inserter(fileDescriptions));
    }
    return fileDescriptions;
}

std::vector<RE::BSFixedString> GetAllLoadedModDescriptions(RE::StaticFunctionTag*, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<RE::BSFixedString> fileDescriptions;

    if (sortOption == 3 || sortOption == 4) {
        std::vector<std::string> sfileNamesAndDescriptions = GetAllLoadedModNamesAndDescriptionsAsStrings(sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer);
        for (int i = 0; i < sfileNamesAndDescriptions.size(); i++) {
            auto delimIndex = sfileNamesAndDescriptions[i].find("||");
            if (delimIndex != std::string::npos) {
                delimIndex += 2;
                fileDescriptions.push_back(sfileNamesAndDescriptions[i].substr(delimIndex));
            }
        }
    }
    else {
        std::vector<std::string> sfileDescriptions = GetAllLoadedModDescriptionsAsStrings(sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer);
        std::copy(sfileDescriptions.begin(), sfileDescriptions.end(), std::back_inserter(fileDescriptions));
    }
    return fileDescriptions;
}

bool FormNameMatches(RE::TESForm* akForm, std::string& sFormName) {
    bool nameMatches = false;
    if (gfuncs::IsFormValid(akForm)) {
        auto* ref = akForm->AsReference();
        if (gfuncs::IsFormValid(ref)) {
            nameMatches = (ref->GetDisplayFullName() == sFormName);
            if (!nameMatches) {
                RE::TESForm* baseForm = ref->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    nameMatches = (baseForm->GetName() == sFormName);
                }
            }
        }
        else {
            nameMatches = (akForm->GetName() == sFormName);
        }
    }
    return nameMatches;
}

bool FormNameContains(RE::TESForm* akForm, std::string& sFormName) {
    std::string akFormName = "";
    bool akFormNameContainsSFormName = false;
    if (gfuncs::IsFormValid(akForm)) {
        auto* ref = akForm->AsReference();
        if (gfuncs::IsFormValid(ref)) {
            akFormName = (ref->GetDisplayFullName());
            if (akFormName != "") {
                akFormNameContainsSFormName = (akFormName.find(sFormName) != std::string::npos);
            }
            if (!akFormNameContainsSFormName) {
                RE::TESForm* baseForm = ref->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    akFormName = baseForm->GetName();
                    akFormNameContainsSFormName = (akFormName.find(sFormName) != std::string::npos);
                }
            }
        }
        else {
            akFormName = akForm->GetName();
            akFormNameContainsSFormName = (akFormName.find(sFormName) != std::string::npos);
        }
    }
    return akFormNameContainsSFormName;
}
