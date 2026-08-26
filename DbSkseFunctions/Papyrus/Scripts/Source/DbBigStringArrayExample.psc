scriptname DbBigStringArrayExample extends quest 
{examples of how to use DbBigStringArray as either one big array or as multidimensional arrays.}

MiscObject Property DbBigStringArrayMisc Auto 
{this misc object has the DbBigStringArray and DynamicStringsArrays scripts attached}

DbBigStringArray myBigStringArray

Event OnInit()
    ;create a new persistent big array of 1000 elements, each element set to "testing"
    myBigStringArray = DbBigStringArray.create(DbBigStringArrayMisc, 1000, "testing")
EndEvent

function testPushBack()
    Debug.MessageBox("testPushBack")
    Utility.wait(0.1)

    ;create new temp big array with initial size of 0
    DbBigStringArray tempBigArray = DbBigStringArray.create(DbBigStringArrayMisc, persistent = false)

    int i = 0 
    while i < 1000 
        tempBigArray.pushBack("index: " + i)
        i += 1
    EndWhile 

    FindAndMessageInArray("index: 50", "index: 500", tempBigArray)
    Utility.wait(0.1)
    tempBigArray.Destroy() ;always destroy temp big arrays after done using them
EndFunction

function testBigArrayRemoveAt()
    Debug.MessageBox("testBigArrayRemoveAt")
    Utility.wait(0.1)

    string printString = ("myBigStringArray.size = " + myBigStringArray.size + "\n")
    printString += ("GetAt(0) = " + myBigStringArray.GetAt(0) + "\n")
    printString += ("GetAt(999) = " + myBigStringArray.GetAt(999) + "\n")
    printString += ("GetAt(1000) = " + myBigStringArray.GetAt(1000) + "\n")

    ;remove the string at index 700, shortening the array by 1.
    string removedString = myBigStringArray.RemoveAt(700)
    printString += ("new size = " + myBigStringArray.size + "\n")
    printString += ("removedString = " + removedString + "\n")
    printString += ("GetAt(0) = " + myBigStringArray.GetAt(0) + "\n")
    printString += ("GetAt(998) = " + myBigStringArray.GetAt(998) + "\n")
    printString += ("GetAt(999) = " + myBigStringArray.GetAt(999) + "\n")
    Debug.MessageBox(printString)
EndFunction

function testBigArrayInsert()
    Debug.MessageBox("testBigArrayInsert")
    Utility.wait(0.1)

    myBigStringArray = DbBigStringArray.create(DbBigStringArrayMisc, 1000, "testing")

    string printString = ("myBigStringArray.size = " + myBigStringArray.size + "\n")
    printString += ("GetAt(0) = " + myBigStringArray.GetAt(0) + "\n")
    printString += ("GetAt(999) = " + myBigStringArray.GetAt(999) + "\n")
    printString += ("GetAt(1000) = " + myBigStringArray.GetAt(1000) + "\n")

    myBigStringArray.InsertAt(700, "yo")
    printString += ("new size = " + myBigStringArray.size + "\n")
    printString += ("GetAt(699) = " + myBigStringArray.GetAt(699) + "\n")
    printString += ("GetAt(700) = " + myBigStringArray.GetAt(700) + "\n")
    printString += ("GetAt(701) = " + myBigStringArray.GetAt(701) + "\n")
    printString += ("GetAt(1000) = " + myBigStringArray.GetAt(1000) + "\n")
    printString += ("GetAt(1001) = " + myBigStringArray.GetAt(1001) + "\n")
    Debug.MessageBox(printString)
EndFunction

function IterateThroughBigArray_OptionA()
    Debug.MessageBox("IterateThroughBigArray_OptionA")
    Utility.wait(0.1)

    ;slow way to iterate through the big array (not recommended)
    int i = 0 
    while i < myBigStringArray.size 
        myBigStringArray.SetAt(i, "index: " + i)
        i += 1

        if i % 100 == 0 
            Debug.Notification(i) ;just some feedback to tell this function is running
        Endif
    EndWhile 

    FindAndMessageInArray("index: 50", "index: 500")
EndFunction

