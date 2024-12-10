scriptname DbBigStringArray extends ObjectReference 
{String array with max size of 16384 (128 * 128)
Can be used as a big array or multidimensional array without skse.
Attach this script and the DynamicStringArrays script to a new form in the creation kit. I recommend a misc object. 
To create big arrays of different types, duplicate this script and replace all "string" with "newType".
Also do the same thing with the DynamicStringArrays script. 
Example, you would have DbBigActorArray.psc and DynmaicActorArrays.psc scripts attached to the same form}

string[] property Array0 Auto Hidden
string[] property Array1 Auto Hidden
string[] property Array2 Auto Hidden
string[] property Array3 Auto Hidden
string[] property Array4 Auto Hidden
string[] property Array5 Auto Hidden
string[] property Array6 Auto Hidden
string[] property Array7 Auto Hidden
string[] property Array8 Auto Hidden
string[] property Array9 Auto Hidden
string[] property Array10 Auto Hidden
string[] property Array11 Auto Hidden
string[] property Array12 Auto Hidden
string[] property Array13 Auto Hidden
string[] property Array14 Auto Hidden
string[] property Array15 Auto Hidden
string[] property Array16 Auto Hidden
string[] property Array17 Auto Hidden
string[] property Array18 Auto Hidden
string[] property Array19 Auto Hidden
string[] property Array20 Auto Hidden
string[] property Array21 Auto Hidden
string[] property Array22 Auto Hidden
string[] property Array23 Auto Hidden
string[] property Array24 Auto Hidden
string[] property Array25 Auto Hidden
string[] property Array26 Auto Hidden
string[] property Array27 Auto Hidden
string[] property Array28 Auto Hidden
string[] property Array29 Auto Hidden
string[] property Array30 Auto Hidden
string[] property Array31 Auto Hidden
string[] property Array32 Auto Hidden
string[] property Array33 Auto Hidden
string[] property Array34 Auto Hidden
string[] property Array35 Auto Hidden
string[] property Array36 Auto Hidden
string[] property Array37 Auto Hidden
string[] property Array38 Auto Hidden
string[] property Array39 Auto Hidden
string[] property Array40 Auto Hidden
string[] property Array41 Auto Hidden
string[] property Array42 Auto Hidden
string[] property Array43 Auto Hidden
string[] property Array44 Auto Hidden
string[] property Array45 Auto Hidden
string[] property Array46 Auto Hidden
string[] property Array47 Auto Hidden
string[] property Array48 Auto Hidden
string[] property Array49 Auto Hidden
string[] property Array50 Auto Hidden
string[] property Array51 Auto Hidden
string[] property Array52 Auto Hidden
string[] property Array53 Auto Hidden
string[] property Array54 Auto Hidden
string[] property Array55 Auto Hidden
string[] property Array56 Auto Hidden
string[] property Array57 Auto Hidden
string[] property Array58 Auto Hidden
string[] property Array59 Auto Hidden
string[] property Array60 Auto Hidden
string[] property Array61 Auto Hidden
string[] property Array62 Auto Hidden
string[] property Array63 Auto Hidden
string[] property Array64 Auto Hidden
string[] property Array65 Auto Hidden
string[] property Array66 Auto Hidden
string[] property Array67 Auto Hidden
string[] property Array68 Auto Hidden
string[] property Array69 Auto Hidden
string[] property Array70 Auto Hidden
string[] property Array71 Auto Hidden
string[] property Array72 Auto Hidden
string[] property Array73 Auto Hidden
string[] property Array74 Auto Hidden
string[] property Array75 Auto Hidden
string[] property Array76 Auto Hidden
string[] property Array77 Auto Hidden
string[] property Array78 Auto Hidden
string[] property Array79 Auto Hidden
string[] property Array80 Auto Hidden
string[] property Array81 Auto Hidden
string[] property Array82 Auto Hidden
string[] property Array83 Auto Hidden
string[] property Array84 Auto Hidden
string[] property Array85 Auto Hidden
string[] property Array86 Auto Hidden
string[] property Array87 Auto Hidden
string[] property Array88 Auto Hidden
string[] property Array89 Auto Hidden
string[] property Array90 Auto Hidden
string[] property Array91 Auto Hidden
string[] property Array92 Auto Hidden
string[] property Array93 Auto Hidden
string[] property Array94 Auto Hidden
string[] property Array95 Auto Hidden
string[] property Array96 Auto Hidden
string[] property Array97 Auto Hidden
string[] property Array98 Auto Hidden
string[] property Array99 Auto Hidden
string[] property Array100 Auto Hidden
string[] property Array101 Auto Hidden
string[] property Array102 Auto Hidden
string[] property Array103 Auto Hidden
string[] property Array104 Auto Hidden
string[] property Array105 Auto Hidden
string[] property Array106 Auto Hidden
string[] property Array107 Auto Hidden
string[] property Array108 Auto Hidden
string[] property Array109 Auto Hidden
string[] property Array110 Auto Hidden
string[] property Array111 Auto Hidden
string[] property Array112 Auto Hidden
string[] property Array113 Auto Hidden
string[] property Array114 Auto Hidden
string[] property Array115 Auto Hidden
string[] property Array116 Auto Hidden
string[] property Array117 Auto Hidden
string[] property Array118 Auto Hidden
string[] property Array119 Auto Hidden
string[] property Array120 Auto Hidden
string[] property Array121 Auto Hidden
string[] property Array122 Auto Hidden
string[] property Array123 Auto Hidden
string[] property Array124 Auto Hidden
string[] property Array125 Auto Hidden
string[] property Array126 Auto Hidden
string[] property Array127 Auto Hidden

