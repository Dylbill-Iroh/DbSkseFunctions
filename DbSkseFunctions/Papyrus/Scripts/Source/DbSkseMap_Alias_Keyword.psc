scriptname DbSkseMap_Alias_Keyword extends ObjectReference
;map functionality. Keys are unique, values aren't. Keys are bound to Values via array indexes.
;Arrays are accessible properties by necessity, but you shouldn't alter them directly as that has 
;the potential for them to get out of sync. Use the insert, remove ect functions instead. 
;Iterating through an array where you're reading values is fine.
;Must have DbSkseArray scripts for the keys and values types.

; To make new maps with different key and value types, copy this script and rename it. 
; Replace all Alias with the new keys type. E.G TextureSet
; Replace all int with the new values type. E.G string
; Replace all DbSkseArray_Alias with the DbSkseArray script you use for the key type E.G MyMod_DbSkseArray_TextureSet
; Replace all DbSkseArray_Keyword with the DbSkseArray script you use for the value type E.G MyMod_DbSkseArray_String
; Replace all DbSkseMap_Alias_Keyword with the name of your new map script E.G MyMod_DbSkseMap_TextureSet_String

; DbSkseArray_Alias 
Alias[] property Keys auto hidden

; DbSkseArray_Keyword
Keyword[] property values auto hidden

;Uses PlaceAtMe to Create a new map.
;If the mapBaseForm is none, creates a new xMarker with this script is attached. 
;If the mapBaseForm is not none, or this script is not attached, attempts to attach.
;If placeAtMeRef is none, places at the player.
DbSkseMap_Alias_Keyword function Create(Alias[] initialKeys = none, Keyword[] initialValues = none, ObjectReference placeAtMeRef = none, bool persistent = true, bool initiallyDisabled = true, Form mapBaseForm = none) Global 
	
	objectReference mapRef
	
	if !placeAtMeRef 
		placeAtMeRef = game.getplayer()
	Endif
	
	if !mapBaseForm 
    	mapBaseForm = Game.GetFormFromFile(0x00003B, "Skyrim.esm")
		if !mapBaseForm ;xMarker static not found
			Debug.Trace("xmarker Alias not found", 2)
			return none 
		Endif
	Endif
	
	mapRef = placeAtMeRef.PlaceAtMe(mapBaseForm, 1, persistent, initiallyDisabled)
	
	DbSkseMap_Alias_Keyword map = mapRef as DbSkseMap_Alias_Keyword
	if !map 
		;attempt to attach script to map
		DbSkseFunctions.ExecuteConsoleCommand("aps DbSkseMap_Alias_Keyword", mapRef)
		Utility.WaitMenuMode(0.5)
		map = mapRef as DbSkseMap_Alias_Keyword
	Endif
	
	if !map 
		debug.trace("failed to create " + DbSkseFunctions.GetThisScriptName(), 2)
		mapRef.disable()
		mapRef.Delete()
		return none
	Endif
	
	if (initialKeys && initialKeys.Length > 0 && initialKeys.length == initialValues.length)
		map.Keys = DbSkseArray_Alias.Resize(initialKeys, initialKeys.length) ;copies arr elements rather than asigning array
		map.values = DbSkseArray_Keyword.Resize(initialValues, initialValues.length) ;copies arr elements rather than asigning array
	Endif 
	
	return map
EndFunction

;copy this map's keys and values onto another map, replacing them. 
;use merge instead if you want to append keys and values.
bool Function CopyTo(DbSkseMap_Alias_Keyword other)
	if other 
		;running this function returns a new array with the same elements as the passed in array
		other.Keys = DbSkseArray_Alias.Resize(keys, keys.length)
		other.values = DbSkseArray_Keyword.Resize(values, values.length)
		return (other.keys.Length == keys.Length && other.values.length == values.length)
	Endif
	
	return false
EndFunction

