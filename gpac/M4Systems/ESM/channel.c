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


#include <gpac/intern/m4_esm_dev.h>
#include "MediaMemory.h"

/*reset channel*/
static void Channel_Reset(Channel *ch)
{
	Channel_Lock(ch, 1);

	ch->IsClockInit = 0;
	ch->au_sn = 0;
	ch->pck_sn = 0;
	ch->NeedRap = 1;
	ch->IsRap = 0;
	ch->IsEndOfStream = 0;
	ch->ts_offset = 0;
	ch->seed_ts = 0;

	/*just in case*/
	if (ch->BufferOn) {
		CK_BufferOff(ch->clock);
	}
	ch->BufferOn = 0;

	if (ch->buffer) free(ch->buffer);
	ch->buffer = NULL;
	ch->len = ch->allocSize = 0;

	DB_Delete(ch->AU_buffer_first);
	ch->AU_buffer_first = ch->AU_buffer_last = NULL;
	ch->AU_Count = 0;
	ch->BufferTime = 0;
	ch->NextIsAUStart = 1;

	ch->first_au_fetched = 0;
	
	if (ch->AU_buffer_pull) {
		ch->AU_buffer_pull->data = NULL;
		DB_Delete(ch->AU_buffer_pull);
		ch->AU_buffer_pull = NULL;
	}
	Channel_Lock(ch, 0);
}

Channel *NewChannel(ESDescriptor *esd)
{
	u32 nbBits;
	Channel *tmp = malloc(sizeof(Channel));
	if (!tmp) return NULL;
	memset((void *)tmp, 0, sizeof(Channel));

	tmp->mx = NewMutex();

	tmp->esd = esd;
	tmp->chan_id = (u32) tmp;
	tmp->es_state = ES_Setup;

	nbBits = sizeof(u32) * 8 - esd->slConfig->AUSeqNumLength;
	tmp->max_au_sn = 0xFFFFFFFF >> nbBits;
	nbBits = sizeof(u32) * 8 - esd->slConfig->packetSeqNumLength;
	tmp->max_pck_sn = 0xFFFFFFFF >> nbBits;

	tmp->skip_sl = (esd->slConfig->predefined == SLPredef_SkipSL) ? 1 : 0;

	/*take care of dummy streams*/
	if (!esd->slConfig->timestampResolution) esd->slConfig->timestampResolution = 1000;
	if (!esd->slConfig->OCRResolution) esd->slConfig->OCRResolution = esd->slConfig->timestampResolution;

	tmp->ts_scale = 1000;
	tmp->ts_scale /= esd->slConfig->timestampResolution;

	tmp->ocr_scale = 0;
	if (esd->slConfig->OCRResolution) {
		tmp->ocr_scale = 1000;
		tmp->ocr_scale /= esd->slConfig->OCRResolution;
	}


	Channel_Reset(tmp);
	return tmp;
}


/*reconfig SL settings for this channel - this is needed by some net services*/
void Channel_ReconfigSL(Channel *ch, SLConfigDescriptor *slc)
{
	u32 nbBits;
	
	memcpy(ch->esd->slConfig, slc, sizeof(SLConfigDescriptor));

	nbBits = sizeof(u32) * 8 - ch->esd->slConfig->AUSeqNumLength;
	ch->max_au_sn = 0xFFFFFFFF >> nbBits;
	nbBits = sizeof(u32) * 8 - ch->esd->slConfig->packetSeqNumLength;
	ch->max_pck_sn = 0xFFFFFFFF >> nbBits;

	ch->skip_sl = (slc->predefined == SLPredef_SkipSL) ? 1 : 0;

	/*take care of dummy streams*/
	if (!ch->esd->slConfig->timestampResolution) ch->esd->slConfig->timestampResolution = 1000;
	if (!ch->esd->slConfig->OCRResolution) ch->esd->slConfig->OCRResolution = ch->esd->slConfig->timestampResolution;
	ch->ts_scale = 1000;
	ch->ts_scale /= slc->timestampResolution;
	ch->ocr_scale = 0;
	if (ch->esd->slConfig->OCRResolution) {
		ch->ocr_scale = 1000;
		ch->ocr_scale /= ch->esd->slConfig->OCRResolution;
	}
}


