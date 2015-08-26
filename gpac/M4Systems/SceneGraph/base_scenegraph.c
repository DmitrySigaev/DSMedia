/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Scene Graph sub-project
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

#include <gpac/intern/m4_scenegraph_dev.h>
/*svg proto*/
#include <gpac/m4_scenegraph_svg.h>
/*MPEG4 tags (for internal nodes)*/
#include <gpac/m4_mpeg4_nodes.h>
/*X3D tags (for internal nodes)*/
#include <gpac/m4_x3d_nodes.h>


#ifdef M4_USE_LASeR
#include "../LASeR/m4_laser_dev.h"
#endif


static void ReplaceDEFNode(SFNode *FromNode, u32 NodeID, SFNode *newNode, Bool updateOrderedGroup);


#define DEFAULT_MAX_CYCLIC_RENDER	30

LPSCENEGRAPH NewSceneGraph()
{
	LPSCENEGRAPH tmp;
	SAFEALLOC(tmp, sizeof(SceneGraph));
	if (!tmp) return NULL;

	tmp->protos = NewChain();
	tmp->unregistered_protos = NewChain();

	tmp->node_registry = malloc(sizeof(SFNode *) * NODEREG_STEP_ALLOC);
	tmp->node_reg_alloc = NODEREG_STEP_ALLOC;

	tmp->max_cyclic_render = DEFAULT_MAX_CYCLIC_RENDER;
	tmp->Routes = NewChain();
	tmp->routes_to_activate = NewChain();
	tmp->routes_to_destroy = NewChain();
	return tmp;
}

LPSCENEGRAPH SG_NewSubScene(LPSCENEGRAPH scene)
{
	LPSCENEGRAPH tmp;
	if (!scene) return NULL;
	tmp = NewSceneGraph();
	if (!tmp) return NULL;
	tmp->parent_scene = scene;
	tmp->userpriv = scene->userpriv;
	/*by default use the same scene time (protos need that) - user overrides it if needed (inlineScene)*/
	tmp->GetSceneTime = scene->GetSceneTime;
	tmp->SceneCallback = scene->SceneCallback;
	tmp->GetExternProtoLib = scene->GetExternProtoLib;
	tmp->js_ifce = scene->js_ifce;
	tmp->Script_Load = scene->Script_Load;

	tmp->max_cyclic_render = scene->max_cyclic_render;
	tmp->UserNodeInit = scene->UserNodeInit;
	tmp->NodeInitCallback = scene->NodeInitCallback;
	tmp->NodeModified = scene->NodeModified;
	tmp->ModifCallback = scene->ModifCallback;
	return tmp;
}


void SG_SetInitCallback(LPSCENEGRAPH sg, void (*UserNodeInit)(void *NodeInitCallback, SFNode *newNode), void *NodeInitCallback)
{
	sg->UserNodeInit = UserNodeInit;
	sg->NodeInitCallback = NodeInitCallback;
}

/*set node modified callback*/
void SG_SetModifiedCallback(LPSCENEGRAPH sg, void (*UserNodeModified)(void *NodeModifiedCallback, SFNode *newNode), void *NodeModifiedCallback)
{
	sg->NodeModified = UserNodeModified;
	sg->ModifCallback = NodeModifiedCallback;
}

void SG_SetSceneTimeCallback(LPSCENEGRAPH sg, Double (*GetSceneTime)(void *scene_callback), void *cbck)
{
	sg->GetSceneTime = GetSceneTime;
	sg->SceneCallback = cbck;
}


Double Node_GetSceneTime(SFNode *node)
{
	if (!node || !node->sgprivate->scenegraph->GetSceneTime) return 0.0;
	return node->sgprivate->scenegraph->GetSceneTime(node->sgprivate->scenegraph->SceneCallback);
}

void SG_SetCyclicRenderLimit(LPSCENEGRAPH sg, u16 max_cycle)
{
	/*this is a safety guard for the 3D renderer which may call Shape.render from within Shape.render*/
	if (max_cycle<2) max_cycle = 2;
	sg->max_cyclic_render = (u32) max_cycle;
}

void SG_Delete(LPSCENEGRAPH sg)
{	
	if (!sg) return;

	SG_Reset(sg);

	DeleteChain(sg->Routes);
	DeleteChain(sg->protos);
	DeleteChain(sg->unregistered_protos);
	DeleteChain(sg->routes_to_activate);
	DeleteChain(sg->routes_to_destroy);
	free(sg->node_registry);

	free(sg);
}

