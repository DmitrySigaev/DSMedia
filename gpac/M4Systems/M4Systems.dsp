# Microsoft Developer Studio Project File - Name="M4Systems" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=M4Systems - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "M4Systems.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "M4Systems.mak" CFG="M4Systems - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "M4Systems - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "M4Systems - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "M4Systems - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Obj/W32Rel"
# PROP Intermediate_Dir "Obj/W32Rel"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "../Include" /I "./Tools" /I "./OD" /I "./IETF" /I "./MP4" /I "./SceneGraph" /I "./BIFS" /I "./ESM" /I "../extra_lib/include/js" /I "../extra_lib/include/zlib" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "M4_USE_SPIDERMONKEY" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "M4Systems - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Obj/W32Deb"
# PROP Intermediate_Dir "Obj/W32Deb"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../Include" /I "./Tools" /I "./OD" /I "./IETF" /I "./MP4" /I "./SceneGraph" /I "./BIFS" /I "./ESM" /I "../extra_lib/include/js" /I "../extra_lib/include/zlib" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "M4_USE_SPIDERMONKEY" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
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

# Name "M4Systems - Win32 Release"
# Name "M4Systems - Win32 Debug"
# Begin Group "Tools"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Tools\base_encoding.c
# End Source File
# Begin Source File

SOURCE=.\Tools\BitStream.c
# End Source File
# Begin Source File

SOURCE=.\Tools\Chain.c
# End Source File
# Begin Source File

SOURCE=.\Tools\error.c
# End Source File
# Begin Source File

SOURCE=.\Tools\IniFile.c
# End Source File
# Begin Source File

SOURCE=.\Tools\w32\os_divers.c
# End Source File
# Begin Source File

SOURCE=.\Tools\w32\os_net.c
# End Source File
# Begin Source File

SOURCE=.\Tools\w32\os_plug.c
# End Source File
# Begin Source File

SOURCE=.\Tools\w32\os_thread.c
# End Source File
# Begin Source File

SOURCE=.\Tools\plugin.c
# End Source File
# Begin Source File

SOURCE=.\Tools\plugin.h
# End Source File
# Begin Source File

SOURCE=.\Tools\StringParser.c
# End Source File
# Begin Source File

SOURCE=.\Tools\URL.c
# End Source File
# Begin Source File

SOURCE=.\Tools\UTF8.c
# End Source File
# End Group
# Begin Group "IETF"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\gpac\intern\m4_ietf_dev.h
# End Source File
# Begin Source File

SOURCE=.\IETF\RTCP.c
# End Source File
# Begin Source File

SOURCE=.\IETF\RTP.c
# End Source File
# Begin Source File

SOURCE=.\IETF\RTPReorder.c
# End Source File
# Begin Source File

SOURCE=.\IETF\RTSP_command.c
# End Source File
# Begin Source File

SOURCE=.\IETF\RTSP_common.c
# End Source File
# Begin Source File

SOURCE=.\IETF\RTSP_http.c
# End Source File
# Begin Source File

SOURCE=.\IETF\RTSP_response.c
# End Source File
# Begin Source File

SOURCE=.\IETF\RTSP_server.c
# End Source File
# Begin Source File

SOURCE=.\IETF\RTSP_session.c
# End Source File
# Begin Source File

SOURCE=.\IETF\SDP.c
# End Source File
# End Group
# Begin Group "OD"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\OD\comPrivate.c
# End Source File
# Begin Source File

SOURCE=.\OD\descPrivate.c
# End Source File
# Begin Source File

SOURCE=.\OD\descriptors.c
# End Source File
# Begin Source File

SOURCE=.\OD\ESDescriptor.c
# End Source File
# Begin Source File

SOURCE=.\OD\InitialObjectDescriptor.c
# End Source File
# Begin Source File

SOURCE=..\include\gpac\intern\m4_od_dev.h
# End Source File
# Begin Source File

SOURCE=.\OD\M4F_InitialObjectDescriptor.c
# End Source File
# Begin Source File

SOURCE=.\OD\M4F_ObjectDescriptor.c
# End Source File
# Begin Source File

SOURCE=.\OD\ObjectDescriptor.c
# End Source File
# Begin Source File

SOURCE=.\OD\OCICodec.c
# End Source File
# Begin Source File

SOURCE=.\OD\od_code.c
# End Source File
# Begin Source File

SOURCE=.\OD\OD_Dump.c
# End Source File
# Begin Source File

SOURCE=.\OD\OD_Parse.c
# End Source File
# Begin Source File

SOURCE=.\OD\ODCoDec.c
# End Source File
# Begin Source File

SOURCE=.\OD\ODCommand.c
# End Source File
# Begin Source File

SOURCE=.\OD\QoSDescriptor.c
# End Source File
# Begin Source File

