#include <Windows.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <stdarg.h>
#include <winbase.h>
#include <iostream>

namespace logger = SKSE::log;
bool bPlayerIsInCombat = false;
bool bRegisteredForPlayerCombatChange = false;
RE::PlayerCharacter* player;
RE::Actor* playerRef;
RE::BSScript::IVirtualMachine* gvm;
RE::SkyrimVM* svm;
RE::ScriptEventSourceHolder* eventSourceholder;

//forward dec
void AddSink(int index);
void RemoveSink(int index);

//general functions============================================================================================================================================================

//call the function after delay (milliseconds)
void DelayedFunction(auto function, int delay) {
    std::thread t([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        function();
        });
    t.detach();
}

RE::BSFixedString GetFormName(RE::TESForm* akForm) {
    if (!akForm) {
        return "null";
    }

    RE::TESObjectREFR* ref = akForm->As<RE::TESObjectREFR>();
    if (ref) {
        return ref->GetDisplayFullName();
    }
    else {
        return akForm->GetName();
    }
}

int GetIndexInVector(std::vector<RE::FormID>& v, RE::FormID element) {
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

int GetIndexInVector(std::vector<RE::VMHandle> v, RE::VMHandle element) {
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

int GetIndexInVector(std::vector<RE::TESObjectREFR*> v, RE::TESObjectREFR* element) {
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

void RemoveDuplicates(std::vector<RE::VMHandle>& vec)
{
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::warn);
    spdlog::flush_on(spdlog::level::trace);
}

enum logLevel { trace, debug, info, warn, error, critical };
enum debugLevel { notification, messageBox };

void LogAndMessage(std::string message, int logLevel = info, int debugLevel = notification) {
    switch (logLevel) {
    case trace:
        logger::trace("{}", message);
        break;

    case debug:
        logger::debug("{}", message);
        break;

    case info:
        logger::info("{}", message);
        break;

    case warn:
        logger::warn("{}", message);
        break;

    case error:
        logger::error("{}", message);
        break;

    case critical:
        logger::critical("{}", message);
        break;
    }

    /*switch (debugLevel) {
    case notification:
        RE::DebugNotification(message.data());
        break;
    case messageBox:
        RE::DebugMessageBox(message.data());
        break;
    }*/
}

//serialization============================================================================================================================================================

RE::TESForm* LoadForm(SKSE::SerializationInterface* a_intfc) {
    RE::FormID formID;
    if (!a_intfc->ReadRecordData(formID)) {
        logger::error("{}: Failed to load formID!", __func__);
        return nullptr;
    }

    if (!a_intfc->ResolveFormID(formID, formID)) {
        logger::warn("{}: warning, failed to resolve formID[{:x}]", __func__, formID);
    }

    RE::TESForm* akForm = RE::TESForm::LookupByID(formID);

    if (!akForm) {
        logger::error("{} failed to load", __func__);
        return nullptr;
    }

    return akForm;
}

bool LoadFormIDVector(std::vector<RE::FormID>& arr, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    arr.clear();
    std::size_t size;

    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record: [{}] Failed to load size of arr!", __func__, record);
        return false;
    }

    logger::info("{}: load arr size = {}", __func__, size);

    for (std::size_t i = 0; i < size; ++i) {
        RE::FormID formID;
        if (!a_intfc->ReadRecordData(formID)) {
            logger::error("{}: {}: Failed to load formID!", __func__, i);
            return false;
        }

        if (!a_intfc->ResolveFormID(formID, formID)) {
            logger::warn("{}: {}: warning, failed to resolve formID[{:x}]", __func__, i, formID);
            continue;
        }

        arr.push_back(formID);
    }
    return true;
}

bool SaveFormIDVector(std::vector<RE::FormID>& arr, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    const std::size_t size = arr.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: record[{}] Failed to write size of arr!", __func__, record);
        return false;
    }

    for (const auto& formID : arr) {
        if (!a_intfc->WriteRecordData(formID)) {
            logger::error("{}: record[{}] Failed to write data for handle[{}]", __func__, record, formID);
            return false;
        }
    }

    return true;
}

bool LoadHandlesVector(std::vector<RE::VMHandle>& arr, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    arr.clear();
    std::size_t size;

    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record: [{}] Failed to load size of arr!", __func__, record);
        return false;
    }

    logger::info("{}: load arr size = {}", __func__, size);

    for (std::size_t i = 0; i < size; ++i) {
        RE::VMHandle handle;
        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: {}: Failed to load handle!", __func__, i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: {}: warning, failed to resolve handle[{}]", __func__, i, handle);
            continue;
        }

        arr.push_back(handle);
    }
    return true;
}

bool SaveHandlesVector(std::vector<RE::VMHandle>& arr, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    const std::size_t size = arr.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: record[{}] Failed to write size of arr!", __func__, record);
        return false;
    }

    for (const auto& handle : arr) {
        if (!a_intfc->WriteRecordData(handle)) {
            logger::error("{}: record[{}] Failed to write data for handle[{}]", __func__, record, handle);
            return false;
        }
    }

    return true;
}

bool LoadFormHandlesMap(std::map<RE::TESForm*, std::vector<RE::VMHandle>>& akMap, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    akMap.clear();

    std::size_t size;

    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record[{}] Failed to load size of akMap!", __func__, record);
        return false;
    }

    logger::info("{}: load akMap size = {}", __func__, size);

    for (std::size_t i = 0; i < size; ++i) { //size = number of pairs in map 

        RE::FormID formID;

        if (!a_intfc->ReadRecordData(formID)) {
            logger::error("{}: {}: Failed to load formID!", __func__, i);
            return false;
        }

        bool formIdResolved = a_intfc->ResolveFormID(formID, formID);

        if (!formIdResolved) {
            logger::warn("{}: {}: Failed to resolve formID {:x}", __func__, i, formID);
            return false;
        }

        //logger::info("{}: {}: formID[{:x}] loaded and resolved", __func__, i, formID);

        RE::TESForm* akForm;
        if (formIdResolved) {
            akForm = RE::TESForm::LookupByID<RE::TESForm>(formID);
            if (!akForm) {
                logger::error("{}: {}: error, failed to load akForm!", __func__, i);
                //return false;
            }
            else {
                logger::info("{}: {}: akForm[{}] loaded", __func__, i, formID);
            }
        }

        std::size_t handlesSize;

        if (!a_intfc->ReadRecordData(handlesSize)) {
            logger::error("{}: {}: Failed to load handlesSize!", __func__, i);
            return false;
        }

        logger::error("{}: {}: handlesSize loaded. Size[{}]", __func__, i, handlesSize);

        std::vector<RE::VMHandle> handles;

        for (std::size_t ib = 0; ib < handlesSize; ib++) {
            RE::VMHandle handle;

            if (!a_intfc->ReadRecordData(handle)) {
                logger::error("{}: {}: Failed to load handle", __func__, ib);
                return false;
            }

            if (!a_intfc->ResolveHandle(handle, handle)) {
                logger::warn("{}: {}: Failed to resolve handle {}", __func__, ib, handle);
                return false;
            }
            else {
                handles.push_back(handle);
            }
        }

        if (handles.size() > 0 && akForm != nullptr) {
            akMap.insert(std::pair<RE::TESForm*, std::vector<RE::VMHandle>>(akForm, handles));
            logger::info("{}: {}: record[{}] akForm[{}] formID[{:x}] loaded", __func__, i, record, GetFormName(akForm), formID);
        }
    }
    return true;
}

bool SaveFormHandlesMap(std::map<RE::TESForm*, std::vector<RE::VMHandle>>& akMap, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    const std::size_t  size = akMap.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: Failed to write size of akMap!", __func__);
        return false;
    }
    else {

        for (auto it : akMap)
        {
            auto formID = it.first->GetFormID();

            logger::info("{}: saving handles for ref[{}] formId[{:x}]", __func__, GetFormName(it.first), formID);

            if (!a_intfc->WriteRecordData(formID)) {
                logger::error("{}: Failed to write formID[{:x}]", __func__, formID);
                return false;
            }

            logger::info("{}: formID[{:x}] written successfully", __func__, formID);

            const std::size_t handlesSize = it.second.size();

            if (!a_intfc->WriteRecordData(handlesSize)) {
                logger::error("{} it.second (handles)", __func__);
                return false;
            }

            for (const auto& handle : it.second) {
                if (!a_intfc->WriteRecordData(handle)) {
                    logger::error("{}: Failed to write data for handle[{}]", __func__, handle);
                    return false;
                }
            }
        }
    }

    return true;
}

//papyrus functions=============================================================================================================================
float GetThisVersion(/*RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, */ RE::StaticFunctionTag* functionTag) {
    return float(4.9);
}

std::string GetClipBoardText(RE::StaticFunctionTag*) {
    // Try opening the clipboard
    if (!OpenClipboard(nullptr)) {
        LogAndMessage("Couldn't open clipboard", error);
        return "";
    }

    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) {
        LogAndMessage("Clipboard data not found", error);
        CloseClipboard();
        return "";
    }

    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) {
        LogAndMessage("Couldn't GlobalLock Clipboard Data", error);
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
        LogAndMessage("Couldn't open clipboard", error);
        return false;
    }

    EmptyClipboard();
    auto Handle = SetClipboardData(CF_TEXT, hMem);

    if (Handle == NULL) {
        LogAndMessage("Couldn't set clipboard data", error);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    delete output;
    return true;
}

