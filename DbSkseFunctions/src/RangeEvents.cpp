#include "RangeEvents.h"
#include "GeneralFunctions.h"
#include "ObjectReference.h"

//Events for when an objectReference moves in range or out of range of another objectReference

int eventPollingInterval = 500; //in milliseconds

namespace rangeEvents {
    RE::BSFixedString enterRangeEventName = "OnEnterRange";
    RE::BSFixedString leaveRangeEventName = "OnLeaveRange";
    std::mutex updateMutex;
    std::condition_variable updateCv;
    bool isEmpty = true;

    //forward declaration
    struct RefRangeEvent;
    bool EraseEvent(RefRangeEvent* event);

    struct RefRangeEvent {
        bool active;
        bool inRange = false;
        RE::TESObjectREFR* target;
        RE::TESObjectREFR* centerRef;
        RE::TESCondition* distanceCondition;
        std::vector<RE::VMHandle> registeredHandles;
        float fDistance;

        RefRangeEvent(RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
            //akTarget and akCenterRef is valid checked in the RegisterHandle function
            
            RE::TESCondition* condition = new RE::TESCondition;
            RE::TESConditionItem* conditionItem = new RE::TESConditionItem;
            conditionItem->data.functionData.function = RE::FUNCTION_DATA::FunctionID::kGetDistance;
            conditionItem->data.flags.opCode = static_cast<RE::CONDITION_ITEM_DATA::OpCode>(4); //'<'
            conditionItem->data.comparisonValue.f = distance;
            conditionItem->data.functionData.params[0] = akTarget;
            condition->head = conditionItem;
            distanceCondition = condition;
            target = akTarget;
            centerRef = akCenterRef;
            fDistance = distance;
            inRange = distanceCondition->IsTrue(centerRef, nullptr);
            startPolling();
        }
    private:
        void startPolling() {
            poll();
        }

        void poll() {
            std::thread t([=]() {
                if (!centerRef || !target) {
                    logger::error("centerRef[{}] and/or target[{}] is nullptr. Erasing event. Distance[{}]",
                        gfuncs::GetFormNameAndId(centerRef), gfuncs::GetFormNameAndId(target), fDistance);

                    rangeEvents::EraseEvent(this);
                }
                else {
                    isEmpty = false; //tell the update hook in plugin.cpp to notify the updateCv
                    bool akInRange = distanceCondition->IsTrue(centerRef, nullptr);
                    if (inRange != akInRange) {
                        inRange = akInRange;
                        handleEvent(inRange);
                    }
                    else {
                        {
                            std::unique_lock<std::mutex> lock(updateMutex);
                            // waiting
                            updateCv.wait(lock, [=] {
                                int status = ConditionStatus();
                                if (status == -1) {
                                    return true;
                                }
                                else {
                                    return (bool(status) != akInRange);
                                }
                            });
                        }
                         
                        if (ConditionStatus() == -1) {
                            logger::error("centerRef[{}] and/or target[{}] is nullptr. Erasing event. Distance[{}]",
                                gfuncs::GetFormNameAndId(centerRef), gfuncs::GetFormNameAndId(target), fDistance);

                            rangeEvents::EraseEvent(this);
                        }
                        else {
                            inRange = !inRange;
                            handleEvent(inRange);
                        }
                    }
                }
            });
            t.detach();
        }

        int ConditionStatus() {
            if (!centerRef) {
                return -1;
            }
            return int(distanceCondition->IsTrue(centerRef, nullptr));
        }

