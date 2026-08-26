Scriptname DynamicArrays Extends Form
;This allows to create arrays of dynamic length. Like Utility.CreateStringArray(i) ect except doesn't require skse.
;attach this script and the DynamicArrays_B script to a new form in the CK that you won't use for anything else. 
;I'd recommend a new MiscObject, but can be any type of form.
;Both scripts should be attached to the same form. 
;I had to split them up because either scripts can only have a max number of states or only be a certain length.
;Putting them all on the same script didn't work.

;How to use:
;Let's say you attach these scripts to a MiscObject called DynamicArraysMisc
;Then in another script you can do: 
;DynamicArrays DArrays = (DynamicArraysMisc as form) as DynamicArrays
;String[] MyStringArray = DArrays.CreateStringArray(50)

;This script has to be attached to a form because you can't use states with global functions.
;Note you can still only create arrays up to 128 elements

String[] Function CreateStringArray(int L)
    If L < 65
        GoToState(("A" + L))
        return GetStringArray()
    Else 
        return ((Self as Form) as DynamicArrays_B).CreateStringArray(L)
    Endif
EndFunction

Bool[] Function CreateBoolArray(int L)
    If L < 65
        GoToState(("A" + L))
        return GetBoolArray()
    Else 
        return ((Self as Form) as DynamicArrays_B).CreateBoolArray(L)
    Endif
EndFunction

Int[] Function CreateIntArray(int L)
    If L < 65
        GoToState(("A" + L))
        return GetIntArray()
    Else 
        return ((Self as Form) as DynamicArrays_B).CreateIntArray(L)
    Endif
EndFunction   
    
Float[] Function CreateFloatArray(int L)
    If L < 65
        GoToState(("A" + L))
        return GetFloatArray()
    Else 
        return ((Self as Form) as DynamicArrays_B).CreateFloatArray(L)
    Endif
EndFunction
      
ObjectReference[] Function CreateObjectReferenceArray(int L)
    If L < 65
        GoToState(("A" + L))
        return GetObjectReferenceArray()
    Else 
        return ((Self as Form) as DynamicArrays_B).CreateObjectReferenceArray(L)
    Endif
EndFunction

Actor[] Function CreateActorArray(int L)
    If L < 65
        GoToState(("A" + L))
        return GetActorArray()
    Else 
        return ((Self as Form) as DynamicArrays_B).CreateActorArray(L)
    Endif
EndFunction
    
Form[] Function CreateFormArray(int L)
    If L < 65
        GoToState(("A" + L))
        return GetFormArray()
    Else 
        return ((Self as Form) as DynamicArrays_B).CreateFormArray(L)
    Endif
EndFunction


;Array Utility functions. Similar to PapyrusUtil, but doesn't require skse. 
;Note that these functions involve creating new arrays and cycling through the passed in arrays to get return arrays. 
;As such, use sparingly and don't populate arrays with the Push functions. 

;Functions include: 

;Sort ================================================================================================================
;Sort the array from smallest to largest or vice versa. 
;Note that if Direct == true (default) this affects the passed in akArray directly. 
;So no need to do MyIntArray[] = SortIntArray(MyIntArray)
;Can just do: SortIntArray(MyIntArray)
;If Direct == false, it first copy's the array, sorts the copied array and returns the copied array so the passed in akArray is unaffected.
;If Direct == false, passed in akArray must be less than or equal to 128 elements in length.

;Int[] Function SortIntArray(Int[] akArray, Bool Acending = true, Bool Direct = true)
;Float[] Function SortFloatArray(Float[] akArray, Bool Acending = true, Bool Direct = true)
;String[] Function SortStringArray(String[] akArray, Bool Ascending = true, Bool Direct = true)


;Resize ===========================================================================================================================================
;Resize akArray to NewSize and return New Array. 
;If NewSize is less than current size, removes elements after NewSize in akArray. 
;If NewSize is greater than current size, the Fill element to the end of the akArray. 

;String[] Function ResizeStringArray(String[] akArray, int NewSize, String Fill = "")
;Bool[] Function ResizeBoolArray(Bool[] akArray, int NewSize, Bool Fill = false)
;Int[] Function ResizeIntArray(Int[] akArray, int NewSize, Int Fill = 0)
;Float[] Function ResizeFloatArray(Float[] akArray, int NewSize, Float Fill = 0.0)
;Actor[] Function ResizeActorArray(Actor[] akArray, int NewSize, Actor Fill = None)
;ObjectReference[] Function ResizeObjectReferenceArray(ObjectReference[] akArray, int NewSize, ObjectReference Fill = None)
;Form[] Function ResizeFormArray(Form[] akArray, int NewSize, Form Fill = None)

;Join =======================================================================================================================
;Join a_Array with b_Array and return new array. 
;The added lengths of the arrays must be less than or equil to 128 elements.
;If greater than, the tail end of b_array is clipped off where it exceeds 128.

;String[] Function JoinStringArrays(String[] a_Array, String[] b_Array)
;Bool[] Function JoinBoolArrays(Bool[] a_Array, Bool[] b_Array)
;Int[] Function JoinIntArrays(Int[] a_Array, Int[] b_Array)
;Float[] Function JoinFloatArrays(Float[] a_Array, Float[] b_Array)
;Actor[] Function JoinActorArrays(Actor[] a_Array, Actor[] b_Array)
;ObjectReference[] Function JoinObjectReferenceArrays(ObjectReference[] a_Array, ObjectReference[] b_Array)
;Form[] Function JoinFormArrays(Form[] a_Array, Form[] b_Array)


;Push =====================================================================================================================
;Add an element to the end of the array and return new array. 
;The passed in akArray must be less than 128 elements in length.

;String[] Function PushString(String[] akArray, String ToPush)
;Bool[] Function PushBool(Bool[] akArray, Bool ToPush)
;Int[] Function PushInt(Int[] akArray, Int ToPush)
;Float[] Function PushFloat(Float[] akArray, Float ToPush)
;Actor[] Function PushActor(Actor[] akArray, Actor ToPush)
;ObjectReference[] Function PushObjectReference(ObjectReference[] akArray, ObjectReference ToPush)
;Form[] Function PushForm(Form[] akArray, Form ToPush)


;InsertAt ================================================================================================================
;Insert the ToInsert element to the akArray at Index and return new array. 
;Passed in akArray must be less than 128 elements in length.

;String[] Function InsertStringAt(String[] akArray, String ToInsert, Int Index)
;Bool[] Function InsertBoolAt(Bool[] akArray, Bool ToInsert, Int Index)
;Int[] Function InsertIntAt(Int[] akArray, Int ToInsert, Int Index)
;Float[] Function InsertFloatAt(Float[] akArray, Float ToInsert, Int Index)
;Actor[] Function InsertActorAt(Actor[] akArray, Actor ToInsert, Int Index)
;ObjectReference[] Function InsertObjectReferenceAt(ObjectReference[] akArray, ObjectReference ToInsert, Int Index)
;Form[] Function InsertFormAt(Form[] akArray, Form ToInsert, Int Index)


;InsertArrayAt =======================================================================================================
;Insert the ToInsert array into the akArray at Index and return new combined array. 
;Only adds elements up to 128.

;String[] Function InsertStringArrayAt(String[] akArray, String[] ToInsert, Int Index)
;Bool[] Function InsertBoolArrayAt(Bool[] akArray, Bool[] ToInsert, Int Index)
;Int[] Function InsertIntArrayAt(Int[] akArray, Int[] ToInsert, Int Index)
;Float[] Function InsertFloatArrayAt(Float[] akArray, Float[] ToInsert, Int Index)
;Actor[] Function InsertActorArrayAt(Actor[] akArray, Actor[] ToInsert, Int Index)
;ObjectReference[] Function InsertObjectReferenceArrayAt(ObjectReference[] akArray, ObjectReference[] ToInsert, Int Index)
;Form[] Function InsertFormArrayAt(Form[] akArray, Form[] ToInsert, Int Index)

;shift ===================================================================================================================== 
;Remove either the first or last element from the array and return new shortened array
;Passed in array must be less than or equal to 129 elements in length.

;String[] Function ShiftString(String[] akArray, Bool First = true)
;Bool[] Function ShiftBool(Bool[] akArray, Bool First = true)
;Int[] Function ShiftInt(Int[] akArray, Bool First = true)
;Float[] Function ShiftFloat(Float[] akArray, Bool First = true)
;Actor[] Function ShiftActor(Actor[] akArray, Bool First = true)
;ObjectReference[] Function ShiftObjectReference(ObjectReference[] akArray, Bool First = true)
;Form[] Function ShiftForm(Form[] akArray, Bool First = true)


;RemoveAt ==================================================================================================================
;Remove the element at the Index of the akArray and return new array. 
;Passed in array must be less than or equal to 129 elements in length.

