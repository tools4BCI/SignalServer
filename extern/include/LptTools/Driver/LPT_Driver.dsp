# Microsoft Developer Studio Project File - Name="LPT_Driver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=LPT_Driver - Win32 Release
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "LPT_Driver.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "LPT_Driver.mak" CFG="LPT_Driver - Win32 Release"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "LPT_Driver - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE "LPT_Driver - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "LPT_Driver - Win32 Debug"

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
# ADD CPP /nologo /Gz /ML /W3 /Zi /Oi /Ob2 /Gf /Gy /I "$(DDKROOT)\inc\ddk" /I "$(DDKROOT)\inc" /D "DEBUG" /D "_DEBUG" /D "RDRDBG" /D "SRVDBG" /D "DBG" /D "_X86_" /D "i386" /D "STD_CALL" /D "CONDITION_HANDLING" /D "WIN32_LEAN_AND_MEAN" /D "NT_UP" /D "_IDWBUILD" /D "IS_R0" /D "WINNT" /D "NDIS50" /D "WIN_NT_DRIVER" /D "WIN32_EXT" /D "WIN32" /U "NT_INST" /FR".\Debug/" /Fo".\Debug/" /Fd".\Debug/" /FD /Zel /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ntoskrnl.lib hal.lib /nologo /base:"0x10000" /entry:"DriverEntry@8" /incremental:no /map:".\Debug/LPT_Driver.map" /debug /machine:I386 /nodefaultlib /out:"Debug/LPT_Driver.sys" /libpath:"$(DDKROOT)\libchk\i386" /SUBSYSTEM:native /SUBSYSTEM:native
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "LPT_Driver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "LPT_Driver___Win32_Release"
# PROP BASE Intermediate_Dir "LPT_Driver___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /ML /W3 /Zi /Oi /Ob2 /Gf /Gy /I "C:\Projekte\CVS\ethereal\win32\winpcap\Common" /D "DEBUG" /D "_DEBUG" /D "RDRDBG" /D "SRVDBG" /D "DBG" /D "_X86_" /D "i386" /D "STD_CALL" /D "CONDITION_HANDLING" /D "WIN32_LEAN_AND_MEAN" /D "NT_UP" /D "_IDWBUILD" /D "IS_R0" /D "WINNT" /D "NDIS50" /D "WIN_NT_DRIVER" /D "WIN32_EXT" /D "WIN32" /U "NT_INST" /FR".\WNtDebug/" /Fo".\WNtDebug/" /Fd".\WNtDebug/" /FD /Zel /c
# ADD CPP /nologo /Gz /ML /W3 /Zi /Oi /Ob2 /Gf /Gy /I "$(DDKROOT)\inc\ddk" /I "$(DDKROOT)\inc" /D "_X86_" /D "i386" /D "STD_CALL" /D "CONDITION_HANDLING" /D "WIN32_LEAN_AND_MEAN" /D "NT_UP" /D "_IDWBUILD" /D "IS_R0" /D "WINNT" /D "NDIS50" /D "WIN_NT_DRIVER" /D "WIN32_EXT" /D "WIN32" /U "NT_INST" /FR".\Debug/" /Fo".\Debug/" /Fd".\Debug/" /FD /Zel /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ntoskrnl.lib hal.lib ndis.lib /nologo /base:"0x10000" /entry:"DriverEntry@8" /incremental:no /pdb:"WNtDebug/npf.pdb" /map:".\WNtDebug/npf.map" /debug /machine:I386 /nodefaultlib /out:"WNtDebug/ecatnpf.sys" /SUBSYSTEM:native /SUBSYSTEM:native
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 ntoskrnl.lib hal.lib /nologo /base:"0x10000" /entry:"DriverEntry@8" /incremental:no /map:".\Debug/LPT_Driver.map" /debug /machine:I386 /nodefaultlib /out:"Release/LPT_Driver.sys" /libpath:"$(DDKROOT)\libfre\i386" /SUBSYSTEM:native /SUBSYSTEM:native
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\LPT_Driver.sys %systemroot%\system32\drivers
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "LPT_Driver - Win32 Debug"
# Name "LPT_Driver - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\LPT_Driver.c
# End Source File
# Begin Source File

SOURCE=.\LPT_Driver.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\LPT_Driver.h
# End Source File
# Begin Source File

SOURCE=.\LPT_Driver_Ioctl.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
