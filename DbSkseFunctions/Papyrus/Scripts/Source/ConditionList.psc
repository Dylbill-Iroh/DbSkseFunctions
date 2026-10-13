Scriptname ConditionList Extends Form 
; This script allows to get or create conditionLists. Example: 
; ConditionList magEffectList = GetMagicEffectConditionList(MyMagicEffect)
; Condition[] conditions = magEffectList.GetConditions() ;get an array of all conditions for MyMagicEffect.

; Global functions ================================================================================================================================
; Native global function getters get conditions as forms as it's not possible to return new papyrus types from c++ without extensive hooking. 
; There are non native functions to return conditions as condition type for convenience.

; Create a new form with the ConditionList script attached with one Condition with the conditionFunction, comparison and value in the list.
; Use the Create function to return form as ConditionList (recommended).
; int comparison options are: 
; 0 = "==" 
; 1 = "!="
; 2 = ">" 
; 3 = ">=" 
; 4 = "<" 
; 5 = "<="
; Use ConditionList.GetConditions() to get the conditions in an array. Array will be length of 1 with the single condition in the list.
; See below for valid conditionFunctions.
Form Function CreateConditionListForm(int conditionFunction, int comparison = 0, float value = 1.0) Global Native 

; Create a new condition.
; int comparison options are: 
; 0 = "==" 
; 1 = "!="
; 2 = ">" 
; 3 = ">=" 
; 4 = "<" 
; 5 = "<="
; Use ConditionList.GetConditions() to get the conditions in an array. Array will be length of 1 with the single condition in the list.
; See below for valid conditionFunctions.
ConditionList Function Create(int conditionFunction, int comparison = 0, float value = 1.0) global
	return CreateConditionListForm(conditionFunction, comparison, value) as ConditionList
EndFunction

; Conditionlist getters

; MagicEffect =============================================================================================================================

; Get a form with the ConditionList script attached that contains all the Conditions on the akMagicEffect. 
; Use GetMagicEffectConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetMagicEffectConditionListForm(MagicEffect akMagicEffect) Global Native 

; Get ConditionList on the magic effect.
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetMagicEffectConditionList(MagicEffect akMagicEffect) Global
	return GetMagicEffectConditionListForm(akMagicEffect) as ConditionList
EndFunction

; Get a form with the ConditionList script attached that contains all the Conditions for the nth magic effect on the magic item.
; The magicItem must be a spell, scroll, potion, or enchantment
; Use GetNthMagicEffectConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetNthMagicEffectConditionListForm(Form magicItem, int nthIndex = 0) Global Native 

; Get the ConditionList that contains all the Conditions for the nth magic effect on the magic item.
; The magicItem must be a spell, scroll, potion, or enchantment
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetNthMagicEffectConditionList(Form magicItem, int nthIndex = 0) Global
	return GetNthMagicEffectConditionListForm(magicItem, nthIndex) as ConditionList
EndFunction

; Perk =============================================================================================================================

; Get a form with the ConditionList script attached that contains all the Conditions on the akPerk. 
; Use GetPerkConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetPerkConditionListForm(Perk akPerk) Global Native 

; Get the ConditionList that contains all the Conditions for the akPerk.
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetPerkConditionList(Perk akPerk) Global
	return GetPerkConditionListForm(akPerk) as ConditionList
EndFunction

; Get a form with the ConditionList script attached that contains all the Conditions for the nth perk entry on the akPerk.
; if perkOwner is true, gets the list for the perk entry owner, otherwise gets the list for the perk entry target.
; Use GetNthPerkEntryConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetNthPerkEntryConditionListForm(Perk akPerk, int nthIndex = 0, bool perkOwner = true) Global Native 

; Get conditions on the perk.
ConditionList Function GetNthPerkEntryConditionList(Perk akPerk, int nthIndex = 0, bool perkOwner = true) Global
	return GetNthPerkEntryConditionListForm(akPerk, nthIndex, perkOwner) as ConditionList
