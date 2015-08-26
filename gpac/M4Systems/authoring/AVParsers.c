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


#include <math.h>
#include <gpac/m4_author.h>

//video tools
#define VO_START_CODE					0x00
#define VOL_START_CODE					0x20
#define VOP_START_CODE					0xB6
#define VISOBJ_START_CODE				0xB5
#define VOS_START_CODE					0xB0
#define GOV_START_CODE					0xB3
#define UDTA_START_CODE					0xB2

typedef struct _tagM4V_Parser
{
	BitStream *bs;
	u32 current_object_type;
	u32 current_object_start;
	u32 tc_dec, prev_tc_dec, tc_disp, prev_tc_disp;
} M4V_Parser;


M4V_Parser *NewMPEG4VideoParser(unsigned char *data, u32 data_size)
{
	M4V_Parser *tmp;
	if (!data || !data_size) return NULL;
	tmp = malloc(sizeof(M4V_Parser));
	memset(tmp, 0, sizeof(M4V_Parser));
	tmp->bs = NewBitStream(data, data_size, BS_READ);
	return tmp;
}

void DeleteMPEG4VideoParser(M4V_Parser *m4v)
{
	DeleteBitStream(m4v->bs);
	free(m4v);
}


s32 M4V_LoadObject(M4V_Parser *m4v)
{
	Bool BS_CheckVideoStartCode(BitStream *bs);
	if (!m4v) return 0;

	while (BS_Available(m4v->bs)) {
		if (BS_CheckVideoStartCode(m4v->bs)) {
			m4v->current_object_start = (u32) BS_GetPosition(m4v->bs);
			BS_SkipBytes(m4v->bs, 3);
			m4v->current_object_type = BS_ReadInt(m4v->bs, 8);
			return (s32) m4v->current_object_type;
		}
		BS_SkipBytes(m4v->bs, 1);
	}
	return -1;
}

const char *M4V_VideoProfileName(u8 video_pl)
{
	switch (video_pl) {
	case 0x00: return "Reserved (0x00) Profile";
	case 0x01: return "Simple Profile @ Level 1";
	case 0x02: return "Simple Profile @ Level 2";
	case 0x03: return "Simple Profile @ Level 3";
	case 0x08: return "Simple Profile @ Level 0";
	case 0x10: return "Simple Scalable Profile @ Level 0";
	case 0x11: return "Simple Scalable Profile @ Level 1";
	case 0x12: return "Simple Scalable Profile @ Level 2";
	case 0x21: return "Core Profile @ Level 1";
	case 0x22: return "Core Profile @ Level 2";
	case 0x32: return "Main Profile @ Level 2";
	case 0x33: return "Main Profile @ Level 3";
	case 0x34: return "Main Profile @ Level 4";
	case 0x42: return "N-bit Profile @ Level 2";
	case 0x51: return "Scalable Texture Profile @ Level 1";
	case 0x61: return "Simple Face Animation Profile @ Level 1";
	case 0x62: return "Simple Face Animation Profile @ Level 2";
	case 0x63: return "Simple FBA Profile @ Level 1";
	case 0x64: return "Simple FBA Profile @ Level 2";
	case 0x71: return "Basic Animated Texture Profile @ Level 1";
	case 0x72: return "Basic Animated Texture Profile @ Level 2";
	case 0x81: return "Hybrid Profile @ Level 1";
	case 0x82: return "Hybrid Profile @ Level 2";
	case 0x91: return "Advanced Real Time Simple Profile @ Level 1";
	case 0x92: return "Advanced Real Time Simple Profile @ Level 2";
	case 0x93: return "Advanced Real Time Simple Profile @ Level 3";
	case 0x94: return "Advanced Real Time Simple Profile @ Level 4";
	case 0xA1: return "Core Scalable Profile @ Level1";
	case 0xA2: return "Core Scalable Profile @ Level2";
	case 0xA3: return "Core Scalable Profile @ Level3";
	case 0xB1: return "Advanced Coding Efficiency Profile @ Level 1";
	case 0xB2: return "Advanced Coding Efficiency Profile @ Level 2";
	case 0xB3: return "Advanced Coding Efficiency Profile @ Level 3";
	case 0xB4: return "Advanced Coding Efficiency Profile @ Level 4";
	case 0xC1: return "Advanced Core Profile @ Level 1";
	case 0xC2: return "Advanced Core Profile @ Level 2";
	case 0xD1: return "Advanced Scalable Texture @ Level1";
	case 0xD2: return "Advanced Scalable Texture @ Level2";
	case 0xE1: return "Simple Studio Profile @ Level 1";
	case 0xE2: return "Simple Studio Profile @ Level 2";
	case 0xE3: return "Simple Studio Profile @ Level 3";
	case 0xE4: return "Simple Studio Profile @ Level 4";
	case 0xE5: return "Core Studio Profile @ Level 1";
	case 0xE6: return "Core Studio Profile @ Level 2";
	case 0xE7: return "Core Studio Profile @ Level 3";
	case 0xE8: return "Core Studio Profile @ Level 4";
	case 0xF0: return "Advanced Simple Profile @ Level 0";
	case 0xF1: return "Advanced Simple Profile @ Level 1";
	case 0xF2: return "Advanced Simple Profile @ Level 2";
	case 0xF3: return "Advanced Simple Profile @ Level 3";
	case 0xF4: return "Advanced Simple Profile @ Level 4";
	case 0xF5: return "Advanced Simple Profile @ Level 5";
	case 0xF7: return "Advanced Simple Profile @ Level 3b";
	case 0xF8: return "Fine Granularity Scalable Profile @ Level 0";
	case 0xF9: return "Fine Granularity Scalable Profile @ Level 1";
	case 0xFA: return "Fine Granularity Scalable Profile @ Level 2";
	case 0xFB: return "Fine Granularity Scalable Profile @ Level 3";
	case 0xFC: return "Fine Granularity Scalable Profile @ Level 4";
	case 0xFD: return "Fine Granularity Scalable Profile @ Level 5";
	case 0xFF: return "Reserved Profile for escape";
	default: return "ISO Reserved Profile";
	}
}

