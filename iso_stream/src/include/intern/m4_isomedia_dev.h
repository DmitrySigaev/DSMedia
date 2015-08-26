/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / ISO Media File Format sub-project
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

#ifndef __M4_ISOMEDIA_DEV_H
#define __M4_ISOMEDIA_DEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <m4_isomedia.h>



/*
	All atoms
*/


//the default size is 64, cause we need to handle large atoms...
#define BASE_ATOM			\
	u32 type;			\
	u8 uuid[16];			\
	u64 size;			\

#define FULL_ATOM		\
	BASE_ATOM			\
	u8 version;			\
	u32 flags;			\


typedef struct tagAtom
{
	BASE_ATOM
} Atom;

typedef struct tagFullAtom
{
	FULL_ATOM
} FullAtom;

M4Err WriteAtom(Atom *ptr, BitStream *bs);
M4Err ReadAtom(Atom *ptr, BitStream *bs, u64 *read);
void DelAtom(Atom *ptr);
M4Err SizeAtom(Atom *ptr);

M4Err ParseAtom(Atom **outAtom, BitStream *bs, u64 *read);
M4Err Atom_Size(Atom *ptr);
M4Err FullAtom_Size(Atom *ptr);
M4Err Atom_Write(Atom *ptr, BitStream *bs);
M4Err FullAtom_Read(Atom *ptr, BitStream *bs, u64 *read);
M4Err FullAtom_Write(Atom *s, BitStream *bs);
void InitFullAtom(Atom *ptr);
void DeleteAtomList(Chain *atomList);
M4Err WriteAtomList(Atom *parent, Chain *list, BitStream *bs);
M4Err SizeAtomList(Atom *parent, Chain *list);

/*constructor*/
Atom *CreateAtom(u32 atomType);

enum
{
	ChunkLargeOffsetAtomType			= FOUR_CHAR_INT( 'c', 'o', '6', '4' ),
	ChunkOffsetAtomType					= FOUR_CHAR_INT( 's', 't', 'c', 'o' ),
	ClockReferenceMediaHeaderAtomType	= FOUR_CHAR_INT( 'c', 'r', 'h', 'd' ),
	CompositionOffsetAtomType			= FOUR_CHAR_INT( 'c', 't', 't', 's' ),
	CopyrightAtomType					= FOUR_CHAR_INT( 'c', 'p', 'r', 't' ),
	DataEntryURLAtomType				= FOUR_CHAR_INT( 'u', 'r', 'l', ' ' ),
	DataEntryURNAtomType				= FOUR_CHAR_INT( 'u', 'r', 'n', ' ' ),
	DataInformationAtomType				= FOUR_CHAR_INT( 'd', 'i', 'n', 'f' ),
	DataReferenceAtomType				= FOUR_CHAR_INT( 'd', 'r', 'e', 'f' ),
	DegradationPriorityAtomType			= FOUR_CHAR_INT( 's', 't', 'd', 'p' ),
	EditAtomType						= FOUR_CHAR_INT( 'e', 'd', 't', 's' ),
	EditListAtomType					= FOUR_CHAR_INT( 'e', 'l', 's', 't' ),
	ExtendedAtomType					= FOUR_CHAR_INT( 'u', 'u', 'i', 'd' ),
	FreeSpaceAtomType					= FOUR_CHAR_INT( 'f', 'r', 'e', 'e' ),
	HandlerAtomType						= FOUR_CHAR_INT( 'h', 'd', 'l', 'r' ),
	HintMediaHeaderAtomType				= FOUR_CHAR_INT( 'h', 'm', 'h', 'd' ),
	HintTrackReferenceAtomType			= FOUR_CHAR_INT( 'h', 'i', 'n', 't' ),
	MediaAtomType						= FOUR_CHAR_INT( 'm', 'd', 'i', 'a' ),
	MediaDataAtomType					= FOUR_CHAR_INT( 'm', 'd', 'a', 't' ),
	MediaHeaderAtomType					= FOUR_CHAR_INT( 'm', 'd', 'h', 'd' ),
	MediaInformationAtomType			= FOUR_CHAR_INT( 'm', 'i', 'n', 'f' ),
	MovieAtomType						= FOUR_CHAR_INT( 'm', 'o', 'o', 'v' ),
	MovieHeaderAtomType					= FOUR_CHAR_INT( 'm', 'v', 'h', 'd' ),
	SampleDescriptionAtomType			= FOUR_CHAR_INT( 's', 't', 's', 'd' ),
	SampleSizeAtomType					= FOUR_CHAR_INT( 's', 't', 's', 'z' ),
	CompactSampleSizeAtomType			= FOUR_CHAR_INT( 's', 't', 'z', '2' ),
	SampleTableAtomType					= FOUR_CHAR_INT( 's', 't', 'b', 'l' ),
	SampleToChunkAtomType				= FOUR_CHAR_INT( 's', 't', 's', 'c' ),
	ShadowSyncAtomType					= FOUR_CHAR_INT( 's', 't', 's', 'h' ),
	SkipAtomType						= FOUR_CHAR_INT( 's', 'k', 'i', 'p' ),
	SoundMediaHeaderAtomType			= FOUR_CHAR_INT( 's', 'm', 'h', 'd' ),
	SyncSampleAtomType					= FOUR_CHAR_INT( 's', 't', 's', 's' ),
	TimeToSampleAtomType				= FOUR_CHAR_INT( 's', 't', 't', 's' ),
	TrackAtomType						= FOUR_CHAR_INT( 't', 'r', 'a', 'k' ),
	TrackHeaderAtomType					= FOUR_CHAR_INT( 't', 'k', 'h', 'd' ),
	TrackReferenceAtomType				= FOUR_CHAR_INT( 't', 'r', 'e', 'f' ),
	UserDataAtomType					= FOUR_CHAR_INT( 'u', 'd', 't', 'a' ),
	VideoMediaHeaderAtomType			= FOUR_CHAR_INT( 'v', 'm', 'h', 'd' ),
	
	GenericMediaSampleEntryAtomType		= FOUR_CHAR_INT( 'g', 'n', 'r', 'm' ),
	GenericVisualSampleEntryAtomType	= FOUR_CHAR_INT( 'g', 'n', 'r', 'v' ),
	GenericAudioSampleEntryAtomType		= FOUR_CHAR_INT( 'g', 'n', 'r', 'a' ),

	/*V2 atoms*/
	FileTypeAtomType					= FOUR_CHAR_INT( 'f', 't', 'y', 'p' ),
	PaddingBitsAtomType					= FOUR_CHAR_INT( 'p', 'a', 'd', 'b' ),

	/*MP4 Atoms*/
	SceneDescriptionMediaHeaderAtomType	= FOUR_CHAR_INT( 's', 'd', 'h', 'd' ),
	StreamDependenceAtomType			= FOUR_CHAR_INT( 'd', 'p', 'n', 'd' ),
	ObjectDescriptorAtomType			= FOUR_CHAR_INT( 'i', 'o', 'd', 's' ),
	ObjectDescriptorMediaHeaderAtomType	= FOUR_CHAR_INT( 'o', 'd', 'h', 'd' ),
	ODTrackReferenceAtomType			= FOUR_CHAR_INT( 'm', 'p', 'o', 'd' ),
	MPEGMediaHeaderAtomType				= FOUR_CHAR_INT( 'n', 'm', 'h', 'd' ),
	ESDAtomType							= FOUR_CHAR_INT( 'e', 's', 'd', 's' ),
	OCRReferenceAtomType				= FOUR_CHAR_INT( 's', 'y', 'n', 'c' ),
	IPIReferenceAtomType				= FOUR_CHAR_INT( 'i', 'p', 'i', 'r' ),
	MPEGSampleEntryAtomType				= FOUR_CHAR_INT( 'm', 'p', '4', 's' ),
	MPEGAudioSampleEntryAtomType		= FOUR_CHAR_INT( 'm', 'p', '4', 'a' ),
	MPEGVisualSampleEntryAtomType		= FOUR_CHAR_INT( 'm', 'p', '4', 'v' ),

	/*3GPP atoms*/
	AMRSampleEntryAtomType				= FOUR_CHAR_INT( 's', 'a', 'm', 'r' ),
	WB_AMRSampleEntryAtomType			= FOUR_CHAR_INT( 's', 'a', 'w', 'b' ),
	AMRConfigAtomType					= FOUR_CHAR_INT( 'd', 'a', 'm', 'r' ),
	H263SampleEntryAtomType				= FOUR_CHAR_INT( 's', '2', '6', '3' ),
	H263ConfigAtomType					= FOUR_CHAR_INT( 'd', '2', '6', '3' ),

#ifndef	M4_ISO_NO_FRAGMENTS
	/*V2 atoms - Movie Fragments*/
	MovieExtendsAtomType				= FOUR_CHAR_INT( 'm', 'v', 'e', 'x' ),
	TrackExtendsAtomType				= FOUR_CHAR_INT( 't', 'r', 'e', 'x' ),
	MovieFragmentAtomType				= FOUR_CHAR_INT( 'm', 'o', 'o', 'f' ),
	MovieFragmentHeaderAtomType			= FOUR_CHAR_INT( 'm', 'f', 'h', 'd' ),
	TrackFragmentAtomType				= FOUR_CHAR_INT( 't', 'r', 'a', 'f' ),
	TrackFragmentHeaderAtomType			= FOUR_CHAR_INT( 't', 'f', 'h', 'd' ),
	TrackFragmentRunAtomType			= FOUR_CHAR_INT( 't', 'r', 'u', 'n' ),
#endif

	/*AVC / H264*/
	AVCConfigurationAtomType			= FOUR_CHAR_INT( 'a', 'v', 'c', 'C' ),
	MPEG4BitRateAtomType				= FOUR_CHAR_INT( 'b', 't', 'r', 't' ),
	MPEG4ExtensionDescriptorsAtomType	= FOUR_CHAR_INT( 'm', '4', 'd', 's' ),
	AVCSampleEntryAtomType				= FOUR_CHAR_INT( 'a', 'v', 'c', '1' ),

	/*3GPP-StreamingText*/
	FontTableAtomType					= FOUR_CHAR_INT( 'f', 't', 'a', 'b' ),
	TextSampleEntryAtomType				= FOUR_CHAR_INT( 't', 'x', '3', 'g' ),
	TextStyleAtomType					= FOUR_CHAR_INT( 's', 't', 'y', 'l' ),
	TextHighlightAtomType				= FOUR_CHAR_INT( 'h', 'l', 'i', 't' ),
	TextHighlightColorAtomType			= FOUR_CHAR_INT( 'h', 'c', 'l', 'r' ),
	TextKaraokeAtomType					= FOUR_CHAR_INT( 'k', 'r', 'o', 'k' ),
	TextScrollDelayAtomType				= FOUR_CHAR_INT( 'd', 'l', 'a', 'y' ),
	TextHyperTextAtomType				= FOUR_CHAR_INT( 'h', 'r', 'e', 'f' ),
	TextBoxAtomType						= FOUR_CHAR_INT( 't', 'b', 'o', 'x' ),
	TextBlinkAtomType					= FOUR_CHAR_INT( 'b', 'l', 'n', 'k' ),
	TextWrapAtomType					= FOUR_CHAR_INT( 't', 'w', 'r', 'p' ),

	/*
			Hint atoms
	*/

	//for compatibility with Darwin - not used
	VoidAtomType						= FOUR_CHAR_INT( 'V', 'O', 'I', 'D' ),
	//temp: storage of AU fragments
	SampleFragmentAtomType				=  FOUR_CHAR_INT( 'S', 'T', 'S', 'F' ),

