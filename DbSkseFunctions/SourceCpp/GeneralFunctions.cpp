#include <Windows.h>
#include "GeneralFunctions.h"
//#include <spdlog/sinks/basic_file_sink.h>
//#include <stdarg.h>
//#include <winbase.h>
//#include <iostream>
//#include <stdio.h>
//#include <chrono> //*
//#include <ctime>
//#include <cstdlib>
//#include <algorithm>
//#include <string>
//#include "mini/ini.h"
#include "editorID.hpp"
//#include "STLThunk.h"

namespace logger = SKSE::log;

namespace gfuncs {
    RE::SkyrimVM* gsvm;
    RE::Actor* gfuncsPlayerRef;

    int GetRandomInt(int min, int max) {
        // return clib_util::RNG().generate<std::uint32_t>(a_min, a_max);

        return (min + (rand() % (max - min + 1)));
    }

    std::string IntToHex(int i) {
        std::stringstream stream;
        stream << "0x"
            << std::setfill('0') << std::setw(sizeof(int) * 2)
            << std::hex << i;
        return stream.str();
    }

    int HexToInt(std::string hex) {
        return stoi(hex, 0, 16);
    }

    uint64_t StringToUint64_t(std::string s) {
        uint64_t value;
        std::istringstream iss(s);
        iss >> value; 
        return value;
    }

    //get the int after start char in string s. Example: GetIntAfterCharInString("arrows (21)") returns 21
    int GetIntAfterCharInString(std::string s, char startChar, int iDefault, int startIndex) {
        int iStart = -1;
        int iEnd = -1;
        int L = s.length();
        int i = startIndex;
        while (i < L) {
            if (s.at(i) == startChar) {
                if (isdigit(s.at(i + 1))) {
                    iStart = i + 1;
                    i++;
                    break;
                }
            }
            i++;
        }

        if (iStart != -1) {
            while (i < L) {
                if (!isdigit(s.at(i))) {
                    iEnd = i;
                    break;
                }
                i++;
            }
            if (iEnd == -1) {
                iEnd = L;
            }
            return stoi(s.substr(iStart, (iEnd - iStart)));
        }
        return iDefault;
    }

    bool IsFormValid(RE::TESForm* form, bool checkDeleted) {
        if (!form) {
            return false;
        }

        if (IsBadReadPtr(form, sizeof(form))) {
            return false;
        }

        if (form->GetFormID() == 0) {
            return false;
        }

        if (checkDeleted) {
            if (form->IsDeleted()) {
                return false;
            }
        }

        return true;
    }

    std::string GetFormEditorId(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string nullFormString) {
        if (!IsFormValid(akForm)) {
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

    RE::BSFixedString GetFormName(RE::TESForm* akForm, RE::BSFixedString nullString, RE::BSFixedString noNameString, bool returnIdIfNull) {
        if (!IsFormValid(akForm)) {
            return nullString;
        }

        RE::TESObjectREFR* ref = akForm->As<RE::TESObjectREFR>();
        RE::BSFixedString name;
        if (IsFormValid(ref)) {
            name = ref->GetDisplayFullName();
            if (name == "") {
                auto* baseForm = ref->GetBaseObject();
                if (IsFormValid(baseForm)) {
                    name = baseForm->GetName();
                }
            }
        }
        else {
            name = akForm->GetName();
        }

        if (name == "") {
            if (returnIdIfNull) {
                name = GetFormEditorId(nullptr, akForm, "");
            }
            else {
                name = noNameString;
            }
        }
        return name;
    }

    std::string GetFormDataString(RE::TESForm* akForm, std::string nullString, std::string noNameString) {
        if (!IsFormValid(akForm)) {
            return nullString;
        }

        std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullString, noNameString, false));

        std::string editorID = GetFormEditorId(nullptr, akForm, "");

        std::string dataString = std::format("(name[{}] editorID[{}] formID[{:x}]) type[{}]", name, editorID, akForm->GetFormID(), akForm->GetFormType());

        return dataString;
    }

