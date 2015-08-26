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


#ifndef M4_SG_VRML_H_
#define M4_SG_VRML_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/m4_scenegraph.h>

/*
	All extensions for VRML/MPEG-4/X3D graph structure	
*/

/*reserved NDT for MPEG4 (match binary coding)*/
#define MPEG4_RESERVED_NDT		200

/*the NDTs used in X3D not defined in MPEG4*/
enum
{
	NDT_SFMetadataNode = MPEG4_RESERVED_NDT+1,
	NDT_SFFillPropertiesNode,
	NDT_SFX3DLinePropertiesNode,
	NDT_SFGeoOriginNode,
	NDT_SFHAnimNode,
	NDT_SFHAnimDisplacerNode,
	NDT_SFNurbsControlCurveNode,
	NDT_SFNurbsSurfaceNode,
	NDT_SFNurbsCurveNode
};

/*
	VRML / BIFS TYPES DEFINITION
*/

/*
				event types, as defined in the specs 
	this should not be needed by non binary codecs
*/
enum
{
	ET_Field		=	0,
	ET_ExposedField	=	1,
	ET_EventIn		=	2,
	ET_EventOut		=	3,
	ET_Unknown	=	4
};
const char *VRML_GetEventTypeName(u32 EventType, Bool forX3D);

/*
				field coding mode

	BIFS defines the bitstream syntax contextually, and therefore sometimes refer to fields as indexed
  in the node ("all" mode) or just as a sub-set (in, out, def, dyn modes) of similar types
*/
enum
{
	/*all fields and events*/
	FCM_ALL		=	0,
	/*defined fields (exposedField and Field)*/
	FCM_DEF		=	1,
	/*input field (exposedField and eventIn)*/
	FCM_IN		=	2,
	/*output field (exposedField and eventOut)*/
	FCM_OUT		=	3,
	/*field that can be animated (subset of inFields) used in BIFS_Anim only*/
	FCM_DYN		=	4
};

/*get the number of field in the given mode (BIFS specific)*/
u32 Node_GetNumFields(SFNode *Node, u8 IndexMode);

/*		SF Types	*/
typedef Bool SFBool;
typedef s32 SFInt32;
typedef s32 SFInt;
typedef Float SFFloat;
typedef Double SFDouble;

typedef struct
{
	unsigned char* buffer;
} SFString;

typedef Double SFTime;

typedef struct {
	Float	red;
	Float	green;
	Float	blue;
} SFColor;
typedef struct {
	Float	red;
	Float	green;
	Float	blue;
	Float	alpha;
} SFColorRGBA;
typedef struct {
	Float	xAxis;
	Float	yAxis;
	Float	zAxis;
	Float	angle;
} SFRotation;
typedef struct {
	u32 OD_ID;
	char *url;
} SFURL;
typedef struct {
	Float	x;
	Float	y;
} SFVec2f;
typedef struct {
	Double	x;
	Double	y;
} SFVec2d;
typedef struct {
	Float	x;
	Float	y;
	Float	z;
} SFVec3f;
typedef struct {
	Double	x;
	Double	y;
	Double	z;
} SFVec3d;

typedef struct {
	Float	x;
	Float	y;
	Float	z;
	Float	q;
} SFVec4f;
typedef struct {
	u32 width;
	u32 height;
	u8 numComponents;
	unsigned char* pixels;
} SFImage;
typedef struct {
	u32 bufferSize;
	unsigned char* buffer;
	/*uncompressed command list*/
	Chain *commandList;
} SFCommandBuffer;

/*Note on SFScript: the javascript or vrml script is handled in its decompressed (textual) format
since most JS interpreter work with text*/
typedef struct {
	unsigned char* script_text;
} SFScript;


/*		MF Types	*/

/*generic MF field: all MF fields use the same syntax except MFNode which uses Chain. You  can thus use
this structure to safely typecast MF field pointers*/
typedef struct {
	u32 count;
	char *array;
} GenMFField;