/*recursive traverse of the whole graph to check for scope mixes (nodes from an inline graph
inserted in a parent graph through bind or routes). We must do this otherwise we're certain to get random
crashes or mem leaks.*/
void SG_GraphRemoved(SFNode *node, LPSCENEGRAPH sg)
{
	u32 i, j, count;
	FieldInfo info;
	SFNode *n;
	Chain *mflist;

	count = Node_GetFieldCount(node);
	for (i=0; i<count; i++) {
		Node_GetField(node, i, &info);
		if (info.fieldType==FT_SFNode) {
			n = *(SFNode **) info.far_ptr;
			if (n) {
				if (n->sgprivate->scenegraph==sg) {
					/*if root of graph, skip*/
					if (sg->RootNode!=n) {
						Node_Unregister(n, node);
						/*don't forget to remove node...*/
						*(SFNode **) info.far_ptr = NULL;
					}
				} else {
					SG_GraphRemoved(n, sg);
				}
			}
		}
		else if (info.fieldType==FT_MFNode) {
			mflist = *(Chain **) info.far_ptr;
			for (j=0; j<ChainGetCount(mflist); j++) {
				n = ChainGetEntry(mflist, j);
				if (n->sgprivate->scenegraph==sg) {
					Node_Unregister(n, node);
					ChainDeleteEntry(mflist, j);
					j--;
				} else {
					SG_GraphRemoved(n, sg);
				}
			}
		}
	}
}

void SG_Reset(LPSCENEGRAPH sg)
{
	u32 i;
	if (!sg) return;

	/*inlined graph, remove any of this graph nodes from the parent graph*/
	if (!sg->pOwningProto && sg->parent_scene) {
		LPSCENEGRAPH par = sg->parent_scene;
		while (par->parent_scene) par = par->parent_scene;
		if (par->RootNode) SG_GraphRemoved(par->RootNode, sg);
	}
	if (sg->RootNode) Node_Unregister(sg->RootNode, NULL);
	sg->RootNode = NULL;

	while (ChainGetCount(sg->routes_to_activate)) {
		ChainDeleteEntry(sg->routes_to_activate, 0);
	}

	/*destroy all routes*/
	while (ChainGetCount(sg->Routes)) {
		Route *r = ChainGetEntry(sg->Routes, 0);
		/*this will unregister the route from the graph, so don't delete the chain entry*/
		SG_DeleteRoute(r);

	}

	/*WATCHOUT: we may have cyclic dependencies due to
	1- a node referencing itself (forbidden in VRML)
	2- nodes refered to in commands of conditionals children of this node (MPEG-4 is mute about that)
	*/
	for (i=0; i<sg->node_reg_size; i++) {
		u32 j;
		SFNode *node = sg->node_registry[i];
		/*first replace all instances in parents by NULL WITHOUT UNREGISTERING (to avoid destroying the node).
		This will take care of nodes referencing themselves*/
		for (j=0; j<ChainGetCount(node->sgprivate->parentNodes); j++) {
			SFNode *par = ChainGetEntry(node->sgprivate->parentNodes, j);
			ReplaceDEFNode(par, node->sgprivate->NodeID, NULL, 0);
		}
		/*then we remove the node from the registry and destroy it. This will take 
		care of conditional case as we perform special checking when destroying commands*/
		ChainReset(node->sgprivate->parentNodes);
		sg->node_registry[i] = NULL;
		DestroyNode(node);
	}
	sg->node_reg_size = 0;

	/*destroy all proto*/
	while (ChainGetCount(sg->protos)) {
		PrototypeNode *p = ChainGetEntry(sg->protos, 0);
		/*this will unregister the proto from the graph, so don't delete the chain entry*/
		SG_DeleteProto(p);
	}
	/*destroy all unregistered proto*/
	while (ChainGetCount(sg->unregistered_protos)) {
		PrototypeNode *p = ChainGetEntry(sg->unregistered_protos, 0);
		/*this will unregister the proto from the graph, so don't delete the chain entry*/
		SG_DeleteProto(p);
	}

	/*last destroy all routes*/
	SG_DestroyRoutes(sg);

	sg->simulation_tick = 0;
}


M4INLINE SFNode *SG_SearchForDuplicateNodeID(LPSCENEGRAPH sg, u32 nodeID, SFNode *toExclude)
{
	u32 i;
	for (i=0; i<sg->node_reg_size; i++) {
		if (sg->node_registry[i] == toExclude) continue;
		if (sg->node_registry[i]->sgprivate->NodeID == nodeID) {
			return sg->node_registry[i];
		}
	}
	return NULL;
}

M4INLINE SFNode *SG_SearchForNode(LPSCENEGRAPH sg, SFNode *node)
{
	u32 i;
	for (i=0; i<sg->node_reg_size; i++) {
		if (sg->node_registry[i] == node) {
			return sg->node_registry[i];
		}
	}
	return NULL;
}

