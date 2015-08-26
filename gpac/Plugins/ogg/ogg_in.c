/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004 
 *					All rights reserved
 *
 *  This file is part of GPAC / XIPH.org plugin
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *		
 */

#include "ogg_in.h"
#include <ogg/ogg.h>

#include <gpac/m4_author.h>

typedef struct 
{
	u32 streamType, oti;	/*MPEG-4 streamType*/
	u32 num_init_headers;
	u32 sample_rate, bitrate;

	u32 theora_kgs;
	Float frame_rate;
	u32 frame_rate_base;
} OGGInfo;

typedef struct
{
	ogg_stream_state os;
	u32 serial_no;
	/*DSI for ogg streams is simply the BOS page*/
	char *dsi;
	u32 dsi_len;

	OGGInfo info;
	Bool need_second_header, headers_sent;
	s64 seek_granule, last_granule;

	Bool is_running;
	u32 force_headers;
	LPNETCHANNEL ch;
	u16 ESID;
	Bool eos_detected, map_time;
} OGGStream;

typedef struct
{
	LPNETSERVICE service;
	M4Thread *demuxer;

	Chain *streams;

	FILE *ogfile;
	u32 file_size;
	Bool is_remote;
	u32 nb_playing, kill_demux, do_seek, service_type;

	/*ogg ogfile state*/
    ogg_sync_state oy;
	
	Bool needs_od;
	OGGStream *resync_stream;


	Bool has_video, has_audio, bos_done, is_single_media;

	Double dur;
	u32 data_buffer_ms;

	LPNETCHANNEL od_ch;

	Bool needs_connection;
	Double start_range, end_range;
	/*file downloader*/
	LPFILEDOWNLOADER dnload;
	Bool is_live;
} OGGReader;


void OGG_EndOfFile(OGGReader *read)
{
	u32 i;
	NM_OnSLPRecieved(read->service, read->od_ch, NULL, 0, NULL, M4EOF);
	for (i=0; i<ChainGetCount(read->streams); i++) {
		OGGStream *st = ChainGetEntry(read->streams, i);
		NM_OnSLPRecieved(read->service, st->ch, NULL, 0, NULL, M4EOF);
	}
}

#define OGG_BUFFER_SIZE 4096

static Bool OGG_ReadPage(OGGReader *read, ogg_page *oggpage)
{
	char buf[OGG_BUFFER_SIZE];
	M4Err e;

	/*remote file, check if we use cache*/
	if (read->is_remote) {
		u32 total_size, status;
		e = NM_GetDownloaderStats(read->dnload, &total_size, NULL, NULL, &status);
		/*not ready*/
		if ((e<M4OK) || (status != DL_Running)) return 0;
		if (!total_size) {
			read->is_live = 1;
		}
		if (!read->is_live  && !read->ogfile) {
			const char *szCache = NM_GetCacheFileName(read->dnload);
			if (!szCache) return 0;
			read->ogfile = fopen((char *) szCache, "rb");
			if (!read->ogfile) return 0;
		}
	}

    while (ogg_sync_pageout(&read->oy, oggpage ) != 1 ) {
        char *buffer;
		u32 bytes;
		
		if (read->ogfile) {
			if (feof(read->ogfile)) {
				OGG_EndOfFile(read);
				return 0;
			}
			bytes = fread(buf, 1, OGG_BUFFER_SIZE, read->ogfile);
		} else {
			e = NM_FetchData(read->dnload, buf, OGG_BUFFER_SIZE, &bytes);
			if (e) return 0;
		}
		if (!bytes) return 0;

		buffer = ogg_sync_buffer(&read->oy, bytes);
		memcpy(buffer, buf, bytes);
        ogg_sync_wrote(&read->oy, bytes);
    }
    return 1;
}

static OGGStream *OGG_FindStreamForPage(OGGReader *read, ogg_page *oggpage)
{
	u32 i, count;
	count = ChainGetCount(read->streams);
	for (i=0; i<count; i++) {
		OGGStream *st = ChainGetEntry(read->streams, i);
        if (ogg_stream_pagein(&st->os, oggpage) == 0) return st;
	}
	return NULL;
}



