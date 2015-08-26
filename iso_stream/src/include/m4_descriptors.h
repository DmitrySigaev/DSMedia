/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Object Descriptor sub-project
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

#ifndef M4_DESCRIPTORS_H
#define M4_DESCRIPTORS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <m4_tools.h>

/***************************************
			Descriptors Tag
***************************************/
enum
{
	ForbiddenZero_Tag					= 0x00,
	ObjectDescriptor_Tag				= 0x01,
	InitialObjectDescriptor_Tag			= 0x02,
	ESDescriptor_Tag					= 0x03,
	DecoderConfigDescriptor_Tag			= 0x04,
	DecoderSpecificInfo_Tag				= 0x05,
	SLConfigDescriptor_Tag				= 0x06,
	ContentIdentification_Tag			= 0x07,
	SuppContentIdentification_Tag		= 0x08,
	IPIPtr_Tag							= 0x09,
	IPMPPtr_Tag							= 0x0A,
	IPMP_Tag							= 0x0B,
	QoS_Tag								= 0x0C,
	RegistrationDescriptor_Tag			= 0x0D,

	/*FILE FORMAT RESERVED IDs - NEVER CREATE / USE THESE DESCRIPTORS*/
	ES_ID_IncTag						= 0x0E,
	ES_ID_RefTag						= 0x0F,
	MP4_IOD_Tag							= 0x10,
	MP4_OD_Tag							= 0x11,
	IPI_DescPtr_Tag						= 0x12,
	/*END FILE FORMAT RESERVED*/
	
	ExtensionPL_Tag						= 0x13,
	PL_IndicationIndex_Tag				= 0x14,
	
	ISO_DESC_RANGE_BEGIN				= 0x15,
	ISO_DESC_RANGE_END					= 0x3F,

	ContentClassification_Tag			= 0x40,
	KeyWordDescriptor_Tag				= 0x41,
	RatingDescriptor_Tag				= 0x42,
	LanguageDescriptor_Tag				= 0x43,
	ShortTextualDescriptor_Tag			= 0x44,
	ExpandedTextualDescriptor_Tag		= 0x45,
	ContentCreatorName_Tag				= 0x46,
	ContentCreationDate_Tag				= 0x47,
	OCI_CreatorName_Tag					= 0x48,
	OCI_CreationDate_Tag				= 0x49,
	SmpteCameraPosition_Tag				= 0x4A,

	SegmentDescriptor_Tag				= 0x4B,
	MediaTimeDescriptor_Tag				= 0x4C,


	ISO_RESERVED_RANGE_BEGIN	= 0x60,
	ISO_RESERVED_RANGE_END		= 0xBF,
	
	USER_DESC_RANGE_BEGIN		= 0xC0,

	/*internal descriptor for mux input description*/
	MuxInfoDescriptor_Tag		= USER_DESC_RANGE_BEGIN,
	/*internal descriptor for bifs config input description*/
	BIFSConfig_Tag				= USER_DESC_RANGE_BEGIN + 1,
	/*internal descriptor for UI config input description*/
	UIConfig_Tag				= USER_DESC_RANGE_BEGIN + 2,
	/*internal descriptor for TextConfig description*/
	TextConfig_Tag				= USER_DESC_RANGE_BEGIN + 3,
	TextSampleDescriptor_Tag	= USER_DESC_RANGE_BEGIN + 4,


	USER_DESC_RANGE_END			= 0xFE,
	
	ForbiddenFF_Tag				= 0xFF
};

/*use to avoid rewriting the code in case ISO defines new extension desc...*/
#define ISO_EXT_RANGE_START		0x80
#define ISO_EXT_RANGE_END		0xFE

#define ISO_OCI_RANGE_START		0x40
#define ISO_OCI_RANGE_END		(ISO_RESERVED_RANGE_BEGIN - 1)

/*********************************************
	currently supported MPEG-4 stream types
*********************************************/
enum
{
	M4ST_FORBIDDEN	= 0x00,
	M4ST_OD			= 0x01,
	M4ST_OCR		= 0x02,
	M4ST_SCENE		= 0x03,
	M4ST_VISUAL		= 0x04,
	M4ST_AUDIO		= 0x05,
	M4ST_MPEG7		= 0x06,
	M4ST_IPMP		= 0x07,
	M4ST_OCI		= 0x08,
	M4ST_MPEGJ		= 0x09,
	M4ST_INTERACT	= 0x0A,
	M4ST_TEXT		= 0x0B,
};