static M4INLINE u32 node_search(LPSCENEGRAPH sg, u32 low_pos, u32 high_pos, u32 ID) 
{
	u32 mid_pos;

	assert(low_pos<high_pos);

	mid_pos = (high_pos+low_pos)/2;
	
	if (sg->node_registry[mid_pos]->sgprivate->NodeID == ID) return mid_pos;

	/* greater than middle, search upper half */
	if (sg->node_registry[mid_pos]->sgprivate->NodeID < ID) {
		if (mid_pos+1==sg->node_reg_size) {
			if (sg->node_registry[sg->node_reg_size-1]->sgprivate->NodeID >= ID) return sg->node_reg_size-1;
			return sg->node_reg_size;
		}
		if (sg->node_registry[mid_pos+1]->sgprivate->NodeID >= ID) return mid_pos+1;
		
		return node_search(sg, mid_pos+1, high_pos, ID);
	}

	/* less than middle, search lower half */
	if (mid_pos<=1) {
		if (sg->node_registry[0]->sgprivate->NodeID<ID) return 1;
		return 0;
	}
	if (sg->node_registry[mid_pos-1]->sgprivate->NodeID < ID) return mid_pos;
	return node_search(sg, low_pos, mid_pos-1, ID);
}


M4INLINE SFNode *SG_SearchForNodeByID(LPSCENEGRAPH sg, u32 nodeID)
{
	u32 i;
	if (!sg->node_reg_size) return NULL;

	i = node_search(sg, 0, sg->node_reg_size, nodeID);
	if (i>=sg->node_reg_size ||sg->node_registry[i]->sgprivate->NodeID != nodeID) return NULL;
	return sg->node_registry[i];
}

M4INLINE Bool SG_SearchForNodeIndex(LPSCENEGRAPH sg, SFNode *node, u32 *out_index)
{
	u32 i;
	for (i=0; i<sg->node_reg_size; i++) {
		if (sg->node_registry[i] == node) {
			*out_index = i;
			return 1;
		}
	}
	return 0;
}


void SG_SetPrivate(LPSCENEGRAPH sg, void *ptr)
{
	if (sg) sg->userpriv = ptr;
}

void *SG_GetPrivate(LPSCENEGRAPH sg)
{
	return sg ? sg->userpriv : NULL;
}


void SG_SetSizeInfo(LPSCENEGRAPH sg, u32 width, u32 height, Bool usePixelMetrics)
{
	if (!sg) return;
	if (width && height) {
		sg->width = width;
		sg->height = height;
	} else {
		sg->width = sg->height = 0;
	}
	sg->usePixelMetrics = usePixelMetrics;
}

Bool SG_UsePixelMetrics(LPSCENEGRAPH sg)
{
	return (sg ? sg->usePixelMetrics : 0);
}

Bool SG_GetSizeInfo(LPSCENEGRAPH sg, u32 *width, u32 *height)
{
	if (!sg) return 0;
	*width = sg->width;
	*height = sg->height;
	return (sg->width && sg->height) ? 1 : 0;
}


SFNode *SG_GetRootNode(LPSCENEGRAPH sg)
{
	return sg ? sg->RootNode : NULL;
}

void SG_SetRootNode(LPSCENEGRAPH sg, SFNode *node)
{
	if (sg) sg->RootNode = node;
}

M4Err Node_Unregister(SFNode *pNode, SFNode *parentNode)
{
	u32 node_ind, j;
	SceneGraph *pSG;

	if (!pNode) return M4OK;
	pSG = pNode->sgprivate->scenegraph;
	/*if this is a proto its is registered in its parent graph, not the current*/
	if (pNode == (SFNode*)pSG->pOwningProto) pSG = pSG->parent_scene;
	assert(pSG);

	if (parentNode) ChainDeleteItem(pNode->sgprivate->parentNodes, parentNode);

	/*unregister the instance*/
	assert(pNode->sgprivate->num_instances);
	pNode->sgprivate->num_instances -= 1;
	
	/*this is just an instance removed*/
	if (pNode->sgprivate->num_instances) return M4OK;
	
	assert(ChainGetCount(pNode->sgprivate->parentNodes)==0);

	/*if def, remove from sg def table*/
	if (pNode->sgprivate->NodeID) {
		if (!SG_SearchForNodeIndex(pSG, pNode, &node_ind)) {
			assert(0);
		}
		assert (pNode == pSG->node_registry[node_ind]);
		j = pSG->node_reg_size - node_ind - 1;
		if (j) memmove( & pSG->node_registry[node_ind], & pSG->node_registry[node_ind+1], j * sizeof(SFNode *));
		pSG->node_reg_size -= 1;
	}

	/*check all routes from or to this node and destroy them - cf spec*/
	for (j=0; j<ChainGetCount(pSG->Routes); j++) {
		Route *r = ChainGetEntry(pSG->Routes, j);
		if ( (r->ToNode == pNode) || (r->FromNode == pNode)) {
			SG_DeleteRoute(r);
			j--;
		}
	}

	/*delete the node*/
	DestroyNode(pNode);
	return M4OK;
}

