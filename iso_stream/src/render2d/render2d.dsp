# Microsoft Developer Studio Project File - Name="render2d" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=render2d - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "render2d.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "render2d.mak" CFG="render2d - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "render2d - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "render2d - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "render2d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Obj/Release"
# PROP Intermediate_Dir "Obj/Release"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
F90=df.exe
LINK32=link.exe -lib
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RENDER2D_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "RENDER2D_STATIC_BUILD" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "render2d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Obj/DebugVC6"
# PROP Intermediate_Dir "Obj/DebugVC6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
F90=df.exe
LINK32=link.exe -lib
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RENDER2D_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "RENDER2D_STATIC_BUILD" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "render2d - Win32 Release"
# Name "render2d - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "LASeR"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\LASeR\laser_animate.c
# End Source File
# Begin Source File

SOURCE=.\LASeR\laser_stacks.h
# End Source File
# Begin Source File

SOURCE=.\LASeR\render_laser_nodes.c
# End Source File
# End Group
# Begin Group "SVG"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SVG\render_svg_nodes.c
# End Source File
# Begin Source File

SOURCE=.\SVG\render_svg_text.c
# End Source File
# Begin Source File

SOURCE=.\SVG\svg_animation.c
# End Source File
# Begin Source File

SOURCE=.\SVG\svg_media.c
# End Source File
# Begin Source File

SOURCE=.\SVG\svg_stacks.h
# End Source File
# End Group
# Begin Group "SMIL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SMIL\smil_animation.c
# End Source File
# Begin Source File

SOURCE=.\SMIL\smil_stacks.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\background2d.c
# End Source File
# Begin Source File

SOURCE=.\copy_pixels.c
# End Source File
# Begin Source File

SOURCE=.\drawable.c
# End Source File
# Begin Source File

SOURCE=.\flashshape_proto.c
# End Source File
# Begin Source File

SOURCE=.\form.c
# End Source File
# Begin Source File

SOURCE=.\geometry_stacks.c
# End Source File
# Begin Source File

SOURCE=.\grouping.c
# End Source File
# Begin Source File

SOURCE=.\grouping_stacks.c
# End Source File
# Begin Source File

SOURCE=.\ifs2d.c
# End Source File
# Begin Source File

SOURCE=.\ils2d.c
# End Source File
# Begin Source File

SOURCE=.\layout.c
# End Source File
# Begin Source File

SOURCE=.\path_layout.c
# End Source File
# Begin Source File

SOURCE=.\render2d.c
# End Source File
# Begin Source File

SOURCE=.\render2d.def
# End Source File
# Begin Source File

SOURCE=.\render2d_nodes.c
# End Source File
# Begin Source File

SOURCE=.\sensor_stacks.c
# End Source File
# Begin Source File

SOURCE=.\sound.c
# End Source File
# Begin Source File

SOURCE=.\text.c
# End Source File
# Begin Source File

SOURCE=.\texture_stacks.c
# End Source File
# Begin Source File

SOURCE=.\viewport.c
# End Source File
# Begin Source File

SOURCE=.\visualsurface2d.c
# End Source File
# Begin Source File

SOURCE=.\visualsurface2d_draw.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\drawable.h
# End Source File
# Begin Source File

SOURCE=.\grouping.h
# End Source File
# Begin Source File

SOURCE=.\render2d.h
# End Source File
# Begin Source File

SOURCE=.\stacks2d.h
# End Source File
# Begin Source File

SOURCE=.\visualsurface2d.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