int Property size = 0 Auto Hidden
int Property NumberOfArrays = 128 Auto Hidden

int Property MaxSize = 16384 AutoReadOnly Hidden
int Property MaxNumberOfArrays = 128 AutoReadOnly Hidden

;for GetNext, SetNext, GetPrevious, SetPrevious functions
String[] CurrentSubArray
Int CurrentIndex = -1
Int CurrentSubArrayIndex = -1
Int CurrentArrayIndex = -1

;for faster pushBack
String[] CurrentSizeSubArray
int CurrentSizeSubArrayIndex = -1
Int CurrentSizeArrayIndex = -1

bool busy = false 
bool property iterating = false auto

DynamicStringArrays property stringArrays auto 

Event OnInit()
    stringArrays = GetBaseObject() as DynamicStringArrays
EndEvent

bool function isBusy()
    return busy
EndFunction

function waitWhileBusy(float waitInterval = 0.1)
    while busy
        Utility.waitMenuMode(waitInterval)
    EndWhile
EndFunction

bool function IsIterating()
    return iterating
EndFunction

function waitWhileIterating(float waitInterval = 0.1)
    while iterating
        Utility.waitMenuMode(waitInterval)
    EndWhile
EndFunction

function waitForState(string akState, float waitInterval = 0.1)
    int count = 100
    while GetState() != akState && count > 0
        count -= 1
        Utility.waitMenuMode(waitInterval)
    EndWhile
EndFunction 

;set the current index (between -1 and size)
;for use with getNext, SetNext, GetPrevious, SetPrevious functions
function SetCurrentIndex(int index)
    index = DbMiscfunctions.ClampInt(index, -1, size)
    currentIndex = index 

    int[] indexes

    if index == -1
        indexes = new int[2]
        indexes[0] = 0 ;sub array index 
        indexes[1] = -1 ;CurrentArrayIndex 

    elseif index == size 
        indexes = GetSubIndexesForIndex(size - 1)
        indexes[1] = (indexes[1] + 1)
    Else 
        indexes = GetSubIndexesForIndex(index)
    Endif

    CurrentSubArrayIndex = indexes[0]
    CurrentSubArray = GetNthSubArray(CurrentSubArrayIndex)
    CurrentArrayIndex = indexes[1]
Endfunction

;don't use, for internal use only
function SetCurrentSizeVariables(bool forceSetArray = false)
    int[] indexes = GetSubIndexesForIndex(size)
    if CurrentSizeSubArrayIndex != indexes[0] || forceSetArray
        CurrentSizeSubArrayIndex = indexes[0]
        CurrentSizeSubArray = GetNthSubArray(CurrentSizeSubArrayIndex)
    Endif
    CurrentSizeArrayIndex = indexes[1]
Endfunction

;CurrentIndex is used for getNext, SetNext, GetPrevious, SetPrevious functions
int function GetCurrentIndex()
    return CurrentIndex
EndFunction

;CurrentIndex is used for getNext, SetNext, GetPrevious, SetPrevious functions
int function GetCurrentArrayIndex()
    return CurrentArrayIndex
EndFunction

;CurrentIndex is used for getNext, SetNext, GetPrevious, SetPrevious functions
int function GetCurrentSubArrayIndex()
    return CurrentSubArrayIndex
EndFunction

;current sub array set internally. (Array0, Array1, Array2 ect. Matches the CurrentSubArrayIndex)
;used for getNext, SetNext, GetPrevious, SetPrevious functions
String[] function GetCurrentSubArray()
    return CurrentSubArray
EndFunction 

int function GetSize()
    return size
Endfunction

int function GetMaxSize()
    return MaxSize
EndFunction

int function GetNumberOfArrays()
    return NumberOfArrays
EndFunction

int function GetMaxNumberOfArrays()
    return MaxNumberOfArrays
EndFunction

;create a multi dimensional array with numberOfSubArrays of subArraySize
;BigArrayForm should have this script attached and the DynamicStringArrays script attached
DbBigStringArray function CreateMultiArray(Form BigArrayForm, int numberOfSubArrays = 1, int subArraySize = 1, string fillElement = "", bool persistent = true, bool abInitiallyDisabled = true) Global
    if !BigArrayForm
        ; Debug.notification("no BigArrayForm, aborting")
        return none 
    endif 

    ObjectReference ref = Game.GetPlayer().PlaceAtMe(BigArrayForm, 1, persistent, abInitiallyDisabled)
    DbBigStringArray bigArray = ref as DbBigStringArray
    if bigArray 
        if !bigArray.stringArrays
            bigArray.stringArrays = BigArrayForm as DynamicStringArrays
        Endif 

        if !bigArray.stringArrays
            return none 
        Endif

        numberOfSubArrays = DbMiscFunctions.clampInt(numberOfSubArrays, 1, bigArray.MaxNumberOfArrays)
        subArraySize = DbMiscFunctions.clampInt(subArraySize, 1, 128)
        bigArray.NumberOfArrays = numberOfSubArrays

        ; Debug.MessageBox("bigArray.NumberOfArrays = " + bigArray.NumberOfArrays + "\nnumberOfSubArrays = " + numberOfSubArrays \
        ; + "\nsubArraySize = " + subArraySize)
        ; Utility.wait(0.1)

        if fillElement
            int outerIndex = 0 
            while outerIndex < numberOfSubArrays
                string[] subArray = bigArray.GetNthSubArray(outerIndex, subArraySize) ;create the sub array at outerIndex of subArraySize
                int i = 0 
                while i < subArray.length 
                    subArray[i] = fillElement
                    i += 1
                EndWhile
                outerIndex += 1
            EndWhile
        Else 
            int outerIndex = 0 
            while outerIndex < numberOfSubArrays
                bigArray.GetNthSubArray(outerIndex, subArraySize) ;create the sub array at outerIndex of subArraySize
                outerIndex += 1
            EndWhile
        Endif
        
        return bigArray
    Else 
        ref.disable()
        ref.delete()
        return none 
    Endif