typedef struct {
	u32 count;
	s32* vals;
} MFInt32;
typedef struct {
	u32 count;
	s32* vals;
} MFInt;
typedef struct {
	u32 count;
	Float* vals;
} MFFloat;
typedef struct {
	u32 count;
	Double* vals;
} MFDouble;
typedef struct {
	u32 count;
	u32* vals;
} MFBool;
typedef struct {
	u32 count;
	SFColor* vals;
} MFColor;
typedef struct {
	u32 count;
	SFColorRGBA* vals;
} MFColorRGBA;
typedef struct {
	u32 count;
	SFRotation*	vals;
} MFRotation;
typedef struct {
	u32 count;
	Double* vals;
} MFTime;
typedef struct {
	u32 count;
	SFVec2f* vals;
} MFVec2f;
typedef struct {
	u32 count;
	SFVec2d* vals;
} MFVec2d;
typedef struct {
	u32 count;
	SFVec3f* vals;
} MFVec3f;
typedef struct {
	u32 count;
	SFVec3d* vals;
} MFVec3d;
typedef struct {
	u32 count;
	SFVec4f* vals;
} MFVec4f;

typedef struct {
	u32 count;
	SFURL* vals;
} MFURL;
typedef struct {
	u32 count;
	char** vals;
} MFString;
typedef struct {
	u32 count;
	SFScript *vals;
} MFScript;


SFColorRGBA SFColor_ToRGBA(SFColor val);

/*field types, as defined in BIFS encoding (used for scripts and proto coding)*/
enum
{
	FT_SFBool		=	0,
	FT_SFFloat		=	1,
	FT_SFTime		=	2,
	FT_SFInt32		=	3,
	FT_SFString		=	4,
	FT_SFVec3f		=	5,
	FT_SFVec2f		=	6,
	FT_SFColor		=	7,
	FT_SFRotation	=	8,
	FT_SFImage		=	9,
	FT_SFNode		=	SFNodeFieldType,
	/*TO CHECK*/
	FT_SFVec4f		=	11,

	FT_MFBool		=	32,
	FT_MFFloat		=	33,
	FT_MFTime		=	34,
	FT_MFInt32		=	35,
	FT_MFString		=	36,
	FT_MFVec3f		=	37,
	FT_MFVec2f		=	38,
	FT_MFColor		=	39,
	FT_MFRotation	=	40,
	FT_MFImage		=	41,
	FT_MFNode		=	MFNodeFieldType,
	/*TO CHECK*/
	FT_MFVec4f		=	43,

	/*used types in GPAC but not defined in the MPEG4 spec*/
	FT_SFURL			=	50,
	FT_MFURL,
	FT_SFCommandBuffer,
	FT_SFScript,
	FT_MFScript,

	/*used types in X3D*/
	FT_SFDouble,
	FT_SFColorRGBA,
	FT_SFVec2d,
	FT_SFVec3d,
	FT_MFDouble,
	FT_MFColorRGBA,
	FT_MFVec2d,
	FT_MFVec3d,

	FT_Unknown
};
const char *VRML_GetFieldTypeName(u32 FieldType);


/*
allocates a new field and gets it back. 
	NOTE:
			FT_MFNode will return a pointer to a Chain structure (eg Chain *)
			FT_SFNode will return NULL
*/
void *VRML_NewFieldPointer(u32 FieldType);
/*deletes a field pointer (including SF an,d MF nodes)*/
void VRML_DeleteFieldPointer(void *field, u32 FieldType);

Bool VRML_IsSFField(u32 FieldType);

/*translates MF/SF to SF type*/
u32 VRML_GetSFType(u32 FieldType);


/*
	MFField manipulation  - MFNode cannot use these, use the Chain functions instead
	or the Node_* insertion functions
	FieldType shall always be given when manipulating MFFields
*/
/*Insert (+alloc) a slot in the MFField with a specified position for insertion and sets the ptr
to the newly created slot
@InsertAt is the 0-based index for the new slot
*/
M4Err VRML_MF_Insert(void *mf, u32 FieldType, void **new_ptr, u32 InsertAt);
/*adds at the end and gets the ptr*/
M4Err VRML_MF_Append(void *mf, u32 FieldType, void **new_ptr);
/*remove the desired item*/
M4Err VRML_MF_Remove(void *mf, u32 FieldType, u32 RemoveFrom);
/*alloc a fixed array*/
M4Err VRML_MF_Alloc(void *mf, u32 FieldType, u32 NbItems);
/*get the item in the array*/
M4Err VRML_MF_GetItem(void *mf, u32 FieldType, void **new_ptr, u32 ItemPos);
/*remove all items of the MFField*/
M4Err VRML_MF_Reset(void *mf, u32 FieldType);

/*clones a field content EXCEPT SF/MFNode. Pointers to field shall be used
@dest, @orig: pointers to field
@FieldType: type of the field
*/
void VRML_FieldCopy(void *dest, void *orig, u32 FieldType);