M4Err Node_Register(SFNode *node, SFNode *parentNode)
{
	SceneGraph *pSG; 
	
	pSG = node->sgprivate->scenegraph;
	/*if this is a proto register to the parent graph, not the current*/
	if (node == (SFNode*)pSG->pOwningProto) pSG = pSG->parent_scene;
	assert(pSG);

#if 1
	if (node->sgprivate->NodeID) {
		SFNode *the_node = SG_SearchForNode(pSG, node);
		assert(the_node);
		assert(the_node == node);
	}
#endif

	node->sgprivate->num_instances += 1;
	/*parent may be NULL (top node and proto)*/
	if (parentNode) ChainAddEntry(node->sgprivate->parentNodes, parentNode);
	return M4OK;
}

void Node_UnregisterChildren(SFNode *container, Chain *list)
{
	while (ChainGetCount(list)) {
		SFNode *p = ChainGetEntry(list, 0);
		ChainDeleteEntry(list, 0);
		Node_Unregister(p, container);
	}
}

/*replace or remove node instance in the given node (eg in all SFNode or MFNode fields)
this doesn't propagate in the scene graph. If updateOrderedGroup and new_node is NULL, the order field of OG
is updated*/
static void ReplaceDEFNode(SFNode *FromNode, u32 NodeID, SFNode *newNode, Bool updateOrderedGroup)
{
	u32 i, j;
	SFNode *p;
	Chain *container;

	FieldInfo field;

	/*browse all fields*/
	for (i=0; i<Node_GetFieldCount(FromNode); i++) {
		Node_GetField(FromNode, i, &field);
		switch (field.fieldType) {
		case FT_SFNode:
			/*set to NULL for SFFields*/
			p = *((SFNode **) field.far_ptr);
			/*this is a USE / DEF*/
			if (p && (Node_GetID(p) == NodeID) ) {
				*((SFNode **) field.far_ptr) = NULL;
				if (newNode) {
					*((SFNode **) field.far_ptr) = newNode;
				}
				goto exit;
			}
			break;
		case FT_MFNode:
			container = *(Chain **) field.far_ptr;
			for (j=0; j<ChainGetCount(container); j++) {
				p = ChainGetEntry(container, j);
				/*replace nodes different from newNode but with same ID*/
				if ((newNode == p) || (Node_GetID(p) != NodeID)) continue;

				ChainDeleteEntry(container, j);
				if (newNode) {
					ChainInsertEntry(container, newNode, j);
				}
				else if (updateOrderedGroup && (FromNode->sgprivate->tag==TAG_MPEG4_OrderedGroup)) {
					M_OrderedGroup *og = (M_OrderedGroup *)FromNode;
					VRML_MF_Remove(&og->order, FT_SFInt32, j);
				}
				goto exit;
			}
			break;
			/*not a node, continue*/
		default:
			continue;
		}
	}
	/*since we don't filter parent nodes this is called once per USE, not per container, so return if found*/
exit:
	SG_NodeChanged(FromNode, &field);
}

/*get all parents of the node and replace, the instance of the node and finally destroy the node*/
M4Err Node_ReplaceAllInstances(SFNode *node, SFNode *new_node, Bool updateOrderedGroup)
{
	u32 i;
	Bool replace_root;
	SFNode *par;
	SceneGraph *pSG = node->sgprivate->scenegraph;

	/*if this is a proto its is registered in its parent graph, not the current*/
	if (node == (SFNode*)pSG->pOwningProto) pSG = pSG->parent_scene;
	if (!SG_SearchForNodeIndex(pSG, node, &i)) return M4BadParam;
	assert(node == pSG->node_registry[i]);

	/*first check if this is the root node*/
	replace_root = (node->sgprivate->scenegraph->RootNode == node) ? 1 : 0;

	while ( (i = ChainGetCount(node->sgprivate->parentNodes)) ) {
		par = ChainGetEntry(node->sgprivate->parentNodes, 0);
		ReplaceDEFNode(par, node->sgprivate->NodeID, new_node, updateOrderedGroup);
				
		/*adds the parent to the new node*/
		if (new_node) Node_Register(new_node, par);

		/*unregister node*/
		Node_Unregister(node, par);
		if (i==1) break;	/*we may be destroyed now*/
	}

	if (replace_root && new_node) new_node->sgprivate->scenegraph->RootNode = new_node;
	return M4OK;
}

