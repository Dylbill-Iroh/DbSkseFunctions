scriptname DbCondition extends ObjectReference 
;this is just a wrapper script for DbConditionFunctions, for convenience. 
;param persistence and load game maintenance are handled automatically.
;See the DbConditionFunctions.psc script for more info about events and condition functions.

string property conditionId auto 
int property conditionFunction auto 
int property comparison auto 
float property value auto 

int property paramType0 auto       ;Types, none = 0
bool property boolParam0 auto      ;1
int property intParam0 auto        ;2
int property rawParam0 auto        ;3
float property floatParam0 auto    ;4
string property stringParam0 auto  ;5
form property formParam0 auto      ;6
alias property aliasParam0 auto    ;7

int property paramType1 auto       ;Types, none = 0
bool property boolParam1 auto      ;1
int property intParam1 auto        ;2
int property rawParam1 auto        ;3
float property floatParam1 auto    ;4
string property stringParam1 auto  ;5
form property formParam1 auto      ;6
alias property aliasParam1 auto    ;7

ObjectReference[] Property ConditionEventTargets Auto Hidden

;create a new condition. ConditionId should be unique, so for example use "modPrefix_conditionName".
;int comparison options are: 
;0 = "==" 
;1 = "!="
;2 = ">" 
;3 = ">=" 
;4 = "<" 
;5 = "<="
;If this condition already exists, this will fail and return none to prevent overwritting another condition.
;This creates a new object reference using PlaceAtMe. If baseform is none, it will create a new xmarker ref and attempt to attach this script.
;Safer to Attach this script to a baseform, such as a misc object / item in the creation kit, and pass it in to the baseForm param of this function.
;If placeAtMeRef is none, places new object at the player.
;Be sure to use DestroyCondition before deleting one of these.
DbCondition Function Create(string conditionId, int conditionFunction, int comparison = 0, float value = 1.0, bool persistent = true, bool initiallyDisabled = true, ObjectReference placeAtMeRef = none, Form baseForm = none) Global 
	If DbConditionFunctions.ConditionExists(conditionId)
		Debug.Trace("ConditionId " + conditionId + " already exists", 2)
		return none
	Endif 
	
	;reserve condition asap
	DbConditionFunctions.CreateCondition(conditionId, conditionFunction, comparison, value)
	
	if !baseForm 
		baseForm = Game.GetFormFromFile(0x00003B, "Skyrim.esm") ;xMarker static
		if !baseForm ;xMarker static not found
			Debug.Trace("xmarker form not found", 2)
			DbConditionFunctions.DestroyCondition(conditionId)
			return none 
		Endif
	endif 
	
	if placeAtMeRef == none 
		placeAtMeRef = Game.GetPlayer()
	Endif
	
	ObjectReference ref = placeAtMeRef.PlaceAtMe(baseForm, 1, persistent, initiallyDisabled)
	if !ref 
		Debug.Trace("failed to place ref", 2)
		DbConditionFunctions.DestroyCondition(conditionId)
		return none 
	Endif 
	
	DbCondition condition = ref as DbCondition 
	if !condition 
		;attampt to attach DbCondition script to ref with aps or 'attach papyrus script' console command
		DbSkseFunctions.ExecuteConsoleCommand("aps DbCondition", ref)
		Utility.WaitMenuMode(1)
		condition = ref as DbCondition 
		if !condition ;failed to attach
			Debug.Trace("failed to attach DbCondition script", 2)
			ref.Disable()
			ref.Delete()
			DbConditionFunctions.DestroyCondition(conditionId)
			return none
		Endif
	Endif 
	
	condition.conditionId = conditionId
	condition.conditionFunction = conditionFunction
	condition.comparison = comparison
	condition.value = value
	
	return condition
EndFunction

Event OnInit()
	LoadCondition()
EndEvent 

Event OnLoadGameGlobal()
	LoadCondition()
EndEvent

