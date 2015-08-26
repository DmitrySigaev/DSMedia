/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Authoring Tools sub-project
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
#include <gpac/m4_bifs.h>
#include <gpac/intern/m4_scenegraph_dev.h>

static u32 GetNbBits(u32 MaxVal)
{
	u32 k=0;
	while ((s32) MaxVal > ((1<<k)-1) ) k+=1;
	return k;
}


M4SceneManager *NewSceneManager(LPSCENEGRAPH graph)
{
	M4SceneManager *tmp;
	
	if (!graph) return NULL;
	SAFEALLOC(tmp, sizeof(M4SceneManager));
	tmp->streams = NewChain();
	tmp->scene_graph = graph;
	return tmp;
}

M4StreamContext *M4SM_NewStream(M4SceneManager *ctx, u16 ES_ID, u8 streamType, u8 objectType)
{
	u32 i;
	M4StreamContext *tmp;

	for (i=0; i<ChainGetCount(ctx->streams); i++) {
		tmp = ChainGetEntry(ctx->streams, i);
		/*we MUST use the same ST*/
		if (tmp->streamType!=streamType) continue;
		/*if no ESID/OTI specified this is a base layer (default stream created by parsers)
		if ESID/OTI specified this is a stream already setup
		*/
		if ( (ES_ID && (tmp->ESID==ES_ID)) ) {
			tmp->ESID = ES_ID;
			tmp->objectType = objectType;
			return tmp;
		}
	}
	
	tmp = malloc(sizeof(M4StreamContext));
	memset(tmp , 0, sizeof(M4StreamContext));
	tmp->AUs = NewChain();
	tmp->ESID = ES_ID;
	tmp->streamType = streamType;
	tmp->objectType = objectType;
	tmp->timeScale = 1000;
	ChainAddEntry(ctx->streams, tmp);
	return tmp;
}

void M4SM_DeleteStream(M4StreamContext *sc)
{
	u32 count, count2;
	while ( (count = ChainGetCount(sc->AUs)) ) {
		M4AUContext *au = ChainGetEntry(sc->AUs, count-1);
		ChainDeleteEntry(sc->AUs, count-1);

		while ( (count2 = ChainGetCount(au->commands)) ) {
			void *comptr = ChainGetEntry(au->commands, count2 - 1);
			ChainDeleteEntry(au->commands, count2 - 1);
			switch (sc->streamType) {
			case M4ST_OD:
				OD_DeleteCommand((ODCommand**) & comptr);
				break;
			case M4ST_SCENE:
			{
				if (sc->objectType == 0x09) { // LASeR
					assert(0);
				} else {
					SG_DeleteCommand((SGCommand *)comptr);
				}
			}
				break;
			}
		}
		DeleteChain(au->commands);
		free(au);
	}
	DeleteChain(sc->AUs);
	free(sc);
}

void M4SM_RemoveStream(M4SceneManager *ctx, M4StreamContext *sc)
{
	u32 i;
	for (i=0; i<ChainGetCount(ctx->streams); i++) {
		M4StreamContext *ptr = ChainGetEntry(ctx->streams, i);
		if (ptr == sc) {
			ChainDeleteEntry(ctx->streams, i);
			M4SM_DeleteStream(sc);
			return;
		}
	}
}

void M4SM_Delete(M4SceneManager *ctx)
{
	u32 count;
	while ( (count = ChainGetCount(ctx->streams)) ) {
		M4StreamContext *sc = ChainGetEntry(ctx->streams, count-1);
		ChainDeleteEntry(ctx->streams, count-1);
		M4SM_DeleteStream(sc);
	}
	DeleteChain(ctx->streams);
	if (ctx->root_od) OD_DeleteDescriptor((Descriptor **) &ctx->root_od);
	free(ctx);
}


M4AUContext *M4SM_NewAU(M4StreamContext *stream, u32 timing, Double time_sec, Bool isRap)
{
	u32 i;
	M4AUContext *tmp;

	/*look for existing AU*/
	for (i=0; i<ChainGetCount(stream->AUs); i++) {
		tmp = ChainGetEntry(stream->AUs, i);
		if (timing && (tmp->timing==timing)) return tmp;
		else if (time_sec && (tmp->timing_sec == time_sec)) return tmp;
		else if (!time_sec && !timing && !tmp->timing && !tmp->timing_sec) return tmp;
		/*insert AU*/
		else if ((time_sec && time_sec<tmp->timing_sec) || (timing && timing<tmp->timing)) {
			tmp = malloc(sizeof(M4AUContext));
			tmp->commands = NewChain();
			tmp->is_rap = isRap;
			tmp->timing = timing;
			tmp->timing_sec = time_sec;
			tmp->owner = stream;
			ChainInsertEntry(stream->AUs, tmp, i);
			return tmp;
		}
	}
	tmp = malloc(sizeof(M4AUContext));
	tmp->commands = NewChain();
	tmp->is_rap = isRap;
	tmp->timing = timing;
	tmp->timing_sec = time_sec;
	tmp->owner = stream;
	ChainAddEntry(stream->AUs, tmp);
	return tmp;
}

