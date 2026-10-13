Scriptname Condition extends Form 

; Global functions ========================================================================================================================
; Native global function getters get conditions as forms as it's not possible to return new papyrus types from c++ without extensive hooking. 
; There are non native functions to return conditions as condition type for convenience.

; Create a new condition form and attach this Condition script to said form. Use the Create function to return form as condition. (recommended)
; If the conditionListForm is not none, adds this condition to the list at nthIndex. If nthIndex is -1 (default), adds the new condition to the end of the list. 
; If the conditionListform is none, creates a new ConditionsList and adds this condition to it.
; int comparison options are: 
; 0 = "==" 
; 1 = "!="
; 2 = ">" 
; 3 = ">="
; 4 = "<" 
; 5 = "<="
; See below for valid conditionFunctions
Form Function CreateConditionForm(int conditionFunction, int comparison = 0, float value = 1.0, Form ConditionListForm = none, int nthIndex = -1) Global Native 

; Create a new condition.
; If the akConditionList is not none, inserts this condition to the list at nthIndex. If nthIndex is -1 (default), adds the new condition to the end of the list. 
; If the akConditionList is none, it creates a new ConditionsList and adds this condition to it.
; int comparison options are: 
; 0 = "==" 
; 1 = "!="
; 2 = ">" 
; 3 = ">=" 
; 4 = "<" 
; 5 = "<="
; See below for valid conditionFunctions
Condition Function Create(int conditionFunction, int comparison = 0, float value = 1.0, ConditionList akConditionList = none, int listIndex = -1) global
	return CreateConditionForm(conditionFunction, comparison, value, akConditionList, listIndex) as Condition
EndFunction

; Member functions ====================================================================================================================================

; Does this condition form have a valid condition? 
; Will return false if Destroy was used on this condition, or something went wrong and the linked internal condition was not found.
bool function IsValid() Native

; Run the this condition on the optional target and evaluate. Return true if the condition is met.
; This is for this condition only, not for this condition's conditionList. 
; Use GetConditionList().Evaluate(target). 
; If you know that this condition is the only condition in it's list, you should run the above as well,
; Because internally this functions clones this condition, creates a new conditionList, puts the clone in the list, then runs ->IsTrue(target)
; Then deletes the the new list and cloned condition afterwords.
bool function Evaluate(ObjectReference target = none) Native 

; Get the conditionList form that this condition is in. Use the GetConditionList function to return as ConditionList instead of Form. (recommended)
Form Function GetConditionListForm() Native

; Get the conditionList that this condition is in.
ConditionList Function GetConditionList()
	return GetConditionListForm() as ConditionList
EndFunction

; Get the index that this condition is in, in its parent ConditionList.
int function GetConditionListIndex() Native

;Copy this condition's data to another condition. conditionFunction, comparison, AndOr, params ect...
bool function Copy(Condition other) Native

; Remove this condition from its current conditionList and add it to the akConditionList at the nthIndex.
; If nthIndex == -1 (default), adds it to the end of the akConditionList.
; Returns true if successful.
; The akConditionList form must have the ConditionList script attached.
bool function MoveToList(form akConditionList, int nthIndex = -1) Native

; Destroys this condition, removing it from its ConditionList and making IsValid() return false. 
; Returns true if successfully destroyed or false if already destroyed, E.G IsValid() is already false for this condition.
bool function Destroy() Native

; Setters ===============================================================================================================================================

; Set the condition's comparison for the conditionId. 
; int comparison options are: 
; 0 = "==" 
; 1 = "!="
; 2 = ">" 
; 3 = ">=" 
; 4 = "<" 
; 5 = "<="
bool function SetComparison(int comparison) Native

; Set the condition's comparison value.
; Note, this will set uses global to false. 
bool function SetValue(float value) Native 

; Set the condition's global variable.
; Note, this will set uses global to true. 
bool function SetGlobalVariable(GlobalVariable globalVar) Native 

; See below for valid conditionFunctions
bool function SetFunction(int conditionFunction)  Native

; Set this conditions AndOr behavior for evaluating the next condition in the list. 
; if or is true, sets to or, otherwise sets to and.
bool function SetAndOr(bool or) Native

bool function SetUsesAliases(bool usesAliases) Native

bool function SetUsesPackData(bool usesPackData) Native

bool function SetSwapsSubjectAndTarget(bool swap) Native

; Sets the Run On flag set for this condition. 
; Flags are: 
; kSelf = 0, (subject)
; kTarget = 1,
; kRef = 2,
; kCombatTarget = 3,
; kLinkedRef = 4,
; kQuestAlias = 5,
; kPackData = 6,
; kEventData = 7,
; kCommandTarget = 8
bool function SetRunOnFlag(int flag) Native


bool function SetRunOnRef(ObjectReference ref) Native

; Set the nth parameter for the condition, paramIndex must be 0 or 1.
; Note that for the kGetEventData condition, index 0 = Event Function (int), 1 = Event Member (int), 2 = Event Data (form). See kGetEventData Info below for more info.
; This is for integer conditions and conditions with an enum dropdown list, such as GetIsSex, GetActorValue, etc.
; Compare creationkit created conditions with GetParameterInt to get correct values for what it expects.
; Make sure you're using this on the correct type for param. Use GetFunctionParameterBaseType to check. It will return 1 for integer types.
bool function SetParameterInt(Int param, int paramIndex = 0) Native

; Set the nth parameter for the condition to the Float param. paramIndex must be 0 or 1.
; Make sure you're setting the correct type for param. Use GetFunctionParameterBaseType to check. 2 is float type.
bool function SetParameterFloat(Float param, int paramIndex = 0) Native

