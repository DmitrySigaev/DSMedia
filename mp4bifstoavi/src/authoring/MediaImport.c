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
#include <intern/avilib.h>
#include <m4_ogg.h>

#ifndef M4_READ_ONLY


M4Err import_message(M4TrackImporter *import, M4Err e, char *format, ...)
{
	va_list args;
	va_start(args, format);
	if (import->import_message) {
		char szMsg[1024];
		vsprintf(szMsg, format, args);
		import->import_message(import, e, szMsg);
	} else {
		vfprintf(stderr,format,args);
		if (e) fprintf(stderr, " Error: %s", M4ErrToString(e));
		fprintf(stderr, "\n");
	}
	va_end(args);
	return e;
}

void import_progress(M4TrackImporter *import, u32 cur_samp, u32 count)
{
	if (import->import_progress) {
		import->import_progress(import, cur_samp, count);
	} else {
		fprintf(stdout, "Importing sample %d / %d (%.2f)\r", cur_samp, count, ((Float)cur_samp)*100 / count);
		if (cur_samp==count) fprintf(stderr, "\n");
	}
}


void MP4T_GetImageSize(BitStream *bs, u8 OTI, u32 *width, u32 *height)
{
	u64 pos;
	pos = BS_GetPosition(bs);
	BS_Seek(bs, 0);
	
	*width = *height = 0;
	switch (OTI) {
	case 0x6C:
		/*check for SOI marker and JFIF marker*/
		if (BS_ReadInt(bs, 8) != 0xFF) goto exit;
		if (BS_ReadInt(bs, 8) != 0xD8) goto exit;
		if (BS_ReadInt(bs, 8) != 0xFF) goto exit;
		BS_ReadInt(bs, 8);
		/*get frame header FFC0*/
		while (BS_Available(bs)) {
			u32 type, w, h;
			if (BS_ReadInt(bs, 8) != 0xFF) continue;
			type = BS_ReadInt(bs, 8);
			switch (type) {
			case 0xC0:
			case 0xC1:
			case 0xC2:
				BS_SkipBytes(bs, 3);
				h = BS_ReadInt(bs, 16);
				w = BS_ReadInt(bs, 16);
				if ((w > *width) || (h > *height)) {
					*width = w;
					*height = h;
				}
				break;
			case 0xD0:
			case 0xD1:
			case 0xD2:
			case 0xD3:
			case 0xD4:
			case 0xD5:
			case 0xD6:
			case 0xD7:
				break;
			}
		}
		break;
	case 0x6D:
		/*check for PNG sig*/
		if ( (BS_ReadInt(bs, 8) != 0x89) || (BS_ReadInt(bs, 8) != 0x50) || (BS_ReadInt(bs, 8) != 0x4E) 
			|| (BS_ReadInt(bs, 8) != 0x47) || (BS_ReadInt(bs, 8) != 0x0D) || (BS_ReadInt(bs, 8) != 0x0A) 
			|| (BS_ReadInt(bs, 8) != 0x1A) || (BS_ReadInt(bs, 8) != 0x0A) ) goto exit;
		BS_ReadInt(bs, 32);
		/*check for PNG IHDR*/
		if ( (BS_ReadInt(bs, 8) != 'I') || (BS_ReadInt(bs, 8) != 'H') 
			|| (BS_ReadInt(bs, 8) != 'D') || (BS_ReadInt(bs, 8) != 'R')) goto exit;

		*width = BS_ReadInt(bs, 32);
		*height = BS_ReadInt(bs, 32);
		break;
	default:
		break;
	}

exit:
	BS_Seek(bs, pos);
}

static M4Err MP4T_ImportStillImage(M4TrackImporter *import, u8 OTI)
{
	BitStream *bs;
	M4Err e;
	Bool destroy_esd;
	u32 size, track, di, w, h;
	M4Sample *samp;
	ESDescriptor *esd;
	unsigned char *data;

	FILE *src = fopen(import->in_name, "rb");
	if (!src) return import_message(import, M4URLNotFound, "Opening file %s failed", import->in_name);

	fseek(src, 0, SEEK_END);
	size = ftell(src);
	fseek(src, 0, SEEK_SET);
	data = malloc(sizeof(unsigned char)*size);
	fread(data, sizeof(unsigned char)*size, 1, src);
	fclose(src);

	/*get image size*/
	bs = NewBitStream(data, size, BS_READ);
	MP4T_GetImageSize(bs, OTI, &w, &h);
	DeleteBitStream(bs);

	if (!w || !h) {
		free(data);
		return import_message(import, M4NonCompliantBitStream, "Invalid %s file", (OTI==0x6C) ? "JPEG" : "PNG");
	}

	e = M4OK;
	destroy_esd = 0;
	esd = import->esd;
	if (!esd) {
		esd = OD_NewESDescriptor(2);
		destroy_esd = 1;
	}
	/*update stream type/oti*/
	if (!esd->decoderConfig) esd->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
	if (!esd->slConfig) esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
	esd->decoderConfig->streamType = M4ST_VISUAL;
	esd->decoderConfig->objectTypeIndication = OTI;
	esd->decoderConfig->bufferSizeDB = size;
	esd->slConfig->timestampResolution = 1000;
	
	track = M4_NewTrack(import->dest, esd->ESID, M4_VisualMediaType, 1000);
	if (!track) {
		e = M4_GetLastError(import->dest);
		goto exit;
	}
	M4_SetTrackEnabled(import->dest, track, 1);
	if (!esd->ESID) esd->ESID = M4_GetTrackID(import->dest, track);
	e = M4_NewStreamDescription(import->dest, track, esd, (import->flags & M4TI_USE_DATAREF) ? import->in_name : NULL, NULL, &di);
	if (e) goto exit;
	M4_SetVisualEntrySize(import->dest, track, di, w, h);
	samp = M4_NewSample();
	samp->IsRAP = 1;
	samp->dataLength = size;

	import_message(import, M4OK, "%s import - size %d x %d", (OTI==0x6C) ? "JPEG" : "PNG", w, h);

	import_progress(import, 0, 1);
	if (import->flags & M4TI_USE_DATAREF) {
		e = M4_AddSampleReference(import->dest, track, di, samp, (u64) 0);
	} else {
		samp->data = data;
		e = M4_AddSample(import->dest, track, di, samp);
		samp->data = NULL;
	}
	import_progress(import, 1, 1);

	M4_DeleteSample(&samp);

exit:
	free(data);
	if (esd && destroy_esd) OD_DeleteDescriptor((Descriptor **) &esd);
	return e;
}


M4Err MP4T_ImportMP3(M4TrackImporter *import)
{
	u8 oti;
	Bool destroy_esd;
	M4Err e;
	ESDescriptor *esd;
	Double dur;
	u16 sr;
	u32 nb_chan;
	FILE *in;
	u32 hdr, size, max_size, track, di, rate, time_wnd, tot_size, done, offset, duration;
	M4Sample *samp;

	in = fopen(import->in_name, "rb");
	if (!in) return import_message(import, M4URLNotFound, "Opening file %s failed", import->in_name);

	hdr = MP3_GetNextHeader(in);
	if (!hdr) {
		fclose(in);
		return import_message(import, M4NonCompliantBitStream, "Audio isn't MPEG-1/2 audio");
	}
	sr = MP3_GetSamplingRate(hdr);
	oti = MP3_GetObjectTypeIndication(hdr);
	if (!oti) {
		fclose(in);
		return import_message(import, M4NonCompliantBitStream, "Audio isn't MPEG-1/2 audio");
	}

	e = M4OK;
	destroy_esd = 0;
	esd = import->esd;
	if (!esd) {
		esd = OD_NewESDescriptor(2);
		destroy_esd = 1;
	}
	if (!esd->decoderConfig) esd->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
	if (!esd->slConfig) esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
	/*update stream type/oti*/
	esd->decoderConfig->streamType = M4ST_AUDIO;
	esd->decoderConfig->objectTypeIndication = oti;
	esd->decoderConfig->bufferSizeDB = 20;
	esd->slConfig->timestampResolution = sr;

	samp = NULL;
	nb_chan = MP3_GetNumChannels(hdr);
	import_message(import, M4OK, "MP3 import - sample rate %d - %s audio - %d channel%s", sr, (oti==0x6B) ? "MPEG-1" : "MPEG-2", nb_chan, (nb_chan>1) ? "s" : "");

	track = M4_NewTrack(import->dest, esd->ESID, M4_AudioMediaType, sr);
	if (!track) {
		e = M4_GetLastError(import->dest);
		goto exit;
	}
	M4_SetTrackEnabled(import->dest, track, 1);
	if (!esd->ESID) esd->ESID = M4_GetTrackID(import->dest, track);
	if (esd->decoderConfig->decoderSpecificInfo) OD_DeleteDescriptor((Descriptor **) &esd->decoderConfig->decoderSpecificInfo);
	M4_NewStreamDescription(import->dest, track, esd, (import->flags & M4TI_USE_DATAREF) ? import->in_name : NULL, NULL, &di);
	M4_SetAudioDescriptionInfo(import->dest, track, di, sr, nb_chan, 16);

	fseek(in, 0, SEEK_END);
	tot_size = ftell(in);
	fseek(in, 0, SEEK_SET);

	e = M4OK;
	samp = M4_NewSample();
	samp->IsRAP = 1;

	duration = import->duration*sr;
	duration /= 1000;

	max_size = 0;
	rate = 0;
	time_wnd = 0;
	done = 0;
	while (tot_size > done) {
		/* get the next MP3 frame header */
		hdr = MP3_GetNextHeader(in);
		/*MP3 stream truncated*/
		if (!hdr) break;

		offset = ftell(in) - 4;
		size = MP3_GetFrameSize(hdr);
		if (size>max_size) {
			samp->data = realloc(samp->data, sizeof(char) * size);
			max_size = size;
		}
		samp->data[0] = (hdr >> 24) & 0xFF;
		samp->data[1] = (hdr >> 16) & 0xFF;
		samp->data[2] = (hdr >> 8) & 0xFF;
		samp->data[3] = hdr & 0xFF;
		samp->dataLength = size;

		/* read the frame data into the buffer */
		if (fread(&samp->data[4], 1, size - 4, in) != size - 4) break;

		if (import->flags & M4TI_USE_DATAREF) {
			M4_AddSampleReference(import->dest, track, di, samp, offset);
		} else {
			M4_AddSample(import->dest, track, di, samp);
		}

		samp->DTS += MP3_GetSamplesPerFrame(hdr);

		if (esd->decoderConfig->bufferSizeDB < samp->dataLength) esd->decoderConfig->bufferSizeDB = samp->dataLength;
		esd->decoderConfig->avgBitrate += samp->dataLength;
		rate += samp->dataLength;
		if (samp->DTS > time_wnd + sr) {
			if (rate > esd->decoderConfig->maxBitrate) esd->decoderConfig->maxBitrate = rate;
			time_wnd = samp->DTS;
			rate = 0;
		}
		done += samp->dataLength;
		import_progress(import, done, tot_size);
		if (duration && (samp->DTS > duration)) break;
		if (import->flags & M4TI_DO_ABORT) break;
	}
	dur = samp->DTS;
	dur /= sr;
	esd->decoderConfig->avgBitrate = (u32) (esd->decoderConfig->avgBitrate / dur);
	esd->decoderConfig->avgBitrate *= 8;
	esd->decoderConfig->maxBitrate *= 8;
	M4_ChangeStreamDescriptor(import->dest, track, di, esd);

exit:
	if (esd && destroy_esd) OD_DeleteDescriptor((Descriptor **) &esd);
	if (samp) M4_DeleteSample(&samp);
	fclose(in);
	return e;
}