function IterateThroughBigArray_OptionB()
    Debug.MessageBox("IterateThroughBigArray_OptionB")
    Utility.wait(0.1)

    ;faster way to iterate through the big array
    ;downside of this way is you can't iterate through more than once at a time

    ;wait until other functions are done iterating through the array (if any)
    myBigStringArray.waitWhileIterating()
    
    ;tell other functions to wait until this one is done iterating
    myBigStringArray.iterating = true

    ;set the current index of the array to -1 for use with SetNext or GetNext functions 
    ;will set or get the first element (0) in the array
    myBigStringArray.SetCurrentIndex(-1)
    int i = 0

    while i < myBigStringArray.size 
        myBigStringArray.setNext("index: " + i)
        i += 1
    EndWhile 

    ;tell other functions this one is done iterating. always set this to false if previously set to true.
    myBigStringArray.iterating = false
    
    ; ;iterate backwards throught the array
    ; int i = myBigStringArray.size
    ; myBigStringArray.SetCurrentIndex(i)

    ; while i > 0
    ;     i -= 1
    ;     myBigStringArray.setPrevious("index: " + i)
    ; EndWhile 
    
    ;tell other functions this one is done iterating. always set this to false if previously set to true.
    ;myBigStringArray.iterating = false

    FindAndMessageInArray("index: 50", "index: 500")
EndFunction

function IterateThroughBigArray_OptionC()
    myBigStringArray = DbBigStringArray.create(DbBigStringArrayMisc, 1000)

    ;fastest way to iterate through the big array
    int index = 0
    int subArrayindex = 0
    while index < myBigStringArray.size
        string[] subArray = myBigStringArray.GetNthSubArray(subArrayIndex)
        int i = 0 
        while i < subArray.length && index < myBigStringArray.size
            subArray[i] = "index: " + index
            i += 1
            index += 1
        EndWhile

        subArrayIndex += 1
    Endwhile 

    FindAndMessageInArray("index: 50", "index: 500")
EndFunction

function ArrayResizeTest1()
    Debug.MessageBox("ArrayResizeTest1")
    Utility.wait(0.1)

    DbBigStringArray tempArray = DbBigStringArray.create(DbBigStringArrayMisc, persistent = false)
    bool success = tempArray.resize(50, "bruh")
    Debug.MessageBox("tempArray.size = " + tempArray.size + "\ntempArray.array0.length = " + \
    tempArray.array0.length + "\ntempArray.array1.length = " + tempArray.array1.length + \
    "\nGetAt(49) = " + tempArray.GetAt(49) + "\nGetAt(50) = " + tempArray.GetAt(50) + "\nGetAt(51) = " + tempArray.GetAt(51) + \
    "\n[49] = " + tempArray.array0[49] + "\n[50] = " + tempArray.array0[50] + "\n[51] = " + tempArray.array0[51])

    Utility.wait(0.1)
    tempArray.destroy() ;always destory the temp big array after done using
EndFunction

function BigStringArrayResizeTest2()
    Debug.MessageBox("BigStringArrayResizeTest2")
    Utility.wait(0.1)

    DbBigStringArray tempArray = DbBigStringArray.create(DbBigStringArrayMisc, persistent = false)
    bool success = tempArray.resize(51, "yo")
    Debug.MessageBox(tempArray.GetAt(50) + " " + tempArray.Array0[50])

    Utility.wait(0.1)
    success = tempArray.resize(129, "bruh")

    Debug.MessageBox("success = " + success + "\ntempArray.size = " + tempArray.size + "\ntempArray.array0.length = " + \
    tempArray.array0.length + "\ntempArray.array1.length = " + tempArray.array1.length + \
    "\nGetAt(127) = " + tempArray.GetAt(127) + "\nGetAt(128) = " + tempArray.GetAt(128) + "\nGetAt(129) = " + tempArray.GetAt(129) + \
    "\n[127] = " + tempArray.array0[127] + "\n[0] = " + tempArray.array1[0] + "\n[1] = " + tempArray.array1[1])
    Utility.wait(0.1)
    
    success = tempArray.resize(101)
    ExtendedVanillaMenus.MessageBox("success = " + success + "\ntempArray.size = " + tempArray.size + "\ntempArray.array0.length = " + \
    tempArray.array0.length + "\ntempArray.array1.length = " + tempArray.array1.length + \
    "\nGetAt(127) = " + tempArray.GetAt(127) + "\nGetAt(128) = " + tempArray.GetAt(128) + "\nGetAt(129) = " + tempArray.GetAt(129) + \
    "\n[127] = " + tempArray.array0[127] + "\n[0] = " + tempArray.array1[0] + "\n[1] = " + tempArray.array1[1] + \
    "\nGetAt(100) = " + tempArray.GetAt(100) + "\n[100] = " + tempArray.array0[100] + \
    "\nGetAt(50) = " + tempArray.GetAt(50) + "\n[50] = " + tempArray.array0[50], html = false)

    Utility.wait(0.1)
    tempArray.destroy() ;always destory the temp big array after done using
