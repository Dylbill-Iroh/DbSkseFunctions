scriptname DbSkseArray_Alias hidden 
; To create these array functions for different papyrus types, duplicate this script, 
; rename it and and replace all of Alias with a new type you want these array functions for, E.G TextureSet
; Compile the script and copy the source .psc file to Data/Scripts/Source/DbSkseArrays/ 
; IMPORTANT!!! You can change the type or add ArrayAs functions, but don't change the Aliasatting because 
; the plugin gets papyrus types by reading this source .psc file. Specifically, no spaces between the function name and first left parentheses "("
; That is how the DbSkseFunctions.dll knows to register the functions and what type to register functions as.
; In your mod's download, include the .pex files for these scripts in Data/Scripts folder like you would a normal script. 
; Also include the Data/Scripts/Source/DbSkseArrays/ folder with the new source .psc files or the functions here won't work.
; You can only have one array type per script. If you need more types, duplicate more of these scripts in the same manner.
; The exception is that you can add as many ArrayAs functions as you like, as long as the types are compatible. More details below.
; Custom user made types are not supported. Only vanilla types are supported, E.G types found in
; AliasType.psc as well as Alias, ReferenceAlias, LocationAlias and ActiveMagicEffect

; Create a new array of size, filled with optional filler.
Alias[] Function Create(int size, Alias filler = none) Global Native 

; Resize the passed in array. If the size is larger, fills the new indexes with filler. 
Alias[] Function Resize(Alias[] arr, int size, Alias filler = none) Global Native

; Merge the two arrays and return the new array containing both array's elements.
Alias[] Function Merge(Alias[] arr_A, Alias[] arr_B) Global Native

; Returns a sub section of the passed in array indicated by a starting and ending index.
; The default argument "int endIndex = -1" clamps the to the end of the array. Equivalent of setting EndIndex = (ArrayValues.Length - 1)
Alias[] function Slice(Alias[] ArrayValues, int startIndex, int endIndex = -1) global native

; Add the toPush Alias to the end of the arr, increasing its size.
Alias[] Function Push(Alias[] arr, Alias toPush) Global Native 

; insert the toInsert Alias into the array, increasing its size. If the index is out of bound, puts the Alias at the end of the array.
Alias[] Function Insert(Alias[] arr, Alias toInsert, int index) global native

; Remove the toRemove Alias from the array, decreasing its size. If all is true, remove all instances, otherwise, remove the first found instance.
Alias[] Function Remove(Alias[] arr, Alias toRemove, bool all = true) global native

; remove the element at the index in the array, decreasing its size. If the index is out of bounds, removes the last element in the array.
Alias[] Function RemoveAt(Alias[] arr, int index) Global Native 

; remove all duplicates from the arr, making each element in the arr unique, and return the new array.
Alias[] Function RemoveDuplicates(Alias[] arr) Global Native 

; returns new array that contains the Aliass of the passed in akAliass array, but sorted. 
; Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
; 1 = by Alias name ascending, 
; 2 = by Alias name descending, 
; 3 = by Alias editor Id name ascending,
; 4 = by Alias editor Id name descending,
; 5 = by Alias Id ascending, 
; 6 = by Alias Id descending
Alias[] Function Sort(Alias[] arr, int sortOption) Global Native 

; Get the strings for the passed in array. mode options are:
; 1 = Alias names
; 2 = editor names 
; 3 = IDs as hexidecimal (AliasId or AliasId)
; SortOptions are: 
; 1 = not sorted 
; 2 = sorted ascending 
; 3 = sorted descending
String[] Function GetStrings(Alias[] arr, int mode, int sortOption, string nullName = "NONE", string emptyName = " - ")  Global Native 

; count the number of times the item appears in the array.
int Function Count(Alias[] arr, Alias item) Global Native 

; return true if all of the elements in arr_A equal the elements in arr_B.
bool function IsEqual(Alias[] arr_A, Alias[] arr_B) Global Native 

; for linking arrays. Return an int array that contains all indexes in arr that match the value
; use with RemoveIndexes to remove all of value from the same arr, and remove the same indexes from another array.
; See the DbSkseMap_Alias_Alias.psc script for example usage.
int[] function GetIndexes(Alias[] arr, Alias value) global native

; for linking arrays. Returns an int array containing all indexes of duplicate elements in the arr.
; use with RemoveIndexes to remove all duplicates from the same arr, and remove the same indexes from another array.
; See the DbSkseMap_Alias_Alias.psc script for example usage.
int[] function GetRemoveDuplicatesIndexes(Alias[] arr) global native

; Removes all indexes in the arr. Example, if indexes contains [0] = 2, [1] = 7, 
; Removes the elements that are currently at [2] and [7] from the arr. 
; See the DbSkseMap_Alias_Alias.psc script for example usage.
Alias[] Function RemoveIndexes(Alias[] arr, int[] indexes) Global Native 

; for linking arrays, get new sorted index for the arr. 
; See the DbSkseMap_Alias_Alias.psc script for example usage.
;  Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
;  1 = by Alias name ascending, 
;  2 = by Alias name descending, 
;  3 = by Alias editor Id name ascending,
;  4 = by Alias editor Id name descending,
;  5 = by Alias Id ascending, 
;  6 = by Alias Id descending
int[] Function GetSortIndexes(Alias[] arr, int sortOption) Global Native 

; See the DbSkseMap_Alias_Alias.psc script for example usage.
Alias[] Function SortByIndexes(Alias[] arr, int[] indexes) Global Native 

; Get the indexes of the postChangedArr in the original preChangedArr. Allows to alter another array the exact same way the preChangedArr was altered.
; See DbSkseArray_Form.GetChangedIndexes for an example.
int[] Function GetChangedIndexes(Alias[] preChangedArr, Alias[] posChangedArr) Global Native 

; Filter arr by indexes and return new array. See DbSkseArray_Form.GetChangedIndexes for more info.
Alias[] Function ChangeByIndexes(Alias[] arr, int[] indexes) Global Native 

;  ArrayAs functions, returns a new array where each element of the passed in array is cast as the return type. 
;  If removeNoneArrElements, any none entry in the passed in array is removed from the return array. 
;  If removeFailedToConvertElements, any valid entry in the passed in array that fails to convert to the return type is removed from the return array.
;  You can add as many ArrayAs functions here as you want, converting from any compatible type. 
;  Only vanilla types are supported. Any type found in AliasType.psc plus bool, int, float, string, Alias, Alias and ActiveMagicEffect.
;  Make sure the new function names contain ArrayAs.

;  Cast all Alias array elements to ReferenceAlias and return new array.
ReferenceAlias[] Function ArrayAsRefAlias(Alias[] arr, bool removeNoneArrElements = true, bool removeFailedToConvertElements = true) Global Native