; Set the nth parameter for the condition to the String param. paramIndex must be 0 or 1.
; Make sure you're setting the correct type for param. Use GetFunctionParameterBaseType to check.
bool function SetParameterString(String param, int paramIndex = 0) Native

; Set the nth parameter for the condition to the form param. paramIndex must be 0 or 1.
; The exception is kGetEventData condition where index 0 = Event Function (int), 1 = Event Member (int), 2 = Event Data (form). See kGetEventData Info below for more info.
; Make sure you're setting the correct type for param. Use GetFunctionParameterBaseType to check.
bool function SetParameterForm(form param, int paramIndex = 0) Native

; Getters ===========================================================================================================================================================

; Get all condition forms in this condition's list. The root condition will be the first [0] condition form in this array.
; Use GetConditionsInThisList to to return a condition array, (recommended.)
Form[] Function GetConditionFormsInThisList() Native

; Get all conditions in this condition's list. The root condition will be the first [0] condition in this array.
Condition[] Function GetConditionsInThisList() 
	return DbConditionArray.AsConditionArray(GetConditionFormsInThisList())
EndFunction

; Get the condition's comparison for the conditionId. 
; int comparison options are: 
; 0 = "==" 
; 1 = "!="
; 2 = ">" 
; 3 = ">=" 
; 4 = "<" 
; 5 = "<="
int function GetComparison() Native

; Get the condition's comparison value.
float function GetValue() Native 

; Get the condition's global variable, if any.
GlobalVariable function GetGlobalVariable() Native

; Get the condition function for this condition.
; See below valid conditionFunctions
int function GetFunction() Native

; Get the condition function for this condition as a string. E.G kGetWantBlocking, kGetDistance ect...
string function GetFunctionString() Native

; Returns base type for this conditions paramIndex.
; 0 = kNone
; 1 = kInt
; 2 = kFloat
; 3 = kString 
; 4 = kForm
; 5 = kObjectReference
; note that both kForm and kObjectReference are valid for GetParameterForm and SetParameterForm.
int function GetFunctionParameterBaseType(int paramIndex) Native

; Returns base type for this conditions paramIndex as string.
; base types are 
; kNone
; kInt
; kFloat
; kString
; kForm
; kObjectReference
; note that both kForm and kObjectReference are valid for GetParameterForm and SetParameterForm.
String function GetFunctionParameterBaseTypeString(int paramIndex) Native

; Get the type for this condition's function at the parameter index. paramIndex must be 0 or 1.
; See below for expected types.
int Function GetFunctionParameterType(int paramIndex = 0) Native 

; Get the type for this condition's function at the parameter index as string. E.G kChar, kInt, kActor ect...
; paramIndex must be 0 or 1.
; See below for expected types.
string Function GetFunctionParameterTypeString(int paramIndex = 0) Native 

; Get this conditions AndOr behavior for evaluating the next condition in the list. 
; if true, it's set to or. If false, it's set to and.
bool function GetAndOr() Native

bool function GetUsesAliases() Native

bool function GetUsesPackData() Native

bool function GetSwapsSubjectAndTarget() Native

; Gets the Run On flag set for this condition. 
; Flags are: 
; kSelf = 0, (subject)
; kTarget = 1,
; kRef = 2,
; kCombatTarget = 3,
; kLinkedRef = 4,
; kQuestAlias = 5,
; kPackData = 6,
; kEventData = 7,
; kCommandTarget = 8
int function GetRunOnFlag() Native

ObjectReference function GetRunOnRef() Native

; Get the nth parameter for the condition, paramIndex must be 0 or 1.
; Note that for the kGetEventData condition, index 0 = Event Function (int), 1 = Event Member (int), 2 = Event Data (form). See kGetEventData Info below for more info.
; This is for integer conditions and conditions with an enum dropdown list, such as GetIsSex, GetActorValue, etc.
; Compare creationkit created conditions with GetParameterInt to get correct values for what it expects.
; Make sure you're using this on the correct type for param. Use GetFunctionParameterBaseType to check. It will return 1 for integer types.
int function GetParameterInt(int paramIndex = 0) Native

; Get the nth parameter for this condition. ParamIndex must be 0 or 1.
; Make sure you're using this on the correct type for param. Use GetFunctionParameterBaseType to check. 2 is float type.
float function GetParameterFloat(int paramIndex = 0) Native

; Get the nth parameter for this condition. ParamIndex must be 0 or 1.
; Make sure you're using this on the correct type for param. Use GetFunctionParameterBaseType to check.
string function GetParameterString(int paramIndex = 0) Native

; Get the nth parameter for the condition. paramIndex must be 0 or 1.
; The exception is kGetEventData condition where index 0 = Event Function (int), 1 = Event Member (int), 2 = Event Data (form). See kGetEventData Info below for more info.
; Make sure you're getting the correct type for param. Use GetFunctionParameterBaseType to check.
Form function GetParameterForm(int paramIndex = 0) Native


