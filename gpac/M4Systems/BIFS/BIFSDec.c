/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / BIFS codec sub-project
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


#include <gpac/intern/m4_bifs_dev.h>

M4Err ParseConfig(BitStream *bs, BIFSStreamInfo *info, u32 version)
{
	Bool hasSize;

	if (version==2) {
		info->config.Use3DMeshCoding = BS_ReadInt(bs, 1);
		info->config.UsePredictiveMFField = BS_ReadInt(bs, 1);
	}
	info->config.NodeIDBits = BS_ReadInt(bs, 5);
	info->config.RouteIDBits = BS_ReadInt(bs, 5);
	if (version==2) {
		info->config.ProtoIDBits = BS_ReadInt(bs, 5);
	}
	info->config.IsCommandStream = BS_ReadInt(bs, 1);

	if (info->config.IsCommandStream) {
		info->config.PixelMetrics = BS_ReadInt(bs, 1);
		hasSize = BS_ReadInt(bs, 1);
		if (hasSize) {
			info->config.Width = BS_ReadInt(bs, 16);
			info->config.Height = BS_ReadInt(bs, 16);
		}
		BS_Align(bs);

		if (BS_GetSize(bs) != BS_GetPosition(bs)) return M4ReadDescriptorFailed;
		return M4OK;
	} else {
		return M4NotSupported;
	}
}


LPBIFSDEC BIFS_NewDecoder(LPSCENEGRAPH scenegraph, Bool command_dec)
{
	BifsDecoder * tmp = malloc(sizeof(BifsDecoder));
	memset(tmp, 0, sizeof(BifsDecoder));

	tmp->QPs = NewChain();
	tmp->streamInfo = NewChain();
	tmp->info = NULL;	

	tmp->pCurrentProto = NULL;
	tmp->scenegraph = scenegraph;
	if (command_dec) {
		tmp->dec_memory_mode = 1;
		tmp->force_keep_qp = 1;
		tmp->conditionals = NewChain();
	}
	tmp->current_graph = NULL;
	tmp->mx = NewMutex();
	return tmp;
}


BIFSStreamInfo *BD_GetStream(BifsDecoder * codec, u16 ESID)
{
	u32 i;
	BIFSStreamInfo *ptr;

	for (i=0;i<ChainGetCount(codec->streamInfo);i++) {
		ptr = (BIFSStreamInfo *) ChainGetEntry(codec->streamInfo, i);
		if(ptr->ESID==ESID) return ptr;
	}
	return NULL;
}

M4Err BIFS_ConfigureStream(BifsDecoder * codec, u16 ESID, char *DecoderSpecificInfo, u32 DecoderSpecificInfoLength, u32 objectTypeIndication)
{
	BitStream *bs;
	BIFSStreamInfo *pInfo;
	M4Err e;
	
	if (!DecoderSpecificInfo) return M4BadParam;
	MX_P(codec->mx);
	if (BD_GetStream(codec, ESID) != NULL) {
		MX_V(codec->mx);
		return M4BadParam;
	}

	
	bs = NewBitStream(DecoderSpecificInfo, DecoderSpecificInfoLength, BS_READ);
	pInfo = malloc(sizeof(BIFSStreamInfo));
	memset(pInfo, 0, sizeof(BIFSStreamInfo));
	pInfo->ESID = ESID;

	pInfo->config.version = objectTypeIndication;
	/*parse config with indicated oti*/
	e = ParseConfig(bs, pInfo, (u32) objectTypeIndication);
	if (e) {
		memset(pInfo, 0, sizeof(BIFSStreamInfo));
		pInfo->ESID = ESID;
		/*some content indicates a wrong OTI, so try to parse with v1 or v2*/
		BS_Seek(bs, 0);
		/*try with reverse config*/
		e = ParseConfig(bs, pInfo, (objectTypeIndication==2) ? 1 : 2);
		pInfo->config.version = (objectTypeIndication==2) ? 1 : 2;
	}

	if (e && (e != M4ReadDescriptorFailed)) {
		free(pInfo);
		DeleteBitStream(bs);
		return M4UnknowBIFSVersion;
	}
	DeleteBitStream(bs);

	//first stream, configure size
	if (!codec->ignore_size && !ChainGetCount(codec->streamInfo)) {
		SG_SetSizeInfo(codec->scenegraph, pInfo->config.Width, pInfo->config.Height, pInfo->config.PixelMetrics);
	}

	ChainAddEntry(codec->streamInfo, pInfo);
	MX_V(codec->mx);
	return M4OK;
}

void BIFS_IgnoreSizeInfo(LPBIFSDEC codec)
{
	if (codec) codec->ignore_size = 1;
}

