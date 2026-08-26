#include <mutex>
#include <typeinfo>
#include "ConditionFunctions.h"
#include "GeneralFunctions.h"
#include "RangeEvents.h"
#include "SharedVariables.h"

namespace conditions {
    std::unordered_map<std::string, RE::TESCondition*> conditionsMap;
    std::mutex mutex;
    std::condition_variable updateCv;
    bool isEmpty = true;

    std::list<void*> createdConditionParams;

    std::string ConditionComparisonToString(int comparison) {
        switch (comparison) {
            case 0: {
                return "==";
            }
            case 1: {
                return "!=";
            }
            case 2: {
                return ">";
            }
            case 3: {
                return ">=";
            }
            case 4: {
                return "<";
            }
            case 5: {
                return "<=";
            }
        }
        return "";
    } 
    
    struct ConditionChangedEvent;
    void DeleteConditionEvent(ConditionChangedEvent* conditionEvent);

    struct ConditionChangedEvent {
        ConditionChangedEvent(std::string asConditionId, RE::TESCondition* akCondition, RE::TESObjectREFR* akTarget) {
            if (!akCondition) {
                deleted = true;
            }
            else {
                condition = akCondition;
                target = akTarget;
                conditionId = asConditionId;
                sEvent = "On" + conditionId + "Changed";
                ListenForConditionEventChangeStart();
            }
        }

        bool ParamsMatch(std::string asConditionId, RE::TESCondition* akCondition, RE::TESObjectREFR* akTarget) {
            mutex.lock();
            bool result = (asConditionId == conditionId && akCondition == condition && akTarget == target);
            mutex.unlock();
            return result;
        }

        int GetConditionStatus() {
            mutex.lock();
            int result = -1;
            if (condition && !deleted) {
                result = int(condition->IsTrue(target, nullptr));
            }
            mutex.unlock();
            return result;
        }

        bool MarkForDelete() {
            mutex.lock();
            bool result = false;
            if (!deleted) {
                condition = nullptr;
                target = nullptr;
                deleted = true;
                result = true;
            }
            mutex.unlock();
            return result;
        }

        bool IsDeleted() {
            mutex.lock();
            bool result = deleted; 
            mutex.unlock();
            return result;
        }

        std::string GetConditionId() {
            mutex.lock();
            std::string result = conditionId;
            mutex.unlock();
            return result;
        }

        //can only set if isPolling is the opposite of set
        bool SetPolling(bool set) {
            mutex.lock();
            if (set) {
                if (!isPolling) {
                    isPolling = true;
                    mutex.unlock();
                    return true;
                }
                else {
                    mutex.unlock();
                    return false;
                }
            }
            else {
                if (isPolling) {
                    isPolling = false;
                    mutex.unlock();
                    return true;
                }
                else {
                    mutex.unlock();
                    return false;
                }
            }
        }

    private:
        void ListenForConditionEventChangeStart() {
            ListenForConditionEventChange();
        }
        //
        void ListenForConditionEventChange() {
            isEmpty = false;
            std::thread t([=]() {
                if (!SetPolling(true)) {
                    logger::trace("already polling");
                }
                else { //succussfully set polling

                    int startStatus = GetConditionStatus();
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        // waiting
                        updateCv.wait(lock, [=] {
                            int status = -1;
                            if (condition && !deleted) {
                                status = int(condition->IsTrue(target, nullptr));
                            }
                            if (status == -1) {
                                return true;
                            }
                            else {
                                return (status != startStatus);
                            }
                        });
                    }

                    int status = GetConditionStatus();

                    if (status != -1) {
                        HandleConditionChangeEvent(bool(status));
                        SetPolling(false);
                        ListenForConditionEventChangeStart();
                    }
                    else {
                        DeleteConditionEvent(this);
                    }
                }
            });
            t.detach();
        }

