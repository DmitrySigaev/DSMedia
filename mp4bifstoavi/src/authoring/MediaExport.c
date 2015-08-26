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
#include <m4_bifs.h>
#include <m4_scenegraph.h>
#include <intern/avilib.h>
#include <m4_ogg.h>

M4Err dump_message(M4TrackDumper *dumper, M4Err e, char *format, ...)
{
	va_list args;
	va_start(args, format);
	if (dumper->dump_message) {
		char szMsg[1024];
		vsprintf(szMsg, format, args);
		dumper->dump_message(dumper, e, szMsg);
	} else {
		vfprintf(stderr,format,args);
		if (e) fprintf(stderr, "\nError: %s", M4ErrToString(e));
		fprintf(stderr, "\n");
	}
	va_end(args);
	return e;
}

void dump_progress(M4TrackDumper *dumper, u32 cur_samp, u32 count)
{
	if (dumper->dump_progress) {
		dumper->dump_progress(dumper, cur_samp, count);
	} else {
		fprintf(stdout, "Extracting sample %d / %d (%.2f)\r", cur_samp, count, ((Float)cur_samp)*100 / count);
		if (cur_samp==count) fprintf(stderr, "\n");
	}
}

/*that's very very crude, we only support vorbis & theora in MP4 - this will need cleanup as soon as possible*/
static M4Err dump_to_ogg(M4TrackDumper *dumper, char *szName, u32 track)
{
	FILE *out;
	ogg_stream_state os;
	ogg_packet op;
	ogg_page og;
	u32 count, i, di, theora_kgs, nb_i, nb_p;
	BitStream *bs;
	M4Sample *samp;
	ESDescriptor *esd = M4_GetStreamDescriptor(dumper->file, track, 1);


	M4_RandInit(1);
	ogg_stream_init(&os, M4_Rand());

	op.granulepos = 0;
	op.packetno = 0;
	op.b_o_s = 1;
	op.e_o_s = 0;

	out = fopen(szName, "wb");
	if (!out) return dump_message(dumper, M4IOErr, "Error opening %s for writing - check disk access & permissions", szName);

	theora_kgs = 0;
	bs = NewBitStream(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, BS_READ);
	while (BS_Available(bs)) {
		op.bytes = BS_ReadInt(bs, 16);
		op.packet = malloc(sizeof(char) * op.bytes);
		BS_ReadData(bs, op.packet, op.bytes);
		ogg_stream_packetin(&os, &op);

		if (op.b_o_s) {
			ogg_stream_pageout(&os, &og);
			fwrite(og.header, 1, og.header_len, out);
			fwrite(og.body, 1, og.body_len, out);
			op.b_o_s = 0;

			if (esd->decoderConfig->objectTypeIndication==0xDF) {
				u32 kff;
				BitStream *vbs = NewBitStream(op.packet, op.bytes, BS_READ);
				BS_SkipBytes(vbs, 40);
				BS_ReadInt(vbs, 6); /* quality */
				kff = 1 << BS_ReadInt(vbs, 5);
				DeleteBitStream(vbs);

				theora_kgs = 0;
				kff--;
				while (kff) {
					theora_kgs ++;
					kff >>= 1;
				}
			}
		}
		free(op.packet);
		op.packetno ++;
	}
	DeleteBitStream(bs);
	OD_DeleteDescriptor((Descriptor **)&esd);

	while (ogg_stream_pageout(&os, &og)>0) {
		fwrite(og.header, 1, og.header_len, out);
		fwrite(og.body, 1, og.body_len, out);
	}
	
	op.granulepos = -1;

	count = M4_GetSampleCount(dumper->file, track);

	nb_i = nb_p = 0;
	samp = M4_GetSample(dumper->file, track, 1, &di);
	for (i=0; i<count; i++) {
		M4Sample *next_samp = M4_GetSample(dumper->file, track, i+2, &di);
		if (!samp) break;
		op.bytes = samp->dataLength;
		op.packet = samp->data;
		op.packetno ++;

		if (theora_kgs) {
			if (samp->IsRAP) {
				if (i) nb_i+=nb_p+1;
				nb_p = 0;
			} else {
				nb_p++;
			}
			op.granulepos = nb_i;
			op.granulepos <<= theora_kgs;
			op.granulepos |= nb_p;
		} else {
			if (next_samp) op.granulepos = next_samp->DTS;
		}
		if (!next_samp) op.e_o_s = 1;

		ogg_stream_packetin(&os, &op);

		M4_DeleteSample(&samp);
		samp = next_samp;
		next_samp = NULL;
		dump_progress(dumper, i+1, count);
		if (dumper->do_abort) break;

		while (ogg_stream_pageout(&os, &og)>0) {
			fwrite(og.header, 1, og.header_len, out);
			fwrite(og.body, 1, og.body_len, out);
		}
	}
	if (samp) M4_DeleteSample(&samp);

	while (ogg_stream_flush(&os, &og)>0) {
		fwrite(og.header, 1, og.header_len, out);
		fwrite(og.body, 1, og.body_len, out);
	}
    ogg_stream_clear(&os);
	fclose(out);
	return M4OK;
}

