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

 /*
			3GPP AMR and H263 hinting written by Andrew Voznytsa
 */

#include <gpac/intern/m4_author_dev.h>


static const u32 amr_nb_frame_size_bytes[16] = {
	12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0
};

static const u32 amr_wb_frame_size_bytes[16] = {
	17, 23, 32, 36, 40, 46, 50, 58, 60, 5, 5, 0, 0, 0, 0, 0 
};

M4Err M4RTP_ProcessAMR(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize)
{
	BitStream *hdr, *bs;
	u8 q, ft, f;

	/* cmr + max 15 toc entries because of 3GPP limitation. only 15 AMR frames can be saved in one M4Sample*/
	u8	payload[1 + 15];
	u32 i, bitstream_offset, bitstream_size, max_bitstream_size, frame_size, number_of_frames;

	builder->rtp_header.TimeStamp = (u32) builder->sl_header.compositionTimeStamp;
	builder->rtp_header.Marker = 1;
	
	bitstream_offset = 0;
	/*the AMR hinter doesn't perform inter-sample concatenation*/
	if (!data) return M4OK;

	bs = NewBitStream(data, data_size, BS_READ);
	f = BS_ReadInt(bs, 1);
	ft = BS_ReadInt(bs, 4);
	q = BS_ReadInt(bs, 1);
	BS_ReadInt(bs, 2);	/*pad (octet align)*/
	DeleteBitStream(bs);

	if (builder->rtp_payt == RTP_PAYT_AMR_WB) {
		frame_size = amr_nb_frame_size_bytes[ft];
	} else {
		frame_size = amr_wb_frame_size_bytes[ft];
	}
	/* + 1 byte because of header */
	frame_size += 1;

	number_of_frames = MIN(15, MIN(data_size / frame_size, builder->Path_MTU / frame_size));
	max_bitstream_size = number_of_frames * frame_size;

	hdr = NULL;
	while (data_size > 0){
		if(data_size > max_bitstream_size){
			bitstream_size = max_bitstream_size;
		}else{
			bitstream_size = data_size;
		}

		data_size -= bitstream_size;

		/*write toc*/
		bs = NewBitStream(payload, 16, BS_WRITE);
		BS_WriteInt(bs, 1, 1); /*0 or 1 ? shouldn't be used..*/
		BS_WriteInt(bs, ft, 4);
		BS_WriteInt(bs, q, 1);	
		BS_WriteInt(bs, 0, 2);
		for(i = 0; i < number_of_frames; i++){
			BS_WriteInt(bs, (i + 1 < number_of_frames) ? 1 : f, 1);
			BS_WriteInt(bs, ft, 4);
			BS_WriteInt(bs, q, 1);
			BS_WriteInt(bs, 0, 2);
		}
		DeleteBitStream(bs);

		builder->rtp_header.SequenceNumber += 1;
		/*new packet*/
		builder->OnNewPacket(builder->cbk_obj, &builder->rtp_header);
		if (!builder->OnDataReference) hdr = NewBitStream(NULL, 0, BS_WRITE);

		/*add data reference or copy data*/
		for(i = 0; i < number_of_frames; i++) {
			if (builder->OnDataReference) {
				builder->OnDataReference(builder->cbk_obj, frame_size - 1, bitstream_offset + i * frame_size + 1);
			} else {
				BS_WriteData(hdr, data + bitstream_offset + i * frame_size + 1, frame_size - 1);
			}
		}

		/*done with packet*/
		if (hdr) {
			char *dtmp;
			u32 dsize;
			BS_GetContent(hdr, (unsigned char **) &dtmp, &dsize);
			DeleteBitStream(hdr);
			hdr = NULL;
			builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, (char*)payload, 1 + number_of_frames, dtmp, dsize);
		} else {
			builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, (char*)payload, 1 + number_of_frames, NULL, 0);
		}
		bitstream_offset += bitstream_size;
		/*this is a silence frame*/
		if (!number_of_frames) break;
	}
	assert(hdr==NULL);
	return M4OK;
}