/***************************************
			Descriptors
***************************************/

#define BASE_DESCRIPTOR \
		u8 tag;

typedef struct BaseDescriptor
{
	BASE_DESCRIPTOR
} Descriptor;


/*	default descriptor. 
	NOTE: The decoderSpecificInfo is used as a default desc with tag 0x05 */
typedef struct
{
	BASE_DESCRIPTOR
	u32 dataLength;
	char *data;
} DefaultDescriptor;


/*Object Descriptor*/
typedef struct
{
	BASE_DESCRIPTOR
	u16 objectDescriptorID;
	char *URLString;
	Chain *ESDescriptors;
	Chain *OCIDescriptors;
	Chain *IPMPDescriptorPointers;
	Chain *extensionDescriptors;
} ObjectDescriptor;

/*InitialObjectDescriptor - WARNING: even though the bitstream IOD is not
a bit extension of OD, internally it is a real overclass of OD
we usually typecast IOD to OD when flags are not needed !!!*/
typedef struct
{
	BASE_DESCRIPTOR
	u16 objectDescriptorID;
	char *URLString;
	Chain *ESDescriptors;
	Chain *OCIDescriptors;
	Chain *IPMPDescriptorPointers;
	Chain *extensionDescriptors;

	/*IOD extensions*/
	u8 inlineProfileFlag;
	u8 OD_profileAndLevel;
	u8 scene_profileAndLevel;
	u8 audio_profileAndLevel;
	u8 visual_profileAndLevel;
	u8 graphics_profileAndLevel;

} InitialObjectDescriptor;

/*File Format Object Descriptor*/
typedef struct
{
	BASE_DESCRIPTOR
	u16 objectDescriptorID;
	char *URLString;
	Chain *ES_ID_RefDescriptors;
	Chain *OCIDescriptors;
	Chain *IPMPDescriptorPointers;
	Chain *extensionDescriptors;
	Chain *ES_ID_IncDescriptors;
} M4F_ObjectDescriptor;

/*File Format Initial Object Descriptor - same remark as IOD*/
typedef struct
{
	BASE_DESCRIPTOR
	u16 objectDescriptorID;
	char *URLString;
	Chain *ES_ID_RefDescriptors;
	Chain *OCIDescriptors;
	Chain *IPMPDescriptorPointers;
	Chain *extensionDescriptors;
	Chain *ES_ID_IncDescriptors;

	u8 inlineProfileFlag;
	u8 OD_profileAndLevel;
	u8 scene_profileAndLevel;
	u8 audio_profileAndLevel;
	u8 visual_profileAndLevel;
	u8 graphics_profileAndLevel;
} M4F_InitialObjectDescriptor;


/*File Format ES Descriptor for IOD*/
typedef struct {
	BASE_DESCRIPTOR
	u32 trackID;
} ES_ID_Inc;

/*File Format ES Descriptor for OD*/
typedef struct {
	BASE_DESCRIPTOR
	u16 trackRef;
} ES_ID_Ref;

/*Decoder config Descriptor*/
typedef struct
{
	BASE_DESCRIPTOR
	u8 objectTypeIndication;
	u8 streamType;
	u8 upstream;
	u32 bufferSizeDB;
	u32 maxBitrate;
	u32 avgBitrate;
	DefaultDescriptor *decoderSpecificInfo;
	Chain *profileLevelIndicationIndexDescriptor;
} DecoderConfigDescriptor;

/*the Sync Layer config descriptor*/
typedef struct
{
	BASE_DESCRIPTOR
	u8 predefined;
	u8 useAccessUnitStartFlag;
	u8 useAccessUnitEndFlag;
	u8 useRandomAccessPointFlag;
	u8 useRandomAccessUnitsOnlyFlag;
	u8 usePaddingFlag;
	u8 useTimestampsFlag;
	u8 useIdleFlag;
	u8 durationFlag;
	u32 timestampResolution;
	u32 OCRResolution;
	u8 timestampLength;
	u8 OCRLength;
	u8 AULength;
	u8 instantBitrateLength;
	u8 degradationPriorityLength;
	u8 AUSeqNumLength;
	u8 packetSeqNumLength;
	u32 timeScale;
	u16 AUDuration;
	u16 CUDuration;
	u64 startDTS;
	u64 startCTS;
} SLConfigDescriptor;