bool IsWhiteSpace(RE::StaticFunctionTag*, std::string s) {
    return isspace(int(s.at(0)));
}

int CountWhiteSpaces(RE::StaticFunctionTag*, std::string s) {
    int spaces = std::count_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
    return spaces;
}

bool IsMapMarker(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker) {
    LogAndMessage("IsMapMarker function called");
    if (!mapMarker) {
        LogAndMessage("IsMapMarker: mapMarker doesn't exist");
        return false;
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        LogAndMessage("map marker list not found.");
        return false;
    }

    if (!mapMarkerData->mapData) {
        LogAndMessage("mapData not found.");
        return false;
    }

    return true;
}

bool SetMapMarkerName(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker, std::string name) {
    LogAndMessage("Renaming map marker");
    if (!mapMarker) {
        LogAndMessage("SetMapMarkerName: mapMarker doesn't exist", error);
        return false;
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        LogAndMessage("Warning, map marker list not found.", warn);
        return false;
    }

    if (!mapMarkerData->mapData) {
        LogAndMessage("Warning, mapData not found.", warn);
        return false;
    }

    if (mapMarkerData->mapData->locationName.fullName == NULL) {
        LogAndMessage("Warning, LocationName not found.", warn);
        return false;
    }

    const char* cName = name.data();
    mapMarkerData->mapData->locationName.fullName = cName;
    LogAndMessage(std::format("New map marker name = {}", mapMarkerData->mapData->locationName.GetFullName()));

    return true;
}

std::string GetMapMarkerName(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker) {
    LogAndMessage("Getting Marker Name");
    if (!mapMarker) {
        LogAndMessage("GetMapMarkerName: mapMarker doesn't exist", error);
        return "";
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        LogAndMessage("GetMapMarkerName Warning, map marker list not found.", warn);
        return "";
    }

    if (!mapMarkerData->mapData) {
        LogAndMessage("GetMapMarkerName Warning, mapData not found.", warn);
        return "";
    }

    if (mapMarkerData->mapData->locationName.fullName == NULL) {
        LogAndMessage("GetMapMarkerName Warning, LocationName not found.", warn);
        return "";
    }

    return std::string(mapMarkerData->mapData->locationName.GetFullName());
}

bool SetMapMarkerIconType(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker, int iconType) {
    if (!mapMarker) {
        LogAndMessage("SetMapMarkerIconType: mapMarker doesn't exist", error);
        return false;
    }

    LogAndMessage(std::format("Setting Map Marker Type to {}", iconType));

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        LogAndMessage("SetMapMarkerIconType Warning, map marker extra data list not found.", warn);
        return false;
    }

    if (!mapMarkerData->mapData) {
        LogAndMessage("SetMapMarkerIconType Warning, mapData not found.", warn);
        return false;
    }

    mapMarkerData->mapData->type = static_cast<RE::MARKER_TYPE>(iconType);

    return true;
}

int GetMapMarkerIconType(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker) {
    LogAndMessage("Getting Map Marker Type");
    if (!mapMarker) {
        LogAndMessage("GetMapMarkerIconType: mapMarker doesn't exist", error);
        return false;
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        LogAndMessage("GetMapMarkerIconType Warning, map marker list not found.", warn);
        return false;
    }

    if (!mapMarkerData->mapData) {
        LogAndMessage("GetMapMarkerIconType Warning, mapData not found.", warn);
        return false;
    }

    return static_cast<int>(mapMarkerData->mapData->type.get());
}

//RE::ExtraMapMarker newMapMarker;
bool CreateMapMarker(RE::StaticFunctionTag*, RE::TESObjectREFR* objRef, std::string name, int iconType) {
    logger::info("Creating Map Marker");
    // function to create copied from BSExtraData

    if (!objRef) {
        logger::error("{}: error, objRef doesn't exists", __func__);
        return false;
    }

    if (IsMapMarker(nullptr, objRef)) {
        return false;
    }
    else {
        RE::ExtraMapMarker* newMapMarker = RE::BSExtraData::Create<RE::ExtraMapMarker>();

        if (!newMapMarker) {
            logger::error("{}: error, newMapMarker doesn't exists", __func__);
            return false;
        }

        logger::info("{}: error, newMapMarker doesn't exists", __func__);

        if (!newMapMarker->mapData) {
            logger::error("{}: error, mapData doesn't exists", __func__);
            return false;
        }

        if (newMapMarker->mapData->locationName.fullName == NULL) {
            LogAndMessage("Warning, LocationName not found.", warn);
            //RE::MapMarkerData data;
            //RE::MapMarkerData* newData = &data; /*= RE::IFormFactory::GetConcreteFormFactoryByType<RE::MapMarkerData>()->Create();*/

            //newMapMarker->mapData = newData;
            return false;
        }

        const char* cName = name.data();
        newMapMarker->mapData->locationName.fullName = cName;
        newMapMarker->mapData->type = static_cast<RE::MARKER_TYPE>(iconType);

        std::string name = newMapMarker->mapData->locationName.GetFullName(); 
        int type = static_cast<int>(newMapMarker->mapData->type.get());

        logger::info("New map marker name = {} iconType = {}", name, type);
        objRef->extraList.Add(newMapMarker);
    }
    return false;
}


//edited form ConsoleUtil NG
static inline void ExecuteConsoleCommand(RE::StaticFunctionTag*, std::string a_command, RE::TESObjectREFR* objRef) {
    LogAndMessage(std::format("{} called. Command = {}", __func__, a_command));

    const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
    const auto script = scriptFactory ? scriptFactory->Create() : nullptr;
    if (script) {
        // const auto selectedRef = RE::Console::GetSelectedRef();
        script->SetCommand(a_command);

        if (objRef) {
            script->CompileAndRun(objRef);
        }
        else {
            script->CompileAndRun(RE::Console::GetSelectedRef().get());
        }
        delete script;
    }
}

bool HasCollision(RE::StaticFunctionTag*, RE::TESObjectREFR* objRef) {
    LogAndMessage(std::format("{} called", __func__));

    if (!objRef) {
        LogAndMessage(std::format("{}: objRef doesn't exist", __func__), warn);
        return false;
    }
    return objRef->HasCollision();
}

//function copied from More Informative Console
RE::BGSMusicType* GetCurrentMusicType(RE::StaticFunctionTag*)
{
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    RE::BGSMusicType* currentPriorityType = nullptr;

    if (dataHandler)
    {
        RE::BSTArray<RE::TESForm*>* musicTypeArray = &(dataHandler->GetFormArray(RE::FormType::MusicType));

        RE::BSTArray<RE::TESForm*>::iterator itrEndType = musicTypeArray->end();

        RE::BSIMusicTrack* currentPriorityTrack = nullptr;
        std::int8_t currentPriority = 127;

        //loop through all music types to check which one is running and what the current track is for said type
        for (RE::BSTArray<RE::TESForm*>::iterator itr = musicTypeArray->begin(); itr != itrEndType; itr++)
        {
            RE::TESForm* baseForm = *itr;

            if (baseForm)
            {
                RE::BGSMusicType* musicType = static_cast<RE::BGSMusicType*>(baseForm);
                RE::BSIMusicType::MUSIC_STATUS musicStatus = musicType->typeStatus.get();

                if (musicStatus == RE::BSIMusicType::MUSIC_STATUS::kPlaying)
                {
                    uint32_t currentTrackIndex = musicType->currentTrackIndex;

                    if (musicType->tracks.size() > currentTrackIndex)
                    {
                        RE::BSIMusicTrack* currentTrack = musicType->tracks[currentTrackIndex];

                        //if the track takes priority of the current priority track we found
                        if (currentTrack && currentPriority > musicType->priority) {
                            currentPriorityTrack = currentTrack;
                            currentPriorityType = musicType;
                            currentPriority = musicType->priority;
                        }
                    }
                }

            }
        }
    }
    return currentPriorityType;
}

int GetNumberOfTracksInMusicType(RE::StaticFunctionTag*, RE::BGSMusicType* musicType) {
    return musicType->tracks.size();
}

int GetMusicTypeTrackIndex(RE::StaticFunctionTag*, RE::BGSMusicType* musicType) {
    return musicType->currentTrackIndex;
}

void SetMusicTypeTrackIndex(RE::StaticFunctionTag*, RE::BGSMusicType* musicType, int index) {
    if (index >= musicType->tracks.size()) {
        index = musicType->tracks.size() - 1;
    }
    else if (index < 0) {
        index = 0;
    }

    if (musicType == GetCurrentMusicType(nullptr)) {
        musicType->tracks[GetMusicTypeTrackIndex(nullptr, musicType)]->DoFinish(false, 3.0);
        musicType->currentTrackIndex = index;
        musicType->tracks[index]->DoPlay();
    }
    else {
        musicType->currentTrackIndex = index;
    }
}

int GetMusicTypePriority(RE::StaticFunctionTag*, RE::BGSMusicType* musicType) {
    return musicType->priority;
}

void SetMusicTypePriority(RE::StaticFunctionTag*, RE::BGSMusicType* musicType, int priority) {
    musicType->priority = priority;
}

int GetMusicTypeStatus(RE::StaticFunctionTag*, RE::BGSMusicType* musicType) {
    return musicType->typeStatus.underlying();
}

