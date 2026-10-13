Scriptname DbSkseFunctions Hidden 
;The .dll file was compiled with CommonLib NG, so should work with Skyrim SE AE and VR

;Get the installed version of this plugin .dll
Float Function GetVersion() Global Native

;Get the current skyrim version installed. 
;[0] = Major, [1] = Minor, [2] = patch, [3] = build. E.G SE 1.5.97 returns: 
;[0] = 1, [1] = 5, [2] = 97, [3] = 0
int[] Function GetSkyrimVersion() Global Native

;return the current skyrim version as a string. E.G "1.5.97.0"
string Function GetSkyrimVersionString() Global Native

bool function LoadMostRecentSaveGame() Global Native

;get the script that called this function's name.  
;eg return "MyScript" if "MyScript" calls this function.
string function GetThisScriptName() Global Native 

;get the function or event that called this function's name
;eg return "MyFunction" if you have: 
;Function MyFunction 
;	string func = DbSkseFunctions.GetThisFunctionName() ;return "MyFunction"
;EndFunction
string function GetThisFunctionName() Global Native 

;get and set text from system clipboard, for copy / paste functionality
String Function GetClipBoardText() Global Native
Bool Function SetClipBoardText(String s) Global Native

;is the string c char whitespace? Uses c++ isspace function
Bool Function IsWhiteSpace(String c) Global Native 
int Function CountWhiteSpaces(String s) Global Native
String Function RemoveWhiteSpaces(String s) Global Native

;does the mod have at least 1 form of formType?
Bool Function ModHasFormType(String modName, int formType) Global Native

;returns new form array that contains the forms of the passed in akForms array, but sorted. 
;Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
;1 = by form name ascending, 
;2 = by form name descending, 
;3 = by form editor Id name ascending,
;4 = by form editor Id name descending,
;5 = by form Id ascending, 
;6 = by form Id descending
Form[] Function SortFormArray(Form[] akForms, int sortOption = 1) Global Native

;returns new form array that contains the forms in akList 
;Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
;1 = by form name ascending, 
;2 = by form name descending, 
;3 = by form editor Id name ascending,
;4 = by form editor Id name descending,
;5 = by form Id ascending, 
;6 = by form Id descending
Form[] Function FormListToArray(Formlist akList, int sortOption = 0) Global Native

function RemoveFormListAddedForm(formlist akList, form akForm) Global Native

;Add forms in akForms array to akList
Function AddFormsToList(Form[] akForms, Formlist akList) Global Native

;if maxCharacters is greater than 0, limits the number of characters for descriptions. 
;If a description exceeds maxCharacters, adds the overMaxCharacterSuffix to the end of the description.
;if newLineReplacer is not empty "", replaces new lines in description with newLineReplacer
;if noneStringType is 1 and a description is empty, "", gets editorID instead of the description
;if noneStringType is 2 and a description is empty, "", gets form ID instead of the description
;if akForm is none, returns nullFormString
String Function GetFormDescription(form akForm, int maxCharacters = 0, string overMaxCharacterSuffix = "...", string newLineReplacer = " ", int noneStringType = 0, string nullFormString = "Null") Global Native 

bool Function SetFormDescription(Form akForm, string desc) global native

;get form descriptions for akForms.
;if maxCharacters is greater than 0, limits the number of characters for descriptions. 
;If a description exceeds maxCharacters, adds the overMaxCharacterSuffix to the end of the description.
;if noneStringType is 1 and a description is empty, "", gets editorID instead of the description
;if noneStringType is 2 and a description is empty, "", gets form ID instead of the description
;if akForm is none, sets nullFormString for that form.
;Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
;0 = not sorted, 
;1 = sorted by description ascending, 
;2 = sorted by description descending, 
;3 = by form name ascending, 
;4 = by form name descending, 
;5 = by form editor Id name ascending,
;6 = by form editor Id name descending,
;7 = by form Id ascending, 
;8 = by form Id descending
String[] Function GetFormDescriptions(Form[] akForms, int sortOption = 0, int maxCharacters = 0, string overMaxCharacterSuffix = "...", string newLineReplacer = " ", int noneStringType = 0, string nullFormString = "Null") Global Native

;get form descriptions for forms in akFormList.
;if maxCharacters is greater than 0, limits the number of characters for descriptions. 
;If a description exceeds maxCharacters, adds the overMaxCharacterSuffix to the end of the description.
;if noneStringType is 1 and a description is empty, "", gets editorID instead of the description
;if noneStringType is 2 and a description is empty, "", gets form ID instead of the description
;if akForm is none, sets nullFormString for that form.
;Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
;0 = not sorted, 
;1 = sorted by description ascending, 
;2 = sorted by description descending, 
;3 = by form name ascending, 
;4 = by form name descending, 
;5 = by form editor Id name ascending,
;6 = by form editor Id name descending,
;7 = by form Id ascending, 
;8 = by form Id descending
String[] Function GetFormDescriptionsFromList(Formlist akFormList, int sortOption = 0, int maxCharacters = 0, string overMaxCharacterSuffix = "...", string newLineReplacer = " ", int noneStringType = 0, string nullFormString = "Null") Global Native

;get form names for akForms
;if noneStringType is 1 and a name is empty, "", gets editorID instead of name
;if noneStringType is 2 and a name is empty, "", gets form ID instead of name
;if a form is none, sets nullFormString for that form
;Sort options are as follows. 0 = not sorted, 1 = sorted by name ascending, 2 = sorted by name descending.
String[] Function GetFormNames(Form[] akForms, int sortOption = 0, int noneStringType = 0, string nullFormString = "Null") Global Native

;get form names for forms in akFormList
;if noneStringType is 1 and a name is empty, "", gets editorID instead of name
;if noneStringType is 2 and a name is empty, "", gets form ID instead of name
;if a form is none, sets nullFormString for that form
;Sort options are as follows. 0 = not sorted, 1 = sorted by name ascending, 2 = sorted by name descending.
String[] Function GetFormNamesFromList(Formlist akFormList, int sortOption = 0, int noneStringType = 0, string nullFormString = "Null") Global Native

;Get form editor Id name. 
;If akForm is none, returns nullFormString
string function GetFormEditorId(Form akForm, string nullFormString = "Null") Global Native

;get form editor id names for akForms 
;if a form is none, sets nullFormString for that form
;Sort options are as follows. 0 = not sorted, 1 = sorted by name ascending, 2 = sorted by name descending.
;Note, to get editor Ids reliably, po3 tweaks must be installed.
string[] function GetFormEditorIds(Form[] akForms, int sortOption = 0, string nullFormString = "Null") Global Native

;get form editor id names for forms in akFormList
;if a form is none, sets nullFormString for that form
;Sort options are as follows. 0 = not sorted, 1 = sorted by name ascending, 2 = sorted by name descending.
;Note, to get editor Ids reliably, po3 tweaks must be installed.
string[] function GetFormEditorIdsFromList(Formlist akFormList, int sortOption = 0, string nullFormString = "Null") Global Native

;Sort options are as follows. 0 = not sorted, 1 = sorted by name ascending, 2 = sorted by name descending.
String[] Function GetLoadedModNames(int sortOption = 0) Global Native

;Sort options are as follows. 0 = not sorted, 1 = sorted by name ascending, 2 = sorted by name descending.
String[] Function GetLoadedLightModNames(int sortOption = 0) Global Native

;Get all loaded mod names, (regular mods and light mods)
;Sort options are as follows. 0 = not sorted, 1 = sorted by name ascending, 2 = sorted by name descending.
String[] Function GetAllLoadedModNames(int sortOption = 0) Global Native

;get loaded mod descriptions.
;if maxCharacters is greater than 0, limits the number of characters for descriptions. 
;If a description exceeds maxCharacters, adds the overMaxCharacterSuffix to the end of the description.
;Sort options are as follows. 0 = not sorted, 1 = sorted by Description ascending, 2 = sorted by Description descending, 3 sorted by mod name ascending, 4 = sorted by mod name descending.
String[] Function GetLoadedModDescriptions(int sortOption = 0, int maxCharacters = 0, string overMaxCharacterSuffix = "...", string newLineReplacer = " ") Global Native
   
;get loaded light mod descriptions. 
;if maxCharacters is greater than 0, limits the number of characters for descriptions. 
;If a description exceeds maxCharacters, adds the overMaxCharacterSuffix to the end of the description.
;Sort options are as follows. 0 = not sorted, 1 = sorted by Description ascending, 2 = sorted by Description descending, 3 sorted by mod name ascending, 4 = sorted by mod name descending.
String[] Function GetLoadedLightModDescriptions(int sortOption = 0, int maxCharacters = 0, string overMaxCharacterSuffix = "...", string newLineReplacer = " ") Global Native

;get all loaded mod descriptions, (regular mods and light mods)
;if maxCharacters is greater than 0, limits the number of characters for descriptions. 
;If a description exceeds maxCharacters, adds the overMaxCharacterSuffix to the end of the description.
;Sort options are as follows. 0 = not sorted, 1 = sorted by Description ascending, 2 = sorted by Description descending, 3 sorted by mod name ascending, 4 = sorted by mod name descending.
String[] Function GetAllLoadedModDescriptions(int sortOption = 0, int maxCharacters = 0, string overMaxCharacterSuffix = "...", string newLineReplacer = " ") Global Native

;get all constructible objects that create the createdObject. 
;if none is passed in, get's all constructible objects in game.
ConstructibleObject[] Function GetAllConstructibleObjects(Form createdObject) global native

;Get all armors in game that use the slotMask
Armor[] Function GetAllArmorsForSlotMask(int slotMask) global native

WorldSpace function GetCellWorldSpace(cell akCell) global native

Location Function GetCellLocation(cell akCell) global native

Location Function GetWorldSpaceLocation(WorldSpace akWorldSpace) global native

bool Function IsCellPublic(cell akCell) global native

Function SetCellPublic(cell akCell, bool bPublic) global native

bool Function IsCellOffLimits(cell akCell) global native

Function SetCellOffLimits(cell akCell, bool bOffLimits) global native

;Get all interior cells in game that match the akLocation and or akOwner 
;if matchMode == 0, get all cells in game where either the passed in akLocation or akOwner match.
;if matchMode == 1, get all cells in game where both the passed in akLocation and akOwner match. 
;if matchMode == anything else, get all interior cells in game
cell[] Function GetAllInteriorCells(Location akLocation, Actorbase akOwner, int matchMode = 0) global native

;Get all exterior cells in game that match the akLocation and or akWorldSpace 
;if matchMode == 0, get all cells in game where either the passed in akLocation or akWorldSpace match.
;if matchMode == 1, get all cells in game where both the passed in akLocation and akWorldSpace match. 
;if matchMode == anything else, get all interior cells in game
cell[] Function GetAllExteriorCells(Location akLocation, WorldSpace akWorldSpace, int matchMode = 0) global native

;Get all cells currently attached
cell[] Function GetAttachedCells() global native

