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

/*
		WARNING: this is a brute XMT parser, has nothing generic and only accepts 100% XMT-A files only
*/

#include <intern/m4_author_dev.h>
#include <intern/m4_bifs_dev.h>
#include <intern/m4_scenegraph_dev.h>
#include <m4_x3d_nodes.h>


typedef struct
{
	M4ContextLoader *load;
	M4Err last_error;
	XMLParser xml_parser;

	Bool is_x3d;

	/*routes are not created in the graph when parsing, so we need to track insert and delete/replace*/
	Chain *unresolved_routes, *inserted_routes;
	Chain *def_nodes;

	char *temp_att;

	/*set when parsing proto*/
	LPPROTO parsing_proto;

	/*current stream ID, AU time and RAP flag*/
	u32 stream_id;
	Double au_time;
	Bool au_is_rap;

	/*current BIFS stream & AU*/
	M4StreamContext *bifs_es;
	M4AUContext *bifs_au;
	u32 base_bifs_id;
	/*for script storage*/
	SGCommand *cur_com;

	/*current OD stream & AU*/
	M4StreamContext *od_es;
	M4AUContext *od_au;
	u32 base_od_id;
	u32 bifs_w, bifs_h;
	Bool pixelMetrics;

	/*XMT is soooo ugly for descritpors...*/
	Chain *od_links;
	Chain *esd_links;
} XMTParser;


typedef struct
{
	char *desc_name;
	u32 ID;
	/*store nodes refering to this URL*/
	Chain *nodes;
	ObjectDescriptor *od;
} ODLink;

typedef struct
{
	char *desc_name;
	u32 ESID;
	ESDescriptor *esd;
	char *OCR_Name;
	char *Depends_Name;
} ESDLink;

void xmt_parse_command(XMTParser *parser, char *name, Chain *com_list);
void xmt_resolve_routes(XMTParser *parser);


static M4Err xmt_report(XMTParser *parser, M4Err e, char *format, ...)
{
	va_list args;
	va_start(args, format);
	if (parser->load->OnMessage) {
		char szMsg[2048];
		char szMsgFull[2048];
		vsprintf(szMsg, format, args);
		sprintf(szMsgFull, "(line %d) %s", parser->xml_parser.line, szMsg);
		parser->load->OnMessage(parser->load->cbk, szMsgFull, e);
	} else {
		fprintf(stdout, "(line %d) ", parser->xml_parser.line);
		vfprintf(stdout, format, args);
		fprintf(stdout, "\n");
	}
	va_end(args);
	if (e) parser->last_error = e;
	return e;
}



void xmt_new_od_link(XMTParser *parser, ObjectDescriptor *od, char *name)
{
	u32 i, ID, j;
	ODLink *odl;

	ID = 0;
	if (!strnicmp(name, "od", 2)) ID = atoi(name + 2);
	else if (!strnicmp(name, "iod", 3)) ID = atoi(name+ 3);
	/*be carefull, an url like "11-regression-test.mp4" will return 1 on sscanf :)*/
	else if (sscanf(name, "%d", &ID) == 1) {
		char szURL[20];
		sprintf(szURL, "%d", ID);
		if (strcmp(szURL, name)) {
			ID = 0;
		} else {
			name = NULL;
		}
	}
	else ID = 0;

	for (i=0; i<ChainGetCount(parser->od_links); i++) {
		odl = ChainGetEntry(parser->od_links, i);
		if ( (ID && (odl->ID == ID))
			|| (odl->od == od)
			|| (odl->desc_name && name && !strcmp(odl->desc_name, name))
		) {
			if (!odl->od) odl->od = od;
			if (!odl->desc_name && name) odl->desc_name = strdup(name);
			if (!od->objectDescriptorID) {
				od->objectDescriptorID = ID;
			} else if (ID && (od->objectDescriptorID != ID)) {
				xmt_report(parser, M4BadParam, "Conflicting OD IDs %d vs %d\n", ID, od->objectDescriptorID);
			}

			for (j=i+1; j<ChainGetCount(parser->od_links); j++) {
				ODLink *l2 = ChainGetEntry(parser->od_links, j);
				if (l2->od == od) {
					odl->ID = od->objectDescriptorID = odl->od->objectDescriptorID;
					ChainDeleteEntry(parser->od_links, j);
					if (l2->desc_name) free(l2->desc_name);
					free(l2);
					break;
				}
			}
			return;
		}
	}
	odl = malloc(sizeof(ODLink));
	memset(odl, 0, sizeof(ODLink));
	odl->nodes = NewChain();
	odl->od = od;
	if (ID) od->objectDescriptorID = ID;
	if (name) odl->desc_name = strdup(name);
	ChainAddEntry(parser->od_links, odl);
}
void xmt_new_od_link_from_node(XMTParser *parser, char *name, SFNode *in_node)
{
	u32 i, ID;
	ODLink *odl;

	ID = 0;
	if (!strnicmp(name, "od", 2)) ID = atoi(name + 2);
	else if (!strnicmp(name, "iod", 3)) ID = atoi(name + 3);
	/*be carefull, an url like "11-regression-test.mp4" will return 1 on sscanf :)*/
	else if (sscanf(name, "%d", &ID) == 1) {
		char szURL[20];
		sprintf(szURL, "%d", ID);
		if (strcmp(szURL, name)) {
			ID = 0;
		} else {
			name = NULL;
		}
	}
	else ID = 0;
	
	for (i=0; i<ChainGetCount(parser->od_links); i++) {
		odl = ChainGetEntry(parser->od_links, i);
		if ( (name && odl->desc_name && !strcmp(odl->desc_name, name))
			|| (ID && odl->od && odl->od->objectDescriptorID==ID)
			|| (ID && (odl->ID==ID))
			) {
			if (in_node && (ChainFindEntry(odl->nodes, in_node)<0) ) ChainAddEntry(odl->nodes, in_node);
			return;
		}
	}
	odl = malloc(sizeof(ODLink));
	memset(odl, 0, sizeof(ODLink));
	odl->nodes = NewChain();
	if (in_node) ChainAddEntry(odl->nodes, in_node);
	if (ID) odl->ID = ID;
	else odl->desc_name = strdup(name);
	ChainAddEntry(parser->od_links, odl);
}

void xmt_new_esd_link(XMTParser *parser, ESDescriptor *esd, char *desc_name, u32 ID)
{
	u32 i, j;
	ESDLink *esdl;

	for (i=0; i<ChainGetCount(parser->esd_links); i++) {
		esdl = ChainGetEntry(parser->esd_links, i);
		if (esdl->esd  && (esd!=esdl->esd)) continue;
		if (!esdl->esd) {
			if (!esdl->ESID || !desc_name || strcmp(esdl->desc_name, desc_name)) continue;
			esdl->esd = esd;
		}
		if (ID) {
			/*remove temp links*/
			if (esdl->ESID == (u16) ( ( ((u32) esdl) >> 16) | ( ((u32)esdl) & 0x0000FFFF) ) ) {
				for (j=0; j<ChainGetCount(parser->load->ctx->streams); j++) {
					M4StreamContext *sc = ChainGetEntry(parser->load->ctx->streams, j);
					if (sc->ESID!=esdl->ESID) continue;
					/*reassign*/
					sc->ESID = ID;
					break;
				}
			}
			esdl->ESID = esdl->esd->ESID = ID;
		}
		if (desc_name && !esdl->desc_name) {
			esdl->desc_name = strdup(desc_name);
			if (!esdl->ESID && !strnicmp(desc_name, "es", 2)) esdl->ESID = atoi(&desc_name[2]);
		}
		return;
	}
	esdl = malloc(sizeof(ESDLink));
	memset(esdl, 0, sizeof(ESDLink));
	esdl->esd = esd;
	if (ID) esd->ESID = esdl->ESID = ID;
	if (desc_name) {
		if (!esdl->ESID && !strnicmp(desc_name, "es", 2)) esdl->ESID = atoi(&desc_name[2]);
		esdl->desc_name = strdup(desc_name);
	}
	ChainAddEntry(parser->esd_links, esdl);
}

u16 xmt_locate_stream(XMTParser *parser, char *stream_name)
{
	ESDLink *esdl;
	u32 i;
	char szN[200];

	for (i=0; i<ChainGetCount(parser->esd_links); i++) {
		esdl = ChainGetEntry(parser->esd_links, i);
		if (esdl->desc_name && !strcmp(esdl->desc_name, stream_name)) return esdl->ESID;
		if (esdl->ESID) {
			sprintf(szN, "es%d", esdl->ESID);
			if (!strcmp(szN, stream_name)) return esdl->ESID;
			sprintf(szN, "%d", esdl->ESID);
			if (!strcmp(szN, stream_name)) return esdl->ESID;
		}
	}
	/*create a temp one*/
	esdl = malloc(sizeof(ESDLink));
	memset(esdl, 0, sizeof(ESDLink));
	esdl->ESID = (u16) ( ((u32) esdl) >> 16) | ( ((u32)esdl) & 0x0000FFFF);
	if (!strnicmp(stream_name, "es", 2)) esdl->ESID = atoi(&stream_name[2]);
	esdl->desc_name = strdup(stream_name);
	ChainAddEntry(parser->esd_links, esdl);
	return esdl->ESID;
}

Bool xmt_set_dep_id(XMTParser *parser, ESDescriptor *desc, char *es_name, Bool is_ocr_dep)
{
	u32 i;
	if (!desc || !es_name) return 0;

	for (i=0; i<ChainGetCount(parser->esd_links); i++) {
		ESDLink *esdl = ChainGetEntry(parser->esd_links, i);
		if (esdl->esd == desc) {
			if (is_ocr_dep)
				esdl->OCR_Name = strdup(es_name);
			else
				esdl->Depends_Name = strdup(es_name);
			return 1;
		}
	}
	return 0;
}


u32 xmt_get_next_node_id(XMTParser *parser)
{
	u32 ID;
	LPSCENEGRAPH sc = parser->load->scene_graph;
	if (parser->parsing_proto) sc = Proto_GetSceneGraph(parser->parsing_proto);
	ID = SG_GetNextAvailableNodeID(sc);
	if (parser->load->ctx && (ID>parser->load->ctx->max_node_id)) 
		parser->load->ctx->max_node_id = ID;
	return ID;
}
u32 xmt_get_next_route_id(XMTParser *parser)
{
	u32 ID;
	LPSCENEGRAPH sc = parser->load->scene_graph;
	if (parser->parsing_proto) sc = Proto_GetSceneGraph(parser->parsing_proto);

	ID = SG_GetNextAvailableRouteID(sc);
	if (parser->load->ctx && (ID>parser->load->ctx->max_route_id)) 
		parser->load->ctx->max_route_id = ID;
	return ID;
}
u32 xmt_get_next_proto_id(XMTParser *parser)
{
	u32 ID;
	LPSCENEGRAPH sc = parser->load->scene_graph;
	if (parser->parsing_proto) sc = Proto_GetSceneGraph(parser->parsing_proto);
	ID = SG_GetNextAvailableProtoID(sc);
	if (parser->load->ctx && (ID>parser->load->ctx->max_node_id)) 
		parser->load->ctx->max_proto_id = ID;
	return ID;
}

u32 xmt_get_node_id(XMTParser *parser)
{
	SFNode *n;
	u32 ID;
	if (sscanf(parser->xml_parser.value_buffer, "N%d", &ID) == 1) {
		ID ++;
		n = SG_FindNode(parser->load->scene_graph, ID);
		if (n) {
			u32 nID = xmt_get_next_node_id(parser);
			xmt_report(parser, M4OK, "WARNING: changing node \"%s\" ID from %d to %d", n->sgprivate->NodeName, n->sgprivate->NodeID-1, nID-1);
			Node_SetDEF(n, nID, n->sgprivate->NodeName);
		}
		if (parser->load->ctx && (parser->load->ctx->max_node_id<ID)) parser->load->ctx->max_node_id=ID;
	} else {
		ID = xmt_get_next_node_id(parser);
	}
	return ID;
}

static void xmt_offset_time(XMTParser *parser, Double *time)
{
	*time += parser->au_time;
}

static void xmt_check_time_offset(XMTParser *parser, SFNode *n, FieldInfo *info)
{
	if (!(parser->load->flags & M4CL_FOR_PLAYBACK)) return;
	if (Node_GetTag(n) != TAG_ProtoNode) {
		if (!stricmp(info->name, "startTime") || !stricmp(info->name, "stopTime")) 
			xmt_offset_time(parser, (Double *)info->far_ptr);
	} else if (Proto_FieldIsSFTimeOffset(n, info)) {
		xmt_offset_time(parser, (Double *)info->far_ptr);
	}
}
static void xmt_update_timenode(XMTParser *parser, SFNode *node)
{
	if (!(parser->load->flags & M4CL_FOR_PLAYBACK)) return;

	switch (Node_GetTag(node)) {
	case TAG_MPEG4_AnimationStream:
		xmt_offset_time(parser, & ((M_AnimationStream*)node)->startTime);
		xmt_offset_time(parser, & ((M_AnimationStream*)node)->stopTime);
		break;
	case TAG_MPEG4_AudioBuffer:
		xmt_offset_time(parser, & ((M_AudioBuffer*)node)->startTime);
		xmt_offset_time(parser, & ((M_AudioBuffer*)node)->stopTime);
		break;
	case TAG_MPEG4_AudioClip:
		xmt_offset_time(parser, & ((M_AudioClip*)node)->startTime);
		xmt_offset_time(parser, & ((M_AudioClip*)node)->stopTime);
		break;
	case TAG_MPEG4_AudioSource:
		xmt_offset_time(parser, & ((M_AudioSource*)node)->startTime);
		xmt_offset_time(parser, & ((M_AudioSource*)node)->stopTime);
		break;
	case TAG_MPEG4_MovieTexture:
		xmt_offset_time(parser, & ((M_MovieTexture*)node)->startTime);
		xmt_offset_time(parser, & ((M_MovieTexture*)node)->stopTime);
		break;
	case TAG_MPEG4_TimeSensor:
		xmt_offset_time(parser, & ((M_TimeSensor*)node)->startTime);
		xmt_offset_time(parser, & ((M_TimeSensor*)node)->stopTime);
		break;
	case TAG_ProtoNode:
	{
		u32 i, nbFields;
		FieldInfo inf;
		nbFields = Node_GetNumFields(node, FCM_ALL);
		for (i=0; i<nbFields; i++) {
			Node_GetField(node, i, &inf);
			if (inf.fieldType != FT_SFTime) continue;
			xmt_check_time_offset(parser, node, &inf);
		}
	}
		break;
	}
}