typedef struct
{
	Bool is_mp2, no_crc;
	u32 profile, sr_idx, nb_ch, frame_size;
} ADTSHeader;

static Bool ADTS_SyncFrame(BitStream *bs, ADTSHeader *hdr)
{
	u32 val, hdr_size, pos;
	while (BS_Available(bs)) {
		val = BS_ReadInt(bs, 8);
		if (val!=0xFF) continue;
		val = BS_ReadInt(bs, 4);
		if (val != 0x0F) {
			BS_ReadInt(bs, 4);
			continue;
		}
		hdr->is_mp2 = BS_ReadInt(bs, 1);
		BS_ReadInt(bs, 2);
		hdr->no_crc = BS_ReadInt(bs, 1);
		pos = (u32) BS_GetPosition(bs) - 2;

		hdr->profile = BS_ReadInt(bs, 2);
		if (!hdr->is_mp2) hdr->profile += 1;
		hdr->sr_idx = BS_ReadInt(bs, 4);
		BS_ReadInt(bs, 1);
		hdr->nb_ch = BS_ReadInt(bs, 3);
		BS_ReadInt(bs, 4);
		hdr->frame_size = BS_ReadInt(bs, 13);
		BS_ReadInt(bs, 11);
		BS_ReadInt(bs, 2);
		hdr_size = hdr->no_crc ? 7 : 9;
		if (!hdr->no_crc) BS_ReadInt(bs, 16);
		if (hdr->frame_size < hdr_size) {
			BS_Seek(bs, pos+1);
			continue;
		}
		hdr->frame_size -= hdr_size;
		if (BS_Available(bs) == hdr->frame_size) return 1;

		BS_SkipBytes(bs, hdr->frame_size);
		val = BS_ReadInt(bs, 8);
		if (val!=0xFF) {
			BS_Seek(bs, pos+1);
			continue;
		}
		val = BS_ReadInt(bs, 4);
		if (val!=0x0F) {
			BS_ReadInt(bs, 4);
			BS_Seek(bs, pos+2);
			continue;
		}
		BS_Seek(bs, pos+hdr_size);
		return 1;
	}
	return 0;
}

