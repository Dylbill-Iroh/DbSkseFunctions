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
#include <condition_variable>
#include <thread>
#include <future>
#include <typeinfo>
#include <any>
#include <xbyak/xbyak.h>
#include "mini/ini.h"
#include "logger.h"
#include "GeneralFunctions.h"
#include "Keyword.h"
#include "KeyInput.h"
#include "Magic.h"
#include "Utility.h"
#include "MapMarker.h"
#include "ObjectReference.h"
#include "Actor.h"
#include "Alias.h"
#include "Animation.h"
#include "ArtObject.h"
#include "Book.h"
#include "Cell.h"
#include "ConditionFunctions.h"
#include "ConsoleUtil.h"
#include "CreateForms.h"
#include "EventDefs.h"
#include "FormVectorGetters.h"
#include "Furniture.h"
#include "PapyrusUtilEx.h"
#include "ProjectileFunctions.h"
#include "RangeEvents.h"
#include "BipedSlots.h"
#include "UIEventHooks/Hooks.h"
#include "editorID.hpp"
#include "STLThunk.h"
#include "Sound.h"
#include "Serialization.h"
#include "Timers.h"
#include "mINIHelper.h"
#include "FileSystem.h"
#include "UIGfx.h"

bool bPlayerIsInCombat = false;
bool bRegisteredForPlayerCombatChange = false;

//bool inMenuMode = false; //in utility.h
//tst::TSWrapper<bool> gamePaused = false;
//std::string lastMenuOpened; //in utility.h
//tst::TSWrapper<RE::TESObjectREFR*> lastPlayerActivatedRef = nullptr;
//tst::TSWrapper<RE::TESObjectREFR*> menuRef = nullptr;

//thread safe vector
//tst::TSvector<std::string> openedMenus;


//std::mutex openedMenusMutex; //in utility.h

//using openedMenus vector with mutex lock instead
//int numOfMenusCurrentOpen = 0;
//std::unordered_map<std::string, bool> menuStatusMap = {
//    {std::string(RE::BarterMenu::MENU_NAME), false},
//    {std::string(RE::BookMenu::MENU_NAME), false},
//    {std::string(RE::Console::MENU_NAME), false},
//    {std::string(RE::ConsoleNativeUIMenu::MENU_NAME), false},
//    {std::string(RE::ContainerMenu::MENU_NAME), false},
//    {std::string(RE::CraftingMenu::MENU_NAME), false},
//    {std::string(RE::CreationClubMenu::MENU_NAME), false},
//    {std::string(RE::CreditsMenu::MENU_NAME), false},
//    {std::string(RE::DialogueMenu::MENU_NAME), false},
//    {std::string(RE::FaderMenu::MENU_NAME), false},
//    {std::string(RE::FavoritesMenu::MENU_NAME), false},
//    {std::string(RE::GiftMenu::MENU_NAME), false},
//    {std::string(RE::InventoryMenu::MENU_NAME), false},
//    {std::string(RE::JournalMenu::MENU_NAME), false},
//    {std::string(RE::KinectMenu::MENU_NAME), false},
//    {std::string(RE::LevelUpMenu::MENU_NAME), false},
//    {std::string(RE::LoadingMenu::MENU_NAME), false},
//    {std::string(RE::LockpickingMenu::MENU_NAME), false},
//    {std::string(RE::MagicMenu::MENU_NAME), false},
//    {std::string(RE::MainMenu::MENU_NAME), false},
//    {std::string(RE::MapMenu::MENU_NAME), false},
//    {std::string(RE::MessageBoxMenu::MENU_NAME), false},
//    {std::string(RE::MistMenu::MENU_NAME), false},
//    {std::string(RE::ModManagerMenu::MENU_NAME), false},
//    {std::string(RE::RaceSexMenu::MENU_NAME), false},
//    {std::string(RE::SafeZoneMenu::MENU_NAME), false},
//    {std::string(RE::SleepWaitMenu::MENU_NAME), false},
//    {std::string(RE::StatsMenu::MENU_NAME), false},
//    {std::string(RE::TitleSequenceMenu::MENU_NAME), false},
//    {std::string(RE::TrainingMenu::MENU_NAME), false},
//    {std::string(RE::TutorialMenu::MENU_NAME), false},
//    {std::string(RE::TweenMenu::MENU_NAME), false}
//};

std::vector<RE::BSFixedString> refActivatedMenus = {
    RE::DialogueMenu::MENU_NAME,
    RE::BarterMenu::MENU_NAME,
    RE::GiftMenu::MENU_NAME,
    RE::LockpickingMenu::MENU_NAME,
    RE::ContainerMenu::MENU_NAME,
    RE::BookMenu::MENU_NAME,
    RE::CraftingMenu::MENU_NAME
};

//int numOfItemMenusCurrentOpen = 0;
std::vector<RE::BSFixedString> itemMenus = {
    RE::InventoryMenu::MENU_NAME,
    RE::BarterMenu::MENU_NAME,
    RE::ContainerMenu::MENU_NAME,
    RE::GiftMenu::MENU_NAME,
    RE::MagicMenu::MENU_NAME,
    RE::CraftingMenu::MENU_NAME,
    RE::FavoritesMenu::MENU_NAME
};

bool bActivateEventSinkEnabledByDefault = true; 
bool bMenuOpenCloseEventSinkEnabled = true;
int iMaxArrowsSavedPerReference = 0;
RE::TESForm* nullForm;
RE::TESForm* xMarker;
RE::BSScript::IVirtualMachine* gvm;
RE::NiPoint3 zeroPosition{0.0, 0.0, 0.0};
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

    //eventPollingInterval

    gameTimerPollingInterval = (mINI::GetIniFloat(ini, "Main", "gameTimerPollingInterval", 1.5) * 1000);
    if (gameTimerPollingInterval < 100) {
        gameTimerPollingInterval = 100;
    }

    eventPollingInterval = (mINI::GetIniFloat(ini, "Main", "fEventPollingInterval", 0.5) * 1000);
    if (eventPollingInterval < 50) {
        eventPollingInterval = 50;
    }

    iMaxArrowsSavedPerReference = mINI::GetIniInt(ini, "Main", "iMaxArrowsSavedPerReference", 0);
    if (iMaxArrowsSavedPerReference < 0) {
        iMaxArrowsSavedPerReference = 0;
    }

    bActivateEventSinkEnabledByDefault = mINI::GetIniBool(ini, "Main", "bActivateEventSinkEnabledByDefault", true);
    bMenuOpenCloseEventSinkEnabled = mINI::GetIniBool(ini, "Main", "bMenuOpenCloseEventSinkEnabled", true);

    logger::info("gameTimerPollingInterval set to {} milliseconds", gameTimerPollingInterval);
    logger::info("eventPollingInterval set to {} milliseconds", eventPollingInterval);
    logger::info("iMaxArrowsSavedPerReference set to {} ", iMaxArrowsSavedPerReference);
    logger::info("bActivateEventSinkEnabledByDefault set to {} ", bActivateEventSinkEnabledByDefault);
    logger::info("bMenuOpenCloseEventSinkEnabled set to {} ", bMenuOpenCloseEventSinkEnabled);
}

enum logLevel { trace, debug, info, warn, error, critical };
enum debugLevel { notification, messageBox };

//papyrus functions=============================================================================================================================
float GetThisVersion(RE::BSScript::Internal::VirtualMachine* vm, const RE::VMStackID stackID, RE::StaticFunctionTag* functionTag) {
    return float(10.0);
}

void AttachDbSksePersistentVariablesScript() {
    //std::lock_guard<std::recursive_mutex> lock(openedMenusMutex); 
    auto* player = RE::PlayerCharacter::GetSingleton();
    if (player) {
        if (!gfuncs::IsScriptAttachedToRef(player, "DbSksePersistentVariables")) {
            logger::info("Attaching DbSksePersistentVariables script to player.");
            ConsoleUtil::ExecuteCommand("player.aps DbSksePersistentVariables", nullptr);
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); //wait .1 seconds for console command to finish.
        }
    }
}

RE::TESObjectREFR* GetLastPlayerActivatedRef(RE::StaticFunctionTag*) {
    //std::lock_guard<std::recursive_mutex> lock(openedMenusMutex); 
    
    //gets the TESObjectREFR in a thread safe way.
    RE::TESObjectREFR* returnRef = nullptr;
    sharedVars.read([&](const SharedUIVariables& vars) {
        returnRef = vars.lastPlayerActivatedRef;
    });
    
    if (!gfuncs::IsFormValid(returnRef)) {
        returnRef = nullptr;
    }

    return returnRef;
}

RE::TESObjectREFR* GetLastPlayerMenuActivatedRef(RE::StaticFunctionTag*) {
    //std::lock_guard<std::recursive_mutex> lock(openedMenusMutex); 

    //gets the TESObjectREFR in a thread safe way.
    RE::TESObjectREFR* returnRef = nullptr;
    sharedVars.read([&](const SharedUIVariables& vars) {
        returnRef = vars.menuRef;
    });

    if (!gfuncs::IsFormValid(returnRef)) {
        returnRef = nullptr;
    }
    
    //openedMenusMutex.unlock();
    return returnRef;
}

void ExecuteConsoleCommand(RE::StaticFunctionTag*, std::string a_command, RE::TESObjectREFR* objRef) {
    logger::trace("called. Command = {}", a_command);
    ConsoleUtil::ExecuteCommand(a_command, objRef);
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
    EventEnum_OnEffectStart,
    EventEnum_OnEffectFinish,
    EventEnum_OnMusicTypeChange,
    EventEnum_OnWeatherChange,
    EventEnum_OnPerkEntryRun,
    EventEnum_OnTriggerEnter,
    EventEnum_OnTriggerLeave,
    EventEnum_OnPackageStart,
    EventEnum_OnPackageChange,
    EventEnum_OnPackageEnd,
    EventEnum_OnDestructionStageChanged,
    EventEnum_OnTranslationFailed,
    EventEnum_OnTranslationAlmostComplete,
    EventEnum_OnTranslationComplete,
    EventEnum_First = EventEnum_OnLoadGame,
    EventEnum_Last = EventEnum_OnTranslationComplete
};

struct EventData {
    int eventSinkIndex;
    bool sinkAdded = false;
    RE::BSFixedString sEvent;
    std::uint32_t record;

    std::vector<RE::VMHandle> globalHandles; //handles that receive all events
    std::vector< std::map<RE::TESForm*, std::vector<RE::VMHandle>> > eventParamMaps; //event param form comparisons

    //constructor
    EventData(RE::BSFixedString event, int ceventSinkIndex, int NumberOfFormParams, std::uint32_t crecord) :
        eventSinkIndex(ceventSinkIndex),
        sEvent(event),
        record(crecord) {
        eventParamMaps.resize(NumberOfFormParams);
    }