std::vector<RE::EnchantmentItem*> GetKnownEnchantments(RE::StaticFunctionTag*) {
    std::vector<RE::EnchantmentItem*> returnValues;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    RE::BSTArray<RE::TESForm*>* enchantmentArray = &(dataHandler->GetFormArray(RE::FormType::Enchantment));
    RE::BSTArray<RE::TESForm*>::iterator itrEndType = enchantmentArray->end();

    logger::info("{} enchantmentArray size[{}]", __func__, enchantmentArray->size());

    for (RE::BSTArray<RE::TESForm*>::iterator it = enchantmentArray->begin(); it != itrEndType; it++) {
        RE::TESForm* baseForm = *it;
        
        if (baseForm) {
            RE::EnchantmentItem* enchantment = baseForm->As<RE::EnchantmentItem>();
            if (enchantment) {
                RE::EnchantmentItem* baseEnchantment = enchantment->data.baseEnchantment;
                if (baseEnchantment) {
                    if (baseEnchantment->GetKnown()) {
                        if (std::find(returnValues.begin(), returnValues.end(), baseEnchantment) == returnValues.end()) {
                            // baseEnchantment not in returnValues, add it
                            returnValues.push_back(baseEnchantment);
                        }
                    }
                }
            }
        }
    }
    return returnValues;
}