static MuxInfoDescriptor *M4SM_GetMuxInfo(ESDescriptor *src)
{
	u32 i;
	MuxInfoDescriptor *mux = NULL;
	for (i=0; i<ChainGetCount(src->extensionDescriptors); i++) {
		mux = ChainGetEntry(src->extensionDescriptors, i);
		if (mux->tag == MuxInfoDescriptor_Tag) return mux;
	}
	return NULL;
}

static void M4SM_RemoveMuxInfo(ESDescriptor *src)
{
	u32 i;
	MuxInfoDescriptor *mux = NULL;
	for (i=0; i<ChainGetCount(src->extensionDescriptors); i++) {
		mux = ChainGetEntry(src->extensionDescriptors, i);
		if (mux->tag == MuxInfoDescriptor_Tag) {
			OD_DeleteDescriptor((Descriptor **)&mux);
			ChainDeleteEntry(src->extensionDescriptors, i);
			return;
		}
	}
}

#ifndef M4_READ_ONLY


static void M4SM_FinalizeMux(M4File *mp4, ESDescriptor *src, u32 offset_ts)
{
	u32 track;
	MuxInfoDescriptor *mux = M4SM_GetMuxInfo(src);
	if (!mux && !offset_ts) return;
	track = M4_GetTrackByID(mp4, src->ESID);
	if (!track) return;

	/*set track time offset*/
	if (mux) offset_ts += mux->startTime * src->slConfig->timestampResolution / 1000;
	if (offset_ts) {
		M4_SetEditSegment(mp4, track, 0, offset_ts * M4_GetTimeScale(mp4) / src->slConfig->timestampResolution, 0, M4_EDIT_EMPTY);
		M4_SetEditSegment(mp4, track, offset_ts * M4_GetTimeScale(mp4) / src->slConfig->timestampResolution, (u32) M4_GetMediaDuration(mp4, track), 0, M4_EDIT_NORMAL);
	}
	/*set track interleaving ID*/
	if (mux && mux->GroupID) M4_SetTrackGroup(mp4, track, mux->GroupID);
}

M4Err M4SM_ImportInputSensor(M4File *mp4, ESDescriptor *src)
{
	UIConfigDescriptor *cfg;
	u32 len, i;
	M4Err e;
	if (!src->slConfig) src->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
	src->slConfig->predefined = 2;
	src->slConfig->timestampResolution = 1000;
	if (!src->decoderConfig || !src->decoderConfig->decoderSpecificInfo) return M4InvalidDescriptor;
	if (src->decoderConfig->decoderSpecificInfo->tag == UIConfig_Tag) {
		cfg = (UIConfigDescriptor *) src->decoderConfig->decoderSpecificInfo;
		e = OD_EncodeUIConfig(cfg, &src->decoderConfig->decoderSpecificInfo);
		OD_DeleteDescriptor((Descriptor **) &cfg);
		if (e) return e;
	} else if (src->decoderConfig->decoderSpecificInfo->tag != DecoderSpecificInfo_Tag) {
		return M4InvalidDescriptor;
	}
	/*what's the media type for input sensor ??*/
	len = M4_NewTrack(mp4, src->ESID, M4_BIFSMediaType, 1000);
	if (!len) return M4_GetLastError(mp4);
	M4_SetTrackEnabled(mp4, len, 1);
	if (!src->ESID) src->ESID = M4_GetTrackID(mp4, len);
	return M4_NewStreamDescription(mp4, len, src, NULL, NULL, &i);
}



