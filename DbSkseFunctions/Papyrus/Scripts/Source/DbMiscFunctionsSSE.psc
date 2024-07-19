Scriptname DbMiscFunctionsSSE Hidden

Import DbMiscFunctions

;Get all file paths in directory, including files in sub folders. 
;If Fullpath == true (default) get's full paths, e.g Data/Interface/MyFile.txt 
;Otherwise gets e.g MyFile.txt 
;Requires skse and papyrusUtil.
String[] Function GetAllFilesInFolder(string directory, string extension="*", Bool FullPath = true) Global
    If FullPath
        String[] Folders = GetAllFoldersInFolder(directory)
        String[] Files = MiscUtil.FilesInFolder(directory, extension)
        
        Files = AddPrefixToStrings(Files, (directory + "/"), true)
        
        Int i = 0 
        Int L = Folders.Length 
        While i < L 
            String[] SubFiles = MiscUtil.FilesInFolder(Folders[i], extension)
            If SubFiles.Length > 0 
                SubFiles = AddPrefixToStrings(SubFiles, (Folders[i] + "/"), true)
                Files = PapyrusUtil.MergeStringArray(Files, SubFiles)
            Endif
            i += 1
        EndWhile
        
        Return Files
    Else 
        String[] Folders = GetAllFoldersInFolder(directory)
        String[] Files = MiscUtil.FilesInFolder(directory, extension)
        Int i = 0 
        Int L = Folders.Length 
        While i < L 
            String[] SubFiles = MiscUtil.FilesInFolder(Folders[i], extension)
            If SubFiles.Length > 0 
                Files = PapyrusUtil.MergeStringArray(Files, SubFiles)
            Endif
            i += 1
        EndWhile
        
        Return Files
    Endif
EndFunction 

;Get all folder paths in directory, including sub folders. 
;Requires skse and papyrusUtil.
String[] Function GetAllFoldersInFolder(String directory) Global
    String[] Folders = MiscUtil.FoldersInFolder(directory)
    Folders = AddPrefixToStrings(Folders, (directory + "/"))
    
    Int i = 0 
    
    While i < Folders.Length  
        String[] SubFolders = MiscUtil.FoldersInFolder(Folders[i])
        If SubFolders.Length > 0 
            SubFolders = AddPrefixToStrings(SubFolders, (Folders[i] + "/"), true)
            Folders = PapyrusUtil.MergeStringArray(Folders, SubFolders)
        Endif 
        i += 1
    EndWhile 
    
    Return Folders
EndFunction

;Write all of the file paths found in the Directory, including files in sub folders to the OutputFilePath. 
;If NullString != none, will only write file paths not found in the NullString
;If AllowDuplicates == false (default), only writes file paths not already present in the OutputFilePath
;Requires skse and PapyrusUtil
Function WriteAllFilePathsToFile(String OutputFilePath, String Directory, string extension="*", Bool FullPath = true, String NullString = "", Bool AllowDuplicates = False) Global
    String[] Files = GetAllFilesInFolder(Directory, extension, FullPath)
    Int i = 0
    Int L = Files.length 
    
    String FileContents = MiscUtil.ReadFromFile(OutputFilePath)
    
    If !AllowDuplicates && NullString != ""
        While i < L 
            If StringUtil.Find(FileContents, Files[i]) == -1 && StringUtil.Find(NullString, Files[i]) == -1
                MiscUtil.WriteToFile(OutputFilePath, "\n" + Files[i])
                FileContents += ("\n" + Files[i])
            Endif
            i += 1 
        EndWhile
        
    Elseif !AllowDuplicates
        While i < L 
            If StringUtil.Find(FileContents, Files[i]) == -1
                MiscUtil.WriteToFile(OutputFilePath, "\n" + Files[i])
                FileContents += ("\n" + Files[i])
            Endif
            i += 1 
        EndWhile
        
    Elseif NullString != ""
        While i < L 
            If StringUtil.Find(NullString, Files[i]) == -1
                MiscUtil.WriteToFile(OutputFilePath, "\n" + Files[i])
            Endif
            i += 1 
        EndWhile
        
    Else 
        While i < L 
            MiscUtil.WriteToFile(OutputFilePath, "\n" + Files[i])
            i += 1 
        EndWhile
    Endif
EndFunction

;Write all of the Folder paths found in the Directory, including sub folders, to the OutputFolderPath. 
;If NullString != none, will only write Folder paths not found in the NullString
;If AllowDuplicates == false (default), only writes Folder paths not already present in the OutputFolderPath
;Requires skse and PapyrusUtil
Function WriteAllFolderPathsToFile(String OutputFilePath, String Directory, String NullString = "", Bool AllowDuplicates = False) Global
    String[] Folders = GetAllFoldersInFolder(Directory)
    Int i = 0
    Int L = Folders.length 
    
    String FolderContents = MiscUtil.ReadFromFile(OutputFilePath)
    
    If !AllowDuplicates && NullString != ""
        While i < L 
            If StringUtil.Find(FolderContents, Folders[i]) == -1 && StringUtil.Find(NullString, Folders[i]) == -1
                MiscUtil.WriteToFile(OutputFilePath, "\n" + Folders[i])
                FolderContents += ("\n" + Folders[i])
            Endif
            i += 1 
        EndWhile
        
    Elseif !AllowDuplicates
        While i < L 
            If StringUtil.Find(FolderContents, Folders[i]) == -1
                MiscUtil.WriteToFile(OutputFilePath, "\n" + Folders[i])
                FolderContents += ("\n" + Folders[i])
            Endif
            i += 1 
        EndWhile
        
    Elseif NullString != ""
        While i < L 
            If StringUtil.Find(NullString, Folders[i]) == -1
                MiscUtil.WriteToFile(OutputFilePath, "\n" + Folders[i])
            Endif
            i += 1 
        EndWhile
        
    Else 
        While i < L 
            MiscUtil.WriteToFile(OutputFilePath, "\n" + Folders[i])
            i += 1 
        EndWhile
    Endif
EndFunction