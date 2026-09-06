scriptname DbSkseArray_Alias hidden 
; To create these array functions for different papyrus types, duplicate this script, 
; rename it and and replace all of Alias with a new type you want these array functions for, E.G TextureSet
; Compile the script and copy the source .psc file to Data/Scripts/Source/DbSkseArrays/ 
; IMPORTANT!!! You can change the type or add ArrayAs functions, but don't change the formatting because 
; the plugin gets papyrus types by reading this source .psc file. Specifically, no spaces between the function name and first left parentheses "("
; That is how the DbSkseFunctions.dll knows to register the functions and what type to register functions as.
; In your mod's download, include the .pex files for these scripts in Data/Scripts folder like you would a normal script. 
; Also include the Data/Scripts/Source/DbSkseArrays/ folder with the new source .psc files or the functions here won't work.
; You can only have one array type per script. If you need more types, duplicate more of these scripts in the same manner.
; The exception is that you can add as many ArrayAs functions as you like, as long as the types are compatible. More details below.
; Custom user made types are not supported. Only vanilla types are supported, E.G types found in
; FormType.psc as well as Alias, ReferenceAlias, LocationAlias and ActiveMagicEffect

; Create a new array of size, filled with optional filler.
Alias[] Function Create(int size, Alias filler = none) Global Native 

; Resize the passed in array. If the size is larger, fills the new indexes with filler. 
Alias[] Function Resize(Alias[] arr, int size, Alias filler = none) Global Native

; Merge the two arrays and return the new array containing both array's elements.
Alias[] Function Merge(Alias[] arr_A, Alias[] arr_B) Global Native

; Returns a sub section of the passed in array indicated by a starting and ending index.
; The default argument "int endIndex = -1" clamps the to the end of the array. Equivalent of setting EndIndex = (ArrayValues.Length - 1)
Alias[] function Slice(Alias[] ArrayValues, int startIndex, int endIndex = -1) global native

; Add the toPush form to the end of the arr, increasing its size.
Alias[] Function Push(Alias[] arr, Alias toPush) Global Native 

; insert the toInsert form into the array, increasing its size. If the index is out of bound, puts the form at the end of the array.
Alias[] Function Insert(Alias[] arr, Alias toInsert, int index) global native

; Remove the toRemove form from the array, decreasing its size. If all is true, remove all instances, otherwise, remove the first found instance.
Alias[] Function Remove(Alias[] arr, Alias toRemove, bool all = true) global native

; remove the element at the index in the array, decreasing its size. If the index is out of bounds, removes the last element in the array.
Alias[] Function RemoveAt(Alias[] arr, int index) Global Native 

; returns new array that contains the forms of the passed in akForms array, but sorted. 
; Sort options are as follows. Note, to sort by editor Id reliably, po3 tweaks must be installed.
; 1 = by form name ascending, 
; 2 = by form name descending, 
; 3 = by form editor Id name ascending,
; 4 = by form editor Id name descending,
; 5 = by form Id ascending, 
; 6 = by form Id descending
Alias[] Function Sort(Alias[] arr, int sortOption) Global Native 

; Get the strings for the passed in array. mode options are:
; 1 = form names
; 2 = editor names 
; 3 = IDs as hexidecimal (formId or AliasId)
; SortOptions are: 
; 1 = not sorted 
; 2 = sorted ascending 
; 3 = sorted descending
String[] Function GetStrings(Alias[] arr, int mode, int sortOption, string nullName = "NONE", string emptyName = " - ")  Global Native 

; count the number of times the item appears in the array.
int Function Count(Alias[] arr, Alias item) Global Native 

; ArrayAs functions, returns a new array where each element of the passed in array is cast as the return type. 
; If removeNoneArrElements, any none entry in the passed in array is removed from the return array. 
; If removeFailedToConvertElements, any valid entry in the passed in array that fails to convert to the return type is removed from the return array.
; You can add as many ArrayAs functions here as you want, converting from any compatible type.
; Make sure the new function names contains ArrayAs.

; Cast all Alias array elements to ReferenceAlias and return new array.
ReferenceAlias[] Function ArrayAsReferenceAlias(Alias[] arr, bool removeNoneArrElements = true, bool removeFailedToConvertElements = true) Global Native

; Cast all Alias array elements to LocationAlias and return new array.
LocationAlias[] Function ArrayAsLocationAlias(Alias[] arr, bool removeNoneArrElements = true, bool removeFailedToConvertElements = true) Global Native