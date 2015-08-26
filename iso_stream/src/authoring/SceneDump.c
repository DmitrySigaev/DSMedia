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

#include <m4_author.h>
#include <intern/m4_scenegraph_dev.h>
#include <m4_x3d_nodes.h>

#ifndef M4_READ_ONLY


/*for QP types*/
#include "../BIFS/Quantize.h"

#ifdef M4_USE_LASeR
#include <../LASeR/m4_laser_dev.h>
#endif

typedef struct _scenedump
{
	/*the scene we're dumping - set at each SceneReplace or mannually*/
	LPSCENEGRAPH sg;
	/*the proto we're dumping*/
	LPPROTO current_proto;
	FILE *trace;
	u32 indent;
	
	u32 dump_mode;
	u16 CurrentESID;
	u8 ind_char;
	Bool XMLDump, X3DDump;

	Chain *dump_nodes;

	/*nodes created through conditionals while parsing but not applied*/
	Chain *mem_def_nodes;

	Bool skip_scene_replace;
	/*for route insert/replace in conditionals in current scene replace*/
	Chain *current_com_list;

} SceneDumper;


M4Err DumpRoute(SceneDumper *sdump, Route *r, u32 dump_type);
void DumpNode(SceneDumper *sdump, SFNode *node, Bool in_list, char *fieldContainer);

M4Err SD_DumpCommandList(SceneDumper *sdump, Chain *comList, u32 indent, Bool skip_first_replace);

SceneDumper *NewSceneDumper(LPSCENEGRAPH graph, char *rad_name, char indent_char, u32 dump_mode)
{
	SceneDumper *tmp;
	if (!graph) return NULL;
	tmp = malloc(sizeof(SceneDumper));
	memset(tmp, 0, sizeof(SceneDumper));


	/*store original*/
	tmp->dump_mode = dump_mode;

	if (dump_mode==M4SM_DUMP_AUTO_TXT) {
		if (!graph->RootNode || (graph->RootNode->sgprivate->tag<=TAG_RANGE_LAST_MPEG4) ) {
			dump_mode = M4SM_DUMP_BT;
		} else {
			dump_mode = M4SM_DUMP_X3D_VRML;
		}
	}
	else if (dump_mode==M4SM_DUMP_AUTO_XML) {
		if (!graph->RootNode || (graph->RootNode->sgprivate->tag<=TAG_RANGE_LAST_MPEG4) ) {
			dump_mode = M4SM_DUMP_XMTA;
		} else {
			dump_mode = M4SM_DUMP_X3D_XML;
		}
	}

	tmp->XMLDump = 0;
	tmp->X3DDump = 0;
	if (rad_name) {
		switch (dump_mode) {
		case M4SM_DUMP_X3D_XML: strcat(rad_name, ".x3d"); tmp->XMLDump = 1; tmp->X3DDump = 1; break;
		case M4SM_DUMP_XMTA: strcat(rad_name, ".xmt"); tmp->XMLDump = 1; break;
		case M4SM_DUMP_X3D_VRML: strcat(rad_name, ".x3dv"); tmp->X3DDump = 1; break;
		case M4SM_DUMP_VRML: strcat(rad_name, ".wrl"); break;
		default: strcat(rad_name, ".bt"); break;
		}
		tmp->trace = fopen(rad_name, "wt");
		if (!tmp->trace) {
			free(tmp);
			return NULL;
		}
	} else {
		tmp->trace = stdout;
		switch (dump_mode) {
		case M4SM_DUMP_X3D_XML: tmp->XMLDump = 1; tmp->X3DDump = 1; break;
		case M4SM_DUMP_XMTA: tmp->XMLDump = 1; break;
		case M4SM_DUMP_X3D_VRML: tmp->X3DDump = 1; break;
		default: break;
		}
	}
	tmp->ind_char = indent_char;
	tmp->dump_nodes = NewChain();
	tmp->mem_def_nodes = NewChain();
	tmp->sg = graph;
	return tmp;
}

void DeleteSceneDumper(SceneDumper *sdump)
{
	DeleteChain(sdump->dump_nodes);
	while (ChainGetCount(sdump->mem_def_nodes)) {
		SFNode *tmp = ChainGetEntry(sdump->mem_def_nodes, 0);
		ChainDeleteEntry(sdump->mem_def_nodes, 0);
		Node_Unregister(tmp, NULL);
	}
	DeleteChain(sdump->mem_def_nodes);
	if (sdump->trace != stdout) fclose(sdump->trace);
	free(sdump);
}


