/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Stream Management sub-project
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

#include <gpac/m4_author.h>
#include <gpac/intern/m4_esm_dev.h>
#include "MediaMemory.h"


static Bool check_in_scene(InlineScene *scene, LPODMANAGER odm)
{
	u32 i;
	LPODMANAGER root;
	if (!scene) return 0;
	root = scene->root_od;
	while (1) {
		if (odm == root) return 1;
		if (!root->remote_OD) break;
		root = root->remote_OD;
	}
	scene = root->subscene;

	for (i=0; i<ChainGetCount(scene->ODlist); i++) {
		ODManager *ptr = ChainGetEntry(scene->ODlist, i);
		while (1) {
			if (ptr == odm) return 1;
			if (!ptr->remote_OD) break;
			ptr = ptr->remote_OD;
		}
		if (check_in_scene(ptr->subscene, odm)) return 1;
	}
	return 0;
}

static Bool M4T_CheckODM(MPEG4CLIENT term, LPODMANAGER odm)
{
	if (!term->root_scene) return 0;
	return check_in_scene(term->root_scene, odm);
}


/*returns top-level OD of the presentation*/
LPODMANAGER M4T_GetRootOD(MPEG4CLIENT term)
{
	if (!term) return NULL;
	if (!term->root_scene) return NULL;
	return term->root_scene->root_od;
}

/*returns number of sub-ODs in the current root. scene_od must be an inline OD*/
u32 M4T_GetODCount(MPEG4CLIENT term, LPODMANAGER scene_od)
{
	if (!term || !scene_od) return 0;
	if (!M4T_CheckODM(term, scene_od)) return 0;
	if (!scene_od->subscene) return 0;
	return ChainGetCount(scene_od->subscene->ODlist);
}

/*returns indexed (0-based) OD manager in the scene*/
LPODMANAGER M4T_GetODManager(MPEG4CLIENT term, LPODMANAGER scene_od, u32 index)
{
	if (!term || !scene_od) return NULL;
	if (!M4T_CheckODM(term, scene_od)) return NULL;
	if (!scene_od->subscene) return NULL;
	return (LPODMANAGER ) ChainGetEntry(scene_od->subscene->ODlist, index);
}

/*returns remote ODManager of this OD if any, NULL otherwise*/
LPODMANAGER M4T_GetRemoteOD(MPEG4CLIENT term, LPODMANAGER odm)
{
	if (!term || !odm) return NULL;
	if (!M4T_CheckODM(term, odm)) return NULL;
	return odm->remote_OD;
}


u32 M4T_IsInlineOD(MPEG4CLIENT term, LPODMANAGER odm)
{
	Bool IS_IsProtoLibObject(InlineScene *is, ODManager *odm);

	if (!term || !odm) return 0;
	if (!M4T_CheckODM(term, odm)) return 0;

	if (!odm->subscene) return 0;
	if (odm->parentscene) return IS_IsProtoLibObject(odm->parentscene, odm) ? 2 : 1;
	return 1;
}

/*select given object when stream selection is available*/
void M4T_SelectOD(MPEG4CLIENT term, LPODMANAGER odm)
{
	if (!term || !odm) return;
	if (!M4T_CheckODM(term, odm)) return;

	IS_SelectODM(term->root_scene, odm);
}



static void get_codec_stats(GenericCodec *dec, ODInfo *info)
{
	info->avg_bitrate = dec->avg_bit_rate;
	info->max_bitrate = dec->max_bit_rate;
	info->nb_dec_frames = dec->nb_dec_frames;
	info->max_dec_time = dec->max_dec_time;
	info->total_dec_time = dec->total_dec_time;
}