static M4INLINE void insert_node_def(LPSCENEGRAPH sg, SFNode *def)
{
	u32 i, remain;

	if (sg->node_reg_alloc==sg->node_reg_size) {
		sg->node_reg_alloc+=NODEREG_STEP_ALLOC;
		sg->node_registry = realloc(sg->node_registry, sg->node_reg_alloc * sizeof(SFNode *));
	}

	i=0;
	if (sg->node_reg_size) {
		i = node_search(sg, 0, sg->node_reg_size, def->sgprivate->NodeID);
	}
	if (i<sg->node_reg_size) {
		remain = sg->node_reg_size-i;
		memmove(&sg->node_registry[i+1], &sg->node_registry[i], sizeof(SFNode *) * remain);
	}
	sg->node_registry[i] = def;
	sg->node_reg_size++;
}



M4Err Node_SetDEF(SFNode *p, u32 ID, const char *name)
{
	char *new_name;
	u32 i, j;
	SceneGraph *pSG; 
	if (!p || !p->sgprivate->scenegraph) return M4BadParam;

	pSG = p->sgprivate->scenegraph;
	/*if this is a proto register to the parent graph, not the current*/
	if (p == (SFNode*)pSG->pOwningProto) pSG = pSG->parent_scene;

	/*new DEF ID*/
	if (!p->sgprivate->NodeID) {
		p->sgprivate->NodeID = ID;
		if (p->sgprivate->NodeName) free(p->sgprivate->NodeName);
		p->sgprivate->NodeName = NULL;
		if (name) p->sgprivate->NodeName = strdup(name);
		assert(pSG);
		if (ID) insert_node_def(pSG, p);
		return M4OK;
	}

	/*just change DEF name*/
	new_name = name ? strdup(name) : NULL;
	if (p->sgprivate->NodeName) free(p->sgprivate->NodeName);
	p->sgprivate->NodeName = new_name;
	/*same ID, just return*/
	if (p->sgprivate->NodeID == ID) return M4OK;

	/*different ID, remove from node registry and re-insert (needed because node registry is sorted by IDs)*/
	if (!SG_SearchForNodeIndex(pSG, p, &i)) {
		assert(0);
	}
	assert (p == pSG->node_registry[i]);
	j = pSG->node_reg_size - i - 1;
	if (j) memmove( & pSG->node_registry[i], & pSG->node_registry[i+1], j * sizeof(SFNode *));
	pSG->node_reg_size -= 1;
	p->sgprivate->NodeID = ID;
	if (ID) insert_node_def(pSG, p);
	return M4OK;
}

/*calls RenderNode on this node*/
void Node_Render(SFNode *node, void *renderStack)
{
	u32 max_pass;
	if (!node) return;

	max_pass = (node->sgprivate->render_pass>>16);
	if (!max_pass) max_pass = node->sgprivate->scenegraph->max_cyclic_render;

	if (node->sgprivate->tag != TAG_ProtoNode) {
		if (node->sgprivate->RenderNode && (node->sgprivate->render_pass < max_pass)) { 
			node->sgprivate->render_pass ++;
			node->sgprivate->RenderNode(node, renderStack);
			node->sgprivate->render_pass --;
		}
		return;
	}

	/*proto only traverses its first child*/
	if (((ProtoInstance *) node)->RenderingNode) {
		node = ((ProtoInstance *) node)->RenderingNode;
	}
	/*if no rendering function is assigned this is a real proto (otherwise this is an hardcoded one)*/
	else if (!node->sgprivate->RenderNode) {
		/*if no rendering node, check if the proto is fully instanciated (externProto)*/
		ProtoInstance *proto_inst = (ProtoInstance *) node;
		Node_ClearDirty(node);
		/*proto has been deleted or dummy proto (without node code)*/
		if (!proto_inst->proto_interface || proto_inst->is_loaded) return;
		/*try to load the code*/
		proto_instanciate(proto_inst);
		if (!proto_inst->RenderingNode) {
			Node_SetDirty(node, 1);
			return;
		}
		node = proto_inst->RenderingNode;
		node->sgprivate->scenegraph->NodeModified(node->sgprivate->scenegraph->ModifCallback, node);
	}
	if (node->sgprivate->RenderNode && (node->sgprivate->render_pass < node->sgprivate->scenegraph->max_cyclic_render)) {
		node->sgprivate->render_pass ++;
		node->sgprivate->RenderNode(node, renderStack);
		node->sgprivate->render_pass --;
	}
}