/*destroy channel*/
void DeleteChannel(Channel *ch)
{
	Channel_Reset(ch);
	if (ch->AU_buffer_pull) {
		ch->AU_buffer_pull->data = NULL;
		DB_Delete(ch->AU_buffer_pull);
	}
	if (ch->mx) MX_Delete(ch->mx);
	free(ch);
}

Bool Channel_OwnsClock(Channel *ch)
{
	/*if the clock is not in the same namespace (used with dynamic scenes), it's not ours*/
	if (ChainFindEntry(ch->odm->net_service->Clocks, ch->clock)<0) return 0;
	return (ch->esd->ESID==ch->clock->clockID) ? 1 : 0;
}

M4Err Channel_Start(Channel *ch)
{
	if (!ch) return M4BadParam;

	switch (ch->es_state) {
	case ES_Unavailable:
	case ES_Setup:
		return M4BadParam;
	default:
		break;
	}

	/*reset clock if we own it*/
	if (Channel_OwnsClock(ch)) CK_Reset(ch->clock);

	/*reset channel*/
	Channel_Reset(ch);
	/*create pull buffer if needed*/
	if (ch->is_pulling && !ch->AU_buffer_pull) ch->AU_buffer_pull = DB_New();

	/*and start buffering - pull channels always turn off buffering immediately, otherwise 
	buffering size is setup by the network service - except InputSensor*/
	if ((ch->esd->decoderConfig->streamType != M4ST_INTERACT) || ch->esd->URLString) {
		ch->BufferOn = 1;
		CK_BufferOn(ch->clock);
	}
	ch->last_au_time = Term_GetTime(ch->odm->term);
	ch->es_state = ES_Running;
	return M4OK;
}

M4Err Channel_Stop(Channel *ch)
{
	if (!ch) return M4BadParam;

	switch (ch->es_state) {
	case ES_Unavailable:
	case ES_Setup:
		return M4BadParam;
	default:
		break;
	}

	if (ch->BufferOn) {
		CK_BufferOff(ch->clock);
		ch->BufferOn = 0;
	}
	ch->es_state = ES_Connected;
	if (ch->buffer) free(ch->buffer);
	ch->buffer = NULL;
	ch->len = 0;
	return M4OK;
}


void Channel_WaitRAP(Channel *ch)
{
	ch->pck_sn = 0;

	/*if using RAP signal and codec not resilient, wait for rap. If RAP isn't signaled DON'T wait for it :)*/
	if (ch->esd->slConfig->useRandomAccessPointFlag && !ch->codec_resilient) ch->NeedRap = 1;
	if (ch->buffer) free(ch->buffer);
	ch->buffer = NULL;
	ch->AULength = 0;
	ch->au_sn = 0;
}

void Channel_TimeMapped(Channel *ch, Bool reset)
{
	MX_P(ch->mx);
	if (ch->buffer) free(ch->buffer);
	ch->buffer = NULL;
	ch->len = ch->allocSize = 0;

	if (reset) {
		DB_Delete(ch->AU_buffer_first);
		ch->AU_buffer_first = ch->AU_buffer_last = NULL;
		ch->AU_Count = 0;
	} else {
		LPAUBUFFER au = ch->AU_buffer_first;
		while (au) {
			au->DTS = au->CTS = ch->ts_offset;
			au = au->next;
		}
	}
	ch->BufferTime = 0;
	MX_V(ch->mx);
}

/*data timeout: if no data is received in this amount of time and channel is buffering, buffering aborts */
#define CH_DATA_TIMEOUT		2000

static Bool Channel_NeedsBuffering(Channel *ch, u32 ForRebuffering)
{
	if (!ch->MaxBuffer || ch->IsEndOfStream) return 0;

	/*for rebuffering, check we're not below min buffer*/
	if (ForRebuffering) {
		if (ch->MinBuffer && (ch->BufferTime <= (s32) ch->MinBuffer)) {
			return 1;
		}
		return 0;
	}
	/*nothing received, buffer needed*/
	if (!ch->first_au_fetched && !ch->AU_buffer_first) {
		/*data timeout (no data sent)*/
		if (Term_GetTime(ch->odm->term) > ch->last_au_time + CH_DATA_TIMEOUT) {
			ch->MinBuffer = ch->MaxBuffer = 0;
			IS_UpdateBufferingInfo(ch->odm->parentscene ? ch->odm->parentscene : ch->odm->subscene);
			return 0;
		}
		return 1;
	}

	/*buffer not filled yet*/
	if (ch->BufferTime < (s32) ch->MaxBuffer) {
		/*check last AU time*/
		u32 now = Term_GetTime(ch->odm->term);
		/*if more than N sec since last AU don't buffer and prevent rebuffering on short streams
		this will also work for channels ignoring timing*/
		if (now>ch->last_au_time + MAX(ch->BufferTime, CH_DATA_TIMEOUT) ) {
			/*this can be safely seen as a stream with very few updates (likely only one)*/
			if (!ch->AU_buffer_first && ch->first_au_fetched) ch->MinBuffer = 0;
			return 0;
		}
		return 1;
	}
	return 0;
}