#define XMT_GET_VAL \
	char value[100];	\
	u32 i;			\
	char *str = parser->temp_att;	\
	if (!str) {		\
		xmt_report(parser, M4BadParam, "%s: Number expected", name);	\
		return;		\
	}		\
	while (str[0] == ' ') str += 1;	\
	i = 0;	\
	while ((str[i] != ' ') && str[i]) {	\
		value[i] = str[i];			\
		i++;				\
	}					\
	value[i] = 0;	\
	while ((str[i] == ' ') && str[i]) i++;	\
	if (!str[i]) parser->temp_att = NULL;	\
	else parser->temp_att = &str[i];	

void xmt_parse_int(XMTParser *parser, const char *name, SFInt32 *val)
{
	XMT_GET_VAL
	*val = atoi(value);
}
void xmt_parse_float(XMTParser *parser, const char *name, SFFloat *val)
{
	XMT_GET_VAL
	*val = (Float) atof(value);
}
void xmt_parse_time(XMTParser *parser, const char *name, SFTime *val)
{
	XMT_GET_VAL
	*val = atof(value);
}
void xmt_parse_bool(XMTParser *parser, const char *name, SFBool *val)
{
	XMT_GET_VAL
	if (!stricmp(value, "1") || !stricmp(value, "true"))
		*val = 1;
	else
		*val = 0;
}

void xmt_parse_string(XMTParser *parser, const char *name, SFString *val)
{
	char value[5000];
	u32 i=0;
	u32 k=0;
	char *str = parser->temp_att;
	if (!str) {
		xmt_report(parser, M4BadParam, "%s: String expected", name);
		return;
	}
	if (strchr(str, '\'')) {
		while (str[0] != '\'')  str += 1;
		str+=1;
		while (str[i] && (str[i] != '\'')) {
			/*handle UTF-8 - WARNING: if parser is in unicode string is already utf8 multibyte chars*/
			if (!parser->xml_parser.unicode_type && (str[i] & 0x80)) {
				value[k] = 0xc0 | ( (str[i] >> 6) & 0x3 );
				k++;
				str[i] &= 0xbf;
			}
			value[k] = str[i];
			i++;
			k++;
		}
		value[k] = 0;
		if (!str[i+1]) parser->temp_att = NULL;
		else parser->temp_att = &str[i+1];	
	} else {
		strcpy(value, str);
		parser->temp_att = NULL;
	}
	if (val->buffer) free(val->buffer);
	val->buffer = NULL;
	if (strlen(value)) val->buffer = xml_translate_xml_string(value);
}

void xmt_parse_url(XMTParser *parser, const char *name, SFURL *val, SFNode *owner)
{
	char value[5000], *tmp;
	u32 i=0;
	char *str = parser->temp_att;
	if (!str) {
		xmt_report(parser, M4BadParam, "%s: String expected", name);
		return;
	}
	if (strchr(str, '\'')) {
		while (str[0] != '\'')  str += 1;
		str+=1;
		while ((str[i] != '\'') && str[i]) {
			value[i] = str[i];
			i++;
		}
		if (!str[i+1]) parser->temp_att = NULL;
		else parser->temp_att = &str[i+1];	
		value[i] = 0;
	} else {
		strcpy(value, str);
		parser->temp_att = NULL;
	}
	if (val->url) free(val->url);
	val->url = xml_translate_xml_string(value);
	val->OD_ID = 0;
	tmp = strstr(value, "#");
	if (tmp) tmp[0] = 0;
	if (!strnicmp(value, "od:", 3))
		xmt_new_od_link_from_node(parser, value+3, owner);
	else
		xmt_new_od_link_from_node(parser, value, owner);
}


void xmt_parse_script(XMTParser *parser, const char *name, SFScript *val)
{
	char *value;
	char *str = parser->temp_att;
	if (!str) {
		xmt_report(parser, M4BadParam, "%s: String expected", name);
		return;
	}
	while ((str[0] == ' ') || (str[0] == '\t') ) str += 1;

	value = xml_translate_xml_string(str);
	if (val->script_text) free(val->script_text);
	val->script_text = value;
	parser->temp_att = NULL;
}

void xmt_sffield(XMTParser *parser, FieldInfo *info, SFNode *n)
{
	switch (info->fieldType) {
	case FT_SFInt32:
		xmt_parse_int(parser, info->name, (SFInt32 *)info->far_ptr); 
		break;
	case FT_SFBool:
		xmt_parse_bool(parser, info->name, (SFBool *)info->far_ptr); 
		break;
	case FT_SFFloat:
		xmt_parse_float(parser, info->name, (SFFloat *)info->far_ptr); 
		break;
	case FT_SFTime:
		xmt_parse_time(parser, info->name, (SFTime *)info->far_ptr); 
		xmt_check_time_offset(parser, n, info);
		break;
	case FT_SFColor:
		xmt_parse_float(parser, info->name, & ((SFColor *)info->far_ptr)->red); 
		xmt_parse_float(parser, info->name, & ((SFColor *)info->far_ptr)->green); 
		xmt_parse_float(parser, info->name, & ((SFColor *)info->far_ptr)->blue); 
		break;
	case FT_SFVec2f:
		xmt_parse_float(parser, info->name, & ((SFVec2f *)info->far_ptr)->x); 
		xmt_parse_float(parser, info->name, & ((SFVec2f *)info->far_ptr)->y); 
		break;
	case FT_SFVec3f:
		xmt_parse_float(parser, info->name, & ((SFVec3f *)info->far_ptr)->x); 
		xmt_parse_float(parser, info->name, & ((SFVec3f *)info->far_ptr)->y); 
		xmt_parse_float(parser, info->name, & ((SFVec3f *)info->far_ptr)->z); 
		break;
	case FT_SFRotation:
		xmt_parse_float(parser, info->name, & ((SFRotation *)info->far_ptr)->xAxis); 
		xmt_parse_float(parser, info->name, & ((SFRotation *)info->far_ptr)->yAxis); 
		xmt_parse_float(parser, info->name, & ((SFRotation *)info->far_ptr)->zAxis); 
		xmt_parse_float(parser, info->name, & ((SFRotation *)info->far_ptr)->angle); 
		break;
	case FT_SFString:
		xmt_parse_string(parser, info->name, (SFString*)info->far_ptr); 
		break;
	case FT_SFURL:
		xmt_parse_url(parser, info->name, (SFURL*)info->far_ptr, n); 
		break;
	case FT_SFCommandBuffer:
	{
		SGCommand *prev_com = parser->cur_com;
		SFCommandBuffer *cb = (SFCommandBuffer *)info->far_ptr;
		xml_skip_attributes(&parser->xml_parser);
		while (!xml_element_done(&parser->xml_parser, "buffer") && !parser->last_error) {
			xmt_parse_command(parser, NULL, cb->commandList);
		}
		parser->cur_com = prev_com;
	}
		break;
	case FT_SFScript:
		xmt_parse_script(parser, info->name, (SFScript *)info->far_ptr); 
		break;

	case FT_SFImage:
	{
		u32 k, size, v;
		SFImage *img = (SFImage *)info->far_ptr;
		xmt_parse_int(parser, "width", &img->width);
		if (parser->last_error) return;
		xmt_parse_int(parser, "height", &img->height);
		if (parser->last_error) return;
		xmt_parse_int(parser, "nbComp", &v);
		if (parser->last_error) return;
		img->numComponents = v;
		size = img->width * img->height * img->numComponents;
		if (img->pixels) free(img->pixels);
		img->pixels = malloc(sizeof(char) * size);
		for (k=0; k<size; k++) {
			char *name = "pixels";
			XMT_GET_VAL
			if (strstr(value, "0x")) sscanf(value, "%x", &v);
			else sscanf(value, "%d", &v);
			switch (img->numComponents) {
			case 1:
				img->pixels[k] = (char) v;
				break;
			case 2:
				img->pixels[k] = (char) (v>>8)&0xFF;
				img->pixels[k+1] = (char) (v)&0xFF;
				k++;
				break;
			case 3:
				img->pixels[k] = (char) (v>>16)&0xFF;
				img->pixels[k+1] = (char) (v>>8)&0xFF;
				img->pixels[k+2] = (char) (v)&0xFF;
				k+=2;
				break;
			case 4:
				img->pixels[k] = (char) (v>>24)&0xFF;
				img->pixels[k+1] = (char) (v>>16)&0xFF;
				img->pixels[k+2] = (char) (v>>8)&0xFF;
				img->pixels[k+3] = (char) (v)&0xFF;
				k+=3;
				break;
			}
		}
	}
		break;

	default:
		parser->last_error = M4NotSupported;
		break;

	}
}

void xmt_mffield(XMTParser *parser, FieldInfo *info, SFNode *n)
{
	FieldInfo sfInfo;

	sfInfo.fieldType = VRML_GetSFType(info->fieldType);
	sfInfo.name = info->name;
	VRML_MF_Reset(info->far_ptr, info->fieldType);

	if (!strlen(parser->xml_parser.value_buffer)) return;

	parser->temp_att = parser->xml_parser.value_buffer;
	while (parser->temp_att && !parser->last_error) {
		VRML_MF_Append(info->far_ptr, info->fieldType, &sfInfo.far_ptr);
		xmt_sffield(parser, &sfInfo, n);
	}

	/*seen so many times, even in x3d conformance tests: MFURL/Scripts are written as a single string with 
	XML escapes, which is forbidden...*/
	if (info->fieldType==FT_MFURL) {
		MFURL *url = (MFURL *)info->far_ptr;
		if ((url->count==1) && url->vals[0].url && ((url->vals[0].url[0]=='\"') || (url->vals[0].url[0]=='\'')) ) {
			char szURL[5000], *the_url, sep;

			sep = url->vals[0].url[0];
			strcpy(szURL, url->vals[0].url);
			VRML_MF_Reset(info->far_ptr, FT_MFURL);
			the_url = szURL;
			while (1) {
				char strU[5000];
				SFURL *u;
				char *str = strchr(the_url, sep);
				if (!str) break;
				str += 1;
				if (!str[0]) break;
				VRML_MF_Append(info->far_ptr, FT_MFURL, (void **) &u);
				strcpy(strU, str);
				str = strchr(strU, sep);
				if (str) str[0] = 0;
				u->url = strdup(strU);
				the_url = strchr(the_url + 1, sep) + 1;
			}
		}
	}
	else if (info->fieldType==FT_MFScript) {
		MFScript *sc = (MFScript *)info->far_ptr;
		if ((sc->count==1) && sc->vals[0].script_text && ((sc->vals[0].script_text[0]=='\"') || (sc->vals[0].script_text[0]=='\'')) ) {
			char szURL[5000], *the_url, sep;

			sep = sc->vals[0].script_text[0];
			strcpy(szURL, sc->vals[0].script_text);
			VRML_MF_Reset(info->far_ptr, FT_MFScript);
			the_url = szURL;
			while (1) {
				char strU[5000];
				SFScript *s;
				char *str = strchr(the_url, sep);
				if (!str) break;
				str += 1;
				while (str && str[0]==' ') str++;
				if (!str[0]) break;
				VRML_MF_Append(info->far_ptr, FT_MFScript, (void **) &s);
				strcpy(strU, str);
				str = strchr(strU, sep);
				if (str) str[0] = 0;
				s->script_text = strdup(strU);
				the_url = strchr(the_url + 1, sep) + 1;
			}
		}
	}
}


Bool XMTCheckNDT(XMTParser *parser, FieldInfo *info, SFNode *node, SFNode *parent)
{
	if (parent->sgprivate->tag == TAG_MPEG4_Script) return 1;
	if (parent->sgprivate->tag == TAG_X3D_Script) return 1;
	/*this handles undefined nodes*/
	if (Node_IsInTable(node, info->NDTtype)) return 1;
	/*not found*/
	xmt_report(parser, M4InvalidNode, "Node %s not valid in field %s\n", Node_GetName(node), info->name);
	Node_Unregister(node, parent);
	return 0;
}