/***************************************
			SLConfig Tag
***************************************/
enum
{
	SLPredef_Null = 0x01,
	SLPredef_MP4 = 0x02,
	/*intern to GPAC, means NO SL at all (for streams unable to handle AU reconstruction a timing)*/
	SLPredef_SkipSL = 0xF0
};

/*set SL predefined (assign all fields according to sl->predefined value)*/
M4Err SLSetPredefined(SLConfigDescriptor *sl);

/*Content Identification Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u8 compatibility;
	u8 protectedContent;
	u8 contentTypeFlag;
	u8 contentIdentifierFlag;
	u8 contentType;
	u8 contentIdentifierType;
	/*international code string*/
	unsigned char *contentIdentifier;	
} ContentIdentificationDescriptor;

/*Supplementary Content Identification Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 languageCode;
	char *supplContentIdentifierTitle;
	char *supplContentIdentifierValue;
} SupplementaryContentIdentificationDescriptor;

/*IPI (Intelectual Property Identification) Descriptor Pointer*/
typedef struct {
	BASE_DESCRIPTOR
	u16 IPI_ES_Id;
} IPI_DescrPointer;


/*IPMP Descriptor Pointer*/
typedef struct {
	BASE_DESCRIPTOR
	u8 IPMPDescrID;	
} IPMP_DescrPointer;

/*IPMP descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u8 IPMP_DescID;
	u16 IPMPSType;
	char *URLstring;
	u32 IPMP_DataLength;
	unsigned char *IPMP_Data;
} IPMP_Descriptor;



/*BIFSConfig - parsing only, STORED IN ESD:DCD:DSI*/
typedef struct {
	BASE_DESCRIPTOR
	u32 version;
	u16 nodeIDbits;
	u16 routeIDbits;
	u16 protoIDbits;
	Bool isCommandStream;
	Bool pixelMetrics;
	u16 pixelWidth, pixelHeight;
} BIFSConfigDescriptor;

/*sepcial function for authoring - convert DSI to BIFSConfig*/
M4Err OD_GetBIFSConfig(DefaultDescriptor *dsi, u8 oti, BIFSConfigDescriptor *cfg);

/*flags for style*/
enum
{
	STF_Normal = 0,
	STF_Bold = 1,
	STF_Italic = 2,
	STF_Underlined = 4
};

typedef struct
{
	u16 startChar;
	u16 endChar;
	u16 fontID;
	u8 style_flags;
	u8 font_size;
	/*ARGB*/
	u32 text_color;
} StyleRecord;

typedef struct
{
	u16 fontID;
	char *fontName;
} FontRecord;

typedef struct
{
	s16 top, left, bottom, right;
} BoxRecord;

/*scroll flags*/
enum
{
	STF_ScrollCredits = 0,
	STF_ScrollMarquee = 1,
	STF_ScrollDown = 2,
	STF_ScrollRight = 3
};

/* display flags*/
enum
{
	STF_ScrollIn = 0x00000020,
	STF_ScrollOut = 0x00000040,
	/*use one of the scroll flags, eg STF_ScrollDirection | STF_ScrollCredits*/
	STF_ScrollDirection = 0x00000180,
	STF_Karaoke	= 0x00000800,
	STF_VerticalText = 0x00020000,
	STF_FillRegion = 0x00040000,
};

typedef struct
{
	/*this is defined as a descriptor for parsing*/
	BASE_DESCRIPTOR

	u32 displayFlags;
	/*left, top: 0 -  centered: 1 - bottom, right: -1*/
	s8 horiz_justif, vert_justif;
	/*ARGB*/
	u32 back_color;
	BoxRecord default_pos;
	StyleRecord	default_style;

	u32 font_count;
	FontRecord *fonts;

	/*unused in isomedia but needed for streamingText*/
	u8 sample_index;
} TextSampleDescriptor;