static ObjectDescriptor *OGG_GetOD(OGGStream *st, Bool no_ocr)
{
	ObjectDescriptor *od;
	ESDescriptor *esd;

	od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
	od->objectDescriptorID = (st->info.streamType==M4ST_AUDIO) ? ISMA_AUDIO_OD_ID : ISMA_VIDEO_OD_ID;
	esd = OD_NewESDescriptor(0);
	esd->decoderConfig->streamType = st->info.streamType;
	esd->decoderConfig->objectTypeIndication = st->info.oti;
	esd->decoderConfig->avgBitrate = st->info.bitrate;
	esd->ESID = st->ESID;
	esd->OCRESID = no_ocr ? 0 : 1;
	/*we can't rely on granulepo to get something decent, so we let the term handle ts compute*/
	esd->slConfig->predefined = SLPredef_SkipSL;
	
	esd->decoderConfig->decoderSpecificInfo->dataLength = st->dsi_len;
	SAFEALLOC(esd->decoderConfig->decoderSpecificInfo->data, st->dsi_len);
	memcpy(esd->decoderConfig->decoderSpecificInfo->data, st->dsi, sizeof(char) * st->dsi_len);
	ChainAddEntry(od->ESDescriptors, esd);
	return od;
}

static void OGG_SendStreams(OGGReader *read)
{
	LPODCODEC codec;
	char *data;
	u32 data_len, i;
	SLHeader slh;
	ObjectDescriptorUpdate *odU;
	ObjectDescriptor *od;

	if (!read->needs_od) return;
	read->needs_od = 0;

	codec = OD_NewCodec(OD_WRITE);
	odU = (ObjectDescriptorUpdate *) OD_NewCommand(ODUpdate_Tag);

	/*this will NOT work properly for multi track files other than 1 video, 1 audio*/
	for (i=0; i<ChainGetCount(read->streams); i++) {
		OGGStream *st = ChainGetEntry(read->streams, i);
		od = OGG_GetOD(st, 0);
		ChainAddEntry(odU->objectDescriptors, od);
	}
	OD_AddCommand(codec, (ODCommand *)odU);
	OD_EncodeAU(codec);
	data = NULL;
	data_len = 0;
	OD_GetEncodedAU(codec, &data, &data_len);
	OD_DeleteCodec(codec);
	memset(&slh, 0, sizeof(SLHeader));
	slh.accessUnitEndFlag = slh.accessUnitStartFlag = 1;
	slh.compositionTimeStampFlag = 1;
	slh.compositionTimeStamp = (u64) (read->start_range * 1000);
	slh.randomAccessPointFlag = 1;
	NM_OnSLPRecieved(read->service, read->od_ch, data, data_len, &slh, M4OK);
	free(data);
}

u64 OGG_GranuleToTime(OGGInfo *cfg, s64 granule)
{
	if (cfg->sample_rate) {
		return granule;
	}
	if (cfg->frame_rate) {
		s64 iframe = granule>>cfg->theora_kgs;
		s64 pframe = granule - (iframe<<cfg->theora_kgs);
		pframe += iframe;
		pframe *= cfg->frame_rate_base;
		return (u64) (pframe / cfg->frame_rate);
	}
	return 0;
}

Double OGG_GranuleToMediaTime(OGGInfo *cfg, s64 granule)
{
	Double t = (Double) (s64) OGG_GranuleToTime(cfg, granule);
	if (cfg->sample_rate) t /= cfg->sample_rate;
	else t /= cfg->frame_rate_base;
	return t;
}


