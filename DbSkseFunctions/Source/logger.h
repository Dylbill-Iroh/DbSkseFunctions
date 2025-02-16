#pragma once
#include <spdlog/sinks/basic_file_sink.h>
#include "mini/ini.h"
//#include "MiniIniHelper.h"

namespace logger = SKSE::log;

void SetupLog(std::string iniFilePath = "", std::string iniSection = "LOG", std::string iniKey = "iMinLevel") {
    spdlog::level::level_enum iLevel = static_cast<spdlog::level::level_enum>(0);

    auto ini = mINI::GetIniFile(iniFilePath); 
    int iValue = mINI::GetIniInt(ini, iniSection, iniKey);

    if (iValue > -1) {
        iLevel = static_cast<spdlog::level::level_enum>(iValue);
    }

    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(iLevel);
    spdlog::flush_on(spdlog::level::trace);
    logger::info("{} level set to [{}] iValue[{}]", __func__, iLevel, iValue);
}