EndFunction

;create a big string array with a max size of 16384
;this script and the DynamicStringArrays script should be attached to the BigArrayForm
DbBigStringArray function Create(Form BigArrayForm, int akSize = 0, string fillElement = "", bool persistent = true, bool abInitiallyDisabled = true) Global
    ; Debug.notification("creating big string array")
    if !BigArrayForm
        ; Debug.notification("no BigArrayForm, aborting")
        return none 
    endif 

    ObjectReference ref = Game.GetPlayer().PlaceAtMe(BigArrayForm, 1, persistent, abInitiallyDisabled)
    DbBigStringArray bigArray = ref as DbBigStringArray
    if bigArray 
        akSize = DbMiscFunctions.ClampInt(akSize, 0, bigArray.maxSize)
        if fillElement
            ; Debug.notification("filling array with " + fillElement + " size is " + akSize)
            bigArray.resize(akSize, fillElement)
        else 
            int[] indexes = bigArray.GetSubIndexesForIndex(akSize)
            
            bigArray.size = akSize

            int i = 0
            string[] akArray
            while i <= indexes[0]
                akArray = bigArray.GetNthSubArray(i, 128)
                i += 1
            EndWhile 

            ; Debug.MessageBox("creating array, no fill element, size = " + bigArray.size + "\n subArrays = " + indexes[0] + \
            ; "\narray index = " + indexes[1] + "\narray0.length = " + bigArray.array0.length)
            ; Utility.wait(0.1)

            bigArray.SetCurrentSizeVariables()
            bigArray.SetCurrentIndex(0)
        Endif

        ; Debug.MessageBox("returning big string array")
        ; Utility.wait(0.1)
        return bigArray
    else 
        ; Debug.notification("big string array script not attached, returning none")
        ref.disable()
        ref.delete()
        return none 
    Endif
EndFunction 

bool function resize(int newSize, string fillElement = "")
    ; Debug.MessageBox("resize size from " + size + " to " + newSize)
    ; Utility.wait(0.1)
    newSize = DbMiscFunctions.ClampInt(newSize, 0, MaxSize)
    waitWhileBusy()
    busy = true

    if newSize > size ;increase size
        ; Debug.MessageBox("increase size from " + size + " to " + newSize)
        ; Utility.wait(0.1)
        int[] oldIndexes = GetSubIndexesForIndex(size)
        int subArrayIndex = oldIndexes[0]
        int i = oldIndexes[1]
        string[] subArray = GetNthSubArray(subArrayIndex)
        if subArray.length < 128 
            subArray = GetNthSubArray(subArrayIndex, 128)
        Endif 

        while size < newSize
            ; Debug.MessageBox("subArrayIndex = " + subArrayIndex)
            ; Utility.wait(0.1)
            while i < 128 && size < newSize
                subArray[i] = fillElement
                size += 1
                i += 1
            EndWhile 

            if size < newSize
                i = 0 
                subArrayIndex += 1
                subArray = GetNthSubArray(subArrayIndex)
                if subArray.length < 128 
                    subArray = GetNthSubArray(subArrayIndex, 128)
                Endif 
            endif
        EndWhile

        SetCurrentSizeVariables(true)

    elseif newSize < size ;decrease size
        int[] newIndexes = GetSubIndexesForIndex(newSize)
        int[] oldIndexes = GetSubIndexesForIndex(size)

        int newSubArrayIndex = newIndexes[0]
        int oldSubArrayIndex = oldIndexes[0]
        int sizeSubtractAmount = oldIndexes[1]
        string[] subArray

        ; Debug.MessageBox("Decreasing size from " + size + " to " + newsize + \
        ; "\noldSubArrayIndex = " + oldSubArrayIndex + " newSubArrayIndex = " + newSubArrayIndex)
        ; Utility.wait(0.1)

        while oldSubArrayIndex > newSubArrayIndex
            subArray = GetNthSubArray(oldSubArrayIndex, 1) ;clear unused subArrays
            size -= sizeSubtractAmount
            oldSubArrayIndex -= 1
            sizeSubtractAmount = 128
        EndWhile

        subArray = GetNthSubArray(newSubArrayIndex)
        if subArray.length < 128 
            subArray = GetNthSubArray(newSubArrayIndex, 128) ;init subArray size to 128
        else 
            int i = 128
            while i > newIndexes[1]
                i -= 1 
                size -= 1
                subArray[i] = ""
            EndWhile
        Endif 

        SetCurrentSizeVariables(true)
    Endif

    busy = false
    return (size == newSize)
EndFunction

