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



#ifndef _M4_SCENEGRAPH_DEV_H
#define _M4_SCENEGRAPH_DEV_H

//#define NODE_USE_POINTERS

/*for vrml base types, ROUTEs and PROTOs*/
#include <m4_scenegraph_vrml.h>


void Node_Setup(SFNode *p, u32 tag);

typedef struct _nodepriv
{
	/*node type*/
	u32 tag;
	/*node ID or 0*/
	u32 NodeID;

	u16 is_dirty;
	u8 active;

#ifdef NODE_USE_POINTERS
	const char *name;
	u32 (*get_field_count)(struct _sfNode *node, u8 IndexMode);
	void (*node_del)(struct _sfNode *node);
	M4Err (*get_field) (struct _sfNode *node, FieldInfo *info);

	/*static field translation table*/
	M4Err (*get_field_index)(struct _sfNode *node, u32 inField, u8 IndexMode, u32 *allField);
	/*get quantif and anim params - returns 0 if no AQ */
	Bool (*get_aq_info)(struct _sfNode *node, u32 FieldIndex, u8 *QType, u8 *AType, Float *b_min, Float *b_max, u32 *QT13_bits);
#endif

	/*node def name (MPEGJ interfaces, VRML/X3D)*/
	char *NodeName;
	/*scenegraph holding the node*/
	struct _tagSceneGraph *scenegraph;
	/*routes on eventOut*/
	Chain *outRoutes;
	/*user defined rendering function */
	void (*RenderNode)(struct _sfNode *node, void *render_stack);
	/*user defined pre-destroy function */
	void (*PreDestroyNode)(struct _sfNode *node);
	/*user defined stack*/
	void *privateStack;


	/*
		DEF/USE (implicit or explicit) handling - implicit DEF do NOT use the parentNodes list
		
		NOTE on DEF/USE: in VRML a node is DEF and then USE, but in MPEG4 a node can be deleted, replaced,
		a USE can be inserted before (in scene graph depth) the DEF, etc.
		this library considers that a DEF node is valid until all instances are deleted. If so the node is removed
		from the scene graph manager
	*/

	/*number of instances of this node in the graph*/
	u32 num_instances;
	/*list of all parent nodes (whether DEF or not, needed to invalidate parent tree)*/
	Chain *parentNodes;

	u32 render_pass;
} NodePriv;


#ifndef NODEREG_STEP_ALLOC
#define NODEREG_STEP_ALLOC	50
#endif

typedef struct _tagSceneGraph 
{
	/*all DEF nodes (explicit)*/
	SFNode **node_registry;
	u32 node_reg_alloc, node_reg_size;


	/*all routes available*/
	Chain *Routes;

	/*when a proto is instanciated it creates its own scene graph. BIFS/VRML specify that the namespace is the same 
	(eg cannot reuse a NodeID or route name/ID), but this could be done differently by some other stds
	if NULL this is the main scenegraph*/
	struct _proto_instance *pOwningProto;

	/*all first-level protos of the graph (the only ones that can be instanciated in this graph)*/
	Chain *protos;
	/*all first-level protos of the graph not currently registered - memory handling of graph only*/
	Chain *unregistered_protos;

	/*pointer to the root node*/
	SFNode *RootNode;

	/*routes to be activated (cascade model). This is used at the top-level graph only (eg
	proto routes use that too, ecept ISed fields). It is the app responsability to 
	correctly connect or browse scene graphs connected through Inline*/
	Chain *routes_to_activate;

	/*since events may trigger deletion of objects we use a 2 step delete*/
	Chain *routes_to_destroy;

	/*node init callback*/
	void (*UserNodeInit)(void *appCallback, SFNode *newNode);
	void *NodeInitCallback;
	/*real scene time callback*/
	Double (*GetSceneTime)(void *scene_callback);
	void *SceneCallback;

	u32 simulation_tick;

	/*node modification callback*/
	void (*NodeModified)(void *ModifCallback, SFNode *node);
	void *ModifCallback;


	LPSCENEGRAPH (*GetExternProtoLib)(void *SceneCallback, MFURL *lib_url);

	/*user private data*/
	void *userpriv;

	/*parent scene if any*/
	struct _tagSceneGraph *parent_scene;

	/*size info and pixel metrics - this is not used internally, however it helps when rendering
	and decoding modules don't know each-other (as in MPEG4)*/
	u32 width, height;
	Bool usePixelMetrics;

	/*application interface for javascript*/
	JSInterface *js_ifce;
	/*script loader*/
	void (*Script_Load)(SFNode *node);

	u32 max_defined_route_id;

	/*max number of render() for cyclic graphs*/
	u32 max_cyclic_render;
} SceneGraph;