void AddKnownEnchantmentsToFormList(RE::StaticFunctionTag*, RE::BGSListForm* akList) {
    if (!akList) {
        logger::error("{} akList doesn't exist", __func__);
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    RE::BSTArray<RE::TESForm*>* enchantmentArray = &(dataHandler->GetFormArray(RE::FormType::Enchantment));
    RE::BSTArray<RE::TESForm*>::iterator itrEndType = enchantmentArray->end();

    logger::info("{} enchantmentArray size[{}]", __func__, enchantmentArray->size());

    for (RE::BSTArray<RE::TESForm*>::iterator it = enchantmentArray->begin(); it != itrEndType; it++) {
        RE::TESForm* baseForm = *it;

        if (baseForm) {
            RE::EnchantmentItem* enchantment = baseForm->As<RE::EnchantmentItem>();
            if (enchantment) {
                RE::EnchantmentItem* baseEnchantment = enchantment->data.baseEnchantment;
                if (baseEnchantment) {
                    if (baseEnchantment->GetKnown()) {
                        //if (!akList->HasForm(baseEnchantment)) {
                            akList->AddForm(baseEnchantment);
                        //}
                    }
                }
            }
        }
    }
}

RE::TESCondition* condition_isPowerAttacking;
bool IsActorPowerAttacking(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }

    if (!condition_isPowerAttacking) {
        logger::info("creating condition_isPowerAttacking condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsPowerAttacking;

        condition_isPowerAttacking = new RE::TESCondition;
        condition_isPowerAttacking->head = conditionItem;
    }

    return condition_isPowerAttacking->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsAttacking;
bool IsActorAttacking(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }

    if (!condition_IsAttacking) {
        logger::info("creating condition_IsAttacking condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsAttacking;
        
        condition_IsAttacking = new RE::TESCondition;
        condition_IsAttacking->head = conditionItem;
    }

    return condition_IsAttacking->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_isActorSpeaking;
bool IsActorSpeaking(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }

    if (!condition_isActorSpeaking) {
        logger::info("creating condition_isActorSpeaking condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsTalking;

        condition_isActorSpeaking = new RE::TESCondition;
        condition_isActorSpeaking->head = conditionItem;
    }

    return condition_isActorSpeaking->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsBlocking;
bool IsActorBlocking(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }

    if (!condition_IsBlocking) {
        logger::info("creating condition_IsBlocking condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsBlocking;

        condition_IsBlocking = new RE::TESCondition;
        condition_IsBlocking->head = conditionItem;
    }

    return condition_IsBlocking->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsCasting;
bool IsActorCasting(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }

    if (!condition_IsCasting) {
        logger::info("creating condition_IsCasting condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsCasting;

        condition_IsCasting = new RE::TESCondition;
        condition_IsCasting->head = conditionItem;
    }

    return condition_IsCasting->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsDualCasting;
bool IsActorDualCasting(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }

    if (!condition_IsDualCasting) {
        logger::info("creating condition_IsDualCasting condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsDualCasting;

        condition_IsDualCasting = new RE::TESCondition;
        condition_IsDualCasting->head = conditionItem;
    }

    return condition_IsDualCasting->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsStaggered;
bool IsActorStaggered(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }

    if (!condition_IsStaggered) {
        logger::info("creating condition_IsStaggered condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsStaggered;

        condition_IsStaggered = new RE::TESCondition;
        condition_IsStaggered->head = conditionItem;
    }

    return condition_IsStaggered->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsRecoiling;
bool IsActorRecoiling(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }

    if (!condition_IsRecoiling) {
        logger::info("creating condition_IsRecoiling condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsRecoiling;

        condition_IsRecoiling = new RE::TESCondition;
        condition_IsRecoiling->head = conditionItem;
    }

    return condition_IsRecoiling->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsIgnoringCombat;
bool IsActorIgnoringCombat(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }

    if (!condition_IsIgnoringCombat) {
        logger::info("creating condition_IsIgnoringCombat condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsIgnoringCombat;

        condition_IsIgnoringCombat = new RE::TESCondition;
        condition_IsIgnoringCombat->head = conditionItem;
    }

    return condition_IsIgnoringCombat->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsUndead;
bool IsActorUndead(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }

    if (!condition_IsUndead) {
        logger::info("creating condition_IsUndead condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsUndead;

        condition_IsUndead = new RE::TESCondition;
        condition_IsUndead->head = conditionItem;
    }

    return condition_IsUndead->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsOnFlyingMount;
bool IsActorOnFlyingMount(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }

    if (!condition_IsOnFlyingMount) {
        logger::info("creating condition_IsOnFlyingMount condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsOnFlyingMount;

        condition_IsOnFlyingMount = new RE::TESCondition;
        condition_IsOnFlyingMount->head = conditionItem;
    }

    return condition_IsOnFlyingMount->IsTrue(akActor, nullptr);
}

bool IsActorAMount(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }
    return akActor->IsAMount();
}

bool IsActorInMidAir(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }
    return akActor->IsInMidair();
}

bool IsActorInRagdollState(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return false;
    }
    return akActor->IsInRagdollState();
}

int GetDetectionLevel(RE::StaticFunctionTag*, RE::Actor* akActor, RE::Actor* akTarget) {
    if (!akActor) {
        logger::error("{}: error, akActor doesn't exist", __func__);
        return -1;
    }

    if (!akTarget) {
        logger::error("{}: error, akTarget doesn't exist", __func__);
        return -1;
    }
    return akActor->RequestDetectionLevel(akTarget);
}

std::string GetKeywordString(RE::StaticFunctionTag*, RE::BGSKeyword* akKeyword) {
    logger::info("{}", __func__);
    if (!akKeyword) {
        logger::warn("{} akKeyword doesn't exist", __func__);
        return "";
    }
    return std::string(akKeyword->GetFormEditorID());
}

void SetKeywordString(RE::StaticFunctionTag*, RE::BGSKeyword* akKeyword, std::string keywordString) {
    logger::info("{} {}", __func__, keywordString);

    //if (!savedFormIDs) { savedFormIDs = new SavedFormIDs(); }

    if (!akKeyword) {
        logger::warn("{} akKeyword doesn't exist", __func__);
        return;
    }
    akKeyword->SetFormEditorID(keywordString.c_str());
}

RE::BGSKeyword* CreateKeyword(RE::StaticFunctionTag*) {
    logger::info("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>()->Create();
    if (!newForm) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::info("{} success, dynamic form[{}]", __func__, newForm->IsDynamicForm());
    }

    return newForm;
}

RE::BGSListForm* CreateFormList(RE::StaticFunctionTag*, RE::BGSListForm* formListFiller) {
    logger::info("{} called", __func__);

    //RE::BGS
    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSListForm>()->Create();
    if (!newForm) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::info("{} success", __func__);
        if (formListFiller) {
            logger::info("{} IsDynamicForm[{}]", __func__, formListFiller->IsDynamicForm());
            int max = formListFiller->forms.size();
            for (int i = 0; i < max; i++) {
                newForm->AddForm(formListFiller->forms[i]);
            }
        }
    }
    return newForm;
}

RE::BGSColorForm* CreateColorForm(RE::StaticFunctionTag*, int color) {
    logger::info("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSColorForm>()->Create();
    if (!newForm) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::info("{} success", __func__);
        newForm->color = color;
    }

    return newForm;
}

RE::BGSConstructibleObject* CreateConstructibleObject(RE::StaticFunctionTag*) {
    logger::info("{} called", __func__);

    //RE::BGSConstructibleObject
    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSConstructibleObject>()->Create();
    if (!newForm) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::info("{} success", __func__);
    }

    return newForm;
}

RE::BGSTextureSet* CreateTextureSet(RE::StaticFunctionTag*) {
    logger::info("{} called", __func__);

    //RE::BGSConstructibleObject
    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSTextureSet>()->Create();
    if (!newForm) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::info("{} success", __func__);
    }

    return newForm;
}

//Papyrus Events =============================================================================================================================

enum EventsEnum {
    EventEnum_OnLoadGame,
    EventEnum_OnCombatStateChanged,
    EventEnum_FurnitureEnter,
    EventEnum_FurnitureExit,
    EventEnum_OnActivate,
    EventEnum_HitEvent,
    EventEnum_DeathEvent,
    EventEnum_DyingEvent,
    EventEnum_OnObjectEquipped,
    EventEnum_OnObjectUnequipped,
    EventEnum_First = EventEnum_OnLoadGame,
    EventEnum_Last = EventEnum_OnObjectUnequipped
};


void SendEvents(std::vector<RE::VMHandle> handles, RE::BSFixedString& sEvent, RE::BSScript::IFunctionArguments* args) {
    int max = handles.size();

    if (max == 0) {
        return;
    }

    for (int i = 0; i < max; i++) {
        svm->SendAndRelayEvent(handles[i], &sEvent, args, nullptr);
    }

    delete args; //args is created using makeFunctionArguments. Delete as it's no longer needed.
}

void CombineEventHandles(std::vector<RE::VMHandle>& handles, RE::TESForm* akForm, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& formHandles) {
    if (formHandles.size() == 0) {
        return;
    }

    if (!akForm) {
        return;
    }

    auto it = formHandles.find(akForm);

    if (it != formHandles.end()) {
        logger::info("{}: events for form: [{}] ID[{:x}] found", __func__, GetFormName(akForm), akForm->GetFormID());
        handles.reserve(handles.size() + it->second.size());
        handles.insert(handles.end(), it->second.begin(), it->second.end());
    }
    else {
        logger::info("{}: events for form: [{}] ID[{:x}] not found", __func__, GetFormName(akForm), akForm->GetFormID());
    }
}

struct EventData {
    int eventSinkIndex;
    bool sinkAdded = false;
    RE::BSFixedString sEvent;
    std::uint32_t record;

    std::vector<RE::VMHandle> globalHandles; //handles that receive all events
    std::vector< std::map<RE::TESForm*, std::vector<RE::VMHandle>> > eventParamMaps; //event param form comparisons

    //constructor
    EventData(RE::BSFixedString event, int ceventSinkIndex, int NumberOfParams, std::uint32_t crecord) :
        eventSinkIndex(ceventSinkIndex),
        sEvent(event),
        record(crecord)
    {
        eventParamMaps.resize(NumberOfParams);
    }

    bool PlayerIsRegistered() {
        int max = eventParamMaps.size();
        for (int i = 0; i < max; i++) {
            auto it = eventParamMaps[i].find(playerRef);
            if (it != eventParamMaps[i].end()) {
                return true;
            }
        }
        return false;
    }

    bool isEmpty() {
        for (auto formMapHandle : eventParamMaps) {
            if (formMapHandle.size() > 0) {
                return false;
            }
        }
        return (globalHandles.size() == 0);
    }

    void removeSinkIfEmpty() {
        if (isEmpty()) {
            RemoveSink(eventSinkIndex);
        }
        if (eventSinkIndex == EventEnum_OnCombatStateChanged) {
            if (bRegisteredForPlayerCombatChange) {
                if (!PlayerIsRegistered()) {
                    bRegisteredForPlayerCombatChange = false;
                }
            }
        }
    }

    void InsertIntoFormHandles(RE::VMHandle handle, RE::TESForm* akForm, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& eventFormHandles) {
        if (akForm) {
            auto it = eventFormHandles.find(akForm);
            if (it != eventFormHandles.end()) { //form found in activator param map
                int handleIndex = GetIndexInVector(it->second, handle);
                if (handleIndex == -1) { //handle not already added for this form (activator param)
                    it->second.push_back(handle);
                    logger::info("{}: akForm[{}] ID[{:x}] found, handles sixe[{}]", __func__, GetFormName(akForm), akForm->GetFormID(), it->second.size());
                }
            }
            else { //form not found
                std::vector<RE::VMHandle> handles;
                handles.push_back(handle);
                eventFormHandles.insert(std::pair<RE::TESForm*, std::vector<RE::VMHandle>>(akForm, handles));

                logger::info("{}: akForm[{}] ID[{:x}] doesn't already have handles, handles size[{}] eventFormHandles size[{}]", __func__, GetFormName(akForm), akForm->GetFormID(), handles.size(), eventFormHandles.size());
            }
        }
    }

    void EraseFromFormHandles(RE::VMHandle handle, RE::TESForm* akForm, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& eventFormHandles) {
        if (akForm) {
            auto it = eventFormHandles.find(akForm);
            if (it != eventFormHandles.end()) { //form found in activator param map
                int handleIndex = GetIndexInVector(it->second, handle);
                if (handleIndex != -1) { //handle not already added for this form (activator param)
                    auto itb = it->second.begin() + handleIndex;
                    it->second.erase(itb);

                    if (it->second.size() == 0) { //no more handles for this akForm
                        eventFormHandles.erase(akForm);
                    }
                }
            }
        }
    }

    //erase all instances of handle
    void EraseFromFormHandles(RE::VMHandle handle, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& eventFormHandles) { //erase all instances of handle
        for (auto it : eventFormHandles) {
            int handleIndex = GetIndexInVector(it.second, handle);
            if (handleIndex != -1) {
                auto itb = it.second.begin() + handleIndex;
                it.second.erase(itb);

                if (it.second.size() == 0) { //no more handles for this akForm
                    eventFormHandles.erase(it.first);
                }
            }
        }
    }

    void AddHandle(RE::VMHandle handle, RE::TESForm* paramFilter, int paramFilterIndex) {
        if (!paramFilter) {
            globalHandles.push_back(handle);

            if (eventSinkIndex == EventEnum_OnCombatStateChanged) {
                bRegisteredForPlayerCombatChange = true;
            }
        }
        else {
            int index = paramFilterIndex;
            if (index >= eventParamMaps.size()) {
                index = eventParamMaps.size() - 1;
            }
            if (index < 0) {
                index = 0;
            }
            InsertIntoFormHandles(handle, paramFilter, eventParamMaps[paramFilterIndex]);

            if (eventSinkIndex == EventEnum_OnCombatStateChanged && paramFilterIndex == 0) {
                logger::info("{}: adding handle for Combat State change", __func__);
                if (paramFilter->As<RE::Actor>() == playerRef) {
                    //playerForm = paramFilter;
                    bRegisteredForPlayerCombatChange = true;
                    logger::info("{}: bRegisteredForPlayerCombatChange = true", __func__);
                }
            }
        }
    }

    void RemoveHandle(RE::VMHandle handle, RE::TESForm* paramFilter, int paramFilterIndex) {
        int gIndex = GetIndexInVector(globalHandles, handle);

        if (!paramFilter) {
            if (gIndex != -1) {
                auto it = globalHandles.begin() + gIndex;
                globalHandles.erase(it);
            }
        }
        else {
            int index = paramFilterIndex;
            if (index >= eventParamMaps.size()) {
                index = eventParamMaps.size() - 1;
            }
            if (index < 0) {
                index = 0;
            }
            logger::info("{}: eventSinkIndex [{}] Index = [{}]", __func__, eventSinkIndex, index);
            EraseFromFormHandles(handle, paramFilter, eventParamMaps[paramFilterIndex]);
        }

        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1500)); //wait for 1.5 seconds before checking removeSinkIfEmpty
            removeSinkIfEmpty();

            });
        t.detach();
    }

    void RemoveAllHandles(RE::VMHandle handle) {
        int gIndex = GetIndexInVector(globalHandles, handle);
        if (gIndex != -1) {
            auto it = globalHandles.begin() + gIndex;
            globalHandles.erase(it);
        }

        int max = eventParamMaps.size();
        for (int i = 0; i < max; i++) {
            EraseFromFormHandles(handle, eventParamMaps[i]);
        }
    }

    bool HasHandle(RE::VMHandle handle, RE::TESForm* paramFilter, int paramFilterIndex) {
        if (!paramFilter) {
            int gIndex = GetIndexInVector(globalHandles, handle);
            return (gIndex != -1);
        }

        int index = paramFilterIndex;
        if (index >= eventParamMaps.size()) {
            index = eventParamMaps.size() - 1;
        }
        if (index < 0) {
            index = 0;
        }

        auto it = eventParamMaps[index].find(paramFilter);
        if (it != eventParamMaps[index].end()) {
            return (GetIndexInVector(it->second, handle) != -1);
        }
        return false;
    }

    bool Load(SKSE::SerializationInterface* a_intfc) {
        if (!LoadHandlesVector(globalHandles, record, a_intfc)) {
            return false;
        }

        std::size_t size;
        if (!a_intfc->ReadRecordData(size)) {
            logger::error("{}: Event[{}] Failed to load size of eventParamMaps!", __func__, sEvent);
            return false;
        }

        for (std::size_t i = 0; i < size; i++) {
            if (!LoadFormHandlesMap(eventParamMaps[i], record, a_intfc)) {
                return false;
            }
        }
        return true;
    }

    bool Save(SKSE::SerializationInterface* a_intfc) {
        if (!a_intfc->OpenRecord(record, 1)) {
            logger::error("{}: Failed to open record[{}]", __func__, record);
            return false;
        }

        if (!SaveHandlesVector(globalHandles, record, a_intfc)) {
            return false;
        }

        const std::size_t size = eventParamMaps.size();

        if (!a_intfc->WriteRecordData(size)) {
            logger::error("{}: record[{}] Failed to write size of arr!", __func__, record);
            return false;
        }

        for (std::size_t i = 0; i < size; i++) {
            if (!SaveFormHandlesMap(eventParamMaps[i], record, a_intfc)) {
                return false;
            }
        }
        return true;
    }
};

std::vector<EventData*> eventDataPtrs;

struct HitEventSink : public RE::BSTEventSink<RE::TESHitEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* event, RE::BSTEventSource<RE::TESHitEvent>*/*source*/) {

