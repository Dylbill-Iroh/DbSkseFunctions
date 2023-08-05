Scriptname DbSkseEvents hidden
;/
These are global events. The eventreceiver is what receives the event.  

Filters:
You can pass in one form paramFilter to compare with the event. 
the paramFilterIndex chooses which parameter to compare. 0 is the first form param, 1 is the second ect.

example script: 
Scriptname MyQuestScript extends Quest

Weapon Property IronSword Auto
Ammo Property IronArrow Auto
Projectile Property ArrowIronProjectile Auto

Event Oninit()
	DbSkseEvents.RegisterFormForGlobalEvent("OnHitGlobal", Self, game.GetPlayer(), 0) 	 ;compare player with 'Attacker'. Registers for when the player hits anything
	DbSkseEvents.RegisterFormForGlobalEvent("OnHitGlobal", Self, game.GetPlayer(), 1) 	 ;compare player with 'Target'. Registers for when the player is hit by anything
	DbSkseEvents.RegisterFormForGlobalEvent("OnHitGlobal", Self, IronSword, 2) 		  	 ;compare IronSword with 'Source'. Register for when anything is hit with an IronSword.
	DbSkseEvents.RegisterFormForGlobalEvent("OnHitGlobal", Self, IronArrow, 3) 		  	 ;compare IronArrow with 'akAmmo'. Register for when anything is hit with an IronArrow.
	DbSkseEvents.RegisterFormForGlobalEvent("OnHitGlobal", Self, ArrowIronProjectile, 4) ;compare ArrowIronProjectile with 'akProjectile'. Register for when anything is hit with an ArrowIronProjectile. (this is unreliable, better to compare akAmmo when possible.)
	DbSkseEvents.RegisterFormForGlobalEvent("OnHitGlobal", Self) 						 ;no param comparison. Register for ALL hit events. Use this sparingly!!!
EndEvent 

Event OnHitGlobal(ObjectReference Attacker, ObjectReference Target, Form Source, Ammo akAmmo, Projectile akProjectile, bool abPowerAttack, bool abSneakAttack, bool abBashAttack, bool abHitBlocked)

	Debug.MessageBox("Attacker = " + Attacker.getDisplayName() + "\nTarget = " + Target.GetDisplayName() + "\nSource = " + Source.GetName() + "\nAmmo = " + akAmmo.getName() + "\nProjectile = " + akProjectile.getName())
	
EndEvent


Note that scripts attached to ReferenceAlias's or ActiveMagicEffects will receive the event if the reference they're filled with is registered for the event. 
Example:

Scriptname MyRefAliasScript extends ReferenceAlias 

Event OnInit()
	;these two lines achieve the same thing
    DbSkseEvents.RegisterAliasForGlobalEvent("OnLoadGameGlobal", self)
    DbSkseEvents.RegisterFormForGlobalEvent("OnLoadGameGlobal", self.GetReference())
EndEvent


Same goes for ActiveMagicEffects:
Scriptname MyRefAliasScript extends ReferenceAlias 

Event OnEffectStart(Actor akTarget, Actor akCaster)
	;these two lines achieve the same thing
    DbSkseEvents.RegisterAliasForGlobalEvent("OnLoadGameGlobal", self)
    DbSkseEvents.RegisterFormForGlobalEvent("OnLoadGameGlobal", akTarget)
EndEvent
/;

;form ==================================================================================================================================================
Bool Function IsFormRegisteredForGlobalEvent(String asEvent, Form eventReceiver, Form paramFilter = none, int paramFilterIndex = 0) global Native
function RegisterFormForGlobalEvent(String asEvent, Form eventReceiver, Form paramFilter = none, int paramFilterIndex = 0) global Native
function UnregisterFormForGlobalEvent(String asEvent, Form eventReceiver, Form paramFilter = none, int paramFilterIndex = 0) global Native
function UnregisterFormForGlobalEvent_All(String asEvent, Form eventReceiver) global Native

;returns true if registering, or false if unregistering.
bool function ToggleGlobalEventOnForm(String sEvent, form eventReceiver, form paramFilter, int paramFilterIndex) global
	if IsFormRegisteredForGlobalEvent(sEvent, eventReceiver, paramFilter, paramFilterIndex)
		UnregisterFormForGlobalEvent(sEvent, eventReceiver, paramFilter, paramFilterIndex)
		return false 
	else
		RegisterFormForGlobalEvent(sEvent, eventReceiver, paramFilter, paramFilterIndex)
		return True
	endif
