scriptname StoryManagerEventNode extends StoryManagerBranchNode

; Get all top level StoryManagerEventNodes in game. 
StoryManagerEventNode[] Function GetAllEventNodes() Global Native

; Get the StoryManagerEventNode that matches the eventNodeName, or none if not found.
StoryManagerEventNode function GetEventNode(string eventNodeName) Global Native

string function GetEventName() native 

int function GetNumberOfMembers() native

; types are: 
; -1 = invalid
; 0 = Reference
; 1 = Form
; 2 = Location
; 3 = Integer
; 4 = Keyword
int function GetNthMemberType(int nthIndex) native 

string function GetNthMemberName(int nthIndex) native