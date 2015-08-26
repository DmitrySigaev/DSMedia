/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / common tools sub-project
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

#include <gpac/m4_tools.h>

/*the default size for new streams allocation...*/
#define BS_MEM_BLOCK_ALLOC_SIZE		250

/*private mode if we own the buffer*/
#define BS_WRITE_DYN	5

struct BS
{
	/*original stream data*/
	FILE *stream;

	/*original data*/
	char *original;
	/*the size of our buffer*/
	u64 size;
	/*current position in BYTES*/
	u64 position;
	/*the byte readen/written */
	u32 current;
	/*the number of bits in the current byte*/
	u32 nbBits;
	/*the bitstream mode*/
	u32 bsmode;

	void (*EndOfStream)(void *par);
	void *par;
};


BitStream *NewBitStream(unsigned char *buffer, u64 BufferSize, u32 mode)
{
	BitStream *tmp;
	if ( (buffer && ! BufferSize)) return NULL;

	tmp = (BitStream *)malloc(sizeof(BitStream));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(BitStream));

	tmp->original = (char*)buffer;
	tmp->size = BufferSize;

	tmp->position = 0;
	tmp->current = 0;
	tmp->bsmode = mode;
	tmp->stream = NULL;

	switch (tmp->bsmode) {
	case BS_READ:
		tmp->nbBits = 8;
		tmp->current = 0;
		break;
	case BS_WRITE:
		tmp->nbBits = 0;
		if (! buffer) {
			/*if BufferSize is specified, use it. This is typically used when AvgSize of
			some buffers is known, but some exceed it.*/
			if (BufferSize) {
				tmp->size = BufferSize;
			} else {
				tmp->size = BS_MEM_BLOCK_ALLOC_SIZE;
			}
			tmp->original = (char *) malloc(sizeof(char) * ((u32) tmp->size));
			if (! tmp->original) {
				free(tmp);
				return NULL;
			}
			tmp->bsmode = BS_WRITE_DYN;
		} else {
			tmp->original = (char*)buffer;
			tmp->size = BufferSize;
		}
		break;
	default:
		/*the stream constructor is not the same...*/
		free(tmp);
		return NULL;
	}
	return tmp;
}

BitStream *NewBitStreamFromFile(FILE *f, u32 mode)
{
	BitStream *tmp;
	if (!f) return NULL;

	if ( (mode != BS_FILE_READ) 
		&& (mode != BS_FILE_WRITE_ONLY) 
		&& (mode != BS_FILE_READ_WRITE) )
		return NULL;

	tmp = (BitStream *)malloc(sizeof(BitStream));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(BitStream));


	tmp->bsmode = mode;
	tmp->current = 0;
	tmp->nbBits = (mode == BS_FILE_READ) ? 8 : 0;
	tmp->original = NULL;
	tmp->position = 0;
	tmp->stream = f;

	/*get the size of this file (for read streams)*/
	tmp->position = f64_tell(f);
	f64_seek(f, 0, SEEK_END);
	tmp->size = f64_tell(f);
	f64_seek(f, tmp->position, SEEK_SET);
	return tmp;
}

void DeleteBitStream(BitStream *bs)
{
	if (!bs) return;
	/*if we are in dynamic mode (alloc done by the bitstream), free the buffer if still present*/
	if ((bs->bsmode == BS_WRITE_DYN) && bs->original) free(bs->original);
	free(bs);
}


/*returns 1 if aligned wrt current mode, 0 otherwise*/
Bool BS_IsAlign(BitStream *bs)
{
	switch (bs->bsmode) {
	case BS_READ:
	case BS_FILE_READ:
		return ( (8 == bs->nbBits) ? 1 : 0);
	default:
		return !bs->nbBits;
	}
}