M4Err BIFS_GetConfig(LPBIFSDEC codec, u16 ESID, BIFSConfigDescriptor *cfg)
{
	BIFSStreamInfo *info;
	if (!codec || !ESID || !cfg) return M4BadParam;
	info = BD_GetStream(codec, ESID);
	if (!info) return M4BadParam;
	memset(cfg, 0, sizeof(BIFSConfigDescriptor));
	cfg->isCommandStream = info->config.IsCommandStream;
	cfg->nodeIDbits = info->config.NodeIDBits;
	cfg->pixelHeight = info->config.Height;
	cfg->pixelMetrics = info->config.PixelMetrics;
	cfg->pixelWidth = info->config.Width;
	cfg->protoIDbits = info->config.ProtoIDBits;
	cfg->routeIDbits = info->config.RouteIDBits;
	cfg->version = info->config.version;
	return M4OK;
}


M4Err BIFS_RemoveStream(LPBIFSDEC codec, u16 ESID)
{
	u32 i;
	BIFSStreamInfo *ptr;

	MX_P(codec->mx);
	for (i=0;i<ChainGetCount(codec->streamInfo);i++) {
		ptr = (BIFSStreamInfo *) ChainGetEntry(codec->streamInfo, i);
		if(ptr->ESID==ESID) {
			free(ptr);
			ChainDeleteEntry(codec->streamInfo, i);
			MX_V(codec->mx);
			return M4OK;
		}
	}
	MX_V(codec->mx);
	return M4BadParam;
}

void BIFS_DeleteDecoder(LPBIFSDEC codec)
{	
	if (codec->GlobalQP) Node_Unregister((SFNode *) codec->GlobalQP, NULL);
	
	assert(ChainGetCount(codec->QPs)==0);
	DeleteChain(codec->QPs);

	/*destroy all config*/
	while (ChainGetCount(codec->streamInfo)) {
		BIFSStreamInfo *p = ChainGetEntry(codec->streamInfo, 0);
		free(p);
		ChainDeleteEntry(codec->streamInfo, 0);
	}
	DeleteChain(codec->streamInfo);
	if (codec->dec_memory_mode) {
		assert(ChainGetCount(codec->conditionals) == 0);
		DeleteChain(codec->conditionals);
	}
	MX_Delete(codec->mx);
	free(codec);
}


void BD_EndOfStream(void *co)
{
	((BifsDecoder *) co)->LastError = M4IOErr;
}

void BS_SetEOSCallback(BitStream *bs, void (*EndOfStream)(void *par), void *par);

M4Err BIFS_DecodeAU(LPBIFSDEC codec, u16 ESID, char *data, u32 data_length)
{
	BitStream *bs;
	M4Err e;

	if (!codec || !data || codec->dec_memory_mode) return M4BadParam;

	MX_P(codec->mx);
	codec->info = BD_GetStream(codec, ESID);
	if (!codec->info) {
		MX_V(codec->mx);
		return M4BadParam;
	}
	/*setup current scene graph*/
	codec->current_graph = codec->scenegraph;

	bs = NewBitStream(data, data_length, BS_READ);
	BS_SetEOSCallback(bs, BD_EndOfStream, codec);

	if (codec->info->config.IsCommandStream) {
		e = BIFS_ParseCommand(codec, bs);
	} else {
		e = M4AnimNotSupported;
	}
	DeleteBitStream(bs);
	/*reset current config*/
	codec->info = NULL;
	codec->current_graph = NULL;

	MX_V(codec->mx);
	return e;
}
	

SFNode *BIFS_FindNode(BifsDecoder * codec, u32 NodeID)
{	
	assert(codec->current_graph);
	return SG_FindNode(codec->current_graph, NodeID);
}

M4Err InsertSFNode(void *mfnode_far_ptr, SFNode *new_child, s32 Position)
{
	Chain *list = *(Chain **) mfnode_far_ptr;
	if (Position == -1) {
		return ChainAddEntry(list, new_child);
	} 
	return ChainInsertEntry(list, new_child, Position);
}


u32 GetNumBits(u32 MaxVal)
{
	u32 k=0;
	while ((s32) MaxVal > ((1<<k)-1) ) k+=1;
	return k;
}


void BIFS_SetClock(LPBIFSDEC codec, Double (*GetSceneTime)(void *st_cbk), void *st_cbk )
{
	codec->GetSceneTime = GetSceneTime;
	codec->st_cbk = st_cbk;
}

u16 BIFS_GetActiveStream(LPBIFSDEC codec)
{
	if (!codec || !codec->info) return 0;
	return codec->info->ESID;
}