;copy the other map's keys and values onto this map, replacing them. 
;use merge instead if you want to append keys and values.
bool Function CopyFrom(DbSkseMap_Alias_Keyword other)
	if other 
		;running this function returns a new array with the same elements as the passed in array
		Keys = DbSkseArray_Alias.Resize(other.keys, other.keys.length)
		values = DbSkseArray_Keyword.Resize(other.values, other.values.length)
		return (other.keys.Length == keys.Length && other.values.length == values.length)
	Endif
	
	return false
EndFunction

; insert pair into this map. If the key already exists, replaces its value with the passed in value
function Insert(Alias akKey, keyword value)
	int i = keys.Find(akKey)
	if i != -1 
		values[i] = value
	Else 
		Keys = DbSkseArray_Alias.Push(Keys, akKey)
		values = DbSkseArray_Keyword.Push(values, value)
	Endif
EndFunction

function Clear()
	Keys = DbSkseArray_Alias.Create(0)
	values = DbSkseArray_Keyword.Create(0)
EndFunction

;get the value for the akKey, or default if not found
keyword function Value(Alias akKey, keyword default) 
	int i = keys.Find(akKey)
	if i != -1 
		return values[i]
	Else 
		return default
	Endif
EndFunction

;get the key for the value, or default if not found.
Alias function Key(keyword value, Alias default) 
	int i = values.Find(value)
	if i != -1 
		return keys[i]
	Else 
		return default
	Endif
EndFunction 

Alias function GetKeyAt(int index)
	return keys[index]
EndFunction

keyword function GetValueAt(int index)
	return values[index]
EndFunction

int function GetSize()
	return keys.length
EndFunction

bool function IsSynced()
	return keys.Length == values.Length
EndFunction

; Add the mapBaseForm array elements to the end of this map's array elements, increasing the size
bool function Merge(DbSkseMap_Alias_Keyword map) 
	int targetSize = keys.Length + map.keys.Length
	
	keys = DbSkseArray_Alias.Merge(keys, map.keys)
	values = DbSkseArray_Keyword.Merge(values, map.values)
	
	bool success = (keys.Length == targetSize && values.Length == targetSize)
	
	int[] removeDupesIndexes = DbSkseArray_Alias.GetRemoveDuplicatesIndexes(keys) 
	keys = DbSkseArray_Alias.RemoveIndexes(keys, removeDupesIndexes) ;remove duplicates from keys
	values = DbSkseArray_Keyword.RemoveIndexes(values, removeDupesIndexes) ;sync the values array 
	
	return (success && keys.Length == values.Length)
EndFunction

;merge the arrays directly. passed in keys and values arrays must be the same size.
bool function MergeArrays(Alias[] keysToMerge, Keyword[] valuesToMerge)
	if keysToMerge.Length != valuesToMerge.length || keysToMerge.Length == 0
		return false
	Endif 
	
	int targetSize = keys.Length + keysToMerge.Length
	
	keys = DbSkseArray_Alias.Merge(keys, keysToMerge)
	values = DbSkseArray_Keyword.Merge(values, valuesToMerge)
	
	bool success = (keys.Length == targetSize && values.Length == targetSize)
	
	int[] removeDupesIndexes = DbSkseArray_Alias.GetRemoveDuplicatesIndexes(keys) 
	keys = DbSkseArray_Alias.RemoveIndexes(keys, removeDupesIndexes) ;remove duplicates from keys
	values = DbSkseArray_Keyword.RemoveIndexes(values, removeDupesIndexes) ;sync the values array 
	
	return (success && keys.Length == values.Length)
Endfunction

; Keep the elements between startIndex and endIndex from this map's arrays and remove the rest.
; The default argument "int endIndex = -1" clamps the to the end of the mapBaseFormay. Equivalent of setting EndIndex = (mapBaseFormayValues.Length - 1)
function Slice(int startIndex, int endIndex = -1) 
	keys = DbSkseArray_Alias.slice(keys, startIndex, endIndex)
	values = DbSkseArray_Keyword.slice(values, startIndex, endIndex)
EndFunction

bool function Remove(Alias akKey) 
	int size = keys.Length
	int i = keys.Find(akKey)
	if i != -1 
		keys = DbSkseArray_Alias.RemoveAt(keys, i)
		values = DbSkseArray_Keyword.RemoveAt(values, i)
		return keys.length == (size - 1)
	endif 
	
	return false