;Get forms currently favorited by the player
;formTypeMatchMode 1 = forms who match a type in formTypes.
;formTypeMatchMode 0 = forms that match none of the types in formTypes. 
;formTypeMatchMode -1 (or if formTypes == none) = formType filter is ignored completely, get all favorited forms regardless of type.
Form[] function GetFavorites(int[] formTypes = none, int formTypeMatchMode = 1) Global Native

;Get all forms who's name (with GetName()) match the sFormName.
;nameMatchMode 0 = exact match, 1 = name contains sFormName.  
;formTypeMatchMode 1 = forms who match a type in formTypes.
;formTypeMatchMode 0 = forms that match none of the types in formTypes. 
;formTypeMatchMode -1 (or if formTypes == none) = formType filter is ignored completely, get all forms regardless of type that match (or contain) sFormName.
Form[] Function GetAllFormsWithName(string sFormName, int nameMatchMode = 0, int[] formTypes = none, int formTypeMatchMode = 1) Global Native 

;formTypeMatchMode 1 = forms that have a type in formTypes.
;formTypeMatchMode 0 = forms that do not have a type in formTypes. 
;formTypeMatchMode -1 (or if formTypes == none) = formType filter is ignored completely, get all forms regardless of type that have the script with sScriptName attached
Form[] Function GetAllFormsWithScriptAttached(string sScriptName, int[] formTypes = none, int formTypeMatchMode = 0) Global Native 

Alias[] Function GetAllAliasesWithScriptAttached(string sScriptName) Global Native 

ReferenceAlias[] Function GetAllRefAliasesWithScriptAttached(string sScriptName, bool onlyQuestObjects = false, bool onlyFilled = false) Global Native 

; get the papyrus script name associated with the formType. E.G GetScriptForFormType(32) returns "MiscObject", GetScriptForFormType(62) returns "Actor" ect.
; For valid form types see FormType.psc. 
; The ones not included there for this function are these:
; 200 = Alias
; 201 = ReferenceAlias
; 202 = LocationAlias
; 203 = ActiveMagicEffect
; 204 = Form 
string Function GetScriptForFormType(int formType) Global Native
 
; get the papyrus formType associated with the baseScriptName. E.G GetScriptForFormType("MiscObject") returns 32, GetScriptForFormType("Actor") returns 62 ect.
; For valid form types see FormType.psc. 
; The ones not included there for this function are these:
; 200 = Alias
; 201 = ReferenceAlias
; 202 = LocationAlias
; 203 = ActiveMagicEffect
; 204 = Form
int Function GetFormTypeForScript(string baseScriptType) Global Native 

;Attaches the script to the akForm. Passing in an ObjectReference and attaching a script that extends ObjectReference is fine here. 
;Or attaching a script that extends something else, say armor and passing in an armor is fine. 
;Use AttachObjectReferenceScriptToBaseForm for attaching scripts that extend ObjectReference to base forms.
;If the script is already attached, this function does nothing.
bool Function AttachScriptToForm(string sScriptName, form akForm, bool allowDuplicates = false) Global Native

;Attaches the script to the akAlias.
bool Function AttachScriptToAlias(string sScriptName, Alias akAlias, bool allowDuplicates = false) Global Native

;For attaching a script that extends ObjectReference to a base form. 
;If attachToCurrentRefs is true, attaches the script to currently loaded ObjectReferences whose base form (GetBaseObject) match the akForm. 
;If attachToNewRefs is true, the plugin listens for new ObjectReferences that are created or loaded, and attaches the script if the base form (GetBaseObject) matches the akForm,
;If the script isn't already attached.
Function AttachObjectReferenceScriptToBaseForm(string sScriptName, form akForm, bool attachToCurrentRefs, bool attachToNewRefs) Global Native

;If used AttachObjectReferenceScriptToBaseForm with attachToNewRefs, stops doing that for the sScriptName and akForm.
;This does not stop ObjectReference scripts attached normally in the creation kit to a base from attaching to new refs of said base form.
bool Function StopAttachingObjectReferenceScriptsToNewRefs(string sScriptName, form akForm) Global Native


;get script names of all scripts attached to akForm. 
;includes objectReference scripts attached to base forms with AttachObjectReferenceScriptToBaseForm.
;Does not include objectReference scripts attached to base forms in the creation kit.
;If you need those, create a new ref with PlaceAtMe(baseForm) and run this on the placed ref.
string[] Function GetAllScriptsAttachedToForm(Form akForm, bool RemoveDuplicates = true) Global Native

;E.G using MyMiscScript, returns ["MyMiscScript", "MiscObject", "Form"], if MyMiscScript is attached to akForm.
;Note that swapping out sScriptName for any of the scriptnames in the hierarchy returns the same thing.
string[] Function GetScriptHierarchyOnForm(Form akForm, string sScriptName) Global Native

;returns true if the sScriptName is attached to the form and the sParentScriptName exists in the script hierarchy,
;and is higher in the hierarchy than the sScriptName. 
;E.G,  ["MyMiscScript", "MiscObject", "Form"], sScriptName = "MyMiscScript", sParentScriptName = "MiscObject" returns true. 
; switch sScriptName to "MiscObject" and sParentScriptName to "MyMiscScript", returns false.
bool Function ScriptHasParentOnForm(Form akForm, string sScriptName, string sParentScriptName) Global Native

;returns true if the sScriptName is attached to the form and the sChildScriptName exists in the script hierarchy,
;and is lower in the hierarchy than the sScriptName. 
;E.G,  ["MyMiscScript", "MiscObject", "Form"], sScriptName = "MiscObject", sChildScriptName = "MyMiscScript" returns true. 
;switch sScriptName to "MyMiscScript" and sChildScriptName to "MiscObject", returns false.
bool Function ScriptHasChildOnForm(Form akForm, string sScriptName, string sChildScriptName) Global Native

;includes objectReference scripts attached to base forms with AttachObjectReferenceScriptToBaseForm.
;Does not include objectReference scripts attached to base forms in the creation kit.
;If you need those, create a new ref with PlaceAtMe(baseForm) and run this on the placed ref.
bool function IsScriptAttachedToForm(string sScriptName, form akForm) Global Native
	
;get script names of all scripts attached to akAlias
string[] Function GetAllScriptsAttachedToAlias(Alias akAlias, bool RemoveDuplicates = true) Global Native

;E.G using MyAliasReferenceAliasScript, returns ["MyAliasReferenceAliasScript", "ReferenceAlias", "Alias"], if MyAliasReferenceAliasScript is attached to akAlias.
;Note that swapping out sScriptName for any of the scriptnames in the hierarchy returns the same thing.
string[] Function GetScriptHierarchyOnAlias(Alias akAlias, string sScriptName) Global Native

;returns true if the sScriptName is attached to the Alias and the sParentScriptName exists in the script hierarchy,
;and is higher in the hierarchy than the sScriptName. 
;E.G,  ["MyAliasReferenceAliasScript", "ReferenceAlias", "Alias"], sScriptName = "MyAliasReferenceAliasScript", sParentScriptName = "ReferenceAlias" returns true. 
;switch sScriptName to "ReferenceAlias" and sParentScriptName to "MyAliasReferenceAliasScript", returns false.
bool Function ScriptHasParentOnAlias(Alias akAlias, string sScriptName, string sParentScriptName) Global Native

;returns true if the sScriptName is attached to the Alias and the sChildScriptName exists in the script hierarchy,
;and is lower in the hierarchy than the sScriptName. 
;E.G,  ["MyAliasReferenceAliasScript", "ReferenceAlias", "Alias"], sScriptName = "ReferenceAlias", sChildScriptName = "MyAliasReferenceAliasScript" returns true. 
;switch sScriptName to "MyAliasReferenceAliasScript" and sChildScriptName to "ReferenceAlias", returns false.
bool Function ScriptHasChildOnAlias(Alias akAlias, string sScriptName, string sChildScriptName) Global Native

bool Function IsScriptAttachedToAlias(string sScriptName, Alias akAlias) Global Native

;Get all quests in game currently being tracked by the player.
Quest[] Function GetAllActiveQuests() Global Native

;Get all ReferenceAlias's in game.
;if onlyQuestObjects is true, only gets ref alias's that have the Quest Object box checked
;if onlyFilled is true, only gets ref alias's that are filled with a valid object reference.
ReferenceAlias[] function GetAllRefaliases(bool onlyQuestObjects = false, bool onlyFilled = false) global native

;Get all references aliases that are currently filled with the ref.
ReferenceAlias[] function GetAllRefAliasesForRef(ObjectReference ref) Global Native

; Get the ReferenceAlias targets for the nth objective in akQuest.
; If useObjectiveIndex is false, index is treated as the order in which objectives appear in the quest. 
; Example, if you have 3 objectives in myQuest, indexes set to 0, 50 and 100, 
; GetNthQuestObjectiveTargets(myQuest, 50, true) and GetNthQuestObjectiveTargets(myQuest, 1, false) both return the same ReferenceAlias[] array.
ReferenceAlias[] function GetNthQuestObjectiveTargets(Quest akQuest, int index, bool useObjectiveIndex = true) Global Native

;Get all quest object references in game
ObjectReference[] function GetAllQuestObjectRefs() Global Native
 
;Get all quest object references in the containerRef
ObjectReference[] function GetQuestObjectRefsInContainer(ObjectReference containerRef) Global Native

;Get all persistent object references in the containerRef, regardless if they're quest objects or not. 
;Object refs must be persistent to be in a container.
ObjectReference[] function GetAllObjectRefsInContainer(ObjectReference containerRef) Global Native

;Get the container that the ref is currently in, if any.
ObjectReference function GetRefContainer(ObjectReference ref) Global Native

;Get the current gold value for the ref, adjusted for tempering and enchantments.
;If the ref is in a container, this function will only work if the ref is persistent. 
;Note that saving a persistent ref to a script property, putting it inside a container,
;Saving your game, quiting to desktop, then loading the game and your save, this will also 
;return not accurate data. If the ref is removed from the container after and re-added to a container, 
;This function will be accurate again. 
int function GetRefGoldValue(ObjectReference ref) Global Native

;Get the current gold value for the form, adjusted for enchantments.
;You can pass in an ObjectReference here and the plugin will detect and return GetRefGoldValue if so. 
;If the ref is in a container, this function will only work if the ref is persistent.
;See the GetRefGoldValue for more details.
int function GetFormGoldValue(Form akForm) Global Native

;Calculate the sell or buy price of the akForm when bartering with the merchant. Can be off by 1 gold due to rounding errors.
;AkForm can be a base form or an objectReference.
;Note, if the akForm is an objectReference and the ref is in a container, this function will only work if the ref is persistent.
;See the GetRefGoldValue for more details.
int function CalculateBarterValue(Actor merchant, Form akForm, bool buying) Global Native

;Sets or clears the Quest Object flag for the akAlias. Returns true if successful
bool function SetAliasQuestObjectFlag(alias akAlias, bool set) Global Native

