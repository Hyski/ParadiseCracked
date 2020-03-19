# Microsoft Developer Studio Project File - Name="logic2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=logic2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "logic2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "logic2.mak" CFG="logic2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "logic2 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "logic2 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "logic2 - Win32 Out release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Virtuality/Game2/logic2", MOFAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "logic2 - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_STACK_GUARD_ON" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"logic2.lib"

!ELSEIF  "$(CFG)" == "logic2 - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_HOME_VERSION" /FR /Yu"logicdefs.h" /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"logic2_dbg.lib"

!ELSEIF  "$(CFG)" == "logic2 - Win32 Out release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "logic2___Win32_Out_release"
# PROP BASE Intermediate_Dir "logic2___Win32_Out_release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "STACK_GUARD_ON" /D "HOME_VERSION" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "STACK_GUARD_ON" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"logic2.lib"
# ADD LIB32 /nologo /out:"logic2.lib"

!ENDIF 

# Begin Target

# Name "logic2 - Win32 Release"
# Name "logic2 - Win32 Debug"
# Name "logic2 - Win32 Out release"
# Begin Group "Players"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Player.cpp
# End Source File
# Begin Source File

SOURCE=.\Player.h
# End Source File
# End Group
# Begin Group "Entities"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AIContext.h
# End Source File
# Begin Source File

SOURCE=.\entity.cpp
# End Source File
# Begin Source File

SOURCE=.\entity.h
# End Source File
# Begin Source File

SOURCE=.\entityaux.h
# End Source File
# Begin Source File

SOURCE=.\EntityFactory.h
# End Source File
# Begin Source File

SOURCE=.\Spawn.cpp
# End Source File
# Begin Source File

SOURCE=.\Spawn.h
# End Source File
# Begin Source File

SOURCE=.\Strategy.cpp
# End Source File
# Begin Source File

SOURCE=.\Strategy.h
# End Source File
# End Group
# Begin Group "Menus"

# PROP Default_Filter ""
# Begin Group "Game"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GameForm.cpp
# End Source File
# Begin Source File

SOURCE=.\GameFormImp.h
# End Source File
# End Group
# Begin Group "Inventory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\InventoryFormImp3.cpp
# End Source File
# Begin Source File

SOURCE=.\InventoryFormImp3.h
# End Source File
# End Group
# Begin Group "Shop"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ShopFormImp.cpp
# End Source File
# Begin Source File

SOURCE=.\ShopFormImp.h
# End Source File
# End Group
# Begin Group "Journal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\JournalFormImp3.cpp
# End Source File
# Begin Source File

SOURCE=.\JournalFormImp3.h
# End Source File
# End Group
# Begin Group "Drag&Drop"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DragDropManager.cpp
# End Source File
# Begin Source File

SOURCE=.\DragDropManager.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\form.cpp
# End Source File
# Begin Source File

SOURCE=.\Form.h
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter ""
# Begin Group "VisUtils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\VisUtils3.cpp
# End Source File
# Begin Source File

SOURCE=.\VisUtils3.h
# End Source File
# End Group
# Begin Group "SndUtils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SndUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\SndUtils.h
# End Source File
# End Group
# Begin Group "HexUtils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\HexGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\HexGrid.h
# End Source File
# Begin Source File

SOURCE=.\hexutils.cpp

!IF  "$(CFG)" == "logic2 - Win32 Release"

!ELSEIF  "$(CFG)" == "logic2 - Win32 Debug"

# ADD CPP /Yc"logicdefs.h"

!ELSEIF  "$(CFG)" == "logic2 - Win32 Out release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hexutils.h
# End Source File
# End Group
# Begin Group "Graph"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GraphHexGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\GraphHexGrid.h
# End Source File
# Begin Source File

SOURCE=.\Graphic.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphic.h
# End Source File
# End Group
# Begin Group "Quests"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PhraseManager.h
# End Source File
# Begin Source File

SOURCE=.\questengine.cpp
# End Source File
# Begin Source File

SOURCE=.\questengine.h
# End Source File
# Begin Source File

SOURCE=.\questengine_loc.h
# End Source File
# Begin Source File

SOURCE=.\QuestServer.cpp
# End Source File
# Begin Source File

SOURCE=.\QuestServer.h
# End Source File
# Begin Source File

SOURCE=.\questsparser.cpp
# End Source File
# End Group
# Begin Group "Actions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Activity.cpp
# End Source File
# Begin Source File

SOURCE=.\Activity.h
# End Source File
# Begin Source File

SOURCE=.\SScene.h
# End Source File
# End Group
# Begin Group "EventPipe"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GameObserver.h
# End Source File
# End Group
# Begin Group "Cheat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CheatParser.cpp
# End Source File
# Begin Source File

SOURCE=.\CheatParser.h
# End Source File
# End Group
# Begin Group "Test"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\profiler.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Damager.h
# End Source File
# Begin Source File

SOURCE=.\DirtyLinks.cpp
# End Source File
# Begin Source File

SOURCE=.\DirtyLinks.h
# End Source File
# Begin Source File

SOURCE=.\DynUtils.h
# End Source File
# Begin Source File

SOURCE=.\InfoUtils.h
# End Source File
# Begin Source File

SOURCE=.\Observer.h
# End Source File
# Begin Source File

SOURCE=.\PathUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\PathUtils.h
# End Source File
# Begin Source File

SOURCE=.\TraceUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\TraceUtils.h
# End Source File
# Begin Source File

SOURCE=.\XlsReader.h
# End Source File
# End Group
# Begin Group "AI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AIAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\AIAPI.h
# End Source File
# Begin Source File

SOURCE=.\AIBase.cpp
# End Source File
# Begin Source File

SOURCE=.\AIBase.h
# End Source File
# Begin Source File

SOURCE=.\AIUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\AIUtils.h
# End Source File
# Begin Source File

SOURCE=.\Cameraman.h
# End Source File
# Begin Source File

SOURCE=.\CivilianAI.cpp
# End Source File
# Begin Source File

SOURCE=.\CivilianAI.h
# End Source File
# Begin Source File

SOURCE=.\EnemyAI.cpp
# End Source File
# Begin Source File

SOURCE=.\EnemyAI.h
# End Source File
# Begin Source File

SOURCE=.\EnemyDetection.cpp
# End Source File
# Begin Source File

SOURCE=.\EnemyDetection.h
# End Source File
# Begin Source File

SOURCE=.\PanicBehaviour.cpp
# End Source File
# Begin Source File

SOURCE=.\PanicBehaviour.h
# End Source File
# End Group
# Begin Group "Things"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Bureau.h
# End Source File
# Begin Source File

SOURCE=.\Thing.cpp
# End Source File
# Begin Source File

SOURCE=.\Thing.h
# End Source File
# Begin Source File

SOURCE=.\ThingFactory.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\GameLogic.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLogic.h
# End Source File
# Begin Source File

SOURCE=.\logicdefs.h
# End Source File
# End Target
# End Project
