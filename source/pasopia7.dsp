# Microsoft Developer Studio Project File - Name="pasopia7" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** �ҏW���Ȃ��ł������� **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=pasopia7 - Win32 Debug
!MESSAGE ����͗L����Ҳ�̧�قł͂���܂���B ������ۼު�Ă�����ނ��邽�߂ɂ� NMAKE ���g�p���Ă��������B
!MESSAGE [Ҳ�̧�ق̴���߰�] ����ނ��g�p���Ď��s���Ă�������
!MESSAGE 
!MESSAGE NMAKE /f "pasopia7.mak".
!MESSAGE 
!MESSAGE NMAKE �̎��s���ɍ\�����w��ł��܂�
!MESSAGE ����� ײݏ��ϸۂ̐ݒ���`���܂��B��:
!MESSAGE 
!MESSAGE NMAKE /f "pasopia7.mak" CFG="pasopia7 - Win32 Debug"
!MESSAGE 
!MESSAGE �I���\������� Ӱ��:
!MESSAGE 
!MESSAGE "pasopia7 - Win32 Release" ("Win32 (x86) Application" �p)
!MESSAGE "pasopia7 - Win32 Debug" ("Win32 (x86) Application" �p)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pasopia7 - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_PASOPIA7" /FR /FD /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib dsound.lib imm32.lib /nologo /subsystem:windows /map /machine:I386

!ELSEIF  "$(CFG)" == "pasopia7 - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_PASOPIA7" /FR /YX /FD /GZ /c
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

# Name "pasopia7 - Win32 Release"
# Name "pasopia7 - Win32 Debug"
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

SOURCE=.\src\vm\beep.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\datarec.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\disk.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\event.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\i8255.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\sn76489an.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\upd765a.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\z80.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\z80ctc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\z80pic.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\z80pio.cpp
# End Source File
# End Group
# Begin Group "VM Driver Source Files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\src\vm\pasopia7\floopy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\hd46505.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\io8.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\iotrap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\joypac2.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\kanjipac2.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\pac2.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\pasopia7.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\rampac2.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\timer.cpp
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

SOURCE=.\src\vm\beep.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\datarec.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\device.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\disk.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\event.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\fifo.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\i8255.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\sn76489an.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\upd765a.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\vm.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\z80.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\z80ctc.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\z80pic.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\z80pio.h
# End Source File
# End Group
# Begin Group "VM Driver Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\src\vm\pasopia7\floppy.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\hd46505.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\io8.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\iotrap.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\joypac2.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\kanjipac2.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\keyboard.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\memory.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\not.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\pac2.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\pac2dev.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\pasopia7.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\rampac2.h
# End Source File
# Begin Source File

SOURCE=.\src\vm\pasopia7\timer.h
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

SOURCE=.\src\res\PASOPIA7.ico
# End Source File
# Begin Source File

SOURCE=.\src\res\pasopia7.rc
# End Source File
# Begin Source File

SOURCE=.\src\res\resource.h
# End Source File
# End Group
# End Target
# End Project
