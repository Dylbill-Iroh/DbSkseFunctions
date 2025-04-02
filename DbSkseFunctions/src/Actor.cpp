#include "Actor.h"
#include "GeneralFunctions.h"

std::map<RE::Actor*, RE::TESRace*> savedActorRacesMap;

std::map<int, std::string>ActorValueIntsMap = {
    {0, "aggression"},
    {1, "confidence"},
    {2, "energy"},
    {3, "morality"},
    {4, "mood"},
    {5, "assistance"},
    {6, "onehanded"},
    {7, "twohanded"},
    {8, "marksman"},
    {9, "block"},
    {10, "smithing"},
    {11, "heavyarmor"},
    {12, "lightarmor"},
    {13, "pickpocket"},
    {14, "lockpicking"},
    {15, "sneak"},
    {16, "alchemy"},
    {17, "speechcraft"},
    {18, "alteration"},
    {19, "conjuration"},
    {20, "destruction"},
    {21, "illusion"},
    {22, "restoration"},
    {23, "enchanting"},
    {24, "health"},
    {25, "magicka"},
    {26, "stamina"},
    {27, "healrate"},
    {28, "magickarate"},
    {29, "staminarate"},
    {30, "speedmult"},
    {31, "inventoryweight"},
    {32, "carryweight"},
    {33, "criticalchance"},
    {34, "meleedamage"},
    {35, "unarmeddamage"},
    {36, "mass"},
    {37, "voicepoints"},
    {38, "voicerate"},
    {39, "damageresist"},
    {40, "poisonresist"},
    {41, "resistfire"},
    {42, "resistshock"},
    {43, "resistfrost"},
    {44, "resistmagic"},
    {45, "resistdisease"},
    {46, "perceptioncondition"},
    {47, "endurancecondition"},
    {48, "leftattackcondition"},
    {49, "rightattackcondition"},
    {50, "leftmobilitycondition"},
    {51, "rightmobilitycondition"},
    {52, "braincondition"},
    {53, "paralysis"},
    {54, "invisibility"},
    {55, "nighteye"},
    {56, "detectliferange"},
    {57, "waterbreathing"},
    {58, "waterwalking"},
    {59, "ignorecrippledlimbs"},
    {60, "fame"},
    {61, "infamy"},
    {62, "jumpingbonus"},
    {63, "wardpower"},
    {64, "rightitemcharge"},
    {65, "armorperks"},
    {66, "shieldperks"},
    {67, "warddeflection"},
    {68, "variable01"},
    {69, "variable02"},
    {70, "variable03"},
    {71, "variable04"},
    {72, "variable05"},
    {73, "variable06"},
    {74, "variable07"},
    {75, "variable08"},
    {76, "variable09"},
    {77, "variable10"},
    {78, "bowspeedbonus"},
    {79, "favoractive"},
    {80, "favorsperday"},
    {81, "favorsperdaytimer"},
    {82, "leftitemcharge"},
    {83, "absorbchance"},
    {84, "blindness"},
    {85, "weaponspeedmult"},
    {86, "shoutrecoverymult"},
    {87, "bowstaggerbonus"},
    {88, "telekinesis"},
    {89, "favorpointsbonus"},
    {90, "lastbribedintimidated"},
    {91, "lastflattered"},
    {92, "movementnoisemult"},
    {93, "bypassvendorstolencheck"},
    {94, "bypassvendorkeywordcheck"},
    {95, "waitingforplayer"},
    {96, "onehandedmodifier"},
    {97, "twohandedmodifier"},
    {98, "marksmanmodifier"},
    {99, "blockmodifier"},
    {100, "smithingmodifier"},
    {101, "heavyarmormodifier"},
    {102, "lightarmormodifier"},
    {103, "pickpocketmodifier"},
    {104, "lockpickingmodifier"},
    {105, "sneakingmodifier"},
    {106, "alchemymodifier"},
    {107, "speechcraftmodifier"},
    {108, "alterationmodifier"},
    {109, "conjurationmodifier"},
    {110, "destructionmodifier"},
    {111, "illusionmodifier"},
    {112, "restorationmodifier"},
    {113, "enchantingmodifier"},
    {114, "onehandedskilladvance"},
    {115, "twohandedskilladvance"},
    {116, "marksmanskilladvance"},
    {117, "blockskilladvance"},
    {118, "smithingskilladvance"},
    {119, "heavyarmorskilladvance"},
    {120, "lightarmorskilladvance"},
    {121, "pickpocketskilladvance"},
    {122, "lockpickingskilladvance"},
    {123, "sneakingskilladvance"},
    {124, "alchemyskilladvance"},
    {125, "speechcraftskilladvance"},
    {126, "alterationskilladvance"},
    {127, "conjurationskilladvance"},
    {128, "destructionskilladvance"},
    {129, "illusionskilladvance"},
    {130, "restorationskilladvance"},
    {131, "enchantingskilladvance"},
    {132, "leftweaponspeedmultiply"},
    {133, "dragonsouls"},
    {134, "combathealthregenmultiply"},
    {135, "onehandedpowermodifier"},
    {136, "twohandedpowermodifier"},
    {137, "marksmanpowermodifier"},
    {138, "blockpowermodifier"},
    {139, "smithingpowermodifier"},
    {140, "heavyarmorpowermodifier"},
    {141, "lightarmorpowermodifier"},
    {142, "pickpocketpowermodifier"},
    {143, "lockpickingpowermodifier"},
    {144, "sneakingpowermodifier"},
    {145, "alchemypowermodifier"},
    {146, "speechcraftpowermodifier"},
    {147, "alterationpowermodifier"},
    {148, "conjurationpowermodifier"},
    {149, "destructionpowermodifier"},
    {150, "illusionpowermodifier"},
    {151, "restorationpowermodifier"},
    {152, "enchantingpowermodifier"},
    {153, "dragonrend"},
    {154, "attackdamagemult"},
    {155, "healratemult"},
    {156, "magickarate"},
    {157, "staminarate"},
    {158, "werewolfperks"},
    {159, "vampireperks"},
    {160, "grabactoroffset"},
    {161, "grabbed"},
    {162, "deprecated05"},
    {163, "reflectdamage"}
};