M4Err M4SM_ImportStream(M4SceneManager *ctx, M4File *mp4, ESDescriptor *src, char *mediaSource)
{
	u32 track, di;
	M4Err e;
	Bool isAudio, isVideo;
	char szName[1024];
	char *ext;
	M4TrackImporter import;
	MuxInfoDescriptor *mux = NULL;

	/*no import if URL string*/
	if (src->URLString) {
		u32 mtype, track;
		if (!src->slConfig) src->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
		if (!src->decoderConfig) {
			fprintf(stdout, "ESD with URL string needs a decoder config with remote stream type to be encoded\n");
			return M4BadParam;
		}
		/*however we still need a track to store the ESD ...*/
		switch (src->decoderConfig->streamType) {
		case M4ST_VISUAL:
			mtype = M4_VisualMediaType;
			break;
		case M4ST_AUDIO:
			mtype = M4_AudioMediaType;
			break;
		case M4ST_MPEG7:
			mtype = M4_MPEG7MediaType;
			break;
		case M4ST_IPMP:
			mtype = M4_IPMPMediaType;
			break;
		case M4ST_OCI:
			mtype = M4_OCIMediaType;
			break;
		case M4ST_MPEGJ:
			mtype = M4_MPEGJMediaType;
			break;
		case M4ST_INTERACT:
		case M4ST_SCENE:
			mtype = M4_BIFSMediaType;
			break;
		case M4ST_TEXT:
			mtype = M4_TimedTextMediaType;
			break;
		default:
			fprintf(stdout, "Unsupported media type %d for ESD with URL string\n", src->decoderConfig->streamType);
			return M4BadParam;
		}
		track = M4_NewTrack(mp4, src->ESID, mtype, 1000);
		if (!src->ESID) src->ESID = M4_GetTrackID(mp4, track);
		return M4_NewStreamDescription(mp4, track, src, NULL, NULL, &di);
	}

	/*look for muxInfo*/
	mux = M4SM_GetMuxInfo(src);

	/*special streams*/
	if (src->decoderConfig) {
		/*InputSensor*/
		if (src->decoderConfig->decoderSpecificInfo && (src->decoderConfig->decoderSpecificInfo->tag == UIConfig_Tag)) 
			src->decoderConfig->streamType = M4ST_INTERACT;
		if (src->decoderConfig->streamType == M4ST_INTERACT) return M4SM_ImportInputSensor(mp4, src);
	}


	/*OCR streams*/
	if (src->decoderConfig && src->decoderConfig->streamType == M4ST_OCR) {
		track = M4_NewTrack(mp4, src->ESID, M4_OCRMediaType, 1000);
		if (!track) return M4_GetLastError(mp4);
		M4_SetTrackEnabled(mp4, track, 1);
		if (!src->ESID) src->ESID = M4_GetTrackID(mp4, track);
		if (!src->slConfig) src->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
		src->slConfig->predefined = 2;
		e = M4_NewStreamDescription(mp4, track, src, NULL, NULL, &di);
		if (e) return e;
		if (mux && mux->duration) 
			e = M4_SetEditSegment(mp4, track, 0, mux->duration * M4_GetTimeScale(mp4) / 1000, 0, M4_EDIT_NORMAL);
		return e;
	}

	if (!mux) {
		/*if existing don't import (systems tracks)*/
		track = M4_GetTrackByID(mp4, src->ESID);
		if (track) return M4OK;
		if (mediaSource) {
			memset(&import, 0, sizeof(M4TrackImporter));
			import.dest = mp4;
			import.trackID = src->ESID;
			import.orig = M4_MovieOpen(mediaSource, M4_OPEN_READ);
			if (import.orig) {
				e = MP4T_ImportMedia(&import);
				M4_MovieDelete(import.orig);
				return e;
			}
		}
		return M4OK;
	}

	if (!mux->file_name) return M4OK;

	memset(&import, 0, sizeof(M4TrackImporter));
	strcpy(szName, mux->file_name);
	ext = strrchr(szName, '.');	

	/*get track types for AVI*/
	if (ext && !strnicmp(ext, ".avi", 4)) {
		isAudio = isVideo = 0;
		if (ext && !stricmp(ext, ".avi#video")) isVideo = 1;
		else if (ext && !stricmp(ext, ".avi#audio")) isAudio = 1;
		else if (src->decoderConfig) {
			if (src->decoderConfig->streamType == M4ST_VISUAL) isVideo = 1;
			else if (src->decoderConfig->streamType == M4ST_AUDIO) isAudio = 1;
		}
		if (!isAudio && !isVideo) {
			fprintf(stdout, "Please specify video or audio for AVI import (file#audio, file#video)\n");
			return M4NotSupported;
		}
		if (isVideo) import.trackID = 1;
		else import.trackID = 2;
		ext = strchr(ext, '#');
		if (ext) ext[0] = 0;
	}
	/*get track ID for MP4/others*/
	if (ext) {
		ext = strchr(ext, '#');
		if (ext) {
			import.trackID = atoi(ext+1);
			ext[0] = 0;
		}
	}

	import.dest = mp4;
	import.esd = src;
	import.duration = mux->duration;
	import.flags = mux->import_flags;
	import.in_name = szName;
	e = MP4T_ImportMedia(&import);
	if (e) return e;

	/*if desired delete input*/
	if (mux->delete_file) M4_DeleteFile(mux->file_name);
	return e;
}

static M4Err M4SM_ImportSpecialESD(M4SceneManager *ctx, ESDescriptor *esd)
{
	char *ext;
	M4Err e;
	MuxInfoDescriptor *mux = M4SM_GetMuxInfo(esd);
	if (!mux || !mux->file_name) return M4OK;
	
	if (esd->decoderConfig && esd->decoderConfig->decoderSpecificInfo
		&& (esd->decoderConfig->decoderSpecificInfo->tag==TextConfig_Tag)) return M4OK;


	e = M4OK;
	ext = strrchr(mux->file_name, '.');
	/*SRT import*/
	if ((ext && !stricmp(ext, ".srt")) || (mux->streamFormat && !stricmp(mux->streamFormat, "SRT")) ) {
		/*if text node unspecified, use StreamingText import*/
		if (mux->textNode) {
			e = M4SM_ImportSRT_BIFS(ctx, esd, mux);
			M4SM_RemoveMuxInfo(esd);
		} 
	}
	return e;
}

