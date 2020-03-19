# Microsoft Developer Studio Project File - Name="game2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=game2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Game2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Game2.mak" CFG="game2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "game2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "game2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""$/Virtuality/game2", WVCAAAAA"
# PROP Scc_LocalPath "."
CPP=d:\tools\clfilt.bat
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "game2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MT /GX /Zd /Ox /Og /Oi /Os /Oy- /Ob2 /I "common/pccts" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "D3D_OVERLOADS" /D "_STACK_GUARD_ON" /Yu"precomp.h" /FD /c
# SUBTRACT CPP /Ot /Oa /Ow
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib ddraw.lib d3dx.lib dinput.lib dxguid.lib shlwapi.lib logic2/logic2.lib common/pccts/pccts.lib scriptscene/scriptscene.lib muffle.lib /nologo /subsystem:windows /map /machine:I386 /nodefaultlib:"libc.lib" /out:"Executable/game.exe" /fixed:no
# SUBTRACT LINK32 /pdb:none /debug

!ELSEIF  "$(CFG)" == "game2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /w /W0 /Gm /Gi /GX /Zi /Od /I "common/pccts" /D "_MBCS" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "D3D_OVERLOADS" /D "_DEBUG_GRAPHICS" /D "_HOME_VERSION" /D "STACK_GUARD_ON" /D "NO_BLOOD" /FR /Yu"precomp.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dsound.lib ddraw.lib d3dx.lib dinput.lib dxguid.lib shlwapi.lib logic2/logic2_dbg.lib common/pccts/pccts_dbg.lib scriptscene/scriptscene_dbg.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ./muffle_debug.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libcmt.lib" /out:"Executable/game.exe" /pdbtype:sept /libpath:"common/pccts/"
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "game2 - Win32 Release"
# Name "game2 - Win32 Debug"
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Group "3d"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\3d\geometry.h
# End Source File
# Begin Source File

SOURCE=.\Common\3d\quat.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3d\quat.h
# End Source File
# End Group
# Begin Group "3DEffects"

# PROP Default_Filter ""
# Begin Group "weapons"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\3dEffects\LensFlareEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\LensFlareEffect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\LineEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\LineEffect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\ShootSmokeEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\ShootSmokeEffect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\SpangleEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\SpangleEffect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\SpotEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\SpotEffect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\TailEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\TailEffect.h
# End Source File
# End Group
# Begin Group "Info"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\3dEffects\EffectInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\EffectInfo.h
# End Source File
# End Group
# Begin Group "Base"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\3dEffects\Effect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\Effect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\EffectManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\EffectManager.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\ModelManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\ModelManager.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\particle.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\Particle.h
# End Source File
# End Group
# Begin Group "Constant"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\3dEffects\FireEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\FireEffect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\HaloEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\HaloEffect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\SmokeEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\SmokeEffect.h
# End Source File
# End Group
# Begin Group "Selection"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\3dEffects\SelectionEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\SelectionEffect.h
# End Source File
# End Group
# Begin Group "Lights"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\3dEffects\DLightEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\DLightEffect.h
# End Source File
# End Group
# Begin Group "Destructible"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\3dEffects\DestructEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\DestructEffect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\MeatEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\MeatEffect.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Common\3dEffects\BombEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\BombEffect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\DirectExplosion.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\DirectExplosion.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\ExplosionEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\ExplosionEffect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\GrenadeTracer.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\GrenadeTracer.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\ModelDestructEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\ModelDestructEffect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\ShieldEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\ShieldEffect.h
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\SparkleEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\3dEffects\SparkleEffect.h
# End Source File
# End Group
# Begin Group "D3DApp"