/*indicates whether 2 fields of same type EXCEPT SF/MFNode are equal
@dest, @orig: pointers to field
@FieldType: type of the field
*/
Bool VRML_FieldsEqual(void *dest, void *orig, u32 FieldType);



/*VRML grouping nodes macro - note we have inverted the children field to be 
compatible with the base SFParent node
All grouping nodes (with "children" field) implement the following: 

addChildren: chain containing nodes to add passed as eventIn - handled internally through ROUTE
void (*on_addChildren)(SFNode *pNode): add eventIn signaler - this is handled internally by the scene_graph and SHALL 
NOT BE OVERRIDEN since it takes care of node(s) routing

removeChildren: chain containing nodes to remove passed as eventIn - handled internally through ROUTE

void (*on_removeChildren)(SFNode *pNode): remove eventIn signaler - this is handled internally by the scene_graph and SHALL 
NOT BE OVERRIDEN since it takes care of node(s) routing

children: list of children SFNodes
*/

#define VRML_CHILDREN							\
	CHILDREN									\
	Chain *addChildren;							\
	void (*on_addChildren)(SFNode *pNode);		\
	Chain *removeChildren;						\
	void (*on_removeChildren)(SFNode *pNode);		\

typedef struct
{
	BASE_NODE
	VRML_CHILDREN
} VRMLParent;

void SetupVRMLParent(SFNode *pNode);
void DestroyVRMLParent(SFNode *pNode);


/*set proto loader - callback is the same as simulation time callback
	GetExternProtoLib is a pointer to the proto lib loader - this callback shall return the LPSCENEGRAPH
of the extern proto lib if found and loaded, NULL if not found and SG_INTERNAL_PROTO for internal
hardcoded protos (extensions of MPEG-4 scene graph used for module deveopment)
*/
#define SG_INTERNAL_PROTO	(LPSCENEGRAPH) 0xFFFFFFFF

/*Route manip: routes are used to pass events between nodes. Event handling is managed by the scene graph
however only the nodes overloading the EventIn handler associated with the event will process the eventIn*/
typedef struct _route *LPROUTE;

/*creates a new route:
	@fromNode: @fromField: address of the eventOut field triggering the route
	@toNode: @toField: address of the destination eventIn field
NOTE: routes are automatically destroyed if either the target or origin node of the route is destroyed
*/
LPROUTE SG_NewRoute(LPSCENEGRAPH sg, SFNode *fromNode, u32 fromField, SFNode *toNode, u32 toField);

/*delete route*/
void SG_DeleteRoute(LPROUTE route);
M4Err SG_DeleteRouteByID(LPSCENEGRAPH sg,u32 routeID);

/*locate route by ID/name*/
LPROUTE SG_FindRoute(LPSCENEGRAPH sg, u32 RouteID);
LPROUTE SG_FindRouteByName(LPSCENEGRAPH sg, char *name);
/*assign route ID - fails if a route with same ID already exist*/
M4Err SG_SetRouteID(LPROUTE route, u32 ID);
u32 SG_GetRouteID(LPROUTE route);
/*assign route name if desired*/
M4Err SG_SetRouteName(LPROUTE route, char *name);
char *SG_GetRouteName(LPROUTE route);

/*retuns next available RouteID - Note this doesn't track inserted routes, that's the user responsability*/
u32 SG_GetNextAvailableRouteID(LPSCENEGRAPH sg);
/*set max defined route ID used in the scene - used to handle RouteInsert commands
note that this must be called by the user to be effective,; otherwise the max route ID is computed
from the routes present in scene*/
void SG_SetMaxDefinedRouteID(LPSCENEGRAPH sg, u32 ID);


/*activates all routes currently triggered - this follows the event cascade model of VRML/MPEG4:
	- routes are collected during eventOut generation
	- routes are activated. If eventOuts are generated during activation the cycle goes on.

  A route cannot be activated twice in the same simulation tick, hence this function shall be called 
  ONCE AND ONLY ONCE per simulation tick

Note that children scene graphs register their routes with the top-level graph, so only the main 
scene graph needs to be activated*/
void SG_ActivateRoutes(LPSCENEGRAPH sg);


/*
				proto handling

	The lib allows you to construct prototype nodes as defined in VRML/MPEG4 by constructing 
	proto interfaces and instanciating them. An instanciated proto is handled as a single node for
	rendering, thus an application will never handle proto instances for rendering
*/