M4Err M4RTP_ProcessH263(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize)
{
	BitStream *bs;
	char hdr[2];
	Bool Pbit;
	u32 offset, size, max_size;

	builder->rtp_header.TimeStamp = (u32) builder->sl_header.compositionTimeStamp;
	
	/*the H263 hinter doesn't perform inter-sample concatenation*/
	if (!data) return M4OK;

	Pbit = 1;

	/*skip 16 0'ed bits of start code*/
	offset = 2;
	data_size -= 2;
	max_size = builder->Path_MTU - 2;

	while(data_size > 0) {
		if(data_size > max_size){
			size = max_size;
			builder->rtp_header.Marker = 0;
		}else{
			size = data_size;
			builder->rtp_header.Marker = 1;
		}

		data_size -= size;

		/*create new RTP Packet */
		builder->rtp_header.SequenceNumber += 1;
		builder->OnNewPacket(builder->cbk_obj, &builder->rtp_header);
		
		/*add reference*/
		if (builder->OnDataReference) 
			builder->OnDataReference(builder->cbk_obj, size, offset);

		bs = NewBitStream(hdr, 2, BS_WRITE);
		BS_WriteInt(bs, 0, 5);
		BS_WriteInt(bs, Pbit, 1);
		BS_WriteInt(bs, 0, 10);
		DeleteBitStream(bs);

		/*add payload header - we pass the payload data too, and let the user decide what to do*/
		builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, (char*) hdr, 2, data + offset, size);

		offset += size;
		Pbit = 0;
	}
	return M4OK;
}

