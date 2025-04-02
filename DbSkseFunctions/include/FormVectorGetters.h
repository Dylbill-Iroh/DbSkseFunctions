#pragma once
std::vector<RE::TESObjectREFR*> GetAllContainerRefsThatContainForm(RE::StaticFunctionTag*, RE::TESForm* akForm);

std::vector<RE::TESForm*> GetAllFormsThatUseTextureSet(RE::StaticFunctionTag*, RE::BGSTextureSet* akTextureSet, std::string modName);

std::vector<RE::TESQuest*> GetAllActiveQuests(RE::StaticFunctionTag*);

std::vector<RE::TESForm*> GetAllConstructibleObjects(RE::StaticFunctionTag*, RE::TESForm * createdObject);

std::vector<RE::TESObjectARMO*> GetAllArmorsForSlotMask(RE::StaticFunctionTag*, int slotMask);

std::vector<RE::TESObjectCELL*> GetAllInteriorCells(RE::StaticFunctionTag*, RE::BGSLocation * akLocation, RE::TESNPC * akOwner, int matchMode);

std::vector<RE::TESObjectCELL*> GetAllExteriorCells(RE::StaticFunctionTag*, RE::BGSLocation * akLocation, RE::TESWorldSpace * akWorldSpace, int matchMode);

std::vector<RE::TESObjectCELL*> GetAttachedCells(RE::StaticFunctionTag*);

std::vector<RE::TESForm*> GetFavorites(RE::StaticFunctionTag*, std::vector<int> formTypes, int formTypeMatchMode);

std::vector<RE::TESForm*> GetAllFormsWithName(RE::StaticFunctionTag*, std::string sFormName, int nameMatchMode, std::vector<int> formTypes, int formTypeMatchMode);

std::vector<RE::TESForm*> GetAllFormsWithScriptAttached(RE::StaticFunctionTag*, RE::BSFixedString sScriptName, std::vector<int> formTypes, int formTypeMatchMode);

std::vector<RE::BGSBaseAlias*> GetAllAliasesWithScriptAttached(RE::StaticFunctionTag*, RE::BSFixedString sScriptName);

std::vector<RE::BGSRefAlias*> GetAllRefAliasesWithScriptAttached(RE::StaticFunctionTag*, RE::BSFixedString sScriptName, bool onlyQuestObjects, bool onlyFilled);

std::vector<RE::BGSRefAlias*> GetAllRefaliases(RE::StaticFunctionTag*, bool onlyQuestObjects, bool onlyFilled);

std::vector<RE::BGSRefAlias*> GetAllRefAliasesForRef(RE::StaticFunctionTag*, RE::TESObjectREFR * ref);

std::vector<RE::TESObjectREFR*> GetAllQuestObjectRefs(RE::StaticFunctionTag*);

std::vector<RE::TESObjectREFR*> GetQuestObjectRefsInContainer(RE::StaticFunctionTag*, RE::TESObjectREFR * containerRef);

std::vector<RE::TESObjectREFR*> GetAllObjectRefsInContainer(RE::StaticFunctionTag*, RE::TESObjectREFR * containerRef);

std::vector<RE::EnchantmentItem*> GetKnownEnchantments(RE::StaticFunctionTag*);

void AddKnownEnchantmentsToFormList(RE::StaticFunctionTag*, RE::BGSListForm* akList);
