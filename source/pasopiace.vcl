<html>
<body>
<pre>
<h1>�r���h�̃��O</h1>
<h3>
--------------------�\�� : pasopiace - Win32 (WCE ARMV4I) GDI_DSOUND--------------------
</h3>
<h3>�R�}���h ���C��</h3>
Creating command line "rc.exe /l 0x411 /fo"GDI_DSOUND/pasopia.res" /i "src\res" /d UNDER_CE=400 /d _WIN32_WCE=400 /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "WCE_PLATFORM_STANDARDSDK" /d "THUMB" /d "_THUMB_" /d "ARM" /d "_ARM_" /d "ARMV4I" /r "D:\Develop\Common\source\source\src\res\pasopia.rc"" 
�ꎞ�t�@�C�� "C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP181.tmp" ���쐬���A���̓��e���L�^���܂�
[
/nologo /W3 /Oxt /Ob2 /I ".\WinCE" /D _WIN32_WCE=400 /D "ARM" /D "_ARM_" /D "WCE_PLATFORM_STANDARDSDK" /D "ARMV4I" /D UNDER_CE=400 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_PASOPIA" /FR"GDI_DSOUND/" /Fo"GDI_DSOUND/" /QRarch4T /QRinterwork-return /MC /c 
"D:\Develop\Common\source\source\src\emu.cpp"
"D:\Develop\Common\source\source\src\win32_input.cpp"
"D:\Develop\Common\source\source\src\win32_screen.cpp"
"D:\Develop\Common\source\source\src\win32_sound.cpp"
"D:\Develop\Common\source\source\src\win32_timer.cpp"
"D:\Develop\Common\source\source\src\vm\beep.cpp"
"D:\Develop\Common\source\source\src\vm\datarec.cpp"
"D:\Develop\Common\source\source\src\vm\event.cpp"
"D:\Develop\Common\source\source\src\vm\hd46505.cpp"
"D:\Develop\Common\source\source\src\vm\i8255.cpp"
"D:\Develop\Common\source\source\src\vm\io8.cpp"
"D:\Develop\Common\source\source\src\vm\z80ctc.cpp"
"D:\Develop\Common\source\source\src\vm\z80pio.cpp"
"D:\Develop\Common\source\source\src\vm\pasopia\display.cpp"
"D:\Develop\Common\source\source\src\vm\pasopia\keyboard.cpp"
"D:\Develop\Common\source\source\src\vm\pasopia\memory.cpp"
"D:\Develop\Common\source\source\src\vm\pasopia\pac2.cpp"
"D:\Develop\Common\source\source\src\vm\pasopia\pasopia.cpp"
"D:\Develop\Common\source\source\src\vm\pasopia\rampac2.cpp"
"D:\Develop\Common\source\source\src\config.cpp"
"D:\Develop\Common\source\source\src\fileio.cpp"
"D:\Develop\Common\source\source\src\winmain.cpp"
]
Creating command line "clarm.exe @C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP181.tmp" 
�ꎞ�t�@�C�� "C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP182.tmp" ���쐬���A���̓��e���L�^���܂�
[
/nologo /W3 /Oxt /Ob0 /I ".\WinCE" /D _WIN32_WCE=400 /D "ARM" /D "_ARM_" /D "WCE_PLATFORM_STANDARDSDK" /D "ARMV4I" /D UNDER_CE=400 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_PASOPIA" /FR"GDI_DSOUND/" /Fo"GDI_DSOUND/" /QRarch4T /QRinterwork-return /MC /c 
"D:\Develop\Common\source\source\src\vm\z80.cpp"
]
Creating command line "clarm.exe @C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP182.tmp" 
�ꎞ�t�@�C�� "C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP183.tmp" ���쐬���A���̓��e���L�^���܂�
[
commctrl.lib coredll.lib WinCE\dsound.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:no /pdb:"GDI_DSOUND/pasopiace.pdb" /nodefaultlib:"libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib" /out:"GDI_DSOUND/pasopiace.exe" /subsystem:windowsce,4.00 /MACHINE:THUMB 
.\GDI_DSOUND\emu.obj
.\GDI_DSOUND\win32_input.obj
.\GDI_DSOUND\win32_screen.obj
.\GDI_DSOUND\win32_sound.obj
.\GDI_DSOUND\win32_timer.obj
.\GDI_DSOUND\beep.obj
.\GDI_DSOUND\datarec.obj
.\GDI_DSOUND\event.obj
.\GDI_DSOUND\hd46505.obj
.\GDI_DSOUND\i8255.obj
.\GDI_DSOUND\io8.obj
.\GDI_DSOUND\z80.obj
.\GDI_DSOUND\z80ctc.obj
.\GDI_DSOUND\z80pio.obj
.\GDI_DSOUND\display.obj
.\GDI_DSOUND\keyboard.obj
.\GDI_DSOUND\memory.obj
.\GDI_DSOUND\pac2.obj
.\GDI_DSOUND\pasopia.obj
.\GDI_DSOUND\rampac2.obj
.\GDI_DSOUND\config.obj
.\GDI_DSOUND\fileio.obj
.\GDI_DSOUND\winmain.obj
.\GDI_DSOUND\pasopia.res
]
�R�}���h ���C�� "link.exe @C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP183.tmp" �̍쐬��
<h3>�A�E�g�v�b�g �E�B���h�E</h3>
���\�[�X���R���p�C����...
�R���p�C����...
emu.cpp
win32_input.cpp
win32_screen.cpp
win32_sound.cpp
win32_timer.cpp
beep.cpp
datarec.cpp
event.cpp
hd46505.cpp
i8255.cpp
io8.cpp
z80ctc.cpp
z80pio.cpp
display.cpp
keyboard.cpp
memory.cpp
pac2.cpp
pasopia.cpp
rampac2.cpp
config.cpp
Generating Code...
Compiling...
fileio.cpp
winmain.cpp
Generating Code...
�R���p�C����...
z80.cpp
�����N��...





<h3>����</h3>
pasopiace.exe - �G���[ 0�A�x�� 0
</pre>
</body>
</html>