	//general protocols
	GenericHintSampleEntryAtomType			= FOUR_CHAR_INT( 'g', 'h', 'n', 't' ),
	RTPHintSampleEntryAtomType				= FOUR_CHAR_INT( 'r', 't', 'p', ' ' ),
	//Hint track information (in UDTA)
	HintTrackInfoAtomType					= FOUR_CHAR_INT( 'h', 'n', 't', 'i' ),
	RTPAtomType								= FOUR_CHAR_INT( 'r', 't', 'p', ' ' ),
	SDPAtomType								= FOUR_CHAR_INT( 's', 'd', 'p', ' ' ),
	HintInfoAtomType						= FOUR_CHAR_INT( 'h', 'i', 'n', 'f' ),
	nameAtomType							= FOUR_CHAR_INT( 'n', 'a', 'm', 'e' ),
	//Hint track information (in UDTA/hinf)
	trpyAtomType							= FOUR_CHAR_INT( 't', 'r', 'p', 'y' ),
	numpAtomType							= FOUR_CHAR_INT( 'n', 'u', 'm', 'p' ),
	totlAtomType							= FOUR_CHAR_INT( 't', 'o', 't', 'l' ),
	npckAtomType							= FOUR_CHAR_INT( 'n', 'p', 'c', 'k' ),
	tpylAtomType							= FOUR_CHAR_INT( 't', 'p', 'y', 'l' ),
	tpayAtomType							= FOUR_CHAR_INT( 't', 'p', 'a', 'y' ),
	maxrAtomType							= FOUR_CHAR_INT( 'm', 'a', 'x', 'r' ),
	dmedAtomType							= FOUR_CHAR_INT( 'd', 'm', 'e', 'd' ),
	dimmAtomType							= FOUR_CHAR_INT( 'd', 'i', 'm', 'm' ),
	drepAtomType							= FOUR_CHAR_INT( 'd', 'r', 'e', 'p' ),
	tminAtomType							= FOUR_CHAR_INT( 't', 'm', 'i', 'n' ),
	tmaxAtomType							= FOUR_CHAR_INT( 't', 'm', 'a', 'x' ),
	pmaxAtomType							= FOUR_CHAR_INT( 'p', 'm', 'a', 'x' ),
	dmaxAtomType							= FOUR_CHAR_INT( 'd', 'm', 'a', 'x' ),
	paytAtomType							= FOUR_CHAR_INT( 'p', 'a', 'y', 't' ),

	//sample entry extensions
	relyHintEntryType					= FOUR_CHAR_INT( 'r', 'e', 'l', 'y' ),
	timsHintEntryType					= FOUR_CHAR_INT( 't', 'i', 'm', 's' ),
	tsroHintEntryType					= FOUR_CHAR_INT( 't', 's', 'r', 'o' ),
	snroHintEntryType					= FOUR_CHAR_INT( 's', 'n', 'r', 'o' ),

	//RTP Packets extensions
	rtpoAtomType						= FOUR_CHAR_INT( 'r', 't', 'p', 'o' )
};


// The mdat has its own File for EDIT
//	All the samples will therefore be read from this file...
typedef struct tagMediaDataAtom
{
	BASE_ATOM
	u64 dataSize;
	char *data;
} MediaDataAtom;

typedef struct tagUnknownAtom
{
	BASE_ATOM
	char *data;
	u32 dataSize;
} UnknownAtom;

typedef struct tagMovieAtom
{
	BASE_ATOM
	struct tagMovieHeaderAtom *mvhd;
	struct tagObjectDescriptorAtom *iods;
	struct tagUserDataAtom *udta;
#ifndef	M4_ISO_NO_FRAGMENTS
	struct tagMovieExtendsAtom *mvex;
#endif

	struct tagM4File *mov;

	Chain *atomList;
	Chain *trackList;
} MovieAtom;

typedef struct tagMovieHeaderAtom
{
	FULL_ATOM
	u64 creationTime;
	u64 modificationTime;
	u32 timeScale;
	u64 duration;
	u32 nextTrackID;
	//the following field are reserved in MPEG4, but used in QT and MJPEG2K
	u32 preferredRate;
	u16 preferredVolume;
	char reserved[10];
	u32 matrixA;
	u32 matrixB;
	u32 matrixU;
	u32 matrixC;
	u32 matrixD;
	u32 matrixV;
	u32 matrixW;
	u32 matrixX;
	u32 matrixY;
	u32 previewTime;
	u32 previewDuration;
	u32 posterTime;
	u32 selectionTime;
	u32 selectionDuration;
	u32 currentTime;
} MovieHeaderAtom;

typedef struct tagObjectDescriptorAtom
{
	FULL_ATOM
	Descriptor *descriptor;
} ObjectDescriptorAtom;

typedef struct tagTrackAtom
{
	BASE_ATOM
	struct tagUserDataAtom *udta;
	struct tagTrackHeaderAtom *Header;
	struct tagMediaAtom *Media;
	struct tagEditAtom *EditAtom;
	struct tagTrackReferenceAtom *References;
	struct tagMovieAtom *moov;
	Chain *atomList;
	/*private for media padding*/
	u32 padding_bytes;
	/*private for editing*/
	char *name;
	/*private for editing*/
	Bool is_unpacked;
} TrackAtom;

typedef struct tagTrackHeaderAtom
{
	FULL_ATOM
	u64 creationTime;
	u64 modificationTime;
	u32 trackID;
	u32 reserved1;
	u64 duration;

	u32 reserved2[2];
	u16 layer;
	u16 alternate_group;
	u16 volume;
	u16 reserved3;
	u32 matrix[9];	//reserved 5
	u32 width, height;
} TrackHeaderAtom;

typedef struct tagTrackReferenceAtom
{
	BASE_ATOM
	Chain *atomList;
} TrackReferenceAtom;

typedef struct tagMediaAtom
{
	BASE_ATOM
	struct tagTrackAtom *mediaTrack;
	struct tagMediaHeaderAtom *mediaHeader;
	struct tagHandlerAtom *handler;
	struct tagMediaInformationAtom *information;
	Chain *atomList;
	u64 BytesMissing;
} MediaAtom;

typedef struct tagMediaHeaderAtom
{
	FULL_ATOM
	u64 creationTime;
	u64 modificationTime;
	u32 timeScale;
	u64 duration;
	char packedLanguage[4];
	u16 reserved;
} MediaHeaderAtom;

typedef struct tagHandlerAtom
{
	FULL_ATOM
	u32 reserved1;
	u32 handlerType;
	u8 reserved2[12];
	u32 nameLength;
	char *nameUTF8;
} HandlerAtom;

typedef struct tagMediaInformationAtom
{
	BASE_ATOM
	struct tagDataInformationAtom *dataInformation;
	struct tagSampleTableAtom *sampleTable;
	Atom *InfoHeader;
	struct tagDataMap *dataHandler;
	u32 dataEntryIndex;
	Chain *atomList;
} MediaInformationAtom;

typedef struct tagVideoMediaHeaderAtom
{
	FULL_ATOM
	u64 reserved;
} VideoMediaHeaderAtom;

typedef struct tagSoundMediaHeaderAtom
{
	FULL_ATOM
	u32 reserved;
} SoundMediaHeaderAtom;

typedef struct tagHintMediaHeaderAtom
{
	FULL_ATOM
	//this is used for us INTERNALLY
	u32 subType;
	//the following need refinement in MPEG, why is it used for ???
	u32 maxPDUSize;
	u32 avgPDUSize;
	u32 maxBitrate;
	u32 avgBitrate;
	u32 slidingAverageBitrate;
} HintMediaHeaderAtom;

typedef struct tagMPEGMediaHeaderAtom
{
	FULL_ATOM
} MPEGMediaHeaderAtom;

typedef struct tagObjectDescriptorMediaHeaderAtom
{
	FULL_ATOM
} ObjectDescriptorMediaHeaderAtom;

typedef struct tagClockReferenceMediaHeaderAtom
{
	FULL_ATOM
} ClockReferenceMediaHeaderAtom;

typedef struct tagSceneDescriptionMediaHeaderAtom
{
	FULL_ATOM
} SceneDescriptionMediaHeaderAtom;

typedef struct tagDataInformationAtom
{
	BASE_ATOM
	struct tagDataReferenceAtom *dref;
	Chain *atomList;
} DataInformationAtom;

#define DATAENTRY_FIELDS	\
	char *location;

typedef struct tagDataEntryAtom
{
	FULL_ATOM
	DATAENTRY_FIELDS
} DataEntryAtom;

typedef struct tagDataEntryURLAtom
{
	FULL_ATOM
	DATAENTRY_FIELDS
} DataEntryURLAtom;

typedef struct tagDataEntryURNAtom
{
	FULL_ATOM
	DATAENTRY_FIELDS
	char *nameURN;
} DataEntryURNAtom;

typedef struct tagDataReferenceAtom
{
	FULL_ATOM
	Chain *atomList;
} DataReferenceAtom;

typedef struct tagSampleTableAtom
{
	BASE_ATOM
	struct tagTimeToSampleAtom *TimeToSample;
	struct tagCompositionOffsetAtom *CompositionOffset;
	struct tagSyncSampleAtom *SyncSample;
	struct tagSampleDescriptionAtom *SampleDescription;
	struct tagSampleSizeAtom *SampleSize;
	struct tagSampleToChunkAtom *SampleToChunk;
	Atom *ChunkOffset;		//to handle normal and large chunk
	struct tagShadowSyncAtom *ShadowSync;
	struct tagDegradationPriorityAtom *DegradationPriority;
	struct _tagPaddingBitsAtom *PaddingBits;
	struct tagSampleFragmentAtom *Fragments;

	u32 MaxSamplePerChunk;
	u16 groupID;
	u16 trackPriority;
	u32 currentEntryIndex;
} SampleTableAtom;

//used for time to sample atom
typedef struct
{
	u32 sampleCount;
	u32 sampleDelta;
} sttsEntry;

typedef struct tagTimeToSampleAtom
{
	FULL_ATOM
	Chain *entryList;
#ifndef M4_READ_ONLY
	//DTS cache for WRITE
	sttsEntry *w_currentEntry;
	u32 w_currentSampleNum;
	u32 w_LastDTS;
#endif
	//DTS cache for READ
	u32 r_FirstSampleInEntry;
	u32 r_currentEntryIndex;
	u32 r_CurrentDTS;
} TimeToSampleAtom;

//used for CompositionOffset
typedef struct
{
	u32 sampleCount;
	u32 decodingOffset;
} dttsEntry;

typedef struct tagCompositionOffsetAtom
{
	FULL_ATOM
	Chain *entryList;
#ifndef M4_READ_ONLY
	//CTS Cache for write
	dttsEntry *w_currentEntry;
	u32 w_LastSampleNumber;
	/*force one sample per entry*/
	Bool unpack_mode;
#endif
	//CTS Cache for read
	u32 r_currentEntryIndex;
	u32 r_FirstSampleInEntry;
} CompositionOffsetAtom;


//used for SampleFragment
typedef struct
{
	u32 SampleNumber;
	u32 fragmentCount;
	u16 *fragmentSizes;
} stsfEntry;

typedef struct tagSampleFragmentAtom
{
	FULL_ATOM
	Chain *entryList;
#ifndef M4_READ_ONLY
	//CTS Cache for write
	stsfEntry *w_currentEntry;
	u32 w_currentEntryIndex;
#endif
	//CTS Cache for read
	u32 r_currentEntryIndex;
	stsfEntry *r_currentEntry;
} SampleFragmentAtom;


#define SAMPLE_ENTRY_FIELDS		\
	u16 dataReferenceIndex;	\
	char reserved[ 6 ];

//this is an abstract struct
typedef struct tagSampleEntryAtom
{
	BASE_ATOM
	SAMPLE_ENTRY_FIELDS
} SampleEntryAtom;

typedef struct tagGenericMediaSampleEntryAtom
{
	BASE_ATOM
	SAMPLE_ENTRY_FIELDS
	/*atom type as specified in the file (not this atom's type!!)*/
	u32 EntryType;

	char *data;
	u32 data_size;
} GenericMediaSampleEntryAtom;

