/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / IETF RTP/RTSP/SDP sub-project
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


#include <gpac/intern/m4_ietf_dev.h>

u32 RTCP_ReadChannel(RTPChannel *ch, char *buffer, u32 buffer_size)
{
	M4Err e;
	u32 res;

	//only if the socket exist (otherwise RTSP interleaved channel)
	if (!ch || !ch->rtcp) return 0;

	e = SK_Receive(ch->rtcp, buffer, buffer_size, 0, &res);
	if (e) return 0;
	return res;
}

M4Err RTCP_DecodePacket(RTPChannel *ch, char *pck, u32 pck_size)
{
	RTCPHeader rtcp_hdr;
	BitStream *bs;
	char sdes_buffer[300];
	u32 i, sender_ssrc, cur_ssrc, val, NTP_H, NTP_L, sdes_type, sdes_len, res, first;
	M4Err e = M4OK;
	
	//bad RTCP packet
	if (pck_size < 4 ) return M4NonCompliantBitStream;
	bs = NewBitStream((unsigned char *)pck, pck_size, BS_READ);

	first = 1;
	while (pck_size) {
		//global header
		rtcp_hdr.Version = BS_ReadInt(bs, 2);
		if (rtcp_hdr.Version != 2 ) {
			DeleteBitStream(bs);
			return M4NotSupported;
		}
		rtcp_hdr.Padding = BS_ReadInt(bs, 1);
		rtcp_hdr.Count = BS_ReadInt(bs, 5);
		rtcp_hdr.PayloadType = BS_ReadInt(bs, 8);
		rtcp_hdr.Length = BS_ReadInt(bs, 16);	

		//check pck size
		if (pck_size < (u32) (rtcp_hdr.Length + 1) * 4) {
			DeleteBitStream(bs);
			//we return OK
			return M4CorruptedData;
		}
		//substract this RTCP pck size
		pck_size -= (rtcp_hdr.Length + 1) * 4;
		//all RTCP are Compounds (>1 pck), the first SHALL be SR or RR without padding
		if (first) {
			if ( ( (rtcp_hdr.PayloadType!=200) && (rtcp_hdr.PayloadType!=201) )
				|| rtcp_hdr.Padding
				|| !pck_size
				) {
				DeleteBitStream(bs);
				return M4CorruptedData;
			}
			first = 0;
		}
			
		//specific extensions
		switch (rtcp_hdr.PayloadType) {
		//Sender report - we assume there's only one sender
		case 200:
			//sender ssrc
			sender_ssrc = BS_ReadInt(bs, 32);
			rtcp_hdr.Length -= 1;
			/*not for us...*/
			if (ch->SenderSSRC && (ch->SenderSSRC != sender_ssrc)) break;

			//NTP
			NTP_H = BS_ReadInt(bs, 32);
			NTP_L = BS_ReadInt(bs, 32);
			if (ch->first_SR) {
				ch->first_SR = 0;
				RTCP_GetNextReportTime(ch);
				//this is to make sure we only handle ONE sender
				ch->SenderSSRC = sender_ssrc;
				if (ch->rtp_log) fprintf(ch->rtp_log, "Got Sender SSRC: %d\n", ch->SenderSSRC); 
			}
			if (sender_ssrc == ch->SenderSSRC) {
				ch->last_report_time = RTP_GetReportTime();
				ch->last_SR_NTP_sec = NTP_H;
				ch->last_SR_NTP_frac = NTP_L;
			}
			//extract RTP ts too for multicast
			val = BS_ReadInt(bs, 32);
			if (sender_ssrc == ch->SenderSSRC) ch->last_SR_rtp_time = val;
			//pck count
			val = BS_ReadInt(bs, 32);
			if (sender_ssrc == ch->SenderSSRC) ch->total_pck = val;
			//payload byte count
			val = BS_ReadInt(bs, 32);
			if (sender_ssrc == ch->SenderSSRC) ch->total_bytes = val;

			rtcp_hdr.Length -= 5;
			
			//common encoding for SR and RR
			goto process_reports;


		case 201:
			//sender ssrc
			sender_ssrc = BS_ReadInt(bs, 32);
			rtcp_hdr.Length -= 1;

process_reports:
			//process all reports - we actually don't since we do not handle sources
			//to add
			for (i=0; i<rtcp_hdr.Count; i++) {
				//ssrc slot
				cur_ssrc = BS_ReadInt(bs, 32);
				//frac lost
				BS_ReadInt(bs, 8);
				//cumulative lost
				BS_ReadInt(bs, 24);
				//extended seq num
				BS_ReadInt(bs, 32);
				//jitter
				BS_ReadInt(bs, 32);
				//LSR
				BS_ReadInt(bs, 32);
				//DLSR
				BS_ReadInt(bs, 32);

				rtcp_hdr.Length -= 6;
			}
			//remaining bytes? we skip (this includes padding and crypto - not supported)
			while (rtcp_hdr.Length) {
				BS_ReadInt(bs, 32);
				rtcp_hdr.Length -= 1;
			}
			break;

		//SDES
		case 202:
			for (i=0; i<rtcp_hdr.Count; i++) {
				cur_ssrc = BS_ReadInt(bs, 32);
				rtcp_hdr.Length -= 1;

				val = 0;
				while (1) {
					sdes_type = BS_ReadInt(bs, 8);
					val += 1;
					if (!sdes_type) break;
					sdes_len = BS_ReadInt(bs, 8);
					val += 1;
					BS_ReadData(bs, sdes_buffer, sdes_len);
					sdes_buffer[sdes_len] = 0;
					val += sdes_len;
				}

				//re-align on 32bit
				res = val%4;
				if (res) {
					BS_ReadInt(bs, 8*(4-res));
					val = val/4 + 1;
				} else {
					val = val/4;
				}
				rtcp_hdr.Length -= val;
			}
			break;

		//BYE packet - close the channel - we work with 1 SSRC only */
		case 203:
			for (i=0; i<rtcp_hdr.Count; i++) {
				cur_ssrc = BS_ReadInt(bs, 32);
				rtcp_hdr.Length -= 1;
				if (ch->SenderSSRC == cur_ssrc) {
					e = M4EOF;
					break;
				}
			}
			//extra info - skip it
			while (rtcp_hdr.Length) {
				BS_ReadInt(bs, 32);
				rtcp_hdr.Length -= 1;
			}
			break;
/*
		//APP packet
		case 204:


			//sender ssrc
			sender_ssrc = BS_ReadInt(bs, 32);
			//ASCI 4 char type
			BS_ReadInt(bs, 8);
			BS_ReadInt(bs, 8);
			BS_ReadInt(bs, 8);
			BS_ReadInt(bs, 8);
			
			rtcp_hdr.Length -= 2;

			//till endd of pck
			BS_ReadData(bs, sdes_buffer, rtcp_hdr.Length*4);
			rtcp_hdr.Length = 0;
			break;
*/
		default:
			//read all till end
			BS_ReadData(bs, sdes_buffer, rtcp_hdr.Length*4);
			rtcp_hdr.Length = 0;
			break;
		}
		//WE SHALL CONSUME EVERYTHING otherwise the packet is bad
		if (rtcp_hdr.Length) {
			DeleteBitStream(bs);
			return M4CorruptedData;
		}
	}

	DeleteBitStream(bs);
	return e;
}