    bool PlayerIsRegistered() {
        RE::Actor* playerRef = static_cast<RE::Actor*>(RE::PlayerCharacter::GetSingleton());
        if (!playerRef) {
            logger::error("playerRef not found");
            return false;
        }

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

    void AddHandle(RE::VMHandle handle, RE::TESForm* paramFilter, int paramFilterIndex, std::string eventReceiverName, RE::FormID eventReceiverId) {
        if (!gfuncs::IsFormValid(paramFilter)) {
            globalHandles.push_back(handle);
            logger::debug("registering [{}] ID[{:x}] for all [{}] events", eventReceiverName, eventReceiverId, sEvent);

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
            logger::debug("registering [{}] ID[{:x}] paramFilter[{}] paramFilterIndex[{}] sEvent[{}]", 
                eventReceiverName, eventReceiverId, gfuncs::GetFormName(paramFilter), paramFilterIndex, sEvent);

            if (eventSinkIndex == EventEnum_OnCombatStateChanged && paramFilterIndex == 0) {
                logger::debug("adding handle for Combat State change");
                RE::Actor* playerRef = static_cast<RE::Actor*>(RE::PlayerCharacter::GetSingleton());
                if (playerRef) {
                    if (paramFilter->As<RE::Actor>() == playerRef) {
                        //playerForm = paramFilter;
                        bRegisteredForPlayerCombatChange = true;
                        logger::debug("bRegisteredForPlayerCombatChange = true");
                    }
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

    void CombineEventHandles(std::vector<RE::VMHandle>& handles, RE::TESForm* akForm, int index) {
        if (index >= eventParamMaps.size()) {
            return;
        }

        if (!gfuncs::IsFormValid(akForm)) {
            return;
        }

        auto it = eventParamMaps[index].find(akForm);

        if (it != eventParamMaps[index].end()) {
            logger::trace("events for form: [{}] ID[{:x}] found", gfuncs::GetFormName(akForm), akForm->GetFormID());
            handles.reserve(handles.size() + it->second.size());
            handles.insert(handles.end(), it->second.begin(), it->second.end());
        }
        else {
            logger::trace("events for form: [{}] ID[{:x}] not found", gfuncs::GetFormName(akForm), akForm->GetFormID());
        }
    }

    std::vector<RE::VMHandle> GetHandles(std::vector<RE::TESForm*> formParams) {
        std::vector<RE::VMHandle> eventHandles = globalHandles;

        auto size = formParams.size(); 
        if (size > eventParamMaps.size()) {
            size = eventParamMaps.size();
        }

        for (int i = 0; i < size; i++) {
            CombineEventHandles(eventHandles, formParams[i], i);
            if (formParams[i]) {
                auto* ref = skyrim_cast<RE::TESObjectREFR*>(formParams[i]);
                if (gfuncs::IsFormValid(ref)) {
                    CombineEventHandles(eventHandles, ref->GetBaseObject(), i);
                }
            }
        }

        gfuncs::RemoveDuplicates(eventHandles);
        return eventHandles;
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
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (event->tag == "BowRelease") {
            auto* calendar = RE::Calendar::GetSingleton();
            if (calendar) {
                lastReleaseTime = std::chrono::system_clock::now();
                lastReleaseGameTime = calendar->GetHoursPassed();
            }
            else {
                logger::error("calendar not found");
            }
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
    static bool thunk(RE::Projectile* projectile) {
        if ((projectile)) {
            bool killOnCollision = projectile->GetKillOnCollision();

            auto* calendar = RE::Calendar::GetSingleton();
            float gameHoursPassed = 0.0;
            if (calendar) {
                gameHoursPassed = calendar->GetHoursPassed();
            }
            else {
                logger::error("calendar not found");
            }

            //uint32_t runTime = RE::GetDurationOfApplicationRunTime();
            //auto now = std::chrono::system_clock::now();

            //return projectile->Unk_B8();

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

    std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnProjectileImpact]->GetHandles({ data.shooter, data.target,
        source, data.ammo, data.projectileBase });

    if (handles.size() > 0) {

        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)data.shooter, (RE::TESObjectREFR*)data.target, (RE::TESForm*)source,
            (RE::TESAmmo*)data.ammo, (RE::BGSProjectile*)data.projectileBase, (bool)SneakAttack, (bool)HitBlocked,
            (int)data.impactResult, (int)data.collidedLayer, (float)data.distanceTraveled, (std::string)data.hitPartNodeName,
            (RE::TESObjectREFR*)data.projectileMarker, (std::vector<float>)hitTranslation);

        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnProjectileImpact]->sEvent, args);

        logger::trace("shooter[{}] target[{}] source[{}] ammo[{}] projectile[{}]",
            gfuncs::GetFormName(data.shooter), gfuncs::GetFormName(data.target), gfuncs::GetFormName(source),
            gfuncs::GetFormName(data.ammo), gfuncs::GetFormName(data.projectileBase));

        logger::trace("sneak[{}] blocked[{}] impactResult[{}] collidedLayer[{}] distanceTraveled[{}] hitPartNodeName[{}]",
            SneakAttack, HitBlocked, data.impactResult, data.collidedLayer, data.distanceTraveled, data.hitPartNodeName);

        logger::trace("hitTranslation posX[{}] posY[{}] posZ[{}] directionX[{}] directionY[{}] directionZ[{}]",
            hitTranslation[0], hitTranslation[1], hitTranslation[2], hitTranslation[3], hitTranslation[4], hitTranslation[5]);
    }
}

TrackedProjectileData GetRecentTrackedProjectileData(RE::TESObjectREFR* shooter, RE::TESObjectREFR* target, float hitGameTime) {
    TrackedProjectileData nullData;

    if (gfuncs::IsFormValid(shooter) && gfuncs::IsFormValid(target) && hitGameTime > -1.0) {
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
            logger::trace("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        std::chrono::system_clock::time_point hitTime = std::chrono::system_clock::now();
        auto* calendar = RE::Calendar::GetSingleton();
        float currentGameTime = -1.0;
        if (calendar) {
            currentGameTime = calendar->GetHoursPassed();
        }
        else {
            logger::error("calendar not found");
        }

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
                logger::trace("no target");
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

            if (bowEquipped && iMaxArrowsSavedPerReference <= 0) {
                if (gfuncs::IsFormValid(attacker) && gfuncs::IsFormValid(target)) {
                    RE::Actor* actor = attacker->As<RE::Actor>();
                    if (gfuncs::IsFormValid(actor)) {
                        ammo = actor->GetCurrentAmmo();
                        auto it = animationEventActorsMap.find(attacker);
                        if (it != animationEventActorsMap.end()) { //akactor found in animationEventActorsMap
                            if (it->second) {
                                float hitTimeDiff = gfuncs::timePointDiffToFloat(hitTime, it->second->lastHitTime);
                                it->second->lastHitTime = hitTime;
                                logger::trace("found ammo tracking data for [{}] hitTimeDiff = [{}]", gfuncs::GetFormName(attacker), hitTimeDiff);

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
                                        float gameHoursPassed = -1.0;
                                        if (calendar) {
                                            gameHoursPassed = calendar->GetHoursPassed();
                                        }
                                        bool ammoFound = false;

                                        if (iMaxArrowsSavedPerReference > 0 && calendar) {
                                            auto recentHitIt = recentShotProjectiles.find(attacker);
                                            if (recentHitIt != recentShotProjectiles.end()) {
                                                for (int i = 0; i < it->second->forceChangedAmmos.size() && !ammoFound; i++) { //cycle through force unequipped ammos due to shooting last arrow of type. 
                                                    for (int ii = recentHitIt->second.size() - 1; ii >= 0 && !ammoFound; --ii) { //cycle through projectiles that recently hit the target to find matching ammo
                                                        float timeDiff = GameHoursToRealTimeSeconds(nullptr, (gameHoursPassed - recentHitIt->second[ii].gameTimeStamp));
                                                        //logger::trace("recentHitProjectile[{}] timeDiff = [{}]", gfuncs::GetFormName(recentHitIt->second[ii].projectile), timeDiff);
                                                        if (timeDiff < 0.1) {
                                                            if (DidProjectileHitRefWithAmmoFromShooter(attacker, target, it->second->forceChangedAmmos[i].ammo, recentHitIt->second[ii])) {
                                                                auto trackedProjectileData = recentHitIt->second[ii];

                                                                logger::trace("attacker[{}] target[{}] ammo changed from [{}] to [{}]. Recent projectile found[{}]. TimeDiff = [{}]",
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

                                        if (!ammoFound && calendar) { //ammo not found in previous search
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
            if (!gfuncs::IsFormValid(ammo)) {
                ammo = nullptr;
            }

            std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_HitEvent]->GetHandles({ attacker, target, source, ammo, projectileForm });

            if (handles.size() > 0) {
                logger::trace("attacker[{}]  target[{}]  source[{}]  ammo[{}]  projectile[{}]", gfuncs::GetFormName(attacker), gfuncs::GetFormName(target), gfuncs::GetFormName(source), gfuncs::GetFormName(ammo), gfuncs::GetFormName(projectileForm));
                logger::trace("powerAttack[{}]  SneakAttack[{}]  BashAttack[{}]  HitBlocked[{}]", powerAttack, SneakAttack, bBashAttack, HitBlocked);

                auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)attacker, (RE::TESObjectREFR*)target, (RE::TESForm*)source,
                    (RE::TESAmmo*)ammo, (RE::BGSProjectile*)projectileForm, (bool)powerAttack, (bool)SneakAttack, (bool)bBashAttack, (bool)HitBlocked);

                gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_HitEvent]->sEvent, args);
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

void CheckForPlayerCombatStatusChange() {
    logger::trace("");
    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        logger::error("player* not found");
        return;
    }

    RE::Actor* playerRef = static_cast<RE::Actor*>(player);
    if (!playerRef) {
        logger::error("playerRef* not found");
        return;
    }

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

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnCombatStateChanged]->GetHandles({ playerRef, target, });

        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::Actor*)playerRef, (RE::Actor*)target, (int)combatState);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnCombatStateChanged]->sEvent, args);
        }
    }
}

struct CombatEventSink : public RE::BSTEventSink<RE::TESCombatEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* event, RE::BSTEventSource<RE::TESCombatEvent>*/*source*/) {

        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
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

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnCombatStateChanged]->GetHandles(
            { actorObjRef, target });

        if (handles.size() > 0) {

            auto* args = RE::MakeFunctionArguments((RE::Actor*)actorObjRef, (RE::Actor*)target, (int)combatState);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnCombatStateChanged]->sEvent, args);

            if (bRegisteredForPlayerCombatChange) {
                gfuncs::DelayedFunction(&CheckForPlayerCombatStatusChange, 1200); //check for player combat status change after 1.2 seconds.
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

struct FurnitureEventSink : public RE::BSTEventSink<RE::TESFurnitureEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESFurnitureEvent* event, RE::BSTEventSource<RE::TESFurnitureEvent>*/*source*/) {

        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
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
        
        std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->GetHandles(
            { actorObjRef, furnitureRef });

        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::Actor*)actorRef, (RE::TESObjectREFR*)furnitureRef);
            gfuncs::SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

void HandleActivateEvent(RE::TESObjectREFRPtr actionRef, RE::TESObjectREFRPtr objectActivated) {
    RE::TESObjectREFR* activatorRef = nullptr;
    if (actionRef) {
        activatorRef = actionRef.get();
        if (!gfuncs::IsFormValid(activatorRef)) {
            activatorRef = nullptr;
        }
    }

    RE::TESObjectREFR* activatedRef = nullptr;
    if (objectActivated) {
        activatedRef = objectActivated.get();
        if (!gfuncs::IsFormValid(activatedRef, true)) {
            activatedRef = nullptr;
        }
    }

    //RE::Actor* playerRef = static_cast<RE::Actor*>(RE::PlayerCharacter::GetSingleton());

    if (bActivateEventSinkEnabledByDefault) {
        RE::Actor* playerRef = static_cast<RE::Actor*>(RE::PlayerCharacter::GetSingleton());

        if (playerRef) {
            if (activatorRef == playerRef) {
                if (activatedRef) {
                    sharedVars.update([activatedRef](SharedUIVariables& vars) {
                        vars.lastPlayerActivatedRef = activatedRef;
                    });

                    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
                    if (vm) {
                        AttachDbSksePersistentVariablesScript();
                        auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)activatedRef);
                        vm->SendEventAll("OnDbSksePlayerActivatedRef", args);
                        delete args;
                    }
                }
            }
        }
    }

    if (eventDataPtrs[EventEnum_OnActivate]->sinkAdded) {
        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnActivate]->GetHandles({ activatorRef, activatedRef });
        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)activatorRef, (RE::TESObjectREFR*)activatedRef);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnActivate]->sEvent, args);
        }
    }
}

struct ActivateEventSink : public RE::BSTEventSink<RE::TESActivateEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* event, RE::BSTEventSource<RE::TESActivateEvent>*/*source*/) {
        //std::lock_guard<std::recursive_mutex> lock(openedMenusMutex); 

        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        //HandleActivateEvent(event->actionRef, event->objectActivated);

        std::thread t(HandleActivateEvent, event->actionRef, event->objectActivated);
        t.detach();

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct DeathEventSink : public RE::BSTEventSink<RE::TESDeathEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* event, RE::BSTEventSource<RE::TESDeathEvent>* source) {

        if (!event) {
            logger::warn("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

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
                logger::warn("victim doesn't exist or isn't valid");
                return RE::BSEventNotifyControl::kContinue;
            }
            logger::trace("valid victimRef pointer");
        }
        else {
            logger::warn("0 victimRef doesn't exist or isn't valid");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::Actor* killer = nullptr;
        if (gfuncs::IsFormValid(killerRef)) {
            killer = static_cast<RE::Actor*>(killerRef);
            if (!gfuncs::IsFormValid(killer)) {
                killer = nullptr;
            }
            else {
                logger::trace("valid killerRef pointer");
            }
        }

        bool dead = event->dead;

        logger::trace("victim[{}], Killer[{}], Dead[{}]", gfuncs::GetFormName(victim), gfuncs::GetFormName(killer), dead);

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

        std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->GetHandles(
            { victim, killer });

        if (handles.size() > 0) {

            auto* args = RE::MakeFunctionArguments((RE::Actor*)victim, (RE::Actor*)killer);
            gfuncs::SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);
        }
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
    if (gfuncs::IsFormValid(actorRef)) {
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
    if (eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded || eventDataPtrs[EventEnum_HitEvent]->sinkAdded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                RE::TESObjectREFR* ref = form->AsReference();
                RegisterActorForBowDrawAnimEvent(ref);
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (gfuncs::IsFormValid(form)) {
                RE::TESObjectREFR* ref = form->AsReference();
                UnRegisterActorForBowDrawAnimEvent(ref);
            }
        }
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
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESObjectREFR* akActorRef = nullptr;
        if (event->actor) {
            akActorRef = event->actor.get();
            if (!gfuncs::IsFormValid(akActorRef)) {
                akActorRef = nullptr;
            }
        }

        RE::Actor* akActor = nullptr;
        if (akActorRef) {
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

        auto* calendar = RE::Calendar::GetSingleton();
        float fTime = 0.0;
        if (calendar) {
            fTime = calendar->GetHoursPassed();
            //equip events can get sent twice apparently. This happens when an ammo is force unequipped after shooting the last arrow or bolt of type in inventory. 
            //skip the event if time and other variables match last equip event
            if (fTime == lastEquipEvent.gameTimeStamp) {
                if (lastEquipEvent.akActorRef == akActorRef && lastEquipEvent.baseObject == baseObject && lastEquipEvent.equipped == equipped) {
                    logger::debug("Skipping Event: Actor[{}], BaseObject[{}], Equipped[{}] gameTimeStamp[{}]", gfuncs::GetFormName(akActorRef), gfuncs::GetFormName(baseObject), equipped, fTime);
                    return RE::BSEventNotifyControl::kContinue;
                }
            }
            lastEquipEvent.gameTimeStamp = fTime;
        }

        lastEquipEvent.akActorRef = akActorRef;
        lastEquipEvent.baseObject = baseObject;
        lastEquipEvent.equipped = equipped;

        if (!equipped) {
            if (baseObject) {
                RE::TESAmmo* akAmmo = baseObject->As<RE::TESAmmo>();
                if (gfuncs::IsFormValid(akAmmo)) {
                    if (akActorRef) {
                        auto it = animationEventActorsMap.find(akActorRef);
                        if (it != animationEventActorsMap.end()) {
                            float timeDiff = GameHoursToRealTimeSeconds(nullptr, (fTime - it->second->lastReleaseGameTime));
                            logger::trace("actor[{}] unequipped ammo[{}]. Time since last bow release is [{}]", gfuncs::GetFormName(akActor), gfuncs::GetFormName(akAmmo), timeDiff);

                            if (timeDiff < 0.2) { //actor released their bow less than 0.2 seconds ago.
                                logger::debug("saved force unequipped ammo[{}] for actor[{}]", gfuncs::GetFormName(akAmmo), gfuncs::GetFormName(akActor));
                                TrackedActorAmmoData data;
                                data.ammo = akAmmo;
                                data.gameTimeStamp = fTime;
                                it->second->forceChangedAmmos.push_back(data);
                            }
                        }
                    }
                }
                if (gfuncs::formIsBowOrCrossbow(baseObject)) {
                    logger::trace("Unregistering actor[{}] for animation events", gfuncs::GetFormName(akActorRef));
                    UnRegisterActorForBowDrawAnimEvent(akActorRef);
                }
            }

            if (eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded) {
                eventIndex = EventEnum_OnObjectUnequipped;
            }
        }
        else {
            if (gfuncs::formIsBowOrCrossbow(baseObject)) {
                if (eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded || eventDataPtrs[EventEnum_HitEvent]->sinkAdded) {
                    logger::trace("registering actor [{}] for animation events", gfuncs::GetFormName(akActorRef));
                    RegisterActorForBowDrawAnimEvent(akActorRef);
                }
            }

            if (eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded) {
                eventIndex = EventEnum_OnObjectEquipped;
            }
        }

        //logger::trace("Actor[{}], BaseObject[{}], Equipped[{}]", gfuncs::GetFormName(akActorRef), gfuncs::GetFormName(baseObject), equipped);

        if (eventIndex != -1) {
            RE::TESForm* ref = RE::TESForm::LookupByID(event->originalRefr);
            if (!gfuncs::IsFormValid(ref)) {
                ref = nullptr;
            }

            logger::trace("Actor[{}], BaseObject[{}], Ref[{}] Equipped[{}]", gfuncs::GetFormName(akActorRef), gfuncs::GetFormName(baseObject), gfuncs::GetFormName(ref), equipped);

            std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->GetHandles(
                { akActorRef, baseObject, ref });

            if (handles.size() > 0) {

                auto* args = RE::MakeFunctionArguments((RE::Actor*)akActorRef, (RE::TESForm*)baseObject, (RE::TESObjectREFR*)ref);
                gfuncs::SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct WaitStartEventSink : public RE::BSTEventSink<RE::TESWaitStartEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESWaitStartEvent* event, RE::BSTEventSource<RE::TESWaitStartEvent>*/*source*/) {

        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("");

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnWaitStart]->globalHandles;

        gfuncs::RemoveDuplicates(handles);
        if (handles.size() > 0) {

            auto* args = RE::MakeFunctionArguments();
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnWaitStart]->sEvent, args);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

struct WaitStopEventSink : public RE::BSTEventSink<RE::TESWaitStopEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESWaitStopEvent* event, RE::BSTEventSource<RE::TESWaitStopEvent>*/*source*/) {

        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("");

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnWaitStop]->globalHandles;

        gfuncs::RemoveDuplicates(handles);
        if (handles.size() > 0) {

            auto* args = RE::MakeFunctionArguments((bool)event->interrupted);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnWaitStop]->sEvent, args);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

struct MagicEffectApplyEventSink : public RE::BSTEventSink<RE::TESMagicEffectApplyEvent> { //
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESMagicEffectApplyEvent* event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*/*source*/) {

        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("");

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
        
        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnMagicEffectApply]->GetHandles(
            { caster, target, magicEffect });

        if (handles.size() > 0) {

            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)caster, (RE::TESObjectREFR*)target, (RE::EffectSetting*)magicEffect);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnMagicEffectApply]->sEvent, args);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

struct LockChangedEventSink : public RE::BSTEventSink<RE::TESLockChangedEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESLockChangedEvent* event, RE::BSTEventSource<RE::TESLockChangedEvent>*/*source*/) {

        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("");

        RE::TESObjectREFR* lockedObject = nullptr;
        if (event->lockedObject) {
            lockedObject = event->lockedObject.get();
        }

        if (!gfuncs::IsFormValid(lockedObject)) {
            return RE::BSEventNotifyControl::kContinue;
        }

        bool Locked = lockedObject->IsLocked();

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_LockChanged]->GetHandles(
            { lockedObject});

        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)lockedObject, (bool)Locked);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_LockChanged]->sEvent, args);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

struct OpenCloseEventSink : public RE::BSTEventSink<RE::TESOpenCloseEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESOpenCloseEvent* event, RE::BSTEventSource<RE::TESOpenCloseEvent>*/*source*/) {

        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
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
        
        std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->GetHandles(
            { activatorRef, akActionRef });

        if (handles.size() > 0) {

            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)activatorRef, (RE::TESObjectREFR*)akActionRef);
            gfuncs::SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

struct SpellCastEventSink : public RE::BSTEventSink<RE::TESSpellCastEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESSpellCastEvent* event, RE::BSTEventSource<RE::TESSpellCastEvent>*/*source*/) {

        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("");

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
        
        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnSpellCast]->GetHandles(
            { caster, spell });

        if (handles.size() > 0) {

            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)caster, (RE::TESForm*)spell);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnSpellCast]->sEvent, args);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

