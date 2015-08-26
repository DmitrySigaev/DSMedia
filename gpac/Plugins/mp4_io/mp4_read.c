/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / MP4 reader plugin
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

#include "mp4_io.h"

static M4INLINE M4Channel *m4_get_channel(M4Reader *reader, LPNETCHANNEL channel)
{
	u32 i;
	M4Channel *ch;
	for (i=0; i<ChainGetCount(reader->channels); i++) {
		ch = ChainGetEntry(reader->channels, i);
		if (ch->channel == channel) return ch;
	}
	return NULL;
}

static void m4_delete_channel(M4Reader *reader, M4Channel *ch)
{
	u32 i;
	M4Channel *ch2;
	for (i=0; i<ChainGetCount(reader->channels); i++) {
		ch2 = ChainGetEntry(reader->channels, i);
		if (ch2 == ch) {
			m4_reset_reader(ch);
			free(ch);
			ChainDeleteEntry(reader->channels, i);
			return;
		}
	}
}

static M4INLINE Bool mp4_is_local(const char *url)
{
	if (!strnicmp(url, "file://", 7)) return 1;
	if (strstr(url, "://")) return 0;
	/*the rest is local (mounted on FS)*/
	return 1;
}

Bool M4R_CanHandleURL(NetClientPlugin *plug, const char *url)
{
	char *ext;
	if (!strnicmp(url, "rtsp://", 7)) return 0;
	ext = strrchr(url, '.');
	if (!ext) return 0;

	if (NM_CheckExtension(plug, "video/mp4", "mp4 mpg4", "MPEG-4 Movies", ext)) return 1;
	if (NM_CheckExtension(plug, "audio/mp4", "m4a mp4 mpg4", "MPEG-4 Music", ext)) return 1;
	if (NM_CheckExtension(plug, "application/mp4", "mp4 mpg4", "MPEG-4 Applications", ext)) return 1;
	if (NM_CheckExtension(plug, "video/3gpp", "3gp 3gpp", "3GPP/MMS Movies", ext)) return 1;
	if (NM_CheckExtension(plug, "audio/3gpp", "3gp 3gpp", "3GPP/MMS Music",ext)) return 1;
	if (NM_CheckExtension(plug, "video/3gpp2", "3g2 3gp2", "3GPP2/MMS Movies",ext)) return 1;
	if (NM_CheckExtension(plug, "audio/3gpp2", "3g2 3gp2", "3GPP2/MMS Music",ext)) return 1;
	return 0;
}

M4Err M4R_ConnectService(NetClientPlugin *plug, LPNETSERVICE serv, const char *url)
{
	char szURL[2048];
	char *tmp;
	M4Reader *read;
	if (!plug || !plug->priv || !serv) return M4ServiceError;
	read = (M4Reader *) plug->priv;

	read->service = serv;

	if (read->dnload) NM_CloseDownloader(read->dnload);
	read->dnload = NULL;

	read->base_track_id = 0;
	strcpy(szURL, url);
	tmp = strrchr(szURL, '.');
	if (tmp) {
		tmp = strchr(tmp, '#');
		if (tmp) {
			if (!strnicmp(tmp, "#trackID=", 9)) {
				read->base_track_id = atoi(tmp+9);
			} else {
				read->base_track_id = atoi(tmp+1);
			}
			tmp[0] = 0;
		}
	}

	if (mp4_is_local(szURL)) {
		if (!read->mov) read->mov = M4_MovieOpen(szURL, M4_OPEN_READ);
		if (!read->mov) {
			NM_OnConnect(serv, NULL, M4_GetLastError(NULL));
			return M4OK;
		}
		read->time_scale = M4_GetTimeScale(read->mov);
		/*reply to user*/
		NM_OnConnect(serv, NULL, M4OK);
	} else {
		/*setup downloader*/
		MP4_SetupDownload(plug, szURL);
	}

	return M4OK;
}