/*fetch a new byte in the bitstream switch between packets*/
u8 BS_ReadByte(BitStream *bs)
{

#if 0
	/*we don't allow read on WRITE buffers*/
	if ( (bs->bsmode == BS_WRITE) 
		|| (bs->bsmode == BS_WRITE_DYN) 
		|| (bs->bsmode == BS_FILE_WRITE_ONLY) 
		
		) return 0;
	if (!bs->original && !bs->stream) return 0;
#endif

	if (bs->bsmode == BS_READ) {
		if (bs->position == bs->size) {
			if (bs->EndOfStream) bs->EndOfStream(bs->par);
			return 0;
		}
		return (u8) bs->original[bs->position++];
	}
	/*we are in FILE mode, test for end of file*/
	if (!feof(bs->stream)) {
		bs->position++;
		return fgetc(bs->stream);
	}
	if (bs->EndOfStream) bs->EndOfStream(bs->par);
	return 0;
}

u8 BS_ReadBit(BitStream *bs)
{
	s32 ret;
	if (bs->nbBits >= 8) {	
		bs->current = BS_ReadByte(bs);
	 	bs->nbBits = 0;
	}
	bs->current <<= 1;
	bs->nbBits++;
	ret = (bs->current & 0x100) >> 8;
	return (u8) ret;
}

u32 BS_ReadInt(BitStream *bs, u32 nBits)
{
	u32 ret = 0;
	while (nBits-- > 0) {
		ret <<= 1;
		ret |= BS_ReadBit(bs);
	}
	return ret;
}


u64 BS_ReadLongInt(BitStream *bs, u32 nBits)
{
	u64 ret = 0;
	while (nBits-- > 0) {
		ret <<= 1;
		ret |= BS_ReadBit(bs);
	}
	return ret;
}


float BS_ReadFloat(BitStream *bs)
{	
	char buf [4] = "\0\0\0";
	s32 i;
	for (i = 0; i < 32; i++)
		buf[3-i/8] |= BS_ReadBit(bs) << (7 - i%8);
	return (* (float *) buf);
}

double BS_ReadDouble(BitStream *bs)
{
	char buf [8] = "\0\0\0\0\0\0\0";
	s32 i;
	for (i = 0; i < 64; i++)
		buf[7-i/8] |= BS_ReadBit(bs) << (7 - i%8);
	return (* (double *) buf);
}

u32 BS_ReadData(BitStream *bs, unsigned char *data, u32 nbBytes)
{
	u64 orig = bs->position;

	if (bs->position+nbBytes > bs->size) return 0;

	if (BS_IsAlign(bs)) {
		switch (bs->bsmode) {
		case BS_READ:
			memcpy(data, bs->original + bs->position, nbBytes);
			bs->position += nbBytes;
			return nbBytes;
		case BS_FILE_READ:
		case BS_FILE_READ_WRITE:
			fread(data, nbBytes, 1, bs->stream);
			bs->position += nbBytes;
			return nbBytes;
		default:
			return 0;
		}
	}

	while (nbBytes-- > 0) {
		*data++ = BS_ReadInt(bs, 8);
	}
	return (u32) (bs->position - orig);

}



void BS_WriteByte(BitStream *bs)
{
	/*we don't allow write on READ buffers*/
	if ( (bs->bsmode == BS_READ) || (bs->bsmode == BS_FILE_READ) ) return;
	if (!bs->original && !bs->stream) return;

	/*we are in MEM mode*/
	if ( (bs->bsmode == BS_WRITE) || (bs->bsmode == BS_WRITE_DYN) ) {
		if (bs->position == bs->size) {
			/*no more space...*/
			if (bs->bsmode != BS_WRITE_DYN) return;
			/*realloc if enough space...*/
			if (bs->size > 0xFFFFFFFF) return;
			bs->original = (char*)realloc(bs->original, (u32) (bs->size + BS_MEM_BLOCK_ALLOC_SIZE));
			if (!bs->original) return;
			bs->size += BS_MEM_BLOCK_ALLOC_SIZE;
		}
		bs->original[bs->position] = bs->current;
		bs->position++;
		return;
	}
	/*we are in FILE mode, no pb for any realloc...*/
	fputc(bs->current, bs->stream);
	/*check we didn't rewind the stream*/
	if (bs->size == bs->position) bs->size++;
	bs->position += 1;
}