typedef struct
{
	BASE_DESCRIPTOR
	/*only 0x10 shall be used for 3GP text stream*/
	u8 Base3GPPFormat;
	/*only 0x10 shall be used for StreamingText*/
	u8 MPEGExtendedFormat;
	/*only 0x10 shall be used for StreamingText (base profile, base level)*/
	u8 profileLevel;
	u32 timescale;
	/*0 forbidden, 1: out-of-band desc only, 2: in-band desc only, 3: both*/
	u8 sampleDescriptionFlags;
	/*More negative layer values are towards the viewer*/
	s16 layer;
	/*text track width & height*/
	u16 text_width;
	u16 text_height;
	/*compatible 3GP formats, same coding as 3GPPBaseFormat*/
	u8 nb_compatible_formats;
	u8 compatible_formats[20];
	/*defined in m4_isomedia.h*/
	Chain *sample_descriptions;

	/*if true info below are valid (cf 3GPP for their meaning)*/
	Bool has_vid_info;
	u16 video_width;
	u16 video_height;
	s16 horiz_offset;
	s16 vert_offset;
} TextConfigDescriptor;

/*sepcial function for authoring - convert DSI to TextConfig*/
M4Err OD_GetTextConfig(DefaultDescriptor *dsi, u8 oti, TextConfigDescriptor *cfg);


/*MuxInfo descriptor - parsing only, stored in ESD:extDescr*/
typedef struct {
	BASE_DESCRIPTOR
	/*input location*/
	char *file_name;
	/*input groupID for interleaving*/
	u32 GroupID;
	/*input stream format (not required, guessed from file_name)*/
	char *streamFormat;
	/*time offset in ms from first TS (appends an edit list in mp4)*/
	u32 startTime;

	/*media length to import in ms (from 0)*/
	u32 duration;

	/*SRT import extensions - only support for text and italic style*/
	char *textNode;
	char *fontNode;

	/*same as importer flags, cf m4_author.h*/
	u32 import_flags;

	/*indicates input file shall be destryed - used during SWF import*/
	Bool delete_file;
} MuxInfoDescriptor;

typedef struct
{
	BASE_DESCRIPTOR
	/*input type*/
	char *deviceName;
	/*string sensor terminaison (validation) char*/
	char termChar;
	/*string sensor deletion char*/
	char delChar;
	/*device-specific data*/
	char *ui_data;
	u32 ui_data_length;
} UIConfigDescriptor;

/*sepcial function for authoring - convert DSI to UIConfig*/
M4Err OD_GetUIConfig(DefaultDescriptor *dsi, UIConfigDescriptor *cfg);
/*converts UIConfig to dsi - does not destroy input descr but does create output one*/
M4Err OD_EncodeUIConfig(UIConfigDescriptor *cfg, DefaultDescriptor **out_dsi);


/***************************************
			QoS Tags
***************************************/
enum
{
	QoSMaxDelayTag = 0x01,
	QoSPrefMaxDelayTag = 0x02,
	QoSLossProbTag = 0x03,
	QoSMaxGapLossTag = 0x04,
	QoSMaxAUSizeTag = 0x41,
	QoSAvgAUSizeTag = 0x42,
	QoSMaxAURateTag = 0x43
};

/***************************************
			QoS Qualifiers
***************************************/
typedef struct {
	BASE_DESCRIPTOR
	u8 predefined;
	Chain *QoS_Qualifiers;
} QoS_Descriptor;


#define QOS_BASE_QUALIFIER \
	u8 tag;	\
	u32 size;

typedef struct {
	QOS_BASE_QUALIFIER
} QoS_Default;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 MaxDelay;
} QoS_MaxDelay;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 PrefMaxDelay;
} QoS_PrefMaxDelay;

typedef struct {
	QOS_BASE_QUALIFIER
	Float LossProb;
} QoS_LossProb;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 MaxGapLoss;
} QoS_MaxGapLoss;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 MaxAUSize;
} QoS_MaxAUSize;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 AvgAUSize;
} QoS_AvgAUSize;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 MaxAURate;
} QoS_MaxAURate;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 DataLength;		/*max size class : 2^28 - 1*/
	unsigned char *Data;
} QoS_Private;


/*Registration Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 formatIdentifier;
	u32 dataLength;
	unsigned char *additionalIdentificationInfo;
} RegistrationDescriptor;

/*Elementary Stream Descriptor*/
typedef struct
{
	BASE_DESCRIPTOR
	u16 ESID;
	u16 OCRESID;
	u16 dependsOnESID;
	u8 streamPriority;
	char *URLString;
	DecoderConfigDescriptor *decoderConfig;
	SLConfigDescriptor *slConfig;
	IPI_DescrPointer *ipiPtr;
	QoS_Descriptor *qos;
	RegistrationDescriptor *RegDescriptor;
	Chain *IPIDataSet;
	Chain *langDesc;
	Chain *IPMPDescriptorPointers;
	Chain *extensionDescriptors;

	struct _tagStreamPLDescriptor *local_pl;

} ESDescriptor;