M4Err MP4T_ImportAAC_ADTS(M4TrackImporter *import)
{
	u8 oti;
	Bool destroy_esd;
	M4Err e;
	ESDescriptor *esd;
	Double dur;
	Bool sync_frame;
	u16 sr, sbr_sr, sbr_sr_idx;
	BitStream *bs, *dsi;
	ADTSHeader hdr;
	FILE *in;
	u64 offset;
	u32 max_size, track, di, rate, time_wnd, tot_size, done, duration, prof, i;
	M4Sample *samp;

	in = fopen(import->in_name, "rb");
	if (!in) return import_message(import, M4URLNotFound, "Opening file %s failed", import->in_name);

	bs = NewBitStreamFromFile(in, BS_FILE_READ);

	sync_frame = ADTS_SyncFrame(bs, &hdr);
	if (!sync_frame) {
		DeleteBitStream(bs);
		fclose(in);
		return import_message(import, M4NonCompliantBitStream, "Audio isn't MPEG-2/4 AAC with ADTS");
	}
	if (import->flags & M4TI_FORCE_SBR) hdr.is_mp2 = 0;
	oti = hdr.is_mp2 ? hdr.profile+0x66 : 0x40;
	sr = m4a_sample_rates[hdr.sr_idx];


	dsi = NewBitStream(NULL, 0, BS_WRITE);

	sbr_sr = sr;
	sbr_sr_idx = hdr.sr_idx;
	for (i=0; i<16; i++) {
		if (m4a_sample_rates[i] == (u32) 2*sr) {
			sbr_sr_idx = i;
			sbr_sr = 2*sr;
			break;
		}
	}

	/*direct SBR signal (non backward-compatible)*/
#if 0
	BS_WriteInt(dsi, (import->flags & M4TI_FORCE_SBR) ? 5 : hdr.profile, 5);
	BS_WriteInt(dsi, hdr.sr_idx, 4);
	BS_WriteInt(dsi, hdr.nb_ch, 4);
	if (import->flags & M4TI_FORCE_SBR) {
		for (i=0; i<16; i++) {
			if (m4a_sample_rates[i] == (u32) 2*sr) {
				sbr_sr_idx = i;
				sbr_sr = 2*sr;
				break;
			}
		}
		BS_WriteInt(dsi, sbr_sr ? sbr_sr_idx : hdr.sr_idx, 4);
		BS_WriteInt(dsi, hdr.profile, 5);
	}
#else
	BS_WriteInt(dsi, hdr.profile, 5);
	BS_WriteInt(dsi, hdr.sr_idx, 4);
	BS_WriteInt(dsi, hdr.nb_ch, 4);
	BS_Align(dsi);
	if (import->flags & M4TI_FORCE_SBR) {
		BS_WriteInt(dsi, 0x2b7, 11); /*sync extension type*/
		BS_WriteInt(dsi, 5, 5);	/*SBR objectType*/
		BS_WriteInt(dsi, 1, 1);	/*SBR present flag*/
		BS_WriteInt(dsi, sbr_sr_idx, 4);
	}
#endif

	BS_Align(dsi);
	prof = hdr.profile;

	e = M4OK;
	destroy_esd = 0;
	esd = import->esd;
	if (!esd) {
		esd = OD_NewESDescriptor(2);
		destroy_esd = 1;
	}
	if (!esd->decoderConfig) esd->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
	if (!esd->slConfig) esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
	esd->decoderConfig->streamType = M4ST_AUDIO;
	esd->decoderConfig->objectTypeIndication = oti;
	esd->decoderConfig->bufferSizeDB = 20;
	esd->slConfig->timestampResolution = sr;
	if (!esd->decoderConfig->decoderSpecificInfo) esd->decoderConfig->decoderSpecificInfo = (DefaultDescriptor *) OD_NewDescriptor(DecoderSpecificInfo_Tag);
	if (esd->decoderConfig->decoderSpecificInfo->data) free(esd->decoderConfig->decoderSpecificInfo->data);
	BS_GetContent(dsi, (unsigned char **) &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
	DeleteBitStream(dsi);

	samp = NULL;
	import_message(import, M4OK, "AAC import %s- sample rate %d - %s audio - %d channel%s", (import->flags & M4TI_FORCE_SBR) ? "SBR forced " : "", sr, (oti==0x40) ? "MPEG-4" : "MPEG-2", hdr.nb_ch, (hdr.nb_ch>1) ? "s" : "");

	track = M4_NewTrack(import->dest, esd->ESID, M4_AudioMediaType, sr);
	if (!track) {
		e = M4_GetLastError(import->dest);
		goto exit;
	}
	M4_SetTrackEnabled(import->dest, track, 1);
	if (!esd->ESID) esd->ESID = M4_GetTrackID(import->dest, track);
	M4_NewStreamDescription(import->dest, track, esd, (import->flags & M4TI_USE_DATAREF) ? import->in_name : NULL, NULL, &di);
	M4_SetAudioDescriptionInfo(import->dest, track, di, sr, hdr.nb_ch, 16);

	e = M4OK;
	/*add first sample*/
	samp = M4_NewSample();
	samp->IsRAP = 1;
	max_size = samp->dataLength = hdr.frame_size;
	samp->data = malloc(sizeof(char)*hdr.frame_size);
	offset = BS_GetPosition(bs);
	BS_ReadData(bs, samp->data, hdr.frame_size);

	if (import->flags & M4TI_USE_DATAREF) {
		M4_AddSampleReference(import->dest, track, di, samp, offset);
	} else {
		M4_AddSample(import->dest, track, di, samp);
	}
	samp->DTS+=1024;
	rate = samp->dataLength;

	duration = import->duration*sr;
	duration /= 1000;

	tot_size = (u32) BS_GetSize(bs);
	time_wnd = 0;
	done = 0;
	while (BS_Available(bs) ) {
		sync_frame = ADTS_SyncFrame(bs, &hdr);
		if (!sync_frame) break;
		if (hdr.frame_size>max_size) {
			samp->data = realloc(samp->data, sizeof(char) * hdr.frame_size);
			max_size = hdr.frame_size;
		}
		samp->dataLength = hdr.frame_size;

		offset = BS_GetPosition(bs);
		BS_ReadData(bs, samp->data, hdr.frame_size);
		if (import->flags & M4TI_USE_DATAREF) {
			M4_AddSampleReference(import->dest, track, di, samp, offset);
		} else {
			M4_AddSample(import->dest, track, di, samp);
		}

		samp->DTS += 1024;

		if (esd->decoderConfig->bufferSizeDB < samp->dataLength) esd->decoderConfig->bufferSizeDB = samp->dataLength;
		esd->decoderConfig->avgBitrate += samp->dataLength;
		rate += samp->dataLength;
		if (samp->DTS > time_wnd + sr) {
			if (rate > esd->decoderConfig->maxBitrate) esd->decoderConfig->maxBitrate = rate;
			time_wnd = samp->DTS;
			rate = 0;
		}
		done += samp->dataLength;
		import_progress(import, done, tot_size);
		if (duration && (samp->DTS > duration)) break;
		if (import->flags & M4TI_DO_ABORT) break;
	}
	dur = samp->DTS;
	dur /= sr;
	esd->decoderConfig->avgBitrate = (u32) (esd->decoderConfig->avgBitrate / dur);
	esd->decoderConfig->avgBitrate *= 8;
	esd->decoderConfig->maxBitrate *= 8;
	M4_ChangeStreamDescriptor(import->dest, track, di, esd);

exit:
	if (esd && destroy_esd) OD_DeleteDescriptor((Descriptor **) &esd);
	if (samp) M4_DeleteSample(&samp);
	DeleteBitStream(bs);
	fclose(in);
	return e;
}

M4Err MP4T_ImportAVIVideo(M4TrackImporter *import)
{
	M4Err e;
	Double FPS, dur;
	FILE *test;
	M4Sample *samp;
	u32 i, num_samples, timescale, size, max_size, samp_offset, track, di, rate, time_wnd, PL, max_b, nb_b, nb_f;
	u32 nbI, nbP, nbB, nbDummy, nbNotCoded;
	Bool is_vfr, enable_vfr, erase_pl;
	M4VDecoderSpecificInfo dsi;
	LPM4VPARSER vparse;
	s32 key;
	u32 duration;
	Bool destroy_esd, is_packed, is_init;
	ESDescriptor *esd;
	char *comp, *frame;
	avi_t *in;

	if (import->trackID>1) return M4OK;

	test = fopen(import->in_name, "rb");
	if (!test) return import_message(import, M4URLNotFound, "Opening %s failed", import->in_name);
	fclose(test);
	in = AVI_open_input_file(import->in_name, 1);
	if (!in) return import_message(import, M4URLNotFound, "Unsupported avi file");

	destroy_esd = 0;
	frame = NULL;
	AVI_seek_start(in);

	erase_pl = 0;
	comp = AVI_video_compressor(in);
	if (!comp) {
		e = M4NotSupported;
		goto exit;
	}

	/*these are/should be OK*/
	if (!stricmp(comp, "DIVX") || !stricmp(comp, "DX50")	/*DivX*/
		|| !stricmp(comp, "XVID") /*XviD*/
		|| !stricmp(comp, "3iv2") /*3ivX*/
		|| !stricmp(comp, "fvfw") /*ffmpeg*/
		|| !stricmp(comp, "NDIG") /*nero*/
		|| !stricmp(comp, "MP4V") /*!! not tested*/
		|| !stricmp(comp, "M4CC") /*Divio - not tested*/
		|| !stricmp(comp, "PVMM") /*PacketVideo - not tested*/
		|| !stricmp(comp, "SEDG") /*Samsung - not tested*/
		|| !stricmp(comp, "RMP4") /*Sigma - not tested*/
		) {
		e = M4OK;
	}
	else if (!stricmp(comp, "DIV3") || !stricmp(comp, "DIV4")) {
		import_message(import, M4NotSupported, "Video format %s not compliant with MPEG-4 Visual - please recompress the file first", comp);
		e = M4NotSupported;
		goto exit;
	} else {
		import_message(import, M4NotSupported, "Video format %s not supported - recompress the file first", comp);
		e = M4NotSupported;
		goto exit;
	}

	/*import at 1000* the framerate (this is extremely usefull when importing DivX/other avis with adjusted FPS*/
	FPS = AVI_frame_rate(in);
	timescale = (u32) (FPS * 1000);
	duration = (u32) (import->duration*FPS);

	e = M4OK;
	max_size = 0;
	samp_offset = 0;
	frame = NULL;
	num_samples = AVI_video_frames(in);
	samp = M4_NewSample();
	time_wnd = 0;
	rate = 0;
	PL = 0;
	track = 0;
	is_vfr = 0;

	is_packed = 0;
	nbDummy = nbNotCoded = nbI = nbP = nbB = nb_b = max_b = 0;
	enable_vfr = 0;

	is_init = 0;
	esd = import->esd;

	for (i=0; i<num_samples; i++) {
		size = AVI_frame_size(in, i);
		if (!size) {
			AVI_read_frame(in, NULL, &key);
			continue;
		}

		if (size > max_size) {
			frame = realloc(frame, sizeof(char) * size);
			max_size = size;
		}
		AVI_read_frame(in, frame, &key);

		/*get DSI*/
		if (!is_init) {
			is_init = 1;
			vparse = NewMPEG4VideoParser(frame, size);
			e = M4V_ParseConfig(vparse, &dsi);
			PL = dsi.VideoPL;
			if (!PL) {
				PL = 0x01;
				erase_pl = 1;
			}
			samp_offset = M4V_GetObjectStartPos(vparse);
			DeleteMPEG4VideoParser(vparse);
			if (e) {
				import_message(import, e, "Cannot import decoder config in first frame");
				goto exit;
			}

			if (!esd) {
				esd = OD_NewESDescriptor(0);
				destroy_esd = 1;
			}
			track = M4_NewTrack(import->dest, esd->ESID, M4_VisualMediaType, timescale);
			if (!track) {
				e = M4_GetLastError(import->dest);
				goto exit;
			}
			M4_SetTrackEnabled(import->dest, track, 1);
			if (!esd->ESID) esd->ESID = M4_GetTrackID(import->dest, track);
			
			if (!esd->slConfig) esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
			esd->slConfig->timestampResolution = timescale;

			if (!esd->decoderConfig) esd->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
			if (esd->decoderConfig->decoderSpecificInfo) OD_DeleteDescriptor((Descriptor **) &esd->decoderConfig->decoderSpecificInfo);
			esd->decoderConfig->decoderSpecificInfo = (DefaultDescriptor *) OD_NewDescriptor(DecoderSpecificInfo_Tag);
			esd->decoderConfig->streamType = M4ST_VISUAL;
			esd->decoderConfig->objectTypeIndication = 0x20;
			esd->decoderConfig->decoderSpecificInfo->data = malloc(sizeof(char) * samp_offset);
			memcpy(esd->decoderConfig->decoderSpecificInfo->data, frame, sizeof(char) * samp_offset);
			esd->decoderConfig->decoderSpecificInfo->dataLength = samp_offset;

			M4_SetCTSPackMode(import->dest, track, 1, 0);

			/*remove packed flag if any (VOSH user data)*/
			while (i+3<samp_offset) {
				while ((frame[i]!=0) || (frame[i+1]!=0) || (frame[i+2]!=1)) i++;
				if ((frame[i+4] != 'D') || (frame[i+5] != 'i') || (frame[i+6] != 'v')) {
					i += 4;
					continue;
				}
				if (esd->decoderConfig->decoderSpecificInfo->data[i+15] == 'p') 
					esd->decoderConfig->decoderSpecificInfo->data[i+15] = 'n';
				break;
			}
			i = 0;
			
			e = M4_NewStreamDescription(import->dest, track, esd, (import->flags & M4TI_USE_DATAREF) ? import->in_name: NULL, NULL, &di);
			if (e) goto exit;
			M4_SetVisualEntrySize(import->dest, track, di, dsi.width, dsi.height);
			import_message(import, M4OK, "AVI %s video import - %d x %d @ %.2f FPS - %d Frames\nIndicated Profile: %s", comp, dsi.width, dsi.height, FPS, num_samples, M4V_VideoProfileName((u8) PL));
		}


		if (size > samp_offset) {
			u8 ftype;
			u32 tinc, framesize, frame_start;
			u64 file_offset;
			Bool is_coded;

			size -= samp_offset;
			file_offset = (u64) AVI_get_video_position(in, i);

			vparse = NewMPEG4VideoParser(frame + samp_offset, size);

			samp->dataLength = 0;
			/*removing padding frames*/
			if (size<4) {
				nbDummy ++;
				size = 0;
			}

			nb_f=0;
			while (size) {
				M4Err e = M4V_GetFrame(vparse, dsi, &ftype, &tinc, &framesize, &frame_start, &is_coded);

				if (!is_coded) {
					if (!M4V_IsValidObjectType(vparse)) import_message(import, M4OK, "WARNING: AVI frame %d doesn't look like MPEG-4 Visual", i+1);
					nbNotCoded ++;
					if (!is_packed) {
						is_vfr = 1;
						/*policy is to import at constant frame rate from AVI*/
						if (import->flags & M4TI_NO_FRAME_DROP) goto proceed;
						/*policy is to import at variable frame rate from AVI*/
						samp->DTS += 1000;
					}
					/*assume this is packed bitstream n-vop and discard it.*/
				} else {
proceed:
				if (e==M4EOF) size = 0;
					else is_packed = 1;
					nb_f++;

					if (ftype==2) {
						nb_b ++;
						nbB++;
						samp->CTS_Offset = 0;
					} else {
						if (max_b<nb_b) max_b=nb_b;
						nb_b = 0;
						if (ftype) nbP++;
						else nbI++;

						/*default offset for initial frame (CTS>=DTS) to handle B-frames. If no B the CTS offset table is removed*/
						samp->CTS_Offset = 1000;
					}

					samp->IsRAP = (ftype==0) ? 1 : 0;
					/*frame_start indicates start of VOP (eg we always remove VOL from each I)*/
					samp->data = frame + samp_offset + frame_start;
					samp->dataLength = framesize;

					if (import->flags & M4TI_USE_DATAREF) {
						samp->data = NULL;
						e = M4_AddSampleReference(import->dest, track, di, samp, file_offset + samp_offset + frame_start);
					} else {
						e = M4_AddSample(import->dest, track, di, samp);
					}

					if (esd->decoderConfig->bufferSizeDB < samp->dataLength) esd->decoderConfig->bufferSizeDB = samp->dataLength;
					esd->decoderConfig->avgBitrate += samp->dataLength;
					rate += samp->dataLength;
					if (samp->DTS > time_wnd + timescale) {
						if (rate > esd->decoderConfig->maxBitrate) esd->decoderConfig->maxBitrate = rate;
						time_wnd = samp->DTS;
						rate = 0;
					}


					samp->DTS += 1000;
					if (e) import_message(import, M4OK, "Error importing AVI frame %d", i+1);
				}
				if (!size || (size == framesize + frame_start)) break;
			}
			DeleteMPEG4VideoParser(vparse);
			if (nb_f>2) import_message(import, M4OK, "Warning: more than 2 frames packed together");
		}
		samp_offset = 0;
		import_progress(import, i+1, num_samples);
		if (duration && (samp->DTS > duration)) break;
		if (import->flags & M4TI_DO_ABORT) break;
	}

	num_samples = M4_GetSampleCount(import->dest, track);
	if (max_b) {
		import_message(import, M4OK, "Has B-Frames (%d max consecutive B-VOPs%s) - Adjusting CTS", max_b, is_packed ? " - packed bitstream" : "");
		/*repack CTS tables and adjust offsets for B-frames*/
		M4_SetCTSPackMode(import->dest, track, 0, 1);
		import_message(import, M4OK, "CTS Adjustment done");
		/*this is plain ugly but since some encoders (divx) use the video PL correctly we force the system video_pl
		to ASP@L5 since we have I, P, B in base layer*/
		if (PL<=3) {
			PL = 0xF5;
			erase_pl = 1;
			import_message(import, M4OK, "WARNING: indicated profile doesn't include B-VOPs - forcing %s", M4V_VideoProfileName((u8) PL));
		}
		import_message(import, M4OK, "Import results: %d VOPs (%d Is - %d Ps - %d Bs)", num_samples, nbI, nbP, nbB);
	} else {
		/*no B-frames, remove CTS offsets*/
		M4_DeleteCTSTable(import->dest, track);
		import_message(import, M4OK, "Import results: %d VOPs (%d Is - %d Ps)", num_samples, nbI, nbP);
	}

	if (erase_pl) M4V_RewritePL(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, (u8) PL);
	dur = samp->DTS;
	dur /= timescale;
	esd->decoderConfig->avgBitrate = (u32) (esd->decoderConfig->avgBitrate / dur);
	/*move to bps*/
	esd->decoderConfig->avgBitrate *= 8;
	esd->decoderConfig->maxBitrate *= 8;
	M4_ChangeStreamDescriptor(import->dest, track, di, esd);
	samp->data = NULL;
	M4_DeleteSample(&samp);

	if (is_vfr) {
		if (nbB) {
			if (is_packed) import_message(import, M4OK, "Warning: Mix of non-coded frames: packed bitstream and encoder skiped - unpredictable timing");
		} else {
			if (import->flags & M4TI_NO_FRAME_DROP) {
				if (nbNotCoded) import_message(import, M4OK, "Stream has %d N-VOPs", nbNotCoded);
			} else {
				import_message(import, M4OK, "import using Variable Frame Rate - Removed %d N-VOPs", nbNotCoded);
			}
			nbNotCoded = 0;
		}
	}

	if (nbDummy || nbNotCoded) import_message(import, M4OK, "Removed Frames: %d VFW delay frames - %d N-VOPs", nbDummy, nbNotCoded);
	M4_SetMoviePLIndication(import->dest, M4_PL_VISUAL, (u8) PL);

exit:
	if (esd && destroy_esd) OD_DeleteDescriptor((Descriptor **) &esd);
	if (frame) free(frame);
	AVI_close(in);
	return e;
}




/*credits to CISCO MPEG4/IP for MP3 parsing*/
M4Err MP4T_ImportAVIAudio(M4TrackImporter *import)
{
	M4Err e;
	FILE *test;
	u32 hdr, di, track, i, rate, time_wnd, tot_size, duration;
	s64 offset;
	s32 size, max_size, done;
	u16 sampleRate;
	Double dur;
	Bool is_cbr;
	u8 oti;
	M4Sample *samp;
	unsigned char *frame;
	Bool destroy_esd;
	s32 continuous;
	ESDescriptor *esd;
	unsigned char temp[4];
	avi_t *in;

	/*video only, ignore*/
	if (import->trackID==1) return M4OK;

	test = fopen(import->in_name, "rb");
	if (!test) return import_message(import, M4URLNotFound, "Opening file %s failed", import->in_name);
	fclose(test);
	in = AVI_open_input_file(import->in_name, 1);
	if (!in) return import_message(import, M4NotSupported, "Unsupported avi file");

	AVI_seek_start(in);

	e = M4OK;

	if (AVI_read_audio(in, (char *) temp, 4, &continuous) != 4) {
		AVI_close(in);
		return import_message(import, M4OK, "No audio track found");
	}
	hdr = FOUR_CHAR_INT(temp[0], temp[1], temp[2], temp[3]);
	if ((hdr &0xFFE00000) != 0xFFE00000) {
		AVI_close(in);
		return import_message(import, M4NotSupported, "Unsupported AVI audio format");
	}

	sampleRate = MP3_GetSamplingRate(hdr);
	oti = MP3_GetObjectTypeIndication(hdr);
	if (!oti || !sampleRate) {
		AVI_close(in);
		return import_message(import, M4NotSupported, "Error: invalid MPEG Audio track");
	}
		
	frame = NULL;
	destroy_esd = 0;
	esd = import->esd;

	if (!esd) {
		destroy_esd = 1;
		esd = OD_NewESDescriptor(0);
	}
	track = M4_NewTrack(import->dest, esd->ESID, M4_AudioMediaType, sampleRate);
	if (!track) goto exit;
	M4_SetTrackEnabled(import->dest, track, 1);
	if (!esd->ESID) esd->ESID = M4_GetTrackID(import->dest, track);

	if (!esd->decoderConfig) esd->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
	if (!esd->slConfig) esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
	esd->slConfig->timestampResolution = sampleRate;
	if (esd->decoderConfig->decoderSpecificInfo) OD_DeleteDescriptor((Descriptor **) &esd->decoderConfig->decoderSpecificInfo);
	esd->decoderConfig->streamType = M4ST_AUDIO;
	esd->decoderConfig->objectTypeIndication = oti;
	e = M4_NewStreamDescription(import->dest, track, esd, (import->flags & M4TI_USE_DATAREF) ? import->in_name : NULL, NULL, &di);
	if (e) goto exit;

	import_message(import, M4OK, "AVI MP3 import - sample rate %d - %s audio - %d channel%s", sampleRate, (oti==0x6B) ? "MPEG-1" : "MPEG-2", MP3_GetNumChannels(hdr), (MP3_GetNumChannels(hdr)>1) ? "s" : "");

	AVI_seek_start(in);
	AVI_set_audio_position(in, 0);

	i = 0;
	tot_size = max_size = 0;
	while ((size = AVI_audio_size(in, i) )>0) {
		if (max_size<size) max_size=size;
		tot_size += size;
		i++;
	}

	frame = malloc(sizeof(char) * max_size);
	AVI_seek_start(in);
	AVI_set_audio_position(in, 0);

	dur = import->duration;
	dur *= sampleRate;
	dur /= 1000;
	duration = (u32) dur;

	samp = M4_NewSample();
	rate = 0;
	time_wnd = 0;
	done=max_size=0;
	is_cbr = 1;
	while (1) {
		if (AVI_read_audio(in, frame, 4, &continuous) != 4) break;
		offset = f64_tell(in->fdes) - 4;
		hdr = FOUR_CHAR_INT(frame[0], frame[1], frame[2], frame[3]);

		size = MP3_GetFrameSize(hdr);
		if (size>max_size) {
			frame = realloc(frame, sizeof(char) * size);
			if (max_size) is_cbr = 0;
			max_size = size;
		}
		size = 4 + AVI_read_audio(in, (char*) &frame[4], size - 4, &continuous);

		if ((import->flags & M4TI_USE_DATAREF) && !continuous) {
			import_message(import, M4IOErr, "Cannot use media references, splited input audio frame found");
			e = M4IOErr;
			goto exit;
		}
		samp->IsRAP = 1;
		samp->data = frame;
		samp->dataLength = size;
		if (import->flags & M4TI_USE_DATAREF) {
			e = M4_AddSampleReference(import->dest, track, di, samp, offset);
		} else {
			e = M4_AddSample(import->dest, track, di, samp);
		}

		samp->DTS += MP3_GetSamplesPerFrame(hdr);

		if (esd->decoderConfig->bufferSizeDB < samp->dataLength) esd->decoderConfig->bufferSizeDB = samp->dataLength;
		esd->decoderConfig->avgBitrate += samp->dataLength;
		rate += samp->dataLength;
		if (samp->DTS > time_wnd + sampleRate) {
			if (rate > esd->decoderConfig->maxBitrate) esd->decoderConfig->maxBitrate = rate;
			time_wnd = samp->DTS;
			rate = 0;
		}

		done += size;
		import_progress(import, done, tot_size);
		if (duration && (samp->DTS > duration) ) break;
		if (import->flags & M4TI_DO_ABORT) break;
	}
	
	import_message(import, M4OK, "Import done - %s bit rate MP3 detected", is_cbr ? "constant" : "variable");
	dur = samp->DTS;
	dur /= sampleRate;
	esd->decoderConfig->avgBitrate = (u32) (esd->decoderConfig->avgBitrate / dur);
	/*move to bps*/
	esd->decoderConfig->avgBitrate *= 8;
	esd->decoderConfig->maxBitrate *= 8;
	M4_ChangeStreamDescriptor(import->dest, track, di, esd);
	samp->data = NULL;
	M4_DeleteSample(&samp);
	M4_SetMoviePLIndication(import->dest, M4_PL_AUDIO, 0xFE);

	
exit:
	if (esd && destroy_esd) OD_DeleteDescriptor((Descriptor **) &esd);
	if (frame) free(frame);
	AVI_close(in);
	return e;
}

M4Err MP4T_ImportMP4(M4TrackImporter *import)
{
	M4Err e;
	u64 offset;
	u32 track, di, trackID, track_in, i, num_samples, sampDTS, mtype, w, h, duration;
	M4Sample *samp;
	ESDescriptor *in_esd;
	InitialObjectDescriptor *iod;

	trackID = import->trackID;
	if (!trackID) {
		if (M4_GetTrackCount(import->orig) != 1) return import_message(import, M4BadParam, "Several tracks in MP4 - please indicate track to import");
		trackID = M4_GetTrackID(import->orig, 1);
	}
	track_in = M4_GetTrackByID(import->orig, trackID);
	if (!track_in) return import_message(import, M4URLNotFound, "Cannot find track ID %d in file", trackID);

	in_esd = M4_GetStreamDescriptor(import->orig, track_in, 1);
	if (!in_esd) {
		e = M4_GetLastError(import->dest);
		goto exit;
	}
	e = M4OK;
	if (import->esd) {
		in_esd->OCRESID = import->esd->OCRESID;
		/*there may be other things to import...*/
	}

	iod = (InitialObjectDescriptor *) M4_GetRootOD(import->orig);
	if (iod && (iod->tag != InitialObjectDescriptor_Tag)) {
		OD_DeleteDescriptor((Descriptor **) &iod);
		iod = NULL;
	}
	mtype = M4_GetMediaType(import->orig, track_in);
	if (mtype==M4_VisualMediaType) {
		u8 PL = iod ? iod->visual_profileAndLevel : 0xFE;
		w = h = 0;
		M4_GetVisualEntrySize(import->orig, track_in, 1, &w, &h);
		/*for MPEG-4 visual, always check size (don't trust input file)*/
		if (in_esd->decoderConfig->objectTypeIndication==0x20) {
			M4VDecoderSpecificInfo dsi;
			M4V_GetConfig(in_esd->decoderConfig->decoderSpecificInfo->data, in_esd->decoderConfig->decoderSpecificInfo->dataLength, &dsi);
			w = dsi.width;
			h = dsi.height;
			PL = dsi.VideoPL;
		}
		M4_SetMoviePLIndication(import->dest, M4_PL_VISUAL, PL);
	}
	else if (mtype==M4_AudioMediaType) {
		M4_SetMoviePLIndication(import->dest, M4_PL_AUDIO, (u8) (iod ? iod->audio_profileAndLevel : 0xFE));
	}
	OD_DeleteDescriptor((Descriptor **) &iod);
	
	track = M4_NewTrack(import->dest, import->esd ? import->esd->ESID : 0, M4_GetMediaType(import->orig, track_in), M4_GetMediaTimeScale(import->orig, track_in));
	if (!track) {
		e = M4_GetLastError(import->dest);
		goto exit;
	}
	M4_SetTrackEnabled(import->dest, track, 1);
	if (import->esd && !import->esd->ESID) import->esd->ESID = M4_GetTrackID(import->dest, track);
	else if (!import->esd) in_esd->ESID = M4_GetTrackID(import->dest, track);

	if (import->flags & M4TI_USE_DATAREF) {
		const char *urn = NULL;
		const char *url = M4_GetFilename(import->orig);
		if (!M4_IsSelfContained(import->orig, track_in, 1)) {
			e = M4_GetStreamDataReference(import->orig, track_in, 1, &url, &urn);
			if (e) goto exit;
		}
		e = M4_NewStreamDescription(import->dest, track, in_esd, (char *) url, (char *) urn, &di);
	} else {
		e = M4_NewStreamDescription(import->dest, track, in_esd, NULL, NULL, &di);
	}
	if (e) goto exit;
	switch (mtype) {
	case M4_VisualMediaType:
		M4_SetVisualEntrySize(import->dest, track, di, w, h);
		import_message(import, M4OK, "IsoMedia import - track ID %d - Video size %d x %d", trackID, w, h);
		break;
	case M4_AudioMediaType:
	{
		u32 sr, ch;
		u8 bps;
		M4_GetAudioEntryInfo(import->orig, track_in, di, &sr, &ch, &bps);
		M4_SetAudioDescriptionInfo(import->dest, track, di, sr, ch, bps);
		import_message(import, M4OK, "IsoMedia import - track ID %d - Audio sample rate %d - %d channels", trackID, sr, ch);
	}
		break;
	default:
	{
		char szType[5], szSubType[5];
		szType[4] = szSubType[4] = 0;
		MP4TypeToString(mtype, szType);
		MP4TypeToString(M4_GetMediaSubType(import->orig, track_in, di), szSubType);
		import_message(import, M4OK, "IsoMedia import - track ID %d - media type %s sub-type %s", trackID, szType, szSubType);
	}
		break;
	}
	
	duration = import->duration * M4_GetMediaTimeScale(import->orig, track_in);
	duration /= 1000;

	num_samples = M4_GetSampleCount(import->orig, track_in);
	for (i=0; i<num_samples; i++) {
		if (import->flags & M4TI_USE_DATAREF) {
			samp = M4_GetSampleInfo(import->orig, track_in, i+1, &di, &offset);
			if (!samp) {
				e = M4_GetLastError(import->orig);
				goto exit;
			}
			M4_AddSampleReference(import->dest, track, 1, samp, offset);
		} else {
			samp = M4_GetSample(import->orig, track_in, i+1, &di);
			M4_AddSample(import->dest, track, 1, samp);
		}
		sampDTS = samp->DTS;
		M4_DeleteSample(&samp);
		import_progress(import, i+1, num_samples);
		if (duration && (sampDTS > duration) ) break;
		if (import->flags & M4TI_DO_ABORT) break;
	}

	if (import->esd) {
		if (!import->esd->slConfig) {
			import->esd->slConfig = in_esd->slConfig;
			in_esd->slConfig = NULL;
		}
		if (!import->esd->decoderConfig) {
			import->esd->decoderConfig = in_esd->decoderConfig;
			in_esd->decoderConfig = NULL;
		}
	}		

exit:
	if (in_esd) OD_DeleteDescriptor((Descriptor **) &in_esd);
	return e;
}

#include "mpeg2_ps.h"

M4Err MP4T_ImportMPEG12Video(M4TrackImporter *import)
{
	M4Err e;
	mpeg2ps_t *ps;
	char *buf;
	u8 ftype;
	u32 track, di, streamID, mtype, w, h, nb_streams, buf_len, frames, ref_frame, timescale, duration;
	Bool destroy_esd;
	ESDescriptor *esd;

	if (import->flags & M4TI_USE_DATAREF) 
		return import_message(import, M4NotSupported, "Cannot use data referencing with MPEG-1/2 files");

	ps = mpeg2ps_init(import->in_name);
	if (!ps) return import_message(import, M4NonCompliantBitStream, "Failed to open MPEG file %s", import->in_name);


	streamID = 0;
	nb_streams = mpeg2ps_get_video_stream_count(ps);
	if ((nb_streams>1) && !import->trackID) {
		mpeg2ps_close(ps);
		return import_message(import, M4BadParam, "%d video tracks in MPEG file - please indicate track to import", nb_streams);
	}
	if (import->trackID) {
		/*FIXME - stream selection to do...*/

//		streamID = import->trackID - 1;
		if (import->trackID==2) {
			mpeg2ps_close(ps);
			return M4OK;
		}
		streamID = 0;
	}

	if (streamID>=nb_streams) {
		mpeg2ps_close(ps);
		return import_message(import, M4BadParam, "Desired video track not found in MPEG file (%d visual streams)", nb_streams);
	}
	w = mpeg2ps_get_video_stream_width(ps, streamID);
	h = mpeg2ps_get_video_stream_height(ps, streamID);
	mtype = (mpeg2ps_get_video_stream_type(ps, streamID) == MPEG_VIDEO_MPEG2) ? 0x61 : 0x6A;
	timescale = (u32) (1000.0 * mpeg2ps_get_video_stream_framerate(ps, streamID));

	duration = import->duration*timescale;
	duration /= 1000;

	destroy_esd = 0;
	esd = import->esd;
	if (!esd) {
		destroy_esd = 1;
		esd = OD_NewESDescriptor(0);
	}
	track = M4_NewTrack(import->dest, esd->ESID, M4_VisualMediaType, timescale);
	e = M4_GetLastError(import->dest);
	if (!track) goto exit;
	M4_SetTrackEnabled(import->dest, track, 1);
	if (!esd->ESID) esd->ESID = M4_GetTrackID(import->dest, track);

	if (!esd->decoderConfig) esd->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
	if (!esd->slConfig) esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
	esd->slConfig->timestampResolution = timescale;
	if (esd->decoderConfig->decoderSpecificInfo) OD_DeleteDescriptor((Descriptor **) &esd->decoderConfig->decoderSpecificInfo);
	esd->decoderConfig->streamType = M4ST_VISUAL;
	esd->decoderConfig->objectTypeIndication = mtype;
	e = M4_NewStreamDescription(import->dest, track, esd, NULL, NULL, &di);
	if (e) goto exit;

	import_message(import, M4OK, "%s Video import - Resolution %d x %d @ %g FPS", (mtype==0x6A) ? "MPEG-1" : "MPEG-2", w, h, mpeg2ps_get_video_stream_framerate(ps, streamID) );
	M4_SetVisualEntrySize(import->dest, track, di, w, h);

	M4_SetCTSPackMode(import->dest, track, 1, 0);

	frames = 1;
	ref_frame = 1;
	while (mpeg2ps_get_video_frame(ps, streamID, (u8 **) &buf, &buf_len, &ftype, TS_90000, NULL)) {
		M4Sample *samp;
		if ((buf[buf_len - 4] == 0) && (buf[buf_len - 3] == 0) && (buf[buf_len - 2] == 1)) buf_len -= 4;
		samp = M4_NewSample();
		samp->data = buf;
		samp->dataLength = buf_len;
		samp->DTS = 1000*(frames-1);
		samp->IsRAP = (ftype==1) ? 1 : 0;
		samp->CTS_Offset = 0;
		M4_AddSample(import->dest, track, di, samp);
		samp->data = NULL;
		M4_DeleteSample(&samp);

		import_progress(import, frames-1, frames);

		if (ftype != 3) {
			M4_SetSampleCTSOffset(import->dest, track, ref_frame, (frames-ref_frame)*1000);
			ref_frame = frames;
		}
		frames++;

		if (duration && (1000*(frames-1) >= duration) ) break;
		if (import->flags & M4TI_DO_ABORT) break;
	}
	M4_SetCTSPackMode(import->dest, track, 0, 0);
	import_progress(import, frames, frames);

exit:
	if (destroy_esd) OD_DeleteDescriptor((Descriptor **) &esd);
	mpeg2ps_close(ps);
	return e;
}

M4Err MP4T_ImportMPEG12Audio(M4TrackImporter *import)
{
	M4Err e;
	mpeg2ps_t *ps;
	u8 *buf;
	u32 track, di, streamID, mtype, sr, nb_ch, nb_streams, buf_len, frames, hdr, duration;
	Bool destroy_esd;
	M4Sample *samp;
	ESDescriptor *esd;

	if (import->flags & M4TI_USE_DATAREF) 
		return import_message(import, M4NotSupported, "Cannot use data referencing with MPEG-1/2 files");

	ps = mpeg2ps_init(import->in_name);
	if (!ps) return import_message(import, M4NonCompliantBitStream, "Failed to open MPEG file %s", import->in_name);


	streamID = 0;
	nb_streams = mpeg2ps_get_audio_stream_count(ps);
	if ((nb_streams>1) && !import->trackID) {
		mpeg2ps_close(ps);
		return import_message(import, M4BadParam, "%d audio tracks in MPEG file - please indicate track to import", nb_streams);
	}
	if (import->trackID) {
		/*FIXME - stream selection to do...*/

//		streamID = import->trackID - 1;
		if (import->trackID==1) {
			mpeg2ps_close(ps);
			return M4OK;
		}
		streamID = 0;
	}

	if (streamID>=nb_streams) {
		mpeg2ps_close(ps);
		return import_message(import, M4BadParam, "Desired audio track not found in MPEG file (%d audio streams)", nb_streams);
	}
	
	mtype = mpeg2ps_get_audio_stream_type(ps, streamID);
	if (mtype != MPEG_AUDIO_MPEG) {
		mpeg2ps_close(ps);
		return import_message(import, M4NotSupported, "Audio format not supported in MP4");
	}

	if (mpeg2ps_get_audio_frame(ps, streamID, &buf, &buf_len, TS_90000, NULL, NULL) == 0) {
		mpeg2ps_close(ps);
		return import_message(import, M4IOErr, "Cannot fetch audio frame from MPEG file");
	}
    
	hdr = FOUR_CHAR_INT(buf[0],buf[1],buf[2],buf[3]);
	mtype = MP3_GetObjectTypeIndication(hdr);
	sr = MP3_GetSamplingRate(hdr);
	nb_ch = MP3_GetNumChannels(hdr);

	destroy_esd = 0;
	esd = import->esd;
	if (!esd) {
		destroy_esd = 1;
		esd = OD_NewESDescriptor(0);
	}
	track = M4_NewTrack(import->dest, esd->ESID, M4_AudioMediaType, sr);
	e = M4_GetLastError(import->dest);
	if (!track) goto exit;
	M4_SetTrackEnabled(import->dest, track, 1);
	if (!esd->ESID) esd->ESID = M4_GetTrackID(import->dest, track);

	if (!esd->decoderConfig) esd->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
	if (!esd->slConfig) esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
	esd->slConfig->timestampResolution = sr;
	if (esd->decoderConfig->decoderSpecificInfo) OD_DeleteDescriptor((Descriptor **) &esd->decoderConfig->decoderSpecificInfo);
	esd->decoderConfig->streamType = M4ST_AUDIO;
	esd->decoderConfig->objectTypeIndication = mtype;
	e = M4_NewStreamDescription(import->dest, track, esd, NULL, NULL, &di);
	if (e) goto exit;

	M4_SetAudioDescriptionInfo(import->dest, track, di, sr, nb_ch, 16);
	import_message(import, M4OK, "%s Audio import - sample rate %d - %d channel%s", (mtype==0x6B) ? "MPEG-1" : "MPEG-2", sr, nb_ch, (nb_ch>1) ? "s" : "");


	duration = (u32) ((Double)import->duration/1000.0 * sr);

	samp = M4_NewSample();
	samp->IsRAP = 1;
	samp->DTS = 0;

	frames = 0;
	do {
		samp->data = buf;
		samp->dataLength = buf_len;
		M4_AddSample(import->dest, track, di, samp);
		samp->DTS += MP3_GetSamplesPerFrame(hdr);
		import_progress(import, frames, frames+1);
		frames++;
		if (duration && (samp->DTS>=duration)) break;
		if (import->flags & M4TI_DO_ABORT) break;
	}  while (mpeg2ps_get_audio_frame(ps, streamID, &buf, &buf_len, TS_90000, NULL, NULL));

	samp->data = NULL;
	M4_DeleteSample(&samp);
	import_progress(import, frames, frames);

exit:
	if (destroy_esd) OD_DeleteDescriptor((Descriptor **) &esd);
	mpeg2ps_close(ps);
	return e;
}

/*FIXME - need LARGE FILE support in NHNT - add a new version*/
M4Err MP4T_ImportNHNT(M4TrackImporter *import)
{
	M4Err e;
	Bool destroy_esd;
	u32 track, di, mtype, max_size, duration, count, w, h;
	M4Sample *samp;
	ESDescriptor *esd;
	s64 media_size, media_done, offset;
	BitStream *bs;
	FILE *nhnt, *mdia, *info;
	char *ext, szName[1000], szMedia[1000], szNhnt[1000];

	strcpy(szName, import->in_name);
	ext = strrchr(szName, '.');
	if (ext) ext[0] = 0;

	strcpy(szMedia, szName);
	strcat(szMedia, ".nhnt");
	nhnt = fopen(szMedia, "rb");
	if (!nhnt) return import_message(import, M4URLNotFound, "Cannot find NHNT file %s", szMedia);

	strcpy(szMedia, szName);
	strcat(szMedia, ".media");
	mdia = fopen(szMedia, "rb");
	if (!mdia) {
		fclose(nhnt);
		return import_message(import, M4URLNotFound, "Cannot find MEDIA file %s", szMedia);
	}
	

	e = M4OK;
	destroy_esd = 0;
	esd = import->esd;
	if (!esd) {
		esd = OD_NewESDescriptor(2);
		destroy_esd = 1;
	}
	/*update stream type/oti*/
	if (!esd->decoderConfig) esd->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
	if (!esd->slConfig) esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);


	strcpy(szNhnt, szName);
	strcat(szNhnt, ".info");
	info = fopen(szNhnt, "rb");
	if (esd->decoderConfig->decoderSpecificInfo) OD_DeleteDescriptor((Descriptor **) &esd->decoderConfig->decoderSpecificInfo);
	if (info) {
		esd->decoderConfig->decoderSpecificInfo = (DefaultDescriptor *) OD_NewDescriptor(DecoderSpecificInfo_Tag);
		fseek(info, 0, SEEK_END);
		esd->decoderConfig->decoderSpecificInfo->dataLength = (u32) ftell(info);
		esd->decoderConfig->decoderSpecificInfo->data = malloc(sizeof(char) * esd->decoderConfig->decoderSpecificInfo->dataLength);
		fseek(info, 0, SEEK_SET);
		fread(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, 1, info);
		fclose(info);
	}

	bs = NewBitStreamFromFile(nhnt, BS_FILE_READ);
	if ((BS_ReadInt(bs, 8)!='N') || (BS_ReadInt(bs, 8) != 'H') || (BS_ReadInt(bs, 8) != 'n')|| (BS_ReadInt(bs, 8) != 't')) {
		import_message(import, M4NonCompliantBitStream, "Invalid NHNT signature");
		e = M4NonCompliantBitStream;
		goto exit;
	}
	/*version*/
	BS_ReadInt(bs, 8);
	esd->decoderConfig->streamType = BS_ReadInt(bs, 8);
	esd->decoderConfig->objectTypeIndication = BS_ReadInt(bs, 8);
	BS_ReadInt(bs, 16);
	esd->decoderConfig->bufferSizeDB = BS_ReadInt(bs, 24);
	esd->decoderConfig->avgBitrate = BS_ReadInt(bs, 32);
	esd->decoderConfig->maxBitrate = BS_ReadInt(bs, 32);
	esd->slConfig->timestampResolution = BS_ReadInt(bs, 32);
	
	w = h = 0;
	switch (esd->decoderConfig->streamType) {
	case M4ST_SCENE:
		mtype = M4_BIFSMediaType;
		/*we don't know PLs from NHNT...*/
		M4_SetMoviePLIndication(import->dest, M4_PL_SCENE, 0xFE);
		M4_SetMoviePLIndication(import->dest, M4_PL_GRAPHICS, 0xFE);
		break;
	case M4ST_VISUAL:
		mtype = M4_VisualMediaType;
		if (esd->decoderConfig->objectTypeIndication==0x20) {
			M4VDecoderSpecificInfo dsi;
			if (!esd->decoderConfig->decoderSpecificInfo) {
				e = M4NonCompliantBitStream;
				goto exit;
			}
			e = M4V_GetConfig(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, &dsi);
			if (e) goto exit;
			w = dsi.width;
			h = dsi.height;
		}
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
	/*note we cannot import OD, OCR and interaction streams from NHNT*/
	default:
		e = M4NotSupported;
		goto exit;
	}
	track = M4_NewTrack(import->dest, esd->ESID, mtype, esd->slConfig->timestampResolution);
	if (!track) {
		e = M4_GetLastError(import->dest);
		goto exit;
	}
	M4_SetTrackEnabled(import->dest, track, 1);
	if (!esd->ESID) esd->ESID = M4_GetTrackID(import->dest, track);
	e = M4_NewStreamDescription(import->dest, track, esd, (import->flags & M4TI_USE_DATAREF) ? szMedia : NULL, NULL, &di);
	if (e) goto exit;

	if (w && h) M4_SetVisualEntrySize(import->dest, track, di, w, h);

	import_message(import, M4OK, "NHNT import - Stream Type %s - ObjectTypeIndication %d", OD_GetStreamTypeName(esd->decoderConfig->streamType), esd->decoderConfig->objectTypeIndication);

	duration = import->duration*esd->slConfig->timestampResolution;
	duration /= 1000;

	samp = M4_NewSample();
	samp->data = malloc(sizeof(char) * 1024);
	max_size = 1024;
	count = 0;
	fseek(mdia, 0, SEEK_END);
	media_size = ftell(mdia);
	fseek(mdia, 0, SEEK_SET);
	media_done = 0;

	while (!feof(nhnt)) {
		samp->dataLength = BS_ReadInt(bs, 24);
		samp->IsRAP = BS_ReadInt(bs, 1);
		BS_ReadInt(bs, 7);
		offset = BS_ReadInt(bs, 32);
		samp->CTS_Offset = BS_ReadInt(bs, 32);
		samp->DTS = BS_ReadInt(bs, 32);
		samp->CTS_Offset -= samp->DTS;
		if (!count && samp->DTS) samp->DTS = 0;
		count++;

		if (import->flags & M4TI_USE_DATAREF) {
			M4_AddSampleReference(import->dest, track, di, samp, offset);
		} else {
			if (samp->dataLength>max_size) {
				samp->data = realloc(samp->data, sizeof(char) * samp->dataLength);
				max_size = samp->dataLength;
			}
			f64_seek(mdia, offset, SEEK_SET);
			fread( samp->data, samp->dataLength, 1, mdia); 
			M4_AddSample(import->dest, track, di, samp);
		}
		media_done += samp->dataLength;
		import_progress(import, (u32) media_done, (u32) media_size);
		if (!BS_Available(bs)) break;
		if (duration && (samp->DTS > duration)) break;
		if (import->flags & M4TI_DO_ABORT) break;
	}
	M4_DeleteSample(&samp);	

exit:
	DeleteBitStream(bs);
	fclose(nhnt);
	fclose(mdia);
	if (esd && destroy_esd) OD_DeleteDescriptor((Descriptor **) &esd);
	return e;
}