        RE::TESObjectREFR* attacker = event->cause.get();
        RE::TESObjectREFR* target = event->target.get();
        RE::TESForm* source = RE::TESForm::LookupByID(event->source);
        RE::TESAmmo* ammo = nullptr;
        RE::BGSProjectile* projectile = RE::TESForm::LookupByID<RE::BGSProjectile>(event->projectile);
        bool powerAttack = event->flags.any(RE::TESHitEvent::Flag::kPowerAttack);
        bool SneakAttack = event->flags.any(RE::TESHitEvent::Flag::kSneakAttack);
        bool bBashAttack = event->flags.any(RE::TESHitEvent::Flag::kBashAttack);
        bool HitBlocked = event->flags.any(RE::TESHitEvent::Flag::kHitBlocked);

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_HitEvent]->globalHandles;

        if (source) {
            RE::TESObjectWEAP* weapon = source->As<RE::TESObjectWEAP>();
            if (weapon) {
                if (weapon->IsBow() || weapon->IsCrossbow()) {
                    if (attacker) {
                        RE::Actor* actorRef = attacker->As<RE::Actor>();
                        if (actorRef) {
                            ammo = actorRef->GetCurrentAmmo();
                        }
                    }
                }
            }
        }

        CombineEventHandles(handles, attacker, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[0]);
        CombineEventHandles(handles, target, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[1]);
        CombineEventHandles(handles, source, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[2]);
        CombineEventHandles(handles, ammo, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[3]);
        CombineEventHandles(handles, projectile, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[4]);

        RemoveDuplicates(handles);

        logger::info("HitEvent: attacker[{}]  target[{}]  source[{}]  ammo[{}]  projectile[{}]", GetFormName(attacker), GetFormName(target), GetFormName(source), GetFormName(ammo), GetFormName(projectile));
        logger::info("HitEvent: powerAttack[{}]  SneakAttack[{}]  BashAttack[{}]  HitBlocked[{}]  ", powerAttack, SneakAttack, bBashAttack, HitBlocked);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)attacker, (RE::TESObjectREFR*)target, (RE::TESForm*)source,
            (RE::TESAmmo*)ammo, (RE::BGSProjectile*)projectile, (bool)powerAttack, (bool)SneakAttack, (bool)bBashAttack, (bool)HitBlocked);

        SendEvents(handles, eventDataPtrs[EventEnum_HitEvent]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

void CheckForPlayerCombatStatusChange() {
    logger::info("{}", __func__);

    bool playerInCombat = player->IsInCombat();
    if (bPlayerIsInCombat != playerInCombat) {
        bPlayerIsInCombat = playerInCombat;
        int combatState = static_cast<int>(bPlayerIsInCombat);

        RE::Actor* target = nullptr;
        auto* combatGroup = player->GetCombatGroup();
        if (combatGroup) {
            if (combatGroup->targets.size() > 0) {
                auto combatHandle = combatGroup->targets[0].targetHandle;
                if (combatHandle) {
                    auto combatPtr = combatHandle.get();
                    if (combatPtr) {
                        target = combatPtr.get();
                    }
                }
            }
        }

        logger::info("{} target[{}]", __func__, GetFormName(target));

        // RE::TESForm* Target = .attackedMember.get().get();

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnCombatStateChanged]->globalHandles; //
        CombineEventHandles(handles, playerRef, eventDataPtrs[EventEnum_OnCombatStateChanged]->eventParamMaps[0]);
        CombineEventHandles(handles, target, eventDataPtrs[EventEnum_OnCombatStateChanged]->eventParamMaps[1]);

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)playerRef, (RE::Actor*)target, (int)combatState);
        SendEvents(handles, eventDataPtrs[EventEnum_OnCombatStateChanged]->sEvent, args);
    }
}

struct CombatEventSink : public RE::BSTEventSink<RE::TESCombatEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* event, RE::BSTEventSource<RE::TESCombatEvent>*/*source*/) {

        RE::TESObjectREFR* actorObjRef = event->actor.get();
        RE::TESObjectREFR* Target = event->targetActor.get();
        //RE::Actor* actorRef = actorObjRef->As<RE::Actor>();
        int combatState = static_cast<int>(event->newState.get());
        //logger::info("Actor {} changed to combat state to {} with", GetFormName(actorObjRef), combatState);

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnCombatStateChanged]->globalHandles;
        CombineEventHandles(handles, actorObjRef, eventDataPtrs[EventEnum_OnCombatStateChanged]->eventParamMaps[0]);
        CombineEventHandles(handles, Target, eventDataPtrs[EventEnum_OnCombatStateChanged]->eventParamMaps[1]);

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)actorObjRef, (RE::Actor*)Target, (int)combatState);
        SendEvents(handles, eventDataPtrs[EventEnum_OnCombatStateChanged]->sEvent, args);

        if (bRegisteredForPlayerCombatChange) {
            DelayedFunction(&CheckForPlayerCombatStatusChange, 1200); //check for player combat status change after 1.2 seconds.
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct FurnitureEventSink : public RE::BSTEventSink<RE::TESFurnitureEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESFurnitureEvent* event, RE::BSTEventSource<RE::TESFurnitureEvent>*/*source*/) {

        RE::TESObjectREFR* actorObjRef = event->actor.get();
        RE::Actor* actorRef = actorObjRef->As<RE::Actor>();
        RE::TESObjectREFR* furnitureRef = event->targetFurniture.get();
        int type = event->type.underlying();

        RE::BSFixedString sEvent;
        int eventIndex;

        switch (type) {
        case 0: //enter
            eventIndex = EventEnum_FurnitureEnter;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
            break;
        case 1: //exit
            eventIndex = EventEnum_FurnitureExit;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
            break;
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->globalHandles;
        CombineEventHandles(handles, actorObjRef, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        CombineEventHandles(handles, furnitureRef, eventDataPtrs[eventIndex]->eventParamMaps[1]);

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)actorRef, (RE::TESObjectREFR*)furnitureRef);
        SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct ActivateEventSink : public RE::BSTEventSink<RE::TESActivateEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* event, RE::BSTEventSource<RE::TESActivateEvent>*/*source*/) {
        RE::TESObjectREFR* activatorRef = event->actionRef.get();
        RE::TESObjectREFR* activatedRef = event->objectActivated.get();

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnActivate]->globalHandles;

        CombineEventHandles(handles, activatorRef, eventDataPtrs[EventEnum_OnActivate]->eventParamMaps[0]);
        CombineEventHandles(handles, activatedRef, eventDataPtrs[EventEnum_OnActivate]->eventParamMaps[1]);

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)activatorRef, (RE::TESObjectREFR*)activatedRef);
        SendEvents(handles, eventDataPtrs[EventEnum_OnActivate]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct DeathEventSink : public RE::BSTEventSink<RE::TESDeathEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* event, RE::BSTEventSource<RE::TESDeathEvent>* source) {
        logger::info("Death Event: dead[{}]", event->dead);

        RE::TESObjectREFR* victimRef;
        if (event->actorDying) {
            victimRef = event->actorDying.get();
        }

        RE::TESObjectREFR* killerRef;
        if (event->actorKiller) {
            killerRef = event->actorKiller.get();
        }

        RE::Actor* victim;
        if (victimRef != nullptr) {
            victim = victimRef->As<RE::Actor>();
        }

        RE::Actor* killer;
        if (killerRef != nullptr) {
            //this is necessarry because when using console command kill or script command actor.kill() killer will be a bad ptr and cause ctd.
            if (IsBadReadPtr(killerRef, sizeof(killerRef))) {  
                killer = nullptr;
                logger::info("death event: bad pointer");
            }
            else if (killerRef->GetFormID() == 0) {
                killer = nullptr;
                logger::info("death event: 0 pointer");
            }
            else {
                killer = static_cast<RE::Actor*>(killerRef);
                logger::info("death event: valid pointer");
            }
        }

        bool dead = event->dead;

        logger::info("Death Event: victim[{}], Killer[{}], Dead[{}]", GetFormName(victim), GetFormName(killer), dead);

        RE::BSFixedString sEvent;
        int eventIndex;

        if (dead) {
            eventIndex = EventEnum_DeathEvent;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
        }
        else {
            eventIndex = EventEnum_DyingEvent;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->globalHandles;
        CombineEventHandles(handles, victim, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        CombineEventHandles(handles, killer, eventDataPtrs[eventIndex]->eventParamMaps[1]);
        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)victim, (RE::Actor*)killer);
        SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct EquipEventSink : public RE::BSTEventSink<RE::TESEquipEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*/*source*/) {

        logger::info("Equip Event");

        RE::TESObjectREFR* akActorRef = event->actor.get();
        //RE::Actor* akActor = akActorRef->As<RE::Actor>();
        RE::TESForm* baseObject = RE::TESForm::LookupByID(event->baseObject);
        RE::TESForm* ref = RE::TESForm::LookupByID(event->originalRefr);
        bool equipped = event->equipped;

        logger::info("Equip Event: Actor[{}], BaseObject[{}], Ref[{}] Equipped[{}]", GetFormName(akActorRef), GetFormName(baseObject), GetFormName(ref), equipped);

        int eventIndex;

        if (equipped) {
            eventIndex = EventEnum_OnObjectEquipped;
        }
        else {
            eventIndex = EventEnum_OnObjectUnequipped;
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->globalHandles;

        CombineEventHandles(handles, akActorRef, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        CombineEventHandles(handles, baseObject, eventDataPtrs[eventIndex]->eventParamMaps[1]);
        CombineEventHandles(handles, ref, eventDataPtrs[eventIndex]->eventParamMaps[2]);

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)akActorRef, (RE::TESForm*)baseObject, (RE::TESObjectREFR*)ref);
        SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct TestEventSink : public RE::BSTEventSink<RE::TESActivateEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* event, RE::BSTEventSource<RE::TESActivateEvent>*/*source*/) {

        //logger::info("music Event");

        //event.

        /*std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->globalHandles;

        CombineEventHandles(handles, akActorRef, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        CombineEventHandles(handles, baseObject, eventDataPtrs[eventIndex]->eventParamMaps[1]);
        CombineEventHandles(handles, ref, eventDataPtrs[eventIndex]->eventParamMaps[2]);

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)akActorRef, (RE::TESForm*)baseObject, (RE::TESObjectREFR*)ref);
        SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);*/

        return RE::BSEventNotifyControl::kContinue;
    }
};

