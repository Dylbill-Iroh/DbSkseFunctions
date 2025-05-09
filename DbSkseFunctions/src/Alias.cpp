#include "Alias.h"
#include "GeneralFunctions.h"

namespace alias {
	bool SetAliasQuestObjectFlag(RE::StaticFunctionTag*, RE::BGSBaseAlias* akAlias, bool set) {
		return gfuncs::SetAliasQuestObjectFlag(akAlias, set);
	}

	bool IsAliasQuestObjectFlagSet(RE::StaticFunctionTag*, RE::BGSBaseAlias* akAlias) {
		return gfuncs::IsAliasQuestObjectFlagSet(akAlias);
	}

	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
		vm->RegisterFunction("SetAliasQuestObjectFlag", "DbSkseFunctions", SetAliasQuestObjectFlag);
		vm->RegisterFunction("IsAliasQuestObjectFlagSet", "DbSkseFunctions", IsAliasQuestObjectFlagSet);

		return true;
	}
}