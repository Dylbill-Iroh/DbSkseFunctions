#include "Keyboard.h"
#include "GeneralFunctions.h"

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

	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("GetKeywordString", "DbSkseFunctions", GetKeywordString);
        vm->RegisterFunction("SetKeywordString", "DbSkseFunctions", SetKeywordString);
		return true;
	}
}