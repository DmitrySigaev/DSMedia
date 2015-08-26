# Microsoft Developer Studio Project File - Name="xvid_dec" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=xvid_dec - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xvid_dec.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xvid_dec.mak" CFG="xvid_dec - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xvid_dec - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "xvid_dec - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xvid_dec - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Obj/Release"
# PROP Intermediate_Dir "Obj/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "XVID_DEC_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../include" /I "../../extra_lib/include/xvid" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 libxvidcore.lib /nologo /dll /machine:I386 /out:"../../bin/Release/xvid_dec.dll" /libpath:"../../extra_lib/lib/w32_release"

!ELSEIF  "$(CFG)" == "xvid_dec - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Obj/Debug"
# PROP Intermediate_Dir "Obj/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "XVID_DEC_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../extra_lib/include/xvid" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /dll /debug /machine:I386 /out:"../../bin/Debug/xvid_dec.dll" /pdbtype:sept /libpath:"../../extra_lib/lib/w32_debug"

!ENDIF 

# Begin Target

# Name "xvid_dec - Win32 Release"
# Name "xvid_dec - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "WCE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xvid_wce\bitstream.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\bitstream.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\CodecAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\decoder.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\decoder.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\font.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\global.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\gmc.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\gmc.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\idct.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\image.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\image.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\interpolate8x8.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\interpolate8x8.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\mbcoding.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\mbprediction.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\mbprediction.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\mem_align.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\mem_align.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\mem_transfer.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\mem_transfer.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\portab.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\qpel.inl
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\qpel_tab.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\quant.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\quant_h263.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\quant_matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\quant_matrix.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\quant_mpeg.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\reduced.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\reduced.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\Rules.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\vlc_codes.h
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\xvid.cpp
# End Source File
# Begin Source File

SOURCE=.\xvid_wce\xvid.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\xvid_dec.def
# End Source File
# Begin Source File

SOURCE=.\xvid_dec_wce.cpp
# End Source File
# End Group
# End Target
# End Project