TestEventSink* testEventSink;

HitEventSink* hitEventSink;
CombatEventSink* combatEventSink;
FurnitureEventSink* furnitureEventSink;
ActivateEventSink* activateEventSink;
DeathEventSink* deathEventSink;
EquipEventSink* equipEventSink;

void AddSink(int index) {

    switch (index) {
    case EventEnum_OnCombatStateChanged:
        if (!eventDataPtrs[EventEnum_OnCombatStateChanged]->sinkAdded && !eventDataPtrs[EventEnum_OnCombatStateChanged]->isEmpty()) {
            logger::info("{}, EventEnum_OnCombatStateChanged sink added", __func__);
            eventDataPtrs[EventEnum_OnCombatStateChanged]->sinkAdded = true;
            eventSourceholder->AddEventSink(combatEventSink);
        }
        break;

    case EventEnum_FurnitureEnter:

    case EventEnum_FurnitureExit:
        if (!eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded && !eventDataPtrs[EventEnum_FurnitureExit]->sinkAdded && (!eventDataPtrs[EventEnum_FurnitureEnter]->isEmpty() || !eventDataPtrs[EventEnum_FurnitureExit]->isEmpty())) {
            logger::info("{}, EventEnum_FurnitureExit sink added", __func__);
            eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded = true;
            eventDataPtrs[EventEnum_FurnitureExit]->sinkAdded = true;
            eventSourceholder->AddEventSink(furnitureEventSink);
        }
        break;

    case EventEnum_OnActivate:
        if (!eventDataPtrs[EventEnum_OnActivate]->sinkAdded && !eventDataPtrs[EventEnum_OnActivate]->isEmpty()) {
            logger::info("{}, EventEnum_OnActivate sink added", __func__);
            eventDataPtrs[EventEnum_OnActivate]->sinkAdded = true;
            eventSourceholder->AddEventSink(activateEventSink);
        }
        break;

    case EventEnum_HitEvent:
        if (!eventDataPtrs[EventEnum_HitEvent]->sinkAdded && !eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
            logger::info("{}, EventEnum_hitEvent sink added", __func__);
            eventDataPtrs[EventEnum_HitEvent]->sinkAdded = true;
            eventSourceholder->AddEventSink(hitEventSink);
        }
        break;

    case EventEnum_DeathEvent:

    case EventEnum_DyingEvent:
        if (!eventDataPtrs[EventEnum_DeathEvent]->sinkAdded && !eventDataPtrs[EventEnum_DyingEvent]->sinkAdded && (!eventDataPtrs[EventEnum_DeathEvent]->isEmpty() || !eventDataPtrs[EventEnum_DyingEvent]->isEmpty())) {
            logger::info("{}, EventEnum_DyingEvent sink added", __func__);
            eventDataPtrs[EventEnum_DyingEvent]->sinkAdded = true;
            eventDataPtrs[EventEnum_DeathEvent]->sinkAdded = true;
            eventSourceholder->AddEventSink(deathEventSink);
        }
        break;

    case EventEnum_OnObjectEquipped:

    case EventEnum_OnObjectUnequipped:
        if (!eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded && !eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded && (!eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty() || !eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty())) {
            logger::info("{}, EventEnum_OnObjectUnequipped sink added", __func__);
            eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded = true;
            eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded = true;
            eventSourceholder->AddEventSink(equipEventSink);
        }
        break;
    }
}

void RemoveSink(int index) {
    logger::info("removing sink {}", index);

    switch (index) {
    case EventEnum_OnCombatStateChanged:
        if (!eventDataPtrs[EventEnum_OnCombatStateChanged]->sinkAdded && !eventDataPtrs[EventEnum_OnCombatStateChanged]->isEmpty()) {
            logger::info("{}, EventEnum_OnCombatStateChanged sink removed", __func__);
            eventDataPtrs[EventEnum_OnCombatStateChanged]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(combatEventSink);
        }
        break;

    case EventEnum_FurnitureEnter:

    case EventEnum_FurnitureExit:
        if (eventDataPtrs[EventEnum_FurnitureEnter]->isEmpty() && eventDataPtrs[EventEnum_FurnitureExit]->isEmpty()) {
            logger::info("{}, EventEnum_FurnitureEnter sink removed", __func__);
            eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded = false;
            eventDataPtrs[EventEnum_FurnitureExit]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(furnitureEventSink);
        }
        break;

    case EventEnum_OnActivate:
        if (eventDataPtrs[EventEnum_OnActivate]->sinkAdded) {
            logger::info("{}, EventEnum_OnActivate sink removed", __func__);
            eventDataPtrs[EventEnum_OnActivate]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(activateEventSink);
        }
        break;

    case EventEnum_HitEvent:
        if (!eventDataPtrs[EventEnum_HitEvent]->sinkAdded && !eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
            logger::info("{}, EventEnum_hitEvent sink removed", __func__);
            eventDataPtrs[EventEnum_HitEvent]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(hitEventSink);
        }
        break;

    case EventEnum_DeathEvent:

    case EventEnum_DyingEvent:
        if (eventDataPtrs[EventEnum_DeathEvent]->isEmpty() && eventDataPtrs[EventEnum_DyingEvent]->isEmpty()) {
            logger::info("{}, EventEnum_DeathEvent sink removed", __func__);
            eventDataPtrs[EventEnum_DeathEvent]->sinkAdded = false;
            eventDataPtrs[EventEnum_DyingEvent]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(deathEventSink);
        }
        break;

    case EventEnum_OnObjectEquipped:

    case EventEnum_OnObjectUnequipped:
        if (eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty() && eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty()) {
            logger::info("{}, EventEnum_OnObjectEquipped sink removed", __func__);
            eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded = false;
            eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(deathEventSink);
        }
        break;
    }
}

//Global Events papyrus===================================================================================================================================

int GetEventIndex(std::vector<EventData*> v, RE::BSFixedString asEvent) {
    if (asEvent == "") {
        return -1;
    }

    int m = v.size();
    if (m == 0) {
        return -1;
    }

    for (int i = 0; i < m; i++) {
        if (v[i]->sEvent == asEvent) {
            return i;
        }
    }

    return -1;
}

// is registered
bool IsFormRegisteredForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return false;
    }

    logger::info("{} getting handle is registered for: {}", __func__, GetFormName(eventReceiver));

    RE::VMTypeID id = static_cast<RE::VMTypeID>(eventReceiver->GetFormType());
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return false;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        return eventDataPtrs[index]->HasHandle(handle, paramFilter, paramFilterIndex);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
        return false;
    }
}

bool IsAliasRegisteredForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::BGSBaseAlias* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return false;
    }

    logger::info("{} getting handle is registered for: {}", __func__, eventReceiver->aliasName);

    RE::VMTypeID id = eventReceiver->GetVMTypeID();
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->GetVMTypeID());
        return false;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        return eventDataPtrs[index]->HasHandle(handle, paramFilter, paramFilterIndex);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
        return false;
    }
}

bool IsActiveMagicEffectRegisteredForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::ActiveEffect* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return false;
    }

    logger::info("{} getting handle is registered for: {} instance", __func__, GetFormName(eventReceiver->GetBaseObject()));

    RE::VMTypeID id = eventReceiver->VMTYPEID;
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for effect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->VMTYPEID);
        return false;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        return eventDataPtrs[index]->HasHandle(handle, paramFilter, paramFilterIndex);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
        return false;
    }
}

//register
void RegisterFormForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    logger::info("{} adding handle for: {}", __func__, GetFormName(eventReceiver));

    RE::VMTypeID id = static_cast<RE::VMTypeID>(eventReceiver->GetFormType());
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->AddHandle(handle, paramFilter, paramFilterIndex);
        AddSink(index);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

void RegisterAliasForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::BGSBaseAlias* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    logger::info("{} adding handle for: {}", __func__, eventReceiver->aliasName);

    RE::VMTypeID id = eventReceiver->GetVMTypeID();
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->GetVMTypeID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->AddHandle(handle, paramFilter, paramFilterIndex);
        AddSink(index);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

void RegisterActiveMagicEffectForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::ActiveEffect* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    logger::info("{} adding handle for: {} instance", __func__, GetFormName(eventReceiver->GetBaseObject()));

    RE::VMTypeID id = eventReceiver->VMTYPEID;
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for effect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->VMTYPEID);
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->AddHandle(handle, paramFilter, paramFilterIndex);
        AddSink(index);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

//unregister
void UnregisterFormForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    logger::info("{} adding handle for: {}", __func__, GetFormName(eventReceiver));

    RE::VMTypeID id = static_cast<RE::VMTypeID>(eventReceiver->GetFormType());
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->RemoveHandle(handle, paramFilter, paramFilterIndex);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

void UnregisterAliasForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::BGSBaseAlias* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    logger::info("{} removing handle for: {}", __func__, eventReceiver->aliasName);

    RE::VMTypeID id = eventReceiver->GetVMTypeID();
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->GetVMTypeID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->RemoveHandle(handle, paramFilter, paramFilterIndex);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

void UnregisterActiveMagicEffectForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::ActiveEffect* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    logger::info("{} removing handle for: {} instance", __func__, GetFormName(eventReceiver->GetBaseObject()));

    RE::VMTypeID id = eventReceiver->VMTYPEID;
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for effect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->VMTYPEID);
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->RemoveHandle(handle, paramFilter, paramFilterIndex);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

//unregister all
void UnregisterFormForGlobalEvent_All(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    logger::info("{} adding handle for: {}", __func__, GetFormName(eventReceiver));

    RE::VMTypeID id = static_cast<RE::VMTypeID>(eventReceiver->GetFormType());
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->RemoveAllHandles(handle);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

void UnregisterAliasForGlobalEvent_All(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::BGSBaseAlias* eventReceiver) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    logger::info("{} removing all handles for: {}", __func__, eventReceiver->aliasName);

    RE::VMTypeID id = eventReceiver->GetVMTypeID();
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->GetVMTypeID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->RemoveAllHandles(handle);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

void UnregisterActiveMagicEffectForGlobalEvent_All(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::ActiveEffect* eventReceiver) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    logger::info("{} removing all handles for: {} instance", __func__, GetFormName(eventReceiver->GetBaseObject()));

    RE::VMTypeID id = eventReceiver->VMTYPEID;
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for effect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->VMTYPEID);
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->RemoveAllHandles(handle);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

// plugin load / maintenance==================================================================================================================================================
void CreateEventSinks() {
    logger::info("creating event sinks");

    if (!player) { player = RE::PlayerCharacter::GetSingleton(); }
    if (!playerRef) { playerRef = RE::TESForm::LookupByID<RE::TESForm>(20)->As<RE::Actor>(); }
    if (!eventSourceholder) { eventSourceholder = RE::ScriptEventSourceHolder::GetSingleton(); }

    if (eventDataPtrs.size() == 0) {
        eventDataPtrs.resize((EventEnum_Last + 1), nullptr);
        eventDataPtrs[EventEnum_OnLoadGame] = new EventData("OnLoadGameGlobal", EventEnum_OnLoadGame, 0, 'EDl0');
        eventDataPtrs[EventEnum_OnCombatStateChanged] = new EventData("OnCombatStateChangedGlobal", EventEnum_OnCombatStateChanged, 2, 'EDc0');
        eventDataPtrs[EventEnum_FurnitureEnter] = new EventData("OnFurnitureEnterGlobal", EventEnum_FurnitureEnter, 2, 'EDf0');
        eventDataPtrs[EventEnum_FurnitureExit] = new EventData("OnFurnitureExitGlobal", EventEnum_FurnitureExit, 2, 'EDf1');
        eventDataPtrs[EventEnum_OnActivate] = new EventData("OnActivateGlobal", EventEnum_OnActivate, 2, 'EDa0');
        eventDataPtrs[EventEnum_HitEvent] = new EventData("OnhitGlobal", EventEnum_HitEvent, 5, 'EDh0');
        eventDataPtrs[EventEnum_DeathEvent] = new EventData("OnDeathGlobal", EventEnum_HitEvent, 2, 'EDd0');
        eventDataPtrs[EventEnum_DyingEvent] = new EventData("OnDyingGlobal", EventEnum_HitEvent, 2, 'EDd1');
        eventDataPtrs[EventEnum_OnObjectEquipped] = new EventData("OnObjectEquippedGlobal", EventEnum_OnObjectEquipped, 3, 'EDe0');
        eventDataPtrs[EventEnum_OnObjectUnequipped] = new EventData("OnObjectUnequippedGlobal", EventEnum_OnObjectUnequipped, 3, 'EDe1');
    }

    if (!combatEventSink) { combatEventSink = new CombatEventSink(); }
    if (!furnitureEventSink) { furnitureEventSink = new FurnitureEventSink(); }
    if (!activateEventSink) { activateEventSink = new ActivateEventSink(); }
    if (!hitEventSink) { hitEventSink = new HitEventSink(); }
    if (!deathEventSink) { deathEventSink = new DeathEventSink(); }
    if (!equipEventSink) { equipEventSink = new EquipEventSink(); }
    if (!testEventSink) { testEventSink = new TestEventSink(); }
    //eventSourceholder->AddEventSink(testEventSink);

    //RE::ScriptEventSourceHolder

    //eventSourceholder->AddEventSink(updateEventSink);
}

bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    // vm->RegisterFunction("MyNativeFunction", "DbSkseFunctions", MyNativeFunction);
    logger::info("Binding Papyrus Functions");

    gvm = vm;
    svm = RE::SkyrimVM::GetSingleton();

    //functions
    vm->RegisterFunction("GetVersion", "DbSkseFunctions", GetThisVersion);
    vm->RegisterFunction("GetClipBoardText", "DbSkseFunctions", GetClipBoardText);
    vm->RegisterFunction("SetClipBoardText", "DbSkseFunctions", SetClipBoardText);
    vm->RegisterFunction("IsWhiteSpace", "DbSkseFunctions", IsWhiteSpace);
    vm->RegisterFunction("CountWhiteSpaces", "DbSkseFunctions", CountWhiteSpaces);
    vm->RegisterFunction("SetMapMarkerName", "DbSkseFunctions", SetMapMarkerName);
    vm->RegisterFunction("GetMapMarkerName", "DbSkseFunctions", GetMapMarkerName);
    vm->RegisterFunction("SetMapMarkerIconType", "DbSkseFunctions", SetMapMarkerIconType);
    vm->RegisterFunction("GetMapMarkerIconType", "DbSkseFunctions", GetMapMarkerIconType);
    vm->RegisterFunction("IsMapMarker", "DbSkseFunctions", IsMapMarker);
    vm->RegisterFunction("CreateMapMarker", "DbSkseFunctions", CreateMapMarker);

    vm->RegisterFunction("ExecuteConsoleCommand", "DbSkseFunctions", ExecuteConsoleCommand);
    vm->RegisterFunction("HasCollision", "DbSkseFunctions", HasCollision);
    vm->RegisterFunction("GetCurrentMusicType", "DbSkseFunctions", GetCurrentMusicType);
    vm->RegisterFunction("GetNumberOfTracksInMusicType", "DbSkseFunctions", GetNumberOfTracksInMusicType);
    vm->RegisterFunction("GetMusicTypeTrackIndex", "DbSkseFunctions", GetMusicTypeTrackIndex);
    vm->RegisterFunction("SetMusicTypeTrackIndex", "DbSkseFunctions", SetMusicTypeTrackIndex);
    vm->RegisterFunction("GetMusicTypePriority", "DbSkseFunctions", GetMusicTypePriority);
    vm->RegisterFunction("SetMusicTypePriority", "DbSkseFunctions", SetMusicTypePriority);
    vm->RegisterFunction("GetMusicTypeStatus", "DbSkseFunctions", GetMusicTypeStatus);
    vm->RegisterFunction("GetKnownEnchantments", "DbSkseFunctions", GetKnownEnchantments);
    vm->RegisterFunction("AddKnownEnchantmentsToFormList", "DbSkseFunctions", AddKnownEnchantmentsToFormList);
    vm->RegisterFunction("IsActorAttacking", "DbSkseFunctions", IsActorAttacking);
    vm->RegisterFunction("IsActorPowerAttacking", "DbSkseFunctions", IsActorPowerAttacking);
    vm->RegisterFunction("IsActorSpeaking", "DbSkseFunctions", IsActorSpeaking);
    vm->RegisterFunction("IsActorBlocking", "DbSkseFunctions", IsActorBlocking);
    vm->RegisterFunction("IsActorCasting", "DbSkseFunctions", IsActorCasting);
    vm->RegisterFunction("IsActorDualCasting", "DbSkseFunctions", IsActorDualCasting);
    vm->RegisterFunction("IsActorStaggered", "DbSkseFunctions", IsActorStaggered);
    vm->RegisterFunction("IsActorRecoiling", "DbSkseFunctions", IsActorRecoiling);
    vm->RegisterFunction("IsActorIgnoringCombat", "DbSkseFunctions", IsActorIgnoringCombat);
    vm->RegisterFunction("IsActorUndead", "DbSkseFunctions", IsActorUndead);
    vm->RegisterFunction("IsActorOnFlyingMount", "DbSkseFunctions", IsActorOnFlyingMount);
    vm->RegisterFunction("IsActorAMount", "DbSkseFunctions", IsActorAMount);
    vm->RegisterFunction("IsActorInMidAir", "DbSkseFunctions", IsActorInMidAir);
    vm->RegisterFunction("IsActorInRagdollState", "DbSkseFunctions", IsActorInRagdollState);
    vm->RegisterFunction("GetDetectionLevel", "DbSkseFunctions", GetDetectionLevel);

    vm->RegisterFunction("GetKeywordString", "DbSkseFunctions", GetKeywordString);
    vm->RegisterFunction("SetKeywordString", "DbSkseFunctions", SetKeywordString);

    vm->RegisterFunction("CreateKeyword", "DbSkseFunctions", CreateKeyword);
    vm->RegisterFunction("CreateFormList", "DbSkseFunctions", CreateFormList);
    vm->RegisterFunction("CreateColorForm", "DbSkseFunctions", CreateColorForm);
    vm->RegisterFunction("CreateConstructibleObject", "DbSkseFunctions", CreateConstructibleObject);
    vm->RegisterFunction("CreateTextureSet", "DbSkseFunctions", CreateTextureSet);

    //global events ====================================================================================================
    //form
    vm->RegisterFunction("IsFormRegisteredForGlobalEvent", "DbSkseEvents", IsFormRegisteredForGlobalEvent);
    vm->RegisterFunction("RegisterFormForGlobalEvent", "DbSkseEvents", RegisterFormForGlobalEvent);
    vm->RegisterFunction("UnregisterFormForGlobalEvent", "DbSkseEvents", UnregisterFormForGlobalEvent);
    vm->RegisterFunction("UnregisterFormForGlobalEvent_All", "DbSkseEvents", UnregisterFormForGlobalEvent_All);

    //alias
    vm->RegisterFunction("IsAliasRegisteredForGlobalEvent", "DbSkseEvents", IsAliasRegisteredForGlobalEvent);
    vm->RegisterFunction("RegisterAliasForGlobalEvent", "DbSkseEvents", RegisterAliasForGlobalEvent);
    vm->RegisterFunction("UnregisterAliasForGlobalEvent", "DbSkseEvents", UnregisterAliasForGlobalEvent);
    vm->RegisterFunction("UnregisterAliasForGlobalEvent_All", "DbSkseEvents", UnregisterAliasForGlobalEvent_All);

    //activeMagicEffect
    vm->RegisterFunction("IsActiveMagicEffectRegisteredForGlobalEvent", "DbSkseEvents", IsActiveMagicEffectRegisteredForGlobalEvent);
    vm->RegisterFunction("RegisterActiveMagicEffectForGlobalEvent", "DbSkseEvents", RegisterActiveMagicEffectForGlobalEvent);
    vm->RegisterFunction("UnregisterActiveMagicEffectForGlobalEvent", "DbSkseEvents", UnregisterActiveMagicEffectForGlobalEvent);
    vm->RegisterFunction("UnregisterActiveMagicEffectForGlobalEvent_All", "DbSkseEvents", UnregisterActiveMagicEffectForGlobalEvent_All);

    logger::info("Papyrus Functions Bound");

    return true;
}

