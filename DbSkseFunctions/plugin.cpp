#include <Windows.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <stdarg.h>
#include <winbase.h>
#include <iostream>
#include <stdio.h>
#include <chrono>
#include <ctime>
#include "mini/ini.h"
#include "STLThunk.h"

namespace logger = SKSE::log;
bool bPlayerIsInCombat = false;
bool bRegisteredForPlayerCombatChange = false;
bool inMenuMode = false;
bool gamePaused = false;
std::string lastMenuOpened;
std::vector<RE::BSFixedString> menusCurrentlyOpen;
std::chrono::system_clock::time_point lastTimeMenuWasOpened;
std::chrono::system_clock::time_point lastTimeGameWasPaused;
int gameTimerPollingInterval = 1500; //in milliseconds
float secondsPassedGameNotPaused = 0.0;
float lastFrameDelta = 0.1;
RE::PlayerCharacter* player;
RE::Actor* playerRef;
RE::BSScript::IVirtualMachine* gvm;
RE::SkyrimVM* svm;
RE::UI* ui;
RE::Calendar* calendar;
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

template< typename T >
std::string IntToHex(T i)
{
    std::stringstream stream;
    stream << ""
        << std::setfill('0') << std::setw(sizeof(T) * 2)
        << std::hex << i;
    return stream.str();
}

float timePointDiffToFloat(std::chrono::system_clock::time_point end, std::chrono::system_clock::time_point start) {
    std::chrono::duration<float> timeElapsed = end - start;
    return timeElapsed.count();
}

RE::VMHandle GetHandle(RE::TESForm* akForm) {
    if (!akForm) {
        logger::warn("{}: akForm doesn't exist", __func__);
        return NULL;
    }

    RE::VMTypeID id = static_cast<RE::VMTypeID>(akForm->GetFormType());
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, akForm);

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
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, akAlias);

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
    RE::VMHandle handle = svm->handlePolicy.GetHandleForObject(id, akEffect);

    if (handle == NULL) {
        return NULL;
    }
    return handle;
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

std::string GetDescription(RE::TESForm* akForm) {
    logger::info("{} called", __func__);

    if (!akForm) {
        logger::warn("{}: akForm doesn't exist", __func__);
        return "";
    }

    auto* description = akForm->As<RE::TESDescription>();

    if (!description) {
        logger::error("{}: couldn't get description for form [{}] ID [{:x}]", __func__, GetFormName(akForm), akForm->GetFormID());
        return "";
    }

    RE::BSString descriptionString;
    description->GetDescription(descriptionString, akForm);

    return static_cast<std::string>(descriptionString);
}

template <class T>
void RemoveFromVectorByValue(std::vector<T>& v, T value) {
    v.erase(std::remove(v.begin(), v.end(), value), v.end());
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

void SetSettingsFromIniFile() {
    // first, create a file instance
    mINI::INIFile file("Data/SKSE/Plugins/DbSkseFunctions.ini");

    // next, create a structure that will hold data
    mINI::INIStructure ini;

    // now we can read the file
    file.read(ini);

    std::string intervalString = ini["Main"]["gameTimerPollingInterval"];
    gameTimerPollingInterval = std::stof(intervalString) * 1000; //convert float value from seconds to int milliseconds
    if (gameTimerPollingInterval <= 0) {
        gameTimerPollingInterval = 100;
    }
    logger::debug("{} gameTimerPollingInterval set to {}", __func__, gameTimerPollingInterval);
}

void SetupLog() {
    // first, create a file instance
    mINI::INIFile file("Data/SKSE/Plugins/DbSkseFunctions.ini");

    // next, create a structure that will hold data
    mINI::INIStructure ini;

    // now we can read the file
    file.read(ini);

    std::string akLevel = ini["LOG"]["iMinLevel"];
    spdlog::level::level_enum iLevel = static_cast<spdlog::level::level_enum>(std::stoi(akLevel));

    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(iLevel);
    spdlog::flush_on(spdlog::level::trace);
    logger::info("{} level set to {}", __func__, akLevel);
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
    return float(5.3);
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

float GameHoursToRealTimeSeconds(RE::StaticFunctionTag*, float gameHours) {
    float timeScale = calendar->GetTimescale(); //timeScale is minutes ratio
    float gameMinutes = gameHours * 60.0;
    float realMinutes = gameMinutes / timeScale;
    return (realMinutes * 60.0);
}

bool IsGamePaused(RE::StaticFunctionTag*) {
    return ui->GameIsPaused();
}

bool IsInMenu(RE::StaticFunctionTag*) {
    return inMenuMode;
}

std::string GetLastMenuOpened(RE::StaticFunctionTag*) {
    return lastMenuOpened;
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

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* musicTypeArray = &(dataHandler->GetFormArray(RE::FormType::MusicType));

        RE::BSTArray<RE::TESForm*>::iterator itrEndType = musicTypeArray->end();

        RE::BSIMusicTrack* currentPriorityTrack = nullptr;
        std::int8_t currentPriority = 127;

        //loop through all music types to check which one is running and what the current track is for said type
        for (RE::BSTArray<RE::TESForm*>::iterator itr = musicTypeArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;

            if (baseForm) {
                RE::BGSMusicType* musicType = static_cast<RE::BGSMusicType*>(baseForm);
                RE::BSIMusicType::MUSIC_STATUS musicStatus = musicType->typeStatus.get();

                if (musicStatus == RE::BSIMusicType::MUSIC_STATUS::kPlaying) {
                    uint32_t currentTrackIndex = musicType->currentTrackIndex;

                    if (musicType->tracks.size() > currentTrackIndex) {
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

std::string GetWordOfPowerTranslation(RE::StaticFunctionTag*, RE::TESWordOfPower* akWord) {
    if (!akWord) {
        logger::warn("{} akWord doesn't exist.", __func__);
        return "";
    }

    return static_cast<std::string>(akWord->translation);
}

void UnlockShout(RE::StaticFunctionTag*, RE::TESShout* akShout) {
    if (!akShout) {
        logger::warn("{} akShout doesn't exist.", __func__);
        return;
    }

    player->AddShout(akShout);

    logger::info("{} {} ID {:x}", __func__, akShout->GetName(), akShout->GetFormID());

    RE::TESWordOfPower* word = akShout->variations[0].word;
    if (word) {
        //playerRef->UnlockWord(word);
        logger::info("{} unlock word 1 {} ID {:x}", __func__, word->GetName(), word->GetFormID());
        std::string command = "player.teachword " + IntToHex(int(word->GetFormID())); //didn't find a teachword function in NG, so using console command as workaround. 
        ExecuteConsoleCommand(nullptr, command, nullptr);
        player->UnlockWord(word);
    }

    word = akShout->variations[1].word;
    if (word) {
        //playerRef->UnlockWord(word);
        logger::info("{} unlock word 2 {} ID {:x}", __func__, word->GetName(), word->GetFormID());
        std::string command = "player.teachword " + IntToHex(int(word->GetFormID()));
        ExecuteConsoleCommand(nullptr, command, nullptr);
        player->UnlockWord(word);
    }

    word = akShout->variations[2].word;
    if (word) {
        //playerRef->UnlockWord(word);
        logger::info("{} unlock word 3 {} ID {:x}", __func__, word->GetName(), word->GetFormID());
        std::string command = "player.teachword " + IntToHex(int(word->GetFormID()));
        ExecuteConsoleCommand(nullptr, command, nullptr);
        player->UnlockWord(word);
    }
}

void AddAndUnlockAllShouts(RE::StaticFunctionTag*, int minNumberOfWordsWithTranslations, bool onlyShoutsWithNames, bool onlyShoutsWithDescriptions) {
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    bool minNumberOfWordsCheck = (minNumberOfWordsWithTranslations > 0 && minNumberOfWordsWithTranslations <= 3);

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Shout));

        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();

        //loop through all shouts
        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;
            if (baseForm) {
                RE::TESShout* akShout = baseForm->As<RE::TESShout>();
                if (akShout) {
                    if (onlyShoutsWithNames) {
                        if (akShout->GetName() == "") {
                            continue;
                        }
                    }

                    if (onlyShoutsWithDescriptions) {
                        if (GetDescription(akShout) == "") {
                            continue;
                        }
                    }

                    if (minNumberOfWordsCheck) {
                        RE::TESWordOfPower* word = akShout->variations[minNumberOfWordsWithTranslations - 1].word;
                        if (GetWordOfPowerTranslation(nullptr, word) == "") {
                            continue;
                        }
                    }
                    UnlockShout(nullptr, akShout);
                }
            }
        }
    }
}

RE::TESForm* GetActiveEffectSource(RE::StaticFunctionTag*, RE::ActiveEffect* akEffect) {
    if (!akEffect) {
        logger::error("{} akEffect doesn't exist", __func__);
        return nullptr;
    }

    return akEffect->spell;
}

int GetActiveEffectCastingSource(RE::StaticFunctionTag*, RE::ActiveEffect* akEffect) {
    if (!akEffect) {
        logger::error("{} akEffect doesn't exist", __func__);
        return -1;
    }

    return static_cast<int>(akEffect->castingSource);
}

//kNone = 0,
//kPetty = 1,
//kLesser = 2,
//kCommon = 3,
//kGreater = 4,
//kGrand = 5
void SetSoulGemSize(RE::StaticFunctionTag*, RE::TESSoulGem* akGem, int level) {
    logger::info("{} called", __func__);

    if (!akGem) {
        logger::error("{}: error akGem doesn't exist", __func__);
        return;
    }

    if (level < 0) {
        level = 0;
    }
    else if (level > 5) {
        level = 5;
    }

    akGem->soulCapacity = static_cast<RE::SOUL_LEVEL>(level);
}

void SetSoulGemCanHoldNPCSoul(RE::StaticFunctionTag*, RE::TESSoulGem* akGem, bool canCarry) {
    logger::info("{} called", __func__);

    if (!akGem) {
        logger::error("{}: error akGem doesn't exist", __func__);
        return;
    }

    if (canCarry) {
        akGem->formFlags |= RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
    }
    else {
        akGem->formFlags &= ~RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
    }
}

bool CanSoulGemHoldNPCSoul(RE::StaticFunctionTag*, RE::TESSoulGem* akGem) {
    logger::info("{} called", __func__);

    if (!akGem) {
        logger::error("{}: error akGem doesn't exist", __func__);
        return false;
    }
    return akGem->CanHoldNPCSoul();
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

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSTextureSet>()->Create();
    if (!newForm) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::info("{} success", __func__);
    }

    return newForm;
}

RE::TESSound* CreateSoundMarker(RE::StaticFunctionTag*) {
    logger::info("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESSound>()->Create();
    if (!newForm) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::info("{} success", __func__);
    }

    return newForm;
}

RE::BSFixedString soundFinishEvent = "OnSoundFinish";
void CreateSoundEvent(RE::TESForm* soundOrDescriptor, RE::BSSoundHandle& soundHandle, std::vector<RE::VMHandle> vmHandles, int intervalCheck) {
    std::thread t([=]() {
        //wait for sound to finish playing, then send events for handles.
        while (soundHandle.state.underlying() != 2 && soundHandle.IsValid()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalCheck));
        }
        auto* args = RE::MakeFunctionArguments((RE::TESForm*)soundOrDescriptor, (int)soundHandle.soundID);

        SendEvents(vmHandles, soundFinishEvent, args);
        });
    t.detach();
}