static u32 RTCP_FormatReport(RTPChannel *ch, BitStream *bs, u32 NTP_Time)
{
	u32 length, is_sr, sec, frac, expected, val, size;
	s32 extended, expect_diff, loss_diff;
	Double f;

	is_sr = ch->pck_sent_since_last_sr ? 1 : 0;

	//common header
	//version
	BS_WriteInt(bs, 2, 2);
	//padding - reports are aligned
	BS_WriteInt(bs, 0, 1);
	//count - only one for now in RR, 0 in sender mode
	BS_WriteInt(bs, !is_sr, 5);
	//if we have sent stuff send an SR, otherwise an RR. We need to determine whether 
	//we are active or not
	//type
	BS_WriteInt(bs, is_sr ? 200 : 201, 8);
	//length = (num of 32bit words in full pck) - 1
	//we're updating only one ssrc for now in RR and none in SR
	length = is_sr ? 6 : (1 + 6 * 1);
	BS_WriteInt(bs, length, 16);

	//sender SSRC
	BS_WriteInt(bs, ch->SSRC, 32);

	size = 8;

	//SenderReport part
	if (is_sr) {
		M4_GetNTP(&sec, &frac);
		//sender time
		BS_WriteInt(bs, sec, 32);
		BS_WriteInt(bs, frac, 32);
		//RTP time at this time
		f = 1000 * (sec - ch->last_pck_ntp_sec);
		f += ((frac - ch->last_pck_ntp_frac) >> 4) / 0x10000;
		f /= 1000;
		f *= ch->TimeScale;
		val = (u32) f + ch->last_pck_ts;
		BS_WriteInt(bs, val, 32);
		//num pck sent
		BS_WriteInt(bs, ch->num_pck_sent, 32);
		//num payload bytes sent
		BS_WriteInt(bs, ch->num_payload_bytes, 32);


		size += 20;
		//nota: as we only support single-way channels we are done for SR...
		return size;
	}
	//loop through all our sources (1) and send information...
	BS_WriteInt(bs, ch->SenderSSRC, 32);

	//Fraction lost and cumulative lost
	extended = ( (ch->num_sn_loops << 16) | ch->last_pck_sn);
	expected = extended - ch->rtp_first_SN;
	expect_diff = expected - ch->tot_num_pck_expected;
	loss_diff = expect_diff - ch->last_num_pck_rcv;	

	if (!expect_diff || (loss_diff <= 0)) loss_diff = 0;
	else loss_diff = (loss_diff<<8) / expect_diff;

	if (ch->rtp_log)
		fprintf(ch->rtp_log, "SSRC %d Sending report at %u: %d extended - since last: %d expected %d loss %u Jitter\n", 
								ch->SSRC, NTP_Time, extended, expect_diff, loss_diff, ch->Jitter >> 4);

	
	BS_WriteInt(bs, loss_diff, 8);

	//update and write cumulative loss
	ch->tot_num_pck_rcv += ch->last_num_pck_rcv;
	ch->tot_num_pck_expected = expected;
	BS_WriteInt(bs, (expected - ch->tot_num_pck_rcv), 24);

	//Extend sequence number
	BS_WriteInt(bs, extended, 32);

	
	//Jitter
	//RTP specs annexe A.8
	BS_WriteInt(bs, ( ch->Jitter >> 4) , 32);
	//LSR
	val = ch->last_SR_NTP_sec ? M4_GetNTP32(ch->last_SR_NTP_sec, ch->last_SR_NTP_frac) : 0;
	BS_WriteInt(bs, val, 32);

	// DLSR
	BS_WriteInt(bs, (NTP_Time - ch->last_report_time), 32);

	size += 24;
	return size;
}