;String[] Function RemoveStringAt(String[] akArray, Int Index)
;Bool[] Function RemoveBoolAt(Bool[] akArray, Int Index)
;Int[] Function RemoveIntAt(Int[] akArray, Int Index)
;Float[] Function RemoveFloatAt(Float[] akArray, Int Index)
;Actor[] Function RemoveActorAt(Actor[] akArray, Int Index)
;ObjectReference[] Function RemoveObjectReferenceAt(ObjectReference[] akArray, Int Index)
;Form[] Function RemoveFormAt(Form[] akArray, Int Index)


;Remove ==================================================================================================================
;Find the ToRemove element in the akArray and remove it, returning the shortened array. 
;If First == true (default) finds first instance of ToRemove, otherwise finds last instance of ToRemove (rFind) 

;String[] Function RemoveString(String[] akArray, String ToRemove, Bool First = true)
;Bool[] Function RemoveBool(Bool[] akArray, Bool ToRemove, Bool First = true)
;Int[] Function RemoveInt(Int[] akArray, Int ToRemove, Bool First = true)
;Float[] Function RemoveFloat(Float[] akArray, Float ToRemove, Bool First = true)
;Actor[] Function RemoveActor(Actor[] akArray, Actor ToRemove, Bool First = true)
;ObjectReference[] Function RemoveObjectReference(ObjectReference[] akArray, ObjectReference ToRemove, Bool First = true)
;Form[] Function RemoveForm(Form[] akArray, Form ToRemove, Bool First = true)


;Sub arrays =================================================================================================
;Put the elements between StartIndex and EndIndex of akArray into a new array and return said array.

;String[] Function SubStringArray(String[] akArray, Int StartIndex, Int EndIndex)
;Bool[] Function SubBoolArray(Bool[] akArray, Int StartIndex, Int EndIndex)
;Int[] Function SubIntArray(Int[] akArray, Int StartIndex, Int EndIndex)
;Float[] Function SubFloatArray(Float[] akArray, Int StartIndex, Int EndIndex)
;Actor[] Function SubActorArray(Actor[] akArray, Int StartIndex, Int EndIndex)
;ObjectReference[] Function SubObjectReferenceArray(ObjectReference[] akArray, Int StartIndex, Int EndIndex)
;Form[] Function SubFormArray(Form[] akArray, Int StartIndex, Int EndIndex)


;Clear =======================================================================================================
;Remove all of the ToClear elements from the akArray and return new array. 
;The length of the new array must be 128 or less, otherwise returns the akArray unedited.

;String[] Function ClearStrings(String[] akArray, String ToClear)
;Bool[] Function ClearBools(Bool[] akArray, Bool ToClear)
;Int[] Function ClearInts(Int[] akArray, Int ToClear)
;Float[] Function ClearFloats(Float[] akArray, Float ToClear)
;Actor[] Function ClearActors(Actor[] akArray, Actor ToClear)
;ObjectReference[] Function ClearObjectReferences(ObjectReference[] akArray, ObjectReference ToClear)
;Form[] Function ClearForms(Form[] akArray, Form ToClear)


;Copy =================================================================================================
;Copy all the elements from akArray to NewArray and return NewArray. 
;Only copy's up to 128 elements. 
;different than doing ArrayA = ArrayB. 
;When doing that, altering ArrayB will also alter ArrayA. Not so with these copy functions.

;String[] Function CopyStringArray(String[] akArray)
;Bool[] Function CopyBoolArray(Bool[] akArray)
;Int[] Function CopyIntArray(Int[] akArray)
;Float[] Function CopyFloatArray(Float[] akArray)
;Actor[] Function CopyActorArray(Actor[] akArray)
;ObjectReference[] Function CopyObjectReferenceArray(ObjectReference[] akArray)
;Form[] Function CopyFormArray(Form[] akArray)


;count =================================================================================
;count how many of the ToCount elements are in the array.

;Int Function CountStrings(String[] akArray, String ToCount)
;Int Function CountBools(Bool[] akArray, Bool ToCount)
;Int Function CountInts(Int[] akArray, Int ToCount)
;Int Function CountFloats(Float[] akArray, Float ToCount)
;Int Function CountActors(Actor[] akArray, Actor ToCount)
;Int Function CountObjectReferences(ObjectReference[] akArray, ObjectReference ToCount)
;Int Function CountForms(Form[] akArray, Form ToCount)

;Sort========================================================================================
String[] Function SortStringArray(String[] akArray, Bool Ascending = true, Bool Direct = true)
    int L = akArray.Length
    String[] NewArray
    
    If Direct == False
        If L > 128
            return akArray
        Else
            NewArray = CopyStringArray(akArray)
        Endif 
    Else 
        NewArray = akArray 
    Endif
    
    int i = 0 
     
    If Ascending
        While i < L 
            int ii = i + 1 
            While ii < L 
                if NewArray[ii] < NewArray[i] 
                    DbMiscFunctions.SwapStrings(NewArray, ii, i)
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
                    DbMiscFunctions.SwapStrings(NewArray, ii, i)
                Endif 
                ii += 1
            EndWhile
            i += 1
        EndWhile 
    EndIf
    
    return NewArray
EndFunction

Int[] Function SortIntArray(Int[] akArray, Bool Ascending = true, Bool Direct = true)
    int L = akArray.Length
    Int[] NewArray
    
    If Direct == False
        If L > 128
            return akArray
        Else
            NewArray = CopyIntArray(akArray)
        Endif 
    Else 
        NewArray = akArray 
    Endif
    
    int i = 0 
     
    If Ascending
        While i < L 
            int ii = i + 1 
            While ii < L 
                if NewArray[ii] < NewArray[i] 
                    DbMiscFunctions.SwapInts(NewArray, ii, i)
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
                    DbMiscFunctions.SwapInts(NewArray, ii, i)
                Endif 
                ii += 1
            EndWhile
            i += 1
        EndWhile 
    EndIf
    
    return NewArray
EndFunction

Float[] Function SortFloatArray(Float[] akArray, Bool Ascending = true,  Bool Direct = true)
    int L = akArray.Length
    Float[] NewArray   
    
    If Direct == False
        If L > 128
            return akArray
        Else
            NewArray = CopyFloatArray(akArray)
        Endif 
    Else 
        NewArray = akArray 
    Endif
    
    int i = 0
    
    If Ascending
        While i < L 
            int ii = i + 1 
            While ii < L 
                if NewArray[ii] < NewArray[i] 
                    DbMiscFunctions.SwapFloats(NewArray, ii, i)
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
                    DbMiscFunctions.SwapFloats(NewArray, ii, i)
                Endif 
                ii += 1
            EndWhile
            i += 1
        EndWhile 
    EndIf
    
    return NewArray
EndFunction

;Resize =========================================================================== 
String[] Function ResizeStringArray(String[] akArray, int NewSize, String Fill = "")
    Int L = akArray.Length 
    
    If NewSize == L || NewSize < 0 || NewSize > 128 
        Return akArray 
    Endif 
    
    String[] NewArray = CreateStringArray(NewSize) 
    Int i = 0 
    While i < L 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    If Fill != "" 
        While i < NewSize 
            NewArray[i] = Fill
            i += 1 
        EndWhile 
    Endif 
    
    return NewArray
EndFunction

Bool[] Function ResizeBoolArray(Bool[] akArray, int NewSize, Bool Fill = false)
    Int L = akArray.Length 
    
    If NewSize == L || NewSize < 0 || NewSize > 128 
        Return akArray 
    Endif 
    
    Bool[] NewArray = CreateBoolArray(NewSize) 
    Int i = 0 
    While i < L 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    If Fill != false 
        While i < NewSize 
            NewArray[i] = Fill
            i += 1 
        EndWhile 
    Endif 
    
    return NewArray
EndFunction

Int[] Function ResizeIntArray(Int[] akArray, int NewSize, Int Fill = 0)
    Int L = akArray.Length 
    
    If NewSize == L || NewSize < 0 || NewSize > 128 
        Return akArray 
    Endif 
    
    Int[] NewArray = CreateIntArray(NewSize) 
    Int i = 0 
    While i < L 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    If Fill != 0
        While i < NewSize 
            NewArray[i] = Fill
            i += 1 
        EndWhile 
    Endif 
    
    return NewArray
EndFunction

Float[] Function ResizeFloatArray(Float[] akArray, int NewSize, Float Fill = 0.0)
    Int L = akArray.Length 
    
    If NewSize == L || NewSize < 0 || NewSize > 128 
        Return akArray 
    Endif 
    
    Float[] NewArray = CreateFloatArray(NewSize) 
    Int i = 0 
    While i < L 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    If Fill != 0.0 
        While i < NewSize 
            NewArray[i] = Fill
            i += 1 
        EndWhile 
    Endif 
    
    return NewArray
EndFunction