struct ContainerChangedEventSink : public RE::BSTEventSink<RE::TESContainerChangedEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* event, RE::BSTEventSource<RE::TESContainerChangedEvent>*/*source*/) {

        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        //logger::debug("Event");

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
        
        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnContainerChanged]->GetHandles(
            { newContainer, oldContainer, itemReference, baseObj });

        if (handles.size() > 0) {

            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)newContainer, (RE::TESObjectREFR*)oldContainer, (RE::TESObjectREFR*)itemReference, (RE::TESForm*)baseObj, (int)itemCount);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnContainerChanged]->sEvent, args);
        }
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
            logger::error("Event event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Event event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("Event");

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

        logger::trace("event, Actor[{}] Source[{}]  type[{}] slot[{}]", gfuncs::GetFormName(akActor), gfuncs::GetFormName(akSource),
            actionTypeStrings[type], actionSlotStrings[slot]);

        if (eventIndex == -1) {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (akActor) {
            std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->GetHandles(
                { akActor, akSource });

            if (handles.size() > 0) {
                auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor, (RE::TESForm*)akSource, (int)slot);
                gfuncs::SendEvents(handles, eventDataPtrs[eventIndex]->sEvent, args);

                //draw / sheathe events aren't triggered for left hand. Send left hand events manually
                if (eventIndex >= EventEnum_BeginDraw && eventIndex <= EventEnum_EndSheathe) {
                    if (gfuncs::IsFormValid(akActor)) {
                    }
                }
            }

            RE::TESForm* leftHandSource = akActor->GetEquippedObject(true);
            if (gfuncs::IsFormValid(leftHandSource)) {
                if (leftHandSource != akSource) {
                    std::vector<RE::VMHandle> handlesB = eventDataPtrs[eventIndex]->GetHandles(
                        { akActor, leftHandSource });

                    if (handlesB.size() > 0) {
                        auto* argsB = RE::MakeFunctionArguments((RE::Actor*)akActor, (RE::TESForm*)leftHandSource, (int)0);
                        gfuncs::SendEvents(handlesB, eventDataPtrs[eventIndex]->sEvent, argsB);

                        logger::trace("event, Actor[{}] Source[{}]  type[{}] slot[{}]", gfuncs::GetFormName(akActor), gfuncs::GetFormName(leftHandSource),
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
        //std::lock_guard<std::recursive_mutex> lock(openedMenusMutex); 

        if (!event) {
            logger::error("Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        int benchType = 0;
        std::string skill = "";
        int count = 1;

        RE::TESObjectREFR* workbenchRef = nullptr;

        RE::Actor* playerRef = static_cast<RE::Actor*>(RE::PlayerCharacter::GetSingleton());
        if (playerRef) {
            auto* aiProcess = playerRef->GetActorRuntimeData().currentProcess;
            if (aiProcess) {
                if (aiProcess->middleHigh) {
                    if (aiProcess->middleHigh->occupiedFurniture) {
                        auto refPtr = aiProcess->middleHigh->occupiedFurniture.get();
                        if (refPtr) {
                            //logger::trace("Event: occupiedFurniture found");
                            workbenchRef = refPtr.get();
                        }
                    }
                }
            }
        }


        if (!gfuncs::IsFormValid(workbenchRef)) {

            RE::TESObjectREFR* ref = GetLastPlayerMenuActivatedRef(nullptr);

            if (gfuncs::IsFormValid(ref, true)) {
                workbenchRef = ref;
                //logger::trace("Event: occupiedFurniture not valid, setting to menuRef");
            }
            else {
                workbenchRef = nullptr;
            }
        }

        if (gfuncs::IsFormValid(workbenchRef)) {
            RE::TESForm* baseObj = workbenchRef->GetBaseObject();
            if (gfuncs::IsFormValid(baseObj)) {
                RE::TESFurniture* furniture = baseObj->As<RE::TESFurniture>();
                benchType = furniture::GetFurnitureWorkbenchType(nullptr, furniture);
                skill = furniture::GetFurnitureWorkbenchSkillString(nullptr, furniture);
            }
        }

        RE::TESForm* craftedItem = nullptr;
        if (gfuncs::IsFormValid(event->item)) {
            craftedItem = event->item;
            if (!gfuncs::IsFormValid(craftedItem)) {
                return RE::BSEventNotifyControl::kContinue;
            }
        }
        
        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnItemCrafted]->GetHandles(
            { craftedItem, workbenchRef });

        if (handles.size() > 0) {
            if (benchType == 1) { //CreateObject
                count = UIEvents::uiLastSelectedFormData.count;
            }

            auto* args = RE::MakeFunctionArguments((RE::TESForm*)craftedItem, (RE::TESObjectREFR*)workbenchRef,
                (int)count, (int)benchType, (std::string)skill);

            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnItemCrafted]->sEvent, args);

            logger::trace("Event: workbenchRef[{}] craftedItem[{}] benchType[{}] skill[{}]",
                gfuncs::GetFormName(craftedItem), gfuncs::GetFormName(workbenchRef), benchType, skill);
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

ItemCraftedEventSink* itemCraftedEventSink;

struct ItemsPickpocketedEventSink : public RE::BSTEventSink<RE::ItemsPickpocketed::Event> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::ItemsPickpocketed::Event* event, RE::BSTEventSource<RE::ItemsPickpocketed::Event>*/*source*/) {
        //std::lock_guard<std::recursive_mutex> lock(openedMenusMutex); 

        if (!event) {
            logger::error("Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESForm* akForm = UIEvents::uiSelectedFormData.form;
        if (!gfuncs::IsFormValid(akForm)) {
            akForm = nullptr;
        }

        RE::Actor* target = nullptr;
        RE::TESObjectREFR* ref = GetLastPlayerMenuActivatedRef(nullptr);

        if (gfuncs::IsFormValid(ref, true)) {
            target = ref->As<RE::Actor>();
            if (!gfuncs::IsFormValid(target)) {
                return RE::BSEventNotifyControl::kContinue;
            }
        }
        
        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnItemsPickpocketed]->GetHandles(
            { target, akForm });

        if (handles.size() > 0) {

            auto* args = RE::MakeFunctionArguments((RE::Actor*)target, (RE::TESForm*)akForm, (int)event->numItems);

            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnItemsPickpocketed]->sEvent, args);

            logger::trace("numItems[{}] form[{}]", event->numItems, gfuncs::GetFormName(akForm));
        }

        //openedMenusMutex.unlock();
        return RE::BSEventNotifyControl::kContinue;
    }
};

ItemsPickpocketedEventSink* itemsPickpocketedEventSink;

struct LocationClearedEventSink : public RE::BSTEventSink<RE::LocationCleared::Event> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::LocationCleared::Event* event, RE::BSTEventSource<RE::LocationCleared::Event>*/*source*/) {
        if (!event) {
            logger::error("Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::BGSLocation* location = nullptr;
        RE::Actor* playerRef = static_cast<RE::Actor*>(RE::PlayerCharacter::GetSingleton());
        if (playerRef) {
            location = playerRef->GetCurrentLocation();
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
        }
        if (!gfuncs::IsFormValid(location)) {
            location = nullptr;
        }
        
        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnLocationCleared]->GetHandles(
            { location });

        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::BGSLocation*)location);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnLocationCleared]->sEvent, args);
            logger::trace("event: location[{}]", gfuncs::GetFormName(location));
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

LocationClearedEventSink* locationClearedEventSink;

struct EnterBleedoutEventSink : public RE::BSTEventSink<RE::TESEnterBleedoutEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESEnterBleedoutEvent* event, RE::BSTEventSource<RE::TESEnterBleedoutEvent>*/*source*/) {
        if (!event) {
            logger::error("Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Event IsBadReadPtr");
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

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnEnterBleedout]->GetHandles(
            { akActor });

        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnEnterBleedout]->sEvent, args);
            logger::trace("Event: akActor[{}]", gfuncs::GetFormName(akActor));
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

EnterBleedoutEventSink* enterBleedoutEventSink;

struct SwitchRaceCompleteEventSink : public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*/*source*/) {
        //logger::trace("SwitchRaceCompleteEvent");

        if (!event) {
            logger::error("Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Event IsBadReadPtr");
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

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnSwitchRaceComplete]->GetHandles(
            { akActor, akOldRace, akNewRace });

        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor, (RE::TESRace*)akOldRace, (RE::TESRace*)akNewRace);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnSwitchRaceComplete]->sEvent, args);
            logger::trace("Actor[{}] akOldRace[{}] akNewRace[{}]",
                gfuncs::GetFormName(akActor), gfuncs::GetFormName(akOldRace), gfuncs::GetFormName(akNewRace));
        }
        //EventEnum_OnSwitchRaceComplete
        return RE::BSEventNotifyControl::kContinue;
    }
};

SwitchRaceCompleteEventSink* switchRaceCompleteEventSink;

struct FootstepEventSink : public RE::BSTEventSink<RE::BGSFootstepEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::BGSFootstepEvent* event, RE::BSTEventSource<RE::BGSFootstepEvent>*/*source*/) {
        if (!event) {
            logger::error("Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Event IsBadReadPtr");
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
        
        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnActorFootStep]->GetHandles(
            { akActor });

        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::Actor*)akActor, (std::string)event->tag);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnActorFootStep]->sEvent, args);
            logger::trace("Actor[{}] tag[{}]",
                gfuncs::GetFormName(akActor), event->tag);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

FootstepEventSink* footstepEventSink;

struct QuestObjectiveEventSink : public RE::BSTEventSink<RE::ObjectiveState::Event> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::ObjectiveState::Event* event, RE::BSTEventSource<RE::ObjectiveState::Event>*/*source*/) {
        if (!event) {
            logger::error("Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (!event->objective) {
            logger::error("Event->objective is nullptr");
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
        
        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->GetHandles(
            { akQuest });

        if (handles.size() > 0) {

            auto* args = RE::MakeFunctionArguments((RE::TESQuest*)akQuest, (std::string)displayText, (int)oldState, (int)newState,
                (int)objectiveIndex, (std::vector<RE::BGSBaseAlias*>)ojbectiveAliases);

            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->sEvent, args);

            logger::trace("Event: quest[{}] displayText[{}] objectiveIndex[{}] oldState[{}] newState[{}] targets[{}]",
                gfuncs::GetFormName(akQuest), displayText, objectiveIndex, oldState, newState, event->objective->numTargets);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

QuestObjectiveEventSink* questObjectiveEventSink;

//offset difference for PLAYER_RUNTIME_DATA members between AE and SE is 8.
RE::PLAYER_TARGET_LOC* GetPlayerQueuedTargetLoc() {
    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        logger::error("player* not found");
        return nullptr;
    }
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
            logger::error("Event is nullptr");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("Event IsBadReadPtr");
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

            std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnPositionPlayerStart]->GetHandles(
                { fastTravelMarker, moveToRef, akWorldSpace, interiorCell });

            if (handles.size() > 0) {

                auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)fastTravelMarker, (RE::TESObjectREFR*)moveToRef,
                    (RE::TESWorldSpace*)akWorldSpace, (RE::TESObjectCELL*)interiorCell);

                gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnPositionPlayerStart]->sEvent, args);
            }
        }
        else if (type == 4) { //post load
            std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnPositionPlayerFinish]->GetHandles(
                { fastTravelMarker, moveToRef, akWorldSpace, interiorCell });

            if (handles.size() > 0) {
                auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)fastTravelMarker, (RE::TESObjectREFR*)moveToRef,
                    (RE::TESWorldSpace*)akWorldSpace, (RE::TESObjectCELL*)interiorCell);

                gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnPositionPlayerFinish]->sEvent, args);
            }
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
                RE::Actor* playerRef = static_cast<RE::Actor*>(RE::PlayerCharacter::GetSingleton());
                if (playerRef) {
                    newCell = playerRef->GetParentCell();
                }
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
                std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnPlayerChangeCell]->GetHandles(
                    { newCell, akPreviousCell });

                if (handles.size() > 0) {

                    auto* args = RE::MakeFunctionArguments((RE::TESObjectCELL*)newCell, (RE::TESObjectCELL*)akPreviousCell);

                    gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnPlayerChangeCell]->sEvent, args);
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

ActorCellEventSink* actorCellEventSink;

//new events =============================================================================================================================================

struct PerkEntryRunEventSink : public RE::BSTEventSink<RE::TESPerkEntryRunEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESPerkEntryRunEvent* event, RE::BSTEventSource<RE::TESPerkEntryRunEvent>*/*source*/) {
        if (!event) {
            logger::warn("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        auto perkId = event->perkId;
        auto owner = event->owner;
        auto target = event->target;
        auto flag = event->flag;

        RE::BGSPerk* perk = RE::TESForm::LookupByID< RE::BGSPerk>(perkId);
        if (!gfuncs::IsFormValid(perk)) {
            logger::warn("perk doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESObjectREFR* ownerRef = nullptr;
        if (owner) {
            ownerRef = owner.get();
            if (!gfuncs::IsFormValid(ownerRef)) {
                logger::warn("owner doesn't exist");
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        RE::TESObjectREFR* targetRef = nullptr;
        if (target) {
            targetRef = target.get();
            if (!gfuncs::IsFormValid(targetRef)) {
                targetRef = nullptr;
            }
        }

        logger::trace("perk[{}] target[{}] owner[{}] flag[{}]",
            gfuncs::GetFormNameAndId(perk), gfuncs::GetFormNameAndId(targetRef), gfuncs::GetFormNameAndId(ownerRef), flag);

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnPerkEntryRun]->GetHandles({ perk, targetRef, ownerRef });
        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::BGSPerk*)perk, (RE::TESObjectREFR*)targetRef, (RE::TESObjectREFR*)ownerRef, int(flag));
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnPerkEntryRun]->sEvent, args);
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

PerkEntryRunEventSink* perkEntryRunEventSink;

struct TriggerEnterEventSink : public RE::BSTEventSink<RE::TESTriggerEnterEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESTriggerEnterEvent* event, RE::BSTEventSource<RE::TESTriggerEnterEvent>*/*source*/) {
        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESObjectREFR* target = nullptr;
        if (event->target) {
            target = event->target.get();
            if (!gfuncs::IsFormValid(target)) {
                logger::warn("target doesn't exist");
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        RE::TESObjectREFR* caster = nullptr;
        if (event->caster) {
            caster = event->caster.get();
            if (!gfuncs::IsFormValid(caster)) {
                logger::warn("caster doesn't exist");
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        logger::trace("caster[{}] target[{}]",
            gfuncs::GetFormNameAndId(caster), gfuncs::GetFormNameAndId(target));

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnTriggerEnter]->GetHandles({ caster, target });
        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)caster, (RE::TESObjectREFR*)target);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnTriggerEnter]->sEvent, args);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

TriggerEnterEventSink* triggerEnterEventSink;

struct TriggerLeaveEventSink : public RE::BSTEventSink<RE::TESTriggerLeaveEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESTriggerLeaveEvent* event, RE::BSTEventSource<RE::TESTriggerLeaveEvent>*/*source*/) {
        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESObjectREFR* target = nullptr;
        if (event->target) {
            target = event->target.get();
            if (!gfuncs::IsFormValid(target)) {
                logger::warn("target doesn't exist");
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        RE::TESObjectREFR* caster = nullptr;
        if (event->caster) {
            caster = event->caster.get();
            if (!gfuncs::IsFormValid(caster)) {
                logger::warn("caster doesn't exist");
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        logger::trace("caster[{}] target[{}]",
            gfuncs::GetFormNameAndId(caster), gfuncs::GetFormNameAndId(target));

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnTriggerLeave]->GetHandles({ caster, target });
        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)caster, (RE::TESObjectREFR*)target);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnTriggerLeave]->sEvent, args);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

TriggerLeaveEventSink* triggerLeaveEventSink;

struct PackageEventSink : public RE::BSTEventSink<RE::TESPackageEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESPackageEvent* event, RE::BSTEventSource<RE::TESPackageEvent>*/*source*/) {
        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::Actor* actor = nullptr;
        if (event->actor) {
            RE::TESObjectREFR* actorRef = event->actor.get();
            if (gfuncs::IsFormValid(actorRef)) {
                actor = skyrim_cast<RE::Actor*>(actorRef);
                if (!gfuncs::IsFormValid(actor)) {
                    logger::warn("actor doesn't exist");
                    return RE::BSEventNotifyControl::kContinue;
                }
            }
        }