static void Channel_UpdateBuffering(Channel *ch, Bool update_info)
{
	if (update_info && ch->MaxBuffer) IS_UpdateBufferingInfo(ch->odm->parentscene ? ch->odm->parentscene : ch->odm->subscene);
	if (!Channel_NeedsBuffering(ch, 0)) {
		ch->BufferOn = 0;
		CK_BufferOff(ch->clock);
		if (ch->MaxBuffer) IS_UpdateBufferingInfo(ch->odm->parentscene ? ch->odm->parentscene : ch->odm->subscene);
	}
}

static void Channel_UpdateBufferTime(Channel *ch)
{
	if (!ch->AU_buffer_first) {
		ch->BufferTime = 0;
	}
	else if (ch->skip_sl) {
		LPAUBUFFER au;
		/*compute buffer size from avg bitrate*/
		u32 avg_rate = ch->esd->decoderConfig->avgBitrate;
		if (!avg_rate && ch->odm->codec) avg_rate = ch->odm->codec->avg_bit_rate;
		if (avg_rate) {
			u32 bsize = 0;
			au = ch->AU_buffer_first;
			while (1) {
				bsize += au->dataLength*8;
				if (!au->next) break;
				au = au->next;
			}
			ch->BufferTime = 1000*bsize/avg_rate;
		} else {
			/*we're in the dark, so don't buffer too much (assume 50ms per unit) so that we start decoding asap*/
			ch->BufferTime = 50*ch->AU_Count;
		}
	} else {
		s32 bt = ch->AU_buffer_last->DTS - CK_GetTime(ch->clock);
		ch->BufferTime = 0;
		if (bt>0) ch->BufferTime = (u32) bt;
	}
	ch->BufferTime += ch->au_duration;
}