static u32 RTCP_FormatSDES(RTPChannel *ch, BitStream *bs)
{
	u32 length, padd;

	//one item (type, len, data) + \0 marker at the end of the item
	length = 2+strlen(ch->CName) + 1;
//	length = 2+strlen(ch->CName);
	padd = length % 4;
	if (padd*4 != length) {
		//padding octets
		padd = 4 - padd;
		//header length
		length = length/4 + 1;
	} else {
		padd = 0;
		length = length/4;
	}
	//add SSRC
	length += 1;
	
	//common part as usual
	BS_WriteInt(bs, 2, 2);
	//notify padding? according to RFC1889 "In a compound RTCP packet, padding should 
	//only be required on the last individual packet because the compound packet is 
	//encrypted as a whole" -> we write it without notifying it (this is a bit messy in 
	//the spec IMO)
	BS_WriteInt(bs, 0, 1);
	BS_WriteInt(bs, 1, 5);
	//SDES pck type
	BS_WriteInt(bs, 202, 8);
	//length
	BS_WriteInt(bs, length, 16);

	//SSRC
	BS_WriteInt(bs, ch->SSRC, 32);

	//CNAME type
	BS_WriteInt(bs, 1, 8);
	//length and cname
	BS_WriteInt(bs, strlen(ch->CName), 8);	
	BS_WriteData(bs, ch->CName, strlen(ch->CName));

	BS_WriteInt(bs, 0, 8);

	//32-align field with 0
	BS_WriteInt(bs, 0, 8*padd);
	return (length + 1)*4;
}


static u32 RTCP_FormatBYE(RTPChannel *ch, BitStream *bs)
{
	//version
	BS_WriteInt(bs, 2, 2);
	//no padding
	BS_WriteInt(bs, 0, 1);
	//count - only one for now
	BS_WriteInt(bs, 1, 5);
	//type=BYE
	BS_WriteInt(bs, 203, 8);
	//length = (num of 32bit words in full pck) - 1
	BS_WriteInt(bs, 1, 16);

	//sender SSRC
	BS_WriteInt(bs, ch->SSRC, 32);
	return 8;
}

