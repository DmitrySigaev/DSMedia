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

#include <m4_author.h>
#include <intern/m4_scenegraph_dev.h>

#ifndef M4_READ_ONLY

typedef struct _statman
{
	M4SceneStatistics *stats;
	Chain *def_nodes;
} StatManager;

static M4SceneStatistics *NewSceneStats()
{
	M4SceneStatistics *tmp = malloc(sizeof(M4SceneStatistics));
	memset(tmp, 0, sizeof(M4SceneStatistics));
	tmp->node_stats = NewChain();
	tmp->proto_stats = NewChain();

	tmp->max_2d.x = M4_MIN_FLOAT;
	tmp->max_2d.y = M4_MIN_FLOAT;
	tmp->max_3d.x = M4_MIN_FLOAT;
	tmp->max_3d.y = M4_MIN_FLOAT;
	tmp->max_3d.z = M4_MIN_FLOAT;
	tmp->min_2d.x = M4_MAX_FLOAT;
	tmp->min_2d.y = M4_MAX_FLOAT;
	tmp->min_3d.x = M4_MAX_FLOAT;
	tmp->min_3d.y = M4_MAX_FLOAT;
	tmp->min_3d.z = M4_MAX_FLOAT;
	return tmp;
}

static void ResetStatisitics(M4SceneStatistics *stat)
{
	while (ChainGetCount(stat->node_stats)) {
		NodeStats *ptr = ChainGetEntry(stat->node_stats, 0);
		ChainDeleteEntry(stat->node_stats, 0);
		free(ptr);
	}
	while (ChainGetCount(stat->proto_stats)) {
		NodeStats *ptr = ChainGetEntry(stat->proto_stats, 0);
		ChainDeleteEntry(stat->proto_stats, 0);
		free(ptr);
	}
	stat->max_2d.x = M4_MIN_FLOAT;
	stat->max_2d.y = M4_MIN_FLOAT;
	stat->max_3d.x = M4_MIN_FLOAT;
	stat->max_3d.y = M4_MIN_FLOAT;
	stat->max_3d.z = M4_MIN_FLOAT;
	stat->min_2d.x = M4_MAX_FLOAT;
	stat->min_2d.y = M4_MAX_FLOAT;
	stat->min_3d.x = M4_MAX_FLOAT;
	stat->min_3d.y = M4_MAX_FLOAT;
	stat->min_3d.z = M4_MAX_FLOAT;
	stat->count_2d = stat->rem_2d = stat->count_3d = stat->rem_3d = stat->count_float = 0;
	stat->rem_float = stat->count_color = stat->rem_color = stat->count_2f = stat->count_3f = 0;
}

static void DeleteStatisitics(M4SceneStatistics *stat)
{
	ResetStatisitics(stat);
	DeleteChain(stat->node_stats);
	DeleteChain(stat->proto_stats);
	free(stat);
}

static void StatNode(M4SceneStatistics *stat, SFNode *n, Bool isUsed, Bool isDelete, SFNode *prev)
{
	u32 i;
	NodeStats *ptr = NULL;
	if (!stat) return;

	if (n->sgprivate->tag == TAG_ProtoNode) {
		ProtoInstance *pr = (ProtoInstance *)n;
		for (i=0; i<ChainGetCount(stat->proto_stats); i++) {
			ptr = ChainGetEntry(stat->proto_stats, i);
			if (pr->proto_interface->ID == ptr->tag) break;
			ptr = NULL;
		}
		if (!ptr) {
			ptr = malloc(sizeof(NodeStats));
			memset(ptr, 0, sizeof(NodeStats));
			ptr->tag = pr->proto_interface->ID;
			ptr->name = Proto_GetName(pr->proto_interface);
			ChainAddEntry(stat->proto_stats, ptr);
		}
	} else {
		for (i=0; i<ChainGetCount(stat->node_stats); i++) {
			ptr = ChainGetEntry(stat->node_stats, i);
			if (n->sgprivate->tag == ptr->tag) break;
			ptr = NULL;
		}
		if (!ptr) {
			ptr = malloc(sizeof(NodeStats));
			memset(ptr, 0, sizeof(NodeStats));
			ptr->tag = n->sgprivate->tag;
			ptr->name = Node_GetName(n);
			ChainAddEntry(stat->node_stats, ptr);
		}
	}
	if (isDelete) ptr->nb_del += n->sgprivate->num_instances;
	else if (isUsed) ptr->nb_used += 1;
	/*this is because the node passes twice in the stat, once on DumpNode and once in replaceALL*/
	else ptr->nb_created += prev ? (prev->sgprivate->num_instances - 1) : 1;
}

static void StatSFVec2f(M4SceneStatistics *stat, SFVec2f *val)
{
	if (!stat) return;
	if (stat->max_2d.x < val->x) stat->max_2d.x = val->x;
	if (stat->max_2d.y < val->y) stat->max_2d.y = val->y;
	if (stat->min_2d.x > val->x) stat->min_2d.x = val->x;
	if (stat->min_2d.y > val->y) stat->min_2d.y = val->y;
}

