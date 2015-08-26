/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / codec pack plugin
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


#include <gpac/m4_decoder.h>
#include <gpac/m4_author.h>


#include "decore.h"
/*beerk*/
static Bool DIVX_is_init = 0;

typedef struct
{
	void *codec;
	/*no support for scalability in DIVX yet*/
	u16 ES_ID;
	DEC_FRAME df;
	u32 width, height, out_size;
	u32 cb_size, cb_trig;

	s32 base_filters;
} DIVXDec;

#define DIVXCTX()	DIVXDec *ctx = (DIVXDec *) ifcg->privateStack
#define SWAPP_BYTES(a) ((a>>24)|((a&0x00FF0000)>>8)|((a&0x0000FF00)<<8)|(a<<24))

static M4Err DIVX_AttachStream(BaseDecoder *ifcg, u16 ES_ID, unsigned char *decSpecInfo, u32 decSpecInfoSize, u16 DependsOnES_ID, u32 objectTypeIndication, Bool UpStream)
{
	M4VDecoderSpecificInfo dsi;
	M4Err e;
	DIVXCTX();

	if (ctx->ES_ID && ctx->ES_ID!=ES_ID) return M4NotSupported;
	if (!decSpecInfoSize || !decSpecInfo) return M4NonCompliantBitStream;
//	if (ctx->codec) DIVX_decore(ctx->codec, DIVX_DEC_DESTROY, NULL, NULL);
	if (ctx->codec) decore(ctx->codec, DEC_OPT_RELEASE, NULL, NULL);

	/*decode DSI*/
	e = M4V_GetConfig(decSpecInfo, decSpecInfoSize, &dsi);
	if (e) return e;
	if (!dsi.width || !dsi.height) return M4NonCompliantBitStream;

	if (!DIVX_is_init)
	{
		DivXBitmapInfoHeader bi;
		DEC_VOL_INFO dv;


		DEC_INIT di;
        di.alloc = NULL;
		di.free = NULL;
		di.flag_logo = 0;
		if (decore(&(ctx->codec), DEC_OPT_INIT, &di, 0) != DEC_OK)
		{
			printf("Decore >DEC_OPT_INIT< failed\n");
		}
		ctx->df.render_flag = 1;
		ctx->df.skip_decoding = 0;

		ctx->df.length = decSpecInfoSize;
		ctx->df.bitstream = (void *) decSpecInfo; 
        
		// Always call DEC_OPT_INIT_VOL to ensure use of values from m4v bitstream.
		if (decore (ctx->codec, DEC_OPT_INIT_VOL, &ctx->df, &dv) != 	DEC_OK)
		{
			printf(">Decore >DEC_OPT_INIT_VOL< failed\n");
		}
		bi.biSize = sizeof (bi);
		bi.biWidth = dsi.width;
		bi.biHeight = dsi.height;
//		bi.biCompression = SWAPP_BYTES(M4PF_YV12);/*mmioFOURCC('I', 'Y', 'U', 'V');*/
		bi.biCompression = mmioFOURCC('I', '4', '2', '0');
        /*frame.colorspace = DIVX_CSP_I420;*/

		if (decore (ctx->codec, DEC_OPT_SETOUT, &bi, 0) != DEC_OK)
		{
			printf ("Decore >DEC_OPT_SETOUT< failed \n");
		}
		ctx->df.stride = dsi.width;
		DIVX_is_init = 1;
	}

    ctx->df.bmp = NULL; 
    ctx->df.render_flag = 0;
    ctx->df.skip_decoding = 0;

//	if (decore (ctx->codec, DEC_OPT_FRAME, &ctx->df, 0) != DEC_OK) {
//		return M4NonCompliantBitStream;
//	}


	ctx->width = dsi.width;
	ctx->height =  dsi.height;

	ctx->ES_ID = ES_ID;	
	/*output in YV12 only - let the player handle conversion*/
	ctx->out_size = ctx->width * ctx->height * 3 / 2;
	return M4OK;
}
static M4Err DIVX_DetachStream(BaseDecoder *ifcg, u16 ES_ID)
{
	DIVXCTX();
	if (ctx->ES_ID != ES_ID) return M4BadParam;
	if (ctx->codec)
	{
		if (decore (ctx->codec, DEC_OPT_RELEASE, 0, 0) != DEC_OK)
		{
			printf ("Decore >DEC_OPT_RELEASE< returned \n");
		}
	}
	DIVX_is_init = 0;
	ctx->codec = NULL;
	ctx->ES_ID = 0;
	ctx->width = ctx->height = ctx->out_size = 0;
	return M4OK;
}
static M4Err DIVX_GetCapabilities(BaseDecoder *ifcg, CapObject *capability)
{
	DIVXCTX();

	switch (capability->CapCode) {
	case CAP_WIDTH:
		capability->cap.valueINT = ctx->width;
		break;
	case CAP_HEIGHT:
		capability->cap.valueINT = ctx->height;
		break;
	case CAP_STRIDE:
		capability->cap.valueINT = ctx->width;
		break;
	case CAP_OUTPUTSIZE:
		capability->cap.valueINT = ctx->out_size;
		break;
	case CAP_COLORMODE:
		capability->cap.valueINT = M4PF_YV12;
		break;
	/*no postprocessing API for xvid*/
	case CAP_VID_POSTPROC:
		capability->cap.valueINT = 0;
		break;
	case CAP_BUFFER_MIN:
		capability->cap.valueINT = 1;
		break;
	case CAP_BUFFER_MAX:
		capability->cap.valueINT = 4;
		break;
	/*no support for shape coding*/
	case CAP_VID_SHAPE:
		capability->cap.valueINT = 0;
		break;
	/*by default we use 4 bytes padding (otherwise it happens that XviD crashes on some videos...)*/
	case CAP_PADDING_BYTES:
		capability->cap.valueINT = 32;
		break;
	/*XviD performs frame reordering internally*/
	case CAP_VID_REORDER:
		capability->cap.valueINT = 1;
		break;
	case CAP_CODEC_WANTSTHREAD:
	{
		const char *sOpt = PMI_GetOpt(ifcg, "XviD", "Threaded");
		capability->cap.valueINT = (sOpt && stricmp(sOpt, "yes")) ? 1 : 0;
	}
		break;
	/*not known at our level...*/
	case CAP_CU_DURATION:
	default:
		capability->cap.valueINT = 0;
		break;
	}
	return M4OK;
}
static M4Err DIVX_SetCapabilities(BaseDecoder *ifcg, CapObject capability)
{
	/*return unsupported to avoid confusion by the player (like color space changing ...) */
	return M4NotSupported;
}
static M4Err DIVX_ProcessData(MediaDecoder *ifcg, 
		unsigned char *inBuffer, u32 inBufferLength,
		u16 ES_ID,
		unsigned char *outBuffer, u32 *outBufferLength,
		u8 PaddingBits, u32 mmlevel)
{
	DIVXCTX();

	/*check not using scalabilty*/
	if (ES_ID != ctx->ES_ID) return M4BadParam;

	if (*outBufferLength < ctx->out_size) {
		*outBufferLength = ctx->out_size;
		return M4BufferTooSmall;
	}

	ctx->df.bitstream = (void *) inBuffer;
	ctx->df.length = inBufferLength;
    ctx->df.bmp = (void *) outBuffer; 
    ctx->df.render_flag = 1;
    ctx->df.skip_decoding = 0;
    ctx->df.stride = ctx->width;

	if (decore (ctx->codec, DEC_OPT_FRAME, &ctx->df, 0) != DEC_OK) {
		*outBufferLength = 0;
		return M4NonCompliantBitStream;
	}

	/*dispatch nothing if seeking or droping*/
	switch (mmlevel) {
	case MM_LEVEL_SEEK:
	case MM_LEVEL_DROP:
		*outBufferLength = 0;
		break;
	default:
		*outBufferLength = ctx->out_size;
		break;
	}
	return M4OK;
}