M4Err M4R_CloseService(NetClientPlugin *plug, Bool immediate_shutdown)
{
	M4Err reply;
	M4Reader *read;
	if (!plug || !plug->priv) return M4ServiceError;
	read = (M4Reader *) plug->priv;
	reply = M4ServiceNotFound;

	if (read->mov) {
		M4_MovieClose(read->mov);
		reply = M4OK;
	}
	read->mov = NULL;
	while (ChainGetCount(read->channels)) {
		M4Channel *ch = ChainGetEntry(read->channels, 0);
		ChainDeleteEntry(read->channels, 0);
		m4_delete_channel(read, ch);
	}

	if (read->od_au) free(read->od_au);
	read->od_au = NULL;

	if (read->dnload) NM_CloseDownloader(read->dnload);
	read->dnload = NULL;

	NM_OnDisconnect(read->service, NULL, reply);
	return M4OK;
}

/*fixme, this doesn't work properly with respect to @expect_type*/
M4Err M4R_Get_MPEG4_IOD(NetClientPlugin *plug, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size)
{
	u32 count, nb_st, i, trackID;
	M4Err e;
	ESDescriptor *esd;
	M4Reader *read;
	InitialObjectDescriptor *iod;
	if (!plug || !plug->priv) return M4ServiceError;
	read = (M4Reader *) plug->priv;
	if (!read->mov) return M4ServiceError;

	/*no matter what always read text as TTUs*/
	M4_SetStreamingTextMode(read->mov, 1);

	if ((expect_type==NM_OD_SCENE) || (expect_type==NM_OD_UNDEF)) trackID = 0;
	else if (!sub_url) {
		trackID = read->base_track_id;
		read->base_track_id = 0;
	} else {
		char *ext = strrchr(sub_url, '#');
		if (!ext) {
			trackID = 0;
		} else {
			if (!strnicmp(ext, "#trackID=", 9)) ext += 9;
			trackID = atoi(ext);
		}
	}

	if (trackID && (expect_type!=NM_OD_SCENE) ) {
		u32 track = M4_GetTrackByID(read->mov, trackID);
		if (!track) return M4URLNotFound;
		esd = MP4T_MapESDescriptor(read->mov, track);
		esd->OCRESID = 0;
		iod = (InitialObjectDescriptor *) M4_GetRootOD(read->mov);
		if (!iod) {
			iod = (InitialObjectDescriptor *) OD_NewDescriptor(InitialObjectDescriptor_Tag);
			iod->OD_profileAndLevel = iod->audio_profileAndLevel = iod->graphics_profileAndLevel = iod->scene_profileAndLevel = iod->visual_profileAndLevel = 0xFE;
		} else {
			while (ChainGetCount(iod->ESDescriptors)) {
				ESDescriptor *old = ChainGetEntry(iod->ESDescriptors, 0);
				OD_DeleteDescriptor((Descriptor **) &old);
				ChainDeleteEntry(iod->ESDescriptors, 0);
			}
		}
		ChainAddEntry(iod->ESDescriptors, esd);
		e = OD_EncDesc((Descriptor *) iod, raw_iod, raw_iod_size);
		OD_DeleteDescriptor((Descriptor**) &iod);
		return e;
	}
	iod = (InitialObjectDescriptor *) M4_GetRootOD(read->mov);
	if (!iod) return M4R_EmulateIOD(read, raw_iod, raw_iod_size);

	count = ChainGetCount(iod->ESDescriptors);
	if (!count) {
		OD_DeleteDescriptor((Descriptor**) &iod);
		return M4R_EmulateIOD(read, raw_iod, raw_iod_size);
	}
	if (count==1) {
		esd = ChainGetEntry(iod->ESDescriptors, 0);
		switch (esd->decoderConfig->streamType) {
		case M4ST_SCENE:
			break;
		case M4ST_VISUAL:
			if (expect_type!=NM_OD_VIDEO) {
				OD_DeleteDescriptor((Descriptor**) &iod);
				return M4R_EmulateIOD(read, raw_iod, raw_iod_size);
			}
			break;
		case M4ST_AUDIO:
			/*we need a fake scene graph*/
			if (expect_type!=NM_OD_AUDIO) {
				OD_DeleteDescriptor((Descriptor**) &iod);
				return M4R_EmulateIOD(read, raw_iod, raw_iod_size);
			}
			break;
		default:
			OD_DeleteDescriptor((Descriptor**) &iod);
			return M4InvalidURL;
		}
	}
	/*check IOD is not badly formed (eg mixing audio, video or text streams)*/
	nb_st = 0;
	for (i=0; i<count; i++) {
		esd = ChainGetEntry(iod->ESDescriptors, i);
		switch (esd->decoderConfig->streamType) {
		case M4ST_VISUAL: nb_st |= 1; break;
		case M4ST_AUDIO: nb_st |= 2; break;
		case M4ST_TEXT: nb_st |= 4; break;
		}
	}
	if ( (nb_st & 1) + (nb_st & 2) + (nb_st & 4) > 1) {
		OD_DeleteDescriptor((Descriptor**) &iod);
		return M4R_EmulateIOD(read, raw_iod, raw_iod_size);
	}
	e = OD_EncDesc((Descriptor *) iod, raw_iod, raw_iod_size);
	OD_DeleteDescriptor((Descriptor**) &iod);
	return e;
}


