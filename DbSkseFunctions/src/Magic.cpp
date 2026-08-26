#include <Windows.h>
#include "Magic.h"
#include "GeneralFunctions.h"
#include "ConsoleUtil.h"
#include "Utility.h"
#include "SharedVariables.h"

namespace magic {
    std::string GetWordOfPowerTranslation(RE::StaticFunctionTag*, RE::TESWordOfPower* akWord) {
        if (!gfuncs::IsFormValid(akWord)) {
            logger::warn("akWord doesn't exist.");
            return "";
        }

        return static_cast<std::string>(akWord->translation);
    }

    void UnlockShout(RE::StaticFunctionTag*, RE::TESShout* akShout) {
        if (!gfuncs::IsFormValid(akShout)) {
            logger::warn("akShout doesn't exist.");
            return;
        }

        if (!sv::player) {
            logger::error("sv::player* not found. shout[{}] not unlocked", gfuncs::GetFormDataString(akShout));
            return;
        }

        sv::player->AddShout(akShout);

        logger::debug("{} ID {:x}", akShout->GetName(), akShout->GetFormID());

        RE::TESWordOfPower* word = akShout->variations[0].word;
        if (gfuncs::IsFormValid(word)) {
            //gfuncs::gfuncs::install->UnlockWord(word);
            logger::debug("unlock word 1 {} ID {:x}", word->GetName(), word->GetFormID());
            std::string command = "sv::player.teachword " + gfuncs::IntToHex(int(word->GetFormID())); //didn't find a teachword function in NG, so using console command as workaround. 
            ConsoleUtil::ExecuteCommand(command, nullptr);
            sv::player->UnlockWord(word);
        }

        word = akShout->variations[1].word;
        if (gfuncs::IsFormValid(word)) {
            //playerRef->UnlockWord(word);
            logger::debug("unlock word 2 {} ID {:x}", word->GetName(), word->GetFormID());
            std::string command = "sv::player.teachword " + gfuncs::IntToHex(int(word->GetFormID()));
            ConsoleUtil::ExecuteCommand(command, nullptr);
            sv::player->UnlockWord(word);
        }

        word = akShout->variations[2].word;
        if (gfuncs::IsFormValid(word)) {
            //playerRef->UnlockWord(word);
            logger::debug("unlock word 3 {} ID {:x}", word->GetName(), word->GetFormID());
            std::string command = "sv::player.teachword " + gfuncs::IntToHex(int(word->GetFormID()));
            ConsoleUtil::ExecuteCommand(command, nullptr);
            sv::player->UnlockWord(word);
        }
    }