void SD_SetupDump(SceneDumper *sdump, Descriptor *root_od)
{
	if (!sdump->X3DDump) {
		/*setup XMT*/
		if (sdump->XMLDump) {
			fprintf(sdump->trace, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
			fprintf(sdump->trace, "<XMT-A xmlns=\"urn:mpeg:mpeg4:xmta:schema:2002\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"urn:mpeg:mpeg4:xmta:schema:2002 xmt-a.xsd\">\n");
			fprintf(sdump->trace, " <Header>\n");
			if (root_od) OD_DumpDescriptor(root_od, sdump->trace, 1, 1);
			fprintf(sdump->trace, " </Header>\n");
			fprintf(sdump->trace, " <Body>\n");
		} else {
			if (sdump->dump_mode==M4SM_DUMP_VRML) {
				fprintf(sdump->trace, "#VRML V2.0\n");
			} else {
				/*dump root OD*/
				if (root_od) OD_DumpDescriptor(root_od, sdump->trace, 0, 0);
			}
			fprintf(sdump->trace, "\n");
		}
	} else {
		if (sdump->XMLDump) {
			fprintf(sdump->trace, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
			fprintf(sdump->trace, "<!DOCTYPE X3D PUBLIC \"ISO//Web3D//DTD X3D 3.0//EN\" \"http://www.web3d.org/specifications/x3d-3.0.dtd\">\n");
			fprintf(sdump->trace, "<X3D xmlns:xsd=\"http://www.w3.org/2001/XMLSchema-instance\" xsd:noNamespaceSchemaLocation=\"http://www.web3d.org/specifications/x3d-3.0.xsd\" version=\"3.0\">\n");
			fprintf(sdump->trace, "<head>\n");
			fprintf(sdump->trace, "<meta content=\"X3D File Converted/Dumped by GPAC Version %s\" name=\"generator\"/>\n", M4_VERSION);
			fprintf(sdump->trace, "</head>\n");
		} else {
			fprintf(sdump->trace, "#X3D V3.0\n\n");
		}
	}
}

void SD_FinalizeDump(SceneDumper *sdump)
{
	if (!sdump->XMLDump) return;

	if (!sdump->X3DDump) {
		fprintf(sdump->trace, " </Body>\n");
		fprintf(sdump->trace, "</XMT-A>\n");
	} else {
		fprintf(sdump->trace, "</X3D>\n");
	}
}

Bool SD_IsDEFNode(SceneDumper *sdump, SFNode *node)
{
	u32 i;

	for (i=0; i<ChainGetCount(sdump->dump_nodes); i++) {
		SFNode *tmp = ChainGetEntry(sdump->dump_nodes, i);
		if (tmp == node) return 0;
	}
	ChainAddEntry(sdump->dump_nodes, node);
	return 1;
}

SFNode *SD_FindNode(SceneDumper *sdump, u32 ID)
{
	SFNode *ret = SG_FindNode(sdump->sg, ID);
	if (ret) return ret;
/*	
	u32 i;
	for (i=0; i<ChainGetCount(sdump->mem_def_nodes); i++) {
		ret = ChainGetEntry(sdump->mem_def_nodes, i);
		if (ret->sgprivate->NodeID == ID) return ret;
	}
*/
	
	return NULL;
}

#define DUMP_IND(sdump)	\
	if (sdump->trace && !sdump->XMLDump) {		\
		u32 z;	\
		for (z=0; z<sdump->indent; z++) fprintf(sdump->trace, "%c", sdump->ind_char);	\
	}


void StartElement(SceneDumper *sdump, const char *name)
{
	if (!sdump->trace) return;
	DUMP_IND(sdump);
	if (!sdump->XMLDump) {
		fprintf(sdump->trace, "%s {\n", name);
	} else if (sdump->XMLDump) {
		fprintf(sdump->trace, "<%s", name);
	}
}
void EndElementHeader(SceneDumper *sdump, Bool has_sub_el)
{
	if (!sdump->trace) return;
	if (sdump->XMLDump) {
		if (has_sub_el) {
			fprintf(sdump->trace, ">\n");
		} else {
			fprintf(sdump->trace, "/>\n");
		}
	}
}
void EndElement(SceneDumper *sdump, const char *name, Bool had_sub_el)
{
	if (!sdump->trace) return;
	if (!sdump->XMLDump) {
		DUMP_IND(sdump);
		fprintf(sdump->trace, "}\n");
	} else {
		if (had_sub_el) {
			DUMP_IND(sdump);
			fprintf(sdump->trace, "</%s>\n", name);
		}
	}
}

void StartAttribute(SceneDumper *sdump, char *name)
{
	if (!sdump->trace) return;
	if (!sdump->XMLDump) {
		DUMP_IND(sdump);
		fprintf(sdump->trace, "%s ", name);
	} else {
		fprintf(sdump->trace, " %s=\"", name);
	}
}

void EndAttribute(SceneDumper *sdump)
{
	if (!sdump->trace) return;
	if (!sdump->XMLDump) {
		fprintf(sdump->trace, "\n");
	} else {
		fprintf(sdump->trace, "\"");
	}
}


void StartList(SceneDumper *sdump, char *name)
{
	if (!sdump->trace) return;
	DUMP_IND(sdump);
	if (!sdump->XMLDump) {
		if (name) 
			fprintf(sdump->trace, "%s [\n", name);
		else
			fprintf(sdump->trace, "[\n");
	} else {
		fprintf(sdump->trace, "<%s>\n", name);
	}
}

void EndList(SceneDumper *sdump, char *name)
{
	if (!sdump->trace) return;
	DUMP_IND(sdump);
	if (!sdump->XMLDump) {
		fprintf(sdump->trace, "]\n");
	} else {
		fprintf(sdump->trace, "</%s>\n", name);
	}
}

void DumpNodeID(SceneDumper *sdump, SFNode *node)
{
	if (!sdump->trace) return;
	if (node->sgprivate->NodeName) 
		fprintf(sdump->trace, "%s", node->sgprivate->NodeName);
	else
		fprintf(sdump->trace, "N%d", node->sgprivate->NodeID - 1);
}

Bool DumpFindRouteName(SceneDumper *sdump, u32 ID, const char **outName)
{
	LPROUTE r;
	u32 i;
	r = SG_FindRoute(sdump->sg, ID);
	if (r) { (*outName) = r->name; return 1; }

	if (!sdump->current_com_list) return 0;

	for (i=1; i<ChainGetCount(sdump->current_com_list); i++) {
		SGCommand *com = ChainGetEntry(sdump->current_com_list, i);
		if ((com->tag == SG_RouteInsert) || (com->tag == SG_RouteReplace)) {
			if (com->RouteID==ID) {
				(*outName) = com->def_name;
				return 1;
			}
		} else return 0;
	}
	return 0;
}

void DumpRouteID(SceneDumper *sdump, u32 routeID, char *rName)
{
	if (!sdump->trace) return;
	if (!rName) DumpFindRouteName(sdump, routeID, (const char **) &rName);

	if (rName) 
		fprintf(sdump->trace, "%s", rName);
	else
		fprintf(sdump->trace, "R%d", routeID - 1);
}

void DumpBool(SceneDumper *sdump, char *name, u32 value)
{
	if (!sdump->trace) return;
	StartAttribute(sdump, name);
	fprintf(sdump->trace, "%s", value ? "true" : "false");
	EndAttribute(sdump);
}


void DumpSFField(SceneDumper *sdump, u32 type, void *ptr, Bool is_mf)
{
	switch (type) {
	case FT_SFBool:
		fprintf(sdump->trace, "%s", * ((SFBool *)ptr) ? "TRUE" : "FALSE");
		break;
	case FT_SFInt32:
		fprintf(sdump->trace, "%d", * ((SFInt32 *)ptr) );
		break;
	case FT_SFFloat:
		fprintf(sdump->trace, "%g", * ((SFFloat *)ptr) );
		break;
	case FT_SFDouble:
		fprintf(sdump->trace, "%g", * ((SFDouble *)ptr) );
		break;
	case FT_SFTime:
		fprintf(sdump->trace, "%g", * ((SFTime *)ptr) );
		break;
	case FT_SFColor:
		fprintf(sdump->trace, "%g %g %g", ((SFColor *)ptr)->red, ((SFColor *)ptr)->green, ((SFColor *)ptr)->blue);
		break;
	case FT_SFColorRGBA:
		fprintf(sdump->trace, "%g %g %g %g", ((SFColorRGBA *)ptr)->red, ((SFColorRGBA *)ptr)->green, ((SFColorRGBA *)ptr)->blue, ((SFColorRGBA *)ptr)->alpha);
		break;
	case FT_SFVec2f:
		fprintf(sdump->trace, "%g %g", ((SFVec2f *)ptr)->x, ((SFVec2f *)ptr)->y);
		break;
	case FT_SFVec2d:
		fprintf(sdump->trace, "%g %g", ((SFVec2d *)ptr)->x, ((SFVec2d *)ptr)->y);
		break;
	case FT_SFVec3f:
		fprintf(sdump->trace, "%g %g %g", ((SFVec3f *)ptr)->x, ((SFVec3f *)ptr)->y, ((SFVec3f *)ptr)->z);
		break;
	case FT_SFVec3d:
		fprintf(sdump->trace, "%g %g %g", ((SFVec3d *)ptr)->x, ((SFVec3d *)ptr)->y, ((SFVec3d *)ptr)->z);
		break;
	case FT_SFRotation:
		fprintf(sdump->trace, "%g %g %g %g", ((SFRotation *)ptr)->xAxis, ((SFRotation *)ptr)->yAxis, ((SFRotation *)ptr)->zAxis, ((SFRotation *)ptr)->angle);
		break;

	case FT_SFScript:
	{
		u32 len, i;
		char *str;
		short *uniLine;
		str = ((SFScript *)ptr)->script_text;
		len = strlen(str);
		uniLine = malloc(sizeof(short) * len);
		len = utf8_mbstowcs(uniLine, len, (const char **) &str);
		if (len != (size_t) -1) {
			if (!sdump->XMLDump) fputc('\"', sdump->trace);

			for (i=0; i<len; i++) {
				if (!sdump->XMLDump) {
					fputwc(uniLine[i], sdump->trace);
				} else {
					switch (uniLine[i]) {
					case '&': fprintf(sdump->trace, "&amp;"); break;
					case '<': fprintf(sdump->trace, "&lt;"); break;
					case '>': fprintf(sdump->trace, "&gt;"); break;
					case '\'': 
					case '"': 
						fprintf(sdump->trace, "&apos;");
						break;
					case 0:
						break;
					/*FIXME: how the heck can we preserve newlines and spaces of JavaScript in 
					an XML attribute in any viewer ? */
					default:
						fputwc(uniLine[i], sdump->trace);
						break;
					}
				}
			} 
			if (!sdump->XMLDump) fprintf(sdump->trace, "\"\n");
		}
		free(uniLine);
		DUMP_IND(sdump);
	}
		break;

	case FT_SFString:
	{
		u32 i, len;
		char *str;
		short *uniLine;
		if (sdump->XMLDump) {
			if (is_mf) fprintf(sdump->trace, "\'");
		} else {
			fprintf(sdump->trace, "\"");
		}
		/*dump in unicode*/
		str = ((SFString *)ptr)->buffer;
		if (str) {
			len = strlen(str);
			if (len) {
				uniLine = malloc(sizeof(short) * len);
				len = utf8_mbstowcs(uniLine, len, (const char **) &str);
				if (len != (size_t) (-1)) {
					for (i=0; i<len; i++) {
						if (uniLine[i] == (u16) '\"') fprintf(sdump->trace, "\\");
						if (sdump->XMLDump) {
							switch (uniLine[i]) {
							case '\'': fprintf(sdump->trace, "&apos;"); break;
							case '\"': fprintf(sdump->trace, "&quot;"); break;
							case '&': fprintf(sdump->trace, "&amp;"); break;
							case '>': fprintf(sdump->trace, "&gt;"); break;
							case '<': fprintf(sdump->trace, "&lt;"); break;
							default:
								if (uniLine[i]<128) {
									fprintf(sdump->trace, "%c", (u8) uniLine[i]);
								} else {
									fprintf(sdump->trace, "&#%d;", uniLine[i]);
								}
								break;
							}
						} else {
							fputwc(uniLine[i], sdump->trace);
						}
					}
				}
				free(uniLine);
			}
		}

		if (sdump->XMLDump) {
			if (is_mf) fprintf(sdump->trace, "\'");
		} else {
			fprintf(sdump->trace, "\"");
		}
	}
		break;

	case FT_SFURL:
		if (((SFURL *)ptr)->url) {
			u32 len;
			char *str;
			short uniLine[5000];
			str = ((SFURL *)ptr)->url;
			len = utf8_mbstowcs(uniLine, 5000, (const char **) &str);
			if (len != (size_t) -1) {
				fprintf(sdump->trace, sdump->XMLDump ? "'" : "\"");
				fwprintf(sdump->trace, uniLine);
				fprintf(sdump->trace, sdump->XMLDump ? "'" : "\"");
			}
		} else {
			fprintf(sdump->trace, "od:%d", ((SFURL *)ptr)->OD_ID);
		}
		break;
	case FT_SFImage:
	{
		u32 i, count;
		SFImage *img = (SFImage *)ptr;
		fprintf(sdump->trace, "%d %d %d", img->width, img->height, img->numComponents);
		count = img->width * img->height * img->numComponents;
		for (i=0; i<count; ) {
			switch (img->numComponents) {
			case 1:
				fprintf(sdump->trace, " 0x%02X", img->pixels[i]);
				i++;
				break;
			case 2:
				fprintf(sdump->trace, " 0x%02X%02X", img->pixels[i], img->pixels[i+1]);
				i+=2;
				break;
			case 3:
				fprintf(sdump->trace, " 0x%02X%02X%02X", img->pixels[i], img->pixels[i+1], img->pixels[i+2]);
				i+=3;
				break;
			case 4:
				fprintf(sdump->trace, " 0x%02X%02X%02X%02X", img->pixels[i], img->pixels[i+1], img->pixels[i+2], img->pixels[i+3]);
				i+=4;
				break;
			}
		}
	}
	break;
	}
}


void DumpFieldValue(SceneDumper *sdump, FieldInfo field)
{
	GenMFField *mffield;
	u32 i, j, sf_type;
	SFNode *child;
	Chain *list;
	void *slot_ptr;

	switch (field.fieldType) {
	case FT_SFNode:
		assert ( *(SFNode **)field.far_ptr);
		DumpNode(sdump, *(SFNode **)field.far_ptr, 0, NULL);
		return;
	case FT_MFNode:
		list = * ((Chain **) field.far_ptr);
		assert(ChainGetCount(list));
		sdump->indent++;
		for (j=0; j<ChainGetCount(list); j++) {
			child = ChainGetEntry(list, j);
			DumpNode(sdump, child, 1, NULL);
		}
		sdump->indent--;
		return;
	case FT_SFCommandBuffer:
		{
		}
		return;
	}
	if (VRML_IsSFField(field.fieldType)) {
		if (sdump->XMLDump) StartAttribute(sdump, "value");
		DumpSFField(sdump, field.fieldType, field.far_ptr, 0);
		if (sdump->XMLDump) EndAttribute(sdump);
	} else {
		mffield = (GenMFField *) field.far_ptr;
		sf_type = VRML_GetSFType(field.fieldType);
		if (!sdump->XMLDump) {
			fprintf(sdump->trace, "[");
		} else {
			StartAttribute(sdump, "value");
		}
		for (i=0; i<mffield->count; i++) {
			if (i) fprintf(sdump->trace, " ");
			VRML_MF_GetItem(field.far_ptr, field.fieldType, &slot_ptr, i);
			/*this is to cope with single MFString which shall appear as SF in XMT*/
			DumpSFField(sdump, sf_type, slot_ptr, (mffield->count>1) ? 1 : 0);
		}
		if (!sdump->XMLDump) {
			fprintf(sdump->trace, "]");
		} else {
			EndAttribute(sdump);
		}
	}
}

Bool SD_NeedsFieldContainer(SFNode *node, FieldInfo *fi)
{
	u32 i, count, nb_ndt;
	FieldInfo info;
	if (!strcmp(fi->name, "children")) return 0;
	nb_ndt = 0;
	count = Node_GetFieldCount(node);
	for (i=0; i<count; i++) {
		Node_GetField(node, i, &info);
		if ((info.eventType==ET_EventIn) || (info.eventType==ET_EventOut)) continue;
		if (info.NDTtype==fi->NDTtype) nb_ndt++;
	}
	return (nb_ndt>1) ? 1 : 0;
}

void DumpField(SceneDumper *sdump, SFNode *node, FieldInfo field)
{
	GenMFField *mffield;
	u32 i, j, sf_type;
	SFNode *child;
	Bool needs_field_container;
	Chain *list;
	void *slot_ptr;

	switch (field.fieldType) {
	case FT_SFNode:
		assert ( *(SFNode **)field.far_ptr);
		
		if (sdump->XMLDump) {
			if (!sdump->X3DDump) {
				StartElement(sdump, (char *) field.name);
				EndElementHeader(sdump, 1);
				sdump->indent++;
			}
		} else {
			StartAttribute(sdump, (char *)field.name);
		}
		DumpNode(sdump, *(SFNode **)field.far_ptr, 0, NULL);
		
		if (sdump->XMLDump) {
			if (!sdump->X3DDump) {
				sdump->indent--;
				EndElement(sdump, (char *) field.name, 1);
			}
		} else {
			EndAttribute(sdump);
		}
		return;
	case FT_MFNode:
		needs_field_container = 0;
		if (sdump->XMLDump && sdump->X3DDump) needs_field_container = SD_NeedsFieldContainer(node, &field);
		list = * ((Chain **) field.far_ptr);
		assert(ChainGetCount(list));
		if (!sdump->XMLDump || !sdump->X3DDump) StartList(sdump, (char *) field.name);
		sdump->indent++;
		for (j=0; j<ChainGetCount(list); j++) {
			child = ChainGetEntry(list, j);
			DumpNode(sdump, child, 1, needs_field_container ? (char *) field.name : NULL);
		}
		sdump->indent--;
		if (!sdump->XMLDump || !sdump->X3DDump) EndList(sdump, (char *) field.name);
		return;
	case FT_SFCommandBuffer:
	{
		SFCommandBuffer *cb = (SFCommandBuffer *)field.far_ptr;
		StartElement(sdump, (char *) field.name);
		EndElementHeader(sdump, 1);
		sdump->indent++;
		if (!ChainGetCount(cb->commandList)) {
			/*the arch does not allow for that (we would need a codec and so on, or decompress the command list
			in all cases...)*/
			if (sdump->trace && cb->bufferSize) {
				if (sdump->XMLDump) fprintf(sdump->trace, "<!--SFCommandBuffer cannot be dumped while playing - use MP4Box instead-->\n");
				else fprintf(sdump->trace, "#SFCommandBuffer cannot be dumped while playing - use MP4Box instead\n");
			}
		} else {
			SD_DumpCommandList(sdump, cb->commandList, sdump->indent, 0);
		}
		sdump->indent--;
		EndElement(sdump, (char *) field.name, 1);
	}
		return;
	}
	if (VRML_IsSFField(field.fieldType)) {
		StartAttribute(sdump, (char *) field.name);
		DumpSFField(sdump, field.fieldType, field.far_ptr, 0);
		EndAttribute(sdump);
	} else {
		mffield = (GenMFField *) field.far_ptr;
		sf_type = VRML_GetSFType(field.fieldType);
		StartAttribute(sdump, (char *) field.name);
		if (!sdump->XMLDump) fprintf(sdump->trace, "[");
		for (i=0; i<mffield->count; i++) {
			if (i) fprintf(sdump->trace, " ");
			VRML_MF_GetItem(field.far_ptr, field.fieldType, &slot_ptr, i);
			DumpSFField(sdump, sf_type, slot_ptr, (mffield->count>1) ? 1 : 0);
		}
		if (!sdump->XMLDump) fprintf(sdump->trace, "]");
		EndAttribute(sdump);
	}
}

const char * GetXMTFieldTypeName(u32 fieldType)
{
	switch (fieldType) {
	case FT_SFBool: return "Boolean";
	case FT_SFInt32: return "Integer";
	case FT_SFColor: return "Color";
	case FT_SFVec2f: return "Vector2";
	case FT_SFImage: return "Image";
	case FT_SFTime: return "Time";
	case FT_SFFloat: return "Float";
	case FT_SFVec3f: return "Vector3";
	case FT_SFRotation: return "Rotation";
	case FT_SFString: return "String";
	case FT_SFNode: return "Node";
	case FT_MFBool: return "Booleans";
	case FT_MFInt32: return "Integers";
	case FT_MFColor: return "Colors";
	case FT_MFVec2f: return "Vector2s";
	case FT_MFImage: return "Images";
	case FT_MFTime: return "Times";
	case FT_MFFloat: return "Floats";
	case FT_MFVec3f: return "Vector3s";
	case FT_MFRotation: return "Rotations";
	case FT_MFString: return "Strings";
	case FT_MFNode: return "Nodes";
	default: return "unknown";
	}
}
const char * GetXMTFieldTypeValueName(u32 fieldType)
{
	switch (fieldType) {
	case FT_SFBool: return "booleanValue";
	case FT_SFInt32: return "integerValue";
	case FT_SFColor: return "colorValue";
	case FT_SFVec2f: return "vector2Value";
	case FT_SFImage: return "imageValue";
	case FT_SFTime: return "timeValue";
	case FT_SFFloat: return "floatValue";
	case FT_SFVec3f: return "vector3Value";
	case FT_SFRotation: return "rotationValue";
	case FT_SFString: return "stringValue";
	case FT_MFBool: return "booleanArrayValue";
	case FT_MFInt32: return "integerArrayValue";
	case FT_MFColor: return "colorArrayValue";
	case FT_MFVec2f: return "vector2ArrayValue";
	case FT_MFImage: return "imageArrayValue";
	case FT_MFTime: return "timeArrayValue";
	case FT_MFFloat: return "floatArrayValue";
	case FT_MFVec3f: return "vector3ArrayValue";
	case FT_MFRotation: return "rotationArrayValue";
	case FT_MFString: return "stringArrayValue";
	default: return "unknown";
	}
}

const char *SD_GetQuantCatName(u32 QP_Type)
{
	switch (QP_Type) {
	case QC_3DPOS: return "position3D";
	case QC_2DPOS: return "position2D";
	case QC_ORDER: return "drawingOrder";
	case QC_COLOR: return "color";
	case QC_TEXTURE_COORD: return "textureCoordinate";
	case QC_ANGLE: return "angle";
	case QC_SCALE: return "scale";
	case QC_INTERPOL_KEYS: return "keys";
	case QC_NORMALS: return "normals";
	case QC_ROTATION: return "rotations";
	case QC_SIZE_3D: return "size3D";
	case QC_SIZE_2D: return "size2D";
	case QC_LINEAR_SCALAR: return "linear";
	case QC_COORD_INDEX: return "coordIndex";
	default: return "unknown";
	}
}

/*field dumping for proto declaration and Script*/
void DumpDynField(SceneDumper *sdump, SFNode *node, FieldInfo field, Bool has_sublist)
{
	GenMFField *mffield;
	u32 i, sf_type;
	void *slot_ptr;

	if (VRML_IsSFField(field.fieldType)) {
		DUMP_IND(sdump);
		if (sdump->XMLDump) {
			if (sdump->X3DDump) {
				fprintf(sdump->trace, "<field name=\"%s\" type=\"%s\" accessType=\"%s\"",
					field.name, VRML_GetFieldTypeName(field.fieldType), VRML_GetEventTypeName(field.eventType, 1));
			} else {
				fprintf(sdump->trace, "<field name=\"%s\" type=\"%s\" vrml97Hint=\"%s\"",
					field.name, GetXMTFieldTypeName(field.fieldType), VRML_GetEventTypeName(field.eventType, 0));
			}

			if ((field.eventType == ET_Field) || (field.eventType == ET_ExposedField)) {
				if (field.fieldType == FT_SFNode) {
					if (!sdump->X3DDump) {
						fprintf(sdump->trace, ">\n");
						sdump->indent++;
						fprintf(sdump->trace, "<node>");
						DumpNode(sdump, field.far_ptr ? *(SFNode **)field.far_ptr : NULL, 0, NULL);
						fprintf(sdump->trace, "</node>");
						sdump->indent--;
						if (!has_sublist)
							fprintf(sdump->trace, "</field>\n");
					} else {
						if (field.far_ptr) {
							fprintf(sdump->trace, ">\n");
							DumpNode(sdump, *(SFNode **)field.far_ptr, 0, NULL);
							fprintf(sdump->trace, "</field>\n");
						} else {
							fprintf(sdump->trace, "/>\n");
						}
					}
					DUMP_IND(sdump);
				} else {
					if (sdump->X3DDump) {
						fprintf(sdump->trace, " value=\"");
					} else {
						fprintf(sdump->trace, " %s=\"", GetXMTFieldTypeValueName(field.fieldType));
					}
					DumpSFField(sdump, field.fieldType, field.far_ptr, 0);
					if (has_sublist)
						fprintf(sdump->trace, "\">\n");
					else
						fprintf(sdump->trace, "\"/>\n");
				}
			} else {
				fprintf(sdump->trace, "/>\n");
			}
		} else {
			fprintf(sdump->trace, "%s %s %s", VRML_GetEventTypeName(field.eventType, sdump->X3DDump), VRML_GetFieldTypeName(field.fieldType), field.name);
			if ((field.eventType==ET_Field) || (field.eventType==ET_ExposedField)) {
				fprintf(sdump->trace, " ");
				if (field.fieldType == FT_SFNode) {
					DumpNode(sdump, field.far_ptr ? *(SFNode **)field.far_ptr : NULL, 0, NULL);
				} else {
					DumpFieldValue(sdump, field);
				}
			}
			fprintf(sdump->trace, "\n");
		}
	} else {
		mffield = (GenMFField *) field.far_ptr;
		sf_type = VRML_GetSFType(field.fieldType);

		DUMP_IND(sdump);
		if (!sdump->XMLDump) {
			fprintf(sdump->trace, "%s %s %s", VRML_GetEventTypeName(field.eventType, sdump->X3DDump), VRML_GetFieldTypeName(field.fieldType), field.name);
			if ((field.eventType==ET_Field) || (field.eventType==ET_ExposedField)) {
				fprintf(sdump->trace, " [");

				for (i=0; i<mffield->count; i++) {
					if (i) fprintf(sdump->trace, " ");
					if (field.fieldType != FT_MFNode) {
						VRML_MF_GetItem(field.far_ptr, field.fieldType, &slot_ptr, i);
						DumpSFField(sdump, sf_type, slot_ptr, (mffield->count>1) ? 1 : 0);
					}
				}
				fprintf(sdump->trace, "]");
			}
			fprintf(sdump->trace, "\n");
		} else {
			if (sdump->X3DDump) {
				fprintf(sdump->trace, "<field name=\"%s\" type=\"%s\" accessType=\"%s\"",
					field.name, VRML_GetFieldTypeName(field.fieldType), VRML_GetEventTypeName(field.eventType, 1));
			} else {
				fprintf(sdump->trace, "<field name=\"%s\" type=\"%s\" vrml97Hint=\"%s\"",
					field.name, GetXMTFieldTypeName(field.fieldType), VRML_GetEventTypeName(field.eventType, 0));
			}

			if ((field.eventType==ET_Field) || (field.eventType==ET_ExposedField)) {
				if (sf_type == FT_SFNode) {
					Chain *list = *(Chain **)field.far_ptr;
					fprintf(sdump->trace, ">\n");
					sdump->indent++;
					if (!sdump->X3DDump) fprintf(sdump->trace, "<nodes>");
					for (i=0; i<ChainGetCount(list); i++) {
						SFNode *tmp = ChainGetEntry(list, i);
						DumpNode(sdump, tmp, 1, NULL);
					}
					if (!sdump->X3DDump) fprintf(sdump->trace, "</nodes>");
					sdump->indent++;
					DUMP_IND(sdump);
					if (!has_sublist)
						fprintf(sdump->trace, "</field>\n");
				} else {
					if (sdump->X3DDump) {
						fprintf(sdump->trace, " value=\"");
					} else {
						fprintf(sdump->trace, " %s=\"", GetXMTFieldTypeValueName(field.fieldType));
					}
					for (i=0; i<mffield->count; i++) {
						if (i) fprintf(sdump->trace, " ");
						if (field.fieldType != FT_MFNode) {
							VRML_MF_GetItem(field.far_ptr, field.fieldType, &slot_ptr, i);
							DumpSFField(sdump, sf_type, slot_ptr, (mffield->count>1) ? 1 : 0);
						}
					}
					if (has_sublist)
						fprintf(sdump->trace, "\">\n");
					else
						fprintf(sdump->trace, "\"/>\n");
				}
			} else {
				fprintf(sdump->trace, "/>\n");
			}
		}
	}
}


/*field dumping for proto instance*/
void DumpProtoField(SceneDumper *sdump, SFNode *node, FieldInfo field)
{
	GenMFField *mffield;
	u32 i, sf_type;
	void *slot_ptr;

	DUMP_IND(sdump);
	fprintf(sdump->trace, "<fieldValue name=\"%s\" ", field.name);
	if (VRML_IsSFField(field.fieldType)) {
		if (field.fieldType == FT_SFNode) {
			fprintf(sdump->trace, ">\n");
			sdump->indent++;
			if (!sdump->X3DDump) fprintf(sdump->trace, "<node>");
			DumpNode(sdump, field.far_ptr ? *(SFNode **)field.far_ptr : NULL, 0, NULL);
			if (!sdump->X3DDump) fprintf(sdump->trace, "</node>");
			sdump->indent--;
			DUMP_IND(sdump);
			fprintf(sdump->trace, "</fieldValue>\n");
		} else {
			if (sdump->X3DDump) {
				fprintf(sdump->trace, " value=\"");
			} else {
				fprintf(sdump->trace, " %s=\"", GetXMTFieldTypeValueName(field.fieldType));
			}
			DumpSFField(sdump, field.fieldType, field.far_ptr, 0);
			fprintf(sdump->trace, "\"/>\n");
		}
	} else {
		mffield = (GenMFField *) field.far_ptr;
		sf_type = VRML_GetSFType(field.fieldType);

		if ((field.eventType==ET_Field) || (field.eventType==ET_ExposedField)) {
			if (sf_type == FT_SFNode) {
				Chain *list = *(Chain **)field.far_ptr;
				fprintf(sdump->trace, ">\n");
				sdump->indent++;
				if (!sdump->X3DDump) fprintf(sdump->trace, "<nodes>");
				for (i=0; i<ChainGetCount(list); i++) {
					SFNode *tmp = ChainGetEntry(list, i);
					DumpNode(sdump, tmp, 1, NULL);
				}
				if (!sdump->X3DDump) fprintf(sdump->trace, "</nodes>");
				sdump->indent--;
				DUMP_IND(sdump);
				fprintf(sdump->trace, "</fieldValue>\n");
			} else {
				if (sdump->X3DDump) {
					fprintf(sdump->trace, " value=\"");
				} else {
					fprintf(sdump->trace, " %s=\"", GetXMTFieldTypeValueName(field.fieldType));
				}
				for (i=0; i<mffield->count; i++) {
					if (i) fprintf(sdump->trace, " ");
					if (field.fieldType != FT_MFNode) {
						VRML_MF_GetItem(field.far_ptr, field.fieldType, &slot_ptr, i);
						DumpSFField(sdump, sf_type, slot_ptr, (mffield->count>1) ? 1 : 0);
					}
				}
				fprintf(sdump->trace, "\"/>\n");
			}
		}
	}
}

Route *SD_GetISedField(SceneDumper *sdump, SFNode *node, FieldInfo *field) 
{
	u32 i;
	for (i=0; i<ChainGetCount(sdump->current_proto->sub_graph->Routes); i++) {
		Route *r = ChainGetEntry(sdump->current_proto->sub_graph->Routes, i);
		if (!r->IS_route) continue;
		if ((r->ToNode==node) && (r->ToFieldIndex==field->fieldIndex)) return r;
	}
	if (!node) return NULL;
	for (i=0; i<ChainGetCount(node->sgprivate->outRoutes); i++) {
		Route *r = ChainGetEntry(node->sgprivate->outRoutes, i);
		if (!r->IS_route) continue;
		if (r->FromFieldIndex == field->fieldIndex) return r;
	}
	return NULL;
}

void DumpISField(SceneDumper *sdump, SFNode *node, FieldInfo field, Bool isScript, Bool skip_is)
{
	FieldInfo pfield;

	Route *r = SD_GetISedField(sdump, node, &field);
	if (r->FromNode) {
		pfield.fieldIndex = r->ToFieldIndex;
		Proto_GetField(sdump->current_proto, NULL, &pfield);
	} else {
		pfield.fieldIndex = r->FromFieldIndex;
		Proto_GetField(sdump->current_proto, NULL, &pfield);
	}
	
	if (!sdump->XMLDump) {
		DUMP_IND(sdump);
		if (isScript) fprintf(sdump->trace, "%s %s ", VRML_GetEventTypeName(field.eventType, sdump->X3DDump), VRML_GetFieldTypeName(field.fieldType));
		fprintf(sdump->trace, "%s IS %s\n", field.name, pfield.name);
	} else {
		if (!skip_is) {
			StartElement(sdump, "IS");
			EndElementHeader(sdump, 1);
			sdump->indent++;
		}
		DUMP_IND(sdump);
		fprintf(sdump->trace, "<connect nodeField=\"%s\" protoField=\"%s\"/>\n", field.name, pfield.name);
		if (!skip_is) {
			sdump->indent--;
			EndElement(sdump, "IS", 1);
		}
	}
}
const char *SD_GetNodeName(SceneDumper *sdump, SFNode *node)
{
	/*convert whatever possible*/
	if (sdump->X3DDump) {
		if (node->sgprivate->tag == TAG_MPEG4_Circle) return "Circle2D";
		else if (node->sgprivate->tag == TAG_MPEG4_Rectangle) return "Rectangle2D";
	} else if (!sdump->X3DDump) {
		if (node->sgprivate->tag == TAG_X3D_Circle2D) return "Circle";
		else if (node->sgprivate->tag == TAG_X3D_Rectangle2D) return "Rectangle";
	}
	return Node_GetName(node);
}

Bool SD_CanDumpNode(SceneDumper *sdump, SFNode *node)
{
	const char *name;
	u32 tag;

	if (node->sgprivate->tag==TAG_ProtoNode) return 1;

	if (sdump->X3DDump || (sdump->dump_mode==M4SM_DUMP_VRML)) {
		if (node->sgprivate->tag>=TAG_RANGE_FIRST_X3D) return 1;
		if (node->sgprivate->tag==TAG_MPEG4_Rectangle) return 1;
		if (node->sgprivate->tag==TAG_MPEG4_Circle) return 1;
		name = Node_GetName(node);
		tag = X3D_GetTagByName(name);
		return tag ? 1 : 0;
	} else {
		if (node->sgprivate->tag<=TAG_RANGE_LAST_MPEG4) return 1;
		if (node->sgprivate->tag==TAG_X3D_Rectangle2D) return 1;
		if (node->sgprivate->tag==TAG_X3D_Circle2D) return 1;
		name = Node_GetName(node);
		tag = MPEG4_GetTagByName(name);
		return tag ? 1 : 0;
	}
}

void DumpNode(SceneDumper *sdump, SFNode *node, Bool in_list, char *fieldContainer)
{
	u32 i, count, to_dump, sub_el, ID;
	u32 *def_fields;
	Bool isDEF, isScript, isProto, hasISed;
	char *name;
	SFNode *base;
	FieldInfo field, base_field;
	
	if (!node) {
		fprintf(sdump->trace, "NULL");
		return;
	}

	/*this dumper works only for VRML like graphs*/
	if (node->sgprivate->tag>TAG_RANGE_LAST_X3D) return;

	if (!SD_CanDumpNode(sdump, node)) {
		fprintf(stdout, "Warning: node %s not part of %s standard - removing\n", Node_GetName(node), sdump->X3DDump ? "X3D" : (sdump->dump_mode==M4SM_DUMP_VRML) ? "VRML" : "MPEG4");
		if (!in_list) fprintf(sdump->trace, "NULL");
		return;
	}

	name = (char *) SD_GetNodeName(sdump, node);
	isProto = (Node_GetTag(node) == TAG_ProtoNode) ? 1 : 0;
	ID = node->sgprivate->NodeID;
	isDEF = 0;
	if (ID) {
		isDEF = SD_IsDEFNode(sdump, node);
		if (!isDEF) {
			if (!sdump->XMLDump) {
				if (in_list) DUMP_IND(sdump);
				fprintf(sdump->trace, "USE ");
				DumpNodeID(sdump, node);
				if (in_list) fprintf(sdump->trace, "\n");
			} else {
				if (isProto) {
					StartElement(sdump, "ProtoInstance");
					StartAttribute(sdump, "name");
					fprintf(sdump->trace, "%s", name);
					EndAttribute(sdump);
				} else {
					StartElement(sdump, name);
				}
				StartAttribute(sdump, "USE");
				DumpNodeID(sdump, node);
				EndAttribute(sdump);
				EndElementHeader(sdump, 0);
			}
			return;
		}
	}

	/*get all fields*/
	count = Node_GetFieldCount(node);
	def_fields = malloc(sizeof(u32) * count);

	base = NULL;
	switch (Node_GetTag(node)) {
	case TAG_X3D_Script:
	case TAG_MPEG4_Script:
		isScript = 1;
		break;
	default:
		isScript = 0;
		break;
	}


	if (!isScript) {
		if (isProto) {
			base = Proto_CreateInstance(node->sgprivate->scenegraph, ((ProtoInstance *)node)->proto_interface);
		} else {
			base = SG_NewNode(node->sgprivate->scenegraph, node->sgprivate->tag);
		}
	}

	if (base) Node_Register(base, NULL);
	
	hasISed = 0;
	to_dump = sub_el = 0;
	for (i=0;i<count; i++) {
		if (isScript) {
			/*dyn script fields are complex types*/
			def_fields[i] = (i>2) ? 2 : 1;
		} else {
			def_fields[i] = 0;
		}

		Node_GetField(node, i, &field);

		if (sdump->current_proto) {
			if (SD_GetISedField(sdump, node, &field) != NULL) {
				def_fields[i] = 3;
				if ((field.fieldType == FT_SFNode) || (field.fieldType == FT_MFNode))
					def_fields[i] = sdump->XMLDump ? 4 : 3;
				/*in XMT the ISed is not an attribute*/
				if (sdump->XMLDump) sub_el++;
				to_dump++;
				hasISed = 1;
				continue;
			}
		}

		if (!isScript && ((field.eventType == ET_EventIn) || (field.eventType == ET_EventOut)) ) {
			continue;
		}
		/*proto instance in XMT lists all fields as elements*/
		if (sdump->XMLDump && isProto) {
			def_fields[i] = 2;
			to_dump++;
			sub_el++;
			continue;
		}
		switch (field.fieldType) {
		case FT_SFNode:
			if (* (SFNode **) field.far_ptr) {
				def_fields[i] = 2;
				to_dump++;
				sub_el++;
			}
			break;
		case FT_MFNode:
			if (ChainGetCount(* (Chain **) field.far_ptr) ) {
				def_fields[i] = 2;
				to_dump++;
				sub_el++;
			}
			break;
		case FT_SFCommandBuffer:
		{
			SFCommandBuffer *p = (SFCommandBuffer *)field.far_ptr;
			if (p->bufferSize || ChainGetCount(p->commandList)) {
				def_fields[i] = 2;
				to_dump++;
				sub_el++;
			}
		}
			break;
		default:
			if (isScript) {
				to_dump++;
			} else {
				Node_GetField(base, i, &base_field);
				if (!VRML_FieldsEqual(base_field.far_ptr, field.far_ptr, field.fieldType)) {
					def_fields[i] = 1;
					to_dump++;
				}
			}
			break;
		}
	}
	if (base) Node_Unregister(base, NULL);

	if (!to_dump) {
		if (in_list) DUMP_IND(sdump);
		if (!sdump->XMLDump) {
			if (isDEF) {
				fprintf(sdump->trace, "DEF ");
				DumpNodeID(sdump, node);
				fprintf(sdump->trace, " ");
			}
			fprintf(sdump->trace, "%s {}\n", name);
		} else {
			if (isDEF) {
				if (isProto) {
					fprintf(sdump->trace, "<ProtoInstance name=\"%s\" DEF=\"", name);
				} else {
					fprintf(sdump->trace, "<%s DEF=\"", name);
				}
				DumpNodeID(sdump, node);
				fprintf(sdump->trace, "\"/>\n");
			} else {
				if (isProto) {
					fprintf(sdump->trace, "<ProtoInstance name=\"%s\"/>\n", name);
				} else {
					fprintf(sdump->trace, "<%s/>\n", name);
				}
			}
		}
		free(def_fields);
		return;
	}

	if (!sdump->XMLDump) {
		if (in_list) DUMP_IND(sdump);
		if (isDEF) {
			fprintf(sdump->trace, "DEF ");
			DumpNodeID(sdump, node);
			fprintf(sdump->trace, " ");
		}
		fprintf(sdump->trace, "%s {\n", name);
	} else {
		if (isProto) {
			StartElement(sdump, "ProtoInstance");
			StartAttribute(sdump, "name");
			fprintf(sdump->trace, "%s", name);
			EndAttribute(sdump);
		} else {
			StartElement(sdump, name);
		}
		if (isDEF) {
			StartAttribute(sdump, "DEF");
			DumpNodeID(sdump, node);
			EndAttribute(sdump);
		}
	}

	sdump->indent ++;
	for (i=0;i<count; i++) {
		switch (def_fields[i]) {
		/*regular field*/
		case 1:
			Node_GetField(node, i, &field);
			if (!isScript) {
				DumpField(sdump, node, field);
			}
			/*special script dump case, static fields except url*/
			else if (i==1 || i==2) {
				if (*((SFBool *)field.far_ptr)) DumpField(sdump, node, field);
			}
			/*in bt first dump fields - in XMT first dump url*/
			else if (i && !sdump->XMLDump) {
				DumpDynField(sdump, node, field, 0);
			} else if (!i && sdump->XMLDump) {
				DumpField(sdump, node, field);
			}
			break;
		/*IS field*/
		case 3:
			if (sdump->XMLDump) break;
			Node_GetField(node, i, &field);
			DumpISField(sdump, node, field, isScript, 0);
			def_fields[i] = 0;
			break;
		default:
			break;
		}
	}
	if (fieldContainer) fprintf(sdump->trace, " fieldContainer=\"%s\"", fieldContainer);

	if (isScript) sub_el = 1;
	EndElementHeader(sdump, sub_el ? 1 : 0);
	
	if (sub_el) {
		/*dump all normal IS elements for XMT*/
		if (hasISed && sdump->XMLDump) {
			StartElement(sdump, "IS");
			EndElementHeader(sdump, 1);
			sdump->indent++;
		}
		for (i=0;i<count; i++) {
			if (def_fields[i]==3) {
				Node_GetField(node, i, &field);
				DumpISField(sdump, node, field, isScript, 1);
			}
		}
		if (hasISed && sdump->XMLDump) {
			sdump->indent--;
			EndElement(sdump, "IS", 1);
		}
		/*dump all sub elements and complex IS*/
		for (i=0;i<count; i++) {
			switch (def_fields[i]) {
			case 2:
				Node_GetField(node, i, &field);
				if (!isScript) {
					if (isProto && sdump->XMLDump) {
						DumpProtoField(sdump, node, field);
					} else {
						DumpField(sdump, node, field);
					}
				} else {
					/*X3D script metadata, NOT DYN*/
					if ((i==3) && (node->sgprivate->tag==TAG_X3D_Script) ) {
						if (*((SFNode **)field.far_ptr)) DumpField(sdump, node, field);
					} else {
						DumpDynField(sdump, node, field, 0);
					}
				}
				break;
			case 4:
				Node_GetField(node, i, &field);
				DumpISField(sdump, node, field, isScript, 0);
				break;
			}
		}
	}

	/*finally dump script - XMT dumping is broken!!*/
	if (isScript && !sdump->XMLDump) {
		Node_GetField(node, 0, &field);
		DumpField(sdump, node, field);
	} 

	sdump->indent --;
	if (!sdump->XMLDump && !in_list) {
		DUMP_IND(sdump);
		fprintf(sdump->trace, "}");
	} else {
		EndElement(sdump, isProto ? "ProtoInstance" : name, sub_el);
	}
	free(def_fields);
}

M4Err DumpMultipleIndexedReplace(SceneDumper *sdump, SGCommand *com)
{
	u32 i;
	FieldInfo field;
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);
	Node_GetField(com->node, inf->fieldIndex, &field);
	field.fieldType = inf->fieldType;

	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<Replace extended=\"indices\" atNode=\"");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, "\" atField=\"%s\">\n", field.name);
	} else {
		fprintf(sdump->trace, "MULTIPLEINDREPLACE ");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, ".%s [\n", field.name);
	}
	sdump->indent++;
	for (i=0; i<ChainGetCount(com->command_fields); i++) {
		inf = ChainGetEntry(com->command_fields, i);
		field.far_ptr = inf->field_ptr;

		DUMP_IND(sdump);
		if (sdump->XMLDump) {
			fprintf(sdump->trace, "<repValue position=\"%d\" ", inf->pos);
		} else {
			fprintf(sdump->trace, "%d BY ", inf->pos);
		}
		DumpFieldValue(sdump, field);
		if (sdump->XMLDump) {
			fprintf(sdump->trace, "/>");
		} else {
			fprintf(sdump->trace, "\n");
		}
	}
	sdump->indent--;
	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "</Replace>\n");
	} else {
		fprintf(sdump->trace, "]\n");
	}
	return M4OK;
}