u32 GetXMTFieldTypeByName(const char *name)
{
	if (!strcmp(name, "Boolean") || !strcmp(name, "SFBool")) return FT_SFBool;
	else if (!strcmp(name, "Integer") || !strcmp(name, "SFInt32")) return FT_SFInt32;
	else if (!strcmp(name, "Color") || !strcmp(name, "SFColor")) return FT_SFColor;
	else if (!strcmp(name, "Vector2") || !strcmp(name, "SFVec2f")) return FT_SFVec2f;
	else if (!strcmp(name, "Image") || !strcmp(name, "SFImage")) return FT_SFImage;
	else if (!strcmp(name, "Time") || !strcmp(name, "SFTime")) return FT_SFTime;
	else if (!strcmp(name, "Float") || !strcmp(name, "SFFloat")) return FT_SFFloat;
	else if (!strcmp(name, "Vector3") || !strcmp(name, "SFVec3f")) return FT_SFVec3f;
	else if (!strcmp(name, "Rotation") || !strcmp(name, "SFRotation")) return FT_SFRotation;
	else if (!strcmp(name, "String") || !strcmp(name, "SFString")) return FT_SFString;
	else if (!strcmp(name, "Node") || !strcmp(name, "SFNode")) return FT_SFNode;
	else if (!strcmp(name, "Booleans") || !strcmp(name, "MFBool")) return FT_MFBool;
	else if (!strcmp(name, "Integers") || !strcmp(name, "MFInt32")) return FT_MFInt32;
	else if (!strcmp(name, "Colors") || !strcmp(name, "MFColor")) return FT_MFColor;
	else if (!strcmp(name, "Vector2s") || !strcmp(name, "MFVec2f")) return FT_MFVec2f;
	else if (!strcmp(name, "Images") || !strcmp(name, "MFImage")) return FT_MFImage;
	else if (!strcmp(name, "Times") || !strcmp(name, "MFTime")) return FT_MFTime;
	else if (!strcmp(name, "Floats") || !strcmp(name, "MFFloat")) return FT_MFFloat;
	else if (!strcmp(name, "Vector3s") || !strcmp(name, "MFVec3f")) return FT_MFVec3f;
	else if (!strcmp(name, "Rotations") || !strcmp(name, "MFRotation")) return FT_MFRotation;
	else if (!strcmp(name, "Strings") || !strcmp(name, "MFString")) return FT_MFString;
	else if (!strcmp(name, "Nodes") || !strcmp(name, "MFNode")) return FT_MFNode;

	else if (!strcmp(name, "SFColorRGBA")) return FT_SFColorRGBA;
	else if (!strcmp(name, "MFColorRGBA")) return FT_MFColorRGBA;
	else if (!strcmp(name, "SFDouble")) return FT_SFDouble;
	else if (!strcmp(name, "MFDouble")) return FT_MFDouble;
	else if (!strcmp(name, "SFVec3d")) return FT_SFVec3d;
	else if (!strcmp(name, "MFVec3d")) return FT_MFVec3d;
	else if (!strcmp(name, "SFVec2d")) return FT_SFVec2d;
	else if (!strcmp(name, "MFVec2d")) return FT_MFVec2d;
	else return FT_Unknown;
}
u32 GetXMTScriptEventTypeByName(const char *name)
{
	if (!strcmp(name, "eventIn") || !strcmp(name, "inputOnly") ) return SFET_EventIn;
	else if (!strcmp(name, "eventOut") || !strcmp(name, "outputOnly")) return SFET_EventOut;
	else if (!strcmp(name, "field") || !strcmp(name, "initializeOnly") ) return SFET_Field;
	else return ET_Unknown;
}
u32 GetXMTEventTypeByName(const char *name)
{
	if (!strcmp(name, "eventIn") || !strcmp(name, "inputOnly") ) return ET_EventIn;
	else if (!strcmp(name, "eventOut") || !strcmp(name, "outputOnly")) return ET_EventOut;
	else if (!strcmp(name, "field") || !strcmp(name, "initializeOnly") ) return ET_Field;
	else if (!strcmp(name, "exposedField") || !strcmp(name, "inputOutput")) return ET_ExposedField;
	else return ET_Unknown;
}

SFNode *xmt_parse_node(XMTParser *parser, char *node_name, SFNode *parent, s32 *outFieldIndex);


void xmt_parse_field_node(XMTParser *parser, SFNode *node, FieldInfo *field)
{
	char *str;
	s32 fieldIndex;
	char szType[20];

	/*X3D doesn't use node/nodes syntax element, just a regular listing*/
	if (parser->is_x3d) {
		SFNode *n = xmt_parse_node(parser, NULL, node, &fieldIndex);
		if (n) {
			if (field->fieldType==FT_SFNode) * ((SFNode **)field->far_ptr) = n;
			else if (field->fieldType==FT_MFNode) ChainAddEntry(*(Chain **)field->far_ptr, n);
		}
		return;
	}

	str = xml_get_element(&parser->xml_parser);
	strcpy(szType, str);

	if ((field->fieldType==FT_SFNode) && strcmp(str, "node") ) {
		xmt_report(parser, M4BadParam, "Invalid SFNode field declaration: expecting \"node\" parent element", str);
		return;
	}
	else if ((field->fieldType==FT_MFNode) && strcmp(str, "nodes")) {
		xmt_report(parser, M4BadParam, "Invalid MFNode field declaration: expecting \"nodes\" parent element", str);
		return;
	}
	xml_skip_attributes(&parser->xml_parser);
	while (!xml_element_done(&parser->xml_parser, szType)) {
		SFNode *n = xmt_parse_node(parser, NULL, node, &fieldIndex);
		if (n) {
			if (field->fieldType==FT_SFNode) * ((SFNode **)field->far_ptr) = n;
			else if (field->fieldType==FT_MFNode) ChainAddEntry(*(Chain **)field->far_ptr, n);
		}
	}
}

void xmt_parse_script_field(XMTParser *parser, SFNode *node)
{
	LPSCRIPTFIELD scfield;
	FieldInfo field;
	char *str, *val;
	u32 fieldType, eventType;
	char fieldName[1024];

	fieldType = eventType = 0;
	val = NULL;
	while (xml_has_attributes(&parser->xml_parser)) {
		str = xml_get_attribute(&parser->xml_parser);
		if (!strcmp(str, "name")) strcpy(fieldName, parser->xml_parser.value_buffer); 
		else if (!strcmp(str, "type")) fieldType = GetXMTFieldTypeByName(parser->xml_parser.value_buffer); 
		else if (!strcmp(str, "vrml97Hint") || !strcmp(str, "accessTtpe")) eventType = GetXMTScriptEventTypeByName(parser->xml_parser.value_buffer); 
		else if (strstr(str, "value") || strstr(str, "Value")) val = strdup(parser->xml_parser.value_buffer);
	}
	scfield = SG_NewScriptField(node, eventType, fieldType, fieldName);

	if (!scfield) {
		xmt_report(parser, M4BadParam, "cannot create script field - check syntax");
		return;
	}
	if (val) {
		Node_GetFieldByName(node, fieldName, &field);
		str = parser->xml_parser.value_buffer;
		parser->xml_parser.value_buffer = parser->temp_att = val;
		if (VRML_IsSFField(fieldType)) {
			xmt_sffield(parser, &field, node);
		} else {
			xmt_mffield(parser, &field, node);
		}
		parser->xml_parser.value_buffer = str;
		free(val);
	}
	/*for SF/MF Nodes*/
	while (!xml_element_done(&parser->xml_parser, "field")) {
		Node_GetFieldByName(node, fieldName, &field);
		xmt_parse_field_node(parser, node, &field);
		if (parser->last_error) return;
	}
}

void xmt_parse_ised(XMTParser *parser, SFNode *node)
{
	M4Err e;
	char *str;
	LPPROTOFIELD pf;
	FieldInfo pfield, nfield;
	char szNode[1024], szProto[1024];

	while (!xml_element_done(&parser->xml_parser, "IS")) {
		str = xml_get_element(&parser->xml_parser);
		if (!strcmp(str, "connect")) {
			while (xml_has_attributes(&parser->xml_parser)) {
				str = xml_get_attribute(&parser->xml_parser);
				if (!strcmp(str, "nodeField")) strcpy(szNode, parser->xml_parser.value_buffer);
				else if (!strcmp(str, "protoField")) strcpy(szProto, parser->xml_parser.value_buffer);
			}
			xml_element_done(&parser->xml_parser, "connect");
			e = Node_GetFieldByName(node, szNode, &nfield);
			if (e) {
				xmt_report(parser, M4BadParam, "%s: Unknown node field", szNode);
				return;
			}
			pf = Proto_FindFieldByName(parser->parsing_proto, szProto);
			if (!pf) {
				xmt_report(parser, M4BadParam, "%s: Unknown proto field", szProto);
				return;
			}
			ProtoField_GetField(pf, &pfield);
			e = Proto_SetISField(parser->parsing_proto, pfield.fieldIndex, node, nfield.fieldIndex);
			if (e) xmt_report(parser, M4BadParam, "Cannot set IS field (Error %s)", M4ErrToString(e));
		}
		else xml_skip_element(&parser->xml_parser, str);
	}
}


SFNode *xmt_proto_instance(XMTParser *parser, SFNode *parent)
{
	u32 ID;
	LPPROTO p;
	char szDEFName[1024], szProtoName[1024];
	Bool isUSE;
	SFNode *node;
	FieldInfo info;
	LPSCENEGRAPH sg;
	M4Err e;
	char *str;

	p = NULL;

	isUSE = 0;
	szDEFName[0] = 0;
	szProtoName[0] = 0;
	ID = 0;
	while (xml_has_attributes(&parser->xml_parser)) {
		str = xml_get_attribute(&parser->xml_parser);
		/*DEF node*/
		if (!strcmp(str, "DEF")) strcpy(szDEFName, parser->xml_parser.value_buffer);
		/*USE node*/
		else if (!strcmp(str, "USE")) {
			strcpy(szDEFName, parser->xml_parser.value_buffer);
			isUSE = 1;
		}
		/*proto name*/
		else if (!strcmp(str, "name")) strcpy(szProtoName, parser->xml_parser.value_buffer);
	}

	sg = parser->load->scene_graph;
	while (1) {
		p = SG_FindProto(sg, 0, szProtoName);
		if (p) break;
		sg = sg->parent_scene;
		if (!sg) break;
	}
	if (!p) {
		xmt_report(parser, M4BadParam, "%s: not a valid/supported proto", szProtoName);
		return NULL;
	}

	if (isUSE) {
		node = SG_FindNodeByName(parser->load->scene_graph, parser->xml_parser.value_buffer);
		if (!node) {
			/*create a temp node (undefined)*/
			node = Proto_CreateInstance(parser->load->scene_graph, p);
			str = parser->xml_parser.value_buffer;
			parser->xml_parser.value_buffer = szDEFName;
			ID = xmt_get_node_id(parser);
			Node_SetDEF(node, ID, str);
			parser->xml_parser.value_buffer = str;
		}
		Node_Register(node, parent);
		if (!xml_element_done(&parser->xml_parser, NULL)) {
			xmt_report(parser, M4BadParam, "Too many attributes - only USE=\"ID\" allowed");
		}
		return node;
	}

	if (strlen(szDEFName)) {
		node = SG_FindNodeByName(parser->load->scene_graph, szDEFName);
		if (node) {
			ID = node->sgprivate->NodeID;
		} else {
			ID = xmt_get_node_id(parser);
			node = Proto_CreateInstance(parser->load->scene_graph, p);
			Node_SetDEF(node, ID, szDEFName);
		}
	} else {
		node = Proto_CreateInstance(parser->load->scene_graph, p);
	}
	Node_Register(node, parent);

	while (!xml_element_done(&parser->xml_parser, "ProtoInstance")) {
		str = xml_get_element(&parser->xml_parser);
		if (!strcmp(str, "fieldValue")) {
			char szField[1024];
			char *szVal;
			szVal = NULL;
			while (xml_has_attributes(&parser->xml_parser)) {
				str = xml_get_attribute(&parser->xml_parser);
				if (!strcmp(str, "name")) strcpy(szField, parser->xml_parser.value_buffer);
				else if (strstr(str, "value") || strstr(str, "Value")) szVal = strdup(parser->xml_parser.value_buffer);
			}
			e = Node_GetFieldByName(node, szField, &info);
			if (e) {
				xmt_report(parser, M4BadParam, "%s: Unknown proto field", szField);
				goto err_exit;
			}
			if (szVal) {
				str = parser->xml_parser.value_buffer;
				parser->xml_parser.value_buffer = parser->temp_att = szVal;
				if (VRML_IsSFField(info.fieldType)) {
					xmt_sffield(parser, &info, node);
				} else {
					xmt_mffield(parser, &info, node);
				}
				parser->xml_parser.value_buffer = str;
				free(szVal);
			} else {
				/*for X3D, in XMT only 1 value possible with MFNodes*/
				while (!xml_element_done(&parser->xml_parser, "fieldValue")) {
					xmt_parse_field_node(parser, node, &info);
					if (parser->last_error) goto err_exit;
				}
			}
			xml_element_done(&parser->xml_parser, "fieldValue");
		}
		else xml_skip_element(&parser->xml_parser, str);
	}

	if (!parser->parsing_proto) Node_Init(node);
	return node;

err_exit:
	Node_Unregister(node, parent);
	return NULL;
}

Bool XMT_HasBeenDEF(XMTParser *parser, char *node_name)
{
	u32 i, count;
	count = ChainGetCount(parser->def_nodes);
	for (i=0; i<count; i++) {
		SFNode *n = ChainGetEntry(parser->def_nodes, i);
		if (!strcmp(n->sgprivate->NodeName, node_name)) return 1;
	}
	return 0;
}

u32 xmt_get_node_tag(XMTParser *parser, char *node_name) 
{
	u32 tag;

	/*if VRML and allowing non MPEG4 nodes, use X3D*/
	if (parser->is_x3d && !(parser->load->flags & M4CL_MPEG4_STRICT)) {
		tag = X3D_GetTagByName(node_name);
		if (!tag) tag = MPEG4_GetTagByName(node_name);
	} else {
		tag = MPEG4_GetTagByName(node_name);
		/*if allowing non MPEG4 nodes, try X3D*/
		if (!tag && !(parser->load->flags & M4CL_MPEG4_STRICT)) tag = X3D_GetTagByName(node_name);
	}
	return tag;
}

M4Err xmt_get_default_container(SFNode *par, SFNode *n, FieldInfo *info)
{
	u32 i, count;
	count = Node_GetFieldCount(par);
	/*get the first field/exposedField accepting this child*/
	for (i=0; i<count; i++) {
		Node_GetField(par, i, info);
		if ((info->eventType==ET_EventOut) || (info->eventType==ET_EventIn)) continue;
		if (Node_IsInTable(n, info->NDTtype)) return M4OK;
	}
	return M4BadParam;
}