//this is the common struct for MPEG4 media
typedef struct tagMPEGSampleEntryAtom
{
	BASE_ATOM
	SAMPLE_ENTRY_FIELDS
	struct tagESDAtom *esd;
	//used for Publishing
	SLConfigDescriptor *slc;
} MPEGSampleEntryAtom;


#define VISUAL_SAMPLE_ENTRY_BASE		\
	BASE_ATOM							\
	SAMPLE_ENTRY_FIELDS					\
	u16 version;						\
	u16 revision;						\
	u32 vendor;							\
	u32 temporal_quality;				\
	u32 spacial_quality;				\
	u16 Width, Height;					\
	u32 horiz_res, vert_res;			\
	u32 entry_data_size;				\
	u16 frames_per_sample;				\
	unsigned char compressor_name[33];	\
	u16 bit_depth;						\
	s16 color_table_index;

typedef struct tagVisualSampleEntryAtom
{
	VISUAL_SAMPLE_ENTRY_BASE
} VisualSampleEntryAtom;

void InitVideoSampleEntry(VisualSampleEntryAtom *ent);
void ReadVideoSampleEntry(VisualSampleEntryAtom *ptr, BitStream *bs, u64 *read);
#ifndef M4_READ_ONLY
void WriteVideoSampleEntry(VisualSampleEntryAtom *ent, BitStream *bs);
void SizeVideoSampleEntry(VisualSampleEntryAtom *ent);
#endif


typedef struct tagMPEGVisualSampleEntryAtom
{
	VISUAL_SAMPLE_ENTRY_BASE

	struct tagESDAtom *esd;
	//used for Publishing
	SLConfigDescriptor *slc;
} MPEGVisualSampleEntryAtom;


typedef struct
{
	BASE_ATOM
	u32 vendor;
	u8 decoder_version;
	u8 Profile;
	u8 Level;
} H263ConfigAtom;

typedef struct
{
	VISUAL_SAMPLE_ENTRY_BASE
	H263ConfigAtom *h263_config;
} H263SampleEntryAtom;

/*this is the default visual sdst (to handle unknown media)*/
typedef struct tagGenericVisualSampleEntryAtom
{
	VISUAL_SAMPLE_ENTRY_BASE
	/*atom type as specified in the file (not this atom's type!!)*/
	u32 EntryType;
	/*opaque description data (ESDS in MP4, SMI in SVQ3, ...)*/
	char *data;
	u32 data_size;
} GenericVisualSampleEntryAtom;


typedef struct tagMPEG4BitRateAtom
{
	BASE_ATOM
	u32 bufferSizeDB;
	u32 maxBitrate;
	u32 avgBitrate;
} MPEG4BitRateAtom;

typedef struct tagMPEG4ExtensionDescriptorsAtom
{
	BASE_ATOM
	Chain *descriptors;
} MPEG4ExtensionDescriptorsAtom;

typedef struct tagAVCConfigurationAtom
{
	BASE_ATOM
	AVCDecoderConfigurationRecord *config;
} AVCConfigurationAtom;

typedef struct tagAVCSampleEntryAtom
{
	VISUAL_SAMPLE_ENTRY_BASE

	AVCConfigurationAtom *avc_config;
	MPEG4BitRateAtom *bitrate;
	/*ext descriptors*/
	MPEG4ExtensionDescriptorsAtom *descr;
	/*used for Publishing*/
	SLConfigDescriptor *slc;
	/*internally emulated esd*/
	ESDescriptor *esd;
} AVCSampleEntryAtom;


#define AUDIO_SAMPLE_ENTRY_BASE		\
	BASE_ATOM						\
	SAMPLE_ENTRY_FIELDS				\
	u16 version;					\
	u16 revision;					\
	u32 vendor;						\
	u16 channel_count;				\
	u16 bitspersample;				\
	u16 compression_id;				\
	u16 packet_size;				\
	u16 samplerate_hi;				\
	u16 samplerate_lo;

typedef struct 
{
	AUDIO_SAMPLE_ENTRY_BASE
} AudioSampleEntryAtom;

void InitAudioSampleEntry(AudioSampleEntryAtom *ptr);
void ReadAudioSampleEntry(AudioSampleEntryAtom *ptr, BitStream *bs, u64 *read);
#ifndef M4_READ_ONLY
void WriteAudioSampleEntry(AudioSampleEntryAtom *ptr, BitStream *bs);
void SizeAudioSampleEntry(AudioSampleEntryAtom *ptr);
#endif	//M4_READ_ONLY


typedef struct tagMPEGAudioSampleEntryAtom
{
	AUDIO_SAMPLE_ENTRY_BASE
	struct tagESDAtom *esd;
	//used for Publishing
	SLConfigDescriptor *slc;
} MPEGAudioSampleEntryAtom;

typedef struct
{
	AUDIO_SAMPLE_ENTRY_BASE
	struct _tagAMRConfig *amr_info;
} AMRSampleEntryAtom;

typedef struct _tagAMRConfig
{
	BASE_ATOM
	u32 vendor;
	u8 decoder_version;
	u16 mode_set;
	u8 mode_change_period;
	u8 frames_per_sample;
} AMRConfigAtom;


/*this is the default visual sdst (to handle unknown media)*/
typedef struct tagGenericAudioSampleEntryAtom
{
	AUDIO_SAMPLE_ENTRY_BASE
	/*atom type as specified in the file (not this atom's type!!)*/
	u32 EntryType;
	/*opaque description data (ESDS in MP4, ...)*/
	char *data;
	u32 data_size;
} GenericAudioSampleEntryAtom;


typedef struct tagSampleDescriptionAtom
{
	FULL_ATOM
	Chain *atomList;
} SampleDescriptionAtom;


typedef struct tagESDAtom
{
	FULL_ATOM
	ESDescriptor *desc;
} ESDAtom;

typedef struct tagSampleSizeAtom
{
	FULL_ATOM
	//if this is the compact version, sample size is actually fieldSize
	u32 sampleSize;
	u32 sampleCount;
 	u32 alloc_size;
	u32 *sizes;
} SampleSizeAtom;

typedef struct tagChunkOffsetAtom
{
	FULL_ATOM
	u32 entryCount;
	u32 alloc_size;
	u32 *offsets;
} ChunkOffsetAtom;

typedef struct tagChunkLargeOffsetAtom
{
	FULL_ATOM
	u32 entryCount;
	u32 alloc_size;
	u64 *offsets;
} ChunkLargeOffsetAtom;

//used in SampleToChunk
typedef struct
{
	u32 firstChunk;
	u32 nextChunk;
	u32 samplesPerChunk;
	u32 sampleDescriptionIndex;
	u8 isEdited;
} stscEntry;


typedef struct tagSampleToChunkAtom
{
	FULL_ATOM
	Chain *entryList;
	stscEntry *currentEntry;
	//Cache for READ. In WRITE mode, we always have 1 sample per chunk
	//so no need for a cache
	//0-based entry index ...
	u32 currentIndex;
	//first sample number in this chunk
	u32 firstSampleInCurrentChunk;
	u32 currentChunk;
	u32 ghostNumber;
} SampleToChunkAtom;

typedef struct tagSyncSampleAtom
{
	FULL_ATOM
	u32 entryCount;
	u32 *sampleNumbers;
	//cache for READ mode (in write we realloc no matter what)
	u32 r_LastSyncSample;
	//0-based index in the array
	u32 r_LastSampleIndex;
} SyncSampleAtom;

//used in ShadowSync
typedef struct
{
	u32 shadowedSampleNumber;
	s32 syncSampleNumber;
} stshEntry;

typedef struct tagShadowSyncAtom
{
	FULL_ATOM
	Chain *entries;
	//Cache for read mode
	u32 r_LastEntryIndex;
	u32 r_LastFoundSample;
} ShadowSyncAtom;

typedef struct tagDegradationPriorityAtom
{
	FULL_ATOM
	u32 entryCount;
	u16 *priorities;
} DegradationPriorityAtom;

typedef struct FreeSpaceAtom
{
	BASE_ATOM
	char *data;
	u32 dataSize;
} FreeSpaceAtom;

typedef struct tagEditAtom
{
	BASE_ATOM
	Chain *atomList;
	struct tagEditListAtom *editList;
} EditAtom;


//used for entry list
typedef struct
{
	u64 segmentDuration;
	s64 mediaTime;
	u32 mediaRate;
} edtsEntry;

typedef struct tagEditListAtom
{
	FULL_ATOM
	Chain *entryList;
} EditListAtom;

//used to classify atoms in the UserData Atom
typedef struct
{
	u32 atomType;
	u8 uuid[16];
	Chain *atomList;
} UserDataMap;

typedef struct tagUserDataAtom
{
	BASE_ATOM
	Chain *recordList;
	//we use a void atom to close the udta...
	Atom *voidAtom;
} UserDataAtom;

typedef struct tagCopyrightAtom
{
	FULL_ATOM
	char packedLanguageCode[4];
	char *notice;
} CopyrightAtom;

#define WatermarkAtom UnknownAtom

typedef struct tagTrackReferenceTypeAtom
{
	BASE_ATOM
	u32 trackIDCount;
	u32 *trackIDs;
} TrackReferenceTypeAtom;

//
//		V2 atoms
//

typedef struct tagFileTypeAtom
{
	BASE_ATOM
	u32 majorBrand;
	u32 minorVersion;
	u32 altCount;
	u32 *altBrand;
} FileTypeAtom;

typedef struct _tagPaddingBitsAtom
{
	FULL_ATOM
	u32 SampleCount;
	u8 *padbits;
} PaddingBitsAtom;


/*
	3GPP streaming text atoms
*/

typedef struct
{
	BASE_ATOM
	u32 entry_count;
	FontRecord *fonts;
} FontTableAtom;

typedef struct
{
	BASE_ATOM						\
	SAMPLE_ENTRY_FIELDS				\
	u32 displayFlags;
	s8 horizontal_justification;
	s8 vertical_justification;
	/*ARGB*/
	u32 back_color;
	BoxRecord default_box; 
	StyleRecord	default_style;
	FontTableAtom *font_table;
} TextSampleEntryAtom;

typedef struct
{
	BASE_ATOM
	u32 entry_count;
	StyleRecord *styles;
} TextStyleAtom;

typedef struct
{
	BASE_ATOM
	u16 startcharoffset;
	u16 endcharoffset; 
} TextHighlightAtom;

typedef struct
{
	BASE_ATOM
	/*ARGB*/
	u32 hil_color;
} TextHighlightColorAtom;

typedef struct
{
	u32 highlight_endtime;
	u16 start_charoffset;
	u16 end_charoffset;
} KaraokeRecord;

typedef struct
{
	BASE_ATOM
	u32 highlight_starttime;
	u16 entrycount;
	KaraokeRecord *records;
} TextKaraokeAtom;

typedef struct
{
	BASE_ATOM
	u32 scroll_delay;
} TextScrollDelayAtom;

typedef struct
{
	BASE_ATOM
	u16 startcharoffset;
	u16 endcharoffset;
	char *URL;
	char *URL_hint;
} TextHyperTextAtom;

typedef struct
{
	BASE_ATOM
	BoxRecord box;
} TextBoxAtom;

typedef struct
{
	BASE_ATOM
	u16 startcharoffset;
	u16 endcharoffset;
} TextBlinkAtom;

typedef struct
{
	BASE_ATOM
	u8 wrap_flag;
} TextWrapAtom;


#ifndef	M4_ISO_NO_FRAGMENTS
//
//V2 atoms - Movie Fragments

//MovieExtends is just a container IN THE MOOV ATOM
typedef struct tagMovieExtendsAtom
{
	BASE_ATOM
	Chain *TrackExList;
	struct tagM4File *mov;
} MovieExtendsAtom;

