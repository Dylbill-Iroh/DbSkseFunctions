#pragma once 

namespace furniture {
	int GetFurnitureWorkbenchType(RE::StaticFunctionTag*, RE::TESFurniture* akFurniture);

	int GetFurnitureWorkbenchSkillInt(RE::StaticFunctionTag*, RE::TESFurniture * akFurniture);

	std::string GetFurnitureWorkbenchSkillString(RE::StaticFunctionTag*, RE::TESFurniture * akFurniture);

	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}