static Bool DIVX_CanHandleStream(BaseDecoder *dec, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL)
{
	if (StreamType != M4ST_VISUAL) return 0;
	switch (ObjectType) {
	case 0x20:
		return 1;
	/*cap query*/
	case 0:
		return 1;
	}
	return 0;
}

static const char *DIVX_GetCodecName(BaseDecoder *dec)
{
#ifdef DIVX_USE_OLD_API
	return "DIVX Dev Version";
#else
	return "DIVX 1.0";
#endif
} 

BaseDecoder *NewDIVXDec()
{
	MediaDecoder *ifcd;
	DIVXDec *dec;
	
	SAFEALLOC(ifcd, sizeof(MediaDecoder));
	SAFEALLOC(dec, sizeof(DIVXDec));
	M4_REG_PLUG(ifcd, M4MEDIADECODERINTERFACE, "DIVX Decoder", "gpac distribution", 0)

	ifcd->privateStack = dec;

	/*get config*/
	dec->base_filters = 0;

	/*setup our own interface*/	
	ifcd->AttachStream = DIVX_AttachStream;
	ifcd->DetachStream = DIVX_DetachStream;
	ifcd->GetCapabilities = DIVX_GetCapabilities;
	ifcd->SetCapabilities = DIVX_SetCapabilities;
	ifcd->GetName = DIVX_GetCodecName;
	ifcd->CanHandleStream = DIVX_CanHandleStream;
	ifcd->ProcessData = DIVX_ProcessData;
	return (BaseDecoder *) ifcd;
}

void DeleteXVIDDec(BaseDecoder *ifcg)
{
	DIVXCTX();
	if (ctx->codec)
	{
		if (decore (ctx->codec, DEC_OPT_RELEASE, 0, 0) != DEC_OK)
		{
			printf ("Decore >DEC_OPT_RELEASE< returned \n");
		}
	}
	DIVX_is_init = 0;
	free(ctx);
	free(ifcg);
}


Bool QueryInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4MEDIADECODERINTERFACE) return 1;
	return 0;
}

void *LoadInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4MEDIADECODERINTERFACE) return NewDIVXDec();
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	BaseDecoder *ptr = (BaseDecoder*)ifce;
	switch (ptr->InterfaceType) {
	case M4MEDIADECODERINTERFACE: 
		DeleteXVIDDec(ptr);
		break;
	}
}
