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
#include <mutex>
#include <thread>
#include "mini/ini.h"
#include "logger.h"
#include "GeneralFunctions.h"
#include "Magic.h"
#include "Utility.h"
#include "MapMarker.h"
#include "Actor.h"
#include "ArtObject.h"
#include "Book.h"
#include "Cell.h"
#include "ConsoleUtil.h"
#include "CreateForms.h"
#include "FormVectorGetters.h"
#include "PapyrusUtilEx.h"
#include "ProjectileFunctions.h"
#include "UIEventHooks/Hooks.h"
#include "editorID.hpp"
#include "STLThunk.h"
#include "Sound.h"
#include "Serialization.h"
#include "Timers.h"
#include "mINIHelper.h"
#include "FileSystem.h"
#include "UIGfx.h"

namespace logger = SKSE::log;
bool bPlayerIsInCombat = false;
bool bRegisteredForPlayerCombatChange = false;
//bool inMenuMode = false;
bool gamePaused = false;
bool bIsLoadingSerialization = false;
bool bIsSavingSerialization = false;
bool DbSkseCppCallbackEventsAttached = false;
//std::string lastMenuOpened;
RE::TESObjectREFR* lastPlayerActivatedRef = nullptr;
RE::TESObjectREFR* menuRef = nullptr;

//std::vector<RE::BSFixedString> menusCurrentlyOpen;
int numOfMenusCurrentOpen = 0;
std::unordered_map<std::string, bool> menuStatusMap = {
    {std::string(RE::BarterMenu::MENU_NAME), false},
    {std::string(RE::BookMenu::MENU_NAME), false},
    {std::string(RE::Console::MENU_NAME), false},
    {std::string(RE::ConsoleNativeUIMenu::MENU_NAME), false},
    {std::string(RE::ContainerMenu::MENU_NAME), false},
    {std::string(RE::CraftingMenu::MENU_NAME), false},
    {std::string(RE::CreationClubMenu::MENU_NAME), false},
    {std::string(RE::CreditsMenu::MENU_NAME), false},
    {std::string(RE::DialogueMenu::MENU_NAME), false},
    {std::string(RE::FaderMenu::MENU_NAME), false},
    {std::string(RE::FavoritesMenu::MENU_NAME), false},
    {std::string(RE::GiftMenu::MENU_NAME), false},
    {std::string(RE::InventoryMenu::MENU_NAME), false},
    {std::string(RE::JournalMenu::MENU_NAME), false},
    {std::string(RE::KinectMenu::MENU_NAME), false},
    {std::string(RE::LevelUpMenu::MENU_NAME), false},
    {std::string(RE::LoadingMenu::MENU_NAME), false},
    {std::string(RE::LockpickingMenu::MENU_NAME), false},
    {std::string(RE::MagicMenu::MENU_NAME), false},
    {std::string(RE::MainMenu::MENU_NAME), false},
    {std::string(RE::MapMenu::MENU_NAME), false},
    {std::string(RE::MessageBoxMenu::MENU_NAME), false},
    {std::string(RE::MistMenu::MENU_NAME), false},
    {std::string(RE::ModManagerMenu::MENU_NAME), false},
    {std::string(RE::RaceSexMenu::MENU_NAME), false},
    {std::string(RE::SafeZoneMenu::MENU_NAME), false},
    {std::string(RE::SleepWaitMenu::MENU_NAME), false},
    {std::string(RE::StatsMenu::MENU_NAME), false},
    {std::string(RE::TitleSequenceMenu::MENU_NAME), false},
    {std::string(RE::TrainingMenu::MENU_NAME), false},
    {std::string(RE::TutorialMenu::MENU_NAME), false},
    {std::string(RE::TweenMenu::MENU_NAME), false}
};

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

int iMaxArrowsSavedPerReference = 0;
float secondsPassedGameNotPaused = 0.0;
float lastFrameDelta = 0.1;
RE::PlayerCharacter* player;
RE::Actor* playerRef;
RE::TESForm* nullForm;
RE::TESForm* xMarker;
RE::BSScript::IVirtualMachine* gvm;
RE::SkyrimVM* svm;
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
//std::string GetFormEditorId(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string nullFormString);

struct TrackedActorAmmoData {
    RE::TESAmmo* ammo;
    float gameTimeStamp; //game time when the ammo was saved with the BowReleased animation event (if the ammo changed between BowDraw and BowReleased)
    std::chrono::system_clock::time_point timeStamp; //time point when the ammo was saved with the BowReleased animation event  (if the ammo changed between BowDraw and BowReleased)
};

//general functions============================================================================================================================================================

void SetSettingsFromIniFile() {
    mINI::INIFile file("Data/SKSE/Plugins/DbSkseFunctions.ini");
    mINI::INIStructure ini;
    file.read(ini);

    gameTimerPollingInterval = (mINI::GetIniFloat(ini, "Main", "gameTimerPollingInterval", 1.5) * 1000);
    if (gameTimerPollingInterval < 100) {
        gameTimerPollingInterval = 100;
    }

    iMaxArrowsSavedPerReference = mINI::GetIniInt(ini, "Main", "iMaxArrowsSavedPerReference", 0);
    if (iMaxArrowsSavedPerReference < 0) {
        iMaxArrowsSavedPerReference = 0;
    }

    logger::info("gameTimerPollingInterval set to {} ", gameTimerPollingInterval);
    logger::info("iMaxArrowsSavedPerReference set to {} ", iMaxArrowsSavedPerReference);
}

enum logLevel { trace, debug, info, warn, error, critical };
enum debugLevel { notification, messageBox };

//papyrus functions=============================================================================================================================
float GetThisVersion(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::StaticFunctionTag* functionTag) {
    return float(8.7);
}

bool SetAliasQuestObjectFlag(RE::StaticFunctionTag*, RE::BGSBaseAlias* akAlias, bool set) {
    return gfuncs::SetAliasQuestObjectFlag(akAlias, set);
}

bool IsAliasQuestObjectFlagSet(RE::StaticFunctionTag*, RE::BGSBaseAlias* akAlias) {
    return gfuncs::IsAliasQuestObjectFlagSet(akAlias);
}

RE::TESObjectREFR* GetLastPlayerActivatedRef(RE::StaticFunctionTag*) {
    if (gfuncs::IsFormValid(lastPlayerActivatedRef)) {
        return lastPlayerActivatedRef;
    }
    else {
        return nullptr;
    }
}

RE::TESObjectREFR* GetLastPlayerMenuActivatedRef(RE::StaticFunctionTag*) {
    if (gfuncs::IsFormValid(menuRef)) {
        return menuRef;
    }
    else {
        return nullptr;
    }
}

RE::TESObjectREFR* GetAshPileLinkedRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("ref doesn't exist or isn't valid");
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

void ExecuteConsoleCommand(RE::StaticFunctionTag*, std::string a_command, RE::TESObjectREFR* objRef) {
    logger::trace("called. Command = {}", a_command);
    ConsoleUtil::ExecuteCommand(a_command, objRef);
}

bool HasCollision(RE::StaticFunctionTag*, RE::TESObjectREFR* objRef) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(objRef)) {
        logger::warn("objRef doesn't exist");
        return false;
    }
    return objRef->HasCollision();
}

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
    //logger::debug("sending event to papyrus to save ammo projectiles. Size of ammos = {}", allAmmos.size());
}

//called from DbSkseCppCallbackEvents papyrus script on init and game load
void SetDbSkseCppCallbackEventsAttached(RE::StaticFunctionTag*) {
    //DbSkseCppCallbackEventsAttached = true;
    //logger::trace("called");
    //SaveAllAmmoProjectilesFromPapyrus();
}