SFNode *xmt_parse_node(XMTParser *parser, char *node_name, SFNode *parent, s32 *fieldIndex)
{
	u32 tag, ID;
	Bool is_script, register_def;
	SFNode *node, *undef_node, *n;
	FieldInfo info;
	char nName[100], szDEFName[1000];
	/*for X3D child node syntax*/
	char * str;

	if (node_name) {
		str = node_name;
	} else {
		str = xml_get_element(&parser->xml_parser);
	}
	ID = 0;
	if (!strcmp(str, "ProtoInstance")) return xmt_proto_instance(parser, parent);
	if (!strcmp(str, "NULL")) return NULL;
	tag = xmt_get_node_tag(parser, str);
	if (!tag) {
		xmt_report(parser, M4OK, "%s: not a valid/supported node, skipping", str);
		xml_skip_element(&parser->xml_parser, str);
		return NULL;
	}
	strcpy(nName, str);
	node = SG_NewNode(parser->load->scene_graph, tag);
//	printf("create node %s\n", nName);
	
	is_script = 0;
	if ((tag==TAG_MPEG4_Script) || (tag==TAG_X3D_Script)) is_script = 1;

	if (!node) {
		parser->last_error = M4UnknownNode;
		return NULL;
	}

	if (!parser->parsing_proto) xmt_update_timenode(parser, node);

	if (fieldIndex) (*fieldIndex) = -1;
	undef_node = NULL;
	ID = 0;
	register_def = 0;
	/*parse all attributes*/
	while (xml_has_attributes(&parser->xml_parser)) {
		str = xml_get_attribute(&parser->xml_parser);
		/*DEF node*/
		if (!strcmp(str, "DEF")) {
			register_def = 1;
			undef_node = SG_FindNodeByName(parser->load->scene_graph, parser->xml_parser.value_buffer);
			if (undef_node) {
				ID = undef_node->sgprivate->NodeID;
				/*if we see twice a DEF N1 then force creation of a new node*/
				if (XMT_HasBeenDEF(parser, parser->xml_parser.value_buffer)) {
					undef_node = NULL;
					ID = xmt_get_node_id(parser);
					xmt_report(parser, M4OK, "Warning: Node %s has been DEFed several times, IDs may get corrupted", parser->xml_parser.value_buffer);
				}
			} else {
				ID = xmt_get_node_id(parser);
			}
			strcpy(szDEFName, parser->xml_parser.value_buffer);
		}
		/*USE node*/
		else if (!strcmp(str, "USE")) {
			SFNode *def_node = SG_FindNodeByName(parser->load->scene_graph, parser->xml_parser.value_buffer);

			/*DESTROY NODE*/
			Node_Register(node, NULL);
			Node_Unregister(node, NULL);

			if (!def_node) {
				xmt_report(parser, M4BadParam, "USE: Cannot find node %s - skipping", parser->xml_parser.value_buffer);
				xml_skip_element(&parser->xml_parser, nName);
				parser->last_error = M4OK;
				return NULL;
			}
			node = def_node;
			Node_Register(node, parent);
			if (!xml_element_done(&parser->xml_parser, NULL)) {
				xmt_report(parser, M4BadParam, "Too many attributes - only USE=\"ID\" allowed");
			}
			return node;
		}
		/*X3D stuff*/
		else if (!strcmp(str, "containerField")) {
			if (fieldIndex && parent) {
				Node_GetFieldByName(parent, str, &info);
				(*fieldIndex) = info.fieldIndex;
			}
		}
		/*all other fields*/
		else {
			parser->last_error = Node_GetFieldByName(node, str, &info);
			if (parser->last_error) {
				xmt_report(parser, parser->last_error, "%s: Unknown/Invalid node field", str);
				Node_Register(node, parent);
				goto err;
			}
			if (VRML_IsSFField(info.fieldType)) {
				parser->temp_att = parser->xml_parser.value_buffer;
				xmt_sffield(parser, &info, node);
			} else {
				xmt_mffield(parser, &info, node);
			}
		}
	}
	Node_Register(node, parent);
	if (register_def) ChainAddEntry(parser->def_nodes, node);

	/*VRML: "The transformation hierarchy shall be a directed acyclic graph; results are undefined if a node 
	in the transformation hierarchy is its own ancestor"
	that's good, because the scene graph can't handle cyclic graphs (destroy will never be called).
	However we still have to register the node before parsing it, to update node registry and get correct IDs*/
	if (ID) {
		Node_SetDEF(node, ID, szDEFName);
		if (undef_node) Node_ReplaceAllInstances(undef_node, node, 0);
	}

	/*sub-nodes or complex att*/
	while (!xml_element_done(&parser->xml_parser, nName)) {
		char subName[100];

		if (parser->last_error) goto err;
		
		str = xml_get_element(&parser->xml_parser);

		if (is_script && !strcmp(str, "field")) {
			xmt_parse_script_field(parser, node);
			continue;
		}
		else if (parser->parsing_proto && !strcmp(str, "IS")) {
			xml_skip_attributes(&parser->xml_parser);
			xmt_parse_ised(parser, node);
			continue;
		}

		/*X3D has a different (and lighter) syntax here: SF/MFNode field names are never declared, 
		only the node is*/
		parser->last_error = Node_GetFieldByName(node, str, &info);
		if (parser->last_error) {
			s32 fieldIndex;
			u32 tag = xmt_get_node_tag(parser, str);
			if (!tag) {
				xml_skip_element(&parser->xml_parser, str);
				parser->last_error = M4OK;
				continue;
			}
			parser->last_error = M4OK;
			n = xmt_parse_node(parser, str, node, &fieldIndex);
			if (!n) continue;
			/*we had a container*/
			if (fieldIndex>=0) {
				parser->last_error = Node_GetField(node, (u32) fieldIndex, &info);
				if (!Node_IsInTable(n, info.NDTtype)) {
					xmt_report(parser, M4InvalidNode, "Node %s not valid in field %s\n", Node_GetName(n), info.name);
					Node_Unregister(n, node);
					goto err;
				}
			}
			/*get default container*/
			else {
				parser->last_error = xmt_get_default_container(node, n, &info);
			}
			if (parser->last_error) {
				xmt_report(parser, parser->last_error, "Cannot locate container for node %s in node %s\n", Node_GetName(n), Node_GetName(node));
				goto err;
			}

			if (info.fieldType==FT_SFNode) {
				* ((SFNode **)info.far_ptr) = n;
			} else if (info.fieldType==FT_MFNode) {
				ChainAddEntry(*(Chain **)info.far_ptr, n);
			} else {
				xmt_report(parser, parser->last_error, "Bad container (%s) for for node %s in node %s\n", info.name, Node_GetName(n), Node_GetName(node));
				goto err;
			}
			continue;
		}

		strcpy(subName, str);
		if (VRML_GetSFType(info.fieldType) != FT_SFNode) {
			if (VRML_IsSFField(info.fieldType)) {
				parser->temp_att = parser->xml_parser.value_buffer;
				xmt_sffield(parser, &info, node);
			} else {
				xmt_mffield(parser, &info, node);
			}
		} else {
			s32 fieldIndex;
			xml_skip_attributes(&parser->xml_parser);
			while (!xml_element_done(&parser->xml_parser, subName) && !parser->last_error) {
				n = xmt_parse_node(parser, NULL, node, &fieldIndex);
				if (n) {
					switch (info.fieldType) {
					case FT_SFNode:
						if (!XMTCheckNDT(parser, &info, n, node)) goto err;
						* ((SFNode **)info.far_ptr) = n;
						break;
					case FT_MFNode:
						if (!XMTCheckNDT(parser, &info, n, node)) goto err;
						ChainAddEntry(*(Chain **)info.far_ptr, n);
						break;
					}
				}
			}
		}
	}

	if (!parser->parsing_proto) {
		Node_Init(node);
		if (is_script && (parser->load->flags & M4CL_FOR_PLAYBACK) ) {
			if (parser->cur_com) {
				if (!parser->cur_com->scripts_to_load) parser->cur_com->scripts_to_load = NewChain();
				ChainAddEntry(parser->cur_com->scripts_to_load, node);
			} else {
				Script_Load(node);
			}
		}
	}
	return node;

err:
	Node_Unregister(node, parent);
	return NULL;
}


Descriptor *xmt_parse_descriptor(XMTParser *parser, char *name, Descriptor *the_desc);

void xmt_parse_descr_field(XMTParser *parser, Descriptor *desc, char *name, char *value)
{
	char field[1024];
	u32 type;
	Descriptor *subdesc;
	/*regular field*/
	if (value) {
		parser->last_error = OD_SetDescriptorField(desc, name, parser->xml_parser.value_buffer);
	} 
	/*descriptor field*/
	else {
		strcpy(field, name);
		type = OD_DescriptorFieldType(desc, name);
		switch (type) {
		/*desc list*/
		case 2:
			xml_skip_attributes(&parser->xml_parser);
			while (!xml_element_done(&parser->xml_parser, field)) {
				subdesc = xmt_parse_descriptor(parser, NULL, NULL);
				if (!subdesc) break;
				OD_AddDescToDesc(desc, subdesc);
			}
			break;
		/*desc*/
		case 1:
			if (!strcmp(field, "StreamSource")) {
				subdesc = OD_NewDescriptor(MuxInfoDescriptor_Tag);
				subdesc = xmt_parse_descriptor(parser, field, subdesc);
			} else {
				xml_skip_attributes(&parser->xml_parser);
				subdesc = xmt_parse_descriptor(parser, NULL, NULL);
			}
			if (!subdesc) break;
			OD_AddDescToDesc(desc, subdesc);
			xml_element_done(&parser->xml_parser, field);
			break;
		}
	}
	if (parser->last_error) {
		xmt_report(parser, parser->last_error, "%s: unknown field", name);
	}
}

Descriptor *xmt_parse_descriptor(XMTParser *parser, char *name, Descriptor *the_desc)
{
	char *str, desc_name[1024];
	Descriptor *desc;
	u8 tag;
	u16 ES_ID;
	Bool has_link;

	if (!the_desc) {
		if (name) {
			strcpy(desc_name,  name);
		} else {
			str = xml_get_element(&parser->xml_parser);
			if (!str) return NULL;
			strcpy(desc_name, str);
		}
		tag = OD_GetDescriptorTag(desc_name);
		if (!tag) {
			xml_skip_element(&parser->xml_parser, desc_name);
			return xmt_parse_descriptor(parser, name, NULL);
		}
		desc = OD_NewDescriptor(tag);

		if (!desc) return NULL;
	} else {
		strcpy(desc_name,  name);
		desc = the_desc;
	}

	has_link = 0;
	ES_ID = 0;
	/*parse attributes*/
	while (xml_has_attributes(&parser->xml_parser)) {
		str = xml_get_attribute(&parser->xml_parser);
		if (!strcmp(str, "objectDescriptorID")) {
			xmt_new_od_link(parser, (ObjectDescriptor *) desc, parser->xml_parser.value_buffer);
		} else if (!strcmp(str, "binaryID")) {
			if (desc->tag==ESDescriptor_Tag) {
				ES_ID = atoi(parser->xml_parser.value_buffer);
				if (!ES_ID && !strnicmp(parser->xml_parser.value_buffer, "es", 2)) 
					ES_ID = atoi(parser->xml_parser.value_buffer + 2);
				if (has_link && ES_ID) xmt_new_esd_link(parser, (ESDescriptor *) desc, NULL, ES_ID);
			} else {
				xmt_new_od_link(parser, (ObjectDescriptor *) desc, parser->xml_parser.value_buffer);
			}
		} else if (!strcmp(str, "ES_ID")) {
			has_link = 1;
			xmt_new_esd_link(parser, (ESDescriptor *) desc, parser->xml_parser.value_buffer, ES_ID);
		} else if (!strcmp(str, "OCR_ES_ID")) {
			xmt_set_dep_id(parser, (ESDescriptor *) desc, parser->xml_parser.value_buffer, 1);
		} else if (!strcmp(str, "dependsOn_ES_ID")) {
			xmt_set_dep_id(parser, (ESDescriptor *) desc, parser->xml_parser.value_buffer, 0);
		} else {
			if (!strcmp(str, "value")) {
				xmt_parse_descr_field(parser, desc, name, parser->xml_parser.value_buffer);
			} else {
				xmt_parse_descr_field(parser, desc, str, parser->xml_parser.value_buffer);
			}
		}
		if (parser->last_error) {
			OD_DeleteDescriptor(&desc);
			return NULL;
		}
	}
	/*parse sub desc - WARNING: XMT defines some properties as elements in OD and also introduces dummy containers!!!*/
	while (!xml_element_done(&parser->xml_parser, desc_name) && !parser->last_error) {
		str = xml_get_element(&parser->xml_parser);
		/*special cases in IOD*/
		if (!strcmp(str, "Profiles")) xmt_parse_descriptor(parser, "Profiles", desc);
		/*special cases in OD/IOD*/
		else if (!strcmp(str, "Descr")) {
			xml_skip_attributes(&parser->xml_parser);
			while (!xml_element_done(&parser->xml_parser, "Descr") && !parser->last_error) {
				str = xml_get_element(&parser->xml_parser);
				if (str) xmt_parse_descr_field(parser, desc, str, NULL);
			}
		}
		/*special cases in BIFS config*/
		else if (!strcmp(str, "commandStream")) {
			((BIFSConfigDescriptor *)desc)->isCommandStream = 1;
			xmt_parse_descriptor(parser, "commandStream", desc);
		}
		/*special cases OD URL*/
		else if (!strcmp(str, "URL")) xmt_parse_descriptor(parser, "URL", desc);
		else if (!strcmp(str, "size")) xmt_parse_descriptor(parser, "size", desc);
		else if (!strcmp(str, "predefined")) xmt_parse_descriptor(parser, "predefined", desc);
		else if (!strcmp(str, "custom")) xmt_parse_descriptor(parser, "custom", desc);
		else if (!strcmp(str, "MP4MuxHints")) xmt_parse_descriptor(parser, "MP4MuxHints", desc);
		/*all other desc*/
		else {
			xmt_parse_descr_field(parser, desc, str, NULL);
		}
	}
	if (desc->tag == BIFSConfig_Tag) {
		BIFSConfigDescriptor *bcfg = (BIFSConfigDescriptor *)desc;
		parser->pixelMetrics = bcfg->pixelMetrics;
		parser->load->ctx->scene_width = parser->bifs_w = bcfg->pixelWidth;
		parser->load->ctx->scene_height = parser->bifs_h = bcfg->pixelHeight;
		parser->load->ctx->is_pixel_metrics = bcfg->pixelMetrics;
		/*for xmt->bt*/
		if (!bcfg->isCommandStream) bcfg->isCommandStream = 1;
		if (!bcfg->version) bcfg->version = 1;
	}
	else if (desc->tag==ESDescriptor_Tag) {
		ESDescriptor *esd  =(ESDescriptor*)desc;
		if (esd->decoderConfig) {
			switch (esd->decoderConfig->streamType) {
			case M4ST_SCENE:
			case M4ST_OD:
			{
			/*watchout for default BIFS stream*/
				if (parser->bifs_es && !parser->base_bifs_id && (esd->decoderConfig->streamType==M4ST_SCENE)) {
					parser->bifs_es->ESID = parser->base_bifs_id = esd->ESID;
					parser->bifs_es->timeScale = esd->slConfig ? esd->slConfig->timestampResolution : 1000;
				} else {
					M4StreamContext *sc = M4SM_NewStream(parser->load->ctx, esd->ESID, esd->decoderConfig->streamType, esd->decoderConfig->objectTypeIndication);
					/*set default timescale for systems tracks (ignored for other)*/
					if (sc) sc->timeScale = (esd->slConfig && esd->slConfig->timestampResolution) ? esd->slConfig->timestampResolution : 1000;
					if (!parser->base_od_id && (esd->decoderConfig->streamType==M4ST_OD)) parser->base_od_id = esd->ESID;
				}
			}
				break;
			}
		}
	}
	return desc;
}

