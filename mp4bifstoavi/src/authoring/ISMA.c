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



#include <m4_author.h>
#include <m4_decoder.h>

#ifndef M4_READ_ONLY


void log_message(void (*LogMsg)(const char *szMsg), char *format, ...)
{
	va_list args;
	va_start(args, format);
	if (LogMsg) {
		char szMsg[1024];
		vsprintf(szMsg, format, args);
		LogMsg(szMsg);
	} else {
		vfprintf(stderr,format,args);
		fprintf(stderr, "\n");
	}
	va_end(args);
}

M4Err MP4T_MakeISMA(M4File *mp4file, Bool keepImage, Bool no_ocr, void (*LogMsg)(const char *szMsg))
{
	u32 AudioTrack, VideoTrack, Tracks, i, mType, bifsT, odT, descIndex, VideoType, VID, AID;
	u32 bifs, w, h;
	Bool is_image, image_track;
	ESDescriptor *a_esd, *v_esd, *_esd;
	Bool update_vid_esd;
	ObjectDescriptor *od;
	ObjectDescriptorUpdate *odU;
	LPODCODEC codec;
	M4Sample *samp;
	BitStream *bs;
	u8 audioPL, visualPL;
	
	switch (M4_GetOpenMode(mp4file)) {
	case M4_OPEN_EDIT:
	case M4_OPEN_WRITE:
	case M4_WRITE_EDIT:
		break;
	default:
		return M4BadParam;
	}


	Tracks = M4_GetTrackCount(mp4file);
	AID = VID = 0;
	is_image = 0;

	//search for tracks
	for (i=0; i<Tracks; i++) {
		ESDescriptor *esd = M4_GetStreamDescriptor(mp4file, i+1, 1);
		//remove from IOD
		M4_RemoveTrackFromRootOD(mp4file, i+1);

		mType = M4_GetMediaType(mp4file, i+1);
		switch (mType) {
		case M4_VisualMediaType:
			image_track = 0;
			if (esd && ((esd->decoderConfig->objectTypeIndication==0x6C) || (esd->decoderConfig->objectTypeIndication==0x6D)) )
				image_track = 1;

			/*remove image tracks if wanted*/
			if (keepImage || !image_track) {
				/*only ONE video stream possible with ISMA*/
				if (VID) {
					if (esd) OD_DeleteDescriptor((Descriptor**)&esd);
					log_message(LogMsg, "More than one video track found, cannot ISMA'ize file - remove extra track(s)");
					return M4NotSupported;
				}
				VID = M4_GetTrackID(mp4file, i+1);
				is_image = image_track;
			} else {
				log_message(LogMsg, "Visual track ID %d: only one sample found, assuming image and removing track", M4_GetTrackID(mp4file, i+1) );
				M4_RemoveTrack(mp4file, i+1);
				i -= 1;
				Tracks = M4_GetTrackCount(mp4file);
			}
			break;
		case M4_AudioMediaType:
			if (AID) {
				if (esd) OD_DeleteDescriptor((Descriptor**)&esd);
				log_message(LogMsg, "more than one audio track found, cannot ISMA'ized file - remove extra track(s)");
				return M4NotSupported;
			}
			AID = M4_GetTrackID(mp4file, i+1);
			break;
		/*clean file*/
		default:
			if (mType==M4_HintMediaType) {
				log_message(LogMsg, "Removing Hint track ID %d", M4_GetTrackID(mp4file, i+1) );
			} else {
				log_message(LogMsg, "Removing MPEG-4 Systems track ID %d", M4_GetTrackID(mp4file, i+1) );
			}
			M4_RemoveTrack(mp4file, i+1);
			i -= 1;
			Tracks = M4_GetTrackCount(mp4file);
			break;
		}
		if (esd) OD_DeleteDescriptor((Descriptor**)&esd);
	}
	//no audio no video
	if (!AID && !VID) return M4OK;

	audioPL = visualPL = 1;
	od = (ObjectDescriptor *) M4_GetRootOD(mp4file);
	if (od && (od->tag==InitialObjectDescriptor_Tag)) {
		audioPL = ((InitialObjectDescriptor*)od)->audio_profileAndLevel;
		visualPL = ((InitialObjectDescriptor*)od)->visual_profileAndLevel;
	}
	if (od) OD_DeleteDescriptor((Descriptor **)&od);


	//create the OD AU
	bifs = 0;
	odU = (ObjectDescriptorUpdate *) OD_NewCommand(ODUpdate_Tag);
	
	a_esd = v_esd = NULL;
	update_vid_esd = 0;

	M4_SetRootOD_ID(mp4file, 1);


	VideoTrack = M4_GetTrackByID(mp4file, VID);
	AudioTrack = M4_GetTrackByID(mp4file, AID);
	
	/*reset all PLs*/
	visualPL = 0xFE;
	audioPL = 0xFE;

	w = h = 0;
	if (VideoTrack) {
		bifs = 1;
		VideoType = 0;
		od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
		od->objectDescriptorID = ISMA_VIDEO_OD_ID;

		if (VID != ISMA_VIDEO_ES_ID) {
			M4_ChangeTrackID(mp4file, VideoTrack, ISMA_VIDEO_ES_ID);
		}

		v_esd = M4_GetStreamDescriptor(mp4file, VideoTrack, 1);
		if (v_esd) {
			v_esd->ESID = ISMA_VIDEO_ES_ID;
			v_esd->OCRESID = no_ocr ? 0 : 1;

			OD_AddDescToDesc((Descriptor *)od, (Descriptor *)v_esd);
			ChainAddEntry(odU->objectDescriptors, od);
			M4_GetVisualEntrySize(mp4file, VideoTrack, 1, &w, &h);
			if ((v_esd->decoderConfig->objectTypeIndication==0x20) && (v_esd->decoderConfig->streamType==M4ST_VISUAL)) {
				M4VDecoderSpecificInfo dsi;
				M4V_GetConfig(v_esd->decoderConfig->decoderSpecificInfo->data, v_esd->decoderConfig->decoderSpecificInfo->dataLength, &dsi);
				if (!is_image && (!w || !h)) {
					w = dsi.width;
					h = dsi.height;
					M4_SetVisualEntrySize(mp4file, VideoTrack, 1, w, h);
					log_message(LogMsg, "Adjusting visual track size to %d x %d", w, h);
				}
				if (dsi.VideoPL) visualPL = dsi.VideoPL;
			}
		}
	}

	if (AudioTrack) {
		od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
		od->objectDescriptorID = ISMA_AUDIO_OD_ID;

		if (AID != ISMA_AUDIO_ES_ID) {
			M4_ChangeTrackID(mp4file, AudioTrack, ISMA_AUDIO_ES_ID);
		}

		a_esd = M4_GetStreamDescriptor(mp4file, AudioTrack, 1);
		if (a_esd) {
			a_esd->OCRESID = no_ocr ? 0 : 1;

			a_esd->ESID = ISMA_AUDIO_ES_ID;
			OD_AddDescToDesc((Descriptor *)od, (Descriptor *)a_esd);
			ChainAddEntry(odU->objectDescriptors, od);
			if (!bifs) {
				bifs = 3;
			} else {
				bifs = 2;
			}

			if (a_esd->decoderConfig->objectTypeIndication == 0x40) {
				M4ADecoderSpecificInfo cfg;
				M4A_GetConfig(a_esd->decoderConfig->decoderSpecificInfo->data, a_esd->decoderConfig->decoderSpecificInfo->dataLength, &cfg);
				audioPL = cfg.audioPL;
			}
		}
	}

	/*update video cfg if needed*/
	if (v_esd) M4_ChangeStreamDescriptor(mp4file, VideoTrack, 1, v_esd);
	if (a_esd) M4_ChangeStreamDescriptor(mp4file, AudioTrack, 1, a_esd);

	/*likely 3GP or other files...*/
	if ((!a_esd && AudioTrack) || (!v_esd && VideoTrack)) return M4OK;

	//get the OD sample
	codec = OD_NewCodec(OD_WRITE);
	samp = M4_NewSample();
	OD_AddCommand(codec, (ODCommand *)odU);
	OD_EncodeAU(codec);
	OD_GetEncodedAU(codec, &samp->data, &samp->dataLength);
	OD_DeleteCodec(codec);
	samp->CTS_Offset = 0;
	samp->DTS = 0;
	samp->IsRAP = 1;

	
	/*create the OD track*/
	odT = M4_NewTrack(mp4file, 2, M4_ODMediaType, M4_GetTimeScale(mp4file));
	if (!odT) return M4_GetLastError(mp4file);

	_esd = OD_NewESDescriptor(SLPredef_MP4);
	_esd->decoderConfig->bufferSizeDB = samp->dataLength;
	_esd->decoderConfig->objectTypeIndication = 0x01;
	_esd->decoderConfig->streamType = M4ST_OD;
	_esd->ESID = 2;
	_esd->OCRESID = no_ocr ? 0 : 1;
	M4_NewStreamDescription(mp4file, odT, _esd, NULL, NULL, &descIndex);
	OD_DeleteDescriptor((Descriptor **)&_esd);
	M4_AddSample(mp4file, odT, 1, samp);
	M4_DeleteSample(&samp);

	M4_SetTrackGroup(mp4file, odT, ISMA_SYS_GROUP_ID);

	/*create the BIFS track*/
	bifsT = M4_NewTrack(mp4file, 1, M4_BIFSMediaType, M4_GetTimeScale(mp4file));
	if (!bifsT) return M4_GetLastError(mp4file);

	_esd = OD_NewESDescriptor(SLPredef_MP4);
	_esd->decoderConfig->bufferSizeDB = sizeof(ISMA_BIFS_CONFIG);
	_esd->decoderConfig->objectTypeIndication = 0x02;
	_esd->decoderConfig->streamType = M4ST_SCENE;
	_esd->ESID = 1;
	_esd->OCRESID = 0;

	/*rewrite ISMA BIFS cfg*/
	bs = NewBitStream(NULL, 0, BS_WRITE);
	/*empty bifs stuff*/
	BS_WriteInt(bs, 0, 17);
	/*command stream*/
	BS_WriteInt(bs, 1, 1);
	/*in pixel metrics*/
	BS_WriteInt(bs, 1, 1);
	/*with size*/
	BS_WriteInt(bs, 1, 1);
	BS_WriteInt(bs, w, 16);
	BS_WriteInt(bs, h, 16);
	BS_Align(bs);
	BS_GetContent(bs, (unsigned char **) &_esd->decoderConfig->decoderSpecificInfo->data, &_esd->decoderConfig->decoderSpecificInfo->dataLength);
	M4_NewStreamDescription(mp4file, bifsT, _esd, NULL, NULL, &descIndex);
	OD_DeleteDescriptor((Descriptor **)&_esd);
	DeleteBitStream(bs);
	
	samp = M4_NewSample();
	samp->CTS_Offset = 0;
	samp->DTS = 0;
	switch (bifs) {
	case 1:
		if (is_image) {
			samp->data = (char *) ISMA_BIFS_IMAGE;
			samp->dataLength = 10;
		} else {
			samp->data = (char *) ISMA_BIFS_VIDEO;
			samp->dataLength = 11;
		}
		break;
	case 2:
		if (is_image) {
			samp->data = (char *) ISMA_BIFS_AI;
			samp->dataLength = 15;
		} else {
			samp->data = (char *) ISMA_BIFS_AV;
			samp->dataLength = 16;
		}
		break;
	case 3:
		samp->data = (char *) ISMA_BIFS_AUDIO;
		samp->dataLength = 8;
		break;
	}

	samp->IsRAP = 1;

	M4_AddSample(mp4file, bifsT, 1, samp);
	samp->data = NULL;
	M4_DeleteSample(&samp);
	M4_SetTrackGroup(mp4file, bifsT, ISMA_SYS_GROUP_ID);

	M4_SetTrackEnabled(mp4file, bifsT, 1);
	M4_SetTrackEnabled(mp4file, odT, 1);
	M4_AddTrackToRootOD(mp4file, bifsT);
	M4_AddTrackToRootOD(mp4file, odT);

	M4_SetMoviePLIndication(mp4file, M4_PL_SCENE, 1);
	M4_SetMoviePLIndication(mp4file, M4_PL_GRAPHICS, 1);
	M4_SetMoviePLIndication(mp4file, M4_PL_OD, 1);
	M4_SetMoviePLIndication(mp4file, M4_PL_AUDIO, audioPL);
	M4_SetMoviePLIndication(mp4file, M4_PL_VISUAL, (u8) (is_image ? 0xFE : visualPL));
	return M4OK;
}