;Does the akAlias have the quest obect flag checked?
bool function IsAliasQuestObjectFlagSet(alias akAlias) Global Native

;Base Projectile functions ===============================================================================================

TextureSet Function GetProjectileBaseDecal(projectile akProjectile) Global Native
Bool Function SetProjectileBaseDecal(projectile akProjectile, TextureSet decalTextureSet) Global Native

Explosion Function GetProjectileBaseExplosion(projectile akProjectile) Global Native
Bool Function SetProjectileBaseExplosion(projectile akProjectile, Explosion akExplosion) Global Native

Float Function GetProjectileBaseCollisionRadius(projectile akProjectile) Global Native
Bool Function SetProjectileBaseCollisionRadius(projectile akProjectile, Float radius) Global Native

Float Function GetProjectileBaseCollisionConeSpread(projectile akProjectile) Global Native
Bool Function SetProjectileBaseCollisionConeSpread(projectile akProjectile, Float coneSpread) Global Native

;ObjectReference Projectile functions ===============================================================================================

;get the type of the projectileRef
;projectile types are: 1 = Missile, 2 = Grenade, 3 = Beam, 4 = Flamethrower, 5 = Cone, 6 = Barrier, 7 = Arrow, 0 = unrecognized.
int function GetProjectileRefType(ObjectReference projectileRef) Global Native

;get projectiles currently attached to the ref
;only works if the ref is an actor
Projectile[] Function GetAttachedProjectiles(ObjectReference ref) Global Native

;get projectile object references attached to the ref 
;only works if the ref is not an actor
Projectile[] Function GetAttachedProjectileRefs(ObjectReference ref) Global Native

;get all projectile object references that hit the ref that match the conditions. 
;projectileTypes are: 1 = Missile, 2 = Grenade, 3 = Beam, 4 = Flamethrower, 5 = Cone, 6 = Barrier, 7 = Arrow.
;if the projectileType param is none of those types, returns all projectiles that have hit the ref regardless of type.
ObjectReference[] function GetAllHitProjectileRefsOfType(ObjectReference ref, bool only3dLoaded = true, bool onlyEnabled = true, int projectileType = 7) Global Native

;get all projectile object references that were shot by the ref that match the conditions.
;projectileTypes are: 1 = Missile, 2 = Grenade, 3 = Beam, 4 = Flamethrower, 5 = Cone, 6 = Barrier, 7 = Arrow.
;if the projectileType param is none of those types, returns all projectiles that the ref has shot regardless of type.
ObjectReference[] function GetAllShotProjectileRefsOfType(ObjectReference ref, bool only3dLoaded = true, bool onlyEnabled = true, int projectileType = 7) Global Native

;get recent projectile object references that hit the ref that match the conditions. 
;projectileTypes are: 1 = Missile, 2 = Grenade, 3 = Beam, 4 = Flamethrower, 5 = Cone, 6 = Barrier, 7 = Arrow.
;if the projectileType param is none of those types, returns all recent projectiles that have hit the ref regardless of type.
;requires iMaxArrowsSavedPerReference to be set to greater than 0 in DbSkseFunctions.ini 
ObjectReference[] function GetRecentProjectileHitRefs(ObjectReference ref, bool only3dLoaded = true, bool onlyEnabled = true, int projectileType = 7) Global Native

;get the last projectile object reference that hit the ref that match the conditions. 
;projectileTypes are: 1 = Missile, 2 = Grenade, 3 = Beam, 4 = Flamethrower, 5 = Cone, 6 = Barrier, 7 = Arrow.
;if the projectileType param is none of those types, returns the last projectile that hit the ref regardless of type
;requires iMaxArrowsSavedPerReference to be set to greater than 0 in DbSkseFunctions.ini 
ObjectReference function GetLastProjectileHitRef(ObjectReference ref, bool only3dLoaded = true, bool onlyEnabled = true, int projectileType = 7) Global Native

;get recent projectile object references that were shot by the ref that match the conditions.
;projectileTypes are: 1 = Missile, 2 = Grenade, 3 = Beam, 4 = Flamethrower, 5 = Cone, 6 = Barrier, 7 = Arrow.
;if the projectileType param is none of those types, returns all recent projectiles that the ref has shot regardless of type.
;requires iMaxArrowsSavedPerReference to be set to greater than 0 in DbSkseFunctions.ini 
ObjectReference[] function GetRecentProjectileShotRefs(ObjectReference ref, bool only3dLoaded = true, bool onlyEnabled = true, int projectileType = 7) Global Native

;get the last projectile object reference that was shot by the ref that match the conditions.
;projectileTypes are: 1 = Missile, 2 = Grenade, 3 = Beam, 4 = Flamethrower, 5 = Cone, 6 = Barrier, 7 = Arrow.
;if the projectileType param is none of those types, returns the last projectile was shot by the ref regardless of type
;requires iMaxArrowsSavedPerReference to be set to greater than 0 in DbSkseFunctions.ini 
ObjectReference function GetLastProjectileShotRef(ObjectReference ref, bool only3dLoaded = true, bool onlyEnabled = true, int projectileType = 7) Global Native

;get the object reference(s) that the projectileRef hit (collided with). Most of the time this is 1 object, sometimes it's more.
ObjectReference[] function GetProjectileHitRefs(ObjectReference projectileRef) Global Native

;get the object reference that shot the projectileRef
ObjectReference function GetProjectileShooter(ObjectReference projectileRef) Global Native

;Get the ammo that the projectileRef was shot with if any
Ammo function GetProjectileAmmoSource(ObjectReference projectileRef) Global Native

;Get the poison the projectileRef was shot with if any
Potion function GetProjectilePoison(ObjectReference projectileRef) Global Native

;Get the enchantment the projectileRef was shot with if any
Enchantment function GetProjectileEnchantment(ObjectReference projectileRef) Global Native

;Get the magic item that shot the projectileRef if any
;Most likely a spell or shout
Form function GetProjectileMagicSource(ObjectReference projectileRef) Global Native

;Get the weapon that the projectileRef was shot from if any
;Most likely a bow or crossbow.
Weapon function GetProjectileWeaponSource(ObjectReference projectileRef) Global Native

; Get the damage of the weapon that shot the projectileRef if any
float function GetProjectileWeaponDamage(ObjectReference projectileRef) Global Native

;get the explosion for the projectileRef, if any.
Explosion function GetProjectileExplosion(ObjectReference projectileRef) Global Native

float function GetProjectilePower(ObjectReference projectileRef) Global Native

float function GetProjectileDistanceTraveled(ObjectReference projectileRef) Global Native

;impactResults are: 0 = none, 1 = destroy, 2 = bounce, 3 = impale, 4 = stick
int function GetProjectileImpactResult(ObjectReference projectileRef) Global Native

;get the node names that the projectileRef has hit. 
;these only seem to be valid if the projectileRef hit an actor. 
;i.e "NPC Head [Head]", "NPC Spine1 [Spn1]" ect.
string[] function GetProjectileNodeHitNames(ObjectReference projectileRef) Global Native
    
; get the collision layers the projectileRef has collided with
int[] function GetProjectileCollidedLayers(ObjectReference projectileRef) Global Native

; get names of the collision layers the projectileRef has collided with
string[] function GetProjectileCollidedLayerNames(ObjectReference projectileRef) Global Native

; get the string of the collision layer. i.e "Biped", "static", "trees" ect
; names are as follows: 
; Unidentified = 0,
; Static = 1,
; AnimStatic = 2,
; Transparent = 3,
; Clutter = 4,
; Weapon = 5,
; Projectile = 6,
; Spell = 7,
; Biped = 8,
; Trees = 9,
; Props = 10,
; Water = 11,
; Trigger = 12,
; Terrain = 13,
; Trap = 14,
; NonCollidable = 15,
; CloudTrap = 16,
; Ground = 17,
; Portal = 18,
; DebrisSmall = 19,
; DebrisLarge = 20,
; AcousticSpace = 21,
; ActorZone = 22,
; ProjectileZone = 23,
; GasTrap = 24,
; ShellCasting = 25,
; TransparentWall = 26,
; InvisibleWall = 27,
; TransparentSmallAnim = 28,
; ClutterLarge = 29,
; CharController = 30,
; StairHelper = 31,
; DeadBip = 32,
; BipedNoCC = 33,
; AvoidBox = 34,
; CollisionBox = 35,
; CameraSphere = 36,
; DoorDetection = 37,
; ConeProjectile = 38,
; Camera = 39,
; ItemPicker = 40,
; LOS = 41,
; PathingPick = 42,
; Unused0 = 43,
; Unused1 = 44,
; SpellExplosion = 45,
; DroppingPick = 46
string function GetCollisionLayerName(int layer) Global Native

ObjectReference function GetTestPersistentRef() Global Native

; Check if the akForm is in use. 
; Returns true if it's occupying any papyrus script property or variable. 
; Also returns true if the form is an owner of any ObjectReference, E.G it's in any ref's promotedRefOwners list.
; If the akForm is an ObjectReference, also returns true if it's in any referenceAlias's,
; or if its promotedRefOwners list contains any owners.
; if the akForm is an objectReference and this returns false, it's reasonable to assume it is not persistent.
; Note, if using this on a form that's saved to a property in the global scope, be sure to free it first or this will return true. 
; See the example MyFormIsInUse function below.
bool function IsFormInUse(Form akForm) Global Native

;/ IsFormInUse Example:

Form Property MyForm Auto

bool function MyFormIsInUse()
	;save MyForm to a temp property.
	form akMyForm = MyForm 
	
	;free myForm from the saved property in the global scope. 
	;Temp propeties that exist inside of functions or events aren't seen by IsFormInUse.
	MyForm = none 
	
	;function now returns correct result.
	bool inUse = DbSkseFunctions.IsFormInUse(MyForm) 
	
	;optionally resave MyForm to the property in the global scope.
	MyForm = akMyForm 
	
	;return result. If false, myForm is only being used by this specific script.
	return inUse
EndFunction
 /;

; Promote the ref to be held by the owner.
; This force the ref to be persistent until DemoteObjectRefPersistence is called on the owner. 
; The ref could still be persistent if it is held by other owners. Use IsObjectReferencePersistent to check.
; Returns true if the owner is in the ref's promotedRefOwners list.
bool function PromoteObjectRefPersistence(ObjectReference ref, form owner) Global Native

; Demote / free the ref from being held by the owner. 
; Allows the ref to become non persistent. 
; The ref could still be persistent if it is held by other owners. Use IsObjectReferencePersistent to check.
; Returns true if the owner is not in the ref's promotedRefOwners list.
bool function DemoteObjectRefPersistence(ObjectReference ref, form owner, bool allowActors = false) Global Native

; get all forms that own the ref. All forms in the promotedRefOwners. 
; use PromoteObjectRefPersistence or DemoteObjectRefPersistence to add or remove forms from this list.
form[] function GetObjectRefOwners(ObjectReference ref) Global Native 