static M4Err M4SM_ImportSpecials(M4SceneManager *ctx)
{
	M4Err e;
	u32 i, j, n, m, k;
	ESDescriptor *esd;
	M4StreamContext *sc;

	for (i=0; i<ChainGetCount(ctx->streams); i++) {
		sc = ChainGetEntry(ctx->streams, i);
		if (sc->streamType != M4ST_OD) continue;
		esd = NULL;
		for (j=0; j<ChainGetCount(sc->AUs); j++) {
			M4AUContext *au = ChainGetEntry(sc->AUs, j);
			
			for (k=0; k<ChainGetCount(au->commands); k++) {
				ODCommand *com = ChainGetEntry(au->commands, k);
				switch (com->tag) {
				case ODUpdate_Tag:
				{
					ObjectDescriptorUpdate *odU = (ObjectDescriptorUpdate *)com;
					for (n=0; n<ChainGetCount(odU->objectDescriptors); n++) {
						ObjectDescriptor *od = ChainGetEntry(odU->objectDescriptors, n);
						for (m=0; m<ChainGetCount(od->ESDescriptors); m++) {
							ESDescriptor *imp_esd = ChainGetEntry(od->ESDescriptors, m);
							e = M4SM_ImportSpecialESD(ctx, imp_esd);
							if (e != M4OK) return e;
						}
					}
				}
					break;
				case ESDUpdate_Tag:
				{
					ESDescriptorUpdate *esdU = (ESDescriptorUpdate *)com;
					for (m=0; m<ChainGetCount(esdU->ESDescriptors); m++) {
						ESDescriptor *imp_esd = ChainGetEntry(esdU->ESDescriptors, m);
						e = M4SM_ImportSpecialESD(ctx, imp_esd);
						if (e != M4OK) return e;
					}
				}
					break;
				}
			}
		}
	}
	return M4OK;
}

/*locate stream in all OD updates/ESD updates (needed for systems tracks)*/
static ESDescriptor *M4SM_LocateESD(M4SceneManager *ctx, u16 ES_ID)
{
	u32 i, j, n, m, k;
	ESDescriptor *esd;
	M4StreamContext *sc;
	if (!ES_ID) return NULL;

	for (i=0; i<ChainGetCount(ctx->streams); i++) {
		sc = ChainGetEntry(ctx->streams, i);
		if (sc->streamType != M4ST_OD) continue;
		esd = NULL;
		for (j=0; j<ChainGetCount(sc->AUs); j++) {
			M4AUContext *au = ChainGetEntry(sc->AUs, j);
			
			for (k=0; k<ChainGetCount(au->commands); k++) {
				ODCommand *com = ChainGetEntry(au->commands, k);
				switch (com->tag) {
				case ODUpdate_Tag:
				{
					ObjectDescriptorUpdate *odU = (ObjectDescriptorUpdate *)com;
					for (n=0; n<ChainGetCount(odU->objectDescriptors); n++) {
						ObjectDescriptor *od = ChainGetEntry(odU->objectDescriptors, n);
						for (m=0; m<ChainGetCount(od->ESDescriptors); m++) {
							ESDescriptor *imp_esd = ChainGetEntry(od->ESDescriptors, m);
							if (imp_esd->ESID == ES_ID) return imp_esd;
						}
					}
				}
					break;
				case ESDUpdate_Tag:
				{
					ESDescriptorUpdate *esdU = (ESDescriptorUpdate *)com;
					for (m=0; m<ChainGetCount(esdU->ESDescriptors); m++) {
						ESDescriptor *imp_esd = ChainGetEntry(esdU->ESDescriptors, m);
						if (imp_esd->ESID == ES_ID) return imp_esd;
					}
				}
					break;
				}
			}
		}
	}
	return NULL;
}

