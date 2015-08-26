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

#ifndef M4_READ_ONLY


Bool MP4T_CanEmbbedData(char *data, u32 data_size, u32 streamType)
{
	char data64[5000];
	u32 size64;

	size64 = Base64_enc(data, data_size, data64, 5000);
	if (!size64) return 0;
	switch (streamType) {
	case M4ST_OD:
		size64 += strlen("data:application/mpeg4-od-au;base64,");
		break;
	case M4ST_SCENE:
		size64 += strlen("data:application/mpeg4-bifs-au;base64,");
		break;
	default:
		/*NOT NORMATIVE*/
		size64 += strlen("data:application/mpeg4-es-au;base64,");
		break;
	}
	if (size64>=255) return 0;
	return 1;
}

void MP4T_DumpSDP(M4File *file, const char *name)
{
	const char *sdp;
	u32 size, i;
	FILE *f;

	f = fopen(name, "wt");
	//get the movie SDP
	M4H_SDP_GetSDP(file, &sdp, &size);
	fwrite(sdp, size, 1, f);
	fprintf(f, "\r\n");

	//then tracks
	for (i=0; i<M4_GetTrackCount(file); i++) {
		if (M4_GetMediaType(file, i+1) != M4_HintMediaType) continue;
		M4H_SDP_GetTrackSDP(file, i+1, &sdp, &size);
		fwrite(sdp, size, 1, f);
	}
	fclose(f);
}


