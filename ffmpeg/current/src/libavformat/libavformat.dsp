# Microsoft Developer Studio Project File - Name="libavformat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libavformat - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libavformat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libavformat.mak" CFG="libavformat - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libavformat - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libavformat - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libavformat - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../build/vc6.0/Release/libavformat"
# PROP Intermediate_Dir "../../build/vc6.0/Release/libavformat"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../libavcodec" /I "..\libavcodec\libac3" /I ".." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_AV_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libavformat - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../build/vc6.0/Debug/libavformat"
# PROP Intermediate_Dir "../../build/vc6.0/Debug/libavformat"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../libavcodec" /I "..\libavcodec\libac3" /I ".." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_AV_CONFIG_H" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /out:"../../build/vc6.0/Debug/libavformat\avformat.lib"
# SUBTRACT LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libavformat - Win32 Release"
# Name "libavformat - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\4xm.c
# End Source File
# Begin Source File

SOURCE=.\allformats.c
# End Source File
# Begin Source File

SOURCE=".\asf-enc.c"
# End Source File
# Begin Source File

SOURCE=.\asf.c
# End Source File
# Begin Source File

SOURCE=.\au.c
# End Source File
# Begin Source File

SOURCE=.\avidec.c
# End Source File
# Begin Source File

SOURCE=.\avienc.c
# End Source File
# Begin Source File

SOURCE=.\avio.c
# End Source File
# Begin Source File

SOURCE=.\aviobuf.c
# End Source File
# Begin Source File

SOURCE=.\crc.c
# End Source File
# Begin Source File

SOURCE=.\cutils.c
# End Source File
# Begin Source File

SOURCE=.\dv.c
# End Source File
# Begin Source File

SOURCE=.\ffm.c
# End Source File
# Begin Source File

SOURCE=.\file.c
# End Source File
# Begin Source File

SOURCE=.\flic.c
# End Source File
# Begin Source File

SOURCE=.\flvdec.c
# End Source File
# Begin Source File

SOURCE=.\flvenc.c
# End Source File
# Begin Source File

SOURCE=.\framehook.c
# End Source File
# Begin Source File

SOURCE=.\gif.c
# End Source File
# Begin Source File

SOURCE=.\gifdec.c
# End Source File
# Begin Source File

SOURCE=.\idcin.c
# End Source File
# Begin Source File

SOURCE=.\idroq.c
# End Source File
# Begin Source File

SOURCE=.\img.c
# End Source File
# Begin Source File

SOURCE=.\ipmovie.c
# End Source File
# Begin Source File

SOURCE=.\jpeg.c
# End Source File
# Begin Source File

SOURCE=.\matroska.c
# End Source File
# Begin Source File

SOURCE=.\mov.c
# End Source File
# Begin Source File

SOURCE=.\movenc.c
# End Source File
# Begin Source File

SOURCE=.\mp3.c
# End Source File
# Begin Source File

SOURCE=.\mpeg.c
# End Source File
# Begin Source File

SOURCE=.\mpegts.c
# End Source File
# Begin Source File

SOURCE=.\mpegtsenc.c
# End Source File
# Begin Source File

SOURCE=.\mpjpeg.c
# End Source File
# Begin Source File

SOURCE=.\nut.c
# End Source File
# Begin Source File

SOURCE=.\os_support.c
# End Source File
# Begin Source File

SOURCE=.\pnm.c
# End Source File
# Begin Source File

SOURCE=.\psxstr.c
# End Source File
# Begin Source File

SOURCE=.\raw.c
# End Source File
# Begin Source File

SOURCE=.\rm.c
# End Source File
# Begin Source File

SOURCE=.\segafilm.c
# End Source File
# Begin Source File

SOURCE=.\sgi.c
# End Source File
# Begin Source File

SOURCE=.\sierravmd.c
# End Source File
# Begin Source File

SOURCE=.\swf.c
# End Source File
# Begin Source File

SOURCE=.\utils.c
# End Source File
# Begin Source File

SOURCE=.\wav.c
# End Source File
# Begin Source File

SOURCE=.\wc3movie.c
# End Source File
# Begin Source File

SOURCE=.\westwood.c
# End Source File
# Begin Source File

SOURCE=.\yuv.c
# End Source File
# Begin Source File

SOURCE=.\yuv4mpeg.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\avformat.h
# End Source File
# End Group
# End Target
# End Project