M4Err M4R_ConnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	u32 ESID;
	M4Channel *ch;
	u32 track;
	Bool is_esd_url;
	M4Err e;
	M4Reader *read;
	if (!plug || !plug->priv) return M4ServiceError;
	read = (M4Reader *) plug->priv;

	ch = NULL;
	is_esd_url = 0;
	e = M4OK;
	if (upstream) {
		e = M4InvalidMP4File;
		goto exit;
	}
	if (!read->mov) return M4ServiceError;


	if (strstr(url, "ES_ID")) {
		sscanf(url, "ES_ID=%d", &ESID);
	} else {
		/*handle url like mypath/myfile.mp4#trackID*/
		char *track_id = strrchr(url, '.');
		if (track_id) {
			track_id = strchr(url, '#');
			if (track_id) track_id ++;
		}
		is_esd_url = 1;

		ESID = 0;
		/*if only one track ok*/
		if (M4_GetTrackCount(read->mov)==1) ESID = M4_GetTrackID(read->mov, 1);
		else if (track_id) {
			ESID = atoi(track_id);
			track = M4_GetTrackByID(read->mov, (u32) ESID);
			if (!track) ESID = 0;
		}

	}
	if (!ESID) {
		e = M4UnsupportedURL;
		goto exit;
	}

	/*a channel cannot be open twice, it has to be closed before - NOTE a track is NOT a channel and the user can open
	several times the same track as long as a dedicated channel is used*/
	ch = m4_get_channel(read, channel);
	if (ch) {
		e = M4ServiceError;
		goto exit;
	}
	if (ESID == read->OD_ESID) {
		track = 0;
	} else {
		track = M4_GetTrackByID(read->mov, (u32) ESID);
		if (!track) {
			e = M4ChannelNotFound;
			goto exit;
		}
	}
	ch = malloc(sizeof(M4Channel));
	memset(ch, 0, sizeof(M4Channel));
	ch->owner = read;
	ch->channel = channel;
	ChainAddEntry(read->channels, ch);
	ch->track = track;
	if (M4_GetMediaType(ch->owner->mov, ch->track) == M4_OCRMediaType) ch->is_ocr = 1;

	if (track) {
		ch->has_edit_list = M4_GetEditSegmentCount(ch->owner->mov, ch->track) ? 1 : 0;
		ch->time_scale = M4_GetMediaTimeScale(ch->owner->mov, ch->track);
	} else {
		ch->FAKE_ESID = ESID;
		ch->time_scale = 1000;
	}

exit:
	NM_OnConnect(read->service, channel, e);
	/*if esd url reconfig SL layer*/
	if (!e && is_esd_url) {
		NetworkCommand com;
		ESDescriptor *esd;
		memset(&com, 0, sizeof(NetworkCommand));
		com.base.on_channel = channel;
		com.command_type = CHAN_RECONFIG;
		esd = M4_GetStreamDescriptor(read->mov, ch->track, 1);
		if (esd) {
			memcpy(&com.cfg.sl_config, esd->slConfig, sizeof(SLConfigDescriptor));
			OD_DeleteDescriptor((Descriptor **)&esd);
		} else {
			com.cfg.sl_config.tag = SLConfigDescriptor_Tag;
			com.cfg.sl_config.timestampLength = 32;
			com.cfg.sl_config.timestampResolution = ch->time_scale;
			com.cfg.sl_config.useRandomAccessPointFlag = 1;
		}
		NM_OnCommand(read->service, &com, M4OK);
	}
	return e;
}