/*dispatch the AU in the DB*/
static void Channel_DispatchAU(Channel *ch, u32 duration)
{
	LPAUBUFFER au;
	if (!ch->buffer || !ch->len) {
		if (ch->buffer) {
			free(ch->buffer);
			ch->buffer = NULL;
		}
		return;
	}

	au = DB_New();
	if (!au) {
		free(ch->buffer);
		ch->buffer = NULL;
		ch->len = 0;
		return;
	}

	au->CTS = ch->CTS;
	au->DTS = ch->DTS;
	au->RAP = ch->IsRap;
	au->data = ch->buffer;
	au->dataLength = ch->len;
	au->PaddingBits = ch->padingBits;

	ch->padingBits = 0;
	au->next = NULL;
	ch->buffer = NULL;

	if (ch->len + ch->media_padding_bytes != ch->allocSize) {
		au->data = realloc(au->data, sizeof(char) * (au->dataLength + ch->media_padding_bytes));
	}
	if (ch->media_padding_bytes) memset(au->data + au->dataLength, 0, sizeof(char)*ch->media_padding_bytes);
	
	ch->len = ch->allocSize = 0;
	if (ch->NeedRap && au->RAP) ch->NeedRap = 0;

	Channel_Lock(ch, 1);

	if (!ch->AU_buffer_first) {
		ch->AU_buffer_first = au;
		ch->AU_buffer_last = au;
		ch->AU_Count = 1;
	} else {
		if (ch->AU_buffer_last->DTS<=au->DTS) {
			ch->AU_buffer_last->next = au;
			ch->AU_buffer_last = ch->AU_buffer_last->next;
		}
		/*enable deinterleaving only for audio channels (some video transport may not be able to compute DTS, cf MPEG1-2/RTP)
		HOWEVER, we must recompute a monotone increasing DTS in case the decoder does perform frame reordering
		in which case the DTS is used for presentation time!!*/
		else if (ch->odm->codec->type!=M4ST_AUDIO) {
			LPAUBUFFER au_prev, ins_au;
			u32 DTS;

			/*append AU*/
			ch->AU_buffer_last->next = au;
			ch->AU_buffer_last = ch->AU_buffer_last->next;

			DTS = au->DTS;
			au_prev = ch->AU_buffer_first;
			/*locate first AU in buffer with DTS greater than new unit CTS*/
			while (au_prev->next && (au_prev->DTS < DTS) ) au_prev = au_prev->next;
			/*remember insertion point*/
			ins_au = au_prev;
			/*shift all following frames DTS*/
			while (au_prev->next) {
				au_prev->next->DTS = au_prev->DTS;
				au_prev = au_prev->next;
			}
			/*and apply*/
			ins_au->DTS = DTS;

#if 0
			au_prev = ch->AU_buffer_first;
			fprintf(stdout, "\nDTS ");
			while (au_prev->next) {
				assert(au_prev->DTS < au_prev->next->DTS);
				fprintf(stdout, "%d ", au_prev->DTS);
				au_prev = au_prev->next;
			}
			fprintf(stdout, "%d\n", au_prev->DTS);
#endif
		} else {
			fprintf(stdout, "Audio deinterleaving OD %d ch %d\n", ch->esd->ESID, ch->odm->OD->objectDescriptorID);
			/*de-interleaving of AUs*/
			if (ch->AU_buffer_first->DTS > au->DTS) {
				au->next = ch->AU_buffer_first;
				ch->AU_buffer_first = au;
			} else {
				LPAUBUFFER au_prev = ch->AU_buffer_first;
				while (au_prev->next && au_prev->next->DTS<au->DTS) {
					au_prev = au_prev->next;
				}
				assert(au_prev);
				if (au_prev->next->DTS==au->DTS) {
					free(au->data);
					free(au);
				} else {
					au->next = au_prev->next;
					au_prev->next = au;
				}
			}
		}
		ch->AU_Count += 1;
	}

	Channel_UpdateBufferTime(ch);
	ch->au_duration = 0;
	if (duration) ch->au_duration = (u32) (ch->ts_scale * duration);

//	fprintf(stdout, "CH %d - Dispatch AU CTS %d time %d Buffer %d Nb AUs %d\n", ch->esd->ESID, au->CTS, CK_GetTime(ch->clock), ch->BufferTime, ch->AU_Count);

	if (ch->BufferOn) {
		ch->last_au_time = Term_GetTime(ch->odm->term);
		Channel_UpdateBuffering(ch, 1);
	}

	Channel_Lock(ch, 0);
	return;
}

void Channel_RecieveSkipSL(LPNETSERVICE serv, Channel *ch, char *StreamBuf, u32 StreamLength)
{
	LPAUBUFFER au;
	if (!StreamLength) return;

	Channel_Lock(ch, 1);
	au = DB_New();
	au->RAP = 1;
	au->DTS = CK_GetTime(ch->clock);
	au->data = malloc(sizeof(char) * (ch->media_padding_bytes + StreamLength));
	memcpy(au->data, StreamBuf, sizeof(char) * StreamLength);
	if (ch->media_padding_bytes) memset(au->data + StreamLength, 0, sizeof(char)*ch->media_padding_bytes);
	au->dataLength = StreamLength;
	au->next = NULL;

	/*if channel owns the clock, start it*/
	if (ch->clock && !ch->IsClockInit) {
		if (Channel_OwnsClock(ch)) {
			CK_SetTime(ch->clock, 0);
			ch->IsClockInit = 1;
			ch->seed_ts = 0;
		}
		if (ch->clock->clock_init && !ch->IsClockInit) {
			ch->IsClockInit = 1;
			ch->seed_ts = CK_GetTime(ch->clock);
		}
	}

	if (!ch->AU_buffer_first) {
		ch->AU_buffer_first = au;
		ch->AU_buffer_last = au;
		ch->AU_Count = 1;
	} else {
		ch->AU_buffer_last->next = au;
		ch->AU_buffer_last = ch->AU_buffer_last->next;
		ch->AU_Count += 1;
	}

	Channel_UpdateBufferTime(ch);

	if (ch->BufferOn) {
		ch->last_au_time = Term_GetTime(ch->odm->term);
		Channel_UpdateBuffering(ch, 1);
	}
	Channel_Lock(ch, 0);
}