/*Content Classification Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 classificationEntity;
	u16 classificationTable;
	u32 dataLength;
	unsigned char *contentClassificationData;
} ContentClassificationDescriptor;


/*this structure is used in KeyWordDescriptor*/
typedef struct {
	char *keyWord;
} KeyWordItem;

/*Key Word Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 languageCode;
	u8 isUTF8;
	Chain *keyWordsList;
} KeyWordDescriptor;

/*Rating Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 ratingEntity;
	u16 ratingCriteria;
	u32 infoLength;
	unsigned char *ratingInfo;
} RatingDescriptor;

/*Language Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 langCode;
} LanguageDescriptor;


/*Short Textual Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 langCode;
	u8 isUTF8;
	char *eventName;
	char *eventText;
} ShortTextualDescriptor;


/*this structure is used in ExpandedTextualDescriptor*/
typedef struct {
	char *text;
} ETD_ItemText;

/*Expanded Textual Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 langCode;
	u8 isUTF8;
	Chain *itemDescriptionList;
	Chain *itemTextList;
	unsigned char *NonItemText;
} ExpandedTextualDescriptor;

/*this structure is used in ContentCreatorNameDescriptor*/
typedef struct {
	u32 langCode;
	u8 isUTF8;
	char *contentCreatorName;
} contentCreatorInfo;

/*Content Creator Name Descriptor
NOTE: the desctructor will delete all the items in the list
(contentCreatorInfo items) */
typedef struct {
	BASE_DESCRIPTOR
	Chain *ContentCreators;
} ContentCreatorNameDescriptor;

/*Content Creation Date Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	unsigned char contentCreationDate[5];
} ContentCreationDateDescriptor;


/*this structure is used in OCICreatorNameDescriptor*/
typedef struct {
	u32 langCode;
	u8 isUTF8;
	char *OCICreatorName;
} OCICreator;

/*OCI Creator Name Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	Chain *OCICreators;
} OCICreatorNameDescriptor;

/*OCI Creation Date Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	unsigned char OCICreationDate[5];
} OCICreationDateDescriptor;


/*this structure is used in SmpteCameraPositionDescriptor*/
typedef struct {
	u8 paramID;
	u32 param;
} SmpteParam;

/*Smpte Camera Position Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u8 cameraID;
	Chain *ParamList;
} SmpteCameraPositionDescriptor;


/*Extension Profile Level Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u8 profileLevelIndicationIndex;
	u8 ODProfileLevelIndication;
	u8 SceneProfileLevelIndication;
	u8 AudioProfileLevelIndication;
	u8 VisualProfileLevelIndication;
	u8 GraphicsProfileLevelIndication;
	u8 MPEGJProfileLevelIndication;
} ExtensionProfileLevelDescriptor;

/*Profile Level Indication Index Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u8 profileLevelIndicationIndex;
} ProfileLevelIndicationIndexDescriptor;


/****************************************************************************

			MPEG-4 SYSTEM - OBJECT DESCRIPTORS COMMANDS DECLARATION

****************************************************************************/


/***************************************
			Commands Tags
***************************************/
enum
{
	ODUpdate_Tag					= 0x01,
	ODRemove_Tag					= 0x02,
	ESDUpdate_Tag					= 0x03,
	ESDRemove_Tag					= 0x04,
	IPMPDUpdate_Tag					= 0x05,
	IPMPDRemove_Tag					= 0x06,

	/*file format reserved*/
	ESDRemove_Ref_Tag				= 0x07,

	ODExecute_Tag					= 0x08,

	IPMPToolDescUpdate_Tag			= 0x09,
	IPMPToolDescRemove_Tag			= 0x0A,
	IPMPToolDescLink_Tag			= 0x0B,
	IPMPToolDescUnlink_Tag			= 0x0C,

	ISO_RESERVED_COMMANDS_BEGIN		= 0x0D,
	ISO_RESERVED_COMMANDS_END		= 0xBF,
	