void M4V_RewritePL(unsigned char *data, u32 dataLen, u8 PL)
{
	u32 pos = 0;
	while (pos+4<dataLen) {
		if (!data[pos] && !data[pos+1] && (data[pos+2]==1) && (data[pos+3]==VOS_START_CODE)) {
			u32 i;
			BitStream *bsr = NewBitStream(data, dataLen, BS_READ);
			BitStream *bsw = NewBitStream(data, dataLen, BS_WRITE);
			for (i=0; i<dataLen; i++) {
				u32 val = BS_ReadInt(bsr, 8);
				if (i==pos+4) {
					BS_WriteInt(bsw, PL, 8);
				} else {
					BS_WriteInt(bsw, val, 8);
				}
			}
			DeleteBitStream(bsr);
			DeleteBitStream(bsw);
			return;
		}
		pos ++;
	}
	/*FIXME - VOS is missing...*/
}

M4Err M4V_ParseConfig(M4V_Parser *m4v, M4VDecoderSpecificInfo *dsi)
{
	s32 o_type;
	u8 go, verid;
	s32 clock_rate;

	if (!m4v || !dsi) return M4BadParam;

	memset(dsi, 0, sizeof(M4VDecoderSpecificInfo));

	go = 1;
	while (go) {
		o_type = M4V_LoadObject(m4v);
		switch (o_type) {
		//vosh
		case VOS_START_CODE:
			dsi->VideoPL = (u8) BS_ReadInt(m4v->bs, 8);
			break;

		case VOL_START_CODE:
			verid = 0;
			dsi->RAP_stream = BS_ReadInt(m4v->bs, 1);
			dsi->objectType = BS_ReadInt(m4v->bs, 8);
			if (BS_ReadInt(m4v->bs, 1)) {
				verid = BS_ReadInt(m4v->bs, 4);
				BS_ReadInt(m4v->bs, 3);
			}
			if (BS_ReadInt(m4v->bs, 4) == 0xF) {
//				dsi->ratio_width = 
				BS_ReadInt(m4v->bs, 8);
//				dsi->ratio_height = 
				BS_ReadInt(m4v->bs, 8);
			}
			if (BS_ReadInt(m4v->bs, 1)) {
				BS_ReadInt(m4v->bs, 3);
				if (BS_ReadInt(m4v->bs, 1)) BS_ReadInt(m4v->bs, 79);
			}
			dsi->has_shape = BS_ReadInt(m4v->bs, 2);
			if (dsi->has_shape && (verid!=1) ) BS_ReadInt(m4v->bs, 4);
			BS_ReadInt(m4v->bs, 1);
			//clock rate
			dsi->clock_rate = BS_ReadInt(m4v->bs, 16);
			//marker
			BS_ReadInt(m4v->bs, 1);

			clock_rate = dsi->clock_rate-1;
			if (clock_rate >= 65536) clock_rate = 65535;
			if (clock_rate > 0) {
				for (dsi->NumBitsTimeIncrement = 1; dsi->NumBitsTimeIncrement < 16; dsi->NumBitsTimeIncrement++)	{	
					if (clock_rate == 1) break;
					clock_rate = (clock_rate >> 1);
				}
			} else {
				//fix from vivien for divX
				dsi->NumBitsTimeIncrement = 1;
			}			
			//fixed FPS stream
			dsi->time_increment = 0;
			if (BS_ReadInt(m4v->bs, 1)) {
				dsi->time_increment = BS_ReadInt(m4v->bs, dsi->NumBitsTimeIncrement);
			}
			if (!dsi->has_shape) {
				BS_ReadInt(m4v->bs, 1);
				dsi->width = BS_ReadInt(m4v->bs, 13);
				BS_ReadInt(m4v->bs, 1);
				dsi->height = BS_ReadInt(m4v->bs, 13);
			} else {
				dsi->width = dsi->height = 0;
			}
			//shape will be done later
			BS_Align(m4v->bs);
			break;

		case VOP_START_CODE:
		case GOV_START_CODE:
			go = 0;
			break;
		/*EOS*/
		case -1:
			go = 0;
			m4v->current_object_start = (u32) BS_GetPosition(m4v->bs);
			break;
		//don't interest us
		case UDTA_START_CODE:
		default:
			break;
		}
	}
	return M4OK;
}