EndFunction

; Quest ======================================================================================================================================

; Get a form with the ConditionList script attached that contains dialogue Conditions for the akQuest. 
; This is in the Quest Data tab in the creation kit.
; Use GetQuestConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetQuestConditionListForm(Quest akQuest) Global Native 

; Get a ConditionList that contains dialogue Conditions for the akQuest. 
; This is in the Quest Data tab in the creation kit.
; Get the ConditionList that contains all the Conditions for the akQuest.
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetQuestConditionList(Quest akQuest) Global
	return GetQuestConditionListForm(akQuest) as ConditionList
EndFunction

; Get a form with the ConditionList script attached that contains all the Conditions for the stage on the akQuest.
; If useStageIndex is true (default), gets the actual index as set for the stage, 50, 100 ect. 
; If useStageIndex is false, index is treated as the order in which stages appear in the quest. 
; Example, if you have 3 stages in myQuest, indexes set to 0, 50 and 100, 
; GetQuestStageConditionListForm(myQuest, 50, true) and GetQuestStageConditionListForm(myQuest, 1, false) both return the same conditionList.
; Use GetQuestStageConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetQuestStageConditionListForm(Quest akQuest, int index, bool useStageIndex = true) Global Native 

; Get the ConditionList script attached that contains all the Conditions for the stage on the akQuest.
; If useStageIndex is true (default), gets the actual index as set for the stage, 50, 100 ect. 
; If useStageIndex is false, index is treated as the order in which stages appear in the quest. 
; Example, if you have 3 stages in myQuest, indexes set to 0, 50 and 100, 
; GetQuestStageConditionListForm(myQuest, 50, true) and GetQuestStageConditionListForm(myQuest, 1, false) both return the same conditionList.
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetQuestStageConditionList(Quest akQuest, int index, bool useStageIndex = true) Global
	return GetQuestStageConditionListForm(akQuest, index, useStageIndex) as ConditionList
EndFunction

; Get a form with the ConditionList script attached that contains all the Conditions for the nth target (target index) on the objective on the akQuest.
; If useObjectiveIndex is true (default), gets the actual index of the objective as set for the objective, 50, 100 ect. 
; If useObjectiveIndex is false, index is treated as the order in which objectives appear in the quest. 
; Example, if you have 3 objectives in myQuest, indexes set to 0, 50 and 100, 
; GetQuestObjectiveTargetConditionListForm(myQuest, 50, true, 0) and GetQuestObjectiveTargetConditionListForm(myQuest, 1, false, 0) both return the same conditionList.
; Use GetQuestObjectiveTargetConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
; Use DbSkseFunctions.GetNthQuestObjectiveTargets to get the referenceAlias target that matches the conditionList. Example: 
; ConditionList firstTargetConditions = ConditionList.GetQuestObjectiveTargetConditionList(myQuest, 0, false, 0) 
; ReferenceAlias[] firstObjectiveTargets = DbSkseFunctions.GetNthQuestObjectiveTargets(myQuest, 0, false)
; ReferenceAlias firstTarget = firstObjectiveTargets[0] ;this reference alias is the target that is from firstTargetConditions;
Form Function GetQuestObjectiveTargetConditionListForm(Quest akQuest, int objectiveIndex, bool useObjectiveIndex, int targetIndex) Global Native

