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


#ifndef M4_SCENEGRAPH_H_
#define M4_SCENEGRAPH_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <m4_tools.h>

//#define M4_USE_LASeR

/*
	TAG definitions are static, in order to be able to mix nodes from different standard
	in a single scenegraph. These TAGs are only used internally (they do not match any
	binary encoding)
*/
enum {
	/*no node shall use this tag*/
	TAG_ForbiddenZero = 0,
	/*undefined node: just the base node class, used for parsing*/
	TAG_UndefinedNode = 1,
	/*all proto instances have this tag*/
	TAG_ProtoNode,

	/*reserved TAG ranges per standard*/

	/*range for MPEG4*/
	TAG_RANGE_FIRST_MPEG4,
	TAG_RANGE_LAST_MPEG4 = TAG_RANGE_FIRST_MPEG4+512,
	/*range for X3D*/
	TAG_RANGE_FIRST_X3D, 
	TAG_RANGE_LAST_X3D = TAG_RANGE_FIRST_X3D+512,
	/*range for SVG*/
	TAG_RANGE_FIRST_SVG, 
	TAG_RANGE_LAST_SVG = TAG_RANGE_FIRST_SVG+512,
	/*range for LASeR*/
	TAG_RANGE_FIRST_LASeR, 
	TAG_RANGE_LAST_LASeR = TAG_RANGE_FIRST_LASeR+128,
};



/*private handler for this library on all nodes*/
#define BASE_NODE	struct _nodepriv *sgprivate;

/*base node type*/
typedef struct _sfNode
{
	BASE_NODE
} SFNode;

/*grouping nodes macro :
	children: list of children SFNodes
*/

#define CHILDREN									\
	Chain *children;

typedef struct
{
	BASE_NODE
	CHILDREN
} SFParent;

/*tag is set upon creation and cannot be modified*/
u32 Node_GetTag(SFNode*);
/*set node def
	@ID: node ID, !=0 set def node - if a different node with the same ID exists, returns error. 
You may change the node ID by recalling the function with a different ID value. You may get a node ID
by calling the SG_GetNextAvailableNodeID function
	@defName: optional readable name (script, MPEGJ). To change the name, recall the function with a different name and the same ID
*/
M4Err Node_SetDEF(SFNode*n, u32 nodeID, const char *nodeDEFName);
/*get def name of the node , NULL if not set*/
const char *Node_GetDefName(SFNode*);
/*get def ID of the node, 0 if node not def*/
u32 Node_GetID(SFNode*);
/* gets node built-in name (eg 'Appearance', ..) */
const char *Node_GetName(SFNode *Node);

/*get/set user private stack*/
void *Node_GetPrivate(SFNode*);
void Node_SetPrivate(SFNode*, void *);

/*set rendering function. When rendering a scene graph, the render stack is passed
through the graph without being touched. If a node has no associated RenderNode(), the traversing of the 
graph won't propagate below it. It is the app responsability to setup traversing functions as needed
VRML/MPEG4:  Instanciated Protos are handled internally as well as interpolators, valuators and scripts
*/
M4Err Node_SetRenderFunction(SFNode *, void (*RenderNode)(SFNode *node, void *render_stack) );
/*set pre-destroy function in order to delete any private data*/
M4Err Node_SetPreDestroyFunction(SFNode *, void (*PreDestroyNode)(struct _sfNode *node) );

/*register a node (DEFed or not), specifying parent if any.
A node must be registered whenever used by something (a parent node, a command, whatever) to prevent its 
destruction (think of it as a reference counting).
NOTE: NODES ARE CREATED WITHOUT BEING REGISTERED
*/
M4Err Node_Register(SFNode *node, SFNode *parent_node);

/*unregister a node from parent (node may or not be DEF'ed). Parent may be NULL (DEF root node, commands).
This MUST be called whenever a node is destroyed (removed from a parent node)
If this is the last instance of the node, the node is destroyed
NOTE: NODES ARE CREATED WITHOUT BEING REGISTERED, hence they MUST be registered at least once before 
being destroyed
*/
M4Err Node_Unregister(SFNode *node, SFNode *parent_node);
/*deletes all node instances in the given list*/
void Node_UnregisterChildren(SFNode *node, Chain *childrenlist);

/*get all parents of the node and replace the old_node by the new node in all parents
Note: if the new node is not DEFed, only the first instance of "old_node" will be replaced, the other ones deleted*/
M4Err Node_ReplaceAllInstances(SFNode *old_node, SFNode *new_node, Bool updateOrderedGroup);