/*
	locate node, if not defined yet parse ahead in current AU
*/
SFNode *xmt_peek_node(XMTParser *parser, char *defID)
{
	SFNode *n;
	u32 tag, ID;
	char *str, *prev_str;
	char nName[1000], elt[1000];
	u32 pos, line, current_pos, i;
	
	n = SG_FindNodeByName(parser->load->scene_graph, defID);
	if (n) return n;

	pos = parser->xml_parser.line_start_pos;
	current_pos = parser->xml_parser.current_pos;
	line = parser->xml_parser.line;
	strcpy(nName, defID);

	/*this is ugly, assumes DEF attribute is in the same text line than element...*/
	xml_skip_attributes(&parser->xml_parser);
	n = NULL;
next_line:
	while (!parser->xml_parser.done) {
		str = parser->xml_parser.line_buffer;
		prev_str = str;
		while (str) {
			str = strstr(str, "DEF=\"");
			if (!str) {
				parser->xml_parser.current_pos = parser->xml_parser.line_size;
				xml_check_line(&parser->xml_parser);
				goto next_line;
			}
			if (!strnicmp(str+5, nName, strlen(nName)) ) break;
			str += 5;
			prev_str = str;
		}
		str = strrchr(prev_str, '<');
		if (!str || !strcmp(str+1, "ROUTE")) {
			parser->xml_parser.current_pos = parser->xml_parser.line_size;
			xml_check_line(&parser->xml_parser);
			continue;
		}
		str+=1;
		i = 0;
		while ((str[i] != ' ') && str[i]) {
			elt[i] = str[i];
			i++;
		}
		elt[i] = 0;
		if (!strcmp(elt, "ProtoInstance")) {
			str = strstr(str, "name=\"");
			if (!str) break;
			i = 0;
			str += 6;
			while ((str[i] != '\"') && str[i]) {
				elt[i] = str[i];
				i++;
			}
			elt[i] = 0;
		}
		tag = xmt_get_node_tag(parser, elt);
		if (!tag) {
			LPPROTO p;
			LPSCENEGRAPH sg = parser->load->scene_graph;
			while (1) {
				p = SG_FindProto(sg, 0, elt);
				if (p) break;
				sg = sg->parent_scene;
				if (!sg) break;
			}
			if (!p) {
				/*locate proto*/
				xmt_report(parser, M4BadParam, "%s: not a valid/supported node", elt);
				return NULL;
			}
			n = Proto_CreateInstance(parser->load->scene_graph, p);
		} else {
			n = SG_NewNode(parser->load->scene_graph, tag);
		}
		strcpy(parser->xml_parser.value_buffer, nName);
		ID = xmt_get_node_id(parser);
		Node_SetDEF(n, ID, nName);
		if (n) Node_Init(n);

		/*NO REGISTER on peek*/
		break;
	}
	/*restore context*/
	parser->xml_parser.done = 0;
	gzrewind(parser->xml_parser.gz_in);
	gzseek(parser->xml_parser.gz_in, pos, SEEK_SET);
	parser->xml_parser.current_pos = parser->xml_parser.line_size;
	xml_check_line(&parser->xml_parser);
	parser->xml_parser.line = line;
	parser->xml_parser.current_pos = current_pos;
	return n;
}


u32 xmt_get_route(XMTParser *parser, char *name, Bool del_com) 
{
	u32 i;
	LPROUTE r = SG_FindRouteByName(parser->load->scene_graph, name);
	if (r) return r->ID;
	for (i=0; i<ChainGetCount(parser->inserted_routes); i++) {
		SGCommand *com = ChainGetEntry(parser->inserted_routes, i);
		if (com->def_name && !strcmp(com->def_name, name)) {
			if (del_com) ChainDeleteEntry(parser->inserted_routes, i);
			return com->RouteID;
		}
	}
	return 0;
}

Bool xmt_route_id_used(XMTParser *parser, u32 ID)
{
	u32 i;
	LPROUTE r = SG_FindRoute(parser->load->scene_graph, ID);
	if (r) return 1;
	for (i=0; i<ChainGetCount(parser->inserted_routes); i++) {
		SGCommand *com = ChainGetEntry(parser->inserted_routes, i);
		if (com->RouteID == ID) return 1;
	}
	return 0;
}

void xmt_parse_route(XMTParser *parser, Bool is_insert, SGCommand *com)
{
	LPROUTE r;
	char *str, toN[1000], toNF[1000], fromN[1000], fromNF[1000], ID[1000];
	SFNode *orig, *dest;
	M4Err e;
	u32 rID;
	FieldInfo orig_field, dest_field;

	ID[0] = toN[0] = toNF[0] = fromN[0] = fromNF[0] = 0;

	while (xml_has_attributes(&parser->xml_parser)) {
		str = xml_get_attribute(&parser->xml_parser);
		if (!strcmp(str, "fromNode")) strcpy(fromN, parser->xml_parser.value_buffer);
		else if (!strcmp(str, "fromField")) strcpy(fromNF, parser->xml_parser.value_buffer);
		else if (!strcmp(str, "toNode")) strcpy(toN, parser->xml_parser.value_buffer);
		else if (!strcmp(str, "toField")) strcpy(toNF, parser->xml_parser.value_buffer);
		else if (!strcmp(str, "DEF")) strcpy(ID, parser->xml_parser.value_buffer);
	}
	xml_element_done(&parser->xml_parser, "ROUTE");

	orig = xmt_peek_node(parser, fromN);
	if (!orig) {
		xmt_report(parser, M4BadParam, "%s: Cannot find node", fromN);
		return;
	}
	e = Node_GetFieldByName(orig, fromNF, &orig_field);
	if ((e != M4OK) && strstr(fromNF, "_changed")) {
		char *sz = strstr(fromNF, "_changed");
		sz[0] = 0;
		e = Node_GetFieldByName(orig, fromNF, &orig_field);
	}
	if (e!=M4OK) {
		xmt_report(parser, M4BadParam, "%s: Invalid node field", fromNF);
		return;
	}
	dest = xmt_peek_node(parser, toN);
	if (!dest) {
		xmt_report(parser, M4BadParam, "%s: cannot find node", toN);
		return;
	}
	e = Node_GetFieldByName(dest, toNF, &dest_field);
	if ((e != M4OK) && !strnicmp(toNF, "set_", 4)) e = Node_GetFieldByName(dest, &toNF[4], &dest_field);
	if (e != M4OK) {
		xmt_report(parser, M4BadParam, "%s: Invalid node field", toNF);
		return;
	}
	rID = 0;
	if (strlen(ID)) {
		rID = xmt_get_route(parser, ID, 0);
		if (!rID && (ID[0]=='R') ) {
			rID = atoi(&ID[1]);
			if (rID) {
				rID++;
				if (xmt_route_id_used(parser, rID)) rID = 0;
			}
		}
		if (!rID) rID = xmt_get_next_route_id(parser);
	}
	if (com) {
		/*for insert command*/
		if (rID) {
			com->RouteID = rID;
			com->def_name = strdup(ID);
			/*whenever inserting routes, keep track of max defined ID*/
			if (is_insert) SG_SetMaxDefinedRouteID(parser->load->scene_graph, rID);
		}
		com->fromNodeID = orig->sgprivate->NodeID;
		com->fromFieldIndex = orig_field.fieldIndex;
		com->toNodeID = dest->sgprivate->NodeID;
		com->toFieldIndex = dest_field.fieldIndex;
		return;
	}
	r = SG_NewRoute(parser->load->scene_graph, orig, orig_field.fieldIndex, dest, dest_field.fieldIndex);
	if (rID) {
		SG_SetRouteID(r, rID);
		SG_SetRouteName(r, ID);
	}
}

void xmt_resolve_routes(XMTParser *parser)
{
	SGCommand *com;
	/*resolve all commands*/
	while(ChainGetCount(parser->unresolved_routes) ) {
		com = ChainGetEntry(parser->unresolved_routes, 0);
		ChainDeleteEntry(parser->unresolved_routes, 0);
		switch (com->tag) {
		case SG_RouteDelete:
		case SG_RouteReplace:
			com->RouteID = xmt_get_route(parser, com->unres_name, 0);
			if (!com->RouteID) {
				xmt_report(parser, M4BadParam, "Cannot resolve Route DEF %s", com->unres_name);
			}
			free(com->unres_name);
			com->unres_name = NULL;
			com->unresolved = 0;
			break;
		}
	}
	while (ChainGetCount(parser->inserted_routes)) ChainDeleteEntry(parser->inserted_routes, 0);
}

u32 xmt_get_od_id(XMTParser *parser, char *od_name)
{
	u32 i, ID;
	if (sscanf(od_name, "%d", &ID)==1) return ID;

	for (i=0; i<ChainGetCount(parser->od_links); i++) {
		ODLink *l = ChainGetEntry(parser->od_links, i);
		if (!l->od) continue;
		if (l->desc_name && !strcmp(l->desc_name, od_name)) return l->od->objectDescriptorID;
	}
	return 0;
}

u32 xmt_get_esd_id(XMTParser *parser, char *esd_name)
{
	u32 i, ID;
	if (sscanf(esd_name, "%d", &ID)==1) return ID;

	for (i=0; i<ChainGetCount(parser->esd_links); i++) {
		ESDLink *l = ChainGetEntry(parser->esd_links, i);
		if (!l->esd) continue;
		if (l->desc_name && !strcmp(l->desc_name, esd_name)) return l->esd->ESID;
	}
	return 0;
}

