# Microsoft Developer Studio Project File - Name="slib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=slib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Slib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Slib.mak" CFG="slib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "slib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "slib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "slib - Win32 IntelCompiler" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Virtuality/Game2/Common/SLib", LTDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "slib - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /Od /I "..\PCCTS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Slib.lib"

!ELSEIF  "$(CFG)" == "slib - Win32 Debug"

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
# ADD CPP /nologo /MTd /w /W0 /Gm /GX /ZI /Od /I "..\PCCTS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"slib.lib"

!ELSEIF  "$(CFG)" == "slib - Win32 IntelCompiler"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "slib___Win32_IntelCompiler"
# PROP BASE Intermediate_Dir "slib___Win32_IntelCompiler"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "slib___Win32_IntelCompiler"
# PROP Intermediate_Dir "slib___Win32_IntelCompiler"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Od /I ".\PCCTS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Od /I ".\PCCTS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"slib.lib"

!ENDIF 

# Begin Target

# Name "slib - Win32 Release"
# Name "slib - Win32 Debug"
# Name "slib - Win32 IntelCompiler"
# Begin Group "DirectSound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dsdev.cpp
# End Source File
# Begin Source File

SOURCE=.\dsdev.h
# End Source File
# End Group
# Begin Group "Aureal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\a3ddev.cpp
# End Source File
# Begin Source File

SOURCE=.\a3ddev.h
# End Source File
# End Group
# Begin Group "EAX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\eaxdev.cpp
# End Source File
# Begin Source File

SOURCE=.\eaxdev.h
# End Source File
# End Group
# Begin Group "MPEG-3 decoder"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Getbits.cpp
# End Source File
# Begin Source File

SOURCE=.\Getdata.cpp
# End Source File
# Begin Source File

SOURCE=.\Huffman.cpp
# End Source File
# Begin Source File

SOURCE=.\iomp3.cpp
# End Source File
# Begin Source File

SOURCE=.\Layer3.cpp
# End Source File
# Begin Source File

SOURCE=.\Misc2.cpp
# End Source File
# Begin Source File

SOURCE=.\Proto.h
# End Source File
# Begin Source File

SOURCE=.\Structs.h
# End Source File
# Begin Source File

SOURCE=.\Transfrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Vary.cpp
# End Source File
# End Group
# Begin Group "Script"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sndcompiler.cpp
# End Source File
# Begin Source File

SOURCE=.\sndcompiler.h
# End Source File
# Begin Source File

SOURCE=.\SndLexer.cpp
# End Source File
# Begin Source File

SOURCE=.\SndLexer.h
# End Source File
# Begin Source File

SOURCE=.\SndParser.cpp
# End Source File
# Begin Source File

SOURCE=.\SndParser.h
# End Source File
# Begin Source File

SOURCE=.\sndrecorder.cpp
# End Source File
# Begin Source File

SOURCE=.\sndrecorder.h
# End Source File
# Begin Source File

SOURCE=.\sndscript.cpp
# End Source File
# Begin Source File

SOURCE=.\tokens.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\abnormal.cpp
# End Source File
# Begin Source File

SOURCE=.\abnormal.h
# End Source File
# Begin Source File

SOURCE=.\placebo.cpp
# End Source File
# Begin Source File

SOURCE=.\placebo.h
# End Source File
# Begin Source File

SOURCE=.\sndscript.g
# End Source File
# Begin Source File

SOURCE=.\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\sound.h
# End Source File
# End Target
# End Project