        void handleEvent(bool isInRange) {
            if (!centerRef || !target) {
                logger::error("centerRef[{}] and/or target[{}] is nullptr. Erasing event. Distance[{}]",
                    gfuncs::GetFormNameAndId(centerRef), gfuncs::GetFormNameAndId(target), fDistance);

                rangeEvents::EraseEvent(this);
                return;
            }

            if (registeredHandles.size() == 0) {
                rangeEvents::EraseEvent(this);
                return;
            }

            logger::trace("centerRef[{}] target[{}] distance[{}]",
                gfuncs::GetFormNameAndId(centerRef), gfuncs::GetFormNameAndId(target), fDistance);

            RE::TESObjectREFR* targetParam = nullptr;
            if (gfuncs::IsFormValid(target)) {
                targetParam = target;
            }

            RE::TESObjectREFR* centerRefParam = nullptr;
            if (gfuncs::IsFormValid(centerRef)) {
                centerRefParam = centerRef;
            }

            //it's possible either the target or centerRef is invalid if it's in a cell that's not loaded.

            if (targetParam || centerRefParam) {
                auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)targetParam, (RE::TESObjectREFR*)centerRefParam, (float)fDistance);
                if (isInRange) {
                    gfuncs::SendEvents(registeredHandles, enterRangeEventName, args);

                }
                else {
                    gfuncs::SendEvents(registeredHandles, leaveRangeEventName, args);
                }
            }
            startPolling();
        }
    };

    std::mutex rangeEventsMutex;
    std::vector< RefRangeEvent*> rangeEvents;

    RefRangeEvent* GetEvent(RE::VMHandle handle, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        rangeEventsMutex.lock();
        for (auto* event : rangeEvents) {
            if (event) {
                int i = gfuncs::GetIndexInVector(event->registeredHandles, handle);
                if (i != -1) {
                    if (akTarget == event->target && akCenterRef == event->centerRef && distance == event->fDistance) {
                        rangeEventsMutex.unlock();
                        return event;
                    }
                }
            }
        }

        rangeEventsMutex.unlock();
        return nullptr;
    }

    bool EraseEvent(RefRangeEvent* event) {
        if (event) {
            rangeEventsMutex.lock();

            event->registeredHandles.clear();

            auto it = std::find(rangeEvents.begin(), rangeEvents.end(), event);
            if (it != rangeEvents.end()) {
                rangeEvents.erase(it);
            }

            if (event->distanceCondition) {
                if (event->distanceCondition->head) {
                    event->distanceCondition->head->data.functionData.params[0] = nullptr;
                    delete event->distanceCondition->head;
                    event->distanceCondition->head = nullptr;
                }
                delete event->distanceCondition;
                event->distanceCondition = nullptr;
            }

            if (rangeEvents.size() == 0) {
                isEmpty = true;
            }

            rangeEventsMutex.unlock();
            return true;
        }
        return false;
    }

    bool IsHandleRegistered(RE::VMHandle handle, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        return (GetEvent(handle, akTarget, akCenterRef, distance) != nullptr);
    }

    bool RegisterHandle(RE::VMHandle handle, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        if (!IsHandleRegistered(handle, akTarget, akCenterRef, distance)) {
            rangeEventsMutex.lock();
            auto* event = new rangeEvents::RefRangeEvent(akTarget, akCenterRef, distance);
            event->registeredHandles.push_back(handle);
            rangeEvents.push_back(event);
            rangeEventsMutex.unlock();
            return true;
        }
        return false;
    }

    bool UnregisterHandle(RE::VMHandle handle, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        auto* event = GetEvent(handle, akTarget, akCenterRef, distance);
        if (event) {
            rangeEventsMutex.lock();
            auto it = std::find(event->registeredHandles.begin(), event->registeredHandles.end(), handle);
            if (it != event->registeredHandles.end()) {
                event->registeredHandles.erase(it);
            }
            return true;
        }
        return false;
    }

    int UnregisterHandle_All(RE::VMHandle handle) {
        rangeEventsMutex.lock();
        int count = 0;
        for (auto* event : rangeEvents) {
            if (event) {
                auto it = std::find(event->registeredHandles.begin(), event->registeredHandles.end(), handle);
                if (it != event->registeredHandles.end()) {
                    event->registeredHandles.erase(it);
                    count++;
                }
            }
        }
        rangeEventsMutex.unlock();
        return count;
    }

    int GetNumRangeEventsRegisteredOnHandle(RE::VMHandle handle) {
        rangeEventsMutex.lock();
        int count = 0;
        for (auto* event : rangeEvents) {
            if (event) {
                auto it = std::find(event->registeredHandles.begin(), event->registeredHandles.end(), handle);
                if (it != event->registeredHandles.end()) {
                    count++;
                }
            }
        }
        rangeEventsMutex.unlock();
        return count;
    }

    //papyrus functions

    //count registered =========================================================================================================================================================================
    int GetNumRangeEventsRegisteredOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver) {
        if (!gfuncs::IsFormValid(eventReceiver)) {
            logger::warn("eventReceiver doesn't exist");
            return 0;
        }

        std::string receiverName = std::string(gfuncs::GetFormNameAndId(eventReceiver));

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        return GetNumRangeEventsRegisteredOnHandle(handle);
    }

    int GetNumRangeEventsRegisteredOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver) {
        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist");
            return 0;
        }

        std::string receiverName = std::string(eventReceiver->aliasName);

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        return GetNumRangeEventsRegisteredOnHandle(handle);
    }

    int GetNumRangeEventsRegisteredOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver) {
        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist");
            return 0;
        }

        std::string receiverName = "no name";
        auto* baseEffect = eventReceiver->GetBaseObject();
        if (baseEffect) {
            receiverName = gfuncs::GetFormNameAndId(baseEffect);
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        return GetNumRangeEventsRegisteredOnHandle(handle);
    }

    //is registered =========================================================================================================================================================================
    bool IsFormRegisteredForRangeEvents(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        if (!gfuncs::AllFormsValid({ akTarget, akCenterRef, eventReceiver },
            { "akTarget", "akCenterRef", "eventReceiver" }, __func__)) {
            return false;
        }

        std::string receiverName = std::string(gfuncs::GetFormNameAndId(eventReceiver));

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        return IsHandleRegistered(handle, akTarget, akCenterRef, distance);
    }

    bool IsAliasRegisteredForRangeEvents(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        if (!gfuncs::AllFormsValid({ akTarget, akCenterRef },
            { "akTarget", "akCenterRef" }, __func__)) {
            return false;
        }

        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist");
            return false;
        }

        std::string receiverName = std::string(eventReceiver->aliasName);

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        return IsHandleRegistered(handle, akTarget, akCenterRef, distance);
    }

    bool IsActiveMagicEffectRegisteredForRangeEvents(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        if (!gfuncs::AllFormsValid({ akTarget, akCenterRef },
            { "akTarget", "akCenterRef" }, __func__)) {
            return false;
        }

        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist");
            return false;
        }

        std::string receiverName = "no name";
        auto* baseEffect = eventReceiver->GetBaseObject();
        if (baseEffect) {
            receiverName = gfuncs::GetFormNameAndId(baseEffect);
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        return IsHandleRegistered(handle, akTarget, akCenterRef, distance);
    }

    //Register =========================================================================================================================================================================
    bool RegisterFormForRangeEvents(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        if (!gfuncs::AllFormsValid({ akTarget, akCenterRef, eventReceiver },
            { "akTarget", "akCenterRef", "eventReceiver" }, __func__)) {
            return false;
        }

        std::string receiverName = std::string(gfuncs::GetFormNameAndId(eventReceiver));

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        bool success = RegisterHandle(handle, akTarget, akCenterRef, distance);
        if (!success) {
            logger::debug("eventReceiver[{}] target[{}] centerRef[{}] distance[{}] already registered.",
                receiverName, gfuncs::GetFormNameAndId(akTarget), gfuncs::GetFormNameAndId(akCenterRef), distance);
        }
        else {
            logger::debug("eventReceiver[{}] target[{}] centerRef[{}] distance[{}] registered.",
                receiverName, gfuncs::GetFormNameAndId(akTarget), gfuncs::GetFormNameAndId(akCenterRef), distance);
        }

        return success;
    }

    bool RegisterAliasForRangeEvents(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        if (!gfuncs::AllFormsValid({ akTarget, akCenterRef },
            { "akTarget", "akCenterRef" }, __func__)) {
            return false;
        }

        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist");
            return false;
        }

        std::string receiverName = std::string(eventReceiver->aliasName);

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        bool success = RegisterHandle(handle, akTarget, akCenterRef, distance);
        if (!success) {
            logger::debug("eventReceiver[{}] target[{}] centerRef[{}] distance[{}] already registered.",
                receiverName, gfuncs::GetFormNameAndId(akTarget), gfuncs::GetFormNameAndId(akCenterRef), distance);
        }
        else {
            logger::debug("eventReceiver[{}] target[{}] centerRef[{}] distance[{}] registered.",
                receiverName, gfuncs::GetFormNameAndId(akTarget), gfuncs::GetFormNameAndId(akCenterRef), distance);
        }

        return success;
    }

    bool RegisterActiveMagicEffectForRangeEvents(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        if (!gfuncs::AllFormsValid({ akTarget, akCenterRef },
            { "akTarget", "akCenterRef" }, __func__)) {
            return false;
        }

        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist");
            return false;
        }

        std::string receiverName = "no name";
        auto* baseEffect = eventReceiver->GetBaseObject();
        if (baseEffect) {
            receiverName = gfuncs::GetFormNameAndId(baseEffect);
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        bool success = RegisterHandle(handle, akTarget, akCenterRef, distance);
        if (!success) {
            logger::debug("eventReceiver[{}] target[{}] centerRef[{}] distance[{}] already registered.",
                receiverName, gfuncs::GetFormNameAndId(akTarget), gfuncs::GetFormNameAndId(akCenterRef), distance);
        }
        else {
            logger::debug("eventReceiver[{}] target[{}] centerRef[{}] distance[{}] registered.",
                receiverName, gfuncs::GetFormNameAndId(akTarget), gfuncs::GetFormNameAndId(akCenterRef), distance);
        }

        return success;
    }

    //Unregister =========================================================================================================================================================================
    bool UnregisterFormForRangeEvents(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        if (!gfuncs::AllFormsValid({ akTarget, akCenterRef, eventReceiver },
            { "akTarget", "akCenterRef", "eventReceiver" }, __func__)) {
            return false;
        }

        std::string receiverName = std::string(gfuncs::GetFormNameAndId(eventReceiver));

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        bool success = UnregisterHandle(handle, akTarget, akCenterRef, distance);
        if (!success) {
            logger::debug("eventReceiver[{}] target[{}] centerRef[{}] distance[{}] wasn't registered.",
                receiverName, gfuncs::GetFormNameAndId(akTarget), gfuncs::GetFormNameAndId(akCenterRef), distance);
        }
        else {
            logger::debug("eventReceiver[{}] target[{}] centerRef[{}] distance[{}] Unregistered.",
                receiverName, gfuncs::GetFormNameAndId(akTarget), gfuncs::GetFormNameAndId(akCenterRef), distance);
        }

        return success;
    }

    bool UnregisterAliasForRangeEvents(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        if (!gfuncs::AllFormsValid({ akTarget, akCenterRef },
            { "akTarget", "akCenterRef" }, __func__)) {
            return false;
        }

        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist");
            return false;
        }

        std::string receiverName = std::string(eventReceiver->aliasName);

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        bool success = UnregisterHandle(handle, akTarget, akCenterRef, distance);
        if (!success) {
            logger::debug("eventReceiver[{}] target[{}] centerRef[{}] distance[{}] wasn't registered.",
                receiverName, gfuncs::GetFormNameAndId(akTarget), gfuncs::GetFormNameAndId(akCenterRef), distance);
        }
        else {
            logger::debug("eventReceiver[{}] target[{}] centerRef[{}] distance[{}] Unregistered.",
                receiverName, gfuncs::GetFormNameAndId(akTarget), gfuncs::GetFormNameAndId(akCenterRef), distance);
        }

        return success;
    }

    bool UnregisterActiveMagicEffectForRangeEvents(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, RE::TESObjectREFR* akTarget, RE::TESObjectREFR* akCenterRef, float distance) {
        if (!gfuncs::AllFormsValid({ akTarget, akCenterRef },
            { "akTarget", "akCenterRef" }, __func__)) {
            return false;
        }

        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist");
            return false;
        }

        std::string receiverName = "no name";
        auto* baseEffect = eventReceiver->GetBaseObject();
        if (baseEffect) {
            receiverName = gfuncs::GetFormNameAndId(baseEffect);
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        bool success = UnregisterHandle(handle, akTarget, akCenterRef, distance);
        if (!success) {
            logger::debug("eventReceiver[{}] target[{}] centerRef[{}] distance[{}] wasn't registered.",
                receiverName, gfuncs::GetFormNameAndId(akTarget), gfuncs::GetFormNameAndId(akCenterRef), distance);
        }
        else {
            logger::debug("eventReceiver[{}] target[{}] centerRef[{}] distance[{}] Unregistered.",
                receiverName, gfuncs::GetFormNameAndId(akTarget), gfuncs::GetFormNameAndId(akCenterRef), distance);
        }

        return success;
    }

    //Unregister_All =========================================================================================================================================================================
    int UnregisterFormForRangeEvents_All(RE::StaticFunctionTag*, RE::TESForm* eventReceiver) {
        if (!gfuncs::IsFormValid(eventReceiver)) {
            logger::warn("eventReceiver doesn't exist");
            return false;
        }

        std::string receiverName = std::string(gfuncs::GetFormNameAndId(eventReceiver));

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        int count = UnregisterHandle_All(handle);
        logger::debug("eventReceiver[{}] unregistered [{}] events",
            receiverName, count);

        return count;
    }

    int UnregisterAliasForRangeEvents_All(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver) {
        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist");
            return false;
        }

        std::string receiverName = std::string(eventReceiver->aliasName);

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        int count = UnregisterHandle_All(handle);
        logger::debug("eventReceiver[{}] unregistered [{}] events",
            receiverName, count);

        return count;
    }

    int UnregisterActiveMagicEffectForRangeEvents_All(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver) {
        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist");
            return false;
        }

        std::string receiverName = "no name";
        auto* baseEffect = eventReceiver->GetBaseObject();
        if (baseEffect) {
            receiverName = gfuncs::GetFormNameAndId(baseEffect);
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);
        int count = UnregisterHandle_All(handle);
        logger::debug("eventReceiver[{}] unregistered [{}] events",
            receiverName, count);

        return count;
    }

    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        //form
        vm->RegisterFunction("GetNumRangeEventsRegisteredOnForm", "DbSkseEvents", GetNumRangeEventsRegisteredOnForm);
        vm->RegisterFunction("IsFormRegisteredForRangeEvents", "DbSkseEvents", IsFormRegisteredForRangeEvents);
        vm->RegisterFunction("RegisterFormForRangeEvents", "DbSkseEvents", RegisterFormForRangeEvents);
        vm->RegisterFunction("UnregisterFormForRangeEvents", "DbSkseEvents", UnregisterFormForRangeEvents);
        vm->RegisterFunction("UnregisterFormForRangeEvents_All", "DbSkseEvents", UnregisterFormForRangeEvents_All);

        //Alias
        vm->RegisterFunction("GetNumRangeEventsRegisteredOnAlias", "DbSkseEvents", GetNumRangeEventsRegisteredOnAlias);
        vm->RegisterFunction("IsAliasRegisteredForRangeEvents", "DbSkseEvents", IsAliasRegisteredForRangeEvents);
        vm->RegisterFunction("RegisterAliasForRangeEvents", "DbSkseEvents", RegisterAliasForRangeEvents);
        vm->RegisterFunction("UnregisterAliasForRangeEvents", "DbSkseEvents", UnregisterAliasForRangeEvents);
        vm->RegisterFunction("UnregisterAliasForRangeEvents_All", "DbSkseEvents", UnregisterAliasForRangeEvents_All);

        //ActiveMagicEffect
        vm->RegisterFunction("GetNumRangeEventsRegisteredOnActiveMagicEffect", "DbSkseEvents", GetNumRangeEventsRegisteredOnActiveMagicEffect);
        vm->RegisterFunction("IsActiveMagicEffectRegisteredForRangeEvents", "DbSkseEvents", IsActiveMagicEffectRegisteredForRangeEvents);
        vm->RegisterFunction("RegisterActiveMagicEffectForRangeEvents", "DbSkseEvents", RegisterActiveMagicEffectForRangeEvents);
        vm->RegisterFunction("UnregisterActiveMagicEffectForRangeEvents", "DbSkseEvents", UnregisterActiveMagicEffectForRangeEvents);
        vm->RegisterFunction("UnregisterActiveMagicEffectForRangeEvents_All", "DbSkseEvents", UnregisterActiveMagicEffectForRangeEvents_All);

        return true;
    }
}
