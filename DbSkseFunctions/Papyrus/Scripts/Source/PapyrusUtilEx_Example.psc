scriptname PapyrusUtilEx_Example extends quest 

MiscObject[] Property miscArrayA Auto 

MiscObject[] miscArrayB

Event OnInit()
    ;arrays must be initialized before used in PapyrusUtilEx functions. You can do it like this or in the creation kit.
    miscArrayA = new MiscObject[1]
    miscArrayB = new MiscObject[1] 

    ;get the handle of this quest for use with PapyrusUtilEx functions
    string handle = PapyrusUtilEx.GetFormHandle(self)

    ;resize miscArrayA to 500 and fill it with the element at index 0 (in this case none)
    int arraySize = 500
    if PapyrusUtilEx.ResizeArray(handle, "PapyrusUtilEx_Example", "miscArrayA", arraySize, 0)
        ;array resized successfully 
    endif 

    ;remove the element at index 10 in miscArrayA, shortening the array length by 1 
    PapyrusUtilEx.RemoveFromArray(handle, "PapyrusUtilEx_Example", "miscArrayA", 10) 

    ;remove all elements from the miscArrayA that match the element at index 20, returning the number removed
    int numRemoved = PapyrusUtilEx.RemoveFromArray(handle, "PapyrusUtilEx_Example", "miscArrayA", 20, true) 
    
    ;remove the elements at indexes from 40 to 50 in miscArrayA, shortening its length and add them to the end of miscArrayB, increasing its length.
    PapyrusUtilEx.SliceArrayOnto(handle, "PapyrusUtilEx_Example", "miscArrayA", handle, "PapyrusUtilEx_Example", "miscArrayB", 40, 50, false, false)

    ;can't do this as miscArrayC is defined inside an event. 
    ;Arrays used in PapyrusUtilEx functions must be defined in the global scope outside of functions or events.
    MiscObject[] miscArrayC = new MiscObject[1]
    PapyrusUtilEx.ResizeArray(handle, "PapyrusUtilEx_Example", "miscArrayC", arraySize, 0) 

    ;Each time you use a PapyrusUtilEx function on an array though, the array is recreated, 
    ;so if you need temporary arrays you can use a placeholder array in the global scope and write a wrapper function like so: 
    TextureSet[] TextureSetArrayA = CreateTextureSetArray(200)
    TextureSet[] TextureSetArrayB = CreateTextureSetArray(300) 

    ;TextureSetArrayA and TextureSetArrayB are now two different arrays of two different sizes (200 and 300)
EndEvent 

TextureSet[] placeHolderTextureSetArray

TextureSet[] function CreateTextureSetArray(int size, TextureSet filler = none)
    ;clear / init the placeholder array first
    placeHolderTextureSetArray = new TextureSet[1]
    placeHolderTextureSetArray[0] = filler 

    string handle = PapyrusUtilEx.GetFormHandle(self)

    PapyrusUtilEx.ResizeArray(handle, "PapyrusUtilEx_Example", "placeHolderTextureSetArray", size, 0)
    return placeHolderTextureSetArray 
EndFunction 

TextureSet[] function PushTextureSet(TextureSet[] akTextureSet, textureSet toPush)
    int size = akTextureSet.length 
    if (size == 0)
        akTextureSet = new TextureSet[1]
        akTextureSet[0] = toPush 
        return akTextureSet
    Else 
        string handle = PapyrusUtilEx.GetFormHandle(self)
        placeHolderTextureSetArray = akTextureSet  
        PapyrusUtilEx.ResizeArray(handle, "PapyrusUtilEx_Example", "placeHolderTextureSetArray", (size + 1))
        akTextureSet = placeHolderTextureSetArray 
        akTextureSet[size] = toPush 
        placeHolderTextureSetArray = new TextureSet[1]
        return akTextureSet
    Endif
EndFunction