scriptname StoryManagerNodeBase extends form
; Can be used with ConditionList.GetStoryManagerConditionListForm(form storyManagerForm) to get the conditions for this node. 

; get the type of this node
; -1 = none, 0 = StoryManagerEventNode, 1 = StoryManagerBranchNode, 2 = StoryManagerQuestNode. 
; Cast as to get the specific node type.
int function GetType() native

; get the parent branch of this node, if any. 
StoryManagerBranchNode function GetParent() native 

; get all parent branches up the tree for this node, if any. The last node in this array will be the top branch.
StoryManagerBranchNode[] function GetParents() native

; Get's the top level StoryManagerEventNode parent for this node. 
; If this node is a StoryManagerEventNode, returns itself as StoryManagerEventNode.
StoryManagerEventNode function GetEventNodeParent() native

; Get is set to random. If false, it's stacked.
bool function GetIsRandom() native 

Bool function GetWarnIfNoChildQuestStarted() native 

; Set to random or not. If false, it'll be stacked.
; Not save persistent so use the Event OnLoadGameGlobal() to maintain.
bool function SetIsRandom(bool random) native 

; Not save persistent so use the Event OnLoadGameGlobal() to maintain.
Bool function SetWarnIfNoChildQuestStarted(bool warn) native 