; is the akForm an owner of the ref? E.G is the akForm is in the promotedRefOwners list for the ref?
bool function IsObjectRefOwner(ObjectReference ref, Form akForm) Global Native 

; get the number of forms that own the ref. Number of forms in the promotedRefOwners. 
int function CountObjectRefOwners(ObjectReference ref) Global Native 

;Get the last object reference that the player activated
;Requires the bActivateEventSinkEnabledByDefault setting in the DbSkseFunctions.ini file to be enabled.
ObjectReference function GetLastPlayerActivatedRef() Global Native

;Get the last object reference that the player activated after a menu was opened
;Requires the bMenuOpenCloseEventSinkEnabled and bActivateEventSinkEnabledByDefault settings in the DbSkseFunctions.ini file to be enabled.
ObjectReference function GetLastPlayerMenuActivatedRef() Global Native

;If the ref is an ashpile, gets the actor linked to it, if any. If the ref is an actor, gets the ashpile linked to it, if any. 
ObjectReference function GetAshPileLinkedRef(ObjectReference ref) Global Native

;Get the enable parent of the ref, if there is one.
ObjectReference function GetEnableParentRef(ObjectReference ref) Global Native

;Get the closest object reference in the refs array to the ref
ObjectReference function GetClosestObjectFromRef(ObjectReference ref, ObjectReference[] refs) Global Native

;Get the index of the closest object reference in the refs array to the ref
int function GetClosestObjectIndexFromRef(ObjectReference ref, ObjectReference[] refs) Global Native

float function GetGameHoursPassed() Global Native

;Calculate how many real time seconds it will take for gameHours to pass based on current time scale.
;Example - with default time scale (20), GameHoursToRealTimeSeconds(1) returns 180.0
float Function GameHoursToRealTimeSeconds(float gameHours) Global Native

bool Function IsGamePaused() Global Native

;returns true if a menu is open, (other than the hud menu which is always open), regardless if the game is paused or not.
bool Function IsInMenu() Global Native

bool Function IsLocalMapMenuOpen() Global Native

string Function GetLastMenuOpened() Global Native

;forces item menus to update if they are open (inventory, container, barter ect...) 
;To display any changes made to items while an item menu is open, such as changing an item's name.
Function RefreshItemMenu() Global Native

Bool Function IsItemMenuOpen() Global Native

;differs from consoleUtil.ExecuteCommand in that you can execute a targeted command on a passed in ref.
;if ref == none and command is targeted command, runs command on console selected ref like normal. 
;If no console selected ref, or is not a targeted command, executes command like normal.
Function ExecuteConsoleCommand(String command, ObjectReference ref = none) Global Native 

ObjectReference Function GetConsoleSelectedRef() Global Native 

;does the ref have collision?
bool Function HasCollision(ObjectReference ref) Global Native 

;toggle collision on ref using tcl console command
;requires powerofthree's Tweaks to work.
;https://www.nexusmods.com/skyrimspecialedition/mods/51073?tab=description
function ToggleCollision(ObjectReference ref) Global
    if ref 
        ExecuteConsoleCommand("tcl", ref) 
    endif
EndFunction 

;enable or disable collision on ref using tcl console command
;requires powerofthree's Tweaks to work.
Function SetCollision(ObjectReference ref, bool enabled) Global 
    if ref
        if enabled != HasCollision(ref)
            ExecuteConsoleCommand("tcl", ref)
        Endif 
    Endif
EndFunction

;set the max Soul size the soulGem (base form) can hold. 0 = no soul up to 5 = grand.
Function SetSoulGemSize(SoulGem akSoulGem, int level) Global Native 

;can the soulGem hold an NPC soul? I.E is it a black soul gem?
bool Function CanSoulGemHoldNPCSoul(SoulGem akSoulGem) Global Native 

;set soul gem can hold npc soul, I.E make it a black soul gem or not.
Function SetSoulGemCanHoldNPCSoul(SoulGem akSoulGem, bool canHold) Global Native 

Enchantment[] Function GetKnownEnchantments() Global Native 
Function AddKnownEnchantmentsToFormList(Formlist akList) Global Native 

String function GetWordOfPowerTranslation(WordOfPower akWord) Global Native 

;add shout to player if necessary and unlock all of its Words
function UnlockShout(shout akShout) Global Native 

;add and unlock all shouts to the player that match the param filters.
;default is adding and unlocking ALL shouts found in game to player
function AddAndUnlockAllShouts(int minNumberOfWordsWithTranslations = 0, bool onlyShoutsWithNames = false, bool onlyShoutsWithDescriptions = false) Global Native 

; furniture workbench types are:
; None = 0,
; CreateObject = 1,
; SmithingWeapon = 2,
; Enchanting = 3,
; EnchantingExperiment = 4,
; Alchemy = 5,
; AlchemyExperiment = 6,
; SmithingArmor = 7
int Function GetFurnitureWorkbenchType(furniture akFurniture) Global Native 

;return string will be an actor value skill such as "smithing", "enchanting" ect.
String Function GetFurnitureWorkbenchSkillString(furniture akFurniture) Global Native 

;/
valid skills for use with the 'string skill' params in following book functions
aggression  
confidence  
energy  
morality  
mood  
assistance  
onehanded  
twohanded  
marksman  
block  
smithing  
heavyarmor  
lightarmor  
pickpocket  
lockpicking  
sneak  
alchemy  
speechcraft  
alteration  
conjuration  
destruction  
illusion  
restoration  
enchanting  
health  
magicka  
stamina  
healrate  
magickarate  
staminarate  
speedmult  
inventoryweight  
carryweight  
criticalchance  
meleedamage  
unarmeddamage  
mass  
voicepoints  
voicerate  
damageresist  
poisonresist  
resistfire  
resistshock  
resistfrost  
resistmagic  
resistdisease  
perceptioncondition  
endurancecondition  
leftattackcondition  
rightattackcondition  
leftmobilitycondition  
rightmobilitycondition  
braincondition  
paralysis  
invisibility  
nighteye  
detectliferange  
waterbreathing  
waterwalking  
ignorecrippledlimbs  
fame  
infamy  
jumpingbonus  
wardpower  
rightitemcharge  
armorperks  
shieldperks  
warddeflection  
variable01  
variable02  
variable03  
variable04  
variable05  
variable06  
variable07  
variable08  
variable09  
variable10  
bowspeedbonus  
favoractive  
favorsperday  
favorsperdaytimer  
leftitemcharge  
absorbchance  
blindness  
weaponspeedmult  
shoutrecoverymult  
bowstaggerbonus  
telekinesis  
favorpointsbonus  
lastbribedintimidated  
lastflattered  
movementnoisemult  
bypassvendorstolencheck  
bypassvendorkeywordcheck  
waitingforplayer  
onehandedmodifier  
twohandedmodifier  
marksmanmodifier  
blockmodifier  
smithingmodifier  
heavyarmormodifier  
lightarmormodifier  
pickpocketmodifier  
lockpickingmodifier  
sneakingmodifier  
alchemymodifier  
speechcraftmodifier  
alterationmodifier  
conjurationmodifier  
destructionmodifier  
illusionmodifier  
restorationmodifier  
enchantingmodifier  
onehandedskilladvance  
twohandedskilladvance  
marksmanskilladvance  
blockskilladvance  
smithingskilladvance  
heavyarmorskilladvance  
lightarmorskilladvance  
pickpocketskilladvance  
lockpickingskilladvance  
sneakingskilladvance  
alchemyskilladvance  
speechcraftskilladvance  
alterationskilladvance  
conjurationskilladvance  
destructionskilladvance  
illusionskilladvance  
restorationskilladvance  
enchantingskilladvance  
leftweaponspeedmultiply  
dragonsouls  
combathealthregenmultiply  
onehandedpowermodifier  
twohandedpowermodifier  
marksmanpowermodifier  
blockpowermodifier  
smithingpowermodifier  
heavyarmorpowermodifier  
lightarmorpowermodifier  
pickpocketpowermodifier  
lockpickingpowermodifier  
sneakingpowermodifier  
alchemypowermodifier  
speechcraftpowermodifier  
alterationpowermodifier  
conjurationpowermodifier  
destructionpowermodifier  
illusionpowermodifier  
restorationpowermodifier  
enchantingpowermodifier  
dragonrend  
attackdamagemult  
healratemult  
magickarate  
staminarate  
werewolfperks  
vampireperks  
grabactoroffset  
grabbed  
deprecated05  
reflectdamage  
/;

string function GetBookSkill(book akBook) Global Native 

;sets the skill book teaches. If skill is "", removes TeachesSkill flag from book. (Book will no longer teach a skill.)
;not save persistent, use a load game event for maintenance
function SetBookSkill(book akBook, string skill) Global Native 

;get all books that teach the skill
book[] function GetSkillBooksForSkill(string skill) Global Native 

;add all books that teach skill to akList
function AddSkillBookForSkillToList(string skill, formlist akList) Global Native 

;Sets spell book tome teaches. If akSpell is none, removes TeachesSpell flag from book. (Book will no longer teach a spell.)
;not save persistent, use a load game event for maintenance
function SetBookSpell(book akBook, spell akSpell) Global Native 

;get the first spell tome found that teaches akSpell, or none if not found.
Book function GetSpellTomeForSpell(spell akSpell) Global Native 

;get all spell tomes that teach akSpell, or empty array if none found.
Book[] function GetSpellTomesForSpell(spell akSpell) Global Native 

;add all spell tomes that teach akSpell to akList
function AddSpellTomesForSpellToList(spell akSpell, FormList akList) Global Native 

;if read is true, set akBook  as 'read', otherwise set akBook as 'unread'
;if read is false and akBook is a skill book, the skill from the book can be increased again when reading.
function SetBookRead(book akBook, bool read) Global Native

;if read is true, set all books in game as 'read', otherwise set all books in game as 'unread'
function SetAllBooksRead(bool read) Global Native

;get source that the ActiveMagicEffect came from
;Could be a spell, enchantment, potion, or ingredient. Use GetType() to find out which.
Form Function GetActiveEffectSource(ActiveMagicEffect akEffect) Global Native

; -1 = not applicable or not found 
; 0 = conditions not met, the active effect is not affecting the reference it's on. 
; 1 = conditions met, the active effect is affecting the reference it's on
int Function GetActiveMagicEffectConditionStatus(ActiveMagicEffect akEffect) Global Native

;Get casting source that the ActiveMagicEffect came from
;LeftHand = 0,
;RightHand = 1,
;Other = 2, (most likely shout) 
;Instant = 3
int Function GetActiveEffectCastingSource(ActiveMagicEffect akEffect) Global Native

;get magic effects for akForm, assuming akForm is a magic item such as a spell, potion, shout, enchantment, scroll ect...
MagicEffect[] Function GetMagicEffectsForForm(Form akForm) Global Native

