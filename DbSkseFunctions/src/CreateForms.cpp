#include "CreateForms.h"
#include "GeneralFunctions.h"

RE::BGSKeyword* CreateKeyword(RE::StaticFunctionTag*) {
    logger::trace("called");

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::warn("failed");
		return nullptr;
    }
    else {
		if (!newForm->GetFormEditorID()) {
			newForm->SetFormEditorID(std::format("DbSkseKeyword_{:08X}", newForm->GetFormID()).c_str());
			logger::debug("assigned editorID[{}]", newForm->GetFormEditorID());
		}
		
		const char* edid = newForm->GetFormEditorID();
		logger::trace("success: ptr[{}] formID[{:08X}] formType[{}] dynamic[{}] editorID[{}]",
			static_cast<const void*>(newForm),
			newForm->GetFormID(),
			static_cast<int>(newForm->GetFormType()),   // 4 == Keyword
			newForm->IsDynamicForm(),
			edid ? edid : "<NULL>"
		);
    }
	
    return newForm;
}

RE::BGSListForm* CreateFormList(RE::StaticFunctionTag*, RE::BGSListForm* formListFiller) {
    logger::trace("called");

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSListForm>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("failed");
		return nullptr;
    }
    else {
        if (!newForm->GetFormEditorID()) {
			newForm->SetFormEditorID(std::format("BGSListForm_{:08X}", newForm->GetFormID()).c_str());
			logger::debug("assigned editorID[{}]", newForm->GetFormEditorID());
		}
		
		const char* edid = newForm->GetFormEditorID();
		logger::trace("success: ptr[{}] formID[{:08X}] formType[{}] dynamic[{}] editorID[{}]",
			static_cast<const void*>(newForm),
			newForm->GetFormID(),
			static_cast<int>(newForm->GetFormType()),   // 4 == Keyword
			newForm->IsDynamicForm(),
			edid ? edid : "<NULL>"
		);
		
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
		return nullptr;
    }
    else {
        if (!newForm->GetFormEditorID()) {
			newForm->SetFormEditorID(std::format("BGSColorForm_{:08X}", newForm->GetFormID()).c_str());
			logger::debug("assigned editorID[{}]", newForm->GetFormEditorID());
		}
		
		const char* edid = newForm->GetFormEditorID();
		logger::trace("success: ptr[{}] formID[{:08X}] formType[{}] dynamic[{}] editorID[{}]",
			static_cast<const void*>(newForm),
			newForm->GetFormID(),
			static_cast<int>(newForm->GetFormType()),   // 4 == Keyword
			newForm->IsDynamicForm(),
			edid ? edid : "<NULL>"
		);
		
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
		return nullptr;
    }
    else {
        if (!newForm->GetFormEditorID()) {
			newForm->SetFormEditorID(std::format("BGSConstructibleObject_{:08X}", newForm->GetFormID()).c_str());
			logger::debug("assigned editorID[{}]", newForm->GetFormEditorID());
		}
		
		const char* edid = newForm->GetFormEditorID();
		logger::trace("success: ptr[{}] formID[{:08X}] formType[{}] dynamic[{}] editorID[{}]",
			static_cast<const void*>(newForm),
			newForm->GetFormID(),
			static_cast<int>(newForm->GetFormType()),   // 4 == Keyword
			newForm->IsDynamicForm(),
			edid ? edid : "<NULL>"
		);
    }

    return newForm;
}

RE::BGSTextureSet* CreateTextureSet(RE::StaticFunctionTag*) {
    logger::trace("called");

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSTextureSet>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("failed");
		return nullptr;
    }
    else {
        if (!newForm->GetFormEditorID()) {
			newForm->SetFormEditorID(std::format("BGSTextureSet_{:08X}", newForm->GetFormID()).c_str());
			logger::debug("assigned editorID[{}]", newForm->GetFormEditorID());
		}
		
		const char* edid = newForm->GetFormEditorID();
		logger::trace("success: ptr[{}] formID[{:08X}] formType[{}] dynamic[{}] editorID[{}]",
			static_cast<const void*>(newForm),
			newForm->GetFormID(),
			static_cast<int>(newForm->GetFormType()),   // 4 == Keyword
			newForm->IsDynamicForm(),
			edid ? edid : "<NULL>"
		);
    }

    return newForm;
}

RE::TESSound* CreateSoundMarker(RE::StaticFunctionTag*) {
    logger::trace("called");

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESSound>()->Create();
    if (!gfuncs::IsFormValid(newForm)) {
        logger::error("failed");
		return nullptr;
    }
    else {
        if (!newForm->GetFormEditorID()) {
			newForm->SetFormEditorID(std::format("TESSound_{:08X}", newForm->GetFormID()).c_str());
			logger::debug("assigned editorID[{}]", newForm->GetFormEditorID());
		}
		
		const char* edid = newForm->GetFormEditorID();
		logger::trace("success: ptr[{}] formID[{:08X}] formType[{}] dynamic[{}] editorID[{}]",
			static_cast<const void*>(newForm),
			newForm->GetFormID(),
			static_cast<int>(newForm->GetFormType()),   // 4 == Keyword
			newForm->IsDynamicForm(),
			edid ? edid : "<NULL>"
		);
    }

    return newForm;
}