M4Err M4T_GetODInfo(MPEG4CLIENT term, LPODMANAGER odm, ODInfo *info)
{
	if (!term || !odm || !info) return M4BadParam;
	if (!M4T_CheckODM(term, odm)) return M4BadParam;

	memset(info, 0, sizeof(ODInfo));
	info->od = odm->OD;

	info->duration = odm->duration;
	info->duration /= 1000;
	if (odm->codec) {
		/*since we don't remove ODs that failed setup, check for clock*/
		if (odm->codec->ck) info->current_time = odm->codec->CB ? odm->current_time : CK_GetTime(odm->codec->ck);
		info->current_time /= 1000;
	} else if (odm->subscene && odm->subscene->scene_codec) {
		info->current_time = CK_GetTime(odm->subscene->scene_codec->ck);
		info->current_time /= 1000;
		info->duration = odm->subscene->duration;
		info->duration /= 1000;
	}

	info->buffer = -2;
	info->db_unit_count = 0;

	/*Warning: is_open==2 means object setup, don't check then*/
	if (odm->is_open==2) {
		info->status = 3;
	} else if (odm->is_open) {
		u32 i, buf;
		Clock *ck;
		
		ck = ODM_GetMediaClock(odm);
		/*no clock means setup failed*/
		if (!ck) {
			info->status = 4;
		} else {
			info->status = CK_IsStarted(ck) ? 1 : 2;
			info->clock_drift = ck->drift;

			info->buffer = -1;
			buf = 0;
			for (i=0; i<ChainGetCount(odm->channels); i++) {
				Channel *ch = ChainGetEntry(odm->channels, i);
				info->db_unit_count += ch->AU_Count;
				if (!ch->is_pulling) {
					if (ch->MaxBuffer) info->buffer = 0;
					buf += ch->BufferTime;
				}
			}
			if (buf) info->buffer = (s32) buf;
		}
	}

	info->has_profiles = (odm->Audio_PL<0) ? 0 : 1;
	if (info->has_profiles) {
		info->inline_pl = odm->ProfileInlining;
		info->OD_pl = odm->OD_PL; 
		info->scene_pl = odm->Scene_PL;
		info->audio_pl = odm->Audio_PL;
		info->visual_pl = odm->Visual_PL;
		info->graphics_pl = odm->Graphics_PL;
	}	

	info->service_handler = odm->net_service->ifce->plugin_name;
	info->service_url = odm->net_service->url;
	if (odm->net_service->owner == odm) info->owns_service = 1;

	if (odm->codec && odm->codec->decio) {
		if (!odm->codec->decio->GetName) {
			info->codec_name = odm->codec->decio->plugin_name;
		} else {
			info->codec_name = odm->codec->decio->GetName(odm->codec->decio);
		}
		info->od_type = odm->codec->type;
		if (odm->codec->CB) {
			info->cb_max_count = odm->codec->CB->Capacity;
			info->cb_unit_count = odm->codec->CB->UnitCount;
		}
	} 
	
	if (odm->subscene && (odm->subscene->root_od==odm) && odm->subscene->scene_codec) {
		BaseDecoder *dec = odm->subscene->scene_codec->decio;
		info->od_type = odm->subscene->scene_codec->type;
		if (!dec->GetName) {
			info->codec_name = dec->plugin_name;
		} else {
			info->codec_name = dec->GetName(dec);
		}
		SG_GetSizeInfo(odm->subscene->graph, &info->width, &info->height);
	} else if (odm->mo) {
		switch (info->od_type) {
		case M4ST_VISUAL:
			info->width = odm->mo->width;
			info->height = odm->mo->height;
			info->pixelFormat = odm->mo->pixelFormat;
			break;
		case M4ST_AUDIO:
			info->sample_rate = odm->mo->sample_rate;
			info->bits_per_sample = odm->mo->bits_per_sample;
			info->num_channels = odm->mo->num_channels;
			info->clock_drift = 0;
			break;
		case M4ST_TEXT:
			info->width = odm->mo->width;
			info->height = odm->mo->height;
			break;
		}
	}
	if (odm->subscene && odm->subscene->scene_codec) get_codec_stats(odm->subscene->scene_codec, info);
	else if (odm->codec) get_codec_stats(odm->codec, info);

	return M4OK;
}


