Scriptname DbStoryManagerNodeBaseArray extends Actor 

;This script is for creating new StoryManagerNodeBase arrays of variable size. It's attached to the player.

StoryManagerNodeBase[] StoryManagerNodeBaseArray 
form[] StoryManagerTempFormArray 

bool property busy = false auto hidden

Event OnLoadGameGlobal()
	busy = false ;safety to prevent infinite loops
EndEvent

StoryManagerNodeBase[] function CreateStoryManagerNodeBaseArray(int size, StoryManagerNodeBase filler = none)
	while busy ;guard, this should only be run once at a time
		Utility.WaitMenuMode(0.01)
	endwhile
	
	busy = true
	
    ;clear / init the placeholder array first
    StoryManagerNodeBaseArray = new StoryManagerNodeBase[1]
    StoryManagerNodeBaseArray[0] = filler 

    string handle = PapyrusUtilEx.GetFormHandle(self)

    PapyrusUtilEx.ResizeArray(handle, "DbStoryManagerNodeBaseArray", "StoryManagerNodeBaseArray", size, 0)
	
	StoryManagerNodeBase[] akArray = StoryManagerNodeBaseArray
    StoryManagerNodeBaseArray = new StoryManagerNodeBase[1]
	busy = false
    return akArray 
EndFunction 

StoryManagerNodeBase[] function CreateArray(int size, StoryManagerNodeBase filler = none) global 
	StoryManagerNodeBase[] StoryManagerNodeBases
	DbStoryManagerNodeBaseArray arrayScript = Game.GetPlayer() as DbStoryManagerNodeBaseArray
	
	if !arrayScript 
		DbSkseFunctions.AttachScriptToForm("DbStoryManagerNodeBaseArray", Game.GetPlayer())
		; DbSkseFunctions.ExecuteConsoleCommand("player.aps DbStoryManagerNodeBaseArray")
		arrayScript = Game.GetPlayer() as DbStoryManagerNodeBaseArray
	Endif  
	
	if arrayScript 
		; Debug.MessageBox("Array script found")
		; Utility.wait(0.1)
		StoryManagerNodeBases = arrayScript.CreateStoryManagerNodeBaseArray(size, filler)
	; else 
		; Debug.MessageBox("Array script not found")
	Endif
	
	return StoryManagerNodeBases
EndFunction

StoryManagerNodeBase[] function FormArrayAsNodeBaseArray(Form[] StoryManagerNodeBaseForms)
	if !StoryManagerNodeBaseForms || StoryManagerNodeBaseForms.Length == 0
		StoryManagerNodeBase[] noneArray 
		return noneArray
	Endif
	
	while busy ;guard, this should only be run once at a time
		Utility.WaitMenuMode(0.01)
	endwhile
	busy = true
	
	;clear / init the placeholder array first
    StoryManagerNodeBaseArray = new StoryManagerNodeBase[1]
	
	StoryManagerTempFormArray = StoryManagerNodeBaseForms
    string handle = PapyrusUtilEx.GetFormHandle(self)
	
	;copy the StoryManagerTempFormArray, which is now set to the passed in StoryManagerNodeBaseForms, to the StoryManagerNodeBaseArray
	PapyrusUtilEx.CopyArray(handle, "DbStoryManagerNodeBaseArray", "StoryManagerTempFormArray", handle, "DbStoryManagerNodeBaseArray", "StoryManagerNodeBaseArray")
	
	StoryManagerNodeBase[] akArray = StoryManagerNodeBaseArray
    StoryManagerNodeBaseArray = new StoryManagerNodeBase[1]
	StoryManagerTempFormArray = new Form[1]
	busy = false
    return akArray 
EndFunction

StoryManagerNodeBase[] function AsStoryManagerNodeBaseArray(Form[] StoryManagerNodeBaseForms) Global
	
	StoryManagerNodeBase[] StoryManagerNodeBases
	DbStoryManagerNodeBaseArray arrayScript = Game.GetPlayer() as DbStoryManagerNodeBaseArray
	
	if !arrayScript 
		DbSkseFunctions.AttachScriptToForm("DbStoryManagerNodeBaseArray", Game.GetPlayer(), true)
		arrayScript = Game.GetPlayer() as DbStoryManagerNodeBaseArray
	Endif 
	
	if arrayScript 
		; this works but is slow, now using PapyrusUtilEx.CopyArray to convert types.
		; StoryManagerNodeBases = arrayScript.CreateStoryManagerNodeBaseArray(StoryManagerNodeBaseForms.Length)
		; int i = 0 
		; int L = StoryManagerNodeBaseForms.Length 
		; while i < L
		; 	StoryManagerNodeBases[i] = StoryManagerNodeBaseForms[i] as StoryManagerNodeBase
		; 	i += 1
		; EndWhile
		
		return arrayScript.FormArrayAsNodeBaseArray(StoryManagerNodeBaseForms)
	Endif
	
	return StoryManagerNodeBases
EndFunction