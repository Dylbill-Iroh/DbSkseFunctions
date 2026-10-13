scriptname StoryManagerInitializer extends actor 
; attached to the player from c++.
; Using StoryManagerNodeBase.GetParent() or GetParents() will sometimes get a none papyrus form. 
; For some reason, getting all nodes in papyrus fixes this issue, hence this script.

Event OnInit()
	InitNodes(true)
EndEvent 

Event OnLoadGameGlobal()
	InitNodes(false)
EndEvent

Function InitNodes(bool log)
	if log
		if !debug.TraceUser("DbSkseLog", "")
			debug.OpenUserLog("DbSkseLog") ;only open user log if it isn't already open
		Endif
	Endif

	;get all event nodes, ensuring StoryManagerNodeBase.GetParent() or GetParents() functions work.
	StoryManagerEventNode[] allEventNodes = StoryManagerEventNode.GetAllEventNodes()
	StoryManagerBranchNode[] allBranchNodes = StoryManagerBranchNode.GetAllNodes("")
	StoryManagerQuestNode[] allQuestNodes = StoryManagerQuestNode.GetAllNodes("")
	
	if log
		string msg = (\
			"\nStoryManagerInitializer: allEventNodes:  size[" + allEventNodes.Length + "] nodes" + allEventNodes + \
			"\nStoryManagerInitializer: allBranchNodes: size[" + allBranchNodes.Length + "] nodes" + allBranchNodes + \
			"\nStoryManagerInitializer: allQuestNodes:  size[" + allQuestNodes.Length + "] nodes" + allQuestNodes \
		)
		
		; ExtendedVanillaMenus.MessageBox(msg, html = false)
		Debug.TraceUser("DbSkseLog", msg)
	Endif
EndFunction