EndFunction

function BigStringArray_A_Example()
    Debug.MessageBox("BigStringArray_A_Example")
    Utility.wait(0.1)
    
    ;create a new big string array with a size of 1000
    myBigStringArray = DbBigStringArray.create(DbBigStringArrayMisc, 1000)
    if myBigStringArray ;success 
        int i = 0 
        while i < myBigStringArray.size ;iterate through the array
            string s = "index " + i
            myBigStringArray.SetAt(i, s) ;set the big array index to s
            i += 1
        EndWhile 
    
        i = 200 
        string printString = "big string array between 200 and 210. Size = " + myBigStringArray.size + "\n"
        while i <= 210
            printString += ("i: " + i + " " + myBigStringArray.GetAt(i) + "\n") ;add string at index i to the printString
            i += 1
        EndWhile
    
        ; printString += (myBigStringArray.GetAt(240)) 

        ; ExtendedVanillaMenus.MessageBox(printString, html = false)
        debug.MessageBox(printString)
    Endif
EndFunction

function BigStringArray_B_Example()
    Debug.MessageBox("BigStringArray_B_Example")
    Utility.wait(0.1)
    
    ;create temporary big string array with initial size of 0
    DbBigStringArray tempArray = DbBigStringArray.create(DbBigStringArrayMisc, persistent = false)
    if tempArray
        tempArray.pushBack("zero")
        tempArray.pushBack("one")
        tempArray.pushBack("two")
        tempArray.pushBack("three") 

        string A = tempArray.GetAt(0)
        string B = tempArray.GetAt(1)
        string C = tempArray.GetAt(2)
        string D = tempArray.GetAt(3)

        Debug.MessageBox("another big array size is " + tempArray.GetSize() + "\n" + A + "\n" + B + "\n" + C + "\n" + D)
        Utility.wait(1)
        tempArray.Destroy() ;always Destroy temporary arrays after done using
    Endif
EndFunction

DbBigStringArray multiArray3X3

Function MultiArray3X3_Example()
    Debug.MessageBox("MultiArray3X3_Example")
    Utility.wait(0.1)
    
    ;create a new persistent big string array to use as a multidimentional array (3X3)
    ;the inner sub arrays have a max size of 100.
    multiArray3X3 = DbBigStringArray.CreateMultiArray(DbBigStringArrayMisc, 3, 3)
    if multiArray3X3 

        ;get the first array in the multi array
        string[] subArray0 = multiArray3X3.GetNthSubArray(0)
        subArray0[0] = "subArray0 0"
        subArray0[1] = "subArray0 1"
        subArray0[2] = "subArray0 2"

        ;get the second array in the multi array
        string[] subArray1 = multiArray3X3.GetNthSubArray(1)
        subArray1[0] = "subArray1 0"
        subArray1[1] = "subArray1 1"
        subArray1[2] = "subArray1 2"

        ;can also reference the sub arrays directly rather than using GetNthSubArray
        multiArray3X3.Array2[0] = "subArray2 0"
        multiArray3X3.Array2[1] = "subArray2 1"
        multiArray3X3.Array2[2] = "subArray2 2"

        string printString = "multiArray 3 X 3\n"
        int outerIndex = 0 
        Debug.Messagebox("multiArray3X3.NumberOfArrays = " + multiArray3X3.NumberOfArrays)
        Utility.wait(0.1)
        while outerIndex < multiArray3X3.NumberOfArrays
            string[] innerArray = multiArray3X3.GetNthSubArray(outerIndex)
            int innerIndex = 0 
            printString += ("innerArray array " + outerIndex + " length = " + innerArray.length + "\n")
            while innerIndex < innerArray.length
                printString += "[" + outerIndex + "][" + innerIndex + "] = " + innerArray[innerIndex] + "\n"
                innerIndex += 1
            EndWhile

            outerIndex += 1
        EndWhile

        ; Debug.MessageBox(printString)
        ExtendedVanillaMenus.MessageBox(printString, html = false)
    Endif
EndFunction

DbBigStringArray[] bigMultiArray3X3