M4Err M4V_Reset(M4V_Parser *m4v, u32 start)
{
	BS_Seek(m4v->bs, start);
	m4v->current_object_start = start;
	m4v->current_object_type = 0;
	return M4OK;
}

M4Err M4V_GetFrame(M4V_Parser *m4v, M4VDecoderSpecificInfo dsi, u8 *frame_type, u32 *time_inc, u32 *size, u32 *start, Bool *is_coded)
{
	u8 go, hasVOP, firstObj, secs;
	s32 o_type;
	u32 vop_inc = 0;

	if (!m4v || !size || !start || !frame_type) return M4BadParam;

	*size = 0;
	firstObj = 1;
	hasVOP = 0;
	*is_coded = 0;
	m4v->current_object_type = -1;
	
	M4V_Reset(m4v, m4v->current_object_start);
	go = 1;
	while (go) {
		o_type = M4V_LoadObject(m4v);
		switch (o_type) {
		case VOP_START_CODE:
			//done
			if (hasVOP) {
				go = 0;
				break;
			}
			if (firstObj) {
				*start = m4v->current_object_start;
				firstObj = 0;
			}
			hasVOP = 1;

			/*coding type*/
			*frame_type = BS_ReadInt(m4v->bs, 2);
			/*modulo time base*/
			secs = 0;
			while (BS_ReadInt(m4v->bs, 1) != 0)
				secs ++;
			/*no support for B frames in parsing*/
			secs += (dsi.enh_layer || *frame_type!=2) ? m4v->tc_dec : m4v->tc_disp;
			/*marker*/
			BS_ReadInt(m4v->bs, 1);
			/*vop_time_inc*/
			if (dsi.NumBitsTimeIncrement)
				vop_inc = BS_ReadInt(m4v->bs, dsi.NumBitsTimeIncrement);
			
			m4v->prev_tc_dec = m4v->tc_dec;
			m4v->prev_tc_disp = m4v->tc_disp;
			if (dsi.enh_layer || *frame_type!=2) {
				m4v->tc_disp = m4v->tc_dec;
				m4v->tc_dec = secs;
			}
			*time_inc = secs * dsi.clock_rate + vop_inc;
			/*marker*/
			BS_ReadInt(m4v->bs, 1);
			/*coded*/
			*is_coded = BS_ReadInt(m4v->bs, 1);
			BS_Align(m4v->bs);
			break;
		case GOV_START_CODE:
			if (firstObj) {
				*start = m4v->current_object_start;
				firstObj = 0;
			}
			if (hasVOP) go = 0;
			break;

		case VO_START_CODE:
		case VOL_START_CODE:
		default:
			break;

		case -1:
			*size = (u32) BS_GetPosition(m4v->bs) - *start;
			return M4EOF;
		}
	}
	*size = m4v->current_object_start - *start;
	return M4OK;
}


u32 M4V_GetObjectStartPos(LPM4VPARSER m4v)
{
	return m4v->current_object_start;
}

Bool M4V_IsValidObjectType(LPM4VPARSER m4v)
{
	return ((s32) m4v->current_object_type==-1) ? 0 : 1;
}


M4Err M4V_GetConfig(char *rawdsi, u32 rawdsi_size, M4VDecoderSpecificInfo *dsi)
{
	M4Err e;
	LPM4VPARSER vparse;
	if (!rawdsi || !rawdsi_size) return M4NonCompliantBitStream;
	vparse = NewMPEG4VideoParser(rawdsi, rawdsi_size);
	e = M4V_ParseConfig(vparse, dsi);
	DeleteMPEG4VideoParser(vparse);
	return e;
}

static const char *M4A_ObjectTypesNames[] = {
    "Reserved",
    "AAC Main",
    "AAC LC",
    "AAC SBR",
    "AAC LTP",
    "SBR",
    "AAC Scalable",
    "TwinVQ",
    "CELP", 
    "HVXC",
    "Reserved", 
    "Reserved"
    "TTSI",
    "Main synthetic",
    "Wavetable synthesis",
    "General MIDI",
    "Algorithmic Synthesis and Audio FX",
    "ER AAC LC",
    "Reserved",
    "ER AAC LTP",
    "ER AAC scalable",
    "ER TwinVQ",
    "ER BSAC",
    "ER AAC LD",
    "ER CELP",
    "ER HVXC",
    "ER HILN",
    "ER Parametric",
    "(Reserved)",
    "(Reserved)",
    "(Reserved)",
    "(Reserved)"
};

const char *M4A_GetObjectTypeName(u32 objectType)
{
  if (objectType>=32) return NULL;
  return M4A_ObjectTypesNames[objectType];
}

