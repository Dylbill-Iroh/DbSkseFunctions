#include "FormVectorGetters.h"
#include "GeneralFunctions.h"
#include "Utility.h"

std::vector<RE::TESObjectREFR*> GetEnableChildrenRefs(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
    std::vector<RE::TESObjectREFR*> v;
    if (!gfuncs::IsFormValid(ref)) {
        logger::error("ref doesn't exist");
        return v;
    }

    auto* data = ref->extraList.GetByType<RE::ExtraEnableStateChildren>();
    if (data) {
        for (auto& refHandle : data->children) {
            auto* childRef = gfuncs::GetRefFromObjectRefHandle(refHandle);
            if (childRef) {
                v.push_back(childRef);
            }
        }
    }

    return v;
}

std::vector<RE::TESObjectREFR*> GetAllContainerRefsThatContainForm(RE::StaticFunctionTag*, RE::TESForm* akForm) {
    std::vector<RE::TESObjectREFR*> v; 
    if (!gfuncs::IsFormValid(akForm)) {
        logger::error("akTextureSet doesn't exist");
        return v;
    } 

    const auto& [allForms, lock] = RE::TESForm::GetAllForms();
    for (auto& [id, form] : *allForms) {
        if (gfuncs::IsFormValid(form)) {
            RE::TESObjectREFR* ref = form->AsReference();
            if (gfuncs::IsFormValid(ref)) {
                if (gfuncs::GetItemCount(ref, akForm) > 0) {
                    v.push_back(ref);
                }
            }
        }
    }
    return v;
}

std::vector<RE::TESForm*> GetAllFormsThatUseTextureSet(RE::StaticFunctionTag*, RE::BGSTextureSet* akTextureSet, std::string modName) {
    std::vector<RE::TESForm*> v;
    if (!gfuncs::IsFormValid(akTextureSet)) {
        logger::error("akTextureSet doesn't exist");
        return v;
    }

    logger::info("Getting forms from [{}] that use textureSet({})", modName, gfuncs::GetFormDataString(akTextureSet));

    gfuncs::ConvertToLowerCase(modName);
    const auto& [allForms, lock] = RE::TESForm::GetAllForms();

    for (auto& [id, form] : *allForms) {
        if (gfuncs::IsFormValid(form)) {
            auto file = form->GetFile(0);
            if (file) {
                std::string sfileName = std::string(file->GetFilename());
                gfuncs::ConvertToLowerCase(sfileName);
                if (sfileName == modName || modName == "") {
                    RE::TESModelTextureSwap* swap = form->As<RE::TESModelTextureSwap>();
                    if (swap) {
                        if (swap->alternateTextures) {
                            bool formFound = false;
                            std::string sFormData = "";
                            for (int i = 0; i < swap->numAlternateTextures; i++) {
                                RE::BGSTextureSet* ts = swap->alternateTextures[i].textureSet;
                                if (swap->alternateTextures[i].textureSet == akTextureSet) {
                                    if (!formFound) {
                                        formFound = true;
                                        sFormData = gfuncs::GetFormDataString(form);
                                        v.push_back(form);
                                    }
                                    logger::info("form({}) index[{}] 3dName[{}] from mod[{}] uses ts",
                                        sFormData, swap->alternateTextures[i].index3D, swap->alternateTextures[i].name3D, sfileName);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return v;
}

std::vector<RE::TESQuest*> GetAllActiveQuests(RE::StaticFunctionTag*) {
    logger::debug("called");

    std::vector<RE::TESQuest*> questItems;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Quest));

        int ic = 0;
        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
            RE::TESForm* baseForm = *itr;
            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                if (gfuncs::IsFormValid(quest)) {
                    if (quest->IsActive()) {
                        questItems.push_back(quest);
                    }
                }
            }
        }
    }
    return questItems;
}

std::vector<RE::TESForm*> GetAllConstructibleObjects(RE::StaticFunctionTag*, RE::TESForm* createdObject) {
    std::vector<RE::TESForm*> forms;
    const auto& [allForms, lock] = RE::TESForm::GetAllForms();

    if (gfuncs::IsFormValid(createdObject)) {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false)) {
                RE::BGSConstructibleObject* object = form->As<RE::BGSConstructibleObject>();
                if (gfuncs::IsFormValid(object, false)) {
                    if (object->createdItem == createdObject) {
                        forms.push_back(form);
                    }
                }
            }
        }
    }
    else {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                RE::BGSConstructibleObject* object = form->As<RE::BGSConstructibleObject>();
                if (gfuncs::IsFormValid(object)) {
                    forms.push_back(form);
                }
            }
        }
    }
    return forms;
}

