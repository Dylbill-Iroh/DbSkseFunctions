#pragma once

extern std::map<RE::TESObjectBOOK*, int> skillBooksMap;

void SetBookSpell(RE::StaticFunctionTag*, RE::TESObjectBOOK* akBook, RE::SpellItem* akSpell);

RE::TESObjectBOOK* GetSpellTomeForSpell(RE::StaticFunctionTag*, RE::SpellItem * akSpell);

std::vector<RE::TESObjectBOOK*> GetSpellTomesForSpell(RE::StaticFunctionTag*, RE::SpellItem * akSpell);

void AddSpellTomesForSpellToList(RE::StaticFunctionTag*, RE::SpellItem * akSpell, RE::BGSListForm * akList);

std::string GetBookSkill(RE::StaticFunctionTag*, RE::TESObjectBOOK * akBook);

void SetBookSkillInt(RE::TESObjectBOOK * akBook, int value, std::string skill = "", bool addToSkillBooksMap = true);

void SetBookSkill(RE::StaticFunctionTag*, RE::TESObjectBOOK * akBook, std::string actorValue);

std::vector<RE::TESObjectBOOK*> GetSkillBooksForSkill(RE::StaticFunctionTag*, std::string actorValue);

void AddSkillBookForSkillToList(RE::StaticFunctionTag*, std::string actorValue, RE::BGSListForm * akList);

void SetBookRead(RE::StaticFunctionTag*, RE::TESObjectBOOK * akBook, bool read);

void SetAllBooksRead(RE::StaticFunctionTag*, bool read);

void SaveSkillBooks();