M4Err DumpMultipleReplace(SceneDumper *sdump, SGCommand *com)
{
	u32 i;
	FieldInfo info;
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;

	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<Replace extended=\"fields\" atNode=\"");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, "\">\n");
		
		sdump->indent++;
		for (i=0; i<ChainGetCount(com->command_fields); i++) {
			inf = ChainGetEntry(com->command_fields, i);
			Node_GetField(com->node, inf->fieldIndex, &info);
			info.far_ptr = inf->field_ptr;

			DUMP_IND(sdump);
			if (VRML_GetSFType(info.fieldType) != FT_SFNode) {
				fprintf(sdump->trace, "<repField atField=\"%s\" ", info.name);
				DumpFieldValue(sdump, info);
				fprintf(sdump->trace, "/>\n");
			} else {
				fprintf(sdump->trace, "<repField>");
				DumpField(sdump, com->node, info);
				fprintf(sdump->trace, "</repField>\n");
			}
		}
		sdump->indent--;
		
		DUMP_IND(sdump);
		fprintf(sdump->trace, "</Replace>\n");
	} else {
		fprintf(sdump->trace, "MULTIPLEREPLACE ");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, " {\n");
		sdump->indent++;
		for (i=0; i<ChainGetCount(com->command_fields); i++) {
			inf = ChainGetEntry(com->command_fields, i);
			Node_GetField(com->node, inf->fieldIndex, &info);
			info.far_ptr = inf->field_ptr;
			DumpField(sdump, com->node, info);
		}
		sdump->indent--;
		DUMP_IND(sdump);
		fprintf(sdump->trace, "}\n");
	}
	return M4OK;
}