//the TrackExtends contains default values for the track fragments
typedef struct tagTrackExtendsAtom
{
	FULL_ATOM
	u32 trackID;
	u32 def_sample_desc_index;
	u32 def_sample_duration;
	u32 def_sample_size;
	u32 def_sample_flags;
	TrackAtom *track;
} TrackExtendsAtom;

//MovieFragment is a container IN THE FILE, contains 1 fragment
typedef struct tagMovieFragmentAtom
{
	BASE_ATOM
	struct tagMovieFragmentHeaderAtom *mfhd;
	Chain *TrackList;
	struct tagM4File *mov;
} MovieFragmentAtom;

//indicates the seq num of this fragment
typedef struct tagMovieFragmentHeaderAtom
{
	FULL_ATOM
	u32 sequence_number;
} MovieFragmentHeaderAtom;


typedef struct tagTrackFragmentAtom
{
	BASE_ATOM
	struct tagTrackFragmentHeaderAtom *tfhd;
	Chain *TrackRuns;
	//keep a pointer to default flags
	TrackExtendsAtom *trex;
	//when data caching is on
	u32 DataCache;
} TrackFragmentAtom;

//FLAGS for TRAF
enum
{
	TF_BASE_OFFSET	=	0x01,
	TF_SAMPLE_DESC	=	0x02,
	TF_SAMPLE_DUR	=	0x08,
	TF_SAMPLE_SIZE	=	0x10,
	TF_SAMPLE_FLAGS	=	0x20,
	TF_DUR_EMPTY	=	0x10000
};

typedef struct tagTrackFragmentHeaderAtom
{
	FULL_ATOM
	u32 trackID;
	// all the following are optional fields 
	u64 base_data_offset;
	u32 sample_desc_index;
	u32 def_sample_duration;
	u32 def_sample_size;
	u32 def_sample_flags;
	u32 EmptyDuration;
	u8 IFrameSwitching;
} TrackFragmentHeaderAtom;

//FLAGS for TRUN : specify what is written in the SampleTable of TRUN
enum
{
	TR_DATA_OFFSET		= 0x01,
	TR_FIRST_FLAG		= 0x04,
	TR_DURATION			= 0x100,
	TR_SIZE				= 0x200,
	TR_FLAGS			= 0x400,
	TR_CTS_OFFSET		= 0x800
};

typedef struct tagTrackFragmentRunAtom
{
	FULL_ATOM
	u32 sample_count;
	// the following are optional fields 
	s32 data_offset;
	u32 first_sample_flags;
	//can be empty
	Chain *entries;

	//in write mode with data caching
	BitStream *cache;
} TrackFragmentRunAtom;

typedef struct tagTrunEntry
{
	u32 Duration;
	u32 size;
	u32 flags;
	u32 CTS_Offset;
} TrunEntry;


#endif


//RTP Hint Track Sample Entry
typedef struct tagHintSampleEntryAtom
{
	BASE_ATOM
	SAMPLE_ENTRY_FIELDS
	u16 HintTrackVersion;
	u16 LastCompatibleVersion;
	//this is where we store the current RTP sample in write mode
	struct _tagHintSample *w_sample;
	u32 MaxPacketSize;
	Chain *HintDataTable;		//can be empty...
} HintSampleEntryAtom;


typedef struct tagRTPAtom
{
	BASE_ATOM
	u32 subType;
	char *sdpText;
} RTPAtom;

typedef struct tagSDPAtom
{
	BASE_ATOM
	char *sdpText;
} SDPAtom;

typedef struct tagRtpoAtom
{
	BASE_ATOM
	s32 timeOffset;
} RtpoAtom;

//Hint Track Info Atom
typedef struct tagHintTrackInfoAtom
{
	BASE_ATOM
	//contains SDPAtom if in track, RTPAtom if in movie (QT Spec, cross-check with MP4 VM
	Atom *SDP;
	Chain *atomList;
} HintTrackInfoAtom;

//Generic Rely indication (not specific to RTP)
typedef struct tagRelyHintEntry
{
	BASE_ATOM
	u8 reserved;		//set to 0
	u8 prefered;
	u8 required;
} RelyHintEntry;

/***********************************************************
			data entry tables for RTP
***********************************************************/
//RTP Time Scale
typedef struct tagTSHintEntry
{
	BASE_ATOM
	u32 timeScale;
} TSHintEntry;

//RTP Time Offset
typedef struct tagTimeOffHintEntry
{
	BASE_ATOM
	u32 TimeOffset;	
} TimeOffHintEntry;

//RTP Pqcket Sequence Number Offset
typedef struct tagSeqOffHintEntry
{
	BASE_ATOM
	u32 SeqOffset;
} SeqOffHintEntry;



/***********************************************************
			hint track information atoms for RTP
***********************************************************/

//Total number of bytes that will be sent, including 12-byte
//RTP headers, but not including any network headers
typedef struct tagTrpyAtom
{
	BASE_ATOM
	u64 nbBytes;
} TrpyAtom;

//32-bits version of trpy used in Darwin
typedef struct tagTotlAtom
{
	BASE_ATOM
	u32 nbBytes;
} TotlAtom;

//Total number of network packets that will be sent
typedef struct tagNumpAtom
{
	BASE_ATOM
	u64 nbPackets;
} NumpAtom;

//32-bits version of nump used in Darwin
typedef struct tagNpckAtom
{
	BASE_ATOM
	u32 nbPackets;
} NpckAtom;


//Total number of bytes that will be sent, not including
//12-byte RTP headers
typedef struct tagTpylAtom
{
	BASE_ATOM
	u64 nbBytes;
} TpylAtom;

//32-bits version of tpyl used in Darwin
typedef struct tagTpayAtom
{
	BASE_ATOM
	u32 nbBytes;
} TpayAtom;

//Maximum data rate in bits per second.
typedef struct tagMaxrAtom
{
	BASE_ATOM
	u32 granularity;
	u32 maxDataRate;
} MaxrAtom;


//Total number of bytes from the media track to be sent
typedef struct tagDmedAtom
{
	BASE_ATOM
	u64 nbBytes;
} DmedAtom;

//Number of bytes of immediate data to be sent
typedef struct tagDimmAtom
{
	BASE_ATOM
	u64 nbBytes;
} DimmAtom;


//Number of bytes of repeated data to be sent
typedef struct tagDrepAtom
{
	BASE_ATOM
	u64 nbBytes;
} DrepAtom;

//Smallest relative transmission time, in milliseconds.
//signed integer for smoothing
typedef struct tagTminAtom
{
	BASE_ATOM
	s32 minTime;
} TminAtom;

//Largest relative transmission time, in milliseconds.
typedef struct tagTmaxAtom
{
	BASE_ATOM
	s32 maxTime;
} TmaxAtom;

//Largest packet, in bytes, including 12-byte RTP header
typedef struct tagPmaxAtom
{
	BASE_ATOM
	u32 maxSize;
} PmaxAtom;

//Longest packet duration, in milliseconds
typedef struct tagDmaxAtom
{
	BASE_ATOM
	u32 maxDur;
} DmaxAtom;

//32-bit payload type number, followed by rtpmap
//payload string (Pascal string)
typedef struct tagPaytAtom
{
	BASE_ATOM
	u32 payloadCode;
	char *payloadString;
} PaytAtom;


typedef struct tagNameAtom
{
	BASE_ATOM
	char *string;
} NameAtom;

typedef struct tagHintInfoAtom
{
	BASE_ATOM

	Atom *numBytes;
	Atom *numPack;
	TpylAtom *nbDataBytes;
	DmedAtom *nbMediaBytes;
	DimmAtom *nbImmediateBytes;
	DrepAtom *nbRepeatedBytes;
	TminAtom *minTransTime;
	TmaxAtom *maxTransTime;
	PmaxAtom *maxPackSize;
	DmaxAtom *maxPackDur;
	PaytAtom *payload;
	Chain *dataRates;
	Chain *atomList;
} HintInfoAtom;



/*
		Data Map (media storage) stuff
*/

/*regular file IO*/
#define DM_FILE			0x01
/*File Mapaing object, read-only mode on complete files (no download)*/
#define DM_MAPPING		0x02
/*External file object. Needs implementation*/
#define DM_FILE_EXT		0x03

/*Data Map modes*/
enum
{
	/*read mode*/
	DM_MODE_R = 1,
	/*write mode*/
	DM_MODE_W = 2,
	/*the following modes are just ways of signaling extended functionalities
	edit mode, to make sure the file is here, set to DM_MODE_R afterwards*/
	DM_MODE_E = 3,
	/*read-only access to the movie file: we create a file mapping object
	mode is set to DM_MODE_R afterwards*/
	DM_MODE_RO = 4,
};

/*this is the DataHandler structure each data handler has its own bitstream*/
#define BASE_DATA_HANDLER	\
	u8	type;		\
	u64	curPos;		\
	u8	mode;		\
	BitStream *bs;

typedef struct tagDataMap 
{
	BASE_DATA_HANDLER
} DataMap;

typedef struct tagFileDataMap 
{
	BASE_DATA_HANDLER
	FILE *stream;
	Bool last_acces_was_read;
#ifndef M4_READ_ONLY
	char *temp_file;
#endif
} FileDataMap;

typedef struct tagMemoryDataMap 
{
	BASE_DATA_HANDLER
} MemoryDataMap;

/*file mapping handler. used if supported, only on read mode for complete files  (not in file download)*/
typedef struct tagFileMappingDataMap 
{
	BASE_DATA_HANDLER
	char *name;
	u32 file_size;
	char *byte_map;
	u32 byte_pos;
} FileMappingDataMap;

M4Err DataMap_New(const char *location, const char *parentPath, u8 mode, DataMap **outDataMap);
void DataMap_Delete(DataMap *ptr);
M4Err DataMap_Open(MediaAtom *minf, u32 dataRefIndex, u8 Edit);
void DataMap_Close(MediaInformationAtom *minf);
u32 DataMap_GetData(DataMap *map, char *buffer, u32 bufferLength, u64 Offset);

/*File-based data map*/
DataMap *FDM_New(const char *sPath, u8 mode);
void FDM_Delete(FileDataMap *ptr);
u32 FDM_GetData(FileDataMap *ptr, char *buffer, u32 bufferLength, u64 fileOffset);

#ifndef M4_READ_ONLY
DataMap *FDM_NewTemp(const char *sTempPath);
#endif

/*file-mapping, read only*/
DataMap *FMO_New(const char *sPath, u8 mode);
void FMO_Delete(FileMappingDataMap *ptr);
u32 FMO_GetData(FileMappingDataMap *ptr, char *buffer, u32 bufferLength, u64 fileOffset);

#ifndef M4_READ_ONLY
u64 DataMap_GetTotalOffset(DataMap *map);
M4Err DataMap_AddData(DataMap *ptr, char *data, u32 dataSize);
u64 FDM_GetTotalOffset(FileDataMap *ptr);
M4Err FDM_AddData(FileDataMap *ptr, char *data, u32 dataSize);
#endif	//M4_READ_ONLY

/*
		Movie stuff
*/


/*time def for MP4/QT/MJ2K files*/
#define MAC_TIME_OFFSET 2082758400

#ifndef	M4_ISO_NO_FRAGMENTS
#define FORMAT_FRAG_FLAGS(pad, sync, deg) ( ( (pad) << 17) | ( ( !(sync) ) << 16) | (deg) );
#define GET_FRAG_PAD(flag) ( (flag) >> 17) & 0x7
#define GET_FRAG_SYNC(flag) ( ! ( ( (flag) >> 16) & 0x1))
#define GET_FRAG_DEG(flag)	(flag) & 0x7FFF
#endif

enum
{
	FRAG_WRITE_READY	=	0x01,
	FRAG_READ_DEBUG		=	0x02,
};