void BS_WriteBit(BitStream *bs, u32 bit)
{
	bs->current <<= 1;
	bs->current |= bit;
	if (++ bs->nbBits == 8) {
		bs->nbBits = 0;
		BS_WriteByte(bs);
		bs->current = 0;
	}
}

void BS_WriteInt(BitStream *bs, s32 value, s32 nBits)
{
	value <<= sizeof (s32) * 8 - nBits;
	while (--nBits >= 0) {
		BS_WriteBit (bs, value < 0);
		value <<= 1;
	}
}

void BS_WriteLongInt(BitStream *bs, s64 value, s32 nBits)
{
	value <<= sizeof (s64) * 8 - nBits;
	while (--nBits >= 0) {
		BS_WriteBit (bs, value < 0);
		value <<= 1;
	}
}

void BS_WriteFloat(BitStream *bs, Float value)
{
	u32 i;
	union
	{	float f;
		char sz [4];
	} float_value;
	float_value.f = value;

	for (i = 0; i < 32; i++)
		BS_WriteBit(bs, (float_value.sz [3 - i / 8] & 1 << (7 - i % 8)) != 0);

}

void BS_WriteDouble (BitStream *bs, Double value)
{
	u32 i;
	union
	{	Double d;
		char sz [8];
	} double_value;
	double_value.d = value;
	for (i = 0; i < 64; i++) {
		BS_WriteBit(bs, (double_value.sz [7 - i / 8] & 1 << (7 - i % 8)) != 0);
	}
}


u32 BS_WriteData(BitStream *bs, unsigned char *data, u32 nbBytes)
{
	/*we need some feedback for this guy...*/
	u64 begin = bs->position;
	if (!nbBytes) return 0;

	if (BS_IsAlign(bs)) {
		switch (bs->bsmode) {
		case BS_WRITE:
			if (bs->position+nbBytes > bs->size) 
				return 0;
			memcpy(bs->original + bs->position, data, nbBytes);
			bs->position += nbBytes;
			return nbBytes;
		case BS_WRITE_DYN:
			/*need to realloc ...*/
			if (bs->position+nbBytes > bs->size) {
				if (bs->size + nbBytes > 0xFFFFFFFF) 
					return 0;
				bs->original = (char*)realloc(bs->original, sizeof(u32)*((u32) bs->size + nbBytes));
				if (!bs->original) 
					return 0;
				bs->size += nbBytes;
			}
			memcpy(bs->original + bs->position, data, nbBytes);
			bs->position += nbBytes;
			return nbBytes;
		case BS_FILE_WRITE_ONLY:
		case BS_FILE_READ_WRITE:
			fwrite(data, nbBytes, 1, bs->stream);
			if (bs->size == bs->position) bs->size += nbBytes;
			bs->position += nbBytes;
			return nbBytes;
		default:
			return 0;
		}
	}

	while (nbBytes) {
		BS_WriteInt(bs, (s32) *data, 8);
		data++;
		nbBytes--;
	}
	return (u32) (bs->position - begin);
}

/*align return the num of bits read in READ mode, 0 in WRITE*/
u8 BS_Align(BitStream *bs)
{
	if ( (bs->bsmode == BS_READ) || (bs->bsmode == BS_FILE_READ)) {
		u8 res = 8 - bs->nbBits;
		if (res > 0) {
			BS_ReadInt(bs, res);
		}
		return res;
	}
	if (bs->nbBits > 0)
		BS_WriteInt (bs, 0, 8 - bs->nbBits);
	return 0;
}


/*size available in the bitstream*/
u64 BS_Available(BitStream *bs)
{
	s64 cur, end;

	/*in WRITE mode only, this should not be called, but return something big in case ...*/
	if ( (bs->bsmode == BS_WRITE) 
		|| (bs->bsmode == BS_WRITE_DYN) 
		|| (bs->bsmode == BS_FILE_WRITE_ONLY))
		return -1;

	/*we are in MEM mode*/
	if (bs->bsmode == BS_READ) {
		return (bs->size - bs->position);
	}
	/*FILE READ: assume size hasn't changed, otherwise the user shall call BS_GetRefreshedSize*/
	if (bs->bsmode==BS_FILE_READ) return (bs->size - bs->position);

	cur = f64_tell(bs->stream);
	f64_seek(bs->stream, 0, SEEK_END);
	end = f64_tell(bs->stream);
	f64_seek(bs->stream, cur, SEEK_SET);	
	return (u64) (end - cur);
}

