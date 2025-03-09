#include "Book.h"
#include "GeneralFunctions.h"
#include "Actor.h"

namespace logger = SKSE::log;

std::map<RE::TESObjectBOOK*, int> skillBooksMap;

void SetBookSpell(RE::StaticFunctionTag*, RE::TESObjectBOOK* akBook, RE::SpellItem* akSpell) {
    logger::debug("called");

    if (!gfuncs::IsFormValid(akBook)) {
        logger::warn("akBook doesn't exist.");
        return;
    }

    if (!gfuncs::IsFormValid(akSpell)) {
        akBook->data.flags.reset(RE::OBJ_BOOK::Flag::kTeachesSpell);
        logger::debug("akSpell is none, removing teaches spell flag");
        return;
    }

    else {
        akBook->data.teaches.spell = akSpell;
        akBook->data.flags.reset(RE::OBJ_BOOK::Flag::kTeachesSpell);
        akBook->data.flags |= RE::OBJ_BOOK::Flag::kTeachesSpell;
    }
}

RE::TESObjectBOOK* GetSpellTomeForSpell(RE::StaticFunctionTag*, RE::SpellItem* akSpell) {
    if (!gfuncs::IsFormValid(akSpell)) {
        logger::warn("akSpell doesn't exist.");
        return nullptr;
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        int ic = 0;
        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                if (gfuncs::IsFormValid(akBook)) {
                    if (akBook->TeachesSpell()) {
                        if (akBook->GetSpell() == akSpell) {
                            return akBook;
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

std::vector<RE::TESObjectBOOK*> GetSpellTomesForSpell(RE::StaticFunctionTag*, RE::SpellItem* akSpell) {
    std::vector<RE::TESObjectBOOK*> v;

    if (!gfuncs::IsFormValid(akSpell)) {
        logger::warn("akSpell doesn't exist.");
        return v;
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        int ic = 0;
        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                if (gfuncs::IsFormValid(akBook)) {
                    if (akBook->TeachesSpell()) {
                        if (akBook->GetSpell() == akSpell) {
                            v.push_back(akBook);
                        }
                    }
                }
            }
        }
    }
    return v;
}

void AddSpellTomesForSpellToList(RE::StaticFunctionTag*, RE::SpellItem* akSpell, RE::BGSListForm* akList) {
    if (!gfuncs::IsFormValid(akSpell)) {
        logger::warn("akSpell doesn't exist.");
        return;
    }

    if (!gfuncs::IsFormValid(akList)) {
        logger::warn("akList doesn't exist.");
        return;
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        int ic = 0;
        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                if (gfuncs::IsFormValid(akBook)) {
                    if (akBook->TeachesSpell()) {
                        if (akBook->GetSpell() == akSpell) {
                            akList->AddForm(akBook);
                        }
                    }
                }
            }
        }
    }
}

std::string GetBookSkill(RE::StaticFunctionTag*, RE::TESObjectBOOK* akBook) {
    if (!gfuncs::IsFormValid(akBook)) {
        logger::warn("akBook doesn't exist.");
        return "";
    }

    if (skillBooksMap.find(akBook) != skillBooksMap.end()) {
        return ActorValueIntsMap[(skillBooksMap[akBook])];
    }
    return "";
}

void SetBookSkillInt(RE::TESObjectBOOK* akBook, int value, std::string skill, bool addToSkillBooksMap) {
    if (value == -1) {
        akBook->data.flags.reset(RE::OBJ_BOOK::Flag::kAdvancesActorValue);
        akBook->RemoveChange(RE::TESObjectBOOK::ChangeFlags::kTeachesSkill);
        if (addToSkillBooksMap) {
            skillBooksMap[akBook] = value;
        }
        logger::debug("book[{}] ID[{:x}] no longer teaches skill", gfuncs::GetFormName(akBook), akBook->GetFormID());
        return;
    }
    else if (value < -1 || value > 163) {
        logger::debug("skill[{}] value[{}] not recognized", skill, value);
        return;
    }

    akBook->data.teaches.actorValueToAdvance = static_cast<RE::ActorValue>(value);
    if (addToSkillBooksMap) {
        skillBooksMap[akBook] = value;
    }
    akBook->data.flags.set(RE::OBJ_BOOK::Flag::kAdvancesActorValue);
    akBook->AddChange(RE::TESObjectBOOK::ChangeFlags::kTeachesSkill);
}

void SetBookSkill(RE::StaticFunctionTag*, RE::TESObjectBOOK* akBook, std::string actorValue) {
    logger::debug("called");

    if (!gfuncs::IsFormValid(akBook)) {
        logger::warn("akBook doesn't exist.");
        return;
    }

    int value = GetActorValueInt(actorValue);
    SetBookSkillInt(akBook, value, actorValue);
}

std::vector<RE::TESObjectBOOK*> GetSkillBooksForSkill(RE::StaticFunctionTag*, std::string actorValue) {
    std::vector<RE::TESObjectBOOK*> v;

    int value = GetActorValueInt(actorValue);
    logger::debug("actorValue = {} int value = {}", actorValue, value);

    if (value < 0) {
        logger::warn("actorValue [{}] not recognized", actorValue);
        return v;
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        int ic = 0;
        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                if (gfuncs::IsFormValid(akBook)) {
                    if (skillBooksMap.find(akBook) != skillBooksMap.end()) {
                        if (skillBooksMap[akBook] == value) {
                            v.push_back(akBook);
                        }
                    }
                }
            }
        }
    }
    return v;
}