Actor[] Function ResizeActorArray(Actor[] akArray, int NewSize, Actor Fill = None)
    Int L = akArray.Length 
    
    If NewSize == L || NewSize < 0 || NewSize > 128 
        Return akArray 
    Endif 
    
    Actor[] NewArray = CreateActorArray(NewSize) 
    Int i = 0 
    While i < L 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    If Fill != None 
        While i < NewSize 
            NewArray[i] = Fill
            i += 1 
        EndWhile 
    Endif 
    
    return NewArray
EndFunction

ObjectReference[] Function ResizeObjectReferenceArray(ObjectReference[] akArray, int NewSize, ObjectReference Fill = None)
    Int L = akArray.Length 
    
    If NewSize == L || NewSize < 0 || NewSize > 128 
        Return akArray 
    Endif 
    
    ObjectReference[] NewArray = CreateObjectReferenceArray(NewSize) 
    Int i = 0 
    While i < L 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    If Fill != None 
        While i < NewSize 
            NewArray[i] = Fill
            i += 1 
        EndWhile 
    Endif 
    
    return NewArray
EndFunction

Form[] Function ResizeFormArray(Form[] akArray, int NewSize, Form Fill = None)
    Int L = akArray.Length 
    
    If NewSize == L || NewSize < 0 || NewSize > 128 
        Return akArray 
    Endif 
    
    Form[] NewArray = CreateFormArray(NewSize) 
    Int i = 0 
    While i < L 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    If Fill != None 
        While i < NewSize 
            NewArray[i] = Fill
            i += 1 
        EndWhile 
    Endif 
    
    return NewArray
EndFunction

;Join =============================================================================
String[] Function JoinStringArrays(String[] a_Array, String[] b_Array)
    Int aL = a_Array.Length 
    Int bL = b_Array.Length 
    Int i = 0 
    Int aIndex = 0 
    
    Int NewL = aL + bL 
    If NewL > 128 
        NewL = 128 
    Endif
    
    String[] NewArray = CreateStringArray(NewL)
    
    While i < NewL && aIndex < aL 
        NewArray[i] = a_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    aIndex = 0
    
    While i < NewL && aIndex < bL 
        NewArray[i] = b_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    return NewArray
EndFunction

Bool[] Function JoinBoolArrays(Bool[] a_Array, Bool[] b_Array)
    Int aL = a_Array.Length 
    Int bL = b_Array.Length 
    Int i = 0 
    Int aIndex = 0 
    
    Int NewL = aL + bL 
    If NewL > 128 
        NewL = 128 
    Endif
    
    Bool[] NewArray = CreateBoolArray(NewL)
    
    While i < NewL && aIndex < aL 
        NewArray[i] = a_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    aIndex = 0
    
    While i < NewL && aIndex < bL 
        NewArray[i] = b_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    return NewArray
EndFunction

Int[] Function JoinIntArrays(Int[] a_Array, Int[] b_Array)
    Int aL = a_Array.Length 
    Int bL = b_Array.Length 
    Int i = 0 
    Int aIndex = 0 
    
    Int NewL = aL + bL 
    If NewL > 128 
        NewL = 128 
    Endif
    
    Int[] NewArray = CreateIntArray(NewL)
    
    While i < NewL && aIndex < aL 
        NewArray[i] = a_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    aIndex = 0
    
    While i < NewL && aIndex < bL 
        NewArray[i] = b_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    return NewArray
EndFunction

Float[] Function JoinFloatArrays(Float[] a_Array, Float[] b_Array)
    Int aL = a_Array.Length 
    Int bL = b_Array.Length 
    Int i = 0 
    Int aIndex = 0 
    
    Int NewL = aL + bL 
    If NewL > 128 
        NewL = 128 
    Endif
    
    Float[] NewArray = CreateFloatArray(NewL)
    
    While i < NewL && aIndex < aL 
        NewArray[i] = a_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    aIndex = 0
    
    While i < NewL && aIndex < bL 
        NewArray[i] = b_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    return NewArray
EndFunction

Actor[] Function JoinActorArrays(Actor[] a_Array, Actor[] b_Array)
    Int aL = a_Array.Length 
    Int bL = b_Array.Length 
    Int i = 0 
    Int aIndex = 0 
    
    Int NewL = aL + bL 
    If NewL > 128 
        NewL = 128 
    Endif
    
    Actor[] NewArray = CreateActorArray(NewL)
    
    While i < NewL && aIndex < aL 
        NewArray[i] = a_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    aIndex = 0
    
    While i < NewL && aIndex < bL 
        NewArray[i] = b_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    return NewArray
EndFunction

ObjectReference[] Function JoinObjectReferenceArrays(ObjectReference[] a_Array, ObjectReference[] b_Array)
    Int aL = a_Array.Length 
    Int bL = b_Array.Length 
    Int i = 0 
    Int aIndex = 0 
    
    Int NewL = aL + bL 
    If NewL > 128 
        NewL = 128 
    Endif
    
    ObjectReference[] NewArray = CreateObjectReferenceArray(NewL)
    
    While i < NewL && aIndex < aL 
        NewArray[i] = a_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    aIndex = 0
    
    While i < NewL && aIndex < bL 
        NewArray[i] = b_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    return NewArray
EndFunction

Form[] Function JoinFormArrays(Form[] a_Array, Form[] b_Array)
    Int aL = a_Array.Length 
    Int bL = b_Array.Length 
    Int i = 0 
    Int aIndex = 0 
    
    Int NewL = aL + bL 
    If NewL > 128 
        NewL = 128 
    Endif
    
    Form[] NewArray = CreateFormArray(NewL)
    
    While i < NewL && aIndex < aL 
        NewArray[i] = a_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    aIndex = 0
    
    While i < NewL && aIndex < bL 
        NewArray[i] = b_Array[aIndex] 
        i += 1
        aIndex += 1 
    EndWhile 
    
    return NewArray
EndFunction

;Push=============================================================
String[] Function PushString(String[] akArray, String ToPush)
    Int L = akArray.length 
    If L > 127
        return akArray 
    Endif 
    
    String[] NewArray = CreateStringArray(L + 1) 
    NewArray[L] = ToPush 
    
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

Bool[] Function PushBool(Bool[] akArray, Bool ToPush)
    Int L = akArray.length 
    If L > 127
        return akArray 
    Endif 
    
    Bool[] NewArray = CreateBoolArray(L + 1) 
    NewArray[L] = ToPush 
    
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

Int[] Function PushInt(Int[] akArray, Int ToPush)
    Int L = akArray.length 
    If L > 127
        return akArray 
    Endif 
    
    Int[] NewArray = CreateIntArray(L + 1) 
    NewArray[L] = ToPush 
    
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

Float[] Function PushFloat(Float[] akArray, Float ToPush)
    Int L = akArray.length 
    If L > 127
        return akArray 
    Endif 
    
    Float[] NewArray = CreateFloatArray(L + 1) 
    NewArray[L] = ToPush 
    
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

Actor[] Function PushActor(Actor[] akArray, Actor ToPush)
    Int L = akArray.length 
    If L > 127
        return akArray 
    Endif 
    
    Actor[] NewArray = CreateActorArray(L + 1) 
    NewArray[L] = ToPush 
    
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

ObjectReference[] Function PushObjectReference(ObjectReference[] akArray, ObjectReference ToPush)
    Int L = akArray.length 
    If L > 127
        return akArray 
    Endif 
    
    ObjectReference[] NewArray = CreateObjectReferenceArray(L + 1) 
    NewArray[L] = ToPush 
    
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

Form[] Function PushForm(Form[] akArray, Form ToPush)
    Int L = akArray.length 
    If L > 127
        return akArray 
    Endif 
    
    Form[] NewArray = CreateFormArray(L + 1) 
    NewArray[L] = ToPush 
    
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction


;InsertAt====================================================================
String[] Function InsertStringAt(String[] akArray, String ToInsert, Int Index)
    Int L = akArray.length
    If L > 127
        return akArray 
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, L)
    
    Int NewL = L + 1
    
    String[] NewArray = CreateStringArray(NewL) 
    NewArray[Index] = ToInsert 
    
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    NewL -= 1
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

Bool[] Function InsertBoolAt(Bool[] akArray, Bool ToInsert, Int Index)
    Int L = akArray.length
    If L > 127
        return akArray 
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, L)
    
    Int NewL = L + 1
    
    Bool[] NewArray = CreateBoolArray(NewL) 
    NewArray[Index] = ToInsert 
    
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    NewL -= 1
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

Int[] Function InsertIntAt(Int[] akArray, Int ToInsert, Int Index)
    Int L = akArray.length
    If L > 127
        return akArray 
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, L)
    
    Int NewL = L + 1
    
    Int[] NewArray = CreateIntArray(NewL) 
    NewArray[Index] = ToInsert 
    
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    NewL -= 1
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