/*blindly calls RenderNode on all nodes in the "children" list*/
void Node_RenderChildren(SFNode *node, void *renderStack)
{
	u32 i;
	SFNode *ptr;
	SFParent *par = (SFParent *)node;
	for (i=0; i<ChainGetCount(par->children); i++) {
		ptr = ChainGetEntry(par->children, i);
		if (ptr) Node_Render(ptr, renderStack);
	}
}


M4Err Node_GetFieldByName(SFNode *node, char *name, FieldInfo *field)
{
	u32 i, count;
	assert(node);
	count = Node_GetFieldCount(node);
	
	memset(field, 0, sizeof(FieldInfo));
	for (i=0; i<count;i++) {
		Node_GetField(node, i, field);
		if (!strcmp(field->name, name)) return M4OK;
	}
	return M4BadParam;
}

LPSCENEGRAPH Node_GetParentGraph(SFNode *node)
{
	return (node ? node->sgprivate->scenegraph : NULL);
}


SFNode *SG_FindNode(LPSCENEGRAPH sg, u32 nodeID)
{
	SFNode *node;
	assert(sg);
	node = SG_SearchForNodeByID(sg, nodeID);
	return node;
}

SFNode *SG_FindNodeByName(LPSCENEGRAPH sg, char *name)
{
	u32 i;
	assert(sg);
	for (i=0; i<sg->node_reg_size; i++) {
		if (!sg->node_registry[i]->sgprivate->NodeName) continue;
		if (!strcmp(sg->node_registry[i]->sgprivate->NodeName, name)) {
			return sg->node_registry[i];
		}
	}
	return NULL;
}


u32 SG_GetNextAvailableNodeID(LPSCENEGRAPH sg) 
{
	u32 i, ID;
	if (sg->node_reg_size == 0) return 1;
	ID = sg->node_registry[0]->sgprivate->NodeID;
	/*nodes are sorted*/
	for (i=1; i<sg->node_reg_size; i++) {
		if (ID+1<sg->node_registry[i]->sgprivate->NodeID) return ID+1;
		ID = sg->node_registry[i]->sgprivate->NodeID;
	}
	return ID+1;
}

u32 SG_GetMaxNodeID(LPSCENEGRAPH sg)
{
	if (sg->node_reg_size == 0) return 0;
	return sg->node_registry[sg->node_reg_size-1]->sgprivate->NodeID;
}

void Node_Setup(SFNode *p, u32 tag)
{
	SAFEALLOC(p->sgprivate, sizeof(NodePriv));
	p->sgprivate->tag = tag;
	p->sgprivate->is_dirty = SG_NODE_DIRTY;
	p->sgprivate->outRoutes = NewChain();
	p->sgprivate->parentNodes = NewChain();
}

NodePriv *Node_GetPriv(SFNode *node)
{
	return node->sgprivate;
}

SFNode *NewSFNode()
{
	SFNode *newnode = malloc(sizeof(SFNode));
	Node_Setup(newnode, TAG_UndefinedNode);
	return newnode;
}
u32 Node_GetTag(SFNode*p)
{
	return p->sgprivate->tag;
}
u32 Node_GetID(SFNode*p)
{
	return p->sgprivate->NodeID;
}
const char *Node_GetDefName(SFNode*p)
{
	return p->sgprivate->NodeName;
}
void *Node_GetPrivate(SFNode*p)
{
	return p->sgprivate->privateStack;
}
void Node_SetPrivate(SFNode*p, void *pr)
{
	p->sgprivate->privateStack = pr;
}
M4Err Node_SetRenderFunction(SFNode *p, void (*RenderNode)(SFNode *node, void *render_stack) )
{
	p->sgprivate->RenderNode = RenderNode;
	return M4OK;
}

M4Err Node_SetPreDestroyFunction(SFNode *p, void (*PreDestroyNode)(struct _sfNode *node) )
{
	p->sgprivate->PreDestroyNode = PreDestroyNode;
	return M4OK;
}

void SetupChildrenNode(SFNode *pNode)
{
	SFParent *par = (SFParent *)pNode;
	par->children = NewChain();
	pNode->sgprivate->is_dirty |= SG_CHILD_DIRTY;
}
void NodeList_Delete(Chain *children, SFNode *parent)
{
	Node_UnregisterChildren(parent, children);
	DeleteChain(children);
}

void DestroyChildrenNode(SFNode *pNode)
{
	SFParent *par = (SFParent *)pNode;
	NodeList_Delete(par->children, pNode);
}