M4Err DumpGlobalQP(SceneDumper *sdump, SGCommand *com)
{
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);

	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<Replace extended=\"globalQuant\">\n");
	} else {
		fprintf(sdump->trace, "GLOBALQP ");
	}
	DumpNode(sdump, inf->new_node, 0, NULL);
	if (sdump->XMLDump) fprintf(sdump->trace, "</Replace>\n");
	else fprintf(sdump->trace, "\n");
	return M4OK;
}

M4Err DumpNodeInsert(SceneDumper *sdump, SGCommand *com)
{
	CommandFieldInfo *inf;
	char posname[20];
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);

	switch (inf->pos) {
	case 0:
		strcpy(posname, "BEGIN");
		break;
	case -1:
		strcpy(posname, "END");
		break;
	default:
		sprintf(posname, "%d", inf->pos);
		break;
	}

	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<Insert atNode=\"");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, "\" position=\"%s\">", posname);
	} else {
		if (inf->pos==-1) { fprintf(sdump->trace, "APPEND TO "); }
		else fprintf(sdump->trace, "INSERT AT ");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, ".children");
		if (inf->pos!=-1) fprintf(sdump->trace, "[%d]", inf->pos);
		fprintf(sdump->trace, " ");
	}

	DumpNode(sdump, inf->new_node, 0, NULL);
	if (sdump->XMLDump) fprintf(sdump->trace, "</Insert>");
	fprintf(sdump->trace, "\n");
	return M4OK;
}

