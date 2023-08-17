Scriptname DbMiscFunctions Hidden 

;/List of functions: =================================================================================================================
Function MoveToLocalOffset(ObjectReference RefToMove, ObjectReference CenterRef, Float Angle, Float Distance, float afZOffset = 0.0, bool abMatchRotation = true) global  
Function ApplyHavokImpulseLocal(ObjectReference Ref, Float Angle, float afZ, Float afMagnitude) Global
Bool Function ToggleCreationKitMarkers(Bool ShowMarkers = true, ObjectReference MoveToRef = none) Global
ObjectReference Function CreateXMarkerRef(Bool PersistentRef = false, ObjectReference PlaceAtMeRef = none) Global
Function DropAllItems(ObjectReference Ref, bool dropIndividual = false, float delay = 0.01) Global
Function DropAllItems_P03(ObjectReference Ref, bool noEquipped = true, bool noFavourited = false, bool noQuestItem = false, bool dropIndividual = false, float delay = 0.01) Global
Function DropIndividualItems(ObjectReference Ref, Form Item, int NumOfItems = 0, float delay = 0.01) Global
Bool Function LocationOrParentsHasKeyword(Location akLocation, Keyword akKeyword) global
Bool Function akFormHasKeywordString(Form akForm, String akString)  global
Bool Function FormHasKeywordInFormList(Form akForm, Formlist akList, Bool AllKeywords = False) Global 
Bool Function FormHasKeywordInArray(Form akForm, Keyword[] akList, Bool AllKeywords = False) Global
Bool Function FormHasKeywordInStorageUtilList(Form akForm, Form ObjKey, String ListKeyName, Bool AllKeywords = False) Global
Bool Function FormHasKeywordInJsonUtilList(Form akForm, String JsonFilePath, String ListKeyName, Bool AllKeywords = False) Global
Bool Function IsNumber(String akString, Bool AllowForDecimals = True, Bool AllowNegativeNumbers = True) global
Int Function ClampInt(Int i, Int Min, Int Max) Global
Float Function ClampFloat(Float f, Float Min, Float Max) Global
Bool Function IsIntInRange(Int I, Int Min, Int Max) Global
Bool Function IsFloatInRange(Float f, Float Min, Float Max) Global 
Bool Function IsStringIndexBetween(String s, Int Index, String StartKey, String EndKey) Global
String function ConvertIntToHex(int i, int minDigits = 8) Global
Int function ConvertHexToInt(string hex, Bool TreatAsNegative = false) global
String Function GetFormIDHex(Form akForm) Global
Int Function IntPow(Int x, Int y) Global
Int Function IntSqrt(Int i) Global 
Int Function IntAbs(Int i) Global 
int function RoundAsInt(Float f) Global
float function RoundAsFloat(Float f) Global
Float Function RoundDownToDec(Float f, Int DecimalPlaces = 0) Global 
String Function RoundDownToDecString(Float f, Int DecimalPlaces = 0) Global 
Int Function CountDecimalPlaces(Float f) Global
Float[] Function SplitAsFloat(String s, int Max = -1, String Divider = "||") Global
Int[] Function SplitAsInt(String s, int Max = -1, String Divider = "||") Global
String[] Function SortStringArray(String[] akArray, Bool Ascending = true, Bool Direct = true) Global
String[] Function CopyStringArray(String[] akArray) Global
String Function JoinStringArray(String[] akArray, String Divider = "||", Bool IgnoreDuplicates = false) Global
String Function JoinFloatArray(Float[] akArray, String Divider = "||", Bool IgnoreDuplicates = false) Global
String Function JoinIntArray(Int[] akArray, String Divider = "||", Bool IgnoreDuplicates = false) Global