        void HandleConditionChangeEvent(bool isTrue) {
            if (sv::vm) {
                auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)target, (bool)isTrue);
                sv::vm->SendEventAll(sEvent, args);
                delete args;
            }
        }

        bool deleted = false;
        bool isPolling = false;
        RE::TESObjectREFR* target = nullptr;
        RE::TESCondition* condition = nullptr;
        RE::BSFixedString sEvent = nullptr;
        std::string conditionId = "";
    };

    std::vector<ConditionChangedEvent*> conditionEvents;

    void DeleteConditionEvent(ConditionChangedEvent* conditionEvent) {
        mutex.lock();
        if (conditionEvent) {
            mutex.unlock(); //MarkForDelete is locked, unlock before calling
            conditionEvent->MarkForDelete(); //ensure condition and target are set to nullptr
            mutex.lock();

            auto it = std::find(conditionEvents.begin(), conditionEvents.end(), conditionEvent);

            if (it != conditionEvents.end()) {
                conditionEvents.erase(it);
            }
            delete conditionEvent;
        }
        if (conditionEvents.size() == 0) {
            isEmpty = true;
        }
        mutex.unlock();
    }

    ConditionChangedEvent* GetConditionEvent(std::string asConditionId, RE::TESObjectREFR* akTarget) {
        mutex.lock();
        auto it = conditionsMap.find(asConditionId);
        if (it != conditionsMap.end()) {
            auto* condition = it->second;
            if (condition) {
                for (int i = 0; i < conditionEvents.size(); i++) {
                    ConditionChangedEvent* conditionEvent = conditionEvents[i];
                    if (conditionEvent) {
                        mutex.unlock(); //ParamsMatch is locked, unlock before calling
                        if (conditionEvent->ParamsMatch(asConditionId, condition, akTarget)) {
                            return conditionEvent;
                        }
                        mutex.lock();
                    }
                }
            }
        }
        mutex.unlock();
        return nullptr;
    }

    bool ConditionEventExists(RE::StaticFunctionTag*, std::string conditionId, RE::TESObjectREFR* akTarget) {
        return GetConditionEvent(conditionId, akTarget) != nullptr;
    }

    bool CreateConditionEvent(RE::StaticFunctionTag*, std::string conditionId, RE::TESObjectREFR* akTarget) {
        ConditionChangedEvent* conditionEvent = GetConditionEvent(conditionId, akTarget);
        mutex.lock();
        if (!conditionEvent) {
            auto it = conditionsMap.find(conditionId);
            if (it != conditionsMap.end()) {
                auto* condition = it->second;
                if (condition) {
                    ConditionChangedEvent* newConditionEvent = new ConditionChangedEvent(conditionId, condition, akTarget);
                    
                    mutex.unlock(); //isDeleted is locked, unlock before calling to prevent deadlock.
                    bool deleted = newConditionEvent->IsDeleted();
                    mutex.lock();

                    if (!deleted) {
                        conditionEvents.push_back(newConditionEvent);
                    }
                    else {
                        logger::error("condition for conditionId[{}] is nullptr", conditionId);
                        delete newConditionEvent;
                    }
                    logger::debug("condition event for conditionId[{}] and target[{}] created",
                        conditionId, gfuncs::GetFormNameAndId(akTarget));

                    mutex.unlock();
                    return true;
                }
                else {
                    logger::error("condition for conditionId[{}] is nullptr", conditionId);
                }
            }
            else {
                logger::error("condition for conditionId[{}] not found", conditionId);
            }
        }
        else {
            logger::debug("condition event for conditionId[{}] and target[{}] already exists", 
                conditionId, gfuncs::GetFormNameAndId(akTarget));

            mutex.unlock();
            return false;
        }
        mutex.unlock();
        return false;
    }

    bool DestroyConditionEvent(RE::StaticFunctionTag*, std::string conditionId, RE::TESObjectREFR* akTarget) {
        mutex.lock();
        auto it = conditionsMap.find(conditionId);
        if (it != conditionsMap.end()) {
            auto* condition = it->second;
            if (condition) {
                for (int i = 0; i < conditionEvents.size(); i++) {
                    ConditionChangedEvent* conditionEvent = conditionEvents[i];
                    if (conditionEvent) {
                        mutex.unlock(); //paramsMatch and MarkForDelete are locked. Unlock before calling to prevent deadlock.
                        if (conditionEvent->ParamsMatch(conditionId, condition, akTarget)) {
                            return conditionEvent->MarkForDelete();
                        }
                        mutex.lock();
                    }
                }
            }
        }

        mutex.unlock();
        return false;
    }
    
    int DestroyAllConditionEvents(RE::StaticFunctionTag*, std::string conditionId) {
        mutex.lock();
        int count = 0;
        auto it = conditionsMap.find(conditionId);
        if (it != conditionsMap.end()) {
            auto* condition = it->second;
            if (condition) {
                for (int i = 0; i < conditionEvents.size(); i++) {
                    ConditionChangedEvent* conditionEvent = conditionEvents[i];
                    if (conditionEvent) {
                        mutex.unlock(); //GetConditionId and MarkForDelete are locked. Unlock before calling to prevent deadlock.
                        if (conditionEvent->GetConditionId() == conditionId) {
                            if (conditionEvent->MarkForDelete()) {
                                count++;
                            }
                        }
                        mutex.lock();
                    }
                }
            }
        }

        mutex.unlock();
        return count;
    }

    int CountConditionEvents(RE::StaticFunctionTag*, std::string conditionId) {
        mutex.lock();
        int count = 0;
        auto it = conditionsMap.find(conditionId);
        if (it != conditionsMap.end()) {
            auto* condition = it->second;
            if (condition) {
                for (int i = 0; i < conditionEvents.size(); i++) {
                    ConditionChangedEvent* conditionEvent = conditionEvents[i];
                    if (conditionEvent) {
                        mutex.unlock(); //GetConditionId and MarkForDelete are locked. Unlock before calling to prevent deadlock.
                        if (conditionEvent->GetConditionId() == conditionId) {
                            if (!conditionEvent->IsDeleted()) {
                                count++;
                            }
                        }
                        mutex.lock();
                    }
                }
            }
        }

        mutex.unlock();
        return count;
    }

    void DestroyConditionParameter(RE::TESConditionItem* item, int paramIndex) {
        mutex.lock();
        auto it = std::find(createdConditionParams.begin(), createdConditionParams.end(), item->data.functionData.params[paramIndex]);

        if (it != createdConditionParams.end()) {
            //param is an RE::BSFixedString, bool, int or float created with new(), delete it to avoid memory leaks.
            delete item->data.functionData.params[paramIndex];
            createdConditionParams.erase(it);
        }
        item->data.functionData.params[paramIndex] = nullptr;
        mutex.unlock();
    }

    void DestroyConditionCpp(std::string conditionId) {
        DestroyAllConditionEvents(nullptr, conditionId);
        mutex.lock();

        logger::trace("conditionId[{}]", conditionId);

        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            mutex.unlock();
            return;
        }

        if (it->second) {
            if (it->second->head) {
                mutex.unlock();
                DestroyConditionParameter(it->second->head, 0);
                DestroyConditionParameter(it->second->head, 1);
                DestroyConditionParameter(it->second->head, 2);
                mutex.lock();

                delete it->second->head;
                it->second->head = nullptr;
            }
            delete it->second;
            it->second = nullptr;
        }

        conditionsMap.erase(it);
        mutex.unlock();
    }

    void DestroyCondition(RE::StaticFunctionTag*, std::string conditionId) {
        DestroyConditionCpp(conditionId);
    }

    void CreateCondition(RE::StaticFunctionTag*, std::string conditionId, int conditionFunction, int comparison, float value) {
        if (conditionId == "") {
            logger::warn("Cannot create a condition with an empty conditionId");
            return;
        }
        
        //destroy condition if it exists
        DestroyConditionCpp(conditionId);

        mutex.lock();

        logger::trace("conditionId[{}], conditionFunction[{}], comparison[{}], value[{}]",
            conditionId, conditionFunction, comparison, value);

        RE::TESCondition* condition = new RE::TESCondition;
        RE::TESConditionItem* conditionItem = new RE::TESConditionItem;

        conditionItem->data.functionData.function = static_cast<RE::FUNCTION_DATA::FunctionID>(conditionFunction);
        
        if (comparison <= 5 && comparison >= 0) {
            conditionItem->data.flags.opCode = static_cast<RE::CONDITION_ITEM_DATA::OpCode>(comparison);
        }
        
        conditionItem->data.comparisonValue.f = value;
        condition->head = conditionItem;

        conditionsMap[conditionId] = condition;
        mutex.unlock();
    }

    bool ConditionExists(RE::StaticFunctionTag*, std::string conditionId) {
        mutex.lock();
        bool result = (conditionsMap.find(conditionId) != conditionsMap.end());
        logger::trace("conditionId[{}] result[{}]", conditionId, result);
        mutex.unlock();
        return result;
    }

    bool SetConditionComparison(RE::StaticFunctionTag*, std::string conditionId, int comparison) {
        mutex.lock();

        if (comparison > 5 || comparison < 0) {
            logger::warn("conditionId[{}] comparison[{}] isn't valid. Must be between 0 and 5", conditionId, comparison);
            mutex.unlock();
            return false;
        } 

        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            logger::warn("conditionId[{}] not found", conditionId);
            mutex.unlock();
            return false;
        }

        auto* condition = it->second;
        if (!condition) {
            logger::warn("something went wrong. conditionId[{}] condition is nullptr", conditionId);
            mutex.unlock();
            return false;
        }

        auto* head = condition->head;
        if (!head) {
            logger::warn("something went wrong. conditionId[{}] condition->head is nullptr", conditionId);
            mutex.unlock();
            return false;
        } 

        logger::trace("conditionId[{}] comparison[{}]", conditionId, comparison);
        head->data.flags.opCode = static_cast<RE::CONDITION_ITEM_DATA::OpCode>(comparison);
        mutex.unlock();
        return true;
    }

    bool SetConditionValue(RE::StaticFunctionTag*, std::string conditionId, float value) {
        mutex.lock();

        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            logger::warn("conditionId[{}] not found", conditionId);
            mutex.unlock();
            return false;
        }

        auto* condition = it->second;
        if (!condition) {
            logger::warn("something went wrong. conditionId[{}] condition is nullptr", conditionId);
            mutex.unlock();
            return false;
        }

        auto* head = condition->head;
        if (!head) {
            logger::warn("something went wrong. conditionId[{}] condition->head is nullptr", conditionId);
            mutex.unlock();
            return false;
        }

        logger::trace("conditionId[{}] value[{}]", conditionId, value);
        head->data.comparisonValue.f = value;
        
        mutex.unlock();
        return true;
    }

    bool SetConditionParameter(std::string conditionId, void* param, int paramIndex, bool createdParam = true) {
        mutex.lock();

        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            logger::warn("conditionId[{}] not found", conditionId);
            mutex.unlock();
            return false;
        }

        if (paramIndex < 0 || paramIndex > 2) {
            logger::warn("conditionId[{}] paramIndex[{}] not valid. Must be between 0 and 2", conditionId, paramIndex);
            mutex.unlock();
            return false;
        }

        RE::TESCondition* condition = it->second;
        if (!condition) {
            logger::warn("something went wrong. conditionId[{}] condition is nullptr", conditionId);
            mutex.unlock();
            return false;
        }

        RE::TESConditionItem* head = condition->head;
        if (!head) {
            logger::warn("something went wrong. conditionId[{}] condition->head is nullptr", conditionId);
            mutex.unlock();
            return false;
        }

        mutex.unlock();
        DestroyConditionParameter(head, paramIndex);
        mutex.lock();
        head->data.functionData.params[paramIndex] = param;
        
        if (createdParam) {
            createdConditionParams.push_back(param);
        }

        mutex.unlock();
        return true;
    }

    bool SetConditionParameterForm(RE::StaticFunctionTag*, std::string conditionId, RE::TESForm* param, int paramIndex) {
        RE::FormType type;
        if (param) {
            type = param->GetFormType();
        } 
        logger::trace("conditionId[{}] param[{}] paramIndex[{}] type[{}]", conditionId, gfuncs::GetFormName(param), paramIndex, type);
        bool result = SetConditionParameter(conditionId, param, paramIndex, false);
        return result;
    }

    bool SetConditionParameterAlias(RE::StaticFunctionTag*, std::string conditionId, RE::BGSBaseAlias* param, int paramIndex) {
        logger::trace("conditionId[{}] param[{}] paramIndex[{}]", conditionId, param->aliasName, paramIndex);
        bool result = SetConditionParameter(conditionId, param, paramIndex, false);
        return result;
    }

    bool SetConditionParameterBool(RE::StaticFunctionTag*, std::string conditionId, bool param, int paramIndex) {
        logger::trace("conditionId[{}] param[{}] paramIndex[{}]", conditionId, param, paramIndex);
        bool* paramPtr = new bool(param);
        bool result = SetConditionParameter(conditionId, paramPtr, paramIndex);
        return result;
    }

    bool SetConditionParameterInt(RE::StaticFunctionTag*, std::string conditionId, int param, int paramIndex) {
        logger::trace("conditionId[{}] param[{}] paramIndex[{}]", conditionId, param, paramIndex);
        auto* paramPtrint = new int(param);
        bool result = SetConditionParameter(conditionId, paramPtrint, paramIndex);
        return result;
    }

    bool SetConditionParameterFloat(RE::StaticFunctionTag*, std::string conditionId, float param, int paramIndex) {
        logger::trace("conditionId[{}] param[{}] paramIndex[{}]", conditionId, param, paramIndex);
        float* paramPtr = new float(param);
        bool result = SetConditionParameter(conditionId, paramPtr, paramIndex);
        return result;
    }

    bool SetConditionParameterString(RE::StaticFunctionTag*, std::string conditionId, std::string param, int paramIndex) {
        logger::trace("conditionId[{}] param[{}] paramIndex[{}]", conditionId, param, paramIndex);
        //must use new fixed string or it causes ctd when using EvaluateCondition
        RE::BSFixedString* paramPtr = new RE::BSFixedString(param);
        bool result = SetConditionParameter(conditionId, paramPtr, paramIndex);
        return result;
    }
    
    bool EvaluateCondition(RE::StaticFunctionTag*, std::string conditionId, RE::TESObjectREFR* target) {
        mutex.lock();

        bool result = false;
        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            logger::warn("conditionId[{}] not found", conditionId);
            mutex.unlock();
            return result;
        }

        auto* condition = it->second;
        if (!condition) {
            logger::warn("something went wrong. conditionId[{}] condition is nullptr", conditionId);
            mutex.unlock();
            return result;
        }

        logger::trace("conditionId[{}]", conditionId);
        result = condition->IsTrue(target, nullptr);
        logger::trace("conditionId[{}] result[{}]", conditionId, result);
        mutex.unlock();
        return result;
    }
    
    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        //events
        vm->RegisterFunction("ConditionEventExists", "DbConditionFunctions", ConditionEventExists);
        vm->RegisterFunction("CreateConditionEvent", "DbConditionFunctions", CreateConditionEvent);
        vm->RegisterFunction("DestroyConditionEvent", "DbConditionFunctions", DestroyConditionEvent);
        vm->RegisterFunction("DestroyAllConditionEvents", "DbConditionFunctions", DestroyAllConditionEvents);
        vm->RegisterFunction("CountConditionEvents", "DbConditionFunctions", CountConditionEvents);

        //functions
        vm->RegisterFunction("CreateCondition", "DbConditionFunctions", CreateCondition);
        vm->RegisterFunction("DestroyCondition", "DbConditionFunctions", DestroyCondition);
        vm->RegisterFunction("ConditionExists", "DbConditionFunctions", ConditionExists);
        vm->RegisterFunction("SetConditionParameterForm", "DbConditionFunctions", SetConditionParameterForm);
        vm->RegisterFunction("SetConditionParameterAlias", "DbConditionFunctions", SetConditionParameterAlias);
        vm->RegisterFunction("SetConditionParameterBool", "DbConditionFunctions", SetConditionParameterBool);
        vm->RegisterFunction("SetConditionParameterInt", "DbConditionFunctions", SetConditionParameterInt);
        vm->RegisterFunction("SetConditionParameterFloat", "DbConditionFunctions", SetConditionParameterFloat);
        vm->RegisterFunction("SetConditionParameterString", "DbConditionFunctions", SetConditionParameterString);
        vm->RegisterFunction("SetConditionComparison", "DbConditionFunctions", SetConditionComparison);
        vm->RegisterFunction("SetConditionValue", "DbConditionFunctions", SetConditionValue);
        vm->RegisterFunction("EvaluateCondition", "DbConditionFunctions", EvaluateCondition);
        return true;
    }
}