static const u32 amr_nb_frame_size[16] = {
	12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0
};

static const u32 amr_wb_frame_size[16] = {
	17, 23, 32, 36, 40, 46, 50, 58, 60, 5, 5, 0, 0, 0, 0, 0 
};

M4Err MP4T_ImportAMR(M4TrackImporter *import)
{
	M4Err e;
	u32 track, trackID, di, max_size, duration;
	M4Sample *samp;
	char magic[20];
	u32 media_size, media_done, offset;
	Bool is_wb;
	AMRConfiguration amr_cfg;
	FILE *mdia;

	mdia = fopen(import->in_name, "rb");
	if (!mdia) return import_message(import, M4URLNotFound, "Cannot find file %s", import->in_name);
	
	is_wb = 0;
	fread(magic, 1, 20, mdia);
	if (!strnicmp(magic, "#!AMR\n", 6)) {
		import_message(import, M4OK, "Importing AMR Audio");
		fseek(mdia, 6, SEEK_SET);
	}
	else if (!strnicmp(magic, "#!AMR_MC1.0\n", 12)) {
		fclose(mdia);
		return import_message(import, M4NotSupported, "Multichannel AMR Audio Not Supported");
	}
	else if (!strnicmp(magic, "#!AMR-WB\n", 9)) {
		is_wb = 1;
		import_message(import, M4OK, "Importing AMR WideBand Audio");
		fseek(mdia, 9, SEEK_SET);
	}
	else if (!strnicmp(magic, "#!AMR-WB_MC1.0\n", 15)) {
		fclose(mdia);
		return import_message(import, M4NotSupported, "Multichannel AMR WideBand Audio Not Supported");
	}
	else {
		fclose(mdia);
		return import_message(import, M4NonCompliantBitStream, "Corrupted AMR file header (expecting '#!AMR...')");
	}

	trackID = 0;
	e = M4OK;
	if (import->esd) trackID = import->esd->ESID;

	track = M4_NewTrack(import->dest, trackID, M4_AudioMediaType, is_wb ? 16000 : 8000);
	if (!track) {
		e = M4_GetLastError(import->dest);
		goto exit;
	}
	M4_SetTrackEnabled(import->dest, track, 1);
	if (import->esd && !import->esd->ESID) import->esd->ESID = M4_GetTrackID(import->dest, track);

	memset(&amr_cfg, 0, sizeof(AMRConfiguration));
	amr_cfg.WideBandAMR = is_wb;
	amr_cfg.frames_per_sample = 1;
	amr_cfg.vendor = FOUR_CHAR_INT('G', 'P', 'A', 'C');
	
	e = M4_AMR_NewStreamConfig(import->dest, track, &amr_cfg, (import->flags & M4TI_USE_DATAREF) ? import->in_name : NULL, NULL, &di);
	if (e) goto exit;
	M4_SetAudioDescriptionInfo(import->dest, track, di, is_wb ? 16000 : 8000, 1, 8);

	duration = import->duration * (is_wb ? 16000 : 8000);
	duration /= 1000;

	samp = M4_NewSample();
	samp->data = malloc(sizeof(char) * 200);
	samp->IsRAP = 1;
	max_size = 200;
	offset = ftell(mdia);
	fseek(mdia, 0, SEEK_END);
	media_size = ftell(mdia) - offset;
	fseek(mdia, offset, SEEK_SET);

	media_done = 0;

	while (!feof(mdia)) {
		u8 ft, toc;
		
		offset = ftell(mdia);
		toc = fgetc(mdia);
		ft = (toc >> 3) & 0x0F;
		
		if (is_wb) {
	        samp->dataLength = amr_wb_frame_size[ft];
		} else {
	        samp->dataLength = amr_nb_frame_size[ft];
		}
		samp->data[0] = toc;

		if (samp->dataLength) 
			fread( samp->data + 1, samp->dataLength, 1, mdia);
		
		samp->dataLength += 1;

		if (import->flags & M4TI_USE_DATAREF) {
			M4_AddSampleReference(import->dest, track, di, samp, offset);
		} else {
			M4_AddSample(import->dest, track, di, samp);
		}

		samp->DTS += is_wb ? 320 : 160;
		media_done += samp->dataLength;
		import_progress(import, (u32) media_done, (u32) media_size);
		if (duration && (samp->DTS > duration)) break;
		if (import->flags & M4TI_DO_ABORT) break;
	}
	M4_DeleteSample(&samp);	

exit:
	fclose(mdia);
	return e;
}

