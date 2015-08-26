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


#ifndef _M4_AUTHOR_DEV_H_
#define _M4_AUTHOR_DEV_H_

#include <gpac/m4_author.h>

/*since 0.2.2, we use zlib for xmt/x3d reading to handle gz files*/
#include <zlib.h>


/*extremely crude xml parsing helper
	All string returns are maintained by parser and shall not be freed by user.
	The string returned are pointers to internal buffers, hence their content will liokely change 
	at each operations. You shall therefore copy their value if you need them later on
*/


#define XML_LINE_SIZE	8000
typedef struct 
{
	/*gz input file*/
	gzFile gz_in;
	/*eof*/
	Bool done;
	/*current line parsed (mainly used for error reports)*/
	u32 line;
	/*0: UTF-8, 1: UTF-16 BE, 2: UTF-16 LE. String input is always converted back to utf8*/
	u32 unicode_type;
	/*line buffer - needs refinement, cannot handle attribute values with size over XMT_LINE_SIZE (except string
	where space is used as a line-break...)*/
	char line_buffer[XML_LINE_SIZE];
	/*name buffer for elements and attributes*/
	char name_buffer[1000];
	/*dynamic buffer for attribute values*/
	char *value_buffer;
	u32 att_buf_size;
	/*line size and current position*/
	s32 line_size, current_pos;
	/*absolute line start position in file (needed for hard seeking in xmt-a)*/
	s32 line_start_pos;
} XMLParser;

/*inits parser with given local file (handles gzip) - checks UTF8/16*/
M4Err xml_init_parser(XMLParser *parser, const char *fileName);
/*reset parser (closes file and destroy value buffer)*/
void xml_reset_parser(XMLParser *parser);
/*get next element name*/
char *xml_get_element(XMLParser *parser);
/*returns 1 if given element is done*/
Bool xml_element_done(XMLParser *parser, char *name);
/*skip given element*/
void xml_skip_element(XMLParser *parser, char *name);
/*skip element attributes*/
void xml_skip_attributes(XMLParser *parser);
/*returns 1 if element has attributes*/
Bool xml_has_attributes(XMLParser *parser);
/*returns next attribute name*/
char *xml_get_attribute(XMLParser *parser);
/*returns attribute value*/
char *xml_get_attribute_value(XMLParser *parser);
/*translate input string, removing all special XML encoding. Returned string shall be freed by caller*/
char *xml_translate_xml_string(char *str);
/*checks if next line should be loaded - is not needed except when seeking file outside the parser routines*/
void xml_check_line(XMLParser *parser);


M4Err import_message(M4TrackImporter *import, M4Err e, char *format, ...);
void import_progress(M4TrackImporter *import, u32 cur_samp, u32 count);


/*
		RTP -> SL packetization tool
	You should ONLY modify the SLHeader while packetizing, all the rest is private
	to the tool.
	Also note that AU start/end is automatically updated, therefore you should only
	set CTS-DTS-OCR-sequenceNumber (which is automatically incremented when spliting a payload)
	-padding-idle infos
	SL flags are computed on the fly, but you may wish to modify them in case of 
	packet drop/... at the encoder side

*/
typedef struct _rtpbuilder
{
	/*input packet sl header cfg. modify oly if needed*/
	SLHeader sl_header;

	/*
	
		PRIVATE _ DO NOT TOUCH
	*/
	
	/*RTP payload type (RFC type, NOT the RTP hdr payT)*/
	u32 rtp_payt;
	/*packetization flags*/
	u32 flags;
	/*Path MTU size without 12-bytes RTP header*/
	u32 Path_MTU;

	/*payload type of RTP packets - only one payload type can be used in GPAC*/
	u8 PayloadType;

	/*RTP header of current packet*/
	RTPHeader rtp_header;

	/*signals new packet*/
	void (*OnNewPacket)(void *cbk_obj, RTPHeader *header);
	/*callback on packet */
	void (*OnPacketDone)(void *cbk_obj, RTPHeader *header, 
		/*payload header if any (AMR, MPEG-4 SL, ...), may be NULL*/
		char *payl_hdr, u32 payl_hdr_size, 
		/*data payload (ptr to input data)*/
		char *payload, u32 payload_size);

	/*offset_from_orig indicates the offset in b ytes the payload is located
	from the original input buffer (for hint tracks mainly)*/
	void (*OnDataReference)(void *cbk_obj, u32 payload_size, u32 offset_from_orig);
	void (*OnData)(void *cbk_obj, char *data, u32 data_size);
	void *cbk_obj;


	/*set to 1 if first AU in RTP packet is RAP*/
	Bool RAP_Packet;
	
	/*rest of struct is for MPEG-4 Generic hinting */
	/*SL to RTP map*/
	RTPSLMap slMap;
	/*SL conf and state*/
	SLConfigDescriptor sl_config;

	/*set to 1 if firstSL in RTP packet*/
	Bool first_sl_in_rtp;
	Bool has_AU_header;
	/*current info writers*/
	BitStream *auheader, *payload;

	/*AU SN of last au*/
	u32 last_au_sn;

	/*info for the current packet*/
	u32 auh_size, bytesInPacket;
} M4RTPBuilder;


/*packetization routines*/
M4Err M4RTP_ProcessMPEG4(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);
M4Err M4RTP_ProcessH263(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);
M4Err M4RTP_ProcessAMR(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);
M4Err M4RTP_ProcessMPEG12(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);
M4Err M4RTP_ProcessText(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize, u32 duration, u8 descIndex);


/*RTP track hinter*/
typedef struct _tagM4Hinter
{
	u32 HinterFlags;

	Bool copy_media;
	u8 PayloadType, SampleIsRAP, hasBFrames;
	u32 Path_MTU, Last_RTP_SeqNum, base_offset_in_sample;
	M4File *file;
	u32 TrackNum, HintTrack, HintSample, RTPTime, TrackID, HintID;
	u32 OrigTimeScale;
	u32 Priority, Group;
	//hinter
	LPM4RTPBUILDER hinter;

	void (*OnProgress)(void *cbk_obj, u32 done, u32 total);
	void *cbk_obj;

	//stats
	u32 TotalSample, CurrentSample;
} MP4_RTPHinter;



M4Err M4SM_LoaderInit_BT(M4ContextLoader *load);
void M4SM_LoaderDone_BT(M4ContextLoader *load);
M4Err M4SM_LoaderRun_BT(M4ContextLoader *load);

M4Err M4SM_LoaderInit_XMT(M4ContextLoader *load);
void M4SM_LoaderDone_XMT(M4ContextLoader *load);
M4Err M4SM_LoaderRun_XMT(M4ContextLoader *load);

M4Err M4SM_LoaderInit_MP4(M4ContextLoader *load);
void M4SM_LoaderDone_MP4(M4ContextLoader *load);
M4Err M4SM_LoaderRun_MP4(M4ContextLoader *load);

M4Err M4SM_LoaderInit_SWF(M4ContextLoader *load);
void M4SM_LoaderDone_SWF(M4ContextLoader *load);
M4Err M4SM_LoaderRun_SWF(M4ContextLoader *load);

#endif