/*handles reception of an SL-PDU, logical or physical*/
void Channel_RecieveSLP(LPNETSERVICE serv, Channel *ch, char *StreamBuf, u32 StreamLength, SLHeader *header, M4Err reception_status)
{
	SLHeader hdr;
	u64 CTS, DTS;
	u32 nbAU, OldLength, size, AUSeqNum, SLHdrLen;
	Bool EndAU, NewAU;
	char *payload;

	if (ch->skip_sl) {
		Channel_RecieveSkipSL(serv, ch, StreamBuf, StreamLength);
		return;
	}

	/*physical SL-PDU - depacketize*/
	if (!header) {
		if (!StreamLength) return;
		SL_Depacketize(ch->esd->slConfig, &hdr, StreamBuf, StreamLength, &SLHdrLen);
		/*FIXME: this assumes the start of the payload is byte-aligned, nothing forces that in systems*/
		StreamLength -= SLHdrLen;
	} else {
		hdr = *header;
		SLHdrLen = 0;
	}
	payload = StreamBuf + SLHdrLen;

	/*check state*/
	if (!ch->codec_resilient && (reception_status==M4CorruptedData)) {
		Channel_WaitRAP(ch);
		return;
	}

	if (!ch->esd->slConfig->useAccessUnitStartFlag) {
		/*no AU signaling - each packet is an AU*/
		if (!ch->esd->slConfig->useAccessUnitEndFlag) 
			hdr.accessUnitEndFlag = hdr.accessUnitStartFlag = 1;
		/*otherwise AU are signaled by end of previous packet*/
		else
			hdr.accessUnitStartFlag = ch->NextIsAUStart;
	}

	/*get RAP*/
	if (ch->esd->slConfig->useRandomAccessUnitsOnlyFlag) {
		hdr.randomAccessPointFlag = hdr.accessUnitStartFlag;
	} else if (!ch->esd->slConfig->useRandomAccessPointFlag) {
		ch->NeedRap = 0;
	}

	if (ch->esd->slConfig->packetSeqNumLength) {
		if (ch->pck_sn && hdr.packetSequenceNumber) {
			/*repeated -> drop*/
			if (ch->pck_sn == hdr.packetSequenceNumber) return;
			/*if codec has no resiliency check packet drops*/
			if (!ch->codec_resilient && !hdr.accessUnitStartFlag) {
				if (ch->pck_sn == (u32) (1<<ch->esd->slConfig->packetSeqNumLength) ) {
					if (hdr.packetSequenceNumber) {
						Channel_WaitRAP(ch);
						return;
					}
				} else if (ch->pck_sn + 1 != hdr.packetSequenceNumber) {
					Channel_WaitRAP(ch);
					return;
				}
			}
		}
		ch->pck_sn = hdr.packetSequenceNumber;
	}

	/*if idle or empty, skip the packet*/
	if (hdr.idleFlag || (hdr.paddingFlag && !hdr.paddingBits)) return;


	NewAU = 0;
	if (hdr.accessUnitStartFlag) {
		NewAU = 1;
		ch->NextIsAUStart = 0;

		/*if we have a pending AU, add it*/
		if (ch->buffer) {
			fprintf(stdout, "MISSED END OF AU\n");
			if (ch->codec_resilient) {
				Channel_DispatchAU(ch, 0);
			} else {
				free(ch->buffer);
				ch->buffer = NULL;
				ch->AULength = 0;
				ch->len = ch->allocSize = 0;
			}
		}
		/*update the RAP		*/
		ch->IsRap = hdr.randomAccessPointFlag;

		AUSeqNum = hdr.AU_sequenceNumber;

		/*Get CTS */
		if (hdr.compositionTimeStampFlag) {
			DTS = CTS = hdr.compositionTimeStamp;
			/*get DTS */
			if (hdr.decodingTimeStampFlag) DTS = hdr.decodingTimeStamp;

			/*until clock is not init check seed ts*/
			if (!ch->IsClockInit && (DTS < ch->seed_ts)) ch->seed_ts = DTS;

			/*TS Wraping not tested*/
			ch->CTS = (u32) (ch->ts_offset + (s64) (CTS - ch->seed_ts) * ch->ts_scale);
			ch->DTS = (u32) (ch->ts_offset + (s64) (DTS - ch->seed_ts) * ch->ts_scale);
		} else {
			/*use CU duration*/
			if (!ch->IsClockInit) ch->DTS = ch->CTS = ch->ts_offset;

			if (!ch->esd->slConfig->AUSeqNumLength) {
				if (!ch->au_sn) {
					ch->CTS = ch->ts_offset;
					ch->au_sn = 1;
				} else {
					ch->CTS += ch->esd->slConfig->CUDuration;
				}
			} else {
				//use the sequence number to get the TS
				if (AUSeqNum < ch->au_sn) {
					nbAU = ( (1<<ch->esd->slConfig->AUSeqNumLength) - ch->au_sn) + AUSeqNum;
				} else {
					nbAU = AUSeqNum - ch->au_sn;
				}
				ch->CTS += nbAU * ch->esd->slConfig->CUDuration;
			}
		}

		/*if channel owns the clock, start it*/
		if (!ch->IsClockInit) {
			if (Channel_OwnsClock(ch)) {
				CK_SetTime(ch->clock, ch->DTS);
				ch->IsClockInit = 1;
			}
			if (ch->clock->clock_init) ch->IsClockInit = 1;
		}
		/*if the AU Length is carried in SL, get its size*/
		if (ch->esd->slConfig->AULength > 0) {
			ch->AULength = hdr.accessUnitLength;
		} else {
			ch->AULength = 0;
		}

		/*skip the AU if we're waiting for a RAP.*/
		if (ch->NeedRap && !hdr.randomAccessPointFlag) {
			return;
		}

		/*TO DO - caroussel for repeated AUs*/
		if (ch->esd->slConfig->AUSeqNumLength) {
			if (AUSeqNum == ch->au_sn) return;
			ch->au_sn = AUSeqNum;
		}
	}

	/*check OCR*/
	if (hdr.OCRflag) {
		s64 OCR_TS = (s64) (((s64) hdr.objectClockReference) * ch->ocr_scale);
		CK_SetTime(ch->clock, (u32) OCR_TS);
		ch->IsClockInit = 1;
	}

	/*get AU end state*/	
	OldLength = ch->buffer ? ch->len : 0;
	EndAU = hdr.accessUnitEndFlag;
	if (ch->AULength == OldLength + StreamLength) EndAU = 1;
	if (EndAU) ch->NextIsAUStart = 1;

	if (!StreamLength && EndAU && ch->buffer) {
		Channel_DispatchAU(ch, 0);
		return;
	}
	if (!StreamLength) return;

	/*missed begining, unusable*/
	if (!ch->buffer && !NewAU) {
		fprintf(stdout, "MISSED BEGIN OF AU\n");
		return;
	}

	/*Write the Packet payload to the buffer*/
	if (NewAU) {
		/*we should NEVER have a bitstream at this stage*/
		assert(!ch->buffer);
		/*ignore length fields*/
		size = StreamLength + ch->media_padding_bytes;
		ch->buffer = malloc(sizeof(char) * size);
		if (!ch->buffer) return;

		ch->allocSize = size;
		memset(ch->buffer, 0, sizeof(char) * size);
		ch->len = 0;
	}
	if (!ch->esd->slConfig->usePaddingFlag) hdr.paddingFlag = 0;

	/*if no bitstream, we missed the AU Start packet. Unusable ...*/
	if (!ch->buffer) return;
	
	if (hdr.paddingFlag && !EndAU) {	
		/*to do - this shouldn't happen anyway */

	} else {
		/*check if enough space*/
		size = ch->allocSize;
		if (size && (StreamLength + ch->len <= size)) {
			memcpy(ch->buffer+ch->len, payload, StreamLength);
			ch->len += StreamLength;
		} else {
			size = StreamLength + ch->len + ch->media_padding_bytes;
			ch->buffer = realloc(ch->buffer, sizeof(char) * size);
			memcpy(ch->buffer+ch->len, payload, StreamLength);
			ch->allocSize = size;
			ch->len += StreamLength;
		}
		if (hdr.paddingFlag) ch->padingBits = hdr.paddingBits;
	}

	if (EndAU) Channel_DispatchAU(ch, hdr.au_duration);
}