/*call this funct to set the buffer size to the nb of bytes written 
Used only in WRITE mode, as we don't know the real size during allocation...
return -1 for bad param or malloc failed
return nbBytes cut*/
u32 BS_CutBuffer(BitStream *bs)
{	
	u32 nbBytes;
	if ( (bs->bsmode != BS_WRITE_DYN) && (bs->bsmode != BS_WRITE)) return -1;
	/*Align our buffer or we're dead!*/
	BS_Align(bs);

	nbBytes = (u32) (bs->size - bs->position);
	if (!nbBytes || (nbBytes == 0xFFFFFFFF) || (bs->position >= 0xFFFFFFFF)) return 0;
	bs->original = (char*)realloc(bs->original, (u32) bs->position);
	if (! bs->original) return -1;
	/*just in case, re-adjust..*/
	bs->size = bs->position;
	return nbBytes;
}

/*For DYN mode, this gets the content out*/
void BS_GetContent(BitStream *bs, unsigned char **output, u32 *outSize)
{
	/*only in WRITE MEM mode*/
	if (bs->bsmode != BS_WRITE_DYN) return;
	if (!bs->position && !bs->nbBits) {
		*output = NULL;
		*outSize = 0;
		free(bs->original);
	} else {
		BS_CutBuffer(bs);
		*output = (unsigned char*)bs->original;
		*outSize = (u32) bs->size;
	}
	bs->original = NULL;
	bs->size = 0;
	bs->position = 0;
}

/*	Skip nbytes. 
	Align
	If READ (MEM or FILE) mode, just read n times 8 bit
	If WRITE (MEM or FILE) mode, write n times 0 on 8 bit
*/
void BS_SkipBytes(BitStream *bs, u64 nbBytes)
{
	if (!bs) return;

	BS_Align(bs);
	
	/*special case for file skipping...*/
	if ((bs->bsmode == BS_FILE_READ_WRITE) || (bs->bsmode == BS_FILE_READ)) {
		f64_seek(bs->stream, nbBytes, SEEK_CUR);
		bs->position += nbBytes;
		return;
	}

	/*special case for reading*/
	if (bs->bsmode == BS_READ) {
		bs->position += nbBytes;
		return;
	}
	/*for writing we must do it this way, otherwise pb in dynamic buffers*/
	while (nbBytes) {
		BS_WriteInt(bs, 0, 8);
		nbBytes--;
	}
}

/*Only valid for files right now AND ONLY READ*/
void BS_Rewind(BitStream *bs, u64 nbBytes)
{
	s64 cur;
	
	/*special case for file skipping...*/
	if (bs->bsmode != BS_FILE_READ) return;

	BS_Align(bs);
	cur = f64_tell(bs->stream);
	if ((u64) cur > nbBytes) {
		f64_seek(bs->stream, (cur - nbBytes), SEEK_SET);
		bs->position = cur - nbBytes;
		return;
	}
	f64_seek(bs->stream, 0, SEEK_SET);
	bs->position = 0;
}

/*Only valid for READ MEMORY*/
void BS_BitRewind(BitStream *bs, u64 nbBits)
{
	u64 nbBytes;
	if (bs->bsmode != BS_READ) return;

	nbBits -= (bs->nbBits);
	nbBytes = (nbBits+8)>>3;
	nbBits = nbBytes*8 - nbBits;
	BS_Align(bs);
	assert(bs->position >= nbBytes);
	bs->position -= nbBytes + 1;
	BS_ReadInt(bs, (u32)nbBits);
	return;
}

