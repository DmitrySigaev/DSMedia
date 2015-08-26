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

#include <intern/m4_scenegraph_dev.h>

/*private stack for conditional*/
typedef struct
{
	/*BIFS decoder*/
	BifsDecoder *codec;
	/*BIFS config of original stream carrying the conditional*/
	BIFSStreamInfo *info;
} ConditionalStack;

void Conditional_PreDestroy(SFNode *n)
{
	ConditionalStack *priv = Node_GetPrivate(n);
	if (priv) free(priv);
}

void Conditional_BufferReplaced(BifsDecoder *codec, SFNode *n)
{
	ConditionalStack *priv = Node_GetPrivate(n);
	if (!priv || (Node_GetTag(n) != TAG_MPEG4_Conditional)) return;
	priv->info = codec->info;
}

static void Conditional_execute(M_Conditional *node)
{
	M4Err e;
	BitStream *bs;
	BifsDecoder *codec;
	LPPROTO prevproto;
	LPSCENEGRAPH prev_graph;
	ConditionalStack *priv = Node_GetPrivate((SFNode*)node);
	if (!priv) return;

	/*set the codec working graph to the node one (to handle conditional in protos)*/
	prev_graph = priv->codec->current_graph;
	priv->codec->current_graph = Node_GetParentGraph((SFNode*)node);
	assert(priv->codec->current_graph);

	priv->codec->info = priv->info;
	prevproto = priv->codec->pCurrentProto;
	priv->codec->pCurrentProto = NULL;
	if (priv->codec->current_graph->pOwningProto) priv->codec->pCurrentProto = priv->codec->current_graph->pOwningProto->proto_interface;

	/*set isActive - to clarify in the specs*/
	node->isActive = 1;
	Node_OnEventOutSTR((SFNode *)node, "isActive");
	if (!node->buffer.bufferSize) return;

	bs = NewBitStream(node->buffer.buffer, node->buffer.bufferSize, BS_READ);
	codec = priv->codec;
	/*this may destroy the conditional...*/
	e = BIFS_ParseCommand(codec, bs);
	DeleteBitStream(bs);
	codec->pCurrentProto = prevproto;
	codec->current_graph = prev_graph;
	//set isActive - to clarify in the specs
//	node->isActive = 0;
}

void Conditional_OnActivate(SFNode *n)
{
	M_Conditional *node = (M_Conditional *)n;
	if (! node->activate) return;
	Conditional_execute(node);
}

void Conditional_OnReverseActivate(SFNode *n)
{
	M_Conditional *node = (M_Conditional *)n;
	if (node->reverseActivate) return;
	Conditional_execute(node);
}

void SetupConditional(BifsDecoder *codec, SFNode *node)
{
	ConditionalStack *priv;
	if (Node_GetTag(node) != TAG_MPEG4_Conditional) return;
	priv =malloc(sizeof(ConditionalStack));

	/*needed when initializing extern protos*/
	if (!codec->info) codec->info = ChainGetEntry(codec->streamInfo, 0);
	if (!codec->info) return;

	priv->info = codec->info;
	priv->codec = codec;
	Node_SetPreDestroyFunction(node, Conditional_PreDestroy);
	Node_SetPrivate(node, priv);
	((M_Conditional *)node)->on_activate = Conditional_OnActivate;
	((M_Conditional *)node)->on_reverseActivate = Conditional_OnReverseActivate;
}

/*this is ugly but we have no choice, we need to clone the conditional stack because of externProto*/
void BIFS_SetupConditionalClone(SFNode *node, SFNode *orig)
{
	M_Conditional *ptr;
	u32 i;
	ConditionalStack *priv_orig, *priv;
	priv_orig = Node_GetPrivate(orig);
	/*looks we're not in BIFS*/
	if (!priv_orig) {
		M_Conditional *c_orig, *c_dest;
		c_orig = (M_Conditional *)orig;
		c_dest = (M_Conditional *)node;
		Node_Init(node);
		/*and clone all commands*/
		for (i=0; i<ChainGetCount(c_orig->buffer.commandList); i++) {
			SGCommand *ori_com = ChainGetEntry(c_orig->buffer.commandList, i);
			SGCommand *dest_com = SG_CloneCommand(ori_com, Node_GetParentGraph(node));
			if (dest_com) ChainAddEntry(c_dest->buffer.commandList, dest_com);
		}
		return;
	}
	priv = malloc(sizeof(ConditionalStack));
	priv->codec = priv_orig->codec;
	priv->info = priv_orig->info;
	Node_SetPreDestroyFunction(node, Conditional_PreDestroy);
	Node_SetPrivate(node, priv);
	ptr = (M_Conditional *)node;
	ptr->on_activate = Conditional_OnActivate;
	ptr->on_reverseActivate = Conditional_OnReverseActivate;
}