std::map<std::string, int>ActorValuesMap = {
    {"aggression", 0},
    {"confidence", 1},
    {"energy", 2},
    {"morality", 3},
    {"mood", 4},
    {"assistance", 5},
    {"onehanded", 6},
    {"twohanded", 7},
    {"marksman", 8},
    {"block", 9},
    {"smithing", 10},
    {"heavyarmor", 11},
    {"lightarmor", 12},
    {"pickpocket", 13},
    {"lockpicking", 14},
    {"sneak", 15},
    {"alchemy", 16},
    {"speechcraft", 17},
    {"alteration", 18},
    {"conjuration", 19},
    {"destruction", 20},
    {"illusion", 21},
    {"restoration", 22},
    {"enchanting", 23},
    {"health", 24},
    {"magicka", 25},
    {"stamina", 26},
    {"healrate", 27},
    {"magickarate", 28},
    {"staminarate", 29},
    {"speedmult", 30},
    {"inventoryweight", 31},
    {"carryweight", 32},
    {"criticalchance", 33},
    {"meleedamage", 34},
    {"unarmeddamage", 35},
    {"mass", 36},
    {"voicepoints", 37},
    {"voicerate", 38},
    {"damageresist", 39},
    {"poisonresist", 40},
    {"resistfire", 41},
    {"resistshock", 42},
    {"resistfrost", 43},
    {"resistmagic", 44},
    {"resistdisease", 45},
    {"perceptioncondition", 46},
    {"endurancecondition", 47},
    {"leftattackcondition", 48},
    {"rightattackcondition", 49},
    {"leftmobilitycondition", 50},
    {"rightmobilitycondition", 51},
    {"braincondition", 52},
    {"paralysis", 53},
    {"invisibility", 54},
    {"nighteye", 55},
    {"detectliferange", 56},
    {"waterbreathing", 57},
    {"waterwalking", 58},
    {"ignorecrippledlimbs", 59},
    {"fame", 60},
    {"infamy", 61},
    {"jumpingbonus", 62},
    {"wardpower", 63},
    {"rightitemcharge", 64},
    {"armorperks", 65},
    {"shieldperks", 66},
    {"warddeflection", 67},
    {"variable01", 68},
    {"variable02", 69},
    {"variable03", 70},
    {"variable04", 71},
    {"variable05", 72},
    {"variable06", 73},
    {"variable07", 74},
    {"variable08", 75},
    {"variable09", 76},
    {"variable10", 77},
    {"bowspeedbonus", 78},
    {"favoractive", 79},
    {"favorsperday", 80},
    {"favorsperdaytimer", 81},
    {"leftitemcharge", 82},
    {"absorbchance", 83},
    {"blindness", 84},
    {"weaponspeedmult", 85},
    {"shoutrecoverymult", 86},
    {"bowstaggerbonus", 87},
    {"telekinesis", 88},
    {"favorpointsbonus", 89},
    {"lastbribedintimidated", 90},
    {"lastflattered", 91},
    {"movementnoisemult", 92},
    {"bypassvendorstolencheck", 93},
    {"bypassvendorkeywordcheck", 94},
    {"waitingforplayer", 95},
    {"onehandedmodifier", 96},
    {"twohandedmodifier", 97},
    {"marksmanmodifier", 98},
    {"blockmodifier", 99},
    {"smithingmodifier", 100},
    {"heavyarmormodifier", 101},
    {"lightarmormodifier", 102},
    {"pickpocketmodifier", 103},
    {"lockpickingmodifier", 104},
    {"sneakingmodifier", 105},
    {"alchemymodifier", 106},
    {"speechcraftmodifier", 107},
    {"alterationmodifier", 108},
    {"conjurationmodifier", 109},
    {"destructionmodifier", 110},
    {"illusionmodifier", 111},
    {"restorationmodifier", 112},
    {"enchantingmodifier", 113},
    {"onehandedskilladvance", 114},
    {"twohandedskilladvance", 115},
    {"marksmanskilladvance", 116},
    {"blockskilladvance", 117},
    {"smithingskilladvance", 118},
    {"heavyarmorskilladvance", 119},
    {"lightarmorskilladvance", 120},
    {"pickpocketskilladvance", 121},
    {"lockpickingskilladvance", 122},
    {"sneakingskilladvance", 123},
    {"alchemyskilladvance", 124},
    {"speechcraftskilladvance", 125},
    {"alterationskilladvance", 126},
    {"conjurationskilladvance", 127},
    {"destructionskilladvance", 128},
    {"illusionskilladvance", 129},
    {"restorationskilladvance", 130},
    {"enchantingskilladvance", 131},
    {"leftweaponspeedmultiply", 132},
    {"dragonsouls", 133},
    {"combathealthregenmultiply", 134},
    {"onehandedpowermodifier", 135},
    {"twohandedpowermodifier", 136},
    {"marksmanpowermodifier", 137},
    {"blockpowermodifier", 138},
    {"smithingpowermodifier", 139},
    {"heavyarmorpowermodifier", 140},
    {"lightarmorpowermodifier", 141},
    {"pickpocketpowermodifier", 142},
    {"lockpickingpowermodifier", 143},
    {"sneakingpowermodifier", 144},
    {"alchemypowermodifier", 145},
    {"speechcraftpowermodifier", 146},
    {"alterationpowermodifier", 147},
    {"conjurationpowermodifier", 148},
    {"destructionpowermodifier", 149},
    {"illusionpowermodifier", 150},
    {"restorationpowermodifier", 151},
    {"enchantingpowermodifier", 152},
    {"dragonrend", 153},
    {"attackdamagemult", 154},
    {"healratemult", 155},
    {"magickarate", 156},
    {"staminarate", 157},
    {"werewolfperks", 158},
    {"vampireperks", 159},
    {"grabactoroffset", 160},
    {"grabbed", 161},
    {"deprecated05", 162},
    { "reflectdamage", 163}
};
bool actorRacesSaved = false;

