#pragma once
namespace magic {
	/*std::string GetWordOfPowerTranslation(RE::StaticFunctionTag*, RE::TESWordOfPower* akWord);

	void UnlockShout(RE::StaticFunctionTag*, RE::TESShout* akShout);

	void AddAndUnlockAllShouts(RE::StaticFunctionTag*, int minNumberOfWordsWithTranslations, bool onlyShoutsWithNames, bool onlyShoutsWithDescriptions);

	int GetActiveMagicEffectConditionStatus(RE::StaticFunctionTag*, RE::ActiveEffect* akEffect);

	RE::TESForm* GetActiveEffectSource(RE::StaticFunctionTag*, RE::ActiveEffect* akEffect);

	int GetActiveEffectCastingSource(RE::StaticFunctionTag*, RE::ActiveEffect* akEffect);

	std::vector<RE::EffectSetting*> GetMagicEffectsForForm(RE::StaticFunctionTag*, RE::TESForm* akForm);

	bool IsFormMagicItem(RE::StaticFunctionTag*, RE::TESForm* akForm);

	bool IsMagicEffectActiveOnRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, RE::EffectSetting* akMagicEffect, RE::TESForm* magicSource);

	void DispelMagicEffectOnRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, RE::EffectSetting* akMagicEffect, RE::TESForm* magicSource);

	void SetSoulGemSize(RE::StaticFunctionTag*, RE::TESSoulGem* akGem, int level);

	void SetSoulGemCanHoldNPCSoul(RE::StaticFunctionTag*, RE::TESSoulGem* akGem, bool canCarry);

	bool CanSoulGemHoldNPCSoul(RE::StaticFunctionTag*, RE::TESSoulGem* akGem);*/

	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}