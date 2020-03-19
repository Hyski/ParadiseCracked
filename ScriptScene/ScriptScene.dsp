# Microsoft Developer Studio Project File - Name="ScriptScene" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ScriptScene - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ScriptScene.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ScriptScene.mak" CFG="ScriptScene - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ScriptScene - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ScriptScene - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Flif projects/ScriptScene", WGIAAAAA"
# PROP Scc_LocalPath "."
CPP=c:\utils\warning_filter\clfilt.bat 
RSC=rc.exe

!IF  "$(CFG)" == "ScriptScene - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"ScriptScene.lib"

!ELSEIF  "$(CFG)" == "ScriptScene - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /Yu"scriptpch.h" /FD /GZ /Zm500 /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"ScriptScene.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"ScriptScene_dbg.lib"

!ENDIF 

# Begin Target

# Name "ScriptScene - Win32 Release"
# Name "ScriptScene - Win32 Debug"
# Begin Group "Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ScriptSceneManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptSceneManager.h
# End Source File
# End Group
# Begin Group "Implementation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\scriptpch.cpp

!IF  "$(CFG)" == "ScriptScene - Win32 Release"

!ELSEIF  "$(CFG)" == "ScriptScene - Win32 Debug"

# ADD CPP /Yc"scriptpch.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scriptpch.h
# End Source File
# Begin Source File

SOURCE=.\SSEpisode1.cpp
# End Source File
# Begin Source File

SOURCE=.\SSEpisode1.h
# End Source File
# Begin Source File

SOURCE=.\SSEpisode2.cpp
# End Source File
# Begin Source File

SOURCE=.\SSEpisode2.h
# End Source File
# Begin Source File

SOURCE=.\SSEpisode3.cpp
# End Source File
# Begin Source File

SOURCE=.\SSEpisode3.h
# End Source File
# Begin Source File

SOURCE=.\SSEpisode4.cpp
# End Source File
# Begin Source File

SOURCE=.\SSEpisode4.h
# End Source File
# End Group
# Begin Group "ExternalAPI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ScriptSceneAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptSceneAPI.h
# End Source File
# End Group
# End Target
# End Project
