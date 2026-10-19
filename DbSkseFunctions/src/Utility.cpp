#include <Windows.h>
#include <mutex>
#include <string.h>
#include <unordered_map>
#include <detours.h>
#include "Utility.h"
#include "GeneralFunctions.h"
#include "RE/B/BGSBaseAlias.h"
#include "RE/B/BSCoreTypes.h"
#include "RE/T/TESObjectREFR.h"
#include "RE/T/TypeTraits.h"
#include "Serialization.h"
#include "editorID.hpp"
#include "SharedVariables.h"
#include "STLThunk.h"

enum logLevel { trace, debug, info, warn, error, critical };
enum debugLevel { notification, messageBox };

std::vector<std::string> magicDescriptionTags = { "<mag>", "<dur>", "<area>" };

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
    //std::lock_guard<std::recursive_mutex> lock(openedMenusMutex); 

	auto* ui = RE::UI::GetSingleton();
    if (ui) {
        //hud menu is always open, unless closed by another mod.
        if (ui->IsMenuOpen(RE::HUDMenu::MENU_NAME)) {
            return (ui->menuStack.size() > 1);
        }
        else {
            return (ui->menuStack.size() > 0);
        }
    }

    return false;
}

bool IsLocalMapMenuOpen(RE::StaticFunctionTag*) {
	auto* ui = RE::UI::GetSingleton();
    if (!ui) {
        return false;
    }
	
	if (!ui->IsMenuOpen(RE::MapMenu::MENU_NAME) ){
		return false;
	}
	
    RE::GPtr<RE::MapMenu> menu = ui->GetMenu<RE::MapMenu>();
    if (!menu) {
        return false;
    }
	
	RE::LocalMapMenu* localMapMenu = nullptr;
	
	if (REL::Module::IsVR()) {
		auto* data = menu->GetVRRuntimeData();
		if (data){
			localMapMenu = &data->localMapMenu; 
		}
	} 
	else { 
		auto* data = menu->GetRuntimeData();
		if (data){
			localMapMenu = &data->localMapMenu; 
		}
	}
	
	if (!localMapMenu){
		return false;
	}
	
	auto* runtimeData = &localMapMenu->GetRuntimeData();
	if (!runtimeData){
		return false;
	}
	
	// logger::trace("showingMap[{}]", runtimeData->showingMap);
	
	return runtimeData->showingMap;
}

