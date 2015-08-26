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

#include <gpac/intern/m4_author_dev.h>

static void mpa12_do_flush(M4RTPBuilder *builder, Bool start_new)
{
	char *tmp1, *tmp2;
	u32 tmp1_size, tmp2_size;
	/*flush*/
	if (builder->auheader) {
		BS_GetContent(builder->auheader, (unsigned char **) &tmp1, &tmp1_size);
		BS_GetContent(builder->payload, (unsigned char **) &tmp2, &tmp2_size);
		builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, tmp1, tmp1_size, tmp2, tmp2_size);
		DeleteBitStream(builder->auheader);
		DeleteBitStream(builder->payload);
		free(tmp1);
		free(tmp2);
	}
	if (!start_new) return;

	builder->rtp_header.TimeStamp = (u32) builder->sl_header.compositionTimeStamp;
	builder->auheader = NewBitStream(NULL, 0, BS_WRITE);
	builder->payload = NewBitStream(NULL, 0, BS_WRITE);
	/*create new RTP Packet */
	builder->rtp_header.SequenceNumber += 1;
	builder->OnNewPacket(builder->cbk_obj, &builder->rtp_header);
	builder->first_sl_in_rtp = 1;
	builder->bytesInPacket = 0;
}

static M4Err M4RTP_ProcessMPEG12Audio(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize)
{
	u32 pck_size;
	u16 offset;

	/*if no data flush, if nothing start if not enough space restart*/
	if (!data || !builder->bytesInPacket || (builder->bytesInPacket + data_size > builder->Path_MTU)) {
		mpa12_do_flush(builder, data ? 1 : 0);
		if (!data) return M4OK;
	}

	offset = 0;
	while (data_size) {
		if (data_size + 4 < builder->Path_MTU) {
			pck_size = data_size;
		} else {
			pck_size = builder->Path_MTU - 4;
		}
		if (builder->first_sl_in_rtp) {
			BS_WriteInt(builder->auheader, 0, 16);
			BS_WriteInt(builder->auheader, offset, 16);
			builder->first_sl_in_rtp = 0;
		}
		/*add reference*/
		if (builder->OnDataReference) 
			builder->OnDataReference(builder->cbk_obj, pck_size, offset);

		/*write data*/		
		BS_WriteData(builder->payload, data + offset, pck_size);

		data_size -= pck_size;
		/*start new packet if fragmenting*/
		if (data_size) {
			offset += (u16) pck_size;
			mpa12_do_flush(builder, 1);
		}
	}
	/*if offset force flush*/
	if (offset) mpa12_do_flush(builder, 1);

	return M4OK;
}

s32 MPEG12_FindNextSliceStart(const u8 *pbuffer, u32 startoffset, u32 buflen, u32 *slice_offset);
s32 MPEG12_FindNextStartCode(const u8 *pbuffer, u32 buflen, u32 *optr, u32 *scode);

#define MPEG12_PICTURE_START_CODE         0x00000100
#define MPEG12_SEQUENCE_START_CODE        0x000001b3

