#include <condition_variable>
#include "Cell.h"
#include "GeneralFunctions.h"

namespace cell {
    RE::TESWorldSpace* GetCellWorldSpace(RE::StaticFunctionTag*, RE::TESObjectCELL* akCell) {
        if (gfuncs::IsFormValid(akCell)) {
            RE::TESWorldSpace* worldSpace = akCell->GetRuntimeData().worldSpace;
            if (gfuncs::IsFormValid(worldSpace)) {
                return worldSpace;
            }
        }

        return nullptr;
    }

    RE::BGSLocation* GetCellLocation(RE::StaticFunctionTag*, RE::TESObjectCELL* akCell) {
        if (gfuncs::IsFormValid(akCell)) {
            RE::BGSLocation* location = akCell->GetLocation();
            if (gfuncs::IsFormValid(location)) {
                return location;
            }
        }

        return nullptr;
    }

    bool IsCellPublic(RE::StaticFunctionTag*, RE::TESObjectCELL* akCell) {
        if (gfuncs::IsFormValid(akCell)) {
            return (akCell->cellFlags.any(RE::TESObjectCELL::Flag::kPublicArea));
        }
        return false;
    }

    void SetCellPublic(RE::StaticFunctionTag*, RE::TESObjectCELL* akCell, bool bPublic) {
        if (gfuncs::IsFormValid(akCell)) {
            akCell->SetPublic(bPublic);
        }
    }

    bool IsCellOffLimits(RE::StaticFunctionTag*, RE::TESObjectCELL* akCell) {
        if (gfuncs::IsFormValid(akCell)) {
            return ((akCell->formFlags & RE::TESObjectCELL::RecordFlags::kOffLimits) != 0);
        }
        return false;
    }

    void SetCellOffLimits(RE::StaticFunctionTag*, RE::TESObjectCELL* akCell, bool bOffLimits) {
        logger::info("called: {}", bOffLimits);

        if (gfuncs::IsFormValid(akCell)) {
            if (bOffLimits) {
                akCell->formFlags |= RE::TESObjectCELL::RecordFlags::kOffLimits;
            }
            else {
                akCell->formFlags &= ~RE::TESObjectCELL::RecordFlags::kOffLimits;
            }
            //akCell->AddChange(RE::TESForm::ChangeFlags::ChangeFlag::kFlags);
        }
    }

    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("GetCellWorldSpace", "DbSkseFunctions", GetCellWorldSpace);
        vm->RegisterFunction("GetCellLocation", "DbSkseFunctions", GetCellLocation);
        vm->RegisterFunction("SetCellPublic", "DbSkseFunctions", SetCellPublic);
        vm->RegisterFunction("IsCellPublic", "DbSkseFunctions", IsCellPublic);
        vm->RegisterFunction("IsCellOffLimits", "DbSkseFunctions", IsCellOffLimits);
        vm->RegisterFunction("SetCellOffLimits", "DbSkseFunctions", SetCellOffLimits);
        return true;
    }
}