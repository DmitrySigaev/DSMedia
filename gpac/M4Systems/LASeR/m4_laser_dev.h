#ifndef _M4_LASER_DEV_H
#define _M4_LASER_DEV_H

/*for BIFS last defined tag , m4_tools & #define M4_USE_LASeR*/
#include <gpac/m4_scenegraph.h>

/* Primitive types */
typedef struct {
	Float r, g, b;
} LASeRColor;

typedef struct {
	Float xx, xy, yx, yy, xz, yz;
} LASeRMatrix;

typedef struct {
	u32 length;
	u8 *string;
} LASeRString;

typedef struct {
	u32 nbPoints;
	Float *x, *y;
} LASeRPointSequence;

typedef struct {
	u8 keyCode;
	Float MoveX, MoveY;
} LASeRKey;

typedef struct {
	LASeRString name;
	u32 source;
} LASeRUrlParam;

typedef struct {
	u8 hasHideObject;
	u32 hideObject;

	u8 hasShowObject;
    u32 showObject;

	u8 hasAnimObject;
    u32 animObject;
  
	u8 hasMoveObject;
	LASeRMatrix moveObject;
    u8 absolute;

	u8 hasPointer;

    Float pointerCenterX, pointerCenterY;
    Float pointerSizeX, pointerSizeY;

	u8 hasTextToChange;

    u32 textToChange;
    LASeRString newText;
  
	u8 hasConcat;
	u32 concatReadID, concatWriteID;  
} LASeRActionKey;

typedef struct {
	u8 hasMatrix;
	LASeRMatrix m;
    Float deltax, deltay;
} LASeRTransformKey;

typedef struct {
	LASeRColor color;
} LASeRColorKey;

typedef struct {
	u32 animObject;
} LASeRActivateKey;

typedef struct {
	u8 type;
	void *update;
} LASeRUpdate;

/* General structures */
typedef struct {
	
	u8 idBits, lenBits, colorBits, scaleBits;
	u8 colorIndexBits, fontIndexBits;

	Float resolution;

	u8 append;
	u8 normalized;
	
	u16 width, height;

	u8 keyLength;
	u8 *key;

	u8 forceKey;

	LASeRString urlPrefix;
	LASeRString name;

	u8 nbColors;
	LASeRColor *colors;

	u8 nbFonts;
	LASeRString *fontNames;

} LASeRSceneHeader;

/* LASeR Nodes */ 
typedef struct {
	BASE_NODE
} LASeRNode;

typedef struct {
	BASE_NODE
	CHILDREN
} LASeRParent;

enum 
{
	/*and are "0-based in bitstream*/
	TAG_LASeRTransform = TAG_RANGE_FIRST_LASeR,
	TAG_LASeRShape,
	TAG_LASeRAction,
	TAG_LASeRVideo,
	TAG_LASeRAudio,
	TAG_LASeRBitmap,
	TAG_LASeRText,
	TAG_LASeRAnimateTransform,
	TAG_LASeRAnimateColor,
	TAG_LASeRAnimateActivate,
	TAG_LASeRUse,
	TAG_LASeRConditional,
	TAG_LASeRTextInput,
	TAG_LASeRCursor,
	TAG_LASeRBackground = TAG_LASeRCursor+50,
};

typedef struct {
	BASE_NODE
	u8 filled;
	LASeRColor fill;	
	Float width;
	u8 stroked;
	LASeRColor linecolor;
	LASeRPointSequence initpointseq;
	u32 nbAdditionalSeqs;
	LASeRPointSequence *morepointseqs;
} LASeRShape;

typedef struct {
	BASE_NODE
	CHILDREN
	u8 hasChoice;
	u32 choice;
	u8 hasDelta;
	Float deltax, deltay;
	LASeRMatrix m;
} LASeRTransform;

#define LASER_TIME_ATTRIBUTES \
	u8 hasStartTime; \
	Float startTime;

typedef struct {
	BASE_NODE
	LASER_TIME_ATTRIBUTES
} LASeRTimedNode;

typedef struct {
	BASE_NODE
	LASER_TIME_ATTRIBUTES
	u8 repeat, streamID;
	u16 width, height;
} LASeRVideo;

typedef struct {
	BASE_NODE
	LASER_TIME_ATTRIBUTES
	u8 repeat, streamID;
} LASeRAudio;

typedef struct {
	BASE_NODE
	LASeRNode *used_node;
} LASeRUse;

typedef struct {
	BASE_NODE
	LASeRString string;

	u8 filled;
	LASeRColor color;

	u8 horizAlign;
	u8 vertAlign;
	u8 wrap;
	Float displayRectX;
	u8 hasDisplayRect;
	Float displayRectY;
	LASeRString *face;
	u8 style;
	Float size;
} LASeRText;

typedef struct {
	BASE_NODE
	u8 streamID;
} LASeRBitmap;

typedef struct {
	BASE_NODE
	LASER_TIME_ATTRIBUTES

	u32 nbUpdates;
	LASeRUpdate *updates;

} LASeRConditional;

