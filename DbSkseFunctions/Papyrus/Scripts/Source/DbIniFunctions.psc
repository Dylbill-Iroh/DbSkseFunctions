Scriptname DbIniFunctions Hidden 
;/This is for convenience to have get / set / has custom ini functions work on either SE or LE 
Uses PapyrusIniManipulator for SE or PapyrusIni for LE
These functions assume using SE PapyrusIniManipulator as default, which means file path  is relative to the Skyrim root folder. So use: "Data/MySettings.ini" for ini files in the data folder. 
PapyrusIni is relative to the data folder, so to work on either SE or LE make sure your custom ini files are in the Data folder.
If called on LE, these functions will auto convert the file paths for use with PapyrusIni. 
Note, if using these you have to have both PapyrusIniManipulator.psc and PapyrusIni.psc in your source folder to compile. 
Requires DbMiscFunctions as well.

example: in MySettings.ini you have: 
[MySection]
iMyInt=3

DbIniFunctions.GetIniInt("Data/MySettings.ini", "MySection", "iMyInt") will return 3 on both SE and LE, if PapyrusIniManipulator is installed on SE or PapyrusIni is installed on LE.

List Of Functions: 
Bool Function GetIniBool(String sFilePath, String sSection, String sKey, Bool Default = false) Global
int Function GetIniInt(String sFilePath, String sSection, String sKey, int Default = 0) Global
Float Function GetIniFloat(String sFilePath, String sSection, String sKey, Float Default = 0.0) Global
String Function GetIniString(String sFilePath, String sSection, String sKey, String Default = "") Global

Bool Function SetIniBool(String sFilePath, String sSection, String sKey, Bool Value, Bool bForce = False) Global
Bool Function SetIniInt(String sFilePath, String sSection, String sKey, Int Value, Bool bForce = False) Global
Bool Function SetIniFloat(String sFilePath, String sSection, String sKey, Float Value, Bool bForce = False) Global
Bool Function SetIniString(String sFilePath, String sSection, String sKey, String Value, Bool bForce = False) Global

Bool Function HasIniBool(String sFilePath, String sSection, String sKey) Global
Bool Function HasIniInt(String sFilePath, String sSection, String sKey) Global
Bool Function HasIniFloat(String sFilePath, String sSection, String sKey) Global
Bool Function HasIniString(String sFilePath, String sSection, String sKey) Global

String Function ConvertFilePathFromSEtoLE(String sFilePath) Global
/;

;Getters============================================================================================
Bool Function GetIniBool(String sFilePath, String sSection, String sKey, Bool Default = false) Global 
    If PapyrusIniManipulator.GetVersion() != ""
        return PapyrusIniManipulator.PullBoolFromIni(sFilePath, sSection, sKey, Default)
        
    ElseIf PapyrusIni.GetPluginVersion() >= 2  
        sFilePath = ConvertFilePathFromSEtoLE(sFilePath)
        Return PapyrusIni.ReadBool(sFilePath, (sKey + ":" + sSection), Default)
    Else 
        return Default 
    Endif
EndFunction 

int Function GetIniInt(String sFilePath, String sSection, String sKey, int Default = 0) Global 
    If PapyrusIniManipulator.GetVersion() != ""
        return PapyrusIniManipulator.PullIntFromIni(sFilePath, sSection, sKey, Default)
        
    ElseIf PapyrusIni.GetPluginVersion() >= 2  
        sFilePath = ConvertFilePathFromSEtoLE(sFilePath)
        Return PapyrusIni.ReadInt(sFilePath, (sKey + ":" + sSection), Default)
    Else 
        return Default 
    Endif
EndFunction 

Float Function GetIniFloat(String sFilePath, String sSection, String sKey, Float Default = 0.0) Global 
    If PapyrusIniManipulator.GetVersion() != ""
        return PapyrusIniManipulator.PullFloatFromIni(sFilePath, sSection, sKey, Default)
        
    ElseIf PapyrusIni.GetPluginVersion() >= 2  
        sFilePath = ConvertFilePathFromSEtoLE(sFilePath)
        Return PapyrusIni.ReadFloat(sFilePath, (sKey + ":" + sSection), Default)
    Else 
        return Default 
    Endif