Float[] Function InsertFloatAt(Float[] akArray, Float ToInsert, Int Index)
    Int L = akArray.length
    If L > 127
        return akArray 
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, L)
    
    Int NewL = L + 1
    
    Float[] NewArray = CreateFloatArray(NewL) 
    NewArray[Index] = ToInsert 
    
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    NewL -= 1
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

Actor[] Function InsertActorAt(Actor[] akArray, Actor ToInsert, Int Index)
    Int L = akArray.length
    If L > 127
        return akArray 
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, L)
    
    Int NewL = L + 1
    
    Actor[] NewArray = CreateActorArray(NewL) 
    NewArray[Index] = ToInsert 
    
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    NewL -= 1
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

ObjectReference[] Function InsertObjectReferenceAt(ObjectReference[] akArray, ObjectReference ToInsert, Int Index)
    Int L = akArray.length
    If L > 127
        return akArray 
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, L)
    
    Int NewL = L + 1
    
    ObjectReference[] NewArray = CreateObjectReferenceArray(NewL) 
    NewArray[Index] = ToInsert 
    
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    NewL -= 1
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

Form[] Function InsertFormAt(Form[] akArray, Form ToInsert, Int Index)
    Int L = akArray.length
    If L > 127
        return akArray 
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, L)
    
    Int NewL = L + 1
    
    Form[] NewArray = CreateFormArray(NewL) 
    NewArray[Index] = ToInsert 
    
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    NewL -= 1
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L] 
    EndWhile
    
    Return NewArray
EndFunction

;InsertArrayAt====================================================================
String[] Function InsertStringArrayAt(String[] akArray, String[] ToInsert, Int Index)
    Int LA = akArray.length
    Int LB = ToInsert.Length 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, LA)
    
    If Index <= 0 
        return JoinStringArrays(ToInsert, akArray)
    Elseif Index >= LA 
        return JoinStringArrays(akArray, ToInsert)
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, 128)
    
    If Index >= 128 
        Return akArray
    Endif
    
    int i = 0 
    Int NewL = DbMiscFunctions.ClampInt((LA + LB), 1, 128)
    
    String[] NewArray = CreateStringArray(NewL)
    
    While i < Index && i < NewL 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    int ii = i
    int iB = 0
    
    While iB < LB && i < NewL 
        NewArray[i] = ToInsert[iB] 
        i += 1
        iB += 1
    EndWhile 
    
    While ii < LA && i < NewL 
        NewArray[i] = akArray[ii] 
        i += 1
        ii += 1 
    EndWhile 
    
    Return NewArray
EndFunction

Bool[] Function InsertBoolArrayAt(Bool[] akArray, Bool[] ToInsert, Int Index)
    Int LA = akArray.length
    Int LB = ToInsert.Length 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, LA)
    
    If Index <= 0 
        return JoinBoolArrays(ToInsert, akArray)
    Elseif Index >= LA 
        return JoinBoolArrays(akArray, ToInsert)
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, 128)
    
    If Index >= 128 
        Return akArray
    Endif
    
    int i = 0 
    Int NewL = DbMiscFunctions.ClampInt((LA + LB), 1, 128)
    
    Bool[] NewArray = CreateBoolArray(NewL)
    
    While i < Index && i < NewL 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    int ii = i
    int iB = 0
    
    While iB < LB && i < NewL 
        NewArray[i] = ToInsert[iB] 
        i += 1
        iB += 1
    EndWhile 
    
    While ii < LA && i < NewL 
        NewArray[i] = akArray[ii] 
        i += 1
        ii += 1 
    EndWhile 
    
    Return NewArray
EndFunction

Int[] Function InsertIntArrayAt(Int[] akArray, Int[] ToInsert, Int Index)
    Int LA = akArray.length
    Int LB = ToInsert.Length 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, LA)
    
    If Index <= 0 
        return JoinIntArrays(ToInsert, akArray)
    Elseif Index >= LA 
        return JoinIntArrays(akArray, ToInsert)
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, 128)
    
    If Index >= 128 
        Return akArray
    Endif
    
    int i = 0 
    Int NewL = DbMiscFunctions.ClampInt((LA + LB), 1, 128)
    
    Int[] NewArray = CreateIntArray(NewL)
    
    While i < Index && i < NewL 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    int ii = i
    int iB = 0
    
    While iB < LB && i < NewL 
        NewArray[i] = ToInsert[iB] 
        i += 1
        iB += 1
    EndWhile 
    
    While ii < LA && i < NewL 
        NewArray[i] = akArray[ii] 
        i += 1
        ii += 1 
    EndWhile 
    
    Return NewArray
EndFunction

Float[] Function InsertFloatArrayAt(Float[] akArray, Float[] ToInsert, Int Index)
    Int LA = akArray.length
    Int LB = ToInsert.Length 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, LA)
    
    If Index <= 0 
        return JoinFloatArrays(ToInsert, akArray)
    Elseif Index >= LA 
        return JoinFloatArrays(akArray, ToInsert)
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, 128)
    
    If Index >= 128 
        Return akArray
    Endif
    
    int i = 0 
    Int NewL = DbMiscFunctions.ClampInt((LA + LB), 1, 128)
    
    Float[] NewArray = CreateFloatArray(NewL)
    
    While i < Index && i < NewL 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    int ii = i
    int iB = 0
    
    While iB < LB && i < NewL 
        NewArray[i] = ToInsert[iB] 
        i += 1
        iB += 1
    EndWhile 
    
    While ii < LA && i < NewL 
        NewArray[i] = akArray[ii] 
        i += 1
        ii += 1 
    EndWhile 
    
    Return NewArray
EndFunction

Actor[] Function InsertActorArrayAt(Actor[] akArray, Actor[] ToInsert, Int Index)
    Int LA = akArray.length
    Int LB = ToInsert.Length 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, LA)
    
    If Index <= 0 
        return JoinActorArrays(ToInsert, akArray)
    Elseif Index >= LA 
        return JoinActorArrays(akArray, ToInsert)
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, 128)
    
    If Index >= 128 
        Return akArray
    Endif
    
    int i = 0 
    Int NewL = DbMiscFunctions.ClampInt((LA + LB), 1, 128)
    
    Actor[] NewArray = CreateActorArray(NewL)
    
    While i < Index && i < NewL 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    int ii = i
    int iB = 0
    
    While iB < LB && i < NewL 
        NewArray[i] = ToInsert[iB] 
        i += 1
        iB += 1
    EndWhile 
    
    While ii < LA && i < NewL 
        NewArray[i] = akArray[ii] 
        i += 1
        ii += 1 
    EndWhile 
    
    Return NewArray
EndFunction

ObjectReference[] Function InsertObjectReferenceArrayAt(ObjectReference[] akArray, ObjectReference[] ToInsert, Int Index)
    Int LA = akArray.length
    Int LB = ToInsert.Length 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, LA)
    
    If Index <= 0 
        return JoinObjectReferenceArrays(ToInsert, akArray)
    Elseif Index >= LA 
        return JoinObjectReferenceArrays(akArray, ToInsert)
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, 128)
    
    If Index >= 128 
        Return akArray
    Endif
    
    int i = 0 
    Int NewL = DbMiscFunctions.ClampInt((LA + LB), 1, 128)
    
    ObjectReference[] NewArray = CreateObjectReferenceArray(NewL)
    
    While i < Index && i < NewL 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    int ii = i
    int iB = 0
    
    While iB < LB && i < NewL 
        NewArray[i] = ToInsert[iB] 
        i += 1
        iB += 1
    EndWhile 
    
    While ii < LA && i < NewL 
        NewArray[i] = akArray[ii] 
        i += 1
        ii += 1 
    EndWhile 
    
    Return NewArray
EndFunction

Form[] Function InsertFormArrayAt(Form[] akArray, Form[] ToInsert, Int Index)
    Int LA = akArray.length
    Int LB = ToInsert.Length 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, LA)
    
    If Index <= 0 
        return JoinFormArrays(ToInsert, akArray)
    Elseif Index >= LA 
        return JoinFormArrays(akArray, ToInsert)
    Endif 
    
    Index = DbMiscFunctions.ClampInt(Index, 0, 128)
    
    If Index >= 128 
        Return akArray
    Endif
    
    int i = 0 
    Int NewL = DbMiscFunctions.ClampInt((LA + LB), 1, 128)
    
    Form[] NewArray = CreateFormArray(NewL)
    
    While i < Index && i < NewL 
        NewArray[i] = akArray[i] 
        i += 1 
    EndWhile 
    
    int ii = i
    int iB = 0
    
    While iB < LB && i < NewL 
        NewArray[i] = ToInsert[iB] 
        i += 1
        iB += 1
    EndWhile 
    
    While ii < LA && i < NewL 
        NewArray[i] = akArray[ii] 
        i += 1
        ii += 1 
    EndWhile 
    
    Return NewArray
