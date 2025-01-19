#include <Windows.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <stdarg.h>
#include <winbase.h>
#include <iostream>
#include <stdio.h>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <string>
#include "mini/ini.h"
#include "UIEventHooks/Hooks.h"
#include "editorID.hpp"
#include "GeneralFunctions.h"
#include "STLThunk.h"

namespace logger = SKSE::log;
bool bPlayerIsInCombat = false;
bool bRegisteredForPlayerCombatChange = false;
bool inMenuMode = false;
bool gamePaused = false;
bool bIsLoadingSerialization = false;
bool bIsSavingSerialization = false;
bool DbSkseCppCallbackEventsAttached = false;
std::string lastMenuOpened;
std::vector<RE::BSFixedString> menusCurrentlyOpen;

std::vector<RE::BSFixedString> refActivatedMenus = {
    RE::DialogueMenu::MENU_NAME,
    RE::BarterMenu::MENU_NAME,
    RE::GiftMenu::MENU_NAME,
    RE::LockpickingMenu::MENU_NAME,
    RE::ContainerMenu::MENU_NAME,
    RE::BookMenu::MENU_NAME,
    RE::CraftingMenu::MENU_NAME
};

int numOfItemMenusCurrentOpen = 0;
std::vector<RE::BSFixedString> itemMenus = {
    RE::InventoryMenu::MENU_NAME,
    RE::BarterMenu::MENU_NAME,
    RE::ContainerMenu::MENU_NAME,
    RE::GiftMenu::MENU_NAME,
    RE::MagicMenu::MENU_NAME,
    RE::CraftingMenu::MENU_NAME,
    RE::FavoritesMenu::MENU_NAME
};

std::chrono::system_clock::time_point lastTimeMenuWasOpened;
std::chrono::system_clock::time_point lastTimeGameWasPaused;
std::map<RE::TESObjectBOOK*, int> skillBooksMap;
std::map<RE::Actor*, RE::TESRace*> savedActorRacesMap;
bool actorRacesSaved = false;
int gameTimerPollingInterval = 1500; //in milliseconds
int iMaxArrowsSavedPerReference = 0;
float secondsPassedGameNotPaused = 0.0;
float lastFrameDelta = 0.1;
std::vector<std::string> magicDescriptionTags = { "<mag>", "<dur>", "<area>" };
RE::PlayerCharacter* player;
//RE::Actor* gfuncs::playerRef;
RE::TESForm* nullForm;
RE::TESForm* xMarker;
RE::BSScript::IVirtualMachine* gvm;
//RE::SkyrimVM* gfuncs::svm;
RE::NiPoint3 zeroPosition{0.0, 0.0, 0.0};
RE::UI* ui;
RE::Calendar* calendar;
RE::ScriptEventSourceHolder* eventSourceholder;
const SKSE::LoadInterface* skseLoadInterface;

//forward dec
void AddSink(int index);
void RemoveSink(int index);
bool RegisterActorForBowDrawAnimEvent(RE::TESObjectREFR* actorRef);
bool UnRegisterActorForBowDrawAnimEvent(RE::TESObjectREFR* actorRef);
std::string GetFormEditorId(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string nullFormString);
int gfuncs::GetIndexInVector(std::vector<RE::TESObjectREFR*> v, RE::TESObjectREFR* element);
float GameHoursToRealTimeSeconds(RE::StaticFunctionTag*, float gameHours);

struct TrackedActorAmmoData {
    RE::TESAmmo* ammo;
    float gameTimeStamp; //game time when the ammo was saved with the BowReleased animation event (if the ammo changed between BowDraw and BowReleased)
    std::chrono::system_clock::time_point timeStamp; //time point when the ammo was saved with the BowReleased animation event  (if the ammo changed between BowDraw and BowReleased)
};


struct TrackedProjectileData {
    RE::Projectile* projectile;
    RE::TESObjectREFR* shooter;
    RE::TESObjectREFR* target;
    RE::TESAmmo* ammo;
    float gameTimeStamp; //game time when the projectile last had an impact event
    float lastImpactEventGameTimeStamp; //last time a projectile impact event was sent for this data
    RE::BGSProjectile* projectileBase;
    int impactResult;
    int collidedLayer;
    float distanceTraveled;
    std::string hitPartNodeName;
    RE::TESObjectREFR* projectileMarker;
    //RE::TESObjectREFR* targetMarker;

    //std::chrono::system_clock::time_point timeStamp;
    //uint32_t runTimeStamp;
};

std::map<RE::TESObjectREFR*, std::vector<TrackedProjectileData>> recentHitProjectiles;
std::map<RE::TESObjectREFR*, std::vector<TrackedProjectileData>> recentShotProjectiles;

//general functions============================================================================================================================================================

bool formIsBowOrCrossbow(RE::TESForm* akForm) {
    if (!gfuncs::IsFormValid(akForm)) {
        return false;
    }
    RE::TESObjectWEAP* weapon = akForm->As<RE::TESObjectWEAP>();
    if (gfuncs::IsFormValid(weapon)) {
        if (weapon->IsBow() || weapon->IsCrossbow()) {
            return true;
        }
    }
    return false;
}

bool actorHasBowEquipped(RE::Actor* actor) {
    if (gfuncs::IsFormValid(actor)) {
        RE::TESForm* equippedObj = actor->GetEquippedObject(false); //right hand
        if (formIsBowOrCrossbow(equippedObj)) {
            return true;
        }

        equippedObj = actor->GetEquippedObject(true); //left hand
        if (formIsBowOrCrossbow(equippedObj)) {
            return true;
        }
    }
    return false;
}

//when reading a skill book in game, it removes the skill from the book, not just the TeachesSkill flag
//this saves skill books and their respective skills for use with skill book functions below.
void SaveSkillBooks() {
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();

        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                if (gfuncs::IsFormValid(akBook)) {
                    if (akBook->TeachesSkill()) {
                        skillBooksMap[akBook] = static_cast<int>(akBook->GetSkill());
                    }
                }
            }
        }
    }
    //gfuncs::logFormMap(skillBooksMap);
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

bool IsScriptAttachedToHandle(RE::VMHandle& handle, RE::BSFixedString& sScriptName) {
    if (handle == NULL) {
        return false;
    }

    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        return false;
    }

    auto it = vm->attachedScripts.find(handle);
    if (it != vm->attachedScripts.end()) {
        for (auto& attachedScript : it->second) {
            if (attachedScript) {
                auto* script = attachedScript.get();
                if (script) {
                    auto info = script->GetTypeInfo();
                    if (info) {
                        if (info->name == sScriptName) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

RE::BSScript::Object* GetAttachedScriptObject(RE::VMHandle& handle, RE::BSFixedString& sScriptName) {
    if (handle == NULL) {
        return nullptr;
    }

    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        return nullptr;
    }

    auto it = vm->attachedScripts.find(handle);
    if (it != vm->attachedScripts.end()) {
        for (auto& attachedScript : it->second) {

            if (attachedScript) {
                auto* script = attachedScript.get();
                if (script) {
                    auto info = script->GetTypeInfo();
                    if (info) {
                        if (info->name == sScriptName) {
                            return script;
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

bool IsScriptAttachedToRef(RE::TESObjectREFR* ref, RE::BSFixedString sScriptName) {
    if (!gfuncs::IsFormValid(ref)) {
        return false;
    }

    RE::VMHandle handle = gfuncs::GetHandle(ref);
    return IsScriptAttachedToHandle(handle, sScriptName);
}

bool IsScriptAttachedToForm(RE::TESForm* akForm, RE::BSFixedString sScriptName) {
    if (!gfuncs::IsFormValid(akForm)) {
        return false;
    }

    RE::VMHandle handle = gfuncs::GetHandle(akForm);
    return IsScriptAttachedToHandle(handle, sScriptName);
}

void SetSettingsFromIniFile() {
    // first, create a file instance
    mINI::INIFile file("Data/SKSE/Plugins/DbSkseFunctions.ini");

    // next, create a structure that will hold data
    mINI::INIStructure ini;

    // now we can read the file
    file.read(ini);

    std::string intervalString = ini["Main"]["gameTimerPollingInterval"];
    gameTimerPollingInterval = std::stof(intervalString) * 1000; //convert float value from seconds to int milliseconds
    if (gameTimerPollingInterval <= 0) {
        gameTimerPollingInterval = 100;
    }

    std::string sMaxArrowsSavedPerReference = ini["Main"]["iMaxArrowsSavedPerReference"];
    iMaxArrowsSavedPerReference = std::stoi(sMaxArrowsSavedPerReference);

    logger::info("{} gameTimerPollingInterval set to {} ", __func__, gameTimerPollingInterval);
    logger::info("{} iMaxArrowsSavedPerReference set to {} ", __func__, iMaxArrowsSavedPerReference);
}

void SetupLog() {
    // first, create a file instance
    mINI::INIFile file("Data/SKSE/Plugins/DbSkseFunctions.ini");

    // next, create a structure that will hold data
    mINI::INIStructure ini;

    // now we can read the file
    file.read(ini);

    std::string akLevel = ini["LOG"]["iMinLevel"];
    spdlog::level::level_enum iLevel = static_cast<spdlog::level::level_enum>(std::stoi(akLevel));

    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(iLevel);
    spdlog::flush_on(spdlog::level::trace);
    logger::info("{} level set to {}", __func__, akLevel);
}

enum logLevel { trace, debug, info, warn, error, critical };
enum debugLevel { notification, messageBox };

void LogAndMessage(std::string message, int logLevel = trace, int debugLevel = notification) {
    switch (logLevel) {
    case trace:
        logger::trace("{}", message);
        break;

    case debug:
        logger::debug("{}", message);
        break;

    case info:
        logger::info("{}", message);
        break;

    case warn:
        logger::warn("{}", message);
        break;

    case error:
        logger::error("{}", message);
        break;

    case critical:
        logger::critical("{}", message);
        break;
    }

    /*switch (debugLevel) {
    case notification:
        RE::DebugNotification(message.data());
        break;
    case messageBox:
        RE::DebugMessageBox(message.data());
        break;
    }*/
}

void SendEvents(std::vector<RE::VMHandle> handles, RE::BSFixedString& sEvent, RE::BSScript::IFunctionArguments* args) {
    int max = handles.size();

    if (max == 0) {
        return;
    }

    for (int i = 0; i < max; i++) {
        gfuncs::svm->SendAndRelayEvent(handles[i], &sEvent, args, nullptr);
    }

    delete args; //args is created using makeFunctionArguments. Delete as it's no longer needed.
}

void CombineEventHandles(std::vector<RE::VMHandle>& handles, RE::TESForm* akForm, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& formHandles) {
    if (formHandles.size() == 0) {
        return;
    }

    if (!gfuncs::IsFormValid(akForm)) {
        return;
    }

    auto it = formHandles.find(akForm);

    if (it != formHandles.end()) {
        logger::trace("{}: events for form: [{}] ID[{:x}] found", __func__, gfuncs::GetFormName(akForm), akForm->GetFormID());
        handles.reserve(handles.size() + it->second.size());
        handles.insert(handles.end(), it->second.begin(), it->second.end());
    }
    else {
        logger::trace("{}: events for form: [{}] ID[{:x}] not found", __func__, gfuncs::GetFormName(akForm), akForm->GetFormID());
    }
}

//forward dec
std::string GetDescription(RE::TESForm* akForm, std::string newLineReplacer);

std::vector<std::string> GetFormDescriptionsAsStrings(std::vector<RE::TESForm*> akForms, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    std::vector<std::string> descriptions;

    if (noneStringType == 2 && maxCharacters > 0) {
        for (auto* akForm : akForms) {
            if (!gfuncs::IsFormValid(akForm)) {
                descriptions.push_back(nullFormString);
                continue;
            }
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = gfuncs::IntToHex(akForm->GetFormID());
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            descriptions.push_back(description);
        }
    }
    else if (noneStringType == 1 && maxCharacters > 0) {
        for (auto* akForm : akForms) {
            if (!gfuncs::IsFormValid(akForm)) {
                descriptions.push_back(nullFormString);
                continue;
            }
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = GetFormEditorId(nullptr, akForm, "");
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            descriptions.push_back(description);
        }
    }
    else if (noneStringType == 2) {
        for (auto* akForm : akForms) {
            if (!gfuncs::IsFormValid(akForm)) {
                descriptions.push_back(nullFormString);
                continue;
            }
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = gfuncs::IntToHex(akForm->GetFormID());
            }
            descriptions.push_back(description);
        }
    }
    else if (noneStringType == 1) {
        for (auto* akForm : akForms) {
            if (!gfuncs::IsFormValid(akForm)) {
                descriptions.push_back(nullFormString);
                continue;
            }
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = GetFormEditorId(nullptr, akForm, "");
            }
            descriptions.push_back(description);
        }
    }
    else if (maxCharacters > 0) {
        for (auto* akForm : akForms) {
            if (!gfuncs::IsFormValid(akForm)) {
                descriptions.push_back(nullFormString);
                continue;
            }
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            descriptions.push_back(description);
        }
    }
    else {
        for (auto* akForm : akForms) {
            if (!gfuncs::IsFormValid(akForm)) {
                descriptions.push_back(nullFormString);
                continue;
            }
            std::string description = GetDescription(akForm, newLineReplacer);
            descriptions.push_back(description);
        }
    }

    if (sortOption == 1 || sortOption == 2) {
        std::sort(descriptions.begin(), descriptions.end());
        if (sortOption == 2) {
            std::reverse(descriptions.begin(), descriptions.end());
        }
    }
    return descriptions;
}

std::vector<std::string> GetFormDescriptionsAsStrings(RE::BGSListForm* akFormlist, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    std::vector<std::string> descriptions;

    int m = akFormlist->forms.size();

    if (noneStringType == 2 && maxCharacters > 0) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            if (!gfuncs::IsFormValid(form)) {
                descriptions.push_back(nullFormString);
                return RE::BSContainer::ForEachResult::kContinue;
            }
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = gfuncs::IntToHex(form->GetFormID());
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 1 && maxCharacters > 0) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            if (!gfuncs::IsFormValid(form)) {
                descriptions.push_back(nullFormString);
                return RE::BSContainer::ForEachResult::kContinue;
            }
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = GetFormEditorId(nullptr, form, "");
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            descriptions.push_back(description);

            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 2) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            if (!gfuncs::IsFormValid(form)) {
                descriptions.push_back(nullFormString);
                return RE::BSContainer::ForEachResult::kContinue;
            }
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = gfuncs::IntToHex(form->GetFormID());
            }
            descriptions.push_back(description);

            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 1) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            if (!gfuncs::IsFormValid(form)) {
                descriptions.push_back(nullFormString);
                return RE::BSContainer::ForEachResult::kContinue;
            }
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = GetFormEditorId(nullptr, form, "");
            }
            descriptions.push_back(description);

            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (maxCharacters > 0) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            if (!gfuncs::IsFormValid(form)) {
                descriptions.push_back(nullFormString);
                return RE::BSContainer::ForEachResult::kContinue;
            }
            std::string description = GetDescription(form, newLineReplacer);
            if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            descriptions.push_back(description);

            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            if (!gfuncs::IsFormValid(form)) {
                descriptions.push_back(nullFormString);
                return RE::BSContainer::ForEachResult::kContinue;
            }
            std::string description = GetDescription(form, newLineReplacer);
            descriptions.push_back(description);

            return RE::BSContainer::ForEachResult::kContinue;
            });
    }

    if (sortOption == 1 || sortOption == 2) {
        std::sort(descriptions.begin(), descriptions.end());
        if (sortOption == 2) {
            std::reverse(descriptions.begin(), descriptions.end());
        }
    }
    return descriptions;
}

std::vector<std::string> getFormNamesAndDescriptionsAsStrings(std::vector<RE::TESForm*> akForms, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    std::vector<std::string> descriptions;

    if (noneStringType == 2 && maxCharacters > 0) {
        for (auto* akForm : akForms) {
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = gfuncs::IntToHex(akForm->GetFormID());
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
        }
    }
    else if (noneStringType == 1 && maxCharacters > 0) {
        for (auto* akForm : akForms) {
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = GetFormEditorId(nullptr, akForm, "");
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
        }
    }
    else if (noneStringType == 2) {
        for (auto* akForm : akForms) {
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = gfuncs::IntToHex(akForm->GetFormID());
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
        }
    }
    else if (noneStringType == 1) {
        for (auto* akForm : akForms) {
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description == "" && akForm) {
                description = GetFormEditorId(nullptr, akForm, "");
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
        }
    }
    else if (maxCharacters > 0) {
        for (auto* akForm : akForms) {
            std::string description = GetDescription(akForm, newLineReplacer);
            if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
        }
    }
    else {
        for (auto* akForm : akForms) {
            std::string description = GetDescription(akForm, newLineReplacer);
            std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
        }
    }

    if (sortOption == 3 || sortOption == 4) {
        std::sort(descriptions.begin(), descriptions.end());
        if (sortOption == 4) {
            std::reverse(descriptions.begin(), descriptions.end());
        }
    }
    return descriptions;
}

std::vector<std::string> getFormNamesAndDescriptionsAsStrings(RE::BGSListForm* akFormlist, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    std::vector<std::string> formNamesAndDescriptions;
    /*std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, "", "", false));
    formNamesAndDescriptions.push_back(name + "||" + description);*/
    int m = akFormlist->forms.size();

    std::vector<std::string> descriptions;
    if (noneStringType == 2 && maxCharacters > 0) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = gfuncs::IntToHex(form->GetFormID());
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 1 && maxCharacters > 0) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = GetFormEditorId(nullptr, form, "");
            }
            else if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 2) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = gfuncs::IntToHex(form->GetFormID());
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 1) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string description = GetDescription(form, newLineReplacer);
            if (description == "" && form) {
                description = GetFormEditorId(nullptr, form, "");
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (maxCharacters > 0) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string description = GetDescription(form, newLineReplacer);
            if (description.size() > maxCharacters) {
                description = description.substr(0, maxCharacters) + overMaxCharacterSuffix;
            }
            std::string name = static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string description = GetDescription(form, newLineReplacer);
            std::string name = static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "", false));
            descriptions.push_back(name + "||" + description);
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }

    if (sortOption == 3 || sortOption == 4) {
        std::sort(descriptions.begin(), descriptions.end());
        if (sortOption == 4) {
            std::reverse(descriptions.begin(), descriptions.end());
        }
    }
    return descriptions;
}

std::vector<std::string> GetFormNamesAsStrings(std::vector<RE::TESForm*> akForms, int sortOption, int noneStringType, std::string nullFormString) {
    std::vector<std::string> formNames;

    if (noneStringType == 2) {
        for (auto* akForm : akForms) {
            std::string noName = "";
            if (gfuncs::IsFormValid(akForm)) {
                noName = gfuncs::IntToHex(akForm->GetFormID());
            }
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, noName)));
        }
    }
    else if (noneStringType == 1) {
        for (auto* akForm : akForms) {
            std::string noName = "";
            if (gfuncs::IsFormValid(akForm)) {
                noName = GetFormEditorId(nullptr, akForm, "");
            }
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, noName)));
        }
    }
    else {
        for (auto* akForm : akForms) {
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(akForm, nullFormString, "")));
        }
    }

    if (sortOption == 1 || sortOption == 2) {
        std::sort(formNames.begin(), formNames.end());
        if (sortOption == 2) {
            std::reverse(formNames.begin(), formNames.end());
        }
    }
    return formNames;
}

std::vector<std::string> GetFormNamesAsStrings(RE::BGSListForm* akFormlist, int sortOption, int noneStringType, std::string nullFormString) {
    std::vector<std::string> formNames;

    int m = akFormlist->forms.size();

    if (noneStringType == 2) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string noName = "";
            if (gfuncs::IsFormValid(form)) {
                noName = gfuncs::IntToHex(form->GetFormID());
            }
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, noName)));
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else if (noneStringType == 1) {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            std::string noName = "";
            if (gfuncs::IsFormValid(form)) {
                noName = GetFormEditorId(nullptr, form, "");
            }
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, noName)));
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }
    else {
        akFormlist->ForEachForm([&](auto& akForm) {
            auto* form = &akForm;
            formNames.push_back(static_cast<std::string>(gfuncs::GetFormName(form, nullFormString, "")));
            return RE::BSContainer::ForEachResult::kContinue;
            });
    }

    if (sortOption == 1 || sortOption == 2) {
        std::sort(formNames.begin(), formNames.end());
        if (sortOption == 2) {
            std::reverse(formNames.begin(), formNames.end());
        }
    }
    return formNames;
}

std::vector<std::string> GetFormEditorIdsAsStrings(std::vector<RE::TESForm*> akForms, int sortOption, std::string nullFormString) {
    std::vector<std::string> formNames;

    int m = akForms.size();

    for (int i = 0; i < m; i++) {
        RE::TESForm* akForm = akForms[i];
        std::string name = nullFormString;
        if (gfuncs::IsFormValid(akForm)) {
            name = GetFormEditorId(nullptr, akForm, "");
        }
        formNames.push_back(name);
    }

    if (sortOption == 1 || sortOption == 2) {
        std::sort(formNames.begin(), formNames.end());
        if (sortOption == 2) {
            std::reverse(formNames.begin(), formNames.end());
        }
    }
    return formNames;
}

std::vector<std::string> GetFormEditorIdsAsStrings(RE::BGSListForm* akFormlist, int sortOption, std::string nullFormString) {
    std::vector<std::string> formNames;

    //int m = akFormlist->forms.size();

    akFormlist->ForEachForm([&](auto& akForm) {
        auto* form = &akForm;
        std::string name = nullFormString;
        if (gfuncs::IsFormValid(form)) {
            name = GetFormEditorId(nullptr, form, "");
        }
        formNames.push_back(name);
        return RE::BSContainer::ForEachResult::kContinue;
        });

    if (sortOption == 1 || sortOption == 2) {
        std::sort(formNames.begin(), formNames.end());
        if (sortOption == 2) {
            std::reverse(formNames.begin(), formNames.end());
        }
    }
    return formNames;
}


std::vector<std::string> GetLoadedModNamesAsStrings(int sortOption) {
    std::vector<std::string> fileNames;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedModCount();
        for (int i = 0; i < modCount; i++) {
            auto* file = dataHandler->LookupLoadedModByIndex(i);
            if (file) {
                fileNames.push_back(static_cast<std::string>(file->GetFilename()));
            }
        }

        if (sortOption == 1 || sortOption == 2) {
            std::sort(fileNames.begin(), fileNames.end());
            if (sortOption == 2) {
                std::reverse(fileNames.begin(), fileNames.end());
            }
        }
    }
    return fileNames;
}

std::vector<std::string> GetLoadedLightModNamesAsStrings(int sortOption) {
    std::vector<std::string> fileNames;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedLightModCount();
        for (int i = 0; i < modCount; i++) {
            auto* file = dataHandler->LookupLoadedLightModByIndex(i);
            if (file) {
                fileNames.push_back(static_cast<std::string>(file->GetFilename()));
            }
        }

        if (sortOption == 1 || sortOption == 2) {
            std::sort(fileNames.begin(), fileNames.end());
            if (sortOption == 2) {
                std::reverse(fileNames.begin(), fileNames.end());
            }
        }
    }
    return fileNames;
}

std::vector<std::string> GetLoadedModDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<std::string> sfileDescriptions;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedModCount();

        if (newLineReplacer != "" && maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else if (maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else if (newLineReplacer != "") {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(description);
                }
            }
        }

        if (sortOption == 1 || sortOption == 2) {
            std::sort(sfileDescriptions.begin(), sfileDescriptions.end());
            if (sortOption == 2) {
                std::reverse(sfileDescriptions.begin(), sfileDescriptions.end());
            }
        }
    }
    return sfileDescriptions;
}

std::vector<std::string> GetLoadedModNamesAndDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<std::string> sfileDescriptions;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedModCount();

        if (newLineReplacer != "" && maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else if (maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else if (newLineReplacer != "") {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }

        if (sortOption == 3 || sortOption == 4) {
            std::sort(sfileDescriptions.begin(), sfileDescriptions.end());
            if (sortOption == 2) {
                std::reverse(sfileDescriptions.begin(), sfileDescriptions.end());
            }
        }
    }
    return sfileDescriptions;
}

std::vector<std::string> GetLoadedLightModDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<std::string> sfileDescriptions;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedLightModCount();

        if (newLineReplacer != "" && maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else if (maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else if (newLineReplacer != "") {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(description);
                }
            }
        }

        if (sortOption == 1 || sortOption == 2) {
            std::sort(sfileDescriptions.begin(), sfileDescriptions.end());
            if (sortOption == 2) {
                std::reverse(sfileDescriptions.begin(), sfileDescriptions.end());
            }
        }
    }
    return sfileDescriptions;
}

std::vector<std::string> GetLoadedLightModNamesAndDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<std::string> sfileDescriptions;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedLightModCount();

        if (newLineReplacer != "" && maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else if (maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else if (newLineReplacer != "") {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }

        if (sortOption == 1 || sortOption == 2) {
            std::sort(sfileDescriptions.begin(), sfileDescriptions.end());
            if (sortOption == 2) {
                std::reverse(sfileDescriptions.begin(), sfileDescriptions.end());
            }
        }
    }
    return sfileDescriptions;
}

std::vector<std::string> GetAllLoadedModDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<std::string> sfileDescriptions;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedModCount();
        int lightModCount = dataHandler->GetLoadedLightModCount();

        if (newLineReplacer != "" && maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else if (maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else if (newLineReplacer != "") {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(description);
                }
            }
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(description);
                }
            }
        }

        if (sortOption == 1 || sortOption == 2) {
            std::sort(sfileDescriptions.begin(), sfileDescriptions.end());
            if (sortOption == 2) {
                std::reverse(sfileDescriptions.begin(), sfileDescriptions.end());
            }
        }
    }
    return sfileDescriptions;
}

std::vector<std::string> GetAllLoadedModNamesAndDescriptionsAsStrings(int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<std::string> sfileDescriptions;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedModCount();
        int lightModCount = dataHandler->GetLoadedLightModCount();

        //sFileNamesAndDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);

        if (newLineReplacer != "" && maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);

                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else if (maxCharacters > 0) {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    if (description.size() > maxCharacters) {
                        description = (description.substr(0, maxCharacters) + overMaxCharacterSuffix);
                    }
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else if (newLineReplacer != "") {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    gfuncs::String_ReplaceAll(description, "\r", newLineReplacer);
                    gfuncs::String_ReplaceAll(description, "\n", newLineReplacer);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }

            for (int i = 0; i < lightModCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    std::string description = static_cast<std::string>(file->summary);
                    sfileDescriptions.push_back(static_cast<std::string>(file->GetFilename()) + "||" + description);
                }
            }
        }

        if (sortOption == 3 || sortOption == 4) {
            std::sort(sfileDescriptions.begin(), sfileDescriptions.end());
            if (sortOption == 2) {
                std::reverse(sfileDescriptions.begin(), sfileDescriptions.end());
            }
        }
    }
    return sfileDescriptions;
}

//serialization============================================================================================================================================================

RE::TESForm* LoadForm(SKSE::SerializationInterface* a_intfc) {
    RE::FormID formID;
    if (!a_intfc->ReadRecordData(formID)) {
        logger::error("{}: Failed to load formID!", __func__);
        return nullptr;
    }

    if (!a_intfc->ResolveFormID(formID, formID)) {
        logger::warn("{}: warning, failed to resolve formID[{:x}]", __func__, formID);
    }

    RE::TESForm* akForm = RE::TESForm::LookupByID(formID);

    if (!gfuncs::IsFormValid(akForm, false)) {
        logger::error("{} failed to load", __func__);
        return nullptr;
    }

    return akForm;
}

bool LoadFormIDVector(std::vector<RE::FormID>& arr, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    arr.clear();
    std::size_t size;

    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record: [{}] Failed to load size of arr!", __func__, record);
        return false;
    }

    logger::debug("{}: load arr size = {}", __func__, size);

    for (std::size_t i = 0; i < size; ++i) {
        RE::FormID formID;
        if (!a_intfc->ReadRecordData(formID)) {
            logger::error("{}: {}: Failed to load formID!", __func__, i);
            return false;
        }

        if (!a_intfc->ResolveFormID(formID, formID)) {
            logger::warn("{}: {}: failed to resolve formID[{:x}]", __func__, i, formID);
            continue;
        }

        arr.push_back(formID);
    }
    return true;
}

bool SaveFormIDVector(std::vector<RE::FormID>& arr, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    const std::size_t size = arr.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: record[{}] Failed to write size of arr!", __func__, record);
        return false;
    }

    for (std::size_t i = 0; i < size; i++) {
        RE::FormID& formID = arr[i];

        if (formID) {
            if (!a_intfc->WriteRecordData(formID)) {
                logger::error("{}: record[{}] Failed to write data for formID[{}]", __func__, record, formID);
                return false;
            }
        }
        else {
            RE::FormID noFormID = -1;
            if (!a_intfc->WriteRecordData(noFormID)) {
                logger::error("{}: record[{}] Failed to write data for noFormID", __func__, record);
                return false;
            }
        }
    }

    return true;
}

bool LoadHandlesVector(std::vector<RE::VMHandle>& arr, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    arr.clear();
    std::size_t size;

    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record: [{}] Failed to load size of arr!", __func__, record);
        return false;
    }

    logger::trace("{}: load arr size = {}", __func__, size);

    for (std::size_t i = 0; i < size; ++i) {
        RE::VMHandle handle;
        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: {}: Failed to load handle!", __func__, i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: {}: warning, failed to resolve handle[{}]", __func__, i, handle);
            continue;
        }

        arr.push_back(handle);
    }
    return true;
}

bool SaveHandlesVector(std::vector<RE::VMHandle>& arr, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    const std::size_t size = arr.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: record[{}] Failed to write size of arr!", __func__, record);
        return false;
    }

    for (std::size_t i = 0; i < size; i++) {
        RE::VMHandle& handle = arr[i];

        if (handle) {
            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("{}: record[{}] Failed to write data for handle[{}]", __func__, record, handle);
                return false;
            }
        }
        else {
            RE::VMHandle noHandle = -1;
            if (!a_intfc->WriteRecordData(noHandle)) {
                logger::error("{}: record[{}] Failed to write data for noHandle", __func__, record);
                return false;
            }
        }
    }

    return true;
}

bool LoadFormHandlesMap(std::map<RE::TESForm*, std::vector<RE::VMHandle>>& akMap, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    akMap.clear();

    std::size_t size;

    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record[{}] Failed to load size of akMap!", __func__, record);
        return false;
    }

    logger::debug("{}: load akMap size = {}", __func__, size);

    for (std::size_t i = 0; i < size; ++i) { //size = number of pairs in map 

        RE::FormID formID;

        if (!a_intfc->ReadRecordData(formID)) {
            logger::error("{}: {}: Failed to load formID!", __func__, i);
            return false;
        }

        bool formIdResolved = a_intfc->ResolveFormID(formID, formID);

        if (!formIdResolved) {
            logger::warn("{}: {}: Failed to resolve formID {:x}", __func__, i, formID);
            return false;
        }

        //logger::trace("{}: {}: formID[{:x}] loaded and resolved", __func__, i, formID);

        RE::TESForm* akForm;
        if (formIdResolved) {
            akForm = RE::TESForm::LookupByID<RE::TESForm>(formID);
            if (!gfuncs::IsFormValid(akForm, false)) {
                logger::error("{}: {}: error, failed to load akForm!", __func__, i);
                return false;
            }
            else {
                logger::debug("{}: {}: akForm[{}] loaded", __func__, i, formID);
            }
        }

        std::size_t handlesSize;

        if (!a_intfc->ReadRecordData(handlesSize)) {
            logger::error("{}: {}: Failed to load handlesSize!", __func__, i);
            return false;
        }

        logger::debug("{}: {}: handlesSize loaded. Size[{}]", __func__, i, handlesSize);

        std::vector<RE::VMHandle> handles;

        for (std::size_t ib = 0; ib < handlesSize; ib++) {
            RE::VMHandle handle;

            if (!a_intfc->ReadRecordData(handle)) {
                logger::error("{}: {}: Failed to load handle", __func__, ib);
                return false;
            }

            if (!a_intfc->ResolveHandle(handle, handle)) {
                logger::warn("{}: {}: Failed to resolve handle {}", __func__, ib, handle);
                return false;
            }
            else {
                handles.push_back(handle);
            }
        }

        if (handles.size() > 0 && akForm != nullptr) {
            akMap.insert(std::pair<RE::TESForm*, std::vector<RE::VMHandle>>(akForm, handles));
            logger::debug("{}: {}: record[{}] akForm[{}] formID[{:x}] loaded", __func__, i, record, gfuncs::GetFormName(akForm), formID);
        }
    }
    return true;
}

bool SaveFormHandlesMap(std::map<RE::TESForm*, std::vector<RE::VMHandle>>& akMap, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    const std::size_t  size = akMap.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: Failed to write size of akMap!", __func__);
        return false;
    }
    else {

        //for (std::pair<RE::TESForm*, std::vector<RE::VMHandle>> it : akMap) //old loop

        std::map<RE::TESForm*, std::vector<RE::VMHandle>>::iterator it;

        for (it = akMap.begin(); it != akMap.end(); it++)
        {
            RE::FormID formID = -1;
            if (it->first) {
                formID = it->first->GetFormID();
                logger::trace("{}: saving handles for ref[{}] formId[{:x}]", __func__, gfuncs::GetFormName(it->first), formID);
            }

            if (!a_intfc->WriteRecordData(formID)) {
                logger::error("{}: Failed to write formID[{:x}]", __func__, formID);
                return false;
            }

            logger::trace("{}: formID[{:x}] written successfully", __func__, formID);

            const std::size_t handlesSize = it->second.size();

            if (!a_intfc->WriteRecordData(handlesSize)) {
                logger::error("{} failed to write it.second handlesSize", __func__);
                return false;
            }

            for (const RE::VMHandle& handle : it->second) {
                if (handle) {
                    if (!a_intfc->WriteRecordData(handle)) {
                        logger::error("{}: Failed to write data for handle[{}]", __func__, handle);
                        return false;
                    }
                }
                else {
                    RE::VMHandle noHandle;
                    if (!a_intfc->WriteRecordData(noHandle)) {
                        logger::error("{}: Failed to write data for noHandle", __func__, handle);
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

//papyrus functions=============================================================================================================================
float GetThisVersion(/*RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, */ RE::StaticFunctionTag* functionTag) {
    return float(7.9);
}

std::string GetClipBoardText(RE::StaticFunctionTag*) {
    // Try opening the clipboard
    if (!OpenClipboard(nullptr)) {
        LogAndMessage("Couldn't open clipboard", error);
        return "";
    }

    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) {
        LogAndMessage("Clipboard data not found", error);
        CloseClipboard();
        return "";
    }

    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) {
        LogAndMessage("Couldn't GlobalLock Clipboard Data", error);
        CloseClipboard();
        return "";
    }

    std::string text(pszText);

    // Release the lock
    GlobalUnlock(hData);

    // Release the clipboard
    CloseClipboard();

    return text;
}

bool SetClipBoardText(RE::StaticFunctionTag*, std::string sText) {
    if (sText.length() == 0) {
        return false;
    }

    const char* output = sText.data();
    const size_t len = strlen(output) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(hMem), output, len);
    GlobalUnlock(hMem);
    // Try opening the clipboard
    if (!OpenClipboard(nullptr)) {
        logger::error("{} Couldn't open clipboard", __func__);
        return false;
    }

    EmptyClipboard();
    auto Handle = SetClipboardData(CF_TEXT, hMem);

    if (Handle == NULL) {
        logger::error("{} Couldn't set clipboard data", __func__);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
}

bool IsWhiteSpace(RE::StaticFunctionTag*, std::string s) {
    return isspace(int(s.at(0)));
}

int CountWhiteSpaces(RE::StaticFunctionTag*, std::string s) {
    int spaces = std::count_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
    return spaces;
}

bool ModHasFormType(RE::StaticFunctionTag*, std::string modName, int formType) {
    logger::debug("{} modName[{}] formType[{}]", __func__, modName, formType);

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {
        logger::error("{} couldn't get dataHandler", __func__);
        return false;
    }

    auto* modFile = dataHandler->LookupModByName(modName);
    if (!modFile) {
        logger::error("{} mod [{}] not loaded", __func__, modName);
        return false;
    }

    RE::BSTArray<RE::TESForm*>* formArray = &(dataHandler->GetFormArray(static_cast<RE::FormType>(formType)));
    RE::BSTArray<RE::TESForm*>::iterator itrEndType = formArray->end();

    for (RE::BSTArray<RE::TESForm*>::iterator it = formArray->begin(); it != itrEndType; it++) {
        RE::TESForm* akForm = *it;

        if (gfuncs::IsFormValid(akForm)) {
            if (modFile->IsFormInMod(akForm->GetFormID())) {
                return true;
            }
        }
    }
    return false;
}

std::string GetFormEditorId(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string nullFormString) {
    if (!gfuncs::IsFormValid(akForm)) {
        return nullFormString;
    }
    else {
        std::string editorId = akForm->GetFormEditorID();
        if (editorId == "") {
            editorId = clib_util::editorID::get_editorID(akForm);
        }
        return editorId;
    }
}

std::vector<RE::BSFixedString> GetFormEditorIds(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, int sortOption, std::string nullFormString) {
    std::vector<RE::BSFixedString> formNames;
    if (akForms.size() == 0) {
        logger::warn("{} akForms is size 0", __func__);
        return formNames;
    }

    std::vector<std::string> sFormNames = GetFormEditorIdsAsStrings(akForms, sortOption, nullFormString);
    std::copy(sFormNames.begin(), sFormNames.end(), std::back_inserter(formNames));

    return formNames;
}

std::vector<RE::BSFixedString> GetFormEditorIdsFromList(RE::StaticFunctionTag*, RE::BGSListForm* akFormlist, int sortOption, std::string nullFormString) {
    std::vector<RE::BSFixedString> formNames;
    if (!akFormlist) {
        logger::warn("{} akFormlist doesn't exist", __func__);
        return formNames;
    }

    std::vector<std::string> sFormNames = GetFormEditorIdsAsStrings(akFormlist, sortOption, nullFormString);
    std::copy(sFormNames.begin(), sFormNames.end(), std::back_inserter(formNames));

    return formNames;
}

std::vector<RE::TESForm*> SortFormArray(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, int sortOption) {
    int numOfForms = akForms.size();
    if (numOfForms == 0) {
        return akForms;
    }

    std::vector<RE::TESForm*> returnForms;

    if (sortOption == 1 || sortOption == 2) { //sort by form name
        std::vector<std::string> formNames;
        std::map<std::string, std::vector<RE::TESForm*>> formNamesMap;

        for (int i = 0; i < numOfForms; i++) {
            auto* akForm = akForms[i];
            std::string formName = static_cast<std::string>(gfuncs::GetFormName(akForm, "", "", false));
            formNames.push_back(formName);
            auto it = formNamesMap.find(formName);
            if (it == formNamesMap.end()) {
                std::vector<RE::TESForm*> formNameForms;
                formNameForms.push_back(akForm);
                formNamesMap[formName] = formNameForms;
            }
            else {
                auto& formNameForms = it->second;
                formNameForms.push_back(akForm);
            }
        }
        std::sort(formNames.begin(), formNames.end());
        if (sortOption == 2) {
            std::reverse(formNames.begin(), formNames.end());
        }
        formNames.erase(std::unique(formNames.begin(), formNames.end()), formNames.end());

        int m = formNames.size();
        for (int i = 0; i < m; i++) {
            auto it = formNamesMap.find(formNames[i]);
            if (it != formNamesMap.end()) {
                auto& v = it->second;
                for (auto* akForm : v) {
                    returnForms.push_back(akForm);
                }
            }
        }
    }
    else if (sortOption == 3 || sortOption == 4) { //sort by editorId
        std::vector<std::string> formNames;
        std::map<std::string, std::vector<RE::TESForm*>> formNamesMap;

        for (int i = 0; i < numOfForms; i++) {
            auto* akForm = akForms[i];
            std::string formName = "";
            if (gfuncs::IsFormValid(akForm)) {
                formName = GetFormEditorId(nullptr, akForm, "");
            }

            formNames.push_back(formName);
            auto it = formNamesMap.find(formName);
            if (it == formNamesMap.end()) {
                std::vector<RE::TESForm*> formNameForms;
                formNameForms.push_back(akForm);
                formNamesMap[formName] = formNameForms;
            }
            else {
                auto& formNameForms = it->second;
                formNameForms.push_back(akForm);
            }
        }
        std::sort(formNames.begin(), formNames.end());
        if (sortOption == 4) {
            std::reverse(formNames.begin(), formNames.end());
        }
        formNames.erase(std::unique(formNames.begin(), formNames.end()), formNames.end());

        int m = formNames.size();
        for (int i = 0; i < m; i++) {
            auto it = formNamesMap.find(formNames[i]);
            if (it != formNamesMap.end()) {
                auto& v = it->second;
                for (auto* akForm : v) {
                    returnForms.push_back(akForm);
                }
            }
        }
    }
    else { //sort by form id
        std::vector<int> formIds;
        std::map<int, std::vector<RE::TESForm*>> formIdsMap;

        for (int i = 0; i < numOfForms; i++) {
            auto* akForm = akForms[i];
            int formID = akForm->GetFormID();
            formIds.push_back(formID);
            auto it = formIdsMap.find(formID);
            if (it == formIdsMap.end()) {
                std::vector<RE::TESForm*> formIdForms;
                formIdForms.push_back(akForm);
                formIdsMap[formID] = formIdForms;
            }
            else {
                auto& formIdForms = it->second;
                formIdForms.push_back(akForm);
            }
        }
        std::sort(formIds.begin(), formIds.end());
        if (sortOption == 6) {
            std::reverse(formIds.begin(), formIds.end());
        }

        formIds.erase(std::unique(formIds.begin(), formIds.end()), formIds.end());

        int m = formIds.size();
        for (int i = 0; i < m; i++) {
            auto it = formIdsMap.find(formIds[i]);
            if (it != formIdsMap.end()) {
                auto& v = it->second;
                for (auto* akForm : v) {
                    returnForms.push_back(akForm);
                }
            }
        }
    }

    return returnForms;
}

std::vector<RE::TESForm*> FormListToArray(RE::StaticFunctionTag*, RE::BGSListForm* akFormlist, int sortOption) {
    std::vector<RE::TESForm*> returnForms;
    if (!akFormlist) {
        logger::warn("{} akFormlist doesn't exist", __func__);
        return returnForms;
    }

    akFormlist->ForEachForm([&](auto& akForm) {
        auto* form = &akForm;
        returnForms.push_back(form);
        return RE::BSContainer::ForEachResult::kContinue;
        });

    if (sortOption >= 1 && sortOption <= 6) {
        return SortFormArray(nullptr, returnForms, sortOption);
    }
    else {
        return returnForms;
    }
}

void AddFormsToList(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, RE::BGSListForm* akFormlist) {
    if (!akFormlist) {
        logger::warn("{} akFormlist doesn't exist", __func__);
        return;
    }

    int m = akForms.size();

    if (m == 0) {
        logger::warn("{} akForms size is 0", __func__);
        return;
    }

    for (auto* akForm : akForms) {
        if (gfuncs::IsFormValid(akForm, false)) {
            akFormlist->AddForm(akForm);
        }
    }
}

std::string GetEffectsDescriptions(RE::BSTArray<RE::Effect*> effects) {
    std::string description = "";

    int m = effects.size();
    for (int i = 0; i < m; i++) {
        RE::Effect* effect = effects[i];
        if (effect) {
            std::string s = (static_cast<std::string>(effect->baseEffect->magicItemDescription));
            std::vector<std::string> values = {
                std::format("{:.0f}", effect->GetMagnitude()),
                std::format("{:.0f}", float(effect->GetDuration())),
                std::format("{:.0f}", float(effect->GetArea()))
            };
            gfuncs::String_ReplaceAll(s, magicDescriptionTags, values);
            description += (s + " ");
        }
    }
    return description;
}

std::string GetMagicItemDescription(RE::MagicItem* magicItem) {
    if (gfuncs::IsFormValid(magicItem)) {
        logger::debug("{} {}", __func__, magicItem->GetName());
        return GetEffectsDescriptions(magicItem->effects);
    }
    return "";
}

std::string GetDescription(RE::TESForm* akForm, std::string newLineReplacer) {
    if (!gfuncs::IsFormValid(akForm)) {
        logger::warn("{}: akForm doesn't exist or isn't valid", __func__);
        return "";
    }

    RE::BSString descriptionString;
    std::string s = "";
    auto description = akForm->As<RE::TESDescription>();

    if (description == NULL) {
        logger::warn("{} couldn't cast form[{}] ID[{}] as description", __func__, gfuncs::GetFormName(akForm), akForm->GetFormID());
        // return "";
    }
    else {
        description->GetDescription(descriptionString, nullptr);
        s = static_cast<std::string>(descriptionString);
    }

    if (s == "") {
        RE::MagicItem* magicItem = akForm->As<RE::MagicItem>();
        s = GetMagicItemDescription(magicItem);
    }

    if (s == "") {
        RE::TESEnchantableForm* enchantForm = akForm->As<RE::TESEnchantableForm>();
        if (enchantForm) {
            RE::EnchantmentItem* enchantment = enchantForm->formEnchanting;
            if (gfuncs::IsFormValid(enchantment)) {
                RE::MagicItem* magicItem = enchantment->As<RE::MagicItem>();
                if (gfuncs::IsFormValid(magicItem)) {
                    s = GetMagicItemDescription(magicItem);
                }
            }
        }
    }

    if (newLineReplacer != "") {
        gfuncs::String_ReplaceAll(s, "\r", newLineReplacer);
        gfuncs::String_ReplaceAll(s, "\n", newLineReplacer);
    }
    return s;
}

std::string GetFormDescription(RE::StaticFunctionTag*, RE::TESForm* akForm, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    if (!gfuncs::IsFormValid(akForm)) {
        return nullFormString;
    }

    std::string s = GetDescription(akForm, newLineReplacer);

    if (s == "") {
        if (noneStringType == 2) {
            if (gfuncs::IsFormValid(akForm)) {
                s = gfuncs::IntToHex(akForm->GetFormID());
            }
        }
        else if (noneStringType == 1) {
            if (gfuncs::IsFormValid(akForm)) {
                s = GetFormEditorId(nullptr, akForm, "");
            }
        }
    }
    else if (maxCharacters > 0) {
        if (s.size() > maxCharacters) {
            s = s.substr(0, maxCharacters) + overMaxCharacterSuffix;
        }
    }
    return s;
}

std::vector<RE::BSFixedString> GetFormDescriptions(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    std::vector<RE::BSFixedString> descriptions;
    if (akForms.size() == 0) {
        logger::warn("{} akForms is size 0", __func__);
        return descriptions;
    }

    if (sortOption >= 3 && sortOption <= 8) {
        sortOption -= 2; // 1 & 2 sorts by description, else sort by name, editorID, or formID, which are between 1 and 6 in SortFormArray function.
        std::vector<RE::TESForm*> sortedForms = SortFormArray(nullptr, akForms, sortOption);
        std::vector<std::string> sDescriptions = GetFormDescriptionsAsStrings(sortedForms, 0, maxCharacters, overMaxCharacterSuffix, newLineReplacer, noneStringType, nullFormString);
        std::copy(sDescriptions.begin(), sDescriptions.end(), std::back_inserter(descriptions));
    }
    else {
        std::vector<std::string> sDescriptions = GetFormDescriptionsAsStrings(akForms, sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer, noneStringType, nullFormString);
        std::copy(sDescriptions.begin(), sDescriptions.end(), std::back_inserter(descriptions));
    }

    return descriptions;
}

std::vector<RE::BSFixedString> GetFormDescriptionsFromList(RE::StaticFunctionTag*, RE::BGSListForm* akFormlist, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer, int noneStringType, std::string nullFormString) {
    std::vector<RE::BSFixedString> descriptions;
    if (!akFormlist) {
        logger::warn("{} akFormlist doesn't exist", __func__);
        return descriptions;
    }

    if (sortOption >= 3 && sortOption <= 8) {
        sortOption -= 2; // 1 & 2 sorts by description, else sort by name, editorID, or formID, which are between 1 and 6 in SortFormArray function.
        std::vector<RE::TESForm*> sortedForms = FormListToArray(nullptr, akFormlist, sortOption);
        std::vector<std::string> sDescriptions = GetFormDescriptionsAsStrings(sortedForms, 0, maxCharacters, overMaxCharacterSuffix, newLineReplacer, noneStringType, nullFormString);
        std::copy(sDescriptions.begin(), sDescriptions.end(), std::back_inserter(descriptions));
    }
    else {
        std::vector<std::string> sDescriptions = GetFormDescriptionsAsStrings(akFormlist, sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer, noneStringType, nullFormString);
        std::copy(sDescriptions.begin(), sDescriptions.end(), std::back_inserter(descriptions));
    }

    return descriptions;
}

std::vector<RE::BSFixedString> GetFormNames(RE::StaticFunctionTag*, std::vector<RE::TESForm*> akForms, int sortOption, int noneStringType, std::string nullFormString) {
    std::vector<RE::BSFixedString> formNames;
    if (akForms.size() == 0) {
        logger::warn("{} akForms is size 0", __func__);
        return formNames;
    }

    std::vector<std::string> sFormNames = GetFormNamesAsStrings(akForms, sortOption, noneStringType, nullFormString);
    std::copy(sFormNames.begin(), sFormNames.end(), std::back_inserter(formNames));

    return formNames;
}

std::vector<RE::BSFixedString> GetFormNamesFromList(RE::StaticFunctionTag*, RE::BGSListForm* akFormlist, int sortOption, int noneStringType, std::string nullFormString) {
    std::vector<RE::BSFixedString> formNames;
    if (!akFormlist) {
        logger::warn("{} akFormlist doesn't exist", __func__);
        return formNames;
    }

    std::vector<std::string> sFormNames = GetFormNamesAsStrings(akFormlist, sortOption, noneStringType, nullFormString);
    std::copy(sFormNames.begin(), sFormNames.end(), std::back_inserter(formNames));

    return formNames;
}

std::vector<RE::BSFixedString> GetLoadedModNames(RE::StaticFunctionTag*, int sortOption) {
    std::vector<RE::BSFixedString> fileNames;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedModCount();

        if (sortOption == 1 || sortOption == 2) {
            std::vector<std::string> sfileNames = GetLoadedModNamesAsStrings(sortOption);
            std::copy(sfileNames.begin(), sfileNames.end(), std::back_inserter(fileNames));
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedModByIndex(i);
                if (file) {
                    fileNames.push_back(file->GetFilename());
                }
            }
        }
    }
    return fileNames;
}

std::vector<RE::BSFixedString> GetLoadedLightModNames(RE::StaticFunctionTag*, int sortOption) {
    std::vector<RE::BSFixedString> fileNames;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        int modCount = dataHandler->GetLoadedLightModCount();

        if (sortOption == 1 || sortOption == 2) {
            std::vector<std::string> sfileNames = GetLoadedLightModNamesAsStrings(sortOption);
            std::copy(sfileNames.begin(), sfileNames.end(), std::back_inserter(fileNames));
        }
        else {
            for (int i = 0; i < modCount; i++) {
                auto* file = dataHandler->LookupLoadedLightModByIndex(i);
                if (file) {
                    fileNames.push_back(file->GetFilename());
                }
            }
        }
    }
    return fileNames;
}

std::vector<RE::BSFixedString> GetAllLoadedModNames(RE::StaticFunctionTag*, int sortOption) {
    std::vector<RE::BSFixedString> fileNames;

    if (sortOption == 1 || sortOption == 2) {
        std::vector<std::string> sfileNames = GetLoadedModNamesAsStrings(0);
        std::vector<std::string> slightfileNames = GetLoadedLightModNamesAsStrings(0);
        sfileNames.reserve(sfileNames.size() + slightfileNames.size());
        sfileNames.insert(sfileNames.end(), slightfileNames.begin(), slightfileNames.end());
        std::sort(sfileNames.begin(), sfileNames.end());
        if (sortOption == 2) {
            std::reverse(sfileNames.begin(), sfileNames.end());
        }
        std::copy(sfileNames.begin(), sfileNames.end(), std::back_inserter(fileNames));
    }
    else {
        fileNames = GetLoadedModNames(nullptr, sortOption);
        std::vector<RE::BSFixedString> lightFileNames = GetLoadedLightModNames(nullptr, sortOption);
        fileNames.reserve(fileNames.size() + lightFileNames.size());
        fileNames.insert(fileNames.end(), lightFileNames.begin(), lightFileNames.end());
    }

    return fileNames;
}

std::vector<RE::BSFixedString> GetLoadedModDescriptions(RE::StaticFunctionTag*, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<RE::BSFixedString> fileDescriptions;

    if (sortOption == 3 || sortOption == 4) {
        std::vector<std::string> sfileNamesAndDescriptions = GetLoadedModNamesAndDescriptionsAsStrings(sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer);
        int m = sfileNamesAndDescriptions.size();
        for (int i = 0; i < m; i++) {
            std::size_t delimIndex = sfileNamesAndDescriptions[i].find("||");
            if (delimIndex != std::string::npos) {
                delimIndex += 2;
                fileDescriptions.push_back(sfileNamesAndDescriptions[i].substr(delimIndex));
            }
        }
    }
    else {
        std::vector<std::string> sfileDescriptions = GetLoadedModDescriptionsAsStrings(sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer);
        std::copy(sfileDescriptions.begin(), sfileDescriptions.end(), std::back_inserter(fileDescriptions));
    }
    return fileDescriptions;
}

std::vector<RE::BSFixedString> GetLoadedLightModDescriptions(RE::StaticFunctionTag*, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<RE::BSFixedString> fileDescriptions;

    if (sortOption == 3 || sortOption == 4) {
        std::vector<std::string> sfileNamesAndDescriptions = GetLoadedLightModNamesAndDescriptionsAsStrings(sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer);
        int m = sfileNamesAndDescriptions.size();
        for (int i = 0; i < m; i++) {
            std::size_t delimIndex = sfileNamesAndDescriptions[i].find("||");
            if (delimIndex != std::string::npos) {
                delimIndex += 2;
                fileDescriptions.push_back(sfileNamesAndDescriptions[i].substr(delimIndex));
            }
        }
    }
    else {
        std::vector<std::string> sfileDescriptions = GetLoadedLightModDescriptionsAsStrings(sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer);
        std::copy(sfileDescriptions.begin(), sfileDescriptions.end(), std::back_inserter(fileDescriptions));
    }
    return fileDescriptions;
}

std::vector<RE::BSFixedString> GetAllLoadedModDescriptions(RE::StaticFunctionTag*, int sortOption, int maxCharacters, std::string overMaxCharacterSuffix, std::string newLineReplacer) {
    std::vector<RE::BSFixedString> fileDescriptions;

    if (sortOption == 3 || sortOption == 4) {
        std::vector<std::string> sfileNamesAndDescriptions = GetAllLoadedModNamesAndDescriptionsAsStrings(sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer);
        int m = sfileNamesAndDescriptions.size();
        for (int i = 0; i < m; i++) {
            auto delimIndex = sfileNamesAndDescriptions[i].find("||");
            if (delimIndex != std::string::npos) {
                delimIndex += 2;
                fileDescriptions.push_back(sfileNamesAndDescriptions[i].substr(delimIndex));
            }
        }
    }
    else {
        std::vector<std::string> sfileDescriptions = GetAllLoadedModDescriptionsAsStrings(sortOption, maxCharacters, overMaxCharacterSuffix, newLineReplacer);
        std::copy(sfileDescriptions.begin(), sfileDescriptions.end(), std::back_inserter(fileDescriptions));
    }
    return fileDescriptions;
}

std::vector<RE::TESQuest*> GetAllActiveQuests(RE::StaticFunctionTag*) {
    logger::debug("{} called", __func__);

    std::vector<RE::TESQuest*> questItems;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Quest));
        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();

        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;
            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                if (gfuncs::IsFormValid(quest)) {
                    if (quest->IsActive()) {
                        questItems.push_back(quest);
                    }
                }
            }
        }
    }
    return questItems;
}

bool FormNameMatches(RE::TESForm* akForm, std::string& sFormName) {
    bool nameMatches = false;
    if (gfuncs::IsFormValid(akForm)) {
        auto* ref = akForm->AsReference();
        if (gfuncs::IsFormValid(ref)) {
            nameMatches = (ref->GetDisplayFullName() == sFormName);
            if (!nameMatches) {
                RE::TESForm* baseForm = ref->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    nameMatches = (baseForm->GetName() == sFormName);
                }
            }
        }
        else {
            nameMatches = (akForm->GetName() == sFormName);
        }
    }
    return nameMatches;
}

bool FormNameContains(RE::TESForm* akForm, std::string& sFormName) {
    std::string akFormName = "";
    bool akFormNameContainsSFormName = false;
    if (gfuncs::IsFormValid(akForm)) {
        auto* ref = akForm->AsReference();
        if (gfuncs::IsFormValid(ref)) {
            akFormName = (ref->GetDisplayFullName());
            if (akFormName != "") {
                akFormNameContainsSFormName = (akFormName.find(sFormName) != std::string::npos);
            }
            if (!akFormNameContainsSFormName) {
                RE::TESForm* baseForm = ref->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    akFormName = baseForm->GetName();
                    akFormNameContainsSFormName = (akFormName.find(sFormName) != std::string::npos);
                }
            }
        }
        else {
            akFormName = akForm->GetName();
            akFormNameContainsSFormName = (akFormName.find(sFormName) != std::string::npos);
        }
    }
    return akFormNameContainsSFormName;
}

std::vector<RE::TESForm*> GetAllConstructibleObjects(RE::StaticFunctionTag*, RE::TESForm* createdObject) {
    std::vector<RE::TESForm*> forms;
    const auto& [allForms, lock] = RE::TESForm::GetAllForms();

    if (gfuncs::IsFormValid(createdObject, false)) {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false)) {
                RE::BGSConstructibleObject* object = form->As<RE::BGSConstructibleObject>();
                if (gfuncs::IsFormValid(object, false)) {
                    if (object->createdItem == createdObject) {
                        forms.push_back(form);
                    }
                }
            }
        }
    }
    else {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false)) {
                RE::BGSConstructibleObject* object = form->As<RE::BGSConstructibleObject>();
                if (gfuncs::IsFormValid(object, false)) {
                    forms.push_back(form);
                }
            }
        }
    }
    return forms;
}

std::vector<RE::TESObjectARMO*> GetAllArmorsForSlotMask(RE::StaticFunctionTag*, int slotMask) {
    std::vector<RE::TESObjectARMO*> armors;

    const auto& [allForms, lock] = RE::TESForm::GetAllForms();

    for (auto& [id, form] : *allForms) {
        if (gfuncs::IsFormValid(form, false)) {
            RE::TESObjectARMO* armor = form->As<RE::TESObjectARMO>();
            if (gfuncs::IsFormValid(armor)) {
                int mask = static_cast<int>(armor->GetSlotMask());
                if (mask == slotMask) {
                    armors.push_back(armor);
                }
            }
        }
    }

    return armors;
}

RE::TESWorldSpace* GetCellWorldSpace(RE::StaticFunctionTag*, RE::TESObjectCELL* akCell) {
    //logger::trace("{}: called", __func__);
    if (gfuncs::IsFormValid(akCell)) {
        RE::TESWorldSpace* worldSpace = akCell->GetRuntimeData().worldSpace;
        if (gfuncs::IsFormValid(worldSpace)) {
            return worldSpace;
        }
    }

    return nullptr;
}

RE::BGSLocation* GetCellLocation(RE::StaticFunctionTag*, RE::TESObjectCELL* akCell) {
    //logger::trace("{}: called", __func__);
    if (gfuncs::IsFormValid(akCell)) {
        RE::BGSLocation* location = akCell->GetLocation();
        if (gfuncs::IsFormValid(location)) {
            return location;
        }
    }

    return nullptr;
}

//TESNPC is ActorBase in papyrus
std::vector<RE::TESObjectCELL*> GetAllInteriorCells(RE::StaticFunctionTag*, RE::BGSLocation* akLocation, RE::TESNPC* akOwner, int matchMode) {
    //logger::trace("{}: called", __func__);
    std::vector<RE::TESObjectCELL*> cells;

    if (!gfuncs::IsFormValid(akLocation)) {
        akLocation = nullptr;
    }

    if (!gfuncs::IsFormValid(akOwner)) {
        akOwner = nullptr;
    }

    const auto& [allForms, lock] = RE::TESForm::GetAllForms();

    if (matchMode == 1) {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false)) {
                RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
                if (gfuncs::IsFormValid(cell)) {
                    if (cell->IsInteriorCell()) {
                        RE::BGSLocation* location = cell->GetLocation();
                        RE::TESNPC* npc = cell->GetActorOwner();

                        if (!gfuncs::IsFormValid(location)) {
                            location = nullptr;
                        }

                        if (!gfuncs::IsFormValid(npc)) {
                            npc = nullptr;
                        }

                        if (npc == akOwner && location == akLocation) {
                            cells.push_back(cell);
                        }
                    }
                }
            }
        }
    }
    else if (matchMode == 0) {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false)) {
                RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
                if (gfuncs::IsFormValid(cell)) {
                    if (cell->IsInteriorCell()) {
                        bool matched = false;
                        RE::BGSLocation* location = cell->GetLocation();
                        if (gfuncs::IsFormValid(location)) {
                            if (location == akLocation) {
                                matched = true;
                                cells.push_back(cell);
                            }
                        }
                        if (!matched) {
                            RE::TESNPC* npc = cell->GetActorOwner();
                            if (gfuncs::IsFormValid(npc)) {
                                if (npc == akOwner) {
                                    cells.push_back(cell);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false)) {
                RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
                if (gfuncs::IsFormValid(cell)) {
                    if (cell->IsInteriorCell()) {
                        cells.push_back(cell);
                    }
                }
            }
        }
    }

    //logger::trace("{}: cells size = {}", __func__, cells.size());
    return cells;
}

std::vector<RE::TESObjectCELL*> GetAllExteriorCells(RE::StaticFunctionTag*, RE::BGSLocation* akLocation, RE::TESWorldSpace* akWorldSpace, int matchMode) {
    std::vector<RE::TESObjectCELL*> cells;

    if (!gfuncs::IsFormValid(akLocation)) {
        akLocation = nullptr;
    }

    if (!gfuncs::IsFormValid(akWorldSpace)) {
        akWorldSpace = nullptr;
    }

    const auto& [allForms, lock] = RE::TESForm::GetAllForms();

    if (matchMode == 1) {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false)) {
                RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
                if (gfuncs::IsFormValid(cell)) {
                    if (cell->IsExteriorCell()) {
                        RE::BGSLocation* location = cell->GetLocation();
                        RE::TESWorldSpace* worldSpace = cell->GetRuntimeData().worldSpace;

                        if (!gfuncs::IsFormValid(location)) {
                            location = nullptr;
                        }

                        if (!gfuncs::IsFormValid(worldSpace)) {
                            worldSpace = nullptr;
                        }

                        if (worldSpace == akWorldSpace && location == akLocation) {
                            cells.push_back(cell);
                        }
                    }
                }
            }
        }
    }
    else if (matchMode == 0) {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false)) {
                RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
                if (gfuncs::IsFormValid(cell)) {
                    if (cell->IsExteriorCell()) {
                        bool matched = false;
                        RE::TESWorldSpace* worldSpace = cell->GetRuntimeData().worldSpace;
                        if (gfuncs::IsFormValid(worldSpace)) {
                            if (worldSpace == akWorldSpace) {
                                matched = true;
                                cells.push_back(cell);
                            }
                        }
                        if (!matched) {
                            RE::BGSLocation* location = cell->GetLocation();
                            if (gfuncs::IsFormValid(location)) {
                                if (location == akLocation) {
                                    cells.push_back(cell);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form, false)) {
                RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
                if (gfuncs::IsFormValid(cell)) {
                    if (cell->IsExteriorCell()) {
                        cells.push_back(cell);
                    }
                }
            }
        }
    }

    return cells;
}

std::vector<RE::TESObjectCELL*> GetAttachedCells(RE::StaticFunctionTag*) {
    std::vector<RE::TESObjectCELL*> cells;

    const auto& [allForms, lock] = RE::TESForm::GetAllForms();

    for (auto& [id, form] : *allForms) {
        if (gfuncs::IsFormValid(form, false)) {
            RE::TESObjectCELL* cell = form->As<RE::TESObjectCELL>();
            if (gfuncs::IsFormValid(cell)) {
                if (cell->IsAttached()) {
                    cells.push_back(cell);
                }
            }
        }
    }
    return cells;
}

std::vector<RE::TESForm*> GetFavorites(RE::StaticFunctionTag*, std::vector<int> formTypes, int formTypeMatchMode) {
    logger::trace("{} called", __func__);

    std::vector<RE::TESForm*> forms;

    auto inventory = gfuncs::playerRef->GetInventory();

    if (inventory.size() == 0) {
        return forms;
    }

    if (formTypes.size() > 0 && formTypeMatchMode == 1) {
        for (auto it = inventory.begin(); it != inventory.end(); it++) {
            auto invData = it->second.second.get();
            if (invData) {
                if (invData->IsFavorited()) {
                    if (gfuncs::IsFormValid(invData->object)) {
                        RE::TESForm* form = invData->object;
                        if (gfuncs::IsFormValid(form)) {
                            int formType = static_cast<int>(form->GetFormType());
                            for (int& i : formTypes) {
                                if (i == formType) {
                                    forms.push_back(form);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (formTypes.size() > 0 && formTypeMatchMode == 0) {
        for (auto it = inventory.begin(); it != inventory.end(); it++) {
            auto invData = it->second.second.get();
            if (invData) {
                if (invData->IsFavorited()) {
                    if (gfuncs::IsFormValid(invData->object)) {
                        RE::TESForm* form = invData->object;
                        if (gfuncs::IsFormValid(form)) {
                            int formType = static_cast<int>(form->GetFormType());
                            bool matchedType = false;
                            for (int& i : formTypes) {
                                if (i == formType) {
                                    matchedType = true;
                                    break;
                                }
                            }
                            if (!matchedType) {
                                forms.push_back(form);
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        for (auto it = inventory.begin(); it != inventory.end(); it++) {
            auto invData = it->second.second.get();
            if (invData) {
                if (invData->IsFavorited()) {
                    if (gfuncs::IsFormValid(invData->object)) {
                        RE::TESForm* akForm = invData->object;
                        if (gfuncs::IsFormValid(akForm)) {
                            forms.push_back(akForm);
                        }
                    }
                }
            }
        }
    }

    logger::trace("{} number of favorites = {}", __func__, forms.size());

    return forms;
}

std::vector<RE::TESForm*> GetAllFormsWithName(RE::StaticFunctionTag*, std::string sFormName, int nameMatchMode, std::vector<int> formTypes, int formTypeMatchMode) {
    std::vector<RE::TESForm*> forms;
    if (sFormName == "") {
        logger::warn("{} sFormName is empty", __func__);
        return forms;
    }

    if (nameMatchMode == 0) {
        if (formTypes.size() == 0 || formTypeMatchMode == -1) {
            const auto& [allForms, lock] = RE::TESForm::GetAllForms();
            for (auto& [id, form] : *allForms) {
                if (FormNameMatches(form, sFormName)) {
                    forms.push_back(form);
                }
            }
        }
        else if (formTypes.size() > 0 && formTypeMatchMode == 1) {
            const auto& [allForms, lock] = RE::TESForm::GetAllForms();
            for (auto& [id, form] : *allForms) {
                if (FormNameMatches(form, sFormName)) {
                    int formType = static_cast<int>(form->GetFormType());
                    for (int& i : formTypes) {
                        if (i == formType) {
                            forms.push_back(form);
                            break;
                        }
                    }
                }
            }
        }
        else if (formTypes.size() > 0 && formTypeMatchMode == 0) {
            const auto& [allForms, lock] = RE::TESForm::GetAllForms();
            for (auto& [id, form] : *allForms) {
                if (FormNameMatches(form, sFormName)) {
                    int formType = static_cast<int>(form->GetFormType());
                    bool matchedType = false;
                    for (int& i : formTypes) {
                        if (i == formType) {
                            matchedType = true;
                            break;
                        }
                    }
                    if (!matchedType) {
                        forms.push_back(form);
                    }
                }
            }
        }
    }
    else {
        if (formTypes.size() == 0 || formTypeMatchMode == -1) {
            const auto& [allForms, lock] = RE::TESForm::GetAllForms();
            for (auto& [id, form] : *allForms) {
                if (FormNameContains(form, sFormName)) {
                    forms.push_back(form);
                }
            }
        }
        else if (formTypes.size() > 0 && formTypeMatchMode == 1) {
            const auto& [allForms, lock] = RE::TESForm::GetAllForms();
            for (auto& [id, form] : *allForms) {
                if (FormNameContains(form, sFormName)) {
                    int formType = static_cast<int>(form->GetFormType());
                    for (int& i : formTypes) {
                        if (i == formType) {
                            forms.push_back(form);
                            break;
                        }
                    }
                }
            }
        }
        else if (formTypes.size() > 0 && formTypeMatchMode == 0) {
            const auto& [allForms, lock] = RE::TESForm::GetAllForms();
            for (auto& [id, form] : *allForms) {
                if (FormNameContains(form, sFormName)) {
                    int formType = static_cast<int>(form->GetFormType());
                    bool matchedType = false;
                    for (int& i : formTypes) {
                        if (i == formType) {
                            matchedType = true;
                            break;
                        }
                    }
                    if (!matchedType) {
                        forms.push_back(form);
                    }
                }
            }
        }
    }
    gfuncs::RemoveDuplicates(forms);
    return forms;
}

std::vector<RE::TESForm*> GetAllFormsWithScriptAttached(RE::StaticFunctionTag*, RE::BSFixedString sScriptName, std::vector<int> formTypes, int formTypeMatchMode) {
    std::vector<RE::TESForm*> forms;
    if (formTypes.size() == 0 || formTypeMatchMode == -1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (IsScriptAttachedToForm(form, sScriptName)) {
                forms.push_back(form);
            }
        }
    }
    else if (formTypes.size() > 0 && formTypeMatchMode == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (IsScriptAttachedToForm(form, sScriptName)) {
                int formType = static_cast<int>(form->GetFormType());
                for (int& i : formTypes) {
                    if (i == formType) {
                        forms.push_back(form);
                    }
                }
            }
        }
    }
    else if (formTypes.size() > 0 && formTypeMatchMode == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (IsScriptAttachedToForm(form, sScriptName)) {
                int formType = static_cast<int>(form->GetFormType());
                bool matchedType = false;
                for (int& i : formTypes) {
                    if (i == formType) {
                        matchedType = true;
                        break;
                    }
                }
                if (!matchedType) {
                    forms.push_back(form);
                }
            }
        }
    }
    gfuncs::RemoveDuplicates(forms);
    return forms;
}

std::vector<RE::BGSBaseAlias*> GetAllAliasesWithScriptAttached(RE::StaticFunctionTag*, RE::BSFixedString sScriptName) {
    logger::debug("{} called", __func__);

    std::vector<RE::BGSBaseAlias*> questItems;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Quest));
        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();

        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;
            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                if (gfuncs::IsFormValid(quest)) {
                    if (quest->aliases.size() > 0) {
                        for (int i = 0; i < quest->aliases.size(); i++) {
                            if (quest->aliases[i]) {
                                RE::VMHandle handle = gfuncs::GetHandle(quest->aliases[i]);
                                if (IsScriptAttachedToHandle(handle, sScriptName)) {
                                    questItems.push_back(quest->aliases[i]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return questItems;
}

std::vector<RE::BGSRefAlias*> GetAllRefAliasesWithScriptAttached(RE::StaticFunctionTag*, RE::BSFixedString sScriptName, bool onlyQuestObjects, bool onlyFilled) {
    logger::debug("{} called", __func__);

    std::vector<RE::BGSRefAlias*> questItems;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Quest));
        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();

        if (onlyQuestObjects && onlyFilled) {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    if (quest->aliases[i]->IsQuestObject()) {
                                        RE::VMHandle handle = gfuncs::GetHandle(quest->aliases[i]);
                                        if (IsScriptAttachedToHandle(handle, sScriptName)) {
                                            RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                            if (refAlias) {
                                                RE::TESObjectREFR* akRef = refAlias->GetReference();
                                                if (gfuncs::IsFormValid(akRef)) {
                                                    questItems.push_back(refAlias);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (onlyQuestObjects) {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    if (quest->aliases[i]->IsQuestObject()) {
                                        RE::VMHandle handle = gfuncs::GetHandle(quest->aliases[i]);
                                        if (IsScriptAttachedToHandle(handle, sScriptName)) {
                                            RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                            if (refAlias) {
                                                questItems.push_back(refAlias);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (onlyFilled) {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    RE::VMHandle handle = gfuncs::GetHandle(quest->aliases[i]);
                                    if (IsScriptAttachedToHandle(handle, sScriptName)) {
                                        RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                        if (refAlias) {
                                            RE::TESObjectREFR* akRef = refAlias->GetReference();
                                            if (gfuncs::IsFormValid(akRef)) {
                                                questItems.push_back(refAlias);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    RE::VMHandle handle = gfuncs::GetHandle(quest->aliases[i]);
                                    if (IsScriptAttachedToHandle(handle, sScriptName)) {
                                        RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                        if (refAlias) {
                                            questItems.push_back(refAlias);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return questItems;
}

std::vector<RE::BGSRefAlias*> GetAllRefaliases(RE::StaticFunctionTag*, bool onlyQuestObjects, bool onlyFilled) {
    logger::debug("{} called", __func__);

    std::vector<RE::BGSRefAlias*> questItems;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Quest));
        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();

        if (onlyQuestObjects && onlyFilled) {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    if (quest->aliases[i]->IsQuestObject()) {
                                        RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                        if (refAlias) {
                                            RE::TESObjectREFR* akRef = refAlias->GetReference();
                                            if (gfuncs::IsFormValid(akRef)) {
                                                questItems.push_back(refAlias);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (onlyQuestObjects) {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    if (quest->aliases[i]->IsQuestObject()) {
                                        RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                        if (refAlias) {
                                            questItems.push_back(refAlias);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (onlyFilled) {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                    if (refAlias) {
                                        RE::TESObjectREFR* akRef = refAlias->GetReference();
                                        if (gfuncs::IsFormValid(akRef)) {
                                            questItems.push_back(refAlias);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else {
            for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
                RE::TESForm* baseForm = *itr;
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                    if (gfuncs::IsFormValid(quest)) {
                        if (quest->aliases.size() > 0) {
                            for (int i = 0; i < quest->aliases.size(); i++) {
                                if (quest->aliases[i]) {
                                    RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                    if (refAlias) {
                                        questItems.push_back(refAlias);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return questItems;
}

std::vector<RE::TESObjectREFR*> GetAllQuestObjectRefs(RE::StaticFunctionTag*) {
    logger::debug("{} called", __func__);

    std::vector<RE::TESObjectREFR*> questItems;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Quest));
        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();
        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                if (gfuncs::IsFormValid(quest)) {
                    if (quest->aliases.size() > 0) {
                        for (int i = 0; i < quest->aliases.size(); i++) {
                            if (quest->aliases[i]) {
                                if (quest->aliases[i]->IsQuestObject()) {
                                    //quest->aliases[i].
                                    RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                    if (refAlias) {
                                        RE::TESObjectREFR* akRef = refAlias->GetReference();
                                        if (gfuncs::IsFormValid(akRef)) {
                                            questItems.push_back(akRef);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return questItems;
}

std::vector<RE::TESObjectREFR*> GetQuestObjectRefsInContainer(RE::StaticFunctionTag*, RE::TESObjectREFR* containerRef) {
    logger::trace("{} called", __func__);

    std::vector<RE::TESObjectREFR*> questItems;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    if (!gfuncs::IsFormValid(containerRef)) {
        logger::warn("{} containerRef doesn't exist", __func__);
        return questItems;
    }

    auto inventory = containerRef->GetInventory();

    if (inventory.size() == 0) {
        logger::warn("{} {} containerRef doesn't contain any items", __func__, gfuncs::GetFormName(containerRef, "", "", true));
        return questItems;
    }

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Quest));
        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();

        logger::debug("{} number of quests is {}", __func__, akArray->size());
        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESQuest* quest = baseForm->As<RE::TESQuest>();
                if (gfuncs::IsFormValid(quest)) {
                    if (quest->aliases.size() > 0) {
                        for (int i = 0; i < quest->aliases.size(); i++) {
                            if (quest->aliases[i]) {
                                if (quest->aliases[i]->IsQuestObject()) {
                                    RE::BGSRefAlias* refAlias = static_cast<RE::BGSRefAlias*>(quest->aliases[i]);
                                    if (refAlias) {
                                        RE::TESObjectREFR* akRef = refAlias->GetReference();
                                        if (gfuncs::IsFormValid(akRef)) {
                                            RE::TESBoundObject* boundObj = akRef->GetObjectReference();
                                            if (gfuncs::IsFormValid(boundObj)) {
                                                if (inventory.contains(boundObj)) {
                                                    questItems.push_back(akRef);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    logger::trace("{} number of refs is {}", __func__, questItems.size());
    return questItems;
}

//general projectile functions==========================================================================================================================

bool DidShooterHitRefWithProjectile(RE::TESObjectREFR* shooter, RE::TESObjectREFR* ref, TrackedProjectileData& data) {
    return (shooter == data.shooter && ref == data.target);
}

bool DidProjectileHitRef(RE::Projectile* akProjectile, RE::TESObjectREFR* ref) {
    if (gfuncs::IsFormValid(akProjectile) && gfuncs::IsFormValid(ref)) {

        //logger::trace("akProjectile[{}] found", akProjectile->GetDisplayFullName());
        auto impacts = akProjectile->GetProjectileRuntimeData().impacts;
        if (!impacts.empty()) {
            for (auto* impactData : impacts) {
                if (impactData) {
                    auto hitRef = impactData->collidee;
                    if (hitRef) {
                        //logger::trace("hitRef found");
                        auto hitRefPtr = hitRef.get();
                        if (hitRefPtr) {
                            //logger::trace("hitRefPtr found");
                            auto* hitRefRef = hitRefPtr.get();
                            if (gfuncs::IsFormValid(hitRefRef)) {
                                return (hitRefRef == ref);
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool DidRefShootProjectile(RE::Projectile* akProjectile, RE::TESObjectREFR* ref) {
    if (gfuncs::IsFormValid(akProjectile) && gfuncs::IsFormValid(ref)) {
        auto shooterHandle = akProjectile->GetProjectileRuntimeData().shooter;
        if (shooterHandle) {
            auto shooterRefPtr = shooterHandle.get();
            if (shooterRefPtr) {
                auto* shooter = shooterRefPtr.get();
                if (gfuncs::IsFormValid(shooter)) {
                    return (shooter == ref);
                }
            }
        }
    }
    return false;
}

bool DidProjectileHitRefWithAmmoFromShooter(RE::TESObjectREFR* shooter, RE::TESObjectREFR* ref, RE::TESAmmo* akAmmo, TrackedProjectileData& data) {
    return (shooter == data.shooter && ref == data.target && akAmmo == data.ammo);
}

bool DidProjectileHitRefWithAmmoFromShooter(RE::TESObjectREFR* shooter, RE::TESObjectREFR* ref, RE::Projectile* akProjectile, RE::TESAmmo* akAmmo) {
    if (gfuncs::IsFormValid(shooter) && gfuncs::IsFormValid(ref) && gfuncs::IsFormValid(akProjectile) && gfuncs::IsFormValid(akAmmo)) {
        auto& runtimeData = akProjectile->GetProjectileRuntimeData();

        if (runtimeData.ammoSource != akAmmo) {
            return false;
        }

        RE::TESObjectREFR* projectileShooter = nullptr;

        if (runtimeData.shooter) {
            auto projectileShooterPtr = runtimeData.shooter.get();
            if (projectileShooterPtr) {
                projectileShooter = projectileShooterPtr.get();
            }
        }

        if (gfuncs::IsFormValid(projectileShooter)) {
            if (projectileShooter != shooter) {
                return false;
            }
        }

        auto impacts = runtimeData.impacts;
        if (!impacts.empty()) {
            for (auto* impactData : impacts) {
                if (impactData) {
                    //logger::trace("impactData found");
                    auto hitRef = impactData->collidee;
                    if (hitRef) {
                        //logger::trace("hitRef found");
                        auto hitRefPtr = hitRef.get();
                        if (hitRefPtr) {
                            //logger::trace("hitRefPtr found");
                            auto* hitRefRef = hitRefPtr.get();
                            if (gfuncs::IsFormValid(hitRefRef)) {
                                return (hitRefRef == ref);
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

//projectile papyrus functions ==================================================================================================

int GetProjectileType(RE::BGSProjectile* projectile) {
    if (!gfuncs::IsFormValid(projectile)) {
        return 0;
    }

    if (projectile->IsMissile()) {
        return 1;
    }
    else if (projectile->IsGrenade()) {
        return 2;
    }
    else if (projectile->IsBeam()) {
        return 3;
    }
    else if (projectile->IsFlamethrower()) {
        return 4;
    }
    else if (projectile->IsCone()) {
        return 5;
    }
    else if (projectile->IsBarrier()) {
        return 6;
    }
    else if (projectile->IsArrow()) {
        return 7;
    }
    return 0;
}

RE::BGSTextureSet* GetProjectileBaseDecal(RE::StaticFunctionTag*, RE::BGSProjectile* projectile) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectile doesn't exist or isn't valid", __func__);
        return nullptr;
    }

    if (gfuncs::IsFormValid(projectile->data.decalData)) {
        return projectile->data.decalData;
    }
    return nullptr;
}

bool SetProjectileBaseDecal(RE::StaticFunctionTag*, RE::BGSProjectile* projectile, RE::BGSTextureSet* decalTextureSet) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectile doesn't exist or isn't valid", __func__);
        return false;
    }

    projectile->data.decalData = decalTextureSet;

    return (projectile->data.decalData == decalTextureSet);
}

RE::BGSExplosion* GetProjectileBaseExplosion(RE::StaticFunctionTag*, RE::BGSProjectile* projectile) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectile doesn't exist or isn't valid", __func__);
        return nullptr;
    }

    if (gfuncs::IsFormValid(projectile->data.explosionType)) {
        return projectile->data.explosionType;
    }
    return nullptr;
}

bool SetProjectileBaseExplosion(RE::StaticFunctionTag*, RE::BGSProjectile* projectile, RE::BGSExplosion* akExplosion) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectile doesn't exist or isn't valid", __func__);
        return false;
    }

    projectile->data.explosionType = akExplosion;

    return (projectile->data.explosionType == akExplosion);
}

float GetProjectileBaseCollisionRadius(RE::StaticFunctionTag*, RE::BGSProjectile* projectile) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectile doesn't exist or isn't valid", __func__);
        return 0.0;
    }

    return projectile->data.collisionRadius;
}

bool SetProjectileBaseCollisionRadius(RE::StaticFunctionTag*, RE::BGSProjectile* projectile, float radius) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectile doesn't exist or isn't valid", __func__);
        return false;
    }

    projectile->data.collisionRadius = radius;

    return (projectile->data.collisionRadius == radius);
}

float GetProjectileBaseCollisionConeSpread(RE::StaticFunctionTag*, RE::BGSProjectile* projectile) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectile doesn't exist or isn't valid", __func__);
        return 0.0;
    }

    return projectile->data.coneSpread;
}

bool SetProjectileBaseCollisionConeSpread(RE::StaticFunctionTag*, RE::BGSProjectile* projectile, float coneSpread) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectile doesn't exist or isn't valid", __func__);
        return false;
    }

    projectile->data.coneSpread = coneSpread;

    return (projectile->data.coneSpread == coneSpread);
}


int GetProjectileRefType(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        return 0;
    }

    auto* akProjectile = projectileRef->AsProjectile();

    if (!gfuncs::IsFormValid(akProjectile)) {
        return 0;
    }

    RE::BGSProjectile* projectileBase = akProjectile->GetProjectileBase();
    return GetProjectileType(projectileBase);
}

RE::TESObjectREFR* GetProjectileHitRef(RE::Projectile::ImpactData* impactData) {
    if (impactData) {
        if (impactData->collidee) {
            auto hitRefHandle = impactData->collidee;
            if (hitRefHandle) {
                auto hitRefPtr = hitRefHandle.get();
                if (hitRefPtr) {
                    return hitRefPtr.get();
                }
            }
        }
    }
    return nullptr;
}

std::vector<RE::TESObjectREFR*> GetProjectileHitRefs(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    std::vector<RE::TESObjectREFR*> hitRefs;
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return hitRefs;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectileRef[{}] is not a projectile", __func__, gfuncs::GetFormName(projectileRef));
        return hitRefs;
    }

    auto impacts = projectile->GetProjectileRuntimeData().impacts;
    if (!impacts.empty()) {
        for (auto* impactData : impacts) {
            auto* hitRef = GetProjectileHitRef(impactData);
            if (gfuncs::IsFormValid(hitRef)) {
                hitRefs.push_back(hitRef);
            }
        }
    }
    return hitRefs;
}

RE::TESObjectREFR* GetLastProjectileHitRefFromRuntimeData(RE::Projectile::PROJECTILE_RUNTIME_DATA& runtimeData) {
    RE::TESObjectREFR* hitRef;
    if (!runtimeData.impacts.empty()) {
        for (auto* impactData : runtimeData.impacts) {
            if (impactData) {
                RE::TESObjectREFR* akHitRef = GetProjectileHitRef(impactData);
                if (gfuncs::IsFormValid(akHitRef)) {
                    hitRef = akHitRef;
                }
            }
        }
    }
    return hitRef;
}

RE::TESObjectREFR* GetProjectileShooterFromRuntimeData(RE::Projectile::PROJECTILE_RUNTIME_DATA& runtimeData) {
    if (runtimeData.shooter) {
        auto refPtr = runtimeData.shooter.get();
        if (refPtr) {
            return refPtr.get();
        }
    }
    return nullptr;
}

RE::TESObjectREFR* GetProjectileShooter(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return nullptr;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectileRef[{}] is not a projectile", __func__, gfuncs::GetFormName(projectileRef));
        return nullptr;
    }

    return GetProjectileShooterFromRuntimeData(projectile->GetProjectileRuntimeData());
}

RE::BGSExplosion* GetProjectileExplosion(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return nullptr;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectileRef[{}] is not a projectile", __func__, gfuncs::GetFormName(projectileRef));
        return nullptr;
    }

    return projectile->GetProjectileRuntimeData().explosion;
}

RE::TESAmmo* GetProjectileAmmoSource(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return nullptr;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectileRef[{}] is not a projectile", __func__, gfuncs::GetFormName(projectileRef));
        return nullptr;
    }

    return projectile->GetProjectileRuntimeData().ammoSource;
}

RE::AlchemyItem* GetProjectilePoison(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    RE::AlchemyItem* poison;
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return nullptr;
    }

    auto* arrowProjectile = projectileRef->As<RE::ArrowProjectile>();
    if (gfuncs::IsFormValid(arrowProjectile)) {
        poison = arrowProjectile->GetArrowRuntimeData().poison;
    }
    return poison;
}

RE::EnchantmentItem* GetProjectileEnchantment(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    RE::EnchantmentItem* enchantment;

    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return nullptr;
    }

    auto* arrowProjectile = projectileRef->As<RE::ArrowProjectile>();
    if (gfuncs::IsFormValid(arrowProjectile)) {
        enchantment = arrowProjectile->GetArrowRuntimeData().enchantItem;
    }
    return enchantment;
}

RE::TESForm* GetProjectileMagicSource(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return nullptr;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectileRef[{}] is not a projectile", __func__, gfuncs::GetFormName(projectileRef));
        return nullptr;
    }

    return projectile->GetProjectileRuntimeData().spell;
}

RE::TESObjectWEAP* GetProjectileWeaponSource(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return nullptr;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectileRef[{}] is not a projectile", __func__, gfuncs::GetFormName(projectileRef));
        return nullptr;
    }

    return projectile->GetProjectileRuntimeData().weaponSource;
}

float GetProjectileWeaponDamage(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return 0.0;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectileRef[{}] is not a projectile", __func__, gfuncs::GetFormName(projectileRef));
        return 0.0;
    }

    return projectile->GetProjectileRuntimeData().weaponDamage;
}

float GetProjectilePower(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return 0.0;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectileRef[{}] is not a projectile", __func__, gfuncs::GetFormName(projectileRef));
        return 0.0;
    }

    return projectile->GetProjectileRuntimeData().power;
}

float GetProjectileDistanceTraveled(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return 0.0;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectileRef[{}] is not a projectile", __func__, gfuncs::GetFormName(projectileRef));
        return 0.0;
    }

    return projectile->GetProjectileRuntimeData().distanceMoved;
}

int GetProjectileImpactResult(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    int impactResult = 0;
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef isn't valid or doesn't exist", __func__);
        return impactResult;
    }

    auto* missileProjectile = projectileRef->As<RE::MissileProjectile>();

    if (gfuncs::IsFormValid(missileProjectile)) {
        impactResult = static_cast<int>(missileProjectile->GetMissileRuntimeData().impactResult);
    }

    if (impactResult == 0) {
        if (gfuncs::IsFormValid(projectileRef)) { //check this again to be sure
            auto* coneProjectile = projectileRef->As<RE::ConeProjectile>();
            if (gfuncs::IsFormValid(coneProjectile)) {
                impactResult = static_cast<int>(coneProjectile->GetConeRuntimeData().impactResult);
            }
        }
    }
    return impactResult;
}

std::vector<int> GetProjectileCollidedLayers(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    std::vector<int> layers;
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return layers;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectileRef[{}] is not a projectile", __func__, gfuncs::GetFormName(projectileRef));
        return layers;
    }

    auto impacts = projectile->GetProjectileRuntimeData().impacts;
    if (!impacts.empty()) {
        for (auto* impactData : impacts) {
            if (impactData) {
                int collidedLayer = impactData->collidedLayer.underlying();
                layers.push_back(collidedLayer);
            }
        }
    }
    return layers;
}

std::string GetCollisionLayerName(RE::StaticFunctionTag*, int layer) {
    switch (layer) {
    case 1:
        return "Static";
    case 2:
        return "AnimStatic";
    case 3:
        return "Transparent";
    case 4:
        return "Clutter";
    case 5:
        return "Weapon";
    case 6:
        return "Projectile";
    case 7:
        return "Spell";
    case 8:
        return "Biped";
    case 9:
        return "Trees";
    case 10:
        return "Props";
    case 11:
        return "Water";
    case 12:
        return "Trigger";
    case 13:
        return "Terrain";
    case 14:
        return "Trap";
    case 15:
        return "NonCollidable";
    case 16:
        return "CloudTrap";
    case 17:
        return "Ground";
    case 18:
        return "Portal";
    case 19:
        return "DebrisSmall";
    case 20:
        return "DebrisLarge";
    case 21:
        return "AcousticSpace";
    case 22:
        return "ActorZone";
    case 23:
        return "ProjectileZone";
    case 24:
        return "GasTrap";
    case 25:
        return "ShellCasting";
    case 26:
        return "TransparentWall";
    case 27:
        return "InvisibleWall";
    case 28:
        return "TransparentSmallAnim";
    case 29:
        return "ClutterLarge";
    case 30:
        return "CharController";
    case 31:
        return "StairHelper";
    case 32:
        return "DeadBip";
    case 33:
        return "BipedNoCC";
    case 34:
        return "AvoidBox";
    case 35:
        return "CollisionBox";
    case 36:
        return "CameraSphere";
    case 37:
        return "DoorDetection";
    case 38:
        return "ConeProjectile";
    case 39:
        return "Camera";
    case 40:
        return "ItemPicker";
    case 41:
        return "LOS";
    case 42:
        return "PathingPick";
    case 43:
        return "Unused0";
    case 44:
        return "Unused1";
    case 45:
        return "SpellExplosion";
    case 46:
        return "DroppingPick";
    default:
        return "Unidentified";
    }
}

std::vector<std::string> GetProjectileCollidedLayerNames(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    std::vector<std::string> layers;
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return layers;
    }

    std::vector<int> intLayers = GetProjectileCollidedLayers(nullptr, projectileRef);
    for (auto& layer : intLayers) {
        layers.push_back(GetCollisionLayerName(nullptr, layer));
    }

    return layers;
}

std::string GetProjectileNodeHitName(RE::Projectile::ImpactData* impactData) {
    RE::BSFixedString hitPartNodeName;
    if (impactData) {
        RE::NiNode* hitPart = impactData->damageRootNode;
        if (hitPart) {
            RE::BSFixedString hitPartNodeName = hitPart->name;
            if (hitPart->parent) {
                if (hitPart->parent->name == "SHIELD" || hitPartNodeName == "") {
                    hitPartNodeName = hitPart->parent->name;
                }
            }
        }
    }
    return hitPartNodeName.data();
}

std::vector<std::string> GetProjectileNodeHitNames(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    std::vector<std::string> nodeNames;
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("{}: projectileRef doesn't exist or isn't valid", __func__);
        return nodeNames;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("{}: projectileRef[{}] is not a projectile", __func__, gfuncs::GetFormName(projectileRef));
        return nodeNames;
    }

    auto impacts = projectile->GetProjectileRuntimeData().impacts;
    if (!impacts.empty()) {
        for (auto* impactData : impacts) {
            if (impactData) {
                std::string nodeName = GetProjectileNodeHitName(impactData);
                if (nodeName != "") {
                    nodeNames.push_back(nodeName);
                }
            }
        }
    }
    return nodeNames;
}

std::vector<RE::TESObjectREFR*> GetRecentProjectileHitRefs(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled, int projectileType) {
    std::vector<RE::TESObjectREFR*> refs;
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{}: ref doesn't exist", __func__);
        return refs;
    }

    auto it = recentHitProjectiles.find(ref);
    if (it != recentHitProjectiles.end()) {
        if (it->second.size() == 0) {
            return refs;
        }
        if (projectileType >= 1 && projectileType <= 7) {
            for (auto& data : it->second) {
                if (GetProjectileRefType(nullptr, data.projectile) == projectileType) {
                    if ((data.projectile->Is3DLoaded() || !only3dLoaded) && (!data.projectile->IsDisabled() || !onlyEnabled)) {
                        refs.push_back(data.projectile);
                    }
                }
            }
        }
        else {
            for (auto& data : it->second) {
                if (gfuncs::IsFormValid(data.projectile)) {
                    if ((data.projectile->Is3DLoaded() || !only3dLoaded) && (!data.projectile->IsDisabled() || !onlyEnabled)) {
                        refs.push_back(data.projectile);
                    }
                }
            }
        }
    }
    return refs;
}

RE::TESObjectREFR* GetLastProjectileHitRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled, int projectileType) {
    RE::TESObjectREFR* returnRef = nullptr;
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{}: ref isn't valid or doesn't exist", __func__);
        return returnRef;
    }

    auto it = recentHitProjectiles.find(ref);
    if (it != recentHitProjectiles.end()) {
        if (it->second.size() == 0) {
            return nullptr;
        }
        if (projectileType >= 1 && projectileType <= 7) {
            for (int i = it->second.size() - 1; i >= 0 && !returnRef; --i) {
                if (GetProjectileRefType(nullptr, it->second[i].projectile) == projectileType) {
                    if ((it->second[i].projectile->Is3DLoaded() || !only3dLoaded) && (!it->second[i].projectile->IsDisabled() || !onlyEnabled)) {
                        returnRef = it->second[i].projectile;
                    }
                }
            }
        }
        else {
            for (int i = it->second.size() - 1; i >= 0 && !returnRef; --i) {
                if (gfuncs::IsFormValid(it->second[i].projectile)) {
                    if ((it->second[i].projectile->Is3DLoaded() || !only3dLoaded) && (!it->second[i].projectile->IsDisabled() || !onlyEnabled)) {
                        returnRef = it->second[i].projectile;
                    }
                }
            }
        }
    }
    return returnRef;
}

std::vector<RE::TESObjectREFR*> GetRecentProjectileShotRefs(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled, int projectileType) {
    std::vector<RE::TESObjectREFR*> refs;
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{}: ref doesn't exist", __func__);
        return refs;
    }

    auto it = recentShotProjectiles.find(ref);
    if (it != recentShotProjectiles.end()) {
        if (it->second.size() == 0) {
            return refs;
        }
        if (projectileType >= 1 && projectileType <= 7) {
            for (auto& data : it->second) {
                if (GetProjectileRefType(nullptr, data.projectile) == projectileType) {
                    if ((data.projectile->Is3DLoaded() || !only3dLoaded) && (!data.projectile->IsDisabled() || !onlyEnabled)) {
                        refs.push_back(data.projectile);
                    }
                }
            }
        }
        else {
            for (auto& data : it->second) {
                if (gfuncs::IsFormValid(data.projectile)) {
                    if ((data.projectile->Is3DLoaded() || !only3dLoaded) && (!data.projectile->IsDisabled() || !onlyEnabled)) {
                        refs.push_back(data.projectile);
                    }
                }
            }
        }
    }
    return refs;
}

RE::TESObjectREFR* GetLastProjectileShotRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled, int projectileType) {
    RE::TESObjectREFR* returnRef = nullptr;
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{}: ref isn't valid or doesn't exist", __func__);
        return returnRef;
    }

    auto it = recentShotProjectiles.find(ref);
    if (it != recentShotProjectiles.end()) {
        if (it->second.size() == 0) {
            return nullptr;
        }
        if (projectileType >= 1 && projectileType <= 7) {
            for (int i = it->second.size() - 1; i >= 0 && !returnRef; --i) {
                if (GetProjectileRefType(nullptr, it->second[i].projectile) == projectileType) {
                    if ((it->second[i].projectile->Is3DLoaded() || !only3dLoaded) && (!it->second[i].projectile->IsDisabled() || !onlyEnabled)) {
                        returnRef = it->second[i].projectile;
                    }
                }
            }
        }
        else {
            for (int i = it->second.size() - 1; i >= 0 && !returnRef; --i) {
                if (gfuncs::IsFormValid(it->second[i].projectile)) {
                    if ((it->second[i].projectile->Is3DLoaded() || !only3dLoaded) && (!it->second[i].projectile->IsDisabled() || !onlyEnabled)) {
                        returnRef = it->second[i].projectile;
                    }
                }
            }
        }
    }
    return returnRef;
}

std::vector<RE::TESObjectREFR*> GetAttachedProjectileRefs(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
    logger::trace("{} called", __func__);

    std::vector<RE::TESObjectREFR*> attachedProjectiles;
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{}: ref isn't valid or doesn't exist", __func__);
        return attachedProjectiles;
    }

    const auto& [allForms, lock] = RE::TESForm::GetAllForms();
    for (auto& [id, form] : *allForms) {
        auto* akRef = form->AsReference();
        if (gfuncs::IsFormValid(akRef)) {
            RE::Projectile* projectileRef = akRef->AsProjectile();
            if (DidProjectileHitRef(projectileRef, ref)) {
                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                    int impactResult = GetProjectileImpactResult(nullptr, projectileRef);
                    if (impactResult == 3 || impactResult == 4) { //impale or stick
                        float distance = akRef->GetPosition().GetDistance(ref->GetPosition());
                        if (distance < 3000.0) {
                            attachedProjectiles.push_back(akRef);
                        }
                    }
                }
            }
        }
    }
    return attachedProjectiles;
}

std::vector<RE::BGSProjectile*> GetAttachedProjectiles(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
    std::vector<RE::BGSProjectile*> attachedProjectiles;

    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{}: ref isn't valid or doesn't exist", __func__);
        return attachedProjectiles;
    }

    auto* data = ref->extraList.GetByType<RE::ExtraAttachedArrows3D>();
    if (data) {
        if (!data->data.empty()) {
            for (auto& dataItem : data->data) {
                if (dataItem.source) {
                    attachedProjectiles.push_back(dataItem.source);
                }
            }
        }
    }
    else {
        logger::debug("{}: couldn't get ExtraAttachedArrows3D for ref[{}]", __func__, gfuncs::GetFormName(ref));
    }
    return attachedProjectiles;
}

//Get all hit projectiles of type =======================================================================================================
//All Projectiles that hit ref
std::vector<RE::TESObjectREFR*> GetAllHitProjectileRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;

    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{}: ref isn't valid or doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidProjectileHitRef(projectileRef, ref)) {
                    if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                        projectileRefs.push_back(akRef);
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidProjectileHitRef(projectileRef, ref)) {
                    if (!akRef->IsDisabled()) {
                        projectileRefs.push_back(akRef);
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidProjectileHitRef(projectileRef, ref)) {
                    if (akRef->Is3DLoaded()) {
                        projectileRefs.push_back(akRef);
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidProjectileHitRef(projectileRef, ref)) {
                    projectileRefs.push_back(akRef);
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 7 arrow
std::vector<RE::TESObjectREFR*> GetAllHitProjectileArrowRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;

    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{} ref doesn't exist or isn't valid", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 6 barrier
std::vector<RE::TESObjectREFR*> GetAllHitProjectileBarrierRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 5 cone
std::vector<RE::TESObjectREFR*> GetAllHitProjectileConeRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 4 Flamethrower
std::vector<RE::TESObjectREFR*> GetAllHitProjectileFlamethrowerRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 3 Beam
std::vector<RE::TESObjectREFR*> GetAllHitProjectileBeamRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 2 Grenade
std::vector<RE::TESObjectREFR*> GetAllHitProjectileGrenadeRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 1 Missile
std::vector<RE::TESObjectREFR*> GetAllHitProjectileMissileRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

std::vector<RE::TESObjectREFR*> GetAllHitProjectileRefsOfType(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled, int projectileType) {
    switch (projectileType) {
    case 1:
        return GetAllHitProjectileMissileRefs(ref, only3dLoaded, onlyEnabled);
    case 2:
        return GetAllHitProjectileGrenadeRefs(ref, only3dLoaded, onlyEnabled);
    case 3:
        return GetAllHitProjectileBeamRefs(ref, only3dLoaded, onlyEnabled);
    case 4:
        return GetAllHitProjectileFlamethrowerRefs(ref, only3dLoaded, onlyEnabled);
    case 5:
        return GetAllHitProjectileConeRefs(ref, only3dLoaded, onlyEnabled);
    case 6:
        return GetAllHitProjectileBarrierRefs(ref, only3dLoaded, onlyEnabled);
    case 7:
        return GetAllHitProjectileArrowRefs(ref, only3dLoaded, onlyEnabled);
    default:
        return GetAllHitProjectileRefs(ref, only3dLoaded, onlyEnabled);
    }
}

//=======================================================================================================================================================

//Get all shot arrows of type ===========================================================================================================================
//All Projectiles that the ref shot
std::vector<RE::TESObjectREFR*> GetAllShotProjectileRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidRefShootProjectile(projectileRef, ref)) {
                    if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                        projectileRefs.push_back(akRef);
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidRefShootProjectile(projectileRef, ref)) {
                    if (!akRef->IsDisabled()) {
                        projectileRefs.push_back(akRef);
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidRefShootProjectile(projectileRef, ref)) {
                    if (akRef->Is3DLoaded()) {
                        projectileRefs.push_back(akRef);
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidRefShootProjectile(projectileRef, ref)) {
                    projectileRefs.push_back(akRef);
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 7 arrow
std::vector<RE::TESObjectREFR*> GetAllShotProjectileArrowRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 6 barrier
std::vector<RE::TESObjectREFR*> GetAllShotProjectileBarrierRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 5 cone
std::vector<RE::TESObjectREFR*> GetAllShotProjectileConeRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 4 Flamethrower
std::vector<RE::TESObjectREFR*> GetAllShotProjectileFlamethrowerRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 3 Beam
std::vector<RE::TESObjectREFR*> GetAllShotProjectileBeamRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 2 Grenade
std::vector<RE::TESObjectREFR*> GetAllShotProjectileGrenadeRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 1 Missile
std::vector<RE::TESObjectREFR*> GetAllShotProjectileMissileRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

std::vector<RE::TESObjectREFR*> GetAllShotProjectileRefsOfType(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled, int projectileType) {
    switch (projectileType) {
    case 1:
        return GetAllShotProjectileMissileRefs(ref, only3dLoaded, onlyEnabled);
    case 2:
        return GetAllShotProjectileGrenadeRefs(ref, only3dLoaded, onlyEnabled);
    case 3:
        return GetAllShotProjectileBeamRefs(ref, only3dLoaded, onlyEnabled);
    case 4:
        return GetAllShotProjectileFlamethrowerRefs(ref, only3dLoaded, onlyEnabled);
    case 5:
        return GetAllShotProjectileConeRefs(ref, only3dLoaded, onlyEnabled);
    case 6:
        return GetAllShotProjectileBarrierRefs(ref, only3dLoaded, onlyEnabled);
    case 7:
        return GetAllShotProjectileArrowRefs(ref, only3dLoaded, onlyEnabled);
    default:
        return GetAllShotProjectileRefs(ref, only3dLoaded, onlyEnabled);
    }
}

// ==================================================================================================================================================================================

RE::TESObjectREFR* GetLastPlayerActivatedRef(RE::StaticFunctionTag*) {
    if (gfuncs::IsFormValid(gfuncs::lastPlayerActivatedRef)) {
        return gfuncs::lastPlayerActivatedRef;
    }
    else {
        return nullptr;
    }
}

RE::TESObjectREFR* GetLastPlayerMenuActivatedRef(RE::StaticFunctionTag*) {
    if (gfuncs::IsFormValid(gfuncs::menuRef)) {
        return gfuncs::menuRef;
    }
    else {
        return nullptr;
    }
}

RE::TESObjectREFR* GetAshPileLinkedRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{}: ref doesn't exist or isn't valid", __func__);
        return nullptr;
    }

    auto* data = ref->extraList.GetByType<RE::ExtraAshPileRef>();
    if (data) {
        if (data->ashPileRef) {
            auto refPtr = data->ashPileRef.get();
            if (refPtr) {
                RE::TESObjectREFR* returnRef = refPtr.get();
                if (gfuncs::IsFormValid(returnRef)) {
                    return returnRef;
                }
            }
        }
    }
    return nullptr;
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

float GetGameHoursPassed(RE::StaticFunctionTag*) {
    return calendar->GetHoursPassed();
}

float GameHoursToRealTimeSeconds(RE::StaticFunctionTag*, float gameHours) {
    float timeScale = calendar->GetTimescale(); //timeScale is minutes ratio
    float gameMinutes = gameHours * 60.0;
    float realMinutes = gameMinutes / timeScale;
    return (realMinutes * 60.0);
}

bool IsGamePaused(RE::StaticFunctionTag*) {
    return ui->GameIsPaused();
}

bool IsInMenu(RE::StaticFunctionTag*) {
    return inMenuMode;
}

bool IsMenuOpen(RE::StaticFunctionTag*, std::string menuName) {
    std::transform(menuName.begin(), menuName.end(), menuName.begin(), tolower);

    for (auto& menu : menusCurrentlyOpen) {
        std::string sMenu = menu.c_str();
        std::transform(sMenu.begin(), sMenu.end(), sMenu.begin(), tolower);
        if (sMenu == menuName) {
            return true;
        }
    }
    return false;
}

std::string GetLastMenuOpened(RE::StaticFunctionTag*) {
    return lastMenuOpened;
}

RE::TESForm* GetLoadMenuLocation() {
    if (!ui) {
        logger::error("{}: couldn't find ui", __func__);
        return nullptr;
    }

    auto loadingMenuGPtr = ui->GetMenu(RE::LoadingMenu::MENU_NAME);
    if (!loadingMenuGPtr) {
        logger::error("{}: couldn't find loadingMenu", __func__);
        return nullptr;
    }

    auto* uiMenu = loadingMenuGPtr.get();
    if (!uiMenu) {
        logger::error("{}: couldn't find uiMenu from loadingMenuGPtr", __func__);
        return nullptr;
    }

    RE::LoadingMenu* loadingMenu = static_cast<RE::LoadingMenu*>(uiMenu);
    if (!loadingMenu) {
        logger::error("{}: couldn't find loadingMenu from uiMenu", __func__);
        return nullptr;
    }

    auto data = loadingMenu->GetRuntimeData();
    if (gfuncs::IsFormValid(data.currentLocation)) {
        return data.currentLocation;
    }
    else {
        return nullptr;
    }
}

bool IsMapMarker(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker) {
    logger::trace("{}: function called", __func__);

    if (!gfuncs::IsFormValid(mapMarker)) {
        logger::warn("IsMapMarker: mapMarker doesn't exist", warn);
        return false;
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        logger::warn("map marker list not found.", debug);
        return false;
    }

    if (!mapMarkerData->mapData) {
        logger::warn("mapData not found.", debug);
        return false;
    }

    return true;
}

bool SetMapMarkerVisible(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker, bool visible) {
    logger::trace("{}: function called", __func__);

    if (!gfuncs::IsFormValid(mapMarker)) {
        logger::warn("{}: mapMarker isn't a valid form", __func__);
        return false;
    }

    auto* marker = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();
    if (!marker) {
        logger::warn("{}: mapMarker ref[{}] isn't a mapMarker", __func__, gfuncs::GetFormName(mapMarker));
        return false;
    }

    if (!marker->mapData) {
        logger::warn("{}: marker->mapData for ref[{}] doesn't exist", __func__, gfuncs::GetFormName(mapMarker));
        return false;
    }

    if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) != visible) {
        marker->mapData->SetVisible(visible);
        return (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == visible);
    }

    return false;
}

bool SetCanFastTravelToMarker(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker, bool canTravelTo) {
    logger::trace("{}: function called", __func__);

    if (!gfuncs::IsFormValid(mapMarker)) {
        logger::warn("{}: mapMarker isn't a valid form", __func__);
        return false;
    }

    auto* marker = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();
    if (!marker) {
        logger::warn("{}: mapMarker ref[{}] isn't a mapMarker", __func__, gfuncs::GetFormName(mapMarker));
        return false;
    }

    if (!marker->mapData) {
        logger::warn("{}: marker->mapData for ref[{}] doesn't exist", __func__, gfuncs::GetFormName(mapMarker));
        return false;
    }

    if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) != canTravelTo) {
        if (canTravelTo) {
            marker->mapData->flags.set(RE::MapMarkerData::Flag::kCanTravelTo);
        }
        else {
            marker->mapData->flags.reset(RE::MapMarkerData::Flag::kCanTravelTo);
        }
        return (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == canTravelTo);
    }
    else {
        return true;
    }
}

bool LoadMostRecentSaveGame(RE::StaticFunctionTag*) {
    //auto manager = RE::UISaveLoadManager::ProcessEvent();
    auto* manager = RE::BGSSaveLoadManager::GetSingleton();
    if (!manager) {
        logger::error("{}: BGSSaveLoadManager not found", __func__);
        return false;
    }

    logger::trace("{}: loading most recent save", __func__);

    return manager->LoadMostRecentSaveGame();
}

// Get all map markers valid for the current worldspace or interior cell grid
RE::BSTArray<RE::ObjectRefHandle>* GetPlayerMapMarkers() {
    std::uint32_t offset = 0;
    if (REL::Module::IsAE())
        offset = 0x500;
    else if (REL::Module::IsSE())
        offset = 0x4F8;
    else
        offset = 0xAE8;
    return reinterpret_cast<RE::BSTArray<RE::ObjectRefHandle>*>((uintptr_t)player + offset);
}

std::vector<RE::TESObjectREFR*> GetAllMapMarkerRefs(RE::StaticFunctionTag*, int visibleFilter, int canTravelToFilter) {
    std::vector<RE::TESObjectREFR*> allMapMarkers;

    if (visibleFilter == 1 && canTravelToFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 1 && canTravelToFilter == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 0 && canTravelToFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 0 && canTravelToFilter == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (canTravelToFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                allMapMarkers.push_back(ref);
                            }
                        }
                    }
                }
            }
        }
    }
    else if (canTravelToFilter == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                allMapMarkers.push_back(ref);
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 1) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                allMapMarkers.push_back(ref);
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 0) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                allMapMarkers.push_back(ref);
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                auto* ref = form->AsReference();
                if (gfuncs::IsFormValid(ref)) {
                    auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                    if (marker) {
                        if (marker->mapData) {
                            allMapMarkers.push_back(ref);
                        }
                    }
                }
            }
        }
    }
    return allMapMarkers;
}

// get all map markers valid for the current world space or interior cell grid
std::vector<RE::TESObjectREFR*> GetCurrentMapMarkerRefs(RE::StaticFunctionTag*, int visibleFilter, int canTravelToFilter) {
    auto* playerMapMarkers = GetPlayerMapMarkers();

    std::vector<RE::TESObjectREFR*> allMapMarkers;

    if (!playerMapMarkers) {
        return allMapMarkers;
    }

    if (playerMapMarkers->size() == 0) {
        return allMapMarkers;
    }

    if (visibleFilter == 1 && canTravelToFilter == 1) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                    if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                        allMapMarkers.push_back(ref);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 1 && canTravelToFilter == 0) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                    if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                        allMapMarkers.push_back(ref);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 0 && canTravelToFilter == 1) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                    if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                        allMapMarkers.push_back(ref);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 0 && canTravelToFilter == 0) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                    if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                        allMapMarkers.push_back(ref);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (canTravelToFilter == 1) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == true) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (canTravelToFilter == 0) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo) == false) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 1) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == true && !ref->IsDisabled()) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (visibleFilter == 0) {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                if (marker->mapData->flags.any(RE::MapMarkerData::Flag::kVisible) == false || ref->IsDisabled()) {
                                    allMapMarkers.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        for (auto& mapMarker : *playerMapMarkers) {
            if (mapMarker) {
                auto refPtr = mapMarker.get();
                if (refPtr) {
                    auto* ref = refPtr.get();
                    if (gfuncs::IsFormValid(ref)) {
                        auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                        if (marker) {
                            if (marker->mapData) {
                                allMapMarkers.push_back(ref);
                            }
                        }
                    }
                }
            }
        }
    }
    return allMapMarkers;
}

RE::TESForm* GetRefWorldSpaceOrCell(RE::TESObjectREFR* ref) {
    RE::TESForm* form = ref->GetWorldspace();
    if (!gfuncs::IsFormValid(form)) {
        form = ref->GetParentCell();
    }
    if (!gfuncs::IsFormValid(form)) {
        return nullptr;
    }
    return form;
}

RE::TESForm* GetCellOrWorldSpaceOriginForRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{}: ref doesn't exist", __func__);
        return nullptr;
    }

    auto* originData = ref->extraList.GetByType<RE::ExtraStartingWorldOrCell>();
    if (!originData) {
        logger::debug("{}: originData for ref({}) not found, getting current worldspace or cell.", __func__, gfuncs::GetFormDataString(ref));

        //origin data on a reference will only exist if it has been moved from its original worldspace or interior cell
        return GetRefWorldSpaceOrCell(ref);
    }

    if (!gfuncs::IsFormValid(originData->startingWorldOrCell)) {
        logger::debug("{}: originData->startingWorldOrCell for ref({}) doesn't exist, getting current worldspace or cell.", __func__, gfuncs::GetFormDataString(ref));
        return GetRefWorldSpaceOrCell(ref);
    }

    return originData->startingWorldOrCell;
}

bool SetCellOrWorldSpaceOriginForRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, RE::TESForm* cellOrWorldSpace) {
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{}: ref doesn't exist", __func__);
        return false;
    }

    if (!gfuncs::IsFormValid(cellOrWorldSpace)) {
        logger::warn("{}: cellOrWorldSpace doesn't exist", __func__);
        return false;
    }

    auto* originData = ref->extraList.GetByType<RE::ExtraStartingWorldOrCell>();
    if (!originData) {
        logger::warn("{}: originData for ref({}) not found", __func__, gfuncs::GetFormDataString(ref));
        return false;
    }

    originData->startingWorldOrCell = cellOrWorldSpace;

    if (IsMapMarker(nullptr, ref)) {
        RE::TESWorldSpace* newOriginWorld = static_cast<RE::TESWorldSpace*>(cellOrWorldSpace);

        if (gfuncs::IsFormValid(newOriginWorld)) {
            RE::TESWorldSpace* currentWorld = player->GetWorldspace();
            if (gfuncs::IsFormValid(currentWorld)) {
                if (newOriginWorld == currentWorld) {
                    auto* mapMarkers = GetPlayerMapMarkers();
                    if (mapMarkers) {
                        //logger::critical("{}: currentMapMarkers size is {}", __func__, mapMarkers->size());

                        auto refHandle = ref->GetHandle();
                        if (!gfuncs::IsObjectInBSTArray(mapMarkers, refHandle)) {
                            //logger::critical("{}: adding ref to currentMapMarkers", __func__);
                            mapMarkers->push_back(refHandle);
                        }
                        else {
                            //logger::critical("{}: ref already in currentMapMarkers", __func__);
                        }
                    }
                    else {
                        //logger::critical("{}: currentMapMarkers not found", __func__);
                    }
                }
            }
        }
    }
    //ref->Update3DPosition(false);

    logger::trace("{}: ref({}) origin set to \n({})", __func__, gfuncs::GetFormDataString(ref),
        gfuncs::GetFormDataString(cellOrWorldSpace));
    //SaveAndLoad();

    //return (originData->startingWorldOrCell == cellOrWorldSpace);
    return true;
}

bool SetMapMarkerName(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker, std::string name) {
    LogAndMessage("Renaming map marker");
    if (!gfuncs::IsFormValid(mapMarker)) {
        LogAndMessage("SetMapMarkerName: mapMarker doesn't exist", warn);
        return false;
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        LogAndMessage("Warning, map marker list not found.", warn);
        return false;
    }

    if (!mapMarkerData->mapData) {
        LogAndMessage("Warning, mapData not found.", warn);
        return false;
    }

    if (mapMarkerData->mapData->locationName.fullName == NULL) {
        LogAndMessage("Warning, LocationName not found.", warn);
        return false;
    }

    const char* cName = name.data();
    mapMarkerData->mapData->locationName.fullName = cName;
    LogAndMessage(std::format("New map marker name = {}", mapMarkerData->mapData->locationName.GetFullName()));

    return true;
}

std::string GetMapMarkerName(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker) {
    LogAndMessage("Getting Marker Name");
    if (!gfuncs::IsFormValid(mapMarker)) {
        LogAndMessage("GetMapMarkerName: mapMarker doesn't exist", warn);
        return "";
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        LogAndMessage("GetMapMarkerName Warning, map marker list not found.", warn);
        return "";
    }

    if (!mapMarkerData->mapData) {
        LogAndMessage("GetMapMarkerName Warning, mapData not found.", warn);
        return "";
    }

    if (mapMarkerData->mapData->locationName.fullName == NULL) {
        LogAndMessage("GetMapMarkerName Warning, LocationName not found.", warn);
        return "";
    }

    return std::string(mapMarkerData->mapData->locationName.GetFullName());
}

bool SetMapMarkerIconType(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker, int iconType) {
    if (!gfuncs::IsFormValid(mapMarker)) {
        LogAndMessage("SetMapMarkerIconType: mapMarker doesn't exist", warn);
        return false;
    }

    LogAndMessage(std::format("Setting Map Marker Type to {}", iconType));

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        LogAndMessage("SetMapMarkerIconType Warning, map marker extra data list not found.", warn);
        return false;
    }

    if (!mapMarkerData->mapData) {
        LogAndMessage("SetMapMarkerIconType Warning, mapData not found.", warn);
        return false;
    }

    mapMarkerData->mapData->type = static_cast<RE::MARKER_TYPE>(iconType);

    return true;
}

int GetMapMarkerIconType(RE::StaticFunctionTag*, RE::TESObjectREFR* mapMarker) {
    LogAndMessage("Getting Map Marker Type");
    if (!gfuncs::IsFormValid(mapMarker)) {
        LogAndMessage("GetMapMarkerIconType: mapMarker doesn't exist", warn);
        return false;
    }

    auto* mapMarkerData = mapMarker->extraList.GetByType<RE::ExtraMapMarker>();

    if (!mapMarkerData) {
        LogAndMessage("GetMapMarkerIconType Warning, map marker list not found.", warn);
        return false;
    }

    if (!mapMarkerData->mapData) {
        LogAndMessage("GetMapMarkerIconType Warning, mapData not found.", warn);
        return false;
    }

    return static_cast<int>(mapMarkerData->mapData->type.get());
}

static inline void CompileAndRunImpl(RE::Script* script, RE::ScriptCompiler* compiler, RE::COMPILER_NAME name, RE::TESObjectREFR* targetRef) {
    using func_t = decltype(CompileAndRunImpl);
    REL::Relocation<func_t> func{ RELOCATION_ID(21416, REL::Module::get().version().patch() < 1130 ? 21890 : 441582) };
    return func(script, compiler, name, targetRef);
}

static inline void CompileAndRun(RE::Script* script, RE::TESObjectREFR* targetRef, RE::COMPILER_NAME name = RE::COMPILER_NAME::kSystemWindowCompiler)
{
    RE::ScriptCompiler compiler;
    CompileAndRunImpl(script, &compiler, name, targetRef);
}

static inline RE::ObjectRefHandle GetSelectedRefHandle() {
    REL::Relocation<RE::ObjectRefHandle*> selectedRef{ RELOCATION_ID(519394, REL::Module::get().version().patch() < 1130 ? 405935 : 504099) };
    return *selectedRef;
}

static inline RE::NiPointer<RE::TESObjectREFR> GetSelectedRef() {
    auto handle = GetSelectedRefHandle();
    return handle.get();
}

//edited form ConsoleUtil NG
static inline void ExecuteConsoleCommand(RE::StaticFunctionTag*, std::string a_command, RE::TESObjectREFR* objRef) {
    LogAndMessage(std::format("{} called. Command = {}", __func__, a_command));

    const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
    const auto script = scriptFactory ? scriptFactory->Create() : nullptr;
    if (script) {
        script->SetCommand(a_command);

        if (gfuncs::IsFormValid(objRef)) {
            CompileAndRun(script, objRef);
        }
        else {
            const auto selectedRef = GetSelectedRef();
            //script->CompileAndRun(selectedRef.get());
            CompileAndRun(script, selectedRef.get());
        }

        delete script;
    }
}

bool HasCollision(RE::StaticFunctionTag*, RE::TESObjectREFR* objRef) {
    LogAndMessage(std::format("{} called", __func__));

    if (!gfuncs::IsFormValid(objRef)) {
        logger::warn("{}: objRef doesn't exist", __func__);
        return false;
    }
    return objRef->HasCollision();
}

int GetFurnitureWorkbenchType(RE::StaticFunctionTag*, RE::TESFurniture* akFurniture) {
    if (!gfuncs::IsFormValid(akFurniture)) {
        logger::warn("{}: akFurniture doesn't exist or isn't valid", __func__);
        return -1;
    }

    return static_cast<int>(akFurniture->workBenchData.benchType.get());
}


int GetFurnitureWorkbenchSkillInt(RE::StaticFunctionTag*, RE::TESFurniture* akFurniture) {
    if (!gfuncs::IsFormValid(akFurniture)) {
        logger::warn("{}: akFurniture doesn't exist or isn't valid", __func__);
        return -1;
    }

    return static_cast<int>(akFurniture->workBenchData.usesSkill.get());
}

std::string GetFurnitureWorkbenchSkillString(RE::StaticFunctionTag*, RE::TESFurniture* akFurniture) {
    if (!gfuncs::IsFormValid(akFurniture)) {
        logger::warn("{}: akFurniture doesn't exist or isn't valid", __func__);
        return "";
    }

    int value = static_cast<int>(akFurniture->workBenchData.usesSkill.get());
    return ActorValueIntsMap[value];
}

//function copied from More Informative Console
RE::BGSMusicType* GetCurrentMusicType(RE::StaticFunctionTag*)
{
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    RE::BGSMusicType* currentPriorityType = nullptr;

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* musicTypeArray = &(dataHandler->GetFormArray(RE::FormType::MusicType));

        RE::BSTArray<RE::TESForm*>::iterator itrEndType = musicTypeArray->end();

        RE::BSIMusicTrack* currentPriorityTrack = nullptr;
        std::int8_t currentPriority = 127;


        for (RE::BSTArray<RE::TESForm*>::iterator itr = musicTypeArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::BGSMusicType* musicType = static_cast<RE::BGSMusicType*>(baseForm);
                RE::BSIMusicType::MUSIC_STATUS musicStatus = musicType->typeStatus.get();

                if (musicStatus == RE::BSIMusicType::MUSIC_STATUS::kPlaying) {
                    uint32_t currentTrackIndex = musicType->currentTrackIndex;

                    if (musicType->tracks.size() > currentTrackIndex) {
                        RE::BSIMusicTrack* currentTrack = musicType->tracks[currentTrackIndex];

                        //if the track takes priority of the current priority track we found
                        if (currentTrack && currentPriority > musicType->priority) {
                            currentPriorityTrack = currentTrack;
                            currentPriorityType = musicType;
                            currentPriority = musicType->priority;
                        }
                    }
                }
            }
        }
    }
    return currentPriorityType;
}

int GetNumberOfTracksInMusicType(RE::StaticFunctionTag*, RE::BGSMusicType* musicType) {
    if (!gfuncs::IsFormValid(musicType)) {
        logger::warn("{}: musicType doesn't exist or isn't valid", __func__);
        return false;
    }
    return musicType->tracks.size();
}

int GetMusicTypeTrackIndex(RE::StaticFunctionTag*, RE::BGSMusicType* musicType) {
    if (!gfuncs::IsFormValid(musicType)) {
        logger::warn("{}: musicType doesn't exist or isn't valid", __func__);
        return false;
    }
    return musicType->currentTrackIndex;
}

void SetMusicTypeTrackIndex(RE::StaticFunctionTag*, RE::BGSMusicType* musicType, int index) {
    if (!gfuncs::IsFormValid(musicType)) {
        logger::warn("{}: musicType doesn't exist or isn't valid", __func__);
        return;
    }
    if (index >= musicType->tracks.size()) {
        index = musicType->tracks.size() - 1;
    }
    else if (index < 0) {
        index = 0;
    }

    if (musicType == GetCurrentMusicType(nullptr)) {
        musicType->tracks[GetMusicTypeTrackIndex(nullptr, musicType)]->DoFinish(false, 3.0);
        musicType->currentTrackIndex = index;
        musicType->tracks[index]->DoPlay();
    }
    else {
        musicType->currentTrackIndex = index;
    }
}

int GetMusicTypePriority(RE::StaticFunctionTag*, RE::BGSMusicType* musicType) {
    if (!gfuncs::IsFormValid(musicType)) {
        logger::warn("{}: musicType doesn't exist or isn't valid", __func__);
        return -1;
    }
    return musicType->priority;
}

void SetMusicTypePriority(RE::StaticFunctionTag*, RE::BGSMusicType* musicType, int priority) {
    if (!gfuncs::IsFormValid(musicType)) {
        logger::warn("{}: musicType doesn't exist or isn't valid", __func__);
        return;
    }
    musicType->priority = priority;
}

int GetMusicTypeStatus(RE::StaticFunctionTag*, RE::BGSMusicType* musicType) {
    if (!gfuncs::IsFormValid(musicType)) {
        logger::warn("{}: musicType doesn't exist or isn't valid", __func__);
        return -1;
    }
    return musicType->typeStatus.underlying();
}

std::vector<RE::EnchantmentItem*> GetKnownEnchantments(RE::StaticFunctionTag*) {
    std::vector<RE::EnchantmentItem*> returnValues;
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    RE::BSTArray<RE::TESForm*>* enchantmentArray = &(dataHandler->GetFormArray(RE::FormType::Enchantment));
    RE::BSTArray<RE::TESForm*>::iterator itrEndType = enchantmentArray->end();

    logger::debug("{} enchantmentArray size[{}]", __func__, enchantmentArray->size());

    for (RE::BSTArray<RE::TESForm*>::iterator it = enchantmentArray->begin(); it != itrEndType; it++) {
        RE::TESForm* baseForm = *it;

        if (gfuncs::IsFormValid(baseForm)) {
            RE::EnchantmentItem* enchantment = baseForm->As<RE::EnchantmentItem>();
            if (gfuncs::IsFormValid(enchantment)) {
                RE::EnchantmentItem* baseEnchantment = enchantment->data.baseEnchantment;
                if (gfuncs::IsFormValid(baseEnchantment)) {
                    if (baseEnchantment->GetKnown()) {
                        if (std::find(returnValues.begin(), returnValues.end(), baseEnchantment) == returnValues.end()) {
                            // baseEnchantment not in returnValues, add it
                            returnValues.push_back(baseEnchantment);
                        }
                    }
                }
            }
        }
    }
    return returnValues;
}

void AddKnownEnchantmentsToFormList(RE::StaticFunctionTag*, RE::BGSListForm* akList) {
    if (!gfuncs::IsFormValid(akList)) {
        logger::warn("{} akList doesn't exist", __func__);
        return;
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

    RE::BSTArray<RE::TESForm*>* enchantmentArray = &(dataHandler->GetFormArray(RE::FormType::Enchantment));
    RE::BSTArray<RE::TESForm*>::iterator itrEndType = enchantmentArray->end();

    logger::debug("{} enchantmentArray size[{}]", __func__, enchantmentArray->size());

    for (RE::BSTArray<RE::TESForm*>::iterator it = enchantmentArray->begin(); it != itrEndType; it++) {
        RE::TESForm* baseForm = *it;

        if (gfuncs::IsFormValid(baseForm)) {
            RE::EnchantmentItem* enchantment = baseForm->As<RE::EnchantmentItem>();
            if (gfuncs::IsFormValid(enchantment)) {
                RE::EnchantmentItem* baseEnchantment = enchantment->data.baseEnchantment;
                if (gfuncs::IsFormValid(baseEnchantment)) {
                    if (baseEnchantment->GetKnown()) {
                        //if (!akList->HasForm(baseEnchantment)) {
                        akList->AddForm(baseEnchantment);
                        //}
                    }
                }
            }
        }
    }
}

std::string GetWordOfPowerTranslation(RE::StaticFunctionTag*, RE::TESWordOfPower* akWord) {
    if (!gfuncs::IsFormValid(akWord)) {
        logger::warn("{} akWord doesn't exist.", __func__);
        return "";
    }

    return static_cast<std::string>(akWord->translation);
}

void UnlockShout(RE::StaticFunctionTag*, RE::TESShout* akShout) {
    if (!gfuncs::IsFormValid(akShout)) {
        logger::warn("{} akShout doesn't exist.", __func__);
        return;
    }

    player->AddShout(akShout);

    logger::debug("{} {} ID {:x}", __func__, akShout->GetName(), akShout->GetFormID());

    RE::TESWordOfPower* word = akShout->variations[0].word;
    if (gfuncs::IsFormValid(word)) {
        //gfuncs::gfuncs::install->UnlockWord(word);
        logger::debug("{} unlock word 1 {} ID {:x}", __func__, word->GetName(), word->GetFormID());
        std::string command = "player.teachword " + gfuncs::IntToHex(int(word->GetFormID())); //didn't find a teachword function in NG, so using console command as workaround. 
        ExecuteConsoleCommand(nullptr, command, nullptr);
        player->UnlockWord(word);
    }

    word = akShout->variations[1].word;
    if (gfuncs::IsFormValid(word)) {
        //gfuncs::playerRef->UnlockWord(word);
        logger::debug("{} unlock word 2 {} ID {:x}", __func__, word->GetName(), word->GetFormID());
        std::string command = "player.teachword " + gfuncs::IntToHex(int(word->GetFormID()));
        ExecuteConsoleCommand(nullptr, command, nullptr);
        player->UnlockWord(word);
    }

    word = akShout->variations[2].word;
    if (gfuncs::IsFormValid(word)) {
        //gfuncs::playerRef->UnlockWord(word);
        logger::debug("{} unlock word 3 {} ID {:x}", __func__, word->GetName(), word->GetFormID());
        std::string command = "player.teachword " + gfuncs::IntToHex(int(word->GetFormID()));
        ExecuteConsoleCommand(nullptr, command, nullptr);
        player->UnlockWord(word);
    }
}

void AddAndUnlockAllShouts(RE::StaticFunctionTag*, int minNumberOfWordsWithTranslations, bool onlyShoutsWithNames, bool onlyShoutsWithDescriptions) {
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    bool minNumberOfWordsCheck = (minNumberOfWordsWithTranslations > 0 && minNumberOfWordsWithTranslations <= 3);

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Shout));

        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();

        //loop through all shouts
        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;
            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESShout* akShout = baseForm->As<RE::TESShout>();
                if (gfuncs::IsFormValid(akShout)) {
                    if (onlyShoutsWithNames) {
                        if (akShout->GetName() == "") {
                            continue;
                        }
                    }

                    if (onlyShoutsWithDescriptions) {
                        if (GetDescription(akShout, "") == "") {
                            continue;
                        }
                    }

                    if (minNumberOfWordsCheck) {
                        RE::TESWordOfPower* word = akShout->variations[minNumberOfWordsWithTranslations - 1].word;
                        if (GetWordOfPowerTranslation(nullptr, word) == "") {
                            continue;
                        }
                    }
                    UnlockShout(nullptr, akShout);
                }
            }
        }
    }
}

void SetBookSpell(RE::StaticFunctionTag*, RE::TESObjectBOOK* akBook, RE::SpellItem* akSpell) {
    logger::debug("{} called", __func__);

    if (!gfuncs::IsFormValid(akBook)) {
        logger::warn("{} akBook doesn't exist.", __func__);
        return;
    }

    if (!gfuncs::IsFormValid(akSpell)) {
        akBook->data.flags.reset(RE::OBJ_BOOK::Flag::kTeachesSpell);
        logger::debug("{} akSpell is none, removing teaches spell flag", __func__);
        return;
    }

    else {
        akBook->data.teaches.spell = akSpell;
        akBook->data.flags.reset(RE::OBJ_BOOK::Flag::kTeachesSpell);
        akBook->data.flags |= RE::OBJ_BOOK::Flag::kTeachesSpell;
    }
}

RE::TESObjectBOOK* GetSpellTomeForSpell(RE::StaticFunctionTag*, RE::SpellItem* akSpell) {
    if (!gfuncs::IsFormValid(akSpell)) {
        logger::warn("{} akSpell doesn't exist.", __func__);
        return nullptr;
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();


        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                if (gfuncs::IsFormValid(akBook)) {
                    if (akBook->TeachesSpell()) {
                        if (akBook->GetSpell() == akSpell) {
                            return akBook;
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

std::vector<RE::TESObjectBOOK*> GetSpellTomesForSpell(RE::StaticFunctionTag*, RE::SpellItem* akSpell) {
    std::vector<RE::TESObjectBOOK*> v;

    if (!gfuncs::IsFormValid(akSpell)) {
        logger::warn("{} akSpell doesn't exist.", __func__);
        return v;
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();


        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                if (gfuncs::IsFormValid(akBook)) {
                    if (akBook->TeachesSpell()) {
                        if (akBook->GetSpell() == akSpell) {
                            v.push_back(akBook);
                        }
                    }
                }
            }
        }
    }
    return v;
}

void AddSpellTomesForSpellToList(RE::StaticFunctionTag*, RE::SpellItem* akSpell, RE::BGSListForm* akList) {
    if (!gfuncs::IsFormValid(akSpell)) {
        logger::warn("{} akSpell doesn't exist.", __func__);
        return;
    }

    if (!gfuncs::IsFormValid(akList)) {
        logger::warn("{} akList doesn't exist.", __func__);
        return;
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();

        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                if (gfuncs::IsFormValid(akBook)) {
                    if (akBook->TeachesSpell()) {
                        if (akBook->GetSpell() == akSpell) {
                            akList->AddForm(akBook);
                        }
                    }
                }
            }
        }
    }
}

std::string GetBookSkill(RE::StaticFunctionTag*, RE::TESObjectBOOK* akBook) {
    if (!gfuncs::IsFormValid(akBook)) {
        logger::warn("{} akBook doesn't exist.", __func__);
        return "";
    }

    if (skillBooksMap.find(akBook) != skillBooksMap.end()) {
        return ActorValueIntsMap[(skillBooksMap[akBook])];
    }
    return "";
}

void SetBookSkillInt(RE::TESObjectBOOK* akBook, int value, std::string skill = "", bool addToSkillBooksMap = true) {
    if (value == -1) {
        akBook->data.flags.reset(RE::OBJ_BOOK::Flag::kAdvancesActorValue);
        akBook->RemoveChange(RE::TESObjectBOOK::ChangeFlags::kTeachesSkill);
        if (addToSkillBooksMap) {
            skillBooksMap[akBook] = value;
        }
        logger::debug("{} book[{}] ID[{:x}] no longer teaches skill", __func__, gfuncs::GetFormName(akBook), akBook->GetFormID());
        return;
    }
    else if (value < -1 || value > 163) {
        logger::debug("{} skill[{}] value[{}] not recognized", __func__, skill, value);
        return;
    }

    akBook->data.teaches.actorValueToAdvance = static_cast<RE::ActorValue>(value);
    if (addToSkillBooksMap) {
        skillBooksMap[akBook] = value;
    }
    akBook->data.flags.set(RE::OBJ_BOOK::Flag::kAdvancesActorValue);
    akBook->AddChange(RE::TESObjectBOOK::ChangeFlags::kTeachesSkill);
}

void SetBookSkill(RE::StaticFunctionTag*, RE::TESObjectBOOK* akBook, std::string actorValue) {
    logger::debug("{} called", __func__);

    if (!gfuncs::IsFormValid(akBook)) {
        logger::warn("{} akBook doesn't exist.", __func__);
        return;
    }

    int value = GetActorValueInt(actorValue);
    SetBookSkillInt(akBook, value, actorValue);
}

std::vector<RE::TESObjectBOOK*> GetSkillBooksForSkill(RE::StaticFunctionTag*, std::string actorValue) {
    std::vector<RE::TESObjectBOOK*> v;

    int value = GetActorValueInt(actorValue);
    logger::debug("{} actorValue = {} int value = {}", __func__, actorValue, value);

    if (value < 0) {
        logger::warn("{} actorValue [{}] not recognized", __func__, actorValue);
        return v;
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();


        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                if (gfuncs::IsFormValid(akBook)) {
                    if (skillBooksMap.find(akBook) != skillBooksMap.end()) {
                        if (skillBooksMap[akBook] == value) {
                            v.push_back(akBook);
                        }
                    }
                }
            }
        }
    }
    return v;
}

void AddSkillBookForSkillToList(RE::StaticFunctionTag*, std::string actorValue, RE::BGSListForm* akList) {
    if (!gfuncs::IsFormValid(akList)) {
        logger::warn("{} akList doesn't exist.", __func__);
        return;
    }

    int value = GetActorValueInt(actorValue);
    logger::debug("{} actorValue = {} int value = {}", __func__, actorValue, value);

    if (value < 0) {
        logger::warn("{} actorValue [{}] not recognized", __func__, actorValue);
        return;
    }

    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();


        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                if (gfuncs::IsFormValid(akBook)) {
                    if (skillBooksMap.find(akBook) != skillBooksMap.end()) {
                        if (skillBooksMap[akBook] == value) {
                            akList->AddForm(akBook);
                        }
                    }
                }
            }
        }
    }
}

void SetBookRead(RE::StaticFunctionTag*, RE::TESObjectBOOK* akBook, bool read) {
    if (!gfuncs::IsFormValid(akBook)) {
        logger::warn("{} akBook doesn't exist.", __func__);
        return;
    }

    if (read) {
        if (!akBook->IsRead()) {
            akBook->data.flags.set(RE::OBJ_BOOK::Flag::kHasBeenRead);
            akBook->AddChange(RE::TESObjectBOOK::ChangeFlags::kRead);

            if (skillBooksMap.find(akBook) != skillBooksMap.end()) { //book is a skill book
                SetBookSkillInt(akBook, -1, "", false);
            }
        }
    }
    else {
        if (akBook->IsRead()) {
            akBook->data.flags.reset(RE::OBJ_BOOK::Flag::kHasBeenRead);
            akBook->RemoveChange(RE::TESObjectBOOK::ChangeFlags::kRead);

            if (skillBooksMap.find(akBook) != skillBooksMap.end()) { //book is a skill book
                SetBookSkillInt(akBook, skillBooksMap[akBook]);
            }
        }
    }
}

void SetAllBooksRead(RE::StaticFunctionTag*, bool read) {
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* akArray = &(dataHandler->GetFormArray(RE::FormType::Book));

        RE::BSTArray<RE::TESForm*>::iterator itrEndType = akArray->end();

        for (RE::BSTArray<RE::TESForm*>::iterator itr = akArray->begin(); itr != itrEndType; itr++) {
            RE::TESForm* baseForm = *itr;

            if (gfuncs::IsFormValid(baseForm)) {
                RE::TESObjectBOOK* akBook = baseForm->As<RE::TESObjectBOOK>();
                SetBookRead(nullptr, akBook, read);
            }
        }
    }
}


RE::TESForm* FindNullForm() {
    const auto& [allForms, lock] = RE::TESForm::GetAllForms();

    for (auto& [id, form] : *allForms) {
        if (form) {
            if (form->GetFormType() == RE::FormType::None) {
                logger::trace("null TESForm* found");
                return form;
            }
        }
    }
    return nullptr;
}

int GetActiveMagicEffectConditionStatus(RE::StaticFunctionTag*, RE::ActiveEffect* akEffect) {
    if (!akEffect) {
        logger::warn("{} akEffect doesn't exist", __func__);
        return -1;
    }

    if (akEffect->conditionStatus.any(RE::ActiveEffect::ConditionStatus::kTrue)) {
        //logger::trace("{}: conditionStatus is kTrue", __func__);
        return 1;
    }
    else if (akEffect->conditionStatus.any(RE::ActiveEffect::ConditionStatus::kFalse)) {
        //logger::trace("{}: conditionStatus is kFalse", __func__);
        return 0;
    }
    else {
        return -1;
    }
}

RE::TESForm* GetActiveEffectSource(RE::StaticFunctionTag*, RE::ActiveEffect* akEffect) {
    if (!akEffect) {
        logger::warn("{} akEffect doesn't exist", __func__);
        return nullptr;
    }

    return akEffect->spell;
}

int GetActiveEffectCastingSource(RE::StaticFunctionTag*, RE::ActiveEffect* akEffect) {
    if (!akEffect) {
        logger::warn("{} akEffect doesn't exist", __func__);
        return -1;
    }

    return static_cast<int>(akEffect->castingSource);
}

std::vector<RE::EffectSetting*> GetMagicEffectsForForm(RE::StaticFunctionTag*, RE::TESForm* akForm) {
    std::vector<RE::EffectSetting*> akEffects;
    if (!gfuncs::IsFormValid(akForm)) {
        logger::warn("{} akForm doesn't exist", __func__);
        return akEffects;
    }

    RE::MagicItem* magicItem = akForm->As<RE::MagicItem>();
    if (!gfuncs::IsFormValid(magicItem)) {
        logger::warn("{} akForm name[{}] editorID[{}] formID[{}], is not a magic item", __func__, gfuncs::GetFormName(akForm), GetFormEditorId(nullptr, akForm, ""), akForm->GetFormID());
        return akEffects;
    }

    int m = magicItem->effects.size();
    for (int i = 0; i < m; i++) {
        RE::EffectSetting* effect = magicItem->effects[i]->baseEffect;
        if (gfuncs::IsFormValid(effect)) {
            akEffects.push_back(effect);
        }
    }
    return akEffects;
}

bool IsFormMagicItem(RE::StaticFunctionTag*, RE::TESForm* akForm) {
    if (!gfuncs::IsFormValid(akForm)) {
        logger::warn("{} akForm doesn't exist", __func__);
        return false;
    }

    RE::MagicItem* magicItem = akForm->As<RE::MagicItem>();
    if (gfuncs::IsFormValid(magicItem)) {
        return true;
    }
    else {
        return false;
    }
}

bool IsMagicEffectActiveOnRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, RE::EffectSetting* akMagicEffect, RE::TESForm* magicSource) {
    //logger::trace("{}: called", __func__);

    if (!gfuncs::IsFormValid(ref)) {
        return false;
    }

    if (!gfuncs::IsFormValid(akMagicEffect)) {
        return false;
    }

    RE::MagicTarget* magicTarget = ref->GetMagicTarget();

    if (magicTarget) {
        auto activeEffects = magicTarget->GetActiveEffectList();
        if (activeEffects) {

            if (gfuncs::IsFormValid(magicSource)) {
                for (RE::ActiveEffect* effect : *activeEffects) {
                    if (effect) {
                        if (!IsBadReadPtr(effect, sizeof(effect))) {
                            auto* baseEffect = effect->GetBaseObject();
                            if (gfuncs::IsFormValid(baseEffect)) {
                                if (baseEffect == akMagicEffect && effect->spell == magicSource) {
                                    if (effect->conditionStatus.any(RE::ActiveEffect::ConditionStatus::kTrue)) {
                                        logger::trace("{}: conditionStatus for effect[{}] from source[{}] on ref[{}] is True",
                                            __func__, gfuncs::GetFormName(baseEffect), gfuncs::GetFormName(magicSource), gfuncs::GetFormName(ref));

                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else {
                for (RE::ActiveEffect* effect : *activeEffects) {
                    if (effect) {
                        if (!IsBadReadPtr(effect, sizeof(effect))) {
                            auto* baseEffect = effect->GetBaseObject();
                            if (gfuncs::IsFormValid(baseEffect)) {
                                if (baseEffect == akMagicEffect) {
                                    if (effect->conditionStatus.any(RE::ActiveEffect::ConditionStatus::kTrue)) {
                                        logger::trace("{}: conditionStatus for effect[{}] on ref[{}] is True",
                                            __func__, gfuncs::GetFormName(baseEffect), gfuncs::GetFormName(ref));

                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

//Function DispelMagicEffectOnRef(ObjectReference ref, MagicEffect akMagicEffect, Form magicSource = none) Global Native
void DispelMagicEffectOnRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, RE::EffectSetting* akMagicEffect, RE::TESForm* magicSource) {
    //logger::trace("{}: called", __func__);

    if (!gfuncs::IsFormValid(ref)) {
        return;
    }

    if (!gfuncs::IsFormValid(akMagicEffect)) {
        return;
    }

    auto effectName = gfuncs::GetFormName(akMagicEffect);
    auto refName = gfuncs::GetFormName(ref);

    RE::MagicTarget* magicTarget = ref->GetMagicTarget();

    if (magicTarget) {
        //logger::trace("{}: magicTarget found", __func__);

        auto activeEffects = magicTarget->GetActiveEffectList();
        if (activeEffects) {
            if (gfuncs::IsFormValid(magicSource)) {
                auto sourceName = gfuncs::GetFormName(magicSource);
                for (RE::ActiveEffect* effect : *activeEffects) {
                    if (effect) {
                        if (!IsBadReadPtr(effect, sizeof(effect))) {
                            auto* baseEffect = effect->GetBaseObject();
                            if (gfuncs::IsFormValid(baseEffect)) {
                                if (baseEffect == akMagicEffect && magicSource == effect->spell) {
                                    logger::debug("{}: dispelling effect[{}] from source [{}] on ref[{}]", __func__, effectName, sourceName, refName);
                                    effect->Dispel(true);
                                }
                            }
                        }
                    }
                }
            }
            else {
                for (RE::ActiveEffect* effect : *activeEffects) {
                    if (effect) {
                        if (!IsBadReadPtr(effect, sizeof(effect))) {
                            auto* baseEffect = effect->GetBaseObject();
                            if (gfuncs::IsFormValid(baseEffect)) {
                                if (baseEffect == akMagicEffect) {
                                    logger::debug("{}: dispelling effect[{}] on ref[{}]", __func__, effectName, refName);
                                    effect->Dispel(true);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//kNone = 0,
//kPetty = 1,
//kLesser = 2,
//kCommon = 3,
//kGreater = 4,
//kGrand = 5
void SetSoulGemSize(RE::StaticFunctionTag*, RE::TESSoulGem* akGem, int level) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(akGem)) {
        logger::warn("{}: error akGem doesn't exist", __func__);
        return;
    }

    if (level < 0) {
        level = 0;
    }
    else if (level > 5) {
        level = 5;
    }

    akGem->soulCapacity = static_cast<RE::SOUL_LEVEL>(level);
}

void SetSoulGemCanHoldNPCSoul(RE::StaticFunctionTag*, RE::TESSoulGem* akGem, bool canCarry) {
    logger::debug("{} called", __func__);

    if (!gfuncs::IsFormValid(akGem)) {
        logger::warn("{}: error akGem doesn't exist", __func__);
        return;
    }

    if (canCarry) {
        akGem->formFlags |= RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
    }
    else {
        akGem->formFlags &= ~RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
    }
}

bool CanSoulGemHoldNPCSoul(RE::StaticFunctionTag*, RE::TESSoulGem* akGem) {
    logger::debug("{} called", __func__);

    if (!gfuncs::IsFormValid(akGem)) {
        logger::warn("{}: error akGem doesn't exist", __func__);
        return false;
    }
    return akGem->CanHoldNPCSoul();
}

RE::TESCondition* condition_isPowerAttacking;
bool IsActorPowerAttacking(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
        return false;
    }

    if (!gfuncs::IsFormValid(akTarget)) {
        logger::warn("{}: error, akTarget doesn't exist", __func__);
        return false;
    }

    return akActor->WouldBeStealing(akTarget);
}

RE::TESCondition* condition_IsAttacking;
bool IsActorAttacking(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
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
        logger::warn("{}: error, akActor doesn't exist", __func__);
        return false;
    }
    return akActor->IsAMount();
}

bool IsActorInMidAir(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("{}: error, akActor doesn't exist", __func__);
        return false;
    }
    return akActor->IsInMidair();
}

bool IsActorInRagdollState(RE::StaticFunctionTag*, RE::Actor* akActor) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("{}: error, akActor doesn't exist", __func__);
        return false;
    }
    return akActor->IsInRagdollState();
}

int GetDetectionLevel(RE::StaticFunctionTag*, RE::Actor* akActor, RE::Actor* akTarget) {
    if (!gfuncs::IsFormValid(akActor)) {
        logger::warn("{}: error, akActor doesn't exist", __func__);
        return -1;
    }

    if (!akTarget) {
        logger::warn("{}: error, akTarget doesn't exist", __func__);
        return -1;
    }
    return akActor->RequestDetectionLevel(akTarget);
}

std::string GetKeywordString(RE::StaticFunctionTag*, RE::BGSKeyword* akKeyword) {
    logger::trace("{}", __func__);
    if (!gfuncs::IsFormValid(akKeyword)) {
        logger::warn("{} akKeyword doesn't exist", __func__);
        return "";
    }
    return std::string(akKeyword->GetFormEditorID());
}

void SetKeywordString(RE::StaticFunctionTag*, RE::BGSKeyword* akKeyword, std::string keywordString) {
    logger::trace("{} {}", __func__, keywordString);

    //if (!savedFormIDs) { savedFormIDs = new SavedFormIDs(); }

    if (!gfuncs::IsFormValid(akKeyword)) {
        logger::warn("{} akKeyword doesn't exist", __func__);
        return;
    }
    akKeyword->SetFormEditorID(keywordString.c_str());
}

RE::BGSKeyword* CreateKeyword(RE::StaticFunctionTag*) {
    logger::trace("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>()->Create();
    if (!gfuncs::IsFormValid(newForm, false)) {
        logger::warn("{} failed", __func__);
    }
    else {
        logger::debug("{} success, dynamic form[{}]", __func__, newForm->IsDynamicForm());
    }

    return newForm;
}

RE::BGSListForm* CreateFormList(RE::StaticFunctionTag*, RE::BGSListForm* formListFiller) {
    logger::trace("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSListForm>()->Create();
    if (!gfuncs::IsFormValid(newForm, false)) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::debug("{} success", __func__);
        if (gfuncs::IsFormValid(formListFiller)) {
            logger::debug("{} IsDynamicForm[{}]", __func__, formListFiller->IsDynamicForm());

            formListFiller->ForEachForm([&](auto& akForm) {
                auto* form = &akForm;
                newForm->AddForm(form);
                return RE::BSContainer::ForEachResult::kContinue;
                });
        }
    }
    return newForm;
}

RE::BGSColorForm* CreateColorForm(RE::StaticFunctionTag*, int color) {
    logger::trace("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSColorForm>()->Create();
    if (!gfuncs::IsFormValid(newForm, false)) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::debug("{} success", __func__);
        newForm->color = color;
    }

    return newForm;
}

RE::BGSConstructibleObject* CreateConstructibleObject(RE::StaticFunctionTag*) {
    logger::trace("{} called", __func__);

    //RE::BGSConstructibleObject
    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSConstructibleObject>()->Create();
    if (!gfuncs::IsFormValid(newForm, false)) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::debug("{} success", __func__);
    }

    return newForm;
}

RE::BGSTextureSet* CreateTextureSet(RE::StaticFunctionTag*) {
    logger::trace("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSTextureSet>()->Create();
    if (!gfuncs::IsFormValid(newForm, false)) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::debug("{} success", __func__);
    }

    return newForm;
}

RE::TESSound* CreateSoundMarker(RE::StaticFunctionTag*) {
    logger::trace("{} called", __func__);

    auto* newForm = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESSound>()->Create();
    if (!gfuncs::IsFormValid(newForm, false)) {
        logger::error("{} failed", __func__);
    }
    else {
        logger::debug("{} success", __func__);
    }

    return newForm;
}

std::map<int, RE::BSSoundHandle> playedSoundHandlesMap;
RE::BSFixedString soundFinishEvent = "OnSoundFinish";
void CreateSoundEvent(RE::TESForm* soundOrDescriptor, RE::BSSoundHandle& soundHandle, std::vector<RE::VMHandle> vmHandles, int intervalCheck) {
    std::thread t([=]() {
        playedSoundHandlesMap[int(soundHandle.soundID)] = soundHandle;
        //wait for sound to finish playing, then send events for handles. state 2 == stopped
        while (soundHandle.state.underlying() != 2 && soundHandle.IsValid()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalCheck));
        }

        //remove sound handle from playedSoundHandlesMap
        auto it = playedSoundHandlesMap.find(soundHandle.soundID);
        if (it != playedSoundHandlesMap.end()) {
            playedSoundHandlesMap.erase(it);
        }

        auto* args = RE::MakeFunctionArguments((RE::TESForm*)soundOrDescriptor, (int)soundHandle.soundID);
        SendEvents(vmHandles, soundFinishEvent, args);
        });
    t.detach();
}

RE::BSSoundHandle* GetSoundHandleById(int id) {
    auto it = playedSoundHandlesMap.find(id);
    if (it != playedSoundHandlesMap.end()) {
        if (it->second.soundID == id) {
            return &it->second;
        }
    }

    return nullptr;
}

int PlaySound(RE::StaticFunctionTag*, RE::TESSound* akSound, RE::TESObjectREFR* akSource, float volume,
    RE::TESForm* eventReceiverForm, RE::BGSBaseAlias* eventReceiverAlias, RE::ActiveEffect* eventReceiverActiveEffect) {

    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(akSound)) {
        logger::warn("{}: error, akSound doesn't exist", __func__);
        return -1;
    }

    if (!gfuncs::IsFormValid(akSource)) {
        logger::warn("{}: error, akSource doesn't exist", __func__);
        return -1;
    }

    auto* audiomanager = RE::BSAudioManager::GetSingleton();
    RE::BSSoundHandle soundHandle;

    audiomanager->BuildSoundDataFromDescriptor(soundHandle, akSound->descriptor->soundDescriptor);

    soundHandle.SetObjectToFollow(akSource->Get3D());
    soundHandle.SetVolume(volume);
    soundHandle.Play();

    std::vector<RE::VMHandle> vmHandles;

    if (gfuncs::IsFormValid(eventReceiverForm)) {
        RE::VMHandle vmHandle = gfuncs::GetHandle(eventReceiverForm);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (eventReceiverAlias) {
        RE::VMHandle vmHandle = gfuncs::GetHandle(eventReceiverAlias);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (eventReceiverActiveEffect) {
        RE::VMHandle vmHandle = gfuncs::GetHandle(eventReceiverActiveEffect);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    CreateSoundEvent(akSound, soundHandle, vmHandles, 1000);
    return soundHandle.soundID;
}

int PlaySoundDescriptor(RE::StaticFunctionTag*, RE::BGSSoundDescriptorForm* akSoundDescriptor, RE::TESObjectREFR* akSource, float volume,
    RE::TESForm* eventReceiverForm, RE::BGSBaseAlias* eventReceiverAlias, RE::ActiveEffect* eventReceiverActiveEffect) {

    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(akSoundDescriptor)) {
        logger::warn("{}: error, akSoundDescriptor doesn't exist", __func__);
        return -1;
    }

    if (!gfuncs::IsFormValid(akSource)) {
        logger::warn("{}: error, akSource doesn't exist", __func__);
        return -1;
    }

    auto* audiomanager = RE::BSAudioManager::GetSingleton();
    RE::BSSoundHandle soundHandle;

    audiomanager->BuildSoundDataFromDescriptor(soundHandle, akSoundDescriptor->soundDescriptor);

    soundHandle.SetObjectToFollow(akSource->Get3D());
    soundHandle.SetVolume(volume);
    soundHandle.Play();

    std::vector<RE::VMHandle> vmHandles;

    if (gfuncs::IsFormValid(eventReceiverForm)) {
        RE::VMHandle vmHandle = gfuncs::GetHandle(eventReceiverForm);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (eventReceiverAlias) {
        RE::VMHandle vmHandle = gfuncs::GetHandle(eventReceiverAlias);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (eventReceiverActiveEffect) {
        RE::VMHandle vmHandle = gfuncs::GetHandle(eventReceiverActiveEffect);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    CreateSoundEvent(akSoundDescriptor, soundHandle, vmHandles, 1000);
    return soundHandle.soundID;
}

bool SetSoundInstanceSource(RE::StaticFunctionTag*, int instanceID, RE::TESObjectREFR* ref) {
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{} error: ref doesn't exist or isn't valid.", __func__);
        return false;
    }

    RE::BSSoundHandle* soundHandle = GetSoundHandleById(instanceID);
    if (!soundHandle) {
        logger::warn("{} error: couldn't find sound handle for instanceID [{}]", __func__, instanceID);
        return false;
    }

    if (soundHandle->state.underlying() != 2 && soundHandle->IsValid()) {
        RE::NiAVObject* obj3d = gfuncs::GetNiAVObjectForRef(ref);
        if (!obj3d) {
            logger::warn("{} error: couldn't get 3d for ref: [{}]", __func__, gfuncs::GetFormName(ref));
            return false;
        }
        else {
            soundHandle->SetObjectToFollow(obj3d);
            logger::debug("{}: instanceID [{}] set to follow ref: [{}]", __func__, instanceID, gfuncs::GetFormName(ref));
            return true;
        }
    }

    logger::warn("{} error: instanceID [{}] is no longer playing or valid. Not set to follow ref: [{}]", __func__, instanceID, gfuncs::GetFormName(ref));
    return false;
}

RE::BGSSoundCategory* GetParentSoundCategory(RE::StaticFunctionTag*, RE::BGSSoundCategory* akSoundCategory) {
    if (!gfuncs::IsFormValid(akSoundCategory)) {
        logger::warn("{}: error, akSoundCategory doesn't exist", __func__);
        return nullptr;
    }

    return akSoundCategory->parentCategory;
}

RE::BGSSoundCategory* GetSoundCategoryForSoundDescriptor(RE::StaticFunctionTag*, RE::BGSSoundDescriptorForm* akSoundDescriptor) {
    if (!gfuncs::IsFormValid(akSoundDescriptor)) {
        logger::warn("{}: error, akSoundDescriptor doesn't exist", __func__);
        return nullptr;
    }

    if (!akSoundDescriptor->soundDescriptor) {
        logger::warn("{}: error, akSoundDescriptor->soundDescriptor doesn't exist", __func__);
        return nullptr;
    }

    return akSoundDescriptor->soundDescriptor->category;
}

void SetSoundCategoryForSoundDescriptor(RE::StaticFunctionTag*, RE::BGSSoundDescriptorForm* akSoundDescriptor, RE::BGSSoundCategory* akSoundCategory) {
    if (!gfuncs::IsFormValid(akSoundCategory)) {
        logger::warn("{}: error, akSoundCategory doesn't exist", __func__);
        return;
    }

    if (!gfuncs::IsFormValid(akSoundDescriptor)) {
        logger::warn("{}: error, akSoundDescriptor doesn't exist", __func__);
        return;
    }

    if (!akSoundDescriptor->soundDescriptor) {
        logger::error("{}: error, akSoundDescriptor->soundDescriptor doesn't exist", __func__);
        return;
    }

    akSoundDescriptor->soundDescriptor->category = akSoundCategory;
}

float GetSoundCategoryVolume(RE::StaticFunctionTag*, RE::BGSSoundCategory* akCategory) {
    if (!gfuncs::IsFormValid(akCategory)) {
        logger::warn("{}: error, akCategory doesn't exist", __func__);
        return -1.0;
    }

    return akCategory->GetCategoryVolume();
}

float GetSoundCategoryFrequency(RE::StaticFunctionTag*, RE::BGSSoundCategory* akCategory) {
    if (!gfuncs::IsFormValid(akCategory)) {
        logger::warn("{}: error, akCategory doesn't exist", __func__);
        return -1.0;
    }

    return akCategory->GetCategoryFrequency();
}

//event callbacks from papyrus=========================================================================================================================
//no longer needed as not referencing ammo.projectile at all anymore
// 
//from papyrus
void SaveProjectileForAmmo(RE::StaticFunctionTag*, RE::TESAmmo* akAmmo, RE::BGSProjectile* akProjectile) {
    //if (gfuncs::IsFormValid(akAmmo) && gfuncs::IsFormValid(akProjectile)) {
    //    RE::TESForm* ammoProjectileForm = RE::TESForm::LookupByID(akAmmo->data.projectile->GetFormID());
    //    if (!ammoProjectileForm) { //projectile for ammo not set correctly
    //        akAmmo->data.projectile = akProjectile;
    //        logger::debug("ammo {} projectile {} saved from papyrus", gfuncs::GetFormDataString(akAmmo), gfuncs::GetFormDataString(akAmmo->data.projectile));
    //    }
    //}
}

//initially ammo->data.projectile is not set correctly and causes CTDs if trying to access. 
//This saves ammo projectiles from papyrus if necessarry so they are set correctly
void SaveAllAmmoProjectilesFromPapyrus() {
    //auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    //if (!vm) {
    //    return;
    //}

    //std::vector<RE::TESAmmo*> allAmmos;
    //const auto& [allForms, lock] = RE::TESForm::GetAllForms();
    //for (auto& [id, form] : *allForms) {
    //    if (gfuncs::IsFormValid(form)) {
    //        RE::TESAmmo* ammo = form->As<RE::TESAmmo>();
    //        if (gfuncs::IsFormValid(ammo)) {
    //            allAmmos.push_back(ammo);
    //        }
    //    }
    //}
    //auto* args = RE::MakeFunctionArguments((std::vector<RE::TESAmmo*>)allAmmos);
    //RE::BSFixedString sEvent = "DbSkseFunctions_OnSaveAllAmmoProjectiles";
    //vm->SendEventAll(sEvent, args);
    //logger::debug("{} sending event to papyrus to save ammo projectiles. Size of ammos = {}", __func__, allAmmos.size());
}

//called from DbSkseCppCallbackEvents papyrus script on init and game load
void SetDbSkseCppCallbackEventsAttached(RE::StaticFunctionTag*) {
    //DbSkseCppCallbackEventsAttached = true;
    //logger::trace("{} called", __func__);
    //SaveAllAmmoProjectilesFromPapyrus();
}

void DbSkseCppCallbackLoad() {
    //if (!IsScriptAttachedToRef(gfuncs::playerRef, "DbSkseCppCallbackEvents")) {
    //    logger::debug("{} attaching DbSkseCppCallbackEvents script to the player.", __func__);
    //    ExecuteConsoleCommand(nullptr, "player.aps DbSkseCppCallbackEvents", nullptr);
    //}
    //else {
    //    logger::debug("{} DbSkseCppCallbackEvents script already attached to the player", __func__);
    //}
}

//menu mode timer ===================================================================================================================================

void EraseFinishedMenuModeTimers();
bool erasingMenuModeTimers = false;
RE::BSFixedString sMenuModeTimerEvent = "OnTimerMenuMode";

struct MenuModeTimer {
    std::chrono::system_clock::time_point startTime;
    RE::VMHandle handle;
    int timerID;
    float interval;
    float savedTimeElapsed = 0.0;
    bool cancelled = false;
    bool finished = false;

    MenuModeTimer(RE::VMHandle akHandle, float afInterval, int aiTimerID, float afSavedTimeElapsed = 0.0) {
        handle = akHandle;
        timerID = aiTimerID;
        interval = afInterval;
        savedTimeElapsed = afSavedTimeElapsed;

        std::thread t([=]() {
            startTime = std::chrono::system_clock::now();

            int milliSecondInterval = (interval * 1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(milliSecondInterval));

            if (!cancelled) {
                float elapsedTime = (savedTimeElapsed + gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime));
                auto* args = RE::MakeFunctionArguments((int)timerID);
                gfuncs::svm->SendAndRelayEvent(handle, &sMenuModeTimerEvent, args, nullptr);
                delete args;
                logger::debug("menu mode timer event sent. ID[{}], interval[{}], elapsed time[{}]", timerID, interval, elapsedTime);
            }

            //while (erasingMenuModeTimers) { //only 1 thread should call EraseFinishedMenuModeTimers at a time.
            //    std::this_thread::sleep_for(std::chrono::milliseconds(150));
            //}

            finished = true;
            EraseFinishedMenuModeTimers();
            });
        t.detach();
    }

    float GetElapsedTime() {
        return (savedTimeElapsed + gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime));
    }

    float GetCurrentElapsedTime() { //for current - after loading a save startTime and interval are reset.
        return gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime);
    }

    float GetTimeLeft() {
        float timeLeft = (interval - GetCurrentElapsedTime());
        if (timeLeft < 0.0) {
            timeLeft = 0.0;
        }
        return (timeLeft);
    }
};

std::vector<MenuModeTimer*> currentMenuModeTimers;

void EraseFinishedMenuModeTimers_B() {
    if (currentMenuModeTimers.size() == 0) {
        return;
    }

    erasingMenuModeTimers = true;

    for (auto it = currentMenuModeTimers.begin(); it != currentMenuModeTimers.end(); ) {
        auto* timer = *it;
        if (timer) {
            if (timer->finished) {
                delete timer;
                timer = nullptr;
                it = currentMenuModeTimers.erase(it);
                logger::debug("erased menuModeTimer, Timers left = {}", currentMenuModeTimers.size());
            }
            else {
                ++it;
            }
        }
        else { //safety, this shouldn't occure.
            it = currentMenuModeTimers.erase(it);
        }
    }
    erasingMenuModeTimers = false;
}

void EraseFinishedMenuModeTimers() {
    if (currentMenuModeTimers.size() == 0) {
        return;
    }

    erasingMenuModeTimers = true;

    for (auto* timer : currentMenuModeTimers) {
        if (timer) {
            if (timer->finished) {
                gfuncs::RemoveFromVectorByValue(currentMenuModeTimers, timer);
                delete timer;
                timer = nullptr;
                logger::debug("erased menuModeTimer, Timers left = {}", currentMenuModeTimers.size());
            }
        }
        else {
            gfuncs::RemoveFromVectorByValue(currentMenuModeTimers, timer);
        }
    }
    erasingMenuModeTimers = false;
}

MenuModeTimer* GetTimer(std::vector<MenuModeTimer*>& v, RE::VMHandle akHandle, int aiTimerID) {
    if (v.size() == 0) {
        return nullptr;
    }

    for (auto* timer : v) {
        if (timer) {
            if (!timer->cancelled && !timer->finished) {
                if (timer->handle == akHandle && timer->timerID == aiTimerID) {
                    return timer;
                }
            }
        }
    }
    return nullptr;
}

bool SaveTimers(std::vector<MenuModeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    if (!a_intfc->OpenRecord(record, 1)) {
        logger::error("{}: MenuModeTimers Failed to open record[{}]", __func__, record);
        return false;
    }

    const std::size_t size = v.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: record[{}] Failed to write size of MenuModeTimers!", __func__, record);
        return false;
    }

    for (auto* timer : v) {
        if (timer) {
            float timeLeft = timer->GetTimeLeft();
            float timeElapsed = timer->GetElapsedTime();

            if (!a_intfc->WriteRecordData(timeLeft)) {
                logger::error("{}: record[{}] Failed to write time left", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timeElapsed)) {
                logger::error("{}: record[{}] Failed to write time elapsed", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->handle)) {
                logger::error("{}: record[{}] Failed to write handle[{}]", __func__, record, timer->handle);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->timerID)) {
                logger::error("{}: record[{}] Failed to write timerID[{}]", __func__, record, timer->timerID);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->cancelled)) {
                logger::error("{}: record[{}] Failed to write cancelled[{}]", __func__, record, timer->cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->finished)) {
                logger::error("{}: record[{}] Failed to write finished[{}]", __func__, record, timer->finished);
                return false;
            }

            logger::debug("MenuModeTimer saved : timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
                timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
        }
        else {
            float noTimerTimeLeft = -1.0;
            float noTimerTimeElapsed = 0.0;
            RE::VMHandle handle;
            int ID = -1;
            bool cancelled = true;
            bool finished = true;

            if (!a_intfc->WriteRecordData(noTimerTimeLeft)) {
                logger::error("{}: record[{}] Failed to write no Timer Time Left", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(noTimerTimeElapsed)) {
                logger::error("{}: record[{}] Failed to write no Timer Time Elapsed", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("{}: record[{}] Failed to write no timer handle", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(ID)) {
                logger::error("{}: record[{}] Failed to write no timer Id", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(cancelled)) {
                logger::error("{}: record[{}] Failed to write no timer cancelled[{}]", __func__, record, cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(finished)) {
                logger::error("{}: record[{}] Failed to write no timer finished[{}]", __func__, record, finished);
                return false;
            }
        }
    }
    return true;
}

bool loadTimers(std::vector<MenuModeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::size_t size;
    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record[{}] Failed to load size of MenuModeTimers!", __func__, record);
        return false;
    }

    if (v.size() > 0) { //cancel current timers
        for (auto* timer : v) {
            if (timer) {
                timer->cancelled = true;
            }
        }
    }

    for (std::size_t i = 0; i < size; i++) {
        float timeLeft = -1.0;
        float timeElapsed = 0.0;
        RE::VMHandle handle;
        int ID = -1;
        bool cancelled = false;
        bool finished = false;

        if (!a_intfc->ReadRecordData(timeLeft)) {
            logger::error("{}: {}: MenuModeTimer Failed to load timeLeft!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(timeElapsed)) {
            logger::error("{}: {}: MenuModeTimer Failed to load timeElapsed!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: {}: MenuModeTimer Failed to load handle!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(ID)) {
            logger::error("{}: {}: MenuModeTimer Failed to load ID!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(cancelled)) {
            logger::error("{}: {}: MenuModeTimer Failed to load cancelled!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(finished)) {
            logger::error("{}: {}: MenuModeTimer Failed to load finished!", __func__, i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: {}: MenuModeTimer warning, failed to resolve handle[{}]", __func__, i, handle);
            continue;
        }

        if (cancelled || finished) {
            continue;
        }

        MenuModeTimer* timer = new MenuModeTimer(handle, timeLeft, ID, timeElapsed);
        v.push_back(timer);

        logger::debug("MenuModeTimer loaded: timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
            timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
    }
    return true;
}

//NoMenuModeTimer =========================================================================================================================================
void EraseFinishedNoMenuModeTimers();
bool erasingNoMenuModeTimers = false;
RE::BSFixedString sNoMenuModeTimerEvent = "OnTimerNoMenuMode";

struct NoMenuModeTimer {
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point lastMenuCheck;
    bool lastMenuCheckSet = false;
    float initInterval;
    float currentInterval;
    float totalTimePaused;
    float savedTimeElapsed;
    RE::VMHandle handle;
    int timerID;
    bool cancelled = false;
    bool started = false;
    bool finished = false;
    bool canDelete = false;

    NoMenuModeTimer(RE::VMHandle akHandle, float afInterval, int aitimerID, float afSavedTimeElapsed = 0.0) {
        startTime = std::chrono::system_clock::now();
        initInterval = afInterval;
        currentInterval = afInterval;
        totalTimePaused = 0.0;
        savedTimeElapsed = afSavedTimeElapsed;
        handle = akHandle;
        timerID = aitimerID;
        if (!inMenuMode) {
            StartTimer();
        }
        else {
            lastMenuCheckSet = true;
            lastMenuCheck = std::chrono::system_clock::now();
        }
    }

    void StartTimer() {
        started = true;

        if (lastMenuCheckSet) {
            lastMenuCheckSet = false;
            totalTimePaused += gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), lastMenuCheck);
        }

        std::thread t([=]() {
            int milliSecondInterval = (currentInterval * 1000);
            currentInterval = 0.0;
            std::this_thread::sleep_for(std::chrono::milliseconds(milliSecondInterval));

            if (!cancelled) {
                if (inMenuMode) {
                    started = false;
                    float timeInMenu = gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), lastTimeMenuWasOpened);
                    currentInterval += timeInMenu;
                    totalTimePaused += timeInMenu;
                    lastMenuCheckSet = true;
                    lastMenuCheck = std::chrono::system_clock::now();
                }

                if (currentInterval <= 0.0 && !cancelled) {
                    float elapsedTime = (savedTimeElapsed + (gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime) - totalTimePaused));
                    auto* args = RE::MakeFunctionArguments((int)timerID);
                    gfuncs::svm->SendAndRelayEvent(handle, &sNoMenuModeTimerEvent, args, nullptr);
                    delete args;
                    logger::debug("NoMenuModeTimer event sent: timerID[{}] initInterval[{}] totalTimePaused[{}] elapsedTime[{}]",
                        timerID, initInterval, totalTimePaused, elapsedTime);

                    finished = true;
                }
                else if (!inMenuMode && !cancelled) {
                    StartTimer();
                }
                else if (!cancelled) {
                    started = false;
                }
            }

            if (cancelled || finished) {
                //while (erasingNoMenuModeTimers) { //only 1 thread should call EraseFinishedNoMenuModeTimers at a time.
                //    std::this_thread::sleep_for(std::chrono::milliseconds(150));
                //}

                canDelete = true;
                EraseFinishedNoMenuModeTimers();
            }
            });
        t.detach();
    }

    float GetElapsedTime() {
        return (savedTimeElapsed + (gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime) - totalTimePaused));
    }

    float GetCurrentElapsedTime() {
        return (gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime) - totalTimePaused);
    }

    float GetTimeLeft() {
        float timeLeft = initInterval - GetCurrentElapsedTime();
        if (timeLeft < 0.0) {
            timeLeft = 0.0;
        }
        return (timeLeft);
    }
};

std::vector<NoMenuModeTimer*> currentNoMenuModeTimers;

//called after menu close event if game not paused
void UpdateNoMenuModeTimers(float timeElapsedWhilePaused) {
    if (currentNoMenuModeTimers.size() == 0) {
        return;
    }

    for (auto* noMenuModeTimer : currentNoMenuModeTimers) {
        if (noMenuModeTimer) {
            if (!noMenuModeTimer->cancelled && !noMenuModeTimer->finished) {
                if (noMenuModeTimer->started) {
                    noMenuModeTimer->currentInterval += timeElapsedWhilePaused;
                    noMenuModeTimer->totalTimePaused += timeElapsedWhilePaused;
                }
                else {
                    noMenuModeTimer->StartTimer();
                }
            }
        }
    }
}

void EraseFinishedNoMenuModeTimers_B() {
    if (currentNoMenuModeTimers.size() == 0) {
        return;
    }

    erasingNoMenuModeTimers = true;

    for (auto it = currentNoMenuModeTimers.begin(); it != currentNoMenuModeTimers.end(); ) {
        auto* noMenuModeTimer = *it;
        if (noMenuModeTimer) {
            if (noMenuModeTimer->canDelete) {
                delete noMenuModeTimer;
                noMenuModeTimer = nullptr;
                it = currentNoMenuModeTimers.erase(it);
                logger::debug("erased NoMenuModeTimer, NoMenuModeTimers left = {}", currentNoMenuModeTimers.size());
            }
            else {
                ++it;
            }
        }
        else { //safety, this shouldn't occure.
            it = currentNoMenuModeTimers.erase(it);
        }
    }
    erasingNoMenuModeTimers = false;
}

void EraseFinishedNoMenuModeTimers() {
    if (currentNoMenuModeTimers.size() == 0) {
        return;
    }

    erasingNoMenuModeTimers = true;

    for (auto* timer : currentNoMenuModeTimers) {
        if (timer) {
            if (timer->canDelete) {
                gfuncs::RemoveFromVectorByValue(currentNoMenuModeTimers, timer);
                delete timer;
                timer = nullptr;
                logger::debug("erased NoMenuModeTimer, Timers left = {}", currentNoMenuModeTimers.size());
            }
        }
        else {
            gfuncs::RemoveFromVectorByValue(currentNoMenuModeTimers, timer);
        }
    }
    erasingNoMenuModeTimers = false;
}

NoMenuModeTimer* GetTimer(std::vector<NoMenuModeTimer*>& v, RE::VMHandle akHandle, int aiTimerID) {
    if (v.size() == 0) {
        return nullptr;
    }

    for (auto* noMenuModeTimer : v) {
        if (noMenuModeTimer) {
            if (!noMenuModeTimer->cancelled && !noMenuModeTimer->finished) {
                if (noMenuModeTimer->handle == akHandle && noMenuModeTimer->timerID == aiTimerID) {
                    return noMenuModeTimer;
                }
            }
        }
    }
    return nullptr;
}

bool SaveTimers(std::vector<NoMenuModeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    if (!a_intfc->OpenRecord(record, 1)) {
        logger::error("{}: MenuModeTimers Failed to open record[{}]", __func__, record);
        return false;
    }

    const std::size_t size = v.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: record[{}] Failed to write size of MenuModeTimers!", __func__, record);
        return false;
    }

    for (auto* timer : v) {
        if (timer) {
            float timeLeft = timer->GetTimeLeft();
            float timeElapsed = timer->GetElapsedTime();

            if (!a_intfc->WriteRecordData(timeLeft)) {
                logger::error("{}: record[{}] Failed to write time left", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timeElapsed)) {
                logger::error("{}: record[{}] Failed to write time elapsed", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->handle)) {
                logger::error("{}: record[{}] Failed to write handle[{}]", __func__, record, timer->handle);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->timerID)) {
                logger::error("{}: record[{}] Failed to write timerID[{}]", __func__, record, timer->timerID);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->cancelled)) {
                logger::error("{}: record[{}] Failed to write cancelled[{}]", __func__, record, timer->cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->finished)) {
                logger::error("{}: record[{}] Failed to write finished[{}]", __func__, record, timer->finished);
                return false;
            }

            logger::debug("NoMenuModeTimer saved : timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
                timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
        }
        else {
            float noTimerTimeLeft = -1.0;
            float noTimerTimeElapsed = 0.0;
            RE::VMHandle handle;
            int ID = -1;
            bool cancelled = true;
            bool finished = true;

            if (!a_intfc->WriteRecordData(noTimerTimeLeft)) {
                logger::error("{}: record[{}] Failed to write no Timer Time Left", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(noTimerTimeElapsed)) {
                logger::error("{}: record[{}] Failed to write no Timer Time Elapsed", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("{}: record[{}] Failed to write no timer handle", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(ID)) {
                logger::error("{}: record[{}] Failed to write no timer Id", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(cancelled)) {
                logger::error("{}: record[{}] Failed to write no timer cancelled[{}]", __func__, record, cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(finished)) {
                logger::error("{}: record[{}] Failed to write no timer finished[{}]", __func__, record, finished);
                return false;
            }
        }
    }
    return true;
}

bool loadTimers(std::vector<NoMenuModeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::size_t size;
    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record[{}] Failed to load size of MenuModeTimers!", __func__, record);
        return false;
    }

    if (v.size() > 0) { //cancel current timers
        for (auto* timer : v) {
            if (timer) {
                timer->cancelled = true;
            }
        }
    }

    for (std::size_t i = 0; i < size; i++) {
        float timeLeft = -1.0;
        float timeElapsed = 0.0;
        RE::VMHandle handle;
        int ID = -1;
        bool cancelled = false;
        bool finished = false;

        if (!a_intfc->ReadRecordData(timeLeft)) {
            logger::error("{}: {}: MenuModeTimer Failed to load timeLeft!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(timeElapsed)) {
            logger::error("{}: {}: MenuModeTimer Failed to load timeElapsed!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: {}: MenuModeTimer Failed to load handle!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(ID)) {
            logger::error("{}: {}: MenuModeTimer Failed to load ID!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(cancelled)) {
            logger::error("{}: {}: MenuModeTimer Failed to load cancelled!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(finished)) {
            logger::error("{}: {}: MenuModeTimer Failed to load finished!", __func__, i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: {}: MenuModeTimer warning, failed to resolve handle[{}]", __func__, i, handle);
            continue;
        }

        if (cancelled || finished) {
            continue;
        }

        NoMenuModeTimer* timer = new NoMenuModeTimer(handle, timeLeft, ID, timeElapsed);
        v.push_back(timer);

        logger::debug("NoMenuModeTimer loaded: timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
            timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
    }
    return true;
}

//timer ===================================================================================================================================

void EraseFinishedTimers();
bool erasingTimers = false;
RE::BSFixedString sTimerEvent = "OnTimer";

struct Timer {
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point lastPausedTimeCheck;
    bool lastPausedTimeCheckSet = false;
    float initInterval;
    float currentInterval;
    float totalTimePaused;
    float savedTimeElapsed;
    RE::VMHandle handle;
    int timerID;
    bool cancelled = false;
    bool started = false;
    bool finished = false;
    bool canDelete = false;

    Timer(RE::VMHandle akHandle, float afInterval, int aiTimerID, float afSavedTimeElapsed = 0.0) {
        startTime = std::chrono::system_clock::now();
        initInterval = afInterval;
        currentInterval = afInterval;
        totalTimePaused = 0.0;
        savedTimeElapsed = afSavedTimeElapsed;
        handle = akHandle;
        timerID = aiTimerID;
        if (!ui->GameIsPaused()) {
            StartTimer();
        }
        else {
            lastPausedTimeCheckSet = true;
            lastPausedTimeCheck = std::chrono::system_clock::now();
        }
    }

    void StartTimer() {
        started = true;

        if (lastPausedTimeCheckSet) {
            lastPausedTimeCheckSet = false;
            totalTimePaused += gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), lastPausedTimeCheck);
        }

        std::thread t([=]() {
            int milliSecondInterval = (currentInterval * 1000);
            currentInterval = 0.0;
            std::this_thread::sleep_for(std::chrono::milliseconds(milliSecondInterval));

            if (!cancelled) {
                if (ui->GameIsPaused()) {
                    started = false;
                    float timeInMenu = gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), lastTimeGameWasPaused);
                    currentInterval += timeInMenu;
                    totalTimePaused += timeInMenu;
                    lastPausedTimeCheckSet = true;
                    lastPausedTimeCheck = std::chrono::system_clock::now();
                }

                if (currentInterval <= 0.0 && !cancelled) {
                    float elapsedTime = (savedTimeElapsed + (gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime) - totalTimePaused));
                    auto* args = RE::MakeFunctionArguments((int)timerID);
                    gfuncs::svm->SendAndRelayEvent(handle, &sTimerEvent, args, nullptr);
                    delete args;
                    logger::debug("timer event sent: timerID[{}] initInterval[{}] totalTimePaused[{}] elapsedTime[{}]",
                        timerID, initInterval, totalTimePaused, elapsedTime);

                    finished = true;
                }
                else if (!ui->GameIsPaused() && !cancelled) {
                    StartTimer();
                }
                else if (!cancelled) {
                    started = false;
                }
            }

            if (cancelled || finished) {
                //while (erasingTimers) { //only 1 thread should call EraseFinishedTimers at a time.
                //    std::this_thread::sleep_for(std::chrono::milliseconds(150));
                //}

                canDelete = true;
                EraseFinishedTimers();
            }
            });
        t.detach();
    }

    float GetElapsedTime() {
        return (savedTimeElapsed + (gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime) - totalTimePaused));
    }

    float getCurrentElapsedTime() {
        return (gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime) - totalTimePaused);
    }

    float GetTimeLeft() {
        float timeLeft = initInterval - getCurrentElapsedTime();
        if (timeLeft < 0.0) {
            timeLeft = 0.0;
        }
        return (timeLeft);
    }

    void CancelTimer() {
        cancelled = true;
    }
};

std::vector<Timer*> currentTimers;

//called after menu close event if game not paused
void UpdateTimers(float timeElapsedWhilePaused) {
    if (currentTimers.size() == 0) {
        return;
    }

    for (auto* timer : currentTimers) {
        if (timer) {
            if (!timer->cancelled && !timer->finished) {
                if (timer->started) {
                    timer->currentInterval += timeElapsedWhilePaused;
                    timer->totalTimePaused += timeElapsedWhilePaused;
                }
                else {
                    timer->StartTimer();
                }
            }
        }
    }
}

void EraseFinishedTimers_B() {
    if (currentTimers.size() == 0) {
        return;
    }

    erasingTimers = true;

    for (auto it = currentTimers.begin(); it != currentTimers.end(); ) {
        auto* timer = *it;
        if (timer) {
            if (timer->canDelete) {
                delete timer;
                timer = nullptr;
                it = currentTimers.erase(it);
                logger::debug("erased Timer, Timers left = {}", currentTimers.size());
            }
            else {
                ++it;
            }
        }
        else { //safety, this shouldn't occure.
            it = currentTimers.erase(it);
        }
    }
    erasingTimers = false;
}

void EraseFinishedTimers() {
    if (currentTimers.size() == 0) {
        return;
    }

    erasingTimers = true;

    for (auto* timer : currentTimers) {
        if (timer) {
            if (timer->canDelete) {
                gfuncs::RemoveFromVectorByValue(currentTimers, timer);
                delete timer;
                timer = nullptr;
                logger::debug("erased timer, Timers left = {}", currentTimers.size());
            }
        }
        else {
            gfuncs::RemoveFromVectorByValue(currentTimers, timer);
        }
    }
    erasingTimers = false;
}

Timer* GetTimer(std::vector<Timer*>& v, RE::VMHandle akHandle, int aiTimerID) {
    if (v.size() == 0) {
        return nullptr;
    }

    for (auto* timer : v) {
        if (timer) {
            if (!timer->cancelled && !timer->finished) {
                if (timer->handle == akHandle && timer->timerID == aiTimerID) {
                    return timer;
                }
            }
        }
    }
    return nullptr;
}

bool SaveTimers(std::vector<Timer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    if (!a_intfc->OpenRecord(record, 1)) {
        logger::error("{}: MenuModeTimers Failed to open record[{}]", __func__, record);
        return false;
    }

    const std::size_t size = v.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: record[{}] Failed to write size of MenuModeTimers!", __func__, record);
        return false;
    }

    for (auto* timer : v) {
        if (timer) {
            float timeLeft = timer->GetTimeLeft();
            float timeElapsed = timer->GetElapsedTime();

            if (!a_intfc->WriteRecordData(timeLeft)) {
                logger::error("{}: record[{}] Failed to write time left", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timeElapsed)) {
                logger::error("{}: record[{}] Failed to write time elapsed", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->handle)) {
                logger::error("{}: record[{}] Failed to write handle[{}]", __func__, record, timer->handle);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->timerID)) {
                logger::error("{}: record[{}] Failed to write timerID[{}]", __func__, record, timer->timerID);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->cancelled)) {
                logger::error("{}: record[{}] Failed to write cancelled[{}]", __func__, record, timer->cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->finished)) {
                logger::error("{}: record[{}] Failed to write finished[{}]", __func__, record, timer->finished);
                return false;
            }

            logger::debug("Timer saved : timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
                timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
        }
        else {
            float noTimerTimeLeft = -1.0;
            float noTimerTimeElapsed = 0.0;
            RE::VMHandle handle;
            int ID = -1;
            bool cancelled = true;
            bool finished = true;

            if (!a_intfc->WriteRecordData(noTimerTimeLeft)) {
                logger::error("{}: record[{}] Failed to write no Timer Time Left", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(noTimerTimeElapsed)) {
                logger::error("{}: record[{}] Failed to write no Timer Time Elapsed", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("{}: record[{}] Failed to write no timer handle", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(ID)) {
                logger::error("{}: record[{}] Failed to write no timer Id", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(cancelled)) {
                logger::error("{}: record[{}] Failed to write no timer cancelled[{}]", __func__, record, cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(finished)) {
                logger::error("{}: record[{}] Failed to write no timer finished[{}]", __func__, record, finished);
                return false;
            }
        }
    }
    return true;
}

bool loadTimers(std::vector<Timer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::size_t size;
    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record[{}] Failed to load size of MenuModeTimers!", __func__, record);
        return false;
    }

    if (v.size() > 0) { //cancel current timers
        for (auto* timer : v) {
            if (timer) {
                timer->cancelled = true;
            }
        }
    }

    for (std::size_t i = 0; i < size; i++) {
        float timeLeft = -1.0;
        float timeElapsed = 0.0;
        RE::VMHandle handle;
        int ID = -1;
        bool cancelled = false;
        bool finished = false;

        if (!a_intfc->ReadRecordData(timeLeft)) {
            logger::error("{}: {}: MenuModeTimer Failed to load timeLeft!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(timeElapsed)) {
            logger::error("{}: {}: MenuModeTimer Failed to load timeElapsed!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: {}: MenuModeTimer Failed to load handle!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(ID)) {
            logger::error("{}: {}: MenuModeTimer Failed to load ID!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(cancelled)) {
            logger::error("{}: {}: MenuModeTimer Failed to load cancelled!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(finished)) {
            logger::error("{}: {}: MenuModeTimer Failed to load finished!", __func__, i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: {}: MenuModeTimer warning, failed to resolve handle[{}]", __func__, i, handle);
            continue;
        }

        if (cancelled || finished) {
            continue;
        }

        Timer* timer = new Timer(handle, timeLeft, ID, timeElapsed);
        v.push_back(timer);

        logger::debug("Timer loaded: timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
            timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
    }
    return true;
}

//GameTimeTimer =========================================================================================================================================

void EraseFinishedGameTimers();
bool erasingGameTimers = false;
RE::BSFixedString sGameTimeTimerEvent = "OnTimerGameTime";



struct GameTimeTimer {
    float startTime;
    float endTime;
    float initGameHoursInterval;
    int tick;
    int timerID;
    RE::VMHandle handle;
    bool cancelled = false;
    bool started = false;
    bool finished = false;
    bool canDelete = false;

    GameTimeTimer(RE::VMHandle akHandle, float afInterval, int aiTimerID, float afStartTime = -1.0, float afEndTime = -1.0) {
        handle = akHandle;
        timerID = aiTimerID;
        initGameHoursInterval = afInterval;

        tick = gameTimerPollingInterval;

        if (afStartTime != -1.0) { //timer created from LoadTimers on game load.
            startTime = afStartTime;
            endTime = afEndTime;
        }
        else {
            startTime = calendar->GetHoursPassed();
            endTime = startTime + afInterval;
        }

        float secondsInterval = GameHoursToRealTimeSeconds(nullptr, afInterval);
        int milliTick = (secondsInterval * 1000);

        if (tick > milliTick) {
            tick = milliTick;
        }

        StartTimer();
    }

    void StartTimer() {
        std::thread t([=]() {
            while (calendar->GetHoursPassed() < endTime && !cancelled) {
                std::this_thread::sleep_for(std::chrono::milliseconds(tick));
            }

            if (!cancelled) {
                auto now = std::chrono::system_clock::now();
                float endTime = calendar->GetHoursPassed();
                float elapsedGameHours = (calendar->GetHoursPassed() - startTime);

                auto* args = RE::MakeFunctionArguments((int)timerID);
                gfuncs::svm->SendAndRelayEvent(handle, &sGameTimeTimerEvent, args, nullptr);
                delete args;
                logger::debug("game timer event sent. ID[{}] gameHoursInterval[{}] startTime[{}] endTime[{}] elapsedGameHours[{}]",
                    timerID, initGameHoursInterval, startTime, endTime, elapsedGameHours);

                finished = true;
            }

            if (cancelled || finished) {
                //while (erasingGameTimers) { //only 1 thread should call EraseFinishedTimers at a time.
                //    std::this_thread::sleep_for(std::chrono::milliseconds(150));
                //}

                canDelete = true;
                EraseFinishedGameTimers();
            }
            });
        t.detach();
    }

    float GetElapsedTime() {
        return (calendar->GetHoursPassed() - startTime);
    }

    float GetTimeLeft() {
        return (endTime - calendar->GetHoursPassed());
    }

    void CancelTimer() {
        cancelled = true;
    }
};

std::vector<GameTimeTimer*> currentGameTimeTimers;

void EraseFinishedGameTimers_B() {
    if (currentGameTimeTimers.size() == 0) {
        return;
    }

    erasingGameTimers = true;

    for (auto it = currentGameTimeTimers.begin(); it != currentGameTimeTimers.end(); ) {
        auto* timer = *it;
        if (timer) {
            if (timer->canDelete) {
                delete timer;
                timer = nullptr;
                it = currentGameTimeTimers.erase(it);
                logger::debug("erased gameTimer, Timers left = {}", currentGameTimeTimers.size());
            }
            else {
                ++it;
            }
        }
        else { //safety, this shouldn't occure.
            it = currentGameTimeTimers.erase(it);
        }
    }
    erasingGameTimers = false;
}

void EraseFinishedGameTimers() {
    if (currentGameTimeTimers.size() == 0) {
        return;
    }

    erasingGameTimers = true;

    for (auto* timer : currentGameTimeTimers) {
        if (timer) {
            if (timer->canDelete) {
                gfuncs::RemoveFromVectorByValue(currentGameTimeTimers, timer);
                delete timer;
                timer = nullptr;
                logger::debug("erased gameTimer, Timers left = {}", currentGameTimeTimers.size());
            }
        }
        else {
            gfuncs::RemoveFromVectorByValue(currentGameTimeTimers, timer);
        }
    }
    erasingGameTimers = false;
}

GameTimeTimer* GetTimer(std::vector<GameTimeTimer*>& v, RE::VMHandle akHandle, int aiTimerID) {
    if (v.size() == 0) {
        return nullptr;
    }

    for (auto* timer : v) {
        if (timer) {
            if (!timer->cancelled && !timer->finished) {
                if (timer->handle == akHandle && timer->timerID == aiTimerID) {
                    return timer;
                }
            }
        }
    }
    return nullptr;
}

bool SaveTimers(std::vector<GameTimeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    if (!a_intfc->OpenRecord(record, 1)) {
        logger::error("{}: MenuModeTimers Failed to open record[{}]", __func__, record);
        return false;
    }

    const std::size_t size = v.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("{}: record[{}] Failed to write size of MenuModeTimers!", __func__, record);
        return false;
    }

    for (auto* timer : v) {
        if (timer) {

            if (!a_intfc->WriteRecordData(timer->startTime)) {
                logger::error("{}: record[{}] Failed to write startTime", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->endTime)) {
                logger::error("{}: record[{}] Failed to write endTime", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->initGameHoursInterval)) {
                logger::error("{}: record[{}] Failed to write initGameHoursInterval", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->handle)) {
                logger::error("{}: record[{}] Failed to write handle[{}]", __func__, record, timer->handle);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->timerID)) {
                logger::error("{}: record[{}] Failed to write timerID[{}]", __func__, record, timer->timerID);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->cancelled)) {
                logger::error("{}: record[{}] Failed to write cancelled[{}]", __func__, record, timer->cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->finished)) {
                logger::error("{}: record[{}] Failed to write finished[{}]", __func__, record, timer->finished);
                return false;
            }

            logger::debug("gameTimer saved : startTime[{}], endTime[{}], initGameHoursInterval[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
                timer->startTime, timer->endTime, timer->initGameHoursInterval, timer->handle, timer->timerID, timer->cancelled, timer->finished);
        }
        else {
            float startTime = 0.0;
            float endTime = 0.0;
            float initGameHoursInterval = 0.1;
            RE::VMHandle handle;
            int ID = -1;
            bool cancelled = true;
            bool finished = true;

            if (!a_intfc->WriteRecordData(startTime)) {
                logger::error("{}: record[{}] Failed to write no Timer startTime", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(endTime)) {
                logger::error("{}: record[{}] Failed to write no Timer endTime", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(initGameHoursInterval)) {
                logger::error("{}: record[{}] Failed to write no Timer initGameHoursInterval", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("{}: record[{}] Failed to write no timer handle", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(ID)) {
                logger::error("{}: record[{}] Failed to write no timer Id", __func__, record);
                return false;
            }

            if (!a_intfc->WriteRecordData(cancelled)) {
                logger::error("{}: record[{}] Failed to write no timer cancelled[{}]", __func__, record, cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(finished)) {
                logger::error("{}: record[{}] Failed to write no timer finished[{}]", __func__, record, finished);
                return false;
            }
        }
    }
    return true;
}

bool loadTimers(std::vector<GameTimeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::size_t size;
    if (!a_intfc->ReadRecordData(size)) {
        logger::error("{}: record[{}] Failed to load size of MenuModeTimers!", __func__, record);
        return false;
    }

    if (v.size() > 0) { //cancel current timers
        for (auto* timer : v) {
            if (timer) {
                timer->cancelled = true;
            }
        }
    }

    for (std::size_t i = 0; i < size; i++) {
        float startTime = 0.0;
        float endTime = 0.0;
        float initGameHoursInterval = 0.1;
        RE::VMHandle handle;
        int ID = -1;
        bool cancelled = true;
        bool finished = true;

        if (!a_intfc->ReadRecordData(startTime)) {
            logger::error("{}: {}: MenuModeTimer Failed to load startTime!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(endTime)) {
            logger::error("{}: {}: MenuModeTimer Failed to load endTime!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(initGameHoursInterval)) {
            logger::error("{}: {}: MenuModeTimer Failed to load initGameHoursInterval!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: {}: MenuModeTimer Failed to load handle!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(ID)) {
            logger::error("{}: {}: MenuModeTimer Failed to load ID!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(cancelled)) {
            logger::error("{}: {}: MenuModeTimer Failed to load cancelled!", __func__, i);
            return false;
        }

        if (!a_intfc->ReadRecordData(finished)) {
            logger::error("{}: {}: MenuModeTimer Failed to load finished!", __func__, i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: {}: MenuModeTimer warning, failed to resolve handle[{}]", __func__, i, handle);
            continue;
        }

        if (cancelled || finished) {
            continue;
        }

        GameTimeTimer* timer = new GameTimeTimer(handle, initGameHoursInterval, ID, startTime, endTime);
        v.push_back(timer);

        logger::debug("gameTimer loaded: startTime[{}], endTime[{}], initGameHoursInterval[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
            timer->startTime, timer->endTime, timer->initGameHoursInterval, timer->handle, timer->timerID, timer->cancelled, timer->finished);
    }
    return true;
}

//timer papyrus functions==============================================================================================================================================

//forms ==============================================================================================================================================

//menuModeTimer=======================================================================================================================
void StartMenuModeTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
        logger::debug("{} reset timer on form [{}] ID[{:x}] timerID[{}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID(), aiTimerID);
    }

    MenuModeTimer* newTimer = new MenuModeTimer(handle, afInterval, aiTimerID);
    currentMenuModeTimers.push_back(newTimer);
}

void CancelMenuModeTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("{} called, ID {}", __func__, aiTimerID);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
}

float GetTimeElapsedOnMenuModeTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnMenuModeTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//NoMenuModeTimer=======================================================================================================================
void StartNoMenuModeTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("{} called, time: {}", __func__, afInterval);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    NoMenuModeTimer* newTimer = new NoMenuModeTimer(handle, afInterval, aiTimerID);
    currentNoMenuModeTimers.push_back(newTimer);
}

void CancelNoMenuModeTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnNoMenuModeTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnNoMenuModeTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer=======================================================================================================================
void StartTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("{} called, time: {}", __func__, afInterval);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    Timer* newTimer = new Timer(handle, afInterval, aiTimerID);
    currentTimers.push_back(newTimer);
}

void CancelTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//gametime timer===========================================================================================================================
void StartGameTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    GameTimeTimer* newTimer = new GameTimeTimer(handle, afInterval, aiTimerID);
    currentGameTimeTimers.push_back(newTimer);
}

void CancelGameTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnGameTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnGameTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer Alias==============================================================================================================================================

//menu Mode Timer alias=======================================================================================================================
void StartMenuModeTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
        logger::debug("{} reset timer on Alias [{}] ID[{:x}] timerID[{}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID, aiTimerID);
    }

    MenuModeTimer* newTimer = new MenuModeTimer(handle, afInterval, aiTimerID);
    currentMenuModeTimers.push_back(newTimer);
}

void CancelMenuModeTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("{} called, ID {}", __func__, aiTimerID);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnMenuModeTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnMenuModeTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//NoMenuModeTimer Alias=======================================================================================================================
void StartNoMenuModeTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("{} called, time: {}", __func__, afInterval);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    NoMenuModeTimer* newTimer = new NoMenuModeTimer(handle, afInterval, aiTimerID);
    currentNoMenuModeTimers.push_back(newTimer);
}

void CancelNoMenuModeTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnNoMenuModeTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnNoMenuModeTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer alias=======================================================================================================================
void StartTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    Timer* newTimer = new Timer(handle, afInterval, aiTimerID);
    currentTimers.push_back(newTimer);
}

void CancelTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//gametime timer alias===========================================================================================================================
void StartGameTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    GameTimeTimer* newTimer = new GameTimeTimer(handle, afInterval, aiTimerID);
    currentGameTimeTimers.push_back(newTimer);
}

void CancelGameTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnGameTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnGameTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for Alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer ActiveMagicEffect==============================================================================================================================================

//menu Mode Timer ActiveMagicEffect=======================================================================================================================
void StartMenuModeTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
        logger::debug("{} reset timer on ActiveMagicEffect [{}] ID[{:x}] timerID[{}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID, aiTimerID);
    }

    MenuModeTimer* newTimer = new MenuModeTimer(handle, afInterval, aiTimerID);
    currentMenuModeTimers.push_back(newTimer);
}

void CancelMenuModeTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("{} called, ID {}", __func__, aiTimerID);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnMenuModeTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}


float GetTimeLeftOnMenuModeTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//NoMenuModeTimer ActiveMagicEffect=======================================================================================================================
void StartNoMenuModeTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("{} called, time: {}", __func__, afInterval);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    NoMenuModeTimer* newTimer = new NoMenuModeTimer(handle, afInterval, aiTimerID);
    currentNoMenuModeTimers.push_back(newTimer);
}

void CancelNoMenuModeTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnNoMenuModeTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnNoMenuModeTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer ActiveMagicEffect=======================================================================================================================
void StartTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    Timer* newTimer = new Timer(handle, afInterval, aiTimerID);
    currentTimers.push_back(newTimer);
}

void CancelTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//gametime timer ActiveMagicEffect===========================================================================================================================
void StartGameTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    GameTimeTimer* newTimer = new GameTimeTimer(handle, afInterval, aiTimerID);
    currentGameTimeTimers.push_back(newTimer);
}

void CancelGameTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnGameTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnGameTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("{} called", __func__);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//Papyrus Events =============================================================================================================================

enum EventsEnum {
    EventEnum_OnLoadGame,
    EventEnum_OnCombatStateChanged,
    EventEnum_FurnitureEnter,
    EventEnum_FurnitureExit,
    EventEnum_OnActivate,
    EventEnum_HitEvent,
    EventEnum_DeathEvent,
    EventEnum_DyingEvent,
    EventEnum_OnObjectEquipped,
    EventEnum_OnObjectUnequipped,
    EventEnum_OnWaitStart,
    EventEnum_OnWaitStop,
    EventEnum_OnMagicEffectApply,
    EventEnum_OnSpellCast,
    EventEnum_LockChanged,
    EventEnum_OnOpen,
    EventEnum_OnClose,
    EventEnum_OnWeaponSwing,
    EventEnum_OnActorSpellCast,
    EventEnum_OnActorSpellFire,
    EventEnum_VoiceCast,
    EventEnum_VoiceFire,
    EventEnum_BowDraw,
    EventEnum_BowRelease,
    EventEnum_BeginDraw,
    EventEnum_EndDraw,
    EventEnum_BeginSheathe,
    EventEnum_EndSheathe,
    EventEnum_OnContainerChanged,
    EventEnum_OnProjectileImpact,
    EventEnum_OnItemCrafted,
    EventEnum_OnItemsPickpocketed,
    EventEnum_OnLocationCleared,
    EventEnum_OnEnterBleedout,
    EventEnum_OnSwitchRaceComplete,
    EventEnum_OnActorFootStep,
    EventEnum_OnQuestObjectiveStateChanged,
    EventEnum_OnPositionPlayerStart,
    EventEnum_OnPositionPlayerFinish,
    EventEnum_OnPlayerChangeCell,
    EventEnum_First = EventEnum_OnLoadGame,
    EventEnum_Last = EventEnum_OnPlayerChangeCell
};

struct EventData {
    int eventSinkIndex;
    bool sinkAdded = false;
    RE::BSFixedString sEvent;
    std::uint32_t record;

    std::vector<RE::VMHandle> globalHandles; //handles that receive all events
    std::vector< std::map<RE::TESForm*, std::vector<RE::VMHandle>> > eventParamMaps; //event param form comparisons

    //constructor
    EventData(RE::BSFixedString event, int ceventSinkIndex, int NumberOfParams, std::uint32_t crecord) :
        eventSinkIndex(ceventSinkIndex),
        sEvent(event),
        record(crecord)
    {
        eventParamMaps.resize(NumberOfParams);
    }

    bool PlayerIsRegistered() {
        int max = eventParamMaps.size();
        for (int i = 0; i < max; i++) {
            auto it = eventParamMaps[i].find(gfuncs::playerRef);
            if (it != eventParamMaps[i].end()) {
                return true;
            }
        }
        return false;
    }

    bool isEmpty() {
        for (auto formMapHandle : eventParamMaps) {
            if (formMapHandle.size() > 0) {
                return false;
            }
        }
        return (globalHandles.size() == 0);
    }

    void removeSinkIfEmpty() {
        if (isEmpty()) {
            RemoveSink(eventSinkIndex);
        }
        if (eventSinkIndex == EventEnum_OnCombatStateChanged) {
            if (bRegisteredForPlayerCombatChange) {
                if (!PlayerIsRegistered()) {
                    bRegisteredForPlayerCombatChange = false;
                }
            }
        }
    }

    void InsertIntoFormHandles(RE::VMHandle handle, RE::TESForm* akForm, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& eventFormHandles) {
        if (gfuncs::IsFormValid(akForm)) {
            auto it = eventFormHandles.find(akForm);
            if (it != eventFormHandles.end()) { //form found in param map
                int handleIndex = gfuncs::GetIndexInVector(it->second, handle);
                if (handleIndex == -1) { //handle not already added for this form param
                    it->second.push_back(handle);
                    logger::debug("{}: akForm[{}] ID[{:x}] found, handles sixe[{}]", __func__, gfuncs::GetFormName(akForm), akForm->GetFormID(), it->second.size());
                }
            }
            else { //form not found
                std::vector<RE::VMHandle> handles;
                handles.push_back(handle);
                eventFormHandles.insert(std::pair<RE::TESForm*, std::vector<RE::VMHandle>>(akForm, handles));

                logger::debug("{}: akForm[{}] ID[{:x}] doesn't already have handles, handles size[{}] eventFormHandles size[{}]", __func__, gfuncs::GetFormName(akForm), akForm->GetFormID(), handles.size(), eventFormHandles.size());
            }
        }
    }

    void EraseFromFormHandles(RE::VMHandle handle, RE::TESForm* akForm, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& eventFormHandles) {
        if (gfuncs::IsFormValid(akForm)) {
            auto it = eventFormHandles.find(akForm);
            if (it != eventFormHandles.end()) { //form found in param map
                int handleIndex = gfuncs::GetIndexInVector(it->second, handle);
                if (handleIndex != -1) { //handle not already added for this form (activator param)
                    auto itb = it->second.begin() + handleIndex;
                    it->second.erase(itb);

                    if (it->second.size() == 0) { //no more handles for this akForm
                        eventFormHandles.erase(akForm);
                    }
                }
            }
        }
    }

    //erase all instances of handle
    void EraseFromFormHandles(RE::VMHandle handle, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& eventFormHandles) { //erase all instances of handle
        for (auto it : eventFormHandles) {
            int handleIndex = gfuncs::GetIndexInVector(it.second, handle);
            if (handleIndex != -1) {
                auto itb = it.second.begin() + handleIndex;
                it.second.erase(itb);

                if (it.second.size() == 0) { //no more handles for this akForm
                    eventFormHandles.erase(it.first);
                }
            }
        }
    }

    void AddHandle(RE::VMHandle handle, RE::TESForm* paramFilter, int paramFilterIndex) {
        if (!paramFilter) {
            globalHandles.push_back(handle);

            if (eventSinkIndex == EventEnum_OnCombatStateChanged) {
                bRegisteredForPlayerCombatChange = true;
            }
        }
        else {
            int index = paramFilterIndex;
            if (index >= eventParamMaps.size()) {
                index = eventParamMaps.size() - 1;
            }
            if (index < 0) {
                index = 0;
            }
            InsertIntoFormHandles(handle, paramFilter, eventParamMaps[paramFilterIndex]);

            if (eventSinkIndex == EventEnum_OnCombatStateChanged && paramFilterIndex == 0) {
                logger::debug("{}: adding handle for Combat State change", __func__);
                if (paramFilter->As<RE::Actor>() == gfuncs::playerRef) {
                    //playerForm = paramFilter;
                    bRegisteredForPlayerCombatChange = true;
                    logger::debug("{}: bRegisteredForPlayerCombatChange = true", __func__);
                }
            }
        }
    }

    void CheckAndRemoveSink() {
        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1500)); //wait for 1.5 seconds before checking removeSinkIfEmpty
            removeSinkIfEmpty();

            });
        t.detach();
    }

    void RemoveHandle(RE::VMHandle handle, RE::TESForm* paramFilter, int paramFilterIndex, bool removeSink = true) {
        int gIndex = gfuncs::GetIndexInVector(globalHandles, handle);

        if (!paramFilter) {
            if (gIndex != -1) {
                auto it = globalHandles.begin() + gIndex;
                globalHandles.erase(it);
            }
        }
        else {
            int index = paramFilterIndex;
            if (index >= eventParamMaps.size()) {
                index = eventParamMaps.size() - 1;
            }
            if (index < 0) {
                index = 0;
            }
            logger::debug("{}: eventSinkIndex [{}] Index = [{}]", __func__, eventSinkIndex, index);
            EraseFromFormHandles(handle, paramFilter, eventParamMaps[paramFilterIndex]);
        }

        if (removeSink) {
            CheckAndRemoveSink();
        }
    }

    void RemoveAllHandles(RE::VMHandle handle) {
        int gIndex = gfuncs::GetIndexInVector(globalHandles, handle);
        if (gIndex != -1) {
            auto it = globalHandles.begin() + gIndex;
            globalHandles.erase(it);
        }

        int max = eventParamMaps.size();
        for (int i = 0; i < max; i++) {
            EraseFromFormHandles(handle, eventParamMaps[i]);
        }
    }

    bool HasHandle(RE::VMHandle handle, RE::TESForm* paramFilter, int paramFilterIndex) {
        if (!paramFilter) {
            int gIndex = gfuncs::GetIndexInVector(globalHandles, handle);
            return (gIndex != -1);
        }

        int index = paramFilterIndex;
        if (index >= eventParamMaps.size()) {
            index = eventParamMaps.size() - 1;
        }
        if (index < 0) {
            index = 0;
        }

        auto it = eventParamMaps[index].find(paramFilter);
        if (it != eventParamMaps[index].end()) {
            return (gfuncs::GetIndexInVector(it->second, handle) != -1);
        }
        return false;
    }

    bool Load(SKSE::SerializationInterface* a_intfc) {
        if (!LoadHandlesVector(globalHandles, record, a_intfc)) {
            return false;
        }

        std::size_t size;
        if (!a_intfc->ReadRecordData(size)) {
            logger::error("{}: Event[{}] Failed to load size of eventParamMaps!", __func__, sEvent);
            return false;
        }

        for (std::size_t i = 0; i < size; i++) {
            if (!LoadFormHandlesMap(eventParamMaps[i], record, a_intfc)) {
                return false;
            }
        }
        return true;
    }

    bool Save(SKSE::SerializationInterface* a_intfc) {
        if (!a_intfc->OpenRecord(record, 1)) {
            logger::error("{}: Failed to open record[{}]", __func__, record);
            return false;
        }

        if (!SaveHandlesVector(globalHandles, record, a_intfc)) {
            return false;
        }

        const std::size_t size = eventParamMaps.size();

        if (!a_intfc->WriteRecordData(size)) {
            logger::error("{}: record[{}] Failed to write size of arr!", __func__, record);
            return false;
        }

        for (std::size_t i = 0; i < size; i++) {
            if (!SaveFormHandlesMap(eventParamMaps[i], record, a_intfc)) {
                return false;
            }
        }
        return true;
    }
};

std::vector<EventData*> eventDataPtrs;

//one event sink per actor
//track ammo
struct AnimationEventSink : public RE::BSTEventSink<RE::BSAnimationGraphEvent> {
    bool sinkAdded = false;
    RE::Actor* actor; //actor that's registered for animation events
    bool isCleaningTrackedActorAmmoDataVector = false;
    std::vector<TrackedActorAmmoData> forceChangedAmmos;
    RE::TESAmmo* lastDrawnAmmo;
    RE::TESAmmo* lastHitAmmo;
    RE::TESObjectREFR* lastHitTarget;
    std::chrono::system_clock::time_point lastHitTime;
    std::chrono::system_clock::time_point lastReleaseTime;
    float lastReleaseGameTime;

    RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* source) {
        if (!gfuncs::IsFormValid(actor)) {
            source->RemoveEventSink(this);
            return RE::BSEventNotifyControl::kContinue;
        }

        if (!event) {
            logger::error("AnimationEventSink event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("AnimationEventSink IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (event->tag == "BowRelease") {
            lastReleaseTime = std::chrono::system_clock::now();
            lastReleaseGameTime = calendar->GetHoursPassed();
        }
        else if (event->tag == "bowDraw") {
            lastDrawnAmmo = actor->GetCurrentAmmo();
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

std::map<RE::TESObjectREFR*, AnimationEventSink*> animationEventActorsMap;

int GetImpactResultInt(RE::ImpactResult& result) {
    switch (result) {

    case RE::ImpactResult::kDestroy:
        return 1;
    case RE::ImpactResult::kBounce:
        return 2;
    case RE::ImpactResult::kImpale:
        return 3;
    case RE::ImpactResult::kStick:
        return 4;
    default:
        return 0;
    }
}

void SaveRecentProjectile(RE::Projectile* projectile, RE::TESObjectREFR* shooter, RE::TESObjectREFR* target, RE::TESAmmo* ammo,
    float& gameTime, RE::BGSProjectile* projectileBase, int& impactResult, int& collidedLayer, float& distanceTraveled,
    std::string hitPartNodeName, RE::TESObjectREFR* projectileMarker/*, uint32_t& runTime, std::chrono::system_clock::time_point& timePoint*/) {

    /*RE::TESObjectREFR* targetMarker = nullptr;

    if (gfuncs::IsFormValid(target)) {
        if (xMarker) {
            auto refPtr = target->PlaceObjectAtMe(xMarker->As<RE::TESBoundObject>(), false);
            if (refPtr) {
                targetMarker = refPtr.get();
            }
        }
    }*/

    TrackedProjectileData data;
    data.gameTimeStamp = gameTime;
    data.lastImpactEventGameTimeStamp = 0.0;
    //data.runTimeStamp = runTime;
    //data.timeStamp = timePoint;
    data.projectile = projectile;
    data.shooter = shooter;
    data.target = target;
    data.ammo = ammo;
    data.projectileBase = projectileBase;
    data.impactResult = impactResult;
    data.collidedLayer = collidedLayer;
    data.distanceTraveled = distanceTraveled;
    data.hitPartNodeName = hitPartNodeName;
    data.projectileMarker = projectileMarker;
    //data.targetMarker = targetMarker;

    auto it = recentShotProjectiles.find(shooter);
    if (it != recentShotProjectiles.end()) {
        it->second.push_back(data);
        if (it->second.size() > iMaxArrowsSavedPerReference) {
            it->second.erase(it->second.begin());
        }
    }
    else {
        std::vector<TrackedProjectileData> datas;
        datas.push_back(data);
        recentShotProjectiles.insert(std::pair<RE::TESObjectREFR*, std::vector<TrackedProjectileData>>(shooter, datas));
    }

    auto itt = recentHitProjectiles.find(target);
    if (itt != recentHitProjectiles.end()) {
        itt->second.push_back(data);
        if (itt->second.size() > iMaxArrowsSavedPerReference) {
            itt->second.erase(itt->second.begin());
        }
    }
    else {
        std::vector<TrackedProjectileData> datas;
        datas.push_back(data);
        recentHitProjectiles.insert(std::pair<RE::TESObjectREFR*, std::vector<TrackedProjectileData>>(target, datas));
    }

    //SendProjectileImpactEvent(data);
}

bool projectileImpacthookInstalled = false;

struct ProjectileImpactHook
{
    static bool thunk(RE::Projectile* projectile)
    {
        if (gfuncs::IsFormValid(projectile)) {
            float gameHoursPassed = calendar->GetHoursPassed();

            //uint32_t runTime = RE::GetDurationOfApplicationRunTime();
            //auto now = std::chrono::system_clock::now();

            //return projectile->Unk_B8();
            bool killOnCollision = projectile->GetKillOnCollision();

            if (iMaxArrowsSavedPerReference <= 0) {
                return killOnCollision;
            }

            RE::TESObjectREFR* projectileMarker = nullptr;

            if (eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded) {
                if (xMarker) {
                    auto refPtr = projectile->PlaceObjectAtMe(xMarker->As<RE::TESBoundObject>(), false);
                    if (refPtr) {
                        projectileMarker = refPtr.get();
                    }
                }
            }
            //logger::trace("impact event: getting runtimeData");
            auto& runtimeData = projectile->GetProjectileRuntimeData();

            //auto niTransform = runtimeData.unk0A8;
            //RE::NiPoint3 desiredTargetPoint;

            RE::BGSProjectile* projectileBase = projectile->GetProjectileBase();
            int impactResult = GetProjectileImpactResult(nullptr, projectile);

            RE::TESObjectREFR* shooter = GetProjectileShooterFromRuntimeData(runtimeData);
            RE::TESObjectREFR* target;
            RE::TESAmmo* ammo = runtimeData.ammoSource;
            float distanceTraveled = runtimeData.distanceMoved;
            std::string hitPartNodeName;
            int collidedLayer = 0;

            //for (auto* impactData : runtimeData.impacts) {
            if (!runtimeData.impacts.empty()) {
                auto* impactData = *runtimeData.impacts.begin();
                if (impactData) {

                    //desiredTargetPoint = impactData->desiredTargetLoc;

                    if (impactData->collidee) {
                        auto hitRefHandle = impactData->collidee;
                        if (hitRefHandle) {
                            //logger::trace("hitRef found");
                            auto hitRefPtr = hitRefHandle.get();
                            if (hitRefPtr) {
                                target = hitRefPtr.get();
                            }
                        }
                    }
                    RE::NiNode* hitPart = impactData->damageRootNode;
                    if (hitPart) {
                        hitPartNodeName = hitPart->name;
                        if (hitPart->parent) {
                            if (hitPart->parent->name == "SHIELD" || hitPartNodeName == "") {
                                hitPartNodeName = static_cast<std::string>(hitPart->parent->name);
                            }
                        }
                    }
                    collidedLayer = impactData->collidedLayer.underlying();
                }
            }

            SaveRecentProjectile(projectile, shooter, target, ammo, gameHoursPassed, projectileBase, impactResult, collidedLayer, distanceTraveled, hitPartNodeName, projectileMarker/*, runTime, now*/);

            return killOnCollision;
        }
        else {
            return false;
        }
    }

    static REL::Relocation<uintptr_t> GetRelocationID(REL::Version a_ver) {
        return a_ver <= SKSE::RUNTIME_SSE_1_5_97 ?
            REL::Relocation<uintptr_t>(REL::ID(43013), 0x3E3) : // SkyrimSE.exe+0x7521f0+0x3E3
            REL::Relocation<uintptr_t>(REL::ID(44204), 0x3D4); // SkyrimSE.exe+0x780870+0x3D4
    }

    static inline REL::Relocation<decltype(thunk)> func;

    static bool Check(REL::Version a_ver)
    {
        auto hook = GetRelocationID(a_ver);
        if (*(uint16_t*)hook.address() != 0x90FF)
        {
            logger::critical("ProjectileImpactHook check: Opcode at injection site not matched. Aborting...");
            return false;
        }

        return true;
    }

    static bool Install(REL::Version a_ver)
    {
        if (Check(a_ver)) {
            // 44204+0x3EC 0x780870
            if (!projectileImpacthookInstalled) {
                projectileImpacthookInstalled = true;
                auto hook = GetRelocationID(a_ver);
                REL::safe_fill(hook.address(), 0x90, 6);
                stl::write_thunk_call<ProjectileImpactHook>(hook.address());
                return true;
            }
            else {
                return false;
            }
        }
        return false;
    }
};

void SendProjectileImpactEvent(TrackedProjectileData& data, RE::TESForm* source, bool SneakAttack, bool HitBlocked, float currentGameTime, std::vector<float>& hitTranslation) {
    if (!eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded) {
        return;
    }

    if (GameHoursToRealTimeSeconds(nullptr, (currentGameTime - data.lastImpactEventGameTimeStamp)) < 0.1) {
        return; //event for this shooter and target was sent less than 0.1 seconds ago. Skip
    }

    data.lastImpactEventGameTimeStamp = currentGameTime;

    std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnProjectileImpact]->globalHandles;

    CombineEventHandles(handles, data.shooter, eventDataPtrs[EventEnum_OnProjectileImpact]->eventParamMaps[0]);
    CombineEventHandles(handles, data.target, eventDataPtrs[EventEnum_OnProjectileImpact]->eventParamMaps[1]);
    CombineEventHandles(handles, source, eventDataPtrs[EventEnum_OnProjectileImpact]->eventParamMaps[2]);
    CombineEventHandles(handles, data.ammo, eventDataPtrs[EventEnum_OnProjectileImpact]->eventParamMaps[3]);
    CombineEventHandles(handles, data.projectileBase, eventDataPtrs[EventEnum_OnProjectileImpact]->eventParamMaps[3]);

    gfuncs::RemoveDuplicates(handles);

    auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)data.shooter, (RE::TESObjectREFR*)data.target, (RE::TESForm*)source,
        (RE::TESAmmo*)data.ammo, (RE::BGSProjectile*)data.projectileBase, (bool)SneakAttack, (bool)HitBlocked,
        (int)data.impactResult, (int)data.collidedLayer, (float)data.distanceTraveled, (std::string)data.hitPartNodeName,
        (RE::TESObjectREFR*)data.projectileMarker, (std::vector<float>)hitTranslation);

    SendEvents(handles, eventDataPtrs[EventEnum_OnProjectileImpact]->sEvent, args);

    logger::trace("projectile impact: shooter[{}] target[{}] source[{}] ammo[{}] projectile[{}]",
        gfuncs::GetFormName(data.shooter), gfuncs::GetFormName(data.target), gfuncs::GetFormName(source),
        gfuncs::GetFormName(data.ammo), gfuncs::GetFormName(data.projectileBase));

    logger::trace("projectile impact: sneak[{}] blocked[{}] impactResult[{}] collidedLayer[{}] distanceTraveled[{}] hitPartNodeName[{}]",
        SneakAttack, HitBlocked, data.impactResult, data.collidedLayer, data.distanceTraveled, data.hitPartNodeName);

    logger::trace("projectile impact: hitTranslation posX[{}] posY[{}] posZ[{}] directionX[{}] directionY[{}] directionZ[{}]",
        hitTranslation[0], hitTranslation[1], hitTranslation[2], hitTranslation[3], hitTranslation[4], hitTranslation[5]);
}

TrackedProjectileData GetRecentTrackedProjectileData(RE::TESObjectREFR* shooter, RE::TESObjectREFR* target, float hitGameTime) {
    TrackedProjectileData nullData;

    if (gfuncs::IsFormValid(shooter) && gfuncs::IsFormValid(target)) {
        auto it = recentHitProjectiles.find(target);
        if (it != recentHitProjectiles.end()) {
            if (it->second.size() > 0) {
                for (int i = it->second.size() - 1; i >= 0; --i) {
                    auto akData = it->second[i];
                    float hitTimeDiff = GameHoursToRealTimeSeconds(nullptr, hitGameTime - akData.gameTimeStamp);
                    //logger::trace("{}: hitTimeDiff = [{}]", __func__, hitTimeDiff);
                    if (hitTimeDiff < 0.1) {
                        if (akData.shooter == shooter && akData.target == target) {
                            return akData;
                        }
                    }
                }
            }
        }
    }
    return nullData;
}

struct HitEventSink : public RE::BSTEventSink<RE::TESHitEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* event, RE::BSTEventSource<RE::TESHitEvent>*/*source*/) {
        //logger::trace("hit event");

        if (!event) {
            logger::trace("hit event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("hit event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        std::chrono::system_clock::time_point hitTime = std::chrono::system_clock::now();
        float currentGameTime = calendar->GetHoursPassed();

        RE::TESObjectREFR* attacker = nullptr;
        if (event->cause) {
            attacker = event->cause.get();
            if (!gfuncs::IsFormValid(attacker)) {
                attacker = nullptr;
            }
        }

        RE::TESObjectREFR* target = nullptr;
        if (event->target) {
            target = event->target.get();
            if (!gfuncs::IsFormValid(target)) {
                logger::trace("hit event: no target");
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        RE::TESForm* source = nullptr;
        if (event->source) {
            source = RE::TESForm::LookupByID(event->source);
            if (!gfuncs::IsFormValid(source)) {
                source = nullptr;
            }
        }

        RE::BGSProjectile* projectileForm = nullptr;
        if (event->projectile) {
            projectileForm = RE::TESForm::LookupByID<RE::BGSProjectile>(event->projectile);
            if (!gfuncs::IsFormValid(projectileForm)) {
                projectileForm = nullptr;
            }
        }

        RE::TESAmmo* ammo = nullptr;

        bool powerAttack = event->flags.any(RE::TESHitEvent::Flag::kPowerAttack);
        bool SneakAttack = event->flags.any(RE::TESHitEvent::Flag::kSneakAttack);
        bool bBashAttack = event->flags.any(RE::TESHitEvent::Flag::kBashAttack);
        bool HitBlocked = event->flags.any(RE::TESHitEvent::Flag::kHitBlocked);

        bool hitEventDataEmpty = eventDataPtrs[EventEnum_OnProjectileImpact]->isEmpty();

        if (!bBashAttack) {
            bool bowEquipped = formIsBowOrCrossbow(source);

            if (bowEquipped || gfuncs::IsFormValid(projectileForm)) {
                if (iMaxArrowsSavedPerReference > 0) {

                    auto recentProjectileData = GetRecentTrackedProjectileData(attacker, target, currentGameTime);

                    if (gfuncs::IsFormValid(recentProjectileData.target) && gfuncs::IsFormValid(recentProjectileData.shooter)) {

                        std::vector<float> hitTranslation;

                        if (gfuncs::IsFormValid(recentProjectileData.target)) {
                            RE::Actor* actor = target->As<RE::Actor>();
                            if (gfuncs::IsFormValid(actor)) {
                                auto* aiProcess = actor->GetActorRuntimeData().currentProcess;
                                if (aiProcess) {
                                    if (aiProcess->middleHigh) {
                                        if (aiProcess->middleHigh->lastHitData) {
                                            auto pos = aiProcess->middleHigh->lastHitData->hitPosition;
                                            auto direction = aiProcess->middleHigh->lastHitData->hitDirection;
                                            hitTranslation.push_back(pos.x);
                                            hitTranslation.push_back(pos.y);
                                            hitTranslation.push_back(pos.z);
                                            hitTranslation.push_back(direction.x);
                                            hitTranslation.push_back(direction.y);
                                            hitTranslation.push_back(direction.z);
                                        }
                                    }
                                }
                            }
                        }
                        if (hitTranslation.size() == 0) {
                            hitTranslation.resize(1); //can't send empty array or it causes ctd
                        }
                        SendProjectileImpactEvent(recentProjectileData, source, SneakAttack, HitBlocked, currentGameTime, hitTranslation);
                    }
                }
            }

            if (bowEquipped && eventDataPtrs[EventEnum_HitEvent]->sinkAdded) {
                if (gfuncs::IsFormValid(attacker) && gfuncs::IsFormValid(target)) {
                    RE::Actor* actor = attacker->As<RE::Actor>();
                    if (gfuncs::IsFormValid(actor)) {
                        ammo = actor->GetCurrentAmmo();
                        auto it = animationEventActorsMap.find(attacker);
                        if (it != animationEventActorsMap.end()) { //akactor found in animationEventActorsMap
                            if (it->second) {
                                float hitTimeDiff = gfuncs::timePointDiffToFloat(hitTime, it->second->lastHitTime);
                                it->second->lastHitTime = hitTime;
                                logger::trace("hit event: found ammo tracking data for [{}] hitTimeDiff = [{}]", gfuncs::GetFormName(attacker), hitTimeDiff);

                                bool bSetAmmoToLastHitAmmo = false;
                                if (hitTimeDiff <= 0.1 && target == it->second->lastHitTarget) { //assume this is the same hit event run again (for enchantments ect) ) {
                                    if (it->second->lastHitAmmo) {
                                        logger::debug("hit event multiple, setting ammo for attacker [{}] from [{}] to [{}]", gfuncs::GetFormName(attacker), gfuncs::GetFormName(ammo), gfuncs::GetFormName(it->second->lastHitAmmo));
                                        ammo = it->second->lastHitAmmo;
                                        bSetAmmoToLastHitAmmo = true;
                                    }
                                }

                                it->second->lastHitTarget = target;

                                if (!bSetAmmoToLastHitAmmo) {
                                    if (it->second->forceChangedAmmos.size() > 0) { //the attacker did have their ammo force unequipped by shooting last arrow of type. 
                                        int indexToErase = -1;
                                        float gameHoursPassed = calendar->GetHoursPassed();

                                        bool ammoFound = false;

                                        if (iMaxArrowsSavedPerReference > 0) {
                                            auto recentHitIt = recentShotProjectiles.find(attacker);
                                            if (recentHitIt != recentShotProjectiles.end()) {
                                                for (int i = 0; i < it->second->forceChangedAmmos.size() && !ammoFound; i++) { //cycle through force unequipped ammos due to shooting last arrow of type. 
                                                    for (int ii = recentHitIt->second.size() - 1; ii >= 0 && !ammoFound; --ii) { //cycle through projectiles that recently hit the target to find matching ammo
                                                        float timeDiff = GameHoursToRealTimeSeconds(nullptr, (gameHoursPassed - recentHitIt->second[ii].gameTimeStamp));
                                                        //logger::trace("hit event: recentHitProjectile[{}] timeDiff = [{}]", gfuncs::GetFormName(recentHitIt->second[ii].projectile), timeDiff);
                                                        if (timeDiff < 0.1) {
                                                            if (DidProjectileHitRefWithAmmoFromShooter(attacker, target, it->second->forceChangedAmmos[i].ammo, recentHitIt->second[ii])) {
                                                                auto trackedProjectileData = recentHitIt->second[ii];

                                                                logger::trace("hit event: attacker[{}] target[{}] ammo changed from [{}] to [{}]. Recent projectile found[{}]. TimeDiff = [{}]",
                                                                    gfuncs::GetFormName(attacker), gfuncs::GetFormName(target), gfuncs::GetFormName(ammo), gfuncs::GetFormName(it->second->forceChangedAmmos[i].ammo), gfuncs::GetFormName(trackedProjectileData.projectileBase), timeDiff);

                                                                ammo = it->second->forceChangedAmmos[i].ammo;
                                                                it->second->forceChangedAmmos.erase(it->second->forceChangedAmmos.begin() + i);
                                                                ammoFound = true; //break both loops and set ammo as found
                                                            }
                                                        }
                                                        else {
                                                            break; //break out of this loop, check next force unequipped ammo
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                        if (!ammoFound) { //ammo not found in previous search
                                            for (int i = 0; i < it->second->forceChangedAmmos.size(); i++) {
                                                if (GameHoursToRealTimeSeconds(nullptr, (gameHoursPassed - it->second->forceChangedAmmos[0].gameTimeStamp) < 1.5)) {
                                                    ammo = it->second->forceChangedAmmos[i].ammo;
                                                    it->second->forceChangedAmmos.erase(it->second->forceChangedAmmos.begin() + i);
                                                    break;
                                                }
                                                else {
                                                    indexToErase = i;
                                                }
                                            }
                                            if (indexToErase > -1 && it->second->forceChangedAmmos.size() > 0) {
                                                it->second->forceChangedAmmos.erase(it->second->forceChangedAmmos.begin() + indexToErase);
                                            }
                                        }
                                    }
                                }
                                it->second->lastHitAmmo = ammo;
                            }
                        }
                    }
                }
            }
        }

        if (eventDataPtrs[EventEnum_HitEvent]->sinkAdded) {
            std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_HitEvent]->globalHandles;

            if (gfuncs::IsFormValid(ammo)) {
                ammo = nullptr;
            }

            CombineEventHandles(handles, attacker, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[0]);
            CombineEventHandles(handles, target, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[1]);
            CombineEventHandles(handles, source, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[2]);
            CombineEventHandles(handles, ammo, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[3]);
            CombineEventHandles(handles, projectileForm, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[4]);

            gfuncs::RemoveDuplicates(handles);

            logger::trace("HitEvent: attacker[{}]  target[{}]  source[{}]  ammo[{}]  projectile[{}]", gfuncs::GetFormName(attacker), gfuncs::GetFormName(target), gfuncs::GetFormName(source), gfuncs::GetFormName(ammo), gfuncs::GetFormName(projectileForm));
            logger::trace("HitEvent: powerAttack[{}]  SneakAttack[{}]  BashAttack[{}]  HitBlocked[{}]", powerAttack, SneakAttack, bBashAttack, HitBlocked);

            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)attacker, (RE::TESObjectREFR*)target, (RE::TESForm*)source,
                (RE::TESAmmo*)ammo, (RE::BGSProjectile*)projectileForm, (bool)powerAttack, (bool)SneakAttack, (bool)bBashAttack, (bool)HitBlocked);

            SendEvents(handles, eventDataPtrs[EventEnum_HitEvent]->sEvent, args);
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

void CheckForPlayerCombatStatusChange() {
    logger::trace("{}", __func__);

    bool playerInCombat = player->IsInCombat();
    if (bPlayerIsInCombat != playerInCombat) {
        bPlayerIsInCombat = playerInCombat;
        int combatState = static_cast<int>(bPlayerIsInCombat);

        RE::Actor* target = nullptr;
        auto* combatGroup = player->GetCombatGroup();
        if (combatGroup) {
            if (combatGroup->targets.size() > 0) {
                auto combatHandle = combatGroup->targets[0].targetHandle;
                if (combatHandle) {
                    auto combatPtr = combatHandle.get();
                    if (combatPtr) {
                        target = combatPtr.get();
                    }
                }
            }
        }

        logger::debug("{} target[{}]", __func__, gfuncs::GetFormName(target));

        // RE::TESForm* Target = .attackedMember.get().get();

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnCombatStateChanged]->globalHandles; //
        CombineEventHandles(handles, gfuncs::playerRef, eventDataPtrs[EventEnum_OnCombatStateChanged]->eventParamMaps[0]);
        CombineEventHandles(handles, target, eventDataPtrs[EventEnum_OnCombatStateChanged]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)gfuncs::playerRef, (RE::Actor*)target, (int)combatState);
        SendEvents(handles, eventDataPtrs[EventEnum_OnCombatStateChanged]->sEvent, args);
    }
}

struct CombatEventSink : public RE::BSTEventSink<RE::TESCombatEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* event, RE::BSTEventSource<RE::TESCombatEvent>*/*source*/) {

        if (!event) {
            logger::error("combat change event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("combat change event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESObjectREFR* actorObjRef = nullptr;
        if (event->actor) {
            actorObjRef = event->actor.get();
            if (!gfuncs::IsFormValid(actorObjRef)) {
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        RE::TESObjectREFR* target = nullptr;
        if (event->targetActor) {
            target = event->targetActor.get();
            if (!gfuncs::IsFormValid(target)) {
                target = nullptr;
            }
        }

        int combatState = static_cast<int>(event->newState.get());

        //RE::Actor* actorRef = actorObjRef->As<RE::Actor>();
        //logger::trace("Actor {} changed to combat state to {} with", gfuncs::GetFormName(actorObjRef), combatState);

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnCombatStateChanged]->globalHandles;
        CombineEventHandles(handles, actorObjRef, eventDataPtrs[EventEnum_OnCombatStateChanged]->eventParamMaps[0]);
        CombineEventHandles(handles, target, eventDataPtrs[EventEnum_OnCombatStateChanged]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)actorObjRef, (RE::Actor*)target, (int)combatState);
        SendEvents(handles, eventDataPtrs[EventEnum_OnCombatStateChanged]->sEvent, args);

        if (bRegisteredForPlayerCombatChange) {
            gfuncs::DelayedFunction(&CheckForPlayerCombatStatusChange, 1200); //check for player combat status change after 1.2 seconds.
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct FurnitureEventSink : public RE::BSTEventSink<RE::TESFurnitureEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESFurnitureEvent* event, RE::BSTEventSource<RE::TESFurnitureEvent>*/*source*/) {

        if (!event) {
            logger::error("furniture event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("furniture event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESObjectREFR* actorObjRef = nullptr;
        if (event->actor) {
            actorObjRef = event->actor.get();
            if (!gfuncs::IsFormValid(actorObjRef)) {
                actorObjRef = nullptr;
            }
        }

        RE::Actor* actorRef = nullptr;
        if (gfuncs::IsFormValid(actorObjRef)) {
            actorRef = actorObjRef->As<RE::Actor>();
            if (!gfuncs::IsFormValid(actorRef)) {
                actorRef = nullptr;
            }
        }

        RE::TESObjectREFR* furnitureRef = nullptr;
        if (event->targetFurniture) {
            furnitureRef = event->targetFurniture.get();
            if (!gfuncs::IsFormValid(furnitureRef)) {
                furnitureRef = nullptr;
            }
        }

        int type = event->type.underlying();

        RE::BSFixedString sEvent;
        int eventIndex;

        switch (type) {
        case 0: //enter
            eventIndex = EventEnum_FurnitureEnter;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
            break;
        case 1: //exit
            eventIndex = EventEnum_FurnitureExit;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
            break;
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->globalHandles;
        CombineEventHandles(handles, actorObjRef, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        CombineEventHandles(handles, furnitureRef, eventDataPtrs[eventIndex]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)actorRef, (RE::TESObjectREFR*)furnitureRef);
        SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct ActivateEventSink : public RE::BSTEventSink<RE::TESActivateEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* event, RE::BSTEventSource<RE::TESActivateEvent>*/*source*/) {

        if (!event) {
            logger::error("activate event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("activate event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESObjectREFR* activatorRef = nullptr;
        if (event->actionRef) {
            activatorRef = event->actionRef.get();
            if (!gfuncs::IsFormValid(activatorRef)) {
                activatorRef = nullptr;
            }
        }

        RE::TESObjectREFR* activatedRef = nullptr;
        if (event->objectActivated) {
            activatedRef = event->objectActivated.get();
            if (!gfuncs::IsFormValid(activatedRef)) {
                activatedRef = nullptr;
            }
        }

        if (activatorRef == gfuncs::playerRef) {
            if (gfuncs::IsFormValid(activatedRef)) {
                gfuncs::lastPlayerActivatedRef = activatedRef;
            }
        }

        if (eventDataPtrs[EventEnum_OnActivate]->sinkAdded) {
            std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnActivate]->globalHandles;

            CombineEventHandles(handles, activatorRef, eventDataPtrs[EventEnum_OnActivate]->eventParamMaps[0]);
            CombineEventHandles(handles, activatedRef, eventDataPtrs[EventEnum_OnActivate]->eventParamMaps[1]);

            gfuncs::RemoveDuplicates(handles);

            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)activatorRef, (RE::TESObjectREFR*)activatedRef);
            SendEvents(handles, eventDataPtrs[EventEnum_OnActivate]->sEvent, args);
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct DeathEventSink : public RE::BSTEventSink<RE::TESDeathEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* event, RE::BSTEventSource<RE::TESDeathEvent>* source) {

        if (!event) {
            logger::warn("death / dying event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("death / dying event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("Death Event");

        RE::TESObjectREFR* victimRef = nullptr;
        if (event->actorDying) {
            victimRef = event->actorDying.get();
            if (!gfuncs::IsFormValid(victimRef)) {
                victimRef = nullptr;
            }
        }

        RE::TESObjectREFR* killerRef = nullptr;
        if (event->actorKiller) {
            killerRef = event->actorKiller.get();
            if (!gfuncs::IsFormValid(killerRef)) {
                killerRef = nullptr;
            }
        }

        RE::Actor* victim = nullptr;
        if (gfuncs::IsFormValid(victimRef)) {
            victim = static_cast<RE::Actor*>(victimRef);
            if (!gfuncs::IsFormValid(victim)) {
                logger::warn("death event: victim doesn't exist or isn't valid");
                return RE::BSEventNotifyControl::kContinue;
            }
            logger::trace("death event: valid victimRef pointer");
        }
        else {
            logger::warn("death event: 0 victimRef doesn't exist or isn't valid");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::Actor* killer = nullptr;
        if (gfuncs::IsFormValid(killerRef)) {
            killer = static_cast<RE::Actor*>(killerRef);
            if (!gfuncs::IsFormValid(killer)) {
                killer = nullptr;
            }
            else {
                logger::trace("death event: valid killerRef pointer");
            }
        }

        bool dead = event->dead;

        logger::trace("Death Event: victim[{}], Killer[{}], Dead[{}]", gfuncs::GetFormName(victim), gfuncs::GetFormName(killer), dead);

        RE::BSFixedString sEvent;
        int eventIndex;

        if (dead) {
            eventIndex = EventEnum_DeathEvent;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
        }
        else {
            eventIndex = EventEnum_DyingEvent;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->globalHandles;
        CombineEventHandles(handles, victim, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        CombineEventHandles(handles, killer, eventDataPtrs[eventIndex]->eventParamMaps[1]);
        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)victim, (RE::Actor*)killer);
        SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

bool RegisterActorForBowDrawAnimEvent(RE::TESObjectREFR* actorRef) {
    if (gfuncs::IsFormValid(actorRef)) {
        RE::Actor* actor = actorRef->As<RE::Actor>();
        if (actorHasBowEquipped(actor)) {
            auto it = animationEventActorsMap.find(actorRef);
            if (it == animationEventActorsMap.end()) { //actorRef not already registered
                AnimationEventSink* animEventSink = new AnimationEventSink();
                animEventSink->actor = actor;
                animationEventActorsMap.insert(std::pair<RE::TESObjectREFR*, AnimationEventSink*>(actorRef, animEventSink));
                actor->AddAnimationGraphEventSink(animEventSink);
                logger::info("{}: actor [{}]", __func__, gfuncs::GetFormName(actorRef));
                return true;
            }
        }
    }
    return false;
}

bool UnRegisterActorForBowDrawAnimEvent(RE::TESObjectREFR* actorRef) {
    bool bUnregistered = false;
    if (actorRef) {
        auto it = animationEventActorsMap.find(actorRef);
        if (it != animationEventActorsMap.end()) { //actorRef registered
            if (it->second) {
                if (it->second->actor) {
                    if (!actorHasBowEquipped(it->second->actor)) {
                        it->second->actor->RemoveAnimationGraphEventSink(it->second);
                        it->second->actor = nullptr;
                        delete it->second;
                        it->second = nullptr;
                        animationEventActorsMap.erase(it);
                        logger::debug("{}: actor [{}]", __func__, gfuncs::GetFormName(actorRef));
                        bUnregistered = true;
                    }
                }
                else {
                    //it->second->bowReleasedTrackedAmmoDatas.clear();
                    delete it->second;
                    it->second = nullptr;
                    animationEventActorsMap.erase(it);
                    logger::debug("{}: actor [{}]", __func__, gfuncs::GetFormName(actorRef));
                    bUnregistered = true;
                }
            }
            else { //animation event doesn't exist
                animationEventActorsMap.erase(it);
                logger::debug("{}: actor [{}]", __func__, gfuncs::GetFormName(actorRef));
                bUnregistered = true;
            }
        }
    }
    return bUnregistered;
}

//register all actors with bows or crossbows equipped for animation events to track equipped ammo with "bowDraw" event
void RegisterActorsForBowDrawAnimEvents() {
    const auto& [allForms, lock] = RE::TESForm::GetAllForms();
    for (auto& [id, form] : *allForms) {
        RE::TESObjectREFR* ref = form->AsReference();
        RegisterActorForBowDrawAnimEvent(ref);
    }
}

struct EquipEventData {
    float gameTimeStamp;
    RE::TESObjectREFR* akActorRef;
    RE::TESForm* baseObject;
    bool equipped;
};

EquipEventData lastEquipEvent;

struct EquipEventSink : public RE::BSTEventSink<RE::TESEquipEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*/*source*/) {
        if (!event) {
            logger::error("equip event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("equip event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESObjectREFR* akActorRef = nullptr;
        if (event->actor) {
            akActorRef = event->actor.get();
        }

        RE::Actor* akActor = nullptr;
        if (gfuncs::IsFormValid(akActorRef)) {
            akActor = akActorRef->As<RE::Actor>();
            if (!gfuncs::IsFormValid(akActor)) {
                akActor = nullptr;
            }
        }

        RE::TESForm* baseObject = nullptr;
        if (event->baseObject) {
            baseObject = RE::TESForm::LookupByID(event->baseObject);
            if (!gfuncs::IsFormValid(baseObject)) {
                baseObject = nullptr;
            }
        }

        bool equipped = event->equipped;
        int eventIndex = -1;

        float fTime = calendar->GetHoursPassed();

        //equip events can get sent twice apparently. This happens when an ammo is force unequipped after shooting the last arrow or bolt of type in inventory. 
        //skip the event if time and other variables match last equip event
        if (fTime == lastEquipEvent.gameTimeStamp) {
            if (lastEquipEvent.akActorRef == akActorRef && lastEquipEvent.baseObject == baseObject && lastEquipEvent.equipped == equipped) {
                logger::debug("Skipping Equip Event: Actor[{}], BaseObject[{}], Equipped[{}] gameTimeStamp[{}]", gfuncs::GetFormName(akActorRef), gfuncs::GetFormName(baseObject), equipped, fTime);
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        lastEquipEvent.gameTimeStamp = fTime;
        lastEquipEvent.akActorRef = akActorRef;
        lastEquipEvent.baseObject = baseObject;
        lastEquipEvent.equipped = equipped;

        if (equipped) {
            if (formIsBowOrCrossbow(baseObject)) {
                logger::trace("equip event registering actor [{}] for animation events", gfuncs::GetFormName(akActorRef));
                RegisterActorForBowDrawAnimEvent(akActorRef);
            }

            if (eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded) {
                eventIndex = EventEnum_OnObjectEquipped;
            }
        }
        else {
            if (gfuncs::IsFormValid(baseObject)) {
                RE::TESAmmo* akAmmo = baseObject->As<RE::TESAmmo>();
                if (gfuncs::IsFormValid(akAmmo)) {
                    if (gfuncs::IsFormValid(akActor)) {
                        auto it = animationEventActorsMap.find(akActorRef);
                        if (it != animationEventActorsMap.end()) {
                            float timeDiff = GameHoursToRealTimeSeconds(nullptr, (fTime - it->second->lastReleaseGameTime));
                            logger::trace("equip event: actor[{}] unequipped ammo[{}]. Time since last bow release is [{}]", gfuncs::GetFormName(akActor), gfuncs::GetFormName(akAmmo), timeDiff);

                            if (timeDiff < 0.2) { //actor released their bow less than 0.2 seconds ago.
                                logger::debug("unequip event: saved force unequipped ammo[{}] for actor[{}]", gfuncs::GetFormName(akAmmo), gfuncs::GetFormName(akActor));
                                TrackedActorAmmoData data;
                                data.ammo = akAmmo;
                                data.gameTimeStamp = fTime;
                                it->second->forceChangedAmmos.push_back(data);
                            }
                        }
                    }
                }
            }
            else if (formIsBowOrCrossbow(baseObject)) {
                logger::trace("equip event Unregistering actor[{}] for animation events", gfuncs::GetFormName(akActorRef));
                UnRegisterActorForBowDrawAnimEvent(akActorRef);
            }

            if (eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded) {
                eventIndex = EventEnum_OnObjectUnequipped;
            }
        }

        //logger::trace("Equip Event: Actor[{}], BaseObject[{}], Equipped[{}]", gfuncs::GetFormName(akActorRef), gfuncs::GetFormName(baseObject), equipped);

        if (eventIndex != -1) {
            RE::TESForm* ref = RE::TESForm::LookupByID(event->originalRefr);
            if (!gfuncs::IsFormValid(ref)) {
                ref = nullptr;
            }

            logger::trace("Equip Event: Actor[{}], BaseObject[{}], Ref[{}] Equipped[{}]", gfuncs::GetFormName(akActorRef), gfuncs::GetFormName(baseObject), gfuncs::GetFormName(ref), equipped);

            std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->globalHandles;

            CombineEventHandles(handles, akActorRef, eventDataPtrs[eventIndex]->eventParamMaps[0]);
            CombineEventHandles(handles, baseObject, eventDataPtrs[eventIndex]->eventParamMaps[1]);
            CombineEventHandles(handles, ref, eventDataPtrs[eventIndex]->eventParamMaps[2]);

            gfuncs::RemoveDuplicates(handles);

            auto* args = RE::MakeFunctionArguments((RE::Actor*)akActorRef, (RE::TESForm*)baseObject, (RE::TESObjectREFR*)ref);
            SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct WaitStartEventSink : public RE::BSTEventSink<RE::TESWaitStartEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESWaitStartEvent* event, RE::BSTEventSource<RE::TESWaitStartEvent>*/*source*/) {

        if (!event) {
            logger::error("wait start event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("wait start event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("Wait Start Event");

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnWaitStart]->globalHandles;

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments();
        SendEvents(handles, eventDataPtrs[EventEnum_OnWaitStart]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct WaitStopEventSink : public RE::BSTEventSink<RE::TESWaitStopEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESWaitStopEvent* event, RE::BSTEventSource<RE::TESWaitStopEvent>*/*source*/) {

        if (!event) {
            logger::error("wait stop event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("wait stop event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("Wait Stop Event");

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnWaitStop]->globalHandles;

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((bool)event->interrupted);
        SendEvents(handles, eventDataPtrs[EventEnum_OnWaitStop]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct MagicEffectApplyEventSink : public RE::BSTEventSink<RE::TESMagicEffectApplyEvent> { //
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESMagicEffectApplyEvent* event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*/*source*/) {

        if (!event) {
            logger::error("MagicEffectApply event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("MagicEffectApply event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("MagicEffectApply Event");

        RE::TESObjectREFR* caster = nullptr;
        if (event->caster) {
            caster = event->caster.get();
            if (!gfuncs::IsFormValid(caster)) {
                caster = nullptr;
            }
        }

        RE::TESObjectREFR* target = nullptr;
        if (event->target) {
            target = event->target.get();
            if (!gfuncs::IsFormValid(target)) {
                target = nullptr;
            }
        }

        RE::EffectSetting* magicEffect = nullptr;
        if (event->magicEffect) {
            magicEffect = RE::TESForm::LookupByID<RE::EffectSetting>(event->magicEffect);
            if (!gfuncs::IsFormValid(magicEffect)) {
                magicEffect = nullptr;
            }
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnMagicEffectApply]->globalHandles;

        CombineEventHandles(handles, caster, eventDataPtrs[EventEnum_OnMagicEffectApply]->eventParamMaps[0]);
        CombineEventHandles(handles, target, eventDataPtrs[EventEnum_OnMagicEffectApply]->eventParamMaps[1]);
        CombineEventHandles(handles, magicEffect, eventDataPtrs[EventEnum_OnMagicEffectApply]->eventParamMaps[2]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)caster, (RE::TESObjectREFR*)target, (RE::EffectSetting*)magicEffect);
        SendEvents(handles, eventDataPtrs[EventEnum_OnMagicEffectApply]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct LockChangedEventSink : public RE::BSTEventSink<RE::TESLockChangedEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESLockChangedEvent* event, RE::BSTEventSource<RE::TESLockChangedEvent>*/*source*/) {

        if (!event) {
            logger::error("Lock Changed event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Lock Changed event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("Lock Changed Event");

        RE::TESObjectREFR* lockedObject = nullptr;
        if (event->lockedObject) {
            lockedObject = event->lockedObject.get();
        }

        if (!gfuncs::IsFormValid(lockedObject)) {
            return RE::BSEventNotifyControl::kContinue;
        }

        bool Locked = lockedObject->IsLocked();

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_LockChanged]->globalHandles;
        CombineEventHandles(handles, lockedObject, eventDataPtrs[EventEnum_LockChanged]->eventParamMaps[0]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)lockedObject, (bool)Locked);
        SendEvents(handles, eventDataPtrs[EventEnum_LockChanged]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct OpenCloseEventSink : public RE::BSTEventSink<RE::TESOpenCloseEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESOpenCloseEvent* event, RE::BSTEventSource<RE::TESOpenCloseEvent>*/*source*/) {

        if (!event) {
            logger::error("OpenClose event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("OpenClose event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("Open Close Event");

        RE::TESObjectREFR* activatorRef = nullptr;
        if (event->activeRef) {
            activatorRef = event->activeRef.get();
            if (!gfuncs::IsFormValid(activatorRef)) {
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        RE::TESObjectREFR* akActionRef = nullptr;
        if (event->ref) {
            akActionRef = event->ref.get();
            if (!gfuncs::IsFormValid(akActionRef)) {
                akActionRef = nullptr;
            }
        }

        int eventIndex;

        if (event->opened) {
            eventIndex = EventEnum_OnOpen;
        }
        else {
            eventIndex = EventEnum_OnClose;
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->globalHandles;

        CombineEventHandles(handles, activatorRef, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        CombineEventHandles(handles, akActionRef, eventDataPtrs[eventIndex]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)activatorRef, (RE::TESObjectREFR*)akActionRef);
        SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct SpellCastEventSink : public RE::BSTEventSink<RE::TESSpellCastEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESSpellCastEvent* event, RE::BSTEventSource<RE::TESSpellCastEvent>*/*source*/) {

        if (!event) {
            logger::error("spell cast event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("spell cast event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("spell cast Event");

        RE::TESObjectREFR* caster = nullptr;
        if (event->object) {
            caster = event->object.get();
            if (!gfuncs::IsFormValid(caster)) {
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        RE::TESForm* spell = nullptr;
        if (event->spell) {
            spell = RE::TESForm::LookupByID(event->spell);
            if (!gfuncs::IsFormValid(spell)) {
                spell = nullptr;
            }
        }

        //logger::trace("spell cast: [{}] obj [{}]", gfuncs::GetFormName(spell), gfuncs::GetFormName(caster));

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnSpellCast]->globalHandles;

        CombineEventHandles(handles, caster, eventDataPtrs[EventEnum_OnSpellCast]->eventParamMaps[0]);
        CombineEventHandles(handles, spell, eventDataPtrs[EventEnum_OnSpellCast]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)caster, (RE::TESForm*)spell);
        SendEvents(handles, eventDataPtrs[EventEnum_OnSpellCast]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct ContainerChangedEventSink : public RE::BSTEventSink<RE::TESContainerChangedEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* event, RE::BSTEventSource<RE::TESContainerChangedEvent>*/*source*/) {

        if (!event) {
            logger::error("Container Change event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Container Change event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        //logger::debug("Container Change Event");

        RE::TESForm* baseObj = nullptr;
        if (event->baseObj) {
            baseObj = RE::TESForm::LookupByID(event->baseObj);
            if (!gfuncs::IsFormValid(baseObj)) {
                baseObj = nullptr;
            }
        }

        RE::TESObjectREFR* itemReference = nullptr;
        auto refHandle = event->reference;
        if (refHandle) {
            //logger::debug("refHandle found");

            RE::TESForm* refForm = RE::TESForm::LookupByID(refHandle.native_handle());
            if (gfuncs::IsFormValid(refForm)) {
                //logger::debug("refForm found");
                itemReference = refForm->AsReference();
                if (!gfuncs::IsFormValid(itemReference)) {
                    return RE::BSEventNotifyControl::kContinue;
                }
            }
        }

        RE::TESObjectREFR* newContainer = nullptr;
        if (event->newContainer) {
            RE::TESForm* newContainerForm = RE::TESForm::LookupByID(event->newContainer);
            if (gfuncs::IsFormValid(newContainerForm)) {
                newContainer = newContainerForm->AsReference();
                if (!gfuncs::IsFormValid(newContainer)) {
                    newContainer = nullptr;
                }
            }
        }

        RE::TESObjectREFR* oldContainer = nullptr;
        if (event->oldContainer) {
            RE::TESForm* oldContainerForm = RE::TESForm::LookupByID(event->oldContainer);
            if (gfuncs::IsFormValid(oldContainerForm)) {
                oldContainer = oldContainerForm->AsReference();
                if (!gfuncs::IsFormValid(oldContainer)) {
                    oldContainer = nullptr;
                }
            }
        }

        int itemCount = event->itemCount;

        //logger::debug("newContainer[{}] oldContainer[{}] itemReference[{}] baseObj[{}] itemCount[{}]", gfuncs::GetFormName(newContainer), gfuncs::GetFormName(oldContainer), gfuncs::GetFormName(itemReference), gfuncs::GetFormName(baseObj), itemCount);

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnContainerChanged]->globalHandles;

        CombineEventHandles(handles, newContainer, eventDataPtrs[EventEnum_OnContainerChanged]->eventParamMaps[0]);
        CombineEventHandles(handles, oldContainer, eventDataPtrs[EventEnum_OnContainerChanged]->eventParamMaps[1]);
        CombineEventHandles(handles, itemReference, eventDataPtrs[EventEnum_OnContainerChanged]->eventParamMaps[2]);
        CombineEventHandles(handles, baseObj, eventDataPtrs[EventEnum_OnContainerChanged]->eventParamMaps[3]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)newContainer, (RE::TESObjectREFR*)oldContainer, (RE::TESObjectREFR*)itemReference, (RE::TESForm*)baseObj, (int)itemCount);
        SendEvents(handles, eventDataPtrs[EventEnum_OnContainerChanged]->sEvent, args);

        return RE::BSEventNotifyControl::kContinue;
    }
};

std::vector<std::string> actionSlotStrings{"left", "right", "voice"};
std::vector<std::string> actionTypeStrings{
    "kWeaponSwing",
        "SpellCast",
        "SpellFire",
        "VoiceCast",
        "VoiceFire",
        "BowDraw",
        "BowRelease",
        "BeginDraw",
        "EndDraw",
        "BeginSheathe",
        "EndSheathe"};

struct ActorActionEventSink : public RE::BSTEventSink<SKSE::ActionEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const SKSE::ActionEvent* event, RE::BSTEventSource<SKSE::ActionEvent>*/*source*/) {

        if (!event) {
            logger::error("Action Event event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Action Event event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("Action Event");

        RE::Actor* akActor = event->actor;
        if (!gfuncs::IsFormValid(akActor)) {
            return RE::BSEventNotifyControl::kContinue;
        }

        //0 = left hand, 1 = right hand, 2 = voice
        int slot = event->slot.underlying();

        RE::TESForm* akSource = event->sourceForm;
        if (!gfuncs::IsFormValid(akSource)) {
            akSource = nullptr;
        }
        /*  kWeaponSwing = 0,
            kSpellCast = 1,
            kSpellFire = 2,
            kVoiceCast = 3,
            kVoiceFire = 4,
            kBowDraw = 5,
            kBowRelease = 6,
            kBeginDraw = 7,
            kEndDraw = 8,
            kBeginSheathe = 9,
            kEndSheathe = 10*/

        int type = event->type.underlying();

        int eventIndex = -1;

        switch (type) {
        case 0:
            eventIndex = EventEnum_OnWeaponSwing;
            break;
        case 1:
            eventIndex = EventEnum_OnActorSpellCast;
            break;
        case 2:
            eventIndex = EventEnum_OnActorSpellFire;
            break;
        case 3:
            eventIndex = EventEnum_VoiceCast;
            break;
        case 4:
            eventIndex = EventEnum_VoiceFire;
            break;
        case 5:
            eventIndex = EventEnum_BowDraw;
            break;
        case 6:
            eventIndex = EventEnum_BowRelease;
            break;
        case 7:
            eventIndex = EventEnum_BeginDraw;
            break;
        case 8:
            eventIndex = EventEnum_EndDraw;
            break;
        case 9:
            eventIndex = EventEnum_BeginSheathe;
            break;
        case 10:
            eventIndex = EventEnum_EndSheathe;
            break;
        }

        logger::trace("action event, Actor[{}] Source[{}]  type[{}] slot[{}]", gfuncs::GetFormName(akActor), gfuncs::GetFormName(akSource),
            actionTypeStrings[type], actionSlotStrings[slot]);

        if (eventIndex == -1) {
            return RE::BSEventNotifyControl::kContinue;
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->globalHandles;

        CombineEventHandles(handles, akActor, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        CombineEventHandles(handles, akSource, eventDataPtrs[eventIndex]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor, (RE::TESForm*)akSource, (int)slot);
        SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

        //draw / sheathe events aren't triggered for left hand. Send left hand events manually
        if (eventIndex >= EventEnum_BeginDraw && eventIndex <= EventEnum_EndSheathe) {
            if (gfuncs::IsFormValid(akActor)) {

                RE::TESForm* leftHandSource = akActor->GetEquippedObject(true);

                if (gfuncs::IsFormValid(leftHandSource)) {
                    if (leftHandSource != akSource) {
                        std::vector<RE::VMHandle> handlesB = eventDataPtrs[eventIndex]->globalHandles;

                        CombineEventHandles(handlesB, akActor, eventDataPtrs[eventIndex]->eventParamMaps[0]);
                        CombineEventHandles(handlesB, leftHandSource, eventDataPtrs[eventIndex]->eventParamMaps[1]);

                        gfuncs::RemoveDuplicates(handlesB);

                        auto* argsB = RE::MakeFunctionArguments((RE::Actor*)akActor, (RE::TESForm*)leftHandSource, (int)0);
                        SendEvents(handlesB, eventDataPtrs[eventIndex]->sEvent, argsB);

                        logger::trace("action event, Actor[{}] Source[{}]  type[{}] slot[{}]", gfuncs::GetFormName(akActor), gfuncs::GetFormName(leftHandSource),
                            actionTypeStrings[type], actionSlotStrings[0]);
                    }
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

void CheckMenusCurrentlyOpen() {
    if (menusCurrentlyOpen.size() == 0) {
        return;
    }
    for (auto& menu : menusCurrentlyOpen) {
        if (!ui->IsMenuOpen(menu)) {
            gfuncs::RemoveFromVectorByValue(menusCurrentlyOpen, menu);
        }
    }
}

struct ItemCraftedEventSink : public RE::BSTEventSink<RE::ItemCrafted::Event> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::ItemCrafted::Event* event, RE::BSTEventSource<RE::ItemCrafted::Event>*/*source*/) {

        if (!event) {
            logger::error("Item Crafted Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Item Crafted Event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        int benchType = 0;
        std::string skill = "";
        int count = 1;

        RE::TESObjectREFR* workbenchRef = nullptr;

        auto* aiProcess = gfuncs::playerRef->GetActorRuntimeData().currentProcess;
        if (aiProcess) {
            if (aiProcess->middleHigh) {
                if (aiProcess->middleHigh->occupiedFurniture) {
                    auto refPtr = aiProcess->middleHigh->occupiedFurniture.get();
                    if (refPtr) {
                        //logger::trace("Item Crafted Event: occupiedFurniture found");
                        workbenchRef = refPtr.get();
                    }
                }
            }
        }

        if (!gfuncs::IsFormValid(workbenchRef)) {
            if (gfuncs::IsFormValid(gfuncs::menuRef)) {
                workbenchRef = gfuncs::menuRef;
                //logger::trace("Item Crafted Event: occupiedFurniture not valid, setting to gfuncs::menuRef");
            }
            else {
                workbenchRef = nullptr;
            }
        }

        if (gfuncs::IsFormValid(workbenchRef)) {
            RE::TESForm* baseObj = workbenchRef->GetBaseObject();
            if (gfuncs::IsFormValid(baseObj)) {
                RE::TESFurniture* furniture = baseObj->As<RE::TESFurniture>();
                benchType = GetFurnitureWorkbenchType(nullptr, furniture);
                skill = GetFurnitureWorkbenchSkillString(nullptr, furniture);
            }
        }

        RE::TESForm* craftedItem = nullptr;
        if (gfuncs::IsFormValid(event->item)) {
            craftedItem = event->item;
            if (!gfuncs::IsFormValid(craftedItem)) {
                return RE::BSEventNotifyControl::kContinue;
            }
        }
        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnItemCrafted]->globalHandles;

        CombineEventHandles(handles, craftedItem, eventDataPtrs[EventEnum_OnItemCrafted]->eventParamMaps[0]);
        CombineEventHandles(handles, workbenchRef, eventDataPtrs[EventEnum_OnItemCrafted]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        if (benchType == 1) { //CreateObject
            count = UIEvents::uiLastSelectedFormData.count;
        }

        auto* args = RE::MakeFunctionArguments((RE::TESForm*)craftedItem, (RE::TESObjectREFR*)workbenchRef,
            (int)count, (int)benchType, (std::string)skill);

        SendEvents(handles, eventDataPtrs[EventEnum_OnItemCrafted]->sEvent, args);

        logger::trace("Item Crafted Event: workbenchRef[{}] craftedItem[{}] benchType[{}] skill[{}]",
            gfuncs::GetFormName(craftedItem), gfuncs::GetFormName(workbenchRef), benchType, skill);

        return RE::BSEventNotifyControl::kContinue;
    }
};

ItemCraftedEventSink* itemCraftedEventSink;

struct ItemsPickpocketedEventSink : public RE::BSTEventSink<RE::ItemsPickpocketed::Event> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::ItemsPickpocketed::Event* event, RE::BSTEventSource<RE::ItemsPickpocketed::Event>*/*source*/) {
        if (!event) {
            logger::error("Item Pickpocketed Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Item Pickpocketed Event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESForm* akForm = UIEvents::uiSelectedFormData.form;
        if (!gfuncs::IsFormValid(akForm)) {
            akForm = nullptr;
        }

        RE::Actor* target = nullptr;

        if (gfuncs::IsFormValid(gfuncs::menuRef)) {
            target = gfuncs::menuRef->As<RE::Actor>();
            if (!gfuncs::IsFormValid(target)) {
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnItemsPickpocketed]->globalHandles;
        CombineEventHandles(handles, target, eventDataPtrs[EventEnum_OnItemsPickpocketed]->eventParamMaps[0]);
        CombineEventHandles(handles, akForm, eventDataPtrs[EventEnum_OnItemsPickpocketed]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)target, (RE::TESForm*)akForm, (int)event->numItems);

        SendEvents(handles, eventDataPtrs[EventEnum_OnItemsPickpocketed]->sEvent, args);

        logger::trace("pickpocket event: numItems[{}] form[{}]", event->numItems, gfuncs::GetFormName(akForm));

        return RE::BSEventNotifyControl::kContinue;
    }
};

ItemsPickpocketedEventSink* itemsPickpocketedEventSink;

struct LocationClearedEventSink : public RE::BSTEventSink<RE::LocationCleared::Event> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::LocationCleared::Event* event, RE::BSTEventSource<RE::LocationCleared::Event>*/*source*/) {
        if (!event) {
            logger::error("Location Cleared Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Location Cleared Event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::BGSLocation* location = gfuncs::playerRef->GetCurrentLocation();
        if (gfuncs::IsFormValid(location)) {
            bool locationCleared = location->IsCleared();
            while (!locationCleared) {
                location = location->parentLoc;
                if (gfuncs::IsFormValid(location)) {
                    bool locationCleared = location->IsCleared();
                }
                else {
                    break;
                }
            }
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnLocationCleared]->globalHandles;

        if (gfuncs::IsFormValid(location)) {
            CombineEventHandles(handles, location, eventDataPtrs[EventEnum_OnLocationCleared]->eventParamMaps[0]);
        }
        else {
            location = nullptr;
        }

        gfuncs::RemoveDuplicates(handles);
        auto* args = RE::MakeFunctionArguments((RE::BGSLocation*)location);

        SendEvents(handles, eventDataPtrs[EventEnum_OnLocationCleared]->sEvent, args);

        logger::trace("location cleared event: location[{}]", gfuncs::GetFormName(location));

        return RE::BSEventNotifyControl::kContinue;
    }
};

LocationClearedEventSink* locationClearedEventSink;

struct EnterBleedoutEventSink : public RE::BSTEventSink<RE::TESEnterBleedoutEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESEnterBleedoutEvent* event, RE::BSTEventSource<RE::TESEnterBleedoutEvent>*/*source*/) {
        if (!event) {
            logger::error("Enter Bleedout Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Enter Bleedout Event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::Actor* akActor = nullptr;

        if (event->actor) {
            auto* actorRef = event->actor.get();
            if (gfuncs::IsFormValid(actorRef)) {
                akActor = actorRef->As<RE::Actor>();
                if (!gfuncs::IsFormValid(akActor)) {
                    return RE::BSEventNotifyControl::kContinue;
                }
            }
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnEnterBleedout]->globalHandles;
        CombineEventHandles(handles, akActor, eventDataPtrs[EventEnum_OnEnterBleedout]->eventParamMaps[0]);

        gfuncs::RemoveDuplicates(handles);
        auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor);

        SendEvents(handles, eventDataPtrs[EventEnum_OnEnterBleedout]->sEvent, args);

        logger::trace("Enter Bleedout Event: akActor[{}]", gfuncs::GetFormName(akActor));

        return RE::BSEventNotifyControl::kContinue;
    }
};

EnterBleedoutEventSink* enterBleedoutEventSink;

struct SwitchRaceCompleteEventSink : public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*/*source*/) {
        //logger::trace("SwitchRaceCompleteEvent");

        if (!event) {
            logger::error("Switch Race Complete Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Switch Race Complete Event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::Actor* akActor = nullptr;
        RE::TESRace* akOldRace = nullptr;
        RE::TESRace* akNewRace = nullptr;

        if (event->subject) {
            auto* ref = event->subject.get();
            if (gfuncs::IsFormValid(ref)) {
                auto actor = ref->As<RE::Actor>();
                if (gfuncs::IsFormValid(actor)) {
                    akActor = actor;
                    auto it = savedActorRacesMap.find(akActor);
                    if (it != savedActorRacesMap.end()) {
                        if (gfuncs::IsFormValid(it->second)) {
                            akOldRace = it->second;
                        }
                    }
                    RE::TESRace* race = actor->GetRace();
                    if (gfuncs::IsFormValid(race)) {
                        akNewRace = race;
                        savedActorRacesMap[actor] = race;
                    }
                }
            }
        }

        if (!akActor || !akNewRace) {
            return RE::BSEventNotifyControl::kContinue;
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnSwitchRaceComplete]->globalHandles;
        CombineEventHandles(handles, akActor, eventDataPtrs[EventEnum_OnSwitchRaceComplete]->eventParamMaps[0]);
        CombineEventHandles(handles, akOldRace, eventDataPtrs[EventEnum_OnSwitchRaceComplete]->eventParamMaps[1]);
        CombineEventHandles(handles, akNewRace, eventDataPtrs[EventEnum_OnSwitchRaceComplete]->eventParamMaps[2]);

        gfuncs::RemoveDuplicates(handles);
        auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor, (RE::TESRace*)akOldRace, (RE::TESRace*)akNewRace);

        SendEvents(handles, eventDataPtrs[EventEnum_OnSwitchRaceComplete]->sEvent, args);

        logger::trace("SwitchRaceCompleteEvent: Actor[{}] akOldRace[{}] akNewRace[{}]",
            gfuncs::GetFormName(akActor), gfuncs::GetFormName(akOldRace), gfuncs::GetFormName(akNewRace));

        //EventEnum_OnSwitchRaceComplete
        return RE::BSEventNotifyControl::kContinue;
    }
};

SwitchRaceCompleteEventSink* switchRaceCompleteEventSink;

struct FootstepEventSink : public RE::BSTEventSink<RE::BGSFootstepEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::BGSFootstepEvent* event, RE::BSTEventSource<RE::BGSFootstepEvent>*/*source*/) {
        if (!event) {
            logger::error("footstep Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("footstep Event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::Actor* akActor = nullptr;

        RE::BGSActorEvent* actorEvent = const_cast<RE::BGSFootstepEvent*>(event);
        if (actorEvent) {
            if (actorEvent->actor) {
                auto ptr = actorEvent->actor.get();
                if (ptr) {
                    auto* actor = ptr.get();
                    if (gfuncs::IsFormValid(actor)) {
                        akActor = actor;
                    }
                    else {
                        return RE::BSEventNotifyControl::kContinue;
                    }
                }
            }
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnActorFootStep]->globalHandles;
        CombineEventHandles(handles, akActor, eventDataPtrs[EventEnum_OnActorFootStep]->eventParamMaps[0]);

        gfuncs::RemoveDuplicates(handles);
        auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor, (std::string)event->tag);

        SendEvents(handles, eventDataPtrs[EventEnum_OnActorFootStep]->sEvent, args);

        logger::trace("FootstepEvent: Actor[{}] tag[{}]",
            gfuncs::GetFormName(akActor), event->tag);

        return RE::BSEventNotifyControl::kContinue;
    }
};

FootstepEventSink* footstepEventSink;

struct QuestObjectiveEventSink : public RE::BSTEventSink<RE::ObjectiveState::Event> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::ObjectiveState::Event* event, RE::BSTEventSource<RE::ObjectiveState::Event>*/*source*/) {
        if (!event) {
            logger::error("QuestObjective Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("QuestObjective Event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (!event->objective) {
            logger::error("QuestObjective Event->objective is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        int oldState = static_cast<int>(event->oldState);
        int newState = static_cast<int>(event->newState);
        int objectiveIndex = event->objective->index;
        std::string displayText = event->objective->displayText.c_str();
        RE::TESQuest* akQuest = nullptr;
        std::vector<RE::BGSBaseAlias*> ojbectiveAliases;

        if (gfuncs::IsFormValid(event->objective->ownerQuest)) {
            akQuest = event->objective->ownerQuest;
            for (int i = 0; i < event->objective->numTargets; i++) {
                auto* target = event->objective->targets[i];
                if (target) {
                    auto* alias = gfuncs::GetQuestAliasById(akQuest, target->alias);
                    if (alias) {
                        ojbectiveAliases.push_back(alias);
                    }
                }
            }
        }
        else {
            return RE::BSEventNotifyControl::kContinue;
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->globalHandles;
        CombineEventHandles(handles, akQuest, eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->eventParamMaps[0]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESQuest*)akQuest, (std::string)displayText, (int)oldState, (int)newState,
            (int)objectiveIndex, (std::vector<RE::BGSBaseAlias*>)ojbectiveAliases);

        SendEvents(handles, eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->sEvent, args);

        logger::trace("QuestObjective Event: quest[{}] displayText[{}] objectiveIndex[{}] oldState[{}] newState[{}] targets[{}]",
            gfuncs::GetFormName(akQuest), displayText, objectiveIndex, oldState, newState, event->objective->numTargets);

        return RE::BSEventNotifyControl::kContinue;
    }
};

QuestObjectiveEventSink* questObjectiveEventSink;

//offset difference for PLAYER_RUNTIME_DATA members between AE and SE is 8.
RE::PLAYER_TARGET_LOC* GetPlayerQueuedTargetLoc() {
    if (REL::Module::IsAE()) {
        uint32_t offset = 0x648;
        //logger::critical("{}: AE Offset [{:x}]", __func__, offset);
        return reinterpret_cast<RE::PLAYER_TARGET_LOC*>((uintptr_t)player + offset);
    }
    else if (REL::Module::IsSE()) {
        uint32_t offset = 0x640;
        //logger::critical("{}: SE Offset [{:x}]", __func__, offset);
        return reinterpret_cast<RE::PLAYER_TARGET_LOC*>((uintptr_t)player + offset);
    }
    else {
        return nullptr;
    }
}

struct PositionPlayerEventSink : public RE::BSTEventSink<RE::PositionPlayerEvent> {
    bool sinkAdded = false;
    RE::TESObjectREFR* fastTravelMarker = nullptr;
    RE::TESObjectREFR* moveToRef = nullptr;
    RE::TESWorldSpace* akWorldSpace = nullptr;
    RE::TESObjectCELL* interiorCell = nullptr;

    RE::BSEventNotifyControl ProcessEvent(const RE::PositionPlayerEvent* event, RE::BSTEventSource<RE::PositionPlayerEvent>*/*source*/) {
        if (!event) {
            logger::error("PositionPlayer Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("PositionPlayer Event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        int type = event->type.underlying();

        if (type == 0) { //pre load
            auto* loc = GetPlayerQueuedTargetLoc();
            fastTravelMarker = nullptr;
            akWorldSpace = nullptr;
            interiorCell = nullptr;

            if (loc) {
                if (!IsBadReadPtr(loc, sizeof(loc))) {
                    fastTravelMarker = gfuncs::GetRefFromHandle(loc->fastTravelMarker);
                    moveToRef = gfuncs::GetRefFromHandle(loc->furnitureRef);
                    akWorldSpace = loc->world;
                    interiorCell = loc->interior;

                    if (!gfuncs::IsFormValid(fastTravelMarker)) {
                        fastTravelMarker = nullptr;
                    }
                    else {
                        akWorldSpace = fastTravelMarker->GetWorldspace();
                        RE::TESObjectCELL* pCell = fastTravelMarker->GetParentCell();
                        if (gfuncs::IsFormValid(pCell)) {
                            if (pCell->IsInteriorCell()) {
                                interiorCell = pCell;
                            }
                        }
                    }

                    if (!gfuncs::IsFormValid(moveToRef)) {
                        moveToRef = nullptr;
                    }

                    if (!gfuncs::IsFormValid(akWorldSpace)) {
                        akWorldSpace = nullptr;
                    }

                    if (!gfuncs::IsFormValid(interiorCell)) {
                        interiorCell = nullptr;
                    }

                    logger::trace("OnPositionPlayerStart: type[{}] \nfastTravelMarker({}) \nmoveToRef({}) \ninterior({}) \nworld({})",
                        type,
                        gfuncs::GetFormDataString(fastTravelMarker),
                        gfuncs::GetFormDataString(moveToRef),
                        gfuncs::GetFormDataString(interiorCell),
                        gfuncs::GetFormDataString(akWorldSpace));
                }
                else {
                    logger::warn("OnPositionPlayerStart: type[{}] loc IsBadReadPtr", type);
                }
            }
            else {
                logger::warn("OnPositionPlayerStart: type[{}] loc not found", type);
            }
            std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnPositionPlayerStart]->globalHandles;
            CombineEventHandles(handles, fastTravelMarker, eventDataPtrs[EventEnum_OnPositionPlayerStart]->eventParamMaps[0]);
            CombineEventHandles(handles, moveToRef, eventDataPtrs[EventEnum_OnPositionPlayerStart]->eventParamMaps[1]);
            CombineEventHandles(handles, akWorldSpace, eventDataPtrs[EventEnum_OnPositionPlayerStart]->eventParamMaps[2]);
            CombineEventHandles(handles, interiorCell, eventDataPtrs[EventEnum_OnPositionPlayerStart]->eventParamMaps[3]);

            gfuncs::RemoveDuplicates(handles);

            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)fastTravelMarker, (RE::TESObjectREFR*)moveToRef,
                (RE::TESWorldSpace*)akWorldSpace, (RE::TESObjectCELL*)interiorCell);

            SendEvents(handles, eventDataPtrs[EventEnum_OnPositionPlayerStart]->sEvent, args);
        }
        else if (type == 4) { //post load
            std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnPositionPlayerFinish]->globalHandles;
            CombineEventHandles(handles, fastTravelMarker, eventDataPtrs[EventEnum_OnPositionPlayerFinish]->eventParamMaps[0]);
            CombineEventHandles(handles, moveToRef, eventDataPtrs[EventEnum_OnPositionPlayerFinish]->eventParamMaps[1]);
            CombineEventHandles(handles, akWorldSpace, eventDataPtrs[EventEnum_OnPositionPlayerFinish]->eventParamMaps[2]);
            CombineEventHandles(handles, interiorCell, eventDataPtrs[EventEnum_OnPositionPlayerFinish]->eventParamMaps[3]);

            gfuncs::RemoveDuplicates(handles);

            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)fastTravelMarker, (RE::TESObjectREFR*)moveToRef,
                (RE::TESWorldSpace*)akWorldSpace, (RE::TESObjectCELL*)interiorCell);

            SendEvents(handles, eventDataPtrs[EventEnum_OnPositionPlayerFinish]->sEvent, args);
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

PositionPlayerEventSink* positionPlayerEventSink;

//used only for the player
struct ActorCellEventSink : public RE::BSTEventSink<RE::BGSActorCellEvent> {
    bool sinkAdded = false;
    RE::FormID previousCellId;

    RE::BSEventNotifyControl ProcessEvent(const RE::BGSActorCellEvent* event, RE::BSTEventSource<RE::BGSActorCellEvent>*/*source*/) {
        if (!event) {
            logger::warn("ActorCellEventSink doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::warn("ActorCellEventSink IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESForm* newCellForm = RE::TESForm::LookupByID(event->cellID);
        RE::TESForm* akPreviousCellForm = RE::TESForm::LookupByID(previousCellId);

        RE::TESObjectCELL* newCell = nullptr;
        RE::TESObjectCELL* akPreviousCell = nullptr;

        if (gfuncs::IsFormValid(newCellForm)) {
            newCell = static_cast<RE::TESObjectCELL*>(newCellForm);
            if (!gfuncs::IsFormValid(newCell)) {
                newCell = gfuncs::playerRef->GetParentCell();
            }
        }

        if (gfuncs::IsFormValid(akPreviousCellForm)) {
            akPreviousCell = static_cast<RE::TESObjectCELL*>(akPreviousCellForm);
            if (!gfuncs::IsFormValid(akPreviousCell)) {
                akPreviousCell = nullptr;
            }
        }

        if (gfuncs::IsFormValid(newCell)) {
            previousCellId = newCell->GetFormID();

            if (newCell != akPreviousCell) {
                logger::trace("PlayerChangedCellEvent: newCell({}) \n previousCell({})",
                    gfuncs::GetFormDataString(newCell), gfuncs::GetFormDataString(akPreviousCell));

                std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnPlayerChangeCell]->globalHandles;
                CombineEventHandles(handles, newCell, eventDataPtrs[EventEnum_OnPlayerChangeCell]->eventParamMaps[0]);
                CombineEventHandles(handles, akPreviousCell, eventDataPtrs[EventEnum_OnPlayerChangeCell]->eventParamMaps[1]);

                gfuncs::RemoveDuplicates(handles);

                auto* args = RE::MakeFunctionArguments((RE::TESObjectCELL*)newCell, (RE::TESObjectCELL*)akPreviousCell);

                SendEvents(handles, eventDataPtrs[EventEnum_OnPlayerChangeCell]->sEvent, args);
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

ActorCellEventSink* actorCellEventSink;

struct ObjectInitEventSink : public RE::BSTEventSink<RE::TESInitScriptEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESInitScriptEvent* event, RE::BSTEventSource<RE::TESInitScriptEvent>*/*source*/) {
        if (!event) {
            //logger::error("ObjectInitEvent doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (event->objectInitialized) {
            RE::TESObjectREFR* loadedRef = event->objectInitialized.get();
            if (gfuncs::IsFormValid(loadedRef)) {
                RE::Actor* akActor = loadedRef->As<RE::Actor>();
                if (gfuncs::IsFormValid(akActor)) {
                    //logger::trace("ObjectInitEvent: Actor[{}] loaded", gfuncs::GetFormName(akActor));
                    savedActorRacesMap[akActor] = akActor->GetRace();
                    return RE::BSEventNotifyControl::kContinue;
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

ObjectInitEventSink* objectInitEventSink;

//TESObjectLoadedEvent
struct ObjectLoadedEventSink : public RE::BSTEventSink<RE::TESObjectLoadedEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*/*source*/) {
        //logger::trace("ObjectLoadedEvent");

        if (!event) {
            //logger::error("load object Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (event->loaded) {
            RE::TESForm* akForm = RE::TESForm::LookupByID(event->formID);

            if (gfuncs::IsFormValid(akForm)) {
                RE::FormType type = akForm->GetFormType();

                RE::Actor* akActor = akForm->As<RE::Actor>();
                if (gfuncs::IsFormValid(akActor)) {
                    logger::trace("ObjectLoadedEvent: Actor[{}] loaded", gfuncs::GetFormName(akActor));
                    return RE::BSEventNotifyControl::kContinue;
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

ObjectLoadedEventSink* objectLoadedEventSink;

bool IsItemMenuOpen() {
    for (auto& menu : itemMenus) {
        if (ui->IsMenuOpen(menu)) {
            return true;
        }
    }
    return false;
}

class InputEventSink : public RE::BSTEventSink<RE::InputEvent*> {

public:
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*) {

        //logger::trace("input event");

        //don't want to send ui select events to papyrus if message box context is open, only when selecting item.
        if (ui->IsMenuOpen(RE::MessageBoxMenu::MENU_NAME)) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto* event = *eventPtr;
        if (!event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (event->eventType == RE::INPUT_EVENT_TYPE::kButton) {
            auto* buttonEvent = event->AsButtonEvent();
            if (buttonEvent) {
                if (buttonEvent->IsDown()) {
                    if (buttonEvent->GetIDCode() == 28) { //enter key pressed
                        if (IsItemMenuOpen()) {
                            UIEvents::ProcessUiItemSelectEvent();
                            logger::trace("input Event: button[{}] pressed. ProcessUiItemSelectEvent", buttonEvent->GetIDCode());
                        }
                    }
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

InputEventSink* inputEventSink;

struct MenuOpenCloseEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*/*source*/) {
        //this sink is for managing timers and GetCurrentMenuOpen function.

        if (!event) {
            logger::warn("MenuOpenClose Event doesn't exist!");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::warn("AnimationEventSink IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        //logger::trace("Menu Open Close Event, menu[{}], opened[{}]", event->menuName, event->opening);

        if (event->menuName != RE::HUDMenu::MENU_NAME) { //hud menu is always open, don't need to do anything for it.

            RE::BSFixedString bsMenuName = event->menuName;

            if (event->opening) {
                lastMenuOpened = event->menuName;

                if (ui->GameIsPaused() && !gamePaused) {
                    gamePaused = true;
                    lastTimeGameWasPaused = std::chrono::system_clock::now();
                    logger::trace("game was paused");
                }

                if (!inMenuMode) { //opened menu
                    inMenuMode = true;
                    lastTimeMenuWasOpened = std::chrono::system_clock::now();
                    logger::trace("inMenuMode = true");
                }
                menusCurrentlyOpen.push_back(event->menuName);

                if (gfuncs::GetIndexInVector(refActivatedMenus, bsMenuName) > -1) {
                    gfuncs::menuRef = gfuncs::lastPlayerActivatedRef;
                    logger::trace("Menu[{}] opened. saved menuRef[{}]", bsMenuName, gfuncs::GetFormName(gfuncs::menuRef));
                }

                if (gfuncs::GetIndexInVector(itemMenus, bsMenuName) > -1) {
                    numOfItemMenusCurrentOpen += 1;

                    if (UIEvents::registeredUIEventDatas.size() > 0) {
                        if (!inputEventSink->sinkAdded) {
                            inputEventSink->sinkAdded = true;
                            RE::BSInputDeviceManager::GetSingleton()->AddEventSink(inputEventSink);
                            logger::trace("item menu [{}] opened. Added input event sink", bsMenuName);
                        }
                    }
                }
            }
            else {
                if (!ui->GameIsPaused() && gamePaused) {
                    gamePaused = false;
                    auto now = std::chrono::system_clock::now();
                    UpdateTimers(gfuncs::timePointDiffToFloat(now, lastTimeGameWasPaused));
                    logger::trace("game was unpaused");
                }

                //menusCurrentlyOpen.erase(std::remove(menusCurrentlyOpen.begin(), menusCurrentlyOpen.end(), event->menuName), menusCurrentlyOpen.end());
                gfuncs::RemoveFromVectorByValue(menusCurrentlyOpen, event->menuName);
                CheckMenusCurrentlyOpen();
                //logger::trace("menu close, number of menusCurrentlyOpen = {}", menusCurrentlyOpen.size());

                if (menusCurrentlyOpen.size() == 0) { //closed menu
                    inMenuMode = false;
                    auto now = std::chrono::system_clock::now();
                    UpdateNoMenuModeTimers(gfuncs::timePointDiffToFloat(now, lastTimeMenuWasOpened));
                    logger::trace("inMenuMode = false");
                }

                if (gfuncs::GetIndexInVector(itemMenus, bsMenuName) > -1) {
                    numOfItemMenusCurrentOpen -= 1;

                    if (numOfItemMenusCurrentOpen == 0) {
                        if (inputEventSink->sinkAdded) {
                            inputEventSink->sinkAdded = false;
                            RE::BSInputDeviceManager::GetSingleton()->RemoveEventSink(inputEventSink);
                            logger::trace("item menu [{}] closed. Removed input event sink", bsMenuName);
                        }
                    }
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

HitEventSink* hitEventSink;
CombatEventSink* combatEventSink;
FurnitureEventSink* furnitureEventSink;
ActivateEventSink* activateEventSink;
DeathEventSink* deathEventSink;
EquipEventSink* equipEventSink;
WaitStartEventSink* waitStartEventSink;
WaitStopEventSink* waitStopEventSink;
MagicEffectApplyEventSink* magicEffectApplyEventSink;
LockChangedEventSink* lockChangedEventSink;
OpenCloseEventSink* openCloseEventSink;
SpellCastEventSink* spellCastEventSink;
ActorActionEventSink* actorActionEventSink;
ContainerChangedEventSink* containerChangedEventSink;
MenuOpenCloseEventSink* menuOpenCloseEventSink;

bool ShouldActorActionEventSinkBeAdded() {
    for (int i = EventEnum_OnWeaponSwing; i <= EventEnum_EndSheathe; i++) {
        if (!eventDataPtrs[i]->isEmpty()) {
            return true;
        }
    }
    return false;
}

//hit events use the equip event to track equipped ammo on actors
bool AddEquipEventSink() {
    logger::trace("{}", __func__);
    if (!eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty() || !eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty() || !eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
        if (!equipEventSink->sinkAdded) {
            equipEventSink->sinkAdded = true;
            eventSourceholder->AddEventSink(equipEventSink);
            RegisterActorsForBowDrawAnimEvents();
            logger::debug("{} Equip Event Sink Added", __func__);
            return true;
        }
    }
    return false;
}

bool RemoveEquipEventSink() {
    logger::trace("{}", __func__);
    if (eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty() && eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty() && eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
        if (equipEventSink->sinkAdded) {
            equipEventSink->sinkAdded = false;
            eventSourceholder->RemoveEventSink(equipEventSink); //always added to track recent hit projectiles for the GetRecentHitArrowRefsMap function
            logger::debug("{} Equip Event Sink Removed", __func__);
            return true;
        }
    }
    return false;
}

bool AddHitEventSink() {
    if (!hitEventSink->sinkAdded && (!eventDataPtrs[EventEnum_HitEvent]->isEmpty() || !eventDataPtrs[EventEnum_OnProjectileImpact]->isEmpty())) {
        hitEventSink->sinkAdded = true;
        eventSourceholder->AddEventSink(hitEventSink);
        logger::trace("{}", __func__);
        return true;
    }
    return false;
}

bool RemoveHitEventSink() {
    if (hitEventSink->sinkAdded && eventDataPtrs[EventEnum_HitEvent]->isEmpty() && eventDataPtrs[EventEnum_OnProjectileImpact]->isEmpty()) {
        hitEventSink->sinkAdded = false;
        eventSourceholder->RemoveEventSink(hitEventSink);
        logger::debug("{}", __func__);
        return true;
    }
    return false;
}

bool ShouldPositionPlayerEventSinkBeAdded() {
    return(!eventDataPtrs[EventEnum_OnPositionPlayerStart]->isEmpty() || !eventDataPtrs[EventEnum_OnPositionPlayerFinish]->isEmpty());
}

void AddSink(int index) {
    switch (index) {
    case EventEnum_OnCombatStateChanged:
        if (!combatEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnCombatStateChanged]->isEmpty()) {
            combatEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnCombatStateChanged]->sinkAdded = true;
            eventSourceholder->AddEventSink(combatEventSink);
            logger::debug("{}, EventEnum_OnCombatStateChanged sink added", __func__);
        }
        break;

    case EventEnum_FurnitureEnter:

    case EventEnum_FurnitureExit:
        if (!furnitureEventSink->sinkAdded && (!eventDataPtrs[EventEnum_FurnitureEnter]->isEmpty() || !eventDataPtrs[EventEnum_FurnitureExit]->isEmpty())) {
            furnitureEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded = true;
            eventSourceholder->AddEventSink(furnitureEventSink);
            logger::debug("{}, EventEnum_FurnitureExit sink added", __func__);
        }
        break;

    case EventEnum_OnActivate:
        if (!activateEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnActivate]->isEmpty()) {
            activateEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnActivate]->sinkAdded = true;
            //eventSourceholder->AddEventSink(activateEventSink); //always activate to track lastPlayerActivatedRef
            logger::debug("{}, EventEnum_OnActivate sink added", __func__);
        }
        break;

    case EventEnum_HitEvent:
        if (AddHitEventSink()) {
            //logger::debug("{}, EventEnum_hitEvent sink added", __func__);
        }
        if (AddEquipEventSink()); {
            logger::debug("{}, EventEnum_HitEvent Equip event sink added", __func__);
        }
        if (!eventDataPtrs[EventEnum_HitEvent]->sinkAdded && !eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
            eventDataPtrs[EventEnum_HitEvent]->sinkAdded = true;
            logger::debug("{}, EventEnum_hitEvent sink added", __func__);
        }
        break;

    case EventEnum_DeathEvent:

    case EventEnum_DyingEvent:
        if (!deathEventSink->sinkAdded && (!eventDataPtrs[EventEnum_DeathEvent]->isEmpty() || !eventDataPtrs[EventEnum_DyingEvent]->isEmpty())) {
            deathEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_DeathEvent]->sinkAdded = true;
            eventSourceholder->AddEventSink(deathEventSink);
            logger::debug("{}, EventEnum_DyingEvent sink added", __func__);
        }
        break;

    case EventEnum_OnObjectEquipped:
        if (!eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty()) {
            eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded = true;
            logger::debug("{}, EventEnum_OnObjectEquipped sink added", __func__);
        }
        if (AddEquipEventSink()) {
            logger::debug("{}, EventEnum_OnObjectEquipped Equip event sink added", __func__);
        }
        break;

    case EventEnum_OnObjectUnequipped:
        if (!eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty()) {
            eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded = true;
            logger::debug("{}, EventEnum_OnObjectUnequipped sink added", __func__);
        }
        if (AddEquipEventSink()) {
            logger::debug("{}, EventEnum_OnObjectUnequipped Equip event sink added", __func__);
        }
        break;

    case EventEnum_OnWaitStart:
        if (!waitStartEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnWaitStart]->isEmpty()) {
            waitStartEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnWaitStart]->sinkAdded = true;
            eventSourceholder->AddEventSink(waitStartEventSink);
            logger::debug("{}, EventEnum_OnWaitStart sink added", __func__);
        }
        break;

    case EventEnum_OnWaitStop:
        if (!waitStopEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnWaitStop]->isEmpty()) {
            waitStopEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnWaitStop]->sinkAdded = true;
            eventSourceholder->AddEventSink(waitStopEventSink);
            logger::debug("{}, EventEnum_OnWaitStop sink added", __func__);
        }
        break;

    case EventEnum_OnMagicEffectApply:
        if (!magicEffectApplyEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnMagicEffectApply]->isEmpty()) {
            magicEffectApplyEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnMagicEffectApply]->sinkAdded = true;
            eventSourceholder->AddEventSink(magicEffectApplyEventSink);
            logger::debug("{}, EventEnum_OnMagicEffectApply sink added", __func__);
        }
        break;

    case EventEnum_OnSpellCast:
        if (!spellCastEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnSpellCast]->isEmpty()) {
            spellCastEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnSpellCast]->sinkAdded = true;
            eventSourceholder->AddEventSink(spellCastEventSink);
            logger::debug("{}, EventEnum_OnSpellCast sink added", __func__);
        }
        break;

    case EventEnum_OnContainerChanged:
        if (!containerChangedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnContainerChanged]->isEmpty()) {
            containerChangedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnContainerChanged]->sinkAdded = true;
            eventSourceholder->AddEventSink(containerChangedEventSink);
            logger::debug("{}, EventEnum_OnContainerChanged sink added", __func__);
        }
        break;

    case EventEnum_LockChanged:
        if (!lockChangedEventSink->sinkAdded && !eventDataPtrs[EventEnum_LockChanged]->isEmpty()) {
            lockChangedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_LockChanged]->sinkAdded = true;
            eventSourceholder->AddEventSink(lockChangedEventSink);
            logger::debug("{}, EventEnum_LockChanged sink added", __func__);
        }
        break;

    case EventEnum_OnOpen:

    case EventEnum_OnClose:
        if (!openCloseEventSink->sinkAdded && (!eventDataPtrs[EventEnum_OnOpen]->isEmpty() || !eventDataPtrs[EventEnum_OnClose]->isEmpty())) {
            openCloseEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnOpen]->sinkAdded = true;
            eventSourceholder->AddEventSink(openCloseEventSink);
            logger::debug("{}, EventEnum_OnClose sink added", __func__);
        }
        break;

        //actorActionEventSink events
    case EventEnum_OnWeaponSwing:
    case EventEnum_OnActorSpellCast:
    case EventEnum_OnActorSpellFire:
    case EventEnum_VoiceCast:
    case EventEnum_VoiceFire:
    case EventEnum_BowDraw:
    case EventEnum_BowRelease:
    case EventEnum_BeginDraw:
    case EventEnum_EndDraw:
    case EventEnum_BeginSheathe:

    case EventEnum_EndSheathe:
        if (!actorActionEventSink->sinkAdded && ShouldActorActionEventSinkBeAdded()) {
            actorActionEventSink->sinkAdded = true;
            logger::debug("{}, actorActionEventSink sink added", __func__);
            eventDataPtrs[EventEnum_EndSheathe]->sinkAdded = true;
            SKSE::GetActionEventSource()->AddEventSink(actorActionEventSink);
        }
        break;

    case EventEnum_OnProjectileImpact:
        if (AddHitEventSink()) {

        }
        if (!eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded && !eventDataPtrs[EventEnum_OnProjectileImpact]->isEmpty()) {
            eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded = true;
            logger::debug("{}, EventEnum_OnProjectileImpact sink added", __func__);
        }
        break;

    case EventEnum_OnItemCrafted:
        if (!itemCraftedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnItemCrafted]->isEmpty()) {
            itemCraftedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnItemCrafted]->sinkAdded = true;
            UIEvents::InstallUiEventHook(UIEvents::UiItemMenuEnum_Crafting_Menu); //track crafting menu selection data, (mostly needed for item count).
            RE::ItemCrafted::GetEventSource()->AddEventSink(itemCraftedEventSink);
            logger::debug("{}, EventEnum_OnItemCrafted sink added", __func__);
        }
        break;

    case EventEnum_OnItemsPickpocketed:
        if (!itemsPickpocketedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnItemsPickpocketed]->isEmpty()) {
            itemsPickpocketedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnItemsPickpocketed]->sinkAdded = true;
            UIEvents::InstallUiEventHook(UIEvents::UiItemMenuEnum_ContainerMenu); //track container menu selection data, (to get the form taken / pickpocketed).
            RE::ItemsPickpocketed::GetEventSource()->AddEventSink(itemsPickpocketedEventSink);
            logger::debug("{}, EventEnum_OnItemsPickpocketed sink added", __func__);
        }
        break;

    case EventEnum_OnLocationCleared:
        if (!locationClearedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnLocationCleared]->isEmpty()) {
            locationClearedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnLocationCleared]->sinkAdded = true;
            RE::LocationCleared::GetEventSource()->AddEventSink(locationClearedEventSink);
            logger::debug("{}, EventEnum_OnLocationCleared sink added", __func__);
        }
        break;

    case EventEnum_OnEnterBleedout:
        if (!enterBleedoutEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnEnterBleedout]->isEmpty()) {
            enterBleedoutEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnEnterBleedout]->sinkAdded = true;
            eventSourceholder->AddEventSink(enterBleedoutEventSink);
            logger::debug("{}, EventEnum_OnEnterBleedout sink added", __func__);
        }
        break;

    case EventEnum_OnSwitchRaceComplete:
        if (!switchRaceCompleteEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnSwitchRaceComplete]->isEmpty()) {
            switchRaceCompleteEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnSwitchRaceComplete]->sinkAdded = true;
            SaveActorRaces(); //save current actors loaded in game race's
            eventSourceholder->AddEventSink(switchRaceCompleteEventSink);
            eventSourceholder->AddEventSink(objectInitEventSink); //save new actors loaded races to send akOldRace parameter on switchRaceComplete event
            logger::debug("{}, EventEnum_OnSwitchRaceComplete sink added", __func__);
        }
        break;

    case EventEnum_OnActorFootStep:
        if (!footstepEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnActorFootStep]->isEmpty()) {
            footstepEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnActorFootStep]->sinkAdded = true;
            RE::BGSFootstepManager::GetSingleton()->AddEventSink(footstepEventSink);
            logger::debug("{}, EventEnum_OnActorFootStep sink added", __func__);
        }
        break;

    case EventEnum_OnQuestObjectiveStateChanged:
        if (!questObjectiveEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->isEmpty()) {
            questObjectiveEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->sinkAdded = true;
            RE::ObjectiveState::GetEventSource()->AddEventSink(questObjectiveEventSink);
            logger::debug("{}, EventEnum_OnQuestObjectiveStateChanged sink added", __func__);
        }
        break;

    case EventEnum_OnPositionPlayerStart:

    case EventEnum_OnPositionPlayerFinish:
        if (!positionPlayerEventSink->sinkAdded && ShouldPositionPlayerEventSinkBeAdded()) {
            auto* posPlayerEventSource = player->AsPositionPlayerEventSource();
            if (posPlayerEventSource) {
                positionPlayerEventSink->sinkAdded = true;
                posPlayerEventSource->AddEventSink(positionPlayerEventSink);
                logger::debug("{}, positionPlayerEventSink added", __func__);
            }
            else {
                logger::error("{}, posPlayerEventSource not found, positionPlayerEventSink sink not added", __func__);
            }
        }
        break;

    case EventEnum_OnPlayerChangeCell:
        if (!actorCellEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnPlayerChangeCell]->isEmpty()) {
            auto* playerCellChangeSource = player->AsBGSActorCellEventSource();
            if (playerCellChangeSource) {
                actorCellEventSink->sinkAdded = true;
                eventDataPtrs[EventEnum_OnPlayerChangeCell]->sinkAdded = true;
                //actorCellEventSink->previousCell = gfuncs::playerRef->GetParentCell();
                playerCellChangeSource->AddEventSink(actorCellEventSink);
                logger::debug("{}, EventEnum_OnPlayerChangeCell sink added", __func__);
            }
            else {
                logger::error("{}, playerCellChangeSource not found, EventEnum_OnPlayerChangeCell sink not added", __func__);
            }
        }
        break;
    }
}

void RemoveSink(int index) {
    logger::debug("removing sink {}", index);

    switch (index) {
    case EventEnum_OnCombatStateChanged:
        if (combatEventSink->sinkAdded && eventDataPtrs[EventEnum_OnCombatStateChanged]->isEmpty()) {
            combatEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnCombatStateChanged]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(combatEventSink);
            logger::debug("{}, EventEnum_OnCombatStateChanged sink removed", __func__);
        }
        break;

    case EventEnum_FurnitureEnter:

    case EventEnum_FurnitureExit:
        if (furnitureEventSink->sinkAdded && eventDataPtrs[EventEnum_FurnitureEnter]->isEmpty() && eventDataPtrs[EventEnum_FurnitureExit]->isEmpty()) {
            furnitureEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(furnitureEventSink);
            logger::debug("{}, EventEnum_FurnitureEnter sink removed", __func__);
        }
        break;

    case EventEnum_OnActivate:
        if (activateEventSink->sinkAdded && eventDataPtrs[EventEnum_OnActivate]->isEmpty()) {
            activateEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnActivate]->sinkAdded = false;
            //eventSourceholder->RemoveEventSink(activateEventSink); //always activate to track lastPlayerActivatedRef
            logger::debug("{}, EventEnum_OnActivate sink removed", __func__);
        }
        break;

    case EventEnum_HitEvent:
        if (RemoveHitEventSink()) {
            //logger::debug("{}, EventEnum_hitEvent sink removed", __func__);
        }
        if (RemoveEquipEventSink()) {
            logger::debug("{}, EventEnum_HitEvent equip event sink removed", __func__);
        }
        if (eventDataPtrs[EventEnum_HitEvent]->sinkAdded && eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
            eventDataPtrs[EventEnum_HitEvent]->sinkAdded = false;
            logger::debug("{}, EventEnum_hitEvent sink removed", __func__);
        }
        break;

    case EventEnum_DeathEvent:

    case EventEnum_DyingEvent:
        if (deathEventSink->sinkAdded && eventDataPtrs[EventEnum_DeathEvent]->isEmpty() && eventDataPtrs[EventEnum_DyingEvent]->isEmpty()) {
            deathEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_DeathEvent]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(deathEventSink);
            logger::debug("{}, EventEnum_DeathEvent sink removed", __func__);
        }
        break;

    case EventEnum_OnObjectEquipped:
        if (eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded && eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty()) {
            eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded = false;
            logger::debug("{}, EventEnum_OnObjectEquipped sink removed", __func__);
        }
        if (RemoveEquipEventSink()) {
            logger::debug("{}, EventEnum_OnObjectEquipped equip event sink removed", __func__);
        }
        break;

    case EventEnum_OnObjectUnequipped:
        if (eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded && eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty()) {
            eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded = false;
            logger::debug("{}, EventEnum_OnObjectUnequipped sink removed", __func__);
        }
        if (RemoveEquipEventSink()) {
            logger::debug("{}, EventEnum_OnObjectUnequipped equip event sink removed", __func__);
        }
        break;

    case EventEnum_OnWaitStart:
        if (waitStartEventSink->sinkAdded && eventDataPtrs[EventEnum_OnWaitStart]->isEmpty()) {
            waitStartEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnWaitStart]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(waitStartEventSink);
            logger::debug("{}, EventEnum_OnWaitStart sink removed", __func__);
        }
        break;

    case EventEnum_OnWaitStop:
        if (waitStopEventSink->sinkAdded && eventDataPtrs[EventEnum_OnWaitStop]->isEmpty()) {
            waitStopEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnWaitStop]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(waitStopEventSink);
            logger::debug("{}, EventEnum_OnWaitStop sink removed", __func__);
        }
        break;

    case EventEnum_OnMagicEffectApply:
        if (magicEffectApplyEventSink->sinkAdded && eventDataPtrs[EventEnum_OnMagicEffectApply]->isEmpty()) {
            magicEffectApplyEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnMagicEffectApply]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(magicEffectApplyEventSink);
            logger::debug("{}, EventEnum_OnMagicEffectApply sink removed", __func__);
        }
        break;

    case EventEnum_OnSpellCast:
        if (spellCastEventSink->sinkAdded && eventDataPtrs[EventEnum_OnSpellCast]->isEmpty()) {
            spellCastEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnSpellCast]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(spellCastEventSink);
            logger::debug("{}, EventEnum_OnSpellCast sink removed", __func__);
        }
        break;

    case EventEnum_OnContainerChanged:
        if (containerChangedEventSink->sinkAdded && eventDataPtrs[EventEnum_OnContainerChanged]->isEmpty()) {
            containerChangedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnContainerChanged]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(containerChangedEventSink);
            logger::debug("{}, EventEnum_OnContainerChanged sink removed", __func__);
        }
        break;

    case EventEnum_LockChanged:
        if (lockChangedEventSink->sinkAdded && eventDataPtrs[EventEnum_LockChanged]->isEmpty()) {
            lockChangedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_LockChanged]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(lockChangedEventSink);
            logger::debug("{}, EventEnum_LockChanged sink removed", __func__);
        }
        break;

    case EventEnum_OnOpen:

    case EventEnum_OnClose:
        if (openCloseEventSink->sinkAdded && eventDataPtrs[EventEnum_OnOpen]->isEmpty() && eventDataPtrs[EventEnum_OnClose]->isEmpty()) {
            openCloseEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnOpen]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(openCloseEventSink);
            logger::debug("{}, EventEnum_OnOpen sink removed", __func__);
        }
        break;

        //actorActionEventSink events
    case EventEnum_OnWeaponSwing:
    case EventEnum_OnActorSpellCast:
    case EventEnum_OnActorSpellFire:
    case EventEnum_VoiceCast:
    case EventEnum_VoiceFire:
    case EventEnum_BowDraw:
    case EventEnum_BowRelease:
    case EventEnum_BeginDraw:
    case EventEnum_EndDraw:
    case EventEnum_BeginSheathe:

    case EventEnum_EndSheathe:
        if (actorActionEventSink->sinkAdded && !ShouldActorActionEventSinkBeAdded()) {
            actorActionEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_EndSheathe]->sinkAdded = false;
            SKSE::GetActionEventSource()->RemoveEventSink(actorActionEventSink);
            logger::debug("{}, actorActionEventSink sink removed", __func__);
        }
        break;

    case EventEnum_OnProjectileImpact:
        if (RemoveHitEventSink()) {

        }
        if (eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded && eventDataPtrs[EventEnum_OnProjectileImpact]->isEmpty()) {
            eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded = false;
            logger::debug("{}, EventEnum_OnProjectileImpact sink removed", __func__);
        }
        break;

    case EventEnum_OnItemCrafted:
        if (itemCraftedEventSink->sinkAdded && eventDataPtrs[EventEnum_OnItemCrafted]->isEmpty()) {
            itemCraftedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnItemCrafted]->sinkAdded = false;
            RE::ItemCrafted::GetEventSource()->RemoveEventSink(itemCraftedEventSink);
            logger::debug("{}, EventEnum_OnItemCrafted sink removed", __func__);
        }
        break;

    case EventEnum_OnItemsPickpocketed:
        if (itemsPickpocketedEventSink->sinkAdded && eventDataPtrs[EventEnum_OnItemsPickpocketed]->isEmpty()) {
            itemsPickpocketedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnItemsPickpocketed]->sinkAdded = false;
            RE::ItemsPickpocketed::GetEventSource()->RemoveEventSink(itemsPickpocketedEventSink);
            logger::debug("{}, EventEnum_OnItemsPickpocketed sink removed", __func__);
        }
        break;

    case EventEnum_OnLocationCleared:
        if (locationClearedEventSink->sinkAdded && eventDataPtrs[EventEnum_OnLocationCleared]->isEmpty()) {
            locationClearedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnLocationCleared]->sinkAdded = false;
            RE::LocationCleared::GetEventSource()->RemoveEventSink(locationClearedEventSink);
            logger::debug("{}, EventEnum_OnLocationCleared sink removed", __func__);
        }
        break;

    case EventEnum_OnEnterBleedout:
        if (enterBleedoutEventSink->sinkAdded && eventDataPtrs[EventEnum_OnEnterBleedout]->isEmpty()) {
            enterBleedoutEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnEnterBleedout]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(enterBleedoutEventSink);
            logger::debug("{}, EventEnum_OnEnterBleedout sink removed", __func__);
        }
        break;

    case EventEnum_OnSwitchRaceComplete:
        if (switchRaceCompleteEventSink->sinkAdded && eventDataPtrs[EventEnum_OnSwitchRaceComplete]->isEmpty()) {
            switchRaceCompleteEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnSwitchRaceComplete]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(switchRaceCompleteEventSink);
            eventSourceholder->RemoveEventSink(objectInitEventSink);
            actorRacesSaved = false;
            logger::debug("{}, EventEnum_OnSwitchRaceComplete sink removed", __func__);
        }
        break;

    case EventEnum_OnActorFootStep:
        if (footstepEventSink->sinkAdded && eventDataPtrs[EventEnum_OnActorFootStep]->isEmpty()) {
            footstepEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnActorFootStep]->sinkAdded = false;
            RE::BGSFootstepManager::GetSingleton()->RemoveEventSink(footstepEventSink);
            logger::debug("{}, EventEnum_OnActorFootStep sink removed", __func__);
        }
        break;

    case EventEnum_OnQuestObjectiveStateChanged:
        if (questObjectiveEventSink->sinkAdded && eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->isEmpty()) {
            questObjectiveEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->sinkAdded = false;
            RE::ObjectiveState::GetEventSource()->RemoveEventSink(questObjectiveEventSink);
            logger::debug("{}, EventEnum_OnQuestObjectiveStateChanged sink removed", __func__);
        }
        break;

    case EventEnum_OnPositionPlayerStart:

    case EventEnum_OnPositionPlayerFinish:
        if (positionPlayerEventSink->sinkAdded && !ShouldPositionPlayerEventSinkBeAdded()) {
            auto posPlayerEventSource = player->AsPositionPlayerEventSource();
            if (posPlayerEventSource) {
                positionPlayerEventSink->sinkAdded = false;
                posPlayerEventSource->RemoveEventSink(positionPlayerEventSink);
                logger::debug("{}, positionPlayerEventSink removed", __func__);
            }
            else {
                logger::error("{}, posPlayerEventSource not found, positionPlayerEventSink sink not removed", __func__);
            }
        }
        break;

    case EventEnum_OnPlayerChangeCell:
        if (actorCellEventSink->sinkAdded && eventDataPtrs[EventEnum_OnPlayerChangeCell]->isEmpty()) {
            auto* playerCellChangeSource = player->AsBGSActorCellEventSource();
            if (playerCellChangeSource) {
                actorCellEventSink->sinkAdded = false;
                eventDataPtrs[EventEnum_OnPlayerChangeCell]->sinkAdded = false;
                playerCellChangeSource->RemoveEventSink(actorCellEventSink);
                logger::debug("{}, EventEnum_OnPlayerChangeCell sink removed", __func__);
            }
            else {
                logger::error("{}, playerCellChangeSource not found, EventEnum_OnPlayerChangeCell sink not removed", __func__);
            }
        }
        break;
    }
}

int GetEventIndex(std::vector<EventData*> v, RE::BSFixedString asEvent) {
    if (asEvent == "") {
        return -1;
    }

    int m = v.size();
    if (m == 0) {
        return -1;
    }

    for (int i = 0; i < m; i++) {
        if (v[i]->sEvent == asEvent) {
            return i;
        }
    }

    return -1;
}

//global events ==========================================================================================================================================================================================

// is registered
bool IsFormRegisteredForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{} {}", __func__, asEvent);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return false;
    }

    logger::trace("{} getting handle is registered for: {}", __func__, gfuncs::GetFormName(eventReceiver));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return false;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        return eventDataPtrs[index]->HasHandle(handle, paramFilter, paramFilterIndex);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
        return false;
    }
}

bool IsAliasRegisteredForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::BGSBaseAlias* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return false;
    }

    logger::trace("{} getting handle is registered for: {}", __func__, eventReceiver->aliasName);

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->GetVMTypeID());
        return false;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        return eventDataPtrs[index]->HasHandle(handle, paramFilter, paramFilterIndex);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
        return false;
    }
}

bool IsActiveMagicEffectRegisteredForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::ActiveEffect* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return false;
    }

    logger::trace("{} getting handle is registered for: {} instance", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for effect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->VMTYPEID);
        return false;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        return eventDataPtrs[index]->HasHandle(handle, paramFilter, paramFilterIndex);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
        return false;
    }
}

bool RegisterFormListForGlobalEvent(RE::BGSListForm* list, int eventIndex, int paramFilterIndex, RE::VMHandle& handle) {
    if (!gfuncs::IsFormValid(list)) {
        return false;
    }

    int iCount = 0;

    list->ForEachForm([&](auto& akForm) {
        auto* form = &akForm;
        if (gfuncs::IsFormValid(form)) {
            iCount++;
            RE::BGSListForm* nestedList = form->As<RE::BGSListForm>();
            if (gfuncs::IsFormValid(nestedList)) {
                RegisterFormListForGlobalEvent(nestedList, eventIndex, paramFilterIndex, handle);
            }
            else {
                eventDataPtrs[eventIndex]->AddHandle(handle, form, paramFilterIndex);
            }
        }
        return RE::BSContainer::ForEachResult::kContinue;
        });

    int size = list->forms.size();

    logger::trace("{}: size[{}] scriptAddedFormCount[{}] iCount[{}]", __func__, size, list->scriptAddedFormCount, iCount);

    return true;
}

bool UnregisterFormListForGlobalEvent(RE::BGSListForm* list, int eventIndex, int paramFilterIndex, RE::VMHandle& handle) {
    if (!gfuncs::IsFormValid(list)) {
        return false;
    }

    list->ForEachForm([&](auto& akForm) {
        auto* form = &akForm;
        if (gfuncs::IsFormValid(form)) {
            RE::BGSListForm* nestedList = form->As<RE::BGSListForm>();
            if (gfuncs::IsFormValid(nestedList)) {
                UnregisterFormListForGlobalEvent(nestedList, eventIndex, paramFilterIndex, handle);
            }
            else {
                eventDataPtrs[eventIndex]->RemoveHandle(handle, form, paramFilterIndex, false);
            }
        }
        return RE::BSContainer::ForEachResult::kContinue;
        });

    int size = list->forms.size();

    logger::trace("{}: size[{}] scriptAddedFormCount[{}]", __func__, size, list->scriptAddedFormCount);

    return true;
}

//register
void RegisterFormForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{} {}", __func__, asEvent);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    logger::trace("{} adding handle for: {}", __func__, gfuncs::GetFormName(eventReceiver));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        bool paramFilterIsFormList = false;
        if (gfuncs::IsFormValid(paramFilter)) {
            auto* akFormList = paramFilter->As<RE::BGSListForm>();
            if (gfuncs::IsFormValid(akFormList)) {
                paramFilterIsFormList = true;
                if (RegisterFormListForGlobalEvent(akFormList, index, paramFilterIndex, handle)) {
                    AddSink(index);
                }
                else {
                    logger::warn("{}: passed in paramFilter formlist[{}] for event [{}] has no forms to register", __func__, gfuncs::GetFormName(akFormList), asEvent);
                }
            }
        }

        if (!paramFilterIsFormList) {
            eventDataPtrs[index]->AddHandle(handle, paramFilter, paramFilterIndex);
            AddSink(index);
        }
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

void RegisterAliasForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::BGSBaseAlias* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    logger::trace("{} adding handle for: {}", __func__, eventReceiver->aliasName);

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->GetVMTypeID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        bool paramFilterIsFormList = false;
        if (gfuncs::IsFormValid(paramFilter)) {
            auto* akFormList = paramFilter->As<RE::BGSListForm>();
            if (gfuncs::IsFormValid(akFormList)) {
                paramFilterIsFormList = true;
                if (RegisterFormListForGlobalEvent(akFormList, index, paramFilterIndex, handle)) {
                    AddSink(index);
                }
                else {
                    logger::warn("{}: passed in paramFilter formlist[{}] for event [{}] has no forms to register", __func__, gfuncs::GetFormName(akFormList), asEvent);
                }
            }
        }

        if (!paramFilterIsFormList) {
            eventDataPtrs[index]->AddHandle(handle, paramFilter, paramFilterIndex);
            AddSink(index);
        }
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

void RegisterActiveMagicEffectForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::ActiveEffect* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    logger::trace("{} adding handle for: {} instance", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for effect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->VMTYPEID);
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        bool paramFilterIsFormList = false;
        if (gfuncs::IsFormValid(paramFilter)) {
            auto* akFormList = paramFilter->As<RE::BGSListForm>();
            if (gfuncs::IsFormValid(akFormList)) {
                paramFilterIsFormList = true;
                if (RegisterFormListForGlobalEvent(akFormList, index, paramFilterIndex, handle)) {
                    AddSink(index);
                }
                else {
                    logger::warn("{}: passed in paramFilter formlist[{}] for event [{}] has no forms to register", __func__, gfuncs::GetFormName(akFormList), asEvent);
                }
            }
        }

        if (!paramFilterIsFormList) {
            eventDataPtrs[index]->AddHandle(handle, paramFilter, paramFilterIndex);
            AddSink(index);
        }
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

//unregister
void UnregisterFormForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{} {}", __func__, asEvent);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    logger::trace("{} removing handle for: {}", __func__, gfuncs::GetFormName(eventReceiver));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        bool paramFilterIsFormList = false;
        if (gfuncs::IsFormValid(paramFilter)) {
            auto* akFormList = paramFilter->As<RE::BGSListForm>();
            if (gfuncs::IsFormValid(akFormList)) {
                bool paramFilterIsFormList = true;
                if (UnregisterFormListForGlobalEvent(akFormList, index, paramFilterIndex, handle)) {
                    eventDataPtrs[index]->CheckAndRemoveSink();
                }
                else {
                    logger::warn("{}: passed in paramFilter formlist[{}] for event [{}] has no forms to unregister", __func__, gfuncs::GetFormName(akFormList), asEvent);
                }
            }
        }

        if (!paramFilterIsFormList) {
            eventDataPtrs[index]->RemoveHandle(handle, paramFilter, paramFilterIndex);
        }
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

void UnregisterAliasForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::BGSBaseAlias* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    logger::trace("{} removing handle for: {}", __func__, eventReceiver->aliasName);

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->GetVMTypeID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        bool paramFilterIsFormList = false;
        if (gfuncs::IsFormValid(paramFilter)) {
            auto* akFormList = paramFilter->As<RE::BGSListForm>();
            if (gfuncs::IsFormValid(akFormList)) {
                bool paramFilterIsFormList = true;
                if (UnregisterFormListForGlobalEvent(akFormList, index, paramFilterIndex, handle)) {
                    eventDataPtrs[index]->CheckAndRemoveSink();
                }
                else {
                    logger::warn("{}: passed in paramFilter formlist[{}] for event [{}] has no forms to unregister", __func__, gfuncs::GetFormName(akFormList), asEvent);
                }
            }
        }

        if (!paramFilterIsFormList) {
            eventDataPtrs[index]->RemoveHandle(handle, paramFilter, paramFilterIndex);
        }
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

void UnregisterActiveMagicEffectForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::ActiveEffect* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    logger::trace("{} removing handle for: {} instance", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for effect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->VMTYPEID);
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        bool paramFilterIsFormList = false;
        if (gfuncs::IsFormValid(paramFilter)) {
            auto* akFormList = paramFilter->As<RE::BGSListForm>();
            if (gfuncs::IsFormValid(akFormList)) {
                bool paramFilterIsFormList = true;
                if (UnregisterFormListForGlobalEvent(akFormList, index, paramFilterIndex, handle)) {
                    eventDataPtrs[index]->CheckAndRemoveSink();
                }
                else {
                    logger::warn("{}: passed in paramFilter formlist[{}] for event [{}] has no forms to unregister", __func__, gfuncs::GetFormName(akFormList), asEvent);
                }
            }
        }

        if (!paramFilterIsFormList) {
            eventDataPtrs[index]->RemoveHandle(handle, paramFilter, paramFilterIndex);
        }
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

//unregister all
void UnregisterFormForGlobalEvent_All(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver) {
    logger::trace("{} {}", __func__, asEvent);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    logger::trace("{} removing handle for: {}", __func__, gfuncs::GetFormName(eventReceiver));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for form [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->RemoveAllHandles(handle);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

void UnregisterAliasForGlobalEvent_All(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::BGSBaseAlias* eventReceiver) {
    logger::trace("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    logger::trace("{} removing all handles for: {}", __func__, eventReceiver->aliasName);

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for alias [{}] ID [{:x}]", __func__, eventReceiver->aliasName, eventReceiver->GetVMTypeID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->RemoveAllHandles(handle);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

void UnregisterActiveMagicEffectForGlobalEvent_All(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::ActiveEffect* eventReceiver) {
    logger::trace("{} {}", __func__, asEvent);

    if (!eventReceiver) {
        logger::warn("{}: eventReceiver not found", __func__);
        return;
    }

    logger::trace("{} removing all handles for: {} instance", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("{}: couldn't get handle for effect [{}] ID [{:x}]", __func__, gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->VMTYPEID);
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->RemoveAllHandles(handle);
    }
    else {
        logger::error("{}: event [{}] not recognized", __func__, asEvent);
    }
}

// plugin load / maintenance==================================================================================================================================================

void CreateEventSinks() {
    if (!player) { player = RE::PlayerCharacter::GetSingleton(); }
    if (!eventSourceholder) { eventSourceholder = RE::ScriptEventSourceHolder::GetSingleton(); }
    if (!xMarker) { xMarker = RE::TESForm::LookupByID(59); }

    if (xMarker) {
        logger::trace("{}: xmarker [{}] found", __func__, gfuncs::GetFormName(xMarker));
    }
    else {
        logger::warn("{}: xmarker form not found", __func__);
    }

    if (eventDataPtrs.size() == 0) {
        eventDataPtrs.resize((EventEnum_Last + 1), nullptr);
        eventDataPtrs[EventEnum_OnLoadGame] = new EventData("OnLoadGameGlobal", EventEnum_OnLoadGame, 0, 'EDl0');
        eventDataPtrs[EventEnum_OnCombatStateChanged] = new EventData("OnCombatStateChangedGlobal", EventEnum_OnCombatStateChanged, 2, 'EDc0');
        eventDataPtrs[EventEnum_FurnitureEnter] = new EventData("OnFurnitureEnterGlobal", EventEnum_FurnitureEnter, 2, 'EDf0');
        eventDataPtrs[EventEnum_FurnitureExit] = new EventData("OnFurnitureExitGlobal", EventEnum_FurnitureExit, 2, 'EDf1');
        eventDataPtrs[EventEnum_OnActivate] = new EventData("OnActivateGlobal", EventEnum_OnActivate, 2, 'EDa0');
        eventDataPtrs[EventEnum_HitEvent] = new EventData("OnhitGlobal", EventEnum_HitEvent, 5, 'EDh0');
        eventDataPtrs[EventEnum_DeathEvent] = new EventData("OnDeathGlobal", EventEnum_HitEvent, 2, 'EDd0');
        eventDataPtrs[EventEnum_DyingEvent] = new EventData("OnDyingGlobal", EventEnum_HitEvent, 2, 'EDd1');
        eventDataPtrs[EventEnum_OnObjectEquipped] = new EventData("OnObjectEquippedGlobal", EventEnum_OnObjectEquipped, 3, 'EDe0');
        eventDataPtrs[EventEnum_OnObjectUnequipped] = new EventData("OnObjectUnequippedGlobal", EventEnum_OnObjectUnequipped, 3, 'EDe1');
        eventDataPtrs[EventEnum_OnWaitStart] = new EventData("OnWaitStartGlobal", EventEnum_OnWaitStart, 0, 'EDw0');
        eventDataPtrs[EventEnum_OnWaitStop] = new EventData("OnWaitStopGlobal", EventEnum_OnWaitStop, 0, 'EDw1');
        eventDataPtrs[EventEnum_OnMagicEffectApply] = new EventData("OnMagicEffectAppliedGlobal", EventEnum_OnMagicEffectApply, 3, 'EDm2');
        eventDataPtrs[EventEnum_OnSpellCast] = new EventData("OnSpellCastGlobal", EventEnum_OnSpellCast, 2, 'EDs0');
        eventDataPtrs[EventEnum_LockChanged] = new EventData("OnLockChangedGlobal", EventEnum_LockChanged, 1, 'EDlc');
        eventDataPtrs[EventEnum_OnOpen] = new EventData("OnOpenGlobal", EventEnum_OnOpen, 2, 'EDo0');
        eventDataPtrs[EventEnum_OnClose] = new EventData("OnCloseGlobal", EventEnum_OnClose, 2, 'EDo1');
        eventDataPtrs[EventEnum_OnWeaponSwing] = new EventData("OnWeaponSwingGlobal", EventEnum_OnWeaponSwing, 2, 'EAA0');              //actorActionEventSink
        eventDataPtrs[EventEnum_OnActorSpellCast] = new EventData("OnActorSpellCastGlobal", EventEnum_OnActorSpellCast, 2, 'EAA1');     //actorActionEventSink
        eventDataPtrs[EventEnum_OnActorSpellFire] = new EventData("OnActorSpellFireGlobal", EventEnum_OnActorSpellFire, 2, 'EAA2');     //actorActionEventSink
        eventDataPtrs[EventEnum_VoiceCast] = new EventData("OnVoiceCastGlobal", EventEnum_VoiceCast, 2, 'EAA3');                        //actorActionEventSink
        eventDataPtrs[EventEnum_VoiceFire] = new EventData("OnVoiceFireGlobal", EventEnum_VoiceFire, 2, 'EAA4');                        //actorActionEventSink
        eventDataPtrs[EventEnum_BowDraw] = new EventData("OnBowDrawGlobal", EventEnum_BowDraw, 2, 'EAA5');                              //actorActionEventSink
        eventDataPtrs[EventEnum_BowRelease] = new EventData("OnBowReleaseGlobal", EventEnum_BowRelease, 2, 'EAA6');                     //actorActionEventSink
        eventDataPtrs[EventEnum_BeginDraw] = new EventData("OnBeginDrawGlobal", EventEnum_BeginDraw, 2, 'EAA7');                        //actorActionEventSink
        eventDataPtrs[EventEnum_EndDraw] = new EventData("OnEndDrawGlobal", EventEnum_EndDraw, 2, 'EAA8');                              //actorActionEventSink
        eventDataPtrs[EventEnum_BeginSheathe] = new EventData("OnBeginSheatheGlobal", EventEnum_BeginSheathe, 2, 'EAA9');               //actorActionEventSink
        eventDataPtrs[EventEnum_EndSheathe] = new EventData("OnEndSheatheGlobal", EventEnum_EndSheathe, 2, 'EA10');                     //actorActionEventSink
        eventDataPtrs[EventEnum_OnContainerChanged] = new EventData("OnContainerChangedGlobal", EventEnum_OnContainerChanged, 4, 'ECc0');
        eventDataPtrs[EventEnum_OnProjectileImpact] = new EventData("OnProjectileImpactGlobal", EventEnum_OnProjectileImpact, 5, 'PIi0');
        eventDataPtrs[EventEnum_OnItemCrafted] = new EventData("OnItemCraftedGlobal", EventEnum_OnItemCrafted, 2, 'ICa0');
        eventDataPtrs[EventEnum_OnItemsPickpocketed] = new EventData("OnItemsPickpocketedGlobal", EventEnum_OnItemsPickpocketed, 2, 'IPa0');
        eventDataPtrs[EventEnum_OnLocationCleared] = new EventData("OnLocationClearedGlobal", EventEnum_OnLocationCleared, 1, 'LCa0');
        eventDataPtrs[EventEnum_OnEnterBleedout] = new EventData("OnEnterBleedoutGlobal", EventEnum_OnEnterBleedout, 1, 'EBa0');
        eventDataPtrs[EventEnum_OnSwitchRaceComplete] = new EventData("OnRaceSwitchCompleteGlobal", EventEnum_OnSwitchRaceComplete, 3, 'SWr0');
        eventDataPtrs[EventEnum_OnActorFootStep] = new EventData("OnActorFootStepGlobal", EventEnum_OnActorFootStep, 1, 'AFs0');
        eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged] = new EventData("OnQuestObjectiveStateChangedGlobal", EventEnum_OnQuestObjectiveStateChanged, 1, 'QOs0');
        eventDataPtrs[EventEnum_OnPositionPlayerStart] = new EventData("OnPositionPlayerStartGlobal", EventEnum_OnPositionPlayerStart, 4, 'PPa0');
        eventDataPtrs[EventEnum_OnPositionPlayerFinish] = new EventData("OnPositionPlayerFinishGlobal", EventEnum_OnPositionPlayerFinish, 4, 'PPa1');
        eventDataPtrs[EventEnum_OnPlayerChangeCell] = new EventData("OnPlayerChangeCellGlobal", EventEnum_OnPlayerChangeCell, 2, 'PCC0');
    }

    if (!combatEventSink) { combatEventSink = new CombatEventSink(); }
    if (!furnitureEventSink) { furnitureEventSink = new FurnitureEventSink(); }
    if (!activateEventSink) { activateEventSink = new ActivateEventSink(); }
    if (!hitEventSink) { hitEventSink = new HitEventSink(); }
    if (!deathEventSink) { deathEventSink = new DeathEventSink(); }
    if (!equipEventSink) { equipEventSink = new EquipEventSink(); }
    if (!waitStartEventSink) { waitStartEventSink = new WaitStartEventSink(); }
    if (!waitStopEventSink) { waitStopEventSink = new WaitStopEventSink(); }
    if (!magicEffectApplyEventSink) { magicEffectApplyEventSink = new MagicEffectApplyEventSink(); }
    if (!spellCastEventSink) { spellCastEventSink = new SpellCastEventSink(); }
    if (!containerChangedEventSink) { containerChangedEventSink = new ContainerChangedEventSink(); }
    if (!lockChangedEventSink) { lockChangedEventSink = new LockChangedEventSink(); }
    if (!openCloseEventSink) { openCloseEventSink = new OpenCloseEventSink(); }
    if (!actorActionEventSink) { actorActionEventSink = new ActorActionEventSink(); }
    if (!menuOpenCloseEventSink) { menuOpenCloseEventSink = new MenuOpenCloseEventSink(); }
    if (!itemCraftedEventSink) { itemCraftedEventSink = new ItemCraftedEventSink(); }
    if (!itemsPickpocketedEventSink) { itemsPickpocketedEventSink = new ItemsPickpocketedEventSink(); }
    if (!locationClearedEventSink) { locationClearedEventSink = new LocationClearedEventSink(); }
    if (!enterBleedoutEventSink) { enterBleedoutEventSink = new EnterBleedoutEventSink(); }
    if (!switchRaceCompleteEventSink) { switchRaceCompleteEventSink = new SwitchRaceCompleteEventSink(); }
    if (!footstepEventSink) { footstepEventSink = new FootstepEventSink(); }
    if (!questObjectiveEventSink) { questObjectiveEventSink = new QuestObjectiveEventSink(); }
    if (!objectInitEventSink) { objectInitEventSink = new ObjectInitEventSink(); }
    if (!objectLoadedEventSink) { objectLoadedEventSink = new ObjectLoadedEventSink(); }
    if (!inputEventSink) { inputEventSink = new InputEventSink(); }
    if (!positionPlayerEventSink) { positionPlayerEventSink = new PositionPlayerEventSink(); }
    if (!actorCellEventSink) { actorCellEventSink = new ActorCellEventSink(); }

    if (!activateEventSink->sinkAdded) {
        //always active to track lastPlayerActivatedRef
        activateEventSink->sinkAdded = true;
        eventSourceholder->AddEventSink(activateEventSink);
    }

    //eventSourceholder->AddEventSink(objectLoadedEventSink);

    if (!menuOpenCloseEventSink->sinkAdded) {
        menuOpenCloseEventSink->sinkAdded = true;
        //always active to track opened menus / game pausing
        ui->AddEventSink<RE::MenuOpenCloseEvent>(menuOpenCloseEventSink);
    }

    if (iMaxArrowsSavedPerReference > 0) {
        if (skseLoadInterface) {
            if (ProjectileImpactHook::Install(skseLoadInterface->RuntimeVersion())) {
                logger::info("{}: ProjectileImpactHook installed successfully", __func__);
            }
            else {
                logger::warn("{}: ProjectileImpactHook not installed", __func__);
            }
        }
        else {
            logger::critical("{}: skseLoadInterface not found, aborting ProjectileImpactHook install", __func__);
        }
    }
    else {
        logger::info("{}: iMaxArrowsSavedPerReference is [{}], aborting ProjectileImpactHook install", __func__, iMaxArrowsSavedPerReference);
    }

    UIEvents::Install();

    logger::trace("Event Sinks Created");
}

bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    // vm->RegisterFunction("MyNativeFunction", "DbSkseFunctions", MyNativeFunction);
    logger::trace("Binding Papyrus Functions");

    gvm = vm;
    //gfuncs::svm = RE::SkyrimVM::GetSingleton();
    ui = RE::UI::GetSingleton();
    calendar = RE::Calendar::GetSingleton();

    //functions 
    vm->RegisterFunction("SaveProjectileForAmmo", "DbSkseCppCallbackEvents", SaveProjectileForAmmo);
    vm->RegisterFunction("SetDbSkseCppCallbackEventsAttached", "DbSkseCppCallbackEvents", SetDbSkseCppCallbackEventsAttached);

    vm->RegisterFunction("GetVersion", "DbSkseFunctions", GetThisVersion);
    vm->RegisterFunction("GetClipBoardText", "DbSkseFunctions", GetClipBoardText);
    vm->RegisterFunction("SetClipBoardText", "DbSkseFunctions", SetClipBoardText);
    vm->RegisterFunction("IsWhiteSpace", "DbSkseFunctions", IsWhiteSpace);
    vm->RegisterFunction("CountWhiteSpaces", "DbSkseFunctions", CountWhiteSpaces);
    vm->RegisterFunction("ModHasFormType", "DbSkseFunctions", ModHasFormType);
    vm->RegisterFunction("GetFormDescription", "DbSkseFunctions", GetFormDescription);
    vm->RegisterFunction("GetFormDescriptions", "DbSkseFunctions", GetFormDescriptions);
    vm->RegisterFunction("GetFormDescriptionsFromList", "DbSkseFunctions", GetFormDescriptionsFromList);
    vm->RegisterFunction("GetFormNames", "DbSkseFunctions", GetFormNames);
    vm->RegisterFunction("GetFormNamesFromList", "DbSkseFunctions", GetFormNamesFromList);
    vm->RegisterFunction("GetFormEditorId", "DbSkseFunctions", GetFormEditorId);
    vm->RegisterFunction("GetFormEditorIds", "DbSkseFunctions", GetFormEditorIds);
    vm->RegisterFunction("GetFormEditorIdsFromList", "DbSkseFunctions", GetFormEditorIdsFromList);
    vm->RegisterFunction("GetLoadedModNames", "DbSkseFunctions", GetLoadedModNames);
    vm->RegisterFunction("GetLoadedLightModNames", "DbSkseFunctions", GetLoadedLightModNames);
    vm->RegisterFunction("GetAllLoadedModNames", "DbSkseFunctions", GetAllLoadedModNames);
    vm->RegisterFunction("GetLoadedModDescriptions", "DbSkseFunctions", GetLoadedModDescriptions);
    vm->RegisterFunction("GetLoadedLightModDescriptions", "DbSkseFunctions", GetLoadedLightModDescriptions);
    vm->RegisterFunction("GetAllLoadedModDescriptions", "DbSkseFunctions", GetAllLoadedModDescriptions);
    vm->RegisterFunction("SortFormArray", "DbSkseFunctions", SortFormArray);
    vm->RegisterFunction("FormListToArray", "DbSkseFunctions", FormListToArray);
    vm->RegisterFunction("AddFormsToList", "DbSkseFunctions", AddFormsToList);
    vm->RegisterFunction("GetAllActiveQuests", "DbSkseFunctions", GetAllActiveQuests);
    vm->RegisterFunction("GetAllConstructibleObjects", "DbSkseFunctions", GetAllConstructibleObjects);
    vm->RegisterFunction("GetAllArmorsForSlotMask", "DbSkseFunctions", GetAllArmorsForSlotMask);
    vm->RegisterFunction("GetCellWorldSpace", "DbSkseFunctions", GetCellWorldSpace);
    vm->RegisterFunction("GetCellLocation", "DbSkseFunctions", GetCellLocation);
    vm->RegisterFunction("GetAllInteriorCells", "DbSkseFunctions", GetAllInteriorCells);
    vm->RegisterFunction("GetAllExteriorCells", "DbSkseFunctions", GetAllExteriorCells);
    vm->RegisterFunction("GetAttachedCells", "DbSkseFunctions", GetAttachedCells);
    vm->RegisterFunction("GetAllFormsWithName", "DbSkseFunctions", GetAllFormsWithName);
    vm->RegisterFunction("GetAllFormsWithScriptAttached", "DbSkseFunctions", GetAllFormsWithScriptAttached);
    vm->RegisterFunction("GetAllAliasesWithScriptAttached", "DbSkseFunctions", GetAllAliasesWithScriptAttached);
    vm->RegisterFunction("GetAllRefAliasesWithScriptAttached", "DbSkseFunctions", GetAllRefAliasesWithScriptAttached);
    vm->RegisterFunction("GetAllRefaliases", "DbSkseFunctions", GetAllRefaliases);
    vm->RegisterFunction("GetAllQuestObjectRefs", "DbSkseFunctions", GetAllQuestObjectRefs);
    vm->RegisterFunction("GetQuestObjectRefsInContainer", "DbSkseFunctions", GetQuestObjectRefsInContainer);
    vm->RegisterFunction("GetFavorites", "DbSkseFunctions", GetFavorites);
    vm->RegisterFunction("GetProjectileBaseDecal", "DbSkseFunctions", GetProjectileBaseDecal);
    vm->RegisterFunction("SetProjectileBaseDecal", "DbSkseFunctions", SetProjectileBaseDecal);
    vm->RegisterFunction("GetProjectileBaseExplosion", "DbSkseFunctions", GetProjectileBaseExplosion);
    vm->RegisterFunction("SetProjectileBaseExplosion", "DbSkseFunctions", SetProjectileBaseExplosion);
    vm->RegisterFunction("GetProjectileBaseCollisionRadius", "DbSkseFunctions", GetProjectileBaseCollisionRadius);
    vm->RegisterFunction("SetProjectileBaseCollisionRadius", "DbSkseFunctions", SetProjectileBaseCollisionRadius);
    vm->RegisterFunction("GetProjectileBaseCollisionConeSpread", "DbSkseFunctions", GetProjectileBaseCollisionConeSpread);
    vm->RegisterFunction("SetProjectileBaseCollisionConeSpread", "DbSkseFunctions", SetProjectileBaseCollisionConeSpread);
    vm->RegisterFunction("GetProjectileRefType", "DbSkseFunctions", GetProjectileRefType);
    vm->RegisterFunction("GetAttachedProjectiles", "DbSkseFunctions", GetAttachedProjectiles);
    vm->RegisterFunction("GetAttachedProjectileRefs", "DbSkseFunctions", GetAttachedProjectileRefs);
    vm->RegisterFunction("GetAllHitProjectileRefsOfType", "DbSkseFunctions", GetAllHitProjectileRefsOfType);
    vm->RegisterFunction("GetAllShotProjectileRefsOfType", "DbSkseFunctions", GetAllShotProjectileRefsOfType);
    vm->RegisterFunction("GetRecentProjectileHitRefs", "DbSkseFunctions", GetRecentProjectileHitRefs);
    vm->RegisterFunction("GetLastProjectileHitRef", "DbSkseFunctions", GetLastProjectileHitRef);
    vm->RegisterFunction("GetRecentProjectileShotRefs", "DbSkseFunctions", GetRecentProjectileShotRefs);
    vm->RegisterFunction("GetLastProjectileShotRef", "DbSkseFunctions", GetLastProjectileShotRef);
    vm->RegisterFunction("GetProjectileHitRefs", "DbSkseFunctions", GetProjectileHitRefs);
    vm->RegisterFunction("GetProjectileShooter", "DbSkseFunctions", GetProjectileShooter);
    vm->RegisterFunction("GetProjectileExplosion", "DbSkseFunctions", GetProjectileExplosion);
    vm->RegisterFunction("GetProjectileAmmoSource", "DbSkseFunctions", GetProjectileAmmoSource);
    vm->RegisterFunction("GetProjectilePoison", "DbSkseFunctions", GetProjectilePoison);
    vm->RegisterFunction("GetProjectileEnchantment", "DbSkseFunctions", GetProjectileEnchantment);
    vm->RegisterFunction("GetProjectileMagicSource", "DbSkseFunctions", GetProjectileMagicSource);
    vm->RegisterFunction("GetProjectileWeaponSource", "DbSkseFunctions", GetProjectileWeaponSource);
    vm->RegisterFunction("GetProjectileWeaponDamage", "DbSkseFunctions", GetProjectileWeaponDamage);
    vm->RegisterFunction("GetProjectilePower", "DbSkseFunctions", GetProjectilePower);
    vm->RegisterFunction("GetProjectileDistanceTraveled", "DbSkseFunctions", GetProjectileDistanceTraveled);
    vm->RegisterFunction("GetProjectileImpactResult", "DbSkseFunctions", GetProjectileImpactResult);
    vm->RegisterFunction("GetProjectileNodeHitNames", "DbSkseFunctions", GetProjectileNodeHitNames);
    vm->RegisterFunction("GetProjectileCollidedLayers", "DbSkseFunctions", GetProjectileCollidedLayers);
    vm->RegisterFunction("GetProjectileCollidedLayerNames", "DbSkseFunctions", GetProjectileCollidedLayerNames);
    vm->RegisterFunction("GetCollisionLayerName", "DbSkseFunctions", GetCollisionLayerName);
    vm->RegisterFunction("GetLastPlayerActivatedRef", "DbSkseFunctions", GetLastPlayerActivatedRef);
    vm->RegisterFunction("GetLastPlayerMenuActivatedRef", "DbSkseFunctions", GetLastPlayerMenuActivatedRef);
    vm->RegisterFunction("GetAshPileLinkedRef", "DbSkseFunctions", GetAshPileLinkedRef);
    vm->RegisterFunction("GetClosestObjectFromRef", "DbSkseFunctions", GetClosestObjectFromRef);
    vm->RegisterFunction("GetClosestObjectIndexFromRef", "DbSkseFunctions", GetClosestObjectIndexFromRef);
    vm->RegisterFunction("GetGameHoursPassed", "DbSkseFunctions", GetGameHoursPassed);
    vm->RegisterFunction("GameHoursToRealTimeSeconds", "DbSkseFunctions", GameHoursToRealTimeSeconds);
    vm->RegisterFunction("IsGamePaused", "DbSkseFunctions", IsGamePaused);
    vm->RegisterFunction("IsInMenu", "DbSkseFunctions", IsInMenu);
    vm->RegisterFunction("GetLastMenuOpened", "DbSkseFunctions", GetLastMenuOpened);
    vm->RegisterFunction("SetMapMarkerName", "DbSkseFunctions", SetMapMarkerName);
    vm->RegisterFunction("GetMapMarkerName", "DbSkseFunctions", GetMapMarkerName);
    vm->RegisterFunction("SetMapMarkerIconType", "DbSkseFunctions", SetMapMarkerIconType);
    vm->RegisterFunction("GetMapMarkerIconType", "DbSkseFunctions", GetMapMarkerIconType);
    vm->RegisterFunction("IsMapMarker", "DbSkseFunctions", IsMapMarker);
    vm->RegisterFunction("SetMapMarkerVisible", "DbSkseFunctions", SetMapMarkerVisible);
    vm->RegisterFunction("SetCanFastTravelToMarker", "DbSkseFunctions", SetCanFastTravelToMarker);
    vm->RegisterFunction("GetAllMapMarkerRefs", "DbSkseFunctions", GetAllMapMarkerRefs);
    vm->RegisterFunction("GetCurrentMapMarkerRefs", "DbSkseFunctions", GetCurrentMapMarkerRefs);
    vm->RegisterFunction("GetCellOrWorldSpaceOriginForRef", "DbSkseFunctions", GetCellOrWorldSpaceOriginForRef);
    vm->RegisterFunction("SetCellOrWorldSpaceOriginForRef", "DbSkseFunctions", SetCellOrWorldSpaceOriginForRef);
    vm->RegisterFunction("LoadMostRecentSaveGame", "DbSkseFunctions", LoadMostRecentSaveGame);
    vm->RegisterFunction("ExecuteConsoleCommand", "DbSkseFunctions", ExecuteConsoleCommand);
    vm->RegisterFunction("HasCollision", "DbSkseFunctions", HasCollision);
    vm->RegisterFunction("GetCurrentMusicType", "DbSkseFunctions", GetCurrentMusicType);
    vm->RegisterFunction("GetFurnitureWorkbenchType", "DbSkseFunctions", GetFurnitureWorkbenchType);
    vm->RegisterFunction("GetFurnitureWorkbenchSkillInt", "DbSkseFunctions", GetFurnitureWorkbenchSkillInt);
    vm->RegisterFunction("GetFurnitureWorkbenchSkillString", "DbSkseFunctions", GetFurnitureWorkbenchSkillString);
    vm->RegisterFunction("GetNumberOfTracksInMusicType", "DbSkseFunctions", GetNumberOfTracksInMusicType);
    vm->RegisterFunction("GetMusicTypeTrackIndex", "DbSkseFunctions", GetMusicTypeTrackIndex);
    vm->RegisterFunction("SetMusicTypeTrackIndex", "DbSkseFunctions", SetMusicTypeTrackIndex);
    vm->RegisterFunction("GetMusicTypePriority", "DbSkseFunctions", GetMusicTypePriority);
    vm->RegisterFunction("SetMusicTypePriority", "DbSkseFunctions", SetMusicTypePriority);
    vm->RegisterFunction("GetMusicTypeStatus", "DbSkseFunctions", GetMusicTypeStatus);
    vm->RegisterFunction("GetKnownEnchantments", "DbSkseFunctions", GetKnownEnchantments);
    vm->RegisterFunction("AddKnownEnchantmentsToFormList", "DbSkseFunctions", AddKnownEnchantmentsToFormList);
    vm->RegisterFunction("GetWordOfPowerTranslation", "DbSkseFunctions", GetWordOfPowerTranslation);
    vm->RegisterFunction("UnlockShout", "DbSkseFunctions", UnlockShout);
    vm->RegisterFunction("AddAndUnlockAllShouts", "DbSkseFunctions", AddAndUnlockAllShouts);
    vm->RegisterFunction("GetSpellTomeForSpell", "DbSkseFunctions", GetSpellTomeForSpell);
    vm->RegisterFunction("SetBookSpell", "DbSkseFunctions", SetBookSpell);
    vm->RegisterFunction("GetSpellTomesForSpell", "DbSkseFunctions", GetSpellTomesForSpell);
    vm->RegisterFunction("AddSpellTomesForSpellToList", "DbSkseFunctions", AddSpellTomesForSpellToList);
    vm->RegisterFunction("GetSkillBooksForSkill", "DbSkseFunctions", GetSkillBooksForSkill);
    vm->RegisterFunction("AddSkillBookForSkillToList", "DbSkseFunctions", AddSkillBookForSkillToList);
    vm->RegisterFunction("SetBookSkill", "DbSkseFunctions", SetBookSkill);
    vm->RegisterFunction("GetBookSkill", "DbSkseFunctions", GetBookSkill);
    vm->RegisterFunction("SetBookRead", "DbSkseFunctions", SetBookRead);
    vm->RegisterFunction("SetAllBooksRead", "DbSkseFunctions", SetAllBooksRead);
    vm->RegisterFunction("GetActiveMagicEffectConditionStatus", "DbSkseFunctions", GetActiveMagicEffectConditionStatus);
    vm->RegisterFunction("GetActiveEffectSource", "DbSkseFunctions", GetActiveEffectSource);
    vm->RegisterFunction("GetActiveEffectCastingSource", "DbSkseFunctions", GetActiveEffectCastingSource);
    vm->RegisterFunction("GetMagicEffectsForForm", "DbSkseFunctions", GetMagicEffectsForForm);
    vm->RegisterFunction("IsFormMagicItem", "DbSkseFunctions", IsFormMagicItem);
    vm->RegisterFunction("IsMagicEffectActiveOnRef", "DbSkseFunctions", IsMagicEffectActiveOnRef);
    vm->RegisterFunction("DispelMagicEffectOnRef", "DbSkseFunctions", DispelMagicEffectOnRef);
    vm->RegisterFunction("SetSoulGemSize", "DbSkseFunctions", SetSoulGemSize);
    vm->RegisterFunction("CanSoulGemHoldNPCSoul", "DbSkseFunctions", CanSoulGemHoldNPCSoul);
    vm->RegisterFunction("SetSoulGemCanHoldNPCSoul", "DbSkseFunctions", SetSoulGemCanHoldNPCSoul);
    vm->RegisterFunction("WouldActorBeStealing", "DbSkseFunctions", WouldActorBeStealing);
    vm->RegisterFunction("IsActorAttacking", "DbSkseFunctions", IsActorAttacking);
    vm->RegisterFunction("IsActorPowerAttacking", "DbSkseFunctions", IsActorPowerAttacking);
    vm->RegisterFunction("IsActorSpeaking", "DbSkseFunctions", IsActorSpeaking);
    vm->RegisterFunction("IsActorBlocking", "DbSkseFunctions", IsActorBlocking);
    vm->RegisterFunction("IsActorCasting", "DbSkseFunctions", IsActorCasting);
    vm->RegisterFunction("IsActorDualCasting", "DbSkseFunctions", IsActorDualCasting);
    vm->RegisterFunction("IsActorStaggered", "DbSkseFunctions", IsActorStaggered);
    vm->RegisterFunction("IsActorRecoiling", "DbSkseFunctions", IsActorRecoiling);
    vm->RegisterFunction("IsActorIgnoringCombat", "DbSkseFunctions", IsActorIgnoringCombat);
    vm->RegisterFunction("IsActorUndead", "DbSkseFunctions", IsActorUndead);
    vm->RegisterFunction("IsActorOnFlyingMount", "DbSkseFunctions", IsActorOnFlyingMount);
    vm->RegisterFunction("IsActorFleeing", "DbSkseFunctions", IsActorFleeing);
    vm->RegisterFunction("IsActorAMount", "DbSkseFunctions", IsActorAMount);
    vm->RegisterFunction("IsActorInMidAir", "DbSkseFunctions", IsActorInMidAir);
    vm->RegisterFunction("IsActorInRagdollState", "DbSkseFunctions", IsActorInRagdollState);
    vm->RegisterFunction("GetDetectionLevel", "DbSkseFunctions", GetDetectionLevel);
    vm->RegisterFunction("GetKeywordString", "DbSkseFunctions", GetKeywordString);
    vm->RegisterFunction("SetKeywordString", "DbSkseFunctions", SetKeywordString);
    vm->RegisterFunction("CreateKeyword", "DbSkseFunctions", CreateKeyword);
    vm->RegisterFunction("CreateFormList", "DbSkseFunctions", CreateFormList);
    vm->RegisterFunction("CreateColorForm", "DbSkseFunctions", CreateColorForm);
    vm->RegisterFunction("CreateConstructibleObject", "DbSkseFunctions", CreateConstructibleObject);
    vm->RegisterFunction("CreateTextureSet", "DbSkseFunctions", CreateTextureSet);
    vm->RegisterFunction("CreateSoundMarker", "DbSkseFunctions", CreateSoundMarker);
    vm->RegisterFunction("PlaySound", "DbSkseFunctions", PlaySound);
    vm->RegisterFunction("PlaySoundDescriptor", "DbSkseFunctions", PlaySoundDescriptor);
    vm->RegisterFunction("SetSoundInstanceSource", "DbSkseFunctions", SetSoundInstanceSource);
    vm->RegisterFunction("GetParentSoundCategory", "DbSkseFunctions", GetParentSoundCategory);
    vm->RegisterFunction("GetSoundCategoryForSoundDescriptor", "DbSkseFunctions", GetSoundCategoryForSoundDescriptor);
    vm->RegisterFunction("SetSoundCategoryForSoundDescriptor", "DbSkseFunctions", SetSoundCategoryForSoundDescriptor);
    vm->RegisterFunction("GetSoundCategoryVolume", "DbSkseFunctions", GetSoundCategoryVolume);
    vm->RegisterFunction("GetSoundCategoryFrequency", "DbSkseFunctions", GetSoundCategoryFrequency);

    //global events ====================================================================================================

    //form
    vm->RegisterFunction("IsFormRegisteredForGlobalEvent", "DbSkseEvents", IsFormRegisteredForGlobalEvent);
    vm->RegisterFunction("RegisterFormForGlobalEvent", "DbSkseEvents", RegisterFormForGlobalEvent);
    vm->RegisterFunction("UnregisterFormForGlobalEvent", "DbSkseEvents", UnregisterFormForGlobalEvent);
    vm->RegisterFunction("UnregisterFormForGlobalEvent_All", "DbSkseEvents", UnregisterFormForGlobalEvent_All);

    //alias
    vm->RegisterFunction("IsAliasRegisteredForGlobalEvent", "DbSkseEvents", IsAliasRegisteredForGlobalEvent);
    vm->RegisterFunction("RegisterAliasForGlobalEvent", "DbSkseEvents", RegisterAliasForGlobalEvent);
    vm->RegisterFunction("UnregisterAliasForGlobalEvent", "DbSkseEvents", UnregisterAliasForGlobalEvent);
    vm->RegisterFunction("UnregisterAliasForGlobalEvent_All", "DbSkseEvents", UnregisterAliasForGlobalEvent_All);

    //activeMagicEffect
    vm->RegisterFunction("IsActiveMagicEffectRegisteredForGlobalEvent", "DbSkseEvents", IsActiveMagicEffectRegisteredForGlobalEvent);
    vm->RegisterFunction("RegisterActiveMagicEffectForGlobalEvent", "DbSkseEvents", RegisterActiveMagicEffectForGlobalEvent);
    vm->RegisterFunction("UnregisterActiveMagicEffectForGlobalEvent", "DbSkseEvents", UnregisterActiveMagicEffectForGlobalEvent);
    vm->RegisterFunction("UnregisterActiveMagicEffectForGlobalEvent_All", "DbSkseEvents", UnregisterActiveMagicEffectForGlobalEvent_All);

    //UiItemMenuEvents
    //form
    vm->RegisterFunction("IsFormRegisteredForUiItemMenuEvent", "DbSkseEvents", UIEvents::IsFormRegisteredForUiItemMenuEvent);
    vm->RegisterFunction("RegisterFormForUiItemMenuEvent", "DbSkseEvents", UIEvents::RegisterFormForUiItemMenuEvent);
    vm->RegisterFunction("UnregisterFormForUiItemMenuEvent", "DbSkseEvents", UIEvents::UnregisterFormForUiItemMenuEvent);
    vm->RegisterFunction("UnregisterFormForUiItemMenuEvent_All", "DbSkseEvents", UIEvents::UnregisterFormForUiItemMenuEvent_All);

    //Alias
    vm->RegisterFunction("IsAliasRegisteredForUiItemMenuEvent", "DbSkseEvents", UIEvents::IsAliasRegisteredForUiItemMenuEvent);
    vm->RegisterFunction("RegisterAliasForUiItemMenuEvent", "DbSkseEvents", UIEvents::RegisterAliasForUiItemMenuEvent);
    vm->RegisterFunction("UnregisterAliasForUiItemMenuEvent", "DbSkseEvents", UIEvents::UnregisterAliasForUiItemMenuEvent);
    vm->RegisterFunction("UnregisterAliasForUiItemMenuEvent_All", "DbSkseEvents", UIEvents::UnregisterAliasForUiItemMenuEvent_All);

    //ActiveMagicEffect
    vm->RegisterFunction("IsActiveMagicEffectRegisteredForUiItemMenuEvent", "DbSkseEvents", UIEvents::IsActiveMagicEffectRegisteredForUiItemMenuEvent);
    vm->RegisterFunction("RegisterActiveMagicEffectForUiItemMenuEvent", "DbSkseEvents", UIEvents::RegisterActiveMagicEffectForUiItemMenuEvent);
    vm->RegisterFunction("UnregisterActiveMagicEffectForUiItemMenuEvent", "DbSkseEvents", UIEvents::UnregisterActiveMagicEffectForUiItemMenuEvent);
    vm->RegisterFunction("UnregisterActiveMagicEffectForUiItemMenuEvent_All", "DbSkseEvents", UIEvents::UnregisterActiveMagicEffectForUiItemMenuEvent_All);

    //timers
    //form
    vm->RegisterFunction("StartTimer", "DbFormTimer", StartTimerOnForm);
    vm->RegisterFunction("CancelTimer", "DbFormTimer", CancelTimerOnForm);
    vm->RegisterFunction("GetTimeElapsedOnTimer", "DbFormTimer", GetTimeElapsedOnTimerForm);
    vm->RegisterFunction("GetTimeLeftOnTimer", "DbFormTimer", GetTimeLeftOnTimerForm);

    vm->RegisterFunction("StartNoMenuModeTimer", "DbFormTimer", StartNoMenuModeTimerOnForm);
    vm->RegisterFunction("CancelNoMenuModeTimer", "DbFormTimer", CancelNoMenuModeTimerOnForm);
    vm->RegisterFunction("GetTimeElapsedOnNoMenuModeTimer", "DbFormTimer", GetTimeElapsedOnNoMenuModeTimerForm);
    vm->RegisterFunction("GetTimeLeftOnNoMenuModeTimer", "DbFormTimer", GetTimeLeftOnNoMenuModeTimerForm);

    vm->RegisterFunction("StartMenuModeTimer", "DbFormTimer", StartMenuModeTimerOnForm);
    vm->RegisterFunction("CancelMenuModeTimer", "DbFormTimer", CancelMenuModeTimerOnForm);
    vm->RegisterFunction("GetTimeElapsedOnMenuModeTimer", "DbFormTimer", GetTimeElapsedOnMenuModeTimerForm);
    vm->RegisterFunction("GetTimeLeftOnMenuModeTimer", "DbFormTimer", GetTimeLeftOnMenuModeTimerForm);

    vm->RegisterFunction("StartGameTimer", "DbFormTimer", StartGameTimerOnForm);
    vm->RegisterFunction("CancelGameTimer", "DbFormTimer", CancelGameTimerOnForm);
    vm->RegisterFunction("GetTimeElapsedOnGameTimer", "DbFormTimer", GetTimeElapsedOnGameTimerForm);
    vm->RegisterFunction("GetTimeLeftOnGameTimer", "DbFormTimer", GetTimeLeftOnGameTimerForm);

    //Alias
    vm->RegisterFunction("StartTimer", "DbAliasTimer", StartTimerOnAlias);
    vm->RegisterFunction("CancelTimer", "DbAliasTimer", CancelTimerOnAlias);
    vm->RegisterFunction("GetTimeElapsedOnTimer", "DbAliasTimer", GetTimeElapsedOnTimerAlias);
    vm->RegisterFunction("GetTimeLeftOnTimer", "DbAliasTimer", GetTimeLeftOnTimerAlias);

    vm->RegisterFunction("StartNoMenuModeTimer", "DbAliasTimer", StartNoMenuModeTimerOnAlias);
    vm->RegisterFunction("CancelNoMenuModeTimer", "DbAliasTimer", CancelNoMenuModeTimerOnAlias);
    vm->RegisterFunction("GetTimeElapsedOnNoMenuModeTimer", "DbAliasTimer", GetTimeElapsedOnNoMenuModeTimerAlias);
    vm->RegisterFunction("GetTimeLeftOnNoMenuModeTimer", "DbAliasTimer", GetTimeLeftOnNoMenuModeTimerAlias);

    vm->RegisterFunction("StartMenuModeTimer", "DbAliasTimer", StartMenuModeTimerOnAlias);
    vm->RegisterFunction("CancelMenuModeTimer", "DbAliasTimer", CancelMenuModeTimerOnAlias);
    vm->RegisterFunction("GetTimeElapsedOnMenuModeTimer", "DbAliasTimer", GetTimeElapsedOnMenuModeTimerAlias);
    vm->RegisterFunction("GetTimeLeftOnMenuModeTimer", "DbAliasTimer", GetTimeLeftOnMenuModeTimerAlias);

    vm->RegisterFunction("StartGameTimer", "DbAliasTimer", StartGameTimerOnAlias);
    vm->RegisterFunction("CancelGameTimer", "DbAliasTimer", CancelGameTimerOnAlias);
    vm->RegisterFunction("GetTimeElapsedOnGameTimer", "DbAliasTimer", GetTimeElapsedOnGameTimerAlias);
    vm->RegisterFunction("GetTimeLeftOnGameTimer", "DbAliasTimer", GetTimeLeftOnGameTimerAlias);

    //ActiveMagicEffect
    vm->RegisterFunction("StartTimer", "DbActiveMagicEffectTimer", StartTimerOnActiveMagicEffect);
    vm->RegisterFunction("CancelTimer", "DbActiveMagicEffectTimer", CancelTimerOnActiveMagicEffect);
    vm->RegisterFunction("GetTimeElapsedOnTimer", "DbActiveMagicEffectTimer", GetTimeElapsedOnTimerActiveMagicEffect);
    vm->RegisterFunction("GetTimeLeftOnTimer", "DbActiveMagicEffectTimer", GetTimeLeftOnTimerActiveMagicEffect);

    vm->RegisterFunction("StartNoMenuModeTimer", "DbActiveMagicEffectTimer", StartNoMenuModeTimerOnActiveMagicEffect);
    vm->RegisterFunction("CancelNoMenuModeTimer", "DbActiveMagicEffectTimer", CancelNoMenuModeTimerOnActiveMagicEffect);
    vm->RegisterFunction("GetTimeElapsedOnNoMenuModeTimer", "DbActiveMagicEffectTimer", GetTimeElapsedOnNoMenuModeTimerActiveMagicEffect);
    vm->RegisterFunction("GetTimeLeftOnNoMenuModeTimer", "DbActiveMagicEffectTimer", GetTimeLeftOnNoMenuModeTimerActiveMagicEffect);

    vm->RegisterFunction("StartMenuModeTimer", "DbActiveMagicEffectTimer", StartMenuModeTimerOnActiveMagicEffect);
    vm->RegisterFunction("CancelMenuModeTimer", "DbActiveMagicEffectTimer", CancelMenuModeTimerOnActiveMagicEffect);
    vm->RegisterFunction("GetTimeElapsedOnMenuModeTimer", "DbActiveMagicEffectTimer", GetTimeElapsedOnMenuModeTimerActiveMagicEffect);
    vm->RegisterFunction("GetTimeLeftOnMenuModeTimer", "DbActiveMagicEffectTimer", GetTimeLeftOnMenuModeTimerActiveMagicEffect);

    vm->RegisterFunction("StartGameTimer", "DbActiveMagicEffectTimer", StartGameTimerOnActiveMagicEffect);
    vm->RegisterFunction("CancelGameTimer", "DbActiveMagicEffectTimer", CancelGameTimerOnActiveMagicEffect);
    vm->RegisterFunction("GetTimeElapsedOnGameTimer", "DbActiveMagicEffectTimer", GetTimeElapsedOnGameTimerActiveMagicEffect);
    vm->RegisterFunction("GetTimeLeftOnGameTimer", "DbActiveMagicEffectTimer", GetTimeLeftOnGameTimerActiveMagicEffect);

    logger::trace("Papyrus Functions Bound");

    return true;
}

void MessageListener(SKSE::MessagingInterface::Message* message) {
    switch (message->type) {
        // Descriptions are taken from the original skse64 library
        // See:
        // https://github.com/ianpatt/skse64/blob/09f520a2433747f33ae7d7c15b1164ca198932c3/skse64/PluginAPI.h#L193-L212
     //case SKSE::MessagingInterface::kPostLoad: //
        //    logger::info("kPostLoad: sent to registered plugins once all plugins have been loaded");
        //    break;

    //case SKSE::MessagingInterface::kPostPostLoad:
        //    logger::info(
        //        "kPostPostLoad: sent right after kPostLoad to facilitate the correct dispatching/registering of "
        //        "messages/listeners");
        //    break;

    //case SKSE::MessagingInterface::kPreLoadGame:
        //    // message->dataLen: length of file path, data: char* file path of .ess savegame file
        //    logger::info("kPreLoadGame: sent immediately before savegame is read");
        //    break;

    case SKSE::MessagingInterface::kPostLoadGame:
        // You will probably want to handle this event if your plugin uses a Preload callback
        // as there is a chance that after that callback is invoked the game will encounter an error
        // while loading the saved game (eg. corrupted save) which may require you to reset some of your
        // plugin state.
        //SendLoadGameEvent();
        //CreateEventSinks();
        bPlayerIsInCombat = player->IsInCombat();

        logger::trace("kPostLoadGame: sent after an attempt to load a saved game has finished");
        break;

        //case SKSE::MessagingInterface::kSaveGame:
            //    logger::info("kSaveGame");
            //    break;

        //case SKSE::MessagingInterface::kDeleteGame:
            //    // message->dataLen: length of file path, data: char* file path of .ess savegame file
            //    logger::info("kDeleteGame: sent right before deleting the .skse cosave and the .ess save");
            //    break;

    case SKSE::MessagingInterface::kInputLoaded:
        //logger::info("kInputLoaded: sent right after game input is loaded, right before the main menu initializes");

        break;

    case SKSE::MessagingInterface::kNewGame:
        //logger::trace("kNewGame: sent after a new game is created, before the game has loaded");

        DbSkseCppCallbackEventsAttached = false;
        DbSkseCppCallbackLoad(); //attach papyrus DbSkseCppCallbackEvents script to the player and save all ammo projectiles
        RegisterActorsForBowDrawAnimEvents();
        //LogAndMessage("kNewGame: sent after a new game is created, before the game has loaded", trace);
        break;

    case SKSE::MessagingInterface::kDataLoaded:
        // RE::ConsoleLog::GetSingleton()->Print("DbSkseFunctions Installed");
        if (!nullForm) { nullForm = FindNullForm(); }
        gfuncs::Install();
        SKSE::GetPapyrusInterface()->Register(BindPapyrusFunctions);
        SetSettingsFromIniFile();
        CreateEventSinks();
        SaveSkillBooks();
        logger::trace("kDataLoaded: sent after the data handler has loaded all its forms");
        break;

        //default: //
            //    logger::info("Unknown system message of type: {}", message->type);
            //    break;
    }
}

void LoadCallback(SKSE::SerializationInterface* a_intfc) {
    logger::trace("LoadCallback started");

    int max = EventEnum_Last + 1;
    std::uint32_t type, version, length;

    if (!a_intfc) {
        logger::error("{}: a_intfc doesn't exist, aborting load.", __func__);
        return;
    }

    if (bIsLoadingSerialization || bIsSavingSerialization) {
        logger::debug("{}: already loading or saving. loading = {} saving = {}, aborting load.", __func__, bIsLoadingSerialization, bIsSavingSerialization);
        return;
    }

    bIsLoadingSerialization = true;

    while (a_intfc->GetNextRecordInfo(type, version, length)) {
        if (type == 'DBT0') {
            loadTimers(currentMenuModeTimers, 'DBT0', a_intfc);
        }
        else if (type == 'DBT1') {
            loadTimers(currentNoMenuModeTimers, 'DBT1', a_intfc);
        }
        else if (type == 'DBT2') {
            loadTimers(currentTimers, 'DBT2', a_intfc);
        }
        else if (type == 'DBT3') {
            loadTimers(currentGameTimeTimers, 'DBT3', a_intfc);
        }
        else {
            //this is causing ctd on Skyrim AE when fast traveling too many times too quickly.
            /*for (int i = EventEnum_First; i < max; i++) {
                if (type == eventDataPtrs[i]->record) {
                    eventDataPtrs[i]->Load(a_intfc);
                    break;
                }
            }*/
        }
    }

    if (!player) { player = RE::PlayerCharacter::GetSingleton(); }
    bPlayerIsInCombat = player->IsInCombat();

    //EventEnum_OnLoadGame doesn't have an event sink, hence EventEnum_First + 1
    for (int i = EventEnum_First + 1; i < max; i++) {
        AddSink(i);
    }

    auto* args = RE::MakeFunctionArguments();

    //now sending to all scripts with the OnLoadGameGlobal Event
    //gfuncs::RemoveDuplicates(eventDataPtrs[EventEnum_OnLoadGame]->globalHandles);
    //SendEvents(eventDataPtrs[EventEnum_OnLoadGame]->globalHandles, eventDataPtrs[EventEnum_OnLoadGame]->sEvent, args);

    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (vm) {
        vm->SendEventAll(eventDataPtrs[EventEnum_OnLoadGame]->sEvent, args);
    }

    bIsLoadingSerialization = false;
    logger::trace("LoadCallback complete");

    //gfuncs::DelayedFunction(&DbSkseCppCallbackLoad, 1200);
    DbSkseCppCallbackLoad();
    RegisterActorsForBowDrawAnimEvents();
    SaveActorRaces();
}

void SaveCallback(SKSE::SerializationInterface* a_intfc) {
    logger::trace("SaveCallback started");

    if (!a_intfc) {
        logger::error("{}: a_intfc doesn't exist, aborting load.", __func__);
        return;
    }

    if (bIsLoadingSerialization || bIsSavingSerialization) {
        logger::debug("{}: already loading or saving. loading = {} saving = {}, aborting load.", __func__, bIsLoadingSerialization, bIsSavingSerialization);
        return;
    }

    bIsSavingSerialization = true;

    //this is causing ctd on Skyrim AE when fast traveling too many times too quickly.
    /*int max = EventEnum_Last + 1;
    for (int i = EventEnum_First; i < max; i++) {
        eventDataPtrs[i]->Save(a_intfc);
    }*/

    SaveTimers(currentMenuModeTimers, 'DBT0', a_intfc);
    SaveTimers(currentNoMenuModeTimers, 'DBT1', a_intfc);
    SaveTimers(currentTimers, 'DBT2', a_intfc);
    SaveTimers(currentGameTimeTimers, 'DBT3', a_intfc);

    bIsSavingSerialization = false;
    logger::trace("SaveCallback complete");
}

//init================================================================================================================================================================
SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    skseLoadInterface = skse;

    SetupLog();
    SKSE::GetMessagingInterface()->RegisterListener(MessageListener);

    auto* serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID('DbSF');
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);

    return true;
}