EndFunction 

String Function GetIniString(String sFilePath, String sSection, String sKey, String Default = "") Global 
    If PapyrusIniManipulator.GetVersion() != ""
        return PapyrusIniManipulator.PullStringFromIni(sFilePath, sSection, sKey, Default)
        
    ElseIf PapyrusIni.GetPluginVersion() >= 2  
        sFilePath = ConvertFilePathFromSEtoLE(sFilePath)
        Return PapyrusIni.ReadString(sFilePath, (sKey + ":" + sSection), Default, 1280)
    Else 
        return Default 
    Endif
EndFunction 

;Setters =======================================================================================================
;if bForce == true, will add the sKey to the sSection if it doesn't exist.
Bool Function SetIniBool(String sFilePath, String sSection, String sKey, Bool Value, Bool bForce = False) Global 
    If PapyrusIniManipulator.GetVersion() != ""
        return PapyrusIniManipulator.PushBoolToIni(sFilePath, sSection, sKey, Value, bForce)
        
    ElseIf PapyrusIni.GetPluginVersion() >= 2  
        sFilePath = ConvertFilePathFromSEtoLE(sFilePath)
        String settingName = sKey + ":" + sSection
        If bForce == true || PapyrusIni.HasBool(sFilePath, settingName)
            PapyrusIni.WriteBool(sFilePath, settingName, Value)
            return (PapyrusIni.ReadBool(sFilePath, settingName, !Value) == Value)
        Endif
    Else 
        return false
    Endif
EndFunction 

Bool Function SetIniInt(String sFilePath, String sSection, String sKey, Int Value, Bool bForce = False) Global 
    If PapyrusIniManipulator.GetVersion() != ""
        return PapyrusIniManipulator.PushIntToIni(sFilePath, sSection, sKey, Value, bForce)
        
    ElseIf PapyrusIni.GetPluginVersion() >= 2  
        sFilePath = ConvertFilePathFromSEtoLE(sFilePath)
        String settingName = sKey + ":" + sSection
        If bForce == true || PapyrusIni.HasInt(sFilePath, settingName)
            PapyrusIni.WriteInt(sFilePath, settingName, Value)
            return (PapyrusIni.ReadInt(sFilePath, settingName, (Value + 1)) == Value)
        Endif
    Else 
        return false
    Endif
EndFunction 

Bool Function SetIniFloat(String sFilePath, String sSection, String sKey, Float Value, Bool bForce = False) Global 
    If PapyrusIniManipulator.GetVersion() != ""
        return PapyrusIniManipulator.PushFloatToIni(sFilePath, sSection, sKey, Value, bForce)
        
    ElseIf PapyrusIni.GetPluginVersion() >= 2  
        sFilePath = ConvertFilePathFromSEtoLE(sFilePath)
        String settingName = sKey + ":" + sSection
        If bForce == true || PapyrusIni.HasFloat(sFilePath, settingName)
            PapyrusIni.WriteFloat(sFilePath, settingName, Value)
            return (PapyrusIni.ReadFloat(sFilePath, settingName, (Value + 1.0)) == Value)
        Endif
    Else 
        return false
    Endif
EndFunction 

Bool Function SetIniString(String sFilePath, String sSection, String sKey, String Value, Bool bForce = False) Global 
    If PapyrusIniManipulator.GetVersion() != ""
        return PapyrusIniManipulator.PushStringToIni(sFilePath, sSection, sKey, Value, bForce)
        
    ElseIf PapyrusIni.GetPluginVersion() >= 2  
        sFilePath = ConvertFilePathFromSEtoLE(sFilePath)
        String settingName = sKey + ":" + sSection
        If bForce == true || PapyrusIni.HasString(sFilePath, settingName)
            PapyrusIni.WriteString(sFilePath, settingName, Value)
            return (PapyrusIni.ReadString(sFilePath, settingName, (Value + "|")) == Value)
        Endif
    Else 
        return false
    Endif
EndFunction 