        RE::TESPackage* package = RE::TESForm::LookupByID<RE::TESPackage>(event->package);
        if (!gfuncs::IsFormValid(package)) {
            logger::warn("package doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        int eventIndex = -1;
        RE::BSFixedString sEvent;

        switch (event->type) {
        case RE::TESPackageEvent::EventType::kStart:
            eventIndex = EventEnum_OnPackageStart;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
            break;
        case RE::TESPackageEvent::EventType::kChange:
            eventIndex = EventEnum_OnPackageChange;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
            break;
        case RE::TESPackageEvent::EventType::kEnd:
            eventIndex = EventEnum_OnPackageEnd;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
            break;
        }

        logger::trace("actor[{}] package[{}] type[{}] event[{}]",
            gfuncs::GetFormNameAndId(actor), gfuncs::GetFormNameAndId(package), static_cast<int>(event->type), sEvent);

        if (eventIndex != -1) {
            std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->GetHandles({ actor, package });
            if (handles.size() > 0) {
                auto* args = RE::MakeFunctionArguments((RE::Actor*)actor, (RE::TESPackage*)package);
                gfuncs::SendEvents(handles, sEvent, args);
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

PackageEventSink* packageEventSink;

struct DestructionStageChangedEventSink : public RE::BSTEventSink<RE::TESDestructionStageChangedEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESDestructionStageChangedEvent* event, RE::BSTEventSource<RE::TESDestructionStageChangedEvent>*/*source*/) {
        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESObjectREFR* target = nullptr;
        if (event->target) {
            target = event->target.get();
            if (!gfuncs::IsFormValid(target)) {
                target = nullptr;
            }
        }

        logger::trace("target[{}] oldStage[{}] newStage[{}]",
            gfuncs::GetFormNameAndId(target), event->oldStage, event->newStage);

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnDestructionStageChanged]->GetHandles({ target });
        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)target, (uint32_t)event->oldStage, (uint32_t)event->newStage);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnDestructionStageChanged]->sEvent, args);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

DestructionStageChangedEventSink* destructionStageChangedEventSink;

struct ObjectREFRTranslationEventSink : public RE::BSTEventSink<RE::TESObjectREFRTranslationEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectREFRTranslationEvent* event, RE::BSTEventSource<RE::TESObjectREFRTranslationEvent>*/*source*/) {
        if (!event) {
            logger::error("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::error("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::TESObjectREFR* refr = nullptr;
        if (event->refr) {
            refr = event->refr.get();
            if (!gfuncs::IsFormValid(refr)) {
                logger::warn("ref doesn't exist");
                return RE::BSEventNotifyControl::kContinue;
            }
        }

        int eventIndex = -1;
        RE::BSFixedString sEvent;

        switch (event->type) {
        case RE::TESObjectREFRTranslationEvent::EventType::kFailed:
            eventIndex = EventEnum_OnTranslationFailed;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
            break;
        case RE::TESObjectREFRTranslationEvent::EventType::kCompleted:
            eventIndex = EventEnum_OnTranslationComplete;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
            break;
        case RE::TESObjectREFRTranslationEvent::EventType::kAlmostCompleted:
            eventIndex = EventEnum_OnTranslationAlmostComplete;
            sEvent = eventDataPtrs[eventIndex]->sEvent;
            break;
        }

        logger::trace("refr[{}] type[{}] event[{}]",
            gfuncs::GetFormNameAndId(refr), static_cast<int>(event->type), sEvent);
           

        if (eventIndex != -1) {
            std::vector<RE::VMHandle> handles = eventDataPtrs[eventIndex]->GetHandles({ refr });
            if (handles.size() > 0) {
                auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)refr);
                gfuncs::SendEvents(handles, sEvent, args);
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

ObjectREFRTranslationEventSink* objectREFRTranslationEventSink;

//====================================================================================================================================

struct MusicChangeEventSink {
    bool AddEventSink() {
        mutex.lock();
        if (!sinkAdded) {
            logger::debug("");
            sinkAdded = true;
            mutex.unlock();
            ListenForMusicChangeStart();
            return true;
        }
        else {
            mutex.unlock();
            return false;
        }
    }

    bool RemoveEventSink() {
        mutex.lock();
        if (sinkAdded) {
            sinkAdded = false; 
            mutex.unlock();
            return true;
        }
        else {
            mutex.unlock();
            return false;
        }
    }

    bool IsSinkAdded() {
        bool added;
        mutex.lock();
        added = sinkAdded;
        mutex.unlock();
        return added;
    }

    void SetCurrentMusic(RE::BGSMusicType* musicType) {
        mutex.lock();
        currentMusicType = musicType;
        mutex.unlock();
    }

    RE::BGSMusicType* GetCurrentMusic() {
        RE::BGSMusicType* musicType = nullptr;
        mutex.lock();
        musicType = currentMusicType;
        mutex.unlock();
        return musicType;
    }

private:
    void ListenForMusicChangeStart() {
        ListenForMusicChange();
    }

    void ListenForMusicChange() {
        std::thread t([=]() {
            auto* musicManager = RE::BSMusicManager::GetSingleton();
            mutex.lock();

            if (!musicManager) {
                sinkAdded = false;
                isPolling = false;
                mutex.unlock();
                logger::critical("musicManager* not found. OnMusicChangeGlobal event disabled.");
            }
            else if (isPolling){
                logger::trace("already polling");
                mutex.unlock();
            }
            else {
                isPolling = true;
                mutex.unlock();

                logger::trace("musicManager* found. listening for music change");
                RE::BGSMusicType* bsgMusicType = GetCurrentMusicType(nullptr);
                //RE::BSIMusicType* bsiMusicType = musicManager->current;
                RE::BSIMusicType* bsiMusicType = bsgMusicType;
                SetCurrentMusic(bsgMusicType);

                //using mutex lock with SetCurrentMusicBsi because it could be changed when loading a save.
                while (bsiMusicType == musicManager->current) {
                    //logger::info("while loop");
                    std::this_thread::sleep_for(std::chrono::milliseconds(eventPollingInterval));
                    bsiMusicType = GetCurrentMusic();
                }

                if (sinkAdded) {
                    HandleMusicChangeEvent(GetCurrentMusicType(nullptr), GetCurrentMusic());
                    mutex.lock();
                    isPolling = false;
                    mutex.unlock();
                    ListenForMusicChangeStart();
                }
                else {
                    mutex.lock();
                    isPolling = false;
                    mutex.unlock();
                }
            }
        });
        t.detach();
    }

    void HandleMusicChangeEvent(RE::BGSMusicType* newMusicType, RE::BGSMusicType* oldMusicType) {
        if (newMusicType == oldMusicType) {
            logger::debug("newMusicType == oldMusicType, likely because of loading a save. Aborting sending event.");
            return;
        }

        logger::trace("oldMusicType[{}] newMusicType[{}]", 
            gfuncs::GetFormDataString(oldMusicType), gfuncs::GetFormDataString(newMusicType));

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnMusicTypeChange]->GetHandles(
            { newMusicType, oldMusicType });

        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::BGSMusicType*)newMusicType, (RE::BGSMusicType*)oldMusicType);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnMusicTypeChange]->sEvent, args);
        }
    }

    std::mutex mutex;
    bool sinkAdded = false;
    bool isPolling = false;
    RE::BGSMusicType* currentMusicType = nullptr;
};

MusicChangeEventSink* musicChangeEventSink;

struct WeatherChangeEventSink {
    bool AddEventSink() {
        mutex.lock();
        if (!sinkAdded) {
            logger::debug("");
            sinkAdded = true;
            mutex.unlock();
            ListenForWeatherChangeStart();
            return true;
        }
        else {
            mutex.unlock();
            return false;
        }
    }

    bool RemoveEventSink() {
        mutex.lock();
        if (sinkAdded) {
            sinkAdded = false;
            mutex.unlock();
            return true;
        }
        else {
            mutex.unlock();
            return false;
        }
    }

    bool IsSinkAdded() {
        bool added;
        mutex.lock();
        added = sinkAdded;
        mutex.unlock();
        return added;
    }

    void SetCurrentWeather(RE::TESWeather* weather) {
        mutex.lock();
        currentWeather = weather;
        mutex.unlock();
    }

    RE::TESWeather* GetCurrentWeather() {
        RE::TESWeather* weather = nullptr; 
        mutex.lock();
        weather = currentWeather;
        mutex.unlock();
        return weather;
    }

private:
    void ListenForWeatherChangeStart() {
        ListenForWeatherChange();
    }

    void ListenForWeatherChange() {
        std::thread t([=]() {
            auto* sky = RE::Sky::GetSingleton();
            mutex.lock();

            if (!sky) {
                sinkAdded = false;
                isPolling = false;
                mutex.unlock();
                logger::critical("sky* not found. OnWeatherChangeGlobal event disabled.");
            }
            else if (isPolling) {
                logger::trace("already polling");
                mutex.unlock();
            }
            else {
                isPolling = true;
                mutex.unlock();

                logger::trace("sky* found. listening for weather change");
                
                SetCurrentWeather(sky->currentWeather);

                //using mutex lock to GetCurrentWeather. Loading a save may change the current weather.
                while (GetCurrentWeather() == sky->currentWeather) {
                    //logger::info("while loop");
                    std::this_thread::sleep_for(std::chrono::milliseconds(eventPollingInterval));
                }

                if (sinkAdded) {
                    HandleWeatherChangeEvent(sky->currentWeather, GetCurrentWeather());
                    mutex.lock();
                    isPolling = false;
                    mutex.unlock();
                    ListenForWeatherChangeStart();
                }
                else {
                    mutex.lock();
                    isPolling = false;
                    mutex.unlock();
                }
            }
            });
        t.detach();
    }

    void HandleWeatherChangeEvent(RE::TESWeather* newWeather, RE::TESWeather* oldWeather) {
        if (!gfuncs::IsFormValid(newWeather)) {
            logger::warn("newWeather doesn't exist");
            return;
        }

        if (newWeather == oldWeather) {
            logger::debug("newWeather == oldWeather, likely because of loading a save. Aborting sending event.");
            return;
        }

        logger::trace("oldWeather[{}] newWeather[{}]",
            gfuncs::GetFormDataString(oldWeather), gfuncs::GetFormDataString(newWeather));

        if (!gfuncs::IsFormValid(oldWeather)) {
            oldWeather = nullptr;
        }

        std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnWeatherChange]->GetHandles(
            { newWeather, oldWeather });

        if (handles.size() > 0) {
            auto* args = RE::MakeFunctionArguments((RE::TESWeather*)newWeather, (RE::TESWeather*)oldWeather);
            gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnWeatherChange]->sEvent, args);
        }
    }

    std::mutex mutex;
    bool sinkAdded = false;
    bool isPolling = false;
    RE::TESWeather* currentWeather = nullptr;
};

WeatherChangeEventSink* weatherChangeEventSink;

namespace ActiveEffectEvents {
    std::unordered_map<RE::ActiveEffect*, std::pair<std::chrono::system_clock::time_point, float>> activeEffectStartTimeMap;
    std::mutex activeEffectStartTimeMapMutex;
    bool effectStartRegistered = false;
    bool effectFinishRegistered = false;

    namespace EffectStartEvent {
        bool effectStartInstalled = false;
        std::mutex effectStartInstallMutex;