void AddSkillBookForSkillToList(RE::StaticFunctionTag*, std::string actorValue, RE::BGSListForm* akList) {
    if (!gfuncs::IsFormValid(akList)) {
        logger::warn("akList doesn't exist.");
        return;
    }

    int value = GetActorValueInt(actorValue);
    logger::debug("actorValue = {} int value = {}", actorValue, value);

    if (value < 0) {
        logger::warn("actorValue [{}] not recognized", actorValue);
        return;
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        int ic = 0;
        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                if (gfuncs::IsFormValid(akBook)) {
                    if (skillBooksMap.find(akBook) != skillBooksMap.end()) {
                        if (skillBooksMap[akBook] == value) {
                            akList->AddForm(akBook);
                        }
                    }
                }
            }
        }
    }
}

void SetBookRead(RE::StaticFunctionTag*, RE::TESObjectBOOK* akBook, bool read) {
    if (!gfuncs::IsFormValid(akBook)) {
        logger::warn("akBook doesn't exist.");
        return;
    }

    if (read) {
        if (!akBook->IsRead()) {
            akBook->data.flags.set(RE::OBJ_BOOK::Flag::kHasBeenRead);
            akBook->AddChange(RE::TESObjectBOOK::ChangeFlags::kRead);

            if (skillBooksMap.find(akBook) != skillBooksMap.end()) { //book is a skill book
                SetBookSkillInt(akBook, -1, "", false);
            }
        }
    }
    else {
        if (akBook->IsRead()) {
            akBook->data.flags.reset(RE::OBJ_BOOK::Flag::kHasBeenRead);
            akBook->RemoveChange(RE::TESObjectBOOK::ChangeFlags::kRead);

            if (skillBooksMap.find(akBook) != skillBooksMap.end()) { //book is a skill book
                SetBookSkillInt(akBook, skillBooksMap[akBook]);
            }
        }
    }
}

void SetAllBooksRead(RE::StaticFunctionTag*, bool read) {
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        int ic = 0;
        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                SetBookRead(nullptr, akBook, read);
            }
        }
    }
}

//when reading a skill book in game, it removes the skill from the book, not just the TeachesSkill flag
//this saves skill books and their respective skills for use with skill book functions below.
void SaveSkillBooks() {
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));
        int ic = 0;

        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                if (gfuncs::IsFormValid(akBook)) {
                    if (akBook->TeachesSkill()) {
                        skillBooksMap[akBook] = static_cast<int>(akBook->GetSkill());
                    }
                }
            }
        }
    }
    //gfuncs::logFormMap(skillBooksMap);
}