int function Find(string toFind)
    ; Debug.messageBox("finding " + toFind)
    ; Utility.wait(0.1)

    int subArrayIndex = 0 
    int index = -1
    while subArrayIndex < MaxNumberOfArrays && index == -1
        string[] subArray = GetNthSubArray(subArrayIndex)
        if subArray.length > 0
            index = subArray.find(toFind)
        Endif
        subArrayIndex += 1
    EndWhile

    if index > -1 
        subArrayIndex -= 1
        index = ((subArrayIndex * 128) + index)
    Endif

    return index
EndFunction

int function RFind(string toFind)
    int subArrayIndex = CurrentSizeSubArrayIndex
    int index = -1
    while subArrayIndex >= 0 && index == -1
        string[] subArray = GetNthSubArray(subArrayIndex)
        if subArray.length > -1
            index = subArray.Rfind(toFind)
        Endif
        subArrayIndex -= 1
    EndWhile

    if index > -1 
        subArrayIndex += 1
        index = ((subArrayIndex * 128) + index)
    Endif

    return index
EndFunction

bool function pushBack(string element)
    if size >= maxSize 
        return false 
    endif 

    waitWhileBusy()
    busy = true

    CurrentSizeSubArray[CurrentSizeArrayIndex] = element

    size += 1 
    CurrentSizeArrayIndex += 1 

    if CurrentSizeArrayIndex >= 128 
        CurrentSizeArrayIndex = 0
        CurrentSizeSubArrayIndex += 1
        CurrentSizeSubArray = GetNthSubArray(CurrentSizeSubArrayIndex, 128) ;set CurrentSizeSubArray and initialize the new array to 128 length
    Endif

    busy = false
    return true
EndFunction

;get the next element in the big array. Add 1 to currentIndex and get the element
;if the current index is already at the last valid element (size - 1), goes to the first index in the array (0).
String Function GetNext()
    CurrentIndex += 1
    if CurrentIndex >= size 
        CurrentIndex = 0
        CurrentSubArrayIndex = 0
        CurrentSubArray = GetNthSubArray(CurrentSubArrayIndex)
        CurrentArrayIndex = -1
    Endif 

    CurrentArrayIndex += 1 
    if CurrentArrayIndex >= 128 
        CurrentArrayIndex = 0 
        CurrentSubArrayIndex += 1 
        CurrentSubArray = GetNthSubArray(CurrentSubArrayIndex)
    Endif 

    return CurrentSubArray[CurrentArrayIndex]
Endfunction

;set the next element in the big array. Add 1 to currentIndex and set the element
;if the current index is already at the last valid element (size - 1), goes to the first index in the array (0).
Function SetNext(string element)
    CurrentIndex += 1

    if CurrentIndex >= size 
        CurrentIndex = 0
        CurrentSubArrayIndex = 0
        CurrentSubArray = GetNthSubArray(CurrentSubArrayIndex)
        CurrentArrayIndex = -1
    Endif 

    CurrentArrayIndex += 1 
    if CurrentArrayIndex >= 128 
        CurrentArrayIndex = 0 
        CurrentSubArrayIndex += 1 
        CurrentSubArray = GetNthSubArray(CurrentSubArrayIndex)
    Endif 

    CurrentSubArray[CurrentArrayIndex] = element
EndFunction

;Get the previous element in the big array. Subtract 1 to currentIndex and get the element
;if the current index is at the first element (0), goes to the last valid index in the array (size - 1).
String Function GetPrevious()
    CurrentIndex -= 1
    if CurrentIndex < 0
        CurrentIndex = (size - 1)
        int[] indexes = GetSubIndexesForIndex(CurrentIndex)
        CurrentSubArrayIndex = indexes[0]
        CurrentSubArray = GetNthSubArray(CurrentSubArrayIndex)
        CurrentArrayIndex = (indexes[1] + 1)
    Endif 

    CurrentArrayIndex -= 1 
    if CurrentArrayIndex < 0
        CurrentArrayIndex = 127
        CurrentSubArrayIndex -= 1 
        CurrentSubArray = GetNthSubArray(CurrentSubArrayIndex)
    Endif 

    return CurrentSubArray[CurrentArrayIndex]
Endfunction 

;Set the previous element in the big array. Subtract 1 to currentIndex and set the element
;if the current index is at the first element (0), goes to the last valid index in the array (size - 1).
Function SetPrevious(string element)
    CurrentIndex -= 1
    if CurrentIndex < 0
        CurrentIndex = (size - 1)
        int[] indexes = GetSubIndexesForIndex(CurrentIndex)
        CurrentSubArrayIndex = indexes[0]
        CurrentSubArray = GetNthSubArray(CurrentSubArrayIndex)
        CurrentArrayIndex = (indexes[1] + 1)
    Endif 

    CurrentArrayIndex -= 1 
    if CurrentArrayIndex < 0
        CurrentArrayIndex = 127
        CurrentSubArrayIndex -= 1 
        CurrentSubArray = GetNthSubArray(CurrentSubArrayIndex)
    Endif 

    CurrentSubArray[CurrentArrayIndex] = element
Endfunction