void SFNode_Delete(SFNode *node)
{
	if (!node) return;

	assert(ChainGetCount(node->sgprivate->outRoutes)==0);
/*
	while (ChainGetCount(node->sgprivate->outRoutes)) {
		LPROUTE r = ChainGetEntry(node->sgprivate->outRoutes, 0);
		ChainDeleteEntry(node->sgprivate->outRoutes, 0);
		r->FromNode = NULL;
	}
*/

	DeleteChain(node->sgprivate->outRoutes);
	node->sgprivate->outRoutes = NULL;
	if (node->sgprivate->PreDestroyNode) node->sgprivate->PreDestroyNode(node);
	assert(! ChainGetCount(node->sgprivate->parentNodes));
	DeleteChain(node->sgprivate->parentNodes);
	if (node->sgprivate->NodeName) free(node->sgprivate->NodeName);
	free(node->sgprivate);
	free(node);
}

u32 Node_GetParentCount(SFNode *node)
{
	return ChainGetCount(node->sgprivate->parentNodes);
}

SFNode *Node_GetParent(SFNode *node, u32 idx)
{
	return ChainGetEntry(node->sgprivate->parentNodes, idx);
}

static void dirty_children(SFNode *node, u16 val)
{
	u32 i, count;
	FieldInfo info;
	if (!node) return;
	
	node->sgprivate->is_dirty = val;
	count = Node_GetFieldCount(node);
	for (i=0; i<count; i++) {
		Node_GetField(node, i, &info);
		if (info.fieldType==FT_SFNode) dirty_children(*(SFNode **)info.far_ptr, val);
		else if (info.fieldType==FT_MFNode) {
			Chain *list = *(Chain **) info.far_ptr;
			u32 j, n;
			n = ChainGetCount(list);
			for (j=0; j<n; j++) 
				dirty_children((SFNode *)ChainGetEntry(list, j), val);
		}
	}
}
static void dirty_parents(SFNode *node)
{
	u32 i, count;
	count = ChainGetCount(node->sgprivate->parentNodes);
	for (i=0; i<count; i++) {
		SFNode *p = ChainGetEntry(node->sgprivate->parentNodes, i);
		if (p->sgprivate->is_dirty & SG_CHILD_DIRTY) continue;
		p->sgprivate->is_dirty |= SG_CHILD_DIRTY;
		dirty_parents(p);
	}
}

void Node_SetDirty(SFNode *node, Bool invalidate_parents)
{
	if (!node) return;
	node->sgprivate->is_dirty |= SG_NODE_DIRTY;
	if (invalidate_parents) dirty_parents(node);
}

void Node_ClearDirty(SFNode *node)
{
	if (node) node->sgprivate->is_dirty = 0;
}

u16 Node_GetDirty(SFNode *node)
{
	if (node) return node->sgprivate->is_dirty;
	return 0;
}


void Node_ClearDirtyChildren(SFNode *node, Bool skip_if_clean)
{
	if (!node) return;
	if (skip_if_clean && node && !node->sgprivate->is_dirty) return;
	node->sgprivate->is_dirty = 0;
	dirty_children(node, 0);
}

void Node_SetDirtyFlag(SFNode *node, u16 val)
{
	if (node) node->sgprivate->is_dirty = val;
}

void Node_ClearDirtyFlag(SFNode *node, u16 flag_to_remove)
{
	if (node) node->sgprivate->is_dirty &= ~flag_to_remove;
}

void Node_SetCyclicRenderLimit(SFNode *n, u16 max_cycles)
{
	u32 cycles;
	/*this is a safety guard for the 3D renderer which may call Shape.render from within Shape.render*/
	if (max_cycles<2) max_cycles = 2;
	cycles = max_cycles;
	cycles <<= 16;
	cycles |= (n->sgprivate->render_pass & 0x0000FFFF);
	n->sgprivate->render_pass = cycles;
}

Bool Node_IsFirstCyclicRender(SFNode *n)
{
	return ( (n->sgprivate->render_pass & 0x0000FFFF) == 1);
}


void Node_Init(SFNode *node)
{
	SceneGraph *pSG = node->sgprivate->scenegraph;
	assert(pSG);

	/*internal nodes*/
	if (VRML_NodeInit(node)) return;
	/*user defined init*/
	if (pSG->UserNodeInit) pSG->UserNodeInit(pSG->NodeInitCallback, node);
}


void SG_NodeChanged(SFNode *node, FieldInfo *field)
{
	LPSCENEGRAPH sg;
	if (!node) return;

	sg = node->sgprivate->scenegraph;
	assert(sg);

	/*internal nodes*/
	if (VRML_NodeChanged(node, field)) return;

	/*THIS IS BAD, LASeR MUST BE REWRITTEN TO USE FIELD TYPES for SF/MF Nodes*/
#ifdef M4_USE_LASeR
	switch (node->sgprivate->tag) {
	case TAG_LASeRTransform:
	case TAG_LASeRUse:
		node->sgprivate->is_dirty |= SG_CHILD_DIRTY;
		break;
	}
#endif

	/*force child dirty tag*/
	if (field && ((field->fieldType==FT_SFNode) || (field->fieldType==FT_MFNode))) node->sgprivate->is_dirty |= SG_CHILD_DIRTY;
	if (sg->NodeModified) sg->NodeModified(sg->ModifCallback, node);
}

