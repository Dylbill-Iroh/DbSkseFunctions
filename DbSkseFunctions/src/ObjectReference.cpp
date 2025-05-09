#include "ObjectReference.h"
#include "GeneralFunctions.h"

namespace objectRef {
    RE::TESObjectREFR* GetAshPileLinkedRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
        RE::TESObjectREFR* returnRef = nullptr;

        if (!gfuncs::IsFormValid(ref)) {
            logger::warn("ref doesn't exist or isn't valid");
            return returnRef;
        }

        auto* data = ref->extraList.GetByType<RE::ExtraAshPileRef>();
        if (data) {
            returnRef = gfuncs::GetRefFromObjectRefHandle(data->ashPileRef);
        }
        return returnRef;
    }

    RE::TESObjectREFR* GetClosestObjectFromRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, std::vector<RE::TESObjectREFR*> refs) {
        if (!ref) {
            return nullptr;
        }

        RE::TESObjectREFR* returnRef = nullptr;
        const auto refPosition = ref->GetPosition();
        float distance;
        int i = 0;

        while (i < refs.size() && returnRef == nullptr) { //find first valid ref and distance
            if (gfuncs::IsFormValid(refs[i])) {
                distance = refPosition.GetDistance(refs[i]->GetPosition());
                returnRef = refs[i];
            }
            i++;
        }

        while (i < refs.size()) {
            if (gfuncs::IsFormValid(refs[i])) {
                float refDistance = refPosition.GetDistance(refs[i]->GetPosition());
                if (refDistance < distance) {
                    distance = refDistance;
                    returnRef = refs[i];
                }
            }
            i++;
        }
        return returnRef;
    }

    int GetClosestObjectIndexFromRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, std::vector<RE::TESObjectREFR*> refs) {
        int iReturn = -1;
        if (!ref) {
            return iReturn;
        }

        const auto refPosition = ref->GetPosition();
        float distance;
        int i = 0;

        while (i < refs.size() && iReturn == -1) { //find first valid ref and distance
            if (gfuncs::IsFormValid(refs[i])) {
                distance = refPosition.GetDistance(refs[i]->GetPosition());
                iReturn = i;
            }
            i++;
        }

        while (i < refs.size()) {
            if (gfuncs::IsFormValid(refs[i])) {
                float refDistance = refPosition.GetDistance(refs[i]->GetPosition());
                if (refDistance < distance) {
                    distance = refDistance;
                    iReturn = i;
                }
            }
            i++;
        }
        return iReturn;
    }

    bool HasCollision(RE::StaticFunctionTag*, RE::TESObjectREFR* objRef) {
        logger::trace("called");

        if (!gfuncs::IsFormValid(objRef)) {
            logger::warn("objRef doesn't exist");
            return false;
        }
        return objRef->HasCollision();
    } 

    void UpdateRefLight(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
        if (!gfuncs::IsFormValid(ref)) {
            logger::warn("ref doesn't exist");
            return;
        }

        logger::info("{}", gfuncs::GetFormDataString(ref));
        ref->UpdateRefLight();
    }

    std::vector<float> GetRefLinearVelocity(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
        std::vector<float> v = { 0.0, 0.0, 0.0 };
        if (!gfuncs::IsFormValid(ref)) {
            logger::warn("error, ref doesn't exist");
            return v;
        }

        logger::info("ref[{}]", gfuncs::GetFormName(ref));
        RE::NiPoint3 velocity;
        ref->GetLinearVelocity(velocity);
        v[0] = velocity.x;
        v[1] = velocity.y;
        v[2] = velocity.z;
        return v;
    } 

    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("GetAshPileLinkedRef", "DbSkseFunctions", GetAshPileLinkedRef);
        vm->RegisterFunction("GetClosestObjectFromRef", "DbSkseFunctions", GetClosestObjectFromRef);
        vm->RegisterFunction("GetClosestObjectIndexFromRef", "DbSkseFunctions", GetClosestObjectIndexFromRef);
        vm->RegisterFunction("HasCollision", "DbSkseFunctions", HasCollision);
        vm->RegisterFunction("UpdateRefLight", "DbSkseFunctions", UpdateRefLight);
        vm->RegisterFunction("GetRefLinearVelocity", "DbSkseFunctions", GetRefLinearVelocity);
        return true;
    }
}