static M4Err M4SM_EncodeBIFS(M4SceneManager *ctx, M4File *mp4, char *logFile, u32 flags, u32 rap_freq)
{
	u32 i, j, di;
	char *data;
	Bool is_in_iod, delete_desc, first_bifs_id;
	u32 dur, rate, time_slice, init_offset;
	u32 data_len, count;
	u32 track;
	u32 last_rap, rap_delay;
	M4Err e;
	FILE *logs;
	BIFSConfigDescriptor bcfg;
	InitialObjectDescriptor *iod;
	LPBIFSENC bifsenc;
	M4AUContext *au;
	M4Sample *samp;
	M4StreamContext *sc;
	Bool encode_names, rap_inband, rap_shadow;
	ESDescriptor *esd;

	encode_names = (flags & M4SM_EncodeNames) ? 1 :0;
	rap_inband = rap_shadow = 0;
	if (rap_freq) {
		if (flags & M4SM_RAPInBand) {
			rap_inband = 1;
		} else {
			rap_shadow = 1;
		}
	}

	bifsenc = BIFS_NewEncoder(ctx->scene_graph);

	e = M4OK;
	iod = (InitialObjectDescriptor *) ctx->root_od;
	/*if no iod check we only have one bifs*/
	if (!iod) {
		count = 0;
		for (i=0; i<ChainGetCount(ctx->streams); i++) {
			sc = ChainGetEntry(ctx->streams, i);
			if (sc->streamType == M4ST_OD) count++;
		}
		if (!iod && count>1) return M4NotSupported;
	}

	count = ChainGetCount(ctx->streams);

	logs = NULL;
	if (logFile) {
		logs = fopen(logFile, "wt");
		BE_SetTrace(bifsenc, logs);
	}

	delete_desc = 0;
	first_bifs_id = 0;

	/*configure streams*/
	for (i=0; i<count; i++) {
		M4StreamContext *sc = ChainGetEntry(ctx->streams, i);
		esd = NULL;
		if (sc->streamType != M4ST_SCENE) continue;

		delete_desc = 0;
		esd = NULL;
		is_in_iod = 1;
		if (iod) {
			is_in_iod = 0;
			for (j=0; j<ChainGetCount(iod->ESDescriptors); j++) {
				esd = ChainGetEntry(iod->ESDescriptors, j);
				if (esd->decoderConfig && esd->decoderConfig->streamType == M4ST_SCENE) {
					if (!sc->ESID) sc->ESID = esd->ESID;
					if (sc->ESID == esd->ESID) {
						is_in_iod = 1;
						break;
					}
				}
				/*special BIFS direct import from NHNT*/
				else if (ChainGetCount(iod->ESDescriptors)==1) {
					sc->ESID = esd->ESID;
					is_in_iod = 1;
					break;
				}
				esd = NULL;
			}
		}
		if (!esd && sc->ESID) esd = M4SM_LocateESD(ctx, sc->ESID);

		if (!esd) {
			delete_desc = 1;
			esd = OD_NewESDescriptor(2);
			OD_DeleteDescriptor((Descriptor **) &esd->decoderConfig->decoderSpecificInfo);
			esd->ESID = sc->ESID;
			esd->decoderConfig->streamType = M4ST_SCENE;
		}

		/*special BIFS direct import from NHNT*/
		au = ChainGetEntry(sc->AUs, 0);
		if (ChainGetCount(sc->AUs) == 1) {
			if (ChainGetCount(au->commands) == 1) {
				SGCommand *com = ChainGetEntry(au->commands, 0);
				/*no root node, no protos (empty replace) - that's BIFS NHNT import*/
				if ((com->tag == SG_SceneReplace) && !com->node && !ChainGetCount(com->new_proto_list))
					au = NULL;
			}
		} 
		/*sanity check: remove first command if it is REPLACE SCENE BY NULL*/
		if (au && !au->timing && !au->timing_sec && (ChainGetCount(au->commands) > 1)) {
			SGCommand *com = ChainGetEntry(au->commands, 0);
			if (com->tag==SG_SceneReplace) {
				if (!com->node && !ChainGetCount(com->new_proto_list) ) {
					ChainDeleteEntry(au->commands, 0);
					SG_DeleteCommand(com);
				}
			}
		}
		if (!au) {
			e = M4SM_ImportStream(ctx, mp4, esd, NULL);
			if (e) goto exit;
			M4SM_FinalizeMux(mp4, esd, 0);
			if (is_in_iod) M4_AddTrackToRootOD(mp4, M4_GetTrackByID(mp4, esd->ESID));
			continue;
		}

		if (!esd->slConfig) esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
		if (sc->timeScale) esd->slConfig->timestampResolution = sc->timeScale;
		if (!esd->slConfig->timestampResolution) esd->slConfig->timestampResolution = 1000;
		track = M4_NewTrack(mp4, sc->ESID, M4_BIFSMediaType, esd->slConfig->timestampResolution);
		if (!track) {
			e = M4_GetLastError(mp4);
			goto exit;
		}
		M4_SetTrackEnabled(mp4, track, 1);
		if (!sc->ESID) sc->ESID = M4_GetTrackID(mp4, track);
		esd->ESID = sc->ESID;

		/*force BIFS dependencies (we cannot encode in 2 different scene contexts)*/
		if (!esd->dependsOnESID) {
			if (!first_bifs_id) {
				esd->dependsOnESID = 0;
				first_bifs_id = esd->ESID;
			} else {
				esd->dependsOnESID = first_bifs_id;
			}
		}
		if (!esd->decoderConfig) esd->decoderConfig = (DecoderConfigDescriptor*)OD_NewDescriptor(DecoderConfigDescriptor_Tag);
		esd->decoderConfig->streamType = M4ST_SCENE;

		if (!esd->decoderConfig->decoderSpecificInfo) {
			memset(&bcfg, 0, sizeof(BIFSConfigDescriptor));
			bcfg.tag = BIFSConfig_Tag;
		} else if (esd->decoderConfig->decoderSpecificInfo->tag == BIFSConfig_Tag) {
			memcpy(&bcfg, (BIFSConfigDescriptor *)esd->decoderConfig->decoderSpecificInfo, sizeof(BIFSConfigDescriptor));
		} else {
			OD_GetBIFSConfig(esd->decoderConfig->decoderSpecificInfo, esd->decoderConfig->objectTypeIndication, &bcfg);
		}
		/*create final BIFS config*/
		if (esd->decoderConfig->decoderSpecificInfo) OD_DeleteDescriptor((Descriptor **) &esd->decoderConfig->decoderSpecificInfo);
		esd->decoderConfig->decoderSpecificInfo = (DefaultDescriptor *) OD_NewDescriptor(DecoderSpecificInfo_Tag);

		/*update NodeIDbits and co*/
		/*nodeID bits shall include NULL node*/
		bcfg.nodeIDbits = GetNbBits(ctx->max_node_id);
		bcfg.routeIDbits = GetNbBits(ctx->max_node_id);
		bcfg.protoIDbits = GetNbBits(ctx->max_proto_id);
		
		bcfg.isCommandStream = 1;

		bcfg.pixelMetrics = ctx->is_pixel_metrics;
		bcfg.pixelWidth = ctx->scene_width;
		bcfg.pixelHeight = ctx->scene_height;

		/*this is for safety, otherwise some players may not understand NULL node*/
		if (!bcfg.nodeIDbits) bcfg.nodeIDbits = 1;

		BIFS_NewStream(bifsenc, sc->ESID, &bcfg, encode_names, 0);

		/*get final BIFS config*/
		BIFS_GetStreamConfig(bifsenc, sc->ESID, &data, &data_len);
		esd->decoderConfig->decoderSpecificInfo->data = data;
		esd->decoderConfig->decoderSpecificInfo->dataLength = data_len;
		esd->decoderConfig->objectTypeIndication = BIFS_GetVersion(bifsenc, sc->ESID);		
	
		/*create stream description*/
		M4_NewStreamDescription(mp4, track, esd, NULL, NULL, &di);
		if (is_in_iod) M4_AddTrackToRootOD(mp4, track);

		dur = esd->decoderConfig->avgBitrate = 0;
		esd->decoderConfig->bufferSizeDB = 0;
		esd->decoderConfig->maxBitrate = rate = time_slice = 0;

		last_rap = 0;
		rap_delay = rap_freq * esd->slConfig->timestampResolution / 1000;

		init_offset = 0;
		for (j=0; j<ChainGetCount(sc->AUs); j++) {
			au = ChainGetEntry(sc->AUs, j);
			samp = M4_NewSample();
			/*time in sec conversion*/
			if (au->timing_sec) au->timing = (u32) (au->timing_sec * esd->slConfig->timestampResolution);

			if (!j) init_offset = au->timing;

			samp->DTS = au->timing - init_offset;
			samp->IsRAP = au->is_rap;
			if (samp->IsRAP) last_rap = au->timing;

			/*inband RAP insertion*/
			if (rap_inband) {
				/*apply commands*/
				e = SG_ApplyCommandList(ctx->scene_graph, au->commands, 0);
				if (samp->DTS - last_rap < rap_delay) {
					e = BIFS_EncodeAU(bifsenc, sc->ESID, au->commands, &samp->data, &samp->dataLength);
				} else {
					e = BIFS_GetRAP(bifsenc, &samp->data, &samp->dataLength);
					samp->IsRAP = 1;
					last_rap = samp->DTS;
				}
			} else {
				e = BIFS_EncodeAU(bifsenc, sc->ESID, au->commands, &samp->data, &samp->dataLength);
			}
			/*if no commands don't add the AU*/
			if (!e && samp->dataLength) e = M4_AddSample(mp4, track, di, samp);

			dur = au->timing;
			esd->decoderConfig->avgBitrate += samp->dataLength;
			rate += samp->dataLength;
			if (esd->decoderConfig->bufferSizeDB<samp->dataLength) esd->decoderConfig->bufferSizeDB = samp->dataLength;
			if (samp->DTS - time_slice > esd->slConfig->timestampResolution) {
				if (esd->decoderConfig->maxBitrate < rate) esd->decoderConfig->maxBitrate = rate;
				rate = 0;
				time_slice = samp->DTS;
			}
			
			M4_DeleteSample(&samp);
			if (e) goto exit;
		}

		if (dur) {
			esd->decoderConfig->avgBitrate *= esd->slConfig->timestampResolution * 8 / dur;
			esd->decoderConfig->maxBitrate *= 8;
		} else {
			esd->decoderConfig->avgBitrate = 0;
			esd->decoderConfig->maxBitrate = 0;
		}
		M4_ChangeStreamDescriptor(mp4, track, 1, esd);

		/*sync shadow generation*/
		if (rap_shadow) {
			last_rap = 0;
			for (j=0; j<ChainGetCount(sc->AUs); j++) {
				M4AUContext *au = ChainGetEntry(sc->AUs, j);
				e = SG_ApplyCommandList(ctx->scene_graph, au->commands, 0);
				if (!j || (au->timing - last_rap < rap_delay) ) continue;

				samp = M4_NewSample();
				samp->DTS = au->timing;
				samp->IsRAP = 1;
				last_rap = au->timing;
				/*RAP generation*/
				e = BIFS_GetRAP(bifsenc, &samp->data, &samp->dataLength);
				if (!e) e = M4_AddSampleShadow(mp4, track, samp);
				M4_DeleteSample(&samp);
				if (e) goto exit;
			}
		}

		/*if offset add edit list*/
		M4SM_FinalizeMux(mp4, esd, init_offset);

		if (delete_desc) OD_DeleteDescriptor((Descriptor **) &esd);
	}

	/*to do - proper PL setup according to node used...*/
	M4_SetMoviePLIndication(mp4, M4_PL_SCENE, 1);
	M4_SetMoviePLIndication(mp4, M4_PL_GRAPHICS, 1);

exit:
	BIFS_DeleteEncoder(bifsenc);
	if (logFile) fclose(logs);
	if (esd && delete_desc) OD_DeleteDescriptor((Descriptor **) &esd);
	return e;
}