;Set the element at the index in the big array and set CurrentIndex to index (for getNext, setNext, GetPrevious and SetPrevious functions)
string function GetAt(int index)
    if index >= size || index < 0
        ; Debug.Messagebox("GetAt: index = " + index + " size = " + size + " returning null")
        ; Utility.wait(0.1)
        return "" 
    Endif

    int[] indexes = GetSubIndexesForIndex(index)
    CurrentSubArray = GetNthSubArray(indexes[0])
    CurrentSubArrayIndex = indexes[0]
    CurrentArrayIndex = indexes[1]
    CurrentIndex = index
    ; debug.messageBox("GetAt: index = " + index + "\nsubArray index = " + indexes[0] + "\narray index = " + indexes[1] + "\narray size = " + subArray.length)
    ; Utility.wait(0.1)
    return CurrentSubArray[CurrentArrayIndex] 
EndFunction

;Set the element at the index in the large array and set CurrentIndex (for getNext, setNext, GetPrevious and SetPrevious functions)
bool function SetAt(int index, string element)
    if index >= size || index < 0
        ; Debug.Messagebox("GetAt: index = " + index + " size = " + size + " returning null")
        ; Utility.wait(0.1)
        return false 
    Endif

    int[] indexes = GetSubIndexesForIndex(index)
    CurrentSubArray = GetNthSubArray(indexes[0])
    CurrentSubArrayIndex = indexes[0]
    CurrentArrayIndex = indexes[1]
    CurrentIndex = index

    ; debug.messageBox("GetAt: index = " + index + "\nsubArray index = " + indexes[0] + "\narray index = " + indexes[1] + "\narray size = " + subArray.length)
    ; Utility.wait(0.1)
    CurrentSubArray[CurrentArrayIndex] = element
    return true
EndFunction

;use sparingly, pushback is much faster.
bool function InsertAt(int index, string element)
    if index == size 
        return pushback(element)
    Elseif index > size 
        return false 
    elseif index < 0
        return false 
    endif 

    waitWhileBusy()
    busy = true
    bool success = true
    
    size += 1
    int[] indexes = GetSubIndexesForIndex(index)
    int subArrayIndex = indexes[0]
    int i = indexes[1]
    string[] subArray = GetNthSubArray(subArrayIndex)
    string akElement = element
    string previousElement

    while index < size && subArrayIndex < MaxNumberOfArrays
        while i < 128 
            previousElement = subArray[i]
            subArray[i] = akElement 
            akElement = previousElement
            index += 1
            i += 1
        EndWhile

        if index < size 
            i = 0 
            subArrayIndex += 1 
            subArray = GetNthSubArray(subArrayIndex)
            if subArray.length < 128 
                subArray = GetNthSubArray(subArrayIndex, 128) ;set the size of sub array to 128
            Endif

            if subArray.length < 128  ;something went wrong
                success = false
                index = size ;break the loop
            Endif
        Endif
    EndWhile

    SetCurrentSizeVariables(true)
    busy = false
    return success
EndFunction

;get the last element of the big array and remove it, reducing the size by 1
String function Pop()
    int newSize = (size - 1)
    string element = GetAt(newSize)
    resize(newSize)
    return element
EndFunction

;remove the string at the index, reducing the size by 1 and moving each element after the index back by 1. 
;returns the string that's currently at the index
String function RemoveAt(int index)
    if index >= size 
        return "" 
    endif 

    waitWhileBusy()
    busy = true

    int[] indexes = GetSubIndexesForIndex(index)
    int subArrayIndex = indexes[0]
    int i = indexes[1]
    string[] subArray = GetNthSubArray(subArrayIndex)
    string returnString = subArray[i]

    while index < size && subArrayIndex < MaxNumberOfArrays
        while i < 127 && index < size 
            subArray[i] = subArray[i + 1]
            i += 1
            index += 1
        EndWhile

        if index < size 
            i = 0 
            subArrayIndex += 1 
            string[] lastArray = subArray
            subArray = GetNthSubArray(subArrayIndex)
            lastArray[127] = subArray[0]
        endif
    EndWhile

    size -= 1
    SetCurrentSizeVariables()
    busy = false
    return returnString
EndFunction

function Clear()
    waitWhileBusy()
    busy = true

    int i = 0
    while i < MaxNumberOfArrays
        string[] akArray = GetNthSubArray(i)
        if akArray.length > 1 
            akArray = GetNthSubArray(i, 1) ;clear sub array to size 1
        Endif
        i += 1 
    EndWhile

    size = 0
    SetCurrentSizeVariables()
    busy = false
EndFunction

function Destroy()
    Clear()
    Disable()
    Delete()
EndFunction

int[] function GetSubIndexesForIndex(int index)
    int[] indexes = new int[2]
    indexes[0] = (index / 128)
    indexes[1] = (index - (indexes[0] * 128))
    ; Debug.MessageBox("indexes for index [" + index + "] = " + indexes)
    Utility.wait(0.1)
    return indexes
EndFunction

;get Nth array in this object (0 to 100)
;if akSize > 0, set's the size of subArray to akSize before returning
String[] Function GetNthSubArray(int index, int akSize = 0)
    if index <= 0
        GoToState("")
    Else
        GoToState(("A" + index))
    Endif
    
    string[] akArray = GetArray(akSize)
    ; Debug.MessageBox("GetNthSubArray index = " + index + " akSize = " + akSize + " akArray.length = " + akArray.length)
    ; Utility.wait(0.1)
    return akArray
EndFunction

String[] Function GetArray(int akSize = 0)
    if akSize > 0 
        Array0 = stringArrays.CreateArray(akSize)
    Endif
    return Array0
EndFunction 

State A1
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array1 = stringArrays.CreateArray(akSize)
        Endif
        return Array1
    EndFunction
EndState