void Channel_DistpatchRawData(Channel *ch, char *data, u32 dataLen)
{
	LPCUBUFFER cu;
	assert(ch->odm->codec);

	cu = CB_LockInput(ch->odm->codec->CB, ch->CTS);
	if (!cu) {
		cu = CB_GetOutput(ch->odm->codec->CB);
		if (ch->CTS > 1500 + cu->TS) {
			CB_Reset(ch->odm->codec->CB);
		}
		return;
	}

	if (ch->odm->codec->CB->UnitSize < dataLen) return;

	cu->TS = ch->CTS;
	memcpy(cu->data, data, dataLen);

	CB_UnlockInput(ch->odm->codec->CB, ch->CTS, dataLen);
	if (ch->NeedRap) ch->NeedRap = 0;
}

/*notification of End of stream on this channel*/
void Channel_EndOfStream(Channel *ch)
{
	if (!ch || ch->IsEndOfStream) return;
	ch->IsEndOfStream = 1;
	
	/*flush buffer*/
	if (ch->BufferOn) {
		ch->BufferOn = 0;
		CK_BufferOff(ch->clock);
	}
	ch->clock->has_seen_eos = 1;
	ODM_EndOfStream(ch->odm, ch);
}



LPAUBUFFER Channel_GetAU(Channel *ch)
{
	Bool comp, is_new_data;
	M4Err e, state;
	SLHeader slh;

	if (ch->es_state != ES_Running) return NULL;

	if (!ch->is_pulling) {
		/*we must update buffering before fetching in order to stop buffering for streams with very few
		updates (especially streams with one update, like most of OD streams)*/
		if (ch->BufferOn) Channel_UpdateBuffering(ch, 0);
		if (ch->first_au_fetched && ch->BufferOn) return NULL;
		return ch->AU_buffer_first;
	}

	/*pull from stream - resume clock if needed*/
	if (ch->BufferOn) {
		ch->BufferOn = 0;
		CK_BufferOff(ch->clock);
	}

	e = NM_ChannelGetSLP(ch->service, ch, (char **) &ch->AU_buffer_pull->data, &ch->AU_buffer_pull->dataLength, &slh, &comp, &state, &is_new_data);
	if (e) state = e;
	switch (state) {
	case M4EOF:
		Channel_EndOfStream(ch);
		return NULL;
	case M4OK:
		break;
	default:
		M4_OnMessage(ch->odm->term, ch->service->url , "Data reception failure", state);
		return NULL;
	}
	assert(!comp);
	/*update timing if new stream data but send no data*/
	if (is_new_data) Channel_RecieveSLP(ch->service, ch, NULL, 0, &slh, M4OK);

	/*this may happen in file streaming when data has not arrived yet, in which case we discard the AU*/
	if (!ch->AU_buffer_pull->data) {
		NM_ChannelReleaseSLP(ch->service, ch);
		return NULL;
	}
	ch->AU_buffer_pull->CTS = (u32) ch->CTS;
	ch->AU_buffer_pull->DTS = (u32) ch->DTS;
	ch->AU_buffer_pull->PaddingBits = ch->padingBits;
	ch->AU_buffer_pull->RAP = ch->IsRap;
	return ch->AU_buffer_pull;
}

