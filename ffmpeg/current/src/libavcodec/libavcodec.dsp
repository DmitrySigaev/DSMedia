# Microsoft Developer Studio Project File - Name="libavcodec" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libavcodec - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libavcodec.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libavcodec.mak" CFG="libavcodec - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libavcodec - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libavcodec - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../build/vc6.0/Release/libavcodec"
# PROP Intermediate_Dir "../../build/vc6.0/Release/libavcodec"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\libavcodec" /I "..\libavcodec\libac3 ..\libav" /I "..\." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_AV_CONFIG_H" /D "EMULATE_FAST_INT" /FR /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../build/vc6.0/Debug/libavcodec"
# PROP Intermediate_Dir "../../build/vc6.0/Debug/libavcodec"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\libavcodec" /I "..\libavcodec\libac3 ..\libav" /I "..\." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_AV_CONFIG_H" /D "EMULATE_FAST_INT" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../build/vc6.0/Debug/libavcodec\avcodec.lib"

!ENDIF 

# Begin Target

# Name "libavcodec - Win32 Release"
# Name "libavcodec - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "liba52"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\liba52\bit_allocate.c
# End Source File
# Begin Source File

SOURCE=.\liba52\bitstream.c
# End Source File
# Begin Source File

SOURCE=.\liba52\downmix.c
# End Source File
# Begin Source File

SOURCE=.\liba52\imdct.c
# End Source File
# Begin Source File

SOURCE=.\liba52\parse.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\4xm.c
# End Source File
# Begin Source File

SOURCE=.\8bps.c
# End Source File
# Begin Source File

SOURCE=.\a52dec.c
# End Source File
# Begin Source File

SOURCE=.\ac3enc.c
# End Source File
# Begin Source File

SOURCE=.\adpcm.c
# End Source File
# Begin Source File

SOURCE=.\adx.c
# End Source File
# Begin Source File

SOURCE=.\allcodecs.c
# End Source File
# Begin Source File

SOURCE=.\asv1.c
# End Source File
# Begin Source File

SOURCE=.\cabac.c
# End Source File
# Begin Source File

SOURCE=.\cinepak.c
# End Source File
# Begin Source File

SOURCE=.\cljr.c
# End Source File
# Begin Source File

SOURCE=.\common.c
# End Source File
# Begin Source File

SOURCE=.\cyuv.c
# End Source File
# Begin Source File

SOURCE=.\dpcm.c
# End Source File
# Begin Source File

SOURCE=.\dsputil.c
# End Source File
# Begin Source File

SOURCE=.\dv.c
# End Source File
# Begin Source File

SOURCE=.\error_resilience.c
# End Source File
# Begin Source File

SOURCE=.\eval.c
# End Source File
# Begin Source File

SOURCE=.\faandct.c
# End Source File
# Begin Source File

SOURCE=.\fft.c
# End Source File
# Begin Source File

SOURCE=.\ffv1.c
# End Source File
# Begin Source File

SOURCE=.\flac.c
# End Source File
# Begin Source File

SOURCE=.\flicvideo.c
# End Source File
# Begin Source File

SOURCE=.\g726.c
# End Source File
# Begin Source File

SOURCE=.\golomb.c
# End Source File
# Begin Source File

SOURCE=.\h261.c
# End Source File
# Begin Source File

SOURCE=.\h263.c
# End Source File
# Begin Source File

SOURCE=.\h263dec.c
# End Source File
# Begin Source File

SOURCE=.\h264.c
# End Source File
# Begin Source File

SOURCE=.\huffyuv.c
# End Source File
# Begin Source File

SOURCE=.\idcinvideo.c
# End Source File
# Begin Source File

SOURCE=.\imgconvert.c
# End Source File
# Begin Source File

SOURCE=.\imgresample.c
# End Source File
# Begin Source File

SOURCE=.\integer.c
# End Source File
# Begin Source File

SOURCE=.\interplayvideo.c
# End Source File
# Begin Source File

SOURCE=.\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=.\jfdctint.c
# End Source File
# Begin Source File

SOURCE=.\jrevdct.c
# End Source File
# Begin Source File

SOURCE=.\lcl.c
# End Source File
# Begin Source File

SOURCE=.\mace.c
# End Source File
# Begin Source File

SOURCE=.\mdct.c
# End Source File
# Begin Source File

SOURCE=.\mem.c
# End Source File
# Begin Source File

SOURCE=.\mjpeg.c
# End Source File
# Begin Source File

SOURCE=.\motion_est.c
# End Source File
# Begin Source File

SOURCE=.\mpeg12.c
# End Source File
# Begin Source File

SOURCE=.\mpegaudio.c
# End Source File
# Begin Source File

SOURCE=.\mpegaudiodec.c
# End Source File
# Begin Source File

SOURCE=.\mpegvideo.c
# End Source File
# Begin Source File

SOURCE=.\msmpeg4.c
# End Source File
# Begin Source File

SOURCE=.\msrle.c
# End Source File
# Begin Source File

SOURCE=.\msvideo1.c
# End Source File
# Begin Source File

SOURCE=.\opts.c
# End Source File
# Begin Source File

SOURCE=.\parser.c
# End Source File
# Begin Source File

SOURCE=.\pcm.c
# End Source File
# Begin Source File

SOURCE=.\qtrle.c
# End Source File
# Begin Source File

SOURCE=.\ra144.c
# End Source File
# Begin Source File

SOURCE=.\ra288.c
# End Source File
# Begin Source File

SOURCE=.\ratecontrol.c
# End Source File
# Begin Source File

SOURCE=.\rational.c
# End Source File
# Begin Source File

SOURCE=.\raw.c
# End Source File
# Begin Source File

SOURCE=.\resample.c
# End Source File
# Begin Source File

SOURCE=.\resample2.c
# End Source File
# Begin Source File

SOURCE=.\roqvideo.c
# End Source File
# Begin Source File

SOURCE=.\rpza.c
# End Source File
# Begin Source File

SOURCE=.\rv10.c
# End Source File
# Begin Source File

SOURCE=.\simple_idct.c
# End Source File
# Begin Source File

SOURCE=.\smc.c
# End Source File
# Begin Source File

SOURCE=.\truemotion1.c
# End Source File
# Begin Source File

SOURCE=.\utils.c
# End Source File
# Begin Source File

SOURCE=.\vcr1.c
# End Source File
# Begin Source File

SOURCE=.\vmdav.c
# End Source File
# Begin Source File

SOURCE=.\vp3.c
# End Source File
# Begin Source File

SOURCE=.\vp3dsp.c
# End Source File
# Begin Source File

SOURCE=.\vqavideo.c
# End Source File
# Begin Source File

SOURCE=.\xan.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