static void StatSFVec3f(M4SceneStatistics *stat, SFVec3f *val)
{
	if (!stat) return;
	if (stat->max_3d.x < val->x) stat->max_3d.x = val->x;
	if (stat->max_3d.y < val->y) stat->max_3d.y = val->y;
	if (stat->max_3d.z < val->z) stat->max_3d.z = val->y;
	if (stat->min_3d.x > val->x) stat->min_3d.x = val->x;
	if (stat->min_3d.y > val->y) stat->min_3d.y = val->y;
	if (stat->min_3d.z > val->z) stat->min_3d.z = val->z;
}

static void StatField(M4SceneStatistics *stat, FieldInfo *field)
{
	u32 i;

	switch (field->fieldType) {
	case FT_SFFloat:
		stat->count_float++;
		break;
	case FT_SFColor:
		stat->count_color++;
		break;
	case FT_SFVec2f:
		stat->count_2f++;
		break;
	case FT_SFVec3f:
		stat->count_3f++;
		break;

	case FT_MFFloat:
		stat->count_float+= ((MFFloat *)field->far_ptr)->count;
		break;
	case FT_MFColor:
		stat->count_color+= ((MFColor *)field->far_ptr)->count;
		break;
	case FT_MFVec2f:
	{
		MFVec2f *mf2d = (MFVec2f *)field->far_ptr;
		for (i=0; i<mf2d->count; i++) {
			StatSFVec2f(stat, &mf2d->vals[i]);
			stat->count_2d ++;
		}
	}
		break;
	case FT_MFVec3f:
	{
		MFVec3f *mf3d = (MFVec3f *)field->far_ptr;
		for (i=0; i<mf3d->count; i++) {
			StatSFVec3f(stat, &mf3d->vals[i]);
			stat->count_3d ++;
		}
	}
		break;
	}
}


static void StatSingleField(M4SceneStatistics *stat, FieldInfo *field)
{
	switch (field->fieldType) {
	case FT_SFVec2f:
		StatSFVec2f(stat, (SFVec2f *)field->far_ptr);
		break;
	case FT_MFVec3f:
		StatSFVec3f(stat, (SFVec3f *)field->far_ptr);
		break;
	}
}


static void StatRemField(M4SceneStatistics *stat, u32 fieldType, FieldInfo *field)
{
	u32 count = 1;
	if (field) count = ((GenMFField*)field->far_ptr)->count;
	switch (fieldType) {
	case FT_MFFloat:
		stat->rem_float += count;
		break;
	case FT_SFColor:
		stat->rem_color += count;
		break;
	case FT_MFVec2f:
		stat->rem_2d += count;
		break;
	case FT_MFVec3f:
		stat->rem_3d += count;
		break;
	}
}


Bool StatIsUSE(StatManager *st, SFNode *n) 
{
	u32 i;
	if (!n || !n->sgprivate->NodeID) return 0;
	for (i=0; i<ChainGetCount(st->def_nodes); i++) {
		SFNode *ptr = ChainGetEntry(st->def_nodes, i);
		if (ptr == n) return 1;
	}
	ChainAddEntry(st->def_nodes, n);
	return 0;
}

static M4Err StatNodeGraph(StatManager *st, SFNode *n)
{
	SFNode *child, *clone;
	Chain *list;
	u32 i, count, j;
	FieldInfo field, clone_field;

	if (!n) return M4OK;
	StatNode(st->stats, n, StatIsUSE(st, n), 0, NULL);
	count = Node_GetFieldCount(n);

	if (n->sgprivate->tag != TAG_ProtoNode) {
		clone = SG_NewNode(n->sgprivate->scenegraph, n->sgprivate->tag);
	} else {
		clone = Proto_CreateNode(n->sgprivate->scenegraph, ((ProtoInstance *)n)->proto_interface, NULL);
	}
	Node_Register(clone, NULL);

	for (i=0; i<count; i++) {
		Node_GetField(n, i, &field);
		if (field.eventType==ET_EventIn) continue;
		if (field.eventType==ET_EventOut) continue;

		switch (field.fieldType) {
		case FT_SFNode:
			child = *((SFNode **)field.far_ptr);
			StatNodeGraph(st, child);
			break;
		case FT_MFNode:
			list = *((Chain **)field.far_ptr);
			for (j=0; j<ChainGetCount(list); j++) {
				child = ChainGetEntry(list, j);
				StatNodeGraph(st, child);
			}
			break;
		default:
			Node_GetField(clone, i, &clone_field);
			if (!VRML_FieldsEqual(clone_field.far_ptr, field.far_ptr, field.fieldType)) {
				StatField(st->stats, &field);
			}
			break;
		}
	}
	Node_Unregister(clone, NULL);
	return M4OK;
}