;Has functions==================================================================
Bool Function HasIniBool(String sFilePath, String sSection, String sKey) Global
    If PapyrusIniManipulator.GetVersion() != ""
        return PapyrusIniManipulator.IniDataExists(2, sFilePath, sSection, sKey)
        
    ElseIf PapyrusIni.GetPluginVersion() >= 2  
        sFilePath = ConvertFilePathFromSEtoLE(sFilePath)
        String settingName = sKey + ":" + sSection
        return PapyrusIni.HasBool(sFilePath, settingName)
    Endif
EndFunction

Bool Function HasIniInt(String sFilePath, String sSection, String sKey) Global
    If PapyrusIniManipulator.GetVersion() != ""
        return PapyrusIniManipulator.IniDataExists(2, sFilePath, sSection, sKey)
        
    ElseIf PapyrusIni.GetPluginVersion() >= 2  
        sFilePath = ConvertFilePathFromSEtoLE(sFilePath)
        String settingName = sKey + ":" + sSection
        return PapyrusIni.HasInt(sFilePath, settingName)
    Endif
EndFunction

Bool Function HasIniFloat(String sFilePath, String sSection, String sKey) Global
    If PapyrusIniManipulator.GetVersion() != ""
        return PapyrusIniManipulator.IniDataExists(2, sFilePath, sSection, sKey)
        
    ElseIf PapyrusIni.GetPluginVersion() >= 2  
        sFilePath = ConvertFilePathFromSEtoLE(sFilePath)
        String settingName = sKey + ":" + sSection
        return PapyrusIni.HasFloat(sFilePath, settingName)
    Endif
EndFunction

Bool Function HasIniString(String sFilePath, String sSection, String sKey) Global
    If PapyrusIniManipulator.GetVersion() != ""
        return PapyrusIniManipulator.IniDataExists(2, sFilePath, sSection, sKey)
        
    ElseIf PapyrusIni.GetPluginVersion() >= 2  
        sFilePath = ConvertFilePathFromSEtoLE(sFilePath)
        String settingName = sKey + ":" + sSection
        return PapyrusIni.HasString(sFilePath, settingName)
    Endif
EndFunction
;==================================================================================

;used for the above functions
;Requires DbMiscFunctions and skse
String Function ConvertFilePathFromSEtoLE(String sFilePath) Global
    sFilePath = DbMiscFunctions.StringReplace(sFilePath, "/", "\\") ;replace / with \\ in file path for use with PapyrusIni.
    int iData = StringUtil.Find(sFilePath, "Data")
    If  iData > -1 
        sFilePath = StringUtil.SubString(sFilePath, (iData + 5)) ;remove data/ from file path
    Endif  
    return sFilePath
Endfunction 

