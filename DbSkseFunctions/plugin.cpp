#include <Windows.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <winbase.h>
#include <iostream>

namespace logger = SKSE::log;

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

float GetThisVersion(RE::BSScript::Internal::VirtualMachine *, const RE::VMStackID, RE::StaticFunctionTag *) {
    return float(4.6);
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

std::string GetClipBoardText(RE::StaticFunctionTag *) {
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

    char *pszText = static_cast<char *>(GlobalLock(hData));
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

bool SetClipBoardText(RE::StaticFunctionTag *,
                      std::string sText) {
    if (sText.length() == 0) {
        return false;
    }

    const char *output = sText.data();
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
    return true;
}

bool IsWhiteSpace(RE::StaticFunctionTag *, std::string c) {
    return isspace(int(c.at(0)));
}

int CountWhiteSpaces(RE::StaticFunctionTag *, std::string s) {
    int spaces = std::count_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
    return spaces;
}

bool IsMapMarker(RE::StaticFunctionTag *, RE::TESObjectREFR *mapMarker) {
    
    LogAndMessage("IsMapMarker function called");
    auto *mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

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

bool SetMapMarkerName(RE::StaticFunctionTag *,
                      RE::TESObjectREFR *mapMarker, std::string name) {

    LogAndMessage("Renaming map marker");
    auto *mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

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

    const char *cName = name.data();
    mapMarkerData->mapData->locationName.fullName = cName;
    LogAndMessage(std::format("New map marker name = {}", mapMarkerData->mapData->locationName.GetFullName()));
    
    return true;
}

std::string GetMapMarkerName(RE::StaticFunctionTag *,
                      RE::TESObjectREFR *mapMarker) {
    LogAndMessage("Getting Marker Name");

    auto *mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

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

bool SetMapMarkerIconType(RE::BSScript::Internal::VirtualMachine*, const RE::VMStackID, RE::StaticFunctionTag*,
    RE::TESObjectREFR* mapMarker, int iconType) {

    LogAndMessage(std::format("Setting Map Marker Type to {}", iconType));
    
    auto *mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        LogAndMessage("SetMapMarkerIconType Warning, map marker extra data list not found.", warn);
        return false;
    }

    if (!mapMarkerData->mapData) {
        LogAndMessage("SetMapMarkerIconType Warning, mapData not found.", warn);
        return false;
    }

    //mapMarkerData->mapData->type.set<RE::MARKER_TYPE, iconType>();
    //mapMarkerData->mapData->type.set(static_cast<RE::MARKER_TYPE>(std::uint32_t(iconType)));
    mapMarkerData->mapData->type = static_cast<RE::MARKER_TYPE>(iconType);
    
    return true;
}

int GetMapMarkerIconType(RE::StaticFunctionTag *, RE::TESObjectREFR* mapMarker) {
    LogAndMessage("Getting Map Marker Type");
    auto *mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

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

bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine *vm) {
    // vm->RegisterFunction("MyNativeFunction", "DbSkseFunctions", MyNativeFunction);
    LogAndMessage("Binding Papyrus Functions");
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
    
    return true;
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);

    SetupLog();
    SKSE::GetPapyrusInterface()->Register(BindPapyrusFunctions);

    //SetUpEventSink();

    return true;
}