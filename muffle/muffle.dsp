# Microsoft Developer Studio Project File - Name="muffle" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=muffle - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "muffle.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "muffle.mak" CFG="muffle - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "muffle - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "muffle - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Virtuality/Game2/muffle", OUOAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "muffle - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../temp/muffle/release"
# PROP Intermediate_Dir "../temp/muffle/release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "./include" /I "../shared/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"precomp.h" /FD /Qpchi- /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 antlr_release_static.lib kernel32.lib user32.lib ole32.lib uuid.lib dsound.lib dxguid.lib vorbisfile_static.lib ogg_static.lib vorbis_static.lib winmm.lib /nologo /dll /pdb:"../executable/muffle.pdb" /map /debug /machine:I386 /out:"../executable/muffle.dll" /implib:"../muffle.lib" /libpath:"../shared/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "muffle - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../temp/muffle/debug"
# PROP Intermediate_Dir "../temp/muffle/debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "./include" /I "../shared/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__MUFFLE_TEST__" /Yu"precomp.h" /FD /Qpchi- /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 antlr_debug_static.lib kernel32.lib user32.lib ole32.lib uuid.lib dsound.lib dxguid.lib vorbisfile_static.lib ogg_static.lib vorbis_static.lib winmm.lib /nologo /dll /pdb:"../executable/muffle_debug.pdb" /map /debug /machine:I386 /out:"../executable/muffle_debug.dll" /implib:"../muffle_debug.lib" /pdbtype:sept /libpath:"../shared/lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "muffle - Win32 Release"
# Name "muffle - Win32 Debug"
# Begin Group "mainten"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CallTrack.cpp
# End Source File
# Begin Source File

SOURCE=.\CallTrack.h
# End Source File
# Begin Source File

SOURCE=.\comptr.h
# End Source File
# Begin Source File

SOURCE=.\exception.h
# End Source File
# Begin Source File

SOURCE=.\guid.cpp
# End Source File
# Begin Source File

SOURCE=.\guid.h
# End Source File
# Begin Source File

SOURCE=.\new.h
# End Source File
# Begin Source File

SOURCE=.\precomp.cpp
# ADD CPP /Yc"precomp.h"
# End Source File
# Begin Source File

SOURCE=.\precomp.h
# End Source File
# Begin Source File

SOURCE=.\safety.cpp
# End Source File
# Begin Source File

SOURCE=.\safety.h
# End Source File
# End Group
# Begin Group "kernel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\caps.cpp
# End Source File
# Begin Source File

SOURCE=.\caps.h
# End Source File
# Begin Source File

SOURCE=.\dsound.cpp
# End Source File
# Begin Source File

SOURCE=.\dsound.h
# End Source File
# Begin Source File

SOURCE=.\format.h
# End Source File
# Begin Source File

SOURCE=.\kernel.cpp
# End Source File
# Begin Source File

SOURCE=.\kernel.h
# End Source File
# Begin Source File

SOURCE=.\Module.h
# End Source File
# Begin Source File

SOURCE=.\muffle.cpp
# End Source File
# Begin Source File

SOURCE=.\muffle.h
# End Source File
# Begin Source File

SOURCE=.\primarybuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\primarybuffer.h
# End Source File
# End Group
# Begin Group "emitter"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\emitter.cpp
# End Source File
# Begin Source File

SOURCE=.\emitter.h
# End Source File
# Begin Source File

SOURCE=.\PlayingSound.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayingSound.h
# End Source File
# End Group
# Begin Group "scripts"

# PROP Default_Filter ""
# Begin Group "parsing"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\grammar.g
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\SoundScriptLexer.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundScriptLexer.hpp
# End Source File
# Begin Source File

SOURCE=.\SoundScriptParser.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundScriptParser.hpp
# End Source File
# Begin Source File

SOURCE=.\SoundScriptParserTokenTypes.hpp
# End Source File
# Begin Source File

SOURCE=.\SoundScriptRecorder.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundScriptRecorder.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\script.cpp
# End Source File
# Begin Source File

SOURCE=.\script.h
# End Source File
# Begin Source File