bool actorHasBowEquipped(RE::Actor* actor) {
    if (gfuncs::IsFormValid(actor)) {
        RE::TESForm* equippedObj = actor->GetEquippedObject(false); //right hand
        if (gfuncs::formIsBowOrCrossbow(equippedObj)) {
            return true;
        }

        equippedObj = actor->GetEquippedObject(true); //left hand
        if (gfuncs::formIsBowOrCrossbow(equippedObj)) {
            return true;
        }
    }
    return false;
}

int GetActorValueInt(std::string actorValue) {
    if (actorValue == "") {
        return -1;
    }

    std::transform(actorValue.begin(), actorValue.end(), actorValue.begin(), tolower);
    int value = ActorValuesMap[actorValue];
    if (value == 0 && actorValue != "aggression") {
        return -2;
    }
    else {
        return value;
    }
}

void SaveActorRaces() {
    if (!actorRacesSaved) {
        actorRacesSaved = true;

        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                RE::Actor* actor = form->As<RE::Actor>();
                if (gfuncs::IsFormValid(actor)) {
                    savedActorRacesMap[actor] = actor->GetRace();
                }
            }
        }
    }
    //gfuncs::logFormMap(savedActorRacesMap);
}

//=============================================================================

RE::TESCondition* condition_isPowerAttacking;
bool IsActorPowerAttacking(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!condition_isPowerAttacking) {
        logger::debug("creating condition_isPowerAttacking condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsPowerAttacking;

        condition_isPowerAttacking = new RE::TESCondition;
        condition_isPowerAttacking->head = conditionItem;
    }

    return condition_isPowerAttacking->IsTrue(akActor, nullptr);
}

bool WouldActorBeStealing(RE::StaticFunctionTag*, RE::Actor* akActor, RE::TESObjectREFR* akTarget) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!gfuncs::IsFormValid(akTarget)) {
        logger::warn("error, akTarget doesn't exist");
        return false;
    }

    return akActor->WouldBeStealing(akTarget);
}

