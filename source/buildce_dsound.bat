echo off
set path=%path%;"C:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\Bin"
mkdir build

evc hc40ce.vcp /MAKE "hc40ce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\hc40
copy GDI_DSOUND\hc40ce.exe build\hc40\.

evc hc80ce.vcp /MAKE "hc80ce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\hc80
copy GDI_DSOUND\hc80ce.exe build\hc80\.

evc m5ce.vcp /MAKE "m5ce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\m5
copy GDI_DSOUND\m5ce.exe build\m5\.

evc multi8ce.vcp /MAKE "multi8ce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\multi8
copy GDI_DSOUND\multi8ce.exe build\multi8\.

evc mz2500ce.vcp /MAKE "mz2500ce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\mz2500
copy GDI_DSOUND\mz2500ce.exe build\mz2500\.

evc mz2800ce.vcp /MAKE "mz2800ce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\mz2800
copy GDI_DSOUND\mz2800ce.exe build\mz2800\.

evc mz5500ce.vcp /MAKE "mz5500ce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\mz5500
copy GDI_DSOUND\mz5500ce.exe build\mz5500\.

evc pasopiace.vcp /MAKE "pasopiace - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\pasopia_oa
mkdir build\pasopia_t
copy GDI_DSOUND\pasopiace.exe build\pasopia_oa\.
copy GDI_DSOUND\pasopiace.exe build\pasopia_t\.

evc pasopia7ce.vcp /MAKE "pasopia7ce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
evc pasopia7lcdce.vcp /MAKE "pasopia7lcdce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\pasopia7
copy GDI_DSOUND\pasopia7ce.exe build\pasopia7\.
copy GDI_DSOUND\pasopia7lcdce.exe build\pasopia7\.

evc rx78ce.vcp /MAKE "rx78ce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\rx78
copy GDI_DSOUND\rx78ce.exe build\rx78\.

evc pv2000ce.vcp /MAKE "pv2000ce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\pv2000
copy GDI_DSOUND\pv2000ce.exe build\pv2000\.

evc pyutace.vcp /MAKE "pyutace - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\pyuta
copy GDI_DSOUND\pyutace.exe build\pyuta\.

evc qc10ce.vcp /MAKE "qc10ce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
evc qc10cmsce.vcp /MAKE "qc10cmsce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\qc10
copy GDI_DSOUND\qc10ce.exe build\qc10\.
copy GDI_DSOUND\qc10cmsce.exe build\qc10\.

evc scvce.vcp /MAKE "scvce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\scv
copy GDI_DSOUND\scvce.exe build\scv\.

evc x07ce.vcp /MAKE "x07ce - Win32 (WCE ARMV4I) GDI_DSOUND" /REBUILD /CECONFIG="STANDARDSDK"
mkdir build\x07
copy GDI_DSOUND\x07ce.exe build\x07\.

pause
del *.vcl
echo on