void Channel_InitDummy(Channel *ch)
{
	SLHeader slh;
	Bool comp, is_new_data;
	M4Err e, state;
	if (!ch->is_pulling) return;
	/*pull from stream - resume clock if needed*/
	if (ch->BufferOn) {
		ch->BufferOn = 0;
		CK_BufferOff(ch->clock);
	}
	e = NM_ChannelGetSLP(ch->service, ch, (char **) &ch->AU_buffer_pull->data, &ch->AU_buffer_pull->dataLength, &slh, &comp, &state, &is_new_data);
	if (e) state = e;
	if ((state==M4OK) && is_new_data) Channel_RecieveSLP(ch->service, ch, NULL, 0, &slh, M4OK);
	NM_ChannelReleaseSLP(ch->service, ch);
}

void Channel_DropAU(Channel *ch)
{
	LPAUBUFFER au;

	if (ch->is_pulling) {
		NM_ChannelReleaseSLP(ch->service, ch);
		ch->AU_buffer_pull->data = NULL;
		ch->AU_buffer_pull->dataLength = 0;
		ch->first_au_fetched = 1;
		return;
	}

	if (!ch->AU_buffer_first) return;

	/*lock the channel before touching the queue*/
	Channel_Lock(ch, 1);
	ch->first_au_fetched = 1;

	au = ch->AU_buffer_first;
	ch->AU_buffer_first = au->next;
	au->next = NULL;
	DB_Delete(au);
	ch->AU_Count -= 1;

	if (!ch->AU_Count && ch->AU_buffer_first) {
		ch->AU_buffer_first = NULL;
	}
	if (!ch->AU_buffer_first) ch->AU_buffer_last = NULL;

	Channel_UpdateBufferTime(ch);

	/*if we get under our limit, rebuffer EXCEPT WHEN EOS is signaled*/
	if (!ch->IsEndOfStream && !ch->BufferOn && Channel_NeedsBuffering(ch, 1)) {
		ch->BufferOn = 1;
		CK_BufferOn(ch->clock);
	}

	/*unlock the channel*/
	Channel_Lock(ch, 0);
}