void DbSkseCppCallbackLoad() {
    //if (!gfuncs::IsScriptAttachedToRef(playerRef, "DbSkseCppCallbackEvents")) {
    //    logger::debug("attaching DbSkseCppCallbackEvents script to the player.");
    //    ConsoleUtil::ExecuteCommand("player.aps DbSkseCppCallbackEvents", nullptr);
    // 
    //}
    //else {
    //    logger::debug("DbSkseCppCallbackEvents script already attached to the player");
    //}
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

        for (int i = 0; i < eventParamMaps.size(); i++) {
            auto it = eventParamMaps[i].find(playerRef);
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
                    logger::debug("akForm[{}] ID[{:x}] found, handles sixe[{}]", gfuncs::GetFormName(akForm), akForm->GetFormID(), it->second.size());
                }
            }
            else { //form not found
                std::vector<RE::VMHandle> handles;
                handles.push_back(handle);
                eventFormHandles.insert(std::pair<RE::TESForm*, std::vector<RE::VMHandle>>(akForm, handles));

                logger::debug("akForm[{}] ID[{:x}] doesn't already have handles, handles size[{}] eventFormHandles size[{}]", gfuncs::GetFormName(akForm), akForm->GetFormID(), handles.size(), eventFormHandles.size());
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
                logger::debug("adding handle for Combat State change");
                if (paramFilter->As<RE::Actor>() == playerRef) {
                    //playerForm = paramFilter;
                    bRegisteredForPlayerCombatChange = true;
                    logger::debug("bRegisteredForPlayerCombatChange = true");
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
            logger::debug("eventSinkIndex [{}] Index = [{}]", eventSinkIndex, index);
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
        if (!serialize::LoadHandlesVector(globalHandles, record, a_intfc)) {
            return false;
        }

        std::size_t size;
        if (!a_intfc->ReadRecordData(size)) {
            logger::error("Event[{}] Failed to load size of eventParamMaps!", sEvent);
            return false;
        }

        for (std::size_t i = 0; i < size; i++) {
            if (!serialize::LoadFormHandlesMap(eventParamMaps[i], record, a_intfc)) {
                return false;
            }
        }
        return true;
    }

    bool Save(SKSE::SerializationInterface* a_intfc) {
        if (!a_intfc->OpenRecord(record, 1)) {
            logger::error("Failed to open record[{}]", record);
            return false;
        }

        if (!serialize::SaveHandlesVector(globalHandles, record, a_intfc)) {
            return false;
        }

        const std::size_t size = eventParamMaps.size();

        if (!a_intfc->WriteRecordData(size)) {
            logger::error("record[{}] Failed to write size of arr!", record);
            return false;
        }

        for (std::size_t i = 0; i < size; i++) {
            if (!serialize::SaveFormHandlesMap(eventParamMaps[i], record, a_intfc)) {
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

    gfuncs::CombineEventHandles(handles, data.shooter, eventDataPtrs[EventEnum_OnProjectileImpact]->eventParamMaps[0]);
    gfuncs::CombineEventHandles(handles, data.target, eventDataPtrs[EventEnum_OnProjectileImpact]->eventParamMaps[1]);
    gfuncs::CombineEventHandles(handles, source, eventDataPtrs[EventEnum_OnProjectileImpact]->eventParamMaps[2]);
    gfuncs::CombineEventHandles(handles, data.ammo, eventDataPtrs[EventEnum_OnProjectileImpact]->eventParamMaps[3]);
    gfuncs::CombineEventHandles(handles, data.projectileBase, eventDataPtrs[EventEnum_OnProjectileImpact]->eventParamMaps[3]);

    gfuncs::RemoveDuplicates(handles);

    auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)data.shooter, (RE::TESObjectREFR*)data.target, (RE::TESForm*)source,
        (RE::TESAmmo*)data.ammo, (RE::BGSProjectile*)data.projectileBase, (bool)SneakAttack, (bool)HitBlocked,
        (int)data.impactResult, (int)data.collidedLayer, (float)data.distanceTraveled, (std::string)data.hitPartNodeName,
        (RE::TESObjectREFR*)data.projectileMarker, (std::vector<float>)hitTranslation);

    gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnProjectileImpact]->sEvent, args);

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
                    //logger::trace("hitTimeDiff = [{}]", hitTimeDiff);
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
            bool bowEquipped = gfuncs::formIsBowOrCrossbow(source);

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

            gfuncs::CombineEventHandles(handles, attacker, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[0]);
            gfuncs::CombineEventHandles(handles, target, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[1]);
            gfuncs::CombineEventHandles(handles, source, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[2]);
            gfuncs::CombineEventHandles(handles, ammo, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[3]);
            gfuncs::CombineEventHandles(handles, projectileForm, eventDataPtrs[EventEnum_HitEvent]->eventParamMaps[4]);

            gfuncs::RemoveDuplicates(handles);

            logger::trace("HitEvent: attacker[{}]  target[{}]  source[{}]  ammo[{}]  projectile[{}]", gfuncs::GetFormName(attacker), gfuncs::GetFormName(target), gfuncs::GetFormName(source), gfuncs::GetFormName(ammo), gfuncs::GetFormName(projectileForm));
            logger::trace("HitEvent: powerAttack[{}]  SneakAttack[{}]  BashAttack[{}]  HitBlocked[{}]", powerAttack, SneakAttack, bBashAttack, HitBlocked);

            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)attacker, (RE::TESObjectREFR*)target, (RE::TESForm*)source,
                (RE::TESAmmo*)ammo, (RE::BGSProjectile*)projectileForm, (bool)powerAttack, (bool)SneakAttack, (bool)bBashAttack, (bool)HitBlocked);

            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_HitEvent]->sEvent, args);
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

