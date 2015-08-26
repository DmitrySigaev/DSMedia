/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / RTP input plugin
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

#include "rtp_in.h"
#include <gpac/m4_decoder.h>




M4Err RP_SetupSDP(RTPClient *rtp, SDPInfo *sdp, RTPStream *stream)
{
	M4Err e;
	SDPMedia *media;
	Float Start, End;
	u32 i;
	char *sess_ctrl;
	X_Attribute *att;
	RTSPRange *range;
	RTPStream *ch;

	Start = 0.0;
	End = -1.0;

	sess_ctrl = NULL;
	range = NULL;

	for (i=0; i<ChainGetCount(sdp->Attributes); i++) {
		att = ChainGetEntry(sdp->Attributes, i);
		//session-level control string. Keep it in the current session if any
		if (!strcmp(att->Name, "control") && att->Value) sess_ctrl = att->Value;
		//NPT range only for now
		else if (!strcmp(att->Name, "range") && !range) range = RTSP_ParseRange(att->Value);
	}
	if (range) {
		Start = range->start;
		End = range->end;
		RTSP_DeleteRange(range);
	}
	
	//setup all streams
	for (i=0; i<ChainGetCount(sdp->media_desc); i++) {
		media = ChainGetEntry(sdp->media_desc, i);	
		ch = RP_NewStream(rtp, media, sdp, stream);
		//do not generate error if the channel is not created, just assume
		//1 - this is not an MPEG-4 configured channel -> not needed
		//2 - this is a 2nd describe and the channel was already created
		if (!ch) continue;

		e = RP_AddStream(rtp, ch, sess_ctrl);
		if (e) {
			RP_DeleteStream(ch);
			return e;
		}

		if (!ch->has_range) {
			ch->range_start = Start;
			ch->range_end = End;
			if (End != -1) ch->has_range = 1;
		}
	}
	return M4OK;
}

/*load iod from data:application/mpeg4-iod;base64*/
M4Err RP_SDPLoadIOD(RTPClient *rtp, char *iod_str)
{
	char buf[2000];
	u32 size;

	if (rtp->session_iod) return M4ServiceError;
	/*the only IOD format we support*/
	iod_str += 1;
	if (!strnicmp(iod_str, "data:application/mpeg4-iod;base64", strlen("data:application/mpeg4-iod;base64"))) {
		char *buf64;
		u32 size64;

		buf64 = strstr(iod_str, ",");
		if (!buf64) return M4InvalidURL;
		buf64 += 1;
		size64 = strlen(buf64) - 1;

		size = Base64_dec(buf64, size64, buf, 2000);
		if (!size) return M4ServiceError;
	} else if (!strnicmp(iod_str, "data:application/mpeg4-iod;base16", strlen("data:application/mpeg4-iod;base16"))) {
		char *buf16;
		u32 size16;

		buf16 = strstr(iod_str, ",");
		if (!buf16) return M4InvalidURL;
		buf16 += 1;
		size16 = strlen(buf16) - 1;

		size = Base16_dec(buf16, size16, buf, 2000);
		if (!size) return M4ServiceError;
	} else {
		return M4UnsupportedURL;
	}

	rtp->session_iod_len = size;
	rtp->session_iod = malloc(sizeof(char)*size);
	memcpy(rtp->session_iod, buf, size);
	return M4OK;
}


static u32 get_m4st_from_hint(u32 ht)
{
	switch (ht) {
	case NM_OD_VIDEO: return M4ST_VISUAL;
	case NM_OD_AUDIO: return M4ST_AUDIO;
	case NM_OD_TEXT: return M4ST_TEXT;
	default: return 0;
	}
}

static ObjectDescriptor *RP_GetChannelOD(RTPStream *ch, u16 OCR_ES_ID, u32 ch_idx)
{
	ESDescriptor *esd;
	ObjectDescriptor *od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);

	if (!ch->ES_ID) ch->ES_ID = ch_idx + 1;
	od->objectDescriptorID = ch->ES_ID;
	esd = OD_NewESDescriptor(0);
	esd->slConfig->timestampResolution = ch->clock_rate;
	esd->slConfig->useRandomAccessPointFlag = 1;
	esd->slConfig->useTimestampsFlag = 1;
	esd->ESID = ch->ES_ID;
	esd->OCRESID = OCR_ES_ID;
	esd->decoderConfig->streamType = ch->sl_map.StreamType;
	esd->decoderConfig->objectTypeIndication = ch->sl_map.ObjectTypeIndication;
	if (ch->sl_map.config) {
		esd->decoderConfig->decoderSpecificInfo->data = malloc(sizeof(char) * ch->sl_map.configSize);
		memcpy(esd->decoderConfig->decoderSpecificInfo->data, ch->sl_map.config, sizeof(char) * ch->sl_map.configSize);
		esd->decoderConfig->decoderSpecificInfo->dataLength = ch->sl_map.configSize;
	}
	ChainAddEntry(od->ESDescriptors, esd);
	return od;
}

