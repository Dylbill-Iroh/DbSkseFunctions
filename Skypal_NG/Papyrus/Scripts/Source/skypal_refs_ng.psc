scriptname skypal_refs_ng hidden
; == new functions added by Dylbill as of version 1.1.0 ==

; [0] = major
; [1] = minor
; [2] = patch
int[] function Get_Version() native global

; if (mode == ""): Passes all refs that are in the akWorldSpace.
; if (mode == "!"): Passes all refs that are not in the akWorldSpace.
ObjectReference[] function Filter_WorldSpace(ObjectReference[] refs, Worldspace akWorldSpace, string mode = "") native global

; if (mode == ""): Passes all refs that are 3d Loaded.
; if (mode == "!"): Passes all refs that are not 3d Loaded.
ObjectReference[] function Filter_3dLoaded(ObjectReference[] refs, string mode = "") native global

; if (mode == ""): Passes all refs that are off limits (flagged as stolen if player takes).
; if (mode == "!"): Passes all refs that are not off limits (not flagged as stolen if player takes).
ObjectReference[] function Filter_OffLimits(ObjectReference[] refs, string mode = "") native global

; if (mode == ""): Passes all refs that are inventory objects (can be added to a container's inventory).
; if (mode == "!"): Passes all refs that are not inventory objects (can not be added to a container's inventory).
ObjectReference[] function Filter_InventoryObjects(ObjectReference[] refs, string mode = "") native global

; if (mode == ""): Passes all refs that are playable.
; if (mode == "!"): Passes all refs that are not playable.
ObjectReference[] function Filter_PlayableObjects(ObjectReference[] refs, string mode = "") native global

; if (mode == ""): Passes all refs that are quest objects.
; if (mode == "!"): Passes all refs that are not quest objects.
ObjectReference[] function Filter_QuestObjects(ObjectReference[] refs, string mode = "") native global

; count how many keywords the ref has in the keywords array.
int function CountNumberOfKeywordsRefHas(ObjectReference ref, Keyword[] keywords) native global

; returns true if the ref has at least 1 keyword in the keywords array.
bool function refHasAtLeastOneKeyword(ObjectReference ref, Keyword[] keywords) native global;*

; if (mode == ""): Passes all keywords that the ref has.
; if (mode == "!"): Passes all keywords that the ref does not have.
keyword[] function filter_keywordsOnRef(ObjectReference ref, Keyword[] keywords, string mode = "") native global ;*

; is the akActor an owner of the ref?
; checks Factions/ActorBase owners on Cell, EncounterZone, Worldspace, and ObjectReference
bool function ActorIsOwnerOfRef(ObjectReference ref, actor akActor) native global

; count how many owners are an owner of the ref. 
; checks Factions/ActorBase owners on Cell, EncounterZone, Worldspace, and ObjectReference
int function CountOwnersForRef(ObjectReference ref, Actor[] owners) native global

; returns true if the ref has at least 1 owner in the owners array.
; checks Factions/ActorBase owners on Cell, EncounterZone, Worldspace, and ObjectReference
bool function RefHasAtLeastOneOwner(ObjectReference ref, Actor[] owners) native global

; if (mode == ""): Passes all actors that are owners of the ref.
; if (mode == "!"): Passes all actors that are not owners of the ref.
; checks Factions/ActorBase owners on Cell, EncounterZone, Worldspace, and ObjectReference
Actor[] function filter_OwnersOnRef(ObjectReference ref, Actor[] owners, string mode = "") native global

; is the akActor a potential thief of the ref?
; checks Factions/ActorBase owners on Cell, EncounterZone, Worldspace, and ObjectReference
bool function ActorIsPotentialThiefOfRef(ObjectReference ref, actor akActor) native global

; count how many of the actors are potential thieves for the ref.
; checks Factions/ActorBase owners on Cell, EncounterZone, Worldspace, and ObjectReference
int function CountPotentialThievesForRef(ObjectReference ref, Actor[] actors) native global

; returns true if the ref has at least 1 potential thief in the actors array.
; checks Factions/ActorBase owners on Cell, EncounterZone, Worldspace, and ObjectReference
bool function RefHasAtLeastOnePotentialThief(ObjectReference ref, Actor[] actors) native global

; if (mode == ""): Passes all actors that are a potential thief of the ref.
; if (mode == "!"): Passes all actors that are not a potential thief of the ref.
; checks Factions/ActorBase owners on Cell, EncounterZone, Worldspace, and ObjectReference
Actor[] function filter_PotentialThievesOnRef(ObjectReference ref, Actor[] actors, string mode = "") native global