void CheckForPlayerCombatStatusChange() {
    logger::trace("");

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

        logger::debug("target[{}]", gfuncs::GetFormName(target));

        // RE::TESForm* Target = .attackedMember.get().get();

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnCombatStateChanged]->globalHandles; //
        gfuncs::CombineEventHandles(handles, playerRef, eventDataPtrs[EventEnum_OnCombatStateChanged]->eventParamMaps[0]);
        gfuncs::CombineEventHandles(handles, target, eventDataPtrs[EventEnum_OnCombatStateChanged]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)playerRef, (RE::Actor*)target, (int)combatState);
        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnCombatStateChanged]->sEvent, args);
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
        gfuncs::CombineEventHandles(handles, actorObjRef, eventDataPtrs[EventEnum_OnCombatStateChanged]->eventParamMaps[0]);
        gfuncs::CombineEventHandles(handles, target, eventDataPtrs[EventEnum_OnCombatStateChanged]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)actorObjRef, (RE::Actor*)target, (int)combatState);
        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnCombatStateChanged]->sEvent, args);

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
        gfuncs::CombineEventHandles(handles, actorObjRef, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        gfuncs::CombineEventHandles(handles, furnitureRef, eventDataPtrs[eventIndex]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)actorRef, (RE::TESObjectREFR*)furnitureRef);
        gfuncs::SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

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

        if (activatorRef == playerRef) {
            if (gfuncs::IsFormValid(activatedRef)) {
                lastPlayerActivatedRef = activatedRef;
            }
        }

        if (eventDataPtrs[EventEnum_OnActivate]->sinkAdded) {
            std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnActivate]->globalHandles;

            gfuncs::CombineEventHandles(handles, activatorRef, eventDataPtrs[EventEnum_OnActivate]->eventParamMaps[0]);
            gfuncs::CombineEventHandles(handles, activatedRef, eventDataPtrs[EventEnum_OnActivate]->eventParamMaps[1]);

            gfuncs::RemoveDuplicates(handles);

            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)activatorRef, (RE::TESObjectREFR*)activatedRef);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnActivate]->sEvent, args);
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
        gfuncs::CombineEventHandles(handles, victim, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        gfuncs::CombineEventHandles(handles, killer, eventDataPtrs[eventIndex]->eventParamMaps[1]);
        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)victim, (RE::Actor*)killer);
        gfuncs::SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

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
                logger::info("actor [{}]", gfuncs::GetFormName(actorRef));
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
                        logger::debug("actor [{}]", gfuncs::GetFormName(actorRef));
                        bUnregistered = true;
                    }
                }
                else {
                    //it->second->bowReleasedTrackedAmmoDatas.clear();
                    delete it->second;
                    it->second = nullptr;
                    animationEventActorsMap.erase(it);
                    logger::debug("actor [{}]", gfuncs::GetFormName(actorRef));
                    bUnregistered = true;
                }
            }
            else { //animation event doesn't exist
                animationEventActorsMap.erase(it);
                logger::debug("actor [{}]", gfuncs::GetFormName(actorRef));
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
            if (gfuncs::formIsBowOrCrossbow(baseObject)) {
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
            else if (gfuncs::formIsBowOrCrossbow(baseObject)) {
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

            gfuncs::CombineEventHandles(handles, akActorRef, eventDataPtrs[eventIndex]->eventParamMaps[0]);
            gfuncs::CombineEventHandles(handles, baseObject, eventDataPtrs[eventIndex]->eventParamMaps[1]);
            gfuncs::CombineEventHandles(handles, ref, eventDataPtrs[eventIndex]->eventParamMaps[2]);

            gfuncs::RemoveDuplicates(handles);

            auto* args = RE::MakeFunctionArguments((RE::Actor*)akActorRef, (RE::TESForm*)baseObject, (RE::TESObjectREFR*)ref);
            gfuncs::SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);
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
        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnWaitStart]->sEvent, args);

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
        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnWaitStop]->sEvent, args);

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

        gfuncs::CombineEventHandles(handles, caster, eventDataPtrs[EventEnum_OnMagicEffectApply]->eventParamMaps[0]);
        gfuncs::CombineEventHandles(handles, target, eventDataPtrs[EventEnum_OnMagicEffectApply]->eventParamMaps[1]);
        gfuncs::CombineEventHandles(handles, magicEffect, eventDataPtrs[EventEnum_OnMagicEffectApply]->eventParamMaps[2]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)caster, (RE::TESObjectREFR*)target, (RE::EffectSetting*)magicEffect);
        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnMagicEffectApply]->sEvent, args);

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
        gfuncs::CombineEventHandles(handles, lockedObject, eventDataPtrs[EventEnum_LockChanged]->eventParamMaps[0]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)lockedObject, (bool)Locked);
        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_LockChanged]->sEvent, args);

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

        gfuncs::CombineEventHandles(handles, activatorRef, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        gfuncs::CombineEventHandles(handles, akActionRef, eventDataPtrs[eventIndex]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)activatorRef, (RE::TESObjectREFR*)akActionRef);
        gfuncs::SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

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

        gfuncs::CombineEventHandles(handles, caster, eventDataPtrs[EventEnum_OnSpellCast]->eventParamMaps[0]);
        gfuncs::CombineEventHandles(handles, spell, eventDataPtrs[EventEnum_OnSpellCast]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)caster, (RE::TESForm*)spell);
        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnSpellCast]->sEvent, args);

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

        gfuncs::CombineEventHandles(handles, newContainer, eventDataPtrs[EventEnum_OnContainerChanged]->eventParamMaps[0]);
        gfuncs::CombineEventHandles(handles, oldContainer, eventDataPtrs[EventEnum_OnContainerChanged]->eventParamMaps[1]);
        gfuncs::CombineEventHandles(handles, itemReference, eventDataPtrs[EventEnum_OnContainerChanged]->eventParamMaps[2]);
        gfuncs::CombineEventHandles(handles, baseObj, eventDataPtrs[EventEnum_OnContainerChanged]->eventParamMaps[3]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)newContainer, (RE::TESObjectREFR*)oldContainer, (RE::TESObjectREFR*)itemReference, (RE::TESForm*)baseObj, (int)itemCount);
        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnContainerChanged]->sEvent, args);

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

        gfuncs::CombineEventHandles(handles, akActor, eventDataPtrs[eventIndex]->eventParamMaps[0]);
        gfuncs::CombineEventHandles(handles, akSource, eventDataPtrs[eventIndex]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor, (RE::TESForm*)akSource, (int)slot);
        gfuncs::SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

        //draw / sheathe events aren't triggered for left hand. Send left hand events manually
        if (eventIndex >= EventEnum_BeginDraw && eventIndex <= EventEnum_EndSheathe) {
            if (gfuncs::IsFormValid(akActor)) {

                RE::TESForm* leftHandSource = akActor->GetEquippedObject(true);

                if (gfuncs::IsFormValid(leftHandSource)) {
                    if (leftHandSource != akSource) {
                        std::vector<RE::VMHandle> handlesB = eventDataPtrs[eventIndex]->globalHandles;

                        gfuncs::CombineEventHandles(handlesB, akActor, eventDataPtrs[eventIndex]->eventParamMaps[0]);
                        gfuncs::CombineEventHandles(handlesB, leftHandSource, eventDataPtrs[eventIndex]->eventParamMaps[1]);

                        gfuncs::RemoveDuplicates(handlesB);

                        auto* argsB = RE::MakeFunctionArguments((RE::Actor*)akActor, (RE::TESForm*)leftHandSource, (int)0);
                        gfuncs::SendEvents(handlesB, eventDataPtrs[eventIndex]->sEvent, argsB);

                        logger::trace("action event, Actor[{}] Source[{}]  type[{}] slot[{}]", gfuncs::GetFormName(akActor), gfuncs::GetFormName(leftHandSource),
                            actionTypeStrings[type], actionSlotStrings[0]);
                    }
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

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

        auto* aiProcess = playerRef->GetActorRuntimeData().currentProcess;
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
            if (gfuncs::IsFormValid(menuRef)) {
                workbenchRef = menuRef;
                //logger::trace("Item Crafted Event: occupiedFurniture not valid, setting to menuRef");
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

        gfuncs::CombineEventHandles(handles, craftedItem, eventDataPtrs[EventEnum_OnItemCrafted]->eventParamMaps[0]);
        gfuncs::CombineEventHandles(handles, workbenchRef, eventDataPtrs[EventEnum_OnItemCrafted]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        if (benchType == 1) { //CreateObject
            count = UIEvents::uiLastSelectedFormData.count;
        }

        auto* args = RE::MakeFunctionArguments((RE::TESForm*)craftedItem, (RE::TESObjectREFR*)workbenchRef,
            (int)count, (int)benchType, (std::string)skill);

        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnItemCrafted]->sEvent, args);

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

        if (gfuncs::IsFormValid(menuRef)) {
            target = menuRef->As<RE::Actor>();
            if (!gfuncs::IsFormValid(target)) {
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnItemsPickpocketed]->globalHandles;
        gfuncs::CombineEventHandles(handles, target, eventDataPtrs[EventEnum_OnItemsPickpocketed]->eventParamMaps[0]);
        gfuncs::CombineEventHandles(handles, akForm, eventDataPtrs[EventEnum_OnItemsPickpocketed]->eventParamMaps[1]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::Actor*)target, (RE::TESForm*)akForm, (int)event->numItems);

        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnItemsPickpocketed]->sEvent, args);

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

        RE::BGSLocation* location = playerRef->GetCurrentLocation();
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
            gfuncs::CombineEventHandles(handles, location, eventDataPtrs[EventEnum_OnLocationCleared]->eventParamMaps[0]);
        }
        else {
            location = nullptr;
        }

        gfuncs::RemoveDuplicates(handles);
        auto* args = RE::MakeFunctionArguments((RE::BGSLocation*)location);

        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnLocationCleared]->sEvent, args);

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
        gfuncs::CombineEventHandles(handles, akActor, eventDataPtrs[EventEnum_OnEnterBleedout]->eventParamMaps[0]);

        gfuncs::RemoveDuplicates(handles);
        auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor);

        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnEnterBleedout]->sEvent, args);

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
        gfuncs::CombineEventHandles(handles, akActor, eventDataPtrs[EventEnum_OnSwitchRaceComplete]->eventParamMaps[0]);
        gfuncs::CombineEventHandles(handles, akOldRace, eventDataPtrs[EventEnum_OnSwitchRaceComplete]->eventParamMaps[1]);
        gfuncs::CombineEventHandles(handles, akNewRace, eventDataPtrs[EventEnum_OnSwitchRaceComplete]->eventParamMaps[2]);

        gfuncs::RemoveDuplicates(handles);
        auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor, (RE::TESRace*)akOldRace, (RE::TESRace*)akNewRace);

        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnSwitchRaceComplete]->sEvent, args);

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
        gfuncs::CombineEventHandles(handles, akActor, eventDataPtrs[EventEnum_OnActorFootStep]->eventParamMaps[0]);

        gfuncs::RemoveDuplicates(handles);
        auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor, (std::string)event->tag);

        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnActorFootStep]->sEvent, args);

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
        gfuncs::CombineEventHandles(handles, akQuest, eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->eventParamMaps[0]);

        gfuncs::RemoveDuplicates(handles);

        auto* args = RE::MakeFunctionArguments((RE::TESQuest*)akQuest, (std::string)displayText, (int)oldState, (int)newState,
            (int)objectiveIndex, (std::vector<RE::BGSBaseAlias*>)ojbectiveAliases);

        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->sEvent, args);

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
        //logger::critical("AE Offset [{:x}]", offset);
        return reinterpret_cast<RE::PLAYER_TARGET_LOC*>((uintptr_t)player + offset);
    }
    else if (REL::Module::IsSE()) {
        uint32_t offset = 0x640;
        //logger::critical("SE Offset [{:x}]", offset);
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
            gfuncs::CombineEventHandles(handles, fastTravelMarker, eventDataPtrs[EventEnum_OnPositionPlayerStart]->eventParamMaps[0]);
            gfuncs::CombineEventHandles(handles, moveToRef, eventDataPtrs[EventEnum_OnPositionPlayerStart]->eventParamMaps[1]);
            gfuncs::CombineEventHandles(handles, akWorldSpace, eventDataPtrs[EventEnum_OnPositionPlayerStart]->eventParamMaps[2]);
            gfuncs::CombineEventHandles(handles, interiorCell, eventDataPtrs[EventEnum_OnPositionPlayerStart]->eventParamMaps[3]);

            gfuncs::RemoveDuplicates(handles);

            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)fastTravelMarker, (RE::TESObjectREFR*)moveToRef,
                (RE::TESWorldSpace*)akWorldSpace, (RE::TESObjectCELL*)interiorCell);

            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnPositionPlayerStart]->sEvent, args);
        }
        else if (type == 4) { //post load
            std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnPositionPlayerFinish]->globalHandles;
            gfuncs::CombineEventHandles(handles, fastTravelMarker, eventDataPtrs[EventEnum_OnPositionPlayerFinish]->eventParamMaps[0]);
            gfuncs::CombineEventHandles(handles, moveToRef, eventDataPtrs[EventEnum_OnPositionPlayerFinish]->eventParamMaps[1]);
            gfuncs::CombineEventHandles(handles, akWorldSpace, eventDataPtrs[EventEnum_OnPositionPlayerFinish]->eventParamMaps[2]);
            gfuncs::CombineEventHandles(handles, interiorCell, eventDataPtrs[EventEnum_OnPositionPlayerFinish]->eventParamMaps[3]);

            gfuncs::RemoveDuplicates(handles);

            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)fastTravelMarker, (RE::TESObjectREFR*)moveToRef,
                (RE::TESWorldSpace*)akWorldSpace, (RE::TESObjectCELL*)interiorCell);

            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnPositionPlayerFinish]->sEvent, args);
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
                newCell = playerRef->GetParentCell();
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
                gfuncs::CombineEventHandles(handles, newCell, eventDataPtrs[EventEnum_OnPlayerChangeCell]->eventParamMaps[0]);
                gfuncs::CombineEventHandles(handles, akPreviousCell, eventDataPtrs[EventEnum_OnPlayerChangeCell]->eventParamMaps[1]);

                gfuncs::RemoveDuplicates(handles);

                auto* args = RE::MakeFunctionArguments((RE::TESObjectCELL*)newCell, (RE::TESObjectCELL*)akPreviousCell);

                gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnPlayerChangeCell]->sEvent, args);
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