; Get a ConditionList that contains all the Conditions for the nth target (target index) on the objective on the akQuest.
; If useObjectiveIndex is true (default), gets the actual index of the objective as set for the objective, 50, 100 ect. 
; If useObjectiveIndex is false, index is treated as the order in which objectives appear in the quest. 
; Example, if you have 3 objectives in myQuest, indexes set to 0, 50 and 100, 
; GetQuestObjectiveTargetConditionList(myQuest, 50, true, 0) and GetQuestObjectiveTargetConditionList(myQuest, 1, false, 0) both return the same conditionList.
; Use ConditionList.GetConditions() to get the conditions in an array.
; Use DbSkseFunctions.GetNthQuestObjectiveTargets to get the referenceAlias target that matches the conditionList. Example: 
; ConditionList firstTargetConditions = ConditionList.GetQuestObjectiveTargetConditionList(myQuest, 0, false, 0) 
; ReferenceAlias[] firstObjectiveTargets = DbSkseFunctions.GetNthQuestObjectiveTargets(myQuest, 0, false)
; ReferenceAlias firstTarget = firstObjectiveTargets[0] ;this reference alias is the target that is from firstTargetConditions;
ConditionList Function GetQuestObjectiveTargetConditionList(Quest akQuest, int objectiveIndex, bool useObjectiveIndex, int targetIndex) Global
	return GetQuestObjectiveTargetConditionListForm(akQuest, objectiveIndex, useObjectiveIndex, targetIndex) as ConditionList
EndFunction

; Story Manager ===================================================================================================================================

; Get a ConditionList form with the ConditionList script attached that contains all the Conditions for the storyManagerForm. 
; storyManagerForm must be a quest or a StoryManagerNodeBase which includes StoryManagerQuestNode, StoryManagerBranchNode and StoryManagerEventNode types.
; use GetStoryManagerConditionList to return the form as a ConditionList. (Recommended)
Form Function GetStoryManagerConditionListForm(form storyManagerForm) Global Native

; Get a ConditionList that contains all the Conditions for the storyManagerForm. 
; storyManagerForm must be a quest or a StoryManagerNodeBase which includes StoryManagerQuestNode, StoryManagerBranchNode and StoryManagerEventNode types.
ConditionList Function GetStoryManagerConditionList(form storyManagerForm)  Global
	return GetStoryManagerConditionListForm(storyManagerForm) as ConditionList
EndFunction

; TopicInfo ===================================================================================================================================

; Use DbSkseFunctions.GetTopicInfosInQuest or GetTopicInfosInTopic for getting topicInfos

; Get a form with the ConditionList script attached that contains all the Conditions on the aktopicInfo. 
; Use GettopicInfoConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetTopicInfoConditionListForm(topicInfo aktopicInfo) Global Native 

; Get the ConditionList that contains all the Conditions for the aktopicInfo.
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetTopicInfoConditionList(topicInfo aktopicInfo) Global
	return GetTopicInfoConditionListForm(aktopicInfo) as ConditionList
EndFunction

; Scene ===================================================================================================================================

; Use DbSkseFunctions.GetScenesInQuest or GetScenesInTopic for getting Scenes

; Get a form with the ConditionList script attached that contains all the Conditions on the akScene. 
; Use GetSceneConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetSceneConditionListForm(Scene akScene) Global Native 

; Get the ConditionList that contains all the Conditions for the akScene.
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetSceneConditionList(Scene akScene) Global
	return GetSceneConditionListForm(akScene) as ConditionList
EndFunction

; Get a form with the ConditionList script attached that contains all the start Conditions for the akScene's nth phase. 
; Use GetNthScenePhaseStartConditionList to return the form as a ConditionList. (Recommended)
; Use DbSkseFunctions.GetNumOfScenePhases(akScene) to get total number of scenes. Valid scene phase nthIndexes will be between 0 and (numOfPhases - 1)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetNthScenePhaseStartConditionListForm(Scene akScene, int nthIndex) Global Native 

; Get a ConditionList that contains all the start Conditions for the akScene's nth phase. 
; Use DbSkseFunctions.GetNumOfScenePhases(akScene) to get total number of scenes. Valid scene phase nthIndexes will be between 0 and (numOfPhases - 1)
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetNthScenePhaseStartConditionList(Scene akScene, int nthIndex) Global
	return GetNthScenePhaseStartConditionListForm(akScene, nthIndex) as ConditionList