void SetupChildrenNode(SFNode *pNode);
void DestroyChildrenNode(SFNode *pNode);


typedef struct _route
{
	u32 ID;
	char *name;
	
	/*NB: this assumes a node is never replaced with a # node with same ID without deleting the associated route*/
	SFNode *FromNode;
	u32 FromFieldIndex;
	FieldInfo FromField;

	SFNode *ToNode;
	u32 ToFieldIndex;
	FieldInfo ToField;

	/*this is just a hint for speed up*/
	const char *fromFieldName;

	/*scope of this route*/
	SceneGraph *graph;


	/*set to true for proto IS fields*/
	Bool IS_route;

	u32 lastActivateTime;
	Bool is_setup;
} Route;

void RemoveActivatedRoute(LPSCENEGRAPH sg, Route *r);
/*returns TRUE if route modified destination node*/
Bool ActivateRoute(Route *r);
void QueueRoute(SceneGraph *pSG, Route *r);
/*frees memory*/
void Route_Destroy(LPROUTE r);

void SG_DestroyRoutes(LPSCENEGRAPH sg);


/*MPEG4 def*/
SFNode *MPEG4_CreateNode(u32 NodeTag);
#ifndef NODE_USE_POINTERS
u32 MPEG4Node_GetChildNDT(SFNode *node);
Bool MPEG4Node_GetAQInfo(SFNode *node, u32 FieldIndex, u8 *QType, u8 *AType, Float *b_min, Float *b_max, u32 *QT13_bits);
M4Err MPEG4Node_GetField(SFNode *node, FieldInfo *field);
M4Err MPEG4Node_GetFieldIndex(SFNode *node, u32 inField, u8 code_mode, u32 *fieldIndex);
u32 MPEG4Node_GetFieldCount(SFNode *node, u8 code_mode);
void MPEG4_Node_Del(SFNode *node);
const char *MPEG4_GetNodeName(u32 NodeTag);
#endif

/*X3D def*/
SFNode *X3D_CreateNode(u32 NodeTag);
#ifndef NODE_USE_POINTERS
M4Err X3DNode_GetField(SFNode *node, FieldInfo *field);
u32 X3DNode_GetFieldCount(SFNode *node);
void X3D_Node_Del(SFNode *node);
const char *X3D_GetNodeName(u32 NodeTag);
#endif


void MFInt32_Del(MFInt32 par);
void MFInt_Del(MFInt par);
void MFFloat_Del(MFFloat par);
void MFDouble_Del(MFDouble par);
void MFBool_Del(MFBool par);
void MFColor_Del(MFColor par);
void MFColorRGBA_Del(MFColorRGBA par);
void MFRotation_Del(MFRotation par);
void MFString_Del(MFString par);
void MFTime_Del(MFTime par);
void MFVec2f_Del(MFVec2f par);
void MFVec3f_Del(MFVec3f par);
void MFVec2d_Del(MFVec2d par);
void MFVec3d_Del(MFVec3d par);
void MFURL_Del(MFURL url);
void SFImage_Del(SFImage im);
void SFString_Del(SFString par);
void SFScript_Del(SFScript sc);
void MFScript_Del(MFScript sc);
void SFCommandBuffer_Del(SFCommandBuffer cb);
void SFURL_Del(SFURL url);

Bool VRML_NodeInit(SFNode *node);
Bool VRML_NodeChanged(SFNode *node, FieldInfo *field);


//
//		MF Fields tools
//	WARNING: MF / SF Nodes CANNOT USE THESE FUNCTIONS
//