EndFunction


;alias ==========================================================================================================================================================================
Bool function IsAliasRegisteredForGlobalEvent(String asEvent, Alias eventReceiver, Form paramFilter = none, int paramFilterIndex = 0) global Native
function RegisterAliasForGlobalEvent(String asEvent, Alias eventReceiver, Form paramFilter = none, int paramFilterIndex = 0) global Native
function UnregisterAliasForGlobalEvent(String asEvent, Alias eventReceiver, Form paramFilter = none, int paramFilterIndex = 0) global Native
function UnregisterAliasForGlobalEvent_All(String asEvent, Alias eventReceiver) global Native

;returns true if registering, or false if unregistering.
bool function ToggleGlobalEventOnAlias(String sEvent, Alias eventReceiver, form paramFilter, int paramFilterIndex) global
	if IsAliasregisteredforglobalevent(sEvent, eventReceiver, paramFilter, paramFilterIndex)
		UnregisterAliasforglobalevent(sEvent, eventReceiver, paramFilter, paramFilterIndex)
		return false 
	else
		RegisterAliasforglobalevent(sEvent, eventReceiver, paramFilter, paramFilterIndex)
		return True
	endif
EndFunction


;ActiveMagicEffect ===============================================================================================================================================================
Bool function IsActiveMagicEffectRegisteredForGlobalEvent(String asEvent, ActiveMagicEffect eventReceiver, Form paramFilter = none, int paramFilterIndex = 0) global Native
function RegisterActiveMagicEffectForGlobalEvent(String asEvent, ActiveMagicEffect eventReceiver, Form paramFilter = none, int paramFilterIndex = 0) global Native
function UnregisterActiveMagicEffectForGlobalEvent(String asEvent, ActiveMagicEffect eventReceiver, Form paramFilter = none, int paramFilterIndex = 0) global Native
function UnregisterActiveMagicEffectForGlobalEvent_All(String asEvent, ActiveMagicEffect eventReceiver) global Native

;returns true if registering, or false if unregistering.
bool function ToggleGlobalEventOnActiveMagicEffect(String sEvent, ActiveMagicEffect eventReceiver, form paramFilter, int paramFilterIndex) global
	if IsActiveMagicEffectRegisteredForGlobalEvent(sEvent, eventReceiver, paramFilter, paramFilterIndex)
		UnregisterActiveMagicEffectforglobalevent(sEvent, eventReceiver, paramFilter, paramFilterIndex)
		return false 
	else
		RegisterActiveMagicEffectforglobalevent(sEvent, eventReceiver, paramFilter, paramFilterIndex)
		return True
	endif
EndFunction


;events ===========================================================================================================================================================================
;put these in in a script that's on the registered eventReceiver
Event OnLoadGameGlobal()
EndEvent 

Event OnFurnitureEnterGlobal(Actor akActor, ObjectReference furnitureRef)
EndEvent 

Event OnFurnitureExitGlobal(Actor akActor, ObjectReference furnitureRef)
EndEvent 

Event OnActivateGlobal(ObjectReference ActivatorRef, ObjectReference ActivatedRef)
EndEvent

;note that OnHitGlobal sends Ammo as well as projectile. 
;This is because the projectile in this event is bugged, it doesn't detect reliably. 
;This sends the Ammo the attacker has equipped if the Source is a bow or crossbow.
Event OnHitGlobal(ObjectReference Attacker, ObjectReference Target, Form Source, Ammo akAmmo, Projectile akProjectile, bool abPowerAttack, bool abSneakAttack, bool abBashAttack, bool abHitBlocked)
EndEvent

Event OnDeathGlobal(Actor Victim, Actor Killer)
EndEvent

Event OnDyingGlobal(Actor Victim, Actor Killer)
EndEvent

Event OnObjectEquippedGlobal(Actor akActor, Form akBaseObject, ObjectReference akReference)
EndEvent

Event OnObjectUnequippedGlobal(Actor akActor, Form akBaseObject, ObjectReference akReference)
EndEvent

;note that this will work on the player for most cases. Exception being if stopCombat is called on the player with a script.
;internally this checks the player's combat status whenever another actor changes combat status 
;and sends the event if the player combat status has changed, if the player is registered for akActor.
Event OnCombatStateChangedGlobal(Actor akActor, Actor akTarget, int aeCombatState)
EndEvent



    
    
    