static Bool OGG_GetStreamInfo(ogg_packet *oggpacket, OGGInfo *info) 
{
	oggpack_buffer opb;

	memset(info, 0, sizeof(OGGInfo));

	/*vorbis*/
	if ((oggpacket->bytes >= 7) && !strncmp(&oggpacket->packet[1], "vorbis", 6)) {
		info->streamType = M4ST_AUDIO;
		info->oti = GPAC_OGG_MEDIA_OTI;
		oggpack_readinit(&opb, oggpacket->packet, oggpacket->bytes);
		oggpack_adv( &opb, 88);
		oggpack_adv( &opb, 8);	/*nb chan*/
		info->sample_rate = oggpack_read(&opb, 32);
		oggpack_adv( &opb, 32);	/*max rate*/
		info->bitrate = oggpack_read(&opb, 32);
		info->num_init_headers = 3;
	}
	/*speex*/
	else if ((oggpacket->bytes >= 7) && !strncmp(&oggpacket->packet[0], "Speex", 5)) {
		info->streamType = M4ST_AUDIO;
		info->oti = GPAC_OGG_MEDIA_OTI;
		oggpack_readinit(&opb, oggpacket->packet, oggpacket->bytes);
		oggpack_adv(&opb, 224);
		oggpack_adv(&opb, 32);
		oggpack_adv( &opb, 32);
		info->sample_rate = oggpack_read(&opb, 32);
	}
	/*flac*/
	else if ((oggpacket->bytes >= 4) && !strncmp(&oggpacket->packet[0], "fLaC", 4)) {
		info->streamType = M4ST_AUDIO;
		info->oti = GPAC_OGG_MEDIA_OTI;
		/*we must wait for seconf header to get info*/
		return 1;
	}
	/*theora*/
	else if ((oggpacket->bytes >= 7) && !strncmp(&oggpacket->packet[1], "theora", 6)) {
		BitStream *bs;
		u32 fps_numerator, fps_denominator, keyframe_freq_force;

		info->streamType = M4ST_VISUAL;
		info->oti = GPAC_OGG_MEDIA_OTI;
		bs = NewBitStream(oggpacket->packet, oggpacket->bytes, BS_READ);
		BS_ReadInt(bs, 56);
		BS_ReadInt(bs, 8); /* major version num */
		BS_ReadInt(bs, 8); /* minor version num */
		BS_ReadInt(bs, 8); /* subminor version num */
		BS_ReadInt(bs, 16) /*<< 4*/; /* width */
		BS_ReadInt(bs, 16) /*<< 4*/; /* height */
		BS_ReadInt(bs, 24); /* frame width */
		BS_ReadInt(bs, 24); /* frame height */
		BS_ReadInt(bs, 8); /* x offset */
		BS_ReadInt(bs, 8); /* y offset */
		fps_numerator = BS_ReadInt(bs, 32);
		fps_denominator = BS_ReadInt(bs, 32);
		BS_ReadInt(bs, 24); /* aspect_numerator */
		BS_ReadInt(bs, 24); /* aspect_denominator */
		BS_ReadInt(bs, 8); /* colorspace */
		BS_ReadInt(bs, 24);/* bitrate */
		BS_ReadInt(bs, 6); /* quality */

		keyframe_freq_force = 1 << BS_ReadInt(bs, 5);
		info->theora_kgs = 0;
		keyframe_freq_force--;
		while (keyframe_freq_force) {
			info->theora_kgs ++;
			keyframe_freq_force >>= 1;
		}
	    info->frame_rate = ((Float)fps_numerator) / fps_denominator;
		info->num_init_headers = 3;
		DeleteBitStream(bs);
		info->frame_rate_base = fps_denominator;
	}
	/*other not supported yet*/
	return 0;
}

static void OGG_ResetupStream(OGGReader *read, OGGStream *st, ogg_page *oggpage)
{
	ogg_stream_clear(&st->os);
	ogg_stream_init(&st->os, st->serial_no);
	ogg_stream_pagein(&st->os, oggpage);
	st->force_headers = st->info.num_init_headers;
	
	if (st->info.sample_rate) {
		st->seek_granule = (s64) (read->start_range * st->info.sample_rate);
	} else if (st->info.frame_rate) {
		s64 seek = (s64) (read->start_range * st->info.frame_rate) - 1;
		if (seek<0) seek=0;
		st->seek_granule = (seek)<<st->info.theora_kgs;
	}
	st->last_granule = -1;
}