EndFunction

;Shift============================================================
String[] Function ShiftString(String[] akArray, Bool First = true)
    If First == true
        Return RemoveStringAt(akArray, 0)
    Else 
        return RemoveStringAt(akArray, (akArray.length - 1))
    Endif
EndFunction

Bool[] Function ShiftBool(Bool[] akArray, Bool First = true)
    If First == true
        Return RemoveBoolAt(akArray, 0)
    Else 
        return RemoveBoolAt(akArray, (akArray.length - 1))
    Endif
EndFunction

Int[] Function ShiftInt(Int[] akArray, Bool First = true)
    If First == true
        Return RemoveIntAt(akArray, 0)
    Else 
        return RemoveIntAt(akArray, (akArray.length - 1))
    Endif
EndFunction

Float[] Function ShiftFloat(Float[] akArray, Bool First = true)
    If First == true
        Return RemoveFloatAt(akArray, 0)
    Else 
        return RemoveFloatAt(akArray, (akArray.length - 1))
    Endif
EndFunction

Actor[] Function ShiftActor(Actor[] akArray, Bool First = true)
    If First == true
        Return RemoveActorAt(akArray, 0)
    Else 
        return RemoveActorAt(akArray, (akArray.length - 1))
    Endif
EndFunction

ObjectReference[] Function ShiftObjectReference(ObjectReference[] akArray, Bool First = true)
    If First == true
        Return RemoveObjectReferenceAt(akArray, 0)
    Else 
        return RemoveObjectReferenceAt(akArray, (akArray.length - 1))
    Endif
EndFunction

Form[] Function ShiftForm(Form[] akArray, Bool First = true)
    If First == true
        Return RemoveFormAt(akArray, 0)
    Else 
        return RemoveFormAt(akArray, (akArray.length - 1))
    Endif
EndFunction

;RemoveAt============================================================
String[] Function RemoveStringAt(String[] akArray, Int Index)
    Int L = akArray.length 
    If L > 129 || Index >= L || Index < 0
        return akArray
    Endif 
    
    Index += 1
    Int NewL = L - 1
    
    String[] NewArray = CreateStringArray(L - 1) 
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    L -= 1 
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    akArray = NewArray
    Return NewArray
EndFunction

Bool[] Function RemoveBoolAt(Bool[] akArray, Int Index)
    Int L = akArray.length 
    If L > 129 || Index >= L || Index < 0
        return akArray
    Endif 
    
    Index += 1
    Int NewL = L - 1
    
    Bool[] NewArray = CreateBoolArray(L - 1) 
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    L -= 1 
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    Return NewArray
EndFunction

Int[] Function RemoveIntAt(Int[] akArray, Int Index)
    Int L = akArray.length 
    If L > 129 || Index >= L || Index < 0
        return akArray
    Endif 
    
    Index += 1
    Int NewL = L - 1
    
    Int[] NewArray = CreateIntArray(L - 1) 
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    L -= 1 
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    Return NewArray
EndFunction

Float[] Function RemoveFloatAt(Float[] akArray, Int Index)
    Int L = akArray.length 
    If L > 129 || Index >= L || Index < 0
        return akArray
    Endif 
    
    Index += 1
    Int NewL = L - 1
    
    Float[] NewArray = CreateFloatArray(L - 1) 
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    L -= 1 
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    Return NewArray
EndFunction

Actor[] Function RemoveActorAt(Actor[] akArray, Int Index)
    Int L = akArray.length 
    If L > 129 || Index >= L || Index < 0
        return akArray
    Endif 
    
    Index += 1
    Int NewL = L - 1
    
    Actor[] NewArray = CreateActorArray(L - 1) 
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    L -= 1 
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    Return NewArray
EndFunction

ObjectReference[] Function RemoveObjectReferenceAt(ObjectReference[] akArray, Int Index)
    Int L = akArray.length 
    If L > 129 || Index >= L || Index < 0
        return akArray
    Endif 
    
    Index += 1
    Int NewL = L - 1
    
    ObjectReference[] NewArray = CreateObjectReferenceArray(L - 1) 
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    L -= 1 
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    Return NewArray
EndFunction

Form[] Function RemoveFormAt(Form[] akArray, Int Index)
    Int L = akArray.length 
    If L > 129 || Index >= L || Index < 0
        return akArray
    Endif 
    
    Index += 1
    Int NewL = L - 1
    
    Form[] NewArray = CreateFormArray(L - 1) 
    While L > Index
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    L -= 1 
    
    While L > 0
        L -= 1 
        NewL -= 1
        NewArray[NewL] = akArray[L]
    EndWhile
    
    Return NewArray
EndFunction

;Remove ============================================================================= 
String[] Function RemoveString(String[] akArray, String ToRemove, Bool First = true)
    Int L = akArray.Length 
    If L > 129 
        Return akArray 
    Endif 
    
    Int i 
    If First 
        i = akArray.Find(ToRemove) 
    Else 
        i = akArray.rFind(ToRemove)
    Endif 
    
    If i > -1 
        return RemoveStringAt(akArray, i) 
    Else 
        return akArray 
    Endif 
EndFunction

Bool[] Function RemoveBool(Bool[] akArray, Bool ToRemove, Bool First = true)
    Int L = akArray.Length 
    If L > 129 
        Return akArray 
    Endif 
    
    Int i 
    If First 
        i = akArray.Find(ToRemove) 
    Else 
        i = akArray.rFind(ToRemove)
    Endif 
    
    If i > -1 
        return RemoveBoolAt(akArray, i) 
    Else 
        return akArray 
    Endif 
EndFunction

Int[] Function RemoveInt(Int[] akArray, Int ToRemove, Bool First = true)
    Int L = akArray.Length 
    If L > 129 
        Return akArray 
    Endif 
    
    Int i 
    If First 
        i = akArray.Find(ToRemove) 
    Else 
        i = akArray.rFind(ToRemove)
    Endif 
    
    If i > -1 
        return RemoveIntAt(akArray, i) 
    Else 
        return akArray 
    Endif 
EndFunction

Float[] Function RemoveFloat(Float[] akArray, Float ToRemove, Bool First = true)
    Int L = akArray.Length 
    If L > 129 
        Return akArray 
    Endif 
    
    Int i 
    If First 
        i = akArray.Find(ToRemove) 
    Else 
        i = akArray.rFind(ToRemove)
    Endif 
    
    If i > -1 
        return RemoveFloatAt(akArray, i) 
    Else 
        return akArray 
    Endif 
EndFunction

Actor[] Function RemoveActor(Actor[] akArray, Actor ToRemove, Bool First = true)
    Int L = akArray.Length 
    If L > 129 
        Return akArray 
    Endif 
    
    Int i 
    If First 
        i = akArray.Find(ToRemove) 
    Else 
        i = akArray.rFind(ToRemove)
    Endif 
    
    If i > -1 
        return RemoveActorAt(akArray, i) 
    Else 
        return akArray 
    Endif 
EndFunction

ObjectReference[] Function RemoveObjectReference(ObjectReference[] akArray, ObjectReference ToRemove, Bool First = true)
    Int L = akArray.Length 
    If L > 129 
        Return akArray 
    Endif 
    
    Int i 
    If First 
        i = akArray.Find(ToRemove) 
    Else 
        i = akArray.rFind(ToRemove)
    Endif 
    
    If i > -1 
        return RemoveObjectReferenceAt(akArray, i) 
    Else 
        return akArray 
    Endif 
EndFunction

Form[] Function RemoveForm(Form[] akArray, Form ToRemove, Bool First = true)
    Int L = akArray.Length 
    If L > 129 
        Return akArray 
    Endif 
    
    Int i 
    If First 
        i = akArray.Find(ToRemove) 
    Else 
        i = akArray.rFind(ToRemove)
    Endif 
    
    If i > -1 
        return RemoveFormAt(akArray, i) 
    Else 
        return akArray 
    Endif 
EndFunction

;Sub arrays ============================================================================= 
String[] Function SubStringArray(String[] akArray, Int StartIndex, Int EndIndex)
    Int L = akArray.Length - 1
    EndIndex = DbMiscFunctions.ClampInt(EndIndex, 0, L) 
    StartIndex = DbMiscFunctions.ClampInt(StartIndex, 0, EndIndex)
    
    If (StartIndex == 0 && EndIndex == L) || (StartIndex == EndIndex)
        return akArray 
    Endif 
    
    Int NewL = EndIndex - StartIndex + 1
    String[] NewArray = CreateStringArray(NewL) 
    
    While NewL > 0 && EndIndex >= StartIndex 
        NewL -= 1 
        NewArray[NewL] = akArray[EndIndex] 
        EndIndex -= 1 
    EndWhile 
    
    Return NewArray
EndFunction