Function LoadCondition()
	if conditionId == ""
		return 
	Endif 
	
	;already loaded, no need to do anything
	if DbConditionFunctions.ConditionExists(conditionId)
		return
	Endif
	
	DbConditionFunctions.CreateCondition(conditionId, conditionFunction, comparison, value)
	
	if paramType0 == 1 
		DbConditionFunctions.SetConditionParameterBool(conditionId, boolParam0)
	
	elseif paramType0 == 2 
		DbConditionFunctions.SetConditionParameterInt(conditionId, intParam0)
		
	elseif paramType0 == 3 
		DbConditionFunctions.SetConditionParameterRaw(conditionId, rawParam0)
		
	elseif paramType0 == 4 
		DbConditionFunctions.SetConditionParameterFloat(conditionId, floatParam0)
		
	elseif paramType0 == 5 
		DbConditionFunctions.SetConditionParameterString(conditionId, stringParam0)
		
	elseif paramType0 == 6 
		DbConditionFunctions.SetConditionParameterForm(conditionId, formParam0)
		
	elseif paramType0 == 7 
		DbConditionFunctions.SetConditionParameterAlias(conditionId, aliasParam0)
	Endif
	
	if paramType1 == 1 
		DbConditionFunctions.SetConditionParameterBool(conditionId, boolParam1)
	
	elseif paramType1 == 2 
		DbConditionFunctions.SetConditionParameterInt(conditionId, intParam1)
		
	elseif paramType1 == 3 
		DbConditionFunctions.SetConditionParameterRaw(conditionId, rawParam1)
		
	elseif paramType1 == 4 
		DbConditionFunctions.SetConditionParameterFloat(conditionId, floatParam1)
		
	elseif paramType1 == 5 
		DbConditionFunctions.SetConditionParameterString(conditionId, stringParam1)
		
	elseif paramType1 == 6 
		DbConditionFunctions.SetConditionParameterForm(conditionId, formParam1)
		
	elseif paramType1 == 7 
		DbConditionFunctions.SetConditionParameterAlias(conditionId, aliasParam1)
	Endif
EndFunction

;replace this condition with a new one
function ReplaceCondition(string sConditionId, int iConditionFunction, int iComparison = 0, float fValue = 1.0)
	DbConditionFunctions.DestroyCondition(conditionId)
	conditionId = sConditionId 
	conditionFunction = iConditionFunction
	comparison = iComparison 
	value = fValue 
	ConditionEventTargets = new ObjectReference[1] ;clear
	formParam0 = none 
	formParam1 = none 
	aliasParam0 = none 
	aliasParam1 = none 
	paramType0 = 0
	paramType1 = 0
	DbConditionFunctions.CreateCondition(sConditionId, iConditionFunction, iComparison, fValue)
EndFunction

;Run the this condition on the optional target and evaluate. Return true if the condition is met.
bool function EvaluateCondition(ObjectReference target = none) 
	return DbConditionFunctions.EvaluateCondition(conditionId, target)
EndFunction

;Set the nth parameter for this condition to the Bool param. paramIndex must be 0 or 1.
bool function SetConditionParameterBool(Bool param, int paramIndex = 0) 
	if paramIndex == 0 
		paramType0 = 1
		boolParam0 = param
		return DbConditionFunctions.SetConditionParameterbool(conditionId, param, paramIndex)
		
	Elseif paramIndex == 1 
		paramType1 = 1
		boolParam1 = param
		return DbConditionFunctions.SetConditionParameterbool(conditionId, param, paramIndex)
	Endif
EndFunction

;Set the nth parameter for this condition to the Int param. paramIndex must be 0 or 1.
;Int parameters will explicitely say integer in the creation kit, like GetIsCreatureType. 
;Otherwise for enum drop down lists, such as GetIsSex, use SetConditionParameterRaw.
bool function SetConditionParameterInt(Int param, int paramIndex = 0) 
	if paramIndex == 0 
		paramType0 = 2
		intParam0 = param
		return DbConditionFunctions.SetConditionParameterint(conditionId, param, paramIndex)
		
	Elseif paramIndex == 1 
		paramType1 = 2
		intParam1 = param
		return DbConditionFunctions.SetConditionParameterint(conditionId, param, paramIndex)
	Endif
EndFunction

;Many condition functions take small enums/indices stored directly in the
;pointer field rather than as a pointer to a value. 
;GetIsSex, actor value indices, etc. Compare against a vanilla condition's raw
;params to tell which option a given function expects.
bool function SetConditionParameterRaw(Int param, int paramIndex = 0) 
	if paramIndex == 0 
		paramType0 = 3
		rawParam0 = param
		return DbConditionFunctions.SetConditionParameterraw(conditionId, param, paramIndex)
		
	Elseif paramIndex == 1 
		paramType1 = 3
		rawParam1 = param
		return DbConditionFunctions.SetConditionParameterraw(conditionId, param, paramIndex)
	Endif
EndFunction