static void OGG_NewStream(OGGReader *read, ogg_page *oggpage)
{
	ogg_packet oggpacket;
	u32 serial_no, i;
	OGGStream *st;

	/*reannounce of stream (caroussel in live streams) - until now I don't think icecast uses this*/
	serial_no = ogg_page_serialno(oggpage);
	for (i=0; i<ChainGetCount(read->streams); i++) {
		st = ChainGetEntry(read->streams, i);
		if (st->serial_no==serial_no) {
			OGG_ResetupStream(read, st, oggpage);
			return;
		}
	}

	/*look if we have the same stream defined (eg, reuse first stream dead with same header page)*/
	for (i=0; i<ChainGetCount(read->streams); i++) {
		st = ChainGetEntry(read->streams, i);
		if (st->eos_detected) {
			ogg_stream_state os;
			ogg_stream_init(&os, serial_no);
			ogg_stream_pagein(&os, oggpage);
			ogg_stream_packetpeek(&os, &oggpacket);
			if (st->dsi && !memcmp(st->dsi, oggpacket.packet, oggpacket.bytes)) {
				ogg_stream_clear(&os);
				st->serial_no = serial_no;
				OGG_ResetupStream(read, st, oggpage);
				return;
			}
			ogg_stream_clear(&os);
			/*nope streams are different, signal eos on this one*/
			NM_OnSLPRecieved(read->service, st->ch, NULL, 0, NULL, M4EOF);
		}
	}

	SAFEALLOC(st, sizeof(OGGStream));
    st->serial_no = serial_no;
    ogg_stream_init(&st->os, st->serial_no);
	ogg_stream_pagein(&st->os, oggpage);
	
	ogg_stream_packetpeek(&st->os, &oggpacket);
	st->need_second_header = OGG_GetStreamInfo(&oggpacket, &st->info);

	/*check we don't discard audio or visual streams*/
	if ( ((read->service_type==1) && (st->info.streamType==M4ST_AUDIO))
		|| ((read->service_type==2) && (st->info.streamType==M4ST_VISUAL)) )
	{
	    ogg_stream_clear(&st->os);
		free(st);
		return;
	}

	/*store bos page for dsi*/
	st->dsi_len = oggpacket.bytes;
	SAFEALLOC(st->dsi, sizeof(char)*st->dsi_len);
	memcpy(st->dsi, oggpacket.packet, sizeof(char)*st->dsi_len);

	ChainAddEntry(read->streams, st);
	st->ESID = 2 + ChainGetCount(read->streams);
	st->force_headers = st->info.num_init_headers;
	
	if (st->info.sample_rate) {
		st->seek_granule = (s64) (read->start_range * st->info.sample_rate);
	} else if (st->info.frame_rate) {
		s64 seek = (s64) (read->start_range * st->info.frame_rate) - 1;
		if (seek<0) seek=0;
		st->seek_granule = (seek)<<st->info.theora_kgs;
	}
	st->last_granule = -1;

	if (st->info.streamType==M4ST_VISUAL) {
		read->has_video = 1;
	} else {
		read->has_audio = 1;
	}
}

void OGG_SignalEndOfStream(OGGReader *read, OGGStream *st)
{
	if (st->eos_detected) {
		NM_OnSLPRecieved(read->service, st->ch, NULL, 0, NULL, M4EOF);
		ogg_stream_clear(&st->os);
	}
}


void OGG_Process(OGGReader *read)
{
	OGGStream *st;
    ogg_packet oggpacket;
	ogg_page oggpage;

	if (read->resync_stream) {
		st = read->resync_stream;
		read->resync_stream = NULL;
		goto process_stream;
	}

	if (!OGG_ReadPage(read, &oggpage)) return;

	if (ogg_page_bos(&oggpage)) {
		OGG_NewStream(read, &oggpage);
		return;
	}

	st = OGG_FindStreamForPage(read, &oggpage);
	if (!st) return;

	if (ogg_page_eos(&oggpage)) 
		st->eos_detected = 1;

	if (st->need_second_header) {
		/*process update & send*/
		return;
	}
	/*from here we should have passed all headers*/
	if (!read->bos_done) {
		read->bos_done = 1;
		/*live case, we must resync otherwise we rewind the file*/
		if (!read->ogfile) read->resync_stream = st;
		return;
	}

process_stream:
	/*live insertion (not supported yet, just a reminder)*/
	if (!st->ch) {
		read->resync_stream = st;
		return;
	}

	while (ogg_stream_packetout(&st->os, &oggpacket ) > 0 ) {
		if (oggpacket.granulepos != -1) {
			st->last_granule = oggpacket.granulepos;
		}

		if (st->force_headers) {
			if (!st->headers_sent) NM_OnSLPRecieved(read->service, st->ch, oggpacket.packet, oggpacket.bytes, NULL, M4OK);
			st->force_headers--;
			if (!st->force_headers) st->headers_sent = 1;
		}
		else if (st->map_time) {
			Double t;
			if (read->start_range && (oggpacket.granulepos==-1)) continue;
			t = OGG_GranuleToMediaTime(&st->info, st->last_granule);
			if (t>=read->start_range) {
				NetworkCommand map;
				map.command_type = CHAN_MAP_TIME;
				map.map_time.on_channel = st->ch;
				map.map_time.reset_buffers = (read->start_range>0.2) ? 1 : 0;
				map.map_time.timestamp = 0;
				map.map_time.media_time = t;
				NM_OnCommand(read->service, &map, M4OK);
				st->map_time = 0;
				NM_OnSLPRecieved(read->service, st->ch, oggpacket.packet, oggpacket.bytes, NULL, M4OK);
			}
		}
		else {
			NM_OnSLPRecieved(read->service, st->ch, oggpacket.packet, oggpacket.bytes, NULL, M4OK);
		}
	}
}