Bool[] Function SubBoolArray(Bool[] akArray, Int StartIndex, Int EndIndex)
    Int L = akArray.Length - 1
    EndIndex = DbMiscFunctions.ClampInt(EndIndex, 0, L) 
    StartIndex = DbMiscFunctions.ClampInt(StartIndex, 0, EndIndex)
    
    If (StartIndex == 0 && EndIndex == L) || (StartIndex == EndIndex)
        return akArray 
    Endif 
    
    Int NewL = EndIndex - StartIndex + 1
    Bool[] NewArray = CreateBoolArray(NewL) 
    
    While NewL > 0 && EndIndex >= StartIndex 
        NewL -= 1 
        NewArray[NewL] = akArray[EndIndex] 
        EndIndex -= 1 
    EndWhile 
    
    Return NewArray
EndFunction

Int[] Function SubIntArray(Int[] akArray, Int StartIndex, Int EndIndex)
    Int L = akArray.Length - 1
    EndIndex = DbMiscFunctions.ClampInt(EndIndex, 0, L) 
    StartIndex = DbMiscFunctions.ClampInt(StartIndex, 0, EndIndex)
    
    If (StartIndex == 0 && EndIndex == L) || (StartIndex == EndIndex)
        return akArray 
    Endif 
    
    Int NewL = EndIndex - StartIndex + 1
    Int[] NewArray = CreateIntArray(NewL) 
    
    While NewL > 0 && EndIndex >= StartIndex 
        NewL -= 1 
        NewArray[NewL] = akArray[EndIndex] 
        EndIndex -= 1 
    EndWhile 
    
    Return NewArray
EndFunction

Float[] Function SubFloatArray(Float[] akArray, Int StartIndex, Int EndIndex)
    Int L = akArray.Length - 1
    EndIndex = DbMiscFunctions.ClampInt(EndIndex, 0, L) 
    StartIndex = DbMiscFunctions.ClampInt(StartIndex, 0, EndIndex)
    
    If (StartIndex == 0 && EndIndex == L) || (StartIndex == EndIndex)
        return akArray 
    Endif 
    
    Int NewL = EndIndex - StartIndex + 1
    Float[] NewArray = CreateFloatArray(NewL) 
    
    While NewL > 0 && EndIndex >= StartIndex 
        NewL -= 1 
        NewArray[NewL] = akArray[EndIndex] 
        EndIndex -= 1 
    EndWhile 
    
    Return NewArray
EndFunction

Actor[] Function SubActorArray(Actor[] akArray, Int StartIndex, Int EndIndex)
    Int L = akArray.Length - 1
    EndIndex = DbMiscFunctions.ClampInt(EndIndex, 0, L) 
    StartIndex = DbMiscFunctions.ClampInt(StartIndex, 0, EndIndex)
    
    If (StartIndex == 0 && EndIndex == L) || (StartIndex == EndIndex)
        return akArray 
    Endif 
    
    Int NewL = EndIndex - StartIndex + 1
    Actor[] NewArray = CreateActorArray(NewL) 
    
    While NewL > 0 && EndIndex >= StartIndex 
        NewL -= 1 
        NewArray[NewL] = akArray[EndIndex] 
        EndIndex -= 1 
    EndWhile 
    
    Return NewArray
EndFunction

ObjectReference[] Function SubObjectReferenceArray(ObjectReference[] akArray, Int StartIndex, Int EndIndex)
    Int L = akArray.Length - 1
    EndIndex = DbMiscFunctions.ClampInt(EndIndex, 0, L) 
    StartIndex = DbMiscFunctions.ClampInt(StartIndex, 0, EndIndex)
    
    If (StartIndex == 0 && EndIndex == L) || (StartIndex == EndIndex)
        return akArray 
    Endif 
    
    Int NewL = EndIndex - StartIndex + 1
    ObjectReference[] NewArray = CreateObjectReferenceArray(NewL) 
    
    While NewL > 0 && EndIndex >= StartIndex 
        NewL -= 1 
        NewArray[NewL] = akArray[EndIndex] 
        EndIndex -= 1 
    EndWhile 
    
    Return NewArray
EndFunction

Form[] Function SubFormArray(Form[] akArray, Int StartIndex, Int EndIndex)
    Int L = akArray.Length - 1
    EndIndex = DbMiscFunctions.ClampInt(EndIndex, 0, L) 
    StartIndex = DbMiscFunctions.ClampInt(StartIndex, 0, EndIndex)
    
    If (StartIndex == 0 && EndIndex == L) || (StartIndex == EndIndex)
        return akArray 
    Endif 
    
    Int NewL = EndIndex - StartIndex + 1
    Form[] NewArray = CreateFormArray(NewL) 
    
    While NewL > 0 && EndIndex >= StartIndex 
        NewL -= 1 
        NewArray[NewL] = akArray[EndIndex] 
        EndIndex -= 1 
    EndWhile 
    
    Return NewArray
EndFunction

;Clear==================================================================================
String[] Function ClearStrings(String[] akArray, String ToClear)
    Int Count = CountStrings(akArray, ToClear)
    Int L = akArray.length
    
    Int NewL = L - Count
    If NewL > 128
        return akArray
    Endif 
    
    String[] NewArray = CreateStringArray(NewL) 
    
    While L > 0 
        L -= 1 
        If akArray[L] != ToClear 
            NewL -= 1
            NewArray[NewL] = akArray[L] 
        Endif 
    EndWhile
    
    Return NewArray
EndFunction

Bool[] Function ClearBools(Bool[] akArray, Bool ToClear)
    Int Count = CountBools(akArray, ToClear)
    Int L = akArray.length
    
    Int NewL = L - Count
    If NewL > 128
        return akArray
    Endif 
    
    Bool[] NewArray = CreateBoolArray(NewL) 
    
    While L > 0 
        L -= 1 
        If akArray[L] != ToClear 
            NewL -= 1
            NewArray[NewL] = akArray[L] 
        Endif 
    EndWhile
    
    Return NewArray
EndFunction

Int[] Function ClearInts(Int[] akArray, Int ToClear)
    Int Count = CountInts(akArray, ToClear)
    Int L = akArray.length
    
    Int NewL = L - Count
    If NewL > 128
        return akArray
    Endif 
    
    Int[] NewArray = CreateIntArray(NewL) 
    
    While L > 0 
        L -= 1 
        If akArray[L] != ToClear 
            NewL -= 1
            NewArray[NewL] = akArray[L] 
        Endif 
    EndWhile
    
    Return NewArray
EndFunction

Float[] Function ClearFloats(Float[] akArray, Float ToClear)
    Int Count = CountFloats(akArray, ToClear)
    Int L = akArray.length
    
    Int NewL = L - Count
    If NewL > 128
        return akArray
    Endif 
    
    Float[] NewArray = CreateFloatArray(NewL) 
    
    While L > 0 
        L -= 1 
        If akArray[L] != ToClear 
            NewL -= 1
            NewArray[NewL] = akArray[L] 
        Endif 
    EndWhile
    
    Return NewArray
EndFunction

Actor[] Function ClearActors(Actor[] akArray, Actor ToClear)
    Int Count = CountActors(akArray, ToClear)
    Int L = akArray.length
    
    Int NewL = L - Count
    If NewL > 128
        return akArray
    Endif 
    
    Actor[] NewArray = CreateActorArray(NewL) 
    
    While L > 0 
        L -= 1 
        If akArray[L] != ToClear 
            NewL -= 1
            NewArray[NewL] = akArray[L] 
        Endif 
    EndWhile
    
    Return NewArray
EndFunction

ObjectReference[] Function ClearObjectReferences(ObjectReference[] akArray, ObjectReference ToClear)
    Int Count = CountObjectReferences(akArray, ToClear)
    Int L = akArray.length
    
    Int NewL = L - Count
    If NewL > 128
        return akArray
    Endif 
    
    ObjectReference[] NewArray = CreateObjectReferenceArray(NewL) 
    
    While L > 0 
        L -= 1 
        If akArray[L] != ToClear 
            NewL -= 1
            NewArray[NewL] = akArray[L] 
        Endif 
    EndWhile
    
    Return NewArray
EndFunction

Form[] Function ClearForms(Form[] akArray, Form ToClear)
    Int Count = CountForms(akArray, ToClear)
    Int L = akArray.length
    
    Int NewL = L - Count
    If NewL > 128
        return akArray
    Endif 
    
    Form[] NewArray = CreateFormArray(NewL) 
    
    While L > 0 
        L -= 1 
        If akArray[L] != ToClear 
            NewL -= 1
            NewArray[NewL] = akArray[L] 
        Endif 
    EndWhile
    
    Return NewArray
EndFunction

;Copy========================================================== 
String[] Function CopyStringArray(String[] akArray)
    Int L = DbMiscFunctions.ClampInt(akArray.Length, 0, 128)
    String[] NewArray = CreateStringArray(L) 
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L]
    EndWhile 
    
    Return NewArray