const char *M4A_AudioProfileName(u8 audio_pl)
{
	switch (audio_pl) {
	case 0x00: return "ISO Reserved (0x00)";
	case 0x01: return "Main Audio Profile @ Level 1";
	case 0x02: return "Main Audio Profile @ Level 2";
	case 0x03: return "Main Audio Profile @ Level 3";
	case 0x04: return "Main Audio Profile @ Level 4";
	case 0x05: return "Scalable Audio Profile @ Level 1";
	case 0x06: return "Scalable Audio Profile @ Level 2";
	case 0x07: return "Scalable Audio Profile @ Level 3";
	case 0x08: return "Scalable Audio Profile @ Level 4";
	case 0x09: return "Speech Audio Profile @ Level 1";
	case 0x0A: return "Speech Audio Profile @ Level 2";
	case 0x0B: return "Synthetic Audio Profile @ Level 1";
	case 0x0C: return "Synthetic Audio Profile @ Level 2";
	case 0x0D: return "Synthetic Audio Profile @ Level 3";
	case 0x0E: return "High Quality Audio Profile @ Level 1";
	case 0x0F: return "High Quality Audio Profile @ Level 2";
	case 0x10: return "High Quality Audio Profile @ Level 3";
	case 0x11: return "High Quality Audio Profile @ Level 4";
	case 0x12: return "High Quality Audio Profile @ Level 5";
	case 0x13: return "High Quality Audio Profile @ Level 6";
	case 0x14: return "High Quality Audio Profile @ Level 7";
	case 0x15: return "High Quality Audio Profile @ Level 8";
	case 0x16: return "Low Delay Audio Profile @ Level 1";
	case 0x17: return "Low Delay Audio Profile @ Level 2";
	case 0x18: return "Low Delay Audio Profile @ Level 3";
	case 0x19: return "Low Delay Audio Profile @ Level 4";
	case 0x1A: return "Low Delay Audio Profile @ Level 5";
	case 0x1B: return "Low Delay Audio Profile @ Level 6";
	case 0x1C: return "Low Delay Audio Profile @ Level 7";
	case 0x1D: return "Low Delay Audio Profile @ Level 8";
	case 0x1E: return "Natural Audio Profile @ Level 1";
	case 0x1F: return "Natural Audio Profile @ Level 2";
	case 0x20: return "Natural Audio Profile @ Level 3";
	case 0x21: return "Natural Audio Profile @ Level 4";
	case 0x22: return "Mobile Audio Internetworking Profile @ Level 1";
	case 0x23: return "Mobile Audio Internetworking Profile @ Level 2";
	case 0x24: return "Mobile Audio Internetworking Profile @ Level 3";
	case 0x25: return "Mobile Audio Internetworking Profile @ Level 4";
	case 0x26: return "Mobile Audio Internetworking Profile @ Level 5";
	case 0x27: return "Mobile Audio Internetworking Profile @ Level 6";
	case 0x28: return "AAC Profile @ Level 1";
	case 0x29: return "AAC Profile @ Level 2";
	case 0x2A: return "AAC Profile @ Level 4";
	case 0x2B: return "AAC Profile @ Level 5";
	case 0x2C: return "High Efficiency AAC Profile @ Level 2";
	case 0x2D: return "High Efficiency AAC Profile @ Level 3";
	case 0x2E: return "High Efficiency AAC Profile @ Level 4";
	case 0x2F: return "High Efficiency AAC Profile @ Level 5";
	case 0xFE: return "No audio profile specified";
	case 0xFF: return "No audio capability required";
	default: return "ISO Reserved / User Private";
	}
}

M4Err M4A_GetConfig(char *dsi, u32 dsi_size, M4ADecoderSpecificInfo *cfg)
{
	BitStream *bs;
	memset(cfg, 0, sizeof(M4ADecoderSpecificInfo));
	if (!dsi || !dsi_size || (dsi_size<2) ) return M4NonCompliantBitStream;
	bs = NewBitStream(dsi, dsi_size, BS_READ);

	cfg->base_object_type = BS_ReadInt(bs, 5);
	cfg->base_sr_index = BS_ReadInt(bs, 4);
	if (cfg->base_sr_index == 0x0F) {
		cfg->base_sr = BS_ReadInt(bs, 24);
	} else {
		cfg->base_sr = m4a_sample_rates[cfg->base_sr_index];
	}
	cfg->nb_chan = BS_ReadInt(bs, 4);
	if (cfg->base_object_type==5) {
		cfg->has_sbr = 1;
		cfg->sbr_sr_index = BS_ReadInt(bs, 4);
		if (cfg->sbr_sr_index == 0x0F) {
			cfg->sbr_sr = BS_ReadInt(bs, 24);
		} else {
			cfg->sbr_sr = m4a_sample_rates[cfg->sbr_sr_index];
		}
		cfg->sbr_object_type = BS_ReadInt(bs, 5);
	}

	BS_Align(bs);
	if (BS_Available(bs)>=2) {
		u32 sync = BS_ReadInt(bs, 11);
		if (sync==0x2b7) {
			cfg->sbr_object_type = BS_ReadInt(bs, 5);
			cfg->has_sbr = BS_ReadInt(bs, 1);
			if (cfg->has_sbr) {
				cfg->sbr_sr_index = BS_ReadInt(bs, 4);
				if (cfg->sbr_sr_index == 0x0F) {
					cfg->sbr_sr = BS_ReadInt(bs, 24);
				} else {
					cfg->sbr_sr = m4a_sample_rates[cfg->sbr_sr_index];
				}
			}
		}
	}
	switch (cfg->base_object_type) {
	/*AAC profile @ L5, or AAC-HE @L5*/
	case 2:
	case 5:
		cfg->audioPL = cfg->has_sbr ? 0x2F : 0x2B;
		break;
	/*defaults to MAIN @ L4*/
	default:
		cfg->audioPL = 0x04;
		break;
	}

	DeleteBitStream(bs);
	return M4OK;
}


