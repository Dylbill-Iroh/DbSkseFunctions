#include "CreateForms.h"
#include "GeneralFunctions.h"

namespace logger = SKSE::log;

RE::BGSKeyword* CreateKeyword(RE::StaticFunctionTag*) {
    logger::trace("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::warn("{} failed", __func__);
    }
    else {
        logger::debug("{} success, dynamic form[{}]", __func__, newForm->IsDynamicForm());
    }

    return newForm;
}

RE::BGSListForm* CreateFormList(RE::StaticFunctionTag*, RE::BGSListForm* formListFiller) {
    logger::trace("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSListForm>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::debug("{} success", __func__);
        if (gfuncs::IsFormValid(formListFiller)) {
            logger::debug("{} IsDynamicForm[{}]", __func__, formListFiller->IsDynamicForm());

            formListFiller->ForEachForm([&](auto& akForm) {
                auto* form = &akForm;
                newForm->AddForm(form);
                return RE::BSContainer::ForEachResult::kContinue;
                });
        }
    }
    return newForm;
}

RE::BGSColorForm* CreateColorForm(RE::StaticFunctionTag*, int color) {
    logger::trace("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSColorForm>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::debug("{} success", __func__);
        newForm->color = color;
    }

    return newForm;
}

RE::BGSConstructibleObject* CreateConstructibleObject(RE::StaticFunctionTag*) {
    logger::trace("{} called", __func__);

    //RE::BGSConstructibleObject
    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSConstructibleObject>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::debug("{} success", __func__);
    }

    return newForm;
}

RE::BGSTextureSet* CreateTextureSet(RE::StaticFunctionTag*) {
    logger::trace("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSTextureSet>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::debug("{} success", __func__);
    }

    return newForm;
}

RE::TESSound* CreateSoundMarker(RE::StaticFunctionTag*) {
    logger::trace("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESSound>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::debug("{} success", __func__);
    }

    return newForm;
}