#include "CreateForms.h"
#include "GeneralFunctions.h"
#include "RE/B/BGSBaseAlias.h"
#include "RE/B/BSCoreTypes.h"
#include "RE/T/TESForm.h"
#include "SKSE/Logger.h"
#include <concepts>
#include <cstdint>
#include <unordered_set>

namespace CreateForms {
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

	RE::TESQuest* CreateQuest(RE::StaticFunctionTag*) {
		logger::trace("called");

		auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESQuest>()->Create();
		if (!gfuncs::IsFormValid(newForm)) {
			logger::error("failed");
			return nullptr;
		}
		else {
			if (!newForm->GetFormEditorID()) {
				newForm->SetFormEditorID(std::format("TESQuest_{:08X}", newForm->GetFormID()).c_str());
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

// ========================================================================================================================================
// used for testing ResolveFormID on created forms. 
// after testing, ResolveFormID was successful, but always the same as the original formId
// even after changing load order of the mod .esp where the new forms were created / stored in a papyrus script.
	std::unordered_set<RE::FormID> savedFormIds;

	bool SaveForms(SKSE::SerializationInterface* ssi){
		logger::debug("savedFormIds size[{}]", savedFormIds.size());
		
		if (!ssi->OpenRecord(createFormsRecord, 1)){
			logger::warn("failed to open record STf9");
			return false;
		}
		
		std::size_t size = savedFormIds.size();
		if (!ssi->WriteRecordData(size)){
			logger::warn("failed to write savedFormIds size");
			return false;
		}
		
		int i = 0;
		for (RE::FormID formId : savedFormIds){
			if (!ssi->WriteRecordData(formId)){
				logger::warn("i[{}]: failed to write formId[{:08X}]", i, formId);
				return false;
			}
			i++;
		}
		
		return true;
	}

	bool LoadForms(SKSE::SerializationInterface* ssi){
		logger::debug("savedFormIds. current size[{}]", savedFormIds.size());
		
		std::size_t size = 0;
		if (!ssi->ReadRecordData(size)){
			logger::warn("failed to read record data for size");
			return false;
		}
		
		logger::debug("savedFormIds. loaded size[{}]", size);
		
		for (std::size_t i = 0; i < size; i++){
			RE::FormID formId = 0; 
			if (!ssi->ReadRecordData(formId)){
				logger::warn("failed to read record data for formId");
				return false;
			}
			
			RE::FormID resolvedId = 0;
			if (!ssi->ResolveFormID(formId, resolvedId)){
				logger::warn("failed to resolve formId[{:08X}]", formId);
				return false;
			}
			// else {
			// 	logger::debug("formId[{:08X}] resolvedId[{:08X}]", formId, resolvedId);
			// }
			
			RE::TESForm* akForm = RE::TESForm::LookupByID(resolvedId);
			if (!gfuncs::IsFormValid(akForm, false, false)){
				logger::warn("saved form, oldId[{:08X}] resolvedId[{:08X}] no longer exists", formId, resolvedId);
				return false;
			}
			
			if (resolvedId != formId){
				logger::debug("resolvedId[{:08X}] does not match savedId[{:08X}], updating list.", resolvedId, formId);
				auto it = savedFormIds.find(formId);
				if (it != savedFormIds.end()){
					savedFormIds.erase(it);
				}
				
				savedFormIds.insert(resolvedId);
			}
		}
		
		return true;
	}
// ========================================================================================================================================

	// A type the game can hand us a concrete factory for.
	// Excludes rows 200-204 of DB_MASTER_TYPE_LIST: the aliases, ActiveEffect and
	// TESForm itself are not TESForm-derived types with a FORMTYPE.
	template <class T>
	concept FactoryCreatableForm = std::derived_from<T, RE::TESForm> && requires { T::FORMTYPE; };

	template <class T>
	RE::TESForm* CreateFormOfType(const std::string& baseScriptType, const std::string& scriptName) {
		//typeid(T).name()
		if constexpr (!FactoryCreatableForm<T>) {
			logger::warn("[{}] is not a creatable form type", baseScriptType);
			return nullptr;
		}
		else if constexpr (std::is_same_v<T, RE::TESObjectREFR> || std::is_same_v<T, RE::Actor> ||
						std::is_same_v<T, RE::TESObjectCELL>  || std::is_same_v<T, RE::TESWorldSpace>) {
			// these need a base object / parent cell / NPC template -- a bare one is broken
			logger::warn("can't create new form of type [{}]", baseScriptType);
			return nullptr;
		}
		
		else {
			auto* factory = RE::IFormFactory::GetConcreteFormFactoryByType<T>();
			if (!factory) {
				logger::warn("no form factory registered for formType[{}]", static_cast<int>(T::FORMTYPE));
				return nullptr;
			}

			auto* newForm = factory->Create();
			if (!newForm) {
				logger::error("factory failed to create formType[{}]", static_cast<int>(T::FORMTYPE));
				return nullptr;
			}

			newForm->SetFormEditorID(std::format("{:08X}", newForm->GetFormID()).c_str());

			if (!scriptName.empty()){
				RE::BSTSmartPointer<RE::BSScript::Object> object = gfuncs::AttachPapyrusScriptToForm(newForm, scriptName.c_str());

				if (!object.get()) {
					logger::error("couldn't attach script[{}] to new form[{:08X}]", scriptName, newForm->GetFormID());
					// newForm->SetDelete(true);
					// return nullptr;
				}
			}

			savedFormIds.insert(newForm->GetFormID());
			return newForm;
		}
	}

	RE::TESForm* CreateForm(RE::StaticFunctionTag*, std::string baseScriptType, std::string scriptName) {
		logger::debug("baseScriptType[{}] scriptName[{}]", baseScriptType, scriptName);
		
		if (baseScriptType.empty()){
			return nullptr;
		}
		
	//uses FormTypeTable, can be turned off while developing as it's slow to compile.
	#ifdef DB_FULL_ARRAY_CONVERSIONS
		
		auto result = gfuncs::GetFormTypeFromPapyrusPropertyName(baseScriptType);
		if (!result) {
			logger::error("baseScriptType[{}] is not a known papyrus form type", baseScriptType);
			return nullptr;
		}

		FormTypeTable::ExtendedFormType type = result.value();
		
		switch (type) {
	#define X(ENUM, CLASS, PSC)                                \
			case FormTypeTable::ExtendedFormType::PSC:         \
				return CreateFormOfType<CLASS>(baseScriptType, scriptName);

			DB_MASTER_TYPE_LIST
	#undef X

			default:
				logger::error("form type[{}] for script[{}] scriptName[{}] from FormTypeTable not recognized", static_cast<int>(type), baseScriptType, scriptName);
				return nullptr;
		}
		
	#else
			logger::error("FormTypeTable conversions in this build (aka DB_FULL_ARRAY_CONVERSIONS = off) -- [{}] not created", baseScriptType);
			return nullptr;
	#endif
	}
	
	//get the 
	int GetFormTypeForScript(RE::StaticFunctionTag*, std::string baseScriptType) {
		logger::debug("baseScriptType[{}]", baseScriptType);
		
		if (baseScriptType.empty()){
			return -1;
		}
		
	//uses FormTypeTable, can be turned off while developing as it's slow to compile.
	#ifdef DB_FULL_ARRAY_CONVERSIONS
		
		auto result = gfuncs::GetFormTypeFromPapyrusPropertyName(baseScriptType);
		if (!result) {
			logger::error("baseScriptType[{}] is not a known papyrus form type", baseScriptType);
			return -1;
		}

		FormTypeTable::ExtendedFormType type = result.value();
		return static_cast<int>(type);
		
	#else
			logger::error("FormTypeTable conversions in this build (aka DB_FULL_ARRAY_CONVERSIONS = off) -- [{}] not created", baseScriptType);
			return -1;
	#endif
	}
	
	//get the base scriptname associated with the formType
	std::string GetScriptForFormType(RE::StaticFunctionTag*, int formType) {
		logger::debug("formType[{}]", formType);

		if (formType < 0) {
			return "";
		}

	//uses FormTypeTable, can be turned off while developing as it's slow to compile.
	#ifdef DB_FULL_ARRAY_CONVERSIONS
		const auto type = static_cast<FormTypeTable::ExtendedFormType>(formType);

		switch (type) {
		// the enumerator IS the script name, so the case label and the returned string both
		// come from ScriptName -- # stringifies it, since the table stores a bare token
	#define X(Id, Class, ScriptName) case FormTypeTable::ExtendedFormType::ScriptName: return #ScriptName;

			DB_MASTER_TYPE_LIST

	#undef X

			default:
				logger::error("formType[{}] from FormTypeTable not recognized", formType);
				return "";
		}
	#else
		logger::error("FormTypeTable conversions are off in this build "
			"(DB_FULL_ARRAY_CONVERSIONS not defined) -- can't resolve formType[{}]", formType);
		return "";
	#endif
	}
	
	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("CreateKeyword", "DbSkseFunctions", CreateKeyword);
		vm->RegisterFunction("CreateFormList", "DbSkseFunctions", CreateFormList);
		vm->RegisterFunction("CreateColorForm", "DbSkseFunctions", CreateColorForm);
		vm->RegisterFunction("CreateConstructibleObject", "DbSkseFunctions", CreateConstructibleObject);
		vm->RegisterFunction("CreateTextureSet", "DbSkseFunctions", CreateTextureSet);
		vm->RegisterFunction("CreateSoundMarker", "DbSkseFunctions", CreateSoundMarker);
		vm->RegisterFunction("CreateQuest", "DbSkseFunctions", CreateQuest);
		vm->RegisterFunction("CreateForm", "DbSkseFunctions", CreateForm);
		vm->RegisterFunction("GetFormTypeForScript", "DbSkseFunctions", GetFormTypeForScript);
		vm->RegisterFunction("GetScriptForFormType", "DbSkseFunctions", GetScriptForFormType);
		 
		return true;
	}
}
