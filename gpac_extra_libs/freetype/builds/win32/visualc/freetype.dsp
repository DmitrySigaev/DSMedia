# Microsoft Developer Studio Project File - Name="freetype" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=FREETYPE - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "freetype.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "freetype.mak" CFG="FREETYPE - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "freetype - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "freetype - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "freetype - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\objs\release"
# PROP Intermediate_Dir "..\..\..\objs\release"
# PROP Target_Dir ""
LINK32=link.exe -lib
MTL=midl.exe
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /MD /Za /W4 /GX /Zi /O2 /I "..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /nologo /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\lib\w32_release\freetype.lib"

!ELSEIF  "$(CFG)" == "freetype - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\objs\debug"
# PROP Intermediate_Dir "..\..\..\objs\debug"
# PROP Target_Dir ""
LINK32=link.exe -lib
MTL=midl.exe
F90=df.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /MDd /Za /W4 /GX /Zi /Od /I "..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FT_DEBUG_LEVEL_ERROR" /D "FT_DEBUG_LEVEL_TRACE" /FD /GZ /c
# SUBTRACT CPP /nologo /X /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\lib\w32_debug\freetype.lib"

!ENDIF 

# Begin Target

# Name "freetype - Win32 Release"
# Name "freetype - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\autohint\autohint.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\bdf\bdf.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\cff\cff.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\base\ftbase.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\cache\ftcache.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\ftdebug.c
# ADD CPP /Ze
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\base\ftglyph.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gzip\ftgzip.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\base\ftinit.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\base\ftmm.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\base\ftsystem.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\pcf\pcf.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\pfr\pfr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\psaux\psaux.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\pshinter\pshinter.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\psnames\psmodule.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\raster\raster.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\sfnt\sfnt.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\smooth\smooth.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\truetype\truetype.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\type1\type1.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\cid\type1cid.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\type42\type42.c
# SUBTRACT CPP /Fr
# End Source File
# Begin Source File

SOURCE=..\..\..\src\winfonts\winfnt.c
# SUBTRACT CPP /Fr
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\include\ft2build.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\freetype\config\ftconfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\freetype\config\ftheader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\freetype\config\ftmodule.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\freetype\config\ftoption.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\freetype\config\ftstdlib.h
# End Source File
# End Group
# End Target
# End Project