/*this is our movie object*/
typedef struct tagM4File {
	/*the last fatal error*/
	M4Err LastError;
	/*the original filename*/
	char *fileName;
	/*the original file in read/edit, and also used in fragments mode
	once the first moov has been written
	Nota: this API doesn't allow fragments BEFORE the MOOV in order
	to make easily parsable files (note there could be some data (mdat) before
	the moov*/
	DataMap *movieFileMap;

#ifndef M4_READ_ONLY
	/*the final file name*/
	char *finalName;
	/*the file where we store edited samples (for READ_WRITE and WRITE mode only)*/
	DataMap *editFileMap;
	/*the interleaving time for dummy mode (in movie TimeScale)*/
	u32 interleavingTime;
#endif

	u8 openMode;
	u8 storageMode;

	MovieAtom *moov;
	/*our MDAT atom (one and only one when we store the file)*/
	MediaDataAtom *mdat;
	/*this contains all the root atoms*/
	Chain *TopAtoms;
	/*since v2 (NULL means v1)*/
	FileTypeAtom *brand;
	/*default track for sync of MPEG4 streams - this is the first accessed stream without OCR info - only set in READ mode*/
	s32 es_id_default_sync;
	/*if true 3GPP text streams are read as MPEG-4 StreamingText*/
	Bool convert_streaming_text;

#ifndef	M4_ISO_NO_FRAGMENTS
	/*mvex fragments in the file - read only*/
	Chain *MovieFragments;
	u32 FragmentsFlags, NextMoofNumber;
	/*active fragment*/
	MovieFragmentAtom *moof;
	/*in WRITE mode, this is the current MDAT where data is written*/
	/*in READ mode this is the last valid file position before a ReadAtom failed*/
	u64 current_top_atom_start;
#endif
} M4Movie;

/*time function*/
u64 GetMP4Time();
/*set the last error of the file. if file is NULL, set the static error (used for IO errors*/
void M4SetLastError(struct tagM4File *the_file, M4Err error);
M4Err ParseMovieAtoms(M4Movie *mov, u64 *bytesMissing);
M4Movie *M4NewMovie();
/*Movie and Track access functions*/
M4Movie *GetMovie(struct tagM4File *theFile);
TrackAtom *GetTrackFromFile(struct tagM4File *the_file, u32 trackNumber);
TrackAtom *GetTrack(MovieAtom *moov, u32 trackNumber);
TrackAtom *GetTrackFromID(MovieAtom *moov, u32 trackID);
M4Err GetTrackId(MovieAtom *moov, u32 trackNumber, u32 *trackID);
u32 GetTrackNumberByID(MovieAtom *moov, u32 trackID);
/*open a movie*/
M4Movie *MovieOpen(const char *fileName, u8 OpenMode);
/*close and delete a movie*/
void DelMovie(M4Movie *mov);
/*StreamDescription reconstruction Functions*/
M4Err GetESD(MovieAtom *moov, u32 trackID, u32 StreamDescIndex, ESDescriptor **outESD);
M4Err GetESDForTime(MovieAtom *moov, u32 trackID, u64 CTS, ESDescriptor **outESD);
M4Err Media_GetSampleDesc(MediaAtom *mdia, u32 SampleDescIndex, SampleEntryAtom **out_entry, u32 *dataRefIndex);
M4Err Media_GetSampleDescIndex(MediaAtom *mdia, u64 DTS, u32 *sampleDescIndex);
/*get esd for given sample desc - 
	@true_desc_only: if true doesn't emulate desc and returns native ESD,
				otherwise emulates if needed/possible (TimedText) and return a hard copy of the desc
*/
M4Err Media_GetESD(MediaAtom *mdia, u32 sampleDescIndex, ESDescriptor **esd, Bool true_desc_only);
Bool Track_IsMPEG4Stream(u32 HandlerType);
Bool IsMP4Description(u32 entryType);
/*Find a reference of a given type*/
M4Err Track_FindRef(TrackAtom *trak, u32 ReferenceType, TrackReferenceTypeAtom **dpnd);
/*Time and sample*/
M4Err GetMediaTime(TrackAtom *trak, u32 movieTime, u64 *MediaTime, s64 *SegmentStartTime, s64 *MediaOffset, u8 *useEdit);
M4Err Media_GetSample(MediaAtom *mdia, u32 sampleNumber, M4Sample **samp, u32 *sampleDescriptionIndex, Bool no_data, u64 *out_offset);
M4Err Media_CheckDataEntry(MediaAtom *mdia, u32 dataEntryIndex);
M4Err Media_FindSyncSample(SampleTableAtom *stbl, u32 searchFromTime, u32 *sampleNumber, u8 mode);
M4Err Media_RewriteODFrame(MediaAtom *mdia, M4Sample *sample);
M4Err Media_FindDataRef(DataReferenceAtom *dref, char *URLname, char *URNname, u32 *dataRefIndex);
Bool Media_IsSelfContained(MediaAtom *mdia, u32 StreamDescIndex);

/*check the TimeToSample for the given time and return the Sample number
if the entry is not found, return the closest sampleNumber in prevSampleNumber and 0 in sampleNumber
if the DTS required is after all DTSs in the list, set prevSampleNumber and SampleNumber to 0
useCTS specifies that we're looking for a composition time
*/
M4Err findEntryForTime(SampleTableAtom *stbl, u32 DTS, u8 useCTS, u32 *sampleNumber, u32 *prevSampleNumber);
/*Reading of the sample tables*/
M4Err stbl_GetSampleSize(SampleSizeAtom *stsz, u32 SampleNumber, u32 *Size);
M4Err stbl_GetSampleCTS(CompositionOffsetAtom *ctts, u32 SampleNumber, u32 *CTSoffset);
M4Err stbl_GetSampleDTS(TimeToSampleAtom *stts, u32 SampleNumber, u32 *DTS);
/*find a RAP or set the prev / next RAPs if vars are passed*/
M4Err stbl_GetSampleRAP(SyncSampleAtom *stss, u32 SampleNumber, u8 *IsRAP, u32 *prevRAP, u32 *nextRAP);
M4Err stbl_GetSampleInfos(SampleTableAtom *stbl, u32 sampleNumber, u64 *offset, u32 *chunkNumber, u32 *descIndex, u8 *isEdited);
M4Err stbl_GetSampleShadow(ShadowSyncAtom *stsh, u32 *sampleNumber, u32 *syncNum);
M4Err stbl_GetPaddingBits(PaddingBitsAtom *padb, u32 SampleNumber, u8 *PadBits);
u32 stbl_GetSampleFragmentCount(SampleFragmentAtom *stsf, u32 sampleNumber);
u32 stbl_GetSampleFragmentSize(SampleFragmentAtom *stsf, u32 sampleNumber, u32 FragmentIndex);
/*unpack sample2chunk and chunk offset so that we have 1 sample per chunk (edition mode only)*/
M4Err stbl_UnpackOffsets(SampleTableAtom *stbl);
M4Err SetTrackDuration(TrackAtom *trak);
M4Err Media_SetDuration(TrackAtom *trak);

/*rewrites 3GP samples desc as MPEG-4 ESD*/
M4Err M4_GetStreamingTextESD(MediaAtom *mdia, ESDescriptor **out_esd);
/*inserts TTU header - only used when conversion to StreamingText is on*/
M4Err M4_RewriteTextSample(M4Sample *samp, u32 sampleDescriptionIndex, u32 sample_dur);

#ifndef M4_READ_ONLY

M4Err CanAccessMovie(M4Movie *movie, u32 Mode);
M4Movie *MovieCreate(const char *fileName, u8 OpenMode);
M4Err WriteToFile(M4Movie *movie);
M4Err Track_SetStreamDescriptor(TrackAtom *trak, u32 StreamDescriptionIndex, u32 DataReferenceIndex, ESDescriptor *esd, u32 *outStreamIndex);
u8 RequestTrack(MovieAtom *moov, u32 TrackID);
/*Track-Media setup*/
M4Err NewMedia(MediaAtom **mdia, u32 MediaType, u32 TimeScale);
M4Err Media_ParseODFrame(MediaAtom *mdia, M4Sample *sample);
M4Err Media_AddSample(MediaAtom *mdia, u64 data_offset, M4Sample *sample, u32 StreamDescIndex, u32 syncShadowNumber);
M4Err Media_CreateDataRef(DataReferenceAtom *dref, char *URLname, char *URNname, u32 *dataRefIndex);
/*update a media sample. ONLY in edit mode*/
M4Err Media_UpdateSample(MediaAtom *mdia, u32 sampleNumber, M4Sample *sample);
M4Err Media_UpdateSampleReference(MediaAtom *mdia, u32 sampleNumber, M4Sample *sample, u64 data_offset);
/*addition in the sample tables*/
M4Err stbl_AddDTS(SampleTableAtom *stbl, u32 DTS, u32 *sampleNumber, u32 LastAUDefDuration);
M4Err stbl_AddCTS(SampleTableAtom *stbl, u32 sampleNumber, u32 CTSoffset);
M4Err stbl_AddSize(SampleSizeAtom *stsz, u32 sampleNumber, u32 size);
M4Err stbl_AddRAP(SyncSampleAtom *stss, u32 sampleNumber);
M4Err stbl_AddShadow(ShadowSyncAtom *stsh, u32 sampleNumber, u32 shadowNumber);
M4Err stbl_AddChunkOffset(MediaAtom *mdia, u32 sampleNumber, u32 StreamDescIndex, u64 offset);
/*NB - no add for padding, this is done only through SetPaddingBits*/

M4Err stbl_AddSampleFragment(SampleTableAtom *stbl, u32 sampleNumber, u16 size);

/*update of the sample table
all these functions are called in edit and we always have 1 sample per chunk*/
M4Err stbl_SetChunkOffset(MediaAtom *mdia, u32 sampleNumber, u64 offset);
M4Err stbl_SetSampleCTS(SampleTableAtom *stbl, u32 sampleNumber, u32 offset);
M4Err stbl_SetSampleSize(SampleSizeAtom *stsz, u32 SampleNumber, u32 size);
M4Err stbl_SetSampleRAP(SyncSampleAtom *stss, u32 SampleNumber, u8 isRAP);
M4Err stbl_SetSyncShadow(ShadowSyncAtom *stsh, u32 sampleNumber, u32 syncSample);
M4Err stbl_SetPaddingBits(SampleTableAtom *stbl, u32 SampleNumber, u8 bits);
/*for adding fragmented samples*/
M4Err stbl_SampleSizeAppend(SampleSizeAtom *stsz, u32 data_size);
/*writing of the final chunk info in edit mode*/
M4Err stbl_SetChunkAndOffset(SampleTableAtom *stbl, u32 sampleNumber, u32 StreamDescIndex, SampleToChunkAtom *the_stsc, Atom **the_stco, u64 data_offset, u8 forceNewChunk);
M4Err stbl_AddOffset(Atom **a, u64 offset);
/*EDIT LIST functions*/
edtsEntry *CreateEditEntry(u32 EditDuration, u32 MediaTime, u8 EditMode);

/*REMOVE functions*/
M4Err stbl_RemoveDTS(SampleTableAtom *stbl, u32 sampleNumber, u32 LastAUDefDuration);
M4Err stbl_RemoveCTS(SampleTableAtom *stbl, u32 sampleNumber);
M4Err stbl_RemoveSize(SampleSizeAtom *stsz, u32 sampleNumber);
M4Err stbl_RemoveChunk(SampleTableAtom *stbl, u32 sampleNumber);
M4Err stbl_RemoveRAP(SampleTableAtom *stbl, u32 sampleNumber);
M4Err stbl_RemoveShadow(ShadowSyncAtom *stsh, u32 sampleNumber);
M4Err stbl_RemovePaddingBits(SampleTableAtom *stbl, u32 SampleNumber);
M4Err stbl_RemoveSampleFragments(SampleTableAtom *stbl, u32 sampleNumber);

#ifndef	M4_ISO_NO_FRAGMENTS
M4Err StoreFragment(M4Movie *movie);
#endif


