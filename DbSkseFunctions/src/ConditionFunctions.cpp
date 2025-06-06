#include <mutex>
#include <typeinfo>
#include "ConditionFunctions.h"
#include "GeneralFunctions.h"
#include "RangeEvents.h"

namespace conditions {
    std::recursive_mutex mutex;
	std::unordered_map<std::string, RE::TESCondition*> conditionsMap;

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
    } 
    
    struct ConditionChangedEvent;
    void DeleteConditionEvent(ConditionChangedEvent* conditionEvent);

    struct ConditionChangedEvent {
        ConditionChangedEvent(std::string asConditionId, RE::TESCondition* akCondition, RE::TESObjectREFR* akTarget) {
            std::lock_guard<std::recursive_mutex> lock(mutex);
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
            std::lock_guard<std::recursive_mutex> lock(mutex);
            return (asConditionId == conditionId && akCondition == condition && akTarget == target);
        }

        int GetConditionStatus() {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (condition && !deleted) {
                return int(condition->IsTrue(target, nullptr));
            }
            return -1;
        }

        bool MarkForDelete() {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            bool result = false;
            if (!deleted) {
                condition = nullptr;
                target = nullptr;
                deleted = true;
                result = true;
            }
            return result;
        }

        bool IsDeleted() {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            return deleted;
        }

        std::string GetConditionId() {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            return conditionId;
        }

        //can only set if isPolling is the opposite of set
        bool SetPolling(bool set) {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (set) {
                if (!isPolling) {
                    isPolling = true;
                    return true;
                }
                else {
                    return false;
                }
            }
            else {
                if (isPolling) {
                    isPolling = false;
                    return true;
                }
                else {
                    return false;
                }
            }
        }

    private:
        void ListenForConditionEventChangeStart() {
            ListenForConditionEventChange();
        }

        void ListenForConditionEventChange() {
            std::thread t([=]() {
                if (!SetPolling(true)) {
                    logger::trace("already polling");
                }
                else { //succussfully set polling

                    int startStatus = GetConditionStatus();
                    int status = startStatus;

                    if (status != -1) {
                        while (startStatus == status) {
                            //logger::info("while loop");
                            std::this_thread::sleep_for(std::chrono::milliseconds(eventPollingInterval));
                            status = GetConditionStatus();
                        }
                    }

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
            auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
            if (vm) {
                auto* args = RE::MakeFunctionArguments((RE::TESObjectREFR*)target, (bool)isTrue);
                vm->SendEventAll(sEvent, args);
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
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (conditionEvent) {
            conditionEvent->MarkForDelete(); //ensure condition and target are set to nullptr
            auto it = std::find(conditionEvents.begin(), conditionEvents.end(), conditionEvent);

            if (it != conditionEvents.end()) {
                conditionEvents.erase(it);
            }
            delete conditionEvent;
        }
    }

    ConditionChangedEvent* GetConditionEvent(std::string asConditionId, RE::TESObjectREFR* akTarget) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        auto it = conditionsMap.find(asConditionId);
        if (it != conditionsMap.end()) {
            auto* condition = it->second;
            if (condition) {
                for (ConditionChangedEvent* conditionEvent : conditionEvents) {
                    if (conditionEvent) {
                        if (conditionEvent->ParamsMatch(asConditionId, condition, akTarget)) {
                            return conditionEvent;
                        }
                    }
                }
            }
        }
        return nullptr;
    }

    bool ConditionEventExists(RE::StaticFunctionTag*, std::string conditionId, RE::TESObjectREFR* akTarget) {
        return GetConditionEvent(conditionId, akTarget) != nullptr;
    }

    bool CreateConditionEvent(RE::StaticFunctionTag*, std::string conditionId, RE::TESObjectREFR* akTarget) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        ConditionChangedEvent* conditionEvent = GetConditionEvent(conditionId, akTarget);
        if (!conditionEvent) {
            auto it = conditionsMap.find(conditionId);
            if (it != conditionsMap.end()) {
                auto* condition = it->second;
                if (condition) {
                    ConditionChangedEvent* newConditionEvent = new ConditionChangedEvent(conditionId, condition, akTarget);
                    if (!newConditionEvent->IsDeleted()) {
                        conditionEvents.push_back(newConditionEvent);
                    }
                    else {
                        logger::error("condition for conditionId[{}] is nullptr", conditionId);
                        delete newConditionEvent;
                    }
                    logger::debug("condition event for conditionId[{}] and target[{}] created",
                        conditionId, gfuncs::GetFormNameAndId(akTarget));
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

            return false;
        }
    }

    bool DestroyConditionEvent(RE::StaticFunctionTag*, std::string conditionId, RE::TESObjectREFR* akTarget) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        auto it = conditionsMap.find(conditionId);
        if (it != conditionsMap.end()) {
            auto* condition = it->second;
            if (condition) {
                for (ConditionChangedEvent* conditionEvent : conditionEvents) {
                    if (conditionEvent) {
                        if (conditionEvent->ParamsMatch(conditionId, condition, akTarget)) {
                            return conditionEvent->MarkForDelete();
                        }
                    }
                }
            }
        }

        return false;
    }
    
    int DestroyAllConditionEvents(RE::StaticFunctionTag*, std::string conditionId) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        int count = 0;
        auto it = conditionsMap.find(conditionId);
        if (it != conditionsMap.end()) {
            auto* condition = it->second;
            if (condition) {
                for (ConditionChangedEvent* conditionEvent : conditionEvents) {
                    if (conditionEvent) {
                        if (conditionEvent->GetConditionId() == conditionId) {
                            if (conditionEvent->MarkForDelete()) {
                                count++;
                            }
                        }
                    }
                }
            }
        }

        return count;
    }

    int CountConditionEvents(RE::StaticFunctionTag*, std::string conditionId) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        int count = 0;
        auto it = conditionsMap.find(conditionId);
        if (it != conditionsMap.end()) {
            auto* condition = it->second;
            if (condition) {
                for (ConditionChangedEvent* conditionEvent : conditionEvents) {
                    if (conditionEvent) {
                        if (conditionEvent->GetConditionId() == conditionId) {
                            if (!conditionEvent->IsDeleted()) {
                                count++;
                            }
                        }
                    }
                }
            }
        }

        return count;
    }

    void DestroyConditionParameter(RE::TESConditionItem* item, int paramIndex) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        auto it = std::find(createdConditionParams.begin(), createdConditionParams.end(), item->data.functionData.params[paramIndex]);

        if (it != createdConditionParams.end()) {
            //param is an RE::BSFixedString, bool, int or float created with new(), delete it to avoid memory leaks.
            delete item->data.functionData.params[paramIndex];
            createdConditionParams.erase(it);
        }
        item->data.functionData.params[paramIndex] = nullptr;
    }

    void DestroyConditionCpp(std::string conditionId) {
        std::lock_guard<std::recursive_mutex> lock(mutex);

        DestroyAllConditionEvents(nullptr, conditionId);

        logger::trace("conditionId[{}]", conditionId);

        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            return;
        }

        if (it->second) {
            if (it->second->head) {
                DestroyConditionParameter(it->second->head, 0);
                DestroyConditionParameter(it->second->head, 1);
                DestroyConditionParameter(it->second->head, 2);

                delete it->second->head;
                it->second->head = nullptr;
            }
            delete it->second;
            it->second = nullptr;
        }

        conditionsMap.erase(it);
    }

    void DestroyCondition(RE::StaticFunctionTag*, std::string conditionId) {
        DestroyConditionCpp(conditionId);
    }

    void CreateCondition(RE::StaticFunctionTag*, std::string conditionId, int conditionFunction, int comparison, float value) {
        std::lock_guard<std::recursive_mutex> lock(mutex);

        if (conditionId == "") {
            logger::warn("Cannot create a condition with an empty conditionId");
            return;
        }
        
        //destroy condition if it exists
        DestroyConditionCpp(conditionId);

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
    }

    bool ConditionExists(RE::StaticFunctionTag*, std::string conditionId) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        bool result = (conditionsMap.find(conditionId) != conditionsMap.end());
        logger::trace("conditionId[{}] result[{}]", conditionId, result);
        
        return result;
    }

    bool SetConditionComparison(RE::StaticFunctionTag*, std::string conditionId, int comparison) {
        std::lock_guard<std::recursive_mutex> lock(mutex);

        if (comparison > 5 || comparison < 0) {
            logger::warn("conditionId[{}] comparison[{}] isn't valid. Must be between 0 and 5", conditionId, comparison);
            return false;
        } 

        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            logger::warn("conditionId[{}] not found", conditionId);
            return false;
        }

        auto* condition = it->second;
        if (!condition) {
            logger::warn("something went wrong. conditionId[{}] condition is nullptr", conditionId);
            return false;
        }

        auto* head = condition->head;
        if (!head) {
            logger::warn("something went wrong. conditionId[{}] condition->head is nullptr", conditionId);
            return false;
        } 

        logger::trace("conditionId[{}] comparison[{}]", conditionId, comparison);
        head->data.flags.opCode = static_cast<RE::CONDITION_ITEM_DATA::OpCode>(comparison);
        return true;
    }

    bool SetConditionValue(RE::StaticFunctionTag*, std::string conditionId, float value) {
        std::lock_guard<std::recursive_mutex> lock(mutex);

        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            logger::warn("conditionId[{}] not found", conditionId);
            return false;
        }

        auto* condition = it->second;
        if (!condition) {
            logger::warn("something went wrong. conditionId[{}] condition is nullptr", conditionId);
            return false;
        }

        auto* head = condition->head;
        if (!head) {
            logger::warn("something went wrong. conditionId[{}] condition->head is nullptr", conditionId);
            return false;
        }

        logger::trace("conditionId[{}] value[{}]", conditionId, value);
        head->data.comparisonValue.f = value;
        
        return true;
    }

    bool SetConditionParameter(std::string conditionId, void* param, int paramIndex, bool createdParam = true) {
        std::lock_guard<std::recursive_mutex> lock(mutex);

        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            logger::warn("conditionId[{}] not found", conditionId);
            return false;
        }

        if (paramIndex < 0 || paramIndex > 2) {
            logger::warn("conditionId[{}] paramIndex[{}] not valid. Must be between 0 and 2", conditionId, paramIndex);
            return false;
        }

        RE::TESCondition* condition = it->second;
        if (!condition) {
            logger::warn("something went wrong. conditionId[{}] condition is nullptr", conditionId);
            return false;
        }

        RE::TESConditionItem* head = condition->head;
        if (!head) {
            logger::warn("something went wrong. conditionId[{}] condition->head is nullptr", conditionId);
            return false;
        }

        DestroyConditionParameter(head, paramIndex);
        head->data.functionData.params[paramIndex] = param;
        
        if (createdParam) {
            createdConditionParams.push_back(param);
        }

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
        std::lock_guard<std::recursive_mutex> lock(mutex);

        bool result = false;
        auto it = conditionsMap.find(conditionId);

        if (it == conditionsMap.end()) {
            logger::warn("conditionId[{}] not found", conditionId);
            return result;
        }

        auto* condition = it->second;
        if (!condition) {
            logger::warn("something went wrong. conditionId[{}] condition is nullptr", conditionId);
            
            return result;
        }

        logger::trace("conditionId[{}]", conditionId);
        result = condition->IsTrue(target, nullptr);
        logger::trace("conditionId[{}] result[{}]", conditionId, result);
        
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