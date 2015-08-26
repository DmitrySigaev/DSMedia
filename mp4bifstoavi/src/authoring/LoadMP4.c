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

#include <intern/m4_author_dev.h>
#include <m4_bifs.h>

#ifndef M4_READ_ONLY

static void UpdateODCommand(M4File *mp4, ODCommand *com)
{
	u32 i, j;
	const char *szName;
	char szPath[2048];

	szName = M4_GetFilename(mp4);
	if (com->tag == ODUpdate_Tag) {
		ObjectDescriptorUpdate *odU = (ObjectDescriptorUpdate *)com;
		for (i=0; i<ChainGetCount(odU->objectDescriptors); i++) {
			ObjectDescriptor *od = ChainGetEntry(odU->objectDescriptors, i);
			for (j=0; j<ChainGetCount(od->ESDescriptors); j++) {
				ESDescriptor *esd = ChainGetEntry(od->ESDescriptors, j);
				if (esd->URLString) continue;
				switch (esd->decoderConfig->streamType) {
				case M4ST_OD:
				case M4ST_SCENE:
				case M4ST_OCR:
					break;
				default:
				{
					MuxInfoDescriptor *mi = (MuxInfoDescriptor *) OD_NewDescriptor(MuxInfoDescriptor_Tag);
					ChainAddEntry(esd->extensionDescriptors, mi);
					sprintf(szPath, "%s#%d", szName, esd->ESID);
					mi->file_name = strdup(szPath);
					mi->streamFormat = strdup("MP4");
				}
					break;
				}
			}
		}
		return;
	}
	if (com->tag == ESDUpdate_Tag) {
		ESDescriptorUpdate *esdU = (ESDescriptorUpdate *)com;
		for (i=0; i<ChainGetCount(esdU->ESDescriptors); i++) {
			ESDescriptor *esd = ChainGetEntry(esdU->ESDescriptors, i);
			if (esd->URLString) continue;
			switch (esd->decoderConfig->streamType) {
			case M4ST_OD:
			case M4ST_SCENE:
			case M4ST_OCR:
				break;
			default:
			{
				MuxInfoDescriptor *mi = (MuxInfoDescriptor *) OD_NewDescriptor(MuxInfoDescriptor_Tag);
				ChainAddEntry(esd->extensionDescriptors, mi);
				sprintf(szPath, "%s#%d", szName, esd->ESID);
				mi->file_name = strdup(szPath);
				mi->streamFormat = strdup("MP4");
			}
				break;
			}
		}
		return;
	}
}

static void mp4_report(M4ContextLoader *load, M4Err e, char *format, ...)
{
	va_list args;
	va_start(args, format);
	if (load->OnMessage) {
		char szMsg[1024];
		vsprintf(szMsg, format, args);
		load->OnMessage(load->cbk, szMsg, e);
	} else {
		if (e) fprintf(stdout, "%s: ", M4ErrToString(e));
		vfprintf(stdout, format, args);
		fprintf(stdout, "\n");
	}
	va_end(args);
}