Function PrintT(String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ", int aiSeverity = 0) Global

Function PrintTU(string asUserLog = "", String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ", int aiSeverity = 0) Global

Function PrintN(String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ") Global

Function PrintM(String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ") Global

Function PrintEvm(String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ") Global

Function PrintF(String FilePath, String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ") Global

String Function JoinStrings(String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ") Global

Int[] Function GetGameActorSoulLevels() Global
String[] Function GetGameSoulLevelNames() Global
Int Function GetActorSoulSize(Actor akActor) Global 
String Function GetActorSoulSizeString(Actor akActor, String sBlackSize = "Black") Global
Bool Function IsActorNPC(Actor akActor) Global
Bool Function IsActorMoving(Actor akActor) Global 
Form Function GetRandomFormFromRef(ObjectReference Ref, Int[] TypeArrayFilter = None, Formlist ListFilter = None, Bool TypeFilterHasType = true, Bool akListHasForm = true) Global
Form Function GetRandomFormFromRefA(ObjectReference Ref, Int[] TypeArrayFilter = None, Form[] ListFilter = None, Bool TypeFilterHasType = true, Bool akListHasForm = true) Global
Form Function GetRandomFormFromRefS(ObjectReference Ref, Int[] TypeArrayFilter = None, Form ObjKey = None, String ListKeyName = "", Bool TypeFilterHasType = true, Bool akListHasForm = true) Global
Form Function GetRandomFormFromRefJ(ObjectReference Ref, Int[] TypeArrayFilter = None, String JsonFilePath = "", String ListKeyName = "", Bool TypeFilterHasType = true, Bool akListHasForm = true) Global
Function SortActorArrayByName(Actor[] akArray, Bool Ascending = true) Global
Function SortObjectRefArrayByName(ObjectReference[] akArray, Bool Ascending = true) Global
Function SortFormArrayByName(Form[] akArray, Bool Ascending = true) Global
String[] Function GetActorNames(Actor[] akArray) Global
String[] Function GetObjectRefNames(ObjectReference[] akArray) Global
String[] Function GetFormNames(Form[] akArray) Global
String[] Function GetFormNamesFromList(Formlist akList) Global
Form[] Function FormlistToArray(Formlist akList) Global 
Function AddFormArrayFormsToList(Form[] akArray, Formlist akList) Global
Function RegisterFormForKeys(Form akForm, Int min = 1, Int Max = 281) Global
Function RegisterAliasForKeys(Alias akAlias, Int min = 1, Int Max = 281) Global
Function RegisterActiveMagicEffectForKeys(ActiveMagicEffect akActiveMagicEffect, Int min = 1, Int Max = 281) Global

Function SwapStrings(String[] akArray, Int IndexA, Int IndexB)
Function SwapStringsV(String[] akArray, Int IndexA, Int IndexB) 
Function SwapBools(Bool[] akArray, Int IndexA, Int IndexB)
Function SwapBoolsV(Bool[] akArray, Int IndexA, Int IndexB) 
Function SwapInts(Int[] akArray, Int IndexA, Int IndexB)
Function SwapIntsV(Int[] akArray, Int IndexA, Int IndexB) 
Function SwapFloats(Float[] akArray, Int IndexA, Int IndexB)
Function SwapFloatsV(Float[] akArray, Int IndexA, Int IndexB) 
Function SwapActors(Actor[] akArray, Int IndexA, Int IndexB)
Function SwapActorsV(Actor[] akArray, Int IndexA, Int IndexB) 
Function SwapObjectReferences(ObjectReference[] akArray, Int IndexA, Int IndexB)
Function SwapObjectReferencesV(ObjectReference[] akArray, Int IndexA, Int IndexB) 
Function SwapForms(Form[] akArray, Int IndexA, Int IndexB)
Function SwapFormsV(Form[] akArray, Int IndexA, Int IndexB) 

int function JsonIntListPluck(string FileName, string KeyName, int index, int default = 0) global 
Float function JsonFloatListPluck(string FileName, string KeyName, int index, Float default = 0.0) global 
string function JsonStringListPluck(string FileName, string KeyName, int index, string default = "") global 
Form function JsonFormListPluck(String FileName, String KeyName, int index, Form default = none) global 

int function JsonintListShift(string FileName, string KeyName, int default = 0) global 
Float function JsonFloatListShift(string FileName, string KeyName, Float default = 0.0) global 
String function JsonStringListShift(string FileName, string KeyName, String default = "") global 
Form function JsonFormListShift(string FileName, string KeyName, Form default = none) global 

Int function JsonIntListPop(string FileName, string KeyName, Int default = 0) global 
Float function JsonFloatListPop(string FileName, string KeyName, Float default = 0.0) global 
String function JsonStringListPop(string FileName, string KeyName, String default = "") global 
Form function JsonFormListPop(string FileName, string KeyName, Form default = none) global 

Function JsonIntListRemoveAllDuplicates(string FileName, string KeyName, Bool Acending = True) Global
Function JsonFloatListRemoveAllDuplicates(String FileName, String KeyName, Bool Acending = True) Global
Function JsonStringListRemoveAllDuplicates(string FileName, string KeyName, Bool Acending = True) Global
Function JsonFormListRemoveAllDuplicates(String FileName, String KeyName, Bool Acending = True) Global

String Function JsonJoinIntList(string FileName, string KeyName, string Divider = "||") Global
String Function JsonJoinFloatList(string FileName, string KeyName, string Divider = "||") Global
String Function JsonJoinStringList(string FileName, string KeyName, string Divider = "||") Global

String Function GetFormTypeString(Int Type, String sFilePath = "Data/interface/DbMiscFunctions/DbFormTypeStrings.txt") Global
String Function GetKeyCodeString(Int keyCode, String sFilePath = "Data/interface/DbMiscFunctions/DbKeyCodeStrings.txt") Global
String Function GetModOriginName(Form akForm) Global

get common form types without skse. See the actual functions for which types they return. Different from skse's Form.GetType()
int Function GetActorFormType(Form F) Global
int Function GetAudioFormType(Form F) Global
int Function GetCharacterFormType(Form F) Global
int Function GetItemFormType(Form F) Global
int Function GetMagicFormType(Form F) Global
int Function GetMiscFormType(Form F) Global
int Function GetSpecialEffectFormType(Form F) Global
int Function GetWorldDataFormType(Form F) Global
int Function GetWorldObjectFormType(Form F) Global
int Function GetInventoryItemFormType(Form F) Global
Int Function GetFormTypeAll(Form F) Global

String Function GetActorFormTypeString(Form F, String[] TypeStrings = none) Global
String Function GetAudioFormTypeString(Form F, String[] TypeStrings = none) Global
String Function GetCharacterFormTypeString(Form F, String[] TypeStrings = none) Global
String Function GetItemFormTypeString(Form F, String[] TypeStrings = none) Global
String Function GetMagicFormTypeString(Form F, String[] TypeStrings = none) Global
String Function GetMiscFormTypeString(Form F, String[] TypeStrings = none) Global
String Function GetSpecialEffectFormTypeString(Form F, String[] TypeStrings = none) Global
String Function GetWorldDataFormTypeString(Form F, String[] TypeStrings = none) Global
String Function GetWorldObjectFormTypeString(Form F, String[] TypeStrings = none) Global
String Function GetInventoryItemFormTypeString(Form F, String[] TypeStrings = none) Global
string Function GetFormTypeStringAll(Form F, String[] TypeStrings = none) Global

Function DisableThenEnablePlayerControls(Float Delay = 1.0) Global
Function UpdateActor(Actor akActor, Form akForm) Global
Function SwapEquipment(Actor A, Actor B) Global
Function SetActorValues(Actor akActor, String[] ActorValues, Float[] Values) Global
Function ModActorValues(Actor akActor, String[] ActorValues, Float[] Values) Global
Float[] Function GetActorValues(Actor akActor, String[] ActorValues, DynamicArrays DArrays = none) Global
String[] Function GetActorValueStrings(Actor akActor, String[] ActorValues, String[] ActorValueStrings = none, DynamicArrays DArrays = none) Global
String Function sGetActorValueStrings(Actor akActor, String[] ActorValues, String[] ActorValueStrings = none, String Divider = "||") Global
Float[] Function GetBaseActorValues(Actor akActor, String[] ActorValues, DynamicArrays DArrays = none) Global
String[] Function GetBaseActorValueStrings(Actor akActor, String[] ActorValues, String[] ActorValueStrings = none, DynamicArrays DArrays = none) Global
String Function sGetBaseActorValueStrings(Actor akActor, String[] ActorValues, String[] ActorValueStrings = none, String Divider = "||") Global
Float[] Function GetActorValuesFromFile(Actor akActor, String filePath = "Data/interface/DbMiscFunctions/DbActorValues.txt") Global
String[] Function GetActorValueStringsFromFile(Actor akActor, String filePath = "Data/interface/DbMiscFunctions/DbActorValues.txt") Global
String Function sGetActorValueStringsFromFile(Actor akActor, String Divider = "||", String filePath = "Data/interface/DbMiscFunctions/DbActorValues.txt") Global
Float[] Function GetBaseActorValuesFromFile(Actor akActor, String filePath = "Data/interface/DbMiscFunctions/DbActorValues.txt") Global
String[] Function GetBaseActorValueStringsFromFile(Actor akActor, String filePath = "Data/interface/DbMiscFunctions/DbActorValues.txt") Global
String Function sGetBaseActorValueStringsFromFile(Actor akActor, String Divider = "||", String filePath = "Data/interface/DbMiscFunctions/DbActorValues.txt") Global

Function AttachPapyrusScript(String akScript, ObjectReference Ref) Global
Function OpenMenu(string menuName) Global
Function CloseMenu(string menuName) Global

Int Function FindLastStringIndex(String s, String ToFind) Global
Int Function FindWholeWordString(String s, String ToFind, Int StartIndex = 0) Global
Bool Function IsCharWhiteSpace(String C) Global
String Function FindNextWordInString(String s, int startIndex = 0) global 
String Function RFindNextWordInString(String s, int startIndex = -1) global 
Int function FindNextNonWhiteSpaceCharIndexInString(String s, int startIndex = 0) global
Int function RFindNextNonWhiteSpaceCharIndexInString(String s, int startIndex = -1) global
Int Function FindNextWhiteSpaceCharIndexInString(String s, int startIndex = 0) global
Int Function RFindNextWhiteSpaceCharIndexInString(String s, int startIndex = -1) global
String Function FindNextNonWhiteSpaceCharInString(String s, int startIndex = 0) global
String Function RFindNextNonWhiteSpaceCharInString(String s, int startIndex = -1) global
String Function FindNextWhiteSpaceCharInString(String s, int startIndex = 0) global
String Function RFindNextWhiteSpaceCharInString(String s, int startIndex = -1) global
String Function RemoveWhiteSpaces(String s, Bool IncludeSpaces = True, Bool IncludeTabs = true, Bool IncludeNewLines = true) Global
Int Function CountWhiteSpaces(String s, Bool IncludeSpaces = True, Bool IncludeTabs = true, Bool IncludeNewLines = true) Global 
Int Function CountStringsInString(String s, String ToFind, Bool WholeWordsOnly = false) Global
String Function StringReplace(String TargetStr, String SearchStr, String ReplaceStr, Int Count = 0) Global
String Function StringInsert(String TargetStr, String InsertStr, Int CharPosition = -1) Global
String Function StringRemoveCharAt(String s, Int Index) Global
String Function StringRemoveNonPrintableCharacters(String s) Global
String Function StringRemovePrintableCharacters(String s) Global
String Function AddPrefixToString(String s, String Prefix, Bool OnlyIfNotPresent = true) Global
String Function AddPrefixToStrings(String[] s, String Prefix, Bool OnlyIfNotPresent = true) Global
String Function RemovePrefixFromString(String s, String Prefix) Global
String Function RemovePrefixFromStrings(String[] s, String Prefix) Global
String Function AddSuffixToString(String s, String Suffix, Bool OnlyIfNotPresent = true) Global
String Function AddSuffixToStrings(String[] s, String Suffix, Bool OnlyIfNotPresent = true) Global
String Function RemoveSuffixFromString(String s, String Suffix) Global
String Function RemoveSuffixFromStrings(String[] s, String Suffix) Global
Function AddPrefixToFormName(Form akForm, String Prefix, Bool OnlyIfNotPresent = true) Global
Function AddPrefixToFormNames(Form[] akForm, String Prefix, Bool OnlyIfNotPresent = true) Global
Function RemovePrefixFromFormName(Form akForm, String Prefix) Global
Function RemovePrefixFromFormNames(Form[] akForm, String Prefix) Global
Function AddSuffixToFormName(Form akForm, String Suffix, Bool OnlyIfNotPresent = true) Global
Function AddSuffixToFormNames(Form[] akForm, String Suffix, Bool OnlyIfNotPresent = true) Global
Function RemoveSuffixFromFormName(Form akForm, String Suffix) Global
Function RemoveSuffixFromFormNames(Form[] akForm, String Suffix) Global
Bool Function StringHasPrefix(String s, String Prefix) Global
Bool Function StringHasSuffix(String s, String Suffix) Global

String Function GetStringFromFile(String StringKey, String FileContents = "", String FilePath = "", String StartKey = "[", String EndKey = "]", String Default = "") global 
int Function GetIntFromFile(String StringKey, String FileContents = "", String FilePath = "", String StartKey = "[", String EndKey = "]", int Default = -1) global 
Float Function GetFloatFromFile(String StringKey, String FileContents = "", String FilePath = "", String StartKey = "[", String EndKey = "]", Float Default = -1.0) global 
String[] Function GetAllStringsFromFile(String FileContents = "", String FilePath = "", String RangeStart = "", String RangeEnd = "", String StartKey = "[", String EndKey = "]", String[] Default = None) global
Int[] Function GetAllIntsFromFile(String FileContents = "", String FilePath = "", String RangeStart = "", String RangeEnd = "", String StartKey = "[", String EndKey = "]", Int[] Default = None) global
Float[] Function GetAllFloatsFromFile(String FileContents = "", String FilePath = "", String RangeStart = "", String RangeEnd = "", String StartKey = "[", String EndKey = "]", Float[] Default = None) global
Bool Function PrintStringKeysToFile(String FilePathToSearch, String FilePathToPrintTo, String StartKey = "[", String EndKey = "]", String FinishedMsg = "Done Printing") global
Function PrintContainerItemsToFile(ObjectReference akContainer, String FilePath, String ConfirmMessage = "") global
Function WriteIDsInFormListToFile(Formlist akList, String FilePath, Bool IncludeNames = False, Bool ReplaceIdStartWith0x = true) Global
Function WriteIDsInFormArrayToFile(Form[] akList, String FilePath, Bool IncludeNames = False, Bool ReplaceIdStartWith0x = true) Global
Function WriteIDsInStorageUtilListToFile(Form ObjKey, String ListKeyName, String FilePath, Bool IncludeNames = False, Bool ReplaceIdStartWith0x = true) Global 
Function WriteIDsInJsonUtilListToFile(String JsonFilePath, String ListKeyName, String FilePath, Bool IncludeNames = False, Bool ReplaceIdStartWith0x = true) Global 
Function WriteAnimationVariableBoolsToFile(ObjectReference akRef, String OutputFilePath, String VariablesSourceFilePath = "Data/interface/DbMiscFunctions/DbAnimationVariableBools.txt") Global
Function WriteAnimationVariableIntsToFile(ObjectReference akRef, String OutputFilePath, String VariablesSourceFilePath = "Data/interface/DbMiscFunctions/DbAnimationVariableInts.txt") Global
Function WriteAnimationVariableFloatsToFile(ObjectReference akRef, String OutputFilePath, String VariablesSourceFilePath = "Data/interface/DbMiscFunctions/DbAnimationVariableFloats.txt") Global
    
Function WriteAllAnimationVariablesToFile(ObjectReference akRef, String OutputFilePath, String BoolVariablesSourceFilePath = "Data/interface/DbMiscFunctions/DbAnimationVariableBools.txt", \
                                                                                String IntVariablesSourceFilePath = "Data/interface/DbMiscFunctions/DbAnimationVariableInts.txt", \
                                                                                String FloatVariablesSourceFilePath = "Data/interface/DbMiscFunctions/DbAnimationVariableFloats.txt") Global

Function RegisterFormForAnimationEvents(Form akForm, ObjectReference akSender, String[] AnimationEvents) Global
Function RegisterAliasForAnimationEvents(Alias akAlias, ObjectReference akSender, String[] AnimationEvents) Global
Function RegisterActiveMagicEffectForAnimationEvents(ActiveMagicEffect akActiveMagicEffect, ObjectReference akSender, String[] AnimationEvents) Global
Function RegisterFormForAnimationEventsFromFile(Form akForm, ObjectReference akSender, String FilePath = "Data/interface/DbMiscFunctions/DbAnimationEvents.txt") Global
Function RegisterAliasForAnimationEventsFromFile(Alias akAlias, ObjectReference akSender, String FilePath = "Data/interface/DbMiscFunctions/DbAnimationEvents.txt") Global
Function RegisterActiveMagicEffectForAnimationEventsFromFile(ActiveMagicEffect akActiveMagicEffect, ObjectReference akSender, String FilePath = "Data/interface/DbMiscFunctions/DbAnimationEvents.txt") Global

Function RegisterFormForMenus(Form akForm, String[] Menus) Global
Function RegisterAliasForMenus(Alias akAlias, String[] Menus) Global
Function RegisterActiveMagicEffectForMenus(ActiveMagicEffect akActiveMagicEffect, String[] Menus) Global
Function RegisterFormForMenusFromFile(Form akForm, String FilePath = "Data/interface/DbMiscFunctions/DbMenus.txt") Global
Function RegisterAliasForMenusFromFile(Alias akAlias, String FilePath = "Data/interface/DbMiscFunctions/DbMenus.txt") Global
Function RegisterActiveMagicEffectForMenusFromFile(ActiveMagicEffect akActiveMagicEffect, String FilePath = "Data/interface/DbMiscFunctions/DbMenus.txt") Global

Function WriteAllPscDataInFolderToFile(String SearchFolderPath, String TargetFilePath, String Divider = "\n", String DoneMessage = "Done Writing") Global    
String Function GetPscEventNamesFromFile(String SourceFilePath, String Divider = "\n", int StartIndex = 0) Global
String Function GetPscFunctionNamesFromFile(String SourceFilePath, String Divider = "\n", int StartIndex = 0) Global
String Function GetPscDataNamesFromFile(String SourceFilePath, String NameType, String Divider = "\n", int StartIndex = 0) Global
String Function GetPscEventDefinitionsFromFile(String SourceFilePath, String Divider = "\n", int StartIndex = 0) Global 
String Function GetPscFunctionDefinitionsFromFile(String SourceFilePath, String Divider = "\n", int StartIndex = 0) Global 
String Function GetPscDataDefinitionsFromFile(String SourceFilePath, String NameType, String Divider = "\n", int StartIndex = 0) Global
    
Function WriteJsonSaveAndLoadFunctionsToFile(String SourceFilePath, String DestinationFilePath = "", \
    Bool GlobalVariablesToggle = true, Bool FloatsToggle = true, Bool StringsToggle = true, Bool IntsToggle = true, Bool BoolsToggle = true, \
    Bool GlobalVariableArraysToggle = true, Bool FloatArraysToggle = true, Bool StringArraysToggle = true, Bool IntArraysToggle = true, Bool BoolArraysToggle = true, \
    int Messages = 0, String ConfirmMessage = "Done Writing Json Functions", Bool UsePropertiesAsDefaults = True) global

===================================================================================================================================================================================
/;

;Like MoveTo, but can specifify local angle / distance offset.
;If angle == 0.0, moves object in front of CenterRef by Distance
;If angle == 90.0 moves object the right of CenterRef by Distance
;If angle == -90, moves object to the left of centerRef  by Distance
;If angle == 180, moves object behind centerRef by Distance ect.
;Example: MoveToLocalOffset(MyRef, Game.GetPlayer(), 90.0, 500.0) moves MyRef 500 units to the right of the player.
;No requirements
Function MoveToLocalOffset(ObjectReference RefToMove, ObjectReference CenterRef, Float Angle, Float Distance, float afZOffset = 0.0, bool abMatchRotation = true) global  
    
    float A = CenterRef.GetAngleZ() + Angle
    Float XDist = math.Sin(A)
    Float YDist = Math.Cos(A)
    XDist *= Distance
    YDist *= Distance
    
    RefToMove.MoveTo(CenterRef, XDist, YDist, afZOffset, abMatchRotation)
EndFunction  

;PlaceAtMe but moves placed ref using MoveToLocalOffset function. PlaceAtMeRef is centerRef, new placed ref is RefToMove.
ObjectReference Function PlaceAndMoveToLocalOffset(ObjectReference PlaceAtMeRef, Form akFormToPlace, int aiCount = 1, bool abForcePersist = false, bool abInitiallyDisabled = false, \
Float Angle = 0.0, Float Distance = 100.0, float afZOffset = 0.0, bool abMatchRotation = true) global

    ObjectReference Ref = PlaceAtMeRef.PlaceAtMe(akFormToPlace, aiCount, abForcePersist, true)
    MoveToLocalOffset(Ref, PlaceAtMeRef, Angle, Distance, afZOffset, abMatchRotation)
    If !abInitiallyDisabled 
        Ref.Enable()
    Endif
Endfunction

;Apply Havok Impulse from left / right angle + Z direction. No requirements.
;Examples: 
;ApplyHavokImpulseLocal(MyRef, 0, 5, 10) applies havok impulse so the ref flies forward and up 
;ApplyHavokImpulseLocal(MyRef, 90, -5, 10) applies havok impulse so the ref flies to the right and down 
Function ApplyHavokImpulseLocal(ObjectReference Ref, Float Angle, float afZ, Float afMagnitude) Global
    float A = Ref.GetAngleZ() + Angle
    Float afX = math.Sin(A)
    Float afY = Math.Cos(A)
    
    Ref.ApplyHavokImpulse(afX, afY, afZ, afMagnitude)
EndFunction 

;show or hide creation kit markers in Game 
;If ShowMarkers == true, shows them, otherwise hides them.
;If moveToRef == None (default), moves player to either whiterun or markarth fast travel markers.
;Must move player to new area after changing bShowMarkers ini so cells reload to display markers.
;no requirements 
Bool Function ToggleCreationKitMarkers(Bool ShowMarkers = true, ObjectReference MoveToRef = none) Global
    ObjectReference xMarkerRef = CreateXMarkerRef()
    If !xMarkerRef
        Debug.Notification("Couldn't create xMarkerRef")
        return false 
    Endif 
    
    Utility.SetINIBool("bShowMarkers:Display", ShowMarkers)
    
    If MoveToRef == None
        ObjectReference WhiteRunMapMarker = Game.GetFormFromFile(0x0C07EF, "Skyrim.esm") as ObjectReference
        If !WhiteRunMapMarker 
            Debug.Notification("WhiteRunMapMarker not found")
            return false 
        Endif 
        
        ObjectReference MarkarthMapMarker = Game.GetFormFromFile(0x047015, "Skyrim.esm") as ObjectReference
        If !MarkarthMapMarker 
            Debug.Notification("MarkarthMapMarker not found")
            return false 
        Endif 
        
        If Game.GetPlayer().GetDistance(WhiteRunMapMarker) > Game.GetPlayer().GetDistance(MarkarthMapMarker)
            Game.GetPlayer().MoveTo(WhiteRunMapMarker)
        Else 
            Game.GetPlayer().MoveTo(MarkarthMapMarker)
        Endif 
    Else 
        Game.GetPlayer().MoveTo(MoveToRef)
    Endif
    
    Utility.Wait(2) 
    Game.GetPlayer().MoveTo(xMarkerRef)
    xMarkerRef.Disable()
    xMarkerRef.Delete()
    Return True
EndFunction

;create new xMarker ObjectReference 
;if PlaceAtMeRef == none (default) places new marker at the player.
;no requirements
ObjectReference Function CreateXMarkerRef(Bool PersistentRef = false, ObjectReference PlaceAtMeRef = none) Global
    Static xMarker = Game.GetFormFromFile(0x00003B, "Skyrim.esm") as Static
    If xMarker 
        ObjectReference xMarkerRef
        If PlaceAtMeRef == None 
            xMarkerRef = Game.GetPlayer().PlaceAtMe(xMarker, 1, PersistentRef)
        Else
            xMarkerRef = PlaceAtMeRef.PlaceAtMe(xMarker, 1, PersistentRef)
        Endif
        Return xMarkerRef 
    Endif 
EndFunction 

;Drop all items from ref, Ref must be a container or actor.
;If dropIndividual is true, drops multiple of the same item time individually so they don't stack. If false, items are dropped stacked. 
;Requires SKSE
Function DropAllItems(ObjectReference Ref, bool dropIndividual = false, float delay = 0.01) Global
    Int i = Ref.GetNumItems()
    If !dropIndividual
        While i > 0 
            i -= 1
            Form akForm = Ref.GetNthForm(i) 
            If akForm 
                Ref.DropObject(akForm, ref.GetItemCount(akForm))
                Utility.WaitMenuMode(delay)
            Endif 
        EndWhile
    Else 
        While i > 0 
            i -= 1
            Form akForm = Ref.GetNthForm(i) 
            If akForm 
                DropIndividualItems(Ref, akForm, 0, delay)
            Endif 
        EndWhile
    Endif
EndFunction

;Like DropAllItems except uses Papyrus Extender to filter items.
;Requires skse and Papyrus Extender
;Commented out for disparity between LE and SE
;Function DropAllItems_P03(ObjectReference Ref, bool noEquipped = true, bool noFavourited = false, bool noQuestItem = false, bool dropIndividual = false, float delay = 0.01) Global
;    Form[] Items = PO3_SKSEFunctions.AddAllItemsToArray(Ref, noEquipped, noFavourited, noQuestItem) ;for SE
;    Form[] Items = PO3_SKSEFunctions.AddAllInventoryItemsToArray(Ref, noEquipped, noFavourited, noQuestItem) ;for LE
;    Int i = Items.length 
;    
;    If !dropIndividual 
;        While i > 0 
;            i -= 1
;            Form akForm = Items[i]
;            If akForm 
;                Ref.DropObject(akForm, ref.GetItemCount(akForm))
;                Utility.WaitMenuMode(delay)
;            Endif 
;        EndWhile
;    Else 
;         While i > 0 
;            i -= 1
;            Form akForm = Items[i]
;            If akForm 
;                DropIndividualItems(Ref, akForm, 0, delay)
;            Endif 
;         EndWhile
;    Endif
;EndFunction

;Drop the NumofItems from ref individually so they don't stack.
;If NumofItems == 0, drops all of the item from ref.
;No requirements
Function DropIndividualItems(ObjectReference Ref, Form Item, int NumOfItems = 0, float delay = 0.01) Global
    If NumOfItems < 1
        NumOfItems = ref.GetItemCount(Item) 
    Endif
    
    While NumOfItems > 0 
        NumOfItems -= 1 
        ref.DropObject(Item, 1)
        Utility.WaitMenuMode(delay)
    EndWhile
EndFunction

;add shout to player if necessary and unlock all words of power.
;requires skse
Function UnlockShout(shout akShout) Global
	if !akShout 
		return 
	Endif
	
	if !Game.GetPlayer().HasSpell(akShout)
		Game.GetPlayer().AddShout(akShout)
	Endif
	
	wordofPower akWord = akShout.GetNthWordOfPower(0)
	if akWord 
		Game.TeachWord(akWord)
		Game.UnlockWord(akWord)
	Endif 
	
	akWord = akShout.GetNthWordOfPower(1)
	if akWord 
		Game.TeachWord(akWord)
		Game.UnlockWord(akWord)
	Endif 
	
	akWord = akShout.GetNthWordOfPower(2)
	if akWord 
		Game.TeachWord(akWord)
		Game.UnlockWord(akWord)
	Endif 
EndFunction

;requires skse
Function UnlockEquippedShout() Global
	shout akShout = Game.GetPlayer().GetEquippedShout()
	if akShout 
		UnlockShout(akShout)
	Endif
EndFunction

;check if location or any of it's parents has the keyword 
;Requires Papyrus Extender && SKSE
Bool Function LocationOrParentsHasKeyword(Location akLocation, Keyword akKeyword) global
    int i = 0
    
    While akLocation != none && i < 50 ;max times to try is 50.
        If akLocation.HasKeyWord(akKeyword)
           return true 
        Else 
            akLocation = PO3_SKSEFunctions.GetParentLocation(akLocation)
        Endif
        i += 1
    EndWhile

    Return False
EndFunction

;no requirements.
string Function GetStringIfNull(string s, string nullString = "null") Global
	if s != ""
		return s 
	else
		return nullString
	endif
EndFunction

;requires skse. Get name of form. Checks for ObjectReference. return default nullString if not found.
string Function GetFormName(Form akForm, string nullString = "null", string NoNameString = "no name") Global
	if !akForm 
		return nullString 
	endif
	
	string name
	ObjectReference ref = akForm as ObjectReference 
	
	if ref 
		name = ref.GetDisplayName()
	else
		name = akForm.GetName()
	endif 
	
	return GetStringIfNull(name, NoNameString)
EndFunction

;Like HasKeywordString but returns true if multiple esp's have keyWords with the same name added.
;Requires SKSE
Bool Function akFormHasKeywordString(Form akForm, String akString)  global
    Keyword[] akKeywords = akForm.GetKeywords()
    
    int M = akKeywords.Length 
    While M > 0 
        M -= 1 
        If akKeywords[M].GetString() == akString 
            Return True 
        Endif 
    EndWhile 
    
    Return False 
EndFunction

;If AllKeywords == false (default) returns true if the akForm has any keyword in the akList formlist. 
;If allKeywords == true, only returns true if the akForm has all keywords in the akList. 
;No requirements
Bool Function FormHasKeywordInFormList(Form akForm, Formlist akList, Bool AllKeywords = False) Global 
    Int i = 0
    Int L = akList.GetSize() 
    
    If !AllKeywords 
        While i < L 
            Keyword k = akList.GetAt(i) as Keyword 
            If k 
                If akForm.HasKeyword(k) 
                    return true 
                Endif 
            Endif 
            
            i += 1 
        EndWhile 
        
        Return false
    Else 
        While i < L 
            Keyword k = akList.GetAt(i) as Keyword 
            If k 
                If !akForm.HasKeyword(k) 
                    return false 
                Endif 
            Endif 
            
            i += 1 
        EndWhile 
        
        Return true
    Endif
EndFunction

;If AllKeywords == false (default) returns true if the akForm has any keyword in the akList array. 
;If allKeywords == true, only returns true if the akForm has all keywords in the akList. 
;No requirements
Bool Function FormHasKeywordInArray(Form akForm, Keyword[] akList, Bool AllKeywords = False) Global
    Int i = 0
    Int L = akList.Length
    
    If !AllKeywords 
        While i < L 
            Keyword k = akList[i]
            If k 
                If akForm.HasKeyword(k) 
                    return true 
                Endif 
            Endif 
            
            i += 1 
        EndWhile 
        
        Return false
    Else 
        While i < L 
            Keyword k = akList[i]
            If k 
                If !akForm.HasKeyword(k) 
                    return false 
                Endif 
            Endif 
            
            i += 1 
        EndWhile 
        
        Return true
    Endif
EndFunction

;If AllKeywords == false (default) returns true if the akForm has any keyword in the StorageUtil Form list. 
;If allKeywords == true, only returns true if the akForm has all keywords in the List. 
;Requires skse and PapyrusUtil
Bool Function FormHasKeywordInStorageUtilList(Form akForm, Form ObjKey, String ListKeyName, Bool AllKeywords = False) Global
    Int i = 0 
    Int L = StorageUtil.FormListCount(ObjKey, ListKeyName)
    
    If !AllKeywords 
        While i < L 
            Keyword k = StorageUtil.FormListGet(ObjKey, ListKeyName, i) as Keyword
            If k 
                If akForm.HasKeyword(k) 
                    return true 
                Endif 
            Endif 
            
            i += 1 
        EndWhile 
        
        Return false
    Else 
        While i < L 
            Keyword k = StorageUtil.FormListGet(ObjKey, ListKeyName, i) as Keyword
            If k 
                If !akForm.HasKeyword(k) 
                    return false 
                Endif 
            Endif 
            
            i += 1 
        EndWhile 
        
        Return true
    Endif
EndFunction

;If AllKeywords == false (default) returns true if the akForm has any keyword in the JsonUtil Form list. 
;If allKeywords == true, only returns true if the akForm has all keywords in the List. 
;Requires skse and PapyrusUtil
Bool Function FormHasKeywordInJsonUtilList(Form akForm, String JsonFilePath, String ListKeyName, Bool AllKeywords = False) Global
    Int i = 0 
    Int L = JsonUtil.FormListCount(JsonFilePath, ListKeyName)
    
    If !AllKeywords 
        While i < L 
            Keyword k = JsonUtil.FormListGet(JsonFilePath, ListKeyName, i) as Keyword
            If k 
                If akForm.HasKeyword(k) 
                    return true 
                Endif 
            Endif 
            
            i += 1 
        EndWhile 
        
        Return false
    Else 
        While i < L 
            Keyword k = JsonUtil.FormListGet(JsonFilePath, ListKeyName, i) as Keyword
            If k 
                If !akForm.HasKeyword(k) 
                    return false 
                Endif 
            Endif 
            
            i += 1 
        EndWhile 
        
        Return true
    Endif
EndFunction

;Requires SKSE
Bool Function IsNumber(String akString, Bool AllowForDecimals = True, Bool AllowNegativeNumbers = True) global
    ;is the string a number?
    int I = 0 
    int M = (StringUtil.GetLength(akString))
        
    If AllowNegativeNumbers 
        If StringUtil.GetNthChar(akString, 0) == "-" 
            I = 1
        Endif 
    Endif
    
    If AllowForDecimals
        int PeriodCount = 0
        While I < M
            String CurrentChar = StringUtil.GetNthChar(akString, I)
            
            If CurrentChar == "." 
                PeriodCount += 1 
                If PeriodCount > 1 
                    Return False 
                Endif 
            Elseif (StringUtil.IsDigit(CurrentChar) == false)
                Return false 
            Endif
            
            I += 1
        EndWhile
    Else 
        While I < M
            String CurrentChar = StringUtil.GetNthChar(akString, I)
            If (StringUtil.IsDigit(CurrentChar) == false) ; don't allow decimals.
                Return false 
            Endif
            I += 1
        EndWhile
    Endif
    
    Return True
EndFunction     

Int Function ClampInt(Int i, Int Min, Int Max) Global
    If i < Min 
        i = Min 
    Elseif i > Max 
        i = Max 
    Endif 
    
    Return i
EndFunction 

Float Function ClampFloat(Float f, Float Min, Float Max) Global
    If f < Min 
        f = Min 
    Elseif f > Max 
        f = Max 
    Endif 
    
    Return f
EndFunction

Bool Function IsIntInRange(Int I, Int Min, Int Max) Global
    return (I >= Min && I <= Max)
EndFunction

Bool Function IsFloatInRange(Float f, Float Min, Float Max) Global 
    return (f >= Min && f <= Max)
EndFunction

;is the index in string s between the StartKey and EndKey.
;Example: 
;String s = "() (Some String)"
;Bool b = DbMiscFunctions.IsStringIndexBetween(s, 4, "(", ")") ;true
;Bool bb = DbMiscFunctions.IsStringIndexBetween(s, 2, "(", ")") ;false
;Bool bbb = DbMiscFunctions.IsStringIndexBetween(s, 0, "(", ")") ;false
;requires skse
Bool Function IsStringIndexBetween(String s, Int Index, String StartKey, String EndKey) Global
    Int StartKeyLength = StringUtil.GetLength(StartKey)
    Int iStart = StringUtil.Find(s, StartKey) 
    Int iEnd = StringUtil.Find(s, EndKey, iStart)
    
    While iStart > -1 && iStart < Index && iEnd > -1 
        If DbMiscFunctions.IsIntInRange(Index, iStart, iEnd) 
            Return True 
        Endif 
        
        iStart += StartKeyLength 
        iStart = StringUtil.Find(s, StartKey, iStart) 
        iEnd = StringUtil.Find(s, EndKey, iStart)
    EndWhile 
    
    Return False
EndFunction

;requires skse. Convert int to hex string.
;if result string length is less than minDigits,
;adds 0's to the start for positive numbers, or f's to the start for negative numbers.
;default is 8 (for form IDs)
String function ConvertIntToHex(int i, int minDigits = 8) Global
    String s = ""
     
    If i >= 0
        String HexDigits = "0123456789abcdef"
        while i > 0
            s = StringUtil.GetNthChar(HexDigits , (i % 16)) + s
            i /= 16
        EndWhile 
        
        While StringUtil.GetLength(s) < minDigits 
            s = "0" + s 
        EndWhile
    Else 
        String HexDigits = "fedcba9876543210"
        i = DbMiscFunctions.IntAbs(i) - 1
        while i > 0
            s = StringUtil.GetNthChar(HexDigits , (i % 16)) + s
            i /= 16
        EndWhile 
        
        While StringUtil.GetLength(s) < minDigits 
            s = "F" + s 
        EndWhile
    Endif
    
    return s
EndFunction

;requires skse
;If TreatAsNegative == true, returns hex as negative number. 
;Example: 
;ConvertHexToInt("FD4", true) returns -44 
;ConvertHexToInt("FD4", false) returns 4052 
;Note that if the hex is 8 digits in length (such as form IDs) and starts with "F" it is always treated as negative natively by papyrus. 
Int function ConvertHexToInt(string hex, Bool TreatAsNegative = false) global
    String HexDigits = "0123456789abcdef"
    
    Int iReturn = 0
    Int L = StringUtil.GetLength(hex)
    Int P = L - 1
    Int I = 0 
    
    If TreatAsNegative 
        String Char = StringUtil.GetNthChar(hex, 0) 
        If Char == "F" && L == 8 
            ;do nothing
        Else 
            iReturn = (IntPow(16, P)) * -1
            If Char == "F" 
                I += 1
                P -= 1 
            Endif 
        Endif 
    Endif 
    
    While I < L 
        String Char = Stringutil.GetNthChar(hex, I)
        Int Value = StringUtil.Find(HexDigits, Char)
        
        if Value == -1 
            Value = 0
        Endif
        
        iReturn += (Value * IntPow(16, P))
        
        P -= 1
        I += 1 
    EndWhile
    
    Return iReturn
Endfunction

;For convenience. Returns the akForm ID as a hex string.
;requires SKSE. 
String Function GetFormIDHex(Form akForm) Global
    Return ConvertIntToHex(akForm.GetFormId())
EndFunction

;like Math.Pow, calculates x to the y power, but uses only integers which is more accurate if not needing floats.
;Only works for positive y values.
Int Function IntPow(Int x, Int y) Global
    If Y <= 0 
        Return 1
    Endif 
    
    int xx = x
    While Y > 1
        Y -= 1 
        x *= xx
    EndWhile
    
    return x
EndFunction

;returns floor of the square root of i.
Int Function IntSqrt(Int i) Global 
    if i == 0 || i ==1
        return i 
    Endif 
    
    int iStart = 1 
    int iEnd = i / 2 
    int iReturn
    
    While iStart <= iEnd 
        int iMid = (iStart + iEnd) / 2 
        int sqr = iMid * iMid 
        if (sqr == i) 
            Return iMid
        Endif
        
        if sqr <= i 
            iStart = iMid + 1 
            iReturn = iMid 
        Else 
            iEnd = iMid - 1 
        Endif 
    EndWhile 
    
    return iReturn
EndFunction

;returns absolute value of i.
Int Function IntAbs(Int i) Global 
    If i < 0 
        i *= -1
    Endif 
    
    return i
EndFunction

;rounds the float input and returns int
;5.4 returns 5
;5.5 returns 6
int function RoundAsInt(Float f) Global
    Float fFloor = Math.Floor(f)
    Float Leftover = f - (fFloor) 
    
    If Leftover >= 0.5 
        fFloor += 1.0
    Endif 
    
    Return fFloor as int
    
Endfunction

;rounds the float input and returns float
;5.4 returns 5.0
;5.5 returns 6.0
float function RoundAsFloat(Float f) Global
    Float fFloor = Math.Floor(f)
    Float Leftover = f - (fFloor) 
    
    If Leftover >= 0.5 
        fFloor += 1.0
    Endif 
    
    Return fFloor
Endfunction

;rounds the float down to the specified decimal places
;Example: 
;RoundDownToDec(1.2345, 2) returns 1.23 
;RoundDownToDec(4.5678, 3) return 4.567
;not 100% accurate, limited by string as float conversion. 
;Example: RoundDownToDec(100.78945, 2) returns 100.779999999
Float Function RoundDownToDec(Float f, Int DecimalPlaces = 0) Global 
    If DecimalPlaces == 0 
        return (Math.Floor(F)) as float 
    Endif
    
    String fString = f as string 
    Int p = StringUtil.Find(fString, ".") 
    If p > -1 
        p += (DecimalPlaces + 1)
        return StringUtil.Substring(fString, 0, p) as float 
    Endif 
    
    Return f
Endfunction 

;Same as RoundDownToDec but returns string instead of float. In this case RoundDownToDecString(100.78945, 2) returns "100.78"
String Function RoundDownToDecString(Float f, Int DecimalPlaces = 0) Global 
    If DecimalPlaces == 0 
        return (Math.Floor(F)) as string 
    Endif

    String fString = f as string 
    Int p = StringUtil.Find(fString, ".") 
    If p > -1 
        p += (DecimalPlaces + 1)
        return StringUtil.Substring(fString, 0, p)
    Endif 
    
    Return fString
Endfunction 

;requires skse
Int Function CountDecimalPlaces(Float f) Global
    String fs = f as string 
    int M = StringUtil.GetLength(fs)
    int P = StringUtil.Find(fs, ".")
    
    If P == -1
        return 0
    Else
        String char = "0"
        While M > P && char == "0"
            M -= 1
            char = StringUtil.GetNthChar(fs, M)
        EndWhile 
    EndIf
    
    return  (M - P)
EndFunction
    
;requires SKSE. Splits string by divider and returns as float array.
Float[] Function SplitAsFloat(String s, int Max = -1, String Divider = "||") Global
    String[] StringValues = StringUtil.Split(s, Divider)
    
    If Max == -1 
        Max = StringValues.Length
    Endif 
    
    Float[] Values = Utility.CreateFloatArray(Max)
    
    Int i = 0 
    While i < Max
        Values[i] = StringValues[i] as float 
        i += 1
    EndWhile
    
    return Values
EndFunction

;requires SKSE. Splits string by divider and returns as int array.
Int[] Function SplitAsInt(String s, int Max = -1, String Divider = "||") Global
    String[] StringValues = StringUtil.Split(s, Divider)
    If Max == -1 
        Max = StringValues.Length
    Endif 
    
    int[] Values = Utility.CreateIntArray(Max)
    
    Int i = 0 
    While i < Max
        Values[i] = StringValues[i] as int 
        i += 1
    EndWhile
    
    return Values
EndFunction

;Sort========================================================================================
;if Direct == true, sorts the passed in akArray directly
;if Direct == false, Passed in array is unaffected and returns new array (that is akArray sorted). 
;if Direct == false requires skse to create the new array.
String[] Function SortStringArray(String[] akArray, Bool Ascending = true, Bool Direct = true) Global
    int L = akArray.Length
    String[] NewArray
    
    If Direct == False
        NewArray = CopyStringArray(akArray)
    Else 
        NewArray = akArray 
    Endif
    
    int i = 0 
     
    If Ascending
        While i < L 
            int ii = i + 1 
            While ii < L 
                if NewArray[ii] < NewArray[i] 
                    SwapStrings(NewArray, ii, i)
                Endif 
                ii += 1
            EndWhile
            i += 1
        EndWhile
    Else 
         While i < L 
            int ii = i + 1 
            While ii < L 
                if NewArray[ii] > NewArray[i] 
                    SwapStrings(NewArray, ii, i)
                Endif 
                ii += 1
            EndWhile
            i += 1
        EndWhile 
    EndIf
    
    return NewArray
EndFunction

;requires skse.
String[] Function CopyStringArray(String[] akArray) Global
    Int L = akArray.Length
    String[] NewArray = Utility.CreateStringArray(L) 
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L]
    EndWhile 
    
    Return NewArray
EndFunction

;Opposite of StringUtil.Split. Convert string array to single string, elements separated by divider.
;If IgnoreDuplicates == true, only adds an element to the string once
String Function JoinStringArray(String[] akArray, String Divider = "||", Bool IgnoreDuplicates = false) Global
    String ReturnString 
    Int I = 0 
    Int L = akArray.Length 
    If IgnoreDuplicates == False
        While I < L 
            ReturnString += (akArray[I] + Divider)
            I += 1 
        EndWhile
    Else 
        ReturnString += (akArray[I] + Divider)
        I += 1
        While I < L 
            If akArray.rFind(akArray[I], (I - 1)) == -1 ;this element is not found in any previous elements in array
                ReturnString += (akArray[I] + Divider)
            Endif
            I += 1 
        EndWhile
    Endif
    Return ReturnString
EndFunction

;If IgnoreDuplicates == true, only adds an element to the string once
String Function JoinFloatArray(Float[] akArray, String Divider = "||", Bool IgnoreDuplicates = false) Global
    String ReturnString 
    Int I = 0 
    Int L = akArray.Length 
    If IgnoreDuplicates == False
        While I < L 
            ReturnString += (akArray[I] + Divider)
            I += 1 
        EndWhile
    Else 
        ReturnString += (akArray[I] + Divider)
        I += 1
        While I < L 
            If akArray.rFind(akArray[I], (I - 1)) == -1 ;this element is not found in any previous elements in array
                ReturnString += (akArray[I] + Divider)
            Endif
            I += 1 
        EndWhile
    Endif
    Return ReturnString
EndFunction

;If IgnoreDuplicates == true, only adds an element to the string once
String Function JoinIntArray(Int[] akArray, String Divider = "||", Bool IgnoreDuplicates = false) Global
    String ReturnString 
    Int I = 0 
    Int L = akArray.Length 
    If IgnoreDuplicates == False
        While I < L 
            ReturnString += (akArray[I] + Divider)
            I += 1 
        EndWhile
    Else 
        ReturnString += (akArray[I] + Divider)
        I += 1
        While I < L 
            If akArray.rFind(akArray[I], (I - 1)) == -1 ;this element is not found in any previous elements in array
                ReturnString += (akArray[I] + Divider)
            Endif
            I += 1 
        EndWhile
    Endif
    Return ReturnString
EndFunction

;These print functions are for convenience 
;Allows to trace / messagebox / notification / write to file multiple strings (up to 20) seperated by the divider
;Normally you would have to write, for instance:
;Debug.MessageBox("Player stats: " + PlayerRef.GetAv("OneHanded") + " " + PlayerRef.GetAv("OneHanded") + " " + PlayerRef.GetAv("Marksman")) ;ect...
;With PrintM you can write 
;DbMiscFunctions.PrintM("Player stats:", PlayerRef.GetAv("OneHanded"), PlayerRef.GetAv("OneHanded"), PlayerRef.GetAv("Marksman")) ;ect...

;Trace
;No requirements. 
Function PrintT(String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ", int aiSeverity = 0) Global
    Debug.Trace(JoinStrings(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20, Divider), aiSeverity)
Endfunction

;TraceUser
;No requirements. 
Function PrintTU(string asUserLog = "", String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ", int aiSeverity = 0) Global
    Debug.TraceUser(asUserLog, JoinStrings(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20, Divider), aiSeverity)
Endfunction

;Notification
;No requirements. 
Function PrintN(String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ") Global
    Debug.Notification(JoinStrings(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20, Divider))
Endfunction

;MessageBox
;No requirements. 
Function PrintM(String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ") Global
    Debug.MessageBox(JoinStrings(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20, Divider))
Endfunction

;ExtendedVanillaMenus.MessageBox
;requires ExtendedVanillaMenus and skse
Function PrintEvm(String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ") Global
    ExtendedVanillaMenus.MessageBox(JoinStrings(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20, Divider))
Endfunction

;WriteToFile
;requires PapyrusUtil and skse
Function PrintF(String FilePath, String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ") Global
    MiscUtil.WriteToFile(FilePath, JoinStrings(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20, Divider))
Endfunction

;Join up to 20 strings into a single string seperated by the Divider
;Stops joining at the first empty "" string it finds. 
;no requirements
String Function JoinStrings(String s1 = "", String s2 = "", String s3 = "", String s4 = "", String s5 = "", String s6 = "", String s7 = "", String s8 = "", String s9 = "", String s10 = "", \
String s11 = "", String s12 = "", String s13 = "", String s14 = "", String s15 = "", String s16 = "", String s17 = "", String s18 = "", String s19 = "", String s20 = "", String Divider = " ") Global

    String s = ""
    
    If s1 == "" 
        return s
    Endif
    
    s += s1 
    
    If s2 == "" 
        return s
    Endif
    
    s += (Divider + s2) 
    
    If s3 == "" 
        return s
    Endif
    
    s += (Divider + s3) 
    
    If s4 == "" 
        return s
    Endif
    
    s += (Divider + s4) 
    
    If s5 == "" 
        return s
    Endif
    
    s += (Divider + s5) 
    
    If s6 == "" 
        return s
    Endif
    
    s += (Divider + s6) 
    
    If s7 == "" 
        return s
    Endif
    
    s += (Divider + s7) 
    
    If s8 == "" 
        return s
    Endif
    
    s += (Divider + s8) 
    
    If s9 == "" 
        return s
    Endif
    
    s += (Divider + s9) 
    
    If s10 == "" 
        return s
    Endif
    
    s += (Divider + s10) 
    
    If s11 == "" 
        return s
    Endif
    
    s += (Divider + s11)
    
    If s12 == "" 
        return s
    Endif
    
    s += (Divider + s12) 
    
    If s13 == "" 
        return s
    Endif
    
    s += (Divider + s13) 
    
    If s14 == "" 
        return s
    Endif
    
    s += (Divider + s14) 
    
    If s15 == "" 
        return s
    Endif
    
    s += (Divider + s15) 
    
    If s16 == "" 
        return s
    Endif
    
    s += (Divider + s16) 
    
    If s17 == "" 
        return s
    Endif
    
    s += (Divider + s17) 
    
    If s18 == "" 
        return s
    Endif
    
    s += (Divider + s18) 
    
    If s19 == "" 
        return s
    Endif
    
    s += (Divider + s19) 
    
    If s20 == "" 
        return s
    Endif
    
    s += (Divider + s20) 
    
    return s
EndFunction

;Get game settings for soul levels
;no requirements
Int[] Function GetGameActorSoulLevels() Global
    Int[] SoulLevels = New Int[4]
    SoulLevels[0] = Game.GetGameSettingInt("iLesserSoulActorLevel")
    SoulLevels[1] = Game.GetGameSettingInt("iCommonSoulActorLevel")
    SoulLevels[2] = Game.GetGameSettingInt("iGreaterSoulActorLevel")
    SoulLevels[3] = Game.GetGameSettingInt("iGrandSoulActorLevel")
    Return SoulLevels
EndFunction

;Get game setting soul level names
;no requirements
String[] Function GetGameSoulLevelNames() Global
    String[] SoulSizes = new String[5]
    SoulSizes[0] = Game.GetGameSettingString("sSoulLevelNamePetty")
    SoulSizes[1] = Game.GetGameSettingString("sSoulLevelNameLesser")
    SoulSizes[2] = Game.GetGameSettingString("sSoulLevelNameCommon")
    SoulSizes[3] = Game.GetGameSettingString("sSoulLevelNameGreater")
    SoulSizes[4] = Game.GetGameSettingString("sSoulLevelNameGrand")
    
    Return SoulSizes
EndFunction

;Get the actor soul size. 0 = petty, 1 = lesser, 2 = Common, 3 = Greater, 4 = Grand, 5 = Black (for NPCs)
;No requirements
Int Function GetActorSoulSize(Actor akActor) Global 
    If IsActorNPC(akActor) 
        return 5
    Endif 
    
    Int Level = akActor.GetLevel()
    Int[] SoulLevels = GetGameActorSoulLevels()
    If Level < SoulLevels[0]
        return 0
        
    Elseif Level < SoulLevels[1]
        return 1 
        
    Elseif Level < SoulLevels[2]
        return 2 
        
    Elseif Level < SoulLevels[3]
        return 3
    Else
        return 4
    Endif
EndFunction 

;Get actor soul size as string.
;no requirements
String Function GetActorSoulSizeString(Actor akActor, String sBlackSize = "Black") Global
    If IsActorNPC(akActor) 
        return sBlackSize
    Endif 
    
    String[] akSoulSizes = GetGameSoulLevelNames()
    Return akSoulSizes[GetActorSoulSize(akActor)]
EndFunction 

;Is the akActor an NPC? 
;no requirements
Bool Function IsActorNPC(Actor akActor) Global
    KeyWord ActorTypeNPC = Game.GetForm(0x13794) as KeyWord 
    If ActorTypeNPC 
        Race akRace = akActor.GetRace() 
        Return (akRace.HasKeyword(ActorTypeNPC))
    Endif
    Return False
EndFunction

;Returns true if akActor is moving. 
;I've only tested on NPC humanoid actors. Not sure if it works for other types.
;No requirements
Bool Function IsActorMoving(Actor akActor) Global 
    Float Speed = akActor.GetAnimationVariableFloat("Speed")
    Float TurnDelta = akActor.GetAnimationVariableFloat("TurnDelta")
    return (Speed > 0 || TurnDelta != 0)
EndFunction



;Get random form from Ref's inventory.  
;If TypeArrayFilter != none, filters for form types in TypeArrayFilter. If TypeFilterHasType == true (default) only allows for types in the TypeArrayFilter. If false, only allows for types NOT in the TypeArrayFilter.
;If ListFilter != none, filters for base forms in the ListFilter formlist. If akListHasForm == true (default) only allows for forms in the ListFilter formlist. If false, only allows for forms NOT in the formlist. 
;Requires SKSE
Form Function GetRandomFormFromRef(ObjectReference Ref, Int[] TypeArrayFilter = None, Formlist ListFilter = None, Bool TypeFilterHasType = true, Bool akListHasForm = true) Global
    Form[] Forms = Ref.GetContainerForms()
    Int L = Forms.length 
    If L == 0 
        return none 
    Endif 
    
    Int r = Utility.RandomInt(0, L)
    Int i = r
    Form akForm
    
    If TypeArrayFilter.length > 0 && ListFilter
        While i < L && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                If ListFilter.HasForm(Forms[i]) == akListHasForm
                    akForm = Forms[i] 
                Endif 
            Endif 
            i += 1 
        EndWhile  

        i = 0

        While i < r && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                If ListFilter.HasForm(Forms[i]) == akListHasForm
                    akForm = Forms[i] 
                Endif 
            Endif 
            i += 1 
        EndWhile
        
    Elseif ListFilter
        While i < L && akForm == None
            If ListFilter.HasForm(Forms[i]) == akListHasForm
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile  

        i = 0

        While i < r && akForm == None
            If ListFilter.HasForm(Forms[i]) == akListHasForm
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile
        
    Elseif TypeArrayFilter.length > 0 
        While i < L && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile  

        i = 0

        While i < r && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile
    Else 
        akForm = Forms[r]
    Endif 
    
    Return akForm
EndFunction

;Get random form from Ref's inventory.  
;If TypeArrayFilter != none, filters for form types in TypeArrayFilter. If TypeFilterHasType == true (default) only allows for types in the TypeArrayFilter. If false, only allows for types NOT in the TypeArrayFilter.
;If ListFilter != none, filters for base forms in the ListFilter form array. If akListHasForm == true (default) only allows for forms in the ListFilter form array. If false, only allows for forms NOT in the form array. 
;Requires SKSE
Form Function GetRandomFormFromRefA(ObjectReference Ref, Int[] TypeArrayFilter = None, Form[] ListFilter = None, Bool TypeFilterHasType = true, Bool akListHasForm = true) Global
    Form[] Forms = Ref.GetContainerForms()
    Int L = Forms.length 
    If L == 0 
        return none 
    Endif 
    
    Int r = Utility.RandomInt(0, L)
    Int i = r
    Form akForm
    
    If TypeArrayFilter.length > 0 && ListFilter
        While i < L && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                If ListFilter.Find(Forms[i]) == akListHasForm
                    akForm = Forms[i] 
                Endif 
            Endif 
            i += 1 
        EndWhile  

        i = 0

        While i < r && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                If ListFilter.Find(Forms[i]) == akListHasForm
                    akForm = Forms[i] 
                Endif 
            Endif 
            i += 1 
        EndWhile
        
    Elseif ListFilter
        While i < L && akForm == None
            If ListFilter.Find(Forms[i]) == akListHasForm
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile  

        i = 0

        While i < r && akForm == None
            If ListFilter.Find(Forms[i]) == akListHasForm
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile
        
    Elseif TypeArrayFilter.length > 0 
        While i < L && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile  

        i = 0

        While i < r && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile
    Else 
        akForm = Forms[r]
    Endif 
    
    Return akForm
EndFunction 

;Get random form from Ref's inventory.  
;If TypeArrayFilter != none, filters for form types in TypeArrayFilter. If TypeFilterHasType == true (default) only allows for types in the TypeArrayFilter. If false, only allows for types NOT in the TypeArrayFilter.
;If ListKeyName != none, filters for base forms in the StorageUtil Formlist defined by the ObjKey and ListKeyName. If akListHasForm == true (default) only allows for forms in the StorageUtil Formlist. If false, only allows for forms NOT in the StorageUtil Formlist. 
;Requires SKSE and PapyrusUtil
Form Function GetRandomFormFromRefS(ObjectReference Ref, Int[] TypeArrayFilter = None, Form ObjKey = None, String ListKeyName = "", Bool TypeFilterHasType = true, Bool akListHasForm = true) Global
    Form[] Forms = Ref.GetContainerForms()
    Int L = Forms.length 
    If L == 0 
        return none 
    Endif 
    
    Int r = Utility.RandomInt(0, L)
    Int i = r
    Form akForm
    
    If TypeArrayFilter.length > 0 && ListKeyName != ""
        While i < L && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                If StorageUtil.FormListHas(ObjKey, ListKeyName, Forms[i]) == akListHasForm
                    akForm = Forms[i] 
                Endif 
            Endif 
            i += 1 
        EndWhile  

        i = 0

        While i < r && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                If StorageUtil.FormListHas(ObjKey, ListKeyName, Forms[i]) == akListHasForm
                    akForm = Forms[i] 
                Endif 
            Endif 
            i += 1 
        EndWhile
        
    Elseif ListKeyName != ""
        While i < L && akForm == None
            If StorageUtil.FormListHas(ObjKey, ListKeyName, Forms[i]) == akListHasForm
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile  

        i = 0

        While i < r && akForm == None
            If StorageUtil.FormListHas(ObjKey, ListKeyName, Forms[i]) == akListHasForm
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile
        
    Elseif TypeArrayFilter.length > 0 
        While i < L && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile  

        i = 0

        While i < r && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile
    Else 
        akForm = Forms[r]
    Endif 
    
    Return akForm
EndFunction

;Get random form from Ref's inventory.  
;If TypeArrayFilter != none, filters for form types in TypeArrayFilter. If TypeFilterHasType == true (default) only allows for types in the TypeArrayFilter. If false, only allows for types NOT in the TypeArrayFilter.
;If JsonFilePath != none && ListKeyName != none, filters for base forms in the JsonUtil Formlist defined by the JsonFilePath and ListKeyName. If akListHasForm == true (default) only allows for forms in the JsonUtil Formlist. If false, only allows for forms NOT in the JsonUtil Formlist. 
;Requires SKSE and PapyrusUtil.
Form Function GetRandomFormFromRefJ(ObjectReference Ref, Int[] TypeArrayFilter = None, String JsonFilePath = "", String ListKeyName = "", Bool TypeFilterHasType = true, Bool akListHasForm = true) Global
    Form[] Forms = Ref.GetContainerForms()
    Int L = Forms.length 
    If L == 0 
        return none 
    Endif 
    
    Int r = Utility.RandomInt(0, L)
    Int i = r
    Form akForm
    
    If TypeArrayFilter.length > 0 && ListKeyName != "" && JsonUtil.JsonExists(JsonFilePath)
        While i < L && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                If JsonUtil.FormListHas(JsonFilePath, ListKeyName, Forms[i]) == akListHasForm
                    akForm = Forms[i] 
                Endif 
            Endif 
            i += 1 
        EndWhile  

        i = 0

        While i < r && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                If JsonUtil.FormListHas(JsonFilePath, ListKeyName, Forms[i]) == akListHasForm
                    akForm = Forms[i] 
                Endif 
            Endif 
            i += 1 
        EndWhile
        
    Elseif ListKeyName != "" && JsonUtil.JsonExists(JsonFilePath)
        While i < L && akForm == None
            If JsonUtil.FormListHas(JsonFilePath, ListKeyName, Forms[i]) == akListHasForm
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile  

        i = 0

        While i < r && akForm == None
            If JsonUtil.FormListHas(JsonFilePath, ListKeyName, Forms[i]) == akListHasForm
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile
        
    Elseif TypeArrayFilter.length > 0 
        While i < L && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile  

        i = 0

        While i < r && akForm == None
            If TypeArrayFilter.Find(Forms[i].GetType()) == TypeFilterHasType
                akForm = Forms[i] 
            Endif 
            i += 1 
        EndWhile
    Else 
        akForm = Forms[r]
    Endif 
    
    Return akForm
EndFunction

;requires skse. Sort Actors in akArray by their display name.
Function SortActorArrayByName(Actor[] akArray, Bool Ascending = true) Global
    int L = akArray.Length
    int i = 0 
    
    If Ascending
        While i < L 
            int ii = i + 1 
            While ii < L 
                if akArray[ii].GetDisplayName() < akArray[i].GetDisplayName() 
                    SwapActors(akArray, ii, i)
                Endif 
                ii += 1
            EndWhile
            i += 1
        EndWhile
    Else 
         While i < L 
            int ii = i + 1 
            While ii < L 
                if akArray[ii].GetDisplayName() > akArray[i].GetDisplayName() 
                    SwapActors(akArray, ii, i)
                Endif 
                ii += 1
            EndWhile
            i += 1
        EndWhile 
    EndIf
EndFunction

;requires skse. Sort ObjectReferences in akArray by their display name.
Function SortObjectRefArrayByName(ObjectReference[] akArray, Bool Ascending = true) Global
    int L = akArray.Length
    int i = 0 
    
    If Ascending
        While i < L 
            int ii = i + 1 
            While ii < L 
                if akArray[ii].GetDisplayName() < akArray[i].GetDisplayName() 
                    SwapObjectReferences(akArray, ii, i)
                Endif 
                ii += 1
            EndWhile
            i += 1
        EndWhile
    Else 
         While i < L 
            int ii = i + 1 
            While ii < L 
                if akArray[ii].GetDisplayName() > akArray[i].GetDisplayName() 
                    SwapObjectReferences(akArray, ii, i)
                Endif 
                ii += 1
            EndWhile
            i += 1
        EndWhile 
    EndIf
EndFunction 

;requires skse. Sort Forms in akArray by their name.
Function SortFormArrayByName(Form[] akArray, Bool Ascending = true) Global
    int L = akArray.Length
    int i = 0 
    
    If Ascending
        While i < L 
            int ii = i + 1 
            While ii < L 
                if akArray[ii].GetName() < akArray[i].GetName() 
                    SwapForms(akArray, ii, i)
                Endif 
                ii += 1
            EndWhile
            i += 1
        EndWhile
    Else 
         While i < L 
            int ii = i + 1 
            While ii < L 
                if akArray[ii].GetName() > akArray[i].GetName() 
                    SwapForms(akArray, ii, i)
                Endif 
                ii += 1
            EndWhile
            i += 1
        EndWhile 
    EndIf
EndFunction 

;requires skse, put all actor names of actors in akArray to a string array and return.
String[] Function GetActorNames(Actor[] akArray) Global
    Int L = akArray.Length 
    String[] sArray = Utility.CreateStringArray(L) 
    While L > 0 
        L -= 1 
        sArray[L] = akArray[L].GetDisplayName() 
    EndWhile
    
    return sArray
EndFunction 

;requires skse, put all ObjectReference names of ObjectReferences in akArray to a string array and return.
String[] Function GetObjectRefNames(ObjectReference[] akArray) Global
    Int L = akArray.Length 
    String[] sArray = Utility.CreateStringArray(L) 
    While L > 0 
        L -= 1 
        sArray[L] = akArray[L].GetDisplayName() 
    EndWhile
    return sArray
EndFunction 

;requires skse, put all Form names of Forms in akArray to a string array and return.
String[] Function GetFormNames(Form[] akArray) Global
    Int L = akArray.Length 
    String[] sArray = Utility.CreateStringArray(L) 
    While L > 0 
        L -= 1 
        sArray[L] = akArray[L].GetName() 
    EndWhile
    return sArray
EndFunction 

;requires skse, put all Form names of Forms in akList to a string array and return.
String[] Function GetFormNamesFromList(Formlist akList) Global
    Int L = akList.GetSize() 
    String[] sArray = Utility.CreateStringArray(L) 
    While L > 0 
        L -= 1 
        sArray[L] = GetFormName(akList.GetAt(L))
    EndWhile
    return sArray
EndFunction 

;requires skse, put all Form names of Forms in akList to a single string seperated by divider. Default new line.
String Function FormNamesInFormListToString(Formlist akList, string divider = "\n", string nullName = "null") Global
    Int L = akList.GetSize() 
	if L == 0
		return ""
	endif
	
	int i = 1
    String result = GetFormName(akList.GetAt(0), nullName)
	
	while i < L
        result += divider + GetFormName(akList.GetAt(i), nullName)
		i += 1
    EndWhile
	
    return result
EndFunction 

;Requires skse. Add all forms in akList to new form array.
Form[] Function FormlistToArray(Formlist akList) Global 
    int L = akList.GetSize()
    Form[] akArray = Utility.CreateFormArray(L) 
    
    While L > 0 
        L -= 1 
        akArray[L] = akList.GetAt(L) 
    EndWhile  
    
    Return akArray
EndFunction

;Add all forms in akArray to akList
Function AddFormArrayFormsToList(Form[] akArray, Formlist akList) Global
    int L = akArray.Length
    Int i = 0 
    While i < L 
        akList.AddForm(akArray[i] )
        i += 1 
    EndWhile
EndFunction 

;requires skse, register for all keys between min and max. Default is 1 to 281, or all keys.
Function RegisterFormForKeys(Form akForm, Int min = 1, Int Max = 281) Global
    If akForm == none 
        return 
    Endif 
    
    int i = min 
    While i <= Max 
        akForm.RegisterForKey(i)
        i += 1 
    EndWhile
EndFunction 

Function RegisterAliasForKeys(Alias akAlias, Int min = 1, Int Max = 281) Global
    If akAlias == none 
        return 
    Endif 
    
    int i = min 
    While i <= Max 
        akAlias.RegisterForKey(i)
        i += 1 
    EndWhile
EndFunction 

Function RegisterActiveMagicEffectForKeys(ActiveMagicEffect akActiveMagicEffect, Int min = 1, Int Max = 281) Global
    If akActiveMagicEffect == none 
        return 
    Endif 
    
    int i = min 
    While i <= Max 
        akActiveMagicEffect.RegisterForKey(i)
        i += 1 
    EndWhile
    
EndFunction     

;Swap============================================================================== 
;Swap the element at IndexA with the element at IndexB in the akArray 
;The V functions (V for validate) will first clamp the indexes between 0 and the last available index in the akArray first before swapping.
;If you know for sure that the indexes are in bounds, use Swap as it's faster. If you don't know for sure, use SwapV or you might get none entries.
Function SwapStrings(String[] akArray, Int IndexA, Int IndexB) Global
    String Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapStringsV(String[] akArray, Int IndexA, Int IndexB) Global ;validate indexes first.
    Int L = akArray.Length - 1
    IndexA = ClampInt(IndexA, 0, L) 
    IndexB = ClampInt(IndexB, 0, L) 
    
    String Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapBools(Bool[] akArray, Int IndexA, Int IndexB) Global
    Bool Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapBoolsV(Bool[] akArray, Int IndexA, Int IndexB) Global ;validate indexes first.
    Int L = akArray.Length - 1
    IndexA = ClampInt(IndexA, 0, L) 
    IndexB = ClampInt(IndexB, 0, L) 
    
    Bool Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapInts(Int[] akArray, Int IndexA, Int IndexB) Global
    Int Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapIntsV(Int[] akArray, Int IndexA, Int IndexB) Global ;validate indexes first.
    Int L = akArray.Length - 1
    IndexA = ClampInt(IndexA, 0, L) 
    IndexB = ClampInt(IndexB, 0, L) 
    
    Int Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapFloats(Float[] akArray, Int IndexA, Int IndexB) Global
    Float Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapFloatsV(Float[] akArray, Int IndexA, Int IndexB) Global ;validate indexes first.
    Int L = akArray.Length - 1
    IndexA = ClampInt(IndexA, 0, L) 
    IndexB = ClampInt(IndexB, 0, L) 
    
    Float Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapActors(Actor[] akArray, Int IndexA, Int IndexB) Global
    Actor Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapActorsV(Actor[] akArray, Int IndexA, Int IndexB) Global ;validate indexes first.
    Int L = akArray.Length - 1
    IndexA = ClampInt(IndexA, 0, L) 
    IndexB = ClampInt(IndexB, 0, L) 
    
    Actor Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapObjectReferences(ObjectReference[] akArray, Int IndexA, Int IndexB) Global
    ObjectReference Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapObjectReferencesV(ObjectReference[] akArray, Int IndexA, Int IndexB) Global ;validate indexes first.
    Int L = akArray.Length - 1
    IndexA = ClampInt(IndexA, 0, L) 
    IndexB = ClampInt(IndexB, 0, L) 
    
    ObjectReference Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapForms(Form[] akArray, Int IndexA, Int IndexB) Global
    Form Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction

Function SwapFormsV(Form[] akArray, Int IndexA, Int IndexB) Global ;validate indexes first.
    Int L = akArray.Length - 1
    IndexA = ClampInt(IndexA, 0, L) 
    IndexB = ClampInt(IndexB, 0, L) 
    
    Form Value = akArray[IndexA]
    akArray[IndexA] = akArray[IndexB]
    akArray[IndexB] = Value
EndFunction 
;============================================================================================ 

;================================================================================================
;JsonUtil functions, included in StorageUtil but missing from JsonUtil. Requires PapyrusUtil
;the default value is returned if unable to remove element from the json list.

;/ Plucks a value from list by index
   The index is removed from the list's storage after returning it's value.

   FileName: file to pluck value from.
   KeyName: name of list.
   index: index of value in the list.
   [optional] default: if unable to remove element from the json list, return this value instead.
/;

int function JsonIntListPluck(string FileName, string KeyName, int index, int default = 0) global 
    int a = JsonUtil.IntListGet(FileName, KeyName, index)
    If JsonUtil.IntListRemoveAt(FileName, KeyName, index) 
        return a
    Else 
        return default 
    Endif
EndFunction 

Float function JsonFloatListPluck(string FileName, string KeyName, int index, Float default = 0.0) global 
    Float a = JsonUtil.FloatListGet(FileName, KeyName, index)
    If JsonUtil.FloatListRemoveAt(FileName, KeyName, index) 
        return a 
    Else 
        return default 
    Endif
EndFunction 

string function JsonStringListPluck(string FileName, string KeyName, int index, string default = "") global 
    String a = JsonUtil.StringListGet(FileName, KeyName, index)
    If JsonUtil.StringListRemoveAt(FileName, KeyName, index) 
        return a
    Else 
        return default 
    Endif
EndFunction 

Form function JsonFormListPluck(String FileName, String KeyName, int index, Form default = none) global 
    Form a = JsonUtil.FormListGet(FileName, KeyName, index)
    If JsonUtil.FormListRemoveAt(FileName, KeyName, index) 
        return a
    Else 
        return default 
    Endif
EndFunction 

;/ Gets the value of the very first element in a list, and subsequently removes the index afterward.

   FileName: file to shift value from.
   KeyName: name of list.
   [optional] default: if unable to remove element from the json list, return this value instead.
/;

int function JsonintListShift(string FileName, string KeyName, int default = 0) global 
    return JsonIntListPluck(FileName, KeyName, 0, default)
EndFunction 

Float function JsonFloatListShift(string FileName, string KeyName, Float default = 0.0) global 
    return JsonfloatListPluck(FileName, KeyName, 0, default)
EndFunction 

String function JsonStringListShift(string FileName, string KeyName, String default = "") global 
    return JsonstringListPluck(FileName, KeyName, 0, default)
EndFunction 

Form function JsonFormListShift(string FileName, string KeyName, Form default = none) global 
    return JsonFormListPluck(FileName, KeyName, 0, default)
EndFunction 

;/ Gets the value of the very last element in a list, and subsequently removes the index afterward.

   FileName: file to pop value from.
   KeyName: name of list.
   [optional] default: if unable to remove element from the json list, return this value instead.
/;

Int function JsonIntListPop(string FileName, string KeyName, Int default = 0) global 
    int L = JsonUtil.IntListCount(FileName, KeyName)
    If L > 0
        return JsonIntListPluck(FileName, KeyName, (L - 1), default)
    Else 
        return default 
    endif
EndFunction  

Float function JsonFloatListPop(string FileName, string KeyName, Float default = 0.0) global 
    int L = JsonUtil.FloatListCount(FileName, KeyName)
    If L > 0
        return JsonFloatListPluck(FileName, KeyName, (L - 1), default)
    Else 
        return default 
    endif
EndFunction  

String function JsonStringListPop(string FileName, string KeyName, String default = "") global 
    int L = JsonUtil.StringListCount(FileName, KeyName)
    If L > 0
        return JsonStringListPluck(FileName, KeyName, (L - 1), default)
    Else 
        return default 
    endif
EndFunction  

Form function JsonFormListPop(string FileName, string KeyName, Form default = none) global 
    int L = JsonUtil.FormListCount(FileName, KeyName)
    If L > 0
        return JsonFormListPluck(FileName, KeyName, (L - 1), default)
    Else 
        return default 
    endif
EndFunction  

;remove all duplicates in Json int / float / string / form lists, leaving only 1 of each element in the list.
;If asceding == true (default) removes duplicate entries from the start of the list first, 
;Else removes duplicates from end of list first. ======================================================================================
Function JsonIntListRemoveAllDuplicates(string FileName, string KeyName, Bool Acending = True) Global
    Int L = JsonUtil.IntListCount(FileName, KeyName) 
    int i = 0 
    
    If Acending
        While i < L
            int value = JsonUtil.IntListGet(FileName, keyName, i)
            If JsonUtil.IntListCountValue(FileName, keyName, Value) > 1 
                JsonUtil.IntListRemoveAt(FileName, keyName, i) 
                L -= 1
            Else 
                i += 1
            Endif
        EndWhile
    Else 
        i = L
        While i > 0
            i -= 1
            int value = JsonUtil.IntListGet(FileName, keyName, i)
            If JsonUtil.IntListCountValue(FileName, keyName, Value) > 1 
                JsonUtil.IntListRemoveAt(FileName, keyName, i) 
            Endif
        EndWhile 
    Endif
EndFunction

Function JsonFloatListRemoveAllDuplicates(String FileName, String KeyName, Bool Acending = True) Global
    Int L = JsonUtil.FloatListCount(FileName, KeyName) 
    int i = 0 
    
    If Acending
        While i < L
            Float value = JsonUtil.FloatListGet(FileName, keyName, i)
            If JsonUtil.FloatListCountValue(FileName, keyName, Value) > 1 
                JsonUtil.FloatListRemoveAt(FileName, keyName, i) 
                L -= 1
            Else 
                i += 1
            Endif
        EndWhile
    Else 
        i = L
        While i > 0
            i -= 1
            Float value = JsonUtil.FloatListGet(FileName, keyName, i)
            If JsonUtil.FloatListCountValue(FileName, keyName, Value) > 1 
                JsonUtil.FloatListRemoveAt(FileName, keyName, i) 
            Endif
        EndWhile 
    Endif
EndFunction

Function JsonStringListRemoveAllDuplicates(string FileName, string KeyName, Bool Acending = True) Global
    Int L = JsonUtil.StringListCount(FileName, KeyName) 
    int i = 0 
    
    If Acending
        While i < L
            String value = JsonUtil.StringListGet(FileName, keyName, i)
            If JsonUtil.StringListCountValue(FileName, keyName, Value) > 1 
                JsonUtil.StringListRemoveAt(FileName, keyName, i) 
                L -= 1
            Else 
                i += 1
            Endif
        EndWhile
    Else 
        i = L
        While i > 0
            i -= 1
            String value = JsonUtil.StringListGet(FileName, keyName, i)
            If JsonUtil.StringListCountValue(FileName, keyName, Value) > 1 
                JsonUtil.StringListRemoveAt(FileName, keyName, i) 
            Endif
        EndWhile 
    Endif
EndFunction

Function JsonFormListRemoveAllDuplicates(String FileName, String KeyName, Bool Acending = True) Global
    Int L = JsonUtil.FormListCount(FileName, KeyName) 
    int i = 0 
    
    If Acending
        While i < L
            Form value = JsonUtil.FormListGet(FileName, keyName, i)
            If JsonUtil.FormListCountValue(FileName, keyName, Value) > 1 
                JsonUtil.FormListRemoveAt(FileName, keyName, i) 
                L -= 1
            Else 
                i += 1
            Endif
        EndWhile
    Else 
        i = L
        While i > 0
            i -= 1
            Form value = JsonUtil.FormListGet(FileName, keyName, i)
            If JsonUtil.FormListCountValue(FileName, keyName, Value) > 1 
                JsonUtil.FormListRemoveAt(FileName, keyName, i) 
            Endif
        EndWhile 
    Endif
EndFunction

;Opposite of String.Split() 
;Join all elements in Json Int List to a single string seperated by divider. =================================================================================
String Function JsonJoinIntList(string FileName, string KeyName, string Divider = "||") Global
    String s
    Int L = JsonUtil.IntListCount(FileName, KeyName) - 1
    int i = 0 
    While i < L
        s += JsonUtil.IntListGet(FileName, keyName, i) + Divider 
        i += 1
    EndWhile
    s += JsonUtil.IntListGet(FileName, keyName, i)
    
    return s
EndFunction

String Function JsonJoinFloatList(string FileName, string KeyName, string Divider = "||") Global
    String s
    Int L = JsonUtil.FloatListCount(FileName, KeyName) - 1
    int i = 0 
    While i < L
        s += JsonUtil.FloatListGet(FileName, keyName, i) + Divider 
        i += 1
    EndWhile
    s += JsonUtil.FloatListGet(FileName, keyName, i)
    
    return s
EndFunction

String Function JsonJoinStringList(string FileName, string KeyName, string Divider = "||") Global
    String s
    Int L = JsonUtil.StringListCount(FileName, KeyName) - 1
    int i = 0 
    While i < L
        s += JsonUtil.StringListGet(FileName, keyName, i) + Divider 
        i += 1
    EndWhile
    s += JsonUtil.StringListGet(FileName, keyName, i)
    
    return s
EndFunction

;============================================================================================================================================================

;============================================================================================================

;requires skse and papyrusUtil. Get the string for a form type int. 
;Exampe: FormTypeToString(SomeMiscObj.GetType()) returns "Misc"
;can specify another file other than "Data/interface/DbFormTypeStrings.txt" if desired
String Function GetFormTypeString(Int Type, String sFilePath = "Data/interface/DbMiscFunctions/DbFormTypeStrings.txt") Global
    Type -= 1
    Return GetStringFromFile(Type, FilePath = sFilePath)
EndFunction

;requires skse and papyrusUtil. Get the string for keycode. 
;Exampe: GetKeyCodeString(28) returns "Enter"
;can specify another file other than "Data/interface/DbKeyCodeStrings.txt" if desired
String Function GetKeyCodeString(Int keyCode, String sFilePath = "Data/interface/DbMiscFunctions/DbKeyCodeStrings.txt") Global
    keyCode -= 1
    Return GetStringFromFile(keyCode, FilePath = sFilePath)
EndFunction

;Return mod name string that the akForm comes from. e.g "Skyrim.esm"
;Requires skse
String Function GetModOriginName(Form akForm) Global
    String FormID = ConvertIntToHex(akForm.GetFormId())
    
    If StringUtil.SubString(FormID, 0, 2) == "FE" 
        Int ModIndex = ConvertHexToInt(StringUtil.SubString(FormID, 2, 3)) ;FEXXXYYY, for light plugins. XXX is Load Order
        Return Game.GetLightModName(ModIndex)
    Else 
        Int ModIndex =  ConvertHexToInt(StringUtil.SubString(FormID, 0, 2)) ;XXYYYYYY, for normal plugins, XX is load Order
        Return Game.GetModName(ModIndex)
    Endif 
EndFunction

;get common form types without SKSE =======================================================================
;These functions mimic the main Categories in the creation kit.
;GetActorFormType corresponds to the Actor category. GetAudioFormType corresponds to the Audo category ect.
;Note, can pass in ObjectReference's and it will auto get baseObject and return type.
int Function GetActorFormType(Form F) Global
    Form akForm
    If F as Actor 
        return 0 
    Elseif F As ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif 
    
    If akForm as ActorBase 
        return 1
    ElseIf akForm as Action
        return 2
    Elseif akForm as LeveledActor 
        return 3
    Elseif akForm as Perk 
        return 4
    Elseif akForm as TalkingActivator 
        return 5
    Endif
    
    return -1
EndFunction

int Function GetAudioFormType(Form F) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If akForm as MusicType 
        return 0
    ElseIf akForm as SoundCategory 
        return 1
    ElseIf akForm as SoundDescriptor
        return 2
    Elseif akForm as Sound 
        return 3
    Endif
    
    return -1
EndFunction 

int Function GetCharacterFormType(Form F) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If akForm as AssociationType 
        return 0
    ElseIf akForm as Class 
        return 1
    ElseIf akForm as EquipSlot
        return 2
    Elseif akForm as Faction 
        return 3
    Elseif akForm as Headpart ;require skse?
        return 4
    Elseif akForm as Package 
        return 5
    Elseif akForm as Quest
        return 6
    Elseif akForm as Race 
        return 7
    Elseif akForm as VoiceType 
        return 8
    Endif
    
    return -1
EndFunction

int Function GetItemFormType(Form F) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If akForm as Ammo 
        return 0
    ElseIf akForm as Armor 
        return 1
    ElseIf akForm as ArmorAddon
        return 2
    Elseif akForm as Book 
        return 3
    Elseif akForm as ConstructibleObject 
        return 4
    Elseif akForm as Ingredient 
        return 5
    Elseif akForm as key
        return 6
    Elseif akForm as LeveledItem 
        return 7
    Elseif akForm as MiscObject 
        return 8
    Elseif akForm as Outfit 
        return 9
    Elseif akForm as SoulGem 
        return 10
    Elseif akForm as Weapon 
        return 11
    Endif
    
    return -1
EndFunction

int Function GetMagicFormType(Form F) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If akForm as Enchantment 
        return 0
    ElseIf akForm as LeveledSpell 
        return 1
    ElseIf akForm as MagicEffect
        return 2
    Elseif akForm as Potion 
        return 3
    Elseif akForm as Scroll 
        return 4
    Elseif akForm as Shout 
        return 5
    Elseif akForm as Spell
        return 6
    Elseif akForm as WordOfPower 
        return 7
    Endif
    return -1
EndFunction

int Function GetMiscFormType(Form F) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If akForm as Art 
        return 0
    ElseIf akForm as ColorForm 
        return 1
    ElseIf akForm as CombatStyle
        return 2
    Elseif akForm as Formlist 
        return 3
    Elseif akForm as GlobalVariable 
        return 4
    Elseif akForm as Idle 
        return 5
    Elseif akForm as Keyword
        return 6
    Elseif akForm as Message 
        return 7
    Elseif akForm as TextureSet 
        return 8
    Endif
    
    return -1
EndFunction

int Function GetSpecialEffectFormType(Form F) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If akForm as Explosion 
        return 0 
    ElseIf akForm as Hazard 
        return 1
    ElseIf akForm as ImageSpaceModifier 
        return 2
    ElseIf akForm as ImpactDataSet 
        return 3
    ElseIf akForm as Projectile 
        return 4
    Endif
    
    return -1
EndFunction 

int Function GetWorldDataFormType(Form F) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If akForm as EncounterZone 
        return 0 
    ElseIf akForm as Location 
        return 1
    ElseIf akForm as LocationRefType 
        return 2
    ElseIf akForm as ShaderParticleGeometry 
        return 3
    ElseIf akForm as VisualEffect 
        return 4
    ElseIf akForm as Weather 
        return 5
    Endif
    
    return -1
EndFunction 

int Function GetWorldObjectFormType(Form F) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If akForm as Activator 
        return 0 
    ElseIf akForm as Container 
        return 1
    ElseIf akForm as Door 
        return 2
    ElseIf akForm as Flora 
        return 3
    ElseIf akForm as Furniture 
        return 4
    ElseIf akForm as Light 
        return 5
    ElseIf akForm as Static 
        return 6
    Endif
    
    return -1
EndFunction 

;for base item types in player inventory
int Function GetInventoryItemFormType(Form F) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If akForm as Ammo 
        return 0
    ElseIf akForm as Armor 
        return 1
    Elseif akForm as Book 
        return 2
    Elseif akForm as Ingredient 
        return 3
    Elseif akForm as key
        return 4
    Elseif akForm as MiscObject 
        return 5
    Elseif akForm as SoulGem 
        return 6
    Elseif akForm as Weapon 
        return 7
    Elseif akForm as Potion 
        return 8
    Elseif akForm as Scroll 
        return 9
    Endif
    
    return -1
EndFunction 


;includes all of the above form types
Int Function GetFormTypeAll(Form F) Global
    Form akForm
    If F as Actor 
        return 0 
    Elseif F As ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif 
    
    If akForm as ActorBase 
        return 1
    ElseIf akForm as Action
        return 2
    Elseif akForm as LeveledActor 
        return 3
    Elseif akForm as Perk 
        return 4
    Elseif akForm as TalkingActivator 
        return 5
    ElseIf akForm as MusicType 
        return 6
    ElseIf akForm as SoundCategory 
        return 7
    ElseIf akForm as SoundDescriptor
        return 8
    Elseif akForm as Sound 
        return 9
    ElseIf akForm as AssociationType 
        return 10
    ElseIf akForm as Class 
        return 11
    ElseIf akForm as EquipSlot
        return 12
    Elseif akForm as Faction 
        return 13
    Elseif akForm as Headpart ;require skse?
        return 14
    Elseif akForm as Package 
        return 15
    Elseif akForm as Quest
        return 16
    Elseif akForm as Race 
        return 17
    Elseif akForm as VoiceType 
        return 18
    ElseIf akForm as Ammo 
        return 19
    ElseIf akForm as Armor 
        return 20
    ElseIf akForm as ArmorAddon
        return 21
    Elseif akForm as Book 
        return 22
    Elseif akForm as ConstructibleObject 
        return 23
    Elseif akForm as Ingredient 
        return 24
    Elseif akForm as key
        return 25
    Elseif akForm as LeveledItem 
        return 26
    Elseif akForm as MiscObject 
        return 27
    Elseif akForm as Outfit 
        return 28
    Elseif akForm as SoulGem 
        return 29
    Elseif akForm as Weapon 
        return 30
    ElseIf akForm as Enchantment 
        return 31
    ElseIf akForm as LeveledSpell 
        return 32
    ElseIf akForm as MagicEffect
        return 33
    Elseif akForm as Potion 
        return 34
    Elseif akForm as Scroll 
        return 35
    Elseif akForm as Shout 
        return 36
    Elseif akForm as Spell
        return 37
    Elseif akForm as WordOfPower 
        return 38
    ElseIf akForm as Art 
        return 39
    ElseIf akForm as ColorForm 
        return 40
    ElseIf akForm as CombatStyle
        return 41
    Elseif akForm as Formlist 
        return 42
    Elseif akForm as GlobalVariable 
        return 43
    Elseif akForm as Idle 
        return 44
    Elseif akForm as Keyword
        return 45
    Elseif akForm as Message 
        return 46
    Elseif akForm as TextureSet 
        return 47
    ElseIf akForm as Explosion 
        return 48
    ElseIf akForm as Hazard 
        return 49
    ElseIf akForm as ImageSpaceModifier 
        return 50
    ElseIf akForm as ImpactDataSet 
        return 51
    ElseIf akForm as Projectile 
        return 52
    ElseIf akForm as EncounterZone 
        return 53
    ElseIf akForm as Location 
        return 54
    ElseIf akForm as LocationRefType 
        return 55
    ElseIf akForm as ShaderParticleGeometry 
        return 56
    ElseIf akForm as VisualEffect 
        return 57
    ElseIf akForm as Weather 
        return 58
    ElseIf akForm as Activator 
        return 59
    ElseIf akForm as Container 
        return 60
    ElseIf akForm as Door 
        return 61
    ElseIf akForm as Flora 
        return 62
    ElseIf akForm as Furniture 
        return 63
    ElseIf akForm as Light 
        return 64
    ElseIf akForm as Static 
        return 65
    Endif 
    
    return -1
EndFunction 

;Same as above but returns strings instead of ints==============================================================================
;If TypeStrings == none, returns the ScriptName of type. Can pass in string array to return different strings for translations. 
String Function GetActorFormTypeString(Form F, String[] TypeStrings = none) Global
    Form akForm
    If TypeStrings == None
        
        If F as Actor 
            return "Actor" 
        Elseif F As ObjectReference 
            akForm = (F as ObjectReference).GetBaseObject() 
        Else 
            akForm = F 
        Endif 
        
        If akForm as ActorBase 
            return "ActorBase"
        ElseIf akForm as Action
            return "Action"
        Elseif akForm as LeveledActor 
            return "LeveledActor"
        Elseif akForm as Perk 
            return "Perk"
        Elseif akForm as TalkingActivator 
            return "TalkingActivator"
        Endif 
    Else 
        If F as Actor 
            return TypeStrings[0]
        Elseif F As ObjectReference 
            akForm = (F as ObjectReference).GetBaseObject() 
        Else 
            akForm = F 
        Endif 
        
        If akForm as ActorBase 
            return TypeStrings[1]
        ElseIf akForm as Action
            return TypeStrings[2]
        Elseif akForm as LeveledActor 
            return TypeStrings[3]
        Elseif akForm as Perk 
            return TypeStrings[4]
        Elseif akForm as TalkingActivator 
            return TypeStrings[5]
        Endif 
    Endif
    
    return ""
EndFunction

String Function GetAudioFormTypeString(Form F, String[] TypeStrings = none) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If TypeStrings == None
        If akForm as MusicType 
            return "MusicType"
        ElseIf akForm as SoundCategory 
            return "SoundCategory"
        ElseIf akForm as SoundDescriptor
            return "SoundDescriptor"
        Elseif akForm as Sound 
            return "Sound"
        Endif 
    Else 
        If akForm as MusicType 
            return TypeStrings[0]
        ElseIf akForm as SoundCategory 
            return TypeStrings[1]
        ElseIf akForm as SoundDescriptor
            return TypeStrings[2]
        Elseif akForm as Sound 
            return TypeStrings[3]
        Endif 
    Endif
    
    return ""
EndFunction 

String Function GetCharacterFormTypeString(Form F, String[] TypeStrings = none) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If TypeStrings == None
        
        If akForm as AssociationType 
            return "AssociationType"
        ElseIf akForm as Class 
            return "Class"
        ElseIf akForm as EquipSlot
            return "EquipSlot"
        Elseif akForm as Faction 
            return "Faction"
        Elseif akForm as Headpart ;require skse?
            return "Headpart"
        Elseif akForm as Package 
            return "Package"
        Elseif akForm as Quest
            return "Quest"
        Elseif akForm as Race 
            return "Race"
        Elseif akForm as VoiceType 
            return "VoiceType"
        Endif 
    Else 
        If akForm as AssociationType 
            return TypeStrings[0]
        ElseIf akForm as Class 
            return TypeStrings[1]
        ElseIf akForm as EquipSlot
            return TypeStrings[2]
        Elseif akForm as Faction 
            return TypeStrings[3]
        Elseif akForm as Headpart ;require skse?
            return TypeStrings[4]
        Elseif akForm as Package 
            return TypeStrings[5]
        Elseif akForm as Quest
            return TypeStrings[6]
        Elseif akForm as Race 
            return TypeStrings[7]
        Elseif akForm as VoiceType 
            return TypeStrings[8]
        Endif 
    Endif
    
    return ""
EndFunction

String Function GetItemFormTypeString(Form F, String[] TypeStrings = none) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If TypeStrings == None
        
        If akForm as Ammo 
            return "Ammo"
        ElseIf akForm as Armor 
            return "Armor"
        ElseIf akForm as ArmorAddon
            return "ArmorAddon"
        Elseif akForm as Book 
            return "Book"
        Elseif akForm as ConstructibleObject 
            return "ConstructibleObject"
        Elseif akForm as Ingredient 
            return "Ingredient"
        Elseif akForm as key
            return "key"
        Elseif akForm as LeveledItem 
            return "LeveledItem"
        Elseif akForm as MiscObject 
            return "MiscObject"
        Elseif akForm as Outfit 
            return "Outfit"
        Elseif akForm as SoulGem 
            return "SoulGem"
        Elseif akForm as Weapon 
            return "Weapon"
        Endif 
    Else 
        If akForm as Ammo 
            return TypeStrings[0]
        ElseIf akForm as Armor 
            return TypeStrings[1]
        ElseIf akForm as ArmorAddon
            return TypeStrings[2]
        Elseif akForm as Book 
            return TypeStrings[3]
        Elseif akForm as ConstructibleObject 
            return TypeStrings[4]
        Elseif akForm as Ingredient 
            return TypeStrings[5]
        Elseif akForm as key
            return TypeStrings[6]
        Elseif akForm as LeveledItem 
            return TypeStrings[7]
        Elseif akForm as MiscObject 
            return TypeStrings[8]
        Elseif akForm as Outfit 
            return TypeStrings[9]
        Elseif akForm as SoulGem 
            return TypeStrings[10]
        Elseif akForm as Weapon 
            return TypeStrings[11]
        Endif 
    Endif
    
    return ""
EndFunction

String Function GetMagicFormTypeString(Form F, String[] TypeStrings = none) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If TypeStrings == None
        
        If akForm as Enchantment 
            return "Enchantment"
        ElseIf akForm as LeveledSpell 
            return "LeveledSpell"
        ElseIf akForm as MagicEffect
            return "MagicEffect"
        Elseif akForm as Potion 
            return "Potion"
        Elseif akForm as Scroll 
            return "Scroll"
        Elseif akForm as Shout 
            return "Shout"
        Elseif akForm as Spell
            return "Spell"
        Elseif akForm as WordOfPower 
            return "WordOfPower"
        Endif 
    Else 
        If akForm as Enchantment 
            return TypeStrings[0]
        ElseIf akForm as LeveledSpell 
            return TypeStrings[1]
        ElseIf akForm as MagicEffect
            return TypeStrings[2]
        Elseif akForm as Potion 
            return TypeStrings[3]
        Elseif akForm as Scroll 
            return TypeStrings[4]
        Elseif akForm as Shout 
            return TypeStrings[5]
        Elseif akForm as Spell
            return TypeStrings[6]
        Elseif akForm as WordOfPower 
            return TypeStrings[7]
        Endif  
    Endif 
    
    return ""
EndFunction

String Function GetMiscFormTypeString(Form F, String[] TypeStrings = none) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If TypeStrings == None
        
        If akForm as Art 
            return "Art"
        ElseIf akForm as ColorForm 
            return "ColorForm"
        ElseIf akForm as CombatStyle
            return "CombatStyle"
        Elseif akForm as Formlist 
            return "Formlist"
        Elseif akForm as GlobalVariable 
            return "GlobalVariable"
        Elseif akForm as Idle 
            return "Idle"
        Elseif akForm as Keyword
            return "Keyword"
        Elseif akForm as Message 
            return "Message"
        Elseif akForm as TextureSet 
            return "TextureSet"
        Endif
    Else 
        If akForm as Art 
            return TypeStrings[0]
        ElseIf akForm as ColorForm 
            return TypeStrings[1]
        ElseIf akForm as CombatStyle
            return TypeStrings[2]
        Elseif akForm as Formlist 
            return TypeStrings[3]
        Elseif akForm as GlobalVariable 
            return TypeStrings[4]
        Elseif akForm as Idle 
            return TypeStrings[5]
        Elseif akForm as Keyword
            return TypeStrings[6]
        Elseif akForm as Message 
            return TypeStrings[7]
        Elseif akForm as TextureSet 
            return TypeStrings[8]
        Endif
    Endif
    
    return ""
EndFunction

String Function GetSpecialEffectFormTypeString(Form F, String[] TypeStrings = none) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If TypeStrings == None
        
        If akForm as Explosion 
            return "Explosion" 
        ElseIf akForm as Hazard 
            return "Hazard"
        ElseIf akForm as ImageSpaceModifier 
            return "ImageSpaceModifier"
        ElseIf akForm as ImpactDataSet 
            return "ImpactDataSet"
        ElseIf akForm as Projectile 
            return "Projectile"
        Endif 
    Else 
         
        If akForm as Explosion 
            return TypeStrings[0] 
        ElseIf akForm as Hazard 
            return TypeStrings[1]
        ElseIf akForm as ImageSpaceModifier 
            return TypeStrings[2]
        ElseIf akForm as ImpactDataSet 
            return TypeStrings[3]
        ElseIf akForm as Projectile 
            return TypeStrings[4]
        Endif 
    Endif
    
    return ""
EndFunction 

String Function GetWorldDataFormTypeString(Form F, String[] TypeStrings = none) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If TypeStrings == None
        
        If akForm as EncounterZone 
            return "EncounterZone" 
        ElseIf akForm as Location 
            return "Location"
        ElseIf akForm as LocationRefType 
            return "LocationRefType"
        ElseIf akForm as ShaderParticleGeometry 
            return "ShaderParticleGeometry"
        ElseIf akForm as VisualEffect 
            return "VisualEffect"
        ElseIf akForm as Weather 
            return "Weather"
        Endif
    Else 
        If akForm as EncounterZone 
            return TypeStrings[0] 
        ElseIf akForm as Location 
            return TypeStrings[1]
        ElseIf akForm as LocationRefType 
            return TypeStrings[2]
        ElseIf akForm as ShaderParticleGeometry 
            return TypeStrings[3]
        ElseIf akForm as VisualEffect 
            return TypeStrings[4]
        ElseIf akForm as Weather 
            return TypeStrings[5]
        Endif 
    Endif
    
    return ""
EndFunction 

String Function GetWorldObjectFormTypeString(Form F, String[] TypeStrings = none) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If TypeStrings == None
        If akForm as Activator 
            return "Activator" 
        ElseIf akForm as Container 
            return "Container"
        ElseIf akForm as Door 
            return "Door"
        ElseIf akForm as Flora 
            return "Flora"
        ElseIf akForm as Furniture 
            return "Furniture"
        ElseIf akForm as Light 
            return "Light"
        ElseIf akForm as Static 
            return "Static"
        Endif
    Else 
        If akForm as Activator 
            return TypeStrings[0] 
        ElseIf akForm as Container 
            return TypeStrings[1]
        ElseIf akForm as Door 
            return TypeStrings[2]
        ElseIf akForm as Flora 
            return TypeStrings[3]
        ElseIf akForm as Furniture 
            return TypeStrings[4]
        ElseIf akForm as Light 
            return TypeStrings[5]
        ElseIf akForm as Static 
            return TypeStrings[6]
        Endif 
    Endif 
    return ""
EndFunction 

;for base item types in player inventory
string Function GetInventoryItemFormTypeString(Form F, String[] TypeStrings = none) Global
    Form akForm
    if F as ObjectReference 
        akForm = (F as ObjectReference).GetBaseObject() 
    Else 
        akForm = F 
    Endif
    
    If TypeStrings == None
    
        If akForm as Ammo 
            return "Ammo"
        ElseIf akForm as Armor 
            return "Armor"
        Elseif akForm as Book 
            return "Book"
        Elseif akForm as Ingredient 
            return "Ingredient"
        Elseif akForm as key
            return "key"
        Elseif akForm as MiscObject 
            return "MiscObject"
        Elseif akForm as SoulGem 
            return "SoulGem"
        Elseif akForm as Weapon 
            return "Weapon"
        Elseif akForm as Potion 
            return "Potion"
        Elseif akForm as Scroll 
            return "Scroll"
        Endif
    Else 
        If akForm as Ammo 
            return TypeStrings[0]
        ElseIf akForm as Armor 
            return TypeStrings[1]
        Elseif akForm as Book 
            return TypeStrings[2]
        Elseif akForm as Ingredient 
            return TypeStrings[3]
        Elseif akForm as key
            return TypeStrings[4]
        Elseif akForm as MiscObject 
            return TypeStrings[5]
        Elseif akForm as SoulGem 
            return TypeStrings[6]
        Elseif akForm as Weapon 
            return TypeStrings[7]
        Elseif akForm as Potion 
            return TypeStrings[8]
        Elseif akForm as Scroll 
            return TypeStrings[9]
        Endif
    Endif 
    
    Return ""
EndFunction 

;includes all of the above types.
string Function GetFormTypeStringAll(Form F, String[] TypeStrings = none) Global
    Form akForm
    If TypeStrings == None
        
        If F as Actor 
            return "Actor" 
        Elseif F As ObjectReference 
            akForm = (F as ObjectReference).GetBaseObject() 
        Else 
            akForm = F 
        Endif 
        
        If akForm as ActorBase 
            return "ActorBase"
        ElseIf akForm as Action
            return "Action"
        Elseif akForm as LeveledActor 
            return "LeveledActor"
        Elseif akForm as Perk 
            return "Perk"
        Elseif akForm as TalkingActivator 
            return "TalkingActivator"
        ElseIf akForm as MusicType 
            return "MusicType"
        ElseIf akForm as SoundCategory 
            return "SoundCategory"
        ElseIf akForm as SoundDescriptor
            return "SoundDescriptor"
        Elseif akForm as Sound 
            return "Sound"
        ElseIf akForm as AssociationType 
            return "AssociationType"
        ElseIf akForm as Class 
            return "Class"
        ElseIf akForm as EquipSlot
            return "EquipSlot"
        Elseif akForm as Faction 
            return "Faction"
        Elseif akForm as Headpart ;require skse?
            return "Headpart"
        Elseif akForm as Package 
            return "Package"
        Elseif akForm as Quest
            return "Quest"
        Elseif akForm as Race 
            return "Race"
        Elseif akForm as VoiceType 
            return "VoiceType"
        ElseIf akForm as Ammo 
            return "Ammo"
        ElseIf akForm as Armor 
            return "Armor"
        ElseIf akForm as ArmorAddon
            return "ArmorAddon"
        Elseif akForm as Book 
            return "Book"
        Elseif akForm as ConstructibleObject 
            return "ConstructibleObject"
        Elseif akForm as Ingredient 
            return "Ingredient"
        Elseif akForm as key
            return "key"
        Elseif akForm as LeveledItem 
            return "LeveledItem"
        Elseif akForm as MiscObject 
            return "MiscObject"
        Elseif akForm as Outfit 
            return "Outfit"
        Elseif akForm as SoulGem 
            return "SoulGem"
        Elseif akForm as Weapon 
            return "Weapon"
        ElseIf akForm as Enchantment 
            return "Enchantment"
        ElseIf akForm as LeveledSpell 
            return "LeveledSpell"
        ElseIf akForm as MagicEffect
            return "MagicEffect"
        Elseif akForm as Potion 
            return "Potion"
        Elseif akForm as Scroll 
            return "Scroll"
        Elseif akForm as Shout 
            return "Shout"
        Elseif akForm as Spell
            return "Spell"
        Elseif akForm as WordOfPower 
            return "WordOfPower"
        ElseIf akForm as Art 
            return "Art"
        ElseIf akForm as ColorForm 
            return "ColorForm"
        ElseIf akForm as CombatStyle
            return "CombatStyle"
        Elseif akForm as Formlist 
            return "Formlist"
        Elseif akForm as GlobalVariable 
            return "GlobalVariable"
        Elseif akForm as Idle 
            return "Idle"
        Elseif akForm as Keyword
            return "Keyword"
        Elseif akForm as Message 
            return "Message"
        Elseif akForm as TextureSet 
            return "TextureSet"
        ElseIf akForm as Explosion 
            return "Explosion" 
        ElseIf akForm as Hazard 
            return "Hazard"
        ElseIf akForm as ImageSpaceModifier 
            return "ImageSpaceModifier"
        ElseIf akForm as ImpactDataSet 
            return "ImpactDataSet"
        ElseIf akForm as Projectile 
            return "Projectile"
        ElseIf akForm as EncounterZone 
            return "EncounterZone" 
        ElseIf akForm as Location 
            return "Location"
        ElseIf akForm as LocationRefType 
            return "LocationRefType"
        ElseIf akForm as ShaderParticleGeometry 
            return "ShaderParticleGeometry"
        ElseIf akForm as VisualEffect 
            return "VisualEffect"
        ElseIf akForm as Weather 
            return "Weather"
        ElseIf akForm as Activator 
            return "Activator" 
        ElseIf akForm as Container 
            return "Container"
        ElseIf akForm as Door 
            return "Door"
        ElseIf akForm as Flora 
            return "Flora"
        ElseIf akForm as Furniture 
            return "Furniture"
        ElseIf akForm as Light 
            return "Light"
        ElseIf akForm as Static 
            return "Static"
        Endif
    Else 
        If F as Actor 
            return TypeStrings[0]
        Elseif F As ObjectReference 
            akForm = (F as ObjectReference).GetBaseObject() 
        Else 
            akForm = F 
        Endif 
        
        If akForm as ActorBase 
            return TypeStrings[1]
        ElseIf akForm as Action
            return TypeStrings[2]
        Elseif akForm as LeveledActor 
            return TypeStrings[3]
        Elseif akForm as Perk 
            return TypeStrings[4]
        Elseif akForm as TalkingActivator 
            return TypeStrings[5]
        ElseIf akForm as MusicType 
            return TypeStrings[6]
        ElseIf akForm as SoundCategory 
            return TypeStrings[7]
        ElseIf akForm as SoundDescriptor
            return TypeStrings[8]
        Elseif akForm as Sound 
            return TypeStrings[9]
        ElseIf akForm as AssociationType 
            return TypeStrings[10]
        ElseIf akForm as Class 
            return TypeStrings[11]
        ElseIf akForm as EquipSlot
            return TypeStrings[12]
        Elseif akForm as Faction 
            return TypeStrings[13]
        Elseif akForm as Headpart ;require skse?
            return TypeStrings[14]
        Elseif akForm as Package 
            return TypeStrings[15]
        Elseif akForm as Quest
            return TypeStrings[16]
        Elseif akForm as Race 
            return TypeStrings[17]
        Elseif akForm as VoiceType 
            return TypeStrings[18]
        ElseIf akForm as Ammo 
            return TypeStrings[19]
        ElseIf akForm as Armor 
            return TypeStrings[20]
        ElseIf akForm as ArmorAddon
            return TypeStrings[21]
        Elseif akForm as Book 
            return TypeStrings[22]
        Elseif akForm as ConstructibleObject 
            return TypeStrings[23]
        Elseif akForm as Ingredient 
            return TypeStrings[24]
        Elseif akForm as key
            return TypeStrings[25]
        Elseif akForm as LeveledItem 
            return TypeStrings[26]
        Elseif akForm as MiscObject 
            return TypeStrings[27]
        Elseif akForm as Outfit 
            return TypeStrings[28]
        Elseif akForm as SoulGem 
            return TypeStrings[29]
        Elseif akForm as Weapon 
            return TypeStrings[30]
        ElseIf akForm as Enchantment 
            return TypeStrings[31]
        ElseIf akForm as LeveledSpell 
            return TypeStrings[32]
        ElseIf akForm as MagicEffect
            return TypeStrings[33]
        Elseif akForm as Potion 
            return TypeStrings[34]
        Elseif akForm as Scroll 
            return TypeStrings[35]
        Elseif akForm as Shout 
            return TypeStrings[36]
        Elseif akForm as Spell
            return TypeStrings[37]
        Elseif akForm as WordOfPower 
            return TypeStrings[38]
        ElseIf akForm as Art 
            return TypeStrings[39]
        ElseIf akForm as ColorForm 
            return TypeStrings[40]
        ElseIf akForm as CombatStyle
            return TypeStrings[41]
        Elseif akForm as Formlist 
            return TypeStrings[42]
        Elseif akForm as GlobalVariable 
            return TypeStrings[43]
        Elseif akForm as Idle 
            return TypeStrings[44]
        Elseif akForm as Keyword
            return TypeStrings[45]
        Elseif akForm as Message 
            return TypeStrings[46]
        Elseif akForm as TextureSet 
            return TypeStrings[47]
        ElseIf akForm as Explosion 
            return TypeStrings[48]
        ElseIf akForm as Hazard 
            return TypeStrings[49]
        ElseIf akForm as ImageSpaceModifier 
            return TypeStrings[50]
        ElseIf akForm as ImpactDataSet 
            return TypeStrings[51]
        ElseIf akForm as Projectile 
            return TypeStrings[52]
        ElseIf akForm as EncounterZone 
            return TypeStrings[53]
        ElseIf akForm as Location 
            return TypeStrings[54]
        ElseIf akForm as LocationRefType 
            return TypeStrings[55]
        ElseIf akForm as ShaderParticleGeometry 
            return TypeStrings[56]
        ElseIf akForm as VisualEffect 
            return TypeStrings[57]
        ElseIf akForm as Weather 
            return TypeStrings[58]
        ElseIf akForm as Activator 
            return TypeStrings[59]
        ElseIf akForm as Container 
            return TypeStrings[60]
        ElseIf akForm as Door 
            return TypeStrings[61]
        ElseIf akForm as Flora 
            return TypeStrings[62]
        ElseIf akForm as Furniture 
            return TypeStrings[63]
        ElseIf akForm as Light 
            return TypeStrings[64]
        ElseIf akForm as Static 
            return TypeStrings[65]
        Endif 
    Endif 
    
    return ""
EndFunction 
;=====================================================================

;Useful for force closing the inventory menu, or forcing the player to sheathe their weapon. 
;No requirements
Function DisableThenEnablePlayerControls(Float Delay = 1.0) Global
    Game.DisablePlayerControls() 
    Utility.Wait(Delay)
    Game.EnablePlayerControls()
EndFunction

;add and remove akForm from actor silently. 
;This is useful for instance after changing an actors speed via akActor.SetAv("SpeedMult", 110)
;Or after changing armor model paths, or on the player after changing item names if in the inventory or container menu, will display visually the name change. 
;No requirements
Function UpdateActor(Actor akActor, Form akForm) Global
    If akForm
        akActor.AddItem(akForm, 1, true)
        akActor.RemoveItem(akForm, 1, true)
    Endif
EndFunction 

;requires SKSE and Papyrus Extender 
;swap all worn equipment between actor A and B.
Function SwapEquipment(Actor A, Actor B) Global
    Form[] Aarmors = PO3_SKSEFunctions.AddAllEquippedItemsToArray(A)
    Form[] Barmors = PO3_SKSEFunctions.AddAllEquippedItemsToArray(B)
    
    Int LA = Aarmors.length 
    Int LB = Barmors.length 
    
    Int i = 0 
    While i < LA
        A.UnequipItem(Aarmors[i])
        i += 1 
    EndWhile 
    
    i = 0
    
    While i < LB
        B.UnequipItem(Barmors[i])
        i += 1 
    EndWhile  
    
    i = 0 
    
    While i < LA
        A.RemoveItem(Aarmors[i], 1, true, B)
        B.EquipItem(Aarmors[i], false, true)
        i += 1 
    EndWhile 
    
    i = 0 
    
    While i < LB
        B.RemoveItem(Barmors[i], 1, true, B)
        A.EquipItem(Barmors[i], false, true)
        i += 1 
    EndWhile 
EndFunction

;Set all ActorValues to Values on akActor 
;no requirements
Function SetActorValues(Actor akActor, String[] ActorValues, Float[] Values) Global
   
    int i = 0 
    int sL = ActorValues.length 
   
    While i < sL
        akActor.SetActorValue(ActorValues[i], Values[i])
        i += 1 
    EndWhile 
EndFunction

;Mod all ActorValues with Values on akActor 
;no requirements
Function ModActorValues(Actor akActor, String[] ActorValues, Float[] Values) Global
   
    int i = 0 
    int sL = ActorValues.length 
   
    While i < sL
        akActor.ModActorValue(ActorValues[i], Values[i])
        i += 1 
    EndWhile 
EndFunction

;return all actor values in ActorValueStrings to float array. 
;if DArrays == none, requires skse. Can pass in DynamicArrays form to not use skse.
Float[] Function GetActorValues(Actor akActor, String[] ActorValues, DynamicArrays DArrays = none) Global
    Float[] Values
    if ActorValues.length == 0 
        return Values 
    Endif 
        
    int i = 0 
    int sL = ActorValues.length 
    
    if DArrays == none
        Values = Utility.CreateFloatArray(sL) 
    Else 
        sL = ClampInt(sL, 0, 128)
        Values = DArrays.CreateFloatArray(sL) 
    Endif
    
    While i < sL
        Values[i] = akActor.GetActorValue(ActorValues[i])
        i += 1 
    EndWhile 
    
    return Values
EndFunction

;same as GetActorValues but returns for instance "Health = 100.0" in string array instead of float values
;if ActorValueStrings == none, uses ActorValues for strings. 
;can specify ActorValueStrings for translations. Indexes in ActorValues and ActorValuesStrings should match.
;if DArrays == none, requires skse. Can pass in DynamicArrays form to not use skse.
String[] Function GetActorValueStrings(Actor akActor, String[] ActorValues, String[] ActorValueStrings = none, DynamicArrays DArrays = none) Global
    String[] Values
    
    if ActorValues.length == 0 
        return Values 
    Endif 
    
    int i = 0 
    int sL = ActorValues.length 
    
    if DArrays == none
        String[] Valuesb = Utility.CreateStringArray(sL) 
        Values = Valuesb
    Else 
        sL = ClampInt(sL, 0, 128)
        Values = DArrays.CreateStringArray(sL) 
    Endif
    
    if ActorValueStrings == none
        While i < sL
            Values[i] = ActorValues[i] + " = " + akActor.GetActorValue(ActorValues[i])
            i += 1 
        EndWhile 
    Else 
        While i < sL
            Values[i] = ActorValueStrings[i] + " = " + akActor.GetActorValue(ActorValues[i])
            i += 1 
        EndWhile 
    Endif
    
    return Values
EndFunction

;same as GetActorValueStrings but puts all values in a single string divided by Divider.
;if ActorValueStrings == none, uses ActorValues for strings. 
;can specify ActorValueStrings for translations. Indexes in ActorValues and ActorValuesStrings should match.
;no requirements
String Function sGetActorValueStrings(Actor akActor, String[] ActorValues, String[] ActorValueStrings = none, String Divider = "||") Global
    String Values
    
    if ActorValues.length == 0 
        return Values 
    Endif 
    
    int i = 0 
    int sL = ActorValues.length - 1 ;to not add divider on last element
    
    if ActorValueStrings == none
        While i < sL
            Values += ActorValues[i] + " = " + akActor.GetActorValue(ActorValues[i]) + Divider
            i += 1 
        EndWhile 
        Values += ActorValues[i] + " = " + akActor.GetActorValue(ActorValues[i])
    Else 
        While i < sL
            Values += ActorValueStrings[i] + " = " + akActor.GetActorValue(ActorValues[i]) + Divider
            i += 1 
        EndWhile 
        Values += ActorValues[i] + " = " + akActor.GetActorValue(ActorValues[i])
    Endif
    
    return Values
EndFunction

;return all Base actor values in ActorValueStrings to float array. 
;if DArrays == none, requires skse. Can pass in DynamicArrays form to not use skse.
Float[] Function GetBaseActorValues(Actor akActor, String[] ActorValues, DynamicArrays DArrays = none) Global
    Float[] Values
    if ActorValues.length == 0 
        return Values 
    Endif 
        
    int i = 0 
    int sL = ActorValues.length 
    
    if DArrays == none
        Values = Utility.CreateFloatArray(sL) 
    Else 
        sL = ClampInt(sL, 0, 128)
        Values = DArrays.CreateFloatArray(sL) 
    Endif
    
    While i < sL
        Values[i] = akActor.GetBaseActorValue(ActorValues[i])
        i += 1 
    EndWhile 
    
    return Values
EndFunction

;same as GetBaseActorValues but returns for instance "Health = 100.0" in string array instead of float values
;if ActorValueStrings == none, uses ActorValues for strings. 
;can specify ActorValueStrings for translations. Indexes in ActorValues and ActorValuesStrings should match.
;if DArrays == none, requires skse. Can pass in DynamicArrays form to not use skse.
String[] Function GetBaseActorValueStrings(Actor akActor, String[] ActorValues, String[] ActorValueStrings = none, DynamicArrays DArrays = none) Global
    String[] Values
    
    if ActorValues.length == 0 
        return Values 
    Endif 
    
    int i = 0 
    int sL = ActorValues.length 
    
    if DArrays == none
        String[] Valuesb = Utility.CreateStringArray(sL) 
        Values = Valuesb
    Else 
        sL = ClampInt(sL, 0, 128)
        Values = DArrays.CreateStringArray(sL) 
    Endif
    
    if ActorValueStrings == none
        While i < sL
            Values[i] = ActorValues[i] + " = " + akActor.GetBaseActorValue(ActorValues[i])
            i += 1 
        EndWhile 
    Else 
        While i < sL
            Values[i] = ActorValueStrings[i] + " = " + akActor.GetBaseActorValue(ActorValues[i])
            i += 1 
        EndWhile 
    Endif
    
    return Values
EndFunction

;same as GetBaseActorValueStrings but puts all values in a single string divided by Divider.
;if ActorValueStrings == none, uses ActorValues for strings. 
;can specify ActorValueStrings for translations. Indexes in ActorValues and ActorValuesStrings should match.
;no requirements
String Function sGetBaseActorValueStrings(Actor akActor, String[] ActorValues, String[] ActorValueStrings = none, String Divider = "||") Global
    String Values
    
    if ActorValues.length == 0 
        return Values 
    Endif 
    
    int i = 0 
    int sL = ActorValues.length - 1 ;to not add divider on last element
    
    if ActorValueStrings == none
        While i < sL
            Values += ActorValues[i] + " = " + akActor.GetBaseActorValue(ActorValues[i]) + Divider
            i += 1 
        EndWhile 
        Values += ActorValues[i] + " = " + akActor.GetBaseActorValue(ActorValues[i])
    Else 
        While i < sL
            Values += ActorValueStrings[i] + " = " + akActor.GetBaseActorValue(ActorValues[i]) + Divider
            i += 1 
        EndWhile 
        Values += ActorValues[i] + " = " + akActor.GetBaseActorValue(ActorValues[i])
    Endif
    
    return Values
EndFunction

;return actor values of akActor from actor values in file to float array. 
;can specify your own file path. Look at the structure of DbActorValues.txt to make another file.
;requires skse and papyrusUtil.
Float[] Function GetActorValuesFromFile(Actor akActor, String filePath = "Data/interface/DbMiscFunctions/DbActorValues.txt") Global
    Float[] Values
    
    String Contents = MiscUtil.ReadFromFile(FilePath)
    
    int i = 0 
    int count = 0
    
    int ii = StringUtil.Find(Contents, i) 
    While ii > -1 
        count += 1
        i += 1 
        ii = StringUtil.Find(Contents, i, ii) 
    EndWhile
    
    Values = Utility.CreateFloatArray(Count) 
    
    i = 0
    ii = StringUtil.Find(Contents, i) 
    
    String ActorValue = GetStringFromFile(i, Contents, "", "\"", "\"")
    
    While i < count 
        Values[i] = akActor.GetActorValue(ActorValue) 
        i += 1 
        ii = StringUtil.Find(Contents, i, ii)
        ActorValue = GetStringFromFile(i, Contents, "", "\"", "\"", ii)
    EndWhile 
    
    return Values 
EndFunction 

;same as GetActorValuesFromFile but returns for instance "Health = 100.0" in string array instead of float values
;can specify your own file path. Look at the structure of DbActorValues.txt to make another file.
;requires skse and papyrusUtil.
String[] Function GetActorValueStringsFromFile(Actor akActor, String filePath = "Data/interface/DbMiscFunctions/DbActorValues.txt") Global
    String[] Values
    
    String Contents = MiscUtil.ReadFromFile(FilePath)
    
    int i = 0 
    int count = 0
    
    int ii = StringUtil.Find(Contents, i) 
    While ii > -1 
        count += 1
        i += 1 
        ii = StringUtil.Find(Contents, i, ii) 
    EndWhile
    
    Values = Utility.CreateStringArray(Count) 
    
    i = 0
    ii = StringUtil.Find(Contents, i) 
    
    String ActorValue = GetStringFromFile(i, Contents, "", "\"", "\"")
    
    While i < count 
        String ActorValueString = GetStringFromFile(i, default = ActorValue, StartIndex = ii)
        Values[i] = ActorValueString + " = " + akActor.GetActorValue(ActorValue) 
        i += 1 
        ii = StringUtil.Find(Contents, i, ii)
        ActorValue = GetStringFromFile(i, Contents, "", "\"", "\"", ii)
    EndWhile 
    
    return Values 
EndFunction 

;same as GetActorValueStringsFromFile but puts all the values in a single string seperated by Divider
;can specify your own file path. Look at the structure of DbActorValues.txt to make another file.
;requires skse and papyrusUtil.
String Function sGetActorValueStringsFromFile(Actor akActor, String Divider = "||", String filePath = "Data/interface/DbMiscFunctions/DbActorValues.txt") Global
    String Values = ""
    
    String Contents = MiscUtil.ReadFromFile(FilePath)
    
    int i = 0 
    int count = 0
    
    int ii = StringUtil.Find(Contents, i) 
    While ii > -1 
        count += 1
        i += 1 
        ii = StringUtil.Find(Contents, i, ii) 
    EndWhile
    
    i = 0
    ii = StringUtil.Find(Contents, i) 
    
    String ActorValue = GetStringFromFile(i, Contents, "", "\"", "\"")
    
    While i < count 
        String ActorValueString = GetStringFromFile(i, default = ActorValue, StartIndex = ii)
        Values += ActorValueString + " = " + akActor.GetActorValue(ActorValue) + Divider
        i += 1 
        ii = StringUtil.Find(Contents, i, ii)
        ActorValue = GetStringFromFile(i, Contents, "", "\"", "\"", ii)
    EndWhile 
    
    return Values 
EndFunction  

;return all Base actor values in file to float array. 
;can specify your own file path. Look at the structure of DbActorValues.txt to make another file.
;requires skse and papyrusUtil.
Float[] Function GetBaseActorValuesFromFile(Actor akActor, String filePath = "Data/interface/DbMiscFunctions/DbActorValues.txt") Global
    Float[] Values
    
    String Contents = MiscUtil.ReadFromFile(FilePath)
    
    int i = 0 
    int count = 0
    
    int ii = StringUtil.Find(Contents, i) 
    While ii > -1 
        count += 1
        i += 1 
        ii = StringUtil.Find(Contents, i, ii) 
    EndWhile
    
    Values = Utility.CreateFloatArray(Count) 
    
    i = 0
    ii = StringUtil.Find(Contents, i) 
    
    String ActorValue = GetStringFromFile(i, Contents, "", "\"", "\"")
    
    While i < count 
        Values[i] = akActor.GetBaseActorValue(ActorValue) 
        i += 1 
        ii = StringUtil.Find(Contents, i, ii)
        ActorValue = GetStringFromFile(i, Contents, "", "\"", "\"", ii)
    EndWhile 
    
    return Values 
EndFunction 

;same as GetBaseActorValuesFromFile but returns for instance "Health = 100.0" in string array instead of float values
;can specify your own file path. Look at the structure of DbActorValues.txt to make another file.
;requires skse and papyrusUtil.
String[] Function GetBaseActorValueStringsFromFile(Actor akActor, String filePath = "Data/interface/DbMiscFunctions/DbActorValues.txt") Global
    String[] Values
    
    String Contents = MiscUtil.ReadFromFile(FilePath)
    
    int i = 0 
    int count = 0
    
    int ii = StringUtil.Find(Contents, i) 
    While ii > -1 
        count += 1
        i += 1 
        ii = StringUtil.Find(Contents, i, ii) 
    EndWhile
    
    Values = Utility.CreateStringArray(Count) 
    
    i = 0
    ii = StringUtil.Find(Contents, i) 
    
    String ActorValue = GetStringFromFile(i, Contents, "", "\"", "\"")
    
    While i < count 
        String ActorValueString = GetStringFromFile(i, default = ActorValue, StartIndex = ii)
        Values[i] = ActorValueString + " = " + akActor.GetBaseActorValue(ActorValue) 
        i += 1 
        ii = StringUtil.Find(Contents, i, ii)
        ActorValue = GetStringFromFile(i, Contents, "", "\"", "\"", ii)
    EndWhile 
    
    return Values 
EndFunction 

;same as GetBaseActorValueStringsFromFile but puts all the values in a single string seperated by Divider
;can specify your own file path. Look at the structure of DbActorValues.txt to make another file.
;requires skse and papyrusUtil.
String Function sGetBaseActorValueStringsFromFile(Actor akActor, String Divider = "||", String filePath = "Data/interface/DbMiscFunctions/DbActorValues.txt") Global
    String Values = ""
    
    String Contents = MiscUtil.ReadFromFile(FilePath)
    
    int i = 0 
    int count = 0
    
    int ii = StringUtil.Find(Contents, i) 
    While ii > -1 
        count += 1
        i += 1 
        ii = StringUtil.Find(Contents, i, ii) 
    EndWhile
    
    i = 0
    ii = StringUtil.Find(Contents, i) 
    
    String ActorValue = GetStringFromFile(i, Contents, "", "\"", "\"")
    
    While i < count 
        String ActorValueString = GetStringFromFile(i, default = ActorValue, StartIndex = ii)
        Values += ActorValueString + " = " + akActor.GetBaseActorValue(ActorValue) + Divider
        i += 1 
        ii = StringUtil.Find(Contents, i, ii)
        ActorValue = GetStringFromFile(i, Contents, "", "\"", "\"", ii)
    EndWhile 
    
    return Values 
EndFunction   

;Attach the akScript to the Ref. 
;Mostly for LE as on SE you can use the No Esp mod instead
;if ref == none, attachs akScript to the player
;requires skse and consoleUtil or DbSkseFunctions.
;DbSkseFunctions (included with this mod) only works on SE and AE
Function AttachPapyrusScript(String akScript, ObjectReference Ref) Global
    if Ref
        if DbSkseFunctions.GetVersion() >= 4.7
            DbSkseFunctions.ExecuteConsoleCommand(("APS " + akScript), ref)
            
        ElseIf ConsoleUtil.GetVersion() > 0
            ObjectReference CurrentConsoleRef = ConsoleUtil.GetSelectedReference() 
            ConsoleUtil.SetSelectedReference(Ref)
            ConsoleUtil.ExecuteCommand("APS " + akScript)
            ConsoleUtil.SetSelectedReference(CurrentConsoleRef)
        Endif
    Else 
        ConsoleUtil.ExecuteCommand("Player.APS " + akScript)
    Endif
EndFunction

;menuName must match a valid menu name in UI.psc 
;requires skse
Function OpenMenu(string menuName) Global
    UI.InvokeString("HUD Menu", "_global.skse.OpenMenu", menuName)
    ;Debug.Notification("Opening " + menuName + " Menu")
EndFunction 

Function CloseMenu(string menuName) Global
    UI.InvokeString("HUD Menu", "_global.skse.CloseMenu", menuName)
    ;Debug.Notification("Closing " + menuName + " Menu")
Endfunction

;Finds the last index of String ToFind in string s
;Example: FindLastStringIndex("The dog is the coolest dog in the world", "The") returns 30, the last instance of "the" 
;Requires skse.
Int Function FindLastStringIndex(String s, String ToFind) Global
    Int i = StringUtil.Find(s, ToFind)
    While i > -1 
        i += 1
        Int ii = StringUtil.Find(s, ToFind, i)
        If ii == -1 
            return (i - 1)
        Else 
           i = ii 
        Endif 
    EndWhile 

    Return i
EndFunction

; returns the index of the first character of toFind inside string s
; returns -1 if toFind is not part of the string or if startIndex is invalid or if the characters preceding and following ToFind in s are not whitespace
; Example 
;FindWholeWordString("TestString", "String") returns -1
;FindWholeWordString("Test String", "String") returns 5
Int Function FindWholeWordString(String s, String ToFind, Int StartIndex = 0) Global
    Int i = StringUtil.Find(s, ToFind, StartIndex)  
    Int ToFindLength = StringUtil.GetLength(ToFind)
    If i == -1 || ToFind == "" || s == ""
        return i
    ElseIf IsCharWhiteSpace( StringUtil.GetNthChar(s, (i - 1)) ) && IsCharWhiteSpace( StringUtil.GetNthChar(s, (i + ToFindLength)) ) ;are the characters preceding and following ToFind in s Whitespace.    
        return i
    Else 
        i += ToFindLength 
        return FindWholeWordString(s, ToFind, i)
    Endif
EndFunction

;returns true if the single character string C is whitespace.
;requires skse
Bool Function IsCharWhiteSpace(String C) Global
    return(C == "" || C == " " || C == "\n" || C == "\t" || !StringUtil.IsPrintable(c))
EndFunction

;find next word in string. (Next string of non white space) from startIndex
;Examples:
;String A = FindNextWordInString("This is some text ")      ;A = "This"
;String B = FindNextWordInString("This is some text ", 12)  ;B = "text"
;String C = FindNextWordInString("This is some text ", 9)   ;C = "ome"
;requires skse
String Function FindNextWordInString(String s, int startIndex = 0) global 
    string sResult
    int iStart = FindNextNonWhiteSpaceCharIndexInString(s, startIndex)
        
    If iStart > -1
        int iEnd = FindNextWhiteSpaceCharIndexInString(s, iStart)
        if iEnd == -1 
            iEnd = StringUtil.GetLength(s)
        endif 
        
        sResult = StringUtil.Substring(s, iStart, (iEnd - iStart))
    Endif
    
    return sResult
EndFunction

;same as above but in reverse
;if startIndex is -1, startIndex is string length
;requires skse
String Function RFindNextWordInString(String s, int startIndex = -1) global 
    string sResult
    
    If startIndex < 0
        StringUtil.GetLength(s) - 1
    Endif
    
    int iEnd = RFindNextNonWhiteSpaceCharIndexInString(s, startIndex)
        
    If iEnd > -1
        int iStart = RFindNextWhiteSpaceCharIndexInString(s, iStart)
        if iStart == -1 
            iStart = 0
        endif 
        
        sResult = StringUtil.Substring(s, iStart, (iEnd - iStart))
    Endif
    
    return sResult
EndFunction

;requires skse
Int function FindNextNonWhiteSpaceCharIndexInString(String s, int startIndex = 0) global
    int L = StringUtil.GetLength(s)
    int i = startIndex
    String Char = StringUtil.GetNthChar(s, i)
    While DbMiscFunctions.IsCharWhiteSpace(Char) && i < L
        i += 1
        Char = StringUtil.GetNthChar(s, i)
    EndWhile 
    
    If i < L
        return i
    Else 
        return -1
    Endif
EndFunction

;same as above but in reverse
;if startIndex is -1, startIndex is string length
;requires skse
Int function RFindNextNonWhiteSpaceCharIndexInString(String s, int startIndex = -1) global
    int L = StringUtil.GetLength(s) - 1
    int i = startIndex
    If startIndex < 0
        i = L
    Endif
    
    If startIndex > L 
        startIndex = L
    Endif
    
    String Char = StringUtil.GetNthChar(s, i)
    While DbMiscFunctions.IsCharWhiteSpace(Char) && i > -1
        i -= 1
        Char = StringUtil.GetNthChar(s, i)
    EndWhile 
    
    return i
EndFunction

;requires skse
Int Function FindNextWhiteSpaceCharIndexInString(String s, int startIndex = 0) global
    int L = StringUtil.GetLength(s)
    int i = startIndex
    String Char = StringUtil.GetNthChar(s, i)
    While !DbMiscFunctions.IsCharWhiteSpace(Char) && i < L
        i += 1
        Char = StringUtil.GetNthChar(s, i)
    EndWhile 
    
    If i < L
        return i
    Else 
        return -1
    Endif
EndFunction

;same as above but in reverse
;if startIndex is -1, startIndex is string length
;requires skse
Int Function RFindNextWhiteSpaceCharIndexInString(String s, int startIndex = -1) global
   int L = StringUtil.GetLength(s) - 1
    int i = startIndex
    If startIndex < 0
        i = L
    Endif
    
    If startIndex > L 
        startIndex = L
    Endif
    
    String Char = StringUtil.GetNthChar(s, i)
    While !DbMiscFunctions.IsCharWhiteSpace(Char) && i > -1
        i -= 1
        Char = StringUtil.GetNthChar(s, i)
    EndWhile 
    
    If i < L
        return i
    Else 
        return -1
    Endif
EndFunction

;requires skse
String Function FindNextNonWhiteSpaceCharInString(String s, int startIndex = 0) global
    int i = FindNextNonWhiteSpaceCharIndexInString(s, startIndex)
    If i > -1 
        return StringUtil.GetNthChar(s, i)
    Endif
    
    return ""
EndFunction

;same as above but in reverse
;if startIndex is -1, startIndex is string length
;requires skse
String Function RFindNextNonWhiteSpaceCharInString(String s, int startIndex = -1) global
    int i = RFindNextNonWhiteSpaceCharIndexInString(s, startIndex)
    If i > -1 
        return StringUtil.GetNthChar(s, i)
    Endif
    
    return ""
EndFunction

;requires skse
String Function FindNextWhiteSpaceCharInString(String s, int startIndex = 0) global
    int i = FindNextWhiteSpaceCharIndexInString(s, startIndex)
    If i > -1 
        return StringUtil.GetNthChar(s, i)
    Endif
    
    return ""
EndFunction

;same as above but in reverse
;if startIndex is -1, startIndex is string length
;requires skse
String Function RFindNextWhiteSpaceCharInString(String s, int startIndex = -1) global
   int i = RFindNextWhiteSpaceCharIndexInString(s, startIndex)
    If i > -1 
        return StringUtil.GetNthChar(s, i)
    Endif
    
    return ""
EndFunction

;requires skse
String Function RemoveWhiteSpaces(String s, Bool IncludeSpaces = True, Bool IncludeTabs = true, Bool IncludeNewLines = true) Global
    If IncludeSpaces 
        s = StringReplace(s, " ", "") 
    Endif
    
    If IncludeTabs 
        s = StringReplace(s, "\t", "") 
    Endif
    
    If IncludeNewLines 
        s = StringReplace(s, "\n", "") 
    Endif
    return s
EndFunction

;Requires skse
Int Function CountWhiteSpaces(String s, Bool IncludeSpaces = True, Bool IncludeTabs = true, Bool IncludeNewLines = true) Global 
    Int Count = 0 
    
    If IncludeSpaces 
        Count += CountStringsInString(s, " ") 
    Endif
    
    If IncludeTabs 
        Count += CountStringsInString(s, "\t") 
    Endif
    
    If IncludeNewLines 
        Count += CountStringsInString(s, "\n") 
    Endif 
    return Count
Endfunction

;Count the number of String ToFind that occures in String s. 
;If WholeWordsOnly == true, only counts where the string ToFind occures is surrounded by whiteSpace.
;requires skse
Int Function CountStringsInString(String s, String ToFind, Bool WholeWordsOnly = false) Global
    Int L = StringUtil.GetLength(ToFind)
    Int Count = 0 
    
    If WholeWordsOnly
        Int i = FindWholeWordString(s, ToFind)
        While i > -1 
            Count += 1
            i += L
            i = FindWholeWordString(s, ToFind, i)
        EndWhile 
    Else
        Int i = StringUtil.Find(s, ToFind)
        While i > -1 
            Count += 1
            i += L
            i = StringUtil.Find(s, ToFind, i)
        EndWhile 
    Endif
    
    return Count
EndFunction

;string[] function SliceStringArray(string[] ArrayValues, int StartIndex, int EndIndex = -1) global native

Int[] Function GetPositionsOfStringInStrings(String s, String ToFind, Bool WholeWordsOnly = false) Global
	String Positions
	int[] PositionIndexes
	
	int i
	Int L = StringUtil.GetLength(ToFind)
    Int Count = 0 
    
    If WholeWordsOnly
        i = FindWholeWordString(s, ToFind)
        While i > -1 
			Positions += (i + "||")
            Count += 1
            i += L
            i = FindWholeWordString(s, ToFind, i)
        EndWhile 
    Else
        i = StringUtil.Find(s, ToFind)
        While i > -1 
            Positions += (i + "||")
            Count += 1
            i += L
            i = StringUtil.Find(s, ToFind, i)
        EndWhile 
    Endif
    
	if Positions == "" 
		return PositionIndexes 
	Endif
	
	String[] p = StringUtil.Split(Positions, "||") 
	i = 0 
	L = p.length
	PositionIndexes = Utility.CreateIntArray(L)
	While i < L 
		PositionIndexes[i] = p[i] as int 
		i += 1
	EndWhile
	
    return PositionIndexes
EndFunction

String Function CreateRandomWord(int WordLength, string letters = "bcdfghjklmnpqrstvwxyz", string vowels = "aeiou", \
								 string pairs = "st gr ea ie ei pr qw fr cr tr vr br pl cl ") global

	string word
	int L = stringUtil.GetLength(letters) - 1
	int vL = stringUtil.GetLength(vowels) - 1
	int i = 0
	
	bool vowel = false
	
	while i < WordLength
		if !vowel
			string letter =  StringUtil.GetNthChar(letters, Utility.RandomInt(i, L))
			word += letter
			i += 1
			if i < WordLength 
				int makePair = Utility.RandomInt(0, 1)
				if makePair == 1 
					int pIndex = stringUtil.Find(pairs, letter) 
					if pIndex != -1 
						string pLetter = stringUtil.GetNthChar(pairs, pIndex + 1) 
						if pLetter == " "
							pLetter = stringUtil.GetNthChar(pairs, pIndex - 1) 
						endif
						word += pLetter 
						i += 1 
					Endif 
				Endif 
			Endif
		Else 
			word += StringUtil.GetNthChar(vowels, Utility.RandomInt(i, vL))
		Endif
		
		vowel = !vowel
	EndWhile
	
	return word
EndFunction

String Function GetRandomWordFromString(String s, String Divider = " ") Global
	String[] words = StringUtil.Split(s, Divider) 
	
	int max = words.length - 1
	
	if max == -1
		return ""
	endif
	
	Return words[Utility.RandomInt(0, max)]
EndFunction

String Function GetRandomWordsFromString(String s, int numOfWords, String Divider = " ") Global
	String[] words = StringUtil.Split(s, Divider) 
	
	int max = words.length - 1
	
	if max == -1
		return ""
	endif
	
	string sReturn = words[Utility.RandomInt(0, max)]
	numOfWords -= 1
	
	int i = 1
	
	while i < numOfWords
		sReturn += (Divider + words[Utility.RandomInt(0, max)])
		i += 1
	EndWhile 
	
	return sReturn
EndFunction

String[] Function GetRandomWordsFromStringA(String s, int numOfWords, String Divider = " ") Global
	String[] words = StringUtil.Split(s, Divider) 
	
	int max = words.length - 1
	string[] sReturn
	
	if max == -1
		return sReturn
	endif
	
	sReturn = Utility.CreateStringArray(numOfWords)
	sReturn[0] = words[Utility.RandomInt(0, max)]
	
	int i = 1
	
	while i < numOfWords
		sReturn[i] = words[Utility.RandomInt(0, max)]
		i += 1
	EndWhile 
	
	return sReturn
EndFunction

String Function GetLoremipsumNoPunctuation() Global 
	return "Lorem ipsum dolor sit amet consectetur adipiscing elit Etiam non urna velit Vivamus nisi mauris lacinia nec mollis ac elementum id arcu Nullam venenatis porta odio pharetra efficitur nisi lobortis vitae Maecenas eget orci ut enim euismod feugiat et sit amet arcu Sed eu lorem augue Ut libero risus euismod nec dui a auctor sodales magna In blandit aliquam viverra Vivamus vitae nisi molestie lobortis purus et finibus nulla Orci varius natoque penatibus et magnis dis parturient montes nascetur ridiculus mus Ut rhoncus id enim vel posuere Curabitur tincidunt vitae libero ut pulvinar Sed vitae quam eget orci egestas porttitor at in lectus Aenean tempus consectetur nulla sed rhoncus Nam vel felis erat"
EndFunction

String Function GetLoremipsum() Global 
	return "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam non urna velit. Vivamus nisi mauris, lacinia nec mollis ac, elementum id arcu. Nullam venenatis porta odio, pharetra efficitur nisi lobortis vitae. Maecenas eget orci ut enim euismod feugiat et sit amet arcu. Sed eu lorem augue. Ut libero risus, euismod nec dui a, auctor sodales magna. In blandit aliquam viverra. Vivamus vitae nisi molestie, lobortis purus et, finibus nulla. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Ut rhoncus id enim vel posuere. Curabitur tincidunt vitae libero ut pulvinar. Sed vitae quam eget orci egestas porttitor at in lectus. Aenean tempus consectetur nulla sed rhoncus. Nam vel felis erat."
EndFunction

;remove all SearchStr instances from TragetStr except for the first instance
;requires skse
String Function RemoveDuplicateStrings(String TargetStr, String SearchStr) Global
    
    Int i = StringUtil.Find(TargetStr, SearchStr) 
    If i > -1 
        i += StringUtil.GetLength(SearchStr) + 1
        TargetStr = StringReplace(TargetStr, SearchStr, "", 0, i)
    Endif 
    
    Return TargetStr
EndFunction

;Remove All Duplicate strings separated by Divider. Example: 
;RemoveAllDuplicateStrings("TestString||TestString||Hmmm||TestString|| Test String ||Hmm||TestString") returns "TestString||Hmmm|| Test String ||"
;RemoveAllDuplicateStrings("TestString||TestString||Hmmm||TestString|| Test String ||Hmm||TestString", IncludeDividersInResult = false) returns "TestStringHmmm Test String "
;requires skse
String Function RemoveAllDuplicateStrings(String TargetStr, String Divider = "||", Bool IncludeDividersInResult = true) Global 
    String[] Strings = StringUtil.Split(TargetStr, Divider)  
    String NewString
    Int i = 1
    Int L = Strings.Length 
    
    If IncludeDividersInResult
        NewString = Strings[0] + Divider
        While i < L 
            If Strings[i] != "" 
                If StringUtil.Find(NewString, Strings[i]) == -1 
                    NewString += Strings[i] + Divider 
                Endif
            EndIf 
            i += 1 
        EndWhile 
    Else 
        NewString = Strings[0]
        While i < L 
            If Strings[i] != "" 
                If StringUtil.Find(NewString, Strings[i]) == -1 
                    NewString += Strings[i] 
                Endif
            EndIf 
            i += 1 
        EndWhile 
    Endif
    
    Return NewString
EndFunction

;Replace instances of the SearchStr with the ReplaceStr in the TargetStr
;Default count = 0 which means replace all instances. Otherwise only replace the Count number.
;Example: 
;String MyString = "A Yes, B Yes, C Yes"
;String MyStringB = StringReplace(MyString, "Yes", "No")
;String MyStringC = StringReplace(MyString, "Yes", "No", 2)
;MyStringB == "A No, B No, C No"
;MyStringC == "A No, B No, C Yes"
;Requires SKSE
String Function StringReplace(String TargetStr, String SearchStr, String ReplaceStr, Int Count = 0, Int StartIndex = 0) Global
    Int C = 0
    Int CAdd = 1
    If Count == 0
        C = -1 
        CAdd = 0
    Endif 
    
    Int I = StringUtil.Find(TargetStr, SearchStr, StartIndex)
    Int SL = StringUtil.GetLength(SearchStr)
    Int SR = StringUtil.GetLength(ReplaceStr)
    
    ;If SR <= 0 
    ;    SR = 1 
    ;Endif
    
    While I > -1 && C < Count
        String strBegin = ""
        If I > 0
            strBegin = StringUtil.SubString(TargetStr, 0, I)
        Endif
        String strEnd = StringUtil.SubString(TargetStr, (I + SL))
        TargetStr = strBegin + ReplaceStr + strEnd 
        C += CAdd
        I += SR 
        I = StringUtil.Find(TargetStr, SearchStr, I)
    EndWhile
    
    Return TargetStr
EndFunction

;insert the InsertStr to the TargetStr at the CharPosition and return new string. 
;if CharPosition == -1, appends the InsertStr to the end of TargetStr
String Function StringInsert(String TargetStr, String InsertStr, Int CharPosition = -1) Global
    Int L = StringUtil.GetLength(TargetStr)
    
    if CharPosition == 0 
        return (InsertStr + TargetStr)
    ElseIf CharPosition <= -1 || CharPosition >= L
        return (TargetStr + InsertStr) 
    Else 
        String StartStr = StringUtil.SubString(TargetStr, 0, CharPosition)
        String EndStr = StringUtil.SubString(TargetStr, CharPosition)
        Return (StartStr + InsertStr + EndStr)
    Endif
EndFunction

;Remove a single character in String s at Index 
;Requires skse
String Function StringRemoveCharAt(String s, Int Index) Global
    Int L = StringUtil.GetLength(s)
    If Index == 0 
        Return StringUtil.SubString(s, 1)
    Elseif Index >= (L - 1)
        Return StringUtil.SubString(s, 0, (L - 1)) 
    Else 
        String sStart = StringUtil.SubString(s, 0, Index)
        Return sStart + StringUtil.SubString(s, (Index + 1))
    Endif
EndFunction

;Remove Non printable characters from string 
;Requires skse.
String Function StringRemoveNonPrintableCharacters(String s) Global
    Int i = 0 
    Int L = StringUtil.GetLength(s)
    While i < L 
        String C = StringUtil.GetNthChar(s, i)
        If !StringUtil.IsPrintable(C)
            s = StringRemoveCharAt(s, i) 
            L -= 1
        Else 
            i += 1 
        Endif 
    Endwhile
    
    return s
EndFunction

;Remove Non printable characters from string 
;Requires skse.
String Function StringRemovePrintableCharacters(String s) Global
    Int i = 0 
    Int L = StringUtil.GetLength(s)
    While i < L 
        String C = StringUtil.GetNthChar(s, i)
        If StringUtil.IsPrintable(C)
            s = StringRemoveCharAt(s, i) 
            L -= 1
        Else 
            i += 1 
        Endif 
    Endwhile
    
    return s
EndFunction

;Add prefix to string s and return new string 
;If OnlyIfNotPresent == true (default) only adds the prefix if it's not already present. 
;Requires skse
String Function AddPrefixToString(String s, String Prefix, Bool OnlyIfNotPresent = true) Global
    If StringUtil.Find(s, Prefix) != 0 || !OnlyIfNotPresent
        Return (Prefix + s)
    Endif
    
    Return s
EndFunction

;Same as above but adds to all strings in array
;Requires skse
String[] Function AddPrefixToStrings(String[] s, String Prefix, Bool OnlyIfNotPresent = true) Global
    Int i = 0 
    Int L = s.length 
    While i < L 
        s[i] = AddPrefixToString(s[i], Prefix, OnlyIfNotPresent)
        i += 1 
    EndWhile
    
    return s
EndFunction

;Remove prefix from string s, if it exists and return new string, or return s if not present
;Requires skse
String Function RemovePrefixFromString(String s, String Prefix) Global
    If StringUtil.Find(s, Prefix) == 0 
        Return StringUtil.SubString(s, StringUtil.GetLength(Prefix))
    Endif 
    
    Return s
EndFunction

;Same as above but removes from all strings in array
;Requires skse
String[] Function RemovePrefixFromStrings(String[] s, String Prefix) Global
    Int i = 0 
    Int L = s.length 
    While i < L 
        s[i] = RemovePrefixFromString(s[i], Prefix)
        i += 1 
    EndWhile
    
    Return s
EndFunction

;Add suffix to string s and return new string 
;If OnlyIfNotPresent == true (default) only adds the suffix if it's not already present. 
;Requires skse
String Function AddSuffixToString(String s, String Suffix, Bool OnlyIfNotPresent = true) Global
    Int i = FindLastStringIndex(s, Suffix)
    If i == -1 || i != ( StringUtil.GetLength(s) - StringUtil.GetLength(Suffix) ) || !OnlyIfNotPresent
        Return (s + Suffix)
    Endif 
    
    Return s
EndFunction

;Same as above but adds to all strings in array
;Requires skse
String[] Function AddSuffixToStrings(String[] s, String Suffix, Bool OnlyIfNotPresent = true) Global
    Int i = 0 
    Int L = s.length 
    While i < L 
        s[i] = AddSuffixToString(s[i], Suffix, OnlyIfNotPresent)
        i += 1 
    EndWhile
    
    return s
EndFunction

;Remove suffix from string s, if it exists and return new string, or return s if not present
;Requires skse
String Function RemoveSuffixFromString(String s, String Suffix) Global
    Int i = FindLastStringIndex(s, Suffix)
    
    If i > -1 
        Int index = ( StringUtil.GetLength(s) - StringUtil.GetLength(Suffix) ) 
        If i == index
            Return StringUtil.SubString(s, 0, Index)
        Endif 
    Endif 
    
    Return s
EndFunction

;Same as above but removes from all strings in array
;Requires skse
String[] Function RemoveSuffixFromStrings(String[] s, String Suffix) Global
    Int i = 0 
    Int L = s.length 
    While i < L 
        s[i] = RemoveSuffixFromString(s[i], Suffix)
        i += 1 
    EndWhile
    
    Return s
EndFunction

;Add prefix to akForm's name
;If OnlyIfNotPresent == true (default) only adds the prefix if it's not already present. 
;Requires skse
Function AddPrefixToFormName(Form akForm, String Prefix, Bool OnlyIfNotPresent = true) Global
    If akForm
        ObjectReference Ref = akForm as ObjectReference 
        If Ref 
            Ref.SetDisplayName(AddPrefixToString(Ref.GetDisplayName(), Prefix, OnlyIfNotPresent))
        Else 
            akForm.SetName(AddPrefixToString(akForm.GetName(), Prefix, OnlyIfNotPresent))
        Endif 
    Endif
EndFunction

;Same as above but adds to all form names in array 
;Requires skse
Function AddPrefixToFormNames(Form[] akForms, String Prefix, Bool OnlyIfNotPresent = true) Global
    Int i = 0 
    Int L = akForms.Length 
    While i < L 
        AddPrefixToFormName(akForms[i], Prefix, OnlyIfNotPresent)
        i += 1 
    EndWhile
EndFunction

;Remove prefix from akForm's name if it exists
;Requires skse
Function RemovePrefixFromFormName(Form akForm, String Prefix) Global
     If akForm
        ObjectReference Ref = akForm as ObjectReference 
        If Ref 
            Ref.SetDisplayName(RemovePrefixFromString(Ref.GetDisplayName(), Prefix)) 
        Else 
            akForm.SetName(RemovePrefixFromString(akForm.GetName(), Prefix))
        Endif 
    Endif
EndFunction

;Same as above but removes from all form names in array 
;Requires skse
Function RemovePrefixFromFormNames(Form[] akForms, String Prefix, Bool OnlyIfNotPresent = true) Global
    Int i = 0 
    Int L = akForms.Length 
    While i < L 
        RemovePrefixFromFormName(akForms[i], Prefix)
        i += 1 
    EndWhile
EndFunction

;Add Suffix to akForm's name
;If OnlyIfNotPresent == true (default) only adds the Suffix if it's not already present. 
;Requires skse
Function AddSuffixToFormName(Form akForm, String Suffix, Bool OnlyIfNotPresent = true) Global
    If akForm
        ObjectReference Ref = akForm as ObjectReference 
        If Ref 
            Ref.SetDisplayName(AddSuffixToString(Ref.GetDisplayName(), Suffix, OnlyIfNotPresent))
        Else 
            akForm.SetName(AddSuffixToString(akForm.GetName(), Suffix, OnlyIfNotPresent))
        Endif 
    Endif
EndFunction

;Same as above but adds to all form names in array 
;Requires skse
Function AddSuffixToFormNames(Form[] akForms, String Suffix, Bool OnlyIfNotPresent = true) Global
    Int i = 0 
    Int L = akForms.Length 
    While i < L 
        AddSuffixToFormName(akForms[i], Suffix, OnlyIfNotPresent)
        i += 1 
    EndWhile
EndFunction

;Remove Suffix from akForm's name if it exists
;Requires skse
Function RemoveSuffixFromFormName(Form akForm, String Suffix) Global
     If akForm
        ObjectReference Ref = akForm as ObjectReference 
        If Ref 
            Ref.SetDisplayName(RemoveSuffixFromString(Ref.GetDisplayName(), Suffix)) 
        Else 
            akForm.SetName(RemoveSuffixFromString(akForm.GetName(), Suffix))
        Endif 
    EndIf 
EndFunction

;Same as above but removes from all form names in array 
;Requires skse
Function RemoveSuffixFromFormNames(Form[] akForms, String Suffix, Bool OnlyIfNotPresent = true) Global
    Int i = 0 
    Int L = akForms.Length 
    While i < L 
        RemoveSuffixFromFormName(akForms[i], Suffix)
        i += 1 
    EndWhile
EndFunction

;Does the string have the Prefix?
;Requires skse
Bool Function StringHasPrefix(String s, String Prefix) Global
    return (StringUtil.Find(s, Prefix) == 0)
EndFunction

;Does the string have the Suffix?
;Requires skse
Bool Function StringHasSuffix(String s, String Suffix) Global 
     Int i = FindLastStringIndex(s, Suffix)
     return (i == ( StringUtil.GetLength(s) - StringUtil.GetLength(Suffix) ))
EndFunction

; GetStringFromFile get custom string from external file or string
; finds first string between Startkey and EndKey after StringKey.
; similar to localization but no need to worry about nesting strings in a translation file this way.
; Example: Let's say you have a file Data/interface/MyStrings.txt that contains: 
; MyStringA = [My String A] 
; MyStringB = [My String B] 
;
; String MyStringB = GetStringFromFile("MyStringB", FilePath = "Data/interface/MyStrings.txt") ;Returns "My String B"
;
; To search a string instead of a file path, you can do this: 
; String MyStrings 
; MyStrings = MiscUtil.ReadFromFile("Data/interface/MyStrings.txt") 
; String MyStringB = GetStringFromFile("MyStringB", MyStrings) ;Returns "My String B"
; Note, if using the function this way, don't store MyStrings in the script, outside of events or functions. Storing large strings can cause CTD on game load. 
; If storing this way, be sure to clear the string by doing: MyStrings = "" after it's finished being used.
;
; This method will be better for performance if you need to get a lot of strings from your file, as it won't use MiscUtil.ReadFromFile everytime you use the function. 
;
; StringKeys contained in the file must be unique.
;
; If the stringkey wasn't found and the Default is "", it returns the stringKey 
; Let's say you have: 
; Some Custom Message = [Some Custome Message] 
; in the "Data/interface/MyStrings.txt file"
;
; You can do:
; debug.notification(GetStringFromFile("Some Custom Message", FilePath = "Data/interface/MyStrings.txt"))
; And it will still show "Some Custom Message" if something went wrong and it wasn't found in the file
; You can specify a default if you want something else to return if the stringKey wasn't found.
;
; Requires PapyrusUtil && SKSE
String Function GetStringFromFile(String StringKey, String FileContents = "", String FilePath = "", String StartKey = "[", String EndKey = "]", String Default = "", int StartIndex = 0) global 
    If Default == "" 
        Default = StringKey
    Endif

    If FileContents == "" 
        If FilePath != "" 
            If MiscUtil.FileExists(FilePath) == true
                FileContents = MiscUtil.ReadFromFile(FilePath) 
            Else 
                Return Default 
            Endif
        Else  
            Return Default
        Endif 
    Endif
    
    int i = StringUtil.Find(FileContents, StringKey, StartIndex)
    
    If i > -1
        i += StringUtil.GetLength(StringKey)
        i = StringUtil.Find(FileContents, StartKey, i)
        If i > -1
            i += StringUtil.GetLength(StartKey)
            int End = StringUtil.Find(FileContents, EndKey, i)
            If End > -1 
                Return ( StringUtil.SubString(FileContents, i, (End - i)) )
            Endif 
        Endif       
    Endif 
    
    Return Default
EndFunction

;same as GetStringFromFile but returns value as int.
int Function GetIntFromFile(String StringKey, String FileContents = "", String FilePath = "", String StartKey = "[", String EndKey = "]", int Default = -1, int StartIndex = 0) global 
    If FileContents == "" 
        If FilePath != "" 
            If MiscUtil.FileExists(FilePath) == true
                FileContents = MiscUtil.ReadFromFile(FilePath) 
            Else 
                Return Default 
            Endif
        Else  
            Return Default
        Endif 
    Endif

    int i = StringUtil.Find(FileContents, StringKey, StartIndex)
    
    If i > -1
        i += StringUtil.GetLength(StringKey)
        i = StringUtil.Find(FileContents, StartKey, i)
        If i > -1
            i += StringUtil.GetLength(StartKey)
            int End = StringUtil.Find(FileContents, EndKey, i)
            If End > -1 
                Return ( StringUtil.SubString(FileContents, i, (End - i)) ) as int
                ;Substring(String str, int startIndex, int len)
            Endif 
        Endif       
    Endif 
    
    Return Default
EndFunction

;same as GetStringFromFile but returns value as float.
Float Function GetFloatFromFile(String StringKey, String FileContents = "", String FilePath = "", String StartKey = "[", String EndKey = "]", Float Default = -1.0, int StartIndex = 0) global 
    If FileContents == "" 
        If FilePath != "" 
            If MiscUtil.FileExists(FilePath) == true
                FileContents = MiscUtil.ReadFromFile(FilePath) 
            Else 
                Return Default 
            Endif
        Else  
            Return Default
        Endif 
    Endif
    
    int i = StringUtil.Find(FileContents, StringKey, StartIndex)
    
    If i > -1
        i += StringUtil.GetLength(StringKey)
        i = StringUtil.Find(FileContents, StartKey, i)
        If i > -1
            i += StringUtil.GetLength(StartKey)
            int End = StringUtil.Find(FileContents, EndKey, i)
            If End > -1 
                Return ( StringUtil.SubString(FileContents, i, (End - i)) ) as float
                ;Substring(String str, int startIndex, int len)
            Endif 
        Endif       
    Endif 
    
    Return Default
EndFunction


;Save All strings in the StartKey and Endkey brackets, between the RangeStart and RangeEnd strings from the FileContents or FilePath to a string array. 
;Example: you have a file Data/interface/MyStrings.txt that contains: 
;MyStringA = [My String A] 
;MyStringB = [My String B] 
;
;String[] MyStrings = GetAllStringsFromFile("Data/interface/MyStrings.txt")  
;MyStrings[0] will equal "My String A" and MyStrings[1] will equal "My String B"
;
;to specify a range to search you can do this: 
;
;File contains: 
;
;StringsA
;MyStringA = [My String A] 
;MyStringB = [My String B] 
;StringsAEnd 
;MyStringC = [My String C] 
;
;Using String[] MyStrings = GetAllStringsFromFile(FilePath = "Data/interface/MyStrings.txt", "StringsA", "StringsAEnd")
;Only My String A and My String B are saved to the array.
;
;If RangeStart is "", starts search at the beginning of the file. If RangeEnd is "", stops searching at the end of the file. 
;
;requires SKSE and PapyrusUtil
String[] Function GetAllStringsFromFile(String FileContents = "", String FilePath = "", String RangeStart = "", String RangeEnd = "", String StartKey = "[", String EndKey = "]", String[] Default = None) global
   If FileContents == "" 
        If FilePath != "" 
            If MiscUtil.FileExists(FilePath) == true
                FileContents = MiscUtil.ReadFromFile(FilePath) 
            Else 
                Return Default 
            Endif
        Else  
            Return Default
        Endif 
    Endif
    
    int iRangeStart
    If RangeStart == ""
        iRangeStart = 0
    Else 
        iRangeStart = StringUtil.Find(FileContents, RangeStart)
    Endif 
    
    If iRangeStart == -1 
        Return Default
    Else 
        iRangeStart += StringUtil.GetLength(RangeStart)
    Endif
    
    Int iRangeEnd
    If RangeEnd == "" 
        iRangeEnd = StringUtil.GetLength(FileContents) 
    Else 
        iRangeEnd = StringUtil.Find(FileContents, RangeEnd)
    Endif 
    
    If iRangeEnd == -1 
        Return Default 
    Endif
    
    int i = StringUtil.Find(FileContents, StartKey, iRangeStart) ;first count startkeys to determine array size 
    
    If i == -1 
        return Default
    Endif
    
    Int Count = 0
    Int StartKeyLength = StringUtil.GetLength(StartKey)
    
    While i > -1 && i < iRangeEnd
        Count += 1
        i += StartKeyLength ;Advance i to end of StartKey to find next one in fileContents.
        i = StringUtil.Find(FileContents, StartKey, i)
    EndWhile 
    
    If Count == 0 
       Return Default 
    Endif 
    
    String[] sArray = Utility.CreateStringArray(Count) 
    
    i = StringUtil.Find(FileContents, StartKey, iRangeStart)
    Count = 0
    
    While i > -1 && i < iRangeEnd
        i += StartKeyLength
        int End = StringUtil.Find(FileContents, EndKey, i)
        If End > -1 
            sArray[Count] = ( StringUtil.SubString(FileContents, i, (End - i)) )
            Count += 1
        Endif 
        
        i = StringUtil.Find(FileContents, StartKey, i)
    EndWhile 
    
    Return sArray 

EndFunction

;Same as GetAllStringsFromFile but saves values as ints to int array
;requires SKSE and PapyrusUtil
Int[] Function GetAllIntsFromFile(String FileContents = "", String FilePath = "", String RangeStart = "", String RangeEnd = "", String StartKey = "[", String EndKey = "]", Int[] Default = None) global
    If FileContents == "" 
        If FilePath != "" 
            If MiscUtil.FileExists(FilePath) == true
                FileContents = MiscUtil.ReadFromFile(FilePath) 
            Else 
                Return Default 
            Endif
        Else  
            Return Default
        Endif 
    Endif
    
    int iRangeStart
    If RangeStart == ""
        iRangeStart = 0
    Else 
        iRangeStart = StringUtil.Find(FileContents, RangeStart)
    Endif 
    
    If iRangeStart == -1 
        Return Default
    Else 
        iRangeStart += StringUtil.GetLength(RangeStart)
    Endif
    
    Int iRangeEnd
    If RangeEnd == "" 
        iRangeEnd = StringUtil.GetLength(FileContents) 
    Else 
        iRangeEnd = StringUtil.Find(FileContents, RangeEnd)
    Endif 
    
    If iRangeEnd == -1 
        Return Default 
    Endif
    
    int i = StringUtil.Find(FileContents, StartKey, iRangeStart) ;first count startkeys to determine array size 
    
    If i == -1 
        Return Default 
    Endif
    
    Int Count = 0
    Int StartKeyLength = StringUtil.GetLength(StartKey)
    
    While i > -1 && i < iRangeEnd
        Count += 1
        i += StartKeyLength ;Advance i to end of StartKey to find next one in fileContents.
        i = StringUtil.Find(FileContents, StartKey, i)
    EndWhile 
    
    If Count == 0 
       Return Default 
    Endif 
    
    Int[] iArray = Utility.CreateIntArray(Count) 
    
    i = StringUtil.Find(FileContents, StartKey, iRangeStart)
    Count = 0
    
    While i > -1 && i < iRangeEnd
        i += StartKeyLength
        int End = StringUtil.Find(FileContents, EndKey, i)
        If End > -1 
            iArray[Count] = ( StringUtil.SubString(FileContents, i, (End - i)) ) as int
            Count += 1
        Endif 
        
        i = StringUtil.Find(FileContents, StartKey, i)
    EndWhile 
    
    Return iArray 

EndFunction

;Same as GetAllStringsFromFile but saves values as floats to float array
;requires SKSE and PapyrusUtil
Float[] Function GetAllFloatsFromFile(String FileContents = "", String FilePath = "", String RangeStart = "", String RangeEnd = "", String StartKey = "[", String EndKey = "]", Float[] Default = None) global
    If FileContents == "" 
        If FilePath != "" 
            If MiscUtil.FileExists(FilePath) == true
                FileContents = MiscUtil.ReadFromFile(FilePath) 
            Else 
                Return Default 
            Endif
        Else  
            Return Default
        Endif 
    Endif
    
    int iRangeStart
    If RangeStart == ""
        iRangeStart = 0
    Else 
        iRangeStart = StringUtil.Find(FileContents, RangeStart)
    Endif 
    
    If iRangeStart == -1 
        Return Default
    Else 
        iRangeStart += StringUtil.GetLength(RangeStart)
    Endif
    
    Int iRangeEnd
    If RangeEnd == "" 
        iRangeEnd = StringUtil.GetLength(FileContents) 
    Else 
        iRangeEnd = StringUtil.Find(FileContents, RangeEnd)
    Endif 
    
    If iRangeEnd == -1 
        Return Default 
    Endif
    
    int i = StringUtil.Find(FileContents, StartKey, iRangeStart) ;first count startkeys to determine array size 
    If i == -1 
        Return Default 
    Endif
    
    Int Count = 0
    Int StartKeyLength = StringUtil.GetLength(StartKey)
    
    While i > -1 && i < iRangeEnd
        Count += 1
        i += StartKeyLength ;Advance i to end of StartKey to find next one in fileContents.
        i = StringUtil.Find(FileContents, StartKey, i)
    EndWhile 
    
    If Count == 0 
       Return Default 
    Endif 
    
    Float[] fArray = Utility.CreateFloatArray(Count) 
    
    i = StringUtil.Find(FileContents, StartKey, iRangeStart)
    Count = 0
    
    While i > -1 && i < iRangeEnd
        i += StartKeyLength
        int End = StringUtil.Find(FileContents, EndKey, i)
        If End > -1 
            fArray[Count] = ( StringUtil.SubString(FileContents, i, (End - i)) ) as float
            Count += 1
        Endif 
        
        i = StringUtil.Find(FileContents, StartKey, i)
    EndWhile 
    
    Return fArray 

EndFunction


;PrintStringKeysToFile Finds and Prints all GetString / int / floatFromString StringKeys, from FilePathToSearch to FilePathToPrintTo
;To speed up the process of making your StringKeys.txt file. 
;You can write GetStringFromFile() functions in your .psc file, and when you're finished with your script, have this function print the string keys to another .txt file. 
;
;Example, if in MyScript.psc you have: 
;Debug.Notification(GetStringFromFile("My Message A")) 
;Debug.Notification(GetStringFromFile("My Message B")) 
;Use the function: 
;
;PrintStringKeysToFile("Data/Scripts/Source/MyScript.psc", "Data/interface/MyStrings.txt") 
;
;In the MyStrings.txt file it will write: 
;
;"My Message A" = ["My Message A"]
;"My Message B" = ["My Message B"]
;
;note that the quotes are included. You'll want to get rid of them by pressing ctrl H in your text editor and replace all " with nothing so it looks like: 
;
;My Message A = [My Message A]
;My Message B = [My Message B]
;
;Otherwise the GetStringFromFile functions won't work correctly when reading from your .txt file.
;
;Requires SKSE and PapyrusUtil
Bool Function PrintStringKeysToFile(String FilePathToSearch, String FilePathToPrintTo, String StartKey = "[", String EndKey = "]", String FinishedMsg = "Done Printing") global
    If MiscUtil.FileExists(FilePathToSearch) == False 
        return False
    Endif
    
    string FileContents = MiscUtil.ReadFromFile(FilePathToSearch)
    
    int M = StringUtil.GetLength(FileContents)
    
    Utility.WaitMenuMode(0.01)
    
    String StringKeys
    
    int I = StringUtil.Find(FileContents, "GetStringFromFile")
   
    While I > -1 
        int NextLine = StringUtil.Find(FileContents, "\n", (I + 1)) 
        If NextLine == -1 
            NextLine = M
        Endif
    
        int akStart = StringUtil.Find(FileContents, "(", I) 
        
        If akStart > -1 && akStart < NextLine ;clamp to current line
            akStart += 1
            int akEnd = StringUtil.Find(FileContents, ",", akStart)
            If akEnd > -1 && akEnd < NextLine
                String StringKey = StringUtil.SubString(FileContents, akStart, (akEnd - akStart))
                
                If StringKey != "" && StringUtil.Find(StringKeys, StringKey) == -1 ;string not already printed, fine to continue. 
                    
                    MiscUtil.WriteToFile(FilePathToPrintTo, (StringKey + " = " + StartKey + StringKey + EndKey + "\n"))
                    StringKeys += StringKey + "||" ;save stringKey to StringKeys to prevent duplicates
                Endif 
            Endif 
        Endif 
     
        I += StringUtil.GetLength("GetStringFromFile") ;advance I to end of GetStringFromFile
        I = StringUtil.Find(FileContents, "GetStringFromFile", I) ;find next GetStringFromFile in fileContents
    EndWhile
    
    ;GetintFromFile==================================================================================
    I = StringUtil.Find(FileContents, "GetintFromFile")
   
    While I > -1 
        int NextLine = StringUtil.Find(FileContents, "\n", (I + 1)) 
        If NextLine == -1 
            NextLine = M
        Endif
    
        int akStart = StringUtil.Find(FileContents, "(", I) 
        
        If akStart > -1 && akStart < NextLine ;clamp to current line
            akStart += 1
            int akEnd = StringUtil.Find(FileContents, ",", akStart)
            If akEnd > -1 && akEnd < NextLine
                String StringKey = StringUtil.SubString(FileContents, akStart, (akEnd - akStart))
                
                If StringKey != "" && StringUtil.Find(StringKeys, StringKey) == -1 ;string not already printed, fine to continue. 
                    
                    MiscUtil.WriteToFile(FilePathToPrintTo, (StringKey + " = " + StartKey + StringKey + EndKey + "\n"))
                    StringKeys += StringKey + "||" ;save stringKey to StringKeys to prevent duplicates
                Endif 
            Endif 
        Endif 
     
        I += StringUtil.GetLength("GetintFromFile") ;advance I to end of GetintFromFile
        I = StringUtil.Find(FileContents, "GetintFromFile", I) ;find next GetintFromFile in fileContents
    EndWhile
    
    ;GetFloatFromFile==================================================================================
    I = StringUtil.Find(FileContents, "GetFloatFromFile")
   
    While I > -1 
        int NextLine = StringUtil.Find(FileContents, "\n", (I + 1)) 
        If NextLine == -1 
            NextLine = M
        Endif
    
        int akStart = StringUtil.Find(FileContents, "(", I) 
        
        If akStart > -1 && akStart < NextLine ;clamp to current line
            akStart += 1
            int akEnd = StringUtil.Find(FileContents, ",", akStart)
            If akEnd > -1 && akEnd < NextLine
                String StringKey = StringUtil.SubString(FileContents, akStart, (akEnd - akStart))
                
                If StringKey != "" && StringUtil.Find(StringKeys, StringKey) == -1 ;string not already printed, fine to continue. 
                    
                    MiscUtil.WriteToFile(FilePathToPrintTo, (StringKey + " = " + StartKey + StringKey + EndKey + "\n"))
                    StringKeys += StringKey + "||" ;save stringKey to StringKeys to prevent duplicates
                Endif 
            Endif 
        Endif 
     
        I += StringUtil.GetLength("GetFloatFromFile") ;advance I to end of GetFloatFromFile
        I = StringUtil.Find(FileContents, "GetFloatFromFile", I) ;find next GetFloatFromFile in fileContents
    EndWhile
    
    If FinishedMsg != ""
        Debug.MessageBox("Done Printing")
    Endif
    
    Return True
EndFunction


;Print all items in a container to the FilePath with the mod they come from included. 
;Requires SKSE
Function PrintContainerItemsToFile(ObjectReference akContainer, String FilePath, String ConfirmMessage = "") global
    ;Debug.OpenUserLog(LogFileName)
    
    String[] HexDigits = New String[16]
    HexDigits[0] = "0"
    HexDigits[1] = "1"
    HexDigits[2] = "2"
    HexDigits[3] = "3"
    HexDigits[4] = "4"
    HexDigits[5] = "5"
    HexDigits[6] = "6"
    HexDigits[7] = "7"
    HexDigits[8] = "8"
    HexDigits[9] = "9"
    HexDigits[10] = "a"
    HexDigits[11] = "b"
    HexDigits[12] = "c"
    HexDigits[13] = "d"
    HexDigits[14] = "e"
    HexDigits[15] = "f"
        
    int M = akContainer.GetNumItems()
    MiscUtil.WriteToFile(FilePath, akContainer.GetDisplayName() + " Items:" + "\n")
        
    While M > 0
        M -= 1 
        Form Item = akContainer.GetNthForm(M)
        int DecId = Item.GetFormId()
        String HexId = ConvertintToHex(DecId)
        String DecIdS = DecId as String
        String hexIdB = StringUtil.Substring(HexId, 0, 2)
        String hexIdC = ("0x" + StringUtil.Substring(HexId, 2, -1))
        
        int LO_Index = (HexDigits.Find(StringUtil.GetNthChar(hexIdB, 0)) * 16) + hexDigits.Find(StringUtil.GetNthChar(hexIdB, 1))
        
        String ModName = "\"" + Game.GetModName(LO_Index) + "\""
        
        ;String ContainerName = akContainer.GetDisplayName()
        ;Debug.TraceUser(LogFileName, ContainerName + " Items:")
        ;Debug.TraceUser(LogFileName, "ModName: " + ModName + " ID: " + hexIdC)
        MiscUtil.WriteToFile(FilePath, "ModName: [" + ModName + "]  [" + Item + "]")
        
    EndWhile
    
    ;Debug.CloseUserLog(LogFileName)
    
    If ConfirmMessage != ""
        Debug.MessageBox(ConfirmMessage)
    Endif
EndFunction

;Write form ID's of forms in akList to file. If ReplaceIdStartWith0x == true (default), replaces first two characters of ID with 0x.
;Requires skse and PapyrusUtil
Function WriteIDsInFormListToFile(Formlist akList, String FilePath, Bool IncludeNames = False, Bool ReplaceIdStartWith0x = true) Global
    Int i = 0 
    Int L = akList.GetSize() 
    
    If IncludeNames && ReplaceIdStartWith0x
        While i < L 
            Form akForm = akList.GetAt(i)
            MiscUtil.WriteToFile(FilePath, akForm.GetName() + " 0x" + StringUtil.SubString(ConvertIntToHex(akForm.GetFormId()), 2) + "\n")
            i += 1
        EndWhile 
        
    ElseIf IncludeNames
        While i < L 
            Form akForm = akList.GetAt(i)
            MiscUtil.WriteToFile(FilePath, akForm.GetName() + " " + ConvertIntToHex(akForm.GetFormId()) + "\n")
            i += 1
        EndWhile 
        
    Elseif ReplaceIdStartWith0x
        While i < L 
            Form akForm = akList.GetAt(i)
            MiscUtil.WriteToFile(FilePath, "0x" + StringUtil.SubString(ConvertIntToHex(akForm.GetFormId()), 2) + "\n")
            i += 1
        EndWhile 
        
    Else
        While i < L 
            Form akForm = akList.GetAt(i)
            MiscUtil.WriteToFile(FilePath, ConvertIntToHex(akForm.GetFormId()) + "\n")
            i += 1
        EndWhile 
    Endif
EndFunction

;Write form ID's of forms in akList to file. If ReplaceIdStartWith0x == true (default), replaces first two characters of ID with 0x.
;Requires skse and PapyrusUtil
Function WriteIDsInFormArrayToFile(Form[] akList, String FilePath, Bool IncludeNames = False, Bool ReplaceIdStartWith0x = true) Global
    Int i = 0 
    Int L = akList.Length 
    
    If IncludeNames && ReplaceIdStartWith0x
        While i < L 
            Form akForm = akList[i]
            MiscUtil.WriteToFile(FilePath, akForm.GetName() + " 0x" + StringUtil.SubString(ConvertIntToHex(akForm.GetFormId()), 2) + "\n")
            i += 1
        EndWhile 
        
    ElseIf IncludeNames
        While i < L 
            Form akForm = akList[i]
            MiscUtil.WriteToFile(FilePath, akForm.GetName() + " " + ConvertIntToHex(akForm.GetFormId()) + "\n")
            i += 1
        EndWhile 
        
    Elseif ReplaceIdStartWith0x
        While i < L 
            Form akForm = akList[i]
            MiscUtil.WriteToFile(FilePath, "0x" + StringUtil.SubString(ConvertIntToHex(akForm.GetFormId()), 2) + "\n")
            i += 1
        EndWhile 
        
    Else
        While i < L 
            Form akForm = akList[i]
            MiscUtil.WriteToFile(FilePath, ConvertIntToHex(akForm.GetFormId()) + "\n")
            i += 1
        EndWhile 
    Endif
EndFunction

;Write form ID's of forms in storageutil formlist to file. If ReplaceIdStartWith0x == true (default), replaces first two characters of ID with 0x.
;Requires skse and PapyrusUtil
Function WriteIDsInStorageUtilListToFile(Form ObjKey, String ListKeyName, String FilePath, Bool IncludeNames = False, Bool ReplaceIdStartWith0x = true) Global 
    Int i = 0 
    Int L = StorageUtil.FormListCount(ObjKey, ListKeyName)
    
    If IncludeNames && ReplaceIdStartWith0x
        While i < L 
            Form akForm = StorageUtil.FormListGet(ObjKey, ListKeyName, i)
            MiscUtil.WriteToFile(FilePath, akForm.GetName() + " 0x" + StringUtil.SubString(ConvertIntToHex(akForm.GetFormId()), 2) + "\n")
            i += 1
        EndWhile 
        
    ElseIf IncludeNames
        While i < L 
            Form akForm = StorageUtil.FormListGet(ObjKey, ListKeyName, i)
            MiscUtil.WriteToFile(FilePath, akForm.GetName() + " " + ConvertIntToHex(akForm.GetFormId()) + "\n")
            i += 1
        EndWhile 
        
    Elseif ReplaceIdStartWith0x
        While i < L 
            Form akForm = StorageUtil.FormListGet(ObjKey, ListKeyName, i)
            MiscUtil.WriteToFile(FilePath, "0x" + StringUtil.SubString(ConvertIntToHex(akForm.GetFormId()), 2) + "\n")
            i += 1
        EndWhile 
        
    Else
        While i < L 
            Form akForm = StorageUtil.FormListGet(ObjKey, ListKeyName, i)
            MiscUtil.WriteToFile(FilePath, ConvertIntToHex(akForm.GetFormId()) + "\n")
            i += 1
        EndWhile 
    Endif
EndFunction

;Write form ID's of forms in JsonUtil formlist to file. If ReplaceIdStartWith0x == true (default), replaces first two characters of ID with 0x.
;Requires skse and PapyrusUtil
Function WriteIDsInJsonUtilListToFile(String JsonFilePath, String ListKeyName, String FilePath, Bool IncludeNames = False, Bool ReplaceIdStartWith0x = true) Global 
    Int i = 0 
    Int L = JsonUtil.FormListCount(JsonFilePath, ListKeyName)
    
    If IncludeNames && ReplaceIdStartWith0x
        While i < L 
            Form akForm = JsonUtil.FormListGet(JsonFilePath, ListKeyName, i)
            MiscUtil.WriteToFile(FilePath, akForm.GetName() + " 0x" + StringUtil.SubString(ConvertIntToHex(akForm.GetFormId()), 2) + "\n")
            i += 1
        EndWhile 
        
    ElseIf IncludeNames
        While i < L 
            Form akForm = JsonUtil.FormListGet(JsonFilePath, ListKeyName, i)
            MiscUtil.WriteToFile(FilePath, akForm.GetName() + " " + ConvertIntToHex(akForm.GetFormId()) + "\n")
            i += 1
        EndWhile 
        
    Elseif ReplaceIdStartWith0x
        While i < L 
            Form akForm = JsonUtil.FormListGet(JsonFilePath, ListKeyName, i)
            MiscUtil.WriteToFile(FilePath, "0x" + StringUtil.SubString(ConvertIntToHex(akForm.GetFormId()), 2) + "\n")
            i += 1
        EndWhile 
        
    Else
        While i < L 
            Form akForm = JsonUtil.FormListGet(JsonFilePath, ListKeyName, i)
            MiscUtil.WriteToFile(FilePath, ConvertIntToHex(akForm.GetFormId()) + "\n")
            i += 1
        EndWhile 
    Endif
EndFunction 

;Write bool animation variables of akRef found in DbAnimationVariableBools.txt to OutputFilePath.
;Can specify a different VariablesSourceFilePath if desired.
;Default variables found in DbAnimationVariableBools.txt are from https://www.creationkit.com/index.php?title=List_of_Animation_Variables
;Requires skse and papyrusutil
Function WriteAnimationVariableBoolsToFile(ObjectReference akRef, String OutputFilePath, String VariablesSourceFilePath = "Data/interface/DbMiscFunctions/DbAnimationVariableBools.txt") Global
    String[] Variables = StringUtil.Split(MiscUtil.ReadFromFile(VariablesSourceFilePath), "\n")
    Int i = 0 
    Int L = Variables.length 
    While i < L 
        MiscUtil.WriteToFile(OutputFilePath, Variables[i] + " = " + akRef.GetAnimationVariableBool(Variables[i]) + "\n")
        i += 1
    EndWhile 
Endfunction

;Write int animation variables of akRef found in DbAnimationVariableInts.txt to OutputFilePath.
;Can specify a different VariablesSourceFilePath if desired.
;Default variables found in DbAnimationVariableInts.txt are from https://www.creationkit.com/index.php?title=List_of_Animation_Variables
;Requires skse and papyrusutil
Function WriteAnimationVariableIntsToFile(ObjectReference akRef, String OutputFilePath, String VariablesSourceFilePath = "Data/interface/DbMiscFunctions/DbAnimationVariableInts.txt") Global
    String[] Variables = StringUtil.Split(MiscUtil.ReadFromFile(VariablesSourceFilePath), "\n")
    Int i = 0 
    Int L = Variables.length 
    While i < L 
        MiscUtil.WriteToFile(OutputFilePath, Variables[i] + " = " + akRef.GetAnimationVariableInt(Variables[i]) + "\n")
        i += 1
    EndWhile 
Endfunction

;Write Float animation variables of akRef found in DbAnimationVariableFloats.txt to OutputFilePath.
;Can specify a different VariablesSourceFilePath if desired.
;Default variables found in DbAnimationVariableFloats.txt are from https://www.creationkit.com/index.php?title=List_of_Animation_Variables
;Requires skse and papyrusutil
Function WriteAnimationVariableFloatsToFile(ObjectReference akRef, String OutputFilePath, String VariablesSourceFilePath = "Data/interface/DbMiscFunctions/DbAnimationVariableFloats.txt") Global
    String[] Variables = StringUtil.Split(MiscUtil.ReadFromFile(VariablesSourceFilePath), "\n")
    Int i = 0 
    Int L = Variables.length 
    While i < L 
        MiscUtil.WriteToFile(OutputFilePath, Variables[i] + " = " + akRef.GetAnimationVariableFloat(Variables[i]) + "\n")
        i += 1
    EndWhile 
Endfunction

;Requires skse and papyrusutil
Function WriteAllAnimationVariablesToFile(ObjectReference akRef, String OutputFilePath, String BoolVariablesSourceFilePath = "Data/interface/DbMiscFunctions/DbAnimationVariableBools.txt", \
                                                                                String IntVariablesSourceFilePath = "Data/interface/DbMiscFunctions/DbAnimationVariableInts.txt", \
                                                                                String FloatVariablesSourceFilePath = "Data/interface/DbMiscFunctions/DbAnimationVariableFloats.txt") Global

    WriteAnimationVariableBoolsToFile(akRef, OutputFilePath, BoolVariablesSourceFilePath)
    WriteAnimationVariableIntsToFile(akRef, OutputFilePath, IntVariablesSourceFilePath)
    WriteAnimationVariableFloatsToFile(akRef, OutputFilePath, FloatVariablesSourceFilePath)
EndFunction 

;register the akForm to recieve all AnimationEvents from the akSender  
;no requirements
Function RegisterFormForAnimationEvents(Form akForm, ObjectReference akSender, String[] AnimationEvents) Global
    Int i = 0 
    Int L = AnimationEvents.Length
    While i < L 
        akForm.RegisterForAnimationEvent(akSender, AnimationEvents[i])
        i += 1
    EndWhile
EndFunction

;register the akAlias to recieve all AnimationEvents from the akSender  
;no requirements
Function RegisterAliasForAnimationEvents(Alias akAlias, ObjectReference akSender, String[] AnimationEvents) Global
    Int i = 0 
    Int L = AnimationEvents.Length
    While i < L 
        akAlias.RegisterForAnimationEvent(akSender, AnimationEvents[i])
        i += 1
    EndWhile
EndFunction

;register the akActiveMagicEffect to recieve all AnimationEvents from the akSender  
;no requirements
Function RegisterActiveMagicEffectForAnimationEvents(ActiveMagicEffect akActiveMagicEffect, ObjectReference akSender, String[] AnimationEvents) Global
    Int i = 0 
    Int L = AnimationEvents.Length
    While i < L 
        akActiveMagicEffect.RegisterForAnimationEvent(akSender, AnimationEvents[i])
        i += 1
    EndWhile
EndFunction

;Register the akForm to recieve all Animation Events from akSender found in File specified by FilePath. 
;Events in the file should be separated by new line.
;Requires SKSE and PapyrusUtil
Function RegisterFormForAnimationEventsFromFile(Form akForm, ObjectReference akSender, String FilePath = "Data/interface/DbMiscFunctions/DbAnimationEvents.txt") Global
    String[] AnimationEvents = PapyrusUtil.StringSplit(MiscUtil.ReadFromFile(FilePath), "\n")
    RegisterFormForAnimationEvents(akForm, akSender, AnimationEvents)
EndFunction

;Register the akAlias to recieve all Animation Events from akSender found in File specified by FilePath. 
;Events in the file should be separated by new line.
;Requires SKSE and PapyrusUtil
Function RegisterAliasForAnimationEventsFromFile(Alias akAlias, ObjectReference akSender, String FilePath = "Data/interface/DbMiscFunctions/DbAnimationEvents.txt") Global
    String[] AnimationEvents = PapyrusUtil.StringSplit(MiscUtil.ReadFromFile(FilePath), "\n")
    RegisterAliasForAnimationEvents(akAlias, akSender, AnimationEvents)
EndFunction

;Register the akActiveMagicEffect to recieve all Animation Events from akSender found in File specified by FilePath. 
;Events in the file should be separated by new line.
;Requires SKSE and PapyrusUtil
Function RegisterActiveMagicEffectForAnimationEventsFromFile(ActiveMagicEffect akActiveMagicEffect, ObjectReference akSender, String FilePath = "Data/interface/DbMiscFunctions/DbAnimationEvents.txt") Global
    String[] AnimationEvents = PapyrusUtil.StringSplit(MiscUtil.ReadFromFile(FilePath), "\n")
    RegisterActiveMagicEffectForAnimationEvents(akActiveMagicEffect, akSender, AnimationEvents)
EndFunction

;register the akForm for all Menus in string array.
;Requires SKSE
Function RegisterFormForMenus(Form akForm, String[] Menus) Global
    Int i = 0 
    Int L = Menus.Length
    While i < L 
        akForm.RegisterForMenu(Menus[i])
        i += 1
    EndWhile
EndFunction

;register the akAlias for all Menus in string array.
;Requires SKSE
Function RegisterAliasForMenus(Alias akAlias, String[] Menus) Global
    Int i = 0 
    Int L = Menus.Length
    While i < L 
        akAlias.RegisterForMenu(Menus[i])
        i += 1
    EndWhile
EndFunction

;register the akActiveMagicEffect for all Menus in string array.
;Requires SKSE
Function RegisterActiveMagicEffectForMenus(ActiveMagicEffect akActiveMagicEffect, String[] Menus) Global
    Int i = 0 
    Int L = Menus.Length
    While i < L 
        akActiveMagicEffect.RegisterForMenu(Menus[i])
        i += 1
    EndWhile
EndFunction

;Register the akForm for all menus found in File specified by FilePath. 
;Events in the file should be separated by new line.
;Requires SKSE and PapyrusUtil
Function RegisterFormForMenusFromFile(Form akForm, String FilePath = "Data/interface/DbMiscFunctions/DbMenus.txt") Global
    String[] Menus = PapyrusUtil.StringSplit(MiscUtil.ReadFromFile(FilePath), "\n")
    RegisterFormForMenus(akForm, Menus)
EndFunction

;Register the akAlias for all menus found in File specified by FilePath. 
;Events in the file should be separated by new line.
;Requires SKSE and PapyrusUtil
Function RegisterAliasForMenusFromFile(Alias akAlias, String FilePath = "Data/interface/DbMiscFunctions/DbMenus.txt") Global
    String[] Menus = PapyrusUtil.StringSplit(MiscUtil.ReadFromFile(FilePath), "\n")
    RegisterAliasForMenus(akAlias, Menus)
EndFunction

;Register the ActiveMagicEffect for all menus found in File specified by FilePath. 
;Events in the file should be separated by new line.
;Requires SKSE and PapyrusUtil
Function RegisterActiveMagicEffectForMenusFromFile(ActiveMagicEffect akActiveMagicEffect, String FilePath = "Data/interface/DbMiscFunctions/DbMenus.txt") Global
    String[] Menus = PapyrusUtil.StringSplit(MiscUtil.ReadFromFile(FilePath), "\n")
    RegisterActiveMagicEffectForMenus(akActiveMagicEffect, Menus)
EndFunction

;Write all data info to TargetFilePath from all .psc files in SearchFolderPath. Writes ScriptNames, Function Names, Event Names, Function Definitions and Event Definitions.
;requires skse and papyrus util.
function WriteAllPscDataInFolderToFile(String SearchFolderPath, String TargetFilePath, String Divider = "\n", String DoneMessage = "Done Writing") Global    
    String akScriptNames 
    String akEventNames 
    String akFunctionNames
    String akEventDefinitions 
    String akFunctionDefinitions 
    
    String[] Files = MiscUtil.FilesInFolder(SearchFolderPath, ".psc") ;get all psc files in folder 
    Int L = Files.length 
    Int i = 0
    Debug.Notification("Num of files = " +  L)
    
    While i < L
        String akScriptName
        Int p = StringUtil.Find(Files[i], ".")
        If p > -1
            akScriptName = StringUtil.SubString(Files[i], 0, p)
        Else 
            akScriptName = Files[i] 
        endif 
        
        akScriptNames += (akScriptName + Divider)
        akEventNames += (akScriptName + ": \n" + GetPscEventNamesFromFile(SearchFolderPath + "/" + Files[i], Divider) + "\n\n")
        akFunctionNames += (akScriptName + ": \n" + GetPscFunctionNamesFromFile(SearchFolderPath + "/" + Files[i], Divider) + "\n\n")
        akEventDefinitions += (akScriptName + ": \n" + GetPscEventDefinitionsFromFile(SearchFolderPath + "/" + Files[i], Divider) + "\n\n")
        akFunctionDefinitions += (akScriptName + ": \n" + GetPscFunctionDefinitionsFromFile(SearchFolderPath + "/" + Files[i], Divider) + "\n\n")
        
        If i % 10 == 0
            Debug.Notification(i)
        Endif
        i += 1
    EndWhile
    
    MiscUtil.WriteToFile(TargetFilePath, "Script Names: =======================================================\n" + akScriptNames + "\n\n")
    MiscUtil.WriteToFile(TargetFilePath, "Event Names: ========================================================\n" + akEventNames)
    MiscUtil.WriteToFile(TargetFilePath, "Function Names: =====================================================\n" + akFunctionNames)
    MiscUtil.WriteToFile(TargetFilePath, "Event Definitions: ==================================================\n" + akEventDefinitions)
    MiscUtil.WriteToFile(TargetFilePath, "Function Definitions: ===============================================\n" + akFunctionDefinitions)
    
    akScriptNames = ""
    akEventNames = ""
    akFunctionNames = ""
    akEventDefinitions = ""
    akFunctionDefinitions = ""
    
    If DoneMessage != ""
        Debug.MessageBox(DoneMessage)
    Endif
EndFunction 

String Function GetPscEventNamesFromFile(String SourceFilePath, String Divider = "\n", int StartIndex = 0) Global
    return GetPscDataNamesFromFile(SourceFilePath, "Event", Divider, StartIndex)
EndFunction

String Function GetPscFunctionNamesFromFile(String SourceFilePath, String Divider = "\n", int StartIndex = 0) Global
    return GetPscDataNamesFromFile(SourceFilePath, "Function", Divider, StartIndex)
EndFunction

String Function GetPscDataNamesFromFile(String SourceFilePath, String NameType, String Divider = "\n", int StartIndex = 0) Global
    String ReturnString = ""
    String Contents = MiscUtil.ReadFromFile(SourceFilePath) 
    int i
    int L
    
    i = DbMiscFunctions.FindWholeWordString(Contents, NameType)
    L = StringUtil.GetLength(Contents)
    int sL = StringUtil.GetLength(NameType)
    
    While i > -1 
        i += sL 
        
        Int LineStart = i - 1
        String Char = StringUtil.GetNthChar(Contents, LineStart)
        While LineStart > 0 && Char != "\n" 
            LineStart -= 1 
            Char = StringUtil.GetNthChar(Contents, LineStart)
        EndWhile 
        
        Int LineEnd = StringUtil.Find(Contents, "\n", i) 
        If LineEnd == -1 
            LineEnd = L
        Endif 
        
        int NextBlockCommentStart = StringUtil.Find(Contents, ";/", i)
        int NextBlockCommentEnd   = StringUtil.Find(Contents, "/;", i)
        
        If (NextBlockCommentStart < NextBlockCommentEnd && NextBlockCommentStart > -1) || NextBlockCommentEnd == -1
            int NextComment = StringUtil.Find(Contents, ";", LineStart)
            Int iEnd = StringUtil.Find(Contents, "(", i)
            
            If iEnd > -1 && iEnd < LineEnd
                int iStart = iEnd
                Char = StringUtil.GetNthChar(Contents, iStart) 
                While iStart > LineStart && DbMiscFunctions.IsCharWhiteSpace(Char) == False 
                    iStart -= 1 
                    Char = StringUtil.GetNthChar(Contents, iStart)
                EndWhile 
                
                If DbMiscFunctions.IsCharWhiteSpace(Char) 
                    iStart += 1 
                Endif
                
                If NextComment > iEnd || NextComment == -1
                    
                    String Name = (StringUtil.SubString(Contents, iStart, (iEnd - iStart)) + Divider)
                    ReturnString += Name
                Endif
            Endif
        Endif
        
        i += sL
        i = DbMiscFunctions.FindWholeWordString(Contents, NameType, i)
    EndWhile
    
    Return ReturnString
EndFunction 


;/Get all Event or Function Definitions from source file.  
ignores commented out functions
example if the psc file has: 

Int Function AddInts(int A, int B)
    return A + B
Endfunction

Float Function AddFloats(Float A, Float B)
    return A + B 
EndFunction

GetPscFunctionDefinitionsFromFile returns: 

"Int Function AddInts(int A, int B)
Float Function AddFloats(Float A, Float B)"
/;

String Function GetPscEventDefinitionsFromFile(String SourceFilePath, String Divider = "\n", int StartIndex = 0) Global 
    return GetPscDataDefinitionsFromFile(SourceFilePath, "Event", Divider, StartIndex)
EndFunction

String Function GetPscFunctionDefinitionsFromFile(String SourceFilePath, String Divider = "\n", int StartIndex = 0) Global 
    return GetPscDataDefinitionsFromFile(SourceFilePath, "Function", Divider, StartIndex)
EndFunction

String Function GetPscDataDefinitionsFromFile(String SourceFilePath, String NameType, String Divider = "\n", int StartIndex = 0) Global
    String ReturnString = ""
    String Contents = MiscUtil.ReadFromFile(SourceFilePath) 
    int i
    int L
    
    i = DbMiscFunctions.FindWholeWordString(Contents, NameType)
    L = StringUtil.GetLength(Contents)
    int sL = StringUtil.GetLength(NameType)
    
    While i > -1 
        
        Int LineStart = i - 1
        String Char = StringUtil.GetNthChar(Contents, LineStart)
        While LineStart > 0 && Char != "\n" 
            LineStart -= 1 
            Char = StringUtil.GetNthChar(Contents, LineStart)
        EndWhile 
        
        Int LineEnd = StringUtil.Find(Contents, "\n", i) 
        If LineEnd == -1 
            LineEnd = L
        Endif 
        
        int iStart = i - 1 
        Char = StringUtil.GetNthChar(Contents, iStart)
        While iStart > LineStart && DbMiscFunctions.IsCharWhiteSpace(Char) 
            iStart -= 1
            Char = StringUtil.GetNthChar(Contents, iStart)
        EndWhile 
        
        While iStart > LineStart && DbMiscFunctions.IsCharWhiteSpace(Char) == false
            iStart -= 1
            Char = StringUtil.GetNthChar(Contents, iStart)
        EndWhile 
        
        If DbMiscFunctions.IsCharWhiteSpace(Char) 
            iStart += 1 
        Endif
        
        int NextBlockCommentStart = StringUtil.Find(Contents, ";/", iStart)
        int NextBlockCommentEnd   = StringUtil.Find(Contents, "/;", iStart)
        
        If (NextBlockCommentStart < NextBlockCommentEnd && NextBlockCommentStart > -1) || NextBlockCommentEnd == -1
            int NextComment = StringUtil.Find(Contents, ";", LineStart)
            Int iEnd = StringUtil.Find(Contents, ")", iStart)
            
            If iEnd > -1
                If NextComment > iEnd || NextComment == -1
                    iEnd += 1
                    String Definition = (StringUtil.SubString(Contents, iStart, (iEnd - iStart)) + Divider)
                    ReturnString += Definition
                Endif
            Endif
        Endif
        
        i += sL
        i = DbMiscFunctions.FindWholeWordString(Contents, NameType, i)
    EndWhile
    
    Return ReturnString
EndFunction 

;Search the SournceFilePath for String / Int / Float / Global variables, if their toggles are enabled, (outside of any events or functions) 
;and write Json Save and Load functions to DestinationFilePath for said variables. 
;If DestinationFilePath == "" it will write the functions to the SourceFilePath.
;Set Messages int to 0 to display ConfirmMessage notification when finished, set to 1 to display messagebox.
;requires skse and PapyrusUtil
Function WriteJsonSaveAndLoadFunctionsToFile(String SourceFilePath, String DestinationFilePath = "", \
    Bool GlobalVariablesToggle = true, Bool FloatsToggle = true, Bool StringsToggle = true, Bool IntsToggle = true, Bool BoolsToggle = true, \
    Bool GlobalVariableArraysToggle = true, Bool FloatArraysToggle = true, Bool StringArraysToggle = true, Bool IntArraysToggle = true, Bool BoolArraysToggle = true, \
    int Messages = 0, String ConfirmMessage = "Done Writing Json Functions", Bool UsePropertiesAsDefaults = True) global
 
    If DestinationFilePath == "" 
        DestinationFilePath = SourceFilePath
    Endif
    
    String FileContents = MiscUtil.ReadFromFile(SourceFilePath)
    
    Int ContentLength = StringUtil.GetLength(FileContents) 
    
    ;MiscUtil.WriteToFile(DestinationFilePath, "\n Function SaveMCMSettings(Bool Messages)")
   
    String GlobalVariables
    String Strings
    String Floats
    String Bools
    String Ints
    
    String GlobalVariableArrays
    String StringArrays
    String FloatArrays
    String BoolArrays
    String IntArrays
    
    ;find and save all properties in SourceFilePath to above Strings=====================================================================
    Bool Search = True 
    Int I = 0
    Int ArrayIndex = 0
    While Search == True && I < ContentLength 
        Int GlobIndex = StringUtil.Find(FileContents, "\n", I) ;find next line
        If GlobIndex > -1 && I < ContentLength
            I = (GlobIndex + 1)
            ;Debug.MessageBox("I = " + I)
            ;Utility.Wait(0.5)
            ;
            String Char = StringUtil.GetNthChar(FileContents, I) 
            While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find first actual character of new line 
                I += 1
                Char = StringUtil.GetNthChar(FileContents, I)
            EndWhile 
            
            String FirstWord = StringUtil.SubString(FileContents, I, 14) 
            
            String StringCheck = StringUtil.SubString(FirstWord, 0, 6)
            String FloatCheck = StringUtil.SubString(FirstWord, 0, 5)
            String BoolCheck = StringUtil.SubString(FirstWord, 0, 4)
            String IntCheck = StringUtil.SubString(FirstWord, 0, 3)
            
            ;Only save properties outside of events and functions.
            If StringUtil.Find(FirstWord, "Event") > -1 
                I = StringUtil.Find(FileContents, "EndEvent", I);Skip events
                
            Elseif StringUtil.Find(FirstWord, "Function") > -1
                I = StringUtil.Find(FileContents, "EndFunction", I) ;Skip Functions 
                
            Elseif StringUtil.GetNthChar(FileContents, I) == ";" ;skip commented lines
                Int NextLine = StringUtil.Find(FileContents, "\n", I)
                If NextLine > -1 
                    I = (NextLine - 1)
                Else 
                    Search = False 
                Endif
            
            ;GlobalVariables====================================================================================================
            ElseIf StringUtil.Find(FirstWord, "GlobalVariable") > -1 
                I += 14 
                
                Char = StringUtil.GetNthChar(FileContents, I) 
                While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                Bool IsArray = False
                If Char == "[" ;if property is array 
                    IsArray = True 
                    I += 2
                Endif 
                
                Char = StringUtil.GetNthChar(FileContents, I) 
                While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                If Char == "P"
                    If StringUtil.SubString(FileContents, I, 8) == "Property" 
                        I += 8 ; skip property, move on to name
                        Char = StringUtil.GetNthChar(FileContents, I) 
                        While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                            I += 1
                            Char = StringUtil.GetNthChar(FileContents, I)
                        EndWhile 
                    Endif 
                Endif 
                
                Int NameStart = I 
                While Char != " " && Char != "\t" && char != "\n" && Char != ";" && I < ContentLength
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                If char == "\n"
                    I -= 1
                Endif
                
                String Name = StringUtil.SubString(FileContents, NameStart, (I - NameStart))
                If Name == "Function"
                    ;do nothing
                ElseIf IsArray == True 
                    ;Int EmptySlot = GlobalVariableArrays.Find("")
                    GlobalVariableArrays += ("|" + Name)
                    ;Debug.MessageBox("GV Array name = " + Name)
                    ;Utility.Wait(0.1)
                Else 
                    ;Int EmptySlot = GlobalVariables.Find("")
                    GlobalVariables += ("|" + Name)
                    ;Debug.MessageBox("GV name = " + Name)
                    ;Utility.Wait(0.1)
                Endif
                
            ;Strings====================================================================================================
            ElseIf StringUtil.Find(StringCheck, "String") > -1 
                I += 6
                
                Char = StringUtil.GetNthChar(FileContents, I) 
                While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                Bool IsArray = False
                If Char == "[" ;if property is array 
                    IsArray = True 
                    I += 2
                Endif 
                
                Char = StringUtil.GetNthChar(FileContents, I) 
                While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                If Char == "P"
                    If StringUtil.SubString(FileContents, I, 8) == "Property" 
                        I += 8 ; skip property, move on to name
                        Char = StringUtil.GetNthChar(FileContents, I) 
                        While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                            I += 1
                            Char = StringUtil.GetNthChar(FileContents, I)
                        EndWhile 
                    Endif 
                Endif 
                
                Int NameStart = I 
                While Char != " " && Char != "\t" && char != "\n" && Char != ";" && I < ContentLength
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                If char == "\n"
                    I -= 1
                Endif
                
                String Name = StringUtil.SubString(FileContents, NameStart, (I - NameStart))
                If Name == "Function"
                    ;do nothing
                ElseIf IsArray == True 
                    ;Int EmptySlot = StringArrays.Find("")
                    StringArrays += ("|" + Name)
                    ;Debug.MessageBox("GV Array name = " + Name)
                    ;Utility.Wait(0.1)
                Else 
                    ;Int EmptySlot = Strings.Find("")
                    Strings += ("|" + Name)
                    ;Debug.MessageBox("GV name = " + Name)
                    ;Utility.Wait(0.1)
                Endif
            
            ;floats================================================================================================================
            ElseIf StringUtil.Find(FloatCheck, "Float") > -1 
                I += 5
                
                Char = StringUtil.GetNthChar(FileContents, I) 
                While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                Bool IsArray = False
                If Char == "[" ;if property is array 
                    IsArray = True 
                    I += 2
                Endif 
                
                Char = StringUtil.GetNthChar(FileContents, I) 
                While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                If Char == "P"
                    If StringUtil.SubString(FileContents, I, 8) == "Property" 
                        I += 8 ; skip property, move on to name
                        Char = StringUtil.GetNthChar(FileContents, I) 
                        While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                            I += 1
                            Char = StringUtil.GetNthChar(FileContents, I)
                        EndWhile 
                    Endif 
                Endif 
                
                Int NameStart = I 
                While Char != " " && Char != "\t" && char != "\n" && Char != ";" && I < ContentLength
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                If char == "\n"
                    I -= 1
                Endif
                
                String Name = StringUtil.SubString(FileContents, NameStart, (I - NameStart))
                If Name == "Function"
                    ;do nothing
                ElseIf IsArray == True 
                    ;Int EmptySlot = FloatArrays.Find("")
                    FloatArrays += ("|" + Name)
                    ;Debug.MessageBox("Float Array name = " + Name)
                    ;Utility.Wait(0.1)
                Else 
                    ;Int EmptySlot = Floats.Find("")
                    Floats += ("|" + Name)
                    ;Debug.MessageBox("Float name = " + Name)
                    ;Utility.Wait(0.1)
                Endif
                
            ;Bools================================================================================================================
            ElseIf StringUtil.Find(BoolCheck, "Bool") > -1 
                I += 4 
                
                Char = StringUtil.GetNthChar(FileContents, I) 
                While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                Bool IsArray = False
                If Char == "[" ;if property is array 
                    IsArray = True 
                    I += 2
                Endif 
                
                Char = StringUtil.GetNthChar(FileContents, I) 
                While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                If Char == "P"
                    If StringUtil.SubString(FileContents, I, 8) == "Property" 
                        I += 8 ; skip property, move on to name
                        Char = StringUtil.GetNthChar(FileContents, I) 
                        While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                            I += 1
                            Char = StringUtil.GetNthChar(FileContents, I)
                        EndWhile 
                    Endif 
                Endif 
                
                Int NameStart = I 
                While Char != " " && Char != "\t" && char != "\n" && Char != ";" && I < ContentLength
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                If char == "\n"
                    I -= 1
                Endif
                
                String Name = StringUtil.SubString(FileContents, NameStart, (I - NameStart))
                If Name == "Function"
                    ;do nothing
                ElseIf IsArray == True 
                    ;Int EmptySlot = BoolArrays.Find("")
                    BoolArrays += ("|" + Name)
                    ;Debug.MessageBox("Bool Array name = " + Name)
                    ;Utility.Wait(0.1)
                Else 
                    ;Int EmptySlot = Bools.Find("")
                    Bools += ("|" + Name)
                    ;Debug.MessageBox("Bool name = " + Name)
                    ;Utility.Wait(0.1)
                Endif
                
            ;Ints================================================================================================================
            ElseIf StringUtil.Find(IntCheck, "Int") > -1 
                I += 3 
                
                Char = StringUtil.GetNthChar(FileContents, I) 
                While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                Bool IsArray = False
                If Char == "[" ;if property is array 
                    IsArray = True 
                    I += 2
                Endif 
                
                Char = StringUtil.GetNthChar(FileContents, I) 
                While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                If Char == "P"
                    If StringUtil.SubString(FileContents, I, 8) == "Property" 
                        I += 8 ; skip property, move on to name
                        Char = StringUtil.GetNthChar(FileContents, I) 
                        While (Char == " " || Char == "\t") && I < ContentLength ;space or tab, find next actual character of new line 
                            I += 1
                            Char = StringUtil.GetNthChar(FileContents, I)
                        EndWhile 
                    Endif 
                Endif 
                
                Int NameStart = I 
              
                While Char != " " && Char != "\t" && char != "\n" && Char != ";" && I < ContentLength
                    I += 1
                    Char = StringUtil.GetNthChar(FileContents, I)
                EndWhile 
                
                If char == "\n"
                    I -= 1
                Endif
                
                String Name = StringUtil.SubString(FileContents, NameStart, (I - NameStart))
                If Name == "Function"
                    ;do nothing
                ElseIf IsArray == True 
                    ;Int EmptySlot = IntArrays.Find("")
                    IntArrays += ("|" + Name)
                    ;Debug.MessageBox("Int Array name = " + Name)
                    ;Utility.Wait(0.1)
                Else 
                    ;Int EmptySlot = Ints.Find("")
                    Ints += ("|" + Name)
                    ;Debug.MessageBox("Int name = " + Name)
                    ;Utility.Wait(0.1)
                Endif
            Endif  
        Else 
            Search = False 
        endif 
    EndWhile
    
    ;Debug.Notification("End Search")
    
    ;Write Save settings function=====================================================================================
    
    MiscUtil.WriteToFile(DestinationFilePath, "\n \n" + "Function SaveToJson(String JsonFileName, Int Messages = 0, String ConfirmMsg = \"Settings Saved\")")
    
    Int L = StringUtil.GetLength(GlobalVariables)
    I = 1
    
    If GlobalVariablesToggle == True
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(GlobalVariables, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(GlobalVariables, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t JsonUtil.SetFloatValue(JsonFileName, " + "\"" + Name + "\"" + ", " + Name + ".GetValue())") 
            
            I = (NameEnd + 1)
        EndWhile
    Endif
    
    If FloatsToggle == True
        L = StringUtil.GetLength(Floats)
        I = 1
        
        If L > 0 
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
        Endif
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(floats, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(floats, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t JsonUtil.SetFloatValue(JsonFileName, " + "\"" + Name + "\"" + ", " + Name + ")") 
        
            I = (NameEnd + 1)
        EndWhile
    Endif
    
    If StringsToggle == True
        L = StringUtil.GetLength(Strings)
        I = 1
        
        If L > 0 
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
        Endif
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(Strings, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(Strings, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t JsonUtil.SetStringValue(JsonFileName, " + "\"" + Name + "\"" + ", " + Name + ")") 
        
            I = (NameEnd + 1)
        EndWhile
    Endif
    
    
    If IntsToggle == True
        L = StringUtil.GetLength(Ints)
        I = 1
        
        If L > 0 
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
        Endif
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(Ints, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(Ints, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t JsonUtil.SetIntValue(JsonFileName, " + "\"" + Name + "\"" + ", " + Name + ")") 
        
            I = (NameEnd + 1)
        EndWhile
    Endif
    
    
    If BoolsToggle == True
        L = StringUtil.GetLength(Bools)
        I = 1
        
        If L > 0 
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
        Endif
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(Bools, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(Bools, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t JsonUtil.SetIntValue(JsonFileName, " + "\"" + Name + "\"" + ", " + Name + " As Int)") 
        
            I = (NameEnd + 1)
        EndWhile
    Endif
    
;    ;Arrays ==============================================================================================================================================================================================
    
    If FloatArraysToggle == True
        L = StringUtil.GetLength(FloatArrays)
        I = 1
        
        If L > 0 
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
        Endif
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(FloatArrays, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(FloatArrays, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t JsonUtil.FloatListCopy(JsonFileName, " + "\"" + Name + "\"" + ", " + Name + ")") 
        
            I = (NameEnd + 1)
        EndWhile
    Endif
    
    
    If StringArraysToggle == True
        L = StringUtil.GetLength(StringArrays)
        I = 1
       
        If L > 0 
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
        Endif
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(StringArrays, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(StringArrays, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t JsonUtil.StringListCopy(JsonFileName, " + "\"" + Name + "\"" + ", " + Name + ")") 
        
            I = (NameEnd + 1)
        EndWhile
    Endif
    
    
    If IntArraysToggle == True
        L = StringUtil.GetLength(IntArrays)
        I = 1
        
        If L > 0 
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
        Endif
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(IntArrays, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(IntArrays, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t JsonUtil.IntListCopy(JsonFileName, " + "\"" + Name + "\"" + ", " + Name + ")") 
        
            I = (NameEnd + 1)
        EndWhile
    Endif
    
    Bool ArrayInts = False
    
    If GlobalVariableArraysToggle == True
        L = StringUtil.GetLength(GlobalVariableArrays)
        I = 1
        
        If L > 0
            ArrayInts = True
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t Int I")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t Int L")
        Endif
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(GlobalVariableArrays, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(GlobalVariableArrays, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t I = 0")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t L = " + Name + ".Length")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t While I < L" )
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t JsonUtil.SetFloatValue(JsonFileName, " + "\"" + Name + "\"" + " + I" + ", " + Name + "[I]" + ".GetValue())")   
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t I += 1")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t EndWhile")
        
            I = (NameEnd + 1)
        EndWhile
    Endif
  
    If BoolArraysToggle == True
        L = StringUtil.GetLength(BoolArrays)
        I = 1
        
        If L > 0
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
            If ArrayInts == false
                MiscUtil.WriteToFile(DestinationFilePath, "\n\tInt I")
                MiscUtil.WriteToFile(DestinationFilePath, "\n\tInt L")
            Endif
        Endif
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(BoolArrays, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(BoolArrays, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t I = 0")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t L = " + Name + ".Length")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t While I < L" )
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t JsonUtil.SetIntValue(JsonFileName, " + "\"" + Name + "\"" + " + I" + ", " + Name + "[I]" + " As Int)")   
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t I += 1")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t EndWhile")
        
            I = (NameEnd + 1)
        EndWhile
    Endif
    
    MiscUtil.WriteToFile(DestinationFilePath, "\n\n\tJsonUtil.Save(JsonFileName)")
    
    MiscUtil.WriteToFile(DestinationFilePath, "\n\n\tIf Messages == 1")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\tDebug.Notification(ConfirmMsg)")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\tElseIf Messages == 2")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\tDebug.MessageBox(ConfirmMsg)")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\tEndif")
    MiscUtil.WriteToFile(DestinationFilePath, "\nEndFunction")
    
    ;Write load settings function=====================================================================================
    
    MiscUtil.WriteToFile(DestinationFilePath, "\n\n" + "Function LoadFromJson(String JsonFileName, Int Messages = 0, String JsonCorruptedWarning = \"Json file is corrupted. Load cancelled\", String SettingNotFoundWarning = \"Settings not found\", String ConfirmMsg = \"\")" )    
    
    MiscUtil.WriteToFile(DestinationFilePath, "\n\tIf JsonUtil.JsonExists(JsonFileName)")
    
    MiscUtil.WriteToFile(DestinationFilePath, "\n\n\t\tJsonUtil.Load(JsonFileName)") 
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\tUtility.WaitMenuMode(0.5)")
    
    MiscUtil.WriteToFile(DestinationFilePath, "\n\n\t\tIf JsonUtil.IsGood(JsonFileName) == False") 
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t\tIf Messages == 1")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t\t\tDebug.Notification(JsonCorruptedWarning)")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t\tElseIf Messages == 2")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t\t\tDebug.MessageBox(JsonCorruptedWarning)")
    
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t\tEndif")
    
    MiscUtil.WriteToFile(DestinationFilePath, "\n\n\t\t\tJsonUtil.ClearAll(JsonFileName)")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t\tUtility.WaitMenuMode(0.5)")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t\tReturn")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\tEndif")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\tElse") 
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\tIf Messages == 1")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t\tDebug.Notification(SettingNotFoundWarning)")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\tElseIf Messages == 2")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t\tDebug.MessageBox(SettingNotFoundWarning)")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\tEndif") 
    
    MiscUtil.WriteToFile(DestinationFilePath, "\n\n\t\tUtility.WaitMenuMode(0.5)")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\tReturn")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\tEndif \n")  
 
    If GlobalVariablesToggle == True
        L = StringUtil.GetLength(GlobalVariables)
        I = 1
        
        If UsePropertiesAsDefaults 
             While I < L && I != -1
                Int NameEnd = StringUtil.Find(GlobalVariables, "|", I)
                If NameEnd == -1 
                    NameEnd = L
                Endif
                
                String Name = StringUtil.SubString(GlobalVariables, I, (NameEnd - I))
                MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + ".SetValue(JsonUtil.GetFloatValue(JsonFileName, " + "\"" + Name + "\"" + ", " + Name + ".GetValue()))")   
            
                I = (NameEnd + 1)
            EndWhile
        Else 
            While I < L && I != -1
                Int NameEnd = StringUtil.Find(GlobalVariables, "|", I)
                If NameEnd == -1 
                    NameEnd = L
                Endif
                
                String Name = StringUtil.SubString(GlobalVariables, I, (NameEnd - I))
                MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + ".SetValue(JsonUtil.GetFloatValue(JsonFileName, " + "\"" + Name + "\"" + "))")   
            
                I = (NameEnd + 1)
            EndWhile
        Endif
    Endif
  
    If FloatsToggle == True
        L = StringUtil.GetLength(Floats)
        I = 1
        
        If UsePropertiesAsDefaults 
             While I < L && I != -1
                Int NameEnd = StringUtil.Find(Floats, "|", I)
                If NameEnd == -1 
                    NameEnd = L
                Endif
                
                String Name = StringUtil.SubString(Floats, I, (NameEnd - I))
                MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + " = JsonUtil.GetFloatValue(JsonFileName, " + "\"" + Name + "\"" + ", " + Name + ")")   
            
                I = (NameEnd + 1)
             EndWhile
        Else 
            While I < L && I != -1
                Int NameEnd = StringUtil.Find(Floats, "|", I)
                If NameEnd == -1 
                    NameEnd = L
                Endif
                
                String Name = StringUtil.SubString(Floats, I, (NameEnd - I))
                MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + " = JsonUtil.GetFloatValue(JsonFileName, " + "\"" + Name + "\"" + ")")   
            
                I = (NameEnd + 1)
            EndWhile
        Endif
    Endif
  
    If StringsToggle == True
        L = StringUtil.GetLength(Strings)
        I = 1
        
        If UsePropertiesAsDefaults 
            While I < L && I != -1
                Int NameEnd = StringUtil.Find(Strings, "|", I)
                If NameEnd == -1 
                    NameEnd = L
                Endif
                
                String Name = StringUtil.SubString(Strings, I, (NameEnd - I))
                MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + " = JsonUtil.GetStringValue(JsonFileName, " + "\"" + Name + "\"" + ", " + Name + ")")                   
            
                I = (NameEnd + 1)
            EndWhile
        Else
            While I < L && I != -1
                Int NameEnd = StringUtil.Find(Strings, "|", I)
                If NameEnd == -1 
                    NameEnd = L
                Endif
                
                String Name = StringUtil.SubString(Strings, I, (NameEnd - I))
                MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + " = JsonUtil.GetStringValue(JsonFileName, " + "\"" + Name + "\"" + ")")                   
            
                I = (NameEnd + 1)
            EndWhile
        Endif
    Endif
    
    If IntsToggle == True
        L = StringUtil.GetLength(Ints)
        I = 1
        
        If UsePropertiesAsDefaults 
            While I < L && I != -1
                Int NameEnd = StringUtil.Find(Ints, "|", I)
                If NameEnd == -1 
                    NameEnd = L
                Endif
                
                String Name = StringUtil.SubString(Ints, I, (NameEnd - I))
                MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + " = JsonUtil.GetIntValue(JsonFileName, " + "\"" + Name + "\"" + ", " + Name  + ")")   
            
                I = (NameEnd + 1)
            EndWhile
        Else
            While I < L && I != -1
                Int NameEnd = StringUtil.Find(Ints, "|", I)
                If NameEnd == -1 
                    NameEnd = L
                Endif
                
                String Name = StringUtil.SubString(Ints, I, (NameEnd - I))
                MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + " = JsonUtil.GetIntValue(JsonFileName, " + "\"" + Name + "\"" + ")")   
            
                I = (NameEnd + 1)
            EndWhile
        Endif
    Endif
   
    If BoolsToggle == True
        L = StringUtil.GetLength(Bools)
        I = 1
        If L > 0 
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
        Endif 
        
        If UsePropertiesAsDefaults  
             While I < L && I != -1
                Int NameEnd = StringUtil.Find(Bools, "|", I)
                If NameEnd == -1 
                    NameEnd = L
                Endif
                
                String Name = StringUtil.SubString(Bools, I, (NameEnd - I))
                MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + " = JsonUtil.GetIntValue(JsonFileName, " + "\"" + Name + "\"" + ", " + Name  + " as int) As Bool")   
            
                I = (NameEnd + 1)
            EndWhile
        Else
            While I < L && I != -1
                Int NameEnd = StringUtil.Find(Bools, "|", I)
                If NameEnd == -1 
                    NameEnd = L
                Endif
                
                String Name = StringUtil.SubString(Bools, I, (NameEnd - I))
                MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + " = JsonUtil.GetIntValue(JsonFileName, " + "\"" + Name + "\"" + ") As Bool")   
            
                I = (NameEnd + 1)
            EndWhile
        Endif
    Endif
    
    
    ;Arrays ==============================================================================================================================================================================================
    MiscUtil.WriteToFile(DestinationFilePath, "\n")
    
    If FloatArraysToggle == True
        
        L = StringUtil.GetLength(FloatArrays)
        I = 1
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(FloatArrays, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(FloatArrays, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + " = JsonUtil.FloatListToArray(JsonFileName, " + "\"" + Name + "\"" + ")")   
        
            I = (NameEnd + 1)
        EndWhile
    Endif
    
    
    If StringArraysToggle == True
        L = StringUtil.GetLength(StringArrays)
        I = 1
        
        If L > 0 
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
        Endif
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(StringArrays, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(StringArrays, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + " = JsonUtil.StringListToArray(JsonFileName, " + "\"" + Name + "\"" + ")")   
          
        
            I = (NameEnd + 1)
        EndWhile
    Endif
    
    
    If IntArraysToggle == True
        L = StringUtil.GetLength(IntArrays)
        I = 1
       
        If L > 0 
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
        Endif
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(IntArrays, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(IntArrays, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t" + Name + " = JsonUtil.IntListToArray(JsonFileName, " + "\"" + Name + "\"" + ")")   
            I = (NameEnd + 1)
        
            I = (NameEnd + 1)
        EndWhile
    Endif
    
    ArrayInts = False
   
    If GlobalVariableArraysToggle == True
        L = StringUtil.GetLength(GlobalVariableArrays)
        I = 1
        
        If L > 0
            ArrayInts = True
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\tInt I")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\tInt L")
        Endif  
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(GlobalVariableArrays, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(GlobalVariableArrays, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\tI = 0")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\tL = " + Name + ".Length")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\tWhile I < L" )
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t" + Name + "[I]" + ".SetValue(JsonUtil.GetFloatValue(JsonFileName, " + "\"" + Name + "\"" + " + I))")   
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t\tI += 1")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\tEndWhile")
        
            I = (NameEnd + 1)
        EndWhile
    Endif
   
    If BoolArraysToggle == True
        L = StringUtil.GetLength(BoolArrays)
        I = 1
        
        If L > 0
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
            If ArrayInts == False
                MiscUtil.WriteToFile(DestinationFilePath, "\n\t Int I")
                MiscUtil.WriteToFile(DestinationFilePath, "\n\t Int L")
            Endif
        Endif  
        
        While I < L && I != -1
            Int NameEnd = StringUtil.Find(BoolArrays, "|", I)
            If NameEnd == -1 
                NameEnd = L
            Endif
            
            String Name = StringUtil.SubString(BoolArrays, I, (NameEnd - I))
            MiscUtil.WriteToFile(DestinationFilePath, "\n")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\tI = 0")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\tL = " + Name + ".Length")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\tWhile I < L" )
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t\t"  + Name + "[I] = JsonUtil.GetIntValue(JsonFileName, \"" + Name + "\" + I) As Bool")   
            MiscUtil.WriteToFile(DestinationFilePath, "\n\t\tI += 1")
            MiscUtil.WriteToFile(DestinationFilePath, "\n\tEndWhile")
        
            I = (NameEnd + 1)
        EndWhile
    Endif
    
    ;endwrite=======================================================================================================================================================================

    MiscUtil.WriteToFile(DestinationFilePath, "\n\n\tIf Messages == 1")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\tDebug.Notification(ConfirmMsg)")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\tElseIf Messages == 2")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\t\tDebug.MessageBox(ConfirmMsg)")
    MiscUtil.WriteToFile(DestinationFilePath, "\n\tEndif")
    MiscUtil.WriteToFile(DestinationFilePath, "\nEndFunction")
    
    If Messages == 1 
        Debug.Notification(ConfirmMessage) 
    Elseif Messages == 2 
        Debug.MessageBox(ConfirmMessage) 
    Endif 
EndFunction


;used to write the states in the DynamicArrays script. Saved here for posterity
;requires SKSE and PapyrusUtil. 
Function WriteDynamicArrayState(int i) 
    String f = "Data/Scripts/Source/DynamicArrays.psc"
    
MiscUtil.WriteToFile(f, "State A" + i + "\n")
    MiscUtil.WriteToFile(f, "\t" + "String[] Function GetStringArray()" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "String[] A = New String[" + i + "]" + "\n") 
        MiscUtil.WriteToFile(f, "\t\t" + "Return A" + "\n")
    MiscUtil.WriteToFile(f, "\t" + "EndFunction" + "\n\n")
    
    MiscUtil.WriteToFile(f, "\t" + "Bool[] Function GetBoolArray()" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "Bool[] A = New Bool[" + i + "]" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "Return A" + "\n")
    MiscUtil.WriteToFile(f, "\t" + "EndFunction" + "\n\n")
    
    MiscUtil.WriteToFile(f, "\t" + "Int[] Function GetIntArray()" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "Int[] A = New Int[" + i + "]" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "Return A" + "\n")
    MiscUtil.WriteToFile(f, "\t" + "EndFunction" + "\n\n")
    
    MiscUtil.WriteToFile(f, "\t" + "Float[] Function GetFloatArray()" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "Float[] A = New Float[" + i + "]" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "Return A" + "\n")
    MiscUtil.WriteToFile(f, "\t" + "EndFunction" + "\n\n")
    
    MiscUtil.WriteToFile(f, "\t" + "ObjectReference[] Function GetObjectReferenceArray()" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "ObjectReference[] A = New ObjectReference[" + i + "]" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "Return A" + "\n")
    MiscUtil.WriteToFile(f, "\t" + "EndFunction" + "\n\n")
    
    MiscUtil.WriteToFile(f, "\t" + "Actor[] Function GetActorArray()" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "Actor[] A = New Actor[" + i + "]" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "Return A" + "\n")
    MiscUtil.WriteToFile(f, "\t" + "EndFunction" + "\n\n")
    
    MiscUtil.WriteToFile(f, "\t" + "Form[] Function GetFormArray()" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "Form[] A = New Form[" + i + "]" + "\n")
        MiscUtil.WriteToFile(f, "\t\t" + "Return A" + "\n")
    MiscUtil.WriteToFile(f, "\t" + "EndFunction" + "\n")
MiscUtil.WriteToFile(f, "EndState"+ "\n\n")
EndFunction 