//return the size (in bytes) of fixed fields (buffers are handled as a char ptr , 1 byte)
u32 GetSFFieldSize(u32 FieldType);



void DestroyNode(SFNode *node);

//BASE node (SFNode) destructor
void SFNode_Delete(SFNode *node);

/*adds a new node to the given MFNode field
position is the 0-BASED index in the list of children
-1 means end of list (append)
REGISTERING MUST BE PERFORMED BY CALLER
*/
M4Err InsertSFNode(void *mfnode_far_ptr, SFNode *new_child, s32 Position);

//these 2 functions are used when deleting the nodes (DESTRUCTORS ONLY), because the 
//info about DEF ? USE is stored somewhere else (usually, BIFS codec or XMT parser)
//the parent node is used to determined the acyclic portions of the scene graph
void NodeList_Delete(Chain *children, SFNode *parent);

/*creates an undefined SFNode - for parsing only*/
SFNode *NewSFNode();

void SFColor_fromHSV(SFColor *col);
void SFColor_toHSV(SFColor *col);

/*returns field type from its name*/
u32 GetFieldTypeByName(char *fieldType);



/*
			Proto node

*/

/*field interface to codec. This is used to do the node decoding, index translation
and all QP/BIFS Anim parsing. */
typedef struct _protofield
{
	u8 EventType;
	u8 FieldType;
	/*if UseName, otherwise fieldN*/
	char *FieldName;

	/*default field value*/
	void *default_value;

	/*coding indexes*/
	u32 IN_index, OUT_index, DEF_index, ALL_index;

	/*Quantization*/
	u32 QP_Type, hasMinMax;
	void *qp_min_value, *qp_max_value;
	/*this is for QP=13 only*/
	u32 NumBits;

	/*Animation*/
	u32 Anim_Type;

	void *userpriv;
	void (*OnDelete)(void *ptr);
} ProtoFieldInterface;

ProtoFieldInterface *Proto_NewFieldInterface(u32 FieldType);


/*proto field instance. since it is useless to duplicate all coding info, names and the like
we seperate proto declaration and proto instanciation*/
typedef struct 
{
	u8 EventType;
	u8 FieldType;
	void *field_pointer;
} ProtoField;


typedef struct _proto
{
	/*1 - Prototype interface*/
	u32 ID;
	char *Name;
	Chain *proto_fields;

	/*pointer to parent scene graph*/
	struct _tagSceneGraph *parent_graph;
	/*pointer to proto scene graph*/
	struct _tagSceneGraph *sub_graph;

	/*2 - proto implementation as declared in the bitstream*/
	Chain *node_code;

	/*num fields*/
	u32 NumIn, NumOut, NumDef, NumDyn;

	void *userpriv;
	void (*OnDelete)(void *ptr);

	/*URL of extern proto lib (if none, URL is empty)*/
	MFURL ExternProto;

	/*list of instances*/
	Chain *instances;
} PrototypeNode;

/*create proto interface (never instanciated)*/
PrototypeNode *Proto_NewInterface();

/*proto field API*/
u32 Proto_GetNumFields(SFNode *node, u8 code_mode);
M4Err Proto_GetField(PrototypeNode *proto, SFNode *node, FieldInfo *field);


typedef struct _proto_instance
{
	/*this is a node*/
	BASE_NODE

	/*Prototype interface for coding and field addressing*/
	PrototypeNode *proto_interface;

	/*proto implementation at run-time (aka the state of the nodes may differ accross
	different instances of the proto)*/
	Chain *fields;

	/*a proto doesn't have one root SFnode but a collection of nodes for implementation*/
	Chain *node_code;

	/*node for proto rendering, first of all declared nodes*/
	SFNode *RenderingNode;

#ifndef NODE_USE_POINTERS
	/*in case the PROTO is destroyed*/
	char *proto_name;
#endif

	/*scripts are loaded once all IS routes are activated and node code is loaded*/
	Chain *scripts_to_load;

	Bool is_loaded;
} ProtoInstance;