std::vector<RE::TESObjectARMO*> GetAllArmorsForSlotMask(RE::StaticFunctionTag*, int slotMask) {
    std::vector<RE::TESObjectARMO*> armors;

    const auto& [allForms, lock] = RE::TESForm::GetAllForms();

    for (auto& [id, form] : *allForms) {
        if (gfuncs::IsFormValid(form)) {
            RE::TESObjectARMO* armor = form->As<RE::TESObjectARMO>();
            if (gfuncs::IsFormValid(armor)) {
                int mask = static_cast<int>(armor->GetSlotMask());
                if (mask == slotMask) {
                    armors.push_back(armor);
                }
            }
        }
    }

    return armors;
}

//TESNPC is ActorBase in papyrus
std::vector<RE::TESObjectCELL*> GetAllInteriorCells(RE::StaticFunctionTag*, RE::BGSLocation* akLocation, RE::TESNPC* akOwner, int matchMode) {
    //logger::trace("called");
    std::vector<RE::TESObjectCELL*> cells;

    if (!gfuncs::IsFormValid(akLocation)) {
        akLocation = nullptr;
    }

    if (!gfuncs::IsFormValid(akOwner)) {
        akOwner = nullptr;
    }

    const auto& [allForms, lock] = RE::TESForm::GetAllForms();

    if (matchMode == 1) {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
                if (gfuncs::IsFormValid(cell)) {
                    if (cell->IsInteriorCell()) {
                        RE::BGSLocation* location = cell->GetLocation();
                        RE::TESNPC* npc = cell->GetActorOwner();

                        if (!gfuncs::IsFormValid(location)) {
                            location = nullptr;
                        }

                        if (!gfuncs::IsFormValid(npc)) {
                            npc = nullptr;
                        }

                        if (npc == akOwner && location == akLocation) {
                            cells.push_back(cell);
                        }
                    }
                }
            }
        }
    }
    else if (matchMode == 0) {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
                if (gfuncs::IsFormValid(cell)) {
                    if (cell->IsInteriorCell()) {
                        bool matched = false;
                        RE::BGSLocation* location = cell->GetLocation();
                        if (gfuncs::IsFormValid(location)) {
                            if (location == akLocation) {
                                matched = true;
                                cells.push_back(cell);
                            }
                        }
                        if (!matched) {
                            RE::TESNPC* npc = cell->GetActorOwner();
                            if (gfuncs::IsFormValid(npc)) {
                                if (npc == akOwner) {
                                    cells.push_back(cell);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
                if (gfuncs::IsFormValid(cell)) {
                    if (cell->IsInteriorCell()) {
                        cells.push_back(cell);
                    }
                }
            }
        }
    }

    //logger::trace("cells size = {}", cells.size());
    return cells;
}

std::vector<RE::TESObjectCELL*> GetAllExteriorCells(RE::StaticFunctionTag*, RE::BGSLocation* akLocation, RE::TESWorldSpace* akWorldSpace, int matchMode) {
    std::vector<RE::TESObjectCELL*> cells;

    if (!gfuncs::IsFormValid(akLocation)) {
        akLocation = nullptr;
    }

    if (!gfuncs::IsFormValid(akWorldSpace)) {
        akWorldSpace = nullptr;
    }

    const auto& [allForms, lock] = RE::TESForm::GetAllForms();

    if (matchMode == 1) {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
                if (gfuncs::IsFormValid(cell)) {
                    if (cell->IsExteriorCell()) {
                        RE::BGSLocation* location = cell->GetLocation();
                        RE::TESWorldSpace* worldSpace = cell->GetRuntimeData().worldSpace;

                        if (!gfuncs::IsFormValid(location)) {
                            location = nullptr;
                        }

                        if (!gfuncs::IsFormValid(worldSpace)) {
                            worldSpace = nullptr;
                        }

                        if (worldSpace == akWorldSpace && location == akLocation) {
                            cells.push_back(cell);
                        }
                    }
                }
            }
        }
    }
    else if (matchMode == 0) {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
                if (gfuncs::IsFormValid(cell)) {
                    if (cell->IsExteriorCell()) {
                        bool matched = false;
                        RE::TESWorldSpace* worldSpace = cell->GetRuntimeData().worldSpace;
                        if (gfuncs::IsFormValid(worldSpace)) {
                            if (worldSpace == akWorldSpace) {
                                matched = true;
                                cells.push_back(cell);
                            }
                        }
                        if (!matched) {
                            RE::BGSLocation* location = cell->GetLocation();
                            if (gfuncs::IsFormValid(location)) {
                                if (location == akLocation) {
                                    cells.push_back(cell);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
                if (gfuncs::IsFormValid(cell)) {
                    if (cell->IsExteriorCell()) {
                        cells.push_back(cell);
                    }
                }
            }
        }
    }

    return cells;
}

std::vector<RE::TESObjectCELL*> GetAttachedCells(RE::StaticFunctionTag*) {
    std::vector<RE::TESObjectCELL*> cells;

    const auto& [allForms, lock] = RE::TESForm::GetAllForms();

    for (auto& [id, form] : *allForms) {
        if (gfuncs::IsFormValid(form)) {
            RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
            if (gfuncs::IsFormValid(cell)) {
                if (cell->IsAttached()) {
                    cells.push_back(cell);
                }
            }
        }
    }
    return cells;
}

std::vector<RE::TESForm*> GetFavorites(RE::StaticFunctionTag*, std::vector<int> formTypes, int formTypeMatchMode) {
    logger::trace("called");

    std::vector<RE::TESForm*> forms;

    auto* playerRef = RE::TESForm::LookupByID<RE::TESForm>(20)->As<RE::Actor>();

    auto inventory = playerRef->GetInventory();

    if (inventory.size() == 0) {
        return forms;
    }

    int ic = 0;

    if (formTypes.size() > 0 && formTypeMatchMode == 1) {
        for (auto it = inventory.begin(); it != inventory.end() && ic < inventory.size(); it++, ic++) {
            auto invData = it->second.second.get();
            if (invData) {
                if (invData->IsFavorited()) {
                    if (gfuncs::IsFormValid(invData->object)) {
                        RE::TESForm* form = invData->object;
                        if (gfuncs::IsFormValid(form)) {
                            int formType = static_cast<int>(form->GetFormType());
                            for (int& i : formTypes) {
                                if (i == formType) {
                                    forms.push_back(form);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (formTypes.size() > 0 && formTypeMatchMode == 0) {
        for (auto it = inventory.begin(); it != inventory.end() && ic < inventory.size(); it++, ic++) {
            auto invData = it->second.second.get();
            if (invData) {
                if (invData->IsFavorited()) {
                    if (gfuncs::IsFormValid(invData->object)) {
                        RE::TESForm* form = invData->object;
                        if (gfuncs::IsFormValid(form)) {
                            int formType = static_cast<int>(form->GetFormType());
                            bool matchedType = false;
                            for (int& i : formTypes) {
                                if (i == formType) {
                                    matchedType = true;
                                    break;
                                }
                            }
                            if (!matchedType) {
                                forms.push_back(form);
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        for (auto it = inventory.begin(); it != inventory.end() && ic < inventory.size(); it++, ic++) {
            auto invData = it->second.second.get();
            if (invData) {
                if (invData->IsFavorited()) {
                    if (gfuncs::IsFormValid(invData->object)) {
                        RE::TESForm* akForm = invData->object;
                        if (gfuncs::IsFormValid(akForm)) {
                            forms.push_back(akForm);
                        }
                    }
                }
            }
        }
    }

    logger::trace("number of favorites = {}", forms.size());

    return forms;
}

std::vector<RE::TESForm*> GetAllFormsWithName(RE::StaticFunctionTag*, std::string sFormName, int nameMatchMode, std::vector<int> formTypes, int formTypeMatchMode) {
    std::vector<RE::TESForm*> forms;
    if (sFormName == "") {
        logger::warn("sFormName is empty");
        return forms;
    }

    if (nameMatchMode == 0) {
        if (formTypes.size() == 0 || formTypeMatchMode == -1) {
            const auto& [allForms, lock] = RE::TESForm::GetAllForms();
            for (auto& [id, form] : *allForms) {
                if (FormNameMatches(form, sFormName)) {
                    forms.push_back(form);
                }
            }
        }
        else if (formTypes.size() > 0 && formTypeMatchMode == 1) {
            const auto& [allForms, lock] = RE::TESForm::GetAllForms();
            for (auto& [id, form] : *allForms) {
                if (FormNameMatches(form, sFormName)) {
                    int formType = static_cast<int>(form->GetFormType());
                    for (int& i : formTypes) {
                        if (i == formType) {
                            forms.push_back(form);
                            break;
                        }
                    }
                }
            }
        }
        else if (formTypes.size() > 0 && formTypeMatchMode == 0) {
            const auto& [allForms, lock] = RE::TESForm::GetAllForms();
            for (auto& [id, form] : *allForms) {
                if (FormNameMatches(form, sFormName)) {
                    int formType = static_cast<int>(form->GetFormType());
                    bool matchedType = false;
                    for (int& i : formTypes) {
                        if (i == formType) {
                            matchedType = true;
                            break;
                        }
                    }
                    if (!matchedType) {
                        forms.push_back(form);
                    }
                }
            }
        }
    }
    else {
        if (formTypes.size() == 0 || formTypeMatchMode == -1) {
            const auto& [allForms, lock] = RE::TESForm::GetAllForms();
            for (auto& [id, form] : *allForms) {
                if (FormNameContains(form, sFormName)) {
                    forms.push_back(form);
                }
            }
        }
        else if (formTypes.size() > 0 && formTypeMatchMode == 1) {
            const auto& [allForms, lock] = RE::TESForm::GetAllForms();
            for (auto& [id, form] : *allForms) {
                if (FormNameContains(form, sFormName)) {
                    int formType = static_cast<int>(form->GetFormType());
                    for (int& i : formTypes) {
                        if (i == formType) {
                            forms.push_back(form);
                            break;
                        }
                    }
                }
            }
        }
        else if (formTypes.size() > 0 && formTypeMatchMode == 0) {
            const auto& [allForms, lock] = RE::TESForm::GetAllForms();
            for (auto& [id, form] : *allForms) {
                if (FormNameContains(form, sFormName)) {
                    int formType = static_cast<int>(form->GetFormType());
                    bool matchedType = false;
                    for (int& i : formTypes) {
                        if (i == formType) {
                            matchedType = true;
                            break;
                        }
                    }
                    if (!matchedType) {
                        forms.push_back(form);
                    }
                }
            }
        }
    }
    gfuncs::RemoveDuplicates(forms);
    return forms;
}

std::vector<RE::TESForm*> GetAllFormsWithScriptAttached(RE::StaticFunctionTag*, RE::BSFixedString sScriptName, std::vector<int> formTypes, int formTypeMatchMode) {
    std::vector<RE::TESForm*> forms;
    if (formTypes.size() == 0 || formTypeMatchMode == -1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsScriptAttachedToForm(form, sScriptName)) {
                forms.push_back(form);
            }
        }
    }
    else if (formTypes.size() > 0 && formTypeMatchMode == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsScriptAttachedToForm(form, sScriptName)) {
                int formType = static_cast<int>(form->GetFormType());
                for (int& i : formTypes) {
                    if (i == formType) {
                        forms.push_back(form);
                    }
                }
            }
        }
    }
    else if (formTypes.size() > 0 && formTypeMatchMode == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsScriptAttachedToForm(form, sScriptName)) {
                int formType = static_cast<int>(form->GetFormType());
                bool matchedType = false;
                for (int& i : formTypes) {
                    if (i == formType) {
                        matchedType = true;
                        break;
                    }
                }
                if (!matchedType) {
                    forms.push_back(form);
                }
            }
        }
    }
    gfuncs::RemoveDuplicates(forms);
    return forms;
}

std::vector<RE::BGSBaseAlias*> GetAllAliasesWithScriptAttached(RE::StaticFunctionTag*, RE::BSFixedString sScriptName) {
    logger::debug("called");

    std::vector<RE::BGSBaseAlias*> questItems;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Quest));
        int ic = 0;

        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
            RE::TESForm* baseForm = *itr;
            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                if (gfuncs::IsFormValid(quest)) {
                    if (quest->aliases.size() > 0) {
                        for (int i = 0; i < quest->aliases.size(); i++) {
                            if (quest->aliases[i]) {
                                RE::VMHandle handle = gfuncs::GetHandle(quest->aliases[i]);
                                if (gfuncs::IsScriptAttachedToHandle(handle, sScriptName)) {
                                    questItems.push_back(quest->aliases[i]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return questItems;
}

std::vector<RE::BGSRefAlias*> GetAllRefAliasesWithScriptAttached(RE::StaticFunctionTag*, RE::BSFixedString sScriptName, bool onlyQuestObjects, bool onlyFilled) {
    logger::debug("called");

    std::vector<RE::BGSRefAlias*> questItems;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Quest));

        int ic = 0;
        if (onlyQuestObjects && onlyFilled) {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    if (quest->aliases[i]->IsQuestObject()) {
                                        RE::VMHandle handle = gfuncs::GetHandle(quest->aliases[i]);
                                        if (gfuncs::IsScriptAttachedToHandle(handle, sScriptName)) {
                                            RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                            if (refAlias) {
                                                RE::TESObjectREFR* akRef = refAlias->GetReference();
                                                if (gfuncs::IsFormValid(akRef)) {
                                                    questItems.push_back(refAlias);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (onlyQuestObjects) {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    if (quest->aliases[i]->IsQuestObject()) {
                                        RE::VMHandle handle = gfuncs::GetHandle(quest->aliases[i]);
                                        if (gfuncs::IsScriptAttachedToHandle(handle, sScriptName)) {
                                            RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                            if (refAlias) {
                                                questItems.push_back(refAlias);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (onlyFilled) {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    RE::VMHandle handle = gfuncs::GetHandle(quest->aliases[i]);
                                    if (gfuncs::IsScriptAttachedToHandle(handle, sScriptName)) {
                                        RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                        if (refAlias) {
                                            RE::TESObjectREFR* akRef = refAlias->GetReference();
                                            if (gfuncs::IsFormValid(akRef)) {
                                                questItems.push_back(refAlias);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    RE::VMHandle handle = gfuncs::GetHandle(quest->aliases[i]);
                                    if (gfuncs::IsScriptAttachedToHandle(handle, sScriptName)) {
                                        RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                        if (refAlias) {
                                            questItems.push_back(refAlias);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return questItems;
}

std::vector<RE::BGSRefAlias*> GetAllRefaliases(RE::StaticFunctionTag*, bool onlyQuestObjects, bool onlyFilled) {
    logger::debug("called");

    std::vector<RE::BGSRefAlias*> questItems;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Quest));
        int ic = 0;

        if (onlyQuestObjects && onlyFilled) {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    if (quest->aliases[i]->IsQuestObject()) {
                                        RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                        if (refAlias) {
                                            RE::TESObjectREFR* akRef = refAlias->GetReference();
                                            if (gfuncs::IsFormValid(akRef)) {
                                                questItems.push_back(refAlias);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (onlyQuestObjects) {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    if (quest->aliases[i]->IsQuestObject()) {
                                        RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                        if (refAlias) {
                                            questItems.push_back(refAlias);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (onlyFilled) {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                    if (refAlias) {
                                        RE::TESObjectREFR* akRef = refAlias->GetReference();
                                        if (gfuncs::IsFormValid(akRef)) {
                                            questItems.push_back(refAlias);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                    if (refAlias) {
                                        questItems.push_back(refAlias);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return questItems;
}

std::vector<RE::BGSRefAlias*> GetAllRefAliasesForRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
    std::vector<RE::BGSRefAlias*> aliases;

    if (!gfuncs::IsFormValid(ref)) {
        return aliases;
    }

    auto aliasArray = ref->extraList.GetByType<RE::ExtraAliasInstanceArray>();
    if (aliasArray) {
        //logger::trace("alias array for ref[{}] found", ref->GetDisplayFullName());
        for (auto* akAlias : aliasArray->aliases) {
            if (akAlias) {
                if (akAlias->alias) {
                    if (gfuncs::IsFormValid(akAlias->alias->owningQuest)) {
                        for (auto* alias : akAlias->alias->owningQuest->aliases) {
                            //akAlias-alias is a const, this is a workaround to get the non const alias without using const_cast
                            if (alias == akAlias->alias) {
                                RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(alias);
                                if (refAlias) {
                                    aliases.push_back(refAlias);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return aliases;
}

std::vector<RE::TESObjectREFR*> GetAllQuestObjectRefs(RE::StaticFunctionTag*) {
    logger::debug("called");

    std::vector<RE::TESObjectREFR*> questItems;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Quest));
        int ic = 0;

        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                if (gfuncs::IsFormValid(quest)) {
                    if (quest->aliases.size() > 0) {
                        for (int i = 0; i < quest->aliases.size(); i++) {
                            if (quest->aliases[i]) {
                                if (quest->aliases[i]->IsQuestObject()) {
                                    //quest->aliases[i].
                                    RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                    if (refAlias) {
                                        RE::TESObjectREFR* akRef = refAlias->GetReference();
                                        if (gfuncs::IsFormValid(akRef)) {
                                            questItems.push_back(akRef);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return questItems;
}

std::vector<RE::TESObjectREFR*> GetQuestObjectRefsInContainer(RE::StaticFunctionTag*, RE::TESObjectREFR* containerRef) {
    logger::trace("called");

    std::vector<RE::TESObjectREFR*> invQuestItems;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    if (!gfuncs::IsFormValid(containerRef)) {
        logger::warn("containerRef doesn't exist");
        return invQuestItems;
    }

    auto inventory = containerRef->GetInventory();

    if (inventory.size() == 0) {
        logger::debug("{} containerRef doesn't contain any items", gfuncs::GetFormName(containerRef));
        return invQuestItems;
    }

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Quest));
        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();

        //logger::debug("number of quests is {}", akArray->size());
        int ic = 0;

        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                if (gfuncs::IsFormValid(quest)) {
                    if (quest->aliases.size() > 0) {
                        for (int i = 0; i < quest->aliases.size(); i++) {
                            if (quest->aliases[i]) {
                                if (quest->aliases[i]->IsQuestObject()) {
                                    RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                    if (refAlias) {
                                        RE::TESObjectREFR* akRef = refAlias->GetReference();
                                        if (gfuncs::ContainerContainsRef(containerRef, akRef)) {
                                            invQuestItems.push_back(akRef);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return invQuestItems;
}

std::vector<RE::TESObjectREFR*> GetAllObjectRefsInContainer(RE::StaticFunctionTag*, RE::TESObjectREFR* containerRef) {
    std::vector<RE::TESObjectREFR*> refs;

    if (!gfuncs::IsFormValid(containerRef)) {
        logger::warn("containerRef doesn't exist");
        return refs;
    }

    const auto& [allForms, lock] = RE::TESForm::GetAllForms();
    for (auto& [id, form] : *allForms) {
        auto* ref = form->AsReference();
        if (gfuncs::ContainerContainsRef(containerRef, ref)) {
            refs.push_back(ref);
        }
    }
    return refs;
}

std::vector<RE::EnchantmentItem*> GetKnownEnchantments(RE::StaticFunctionTag*) {
    std::vector<RE::EnchantmentItem*> returnValues;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {
        logger::error("dataHandler* not found");
        return returnValues;
    }

    RE::BSTArray<RE::TESForm*>* enchantmentArray = &(dataHandler->GetFormArray(RE::FormType::Enchantment));

    logger::debug("enchantmentArray size[{}]", enchantmentArray->size());
    int ic = 0;

    for (RE::BSTArray<RE::TESForm*>::iterator it = enchantmentArray->begin(); it != enchantmentArray->end() && ic < enchantmentArray->size(); it++, ic++) {
        RE::TESForm* baseForm = *it;

        if (gfuncs::IsFormValid(baseForm)) {
            RE::EnchantmentItem* enchantment = baseForm->As<RE::EnchantmentItem>();
            if (gfuncs::IsFormValid(enchantment)) {
                RE::EnchantmentItem* baseEnchantment = enchantment->data.baseEnchantment;
                if (gfuncs::IsFormValid(baseEnchantment)) {
                    if (baseEnchantment->GetKnown()) {
                        if (std::find(returnValues.begin(), returnValues.end(), baseEnchantment) == returnValues.end()) {
                            // baseEnchantment not in returnValues, add it
                            returnValues.push_back(baseEnchantment);
                        }
                    }
                }
            }
        }
    }
    return returnValues;
}

void AddKnownEnchantmentsToFormList(RE::StaticFunctionTag*, RE::BGSListForm* akList) {
    if (!gfuncs::IsFormValid(akList)) {
        logger::warn("akList doesn't exist");
        return;
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {
        logger::error("dataHandler* not found");
        return;
    }
    RE::BSTArray<RE::TESForm*>* enchantmentArray = &(dataHandler->GetFormArray(RE::FormType::Enchantment));

    logger::debug("enchantmentArray size[{}]", enchantmentArray->size());
    int ic = 0;

    for (RE::BSTArray<RE::TESForm*>::iterator it = enchantmentArray->begin(); it != enchantmentArray->end() && ic < enchantmentArray->size(); it++, ic++) {
        RE::TESForm* baseForm = *it;

        if (gfuncs::IsFormValid(baseForm)) {
            RE::EnchantmentItem* enchantment = baseForm->As<RE::EnchantmentItem>();
            if (gfuncs::IsFormValid(enchantment)) {
                RE::EnchantmentItem* baseEnchantment = enchantment->data.baseEnchantment;
                if (gfuncs::IsFormValid(baseEnchantment)) {
                    if (baseEnchantment->GetKnown()) {
                        //if (!akList->HasForm(baseEnchantment)) {
                        akList->AddForm(baseEnchantment);
                        //}
                    }
                }
            }
        }
    }
}