EndFunction

; Get a form with the ConditionList script attached that contains all the end / completion conditions for the akScene's nth phase. 
; Use GetNthScenePhaseEndConditionList to return the form as a ConditionList. (Recommended)
; Use DbSkseFunctions.GetNumOfScenePhases(akScene) to get total number of scenes. Valid scene phase nthIndexes will be between 0 and (numOfPhases - 1)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetNthScenePhaseEndConditionListForm(Scene akScene, int nthIndex) Global Native 

; Get a ConditionList that contains all the end / completion conditions for the akScene's nth phase. 
; Use DbSkseFunctions.GetNumOfScenePhases(akScene) to get total number of scenes. Valid scene phase nthIndexes will be between 0 and (numOfPhases - 1)
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetNthScenePhaseEndConditionList(Scene akScene, int nthIndex) Global
	return GetNthScenePhaseEndConditionListForm(akScene, nthIndex) as ConditionList
EndFunction

; Message ======================================================================================================================================

; Get a form with the ConditionList script attached that contains all the conditions for the nth akMessage menu button.
; Use GetNthMessageMenuButtonConditionList to return the form as a ConditionList. (Recommended)
; Use DbSkseFunctions.GetNumOfMessageMenuButtons(akMessage) to get total number of message menu buttons. Valid Message menu button nthIndexes will be between 0 and (numOfButtons - 1)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetNthMessageMenuButtonConditionListForm(Message akMessage, int nthIndex) Global Native 

; Get a ConditionList that contains all the conditions for the nth akMessage menu button.
; Use DbSkseFunctions.GetNumOfMessageMenuButtons(akMessage) to get total number of message menu buttons. Valid Message menu button nthIndexes will be between 0 and (numOfButtons - 1)
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetNthMessageMenuButtonConditionList(Message akMessage, int nthIndex) Global
	return GetNthMessageMenuButtonConditionListForm(akMessage, nthIndex) as ConditionList
EndFunction

; MusicType Track ======================================================================================================================================

; Get a form with the ConditionList script attached that contains all the conditions for the nth music track in the akMusicType.
; Use GetNthMusicTypeTrackConditionList to return the form as a ConditionList. (Recommended)
; Use DbSkseFunctions.GetNumberOfTracksInMusicType(akMusicType) to get total number of MusicType menu buttons. Valid MusicType menu button nthIndexes will be between 0 and (numOfTracks - 1)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetNthMusicTypeTrackConditionListForm(MusicType akMusicType, int nthIndex) Global Native 

; Get a ConditionList that contains all the conditions for the nth music track in the akMusicType.
; Use DbSkseFunctions.GetNumberOfTracksInMusicType(akMusicType) to get total number of MusicType menu buttons. Valid MusicType menu button nthIndexes will be between 0 and (numOfTracks - 1)
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetNthMusicTypeTrackConditionList(MusicType akMusicType, int nthIndex) Global
	return GetNthMusicTypeTrackConditionListForm(akMusicType, nthIndex) as ConditionList
EndFunction

; Package ======================================================================================================================================

; Get a form with the ConditionList script attached that contains all the Conditions on the akPackage. 
; Use GetPackageConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetPackageConditionListForm(Package akPackage) Global Native 

; Get the ConditionList that contains all the Conditions for the akPackage.
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetPackageConditionList(Package akPackage) Global
	return GetPackageConditionListForm(akPackage) as ConditionList
EndFunction

; Idle ======================================================================================================================================

; Get a form with the ConditionList script attached that contains all the Conditions on the akIdle. 
; Use GetIdleConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetIdleConditionListForm(Idle akIdle) Global Native 

; Get the ConditionList that contains all the Conditions for the akIdle.
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetIdleConditionList(Idle akIdle) Global
	return GetIdleConditionListForm(akIdle) as ConditionList
EndFunction

