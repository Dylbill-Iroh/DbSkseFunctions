#include <Windows.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include "Animation.h"
#include "GeneralFunctions.h"

namespace animation {
    RE::BSScript::Internal::VirtualMachine* vm;

    struct AnimationGraphData {
        RE::IAnimationGraphManagerHolder* managerHolder = nullptr;
        RE::BSAnimationGraphManager* manager = nullptr;
        RE::BSTArray<RE::AnimVariableCacheInfo> actorVariablesCache;
    };

    AnimationGraphData GetAnimationGraphData(RE::TESObjectREFR* ref) {
        AnimationGraphData data;
        RE::IAnimationGraphManagerHolder* aniManagerHolder = static_cast<RE::IAnimationGraphManagerHolder*>(ref);
        if (aniManagerHolder) {
            data.managerHolder = aniManagerHolder;
            RE::BSTSmartPointer<RE::BSAnimationGraphManager> aniManagerPtr;

            if (aniManagerHolder->GetAnimationGraphManager(aniManagerPtr)) {
                data.manager = aniManagerPtr.get();
            }

            RE::Actor* akActor = skyrim_cast<RE::Actor*>(ref);
            if (gfuncs::IsFormValid(akActor)) {
                const auto* ai = akActor->GetActorRuntimeData().currentProcess;
                if (ai) {
                    if (ai->middleHigh) {
                        data.actorVariablesCache = ai->middleHigh->animationVariableCache->variableCache;
                    }
                }
            }
        }

        return data;
    }

    std::pair<bool, float> GetAnimationVariableFloat(RE::IAnimationGraphManagerHolder* aniManagerHolder, std::string& variable) {
        float var = 0.0;
        if (aniManagerHolder->GetGraphVariableFloat(variable, var)) {
            return { true, var };
        }
        return { false, var };
    }

    void LogFloatAnimationVariable(RE::IAnimationGraphManagerHolder* aniManagerHolder, std::string& refName, std::string variable) {
        if (variable != "") {
            auto value = GetAnimationVariableFloat(aniManagerHolder, variable);
            logger::info("{} {} = {}", refName, variable, value.second, value.first);
        }
    }