bool IsItemMenuOpenNative(RE::StaticFunctionTag*) {
    return ui->IsItemMenuOpen();
}

bool IsItemMenuOpen() {
    for (auto& menu : itemMenus) {
        if (ui->IsMenuOpen(menu)) {
            return true;
        }
    }
    return false;
}

bool IsRefActivatedMenu(RE::BSFixedString& menu) {
    return (
        menu == RE::DialogueMenu::MENU_NAME ||
        menu == RE::BarterMenu::MENU_NAME ||
        menu == RE::GiftMenu::MENU_NAME ||
        menu == RE::LockpickingMenu::MENU_NAME ||
        menu == RE::ContainerMenu::MENU_NAME ||
        menu == RE::BookMenu::MENU_NAME ||
        menu == RE::CraftingMenu::MENU_NAME
        );
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
            logger::warn("MenuOpenClose Event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::warn("MenuOpenCloseEventSink IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        //logger::trace("Menu Open Close Event, menu[{}], opened[{}]", event->menuName, event->opening);

        if (event->menuName != RE::HUDMenu::MENU_NAME) { //hud menu is always open, don't need to do anything for it.

            RE::BSFixedString bsMenuName = event->menuName;
            std::string sMenuName = std::string(bsMenuName);
            auto menuStatusMapItr = menuStatusMap.find(sMenuName);
            bool menuFoundInStatusMap = (menuStatusMapItr != menuStatusMap.end());

            if (event->opening) {
                lastMenuOpened = event->menuName;

                if (menuFoundInStatusMap) {
                    if (menuStatusMapItr->second == false) {
                        numOfMenusCurrentOpen += 1;
                        menuStatusMapItr->second = true;
                    }
                }

                //logger::trace("menu [{}] opened numOfMenusCurrentOpen[{}]", event->menuName, numOfMenusCurrentOpen);

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

                //AddToMenusCurrentlyOpen(event->menuName);
               /* std::thread tAddToMenusCurrentlyOpen(AddToMenusCurrentlyOpen, event->menuName);
                tAddToMenusCurrentlyOpen.join();*/

                if (IsRefActivatedMenu(bsMenuName)) {
                    menuRef = lastPlayerActivatedRef;
                    logger::trace("Menu[{}] opened. saved menuRef[{}]", bsMenuName, gfuncs::GetFormName(menuRef));
                }

                /*if (gfuncs::GetIndexInVector(refActivatedMenus, bsMenuName) > -1) {
                    menuRef = lastPlayerActivatedRef;
                    logger::trace("Menu[{}] opened. saved menuRef[{}]", bsMenuName, gfuncs::GetFormName(menuRef));
                }*/

                if (IsItemMenuOpen()) {
                    if (UIEvents::registeredUIEventDatas.size() > 0) {
                        if (!inputEventSink->sinkAdded) {
                            inputEventSink->sinkAdded = true;
                            RE::BSInputDeviceManager::GetSingleton()->AddEventSink(inputEventSink);
                            logger::trace("item menu [{}] opened. Added input event sink", bsMenuName);
                        }
                    }
                }

                /*if (gfuncs::GetIndexInVector(itemMenus, bsMenuName) > -1) {
                    numOfItemMenusCurrentOpen += 1;
                }*/
            }
            else {
                if (!ui->GameIsPaused() && gamePaused) {
                    gamePaused = false;
                    auto now = std::chrono::system_clock::now();
                    float fGamePausedTime = gfuncs::timePointDiffToFloat(now, lastTimeGameWasPaused);
                    //UpdateTimers(fGamePausedTime);
                    std::thread tUpdateTimers(timers::UpdateTimers, fGamePausedTime);
                    tUpdateTimers.join();
                    logger::trace("game was unpaused");
                }

                if (menuFoundInStatusMap) {
                    if (menuStatusMapItr->second == true) {
                        menuStatusMapItr->second = false;
                        numOfMenusCurrentOpen -= 1;
                        if (numOfMenusCurrentOpen == 0) { //closed menu
                            inMenuMode = false;
                            auto now = std::chrono::system_clock::now();

                            float timePointDiff = gfuncs::timePointDiffToFloat(now, lastTimeMenuWasOpened);
                            //UpdateNoMenuModeTimers(timePointDiff);
                            std::thread tUpdateNoMenuModeTimers(timers::UpdateNoMenuModeTimers, timePointDiff);
                            tUpdateNoMenuModeTimers.join();
                            logger::trace("inMenuMode = false");
                        }
                    }
                }

                //logger::trace("menu [{}] closed numOfMenusCurrentOpen[{}]", event->menuName, numOfMenusCurrentOpen);

                if (!IsItemMenuOpen()) {
                    if (inputEventSink->sinkAdded) {
                        inputEventSink->sinkAdded = false;
                        RE::BSInputDeviceManager::GetSingleton()->RemoveEventSink(inputEventSink);
                        logger::trace("item menu [{}] closed. Removed input event sink", bsMenuName);
                    }
                }

                /*if (gfuncs::GetIndexInVector(itemMenus, bsMenuName) > -1) {
                    numOfItemMenusCurrentOpen -= 1;

                    if (numOfItemMenusCurrentOpen == 0) {

                    }
                }*/
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
    logger::trace("");
    if (!eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty() || !eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty() || !eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
        if (!equipEventSink->sinkAdded) {
            equipEventSink->sinkAdded = true;
            eventSourceholder->AddEventSink(equipEventSink);
            RegisterActorsForBowDrawAnimEvents();
            logger::debug("Equip Event Sink Added");
            return true;
        }
    }
    return false;
}

bool RemoveEquipEventSink() {
    logger::trace("");
    if (eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty() && eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty() && eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
        if (equipEventSink->sinkAdded) {
            equipEventSink->sinkAdded = false;
            eventSourceholder->RemoveEventSink(equipEventSink); //always added to track recent hit projectiles for the GetRecentHitArrowRefsMap function
            logger::debug("Equip Event Sink Removed");
            return true;
        }
    }
    return false;
}

bool AddHitEventSink() {
    if (!hitEventSink->sinkAdded && (!eventDataPtrs[EventEnum_HitEvent]->isEmpty() || !eventDataPtrs[EventEnum_OnProjectileImpact]->isEmpty())) {
        hitEventSink->sinkAdded = true;
        eventSourceholder->AddEventSink(hitEventSink);
        logger::trace("");
        return true;
    }
    return false;
}

bool RemoveHitEventSink() {
    if (hitEventSink->sinkAdded && eventDataPtrs[EventEnum_HitEvent]->isEmpty() && eventDataPtrs[EventEnum_OnProjectileImpact]->isEmpty()) {
        hitEventSink->sinkAdded = false;
        eventSourceholder->RemoveEventSink(hitEventSink);
        logger::debug("");
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
            logger::debug("EventEnum_OnCombatStateChanged sink added");
        }
        break;

    case EventEnum_FurnitureEnter:

    case EventEnum_FurnitureExit:
        if (!furnitureEventSink->sinkAdded && (!eventDataPtrs[EventEnum_FurnitureEnter]->isEmpty() || !eventDataPtrs[EventEnum_FurnitureExit]->isEmpty())) {
            furnitureEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded = true;
            eventSourceholder->AddEventSink(furnitureEventSink);
            logger::debug("EventEnum_FurnitureExit sink added");
        }
        break;

    case EventEnum_OnActivate:
        if (!eventDataPtrs[EventEnum_OnActivate]->sinkAdded && !eventDataPtrs[EventEnum_OnActivate]->isEmpty()) {
            eventDataPtrs[EventEnum_OnActivate]->sinkAdded = true;
            //eventSourceholder->AddEventSink(activateEventSink); //always activate to track lastPlayerActivatedRef
            logger::debug("EventEnum_OnActivate sink added");
        }
        break;

    case EventEnum_HitEvent:
        if (AddHitEventSink()) {
            //logger::debug("EventEnum_hitEvent sink added");
        }
        if (AddEquipEventSink()); {
            logger::debug("EventEnum_HitEvent Equip event sink added");
        }
        if (!eventDataPtrs[EventEnum_HitEvent]->sinkAdded && !eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
            eventDataPtrs[EventEnum_HitEvent]->sinkAdded = true;
            logger::debug("EventEnum_hitEvent sink added");
        }
        break;

    case EventEnum_DeathEvent:

    case EventEnum_DyingEvent:
        if (!deathEventSink->sinkAdded && (!eventDataPtrs[EventEnum_DeathEvent]->isEmpty() || !eventDataPtrs[EventEnum_DyingEvent]->isEmpty())) {
            deathEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_DeathEvent]->sinkAdded = true;
            eventSourceholder->AddEventSink(deathEventSink);
            logger::debug("EventEnum_DyingEvent sink added");
        }
        break;

    case EventEnum_OnObjectEquipped:
        if (!eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty()) {
            eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded = true;
            logger::debug("EventEnum_OnObjectEquipped sink added");
        }
        if (AddEquipEventSink()) {
            logger::debug("EventEnum_OnObjectEquipped Equip event sink added");
        }
        break;

    case EventEnum_OnObjectUnequipped:
        if (!eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty()) {
            eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded = true;
            logger::debug("EventEnum_OnObjectUnequipped sink added");
        }
        if (AddEquipEventSink()) {
            logger::debug("EventEnum_OnObjectUnequipped Equip event sink added");
        }
        break;

    case EventEnum_OnWaitStart:
        if (!waitStartEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnWaitStart]->isEmpty()) {
            waitStartEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnWaitStart]->sinkAdded = true;
            eventSourceholder->AddEventSink(waitStartEventSink);
            logger::debug("EventEnum_OnWaitStart sink added");
        }
        break;

    case EventEnum_OnWaitStop:
        if (!waitStopEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnWaitStop]->isEmpty()) {
            waitStopEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnWaitStop]->sinkAdded = true;
            eventSourceholder->AddEventSink(waitStopEventSink);
            logger::debug("EventEnum_OnWaitStop sink added");
        }
        break;

    case EventEnum_OnMagicEffectApply:
        if (!magicEffectApplyEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnMagicEffectApply]->isEmpty()) {
            magicEffectApplyEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnMagicEffectApply]->sinkAdded = true;
            eventSourceholder->AddEventSink(magicEffectApplyEventSink);
            logger::debug("EventEnum_OnMagicEffectApply sink added");
        }
        break;

    case EventEnum_OnSpellCast:
        if (!spellCastEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnSpellCast]->isEmpty()) {
            spellCastEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnSpellCast]->sinkAdded = true;
            eventSourceholder->AddEventSink(spellCastEventSink);
            logger::debug("EventEnum_OnSpellCast sink added");
        }
        break;

    case EventEnum_OnContainerChanged:
        if (!containerChangedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnContainerChanged]->isEmpty()) {
            containerChangedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnContainerChanged]->sinkAdded = true;
            eventSourceholder->AddEventSink(containerChangedEventSink);
            logger::debug("EventEnum_OnContainerChanged sink added");
        }
        break;

    case EventEnum_LockChanged:
        if (!lockChangedEventSink->sinkAdded && !eventDataPtrs[EventEnum_LockChanged]->isEmpty()) {
            lockChangedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_LockChanged]->sinkAdded = true;
            eventSourceholder->AddEventSink(lockChangedEventSink);
            logger::debug("EventEnum_LockChanged sink added");
        }
        break;

    case EventEnum_OnOpen:

    case EventEnum_OnClose:
        if (!openCloseEventSink->sinkAdded && (!eventDataPtrs[EventEnum_OnOpen]->isEmpty() || !eventDataPtrs[EventEnum_OnClose]->isEmpty())) {
            openCloseEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnOpen]->sinkAdded = true;
            eventSourceholder->AddEventSink(openCloseEventSink);
            logger::debug("EventEnum_OnClose sink added");
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
            logger::debug("actorActionEventSink sink added");
            eventDataPtrs[EventEnum_EndSheathe]->sinkAdded = true;
            SKSE::GetActionEventSource()->AddEventSink(actorActionEventSink);
        }
        break;

    case EventEnum_OnProjectileImpact:
        if (AddHitEventSink()) {

        }
        if (!eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded && !eventDataPtrs[EventEnum_OnProjectileImpact]->isEmpty()) {
            eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded = true;
            logger::debug("EventEnum_OnProjectileImpact sink added");
        }
        break;

    case EventEnum_OnItemCrafted:
        if (!itemCraftedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnItemCrafted]->isEmpty()) {
            itemCraftedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnItemCrafted]->sinkAdded = true;
            UIEvents::InstallUiEventHook(UIEvents::UiItemMenuEnum_Crafting_Menu); //track crafting menu selection data, (mostly needed for item count).
            RE::ItemCrafted::GetEventSource()->AddEventSink(itemCraftedEventSink);
            logger::debug("EventEnum_OnItemCrafted sink added");
        }
        break;

    case EventEnum_OnItemsPickpocketed:
        if (!itemsPickpocketedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnItemsPickpocketed]->isEmpty()) {
            itemsPickpocketedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnItemsPickpocketed]->sinkAdded = true;
            UIEvents::InstallUiEventHook(UIEvents::UiItemMenuEnum_ContainerMenu); //track container menu selection data, (to get the form taken / pickpocketed).
            RE::ItemsPickpocketed::GetEventSource()->AddEventSink(itemsPickpocketedEventSink);
            logger::debug("EventEnum_OnItemsPickpocketed sink added");
        }
        break;

    case EventEnum_OnLocationCleared:
        if (!locationClearedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnLocationCleared]->isEmpty()) {
            locationClearedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnLocationCleared]->sinkAdded = true;
            RE::LocationCleared::GetEventSource()->AddEventSink(locationClearedEventSink);
            logger::debug("EventEnum_OnLocationCleared sink added");
        }
        break;

    case EventEnum_OnEnterBleedout:
        if (!enterBleedoutEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnEnterBleedout]->isEmpty()) {
            enterBleedoutEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnEnterBleedout]->sinkAdded = true;
            eventSourceholder->AddEventSink(enterBleedoutEventSink);
            logger::debug("EventEnum_OnEnterBleedout sink added");
        }
        break;

    case EventEnum_OnSwitchRaceComplete:
        if (!switchRaceCompleteEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnSwitchRaceComplete]->isEmpty()) {
            switchRaceCompleteEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnSwitchRaceComplete]->sinkAdded = true;
            SaveActorRaces(); //save current actors loaded in game race's
            eventSourceholder->AddEventSink(switchRaceCompleteEventSink);
            eventSourceholder->AddEventSink(objectInitEventSink); //save new actors loaded races to send akOldRace parameter on switchRaceComplete event
            logger::debug("EventEnum_OnSwitchRaceComplete sink added");
        }
        break;

    case EventEnum_OnActorFootStep:
        if (!footstepEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnActorFootStep]->isEmpty()) {
            footstepEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnActorFootStep]->sinkAdded = true;
            RE::BGSFootstepManager::GetSingleton()->AddEventSink(footstepEventSink);
            logger::debug("EventEnum_OnActorFootStep sink added");
        }
        break;

    case EventEnum_OnQuestObjectiveStateChanged:
        if (!questObjectiveEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->isEmpty()) {
            questObjectiveEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->sinkAdded = true;
            RE::ObjectiveState::GetEventSource()->AddEventSink(questObjectiveEventSink);
            logger::debug("EventEnum_OnQuestObjectiveStateChanged sink added");
        }
        break;

    case EventEnum_OnPositionPlayerStart:

    case EventEnum_OnPositionPlayerFinish:
        if (!positionPlayerEventSink->sinkAdded && ShouldPositionPlayerEventSinkBeAdded()) {
            auto* posPlayerEventSource = player->AsPositionPlayerEventSource();
            if (posPlayerEventSource) {
                positionPlayerEventSink->sinkAdded = true;
                posPlayerEventSource->AddEventSink(positionPlayerEventSink);
                logger::debug("positionPlayerEventSink added");
            }
            else {
                logger::error("posPlayerEventSource not found, positionPlayerEventSink sink not added");
            }
        }
        break;

    case EventEnum_OnPlayerChangeCell:
        if (!actorCellEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnPlayerChangeCell]->isEmpty()) {
            auto* playerCellChangeSource = player->AsBGSActorCellEventSource();
            if (playerCellChangeSource) {
                actorCellEventSink->sinkAdded = true;
                eventDataPtrs[EventEnum_OnPlayerChangeCell]->sinkAdded = true;
                //actorCellEventSink->previousCell = playerRef->GetParentCell();
                playerCellChangeSource->AddEventSink(actorCellEventSink);
                logger::debug("EventEnum_OnPlayerChangeCell sink added");
            }
            else {
                logger::error("playerCellChangeSource not found, EventEnum_OnPlayerChangeCell sink not added");
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
            logger::debug("EventEnum_OnCombatStateChanged sink removed");
        }
        break;

    case EventEnum_FurnitureEnter:

    case EventEnum_FurnitureExit:
        if (furnitureEventSink->sinkAdded && eventDataPtrs[EventEnum_FurnitureEnter]->isEmpty() && eventDataPtrs[EventEnum_FurnitureExit]->isEmpty()) {
            furnitureEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(furnitureEventSink);
            logger::debug("EventEnum_FurnitureEnter sink removed");
        }
        break;

    case EventEnum_OnActivate:
        if (eventDataPtrs[EventEnum_OnActivate]->sinkAdded && eventDataPtrs[EventEnum_OnActivate]->isEmpty()) {
            eventDataPtrs[EventEnum_OnActivate]->sinkAdded = false;
            //eventSourceholder->RemoveEventSink(activateEventSink); //always activate to track lastPlayerActivatedRef
            logger::debug("EventEnum_OnActivate sink removed");
        }
        break;

    case EventEnum_HitEvent:
        if (RemoveHitEventSink()) {
            //logger::debug("EventEnum_hitEvent sink removed");
        }
        if (RemoveEquipEventSink()) {
            logger::debug("EventEnum_HitEvent equip event sink removed");
        }
        if (eventDataPtrs[EventEnum_HitEvent]->sinkAdded && eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
            eventDataPtrs[EventEnum_HitEvent]->sinkAdded = false;
            logger::debug("EventEnum_hitEvent sink removed");
        }
        break;

    case EventEnum_DeathEvent:

    case EventEnum_DyingEvent:
        if (deathEventSink->sinkAdded && eventDataPtrs[EventEnum_DeathEvent]->isEmpty() && eventDataPtrs[EventEnum_DyingEvent]->isEmpty()) {
            deathEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_DeathEvent]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(deathEventSink);
            logger::debug("EventEnum_DeathEvent sink removed");
        }
        break;

    case EventEnum_OnObjectEquipped:
        if (eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded && eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty()) {
            eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded = false;
            logger::debug("EventEnum_OnObjectEquipped sink removed");
        }
        if (RemoveEquipEventSink()) {
            logger::debug("EventEnum_OnObjectEquipped equip event sink removed");
        }
        break;

    case EventEnum_OnObjectUnequipped:
        if (eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded && eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty()) {
            eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded = false;
            logger::debug("EventEnum_OnObjectUnequipped sink removed");
        }
        if (RemoveEquipEventSink()) {
            logger::debug("EventEnum_OnObjectUnequipped equip event sink removed");
        }
        break;

    case EventEnum_OnWaitStart:
        if (waitStartEventSink->sinkAdded && eventDataPtrs[EventEnum_OnWaitStart]->isEmpty()) {
            waitStartEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnWaitStart]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(waitStartEventSink);
            logger::debug("EventEnum_OnWaitStart sink removed");
        }
        break;

    case EventEnum_OnWaitStop:
        if (waitStopEventSink->sinkAdded && eventDataPtrs[EventEnum_OnWaitStop]->isEmpty()) {
            waitStopEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnWaitStop]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(waitStopEventSink);
            logger::debug("EventEnum_OnWaitStop sink removed");
        }
        break;

    case EventEnum_OnMagicEffectApply:
        if (magicEffectApplyEventSink->sinkAdded && eventDataPtrs[EventEnum_OnMagicEffectApply]->isEmpty()) {
            magicEffectApplyEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnMagicEffectApply]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(magicEffectApplyEventSink);
            logger::debug("EventEnum_OnMagicEffectApply sink removed");
        }
        break;

    case EventEnum_OnSpellCast:
        if (spellCastEventSink->sinkAdded && eventDataPtrs[EventEnum_OnSpellCast]->isEmpty()) {
            spellCastEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnSpellCast]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(spellCastEventSink);
            logger::debug("EventEnum_OnSpellCast sink removed");
        }
        break;

    case EventEnum_OnContainerChanged:
        if (containerChangedEventSink->sinkAdded && eventDataPtrs[EventEnum_OnContainerChanged]->isEmpty()) {
            containerChangedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnContainerChanged]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(containerChangedEventSink);
            logger::debug("EventEnum_OnContainerChanged sink removed");
        }
        break;

    case EventEnum_LockChanged:
        if (lockChangedEventSink->sinkAdded && eventDataPtrs[EventEnum_LockChanged]->isEmpty()) {
            lockChangedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_LockChanged]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(lockChangedEventSink);
            logger::debug("EventEnum_LockChanged sink removed");
        }
        break;

    case EventEnum_OnOpen:

    case EventEnum_OnClose:
        if (openCloseEventSink->sinkAdded && eventDataPtrs[EventEnum_OnOpen]->isEmpty() && eventDataPtrs[EventEnum_OnClose]->isEmpty()) {
            openCloseEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnOpen]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(openCloseEventSink);
            logger::debug("EventEnum_OnOpen sink removed");
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
            logger::debug("actorActionEventSink sink removed");
        }
        break;

    case EventEnum_OnProjectileImpact:
        if (RemoveHitEventSink()) {

        }
        if (eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded && eventDataPtrs[EventEnum_OnProjectileImpact]->isEmpty()) {
            eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded = false;
            logger::debug("EventEnum_OnProjectileImpact sink removed");
        }
        break;

    case EventEnum_OnItemCrafted:
        if (itemCraftedEventSink->sinkAdded && eventDataPtrs[EventEnum_OnItemCrafted]->isEmpty()) {
            itemCraftedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnItemCrafted]->sinkAdded = false;
            RE::ItemCrafted::GetEventSource()->RemoveEventSink(itemCraftedEventSink);
            logger::debug("EventEnum_OnItemCrafted sink removed");
        }
        break;

    case EventEnum_OnItemsPickpocketed:
        if (itemsPickpocketedEventSink->sinkAdded && eventDataPtrs[EventEnum_OnItemsPickpocketed]->isEmpty()) {
            itemsPickpocketedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnItemsPickpocketed]->sinkAdded = false;
            RE::ItemsPickpocketed::GetEventSource()->RemoveEventSink(itemsPickpocketedEventSink);
            logger::debug("EventEnum_OnItemsPickpocketed sink removed");
        }
        break;

    case EventEnum_OnLocationCleared:
        if (locationClearedEventSink->sinkAdded && eventDataPtrs[EventEnum_OnLocationCleared]->isEmpty()) {
            locationClearedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnLocationCleared]->sinkAdded = false;
            RE::LocationCleared::GetEventSource()->RemoveEventSink(locationClearedEventSink);
            logger::debug("EventEnum_OnLocationCleared sink removed");
        }
        break;

    case EventEnum_OnEnterBleedout:
        if (enterBleedoutEventSink->sinkAdded && eventDataPtrs[EventEnum_OnEnterBleedout]->isEmpty()) {
            enterBleedoutEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnEnterBleedout]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(enterBleedoutEventSink);
            logger::debug("EventEnum_OnEnterBleedout sink removed");
        }
        break;

    case EventEnum_OnSwitchRaceComplete:
        if (switchRaceCompleteEventSink->sinkAdded && eventDataPtrs[EventEnum_OnSwitchRaceComplete]->isEmpty()) {
            switchRaceCompleteEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnSwitchRaceComplete]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(switchRaceCompleteEventSink);
            eventSourceholder->RemoveEventSink(objectInitEventSink);
            actorRacesSaved = false;
            logger::debug("EventEnum_OnSwitchRaceComplete sink removed");
        }
        break;

    case EventEnum_OnActorFootStep:
        if (footstepEventSink->sinkAdded && eventDataPtrs[EventEnum_OnActorFootStep]->isEmpty()) {
            footstepEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnActorFootStep]->sinkAdded = false;
            RE::BGSFootstepManager::GetSingleton()->RemoveEventSink(footstepEventSink);
            logger::debug("EventEnum_OnActorFootStep sink removed");
        }
        break;

    case EventEnum_OnQuestObjectiveStateChanged:
        if (questObjectiveEventSink->sinkAdded && eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->isEmpty()) {
            questObjectiveEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->sinkAdded = false;
            RE::ObjectiveState::GetEventSource()->RemoveEventSink(questObjectiveEventSink);
            logger::debug("EventEnum_OnQuestObjectiveStateChanged sink removed");
        }
        break;

    case EventEnum_OnPositionPlayerStart:

    case EventEnum_OnPositionPlayerFinish:
        if (positionPlayerEventSink->sinkAdded && !ShouldPositionPlayerEventSinkBeAdded()) {
            auto posPlayerEventSource = player->AsPositionPlayerEventSource();
            if (posPlayerEventSource) {
                positionPlayerEventSink->sinkAdded = false;
                posPlayerEventSource->RemoveEventSink(positionPlayerEventSink);
                logger::debug("positionPlayerEventSink removed");
            }
            else {
                logger::error("posPlayerEventSource not found, positionPlayerEventSink sink not removed");
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
                logger::debug("EventEnum_OnPlayerChangeCell sink removed");
            }
            else {
                logger::error("playerCellChangeSource not found, EventEnum_OnPlayerChangeCell sink not removed");
            }
        }
        break;
    }
}