# PROP Default_Filter ""
# Begin Group "D3DInfo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\D3DApp\D3DInfo\D3DInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\D3DInfo\D3DInfo.h
# End Source File
# End Group
# Begin Group "D3DKernel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\D3DApp\D3DKernel\D3DKernel.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\D3DKernel\D3DKernel.h
# End Source File
# End Group
# Begin Group "GammaControl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\D3DApp\GammaControl\GammaControl.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\GammaControl\GammaControl.h
# End Source File
# End Group
# Begin Group "Input"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\D3DApp\Input\CharKeySupport.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\CharKeySupport.h
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\DirectInput.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\DirectInput.h
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\DirectInputDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\DirectInputDevice.h
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\Input.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\Input.h
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\Keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\Keyboard.h
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\Mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\Mouse.h
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\NhtThread.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\D3DApp\Input\NhtThread.h
# End Source File
# End Group
# End Group
# Begin Group "DataMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\DataMgr\DataMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\DataMgr\DataMgr.h
# End Source File
# Begin Source File

SOURCE=.\Common\DataMgr\TblFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\DataMgr\TblFile.h
# End Source File
# Begin Source File

SOURCE=.\Common\DataMgr\TxtFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\DataMgr\TxtFile.h
# End Source File
# Begin Source File

SOURCE=.\Common\DataMgr\ZipInfo\ZipInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\DataMgr\ZipInfo\ZipInfo.h
# End Source File
# Begin Source File

SOURCE=.\Common\DataMgr\ZipInfo\unzip.lib
# End Source File
# End Group
# Begin Group "DebugInfo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\DebugInfo\DebugInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\DebugInfo\DebugInfo.h
# End Source File
# End Group
# Begin Group "Exception"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\Exception\exception.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Exception\exception.h
# End Source File
# End Group
# Begin Group "FontMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\FontMgr\FastFont.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\FastFont.h
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\Font.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\Font.h
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\Font2D.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\Font2D.h
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\FontMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\FontMgr.h
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\FormattedText.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\FormattedText.h
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\GFont.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\GFont.h
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\LogicalText.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\LogicalText.h
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\mlf.h
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\mlff.h
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\VirtualTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\FontMgr\VirtualTexture.h
# End Source File
# End Group
# Begin Group "GraphPipe"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\GraphPipe\camera.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\GraphPipe\camera.h
# End Source File
# Begin Source File

SOURCE=.\Common\GraphPipe\culling.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\GraphPipe\culling.h
# End Source File
# Begin Source File

SOURCE=.\Common\GraphPipe\DynamicVB.h
# End Source File
# Begin Source File

SOURCE=.\Common\GraphPipe\graphpipe.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\GraphPipe\graphpipe.h
# End Source File
# Begin Source File

SOURCE=.\Common\GraphPipe\SimpleTexturedObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\GraphPipe\SimpleTexturedObject.h
# End Source File
# Begin Source File

SOURCE=.\Common\GraphPipe\VShader.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\GraphPipe\VShader.h
# End Source File
# End Group
# Begin Group "Log"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\Log\LConsole.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Log\LConsole.h
# End Source File
# Begin Source File

SOURCE=.\Common\Log\Log.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Log\Log.h
# End Source File
# End Group
# Begin Group "Shader"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\Shader\OutWrapper.h
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\shader.h
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\shd_grammar.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\ShdCompiler.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\ShdCompiler.h
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\ShdGenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\ShdGenerator.h
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\ShdLexer.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\ShdLexer.h
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\ShdParser.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\ShdParser.h
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\ShdRecorder.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\ShdRecorder.h
# End Source File
# Begin Source File

SOURCE=.\Common\Shader\tokens.h
# End Source File
# End Group
# Begin Group "Shell"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\Shell\CpuInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Shell\CpuInfo.h
# End Source File
# Begin Source File

SOURCE=.\Common\Shell\Shell.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Shell\Shell.h
# End Source File
# Begin Source File

SOURCE=.\Common\Shell\SystemInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Shell\SystemInfo.h
# End Source File
# End Group
# Begin Group "TextureMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\TextureMgr\D3DTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\TextureMgr\D3DTexture.h
# End Source File
# Begin Source File

SOURCE=.\Common\TextureMgr\DIBData.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\TextureMgr\DIBData.h
# End Source File
# Begin Source File

SOURCE=.\Common\TextureMgr\NhtBitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\TextureMgr\NhtBitmap.h
# End Source File
# Begin Source File