M4Err RTCP_SendBye(RTPChannel *ch,
						M4Err (*RTP_TCPCallback)(void *cbk, char *pck, u32 pck_size),
						void *rtsp_cbk)
{
	BitStream *bs;
	u32 report_size;
	char *report_buf;
	M4Err e = M4OK;

	bs = NewBitStream(NULL, 0, BS_WRITE);

	//pck were recieved/sent send the RR/SR - note we don't wait for next Report
	//and force its emission now
	if (ch->last_num_pck_rcv || ch->pck_sent_since_last_sr) {
		RTCP_FormatReport(ch, bs, RTP_GetReportTime());
	}

	//always send SDES (CNAME shall be sent at each RTCP)
	RTCP_FormatSDES(ch, bs);

	//send BYE
	RTCP_FormatBYE(ch, bs);


	report_buf = NULL;
	report_size = 0;
	BS_GetContent(bs, (unsigned char **) &report_buf, &report_size);
	DeleteBitStream(bs);

	if (ch->rtcp) {
		e = SK_Send(ch->rtcp, report_buf, report_size);
	} else {
		if (RTP_TCPCallback) 
			e = RTP_TCPCallback(rtsp_cbk, report_buf, report_size);
		else
			e = M4BadParam;
	}
	free(report_buf);
	return e;
}

M4Err RTCP_SendReport(RTPChannel *ch, 
						M4Err (*RTP_TCPCallback)(void *cbk, char *pck, u32 pck_size),
						void *rtsp_cbk)
{
	u32 Time, report_size;
	BitStream *bs;
	char *report_buf;
	M4Err e = M4OK;

	if (ch->first_SR) return M4OK;
	Time = RTP_GetReportTime();
	if ( Time < ch->next_report_time) return M4OK;

	bs = NewBitStream(NULL, 0, BS_WRITE);

	//pck were recieved/sent send the RR/SR
	if (ch->last_num_pck_rcv || ch->pck_sent_since_last_sr) {
		RTCP_FormatReport(ch, bs, Time);
	}

	//always send SDES (CNAME shall be sent at each RTCP)
	RTCP_FormatSDES(ch, bs);


	//get content
	report_buf = NULL;
	report_size = 0;
	BS_GetContent(bs, (unsigned char **) &report_buf, &report_size);
	DeleteBitStream(bs);


	if (ch->rtcp) {
		e = SK_Send(ch->rtcp, report_buf, report_size);
	} else {
		if (RTP_TCPCallback) 
			e = RTP_TCPCallback(rtsp_cbk, report_buf, report_size);
		else
			e = M4BadParam;
	}

	ch->rtcp_bytes_sent += report_size;

	free(report_buf);
	
	if (!e) {
		//Update the channel record if no error - otherwise next RTCP will triger an RR
		ch->last_num_pck_rcv = ch->last_num_pck_expected = ch->last_num_pck_loss = 0;
		RTCP_GetNextReportTime(ch);
	}
	return e;
}



#define RTCP_SAFE_FREE(p) if (p) free(p);	\
					p = NULL;

M4Err RTCP_SetInfo(RTPChannel *ch, u32 InfoCode, char *info_string)
{
	if (!ch) return M4BadParam;

	switch (InfoCode) {
	case RTCP_INFO_NAME:
		RTCP_SAFE_FREE(ch->s_name);
		if (info_string) ch->s_name = strdup(info_string);
		break;
	case RTCP_INFO_EMAIL:
		RTCP_SAFE_FREE(ch->s_email);
		if (info_string) ch->s_email = strdup(info_string);
		break;
	case RTCP_INFO_PHONE:
		RTCP_SAFE_FREE(ch->s_phone);
		if (info_string) ch->s_phone = strdup(info_string);
		break;
	case RTCP_INFO_LOCATION:
		RTCP_SAFE_FREE(ch->s_location);
		if (info_string) ch->s_location = strdup(info_string);
		break;
	case RTCP_INFO_TOOL:
		RTCP_SAFE_FREE(ch->s_tool);
		if (info_string) ch->s_tool = strdup(info_string);
		break;
	case RTCP_INFO_NOTE:
		RTCP_SAFE_FREE(ch->s_note);
		if (info_string) ch->s_note = strdup(info_string);
		break;
	case RTCP_INFO_PRIV:
		RTCP_SAFE_FREE(ch->s_priv);
		if (info_string) ch->s_name = strdup(info_string);
		break;
	default:
		return M4BadParam;
	}
	return M4OK;
}
