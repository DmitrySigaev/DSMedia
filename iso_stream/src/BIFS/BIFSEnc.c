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


#include <intern/m4_bifs_dev.h>

SFNode *BE_FindNode(BifsEncoder *codec, u32 nodeID)
{
	if (codec->current_proto_graph) return SG_FindNode(codec->current_proto_graph, nodeID);
	assert(codec->scene_graph);
	return SG_FindNode(codec->scene_graph, nodeID);
}


LPBIFSENC BIFS_NewEncoder(LPSCENEGRAPH graph)
{
	BifsEncoder * tmp;
	tmp = malloc(sizeof(BifsEncoder));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(BifsEncoder));
	tmp->QPs = NewChain();
	tmp->streamInfo = NewChain();
	tmp->info = NULL;	
	tmp->mx = NewMutex();
	tmp->encoded_nodes = NewChain();
	tmp->scene_graph = graph;
	return tmp;
}

static BIFSStreamInfo *BE_GetStream(BifsEncoder * codec, u16 ESID)
{
	u32 i;
	BIFSStreamInfo *ptr;

	for (i=0;i<ChainGetCount(codec->streamInfo);i++) {
		ptr = (BIFSStreamInfo *) ChainGetEntry(codec->streamInfo, i);
		if(ptr->ESID==ESID) return ptr;
	}
	return NULL;
}

void BIFS_DeleteEncoder(LPBIFSENC codec)
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
	DeleteChain(codec->encoded_nodes);
	MX_Delete(codec->mx);
	free(codec);
}

M4Err BIFS_NewStream(LPBIFSENC codec, u16 ESID, BIFSConfigDescriptor *cfg, Bool encodeNames, Bool has_predictive)
{
	BIFSStreamInfo *pInfo;
	
	MX_P(codec->mx);
	if (BE_GetStream(codec, ESID) != NULL) {
		MX_V(codec->mx);
		return M4BadParam;
	}
	
	pInfo = malloc(sizeof(BIFSStreamInfo));
	memset(pInfo, 0, sizeof(BIFSStreamInfo));
	pInfo->ESID = ESID;
	pInfo->UseName = encodeNames;
	pInfo->config.Height = cfg->pixelHeight;
	pInfo->config.Width = cfg->pixelWidth;
	pInfo->config.IsCommandStream = cfg->isCommandStream;
	pInfo->config.NodeIDBits = cfg->nodeIDbits;
	pInfo->config.RouteIDBits = cfg->routeIDbits;
	pInfo->config.ProtoIDBits = cfg->protoIDbits;
	pInfo->config.PixelMetrics = cfg->pixelMetrics;
	pInfo->config.version = (has_predictive || cfg->protoIDbits) ? 2 : 1;
	pInfo->config.UsePredictiveMFField = has_predictive;
	ChainAddEntry(codec->streamInfo, pInfo);
	MX_V(codec->mx);
	return M4OK;
}

M4Err BIFS_EncodeAU(LPBIFSENC codec, u16 ESID, Chain *command_list, char **out_data, u32 *out_data_length)
{
	BitStream *bs;
	M4Err e;

	if (!codec || !command_list || !out_data || !out_data_length) return M4BadParam;

	MX_P(codec->mx);
	codec->info = BE_GetStream(codec, ESID);
	if (!codec->info) {
		MX_V(codec->mx);
		return M4BadParam;
	}

	bs = NewBitStream(NULL, 0, BS_WRITE);

	if (codec->info->config.IsCommandStream) {
		e = BIFS_EncCommands(codec, command_list, bs);
	} else {
		e = M4AnimNotSupported;
	}
	BS_Align(bs);
	BS_GetContent(bs, (unsigned char **) out_data, out_data_length);
	DeleteBitStream(bs);
	MX_V(codec->mx);
	return e;
}

M4Err BIFS_GetStreamConfig(LPBIFSENC codec, u16 ESID, char **out_data, u32 *out_data_length)
{
	BitStream *bs;

	if (!codec || !out_data || !out_data_length) return M4BadParam;

	MX_P(codec->mx);
	codec->info = BE_GetStream(codec, ESID);
	if (!codec->info) {
		MX_V(codec->mx);
		return M4BadParam;
	}

	bs = NewBitStream(NULL, 0, BS_WRITE);
	
	if (codec->info->config.version==2) {
		BS_WriteInt(bs, codec->info->config.Use3DMeshCoding ? 1 : 0, 1);
		BS_WriteInt(bs, codec->info->config.UsePredictiveMFField ? 1 : 0, 1);
	}
	BS_WriteInt(bs, codec->info->config.NodeIDBits, 5);
	BS_WriteInt(bs, codec->info->config.RouteIDBits, 5);
	if (codec->info->config.version==2) {
		BS_WriteInt(bs, codec->info->config.ProtoIDBits, 5);
	}
	BS_WriteInt(bs, codec->info->config.IsCommandStream ? 1 : 0, 1);

	if (codec->info->config.IsCommandStream) {
		BS_WriteInt(bs, codec->info->config.PixelMetrics ? 1 : 0, 1);
		if (codec->info->config.Width || codec->info->config.Height) {
			BS_WriteInt(bs, 1, 1);
			BS_WriteInt(bs, codec->info->config.Width, 16);
			BS_WriteInt(bs, codec->info->config.Height, 16);
		} else {
			BS_WriteInt(bs, 0, 1);
		}
	}
	
	BS_Align(bs);
	BS_GetContent(bs, (unsigned char **) out_data, out_data_length);
	DeleteBitStream(bs);
	MX_V(codec->mx);
	return M4OK;
}

u8 BIFS_GetVersion(LPBIFSENC codec, u16 ESID)
{
	u8 ret = 0;
	MX_P(codec->mx);
	codec->info = BE_GetStream(codec, ESID);
	if (codec->info) ret = codec->info->config.version;
	MX_V(codec->mx);
	return ret;
}


void BE_LogBits(BifsEncoder *codec, s32 val, u32 nbBits, char *str, char *com)
{
	if (!codec->trace) return;
	fprintf(codec->trace, "%s\t\t%d\t\t%d", str, nbBits, val);
	if (com) fprintf(codec->trace, "\t\t//%s", com);
	fprintf(codec->trace, "\n");
}

void BE_LogFloat(BifsEncoder *codec, Float val, u32 nbBits, char *str, char *com)
{
	if (!codec->trace) return;
	fprintf(codec->trace, "%s\t\t%d\t\t%g", str, nbBits, val);
	if (com) fprintf(codec->trace, "\t\t//%s", com);
	fprintf(codec->trace, "\n");
}

void BE_SetTrace(BifsEncoder *codec, FILE *trace)
{
	codec->trace = trace;
	if (trace) fprintf(codec->trace, "Name\t\tNbBits\t\tValue\t\t//comment\n\n");
}