M4Err M4RTP_ProcessText(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize, u32 duration, u8 descIndex)
{
	BitStream *bs;
	unsigned char *hdr;
	u32 samp_size, txt_size, pay_start, hdr_size, txt_done, cur_frag, nb_frag;
	Bool is_utf_16 = 0;
	
	if (!data) {
		/*flush packet*/
		if (builder->bytesInPacket) {
			builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, NULL, 0, NULL, 0);
			builder->bytesInPacket = 0;
		}
		return M4OK;
	}
	/*cfg packet*/
	txt_size = data[0]; txt_size <<= 8; txt_size |= (unsigned char) data[1];
	/*remove BOM*/
	pay_start = 2;
	if (txt_size>2) {
		/*seems 3GP only accepts BE UTF-16 (no LE, no UTF32)*/
		if (((u8) data[2]==(u8) 0xFE) && ((u8) data[3]==(u8) 0xFF)) {
			is_utf_16 = 1;
			pay_start = 4;
			txt_size -= 2;
		}
	}
	samp_size = data_size - pay_start;

	/*if TTU does not fit in packet flush packet*/
	if (builder->bytesInPacket && (builder->bytesInPacket + 3 + 6 + samp_size > builder->Path_MTU)) {
		builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, NULL, 0, NULL, 0);
		builder->bytesInPacket = 0;
	}

	/*first TTU in packet*/
	if (!builder->bytesInPacket) {
		builder->rtp_header.TimeStamp = (u32) builder->sl_header.compositionTimeStamp;
		builder->rtp_header.Marker = 1;
		builder->rtp_header.SequenceNumber += 1;
		builder->OnNewPacket(builder->cbk_obj, &builder->rtp_header);
	}
	/*fits entirely*/
	if (builder->bytesInPacket + 3 + 6 + samp_size <= builder->Path_MTU) {
		bs = NewBitStream(NULL, 0, BS_WRITE);
		BS_WriteInt(bs, is_utf_16, 1);
		BS_WriteInt(bs, 0, 4);
		BS_WriteInt(bs, 1, 3);
		BS_WriteInt(bs, 8 + samp_size, 16);
		BS_WriteInt(bs, descIndex, 8);
		BS_WriteInt(bs, duration, 24);
		BS_WriteInt(bs, txt_size, 16);
		BS_GetContent(bs, &hdr, &hdr_size);
		DeleteBitStream(bs);
		builder->OnData(builder->cbk_obj, hdr, hdr_size);
		builder->bytesInPacket += hdr_size;
		free(hdr);
		
		if (txt_size) {
			if (builder->OnDataReference) {
				builder->OnDataReference(builder->cbk_obj, samp_size, pay_start);
			} else {
				builder->OnData(builder->cbk_obj, data + pay_start, samp_size);
			}
			builder->bytesInPacket += samp_size;
		}
		/*disable aggregation*/
		if (!(builder->flags & M4HF_UseMulti)) {
			builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, NULL, 0, NULL, 0);
			builder->bytesInPacket = 0;
		}
		return M4OK;
	}
	/*doesn't fit and already data, flush packet*/
	if (builder->bytesInPacket) {
		builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, NULL, 0, NULL, 0);
		builder->rtp_header.TimeStamp = (u32) builder->sl_header.compositionTimeStamp;
		/*split unit*/
		builder->rtp_header.Marker = 0;
		builder->rtp_header.SequenceNumber += 1;
		builder->OnNewPacket(builder->cbk_obj, &builder->rtp_header);
		builder->bytesInPacket = 0;
	}
	/*write all type2 units (text only) - FIXME: split at char boundaries, NOT SUPPORTED YET*/
	txt_done = 0;
	nb_frag = 1;
	/*all fragments needed for Type2 units*/
	while (txt_done + (builder->Path_MTU-10) < txt_size) {
		txt_done += (builder->Path_MTU-10);
		nb_frag += 1;
	}
	/*all fragments needed for Type3/4 units*/
	txt_done = txt_size;
	while (txt_done + (builder->Path_MTU-7) < samp_size) {
		txt_done += (builder->Path_MTU-7);
		nb_frag += 1;
	}


	cur_frag = 0;
	txt_done = 0;
	while (txt_done<txt_size) {
		u32 size;
		if (txt_done + (builder->Path_MTU-10) < txt_size) {
			size = builder->Path_MTU-10;
		} else {
			size = txt_size - txt_done;
		}

		bs = NewBitStream(NULL, 0, BS_WRITE);
		BS_WriteInt(bs, is_utf_16, 1);
		BS_WriteInt(bs, 0, 4);
		BS_WriteInt(bs, 2, 3);
		BS_WriteInt(bs, 9 + size, 16);
		BS_WriteInt(bs, nb_frag, 4);
		BS_WriteInt(bs, cur_frag, 4);
		BS_WriteInt(bs, duration, 24);
		BS_WriteInt(bs, descIndex , 8);
		/*SLEN is the full original length minus text len and BOM (put here for buffer allocation purposes)*/
		BS_WriteInt(bs, samp_size, 16);
		BS_GetContent(bs, &hdr, &hdr_size);
		DeleteBitStream(bs);
		builder->OnData(builder->cbk_obj, hdr, hdr_size);
		builder->bytesInPacket += hdr_size;
		free(hdr);
	
		if (builder->OnDataReference) {
			builder->OnDataReference(builder->cbk_obj, size, pay_start + txt_done);
		} else {
			builder->OnData(builder->cbk_obj, data + pay_start + txt_done, size);
		}
		builder->bytesInPacket += size;
		cur_frag++;
		
		/*flush packet*/
		if (cur_frag == nb_frag) {
			txt_done = txt_size;
			if (pay_start + txt_done == data_size) {
				builder->rtp_header.Marker = 1;
				builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, NULL, 0, NULL, 0);
				builder->bytesInPacket = 0;
			}
		} else {
			txt_done += size;
			builder->rtp_header.Marker = 0;
			builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, NULL, 0, NULL, 0);
			builder->rtp_header.SequenceNumber += 1;
			builder->OnNewPacket(builder->cbk_obj, &builder->rtp_header);
			builder->bytesInPacket = 0;
		}
	}

	txt_done = txt_size;

	/*write all modifiers - OPT: split at modifiers boundaries*/
	while (txt_done<samp_size) {
		u32 size, type;
		type = (txt_done == txt_size) ? 3 : 4;

		if (txt_done + (builder->Path_MTU-7) < samp_size) {
			size = builder->Path_MTU-10;
		} else {
			size = samp_size - txt_done;
		}

		bs = NewBitStream(NULL, 0, BS_WRITE);
		BS_WriteInt(bs, is_utf_16, 1);
		BS_WriteInt(bs, 0, 4);
		BS_WriteInt(bs, type, 3);
		BS_WriteInt(bs, 6 + size, 16);
		BS_WriteInt(bs, nb_frag, 4);
		BS_WriteInt(bs, cur_frag, 4);
		BS_WriteInt(bs, duration, 24);

		BS_GetContent(bs, &hdr, &hdr_size);
		DeleteBitStream(bs);
		builder->OnData(builder->cbk_obj, hdr, hdr_size);
		builder->bytesInPacket += hdr_size;
		free(hdr);

		if (builder->OnDataReference) {
			builder->OnDataReference(builder->cbk_obj, size, pay_start + txt_done);
		} else {
			builder->OnData(builder->cbk_obj, data + pay_start + txt_done, size);
		}
		builder->bytesInPacket += size;
		cur_frag++;
		if (cur_frag==nb_frag) {
			builder->rtp_header.Marker = 1;
			builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, NULL, 0, NULL, 0);
			builder->bytesInPacket = 0;
		} else {
			builder->rtp_header.Marker = 0;
			builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, NULL, 0, NULL, 0);
			builder->rtp_header.SequenceNumber += 1;
			builder->OnNewPacket(builder->cbk_obj, &builder->rtp_header);
			builder->bytesInPacket = 0;
		}
		txt_done += size;
	}
	return M4OK;
}