/*MP3 parsing - credits go to CISCO - MPEG4IP*/
u8 MP3_GetVersion(u32 hdr)
{
	return ((hdr >> 19) & 0x3); 
}

u8 MP3_GetLayer(u32 hdr)
{
	return ((hdr >> 17) & 0x3); 
}

u8 MP3_GetNumChannels(u32 hdr)
{
	if (((hdr >> 6) & 0x3) == 3) return 1;
	return 2;
}


static u16 MP3SamplingRates[4][3] = {
	{ 11025, 12000, 8000 },		/* MPEG-2.5 */
	{ 0, 0, 0 },
	{ 22050, 24000, 16000 },	/* MPEG-2 */
	{ 44100, 48000, 32000 }		/* MPEG-1 */
};

u16 MP3_GetSamplingRate(u32 hdr)
{
	/* extract the necessary fields from the MP3 header */
	u8 version = MP3_GetVersion(hdr);
	u8 sampleRateIndex = (hdr >> 10) & 0x3;
	return MP3SamplingRates[version][sampleRateIndex];
}

u16 MP3_GetSamplesPerFrame(u32 hdr)
{
	u8 version = MP3_GetVersion(hdr);
	u8 layer = MP3_GetLayer(hdr);

	if (layer == 1) {
		if (version == 3) return 1152;
		return 576;
	}
	if (layer == 2) return 1152;
	return 384;
}

u8 MP3_GetObjectTypeIndication(u32 hdr)
{
	switch (MP3_GetVersion(hdr)) {
	case 3:
		return 0x6B;
		break;
	case 2:
	case 0:
		return 0x69;
	default:
		return 0x00;
	}
}
static const char *MP3_Versions[] = 
{
	"MPEG-2.5",
	"Reserved",
	"MPEG-2",
	"MPEG-1"
};

const char *MP3_GetVersionName(u32 hdr)
{
  u32 v = MP3_GetVersion(hdr);
  if (v>3) return NULL;
  return MP3_Versions[v];
}

static u16 MP3BitRates[5][14] = {
	/* MPEG-1, Layer III */
	{ 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 },
	/* MPEG-1, Layer II */
	{ 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384 },
	/* MPEG-1, Layer I */
	{ 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448 },
	/* MPEG-2 or 2.5, Layer II or III */
	{ 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160 },
	/* MPEG-2 or 2.5, Layer I */
	{ 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256 }
};


u16 MP3_GetFrameSize(u32 hdr)
{
	u32 val;
	u8 bitRateIndex1;
	u8 version = MP3_GetVersion(hdr);
	u8 layer = MP3_GetLayer(hdr);
	u8 bitRateIndex2 = (hdr >> 12) & 0xF;
	u8 sampleRateIndex = (hdr >> 10) & 0x3;
	Bool isPadded = (hdr >> 9) & 0x1;
	u16 frameSize = 0;

	if (version == 3) {
		bitRateIndex1 = layer - 1;
	} else {
		if (layer == 3) {
			bitRateIndex1 = 4;
		} else {
			bitRateIndex1 = 3;
		}
	}

	/* compute frame size */
	val = (MP3SamplingRates[version][sampleRateIndex] << !(version & 1));
	if (!val) return 0;
	frameSize = 144 * 1000 * MP3BitRates[bitRateIndex1][bitRateIndex2-1] / val;

	if (isPadded) {
		if (layer == 3) {
			frameSize += 4;
		} else {
			frameSize++;
		}
	}
	return frameSize;
}


u16 MP3_GetBitRate(u32 hdr)
{
	u8 bitRateIndex1;
	u8 version = MP3_GetVersion(hdr);
	u8 layer = MP3_GetLayer(hdr);
	u8 bitRateIndex2 = (hdr >> 12) & 0xF;
	if (version == 3) {
		bitRateIndex1 = layer - 1;
	} else {
		if (layer == 3) {
			bitRateIndex1 = 4;
		} else {
			bitRateIndex1 = 3;
		}
	}

	/* compute frame size */
	return MP3BitRates[bitRateIndex1][bitRateIndex2-1];
}