int GetEventIndex(std::vector<EventData*> v, RE::BSFixedString asEvent) {
    if (asEvent == "") {
        return -1;
    }

    if (v.size() == 0) {
        return -1;
    }

    for (int i = 0; i < v.size(); i++) {
        if (v[i]->sEvent == asEvent) {
            return i;
        }
    }

    return -1;
}

//global events ==========================================================================================================================================================================================

// is registered
bool IsFormRegisteredForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{}", asEvent);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return false;
    }

    logger::trace("getting handle is registered for: {}", gfuncs::GetFormName(eventReceiver));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return false;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        return eventDataPtrs[index]->HasHandle(handle, paramFilter, paramFilterIndex);
    }
    else {
        logger::error("event [{}] not recognized", asEvent);
        return false;
    }
}

bool IsAliasRegisteredForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::BGSBaseAlias* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{}", asEvent);

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return false;
    }

    logger::trace("getting handle is registered for: {}", eventReceiver->aliasName);

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->GetVMTypeID());
        return false;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        return eventDataPtrs[index]->HasHandle(handle, paramFilter, paramFilterIndex);
    }
    else {
        logger::error("event [{}] not recognized", asEvent);
        return false;
    }
}

bool IsActiveMagicEffectRegisteredForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::ActiveEffect* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{}", asEvent);

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return false;
    }

    logger::trace("getting handle is registered for: {} instance", gfuncs::GetFormName(eventReceiver->GetBaseObject()));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for effect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->VMTYPEID);
        return false;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        return eventDataPtrs[index]->HasHandle(handle, paramFilter, paramFilterIndex);
    }
    else {
        logger::error("event [{}] not recognized", asEvent);
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

    logger::trace("size[{}] scriptAddedFormCount[{}] iCount[{}]", size, list->scriptAddedFormCount, iCount);

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

    logger::trace("size[{}] scriptAddedFormCount[{}]", size, list->scriptAddedFormCount);

    return true;
}