M4Err M4SM_EncodeOD(M4SceneManager *ctx, M4File *mp4, char *mediaSource)
{
	u32 i, j, n, m;
	ESDescriptor *esd;
	M4StreamContext *sc;
	u32 count, track, di, init_offset;
	u32 dur, rate, time_slice;
	Bool is_in_iod, delete_desc;

	M4Sample *samp;
	M4Err e;
	LPODCODEC codec;
	InitialObjectDescriptor *iod;

	iod = (InitialObjectDescriptor *) ctx->root_od;
	count = 0;
	for (i=0; i<ChainGetCount(ctx->streams); i++) {
		sc = ChainGetEntry(ctx->streams, i);
		if (sc->streamType == M4ST_OD) count++;
	}
	/*no OD stream, nothing to do*/
	if (!count) return M4OK;
	if (!iod && count>1) return M4NotSupported;

	codec = NULL;

	for (i=0; i<ChainGetCount(ctx->streams); i++) {
		sc = ChainGetEntry(ctx->streams, i);
		if (sc->streamType != M4ST_OD) continue;

		delete_desc = 0;
		esd = NULL;
		is_in_iod = 1;
		if (iod) {
			is_in_iod = 0;
			for (j=0; j<ChainGetCount(iod->ESDescriptors); j++) {
				esd = ChainGetEntry(iod->ESDescriptors, j);
				if (esd->decoderConfig->streamType != M4ST_OD){
					esd = NULL;
					continue;
				}
				if (!sc->ESID) sc->ESID = esd->ESID;
				if (sc->ESID == esd->ESID) {
					is_in_iod = 1;
					break;
				}
			}
		}
		if (!esd) esd = M4SM_LocateESD(ctx, sc->ESID);
		if (!esd) {
			delete_desc = 1;
			esd = OD_NewESDescriptor(2);
			esd->ESID = sc->ESID;
			esd->decoderConfig->objectTypeIndication = 1;
			esd->decoderConfig->streamType = M4ST_OD;
		}

		/*create OD track*/
		if (!esd->slConfig) esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
		if (sc->timeScale) esd->slConfig->timestampResolution = sc->timeScale;
		if (!esd->slConfig->timestampResolution) esd->slConfig->timestampResolution = 1000;
		track = M4_NewTrack(mp4, sc->ESID, M4_ODMediaType, esd->slConfig->timestampResolution);
		if (!sc->ESID) sc->ESID = M4_GetTrackID(mp4, track);
		M4_SetTrackEnabled(mp4, track, 1);
		/*no DSI required*/
		/*create stream description*/
		M4_NewStreamDescription(mp4, track, esd, NULL, NULL, &di);
		/*add to root OD*/
		if (is_in_iod) M4_AddTrackToRootOD(mp4, track);

		codec = OD_NewCodec(OD_WRITE);

		dur = esd->decoderConfig->avgBitrate = 0;
		esd->decoderConfig->bufferSizeDB = 0;
		esd->decoderConfig->maxBitrate = rate = time_slice = 0;

		init_offset = 0;
		/*encode all samples and perform import*/
		for (j=0; j<ChainGetCount(sc->AUs); j++) {
			M4AUContext *au = ChainGetEntry(sc->AUs, j);
			
			while (ChainGetCount(au->commands) ) {
				ODCommand *com = ChainGetEntry(au->commands, 0);
				ChainDeleteEntry(au->commands, 0);
				/*only updates commandes need to be parsed for import*/
				switch (com->tag) {
				case ODUpdate_Tag:
				{
					ObjectDescriptorUpdate *odU = (ObjectDescriptorUpdate *)com;
					for (n=0; n<ChainGetCount(odU->objectDescriptors); n++) {
						ObjectDescriptor *od = ChainGetEntry(odU->objectDescriptors, n);
						for (m=0; m<ChainGetCount(od->ESDescriptors); m++) {
							ESDescriptor *imp_esd = ChainGetEntry(od->ESDescriptors, m);
							switch (imp_esd->tag) {
							case ESDescriptor_Tag:
								e = M4SM_ImportStream(ctx, mp4, imp_esd, mediaSource);
								if (e) {
									fprintf(stdout, "Error importing stream %d\n", imp_esd->ESID);
									OD_DeleteCommand(&com);
									goto err_exit;
								}
								M4SM_FinalizeMux(mp4, imp_esd, 0);
								break;
							case ES_ID_RefTag:
							case ES_ID_IncTag:
								break;
							default:
								fprintf(stdout, "Invalid descriptor in OD%d.ESDescr\n", od->objectDescriptorID);
								e = M4BadParam;
								goto err_exit;
								break;
							}
						}
					}
				}
					break;
				case ESDUpdate_Tag:
				{
					ESDescriptorUpdate *esdU = (ESDescriptorUpdate *)com;
					for (m=0; m<ChainGetCount(esdU->ESDescriptors); m++) {
						ESDescriptor *imp_esd = ChainGetEntry(esdU->ESDescriptors, m);
						switch (imp_esd->tag) {
						case ESDescriptor_Tag:
							e = M4SM_ImportStream(ctx, mp4, imp_esd, mediaSource);
							if (e) {
								fprintf(stdout, "Error importing stream %d\n", imp_esd->ESID);
								OD_DeleteCommand(&com);
								goto err_exit;
							}
							M4SM_FinalizeMux(mp4, imp_esd, 0);
							break;
						case ES_ID_RefTag:
						case ES_ID_IncTag:
							break;
						default:
							fprintf(stdout, "Invalid descriptor in ESDUpdate (OD %d)\n", esdU->ODID);
							e = M4BadParam;
							goto err_exit;
							break;
						}
					}
				}
					break;
				}

				/*add to codec*/
				OD_AddCommand(codec, com);
			}
			e = OD_EncodeAU(codec);
			if (e) goto err_exit;

			/*time in sec conversion*/
			if (au->timing_sec) au->timing = (u32) (au->timing_sec * esd->slConfig->timestampResolution);

			if (!j) init_offset = au->timing;

			samp = M4_NewSample();
			samp->DTS = au->timing - init_offset;
			samp->IsRAP = au->is_rap;
			e = OD_GetEncodedAU(codec, &samp->data, &samp->dataLength);
			if (!e) e = M4_AddSample(mp4, track, di, samp);

			dur = au->timing - init_offset;
			esd->decoderConfig->avgBitrate += samp->dataLength;
			rate += samp->dataLength;
			if (esd->decoderConfig->bufferSizeDB<samp->dataLength) esd->decoderConfig->bufferSizeDB = samp->dataLength;
			if (samp->DTS - time_slice > esd->slConfig->timestampResolution) {
				if (esd->decoderConfig->maxBitrate < rate) esd->decoderConfig->maxBitrate = rate;
				rate = 0;
				time_slice = samp->DTS;
			}

			M4_DeleteSample(&samp);
			if (e) goto err_exit;
		}

		if (dur) {
			esd->decoderConfig->avgBitrate *= esd->slConfig->timestampResolution * 8 / dur;
			esd->decoderConfig->maxBitrate *= 8;
		} else {
			esd->decoderConfig->avgBitrate = 0;
			esd->decoderConfig->maxBitrate = 0;
		}
		M4_ChangeStreamDescriptor(mp4, track, 1, esd);

		M4SM_FinalizeMux(mp4, esd, init_offset);
		if (delete_desc) OD_DeleteDescriptor((Descriptor **) &esd);
	}
	e = M4_SetMoviePLIndication(mp4, M4_PL_OD, 1);
	
err_exit:
	if (codec) OD_DeleteCodec(codec);
	return e;
}