static u32 OggDemux(void *par)
{
	NetworkCommand com;
	Bool go;
	u32 i;
	OGGReader *read = (OGGReader *) par;
	
	read->bos_done = 0;
	memset(&com, 0, sizeof(NetworkCommand));
	com.command_type = CHAN_BUFFER_QUERY;

    ogg_sync_init(&read->oy);

	while (!read->kill_demux) {
		OGG_Process(read);

		if (!read->bos_done) continue;

		if (read->bos_done==1) {
			read->bos_done=2;
			NM_OnConnect(read->service, NULL, M4OK);
			/*wait till we have an OD stream if we need one*/
			if (!read->is_single_media) {
				while (!read->kill_demux && !read->od_ch) Sleep(20);
				if (read->kill_demux) break;
			}
		}
		
		/*idle*/
		while (!read->kill_demux && !read->nb_playing) {
			/*send OD updates*/
			OGG_SendStreams(read);
			Sleep(20);
		}

		/*(re)starting, seek*/
		if (read->do_seek) {
			read->do_seek = 0;
			ogg_sync_clear(&read->oy);
			ogg_sync_init(&read->oy);
			OGG_SendStreams(read);

			if (read->ogfile) {
				u32 seek_to = 0;
				read->resync_stream = NULL;
				if (read->dur) seek_to = (u32) (read->file_size * (read->start_range/read->dur) * 0.6f);
				if ((s32) seek_to > ftell(read->ogfile) ) {
					fseek(read->ogfile, seek_to, SEEK_SET);
				} else {
					fseek(read->ogfile, 0, SEEK_SET);
				}
			}
		}


		/*sleep untill the buffer occupancy is too low - note that this work because all streams in this
		demuxer are synchronized*/
		go = read->nb_playing;
		while (go && !read->kill_demux) {
			for (i=0; i<ChainGetCount(read->streams); i++) {
				OGGStream *st = ChainGetEntry(read->streams, i);
				if (!st->ch) continue;
				com.base.on_channel = st->ch;
				NM_OnCommand(read->service, &com, M4OK);
				if (com.buffer.occupancy < read->data_buffer_ms) {
					//fprintf(stdout, "OGG: channel %d needs fill (%d ms data, %d max buffer)\n", st->ESID, com.buffer.occupancy, read->data_buffer_ms);
					go = 0;
					break;
				}
			}
			if (!i || !read->nb_playing) break;
			Sleep(10);
		}
	}
    ogg_sync_clear(&read->oy);
	read->kill_demux=2;
	return 0;
}

/*get streams & duration*/
Bool OGG_CheckFile(OGGReader *read)
{
	OGGInfo info, the_info;
	ogg_page oggpage;
	ogg_packet oggpacket;
	ogg_stream_state os, the_os;
	u64 max_gran;
	Bool has_stream = 0;
	fseek(read->ogfile, 0, SEEK_SET);

    ogg_sync_init(&read->oy);
	max_gran = 0;
	while (1) {
		if (!OGG_ReadPage(read, &oggpage)) break;

		if (ogg_page_bos(&oggpage)) {
			ogg_stream_init(&os, ogg_page_serialno(&oggpage));
			if (ogg_stream_pagein(&os, &oggpage) >= 0 ) {
				ogg_stream_packetpeek(&os, &oggpacket);
				if (ogg_stream_pagein(&os, &oggpage) >= 0 ) {
					ogg_stream_packetpeek(&os, &oggpacket);
					OGG_GetStreamInfo(&oggpacket, &info); 
				}
				if (!has_stream) {
					has_stream = 1;
					ogg_stream_init(&the_os, ogg_page_serialno(&oggpage));
					the_info = info;
				}
			}
			ogg_stream_clear(&os);
			continue;
		}
		if (has_stream && (ogg_stream_pagein(&the_os, &oggpage) >= 0) ) {
			while (ogg_stream_packetout(&the_os, &oggpacket ) > 0 ) {
				if ((oggpacket.granulepos>=0) && ((u64) oggpacket.granulepos>max_gran) ) {
					max_gran = oggpacket.granulepos;
				}
			}
		}
	}
    ogg_sync_clear(&read->oy);
	read->file_size = ftell(read->ogfile);
	fseek(read->ogfile, 0, SEEK_SET);
	read->dur = 0;
	if (has_stream) {
		ogg_stream_clear(&the_os);
		read->dur = (Double) (s64) OGG_GranuleToTime(&the_info, max_gran);
		if (the_info.sample_rate) read->dur /= the_info.sample_rate;
		else read->dur /= the_info.frame_rate_base;
	}
	return has_stream;
}