//register
void RegisterFormForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{}", asEvent);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return;
    }

    logger::trace("adding handle for: {}", gfuncs::GetFormName(eventReceiver));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
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
                    logger::warn("passed in paramFilter formlist[{}] for event [{}] has no forms to register", gfuncs::GetFormName(akFormList), asEvent);
                }
            }
        }

        if (!paramFilterIsFormList) {
            eventDataPtrs[index]->AddHandle(handle, paramFilter, paramFilterIndex);
            AddSink(index);
        }
    }
    else {
        logger::error("event [{}] not recognized", asEvent);
    }
}

void RegisterAliasForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::BGSBaseAlias* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{}", asEvent);

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    logger::trace("adding handle for: {}", eventReceiver->aliasName);

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->GetVMTypeID());
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
                    logger::warn("passed in paramFilter formlist[{}] for event [{}] has no forms to register", gfuncs::GetFormName(akFormList), asEvent);
                }
            }
        }

        if (!paramFilterIsFormList) {
            eventDataPtrs[index]->AddHandle(handle, paramFilter, paramFilterIndex);
            AddSink(index);
        }
    }
    else {
        logger::error("event [{}] not recognized", asEvent);
    }
}

void RegisterActiveMagicEffectForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::ActiveEffect* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{}", asEvent);

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    logger::trace("adding handle for: {} instance", gfuncs::GetFormName(eventReceiver->GetBaseObject()));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for effect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->VMTYPEID);
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
                    logger::warn("passed in paramFilter formlist[{}] for event [{}] has no forms to register", gfuncs::GetFormName(akFormList), asEvent);
                }
            }
        }

        if (!paramFilterIsFormList) {
            eventDataPtrs[index]->AddHandle(handle, paramFilter, paramFilterIndex);
            AddSink(index);
        }
    }
    else {
        logger::error("event [{}] not recognized", asEvent);
    }
}