M4Err MP4T_CopyTrack(M4File *infile, u32 inTrackNum, M4File *outfile, Bool ResetDependancies, Bool AddToIOD)
{
	ESDescriptor *esd;
	UnknownStreamDescription *udesc;
	LPH263CONFIGURATION h263cfg;
	LPAMRCONFIGURATION amrcfg;
	InitialObjectDescriptor *iod;
	u32 TrackID, newTk, descIndex, i, ts, rate, dur, pos;
	M4Sample *samp;

	if (!inTrackNum) {
		if (M4_GetTrackCount(infile) != 1) return M4BadParam;
		inTrackNum = 1;
	}
	//check the ID is available
	TrackID = M4_GetTrackID(infile, inTrackNum);
	newTk = M4_GetTrackByID(outfile, TrackID);
	if (newTk) TrackID = 0;

	//get the ESD and remove dependancies
	udesc = NULL;
	amrcfg = NULL;
	h263cfg = NULL;
	esd = NULL;
	switch (M4_GetMediaSubType(infile, inTrackNum, 1)) {
	case M4_MPEG4_SubType:
		esd = M4_GetStreamDescriptor(infile, inTrackNum, 1);
		if (esd && ResetDependancies) {
			esd->dependsOnESID = 0;
			esd->OCRESID = 0;
		}
		break;
	case M4_AMR_SubType:
	case M4_AMR_WB_SubType:
		amrcfg = M4_AMR_GetStreamConfig(infile, inTrackNum, 1);
		break;
	case M4_H263_SubType:
		h263cfg = M4_H263_GetStreamConfig(infile, inTrackNum, 1);
		break;
	default:
		udesc = M4_GetUnknownStreamDescription(infile, inTrackNum, 1);
		if (!udesc) return M4InvalidMP4File;
		break;
	}

	newTk = M4_NewTrack(outfile, TrackID, M4_GetMediaType(infile, inTrackNum), M4_GetMediaTimeScale(infile, inTrackNum));
	M4_SetTrackEnabled(outfile, newTk, 1);

	if (esd) {
		M4_NewStreamDescription(outfile, newTk, esd, NULL, NULL, &descIndex);

		if ((esd->decoderConfig->streamType == M4ST_VISUAL) || (esd->decoderConfig->streamType == M4ST_SCENE)) {
			u32 w, h;
			M4_GetVisualEntrySize(infile, inTrackNum, 1, &w, &h);
			/*this is because so many files have reserved values of 320x240 from v1 ... */
			if ((esd->decoderConfig->objectTypeIndication == 0x20) ) {
				M4VDecoderSpecificInfo dsi;
				M4V_GetConfig(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, &dsi);
				w = dsi.width;
				h = dsi.height;
			}
			M4_SetVisualEntrySize(outfile, newTk, 1, w, h);
		}
		esd->decoderConfig->avgBitrate = 0;
		esd->decoderConfig->maxBitrate = 0;
	} else if (amrcfg) {
	    M4_AMR_NewStreamConfig(outfile, newTk, amrcfg, NULL, NULL, &descIndex);
	    free(amrcfg);
	} else if (h263cfg) {
		u32 w, h;
	    M4_H263_NewStreamConfig(outfile, newTk, h263cfg, NULL, NULL, &descIndex);
		M4_GetVisualEntrySize(infile, inTrackNum, 1, &w, &h);
		M4_SetVisualEntrySize(outfile, newTk, 1, w, h);
		free(h263cfg);
	} else {
	  M4_NewUnknownStreamDescription(outfile, newTk, udesc->codec_tag, NULL, NULL, NULL, udesc, &descIndex);
	  free(udesc);
	}

	pos = 0;
	rate = 0;
	ts = M4_GetMediaTimeScale(infile, inTrackNum);

	for (i=0; i<M4_GetSampleCount(infile, inTrackNum); i++) {
		samp = M4_GetSample(infile, inTrackNum, i+1, &descIndex);
		M4_AddSample(outfile, newTk, 1, samp);
		if (esd) {
		rate += samp->dataLength;
		esd->decoderConfig->avgBitrate += samp->dataLength;
		if (esd->decoderConfig->bufferSizeDB<samp->dataLength) esd->decoderConfig->bufferSizeDB = samp->dataLength;
		if (samp->DTS - pos > ts) {
			if (esd->decoderConfig->maxBitrate<rate) esd->decoderConfig->maxBitrate = rate;
			rate = 0;
			pos = 0;
		}
		}
		M4_DeleteSample(&samp);
	}
	/*likely 3gp or any non-MPEG-4 isomedia file*/
	if (!esd) return M4_RemoveRootOD(outfile);

	dur = (u32) M4_GetMediaDuration(outfile, newTk);
	if (!dur) dur = ts;
	esd->decoderConfig->maxBitrate *= 8;
	esd->decoderConfig->avgBitrate *= 8 * ts / dur;
	M4_ChangeStreamDescriptor(outfile, newTk, 1, esd);


	iod = (InitialObjectDescriptor *) M4_GetRootOD(infile);
	switch (esd->decoderConfig->streamType) {
	case M4ST_SCENE:
		if (iod && (iod->tag==InitialObjectDescriptor_Tag)) {
			M4_SetMoviePLIndication(outfile, M4_PL_SCENE, iod->scene_profileAndLevel);
			M4_SetMoviePLIndication(outfile, M4_PL_GRAPHICS, iod->graphics_profileAndLevel);
		} else if (esd->decoderConfig->objectTypeIndication==0x20) {
			fprintf(stdout, "Warning: Scene PLs not found in original MP4 - defaulting to No Profile Specified\n");
			M4_SetMoviePLIndication(outfile, M4_PL_SCENE, 0xFE);
			M4_SetMoviePLIndication(outfile, M4_PL_GRAPHICS, 0xFE);
		}
		break;
	case M4ST_VISUAL:
		if (iod && (iod->tag==InitialObjectDescriptor_Tag)) {
			M4_SetMoviePLIndication(outfile, M4_PL_VISUAL, iod->visual_profileAndLevel);
		} else if (esd->decoderConfig->objectTypeIndication==0x20) {
			M4VDecoderSpecificInfo dsi;
			M4V_GetConfig(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, &dsi);
			M4_SetMoviePLIndication(outfile, M4_PL_VISUAL, dsi.VideoPL);
		} else {
			fprintf(stdout, "Warning: Visual PLs not found in original MP4 - defaulting to No Profile Specified\n");
			M4_SetMoviePLIndication(outfile, M4_PL_VISUAL, 0xFE);
		}
		break;
	case M4ST_AUDIO:
		if (iod && (iod->tag==InitialObjectDescriptor_Tag)) {
			M4_SetMoviePLIndication(outfile, M4_PL_AUDIO, iod->audio_profileAndLevel);
		} else if (esd->decoderConfig->objectTypeIndication==0x40) {
			M4ADecoderSpecificInfo cfg;
			M4A_GetConfig(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, &cfg);
			M4_SetMoviePLIndication(outfile, M4_PL_AUDIO, cfg.audioPL);
		} else {
			fprintf(stdout, "Warning: Audio PLs not found in original MP4 - defaulting to No Profile Specified\n");
			M4_SetMoviePLIndication(outfile, M4_PL_AUDIO, 0xFE);
		}
	default:
		break;
	}
	if (iod) OD_DeleteDescriptor((Descriptor **) &iod);	
	OD_DeleteDescriptor((Descriptor **)&esd);

	if (AddToIOD) M4_AddTrackToRootOD(outfile, newTk);

	return M4OK;
}


