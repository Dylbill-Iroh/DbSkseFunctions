scriptname StoryManagerBranchNode extends StoryManagerNodeBase

; Get all StoryManagerBranchNodes. 
; if storyManagerEventNodeName is not "", only gets nodes where their StoryManagerEventNode parent matches the storyManagerEventNodeName. 
; Otherwise, gets all nodes in game.
StoryManagerBranchNode[] Function GetAllNodes(string storyManagerEventNodeName) Global Native

; get the number of child branches for this node, 1 level down
int function GetNumOfChildren() native

; get the child branches for this node, 1 level down
; these forms will have the StoryManagerNodeBase script attached, as well as another specific StoryManager type. 
; Can't return StoryManagerNodeBase directly because internally StoryManagerNodeBase does not have a formtype and can't be bound directly.
; Use GetChildren() to return an array of StoryManagerNodeBase (recommended)
Form[] function GetChildrenForms() native 

; get the child branches for this node, 1 level down
StoryManagerNodeBase[] function GetChildren()
	return DbStoryManagerNodeBaseArray.AsStoryManagerNodeBaseArray(GetChildrenForms())
EndFunction