std::string GetLastMenuOpened(RE::StaticFunctionTag*) {
    return sv::lastMenuOpened;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
            std::string noName = "";
            if (gfuncs::IsFormValid(form)) {
                noName = gfuncs::IntToHex(form->GetFormID());
            }
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, noName)));
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 1) {
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
            std::string noName = "";
            if (gfuncs::IsFormValid(form)) {
                noName = GetFormEditorId(nullptr, form, "");
            }
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, noName)));
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else {
        akFormlist->ForEachForm([&](auto* form) {
            //auto* form = &akForm;
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

    akFormlist->ForEachForm([&](auto* form) {
        //auto* form = &akForm;
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
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
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
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
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
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
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
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
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
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
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
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
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
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
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
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
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

std::mutex ObjectReferenceBaseScriptsMutex;
std::unordered_map<RE::FormID, std::vector<std::string>> ObjectReferenceBaseScripts;

struct ObjectRefScriptInitEventSink : public RE::BSTEventSink<RE::TESInitScriptEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESInitScriptEvent* event, RE::BSTEventSource<RE::TESInitScriptEvent>*/*source*/) {
        if (!event) {
            //logger::error("ObjectInitEvent doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            return RE::BSEventNotifyControl::kContinue;
        }
	
        if (event->objectInitialized) {
            RE::TESObjectREFR* loadedRef = event->objectInitialized.get();
            if (gfuncs::IsFormValid(loadedRef)) {
                RE::TESForm* baseObj = loadedRef->GetBaseObject();
				if (gfuncs::IsFormValid(baseObj)) {
					RE::FormID formId = baseObj->GetFormID(); 
					
					std::vector<std::string> scriptNames;
					
					ObjectReferenceBaseScriptsMutex.lock();
					auto it = ObjectReferenceBaseScripts.find(formId); 
					if (it != ObjectReferenceBaseScripts.end()){
						scriptNames = it->second; 
					}
					ObjectReferenceBaseScriptsMutex.unlock();  
					
					for (std::string& scriptName : scriptNames) {
						if (!gfuncs::IsScriptAttachedToForm(loadedRef, scriptName)){
							logger::debug("attaching script[{}] to objectRef[{}]", scriptName, gfuncs::GetFormNameAndId(loadedRef));
							gfuncs::AttachPapyrusScriptToForm(loadedRef, scriptName.c_str());
						}
					}
				}
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

ObjectRefScriptInitEventSink* objectRefScriptInitEventSink;

void AddObjectLoadedEventSink() {
	ObjectReferenceBaseScriptsMutex.lock();
    if (!objectRefScriptInitEventSink) { objectRefScriptInitEventSink = new ObjectRefScriptInitEventSink(); }
	
	auto* eventSourceholder = RE::ScriptEventSourceHolder::GetSingleton();
	if (eventSourceholder){
		if (!objectRefScriptInitEventSink->sinkAdded){
			objectRefScriptInitEventSink->sinkAdded = true;
			eventSourceholder->AddEventSink(objectRefScriptInitEventSink); 
		}
	}
	ObjectReferenceBaseScriptsMutex.unlock();
}

std::uint32_t utilRecord = 'UTr7';

bool SaveObjectReferenceBaseScripts(SKSE::SerializationInterface* ssi){
	// std::lock_guard<std::mutex> lock(ObjectReferenceBaseScriptsMutex);
	
	ObjectReferenceBaseScriptsMutex.lock();
	
	if (!ssi->OpenRecord(utilRecord, 1)) {
		logger::error("Failed to open record[{}]", 'UTr7');
		ObjectReferenceBaseScriptsMutex.unlock();
		return false;
	}

	const auto size = ObjectReferenceBaseScripts.size();
	logger::debug("record[{}] ObjectReferenceBaseScripts size[{}]", 'UTr7', size);
	
	if (!ssi->WriteRecordData(size)) {
		logger::error("record[{}] Failed to write size of ObjectReferenceBaseScripts!", 'UTr7');
		ObjectReferenceBaseScriptsMutex.unlock();
		return false;
	}
	
	for (auto& pair : ObjectReferenceBaseScripts){
		auto& v = pair.second;
		auto vSize = v.size(); 
		RE::FormID formId = pair.first; 
		
		if (!ssi->WriteRecordData(formId)) {
			logger::error("record[{}] Failed to write data for formID[{:08X}]", 'UTr7', formId);
			ObjectReferenceBaseScriptsMutex.unlock();
			return false;
		}
		
		if (!ssi->WriteRecordData(vSize)) {
			logger::error("record[{}] Failed to write vSize for formID[{:08X}]", 'UTr7', formId);
			ObjectReferenceBaseScriptsMutex.unlock();
			return false;
		} 
		
		for (std::string scriptName : v){
			if (!serialize::SaveString(utilRecord, ssi, scriptName)){
				logger::error("record[{}] Failed to write scriptName[{}]", 'UTr7', scriptName);
				ObjectReferenceBaseScriptsMutex.unlock();
				return false;
			}
		}
	}
	
	ObjectReferenceBaseScriptsMutex.unlock();
	return true;
}

bool LoadObjectReferenceBaseScripts(SKSE::SerializationInterface* ssi){
	// std::lock_guard<std::mutex> lock(ObjectReferenceBaseScriptsMutex);
	
	ObjectReferenceBaseScriptsMutex.lock();
	
	ObjectReferenceBaseScripts.clear(); 
	
	std::size_t size;

	if (!ssi->ReadRecordData(size)) {
		logger::error("record: [{}] Failed to load size of arr!", 'UTr7');
		ObjectReferenceBaseScriptsMutex.unlock();
		return false;
	}

	logger::debug("load arr size = {}", size);

	for (std::size_t i = 0; i < size; ++i) {
		RE::FormID formID;
		if (!ssi->ReadRecordData(formID)) {
			logger::error("{}: Failed to load formID!", i);
			ObjectReferenceBaseScriptsMutex.unlock();
			return false;
		}

		bool formIdResolved = ssi->ResolveFormID(formID, formID);
		if (!formIdResolved) {
			logger::warn("record[{}] {}: failed to resolve formID[{:x}]", 'UTr7', i, formID);
		}
		
		std::size_t vSize;
		if (!ssi->ReadRecordData(vSize)) {
			logger::error("record: [{}] Failed to load vSize!", 'UTr7');
			ObjectReferenceBaseScriptsMutex.unlock();
			return false;
		} 
		
		std::vector<std::string> v;
		
		for (std::size_t i2 = 0; i2 < vSize; ++i2) {
			bool loadStringSuccess = false;
			std::string scriptName = serialize::LoadString(utilRecord, ssi, loadStringSuccess);
			if (!loadStringSuccess){
				logger::error("record: [{}] Failed to load scriptName!", 'UTr7');
				ObjectReferenceBaseScriptsMutex.unlock();
				return false;
			} 
			v.push_back(scriptName);
			logger::debug("loaded script [{}] on formid[{:08X}]", scriptName, formID);
		}
		
		if (formIdResolved) {
			//init vector for formID
			ObjectReferenceBaseScripts[formID] = v;
		} 
	}
	
	size = ObjectReferenceBaseScripts.size();
	
	ObjectReferenceBaseScriptsMutex.unlock();
	
	logger::debug("new ObjectReferenceBaseScripts size is [{}]", size);
	if (size > 0){ 
		AddObjectLoadedEventSink();
	}
	
	return true;
}

void AttachObjectReferenceScriptToBaseForm(RE::StaticFunctionTag*, std::string scriptName, RE::TESForm* akForm, bool attachToCurrentRefs, bool attachToNewRefs){
	if (!gfuncs::IsFormValid(akForm)){
		logger::warn("akForm doesn't exist");
		return;
	}
	
	if (!gfuncs::ScriptHasParent(scriptName, "ObjectReference")){
		logger::error("scriptName [{}] does not extend ObjectReference, aborting AttachObjectReferenceScriptToBaseForm", scriptName);
		return; //scriptName does not extend ObjectReference
	}
		
	RE::TESObjectREFR* ref = skyrim_cast<RE::TESObjectREFR*>(akForm);
	if (gfuncs::IsFormValid(ref)){
		logger::info("akForm is an objectReference, attaching the scriptName[{}] directly to the ref.", scriptName);
		gfuncs::AttachPapyrusScriptToForm(akForm, scriptName.c_str());
		return;
	}
	
	RE::FormID formId = akForm->GetFormID();
	
	if (attachToCurrentRefs){
		std::vector<RE::TESObjectREFR*> refsToUnattach;
		
		const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		RE::BSReadLockGuard readLock(lock); 
		
		for (auto& [id, form] : *allForms) {
			auto* akRef = form ? form->AsReference() : nullptr;
			if (akRef) {
				RE::TESForm* baseForm = akRef->GetBaseObject(); 
				if (baseForm == akForm){
					refsToUnattach.push_back(akRef);
				}
			}
		}
		
		for (auto* akRef : refsToUnattach){
			if (!gfuncs::IsScriptAttachedToForm(akRef, scriptName)){
				gfuncs::AttachPapyrusScriptToForm(akRef, scriptName.c_str());
			}
		}
	}
	
	if (attachToNewRefs){
		ObjectReferenceBaseScriptsMutex.lock();
		auto it = ObjectReferenceBaseScripts.find(formId);
		std::vector<std::string> v;
		if (it == ObjectReferenceBaseScripts.end()){
			v.push_back(scriptName);
			ObjectReferenceBaseScripts[formId] = v;
		}
		else {
			std::vector<std::string>& vv = it->second;
			if (std::find(vv.begin(), vv.end(), scriptName) == vv.end()){
				vv.push_back(scriptName);
			}
		}
		
		ObjectReferenceBaseScriptsMutex.unlock();
		AddObjectLoadedEventSink();
	}
}

bool StopAttachingObjectReferenceScriptsToNewRefs(RE::StaticFunctionTag*, std::string scriptName, RE::TESForm* akForm){
	if (!gfuncs::IsFormValid(akForm)){
		logger::warn("akForm doesn't exist");
		return false;
	}
	
	ObjectReferenceBaseScriptsMutex.lock();
	auto it = ObjectReferenceBaseScripts.find(akForm->GetFormID());
	if (it != ObjectReferenceBaseScripts.end()){
		auto itt = (std::find(it->second.begin(), it->second.end(), scriptName));
		if (itt != it->second.end()){
			it->second.erase((itt));
		}
	}
	ObjectReferenceBaseScriptsMutex.unlock();
}

bool AttachScriptToForm(RE::StaticFunctionTag*, std::string scriptName, RE::TESForm* akForm, bool allowDuplicates){
	if (!allowDuplicates){
		if (gfuncs::IsScriptAttachedToForm(akForm, scriptName)){
			return false;
		}
	}
	
	return gfuncs::AttachPapyrusScriptToForm(akForm, scriptName.c_str()) != nullptr;
}

bool AttachScriptToAlias(RE::StaticFunctionTag*, std::string scriptName, RE::BGSBaseAlias* akAlias, bool allowDuplicates){
	if (!allowDuplicates){
		if (gfuncs::IsScriptAttachedToAlias(akAlias, scriptName)){
			return false;
		}
	}
	
	return gfuncs::AttachPapyrusScriptToAlias(akAlias, scriptName.c_str()) != nullptr;
}

std::vector<std::string> GetAllScriptsAttachedToForm(RE::StaticFunctionTag*, RE::TESForm* akForm, bool RemoveDuplicates){
	std::vector<std::string> v;
	if (!gfuncs::IsFormValid(akForm)){
		logger::warn("akForm doesn't exist");
		return v;
	}
	
	RE::VMHandle handle = gfuncs::GetHandle(akForm);
	v = gfuncs::GetScriptsAttachedTohandle(handle, RemoveDuplicates);
	
	ObjectReferenceBaseScriptsMutex.lock();
	
	auto it = ObjectReferenceBaseScripts.find(akForm->GetFormID());
	if (it != ObjectReferenceBaseScripts.end()){
		for (std::string scriptName : it->second){
			v.push_back(scriptName);
		}
	}
	
	ObjectReferenceBaseScriptsMutex.unlock();
	
	if (RemoveDuplicates){
		gfuncs::RemoveDuplicates(v);
	}
	
	return v;
}

//E.G using MyMiscScript, returns ["MyMiscScript", "MiscObject", "Form"], if MyMiscScript is attached to akForm.
//Note that swapping out sScriptName for any of the scriptnames in the hierarchy returns the same thing.
std::vector<std::string> GetScriptHierarchyOnForm(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string sScriptName){
	std::vector<std::string> v;
	if (!gfuncs::IsFormValid(akForm)){
		logger::warn("akForm doesn't exist");
		return v;
	}
	
	RE::VMHandle handle = gfuncs::GetHandle(akForm);
	v = gfuncs::GetScriptHierarchyForScriptOnHandle(handle, sScriptName);
	
	//hierarchy not found
	if (v.size() == 0){
		std::vector<std::string> scriptsToCheck; 
		
		ObjectReferenceBaseScriptsMutex.lock();
		
		auto it = ObjectReferenceBaseScripts.find(akForm->GetFormID());
		if (it != ObjectReferenceBaseScripts.end()){
			scriptsToCheck = it->second;
		}
		
		ObjectReferenceBaseScriptsMutex.unlock();
		
		auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (vm){
			auto scriptChar = sScriptName.c_str();
			
			for (auto& scriptName : scriptsToCheck){
				RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
				if (!vm->CreateObject2(scriptName, object) || !object) { 
					continue;
				}
				
				auto* script = object.get();
				if (script) {
					auto info = script->GetTypeInfo();
					if (info) {
						bool scriptNameFound = false;
						//logger::error("info->name = [{}]", info->name);
						v.push_back((std::string)info->name);
						if (_stricmp(scriptChar, info->name.c_str()) == 0){
							scriptNameFound = true;
						}
						
						auto* parentInfo = info->GetParent();
						while (parentInfo){
							v.push_back((std::string)parentInfo->name);
							if (_stricmp(scriptChar, parentInfo->name.c_str()) == 0){
								scriptNameFound = true;
							}
							parentInfo = parentInfo->GetParent();
						}
						if (scriptNameFound){
							return v;
						} 
						else {
							v.clear();
						}
					}
				}
			}
		}
	}
	
	return v;
}

//returns true if the sScriptName is attached to the form and the sParentScriptName exists in the script hierarchy,
//and is higher in the hierarchy than the sScriptName. 
//E.G,  ["MyMiscScript", "MiscObject", "Form"], sScriptName = "MyMiscScript", sParentScriptName = "MiscObject" returns true. 
// switch sScriptName to "MiscObject" and sParentScriptName to "MyMiscScript", returns false.
bool ScriptHasParentOnForm(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string sScriptName, std::string sParentScriptName){
	std::vector<std::string> v = GetScriptHierarchyOnForm(nullptr, akForm, sScriptName);
	auto scriptChar = sScriptName.c_str(); 
	auto parentScriptChar = sParentScriptName.c_str();
	int scriptIndex = -1; 
	int parentScriptIndex = -1; 
	for (int i = 0; i < v.size(); i++){
		std::string script = v[i]; 
		auto c = script.c_str(); 
		if (_stricmp(c, scriptChar) == 0){
			scriptIndex = i;
		}
		else if (_stricmp(c, parentScriptChar) == 0){
			parentScriptIndex = i;
		}
	} 
	
	return (scriptIndex != -1 && parentScriptIndex != -1 && parentScriptIndex > scriptIndex);
}

//returns true if the sScriptName is attached to the form and the sChildScriptName exists in the script hierarchy,
//and is lower in the hierarchy than the sScriptName. 
//E.G,  ["MyMiscScript", "MiscObject", "Form"], sScriptName = "MiscObject", sChildScriptName = "MyMiscScript" returns true. 
// switch sScriptName to "MyMiscScript" and sChildScriptName to "MiscObject", returns false.
bool ScriptHasChildOnForm(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string sScriptName, std::string sChildScriptName){
	std::vector<std::string> v = GetScriptHierarchyOnForm(nullptr, akForm, sScriptName);
	auto scriptChar = sScriptName.c_str(); 
	auto childScriptChar = sChildScriptName.c_str();
	int scriptIndex = -1; 
	int childScriptIndex = -1; 
	for (int i = 0; i < v.size(); i++){
		std::string script = v[i]; 
		auto c = script.c_str(); 
		if (_stricmp(c, scriptChar) == 0){
			scriptIndex = i;
		}
		else if (_stricmp(c, childScriptChar) == 0){
			childScriptIndex = i;
		}
	} 
	
	return (scriptIndex != -1 && childScriptIndex != -1 && childScriptIndex > scriptIndex);
}

bool IsScriptAttachedToForm(RE::StaticFunctionTag*, std::string scriptName, RE::TESForm* akForm){
	bool attached = gfuncs::IsScriptAttachedToForm(akForm, scriptName);
	if (!attached){
		if (gfuncs::IsFormValid(akForm)){
			RE::FormID formID = akForm->GetFormID();
			ObjectReferenceBaseScriptsMutex.lock();
			auto it = ObjectReferenceBaseScripts.find(formID);
			if (it != ObjectReferenceBaseScripts.end()){
				attached = std::find(it->second.begin(), it->second.end(), scriptName) != it->second.end();
			}
			ObjectReferenceBaseScriptsMutex.unlock();
		}
	}
	return attached;
}

std::vector<std::string> GetAllScriptsAttachedToAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* akAlias, bool RemoveDuplicates){
	if (!akAlias){
		std::vector<std::string> v;
		logger::warn("akAlias doesn't exist");
		return v;
	}
	
	RE::VMHandle handle = gfuncs::GetHandle(akAlias);
	return gfuncs::GetScriptsAttachedTohandle(handle, RemoveDuplicates);
}

//E.G using MyAliasReferenceAliasScript, returns ["MyAliasReferenceAliasScript", "ReferenceAlias", "Alias"], if MyAliasReferenceAliasScript is attached to akAlias.
//Note that swapping out sScriptName for any of the scriptnames in the hierarchy returns the same thing.
std::vector<std::string> GetScriptHierarchyOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* akAlias, std::string sScriptName){
	std::vector<std::string> v;
	if (!(akAlias)){
		logger::warn("akForm doesn't exist");
		return v;
	}
	
	RE::VMHandle handle = gfuncs::GetHandle(akAlias);
	return gfuncs::GetScriptHierarchyForScriptOnHandle(handle, sScriptName);
}

bool ScriptHasParentOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* akAlias, std::string sScriptName, std::string sParentScriptName){
	if (!(akAlias)){
		logger::warn("akForm doesn't exist");
		return false;
	}
	
	RE::VMHandle handle = gfuncs::GetHandle(akAlias);
	return gfuncs::ScriptHasParentOnHandle(handle, sScriptName, sParentScriptName);
}

bool ScriptHasChildOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* akAlias, std::string sScriptName, std::string sChildScriptName){
	if (!(akAlias)){
		logger::warn("akForm doesn't exist");
		return false;
	}
	
	RE::VMHandle handle = gfuncs::GetHandle(akAlias);
	return gfuncs::ScriptHasChildOnHandle(handle, sScriptName, sChildScriptName);
}

bool IsScriptAttachedToAlias(RE::StaticFunctionTag*, std::string scriptName, RE::BGSBaseAlias* akAlias){
	return gfuncs::IsScriptAttachedToAlias(akAlias, scriptName);
}

std::string GetThisScriptName(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::StaticFunctionTag* functionTag) {
	return gfuncs::GetCallingScriptName(vm, stackID, "DbSkseFunctions");
} 

std::string GetThisFunctionName(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::StaticFunctionTag* functionTag) {
	return gfuncs::GetCallingScriptFunction(vm, stackID, "DbSkseFunctions");
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

std::string RemoveWhiteSpaces(RE::StaticFunctionTag*, std::string s){
	return gfuncs::RemoveWhiteSpace(s);
}

bool ModHasFormType(RE::StaticFunctionTag*, std::string modName, int formType) {
    logger::debug("modName[{}] formType[{}]", modName, formType);

	auto* dataHandler = RE::TESDataHandler::GetSingleton();
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
			int formID = gfuncs::IsFormValid(akForm, false, false) ? akForm->GetFormID() : 0;
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
    if (!gfuncs::IsFormValid(akFormlist)) {
        logger::warn("akFormlist doesn't exist");
        return returnForms;
    }

    akFormlist->ForEachForm([&](RE::TESForm* form) {
		if (gfuncs::IsFormValid(form, false, false)){
			returnForms.push_back(form);
		}
        
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

namespace descriptionHook {

	namespace {
		//TESDescription::GetDescription is NOT virtual -- the vtable ends at CopyComponent
		//(slot 03) -- so it cannot be vtable hooked. But because it is non virtual there is
		//exactly one copy of it in the exe and every form type's description goes through
		//it, so this single entry hook covers Books, Weapons, Perks and the rest at once.
		//
		//SKSE's Trampoline::write_branch is NOT usable here: it is a one way redirect that
		//returns a stub pointing back at our own thunk, so calling through it recurses until
		//the stack dies. MinHook builds a real detour holding the displaced instructions,
		//so originalFn is a genuine chain.
		using GetDescription_t = void (*)(RE::TESDescription*, RE::BSString&, RE::TESForm*, std::uint32_t);

		GetDescription_t originalFn = nullptr;

		std::unordered_map<RE::TESDescription*, RE::FormID> descOwners;
		std::unordered_map<RE::FormID, std::string>         descOverrides;
		std::shared_mutex                                   descMutex;

		//The game asks for descriptions while building inventory and magic menu lists, so
		//the common case (nothing overridden) must not touch the mutex at all.
		std::atomic<bool> hasAnyOverride{ false };
		std::atomic<bool> installed{ false };

		std::once_flag installOnce;

		void DetourGetDescription(RE::TESDescription* self, RE::BSString& out,
		                          RE::TESForm* parent, std::uint32_t fieldType)
		{
			//let the game fill `out` first, then replace it -- conservative, and it keeps
			//whatever side effects GetDescription has. If you would rather skip the file
			//read when an override exists, move this below the lookup and only call it
			//when haveReplacement is false.
			if (originalFn) {
				originalFn(self, out, parent, fieldType);
			}

			if (!hasAnyOverride.load(std::memory_order_acquire) || !self) {
				return;
			}

			std::string replacement;
			bool haveReplacement = false;

			{
				std::shared_lock lock(descMutex);

				auto owner = descOwners.find(self);
				if (owner != descOwners.end()) {
					auto ovr = descOverrides.find(owner->second);
					if (ovr != descOverrides.end()) {
						//copy out so the game's BSString allocator never runs under our lock
						replacement = ovr->second;
						haveReplacement = true;
					}
				}
			}

			if (haveReplacement) {
				out = replacement.c_str();
			}
		}
	}

	bool IsInstalled() {
		return installed.load(std::memory_order_acquire);
	}

	void Install() {
		std::call_once(installOnce, [](){
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(14399, 14552) };

			//Detours differs from MinHook here: originalFn must already point at the TARGET
			//before DetourAttach. On a successful commit Detours rewrites it to point at the
			//trampoline it built, so afterwards calling originalFn runs the displaced
			//instructions and continues into the real function.
			originalFn = reinterpret_cast<GetDescription_t>(target.address());

			if (DetourTransactionBegin() != NO_ERROR) {
				logger::error("DetourTransactionBegin failed");
				originalFn = nullptr;
				return;
			}

			//Detours fixes up the instruction pointer of the threads you enumerate. We
			//install at kDataLoaded, before anything has queried a description, so the
			//main thread is the only one that could be inside the target.
			if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR) {
				logger::error("DetourUpdateThread failed");
				DetourTransactionAbort();
				originalFn = nullptr;
				return;
			}

			LONG error = DetourAttach(
				reinterpret_cast<PVOID*>(&originalFn),
				reinterpret_cast<PVOID>(&DetourGetDescription));

			if (error != NO_ERROR) {
				logger::error("DetourAttach failed for TESDescription::GetDescription, error[{}]", error);
				DetourTransactionAbort();
				originalFn = nullptr;
				return;
			}

			error = DetourTransactionCommit();
			if (error != NO_ERROR) {
				logger::error("DetourTransactionCommit failed, error[{}]", error);
				originalFn = nullptr;
				return;
			}

			installed.store(true, std::memory_order_release);
			logger::info("TESDescription::GetDescription hooked at [{:X}]", target.address());
		});
	}

	bool SetOverride(RE::TESForm* akForm, std::string desc) {
		if (!IsInstalled()) {
			logger::error("Description hook is not installed, cannot set a description. Set bHookFormDescriptionEnabled to 1 in Data/Skse/Plugins/DbSkseFunctions.ini");
			return false;
		}

		if (!gfuncs::IsFormValid(akForm)) {
			return false;
		}

		auto* description = akForm->As<RE::TESDescription>();
		if (!description) {
			logger::debug("no TESDescription component on [{}]", gfuncs::GetFormNameAndId(akForm));
			return false;
		}

		const RE::FormID formId = akForm->GetFormID();

		{
			std::unique_lock lock(descMutex);
			descOwners[description] = formId;
			descOverrides[formId]   = std::move(desc);
		}

		hasAnyOverride.store(true, std::memory_order_release);
		return true;
	}

	bool ClearOverride(RE::TESForm* akForm) {
		if (!gfuncs::IsFormValid(akForm)) {
			return false;
		}

		const RE::FormID formId = akForm->GetFormID();

		bool erased = false;
		{
			std::unique_lock lock(descMutex);

			erased = (descOverrides.erase(formId) > 0);

			if (auto* description = akForm->As<RE::TESDescription>()) {
				descOwners.erase(description);
			}

			if (descOverrides.empty()) {
				hasAnyOverride.store(false, std::memory_order_release);
			}
		}

		return erased;
	}

	void ClearOverrideById(RE::FormID formId) {
		std::unique_lock lock(descMutex);

		descOverrides.erase(formId);

		//the form may already be destroyed, so sweep descOwners by value rather than
		//looking the component up again
		for (auto it = descOwners.begin(); it != descOwners.end(); ) {
			it = (it->second == formId) ? descOwners.erase(it) : std::next(it);
		}

		if (descOverrides.empty()) {
			hasAnyOverride.store(false, std::memory_order_release);
		}
	}

	std::string GetOverride(RE::TESForm* akForm) {
		if (!gfuncs::IsFormValid(akForm)) {
			return "";
		}

		std::shared_lock lock(descMutex);

		auto it = descOverrides.find(akForm->GetFormID());
		return (it != descOverrides.end()) ? it->second : "";
	}
} 

bool SetFormDescription(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string desc) {
	logger::debug("desc[{}]", desc);
	return descriptionHook::SetOverride(akForm, std::move(desc));
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
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
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
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
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

bool FormNameMatches(RE::TESForm* akForm, std::string& sFormName, bool checkIdentity) {
    bool nameMatches = false;
    if (gfuncs::IsFormValid(akForm, false, checkIdentity)) {
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

bool FormNameContains(RE::TESForm* akForm, std::string& sFormName, bool checkIdentity) {
    std::string akFormName = "";
    bool akFormNameContainsSFormName = false;
    if (gfuncs::IsFormValid(akForm, false, checkIdentity)) {
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