void proto_parse_field_dec(XMTParser *parser, LPPROTO proto, Bool check_code)
{
	FieldInfo info;
	LPPROTOFIELD pfield;
	char *szVal, *str;
	u32 fType, eType;
	char szFieldName[1024];

	if (check_code) {
		str = xml_get_element(&parser->xml_parser);
		if (strcmp(str, "field")) {
			xml_skip_element(&parser->xml_parser, str);
			return;
		}
	}
	szVal = NULL;
	fType = eType = 0;
	while (xml_has_attributes(&parser->xml_parser)) {
		str = xml_get_attribute(&parser->xml_parser);
		if (!strcmp(str, "name")) strcpy(szFieldName, parser->xml_parser.value_buffer);
		else if (!strcmp(str, "type")) fType = GetXMTFieldTypeByName(parser->xml_parser.value_buffer);
		else if (!strcmp(str, "vrml97Hint") || !strcmp(str, "accessType") ) eType = GetXMTEventTypeByName(parser->xml_parser.value_buffer);
		else if (strstr(str, "value") || strstr(str, "Value")) szVal = strdup(parser->xml_parser.value_buffer);
	}
	pfield = Proto_NewField(proto, fType, eType, szFieldName);
	if (szVal) {
		ProtoField_GetField(pfield, &info);
		str = parser->xml_parser.value_buffer;
		parser->temp_att = parser->xml_parser.value_buffer = szVal;
		if (VRML_IsSFField(fType)) {
			xmt_sffield(parser, &info, NULL);
		} else {
			xmt_mffield(parser, &info, NULL);
		}
		parser->xml_parser.value_buffer = str;
		free(szVal);
		xml_element_done(&parser->xml_parser, "field");
	} else if (VRML_GetSFType(fType)==FT_SFNode) {
		while (!xml_element_done(&parser->xml_parser, "field")) {
			xmt_parse_field_node(parser, NULL, &info);
		}
	} else {
		xml_element_done(&parser->xml_parser, "field");
	}
}
void xmt_parse_proto(XMTParser *parser, Chain *proto_list)
{
	FieldInfo info;
	LPPROTO proto, prevproto;
	LPSCENEGRAPH sg;
	char szName[1024];
	char *str, *extURL;
	u32 ID;

	extURL = NULL;
	while (xml_has_attributes(&parser->xml_parser)) {
		str = xml_get_attribute(&parser->xml_parser);
		if (!strcmp(str, "name")) strcpy(szName, parser->xml_parser.value_buffer);
		else if (!strcmp(str, "protoID")) ID = atoi(parser->xml_parser.value_buffer);
		else if (!strcmp(str, "locations")) extURL = strdup(parser->xml_parser.value_buffer);
	}
	ID = xmt_get_next_proto_id(parser);
	proto = SG_NewProto(parser->load->scene_graph, ID, szName, proto_list ? 1 : 0);
	if (proto_list) ChainAddEntry(proto_list, proto);
	if (parser->load->ctx && (parser->load->ctx->max_proto_id<ID)) parser->load->ctx->max_proto_id=ID;

	prevproto = parser->parsing_proto;
	sg = parser->load->scene_graph;
	parser->parsing_proto = proto;
	parser->load->scene_graph = Proto_GetSceneGraph(proto);

	/*parse all fields and proto code*/
	while (!xml_element_done(&parser->xml_parser, "ProtoDeclare")) {
		str = xml_get_element(&parser->xml_parser);
		/*proto field XMT style*/
		if (!strcmp(str, "field")) { 
			proto_parse_field_dec(parser, proto, 0);
		}
		/*proto field X3D style*/
		else if (!strcmp(str, "ProtoInterface")) { 
			xml_skip_attributes(&parser->xml_parser);
			while (!xml_element_done(&parser->xml_parser, "ProtoInterface")) {
				proto_parse_field_dec(parser, proto, 1);
			}
		}
		/*sub proto*/
		else if (!strcmp(str, "ProtoDeclare")) {
			xmt_parse_proto(parser, NULL);
		}
		/*route*/
		else if (!strcmp(str, "ROUTE")) {
			xmt_parse_route(parser, 0, NULL);
		}
		/*proto code X3D style*/
		else if (!strcmp(str, "ProtoBody")) { 
			s32 fieldIndex;
			xml_skip_attributes(&parser->xml_parser);
			while (!xml_element_done(&parser->xml_parser, "ProtoBody")) {
				SFNode *n = xmt_parse_node(parser, NULL, NULL, &fieldIndex);
				Proto_AddNodeCode(proto, n);
			}
		}
		/*proto code*/
		else {
			s32 fieldIndex;
			SFNode *n = xmt_parse_node(parser, str, NULL, &fieldIndex);
			Proto_AddNodeCode(proto, n);
		}
	}

	if (extURL) {
		str = parser->xml_parser.value_buffer;
		parser->temp_att = parser->xml_parser.value_buffer = extURL;
		memset(&info, 0, sizeof(FieldInfo));
		info.fieldType = FT_MFURL;
		info.far_ptr = &proto->ExternProto;
		info.name = "ExternURL";
		xmt_mffield(parser, &info, NULL);
		parser->xml_parser.value_buffer = str;
		free(extURL);
	}

	xmt_resolve_routes(parser);
	parser->load->scene_graph = sg;
	parser->parsing_proto = prevproto;
}


static void xmt_set_com_node(SGCommand *com, SFNode *node)
{
	com->node = node;
	Node_Register(com->node, NULL);
}