State A2
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array2 = stringArrays.CreateArray(akSize)
        Endif
        return Array2
    EndFunction
EndState

State A3
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array3 = stringArrays.CreateArray(akSize)
        Endif
        return Array3
    EndFunction
EndState

State A4
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array4 = stringArrays.CreateArray(akSize)
        Endif
        return Array4
    EndFunction
EndState

State A5
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array5 = stringArrays.CreateArray(akSize)
        Endif
        return Array5
    EndFunction
EndState

State A6
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array6 = stringArrays.CreateArray(akSize)
        Endif
        return Array6
    EndFunction
EndState

State A7
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array7 = stringArrays.CreateArray(akSize)
        Endif
        return Array7
    EndFunction
EndState

State A8
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array8 = stringArrays.CreateArray(akSize)
        Endif
        return Array8
    EndFunction
EndState

State A9
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array9 = stringArrays.CreateArray(akSize)
        Endif
        return Array9
    EndFunction
EndState

State A10
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array10 = stringArrays.CreateArray(akSize)
        Endif
        return Array10
    EndFunction
EndState

State A11
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array11 = stringArrays.CreateArray(akSize)
        Endif
        return Array11
    EndFunction
EndState

State A12
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array12 = stringArrays.CreateArray(akSize)
        Endif
        return Array12
    EndFunction
EndState

State A13
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array13 = stringArrays.CreateArray(akSize)
        Endif
        return Array13
    EndFunction
EndState

State A14
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array14 = stringArrays.CreateArray(akSize)
        Endif
        return Array14
    EndFunction
EndState

State A15
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array15 = stringArrays.CreateArray(akSize)
        Endif
        return Array15
    EndFunction
EndState

State A16
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array16 = stringArrays.CreateArray(akSize)
        Endif
        return Array16
    EndFunction
EndState

State A17
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array17 = stringArrays.CreateArray(akSize)
        Endif
        return Array17
    EndFunction
EndState

State A18
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array18 = stringArrays.CreateArray(akSize)
        Endif
        return Array18
    EndFunction
EndState

State A19
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array19 = stringArrays.CreateArray(akSize)
        Endif
        return Array19
    EndFunction
EndState

State A20
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array20 = stringArrays.CreateArray(akSize)
        Endif
        return Array20
    EndFunction
EndState

State A21
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array21 = stringArrays.CreateArray(akSize)
        Endif
        return Array21
    EndFunction
EndState

State A22
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array22 = stringArrays.CreateArray(akSize)
        Endif
        return Array22
    EndFunction
EndState

State A23
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array23 = stringArrays.CreateArray(akSize)
        Endif
        return Array23
    EndFunction
EndState

State A24
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array24 = stringArrays.CreateArray(akSize)
        Endif
        return Array24
    EndFunction
EndState

State A25
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array25 = stringArrays.CreateArray(akSize)
        Endif
        return Array25
    EndFunction
EndState

State A26
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array26 = stringArrays.CreateArray(akSize)
        Endif
        return Array26
    EndFunction
EndState

State A27
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array27 = stringArrays.CreateArray(akSize)
        Endif
        return Array27
    EndFunction
EndState

State A28
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array28 = stringArrays.CreateArray(akSize)
        Endif
        return Array28
    EndFunction
EndState

State A29
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array29 = stringArrays.CreateArray(akSize)
        Endif
        return Array29
    EndFunction
EndState

State A30
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array30 = stringArrays.CreateArray(akSize)
        Endif
        return Array30
    EndFunction
EndState

State A31
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array31 = stringArrays.CreateArray(akSize)
        Endif
        return Array31
    EndFunction
EndState

State A32
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array32 = stringArrays.CreateArray(akSize)
        Endif
        return Array32
    EndFunction
EndState

State A33
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array33 = stringArrays.CreateArray(akSize)
        Endif
        return Array33
    EndFunction
EndState

State A34
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array34 = stringArrays.CreateArray(akSize)
        Endif
        return Array34
    EndFunction
EndState

State A35
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array35 = stringArrays.CreateArray(akSize)
        Endif
        return Array35
    EndFunction
EndState

State A36
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array36 = stringArrays.CreateArray(akSize)
        Endif
        return Array36
    EndFunction
EndState

State A37
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array37 = stringArrays.CreateArray(akSize)
        Endif
        return Array37
    EndFunction
EndState

State A38
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array38 = stringArrays.CreateArray(akSize)
        Endif
        return Array38
    EndFunction
EndState

State A39
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array39 = stringArrays.CreateArray(akSize)
        Endif
        return Array39
    EndFunction
EndState

State A40
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array40 = stringArrays.CreateArray(akSize)
        Endif
        return Array40
    EndFunction
EndState

State A41
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array41 = stringArrays.CreateArray(akSize)
        Endif
        return Array41
    EndFunction
EndState

State A42
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array42 = stringArrays.CreateArray(akSize)
        Endif
        return Array42
    EndFunction
EndState

State A43
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array43 = stringArrays.CreateArray(akSize)
        Endif
        return Array43
    EndFunction
EndState

State A44
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array44 = stringArrays.CreateArray(akSize)
        Endif
        return Array44
    EndFunction
EndState

State A45
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array45 = stringArrays.CreateArray(akSize)
        Endif
        return Array45
    EndFunction
EndState

State A46
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array46 = stringArrays.CreateArray(akSize)
        Endif
        return Array46
    EndFunction
EndState

