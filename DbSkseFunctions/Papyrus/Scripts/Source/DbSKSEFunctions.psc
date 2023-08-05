Scriptname DbSkseFunctions Hidden 
;compiled with CommonLib NG, so should work with Skyrim SE to Skyrim AE 1.6.640.0.8

;returns 4.9
Float Function GetVersion() Global Native

;get and set text from system clipboard, for copy / paste functionality
String Function GetClipBoardText() Global Native
Bool Function SetClipBoardText(String s) Global Native

;string functions
;is the string c char whitespace? Uses c++ isspace function
Bool Function IsWhiteSpace(String c) Global Native 
int Function CountWhiteSpaces(String s) Global Native
   
;differs from consoleUtil.ExecuteCommand in that you can execute a targeted command on a passed in ref.
;if ref == none and command is targeted command, runs command on console selected ref like normal. 
;If no console selected ref, or is not a targeted command, executes command like normal.
Function ExecuteConsoleCommand(String command, ObjectReference ref = none) Global Native 

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

;get MusicType that's currently playing
MusicType Function GetCurrentMusicType() Global Native 

int Function GetNumberOfTracksInMusicType(MusicType akMusicType) Global Native 

;get the current track index queued in akMusicType
int Function GetMusicTypeTrackIndex(MusicType akMusicType) Global Native 

;if the akMusicType is currently playing, it will jump to the track index passed in.
Function SetMusicTypeTrackIndex(MusicType akMusicType, int index) Global Native 
    
int Function GetMusicTypePriority(MusicType akMusicType) Global Native 

Function SetMusicTypePriority(MusicType akMusicType, int priority) Global Native 

;status is as follows
;kInactive = 0
;kPlaying = 1
;kPaused = 2
;kFinishing = 3
;kFinished = 4
int Function GetMusicTypeStatus(MusicType akMusicType) Global Native 

Enchantment[] Function GetKnownEnchantments() Global Native 
Function AddKnownEnchantmentsToFormList(Formlist akList) Global Native 

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
int Function GetDetectionLevel(actor akActor, actor akTarget) Global Native

String Function GetKeywordString(keyword akKeyword) Global Native

;doesn't carry over between saves. Use load game event for maintenace
function SetKeywordString(keyword akKeyword, string keywordString) Global Native

;create new forms of these types at runtime.
;carefull with these. Using these functions are like using PlaceAtMe to create permanent references. 
;Making too many of these may cause save game bloat, so use sparingly.
Formlist Function CreateFormList(formlist fillerList = none) Global Native 
ColorForm Function CreateColorForm(int color = 0xf) Global Native 
Keyword Function CreateKeyword() Global Native 
ConstructibleObject Function CreateConstructibleObject() Global Native 
TextureSet Function CreateTextureSet() Global Native 
 
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


;returns true if Ref has map data.
bool function IsMapMarker(ObjectReference Ref) Global Native 

;For these functions the MapMarker ObjectReference must be a map marker with map marker data.
;In other words, IsMapMarker must return true;
int function GetMapMarkerIconType(ObjectReference MapMarker) Global Native

bool function SetMapMarkerIconType(ObjectReference MapMarker, int iconType) Global Native

string function GetMapMarkerName(ObjectReference MapMarker) Global Native

bool function SetMapMarkerName(ObjectReference MapMarker, String Name) Global Native