M4Err DumpRouteInsert(SceneDumper *sdump, SGCommand *com, Bool is_scene_replace)
{
	Route r;

	memset(&r, 0, sizeof(Route));
	r.ID = com->RouteID;
	r.name = com->def_name;
	r.FromNode = SD_FindNode(sdump, com->fromNodeID);
	r.FromFieldIndex = com->fromFieldIndex;
	r.ToNode = SD_FindNode(sdump, com->toNodeID);
	r.ToFieldIndex = com->toFieldIndex;

	if (is_scene_replace) {
		DumpRoute(sdump, &r, 0);
	} else {
		DUMP_IND(sdump);
		if (sdump->XMLDump) {
			fprintf(sdump->trace, "<Insert>\n");
		} else {
			fprintf(sdump->trace, "INSERT ");
		}
		DumpRoute(sdump, &r, 2);
		if (sdump->XMLDump) fprintf(sdump->trace, "</Insert>");
	}
	return M4OK;
}

M4Err DumpIndexInsert(SceneDumper *sdump, SGCommand *com)
{
	M4Err e;
	FieldInfo field, sffield;
	CommandFieldInfo *inf;
	char posname[20];
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);

	switch (inf->pos) {
	case 0:
		strcpy(posname, "BEGIN");
		break;
	case -1:
		strcpy(posname, "END");
		break;
	default:
		sprintf(posname, "%d", inf->pos);
		break;
	}

	e = Node_GetField(com->node, inf->fieldIndex, &field);
	if (e) return e;
	if (VRML_IsSFField(field.fieldType)) return M4NonCompliantBitStream;

	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<Insert atNode=\"");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, "\" atField=\"%s\" position=\"%s\"", field.name, posname);
	} else {
		if (inf->pos==-1) { fprintf(sdump->trace, "APPEND TO "); }
		else fprintf(sdump->trace, "INSERT AT ");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, ".%s", field.name);
		if (inf->pos!=-1) fprintf(sdump->trace, "[%d]", inf->pos);
		fprintf(sdump->trace, " ");
	}
	
	memcpy(&sffield, &field, sizeof(FieldInfo));
	sffield.fieldType = VRML_GetSFType(field.fieldType);

	if (field.fieldType==FT_MFNode) {
		if (sdump->XMLDump) fprintf(sdump->trace, ">\n");
		DumpNode(sdump, inf->new_node, 0, NULL);
		if (sdump->XMLDump) fprintf(sdump->trace, "</Insert>");
		fprintf(sdump->trace, "\n");
	} else {
		sffield.far_ptr = inf->field_ptr;
		DumpFieldValue(sdump, sffield);
		if (sdump->XMLDump) fprintf(sdump->trace, "/>");
		fprintf(sdump->trace, "\n");
	}
	return e;
}

