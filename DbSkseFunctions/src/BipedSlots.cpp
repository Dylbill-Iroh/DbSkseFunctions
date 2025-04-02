#include "BipedSlots.h"
#include "GeneralFunctions.h"

namespace biped {
	std::vector<RE::TESRace*> GetArmorAddonRaces(RE::StaticFunctionTag*, RE::TESObjectARMA* armorAddon) {
		std::vector<RE::TESRace*> races;
		if (!gfuncs::IsFormValid(armorAddon)) {
			return races;
		}

		RE::TESRaceForm* raceForm = skyrim_cast<RE::TESRaceForm*>(armorAddon);
		if (raceForm) {
			if (gfuncs::IsFormValid(raceForm->race)) {
				races.push_back(raceForm->race);
			}
		}

		for (auto* race : armorAddon->additionalRaces) {
			if (gfuncs::IsFormValid(race)) {
				races.push_back(race);
			}
		}
		return races;
	}

	bool ArmorAddonHasRace(RE::StaticFunctionTag*, RE::TESObjectARMA* armorAddon, RE::TESRace* akRace) {
		if (!gfuncs::IsFormValid(armorAddon)) {
			return false;
		}

		if (!gfuncs::IsFormValid(akRace)) {
			return false;
		}

		for (auto* race : armorAddon->additionalRaces) {
			if (race == akRace) {
				return true;
			}
		}

		RE::TESRaceForm* raceForm = skyrim_cast<RE::TESRaceForm*>(armorAddon);
		if (raceForm) {
			if (gfuncs::IsFormValid(raceForm->race)) {
				if (raceForm->race == akRace) {
					return true;
				}
			}
		}

		return false;
	}

	void AddAdditionalRaceToArmorAddon(RE::StaticFunctionTag*, RE::TESObjectARMA* armorAddon, RE::TESRace* akRace) {
		if (!gfuncs::IsFormValid(armorAddon)) {
			return;
		}

		if (!gfuncs::IsFormValid(akRace)) {
			return;
		}

		bool hasRace = false;
		for (auto* race : armorAddon->additionalRaces) {
			if (race == akRace) {
				hasRace = true;
				break;
			}
		}
		if (!hasRace) {
			armorAddon->additionalRaces.push_back(akRace);
		}
	}

	void RemoveAdditionalRaceFromArmorAddon(RE::StaticFunctionTag*, RE::TESObjectARMA* armorAddon, RE::TESRace* akRace) {
		if (!gfuncs::IsFormValid(armorAddon)) {
			return;
		}

		if (!gfuncs::IsFormValid(akRace)) {
			return;
		}

		int i = 0;
		for (int i = 0; i < armorAddon->additionalRaces.size(); i++) {
			auto* race = armorAddon->additionalRaces[i];
			if (race == akRace) {
				armorAddon->additionalRaces.erase((armorAddon->additionalRaces.begin() + i));
				break;
			}
			i++;
		}
	}

	bool ArmorSlotMaskHasPartOf(RE::StaticFunctionTag*, RE::TESObjectARMO* armor, uint32_t slotMask) {
		if (gfuncs::IsFormValid(armor)) {
			RE::BGSBipedObjectForm* bipedForm = skyrim_cast<RE::BGSBipedObjectForm*>(armor);
			if (bipedForm) {
				RE::BGSBipedObjectForm::BipedObjectSlot bipedSlot = static_cast<RE::BGSBipedObjectForm::BipedObjectSlot>(slotMask);
				return bipedForm->HasPartOf(bipedSlot);
			}
		}
		return false;
	}

	bool ArmorAddonSlotMaskHasPartOf(RE::StaticFunctionTag*, RE::TESObjectARMA* armorAddon, uint32_t slotMask) {
		if (gfuncs::IsFormValid(armorAddon)) {
			RE::BGSBipedObjectForm* bipedForm = skyrim_cast<RE::BGSBipedObjectForm*>(armorAddon);
			if (bipedForm) {
				RE::BGSBipedObjectForm::BipedObjectSlot bipedSlot = static_cast<RE::BGSBipedObjectForm::BipedObjectSlot>(slotMask);
				return bipedForm->HasPartOf(bipedSlot);
			}
		}
		return false;
	}