M4Err M4SM_LoaderRun_MP4(M4ContextLoader *load)
{
	u32 i, j, di, nbBifs, nb_samp, samp_done;
	M4StreamContext *sc, *base_bifs;
	ESDescriptor *esd;
	LPODCODEC oddec;
	LPBIFSDEC bdec;
	M4Err e;

	if (!load || !load->isom) return M4BadParam;
	base_bifs = ChainGetEntry(load->ctx->streams, 0);
	/*no scene info*/
	if (!base_bifs) return M4OK;
	nbBifs = 1;
	e = M4OK;
	bdec = BIFS_NewDecoder(load->scene_graph, 1);
	oddec = OD_NewCodec(OD_READ);
	esd = NULL;
	/*load each stream*/
	nb_samp = 0;
	for (i=0; i<M4_GetTrackCount(load->isom); i++) {
		u32 type = M4_GetMediaType(load->isom, i+1);
		switch (type) {
		case M4_BIFSMediaType:
		case M4_ODMediaType:
			nb_samp += M4_GetSampleCount(load->isom, i+1);
			break;
		default:
			break;
		}
	}
	samp_done = 1;

	for (i=0; i<M4_GetTrackCount(load->isom); i++) {
		u32 type = M4_GetMediaType(load->isom, i+1);
		switch (type) {
		case M4_BIFSMediaType:
			break;
		case M4_ODMediaType:
			break;
		default:
			continue;
		}
		esd = M4_GetStreamDescriptor(load->isom, i+1, 1);
		if (!esd) continue;

		if (base_bifs->ESID!=esd->ESID) {
			sc = M4SM_NewStream(load->ctx, esd->ESID, esd->decoderConfig->streamType, esd->decoderConfig->objectTypeIndication);
			sc->streamType = esd->decoderConfig->streamType;
			sc->ESID = esd->ESID;
			sc->objectType = esd->decoderConfig->objectTypeIndication;
			sc->timeScale = M4_GetMediaTimeScale(load->isom, i+1);
			j=0;
		} else {
			j=1;
			sc = base_bifs;
		}
		/*we still need to reconfig the BIFS*/
		if (esd->decoderConfig->streamType==M4ST_SCENE) {
			if (!esd->dependsOnESID && nbBifs && !j) 
				mp4_report(load, M4OK, "Warning: several BIFS namespaces used or improper BIFS dependencies in file - import may be incorrect");
			e = BIFS_ConfigureStream(bdec, esd->ESID, esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, esd->decoderConfig->objectTypeIndication);
			if (e) goto exit;
			nbBifs++;
		}

		/*dump all AUs*/
		for (; j<M4_GetSampleCount(load->isom, i+1); j++) {
			M4AUContext *au;
			M4Sample *samp = M4_GetSample(load->isom, i+1, j+1, &di);
			if (!samp) {
				mp4_report(load, M4_GetLastError(load->isom), "Unable to fetch sample %d from track ID %d - aborting track import", j+1, M4_GetTrackID(load->isom, i+1));
				break;
//				e = M4_GetLastError(load->isom);
//				goto exit;
			}
			au = M4SM_NewAU(sc, samp->DTS, ((Double)samp->DTS) / sc->timeScale, samp->IsRAP);

			if (esd->decoderConfig->streamType==M4ST_SCENE) {
				e = BIFS_DecodeAUMemory(bdec, esd->ESID, samp->data, samp->dataLength, au->commands);
			} else {
				e = OD_SetBuffer(oddec, samp->data, samp->dataLength);
				if (!e) e = OD_DecodeAU(oddec);
				if (!e) {
					while (1) {
						ODCommand *odc = OD_GetCommand(oddec);
						if (!odc) break;
						/*update ESDs if any*/
						UpdateODCommand(load->isom, odc);
						ChainAddEntry(au->commands, odc);
					}
				}
			}
			M4_DeleteSample(&samp);
			if (e) goto exit;

			samp_done++;
			if (load->OnProgress) load->OnProgress(load->cbk, samp_done, nb_samp);
		}
		OD_DeleteDescriptor((Descriptor **) &esd);
	}

exit:
	BIFS_DeleteDecoder(bdec);
	OD_DeleteCodec(oddec);
	if (esd) OD_DeleteDescriptor((Descriptor **) &esd);
	return e;
}

M4Err M4SM_LoaderInit_MP4(M4ContextLoader *load)
{
	u32 i, track, di, timeScale;
	M4StreamContext *sc;
	ESDescriptor *esd;
	M4AUContext *au;
	M4Sample *samp;
	LPBIFSDEC bdec;
	M4Err e;
	if (!load->isom) return M4BadParam;

	/*load IOD*/
	load->ctx->root_od = (ObjectDescriptor *) M4_GetRootOD(load->isom);
	if (!load->ctx->root_od) {
		e = M4_GetLastError(load->isom);
		if (e) return e;
	} else if ((load->ctx->root_od->tag != ObjectDescriptor_Tag) && (load->ctx->root_od->tag != InitialObjectDescriptor_Tag)) {
		OD_DeleteDescriptor((Descriptor **) &load->ctx->root_od);
	}

	e = M4OK;
	
	esd = NULL;

	/*get root BIFS stream*/
	for (i=0; i<M4_GetTrackCount(load->isom); i++) {
		u32 type = M4_GetMediaType(load->isom, i+1);
		if (type != M4_BIFSMediaType) continue;
		if (! M4_IsTrackInRootOD(load->isom, i+1) ) continue;
		esd = M4_GetStreamDescriptor(load->isom, i+1, 1);
		/*make sure we load the root BIFS stream first*/
		if (esd && esd->dependsOnESID) {
			u32 track = M4_GetTrackByID(load->isom, esd->dependsOnESID);
			if (M4_GetMediaType(load->isom, track) != M4_ODMediaType) {
				OD_DeleteDescriptor((Descriptor **)&esd);
				continue;
			}
		}
		break;
	}
	if (!esd) return M4OK;

	bdec = BIFS_NewDecoder(load->scene_graph, 1);
	track = i+1;

	sc = M4SM_NewStream(load->ctx, esd->ESID, esd->decoderConfig->streamType, esd->decoderConfig->objectTypeIndication);
	sc->streamType = esd->decoderConfig->streamType;
	sc->ESID = esd->ESID;
	sc->objectType = esd->decoderConfig->objectTypeIndication;
	sc->timeScale = M4_GetMediaTimeScale(load->isom, track);
	e = BIFS_ConfigureStream(bdec, esd->ESID, esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, esd->decoderConfig->objectTypeIndication);
	if (e) goto exit;
	timeScale = M4_GetMediaTimeScale(load->isom, track);
	/*get first AU*/
	samp = M4_GetSample(load->isom, track, 1, &di);
	if (!samp) {
		e = M4_GetLastError(load->isom);
		goto exit;
	}
	au = M4SM_NewAU(sc, samp->DTS, ((Double)samp->DTS) / timeScale, samp->IsRAP);
	e = BIFS_DecodeAUMemory(bdec, esd->ESID, samp->data, samp->dataLength, au->commands);
	if (!e) {
		for (i=0; i<ChainGetCount(au->commands); i++) {
			SGCommand *com = ChainGetEntry(au->commands, i);
			if (com->tag == SG_SceneReplace) {
				BIFSConfigDescriptor bc;
				/*also update size & pixel metrics info*/
				OD_GetBIFSConfig(esd->decoderConfig->decoderSpecificInfo, esd->decoderConfig->objectTypeIndication, &bc);
				if (bc.isCommandStream && bc.pixelWidth && bc.pixelHeight) {
					load->ctx->scene_width = bc.pixelWidth;
					load->ctx->scene_height = bc.pixelHeight;
					load->ctx->is_pixel_metrics = bc.pixelMetrics;
				}
			}
		}
	}
	M4_DeleteSample(&samp);
	OD_DeleteDescriptor((Descriptor **) &esd);

exit:
	BIFS_DeleteDecoder(bdec);
	if (esd) OD_DeleteDescriptor((Descriptor **) &esd);
	return e;
}