/*opaque handler for a proto object (declaration)*/
typedef struct _proto *LPPROTO;
/*opaque handler for a proto field object (declaration)*/
typedef struct _protofield *LPPROTOFIELD;


/*retuns next available NodeID*/
u32 SG_GetNextAvailableProtoID(LPSCENEGRAPH sg);

/*proto constructor identified by ID/name in the given scene
2 protos in the same scene may not have the same ID/name

@unregistered: used for memory handling of scene graph only, the proto is not stored
in the graph main proto list but in an alternate list. Several protos with the same ID/Name can be stored unregistered
*/
LPPROTO SG_NewProto(LPSCENEGRAPH inScene, u32 ProtoID, char *name, Bool unregistered);

/*used for memory handling of scene graph only. move proto from off-graph to in-graph or reverse*/
M4Err Proto_SetInGraph(LPPROTO proto, LPSCENEGRAPH inScene, Bool set_in);

/*destroy proto interface - can be used even if instances of the proto are still present*/
M4Err SG_DeleteProto(LPPROTO proto);

/*returns graph associated with this proto. Such a graph cannot be used for rendering but is needed during
construction of proto dictionaries in case of nested protos*/
LPSCENEGRAPH Proto_GetSceneGraph(LPPROTO proto);

/*get/set private data*/
void Proto_SetPrivate(LPPROTO proto, void *ptr, void (*OnDelete)(void *ptr) );
void *Proto_GetPrivate(LPPROTO proto);

/*add node code - a proto is build of several nodes, the first node is used for rendering
and the others are kept private. This set of nodes is refered to as the proto "node code"*/
M4Err Proto_AddNodeCode(LPPROTO proto, SFNode *pNode);

/*gets number of field in the proto interface*/
u32 Proto_GetFieldCount(LPPROTO proto);
/*locates field declaration by name*/
LPPROTOFIELD Proto_FindFieldByName(LPPROTO proto, char *fieldName);
/*locates field declaration by index (0-based)*/
LPPROTOFIELD Proto_FindField(LPPROTO proto, u32 fieldIndex);

/*creates a new field declaration in the proto. of given fieldtype and eventType
fieldName can be NULL, if so the name will be fieldN, N being the index of the created field*/
LPPROTOFIELD Proto_NewField(LPPROTO proto, u32 fieldType, u32 eventType, char *fieldName);

/*assign the node field to a field of the proto (the node field IS the proto field)
the node shall be a node of the proto scenegraph, and the fieldtype/eventType of both fields shall match
(except SF/MFString and MF/SFURL which are allowed) due to BIFS semantics*/
M4Err Proto_SetISField(LPPROTO proto, u32 protoFieldIndex, SFNode *node, u32 nodeFieldIndex);
/*set/get user private data for the proto field declaration*/
void ProtoField_SetPrivate(LPPROTOFIELD field, void *ptr, void (*OnDelete)(void *ptr) );
void *ProtoField_GetPrivate(LPPROTOFIELD field);
/*returns field info of the field - this is typically used to setup the default value of the field*/
M4Err ProtoField_GetField(LPPROTOFIELD field, FieldInfo *info);

/*
	NOTE on proto instances:
		The proto instance is handled as an SFNode outside the scenegraph lib, and is manipulated with the same functions 
		as an SFNode 
		The proto instance may or may not be loaded. 
		An unloaded instance only contains the proto instance fields 
		A loaded instance contains the proto instance fields plus all the proto code (Nodes, routes) and 
		will load any scripts present in it. This allows keeping the memory usage of proto very low, especially
		when nested protos (protos used as building blocks of their parent proto) are used.
*/

/*creates the proto interface without the proto code.*/
SFNode *Proto_CreateInstance(LPSCENEGRAPH sg, LPPROTO proto);

/*lodes code in this instance - all subprotos are automatically created, thus you must only instanciate
top-level protos. VRML/BIFS doesn't allow for non top-level proto instanciation in the main graph
All nodes created in this proto will be forwarded to the app for initialization*/
M4Err Proto_LoadCode(SFNode *proto_inst);

/*locate a prototype definition by ID or by name. when looking by name, ID is ignored*/
LPPROTO SG_FindProto(LPSCENEGRAPH sg, u32 ProtoID, char *name);

/*deletes all protos in given scene - does NOT delete instances of protos, only the proto object is destroyed */
M4Err SG_DeleteAllProtos(LPSCENEGRAPH scene);