; Faction ======================================================================================================================================

; Get a form with the ConditionList script attached that contains all the Conditions on the akFaction. 
; Use GetFactionVendorConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetFactionVendorConditionListForm(Faction akFaction) Global Native 

; Get the ConditionList that contains all the Conditions for the akFaction.
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetFactionVendorConditionList(Faction akFaction) Global
	return GetFactionVendorConditionListForm(akFaction) as ConditionList
EndFunction

; Alias ======================================================================================================================================

; Get a form with the ConditionList script attached that contains all the Conditions on the akAlias. 
; Use GetAliasConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetAliasConditionListForm(Alias akAlias) Global Native 

; Get the ConditionList that contains all the Conditions for the akAlias.
; Use ConditionList.GetConditions() to get the conditions in an array.
ConditionList Function GetAliasConditionList(Alias akAlias) Global
	return GetAliasConditionListForm(akAlias) as ConditionList
EndFunction

; ConstructibleObject =============================================================================================================================

; Get a form with the ConditionList script attached that contains all the Conditions on the akConstructibleObject. 
; Use GetConstructibleObjectConditionList to return the form as a ConditionList. (Recommended)
; Use ConditionList.GetConditions() to get the conditions in an array.
Form Function GetConstructibleObjectConditionListForm(ConstructibleObject akConstructibleObject) Global Native 

; Get conditions on the ConstructibleObject.
ConditionList Function GetConstructibleObjectConditionList(ConstructibleObject akConstructibleObject) Global
	return GetConstructibleObjectConditionListForm(akConstructibleObject) as ConditionList
EndFunction

; Member functions ================================================================================================================================

; Get the type of this condition list.
; Types are: 
; 0 = kNone. Means this list, that was previously created with the Create function, was destroyed and has no conditions. This list is no longer usable.
; 1 = kCreated. Means this list was created with the Create function and is still valid. 
; 2 = kVanilla. Means this list was gotten from a vanilla form, E.G GetPackageConditionList, GetMagicEffectConditionList ect.
int Function GetType() Native

;Does this condition list evaluate to true on the target?
bool Function Evaluate(ObjectReference target) Native 

; Get the number of conditions in this ConditionList.
Int Function GetNumOfConditions() Native

; Get all condition forms in this list. Use GetConditions for an array of Conditions rather than Forms (recommended)
Form[] Function GetConditionForms() Native

; Get all conditions in this list.
Condition[] Function GetConditions() 
	return DbConditionArray.AsConditionArray(GetConditionForms())
Endfunction

; Clone this condition list. Return a new form with the ConditionList script attached with new conditions that match this list's conditons. 
; Use Clone() to return the form as ConditionList (recommended)
Form Function CloneList() Native 

ConditionList Function Clone()
	return CloneList() as ConditionList
EndFunction 

; If this list was gotten from GetAliasConditionList, returns the alias associated with this list.
Alias Function GetAssociatedAlias() Native

; Get the base game form this list is associated with, if any. 
; Example, if this list was gotten from GetPerkConditionListForm, returns the Perk associated with this list.
Form Function GetAssociatedForm() Native

; If this list was gotten from GetNthPerkEntryConditionList or GetNthMagicEffectConditionList ect, returns the NthIndex gotten from.
int Function GetAssociatedIndex() Native

; The ConditionForm must have the Condition script attached. 
; This does not set the actual Condition to this list, but rather copies the data of the Condition to the Nth condition in this list
; because conditions can only be in one list at a time.
; Returns false if the nthIndex is out of range.
Bool Function SetNthCondition(Form ConditionForm, int nthIndex) Native