; used to write a function in your script to create an ini file for your mod. 
; Let's say you have a script Data/Scripts/Source/MyScript.psc 
; In the script you have a bunch of GetIni functions from this script like: 
; String MyString = DbIniFunctions.GetIniString("Data/Interface/MyMod/Settings.ini", "Strings", "MyString", "My String")
; int MyInt = DbIniFunctions.GetIniInt("Data/Interface/MyMod/Settings.ini", "Main", "MyInt", 42)
; Using this function: 'WriteForceSetIniFunction("Data/Scripts/Source/MyScript.psc", "Data/Scripts/Source/MyScript.psc", "CreateIniFile")'
; Will write this function in your script: 
; Function CreateIniFile()
;     DbIniFunctions.SetIniString("Data/Interface/MyMod/Settings.ini", "Strings", "MyString", "My String", true)
;     DbIniFunctions.SetIniInt("Data/Interface/MyMod/Settings.ini", "Main", "MyInt", 42, true)
; Endfunction
; You can then use this function to create your ini file and write all the inis in one go.
; requires skse
Function WriteForceSetIniFunction(String inputFilePath, string outputFilePath, string functionName, bool onlyInisWithDefaults = true) global
    string writtenLines 

    String Contents = MiscUtil.ReadFromFile(inputFilePath) 
	String outputContents = MiscUtil.ReadFromFile(outputFilePath) 
	MiscUtil.WriteToFile(outputFilePath, "\n\nFunction " + functionName + "()\n")

    int i = StringUtil.Find(Contents, "DbIniFunctions.GetIni")
    int L = StringUtil.GetLength(Contents)
    
    int advanceLength = StringUtil.GetLength("DbIniFunctions.GetIni")
    
    bool dontAdvanceI = false
    
    if onlyInisWithDefaults
        While i > -1 
            Int LineStart = i - 1
            String Char = StringUtil.GetNthChar(Contents, LineStart)
            While LineStart > 0 && Char != "\n" 
                LineStart -= 1 
                Char = StringUtil.GetNthChar(Contents, LineStart)
            EndWhile 
            
            Int LineEnd = StringUtil.Find(Contents, "\n", i) 
            If LineEnd == -1 
                LineEnd = L
            Endif 

            int paramsStart = StringUtil.Find(contents, "(", i)
            
            if paramsStart < lineEnd && paramsStart > -1
                int NextComment = StringUtil.Find(Contents, ";", LineStart)

                if (nextComment != -1 && paramsStart > nextComment) ;function or type is commented

                Elseif DbMiscFunctions.IsIndexInBlockComment(contents, paramsStart) 
                    int NextBlockCommentEnd = StringUtil.Find(Contents, "/;", paramsStart)
                    if NextBlockCommentEnd > -1 
                        i = NextBlockCommentEnd 
                        dontAdvanceI = true
                    Endif
                Else 
                    string params = DbMiscFunctions.GetStringBetweenOuterCharacters(contents, (paramsStart - 1))
                    if DbMiscFunctions.CountStringsInString(params, ",") == 3
                        i += advanceLength
                        dontAdvanceI = true
                        string restOfFunction = StringUtil.SubString(contents, i, (paramsStart - i + 1))
                        string functionLine = "DbIniFunctions.SetIni" + restOfFunction + params + ", true)"
                        if StringUtil.Find(writtenLines, functionLine) == -1 && StringUtil.Find(outputContents, functionLine) == -1
                            MiscUtil.WriteToFile(outputFilePath, "\t" + functionLine + "\n")
                            writtenLines = (writtenLines + functionLine + "\n")
                        Endif
                    Endif
                Endif
            Endif

            if dontAdvanceI
                dontAdvanceI = false
            Else 
                i += advanceLength
            Endif

            i = StringUtil.Find(Contents, "DbIniFunctions.GetIni", i)
        EndWhile 
    Else 
        While i > -1 
            Int LineStart = i - 1
            String Char = StringUtil.GetNthChar(Contents, LineStart)
            While LineStart > 0 && Char != "\n" 
                LineStart -= 1 
                Char = StringUtil.GetNthChar(Contents, LineStart)
            EndWhile 
            
            Int LineEnd = StringUtil.Find(Contents, "\n", i) 
            If LineEnd == -1 
                LineEnd = L
            Endif 

            int paramsStart = StringUtil.Find(contents, "(", i)
            
            if paramsStart < lineEnd && paramsStart > -1
                int NextComment = StringUtil.Find(Contents, ";", LineStart)

                if (nextComment != -1 && paramsStart > nextComment) ;function or type is commented

                Elseif DbMiscFunctions.IsIndexInBlockComment(contents, paramsStart) 
                    int NextBlockCommentEnd = StringUtil.Find(Contents, "/;", paramsStart)
                    if NextBlockCommentEnd > -1 
                        i = NextBlockCommentEnd 
                        dontAdvanceI = true
                    Endif
                Else 
                    string params = DbMiscFunctions.GetStringBetweenOuterCharacters(contents, (paramsStart - 1))
                    if params != ""
                        i += advanceLength
                        dontAdvanceI = true
                        string restOfFunction = StringUtil.SubString(contents, i, (paramsStart - i + 1))
                        string functionLine = "DbIniFunctions.SetIni" + restOfFunction + params + ", true)"
                        if StringUtil.Find(writtenLines, functionLine) == -1 && StringUtil.Find(outputContents, functionLine) == -1
                            MiscUtil.WriteToFile(outputFilePath, "\t" + functionLine + "\n")
                            writtenLines = (writtenLines + functionLine + "\n")
                        Endif
                    Endif
                Endif
            Endif

            if dontAdvanceI
                dontAdvanceI = false
            Else 
                i += advanceLength
            Endif

            i = StringUtil.Find(Contents, "DbIniFunctions.GetIni", i)
        EndWhile 
    Endif
    MiscUtil.WriteToFile(outputFilePath, "EndFunction")
EndFunction