SOURCE=.\Common\TextureMgr\NhtTarga.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\TextureMgr\NhtTarga.h
# End Source File
# Begin Source File

SOURCE=.\Common\TextureMgr\TextureFormatMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\TextureMgr\TextureFormatMgr.h
# End Source File
# Begin Source File

SOURCE=.\Common\TextureMgr\TextureMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\TextureMgr\TextureMgr.h
# End Source File
# End Group
# Begin Group "Timer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\Timer\Timer.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Timer\Timer.h
# End Source File
# End Group
# Begin Group "Utils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\Utils\d3dutil.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\d3dutil.h
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\Dir.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\Dir.h
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\OptSlot.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\OptSlot.h
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\profiler.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\profiler.h
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\stackguard.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\stackguard.h
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\utils.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\utils.h
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\VFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Utils\VFile.h
# End Source File
# End Group
# Begin Group "SaveLoad"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\SaveLoad\SaveLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\SaveLoad\SaveLoad.h
# End Source File
# End Group
# Begin Group "CmdLine"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\CmdLine\CmdLine.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\CmdLine\CmdLine.h
# End Source File
# End Group
# Begin Group "Bink"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\Bink\BINK.H
# End Source File
# Begin Source File

SOURCE=.\Common\Bink\RAD.H
# End Source File
# Begin Source File

SOURCE=.\Common\Bink\binkw32.lib
# End Source File
# End Group
# Begin Group "BinkPlayer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\BinkPlayer\BinkPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\BinkPlayer\BinkPlayer.h
# End Source File
# End Group
# Begin Group "BinkMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\BinkMgr\BinkMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\BinkMgr\BinkMgr.h
# End Source File
# Begin Source File

SOURCE=.\Common\BinkMgr\BinkSurface.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\BinkMgr\BinkSurface.h
# End Source File
# End Group
# Begin Group "SurfaceMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\SurfaceMgr\DDSurface.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\SurfaceMgr\DDSurface.h
# End Source File
# Begin Source File

SOURCE=.\Common\SurfaceMgr\SurfaceMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\SurfaceMgr\SurfaceMgr.h
# End Source File
# End Group
# Begin Group "UI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\UI\Button.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\UI\Button.h
# End Source File
# Begin Source File

SOURCE=.\Common\Ui\Slide.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Ui\Slide.h
# End Source File
# Begin Source File

SOURCE=.\Common\UI\Static.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\UI\Static.h
# End Source File
# Begin Source File

SOURCE=.\Common\UI\VData.h
# End Source File
# Begin Source File

SOURCE=.\Common\UI\Widget.cpp

!IF  "$(CFG)" == "game2 - Win32 Release"

!ELSEIF  "$(CFG)" == "game2 - Win32 Debug"

# ADD CPP /FAcs

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Common\UI\Widget.h
# End Source File
# End Group
# Begin Group "glayer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\GLayer\GLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\GLayer\GLayer.h
# End Source File
# End Group
# Begin Group "Image"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\Image\Bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Image\Bitmap.h
# End Source File
# Begin Source File

SOURCE=.\Common\Image\ijl.h
# End Source File
# Begin Source File

SOURCE=.\Common\Image\Image.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Image\Image.h
# End Source File
# Begin Source File

SOURCE=.\Common\Image\ImageFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Image\ImageFactory.h
# End Source File
# Begin Source File

SOURCE=.\Common\Image\Jpeg.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Image\Jpeg.h
# End Source File
# Begin Source File

SOURCE=.\Common\Image\Targa.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\Image\Targa.h
# End Source File
# Begin Source File

SOURCE=.\Common\Image\ijl.lib
# End Source File
# End Group
# Begin Group "gsound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\gsound\FileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\gsound\FileSystem.h
# End Source File
# Begin Source File

SOURCE=.\Common\gsound\soundInit.cpp
# End Source File
# End Group
# Begin Group "PiracyControl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\PiracyControl\PiracyControl.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\PiracyControl\PiracyControl.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Common\precomp.cpp
# ADD CPP /Yc"precomp.h"
# End Source File
# Begin Source File