	USER_RESERVED_COMMANDS_BEGIN	= 0xC0,
	USER_RESERVED_COMMANDS_END		= 0xFE
};

/***************************************
			OD commands
***************************************/
#define BASE_OD_COMMAND \
	u8 tag;

/*the (abstract) base command. */
typedef struct {
	BASE_OD_COMMAND
} ODCommand;

/*the default bcommand*/
typedef struct {
	BASE_OD_COMMAND
	u32 dataSize;
	char *data;
} BaseODCommand;

/*Object Descriptor Update
NB: the list can contain OD or IOD, except internally in the File Format (only MP4_OD)*/
typedef struct
{
	BASE_OD_COMMAND
	Chain *objectDescriptors;
} ObjectDescriptorUpdate;

/*Object Descriptor Remove*/
typedef struct
{
	BASE_OD_COMMAND
	u32 NbODs;
	u16 *OD_ID;
} ObjectDescriptorRemove;

/*Object Descriptor Execute*/
typedef struct
{
	BASE_OD_COMMAND
	u32 NbODs;
	u16 *OD_ID;
} ObjectDescriptorExecute;

/*Elementary Stream Descriptor Update*/
typedef struct
{
	BASE_OD_COMMAND
	u16 ODID;
	Chain *ESDescriptors;
} ESDescriptorUpdate;

/*Elementary Stream Descriptor Remove*/
typedef struct {
	BASE_OD_COMMAND
	u16 ODID;
	u32 NbESDs;
	u16 *ES_ID;
} ESDescriptorRemove;

/*IPMP Descriptor Update*/
typedef struct {
	BASE_OD_COMMAND
	Chain *IPMPDescList;
} IPMPDescriptorUpdate;

/*IPMP Descriptor Remove*/
typedef struct {
	BASE_OD_COMMAND
	u32 NbIPMPDs;
	u8 *IPMPDescID;
} IPMPDescriptorRemove;


/************************************************************
				Media Control Extensions
************************************************************/
typedef struct
{
	BASE_DESCRIPTOR
	Double startTime;
	Double Duration;
	unsigned char *SegmentName;
} SegmentDescriptor;

typedef struct
{
	BASE_DESCRIPTOR
	Double mediaTimeStamp;
} MediaTimeDescriptor;




/********************************************************************
	OD Exports
********************************************************************/

#define OD_READ		1
#define OD_WRITE	0

/*our main OD CODEC object */
typedef struct tagODCoDec * LPODCODEC;

/*construction / destruction*/
LPODCODEC OD_NewCodec(u32 mode);
void OD_DeleteCodec(LPODCODEC codec);

/*				ENCODER FUNCTIONS
add a command to the codec command list. 
The command WILL BE DESTROYED BY THE CODEC after encoding*/
M4Err OD_AddCommand(LPODCODEC codec, ODCommand *command);

/*encode the previously set-up AU
WARNING: once this function called, the codec commandList is empty 
and commands destroyed*/
M4Err OD_EncodeAU(LPODCODEC codec);

/*get the encoded AU. The memory allocation is done in place
WARNING: once this function called, the codec buffer is deleted*/
M4Err OD_GetEncodedAU(LPODCODEC codec, char **outAU, u32 *au_length);



/*				DECODER FUNCTION
set the encoded FULL AU to the codec*/
M4Err OD_SetBuffer(LPODCODEC codec, char *au, u32 au_length);

/*Decoder: decode the previously set-up AU
the input buffer is cleared once decoded*/
M4Err OD_DecodeAU(LPODCODEC codec);

/*get the first OD command in the list. Once called, the command is removed 
from the command list. Return NULL when commandList is empty*/
ODCommand *OD_GetCommand(LPODCODEC codec);


/************************************************************
		ODCommand Functions
************************************************************/

/*Commands Creation / Destruction*/
ODCommand *OD_NewCommand(u8 tag);
M4Err OD_DeleteCommand(ODCommand **com);


/************************************************************
		Descriptors Functions
************************************************************/

/*Descriptors Creation / Destruction*/
Descriptor *OD_NewDescriptor(u8 tag);
M4Err OD_DeleteDescriptor(Descriptor **desc);

/*use this function to decode a standalone descriptor
the raw descriptor MUST be formatted with tag and size field!!!
a new desc is created and you must delete it when done*/
M4Err OD_ReadDesc(char *raw_desc, u32 descSize, Descriptor * *outDesc);

