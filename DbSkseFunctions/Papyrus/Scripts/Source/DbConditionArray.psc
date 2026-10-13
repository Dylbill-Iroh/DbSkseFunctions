Scriptname DbConditionArray extends Actor 
;This script is only for creating new Condition arrays of variable size. It's attached to the player. 

Condition[] conditionArray 
Form[] TempFormArray

bool property busy = false auto hidden

Event OnLoadGameGlobal()
	busy = false ;safety to prevent infinite loops
EndEvent

Condition[] function CreateConditionArray(int size, Condition filler = none)
	while busy ;guard, this should only be run once at a time
		Utility.WaitMenuMode(0.01) 
	endwhile
	
	busy = true
	
    ;clear / init the placeholder array first
    conditionArray = new Condition[1]
    conditionArray[0] = filler 

    string handle = PapyrusUtilEx.GetFormHandle(self)

    PapyrusUtilEx.ResizeArray(handle, "DbConditionArray", "conditionArray", size, 0)
	
	Condition[] akArray = conditionArray
	conditionArray = new Condition[1] ;clear global scope array before returning
	busy = false
    return akArray 
EndFunction 

Condition[] function CreateArray(int size, condition filler = none) global 
	condition[] conditions
	DbConditionArray arrayScript = Game.GetPlayer() as DbConditionArray
	
	if !arrayScript 
		DbSkseFunctions.AttachScriptToForm("DbConditionArray", Game.GetPlayer())
		; DbSkseFunctions.ExecuteConsoleCommand("player.aps DbConditionArray")
		arrayScript = Game.GetPlayer() as DbConditionArray
	Endif  
	
	if arrayScript 
		; Debug.MessageBox("Array script found")
		; Utility.wait(0.1)
		conditions = arrayScript.CreateConditionArray(size, filler)
	; else 
		; Debug.MessageBox("Array script not found")
	Endif
	
	return conditions
EndFunction

Condition[] function FormArrayAsConditionArray(Form[] ConditionForms)
	if !ConditionForms || ConditionForms.Length == 0
		Condition[] noneArray 
		return noneArray
	Endif
	
	while busy ;guard, this should only be run once at a time
		Utility.WaitMenuMode(0.01)
	endwhile
	busy = true
	
	;clear / init the placeholder array first
    ConditionArray = new Condition[1]
	
	TempFormArray = ConditionForms
    string handle = PapyrusUtilEx.GetFormHandle(self)
	
	;copy the TempFormArray, which is now set to the passed in ConditionForms, to the ConditionArray
	PapyrusUtilEx.CopyArray(handle, "DbConditionArray", "TempFormArray", handle, "DbConditionArray", "ConditionArray")
	 
	Condition[] akArray = conditionArray
	conditionArray = new Condition[1] ;clear global scope array before returning
	TempFormArray = new Form[1] ;clear global scope array before returning
	busy = false
    return akArray 
EndFunction

Condition[] function AsConditionArray(Form[] conditionForms) Global
	condition[] conditions
	DbConditionArray arrayScript = Game.GetPlayer() as DbConditionArray
	
	if !arrayScript 
		DbSkseFunctions.AttachScriptToForm("DbConditionArray", Game.GetPlayer(), true)
		arrayScript = Game.GetPlayer() as DbConditionArray
	Endif 
	
	if arrayScript  
		; this works but is slow, now using PapyrusUtilEx.CopyArray
		; conditions = arrayScript.CreateConditionArray(conditionForms.Length)
		; int i = 0 
		; int L = conditionForms.Length
		; while i < L
		; 	conditions[i] = conditionForms[i] as Condition
		; 	i += 1
		; EndWhile
		
		return arrayScript.FormArrayAsConditionArray(conditionForms)
	Endif
	
	return conditions
EndFunction