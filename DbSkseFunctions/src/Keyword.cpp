#include "Keyboard.h"
#include "GeneralFunctions.h"
#include "RE/B/BGSKeywordForm.h"
#include "RE/B/BGSListForm.h"
#include "RE/RTTI.h"
#include "SKSE/Logger.h"
#include "SharedVariables.h"
#include "RE/B/BGSKeyword.h"
#include <cstddef>

namespace keyword {
    std::string GetKeywordString(RE::StaticFunctionTag*, RE::BGSKeyword* akKeyword) {
        logger::trace("");
        if (!gfuncs::IsFormValid(akKeyword)) {
            logger::warn("akKeyword doesn't exist");
            return "";
        }
        return std::string(akKeyword->GetFormEditorID());
    }

    void SetKeywordString(RE::StaticFunctionTag*, RE::BGSKeyword* akKeyword, std::string keywordString) {
        logger::trace("{}", keywordString);

        //if (!savedFormIDs) { savedFormIDs = new SavedFormIDs(); }

        if (!gfuncs::IsFormValid(akKeyword)) {
            logger::warn("akKeyword doesn't exist");
            return;
        }
        akKeyword->SetFormEditorID(keywordString.c_str());
    } 

	std::vector<RE::BGSKeyword*> GetKeywordsForString(RE::StaticFunctionTag*, std::string keywordString){
		std::vector<RE::BGSKeyword*> v; 
		
        if (sv::dataHandler) {
			RE::BSTArray<RE::TESForm*>* akArray = &(sv::dataHandler->GetFormArray(RE::FormType::Keyword));

			int ic = 0;
			for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != akArray->end() && ic < akArray->size(); itr++, ic++) {
				RE::TESForm* baseForm = *itr;
				if (gfuncs::IsFormValid(baseForm)) {
					RE::BGSKeyword* keyword = baseForm->As<RE::BGSKeyword>();
                	if (gfuncs::IsFormValid(keyword)) {
						if (keyword->GetFormEditorID() == keywordString){
							v.push_back((keyword));
						}
					}
				}
			}
		}
		return v; 
	}
	
	std::vector<RE::BGSKeyword*> GetKeywordsOnForm(RE::TESForm* form) {
		std::vector<RE::BGSKeyword*> v; 
		
		if (gfuncs::IsFormValid(form)){ 
		// if (form){
			// logger::debug("form[{}]", gfuncs::GetFormNameAndId(form));
			// RE::BGSKeywordForm* keywordForm = form->As<RE::BGSKeywordForm>();
			RE::BGSKeywordForm* keywordForm = skyrim_cast<RE::BGSKeywordForm*>(form);
			if (keywordForm){
				// logger::debug("keywordForm found. numKeywords[{}]", keywordForm->numKeywords);
				for (std::uint32_t i = 0; i < keywordForm->numKeywords; i++){
					RE::BGSKeyword* keyword = keywordForm->keywords[i];
					if (gfuncs::IsFormValid(keyword, false, false)){
						v.push_back(keyword);
					}
				}
			}
		}
		// else {
		// 	logger::debug("form not valid");
		// }
		return v;
	}
	
	std::vector<RE::BGSKeyword*> GetKeywordsOnArrayForms(RE::StaticFunctionTag*, std::vector<RE::TESForm*> forms){
		std::vector<RE::BGSKeyword*> v; 
		
		auto size = forms.size();
		logger::trace("called, arr size is [{}]", size);
		if (size == 0){
			return v;
		} 
		
		for (size_t i = 0; i < size; i++){
			std::vector<RE::BGSKeyword*> keywords = GetKeywordsOnForm(forms[i]); 
			if (keywords.size() > 0){
				gfuncs::PushbackVector(v, keywords);
			}
		}
		
		gfuncs::RemoveDuplicates(v);
		
		return v;
	}
	
	//does not get nested list forms
	std::vector<RE::BGSKeyword*> GetKeywordsOnListFormsNotNested(RE::BGSListForm* akFormlist){
		std::vector<RE::BGSKeyword*> v; 
		
		if (!gfuncs::IsFormValid(akFormlist)){
			return v;
		}
		
		akFormlist->ForEachForm([&](RE::TESForm* form) {
			std::vector<RE::BGSKeyword*> keywords = GetKeywordsOnForm(form); 
			if (keywords.size() > 0){
				gfuncs::PushbackVector(v, keywords);
			}
			return RE::BSContainer::ForEachResult::kContinue;
		});
		
		return v;
	}
	
	//get forms in nested lists
	void GetKeywordsOnListFormsNested(RE::BGSListForm* akFormlist, std::vector<RE::BGSListForm*>& checkedLists, std::vector<RE::BGSKeyword*>& v) {
		if (!gfuncs::IsFormValid(akFormlist)) { 
			return; 
		}
		
		if (gfuncs::VectorContains(checkedLists, akFormlist)) { 
			return; 
		}   // guard here too
		
		checkedLists.push_back(akFormlist);

		std::vector<RE::BGSListForm*> nested;

		akFormlist->ForEachForm([&](RE::TESForm* form) {
			if (gfuncs::IsFormValid(form, false, false)) {
				auto keywords = GetKeywordsOnForm(form);
				if (!keywords.empty()) { 
					gfuncs::PushbackVector(v, keywords); 
				}

				if (auto* list = form->As<RE::BGSListForm>()) {
					nested.push_back(list);
				}
			}
			return RE::BSContainer::ForEachResult::kContinue;
		});

		for (auto* list : nested) { // lock released before recurse
			GetKeywordsOnListFormsNested(list, checkedLists, v);
		}
	}
	
	std::vector<RE::BGSKeyword*> GetKeywordsOnListForms(RE::StaticFunctionTag*, RE::BGSListForm* akFormlist, bool getNested){
		std::vector<RE::BGSKeyword*> v; 
		if (!gfuncs::IsFormValid(akFormlist)) {
			logger::warn("akFormlist doesn't exist");
			return v;
		}
		
		if (!getNested){
			v = GetKeywordsOnListFormsNotNested(akFormlist);
		}
		else {
			std::vector<RE::BGSListForm*> checkedLists;
			GetKeywordsOnListFormsNested(akFormlist, checkedLists, v);
		}
		
		gfuncs::RemoveDuplicates(v);
		
		return v;
	}
	
	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("GetKeywordString", "DbSkseFunctions", GetKeywordString);
        vm->RegisterFunction("SetKeywordString", "DbSkseFunctions", SetKeywordString);
        vm->RegisterFunction("GetKeywordsForString", "DbSkseFunctions", GetKeywordsForString);
        vm->RegisterFunction("GetKeywordsOnArrayForms", "DbSkseFunctions", GetKeywordsOnArrayForms);
        vm->RegisterFunction("GetKeywordsOnListForms", "DbSkseFunctions", GetKeywordsOnListForms);
		return true;
	}
}