; Remove / decouple the condition at the nthIndex from this list.
; If destroy is true, destroys this condition form's linked condition, making it invalid and unusable. Conditon.IsValid() will return false.
; Else if the newList is none, this will create a new ConditionList with the removed condition as the sole condition in this list. 
; Else if the newlist exists, puts the removed condition in the newList at the newIndex. If newIndex == -1, adds it to the end of the newList.
; Note if the newList exists by its type is 0 (kNone) meaning it's invalid, or it doesn't have the ConditionList script attached, 
; this function will fail, meaning the condition won't be removed and it'll return none.
; Returns the condition form that was removed, or none if destroy is true. Use Condition.GetConditionList() to get its conditionList.
; Will fail and return none if the nthIndex is out of range.
; Use RemoveNthCondition to return as condition (recommended).
Form Function RemoveNthConditionForm(int nthIndex, Form newConditionList = none, int newIndex = -1, bool destroy = false) Native 

; Remove / decouple the condition at the nthIndex from this list.
; If destroy is true, destroys this condition form's linked condition, making it invalid and unusable. Conditon.IsValid() will return false.
; Else if the newList is none, this will create a new ConditionList with the removed condition as the sole condition in this list. 
; Else if the newlist exists, puts the removed condition in the newList at the newIndex. If newIndex == -1, adds it to the end of the newList.
; Note if the newList exists by its type is 0 (kNone) meaning it's invalid, 
; this function will fail, meaning the condition won't be removed and it'll return none.
; Returns the condition form that was removed, or none if destroy is true. Use Condition.GetConditionList() to get its conditionList.
; Will fail and return none if the nthIndex is out of range.
Condition Function RemoveNthCondition(int nthIndex,  ConditionList newConditionList = none, int newIndex = -1, bool destroy = false)
	return RemoveNthConditionForm(nthIndex, newConditionList, newIndex, destroy) as Condition
EndFunction 

; Insert a condition into this conditionList at nthIndex.
; If nthIndex = -1 (default) adds the condition to the end of the list. 
; Note, this does not insert the actual conditionForm into the list, but rather clones it and adds the clone.
; This is because a condition can only be in one list at a time. 
; You can however, use RemoveNthCondition to actually remove a condition from a list and add it to another.
; Returns the new cloned condition form. Use InsertCondition to return the new form as Condition, (recommended).
Form Function InsertConditionForm(Form ConditionForm, int nthIndex = -1) Native

; Insert a condition into this conditionList at nthIndex. 
; If nthIndex = -1 (default) adds the condition to the end of the list. 
; Note, this does not insert the actual conditionForm into the list, but rather clones it and adds the clone.
; This is because a condition can only be in one list at a time. 
; You can however, use RemoveNthCondition to actually remove a condition from a list and add it to another.
; Returns the new cloned condition.
Condition Function InsertCondition(Condition akCondition, int nthIndex = -1)
	return InsertConditionForm(akCondition, nthIndex) as Condition
EndFunction

; Destroys this condition list. 
; For conditionLists created with this script using the Create function, deletes the internal ConditionList and all its conditions. 
; For ConditionLists gotten from vanilla forms, perk, quest ect, removes all conditions from the list, essentially making it an empty list with no conditions. 
; Also marks the linked ConditionList and Condition forms for delete.
; For ConditionLists gotten from vanilla forms, if you don't want to clear / destroy the conditions but are done using the ConditionList form, just use akConditionList = none
; and the conditionList form will be deleted when able without distroying / clearing the vanilla form's conditions.
bool function Destroy() Native 

; Register for your calling script to recieve OnConditionChanged event
; for when this condition list changes from true to false and vice versa on akTarget
bool Function RegisterForConditionChanged(ObjectReference akTarget) Native

; Unregister for your calling script to recieve OnConditionChanged event
; for when this condition list changes from true to false and vice versa on akTarget, if it was previously registered.
bool Function UnregisterForConditionChanged(ObjectReference akTarget) Native

;put this event in your script that is calling RegisterForConditionChanged or UnregisterForConditionChanged
Event OnConditionChanged(Form conditionListForm, ObjectReference target, bool isTrue)
EndEvent