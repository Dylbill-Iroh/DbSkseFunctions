scriptname DbSkseArray_Bool hidden 
; To create these array functions for different papyrus types, duplicate this script, 
; rename it and and replace all of Bool with a new type you want these array functions for, E.G TextureSet
; Compile the script and copy the source .psc file to Data/Scripts/Source/DbSkseArrays/ 
; IMPORTANT!!! You can change the type or add ArrayAs functions, but don't change the Boolatting because 
; the plugin gets papyrus types by reading this source .psc file. Specifically, no spaces between the function name and first left parentheses "("
; That is how the DbSkseFunctions.dll knows to register the functions and what type to register functions as.
; In your mod's download, include the .pex files for these scripts in Data/Scripts folder like you would a normal script. 
; Also include the Data/Scripts/Source/DbSkseArrays/ folder with the new source .psc files or the functions here won't work.
; You can only have one array type per script. If you need more types, duplicate more of these scripts in the same manner.
; The exception is that you can add as many ArrayAs functions as you like, as long as the types are compatible. More details below.
; Custom user made types are not supported. Only vanilla types are supported, E.G types found in
; BoolType.psc as well as Alias, ReferenceAlias, LocationAlias and ActiveMagicEffect

; Create a new array of size, filled with optional filler.
Bool[] Function Create(int size, Bool filler = false) Global Native 

; Resize the passed in array. If the size is larger, fills the new indexes with filler. 
Bool[] Function Resize(Bool[] arr, int size, Bool filler = false) Global Native

; Merge the two arrays and return the new array containing both array's elements.
Bool[] Function Merge(Bool[] arr_A, Bool[] arr_B) Global Native

; Returns a sub section of the passed in array indicated by a starting and ending index.
; The default argument "int endIndex = -1" clamps the to the end of the array. Equivalent of setting EndIndex = (ArrayValues.Length - 1)
Bool[] function Slice(Bool[] ArrayValues, int startIndex, int endIndex = -1) global native

; Add the toPush Bool to the end of the arr, increasing its size.
Bool[] Function Push(Bool[] arr, Bool toPush) Global Native 

; insert the toInsert Bool into the array, increasing its size. If the index is out of bound, puts the Bool at the end of the array.
Bool[] Function Insert(Bool[] arr, Bool toInsert, int index) global native

; Remove the toRemove Bool from the array, decreasing its size. If all is true, remove all instances, otherwise, remove the first found instance.
Bool[] Function Remove(Bool[] arr, Bool toRemove, bool all = true) global native

; remove the element at the index in the array, decreasing its size. If the index is out of bounds, removes the last element in the array.
Bool[] Function RemoveAt(Bool[] arr, int index) Global Native 

; remove all duplicates from the arr, making each element in the arr unique, and return the new array.
Bool[] Function RemoveDuplicates(Bool[] arr) Global Native 

; returns new array that contains the Bools of the passed in akBools array, but sorted. 
; Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
; 1 = by Bool name ascending, 
; 2 = by Bool name descending, 
; 3 = by Bool editor Id name ascending,
; 4 = by Bool editor Id name descending,
; 5 = by Bool Id ascending, 
; 6 = by Bool Id descending
Bool[] Function Sort(Bool[] arr, int sortOption) Global Native 

; Get the strings for the passed in array. mode options are:
; 1 = Bool names
; 2 = editor names 
; 3 = IDs as hexidecimal (BoolId or BoolId)
; SortOptions are: 
; 1 = not sorted 
; 2 = sorted ascending 
; 3 = sorted descending
String[] Function GetStrings(Bool[] arr, int mode, int sortOption, string nullName = "NONE", string emptyName = " - ")  Global Native 

; count the number of times the item appears in the array.
int Function Count(Bool[] arr, Bool item) Global Native 

; return true if all of the elements in arr_A equal the elements in arr_B.
bool function IsEqual(Bool[] arr_A, Bool[] arr_B) Global Native 

; for linking arrays. Return an int array that contains all indexes in arr that match the value
; use with RemoveIndexes to remove all of value from the same arr, and remove the same indexes from another array.
; See the DbSkseMap_Alias_Bool.psc script for example usage.
int[] function GetIndexes(Bool[] arr, Bool value) global native

; for linking arrays. Returns an int array containing all indexes of duplicate elements in the arr.
; use with RemoveIndexes to remove all duplicates from the same arr, and remove the same indexes from another array.
; See the DbSkseMap_Alias_Bool.psc script for example usage.
int[] function GetRemoveDuplicatesIndexes(Bool[] arr) global native

; Removes all indexes in the arr. Example, if indexes contains [0] = 2, [1] = 7, 
; Removes the elements that are currently at [2] and [7] from the arr. 
; See the DbSkseMap_Alias_Bool.psc script for example usage.
Bool[] Function RemoveIndexes(Bool[] arr, int[] indexes) Global Native 

; for linking arrays, get new sorted index for the arr. 
; See the DbSkseMap_Alias_Bool.psc script for example usage.
;  Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
;  1 = by Bool name ascending, 
;  2 = by Bool name descending, 
;  3 = by Bool editor Id name ascending,
;  4 = by Bool editor Id name descending,
;  5 = by Bool Id ascending, 
;  6 = by Bool Id descending
int[] Function GetSortIndexes(Bool[] arr, int sortOption) Global Native 

; See the DbSkseMap_Alias_Bool.psc script for example usage.
Bool[] Function SortByIndexes(Bool[] arr, int[] indexes) Global Native 

; Get the indexes of the postChangedArr in the original preChangedArr. Allows to alter another array the exact same way the preChangedArr was altered.
; See DbSkseArray_Form.GetChangedIndexes for an example.
int[] Function GetChangedIndexes(Bool[] preChangedArr, Bool[] posChangedArr) Global Native 

; Filter arr by indexes and return new array. See DbSkseArray_Form.GetChangedIndexes for more info.
Bool[] Function ChangeByIndexes(Bool[] arr, int[] indexes) Global Native 

;  ArrayAs functions, returns a new array where each element of the passed in array is cast as the return type. 
;  If removeNoneArrElements, any none entry in the passed in array is removed from the return array. 
;  If removeFailedToConvertElements, any valid entry in the passed in array that fails to convert to the return type is removed from the return array.
;  You can add as many ArrayAs functions here as you want, converting from any compatible type. 
;  Only vanilla types are supported. Any type found in BoolType.psc plus bool, int, float, string, Bool, Alias and ActiveMagicEffect.
;  Make sure the new function names contain ArrayAs.

;  Cast all Bool array elements to Int and return new array.
Int[] Function ArrayAsInt(Bool[] arr, bool removeNoneArrElements = true, bool removeFailedToConvertElements = true) Global Native