void DestroyNode(SFNode *node)
{
#ifdef NODE_USE_POINTERS
	node->sgprivate->node_del(node);
#else

	if (node->sgprivate->tag==TAG_UndefinedNode) SFNode_Delete(node);
	else if (node->sgprivate->tag == TAG_ProtoNode) Proto_DeleteInstance((ProtoInstance *)node);
	else if (node->sgprivate->tag<=TAG_RANGE_LAST_MPEG4) MPEG4_Node_Del(node);
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_X3D) X3D_Node_Del(node);
#ifndef M4_DISABLE_SVG
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_SVG) SVGElement_Del((SVGElement *) node);
#endif
#ifdef M4_USE_LASeR
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_LASeR) LASeRNode_Del(node);
#endif
	else SFNode_Delete(node);
#endif
}

u32 Node_GetFieldCount(SFNode *node)
{
	assert(node);
	if (node->sgprivate->tag <= TAG_UndefinedNode) return 0;
	/*for both MPEG4 & X3D*/
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_X3D) return Node_GetNumFields(node, FCM_ALL);
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_SVG) return SVG_GetAttributeCount(node);
#ifdef M4_USE_LASeR
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_LASeR) return LASeRNode_GetFieldCount(node, 0);
#endif
	return 0;
}



const char *Node_GetName(SFNode *node)
{
	assert(node && node->sgprivate->tag);
#ifdef NODE_USE_POINTERS
	return Node->sgprivate->name;
#else
	if (node->sgprivate->tag==TAG_UndefinedNode) return "UndefinedNode";
	else if (node->sgprivate->tag==TAG_ProtoNode) return ((ProtoInstance*)node)->proto_name;
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_MPEG4) return MPEG4_GetNodeName(node->sgprivate->tag);
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_X3D) return X3D_GetNodeName(node->sgprivate->tag);
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_SVG) return SVG_GetElementName(node->sgprivate->tag);
#ifdef M4_USE_LASER
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_LASeR) return LASeR_GetNodeName(node->sgprivate->tag);
#endif
	else return "UnsupportedNode";
#endif
}

SFNode *SG_NewNode(LPSCENEGRAPH inScene, u32 tag)
{
	SFNode *node;
	if (!inScene) return NULL;
	/*cannot create proto this way*/
	if (tag==TAG_ProtoNode) return NULL;
	else if (tag==TAG_UndefinedNode) node = NewSFNode();
	else if (tag <= TAG_RANGE_LAST_MPEG4) node = MPEG4_CreateNode(tag);
	else if (tag <= TAG_RANGE_LAST_X3D) node = X3D_CreateNode(tag);
	else if (tag <= TAG_RANGE_LAST_SVG) node = (SFNode *) SVG_CreateNode(tag);
#ifdef M4_USE_LASER
	else if (tag <= TAG_RANGE_LAST_LASeR) node = LASeR_CreateNode(tag);
#endif
	else node = NULL;

	if (node) node->sgprivate->scenegraph = inScene;
	/*script is inited as soon as created since fields are dynamically added*/
	if ((tag==TAG_MPEG4_Script) || (tag==TAG_X3D_Script) ) Script_Init(node);
	return node;
}


M4Err Node_GetField(SFNode *node, u32 FieldIndex, FieldInfo *info)
{
	assert(node);
	assert(info);
	memset(info, 0, sizeof(FieldInfo));
	info->fieldIndex = FieldIndex;

#ifdef NODE_USE_POINTERS
	return node->sgprivate->get_field(node, info);
#else
	if (node->sgprivate->tag==TAG_UndefinedNode) return M4BadParam;
	else if (node->sgprivate->tag == TAG_ProtoNode) return Proto_GetField(NULL, node, info);
	else if ((node->sgprivate->tag == TAG_MPEG4_Script) || (node->sgprivate->tag == TAG_X3D_Script) )
		return Script_GetField(node, info);
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_MPEG4) return MPEG4Node_GetField(node, info);
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_X3D) return X3DNode_GetField(node, info);
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_SVG) return SVG_GetAttributeInfo(node, info);
	else if (node->sgprivate->tag <= TAG_RANGE_LAST_LASeR) return M4NotSupported;
#endif
	return M4NotSupported;
}

/*LASeR specifc, to clean up!!*/
u32 Node_GetActive(SFNode*p)
{
	return p->sgprivate->active;
}