void xmt_parse_command(XMTParser *parser, char *name, Chain *com_list)
{
	char *str;
	CommandFieldInfo *field;
	SGCommand *com;
	u32 com_type;
	FieldInfo inf;
	Bool hasField;
	s32 pos;
	u32 com_pos;
	char comName[50], fName[200], fVal[2000], extType[50];
	if (name) {
		strcpy(comName, name);
	} else {
		strcpy(comName, xml_get_element(&parser->xml_parser));
	}

	/*for ReplaceScene insertion*/
	com_pos = parser->bifs_au ? ChainGetCount(parser->bifs_au->commands) : 0;

	/*BIFS commands*/
	if (!strcmp(comName, "Replace") || !strcmp(comName, "Insert") || !strcmp(comName, "Delete") 
		|| (!strcmp(comName, "Scene") && parser->is_x3d) ) {

		com = SG_NewCommand(parser->load->scene_graph, SG_UNDEFINED);
		parser->cur_com = com;

		if (!parser->stream_id) parser->stream_id = parser->base_bifs_id;

		if (!com_list) {
			if (!parser->bifs_es || (parser->bifs_es->ESID != parser->stream_id)) {
				M4StreamContext *prev = parser->bifs_es;
				parser->bifs_es = M4SM_NewStream(parser->load->ctx, (u16) parser->stream_id, M4ST_SCENE, 0);
				/*force new AU if stream changed*/
				if (parser->bifs_es != prev) parser->bifs_au = NULL;
			}
			if (!parser->bifs_au) parser->bifs_au = M4SM_NewAU(parser->bifs_es, 0, parser->au_time, parser->au_is_rap);
		}

		com_type = 0;
		if (!strcmp(comName, "Replace")) com_type = 0;
		else if (!strcmp(comName, "Insert")) com_type = 1;
		else if (!strcmp(comName, "Delete")) com_type = 2;

		/*parse attributes*/
		hasField = 0;
		/*default is END*/
		pos = -2;
		extType[0] = 0;
		while (xml_has_attributes(&parser->xml_parser)) {
			str = xml_get_attribute(&parser->xml_parser);
			if (!strcmp(str, "atNode")) {
				SFNode *atNode = xmt_peek_node(parser, parser->xml_parser.value_buffer);
				if (!atNode ) {
					xmt_report(parser, M4OK, "Cannot find node %s - skipping command", parser->xml_parser.value_buffer);
					xml_skip_element(&parser->xml_parser, comName);
					SG_DeleteCommand(com);
					parser->cur_com = NULL;
					return;
				}
				xmt_set_com_node(com, atNode);
			}
			else if (!strcmp(str, "atField")) {
				strcpy(fName, parser->xml_parser.value_buffer);
				hasField = 1;
			}
			else if (!strcmp(str, "position")) {
				if (!strcmp(parser->xml_parser.value_buffer, "BEGIN")) pos = 0;
				else if (!strcmp(parser->xml_parser.value_buffer, "END")) pos = -1;
				else pos = atoi(parser->xml_parser.value_buffer);
			}
			else if (!strcmp(str, "value")) strcpy(fVal, parser->xml_parser.value_buffer);
			else if (!strcmp(str, "atRoute")) {
				LPROUTE r = SG_FindRouteByName(parser->load->scene_graph, parser->xml_parser.value_buffer);
				if (!r) com->unres_name = strdup(parser->xml_parser.value_buffer);
				else {
					com->RouteID = r->ID;
					/*for bt<->xmt conversions*/
					com->def_name = strdup(parser->xml_parser.value_buffer);
				}
			}
			else if (!strcmp(str, "extended")) strcpy(extType, parser->xml_parser.value_buffer);
			else {
				xmt_report(parser, M4BadParam, "Unknown command element %s", str);
				goto err;
			}
		}
		
		if (strlen(extType)) {
			if (!strcmp(extType, "globalQuant")) com->tag = SG_GlobalQuantizer;
			else if (!strcmp(extType, "fields")) com->tag = SG_MultipleReplace;
			else if (!strcmp(extType, "indices")) com->tag = SG_MultipleIndexedReplace;
			else if (!strcmp(extType, "deleteOrder")) com->tag = SG_NodeDeleteEx;
			else if (!strcmp(extType, "allProtos")) com->tag = SG_ProtoDeleteAll;
			else if (!strcmp(extType, "proto") || !strcmp(extType, "protos")) {
				if (com_type == 1) {
					com->tag = SG_ProtoInsert;
					com->new_proto_list = NewChain();
				} else {
					MFInt32 *IDs = VRML_NewFieldPointer(FT_MFInt32);
					inf.fieldType = FT_MFInt32;
					inf.far_ptr = IDs;
					inf.name = "ProtoIDs";
					str = parser->xml_parser.value_buffer;
					parser->xml_parser.value_buffer = parser->temp_att = fVal;
					xmt_mffield(parser, &inf, NULL);
					parser->xml_parser.value_buffer = str;
					com->tag = SG_ProtoDelete;
					com->del_proto_list = IDs->vals;
					com->del_proto_list_size = IDs->count;
					free(IDs);
				}
			}
			else {
				xmt_report(parser, M4BadParam, "Unknown extended command %s", extType);
				goto err;
			}
		} else {
			switch (com_type) {
			case 0:
				if (com->node) {
					if (hasField && (pos > -2)) com->tag = SG_IndexedReplace;
					else if (hasField) com->tag = SG_FieldReplace;
					else com->tag = SG_NodeReplace;
				} else if (com->RouteID || (com->unres_name && strlen(com->unres_name)) ) {
					com->tag = SG_RouteReplace;
					if (!com->RouteID) {
						com->unresolved = 1;
						ChainAddEntry(parser->unresolved_routes, com);
					}
				} else {
					com->tag = SG_SceneReplace;
				}
				break;
			case 1:
				if (com->node) {
					if (hasField) com->tag = SG_IndexedInsert;
					else com->tag = SG_NodeInsert;
				} else {
					com->tag = SG_RouteInsert;
				}
				break;
			case 2:
				if (com->node) {
					if (hasField) com->tag = SG_IndexedDelete;
					else com->tag = SG_NodeDelete;
				} else {
					com->tag = SG_RouteDelete;
					if (!com->RouteID) {
						com->unresolved = 1;
						ChainAddEntry(parser->unresolved_routes, com);
					}
				}
				break;
			}
		}

		field = NULL;
		if (com->node) {
			if (Node_GetFieldByName(com->node, fName, &inf) != M4OK) {
			}
			/*simple commands*/
			if (hasField && !strlen(extType)) {
				field = SG_NewFieldCommand(com);
				field->fieldType = inf.fieldType;
				field->fieldIndex = inf.fieldIndex;
				if (VRML_GetSFType(inf.fieldType) != FT_SFNode) {
					if (pos==-2) {
						str = parser->xml_parser.value_buffer;
						parser->xml_parser.value_buffer = fVal;
		
						field->field_ptr = VRML_NewFieldPointer(inf.fieldType);
						inf.far_ptr = field->field_ptr;
						if (VRML_IsSFField(inf.fieldType)) {
							parser->temp_att = parser->xml_parser.value_buffer;
							xmt_sffield(parser, &inf, com->node);
						} else {
							xmt_mffield(parser, &inf, com->node);
						}
						parser->xml_parser.value_buffer = str;
					} else {
						field->fieldType = inf.fieldType = VRML_GetSFType(inf.fieldType);
						field->pos = pos;
						if (com->tag != SG_IndexedDelete) {
							str = parser->xml_parser.value_buffer;
							parser->xml_parser.value_buffer = fVal;

							field->field_ptr = VRML_NewFieldPointer(inf.fieldType);
							inf.far_ptr = field->field_ptr;
							parser->temp_att = parser->xml_parser.value_buffer;
							xmt_sffield(parser, &inf, com->node);
							
							parser->xml_parser.value_buffer = str;
						}
					}
				}
			}
		}

		/*parse elements*/
		while (!xml_element_done(&parser->xml_parser, comName) && !parser->last_error) {
			str = NULL;
			if (!parser->is_x3d) str = xml_get_element(&parser->xml_parser);
			/*note that we register nodes*/
			switch (com->tag) {
			case SG_SceneReplace:
				if (!parser->is_x3d && strcmp(str, "Scene")) {
					xmt_report(parser, M4BadParam, "%s Unexpected symbol in scene replace", str);
					goto err;
				}
				/*if we have a previous scene*/
				xmt_resolve_routes(parser);
				while (ChainGetCount(parser->def_nodes)) ChainDeleteEntry(parser->def_nodes, 0);

				if (parser->is_x3d) {
					assert(com->node==NULL);
					com->node = SG_NewNode(parser->load->scene_graph, (parser->load->flags & M4CL_MPEG4_STRICT) ? TAG_MPEG4_Group : TAG_X3D_Group);
					Node_Register(com->node, NULL);
					Node_Init(com->node );
				} else {
					while (xml_has_attributes(&parser->xml_parser)) {
						str = xml_get_attribute(&parser->xml_parser);
						com->use_names = 0;
						if (!strcmp(str, "USENAMES") && !strcmp(parser->xml_parser.value_buffer, "true")) com->use_names = 1;
					}
				}

				while (!xml_element_done(&parser->xml_parser, "Scene") && !parser->last_error) {
					str = xml_get_element(&parser->xml_parser);
					/*NULL*/
					if (!strcmp(str, "NULL")) {
					}
					/*proto */
					else if (!strcmp(str, "ProtoDeclare")) {
						xmt_parse_proto(parser, com->new_proto_list);
					}
					/*route*/
					else if (!strcmp(str, "ROUTE")) {
						SGCommand *sgcom = SG_NewCommand(parser->load->scene_graph, SG_RouteInsert);
						ChainAddEntry(parser->bifs_au->commands, sgcom);
						xmt_parse_route(parser, 0, sgcom);
						if (sgcom->RouteID) ChainAddEntry(parser->inserted_routes, sgcom);
					}
					/*top node*/
					else {
						if (parser->is_x3d) {
							SFNode *n = xmt_parse_node(parser, str, NULL, NULL);
							if (n) Node_InsertChild(com->node, n, -1);
						} else {
							if (com->node) {
								xmt_report(parser, M4BadParam, "More than one top node specified");
								goto err;
							}
							com->node = xmt_parse_node(parser, str, NULL, NULL);
						}
					}
				}
				if (!parser->last_error && parser->is_x3d) parser->last_error = M4EOF;
				break;
			case SG_NodeReplace:
				field = SG_NewFieldCommand(com);
				field->fieldType = FT_SFNode;
				if (!strcmp(str, "NULL")) {
					field->new_node = NULL;
				} else {
					field->new_node = xmt_parse_node(parser, str, com->node, NULL);
				}
				field->field_ptr = &field->new_node;
				break;
			case SG_IndexedReplace:
				assert(field);
				field->fieldType = FT_SFNode;
				field->pos = pos;
				if (!strcmp(str, "NULL")) {
					field->new_node = NULL;
				} else {
					field->new_node = xmt_parse_node(parser, str, com->node, NULL);
				}
				field->field_ptr = &field->new_node;
				break;
			case SG_FieldReplace:
				assert(field && (field->fieldType == FT_SFNode));
				if (!strcmp(str, "NULL")) {
					field->new_node = NULL;
				} else {
					field->new_node = xmt_parse_node(parser, str, com->node, NULL);
				}
				field->field_ptr = &field->new_node;
				break;
			case SG_NodeInsert:
				field = SG_NewFieldCommand(com);
				/*fall through*/
			case SG_IndexedInsert:
				field->fieldType = FT_SFNode;
				if (!strcmp(str, "NULL")) {
					field->new_node = NULL;
				} else {
					field->new_node = xmt_parse_node(parser, str, com->node, NULL);
				}
				field->field_ptr = &field->new_node;
				field->pos = pos;
				break;
			case SG_RouteReplace:
				xmt_parse_route(parser, 0, com);
				break;
			case SG_RouteInsert:
				xmt_parse_route(parser, 1, com);
				break;
			case SG_GlobalQuantizer:
				com->node = NULL;
				field = SG_NewFieldCommand(com);
				if (!strcmp(str, "NULL")) {
					field->new_node = NULL;
				} else {
					field->new_node = xmt_parse_node(parser, str, com->node, NULL);
				}
				field->field_ptr = &field->new_node;
				field->fieldType = FT_SFNode;
				break;
			case SG_MultipleReplace:
			{
				char szName[1024];
				char *szVal;
				if (strcmp(str, "repField")) {
					xmt_report(parser, M4BadParam, "%s: Unexpected symbol in multiple replace", str);
					goto err;
				}
				szVal = NULL;
				while (xml_has_attributes(&parser->xml_parser)) {
					str = xml_get_attribute(&parser->xml_parser);
					if (!strcmp(str, "atField")) strcpy(szName, parser->xml_parser.value_buffer);
					if (!strcmp(str, "value")) szVal = strdup(parser->xml_parser.value_buffer);
				}
				if (szVal) {
					if (Node_GetFieldByName(com->node, szName, &inf) != M4OK) {
					} else {
						field = SG_NewFieldCommand(com);
						inf.far_ptr = field->field_ptr = VRML_NewFieldPointer(inf.fieldType);
						field->fieldType = inf.fieldType;
						field->fieldIndex = inf.fieldIndex;
						str = parser->xml_parser.value_buffer;
						parser->temp_att = parser->xml_parser.value_buffer = szVal;
						if (VRML_IsSFField(inf.fieldType)) {
							xmt_sffield(parser, &inf, NULL);
						} else {
							xmt_mffield(parser, &inf, NULL);
						}
						parser->xml_parser.value_buffer = str;
						free(szVal);
					}
				}
				/*nodes*/
				while (!xml_element_done(&parser->xml_parser, "repField")) {
					str = xml_get_element(&parser->xml_parser);
					strcpy(szName, str);
					if (Node_GetFieldByName(com->node, szName, &inf) != M4OK) {
						xml_skip_element(&parser->xml_parser, str);
					} else {
						xml_skip_attributes(&parser->xml_parser);
						field = SG_NewFieldCommand(com);
						field->fieldIndex = inf.fieldIndex;
						field->fieldType = inf.fieldType;
						if (inf.fieldType == FT_SFNode) {
							field->new_node = xmt_parse_node(parser, NULL, com->node, NULL);
							field->field_ptr = &field->new_node;
							xml_element_done(&parser->xml_parser, szName);
						} else {
							field->node_list = NewChain();
							field->field_ptr = &field->node_list;
							while (!xml_element_done(&parser->xml_parser, szName)) {
								SFNode *n = xmt_parse_node(parser, NULL, com->node, NULL);
								if (n) ChainAddEntry(field->node_list, n);
							}
						}
					}

				}
			}
				break;
			case SG_MultipleIndexedReplace:
			{
				char szName[1024];
				char *szVal;
				if (strcmp(str, "repValue")) {
					xmt_report(parser, M4BadParam, "%s: Unexpected symbol in multiple replace", str);
					goto err;
				}
				szVal = NULL;
				while (xml_has_attributes(&parser->xml_parser)) {
					str = xml_get_attribute(&parser->xml_parser);
					if (!strcmp(str, "position")) {
						if (!strcmp(parser->xml_parser.value_buffer, "BEGIN")) pos = 0;
						else if (!strcmp(parser->xml_parser.value_buffer, "END")) pos = -1;
						else pos = atoi(parser->xml_parser.value_buffer);
					}
					if (!strcmp(str, "value")) szVal = strdup(parser->xml_parser.value_buffer);
				}
				if (szVal) {
					field = SG_NewFieldCommand(com);
					inf.fieldType = field->fieldType = VRML_GetSFType(inf.fieldType);
					inf.far_ptr = field->field_ptr = VRML_NewFieldPointer(field->fieldType);
					field->fieldIndex = inf.fieldIndex;
					field->pos = pos;
					str = parser->xml_parser.value_buffer;
					parser->temp_att = parser->xml_parser.value_buffer = szVal;
					xmt_sffield(parser, &inf, NULL);
					parser->xml_parser.value_buffer = str;
					free(szVal);
				}
				/*nodes*/
				while (!xml_element_done(&parser->xml_parser, "repValue")) {
					str = xml_get_element(&parser->xml_parser);
					strcpy(szName, str);
					if (Node_GetFieldByName(com->node, szName, &inf) != M4OK) {
						xml_skip_element(&parser->xml_parser, str);
					} else {
						xml_skip_attributes(&parser->xml_parser);
						field = SG_NewFieldCommand(com);
						field->pos = pos;
						field->fieldIndex = inf.fieldIndex;
						field->fieldType = FT_SFNode;
						field->new_node = xmt_parse_node(parser, NULL, com->node, NULL);
						field->field_ptr = &field->new_node;
						xml_element_done(&parser->xml_parser, szName);
					}

				}
			}
				break;
			case SG_ProtoInsert:
				if (strcmp(str, "ProtoDeclare")) {
					xmt_report(parser, M4BadParam, "%s: Unexpected symbol in ProtoInsert", str);
					goto err;
				}
				xmt_parse_proto(parser, com->new_proto_list);
				break;
			}
		}
		if (parser->last_error==M4EOF) parser->last_error = M4OK;

		if (com_list) ChainAddEntry(com_list, com);
		else {
			/*ROUTEs have been inserted*/
			if (com->tag==SG_SceneReplace) {
				ChainInsertEntry(parser->bifs_au->commands, com, com_pos);
			} else {
				ChainAddEntry(parser->bifs_au->commands, com);
			}
		}
		if (com->tag==SG_RouteInsert) ChainAddEntry(parser->inserted_routes, com);

		return;
err:
		SG_DeleteCommand(com);
		return;
	}
	/*OD commands*/
	if (!strcmp(comName, "ObjectDescriptorUpdate") || !strcmp(comName, "ObjectDescriptorRemove")
			|| !strcmp(comName, "ES_DescriptorUpdate") || !strcmp(comName, "ES_DescriptorRemove")
			|| !strcmp(comName, "IPMPDescriptorUpdate") || !strcmp(comName, "IPMPDescriptorRemove") ) {
		
		if (!parser->stream_id) parser->stream_id = parser->base_od_id;
		else if (parser->bifs_es && parser->bifs_es->ESID==parser->stream_id) parser->stream_id = parser->base_od_id;

		if (!parser->od_es) parser->od_es = M4SM_NewStream(parser->load->ctx, (u16) parser->stream_id, M4ST_OD, 0);
		if (!parser->od_au) parser->od_au = M4SM_NewAU(parser->od_es, 0, parser->au_time, parser->au_is_rap);

		if (!strcmp(comName, "ObjectDescriptorUpdate") ) {
			ObjectDescriptorUpdate *odU;
			xml_skip_attributes(&parser->xml_parser);
			while (1) {
				str = xml_get_element(&parser->xml_parser);
				if (!str) {
					xmt_report(parser, M4BadParam, "Expecting <OD> in <ObjectDescriptorUpdate>");
					return;
				}
				if (!strcmp(str, "OD")) break;
				xml_skip_element(&parser->xml_parser, str);
			}
			xml_skip_attributes(&parser->xml_parser);
			odU = (ObjectDescriptorUpdate *) OD_NewCommand(ODUpdate_Tag);
			while (!xml_element_done(&parser->xml_parser, "OD")) {
				Descriptor *od = xmt_parse_descriptor(parser, NULL, NULL);
				if (od) ChainAddEntry(odU->objectDescriptors, od);
				else if (parser->last_error) {
					OD_DeleteCommand((ODCommand **) &odU);
					return;
				}
			}
			if (!xml_element_done(&parser->xml_parser, "ObjectDescriptorUpdate")) {
				xmt_report(parser, M4BadParam, "Expecting </ObjectDescriptorUpdate>");
				return;
			}
			ChainAddEntry(parser->od_au->commands, odU);
			return;
		}
		if (!strcmp(comName, "ObjectDescriptorRemove") ) {
			ObjectDescriptorRemove *odR;
			odR = (ObjectDescriptorRemove *) OD_NewCommand(ODRemove_Tag);
			while (xml_has_attributes(&parser->xml_parser)) {
				str = xml_get_attribute(&parser->xml_parser);
				if (!strcmp(str, "objectDescriptorId")) {
					u32 i, j, start;
					char szBuf[100];
					i = j = start = 0;
					while (parser->xml_parser.value_buffer[start+i]) {
						j = 0;
						while (parser->xml_parser.value_buffer[start+i] && (parser->xml_parser.value_buffer[start+i] == ' ')) i++;
						while (parser->xml_parser.value_buffer[start+i] && (parser->xml_parser.value_buffer[start+i] != ' ')) {
							szBuf[j] = parser->xml_parser.value_buffer[start+i];
							i++;
							j++;
						}
						szBuf[j] = 0;
						start += i;
						if (parser->xml_parser.value_buffer[start] == ' ') start ++;
						i = 0;
						j = xmt_get_od_id(parser, szBuf);
						if (!j) {
							xmt_report(parser, M4BadParam, "%s: Cannot find ObjectDescriptor", szBuf);
							OD_DeleteCommand((ODCommand **) &odR);
							return;
						}
						odR->OD_ID = realloc(odR->OD_ID, sizeof(u16) * (odR->NbODs+1));
						odR->OD_ID[odR->NbODs] = j;
						odR->NbODs++;
					}
				}
			}
			xml_element_done(&parser->xml_parser, "ObjectDescriptorRemove");

			ChainAddEntry(parser->od_au->commands, odR);
			return;
		}
		if (!strcmp(comName, "ES_DescriptorRemove") ) {
			ESDescriptorRemove *esdR;
			esdR = (ESDescriptorRemove *) OD_NewCommand(ESDRemove_Tag);
			while (xml_has_attributes(&parser->xml_parser)) {
				str = xml_get_attribute(&parser->xml_parser);
				if (!strcmp(str, "objectDescriptorId")) {
					esdR->ODID = xmt_get_od_id(parser, parser->xml_parser.value_buffer);
				} else if (!strcmp(str, "ES_ID")) {
					u32 i, j, start;
					char szBuf[100];
					i = j = start = 0;
					while (parser->xml_parser.value_buffer[start+i]) {
						j = 0;
						while (parser->xml_parser.value_buffer[start+i] && (parser->xml_parser.value_buffer[start+i] == ' ')) i++;
						while (parser->xml_parser.value_buffer[start+i] && (parser->xml_parser.value_buffer[start+i] != ' ')) {
							szBuf[j] = parser->xml_parser.value_buffer[start+i];
							i++;
							j++;
						}
						szBuf[j] = 0;
						start += i;
						if (parser->xml_parser.value_buffer[start] == ' ') start ++;
						i = 0;
						j = xmt_get_esd_id(parser, szBuf);
						if (!j) {
							xmt_report(parser, M4BadParam, "%s: Cannot find ESDescriptor", szBuf);
							OD_DeleteCommand((ODCommand **) &esdR);
							return;
						}
						esdR->ES_ID = realloc(esdR->ES_ID, sizeof(u16) * (esdR->NbESDs+1));
						esdR->ES_ID[esdR->NbESDs] = j;
						esdR->NbESDs++;
					}
				}
			}
			xml_element_done(&parser->xml_parser, "ES_DescriptorRemove");
			ChainAddEntry(parser->od_au->commands, esdR);
			return;
		}
	}
	/*not found*/
	xmt_report(parser, M4OK, "Unknown command name %s - skipping", comName);
	xml_skip_element(&parser->xml_parser, comName);
}

Bool xmt_esid_available(XMTParser *parser, u16 ESID) 
{
	u32 i;
	for (i=0; i<ChainGetCount(parser->esd_links); i++) {
		ESDLink *esdl = ChainGetEntry(parser->esd_links, i);
		if (esdl->esd->ESID == ESID) return 0;
	}
	return 1;
}
Bool xmt_odid_available(XMTParser *parser, u16 ODID) 
{
	u32 i;
	for (i=0; i<ChainGetCount(parser->od_links); i++) {
		ODLink *l = ChainGetEntry(parser->od_links, i);
		if (l->ID == ODID) return 0;
		if (l->od && l->od->objectDescriptorID == ODID) return 0;
	}
	return 1;
}

