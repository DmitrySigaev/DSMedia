# Microsoft Developer Studio Project File - Name="amr_dec" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=amr_dec - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "amr_dec.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "amr_dec.mak" CFG="amr_dec - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "amr_dec - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "amr_dec - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "amr_dec - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Obj/W32Rel"
# PROP Intermediate_Dir "Obj/W32Rel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "__MSDOS__" /D "MMS_IO" /D "amr_dec_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL"  /D "__MSDOS__" /D "MMS_IO" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /stack:0x800000 /dll /machine:I386 /out:"../../bin/Release/amr_dec.dll"
# SUBTRACT LINK32 /verbose /profile /incremental:yes /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "amr_dec - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Obj/W32Deb"
# PROP Intermediate_Dir "Obj/W32Deb"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL"  /D "__MSDOS__" /D "MMS_IO" /D "amr_dec_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "__MSDOS__" /D "MMS_IO" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /dll /debug /machine:I386 /out:"../../bin/Debug/amr_dec.dll"
# SUBTRACT LINK32 /verbose /profile /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "amr_dec - Win32 Release"
# Name "amr_dec - Win32 Debug"
# Begin Group "Plugin"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\amr_dec.c
# End Source File
# Begin Source File

SOURCE=.\amr_dec.def
# End Source File
# Begin Source File

SOURCE=.\amr_dec.h
# End Source File
# Begin Source File

SOURCE=.\amr_in.c
# End Source File
# End Group
# Begin Group "AMR_NB"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AMR_NB\a_refl.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\a_refl.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\agc.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\agc.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\autocorr.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\autocorr.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\az_lsp.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\az_lsp.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\b_cn_cod.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\b_cn_cod.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\basic_op.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\basicop2.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\bgnscd.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\bgnscd.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\bits2prm.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\bits2prm.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c1035pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c1035pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c2_11pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c2_11pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c2_9pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c2_9pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c3_14pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c3_14pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c4_17pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c4_17pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c8_31pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c8_31pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c_g_aver.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\c_g_aver.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\calc_cor.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\calc_cor.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\calc_en.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\calc_en.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\cbsearch.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\cbsearch.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\cl_ltp.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\cl_ltp.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\cnst.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\cnst_vad.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\cod_amr.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\cod_amr.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\convolve.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\convolve.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\copy.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\copy.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\cor_h.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\cor_h.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\count.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\count.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d1035pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d1035pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d2_11pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d2_11pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d2_9pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d2_9pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d3_14pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d3_14pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d4_17pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d4_17pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d8_31pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d8_31pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d_gain_c.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d_gain_c.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d_gain_p.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d_gain_p.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d_homing.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d_homing.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d_plsf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d_plsf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d_plsf_3.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\d_plsf_5.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\dec_amr.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\dec_amr.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\dec_gain.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\dec_gain.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\dec_lag3.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\dec_lag3.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\dec_lag6.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\dec_lag6.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\dtx_dec.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\dtx_dec.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\dtx_enc.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\dtx_enc.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\e_homing.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\e_homing.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\ec_gains.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\ec_gains.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\enc_lag3.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\enc_lag3.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\enc_lag6.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\enc_lag6.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\ex_ctrl.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\ex_ctrl.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\frame.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\g_adapt.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\g_adapt.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\g_code.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\g_code.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\g_pitch.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\g_pitch.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\gain_q.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\gain_q.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\gc_pred.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\gc_pred.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\gmed_n.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\gmed_n.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\hp_max.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\hp_max.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\int_lpc.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\int_lpc.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\int_lsf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\int_lsf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\inter_36.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\inter_36.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\inv_sqrt.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\inv_sqrt.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lag_wind.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lag_wind.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\levinson.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\levinson.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lflg_upd.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\log2.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\log2.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lpc.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lpc.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lsfwt.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lsfwt.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lsp.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lsp.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lsp_avg.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lsp_avg.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lsp_az.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lsp_az.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lsp_lsf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\lsp_lsf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\mac_32.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\mac_32.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\mode.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\n_proc.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\n_proc.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\ol_ltp.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\ol_ltp.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\oper_32b.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\oper_32b.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\p_ol_wgh.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\p_ol_wgh.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\ph_disp.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\ph_disp.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pitch_fr.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pitch_fr.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pitch_ol.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pitch_ol.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\post_pro.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\post_pro.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pow2.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pow2.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pre_big.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pre_big.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pre_proc.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pre_proc.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pred_lt.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pred_lt.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\preemph.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\preemph.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\prm2bits.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\prm2bits.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pstfilt.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\pstfilt.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\q_gain_c.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\q_gain_c.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\q_gain_p.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\q_gain_p.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\q_plsf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\q_plsf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\q_plsf_3.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\q_plsf_5.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\qgain475.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\qgain475.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\qgain795.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\qgain795.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\qua_gain.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\qua_gain.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\r_fft.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\reorder.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\reorder.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\residu.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\residu.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\s10_8pf.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\s10_8pf.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\set_sign.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\set_sign.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\set_zero.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\set_zero.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\sid_sync.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\sid_sync.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\sp_dec.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\sp_dec.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\sp_enc.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\sp_enc.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\spreproc.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\spreproc.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\spstproc.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\spstproc.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\sqrt_l.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\sqrt_l.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\strfunc.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\strfunc.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\syn_filt.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\syn_filt.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\ton_stab.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\ton_stab.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\typedef.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\typedefs.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\vad.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\vad1.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\vad1.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\vad2.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\vad2.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\vadname.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\vadname.h
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\weight_a.c
# End Source File
# Begin Source File

SOURCE=.\AMR_NB\weight_a.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\include\gpac\m4_decoder.h
# End Source File
# Begin Source File

SOURCE=..\..\include\gpac\m4_tools.h
# End Source File
# End Target
# End Project