    RE::TESFile* GetFileForForm(RE::TESForm* akForm) {
        if (!gfuncs::IsFormValid(akForm)) {
            return nullptr;
        }

        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            return nullptr;
        }

        auto id = akForm->GetFormID();

        for (auto* file : dataHandler->files) {
            if (file) {
                if (file->IsFormInMod(id)) {
                    return file;
                }
            }
        }

        return nullptr;
    }

    void logFormMap(auto& map) {
        logger::trace("logging form map");
        for (auto const& x : map)
        {
            RE::TESForm* akForm = x.first;
            if (akForm) {
                logger::trace("Form[{}] ID[{:x}] value[{}]", gfuncs::GetFormName(akForm), akForm->GetFormID(), x.second);
            }
        }
    }

    float timePointDiffToFloat(std::chrono::system_clock::time_point end, std::chrono::system_clock::time_point start) {
        std::chrono::duration<float> timeElapsed = end - start;
        return timeElapsed.count();
    }

    RE::VMHandle GetHandle(RE::TESForm* akForm) {
        if (!(akForm)) {
            logger::warn("{}: akForm doesn't exist or isn't valid", __func__);
            return NULL;
        }

        RE::VMTypeID id = static_cast<RE::VMTypeID>(akForm->GetFormType());
        RE::VMHandle handle = gsvm->handlePolicy.GetHandleForObject(id, akForm);

        if (handle == NULL) {
            return NULL;
        }

        return handle;
    }

    RE::VMHandle GetHandle(RE::BGSBaseAlias* akAlias) {
        if (!akAlias) {
            logger::warn("{}: akAlias doesn't exist", __func__);
            return NULL;
        }

        RE::VMTypeID id = akAlias->GetVMTypeID();
        RE::VMHandle handle = gsvm->handlePolicy.GetHandleForObject(id, akAlias);

        if (handle == NULL) {
            return NULL;
        }
        return handle;
    }

    RE::VMHandle GetHandle(RE::ActiveEffect* akEffect) {
        if (!akEffect) {
            logger::warn("{}: akEffect doesn't exist", __func__);
            return NULL;
        }

        RE::VMTypeID id = akEffect->VMTYPEID;
        RE::VMHandle handle = gsvm->handlePolicy.GetHandleForObject(id, akEffect);

        if (handle == NULL) {
            return NULL;
        }
        return handle;
    }

    RE::TESObjectREFR* GetRefFromHandle(RE::RefHandle& handle) {
        if (handle) {
            auto niPtr = RE::TESObjectREFR::LookupByHandle(handle);
            if (niPtr) {
                return niPtr.get();
            }
        }
        return nullptr;
    }

    RE::Actor* GetPlayerDialogueTarget() {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (IsFormValid(form)) {
                RE::Actor* actor = form->As<RE::Actor>();
                if (IsFormValid(actor)) {
                    RE::ObjectRefHandle dialogueTargetHandle = actor->GetActorRuntimeData().dialogueItemTarget;
                    if (dialogueTargetHandle) {
                        auto ptr = dialogueTargetHandle.get();
                        if (ptr) {
                            auto* ref = ptr.get();
                            if (IsFormValid(ref)) {
                                RE::Actor* dialogueActorRef = ref->As<RE::Actor>();
                                if (IsFormValid(dialogueActorRef)) {
                                    if (dialogueActorRef == gfuncsPlayerRef) {
                                        return actor;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return nullptr;
    }

    void String_ReplaceAll(std::string& s, std::string searchString, std::string replaceString) {
        if (s == "" || searchString == "") {
            return;
        }

        int sSize = searchString.size();
        std::size_t index = s.find(searchString);

        while (index != std::string::npos)
        {
            s.replace(index, sSize, replaceString);
            index = s.find(searchString);
        }
    }

    void String_ReplaceAll(std::string& s, std::vector<std::string> searchStrings, std::vector<std::string> replaceStrings) {
        int m = searchStrings.size();
        if (replaceStrings.size() < m) {
            m = replaceStrings.size();
        }
        for (int i = 0; i < m; i++) {
            String_ReplaceAll(s, searchStrings[i], replaceStrings[i]);
        }
    }

    int GetIndexInVector(std::vector<RE::FormID>& v, RE::FormID& element) {
        if (element == NULL) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        int m = v.size();
        for (int i = 0; i < m; i++) {
            if (v[i] == element) {
                return i;
            }
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::TESForm*>& v, RE::TESForm* element) {
        if (!IsFormValid(element)) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        int m = v.size();
        for (int i = 0; i < m; i++) {
            if (v[i] == element) {
                return i;
            }
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::BGSProjectile*>& v, RE::BGSProjectile* element) {
        if (!IsFormValid(element)) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        int m = v.size();
        for (int i = 0; i < m; i++) {
            if (v[i] == element) {
                return i;
            }
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::BSSoundHandle*>& v, RE::BSSoundHandle* element) {
        if (!element) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        int m = v.size();
        for (int i = 0; i < m; i++) {
            if (v[i] == element) {
                return i;
            }
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::BSSoundHandle> v, RE::BSSoundHandle element) {
        if (v.size() == 0) {
            return -1;
        }

        int m = v.size();
        for (int i = 0; i < m; i++) {
            if (v[i].soundID == element.soundID) {
                return i;
            }
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::VMHandle> v, RE::VMHandle& element) {
        if (element == NULL) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        int m = v.size();
        for (int i = 0; i < m; i++) {
            if (v[i] == element) {
                return i;
            }
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::BSFixedString> v, RE::BSFixedString& element) {
        if (element == NULL) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        int m = v.size();
        for (int i = 0; i < m; i++) {
            if (v[i] == element) {
                return i;
            }
        }

        return -1;
    }

    int GetIndexInVector(std::vector<std::string> v, std::string& element) {
        if (element == "") {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        int m = v.size();
        for (int i = 0; i < m; i++) {
            if (v[i] == element) {
                return i;
            }
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::TESObjectREFR*> v, RE::TESObjectREFR* element) {
        if (!IsFormValid(element)) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        int m = v.size();
        for (int i = 0; i < m; i++) {
            if (v[i] == element) {
                return i;
            }
        }

        return -1;
    }

    bool IsObjectInBSTArray(RE::BSTArray<RE::ObjectRefHandle>* v, RE::ObjectRefHandle& element) {
        if (v->size() == 0) {
            return false;
        }

        for (auto& obj : *v) {
            if (obj) {
                if (obj == element) {
                    return true;
                }
            }
        }

        return false;
    }

    RE::NiAVObject* GetNiAVObjectForRef(RE::TESObjectREFR* ref) {
        if (!IsFormValid(ref)) {
            return nullptr;
        }

        RE::NiAVObject* obj = ref->Get3D();
        if (!obj) {
            obj = ref->Get3D1(false);

            if (!obj) {
                obj = ref->Get3D2();

                if (!obj) {
                    obj = ref->Get3D1(true);
                }
            }
        }

        return obj;
    }

    RE::BGSBaseAlias* GetQuestAliasById(RE::TESQuest* quest, int id) {
        if (gfuncs::IsFormValid(quest)) {
            if (quest->aliases.size() > 0) {
                for (int i = 0; i < quest->aliases.size(); i++) {
                    if (quest->aliases[i]) {
                        if (quest->aliases[i]->aliasID == id) {
                            return quest->aliases[i];
                        }
                    }
                }
            }
        }

        return nullptr;
    }

    int GetWeatherType(RE::TESWeather* weather) {
        if (gfuncs::IsFormValid(weather)) {
            const auto flags = weather->data.flags;
            if (flags.any(RE::TESWeather::WeatherDataFlag::kNone)) {
                return -1;
            }
            if (flags.any(RE::TESWeather::WeatherDataFlag::kPleasant)) {
                return 0;
            }
            if (flags.any(RE::TESWeather::WeatherDataFlag::kCloudy)) {
                return 1;
            }
            if (flags.any(RE::TESWeather::WeatherDataFlag::kRainy)) {
                return 2;
            }
            if (flags.any(RE::TESWeather::WeatherDataFlag::kSnow)) {
                return 3;
            }
        }
        return -2;
    }

    RE::BSFixedString GetBSUIMessageDataTypeString(RE::BSUIMessageData* msgData) {
        if (msgData) {
            RE::UserEvents* userEvents = RE::UserEvents::GetSingleton();

            if (userEvents) {
                if (msgData->fixedStr == userEvents->forward) {
                    return "forward";
                }
                else if (msgData->fixedStr == userEvents->back) {
                    return "back";
                }
                else if (msgData->fixedStr == userEvents->strafeLeft) {
                    return "strafeLeft";
                }
                else if (msgData->fixedStr == userEvents->strafeRight) {
                    return "strafeRight";
                }
                else if (msgData->fixedStr == userEvents->move) {
                    return "move";
                }
                else if (msgData->fixedStr == userEvents->look) {
                    return "look";
                }
                else if (msgData->fixedStr == userEvents->activate) {
                    return "activate";
                }
                else if (msgData->fixedStr == userEvents->leftAttack) {
                    return "leftAttack";
                }
                else if (msgData->fixedStr == userEvents->rightAttack) {
                    return "rightAttack";
                }
                else if (msgData->fixedStr == userEvents->dualAttack) {
                    return "dualAttack";
                }
                else if (msgData->fixedStr == userEvents->forceRelease) {
                    return "forceRelease";
                }
                else if (msgData->fixedStr == userEvents->pause) {
                    return "pause";
                }
                else if (msgData->fixedStr == userEvents->readyWeapon) {
                    return "readyWeapon";
                }
                else if (msgData->fixedStr == userEvents->togglePOV) {
                    return "togglePOV";
                }
                else if (msgData->fixedStr == userEvents->jump) {
                    return "jump";
                }
                else if (msgData->fixedStr == userEvents->journal) {
                    return "journal";
                }
                else if (msgData->fixedStr == userEvents->sprint) {
                    return "sprint";
                }
                else if (msgData->fixedStr == userEvents->sneak) {
                    return "sneak";
                }
                else if (msgData->fixedStr == userEvents->shout) {
                    return "shout";
                }
                else if (msgData->fixedStr == userEvents->kinectShout) {
                    return "kinectShout";
                }
                else if (msgData->fixedStr == userEvents->grab) {
                    return "grab";
                }
                else if (msgData->fixedStr == userEvents->run) {
                    return "run";
                }
                else if (msgData->fixedStr == userEvents->toggleRun) {
                    return "toggleRun";
                }
                else if (msgData->fixedStr == userEvents->autoMove) {
                    return "autoMove";
                }
                else if (msgData->fixedStr == userEvents->quicksave) {
                    return "quicksave";
                }
                else if (msgData->fixedStr == userEvents->quickload) {
                    return "quickload";
                }
                else if (msgData->fixedStr == userEvents->newSave) {
                    return "newSave";
                }
                else if (msgData->fixedStr == userEvents->inventory) {
                    return "inventory";
                }
                else if (msgData->fixedStr == userEvents->stats) {
                    return "stats";
                }
                else if (msgData->fixedStr == userEvents->map) {
                    return "map";
                }
                else if (msgData->fixedStr == userEvents->screenshot) {
                    return "screenshot";
                }
                else if (msgData->fixedStr == userEvents->multiScreenshot) {
                    return "multiScreenshot";
                }
                else if (msgData->fixedStr == userEvents->console) {
                    return "console";
                }
                else if (msgData->fixedStr == userEvents->cameraPath) {
                    return "cameraPath";
                }
                else if (msgData->fixedStr == userEvents->tweenMenu) {
                    return "tweenMenu";
                }
                else if (msgData->fixedStr == userEvents->takeAll) {
                    return "takeAll";
                }
                else if (msgData->fixedStr == userEvents->accept) {
                    return "accept";
                }
                else if (msgData->fixedStr == userEvents->cancel) {
                    return "cancel";
                }
                else if (msgData->fixedStr == userEvents->up) {
                    return "up";
                }
                else if (msgData->fixedStr == userEvents->down) {
                    return "down";
                }
                else if (msgData->fixedStr == userEvents->left) {
                    return "left";
                }
                else if (msgData->fixedStr == userEvents->right) {
                    return "right";
                }
                else if (msgData->fixedStr == userEvents->pageUp) {
                    return "pageUp";
                }
                else if (msgData->fixedStr == userEvents->pageDown) {
                    return "pageDown";
                }
                else if (msgData->fixedStr == userEvents->pick) {
                    return "pick";
                }
                else if (msgData->fixedStr == userEvents->pickNext) {
                    return "pickNext";
                }
                else if (msgData->fixedStr == userEvents->pickPrevious) {
                    return "pickPrevious";
                }
                else if (msgData->fixedStr == userEvents->cursor) {
                    return "cursor";
                }
                else if (msgData->fixedStr == userEvents->kinect) {
                    return "kinect";
                }
                else if (msgData->fixedStr == userEvents->sprintStart) {
                    return "sprintStart";
                }
                else if (msgData->fixedStr == userEvents->sprintStop) {
                    return "sprintStop";
                }
                else if (msgData->fixedStr == userEvents->sneakStart) {
                    return "sneakStart";
                }
                else if (msgData->fixedStr == userEvents->sneakStop) {
                    return "sneakStop";
                }
                else if (msgData->fixedStr == userEvents->blockStart) {
                    return "blockStart";
                }
                else if (msgData->fixedStr == userEvents->blockStop) {
                    return "blockStop";
                }
                else if (msgData->fixedStr == userEvents->blockBash) {
                    return "blockBash";
                }
                else if (msgData->fixedStr == userEvents->attackStart) {
                    return "attackStart";
                }
                else if (msgData->fixedStr == userEvents->attackPowerStart) {
                    return "attackPowerStart";
                }
                else if (msgData->fixedStr == userEvents->reverseDirection) {
                    return "reverseDirection";
                }
                else if (msgData->fixedStr == userEvents->unequip) {
                    return "unequip";
                }
                else if (msgData->fixedStr == userEvents->zoomIn) {
                    return "zoomIn";
                }
                else if (msgData->fixedStr == userEvents->zoomOut) {
                    return "zoomOut";
                }
                else if (msgData->fixedStr == userEvents->rotateItem) {
                    return "rotateItem";
                }
                else if (msgData->fixedStr == userEvents->leftStick) {
                    return "leftStick";
                }
                else if (msgData->fixedStr == userEvents->prevPage) {
                    return "prevPage";
                }
                else if (msgData->fixedStr == userEvents->nextPage) {
                    return "nextPage";
                }
                else if (msgData->fixedStr == userEvents->prevSubPage) {
                    return "prevSubPage";
                }
                else if (msgData->fixedStr == userEvents->nextSubPage) {
                    return "nextSubPage";
                }
                else if (msgData->fixedStr == userEvents->leftEquip) {
                    return "leftEquip";
                }
                else if (msgData->fixedStr == userEvents->rightEquip) {
                    return "rightEquip";
                }
                else if (msgData->fixedStr == userEvents->toggleFavorite) {
                    return "toggleFavorite";
                }
                else if (msgData->fixedStr == userEvents->favorites) {
                    return "favorites";
                }
                else if (msgData->fixedStr == userEvents->hotkey1) {
                    return "hotkey1";
                }
                else if (msgData->fixedStr == userEvents->hotkey2) {
                    return "hotkey2";
                }
                else if (msgData->fixedStr == userEvents->hotkey3) {
                    return "hotkey3";
                }
                else if (msgData->fixedStr == userEvents->hotkey4) {
                    return "hotkey4";
                }
                else if (msgData->fixedStr == userEvents->hotkey5) {
                    return "hotkey5";
                }
                else if (msgData->fixedStr == userEvents->hotkey6) {
                    return "hotkey6";
                }
                else if (msgData->fixedStr == userEvents->hotkey7) {
                    return "hotkey7";
                }
                else if (msgData->fixedStr == userEvents->hotkey8) {
                    return "hotkey8";
                }
                else if (msgData->fixedStr == userEvents->quickInventory) {
                    return "quickInventory";
                }
                else if (msgData->fixedStr == userEvents->quickMagic) {
                    return "quickMagic";
                }
                else if (msgData->fixedStr == userEvents->quickStats) {
                    return "quickStats";
                }
                else if (msgData->fixedStr == userEvents->quickMap) {
                    return "quickMap";
                }
                else if (msgData->fixedStr == userEvents->toggleCursor) {
                    return "toggleCursor";
                }
                else if (msgData->fixedStr == userEvents->wait) {
                    return "wait";
                }
                else if (msgData->fixedStr == userEvents->click) {
                    return "click";
                }
                else if (msgData->fixedStr == userEvents->mapLookMode) {
                    return "mapLookMode";
                }
                else if (msgData->fixedStr == userEvents->equip) {
                    return "equip";
                }
                else if (msgData->fixedStr == userEvents->dropItem) {
                    return "dropItem";
                }
                else if (msgData->fixedStr == userEvents->rotate) {
                    return "rotate";
                }
                else if (msgData->fixedStr == userEvents->nextFocus) {
                    return "nextFocus";
                }
                else if (msgData->fixedStr == userEvents->prevFocus) {
                    return "prevFocus";
                }
                else if (msgData->fixedStr == userEvents->setActiveQuest) {
                    return "setActiveQuest";
                }
                else if (msgData->fixedStr == userEvents->placePlayerMarker) {
                    return "placePlayerMarker";
                }
                else if (msgData->fixedStr == userEvents->xButton) {
                    return "xButton";
                }
                else if (msgData->fixedStr == userEvents->yButton) {
                    return "yButton";
                }
                else if (msgData->fixedStr == userEvents->chargeItem) {
                    return "chargeItem";
                }
                else if (msgData->fixedStr == userEvents->unk318) {
                    return "unk318";
                }
                else if (msgData->fixedStr == userEvents->playerPosition) {
                    return "playerPosition";
                }
                else if (msgData->fixedStr == userEvents->localMap) {
                    return "localMap";
                }
                else if (msgData->fixedStr == userEvents->localMapMoveMode) {
                    return "localMapMoveMode";
                }
                else if (msgData->fixedStr == userEvents->itemZoom) {
                    return "itemZoom";
                }
            }
        }
        return "Unrecognized";
    }

    void RemoveDuplicates(std::vector<RE::VMHandle>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }

    void RemoveDuplicates(std::vector<RE::TESForm*>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }

    void CombineEventHandles(std::vector<RE::VMHandle>& handles, RE::TESForm* akForm, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& formHandles) {
        if (formHandles.size() == 0) {
            return;
        }

        if (!IsFormValid(akForm)) {
            return;
        }

        auto it = formHandles.find(akForm);

        if (it != formHandles.end()) {
            logger::trace("{}: events for form: [{}] ID[{:x}] found", __func__, GetFormName(akForm), akForm->GetFormID());
            handles.reserve(handles.size() + it->second.size());
            handles.insert(handles.end(), it->second.begin(), it->second.end());
        }
        else {
            logger::trace("{}: events for form: [{}] ID[{:x}] not found", __func__, GetFormName(akForm), akForm->GetFormID());
        }
    }

    void SendEvents(std::vector<RE::VMHandle> handles, RE::BSFixedString& sEvent, RE::BSScript::IFunctionArguments* args) {
        int max = handles.size();

        if (max == 0) {
            return;
        }

        for (int i = 0; i < max; i++) {
            gsvm->SendAndRelayEvent(handles[i], &sEvent, args, nullptr);
        }

        delete args; //args is created using makeFunctionArguments. Delete as it's no longer needed.
    }

    void Install() {
        if (!gsvm) {
            gsvm = RE::SkyrimVM::GetSingleton();
            std::srand((unsigned)std::time(NULL));
        }
        if (!gfuncsPlayerRef) { gfuncsPlayerRef = RE::TESForm::LookupByID<RE::TESForm>(20)->As<RE::Actor>(); }
    }
}