//unregister
void UnregisterFormForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{}", asEvent);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return;
    }

    logger::trace("removing handle for: {}", gfuncs::GetFormName(eventReceiver));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
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
                    logger::warn("passed in paramFilter formlist[{}] for event [{}] has no forms to unregister", gfuncs::GetFormName(akFormList), asEvent);
                }
            }
        }

        if (!paramFilterIsFormList) {
            eventDataPtrs[index]->RemoveHandle(handle, paramFilter, paramFilterIndex);
        }
    }
    else {
        logger::error("event [{}] not recognized", asEvent);
    }
}

void UnregisterAliasForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::BGSBaseAlias* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{}", asEvent);

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    logger::trace("removing handle for: {}", eventReceiver->aliasName);

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->GetVMTypeID());
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
                    logger::warn("passed in paramFilter formlist[{}] for event [{}] has no forms to unregister", gfuncs::GetFormName(akFormList), asEvent);
                }
            }
        }

        if (!paramFilterIsFormList) {
            eventDataPtrs[index]->RemoveHandle(handle, paramFilter, paramFilterIndex);
        }
    }
    else {
        logger::error("event [{}] not recognized", asEvent);
    }
}

void UnregisterActiveMagicEffectForGlobalEvent(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::ActiveEffect* eventReceiver, RE::TESForm* paramFilter, int paramFilterIndex) {
    logger::trace("{}", asEvent);

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    logger::trace("removing handle for: {} instance", gfuncs::GetFormName(eventReceiver->GetBaseObject()));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for effect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->VMTYPEID);
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
                    logger::warn("passed in paramFilter formlist[{}] for event [{}] has no forms to unregister", gfuncs::GetFormName(akFormList), asEvent);
                }
            }
        }

        if (!paramFilterIsFormList) {
            eventDataPtrs[index]->RemoveHandle(handle, paramFilter, paramFilterIndex);
        }
    }
    else {
        logger::error("event [{}] not recognized", asEvent);
    }
}

//unregister all
void UnregisterFormForGlobalEvent_All(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::TESForm* eventReceiver) {
    logger::trace("{}", asEvent);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return;
    }

    logger::trace("removing handle for: {}", gfuncs::GetFormName(eventReceiver));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->RemoveAllHandles(handle);
    }
    else {
        logger::error("event [{}] not recognized", asEvent);
    }
}

void UnregisterAliasForGlobalEvent_All(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::BGSBaseAlias* eventReceiver) {
    logger::trace("{}", asEvent);

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    logger::trace("removing all handles for: {}", eventReceiver->aliasName);

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->GetVMTypeID());
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->RemoveAllHandles(handle);
    }
    else {
        logger::error("event [{}] not recognized", asEvent);
    }
}

void UnregisterActiveMagicEffectForGlobalEvent_All(RE::StaticFunctionTag*, RE::BSFixedString asEvent, RE::ActiveEffect* eventReceiver) {
    logger::trace("{}", asEvent);

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    logger::trace("removing all handles for: {} instance", gfuncs::GetFormName(eventReceiver->GetBaseObject()));

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for effect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->VMTYPEID);
        return;
    }

    int index = GetEventIndex(eventDataPtrs, asEvent);
    if (index != -1) {
        eventDataPtrs[index]->RemoveAllHandles(handle);
    }
    else {
        logger::error("event [{}] not recognized", asEvent);
    }
}

// plugin load / maintenance==================================================================================================================================================