; For GetFunctionParameterType. Known types. Other types returned will be kNone
;-----------------------------------------------------------------------------------------------------
; idx          SCRIPT_PARAM_TYPE         base type        what the value is
; -----------  ------------------------  ---------------- -------------------------------------------
; 0x00 (0)     kChar                     String
; 0x01 (1)     kInt                      Int              integer
; 0x02 (2)     kFloat                    Float
; 0x03 (3)     kInventoryObject          None
; 0x04 (4)     kObjectRef                ObjectReference  ObjectReference
; 0x05 (5)     kActorValue               Int              actor value index
; 0x06 (6)     kActor                    ObjectReference  ObjectReference / Actor
; 0x07 (7)     kSpellItem                Form             Spell
; 0x08 (8)     kAxis                     Int              X / Y / Z
; 0x09 (9)     kCell                     Form             Cell
; 0x0A (10)    kAnimGroup                None
; 0x0B (11)    kMagicItem                Form             Spell, Scroll, Potion, Enchantment, etc
; 0x0C (12)    kSound                    None
; 0x0D (13)    kTopic                    None
; 0x0E (14)    kQuest                    Form             Quest
; 0x0F (15)    kRace                     Form             Race
; 0x10 (16)    kClass                    Form             Class
; 0x11 (17)    kFaction                  Form             Faction
; 0x12 (18)    kSex                      Int
; 0x13 (19)    kGlobal                   Form             GlobalVariable
; 0x14 (20)    kFurnitureOrFormList      Form             Furniture or FormList
; 0x15 (21)    kObject                   Form
; 0x16 (22)    kScriptVar                String
; 0x17 (23)    kStage                    Int              quest stage index as set in the CK (0, 50, 100)
; 0x18 (24)    kMapMarker                None
; 0x19 (25)    kActorBase                Form             ActorBase
; 0x1A (26)    kContainerRef             None
; 0x1B (27)    kWorldOrList              Form             WorldSpace or FormList
; 0x1C (28)    kCrimeType                Int
; 0x1D (29)    kPackage                  Form             Package
; 0x1E (30)    kCombatStyle              None
; 0x1F (31)    kMagicEffect              Form             MagicEffect
; 0x20 (32)    kFormType                 Int
; 0x21 (33)    kWeather                  Form             Weather
; 0x22 (34)    kNPC                      None             not verified -- no condition function found
; 0x23 (35)    kOwner                    Form             ActorBase
; 0x24 (36)    kShaderEffect             None
; 0x25 (37)    kFormList                 Form             FormList
; 0x26 (38)    kMenuIcon                 None             not verified -- no condition function found
; 0x27 (39)    kPerk                     Form             Perk
; 0x28 (40)    kNote                     Int              index
; 0x29 (41)    kMiscStat                 Int
; 0x2A (42)    kImagespaceMod            None             not verified -- unused
; 0x2B (43)    kImagespace               None             not verified -- unused
; 0x2C (44)    kVATSValue                None             not verified -- unused
; 0x2D (45)    kVATSValueData            None             not verified -- unused
; 0x2E (46)    kEventFunction            Int              event function + event member, packed -- see below
; 0x2F (47)    kEventFunctionMember      Form             event data form (optional) -- see below
; 0x30 (48)    kEventFunctionData        None             not verified -- no condition function found
; 0x31 (49)    kVoiceType                Form             VoiceType or FormList
; 0x32 (50)    kEncounterZone            Form             EncounterZone
; 0x33 (51)    kIdleForm                 Form             Idle
; 0x34 (52)    kMessage                  None
; 0x35 (53)    kInvObjectOrFormList      Form             inventory object form or FormList
; 0x36 (54)    kAlignment                Int
; 0x37 (55)    kEquipType                Form             EquipSlot
; 0x38 (56)    kObjectOrFormList         Form             object form or FormList
; 0x39 (57)    kMusic                    None
; 0x3A (58)    kCritStage                Int
; 0x3B (59)    kKeyword                  Form             keyword
; 0x3C (60)    kRefType                  Form             Location
; 0x3D (61)    kLocation                 Form             Location
; 0x3E (62)    kForm                     None
; 0x3F (63)    kAlias                    Int              alias INDEX, not an Alias object
; 0x40 (64)    kShout                    Form             Shout
; 0x41 (65)    kWordOfPower              None
; 0x42 (66)    kRelationshipRank         None             not verified -- no condition function found
; 0x43 (67)    kBGSScene                 Form             Scene
; 0x44 (68)    kCastingSource            Int
; 0x45 (69)    kAssociationType          Form             AssociationType
; 0x46 (70)    kWardState                Int
; 0x47 (71)    kPackageDataCanBeNull     Int              package ID
; 0x48 (72)    kPackageDataNumeric       Int              integer
; 0x49 (73)    kFurnitureAnimType        Int
; 0x4A (74)    kFurnitureEntryType       Int
; 0x4C (76)    kVMScriptVar              String
; 0x4D (77)    kReferenceEffect          None
; 0x4E (78)    kPackageData              Int              package ID
; 0x50 (80)    kSkillAction              Int
; 0x51 (81)    kKnowableForm             Form
; 0x52 (82)    kRegion                   Form             Region
;
; Indices 75 and 79, and anything above 82, have no entry and report kNone.
; Every row marked "not verified" means no condition function was found that uses that parameter type.
; They report kNone, so the accessors will reject them. 