EndFunction

Bool[] Function CopyBoolArray(Bool[] akArray)
    Int L = DbMiscFunctions.ClampInt(akArray.Length, 0, 128)
    Bool[] NewArray = CreateBoolArray(L) 
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L]
    EndWhile 
    
    Return NewArray
EndFunction

Int[] Function CopyIntArray(Int[] akArray)
    Int L = DbMiscFunctions.ClampInt(akArray.Length, 0, 128)
    Int[] NewArray = CreateIntArray(L) 
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L]
    EndWhile 
    
    Return NewArray
EndFunction

Float[] Function CopyFloatArray(Float[] akArray)
    Int L = DbMiscFunctions.ClampInt(akArray.Length, 0, 128)
    Float[] NewArray = CreateFloatArray(L) 
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L]
    EndWhile 
    
    Return NewArray
EndFunction

Actor[] Function CopyActorArray(Actor[] akArray)
    Int L = DbMiscFunctions.ClampInt(akArray.Length, 0, 128)
    Actor[] NewArray = CreateActorArray(L) 
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L]
    EndWhile 
    
    Return NewArray
EndFunction

ObjectReference[] Function CopyObjectReferenceArray(ObjectReference[] akArray)
    Int L = DbMiscFunctions.ClampInt(akArray.Length, 0, 128)
    ObjectReference[] NewArray = CreateObjectReferenceArray(L) 
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L]
    EndWhile 
    
    Return NewArray
EndFunction

Form[] Function CopyFormArray(Form[] akArray)
    Int L = DbMiscFunctions.ClampInt(akArray.Length, 0, 128)
    Form[] NewArray = CreateFormArray(L) 
    While L > 0 
        L -= 1 
        NewArray[L] = akArray[L]
    EndWhile 
    
    Return NewArray
EndFunction

;count=========================================================
;count how many of the ToCount elements are in the array.
Int Function CountStrings(String[] akArray, String ToCount)
    Int Count = 0 
    Int I = akArray.Find(ToCount) 
    While I > -1 
        Count += 1 
        I += 1 
        I = akArray.find(ToCount, I)
    EndWhile
    
    Return Count
EndFunction

Int Function CountBools(Bool[] akArray, Bool ToCount)
    Int Count = 0 
    Int I = akArray.Find(ToCount) 
    While I > -1 
        Count += 1 
        I += 1 
        I = akArray.find(ToCount, I)
    EndWhile
    
    Return Count
EndFunction

Int Function CountInts(Int[] akArray, Int ToCount)
    Int Count = 0 
    Int I = akArray.Find(ToCount) 
    While I > -1 
        Count += 1 
        I += 1 
        I = akArray.find(ToCount, I)
    EndWhile
    
    Return Count
EndFunction

Int Function CountFloats(Float[] akArray, Float ToCount)
    Int Count = 0 
    Int I = akArray.Find(ToCount) 
    While I > -1 
        Count += 1 
        I += 1 
        I = akArray.find(ToCount, I)
    EndWhile
    
    Return Count
EndFunction

Int Function CountActors(Actor[] akArray, Actor ToCount)
    Int Count = 0 
    Int I = akArray.Find(ToCount) 
    While I > -1 
        Count += 1 
        I += 1 
        I = akArray.find(ToCount, I)
    EndWhile
    
    Return Count
EndFunction

Int Function CountObjectReferences(ObjectReference[] akArray, ObjectReference ToCount)
    Int Count = 0 
    Int I = akArray.Find(ToCount) 
    While I > -1 
        Count += 1 
        I += 1 
        I = akArray.find(ToCount, I)
    EndWhile
    
    Return Count
EndFunction

Int Function CountForms(Form[] akArray, Form ToCount)
    Int Count = 0 
    Int I = akArray.Find(ToCount) 
    While I > -1 
        Count += 1 
        I += 1 
        I = akArray.find(ToCount, I)
    EndWhile
    
    Return Count
EndFunction

;For the create array functions.
;functions must also be defined in the empty state.
String[] Function GetStringArray() 
EndFunction

Bool[] Function GetBoolArray() 
EndFunction

Int[] Function GetIntArray() 
EndFunction

Float[] Function GetFloatArray() 
EndFunction

Actor[] Function GetActorArray() 
EndFunction

ObjectReference[] Function GetObjectReferenceArray() 
EndFunction

Form[] Function GetFormArray() 
EndFunction
    
State A1
    String[] Function GetStringArray()
        String[] A = New String[1]
        Return A
    EndFunction
    
    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[1]
        Return A
    EndFunction
    
    Int[] Function GetIntArray()
        Int[] A = New Int[1]
        Return A
    EndFunction
    
    Float[] Function GetFloatArray()
        Float[] A = New Float[1]
        Return A
    EndFunction
    
    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[1]
        Return A
    EndFunction
    
    Actor[] Function GetActorArray()
        Actor[] A = New Actor[1]
        Return A
    EndFunction
    
    Form[] Function GetFormArray()
        Form[] A = New Form[1]
        Return A
    EndFunction
EndState

State A2
    String[] Function GetStringArray()
        String[] A = New String[2]
        Return A
    EndFunction
    
    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[2]
        Return A
    EndFunction
    
    Int[] Function GetIntArray()
        Int[] A = New Int[2]
        Return A
    EndFunction
    
    Float[] Function GetFloatArray()
        Float[] A = New Float[2]
        Return A
    EndFunction
    
    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[2]
        Return A
    EndFunction
    
    Actor[] Function GetActorArray()
        Actor[] A = New Actor[2]
        Return A
    EndFunction
    
    Form[] Function GetFormArray()
        Form[] A = New Form[2]
        Return A
    EndFunction
EndState

State A3
    String[] Function GetStringArray()
        String[] A = New String[3]
        Return A
    EndFunction
    
    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[3]
        Return A
    EndFunction
    
    Int[] Function GetIntArray()
        Int[] A = New Int[3]
        Return A
    EndFunction
    
    Float[] Function GetFloatArray()
        Float[] A = New Float[3]
        Return A
    EndFunction
    
    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[3]
        Return A
    EndFunction
    
    Actor[] Function GetActorArray()
        Actor[] A = New Actor[3]
        Return A
    EndFunction
    
    Form[] Function GetFormArray()
        Form[] A = New Form[3]
        Return A
    EndFunction
EndState

State A4
    String[] Function GetStringArray()
        String[] A = New String[4]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[4]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[4]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[4]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[4]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[4]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[4]
        Return A
    EndFunction
EndState

State A5
    String[] Function GetStringArray()
        String[] A = New String[5]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[5]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[5]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[5]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[5]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[5]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[5]
        Return A
    EndFunction
EndState

State A6
    String[] Function GetStringArray()
        String[] A = New String[6]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[6]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[6]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[6]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[6]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[6]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[6]
        Return A
    EndFunction
EndState

State A7
    String[] Function GetStringArray()
        String[] A = New String[7]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[7]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[7]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[7]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[7]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[7]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[7]
        Return A
    EndFunction
EndState

State A8
    String[] Function GetStringArray()
        String[] A = New String[8]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[8]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[8]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[8]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[8]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[8]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[8]
        Return A
    EndFunction
EndState

State A9
    String[] Function GetStringArray()
        String[] A = New String[9]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[9]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[9]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[9]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[9]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[9]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[9]
        Return A
    EndFunction
EndState

State A10
    String[] Function GetStringArray()
        String[] A = New String[10]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[10]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[10]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[10]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[10]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[10]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[10]
        Return A
    EndFunction
EndState

State A11
    String[] Function GetStringArray()
        String[] A = New String[11]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[11]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[11]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[11]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[11]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[11]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[11]
        Return A
    EndFunction
EndState

State A12
    String[] Function GetStringArray()
        String[] A = New String[12]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[12]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[12]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[12]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[12]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[12]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[12]
        Return A
    EndFunction
EndState

State A13
    String[] Function GetStringArray()
        String[] A = New String[13]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[13]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[13]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[13]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[13]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[13]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[13]
        Return A
    EndFunction
EndState

State A14
    String[] Function GetStringArray()
        String[] A = New String[14]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[14]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[14]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[14]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[14]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[14]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[14]
        Return A
    EndFunction
EndState

State A15
    String[] Function GetStringArray()
        String[] A = New String[15]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[15]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[15]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[15]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[15]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[15]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[15]
        Return A
    EndFunction
EndState

State A16
    String[] Function GetStringArray()
        String[] A = New String[16]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[16]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[16]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[16]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[16]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[16]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[16]
        Return A
    EndFunction
EndState

State A17
    String[] Function GetStringArray()
        String[] A = New String[17]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[17]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[17]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[17]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[17]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[17]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[17]
        Return A
    EndFunction