M4Err MP4T_DumpTrackNative(M4TrackDumper *dumper)
{
	DecoderConfigDescriptor *dcfg;
	UnknownStreamDescription *udesc;
	char szName[1000], szEXT[5], szType[5];
	FILE *out;
	M4ADecoderSpecificInfo a_cfg;
	BitStream *bs;
	u32 track, i, di, count, m_type, m_stype, dsi_size;
	Bool is_ogg;
	u32 aac_type, is_aac;
	char *dsi;

	dsi_size = 0;
	dsi = NULL;

	track = M4_GetTrackByID(dumper->file, dumper->trackID);
	m_type = M4_GetMediaType(dumper->file, track);
	m_stype = M4_GetMediaSubType(dumper->file, track, 1);

	is_aac = aac_type = 0;
	is_ogg = 0;
	udesc = NULL;
	dcfg = M4_GetDecoderInformation(dumper->file, track, 1);
	strcpy(szName, dumper->out_name);
	if (dcfg) {
		switch (dcfg->streamType) {
		case M4ST_VISUAL:
			switch (dcfg->objectTypeIndication) {
			case 0x20:
				dsi = dcfg->decoderSpecificInfo->data;
				dcfg->decoderSpecificInfo->data = NULL;
				dsi_size = dcfg->decoderSpecificInfo->dataLength;
				strcat(szName, ".cmp");
				dump_message(dumper, M4OK, "Extracting MPEG-4 Visual stream to cmp");
				break;
			case 0x6C:
				strcat(szName, ".jpg");
				dump_message(dumper, M4OK, "Extracting JPEG image");
				break;
			case 0x6D:
				strcat(szName, ".png");
				dump_message(dumper, M4OK, "Extracting PNG image");
				break;
			case 0xDF:
				strcat(szName, ".ogg");
				dump_message(dumper, M4OK, "Extracting Ogg Theora video");
				is_ogg = 1;
				break;
			default:
				OD_DeleteDescriptor((Descriptor **) &dcfg);
				return dump_message(dumper, M4NotSupported, "Unknown media in track ID %d - use NHNT instead", dumper->trackID);
			}
			break;
		case M4ST_AUDIO:
			switch (dcfg->objectTypeIndication) {
			case 0x66:
			case 0x67:
			case 0x68:
				dsi = dcfg->decoderSpecificInfo->data;
				dcfg->decoderSpecificInfo->data = NULL;
				dsi_size = dcfg->decoderSpecificInfo->dataLength;
				strcat(szName, ".aac");
				is_aac = 1;
				aac_type = dcfg->objectTypeIndication - 0x66;
				dump_message(dumper, M4OK, "Extracting MPEG-2 AAC");
				break;
			case 0x40:
				dsi = dcfg->decoderSpecificInfo->data;
				dcfg->decoderSpecificInfo->data = NULL;
				dsi_size = dcfg->decoderSpecificInfo->dataLength;
				is_aac = 2;
				strcat(szName, ".aac");
				dump_message(dumper, M4OK, "Extracting MPEG-4 AAC");
				break;
			case 0x69:
			case 0x6B:
				strcat(szName, ".mp3");
				dump_message(dumper, M4OK, "Extracting MPEG-1/2 Audio (MP3)");
				break;
			case 0xDE:
				strcat(szName, ".ogg");
				is_ogg = 1;
				dump_message(dumper, M4OK, "Extracting Ogg Vorbis audio");
				break;
			default:
				OD_DeleteDescriptor((Descriptor **) &dcfg);
				return dump_message(dumper, M4NotSupported, "Unknown audio in track ID %d - use NHNT", dumper->trackID);
			}
			break;
		default:
			OD_DeleteDescriptor((Descriptor **) &dcfg);
			return dump_message(dumper, M4NotSupported, "Cannot extract systems track ID %d in raw format - use NHNT", dumper->trackID);
		}
		OD_DeleteDescriptor((Descriptor **) &dcfg);
	} else {
		if ((m_stype==M4_AMR_SubType) || (m_stype==M4_AMR_WB_SubType)) {
			strcat(szName, ".amr");
			dump_message(dumper, M4OK, "Extracting AMR Audio");
		} else if (m_stype==M4_H263_SubType) {
			dump_message(dumper, M4OK, "Extracting H263 Video");
			strcat(szName, ".263");
		} else if (m_type==M4_FlashMediaType) {
			dump_message(dumper, M4OK, "Extracting Macromedia Flash Movie");
			strcat(szName, ".swf");
		} else {
			szEXT[4] = 0;
			MP4TypeToString(m_stype, szEXT);
			strcat(szName, ".");
			strcat(szName, szEXT);
			udesc = M4_GetUnknownStreamDescription(dumper->file, track, 1);
			if (udesc) {
				dsi = udesc->extension_buf; udesc->extension_buf = NULL;
				dsi_size = udesc->extension_buf_size;
			}
			switch (m_type) {
			case M4_VisualMediaType: dump_message(dumper, M4OK, "Extracting \'%s\' Video - Compressor %s", szEXT, udesc ? udesc->szCompressorName : "Unknown"); break;
			case M4_AudioMediaType: dump_message(dumper, M4OK, "Extracting \'%s\' Audio - Compressor %s", szEXT, udesc ? udesc->szCompressorName : "Unknown"); break;
			default:
				szType[4] = 0;
				MP4TypeToString(m_type, szType);
				dump_message(dumper, M4OK, "Extracting \'%s\' Track (type '%s') - Compressor %s", szEXT, szType, udesc ? udesc->szCompressorName : "Unknown");
				break;
			}
			if (udesc) free(udesc);
		}
	}

	if (is_ogg) return dump_to_ogg(dumper, szName, track);

	out = fopen(szName, "wb");
	if (!out) {
		if (dsi) free(dsi);
		return dump_message(dumper, M4IOErr, "Error opening %s for writing - check disk access & permissions", szName);
	}
	bs = NewBitStreamFromFile(out, BS_FILE_WRITE_ONLY);

	if (is_aac) {
		M4A_GetConfig(dsi, dsi_size, &a_cfg);
		if (is_aac==2) aac_type = a_cfg.base_object_type - 1;
		free(dsi);
		dsi = NULL;
	}
	if (dsi) {
		BS_WriteData(bs, dsi, dsi_size);
		free(dsi);
	}

	if (m_stype==M4_AMR_SubType) BS_WriteData(bs, "#!AMR\n", 6);
	else if (m_stype==M4_AMR_WB_SubType) BS_WriteData(bs, "#!AMR-WB\n", 9);

	count = M4_GetSampleCount(dumper->file, track);
	for (i=0; i<count; i++) {
		M4Sample *samp = M4_GetSample(dumper->file, track, i+1, &di);
		if (!samp) break;
		/*adts frame header*/
		if (is_aac) {
			BS_WriteInt(bs, 0xFFF, 12);/*sync*/
			BS_WriteInt(bs, (is_aac==1) ? 1 : 0, 1);/*mpeg2 aac*/
			BS_WriteInt(bs, 0, 2); /*layer*/
			BS_WriteInt(bs, 1, 1); /* protection_absent*/
			BS_WriteInt(bs, aac_type, 2);
			BS_WriteInt(bs, a_cfg.base_sr_index, 4);
			BS_WriteInt(bs, 0, 1);
			BS_WriteInt(bs, a_cfg.nb_chan, 3);
			BS_WriteInt(bs, 0, 4);
			BS_WriteInt(bs, 7+samp->dataLength, 13);
			BS_WriteInt(bs, 0x7FF, 11);
			BS_WriteInt(bs, 0, 2);
		}
		/*NOTE: with AMR we could check the F bit of the frame header is set to 0...*/
		BS_WriteData(bs, samp->data, samp->dataLength);
		M4_DeleteSample(&samp);
		dump_progress(dumper, i+1, count);
		if (dumper->do_abort) break;
	}
	DeleteBitStream(bs);
	fclose(out);
	return dump_message(dumper, M4OK, "Extracting done");
}