M4Err M4R_DisconnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	M4Channel *ch;
	M4Err e;
	M4Reader *read;
	if (!plug || !plug->priv) return M4ServiceError;
	read = (M4Reader *) plug->priv;
	if (!read->mov) return M4ServiceError;

	e = M4OK;
	ch = m4_get_channel(read, channel);
	assert(ch);
	if (!ch) {
		e = M4ChannelNotFound;
		goto exit;
	}
	/*signal the service is broken but still process the delete*/
	m4_delete_channel(read, ch);
	assert(!m4_get_channel(read, channel));

exit:
	NM_OnDisconnect(read->service, channel, e);
	return e;
}

M4Err M4R_ChannelGetSLP(NetClientPlugin *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, struct tagSLHeader *out_sl_hdr, Bool *sl_compressed, M4Err *out_reception_status, Bool *is_new_data)
{
	M4Channel *ch;
	M4Reader *read;
	if (!plug || !plug->priv) return M4ServiceError;
	/*cannot read native SL-PDUs*/
	if (!out_sl_hdr) return M4NotSupported;
	read = (M4Reader *) plug->priv;
	if (!read->mov) return M4ServiceError;

	*out_data_ptr = NULL;
	*out_data_size = 0;
	*sl_compressed = 0;
	*out_reception_status = M4OK;
	ch = m4_get_channel(read, channel);
	if (!ch) return M4ChannelNotFound;
	if (!ch->is_playing) return M4OK;

	*is_new_data = 0;
	if (!ch->sample) {
		/*get sample*/
		m4_reader_get_sample(ch);
		*is_new_data = 1;
	}

	if (ch->sample) {
		*out_data_ptr = ch->sample->data;
		*out_data_size = ch->sample->dataLength;
		*out_sl_hdr = ch->current_slh;
	}
	*out_reception_status = ch->last_state;
	return M4OK;
}

M4Err M4R_ChannelReleaseSLP(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	M4Channel *ch;
	M4Reader *read;
	if (!plug || !plug->priv) return M4ServiceError;
	read = (M4Reader *) plug->priv;
	if (!read->mov) return M4ServiceError;
	ch = m4_get_channel(read, channel);
	if (!ch) return M4ChannelNotFound;
	if (!ch->is_playing) return M4ServiceError;

	if (ch->sample) {
		m4_reader_release_sample(ch);
		/*release sample*/
	}
	return M4OK;
}

static u32 check_round(M4Channel *ch, u32 val_ts, Double val_range, Bool make_greater)
{
	Double round_check = val_ts;
	round_check /= ch->time_scale;
	if (round_check != val_range) val_ts += make_greater ? 1 : -1;
	return val_ts;
}

