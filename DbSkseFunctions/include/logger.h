#pragma once
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/pattern_formatter.h>
#include <ctre.hpp>
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

    //fix the bug on AE where logs are written to "My Games/Skyrim.INI/ instead of "My Games/Skyrim Special Edition/SKSE
    std::filesystem::path logsFolderPath = logsFolder.value();
    std::string sLogPath = logsFolderPath.generic_string(); 
    gfuncs::ConvertToLowerCase(sLogPath);

    if (sLogPath.find("my games") != std::string::npos) {
        std::string parentPathName = logsFolderPath.filename().string();
        gfuncs::ConvertToLowerCase(parentPathName);
        while (logsFolderPath.has_parent_path() && parentPathName != "my games") {
            logsFolderPath = logsFolderPath.parent_path();
            parentPathName = logsFolderPath.filename().string();
            gfuncs::ConvertToLowerCase(parentPathName);
        }

        if (parentPathName == "my games") {
            logsFolderPath.append("Skyrim Special Edition");
            logsFolderPath.append("SKSE");
        }
        else {
            logsFolderPath = logsFolder.value();
        }
    }

    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    std::string pluginFileName = std::format("{}.log", pluginName);
    logsFolderPath.append(pluginFileName);

    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logsFolderPath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    auto formatter = std::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<function_name_formatter>('*').set_pattern("[%Y-%m-%d %H:%M:%S.%o] [%n] [%l] [%s.%#] [%*] %v");
    loggerPtr->set_formatter(std::move(formatter));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(iLevel);
    spdlog::flush_on(spdlog::level::trace);
    logger::info("level set to [{}]", iLevel);
}