M4Err MP4T_DumpTrackNHNT(M4TrackDumper *dumper)
{
	ESDescriptor *esd;
	char szName[1000];
	FILE *out_med, *out_inf, *out_nhnt;
	BitStream *bs;
	Bool has_b_frames;
	u32 track, i, di, count, pos;

	track = M4_GetTrackByID(dumper->file, dumper->trackID);
	esd = M4_GetStreamDescriptor(dumper->file, track, 1);
	if (!esd) return dump_message(dumper, M4NonCompliantBitStream, "Invalid MPEG-4 stream in track ID %d", dumper->trackID);

	sprintf(szName, "%s.media", dumper->out_name);
	out_med = fopen(szName, "wb");
	if (!out_med) {
		OD_DeleteDescriptor((Descriptor **) &esd);
		return dump_message(dumper, M4IOErr, "Error opening %s for writing - check disk access & permissions", szName);
	}

	sprintf(szName, "%s.nhnt", dumper->out_name);
	out_nhnt = fopen(szName, "wb");
	if (!out_nhnt) {
		fclose(out_med);
		OD_DeleteDescriptor((Descriptor **) &esd);
		return dump_message(dumper, M4IOErr, "Error opening %s for writing - check disk access & permissions", szName);
	}


	bs = NewBitStreamFromFile(out_nhnt, BS_FILE_WRITE_ONLY);

	if (esd->decoderConfig->decoderSpecificInfo  && esd->decoderConfig->decoderSpecificInfo->data) {
		sprintf(szName, "%s.info", dumper->out_name);
		out_inf = fopen(szName, "wb");
		if (out_inf) fwrite(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, 1, out_inf);
		fclose(out_inf);
	}

	/*write header*/
	/*'NHnt' format*/
	BS_WriteInt(bs, 'N', 8);
	BS_WriteInt(bs, 'H', 8);
	BS_WriteInt(bs, 'n', 8);
	BS_WriteInt(bs, 't', 8);
	/*version 1*/
	BS_WriteInt(bs, 0, 8);
	/*streamType*/
	BS_WriteInt(bs, esd->decoderConfig->streamType, 8);
	/*OTI*/
	BS_WriteInt(bs, esd->decoderConfig->objectTypeIndication, 8);
	/*reserved*/
	BS_WriteInt(bs, 0, 16);
	/*bufferDB*/
	BS_WriteInt(bs, esd->decoderConfig->bufferSizeDB, 24);
	/*avg BitRate*/
	BS_WriteInt(bs, esd->decoderConfig->avgBitrate, 32);
	/*max bitrate*/
	BS_WriteInt(bs, esd->decoderConfig->maxBitrate , 32);
	/*timescale*/
	BS_WriteInt(bs, esd->slConfig->timestampResolution, 32);

	OD_DeleteDescriptor((Descriptor **) &esd);

	has_b_frames = M4_TrackHasTimeOffsets(dumper->file, track);

	pos = 0;
	count = M4_GetSampleCount(dumper->file, track);
	for (i=0; i<count; i++) {
		M4Sample *samp = M4_GetSample(dumper->file, track, i+1, &di);
		if (!samp) break;
		fwrite(samp->data, samp->dataLength, 1, out_med);
		
		/*dump nhnt info*/
		BS_WriteInt(bs, samp->dataLength, 24);
		BS_WriteInt(bs, samp->IsRAP, 1);
		/*AU start & end flag always true*/
		BS_WriteInt(bs, 1, 1);
		BS_WriteInt(bs, 1, 1);
		/*reserved*/
		BS_WriteInt(bs, 0, 3);
		/*type - try to guess it*/
		if (has_b_frames) {
			if (samp->IsRAP) BS_WriteInt(bs, 0, 2);
			/*if CTS offset, assime P*/
			else if (samp->CTS_Offset) BS_WriteInt(bs, 1, 2);
			else BS_WriteInt(bs, 2, 2);
		} else {
			BS_WriteInt(bs, samp->IsRAP ? 0 : 1, 2);
		}
		BS_WriteInt(bs, pos, 32);
		BS_WriteInt(bs, samp->DTS + samp->CTS_Offset, 32);
		BS_WriteInt(bs, samp->DTS, 32);

		pos += samp->dataLength;
		M4_DeleteSample(&samp);
		dump_progress(dumper, i+1, count);
		if (dumper->do_abort) break;
	}
	fclose(out_med);
	DeleteBitStream(bs);
	fclose(out_nhnt);
	return dump_message(dumper, M4OK, "Extracting done");
}