State A47
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array47 = stringArrays.CreateArray(akSize)
        Endif
        return Array47
    EndFunction
EndState

State A48
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array48 = stringArrays.CreateArray(akSize)
        Endif
        return Array48
    EndFunction
EndState

State A49
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array49 = stringArrays.CreateArray(akSize)
        Endif
        return Array49
    EndFunction
EndState

State A50
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array50 = stringArrays.CreateArray(akSize)
        Endif
        return Array50
    EndFunction
EndState

State A51
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array51 = stringArrays.CreateArray(akSize)
        Endif
        return Array51
    EndFunction
EndState

State A52
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array52 = stringArrays.CreateArray(akSize)
        Endif
        return Array52
    EndFunction
EndState

State A53
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array53 = stringArrays.CreateArray(akSize)
        Endif
        return Array53
    EndFunction
EndState

State A54
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array54 = stringArrays.CreateArray(akSize)
        Endif
        return Array54
    EndFunction
EndState

State A55
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array55 = stringArrays.CreateArray(akSize)
        Endif
        return Array55
    EndFunction
EndState

State A56
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array56 = stringArrays.CreateArray(akSize)
        Endif
        return Array56
    EndFunction
EndState

State A57
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array57 = stringArrays.CreateArray(akSize)
        Endif
        return Array57
    EndFunction
EndState

State A58
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array58 = stringArrays.CreateArray(akSize)
        Endif
        return Array58
    EndFunction
EndState

State A59
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array59 = stringArrays.CreateArray(akSize)
        Endif
        return Array59
    EndFunction
EndState

State A60
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array60 = stringArrays.CreateArray(akSize)
        Endif
        return Array60
    EndFunction
EndState

State A61
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array61 = stringArrays.CreateArray(akSize)
        Endif
        return Array61
    EndFunction
EndState

State A62
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array62 = stringArrays.CreateArray(akSize)
        Endif
        return Array62
    EndFunction
EndState

State A63
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array63 = stringArrays.CreateArray(akSize)
        Endif
        return Array63
    EndFunction
EndState

State A64
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array64 = stringArrays.CreateArray(akSize)
        Endif
        return Array64
    EndFunction
EndState

State A65
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array65 = stringArrays.CreateArray(akSize)
        Endif
        return Array65
    EndFunction
EndState

State A66
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array66 = stringArrays.CreateArray(akSize)
        Endif
        return Array66
    EndFunction
EndState

State A67
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array67 = stringArrays.CreateArray(akSize)
        Endif
        return Array67
    EndFunction
EndState

State A68
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array68 = stringArrays.CreateArray(akSize)
        Endif
        return Array68
    EndFunction
EndState

State A69
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array69 = stringArrays.CreateArray(akSize)
        Endif
        return Array69
    EndFunction
EndState

State A70
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array70 = stringArrays.CreateArray(akSize)
        Endif
        return Array70
    EndFunction
EndState

State A71
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array71 = stringArrays.CreateArray(akSize)
        Endif
        return Array71
    EndFunction
EndState

State A72
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array72 = stringArrays.CreateArray(akSize)
        Endif
        return Array72
    EndFunction
EndState

State A73
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array73 = stringArrays.CreateArray(akSize)
        Endif
        return Array73
    EndFunction
EndState

State A74
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array74 = stringArrays.CreateArray(akSize)
        Endif
        return Array74
    EndFunction
EndState

State A75
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array75 = stringArrays.CreateArray(akSize)
        Endif
        return Array75
    EndFunction
EndState

State A76
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array76 = stringArrays.CreateArray(akSize)
        Endif
        return Array76
    EndFunction
EndState

State A77
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array77 = stringArrays.CreateArray(akSize)
        Endif
        return Array77
    EndFunction
EndState

State A78
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array78 = stringArrays.CreateArray(akSize)
        Endif
        return Array78
    EndFunction
EndState

State A79
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array79 = stringArrays.CreateArray(akSize)
        Endif
        return Array79
    EndFunction
EndState

State A80
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array80 = stringArrays.CreateArray(akSize)
        Endif
        return Array80
    EndFunction
EndState

State A81
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array81 = stringArrays.CreateArray(akSize)
        Endif
        return Array81
    EndFunction
EndState

State A82
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array82 = stringArrays.CreateArray(akSize)
        Endif
        return Array82
    EndFunction
EndState

State A83
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array83 = stringArrays.CreateArray(akSize)
        Endif
        return Array83
    EndFunction
EndState

State A84
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array84 = stringArrays.CreateArray(akSize)
        Endif
        return Array84
    EndFunction
EndState

State A85
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array85 = stringArrays.CreateArray(akSize)
        Endif
        return Array85
    EndFunction
EndState

State A86
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array86 = stringArrays.CreateArray(akSize)
        Endif
        return Array86
    EndFunction
EndState

State A87
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array87 = stringArrays.CreateArray(akSize)
        Endif
        return Array87
    EndFunction
EndState

State A88
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array88 = stringArrays.CreateArray(akSize)
        Endif
        return Array88
    EndFunction
EndState

State A89
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array89 = stringArrays.CreateArray(akSize)
        Endif
        return Array89
    EndFunction
EndState

State A90
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array90 = stringArrays.CreateArray(akSize)
        Endif
        return Array90
    EndFunction
EndState

State A91
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array91 = stringArrays.CreateArray(akSize)
        Endif
        return Array91
    EndFunction
EndState

