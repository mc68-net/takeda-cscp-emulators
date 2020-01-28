# Microsoft Developer Studio Project File - Name="scv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=scv - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "scv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scv.mak" CFG="scv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "scv - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "scv - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "scv - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_SCV" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib dsound.lib imm32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "scv - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_SCV" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib dsound.lib imm32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "scv - Win32 Release"
# Name "scv - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "EMU Source Files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\src\emu.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32_input.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32_screen.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32_timer.cpp
# End Source File
# End Group
# Begin Group "VM Common Source Files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\src\vm\event.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\upd7801.cpp
# End Source File
# End Group
# Begin Group "VM Driver Source Files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\src\vm\scv\io.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\scv\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\scv\scv.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\scv\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\scv\vdp.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\config.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fileio.cpp
# End Source File
# Begin Source File

SOURCE=.\src\winmain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "EMU Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\src\emu.h
# End Source File
# End Group
# Begin Group "VM Common Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\src\vm\device.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\event.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\upd7801.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\vm.h
# End Source File
# End Group
# Begin Group "VM Driver Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\src\vm\scv\io.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\scv\memory.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\scv\scv.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\scv\sound.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\scv\sound_tbl.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\scv\vdp.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\common.h
# End Source File
# Begin Source File

SOURCE=.\src\config.h
# End Source File
# Begin Source File

SOURCE=.\src\fileio.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\src\res\resource.h
# End Source File
# Begin Source File

SOURCE=.\src\res\SCV.ico
# End Source File
# Begin Source File

SOURCE=.\src\res\scv.rc
# End Source File
# End Group
# End Target
# End Project
