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



#ifndef __M4_EXPORTS_H
#define __M4_EXPORTS_H


#ifdef __cplusplus
extern "C" {
#endif


#include <m4_descriptors.h>

/********************************************************************

					MPEG-4 AU Functions

			NOTE: an AU in File Format terminology is a sample
			this API provides simple way to get media data (payload) and
			meta-data (timing info) through this structure
********************************************************************/

/*the mp4 file*/
typedef struct tagM4File M4File;

/*the sample object*/
typedef struct tagM4Sample
{
	/*data size*/
	u32 dataLength;
	/*data with padding if requested*/
	char *data;
	/*decoding time*/
	u32 DTS;
	/*relative offset for composition if needed*/
	u32 CTS_Offset;
	/*Random Access Point flag - 1 is regular RAP (read/write) , 2 is SyncShadow (read mode only)*/
	u8 IsRAP;
} M4Sample;


/*creates a new empty sample*/
M4Sample *M4_NewSample();

/*creates a new formatted sample. The buffer is attached to the sample(no memory allocation)*/
M4Sample *M4_NewFullSample(char *data, u32 dataLength, u32 DTS,	u32 CTS_Offset, u8 IsRAP);

/*delete a sample. NOTE:the buffer content will be destroyed by default.
if you wish to keep the buffer, set dataLength to 0 in the sample 
before deleting it
the pointer is set to NULL after deletion*/
void M4_DeleteSample(M4Sample **samp);



/********************************************************************
				FILE FORMAT CONSTANTS
********************************************************************/

/*Modes for file opening
		NOTE 1: All the READ function in this API can be used in EDIT/WRITE mode. 
However, some unexpected errors or values may happen in that case, depending
on how much modifications you made (timing, track with 0 samples, ...)
		On the other hand, none of the EDIT/WRITE functions will work in 
READ mode.
		NOTE 2: The output structure of a edited file will sometimes be different 
from the original file, but the media-data and meta-data will be identical.
The only change happens in the file media-data container(s) during edition
		NOTE 3: when editing the file, you MUST set the final name of the modified file
to something different. This API doesn't allow file overwriting.
*/
enum 
{
	/*Opens file for dumping: same as read-only but keeps all movie fragments info untouched*/
	M4_OPEN_READ_DUMP = 0,
	/*Opens a file in READ ONLY mode*/
	M4_OPEN_READ	=	1,
	/*Opens a file in WRITE ONLY mode. Media Data is captured on the fly. In this mode, 
	the editing functions are disabled.*/
	M4_OPEN_WRITE	=	2,
	/*Opens an existing file in EDIT mode*/
	M4_OPEN_EDIT	=	3,
	/*Creates a new file in EDIT mode*/
	M4_WRITE_EDIT	=	4
};

/*Movie Options for file writing*/
enum
{
	/*FLAT: the MediaData (MPEG4 ESs) is stored at the begining of the file*/
	M4_FLAT					= 0x01,
	/*STREAMABLE: the MetaData (MP4 File Info) is stored at the begining of the file 
	for fast access during download*/
	M4_STREAMABLE			= 0x02,
	/*INTERLEAVED: Same as STREAMABLE, plus the media data is mixed by chunk  of fixed duration*/
	M4_INTERLEAVED			= 0x03,
	/*Interleaves samples based on their DTS, therefore allowing better placement of samples in the file.
	This is used for both http interleaving and Hinting optimizations*/
	M4_FULL_INTERLEAVED = 0x04

};

/*Some track may depend on other tracks for several reasons. They reference these tracks 
through the following Reference Types*/
enum
{
	/*ref type for the OD track dependencies*/
	M4_ODTrack_Ref			= FOUR_CHAR_INT( 'm', 'p', 'o', 'd' ),
	/*ref type for stream dependencies*/
	M4_StreamDependence_Ref = FOUR_CHAR_INT( 'd', 'p', 'n', 'd' ),
	/*ref type for OCR (Object Clock Resolution) dependencies*/
	M4_OCR_Ref				= FOUR_CHAR_INT( 's', 'y', 'n', 'c' ),
	/*ref type for IPI (Intellectual Property Information) dependencies*/
	M4_IPI_Ref				= FOUR_CHAR_INT( 'i', 'p', 'i', 'r' ),
	/*ref type for Hint tracks*/
	M4_HintTrack_Ref		= FOUR_CHAR_INT( 'h', 'i', 'n', 't' )
};

/*Track Edition flag*/
enum {
	/*empty segment in the track (no media for this segment)*/
	M4_EDIT_EMPTY		=	0x00,
	/*dwelled segment in the track (one media sample for this segment)*/
	M4_EDIT_DWELL		=	0x01,
	/*normal segment in the track*/
	M4_EDIT_NORMAL		=	0x02
};

/*Generic Media Types (YOU HAVE TO USE ONE OF THESE TYPES FOR COMPLIANT ISO MEDIA FILES)*/
enum
{
	M4_ODMediaType			= FOUR_CHAR_INT( 'o', 'd', 's', 'm' ),
	M4_OCRMediaType			= FOUR_CHAR_INT( 'c', 'r', 's', 'm' ),
	M4_BIFSMediaType		= FOUR_CHAR_INT( 's', 'd', 's', 'm' ),
	M4_VisualMediaType		= FOUR_CHAR_INT( 'v', 'i', 'd', 'e' ),
	M4_AudioMediaType		= FOUR_CHAR_INT( 's', 'o', 'u', 'n' ),
	M4_MPEG7MediaType		= FOUR_CHAR_INT( 'm', '7', 's', 'm' ),
	M4_OCIMediaType			= FOUR_CHAR_INT( 'o', 'c', 's', 'm' ),
	M4_IPMPMediaType		= FOUR_CHAR_INT( 'i', 'p', 's', 'm' ),
	M4_MPEGJMediaType		= FOUR_CHAR_INT( 'm', 'j', 's', 'm' ),
	M4_TimedTextMediaType	= FOUR_CHAR_INT( 't', 'e', 'x', 't' ),
	M4_HintMediaType		= FOUR_CHAR_INT( 'h', 'i', 'n', 't' ),
	M4_FlashMediaType		= FOUR_CHAR_INT( 'f', 'l', 's', 'h' ),
};

/*specific media sub-types - you shall make sure the media sub type is what you expect*/
enum
{
	/*reserved, internal use in the lib. Indicates the track complies to MPEG-4 system
	specification, and the usual OD framework tools may be used*/
	M4_MPEG4_SubType	= FOUR_CHAR_INT( 'M', 'P', 'E', 'G' ),
	
	/*other types are as indicated in the file format. Some types are handled by this lib, some are not*/

	/*AMR media complying to 3GPP specification*/
	M4_AMR_SubType		= FOUR_CHAR_INT( 's', 'a', 'm', 'r' ),
	M4_AMR_WB_SubType	= FOUR_CHAR_INT( 's', 'a', 'w', 'b' ),

	/*H263 media complying to 3GPP specification*/
	M4_H263_SubType		= FOUR_CHAR_INT( 's', '2', '6', '3' ),

	/*AVC/H264 media type, (ESD not available)*/
	M4_AVC_H264_SubType	= FOUR_CHAR_INT( 'a', 'v', 'c', '1' ),

	QT_SVQ3_SubType		= FOUR_CHAR_INT( 'S', 'V', 'Q', '3' )
};




/*direction for sample search (including SyncSamples search)
Function using search allways specify the desired time in composition (presentation) time

		(Sample N-1)	DesiredTime		(Sample N)

FORWARD: will give the next sample given the desired time (eg, N)
BACKWARD: will give the previous sample given the desired time (eg, N-1)
SYNCFORWARD: will search from the desired point in time for a sync sample if any
		If no sync info, behaves as FORWARD
SYNCBACKWARD: will search till the desired point in time for a sync sample if any
		If no sync info, behaves as BACKWARD
SYNCSHADOW: use the sync shadow information to retrieve the sample.
		If no SyncShadow info, behave as SYNCBACKWARD
*/
enum
{
	M4_SearchForward		=	1,
	M4_SearchBackward		=	2,
	M4_SearchSyncForward	=	3,
	M4_SearchSyncBackward	=	4,
	M4_SearchSyncShadow		=	5
};

/*Predefined File Brand codes (MPEG-4 and JPEG2000)*/
enum
{
	/*file complying to the generic ISO Media File (base specification ISO/IEC 14496-12)
	this is the default brand when creating a new movie*/
	ISO_Media_File = FOUR_CHAR_INT( 'i', 's', 'o', 'm' ),
	/*file complying to ISO/IEC 14496-1 2001 edition. A .mp4 file without a brand
	is equivalent to a file compatible with this brand*/
	MP4_V1_File = FOUR_CHAR_INT( 'm', 'p', '4', '1' ),
	/*file complying to ISO/IEC 14496-14 (MP4 spec)*/
	MP4_V2_File = FOUR_CHAR_INT( 'm', 'p', '4', '2' ),
	/*file complying to ISO/IEC 15444-3 (JPEG2000) without profile restriction*/
	MJ2K_Full_File = FOUR_CHAR_INT( 'm', 'j', 'p', '2' ),
	/*file complying to ISO/IEC 15444-3 (JPEG2000) with simple profile restriction*/
	MJ2K_Simple_File = FOUR_CHAR_INT( 'm', 'j', '2', 's' ),
	/*for old version of 3GPP spec, not handled by this lib*/
	GPP_V4_File = FOUR_CHAR_INT('3', 'g', 'p', '4'),
	/*this lib version of 3GPP file spec*/
	GPP_V5_File = FOUR_CHAR_INT('3', 'g', 'p', '5'),
	/*next version of 3GPP file spec*/
	GPP_V6_File = FOUR_CHAR_INT('3', 'g', 'p', '6'),
};


/*MPEG-4 ProfileAndLevel codes*/
enum
{
	M4_PL_AUDIO,
	M4_PL_VISUAL,
	M4_PL_GRAPHICS,
	M4_PL_SCENE,
	M4_PL_OD,
	M4_PL_MPEGJ,
	/*not a profile, just set/unset inlineFlag*/
	M4_PL_INLINE,
};

/********************************************************************
				GENERAL API FUNCTIONS
********************************************************************/

/*get the last fatal error that occured in the file
ANY FUNCTION OF THIS API WON'T BE PROCESSED IF THE FILE HAS AN ERROR
Note: some function may return an error while the movie has no error
the last error is a FatalError, and is not always set if a bad 
param is specified...*/
M4Err M4_GetLastError(M4File *the_file);

/*Open an MP4File.*/
M4File *M4_MovieOpen(const char *fileName, u8 OpenMode);

/*close the file, write it if new/edited*/
M4Err M4_MovieClose(M4File *the_file);

/*delete the movie without saving it.*/
void M4_MovieDelete(M4File *the_file);

/*Get the mode of an open file*/
u8 M4_GetOpenMode(M4File *the_file);

/********************************************************************
				STREAMING API FUNCTIONS
********************************************************************/
/*open a movie that can be uncomplete in READ_ONLY mode
to use for http streaming & co

NOTE: you must buffer the data to a local file, this mode DOES NOT handle 
http/ftp/... streaming

BytesMissing is the predicted number of bytes missing for the file to be loaded
Note that if the file is not optimized for streaming, this number is not accurate
If the movie is successfully loaded (the_file non-NULL), BytesMissing is zero
*/
M4Err M4_MovieOpenEx(const char *fileName, M4File **the_file, u64 *BytesMissing);

/*If requesting a sample fails with error M4UncompleteFile, use this function
to get the number of bytes missing to retrieve the sample*/
u64 M4_GetBytesMissing(M4File *the_file, u32 trackNumber);


/*Fragmented movie extensions*/

/*return 0 if movie isn't fragmented, 1 otherwise*/
u32 M4_IsMovieFragmented(M4File *the_file);
/*return 0 if track isn't fragmented, 1 otherwise*/
u32 M4_IsTrackFragmented(M4File *the_file, u32 TrackID);

/*a file being downloaded may be a fragmented file. In this case only partial info 
is available once the file is successfully open (M4_MovieOpenEx), and since there is 
no information wrt number fragments (which could actually be generated on the fly 
at the sender side), you must call this function on regular bases in order to
load newly downloaded fragments. Note this may result in Track/Movie duration changes
and SampleCount change too ...*/
M4Err M4_RefreshFragmentedMovie(M4File *the_file, u64 *MissingBytes);


/********************************************************************
				READING API FUNCTIONS
********************************************************************/

/*return the number of tracks in the movie, or -1 if error*/
u32 M4_GetTrackCount(M4File *the_file);

/*return the timescale of the movie, 0 if error*/
u32 M4_GetTimeScale(M4File *the_file);

/*return the duration of the movie, 0 if error*/
u64 M4_GetDuration(M4File *the_file);

/*return the creation info of the movie*/
M4Err M4_GetCreationInfo(M4File *the_file, u64 *creationTime, u64 *modificationTime);

/*return the trackID of track number n, or 0 if error*/
u32 M4_GetTrackID(M4File *the_file, u32 trackNumber);

/*return the track number of the track of specified ID, or 0 if error*/
u32 M4_GetTrackByID(M4File *the_file, u32 trackID);

/*gets the enable flag of a track 0: NO, 1: yes, 2: error*/
u8 M4_IsTrackEnabled(M4File *the_file, u32 trackNumber);

/*get the track duration return 0 if bad param*/
u64 M4_GetTrackDuration(M4File *the_file, u32 trackNumber);

/*return the media type FOUR CHAR code type of the media*/
u32 M4_GetMediaType(M4File *the_file, u32 trackNumber);

/*return the media type FOUR CHAR code type of the media*/
u32 M4_GetMediaSubType(M4File *the_file, u32 trackNumber, u32 DescriptionIndex);

/*Get the media (composition) time given the absolute time in the Movie
mediaTime is set to 0 if the media is not playing at that time (empty time segment)*/
M4Err M4_GetMediaTime(M4File *the_file, u32 trackNumber, u32 movieTime, u64 *MediaTime);

/*Get the number of "streams" stored in the media - a media can have several stream descriptions...*/
u32 M4_GetStreamDescriptionCount(M4File *the_file, u32 trackNumber);

/*Get the stream description index (eg, the ESD) for a given time IN MEDIA TIMESCALE
return 0 if error or if empty*/
u32 M4_GetStreamDescriptionIndex(M4File *the_file, u32 trackNumber, u64 for_time);

/*returns 1 if samples refering to the given stream description are present in the file
0 otherwise*/
Bool M4_IsSelfContained(M4File *the_file, u32 trackNumber, u32 sampleDescriptionIndex);

/*get the media duration (without edit) return 0 if no samples (URL streams)*/
u64 M4_GetMediaDuration(M4File *the_file, u32 trackNumber);

/*Get the timeScale of the media. */
u32 M4_GetMediaTimeScale(M4File *the_file, u32 trackNumber);

/*
return 0 if the media has no sync point info (eg, all samples are RAPs)
return 1 if the media has sync points (eg some samples are RAPs)
return 2 if the media has empty sync point info (eg no samples are RAPs). This will likely only happen 
			in scalable context
*/
u8 M4_MediaHasSyncPoints(M4File *the_file, u32 trackNumber);

/*return the maximum chunk duration of the track in milliseconds*/
u32 M4_GetTrackMaxChunkDuration(M4File *the_file, u32 trackNumber);

/*Get the HandlerDescription name. The outName must be:
		 (outName != NULL && *outName == NULL)
the handler name is the string version of the MediaTypes*/
M4Err M4_GetHandlerName(M4File *the_file, u32 trackNumber, const char **outName);

/*Check a DataReference of this track (index >= 1)
A Data Reference allows to construct an MP4 file without integrating the media data*/
M4Err M4_CheckStreamDataReference(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex);

/*get the location of the data. If URL && URN are NULL, the data is in this file
both strings are const: don't free them.*/
M4Err M4_GetStreamDataReference(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, const char **outURL, const char **outURN);

/*Get the number of samples - return 0 if error*/
u32 M4_GetSampleCount(M4File *the_file, u32 trackNumber);

/*It may be desired to fetch samples with a bigger allocated buffer than their real size, in case the decoder
reads more data than available. This sets the amount of extra bytes to allocate when reading samples from this track
NOTE: the dataLength of the sample does NOT include padding*/
M4Err M4_SetSamplesPadding(M4File *the_file, u32 trackNumber, u32 padding_bytes);

/*return a sample given its number, and set the StreamDescIndex of this sample
this index allows to retrieve the stream description if needed (2 media in 1 track)
return NULL if error*/
M4Sample *M4_GetSample(M4File *the_file, u32 trackNumber, u32 sampleNumber, u32 *StreamDescriptionIndex);

/*same as M4_GetSample but doesn't fetch media data
@data_offset (optional): set to sample start offset in file.*/
M4Sample *M4_GetSampleInfo(M4File *the_file, u32 trackNumber, u32 sampleNumber, u32 *StreamDescriptionIndex, u64 *data_offset);

/*returns sample duration in media timeScale*/
u32 M4_GetSampleDuration(M4File *the_file, u32 trackNumber, u32 sampleNumber);

/*gets a sample given a desired decoding time IN MEDIA TIME SCALE
and set the StreamDescIndex of this sample
this index allows to retrieve the stream description if needed (2 media in 1 track)
return M4EOF if the desired time exceeds the media duration
WARNING: the sample may not be sync even though the sync was requested (depends on the media and the editList)
the SampleNum is optional. If non-NULL, will contain the sampleNumber*/
M4Err M4_GetSampleForTime(M4File *the_file, u32 trackNumber, u32 desiredTime, u32 *StreamDescriptionIndex, u8 SearchMode, M4Sample **sample, u32 *SampleNum);


/*Track Edition functions*/

/*return a sample given a desired time in the movie. MovieTime is IN MEDIA TIME SCALE , handles edit list.
and set the StreamDescIndex of this sample
this index allows to retrieve the stream description if needed (2 media in 1 track)
sample must be set to NULL before calling. 

result Sample is NULL if an error occured
if no sample is playing, an empty sample is returned with no data and a DTS set to MovieTime when serching in sync modes
if no sample is playing, the closest sample in the edit time-line is returned when serching in regular modes

WARNING: the sample may not be sync even though the sync was requested (depends on the media and the editList)

Note: this function will handle re-timestamping the sample according to the mapping  of the media time-line 
on the track time-line. The sample TSs (DTS / CTS offset) are expressed in MEDIA TIME SCALE 
(to match the media stream TS resolution as indicated in media header / SLConfig)

sampleNumber is optional and gives the number of the sample in the media
*/
M4Err M4_GetSampleForMovieTime(M4File *the_file, u32 trackNumber, u32 movieTime, u32 *StreamDescriptionIndex, u8 SearchMode, M4Sample **sample, u32 *sampleNumber);


/*get the number of edited segment*/
u32 M4_GetEditSegmentCount(M4File *the_file, u32 trackNumber);

/*Get the desired segment information*/
M4Err M4_GetEditSegment(M4File *the_file, u32 trackNumber, u32 SegmentIndex, u64 *EditTime, u64 *SegmentDuration, u64 *MediaTime, u8 *EditMode);

/*get the number of languages for the copyright*/
u32 M4_GetCopyrightCount(M4File *the_file);
/*get the copyright and its language code given the index*/
M4Err M4_GetCopyright(M4File *the_file, u32 Index, const char **threeCharCodes, const char **notice);
/*get the opaque watermark info if any - returns M4NotSupported if not present*/
M4Err M4_GetWatermark(M4File *the_file, bin128 UUID, u8** data, u32* length);

/*returns 1 if one sample of the track is found to have a composition time offset (DTS<CTS)*/
u32 M4_TrackHasTimeOffsets(M4File *the_file, u32 trackNumber);

/*returns number of random access points - 0 means all samples are random access points*/
u32 M4_GetRandomAccessCount(M4File *the_file, u32 trackNumber);

/*
		MPEG-4 Systems extensions
*/

/*return the root Object descriptor of the movie (can be NULL, OD or IOD, 
you have to check its tag)
YOU HAVE TO DELETE THE DESCRIPTOR
*/
Descriptor *M4_GetRootOD(M4File *the_file);

/*check the presence of a track in IOD. 0: NO, 1: YES, 2: ERROR*/
u8 M4_IsTrackInRootOD(M4File *the_file, u32 trackNumber);

/*Get the ESDescriptor given the StreamDescriptionIndex - YOU HAVE TO DELETE THE DESCRIPTOR*/
ESDescriptor *M4_GetStreamDescriptor(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex);

/*Get the decoderConfigDescriptor given the StreamDescriptionIndex - YOU HAVE TO DELETE THE DESCRIPTOR*/
DecoderConfigDescriptor *M4_GetDecoderInformation(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex);

/*Return the number of track references of a track for a given ReferenceType - return -1 if error*/
s32 M4_GetTrackReferenceCount(M4File *the_file, u32 trackNumber, u32 referenceType);

/*Return the referenced track number for a track and a given ReferenceType and Index
return -1 if error, 0 if the reference is a NULL one, or the trackNumber
*/
M4Err M4_GetTrackReference(M4File *the_file, u32 trackNumber, u32 referenceType, u32 referenceIndex, u32 *refTrack);

u8 M4_GetMoviePLIndication(M4File *the_file, u8 PL_Code);


/*returns file name*/
const char *M4_GetFilename(M4File *the_file);

/*
		Update of the Reading API for MP4 Version 2
*/

/*retrieves the brand of the file. The brand is introduced in V2 to differenciate
MP4, MJPEG2000 and QT while indicating compatibilities
the brand is one of the above defined code, or any other registered brand

minorVersion is an optional parameter (can be set to NULL) , 
		"informative integer for the minor version of the major brand"
AlternateBrandsCount is an optional parameter (can be set to NULL) , 
	giving the number of compatible brands. 

	The function will set brand to 0 if no brand indication is found in the file
*/
M4Err M4_GetMovieVersionInfo(M4File *the_file, u32 *brand, u32 *minorVersion, u32 *AlternateBrandsCount);

/*gets an alternate brand indication. BrandIndex is 1-based
Note that the Major brand should always be indicated in the alternate brands*/
M4Err M4_GetMovieAlternateBrand(M4File *the_file, u32 BrandIndex, u32 *brand);

/*get the number of padding bits at the end of a given sample if any*/
M4Err M4_GetSamplePaddingBits(M4File *the_file, u32 trackNumber, u32 sampleNumber, u8 *NbBits);
/*indicates whether the track samples use padding bits or not*/
Bool M4_HasPaddingBits(M4File *the_file, u32 trackNumber);

/*returns width and height of the given visual sample desc - error if not a visual track*/
M4Err M4_GetVisualEntrySize(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 *Width, u32 *Height);

/*returns samplerate, channels and bps of the given audio track - error if not a audio track*/
M4Err M4_GetAudioEntryInfo(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 *SampleRate, u32 *Channels, u8 *bitsPerSample);

/*returns track visual info - all coord values are expressed as 16.16 fixed point floats*/
M4Err M4_GetTrackVideoInfo(M4File *the_file, u32 trackNumber, u32 *width, u32 *height, s32 *translation_x, s32 *translation_y, s16 *layer);

/*
	User Data Manipulation (cf write API too)
*/

/* Gets the number of UserDataItems with the same ID / UUID in the desired track or 
in the movie if trackNumber is set to 0*/
u32 M4_GetUserDataItemCount(M4File *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID);
/* Gets the UserData for the specified item from the track or the movie if trackNumber is set to 0
data is allocated by the function and is yours to free
you musty pass (userData != NULL && *userData=NULL)*/
M4Err M4_GetUserDataItem(M4File *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID, u32 UserDataIndex, char **userData, u32 *userDataSize);

/*Unknown sample description*/
typedef struct
{
	/*codec tag is the container atom tag*/
	u32 codec_tag;
	/*if any, otherwise 0*/
	bin128 UUID;

	u16 version;
	u16 revision;
	u32 vendor_code;

	/*video codecs only*/
	u32 temporal_quality;
	u32 spacial_quality;
	u16 width, height;
	u32 h_res, v_res;
	u16 depth;
	u16 color_table_index;
	char szCompressorName[33];

	/*audio codecs only*/
	u16 NumChannels;
	u16 bitsPerSample;
	u32 SampleRate;

	/*if present*/
	unsigned char *extension_buf;
	u32 extension_buf_size;
} UnknownStreamDescription;

/*returns wrapper for unknown entries - you must delete it yourself*/
UnknownStreamDescription *M4_GetUnknownStreamDescription(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex);

/*retrieves default values for a track fragment. Each variable is optional and 
if set will contain the default value for this track samples*/
M4Err M4_GetTrackSampleDefaults(M4File *the_file, u32 trackNumber, 
							 u32 *defaultDuration, u32 *defaultSize, u32 *defaultDescriptionIndex,
							 u32 *defaultRandomAccess, u8 *defaultPadding, u16 *defaultDegradationPriority);


/*non standard extensions used for video packets in order to keep AU structure in the file format 
(no normative tables for that). Info is NOT written to disk.
*/
/*get number of fragments for a sample */
u32 M4_GetSampleFragmentCount(M4File *the_file, u32 trackNumber, u32 sampleNumber);
/*get sample fragment size*/
u16 M4_GetSampleFragmentSize(M4File *the_file, u32 trackNumber, u32 sampleNumber, u32 FragmentIndex);



#ifndef M4_READ_ONLY


/********************************************************************
				EDITING/WRITING API FUNCTIONS
********************************************************************/

/*specifies directory for temporary file creation (Edit modes only). This MUST be called before
any sample is added/updated in the file. If not called, temporary files location is OS-dependent through 
stdio tmpfile()*/
M4Err M4_SetTempDir(M4File *file, const char *tmpdir);

/*set the timescale of the movie*/
M4Err M4_SetTimeScale(M4File *the_file, u32 timeScale);

/*creates a new Track. If trackID = 0, the trackID is chosen by the API
returns the track number or 0 if error*/
u32 M4_NewTrack(M4File *the_file, u32 trackID, u32 MediaType, u32 TimeScale);

/*removes the desired track - internal cross dependancies will be updated.
WARNING: any OD streams with references to this track through  ODUpdate, ESDUpdate, ESDRemove commands
will be rewritten*/
M4Err M4_RemoveTrack(M4File *the_file, u32 trackNumber);

/*sets the enable flag of a track*/
M4Err M4_SetTrackEnabled(M4File *the_file, u32 trackNumber, u8 enableTrack);

/*changes the trackID - all track references present in the file are updated
returns error if trackID is already in used in the file*/
M4Err M4_ChangeTrackID(M4File *the_file, u32 trackNumber, u32 trackID);

/*Add samples to a track. Use streamDescriptionIndex to specify the desired stream (if several)*/
M4Err M4_AddSample(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, M4Sample *sample);

/*Add sync shadow sample to a track. 
- There must be a regular sample with the same DTS. 
- Sync Shadow samples MUST be RAP
- Currently, adding sync shadow must be done in order (no sample insertion)
*/
M4Err M4_AddSampleShadow(M4File *the_file, u32 trackNumber, M4Sample *sample);

/*add data to current sample in the track. Use this function for media with
fragmented options such as MPEG-4 video packets. This will update the data size.
Function shall be called in an atomic function with AddTrackSample
CANNOT be used with OD media type*/
M4Err M4_AppendSampleData(M4File *the_file, u32 trackNumber, unsigned char *data, u32 data_size);

/*Add sample references to a track. The dataOffset is the offset of the data in the referenced file
you MUST have created a StreamDescription with URL or URN specifying your referenced file
Use streamDescriptionIndex to specify the desired stream (if several)*/
M4Err M4_AddSampleReference(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, M4Sample *sample, u64 dataOffset);

/*set the duration of the last media sample. If not set, the duration of the last sample is the
duration of the previous one if any, or media TimeScale (default value).*/
M4Err M4_SetLastSampleDuration(M4File *the_file, u32 trackNumber, u32 duration);

/*sets a track reference*/
M4Err M4_SetTrackReference(M4File *the_file, u32 trackNumber, u32 referenceType, u32 ReferencedTrackID);

/*removes a track reference*/
M4Err M4_RemoveTrackReference(M4File *the_file, u32 trackNumber, u32 referenceType, u32 ReferenceIndex);


/*Update Sample functions*/

/*update a given sample of the media.*/
M4Err M4_UpdateSample(M4File *the_file, u32 trackNumber, u32 sampleNumber, M4Sample *sample);

/*update a sample reference in the media. Note that the sample MUST exists,
that sample->data MUST be NULL and sample->dataLength must be NON NULL;*/
M4Err M4_UpdateSampleReference(M4File *the_file, u32 trackNumber, u32 sampleNumber, M4Sample *sample, u64 data_offset);

/*Remove a given sample*/
M4Err M4_RemoveSample(M4File *the_file, u32 trackNumber, u32 sampleNumber);

/*set the save file name of the (edited) movie. 
If the movie is edited, the default fileName is avp_#openName)
NOTE: you cannot save an edited file under the same name (overwrite not allowed)
If the movie is created (WRITE mode), the default filename is #openName*/
M4Err M4_SetFinalFileName(M4File *the_file, char *filename);


/*set the storage mode of a file (FLAT, STREAMABLE, INTERLEAVED)*/
M4Err M4_SetStorageMode(M4File *the_file, u8 storageMode);
u8 M4_GetStorageMode(M4File *the_file);

/*set the interleaving time of media data (INTERLEAVED mode only)
InterleaveTime is in MovieTimeScale*/
M4Err M4_SetInterleavingTime(M4File *the_file, u32 InterleaveTime);
u32 M4_GetInterleavingTime(M4File *the_file);

/*set the copyright in one language.*/
M4Err M4_SetCopyright(M4File *the_file, const char *threeCharCode, char *notice);

/*deletes copyright (1-based indexes)*/
M4Err M4_DeleteCopyright(M4File *the_file, u32 index);

/*set watermark info for movie*/
M4Err M4_SetWatermark(M4File *the_file, bin128 UUID, u8* data, u32 length);

/*Track Edition functions - used to change the normal playback of the media if desired
NOTE: IT IS THE USER RESPONSABILITY TO CREATE A CONSISTENT TIMELINE FOR THE TRACK
This API provides the basic hooks and some basic consistency checking
but can not check the desired functionality of the track edits
*/

/*update or insert a new edit segment in the track time line. Edits are used to modify
the media normal timing. EditTime and EditDuration are expressed in Movie TimeScale
If a segment with EditTime already exists, IT IS ERASED
if there is a segment before this new one, its duration is adjust to match EditTime of
the new segment
WARNING: The first segment always have an EditTime of 0. You should insert an empty or dwelled segment first.*/
M4Err M4_SetEditSegment(M4File *the_file, u32 trackNumber, u32 EditTime, u32 EditDuration, u32 MediaTime, u8 EditMode);

/*same as above except only modifies duartion type and mediaType*/
M4Err M4_ModifyEditSegment(M4File *the_file, u32 trackNumber, u32 seg_index, u32 EditDuration, u32 MediaTime, u8 EditMode);
/*same as above except only appends new segment*/
M4Err M4_AppendEditSegment(M4File *the_file, u32 trackNumber, u32 EditDuration, u32 MediaTime, u8 EditMode);

/*remove the edit segments for the whole track*/
M4Err M4_RemoveEditSegments(M4File *the_file, u32 trackNumber);

/*remove the given edit segment (1-based index). If this is not the last segment, the next segment duration
is updated to maintain a continous timeline*/
M4Err M4_RemoveEditSegment(M4File *the_file, u32 trackNumber, u32 seg_index);

/*
				User Data Manipulation

		You can add specific typed data to either a track or the movie: the UserData
	The type must be formated as a FourCC if you have a registered 4CC type
	but the usual is to set a UUID (128 bit ID for atom type) which never conflict
	with existing structures in the format
		To manipulate a UUID user data set the UserDataType to 0 and specify a valid UUID.
Otherwise the UUID parameter is ignored
		Several items with the same ID or UUID can be added (this allows you to store any
	kind/number of private information under a unique ID / UUID)
*/
/*Add a user data item in the desired track or in the movie if TrackNumber is 0*/
M4Err M4_AddUserData(M4File *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID, char *data, u32 DataLength);

/*remove all user data items from the desired track or from the movie if TrackNumber is 0*/
M4Err M4_RemoveUserData(M4File *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID);

/*remove a user data item from the desired track or from the movie if TrackNumber is 0
use the UDAT read functions to get the item index*/
M4Err M4_RemoveUserDataItem(M4File *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID, u32 UserDataIndex);

/*
		Update of the Writing API for MP4 Version 2
*/	

/*use a compact track version for sample size. This is not usually recommended 
except for speech codecs where the track has a lot of small samples
compaction is done automatically while writing based on the track's sample sizes*/
M4Err M4_UseCompactSize(M4File *the_file, u32 trackNumber, u8 CompactionOn);

/*sets the brand of the movie*/
M4Err M4_SetMovieVersionInfo(M4File *the_file, u32 MajorBrand, u32 MinorVersion);

/*adds or remove an alternate brand for the movie*/
M4Err M4_ModifyAlternateBrand(M4File *the_file, u32 Brand, u8 AddIt);

/*set the number of padding bits at the end of a given sample if needed
if the function is never called the padding bit info is ignored
this MUST be called on an existin sample*/
M4Err M4_SetSamplePaddingBits(M4File *the_file, u32 trackNumber, u32 sampleNumber, u8 NbBits);


/*since v2 you must specify w/h of video tracks for authoring tools (no decode the video cfg / first sample)*/
M4Err M4_SetVisualEntrySize(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 Width, u32 Height);

/*mainly used for 3GPP text since most ISO-based formats ignore these (except MJ2K) 
all coord values are expressed as 16.16 fixed point floats*/
M4Err M4_SetTrackVideoInfo(M4File *the_file, u32 trackNumber, u32 width, u32 height, s32 translation_x, s32 translation_y, s16 layer);

/*set SR & nbChans for audio description*/
M4Err M4_SetAudioDescriptionInfo(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 sampleRate, u32 nbChannels, u8 bitsPerSample);

/*non standard extensions: set/remove a fragment of a sample - this is used for video packets
in order to keep AU structure in the file format (no normative tables for that). Info is NOT written to disk*/
M4Err M4_AddSampleFragment(M4File *the_file, u32 trackNumber, u32 sampleNumber, u16 FragmentSize);
M4Err M4_RemoveSampleFragment(M4File *the_file, u32 trackNumber, u32 sampleNumber);

/*set CTS unpack mode (used for B-frames): in unpack mode, each sample uses one entry in CTTS tables
unpack=0: set unpack on (must be called BEFORE adding any sample to the track)
unpack=1: set unpack off and repacks all table info
adjust_cts: adjusts CTS when repacking. CTS adjustment is done as follows:
	- sample without CTS offsets are considered as B-frames
	- all other samples must be added with a constant CTS offset of one frame delay.
	Whenever N consecutive B-frames are found, the previous sample before the run of Bs has its offset increased by
	N*"one frame delay"
*/
M4Err M4_SetCTSPackMode(M4File *the_file, u32 trackNumber, Bool unpack, Bool adjust_cts);
/*modify CTS offset of a given sample (used for B-frames) - MUST be called in unpack mode only*/
M4Err M4_SetSampleCTSOffset(M4File *the_file, u32 trackNumber, u32 sample_number, u32 offset);
/*remove CTS offset table (used for B-frames)*/
M4Err M4_DeleteCTSTable(M4File *the_file, u32 trackNumber);

/*
	some authoring extensions
*/
/*sets name for authoring - if name is NULL reset authoring name*/
M4Err M4_SetTrackName(M4File *the_file, u32 trackNumber, char *name);
/*gets authoring name*/
const char *M4_GetTrackName(M4File *the_file, u32 trackNumber);

/*
			MPEG-4 Extensions
*/

/*set a profile and level indication for the movie iod (created if needed)
if the flag is ProfileLevel is 0 this means the movie doesn't require
the specific codec (equivalent to 0xFF value in MPEG profiles)*/
M4Err M4_SetMoviePLIndication(M4File *the_file, u8 PL_Code, u8 ProfileLevel);

/*set the rootOD ID of the movie if you need it. By default, movies are created without root ODs*/
M4Err M4_SetRootOD_ID(M4File *the_file, u32 OD_ID);

/*set the rootOD URL of the movie if you need it (only needed to create empty mp4 file pointing 
to external ressource)*/
M4Err M4_SetRootOD_URL(M4File *the_file, char *url_string);

/*remove the root OD*/
M4Err M4_RemoveRootOD(M4File *the_file);

/*Add a system descriptor to the OD of the movie*/
M4Err M4_AddDescriptorToRootOD(M4File *the_file, Descriptor *theDesc);

/*add a track to the root OD*/
M4Err M4_AddTrackToRootOD(M4File *the_file, u32 trackNumber);

/*remove a track to the root OD*/
M4Err M4_RemoveTrackFromRootOD(M4File *the_file, u32 trackNumber);

/*Create a new StreamDescription (ESDescriptor) in the file. The URL and URN are used to 
describe external media, this will creat a data reference for the media*/
M4Err M4_NewStreamDescription(M4File *the_file, u32 trackNumber, ESDescriptor *esd, char *URLname, char *URNname, u32 *outDescriptionIndex);

/*use carefully. Very usefull when you made a lot of changes (IPMP, IPI, OCI, ...)
THIS WILL REPLACE THE WHOLE DESCRIPTOR ...*/
M4Err M4_ChangeStreamDescriptor(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, ESDescriptor *newESD);

/*Add a system descriptor to the ESD of a stream - you have to delete the descriptor*/
M4Err M4_AddDescriptorToESD(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, Descriptor *theDesc);


/*Default extensions*/

/*Create a new unknown StreamDescription in the file. The URL and URN are used to 
describe external media, this will creat a data reference for the media
use this to store media not currently supported by the ISO media format
NOTE: you shall not include ISO media atom/box header info in this buffer
if you wish to use UUID (recommended) specify an entry type of 0
otherwise uuid is ignored and entryType shall be a 4CC integer
*/
M4Err M4_NewUnknownStreamDescription(M4File *the_file, u32 trackNumber, u32 entry_type, bin128 entry_UUID, char *URLname, char *URNname, UnknownStreamDescription *udesc, u32 *outDescriptionIndex);

/*change the data field of an unknown sample description*/
M4Err M4_ChangeUnknownStreamDescription(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, UnknownStreamDescription *udesc);



/*
			Movie Fragments Writing API
		Movie Fragments is a feature of ISO media files for fragmentation
	of a presentation meta-data and interleaving with its media data.
	This enables faster http fast start for big movies, and also reduces the risk
	of data loss in case of a recording crash, because meta data and media data
	can be written to disk at regular times
		This API provides simple function calls to setup such a movie and write it
	The process implies:
		1- creating a movie in the usual way (track, stream descriptions, (IOD setup
	copyright, ...)
		2- possibly add some samples in the regular fashion
		3- setup track fragments for all track that will be written in a fragmented way
	(note that you can create/write a track that has no fragmentation at all)
		4- finalize the movie for fragmentation (this will flush all meta-data and 
	any media-data added to disk, ensuring all vital information for the presentation
	is stored on file and not lost in case of crash/poweroff)
	
	  then 5-6 as often as desired
		5- start a new movie fragment
		6- add samples to each setup track


  IMPORTANT NOTES:
		* Movie Fragments can only be used in M4_OPEN_WRITE mode (capturing)
  and no editing functionalities can be used
		* the fragmented movie API uses TrackID and not TrackNumber 
*/

/*
setup a track for fragmentation by specifying some default values for 
storage efficiency
*TrackID: track identifier
*DefaultStreamDescriptionIndex: the default description used by samples in this track
*DefaultSampleDuration: default duration of samples in this track
*DefaultSampleSize: default size of samples in this track (0 if unknown)
*DefaultSampleIsSync: default key-flag (RAP) of samples in this track
*DefaultSamplePadding: default padding bits for samples in this track
*DefaultDegradationPriority: default degradation priority for samples in this track

*/
M4Err M4_SetupTrackFragment(M4File *the_file, u32 TrackID, 
							 u32 DefaultStreamDescriptionIndex,
							 u32 DefaultSampleDuration,
							 u32 DefaultSampleSize,
							 u8 DefaultSampleIsSync,
							 u8 DefaultSamplePadding,
							 u16 DefaultDegradationPriority);

/*flushes data to disk and prepare movie fragmentation*/
M4Err M4_FinalizeMovieForFragments(M4File *the_file);

/*starts a new movie fragment*/
M4Err M4_StartFragment(M4File *the_file);


enum
{
	/*indicates that the track fragment has no samples but still has a duration
	(silence-detection in audio codecs, ...). 
	param: indicates duration*/
	TFO_EMPTY,
	/*I-Frame detection: this can reduce file size by detecting I-frames and
	optimizing sample flags (padding, priority, ..)
	param: on/off (0/1)*/
	TFO_IFRAME,
	/*activate data cache on track fragment. This is usefull when writing interleaved
	media from a live source (typically audio-video), and greatly reduces file size
	param: Number of samples (> 1) to cache before disk flushing. You shouldn't try 
	to cache too many samples since this will load your memory. base that on FPS/SR*/
	TFO_DATACACHE
};

/*set options. Options can be set at the begining of each new fragment only, and for the
lifetime of the fragment*/
M4Err M4_TrackFragmentSetOptions(M4File *the_file, u32 TrackID, u32 Code, u32 param);


/*adds a sample to a fragmented track

*TrackID: destination track
*sample: sample to add
*StreamDescriptionIndex: stream description for this sample. If 0, the default one 
is used
*Duration: sample duration.
Note: because of the interleaved nature of the meta/media data, the sample duration
MUST be provided (in case of regular tracks, this was computed internally by the lib)
*PaddingBits: padding bits for the sample, or 0
*DegradationPriority for the sample, or 0

*/

M4Err M4_TrackFragmentAddSample(M4File *the_file, u32 TrackID, M4Sample *sample, 
								 u32 StreamDescriptionIndex, 
								 u32 Duration,
								 u8 PaddingBits, u16 DegradationPriority);

/*appends data into last sample of track for video fragments/other media
CANNOT be used with OD tracks*/
M4Err M4_TrackFragmentAppendData(M4File *the_file, u32 TrackID, unsigned char *data, u32 data_size, u8 PaddingBits);



/******************************************************************
		GENERIC Publishing API
******************************************************************/

/*Set the sync shadow on/off for a file.
Shadowing allows you to add Random Access samples that will REPLACE the desired sample
Most of the time the shadowed sample should be place "outside" the track time line 
through an edit list*/
M4Err M4_SetSyncShadowEnabled(M4File *the_file, u32 trackNumber, u8 SyncShadowEnabled);

/*Use this function to do the shadowing if you use shadowing.
the sample to be shadowed MUST be a non-sync sample (ignored if not)
the sample shadowing must be a Sync sample (error if not)*/
M4Err M4_SetSyncShadow(M4File *the_file, u32 trackNumber, u32 sampleNumber, u32 syncSample);

/*set the GroupID of a track (only used for optimized interleaving). By setting GroupIDs
you can specify the storage order for media data of a group of streams. This is usefull
for BIFS presentation so that static resources of the scene can be downloaded before BIFS*/
M4Err M4_SetTrackGroup(M4File *the_file, u32 trackNumber, u32 GroupID);

/*set the priority of a track within a Group (used for optimized interleaving and hinting). 
This allows tracks to be stored before other within a same group, for instance the 
hint track data can be stored just before the media data, reducing disk seeking
for a same time, within a group of tracks, the track with the lowest inversePriority will 
be written first*/
M4Err M4_SetTrackPriorityInGroup(M4File *the_file, u32 trackNumber, u32 InversePriority);

/*set the max SamplesPerChunk (for file optimization, mainly in FLAT and STREAMABLE modes)*/
M4Err M4_SetMaxSamplesPerChunk(M4File *the_file, u32 trackNumber, u32 maxSamplesPerChunk);

/*associate a given SL config with a given ESD while extracting the OD information
all the SL params must be fixed by the calling app!
The SLConfig is stored by the API for further use. A NULL pointer will result
in using the default SLConfig (predefined = 2) remapped to predefined = 0
This is usefull while reading the IOD / OD stream of an MP4 file. Note however that
only full AUs are extracted, therefore the calling application must SL-packetize the streams*/
M4Err M4_SetExtractionSLConfig(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, SLConfigDescriptor *slConfig);

M4Err M4_GetExtractionSLConfig(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, SLConfigDescriptor **slConfig);

u32 M4_GetTrackGroup(M4File *the_file, u32 trackNumber);
u32 M4_GetTrackPriorityInGroup(M4File *the_file, u32 trackNumber);


/*same as M4_MovieClose but keeps movie loaded - used for editing*/
M4Err M4_MovieWrite(M4File *the_file);
/*stores movie config (storage mode, interleave time, track groupIDs, priorities and names) in UDTA(kept on disk)
if @remove_all is set, removes all stored info, otherwise recompute all stored info*/
M4Err M4_StoreMovieConfig(M4File *the_file, Bool remove_all);
/*restores movie config (storage mode, interleave time, track groupIDs, priorities and names) if found*/
M4Err M4_LoadMovieConfig(M4File *the_file);


/******************************************************************
		GENERIC HINTING WRITING API
******************************************************************/

/*supported hint formats - ONLY RTP now*/
enum
{
	M4_Hint_RTP	= FOUR_CHAR_INT( 'r', 't', 'p', ' ' )
};


/*Setup the resources based on the hint format
This function MUST be called after creating a new hint track and before
any other calls on this track*/
M4Err M4H_SetupHintTrack(M4File *the_file, u32 trackNumber, u32 HintType);

/*Create a HintDescription for the HintTrack
the rely flag indicates whether a reliable transport protocol is desired/required
for data transport
	0: not desired (UDP/IP). NB: most RTP streaming servers only support UDP/IP for data
	1: preferable (TCP/IP if possible or UDP/IP)
	2: required (TCP/IP only)
The HintDescriptionIndex is set, to be used when creating a HINT sample
*/
M4Err M4H_NewHintDescription(M4File *the_file, u32 trackNumber, s32 HintTrackVersion, s32 LastCompatibleVersion, u8 Rely, u32 *HintDescriptionIndex);

/*Starts a new sample for the hint track. A sample is just a collection of packets
the transmissionTime is indicated in the media timeScale of the hint track*/
M4Err M4H_BeginHintSample(M4File *the_file, u32 trackNumber, u32 HintDescriptionIndex, u32 TransmissionTime);

/*stores the hint sample in the file once all your packets for this sample are done
set IsRandomAccessPoint if you want to indicate that this is a random access point 
in the stream*/
M4Err M4H_EndHintSample(M4File *the_file, u32 trackNumber, u8 IsRandomAccessPoint);


/******************************************************************
		PacketHandling functions
		Data can be added at the end or at the beginning of the current packet
		by setting AtBegin to 1 the data will be added at the begining
		This allows constructing the packet payload before any meta-data
******************************************************************/

/*adds a blank chunk of data in the sample that is skipped while streaming*/
M4Err M4H_AddBlankData(M4File *the_file, u32 trackNumber, u8 AtBegin);

/*adds a chunk of data in the packet that is directly copied while streaming
NOTE: dataLength MUST BE <= 14 bytes, and you should only use this function
to add small blocks of data (encrypted parts, specific headers, ...)*/
M4Err M4H_AddDirectData(M4File *the_file, u32 trackNumber, char *data, u32 dataLength, u8 AtBegin);

/*adds a reference to some sample data in the packet
SourceTrackID: the ID of the track where the referenced sample is
SampleNumber: the sample number containing the data to be added
DataLength: the length of bytes to copy in the packet
offsetInSample: the offset in bytes in the sample at which to begin copying data

extra_data: only used when the sample is actually the sample that will contain this packet
(usefull to store en encrypted version of a packet only available while streaming)
	In this case, set SourceTrackID to the HintTrack ID and SampleNumber to 0
	In this case, the DataOffset MUST BE NULL and length will indicate the extra_data size

Note that if you want to reference a previous HintSample in the hintTrack, you will 
have to parse the sample yourself ...
*/
M4Err M4H_AddSampleData(M4File *the_file, u32 trackNumber, u32 SourceTrackID, u32 SampleNumber, u16 DataLength, u32 offsetInSample, char *extra_data, u8 AtBegin);


/*adds a reference to some stream description data in the packet (headers, ...)
SourceTrackID: the ID of the track where the referenced sample is
StreamDescriptionIndex: the index of the stream description in the desired track
DataLength: the length of bytes to copy in the packet
offsetInDescription: the offset in bytes in the description at which to begin copying data

Since it is far from being obvious what this offset is, we recommend not using this 
function. The ISO Media Format specification is currently being updated to solve
this issue*/
M4Err M4H_AddStreamDescriptionData(M4File *the_file, u32 trackNumber, u32 SourceTrackID, u32 StreamDescriptionIndex, u16 DataLength, u32 offsetInDescription, u8 AtBegin);


/******************************************************************
		RTP SPECIFIC WRITING API
******************************************************************/

/*Creates a new RTP packet in the HintSample. If a previous packet was created, 
it is stored in the hint sample and a new packet is created.
- relativeTime: RTP time offset of this packet in the HintSample if any - in hint track 
time scale. Used for data smoothing by servers.
- PackingBit: the 'P' bit of the RTP packet header
- eXtensionBit: the'X' bit of the RTP packet header
- MarkerBit: the 'M' bit of the RTP packet header
- PayloadType: the payload type, on 7 bits, format 0x0XXXXXXX
- B_frame: indicates if this is a B-frame packet. Can be skipped by a server
- IsRepeatedPacket: indicates if this is a duplicate packet of a previous one.
Can be skipped by a server
- SequenceNumber: the RTP base sequence number of the packet. Because of support for repeated
packets, you have to set the sequence number yourself.*/
M4Err M4H_RTP_NewPacket(M4File *the_file, u32 trackNumber, s32 relativeTime, u8 PackingBit, u8 eXtensionBit, u8 MarkerBit, u8 PayloadType, u8 B_frame, u8 IsRepeatedPacket, u16 SequenceNumber);

/*set the flags of the RTP packet*/
M4Err M4H_RTP_SetPacketFlags(M4File *the_file, u32 trackNumber, u8 PackingBit, u8 eXtensionBit, u8 MarkerBit, u8 B_frame, u8 IsRepeatedPacket);

/*set the time offset of this packet. This enables packets to be placed in the hint track 
in decoding order, but have their presentation time-stamp in the transmitted 
packet in a different order. Typically used for MPEG video with B-frames
*/
M4Err M4H_RTP_SetPacketTimeOffset(M4File *the_file, u32 trackNumber, s32 timeOffset);

								   
/*set some specific info in the HintDescription for RTP*/

/*sets the RTP TimeScale that the server use to send packets
some RTP payloads may need a specific timeScale that is not the timeScale in the file format
the default timeScale choosen by the API is the MediaTimeScale of the hint track*/
M4Err M4H_RTP_SetTimeScale(M4File *the_file, u32 trackNumber, u32 HintDescriptionIndex, u32 TimeScale);
/*sets the RTP TimeOffset that the server will add to the packets
if not set, the server adds a random offset*/
M4Err M4H_RTP_SetTimeOffset(M4File *the_file, u32 trackNumber, u32 HintDescriptionIndex, u32 TimeOffset);
/*sets the RTP SequenceNumber Offset that the server will add to the packets
if not set, the server adds a random offset*/
M4Err M4H_RTP_SetSequenceNumberOffset(M4File *the_file, u32 trackNumber, u32 HintDescriptionIndex, u32 SequenceNumberOffset);



/******************************************************************
		SDP SPECIFIC WRITING API
******************************************************************/
/*add an SDP line to the SDP container at the track level (media-specific SDP info)
NOTE: the \r\n end of line for SDP is automatically inserted*/
M4Err M4H_SDP_TrackAddLine(M4File *the_file, u32 trackNumber, const char *text);
/*remove all SDP info at the track level*/
M4Err M4H_SDP_CleanTrack(M4File *the_file, u32 trackNumber);

/*add an SDP line to the SDP container at the movie level (presentation SDP info)
NOTE: the \r\n end of line for SDP is automatically inserted*/
M4Err M4H_SDP_MovieAddLine(M4File *the_file, const char *text);
/*remove all SDP info at the movie level*/
M4Err M4H_SDP_CleanMovie(M4File *the_file);

#endif	/*M4_READ_ONLY*/

/*Get SDP info at the movie level*/
M4Err M4H_SDP_GetSDP(M4File *the_file, const char **sdp, u32 *length);
/*Get SDP info at the track level*/
M4Err M4H_SDP_GetTrackSDP(M4File *the_file, u32 trackNumber, const char **sdp, u32 *length);


/*dumps MP4 structures into XML trace file */
M4Err M4_FileDump(M4File *file, FILE *trace);
/*dumps MP4 RTP hint samples structure into XML trace file
	@trackNumber, @SampleNum: hint track and hint sample number
	@trace: output
*/
M4Err M4H_DumpHintSample(M4File *the_file, u32 trackNumber, u32 SampleNum, FILE * trace);



/*
				3GPP specific extensions

		NOTE: MPEG-4 OD Framework cannot be used with 3GPP files. Stream Descriptions
	are not ESDescriptor, just generic config options as specified in this file
*/

typedef struct 
{
	/*indicate type (0 is regular 8k AMR, 1 is 16k wide band)*/
	u8 WideBandAMR;
	/*4CC vendor name*/
	u32 vendor;
	/*codec version*/
	u8 decoder_version;
	/*num of mode for the codec (either AMR or AMR-WB modes)*/
	u16 mode_set;
	/*changes in codec mode per sample - as AMR-FT payload attribute is written
	in the sample, this info is useless*/
	u8 mode_change_period;
	/*number of AMR frames per MP4 sample, >0 and <=15. Note that the very last sample
	may contain less frames. Usually an AMR decoder should be able to figure out
	the number of frames*/
	u8 frames_per_sample;
} AMRConfiguration, *LPAMRCONFIGURATION;

/*return the AMR config for AMR stream descriptions - YOU HAVE TO DELETE THE CONFIG*/
LPAMRCONFIGURATION M4_AMR_GetStreamConfig(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex);


typedef struct 
{
	/*4CC vendor name*/
	u32 vendor;
	/*codec version*/
	u8 decoder_version;
	/*H263 Level*/
	u8 Level;
	/*H263 Profile*/
	u8 Profile;
} H263Configuration, *LPH263CONFIGURATION;

/*return the H263 config for H263 stream description - YOU HAVE TO DELETE THE CONFIG*/
LPH263CONFIGURATION M4_H263_GetStreamConfig(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex);


#ifndef M4_READ_ONLY

/*create the track config*/
M4Err M4_AMR_NewStreamConfig(M4File *the_file, u32 trackNumber, LPAMRCONFIGURATION amr_config, char *URLname, char *URNname, u32 *outDescriptionIndex);
/*update the track config*/
M4Err M4_AMR_UpdateStreamConfig(M4File *the_file, u32 trackNumber, LPAMRCONFIGURATION amr_config, u32 DescriptionIndex);

/*create the track config*/
M4Err M4_H263_NewStreamConfig(M4File *the_file, u32 trackNumber, LPH263CONFIGURATION h263_config, char *URLname, char *URNname, u32 *outDescriptionIndex);
/*update the track config*/
M4Err M4_H263_UpdateStreamConfig(M4File *the_file, u32 trackNumber, LPH263CONFIGURATION h263_config, u32 DescriptionIndex);

#endif	/*M4_READ_ONLY*/

/*AVC/H264 extensions*/

/*used for sequenceParameterSetNALUnit and pictureParameterSetNALUnit*/
typedef struct
{
	u16 size;
	char *data;
} AVCConfigSlot;

typedef struct 
{
	u8 configurationVersion;
	u8 AVCProfileIndication;
	u8 profile_compatibility;
	u8 AVCLevelIndication; 
	u8 nal_unit_size;
	
	u8 numSequenceParameterSets;
	AVCConfigSlot *sequenceParameterSets;

	u8 numPictureParameterSets;
	AVCConfigSlot *pictureParameterSets;
} AVCDecoderConfigurationRecord;

void DeleteAVCConfig(AVCDecoderConfigurationRecord *cfg);


/*
	3GP timed text handling

	NOTE: currently only writing API is developped, the reading one is not used in MPEG-4 since
	MPEG-4 maps 3GP timed text to MPEG-4 Streaming Text (part 17)
*/

/*set streamihng text reading mode: if do_convert is set, all text samples will be retrieved as TTUs
and ESD will be emulated for text tracks.*/
M4Err M4_SetStreamingTextMode(M4File *the_file, Bool do_convert);

M4Err M4_DumpTextTrack(M4File *the_file, u32 track, FILE *dump);

/*returns encoded TX3G atom (text sample description for 3GPP text streams) as needed by RTP or other stds:
	@sidx: 1-based stream description index
	@sidx_offset: 
		if 0, the sidx will NOT be written before the encoded TX3G
		if not 0, the sidx will be written before the encoded TX3G, with the given offset. Offset sshould be at 
		least 128 for most commmon usage of TX3G (RTP, MPEG-4 timed text, etc)

*/
M4Err M4_GetEncodedTX3G(M4File *file, u32 track, u32 sidx, u32 sidx_offset, char **tx3g, u32 *tx3g_size);


/*text sample formatting*/
typedef struct _3gp_text_sample *M4TXTSAMPLE;
/*creates text sample handle*/
M4TXTSAMPLE M4_NewTextSample();
/*destroy text sample handle*/
void M4_DeleteTextSample(M4TXTSAMPLE tx_samp);

#ifndef M4_READ_ONLY

/*Create a new TextSampleDescription in the file. 
The URL and URN are used to describe external media, this will create a data reference for the media
TextSampleDescriptor is defined in m4_descriptors.h
*/
M4Err M4_NewTextDescription(M4File *the_file, u32 trackNumber, TextSampleDescriptor *desc, char *URLname, char *URNname, u32 *outDescriptionIndex);

/*reset text sample content*/
M4Err M4_TxtReset(M4TXTSAMPLE tx_samp);
/*sets UTF16 marker for text data. This MUST be called on an empty sample. If text data added later 
on (cf below) is not formatted as UTF16 data(2 bytes char) the resulting text sample won't be compliant, 
but this library WON'T WARN*/
M4Err M4_TxtSetUTF16Marker(M4TXTSAMPLE samp);
/*append text to sample - text_len is the number of bytes to be written from text_data. This allows 
handling UTF8 and UTF16 strings in a transparent manner*/
M4Err M4_TxtAddText(M4TXTSAMPLE tx_samp, char *text_data, u32 text_len);
/*append style modifyer to sample*/
M4Err M4_TxtAddStyle(M4TXTSAMPLE tx_samp, StyleRecord *rec);
/*appends highlight modifier for the sample 
	@start_char: first char highlighted, 
	@end_char: first char not highlighted*/
M4Err M4_TxtAddHighlight(M4TXTSAMPLE samp, u16 start_char, u16 end_char);
/*sets highlight color for the whole sample*/
M4Err M4_TxtSetHighlightColor(M4TXTSAMPLE samp, u8 r, u8 g, u8 b, u8 a);
M4Err M4_TxtSetHighlightColorARGB(M4TXTSAMPLE samp, u32 argb);
/*appends a new karaoke sequence in the sample
	@start_time: karaoke start time expressed in text stream timescale, but relative to the sample media time
*/
M4Err M4_TxtAddKaraoke(M4TXTSAMPLE samp, u32 start_time);
/*appends a new segment in the current karaoke sequence - you must build sequences in order to be compliant
	@end_time: segment end time expressed in text stream timescale, but relative to the sample media time
	@start_char: first char highlighted, 
	@end_char: first char not highlighted
*/
M4Err M4_TxtSetKaraokeSeg(M4TXTSAMPLE samp, u32 end_time, u16 start_char, u16 end_char);
/*sets scroll delay for the whole sample (scrolling is enabled through TextSampleDescriptor.DisplayFlags)
	@scroll_delay: delay for scrolling expressed in text stream timescale
*/
M4Err M4_TxtSetScrollDelay(M4TXTSAMPLE samp, u32 scroll_delay);
/*appends hyperlinking for the sample
	@URL: ASCII url
	@altString: ASCII hint (tooltip, ...) for end user
	@start_char: first char hyperlinked, 
	@end_char: first char not hyperlinked
*/
M4Err M4_TxtAddHypertext(M4TXTSAMPLE samp, char *URL, char *altString, u16 start_char, u16 end_char);
/*sets current text box (display pos&size within the text track window) for the sample*/
M4Err M4_TxtSetBox(M4TXTSAMPLE samp, s16 top, s16 left, s16 bottom, s16 right);
/*appends blinking for the sample
	@start_char: first char blinking, 
	@end_char: first char not blinking
*/
M4Err M4_TxtAddBlink(M4TXTSAMPLE samp, u16 start_char, u16 end_char);
/*sets wrap flag for the sample - currently only 0 (no wrap) and 1 ("soft wrap") are allowed in 3GP*/
M4Err M4_TxtSetWrap(M4TXTSAMPLE samp, u8 wrap_flags);

/*formats sample as a regular M4Sample. The resulting sample will always be marked as random access
text sample content is kept untouched*/
M4Sample *M4_TxtToSample(M4TXTSAMPLE tx_samp);

#endif	//M4_READ_ONLY


#ifdef __cplusplus
}
#endif


#endif