SOURCE=.\scriptmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\scriptmgr.h
# End Source File
# End Group
# Begin Group "cache"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cache.h
# End Source File
# End Group
# Begin Group "volume"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\volume.cpp
# End Source File
# Begin Source File

SOURCE=.\volume.h
# End Source File
# Begin Source File

SOURCE=.\volumemgr.cpp
# End Source File
# Begin Source File

SOURCE=.\volumemgr.h
# End Source File
# End Group
# Begin Group "decoding"

# PROP Default_Filter ""
# Begin Group "wave"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wave_decoder.cpp
# End Source File
# Begin Source File

SOURCE=.\wave_decoder.h
# End Source File
# End Group
# Begin Group "ogg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ogg_decoder.cpp
# End Source File
# Begin Source File

SOURCE=.\ogg_decoder.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\DecodeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\DecodeMgr.h
# End Source File
# Begin Source File

SOURCE=.\decoder.h
# End Source File
# Begin Source File

SOURCE=.\DecodeStream.h
# End Source File
# Begin Source File

SOURCE=.\DecodeThread.cpp
# End Source File
# Begin Source File

SOURCE=.\DecodeThread.h
# End Source File
# Begin Source File

SOURCE=.\DynamicDecodeStream.cpp
# End Source File
# Begin Source File

SOURCE=.\DynamicDecodeStream.h
# End Source File
# Begin Source File

SOURCE=.\DynamicDecodeStreamQuery.cpp
# End Source File
# Begin Source File

SOURCE=.\DynamicDecodeStreamQuery.h
# End Source File
# Begin Source File

SOURCE=.\StaticDecodeStream.cpp
# End Source File
# Begin Source File

SOURCE=.\StaticDecodeStream.h
# End Source File
# End Group
# Begin Group "playback"

# PROP Default_Filter ""
# Begin Group "SoundBuffer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DynamicSoundBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\DynamicSoundBuffer.h
# End Source File
# Begin Source File

SOURCE=.\SoundBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundBuffer.h
# End Source File
# Begin Source File

SOURCE=.\SoundBufferLock.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundBufferLock.h
# End Source File
# Begin Source File

SOURCE=.\StaticSoundBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\StaticSoundBuffer.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Buffer.h
# End Source File
# End Group
# Begin Group "sync"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CriticalSection.cpp
# End Source File
# Begin Source File

SOURCE=.\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\Entry.cpp
# End Source File
# Begin Source File

SOURCE=.\Entry.h
# End Source File
# Begin Source File

SOURCE=.\WndHandle.h
# End Source File
# End Group
# Begin Group "notify"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\NotifyCluster.cpp
# End Source File
# Begin Source File

SOURCE=.\NotifyCluster.h
# End Source File
# Begin Source File

SOURCE=.\NotifyReceiver.h
# End Source File
# Begin Source File

SOURCE=.\NotifyThread.cpp
# End Source File
# Begin Source File

SOURCE=.\NotifyThread.h
# End Source File
# Begin Source File

SOURCE=.\SoundBufferNotify.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundBufferNotify.h
# End Source File
# End Group
# Begin Group "io"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\filemgmt.cpp
# End Source File
# Begin Source File

SOURCE=.\filemgmt.h
# End Source File
# Begin Source File

SOURCE=.\RealFile.cpp
# End Source File
# Begin Source File

SOURCE=.\RealFile.h
# End Source File
# Begin Source File

SOURCE=.\Stream.cpp
# End Source File
# Begin Source File

SOURCE=.\stream.h
# End Source File
# End Group
# Begin Group "themes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Fade.cpp
# End Source File
# Begin Source File

SOURCE=.\Fade.h
# End Source File
# Begin Source File

SOURCE=.\ThemeManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ThemeManager.h
# End Source File
# Begin Source File

SOURCE=.\ThemeSession.cpp
# End Source File
# Begin Source File

SOURCE=.\ThemeSession.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\include\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\include\ISound.h
# End Source File
# Begin Source File

SOURCE=.\include\quat.h
# End Source File
# Begin Source File

SOURCE=.\include\stackguard.cpp
# End Source File
# Begin Source File

SOURCE=.\include\VFS.h
# End Source File
# End Target
# End Project