#define OGG_BUFFER_SIZE 4096

Bool OGG_ReadPage(FILE *f_in, ogg_sync_state *oy, ogg_page *oggpage)
{
	if (feof(f_in)) return 0;
	while (ogg_sync_pageout(oy, oggpage ) != 1 ) {
		char *buffer = ogg_sync_buffer(oy, OGG_BUFFER_SIZE);
		u32 bytes = fread(buffer, 1, OGG_BUFFER_SIZE, f_in);
		ogg_sync_wrote(oy, bytes);
		if (feof(f_in)) return 1;
	}
	return 1;
}

M4Err MP4T_ImportOggAudio(M4TrackImporter *import)
{
	M4Err e;
    ogg_sync_state oy;
	u32 di, track, done, tot_size, duration;
	s32 block_size;
	M4Sample *samp;
	Bool destroy_esd, go;
	u32 serial_no, num_headers;
	ESDescriptor *esd;
	ogg_packet oggpacket;
	ogg_page oggpage;
	ogg_stream_state os;
	VorbisParser vp;
	BitStream *vbs;
	FILE *f_in;

	if (import->flags & M4TI_USE_DATAREF) return M4NotSupported;


	/*video only, ignore*/
	if (import->trackID==1) return M4OK;

	f_in = fopen(import->in_name, "rb");
	if (!f_in) return import_message(import, M4URLNotFound, "Opening file %s failed", import->in_name);

	e = M4OK;

	done = 0;
	fseek(f_in, 0, SEEK_END);
	tot_size = ftell(f_in);
	fseek(f_in, 0, SEEK_SET);

	destroy_esd = 0;
	esd = import->esd;
	samp = M4_NewSample();
	/*avoids gcc warnings*/
	track = num_headers = duration = 0;

    ogg_sync_init(&oy);

	vbs = NULL;
	serial_no = 0;
	go = 1;
	while (go) {
		if (!OGG_ReadPage(f_in, &oy, &oggpage)) break;

		if (ogg_page_bos(&oggpage)) {
			if (serial_no) continue;
			serial_no = ogg_page_serialno(&oggpage);
			ogg_stream_init(&os, serial_no);
			ogg_stream_pagein(&os, &oggpage);
			ogg_stream_packetpeek(&os, &oggpacket);
			if ((oggpacket.bytes < 7) || strncmp(&oggpacket.packet[1], "vorbis", 6)) {
				ogg_stream_clear(&os);
				serial_no = 0;
				continue;
			}
			num_headers = 0;
			memset(&vp, 0, sizeof(VorbisParser));
			vbs = NewBitStream(NULL, 0, BS_WRITE);
			continue;
		}
		/*FIXME - check end of stream for concatenated files?*/

		/*not our stream*/
        if (ogg_stream_pagein(&os, &oggpage) != 0) continue;
	


		while (ogg_stream_packetout(&os, &oggpacket ) > 0 ) {
			if (num_headers<3) {
				if (!VorbisParseHeader(&vp, oggpacket.packet, oggpacket.bytes)) {
					e = import_message(import, M4NonCompliantBitStream, "Corrupted OGG Vorbis header");
					goto exit;					
				}

				/*copy headers*/
				BS_WriteInt(vbs, oggpacket.bytes, 16);
				BS_WriteData(vbs, oggpacket.packet, oggpacket.bytes);
				num_headers++;

				/*let's go, create the track*/
				if (num_headers==3) {
					if (!vp.is_init) {
						e = import_message(import, M4NonCompliantBitStream, "Corrupted OGG Vorbis headers found");
						goto exit;					
					}

					import_message(import, M4OK, "OGG Vorbis import - sample rate %d - %d channel%s", vp.sample_rate, vp.channels, (vp.channels>1) ? "s" : "");

					if (!esd) {
						destroy_esd = 1;
						esd = OD_NewESDescriptor(0);
					}
					track = M4_NewTrack(import->dest, esd->ESID, M4_AudioMediaType, vp.sample_rate);
					if (!track) goto exit;
					M4_SetTrackEnabled(import->dest, track, 1);
					if (!esd->ESID) esd->ESID = M4_GetTrackID(import->dest, track);
					if (!esd->decoderConfig) esd->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
					if (!esd->slConfig) esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
					esd->slConfig->timestampResolution = vp.sample_rate;
					if (!esd->decoderConfig->decoderSpecificInfo) esd->decoderConfig->decoderSpecificInfo = (DefaultDescriptor *) OD_NewDescriptor(DecoderSpecificInfo_Tag);
					BS_GetContent(vbs, (unsigned char **) &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
					DeleteBitStream(vbs);
					vbs = NULL;
					esd->decoderConfig->streamType = M4ST_AUDIO;
					esd->decoderConfig->avgBitrate = vp.avg_r;
					esd->decoderConfig->maxBitrate = (vp.max_r>0) ? vp.max_r : vp.avg_r;
					esd->decoderConfig->bufferSizeDB = 0;
					esd->decoderConfig->objectTypeIndication = 0xDE;

					e = M4_NewStreamDescription(import->dest, track, esd, NULL, NULL, &di);
					if (e) goto exit;
					M4_SetAudioDescriptionInfo(import->dest, track, di, vp.sample_rate, vp.channels, 16);

					{
						Double d = import->duration;
						d *= vp.sample_rate;
						d /= 1000;
						duration = (u32) d;
					}
				}
				continue;
			}
		
			block_size = VorbisCheckFrame(&vp, oggpacket.packet, oggpacket.bytes);
			if (!block_size) continue;

			/*add packet*/
			samp->IsRAP = 1;
			samp->data = oggpacket.packet;
			samp->dataLength = oggpacket.bytes;
			e = M4_AddSample(import->dest, track, di, samp);
			samp->DTS += block_size;

			if (esd->decoderConfig->bufferSizeDB < samp->dataLength) esd->decoderConfig->bufferSizeDB = samp->dataLength;

			done += oggpacket.bytes;
			import_progress(import, done, tot_size);
			if ((duration && (samp->DTS > duration) ) || (import->flags & M4TI_DO_ABORT)) {
				go = 0;
				break;
			}
		}
	}

	if (!serial_no) {
		import_message(import, M4OK, "OGG: No supported audio found");
	} else {
		M4_ChangeStreamDescriptor(import->dest, track, di, esd);
		samp->data = NULL;
		M4_SetMoviePLIndication(import->dest, M4_PL_AUDIO, 0xFE);
		import_progress(import, tot_size, tot_size);
	}
	
exit:
	M4_DeleteSample(&samp);
	if (vbs) DeleteBitStream(vbs);
    ogg_sync_clear(&oy);
	if (esd && destroy_esd) OD_DeleteDescriptor((Descriptor **) &esd);
	fclose(f_in);
	return e;
}

M4Err MP4T_ImportOggVideo(M4TrackImporter *import)
{
	M4Err e;
    ogg_sync_state oy;
	u32 di, track, done, tot_size, duration;
	u32 w, h, fps_num, fps_den, keyframe_freq_force, theora_kgs, flag;
	Float frame_rate;
	Bool destroy_esd, go;
	u32 serial_no, num_headers;
	ESDescriptor *esd;
	ogg_packet oggpacket;
	ogg_page oggpage;
	ogg_stream_state os;
	oggpack_buffer opb;
	BitStream *bs;
	FILE *f_in;
	M4Sample *samp;

	if (import->flags & M4TI_USE_DATAREF) return M4NotSupported;


	/*video only, ignore*/
	if (import->trackID>1) return M4OK;

	f_in = fopen(import->in_name, "rb");
	if (!f_in) return import_message(import, M4URLNotFound, "Opening file %s failed", import->in_name);

	e = M4OK;
	done = 0;
	fseek(f_in, 0, SEEK_END);
	tot_size = ftell(f_in);
	fseek(f_in, 0, SEEK_SET);


	destroy_esd = 0;
	esd = import->esd;
	samp = M4_NewSample();
	
	/*avoids gcc warnings*/
	duration = 0;
	frame_rate = 0;
	num_headers = w = h = track = 0;

    ogg_sync_init(&oy);

	bs = NULL;
	serial_no = 0;
	go = 1;
	while (go) {
		if (!OGG_ReadPage(f_in, &oy, &oggpage)) break;

		if (ogg_page_bos(&oggpage)) {
			if (serial_no) continue;
			serial_no = ogg_page_serialno(&oggpage);
			ogg_stream_init(&os, serial_no);
			ogg_stream_pagein(&os, &oggpage);
			ogg_stream_packetpeek(&os, &oggpacket);

			if ((oggpacket.bytes < 7) || strncmp(&oggpacket.packet[1], "theora", 6)) {
				ogg_stream_clear(&os);
				serial_no = 0;
				continue;
			}
			/*that's ogg-theora*/
			bs = NewBitStream(oggpacket.packet, oggpacket.bytes, BS_READ);
			BS_ReadInt(bs, 80);
			w = BS_ReadInt(bs, 16) << 4;
			h = BS_ReadInt(bs, 16) << 4;
			BS_ReadInt(bs, 64);
			fps_num = BS_ReadInt(bs, 32);
			fps_den = BS_ReadInt(bs, 32);
			BS_ReadInt(bs, 80);
			BS_ReadInt(bs, 6);
			keyframe_freq_force = 1 << BS_ReadInt(bs, 5);
			theora_kgs = 0;
			keyframe_freq_force--;
			while (keyframe_freq_force) {
				theora_kgs ++;
				keyframe_freq_force >>= 1;
			}

		    frame_rate = ((Float)fps_num) / fps_den;
			num_headers = 0;

			import_message(import, M4OK, "OGG Theora import - %.3f FPS - Resolution %d x %d", frame_rate, w, h);
			bs = NewBitStream(NULL, 0, BS_WRITE);
			continue;
		}
		/*FIXME - check end of stream for concatenated files?*/

		/*not our stream*/
        if (ogg_stream_pagein(&os, &oggpage) != 0) continue;
	


		while (ogg_stream_packetout(&os, &oggpacket ) > 0 ) {
			if (num_headers<3) {
				if(!w || !h) {
					e = import_message(import, M4NonCompliantBitStream, "Couldn't find Theora main header");
					goto exit;
				}
				/*copy headers*/
				BS_WriteInt(bs, oggpacket.bytes, 16);
				BS_WriteData(bs, oggpacket.packet, oggpacket.bytes);
				num_headers++;

				/*let's go, create the track*/
				if (num_headers==3) {
					if (!esd) {
						destroy_esd = 1;
						esd = OD_NewESDescriptor(0);
					}
					track = M4_NewTrack(import->dest, esd->ESID, M4_VisualMediaType, (u32) (1000*frame_rate) );
					if (!track) goto exit;
					M4_SetTrackEnabled(import->dest, track, 1);
					if (!esd->ESID) esd->ESID = M4_GetTrackID(import->dest, track);
					if (!esd->decoderConfig) esd->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
					if (!esd->slConfig) esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
					esd->slConfig->timestampResolution = (u32) (1000*frame_rate);
					if (!esd->decoderConfig->decoderSpecificInfo) esd->decoderConfig->decoderSpecificInfo = (DefaultDescriptor *) OD_NewDescriptor(DecoderSpecificInfo_Tag);
					BS_GetContent(bs, (unsigned char **) &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
					DeleteBitStream(bs);
					bs = NULL;
					esd->decoderConfig->streamType = M4ST_VISUAL;
					esd->decoderConfig->avgBitrate = 0;
					esd->decoderConfig->maxBitrate = 0;
					esd->decoderConfig->bufferSizeDB = 0;
					esd->decoderConfig->objectTypeIndication = 0xDF;

					e = M4_NewStreamDescription(import->dest, track, esd, NULL, NULL, &di);
					if (e) goto exit;
					M4_SetVisualEntrySize(import->dest, track, di, w, h);

					{
						Double d = import->duration;
						d *= esd->slConfig->timestampResolution;
						d /= 1000;
						duration = (u32) d;
					}
				}
				
				continue;
			}
			
			/*we don't need adedicated parser for theora, just check it's a theora frame and get its key type 
			WATCHOUT theora bitsteram is in BE*/
			oggpackB_readinit(&opb, oggpacket.packet, oggpacket.bytes);
			flag = oggpackB_read(&opb, 1);
			if (flag==0) {
				/*add packet*/
				samp->IsRAP = oggpackB_read(&opb, 1) ? 0 : 1;
				samp->data = oggpacket.packet;
				samp->dataLength = oggpacket.bytes;
				e = M4_AddSample(import->dest, track, di, samp);
				samp->DTS += 1000;
				if (esd->decoderConfig->bufferSizeDB < samp->dataLength) esd->decoderConfig->bufferSizeDB = samp->dataLength;
			}

			done += oggpacket.bytes;
			import_progress(import, done, tot_size);
			if ((duration && (samp->DTS > duration) ) || (import->flags & M4TI_DO_ABORT)) {
				go = 0;
				break;
			}
		}
	}

	if (!serial_no) {
		import_message(import, M4OK, "OGG: No supported video found");
	} else {
		M4_ChangeStreamDescriptor(import->dest, track, di, esd);
		M4_SetMoviePLIndication(import->dest, M4_PL_VISUAL, 0xFE);
		import_progress(import, tot_size, tot_size);
	}
	
exit:
	if (bs) DeleteBitStream(bs);
	samp->data = NULL;
	M4_DeleteSample(&samp);
    ogg_sync_clear(&oy);
	if (esd && destroy_esd) OD_DeleteDescriptor((Descriptor **) &esd);
	fclose(f_in);
	return e;
}


M4Err MP4T_ImportMedia(M4TrackImporter *importer)
{
	M4Err e;
	char *ext;
	if (!importer || !importer->dest || (!importer->in_name && !importer->orig) ) return M4BadParam;

	if (importer->orig) return MP4T_ImportMP4(importer);

	ext = strrchr(importer->in_name, '.');
	if (!ext) return import_message(importer, M4BadParam, "Unknown input file type");

	if (!strnicmp(ext, ".avi", 4)) {
		e = MP4T_ImportAVIVideo(importer);
		if (e) return e;
		return MP4T_ImportAVIAudio(importer);
	}
	else if (!strnicmp(ext, ".ogg", 4)) {
		e = MP4T_ImportOggVideo(importer);
		if (e) return e;
		return MP4T_ImportOggAudio(importer);
	}
	else if (!strnicmp(ext, ".mpg", 4) || !strnicmp(ext, ".mpeg", 5) 
		|| !strnicmp(ext, ".vob", 4) || !strnicmp(ext, ".vcd", 4) || !strnicmp(ext, ".svcd", 5)
		) {
		e = MP4T_ImportMPEG12Video(importer);
		if (e) return e;
		return MP4T_ImportMPEG12Audio(importer);
	}

	else if (!strnicmp(ext, ".mp3", 4)) {
		return MP4T_ImportMP3(importer);
	}
	else if (!strnicmp(ext, ".media", 5) || !strnicmp(ext, ".info", 5) || !strnicmp(ext, ".nhnt", 5)) {
		return MP4T_ImportNHNT(importer);
	}
	else if (!strnicmp(ext, ".jpg", 4) || !strnicmp(ext, ".jpeg", 5)) {
		return MP4T_ImportStillImage(importer, 0x6C);
	}
	else if (!strnicmp(ext, ".png", 4)) {
		return MP4T_ImportStillImage(importer, 0x6D);
	}
	else if (!strnicmp(ext, ".aac", 4)) {
		return MP4T_ImportAAC_ADTS(importer);
	}
	else if (!strnicmp(ext, ".amr", 4)) {
		return MP4T_ImportAMR(importer);
	}
	else if (!strnicmp(ext, ".srt", 4) ) {
		M4Err MP4T_ImportSRT_Text(M4TrackImporter *import);
		return MP4T_ImportSRT_Text(importer);
	}
	else if (!strnicmp(ext, ".ttxt", 5) ) {
		M4Err MP4T_ImportTimedText(M4TrackImporter *import);
		return MP4T_ImportTimedText(importer);
	}
	/*try with MP4*/
	else {
		M4Err res;
		importer->orig = M4_MovieOpen(importer->in_name, M4_OPEN_READ);
		if (!importer->orig) return M4_GetLastError(NULL);
		res = MP4T_ImportMP4(importer);
		M4_MovieDelete(importer->orig);
		return res;
	}
	return M4NotSupported;
}


#endif