; kGetEventData Info 
;==============================================================================================================================
; kGetEventData is the one condition with 3 parameters in the Creation Kit.
; The Creation Kit shows kGetEventData with three fields. The engine packs the first two into a
; single parameter, so this condition is addressed with three paramIndex values instead of two:
;
;   paramIndex 0   Event Function   SetParameterInt()  / GetParameterInt()
;   paramIndex 1   Event Member     SetParameterInt()  / GetParameterInt()
;   paramIndex 2   Data (optional)  SetParameterForm() / GetParameterForm()
;
; Event Function values, in the order the Creation Kit lists them:
;
;   0  GetIsID
;   1  GetItemValue
;   2  GetValue
;   3  HasKeyword
;   4  IsInList
;
; Event Member values, in the order the Creation Kit lists them:
;
;   0  (New)Location
;   1  (Old)Location
;   2  CreatedObject
;   3  Form
;   4  Keyword
;   5  None
;   6  Value1
;   7  Value2
;
; Which members a given kGetEventData actually provides depends on the event, so a member that is
; valid syntactically can still never match at runtime. 
; Check the event's definition in the Creation Kit or XEdit to verify.
;
; Example -- HasKeyword on the event's (Old)Location, testing for ActorTypeNPC:
;
;   myCondition.SetParameterInt(3, 0)                      ; HasKeyword
;   myCondition.SetParameterInt(1, 1)                      ; (Old)Location
;   myCondition.SetParameterForm(ActorTypeNPCKeyword, 2)   ; the keyword to test for
;
; A freshly created GetEventData condition reads back as Event Function 0 (GetIsID) and Event
; Member 5 (None), because both halves are zero. That is the default state, not an error.