	bool RaceSlotMaskHasPartOf(RE::StaticFunctionTag*, RE::TESRace* race, uint32_t slotMask) {
		if (gfuncs::IsFormValid(race)) {
			RE::BGSBipedObjectForm* bipedForm = skyrim_cast<RE::BGSBipedObjectForm*>(race);
			if (bipedForm) {
				RE::BGSBipedObjectForm::BipedObjectSlot bipedSlot = static_cast<RE::BGSBipedObjectForm::BipedObjectSlot>(slotMask);
				return bipedForm->HasPartOf(bipedSlot);
			}
		}
		return false;
	}

	uint32_t GetRaceSlotMask(RE::StaticFunctionTag*, RE::TESRace* race) {
		if (gfuncs::IsFormValid(race)) {
			RE::BGSBipedObjectForm* bipedForm = skyrim_cast<RE::BGSBipedObjectForm*>(race);
			if (bipedForm) {
				uint32_t slotmask = static_cast<uint32_t>(bipedForm->GetSlotMask());
				//logger::critical("race[{}] [{}] [{:x}]", race->GetName(), slotmask, slotmask);
				return slotmask;
			}
		}
		return -1;
	}

	void SetRaceSlotMask(RE::StaticFunctionTag*, RE::TESRace* race, uint32_t slotMask) {
		if (gfuncs::IsFormValid(race)) {
			RE::BGSBipedObjectForm* bipedForm = skyrim_cast<RE::BGSBipedObjectForm*>(race);
			if (bipedForm) {
				RE::BGSBipedObjectForm::BipedObjectSlot bipedSlot = static_cast<RE::BGSBipedObjectForm::BipedObjectSlot>(slotMask);
				bipedForm->SetSlotMask(bipedSlot);
			}
		}
	}

	void AddRaceSlotToMask(RE::StaticFunctionTag*, RE::TESRace* race, uint32_t slotMask) {
		if (gfuncs::IsFormValid(race)) {
			RE::BGSBipedObjectForm* bipedForm = skyrim_cast<RE::BGSBipedObjectForm*>(race);
			if (bipedForm) {
				RE::BGSBipedObjectForm::BipedObjectSlot bipedSlot = static_cast<RE::BGSBipedObjectForm::BipedObjectSlot>(slotMask);
				bipedForm->AddSlotToMask(bipedSlot);
			}
		}
	}

	void RemoveRaceSlotFromMask(RE::StaticFunctionTag*, RE::TESRace* race, uint32_t slotMask) {
		if (gfuncs::IsFormValid(race)) {
			RE::BGSBipedObjectForm* bipedForm = skyrim_cast<RE::BGSBipedObjectForm*>(race);
			if (bipedForm) {
				RE::BGSBipedObjectForm::BipedObjectSlot bipedSlot = static_cast<RE::BGSBipedObjectForm::BipedObjectSlot>(slotMask);
				bipedForm->RemoveSlotFromMask(bipedSlot);
			}
		}
	}

	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
		vm->RegisterFunction("GetArmorAddonRaces", "DbSkseFunctions", GetArmorAddonRaces);
		vm->RegisterFunction("ArmorAddonHasRace", "DbSkseFunctions", ArmorAddonHasRace);
		vm->RegisterFunction("AddAdditionalRaceToArmorAddon", "DbSkseFunctions", AddAdditionalRaceToArmorAddon);
		vm->RegisterFunction("RemoveAdditionalRaceFromArmorAddon", "DbSkseFunctions", RemoveAdditionalRaceFromArmorAddon);
		vm->RegisterFunction("ArmorSlotMaskHasPartOf", "DbSkseFunctions", ArmorSlotMaskHasPartOf);
		vm->RegisterFunction("ArmorAddonSlotMaskHasPartOf", "DbSkseFunctions", ArmorAddonSlotMaskHasPartOf);
		vm->RegisterFunction("RaceSlotMaskHasPartOf", "DbSkseFunctions", RaceSlotMaskHasPartOf);
		vm->RegisterFunction("GetRaceSlotMask", "DbSkseFunctions", GetRaceSlotMask);
		vm->RegisterFunction("SetRaceSlotMask", "DbSkseFunctions", SetRaceSlotMask);
		vm->RegisterFunction("AddRaceSlotToMask", "DbSkseFunctions", AddRaceSlotToMask);
		vm->RegisterFunction("RemoveRaceSlotFromMask", "DbSkseFunctions", RemoveRaceSlotFromMask);
		return true;
	}
}