SOURCE=.\OD\SLConfigDescriptor.c
# End Source File
# Begin Source File

SOURCE=.\OD\SyncLayer.c
# End Source File
# End Group
# Begin Group "MP4"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MP4\3GPP_atoms_code.c
# End Source File
# Begin Source File

SOURCE=.\MP4\Atom.c
# End Source File
# Begin Source File

SOURCE=.\MP4\AtomPriv.c
# End Source File
# Begin Source File

SOURCE=.\MP4\atoms_code.c
# End Source File
# Begin Source File

SOURCE=.\MP4\AVC.c
# End Source File
# Begin Source File

SOURCE=.\MP4\base_sample_desc.c
# End Source File
# Begin Source File

SOURCE=.\MP4\DTE.c
# End Source File
# Begin Source File

SOURCE=.\MP4\ghnt.c
# End Source File
# Begin Source File

SOURCE=.\MP4\HintPacket.c
# End Source File
# Begin Source File

SOURCE=.\MP4\HintSample.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_3GPP.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_dataMap.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_Dump.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_FileDataMap.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_FileMapping.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_Fragments.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_HintTrack.c
# End Source File
# Begin Source File

SOURCE=..\include\gpac\intern\m4_isomedia_dev.h
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_Media.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_Media_od.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_Movie_read.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_Movie_write.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_MovieIntern.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_packing.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_Publish.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_RTPHint.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_stbl_read.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_stbl_write.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_store.c
# End Source File
# Begin Source File

SOURCE=.\MP4\M4_Track.c
# End Source File
# Begin Source File

SOURCE=.\MP4\RTPPacket.c
# End Source File
# Begin Source File

SOURCE=.\MP4\TimedText.c
# End Source File
# End Group
# Begin Group "SceneGraph"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SceneGraph\base_scenegraph.c
# End Source File
# Begin Source File

SOURCE=..\include\gpac\intern\m4_scenegraph_dev.h
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\mpeg4_animators.c
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\mpeg4_commands.c
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\mpeg4_nodes.c
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\mpeg4_valuator.c
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\svg_nodes.c
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\svg_tools.c
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\vrml_interpolators.c
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\vrml_proto.c
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\vrml_route.c
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\vrml_script.c
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\vrml_SpiderMonkey.c
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\vrml_tools.c
# End Source File
# Begin Source File

SOURCE=.\SceneGraph\x3d_nodes.c
# End Source File
# End Group
# Begin Group "Authoring"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\authoring\avilib.c
# End Source File
# Begin Source File

SOURCE=..\include\gpac\intern\avilib.h
# End Source File
# Begin Source File

SOURCE=.\authoring\AVParsers.c
# End Source File
# Begin Source File

SOURCE=.\authoring\gpac_ogg.c
# End Source File
# Begin Source File

SOURCE=.\authoring\ISMA.c
# End Source File
# Begin Source File

SOURCE=.\authoring\LoadBT.c
# End Source File
# Begin Source File

SOURCE=.\authoring\LoadMP4.c
# End Source File
# Begin Source File

SOURCE=.\authoring\LoadXMTA.c
# End Source File
# Begin Source File

SOURCE=..\include\gpac\intern\m4_author_dev.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\intern\m4_swf_dev.h
# End Source File
# Begin Source File

SOURCE=.\authoring\M4TrackHinter.c
# End Source File
# Begin Source File

SOURCE=.\authoring\MediaExport.c
# End Source File
# Begin Source File

SOURCE=.\authoring\MediaImport.c
# End Source File
# Begin Source File

SOURCE=.\authoring\mpeg2_ps.c
# End Source File
# Begin Source File

SOURCE=.\authoring\mpeg2_ps.h
# End Source File
# Begin Source File

SOURCE=.\authoring\RTPPacketizer.c
# End Source File
# Begin Source File

SOURCE=.\authoring\RTPPck3GPP.c
# End Source File
# Begin Source File

SOURCE=.\authoring\RTPPckMPEG12.c
# End Source File
# Begin Source File

SOURCE=.\authoring\RTPPckMPEG4.c
# End Source File
# Begin Source File

SOURCE=.\authoring\SceneDump.c
# End Source File
# Begin Source File

SOURCE=.\authoring\SceneManager.c
# End Source File
# Begin Source File

SOURCE=.\authoring\SceneStats.c
# End Source File
# Begin Source File

SOURCE=.\authoring\SWFParse.c
# End Source File
# Begin Source File

SOURCE=.\authoring\SWFShape.c
# End Source File
# Begin Source File

SOURCE=.\authoring\TextImport.c
# End Source File
# Begin Source File

SOURCE=.\authoring\TrackTools.c
# End Source File
# Begin Source File