EndState

State A18
    String[] Function GetStringArray()
        String[] A = New String[18]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[18]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[18]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[18]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[18]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[18]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[18]
        Return A
    EndFunction
EndState

State A19
    String[] Function GetStringArray()
        String[] A = New String[19]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[19]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[19]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[19]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[19]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[19]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[19]
        Return A
    EndFunction
EndState

State A20
    String[] Function GetStringArray()
        String[] A = New String[20]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[20]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[20]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[20]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[20]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[20]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[20]
        Return A
    EndFunction
EndState

State A21
    String[] Function GetStringArray()
        String[] A = New String[21]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[21]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[21]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[21]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[21]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[21]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[21]
        Return A
    EndFunction
EndState

State A22
    String[] Function GetStringArray()
        String[] A = New String[22]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[22]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[22]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[22]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[22]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[22]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[22]
        Return A
    EndFunction
EndState

State A23
    String[] Function GetStringArray()
        String[] A = New String[23]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[23]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[23]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[23]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[23]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[23]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[23]
        Return A
    EndFunction
EndState

State A24
    String[] Function GetStringArray()
        String[] A = New String[24]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[24]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[24]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[24]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[24]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[24]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[24]
        Return A
    EndFunction
EndState

State A25
    String[] Function GetStringArray()
        String[] A = New String[25]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[25]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[25]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[25]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[25]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[25]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[25]
        Return A
    EndFunction
EndState

State A26
    String[] Function GetStringArray()
        String[] A = New String[26]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[26]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[26]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[26]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[26]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[26]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[26]
        Return A
    EndFunction
EndState

State A27
    String[] Function GetStringArray()
        String[] A = New String[27]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[27]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[27]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[27]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[27]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[27]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[27]
        Return A
    EndFunction
EndState

State A28
    String[] Function GetStringArray()
        String[] A = New String[28]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[28]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[28]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[28]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[28]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[28]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[28]
        Return A
    EndFunction
EndState

State A29
    String[] Function GetStringArray()
        String[] A = New String[29]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[29]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[29]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[29]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[29]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[29]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[29]
        Return A
    EndFunction
EndState

State A30
    String[] Function GetStringArray()
        String[] A = New String[30]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[30]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[30]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[30]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[30]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[30]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[30]
        Return A
    EndFunction
EndState

State A31
    String[] Function GetStringArray()
        String[] A = New String[31]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[31]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[31]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[31]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[31]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[31]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[31]
        Return A
    EndFunction
EndState

State A32
    String[] Function GetStringArray()
        String[] A = New String[32]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[32]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[32]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[32]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[32]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[32]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[32]
        Return A
    EndFunction
EndState

State A33
    String[] Function GetStringArray()
        String[] A = New String[33]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[33]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[33]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[33]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[33]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[33]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[33]
        Return A
    EndFunction
EndState

State A34
    String[] Function GetStringArray()
        String[] A = New String[34]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[34]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[34]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[34]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[34]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[34]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[34]
        Return A
    EndFunction
EndState

State A35
    String[] Function GetStringArray()
        String[] A = New String[35]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[35]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[35]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[35]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[35]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[35]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[35]
        Return A
    EndFunction
EndState

State A36
    String[] Function GetStringArray()
        String[] A = New String[36]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[36]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[36]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[36]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[36]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[36]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[36]
        Return A
    EndFunction
EndState

State A37
    String[] Function GetStringArray()
        String[] A = New String[37]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[37]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[37]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[37]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[37]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[37]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[37]
        Return A
    EndFunction
EndState

State A38
    String[] Function GetStringArray()
        String[] A = New String[38]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[38]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[38]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[38]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[38]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[38]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[38]
        Return A
    EndFunction
EndState

State A39
    String[] Function GetStringArray()
        String[] A = New String[39]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[39]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[39]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[39]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[39]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[39]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[39]
        Return A
    EndFunction
EndState

State A40
    String[] Function GetStringArray()
        String[] A = New String[40]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[40]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[40]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[40]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[40]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[40]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[40]
        Return A
    EndFunction
EndState

State A41
    String[] Function GetStringArray()
        String[] A = New String[41]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[41]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[41]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[41]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[41]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[41]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[41]
        Return A
    EndFunction
EndState

State A42
    String[] Function GetStringArray()
        String[] A = New String[42]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[42]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[42]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[42]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[42]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[42]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[42]
        Return A
    EndFunction
EndState

State A43
    String[] Function GetStringArray()
        String[] A = New String[43]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[43]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[43]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[43]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[43]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[43]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[43]
        Return A
    EndFunction
EndState

State A44
    String[] Function GetStringArray()
        String[] A = New String[44]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[44]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[44]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[44]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[44]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[44]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[44]
        Return A
    EndFunction
EndState

State A45
    String[] Function GetStringArray()
        String[] A = New String[45]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[45]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[45]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[45]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[45]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[45]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[45]
        Return A
    EndFunction
EndState

State A46
    String[] Function GetStringArray()
        String[] A = New String[46]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[46]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[46]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[46]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[46]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[46]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[46]
        Return A
    EndFunction
EndState

State A47
    String[] Function GetStringArray()
        String[] A = New String[47]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[47]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[47]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[47]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[47]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[47]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[47]
        Return A
    EndFunction
EndState

State A48
    String[] Function GetStringArray()
        String[] A = New String[48]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[48]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[48]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[48]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[48]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[48]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[48]
        Return A
    EndFunction
EndState

State A49
    String[] Function GetStringArray()
        String[] A = New String[49]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[49]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[49]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[49]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[49]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[49]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[49]
        Return A
    EndFunction
EndState

State A50
    String[] Function GetStringArray()
        String[] A = New String[50]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[50]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[50]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[50]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[50]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[50]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[50]
        Return A
    EndFunction
EndState

State A51
    String[] Function GetStringArray()
        String[] A = New String[51]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[51]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[51]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[51]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[51]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[51]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[51]
        Return A
    EndFunction
EndState

State A52
    String[] Function GetStringArray()
        String[] A = New String[52]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[52]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[52]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[52]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[52]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[52]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[52]
        Return A
    EndFunction
EndState

State A53
    String[] Function GetStringArray()
        String[] A = New String[53]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[53]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[53]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[53]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[53]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[53]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[53]
        Return A
    EndFunction
EndState

State A54
    String[] Function GetStringArray()
        String[] A = New String[54]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[54]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[54]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[54]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[54]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[54]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[54]
        Return A
    EndFunction
EndState

State A55
    String[] Function GetStringArray()
        String[] A = New String[55]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[55]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[55]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[55]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[55]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[55]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[55]
        Return A
    EndFunction
EndState

State A56
    String[] Function GetStringArray()
        String[] A = New String[56]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[56]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[56]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[56]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[56]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[56]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[56]
        Return A
    EndFunction
EndState

State A57
    String[] Function GetStringArray()
        String[] A = New String[57]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[57]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[57]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[57]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[57]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[57]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[57]
        Return A
    EndFunction
EndState

State A58
    String[] Function GetStringArray()
        String[] A = New String[58]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[58]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[58]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[58]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[58]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[58]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[58]
        Return A
    EndFunction
EndState

State A59
    String[] Function GetStringArray()
        String[] A = New String[59]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[59]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[59]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[59]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[59]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[59]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[59]
        Return A
    EndFunction
EndState

State A60
    String[] Function GetStringArray()
        String[] A = New String[60]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[60]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[60]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[60]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[60]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[60]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[60]
        Return A
    EndFunction
EndState

State A61
    String[] Function GetStringArray()
        String[] A = New String[61]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[61]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[61]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[61]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[61]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[61]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[61]
        Return A
    EndFunction
EndState

State A62
    String[] Function GetStringArray()
        String[] A = New String[62]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[62]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[62]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[62]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[62]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[62]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[62]
        Return A
    EndFunction
EndState

State A63
    String[] Function GetStringArray()
        String[] A = New String[63]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[63]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[63]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[63]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[63]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[63]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[63]
        Return A
    EndFunction
EndState

State A64
    String[] Function GetStringArray()
        String[] A = New String[64]
        Return A
    EndFunction

    Bool[] Function GetBoolArray()
        Bool[] A = New Bool[64]
        Return A
    EndFunction

    Int[] Function GetIntArray()
        Int[] A = New Int[64]
        Return A
    EndFunction

    Float[] Function GetFloatArray()
        Float[] A = New Float[64]
        Return A
    EndFunction

    ObjectReference[] Function GetObjectReferenceArray()
        ObjectReference[] A = New ObjectReference[64]
        Return A
    EndFunction

    Actor[] Function GetActorArray()
        Actor[] A = New Actor[64]
        Return A
    EndFunction

    Form[] Function GetFormArray()
        Form[] A = New Form[64]
        Return A
    EndFunction
EndState 