/*destroy proto*/
void Proto_DeleteInstance(ProtoInstance *inst);
M4Err Proto_GetFieldIndex(ProtoInstance *proto, u32 index, u32 code_mode, u32 *all_index);
Bool Proto_GetAQInfo(SFNode *Node, u32 FieldIndex, u8 *QType, u8 *AType, Float *b_min, Float *b_max, u32 *QT13_bits);
M4Err protoinst_get_field_ind(SFNode *Node, u32 inField, u8 IndexMode, u32 *allField);
SFNode *Proto_CreateNode(LPSCENEGRAPH scene, PrototypeNode *proto, ProtoInstance *from_inst);
void proto_instanciate(ProtoInstance *proto_node);

/*get tag of first node in proto code - used for validation only*/
u32 Proto_GetRenderingTag(LPPROTO proto);


/*to call when a proto field has been modified (at creation or through commands, modifications through events 
are handled internally).
node can be the proto instance or a node from the proto code
this will call NodeChanged if needed, forward to proto/node or trigger any route if needed*/
void Proto_CheckFieldChanged(SFNode *node, u32 fieldIndex);

void RenderProtoInstance(SFNode *node, void *renderstack);


/*
		Script node
*/

#ifdef M4_USE_SPIDERMONKEY

/*WIN32 and WinCE config (no configure script)*/
#if defined(WIN32) || defined(_WIN32_WCE)
#ifndef XP_PC
#define XP_PC
#endif
/*WINCE specific config*/
#if defined(_WIN32_WCE)
#define XP_WINCE
#endif
#endif

/*other platforms should be setup through configure*/

#include <jsapi.h> 

#endif

typedef struct 
{
	//extra script fields
	Chain *fields;

	//BIFS coding stuff
	u32 numIn, numDef, numOut;

	/*pointer to original tables*/
#ifdef NODE_USE_POINTERS
	M4Err (*Script_GetField)(SFNode *node, FieldInfo *info);
	M4Err (*Script_GetFieldIndex)(SFNode *node, u32 inField, u8 IndexMode, u32 *allField);
#endif


#ifdef M4_USE_SPIDERMONKEY
	JSContext *js_ctx;
	struct JSObject *js_obj;
	struct JSObject *js_browser;
	/*all attached objects (eg, not created by the script) are stored here so that we don't
	allocate them again and again and again when getting properties...*/
	Chain *obj_bank;
#endif

	void (*JS_PreDestroy)(SFNode *node);
	void (*JS_EventIn)(SFNode *node, FieldInfo *in_field);

	Bool is_loaded;
} ScriptPriv;

/*setup script stack*/
void Script_Init(SFNode *node);
/*get script field*/
M4Err Script_GetField(SFNode *node, FieldInfo *info);
/*get effective field count per event mode*/
u32 Script_GetNumFields(SFNode *node, u8 IndexMode);
/*translate field index from inMode to ALL mode*/
M4Err Script_GetFieldIndex(SFNode *Node, u32 inField, u8 IndexMode, u32 *allField);
/*create dynamic fields in the clone*/
M4Err Script_PrepareClone(SFNode *dest, SFNode *orig);

typedef struct _scriptfield
{
	u32 eventType;
	u32 fieldType;
	char *name;

	s32 IN_index, OUT_index, DEF_index;
	u32 ALL_index;

	//real field
	void *pField;

	Double last_route_time;

	Bool activate_event_out;
} ScriptField;


#ifdef M4_USE_SPIDERMONKEY
void init_spidermonkey_api(ScriptPriv *sc, SFNode *script);
JSBool eventOut_setProperty(JSContext *c, JSObject *obj, jsval id, jsval *val);

typedef struct 
{
	FieldInfo field;
	SFNode *owner;

	/*JS list for MFFields or NULL*/
	JSObject *js_list;

	/*when creating SFnode from inside the script, the node is stored here untill attached to an object*/
	SFNode *temp_node;
	Chain *temp_list;
	/*when not owned by a node*/
	void *field_ptr;
} JSField;

void JS_ToNodeField(JSContext *c, jsval v, FieldInfo *field, SFNode *owner, JSField *parent);
jsval JS_ToJSField(ScriptPriv *priv, FieldInfo *field, SFNode *parent);


#endif


#endif	