M4Err MP4T_Make3GPP(M4File *mp4file, void (*LogMsg)(const char *szMsg))
{
	u32 Tracks, AID, VID, i, mType;

	switch (M4_GetOpenMode(mp4file)) {
	case M4_OPEN_EDIT:
	case M4_OPEN_WRITE:
	case M4_WRITE_EDIT:
		break;
	default:
		return M4BadParam;
	}

	Tracks = M4_GetTrackCount(mp4file);
	AID = VID = 0;

	for (i=0; i<Tracks; i++) {
		M4_RemoveTrackFromRootOD(mp4file, i+1);

		mType = M4_GetMediaType(mp4file, i+1);
		switch (mType) {
		case M4_VisualMediaType:
			/*remove image tracks if wanted*/
			if (M4_GetSampleCount(mp4file, i+1)<=1) {
				log_message(LogMsg, "Visual track ID %d: only one sample found, assuming image and removing track\n", M4_GetTrackID(mp4file, i+1) );
				goto remove_track;
			}
			/*only ONE video stream possible with ISMA*/
			if (VID) {
				log_message(LogMsg, "More than one video track found, cannot 3GP'ize file\n");
				return M4NotSupported;
			}
			switch (M4_GetMediaSubType(mp4file, i+1, 1)) {
			case M4_H263_SubType:
				VID = M4_GetTrackID(mp4file, i+1);
				break;
			case M4_MPEG4_SubType:
				{
					ESDescriptor *esd = M4_GetStreamDescriptor(mp4file, i+1, 1);
					if (esd->decoderConfig->objectTypeIndication==0x20) {
						VID = M4_GetTrackID(mp4file, i+1);
					} else {
						log_message(LogMsg, "Video format not supported by 3GP - removing track ID %d\n", M4_GetTrackID(mp4file, i+1) );
						goto remove_track;
					}
					OD_DeleteDescriptor((Descriptor **)&esd);
				}
				break;
			default:
				log_message(LogMsg, "Video format not supported by 3GP - removing track ID %d\n", M4_GetTrackID(mp4file, i+1) );
				goto remove_track;
			}
			break;
		case M4_AudioMediaType:
			if (AID) {
				log_message(LogMsg, "more than one audio track found, cannot 3GP'ize file\n");
				return M4NotSupported;
			}
			switch (M4_GetMediaSubType(mp4file, i+1, 1)) {
			case M4_AMR_SubType:
			case M4_AMR_WB_SubType:
				AID = M4_GetTrackID(mp4file, i+1);
				break;
			case M4_MPEG4_SubType:
				{
					ESDescriptor *esd = M4_GetStreamDescriptor(mp4file, i+1, 1);
					if (esd->decoderConfig->objectTypeIndication==0x40) {
						AID = M4_GetTrackID(mp4file, i+1);
					} else {
						log_message(LogMsg, "Audio format not supported by 3GP - removing track ID %d\n", M4_GetTrackID(mp4file, i+1) );
						goto remove_track;
					}
					OD_DeleteDescriptor((Descriptor **)&esd);
				}
				break;
			default:
				log_message(LogMsg, "Audio format not supported by 3GP - removing track ID %d\n", M4_GetTrackID(mp4file, i+1) );
				goto remove_track;
			}
			break;
		/*clean file*/
		default:
			if (mType==M4_HintMediaType) {
				log_message(LogMsg, "Removing Hint track ID %d\n", M4_GetTrackID(mp4file, i+1) );
			} else {
				log_message(LogMsg, "Removing system track ID %d\n", M4_GetTrackID(mp4file, i+1) );
			}

remove_track:
			M4_RemoveTrack(mp4file, i+1);
			i -= 1;
			Tracks = M4_GetTrackCount(mp4file);
			break;
		}
	}
	/*no audio no video*/
	if (!AID && !VID) return M4OK;

	/*no more IOD*/
	M4_RemoveRootOD(mp4file);
	/*update FType*/
	M4_SetMovieVersionInfo(mp4file, GPP_V5_File, 1);
	M4_ModifyAlternateBrand(mp4file, MP4_V1_File, 0);
	M4_ModifyAlternateBrand(mp4file, MP4_V2_File, 0);
	M4_ModifyAlternateBrand(mp4file, ISO_Media_File, 1);
	return M4OK;
}