/*tools for hardcoded proto*/
/*gets proto of this node - if the ndoe is not a prototype instan,ce, returns NULL*/
LPPROTO Node_GetProto(SFNode *node);
/*returns the ID of the proto*/
u32 Proto_GetID(LPPROTO proto);
/*returns proto name*/
const char *Proto_GetName(LPPROTO proto);

/*Returns 1 if the given field is ISed to a startTime/stopTime field (MPEG-4 specific for updates)*/
Bool Proto_FieldIsSFTimeOffset(SFNode *node, FieldInfo *field);

/*set an ISed field in a proto instance (not a proto) - this is needed with dynamic node creation inside a proto
instance (conditionals)*/
M4Err ProtoInstance_SetISField(SFNode *protoinst, u32 protoFieldIndex, SFNode *node, u32 nodeFieldIndex);

/*
			JavaScript tools
*/

/*script fields type don't have the same value as the bifs ones...*/
enum
{
	SFET_Field = 0,
	SFET_EventIn,
	SFET_EventOut,
};

typedef struct _scriptfield *LPSCRIPTFIELD;
/*creates new sript field - script fields are dynamically added to the node, and thus can be accessed through the
same functions as other SFNode fields*/
LPSCRIPTFIELD SG_NewScriptField(SFNode *script, u32 eventType, u32 fieldType, const char *name);
/*retrieves field info, usefull to get the field index*/
M4Err ScriptField_GetInfo(LPSCRIPTFIELD field, FieldInfo *info);


/*JavaScript interface with user*/
typedef struct
{
	/*user defined callback*/
	void *callback;

	/*file loading callback*/
	Bool (*GetScriptFile)(void *callback, LPSCENEGRAPH parent_graph, const char *url, void (*OnDone)(void *cbck, Bool success, const char *file_cached), void *cbk);

	/*gets option - values for optName:
	"WorldURL" -> returns worldURL
	*/
	const char *(*GetOption)(void *callback, char *optName);
	/*not supported yet*/
	Bool (*SetOption)(void *callback, char *optName, char *optValue);
	/*signals error*/
	void (*Error)(void *callback, const char *msg);
	/*signals message*/
	void (*Print)(void *callback, const char *msg);
	/*ask the app to load a URL*/
	Bool (*LoadURL)(void *callback, const char *url, const char **params, u32 nb_params);
} JSInterface;

/*assign API to scene graph - by default, sub-graphs inherits the API if set*/
void SG_SetJavaScriptAPI(LPSCENEGRAPH scene, JSInterface *ifce);

/*load script into engine - this should be called only for script in main scene, loading of scripts
in protos is done internally when instanciating the proto*/
void Script_Load(SFNode *script);

/*activate eventIn for script node - needed for BIFS field replace*/
void Script_EventIn(SFNode *node, FieldInfo *in_field);

/*returns true if current lib has javascript support*/
Bool SG_HasScripting();

/*
			BIFS command tools
		These are used to store updates in memory without applying changes to the graph, 
	for dumpers, encoders ... Field Indexes are in "ALL" mode
		The commands can then be applied through this lib
*/

/*
		Currently defined possible modifications
*/
enum
{
	SG_SceneReplace = 0,
	SG_NodeReplace,
	SG_FieldReplace, 
	SG_IndexedReplace,
	SG_RouteReplace,
	SG_NodeDelete,
	SG_IndexedDelete,
	SG_RouteDelete,
	SG_NodeInsert,
	SG_IndexedInsert,
	SG_RouteInsert,
	/*extended updates*/
	SG_ProtoInsert,
	SG_ProtoDelete,
	SG_ProtoDeleteAll,
	SG_MultipleReplace,
	SG_MultipleIndexedReplace,
	SG_GlobalQuantizer,
	/*same as NodeDelete, and also updates OrderedGroup.order when deleting a child*/
	SG_NodeDeleteEx,

	SG_UNDEFINED
};


/*
				single command wrapper

  NOTE: In order to maintain node registry, the nodes replaced/inserted MUST be registered with 
  their parents even when the command is never applied. Registering shall be performed 
  with Node_Register (see below).
  If you fail to do so, a node may be destroyed when destroying a command while still used
  in another command or in the graph - this will just crash.
*/