RE::TESCondition* condition_IsAttacking;
bool IsActorAttacking(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!condition_IsAttacking) {
        logger::debug("creating condition_IsAttacking condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsAttacking;

        condition_IsAttacking = new RE::TESCondition;
        condition_IsAttacking->head = conditionItem;
    }

    return condition_IsAttacking->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_isActorSpeaking;
bool IsActorSpeaking(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!condition_isActorSpeaking) {
        logger::debug("creating condition_isActorSpeaking condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsTalking;

        condition_isActorSpeaking = new RE::TESCondition;
        condition_isActorSpeaking->head = conditionItem;
    }

    return condition_isActorSpeaking->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsBlocking;
bool IsActorBlocking(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!condition_IsBlocking) {
        logger::debug("creating condition_IsBlocking condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsBlocking;

        condition_IsBlocking = new RE::TESCondition;
        condition_IsBlocking->head = conditionItem;
    }

    return condition_IsBlocking->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsCasting;
bool IsActorCasting(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!condition_IsCasting) {
        logger::debug("creating condition_IsCasting condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsCasting;

        condition_IsCasting = new RE::TESCondition;
        condition_IsCasting->head = conditionItem;
    }

    return condition_IsCasting->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsDualCasting;
bool IsActorDualCasting(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!condition_IsDualCasting) {
        logger::debug("creating condition_IsDualCasting condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsDualCasting;

        condition_IsDualCasting = new RE::TESCondition;
        condition_IsDualCasting->head = conditionItem;
    }

    return condition_IsDualCasting->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsStaggered;
bool IsActorStaggered(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!condition_IsStaggered) {
        logger::debug("creating condition_IsStaggered condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsStaggered;

        condition_IsStaggered = new RE::TESCondition;
        condition_IsStaggered->head = conditionItem;
    }

    return condition_IsStaggered->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsRecoiling;
bool IsActorRecoiling(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!condition_IsRecoiling) {
        logger::debug("creating condition_IsRecoiling condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsRecoiling;

        condition_IsRecoiling = new RE::TESCondition;
        condition_IsRecoiling->head = conditionItem;
    }

    return condition_IsRecoiling->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsIgnoringCombat;
bool IsActorIgnoringCombat(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!condition_IsIgnoringCombat) {
        logger::debug("creating condition_IsIgnoringCombat condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsIgnoringCombat;

        condition_IsIgnoringCombat = new RE::TESCondition;
        condition_IsIgnoringCombat->head = conditionItem;
    }

    return condition_IsIgnoringCombat->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsUndead;
bool IsActorUndead(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!condition_IsUndead) {
        logger::debug("creating condition_IsUndead condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsUndead;

        condition_IsUndead = new RE::TESCondition;
        condition_IsUndead->head = conditionItem;
    }

    return condition_IsUndead->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsOnFlyingMount;
bool IsActorOnFlyingMount(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!condition_IsOnFlyingMount) {
        logger::debug("creating condition_IsOnFlyingMount condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsOnFlyingMount;

        condition_IsOnFlyingMount = new RE::TESCondition;
        condition_IsOnFlyingMount->head = conditionItem;
    }

    return condition_IsOnFlyingMount->IsTrue(akActor, nullptr);
}

RE::TESCondition* condition_IsFleeing;
bool IsActorFleeing(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    if (!condition_IsFleeing) {
        logger::debug("creating condition_IsFleeing condition");
        auto* conditionItem = new RE::TESConditionItem;
        conditionItem->data.comparisonValue.f = 1.0f;
        conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsFleeing;

        condition_IsFleeing = new RE::TESCondition;
        condition_IsFleeing->head = conditionItem;
    }

    return condition_IsFleeing->IsTrue(akActor, nullptr);
}

int GetActorWardState(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }

    auto* aiProcess = akActor->GetActorRuntimeData().currentProcess;
    if (aiProcess) {
        if (aiProcess->middleHigh) {
            return static_cast<int>(aiProcess->middleHigh->wardState);
        }
    }

    return 0;
}

bool IsActorAMount(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }
    return akActor->IsAMount();
}

bool IsActorInMidAir(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }
    return akActor->IsInMidair();
}

bool IsActorInRagdollState(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return false;
    }
    return akActor->IsInRagdollState();
}

int GetDetectionLevel(RE::StaticFunctionTag*, RE::Actor* akActor, RE::Actor* akTarget) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("error, akActor doesn't exist");
        return -1;
    }

    if (!akTarget) {
        logger::warn("error, akTarget doesn't exist");
        return -1;
    }
    return akActor->RequestDetectionLevel(akTarget);
}