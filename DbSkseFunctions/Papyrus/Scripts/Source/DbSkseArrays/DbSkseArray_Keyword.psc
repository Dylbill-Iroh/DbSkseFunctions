scriptname DbSkseArray_Keyword hidden 
; To create these array functions for different papyrus types, duplicate this script, 
; rename it and and replace all of Keyword with a new type you want these array functions for, E.G TextureSet
; Compile the script and copy the source .psc file to Data/Scripts/Source/DbSkseArrays/ 
; IMPORTANT!!! You can change the type or add ArrayAs functions, but don't change the Keywordatting because 
; the plugin gets papyrus types by reading this source .psc file. Specifically, no spaces between the function name and first left parentheses "("
; That is how the DbSkseFunctions.dll knows to register the functions and what type to register functions as.
; In your mod's download, include the .pex files for these scripts in Data/Scripts folder like you would a normal script. 
; Also include the Data/Scripts/Source/DbSkseArrays/ folder with the new source .psc files or the functions here won't work.
; You can only have one array type per script. If you need more types, duplicate more of these scripts in the same manner.
; The exception is that you can add as many ArrayAs functions as you like, as long as the types are compatible. More details below.
; Custom user made types are not supported. Only vanilla types are supported, E.G types found in
; KeywordType.psc as well as Alias, ReferenceAlias, LocationAlias and ActiveMagicEffect

; Create a new array of size, filled with optional filler.
Keyword[] Function Create(int size, Keyword filler = none) Global Native 

; Resize the passed in array. If the size is larger, fills the new indexes with filler. 
Keyword[] Function Resize(Keyword[] arr, int size, Keyword filler = none) Global Native

; Merge the two arrays and return the new array containing both array's elements.
Keyword[] Function Merge(Keyword[] arr_A, Keyword[] arr_B) Global Native

; Returns a sub section of the passed in array indicated by a starting and ending index.
; The default argument "int endIndex = -1" clamps the to the end of the array. Equivalent of setting EndIndex = (ArrayValues.Length - 1)
Keyword[] function Slice(Keyword[] ArrayValues, int startIndex, int endIndex = -1) global native

; Add the toPush Keyword to the end of the arr, increasing its size.
Keyword[] Function Push(Keyword[] arr, Keyword toPush) Global Native 

; insert the toInsert Keyword into the array, increasing its size. If the index is out of bound, puts the Keyword at the end of the array.
Keyword[] Function Insert(Keyword[] arr, Keyword toInsert, int index) global native

; Remove the toRemove Keyword from the array, decreasing its size. If all is true, remove all instances, otherwise, remove the first found instance.
Keyword[] Function Remove(Keyword[] arr, Keyword toRemove, bool all = true) global native

; remove the element at the index in the array, decreasing its size. If the index is out of bounds, removes the last element in the array.
Keyword[] Function RemoveAt(Keyword[] arr, int index) Global Native 

; remove all duplicates from the arr, making each element in the arr unique, and return the new array.
Keyword[] Function RemoveDuplicates(Keyword[] arr) Global Native 

; returns new array that contains the Keywords of the passed in akKeywords array, but sorted. 
; Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
; 1 = by Keyword name ascending, 
; 2 = by Keyword name descending, 
; 3 = by Keyword editor Id name ascending,
; 4 = by Keyword editor Id name descending,
; 5 = by Keyword Id ascending, 
; 6 = by Keyword Id descending
Keyword[] Function Sort(Keyword[] arr, int sortOption) Global Native 

; Get the strings for the passed in array. mode options are:
; 1 = Keyword names
; 2 = editor names 
; 3 = IDs as hexidecimal (KeywordId or KeywordId)
; SortOptions are: 
; 1 = not sorted 
; 2 = sorted ascending 
; 3 = sorted descending
String[] Function GetStrings(Keyword[] arr, int mode, int sortOption, string nullName = "NONE", string emptyName = " - ")  Global Native 

; count the number of times the item appears in the array.
int Function Count(Keyword[] arr, Keyword item) Global Native 

; return true if all of the elements in arr_A equal the elements in arr_B.
bool function IsEqual(Keyword[] arr_A, Keyword[] arr_B) Global Native 

; for linking arrays. Return an int array that contains all indexes in arr that match the value
; use with RemoveIndexes to remove all of value from the same arr, and remove the same indexes from another array.
; See the DbSkseMap_Alias_Keyword.psc script for example usage.
int[] function GetIndexes(Keyword[] arr, Keyword value) global native

; for linking arrays. Returns an int array containing all indexes of duplicate elements in the arr.
; use with RemoveIndexes to remove all duplicates from the same arr, and remove the same indexes from another array.
; See the DbSkseMap_Alias_Keyword.psc script for example usage.
int[] function GetRemoveDuplicatesIndexes(Keyword[] arr) global native

; Removes all indexes in the arr. Example, if indexes contains [0] = 2, [1] = 7, 
; Removes the elements that are currently at [2] and [7] from the arr. 
; See the DbSkseMap_Alias_Keyword.psc script for example usage.
Keyword[] Function RemoveIndexes(Keyword[] arr, int[] indexes) Global Native 

; for linking arrays, get new sorted index for the arr. 
; See the DbSkseMap_Alias_Keyword.psc script for example usage.
;  Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
;  1 = by Keyword name ascending, 
;  2 = by Keyword name descending, 
;  3 = by Keyword editor Id name ascending,
;  4 = by Keyword editor Id name descending,
;  5 = by Keyword Id ascending, 
;  6 = by Keyword Id descending
int[] Function GetSortIndexes(Keyword[] arr, int sortOption) Global Native 

; See the DbSkseMap_Alias_Keyword.psc script for example usage.
Keyword[] Function SortByIndexes(Keyword[] arr, int[] indexes) Global Native 

; Get the indexes of the postChangedArr in the original preChangedArr. Allows to alter another array the exact same way the preChangedArr was altered.
; See DbSkseArray_Form.GetChangedIndexes for an example.
int[] Function GetChangedIndexes(Keyword[] preChangedArr, Keyword[] posChangedArr) Global Native 

; Filter arr by indexes and return new array. See DbSkseArray_Form.GetChangedIndexes for more info.
Keyword[] Function ChangeByIndexes(Keyword[] arr, int[] indexes) Global Native 

;  ArrayAs functions, returns a new array where each element of the passed in array is cast as the return type. 
;  If removeNoneArrElements, any none entry in the passed in array is removed from the return array. 
;  If removeFailedToConvertElements, any valid entry in the passed in array that fails to convert to the return type is removed from the return array.
;  You can add as many ArrayAs functions here as you want, converting from any compatible type. 
;  Only vanilla types are supported. Any type found in KeywordType.psc plus bool, int, float, string, Keyword, Alias and ActiveMagicEffect.
;  Make sure the new function names contain ArrayAs.

;  Cast all Keyword array elements to Form and return new array.
Form[] Function ArrayAsForm(Keyword[] arr, bool removeNoneArrElements = true, bool removeFailedToConvertElements = true) Global Native