function bigMultiArray3X3_Example()
    Debug.MessageBox("bigMultiArray3X3_Example")
    Utility.wait(0.1)
    
    ;array of 3 bigStringArrays with an initial size of 1000
    bigMultiArray3X3 = new DbBigStringArray[3]
    bigMultiArray3X3[0] = DbBigStringArray.create(DbBigStringArrayMisc, 1000)
    bigMultiArray3X3[1] = DbBigStringArray.create(DbBigStringArrayMisc, 1000)
    bigMultiArray3X3[2] = DbBigStringArray.create(DbBigStringArrayMisc, 1000)

    if bigMultiArray3X3[0] && bigMultiArray3X3[1] && bigMultiArray3X3[2] ;assert that all arrays were created
        
        bigMultiArray3X3[0].SetAt(500, "array0: 500")
        bigMultiArray3X3[0].SetAt(501, "array0: 501")

        bigMultiArray3X3[1].SetAt(500, "array1: 500")
        bigMultiArray3X3[1].SetAt(501, "array1: 501")

        bigMultiArray3X3[2].SetAt(500, "array2: 500")
        bigMultiArray3X3[2].SetAt(501, "array2: 501")

        string printString = "bigMultiArray 3 X 1000 \n"
        int outerIndex = 0 
        while outerIndex < bigMultiArray3X3.length 
            printString += "[" + outerIndex + "][500] = " + bigMultiArray3X3[outerIndex].GetAt(500) + "\n"
            printString += "[" + outerIndex + "][501] = " + bigMultiArray3X3[outerIndex].GetAt(501) + "\n" 
            outerIndex += 1
        EndWhile

        Debug.MessageBox(printString)
    Endif
EndFunction 

DbBigStringArray[] multiArray2X2X2

Function multiArray2X2X2_Example()
    Debug.MessageBox("multiArray2X2X2_Example")
    Utility.wait(0.1)
    
    ;4 x 4 x 4 multi array
    multiArray2X2X2 = new DbBigStringArray[2]
    multiArray2X2X2[0] = DbBigStringArray.CreateMultiArray(DbBigStringArrayMisc, 2, 2)
    multiArray2X2X2[1] = DbBigStringArray.CreateMultiArray(DbBigStringArrayMisc, 2, 2)

    if multiArray2X2X2[0] && multiArray2X2X2[1] ;assert that all arrays were created
        int outerIndex = 0

        ;initialize the array
        while outerIndex < multiArray2X2X2.length 
            int innerIndex = 0

            while innerIndex < multiArray2X2X2[outerIndex].NumberOfArrays
                string[] innerArray = multiArray2X2X2[outerIndex].GetNthSubArray(innerIndex)
                int i = 0 
                while i < innerArray.length 
                    innerArray[i] = "[" + outerIndex + "][" + innerIndex + "[" + i + "]"
                    i += 1
                EndWhile

                innerIndex += 1
            EndWhile

            outerIndex += 1
        EndWhile

        string printString = "multiArray 2 X 2 X 2 \n"

        outerIndex = 0
        while outerIndex < multiArray2X2X2.length ;iterate through the multi array
            int innerIndex = 0 

            while innerIndex < multiArray2X2X2[outerIndex].NumberOfArrays
                string[] innerArray = multiArray2X2X2[outerIndex].GetNthSubArray(innerIndex)
                int i = 0 
                while i < innerArray.length 
                    printString += innerArray[i] + "\n"
                    i += 1
                EndWhile

                innerIndex += 1
            EndWhile

            outerIndex += 1
        EndWhile

        Debug.MessageBox(printString)
        ; ExtendedVanillaMenus.MessageBox(printString, html = false)
    Endif
EndFunction

function FindAndMessageInArray(string stringA, string stringB, DbBigStringArray akArray = none)
    if akArray 
        int foundIndexA = akArray.find(stringA)
        int foundIndexB = akArray.find(stringB)
        Debug.MessageBox("foundIndexA = " + foundIndexA + " string = [" + akArray.GetAt(foundIndexA) + \
        "]\nfoundIndexB = " + foundIndexB + " string = [" + akArray.GetAt(foundIndexB) + "]")
    Else 
        int foundIndexA = myBigStringArray.find(stringA)
        int foundIndexB = myBigStringArray.find(stringB)
        Debug.MessageBox("foundIndexA = " + foundIndexA + " string = [" + myBigStringArray.GetAt(foundIndexA) + \
        "]\nfoundIndexB = " + foundIndexB + " string = [" + myBigStringArray.GetAt(foundIndexB) + "]")
    Endif
EndFunction