/*calls RenderNode on this node*/
void Node_Render(SFNode *node, void *renderStack);
/*blindly calls RenderNode on all nodes in the "children" list*/
void Node_RenderChildren(SFNode *node, void *renderStack);;
/*returns number of parent for this node (parent are kept regardless of DEF state)*/
u32 Node_GetParentCount(SFNode *node);
/*returns desired parent for this node (parent are kept regardless of DEF state)
idx is 0-based parent index*/
SFNode *Node_GetParent(SFNode *node, u32 idx);


enum
{
	/*flag set whenever a field of the node has been modified*/
	SG_NODE_DIRTY = 1,
	/*flag set whenever a child node of this node has been modified
	NOTE: unloaded extern protos always invalidate their parent subgraph to get a chance
	of being loaded. It is the user responsability to clear the CHILD_DIRTY flag before traversing
	if relying on this flag for sub-tree discarding (eg, culling or similar)*/
	SG_CHILD_DIRTY = 1<<1,
};

/*set dirty flag on (SG_NODE_DIRTY).
If invalidate_parents is set, all parent subtrees nodes are marked as SG_CHILD_DIRTY
Note: parent subtree marking aborts if a node in the subtree is already marked with SG_CHILD_DIRTY*/
void Node_SetDirty(SFNode *node, Bool invalidate_parents);
/*set dirty flag off. It is the user responsability to clear dirty flags*/
void Node_ClearDirty(SFNode *node);
/*same as ClearDirty, but also clears all subtrees
if skip_if_clean is set and node is not dirty, children are not cleared*/
void Node_ClearDirtyChildren(SFNode *node, Bool skip_if_clean);
/*get dirty flag value*/
u16 Node_GetDirty(SFNode *node);
/*removes given flag from dirty flag list*/
void Node_ClearDirtyFlag(SFNode *node, u16 flag_to_remove);
/*forces assignment of dirty flag list*/
void Node_SetDirtyFlag(SFNode *node, u16 val);


/*eventIn handlers definition. Although this is VRML specific, this can be used by any std
If you wish to handle a given eventIn, overwrite the handler of the field by your specific call*/
typedef void (*OnEventIn)(SFNode *pNode);

/*Notes on FieldInfo
all scene graph implementations should answer node field query with this interface. 
In case an implementation does not use this:
	- the implementation shall handle the parent node dirty flag itself most of the time
	- the implementation shall NOT allow referencing of a graph node in a parent graph node (when inlining
content) otherwise the app is guaranteed to crash.
*/

/*these are the 2 fieldtypes that shall be used to identify:*/
#define SFNodeFieldType		10		/*field points to a single node*/
#define MFNodeFieldType		42		/*field points to a list (Chain*) of nodes*/
/*these values SHALL NEVER BE CHANGED as they match BIFS bitstream syntax*/

/*other fieldTypes may be ignored by implmentation not using VRML/MPEG4 native types*/

typedef struct
{	
	/*0-based index of the field in the node*/
	u32 fieldIndex;
	/*field type - VRML/MPEG4 types are listed below*/
	u32 fieldType;
	/*far ptr to the field (eg SFNode **, Chain**, MFInt32 *, ...)*/
	void *far_ptr;
	/*field name*/
	const char *name;
	/*NDT type in case of SF/MFNode field - cf BIFS specific tools*/
	u32 NDTtype;
	/*event type*/
	u32 eventType;
	/*eventin handler if any*/
	OnEventIn on_event_in;
} FieldInfo;

/*returns number of field for this node*/
u32 Node_GetFieldCount(SFNode *node);

/*fill the field info structure for the given field*/
M4Err Node_GetField(SFNode *node, u32 FieldIndex, FieldInfo *info);

/*get the field by its name*/
M4Err Node_GetFieldByName(SFNode *node, char *name, FieldInfo *field);


/*opaque handler for the scene graph object*/
typedef struct _tagSceneGraph *LPSCENEGRAPH;

/*scene graph constructor*/
LPSCENEGRAPH NewSceneGraph();

/*creates a sub scene graph (typically used with Inline node): independent graph with same private stack, 
and user callbacks as parent. All routes triggered in this subgraph are executed in the parent graph (this 
means you only have to activate routes on the main graph)
NOTE: the resulting graph is not destroyed when the parent graph is 
*/
LPSCENEGRAPH SG_NewSubScene(LPSCENEGRAPH scene);