;Set the nth parameter for this condition to the Float param. paramIndex must be 0 or 1.
bool function SetConditionParameterFloat(Float param, int paramIndex = 0) 
	if paramIndex == 0 
		paramType0 = 4
		floatParam0 = param
		return DbConditionFunctions.SetConditionParameterfloat(conditionId, param, paramIndex)
		
	Elseif paramIndex == 1 
		paramType1 = 4
		floatParam1 = param
		return DbConditionFunctions.SetConditionParameterfloat(conditionId, param, paramIndex)
	Endif
EndFunction

;Set the nth parameter for this condition to the String param. paramIndex must be 0 or 1.
bool function SetConditionParameterString(String param, int paramIndex = 0) 
	if paramIndex == 0 
		paramType0 = 5
		stringParam0 = param
		return DbConditionFunctions.SetConditionParameterstring(conditionId, param, paramIndex)
		
	Elseif paramIndex == 1 
		paramType1 = 5
		stringParam1 = param
		return DbConditionFunctions.SetConditionParameterstring(conditionId, param, paramIndex)
	Endif
EndFunction

;Set the nth parameter for this condition to the form param. paramIndex must be 0 or 1.
bool function SetConditionParameterForm(form param, int paramIndex = 0) 
	if paramIndex == 0 
		paramType0 = 6
		formParam0 = param
		return DbConditionFunctions.SetConditionParameterForm(conditionId, param, paramIndex)
		
	Elseif paramIndex == 1 
		paramType1 = 6
		formParam1 = param
		return DbConditionFunctions.SetConditionParameterForm(conditionId, param, paramIndex)
	Endif
EndFunction

;Set the nth parameter for this condition to the Alias param. paramIndex must be 0 or 1.
bool function SetConditionParameterAlias(Alias param, int paramIndex = 0) 
	if paramIndex == 0 
		paramType0 = 7
		aliasParam0 = param
		return DbConditionFunctions.SetConditionParameteralias(conditionId, param, paramIndex)
		
	Elseif paramIndex == 1 
		paramType1 = 7
		aliasParam1 = param
		return DbConditionFunctions.SetConditionParameteralias(conditionId, param, paramIndex)
	Endif
EndFunction

;Set this condition's comparison for the conditionId. 
;int comparison options are: 
;0 = "==" 
;1 = "!="
;2 = ">" 
;3 = ">=" 
;4 = "<" 
;5 = "<="
bool function SetConditionComparison(int iComparison)
	comparison = iComparison
	return DbConditionFunctions.SetConditionComparison(conditionId, comparison)
EndFunction

;Set this condition's comparison value.
bool function SetConditionValue(float fValue) 
	value = fValue
	return DbConditionFunctions.SetConditionValue(conditionId, value)
EndFunction

;Does the condition event for this condition and target exist?
bool function ConditionEventExists(ObjectReference target = none) 
	return DbConditionFunctions.ConditionEventExists(conditionId, target)
EndFunction

;Create a condition event for this condition on the optional target.
;Returns false if the event already exists or this condition wasn't found.
;The event name for these will be "On" + conditionId + "Changed". 
;example: if you create this condition with the conditionId "MyMod_Condition", the event will be: 
;Event OnMyMod_ConditionChanged(ObjectReference akTarget, bool isTrue)
;The event is sent automatically to any scripts containing the event and doesn't have to be registered for.
Bool function CreateConditionEvent(ObjectReference target = none)
	if DbConditionFunctions.CreateConditionEvent(conditionId, target)
		if target 
			;save target to array to keep persistent and prevent ctds
			ConditionEventTargets = DbSkseArray_ObjectReference.push(ConditionEventTargets, target)
		Endif
		return true
	Endif
	
	return false
EndFunction

;Destroy the condition event previously created for this condition on the optional target.
;Returns true if the event exists and was destroyed.
bool function DestroyConditionEvent(ObjectReference target = none)
	if DbConditionFunctions.DestroyConditionEvent(conditionId, target)
		if target 
			;remove target from the array
			ConditionEventTargets = DbSkseArray_ObjectReference.remove(ConditionEventTargets, target, false)
		Endif
		return true
	Endif
	
	return false
EndFunction

;Destroy all condition events created with the CreateConditionEvent function for this condition and return the number destroyed.
;Note that using DestroyCondition() will DestroyAllConditionEvents for this condition as well
int function DestroyAllConditionEvents()
	ConditionEventTargets = new objectReference[1] ;clear
	return DbConditionFunctions.DestroyAllConditionEvents(conditionId)
EndFunction

;Count how many condition events were created with the CreateConditionEvent function for this condition. 
int function CountConditionEvents()
	return DbConditionFunctions.CountConditionEvents(conditionId)
EndFunction