#endif	//M4_READ_ONLY

Bool IsHintTrack(TrackAtom *trak);
Bool CheckHintFormat(TrackAtom *trak, u32 HintType);
u32 GetHintFormat(TrackAtom *trak);



/*
		Hinting stuff
*/

/*the HintType for each protocol*/
enum
{
	PCK_RTP				= 1,
	PCK_MPEG2TS			= 2,
	PCK_MAX_TYPE		= 255
};

/*****************************************************
		RTP Data Entries
*****************************************************/

#define BASE_DTE_ENTRY	\
	u8 source;

typedef struct tagGenericDTE
{
	BASE_DTE_ENTRY
} GenericDTE;

typedef struct tagEmptyDTE
{
	BASE_DTE_ENTRY
} EmptyDTE;

typedef struct tagImmediateDTE
{
	BASE_DTE_ENTRY
	u8 dataLength;
	char data[14];
} ImmediateDTE;

typedef struct tagSampleDTE
{
	BASE_DTE_ENTRY
	s8 trackRefIndex;
	u32 sampleNumber;
	u16 dataLength;
	u32 byteOffset;
	u16 bytesPerComp;
	u16 samplesPerComp;
} SampleDTE;

typedef struct tagStreamDescDTE
{
	BASE_DTE_ENTRY
	s8 trackRefIndex;
	u32 streamDescIndex;
	u16 dataLength;
	u32 byteOffset;
	u32 reserved;
} StreamDescDTE;

GenericDTE *NewDTE(u8 type);
void DelDTE(GenericDTE *dte);
M4Err ReadDTE(GenericDTE *dte, BitStream *bs);
M4Err WriteDTE(GenericDTE *dte, BitStream *bs);
M4Err OffsetDTE(GenericDTE *dte, u32 offset, u32 HintSampleNumber);

/*****************************************************
		RTP Sample
*****************************************************/

typedef struct _tagHintSample
{
	/*used internally for future protocol support (write only)*/
	u8 HintType;
	/*used internally for hinting (write only)*/
	u32 TransmissionTime;
	/*QT packets*/
	u16 reserved;
	Chain *packetTable;
	char *AdditionalData;
	u32 dataLength;
} HintSample;

HintSample *New_HintSample();
void Del_HintSample(HintSample *ptr);
M4Err Read_HintSample(HintSample *ptr, BitStream *bs, u32 sampleSize);
u32 Write_HintSample(HintSample *ptr, BitStream *bs);
u32 Size_HintSample(HintSample *ptr);

/*****************************************************
		Hint Packets (generic packet for future protocol support)
*****************************************************/
#define BASE_PACKET			\
	s32 relativeTransTime;


typedef struct _tagHintPacket
{
	BASE_PACKET
} HintPacket;

HintPacket *New_HintPacket(u8 HintType);
void Del_HintPacket(u8 HintType, HintPacket *ptr);
M4Err Read_HintPacket(u8 HintType, HintPacket *ptr, BitStream *bs);
M4Err Write_HintPacket(u8 HintType, HintPacket *ptr, BitStream *bs);
u32 Size_HintPacket(u8 HintType, HintPacket *ptr);
M4Err Offset_HintPacket(u8 HintType, HintPacket *ptr, u32 offset, u32 HintSampleNumber);
M4Err AddDTE_HintPacket(u8 HintType, HintPacket *ptr, GenericDTE *dte, u8 AtBegin);
/*get the size of the packet AS RECONSTRUCTED BY THE SERVER (without SSRC AND CSRC)*/
u32 Length_HintPacket(u8 HintType, HintPacket *ptr);

/*the RTP packet*/
typedef struct _tagRTPPacket
{
	BASE_PACKET	
	
	/*RTP Header*/
	u8 P_bit;
	u8 X_bit;
	u8 M_bit;
	/*on 7 bits */
	u8 payloadType;
	u16 SequenceNumber;
	/*Hinting flags*/
	u8 B_bit;
	u8 R_bit;
	/*ExtraInfos TLVs - not really used */
	Chain *TLV;
	/*DataTable - contains the DTEs...*/
	Chain *DataTable;
} RTPPacket;

RTPPacket *New_RTPPacket();
void Del_RTPPacket(RTPPacket *ptr);
M4Err Read_RTPPacket(RTPPacket *ptr, BitStream *bs);
M4Err Write_RTPPacket(RTPPacket *ptr, BitStream *bs);
u32 Size_RTPPacket(RTPPacket *ptr);
M4Err Offset_RTPPacket(RTPPacket *ptr, u32 offset, u32 HintSampleNumber);
u32 Length_RTPPacket(RTPPacket *ptr);



typedef struct _3gp_text_sample
{
	char *text;
	u32 len;
	
	TextStyleAtom *styles;
	/*at most one of these*/
	TextHighlightColorAtom *highlight_color;
	TextScrollDelayAtom *scroll_delay;
	TextBoxAtom *box;
	TextWrapAtom *wrap;

	Chain *others;
	TextKaraokeAtom *cur_karaoke;
} TextSample;

TextSample *M4_ParseTextSample(BitStream *bs);
TextSample *M4_ParseTextSampleFromData(char *data, u32 dataLength);


/*
	these are exported just in case, there should never be needed outside the lib
*/

Atom *reftype_New();
Atom *free_New();
Atom *mdat_New();
Atom *moov_New();
Atom *mvhd_New();
Atom *mdhd_New();
Atom *vmhd_New();
Atom *smhd_New();
Atom *hmhd_New();
Atom *nmhd_New();
Atom *stbl_New();
Atom *dinf_New();
Atom *url_New();
Atom *urn_New();
Atom *cprt_New();
Atom *hdlr_New();
Atom *iods_New();
Atom *trak_New();
Atom *mp4s_New();
Atom *mp4v_New();
Atom *mp4a_New();
Atom *edts_New();
Atom *udta_New();
Atom *dref_New();
Atom *stsd_New();
Atom *stts_New();
Atom *ctts_New();
Atom *stsh_New();
Atom *elst_New();
Atom *stsc_New();
Atom *stsz_New();
Atom *stco_New();
Atom *stss_New();
Atom *stdp_New();
Atom *co64_New();
Atom *esds_New();
Atom *minf_New();
Atom *tkhd_New();
Atom *tref_New();
Atom *mdia_New();
Atom *defa_New();
Atom *void_New();
Atom *stsf_New();
Atom *gnrm_New();
Atom *gnrv_New();
Atom *gnra_New();

void reftype_del(Atom *);
void free_del(Atom *);
void mdat_del(Atom *);
void moov_del(Atom *);
void mvhd_del(Atom *);
void mdhd_del(Atom *);
void vmhd_del(Atom *);
void smhd_del(Atom *);
void hmhd_del(Atom *);
void nmhd_del(Atom *);
void stbl_del(Atom *);
void dinf_del(Atom *);
void url_del(Atom *);
void urn_del(Atom *);
void cprt_del(Atom *);
void hdlr_del(Atom *);
void iods_del(Atom *);
void trak_del(Atom *);
void mp4s_del(Atom *);
void mp4v_del(Atom *);
void mp4a_del(Atom *);
void edts_del(Atom *);
void udta_del(Atom *);
void dref_del(Atom *);
void stsd_del(Atom *);
void stts_del(Atom *);
void ctts_del(Atom *);
void stsh_del(Atom *);
void elst_del(Atom *);
void stsc_del(Atom *);
void stsz_del(Atom *);
void stco_del(Atom *);
void stss_del(Atom *);
void stdp_del(Atom *);
void co64_del(Atom *);
void esds_del(Atom *);
void minf_del(Atom *);
void tkhd_del(Atom *);
void tref_del(Atom *);
void mdia_del(Atom *);
void defa_del(Atom *);
void void_del(Atom *);
void stsf_del(Atom *);
void gnrm_del(Atom *);
void gnrv_del(Atom *);
void gnra_del(Atom *);

M4Err reftype_Write(Atom *s, BitStream *bs);
M4Err free_Write(Atom *s, BitStream *bs);
M4Err mdat_Write(Atom *s, BitStream *bs);
M4Err moov_Write(Atom *s, BitStream *bs);
M4Err mvhd_Write(Atom *s, BitStream *bs);
M4Err mdhd_Write(Atom *s, BitStream *bs);
M4Err vmhd_Write(Atom *s, BitStream *bs);
M4Err smhd_Write(Atom *s, BitStream *bs);
M4Err hmhd_Write(Atom *s, BitStream *bs);
M4Err nmhd_Write(Atom *s, BitStream *bs);
M4Err stbl_Write(Atom *s, BitStream *bs);
M4Err dinf_Write(Atom *s, BitStream *bs);
M4Err url_Write(Atom *s, BitStream *bs);
M4Err urn_Write(Atom *s, BitStream *bs);
M4Err cprt_Write(Atom *s, BitStream *bs);
M4Err hdlr_Write(Atom *s, BitStream *bs);
M4Err iods_Write(Atom *s, BitStream *bs);
M4Err trak_Write(Atom *s, BitStream *bs);
M4Err mp4s_Write(Atom *s, BitStream *bs);
M4Err mp4v_Write(Atom *s, BitStream *bs);
M4Err mp4a_Write(Atom *s, BitStream *bs);
M4Err edts_Write(Atom *s, BitStream *bs);
M4Err udta_Write(Atom *s, BitStream *bs);
M4Err dref_Write(Atom *s, BitStream *bs);
M4Err stsd_Write(Atom *s, BitStream *bs);
M4Err stts_Write(Atom *s, BitStream *bs);
M4Err ctts_Write(Atom *s, BitStream *bs);
M4Err stsh_Write(Atom *s, BitStream *bs);
M4Err elst_Write(Atom *s, BitStream *bs);
M4Err stsc_Write(Atom *s, BitStream *bs);
M4Err stsz_Write(Atom *s, BitStream *bs);
M4Err stco_Write(Atom *s, BitStream *bs);
M4Err stss_Write(Atom *s, BitStream *bs);
M4Err stdp_Write(Atom *s, BitStream *bs);
M4Err co64_Write(Atom *s, BitStream *bs);
M4Err esds_Write(Atom *s, BitStream *bs);
M4Err minf_Write(Atom *s, BitStream *bs);
M4Err tkhd_Write(Atom *s, BitStream *bs);
M4Err tref_Write(Atom *s, BitStream *bs);
M4Err mdia_Write(Atom *s, BitStream *bs);
M4Err defa_Write(Atom *s, BitStream *bs);
M4Err void_Write(Atom *s, BitStream *bs);
M4Err stsf_Write(Atom *s, BitStream *bs);
M4Err gnrm_Write(Atom *s, BitStream *bs);
M4Err gnrv_Write(Atom *s, BitStream *bs);
M4Err gnra_Write(Atom *s, BitStream *bs);

M4Err reftype_Size(Atom *);
M4Err free_Size(Atom *);
M4Err mdat_Size(Atom *);
M4Err moov_Size(Atom *);
M4Err mvhd_Size(Atom *);
M4Err mdhd_Size(Atom *);
M4Err vmhd_Size(Atom *);
M4Err smhd_Size(Atom *);
M4Err hmhd_Size(Atom *);
M4Err nmhd_Size(Atom *);
M4Err stbl_Size(Atom *);
M4Err dinf_Size(Atom *);
M4Err url_Size(Atom *);
M4Err urn_Size(Atom *);
M4Err cprt_Size(Atom *);
M4Err hdlr_Size(Atom *);
M4Err iods_Size(Atom *);
M4Err trak_Size(Atom *);
M4Err mp4s_Size(Atom *);
M4Err mp4v_Size(Atom *);
M4Err mp4a_Size(Atom *);
M4Err edts_Size(Atom *);
M4Err udta_Size(Atom *);
M4Err dref_Size(Atom *);
M4Err stsd_Size(Atom *);
M4Err stts_Size(Atom *);
M4Err ctts_Size(Atom *);
M4Err stsh_Size(Atom *);
M4Err elst_Size(Atom *);
M4Err stsc_Size(Atom *);
M4Err stsz_Size(Atom *);
M4Err stco_Size(Atom *);
M4Err stss_Size(Atom *);
M4Err stdp_Size(Atom *);
M4Err co64_Size(Atom *);
M4Err esds_Size(Atom *);
M4Err minf_Size(Atom *);
M4Err tkhd_Size(Atom *);
M4Err tref_Size(Atom *);
M4Err mdia_Size(Atom *);
M4Err defa_Size(Atom *);
M4Err void_Size(Atom *);
M4Err stsf_Size(Atom *);
M4Err gnrm_Size(Atom *);
M4Err gnrv_Size(Atom *);
M4Err gnra_Size(Atom *);