u32 MP3_GetNextHeader(FILE* in)
{
	u8 b, state = 0;
	u32 dropped = 0;
	u8 bytes[4];

	while (1) {
		if (fread(&b, 1, 1, in) == 0) return 0;

		if (state==3) {
			bytes[state] = b;
			return FOUR_CHAR_INT(bytes[0], bytes[1], bytes[2], bytes[3]);
		}
		if (state==2) {
			if (((b & 0xF0) == 0) || ((b & 0xF0) == 0xF0) || ((b & 0x0C) == 0x0C)) {
				if (bytes[1] == 0xFF) state = 1; 
				else state = 0; 
			} else {
				bytes[state] = b;
				state = 3;
			}
		}
		if (state==1) {
			if (((b & 0xE0) == 0xE0) && ((b & 0x18) != 0x08) && ((b & 0x06) != 0)) {
				bytes[state] = b;
				state = 2;
			} else {
				state = 0;
			}
		}

		if (state==0) {
			if (b == 0xFF) {
				bytes[state] = b;
				state = 1;
			} else {
				if ((dropped == 0) && ((b & 0xE0) == 0xE0) && ((b & 0x18) != 0x08) && ((b & 0x06) != 0)) {
					bytes[0] = (u8) 0xFF;
					bytes[1] = b;
					state = 2;
				} else {
					dropped++;
				}
			}
		}
	}
	return 0;
}

u32 MP3_GetNextHeaderMem(char *buffer, u32 size, u32 *pos)
{
	u32 cur;
	u8 b, state = 0;
	u32 dropped = 0;
	u8 bytes[4];

	cur = 0;
	*pos = 0;
	while (cur<size) {
		b = (u8) buffer[cur];
		cur++;

		if (state==3) {
			bytes[state] = b; 
			*pos = dropped;
			return FOUR_CHAR_INT(bytes[0], bytes[1], bytes[2], bytes[3]);;
		}
		if (state==2) {
			if (((b & 0xF0) == 0) || ((b & 0xF0) == 0xF0) || ((b & 0x0C) == 0x0C)) {
				if (bytes[1] == 0xFF) state = 1; 
				else state = 0; 
			} else {
				bytes[state] = b;
				state = 3;
			}
		}
		if (state==1) {
			if (((b & 0xE0) == 0xE0) && ((b & 0x18) != 0x08) && ((b & 0x06) != 0)) {
				bytes[state] = b;
				state = 2;
			} else {
				state = 0;
			}
		}

		if (state==0) {
			if (b == 0xFF) {
				bytes[state] = b;
				state = 1;
			} else {
				if ((dropped == 0) && ((b & 0xE0) == 0xE0) && ((b & 0x18) != 0x08) && ((b & 0x06) != 0)) {
					bytes[0] = (u8) 0xFF;
					bytes[1] = b;
					state = 2;
				} else {
					dropped++;
				}
			}
		}
	}
	return 0;
}


/*
	taken from MPEG4IP - MPL
*/
static u32 AC3_FindSyncCode(u8 *buf, u32 buflen)
{
	u32 end = buflen - 6;
	u32 offset = 0;
	while (offset <= end) {
		if (buf[offset] == 0x0b && buf[offset + 1] == 0x77) {
			return offset;
		}
		offset++;
	}
	return buflen;
}

static const u32 ac3_sizecod_to_bitrate[] = {
  32000, 40000, 48000, 56000, 64000, 80000, 96000,
  112000, 128000, 160000, 192000, 224000, 256000,
  320000, 384000, 448000, 512000, 576000, 640000
};

static const u32 ac3_sizecod2_to_framesize[] = {
  96, 120, 144, 168, 192, 240, 288, 336, 384, 480, 576, 672,
  768, 960, 1152, 1344, 1536, 1728, 1920
};

static const u32 ac3_sizecod1_to_framesize[] = {
  69, 87, 104, 121, 139, 174, 208, 243, 278, 348, 417, 487,
  557, 696, 835, 975, 1114, 1253, 1393
};
static const u32 ac3_sizecod0_to_framesize[] = {
  64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 
  512, 640, 768, 896, 1024, 1152, 1280
};

static const u32 ac3_mod_to_chans[] = {
  2, 1, 2, 3, 3, 4, 4, 5
};