static Bool OGG_CanHandleURL(NetClientPlugin *plug, const char *url)
{
	char *sExt;
	sExt = strrchr(url, '.');
	if (!sExt) return 0;

	if (NM_CheckExtension(plug, "application/ogg", "ogg", "Xiph.org OGG Movie", sExt)) return 1;
	return 0;
}

static Bool ogg_is_local(const char *url)
{
	if (!strnicmp(url, "file://", 7)) return 1;
	if (strstr(url, "://")) return 0;
	return 1;
}

void OGG_OnStatus(void *cbk)
{
	M4Err e;
	u32 bytes_done, net_status;
	OGGReader *read = (OGGReader *) cbk;
	e = NM_GetDownloaderStats(read->dnload, NULL, &bytes_done, NULL, &net_status);

	if (e && read->needs_connection) {
		read->needs_connection = 0;
		read->kill_demux = 2;
		NM_OnConnect(read->service, NULL, e);
	}
	else if (!bytes_done) {
		switch (net_status) {
		case DL_WaitingForAck:
			NM_OnMessage(read->service, M4OK, "Connecting...");
			return;
		case DL_Connected:
			NM_OnMessage(read->service, M4OK, "Connected");
			return;
		}
	}
	else if ((e == M4EOF) && read->ogfile) {
		read->is_remote = 0;
		/*reload file*/
		OGG_CheckFile(read);
		return;
	}
}

void OGG_DownloadFile(NetClientPlugin *plug, char *url)
{
	M4Err e;
	OGGReader *read = (OGGReader*) plug->priv;

	read->needs_connection = 1;

	e = NM_FetchFile(read->service, url, 0, OGG_OnStatus, NULL, read, &read->dnload);
	if (e) {
		read->kill_demux=2;
		read->needs_connection = 0;
		NM_OnConnect(read->service, NULL, e);
	}
	/*service confirm is done once fetched, but start the demuxer thread*/
	TH_Run(read->demuxer, OggDemux, read);
}


static M4Err OGG_ConnectService(NetClientPlugin *plug, LPNETSERVICE serv, const char *url)
{
	char szURL[2048];
	char *ext;
	M4Err reply;
	OGGReader *read = plug->priv;
	read->service = serv;

	if (read->dnload) NM_CloseDownloader(read->dnload);
	read->dnload = NULL;

	read->service_type = 0;
	strcpy(szURL, url);
	ext = strrchr(szURL, '#');
	if (ext) {
		if (!strcmp(ext, "#video")) read->service_type = 1;
		else if (!strcmp(ext, "#audio")) read->service_type = 2;
		ext[0] = 0;
	}

	/*remote fetch*/
	read->is_remote = !ogg_is_local(szURL);
	if (read->is_remote) {
		OGG_DownloadFile(plug, szURL);
		return M4OK;
	} else {
		read->ogfile = fopen(szURL, "rb");
		if (!read->ogfile) {
			reply = M4URLNotFound;
		} else {
			reply = M4OK;
			/*init ogg file in local mode*/
			if (!OGG_CheckFile(read)) {
				fclose(read->ogfile);
				reply = M4NonCompliantBitStream;
			} else {
				/*start the demuxer thread*/
				TH_Run(read->demuxer, OggDemux, read);
				return M4OK;
			}
		}
	}
	/*error*/
	read->kill_demux=2;
	NM_OnConnect(serv, NULL, reply);
	return M4OK;
}