        template <class T>
        class EffectStartHook {
        public:
            static void thunk(T* effect) {
                if (effectStartRegistered || effectFinishRegistered) {
                    if (effect) {
                        RE::ActiveEffect* activeEffect = static_cast<RE::ActiveEffect*>(effect);
                        if (activeEffect) {
                            if (IsBadReadPtr(activeEffect, sizeof(activeEffect))) {
                                logger::warn("EffectStart: activeEffect IsBadReadPtr");
                            }
                            else {
                                auto* calendar = RE::Calendar::GetSingleton();
                                float gameHoursPassed = 0.0; 
                                if (calendar) {
                                    gameHoursPassed = calendar->GetHoursPassed();
                                }

                                std::pair<std::chrono::system_clock::time_point, float> startTime{std::chrono::system_clock::now(), gameHoursPassed};
                                activeEffectStartTimeMapMutex.lock();
                                activeEffectStartTimeMap[activeEffect] = startTime;
                                activeEffectStartTimeMapMutex.unlock();

                                //logger::info("EffectStart: activeEffect found. effectName[{}]", effectName);

                                if (effectStartRegistered) {
                                    RE::EffectSetting* baseEffect = activeEffect->GetBaseObject();
                                    if (!gfuncs::IsFormValid(baseEffect)) {
                                        baseEffect = nullptr;
                                    }

                                    RE::Actor* caster = nullptr;
                                    RE::ActorPtr casterPtr = activeEffect->GetCasterActor();
                                    if (casterPtr) {
                                        caster = casterPtr.get();
                                        if (!gfuncs::IsFormValid(caster)) {
                                            caster = nullptr;
                                        }
                                    }

                                    RE::Actor* target = nullptr;
                                    auto magicTarget = activeEffect->target;
                                    if (magicTarget) {
                                        //logger::info("magicTarget found");
                                        target = skyrim_cast<RE::Actor*>(magicTarget);
                                        if (!gfuncs::IsFormValid(target)) {
                                            target = nullptr;
                                        }
                                    }

                                    RE::TESForm* source = activeEffect->spell;
                                    if (!gfuncs::IsFormValid(source)) {
                                        source = nullptr;
                                    }

                                    std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnEffectStart]->GetHandles(
                                        { caster, target, baseEffect, source });

                                    if (handles.size() > 0) {
                                        int castingSource = static_cast<int>(activeEffect->castingSource);
                                        auto* args = RE::MakeFunctionArguments((RE::Actor*)caster, (RE::Actor*)target,
                                            (RE::EffectSetting*)baseEffect, (RE::TESForm*)source, (int)castingSource);

                                        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnEffectStart]->sEvent, args);

                                        auto effectClassName = typeid(*effect).name();

                                        logger::trace("EffectStart [{}]: effect[{}] caster[{}] target[{}] source[{}] castingSource[{}]",
                                            effectClassName,
                                            gfuncs::GetFormName(baseEffect),
                                            gfuncs::GetFormName(caster),
                                            gfuncs::GetFormName(target),
                                            gfuncs::GetFormName(source),
                                            castingSource
                                        );
                                    }
                                }
                            }
                        }
                        else {
                            logger::trace("EffectStart: activeEffect is nullptr");
                        }
                    }
                    else {
                        logger::trace("EffectStart: effect is nullptr");
                    }
                }
                return func(effect);
            }

            static void Install() {
                stl::write_vfunc2<T, 0, EffectStartHook<T>>();
            }

            static inline REL::Relocation<void(T*)> func;
            static inline std::uint32_t idx = 0x14; //RE::ActiveEffect::Start()
        };

        void Install() {
            effectStartInstallMutex.lock();
            if (!effectStartInstalled) {
                effectStartInstalled = true;
                logger::info("");

                //install all sub classes of RE::ActiveEffect
                EffectStartHook<RE::BoundItemEffect>::Install();
                EffectStartHook<RE::CloakEffect>::Install();
                EffectStartHook<RE::CommandEffect>::Install();
                EffectStartHook<RE::CommandSummonedEffect>::Install();
                EffectStartHook<RE::ConcussionEffect>::Install();
                EffectStartHook<RE::CureEffect>::Install();
                EffectStartHook<RE::DetectLifeEffect>::Install();
                EffectStartHook<RE::DisguiseEffect>::Install();
                EffectStartHook<RE::DispelEffect>::Install();
                EffectStartHook<RE::EtherealizationEffect>::Install();
                EffectStartHook<RE::GuideEffect>::Install();
                EffectStartHook<RE::LightEffect>::Install();
                EffectStartHook<RE::LockEffect>::Install();
                EffectStartHook<RE::OpenEffect>::Install();
                EffectStartHook<RE::ScriptEffect>::Install();
                EffectStartHook<RE::SoulTrapEffect>::Install();
                EffectStartHook<RE::SpawnHazardEffect>::Install();
                EffectStartHook<RE::StaggerEffect>::Install();
                EffectStartHook<RE::SummonCreatureEffect>::Install();
                EffectStartHook<RE::TelekinesisEffect>::Install();
                EffectStartHook<RE::ValueModifierEffect>::Install();
                EffectStartHook<RE::VampireLordEffect>::Install();
                EffectStartHook<RE::WerewolfEffect>::Install();
                EffectStartHook<RE::WerewolfFeedEffect>::Install();

                //sub sub classes that include Start() and Finish() overrides =====================================

                //extends ValueModifierEffect
                EffectStartHook<RE::AccumulatingValueModifierEffect>::Install();

                //extends DualValueModifierEffect which extends ValueModifierEffect. 
                EffectStartHook<RE::EnhanceWeaponEffect>::Install();

                //extends TargetValueModifierEffect which extends ValueModifierEffect. 
                EffectStartHook<RE::FrenzyEffect>::Install();

                //extends ValueModifierEffect
                EffectStartHook<RE::InvisibilityEffect>::Install();

                //extends ValueModifierEffect
                EffectStartHook<RE::DarknessEffect>::Install();

                //extends ValueModifierEffect
                EffectStartHook<RE::NightEyeEffect>::Install();

                //extends ValueModifierEffect
                EffectStartHook<RE::ParalysisEffect>::Install();

                //extends CommandEffect 
                EffectStartHook<RE::ReanimateEffect>::Install();

                //extends ScriptEffect
                EffectStartHook<RE::SlowTimeEffect>::Install();

                //extends ValueModifierEffect
                EffectStartHook<RE::GrabActorEffect>::Install();


                //sub sub classes that only include Start() override
                //===========================================================================================================

                //extends TargetValueModifierEffect which extends ValueModifierEffect. 
                EffectStartHook<RE::CalmEffect>::Install();

                //extends DemoralizeEffect which extends TargetValueModifierEffect which extends ValueModifierEffect. 
                EffectStartHook<RE::BanishEffect>::Install();

                //extends StaggerEffect
                EffectStartHook<RE::DisarmEffect>::Install();

                //extends ValueModifierEffect
                EffectStartHook<RE::TargetValueModifierEffect>::Install();
            }
            effectStartInstallMutex.unlock();
        }
    }

    namespace EffectFinishEvent {
        bool effectFinishInstalled = false;
        std::mutex effectFinishInstallMutex;

        template <class T>
        class EffectFinishHook {
        public:
            static void thunk(T* effect) {
                if (effectStartRegistered || effectFinishRegistered) {
                    if (effect) {
                        RE::ActiveEffect* activeEffect = static_cast<RE::ActiveEffect*>(effect);
                        if (activeEffect) {
                            if (IsBadReadPtr(activeEffect, sizeof(activeEffect))) {
                                logger::warn("EffectFinish: activeEffect IsBadReadPtr");
                            }
                            else {
                                std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                                std::chrono::system_clock::time_point startTime = now;
                                float startGameTime = 0.0;

                                activeEffectStartTimeMapMutex.lock();
                                auto timeMapItr = activeEffectStartTimeMap.find(activeEffect);
                                if (timeMapItr != activeEffectStartTimeMap.end()) {
                                    //timeMapItr->second is a <timepoint, float> pair. 
                                    //timepoint is timepoint when this effect started.
                                    //float is gameHoursPassed when this effect started.
                                    startTime = timeMapItr->second.first;
                                    startGameTime = timeMapItr->second.second;
                                    activeEffectStartTimeMap.erase(timeMapItr);
                                }
                                activeEffectStartTimeMapMutex.unlock();

                                if (effectFinishRegistered) {

                                    //logger::info("EffectFinish: activeEffect found.");

                                    RE::EffectSetting* baseEffect = activeEffect->GetBaseObject();
                                    if (!gfuncs::IsFormValid(baseEffect)) {
                                        baseEffect = nullptr;
                                    }

                                    RE::Actor* caster = nullptr;
                                    RE::ActorPtr casterPtr = activeEffect->GetCasterActor();
                                    if (casterPtr) {
                                        caster = casterPtr.get();
                                        if (!gfuncs::IsFormValid(caster)) {
                                            caster = nullptr;
                                        }
                                    }

                                    RE::Actor* target = nullptr;
                                    auto magicTarget = activeEffect->target;
                                    if (magicTarget) {
                                        //logger::info("magicTarget found");
                                        target = skyrim_cast<RE::Actor*>(magicTarget);
                                        if (!gfuncs::IsFormValid(target)) {
                                            target = nullptr;
                                        }
                                    }

                                    RE::TESForm* source = activeEffect->spell;
                                    if (!gfuncs::IsFormValid(source)) {
                                        source = nullptr;
                                    }

                                    //this only applies to abilities, and is the elapsedSeconds since the ability spell was added, not since it was last started.
                                    //float elapsedSeconds = activeEffect->elapsedSeconds; 

                                    std::vector<RE::VMHandle> handles = eventDataPtrs[EventEnum_OnEffectFinish]->GetHandles(
                                        { caster, target, baseEffect, source });

                                    if (handles.size() > 0) {
                                        float elapsedSeconds = 0.0;
                                        float elapsedGameHours = 0.0;
                                        if (startTime != now) {
                                            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
                                            elapsedSeconds = float(milliseconds.count()) * float(0.001);
                                            auto* calendar = RE::Calendar::GetSingleton();
                                            if (calendar) {
                                                elapsedGameHours = calendar->GetHoursPassed() - startGameTime;
                                            }
                                        }

                                        int castingSource = static_cast<int>(activeEffect->castingSource);

                                        auto* args = RE::MakeFunctionArguments((RE::Actor*)caster, (RE::Actor*)target,
                                            (RE::EffectSetting*)baseEffect, (RE::TESForm*)source, (int)castingSource,
                                            (float)elapsedSeconds, (float)elapsedGameHours);

                                        gfuncs::SendEvents(handles, eventDataPtrs[EventEnum_OnEffectFinish]->sEvent, args);

                                        auto effectClassName = typeid(*effect).name();

                                        logger::trace("EffectFinish [{}]: effect[{}] caster[{}] target[{}] source[{}] castingSource[{}] elapsedSeconds[{}] elapsedGameHours[{}]",
                                            effectClassName,
                                            gfuncs::GetFormName(baseEffect),
                                            gfuncs::GetFormName(caster),
                                            gfuncs::GetFormName(target),
                                            gfuncs::GetFormName(source),
                                            castingSource, elapsedSeconds, elapsedGameHours
                                        );
                                    }
                                }
                            }
                        }
                        else {
                            logger::trace("EffectFinish: activeEffect is nullptr");
                        }
                    }
                    else {
                        logger::trace("EffectFinish: effect is nullptr");
                    }
                }
                return func(effect);
            }

            static void Install() {
                stl::write_vfunc2<T, 0, EffectFinishHook<T>>();
            }

            static inline REL::Relocation<void(T*)> func;
            static inline std::uint32_t idx = 0x15; //RE::ActiveEffect::Finish()
        };

        void Install() {
            effectFinishInstallMutex.lock();
            if (!effectFinishInstalled) {
                effectFinishInstalled = true;
                logger::info("");

                //install all sub classes of RE::ActiveEffect
                EffectFinishHook<RE::BoundItemEffect>::Install();
                EffectFinishHook<RE::CloakEffect>::Install();
                EffectFinishHook<RE::CommandEffect>::Install();
                EffectFinishHook<RE::CommandSummonedEffect>::Install();
                EffectFinishHook<RE::ConcussionEffect>::Install();
                EffectFinishHook<RE::CureEffect>::Install();
                EffectFinishHook<RE::DetectLifeEffect>::Install();
                EffectFinishHook<RE::DisguiseEffect>::Install();
                EffectFinishHook<RE::DispelEffect>::Install();
                EffectFinishHook<RE::EtherealizationEffect>::Install();
                EffectFinishHook<RE::GuideEffect>::Install();
                EffectFinishHook<RE::LightEffect>::Install();
                EffectFinishHook<RE::LockEffect>::Install();
                EffectFinishHook<RE::OpenEffect>::Install();
                EffectFinishHook<RE::ScriptEffect>::Install();
                EffectFinishHook<RE::SoulTrapEffect>::Install();
                EffectFinishHook<RE::SpawnHazardEffect>::Install();
                EffectFinishHook<RE::StaggerEffect>::Install();
                EffectFinishHook<RE::SummonCreatureEffect>::Install();
                EffectFinishHook<RE::TelekinesisEffect>::Install();
                EffectFinishHook<RE::ValueModifierEffect>::Install();
                EffectFinishHook<RE::VampireLordEffect>::Install();
                EffectFinishHook<RE::WerewolfEffect>::Install();
                EffectFinishHook<RE::WerewolfFeedEffect>::Install();

                //sub sub classes that include Start() and Finish() overrides =====================================

                //extends ValueModifierEffect
                EffectFinishHook<RE::AccumulatingValueModifierEffect>::Install();

                //extends DualValueModifierEffect which extends ValueModifierEffect. 
                EffectFinishHook<RE::EnhanceWeaponEffect>::Install();

                //extends TargetValueModifierEffect which extends ValueModifierEffect. 
                EffectFinishHook<RE::FrenzyEffect>::Install();

                //extends ValueModifierEffect
                EffectFinishHook<RE::InvisibilityEffect>::Install();

                //extends ValueModifierEffect
                EffectFinishHook<RE::DarknessEffect>::Install();

                //extends ValueModifierEffect
                EffectFinishHook<RE::NightEyeEffect>::Install();

                //extends ValueModifierEffect
                EffectFinishHook<RE::ParalysisEffect>::Install();

                //extends CommandEffect 
                EffectFinishHook<RE::ReanimateEffect>::Install();

                //extends ScriptEffect
                EffectFinishHook<RE::SlowTimeEffect>::Install();

                //extends ValueModifierEffect
                EffectFinishHook<RE::GrabActorEffect>::Install();
            }
            effectFinishInstallMutex.unlock();
        }
    }
}

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
                    logger::trace("Actor[{}] loaded", gfuncs::GetFormName(akActor));
                    return RE::BSEventNotifyControl::kContinue;
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

ObjectLoadedEventSink* objectLoadedEventSink;

bool IsItemMenuOpenNative(RE::StaticFunctionTag*) {
    auto* ui = RE::UI::GetSingleton();
    if (!ui) {
        logger::error("ui* not found");
        return false;
    }
    return ui->IsItemMenuOpen();
}

bool IsItemMenuOpen() {
    auto* ui = RE::UI::GetSingleton();
    if (!ui) {
        logger::error("ui* not found");
        return false;
    }

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

        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            //don't want to send ui select events to papyrus if message box context is open, only when selecting item.
            if (ui->IsMenuOpen(RE::MessageBoxMenu::MENU_NAME)) {
                return RE::BSEventNotifyControl::kContinue;
            }
        }
        else {
            logger::error("ui* not found");
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
                        if (ui->IsItemMenuOpen()) {
                            UIEvents::ProcessUiItemSelectEvent();
                            logger::trace("button[{}] pressed. ProcessUiItemSelectEvent", buttonEvent->GetIDCode());
                        }
                    }
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

InputEventSink* inputEventSink;

void HandleMenuOpenCloseEvent(bool opening, std::string sMenuName) {
    //std::lock_guard<std::recursive_mutex> lock(openedMenusMutex); 

    //access, read / write SharedUIVariables
    sharedVars.update([&](SharedUIVariables& vars) {
        auto openedMenusItr = std::find(vars.openedMenus.begin(), vars.openedMenus.end(), sMenuName);
        auto* ui = RE::UI::GetSingleton();

        if (opening) {
            vars.lastMenuOpened = sMenuName;

            if (openedMenusItr == vars.openedMenus.end()) {
                vars.openedMenus.push_back(sMenuName);
            }

            logger::trace("menu[{}] opened", sMenuName);

            if (ui) {
                if (ui->GameIsPaused() && !vars.gamePaused) {
                    vars.gamePaused = true;
                    vars.lastTimeGameWasPaused = (std::chrono::system_clock::now());
                    logger::trace("game was paused");
                }
            }

            if (!vars.inMenuMode) { //opened menu
                vars.inMenuMode = true;
                vars.lastTimeMenuWasOpened = (std::chrono::system_clock::now());
                logger::trace("inMenuMode = true");
            }

            if (bActivateEventSinkEnabledByDefault) {
                if (gfuncs::IsRefActivatedMenu(sMenuName)) {
                    if (gfuncs::IsFormValid(vars.lastPlayerActivatedRef)) {
                        vars.menuRef = vars.lastPlayerActivatedRef;
                        auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
                        if (vm) {
                            AttachDbSksePersistentVariablesScript();
                            auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)vars.menuRef);
                            vm->SendEventAll("OnDbSksePlayerActivatedMenuRef", args);
                            delete args;
                        }
                    }
                }
            }

            //AddToMenusCurrentlyOpen(event->menuName);
            /* std::thread tAddToMenusCurrentlyOpen(AddToMenusCurrentlyOpen, event->menuName);
            tAddToMenusCurrentlyOpen.join();*/

            if (ui) {
                if (ui->IsItemMenuOpen()) {
                    if (UIEvents::registeredUIEventDatas.size() > 0) {
                        if (!inputEventSink->sinkAdded) {
                            auto* inputManager = RE::BSInputDeviceManager::GetSingleton();
                            if (inputManager) {
                                inputEventSink->sinkAdded = true;
                                inputManager->AddEventSink(inputEventSink);
                                logger::trace("item menu [{}] opened. Added input event sink", sMenuName);
                            }
                        }
                    }
                }
            }
        }
        else {
            logger::trace("menu[{}] closed", sMenuName);

            if (ui) {
                if (!ui->GameIsPaused() && vars.gamePaused) {
                    vars.gamePaused = false;
                    auto now = std::chrono::system_clock::now();
                    float fGamePausedTime = gfuncs::timePointDiffToFloat(now, vars.lastTimeGameWasPaused);
                    //UpdateTimers(fGamePausedTime);
                    std::thread tUpdateTimers(timers::UpdateTimers, fGamePausedTime);
                    tUpdateTimers.detach();
                    logger::trace("game was unpaused");
                }
            }

            if (openedMenusItr != vars.openedMenus.end()) {
                vars.openedMenus.erase(openedMenusItr);
            }

            if (vars.openedMenus.size() == 0) { //closed all menus
                vars.inMenuMode = false;
                auto now = std::chrono::system_clock::now();
                float timePointDiff = gfuncs::timePointDiffToFloat(now, vars.lastTimeMenuWasOpened);
                //UpdateNoMenuModeTimers(timePointDiff);
                std::thread tUpdateNoMenuModeTimers(timers::UpdateNoMenuModeTimers, timePointDiff);
                tUpdateNoMenuModeTimers.detach();
                logger::trace("inMenuMode = false");
            }

            if (ui) {
                if (!ui->IsItemMenuOpen()) {
                    if (inputEventSink->sinkAdded) {
                        auto* inputManager = RE::BSInputDeviceManager::GetSingleton();
                        if (inputManager) {
                            inputEventSink->sinkAdded = false;
                            inputManager->RemoveEventSink(inputEventSink);
                            logger::trace("item menu [{}] closed. Removed input event sink", sMenuName);
                        }
                    }
                }
            }
        }
    });
}