M4Err reftype_Read(Atom *s, BitStream *bs, u64 *read);
M4Err free_Read(Atom *s, BitStream *bs, u64 *read);
M4Err mdat_Read(Atom *s, BitStream *bs, u64 *read);
M4Err moov_Read(Atom *s, BitStream *bs, u64 *read);
M4Err mvhd_Read(Atom *s, BitStream *bs, u64 *read);
M4Err mdhd_Read(Atom *s, BitStream *bs, u64 *read);
M4Err vmhd_Read(Atom *s, BitStream *bs, u64 *read);
M4Err smhd_Read(Atom *s, BitStream *bs, u64 *read);
M4Err hmhd_Read(Atom *s, BitStream *bs, u64 *read);
M4Err nmhd_Read(Atom *s, BitStream *bs, u64 *read);
M4Err stbl_Read(Atom *s, BitStream *bs, u64 *read);
M4Err dinf_Read(Atom *s, BitStream *bs, u64 *read);
M4Err url_Read(Atom *s, BitStream *bs, u64 *read);
M4Err urn_Read(Atom *s, BitStream *bs, u64 *read);
M4Err cprt_Read(Atom *s, BitStream *bs, u64 *read);
M4Err hdlr_Read(Atom *s, BitStream *bs, u64 *read);
M4Err iods_Read(Atom *s, BitStream *bs, u64 *read);
M4Err trak_Read(Atom *s, BitStream *bs, u64 *read);
M4Err mp4s_Read(Atom *s, BitStream *bs, u64 *read);
M4Err mp4v_Read(Atom *s, BitStream *bs, u64 *read);
M4Err mp4a_Read(Atom *s, BitStream *bs, u64 *read);
M4Err edts_Read(Atom *s, BitStream *bs, u64 *read);
M4Err udta_Read(Atom *s, BitStream *bs, u64 *read);
M4Err dref_Read(Atom *s, BitStream *bs, u64 *read);
M4Err stsd_Read(Atom *s, BitStream *bs, u64 *read);
M4Err stts_Read(Atom *s, BitStream *bs, u64 *read);
M4Err ctts_Read(Atom *s, BitStream *bs, u64 *read);
M4Err stsh_Read(Atom *s, BitStream *bs, u64 *read);
M4Err elst_Read(Atom *s, BitStream *bs, u64 *read);
M4Err stsc_Read(Atom *s, BitStream *bs, u64 *read);
M4Err stsz_Read(Atom *s, BitStream *bs, u64 *read);
M4Err stco_Read(Atom *s, BitStream *bs, u64 *read);
M4Err stss_Read(Atom *s, BitStream *bs, u64 *read);
M4Err stdp_Read(Atom *s, BitStream *bs, u64 *read);
M4Err co64_Read(Atom *s, BitStream *bs, u64 *read);
M4Err esds_Read(Atom *s, BitStream *bs, u64 *read);
M4Err minf_Read(Atom *s, BitStream *bs, u64 *read);
M4Err tkhd_Read(Atom *s, BitStream *bs, u64 *read);
M4Err tref_Read(Atom *s, BitStream *bs, u64 *read);
M4Err mdia_Read(Atom *s, BitStream *bs, u64 *read);
M4Err defa_Read(Atom *s, BitStream *bs, u64 *read);
M4Err void_Read(Atom *s, BitStream *bs, u64 *read);
M4Err stsf_Read(Atom *s, BitStream *bs, u64 *read);


Atom *hinf_New();
Atom *trpy_New();
Atom *totl_New();
Atom *nump_New();
Atom *npck_New();
Atom *tpyl_New();
Atom *tpay_New();
Atom *maxr_New();
Atom *dmed_New();
Atom *dimm_New();
Atom *drep_New();
Atom *tmin_New();
Atom *tmax_New();
Atom *pmax_New();
Atom *dmax_New();
Atom *payt_New();
Atom *name_New();
Atom *rely_New();
Atom *snro_New();
Atom *tims_New();
Atom *tsro_New();
Atom *ghnt_New();
Atom *hnti_New();
Atom *sdp_New();
Atom *rtpo_New();

void hinf_del(Atom *s);
void trpy_del(Atom *s);
void totl_del(Atom *s);
void nump_del(Atom *s);
void npck_del(Atom *s);
void tpyl_del(Atom *s);
void tpay_del(Atom *s);
void maxr_del(Atom *s);
void dmed_del(Atom *s);
void dimm_del(Atom *s);
void drep_del(Atom *s);
void tmin_del(Atom *s);
void tmax_del(Atom *s);
void pmax_del(Atom *s);
void dmax_del(Atom *s);
void payt_del(Atom *s);
void name_del(Atom *s);
void rely_del(Atom *s);
void snro_del(Atom *s);
void tims_del(Atom *s);
void tsro_del(Atom *s);
void ghnt_del(Atom *s);
void hnti_del(Atom *a);
void sdp_del(Atom *a);
void rtpo_del(Atom *s);

M4Err hinf_Read(Atom *s, BitStream *bs, u64 *read);
M4Err trpy_Read(Atom *s, BitStream *bs, u64 *read);
M4Err totl_Read(Atom *s, BitStream *bs, u64 *read);
M4Err nump_Read(Atom *s, BitStream *bs, u64 *read);
M4Err npck_Read(Atom *s, BitStream *bs, u64 *read);
M4Err tpyl_Read(Atom *s, BitStream *bs, u64 *read);
M4Err tpay_Read(Atom *s, BitStream *bs, u64 *read);
M4Err maxr_Read(Atom *s, BitStream *bs, u64 *read);
M4Err dmed_Read(Atom *s, BitStream *bs, u64 *read);
M4Err dimm_Read(Atom *s, BitStream *bs, u64 *read);
M4Err drep_Read(Atom *s, BitStream *bs, u64 *read);
M4Err tmin_Read(Atom *s, BitStream *bs, u64 *read);
M4Err tmax_Read(Atom *s, BitStream *bs, u64 *read);
M4Err pmax_Read(Atom *s, BitStream *bs, u64 *read);
M4Err dmax_Read(Atom *s, BitStream *bs, u64 *read);
M4Err payt_Read(Atom *s, BitStream *bs, u64 *read);
M4Err name_Read(Atom *s, BitStream *bs, u64 *read);
M4Err rely_Read(Atom *s, BitStream *bs, u64 *read);
M4Err snro_Read(Atom *s, BitStream *bs, u64 *read);
M4Err tims_Read(Atom *s, BitStream *bs, u64 *read);
M4Err tsro_Read(Atom *s, BitStream *bs, u64 *read);
M4Err ghnt_Read(Atom *s, BitStream *bs, u64 *read);
M4Err hnti_Read(Atom *s, BitStream *bs, u64 *read);
M4Err sdp_Read(Atom *s, BitStream *bs, u64 *read);
M4Err rtpo_Read(Atom *s, BitStream *bs, u64 *read);

M4Err hinf_Write(Atom *s, BitStream *bs);
M4Err trpy_Write(Atom *s, BitStream *bs);
M4Err totl_Write(Atom *s, BitStream *bs);
M4Err nump_Write(Atom *s, BitStream *bs);
M4Err npck_Write(Atom *s, BitStream *bs);
M4Err tpyl_Write(Atom *s, BitStream *bs);
M4Err tpay_Write(Atom *s, BitStream *bs);
M4Err maxr_Write(Atom *s, BitStream *bs);
M4Err dmed_Write(Atom *s, BitStream *bs);
M4Err dimm_Write(Atom *s, BitStream *bs);
M4Err drep_Write(Atom *s, BitStream *bs);
M4Err tmin_Write(Atom *s, BitStream *bs);
M4Err tmax_Write(Atom *s, BitStream *bs);
M4Err pmax_Write(Atom *s, BitStream *bs);
M4Err dmax_Write(Atom *s, BitStream *bs);
M4Err payt_Write(Atom *s, BitStream *bs);
M4Err name_Write(Atom *s, BitStream *bs);
M4Err rely_Write(Atom *s, BitStream *bs);
M4Err snro_Write(Atom *s, BitStream *bs);
M4Err tims_Write(Atom *s, BitStream *bs);
M4Err tsro_Write(Atom *s, BitStream *bs);
M4Err ghnt_Write(Atom *s, BitStream *bs);
M4Err hnti_Write(Atom *s, BitStream *bs);
M4Err sdp_Write(Atom *s, BitStream *bs);
M4Err rtpo_Write(Atom *s, BitStream *bs);

M4Err hinf_Size(Atom *s);
M4Err trpy_Size(Atom *s);
M4Err totl_Size(Atom *s);
M4Err nump_Size(Atom *s);
M4Err npck_Size(Atom *s);
M4Err tpyl_Size(Atom *s);
M4Err tpay_Size(Atom *s);
M4Err maxr_Size(Atom *s);
M4Err dmed_Size(Atom *s);
M4Err dimm_Size(Atom *s);
M4Err drep_Size(Atom *s);
M4Err tmin_Size(Atom *s);
M4Err tmax_Size(Atom *s);
M4Err pmax_Size(Atom *s);
M4Err dmax_Size(Atom *s);
M4Err payt_Size(Atom *s);
M4Err name_Size(Atom *s);
M4Err rely_Size(Atom *s);
M4Err snro_Size(Atom *s);
M4Err tims_Size(Atom *s);
M4Err tsro_Size(Atom *s);
M4Err ghnt_Size(Atom *s);
M4Err hnti_Size(Atom *s);
M4Err sdp_Size(Atom *s);
M4Err rtpo_Size(Atom *s);


Atom *ftyp_New();
void ftyp_del(Atom *s);
M4Err ftyp_Read(Atom *s,BitStream *bs, u64 *read);
M4Err ftyp_Write(Atom *s, BitStream *bs);
M4Err ftyp_Size(Atom *s);

Atom *padb_New();
void padb_del(Atom *s);
M4Err padb_Read(Atom *s, BitStream *bs, u64 *read);
M4Err padb_Write(Atom *s, BitStream *bs);
M4Err padb_Size(Atom *s);

Atom *amr3_New();
Atom *damr_New();
Atom *h263_New();
Atom *d263_New();

void amr3_del(Atom *s);
void damr_del(Atom *s);
void h263_del(Atom *s);
void d263_del(Atom *s);

M4Err amr3_Read(Atom *s, BitStream *bs, u64 *read);
M4Err damr_Read(Atom *s, BitStream *bs, u64 *read);
M4Err h263_Read(Atom *s, BitStream *bs, u64 *read);
M4Err d263_Read(Atom *s, BitStream *bs, u64 *read);

#ifndef M4_READ_ONLY
M4Err amr3_Write(Atom *s, BitStream *bs);
M4Err damr_Write(Atom *s, BitStream *bs);
M4Err h263_Write(Atom *s, BitStream *bs);
M4Err d263_Write(Atom *s, BitStream *bs);

M4Err amr3_Size(Atom *s);
M4Err damr_Size(Atom *s);
M4Err h263_Size(Atom *s);
M4Err d263_Size(Atom *s);
#endif