static M4Err OGG_CloseService(NetClientPlugin *plug, Bool immediate_shutdown)
{
	OGGReader *read = plug->priv;
	if (!read->kill_demux) {
		read->kill_demux = 1;
		while (read->kill_demux!=2) Sleep(2);
	}

	if (read->ogfile) fclose(read->ogfile);
	read->ogfile = NULL;
	if (read->dnload) NM_CloseDownloader(read->dnload);
	read->dnload = NULL;
	NM_OnDisconnect(read->service, NULL, M4OK);
	return M4OK;
}

static M4Err OGG_Get_MPEG4_IOD(NetClientPlugin *plug, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size)
{
	ESDescriptor *esd;
	M4Err e;
	u32 i;
	ObjectDescriptor *od;
	OGGReader *read = plug->priv;
	/*since we don't handle multitrack in ogg yes, we don't need to check sub_url, only use expected type*/

	/*single object*/
	if ((expect_type==NM_OD_AUDIO) || (expect_type==NM_OD_VIDEO)) {
		if ((expect_type==NM_OD_AUDIO) && !read->has_audio) return M4URLNotFound;
		if ((expect_type==NM_OD_VIDEO) && !read->has_video) return M4URLNotFound;
		for (i=0; i<ChainGetCount(read->streams); i++) {
			OGGStream *st = ChainGetEntry(read->streams, i);
			if ((expect_type==NM_OD_AUDIO) && (st->info.streamType!=M4ST_AUDIO)) continue;
			if ((expect_type==NM_OD_VIDEO) && (st->info.streamType!=M4ST_VISUAL)) continue;
			
			od = OGG_GetOD(st, 1);
			e = OD_EncDesc((Descriptor *) od, raw_iod, raw_iod_size);
			OD_DeleteDescriptor((Descriptor **)&od);
			read->is_single_media = 1;
			return e;
		}
		/*not supported yet - we need to know what's in the ogg stream for that*/
		return M4URLNotFound;
	}

	od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
	od->objectDescriptorID = 1;

	/*inline scene*/
	/*OD ESD*/
	esd = OD_NewESDescriptor(0);
	esd->slConfig->timestampResolution = 1000;
	esd->decoderConfig->streamType = M4ST_OD;
	esd->decoderConfig->objectTypeIndication = GPAC_STATIC_OD_OTI;
	esd->ESID = 1;
	ChainAddEntry(od->ESDescriptors, esd);
	e = OD_EncDesc((Descriptor *) od, raw_iod, raw_iod_size);
	OD_DeleteDescriptor((Descriptor **)&od);
	return e;
}

static M4Err OGG_ConnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	u32 ES_ID, i;
	M4Err e;
	OGGReader *read = plug->priv;

	e = M4ServiceError;
	if (read->od_ch==channel) goto exit;

	e = M4ChannelNotFound;
	if (strstr(url, "ES_ID")) {
		sscanf(url, "ES_ID=%d", &ES_ID);
	}
	/*URL setup*/
//	else if (!read->es_ch && OGG_CanHandleURL(plug, url)) ES_ID = 3;

	switch (ES_ID) {
	case 1:
		read->od_ch = channel;
		e = M4OK;
		break;
	default:
		for (i=0; i<ChainGetCount(read->streams); i++) {
			OGGStream *st = ChainGetEntry(read->streams, i);
			if (st->ESID==ES_ID) {
				st->ch = channel;
				e = M4OK;
				break;
			}
		}
		break;
	}

exit:
	NM_OnConnect(read->service, channel, e);
	return e;
}

static M4Err OGG_DisconnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	M4Err e;
	OGGReader *read = plug->priv;

	e = M4ChannelNotFound;
	if (read->od_ch == channel) {
		read->od_ch = NULL;
		e = M4OK;
	} else {
		u32 i;
		for (i=0; i<ChainGetCount(read->streams); i++) {
			OGGStream *st = ChainGetEntry(read->streams, i);
			if (st->ch==channel) {
				st->ch = NULL;
				e = M4OK;
				break;
			}
		}
	}
	NM_OnDisconnect(read->service, channel, e);
	return M4OK;
}