static M4Err MP4T_CreateReferenceTrack(M4File *in_file, M4File *out_file, u32 inTrack, char *remotePath, Bool ResetDep)
{
	ESDescriptor *esd;
	SLConfigDescriptor *slc;
	char buffer[5000];
	u32 refCount, mediaTk, newTk, descIndex, mType;

	if (M4_GetMediaType(in_file, inTrack) != M4_HintMediaType) return M4BadParam;
			
	//get reference track (only 1 supported)
	refCount = M4_GetTrackReferenceCount(in_file, inTrack, M4_HintTrack_Ref);
	if (refCount > 1) return M4NotSupported;
	M4_GetTrackReference(in_file, inTrack, M4_HintTrack_Ref, 1, &mediaTk);

	mType = 0;
	switch (M4_GetMediaType(in_file, mediaTk)) {
	case M4_VisualMediaType:
		mType = 1;
	case M4_AudioMediaType:
		break;
	default:
		return M4NotSupported;
	}
	//get original ESD
	esd = M4_GetStreamDescriptor(in_file, mediaTk, 1);
	if (esd->URLString) {
		OD_DeleteDescriptor((Descriptor **)&esd);
		return M4NotSupported;
	}

	sprintf(buffer, "%s/trackID=%d", remotePath, M4_GetTrackID(in_file, inTrack));

	esd->URLString = strdup(buffer);
	if (ResetDep) {
		esd->dependsOnESID = 0;
		esd->OCRESID = 0;
	}

	//check wether the hinting just happened
	slc = NULL;
	M4_GetExtractionSLConfig(in_file, mediaTk, 1, &slc);
	if (!slc) {
		//reset SLConfig
		esd->slConfig->predefined = 0;
	} else {
		OD_DeleteDescriptor((Descriptor **) &esd->slConfig);
		esd->slConfig = slc;
	}

	newTk = M4_NewTrack(out_file, M4_GetTrackID(in_file, mediaTk), M4_GetMediaType(in_file, mediaTk), M4_GetMediaTimeScale(in_file, mediaTk));
	M4_SetTrackEnabled(out_file, newTk, 1);
	M4_NewStreamDescription(out_file, newTk, esd, NULL, NULL, &descIndex);

	//no samples to add
	OD_DeleteDescriptor((Descriptor **)&esd);
	return M4OK;
}

void MP4T_CheckMediaProfile(M4File *file, u32 track)
{
	u8 PL;
	ESDescriptor *esd = M4_GetStreamDescriptor(file, track, 1);
	if (!esd) return;

	switch (esd->decoderConfig->streamType) {
	case 0x04:
		PL = M4_GetMoviePLIndication(file, M4_PL_VISUAL);
		if (esd->decoderConfig->objectTypeIndication==0x20) {
			M4VDecoderSpecificInfo dsi;
			M4V_GetConfig(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, &dsi);
			if (dsi.VideoPL > PL) M4_SetMoviePLIndication(file, M4_PL_VISUAL, dsi.VideoPL);
		} else if (!PL) {
			M4_SetMoviePLIndication(file, M4_PL_VISUAL, 0xFE);
		}
		break;
	case 0x05:
		PL = M4_GetMoviePLIndication(file, M4_PL_AUDIO);
		if (esd->decoderConfig->objectTypeIndication==0x40) {
			M4ADecoderSpecificInfo dsi;
			M4A_GetConfig(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, &dsi);
			if (dsi.audioPL > PL) M4_SetMoviePLIndication(file, M4_PL_AUDIO, dsi.audioPL);
		} else if (!PL) {
			M4_SetMoviePLIndication(file, M4_PL_AUDIO, 0xFE);
		}
		break;
	}
	OD_DeleteDescriptor((Descriptor **) &esd);
}