/*destructor*/
void SG_Delete(LPSCENEGRAPH sg);
/*reset the full graph - all nodes, routes and protos are destroyed*/
void SG_Reset(LPSCENEGRAPH sg);

/*set the scene timer (fct returns time in sec)*/
void SG_SetSceneTimeCallback(LPSCENEGRAPH scene, Double (*GetSceneTime)(void *SceneCallback), void *SceneCallback);
/*set node init callback: function called upon node creation. 
Application should instanciate the node rendering stack and any desired callback*/
void SG_SetInitCallback(LPSCENEGRAPH sg, void (*UserNodeInit)(void *NodeInitCallback, SFNode *newNode), void *NodeInitCallback);
/*set node modified callback: any modification on nodes can be notified so that an application 
can decide whether the scene must be redrawned or not. You typically will set the dirty flags here*/
void SG_SetModifiedCallback(LPSCENEGRAPH sg, void (*UserNodeModified)(void *NodeModifiedCallback, SFNode *newNode), void *NodeModifiedCallback);

/*set/get user private data*/
void SG_SetPrivate(LPSCENEGRAPH sg, void *user_priv);
void *SG_GetPrivate(LPSCENEGRAPH sg);

/*get/set the root node of the graph*/
SFNode *SG_GetRootNode(LPSCENEGRAPH sg);
void SG_SetRootNode(LPSCENEGRAPH sg, SFNode *node);

/*finds a registered node by ID*/
SFNode *SG_FindNode(LPSCENEGRAPH sg, u32 nodeID);
/*finds a registered node by DEF name*/
SFNode *SG_FindNodeByName(LPSCENEGRAPH sg, char *name);

/*used to signal modification of a node, indicating which field is modified - exposed for BIFS codec, 
should not be needed by other apps*/
void SG_NodeChanged(SFNode *node, FieldInfo *fieldChanged);

/*returns the graph this node belongs to*/
LPSCENEGRAPH Node_GetParentGraph(SFNode *node);

/*Set size info for the graph - by default graphs have no size and are in meter metrics (VRML like)
if any of width or height is 0, the graph has no size info*/
void SG_SetSizeInfo(LPSCENEGRAPH sg, u32 width, u32 Height, Bool usePixelMetrics);
/*returns 1 if pixelMetrics*/
Bool SG_UsePixelMetrics(LPSCENEGRAPH sg);
/*returns 0 if no size info, otherwise 1 and set width/height*/
Bool SG_GetSizeInfo(LPSCENEGRAPH sg, u32 *width, u32 *Height);

/*creates a node of the given tag. sg is the parent scenegraph of the node,
eg the root one for scene nodes or the proto one for proto code (cf proto)
Note:
	- NODE IS NOT REGISTERED (no instances) AND CANNOT BE DESTROYED UNTIL REGISTERED
	- this doesn't perform application setup for the node, this must be done by the caller
*/
SFNode *SG_NewNode(LPSCENEGRAPH sg, u32 tag);
/*clones a node in the given graph and register with parent cloned. The cloning respects DEF/USE nodes*/
SFNode *SG_CloneNode(LPSCENEGRAPH inScene, SFNode *orig, SFNode *cloned_parent);
/*inits node (either internal stack or user-defined) - usually called once the node has been fully loaded*/
void Node_Init(SFNode *node);
/*gets scene time for scene this node belongs too, 0 if timeline not specified*/
Double Node_GetSceneTime(SFNode *node);


/*retuns next available NodeID*/
u32 SG_GetNextAvailableNodeID(LPSCENEGRAPH sg);
/*retuns max ID used in graph*/
u32 SG_GetMaxNodeID(LPSCENEGRAPH sg);

/*set max number of self-traversing per node for cyclic graphs: 0 or 1 means one traverse only
this value is used for all nodes in the graph*/
void SG_SetCyclicRenderLimit(LPSCENEGRAPH sg, u16 max_cycle);
/*returns TRUE if this node is traversed for the first time in a cyclic subtree*/
Bool Node_IsFirstCyclicRender(SFNode *n);
/*set max number of self-traversing per node for cyclic graphs: 0 or 1 means one traverse only
this value isonly used by the node itself*/
void Node_SetCyclicRenderLimit(SFNode *n, u16 max_cycle);


#ifdef __cplusplus
}
#endif



#endif /*M4_SCENEGRAPH_H_*/