typedef struct {
	BASE_NODE
	LASER_TIME_ATTRIBUTES
	
	u8 hasUrl;
	LASeRString url;
	
	u32 nbUrlParams;
	LASeRUrlParam *urlParams;

	u32 nbActionKeys;
	LASeRActionKey *actionKeys;
} LASeRAction;

#define LASER_BASE_ANIMATION_ATTRIBUTES \
			u8 repeat;	\
			u32 target; \
			u32 nbKeys; \
			u32 *durations;

typedef struct {
	BASE_NODE
	LASER_TIME_ATTRIBUTES
	LASER_BASE_ANIMATION_ATTRIBUTES
} LASeRGenericAnimateNode;

typedef struct {
	BASE_NODE
	LASER_TIME_ATTRIBUTES
	LASER_BASE_ANIMATION_ATTRIBUTES
	LASeRTransformKey *keys;
} LASeRAnimateTransform;

typedef struct {
	BASE_NODE
	LASER_TIME_ATTRIBUTES
	LASER_BASE_ANIMATION_ATTRIBUTES
	LASeRColorKey *keys;
} LASeRAnimateColor;

typedef struct {
	BASE_NODE
	LASER_TIME_ATTRIBUTES
	LASER_BASE_ANIMATION_ATTRIBUTES
	LASeRActivateKey *keys;
} LASeRAnimateActivate;

typedef struct {
	BASE_NODE
	u32 target;
	u32 nbKeys;
	LASeRKey *keys;
} LASeRCursor;

typedef struct {
	BASE_NODE
	u32 text;
	u8 hasTitle;
	LASeRString title;
} LASeRTextInput;

SFNode *LASeR_CreateNode(u32 NodeTag);
void LASeRNode_Del(SFNode *node);

LASeRNode *LASERSG_NewNode(LPSCENEGRAPH inScene, u32 tag);

/* LASeR Updates */


typedef struct {
	u32 target;
	void *node;
} LASeRInsert;

typedef struct {
	u32 target;
} LASeRDelete;

typedef struct {
	u32 target;
	void *node;
} LASeRReplace;

typedef struct {
	u32 index;
	u32 target;
	void *node;
} LASeRReplaceIndexed;

typedef struct {
	u32 target;
	u32 index;
} LASeRDeleteIndexed;

typedef struct {
	BASE_NODE
	u8 isImage;
	u16 streamID;
	LASeRColor *color;
} LASeRBackground;

typedef struct {
	u8 groupID;
	u16 serviceID;
} LASeRClean;

typedef struct {
	u8 groupID;
	u16 serviceID;
	u32 nbIds;
	u32 *ids;
} LASeRSave;

typedef struct {
	u8 groupID;
	u16 serviceID;
} LASeRRestore;

typedef struct {
	u8 fieldIndex;
	u32 target;
	u8 isIndexed;
	u32 index;
	u32 floatEncodedValue1, floatEncodedValue2;
	u32 idvalue;
	u32 colIndex;
	u8 booleanvalue;
	u32 timevalue;
	u8 intvalue;
	LASeRMatrix matrixvalue;
	void *nodevalue;
	LASeRString stringvalue;	
} LASeRReplaceField;


typedef struct {
	u32 time;
	u32 nbUpdates;
	LASeRUpdate *updates;
} LASeRSceneUnit;

typedef struct _tagLASeRDecoder *LPLASERDEC;
M4Err LASeR_ParseSceneUnit(LPLASERDEC codec, char *data, u32 data_length);
LASeRSceneHeader *LASeR_ParseSceneHeader(BitStream *bs);

typedef struct _tagLASeRDecoder
{
	M4Err LastError;

	LASeRSceneHeader *config;

	LPSCENEGRAPH scenegraph;
	/*modified during conditional execution / proto parsing*/
	LPSCENEGRAPH current_graph;

	Double (*GetSceneTime)(void *st_cbk);
	void *st_cbk;
	
	/*when set the decoder works with commands rather than modifying the scene graph directly*/
	Bool dec_memory_mode;

	/*only set in mem mode. Conditionals are stacked while decoding, then decoded once the AU is decoded
	to make sure all nodes potentially used by the conditional command buffer are created*/
	Chain *conditionals;

	Bool ignore_size;
	
	M4Mutex *mx;

	u8 au_num;
} LASeRDecoder;


LPLASERDEC LASeR_NewDecoder(LPSCENEGRAPH scenegraph);
void LASeR_DeleteDecoder(LPLASERDEC codec);
void LASeR_SetClock(LPLASERDEC codec, Double (*GetSceneTime)(void *st_cbk), void *st_cbk );
M4Err LASeR_ConfigureStream(LASeRDecoder * codec, char *DecoderSpecificInfo, u32 DecoderSpecificInfoLength);

Float LASeR_sNbBitsToFloat(LPLASERDEC codec, u32 n, u32 bits);
Float LASeR_s12ToFloat(LPLASERDEC codec, u32 n);

#endif	//_M4_LASER_DEV_H