; condition Functions ==========================================================================================================================================
;/ valid functions for the int conditionFunction parameter
kGetWantBlocking = 0,
kGetDistance = 1,
kAddItem = 2,
kSetEssential = 3,
kRotate = 4,
kGetLocked = 5,
kGetPos = 6,
kSetPos = 7,
kGetAngle = 8,
kSetAngle = 9,
kGetStartingPos = 10,
kGetStartingAngle = 11,
kGetSecondsPassed = 12,
kActivate = 13,
kGetActorValue = 14,
kSetActorValue = 15,
kModActorValue = 16,
kSetAtStart = 17,
kGetCurrentTime = 18,
kPlayGroup = 19,
kLoopGroup = 20,
kSkipAnim = 21,
kStartCombat = 22,
kStopCombat = 23,
kGetScale = 24,
kIsMoving = 25,
kIsTurning = 26,
kGetLineOfSight = 27,
kAddSpell = 28,
kRemoveSpell = 29,
kCast = 30,
kGetButtonPressed = 31,
kGetInSameCell = 32,
kEnable = 33,
kDisable = 34,
kGetDisabled = 35,
kMenuMode = 36,
kPlaceAtMe = 37,
kPlaySound = 38,
kGetDisease = 39,
kFailAllObjectives = 40,
kGetClothingValue = 41,
kSameFaction = 42,
kSameRace = 43,
kSameSex = 44,
kGetDetected = 45,
kGetDead = 46,
kGetItemCount = 47,
kGetGold = 48,
kGetSleeping = 49,
kGetTalkedToPC = 50,
kSay = 51,
kSayTo = 52,
kGetScriptVariable = 53,
kStartQuest = 54,
kStopQuest = 55,
kGetQuestRunning = 56,
kSetStage = 57,
kGetStage = 58,
kGetStageDone = 59,
kGetFactionRankDifference = 60,
kGetAlarmed = 61,
kIsRaining = 62,
kGetAttacked = 63,
kGetIsCreature = 64,
kGetLockLevel = 65,
kGetShouldAttack = 66,
kGetInCell = 67,
kGetIsClass = 68,
kGetIsRace = 69,
kGetIsSex = 70,
kGetInFaction = 71,
kGetIsID = 72,
kGetFactionRank = 73,
kGetGlobalValue = 74,
kIsSnowing = 75,
kFastTravel = 76,
kGetRandomPercent = 77,
kRemoveMusic = 78,
kGetQuestVariable = 79,
kGetLevel = 80,
kIsRotating = 81,
kRemoveItem = 82,
kGetLeveledEncounterValue = 83,
kGetDeadCount = 84,
kAddToMap = 85,
kStartConversation = 86,
kDrop = 87,
kAddTopic = 88,
kShowMessage = 89,
kSetAlert = 90,
kGetIsAlerted = 91,
kLook = 92,
kStopLook = 93,
kEvaluatePackage = 94,
kSendAssaultAlarm = 95,
kEnablePlayerControls = 96,
kDisablePlayerControls = 97,
kGetPlayerControlsDisabled = 98,
kGetHeadingAngle = 99,
kPickIdle = 100,
kIsWeaponMagicOut = 101,
kIsTorchOut = 102,
kIsShieldOut = 103,
kCreateDetectionEvent = 104,
kIsActionRef = 105,
kIsFacingUp = 106,
kGetKnockedState = 107,
kGetWeaponAnimType = 108,
kIsWeaponSkillType = 109,
kGetCurrentAIPackage = 110,
kIsWaiting = 111,
kIsIdlePlaying = 112,
kCompleteQuest = 113,
kLock = 114,
kUnLock = 115,
kIsIntimidatedByPlayer = 116,
kIsPlayerInRegion = 117,
kGetActorAggroRadiusViolated = 118,
kGetCrimeKnown = 119,
kSetEnemy = 120,
kSetAlly = 121,
kGetCrime = 122,
kIsGreetingPlayer = 123,
kStartMisterSandMan = 124,
kIsGuard = 125,
kStartCannibal = 126,
kHasBeenEaten = 127,
kGetStaminaPercentage = 128,
kGetPCIsClass = 129,
kGetPCIsRace = 130,
kGetPCIsSex = 131,
kGetPCInFaction = 132,
kSameFactionAsPC = 133,
kSameRaceAsPC = 134,
kSameSexAsPC = 135,
kGetIsReference = 136,
kSetFactionRank = 137,
kModFactionRank = 138,
kKillActor = 139,
kResurrectActor = 140,
kIsTalking = 141,
kGetWalkSpeed = 142,
kGetCurrentAIProcedure = 143,
kGetTrespassWarningLevel = 144,
kIsTrespassing = 145,
kIsInMyOwnedCell = 146,
kGetWindSpeed = 147,
kGetCurrentWeatherPercent = 148,
kGetIsCurrentWeather = 149,
kIsContinuingPackagePCNear = 150,
kSetCrimeFaction = 151,
kGetIsCrimeFaction = 152,
kCanHaveFlames = 153,
kHasFlames = 154,
kAddFlames = 155,
kRemoveFlames = 156,
kGetOpenState = 157,
kMoveToMarker = 158,
kGetSitting = 159,
kGetFurnitureMarkerID = 160,
kGetIsCurrentPackage = 161,
kIsCurrentFurnitureRef = 162,
kIsCurrentFurnitureObj = 163,
kSetSize = 164,
kRemoveMe = 165,
kDropMe = 166,
kGetFactionReaction = 167,
kSetFactionReaction = 168,
kModFactionReaction = 169,
kGetDayOfWeek = 170,
kIgnoreCrime = 171,
kGetTalkedToPCParam = 172,
kRemoveAllItems = 173,
kWakeUpPC = 174,
kIsPCSleeping = 175,
kIsPCAMurderer = 176,
kSetCombatStyle = 177,
kPlaySound3D = 178,
kSelectPlayerSpell = 179,
kHasSameEditorLocAsRef = 180,
kHasSameEditorLocAsRefAlias = 181,
kGetEquipped = 182,
kWait = 183,
kStopWaiting = 184,
kIsSwimming = 185,
kScriptEffectElapsedSeconds = 186,
kSetCellPublicFlag = 187,
kGetPCSleepHours = 188,
kSetPCSleepHours = 189,
kGetAmountSoldStolen = 190,
kModAmountSoldStolen = 191,
kGetIgnoreCrime = 192,
kGetPCExpelled = 193,
kSetPCExpelled = 194,
kGetPCFactionMurder = 195,
kSetPCFactionMurder = 196,
kGetPCEnemyofFaction = 197,
kSetPCEnemyofFaction = 198,
kGetPCFactionAttack = 199,
kSetPCFactionAttack = 200,
kStartScene = 201,
kStopScene = 202,
kGetDestroyed = 203,
kSetDestroyed = 204,
kGetActionRef = 205,
kGetSelf = 206,
kGetContainer = 207,
kGetForceRun = 208,
kSetForceRun = 209,
kGetForceSneak = 210,
kSetForceSneak = 211,
kAdvancePCSkill = 212,
kAdvancePCLevel = 213,
kHasMagicEffect = 214,
kGetDefaultOpen = 215,
kSetDefaultOpen = 216,
kShowClassMenu = 217,
kShowRaceMenu = 218,
kGetAnimAction = 219,
kShowNameMenu = 220,
kSetOpenState = 221,
kResetReference = 222,
kIsSpellTarget = 223,
kGetVATSMode = 224,
kGetPersuasionNumber = 225,
kGetVampireFeed = 226,
kGetCannibal = 227,
kGetIsClassDefault = 228,
kGetClassDefaultMatch = 229,
kGetInCellParam = 230,
kUnusedFunction1 = 231,
kGetCombatTarget = 232,
kGetPackageTarget = 233,
kShowSpellMaking = 234,
kGetVatsTargetHeight = 235,
kSetGhost = 236,
kGetIsGhost = 237,
kEquipItem = 238,
kUnequipItem = 239,
kSetClass = 240,
kSetUnconscious = 241,
kGetUnconscious = 242,
kSetRestrained = 243,
kGetRestrained = 244,
kForceFlee = 245,
kGetIsUsedItem = 246,
kGetIsUsedItemType = 247,
kIsScenePlaying = 248,
kIsInDialogueWithPlayer = 249,
kGetLocationCleared = 250,
kSetLocationCleared = 251,
kForceRefIntoAlias = 252,
kEmptyRefAlias = 253,
kGetIsPlayableRace = 254,
kGetOffersServicesNow = 255,
kGetGameSetting = 256,
kStopCombatAlarmOnActor = 257,
kHasAssociationType = 258,
kHasFamilyRelationship = 259,
kSetWeather = 260,
kHasParentRelationship = 261,
kIsWarningAbout = 262,
kIsWeaponOut = 263,
kHasSpell = 264,
kIsTimePassing = 265,
kIsPleasant = 266,
kIsCloudy = 267,
kTrapUpdate = 268,
kShowQuestObjectives = 269,
kForceActorValue = 270,
kIncrementPCSkill = 271,
kDoTrap = 272,
kEnableFastTravel = 273,
kIsSmallBump = 274,
kGetParentRef = 275,
kPlayBink = 276,
kGetBaseActorValue = 277,
kIsOwner = 278,
kSetOwnership = 279,
kIsCellOwner = 280,
kSetCellOwnership = 281,
kIsHorseStolen = 282,
kSetCellFullName = 283,
kSetActorFullName = 284,
kIsLeftUp = 285,
kIsSneaking = 286,
kIsRunning = 287,
kGetFriendHit = 288,
kIsInCombat = 289,
kSetPackDuration = 290,
kPlayMagicShaderVisuals = 291,
kPlayMagicEffectVisuals = 292,
kStopMagicShaderVisuals = 293,
kStopMagicEffectVisuals = 294,
kResetInterior = 295,
kIsAnimPlaying = 296,
kSetActorAlpha = 297,
kEnableLinkedPathPoints = 298,
kDisableLinkedPathPoints = 299,
kIsInInterior = 300,
kForceWeather = 301,
kToggleActorsAI = 302,
kIsActorsAIOff = 303,
kIsWaterObject = 304,
kGetPlayerAction = 305,
kIsActorUsingATorch = 306,
kSetLevel = 307,
kResetFallDamageTimer = 308,
kIsXBox = 309,
kGetInWorldspace = 310,
kModPCMiscStat = 311,
kGetPCMiscStat = 312,
kGetPairedAnimation = 313,
kIsActorAVictim = 314,
kGetTotalPersuasionNumber = 315,
kSetScale = 316,
kModScale = 317,
kGetIdleDoneOnce = 318,
kKillAllActors = 319,
kGetNoRumors = 320,
kSetNoRumors = 321,
kDispel = 322,
kGetCombatState = 323,
kTriggerHitShader = 324,
kGetWithinPackageLocation = 325,
kReset3DState = 326,
kIsRidingMount = 327,
kDispelAllSpells = 328,
kIsFleeing = 329,
kAddAchievement = 330,
kDuplicateAllItems = 331,
kIsInDangerousWater = 332,
kEssentialDeathReload = 333,
kSetShowQuestItems = 334,
kDuplicateNPCStats = 335,
kResetHealth = 336,
kSetIgnoreFriendlyHits = 337,
kGetIgnoreFriendlyHits = 338,
kIsPlayersLastRiddenMount = 339,
kSetActorRefraction = 340,
kSetItemValue = 341,
kSetRigidBodyMass = 342,
kShowViewerStrings = 343,
kReleaseWeatherOverride = 344,
kSetAllReachable = 345,
kSetAllVisible = 346,
kSetNoAvoidance = 347,
kSendTrespassAlarm = 348,
kSetSceneIsComplex = 349,
kAutosave = 350,
kStartMasterFileSeekData = 351,
kDumpMasterFileSeekData = 352,
kIsActor = 353,
kIsEssential = 354,
kPreloadMagicEffect = 355,
kShowDialogSubtitles = 356,
kSetPlayerResistingArrest = 357,
kIsPlayerMovingIntoNewSpace = 358,
kGetInCurrentLoc = 359,
kGetInCurrentLocAlias = 360,
kGetTimeDead = 361,
kHasLinkedRef = 362,
kGetLinkedRef = 363,
kDamageObject = 364,
kIsChild = 365,
kGetStolenItemValueNoCrime = 366,
kGetLastPlayerAction = 367,
kIsPlayerActionActive = 368,
kSetTalkingActivatorActor = 369,
kIsTalkingActivatorActor = 370,
kShowBarterMenu = 371,
kIsInList = 372,
kGetStolenItemValue = 373,
kAddPerk = 374,
kGetCrimeGoldViolent = 375,
kGetCrimeGoldNonviolent = 376,
kShowRepairMenu = 377,
kHasShout = 378,
kAddNote = 379,
kRemoveNote = 380,
kGetHasNote = 381,
kAddToFaction = 382,
kRemoveFromFaction = 383,
kDamageActorValue = 384,
kRestoreActorValue = 385,
kTriggerHUDShudder = 386,
kGetObjectiveFailed = 387,
kSetObjectiveFailed = 388,
kSetGlobalTimeMultiplier = 389,
kGetHitLocation = 390,
kIsPC1stPerson = 391,
kPurgeCellBuffers = 392,
kPushActorAway = 393,
kSetActorsAI = 394,
kClearOwnership = 395,
kGetCauseofDeath = 396,
kIsLimbGone = 397,
kIsWeaponInList = 398,
kPlayIdle = 399,
kApplyImageSpaceModifier = 400,
kRemoveImageSpaceModifier = 401,
kIsBribedbyPlayer = 402,
kGetRelationshipRank = 403,
kSetRelationshipRank = 404,
kSetCellImageSpace = 405,
kShowChargenMenu = 406,
kGetVATSValue = 407,
kIsKiller = 408,
kIsKillerObject = 409,
kGetFactionCombatReaction = 410,
kUseWeapon = 411,
kEvaluateSpellConditions = 412,
kToggleMotionBlur = 413,
kExists = 414,
kGetGroupMemberCount = 415,
kGetGroupTargetCount = 416,
kSetObjectiveCompleted = 417,
kSetObjectiveDisplayed = 418,
kGetObjectiveCompleted = 419,
kGetObjectiveDisplayed = 420,
kSetImageSpace = 421,
kPipboyRadio = 422,
kRemovePerk = 423,
kDisableAllActors = 424,
kGetIsFormType = 425,
kGetIsVoiceType = 426,
kGetPlantedExplosive = 427,
kCompleteAllObjectives = 428,
kIsScenePackageRunning = 429,
kGetHealthPercentage = 430,
kSetAudioMultithreading = 431,
kGetIsObjectType = 432,
kShowChargenMenuParams = 433,
kGetDialogueEmotion = 434,
kGetDialogueEmotionValue = 435,
kExitGame = 436,
kGetIsCreatureType = 437,
kPlayerCreatePotion = 438,
kPlayerEnchantObject = 439,
kShowWarning = 440,
kEnterTrigger = 441,
kMarkForDelete = 442,
kSetPlayerAIDriven = 443,
kGetInCurrentLocFormList = 444,
kGetInZone = 445,
kGetVelocity = 446,
kGetGraphVariableFloat = 447,
kHasPerk = 448,
kGetFactionRelation = 449,
kIsLastIdlePlayed = 450,
kSetNPCRadio = 451,
kSetPlayerTeammate = 452,
kGetPlayerTeammate = 453,
kGetPlayerTeammateCount = 454,
kOpenActorContainer = 455,
kClearFactionPlayerEnemyFlag = 456,
kClearActorsFactionsPlayerEnemyFlag = 457,
kGetActorCrimePlayerEnemy = 458,
kGetCrimeGold = 459,
kSetCrimeGold = 460,
kModCrimeGold = 461,
kGetPlayerGrabbedRef = 462,
kIsPlayerGrabbedRef = 463,
kPlaceLeveledActorAtMe = 464,
kGetKeywordItemCount = 465,
kShowLockpickMenu = 466,
kGetBroadcastState = 467,
kSetBroadcastState = 468,
kStartRadioConversation = 469,
kGetDestructionStage = 470,
kClearDestruction = 471,
kCastImmediateOnSelf = 472,
kGetIsAlignment = 473,
kResetQuest = 474,
kSetQuestDelay = 475,
kIsProtected = 476,
kGetThreatRatio = 477,
kMatchFaceGeometry = 478,
kGetIsUsedItemEquipType = 479,
kGetPlayerName = 480,
kFireWeapon = 481,
kPayCrimeGold = 482,
kUnusedFunction2 = 483,
kMatchRace = 484,
kSetPCYoung = 485,
kSexChange = 486,
kIsCarryable = 487,
kGetConcussed = 488,
kSetZoneRespawns = 489,
kSetVATSTarget = 490,
kGetMapMarkerVisible = 491,
kResetInventory = 492,
kPlayerKnows = 493,
kGetPermanentActorValue = 494,
kGetKillingBlowLimb = 495,
kGoToJail = 496,
kCanPayCrimeGold = 497,
kServeTime = 498,
kGetDaysInJail = 499,
kEPAlchemyGetMakingPoison = 500,
kEPAlchemyEffectHasKeyword = 501,
kShowAllMapMarkers = 502,
kGetAllowWorldInteractions = 503,
kResetAI = 504,
kSetRumble = 505,
kSetNoActivationSound = 506,
kClearNoActivationSound = 507,
kGetLastHitCritical = 508,
kAddMusic = 509,
kUnusedFunction3 = 510,
kUnusedFunction4 = 511,
kSetPCToddler = 512,
kIsCombatTarget = 513,
kTriggerScreenBlood = 514,
kGetVATSRightAreaFree = 515,
kGetVATSLeftAreaFree = 516,
kGetVATSBackAreaFree = 517,
kGetVATSFrontAreaFree = 518,
kGetIsLockBroken = 519,
kIsPS3 = 520,
kIsWin32 = 521,
kGetVATSRightTargetVisible = 522,
kGetVATSLeftTargetVisible = 523,
kGetVATSBackTargetVisible = 524,
kGetVATSFrontTargetVisible = 525,
kAttachAshPile = 526,
kSetCriticalStage = 527,
kIsInCriticalStage = 528,
kRemoveFromAllFactions = 529,
kGetXPForNextLevel = 530,
kShowLockpickMenuDebug = 531,
kForceSave = 532,
kGetInfamy = 533,
kGetInfamyViolent = 534,
kGetInfamyNonViolent = 535,
kUnusedFunction5 = 536,
kSin = 537,
kCos = 538,
kTan = 539,
kSqrt = 540,
kLog = 541,
kAbs = 542,
kGetQuestCompleted = 543,
kUnusedFunction6 = 544,
kPipBoyRadioOff = 545,
kAutoDisplayObjectives = 546,
kIsGoreDisabled = 547,
kFadeSFX = 548,
kSetMinimalUse = 549,
kIsSceneActionComplete = 550,
kShowQuestStages = 551,
kGetSpellUsageNum = 552,
kForceRadioStationUpdate = 553,
kGetActorsInHigh = 554,
kHasLoaded3D = 555,
kDisableAllMines = 556,
kSetLastExtDoorActivated = 557,
kKillQuestUpdates = 558,
kIsImageSpaceActive = 559,
kHasKeyword = 560,
kHasRefType = 561,
kLocationHasKeyword = 562,
kLocationHasRefType = 563,
kCreateEvent = 564,
kGetIsEditorLocation = 565,
kGetIsAliasRef = 566,
kGetIsEditorLocAlias = 567,
kIsSprinting = 568,
kIsBlocking = 569,
kHasEquippedSpell = 570,
kGetCurrentCastingType = 571,
kGetCurrentDeliveryType = 572,
kEquipSpell = 573,
kGetAttackState = 574,
kGetAliasedRef = 575,
kGetEventData = 576,
kIsCloserToAThanB = 577,
kEquipShout = 578,
kGetEquippedShout = 579,
kIsBleedingOut = 580,
kUnlockWord = 581,
kTeachWord = 582,
kAddToContainer = 583,
kGetRelativeAngle = 584,
kSendAnimEvent = 585,
kShout = 586,
kAddShout = 587,
kRemoveShout = 588,
kGetMovementDirection = 589,
kIsInScene = 590,
kGetRefTypeDeadCount = 591,
kGetRefTypeAliveCount = 592,
kApplyHavokImpulse = 593,
kGetIsFlying = 594,
kIsCurrentSpell = 595,
kSpellHasKeyword = 596,
kGetEquippedItemType = 597,
kGetLocationAliasCleared = 598,
kSetLocationAliasCleared = 599,
kGetLocAliasRefTypeDeadCount = 600,
kGetLocAliasRefTypeAliveCount = 601,
kIsWardState = 602,
kIsInSameCurrentLocAsRef = 603,
kIsInSameCurrentLocAsRefAlias = 604,
kLocAliasIsLocation = 605,
kGetKeywordDataForLocation = 606,
kSetKeywordDataForLocation = 607,
kGetKeywordDataForAlias = 608,
kSetKeywordDataForAlias = 609,
kLocAliasHasKeyword = 610,
kIsNullPackageData = 611,
kGetNumericPackageData = 612,
kIsFurnitureAnimType = 613,
kIsFurnitureEntryType = 614,
kGetHighestRelationshipRank = 615,
kGetLowestRelationshipRank = 616,
kHasAssociationTypeAny = 617,
kHasFamilyRelationshipAny = 618,
kGetPathingTargetOffset = 619,
kGetPathingTargetAngleOffset = 620,
kGetPathingTargetSpeed = 621,
kGetPathingTargetSpeedAngle = 622,
kGetMovementSpeed = 623,
kGetInContainer = 624,
kIsLocationLoaded = 625,
kIsLocAliasLoaded = 626,
kIsDualCasting = 627,
kDualCast = 628,
kGetVMQuestVariable = 629,
kGetVMScriptVariable = 630,
kIsEnteringInteractionQuick = 631,
kIsCasting = 632,
kGetFlyingState = 633,
kSetFavorState = 634,
kIsInFavorState = 635,
kHasTwoHandedWeaponEquipped = 636,
kIsExitingInstant = 637,
kIsInFriendStateWithPlayer = 638,
kGetWithinDistance = 639,
kGetActorValuePercent = 640,
kIsUnique = 641,
kGetLastBumpDirection = 642,
kCameraShake = 643,
kIsInFurnitureState = 644,
kGetIsInjured = 645,
kGetIsCrashLandRequest = 646,
kGetIsHastyLandRequest = 647,
kUpdateQuestInstanceGlobal = 648,
kSetAllowFlying = 649,
kIsLinkedTo = 650,
kGetKeywordDataForCurrentLocation = 651,
kGetInSharedCrimeFaction = 652,
kGetBribeAmount = 653,
kGetBribeSuccess = 654,
kGetIntimidateSuccess = 655,
kGetArrestedState = 656,
kGetArrestingActor = 657,
kClearArrestState = 658,
kEPTemperingItemIsEnchanted = 659,
kEPTemperingItemHasKeyword = 660,
kGetReceivedGiftValue = 661,
kGetGiftGivenValue = 662,
kForceLocIntoAlias = 663,
kGetReplacedItemType = 664,
kSetHorseActor = 665,
kPlayReferenceEffect = 666,
kStopReferenceEffect = 667,
kPlayShaderParticleGeometry = 668,
kStopShaderParticleGeometry = 669,
kApplyImageSpaceModifierCrossFade = 670,
kRemoveImageSpaceModifierCrossFade = 671,
kIsAttacking = 672,
kIsPowerAttacking = 673,
kIsLastHostileActor = 674,
kGetGraphVariableInt = 675,
kGetCurrentShoutVariation = 676,
kPlayImpactEffect = 677,
kShouldAttackKill = 678,
kSendStealAlarm = 679,
kGetActivationHeight = 680,
kEPModSkillUsage_IsAdvanceSkill = 681,
kWornHasKeyword = 682,
kGetPathingCurrentSpeed = 683,
kGetPathingCurrentSpeedAngle = 684,
kKnockAreaEffect = 685,
kInterruptCast = 686,
kAddFormToFormList = 687,
kRevertFormList = 688,
kAddFormToLeveledList = 689,
kRevertLeveledList = 690,
kEPModSkillUsage_AdvanceObjectHasKeyword = 691,
kEPModSkillUsage_IsAdvanceAction = 692,
kEPMagic_SpellHasKeyword = 693,
kGetNoBleedoutRecovery = 694,
kSetNoBleedoutRecovery = 695,
kEPMagic_SpellHasSkill = 696,
kIsAttackType = 697,
kIsAllowedToFly = 698,
kHasMagicEffectKeyword = 699,
kIsCommandedActor = 700,
kIsStaggered = 701,
kIsRecoiling = 702,
kIsExitingInteractionQuick = 703,
kIsPathing = 704,
kGetShouldHelp = 705,
kHasBoundWeaponEquipped = 706,
kGetCombatTargetHasKeyword = 707,
kUpdateLevel = 708,
kGetCombatGroupMemberCount = 709,
kIsIgnoringCombat = 710,
kGetLightLevel = 711,
kSavePCFace = 712,
kSpellHasCastingPerk = 713,
kIsBeingRidden = 714,
kIsUndead = 715,
kGetRealHoursPassed = 716,
kUnequipAll = 717,
kIsUnlockedDoor = 718,
kIsHostileToActor = 719,
kGetTargetHeight = 720,
kIsPoison = 721,
kWornApparelHasKeywordCount = 722,
kGetItemHealthPercent = 723,
kEffectWasDualCast = 724,
kGetKnockStateEnum = 725,
kDoesNotExist = 726,
kUnequipItemSlot = 727,
kMountActor = 728,
kDismountActor = 729,
kIsOnFlyingMount = 730,
kCanFlyHere = 731,
kIsFlyingMountPatrolQueued = 732,
kIsFlyingMountFastTravelling = 733,
kIsOverEncumbered = 734,
kGetActorWarmth = 735,
/;