M4Err M4SM_EncodeFile(M4SceneManager *ctx, M4File *mp4, char *logFile, char *mediaSource, u32 flags, u32 rap_freq)
{
	u32 i, count;
	Descriptor *desc;
	M4Err e;
	if (!ctx->scene_graph) return M4BadParam;
	if (ctx->root_od && (ctx->root_od->tag != InitialObjectDescriptor_Tag) && (ctx->root_od->tag != ObjectDescriptor_Tag)) return M4BadParam;

	/*import specials, that is input remapping to BIFS*/
	e = M4SM_ImportSpecials(ctx);
	if (e) return e;


	/*encode BIFS*/
	e = M4SM_EncodeBIFS(ctx, mp4, logFile, flags, rap_freq);
	if (e) return e;
	/*then encode OD to setup all streams*/
	e = M4SM_EncodeOD(ctx, mp4, mediaSource);
	if (e) return e;

	/*store iod*/
	if (ctx->root_od) {
		M4_SetRootOD_ID(mp4, ctx->root_od->objectDescriptorID);
		if (ctx->root_od->URLString) M4_SetRootOD_URL(mp4, ctx->root_od->URLString);
		count = ChainGetCount(ctx->root_od->extensionDescriptors);
		for (i=0; i<count; i++) {
			desc = ChainGetEntry(ctx->root_od->extensionDescriptors, i);
			M4_AddDescriptorToRootOD(mp4, desc);
		}
		count = ChainGetCount(ctx->root_od->IPMPDescriptorPointers);
		for (i=0; i<count; i++) {
			desc = ChainGetEntry(ctx->root_od->IPMPDescriptorPointers, i);
			M4_AddDescriptorToRootOD(mp4, desc);
		}
		count = ChainGetCount(ctx->root_od->OCIDescriptors);
		for (i=0; i<count; i++) {
			desc = ChainGetEntry(ctx->root_od->OCIDescriptors, i);
			M4_AddDescriptorToRootOD(mp4, desc);
		}
		/*we assume all ESs described in bt/xmt input are used*/
	}

	/*set PLs*/
	if (ctx->root_od && ctx->root_od->tag==InitialObjectDescriptor_Tag) {
		InitialObjectDescriptor *iod =  (InitialObjectDescriptor *)ctx->root_od;
		M4_SetMoviePLIndication(mp4, M4_PL_OD, iod->OD_profileAndLevel);
		M4_SetMoviePLIndication(mp4, M4_PL_SCENE, iod->scene_profileAndLevel);
		M4_SetMoviePLIndication(mp4, M4_PL_GRAPHICS, iod->graphics_profileAndLevel);
		/*these are setup while importing*/
//		M4_SetMoviePLIndication(mp4, M4_PL_VISUAL, iod->visual_profileAndLevel);
//		M4_SetMoviePLIndication(mp4, M4_PL_AUDIO, iod->audio_profileAndLevel);
	}

	return M4OK;
}

#endif