    void AddAndUnlockAllShouts(RE::StaticFunctionTag*, int minNumberOfWordsWithTranslations, bool onlyShoutsWithNames, bool onlyShoutsWithDescriptions) {
        bool minNumberOfWordsCheck = (minNumberOfWordsWithTranslations > 0 && minNumberOfWordsWithTranslations <= 3);

        if (sv::dataHandler) {
            RE::BSTArray<RE::TESForm*>* akArray = &(sv::dataHandler->GetFormArray(RE::FormType::Shout));

            int ic = 0;
            //loop through all shouts
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESShout* akShout = baseForm->As<RE::TESShout>();
                    if (gfuncs::IsFormValid(akShout)) {
                        if (onlyShoutsWithNames) {
                            if (akShout->GetName() == "") {
                                continue;
                            }
                        }

                        if (onlyShoutsWithDescriptions) {
                            if (GetDescription(akShout, "") == "") {
                                continue;
                            }
                        }

                        if (minNumberOfWordsCheck) {
                            RE::TESWordOfPower* word = akShout->variations[minNumberOfWordsWithTranslations - 1].word;
                            if (GetWordOfPowerTranslation(nullptr, word) == "") {
                                continue;
                            }
                        }
                        UnlockShout(nullptr, akShout);
                    }
                }
            }
        }
    }

    int GetActiveMagicEffectConditionStatus(RE::StaticFunctionTag*, RE::ActiveEffect* akEffect) {
        if (!akEffect) {
            logger::warn("akEffect doesn't exist");
            return -1;
        }

        if (akEffect->conditionStatus.any(RE::ActiveEffect::ConditionStatus::kTrue)) {
            //logger::trace("conditionStatus is kTrue");
            return 1;
        }
        else if (akEffect->conditionStatus.any(RE::ActiveEffect::ConditionStatus::kFalse)) {
            //logger::trace("conditionStatus is kFalse");
            return 0;
        }
        else {
            return -1;
        }
    }

    RE::TESForm* GetActiveEffectSource(RE::StaticFunctionTag*, RE::ActiveEffect* akEffect) {
        if (!akEffect) {
            logger::warn("akEffect doesn't exist");
            return nullptr;
        }

        return akEffect->spell;
    }

    int GetActiveEffectCastingSource(RE::StaticFunctionTag*, RE::ActiveEffect* akEffect) {
        if (!akEffect) {
            logger::warn("akEffect doesn't exist");
            return -1;
        }

        return static_cast<int>(akEffect->castingSource);
    }

    std::vector<RE::EffectSetting*> GetMagicEffectsForForm(RE::StaticFunctionTag*, RE::TESForm* akForm) {
        std::vector<RE::EffectSetting*> akEffects;
        if (!gfuncs::IsFormValid(akForm)) {
            logger::warn("akForm doesn't exist");
            return akEffects;
        }

        RE::MagicItem* magicItem = akForm->As<RE::MagicItem>();
        if (!gfuncs::IsFormValid(magicItem)) {
            logger::warn("akForm name[{}] editorID[{}] formID[{}], is not a magic item", gfuncs::GetFormName(akForm), GetFormEditorId(nullptr, akForm, ""), akForm->GetFormID());
            return akEffects;
        }

        for (int i = 0; i < magicItem->effects.size(); i++) {
            RE::EffectSetting* effect = magicItem->effects[i]->baseEffect;
            if (gfuncs::IsFormValid(effect)) {
                akEffects.push_back(effect);
            }
        }
        return akEffects;
    }

    bool IsFormMagicItem(RE::StaticFunctionTag*, RE::TESForm* akForm) {
        if (!gfuncs::IsFormValid(akForm)) {
            logger::warn("akForm doesn't exist");
            return false;
        }

        RE::MagicItem* magicItem = akForm->As<RE::MagicItem>();
        if (gfuncs::IsFormValid(magicItem)) {
            return true;
        }
        else {
            return false;
        }
    }

    bool IsMagicEffectActiveOnRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, RE::EffectSetting* akMagicEffect, RE::TESForm* magicSource) {
        //logger::trace("called");

        if (!gfuncs::IsFormValid(ref)) {
            return false;
        }

        if (!gfuncs::IsFormValid(akMagicEffect)) {
            return false;
        }

        RE::MagicTarget* magicTarget = ref->GetMagicTarget();

        if (magicTarget) {
            auto activeEffects = magicTarget->GetActiveEffectList();
            if (activeEffects) {

                if (gfuncs::IsFormValid(magicSource)) {
                    for (RE::ActiveEffect* effect : *activeEffects) {
                        if (effect) {
                            if (!IsBadReadPtr(effect, sizeof(effect))) {
                                auto* baseEffect = effect->GetBaseObject();
                                if (gfuncs::IsFormValid(baseEffect)) {
                                    if (baseEffect == akMagicEffect && effect->spell == magicSource) {
                                        if (effect->conditionStatus.any(RE::ActiveEffect::ConditionStatus::kTrue)) {
                                            logger::trace("conditionStatus for effect[{}] from source[{}] on ref[{}] is True",
                                                __func__, gfuncs::GetFormName(baseEffect), gfuncs::GetFormName(magicSource), gfuncs::GetFormName(ref));

                                            return true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else {
                    for (RE::ActiveEffect* effect : *activeEffects) {
                        if (effect) {
                            if (!IsBadReadPtr(effect, sizeof(effect))) {
                                auto* baseEffect = effect->GetBaseObject();
                                if (gfuncs::IsFormValid(baseEffect)) {
                                    if (baseEffect == akMagicEffect) {
                                        if (effect->conditionStatus.any(RE::ActiveEffect::ConditionStatus::kTrue)) {
                                            logger::trace("conditionStatus for effect[{}] on ref[{}] is True",
                                                __func__, gfuncs::GetFormName(baseEffect), gfuncs::GetFormName(ref));

                                            return true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return false;
    }

    //Function DispelMagicEffectOnRef(ObjectReference ref, MagicEffect akMagicEffect, Form magicSource = none) Global Native
    void DispelMagicEffectOnRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, RE::EffectSetting* akMagicEffect, RE::TESForm* magicSource) {
        //logger::trace("called");

        if (!gfuncs::IsFormValid(ref)) {
            return;
        }

        if (!gfuncs::IsFormValid(akMagicEffect)) {
            return;
        }

        auto effectName = gfuncs::GetFormName(akMagicEffect);
        auto refName = gfuncs::GetFormName(ref);

        RE::MagicTarget* magicTarget = ref->GetMagicTarget();

        if (magicTarget) {
            //logger::trace("magicTarget found");

            auto activeEffects = magicTarget->GetActiveEffectList();
            if (activeEffects) {
                if (gfuncs::IsFormValid(magicSource)) {
                    auto sourceName = gfuncs::GetFormName(magicSource);
                    for (RE::ActiveEffect* effect : *activeEffects) {
                        if (effect) {
                            if (!IsBadReadPtr(effect, sizeof(effect))) {
                                auto* baseEffect = effect->GetBaseObject();
                                if (gfuncs::IsFormValid(baseEffect)) {
                                    if (baseEffect == akMagicEffect && magicSource == effect->spell) {
                                        logger::debug("dispelling effect[{}] from source [{}] on ref[{}]", effectName, sourceName, refName);
                                        effect->Dispel(true);
                                    }
                                }
                            }
                        }
                    }
                }
                else {
                    for (RE::ActiveEffect* effect : *activeEffects) {
                        if (effect) {
                            if (!IsBadReadPtr(effect, sizeof(effect))) {
                                auto* baseEffect = effect->GetBaseObject();
                                if (gfuncs::IsFormValid(baseEffect)) {
                                    if (baseEffect == akMagicEffect) {
                                        logger::debug("dispelling effect[{}] on ref[{}]", effectName, refName);
                                        effect->Dispel(true);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    //kNone = 0,
    //kPetty = 1,
    //kLesser = 2,
    //kCommon = 3,
    //kGreater = 4,
    //kGrand = 5
    void SetSoulGemSize(RE::StaticFunctionTag*, RE::TESSoulGem* akGem, int level) {
        logger::trace("called");

        if (!gfuncs::IsFormValid(akGem)) {
            logger::warn("error akGem doesn't exist");
            return;
        }

        if (level < 0) {
            level = 0;
        }
        else if (level > 5) {
            level = 5;
        }

        akGem->soulCapacity = static_cast<RE::SOUL_LEVEL>(level);
    }

    void SetSoulGemCanHoldNPCSoul(RE::StaticFunctionTag*, RE::TESSoulGem* akGem, bool canCarry) {
        logger::debug("called");

        if (!gfuncs::IsFormValid(akGem)) {
            logger::warn("error akGem doesn't exist");
            return;
        }

        if (canCarry) {
            akGem->formFlags |= RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
        }
        else {
            akGem->formFlags &= ~RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
        }
    }

    bool CanSoulGemHoldNPCSoul(RE::StaticFunctionTag*, RE::TESSoulGem* akGem) {
        logger::debug("called");

        if (!gfuncs::IsFormValid(akGem)) {
            logger::warn("error akGem doesn't exist");
            return false;
        }
        return akGem->CanHoldNPCSoul();
    }

    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("GetWordOfPowerTranslation", "DbSkseFunctions", GetWordOfPowerTranslation);
        vm->RegisterFunction("UnlockShout", "DbSkseFunctions", UnlockShout);
        vm->RegisterFunction("AddAndUnlockAllShouts", "DbSkseFunctions", AddAndUnlockAllShouts);
        vm->RegisterFunction("GetActiveMagicEffectConditionStatus", "DbSkseFunctions", GetActiveMagicEffectConditionStatus);
        vm->RegisterFunction("GetActiveEffectSource", "DbSkseFunctions", GetActiveEffectSource);
        vm->RegisterFunction("GetActiveEffectCastingSource", "DbSkseFunctions", GetActiveEffectCastingSource);
        vm->RegisterFunction("GetMagicEffectsForForm", "DbSkseFunctions", GetMagicEffectsForForm);
        vm->RegisterFunction("IsFormMagicItem", "DbSkseFunctions", IsFormMagicItem);
        vm->RegisterFunction("IsMagicEffectActiveOnRef", "DbSkseFunctions", IsMagicEffectActiveOnRef);
        vm->RegisterFunction("DispelMagicEffectOnRef", "DbSkseFunctions", DispelMagicEffectOnRef);
        vm->RegisterFunction("SetSoulGemSize", "DbSkseFunctions", SetSoulGemSize);
        vm->RegisterFunction("CanSoulGemHoldNPCSoul", "DbSkseFunctions", CanSoulGemHoldNPCSoul);
        vm->RegisterFunction("SetSoulGemCanHoldNPCSoul", "DbSkseFunctions", SetSoulGemCanHoldNPCSoul);

        return true;
    }
}