SOURCE=.\Common\precomp.h
# End Source File
# Begin Source File

SOURCE=.\Common\Tree.h
# End Source File
# End Group
# Begin Group "GameLevel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GameLevel\bsp.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLevel\bsp.h
# End Source File
# Begin Source File

SOURCE=.\GameLevel\chop.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLevel\chop.h
# End Source File
# Begin Source File

SOURCE=.\GameLevel\ExplosionManager.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLevel\ExplosionManager.h
# End Source File
# Begin Source File

SOURCE=.\GameLevel\GameLevel.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLevel\GameLevel.h
# End Source File
# Begin Source File

SOURCE=.\GameLevel\Grid.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLevel\Grid.h
# End Source File
# Begin Source File

SOURCE=.\GameLevel\LevelObjects.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLevel\LevelObjects.h
# End Source File
# Begin Source File

SOURCE=.\GameLevel\LevelToLogic.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLevel\LevelToLogic.h
# End Source File
# Begin Source File

SOURCE=.\GameLevel\LongShot.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLevel\LongShot.h
# End Source File
# Begin Source File

SOURCE=.\GameLevel\Mark.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLevel\Mark.h
# End Source File
# Begin Source File

SOURCE=.\GameLevel\objectsNG.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLevel\objectsNG.h
# End Source File
# Begin Source File

SOURCE=.\GameLevel\ScatteredItems.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLevel\ScatteredItems.h
# End Source File
# End Group
# Begin Group "Interface"

# PROP Default_Filter ""
# Begin Group "ELs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Interface\ELs\elText.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\ELs\elText.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Interface\Banner.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\Banner.h
# End Source File
# Begin Source File

SOURCE=.\Interface\BannerMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\BannerMgr.h
# End Source File
# Begin Source File

SOURCE=.\Interface\Console.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\Console.h
# End Source File
# Begin Source File

SOURCE=.\Interface\DescriptionScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\DescriptionScreen.h
# End Source File
# Begin Source File

SOURCE=.\Interface\Dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\Dialog.h
# End Source File
# Begin Source File

SOURCE=.\Interface\DialogScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\DialogScreen.h
# End Source File
# Begin Source File

SOURCE=.\Interface\DXImageMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\DXImageMgr.h
# End Source File
# Begin Source File

SOURCE=.\Interface\Edit.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\Edit.h
# End Source File
# Begin Source File

SOURCE=.\Interface\GameScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\GameScreen.h
# End Source File
# Begin Source File

SOURCE=.\Interface\GraphMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\GraphMgr.h
# End Source File
# Begin Source File

SOURCE=.\Interface\Interface.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\Interface.h
# End Source File
# Begin Source File

SOURCE=.\Interface\InventoryScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\InventoryScreen.h
# End Source File
# Begin Source File

SOURCE=.\Interface\JournalScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\JournalScreen.h
# End Source File
# Begin Source File

SOURCE=.\Interface\Lagoon.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\Lagoon.h
# End Source File
# Begin Source File

SOURCE=.\Interface\LoadingScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\LoadingScreen.h
# End Source File
# Begin Source File

SOURCE=.\Interface\MainMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\MainMenu.h
# End Source File
# Begin Source File

SOURCE=.\Interface\MenuMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\MenuMgr.h
# End Source File
# Begin Source File

SOURCE=.\Interface\MouseCursor.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\MouseCursor.h
# End Source File
# Begin Source File

SOURCE=.\Interface\MultiEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\MultiEdit.h
# End Source File
# Begin Source File

SOURCE=.\Interface\OptionsScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\OptionsScreen.h
# End Source File
# Begin Source File

SOURCE=.\Interface\Screens.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\Screens.h
# End Source File
# Begin Source File

SOURCE=.\Interface\ScreenShot.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\ScreenShot.h
# End Source File
# Begin Source File

SOURCE=.\Interface\SlotsEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\SlotsEngine.h
# End Source File
# Begin Source File

SOURCE=.\Interface\Text.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\Text.h
# End Source File
# Begin Source File