M4Err RP_EmulateIOD(RTPClient *rtp, u32 expect_type, const char *sub_url)
{
	ObjectDescriptor *the_od;
	ESDescriptor *esd;
	LPODCODEC codec;
	ObjectDescriptorUpdate *odU;
	RTPStream *a_str;
	M4Err e;
	u32 i;

	if (expect_type==NM_OD_INTERACT) return M4NotSupported;
	if (expect_type==NM_OD_BIFS) return M4NotSupported;

	/*single object generation*/
	a_str = NULL;
	if (sub_url || (expect_type!=NM_OD_SCENE)) {
		for (i=0; i<ChainGetCount(rtp->channels); i++) {
			RTPStream *ch = ChainGetEntry(rtp->channels, i);
			if (ch->sl_map.StreamType != get_m4st_from_hint(expect_type)) continue;

			if (!sub_url || strstr(sub_url, ch->control)) {
				the_od = RP_GetChannelOD(ch, 0, i);
				if (!the_od) continue;

				e = OD_EncDesc((Descriptor *) the_od, &rtp->session_iod, &rtp->session_iod_len);
				OD_DeleteDescriptor((Descriptor**)&the_od);
				return e;
			}
			if (!a_str) a_str = ch;
		}
		if (a_str) {
			the_od = RP_GetChannelOD(a_str, 0, ChainFindEntry(rtp->channels, a_str) );
			if (!the_od) return M4URLNotFound;
			e = OD_EncDesc((Descriptor *) the_od, &rtp->session_iod, &rtp->session_iod_len);
			OD_DeleteDescriptor((Descriptor**)&the_od);
			return e;
		}
		return M4URLNotFound;
	}

	e = M4OK;
	the_od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
	the_od->objectDescriptorID = 1;
	
	/*generate OD stream*/
	esd = OD_NewESDescriptor(0);
	esd->slConfig->timestampResolution = 1000;
	esd->slConfig->useRandomAccessPointFlag = 1;
	esd->slConfig->useTimestampsFlag = 1;
	esd->decoderConfig->streamType = M4ST_OD;
	esd->decoderConfig->objectTypeIndication = GPAC_STATIC_OD_OTI;
	rtp->od_es_id = esd->OCRESID = esd->ESID = 0xFFFE;

	codec = OD_NewCodec(OD_WRITE);
	odU = (ObjectDescriptorUpdate *) OD_NewCommand(ODUpdate_Tag);

	/*add everything*/
	for (i=0; i<ChainGetCount(rtp->channels); i++) {
		RTPStream *ch = ChainGetEntry(rtp->channels, i);
		ObjectDescriptor *od = RP_GetChannelOD(ch, esd->OCRESID, i);
		if (!od) continue;
		ChainAddEntry(odU->objectDescriptors, od);
	}
	OD_AddCommand(codec, (ODCommand *) odU);
	OD_EncodeAU(codec);
	if (rtp->od_au) free(rtp->od_au);
	rtp->od_au = NULL;
	OD_GetEncodedAU(codec, &rtp->od_au, &rtp->od_au_size);
	OD_DeleteCodec(codec);

	ChainAddEntry(the_od->ESDescriptors, esd);

	/*finally encode IOD*/
	e = OD_EncDesc((Descriptor *) the_od, &rtp->session_iod, &rtp->session_iod_len);
	OD_DeleteDescriptor((Descriptor**)&the_od);
	return e;
}

void RP_LoadSDP(RTPClient *rtp, char *sdp_text, u32 sdp_len, RTPStream *stream)
{
	M4Err e;
	u32 i;
	SDPInfo *sdp;
	Bool is_isma_1;
	char *iod_str;
	X_Attribute *att;

	is_isma_1 = 0;
	iod_str = NULL;
	sdp = NewSDPInfo();
	e = SDP_Parse(sdp, sdp_text, sdp_len);

	if (e == M4OK) e = RP_SetupSDP(rtp, sdp, stream);

	/*root SDP, attach service*/
	if (! stream) {
		/*look for IOD*/
		if (e==M4OK) {
			for (i=0; i<ChainGetCount(sdp->Attributes); i++) {
				att = ChainGetEntry(sdp->Attributes, i);
				if (!iod_str && !strcmp(att->Name, "mpeg4-iod") ) iod_str = att->Value;
				if (!is_isma_1 && !strcmp(att->Name, "isma-compliance") ) {
					if (!stricmp(att->Value, "1,1.0,1")) is_isma_1 = 1;
				}
			}

			/*force iod reconstruction with ISMA to use proper clock dependencies*/
			if (is_isma_1) iod_str = NULL;

			/*folks at QT have weird notions of MPEG-4 systems, they use hardcoded IOD 
			with AAC ESDescriptor even when streaming AMR...*/
			if (iod_str) {
				for (i=0; i<ChainGetCount(rtp->channels); i++) {
					RTPStream *ch = ChainGetEntry(rtp->channels, i);
					if ((ch->rtptype==RTP_PAYT_AMR) || (ch->rtptype==RTP_PAYT_AMR_WB) ) {
						iod_str = NULL;
						break;
					}
				}
			}
			
			if (!iod_str) {
				e = RP_EmulateIOD(rtp, NM_OD_SCENE, NULL);
			} else {
				e = RP_SDPLoadIOD(rtp, iod_str);
			}
		}
		/*attach service*/
		NM_OnConnect(rtp->service, NULL, e);
	}
	/*channel SDP */
	else {
		if (e) {
			NM_OnConnect(rtp->service, stream->channel, e);
			stream->status = RTP_Unavailable;
		} else {
			/*connect*/
			RP_SetupChannel(stream, NULL);
		}
	}

	if (sdp) SDP_Delete(sdp);
}