void CreateEventSinks() {
    if (!player) { player = RE::PlayerCharacter::GetSingleton(); }
    if (!eventSourceholder) { eventSourceholder = RE::ScriptEventSourceHolder::GetSingleton(); }
    if (!xMarker) { xMarker = RE::TESForm::LookupByID(59); }

    if (xMarker) {
        logger::trace("xmarker [{}] found", gfuncs::GetFormName(xMarker));
    }
    else {
        logger::warn("xmarker form not found");
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
                logger::info("ProjectileImpactHook installed successfully");
            }
            else {
                logger::warn("ProjectileImpactHook not installed");
            }
        }
        else {
            logger::critical("skseLoadInterface not found, aborting ProjectileImpactHook install");
        }
    }
    else {
        logger::info("iMaxArrowsSavedPerReference is [{}], aborting ProjectileImpactHook install", iMaxArrowsSavedPerReference);
    }

    UIEvents::Install();

    logger::trace("Event Sinks Created");
}

bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    // vm->RegisterFunction("MyNativeFunction", "DbSkseFunctions", MyNativeFunction);
    logger::trace("Binding Papyrus Functions");

    gvm = vm;
    ui = RE::UI::GetSingleton();
    calendar = RE::Calendar::GetSingleton();
    svm = RE::SkyrimVM::GetSingleton();
    if (!playerRef) { playerRef = RE::TESForm::LookupByID<RE::TESForm>(20)->As<RE::Actor>(); }

    //functions 
    vm->RegisterFunction("SaveProjectileForAmmo", "DbSkseCppCallbackEvents", SaveProjectileForAmmo);
    vm->RegisterFunction("SetDbSkseCppCallbackEventsAttached", "DbSkseCppCallbackEvents", SetDbSkseCppCallbackEventsAttached);

    vm->RegisterFunction("GetVersion", "DbSkseFunctions", GetThisVersion);

    vm->RegisterFunction("GetFormHandle", "PapyrusUtilEx", GetFormHandle);
    vm->RegisterFunction("GetAliasHandle", "PapyrusUtilEx", GetAliasHandle);
    vm->RegisterFunction("GetActiveEffectHandle", "PapyrusUtilEx", GetActiveEffectHandle);
    vm->RegisterFunction("ResizeArray", "PapyrusUtilEx", ResizeArrayProperty);
    vm->RegisterFunction("RemoveFromArray", "PapyrusUtilEx", RemoveFromArray);
    vm->RegisterFunction("SliceArray", "PapyrusUtilEx", SliceArray);
    vm->RegisterFunction("SliceArrayOnto", "PapyrusUtilEx", SliceArrayOnto);
    vm->RegisterFunction("CountInArray", "PapyrusUtilEx", CountInArray);
    vm->RegisterFunction("MergeArrays", "PapyrusUtilEx", MergeArrays);
    vm->RegisterFunction("CopyArray", "PapyrusUtilEx", CopyArray);

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
    vm->RegisterFunction("GetAllInteriorCells", "DbSkseFunctions", GetAllInteriorCells);
    vm->RegisterFunction("GetAllExteriorCells", "DbSkseFunctions", GetAllExteriorCells);
    vm->RegisterFunction("GetAttachedCells", "DbSkseFunctions", GetAttachedCells);
    vm->RegisterFunction("GetAllFormsWithName", "DbSkseFunctions", GetAllFormsWithName);
    vm->RegisterFunction("GetAllFormsWithScriptAttached", "DbSkseFunctions", GetAllFormsWithScriptAttached);
    vm->RegisterFunction("GetAllAliasesWithScriptAttached", "DbSkseFunctions", GetAllAliasesWithScriptAttached);
    vm->RegisterFunction("GetAllRefAliasesWithScriptAttached", "DbSkseFunctions", GetAllRefAliasesWithScriptAttached);
    vm->RegisterFunction("GetAllRefaliases", "DbSkseFunctions", GetAllRefaliases);
    vm->RegisterFunction("GetAllRefAliasesForRef", "DbSkseFunctions", GetAllRefAliasesForRef);
    vm->RegisterFunction("GetAllQuestObjectRefs", "DbSkseFunctions", GetAllQuestObjectRefs);
    vm->RegisterFunction("SetAliasQuestObjectFlag", "DbSkseFunctions", SetAliasQuestObjectFlag);
    vm->RegisterFunction("IsAliasQuestObjectFlagSet", "DbSkseFunctions", IsAliasQuestObjectFlagSet);
    vm->RegisterFunction("GetQuestObjectRefsInContainer", "DbSkseFunctions", GetQuestObjectRefsInContainer);
    vm->RegisterFunction("GetAllObjectRefsInContainer", "DbSkseFunctions", GetAllObjectRefsInContainer);
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
    vm->RegisterFunction("RefreshItemMenu", "DbSkseFunctions", RefreshItemMenu);
    vm->RegisterFunction("IsItemMenuOpen", "DbSkseFunctions", IsItemMenuOpenNative);
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
    vm->RegisterFunction("SetArtObjectNthTextureSet", "DbSkseFunctions", SetArtObjectNthTextureSet);
    vm->RegisterFunction("GetArtObjectNthTextureSet", "DbSkseFunctions", GetArtObjectNthTextureSet);
    vm->RegisterFunction("GetArtObjectModelNth3dName", "DbSkseFunctions", GetArtObjectModelNth3dName);
    vm->RegisterFunction("GetArtObjectNumOfTextureSets", "DbSkseFunctions", GetArtObjectNumOfTextureSets);

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

    //case SKSE::MessagingInterface::kInputLoaded:
        //logger::info("kInputLoaded: sent right after game input is loaded, right before the main menu initializes");

        //break;

    case SKSE::MessagingInterface::kNewGame:
        //logger::trace("kNewGame: sent after a new game is created, before the game has loaded");

        DbSkseCppCallbackEventsAttached = false;
        DbSkseCppCallbackLoad(); //attach papyrus DbSkseCppCallbackEvents script to the player and save all ammo projectiles
        RegisterActorsForBowDrawAnimEvents();
        //logger::trace("kNewGame: sent after a new game is created, before the game has loaded");
        break;

    case SKSE::MessagingInterface::kDataLoaded:
        // RE::ConsoleLog::GetSingleton()->Print("DbSkseFunctions Installed");
        if (!nullForm) { nullForm = gfuncs::FindNullForm(); }
        gfuncs::Install();

        auto* papyrusInterface = SKSE::GetPapyrusInterface();
        papyrusInterface->Register(BindPapyrusFunctions);
        papyrusInterface->Register(timers::BindPapyrusFunctions);
        papyrusInterface->Register(gfx::BindPapyrusFunctions);
        papyrusInterface->Register(cell::BindPapyrusFunctions);

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

    if (a_intfc) {
        if (!bIsLoadingSerialization && !bIsSavingSerialization) {
            bIsLoadingSerialization = true;
            while (a_intfc->GetNextRecordInfo(type, version, length)) {
                if (timers::IsTimerType(type)) {
                    timers::LoadTimers(type, a_intfc);
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
            //gfuncs::SendEvents(eventDataPtrs[EventEnum_OnLoadGame]->globalHandles, eventDataPtrs[EventEnum_OnLoadGame]->sEvent, args);

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
        else {
            logger::debug("already loading or saving. loading = {} saving = {}, aborting load.", bIsLoadingSerialization, bIsSavingSerialization);
        }
    }
    else {
        logger::error("a_intfc doesn't exist, aborting load.");
    }
}

void SaveCallback(SKSE::SerializationInterface* a_intfc) {
    logger::trace("SaveCallback started");

    if (a_intfc) {
        if (!bIsLoadingSerialization && !bIsSavingSerialization) {
            bIsSavingSerialization = true;
            //this is causing ctd on Skyrim AE when fast traveling too many times too quickly.
            /*int max = EventEnum_Last + 1;
            for (int i = EventEnum_First; i < max; i++) {
                eventDataPtrs[i]->Save(a_intfc);
            }*/

            timers::SaveTimers(a_intfc);

            bIsSavingSerialization = false;
            logger::trace("SaveCallback complete");
        }
        else {
            logger::debug("already loading or saving. loading = {} saving = {}, aborting load.", bIsLoadingSerialization, bIsSavingSerialization);
        }
    }
    else {
        logger::error("a_intfc doesn't exist, aborting load.");
    }
}

//init================================================================================================================================================================
SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SetupLog("Data/SKSE/Plugins/DbSkseFunctions.ini");

    skseLoadInterface = skse;

    SKSE::GetMessagingInterface()->RegisterListener(MessageListener);

    auto* serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID('DbSF');
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);

    //replaced gfuncs::LogAndMessage with logger:: functions.
    //fs::ReplaceLogAndMessageFuncs();

    return true;
}