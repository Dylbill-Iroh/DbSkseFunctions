scriptname DbSkseArray_String hidden 
; To create these array functions for different papyrus types, duplicate this script, 
; rename it and and replace all of String with a new type you want these array functions for, E.G TextureSet
; Compile the script and copy the source .psc file to Data/Scripts/Source/DbSkseArrays/ 
; IMPORTANT!!! You can change the type or add ArrayAs functions, but don't change the Stringatting because 
; the plugin gets papyrus types by reading this source .psc file. Specifically, no spaces between the function name and first left parentheses "("
; That is how the DbSkseFunctions.dll knows to register the functions and what type to register functions as.
; In your mod's download, include the .pex files for these scripts in Data/Scripts folder like you would a normal script. 
; Also include the Data/Scripts/Source/DbSkseArrays/ folder with the new source .psc files or the functions here won't work.
; You can only have one array type per script. If you need more types, duplicate more of these scripts in the same manner.
; The exception is that you can add as many ArrayAs functions as you like, as long as the types are compatible. More details below.
; Custom user made types are not supported. Only vanilla types are supported, E.G types found in
; StringType.psc as well as Alias, ReferenceAlias, LocationAlias and ActiveMagicEffect

; Create a new array of size, filled with optional filler.
String[] Function Create(int size, String filler = "") Global Native 

; Resize the passed in array. If the size is larger, fills the new indexes with filler. 
String[] Function Resize(String[] arr, int size, String filler = "") Global Native

; Merge the two arrays and return the new array containing both array's elements.
String[] Function Merge(String[] arr_A, String[] arr_B) Global Native

; Returns a sub section of the passed in array indicated by a starting and ending index.
; The default argument "int endIndex = -1" clamps the to the end of the array. Equivalent of setting EndIndex = (ArrayValues.Length - 1)
String[] function Slice(String[] ArrayValues, int startIndex, int endIndex = -1) global native

; Add the toPush String to the end of the arr, increasing its size.
String[] Function Push(String[] arr, String toPush) Global Native 

; insert the toInsert String into the array, increasing its size. If the index is out of bound, puts the String at the end of the array.
String[] Function Insert(String[] arr, String toInsert, int index) global native

; Remove the toRemove String from the array, decreasing its size. If all is true, remove all instances, otherwise, remove the first found instance.
String[] Function Remove(String[] arr, String toRemove, bool all = true) global native

; remove the element at the index in the array, decreasing its size. If the index is out of bounds, removes the last element in the array.
String[] Function RemoveAt(String[] arr, int index) Global Native 

; remove all duplicates from the arr, making each element in the arr unique, and return the new array.
String[] Function RemoveDuplicates(String[] arr) Global Native 

; returns new array that contains the Strings of the passed in akStrings array, but sorted. 
; Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
; 1 = by String name ascending, 
; 2 = by String name descending, 
; 3 = by String editor Id name ascending,
; 4 = by String editor Id name descending,
; 5 = by String Id ascending, 
; 6 = by String Id descending
String[] Function Sort(String[] arr, int sortOption) Global Native 

; Get the strings for the passed in array. mode options are:
; 1 = String names
; 2 = editor names 
; 3 = IDs as hexidecimal (StringId or StringId)
; SortOptions are: 
; 1 = not sorted 
; 2 = sorted ascending 
; 3 = sorted descending
String[] Function GetStrings(String[] arr, int mode, int sortOption, string nullName = "NONE", string emptyName = " - ")  Global Native 

; count the number of times the item appears in the array.
int Function Count(String[] arr, String item) Global Native 

; return true if all of the elements in arr_A equal the elements in arr_B.
bool function IsEqual(String[] arr_A, String[] arr_B) Global Native 

; for linking arrays. Return an int array that contains all indexes in arr that match the value
; use with RemoveIndexes to remove all of value from the same arr, and remove the same indexes from another array.
; See the DbSkseMap_Alias_String.psc script for example usage.
int[] function GetIndexes(String[] arr, String value) global native

; for linking arrays. Returns an int array containing all indexes of duplicate elements in the arr.
; use with RemoveIndexes to remove all duplicates from the same arr, and remove the same indexes from another array.
; See the DbSkseMap_Alias_String.psc script for example usage.
int[] function GetRemoveDuplicatesIndexes(String[] arr) global native

; Removes all indexes in the arr. Example, if indexes contains [0] = 2, [1] = 7, 
; Removes the elements that are currently at [2] and [7] from the arr. 
; See the DbSkseMap_Alias_String.psc script for example usage.
String[] Function RemoveIndexes(String[] arr, int[] indexes) Global Native 

; for linking arrays, get new sorted index for the arr. 
; See the DbSkseMap_Alias_String.psc script for example usage.
;  Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
;  1 = by String name ascending, 
;  2 = by String name descending, 
;  3 = by String editor Id name ascending,
;  4 = by String editor Id name descending,
;  5 = by String Id ascending, 
;  6 = by String Id descending
int[] Function GetSortIndexes(String[] arr, int sortOption) Global Native 

; See the DbSkseMap_Alias_String.psc script for example usage.
String[] Function SortByIndexes(String[] arr, int[] indexes) Global Native 

; Get the indexes of the postChangedArr in the original preChangedArr. Allows to alter another array the exact same way the preChangedArr was altered. Example:
; Form[] allAmmos = PO3_SKSEFunctions.GetAllForms(DbSkseFunctions.GetFormTypeForScript("Ammo"))
; String[] ammoFormNames = DbSkseFunctions.GetFormNames(allAmmos)
; String[] ironFormNames = DbSkseFunctions.GetStringsThatContain(ammoFormNames, "iron") ;get all names that contain "iron" in formNames 
; int[] indexes = DbSkseArray_String.GetChangedIndexes(ammoFormNames, ironFormNames)
; Form[] ironForms = DbSkseArray_Form.ChangeByIndexes(allAmmos, indexes) ;ironForms now contains all ammo forms from allAmmos whose name containes "iron"
int[] Function GetChangedIndexes(String[] preChangedArr, String[] posChangedArr) Global Native 

; Get the indexes of the postChangedArr in the original preChangedArr. Allows to alter another array the exact same way the preChangedArr was altered. Example:
; Filter arr by indexes and return new array. See GetChangedIndexes for more info. 
String[] Function ChangeByIndexes(String[] arr, int[] indexes) Global Native 

;  ArrayAs functions, returns a new array where each element of the passed in array is cast as the return type. 
;  If removeNoneArrElements, any none entry in the passed in array is removed from the return array. 
;  If removeFailedToConvertElements, any valid entry in the passed in array that fails to convert to the return type is removed from the return array.
;  You can add as many ArrayAs functions here as you want, converting from any compatible type. 
;  Only vanilla types are supported. Any type found in StringType.psc plus bool, int, float, string, String, Alias and ActiveMagicEffect.
;  Make sure the new function names contain ArrayAs.

;  Cast all String array elements to int and return new array.
int[] Function ArrayAsForm(String[] arr, bool removeNoneArrElements = true, bool removeFailedToConvertElements = true) Global Native
