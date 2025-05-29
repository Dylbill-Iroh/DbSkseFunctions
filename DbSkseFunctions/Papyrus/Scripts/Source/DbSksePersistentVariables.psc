scriptname DbSksePersistentVariables extends Actor
;This script is attached to the player and is only used to save variables / properties to make them persistent to help prevent ctds. 
;Currently only saves the last player activated objectReference and the Last Player Menu Activated objectReference. 
;You can include these events in any of your own scripts if you wish, they don't need to be registered for.

ObjectReference Property lastPlayerActivatedRef Auto Hidden
ObjectReference Property LastPlayerMenuActivatedRef Auto Hidden

;When the player activates an objectReference.
;Requires the bActivateEventSinkEnabledByDefault setting in the DbSkseFunctions.ini file to be enabled.
Event OnDbSksePlayerActivatedRef(ObjectReference ref)
    lastPlayerActivatedRef = ref
    ; debug.notification("player activated ref " + ref.GetDisplayName())
EndEvent

;When the player activates an objectReference that opened a menu, e.g container, actor, crafting station ect...
;Requires the bMenuOpenCloseEventSinkEnabled and bActivateEventSinkEnabledByDefault settings in the DbSkseFunctions.ini file to be enabled.
Event OnDbSksePlayerActivatedMenuRef(ObjectReference ref)
    LastPlayerMenuActivatedRef = ref
    ; debug.notification("player activated menu ref " + ref.GetDisplayName())
EndEvent