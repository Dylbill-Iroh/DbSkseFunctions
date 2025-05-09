#include "mINIHelper.h"
#include "mini/ini.h"

namespace mINI {
	//Added by Dylbill
	bool IniHasSectionKey(mINI::INIStructure& ini, std::string section, std::string key) {
		if (ini.has(section)) {
			auto& collection = ini[section];
			if (collection.has(key)) {
				return true;
			}
		}
		return false;
	}

	std::string GetIniString(mINI::INIStructure& ini, std::string section, std::string key, std::string sDefault) {
		std::string sReturn = sDefault;
		if (ini.has(section)) {
			// we have section, we can access it safely without creating a new one
			auto& collection = ini[section];
			if (collection.has(key)) {
				// we have key, we can access it safely without creating a new one
				sReturn = collection[key];
			}
		}
		return sReturn;
	}

	bool GetIniBool(mINI::INIStructure& ini, std::string section, std::string key, bool bDefault) {
		std::string s = GetIniString(ini, section, key);
		transform(s.begin(), s.end(), s.begin(), ::tolower);
		if (s == "true" || s == "1") {
			return true;
		}
		else if (s == "false" || s == "0") {
			return false;
		}
		else {
			return bDefault;
		}
	}

	int GetIniInt(mINI::INIStructure& ini, std::string section, std::string key, int iDefault) {
		std::string s = GetIniString(ini, section, key);
		if (s != "") {
			return std::stoi(s);
		}
		else {
			return iDefault;
		}
	}

	float GetIniFloat(mINI::INIStructure& ini, std::string section, std::string key, float fDefault) {
		std::string s = GetIniString(ini, section, key);
		if (s != "") {
			return std::stof(s);
		}
		else {
			return fDefault;
		}
	}
}
