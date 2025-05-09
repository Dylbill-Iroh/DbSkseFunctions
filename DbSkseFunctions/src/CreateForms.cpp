#include "CreateForms.h"
#include "GeneralFunctions.h"

RE::BGSKeyword* CreateKeyword(RE::StaticFunctionTag*) {
    logger::trace("called");

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::warn("failed");
    }
    else {
        logger::debug("success, dynamic form[{}]", newForm->IsDynamicForm());
    }

    return newForm;
}

RE::BGSListForm* CreateFormList(RE::StaticFunctionTag*, RE::BGSListForm* formListFiller) {
    logger::trace("called");

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSListForm>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("failed");
    }
    else {
        logger::debug("success");
        if (gfuncs::IsFormValid(formListFiller)) {
            logger::debug("IsDynamicForm[{}]", formListFiller->IsDynamicForm());

            formListFiller->ForEachForm([&](RE::TESForm* akForm) {
                newForm->AddForm(akForm);
                return RE::BSContainer::ForEachResult::kContinue;
                });
        }
    }
    return newForm;
}

RE::BGSColorForm* CreateColorForm(RE::StaticFunctionTag*, int color) {
    logger::trace("called");

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSColorForm>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("failed");
    }
    else {
        logger::debug("success");
        newForm->color = color;
    }

    return newForm;
}

RE::BGSConstructibleObject* CreateConstructibleObject(RE::StaticFunctionTag*) {
    logger::trace("called");

    //RE::BGSConstructibleObject
    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSConstructibleObject>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("failed");
    }
    else {
        logger::debug("success");
    }

    return newForm;
}

RE::BGSTextureSet* CreateTextureSet(RE::StaticFunctionTag*) {
    logger::trace("called");

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSTextureSet>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("failed");
    }
    else {
        logger::debug("success");
    }

    return newForm;
}

RE::TESSound* CreateSoundMarker(RE::StaticFunctionTag*) {
    logger::trace("called");

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESSound>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("failed");
    }
    else {
        logger::debug("success");
    }

    return newForm;
}