Bool M4T_GetDownloadInfo(MPEG4CLIENT term, LPODMANAGER odm, u32 *d_enum, const char **url, u32 *bytes_done, u32 *total_bytes, Float *bytes_per_sec)
{
	LPFILEDOWNLOADER dnload;
	void NM_GetDownloaderInfo(LPFILEDOWNLOADER dnload, const char **url, u32 *bytes_done, u32 *total_bytes, Float *bytes_per_sec);
	if (!term || !odm || !M4T_CheckODM(term, odm)) return 0;
	if (odm->net_service->owner != odm) return 0;

	if (*d_enum >= ChainGetCount(odm->net_service->dnloads)) return 0;
	dnload = ChainGetEntry(odm->net_service->dnloads, *d_enum);
	if (!dnload) return 0;
	(*d_enum) ++;
	NM_GetDownloaderInfo(dnload, url, bytes_done, total_bytes, bytes_per_sec);
	return 1;
}

Bool M4T_GetChannelNetInfo(MPEG4CLIENT term, LPODMANAGER odm, u32 *d_enum, u32 *chid, NetStatCommand *netcom, M4Err *ret_code)
{
	Channel *ch;
	NetworkCommand com;
	if (!term || !odm || !M4T_CheckODM(term, odm)) return 0;
	if (*d_enum >= ChainGetCount(odm->channels)) return 0;
	ch = ChainGetEntry(odm->channels, *d_enum);
	if (!ch) return 0;
	(*d_enum) ++;
	if (ch->is_pulling) {
		(*ret_code) = M4NotSupported;
		return 1;
	}
	(*chid) = ch->esd->ESID;
	memset(&com, 0, sizeof(NetworkCommand));
	com.base.on_channel = ch;
	com.command_type = NM_NETSTAT;
	(*ret_code) = NM_ServiceCommand(ch->service, &com);
	memcpy(netcom, &com.net_stats, sizeof(NetStatCommand));
	return 1;
}

char *M4T_GetWorldInfo(MPEG4CLIENT term, LPODMANAGER scene_od, Chain *descriptions)
{
	M_WorldInfo *wi;
	u32 i;
	LPODMANAGER odm;
	if (!term) return NULL;
	wi = NULL;
	if (!scene_od) {
		if (!term->root_scene) return NULL;
		wi = term->root_scene->world_info;
	} else {
		if (!M4T_CheckODM(term, scene_od)) return NULL;
		odm = scene_od;
		while (odm->remote_OD) odm = odm->remote_OD;
		wi = odm->subscene ? odm->subscene->world_info : odm->parentscene->world_info;
	}
	if (!wi) return NULL;

	for (i=0; i<wi->info.count; i++) {
		ChainAddEntry(descriptions, strdup(wi->info.vals[i]));
	}
	return strdup(wi->title.buffer);
}


M4Err M4T_DumpSceneGraph(MPEG4CLIENT term, char *rad_name, Bool xml_dump, Bool skip_protos, LPODMANAGER scene_od)
{
#ifndef M4_READ_ONLY
	LPSCENEGRAPH sg;
	LPODMANAGER odm;
	LPSCENEDUMPER dumper;
	M4Err e;

	if (!term || !term->root_scene) return M4BadParam;
	if (!scene_od) {
		if (!term->root_scene) return M4BadParam;
		odm = term->root_scene->root_od;
	} else {
		odm = scene_od;
		if (!M4T_CheckODM(term, scene_od)) 
			odm = term->root_scene->root_od;
	}

	while (odm->remote_OD) odm = odm->remote_OD;
	if (odm->subscene) {
		if (!odm->subscene->graph) return M4IOErr;
		sg = odm->subscene->graph;
	} else {
		if (!odm->parentscene->graph) return M4IOErr;
		sg = odm->parentscene->graph;
	}

	dumper = NewSceneDumper(sg, rad_name, ' ', xml_dump ? M4SM_DUMP_AUTO_XML : M4SM_DUMP_AUTO_TXT);
	if (!dumper) return M4IOErr;
	e = SD_DumpGraph(dumper, skip_protos, 0);
	DeleteSceneDumper(dumper);
	return e;
#else
	return M4NotSupported;
#endif
}