    void LogFloatAnimationVariables(RE::IAnimationGraphManagerHolder* aniManagerHolder, std::string& refName, std::vector<std::string>& variables) {
        if (variables.size() > 0) {
            for (auto& variable : variables) {
                LogFloatAnimationVariable(aniManagerHolder, refName, variable);
            }
        }
        else {
            LogFloatAnimationVariable(aniManagerHolder, refName, "Speed");
            LogFloatAnimationVariable(aniManagerHolder, refName, "VelocityZ");
            LogFloatAnimationVariable(aniManagerHolder, refName, "camerafromx");
            LogFloatAnimationVariable(aniManagerHolder, refName, "camerafromy");
            LogFloatAnimationVariable(aniManagerHolder, refName, "camerafromz");
            LogFloatAnimationVariable(aniManagerHolder, refName, "FemaleOffset");
            LogFloatAnimationVariable(aniManagerHolder, refName, "bodyMorphWeight");
            LogFloatAnimationVariable(aniManagerHolder, refName, "IsInCastState");
            LogFloatAnimationVariable(aniManagerHolder, refName, "IsInCastStateDamped");
            LogFloatAnimationVariable(aniManagerHolder, refName, "blockDown");
            LogFloatAnimationVariable(aniManagerHolder, refName, "blockLeft");
            LogFloatAnimationVariable(aniManagerHolder, refName, "blockRight");
            LogFloatAnimationVariable(aniManagerHolder, refName, "blockUp");
            LogFloatAnimationVariable(aniManagerHolder, refName, "Direction");
            LogFloatAnimationVariable(aniManagerHolder, refName, "TurnDelta");
            LogFloatAnimationVariable(aniManagerHolder, refName, "SpeedWalk");
            LogFloatAnimationVariable(aniManagerHolder, refName, "SpeedRun");
            LogFloatAnimationVariable(aniManagerHolder, refName, "fSpeedMin");
            LogFloatAnimationVariable(aniManagerHolder, refName, "fEquipWeapAdj");
            LogFloatAnimationVariable(aniManagerHolder, refName, "fIdleTimer");
            LogFloatAnimationVariable(aniManagerHolder, refName, "fMinSpeed");
            LogFloatAnimationVariable(aniManagerHolder, refName, "fTwistDirection");
            LogFloatAnimationVariable(aniManagerHolder, refName, "TurnDeltaDamped");
            LogFloatAnimationVariable(aniManagerHolder, refName, "TurnMin");
            LogFloatAnimationVariable(aniManagerHolder, refName, "Pitch");
            LogFloatAnimationVariable(aniManagerHolder, refName, "PitchLook");
            LogFloatAnimationVariable(aniManagerHolder, refName, "attackPowerStartTime");
            LogFloatAnimationVariable(aniManagerHolder, refName, "PitchDefault");
            LogFloatAnimationVariable(aniManagerHolder, refName, "PitchOverride");
            LogFloatAnimationVariable(aniManagerHolder, refName, "staggerMagnitude");
            LogFloatAnimationVariable(aniManagerHolder, refName, "recoilMagnitude");
            LogFloatAnimationVariable(aniManagerHolder, refName, "SpeedSampled");
            LogFloatAnimationVariable(aniManagerHolder, refName, "attackComboStartFraction");
            LogFloatAnimationVariable(aniManagerHolder, refName, "attackIntroLength");
            LogFloatAnimationVariable(aniManagerHolder, refName, "TimeDelta");
            LogFloatAnimationVariable(aniManagerHolder, refName, "PitchOffset");
            LogFloatAnimationVariable(aniManagerHolder, refName, "PitchAcc");
            LogFloatAnimationVariable(aniManagerHolder, refName, "PitchThresh");
            LogFloatAnimationVariable(aniManagerHolder, refName, "weapChangeStartFraction");
            LogFloatAnimationVariable(aniManagerHolder, refName, "RotMax");
            LogFloatAnimationVariable(aniManagerHolder, refName, "1stPRot");
            LogFloatAnimationVariable(aniManagerHolder, refName, "1stPRotDamped");
            LogFloatAnimationVariable(aniManagerHolder, refName, "PitchManualOverride");
            LogFloatAnimationVariable(aniManagerHolder, refName, "SpeedAcc");
        }
    }

    std::pair<bool, int> GetAnimationVariableInt(RE::IAnimationGraphManagerHolder* aniManagerHolder, std::string& variable) {
        int var = 0;
        if (aniManagerHolder->GetGraphVariableInt(variable, var)) {
            return { true, var };
        }
        return { false, var };
    }

    void LogIntAnimationVariable(RE::IAnimationGraphManagerHolder* aniManagerHolder, std::string& refName, std::string variable) {
        if (variable != "") {
            auto value = GetAnimationVariableInt(aniManagerHolder, variable);
            logger::info("{} {} = {}", refName, variable, value.second, value.first);
        }
    }

