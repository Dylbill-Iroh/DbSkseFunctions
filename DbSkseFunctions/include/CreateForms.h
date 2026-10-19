#pragma once

namespace CreateForms {
	RE::BGSKeyword* CreateKeyword(RE::StaticFunctionTag*);

	RE::BGSListForm* CreateFormList(RE::StaticFunctionTag*, RE::BGSListForm * formListFiller);

	RE::BGSColorForm* CreateColorForm(RE::StaticFunctionTag*, int color);

	RE::BGSConstructibleObject* CreateConstructibleObject(RE::StaticFunctionTag*);

	RE::BGSTextureSet* CreateTextureSet(RE::StaticFunctionTag*);

	RE::TESSound* CreateSoundMarker(RE::StaticFunctionTag*);

	RE::TESQuest* CreateQuest(RE::StaticFunctionTag*);

	namespace Serialization {
		constexpr std::uint32_t createFormsRecord = 'STf9';
		bool SaveForms(SKSE::SerializationInterface* ssi);
		bool LoadForms(SKSE::SerializationInterface* ssi, std::uint32_t version);
	}
	
	
	RE::TESForm* CreateForm(RE::StaticFunctionTag*, std::string baseScriptType, std::string scriptName);
	
	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}