M4Err DumpIndexDelete(SceneDumper *sdump, SGCommand *com)
{
	char posname[20];
	FieldInfo field;
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);

	switch (inf->pos) {
	case -1:
		strcpy(posname, sdump->XMLDump ? "END" : "LAST");
		break;
	case 0:
		strcpy(posname, "BEGIN");
		break;
	default:
		sprintf(posname, "%d", inf->pos);
		break;
	}

	Node_GetField(com->node, inf->fieldIndex, &field);

	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<Delete atNode=\"");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, "\" atField=\"%s\" position=\"%s\"/>", field.name, posname);
	} else {
		fprintf(sdump->trace, "DELETE ");
		if (inf->pos==-1) fprintf(sdump->trace, "%s ", posname);
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, ".%s", field.name);
		if (inf->pos!=-1) fprintf(sdump->trace, "[%d]", inf->pos);
		fprintf(sdump->trace, "\n");
	}
	return M4OK;
}


M4Err DumpNodeDelete(SceneDumper *sdump, SGCommand *com)
{
	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		if (com->tag==SG_NodeDeleteEx) {
			fprintf(sdump->trace, "<Delete extended=\"deleteOrder\" atNode=\"");
		} else {
			fprintf(sdump->trace, "<Delete atNode=\"");
		}
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, "\"/>\n");
	} else {
		if (com->tag==SG_NodeDeleteEx) fprintf(sdump->trace, "X");
		fprintf(sdump->trace, "DELETE ");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, "\n");
	}
	return M4OK;
}

M4Err DumpRouteDelete(SceneDumper *sdump, SGCommand *com)
{
	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<Delete atRoute=\"");
		DumpRouteID(sdump, com->RouteID, com->def_name);
		fprintf(sdump->trace, "\"/>\n");
	} else {
		fprintf(sdump->trace, "DELETE ROUTE ");
		DumpRouteID(sdump, com->RouteID, com->def_name);
		fprintf(sdump->trace, "\n");
	}
	return M4OK;
}



M4Err DumpNodeReplace(SceneDumper *sdump, SGCommand *com)
{
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);
	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<Replace atNode=\"");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, "\">");
		DumpNode(sdump, inf->new_node, 0, NULL);
		fprintf(sdump->trace, "</Replace>\n");
	} else {
		fprintf(sdump->trace, "REPLACE ");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, " BY ");
		DumpNode(sdump, inf->new_node, 0, NULL);
		fprintf(sdump->trace, "\n");
	}
	return M4OK;
}

M4Err DumpFieldReplace(SceneDumper *sdump, SGCommand *com)
{
	M4Err e;
	FieldInfo field;
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);

	e = Node_GetField(com->node, inf->fieldIndex, &field);

	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<Replace atNode=\"");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, "\" atField=\"%s\" ", field.name);
	} else {
		fprintf(sdump->trace, "REPLACE ");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, ".%s BY ", field.name);
	}

	switch (field.fieldType) {
	case FT_SFNode:
		if (sdump->XMLDump) fprintf(sdump->trace, ">");
		DumpNode(sdump, inf->new_node, 0, NULL);
		if (sdump->XMLDump) fprintf(sdump->trace, "</Replace>");
		else fprintf(sdump->trace, "\n");
		break;
	case FT_MFNode:
		{
			u32 i;
			if (sdump->XMLDump) {
				fprintf(sdump->trace, ">");
			} else {
				fprintf(sdump->trace, " [\n");
			}
			sdump->indent++;
			for (i=0; i<ChainGetCount(inf->node_list); i++) {
				SFNode *tmp = ChainGetEntry(inf->node_list, i);
				DumpNode(sdump, tmp, 1, NULL);
			}
			sdump->indent--;
			if (sdump->XMLDump) {
				fprintf(sdump->trace, "</Replace>");
			} else {
				EndList(sdump, NULL);
			}
		}
		break;
	case FT_SFCommandBuffer:
		break;
	default:
		field.far_ptr = inf->field_ptr;
		DumpFieldValue(sdump, field);
		if (sdump->XMLDump) fprintf(sdump->trace, "/>");
		fprintf(sdump->trace, "\n");
	}
	return e;
}


M4Err DumpIndexReplace(SceneDumper *sdump, SGCommand *com)
{
	char posname[20];
	M4Err e;
	FieldInfo field;
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);

	e = Node_GetField(com->node, inf->fieldIndex, &field);
	if (e) return e;
	if (VRML_IsSFField(field.fieldType)) return M4NonCompliantBitStream;

	switch (inf->pos) {
	case 0:
		strcpy(posname, "BEGIN");
		break;
	case -1:
		strcpy(posname, sdump->XMLDump ? "END" : "LAST");
		break;
	default:
		sprintf(posname, "%d", inf->pos);
		break;
	}

	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<Replace atNode=\"");
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, "\" atField=\"%s\" position=\"%s\"", field.name, posname);
	} else {
		fprintf(sdump->trace, "REPLACE ");
		if (inf->pos==-1) fprintf(sdump->trace, "%s ", posname);
		DumpNodeID(sdump, com->node);
		fprintf(sdump->trace, ".%s", field.name);
		if (inf->pos!=-1) fprintf(sdump->trace, "[%d]", inf->pos);
		fprintf(sdump->trace, " BY ");
	}

	if (field.fieldType == FT_MFNode) {
		if (sdump->XMLDump) fprintf(sdump->trace, ">\n");
		DumpNode(sdump, inf->new_node, 0, NULL);
		fprintf(sdump->trace, (sdump->XMLDump) ? "</Replace>\n" : "\n");
	} else {
		field.fieldType = VRML_GetSFType(field.fieldType);
		field.far_ptr = inf->field_ptr;
		DumpFieldValue(sdump, field);
		fprintf(sdump->trace, sdump->XMLDump ? "/>\n" : "\n");
	}
	return M4OK;
}


M4Err DumpRouteReplace(SceneDumper *sdump, SGCommand *com)
{
	const char *name;
	Route r2;

	if (!DumpFindRouteName(sdump, com->RouteID, &name)) return M4BadParam;

	memset(&r2, 0, sizeof(Route));
	r2.FromNode = SD_FindNode(sdump, com->fromNodeID);
	r2.FromFieldIndex = com->fromFieldIndex;
	r2.ToNode = SD_FindNode(sdump, com->toNodeID);
	r2.ToFieldIndex = com->toFieldIndex;

	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<Replace atRoute=\"");
		DumpRouteID(sdump, com->RouteID, (char *) name);
		fprintf(sdump->trace, "\">\n");
	} else {
		fprintf(sdump->trace, "REPLACE ROUTE ");
		DumpRouteID(sdump, com->RouteID, (char *) name);
		fprintf(sdump->trace, " BY ");
	}
	DumpRoute(sdump, &r2, 1);
	if (sdump->XMLDump ) fprintf(sdump->trace, "</Replace>");
	return M4OK;
}