void MessageListener(SKSE::MessagingInterface::Message* message) {
    switch (message->type) {
        // Descriptions are taken from the original skse64 library
        // See:
        // https://github.com/ianpatt/skse64/blob/09f520a2433747f33ae7d7c15b1164ca198932c3/skse64/PluginAPI.h#L193-L212
     //case SKSE::MessagingInterface::kPostLoad: //
        //    logger::info("kPostLoad: sent to registered plugins once all plugins have been loaded");
        //    break;

    //case SKSE::MessagingInterface::kPostPostLoad:
        //    logger::info(
        //        "kPostPostLoad: sent right after kPostLoad to facilitate the correct dispatching/registering of "
        //        "messages/listeners");
        //    break;

    //case SKSE::MessagingInterface::kPreLoadGame:
        //    // message->dataLen: length of file path, data: char* file path of .ess savegame file
        //    logger::info("kPreLoadGame: sent immediately before savegame is read");
        //    break;

    case SKSE::MessagingInterface::kPostLoadGame:
        // You will probably want to handle this event if your plugin uses a Preload callback
        // as there is a chance that after that callback is invoked the game will encounter an error
        // while loading the saved game (eg. corrupted save) which may require you to reset some of your
        // plugin state.
        //logger::info("kPostLoadGame: sent after an attempt to load a saved game has finished");
        //SendLoadGameEvent();
        //CreateEventSinks();
        bPlayerIsInCombat = player->IsInCombat();
        break;

        //case SKSE::MessagingInterface::kSaveGame:
            //    logger::info("kSaveGame");
            //    break;

        //case SKSE::MessagingInterface::kDeleteGame:
            //    // message->dataLen: length of file path, data: char* file path of .ess savegame file
            //    logger::info("kDeleteGame: sent right before deleting the .skse cosave and the .ess save");
            //    break;

        //case SKSE::MessagingInterface::kInputLoaded:
            //    logger::info("kInputLoaded: sent right after game input is loaded, right before the main menu initializes");
            //    break;

        //case SKSE::MessagingInterface::kNewGame:
        //    // message-data: CharGen TESQuest pointer (Note: I haven't confirmed the usefulness of this yet!)
        //    //CreateEventSinks();
        //    logger::info("kNewGame: sent after a new game is created, before the game has loaded");
        //    break;

    case SKSE::MessagingInterface::kDataLoaded:
        CreateEventSinks();
        logger::info("kDataLoaded: sent after the data handler has loaded all its forms");
        break;

        //default: //
            //    logger::info("Unknown system message of type: {}", message->type);
            //    break;
    }
}

void LoadCallback(SKSE::SerializationInterface* a_intfc) {

    std::uint32_t type, version, length;

    while (a_intfc->GetNextRecordInfo(type, version, length)) {
        if (type == eventDataPtrs[EventEnum_OnLoadGame]->record) {
            eventDataPtrs[EventEnum_OnLoadGame]->Load(a_intfc);
        }
        else if (type == eventDataPtrs[EventEnum_FurnitureEnter]->record) {
            eventDataPtrs[EventEnum_FurnitureEnter]->Load(a_intfc);
        }
        else if (type == eventDataPtrs[EventEnum_FurnitureExit]->record) {
            eventDataPtrs[EventEnum_FurnitureExit]->Load(a_intfc);
        }
        else if (type == eventDataPtrs[EventEnum_OnActivate]->record) {
            eventDataPtrs[EventEnum_OnActivate]->Load(a_intfc);
        }
        else if (type == eventDataPtrs[EventEnum_HitEvent]->record) {
            eventDataPtrs[EventEnum_HitEvent]->Load(a_intfc);
        }
        else if (type == eventDataPtrs[EventEnum_DeathEvent]->record) {
            eventDataPtrs[EventEnum_DeathEvent]->Load(a_intfc);
        }
        else if (type == eventDataPtrs[EventEnum_DyingEvent]->record) {
            eventDataPtrs[EventEnum_DyingEvent]->Load(a_intfc);
        }
        else if (type == eventDataPtrs[EventEnum_OnObjectEquipped]->record) {
            eventDataPtrs[EventEnum_OnObjectEquipped]->Load(a_intfc);
        }
        else if (type == eventDataPtrs[EventEnum_OnObjectUnequipped]->record) {
            eventDataPtrs[EventEnum_OnObjectUnequipped]->Load(a_intfc);
        }
    }

    if (!player) { player = RE::PlayerCharacter::GetSingleton(); }
    bPlayerIsInCombat = player->IsInCombat();

    //EventEnum_OnLoadGame doesn't have an events sink, hence EventEnum_First + 1
    for (int i = EventEnum_First + 1; i <= EventEnum_Last; i++) {
        AddSink(i);
    }

    RemoveDuplicates(eventDataPtrs[EventEnum_OnLoadGame]->globalHandles);

    auto* args = RE::MakeFunctionArguments();
    SendEvents(eventDataPtrs[EventEnum_OnLoadGame]->globalHandles, eventDataPtrs[EventEnum_OnLoadGame]->sEvent, args);

    logger::info("LoadCallback complete");
}

void SaveCallback(SKSE::SerializationInterface* a_intfc) {
    eventDataPtrs[EventEnum_OnLoadGame]->Save(a_intfc);
    eventDataPtrs[EventEnum_FurnitureEnter]->Save(a_intfc);
    eventDataPtrs[EventEnum_FurnitureExit]->Save(a_intfc);
    eventDataPtrs[EventEnum_OnActivate]->Save(a_intfc);
    eventDataPtrs[EventEnum_HitEvent]->Save(a_intfc);
    eventDataPtrs[EventEnum_DeathEvent]->Save(a_intfc);
    eventDataPtrs[EventEnum_DyingEvent]->Save(a_intfc);
    eventDataPtrs[EventEnum_OnObjectEquipped]->Save(a_intfc);
    eventDataPtrs[EventEnum_OnObjectUnequipped]->Save(a_intfc);
}

//init================================================================================================================================================================
SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);

    SetupLog();
    //CreateEventSinks();
    SKSE::GetMessagingInterface()->RegisterListener(MessageListener);

    SKSE::GetPapyrusInterface()->Register(BindPapyrusFunctions);

    auto serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID('DbSF');
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);
   

    return true;
}
