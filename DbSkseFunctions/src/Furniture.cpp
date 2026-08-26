#include "Furniture.h"
#include "GeneralFunctions.h"
#include "Actor.h"

namespace furniture {
    int GetFurnitureWorkbenchType(RE::StaticFunctionTag*, RE::TESFurniture* akFurniture) {
        if (!gfuncs::IsFormValid(akFurniture)) {
            logger::warn("akFurniture doesn't exist or isn't valid");
            return -1;
        }

        return static_cast<int>(akFurniture->workBenchData.benchType.get());
    }

    int GetFurnitureWorkbenchSkillInt(RE::StaticFunctionTag*, RE::TESFurniture* akFurniture) {
        if (!gfuncs::IsFormValid(akFurniture)) {
            logger::warn("akFurniture doesn't exist or isn't valid");
            return -1;
        }

        return static_cast<int>(akFurniture->workBenchData.usesSkill.get());
    }

    std::string GetFurnitureWorkbenchSkillString(RE::StaticFunctionTag*, RE::TESFurniture* akFurniture) {
        if (!gfuncs::IsFormValid(akFurniture)) {
            logger::warn("akFurniture doesn't exist or isn't valid");
            return "";
        }

        int value = static_cast<int>(akFurniture->workBenchData.usesSkill.get());
        return ActorValueIntsMap[value];
    } 

	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("GetFurnitureWorkbenchType", "DbSkseFunctions", GetFurnitureWorkbenchType);
        vm->RegisterFunction("GetFurnitureWorkbenchSkillInt", "DbSkseFunctions", GetFurnitureWorkbenchSkillInt);
        vm->RegisterFunction("GetFurnitureWorkbenchSkillString", "DbSkseFunctions", GetFurnitureWorkbenchSkillString);
		return true;
	}
}