M4Err DumpRoute(SceneDumper *sdump, Route *r, u32 dump_type)
{
	char toNode[512], fromNode[512];
	if (!r->is_setup) {
		Node_GetField(r->FromNode, r->FromFieldIndex, &r->FromField);
		Node_GetField(r->ToNode, r->ToFieldIndex, &r->ToField);
		r->is_setup = 1;
	}
	if (!r->FromNode || !r->ToNode) return M4BadParam;
	
	if (sdump->XMLDump || !dump_type) DUMP_IND(sdump);
	if (r->FromNode->sgprivate->NodeName) {
		strcpy(fromNode, r->FromNode->sgprivate->NodeName);
		strcpy(toNode, r->ToNode->sgprivate->NodeName);
	} else {
		sprintf(fromNode, "N%d", r->FromNode->sgprivate->NodeID-1);
		sprintf(toNode, "N%d", r->ToNode->sgprivate->NodeID-1);
	}
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<ROUTE");
		if (r->ID) {
			StartAttribute(sdump, "DEF");
			DumpRouteID(sdump, r->ID, r->name);
			EndAttribute(sdump);
		}
		fprintf(sdump->trace, " fromNode=\"%s\" fromField=\"%s\" toNode=\"%s\" toField=\"%s\"/>\n", fromNode, r->FromField.name, toNode, r->ToField.name);
	} else {
		if (dump_type==2) fprintf(sdump->trace, "ROUTE ");
		if (r->ID) {
			fprintf(sdump->trace, "DEF ");
			DumpRouteID(sdump, r->ID, r->name);
			fprintf(sdump->trace, " ");
		}
		if (dump_type==1) {
			fprintf(sdump->trace, "%s.%s TO %s.%s\n", fromNode, r->FromField.name, toNode, r->ToField.name);
		} else {
			if (dump_type!=2) fprintf(sdump->trace, "ROUTE ");
			fprintf(sdump->trace, "%s.%s TO %s.%s\n", fromNode, r->FromField.name, toNode, r->ToField.name);
		}
	}
	return M4OK;
}


M4Err DumpProtos(SceneDumper *sdump, Chain *protoList)
{
	u32 i, j, count;
	FieldInfo field;
	M4Err e;
	LPSCENEGRAPH prev_sg;
	LPPROTO proto, prev_proto;

	prev_proto = sdump->current_proto;

	for (i=0; i<ChainGetCount(protoList); i++) {
		proto = ChainGetEntry(protoList, i);

		sdump->current_proto = proto;
	
		DUMP_IND(sdump);
		if (!sdump->XMLDump) {
			fprintf(sdump->trace, proto->ExternProto.count ? "EXTERNPROTO " : "PROTO ");
			fprintf(sdump->trace, "%s [\n", proto->Name);
		} else {
			fprintf(sdump->trace, "<ProtoDeclare name=\"%s\" protoID=\"%d\"", proto->Name, proto->ID);
			if (proto->ExternProto.count) {
				fprintf(sdump->trace, " locations=\"");
				DumpSFField(sdump, FT_SFURL, &proto->ExternProto.vals[0], 0);
				fprintf(sdump->trace, "\"");
			}
			fprintf(sdump->trace, ">\n");
		}

		if (sdump->XMLDump && sdump->X3DDump) fprintf(sdump->trace, "<ProtoInterface>");

		sdump->indent++;
		count = ChainGetCount(proto->proto_fields);
		for (j=0; j<count; j++) {
			LPPROTOFIELD pf = ChainGetEntry(proto->proto_fields, j);
			field.fieldIndex = pf->ALL_index;
			field.eventType = pf->EventType;
			field.far_ptr = pf->default_value;
			field.fieldType = pf->FieldType;
			field.name = pf->FieldName;
			field.NDTtype = NDT_SFWorldNode;
			field.on_event_in = NULL;
			
			DumpDynField(sdump, NULL, field, pf->QP_Type ? 1 : 0);

			if (!pf->QP_Type) continue;
			
			/*dump interface coding - BT/TXT extensions, not supported by any other tool*/
			sdump->indent++;
			DUMP_IND(sdump);
			if (sdump->XMLDump) {
				fprintf(sdump->trace, "<InterfaceCodingParameters quantCategoy=\"%s\"", SD_GetQuantCatName(pf->QP_Type));
			} else {
				fprintf(sdump->trace, "{QP %d", pf->QP_Type);
			}
			if (pf->QP_Type==QC_LINEAR_SCALAR) fprintf(sdump->trace, sdump->XMLDump ? " nbBits=\"%d\"" : " nbBits %d", pf->NumBits);
			if (pf->hasMinMax) {
				switch (pf->QP_Type) {
				case QC_LINEAR_SCALAR:
				case QC_COORD_INDEX:
					if (sdump->XMLDump) {
						fprintf(sdump->trace, " intMin=\"%d\" intMax=\"%d\"", *((SFInt32 *)pf->qp_min_value), *((SFInt32 *)pf->qp_max_value));
					} else {
						fprintf(sdump->trace, " b {%d %d}", *((SFInt32 *)pf->qp_min_value), *((SFInt32 *)pf->qp_max_value));
					}
					break;
				default:
					if (sdump->XMLDump) {
						fprintf(sdump->trace, " floatMin=\"%g\" floatMax=\"%g\"", *((SFFloat *)pf->qp_min_value), *((SFFloat *)pf->qp_max_value));
					} else {
						fprintf(sdump->trace, " b {%g %g}", *((SFFloat *)pf->qp_min_value), *((SFFloat *)pf->qp_max_value));
					}
					break;
				}
			}
			fprintf(sdump->trace, sdump->XMLDump ? "/>\n" : "}\n");
			sdump->indent--;
			if (sdump->XMLDump) {
				DUMP_IND(sdump);
				fprintf(sdump->trace, "</field>\n");
			}

		}
		
		sdump->indent--;
		DUMP_IND(sdump);
		if (!sdump->XMLDump) {
			fprintf(sdump->trace, "]");
		} else if (sdump->X3DDump) fprintf(sdump->trace, "</ProtoInterface>\n");

		if (proto->ExternProto.count) {
			if (!sdump->XMLDump) {
				fprintf(sdump->trace, " \"");
				DumpSFField(sdump, FT_SFURL, &proto->ExternProto.vals[0], 0);
				fprintf(sdump->trace, "\"\n\n");
			} else {
				fprintf(sdump->trace, "</ProtoDeclare>\n");
			}
			continue;
		}
		if (!sdump->XMLDump) fprintf(sdump->trace, " {\n");

		sdump->indent++;

		if (sdump->XMLDump && sdump->X3DDump) fprintf(sdump->trace, "<ProtoBody>\n");
		
		e = DumpProtos(sdump, proto->sub_graph->protos);
		if (e) return e;

		/*set namespace to the proto one*/
		prev_sg = sdump->sg;
		sdump->sg = Proto_GetSceneGraph(proto);

		count = ChainGetCount(proto->node_code);
		for (j=0; j<count; j++) {
			SFNode *n = ChainGetEntry(proto->node_code, j);
			DumpNode(sdump, n, 1, NULL);
		}
		count = ChainGetCount(proto->sub_graph->Routes);
		for (j=0; j<count; j++) {
			Route *r = ChainGetEntry(proto->sub_graph->Routes, j);
			if (r->IS_route) continue;
			DumpRoute(sdump, r, 0);
		}

		if (sdump->XMLDump && sdump->X3DDump) fprintf(sdump->trace, "</ProtoBody>\n");

		/*restore namespace*/
		sdump->sg = prev_sg;

		sdump->indent--;
		DUMP_IND(sdump);
		if (!sdump->XMLDump) {
			fprintf(sdump->trace, "}\n");
		} else {
			fprintf(sdump->trace, "</ProtoDeclare>\n");
		}
	}
	sdump->current_proto = prev_proto;
	return M4OK;
}

M4Err DumpSceneReplace(SceneDumper *sdump, SGCommand *com)
{
	if (sdump->XMLDump) {
		if (!sdump->X3DDump) {
			StartElement(sdump, "Replace");
			EndElementHeader(sdump, 1);
			sdump->indent++;
		}
		StartElement(sdump, "Scene");
		if (!sdump->X3DDump && com->use_names) DumpBool(sdump, "USENAMES", com->use_names);
		EndElementHeader(sdump, 1);
		sdump->indent++;
	} else {
		if (!sdump->skip_scene_replace) {
			DUMP_IND(sdump);
			fprintf(sdump->trace, "REPLACE SCENE BY ");
		}
	}
	DumpProtos(sdump, com->new_proto_list);
	DumpNode(sdump, com->node, 0, NULL);
	if (!sdump->XMLDump) fprintf(sdump->trace, "\n\n");

	return M4OK;
}

M4Err DumpProtoInsert(SceneDumper *sdump, SGCommand *com)
{
	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "<Insert extended=\"proto\">\n");
	} else {
		fprintf(sdump->trace, "INSERTPROTO [\n");
	}
	sdump->indent++;
	DumpProtos(sdump, com->new_proto_list);
	sdump->indent--;
	DUMP_IND(sdump);
	if (sdump->XMLDump) {
		fprintf(sdump->trace, "</Insert>\n");
	} else {
		fprintf(sdump->trace, "]\n");
	}
	return M4OK;
}