/*use this function to encode a standalone descriptor
the desc will be formatted with tag and size field
the output buffer is allocated and you must delete it when done*/
M4Err OD_EncDesc(Descriptor *desc, char **outEncDesc, u32 *outSize);

/*use this function to get the size of a standalone descriptor (including tag and size fields)
return 0 if error*/
u32 OD_GetDescSize(Descriptor *desc);

/*this is usefull to duplicate on the fly a descriptor*/
M4Err OD_DuplicateDescriptor(Descriptor *inDesc, Descriptor **outDesc);


/*This functions handles internally what desc can be added to another desc
and adds it. NO DUPLICATION of the descriptor, so
once a desc is added to its parent, destroying the parent WILL DESTROY 
this descriptor*/
M4Err OD_AddDescToDesc(Descriptor *parentDesc, Descriptor *newDesc);


/*this is a helper for building a preformatted ESDescriptor with decoderConfig, decoderSpecificInfo with no data and 
SLConfig descriptor with predefined*/
ESDescriptor *OD_NewESDescriptor(u32 sl_predefined);


/*Since IPMP V2, we introduce a new set of functions to read / write a list of descriptors
that have no containers (a bit like an OD command, but for descriptors)
This is usefull for IPMPv2 DecoderSpecificInfo which contains a set of IPMP_Declarators
As it could be used for other purposes we keep it generic
you must create the list yourself, the functions just encode/decode from/to the list*/

/*uncompress an encoded list of descriptors. You must pass an empty Chain structure
to know exactly what was in the buffer*/
M4Err OD_ReadDescList(char *raw_list, u32 raw_size, Chain *descList);
/*compress all descriptors in the list into a single buffer. The buffer is allocated
by the lib and must be destroyed by your app
you must pass (outEncList != NULL  && *outEncList == NULL)*/
M4Err OD_EncDescList(Chain *descList, char **outEncList, u32 *outSize);
/*returns size of encoded desc list*/
M4Err OD_SizeDescList(Chain *descList, u32 *outSize);
/*destroy the descriptors in a list but not the list*/
M4Err OD_DeleteDescList(Chain *descList);

/*retuns NULL if unknown, otherwise value*/
const char *OD_GetStreamTypeName(u32 streamType);
u32 OD_GetStreamTypeByName(const char *streamType);

/************************************************************
		QoS Qualifiers Functions
************************************************************/

/*QoS Qualifiers Creation / Destruction*/
QoS_Default *OD_NewQoSQualifier(u8 tag);
M4Err OD_DeleteQoSQualifier(QoS_Default **qos);

/*READ/WRITE functions: QoS qualifiers are special descriptors but follow the same rules as descriptors.
therefore, use OD_ReadDesc and OD_EncDesc for QoS*/

/*same function, but for QoS, as a Qualifier IS NOT a descriptor*/
M4Err OD_AddQualifToQoS(QoS_Descriptor *desc, QoS_Default *qualif);



/*
	OCI Stream AU is a list of OCI event (like OD AU is a list of OD commands)
*/

typedef struct tagOCIEvent *LPOCIEVENT;

LPOCIEVENT NewOCIEvent(u16 EventID);
void DeleteOCIEvent(LPOCIEVENT event);

M4Err OCIEvent_SetStartTime(LPOCIEVENT event, u8 Hours, u8 Minutes, u8 Seconds, u8 HundredSeconds, u8 IsAbsoluteTime);
M4Err OCIEvent_SetDuration(LPOCIEVENT event, u8 Hours, u8 Minutes, u8 Seconds, u8 HundredSeconds);
M4Err OCIEvent_AddDescriptor(LPOCIEVENT event, Descriptor *oci_desc);

M4Err OCIEvent_GetEventID(LPOCIEVENT event, u16 *ID);
M4Err OCIEvent_GetStartTime(LPOCIEVENT event, u8 *Hours, u8 *Minutes, u8 *Seconds, u8 *HundredSeconds, u8 *IsAbsoluteTime);
M4Err OCIEvent_GetDuration(LPOCIEVENT event, u8 *Hours, u8 *Minutes, u8 *Seconds, u8 *HundredSeconds);
u32 OCIEvent_GetDescriptorCount(LPOCIEVENT event);
Descriptor *OCIEvent_GetDescriptor(LPOCIEVENT event, u32 DescIndex);
M4Err OCIEvent_RemoveDescriptor(LPOCIEVENT event, u32 DescIndex);