M4Err MP4T_DumpTrackMP4(M4TrackDumper *dumper)
{
	M4File *outfile;
	M4Err e;
	ESDescriptor *esd;
	char szName[1000];
	u32 track;
	u8 mode;

	track = M4_GetTrackByID(dumper->file, dumper->trackID);

	esd = M4_GetStreamDescriptor(dumper->file, track, 1);
	if (esd && (esd->decoderConfig->streamType == M4ST_OD)) {
		if (esd) OD_DeleteDescriptor((Descriptor **) &esd);
		return dump_message(dumper, M4BadParam, "Cannot extract OD track, result is  meaningless");
	}
	if (esd) OD_DeleteDescriptor((Descriptor **) &esd);

	sprintf(szName, "%s.mp4", dumper->out_name);

	mode = M4_WRITE_EDIT;
	if (dumper->merge_tracks) {
		FILE *t = fopen(szName, "rb");
		if (t) {
			mode = M4_OPEN_EDIT;
			fclose(t);
		}
	}
	outfile = M4_MovieOpen(szName, mode);

	if (mode == M4_WRITE_EDIT) {
		M4_SetMoviePLIndication(outfile, M4_PL_AUDIO, 0xFF);
		M4_SetMoviePLIndication(outfile, M4_PL_VISUAL, 0xFF);
		M4_SetMoviePLIndication(outfile, M4_PL_GRAPHICS, 0xFF);
		M4_SetMoviePLIndication(outfile, M4_PL_SCENE, 0xFF);
		M4_SetMoviePLIndication(outfile, M4_PL_OD, 0xFF);
		M4_SetMoviePLIndication(outfile, M4_PL_MPEGJ, 0xFF);
	}

	e = MP4T_CopyTrack(dumper->file, track, outfile, 1, 1);
	M4_MovieClose(outfile);

	return dump_message(dumper, e, "Extracting done");
}