    void LogIntAnimationVariables(RE::IAnimationGraphManagerHolder* aniManagerHolder, std::string& refName, std::vector<std::string>& variables) {
        if (variables.size() > 0) {
            for (auto& variable : variables) {
                LogIntAnimationVariable(aniManagerHolder, refName, variable);
            }
        }
        else {
            LogIntAnimationVariable(aniManagerHolder, refName, "iSyncIdleLocomotion");
            LogIntAnimationVariable(aniManagerHolder, refName, "IsAttackReady_32");
            LogIntAnimationVariable(aniManagerHolder, refName, "iRightHandType");
            LogIntAnimationVariable(aniManagerHolder, refName, "iWantBlock");
            LogIntAnimationVariable(aniManagerHolder, refName, "iAnnotation");
            LogIntAnimationVariable(aniManagerHolder, refName, "iSyncTurnState");
            LogIntAnimationVariable(aniManagerHolder, refName, "i1stPerson");
            LogIntAnimationVariable(aniManagerHolder, refName, "iLeftHandType");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCSprinting");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCDefault");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCSneaking");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCBowDrawn");
            LogIntAnimationVariable(aniManagerHolder, refName, "iDualMagicState");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCBlocking");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCBleedout");
            LogIntAnimationVariable(aniManagerHolder, refName, "iBlockState");
            LogIntAnimationVariable(aniManagerHolder, refName, "iSyncSprintState");
            LogIntAnimationVariable(aniManagerHolder, refName, "iIsInSneak");
            LogIntAnimationVariable(aniManagerHolder, refName, "iMagicEquipped ;Equips magic.");
            LogIntAnimationVariable(aniManagerHolder, refName, "iEquippedItemState");
            LogIntAnimationVariable(aniManagerHolder, refName, "iMagicState");
            LogIntAnimationVariable(aniManagerHolder, refName, "iIsDialogueExpressive");
            LogIntAnimationVariable(aniManagerHolder, refName, "iSyncIdleState");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPC1HM");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPC2HM");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCBow");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCMagic");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCMagicCasting");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCHorse");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_HorseSprint");
            LogIntAnimationVariable(aniManagerHolder, refName, "iCharacterSelector");
            LogIntAnimationVariable(aniManagerHolder, refName, "iCombatStance");
            LogIntAnimationVariable(aniManagerHolder, refName, "iSyncTurnDirection");
            LogIntAnimationVariable(aniManagerHolder, refName, "iRegularAttack");
            LogIntAnimationVariable(aniManagerHolder, refName, "iRightHandEquipped");
            LogIntAnimationVariable(aniManagerHolder, refName, "iLeftHandEquipped");
            LogIntAnimationVariable(aniManagerHolder, refName, "iIsPlayer");
            LogIntAnimationVariable(aniManagerHolder, refName, "iGetUpType");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCAttacking");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCPowerAttacking");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCAttacking2H");
            LogIntAnimationVariable(aniManagerHolder, refName, "iDrunkVariable");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCDrunk");
            LogIntAnimationVariable(aniManagerHolder, refName, "iTempSwitch");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCBowDrawnQuickShot");
            LogIntAnimationVariable(aniManagerHolder, refName, "iState_NPCBlockingShieldCharge");
        }
    }

    std::pair<bool, bool> GetAnimationVariableBool(RE::IAnimationGraphManagerHolder* aniManagerHolder, std::string& variable) {
        bool var = false;
        if (aniManagerHolder->GetGraphVariableBool(variable, var)) {
            return { true, var };
        }
        return { false, var };
    }

    void LogBoolAnimationVariable(RE::IAnimationGraphManagerHolder* aniManagerHolder, std::string& refName, std::string variable) {
        if (variable != "") {
            auto value = GetAnimationVariableBool(aniManagerHolder, variable);
            logger::info("{} {} = {}", refName, variable, value.second, value.first);
        }
    }

    void LogBoolAnimationVariables(RE::IAnimationGraphManagerHolder* aniManagerHolder, std::string& refName, std::vector<std::string>& variables) {
        if (variables.size() > 0) {
            for (auto& variable : variables) {
                LogBoolAnimationVariable(aniManagerHolder, refName, variable);
            }
        }
        else {
            LogBoolAnimationVariable(aniManagerHolder, refName, "bMotionDriven");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsBeastRace");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsSneaking");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsBleedingOut");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsCastingDual");
            LogBoolAnimationVariable(aniManagerHolder, refName, "Is1HM");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsCastingRight");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsCastingLeft");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsBlockHit");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsPlayer");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsNPC");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bIsSynced");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bVoiceReady");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bWantCastLeft");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bWantCastRight");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bWantCastVoice");
            LogBoolAnimationVariable(aniManagerHolder, refName, "b1HM_MLh_attack");
            LogBoolAnimationVariable(aniManagerHolder, refName, "b1HMCombat");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bAnimationDriven");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bCastReady");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bAllowRotation");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bMagicDraw");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bMLh_Ready");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bMRh_Ready");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bInMoveState");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bSprintOK");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bIdlePlaying");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bIsDialogueExpressive");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bAnimObjectLoaded");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bEquipUnequip");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bAttached");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bEquipOK");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bIsH2HSolo");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bHeadTracking");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bIsRiding");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bTalkable");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bRitualSpellActive");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bInJumpState");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bHeadTrackSpine");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bLeftHandAttack");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bIsInMT");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bHumanoidFootIKEnable");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bHumanoidFootIKDisable");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bStaggerPlayerOverride");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bNoStagger");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bIsStaffLeftCasting");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bPerkShieldCharge");
            LogBoolAnimationVariable(aniManagerHolder, refName, "bPerkQuickShot");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsAttacking");
            LogBoolAnimationVariable(aniManagerHolder, refName, "Isblocking");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsBashing");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsStaggering");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsRecoiling");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsEquipping");
            LogBoolAnimationVariable(aniManagerHolder, refName, "IsUnequipping");
        }
    }

    void LogAllTypeAnimationVariable(RE::IAnimationGraphManagerHolder* aniManagerHolder, std::string& refName, std::string variable) {
        if (variable != "") {
            bool bVar = false;
            aniManagerHolder->GetGraphVariableBool(variable, bVar);

            int32_t iVar = 0;
            aniManagerHolder->GetGraphVariableInt(variable, iVar);

            float fVar = 0.0;
            aniManagerHolder->GetGraphVariableFloat(variable, fVar);

            logger::info("[{}] variable[{}] bool[{}] int[{}] float[{}]", refName, variable, bVar, iVar, fVar);
        }
    }

    void LogAllTypeAnimationVariables(RE::IAnimationGraphManagerHolder* aniManagerHolder, std::string& refName, std::vector<std::string>& variables) {
        if (variables.size() > 0) {
            for (auto& variable : variables) {
                LogAllTypeAnimationVariable(aniManagerHolder, refName, variable);
            }
        }
        else {
            LogBoolAnimationVariables(aniManagerHolder, refName, variables);
            LogIntAnimationVariables(aniManagerHolder, refName, variables);
            LogFloatAnimationVariables(aniManagerHolder, refName, variables);
        }
    }

    void LogAnimationVariables(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, std::vector<std::string> variables, int type) {
        if (!gfuncs::IsFormValid(ref)) {
            logger::warn("ref doesn't exist");
            return;
        }

        std::string refName = std::string(gfuncs::GetFormName(ref));

        logger::info("logging animation variables for {}", gfuncs::GetFormDataString(ref));

        RE::IAnimationGraphManagerHolder* aniManagerHolder = static_cast<RE::IAnimationGraphManagerHolder*>(ref);
        if (aniManagerHolder) {
            switch (type) {
                case 0: { 
                    LogBoolAnimationVariables(aniManagerHolder, refName, variables);
                    break;
                }
                case 1: {
                    LogIntAnimationVariables(aniManagerHolder, refName, variables);
                    break;
                }
                case 2: {
                    LogFloatAnimationVariables(aniManagerHolder, refName, variables);
                    break;
                }
                default: {
                    LogAllTypeAnimationVariables(aniManagerHolder, refName, variables);
                    break;
                }
            }
        }
    }

    std::vector<std::string> GetAnimationGraphManagerCharacterProperties(RE::BSAnimationGraphManager* manager, std::string refName) {
        std::vector<std::string> CharacterProperties;

        if (manager) {
            if (IsBadReadPtr(manager, sizeof(manager))) {
                logger::warn("[{}] manager IsBadReadPtr", refName);
                return CharacterProperties;
            }

            logger::info("[{}] ref manager->graphs size[{}]", refName, manager->graphs.size());

            if (manager->graphs.size() > 0) {
                int i = 0;
                for (auto& graph : manager->graphs) {
                    if (graph) {
                        //logger::info("[{}] i[{}] graph found", refName, i);

                        auto* graphPtr = graph.get();
                        if (graphPtr) {
                            //logger::info("[{}] i[{}] graphPtr found", refName, i);

                            auto* behaviorGraph = graphPtr->behaviorGraph;
                            if (behaviorGraph) {
                                //logger::info("[{}] i[{}] behaviorGraph found", refName, i);

                                auto graphData = behaviorGraph->data;
                                if (graphData) {
                                    //logger::info("[{}] i[{}] graphData found", refName, i);

                                    auto* data = graphData.get();
                                    if (data) {
                                        //logger::info("[{}] i[{}] data found", refName, i);

                                        auto stringData = data->stringData;
                                        if (stringData) {
                                            //logger::info("[{}] i[{}] stringData found", refName, i);

                                            auto characterPropertyNames = stringData->characterPropertyNames;

                                            logger::info("[{}] i[{}] characterPropertyNames size[{}]", refName, i, characterPropertyNames.size());
                                            if (characterPropertyNames.size() > 0) {
                                                for (auto& name : characterPropertyNames) {
                                                    auto* cname = name.c_str();

                                                    if (cname) {
                                                        CharacterProperties.push_back(cname);
                                                        //logger::info("[{}] ref graph variable[{}]", refName, cname);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    i++;
                }
            }
        }
        gfuncs::RemoveDuplicates(CharacterProperties);
        return CharacterProperties;
    }

    std::vector<std::string> GetAnimationGraphManagerAttributes(RE::BSAnimationGraphManager* manager, std::string refName) {
        std::vector<std::string> attributes;

        if (manager) {
            if (IsBadReadPtr(manager, sizeof(manager))) {
                logger::warn("[{}] manager IsBadReadPtr", refName);
                return attributes;
            }

            logger::info("[{}] ref manager->graphs size[{}]", refName, manager->graphs.size());

            if (manager->graphs.size() > 0) {
                int i = 0;
                for (auto& graph : manager->graphs) {
                    if (graph) {
                        //logger::info("[{}] i[{}] graph found", refName, i);

                        auto* graphPtr = graph.get();
                        if (graphPtr) {
                            //logger::info("[{}] i[{}] graphPtr found", refName, i);

                            auto* behaviorGraph = graphPtr->behaviorGraph;
                            if (behaviorGraph) {
                                //logger::info("[{}] i[{}] behaviorGraph found", refName, i);

                                auto graphData = behaviorGraph->data;
                                if (graphData) {
                                    //logger::info("[{}] i[{}] graphData found", refName, i);

                                    auto* data = graphData.get();
                                    if (data) {
                                        //logger::info("[{}] i[{}] data found", refName, i);

                                        auto stringData = data->stringData;
                                        if (stringData) {
                                            //logger::info("[{}] i[{}] stringData found", refName, i);

                                            auto attributeNames = stringData->attributeNames;

                                            logger::info("[{}] i[{}] attributeNames size[{}]", refName, i, attributeNames.size());
                                            if (attributeNames.size() > 0) {
                                                for (auto& name : attributeNames) {
                                                    auto* cname = name.c_str();

                                                    if (cname) {
                                                        attributes.push_back(cname);
                                                        //logger::info("[{}] ref graph variable[{}]", refName, cname);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    i++;
                }
            }
        }
        gfuncs::RemoveDuplicates(attributes);
        return attributes;
    }

    std::vector<std::string> GetAnimationGraphManagerEvents(RE::BSAnimationGraphManager* manager, std::string refName) {
        std::vector<std::string> events;

        if (manager) {
            if (IsBadReadPtr(manager, sizeof(manager))) {
                logger::warn("[{}] manager IsBadReadPtr", refName);
                return events;
            }

            logger::info("[{}] ref manager->graphs size[{}]", refName, manager->graphs.size());

            if (manager->graphs.size() > 0) {
                int i = 0;
                for (auto& graph : manager->graphs) {
                    if (graph) {
                        //logger::info("[{}] i[{}] graph found", refName, i);

                        auto* graphPtr = graph.get();
                        if (graphPtr) {
                            //logger::info("[{}] i[{}] graphPtr found", refName, i);

                            auto* behaviorGraph = graphPtr->behaviorGraph;
                            if (behaviorGraph) {
                                //logger::info("[{}] i[{}] behaviorGraph found", refName, i);

                                auto graphData = behaviorGraph->data;
                                if (graphData) {
                                    //logger::info("[{}] i[{}] graphData found", refName, i);

                                    auto* data = graphData.get();
                                    if (data) {
                                        //logger::info("[{}] i[{}] data found", refName, i);

                                        auto stringData = data->stringData;
                                        if (stringData) {
                                            //logger::info("[{}] i[{}] stringData found", refName, i);
                                            
                                            auto eventNames = stringData->eventNames;

                                            logger::info("[{}] i[{}] eventNames size[{}]", refName, i, eventNames.size());
                                            if (eventNames.size() > 0) {
                                                for (auto& name : eventNames) {
                                                    auto* cname = name.c_str();

                                                    if (cname) {
                                                        events.push_back(cname);
                                                        //logger::info("[{}] ref graph variable[{}]", refName, cname);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    i++;
                }
            }
        }
        gfuncs::RemoveDuplicates(events);
        return events;
    }

    std::vector<std::string> GetAnimationGraphManagerVariables(RE::BSAnimationGraphManager* manager, RE::BSTArray<RE::AnimVariableCacheInfo>& actorVariablesCache, std::string refName) {
        std::vector<std::string> variables;

        if (manager) {
            if (IsBadReadPtr(manager, sizeof(manager))) {
                logger::warn("[{}] manager IsBadReadPtr", refName);
                return variables;
            }
            
            logger::info("[{}] ref manager->graphs size[{}]", refName, manager->graphs.size());

            if (manager->graphs.size() > 0) {
                int i = 0;
                for (auto& graph : manager->graphs) {
                    if (graph) {
                        //logger::info("[{}] i[{}] graph found", refName, i);

                        auto* graphPtr = graph.get();
                        if (graphPtr) {
                            //logger::info("[{}] i[{}] graphPtr found", refName, i);

                            auto* behaviorGraph = graphPtr->behaviorGraph;
                            if (behaviorGraph) {
                                //logger::info("[{}] i[{}] behaviorGraph found", refName, i);

                                auto graphData = behaviorGraph->data;
                                if (graphData) {
                                    //logger::info("[{}] i[{}] graphData found", refName, i);

                                    auto* data = graphData.get();
                                    if (data) {
                                        //logger::info("[{}] i[{}] data found", refName, i);

                                        auto stringData = data->stringData;
                                        if (stringData) {
                                            //logger::info("[{}] i[{}] stringData found", refName, i);

                                            auto variableNames = stringData->variableNames;
                                            //logger::info("[{}] i[{}] variableNames size[{}]", refName, i, variableNames.size());
                                            if (variableNames.size() > 0) {
                                                for (auto& name : variableNames) {
                                                    auto* cname = name.c_str();
                                                    
                                                    if (cname) { 
                                                        variables.push_back(cname);
                                                        //logger::info("[{}] ref graph variable[{}]", refName, cname);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    i++;
                }
            }

            logger::info("[{}] ref variableCache size[{}]", refName, manager->variableCache.variableCache.size());
            if (manager->variableCache.variableCache.size() > 0) {
                for (uint32_t i = 0; i < manager->variableCache.variableCache.size(); i++) {
                    auto value = manager->variableCache.variableCache[i];
                    std::string sVariable = std::string(value.variableName);
                    if (sVariable != "") {
                        variables.push_back(sVariable);
                    } 

                    //if (value.variable) {
                    //    if (IsBadReadPtr(value.variable, sizeof(value.variable))) {
                    //        logger::warn("[{}] ref variable[{}] IsBadReadPtr", refName, i);
                    //        continue;
                    //    }
                    //    else {
                    //        std::string sVariable = std::string(value.variableName);
                    //        if (sVariable != "") {
                    //            variables.push_back(sVariable);
                    //        }
                    //        //logger::info("[{}] ref variable[{}] bool[{}] int[{}] float[{}]", refName, value.variableName, value.variable->b, value.variable->i, value.variable->f);
                    //    }
                    //}
                    //else {
                    //    logger::warn("[{}] ref variable[{}] value is nullptr", refName, value.variableName);
                    //}
                }
            }

            logger::info("[{}] ref bound channels size[{}]", refName, manager->boundChannels.size());
            for (uint32_t i = 0; i < manager->boundChannels.size(); i++) {
                auto channel = manager->boundChannels[i];
                if (channel) {
                    auto* channelPtr = channel.get();
                    if (channelPtr) {
                        std::string sVariable = std::string(channelPtr->channelName);
                        if (sVariable != "") {
                            variables.push_back(sVariable);
                        }
                        //logger::info("[{}] ref bound channel[{}] value[{}]", refName, variable, channelPtr->value);
                    }
                }
            }

            //logger::info("[{}] ref bumped channels size[{}]", refName, manager->bumpedChannels.size());
            //for (uint32_t i = 0; i < manager->bumpedChannels.size(); i++) {
            //    auto channel = manager->bumpedChannels[i]; //
            //    if (channel) {
            //        auto* channelPtr = channel.get();
            //        if (channelPtr) {
            //            logger::info("[{}] ref bumped channel[{}] value[{}]", refName, channelPtr->channelName, channelPtr->value);
            //        }
            //    }
            //}

            //causes ctd to log sub manager
            /*if (manager->subManagers.size() > 0) {
                logger::info("logging subManagers for [{}] size[{}]", refName, manager->subManagers.size());
                for (uint32_t i = 0; i < manager->subManagers.size(); i++) {
                    logger::info("logging subManager for [{}] i[{}]", refName, i);
                    auto subManager = manager->subManagers[i];
                    if (subManager) {
                        LogAnimationGraphManager(subManager.get(), refName);
                    }
                }
            }*/
        }
        else {
            logger::warn("manager not found", refName);
        }

        logger::info("[{}] actorVariablesCache size[{}]", refName, actorVariablesCache.size());

        if (actorVariablesCache.size() > 0) {
            for (uint32_t i = 0; i < actorVariablesCache.size(); i++) {
                auto value = actorVariablesCache[i];
                std::string sVariable = std::string(value.variableName);
                if (sVariable != "") {
                    variables.push_back(sVariable);
                } 

                //if (value.variable) {
                //    if (IsBadReadPtr(value.variable, sizeof(value.variable))) {
                //        logger::warn("[{}] actor variable[{}] IsBadReadPtr", refName, i);
                //        continue;
                //    }
                //    else {
                //        
                //        //logger::info("[{}] ref variable[{}] bool[{}] int[{}] float[{}]", refName, value.variableName, value.variable->b, value.variable->i, value.variable->f);
                //    }
                //}
                //else {
                //    logger::warn("[{}] actor variable[{}] value is nullptr", refName, value.variableName);
                //}
            }
        }

        gfuncs::RemoveDuplicates(variables);
        return variables;
    }

    void LogAllAnimationsCharacterProperties(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
        if (!gfuncs::IsFormValid(ref)) {
            logger::warn("ref doesn't exist");
            return;
        }
        std::string refName = std::string(gfuncs::GetFormName(ref));
        logger::info("logging all animation character properties for {}", gfuncs::GetFormDataString(ref));

        std::vector<std::string> properties = {};
        auto graphData = GetAnimationGraphData(ref);
        if (graphData.manager) {
            properties = GetAnimationGraphManagerCharacterProperties(graphData.manager, refName);
        }

        logger::info("[{}] total number of properties found[{}]", refName, properties.size());
        if (properties.size() > 0) {
            for (auto& event : properties) {
                logger::info("[{}] property[{}]", refName, event);
            }
        }
    }

    void LogAllAnimationsAttributes(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
        if (!gfuncs::IsFormValid(ref)) {
            logger::warn("ref doesn't exist");
            return;
        }
        std::string refName = std::string(gfuncs::GetFormName(ref));
        logger::info("logging all animation attributes for {}", gfuncs::GetFormDataString(ref));

        std::vector<std::string> attributes = {};
        auto graphData = GetAnimationGraphData(ref);
        if (graphData.manager) {
            attributes = GetAnimationGraphManagerAttributes(graphData.manager, refName);
        }

        logger::info("[{}] total number of attributes found[{}]", refName, attributes.size());
        if (attributes.size() > 0) {
            for (auto& event : attributes) {
                logger::info("[{}] attribute[{}]", refName, event);
            }
        }
    }

    void LogAllAnimations(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
        if (!gfuncs::IsFormValid(ref)) {
            logger::warn("ref doesn't exist");
            return;
        }
        std::string refName = std::string(gfuncs::GetFormName(ref));
        logger::info("logging all animations for {}", gfuncs::GetFormDataString(ref));

        std::vector<std::string> events = {};
        auto graphData = GetAnimationGraphData(ref);
        if (graphData.manager) {
            events = GetAnimationGraphManagerEvents(graphData.manager, refName);
        } 

        logger::info("[{}] total number of animations found[{}]", refName, events.size());
        if (events.size() > 0) {
            for (auto& event : events) {
                logger::info("[{}] animation[{}]", refName, event);
            }
        }
    }

    void LogAllAnimationVariables(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
        if (!gfuncs::IsFormValid(ref)) {
            logger::warn("ref doesn't exist");
            return;
        }

        std::string refName = std::string(gfuncs::GetFormName(ref));
        logger::info("logging all animation variables for {}", gfuncs::GetFormDataString(ref));

        std::vector<std::string> variables = {};
        
        auto graphData = GetAnimationGraphData(ref);
        if (graphData.manager) {
            variables = GetAnimationGraphManagerVariables(graphData.manager, graphData.actorVariablesCache, refName);
        }

        logger::info("[{}] total number of variables found[{}]", refName, variables.size());

        if (variables.size() > 0) {
            LogAllTypeAnimationVariables(graphData.managerHolder, refName, variables);
        }
    }

    std::string GetBase3DNodeNameForRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool firstPerson) {
        std::string nodeName = "";

        if (!gfuncs::IsFormValid(ref)) {
            logger::warn("ref doesn't exist");
            return nodeName;
        }

        RE::NiAVObject* niObj;
        if (firstPerson) {
            niObj = ref->Get3D1(true);
        }
        else {
            niObj = ref->Get3D();
        }

        auto refDataString = gfuncs::GetFormDataString(ref);

        if (!niObj) {
            logger::warn("Couldn't get 3d for [{}]", refDataString);
            return nodeName;
        }

        RE::NiObjectNET* baseNiNet = niObj;

        if (baseNiNet) {
            logger::info("[{}], node[{}]", refDataString, baseNiNet->name);
            nodeName = (std::string(baseNiNet->name));
        }
        else {
            logger::info("[{}], baseNiNet not found", refDataString);
        }

        return nodeName;
    }

    std::vector<std::string> GetAll3DNodeNamesForRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool firstPerson) {
        std::vector<std::string> nodeNames;

        if (!gfuncs::IsFormValid(ref)) {
            logger::warn("ref doesn't exist");
            return nodeNames;
        }

        RE::NiAVObject* niObj;
        if (firstPerson) {
            niObj = ref->Get3D1(true);
        }
        else {
            niObj = ref->Get3D();
        }

        auto refDataString = gfuncs::GetFormDataString(ref);
        auto refName = gfuncs::GetFormName(ref);

        if (!niObj) {
            logger::warn("couldn't get 3d for [{}]", refDataString);
            return nodeNames;
        }

        RE::NiObjectNET* baseNiNet = niObj;

        logger::info("Getting 3d node names for [{}]", refDataString);

        if (baseNiNet) {
            logger::info("[{}], node[{}]", refName, baseNiNet->name);
            nodeNames.push_back(std::string(baseNiNet->name));
        }
        else {
            logger::info("[{}], baseNiNet not found", refDataString);
        }

        RE::BSVisit::TraverseScenegraphObjects(niObj, [&](RE::NiAVObject* childNodeObj) {
            RE::NiObjectNET* niNet = childNodeObj;
            if (niNet) {
                logger::info("[{}], node[{}]", refName, niNet->name);
                nodeNames.push_back(std::string(niNet->name));
            }

            return RE::BSVisit::BSVisitControl::kContinue;
        });

        
        gfuncs::RemoveDuplicates(nodeNames);
        return nodeNames;
    }
    
    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("LogAllAnimationsCharacterProperties", "DbSkseFunctions", LogAllAnimationsCharacterProperties);
        vm->RegisterFunction("LogAllAnimationsAttributes", "DbSkseFunctions", LogAllAnimationsAttributes);
        vm->RegisterFunction("LogAnimationVariables", "DbSkseFunctions", LogAnimationVariables);
        vm->RegisterFunction("LogAllAnimationVariables", "DbSkseFunctions", LogAllAnimationVariables);
        vm->RegisterFunction("LogAllAnimations", "DbSkseFunctions", LogAllAnimations);
        vm->RegisterFunction("GetBase3DNodeNameForRef", "DbSkseFunctions", GetBase3DNodeNameForRef);
        vm->RegisterFunction("GetAll3DNodeNamesForRef", "DbSkseFunctions", GetAll3DNodeNamesForRef);
        return true;
    }
}