typedef struct tagOCICodec *LPOCICODEC;

/*construction / destruction
IsEncoder specifies an OCI Event encoder
version is for future extensions, and only 0x01 is valid for now*/
LPOCICODEC OCI_NewCodec(u8 IsEncoder, u8 Version);
void OCI_DeleteCodec(LPOCICODEC codec);

/*				ENCODER FUNCTIONS
add a command to the codec event list. 
The event WILL BE DESTROYED BY THE CODEC after encoding*/
M4Err OCI_AddEvent(LPOCICODEC codec, LPOCIEVENT event);

/*encode AU. The memory allocation is done in place
WARNING: once this function called, the codec event List is empty 
and events destroyed
you must set *outAU = NULL*/
M4Err OCI_EncodeAU(LPOCICODEC codec, char **outAU, u32 *au_length);



/*Decoder: decode the previously set-up AU
the input buffer is cleared once decoded*/
M4Err OCI_DecodeAU(LPOCICODEC codec, char *au, u32 au_length);

/*get the first OCI Event in the list. Once called, the event is removed 
from the event list. Return NULL when the event List is empty
you MUST delete events */
LPOCIEVENT OCI_GetEvent(LPOCICODEC codec);


/*OD dump tools*/
M4Err OD_DumpAU(char *data, u32 dataLength, FILE *trace, u32 indent, Bool XMTDump);
M4Err OD_DumpCommand(void *p, FILE *trace, u32 indent, Bool XMTDump);
M4Err OD_DumpDescriptor(void *ptr, FILE *trace, u32 indent, Bool XMTDump);
M4Err OD_DumpCommandList(Chain *commandList, FILE *trace, u32 indent, Bool XMTDump);

/*OCI dump tools*/
M4Err OCI_DumpEvent(LPOCIEVENT ev, FILE *trace, u32 indent, Bool XMTDump);
M4Err OCI_DumpAU(u8 version, char *au, u32 au_length, FILE *trace, u32 indent, Bool XMTDump);


/*OD parsing tools (XMT/BT)*/
/*returns desc tag based on name*/
u32 OD_GetDescriptorTag(char *descName);
/*returns 2 if field is a descriptor list, 1 if field is a descriptor and 0 otherwise*/
u32 OD_DescriptorFieldType(Descriptor *desc, char *fieldName);
/*set non-descriptor field value - value string shall be presented without ' or " characters*/
M4Err OD_SetDescriptorField(Descriptor *desc, char *fieldName, char *val);


/*
	Although this is not really part of the OD framework, it's so related to SLConfig that we keep it here
*/
typedef struct tagSLHeader
{
	u8 accessUnitStartFlag;
	u8 accessUnitEndFlag;
	u8 paddingFlag;
	u8 randomAccessPointFlag;
	u8 OCRflag;
	u8 idleFlag;
	u8 decodingTimeStampFlag;
	u8 compositionTimeStampFlag;
	u8 instantBitrateFlag;
	u8 degradationPriorityFlag;

	u8 paddingBits;
	u16 packetSequenceNumber;
	u64 objectClockReference;
	u16 AU_sequenceNumber;
	u64 decodingTimeStamp;
	u64 compositionTimeStamp;
	u16 accessUnitLength;
	u32 instantBitrate;
	u16 degradationPriority;

	/*this is NOT part of standard SL, only used internally: signals duration of access unit if known
	this is usefull for streams with very random updates, to prevent buffering for instance a subtitle stream
	which is likely to have no updates during the first minutes... expressed in media timescale*/
	u32 au_duration;
} SLHeader, LPSLHDR;

/*packetize SL-PDU*/
void SL_Packetize(SLConfigDescriptor* slConfig, SLHeader *Header, char *PDU, u32 size, char **outPacket, u32 *OutSize);
/*depacketize SL-PDU*/
void SL_Depacketize(SLConfigDescriptor *slConfig, SLHeader *Header, char *PDU, u32 PDULength, u32 *HeaderLen);


#ifdef __cplusplus
}
#endif

#endif	/*M4_DESCRIPTORS_H*/