struct MenuOpenCloseEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    bool sinkAdded = false;

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*/*source*/) {
        //this sink is for managing timers and GetCurrentMenuOpen function.
        
        if (!bMenuOpenCloseEventSinkEnabled) {
            auto* ui = RE::UI::GetSingleton();
            if (ui) {
                sinkAdded = false;
                ui->RemoveEventSink<RE::MenuOpenCloseEvent>(this);
            }
            return RE::BSEventNotifyControl::kContinue;
        }

        if (!event) {
            logger::warn("event doesn't exist");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (IsBadReadPtr(event, sizeof(event))) {
            logger::warn("event IsBadReadPtr");
            return RE::BSEventNotifyControl::kContinue;
        }

        //logger::trace("Menu Open Close Event, menu[{}], opened[{}]", event->menuName, event->opening);
        std::string sMenuName = std::string(event->menuName);
        bool opening = event->opening;

        if (sMenuName != RE::HUDMenu::MENU_NAME) { //hud menu is always open, don't need to do anything for it.
            //HandleMenuOpenCloseEvent(opening, sMenuName);
            std::thread t(HandleMenuOpenCloseEvent, opening, sMenuName);
            t.detach();
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
    //logger::trace("");
    if (!eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty() || !eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty() || !eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
        auto* eventSourceholder = RE::ScriptEventSourceHolder::GetSingleton();
        if (eventSourceholder) {
            if (!equipEventSink->sinkAdded) {
                equipEventSink->sinkAdded = true;
                eventSourceholder->AddEventSink(equipEventSink);
                RegisterActorsForBowDrawAnimEvents();
                logger::debug("Sink Added");
                return true;
            }
        }
        else {
            logger::error("eventSourceholder not found. Equip Event Sink not added");
        }
    }
    return false;
}

bool RemoveEquipEventSink() {
    logger::trace("");
    
    if (eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty() && eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty() && eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
        auto* eventSourceholder = RE::ScriptEventSourceHolder::GetSingleton();
        if (eventSourceholder) {
            if (equipEventSink->sinkAdded) {
                equipEventSink->sinkAdded = false;
                eventSourceholder->RemoveEventSink(equipEventSink); //always added to track recent hit projectiles for the GetRecentHitArrowRefsMap function
                logger::debug("Sink Removed");
                return true;
            }
        }
        else {
            logger::error("eventSourceholder not found. Equip Event Sink not added");
        }
    }
    return false;
}

bool AddHitEventSink() {
    if (!hitEventSink->sinkAdded && (!eventDataPtrs[EventEnum_HitEvent]->isEmpty() || !eventDataPtrs[EventEnum_OnProjectileImpact]->isEmpty())) {
        auto* eventSourceholder = RE::ScriptEventSourceHolder::GetSingleton();
        if (eventSourceholder) {
            hitEventSink->sinkAdded = true;
            RegisterActorsForBowDrawAnimEvents();
            eventSourceholder->AddEventSink(hitEventSink);

            return true;
        }
        else {
            logger::error("eventSourceholder not found. Hit Event Sink not added");
        }
    }
    return false;
}

bool RemoveHitEventSink() {
    if (hitEventSink->sinkAdded && eventDataPtrs[EventEnum_HitEvent]->isEmpty() && eventDataPtrs[EventEnum_OnProjectileImpact]->isEmpty()) {
        auto* eventSourceholder = RE::ScriptEventSourceHolder::GetSingleton();
        if (eventSourceholder) {
            hitEventSink->sinkAdded = false;
            eventSourceholder->RemoveEventSink(hitEventSink);
            RegisterActorsForBowDrawAnimEvents();
            logger::debug("");
            return true;
        }
        else {
            logger::error("eventSourceholder not found. Hiy Event Sink not removed");
        }
    }
    return false;
}

bool ShouldPositionPlayerEventSinkBeAdded() {
    return(!eventDataPtrs[EventEnum_OnPositionPlayerStart]->isEmpty() || !eventDataPtrs[EventEnum_OnPositionPlayerFinish]->isEmpty());
}

void AddSink(int index) {
    auto* eventSourceholder = RE::ScriptEventSourceHolder::GetSingleton();
    std::string eventName = "not found";
    auto* eventData = eventDataPtrs[index];
    if (eventData) {
        eventName = std::string(eventData->sEvent);
    }
    logger::trace("Index[{}] Event[{}].", index, eventName);

    switch (index) {
    case EventEnum_OnCombatStateChanged:
        if (!combatEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnCombatStateChanged]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            combatEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnCombatStateChanged]->sinkAdded = true;
            eventSourceholder->AddEventSink(combatEventSink);
            logger::debug("EventEnum_OnCombatStateChanged sink added");
        }
        break;

    case EventEnum_FurnitureEnter:

    case EventEnum_FurnitureExit:
        if (!furnitureEventSink->sinkAdded && (!eventDataPtrs[EventEnum_FurnitureEnter]->isEmpty() || !eventDataPtrs[EventEnum_FurnitureExit]->isEmpty())) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            furnitureEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded = true;
            eventSourceholder->AddEventSink(furnitureEventSink);
            logger::debug("EventEnum_FurnitureExit sink added");
        }
        break;

    case EventEnum_OnActivate:
        if (!eventDataPtrs[EventEnum_OnActivate]->sinkAdded && !eventDataPtrs[EventEnum_OnActivate]->isEmpty()) {
            if (!eventSourceholder && !activateEventSink->sinkAdded) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }

            if (!activateEventSink->sinkAdded) {
                activateEventSink->sinkAdded = true;
                eventSourceholder->AddEventSink(activateEventSink);
            }

            eventDataPtrs[EventEnum_OnActivate]->sinkAdded = true;

            logger::debug("EventEnum_OnActivate sink added");
        }
        break;

    case EventEnum_HitEvent:
        if (AddHitEventSink()) {
            logger::debug("EventEnum_hitEvent sink added");
        }
        if (AddEquipEventSink()); {
            logger::debug("EventEnum_HitEvent Equip event sink added");
        }
        if (!eventDataPtrs[EventEnum_HitEvent]->sinkAdded && !eventDataPtrs[EventEnum_HitEvent]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            eventDataPtrs[EventEnum_HitEvent]->sinkAdded = true;
            logger::debug("EventEnum_hitEvent sink added");
        }
        break;

    case EventEnum_DeathEvent:

    case EventEnum_DyingEvent:
        if (!deathEventSink->sinkAdded && (!eventDataPtrs[EventEnum_DeathEvent]->isEmpty() || !eventDataPtrs[EventEnum_DyingEvent]->isEmpty())) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            deathEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_DeathEvent]->sinkAdded = true;
            eventSourceholder->AddEventSink(deathEventSink);
            logger::debug("EventEnum_DyingEvent sink added");
        }
        break;

    case EventEnum_OnObjectEquipped:
        if (!eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded = true;
            logger::debug("EventEnum_OnObjectEquipped sink added");
        }
        if (AddEquipEventSink()) {
            logger::debug("EventEnum_OnObjectEquipped Equip event sink added");
        }
        break;

    case EventEnum_OnObjectUnequipped:
        if (!eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded = true;
            logger::debug("EventEnum_OnObjectUnequipped sink added");
        }
        if (AddEquipEventSink()) {
            logger::debug("EventEnum_OnObjectUnequipped Equip event sink added");
        }
        break;

    case EventEnum_OnWaitStart:
        if (!waitStartEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnWaitStart]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            waitStartEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnWaitStart]->sinkAdded = true;
            eventSourceholder->AddEventSink(waitStartEventSink);
            logger::debug("EventEnum_OnWaitStart sink added");
        }
        break;

    case EventEnum_OnWaitStop:
        if (!waitStopEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnWaitStop]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            waitStopEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnWaitStop]->sinkAdded = true;
            eventSourceholder->AddEventSink(waitStopEventSink);
            logger::debug("EventEnum_OnWaitStop sink added");
        }
        break;

    case EventEnum_OnMagicEffectApply:
        if (!magicEffectApplyEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnMagicEffectApply]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            magicEffectApplyEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnMagicEffectApply]->sinkAdded = true;
            eventSourceholder->AddEventSink(magicEffectApplyEventSink);
            logger::debug("EventEnum_OnMagicEffectApply sink added");
        }
        break;

    case EventEnum_OnSpellCast: //13
        if (!spellCastEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnSpellCast]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            spellCastEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnSpellCast]->sinkAdded = true;
            eventSourceholder->AddEventSink(spellCastEventSink);
            logger::debug("EventEnum_OnSpellCast sink added");
        }
        break;

    case EventEnum_LockChanged:
        if (!lockChangedEventSink->sinkAdded && !eventDataPtrs[EventEnum_LockChanged]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            lockChangedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_LockChanged]->sinkAdded = true;
            eventSourceholder->AddEventSink(lockChangedEventSink);
            logger::debug("EventEnum_LockChanged sink added");
        }
        break;

    case EventEnum_OnOpen:

    case EventEnum_OnClose:
        if (!openCloseEventSink->sinkAdded && (!eventDataPtrs[EventEnum_OnOpen]->isEmpty() || !eventDataPtrs[EventEnum_OnClose]->isEmpty())) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
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
            auto* actionEventSource = SKSE::GetActionEventSource();
            if (!eventSourceholder) {
                logger::error("ActionEventSource not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            actorActionEventSink->sinkAdded = true;
            logger::debug("actorActionEventSink sink added");
            eventDataPtrs[EventEnum_EndSheathe]->sinkAdded = true;
            actionEventSource->AddEventSink(actorActionEventSink);
        }
        break;

    case EventEnum_OnContainerChanged:
        if (!containerChangedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnContainerChanged]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            containerChangedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnContainerChanged]->sinkAdded = true;
            eventSourceholder->AddEventSink(containerChangedEventSink);
            logger::debug("EventEnum_OnContainerChanged sink added");
        }
        break;


    case EventEnum_OnProjectileImpact:
        if (AddHitEventSink()) {

        }
        if (!eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded && !eventDataPtrs[EventEnum_OnProjectileImpact]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            eventDataPtrs[EventEnum_OnProjectileImpact]->sinkAdded = true;
            logger::debug("EventEnum_OnProjectileImpact sink added");
        }
        break;

    case EventEnum_OnItemCrafted:
        if (!itemCraftedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnItemCrafted]->isEmpty()) {
            auto* itemCraftedEventSource = RE::ItemCrafted::GetEventSource();
            if (!itemCraftedEventSource) {
                logger::error("itemCraftedEventSource not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            itemCraftedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnItemCrafted]->sinkAdded = true;
            UIEvents::InstallUiEventHook(UIEvents::UiItemMenuEnum_Crafting_Menu); //track crafting menu selection data, (mostly needed for item count).
            itemCraftedEventSource->AddEventSink(itemCraftedEventSink);
            logger::debug("EventEnum_OnItemCrafted sink added");
        }
        break;

    case EventEnum_OnItemsPickpocketed:
        if (!itemsPickpocketedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnItemsPickpocketed]->isEmpty()) {
            auto* itemPickpocketedEventSource = RE::ItemsPickpocketed::GetEventSource();
            if (!itemPickpocketedEventSource) {
                logger::error("itemPickpocketedEventSource not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            itemsPickpocketedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnItemsPickpocketed]->sinkAdded = true;
            UIEvents::InstallUiEventHook(UIEvents::UiItemMenuEnum_ContainerMenu); //track container menu selection data, (to get the form taken / pickpocketed).
            itemPickpocketedEventSource->AddEventSink(itemsPickpocketedEventSink);
            logger::debug("EventEnum_OnItemsPickpocketed sink added");
        }
        break;

    case EventEnum_OnLocationCleared:
        if (!locationClearedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnLocationCleared]->isEmpty()) {
            auto* locationClearedEventSource = RE::LocationCleared::GetEventSource();
            if (!locationClearedEventSource) {
                logger::error("locationClearedEventSource not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            locationClearedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnLocationCleared]->sinkAdded = true;
            locationClearedEventSource->AddEventSink(locationClearedEventSink);
            logger::debug("EventEnum_OnLocationCleared sink added");
        }
        break;

    case EventEnum_OnEnterBleedout:
        if (!enterBleedoutEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnEnterBleedout]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            enterBleedoutEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnEnterBleedout]->sinkAdded = true;
            eventSourceholder->AddEventSink(enterBleedoutEventSink);
            logger::debug("EventEnum_OnEnterBleedout sink added");
        }
        break;

    case EventEnum_OnSwitchRaceComplete:
        if (!switchRaceCompleteEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnSwitchRaceComplete]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
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
            auto* footstepManager = RE::BGSFootstepManager::GetSingleton();
            if (!footstepManager) {
                logger::error("footstepManager not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            footstepEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnActorFootStep]->sinkAdded = true;
            footstepManager->AddEventSink(footstepEventSink);
            logger::debug("EventEnum_OnActorFootStep sink added");
        }
        break;

    case EventEnum_OnQuestObjectiveStateChanged:
        if (!questObjectiveEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->isEmpty()) {
            auto* objectiveStateEventSource = RE::ObjectiveState::GetEventSource();
            if (!objectiveStateEventSource) {
                logger::error("objectiveStateEventSource not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            questObjectiveEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->sinkAdded = true;
            objectiveStateEventSource->AddEventSink(questObjectiveEventSink);
            logger::debug("EventEnum_OnQuestObjectiveStateChanged sink added");
        }
        break;

    case EventEnum_OnPositionPlayerStart:

    case EventEnum_OnPositionPlayerFinish:
        if (!positionPlayerEventSink->sinkAdded && ShouldPositionPlayerEventSinkBeAdded()) {
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (player) {
                auto* posPlayerEventSource = player->AsPositionPlayerEventSource();
                if (posPlayerEventSource) {
                    positionPlayerEventSink->sinkAdded = true;
                    posPlayerEventSource->AddEventSink(positionPlayerEventSink);
                    logger::debug("positionPlayerEventSink added");
                }
                else {
                    logger::error("posPlayerEventSource not found. Index[{}] Event[{}] not added.", index, eventName);
                }
            }
            else {
                logger::error("player* not found. Index[{}] Event[{}] not added.", index, eventName);
            }
        }
        break;

    case EventEnum_OnPlayerChangeCell:
        if (!actorCellEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnPlayerChangeCell]->isEmpty()) {
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (player) {
                auto* playerCellChangeSource = player->AsBGSActorCellEventSource();
                if (playerCellChangeSource) {
                    actorCellEventSink->sinkAdded = true;
                    eventDataPtrs[EventEnum_OnPlayerChangeCell]->sinkAdded = true;
                    //actorCellEventSink->previousCell = playerRef->GetParentCell();
                    playerCellChangeSource->AddEventSink(actorCellEventSink);
                    logger::debug("EventEnum_OnPlayerChangeCell sink added");
                }
                else {
                    logger::error("playerCellChangeSource not found. Index[{}] Event[{}] not added.", index, eventName);
                }
            }
            else {
                logger::error("player* not found. Index[{}] Event[{}] not added.", index, eventName);
            }
        }
        break;

        //ActiveEffectEvents
    case EventEnum_OnEffectStart:
        if (!ActiveEffectEvents::effectStartRegistered && !eventDataPtrs[EventEnum_OnEffectStart]->isEmpty()) {
            ActiveEffectEvents::effectStartRegistered = true;
            ActiveEffectEvents::EffectStartEvent::Install(); // both needed for the elapsed time parameters
            ActiveEffectEvents::EffectFinishEvent::Install();
            logger::debug("EventEnum_OnEffectStart sink added");
        }
        break;

    case EventEnum_OnEffectFinish:
        if (!ActiveEffectEvents::effectFinishRegistered && !eventDataPtrs[EventEnum_OnEffectFinish]->isEmpty()) {
            ActiveEffectEvents::effectFinishRegistered = true;
            ActiveEffectEvents::EffectStartEvent::Install(); // both needed for the elapsed time parameters
            ActiveEffectEvents::EffectFinishEvent::Install();
            logger::debug("EventEnum_OnEffectFinish sink added");
        }
        break;

    case EventEnum_OnMusicTypeChange:
        if (!eventDataPtrs[EventEnum_OnMusicTypeChange]->isEmpty()) {
            if (musicChangeEventSink->AddEventSink()) {
                logger::debug("EventEnum_OnMusicTypeChange sink added");
            }
        }
        break;

    case EventEnum_OnWeatherChange:
        if (!eventDataPtrs[EventEnum_OnWeatherChange]->isEmpty()) {
            if (weatherChangeEventSink->AddEventSink()) {
                logger::debug("EventEnum_OnWeatherChange sink added");
            }
        }
        break;

    case EventEnum_OnPerkEntryRun:
        if (!perkEntryRunEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnPerkEntryRun]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            perkEntryRunEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnPerkEntryRun]->sinkAdded = true;
            eventSourceholder->AddEventSink(perkEntryRunEventSink);
            logger::debug("EventEnum_OnPerkEntryRun sink added");
        }
        break;

    case EventEnum_OnTriggerEnter:
        if (!triggerEnterEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnTriggerEnter]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            triggerEnterEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnTriggerEnter]->sinkAdded = true;
            eventSourceholder->AddEventSink(triggerEnterEventSink);
            logger::debug("EventEnum_OnTriggerEnter sink added");
        }
        break;

    case EventEnum_OnTriggerLeave:
        if (!triggerLeaveEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnTriggerLeave]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            triggerLeaveEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnTriggerLeave]->sinkAdded = true;
            eventSourceholder->AddEventSink(triggerLeaveEventSink);
            logger::debug("EventEnum_OnTriggerLeave sink added");
        }
        break;

    case EventEnum_OnPackageStart:
    case EventEnum_OnPackageChange:
    case EventEnum_OnPackageEnd:
        if (!packageEventSink->sinkAdded && !eventDataPtrs[index]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            packageEventSink->sinkAdded = true;
            eventDataPtrs[index]->sinkAdded = true;
            eventSourceholder->AddEventSink(packageEventSink);
            logger::debug("EventEnum_OnPackage{} sink added", eventName);
        }
        break;

    case EventEnum_OnDestructionStageChanged:
        if (!destructionStageChangedEventSink->sinkAdded && !eventDataPtrs[EventEnum_OnDestructionStageChanged]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            destructionStageChangedEventSink->sinkAdded = true;
            eventDataPtrs[EventEnum_OnDestructionStageChanged]->sinkAdded = true;
            eventSourceholder->AddEventSink(destructionStageChangedEventSink);
            logger::debug("EventEnum_OnDestructionStageChanged sink added");
        }
        break;

    case EventEnum_OnTranslationFailed:
    case EventEnum_OnTranslationAlmostComplete:
    case EventEnum_OnTranslationComplete:
        if (!objectREFRTranslationEventSink->sinkAdded && !eventDataPtrs[index]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }
            objectREFRTranslationEventSink->sinkAdded = true;
            eventDataPtrs[index]->sinkAdded = true;
            eventSourceholder->AddEventSink(objectREFRTranslationEventSink);
            logger::debug("EventEnum_OnTranslation{} sink added", eventName);
        }
        break;
    }
}