M4Err MP4T_MakeInterleaved(M4File *mp4file, Float TimeInSec)
{
	M4Err e;
	if (M4_GetOpenMode(mp4file) < M4_OPEN_EDIT) return M4BadParam;
	e = M4_SetStorageMode(mp4file, M4_INTERLEAVED);
	if (e) return e;
	return M4_SetInterleavingTime(mp4file, (u32) (TimeInSec * M4_GetTimeScale(mp4file)));
}

typedef struct
{
	u32 TrackID;
	u32 SampleNum, SampleCount;
	u32 FragmentLength;
	u32 OriginalTrack;
	u32 TimeScale, MediaType, DefaultDuration;
} TrackFragmenter;


M4Err MP4T_FragmentMovie(M4File *input, char *output_file, Float max_duration)
{
	u8 NbBits;
	u32 i, TrackNum, descIndex, j, count;
	u32 defaultDuration, defaultSize, defaultDescriptionIndex, defaultRandomAccess;
	u8 defaultPadding;
	u16 defaultDegradationPriority;
	ESDescriptor *esd;
	M4File *output;
	M4Sample *sample, *next;
	Chain *fragmenters;
	u32 MaxFragmentDuration;
	TrackFragmenter *tf;

	//create output file
	output = M4_MovieOpen(output_file, M4_OPEN_WRITE);

	M4_SetMovieVersionInfo(output, MP4_V2_File, 0);
	M4_ModifyAlternateBrand(output, ISO_Media_File, 1);

	MaxFragmentDuration = (u32) (max_duration * M4_GetTimeScale(input));
	fragmenters = NewChain();

	//duplicates all tracks
	for (i=0; i<M4_GetTrackCount(input); i++) {
		esd = M4_GetStreamDescriptor(input, i+1, 1);
		TrackNum = M4_NewTrack(output, M4_GetTrackID(input, i+1), M4_GetMediaType(input, i+1), M4_GetMediaTimeScale(input, i+1));
		M4_NewStreamDescription(output, TrackNum, esd, NULL, NULL, &descIndex);

		OD_DeleteDescriptor((Descriptor **) &esd);


		//if few samples don't fragment track
		count = M4_GetSampleCount(input, i+1);
		if (count<=2) {
			for (j=0; j<count; j++) {
				sample = M4_GetSample(input, i+1, j+1, &descIndex);
				M4_AddSample(output, TrackNum, 1, sample);
				M4_DeleteSample(&sample);
			}
		}
		//otherwise setup fragmented
		else {
			M4_GetTrackSampleDefaults(input, i+1, 
										 &defaultDuration, &defaultSize, &defaultDescriptionIndex, &defaultRandomAccess, &defaultPadding, &defaultDegradationPriority);
			//otherwise setup fragmentation
			M4_SetupTrackFragment(output, M4_GetTrackID(output, TrackNum), 
						defaultDescriptionIndex, defaultDuration, 
						defaultSize, (u8) defaultRandomAccess, 
						defaultPadding, defaultDegradationPriority);

			tf = malloc(sizeof(TrackFragmenter));
			memset(tf, 0, sizeof(TrackFragmenter));
			tf->TrackID = M4_GetTrackID(output, TrackNum);
			tf->SampleCount = M4_GetSampleCount(input, i+1);
			tf->OriginalTrack = i+1;
			tf->TimeScale = M4_GetMediaTimeScale(input, i+1);
			tf->MediaType = M4_GetMediaType(input, i+1);
			tf->DefaultDuration = defaultDuration;
			ChainAddEntry(fragmenters, tf);
		}

		if (M4_IsTrackInRootOD(input, i+1)) M4_AddTrackToRootOD(output, TrackNum);
		//copy user data ?
	}
	//copy movie desc


	//flush movie
	M4_FinalizeMovieForFragments(output);


	while ( (count = ChainGetCount(fragmenters)) ) {

		M4_StartFragment(output);
		//setup some default
		for (i=0; i<count; i++) {
			tf = ChainGetEntry(fragmenters, i);
			if (tf->MediaType == M4_VisualMediaType) {
				M4_TrackFragmentSetOptions(output, tf->TrackID, TFO_IFRAME, 1);
			}
		}	
		sample = NULL;
		
		
		//process track by track
		for (i=0; i<count; i++) {
			tf = ChainGetEntry(fragmenters, i);

			//ok write samples
			while (1) {
				if (!sample) {
					sample = M4_GetSample(input, tf->OriginalTrack, tf->SampleNum + 1, &descIndex);
				}
				M4_GetSamplePaddingBits(input, tf->OriginalTrack, tf->SampleNum+1, &NbBits);

				next = M4_GetSample(input, tf->OriginalTrack, tf->SampleNum + 2, &j);
				if (next) {
					defaultDuration = next->DTS - sample->DTS;
				} else {
					defaultDuration = tf->DefaultDuration;
				}

				M4_TrackFragmentAddSample(output, tf->TrackID, sample, descIndex, 
								 defaultDuration, NbBits, 0);

				M4_DeleteSample(&sample);
				sample = next;
				tf->FragmentLength += defaultDuration;
				tf->SampleNum += 1;

				//end of track fragment or track
				if ((tf->SampleNum==tf->SampleCount) || (tf->FragmentLength*1000 > MaxFragmentDuration*tf->TimeScale)) {
					M4_DeleteSample(&next);
					sample = next = NULL;
					tf->FragmentLength = 0;
					break;
				}
			}
			if (tf->SampleNum==tf->SampleCount) {
				free(tf);
				ChainDeleteEntry(fragmenters, i);
				i--;
				count --;
			}
		}
	}
	DeleteChain(fragmenters);


	M4_MovieClose(output);
	return M4OK;
}