SOURCE=.\Interface\TextBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\TextBox.h
# End Source File
# Begin Source File

SOURCE=.\Interface\Tip.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\Tip.h
# End Source File
# Begin Source File

SOURCE=.\Interface\TradeScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\TradeScreen.h
# End Source File
# Begin Source File

SOURCE=.\Interface\WidgetFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\WidgetFactory.h
# End Source File
# Begin Source File

SOURCE=.\Interface\WidgetSoundMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Interface\WidgetSoundMgr.h
# End Source File
# End Group
# Begin Group "Options"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Options\Options.cpp
# End Source File
# Begin Source File

SOURCE=.\Options\Options.h
# End Source File
# Begin Source File

SOURCE=.\Options\PCOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\Options\PCOptions.h
# End Source File
# Begin Source File

SOURCE=.\Options\XRegistry.cpp
# End Source File
# Begin Source File

SOURCE=.\Options\XRegistry.h
# End Source File
# End Group
# Begin Group "Skin"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Skin\AnimaLibrary.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin\AnimaLibrary.h
# End Source File
# Begin Source File

SOURCE=.\Skin\BaseShadow.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin\Bone.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin\Bone.h
# End Source File
# Begin Source File

SOURCE=.\Skin\ComplexShadow.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin\iSlicer.h
# End Source File
# Begin Source File

SOURCE=.\Skin\KeyAnimation.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin\KeyAnimation.h
# End Source File
# Begin Source File

SOURCE=.\Skin\Person.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin\Person.h
# End Source File
# Begin Source File

SOURCE=.\Skin\Shadow.h
# End Source File
# Begin Source File

SOURCE=.\Skin\ShadowBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin\ShadowUtility.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin\SimpleShadow.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin\skanim.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin\skanim.h
# End Source File
# Begin Source File

SOURCE=.\Skin\skeleton.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin\skeleton.h
# End Source File
# Begin Source File

SOURCE=.\Skin\Skin.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin\Skin.h
# End Source File
# Begin Source File

SOURCE=.\Skin\slicer.h
# End Source File
# End Group
# Begin Group "other"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Game.cpp
# End Source File
# Begin Source File

SOURCE=.\Game.h
# End Source File
# Begin Source File

SOURCE=.\globals.cpp

!IF  "$(CFG)" == "game2 - Win32 Release"

!ELSEIF  "$(CFG)" == "game2 - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\MemoryDump.cpp
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\resource.rc

!IF  "$(CFG)" == "game2 - Win32 Release"

!ELSEIF  "$(CFG)" == "game2 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WinMain.cpp
# End Source File
# End Group
# Begin Group "DDFrame"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DDFrame\DDFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\DDFrame\DDFrame.h
# End Source File
# Begin Source File

SOURCE=.\DDFrame\DDMainWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\DDFrame\DDMainWnd.h
# End Source File
# End Group
# Begin Group "sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sound\ISound.h
# End Source File
# Begin Source File

SOURCE=.\sound\VFS.h
# End Source File
# End Group
# Begin Group "SEcuROM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\securom\securom.cpp
# End Source File
# Begin Source File

SOURCE=.\securom\securom.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\bugs.txt
# End Source File
# Begin Source File

SOURCE=.\character.cpp
# End Source File
# Begin Source File

SOURCE=.\character.h
# End Source File
# Begin Source File

SOURCE=.\defines.txt
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\IWorld.cpp
# End Source File
# Begin Source File

SOURCE=.\IWorld.h
# End Source File
# Begin Source File

SOURCE=.\ModMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ModMgr.h
# End Source File
# Begin Source File

SOURCE=.\SaveVersion.cpp
# End Source File
# Begin Source File

SOURCE=.\todo.txt
# End Source File
# Begin Source File

SOURCE=.\uninst.ico
# End Source File
# Begin Source File

SOURCE=.\version.cpp
# End Source File
# Begin Source File

SOURCE=.\World.cpp
# End Source File
# Begin Source File

SOURCE=.\World.h
# End Source File
# End Target
# End Project