;is the form a magic item such as spell, potion, shout, enchantment, scroll ect...?
bool Function IsFormMagicItem(Form akForm) Global Native

;is the akMagicEffect currently affecting the ref?
;if magicSource is not none, only returns true if the activeMagicEffect matches the akMagicEffect and it's condition status is true and comes from the magicSource (spell, shout, potion ect)
;Otherwise, returns true if activeMagicEffect matches the akMagicEffect and it's condition status is true regardless of source.
bool Function IsMagicEffectActiveOnRef(ObjectReference ref, MagicEffect akMagicEffect, Form magicSource = none) Global Native

;if magicSource is not none, only dispels effects that come from the magicSource (spell, shout, potion ect)
;Otherwise, dispels all activeMagicEffects that match the akMagicEffect
Function DispelMagicEffectOnRef(ObjectReference ref, MagicEffect akMagicEffect, Form magicSource = none) Global Native

;would the akActor be stealing the akTarget if they took it?
Bool Function WouldActorBeStealing(actor akActor, ObjectReference akTarget) Global Native
Bool Function IsActorAttacking(actor akActor) Global Native
Bool Function IsActorPowerAttacking(actor akActor) Global Native
Bool Function IsActorSpeaking(actor akActor) Global Native
Bool Function IsActorBlocking(actor akActor) Global Native
Bool Function IsActorCasting(actor akActor) Global Native
Bool Function IsActorDualCasting(actor akActor) Global Native
Bool Function IsActorStaggered(actor akActor) Global Native
Bool Function IsActorRecoiling(actor akActor) Global Native
Bool Function IsActorIgnoringCombat(actor akActor) Global Native
Bool Function IsActorUndead(actor akActor) Global Native
Bool Function IsActorOnFlyingMount(actor akActor) Global Native
Bool Function IsActorAMount(actor akActor) Global Native
Bool Function IsActorInMidAir(actor akActor) Global Native
Bool Function IsActorInRagdollState(actor akActor) Global Native
Bool Function IsActorFleeing(actor akActor) Global Native
int Function GetDetectionLevel(actor akActor, actor akTarget) Global Native

;ward states are 0 = none, 1 = absorbing, 2 = break  
int Function GetActorWardState(actor akActor) Global Native

;Same as the IsPCSleeping condition. Returns true if the player is sleeping
bool function IsPCSleeping() Global Native

function UpdateActor3DModel(Actor akActor) global native
function UpdateActor3DPosition(Actor akActor, bool warp = false) global native

function UpdateRefLight(ObjectReference ref) global native

;[0] = x, [1] = y, [2] = z
float[] function GetRefLinearVelocity(ObjectReference ref) global native

;Get teleport marker position and rotation for the doorRef. doorRef must have teleport data.
;This is in the dooRef's linked door's cell. 
;To get the linked teleport door objectReference use papyrus extender: PO3_SkseFunctions.GetDoorDestination(doorRef)
;[0] = positionX, [1] = positionY, [2] = positionZ [3] = rotationX, [4] = rotationY, [5] = rotationZ
float[] function GetDoorTeleportMarkerPositionAndRotation(ObjectReference doorRef) global native

String Function GetKeywordString(keyword akKeyword) Global Native

;doesn't carry over between saves. Use load game event for maintenace.
function SetKeywordString(keyword akKeyword, string keywordString) Global Native

;get all keywords that match the keywordString.
;Keyword.GetKeyword only returns the first it finds. 
;This gets all keywords that match in case there are duplicates in different mods.
Keyword[] function GetKeywordsForString(string keywordString) Global Native

;get all keywords on all forms in the passed in form[] array.
Keyword[] function GetKeywordsOnArrayForms(form[] forms) Global Native

;get all keywords on all forms in the passed in formlist.
;if getNested is true, checks for nested formlists and gets all keywords for those forms as well.
Keyword[] function GetKeywordsOnListForms(formlist akList, bool getNested) Global Native

;create new forms of these types at runtime.
;carefull with these. Using these functions are like using PlaceAtMe to create permanent references. 
;Making too many of these may cause save game bloat, so use sparingly.
Formlist Function CreateFormList(formlist fillerList = none) Global Native 
ColorForm Function CreateColorForm(int color = 0xf) Global Native 
Keyword Function CreateKeyword() Global Native 
ConstructibleObject Function CreateConstructibleObject() Global Native 
TextureSet Function CreateTextureSet() Global Native 

;Create new sound. to set the sound, use Papyrus Extender, 'Po3_SkseFunctions.SetSoundDescriptor(newSoundMarker, akSoundDescriptor)'
Sound Function CreateSoundMarker() Global Native 

Quest Function CreateQuest() Global Native 

;Create new form of baseScriptType. E.G "MiscObject", "Potion", ect. It must be a vanilla base type, and cannot use "Form" because it must have a form type.
;If you need just a generic form, I recommend using "ColorForm" for baseScriptType, as those are small and not generally used by the base game or other mods.
;if customScriptName is not "", also attaches your custom script with name at the time of creation. E.G "MyMiscScript"
;Cannot create new forms of ObjectReference, Actor, Cell or WorldSpace.
Form Function CreateForm(string baseScriptType, string customScriptName = "") Global Native


;PlaySound / PlaySoundDescriptor returns instanceID like Sound.play(), but you can pass in a form, alias or activeMagicEffect to receive the OnSoundFinish Event.
;Example, if your script extends form: 
;DbSkseFunctions.PlaySound(akSound, Game.GetPlayer(), 1.0, self) ;play sound and receive the OnSoundFinish event when sound finishes playing.
;You can also set a start volume.
int Function PlaySound(Sound akSound, ObjectReference akSource, float volume = 1.0, Form eventReceiverForm = none, Alias eventReceiverAlias = none, activeMagicEffect eventReceiverActiveEffect = none) Global Native 
int Function PlaySoundDescriptor(SoundDescriptor akSoundDescriptor, ObjectReference akSource, float volume = 1.0, Form eventReceiverForm = none, Alias eventReceiverAlias = none, activeMagicEffect eventReceiverActiveEffect = none) Global Native 

;set the sound source for the currently playing soundId to the passed in ref.
;this function will only work for sounds playing from PlaySound or PlaySoundDescriptor from this script 
;I also found a strange bug. If the sound's source is the player and the player is in first person, this function will fail to set the ref as the new source. 
;If however the player is in third person, this function will succeed in setting the ref as the new source for the sound instanceID
Bool Function SetSoundInstanceSource(int instanceID, ObjectReference ref) Global Native

;sends the sound or soundDescriptor played and the instanceID
;only works for sounds played from PlaySound or PlaySoundDescriptor from this script
Event OnSoundFinish(Form SoundOrDescriptor, int instanceID)
EndEvent

SoundCategory Function GetParentSoundCategory(SoundCategory akSoundCategory) Global Native 
SoundCategory Function GetSoundCategoryForSoundDescriptor(SoundDescriptor akSoundDescriptor) Global Native 
Function SetSoundCategoryForSoundDescriptor(SoundDescriptor akSoundDescriptor, SoundCategory akSoundCategory) Global Native 
Float Function GetSoundCategoryVolume(SoundCategory akSoundCategory) Global Native 
Float Function GetSoundCategoryFrequency(SoundCategory akSoundCategory) Global Native 

;get MusicType that's currently playing
MusicType Function GetCurrentMusicType() Global Native 

int Function GetNumberOfTracksInMusicType(MusicType akMusicType) Global Native 

;get the current track index queued in akMusicType
int Function GetMusicTypeTrackIndex(MusicType akMusicType) Global Native 

;if the akMusicType is currently playing, it will jump to the track index passed in.
Function SetMusicTypeTrackIndex(MusicType akMusicType, int index) Global Native 
    
int Function GetMusicTypePriority(MusicType akMusicType) Global Native 

Function SetMusicTypePriority(MusicType akMusicType, int priority) Global Native 

;MusicTypeStatus is as follows
;kInactive = 0
;kPlaying = 1
;kPaused = 2
;kFinishing = 3
;kFinished = 4
int Function GetMusicTypeStatus(MusicType akMusicType) Global Native 

;/map marker icon types:
kNone = 0,
kCity = 1,
kTown = 2,
kSettlement = 3,
kCave = 4,
kCamp = 5,
kFort = 6,
kNordicRuins = 7,
kDwemerRuin = 8,
kShipwreck = 9,
kGrove = 10,
kLandmark = 11,
kDragonLair = 12,
kFarm = 13,
kWoodMill = 14,
kMine = 15,
kImperialCamp = 16,
kStormcloakCamp = 17,
kDoomstone = 18,
kWheatMill = 19,
kSmelter = 20,
kStable = 21,
kImperialTower = 22,
kClearing = 23,
kPass = 24,
kAlter = 25,
kRock = 26,
kLighthouse = 27,
kOrcStronghold = 28,
kGiantCamp = 29,
kShack = 30,
kNordicTower = 31,
kNordicDwelling = 32,
kDocks = 33,
kShrine = 34,
kRiftenCastle = 35,
kRiftenCapitol = 36,
kWindhelmCastle = 37,
kWindhelmCapitol = 38,
kWhiterunCastle = 39,
kWhiterunCapitol = 40,
kSolitudeCastle = 41,
kSolitudeCapitol = 42,
kMarkarthCastle = 43,
kMarkarthCapitol = 44,
kWinterholdCastle = 45,
kWinterholdCapitol = 46,
kMorthalCastle = 47,
kMorthalCapitol = 48,
kFalkreathCastle = 49,
kFalkreathCapitol = 50,
kDawnstarCastle = 51,
kDawnstarCapitol = 52,
kDLC02_TempleOfMiraak = 53,
kDLC02_RavenRock = 54,
kDLC02_BeastStone = 55,
kDLC02_TelMithryn = 56,
kDLC02_ToSkyrim = 57,
kDLC02_ToSolstheim = 58
Big Cave = 59
60 = none
Lock = 61
Flashing Arrow = 62
Flashing quest marker = 63 (don't use, it'll get stuck)
3 flashing arrows = 64
blue arrow = 65
player flashing arrow = 66
Big circle = any other value
/;

;map marker functions are not save persistent so use the Event OnLoadGameGlobal() to maintain them.
;turn the passed in ref into a map marker with name and iconType 
;in order for a newly created map marker to be discoverable, it must have the MapMarker static as a base object, so just place a new one. eg: 
;static property MapMarker auto
;objectReference mapMarkerRef = Game.GetPlayer().PlaceAtMe(MapMarker, 1, true)
;DbSkseFunctions.CreateMapMarker(MapMarkerRef, "new castle map marker", 43, true, false) ;visible but can't fast travel to yet. 
;use the CreateNewMapMarker function for convenience
;If using canTravelTo = true, any objectReference in game can be fast traveled to. 
;Keep the mapmarker objectReference persistent by storing it in script memory so it's not unloaded when traveling too far from its cell.
bool function CreateMapMarker(ObjectReference ref, string name, int iconType, bool visible = true, bool canTravelTo = false) Global Native