static M4Err OGG_ServiceCommand(NetClientPlugin *plug, NetworkCommand *com)
{
	OGGReader *read = plug->priv;

	if (!com->base.on_channel) return M4NotSupported;
	switch (com->command_type) {
	case CHAN_SET_PULL:
		/*no way to demux streams independently, and we keep OD as dynamic ogfile to handle
		chained streams*/
		return M4NotSupported;
	case CHAN_INTERACTIVE:
		//live: return M4NotSupported;
		return M4OK;
	case CHAN_BUFFER:
		com->buffer.min = com->buffer.max = 0;
		if (read->is_live && (com->base.on_channel != read->od_ch)) {
			com->buffer.max = read->data_buffer_ms;
		}
		return M4OK;
	case CHAN_SET_PADDING: return M4NotSupported;

	case CHAN_DURATION:
		com->duration.duration = read->dur;
		return M4OK;
	case CHAN_PLAY:
		read->start_range = com->play.start_range;
		read->end_range = com->play.end_range;
		if (read->od_ch == com->base.on_channel) {
			read->needs_od = 1;
			assert(!read->nb_playing);
		} else {
			u32 i;
			for (i=0; i<ChainGetCount(read->streams); i++) {
				OGGStream *st = ChainGetEntry(read->streams, i);
				if (st->ch == com->base.on_channel) {
					st->is_running = 1;
					st->map_time = read->dur ? 1 : 0;
					if (!read->nb_playing) read->do_seek = 1;
					read->nb_playing ++;
					break;
				}
			}
		}
		/*recfg duration in case*/
		if (!read->is_remote && read->dur) { 
			NetworkCommand rcfg;
			rcfg.base.on_channel = read->od_ch;
			rcfg.base.command_type = CHAN_DURATION;
			rcfg.duration.duration = read->dur;
			NM_OnCommand(read->service, &rcfg, M4OK);
		}
		return M4OK;
	case CHAN_STOP:
		if (read->od_ch == com->base.on_channel) {
		}
		else {
			u32 i;
			for (i=0; i<ChainGetCount(read->streams); i++) {
				OGGStream *st = ChainGetEntry(read->streams, i);
				if (st->ch == com->base.on_channel) {
					st->is_running = 0;
					read->nb_playing --;
					break;
				}
			}
		}
		return M4OK;
	default:
		return M4OK;
	}
}

static Bool OGG_CanHandleURLInService(NetClientPlugin *plug, const char *url)
{
	char szURL[2048], *sep;
	OGGReader *read = (OGGReader *)plug->priv;
	const char *this_url = NM_GetServiceURL(read->service);
	if (!this_url || !url) return 0;

	strcpy(szURL, this_url);
	sep = strrchr(szURL, '#');
	if (sep) sep[0] = 0;

	if ((url[0] != '#') && strnicmp(szURL, url, sizeof(char)*strlen(szURL))) return 0;
	sep = strrchr(url, '#');
	if (!stricmp(sep, "#video") && (read->has_video)) return 1;
	if (!stricmp(sep, "#audio") && (read->has_audio)) return 1;
	return 0;
}

NetClientPlugin *OGG_LoadDemux()
{
	OGGReader *reader;
	NetClientPlugin *plug = malloc(sizeof(NetClientPlugin));
	memset(plug, 0, sizeof(NetClientPlugin));
	M4_REG_PLUG(plug, M4STREAMINGCLIENT, "GPAC OGG Reader", "gpac distribution", 0)

	plug->CanHandleURL = OGG_CanHandleURL;
	plug->ConnectService = OGG_ConnectService;
	plug->CloseService = OGG_CloseService;
	plug->Get_MPEG4_IOD = OGG_Get_MPEG4_IOD;
	plug->ConnectChannel = OGG_ConnectChannel;
	plug->DisconnectChannel = OGG_DisconnectChannel;
	plug->ServiceCommand = OGG_ServiceCommand;
	plug->CanHandleURLInService = OGG_CanHandleURLInService;

	reader = malloc(sizeof(OGGReader));
	memset(reader, 0, sizeof(OGGReader));
	reader->streams = NewChain();
	reader->demuxer = NewThread();
	reader->data_buffer_ms = 1000;

	plug->priv = reader;
	return plug;
}

void OGG_DeleteDemux(void *ifce)
{
	NetClientPlugin *plug = (NetClientPlugin *) ifce;
	OGGReader *read = plug->priv;
	TH_Delete(read->demuxer);

	/*just in case something went wrong*/
	while (ChainGetCount(read->streams)) {
		OGGStream *st = ChainGetEntry(read->streams, 0);
		ChainDeleteEntry(read->streams, 0);
		ogg_stream_clear(&st->os);
		if (st->dsi) free(st->dsi);
		free(st);
	}
	DeleteChain(read->streams);
	free(read);
	free(plug);
}