/*seek from begining of stream: use internally even when non aligned!*/
M4Err BS_SeekIntern(BitStream *bs, u64 offset)
{
	u32 i;
	/*if mem, do it */
	if ((bs->bsmode == BS_READ) || (bs->bsmode == BS_WRITE) || (bs->bsmode == BS_WRITE_DYN)) {
		if (offset > 0xFFFFFFFF) return M4IOErr;
		/*0 for write, read will be done automatically*/
		if (offset >= bs->size) {
			if ( (bs->bsmode == BS_READ) || (bs->bsmode == BS_WRITE) ) return M4BadParam;
			/*in DYN, realloc ...*/
			bs->original = (char*)realloc(bs->original, (u32) (offset + 1));
			for (i = 0; i < (u32) (offset + 1 - bs->size); i++) {
				bs->original[bs->size + i] = 0;
			}
			bs->size = offset + 1;
		}
		bs->current = bs->original[offset];
		bs->position = offset;
		bs->nbBits = (bs->bsmode == BS_READ) ? 8 : 0;
		return M4OK;
	}

	f64_seek(bs->stream, offset, SEEK_SET);

	bs->position = offset;
	bs->current = 0;
	/*setup NbBits so that next acccess to the buffer will trigger read/write*/
	bs->nbBits = (bs->bsmode == BS_FILE_READ) ? 8 : 0;
	return M4OK;
}

/*seek from begining of stream: align before anything else*/
M4Err BS_Seek(BitStream *bs, u64 offset)
{
	/*warning: we allow offset = bs->size for WRITE buffers*/
	if (offset > bs->size) return M4BadParam;

	BS_Align(bs);
	return BS_SeekIntern(bs, offset);
}

/*peek bits (as int!!) from orig position (ON BYTE BOUNDARIES, from 0) - only for read ...*/
u32 BS_PeekBits(BitStream *bs, u32 numBits, u64 orig)
{
	u64 curPos;
	u32 curBits, ret, current;

	if ( (bs->bsmode != BS_READ) && (bs->bsmode != BS_FILE_READ)) return 0;
	if ( (bs->bsmode != BS_READ) && (orig > 0xFFFFFFFF)) return 0;
	if (!numBits || (bs->size < bs->position + orig)) return 0;

	BS_Align(bs);

	/*store our state*/
	curPos = bs->position;
	curBits = bs->nbBits;
	current = bs->current;

	BS_Seek(bs, bs->position + orig);
	ret = BS_ReadInt(bs, numBits);

	/*restore our cache - position*/
	BS_Seek(bs, curPos);
	/*to avoid re-reading our bits ...*/
	bs->nbBits = curBits;
	bs->current = current;
	return ret;
}

M4Err BS_PeekData(BitStream *bs, u64 offset, char *data, u32 size)
{
	if (!bs || !bs->original || offset + size > bs->size) return M4BadParam;

	memcpy(data, bs->original + (u32) offset, size);
	return M4OK;
}

u64 BS_GetRefreshedSize(BitStream *bs)
{
	s64 offset;

	switch (bs->bsmode) {
	case BS_READ:
	case BS_WRITE:
		return bs->size;

	default:
		offset = f64_tell(bs->stream);
		f64_seek(bs->stream, 0, SEEK_END);
		bs->size = f64_tell(bs->stream);
		f64_seek(bs->stream, offset, SEEK_SET);
		return bs->size;
	}
}

u64 BS_GetSize(BitStream *bs)
{
	return bs->size;
}

u64 BS_GetPosition(BitStream *bs)
{
	return bs->position;
}

Bool BS_BitsAvailable(BitStream *bs)
{
	/*we are in MEM mode*/
	if (bs->bsmode != BS_READ) return 0;
	if (bs->size > bs->position) return 1;
	if (bs->nbBits < 8) return 1;
	return 0;
}

void BS_SetEOSCallback(BitStream *bs, void (*EndOfStream)(void *par), void *par)
{
	bs->EndOfStream = EndOfStream;
	bs->par = par;
}


Bool BS_CheckVideoStartCode(BitStream *bs)
{
	if ((bs->position+4<bs->size) && ! bs->original[bs->position] && !bs->original[bs->position+1] && (bs->original[bs->position+2]==1)) return 1;
	return 0;
}