Bool AC3_ParseHeader(u8 *buf, u32 buflen, u32 *pos, AC3Header *hdr)
{
	u32 fscod, frmsizecod, bsid, ac3_mod, freq, framesize;
	if (buflen < 6) return 0;
	(*pos) = AC3_FindSyncCode(buf, buflen);
	if (*pos >= buflen) return 0;

	buf += (*pos);
	fscod = (buf[4] >> 6) & 0x3;
	frmsizecod = (buf[4] & 0x3f);
	bsid = (buf[5] >> 3) & 0x1f;
	ac3_mod = (buf[6] >> 5) & 0x7;
	if (bsid >= 12) return 0;

	if (hdr) memset(hdr, 0, sizeof(AC3Header));
	
	if (hdr) {
		hdr->bitrate = ac3_sizecod_to_bitrate[frmsizecod / 2];
		if (bsid > 8) hdr->bitrate = hdr->bitrate >> (bsid - 8);
	}
	switch (fscod) {
	case 0:
		freq = 48000;
		framesize = ac3_sizecod0_to_framesize[frmsizecod / 2] * 2;
		break;
	case 1:
		freq = 44100;
		framesize = (ac3_sizecod1_to_framesize[frmsizecod / 2] + (frmsizecod & 0x1)) * 2;
		break;
	case 2:
		freq = 32000;
		framesize = ac3_sizecod2_to_framesize[frmsizecod / 2] * 2;
		break;
	default:
	    return 0;
	}
	if (hdr) {
		u16 maskbit, b67;
		hdr->sample_rate = freq;
		hdr->framesize = framesize;

		hdr->channels = ac3_mod_to_chans[ac3_mod];
		maskbit = 0x100;
		if ((ac3_mod & 0x1) && (ac3_mod != 1)) maskbit >>= 2;
		if (ac3_mod & 0x4) maskbit >>= 2;
		if (ac3_mod == 0x2) maskbit += 2;
		b67 = (buf[6] << 8) | buf[7];
		if ((b67 & maskbit) != 0) hdr->channels += 1;
	}
	return 1;
}

#ifndef M4_READ_ONLY

#include <gpac/m4_ogg.h>

static u32 vorbis_book_maptype1_quantvals(u32 entries, u32 dim)
{
	u32 vals = (u32) floor(pow((Float)entries, 1.0f/dim));
	while(1) {
		u32 acc=1;
		u32 acc1=1;
		u32 i;
		for (i=0;i<dim;i++) {
			acc*=vals;
			acc1*=vals+1;
		}
		if(acc<=entries && acc1>entries) return (vals);
		else { 
			if (acc>entries) vals--;
			else vals++;
		}
	}
}

u32 _ilog_(u32 v)
{
	u32 ret=0;
	while(v) {
		ret++;
		v>>=1;
	}
	return(ret);
}

static u32 ilog(u32 v)
{
	u32 ret=0; 
	if(v) --v;
	while(v) {
		ret++;
		v>>=1;
	}
	return (ret);
}

static u32 icount(u32 v)
{
	u32 ret=0;
	while(v) {
		ret += v&1;
		v>>=1;
	}
	return(ret);
}


