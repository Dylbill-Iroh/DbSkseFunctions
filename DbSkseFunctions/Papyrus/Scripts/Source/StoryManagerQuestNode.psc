scriptname StoryManagerQuestNode extends StoryManagerNodeBase 

; Get all StoryManagerQuestNodes.
; if storyManagerEventNodeName is not "", only gets nodes where their StoryManagerEventNode parent matches the storyManagerEventNodeName. 
; Otherwise, gets all nodes in game.
StoryManagerQuestNode[] Function GetAllNodes(string storyManagerEventNodeName) Global Native

; Get all StoryManagerQuestNodes that contain the akQuest 
; Most likely will return an array with 1 node.
StoryManagerQuestNode[] Function GetQuestNodes(Quest akQuest) Global Native

;get all quests in this quest node
Quest[] Function GetQuests() native

; getters =============================================================================================

bool function DoAllBeforeRepeating() native 

bool function SharesEvent() native 

bool function NumQuestsToRunChecked() native

int function GetNumQuestsToRun() native

; A return value of 0 means the "Max Concurrent Quests" box is not checked.
int function GetMaxConcurrentQuests() native

; setters =============================================================================================
; setters functions are not save persistent so use the Event OnLoadGameGlobal() to maintain them.

bool function SetDoAllBeforeRepeating(bool doAll) native 

bool function SetSharesEvent(bool share) native 

bool function SetNumQuestsToRunChecked(bool checked) native

; Setting to any value other than 1 won't work unless NumQuestsToRunChecked is true. 
; Use SetNumQuestsToRunChecked(true) first to ensure this is successful
bool function SetNumQuestsToRun(int toRun) native

; Setting to 0 means the "Max Concurrent Quests" box is not checked.
bool function SetMaxConcurrentQuests(int numOfQuests) native