#endif //M4_READ_ONLY


ESDescriptor *MP4T_MapESDescriptor(M4File *mp4, u32 track)
{
	u32 type;
	UnknownStreamDescription *udesc;
	BitStream *bs;
	ESDescriptor *esd;

	u32 subtype = M4_GetMediaSubType(mp4, track, 1);
	if (subtype == M4_MPEG4_SubType)
		return M4_GetStreamDescriptor(mp4, track, 1);

	if (subtype == M4_AVC_H264_SubType)
		return M4_GetStreamDescriptor(mp4, track, 1);

	if (M4_GetMediaType(mp4, track) == M4_TimedTextMediaType)
		return M4_GetStreamDescriptor(mp4, track, 1);

	if ((subtype == M4_AMR_SubType) || (subtype == M4_AMR_WB_SubType)) {
		esd = OD_NewESDescriptor(0);
		esd->slConfig->timestampResolution = M4_GetMediaTimeScale(mp4, track);
		esd->ESID = M4_GetTrackID(mp4, track);
		esd->OCRESID = esd->ESID;
		esd->decoderConfig->streamType = M4ST_AUDIO;
		/*use private DSI*/
		esd->decoderConfig->objectTypeIndication = GPAC_QT_CODECS_OTI;
		bs = NewBitStream(NULL, 0, BS_WRITE);
		/*format ext*/
		BS_WriteInt(bs, FOUR_CHAR_INT('a', 'm', 'r', ' '), 32);
		/*ignore the rest*/
		BS_WriteInt(bs, 0, 4*32);
		BS_GetContent(bs, (unsigned char **) &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
		DeleteBitStream(bs);
		return esd;
	}

	if (subtype == M4_H263_SubType) {
		u32 w, h;
		esd = OD_NewESDescriptor(0);
		esd->slConfig->timestampResolution = M4_GetMediaTimeScale(mp4, track);
		esd->ESID = M4_GetTrackID(mp4, track);
		esd->OCRESID = esd->ESID;
		esd->decoderConfig->streamType = M4ST_VISUAL;
		/*use private DSI*/
		esd->decoderConfig->objectTypeIndication = GPAC_QT_CODECS_OTI;
		bs = NewBitStream(NULL, 0, BS_WRITE);
		/*format ext*/
		BS_WriteInt(bs, FOUR_CHAR_INT('h', '2', '6', '3'), 32);
		M4_GetVisualEntrySize(mp4, track, 1, &w, &h);
		BS_WriteInt(bs, w, 32);
		BS_WriteInt(bs, h, 32);
		BS_GetContent(bs, (unsigned char **) &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
		DeleteBitStream(bs);
		return esd;
	}

	type = M4_GetMediaType(mp4, track);
	if ((type != M4_AudioMediaType) && (type != M4_VisualMediaType)) return NULL;

	esd = OD_NewESDescriptor(0);
	esd->OCRESID = esd->ESID = M4_GetTrackID(mp4, track);
	esd->slConfig->useTimestampsFlag = 1;
	esd->slConfig->timestampResolution = M4_GetMediaTimeScale(mp4, track);
	esd->decoderConfig->objectTypeIndication = GPAC_QT_CODECS_OTI;
	/*format ext*/
	bs = NewBitStream(NULL, 0, BS_WRITE);
	BS_WriteInt(bs, subtype, 32);
	udesc = M4_GetUnknownStreamDescription(mp4, track, 1);
	if (type==M4_AudioMediaType) {
		esd->decoderConfig->streamType = M4ST_AUDIO;
		BS_WriteInt(bs, udesc->SampleRate, 32);
		BS_WriteInt(bs, udesc->NumChannels, 32);
		BS_WriteInt(bs, udesc->bitsPerSample, 32);
		BS_WriteInt(bs, 0, 32);
	} else {
		esd->decoderConfig->streamType = M4ST_VISUAL;
		BS_WriteInt(bs, udesc->width, 32);
		BS_WriteInt(bs, udesc->height, 32);
	}
	if (udesc && udesc->extension_buf_size) {
		BS_WriteData(bs, udesc->extension_buf, udesc->extension_buf_size);
		free(udesc->extension_buf);
	}
	if (udesc) free(udesc);
	BS_GetContent(bs, (unsigned char **) &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
	DeleteBitStream(bs);
	return esd;
}

