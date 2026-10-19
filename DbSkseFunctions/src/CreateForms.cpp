#include "CreateForms.h"
#include "GeneralFunctions.h"
#include "Serialization.h"
#include "RE/B/BGSBaseAlias.h"
#include "RE/B/BSCoreTypes.h"
#include "RE/T/TESForm.h"
#include "SKSE/Logger.h"
#include <concepts>
#include <cstdint>
#include <mutex>
#include <unordered_set>

namespace CreateForms {
	std::unordered_set<RE::FormID> createdForms;
	std::unordered_set<RE::FormID> formsToDelete;
	std::mutex mutex;
	
	RE::BGSKeyword* CreateKeyword(RE::StaticFunctionTag*) {
		std::lock_guard<std::mutex> lock(mutex);
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
		
		createdForms.insert(newForm->GetFormID());
		return newForm;
	}

	RE::BGSListForm* CreateFormList(RE::StaticFunctionTag*, RE::BGSListForm* formListFiller) {
		std::lock_guard<std::mutex> lock(mutex);
		logger::trace("called");

		auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSListForm>()->Create();
		if (!gfuncs::IsFormValid(newForm)) {
			logger::error("failed");
			return nullptr;
		}
		else {
			if (!newForm->GetFormEditorID()) {
				newForm->SetFormEditorID(std::format("DbSkseListForm_{:08X}", newForm->GetFormID()).c_str());
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
		
		createdForms.insert(newForm->GetFormID());
		return newForm;
	}

	RE::BGSColorForm* CreateColorForm(RE::StaticFunctionTag*, int color) {
		std::lock_guard<std::mutex> lock(mutex);
		logger::trace("called");

		auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSColorForm>()->Create();
		if (!gfuncs::IsFormValid(newForm)) {
			logger::error("failed");
			return nullptr;
		}
		else {
			if (!newForm->GetFormEditorID()) {
				newForm->SetFormEditorID(std::format("DbSkseColorForm_{:08X}", newForm->GetFormID()).c_str());
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

		createdForms.insert(newForm->GetFormID());
		return newForm;
	}

	RE::BGSConstructibleObject* CreateConstructibleObject(RE::StaticFunctionTag*) {
		std::lock_guard<std::mutex> lock(mutex);
		logger::trace("called");

		//RE::BGSConstructibleObject
		auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSConstructibleObject>()->Create();
		if (!gfuncs::IsFormValid(newForm)) {
			logger::error("failed");
			return nullptr;
		}
		else {
			if (!newForm->GetFormEditorID()) {
				newForm->SetFormEditorID(std::format("DbSkseConstructibleObject_{:08X}", newForm->GetFormID()).c_str());
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

		createdForms.insert(newForm->GetFormID());
		return newForm;
	}

	RE::BGSTextureSet* CreateTextureSet(RE::StaticFunctionTag*) {
		std::lock_guard<std::mutex> lock(mutex);
		logger::trace("called");

		auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSTextureSet>()->Create();
		if (!gfuncs::IsFormValid(newForm)) {
			logger::error("failed");
			return nullptr;
		}
		else {
			if (!newForm->GetFormEditorID()) {
				newForm->SetFormEditorID(std::format("DbSkseTextureSet_{:08X}", newForm->GetFormID()).c_str());
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

		createdForms.insert(newForm->GetFormID());
		return newForm;
	}

	RE::TESSound* CreateSoundMarker(RE::StaticFunctionTag*) {
		std::lock_guard<std::mutex> lock(mutex);
		logger::trace("called");

		auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESSound>()->Create();
		if (!gfuncs::IsFormValid(newForm)) {
			logger::error("failed");
			return nullptr;
		}
		else {
			if (!newForm->GetFormEditorID()) {
				newForm->SetFormEditorID(std::format("DbSkseSound_{:08X}", newForm->GetFormID()).c_str());
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

		createdForms.insert(newForm->GetFormID());
		return newForm;
	}

	RE::TESQuest* CreateQuest(RE::StaticFunctionTag*) {
		std::lock_guard<std::mutex> lock(mutex);
		logger::trace("called");

		auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESQuest>()->Create();
		if (!gfuncs::IsFormValid(newForm)) {
			logger::error("failed");
			return nullptr;
		}
		else {
			if (!newForm->GetFormEditorID()) {
				newForm->SetFormEditorID(std::format("DbSkseQuest_{:08X}", newForm->GetFormID()).c_str());
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
		
		createdForms.insert(newForm->GetFormID());
		return newForm;
	}

	// A type the game can hand us a concrete factory for.
	// Excludes rows 200-204 of DB_MASTER_TYPE_LIST: the aliases, ActiveEffect and
	// TESForm itself are not TESForm-derived types with a FORMTYPE.
	template <class T>
	concept FactoryCreatableForm = std::derived_from<T, RE::TESForm> && requires { T::FORMTYPE; };

	template <class T>
	RE::TESForm* CreateFormOfType(const std::string& baseScriptType, const std::string& scriptName) {
		std::lock_guard<std::mutex> lock(mutex);
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

			newForm->SetFormEditorID(std::format("DbSkse{}_{:08X}", baseScriptType, newForm->GetFormID()).c_str());

			if (!scriptName.empty()){
				RE::BSTSmartPointer<RE::BSScript::Object> object = gfuncs::AttachPapyrusScriptToForm(newForm, scriptName.c_str());

				if (!object.get()) {
					logger::error("couldn't attach script[{}] to new form[{:08X}]", scriptName, newForm->GetFormID());
					// newForm->SetDelete(true);
					// return nullptr;
				}
			}

			createdForms.insert(newForm->GetFormID());
			return newForm;
		}
	}

	void NotifyVMOfFormDeletion(RE::TESForm* akForm){
		if (!akForm){
			return;
		}

		auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (!vm){
			return;
		}

		auto* policy = vm->GetObjectHandlePolicy();
		if (!policy){
			return;
		}

		RE::VMHandle handle = gfuncs::GetHandle(akForm);

		if (handle == policy->EmptyHandle()){
			return;
		}

		//unbinds every script on this handle and is what drives SKSE's form delete callback
		vm->HandleCObjectDeletion(handle);
	}
	
	// return -1 for error, not deleted 
	// return 0 for marked for delete 
	// return 1 for actually deleted
	int DeleteFormCpp(RE::TESForm* akForm, bool eraseFromFormsToDelete = true){
		std::lock_guard<std::mutex> lock(mutex);
		
		if (!gfuncs::IsFormValid(akForm)){
			return -1;
		}
		
		auto formId = akForm->GetFormID();
		
		auto it = createdForms.find(akForm->GetFormID());
		if (it == createdForms.end()){
			logger::warn("akForm {} was not created by DbSkseFunctions.dll, aborting delete", gfuncs::GetFormNameAndId(akForm));
			return -1;
		}
		
		if (!gfuncs::IsFormInUse(akForm)){
			akForm->SetDelete(true);
			NotifyVMOfFormDeletion(akForm);
			createdForms.erase(it);
			
			if (eraseFromFormsToDelete){
				auto itt = formsToDelete.find(formId);
				if (itt != formsToDelete.end()){
					formsToDelete.erase(itt);
				}
			}
			
			delete akForm;
			return 1;
		}
		
		formsToDelete.insert(akForm->GetFormID());
		return 0;
	}
	
	// return -1 for error, not deleted 
	// return 0 for marked for delete 
	// return 1 for actually deleted
	int DeleteForm(RE::StaticFunctionTag*, RE::TESForm* akForm){
		return DeleteFormCpp(akForm);
	}
	
	void DeleteForms(){
		std::unordered_set<RE::FormID> formsDeleted;
		for (RE::FormID formId : formsToDelete){
			if (DeleteFormCpp(RE::TESForm::LookupByID(formId), false) == 1){ //actually deleted
				formsDeleted.insert(formId);
			}
		}
		
		std::lock_guard<std::mutex> lock(mutex);
		for (RE::FormID formId : formsDeleted){ //erase from formsToDelete after iterating through the first time.
			auto it = formsToDelete.find(formId);
			if (it != formsToDelete.end()){
				formsToDelete.erase(it);
			}
		}
	}
	
	namespace Serialization {
		enum Version : std::uint32_t {
			kVersion1    = 1,   //first released version, conditions + linked forms + events
			kCurrent = kVersion1,
		};
			
		bool SaveForms(SKSE::SerializationInterface* ssi){
			std::lock_guard<std::mutex> lock(mutex);
			
			if (!ssi->OpenRecord(createFormsRecord, Version::kCurrent)){
				return false;
			}
			
			if (!serialize::SaveFormIDUnorderedSet(createdForms, createFormsRecord, ssi, false)){
				return false;
			}
			
			if (!serialize::SaveFormIDUnorderedSet(formsToDelete, createFormsRecord, ssi, false)){
				return false;
			}
			
			return true;
		}
		
		bool LoadForms(SKSE::SerializationInterface* ssi, std::uint32_t version){
			std::lock_guard<std::mutex> lock(mutex);
			
			if (!serialize::LoadFormIdUnorderedSet(createdForms, createFormsRecord, ssi)){
				return false;
			}
			
			if (!serialize::LoadFormIdUnorderedSet(formsToDelete, createFormsRecord, ssi)){
				return false;
			}
			
			return true;
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
	
	bool GetFormPlayable(RE::TESForm* akForm){
		if (!gfuncs::IsFormValid(akForm)){
			return false;
		}

		//bit SET means non-playable, so playable is the bit being clear
		return (akForm->formFlags & RE::TESForm::RecordFlags::kNonPlayable) == 0;
	}

	bool SetFormPlayable(RE::StaticFunctionTag*, RE::TESForm* akForm, bool playable){
		if (!gfuncs::IsFormValid(akForm)){
			return false;
		}

		constexpr std::uint32_t nonPlayable = RE::TESForm::RecordFlags::kNonPlayable;  // 1 << 2

		const bool wasPlayable = (akForm->formFlags & nonPlayable) == 0;
		if (wasPlayable == playable){
			return true;  //already in the requested state
		}

		if (playable){
			akForm->formFlags &= ~nonPlayable;   //clear the bit
		}
		else {
			akForm->formFlags |= nonPlayable;    //set the bit
		}

		//SetDelete does AddChange(1) after touching the flags -- ChangeFlags::kFlags is that 1.
		//Without it the game doesn't know the form's flags changed.
		akForm->AddChange(RE::TESForm::ChangeFlags::kFlags);

		return ((akForm->formFlags & nonPlayable) == 0) == playable;
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
		vm->RegisterFunction("SetFormPlayable", "DbSkseFunctions", SetFormPlayable);
		 
		return true;
	}
}
