#include "ObjectReference.h"
#include "GeneralFunctions.h"
#include "RE/A/Actor.h"
#include "RE/B/BGSPerkEntry.h"
#include "RE/E/ExtraDataList.h"
#include "RE/RTTI.h"
#include "RE/T/TESBoundObject.h"
#include "RE/T/TESObjectREFR.h"
#include "RE/T/TESObjectWEAP.h"
#include "RE/T/TypeTraits.h"
#include "SharedVariables.h"
#include <cstddef>
#undef max
#undef min

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

        logger::trace("{}", gfuncs::GetFormDataString(ref));
        ref->UpdateRefLight();
    }

    std::vector<float> GetRefLinearVelocity(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
        std::vector<float> v = { 0.0, 0.0, 0.0 };
        if (!gfuncs::IsFormValid(ref)) {
            logger::warn("error, ref doesn't exist");
            return v;
        }

        logger::trace("ref[{}]", gfuncs::GetFormName(ref));
        RE::NiPoint3 velocity;
        ref->GetLinearVelocity(velocity);
        v[0] = velocity.x;
        v[1] = velocity.y;
        v[2] = velocity.z;
        return v;
    } 

	std::vector<float> GetDoorTeleportMarkerPositionAndRotation(RE::StaticFunctionTag*, RE::TESObjectREFR* ref){
		std::vector<float> v(6); 
		if (!gfuncs::IsFormValid(ref)) {
            logger::warn("error, ref doesn't exist");
            return v;
        } 
		
		RE::ExtraTeleport* extraTeleport = ref->extraList.GetByType<RE::ExtraTeleport>();
		if (!extraTeleport){
			logger::warn("extraTeleport for ref[{}] not found", gfuncs::GetFormNameAndId(ref));
			return v;
		} 
		
		if (!extraTeleport->teleportData){
			logger::warn("extraTeleport->teleportData for ref[{}] not found", gfuncs::GetFormNameAndId(ref));
			return v;
		}
		
		v[0] = extraTeleport->teleportData->position.x;
		v[1] = extraTeleport->teleportData->position.y;
		v[2] = extraTeleport->teleportData->position.z;
		v[3] = gfuncs::RadiansToDegrees(extraTeleport->teleportData->rotation.x);
		v[4] = gfuncs::RadiansToDegrees(extraTeleport->teleportData->rotation.y);
		v[5] = gfuncs::RadiansToDegrees(extraTeleport->teleportData->rotation.z);
		
		return v;
	}
	
	RE::TESObjectREFR* GetRefContainer(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
		std::vector<RE::TESObjectREFR*> refs;

		if (!gfuncs::IsFormValid(ref)) {
			logger::warn("ref doesn't exist");
			return nullptr;
		}
		
		//if 3D loaded or has parent cell, the ref is not in an inventory
		if (ref->Is3DLoaded() || ref->GetParentCell()) {
			const auto& [allForms, lock] = RE::TESForm::GetAllForms();
			for (auto& [id, form] : *allForms) {
				if (gfuncs::IsFormValid(form, false, false)){
					auto* containerRef = form->AsReference();
					if (gfuncs::ContainerContainsRef(containerRef, ref, true)) {
						return containerRef;
					}
				}
			}
		}
		return nullptr;
	}
	
	int GetRefContainerGoldValue(RE::TESObjectREFR* a_container, RE::TESBoundObject* a_item) {
		if (!a_container || !a_item) { return 0; }

		auto inv = a_container->GetInventory([&](RE::TESBoundObject& obj) { return &obj == a_item; });
		auto it = inv.find(a_item);
		if (it != inv.end() && it->second.second) {
			return it->second.second->GetValue();
		}
		return a_item->GetGoldValue();   // fall back to base value
	}
	
	int GetRefGoldValue(RE::StaticFunctionTag*, RE::TESObjectREFR* ref){
		if (!gfuncs::IsFormValid(ref)) {
			logger::warn("ref doesn't exist");
			return 0;
		}
		
		RE::TESBoundObject* base = ref->GetObjectReference();
    	if (!base) { 
			logger::debug("no base TESBoundObject, returning ref->GetGoldValue()");
			return ref->GetGoldValue(); 
		}

		// could possibly get an InventoryEntryData with more than 1 in the stack, getting the wrong value.
		// auto* container = GetRefContainer(nullptr, ref);
		// if (gfuncs::IsFormValid(container)){
		// 	return GetRefContainerGoldValue(container, base);
		// }
		
		// countDelta 1 -- always the per-item value, never a stack total
		RE::InventoryEntryData entry(base, 1);
		entry.AddExtraList(&ref->extraList);
		int value = entry.GetValue(); 
		// entry.extraLists->clear(); //handled by the destructor 
		return value;
	}
	
	int GetFormGoldValue(RE::StaticFunctionTag*, RE::TESForm* akForm){
		if (!gfuncs::IsFormValid(akForm)) {
			logger::warn("akForm doesn't exist");
			return 0;
		}
		
		RE::TESObjectREFR* ref = skyrim_cast<RE::TESObjectREFR*>(akForm);
		if (gfuncs::IsFormValid(ref)){
			return GetRefGoldValue(nullptr, ref);
		}
		
		RE::TESBoundObject* base = skyrim_cast<RE::TESBoundObject*>(akForm);
    	if (!base) { 
			logger::debug("no base TESBoundObject, returning akForm->GetGoldValue()");
			return akForm->GetGoldValue(); 
		}
		
		RE::InventoryEntryData entry(base, 1);
		int value = entry.GetValue(); 
		return value;
	}
	
	int CalculateBarterValue(RE::StaticFunctionTag*, RE::Actor* merchant, RE::TESForm* akForm, bool buying) {
		auto* gmst   = RE::GameSettingCollection::GetSingleton();
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!gmst || !player) { return 0; }

		if (!gfuncs::IsFormValid(akForm)) {
			logger::warn("akForm doesn't exist");
			return 0;
		}
		
		const int base = GetFormGoldValue(nullptr, akForm);
		if (base <= 0) { return 0; }

		auto setting = [&](const char* n, float fallback) {
			auto* s = gmst->GetSetting(n);
			return s ? s->GetFloat() : fallback;
		};

		const float fMax     = setting("fBarterMax", 3.3f);
		const float fMin     = setting("fBarterMin", 2.0f);
		const float fBuyMin  = setting("fBarterBuyMin", 1.05f);
		const float fSellMax = setting("fBarterSellMax", 0.95f);

		const float speech = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSpeech);
		// const float SpeechCraftPowerMod = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSpeechcraftPowerModifier);
		// handled by HandleEntryPoint
		
		float mult = 1.0f;
		auto entryPoint = buying ? RE::BGSEntryPoint::ENTRY_POINT::kModBuyPrices : RE::BGSEntryPoint::ENTRY_POINT::kModSellPrices;
		RE::BGSEntryPoint::HandleEntryPoint(entryPoint, player, merchant, std::addressof(mult));
		
		// numerator: 3.3 - (1.3 * skill/100)
		float factor = fMax - ((fMax - fMin) * std::clamp(speech, 0.0f, 100.0f) / 100.0f);

		float value;
		if (buying) {
			value = base * std::max(factor, fBuyMin);
		} else {
			value = base * std::min(1.0f / factor, fSellMax);
		}
		
		// value *= mult;
		// return std::max(buying ? 1 : 0, static_cast<int>(value));
		
		float rValue = std::roundf(value);
		// float rValue = value;
		rValue *= mult;
		
		logger::trace("speech[{}] pfactor[{}] mult[{}] value[{}] result[{}]",
			speech, factor, mult, value, rValue);
		
		return std::max(buying ? 1 : 0, static_cast<int>(std::roundf(rValue)));
	}
	
    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("GetAshPileLinkedRef", "DbSkseFunctions", GetAshPileLinkedRef);
        vm->RegisterFunction("GetClosestObjectFromRef", "DbSkseFunctions", GetClosestObjectFromRef);
        vm->RegisterFunction("GetClosestObjectIndexFromRef", "DbSkseFunctions", GetClosestObjectIndexFromRef);
        vm->RegisterFunction("HasCollision", "DbSkseFunctions", HasCollision);
        vm->RegisterFunction("UpdateRefLight", "DbSkseFunctions", UpdateRefLight);
        vm->RegisterFunction("GetRefLinearVelocity", "DbSkseFunctions", GetRefLinearVelocity);
        vm->RegisterFunction("GetDoorTeleportMarkerPositionAndRotation", "DbSkseFunctions", GetDoorTeleportMarkerPositionAndRotation);
        vm->RegisterFunction("GetRefContainer", "DbSkseFunctions", GetRefContainer);
        vm->RegisterFunction("GetRefGoldValue", "DbSkseFunctions", GetRefGoldValue);
        vm->RegisterFunction("GetFormGoldValue", "DbSkseFunctions", GetFormGoldValue);
        vm->RegisterFunction("CalculateBarterValue", "DbSkseFunctions", CalculateBarterValue);
        return true;
    }
}