/*structure used to store field info, pos and static pointers to SFNode/MFNode in commands*/
typedef struct
{
	u32 fieldIndex;
	/*field type*/
	u32 fieldType;
	/*field pointer for multiple replace/ multiple indexed replace - if multiple indexed replace, must be the SF field being changed*/
	void *field_ptr;
	/*replace/insert/delete pos - -1 is append except in multiple indexed replace*/
	s32 pos;

	/*Whenever field pointer is of type SFNode, store the node here and set the far pointer to this address.*/
	SFNode *new_node;
	/*Whenever field pointer is of type MFNode, store the node list here and set the far pointer to this address.*/
	Chain *node_list;
} CommandFieldInfo;

typedef struct
{
	LPSCENEGRAPH in_scene;
	u32 tag;

	/*scene replace command: 
		root node is stored in com->node
		protos are stored in com->new_proto_list
		routes are stored as RouteInsert in the same frame
	*/
	Bool use_names;


	/*node the command applies to - may be NULL*/
	SFNode *node;

	/*list of CommandFieldInfo for all field commands replace/ index insert / index replace / index delete / MultipleReplace / MultipleIndexedreplace 
	the content is destroyed when deleting the command*/
	Chain *command_fields;
	
	/*route insert, replace and delete*/
	u32 RouteID;
	char *def_name;
	u32 fromNodeID, fromFieldIndex;
	u32 toNodeID, toFieldIndex;

	/*proto list to insert*/
	Chain *new_proto_list;
	/*proto ID list to delete*/
	u32 *del_proto_list;
	u32 del_proto_list_size;

	/*may be NULL, and may be present with any command inserting a node*/
	Chain *scripts_to_load;
	/*for authoring purposes - must be cleaned by user*/
	Bool unresolved;
	char *unres_name;
} SGCommand;


/*creates command - graph only needed for SceneReplace*/
SGCommand *SG_NewCommand(LPSCENEGRAPH in_scene, u32 tag);
/*deletes command*/
void SG_DeleteCommand(SGCommand *com);
/*apply command to graph - the command content is kept unchanged for authoring purposes - THIS NEEDS TESTING AND FIXING
@time_offset: offset for time fields if desired*/
M4Err SG_ApplyCommand(LPSCENEGRAPH inScene, SGCommand *com, Double time_offset);
/*apply list if command to graph - the command content is kept unchanged for authoring purposes
@time_offset: offset for time fields if desired*/
M4Err SG_ApplyCommandList(LPSCENEGRAPH graph, Chain *comList, Double time_offset);
/*returns new commandFieldInfo structure and registers it with command*/
CommandFieldInfo *SG_NewFieldCommand(SGCommand *com);
/*clones the command in another graph - needed for uncompressed conditional in protos*/
SGCommand *SG_CloneCommand(SGCommand *com, LPSCENEGRAPH inGraph);

u32 Node_GetActive(SFNode*);

void SG_SetProtoLoader(LPSCENEGRAPH scene, LPSCENEGRAPH (*GetExternProtoLib)(void *SceneCallback, MFURL *lib_url));

/*get a pointer to the MF URL field for externProto info - DO NOT TOUCH THIS FIELD*/
MFURL *Proto_GetExternURLFieldPointer(LPPROTO proto);

SFRotation Rotation_Interpolate(SFRotation kv1, SFRotation kv2, Float fraction);




/*adds a new node to the "children" field
position is the 0-BASED index in the list of children, -1 means end of list (append)
DOES NOT CHECK CHILD/PARENT type compatibility
*/
M4Err Node_InsertChild(SFNode *parent, SFNode *new_child, s32 Position);
/*removes an existing node from the "children" field*/
M4Err Node_RemoveChild(SFNode *parent, SFNode *toremove_child);
/*remove and replace given child by specified node. If node is NULL, only delete target node
position is the 0-BASED index in the list of children, -1 means end of list (append)
DOES NOT CHECK CHILD/PARENT type compatibility
*/
M4Err Node_ReplaceChild(SFNode *node, Chain *container, s32 pos, SFNode *newNode);

/*signals eventOut has been set. FieldIndex/eventName identify the eventOut field. Routes are automatically triggered
when the event is signaled*/
void Node_OnEventOut(SFNode *node, u32 FieldIndex);
void Node_OnEventOutSTR(SFNode *node, const char *eventName);


/*exported for parsers*/
u32 MPEG4_GetTagByName(const char *node_name);
u32 X3D_GetTagByName(const char *node_name);

/*NDT check - return 1 if node belongs to given NDT. Handles proto, and assumes undefined nodes
always belong to the desired NDT*/
Bool Node_IsInTable(SFNode *node, u32 NDTType);

#ifdef __cplusplus
}
#endif



#endif /*M4_SCENEGRAPH_H_*/
