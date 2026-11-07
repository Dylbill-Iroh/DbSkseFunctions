#pragma once
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/pattern_formatter.h>
#include <ctre.hpp>
#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <string>
#include "SKSE/Logger.h"
#include "mini/ini.h"
#include "mINIHelper.h"
#include "GeneralFunctions.h"

//Add function name tag to log.
//Thanks to Noname365 aka Judah for this
class function_name_formatter final : public spdlog::custom_flag_formatter
{
public:
    void format(const spdlog::details::log_msg& msg, const std::tm&, spdlog::memory_buf_t& dest) override {
        const std::string_view func_name{ extract_function_name(msg.source.funcname) };
        dest.append(func_name.data(), func_name.data() + func_name.size());
    }

    std::unique_ptr<custom_flag_formatter> clone() const override {
        return spdlog::details::make_unique<function_name_formatter>();
    }

private:
    static std::string_view extract_function_name(const std::string_view full_signature) {
        constexpr auto pattern{ ctll::fixed_string{ R"((\w+(::\w+)*)\s*(?=\())" } };

        if (auto match{ ctre::search<pattern>(full_signature) }) {
            return match.get<1>();
        }
        return full_signature.data();
    }
};

namespace logger = SKSE::log;

// Paths are kept as std::filesystem::path / std::wstring the whole way. Converting a path to std::string
// (.string(), .generic_string()) throws std::system_error when it contains characters that aren't in the
// user's ANSI code page (non-Latin Windows user names, OneDrive folders, ...), which crashed the game at startup.
inline std::wstring ToLowerW(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
    return s;
}

// Fix for the AE bug where logs are written to "My Games/Skyrim.INI/SKSE" instead of "My Games/Skyrim Special Edition/SKSE".
inline std::filesystem::path FixLogsFolderPath(const std::filesystem::path& logsFolder) {
    if (ToLowerW(logsFolder.wstring()).find(L"my games") == std::wstring::npos) {
        return logsFolder;
    }

    std::filesystem::path path = logsFolder;
    std::wstring parentPathName = ToLowerW(path.filename().wstring());
    while (path.has_parent_path() && parentPathName != L"my games") {
        auto parent = path.parent_path();
        if (parent == path) {
            break;  // reached the root
        }
        path = parent;
        parentPathName = ToLowerW(path.filename().wstring());
    }

    if (parentPathName == L"my games") {
        path /= L"Skyrim Special Edition";
        path /= L"SKSE";
        return path;
    }
    return logsFolder;
}

void SetupLog(std::string iniFilePath = "", std::string iniSection = "LOG", std::string iniKey = "iMinLevel") {
    spdlog::level::level_enum iLevel = static_cast<spdlog::level::level_enum>(0);

    mINI::INIFile file(iniFilePath);
    mINI::INIStructure ini;
    file.read(ini);

    int iValue = mINI::GetIniInt(ini, iniSection, iniKey);

    if (iValue > -1) {
        iLevel = static_cast<spdlog::level::level_enum>(iValue);
    }

    auto logsFolder{ SKSE::log::log_directory() };
    if (!logsFolder) {
        SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    }

    std::string_view pluginName = "DbSkseFunctions";
    auto* pluginDeclaration = SKSE::PluginDeclaration::GetSingleton();
    if (pluginDeclaration) {
        pluginName = pluginDeclaration->GetName();
    }

    try {
        std::filesystem::path logsFolderPath = FixLogsFolderPath(*logsFolder);

        std::error_code ec;
        std::filesystem::create_directories(logsFolderPath, ec);

        std::filesystem::path logFilePath = logsFolderPath / std::format("{}.log", pluginName);  // plugin name is ASCII

        // Opened with the path itself, which MSVC opens with the wide (Unicode) Windows API, so the folder can contain
        // any characters. spdlog's basic_file_sink takes a narrow std::string path instead, which can't hold them.
        // Allocated once and never freed on purpose: the sink keeps a reference to it, and spdlog may still
        // flush during shutdown after normal static objects are destroyed.
        static std::ofstream* logFile = new std::ofstream();
        logFile->open(logFilePath, std::ios::out | std::ios::trunc);
        if (!logFile->is_open()) {
            throw std::runtime_error("couldn't open the log file");
        }

        auto fileSinkPtr = std::make_shared<spdlog::sinks::ostream_sink_mt>(*logFile, true);  // true = flush after every message
        auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileSinkPtr));
        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->add_flag<function_name_formatter>('*').set_pattern("[%Y-%m-%d %H:%M:%S.%o] [%n] [%l] [%s.%#] [%*] %v");
        loggerPtr->set_formatter(std::move(formatter));
        spdlog::set_default_logger(std::move(loggerPtr));
    }
    catch (const std::exception&) {
        // Logging can't be set up for this user. Use a logger with no sinks so every logger:: call is a no-op
        // instead of crashing the game.
        spdlog::set_default_logger(std::make_shared<spdlog::logger>("log"));
    }

    spdlog::set_level(iLevel);
    spdlog::flush_on(spdlog::level::trace);
	logger::info("level set to [{}]", static_cast<int>(iLevel));
}