/*(un)locks channel*/
void Channel_Lock(Channel *ch, u32 LockIt)
{
	if (LockIt) {
		MX_P(ch->mx);
	} else {
		MX_V(ch->mx);
	}
}

/*performs final setup upon connection confirm*/
void Channel_OnConnect(Channel *ch)
{
	Bool can_buffer;
	NetworkCommand com;
	CapObject cap;

	/*config channel*/
	com.command_type = CHAN_CONFIG;
	com.base.on_channel = ch;

	com.cfg.priority = ch->esd->streamPriority;
	com.cfg.sync_id = (u32) ch->clock;
	memcpy(&com.cfg.sl_config, ch->esd->slConfig, sizeof(SLConfigDescriptor));
	com.cfg.frame_duration = 0;

	/*get the frame duration if audio (used by some network stack)*/
	if (ch->odm->codec && (ch->odm->codec->type==M4ST_AUDIO) ) {
		cap.CapCode = CAP_SAMPLERATE;
		Codec_GetCap(ch->odm->codec, &cap);
		com.cfg.sample_rate = cap.cap.valueINT;
		cap.CapCode = CAP_CU_DURATION;
		Codec_GetCap(ch->odm->codec, &cap);
		com.cfg.frame_duration = cap.cap.valueINT;
	} 
	NM_ServiceCommand(ch->service, &com);

	/*check whether we can work in pull mode or not*/
	can_buffer = 1;
	/*if local interaction streams no buffer nor pull*/
	if ((ch->esd->decoderConfig->streamType == M4ST_INTERACT) && !ch->esd->URLString) can_buffer = 0;

	ch->is_pulling = 0;
	if (can_buffer) {
		/*request padding*/
		com.command_type = CHAN_SET_PADDING;
		com.pad.padding_bytes = ch->media_padding_bytes;
		if (!com.pad.padding_bytes || (NM_ServiceCommand(ch->service, &com) == M4OK)) {
			/*request pull if possible*/
			if (ch->service->ifce->ChannelGetSLP && ch->service->ifce->ChannelReleaseSLP) {
				com.command_type = CHAN_SET_PULL;
				if (NM_ServiceCommand(ch->service, &com) == M4OK) {
					ch->is_pulling = 1;
					can_buffer = 0;
				}
			}
		}
	}

	/*checks whether the stream is interactive or not*/
	com.command_type = CHAN_INTERACTIVE;
	if (NM_ServiceCommand(ch->service, &com)!=M4OK) {
		ch->clock->no_time_ctrl = 1;
		ch->odm->no_time_ctrl = 1;
		ODM_RefreshNonInteractives(ch->odm);
	}

	/*signal channel state*/
	ch->es_state = ES_Connected;
	/*signal only once connected to prevent PLAY trigger on connection callback*/
	ch->odm->pending_channels--;

	/*remember channels connected on service*/
	if (ch->esd->URLString) ch->service->nb_ch_users++;

	/*buffer setup*/
	ch->MinBuffer = ch->MaxBuffer = 0;
	if (can_buffer) {
		char *sOpt;
		com.command_type = CHAN_BUFFER;
		com.base.on_channel = ch;

		/*set default values*/
		com.buffer.max = 1000;
		sOpt = IF_GetKey(ch->odm->term->user->config, "Network", "BufferLength");
		if (sOpt) com.buffer.max = atoi(sOpt);
		com.buffer.min = 0;
		sOpt = IF_GetKey(ch->odm->term->user->config, "Network", "RebufferLength");
		if (sOpt) com.buffer.min = atoi(sOpt);

		if (NM_ServiceCommand(ch->service, &com) == M4OK) {
			ch->MinBuffer = com.buffer.min;
			ch->MaxBuffer = com.buffer.max;
		}
	}

	/*get duration*/
	com.command_type = CHAN_DURATION;
	com.base.on_channel = ch;
	if (NM_ServiceCommand(ch->service, &com) == M4OK)
		ODM_SetDuration(ch->odm, ch, (u32) (1000*com.duration.duration));
}