int PlaySound(RE::StaticFunctionTag*, RE::TESSound* akSound, RE::TESObjectREFR* akSource, float volume,
    RE::TESForm* eventReceiverForm, RE::BGSBaseAlias* eventReceiverAlias, RE::ActiveEffect* eventReceiverActiveEffect) {

    logger::info("{} called", __func__);

    if (!akSound) {
        logger::error("{}: error, akSound doesn't exist", __func__);
        return -1;
    }

    if (!akSource) {
        logger::error("{}: error, akSource doesn't exist", __func__);
        return -1;
    }

    auto* audiomanager = RE::BSAudioManager::GetSingleton();
    RE::BSSoundHandle soundHandle;

    audiomanager->BuildSoundDataFromDescriptor(soundHandle, akSound->descriptor->soundDescriptor);

    soundHandle.SetObjectToFollow(akSource->Get3D());
    soundHandle.SetVolume(volume);
    soundHandle.Play();

    std::vector<RE::VMHandle> vmHandles;

    if (eventReceiverForm) {
        RE::VMHandle vmHandle = GetHandle(eventReceiverForm);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (eventReceiverAlias) {
        RE::VMHandle vmHandle = GetHandle(eventReceiverAlias);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (eventReceiverActiveEffect) {
        RE::VMHandle vmHandle = GetHandle(eventReceiverActiveEffect);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (vmHandles.size() > 0) {
        CreateSoundEvent(akSound, soundHandle, vmHandles, 1000);
    }
    return soundHandle.soundID;
}

int PlaySoundDescriptor(RE::StaticFunctionTag*, RE::BGSSoundDescriptorForm* akSoundDescriptor, RE::TESObjectREFR* akSource, float volume,
    RE::TESForm* eventReceiverForm, RE::BGSBaseAlias* eventReceiverAlias, RE::ActiveEffect* eventReceiverActiveEffect) {

    logger::info("{} called", __func__);

    if (!akSoundDescriptor) {
        logger::error("{}: error, akSoundDescriptor doesn't exist", __func__);
        return -1;
    }

    if (!akSource) {
        logger::error("{}: error, akSource doesn't exist", __func__);
        return -1;
    }

    auto* audiomanager = RE::BSAudioManager::GetSingleton();
    RE::BSSoundHandle soundHandle;

    audiomanager->BuildSoundDataFromDescriptor(soundHandle, akSoundDescriptor->soundDescriptor);

    soundHandle.SetObjectToFollow(akSource->Get3D());
    soundHandle.SetVolume(volume);
    soundHandle.Play();

    std::vector<RE::VMHandle> vmHandles;

    if (eventReceiverForm) {
        RE::VMHandle vmHandle = GetHandle(eventReceiverForm);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (eventReceiverAlias) {
        RE::VMHandle vmHandle = GetHandle(eventReceiverAlias);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (eventReceiverActiveEffect) {
        RE::VMHandle vmHandle = GetHandle(eventReceiverActiveEffect);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (vmHandles.size() > 0) {
        CreateSoundEvent(akSoundDescriptor, soundHandle, vmHandles, 1000);
    }
    return soundHandle.soundID;
}

RE::BGSSoundCategory* GetParentSoundCategory(RE::StaticFunctionTag*, RE::BGSSoundCategory* akSoundCategory) {
    if (!akSoundCategory) {
        logger::error("{}: error, akSoundCategory doesn't exist", __func__);
        return nullptr;
    }

    return akSoundCategory->parentCategory;
}

RE::BGSSoundCategory* GetSoundCategoryForSoundDescriptor(RE::StaticFunctionTag*, RE::BGSSoundDescriptorForm* akSoundDescriptor) {
    if (!akSoundDescriptor) {
        logger::error("{}: error, akSoundDescriptor doesn't exist", __func__);
        return nullptr;
    }

    if (!akSoundDescriptor->soundDescriptor) {
        logger::error("{}: error, akSoundDescriptor->soundDescriptor doesn't exist", __func__);
        return nullptr;
    }

    return akSoundDescriptor->soundDescriptor->category;
}

void SetSoundCategoryForSoundDescriptor(RE::StaticFunctionTag*, RE::BGSSoundDescriptorForm* akSoundDescriptor, RE::BGSSoundCategory* akSoundCategory) {
    if (!akSoundCategory) {
        logger::error("{}: error, akSoundCategory doesn't exist", __func__);
        return;
    }

    if (!akSoundDescriptor) {
        logger::error("{}: error, akSoundDescriptor doesn't exist", __func__);
        return;
    }

    if (!akSoundDescriptor->soundDescriptor) {
        logger::error("{}: error, akSoundDescriptor->soundDescriptor doesn't exist", __func__);
        return;
    }

    akSoundDescriptor->soundDescriptor->category = akSoundCategory;
}

float GetSoundCategoryVolume(RE::StaticFunctionTag*, RE::BGSSoundCategory* akCategory) {
    if (!akCategory) {
        logger::error("{}: error, akCategory doesn't exist", __func__);
        return -1.0;
    }

    return akCategory->GetCategoryVolume();
}

float GetSoundCategoryFrequency(RE::StaticFunctionTag*, RE::BGSSoundCategory* akCategory) {
    if (!akCategory) {
        logger::error("{}: error, akCategory doesn't exist", __func__);
        return -1.0;
    }

    return akCategory->GetCategoryFrequency();
}

//menu mode timer ===================================================================================================================================

void EraseFinishedMenuModeTimers();
bool erasingMenuModeTimers = false;
RE::BSFixedString sMenuModeTimerEvent = "OnTimerMenuMode";

struct MenuModeTimer {
    std::chrono::system_clock::time_point startTime;
    RE::VMHandle handle;
    int timerID;
    float interval;
    float savedTimeElapsed = 0.0;
    bool cancelled = false;
    bool finished = false;

    MenuModeTimer(RE::VMHandle akHandle, float afInterval, int aiTimerID, float afSavedTimeElapsed = 0.0) {
        handle = akHandle;
        timerID = aiTimerID;
        interval = afInterval;
        savedTimeElapsed = afSavedTimeElapsed;

        std::thread t([=]() {
            startTime = std::chrono::system_clock::now();

            int milliSecondInterval = (interval * 1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(milliSecondInterval));

            if (!cancelled) {
                float elapsedTime = (savedTimeElapsed + timePointDiffToFloat(std::chrono::system_clock::now(), startTime));
                auto* args = RE::MakeFunctionArguments((int)timerID);
                svm->SendAndRelayEvent(handle, &sMenuModeTimerEvent, args, nullptr);
                delete args;
                logger::debug("menu mode timer event sent. ID[{}], interval[{}], elapsed time[{}]", timerID, interval, elapsedTime);
            }

            //while (erasingMenuModeTimers) { //only 1 thread should call EraseFinishedMenuModeTimers at a time.
            //    std::this_thread::sleep_for(std::chrono::milliseconds(150));
            //}

            finished = true;
            EraseFinishedMenuModeTimers();
            });
        t.detach();
    }

    float GetElapsedTime() {
        return (savedTimeElapsed + timePointDiffToFloat(std::chrono::system_clock::now(), startTime));
    }

    float GetCurrentElapsedTime() { //for current - after loading a save startTime and interval are reset.
        return timePointDiffToFloat(std::chrono::system_clock::now(), startTime);
    }

    float GetTimeLeft() {
        float timeLeft = (interval - GetCurrentElapsedTime());
        if (timeLeft < 0.0) {
            timeLeft = 0.0;
        }
        return (timeLeft);
    }
};

std::vector<MenuModeTimer*> currentMenuModeTimers;

void EraseFinishedMenuModeTimers_B() {
    if (currentMenuModeTimers.size() == 0) {
        return;
    }

    erasingMenuModeTimers = true;

    for (auto it = currentMenuModeTimers.begin(); it != currentMenuModeTimers.end(); ) {
        auto* timer = *it;
        if (timer) {
            if (timer->finished) {
                delete timer;
                timer = nullptr;
                it = currentMenuModeTimers.erase(it);
                logger::debug("erased menuModeTimer, Timers left = {}", currentMenuModeTimers.size());
            }
            else {
                ++it;
            }
        }
        else { //safety, this shouldn't occure.
            it = currentMenuModeTimers.erase(it);
        }
    }
    erasingMenuModeTimers = false;
}

void EraseFinishedMenuModeTimers() {
    if (currentMenuModeTimers.size() == 0) {
        return;
    }

    erasingMenuModeTimers = true;

    for (auto* timer : currentMenuModeTimers) {
        if (timer) {
            if (timer->finished) {
                RemoveFromVectorByValue(currentMenuModeTimers, timer);
                delete timer;
                timer = nullptr;
                logger::info("erased menuModeTimer, Timers left = {}", currentMenuModeTimers.size());
            }
        }
        else {
            RemoveFromVectorByValue(currentMenuModeTimers, timer);
        }
    }
    erasingMenuModeTimers = false;
}

MenuModeTimer* GetTimer(std::vector<MenuModeTimer*>& v, RE::VMHandle akHandle, int aiTimerID) {
    if (v.size() == 0) {
        return nullptr;
    }

    for (auto* timer : v) {
        if (timer) {
            if (!timer->cancelled && !timer->finished) {
                if (timer->handle == akHandle && timer->timerID == aiTimerID) {
                    return timer;
                }
            }
        }
    }
    return nullptr;
}

bool SaveTimers(std::vector<MenuModeTimer*> &v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    if (!a_intfc->OpenRecord(record, 1)) {
        logger::error("{}: MenuModeTimers Failed to open record[{}]", __func__, record);
        return false;
    }

    const std::size_t size = v.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: record[{}] Failed to write size of MenuModeTimers!", __func__, record);
        return false;
    }

    for (auto* timer : v) {
        if (timer) {
            float timeLeft = timer->GetTimeLeft();
            float timeElapsed = timer->GetElapsedTime();

            if (!a_intfc->WriteRecordData(timeLeft)) {
                logger::error("{}: record[{}] Failed to write time left", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timeElapsed)) {
                logger::error("{}: record[{}] Failed to write time elapsed", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->handle)) {
                logger::error("{}: record[{}] Failed to write handle[{}]", __func__, record, timer->handle);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->timerID)) {
                logger::error("{}: record[{}] Failed to write timerID[{}]", __func__, record, timer->timerID);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->cancelled)) {
                logger::error("{}: record[{}] Failed to write cancelled[{}]", __func__, record, timer->cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->finished)) {
                logger::error("{}: record[{}] Failed to write finished[{}]", __func__, record, timer->finished);
                return false;
            }

            logger::debug("MenuModeTimer saved : timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
                timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
        }
        else {
            float noTimerTimeLeft = -1.0;
            float noTimerTimeElapsed = 0.0;
            RE::VMHandle handle;
            int ID = -1;
            bool cancelled = true;
            bool finished = true;

            if (!a_intfc->WriteRecordData(noTimerTimeLeft)) {
                logger::error("{}: record[{}] Failed to write no Timer Time Left", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(noTimerTimeElapsed)) {
                logger::error("{}: record[{}] Failed to write no Timer Time Elapsed", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("{}: record[{}] Failed to write no timer handle", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(ID)) {
                logger::error("{}: record[{}] Failed to write no timer Id", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(cancelled)) {
                logger::error("{}: record[{}] Failed to write no timer cancelled[{}]", __func__, record, cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(finished)) {
                logger::error("{}: record[{}] Failed to write no timer finished[{}]", __func__, record, finished);
                return false;
            }
        }
    }
    return true;
}

bool loadTimers(std::vector<MenuModeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::size_t size;
    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record[{}] Failed to load size of MenuModeTimers!", __func__, record);
        return false;
    }

    if (v.size() > 0) { //cancel current timers
        for (auto* timer : v) {
            if (timer) {
                timer->cancelled = true;
            }
        }
    }

    for (std::size_t i = 0; i < size; i++) {
        float timeLeft = -1.0;
        float timeElapsed = 0.0;
        RE::VMHandle handle;
        int ID = -1;
        bool cancelled = false;
        bool finished = false;

        if (!a_intfc->ReadRecordData(timeLeft)) {
            logger::error("{}: {}: MenuModeTimer Failed to load timeLeft!", __func__, i);
            return false;
        }
        
        if (!a_intfc->ReadRecordData(timeElapsed)) {
            logger::error("{}: {}: MenuModeTimer Failed to load timeElapsed!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: {}: MenuModeTimer Failed to load handle!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(ID)) {
            logger::error("{}: {}: MenuModeTimer Failed to load ID!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(cancelled)) {
            logger::error("{}: {}: MenuModeTimer Failed to load cancelled!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(finished)) {
            logger::error("{}: {}: MenuModeTimer Failed to load finished!", __func__, i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: {}: MenuModeTimer warning, failed to resolve handle[{}]", __func__, i, handle);
            continue;
        }

        if (cancelled || finished) {
            continue;
        }

        MenuModeTimer* timer = new MenuModeTimer(handle, timeLeft, ID, timeElapsed);
        v.push_back(timer);

        logger::debug("MenuModeTimer loaded: timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
            timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
    }
    return true;
}

//NoMenuModeTimer =========================================================================================================================================
void EraseFinishedNoMenuModeTimers();
bool erasingNoMenuModeTimers = false;
RE::BSFixedString sNoMenuModeTimerEvent = "OnTimerNoMenuMode";

struct NoMenuModeTimer {
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point lastMenuCheck;
    bool lastMenuCheckSet = false;
    float initInterval;
    float currentInterval;
    float totalTimePaused;
    float savedTimeElapsed;
    RE::VMHandle handle;
    int timerID;
    bool cancelled = false;
    bool started = false;
    bool finished = false;
    bool canDelete = false;

    NoMenuModeTimer(RE::VMHandle akHandle, float afInterval, int aitimerID, float afSavedTimeElapsed = 0.0) {
        startTime = std::chrono::system_clock::now();
        initInterval = afInterval;
        currentInterval = afInterval;
        totalTimePaused = 0.0;
        savedTimeElapsed = afSavedTimeElapsed;
        handle = akHandle;
        timerID = aitimerID;
        if (!inMenuMode) {
            StartTimer();
        }
        else {
            lastMenuCheckSet = true;
            lastMenuCheck = std::chrono::system_clock::now();
        }
    }

    void StartTimer() {
        started = true;

        if (lastMenuCheckSet) {
            lastMenuCheckSet = false;
            totalTimePaused += timePointDiffToFloat(std::chrono::system_clock::now(), lastMenuCheck);
        }

        std::thread t([=]() {
            int milliSecondInterval = (currentInterval * 1000);
            currentInterval = 0.0;
            std::this_thread::sleep_for(std::chrono::milliseconds(milliSecondInterval));

            if (!cancelled) {
                if (inMenuMode) {
                    started = false;
                    float timeInMenu = timePointDiffToFloat(std::chrono::system_clock::now(), lastTimeMenuWasOpened);
                    currentInterval += timeInMenu;
                    totalTimePaused += timeInMenu;
                    lastMenuCheckSet = true;
                    lastMenuCheck = std::chrono::system_clock::now();
                }

                if (currentInterval <= 0.0 && !cancelled) {
                    float elapsedTime = (savedTimeElapsed + (timePointDiffToFloat(std::chrono::system_clock::now(), startTime) - totalTimePaused));
                    auto* args = RE::MakeFunctionArguments((int)timerID);
                    svm->SendAndRelayEvent(handle, &sNoMenuModeTimerEvent, args, nullptr);
                    delete args;
                    logger::debug("NoMenuModeTimer event sent: timerID[{}] initInterval[{}] totalTimePaused[{}] elapsedTime[{}]",
                        timerID, initInterval, totalTimePaused, elapsedTime);

                    finished = true;
                }
                else if (!inMenuMode && !cancelled) {
                    StartTimer();
                }
                else if (!cancelled) {
                    started = false;
                }
            }

            if (cancelled || finished) {
                //while (erasingNoMenuModeTimers) { //only 1 thread should call EraseFinishedNoMenuModeTimers at a time.
                //    std::this_thread::sleep_for(std::chrono::milliseconds(150));
                //}

                canDelete = true;
                EraseFinishedNoMenuModeTimers();
            }
            });
        t.detach();
    }

    float GetElapsedTime() {
        return (savedTimeElapsed + (timePointDiffToFloat(std::chrono::system_clock::now(), startTime) - totalTimePaused));
    }

    float GetCurrentElapsedTime() {
        return (timePointDiffToFloat(std::chrono::system_clock::now(), startTime) - totalTimePaused);
    }

    float GetTimeLeft() {
        float timeLeft = initInterval - GetCurrentElapsedTime();
        if (timeLeft < 0.0) {
            timeLeft = 0.0;
        }
        return (timeLeft);
    }
};

std::vector<NoMenuModeTimer*> currentNoMenuModeTimers;

//called after menu close event if game not paused
void UpdateNoMenuModeTimers(float timeElapsedWhilePaused) {
    if (currentNoMenuModeTimers.size() == 0) {
        return;
    }

    for (auto* noMenuModeTimer : currentNoMenuModeTimers) {
        if (noMenuModeTimer) {
            if (!noMenuModeTimer->cancelled && !noMenuModeTimer->finished) {
                if (noMenuModeTimer->started) {
                    noMenuModeTimer->currentInterval += timeElapsedWhilePaused;
                    noMenuModeTimer->totalTimePaused += timeElapsedWhilePaused;
                }
                else {
                    noMenuModeTimer->StartTimer();
                }
            }
        }
    }
}

void EraseFinishedNoMenuModeTimers_B() {
    if (currentNoMenuModeTimers.size() == 0) {
        return;
    }

    erasingNoMenuModeTimers = true;

    for (auto it = currentNoMenuModeTimers.begin(); it != currentNoMenuModeTimers.end(); ) {
        auto* noMenuModeTimer = *it;
        if (noMenuModeTimer) {
            if (noMenuModeTimer->canDelete) {
                delete noMenuModeTimer;
                noMenuModeTimer = nullptr;
                it = currentNoMenuModeTimers.erase(it);
                logger::debug("erased NoMenuModeTimer, NoMenuModeTimers left = {}", currentNoMenuModeTimers.size());
            }
            else {
                ++it;
            }
        }
        else { //safety, this shouldn't occure.
            it = currentNoMenuModeTimers.erase(it);
        }
    }
    erasingNoMenuModeTimers = false;
}

void EraseFinishedNoMenuModeTimers() {
    if (currentNoMenuModeTimers.size() == 0) {
        return;
    }

    erasingNoMenuModeTimers = true;

    for (auto* timer : currentNoMenuModeTimers) {
        if (timer) {
            if (timer->canDelete) {
                RemoveFromVectorByValue(currentNoMenuModeTimers, timer);
                delete timer;
                timer = nullptr;
                logger::debug("erased NoMenuModeTimer, Timers left = {}", currentNoMenuModeTimers.size());
            }
        }
        else {
            RemoveFromVectorByValue(currentNoMenuModeTimers, timer);
        }
    }
    erasingNoMenuModeTimers = false;
}

NoMenuModeTimer* GetTimer(std::vector<NoMenuModeTimer*>& v, RE::VMHandle akHandle, int aiTimerID) {
    if (v.size() == 0) {
        return nullptr;
    }

    for (auto* noMenuModeTimer : v) {
        if (noMenuModeTimer) {
            if (!noMenuModeTimer->cancelled && !noMenuModeTimer->finished) {
                if (noMenuModeTimer->handle == akHandle && noMenuModeTimer->timerID == aiTimerID) {
                    return noMenuModeTimer;
                }
            }
        }
    }
    return nullptr;
}

bool SaveTimers(std::vector<NoMenuModeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    if (!a_intfc->OpenRecord(record, 1)) {
        logger::error("{}: MenuModeTimers Failed to open record[{}]", __func__, record);
        return false;
    }

    const std::size_t size = v.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: record[{}] Failed to write size of MenuModeTimers!", __func__, record);
        return false;
    }

    for (auto* timer : v) {
        if (timer) {
            float timeLeft = timer->GetTimeLeft();
            float timeElapsed = timer->GetElapsedTime();

            if (!a_intfc->WriteRecordData(timeLeft)) {
                logger::error("{}: record[{}] Failed to write time left", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timeElapsed)) {
                logger::error("{}: record[{}] Failed to write time elapsed", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->handle)) {
                logger::error("{}: record[{}] Failed to write handle[{}]", __func__, record, timer->handle);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->timerID)) {
                logger::error("{}: record[{}] Failed to write timerID[{}]", __func__, record, timer->timerID);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->cancelled)) {
                logger::error("{}: record[{}] Failed to write cancelled[{}]", __func__, record, timer->cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->finished)) {
                logger::error("{}: record[{}] Failed to write finished[{}]", __func__, record, timer->finished);
                return false;
            }

            logger::debug("NoMenuModeTimer saved : timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
                timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
        }
        else {
            float noTimerTimeLeft = -1.0;
            float noTimerTimeElapsed = 0.0;
            RE::VMHandle handle;
            int ID = -1;
            bool cancelled = true;
            bool finished = true;

            if (!a_intfc->WriteRecordData(noTimerTimeLeft)) {
                logger::error("{}: record[{}] Failed to write no Timer Time Left", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(noTimerTimeElapsed)) {
                logger::error("{}: record[{}] Failed to write no Timer Time Elapsed", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("{}: record[{}] Failed to write no timer handle", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(ID)) {
                logger::error("{}: record[{}] Failed to write no timer Id", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(cancelled)) {
                logger::error("{}: record[{}] Failed to write no timer cancelled[{}]", __func__, record, cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(finished)) {
                logger::error("{}: record[{}] Failed to write no timer finished[{}]", __func__, record, finished);
                return false;
            }
        }
    }
    return true;
}

bool loadTimers(std::vector<NoMenuModeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::size_t size;
    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record[{}] Failed to load size of MenuModeTimers!", __func__, record);
        return false;
    }

    if (v.size() > 0) { //cancel current timers
        for (auto* timer : v) {
            if (timer) {
                timer->cancelled = true;
            }
        }
    }

    for (std::size_t i = 0; i < size; i++) {
        float timeLeft = -1.0;
        float timeElapsed = 0.0;
        RE::VMHandle handle;
        int ID = -1;
        bool cancelled = false;
        bool finished = false;

        if (!a_intfc->ReadRecordData(timeLeft)) {
            logger::error("{}: {}: MenuModeTimer Failed to load timeLeft!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(timeElapsed)) {
            logger::error("{}: {}: MenuModeTimer Failed to load timeElapsed!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: {}: MenuModeTimer Failed to load handle!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(ID)) {
            logger::error("{}: {}: MenuModeTimer Failed to load ID!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(cancelled)) {
            logger::error("{}: {}: MenuModeTimer Failed to load cancelled!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(finished)) {
            logger::error("{}: {}: MenuModeTimer Failed to load finished!", __func__, i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: {}: MenuModeTimer warning, failed to resolve handle[{}]", __func__, i, handle);
            continue;
        }

        if (cancelled || finished) {
            continue;
        }

        NoMenuModeTimer* timer = new NoMenuModeTimer(handle, timeLeft, ID, timeElapsed);
        v.push_back(timer);

        logger::debug("NoMenuModeTimer loaded: timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
            timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
    }
    return true;
}

//timer ===================================================================================================================================

void EraseFinishedTimers();
bool erasingTimers = false;
RE::BSFixedString sTimerEvent = "OnTimer";

struct Timer {
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point lastPausedTimeCheck;
    bool lastPausedTimeCheckSet = false;
    float initInterval;
    float currentInterval;
    float totalTimePaused;
    float savedTimeElapsed;
    RE::VMHandle handle;
    int timerID;
    bool cancelled = false;
    bool started = false;
    bool finished = false;
    bool canDelete = false;

    Timer(RE::VMHandle akHandle, float afInterval, int aiTimerID, float afSavedTimeElapsed = 0.0) {
        startTime = std::chrono::system_clock::now();
        initInterval = afInterval;
        currentInterval = afInterval;
        totalTimePaused = 0.0;
        savedTimeElapsed = afSavedTimeElapsed;
        handle = akHandle;
        timerID = aiTimerID;
        if (!ui->GameIsPaused()) {
            StartTimer();
        }
        else {
            lastPausedTimeCheckSet = true; 
            lastPausedTimeCheck = std::chrono::system_clock::now();
        }
    }

    void StartTimer() {
        started = true;

        if (lastPausedTimeCheckSet) {
            lastPausedTimeCheckSet = false;
            totalTimePaused += timePointDiffToFloat(std::chrono::system_clock::now(), lastPausedTimeCheck);
        }

        std::thread t([=]() {
            int milliSecondInterval = (currentInterval * 1000);
            currentInterval = 0.0;
            std::this_thread::sleep_for(std::chrono::milliseconds(milliSecondInterval));

            if (!cancelled) {
                if (ui->GameIsPaused()) {
                    started = false;
                    float timeInMenu = timePointDiffToFloat(std::chrono::system_clock::now(), lastTimeGameWasPaused);
                    currentInterval += timeInMenu;
                    totalTimePaused += timeInMenu;
                    lastPausedTimeCheckSet = true;
                    lastPausedTimeCheck = std::chrono::system_clock::now();
                }

                if (currentInterval <= 0.0 && !cancelled) {
                    float elapsedTime = (savedTimeElapsed + (timePointDiffToFloat(std::chrono::system_clock::now(), startTime) - totalTimePaused));
                    auto* args = RE::MakeFunctionArguments((int)timerID);
                    svm->SendAndRelayEvent(handle, &sTimerEvent, args, nullptr);
                    delete args;
                    logger::debug("timer event sent: timerID[{}] initInterval[{}] totalTimePaused[{}] elapsedTime[{}]",
                        timerID, initInterval, totalTimePaused, elapsedTime);

                    finished = true;
                }
                else if (!ui->GameIsPaused() && !cancelled) {
                    StartTimer();
                }
                else if (!cancelled) {
                    started = false;
                }
            }

            if (cancelled || finished) {
                //while (erasingTimers) { //only 1 thread should call EraseFinishedTimers at a time.
                //    std::this_thread::sleep_for(std::chrono::milliseconds(150));
                //}

                canDelete = true;
                EraseFinishedTimers();
            }
            });
        t.detach();
    }

    float GetElapsedTime() {
        return (savedTimeElapsed + (timePointDiffToFloat(std::chrono::system_clock::now(), startTime) - totalTimePaused));
    }

    float getCurrentElapsedTime() {
        return (timePointDiffToFloat(std::chrono::system_clock::now(), startTime) - totalTimePaused);
    }

    float GetTimeLeft() {
        float timeLeft = initInterval - getCurrentElapsedTime();
        if (timeLeft < 0.0) {
            timeLeft = 0.0;
        }
        return (timeLeft);
    }

    void CancelTimer() {
        cancelled = true;
    }
};

std::vector<Timer*> currentTimers;

//called after menu close event if game not paused
void UpdateTimers(float timeElapsedWhilePaused) {
    if (currentTimers.size() == 0) {
        return;
    }

    for (auto* timer : currentTimers) {
        if (timer) {
            if (!timer->cancelled && !timer->finished) {
                if (timer->started) {
                    timer->currentInterval += timeElapsedWhilePaused;
                    timer->totalTimePaused += timeElapsedWhilePaused;
                }
                else {
                    timer->StartTimer();
                }
            }
        }
    }
}

void EraseFinishedTimers_B() {
    if (currentTimers.size() == 0) {
        return;
    }

    erasingTimers = true;

    for (auto it = currentTimers.begin(); it != currentTimers.end(); ) {
        auto* timer = *it;
        if (timer) {
            if (timer->canDelete) {
                delete timer;
                timer = nullptr;
                it = currentTimers.erase(it);
                logger::debug("erased Timer, Timers left = {}", currentTimers.size());
            }
            else {
                ++it;
            }
        }
        else { //safety, this shouldn't occure.
            it = currentTimers.erase(it);
        }
    }
    erasingTimers = false;
}

void EraseFinishedTimers() {
    if (currentTimers.size() == 0) {
        return;
    }

    erasingTimers = true;

    for (auto* timer : currentTimers) {
        if (timer) {
            if (timer->canDelete) {
                RemoveFromVectorByValue(currentTimers, timer);
                delete timer;
                timer = nullptr;
                logger::debug("erased timer, Timers left = {}", currentTimers.size());
            }
        }
        else {
            RemoveFromVectorByValue(currentTimers, timer);
        }
    }
    erasingTimers = false;
}

Timer* GetTimer(std::vector<Timer*>& v, RE::VMHandle akHandle, int aiTimerID) {
    if (v.size() == 0) {
        return nullptr;
    }

    for (auto* timer : v) {
        if (timer) {
            if (!timer->cancelled && !timer->finished) {
                if (timer->handle == akHandle && timer->timerID == aiTimerID) {
                    return timer;
                }
            }
        }
    }
    return nullptr;
}

bool SaveTimers(std::vector<Timer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    if (!a_intfc->OpenRecord(record, 1)) {
        logger::error("{}: MenuModeTimers Failed to open record[{}]", __func__, record);
        return false;
    }

    const std::size_t size = v.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: record[{}] Failed to write size of MenuModeTimers!", __func__, record);
        return false;
    }

    for (auto* timer : v) {
        if (timer) {
            float timeLeft = timer->GetTimeLeft();
            float timeElapsed = timer->GetElapsedTime();

            if (!a_intfc->WriteRecordData(timeLeft)) {
                logger::error("{}: record[{}] Failed to write time left", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timeElapsed)) {
                logger::error("{}: record[{}] Failed to write time elapsed", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->handle)) {
                logger::error("{}: record[{}] Failed to write handle[{}]", __func__, record, timer->handle);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->timerID)) {
                logger::error("{}: record[{}] Failed to write timerID[{}]", __func__, record, timer->timerID);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->cancelled)) {
                logger::error("{}: record[{}] Failed to write cancelled[{}]", __func__, record, timer->cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->finished)) {
                logger::error("{}: record[{}] Failed to write finished[{}]", __func__, record, timer->finished);
                return false;
            }

            logger::debug("Timer saved : timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
                timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
        }
        else {
            float noTimerTimeLeft = -1.0;
            float noTimerTimeElapsed = 0.0;
            RE::VMHandle handle;
            int ID = -1;
            bool cancelled = true;
            bool finished = true;

            if (!a_intfc->WriteRecordData(noTimerTimeLeft)) {
                logger::error("{}: record[{}] Failed to write no Timer Time Left", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(noTimerTimeElapsed)) {
                logger::error("{}: record[{}] Failed to write no Timer Time Elapsed", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("{}: record[{}] Failed to write no timer handle", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(ID)) {
                logger::error("{}: record[{}] Failed to write no timer Id", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(cancelled)) {
                logger::error("{}: record[{}] Failed to write no timer cancelled[{}]", __func__, record, cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(finished)) {
                logger::error("{}: record[{}] Failed to write no timer finished[{}]", __func__, record, finished);
                return false;
            }
        }
    }
    return true;
}

bool loadTimers(std::vector<Timer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::size_t size;
    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record[{}] Failed to load size of MenuModeTimers!", __func__, record);
        return false;
    }

    if (v.size() > 0) { //cancel current timers
        for (auto* timer : v) {
            if (timer) {
                timer->cancelled = true;
            }
        }
    }

    for (std::size_t i = 0; i < size; i++) {
        float timeLeft = -1.0;
        float timeElapsed = 0.0;
        RE::VMHandle handle;
        int ID = -1;
        bool cancelled = false;
        bool finished = false;

        if (!a_intfc->ReadRecordData(timeLeft)) {
            logger::error("{}: {}: MenuModeTimer Failed to load timeLeft!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(timeElapsed)) {
            logger::error("{}: {}: MenuModeTimer Failed to load timeElapsed!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: {}: MenuModeTimer Failed to load handle!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(ID)) {
            logger::error("{}: {}: MenuModeTimer Failed to load ID!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(cancelled)) {
            logger::error("{}: {}: MenuModeTimer Failed to load cancelled!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(finished)) {
            logger::error("{}: {}: MenuModeTimer Failed to load finished!", __func__, i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: {}: MenuModeTimer warning, failed to resolve handle[{}]", __func__, i, handle);
            continue;
        }

        if (cancelled || finished) {
            continue;
        }

        Timer* timer = new Timer(handle, timeLeft, ID, timeElapsed);
        v.push_back(timer);

        logger::debug("Timer loaded: timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
            timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
    }
    return true;
}

//GameTimeTimer =========================================================================================================================================

void EraseFinishedGameTimers();
bool erasingGameTimers = false;
RE::BSFixedString sGameTimeTimerEvent = "OnTimerGameTime";

struct GameTimeTimer {
    float startTime;
    float endTime;
    float initGameHoursInterval;
    int tick;
    int timerID;
    RE::VMHandle handle;
    bool cancelled = false;
    bool started = false;
    bool finished = false;
    bool canDelete = false;

    GameTimeTimer(RE::VMHandle akHandle, float afInterval, int aiTimerID, float afStartTime = -1.0, float afEndTime = -1.0) {
        handle = akHandle;
        timerID = aiTimerID;
        initGameHoursInterval = afInterval;

        tick = gameTimerPollingInterval;

        if (afStartTime != -1.0) { //timer created from LoadTimers on game load.
            startTime = afStartTime;
            endTime = afEndTime;
        }
        else {
            startTime = calendar->GetHoursPassed();
            endTime = startTime + afInterval;
        }
        
        float secondsInterval = GameHoursToRealTimeSeconds(nullptr, afInterval);
        int milliTick = (secondsInterval * 1000);

        if (tick > milliTick) {
            tick = milliTick;
        }

        StartTimer();
    }

    void StartTimer() {
        std::thread t([=]() {
            while (calendar->GetHoursPassed() < endTime && !cancelled) {
                std::this_thread::sleep_for(std::chrono::milliseconds(tick));
            }

            if (!cancelled) {
                auto now = std::chrono::system_clock::now();
                float endTime = calendar->GetHoursPassed();
                float elapsedGameHours = (calendar->GetHoursPassed() - startTime);

                auto* args = RE::MakeFunctionArguments((int)timerID);
                svm->SendAndRelayEvent(handle, &sGameTimeTimerEvent, args, nullptr);
                delete args;
                logger::debug("game timer event sent. ID[{}] gameHoursInterval[{}] startTime[{}] endTime[{}] elapsedGameHours[{}]",
                    timerID, initGameHoursInterval, startTime, endTime, elapsedGameHours);

                finished = true;
            }

            if (cancelled || finished) {
                //while (erasingGameTimers) { //only 1 thread should call EraseFinishedTimers at a time.
                //    std::this_thread::sleep_for(std::chrono::milliseconds(150));
                //}

                canDelete = true;
                EraseFinishedGameTimers();
            }
            });
        t.detach();
    }

    float GetElapsedTime() {
        return (calendar->GetHoursPassed() - startTime);
    }

    float GetTimeLeft() {
        return (endTime - calendar->GetHoursPassed());
    }

    void CancelTimer() {
        cancelled = true;
    }
};

std::vector<GameTimeTimer*> currentGameTimeTimers;

void EraseFinishedGameTimers_B() {
    if (currentGameTimeTimers.size() == 0) {
        return;
    }

    erasingGameTimers = true;

    for (auto it = currentGameTimeTimers.begin(); it != currentGameTimeTimers.end(); ) {
        auto* timer = *it;
        if (timer) {
            if (timer->canDelete) {
                delete timer;
                timer = nullptr;
                it = currentGameTimeTimers.erase(it);
                logger::debug("erased gameTimer, Timers left = {}", currentGameTimeTimers.size());
            }
            else {
                ++it;
            }
        }
        else { //safety, this shouldn't occure.
            it = currentGameTimeTimers.erase(it);
        }
    }
    erasingGameTimers = false;
}

void EraseFinishedGameTimers() {
    if (currentGameTimeTimers.size() == 0) {
        return;
    }

    erasingGameTimers = true;

    for (auto* timer : currentGameTimeTimers) {
        if (timer) {
            if (timer->canDelete) {
                RemoveFromVectorByValue(currentGameTimeTimers, timer);
                delete timer;
                timer = nullptr;
                logger::debug("erased gameTimer, Timers left = {}", currentGameTimeTimers.size());
            }
        }
        else {
            RemoveFromVectorByValue(currentGameTimeTimers, timer);
        }
    }
    erasingGameTimers = false;
}

GameTimeTimer* GetTimer(std::vector<GameTimeTimer*>& v, RE::VMHandle akHandle, int aiTimerID) {
    if (v.size() == 0) {
        return nullptr;
    }

    for (auto* timer : v) {
        if (timer) {
            if (!timer->cancelled && !timer->finished) {
                if (timer->handle == akHandle && timer->timerID == aiTimerID) {
                    return timer;
                }
            }
        }
    }
    return nullptr;
}

bool SaveTimers(std::vector<GameTimeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    if (!a_intfc->OpenRecord(record, 1)) {
        logger::error("{}: MenuModeTimers Failed to open record[{}]", __func__, record);
        return false;
    }

    const std::size_t size = v.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: record[{}] Failed to write size of MenuModeTimers!", __func__, record);
        return false;
    }

    for (auto* timer : v) {
        if (timer) {
            
            if (!a_intfc->WriteRecordData(timer->startTime)) {
                logger::error("{}: record[{}] Failed to write startTime", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->endTime)) {
                logger::error("{}: record[{}] Failed to write endTime", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->initGameHoursInterval)) {
                logger::error("{}: record[{}] Failed to write initGameHoursInterval", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->handle)) {
                logger::error("{}: record[{}] Failed to write handle[{}]", __func__, record, timer->handle);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->timerID)) {
                logger::error("{}: record[{}] Failed to write timerID[{}]", __func__, record, timer->timerID);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->cancelled)) {
                logger::error("{}: record[{}] Failed to write cancelled[{}]", __func__, record, timer->cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->finished)) {
                logger::error("{}: record[{}] Failed to write finished[{}]", __func__, record, timer->finished);
                return false;
            }

            logger::debug("gameTimer saved : startTime[{}], endTime[{}], initGameHoursInterval[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
                timer->startTime, timer->endTime, timer->initGameHoursInterval, timer->handle, timer->timerID, timer->cancelled, timer->finished);
        }
        else {
            float startTime = 0.0;
            float endTime = 0.0;
            float initGameHoursInterval = 0.1;
            RE::VMHandle handle;
            int ID = -1;
            bool cancelled = true;
            bool finished = true;

            if (!a_intfc->WriteRecordData(startTime)) {
                logger::error("{}: record[{}] Failed to write no Timer startTime", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(endTime)) {
                logger::error("{}: record[{}] Failed to write no Timer endTime", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(initGameHoursInterval)) {
                logger::error("{}: record[{}] Failed to write no Timer initGameHoursInterval", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("{}: record[{}] Failed to write no timer handle", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(ID)) {
                logger::error("{}: record[{}] Failed to write no timer Id", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(cancelled)) {
                logger::error("{}: record[{}] Failed to write no timer cancelled[{}]", __func__, record, cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(finished)) {
                logger::error("{}: record[{}] Failed to write no timer finished[{}]", __func__, record, finished);
                return false;
            }
        }
    }
    return true;
}

bool loadTimers(std::vector<GameTimeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::size_t size;
    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record[{}] Failed to load size of MenuModeTimers!", __func__, record);
        return false;
    }

    if (v.size() > 0) { //cancel current timers
        for (auto* timer : v) {
            if (timer) {
                timer->cancelled = true;
            }
        }
    }

    for (std::size_t i = 0; i < size; i++) {
        float startTime = 0.0;
        float endTime = 0.0;
        float initGameHoursInterval = 0.1;
        RE::VMHandle handle;
        int ID = -1;
        bool cancelled = true;
        bool finished = true;

        if (!a_intfc->ReadRecordData(startTime)) {
            logger::error("{}: {}: MenuModeTimer Failed to load startTime!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(endTime)) {
            logger::error("{}: {}: MenuModeTimer Failed to load endTime!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(initGameHoursInterval)) {
            logger::error("{}: {}: MenuModeTimer Failed to load initGameHoursInterval!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: {}: MenuModeTimer Failed to load handle!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(ID)) {
            logger::error("{}: {}: MenuModeTimer Failed to load ID!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(cancelled)) {
            logger::error("{}: {}: MenuModeTimer Failed to load cancelled!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(finished)) {
            logger::error("{}: {}: MenuModeTimer Failed to load finished!", __func__, i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: {}: MenuModeTimer warning, failed to resolve handle[{}]", __func__, i, handle);
            continue;
        }

        if (cancelled || finished) {
            continue;
        }

         GameTimeTimer* timer = new GameTimeTimer(handle, initGameHoursInterval, ID, startTime, endTime);
        v.push_back(timer);

        logger::debug("gameTimer loaded: startTime[{}], endTime[{}], initGameHoursInterval[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
            timer->startTime, timer->endTime, timer->initGameHoursInterval, timer->handle, timer->timerID, timer->cancelled, timer->finished);
    }
    return true;
}

//timer papyrus functions==============================================================================================================================================

//forms ==============================================================================================================================================

//menuModeTimer=======================================================================================================================
void StartMenuModeTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, float afInterval, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
        logger::info("{} reset timer on form [{}] ID[{:x}] timerID[{}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID(), aiTimerID);
    }

    MenuModeTimer* newTimer = new MenuModeTimer(handle, afInterval, aiTimerID);
    currentMenuModeTimers.push_back(newTimer);
}

void CancelMenuModeTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::info("{} called, ID {}", __func__, aiTimerID);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
}

float GetTimeElapsedOnMenuModeTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnMenuModeTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//NoMenuModeTimer=======================================================================================================================
void StartNoMenuModeTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, float afInterval, int aiTimerID) {
    logger::info("{} called, time: {}", __func__, afInterval);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    NoMenuModeTimer* newTimer = new NoMenuModeTimer(handle, afInterval, aiTimerID);
    currentNoMenuModeTimers.push_back(newTimer);
}

void CancelNoMenuModeTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnNoMenuModeTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnNoMenuModeTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer=======================================================================================================================
void StartTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, float afInterval, int aiTimerID) {
    logger::info("{} called, time: {}", __func__, afInterval);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    Timer* newTimer = new Timer(handle, afInterval, aiTimerID);
    currentTimers.push_back(newTimer);
}

void CancelTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//gametime timer===========================================================================================================================
void StartGameTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, float afInterval, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    GameTimeTimer* newTimer = new GameTimeTimer(handle, afInterval, aiTimerID);
    currentGameTimeTimers.push_back(newTimer);
}

void CancelGameTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnGameTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnGameTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer Alias==============================================================================================================================================

//menu Mode Timer alias=======================================================================================================================
void StartMenuModeTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, float afInterval, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
        logger::info("{} reset timer on Alias [{}] ID[{:x}] timerID[{}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID, aiTimerID);
    }

    MenuModeTimer* newTimer = new MenuModeTimer(handle, afInterval, aiTimerID);
    currentMenuModeTimers.push_back(newTimer);
}

void CancelMenuModeTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::info("{} called, ID {}", __func__, aiTimerID);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnMenuModeTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnMenuModeTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//NoMenuModeTimer Alias=======================================================================================================================
void StartNoMenuModeTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, float afInterval, int aiTimerID) {
    logger::info("{} called, time: {}", __func__, afInterval);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    NoMenuModeTimer* newTimer = new NoMenuModeTimer(handle, afInterval, aiTimerID);
    currentNoMenuModeTimers.push_back(newTimer);
}

void CancelNoMenuModeTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnNoMenuModeTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnNoMenuModeTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer alias=======================================================================================================================
void StartTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, float afInterval, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    Timer* newTimer = new Timer(handle, afInterval, aiTimerID);
    currentTimers.push_back(newTimer);
}

void CancelTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//gametime timer alias===========================================================================================================================
void StartGameTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, float afInterval, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    GameTimeTimer* newTimer = new GameTimeTimer(handle, afInterval, aiTimerID);
    currentGameTimeTimers.push_back(newTimer);
}

void CancelGameTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnGameTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnGameTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer ActiveMagicEffect==============================================================================================================================================

//menu Mode Timer ActiveMagicEffect=======================================================================================================================
void StartMenuModeTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, float afInterval, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
        logger::info("{} reset timer on ActiveMagicEffect [{}] ID[{:x}] timerID[{}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID, aiTimerID);
    }

    MenuModeTimer* newTimer = new MenuModeTimer(handle, afInterval, aiTimerID);
    currentMenuModeTimers.push_back(newTimer);
}

void CancelMenuModeTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::info("{} called, ID {}", __func__, aiTimerID);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnMenuModeTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}


float GetTimeLeftOnMenuModeTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//NoMenuModeTimer ActiveMagicEffect=======================================================================================================================
void StartNoMenuModeTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, float afInterval, int aiTimerID) {
    logger::info("{} called, time: {}", __func__, afInterval);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    NoMenuModeTimer* newTimer = new NoMenuModeTimer(handle, afInterval, aiTimerID);
    currentNoMenuModeTimers.push_back(newTimer);
}

void CancelNoMenuModeTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnNoMenuModeTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnNoMenuModeTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer ActiveMagicEffect=======================================================================================================================
void StartTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, float afInterval, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    Timer* newTimer = new Timer(handle, afInterval, aiTimerID);
    currentTimers.push_back(newTimer);
}

void CancelTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//gametime timer ActiveMagicEffect===========================================================================================================================
void StartGameTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, float afInterval, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    GameTimeTimer* newTimer = new GameTimeTimer(handle, afInterval, aiTimerID);
    currentGameTimeTimers.push_back(newTimer);
}

void CancelGameTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnGameTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnGameTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::info("{} called", __func__);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
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
    EventEnum_OnWaitStart,
    EventEnum_OnWaitStop,
    EventEnum_OnMagicEffectApply,
    EventEnum_OnSpellCast,
    EventEnum_LockChanged,
    EventEnum_OnOpen,
    EventEnum_OnClose,
    EventEnum_OnWeaponSwing,
    EventEnum_OnActorSpellCast,
    EventEnum_OnActorSpellFire,
    EventEnum_VoiceCast,
    EventEnum_VoiceFire,
    EventEnum_BowDraw,
    EventEnum_BowRelease,
    EventEnum_BeginDraw,
    EventEnum_EndDraw,
    EventEnum_BeginSheathe,
    EventEnum_EndSheathe,
    EventEnum_First = EventEnum_OnLoadGame,
    EventEnum_Last = EventEnum_EndSheathe
};

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

        if (!event) {
            logger::error("hit event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

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

        if (!event) {
            logger::error("combat change event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

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

        if (!event) {
            logger::error("furniture event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

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

        if (!event) {
            logger::error("activate event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

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

        if (!event) {
            logger::error("death / dying event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::info("Death Event");

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
            if (IsBadReadPtr(victimRef, sizeof(victimRef))) {
                victim = nullptr;
                logger::error("death event: bad victimRef pointer");
                return RE::BSEventNotifyControl::kContinue;
            }
            else if (victimRef->GetFormID() == 0) {
                victim = nullptr;
                logger::error("death event: 0 victimRef pointer");
                return RE::BSEventNotifyControl::kContinue;
            }
            else {
                victim = static_cast<RE::Actor*>(victimRef);
                logger::info("death event: valid victimRef pointer");
            }
        }

        RE::Actor* killer;
        if (killerRef != nullptr) {
            //this is necessarry because when using console command kill or script command actor.kill() killer will be a bad ptr and cause ctd.
            if (IsBadReadPtr(killerRef, sizeof(killerRef))) {
                killer = nullptr;
                logger::error("death event: bad killerRef pointer");
            }
            else if (killerRef->GetFormID() == 0) {
                killer = nullptr;
                logger::error("death event: 0 killerRef pointer");
            }
            else {
                killer = static_cast<RE::Actor*>(killerRef);
                logger::info("death event: valid killerRef pointer");
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

        if (!event) {
            logger::error("equip event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }
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

struct WaitStartEventSink : public RE::BSTEventSink<RE::TESWaitStartEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESWaitStartEvent* event, RE::BSTEventSource<RE::TESWaitStartEvent>*/*source*/) {

        if (!event) {
            logger::error("wait start event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::info("Wait Start Event");

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnWaitStart]->globalHandles;

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments();
        SendEvents(handles, eventDataPtrs[EventEnum_OnWaitStart]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct WaitStopEventSink : public RE::BSTEventSink<RE::TESWaitStopEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESWaitStopEvent* event, RE::BSTEventSource<RE::TESWaitStopEvent>*/*source*/) {

        if (!event) {
            logger::error("wait stop event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::info("Wait Stop Event");

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnWaitStop]->globalHandles;

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((bool)event->interrupted);
        SendEvents(handles, eventDataPtrs[EventEnum_OnWaitStop]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct MagicEffectApplyEventSink : public RE::BSTEventSink<RE::TESMagicEffectApplyEvent> { //
    RE::BSEventNotifyControl ProcessEvent(const RE::TESMagicEffectApplyEvent* event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*/*source*/) {

        if (!event) {
            logger::error("MagicEffectApply event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::info("MagicEffectApply Event");

        RE::TESObjectREFR* caster = event->caster.get();
        RE::TESObjectREFR* target = event->target.get();
        RE::EffectSetting* magicEffect = RE::TESForm::LookupByID<RE::EffectSetting>(event->magicEffect);

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnMagicEffectApply]->globalHandles;

        CombineEventHandles(handles, caster, eventDataPtrs[EventEnum_OnMagicEffectApply]->eventParamMaps[0]);
        CombineEventHandles(handles, target, eventDataPtrs[EventEnum_OnMagicEffectApply]->eventParamMaps[1]);
        CombineEventHandles(handles, magicEffect, eventDataPtrs[EventEnum_OnMagicEffectApply]->eventParamMaps[2]);

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)caster, (RE::TESObjectREFR*)target, (RE::EffectSetting*)magicEffect);
        SendEvents(handles, eventDataPtrs[EventEnum_OnMagicEffectApply]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct LockChangedEventSink : public RE::BSTEventSink<RE::TESLockChangedEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESLockChangedEvent* event, RE::BSTEventSource<RE::TESLockChangedEvent>*/*source*/) {

        if (!event) {
            logger::error("Lock Changed event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::info("Lock Changed Event");

        RE::TESObjectREFR* lockedObject = event->lockedObject.get();

        if (!lockedObject) {
            return RE::BSEventNotifyControl::kContinue;
        }

        bool Locked = lockedObject->IsLocked();

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_LockChanged]->globalHandles;
        CombineEventHandles(handles, lockedObject, eventDataPtrs[EventEnum_LockChanged]->eventParamMaps[0]);

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)lockedObject, (bool)Locked);
        SendEvents(handles, eventDataPtrs[EventEnum_LockChanged]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct OpenCloseEventSink : public RE::BSTEventSink<RE::TESOpenCloseEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESOpenCloseEvent* event, RE::BSTEventSource<RE::TESOpenCloseEvent>*/*source*/) {

        if (!event) {
            logger::error("OpenClose event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::info("Open Close Event");


        RE::TESObjectREFR* activatorRef = event->activeRef.get();
        RE::TESObjectREFR* akActionRef = event->ref.get();
        int eventIndex;

        if (event->opened) {
            eventIndex = EventEnum_OnOpen;
        }
        else {
            eventIndex = EventEnum_OnClose;
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->globalHandles;

        CombineEventHandles(handles, activatorRef, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        CombineEventHandles(handles, akActionRef, eventDataPtrs[eventIndex]->eventParamMaps[1]);

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)activatorRef, (RE::TESObjectREFR*)akActionRef);
        SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct SpellCastEventSink : public RE::BSTEventSink<RE::TESSpellCastEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSpellCastEvent* event, RE::BSTEventSource<RE::TESSpellCastEvent>*/*source*/) {

        if (!event) {
            logger::error("spell cast event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::info("spell cast Event");

        RE::TESObjectREFR* caster = event->object;
        RE::TESForm* spell = RE::TESForm::LookupByID(event->spell);

        //logger::info("spell cast: [{}] obj [{}]", GetFormName(spell), GetFormName(caster));

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnSpellCast]->globalHandles;

        CombineEventHandles(handles, caster, eventDataPtrs[EventEnum_OnSpellCast]->eventParamMaps[0]);
        CombineEventHandles(handles, spell, eventDataPtrs[EventEnum_OnSpellCast]->eventParamMaps[1]);

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)caster, (RE::TESForm*)spell);
        SendEvents(handles, eventDataPtrs[EventEnum_OnSpellCast]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

std::vector<std::string> actionSlotStrings{"left", "right", "voice"};
std::vector<std::string> actionTypeStrings{
"kWeaponSwing",
"SpellCast",
"SpellFire",
"VoiceCast",
"VoiceFire",
"BowDraw",
"BowRelease",
"BeginDraw",
"EndDraw",
"BeginSheathe",
"EndSheathe"};

struct ActorActionEventSink : public RE::BSTEventSink<SKSE::ActionEvent> {
    RE::BSEventNotifyControl ProcessEvent(const SKSE::ActionEvent* event, RE::BSTEventSource<SKSE::ActionEvent>*/*source*/) {

        if (!event) {
            logger::error("Action Event event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::info("Action Event");

        RE::Actor* akActor = event->actor;

        //0 = left hand, 1 = right hand, 2 = voice
        int slot = event->slot.underlying();
        RE::TESForm* akSource = event->sourceForm;

        /*  kWeaponSwing = 0,
            kSpellCast = 1,
            kSpellFire = 2,
            kVoiceCast = 3,
            kVoiceFire = 4,
            kBowDraw = 5,
            kBowRelease = 6,
            kBeginDraw = 7,
            kEndDraw = 8,
            kBeginSheathe = 9,
            kEndSheathe = 10*/

        int type = event->type.underlying();

        int eventIndex = -1;

        switch (type) {
        case 0:
            eventIndex = EventEnum_OnWeaponSwing;
            break;
        case 1:
            eventIndex = EventEnum_OnActorSpellCast;
            break;
        case 2:
            eventIndex = EventEnum_OnActorSpellFire;
            break;
        case 3:
            eventIndex = EventEnum_VoiceCast;
            break;
        case 4:
            eventIndex = EventEnum_VoiceFire;
            break;
        case 5:
            eventIndex = EventEnum_BowDraw;
            break;
        case 6:
            eventIndex = EventEnum_BowRelease;
            break;
        case 7:
            eventIndex = EventEnum_BeginDraw;
            break;
        case 8:
            eventIndex = EventEnum_EndDraw;
            break;
        case 9:
            eventIndex = EventEnum_BeginSheathe;
            break;
        case 10:
            eventIndex = EventEnum_EndSheathe;
            break;
        }

        logger::info("action event, Actor[{}] Source[{}]  type[{}] slot[{}]", GetFormName(akActor), GetFormName(akSource),
            actionTypeStrings[type], actionSlotStrings[slot]);

        if (eventIndex == -1) {
            return RE::BSEventNotifyControl::kContinue;
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->globalHandles;

        CombineEventHandles(handles, akActor, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        CombineEventHandles(handles, akSource, eventDataPtrs[eventIndex]->eventParamMaps[1]);

        RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor, (RE::TESForm*)akSource, (int)slot);
        SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

        //draw / sheathe events aren't triggered for left hand. Send left hand events manually
        if (eventIndex >= EventEnum_BeginDraw && eventIndex <= EventEnum_EndSheathe) {
            RE::TESForm* leftHandSource = akActor->GetEquippedObject(true);

            if (leftHandSource) {
                if (leftHandSource != akSource) {
                    std::vector<RE::VMHandle> handlesB = eventDataPtrs[eventIndex]->globalHandles;

                    CombineEventHandles(handlesB, akActor, eventDataPtrs[eventIndex]->eventParamMaps[0]);
                    CombineEventHandles(handlesB, leftHandSource, eventDataPtrs[eventIndex]->eventParamMaps[1]);

                    RemoveDuplicates(handlesB);

                    auto* argsB = RE::MakeFunctionArguments((RE::Actor*)akActor, (RE::TESForm*)leftHandSource, (int)0);
                    SendEvents(handlesB, eventDataPtrs[eventIndex]->sEvent, argsB);

                    logger::info("action event, Actor[{}] Source[{}]  type[{}] slot[{}]", GetFormName(akActor), GetFormName(leftHandSource),
                        actionTypeStrings[type], actionSlotStrings[0]);
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

void CheckMenusCurrentlyOpen() {
    if (menusCurrentlyOpen.size() == 0) {
        return;
    }
    for (auto& menu : menusCurrentlyOpen) {
        if (!ui->IsMenuOpen(menu)) {
            RemoveFromVectorByValue(menusCurrentlyOpen, menu);
        }
    }
}

struct MenuOpenCloseEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*/*source*/) {
        //this sink is for managing timers and GetCurrentMenuOpen function.

        if (!event) {
            logger::error("MenuOpenClose Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }
        
        logger::debug("Menu Open Close Event, menu[{}], opened[{}]", event->menuName, event->opening);

        if (event->menuName != RE::HUDMenu::MENU_NAME) { //hud menu is always open, don't need to do anything for it.
            if (event->opening) {
                lastMenuOpened = event->menuName;
                if (ui->GameIsPaused() && !gamePaused) {
                    gamePaused = true;
                    lastTimeGameWasPaused = std::chrono::system_clock::now();
                    logger::debug("game was paused");
                }

                if (!inMenuMode) { //opened menu
                    inMenuMode = true;
                    lastTimeMenuWasOpened = std::chrono::system_clock::now();
                    logger::debug("inMenuMode = true");
                }
                menusCurrentlyOpen.push_back(event->menuName);
            }
            else {
                if (!ui->GameIsPaused() && gamePaused) {
                    gamePaused = false;
                    auto now = std::chrono::system_clock::now();
                    UpdateTimers(timePointDiffToFloat(now, lastTimeGameWasPaused));
                    logger::debug("game was unpaused");
                }

                //menusCurrentlyOpen.erase(std::remove(menusCurrentlyOpen.begin(), menusCurrentlyOpen.end(), event->menuName), menusCurrentlyOpen.end());
                RemoveFromVectorByValue(menusCurrentlyOpen, event->menuName);
                CheckMenusCurrentlyOpen();
                //logger::info("menu close, number of menusCurrentlyOpen = {}", menusCurrentlyOpen.size());

                if (menusCurrentlyOpen.size() == 0) { //closed menu
                    inMenuMode = false;
                    auto now = std::chrono::system_clock::now();
                    UpdateNoMenuModeTimers(timePointDiffToFloat(now, lastTimeMenuWasOpened));
                    logger::debug("inMenuMode = false");
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

HitEventSink* hitEventSink;
CombatEventSink* combatEventSink;
FurnitureEventSink* furnitureEventSink;
ActivateEventSink* activateEventSink;
DeathEventSink* deathEventSink;
EquipEventSink* equipEventSink;
WaitStartEventSink* waitStartEventSink;
WaitStopEventSink* waitStopEventSink;
MagicEffectApplyEventSink* magicEffectApplyEventSink;
LockChangedEventSink* lockChangedEventSink;
OpenCloseEventSink* openCloseEventSink;
SpellCastEventSink* spellCastEventSink;
ActorActionEventSink* actorActionEventSink;
MenuOpenCloseEventSink* menuOpenCloseEventSink;

bool ShouldActorActionEventSinkBeAdded() {
    for (int i = EventEnum_OnWeaponSwing; i <= EventEnum_EndSheathe; i++) {
        if (!eventDataPtrs[i]->isEmpty()) {
            return true;
        }
    }
    return false;
}

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
        if (!eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded && (!eventDataPtrs[EventEnum_FurnitureEnter]->isEmpty() || !eventDataPtrs[EventEnum_FurnitureExit]->isEmpty())) {
            logger::info("{}, EventEnum_FurnitureExit sink added", __func__);
            eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded = true;
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
        if (!eventDataPtrs[EventEnum_DeathEvent]->sinkAdded && (!eventDataPtrs[EventEnum_DeathEvent]->isEmpty() || !eventDataPtrs[EventEnum_DyingEvent]->isEmpty())) {
            logger::info("{}, EventEnum_DyingEvent sink added", __func__);
            eventDataPtrs[EventEnum_DeathEvent]->sinkAdded = true;
            eventSourceholder->AddEventSink(deathEventSink);
        }
        break;

    case EventEnum_OnObjectEquipped:

    case EventEnum_OnObjectUnequipped:
        if (!eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded && (!eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty() || !eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty())) {
            logger::info("{}, EventEnum_OnObjectUnequipped sink added", __func__);
            eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded = true;
            eventSourceholder->AddEventSink(equipEventSink);
        }
        break;

    case EventEnum_OnWaitStart:
        if (!eventDataPtrs[EventEnum_OnWaitStart]->sinkAdded && !eventDataPtrs[EventEnum_OnWaitStart]->isEmpty()) {
            logger::info("{}, EventEnum_OnWaitStart sink added", __func__);
            eventDataPtrs[EventEnum_OnWaitStart]->sinkAdded = true;
            eventSourceholder->AddEventSink(waitStartEventSink);
        }
        break;

    case EventEnum_OnWaitStop:
        if (!eventDataPtrs[EventEnum_OnWaitStop]->sinkAdded && !eventDataPtrs[EventEnum_OnWaitStop]->isEmpty()) {
            logger::info("{}, EventEnum_OnWaitStop sink added", __func__);
            eventDataPtrs[EventEnum_OnWaitStop]->sinkAdded = true;
            eventSourceholder->AddEventSink(waitStopEventSink);
        }
        break;

    case EventEnum_OnMagicEffectApply:
        if (!eventDataPtrs[EventEnum_OnMagicEffectApply]->sinkAdded && !eventDataPtrs[EventEnum_OnMagicEffectApply]->isEmpty()) {
            logger::info("{}, EventEnum_OnMagicEffectApply sink added", __func__);
            eventDataPtrs[EventEnum_OnMagicEffectApply]->sinkAdded = true;
            eventSourceholder->AddEventSink(magicEffectApplyEventSink);
        }
        break;

    case EventEnum_OnSpellCast:
        if (!eventDataPtrs[EventEnum_OnSpellCast]->sinkAdded && !eventDataPtrs[EventEnum_OnSpellCast]->isEmpty()) {
            logger::info("{}, EventEnum_OnSpellCast sink added", __func__);
            eventDataPtrs[EventEnum_OnSpellCast]->sinkAdded = true;
            eventSourceholder->AddEventSink(spellCastEventSink);
        }
        break;

    case EventEnum_LockChanged:
        if (!eventDataPtrs[EventEnum_LockChanged]->sinkAdded && !eventDataPtrs[EventEnum_LockChanged]->isEmpty()) {
            logger::info("{}, EventEnum_LockChanged sink added", __func__);
            eventDataPtrs[EventEnum_LockChanged]->sinkAdded = true;
            eventSourceholder->AddEventSink(lockChangedEventSink);
        }
        break;

    case EventEnum_OnOpen:

    case EventEnum_OnClose:
        if (!eventDataPtrs[EventEnum_OnOpen]->sinkAdded && (!eventDataPtrs[EventEnum_OnOpen]->isEmpty() || !eventDataPtrs[EventEnum_OnClose]->isEmpty())) {
            logger::info("{}, EventEnum_OnClose sink added", __func__);
            eventDataPtrs[EventEnum_OnOpen]->sinkAdded = true;
            eventSourceholder->AddEventSink(openCloseEventSink);
        }
        break;

        //actorActionEventSink events
    case EventEnum_OnWeaponSwing:
    case EventEnum_OnActorSpellCast:
    case EventEnum_OnActorSpellFire:
    case EventEnum_VoiceCast:
    case EventEnum_VoiceFire:
    case EventEnum_BowDraw:
    case EventEnum_BowRelease:
    case EventEnum_BeginDraw:
    case EventEnum_EndDraw:
    case EventEnum_BeginSheathe:

    case EventEnum_EndSheathe:
        if (!eventDataPtrs[EventEnum_EndSheathe]->sinkAdded && ShouldActorActionEventSinkBeAdded()) {
            logger::info("{}, actorActionEventSink sink added", __func__);
            eventDataPtrs[EventEnum_EndSheathe]->sinkAdded = true;
            SKSE::GetActionEventSource()->AddEventSink(actorActionEventSink);
        }
        break;
    }
}

void RemoveSink(int index) {
    logger::info("removing sink {}", index);

    switch (index) {
    case EventEnum_OnCombatStateChanged:
        if (eventDataPtrs[EventEnum_OnCombatStateChanged]->sinkAdded && eventDataPtrs[EventEnum_OnCombatStateChanged]->isEmpty()) {
            logger::info("{}, EventEnum_OnCombatStateChanged sink removed", __func__);
            eventDataPtrs[EventEnum_OnCombatStateChanged]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(combatEventSink);
        }
        break;

    case EventEnum_FurnitureEnter:

    case EventEnum_FurnitureExit:
        if (eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded && eventDataPtrs[EventEnum_FurnitureEnter]->isEmpty() && eventDataPtrs[EventEnum_FurnitureExit]->isEmpty()) {
            logger::info("{}, EventEnum_FurnitureEnter sink removed", __func__);
            eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(furnitureEventSink);
        }
        break;

    case EventEnum_OnActivate:
        if (eventDataPtrs[EventEnum_OnActivate]->sinkAdded && eventDataPtrs[EventEnum_OnActivate]->isEmpty()) {
            logger::info("{}, EventEnum_OnActivate sink removed", __func__);
            eventDataPtrs[EventEnum_OnActivate]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(activateEventSink);
        }
        break;

    case EventEnum_HitEvent:
        if (eventDataPtrs[EventEnum_HitEvent]->sinkAdded && eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
            logger::info("{}, EventEnum_hitEvent sink removed", __func__);
            eventDataPtrs[EventEnum_HitEvent]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(hitEventSink);
        }
        break;

    case EventEnum_DeathEvent:

    case EventEnum_DyingEvent:
        if (eventDataPtrs[EventEnum_DeathEvent]->sinkAdded && eventDataPtrs[EventEnum_DeathEvent]->isEmpty() && eventDataPtrs[EventEnum_DyingEvent]->isEmpty()) {
            logger::info("{}, EventEnum_DeathEvent sink removed", __func__);
            eventDataPtrs[EventEnum_DeathEvent]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(deathEventSink);
        }
        break;

    case EventEnum_OnObjectEquipped:

    case EventEnum_OnObjectUnequipped:
        if (eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded && eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty() && eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty()) {
            logger::info("{}, EventEnum_OnObjectEquipped sink removed", __func__);
            eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(equipEventSink);
        }
        break;

    case EventEnum_OnWaitStart:
        if (eventDataPtrs[EventEnum_OnWaitStart]->sinkAdded && eventDataPtrs[EventEnum_OnWaitStart]->isEmpty()) {
            logger::info("{}, EventEnum_OnWaitStart sink removed", __func__);
            eventDataPtrs[EventEnum_OnWaitStart]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(waitStartEventSink);
        }
        break;

    case EventEnum_OnWaitStop:
        if (eventDataPtrs[EventEnum_OnWaitStop]->sinkAdded && eventDataPtrs[EventEnum_OnWaitStop]->isEmpty()) {
            logger::info("{}, EventEnum_OnWaitStop sink removed", __func__);
            eventDataPtrs[EventEnum_OnWaitStop]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(waitStopEventSink);
        }
        break;

    case EventEnum_OnMagicEffectApply:
        if (eventDataPtrs[EventEnum_OnMagicEffectApply]->sinkAdded && eventDataPtrs[EventEnum_OnMagicEffectApply]->isEmpty()) {
            logger::info("{}, EventEnum_OnMagicEffectApply sink removed", __func__);
            eventDataPtrs[EventEnum_OnMagicEffectApply]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(magicEffectApplyEventSink);
        }
        break;

    case EventEnum_OnSpellCast:
        if (eventDataPtrs[EventEnum_OnSpellCast]->sinkAdded && eventDataPtrs[EventEnum_OnSpellCast]->isEmpty()) {
            logger::info("{}, EventEnum_OnSpellCast sink removed", __func__);
            eventDataPtrs[EventEnum_OnSpellCast]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(spellCastEventSink);
        }
        break;

    case EventEnum_LockChanged:
        if (eventDataPtrs[EventEnum_LockChanged]->sinkAdded && eventDataPtrs[EventEnum_LockChanged]->isEmpty()) {
            logger::info("{}, EventEnum_LockChanged sink removed", __func__);
            eventDataPtrs[EventEnum_LockChanged]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(lockChangedEventSink);
        }
        break;

    case EventEnum_OnOpen:

    case EventEnum_OnClose:
        if (eventDataPtrs[EventEnum_OnOpen]->sinkAdded && eventDataPtrs[EventEnum_OnOpen]->isEmpty() && eventDataPtrs[EventEnum_OnClose]->isEmpty()) {
            logger::info("{}, EventEnum_OnOpen sink removed", __func__);
            eventDataPtrs[EventEnum_OnOpen]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(openCloseEventSink);
        }
        break;

        //actorActionEventSink events
    case EventEnum_OnWeaponSwing:
    case EventEnum_OnActorSpellCast:
    case EventEnum_OnActorSpellFire:
    case EventEnum_VoiceCast:
    case EventEnum_VoiceFire:
    case EventEnum_BowDraw:
    case EventEnum_BowRelease:
    case EventEnum_BeginDraw:
    case EventEnum_EndDraw:
    case EventEnum_BeginSheathe:

    case EventEnum_EndSheathe:
        if (eventDataPtrs[EventEnum_EndSheathe]->sinkAdded && !ShouldActorActionEventSinkBeAdded()) {
            logger::info("{}, actorActionEventSink sink removed", __func__);
            eventDataPtrs[EventEnum_EndSheathe]->sinkAdded = false;
            SKSE::GetActionEventSource()->RemoveEventSink(actorActionEventSink);
        }
        break;
    }
}

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

//global events ==========================================================================================================================================================================================

// is registered
bool IsFormRegisteredForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::info("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::error("{}: eventReceiver not found", __func__);
        return false;
    }

    logger::info("{} getting handle is registered for: {}", __func__, GetFormName(eventReceiver));

    RE::VMHandle handle = GetHandle(eventReceiver);

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

    RE::VMHandle handle = GetHandle(eventReceiver);

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

    RE::VMHandle handle = GetHandle(eventReceiver);

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

    RE::VMHandle handle = GetHandle(eventReceiver);

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

    RE::VMHandle handle = GetHandle(eventReceiver);

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

    RE::VMHandle handle = GetHandle(eventReceiver);

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

    logger::info("{} removing handle for: {}", __func__, GetFormName(eventReceiver));

    RE::VMHandle handle = GetHandle(eventReceiver);

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

    RE::VMHandle handle = GetHandle(eventReceiver);

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

    RE::VMHandle handle = GetHandle(eventReceiver);

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

    logger::info("{} removing handle for: {}", __func__, GetFormName(eventReceiver));

    RE::VMHandle handle = GetHandle(eventReceiver);

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

    RE::VMHandle handle = GetHandle(eventReceiver);

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

    RE::VMHandle handle = GetHandle(eventReceiver);

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
        eventDataPtrs[EventEnum_OnWaitStart] = new EventData("OnWaitStartGlobal", EventEnum_OnWaitStart, 0, 'EDw0');
        eventDataPtrs[EventEnum_OnWaitStop] = new EventData("OnWaitStopGlobal", EventEnum_OnWaitStop, 0, 'EDw1');
        eventDataPtrs[EventEnum_OnMagicEffectApply] = new EventData("OnMagicEffectAppliedGlobal", EventEnum_OnMagicEffectApply, 3, 'EDm2');
        eventDataPtrs[EventEnum_OnSpellCast] = new EventData("OnSpellCastGlobal", EventEnum_OnSpellCast, 2, 'EDs0');
        eventDataPtrs[EventEnum_LockChanged] = new EventData("OnLockChangedGlobal", EventEnum_LockChanged, 1, 'EDlc');
        eventDataPtrs[EventEnum_OnOpen] = new EventData("OnOpenGlobal", EventEnum_OnOpen, 2, 'EDo0');
        eventDataPtrs[EventEnum_OnClose] = new EventData("OnCloseGlobal", EventEnum_OnClose, 2, 'EDo1');
        eventDataPtrs[EventEnum_OnWeaponSwing] = new EventData("OnWeaponSwingGlobal", EventEnum_OnWeaponSwing, 2, 'EAA0');              //actorActionEventSink
        eventDataPtrs[EventEnum_OnActorSpellCast] = new EventData("OnActorSpellCastGlobal", EventEnum_OnActorSpellCast, 2, 'EAA1');     //actorActionEventSink
        eventDataPtrs[EventEnum_OnActorSpellFire] = new EventData("OnActorSpellFireGlobal", EventEnum_OnActorSpellFire, 2, 'EAA2');     //actorActionEventSink
        eventDataPtrs[EventEnum_VoiceCast] = new EventData("OnVoiceCastGlobal", EventEnum_VoiceCast, 2, 'EAA3');                        //actorActionEventSink
        eventDataPtrs[EventEnum_VoiceFire] = new EventData("OnVoiceFireGlobal", EventEnum_VoiceFire, 2, 'EAA4');                        //actorActionEventSink
        eventDataPtrs[EventEnum_BowDraw] = new EventData("OnBowDrawGlobal", EventEnum_BowDraw, 2, 'EAA5');                              //actorActionEventSink
        eventDataPtrs[EventEnum_BowRelease] = new EventData("OnBowReleaseGlobal", EventEnum_BowRelease, 2, 'EAA6');                     //actorActionEventSink
        eventDataPtrs[EventEnum_BeginDraw] = new EventData("OnBeginDrawGlobal", EventEnum_BeginDraw, 2, 'EAA7');                        //actorActionEventSink
        eventDataPtrs[EventEnum_EndDraw] = new EventData("OnEndDrawGlobal", EventEnum_EndDraw, 2, 'EAA8');                              //actorActionEventSink
        eventDataPtrs[EventEnum_BeginSheathe] = new EventData("OnBeginSheatheGlobal", EventEnum_BeginSheathe, 2, 'EAA9');               //actorActionEventSink
        eventDataPtrs[EventEnum_EndSheathe] = new EventData("OnEndSheatheGlobal", EventEnum_EndSheathe, 2, 'EA10');                     //actorActionEventSink
    }

    if (!combatEventSink) { combatEventSink = new CombatEventSink(); }
    if (!furnitureEventSink) { furnitureEventSink = new FurnitureEventSink(); }
    if (!activateEventSink) { activateEventSink = new ActivateEventSink(); }
    if (!hitEventSink) { hitEventSink = new HitEventSink(); }
    if (!deathEventSink) { deathEventSink = new DeathEventSink(); }
    if (!equipEventSink) { equipEventSink = new EquipEventSink(); }
    if (!waitStartEventSink) { waitStartEventSink = new WaitStartEventSink(); }
    if (!waitStopEventSink) { waitStopEventSink = new WaitStopEventSink(); }
    if (!magicEffectApplyEventSink) { magicEffectApplyEventSink = new MagicEffectApplyEventSink(); }
    if (!spellCastEventSink) { spellCastEventSink = new SpellCastEventSink(); }
    if (!lockChangedEventSink) { lockChangedEventSink = new LockChangedEventSink(); }
    if (!openCloseEventSink) { openCloseEventSink = new OpenCloseEventSink(); }
    if (!actorActionEventSink) { actorActionEventSink = new ActorActionEventSink(); }
    if (!menuOpenCloseEventSink) { menuOpenCloseEventSink = new MenuOpenCloseEventSink(); }

    //SKSE::GetActionEventSource()->AddEventSink(actorActionEventSink);
    ui->AddEventSink<RE::MenuOpenCloseEvent>(menuOpenCloseEventSink);

    logger::info("Event Sinks Created");
}

bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    // vm->RegisterFunction("MyNativeFunction", "DbSkseFunctions", MyNativeFunction);
    logger::info("Binding Papyrus Functions");

    gvm = vm;
    svm = RE::SkyrimVM::GetSingleton();
    ui = RE::UI::GetSingleton();
    calendar = RE::Calendar::GetSingleton();

    //functions 
    vm->RegisterFunction("GetVersion", "DbSkseFunctions", GetThisVersion);
    vm->RegisterFunction("GetClipBoardText", "DbSkseFunctions", GetClipBoardText);
    vm->RegisterFunction("SetClipBoardText", "DbSkseFunctions", SetClipBoardText);
    vm->RegisterFunction("IsWhiteSpace", "DbSkseFunctions", IsWhiteSpace);
    vm->RegisterFunction("CountWhiteSpaces", "DbSkseFunctions", CountWhiteSpaces);
    vm->RegisterFunction("GameHoursToRealTimeSeconds", "DbSkseFunctions", GameHoursToRealTimeSeconds);
    vm->RegisterFunction("IsGamePaused", "DbSkseFunctions", IsGamePaused); 
    vm->RegisterFunction("IsInMenu", "DbSkseFunctions", IsInMenu);
    vm->RegisterFunction("GetLastMenuOpened", "DbSkseFunctions", GetLastMenuOpened);
    vm->RegisterFunction("SetMapMarkerName", "DbSkseFunctions", SetMapMarkerName);
    vm->RegisterFunction("GetMapMarkerName", "DbSkseFunctions", GetMapMarkerName);
    vm->RegisterFunction("SetMapMarkerIconType", "DbSkseFunctions", SetMapMarkerIconType);
    vm->RegisterFunction("GetMapMarkerIconType", "DbSkseFunctions", GetMapMarkerIconType);
    vm->RegisterFunction("IsMapMarker", "DbSkseFunctions", IsMapMarker);
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
    vm->RegisterFunction("GetWordOfPowerTranslation", "DbSkseFunctions", GetWordOfPowerTranslation);
    vm->RegisterFunction("UnlockShout", "DbSkseFunctions", UnlockShout);
    vm->RegisterFunction("AddAndUnlockAllShouts", "DbSkseFunctions", AddAndUnlockAllShouts);
    vm->RegisterFunction("GetActiveEffectSource", "DbSkseFunctions", GetActiveEffectSource);
    vm->RegisterFunction("GetActiveEffectCastingSource", "DbSkseFunctions", GetActiveEffectCastingSource);
    vm->RegisterFunction("SetSoulGemSize", "DbSkseFunctions", SetSoulGemSize);
    vm->RegisterFunction("CanSoulGemHoldNPCSoul", "DbSkseFunctions", CanSoulGemHoldNPCSoul);
    vm->RegisterFunction("SetSoulGemCanHoldNPCSoul", "DbSkseFunctions", SetSoulGemCanHoldNPCSoul);
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
    vm->RegisterFunction("CreateSoundMarker", "DbSkseFunctions", CreateSoundMarker);
    vm->RegisterFunction("PlaySound", "DbSkseFunctions", PlaySound);
    vm->RegisterFunction("PlaySoundDescriptor", "DbSkseFunctions", PlaySoundDescriptor);
    vm->RegisterFunction("GetParentSoundCategory", "DbSkseFunctions", GetParentSoundCategory);
    vm->RegisterFunction("GetSoundCategoryForSoundDescriptor", "DbSkseFunctions", GetSoundCategoryForSoundDescriptor);
    vm->RegisterFunction("SetSoundCategoryForSoundDescriptor", "DbSkseFunctions", SetSoundCategoryForSoundDescriptor);
    vm->RegisterFunction("GetSoundCategoryVolume", "DbSkseFunctions", GetSoundCategoryVolume);
    vm->RegisterFunction("GetSoundCategoryFrequency", "DbSkseFunctions", GetSoundCategoryFrequency);

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

    //timers
    
    //form
    vm->RegisterFunction("StartTimer", "DbFormTimer", StartTimerOnForm);
    vm->RegisterFunction("CancelTimer", "DbFormTimer", CancelTimerOnForm);
    vm->RegisterFunction("GetTimeElapsedOnTimer", "DbFormTimer", GetTimeElapsedOnTimerForm);
    vm->RegisterFunction("GetTimeLeftOnTimer", "DbFormTimer", GetTimeLeftOnTimerForm);

    vm->RegisterFunction("StartNoMenuModeTimer", "DbFormTimer", StartNoMenuModeTimerOnForm);
    vm->RegisterFunction("CancelNoMenuModeTimer", "DbFormTimer", CancelNoMenuModeTimerOnForm);
    vm->RegisterFunction("GetTimeElapsedOnNoMenuModeTimer", "DbFormTimer", GetTimeElapsedOnNoMenuModeTimerForm);
    vm->RegisterFunction("GetTimeLeftOnNoMenuModeTimer", "DbFormTimer", GetTimeLeftOnNoMenuModeTimerForm);

    vm->RegisterFunction("StartMenuModeTimer", "DbFormTimer", StartMenuModeTimerOnForm);
    vm->RegisterFunction("CancelMenuModeTimer", "DbFormTimer", CancelMenuModeTimerOnForm);
    vm->RegisterFunction("GetTimeElapsedOnMenuModeTimer", "DbFormTimer", GetTimeElapsedOnMenuModeTimerForm);
    vm->RegisterFunction("GetTimeLeftOnMenuModeTimer", "DbFormTimer", GetTimeLeftOnMenuModeTimerForm);

    vm->RegisterFunction("StartGameTimer", "DbFormTimer", StartGameTimerOnForm);
    vm->RegisterFunction("CancelGameTimer", "DbFormTimer", CancelGameTimerOnForm);
    vm->RegisterFunction("GetTimeElapsedOnGameTimer", "DbFormTimer", GetTimeElapsedOnGameTimerForm);
    vm->RegisterFunction("GetTimeLeftOnGameTimer", "DbFormTimer", GetTimeLeftOnGameTimerForm);

    //Alias
    vm->RegisterFunction("StartTimer", "DbAliasTimer", StartMenuModeTimerOnAlias);
    vm->RegisterFunction("CancelTimer", "DbAliasTimer", CancelMenuModeTimerOnAlias);
    vm->RegisterFunction("GetTimeElapsedOnTimer", "DbAliasTimer", GetTimeElapsedOnMenuModeTimerAlias);
    vm->RegisterFunction("GetTimeLeftOnTimer", "DbAliasTimer", GetTimeLeftOnMenuModeTimerAlias);

    vm->RegisterFunction("StartNoMenuModeTimer", "DbAliasTimer", StartTimerOnAlias);
    vm->RegisterFunction("CancelNoMenuModeTimer", "DbAliasTimer", CancelTimerOnAlias);
    vm->RegisterFunction("GetTimeElapsedOnNoMenuModeTimer", "DbAliasTimer", GetTimeElapsedOnTimerAlias);
    vm->RegisterFunction("GetTimeLeftOnNoMenuModeTimer", "DbAliasTimer", GetTimeLeftOnTimerAlias);

    vm->RegisterFunction("StartMenuModeTimer", "DbAliasTimer", StartTimerOnAlias);
    vm->RegisterFunction("CancelMenuModeTimer", "DbAliasTimer", CancelTimerOnAlias);
    vm->RegisterFunction("GetTimeElapsedOnMenuModeTimer", "DbAliasTimer", GetTimeElapsedOnTimerAlias);
    vm->RegisterFunction("GetTimeLeftOnMenuModeTimer", "DbAliasTimer", GetTimeLeftOnTimerAlias);

    vm->RegisterFunction("StartGameTimer", "DbAliasTimer", StartGameTimerOnAlias);
    vm->RegisterFunction("CancelGameTimer", "DbAliasTimer", CancelGameTimerOnAlias);
    vm->RegisterFunction("GetTimeElapsedOnGameTimer", "DbAliasTimer", GetTimeElapsedOnGameTimerAlias);
    vm->RegisterFunction("GetTimeLeftOnGameTimer", "DbAliasTimer", GetTimeLeftOnGameTimerAlias);

    //ActiveMagicEffect
    vm->RegisterFunction("StartTimer", "DbActiveMagicEffectTimer", StartMenuModeTimerOnActiveMagicEffect);
    vm->RegisterFunction("CancelTimer", "DbActiveMagicEffectTimer", CancelMenuModeTimerOnActiveMagicEffect);
    vm->RegisterFunction("GetTimeElapsedOnTimer", "DbActiveMagicEffectTimer", GetTimeElapsedOnMenuModeTimerActiveMagicEffect);
    vm->RegisterFunction("GetTimeLeftOnTimer", "DbActiveMagicEffectTimer", GetTimeLeftOnMenuModeTimerActiveMagicEffect);

    vm->RegisterFunction("StartNoMenuModeTimer", "DbActiveMagicEffectTimer", StartTimerOnActiveMagicEffect);
    vm->RegisterFunction("CancelNoMenuModeTimer", "DbActiveMagicEffectTimer", CancelTimerOnActiveMagicEffect);
    vm->RegisterFunction("GetTimeElapsedOnNoMenuModeTimer", "DbActiveMagicEffectTimer", GetTimeElapsedOnTimerActiveMagicEffect);
    vm->RegisterFunction("GetTimeLeftOnNoMenuModeTimer", "DbActiveMagicEffectTimer", GetTimeLeftOnTimerActiveMagicEffect);

    vm->RegisterFunction("StartMenuModeTimer", "DbActiveMagicEffectTimer", StartTimerOnActiveMagicEffect);
    vm->RegisterFunction("CancelMenuModeTimer", "DbActiveMagicEffectTimer", CancelTimerOnActiveMagicEffect);
    vm->RegisterFunction("GetTimeElapsedOnMenuModeTimer", "DbActiveMagicEffectTimer", GetTimeElapsedOnTimerActiveMagicEffect);
    vm->RegisterFunction("GetTimeLeftOnMenuModeTimer", "DbActiveMagicEffectTimer", GetTimeLeftOnTimerActiveMagicEffect);

    vm->RegisterFunction("StartGameTimer", "DbActiveMagicEffectTimer", StartGameTimerOnActiveMagicEffect);
    vm->RegisterFunction("CancelGameTimer", "DbActiveMagicEffectTimer", CancelGameTimerOnActiveMagicEffect);
    vm->RegisterFunction("GetTimeElapsedOnGameTimer", "DbActiveMagicEffectTimer", GetTimeElapsedOnGameTimerActiveMagicEffect);
    vm->RegisterFunction("GetTimeLeftOnGameTimer", "DbActiveMagicEffectTimer", GetTimeLeftOnGameTimerActiveMagicEffect);

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
        SKSE::GetPapyrusInterface()->Register(BindPapyrusFunctions);
        CreateEventSinks();
        SetSettingsFromIniFile();
        logger::info("kDataLoaded: sent after the data handler has loaded all its forms");
        break;

        //default: //
            //    logger::info("Unknown system message of type: {}", message->type);
            //    break;
    }
}

void LoadCallback(SKSE::SerializationInterface* a_intfc) {
    int max = EventEnum_Last + 1;
    std::uint32_t type, version, length;

    while (a_intfc->GetNextRecordInfo(type, version, length)) {
        if (type == 'DBT0') {
            loadTimers(currentMenuModeTimers, 'DBT0', a_intfc);
        }
        else if (type == 'DBT1') {
            loadTimers(currentNoMenuModeTimers, 'DBT1', a_intfc);
        }
        else if (type == 'DBT2') {
            loadTimers(currentTimers, 'DBT2', a_intfc);
        }
        else if (type == 'DBT3') {
            loadTimers(currentGameTimeTimers, 'DBT3', a_intfc);
        }
        else {
            for (int i = EventEnum_First; i < max; i++) {
                if (type == eventDataPtrs[i]->record) {
                    eventDataPtrs[i]->Load(a_intfc);
                    break;
                }
            }
        }
    }

    if (!player) { player = RE::PlayerCharacter::GetSingleton(); }
    bPlayerIsInCombat = player->IsInCombat();

    //EventEnum_OnLoadGame doesn't have an event sink, hence EventEnum_First + 1
    for (int i = EventEnum_First + 1; i < max; i++) {
        AddSink(i);
    }

    RemoveDuplicates(eventDataPtrs[EventEnum_OnLoadGame]->globalHandles);

    auto* args = RE::MakeFunctionArguments();
    SendEvents(eventDataPtrs[EventEnum_OnLoadGame]->globalHandles, eventDataPtrs[EventEnum_OnLoadGame]->sEvent, args);

    logger::info("LoadCallback complete");
}

void SaveCallback(SKSE::SerializationInterface* a_intfc) {
    int max = EventEnum_Last + 1;
    for (int i = EventEnum_First; i < max; i++) {
        eventDataPtrs[i]->Save(a_intfc);
    }
    
    SaveTimers(currentMenuModeTimers, 'DBT0', a_intfc);
    SaveTimers(currentNoMenuModeTimers, 'DBT1', a_intfc);
    SaveTimers(currentTimers, 'DBT2', a_intfc);
    SaveTimers(currentGameTimeTimers, 'DBT3', a_intfc);
} 

//init================================================================================================================================================================
SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);

    SetupLog();
    //CreateEventSinks();
    SKSE::GetMessagingInterface()->RegisterListener(MessageListener);

    auto serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID('DbSF');
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);

    return true;
}