void M4SM_LoaderDone_MP4(M4ContextLoader *load)
{
	/*nothing to do the file is not ours*/
}

#endif

/*initializes the context loader*/
M4Err M4SM_LoaderInit(M4ContextLoader *load)
{
	char *ext, szExt[50];
	/*we need at least a scene graph*/
	if (!load || (!load->ctx && !load->scene_graph) || (!load->fileName && !load->isom)) return M4BadParam;

	if (!load->type) {
		if (load->isom) {
			load->type = M4CL_MP4;
		} else {
			ext = strrchr(load->fileName, '.');
			if (!ext) return M4NotSupported;
			if (stricmp(ext, ".gz")) ext = strrchr(ext, '.');
			strcpy(szExt, &ext[1]);
			strlwr(szExt);
			if (strstr(szExt, "bt")) load->type = M4CL_BT;
			else if (strstr(szExt, "wrl")) load->type = M4CL_VRML;
			else if (strstr(szExt, "x3dv")) load->type = M4CL_X3DV;
			else if (strstr(szExt, "xmt") || strstr(szExt, "xmta")) load->type = M4CL_XMTA;
			else if (strstr(szExt, "x3d")) load->type = M4CL_X3D;
			else if (strstr(szExt, "swf")) load->type = M4CL_SWF;
		}
	}
	if (!load->type) return M4NotSupported;

	if (!load->scene_graph) load->scene_graph = load->ctx->scene_graph;

	switch (load->type) {
	case M4CL_BT: 
	case M4CL_VRML:
	case M4CL_X3DV:
		return M4SM_LoaderInit_BT(load);
	case M4CL_XMTA:
	case M4CL_X3D:
		return M4SM_LoaderInit_XMT(load);
	case M4CL_SWF: 
		return M4SM_LoaderInit_SWF(load);
#ifndef M4_READ_ONLY
	case M4CL_MP4:
		return M4SM_LoaderInit_MP4(load);
#endif
	}
	return M4NotSupported;
}

void M4SM_LoaderDone(M4ContextLoader *load)
{
	switch (load->type) {
	case M4CL_BT:
	case M4CL_VRML:
	case M4CL_X3DV:
		M4SM_LoaderDone_BT(load); break;
	case M4CL_XMTA:
	case M4CL_X3D:
		M4SM_LoaderDone_XMT(load); break;
	case M4CL_SWF: M4SM_LoaderDone_SWF(load); break;
#ifndef M4_READ_ONLY
	case M4CL_MP4: M4SM_LoaderDone_MP4(load); break;
#endif
	}
}

M4Err M4SM_LoaderRun(M4ContextLoader *load)
{
	switch (load->type) {
	case M4CL_BT:
	case M4CL_VRML:
	case M4CL_X3DV:
		return M4SM_LoaderRun_BT(load);
	case M4CL_XMTA:
	case M4CL_X3D:
		return M4SM_LoaderRun_XMT(load);
	case M4CL_SWF:
		return M4SM_LoaderRun_SWF(load);
#ifndef M4_READ_ONLY
	case M4CL_MP4: return M4SM_LoaderRun_MP4(load);
#endif
	default:
		return M4BadParam;
	}
}