SOURCE=.\authoring\xml_parser.c
# End Source File
# End Group
# Begin Group "BIFS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BIFS\AADecoder.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\bifs_node_tables.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\BIFSComDec.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\BIFSComEnc.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\BIFSDec.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\BIFSEnc.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\BIFSMemDec.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\Conditional.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\FieldDec.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\FieldEnc.c
# End Source File
# Begin Source File

SOURCE=..\include\gpac\intern\m4_bifs_dev.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\intern\m4_bifs_tables.h
# End Source File
# Begin Source File

SOURCE=.\BIFS\PredictiveMFField.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\Quantize.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\Quantize.h
# End Source File
# Begin Source File

SOURCE=.\BIFS\SFScript.h
# End Source File
# Begin Source File

SOURCE=.\BIFS\SFScriptDec.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\SFScriptEnc.c
# End Source File
# Begin Source File

SOURCE=.\BIFS\Unquantize.c
# End Source File
# End Group
# Begin Group "ESM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ESM\channel.c
# End Source File
# Begin Source File

SOURCE=.\ESM\clock.c
# End Source File
# Begin Source File

SOURCE=.\ESM\decoder.c
# End Source File
# Begin Source File

SOURCE=.\ESM\InlineScene.c
# End Source File
# Begin Source File

SOURCE=.\ESM\InputSensor.c
# End Source File
# Begin Source File

SOURCE=.\ESM\InputSensor.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\intern\m4_esm_dev.h
# End Source File
# Begin Source File

SOURCE=.\ESM\M4NodeInit.c
# End Source File
# Begin Source File

SOURCE=.\ESM\M4Term.c
# End Source File
# Begin Source File

SOURCE=.\ESM\MediaControl.c
# End Source File
# Begin Source File

SOURCE=.\ESM\MediaControl.h
# End Source File
# Begin Source File

SOURCE=.\ESM\MediaManager.c
# End Source File
# Begin Source File

SOURCE=.\ESM\MediaMemory.c
# End Source File
# Begin Source File

SOURCE=.\ESM\MediaMemory.h
# End Source File
# Begin Source File

SOURCE=.\ESM\MediaObject.c
# End Source File
# Begin Source File

SOURCE=.\ESM\MediaSensor.c
# End Source File
# Begin Source File

SOURCE=.\ESM\NetService.c
# End Source File
# Begin Source File

SOURCE=.\ESM\ODBrowser.c
# End Source File
# Begin Source File

SOURCE=.\ESM\ODManager.c
# End Source File
# End Group
# Begin Group "Render"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\render\audio_input.c
# End Source File
# Begin Source File

SOURCE=.\render\audio_mixer.c
# End Source File
# Begin Source File

SOURCE=.\render\audio_render.c
# End Source File
# Begin Source File

SOURCE=.\render\audio_stacks.c
# End Source File
# Begin Source File

SOURCE=.\render\base_textures.c
# End Source File
# Begin Source File

SOURCE=.\render\common_stacks.c
# End Source File
# Begin Source File

SOURCE=.\render\common_stacks.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\intern\m4_render_dev.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\intern\m4_render_tools.h
# End Source File
# Begin Source File

SOURCE=.\render\path2D.c
# End Source File
# Begin Source File

SOURCE=.\render\path2D_outline.c
# End Source File
# Begin Source File

SOURCE=.\render\render_tools.c
# End Source File
# Begin Source File

SOURCE=.\render\renderer.c
# End Source File
# Begin Source File

SOURCE=.\render\texturing.c
# End Source File
# Begin Source File

SOURCE=.\render\yuv_to_rgb.c
# End Source File
# End Group
# Begin Group "LASeR"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\LASeR\laser_types.h
# End Source File
# Begin Source File

SOURCE=.\LASeR\LASeRCodec.c
# End Source File
# Begin Source File

SOURCE=.\LASeR\LASeRCodec.h
# End Source File
# Begin Source File

SOURCE=.\LASeR\LASERDec.c
# End Source File
# Begin Source File

SOURCE=.\LASeR\LASeRNodes.c
# End Source File
# Begin Source File

SOURCE=.\LASeR\LASeRParse.c
# End Source File
# Begin Source File

SOURCE=.\LASeR\LASeRSceneGraph.c
# End Source File
# Begin Source File

SOURCE=.\LASeR\m4_laser_dev.h
# End Source File
# Begin Source File

SOURCE=.\LASeR\xmldump.c
# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\gpac\m4_author.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_avhw.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_bifs.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_config.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_decoder.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_descriptors.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_graphics.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_ietf.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_isomedia.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_mpeg4_nodes.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_network.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_ogg.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_render.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_scenegraph.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_scenegraph_svg.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_scenegraph_vrml.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_svg_nodes.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_terminal.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_tools.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_user.h
# End Source File
# Begin Source File

SOURCE=..\include\gpac\m4_x3d_nodes.h
# End Source File
# End Group
# End Target
# End Project