void RemoveSink(int index) {
    auto* eventSourceholder = RE::ScriptEventSourceHolder::GetSingleton();
    std::string eventName = "not found";
    auto* eventData = eventDataPtrs[index];
    if (eventData) {
        eventName = std::string(eventData->sEvent);
    }
    logger::trace("Index[{}] Event[{}].", index, eventName);

    switch (index) {
    case EventEnum_OnCombatStateChanged:
        if (combatEventSink->sinkAdded && eventDataPtrs[EventEnum_OnCombatStateChanged]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            combatEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnCombatStateChanged]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(combatEventSink);
            logger::debug("EventEnum_OnCombatStateChanged sink removed");
        }
        break;

    case EventEnum_FurnitureEnter:

    case EventEnum_FurnitureExit:
        if (furnitureEventSink->sinkAdded && eventDataPtrs[EventEnum_FurnitureEnter]->isEmpty() && eventDataPtrs[EventEnum_FurnitureExit]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            furnitureEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_FurnitureEnter]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(furnitureEventSink);
            logger::debug("EventEnum_FurnitureEnter sink removed");
        }
        break;

    case EventEnum_OnActivate:
        if (eventDataPtrs[EventEnum_OnActivate]->sinkAdded && eventDataPtrs[EventEnum_OnActivate]->isEmpty()) {
            eventDataPtrs[EventEnum_OnActivate]->sinkAdded = false;

            if (!eventSourceholder && activateEventSink->sinkAdded && !bActivateEventSinkEnabledByDefault) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not added.", index, eventName);
                return;
            }

            //only remove if not added by default to track last player activated reference
            if (activateEventSink->sinkAdded && !bActivateEventSinkEnabledByDefault) {
                activateEventSink->sinkAdded = false;
                eventSourceholder->RemoveEventSink(activateEventSink);
            }

            eventDataPtrs[EventEnum_OnActivate]->sinkAdded = false;

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
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            eventDataPtrs[EventEnum_HitEvent]->sinkAdded = false;
            logger::debug("EventEnum_hitEvent sink removed");
        }
        break;

    case EventEnum_DeathEvent:

    case EventEnum_DyingEvent:
        if (deathEventSink->sinkAdded && eventDataPtrs[EventEnum_DeathEvent]->isEmpty() && eventDataPtrs[EventEnum_DyingEvent]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            deathEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_DeathEvent]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(deathEventSink);
            logger::debug("EventEnum_DeathEvent sink removed");
        }
        break;

    case EventEnum_OnObjectEquipped:
        if (eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded && eventDataPtrs[EventEnum_OnObjectEquipped]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            eventDataPtrs[EventEnum_OnObjectEquipped]->sinkAdded = false;
            logger::debug("EventEnum_OnObjectEquipped sink removed");
        }
        if (RemoveEquipEventSink()) {
            logger::debug("EventEnum_OnObjectEquipped equip event sink removed");
        }
        break;

    case EventEnum_OnObjectUnequipped:
        if (eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded && eventDataPtrs[EventEnum_OnObjectUnequipped]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            eventDataPtrs[EventEnum_OnObjectUnequipped]->sinkAdded = false;
            logger::debug("EventEnum_OnObjectUnequipped sink removed");
        }
        if (RemoveEquipEventSink()) {
            logger::debug("EventEnum_OnObjectUnequipped equip event sink removed");
        }
        break;

    case EventEnum_OnWaitStart:
        if (waitStartEventSink->sinkAdded && eventDataPtrs[EventEnum_OnWaitStart]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            waitStartEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnWaitStart]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(waitStartEventSink);
            logger::debug("EventEnum_OnWaitStart sink removed");
        }
        break;

    case EventEnum_OnWaitStop:
        if (waitStopEventSink->sinkAdded && eventDataPtrs[EventEnum_OnWaitStop]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            waitStopEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnWaitStop]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(waitStopEventSink);
            logger::debug("EventEnum_OnWaitStop sink removed");
        }
        break;

    case EventEnum_OnMagicEffectApply:
        if (magicEffectApplyEventSink->sinkAdded && eventDataPtrs[EventEnum_OnMagicEffectApply]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            magicEffectApplyEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnMagicEffectApply]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(magicEffectApplyEventSink);
            logger::debug("EventEnum_OnMagicEffectApply sink removed");
        }
        break;

    case EventEnum_OnSpellCast:
        if (spellCastEventSink->sinkAdded && eventDataPtrs[EventEnum_OnSpellCast]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            spellCastEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnSpellCast]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(spellCastEventSink);
            logger::debug("EventEnum_OnSpellCast sink removed");
        }
        break;

    case EventEnum_LockChanged:
        if (lockChangedEventSink->sinkAdded && eventDataPtrs[EventEnum_LockChanged]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            lockChangedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_LockChanged]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(lockChangedEventSink);
            logger::debug("EventEnum_LockChanged sink removed");
        }
        break;

    case EventEnum_OnOpen:

    case EventEnum_OnClose:
        if (openCloseEventSink->sinkAdded && eventDataPtrs[EventEnum_OnOpen]->isEmpty() && eventDataPtrs[EventEnum_OnClose]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
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
            auto* actionEventSource = SKSE::GetActionEventSource();
            if (!eventSourceholder) {
                logger::error("ActionEventSource not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            actorActionEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_EndSheathe]->sinkAdded = false;
            actionEventSource->RemoveEventSink(actorActionEventSink);
            logger::debug("actorActionEventSink sink removed");
        }
        break;

    case EventEnum_OnContainerChanged:
        if (containerChangedEventSink->sinkAdded && eventDataPtrs[EventEnum_OnContainerChanged]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            containerChangedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnContainerChanged]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(containerChangedEventSink);
            logger::debug("EventEnum_OnContainerChanged sink removed");
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
            auto* itemCraftedEventSource = RE::ItemCrafted::GetEventSource();
            if (!itemCraftedEventSource) {
                logger::error("itemCraftedEventSource not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            itemCraftedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnItemCrafted]->sinkAdded = false;
            itemCraftedEventSource->RemoveEventSink(itemCraftedEventSink);
            logger::debug("EventEnum_OnItemCrafted sink removed");
        }
        break;

    case EventEnum_OnItemsPickpocketed:
        if (itemsPickpocketedEventSink->sinkAdded && eventDataPtrs[EventEnum_OnItemsPickpocketed]->isEmpty()) {
            auto* itemPickpocketedEventSource = RE::ItemsPickpocketed::GetEventSource();
            if (!itemPickpocketedEventSource) {
                logger::error("itemPickpocketedEventSource not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            itemsPickpocketedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnItemsPickpocketed]->sinkAdded = false;
            itemPickpocketedEventSource->RemoveEventSink(itemsPickpocketedEventSink);
            logger::debug("EventEnum_OnItemsPickpocketed sink removed");
        }
        break;

    case EventEnum_OnLocationCleared:
        if (locationClearedEventSink->sinkAdded && eventDataPtrs[EventEnum_OnLocationCleared]->isEmpty()) {
            auto* locationClearedEventSource = RE::LocationCleared::GetEventSource();
            if (!locationClearedEventSource) {
                logger::error("locationClearedEventSource not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            locationClearedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnLocationCleared]->sinkAdded = false;
            locationClearedEventSource->RemoveEventSink(locationClearedEventSink);
            logger::debug("EventEnum_OnLocationCleared sink removed");
        }
        break;

    case EventEnum_OnEnterBleedout:
        if (enterBleedoutEventSink->sinkAdded && eventDataPtrs[EventEnum_OnEnterBleedout]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            enterBleedoutEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnEnterBleedout]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(enterBleedoutEventSink);
            logger::debug("EventEnum_OnEnterBleedout sink removed");
        }
        break;

    case EventEnum_OnSwitchRaceComplete:
        if (switchRaceCompleteEventSink->sinkAdded && eventDataPtrs[EventEnum_OnSwitchRaceComplete]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
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
            auto* footstepManager = RE::BGSFootstepManager::GetSingleton();
            if (!footstepManager) {
                logger::error("footstepManager not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            footstepEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnActorFootStep]->sinkAdded = false;
            footstepManager->RemoveEventSink(footstepEventSink);
            logger::debug("EventEnum_OnActorFootStep sink removed");
        }
        break;

    case EventEnum_OnQuestObjectiveStateChanged:
        if (questObjectiveEventSink->sinkAdded && eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->isEmpty()) {
            auto* objectiveStateEventSource = RE::ObjectiveState::GetEventSource();
            if (!objectiveStateEventSource) {
                logger::error("objectiveStateEventSource not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            questObjectiveEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnQuestObjectiveStateChanged]->sinkAdded = false;
            objectiveStateEventSource->RemoveEventSink(questObjectiveEventSink);
            logger::debug("EventEnum_OnQuestObjectiveStateChanged sink removed");
        }
        break;

    case EventEnum_OnPositionPlayerStart:

    case EventEnum_OnPositionPlayerFinish:
        if (positionPlayerEventSink->sinkAdded && !ShouldPositionPlayerEventSinkBeAdded()) {
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (player) {
                auto posPlayerEventSource = player->AsPositionPlayerEventSource();
                if (posPlayerEventSource) {
                    positionPlayerEventSink->sinkAdded = false;
                    posPlayerEventSource->RemoveEventSink(positionPlayerEventSink);
                    logger::debug("positionPlayerEventSink removed");
                }
                else {
                    logger::error("posPlayerEventSource not found. Index[{}] Event[{}] not removed.", index, eventName);
                }
            }
            else {
                logger::error("player* not found. Index[{}] Event[{}] not removed.", index, eventName);
            }
        }
        break;

    case EventEnum_OnPlayerChangeCell:
        if (actorCellEventSink->sinkAdded && eventDataPtrs[EventEnum_OnPlayerChangeCell]->isEmpty()) {
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (player) {
                auto* playerCellChangeSource = player->AsBGSActorCellEventSource();
                if (playerCellChangeSource) {
                    actorCellEventSink->sinkAdded = false;
                    eventDataPtrs[EventEnum_OnPlayerChangeCell]->sinkAdded = false;
                    playerCellChangeSource->RemoveEventSink(actorCellEventSink);
                    logger::debug("EventEnum_OnPlayerChangeCell sink removed");
                }
                else {
                    logger::error("playerCellChangeSource not found. Index[{}] Event[{}] not removed.", index, eventName);
                }
            }
            else {
                logger::error("player* not found. Index[{}] Event[{}] not removed.", index, eventName);
            }
        }
        break;

    case EventEnum_OnEffectStart:
        if (ActiveEffectEvents::effectStartRegistered && eventDataPtrs[EventEnum_OnEffectStart]->isEmpty()) {
            ActiveEffectEvents::effectStartRegistered = false;
            logger::debug("EventEnum_OnEffectStart sink removed");
        }
        break;

    case EventEnum_OnEffectFinish:
        if (ActiveEffectEvents::effectFinishRegistered && eventDataPtrs[EventEnum_OnEffectFinish]->isEmpty()) {
            ActiveEffectEvents::effectFinishRegistered = false;
            logger::debug("EventEnum_OnEffectFinish sink removed");
        }
        break;

    case EventEnum_OnMusicTypeChange:
        if (eventDataPtrs[EventEnum_OnMusicTypeChange]->isEmpty()) {
            if (musicChangeEventSink->RemoveEventSink()) {
                logger::debug("EventEnum_OnMusicTypeChange sink removed");
            }
        }
        break;

    case EventEnum_OnWeatherChange:
        if (eventDataPtrs[EventEnum_OnWeatherChange]->isEmpty()) {
            if (weatherChangeEventSink->RemoveEventSink()) {
                logger::debug("EventEnum_OnWeatherChange sink removed");
            }
        }
        break;

    //New Events ===================================================================================================================
    case EventEnum_OnPerkEntryRun:
        if (perkEntryRunEventSink->sinkAdded && eventDataPtrs[EventEnum_OnPerkEntryRun]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            perkEntryRunEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnPerkEntryRun]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(perkEntryRunEventSink);
            logger::debug("EventEnum_OnPerkEntryRun sink removed");
        }
        break;

    case EventEnum_OnTriggerEnter:
        if (triggerEnterEventSink->sinkAdded && eventDataPtrs[EventEnum_OnTriggerEnter]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            triggerEnterEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnTriggerEnter]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(triggerEnterEventSink);
            logger::debug("EventEnum_OnTriggerEnter sink removed");
        }
        break;

    case EventEnum_OnTriggerLeave:
        if (triggerLeaveEventSink->sinkAdded && eventDataPtrs[EventEnum_OnTriggerLeave]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            triggerLeaveEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnTriggerLeave]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(triggerLeaveEventSink);
            logger::debug("EventEnum_OnTriggerLeave sink removed");
        }
        break;

    case EventEnum_OnPackageStart:
    case EventEnum_OnPackageChange:
    case EventEnum_OnPackageEnd:
        if (packageEventSink->sinkAdded && 
            eventDataPtrs[EventEnum_OnPackageStart]->isEmpty() && 
            eventDataPtrs[EventEnum_OnPackageChange]->isEmpty() &&
            eventDataPtrs[EventEnum_OnPackageEnd]->isEmpty()) {

            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            packageEventSink->sinkAdded = false;
            eventDataPtrs[index]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(packageEventSink);
            logger::debug("EventEnum_OnPackage{} sink removed", eventName);
        }
        break;

    case EventEnum_OnDestructionStageChanged:
        if (destructionStageChangedEventSink->sinkAdded && eventDataPtrs[EventEnum_OnDestructionStageChanged]->isEmpty()) {
            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            destructionStageChangedEventSink->sinkAdded = false;
            eventDataPtrs[EventEnum_OnDestructionStageChanged]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(destructionStageChangedEventSink);
            logger::debug("EventEnum_OnDestructionStageChanged sink removed");
        }
        break;

    case EventEnum_OnTranslationFailed:
    case EventEnum_OnTranslationAlmostComplete:
    case EventEnum_OnTranslationComplete:
        if (objectREFRTranslationEventSink->sinkAdded &&
            eventDataPtrs[EventEnum_OnTranslationFailed]->isEmpty() &&
            eventDataPtrs[EventEnum_OnTranslationAlmostComplete]->isEmpty() &&
            eventDataPtrs[EventEnum_OnTranslationComplete]->isEmpty()) {

            if (!eventSourceholder) {
                logger::error("eventSourceholder not found. Index[{}] Event[{}] not removed.", index, eventName);
                return;
            }
            objectREFRTranslationEventSink->sinkAdded = false;
            eventDataPtrs[index]->sinkAdded = false;
            eventSourceholder->RemoveEventSink(objectREFRTranslationEventSink);
            logger::debug("EventEnum_OnTranslation{} sink removed", eventName);
        }
        break;
    }
}

int GetEventIndex(std::vector<EventData*>& v, RE::BSFixedString asEvent) {
    if (asEvent == "") {
        return -1;
    }

    auto size = v.size();

    if (size == 0) {
        return -1;
    }

    for (int i = 0; i < size; i++) {
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

    list->ForEachForm([&](RE::TESForm* form) {
        if (gfuncs::IsFormValid(form)) {
            iCount++;
            RE::BGSListForm* nestedList = form->As<RE::BGSListForm>();
            if (gfuncs::IsFormValid(nestedList)) {
                RegisterFormListForGlobalEvent(nestedList, eventIndex, paramFilterIndex, handle);
            }
            else {
                std::string formName = std::string(gfuncs::GetFormName(form));
                auto id = form->GetFormID();
                eventDataPtrs[eventIndex]->AddHandle(handle, form, paramFilterIndex, formName, id);
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

    list->ForEachForm([&](auto* form) {
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

    std::string eventReceiverName = std::string(gfuncs::GetFormName(eventReceiver));
    logger::trace("adding handle for: {}", eventReceiverName);

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
    RE::FormID eventReveiverId = eventReceiver->GetFormID();

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", eventReceiverName, eventReveiverId);
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
            eventDataPtrs[index]->AddHandle(handle, paramFilter, paramFilterIndex, eventReceiverName, eventReveiverId);
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

    std::string eventReceiverName = std::string(eventReceiver->aliasName);

    logger::trace("adding handle for: {}", eventReceiverName);

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for alias [{}] ID [{:x}]", eventReceiverName, eventReceiver->GetVMTypeID());
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
            eventDataPtrs[index]->AddHandle(handle, paramFilter, paramFilterIndex, eventReceiverName, eventReceiver->aliasID);
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

    RE::TESForm* baseForm = eventReceiver->GetBaseObject();
    std::string eventReceiverName = std::string(gfuncs::GetFormName(baseForm));
    RE::FormID eventReceiverId; 
    if (gfuncs::IsFormValid(baseForm)) {
        eventReceiverId = baseForm->GetFormID();
    }

    logger::trace("adding handle for: {} instance", eventReceiverName);

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for effect [{}] ID [{:x}]", eventReceiverName, eventReceiver->VMTYPEID);
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
            eventDataPtrs[index]->AddHandle(handle, paramFilter, paramFilterIndex, eventReceiverName, eventReceiverId);
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

void CreateEventSinks() {
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
        eventDataPtrs[EventEnum_OnEffectStart] = new EventData("OnEffectStartGlobal", EventEnum_OnEffectStart, 4, 'OEs0');
        eventDataPtrs[EventEnum_OnEffectFinish] = new EventData("OnEffectFinishGlobal", EventEnum_OnEffectFinish, 4, 'OEs1');
        eventDataPtrs[EventEnum_OnMusicTypeChange] = new EventData("OnMusicTypeChangeGlobal", EventEnum_OnMusicTypeChange, 2, 'MTc0');
        eventDataPtrs[EventEnum_OnWeatherChange] = new EventData("OnWeatherChangeGlobal", EventEnum_OnWeatherChange, 2, 'OWC0');
        eventDataPtrs[EventEnum_OnPerkEntryRun] = new EventData("OnPerkEntryRunGlobal", EventEnum_OnPerkEntryRun, 3, 'PEr0');
        eventDataPtrs[EventEnum_OnTriggerEnter] = new EventData("OnTriggerEnterGlobal", EventEnum_OnTriggerEnter, 2, 'TrE0');
        eventDataPtrs[EventEnum_OnTriggerLeave] = new EventData("OnTriggerLeaveGlobal", EventEnum_OnTriggerLeave, 2, 'TrL0');
        eventDataPtrs[EventEnum_OnPackageStart] = new EventData("OnPackageStartGlobal", EventEnum_OnPackageStart, 2, 'PaS0');
        eventDataPtrs[EventEnum_OnPackageChange] = new EventData("OnPackageChangeGlobal", EventEnum_OnPackageChange, 2, 'PaC0');
        eventDataPtrs[EventEnum_OnPackageEnd] = new EventData("OnPackageEndGlobal", EventEnum_OnPackageEnd, 2, 'PaE0');
        eventDataPtrs[EventEnum_OnDestructionStageChanged] = new EventData("OnDestructionStageChangedGlobal", EventEnum_OnDestructionStageChanged, 1, 'DSC0');
        eventDataPtrs[EventEnum_OnTranslationFailed] = new EventData("OnTranslationFailedGlobal", EventEnum_OnTranslationFailed, 1, 'OTr0');
        eventDataPtrs[EventEnum_OnTranslationAlmostComplete] = new EventData("OnTranslationAlmostCompleteGlobal", EventEnum_OnTranslationAlmostComplete, 1, 'OTr1');
        eventDataPtrs[EventEnum_OnTranslationComplete] = new EventData("OnTranslationCompleteGlobal", EventEnum_OnTranslationComplete, 1, 'OTr2');
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
    if (!musicChangeEventSink) { musicChangeEventSink = new MusicChangeEventSink(); }
    if (!weatherChangeEventSink) { weatherChangeEventSink = new WeatherChangeEventSink(); }
    if (!perkEntryRunEventSink) { perkEntryRunEventSink = new PerkEntryRunEventSink(); }
    if (!triggerEnterEventSink) { triggerEnterEventSink = new TriggerEnterEventSink(); }
    if (!triggerLeaveEventSink) { triggerLeaveEventSink = new TriggerLeaveEventSink(); }
    if (!packageEventSink) { packageEventSink = new PackageEventSink(); }
    if (!destructionStageChangedEventSink) { destructionStageChangedEventSink = new DestructionStageChangedEventSink(); }
    if (!objectREFRTranslationEventSink) { objectREFRTranslationEventSink = new ObjectREFRTranslationEventSink(); }

    auto* eventSourceholder = RE::ScriptEventSourceHolder::GetSingleton();

    if (!activateEventSink->sinkAdded && bActivateEventSinkEnabledByDefault) {
        if (eventSourceholder) {
            //always active to track lastPlayerActivatedRef
            activateEventSink->sinkAdded = true;
            eventSourceholder->AddEventSink(activateEventSink);
        }
        else {
            logger::error("eventSourceHolder not found. activateEventSink not added");
        }
    }

    if (!menuOpenCloseEventSink->sinkAdded && bMenuOpenCloseEventSinkEnabled) {
        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            menuOpenCloseEventSink->sinkAdded = true;
            //always active to track opened menus / game pausing
            ui->AddEventSink<RE::MenuOpenCloseEvent>(menuOpenCloseEventSink);
        }
        else {
            logger::error("ui* not found. menuOpenCloseEventSink not added");
        }
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

    //RE::BSScript::IVirtualMachine;

    gvm = vm;

    //functions 
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
    vm->RegisterFunction("GetEnableChildrenRefs", "DbSkseFunctions", GetEnableChildrenRefs);
    vm->RegisterFunction("GetAllContainerRefsThatContainForm", "DbSkseFunctions", GetAllContainerRefsThatContainForm);
    vm->RegisterFunction("GetAllFormsThatUseTextureSet", "DbSkseFunctions", GetAllFormsThatUseTextureSet);
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
    vm->RegisterFunction("GetCurrentMusicType", "DbSkseFunctions", GetCurrentMusicType);
    vm->RegisterFunction("GetNumberOfTracksInMusicType", "DbSkseFunctions", GetNumberOfTracksInMusicType);
    vm->RegisterFunction("GetMusicTypeTrackIndex", "DbSkseFunctions", GetMusicTypeTrackIndex);
    vm->RegisterFunction("SetMusicTypeTrackIndex", "DbSkseFunctions", SetMusicTypeTrackIndex);
    vm->RegisterFunction("GetMusicTypePriority", "DbSkseFunctions", GetMusicTypePriority);
    vm->RegisterFunction("SetMusicTypePriority", "DbSkseFunctions", SetMusicTypePriority);
    vm->RegisterFunction("GetMusicTypeStatus", "DbSkseFunctions", GetMusicTypeStatus);
    vm->RegisterFunction("GetKnownEnchantments", "DbSkseFunctions", GetKnownEnchantments);
    vm->RegisterFunction("AddKnownEnchantmentsToFormList", "DbSkseFunctions", AddKnownEnchantmentsToFormList);
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
    vm->RegisterFunction("GetFormWorldModelNth3dName", "DbSkseFunctions", GetFormWorldModelNth3dName);
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

int GetActiveMagicEffectConditionStatus(RE::ActiveEffect* akEffect) {
    if (!akEffect) {
        return -1;
    }

    if (akEffect->conditionStatus.any(RE::ActiveEffect::ConditionStatus::kTrue)) {
        //logger::trace("conditionStatus is kTrue");
        return 1;
    }
    else if (akEffect->conditionStatus.any(RE::ActiveEffect::ConditionStatus::kFalse)) {
        //logger::trace("conditionStatus is kFalse");
        return 0;
    }
    else {
        return -1;
    }
}

void MessageListener(SKSE::MessagingInterface::Message* message) {
    switch (message->type) {
        // Descriptions are taken from the original skse64 library
        // See:
        // https://github.com/ianpatt/skse64/blob/09f520a2433747f33ae7d7c15b1164ca198932c3/skse64/PluginAPI.h#L193-L212
         //case SKSE::MessagingInterface::kPostLoad: //
            //    logger::info("kPostLoad: sent to registered plugins once all plugins have been loaded");
            //    break;

        //case SKSE::MessagingInterface::kPostPostLoad: {
        //    logger::info(
        //        "kPostPostLoad: sent right after kPostLoad to facilitate the correct dispatching/registering of "
        //        "messages/listeners");

        //    
        //    break; //
        //}
        // 
        //case SKSE::MessagingInterface::kPreLoadGame:
            //    // message->dataLen: length of file path, data: char* file path of .ess savegame file
            //    logger::info("kPreLoadGame: sent immediately before savegame is read");
            //    break;

        case SKSE::MessagingInterface::kPostLoadGame: {
            // You will probably want to handle this event if your plugin uses a Preload callback
            // as there is a chance that after that callback is invoked the game will encounter an error
            // while loading the saved game (eg. corrupted save) which may require you to reset some of your
            // plugin state.
            //SendLoadGameEvent();
            //CreateEventSinks();
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (player) {
                bPlayerIsInCombat = player->IsInCombat();
            }
            logger::trace("kPostLoadGame: sent after an attempt to load a saved game has finished");

            break;
        }

        //case SKSE::MessagingInterface::kSaveGame:
            //    logger::info("kSaveGame");
            //    break;

        //case SKSE::MessagingInterface::kDeleteGame:
            //    // message->dataLen: length of file path, data: char* file path of .ess savegame file
            //    logger::info("kDeleteGame: sent right before deleting the .skse cosave and the .ess save");
            //    break;

        //case SKSE::MessagingInterface::kInputLoaded: {
        //    logger::info("kInputLoaded: sent right after game input is loaded, right before the main menu initializes");
        //    //
        //    break;
        //}

        case SKSE::MessagingInterface::kNewGame: {
            RegisterActorsForBowDrawAnimEvents();
            logger::trace("kNewGame: sent after a new game is created, before the game has loaded");
            break;
        }
        case SKSE::MessagingInterface::kDataLoaded: {
            // RE::ConsoleLog::GetSingleton()->Print("DbSkseFunctions Installed");
            if (!nullForm) { nullForm = gfuncs::FindNullForm(); }
            gfuncs::Install();

            auto* papyrusInterface = SKSE::GetPapyrusInterface();
            papyrusInterface->Register(BindPapyrusFunctions);
            papyrusInterface->Register(timers::BindPapyrusFunctions);
            papyrusInterface->Register(gfx::BindPapyrusFunctions);
            papyrusInterface->Register(cell::BindPapyrusFunctions);
            papyrusInterface->Register(biped::BindPapyrusFunctions);
            papyrusInterface->Register(animation::BindPapyrusFunctions);
            papyrusInterface->Register(objectRef::BindPapyrusFunctions);
            papyrusInterface->Register(actor::BindPapyrusFunctions);
            papyrusInterface->Register(alias::BindPapyrusFunctions);
            papyrusInterface->Register(furniture::BindPapyrusFunctions);
            papyrusInterface->Register(keyword::BindPapyrusFunctions);
            papyrusInterface->Register(magic::BindPapyrusFunctions);
            papyrusInterface->Register(papyrusUtilEx::BindPapyrusFunctions);
            papyrusInterface->Register(conditions::BindPapyrusFunctions);
            papyrusInterface->Register(rangeEvents::BindPapyrusFunctions);
            
            SetSettingsFromIniFile();
            CreateEventSinks();
            SaveSkillBooks();

            logger::trace("kDataLoaded: sent after the data handler has loaded all its forms");
            break;
        }
    }
}

void LoadCallback(SKSE::SerializationInterface* ssi) {
    //gfuncs::lastTimeGameWasLoaded = std::chrono::system_clock::now();

    logger::trace("LoadCallback started");

    int max = EventEnum_Last + 1;
    std::uint32_t type, version, length;

    if (ssi) {
        if (serialize::SetSerializing(true)) {
            while (ssi->GetNextRecordInfo(type, version, length)) {
                logger::trace("type[{}]", gfuncs::uint32_to_string(type));

                if (type == eventDataPtrs[EventEnum_OnWeatherChange]->record) {
                    if (weatherChangeEventSink) {
                        if (weatherChangeEventSink->IsSinkAdded()) {
                            RE::TESWeather* currentWeather = serialize::LoadForm<RE::TESWeather>(ssi);
                            weatherChangeEventSink->SetCurrentWeather(currentWeather);
                            logger::trace("weatherChangeEventSink currentWeather set to[{}]", gfuncs::GetFormNameAndId(currentWeather));
                        }
                    }
                    else {
                        logger::warn("weatherChangeEventSink is nullptr");
                    }
                } 
                else if (type == eventDataPtrs[EventEnum_OnMusicTypeChange]->record) {
                    if (musicChangeEventSink) {
                        if (musicChangeEventSink->IsSinkAdded()) {
                            RE::BGSMusicType* currentmusic = serialize::LoadForm<RE::BGSMusicType>(ssi);
                            musicChangeEventSink->SetCurrentMusic(currentmusic);
                            logger::trace("musicChangeEventSink currentmusic set to[{}]", gfuncs::GetFormNameAndId(currentmusic));
                        }
                    }
                    else {
                        logger::warn("weatherChangeEventSink is nullptr");
                    }
                }
                else if (timers::IsTimerType(type)) {
                    timers::LoadTimers(type, ssi);
                }
                else {
                    //this is causing ctd on Skyrim AE when fast traveling too many times too quickly.
                    /*for (int i = EventEnum_First; i < max; i++) {
                        if (type == eventDataPtrs[i]->record) {
                            eventDataPtrs[i]->Load(ssi);
                            break;
                        }
                    }*/
                }
            }

            auto* player = RE::PlayerCharacter::GetSingleton();
            if (player) {
                bPlayerIsInCombat = player->IsInCombat();
            }

            //EventEnum_OnLoadGame doesn't have an event sink, hence EventEnum_First + 1
            /*for (int i = EventEnum_First + 1; i < max; i++) {
                AddSink(i);
            }*/

            //now sending to all scripts with the OnLoadGameGlobal Event
            //gfuncs::RemoveDuplicates(eventDataPtrs[EventEnum_OnLoadGame]->globalHandles);
            //gfuncs::SendEvents(eventDataPtrs[EventEnum_OnLoadGame]->globalHandles, eventDataPtrs[EventEnum_OnLoadGame]->sEvent, args);

            auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton(); 
            if (vm) {
                auto* args = RE::MakeFunctionArguments();
                vm->SendEventAll(eventDataPtrs[EventEnum_OnLoadGame]->sEvent, args);
                delete args;
            }

            RegisterActorsForBowDrawAnimEvents();
            SaveActorRaces();
            serialize::SetSerializing(false);
            logger::trace("LoadCallback complete");
        }
        else {
            logger::debug("already loading or saving");
        }
    }
    else {
        logger::error("ssi doesn't exist, aborting load.");
    }
}

void SaveCallback(SKSE::SerializationInterface* ssi) {
    logger::trace("SaveCallback started");

    if (ssi) {
        if (serialize::SetSerializing(true)) {
            //this is causing ctd on Skyrim AE when fast traveling too many times too quickly.
            /*int max = EventEnum_Last + 1;
            for (int i = EventEnum_First; i < max; i++) {
                eventDataPtrs[i]->Save(ssi);
            }*/

            timers::SaveTimers(ssi);

            if (weatherChangeEventSink) {
                if (weatherChangeEventSink->IsSinkAdded()) {
                    auto* currentWeather = weatherChangeEventSink->GetCurrentWeather();
                    serialize::SaveForm(currentWeather, eventDataPtrs[EventEnum_OnWeatherChange]->record, ssi);
                }
            }
            else {
                logger::warn("weatherChangeEventSink is nullptr");
            }

            if (musicChangeEventSink) {
                if (musicChangeEventSink->IsSinkAdded()) {
                    auto* currentmusic = musicChangeEventSink->GetCurrentMusic();
                    serialize::SaveForm(currentmusic, eventDataPtrs[EventEnum_OnMusicTypeChange]->record, ssi);
                }
            }
            else {
                logger::warn("musicChangeEventSink is nullptr");
            }

            serialize::SetSerializing(false);
            logger::trace("SaveCallback complete");
        }
        else {
            logger::debug("already loading or saving");
        }
    }
    else {
        logger::error("ssi doesn't exist, aborting save.");
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

    //std::stringstream ss;
    //ss << std::hex << std::uppercase << reinterpret_cast<uintptr_t>(SaveCallback);
    //std::string sFunc = ss.str();
    //logger::info("SaveCallback address [{}]", sFunc);

    //replaced gfuncs::LogAndMessage with logger:: functions.
    //fs::ReplaceLogAndMessageFuncs();
    return true;
}