void xmt_resolve_od(XMTParser *parser)
{
	u32 i, j;
	ODLink *l;
	char szURL[5000];

	/*fix ESD IDs*/
	for (i=0; i<ChainGetCount(parser->esd_links); i++) {
		ESDLink *esdl = ChainGetEntry(parser->esd_links, i);
		if (!esdl->esd) {
			xmt_report(parser, M4BadParam, "Stream %s ID %d has no associated ES descriptor\n", esdl->desc_name ? esdl->desc_name : "", esdl->ESID);
			ChainDeleteEntry(parser->esd_links, i);
			if (esdl->desc_name) free(esdl->desc_name);
			free(esdl);
			i--;
			continue;
		}
		if (esdl->ESID) esdl->esd->ESID = esdl->ESID;
		else if (!esdl->esd->ESID) {
			u16 ESID = 1;
			while (!xmt_esid_available(parser, ESID)) ESID++;
			esdl->esd->ESID = ESID;
		}
	}

	/*set OCR es ids*/
	for (i=0; i<ChainGetCount(parser->esd_links); i++) {
		Bool use_old_fmt;
		u16 ocr_id;
		char szTest[50];
		ESDLink *esdl = ChainGetEntry(parser->esd_links, i);
		esdl->esd->OCRESID = 0;
		if (!esdl->OCR_Name) continue;
		
		use_old_fmt = 0;
		ocr_id = atoi(esdl->OCR_Name);
		sprintf(szTest, "%d", ocr_id);
		if (!stricmp(szTest, esdl->OCR_Name)) use_old_fmt = 1;

		for (j=0; j<ChainGetCount(parser->esd_links); j++) {
			ESDLink *esdl2 = ChainGetEntry(parser->esd_links, j);
			if (esdl2->desc_name && !strcmp(esdl2->desc_name, esdl->OCR_Name)) {
				esdl->esd->OCRESID = esdl2->esd->ESID;
				break;
			}
			if (use_old_fmt && (esdl2->esd->ESID==ocr_id)) {
				esdl->esd->OCRESID = ocr_id;
				break;
			}
		}
		if (!esdl->esd->OCRESID) {
			xmt_report(parser, M4OK, "WARNING: Could not find clock reference %s for ES %s - forcing self-synchronization", esdl->OCR_Name, esdl->desc_name);
		}
		free(esdl->OCR_Name);
		esdl->OCR_Name = NULL;
	}

	/*set dependsOn es ids*/
	for (i=0; i<ChainGetCount(parser->esd_links); i++) {
		Bool use_old_fmt;
		u16 dep_id;
		char szTest[50];
		ESDLink *esdl = ChainGetEntry(parser->esd_links, i);
		esdl->esd->dependsOnESID = 0;
		if (!esdl->Depends_Name) continue;
		
		use_old_fmt = 0;
		dep_id = atoi(esdl->Depends_Name);
		sprintf(szTest, "%d", dep_id);
		if (!stricmp(szTest, esdl->Depends_Name)) use_old_fmt = 1;

		for (j=0; j<ChainGetCount(parser->esd_links); j++) {
			ESDLink *esdl2 = ChainGetEntry(parser->esd_links, j);
			if (esdl2->desc_name && !strcmp(esdl2->desc_name, esdl->Depends_Name)) {
				esdl->esd->dependsOnESID = esdl2->esd->ESID;
				break;
			}
			if (use_old_fmt && (esdl2->esd->ESID==dep_id)) {
				esdl->esd->dependsOnESID = dep_id;
				break;
			}
		}
		if (!esdl->esd->dependsOnESID) {
			xmt_report(parser, M4OK, "WARNING: Could not find stream dependance %s for ES %s - forcing self-synchronization", esdl->Depends_Name, esdl->desc_name);
		}
		free(esdl->Depends_Name);
		esdl->Depends_Name = NULL;
	}

	while (ChainGetCount(parser->esd_links)) {
		ESDLink *esdl = ChainGetEntry(parser->esd_links, 0);
		ChainDeleteEntry(parser->esd_links, 0);
		if (esdl->desc_name) free(esdl->desc_name);
		free(esdl);
	}

	for (i=0; i<ChainGetCount(parser->od_links); i++) {
		l = ChainGetEntry(parser->od_links, i);
		if (l->od && !l->od->objectDescriptorID) {
			u16 ODID = 1;
			while (!xmt_odid_available(parser, ODID)) ODID++;
			l->od->objectDescriptorID = ODID;
		}
		if (l->od) {
			if (!l->ID) l->ID = l->od->objectDescriptorID;
			assert(l->ID == l->od->objectDescriptorID);
		}
	}

	/*unroll dep in case some URLs reference ODs by their binary IDs not their string ones*/
	for (i=0; i<ChainGetCount(parser->od_links); i++) {
		l = ChainGetEntry(parser->od_links, i);
		/*not OD URL*/
		if (!l->ID) continue;
		for (j=i+1; j<ChainGetCount(parser->od_links); j++) {
			ODLink *l2 = ChainGetEntry(parser->od_links, j);
			/*not OD URL*/
			if (!l2->ID) continue;
			if (l->ID == l2->ID) {
				while (ChainGetCount(l2->nodes)) {
					SFNode *n = ChainGetEntry(l2->nodes, 0);
					ChainDeleteEntry(l2->nodes, 0);
					ChainAddEntry(l->nodes, n);
				}
				ChainDeleteEntry(parser->od_links, j);
				j--;
				if (l2->desc_name) free(l2->desc_name);
				DeleteChain(l2->nodes);
				free(l2);
			}
		}
	}

	while (ChainGetCount(parser->od_links) ) {
		l = ChainGetEntry(parser->od_links, 0);
		if (!l->od) {
			/*if no ID found this is not an OD URL*/
			if (l->ID) {
				if (l->desc_name) {
					xmt_report(parser, M4OK, "WARNING: OD \"%s\" (ID %d) not assigned", l->desc_name, l->ID);
				} else{
					xmt_report(parser, M4OK, "WARNING: OD ID %d not assigned", l->ID);
				}
			}
		} else {
			for (j=0; j<ChainGetCount(l->nodes); j++) {
				FieldInfo info;
				SFNode *n = ChainGetEntry(l->nodes, j);
				if (Node_GetFieldByName(n, "url", &info) == M4OK) {
					u32 k;
					MFURL *url = (MFURL *)info.far_ptr;
					for (k=0; k<url->count; k++) {
						char *seg = NULL;
						if (url->vals[k].url) seg = strstr(url->vals[k].url, "#");
						if (seg) {
							sprintf(szURL, "od:%d#%s", l->od->objectDescriptorID, seg+1);
							free(url->vals[k].url);
							url->vals[k].url = strdup(szURL);
						} else {
							if (url->vals[k].url) free(url->vals[k].url);
							url->vals[k].url = NULL;
							url->vals[k].OD_ID = l->od->objectDescriptorID;
						}
					}
				}
			}
		}

		if (l->desc_name) free(l->desc_name);
		DeleteChain(l->nodes);
		free(l);
		ChainDeleteEntry(parser->od_links, 0);
	}
}

M4Err M4SM_LoaderRun_XMT(M4ContextLoader *load)
{
	char *str;
	Bool is_resume;
	XMTParser *parser = (XMTParser *)load->loader_priv;
	if (!parser) return M4BadParam;
	if (parser->xml_parser.done) return M4OK;

	is_resume = 1;
	while (!xml_element_done(&parser->xml_parser, "Body") && !parser->last_error) {
		if (!is_resume) str = xml_get_element(&parser->xml_parser);
		is_resume = 0;
		/*ALWAYS explicit command here*/
		parser->stream_id = 0;
		while (xml_has_attributes(&parser->xml_parser)) {
			str = xml_get_attribute(&parser->xml_parser);
			if (!strcmp(str, "begin")) {
				parser->au_time = atof(parser->xml_parser.value_buffer);
			}
			else if (!strcmp(str, "atES_ID")) {
				parser->stream_id = xmt_locate_stream(parser, parser->xml_parser.value_buffer);
				if (!parser->stream_id) xmt_report(parser, M4BadParam, "Cannot find stream %s targeted by command", parser->xml_parser.value_buffer);
			}
		}
		/*reset context - note we don't resolve OD/ESD links untill everything is parsed to make sure we don't remove them*/
		if (parser->od_au && (parser->od_au->timing_sec != parser->au_time)) {
			parser->od_au = NULL;
		}

		if (parser->bifs_au && (parser->bifs_au->timing_sec != parser->au_time)) {
			parser->bifs_au = NULL;
		}
	

		/*parse all commands context*/
		while (!xml_element_done(&parser->xml_parser, "par") && !parser->last_error) {
			xmt_parse_command(parser, NULL, NULL);
		}
	}

	if (!parser->last_error && !xml_element_done(&parser->xml_parser, "XMT-A")) {
		xmt_report(parser, M4BadParam, "Expecting </XMT-A> in XMT-A document");
	}

	xmt_resolve_routes(parser);
	xmt_resolve_od(parser);
	return parser->last_error;
}


void M4SM_LoaderDone_XMT(M4ContextLoader *load)
{
	XMTParser *parser = (XMTParser *)load->loader_priv;
	if (!parser) return;
	xmt_resolve_routes(parser);
	DeleteChain(parser->unresolved_routes);
	DeleteChain(parser->inserted_routes);
	DeleteChain(parser->def_nodes);
	xmt_resolve_od(parser);
	DeleteChain(parser->od_links);
	DeleteChain(parser->esd_links);
	if (parser->xml_parser.value_buffer) free(parser->xml_parser.value_buffer);
	gzclose(parser->xml_parser.gz_in);
	free(parser);
	load->loader_priv = NULL;
}

M4Err M4SM_LoaderInit_XMT(M4ContextLoader *load)
{
	Bool is_done;
	char *str;
	M4Err e;
	XMTParser *parser;

	if (!load->ctx || !load->fileName) return M4BadParam;
	SAFEALLOC(parser, sizeof(XMTParser));

	e = xml_init_parser(&parser->xml_parser, load->fileName);
	if (e) {
		xmt_report(parser, e, "Unable to open file %s", load->fileName);
		free(parser);
		return e;
	}

	parser->load = load;

	load->loader_priv = parser;
	parser->unresolved_routes = NewChain();
	parser->inserted_routes = NewChain();
	parser->od_links = NewChain();
	parser->esd_links = NewChain();
	parser->def_nodes = NewChain();

	/*create at least one empty BIFS stream*/
	parser->bifs_es = M4SM_NewStream(load->ctx, 0, M4ST_SCENE, 0);
	parser->bifs_au = M4SM_NewAU(parser->bifs_es, 0, 0, 1);



	/*check XMT-A doc*/
	while (1) {
		str = xml_get_element(&parser->xml_parser);
		if (!str) {
			xmt_report(parser, M4BadParam, "Invalid XMT-A document");
			goto exit;
		}
		if (!strcmp(str, "XMT-A")) break;
		if (!strcmp(str, "X3D")) {
			parser->is_x3d = 1;
			break;
		}
		if (!strcmp(str, "!DOCTYPE")) 
			xml_skip_attributes(&parser->xml_parser);
		else
			xml_skip_element(&parser->xml_parser, str);
	}
	xml_skip_attributes(&parser->xml_parser);

	if (parser->is_x3d) {
		/*parse all commands*/
		parser->au_time = 0;
		parser->au_is_rap = 1;
		is_done = 1;
		while (!xml_element_done(&parser->xml_parser, "X3D") && !parser->last_error) {
			str = xml_get_element(&parser->xml_parser);
			/*in X3D only get scene*/
			if (!strcmp(str, "Scene")) {
				parser->stream_id = 0;
				xmt_parse_command(parser, str, NULL);
				parser->au_is_rap = 0;
			} else {
				xml_skip_element(&parser->xml_parser, str);
			}
		}
	} else {
		/*check header*/
		while (1) {
			str = xml_get_element(&parser->xml_parser);
			if (!str) {
				xmt_report(parser, M4BadParam, "Expecting <Header> in XMT-A document");
				goto exit;
			}
			if (!strcmp(str, "Header")) break;
			xml_skip_element(&parser->xml_parser, str);
		}
		xml_skip_attributes(&parser->xml_parser);
		parser->load->ctx->root_od = (ObjectDescriptor *) xmt_parse_descriptor(parser, NULL, NULL);
		if (!xml_element_done(&parser->xml_parser, "Header")) {
			xmt_report(parser, M4BadParam, "Expecting </Header> in XMT-A document");
			goto exit;
		}

		/*check body*/
		while (1) {
			str = xml_get_element(&parser->xml_parser);
			if (!str) {
				xmt_report(parser, M4BadParam, "Expecting <Body> in XMT-A document");
				goto exit;
			}
			if (!strcmp(str, "Body")) break;
			xml_skip_element(&parser->xml_parser, str);
		}
		xml_skip_attributes(&parser->xml_parser);

		/*parse all commands*/
		parser->au_time = 0;
		parser->au_is_rap = 1;
		is_done = 1;
		while (!xml_element_done(&parser->xml_parser, "Body") && !parser->last_error) {
			str = xml_get_element(&parser->xml_parser);
			/*explicit commands are parsed in Run*/
			if (!strcmp(str, "par")) {
				is_done = 0;
				break;
			}

			/*implicit command*/
			else {
				parser->stream_id = 0;
				xmt_parse_command(parser, str, NULL);
				parser->au_is_rap = 0;
			}
		}
	}
	if (is_done) parser->xml_parser.done = 1;

exit:
	e = parser->last_error;
	if (e) M4SM_LoaderDone(load);
	return e;
}
