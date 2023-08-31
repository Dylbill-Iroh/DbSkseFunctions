Scriptname DbSkseFunctions Hidden 
;compiled with CommonLib NG, so should work with Skyrim SE to Skyrim AE 1.6.640.0.8

;returns 5.3
Float Function GetVersion() Global Native

;get and set text from system clipboard, for copy / paste functionality
String Function GetClipBoardText() Global Native
Bool Function SetClipBoardText(String s) Global Native

;is the string c char whitespace? Uses c++ isspace function
Bool Function IsWhiteSpace(String c) Global Native 
int Function CountWhiteSpaces(String s) Global Native
   
;Calculate how many real time seconds it will take for gameHours to pass based on current time scale.
;Example - with default time scale (20), GameHoursToRealTimeSeconds(1) returns 180.0
float Function GameHoursToRealTimeSeconds(float gameHours) Global Native

bool Function IsGamePaused() Global Native

;returns true if a menu is open, (other than the hud menu which is always open), regardless if the game is paused or not.
bool Function IsInMenu() Global Native

string Function GetLastMenuOpened() Global Native

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


;set the max Soul size the soulGem can hold 0 = no soul to 5 = grand.
Function SetSoulGemSize(SoulGem akSoulGem, int level) Global Native 

;can the soulGem hold an NPC soul? I.E is it a black soul gem?
bool Function CanSoulGemHoldNPCSoul(SoulGem akSoulGem) Global Native 

;set soul gem can hold npc soul, I.E make it a black soul gem or not.
Function SetSoulGemCanHoldNPCSoul(SoulGem akSoulGem, bool canHold) Global Native 


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

Enchantment[] Function GetKnownEnchantments() Global Native 
Function AddKnownEnchantmentsToFormList(Formlist akList) Global Native 

String function GetWordOfPowerTranslation(WordOfPower akWord) Global Native 

;add shout to player if necessary and unlock all of its Words
function UnlockShout(shout akShout) Global Native 

;add and unlock all shouts to the player that match the param filters.
;default is adding and unlocking ALL shouts found in game to player
function AddAndUnlockAllShouts(int minNumberOfWordsWithTranslations = 0, bool onlyShoutsWithNames = false, bool onlyShoutsWithDescriptions = false) Global Native 

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

;not working currently, don't use. Will fix later.
ActiveMagicEffect[] Function GetActiveEffectsOnActor(Actor akActor, Actor casterFilter = none, spell SpellFilter = none, MagicEffect effectFilter = none, bool MatchAll = true) Global Native

;get source that the ActiveMagicEffect came from
;Could be a spell, enchantment, potion, or ingredient. Use GetType() to find out which.
Form Function GetActiveEffectSource(ActiveMagicEffect akEffect) Global Native

;Get casting source that the ActiveMagicEffect came from
;kLeftHand = 0,
;kRightHand = 1,
;kOther = 2, (most likely shout) 
;kInstant = 3
int Function GetActiveEffectCastingSource(ActiveMagicEffect akEffect) Global Native

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

;Create new sound. to set the sound, use Papyrus Extender, 'Po3_SkseFunctions.SetSoundDescriptor(newSoundMarker, akSoundDescriptor)'
Sound Function CreateSoundMarker() Global Native 

;PlaySound / PlaySoundDescriptor returns instanceID like Sound.play(), but you can pass in a form, alias or activeMagicEffect to receive the OnSoundFinish Event.
;Example, if your script extends form: 
;DbSkseFunctions.PlaySound(akSound, Game.GetPlayer(), 1.0, self) ;play sound and receive the OnSoundFinish event when sound finishes playing.
;You can also set a start volume.
int Function PlaySound(Sound akSound, ObjectReference akSource, float volume = 1.0, Form eventReceiverForm = none, Alias eventReceiverAlias = none, activeMagicEffect eventReceiverActiveEffect = none) Global Native 
int Function PlaySoundDescriptor(SoundDescriptor akSoundDescriptor, ObjectReference akSource, float volume = 1.0, Form eventReceiverForm = none, Alias eventReceiverAlias = none, activeMagicEffect eventReceiverActiveEffect = none) Global Native 

;sends the sound or soundDescriptor played and the instanceID
Event OnSoundFinish(Form SoundOrDescriptor, int instanceID)
EndEvent

SoundCategory Function GetParentSoundCategory(SoundCategory akSoundCategory) Global Native 
SoundCategory Function GetSoundCategoryForSoundDescriptor(SoundDescriptor akSoundDescriptor) Global Native 
Function SetSoundCategoryForSoundDescriptor(SoundDescriptor akSoundDescriptor, SoundCategory akSoundCategory) Global Native 
Float Function GetSoundCategoryVolume(SoundCategory akSoundCategory) Global Native 
Float Function GetSoundCategoryFrequency(SoundCategory akSoundCategory) Global Native 

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
