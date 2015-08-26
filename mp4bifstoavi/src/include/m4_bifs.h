/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / BIFS codec sub-project
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

#ifndef __M4_BIFS__H
#define __M4_BIFS__H

#ifdef __cplusplus
extern "C" {
#endif


#include <m4_mpeg4_nodes.h>
/*for BIFS config*/
#include <m4_descriptors.h>

typedef struct _tagBDecoder *LPBIFSDEC;

/*BIFS decoder constructor - 
 @command_dec: if set, the decoder will only work in memory mode (creating commands for the graph)
 otherwise the decoder will always apply commands while decoding them*/
LPBIFSDEC BIFS_NewDecoder(LPSCENEGRAPH scenegraph, Bool command_dec);
void BIFS_DeleteDecoder(LPBIFSDEC codec);

/*sets the scene time. Scene time is the real clock of the bifs stream in secs*/
void BIFS_SetClock(LPBIFSDEC codec, Double (*GetSceneTime)(void *st_cbk), void *st_cbk );

/*signals the sizeInfo of the config should be ignored - used for BIFS in AnimationStream nodes*/
void BIFS_IgnoreSizeInfo(LPBIFSDEC codec);

/*setup a stream*/
M4Err BIFS_ConfigureStream(LPBIFSDEC codec, u16 ESID, char *DecoderSpecificInfo, u32 DecoderSpecificInfoLength, u32 objectTypeIndication);
/*removes a stream*/
M4Err BIFS_RemoveStream(LPBIFSDEC codec, u16 ESID);

/*decode a BIFS AU and applies it to the graph (non-memory mode only)*/
M4Err BIFS_DecodeAU(LPBIFSDEC codec, u16 ESID, char *data, u32 data_length);

/*Memory BIFS decoding - fills the command list with the content of the AU - cf m4_scenegraph.h for commands usage
	@ESID: ID of input stream
	@data, @data_length: BIFS AU
	@com_list: target list for decoded commands
*/
M4Err BIFS_DecodeAUMemory(LPBIFSDEC codec, u16 ESID, char *data, u32 data_length, Chain *com_list);

/*returns active stream - to use only in NodeInit callback (when an AU is being decoded)*/
u16 BIFS_GetActiveStream(LPBIFSDEC codec);


/*retirves uncompressed BIFS config*/
M4Err BIFS_GetConfig(LPBIFSDEC codec, u16 ESID, BIFSConfigDescriptor *cfg);


/*BIFS encoding*/
typedef struct _tagBEncoder *LPBIFSENC;
/*constructor - @graph: scene graph being encoded*/
LPBIFSENC BIFS_NewEncoder(LPSCENEGRAPH graph);
/*destructor*/
void BIFS_DeleteEncoder(LPBIFSENC codec);
/*setup a destination stream*/
M4Err BIFS_NewStream(LPBIFSENC codec, u16 ESID, BIFSConfigDescriptor *cfg, Bool encodeNames, Bool has_predictive);
/*encodes a list of commands for the given stream in the output buffer - data is dynamically allocated for output
the scenegraph used is the one described in SceneReplace command, hence scalable streams shall be encoded in time order
*/
M4Err BIFS_EncodeAU(LPBIFSENC codec, u16 ESID, Chain *command_list, char **out_data, u32 *out_data_length);
/*returns encoded config desc*/
M4Err BIFS_GetStreamConfig(LPBIFSENC codec, u16 ESID, char **out_data, u32 *out_data_length);
/*returns BIFS version used by codec for given stream*/
u8 BIFS_GetVersion(LPBIFSENC codec, u16 ESID);

/*Encodes current graph as a scene replace*/
M4Err BIFS_GetRAP(LPBIFSENC codec, char **out_data, u32 *out_data_length);

/*set trace file for BIFS encoding*/
void BE_SetTrace(LPBIFSENC codec, FILE *trace);

#ifdef __cplusplus
}
#endif



#endif 