M4Err MP4T_DumpTrackAVI(M4TrackDumper *dumper)
{
	ESDescriptor *esd;
	M4Sample *samp;
	char szName[1000];
	avi_t *avi_out;
	char dumdata[1];
	u32 track, i, di, count, w, h, frame_d;
	M4VDecoderSpecificInfo dsi;
	Double FPS;

	track = M4_GetTrackByID(dumper->file, dumper->trackID);
	esd = M4_GetStreamDescriptor(dumper->file, track, 1);
	if (!esd) return dump_message(dumper, M4NonCompliantBitStream, "Invalid MPEG-4 stream in track ID %d", dumper->trackID);

	if ((esd->decoderConfig->streamType!=M4ST_VISUAL) || (esd->decoderConfig->objectTypeIndication!=0x20)) {
		OD_DeleteDescriptor((Descriptor**)&esd);
		dump_message(dumper, M4NonCompliantBitStream, "Track ID %d is not MPEG-4 Visual - cannot extract to AVI", dumper->trackID);
	}
	if (!esd->decoderConfig->decoderSpecificInfo) {
		OD_DeleteDescriptor((Descriptor**)&esd);
		dump_message(dumper, M4NonCompliantBitStream, "Missing decoder config for track ID %d", dumper->trackID);
	}

	sprintf(szName, "%s.avi", dumper->out_name);
	avi_out = AVI_open_output_file(szName);
	if (!avi_out) {
		OD_DeleteDescriptor((Descriptor **)&esd);
		return dump_message(dumper, M4IOErr, "Error opening %s for writing - check disk access & permissions", szName);
	}
	/*ignore visual size info, get it from dsi*/
	M4V_GetConfig(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, &dsi);
	w = dsi.width;
	h = dsi.height;

	/*compute FPS - note we assume constant frame rate without droped frames...*/
	count = M4_GetSampleCount(dumper->file, track);
	FPS = M4_GetMediaTimeScale(dumper->file, track);
	FPS *= (count-1);
	samp = M4_GetSample(dumper->file, track, count, &di);
	FPS /= samp->DTS;
	M4_DeleteSample(&samp);

	frame_d = 0;
	if (M4_TrackHasTimeOffsets(dumper->file, track)) {
		u32 DTS, max_CTSO;
		u64 dataoff;
		DTS = max_CTSO = 0;
		for (i=0; i<count; i++) {
			samp = M4_GetSampleInfo(dumper->file, track, i+1, &di, &dataoff);
			if (!samp) break;
			if (samp->CTS_Offset>max_CTSO) max_CTSO = samp->CTS_Offset;
			DTS = samp->DTS;
			M4_DeleteSample(&samp);
		}
		DTS /= (count-1);
		frame_d = max_CTSO / DTS;
		frame_d -= 1;
		/*dummy delay frame for xvid unpacked bitstreams*/
		dumdata[0] = 127;
	}

	AVI_set_video(avi_out, w, h, FPS, "XVID");
	dump_message(dumper, M4OK, "Creating AVI file %d x %d @ %.2f FPS - 4CC \"XVID\"", w, h, FPS);
	if (frame_d) dump_message(dumper, M4OK, "B-Frames detected - using unpacked bitstream with max B-VOP delta %d", frame_d);

	for (i=0; i<count; i++) {
		samp = M4_GetSample(dumper->file, track, i+1, &di);
		if (!samp) break;

		if (!i) {
			char *data = malloc(sizeof(char) * (samp->dataLength + esd->decoderConfig->decoderSpecificInfo->dataLength));
			memcpy(data, esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength);
			memcpy(data + esd->decoderConfig->decoderSpecificInfo->dataLength, samp->data, samp->dataLength);
			AVI_write_frame(avi_out, data, samp->dataLength + esd->decoderConfig->decoderSpecificInfo->dataLength, 1);
			free(data);
		} else {
			AVI_write_frame(avi_out, samp->data, samp->dataLength, samp->IsRAP);
		}
		M4_DeleteSample(&samp);
		while (frame_d) {
			AVI_write_frame(avi_out, dumdata, 1, 0);
			frame_d--;
		}
		dump_progress(dumper, i+1, count);
		if (dumper->do_abort) break;
	}

	OD_DeleteDescriptor((Descriptor **) &esd);
	AVI_close(avi_out);
	return dump_message(dumper, M4OK, "Extracting done");
}


M4Err MP4T_DumpTrack(M4TrackDumper *dumper)
{
	if (!dumper) return M4BadParam;
	switch (dumper->dump_type) {
	case DUMP_TRACK_NATIVE:
		return MP4T_DumpTrackNative(dumper);
	case DUMP_TRACK_NHNT:
		return MP4T_DumpTrackNHNT(dumper);
	case DUMP_TRACK_AVI:
		return MP4T_DumpTrackAVI(dumper);
	case DUMP_TRACK_MP4:
		return MP4T_DumpTrackMP4(dumper);
	default:
		return M4BadParam;
	}
}