EndFunction

; remove the pair at the index in this map, decreasing its size. If the index is out of bounds, removes the last pair in the map.
bool function RemoveAt(int index) 
	int size = Keys.length
	keys = DbSkseArray_Alias.RemoveAt(keys, index)
	values = DbSkseArray_Keyword.RemoveAt(values, index)
	return keys.length == (size - 1)
EndFunction  

;remove pairs by searching values, if all is true, removes all pairs where value matches, otherwise removes the first it finds.
bool function RemoveValues(keyword value, bool all) 
	int size = keys.length
	if all 
		int[] indexes = DbSkseArray_Keyword.GetIndexes(values, value)
		values = DbSkseArray_Keyword.RemoveIndexes(values, indexes)
		keys = DbSkseArray_Alias.RemoveIndexes(keys, indexes)
		return values.Find(value) == -1
	Else 
		int i = values.find(value) 
		if i != -1 
			keys = DbSkseArray_Alias.RemoveAt(keys, i)
			values = DbSkseArray_Keyword.RemoveAt(values, i)
			return keys.length == (size - 1)
		endif
	Endif
EndFunction

; remove all duplicate values from this map, making each value element in the map unique.
; keys are already unique.
function RemoveDuplicates() 
	int size = keys.length
	int[] indexes = DbSkseArray_Keyword.GetRemoveDuplicatesIndexes(values) 
	values = DbSkseArray_Keyword.RemoveIndexes(values, indexes)
	keys = DbSkseArray_Alias.RemoveIndexes(keys, indexes)
EndFunction

; ; Sort options are as follows, if the key is a Alias type. Note, to sort by editor Id reliably, po3 tweaks must be installed.
; ; 1 = by Alias name ascending, 
; ; 2 = by Alias name descending, 
; ; 3 = by Alias editor Id name ascending,
; ; 4 = by Alias editor Id name descending,
; ; 5 = by Alias Id ascending, 
; ; 6 = by Alias Id descending
; ; if sortByKeys == true, sorts by sortByKeys, otherwise by values
function Sort(int sortOption, bool sortByKeys) 
	int[] indexes
	if sortByKeys 
		indexes = DbSkseArray_Alias.GetSortIndexes(keys, sortOption)
	else 
		indexes = DbSkseArray_Keyword.GetSortIndexes(values, sortOption)
	Endif 
	
	keys = DbSkseArray_Alias.SortByIndexes(keys, indexes)
	values = DbSkseArray_Keyword.SortByIndexes(values, indexes)
EndFunction

; ; Get the strings for this map. mode options are:
; ; 1 = Alias names
; ; 2 = editor names 
; ; 3 = IDs as hexidecimal (AliasId or AliasId)
; ; SortOptions are: 
; ; 1 = not sorted 
; ; 2 = sorted ascending 
; ; 3 = sorted descending
; ; type 0 = keys, type 1 = values, type 2 = keys and values
String[] Function GetStrings(int mode, int sortOption, int type, string nullName = "NONE", string emptyName = " - ")
	if type == 0
		return DbSkseArray_Alias.GetStrings(keys, mode, sortOption, nullName, emptyName)
		
	Elseif type == 1
		return DbSkseArray_Keyword.GetStrings(values, mode, sortOption, nullName, emptyName)
		
	Else 
		string[] keyStrings = DbSkseArray_Alias.GetStrings(keys, mode, sortOption, nullName, emptyName)
		string[] valueStrings = DbSkseArray_Keyword.GetStrings(values, mode, sortOption, nullName, emptyName)
		string[] returnStrings = LFT_SkseArray_String.Merge(keyStrings, valueStrings)
		
		if sortOption == 2
			returnStrings = LFT_SkseArray_String.sort(returnStrings, 1) ;ascending
		elseif sortOption == 3
			returnStrings = LFT_SkseArray_String.sort(returnStrings, 2) ;descending
		Endif
		
		return returnStrings
	Endif
EndFunction
 