M4Err MP4T_CreateReferenceMovie(M4File *in_file, char *ref_name, char *ref_url)
{
	M4Err e;
	u32 i, type;
	u8 odpl, bifspl, vidpl, audpl, graphics;
	M4File *ref_file;
	InitialObjectDescriptor *iod;

	if (!ref_name) return M4BadParam;

	e = M4OK;
	ref_file = M4_MovieOpen(ref_name, M4_WRITE_EDIT);
	if (!ref_file) return M4_GetLastError(NULL);

	graphics = odpl = bifspl = vidpl = audpl = 0;
	for (i=0; i<M4_GetTrackCount(in_file); i++) {
		type = M4_GetMediaType(in_file, i+1);
		switch (type) {
		case M4_HintMediaType:
			e = MP4T_CreateReferenceTrack(in_file, ref_file, i+1, ref_url, 0);
			if (e) goto exit;
			break;
		//do NOT copy AV except image streams (one AU)
		case M4_VisualMediaType:
			if (M4_GetSampleCount(in_file, i+1) == 1) {
				e = MP4T_CopyTrack(in_file, i+1, ref_file, 0, M4_IsTrackInRootOD(in_file, i+1));
				if (e) goto exit;
			} else {
				vidpl=1;
				continue;
			}
		case M4_AudioMediaType:
			audpl=1;
			continue;

		//duplicate all systems streams but keep dependancies
		default:
			e = MP4T_CopyTrack(in_file, i+1, ref_file, 0, M4_IsTrackInRootOD(in_file, i+1));
			if (e) goto exit;
			if (type == M4_BIFSMediaType)
				bifspl = 1;
			else if (type == M4_ODMediaType)
				odpl = 1;
			break;
		}
	}

	//then duplicate all desc in IOD...
	iod = (InitialObjectDescriptor *) M4_GetRootOD(in_file);
	if (!iod) goto exit;
	for (i=0;i<ChainGetCount(iod->IPMPDescriptorPointers); i++) {
		M4_AddDescriptorToRootOD(ref_file, ChainGetEntry(iod->IPMPDescriptorPointers, i));
	}
	for (i=0;i<ChainGetCount(iod->OCIDescriptors); i++) {
		M4_AddDescriptorToRootOD(ref_file, ChainGetEntry(iod->OCIDescriptors, i));
	}
	for (i=0;i<ChainGetCount(iod->extensionDescriptors); i++) {
		M4_AddDescriptorToRootOD(ref_file, ChainGetEntry(iod->extensionDescriptors, i));
	}
	if (iod->tag == InitialObjectDescriptor_Tag) {

		//erase our PLs with the ones indicated in IOD if present
		odpl = odpl ? iod->OD_profileAndLevel : 0xFF;
		bifspl = bifspl ? iod->scene_profileAndLevel : 0xFF;
		vidpl = vidpl ? iod->visual_profileAndLevel : 0xFF;
		audpl = audpl ? iod->audio_profileAndLevel : 0xFF;
		graphics = iod->graphics_profileAndLevel;
	} else {
		//no more indic ...
		graphics = bifspl;	
	}	

	OD_DeleteDescriptor((Descriptor **)&iod);

exit:
	if (!e) {
		M4_SetStorageMode(ref_file, M4_INTERLEAVED);
		
		M4_SetMoviePLIndication(ref_file, M4_PL_AUDIO, audpl);
		M4_SetMoviePLIndication(ref_file, M4_PL_VISUAL, vidpl);
		M4_SetMoviePLIndication(ref_file, M4_PL_OD, odpl);
		M4_SetMoviePLIndication(ref_file, M4_PL_SCENE, bifspl);
		M4_SetMoviePLIndication(ref_file, M4_PL_GRAPHICS, graphics);

		e = M4_MovieClose(ref_file);
	} else {
		M4_MovieDelete(ref_file);
	}
	return e;
}

#endif

