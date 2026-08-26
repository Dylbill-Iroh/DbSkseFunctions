#pragma once
#include "mini/ini.h"

namespace mINI {
bool IniHasSectionKey(mINI::INIStructure& ini, std::string section, std::string key);

std::string GetIniString(mINI::INIStructure & ini, std::string section, std::string key, std::string sDefault = "");

bool GetIniBool(mINI::INIStructure & ini, std::string section, std::string key, bool bDefault = false);

int GetIniInt(mINI::INIStructure & ini, std::string section, std::string key, int iDefault = -1);

float GetIniFloat(mINI::INIStructure & ini, std::string section, std::string key, float fDefault = -1.0);
}