M4Err M4R_ServiceCommand(NetClientPlugin *plug, NetworkCommand *com)
{
	Double track_dur, media_dur;
	M4Channel *ch;
	M4Reader *read;
	if (!plug || !plug->priv || !com) return M4ServiceError;
	read = (M4Reader *) plug->priv;

	if (!com->base.on_channel) return M4OK;

	ch = m4_get_channel(read, com->base.on_channel);
	if (!ch) return M4ChannelNotFound;

	switch (com->command_type) {
	case CHAN_SET_PADDING:
		if (!ch->track) return M4OK;
		M4_SetSamplesPadding(read->mov, ch->track, com->pad.padding_bytes);
		return M4OK;
	case CHAN_SET_PULL:
		ch->is_pulling = 1;
		return M4OK;
	case CHAN_INTERACTIVE:
		return M4OK;
	case CHAN_BUFFER:
		com->buffer.max = com->buffer.min = 0;
		return M4OK;
	case CHAN_DURATION:
		if (!ch->track) {
			com->duration.duration = 0;
			return M4OK;
		}
		ch->duration = (u32) M4_GetTrackDuration(read->mov, ch->track);
		track_dur = (Double) ch->duration;
		track_dur /= read->time_scale;
		if (M4_GetEditSegmentCount(read->mov, ch->track)) {
			com->duration.duration = (Double) track_dur;
			ch->duration = (u32) (track_dur * ch->time_scale);
		} else {
			/*some file indicate a wrong TrackDuration, get the longest*/
			ch->duration = (u32) M4_GetMediaDuration(read->mov, ch->track);
			media_dur = (Double) ch->duration;
			media_dur /= ch->time_scale;
			com->duration.duration = MAX(track_dur, media_dur);
		}
		return M4OK;
	case CHAN_PLAY:
		if (!ch->is_pulling) return M4NotSupported;
		assert(!ch->is_playing);
		m4_reset_reader(ch);
		ch->speed = com->play.speed;
		ch->start = ch->end = 0;
		if (com->play.speed>0) {
			if (com->play.start_range>=0) {
				ch->start = (u32) (ch->time_scale * com->play.start_range);
				ch->start = check_round(ch, ch->start, com->play.start_range, 1);
			}
			if (com->play.end_range >= com->play.start_range) {
				ch->end = (u32) (ch->time_scale * com->play.end_range);
				ch->end = check_round(ch, ch->end, com->play.end_range, 0);
			}
		} else if (com->play.speed<0) {
			if (com->play.end_range>=com->play.start_range) ch->start = (u32) (ch->time_scale * com->play.start_range);
			if (com->play.end_range >= 0) ch->end = (u32) (ch->time_scale * com->play.end_range);
		}
		ch->is_playing = 1;
		return M4OK;
	case CHAN_STOP:
		m4_reset_reader(ch);
		return M4OK;

	/*nothing to do on MP4 for channel config*/
	case CHAN_CONFIG:
		return M4OK;
	case CHAN_GET_DSI:
	{
		/*it may happen that there are conflicting config when using ESD URLs...*/
		DecoderConfigDescriptor *dcd = M4_GetDecoderInformation(read->mov, ch->track, 1);
		com->get_dsi.dsi = NULL;
		com->get_dsi.dsi_len = 0;
		if (dcd) {
			if (dcd->decoderSpecificInfo) {
				com->get_dsi.dsi = dcd->decoderSpecificInfo->data;
				com->get_dsi.dsi_len = dcd->decoderSpecificInfo->dataLength;
				dcd->decoderSpecificInfo->data = NULL;
			}
			OD_DeleteDescriptor((Descriptor **) &dcd);
		}
	}
		return M4OK;
	}
	return M4NotSupported;
}

static Bool M4R_CanHandleURLInService(NetClientPlugin *plug, const char *url)
{
	char szURL[2048], *sep;
	u32 ID;
	M4Reader *read = (M4Reader *)plug->priv;
	const char *this_url = NM_GetServiceURL(read->service);
	if (!this_url || !url) return 0;

	strcpy(szURL, this_url);
	sep = strrchr(szURL, '#');
	if (sep) sep[0] = 0;

	/*direct addressing in service*/
	if (url[0] == '#') return 1;
	if (strnicmp(szURL, url, sizeof(char)*strlen(szURL))) return 0;
	sep = strrchr(url, '#');
	if (!sep) return 0;
	sep += 1;
	ID = 0;
	if (!strnicmp(sep, "trackID=", 8)) sep+=8;
	ID = atoi(sep);
	if (!ID) return 0;
	if (M4_GetTrackByID(read->mov, ID)) return 1;
	return 0;
}

NetClientPlugin *M4R_LoadPlugin()
{
	M4Reader *reader;
	NetClientPlugin *plug = malloc(sizeof(NetClientPlugin));
	memset(plug, 0, sizeof(NetClientPlugin));
	M4_REG_PLUG(plug, M4STREAMINGCLIENT, "GPAC IsoMedia Reader", "gpac distribution", 0)

	plug->CanHandleURL = M4R_CanHandleURL;
	plug->ConnectService = M4R_ConnectService;
	plug->CloseService = M4R_CloseService;
	plug->Get_MPEG4_IOD = M4R_Get_MPEG4_IOD;
	plug->ConnectChannel = M4R_ConnectChannel;
	plug->DisconnectChannel = M4R_DisconnectChannel;
	plug->ServiceCommand = M4R_ServiceCommand;
	plug->CanHandleURLInService = M4R_CanHandleURLInService;
	/*we do support pull mode*/
	plug->ChannelGetSLP = M4R_ChannelGetSLP;
	plug->ChannelReleaseSLP = M4R_ChannelReleaseSLP;

	reader = malloc(sizeof(M4Reader));
	memset(reader, 0, sizeof(M4Reader));
	reader->channels = NewChain();
	plug->priv = reader;
	return plug;
}

void M4Read_Delete(BaseInterface *bi)
{
	NetClientPlugin *plug = (NetClientPlugin *) bi;
	M4Reader *read = plug->priv;

	DeleteChain(read->channels);
	free(read);
	free(bi);
}