#ifndef	M4_ISO_NO_FRAGMENTS
Atom *mvex_New();
Atom *trex_New();
Atom *moof_New();
Atom *mfhd_New();
Atom *traf_New();
Atom *tfhd_New();
Atom *trun_New();

void mvex_del(Atom *s);
void trex_del(Atom *s);
void moof_del(Atom *s);
void mfhd_del(Atom *s);
void traf_del(Atom *s);
void tfhd_del(Atom *s);
void trun_del(Atom *s);

M4Err mvex_Read(Atom *s, BitStream *bs, u64 *read);
M4Err trex_Read(Atom *s, BitStream *bs, u64 *read);
M4Err moof_Read(Atom *s, BitStream *bs, u64 *read);
M4Err mfhd_Read(Atom *s, BitStream *bs, u64 *read);
M4Err traf_Read(Atom *s, BitStream *bs, u64 *read);
M4Err tfhd_Read(Atom *s, BitStream *bs, u64 *read);
M4Err trun_Read(Atom *s, BitStream *bs, u64 *read);

M4Err mvex_Write(Atom *s, BitStream *bs);
M4Err trex_Write(Atom *s, BitStream *bs);
M4Err moof_Write(Atom *s, BitStream *bs);
M4Err mfhd_Write(Atom *s, BitStream *bs);
M4Err traf_Write(Atom *s, BitStream *bs);
M4Err tfhd_Write(Atom *s, BitStream *bs);
M4Err trun_Write(Atom *s, BitStream *bs);

M4Err mvex_Size(Atom *s);
M4Err trex_Size(Atom *s);
M4Err moof_Size(Atom *s);
M4Err mfhd_Size(Atom *s);
M4Err traf_Size(Atom *s);
M4Err tfhd_Size(Atom *s);
M4Err trun_Size(Atom *s);

#endif 

/*avc ext*/
Atom *avcc_New();
void avcc_del(Atom *s);
M4Err avcc_Read(Atom *s, BitStream *bs, u64 *read);
M4Err avcc_Write(Atom *s, BitStream *bs);
M4Err avcc_Size(Atom *s);

Atom *avc1_New();
void avc1_del(Atom *s);
M4Err avc1_Read(Atom *s, BitStream *bs, u64 *read);
M4Err avc1_Write(Atom *s, BitStream *bs);
M4Err avc1_Size(Atom *s);

Atom *m4ds_New();
void m4ds_del(Atom *s);
M4Err m4ds_Read(Atom *s, BitStream *bs, u64 *read);
M4Err m4ds_Write(Atom *s, BitStream *bs);
M4Err m4ds_Size(Atom *s);

Atom *btrt_New();
void btrt_del(Atom *s);
M4Err btrt_Read(Atom *s, BitStream *bs, u64 *read);
M4Err btrt_Write(Atom *s, BitStream *bs);
M4Err btrt_Size(Atom *s);


/*3GPP streaming text*/
Atom *ftab_New();
Atom *tx3g_New();
Atom *styl_New();
Atom *hlit_New();
Atom *hclr_New();
Atom *krok_New();
Atom *dlay_New();
Atom *href_New();
Atom *tbox_New();
Atom *blnk_New();
Atom *twrp_New();

void ftab_del(Atom *s);
void tx3g_del(Atom *s);
void styl_del(Atom *s);
void hlit_del(Atom *s);
void hclr_del(Atom *s);
void krok_del(Atom *s);
void dlay_del(Atom *s);
void href_del(Atom *s);
void tbox_del(Atom *s);
void blnk_del(Atom *s);
void twrp_del(Atom *s);

M4Err ftab_Read(Atom *s, BitStream *bs, u64 *read);
M4Err tx3g_Read(Atom *s, BitStream *bs, u64 *read);
M4Err styl_Read(Atom *s, BitStream *bs, u64 *read);
M4Err hlit_Read(Atom *s, BitStream *bs, u64 *read);
M4Err hclr_Read(Atom *s, BitStream *bs, u64 *read);
M4Err krok_Read(Atom *s, BitStream *bs, u64 *read);
M4Err dlay_Read(Atom *s, BitStream *bs, u64 *read);
M4Err href_Read(Atom *s, BitStream *bs, u64 *read);
M4Err tbox_Read(Atom *s, BitStream *bs, u64 *read);
M4Err blnk_Read(Atom *s, BitStream *bs, u64 *read);
M4Err twrp_Read(Atom *s, BitStream *bs, u64 *read);

#ifndef M4_READ_ONLY
M4Err ftab_Write(Atom *s, BitStream *bs);
M4Err tx3g_Write(Atom *s, BitStream *bs);
M4Err styl_Write(Atom *s, BitStream *bs);
M4Err hlit_Write(Atom *s, BitStream *bs);
M4Err hclr_Write(Atom *s, BitStream *bs);
M4Err krok_Write(Atom *s, BitStream *bs);
M4Err dlay_Write(Atom *s, BitStream *bs);
M4Err href_Write(Atom *s, BitStream *bs);
M4Err tbox_Write(Atom *s, BitStream *bs);
M4Err blnk_Write(Atom *s, BitStream *bs);
M4Err twrp_Write(Atom *s, BitStream *bs);

M4Err ftab_Size(Atom *s);
M4Err tx3g_Size(Atom *s);
M4Err styl_Size(Atom *s);
M4Err hlit_Size(Atom *s);
M4Err hclr_Size(Atom *s);
M4Err krok_Size(Atom *s);
M4Err dlay_Size(Atom *s);
M4Err href_Size(Atom *s);
M4Err tbox_Size(Atom *s);
M4Err blnk_Size(Atom *s);
M4Err twrp_Size(Atom *s);
#endif



M4Err AtomListDump(Chain *list, FILE * trace);
M4Err reftype_dump(Atom *a, FILE * trace);
M4Err free_dump(Atom *a, FILE * trace);
M4Err mdat_dump(Atom *a, FILE * trace);
M4Err moov_dump(Atom *a, FILE * trace);
M4Err mvhd_dump(Atom *a, FILE * trace);
M4Err mdhd_dump(Atom *a, FILE * trace);
M4Err vmhd_dump(Atom *a, FILE * trace);
M4Err smhd_dump(Atom *a, FILE * trace);
M4Err hmhd_dump(Atom *a, FILE * trace);
M4Err nmhd_dump(Atom *a, FILE * trace);
M4Err stbl_dump(Atom *a, FILE * trace);
M4Err dinf_dump(Atom *a, FILE * trace);
M4Err url_dump(Atom *a, FILE * trace);
M4Err urn_dump(Atom *a, FILE * trace);
M4Err cprt_dump(Atom *a, FILE * trace);
M4Err hdlr_dump(Atom *a, FILE * trace);
M4Err iods_dump(Atom *a, FILE * trace);
M4Err trak_dump(Atom *a, FILE * trace);
M4Err mp4s_dump(Atom *a, FILE * trace);
M4Err mp4v_dump(Atom *a, FILE * trace);
M4Err mp4a_dump(Atom *a, FILE * trace);
M4Err edts_dump(Atom *a, FILE * trace);
M4Err udta_dump(Atom *a, FILE * trace);
M4Err dref_dump(Atom *a, FILE * trace);
M4Err stsd_dump(Atom *a, FILE * trace);
M4Err stts_dump(Atom *a, FILE * trace);
M4Err ctts_dump(Atom *a, FILE * trace);
M4Err stsh_dump(Atom *a, FILE * trace);
M4Err elst_dump(Atom *a, FILE * trace);
M4Err stsc_dump(Atom *a, FILE * trace);
M4Err stsz_dump(Atom *a, FILE * trace);
M4Err stco_dump(Atom *a, FILE * trace);
M4Err stss_dump(Atom *a, FILE * trace);
M4Err stdp_dump(Atom *a, FILE * trace);
M4Err co64_dump(Atom *a, FILE * trace);
M4Err esds_dump(Atom *a, FILE * trace);
M4Err minf_dump(Atom *a, FILE * trace);
M4Err tkhd_dump(Atom *a, FILE * trace);
M4Err tref_dump(Atom *a, FILE * trace);
M4Err mdia_dump(Atom *a, FILE * trace);
M4Err defa_dump(Atom *a, FILE * trace);
M4Err void_dump(Atom *a, FILE * trace);
M4Err ftyp_dump(Atom *a, FILE * trace);
M4Err padb_dump(Atom *a, FILE * trace);
M4Err stsf_dump(Atom *a, FILE * trace);
M4Err gnrm_dump(Atom *a, FILE * trace);
M4Err gnrv_dump(Atom *a, FILE * trace);
M4Err gnra_dump(Atom *a, FILE * trace);


//3GPP atoms
M4Err amr3_dump(Atom *a, FILE * trace);
M4Err damr_dump(Atom *a, FILE * trace);
M4Err h263_dump(Atom *a, FILE * trace);
M4Err d263_dump(Atom *a, FILE * trace);


M4Err hinf_dump(Atom *a, FILE * trace);
M4Err trpy_dump(Atom *a, FILE * trace);
M4Err totl_dump(Atom *a, FILE * trace);
M4Err nump_dump(Atom *a, FILE * trace);
M4Err npck_dump(Atom *a, FILE * trace);
M4Err tpyl_dump(Atom *a, FILE * trace);
M4Err tpay_dump(Atom *a, FILE * trace);
M4Err maxr_dump(Atom *a, FILE * trace);
M4Err dmed_dump(Atom *a, FILE * trace);
M4Err dimm_dump(Atom *a, FILE * trace);
M4Err drep_dump(Atom *a, FILE * trace);
M4Err tmin_dump(Atom *a, FILE * trace);
M4Err tmax_dump(Atom *a, FILE * trace);
M4Err pmax_dump(Atom *a, FILE * trace);
M4Err dmax_dump(Atom *a, FILE * trace);
M4Err payt_dump(Atom *a, FILE * trace);
M4Err name_dump(Atom *a, FILE * trace);
M4Err rely_dump(Atom *a, FILE * trace);
M4Err snro_dump(Atom *a, FILE * trace);
M4Err tims_dump(Atom *a, FILE * trace);
M4Err tsro_dump(Atom *a, FILE * trace);
M4Err ghnt_dump(Atom *a, FILE * trace);
M4Err hnti_dump(Atom *a, FILE * trace);
M4Err sdp_dump(Atom *a, FILE * trace);
M4Err rtpo_dump(Atom *a, FILE * trace);



#ifndef	M4_ISO_NO_FRAGMENTS
M4Err mvex_dump(Atom *a, FILE * trace);
M4Err trex_dump(Atom *a, FILE * trace);
M4Err moof_dump(Atom *a, FILE * trace);
M4Err mfhd_dump(Atom *a, FILE * trace);
M4Err traf_dump(Atom *a, FILE * trace);
M4Err tfhd_dump(Atom *a, FILE * trace);
M4Err trun_dump(Atom *a, FILE * trace);
#endif

M4Err avcc_dump(Atom *a, FILE * trace);
M4Err avc1_dump(Atom *a, FILE * trace);
M4Err m4ds_dump(Atom *a, FILE * trace);
M4Err btrt_dump(Atom *a, FILE * trace);

M4Err ftab_dump(Atom *a, FILE * trace);
M4Err tx3g_dump(Atom *a, FILE * trace);
M4Err styl_dump(Atom *a, FILE * trace);
M4Err hlit_dump(Atom *a, FILE * trace);
M4Err hclr_dump(Atom *a, FILE * trace);
M4Err krok_dump(Atom *a, FILE * trace);
M4Err dlay_dump(Atom *a, FILE * trace);
M4Err href_dump(Atom *a, FILE * trace);
M4Err tbox_dump(Atom *a, FILE * trace);
M4Err blnk_dump(Atom *a, FILE * trace);
M4Err twrp_dump(Atom *a, FILE * trace);


#ifdef __cplusplus
}
#endif

#endif //__M4_ISOMEDIA_DEV_H

