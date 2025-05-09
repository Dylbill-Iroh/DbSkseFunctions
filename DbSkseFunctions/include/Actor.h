#pragma once 

extern std::map<int, std::string>ActorValueIntsMap;
extern std::map<std::string, int>ActorValuesMap;
extern std::map<RE::Actor*, RE::TESRace*> savedActorRacesMap;
extern bool actorRacesSaved; 

bool actorHasBowEquipped(RE::Actor* actor);

int GetActorValueInt(std::string actorValue);

void SaveActorRaces();

namespace actor {
	/*bool IsActorPowerAttacking(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool WouldActorBeStealing(RE::StaticFunctionTag*, RE::Actor* akActor, RE::TESObjectREFR* akTarget);

	bool IsActorAttacking(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorSpeaking(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorBlocking(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorCasting(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorDualCasting(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorStaggered(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorRecoiling(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorIgnoringCombat(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorUndead(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorOnFlyingMount(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorFleeing(RE::StaticFunctionTag*, RE::Actor* akActor);

	int GetActorWardState(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorAMount(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorInMidAir(RE::StaticFunctionTag*, RE::Actor* akActor);

	bool IsActorInRagdollState(RE::StaticFunctionTag*, RE::Actor* akActor);

	int GetDetectionLevel(RE::StaticFunctionTag*, RE::Actor* akActor, RE::Actor* akTarget);*/

	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}
