include ../config.mak

#all OS and lib independent
PLUGDIRS=aac_in bifs_dec codec_pack ctx_load dummy_in file_dnload m4_rend mp3_in mp4_io od_dec rtp_in raw_out render2d render3d timedtext img_in

#FT raster plugin
ifeq ($(CONFIG_FT), no)
else
PLUGDIRS+=ft_font
endif

#FFMPEG plugin
ifeq ($(CONFIG_FFMPEG), no)
else
PLUGDIRS+=ffmpeg_in
endif

#XVID dec
ifeq ($(CONFIG_XVID), no)
else
PLUGDIRS+=xvid_dec
endif

ifeq ($(CONFIG_OGG), no)
else
PLUGDIRS+=ogg
endif

#3GPP AMR-NB plugin
ifeq ($(CONFIG_AMR_NB), yes)
PLUGDIRS+=amr_dec
endif

#OSS audio out plugin
ifeq ($(CONFIG_LINUX_AUDIO_OSS), no)
else
PLUGDIRS+=oss_audio
endif

#SDL output
ifeq ($(CONFIG_SDL), yes)
PLUGDIRS+=SDL_out
endif

#SVG loader
ifeq ($(DISABLE_SVG), no)
ifeq ($(HAS_LIBXML2), yes)
PLUGDIRS+=svg_loader
endif
endif

#w32 plugins
ifeq ($(CONFIG_WIN32),yes)

#wave out
PLUGDIRS+=wav_audio

#direct X
ifeq ($(CONFIG_DIRECTX),yes)
PLUGDIRS+=dx_hw
endif

#w32 plugins
endif

all: plugs

plugs:	
	for i in $(PLUGDIRS) ; do $(MAKE) -C $$i all; done 

dep:
	for i in $(PLUGDIRS) ; do $(MAKE) -C $$i dep; done 

clean:
	for i in $(PLUGDIRS) ; do $(MAKE) -C $$i clean; done 

distclean:	
	for i in $(PLUGDIRS) ; do $(MAKE) -C $$i distclean; done 