M4Err M4SM_GetCommandStatistics(LPSTATMAN stat, SGCommand *com)
{
	FieldInfo field;
	M4Err e;
	u32 i;
	Chain *list;
	CommandFieldInfo *inf = NULL;
	if (ChainGetCount(com->command_fields)) 
		inf = ChainGetEntry(com->command_fields, 0);

	if (!com || !stat) return M4BadParam;
	switch (com->tag) {
	case SG_SceneReplace:
		if (com->node) StatNodeGraph(stat, com->node);
		break;
	case SG_NodeReplace:
		if (inf && inf->new_node) StatNodeGraph(stat, inf->new_node);
		break;
	case SG_FieldReplace:
		if (!inf) return M4OK;
		e = Node_GetField(com->node, inf->fieldIndex, &field);
		if (e) return e;

		switch (field.fieldType) {
		case FT_SFNode:
			if (inf->new_node) StatNodeGraph(stat, inf->new_node);
			break;
		case FT_MFNode:
			list = * ((Chain **) inf->field_ptr);
			for (i=0; i<ChainGetCount(list); i++) {
				SFNode *node = ChainGetEntry(list, i);
				StatNodeGraph(stat, node);
			}
			break;
		default:
			field.far_ptr = inf->field_ptr;
			StatField(stat->stats, &field);
			break;
		}
		break;
	case SG_IndexedReplace:
		if (!inf) return M4OK;
		e = Node_GetField(com->node, inf->fieldIndex, &field);
		if (e) return e;

		if (field.fieldType == FT_MFNode) {
			StatNodeGraph(stat, inf->new_node);
		} else {
			field.fieldType = VRML_GetSFType(field.fieldType);
			field.far_ptr = inf->field_ptr;
			StatSingleField(stat->stats, &field);
		}
		break;
	case SG_NodeDelete:
		if (com->node) StatNode(stat->stats, com->node, 0, 1, NULL);
		break;
	case SG_IndexedDelete:
		if (!inf) return M4OK;
		e = Node_GetField(com->node, inf->fieldIndex, &field);
		if (e) return e;

		/*then we need special handling in case of a node*/
		if (VRML_GetSFType(field.fieldType) == FT_SFNode) {
			SFNode *n = ChainGetEntry(* ((Chain **) field.far_ptr), inf->pos);
			if (n) StatNode(stat->stats, n, 0, 1, NULL);
		} else {
			StatRemField(stat->stats, inf->fieldType, NULL);
		}
		break;
	case SG_NodeInsert:
		if (inf && inf->new_node) StatNodeGraph(stat, inf->new_node);
		break;
	case SG_IndexedInsert:
		if (!inf) return M4OK;
		e = Node_GetField(com->node, inf->fieldIndex, &field);
		if (e) return e;

		/*rescale the MFField and parse the SFField*/
		if (field.fieldType != FT_MFNode) {
			field.fieldType = VRML_GetSFType(field.fieldType);
			field.far_ptr = inf->field_ptr;
			StatSingleField(stat->stats, &field);
		} else {
			if (inf->new_node) StatNodeGraph(stat, inf->new_node);
		}
		break;
	case SG_RouteReplace:
	case SG_RouteDelete:
	case SG_RouteInsert:
		return M4OK;
	default:
		return M4BadParam;
	}
	return M4OK;
}

static M4Err M4SM_StatAU(Chain *commandList, StatManager *st)
{
	u32 i, count;
	count = ChainGetCount(commandList);
	for (i=0; i<count; i++) {
		SGCommand *com = ChainGetEntry(commandList, i);
		M4SM_GetCommandStatistics(st, com);
	}
	return M4OK;
}

M4Err M4SM_GetStatistics(LPSTATMAN stat, M4SceneManager *sm)
{
	u32 i, j;
	M4Err e;

	for (i=0; i<ChainGetCount(sm->streams); i++) {
		M4StreamContext *sc = ChainGetEntry(sm->streams, i);
		if (sc->streamType != M4ST_SCENE) continue;

		for (j=0; j<ChainGetCount(sc->AUs); j++) {
			M4AUContext *au = ChainGetEntry(sc->AUs, j);
			e = M4SM_StatAU(au->commands, stat);
			if (e) return e;
		}
	}
	return M4OK;
}

M4Err M4SM_GetGraphStatistics(LPSTATMAN stat, LPSCENEGRAPH sg)
{
	if (!stat || !sg) return M4BadParam;
	return StatNodeGraph(stat, sg->RootNode);
}

/*creates new stat handler*/
LPSTATMAN M4SM_NewStatisitics()
{
	StatManager *sm = malloc(sizeof(StatManager));
	sm->def_nodes = NewChain();
	sm->stats = NewSceneStats();
	return sm;

}
/*deletes stat object returned by one of the above functions*/
void M4SM_DeleteStatisitics(LPSTATMAN stat)
{
	DeleteChain(stat->def_nodes);
	DeleteStatisitics(stat->stats);
	free(stat);
}

M4SceneStatistics *M4SM_GetStatisitics(LPSTATMAN stat)
{
	return stat->stats;
}

void M4SM_ResetStatisitics(LPSTATMAN stat)
{
	if (!stat) return;
	ResetStatisitics(stat->stats);
}

#endif

