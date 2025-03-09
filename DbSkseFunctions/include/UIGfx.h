#pragma once

namespace gfx {
	//only BindPapyrusFunctions is used in plugin.cpp
	//std::string GetGFxTypeString(int type);

	//std::string GetItemListPathForItemMenu(std::string_view menuName);

	//void Invoke(std::string_view menuPath, std::string target);

	//void InvokeInt(std::string_view menuPath, std::string target, int arg);

	//void InvokeString(std::string_view menuPath, std::string target, std::string arg);

	//void SetString(std::string_view menuPath, std::string target, std::string arg);

	//void SetItemMenuSelection(std::string_view menuPath, int index);

	//std::string GetGfxValueAsString(const RE::GFxValue& gfxValue);

	//std::string GetGfxValueAsString(RE::GFxValue& gfxValue);

	//bool IsGfxMemberValid(const RE::GFxValue& gfxValue, std::string gfxName = "");

	//bool IsGfxMemberValid(RE::GFxValue& gfxValue, std::string gfxName = "");

	//bool GFxMemberNameIsValid(std::string name);

	//std::vector<std::pair<const char*, RE::GFxValue>> GetGFxMembers(const RE::GFxValue& gfxValue);

	//void LogGFxMembers(const RE::GFxValue& gfxValue, std::string gfxName);

	//void LogGFxMembers(RE::GPtr<RE::GFxMovieView> mv, std::vector<std::string> memberStrings);

	//void EraseQuantityStringFromUIitemName(std::string& uiItemName);

	//std::string GetGFxListEntryText(RE::GFxValue& listEntry);

	//int GetEntryDataArrayLength(std::string_view menuName, RE::GPtr<RE::GFxMovieView>mv);

	//int GetEntryDataArrayLength(std::string_view menuName);

	//int GetIndexForMenuItem(std::string_view menuName, std::string sItemName);

	//int GetSelectedEntryIndex(std::string_view menuName);

	//std::string GetSelectedEntryText(std::string_view menuName);

	//RE::GFxValue GetSelectedEntry(std::string_view menuName); //

	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}