M4Err SD_DumpCommandList(SceneDumper *sdump, Chain *comList, u32 indent, Bool skip_first_replace)
{
	M4Err e;
	u32 i, count;
	u32 prev_ind, remain;
	Bool prev_skip, has_scene_replace;

	if (!sdump || !sdump->trace|| !comList || !sdump->sg) return M4BadParam;

	prev_skip = sdump->skip_scene_replace;
	sdump->skip_scene_replace = skip_first_replace;
	prev_ind  = sdump->indent;
	sdump->indent = indent;
	has_scene_replace = 0;

	remain = 0;
	e = M4OK;
	count = ChainGetCount(comList);
	for (i=0; i<count; i++) {
		SGCommand *com = ChainGetEntry(comList, i);
		if (i && !remain && (sdump->X3DDump || (sdump->dump_mode==M4SM_DUMP_VRML))) {
			fprintf(stdout, "Warning: MPEG-4 Commands found, not supported in %s - skipping\n", sdump->X3DDump ? "X3D" : "VRML");
			break;
		}
		if (has_scene_replace && (com->tag != SG_RouteInsert)) {
			has_scene_replace = 0;
			if (sdump->XMLDump) {
				sdump->indent--;
				EndElement(sdump, "Scene", 1);
				sdump->indent--;
				EndElement(sdump, "Replace", 1);
			}
		}
		switch (com->tag) {
		/*insert commands*/
		case SG_NodeInsert:
			e = DumpNodeInsert(sdump, com);
			break;
		case SG_IndexedInsert:
			e = DumpIndexInsert(sdump, com);
			break;
		case SG_RouteInsert:
			e = DumpRouteInsert(sdump, com, has_scene_replace);
			remain--;
			break;
		/*delete commands*/
		case SG_NodeDelete:
			e = DumpNodeDelete(sdump, com);
			break;
		case SG_IndexedDelete:
			e = DumpIndexDelete(sdump, com);
			break;
		case SG_RouteDelete:
			e = DumpRouteDelete(sdump, com);
			break;
		/*replace commands*/
		case SG_NodeReplace:
			e = DumpNodeReplace(sdump, com);
			break;
		case SG_FieldReplace:
			e = DumpFieldReplace(sdump, com);
			break;
		case SG_IndexedReplace:
			e = DumpIndexReplace(sdump, com);
			break;
		case SG_RouteReplace:
			e = DumpRouteReplace(sdump, com);
			break;
		case SG_SceneReplace:
			/*we don't support replace scene in conditional*/
			assert(!sdump->current_com_list);
			sdump->current_com_list = comList;
			e = DumpSceneReplace(sdump, com);
			sdump->current_com_list = NULL;
			has_scene_replace = 1;
			remain = count - i - 1;
			break;
		/*extended commands*/
		case SG_ProtoInsert:
			e = DumpProtoInsert(sdump, com);
			break;
		case SG_ProtoDeleteAll:
			DUMP_IND(sdump);
			if (sdump->XMLDump) {
				fprintf(sdump->trace, "<Delete extended=\"allProtos\"/>\n");
			} else {
				fprintf(sdump->trace, "DELETEPROTO ALL\n");
			}
			e = M4OK;
			break;
		case SG_ProtoDelete:
		{
			u32 i;
			DUMP_IND(sdump);
			if (sdump->XMLDump) {
				fprintf(sdump->trace, "<Delete extended=\"protos\" value=\"");
			} else {
				fprintf(sdump->trace, "DELETEPROTO [");
			}
			for (i=0; i<com->del_proto_list_size; i++) {
				if (i) fprintf(sdump->trace, " ");
				fprintf(sdump->trace, "%d", com->del_proto_list[i]);
			}
			if (sdump->XMLDump) {
				fprintf(sdump->trace, "\"/>\n");
			} else {
				fprintf(sdump->trace, "]\n");
			}
			e = M4OK;
		}
			break;
		case SG_GlobalQuantizer:
			e = DumpGlobalQP(sdump, com);
			break;
		case SG_MultipleReplace:
			e = DumpMultipleReplace(sdump, com);
			break;
		case SG_MultipleIndexedReplace:
			e = DumpMultipleIndexedReplace(sdump, com);
			break;
		case SG_NodeDeleteEx:
			e = DumpNodeDelete(sdump, com);
			break;
		}
		if (e) break;


		if (!has_scene_replace && sdump->skip_scene_replace) {
			sdump->skip_scene_replace = 0;
			if (!sdump->XMLDump && (i+1<count)) {
				DUMP_IND(sdump);
				fprintf(sdump->trace, "\nAT 0 {\n");
				sdump->indent++;
			}
		}
	}

	if (remain && !sdump->XMLDump) {
		DUMP_IND(sdump);
		fprintf(sdump->trace, "}\n");
	}
	if (has_scene_replace && sdump->XMLDump) {
		sdump->indent--;
		EndElement(sdump, "Scene", 1);
		if (!sdump->X3DDump) {
			sdump->indent--;
			EndElement(sdump, "Replace", 1);
		}
	}

	sdump->indent = prev_ind;
	sdump->skip_scene_replace = prev_skip;
	return e;
}

M4Err SD_SetSceneGraph(SceneDumper *sdump, LPSCENEGRAPH sg)
{
	if (sdump) sdump->sg = sg;
	return M4OK;
}

M4Err SD_DumpGraph(SceneDumper *sdump, Bool skip_proto, Bool skip_routes)
{
	u32 tag;
	M4Err e = M4OK;
	if (!sdump->trace || !sdump->sg || !sdump->sg->RootNode) return M4BadParam;


	tag = sdump->sg->RootNode->sgprivate->tag;
	
	if (tag<=TAG_RANGE_LAST_X3D) {
		SD_SetupDump(sdump, NULL);

		if (sdump->XMLDump) {
			StartElement(sdump, "Scene");
			EndElementHeader(sdump, 1);
			sdump->indent++;
		}
		if (!skip_proto) {
			e = DumpProtos(sdump, sdump->sg->protos);
			if (e) return e;
		}

		if (sdump->X3DDump) {
			u32 i, count;
			SFParent *p = (SFParent *)sdump->sg->RootNode;
			count = ChainGetCount(p->children);
			for (i=0; i<count; i++) {
				SFNode *n = ChainGetEntry(p->children, i);
				DumpNode(sdump, n, 0, NULL);
			}
		} else {
			DumpNode(sdump, sdump->sg->RootNode, 0, NULL);
		}
		if (!sdump->XMLDump) fprintf(sdump->trace, "\n\n");
		if (!skip_routes) {
			u32 i;
			for (i=0; i<ChainGetCount(sdump->sg->Routes); i++) {
				Route *r = ChainGetEntry(sdump->sg->Routes, i);
				if (r->IS_route || (r->graph!=sdump->sg)) continue;
				e = DumpRoute(sdump, r, 0);
				if (e) return e;
			}
		}
		if (sdump->XMLDump) {
			sdump->indent--;
			EndElement(sdump, "Scene", 1);
		}
	
		SD_FinalizeDump(sdump);
		return M4OK;
	}
	else if (tag<=TAG_RANGE_LAST_SVG) {
		return M4NotSupported;
	}
#ifdef M4_USE_LASeR
	else if (tag<= TAG_RANGE_LAST_LASeR) {
		void LASeR_DumpNode(FILE *xml, void *current_node);
		LASeR_DumpNode(sdump->trace, sdump->sg->RootNode);
	}
#endif

	return M4OK;
}




static void ReorderAUContext(Chain *sample_list, M4AUContext *au)
{
	u32 i;
	Bool has_base;
	/*this happens when converting from bt to xmt*/
	if (!au->timing_sec) {
		au->timing_sec = au->timing;
		assert(au->owner->timeScale);
		au->timing_sec /= au->owner->timeScale;
	}
	/*this happens when converting from xmt to bt*/
	if (!au->timing) {
		assert(au->owner->timeScale);
		au->timing = (u32) (au->timing_sec * au->owner->timeScale);
	}

	has_base = 0;
	for (i=0; i<ChainGetCount(sample_list); i++) {
		M4AUContext *ptr = ChainGetEntry(sample_list, i);
		if (
			/*time ordered*/
			(ptr->timing_sec > au->timing_sec) 
			/*set bifs first for first AU*/
			|| (!has_base && (ptr->timing_sec == au->timing_sec) && (ptr->owner->streamType < au->owner->streamType) )
		) {
			ChainInsertEntry(sample_list, au, i);
			return;
		}

		has_base = 0;
		if ( (ptr->owner->streamType == au->owner->streamType) && (ptr->timing_sec == au->timing_sec) ) has_base = 1;
	}
	ChainAddEntry(sample_list, au);
}

M4Err M4SM_DumpToText(M4SceneManager *ctx, char *rad_name, u32 dump_mode)
{
	M4Err e;
	Chain *sample_list;
	Bool first_par;
	u32 i, j, indent, num_bifs, num_od, first_bifs, num_tracks;
	Double time;
	LPSCENEDUMPER dumper;

	sample_list = NewChain();

	num_bifs = num_od = 0;
	num_tracks = 0;

	/*configure all systems streams we're dumping*/
	for (i=0; i<ChainGetCount(ctx->streams); i++) {
		M4StreamContext *sc = ChainGetEntry(ctx->streams, i);

		switch (sc->streamType) {
		case M4ST_SCENE:
			num_bifs ++;
			num_tracks ++;
			break;
		case M4ST_OD:
			num_od ++;
			num_tracks ++;
			break;
		default:
			continue;
		}
		
		for (j=0; j<ChainGetCount(sc->AUs); j++) {
			M4AUContext *au = ChainGetEntry(sc->AUs, j);
			ReorderAUContext(sample_list, au);
		}
	}

	num_bifs = (num_bifs>1) ? 1 : 0;
	num_od = (num_od>1) ? 1 : 0;

	fprintf(stdout, "File loaded - dumping %d systems tracks / %d samples\n", num_tracks, ChainGetCount(sample_list));


	indent = 0;
	dumper = NewSceneDumper(ctx->scene_graph, rad_name, ' ', dump_mode);
	SD_SetupDump(dumper, (Descriptor *) ctx->root_od);

	time = 0;
	first_par = 0;
	first_bifs = 1;

	while (ChainGetCount(sample_list)) {
		M4AUContext *au = ChainGetEntry(sample_list, 0);
		ChainDeleteEntry(sample_list, 0);

		if (!dumper->XMLDump) {
		
			if (!first_bifs || (au->owner->streamType != M4ST_SCENE) ) {
				if (au->is_rap) fprintf(dumper->trace, "RAP ");
				fprintf(dumper->trace, "AT %d ", au->timing);
				if ( (au->owner->streamType==M4ST_OD && num_od) || (au->owner->streamType==M4ST_SCENE && num_bifs)) {
					fprintf(dumper->trace, "IN %d ", au->owner->ESID);
				} 
				fprintf(dumper->trace, "{\n");
				indent++;
			}

			switch (au->owner->streamType) {
			case M4ST_OD:
				e = OD_DumpCommandList(au->commands, dumper->trace, indent+1, 0);
				break;
			case M4ST_SCENE:
				e = SD_DumpCommandList(dumper, au->commands, indent, first_bifs);
				break;
			}
			if (first_bifs) {
				first_bifs = 0;
				fprintf(dumper->trace, "\n");

			} else {
				indent--;
				fprintf(dumper->trace, "}\n\n");
			}
		} else {
			if (!time && !num_bifs && first_bifs) {
			} else if (num_bifs || num_od) {
				if (!first_par) {
					first_par = 1;
					indent += 1;
				} else {
					fprintf(dumper->trace, " </par>\n");
				}
				fprintf(dumper->trace, " <par begin=\"%.3f\" atES_ID=\"es%d\">\n", au->timing_sec, au->owner->ESID);
			} else if (au->timing_sec>time) {
				if (!first_par) {
					first_par = 1;
					indent += 1;
				} else {
					fprintf(dumper->trace, " </par>\n");
				}
				fprintf(dumper->trace, "<par begin=\"%.3f\">\n", au->timing_sec);
			}
			switch (au->owner->streamType) {
			case M4ST_OD:
				e = OD_DumpCommandList(au->commands, dumper->trace, indent+1, 1);
				break;
			case M4ST_SCENE:
				assert(ChainGetCount(au->commands));
				e = SD_DumpCommandList(dumper, au->commands, indent+1, first_bifs);
				first_bifs = 0;
				break;
			}
			time = au->timing_sec;
		}
		if (dumper->X3DDump || (dumper->dump_mode==M4SM_DUMP_VRML)) break;
	}
	/*close command*/
	if (!dumper->X3DDump && first_par) fprintf(dumper->trace, " </par>\n");

	if (ChainGetCount(sample_list) && (dumper->X3DDump || (dumper->dump_mode==M4SM_DUMP_VRML)) ) {
		fprintf(stdout, "Warning: MPEG-4 Commands found, not supported in %s - skipping\n", dumper->X3DDump ? "X3D" : "VRML");
	}

	SD_FinalizeDump(dumper);
	DeleteSceneDumper(dumper);
	DeleteChain(sample_list);
	return M4OK;
}

#endif