;Create new map marker and return the ObjectReference. These will be discoverable.
ObjectReference Function CreateNewMapMarker(ObjectReference placeAtMeRef, string name, int iconType, bool visible = true, bool canTravelTo = false, \
bool persistent = true, bool initiallyDisabled = false) Global

	if !placeAtMeRef 
		return none
	Endif 
	
    Static MapMarker = Game.GetFormFromFile(0x000010, "Skyrim.esm") as Static
	if !MapMarker 
		return none
	Endif 
	
	ObjectReference mapMarkerRef = placeAtMeRef.PlaceAtMe(MapMarker, 1, persistent, initiallyDisabled)
	DbSkseFunctions.CreateMapMarker(mapMarkerRef, name, iconType, visible, canTravelTo)
	
	return mapMarkerRef
EndFunction

;remove map marker data from the passed in ref.
bool function DestroyMapMarker(ObjectReference ref) Global Native  

;returns true if Ref has map data.
bool function IsMapMarker(ObjectReference Ref) Global Native 

;get the map marker currently highlighted in the map menu, if any.
;will return none if the "do you want to fast travel" menu is open.
ObjectReference function GetHighlightedMapMarker() Global Native  

;For these functions the MapMarker ObjectReference must be a map marker with map marker data.
;In other words, IsMapMarker must return true;
int function GetMapMarkerIconType(ObjectReference MapMarker) Global Native

bool function SetMapMarkerIconType(ObjectReference MapMarker, int iconType) Global Native

string function GetMapMarkerName(ObjectReference MapMarker) Global Native

bool function SetMapMarkerName(ObjectReference MapMarker, String Name) Global Native

;Get is the vanilla ObjectReference function mapMarker.IsMapMarkerVisible()
bool function SetMapMarkerVisible(ObjectReference MapMarker, bool visible) Global Native

;Get is the vanilla ObjectReference function mapMarker.CanFastTravelToMarker()
bool function SetCanFastTravelToMarker(ObjectReference MapMarker, bool canTravelTo) Global Native

;Is the ref currently visible on the local map? Only works if the local map menu is open.
;Use the OnLocalMapOpenedGlobal event from DbSkseEvents. 
bool function IsRefVisibleOnLocalMap(ObjectReference ref) Global Native

;Get all refs currently visible on the local map. Only works if the local map menu is open.
;Use the OnLocalMapOpenedGlobal event from DbSkseEvents. 
ObjectReference[] function GetRefsVisibleOnTheLocalMap() Global Native

;Find all load doors with teleport data near the centerRef within the radius.
ObjectReference[] Function FindLoadDoorsNearRef(ObjectReference centerRef, float radius) global native

Form function GetCellOrWorldSpaceOriginForRef(ObjectReference ref) Global Native

;This function is usefull if you have to move a map marker from one worldspace to another using MoveTo and have it display on the world map.
;This function will only be successfull if the passed in ref has been moved from its original worldspace or interior cell so...
;use moveto on the ref first before this function is used.
bool function SetCellOrWorldSpaceOriginForRef(ObjectReference ref, Form cellOrWorldSpace) Global Native

; get all map marker refs valid for the current world space or interior cell grid, (can potentially be viewed on the current map)
; for the filter params:
; -1 = filter is ignored 
;  0 = (false) only get markers that are not visible or can't be fast traveled to 
;  1 = (true)  only get markers that are visible or that can be fast traveled to
ObjectReference[] function GetCurrentMapMarkerRefs(int visibleFilter = -1, int canTravelToFilter = -1) Global Native

; get all map marker refs in game
; for the filter params:
; -1 = filter is ignored 
;  0 = (false) only get markers that are not visible or can't be fast traveled to 
;  1 = (true)  only get markers that are visible or that can be fast traveled to
ObjectReference[] function GetAllMapMarkerRefs(int visibleFilter = -1, int canTravelToFilter = -1) Global Native

;This function isn't working yet. Technically it's being set successfully internally but can't get it to display in game. 
;At least with VisualEffect.play(). I'm working on a fix for this.
function SetArtObjectNthTextureSet(art artObject, TextureSet textureSet, int n) Global Native

TextureSet function GetArtObjectNthTextureSet(art akArtObject, int n) Global Native

String function GetArtObjectModelNth3dName(art akArtObject, int n) Global Native

int function GetArtObjectNumOfTextureSets(art akArtObject) Global Native

;this only works on models that have a new texture sets applied to them. The int n is the index of the model with the override texture set.
String function GetFormWorldModelNth3dName(form akForm, int n) Global Native

;get all forms that use the akTextureSet. 
;If modName != "", only gets forms from that mod, otherwise gets all forms in game that use the textureset
form[] function GetAllFormsThatUseTextureSet(TextureSet akTextureSet, string modName = "") Global Native

;get all enable children for the ref. See also objectReference.GetEnableParentRef()
ObjectReference[] Function GetEnableChildrenRefs(ObjectReference ref) Global Native

;get all container refs, including actors, that have at least 1 of the akForm in their inventory.
ObjectReference[] Function GetAllContainerRefsThatContainForm(form akForm) Global Native

;UI functions. These are for use with skse's UI.psc script. Valid menuNames are the same as in UI.psc
;These will allow you to explore UI targets without needing adobe flash.
;Before using these functions make sure the menu is open with for example "If (UI.IsMenuOpen("InventoryMenu"))"
;Using these functions will also log the data to C:/Users/YourUserName/Documents/My Games/Skyrim Special Edition/SKSE/DbSkseFunctions.log

;Get all target members that the UI target has. 
;Example, start with "GetUiTargetMembers("InventoryMenu", "_root)"
;This will get all target members that _root has. One will be Menu_mc.
;Then you can do "GetUiTargetMembers("InventoryMenu", "_root.Menu_mc)"
;And so on and so forth. Another good starting place is for example "GetUiTargetMembers("InventoryMenu", "_global)"
string[] function GetUiTargetMembers(string menuName, string target) Global Native

;This is the same as GetUiTargetMembers except it gets the type, current value and full target path of members. 
;Example, start with "UI.GetUiTargetMembers("InventoryMenu", "_root.Menu_mc)"
;One string in the array will be "type[bool] value[true] member[_root.Menu_mc._visible]"
string[] function GetUiTargetMembersData(string menuName, string target) Global Native

;types are: 
;Undefined = 0
;Null = 1
;Boolean = 2
;Number = 3
;String = 4
;StringW = 5
;Object = 6
;Array = 7
;DisplayObject = 8
int function GetUITargetType(string menuName, string target) Global Native

string function GetUITargetTypeAsString(string menuName, string target) Global Native

;Instead of UI.GetBool, UI.GetString ect, gets the current value of the target as string. Bools will be "true" or "false".
string function GetUITargetValueAsString(string menuName, string target) Global Native

function TraceUiMenuTargetMembersData(string menu, string target, string asUserLog = "") Global
	int iType = DbSkseFunctions.GetUITargetType(menu, target)
	string sType = DbSkseFunctions.GetUITargetTypeAsString(menu, target)
	string sValue = DbSkseFunctions.GetUITargetValueAsString(menu, target)

	String[] Members = DbSkseFunctions.GetUiTargetMembersData(menu, target)
	int i = 0 
    
    string msg = ("Getting members for menu[" + menu + "] target[" + target + "] iType[" + iType + "] sType[" + sType + "] sValue[" + sValue + "]")

    if asUserLog == ""
        Debug.trace(msg)
        while i < Members.length 
            Debug.trace(Members[i])
            i += 1
        EndWhile
    Else 
        Debug.traceUser(asUserLog, msg)
        while i < Members.length 
            Debug.traceUser(asUserLog, Members[i])
            i += 1
        EndWhile
    Endif
EndFunction

;These animation Log functions log to 'info' level, so make sure [LOG] iMinLevel is 2 or less in Data/Skse/Plugins/DbSkseFunctions.ini
;Can view the log in C:/Users/YourUserName/Documents/My Games/Skyrim Special Edition/SKSE/DbSkseFunctions.log
;To view in game check out my mod Console Log Viewer: https://www.nexusmods.com/skyrimspecialedition/mods/144291