static M4Err M4RTP_ProcessMPEG12Video(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize)
{
	u32 startcode, pic_type, max_pck_size, offset, prev_slice, next_slice;
	Bool start_with_slice, slices_done, got_slice, first_slice, have_seq;
	char mpv_hdr[4];
	char *payload, *buffer;

	/*no flsuh (no aggregation)*/
	if (!data) return M4OK;

	offset = 0;
	have_seq = 0;

    while (1) {
		u32 oldoffset;
		oldoffset = offset;
		if (MPEG12_FindNextStartCode(data + offset, data_size - offset, &offset, &startcode) < 0)
			break;

		offset += oldoffset;
		if (startcode == MPEG12_SEQUENCE_START_CODE) have_seq = 1;
		offset += 4;

		if (startcode == MPEG12_PICTURE_START_CODE) break;
	}

	max_pck_size = builder->Path_MTU - 4;

	payload = data + offset;
	pic_type = (payload[1] >> 3) & 0x7;
	/*first 6 bits (MBZ and T bit) not used*/
	/*temp ref on 10 bits*/
    mpv_hdr[0] = (payload[0] >> 6) & 0x3;
    mpv_hdr[1] = (payload[0] << 2) | ((payload[1] >> 6) & 0x3);
    mpv_hdr[2] = pic_type;
    mpv_hdr[3] = 0;

    if ((pic_type==2) || (pic_type== 3)) {
		mpv_hdr[3] = payload[3] << 5;
		if ((payload[4] & 0x80) != 0) mpv_hdr[3] |= 0x10;
		if (pic_type == 3) mpv_hdr[3] |= (payload[4] >> 3) & 0xf;
	}

	/*start packet*/
	builder->rtp_header.TimeStamp = (u32) builder->sl_header.compositionTimeStamp;
	builder->rtp_header.Marker = 1;
	builder->rtp_header.SequenceNumber += 1;
	builder->OnNewPacket(builder->cbk_obj, &builder->rtp_header);

	buffer = data;
    prev_slice = 0;
	start_with_slice = (MPEG12_FindNextSliceStart(buffer, offset, data_size, &next_slice) >= 0) ? 1 : 0;
    offset = 0;
	slices_done = 0;
	got_slice = start_with_slice;
	first_slice = 1;

	while (data_size > 0) {
		Bool last_pck;
		u32 len_to_write;
		
		if (data_size <= max_pck_size) {
			len_to_write = data_size;
			last_pck = 1; 
			prev_slice = 0;
		} else {
			got_slice = (!first_slice && !slices_done && (next_slice <= max_pck_size)) ? 1 : 0;
			first_slice = 0;
			last_pck = 0;
			
			while (!slices_done && (next_slice <= max_pck_size)) {
				prev_slice = next_slice;
				if (MPEG12_FindNextSliceStart(buffer, next_slice + 4, data_size, &next_slice) >= 0) {
					got_slice = 1;
				} else {
					slices_done = 1;
				}
			}
			if (got_slice) len_to_write = prev_slice;
			else len_to_write = MIN(max_pck_size, data_size);
		} 

		mpv_hdr[2] = pic_type;
		
		if (have_seq) {
			mpv_hdr[2] |= 0x20;
			have_seq = 0;
		}
		if (first_slice) mpv_hdr[2] |= 0x10;
		
		if (got_slice || last_pck) {
			mpv_hdr[2] |= 0x08;
			start_with_slice = 1;
		} else {
			start_with_slice = 0;
		}
		
		builder->OnData(builder->cbk_obj, mpv_hdr, 4);
		if (builder->OnDataReference) {
			builder->OnDataReference(builder->cbk_obj, len_to_write, offset);
		} else {
			builder->OnData(builder->cbk_obj, data + offset, len_to_write);
		}
		
		builder->rtp_header.Marker = last_pck ? 1 : 0;
		builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, NULL, 0, NULL, 0);
		
		offset += len_to_write;
		data_size -= len_to_write;
		prev_slice = 0;
		next_slice -= len_to_write;
		buffer += len_to_write;

		if (!last_pck) {
			builder->rtp_header.Marker = 0;
			builder->rtp_header.SequenceNumber += 1;
			builder->OnNewPacket(builder->cbk_obj, &builder->rtp_header);
		}
    }
	return M4OK;
}

M4Err M4RTP_ProcessMPEG12(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize)
{
	switch (builder->slMap.StreamType) {
	case M4ST_VISUAL:
		return M4RTP_ProcessMPEG12Video(builder, data, data_size, IsAUEnd, FullAUSize);
	case M4ST_AUDIO:
		return M4RTP_ProcessMPEG12Audio(builder, data, data_size, IsAUEnd, FullAUSize);
	default:
		return M4NotSupported;
	}
}