Bool VorbisParseHeader(VorbisParser *vp, char *data, u32 data_len)
{
	u32 pack_type, i, j, k, times, nb_part, nb_books, nb_modes;
	char szNAME[8];
	oggpack_buffer opb;

	oggpack_readinit(&opb, data, data_len);
	pack_type = oggpack_read(&opb, 8);
	i=0;
	while (i<6) { szNAME[i] = oggpack_read(&opb, 8); i++;}
	szNAME[i] = 0;
	if (strcmp(szNAME, "vorbis")) return vp->is_init = 0;

	switch (pack_type) {
	case 0x01:
		vp->version = oggpack_read(&opb, 32);
		if (vp->version!=0) return 0;
		vp->channels = oggpack_read(&opb, 8);
		vp->sample_rate = oggpack_read(&opb, 32);
		vp->max_r = oggpack_read(&opb, 32);
		vp->avg_r = oggpack_read(&opb, 32);
		vp->low_r = oggpack_read(&opb, 32);

		vp->min_block = 1<<oggpack_read(&opb, 4);
		vp->max_block = 1<<oggpack_read(&opb, 4);
		if (vp->sample_rate < 1) return vp->is_init = 0;
		if (vp->channels < 1) return vp->is_init = 0;
		if (vp->min_block<8) return vp->is_init = 0;
		if (vp->max_block < vp->min_block) return vp->is_init = 0;
		if (oggpack_read(&opb, 1) != 1) return vp->is_init = 0;
		vp->is_init = 1;
		return 1;
	case 0x03:
		/*trash comments*/
		vp->is_init ++;
		return 1;
	case 0x05:
		/*need at least bitstream header to make sure we're parsing the right thing*/
		if (!vp->is_init) return 0;
		break;
	default:
		vp->is_init = 0;
		return 0;
	}
	/*OK parse codebook*/
	nb_books = oggpack_read(&opb, 8) + 1;
	/*skip vorbis static books*/
	for (i=0; i<nb_books; i++) {
		u32 j, map_type, qb, qq;
		u32 entries, dim;
		oggpack_read(&opb, 24);
		dim = oggpack_read(&opb, 16);
		entries = oggpack_read(&opb, 24);
		if ( (s32) entries < 0) entries = 0;
		if (oggpack_read(&opb, 1) == 0) {
			if (oggpack_read(&opb, 1)) {
				for (j=0; j<entries; j++) {
					if (oggpack_read(&opb, 1)) {
						oggpack_read(&opb, 5);
					}
				}
			} else {
				for (j=0; j<entries; j++) 
					oggpack_read(&opb, 5);
			}
		} else {
			oggpack_read(&opb, 5);
			for (j=0; j<entries;) {
				u32 num = oggpack_read(&opb, _ilog_(entries-j));
				for (k=0; k<num && j<entries; k++, j++) {
				}
			}
		}
		switch ((map_type=oggpack_read(&opb, 4))) {
		case 0: break;
		case 1:
		case 2:
			oggpack_read(&opb, 32);
			oggpack_read(&opb, 32);
			qq = oggpack_read(&opb, 4)+1;
			oggpack_read(&opb, 1);
			if (map_type==1) qb = vorbis_book_maptype1_quantvals(entries, dim);
			else if (map_type==2) qb = entries * dim;
			else qb = 0;
			for (j=0;j<qb;j++) oggpack_read(&opb, qq);
			break;
		}
	}
	times = oggpack_read(&opb, 6)+1;
	for (i=0;i<times;i++) oggpack_read(&opb, 16);
	times = oggpack_read(&opb, 6)+1;
	for (i=0;i<times;i++) {
		u32 type = oggpack_read(&opb, 16);
		if (type) {
			u32 *parts, *class_dims, count, rangebits;
			u32 max_class = 0;
			nb_part = oggpack_read(&opb, 5);
			parts = malloc(sizeof(u32) * nb_part);
			for (j=0;j<nb_part;j++) {
				parts[j] = oggpack_read(&opb, 4);
				if (max_class<parts[j]) max_class = parts[j];
			}
			class_dims = malloc(sizeof(u32) * (max_class+1));
			for (j=0; j<max_class+1;j++) {
				u32 class_sub;
				class_dims[j] = oggpack_read(&opb, 3) + 1;
				class_sub = oggpack_read(&opb, 2);
				if (class_sub) oggpack_read(&opb, 8);
				for (k=0; k < (u32) (1<<class_sub); k++) oggpack_read(&opb, 8);
			}
			oggpack_read(&opb, 2);
			rangebits=oggpack_read(&opb, 4);
			count = 0;
			for (j=0,k=0;j<nb_part;j++) {
				count+=class_dims[parts[j]];
				for (;k<count;k++) oggpack_read(&opb, rangebits);
			}
			free(parts);
			free(class_dims);
		} else {
			u32 j, nb_books;
			oggpack_read(&opb, 8+16+16+6+8);
			nb_books = oggpack_read(&opb, 4)+1;
			for (j=0; j<nb_books; j++) oggpack_read(&opb, 8);
		}
	}
	times = oggpack_read(&opb, 6)+1;
	for (i=0;i<times;i++) {
		u32 acc = 0;
		oggpack_read(&opb, 16);/*type*/
		oggpack_read(&opb, 24);
		oggpack_read(&opb,24);
		oggpack_read(&opb,24);
		nb_part = oggpack_read(&opb, 6)+1;
		oggpack_read(&opb, 8);
		for (j=0; j<nb_part;j++) {
			u32 cascade = oggpack_read(&opb, 3);
			if (oggpack_read(&opb, 1)) cascade |= (oggpack_read(&opb, 5)<<3);
			acc += icount(cascade);
		}
		for (j=0;j<acc;j++) oggpack_read(&opb, 8);
	}
	times = oggpack_read(&opb, 6)+1;
	for (i=0; i<times; i++) {
		u32 sub_maps = 1;
		oggpack_read(&opb, 16);
		if (oggpack_read(&opb, 1)) sub_maps = oggpack_read(&opb, 4)+1;
		if (oggpack_read(&opb, 1)) {
			u32 nb_steps = oggpack_read(&opb, 8)+1;
			for (j=0;j<nb_steps;j++) {
				oggpack_read(&opb, ilog(vp->channels));
				oggpack_read(&opb, ilog(vp->channels));
			}
		}
		oggpack_read(&opb, 2);
		if (sub_maps>1) {
			for(j=0; j<vp->channels; j++) oggpack_read(&opb, 4);
		}
		for (j=0;j<sub_maps;j++) {
			oggpack_read(&opb, 8);
			oggpack_read(&opb, 8);
			oggpack_read(&opb, 8);
		}
	}
	nb_modes = oggpack_read(&opb, 6)+1;
	for (i=0; i<nb_modes; i++) {
		vp->mode_flag[i] = oggpack_read(&opb, 1);
		oggpack_read(&opb, 16);
		oggpack_read(&opb, 16);
		oggpack_read(&opb, 8);
	}

	vp->modebits = 0;
	j = nb_modes;
	while(j>1) {
		vp->modebits++;
		j>>=1;
	}
	return 1;
}

u32 VorbisCheckFrame(VorbisParser *vp, char *data, u32 data_length)
{
	s32 block_size;
	oggpack_buffer opb;
	if (!vp->is_init) return 0;
	oggpack_readinit(&opb, data, data_length);
	/*not audio*/
	if (oggpack_read(&opb, 1) !=0) return 0;
	block_size = oggpack_read(&opb, vp->modebits);
	if (block_size == -1) return 0;
	return ((vp->mode_flag[block_size]) ? vp->max_block : vp->min_block) / (2);
}

#endif