;Log the animation variables in the variables array for the ref. 
;Valid types are: 0 = bool, 1 = int, 2 = float, 3 (default) = log all types.
;If variables == none (default) logs default variables from the CK wiki page for the type, or all variables from the wiki if type is 3.
;CK wiki page: (https://www.creationkit.com/index.php?title=List_of_Animation_Variables). To see which default variables are logged see the
;DbAnimationVariableBools.txt, DbAnimationVariableInts.txt and DbAnimationVariableFloats.txt files in Data/Interface/DbMiscFunctions.
Function LogAnimationVariables(ObjectReference ref, string[] variables = none, int type = 3) Global Native

;Log all animation variables and their values for the ref
Function LogAllAnimationVariables(ObjectReference ref) Global Native

;Log all animations for the ref, to use with: 
;objectReference.PlayAnimation, objectReference.playAnimationAndWait, RegisterForAnimationEvent, OnAnimationEvent ect.
Function LogAllAnimations(ObjectReference ref) Global Native

;Log all animation attributes for the ref, to use with: 
Function LogAllAnimationsAttributes(ObjectReference ref) Global Native

;Log all animation character properties for the ref, to use with: 
Function LogAllAnimationsCharacterProperties(ObjectReference ref) Global Native

;get the top level base 3d node name for ref
string function GetBase3DNodeNameForRef(ObjectReference ref, bool firstPerson = false) Global Native

;This will also log the names to C:/Users/YourUserName/Documents/My Games/Skyrim Special Edition/SKSE/DbSkseFunctions.log
string[] function GetAll3DNodeNamesForRef(ObjectReference ref, bool firstPerson = false) Global Native

;race functions for getting and setting slot masks, similar to skse's Armor GetSlotMask, SetSlotMask ect..
;use Armor.GetMaskForSlot for convenience. Example, to add the ring slot to slot mask: 
;AddRaceSlotToMask(someRace, Armor.GetMaskForSlot(36))

int function GetRaceSlotMask(Race akRace) global native
function SetRaceSlotMask(Race akRace, int slotMask) global native
function AddRaceSlotToMask(Race akRace, int slotMask) global native
function RemoveRaceSlotFromMask(Race akRace, int slotMask) global native

;does the race / armor or armorAddon have the slot mask?
;use Armor.GetMaskForSlot for convenience. Example, find if the race has the ring slot:
;RaceSlotMaskHasPartOf(someRace, Armor.GetMaskForSlot(36))

bool function RaceSlotMaskHasPartOf(Race akRace, int slotMask) global native
bool function ArmorSlotMaskHasPartOf(Armor akArmor, int slotMask) global native
bool function ArmorAddonSlotMaskHasPartOf(Armor akArmorAddon, int slotMask) global native

Race[] function GetArmorAddonRaces(armorAddon akArmorAddon) global native
bool function ArmorAddonHasRace(armorAddon akArmorAddon, race akRace) global native
function AddAdditionalRaceToArmorAddon(armorAddon akArmorAddon, race akRace) global native
function RemoveAdditionalRaceFromArmorAddon(armorAddon akArmorAddon, race akRace) global native

; Message functions ==========================================================================================================================
; Setter message functions are not save game persistent. Use the OnLoadGameGlobal() event to maintain. 
; For the InsertMessageMenuButtonAt and RemoveMessageMenuButtonAt functions, check that the button exists first before using.
; Use ConditionList.GetNthMessageMenuButtonConditionList to get conditions for message menu buttons.

; Get the quest that owns akMessage. Returns none if it has no owner quest.
Quest Function GetMessageOwnerQuest(Message akMessage) global native

; Set the quest that owns akMessage. Returns false if akMessage or akQuest doesn't exist.
bool Function SetMessageOwnerQuest(Message akMessage, Quest akQuest) global native

; Returns true if akMessage is a message box (shows buttons) rather than a notification.
bool Function GetMessageIsMessageBox(Message akMessage) global native

; Set whether akMessage is a message box. Returns true if the flag now matches abSet.
bool Function SetMessageIsMessageBox(Message akMessage, bool abSet) global native

; Get akMessage's display time. Returns -1 if akMessage doesn't exist.
int Function GetMessageDisplayTime(Message akMessage) global native

; Get the number of menu bottons the akMessage contains.
int Function GetNumOfMessageMenuButtons(Message akMessage) global native 

; Set akMessage's display time. Returns true if it was set.
bool Function SetMessageDisplayTime(Message akMessage, int displayTime) global native

string[] Function GetMessageMenuButtons(Message akMessage) global native

Function SetMessageMenuButtons(Message akMessage, string[] menuBottons) global native

; Get the text of the menu button at nthIndex.
; Returns "" if akMessage doesn't exist or nthIndex is out of range.
string Function GetNthMessageMenuButtonText(Message akMessage, int nthIndex) global native

; Set the text of the menu button at nthIndex.
; Returns false if akMessage doesn't exist or nthIndex is out of range.
bool Function SetNthMessageMenuButtonText(Message akMessage, int nthIndex, string text) global native

; Get the index of the first menu button whose text matches text, case insensitive.
; Returns -1 if akMessage doesn't exist or no button matches.
int Function GetMessageMenuButtonIndex(Message akMessage, string text) global native

; Insert a new menu button with the given text at nthIndex.
; If nthIndex is negative or past the end, the button is appended.
bool Function InsertMessageMenuButtonAt(Message akMessage, int nthIndex, string text) global native

; Remove the menu button at nthIndex.
; Returns false if akMessage doesn't exist or nthIndex is out of range.
bool Function RemoveMessageMenuButtonAt(Message akMessage, int nthIndex) global native

; swap the menu button positions in akMessage, changing the order in which the buttons appear.
bool Function SwapMessageMenuButtons(Message akMessage, int indexA, int indexB) global native

;scene functions ==========================================================================================================================

; Scene ===================================================================================================================================

; Get all scenes in the akQuest.
Scene[] function GetAllScenesInQuest(quest akQuest) global native

; Number of valid scenes in a quest
int Function CountScenesInQuest(Quest quest) Global Native

bool Function IsSceneShuttingDown(Scene akScene) Global Native

; -1 if scene is not playing, or current phase starting with 1.
int Function GetSceneCurrentPhaseIndex(Scene akScene) Global Native

; Scene phases ===================================================================================================================================

; -1 if akScene is invalid.
int Function GetNumOfScenePhases(Scene akScene) Global Native

; None if akScene is invalid, nthIndex is out of range, or the phase has no quest node.
StoryManagerQuestNode Function GetNthScenePhaseStoryManagerQuestNode(Scene akScene, int nthIndex) Global Native

; Scene actions ===================================================================================================================================

; -1 if akScene is invalid.
; Use this for valid indexes in any GetNthSceneAction... function.
int Function GetNumOfSceneActions(Scene akScene) Global Native

; -1 = not valid
; 0 = kDialogue
; 1 = kPackage
; 2 = kTimer
int Function GetNthSceneActionType(Scene akScene, int nthIndex) Global Native

int Function GetNthSceneActionStartPhase(Scene akScene, int nthIndex) Global Native
bool Function SetNthSceneActionStartPhase(Scene akScene, int nthIndex, int phase) Global Native

int Function GetNthSceneActionEndPhase(Scene akScene, int nthIndex) Global Native
bool Function SetNthSceneActionEndPhase(Scene akScene, int nthIndex, int phase) Global Native

; the index as the action is labeled in the creation kit
int Function GetNthSceneActionIndex(Scene akScene, int nthIndex) Global Native

; the index as the action is labeled in the creation kit
bool Function SetNthSceneActionIndex(Scene akScene, int nthIndex, int index) Global Native

; flag options:
; 0 = kFaceTarget
; 1 = kLooping
; 2 = kHeadTrackPlayer
bool Function GetNthSceneActionFlagSet(Scene akScene, int nthIndex, int flag) Global Native

; flag options:
; 0 = kFaceTarget
; 1 = kLooping
; 2 = kHeadTrackPlayer
bool Function SetNthSceneActionFlag(Scene akScene, int nthIndex, int flag, bool abSet) Global Native

; Scene action dialogue ===================================================================================================================================

; nth scene action must be kDialogue type
Topic Function GetNthSceneActionTopic(Scene akScene, int nthIndex) Global Native

; nth scene action must be kDialogue type
bool Function SetNthSceneActionTopic(Scene akScene, int nthIndex, Topic akTopic) Global Native

; nth scene action must be kDialogue type
float Function GetNthSceneActionLoopingMax(Scene akScene, int nthIndex) Global Native

; nth scene action must be kDialogue type
bool Function SetNthSceneActionLoopingMax(Scene akScene, int nthIndex, float loopingMax) Global Native

; nth scene action must be kDialogue type
float Function GetNthSceneActionLoopingMin(Scene akScene, int nthIndex) Global Native

; nth scene action must be kDialogue type
bool Function SetNthSceneActionLoopingMin(Scene akScene, int nthIndex, float loopingMin) Global Native

; nth scene action must be kDialogue type
; emotion types are:
; kNeutral = 0,
; kAnger = 1,
; kDisgust = 2,
; kFear = 3,
; kSad = 4,
; kHappy = 5,
; kSurprise = 6,
; kPuzzled = 7,
int Function GetNthSceneActionEmotionType(Scene akScene, int nthIndex) Global Native

; nth scene action must be kDialogue type
; emotion types are:
; kNeutral = 0,
; kAnger = 1,
; kDisgust = 2,
; kFear = 3,
; kSad = 4,
; kHappy = 5,
; kSurprise = 6,
; kPuzzled = 7,
bool Function SetNthSceneActionEmotionType(Scene akScene, int nthIndex, int emotionType) Global Native

; nth scene action must be kDialogue type
; returns emotion intensity
int Function GetNthSceneActionEmotionValue(Scene akScene, int nthIndex) Global Native

; nth scene action must be kDialogue type
; sets emotion intensity
bool Function SetNthSceneActionEmotionValue(Scene akScene, int nthIndex, int emotionValue) Global Native

; Scene action package ===================================================================================================================================

; nth scene action must be kPackage type
Package[] Function GetNthSceneActionPackages(Scene akScene, int nthIndex) Global Native

; nth scene action must be kPackage type
int Function CountNthSceneActionPackages(Scene akScene, int nthIndex) Global Native

; Scene action timer ===================================================================================================================================

; nth scene action must be kTimer type
int Function GetNthSceneActionTimerSeconds(Scene akScene, int nthIndex) Global Native

; nth scene action must be kTimer type
bool Function SetNthSceneActionTimerSeconds(Scene akScene, int nthIndex, int timerSeconds) Global Native

; Scene actors ===================================================================================================================================

; -1 if akScene is invalid. Use for all NthSceneActor functions for valid index range.
int Function GetNumOfSceneActors(Scene akScene) Global Native

; Gets the AliasId for the referenceAlias that holds the actor at the nthIndex in the owning quest. 
; AliasId as defined in the ck, 0, 10, 16 ect... 
int Function GetNthSceneActorAliasId(Scene akScene, int nthIndex) Global Native

; Opposite of GetNthSceneActorAliasId. Get the nthIndex slot the aliasId occupies in the scene actors array. 
; Or return -1 if the aliasId is not found.
int Function GetNthIndexOfSceneActorAliasId(Scene akScene, int aliasId) Global Native

; Get the nth ref alias for akScene
ReferenceAlias Function GetNthSceneActorReferenceAlias(Scene akScene, int nthIndex) Global Native

; None if the alias isn't filled, or the filled ref isn't an actor.
Actor Function GetNthSceneActor(Scene akScene, int nthIndex) Global Native

; flag options are:
; 0 = kNoPlayerActivation
; 1 = kOptional
; 2 = kRunOnlyScenePackages
; 3 = kNoCommandState
bool Function GetNthSceneActorFlagSet(Scene akScene, int nthIndex, int flag) Global Native

; flag options are:
; 0 = kNoPlayerActivation
; 1 = kOptional
; 2 = kRunOnlyScenePackages
; 3 = kNoCommandState
bool Function SetNthSceneActorFlag(Scene akScene, int nthIndex, int flag, bool abSet) Global Native

; flag options are:
; 0 = kDeathPause
; 1 = kDeathEnd
; 2 = kCombatPause
; 3 = kCombatEnd
; 4 = kDialoguePause
; 5 = kDialogueEnd
; 6 = kOBS_COMPause
; 7 = kOBS_COMEnd
bool Function GetNthSceneActorProgressionFlagSet(Scene akScene, int nthIndex, int flag) Global Native

; flag options are:
; 0 = kDeathPause
; 1 = kDeathEnd
; 2 = kCombatPause
; 3 = kCombatEnd
; 4 = kDialoguePause
; 5 = kDialogueEnd
; 6 = kOBS_COMPause
; 7 = kOBS_COMEnd
bool Function SetNthSceneActorProgressionFlag(Scene akScene, int nthIndex, int flag, bool abSet) Global Native


;Topic / Topic info functions ======================================================================================================

; Getters ==========================================================================================================================

; Returns the number of TopicInfos in akTopic, or -1 if akTopic is invalid.
int Function GetNumOfTopicInfos(Topic akTopic) Global Native

; Returns the TopicInfos in akTopic.
; aiType filters by dialogue type (see GetTopicType). -1 returns every type.
TopicInfo[] Function GetTopicInfosInTopic(Topic akTopic, int Type = -1) Global Native

; Returns every TopicInfo in every topic owned by akQuest.
; aiType filters by dialogue type (see GetTopicType). -1 returns every type.
TopicInfo[] Function GetTopicInfosInQuest(Quest akQuest, int Type = -1) Global Native

; Returns the topics owned by akQuest.
; aiType filters by dialogue type (see GetTopicType). -1 returns every type.
Topic[] Function GetTopicsInQuest(Quest akQuest, int Type = -1) Global Native

;Topic ==============================================================================================================================

; Returns the quest that owns akTopic.
Quest Function GetTopicOwningQuest(Topic akTopic) Global Native

; Get the type for the akTopic.
; types:
; kNone = -1
; kPlayerDialogue = 0,
; kCommandDialogue = 1,
; kSceneDialogue = 2,
; kCombat = 3,
; kFavors = 4,
; kDetection = 5,
; kService = 6,
; kMiscellaneous = 7,
int Function GetTopicType(Topic akTopic) Global Native

; Returns the dialogue type of akTopic as a string, or "" if akTopic is invalid.
string Function GetTopicTypeString(Topic akTopic) Global Native

; Sets the dialogue type of akTopic. See GetTopicType for valid values (0-7).
; Returns true if the type was set.
bool Function SetTopicType(Topic akTopic, int type) Global Native

; Returns the dialogue subtype of akTopic (0-102), or -1 if akTopic is invalid.
; types:
; kCustom = 0,
; kForceGreet = 1,
; kRumors = 2,
; kUnk3 = 3,  // custom?
; kIntimidate = 4,
; kFlatter = 5,
; kBribe = 6,
; kAskGift = 7,
; kGift = 8,
; kAskFavor = 9,
; kFavor = 10,
; kShowRelationships = 11,
; kFollow = 12,
; kReject = 13,
; kScene = 14,
; kShow = 15,
; kAgree = 16,
; kRefuse = 17,
; kExitFavorState = 18,
; kMoralRefusal = 19,
; kFlyingMountLand = 20,
; kFlyingMountCancelLand = 21,
; kFlyingMountAcceptTarget = 22,
; kFlyingMountRejectTarget = 23,
; kFlyingMountNoTarget = 24,
; kFlyingMountDestinationReached
; kAttack = 26,
; kPowerAttack = 27,
; kBash = 28,
; kHit = 29,
; kFlee = 30,
; kBleedout = 31,
; kAvoidThreat = 32,
; kDeath = 33,
; kGroupStrategy = 34,
; kBlock = 35,
; kTaunt = 36,
; kAllyKilled = 37,
; kSteal = 38,
; kYield = 39,
; kAcceptYield = 40,
; kPickpocketCombat = 41,
; kAssault = 42,
; kMurder = 43,
; kAssaultNPC = 44,
; kMurderNPC = 45,
; kPickpocketNPC = 46,
; kStealFromNPC = 47,
; kTrespassAgainstNPC = 48,
; kTrespass = 49,
; kWereTransformCrime = 50,
; kVoicePowerStartShort = 51,
; kVoicePowerStartLong = 52,
; kVoicePowerEndShort = 53,
; kVoicePowerEndLong = 54,
; kAlertIdle = 55,
; kLostIdle = 56,
; kNormalToAlert = 57,
; kAlertToCombat = 58,
; kNormalToCombat = 59,
; kAlertToNormal = 60,
; kCombatToNormal = 61,
; kCombatToLost = 62,
; kLostToNormal = 63,
; kLostToCombat = 64,
; kDetectFriendDie = 65,
; kServiceRefusal = 66,
; kRepair = 67,
; kTravel = 68,
; kTraining = 69,
; kBarterExit = 70,
; kRepairExit = 71,
; kRecharge = 72,
; kRechargeExit = 73,
; kTrainingExit = 74,
; kObserveCombat = 75,
; kNoticeCorpse = 76,
; kTimeToGo = 77,
; kGoodBye = 78,
; kHello = 79,
; kSwingMeleeWeapon = 80,
; kShootBow = 81,
; kZKeyObject = 82,
; kJump = 83,
; kKnockOverObject = 84,
; kDestroyObject = 85,
; kStandonFurniture = 86,
; kLockedObject = 87,
; kPickpocketTopic = 88,
; kPursueIdleTopic = 89,
; kSharedInfo = 90,
; kPlayerCastProjectileSpell = 9
; kPlayerCastSelfSpell = 92,
; kPlayerShout = 93,
; kIdle = 94,
; kEnterSprintBreath = 95,
; kEnterBowZoomBreath = 96,
; kExitBowZoomBreath = 97,
; kActorCollidewithActor = 98,
; kPlayerinIronSights = 99,
; kOutofBreath = 100,
; kCombatGrunt = 101,
; kLeaveWaterBreath = 102
int Function GetTopicSubType(Topic akTopic) Global Native

; Returns the dialogue subtype of akTopic as a string, or "" if akTopic is invalid.
string Function GetTopicSubTypeString(Topic akTopic) Global Native

; Sets the dialogue subtype of akTopic. Valid range is 0-102. Set GetTopicSubType for valid types.
; Returns true if the subtype was set.
bool Function SetTopicSubType(Topic akTopic, int subtype) Global Native

; Returns the priority / journal index of akTopic, or -1 if akTopic is invalid.
int Function GetTopicPriorityAndJournalIndex(Topic akTopic) Global Native

; Sets the priority / journal index of akTopic.
int Function SetTopicPriorityAndJournalIndex(Topic akTopic, int index) Global Native


;Topic Info ==============================================================================================================================

; Returns the topic that akTopicInfo belongs to.
Topic Function GetTopicInfoParent(TopicInfo akTopicInfo) Global Native

; DNAM
; Returns the TopicInfo that akTopicInfo shares its response data with,
; or None if it has none.
TopicInfo Function GetTopicInfoDataInfo(TopicInfo akTopicInfo) Global Native

; Returns true if akTopicInfo has already been said once.
bool Function GetTopicInfoSaidOnce(TopicInfo akTopicInfo) Global Native

; Sets the "said once" state of akTopicInfo. Returns true if it was set.
bool Function SetTopicInfoSaidOnce(TopicInfo akTopicInfo, bool abSaidOnce) Global Native

; Returns the favor level of akTopicInfo, or -1 if akTopicInfo is invalid.
; 0 = None
; 1 = Small
; 2 = Medium
; 3 = Large
int Function GetTopicInfoFavorLevel(TopicInfo akTopicInfo) Global Native

; Sets the favor level of akTopicInfo.
; Returns true if the level was set.
; 0 = None
; 1 = Small
; 2 = Medium
; 3 = Large
bool Function SetTopicInfoFavorLevel(TopicInfo akTopicInfo, int Level) Global Native

; kStartSceneOnEnd = 0,
; kRandom = 1,
; kSayOnce = 2,
; kRequiresPlayerActivation = 3,
; kInfoRefusal = 4,
; kRandomEnd = 5,
; kEndRunningScene = 6,
; kIsForceGreet = 7,
; kPlayerAddress = 8,
; kForceSubtitle = 9,
; kCanMoveWhileGreeting = 10,
; kNoLIPFile = 11,
; kPostProcess = 12,
; kCustomSoundOutput = 13,
; kSpendsFavorPoints = 14
bool Function GetTopicInfoFlagSet(TopicInfo akTopicInfo, int flag) Global Native

; Sets or clears the flag at flag on akTopicInfo.
; flags:
; kStartSceneOnEnd = 0,
; kRandom = 1,
; kSayOnce = 2,
; kRequiresPlayerActivation = 3,
; kInfoRefusal = 4,
; kRandomEnd = 5,
; kEndRunningScene = 6,
; kIsForceGreet = 7,
; kPlayerAddress = 8,
; kForceSubtitle = 9,
; kCanMoveWhileGreeting = 10,
; kNoLIPFile = 11,
; kPostProcess = 12,
; kCustomSoundOutput = 13,
; kSpendsFavorPoints = 14
bool Function SetTopicInfoFlag(TopicInfo akTopicInfo, int Flag, bool isTrue) Global Native


;TopicInfo responses ==============================================================================================================================

; Returns the number of responses in akTopicInfo, or -1 if akTopicInfo is invalid.
int Function GetNumOfResponses(TopicInfo akTopicInfo) Global Native

; find the first index of the response that matches the text in the topicInfo, or return -1 if not found.
int Function FindResponseIndexByText(TopicInfo akTopicInfo, string text) Global Native

; find the first index of the response that matches the sound in the topicInfo, or return -1 if not found.
int Function FindResponseIndexBySound(TopicInfo akTopicInfo, SoundDescriptor akSound) Global Native

; Returns the text of the response at index, or "" if the index is invalid.
string Function GetNthResponseText(TopicInfo akTopicInfo, int Index) Global Native

; Sets the text of the response at index. Returns true if the text was set.
bool Function SetNthResponseText(TopicInfo akTopicInfo, int Index, string asText) Global Native

; Returns the sound descriptor of the response at index.
SoundDescriptor Function GetNthResponseSound(TopicInfo akTopicInfo, int Index) Global Native

; Sets the sound descriptor of the response at index. Returns true if it was set.
bool Function SetNthResponseSound(TopicInfo akTopicInfo, int Index, SoundDescriptor akSound) Global Native

; Returns the speaker idle of the response at index.
Idle Function GetNthResponseSpeakerIdle(TopicInfo akTopicInfo, int Index) Global Native

; Sets the speaker idle of the response at index. Returns true if it was set.
bool Function SetNthResponseSpeakerIdle(TopicInfo akTopicInfo, int Index, Idle akIdle) Global Native

; Returns the listener idle of the response at index.
Idle Function GetNthResponseListenerIdle(TopicInfo akTopicInfo, int Index) Global Native

; Sets the listener idle of the response at index. Returns true if it was set.
bool Function SetNthResponseListenerIdle(TopicInfo akTopicInfo, int Index, Idle akIdle) Global Native

;Returns the emotion type of the response at index, or -1 if invalid.
; 0 = Neutral
; 1 = Anger
; 2 = Disgust
; 3 = Fear
; 4 = Sad
; 5 = Happy
; 6 = Surprise
; 7 = Puzzled
int Function GetNthResponseEmotionType(TopicInfo akTopicInfo, int Index) Global Native

; Sets the emotion type of the response at index.
; Returns true if it was set.
; 0 = Neutral
; 1 = Anger
; 2 = Disgust
; 3 = Fear
; 4 = Sad
; 5 = Happy
; 6 = Surprise
; 7 = Puzzled
bool Function SetNthResponseEmotionType(TopicInfo akTopicInfo, int Index, int EmotionType) Global Native

; Returns the emotion value (0-100) of the response at index, or -1 if invalid.
int Function GetNthResponseEmotionValue(TopicInfo akTopicInfo, int Index) Global Native

; Sets the emotion value (0-100) of the response at index. Returns true if it was set.
bool Function SetNthResponseEmotionValue(TopicInfo akTopicInfo, int Index, int Value) Global Native

;===================================================================================================================================