State A92
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array92 = stringArrays.CreateArray(akSize)
        Endif
        return Array92
    EndFunction
EndState

State A93
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array93 = stringArrays.CreateArray(akSize)
        Endif
        return Array93
    EndFunction
EndState

State A94
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array94 = stringArrays.CreateArray(akSize)
        Endif
        return Array94
    EndFunction
EndState

State A95
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array95 = stringArrays.CreateArray(akSize)
        Endif
        return Array95
    EndFunction
EndState

State A96
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array96 = stringArrays.CreateArray(akSize)
        Endif
        return Array96
    EndFunction
EndState

State A97
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array97 = stringArrays.CreateArray(akSize)
        Endif
        return Array97
    EndFunction
EndState

State A98
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array98 = stringArrays.CreateArray(akSize)
        Endif
        return Array98
    EndFunction
EndState

State A99
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array99 = stringArrays.CreateArray(akSize)
        Endif
        return Array99
    EndFunction
EndState

State A100
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array100 = stringArrays.CreateArray(akSize)
        Endif
        return Array100
    EndFunction
EndState

State A101
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array101 = stringArrays.CreateArray(akSize)
        Endif
        return Array101
    EndFunction
EndState

State A102
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array102 = stringArrays.CreateArray(akSize)
        Endif
        return Array102
    EndFunction
EndState

State A103
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array103 = stringArrays.CreateArray(akSize)
        Endif
        return Array103
    EndFunction
EndState

State A104
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array104 = stringArrays.CreateArray(akSize)
        Endif
        return Array104
    EndFunction
EndState

State A105
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array105 = stringArrays.CreateArray(akSize)
        Endif
        return Array105
    EndFunction
EndState

State A106
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array106 = stringArrays.CreateArray(akSize)
        Endif
        return Array106
    EndFunction
EndState

State A107
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array107 = stringArrays.CreateArray(akSize)
        Endif
        return Array107
    EndFunction
EndState

State A108
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array108 = stringArrays.CreateArray(akSize)
        Endif
        return Array108
    EndFunction
EndState

State A109
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array109 = stringArrays.CreateArray(akSize)
        Endif
        return Array109
    EndFunction
EndState

State A110
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array110 = stringArrays.CreateArray(akSize)
        Endif
        return Array110
    EndFunction
EndState

State A111
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array111 = stringArrays.CreateArray(akSize)
        Endif
        return Array111
    EndFunction
EndState

State A112
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array112 = stringArrays.CreateArray(akSize)
        Endif
        return Array112
    EndFunction
EndState

State A113
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array113 = stringArrays.CreateArray(akSize)
        Endif
        return Array113
    EndFunction
EndState

State A114
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array114 = stringArrays.CreateArray(akSize)
        Endif
        return Array114
    EndFunction
EndState

State A115
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array115 = stringArrays.CreateArray(akSize)
        Endif
        return Array115
    EndFunction
EndState

State A116
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array116 = stringArrays.CreateArray(akSize)
        Endif
        return Array116
    EndFunction
EndState

State A117
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array117 = stringArrays.CreateArray(akSize)
        Endif
        return Array117
    EndFunction
EndState

State A118
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array118 = stringArrays.CreateArray(akSize)
        Endif
        return Array118
    EndFunction
EndState

State A119
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array119 = stringArrays.CreateArray(akSize)
        Endif
        return Array119
    EndFunction
EndState

State A120
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array120 = stringArrays.CreateArray(akSize)
        Endif
        return Array120
    EndFunction
EndState

State A121
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array121 = stringArrays.CreateArray(akSize)
        Endif
        return Array121
    EndFunction
EndState

State A122
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array122 = stringArrays.CreateArray(akSize)
        Endif
        return Array122
    EndFunction
EndState

State A123
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array123 = stringArrays.CreateArray(akSize)
        Endif
        return Array123
    EndFunction
EndState

State A124
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array124 = stringArrays.CreateArray(akSize)
        Endif
        return Array124
    EndFunction
EndState

State A125
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array125 = stringArrays.CreateArray(akSize)
        Endif
        return Array125
    EndFunction
EndState

State A126
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array126 = stringArrays.CreateArray(akSize)
        Endif
        return Array126
    EndFunction
EndState

State A127
    String[] Function GetArray(int akSize = 0)
        if akSize > 0
            Array127 = stringArrays.CreateArray(akSize)
        Endif
        return Array127
    EndFunction
EndState

; used to write the array states in this script.
; Function WriteArrayStates(int startIndex = 0, int maxState = 99, string filePath) global
;     int i = startIndex
;     while i < maxState
;         string arrayString = ("Array" + i)
        
;         MiscUtil.WriteToFile(filePath, "\n")
;         MiscUtil.WriteToFile(filePath, "\nState A" + i)
;         MiscUtil.WriteToFile(filePath, "\n    String[] Function GetArray(int akSize = 0)")
;         MiscUtil.WriteToFile(filePath, "\n        if akSize > 0")
;         MiscUtil.WriteToFile(filePath, "\n            " + arrayString + " = stringArrays.CreateArray(akSize)")
;         MiscUtil.WriteToFile(filePath, "\n        Endif")
;         MiscUtil.WriteToFile(filePath, "\n        return " + arrayString)
;         MiscUtil.WriteToFile(filePath, "\n    EndFunction")
;         MiscUtil.WriteToFile(filePath, "\nEndState")
        
;         i += 1
;     EndWhile
; EndFunction