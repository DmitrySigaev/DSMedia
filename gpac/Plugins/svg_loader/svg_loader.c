/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Cyril Concolato 2004
 *					All rights reserved
 *
 *  This file is part of GPAC / SVG Loader plugin
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

#include <gpac/m4_scenegraph_svg.h>

#ifndef M4_DISABLE_SVG


#include <stdio.h>
#include <string.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <gpac/m4_svg_nodes.h>

//#include <gpac/intern/m4_scenegraph_dev.h>
#include <gpac/intern/m4_render_tools.h>
#include <gpac/intern/m4_esm_dev.h>

#include <gpac/m4_author.h>


/*enable this if you want to link SVG loader against static libxml2 & iconv*/
#if 0
#if (_MSC_VER == 1200) && (WINVER < 0x0500)
long _ftol( double );
long _ftol2( double dblSource ) { return _ftol( dblSource ); }
#endif
#endif

typedef struct
{
	InlineScene *inline_scene;
	M4Client *app;

	/*cyril, this should not be needed with SVG, only 1 stream and 1 AU*/
	M4SceneManager *ctx;
	M4StreamContext *svg_es;
	M4AUContext *svg_au;


	xmlXPathContextPtr xpathCtx;

	u8 status; // 0 = not initialized, 1 = initialized

	M4Err last_error;

	char *fileName;

	Chain *ided_nodes;

	u32 svg_w, svg_h;

	M4StreamContext *od_es;
	M4AUContext *od_au;

} SVGParser;

Bool SVG_hasBeenIDed(SVGParser *parser, xmlChar *node_name)
{
	u32 i, count;
	count = ChainGetCount(parser->ided_nodes);
	for (i=0; i<count; i++) {
		SFNode *n = ChainGetEntry(parser->ided_nodes, i);
		const char *nname = Node_GetDefName(n);
		if (!strcmp(nname, node_name)) return 1;
	}
	return 0;
}

u32 svg_get_next_node_id(SVGParser *parser)
{
	u32 ID;
	LPSCENEGRAPH sc = parser->inline_scene->graph;
	ID = SG_GetNextAvailableNodeID(sc);
	if (parser->ctx && (ID>parser->ctx->max_node_id)) 
		parser->ctx->max_node_id = ID;
	return ID;
}

u32 svg_get_node_id(SVGParser *parser, xmlChar *nodename)
{
	SFNode *n;
	u32 ID;
	if (sscanf(nodename, "N%d", &ID) == 1) {
		ID ++;
		n = SG_FindNode(parser->inline_scene->graph, ID);
		if (n) {
			u32 nID = svg_get_next_node_id(parser);
			const char *nname = Node_GetDefName(n);
			Node_SetDEF(n, nID, nname);
		}
		if (parser->ctx && (parser->ctx->max_node_id<ID)) parser->ctx->max_node_id=ID;
	} else {
		ID = svg_get_next_node_id(parser);
	}
	return ID;
}

#define COLOR_CONVERT(color_name, nred, ngreen, nblue) \
	if (!strcmp(attribute_content, (color_name))) { \
		col->red = (Float) (nred) / 255.0f; \
		col->green = (Float) (ngreen) / 255.0f; \
		col->blue = (Float) (nblue) / 255.0f; \
	}

void svg_parse_named_color(SVGParser *parser, const char *name, SVG_SVGColor *col, xmlChar *attribute_content)
{
	COLOR_CONVERT("aliceblue",240, 248, 255)
	COLOR_CONVERT("antiquewhite",250, 235, 215)
	COLOR_CONVERT("aqua", 0, 255, 255)
	COLOR_CONVERT("aquamarine",127, 255, 212)
	COLOR_CONVERT("azure",240, 255, 255)
	COLOR_CONVERT("beige",245, 245, 220)
	COLOR_CONVERT("bisque",255, 228, 196)
	COLOR_CONVERT("black", 0, 0, 0)
	COLOR_CONVERT("blanchedalmond",255, 235, 205)
	COLOR_CONVERT("blue", 0, 0, 255)
	COLOR_CONVERT("blueviolet",138, 43, 226)
	COLOR_CONVERT("brown",165, 42, 42)
	COLOR_CONVERT("burlywood",222, 184, 135)
	COLOR_CONVERT("cadetblue", 95, 158, 160)
	COLOR_CONVERT("chartreuse",127, 255, 0)
	COLOR_CONVERT("chocolate",210, 105, 30)
	COLOR_CONVERT("coral",255, 127, 80)
	COLOR_CONVERT("lightpink",255, 182, 193)
	COLOR_CONVERT("lightsalmon",255, 160, 122)
	COLOR_CONVERT("lightseagreen", 32, 178, 170)
	COLOR_CONVERT("lightskyblue",135, 206, 250)
	COLOR_CONVERT("lightslategray",119, 136, 153)
	COLOR_CONVERT("lightslategrey",119, 136, 153)
	COLOR_CONVERT("lightsteelblue",176, 196, 222)
	COLOR_CONVERT("lightyellow",255, 255, 224)
	COLOR_CONVERT("lime", 0, 255, 0)
	COLOR_CONVERT("limegreen", 50, 205, 50)
	COLOR_CONVERT("linen",250, 240, 230)
	COLOR_CONVERT("magenta",255, 0, 255)
	COLOR_CONVERT("maroon",128, 0, 0)
	COLOR_CONVERT("mediumaquamarine",102, 205, 170)
	COLOR_CONVERT("mediumblue", 0, 0, 205)
	COLOR_CONVERT("mediumorchid",186, 85, 211)
	COLOR_CONVERT("cornflowerblue",100, 149, 237)
	COLOR_CONVERT("cornsilk",255, 248, 220)
	COLOR_CONVERT("crimson",220, 20, 60)
	COLOR_CONVERT("cyan", 0, 255, 255)
	COLOR_CONVERT("darkblue", 0, 0, 139)
	COLOR_CONVERT("darkcyan", 0, 139, 139)
	COLOR_CONVERT("darkgoldenrod",184, 134, 11)
	COLOR_CONVERT("darkgray",169, 169, 169)
	COLOR_CONVERT("darkgreen", 0, 100, 0)
	COLOR_CONVERT("darkgrey",169, 169, 169)
	COLOR_CONVERT("darkkhaki",189, 183, 107)
	COLOR_CONVERT("darkmagenta",139, 0, 139)
	COLOR_CONVERT("darkolivegreen", 85, 107, 47)
	COLOR_CONVERT("darkorange",255, 140, 0)
	COLOR_CONVERT("darkorchid",153, 50, 204)
	COLOR_CONVERT("darkred",139, 0, 0)
	COLOR_CONVERT("darksalmon",233, 150, 122)
	COLOR_CONVERT("darkseagreen",143, 188, 143)
	COLOR_CONVERT("darkslateblue", 72, 61, 139)
	COLOR_CONVERT("darkslategray", 47, 79, 79)
	COLOR_CONVERT("darkslategrey", 47, 79, 79)
	COLOR_CONVERT("darkturquoise", 0, 206, 209)
	COLOR_CONVERT("darkviolet",148, 0, 211)
	COLOR_CONVERT("deeppink",255, 20, 147)
	COLOR_CONVERT("deepskyblue", 0, 191, 255)
	COLOR_CONVERT("dimgray",105, 105, 105)
	COLOR_CONVERT("dimgrey",105, 105, 105)
	COLOR_CONVERT("dodgerblue", 30, 144, 255)
	COLOR_CONVERT("firebrick",178, 34, 34)
	COLOR_CONVERT("floralwhite",255, 250, 240)
	COLOR_CONVERT("forestgreen", 34, 139, 34)
	COLOR_CONVERT("fuchsia",255, 0, 255)
	COLOR_CONVERT("gainsboro",220, 220, 220)
	COLOR_CONVERT("ghostwhite",248, 248, 255)
	COLOR_CONVERT("gold",255, 215, 0)
	COLOR_CONVERT("goldenrod",218, 165, 32)
	COLOR_CONVERT("gray",128, 128, 128)
	COLOR_CONVERT("grey",128, 128, 128)
	COLOR_CONVERT("green", 0, 128, 0)
	COLOR_CONVERT("greenyellow",173, 255, 47)
	COLOR_CONVERT("honeydew",240, 255, 240)
	COLOR_CONVERT("hotpink",255, 105, 180)
	COLOR_CONVERT("indianred",205, 92, 92)
	COLOR_CONVERT("indigo", 75, 0, 130)
	COLOR_CONVERT("ivory",255, 255, 240)
	COLOR_CONVERT("khaki",240, 230, 140)
	COLOR_CONVERT("lavender",230, 230, 250)
	COLOR_CONVERT("lavenderblush",255, 240, 245)
	COLOR_CONVERT("mediumpurple",147, 112, 219)
	COLOR_CONVERT("mediumseagreen", 60, 179, 113)
	COLOR_CONVERT("mediumslateblue",123, 104, 238)
	COLOR_CONVERT("mediumspringgreen", 0, 250, 154)
	COLOR_CONVERT("mediumturquoise", 72, 209, 204)
	COLOR_CONVERT("mediumvioletred",199, 21, 133)
	COLOR_CONVERT("midnightblue", 25, 25, 112)
	COLOR_CONVERT("mintcream",245, 255, 250)
	COLOR_CONVERT("mistyrose",255, 228, 225)
	COLOR_CONVERT("moccasin",255, 228, 181)
	COLOR_CONVERT("navajowhite",255, 222, 173)
	COLOR_CONVERT("navy", 0, 0, 128)
	COLOR_CONVERT("oldlace",253, 245, 230)
	COLOR_CONVERT("olive",128, 128, 0)
	COLOR_CONVERT("olivedrab",107, 142, 35)
	COLOR_CONVERT("orange",255, 165, 0)
	COLOR_CONVERT("orangered",255, 69, 0)
	COLOR_CONVERT("orchid",218, 112, 214)
	COLOR_CONVERT("palegoldenrod",238, 232, 170)
	COLOR_CONVERT("palegreen",152, 251, 152)
	COLOR_CONVERT("paleturquoise",175, 238, 238)
	COLOR_CONVERT("palevioletred",219, 112, 147)
	COLOR_CONVERT("papayawhip",255, 239, 213)
	COLOR_CONVERT("peachpuff",255, 218, 185)
	COLOR_CONVERT("peru",205, 133, 63)
	COLOR_CONVERT("pink",255, 192, 203)
	COLOR_CONVERT("plum",221, 160, 221)
	COLOR_CONVERT("powderblue",176, 224, 230)
	COLOR_CONVERT("purple",128, 0, 128)
	COLOR_CONVERT("red",255, 0, 0)
	COLOR_CONVERT("rosybrown",188, 143, 143)
	COLOR_CONVERT("royalblue", 65, 105, 225)
	COLOR_CONVERT("saddlebrown",139, 69, 19)
	COLOR_CONVERT("salmon",250, 128, 114)
	COLOR_CONVERT("sandybrown",244, 164, 96)
	COLOR_CONVERT("seagreen", 46, 139, 87)
	COLOR_CONVERT("seashell",255, 245, 238)
	COLOR_CONVERT("sienna",160, 82, 45)
	COLOR_CONVERT("silver",192, 192, 192)
	COLOR_CONVERT("skyblue",135, 206, 235)
	COLOR_CONVERT("slateblue",106, 90, 205)
	COLOR_CONVERT("slategray",112, 128, 144)
	COLOR_CONVERT("slategrey",112, 128, 144)
	COLOR_CONVERT("snow",255, 250, 250)
	COLOR_CONVERT("springgreen", 0, 255, 127)
	COLOR_CONVERT("steelblue", 70, 130, 180)
	COLOR_CONVERT("tan",210, 180, 140)
	COLOR_CONVERT("teal", 0, 128, 128)
	COLOR_CONVERT("lawngreen",124, 252, 0)
	COLOR_CONVERT("lemonchiffon",255, 250, 205)
	COLOR_CONVERT("lightblue",173, 216, 230)
	COLOR_CONVERT("lightcoral",240, 128, 128)
	COLOR_CONVERT("lightcyan",224, 255, 255)
	COLOR_CONVERT("lightgoldenrodyellow",250, 250, 210)
	COLOR_CONVERT("lightgray",211, 211, 211)
	COLOR_CONVERT("lightgreen",144, 238, 144)
	COLOR_CONVERT("lightgrey",211, 211, 211)
	COLOR_CONVERT("thistle",216, 191, 216)
	COLOR_CONVERT("tomato",255, 99, 71)
	COLOR_CONVERT("turquoise", 64, 224, 208)
	COLOR_CONVERT("violet",238, 130, 238)
	COLOR_CONVERT("wheat",245, 222, 179)
	COLOR_CONVERT("white",255, 255, 255)
	COLOR_CONVERT("whitesmoke",245, 245, 245)
	COLOR_CONVERT("yellow",255, 255, 0)
	COLOR_CONVERT("yellowgreen",154, 205, 50)
}

void svg_parse_color(SVGParser *parser, const char *name, SVG_SVGColor *col, xmlChar *attribute_content)
{
	char *str = attribute_content;
	while (str[strlen(attribute_content)-1] == ' ') str[strlen(attribute_content)-1] = 0;
	while (*str != 0 && (*str == ' ' || *str == ',' || *str == ';')) str++;

	if (str[0]=='#') {
		u32 val;
		sscanf(str+1, "%x", &val);
		if (strlen(str) == 7) {
			col->red = (Float) ((val>>16) & 0xFF) / 255.0f;
			col->green = (Float) ((val>>8) & 0xFF) / 255.0f;
			col->blue = (Float) (val & 0xFF) / 255.0f;
		} else {
			col->red = (Float) ((val>>8) & 0xF) / 15.0f;
			col->green = (Float) ((val>>4) & 0xF) / 15.0f;
			col->blue = (Float) (val & 0xF) / 15.0f;
		}
	} else if (strstr(str, "rgb(")) {
		sscanf(str, "rgb(%f,%f,%f)", &(col->red),&(col->green),&(col->blue)); 
		col->red /= 255.0f;
		col->green /= 255.0f;
		col->blue /= 255.0f;
	} else if ((str[0] >= 'a' && str[0] <= 'z')
		|| (str[0] >= 'A' && str[0] <= 'Z')) {
		svg_parse_named_color(parser, name, col, str);
	} else {
		sscanf(str, "%f %f %f", &(col->red), &(col->green), &(col->blue)); 
	}

}

u32 svg_parse_coordinate(char *d, Float *f) 
{
	u32 i = 0;
	while (d[i] != 0 && (d[i] == ' ' || d[i] == ',' || d[i] == ';')) i++;
	if (!d[i]) goto end;
	if (d[i] == '+' || d[i] == '-') i++;
	while (d[i] >= '0' && d[i] <= '9' && d[i] != 0) i++;
	if (!d[i]) goto end;
	if (d[i] == '.') {
		i++;
		while (d[i] >= '0' && d[i] <= '9' && d[i] != 0) i++;
		if (!d[i]) goto end;
	}
	if (d[i] == 'e' || d[i] == 'E') {
		i++;
		if (d[i] == '+' || d[i] == '-') i++;
		while (d[i] >= '0' && d[i] <= '9' && d[i] != 0) i++;
	}
end:
	sscanf(d,"%f",f);
	while (d[i] != 0 && (d[i] == ' ' || d[i] == ',' || d[i] == ';')) i++;
	return i;
}

u32 svg_parse_time(char *d, SMIL_BeginOrEndValue *v) 
{
	Float ftime;
	u32 i = 0;
	char time[100];
	char *tmp;
	if (!strcmp(d, "indefinite")) {
		v->type = SMILBeginOrEnd_indefinite;
		return 10;
	} else if ( (tmp = strstr(d, "s")) ) {
		v->type = SMILBeginOrEnd_offset_value;
		memcpy(time, d, tmp - d);
		time[tmp - d]= 0;
		sscanf(time, "%fs", &ftime);
		v->clock_value = ftime;
		return (tmp - d +1);
	} else if ( (tmp = strstr(d, "begin")) ) {
		v->type = SMILBeginOrEnd_syncbase_value;
		return (tmp - d + 5);
	} else if ( (tmp = strstr(d, "end")) ) {
		v->type = SMILBeginOrEnd_syncbase_value;
		return (tmp - d + 3);
	} else if ((tmp = strstr(d, "click"))) {
		v->type = SMILBeginOrEnd_event_value;
		if ((tmp = strstr(tmp+5, "+"))) {
			i = svg_parse_coordinate(tmp+1, &ftime);
			v->clock_value = ftime;
			return (tmp - d + i+1);
		} else {
			return 5;
		}
	} else {
		i = svg_parse_coordinate(d, &ftime);
		v->clock_value = ftime;
		return i;
	}
}

void svg_parse_transform(SVGParser *parser, SVG_Matrix *mat, xmlChar *attribute_content) 
{
	char *str;
	u32 i;
	
	mx2d_init(*mat);
	
	str = attribute_content;
	i = 0;
	while (str[i] != 0) {
		while (str[i] == ' ') i++;
		if (strstr(str+i, "scale")==str+i) {
			Float sx, sy;
			SVG_Matrix tmp;
			i += 5;
			while(str[i] != 0 && str[i] == ' ') i++;
			if (str[i] == '(') {
				i++;
				i+=svg_parse_coordinate(&(str[i]), &sx);
				if (str[i] == ')') {
					sy = sx;
				} else {
					i+=svg_parse_coordinate(&(str[i]), &sy);
				}
				i++;
			}
			mx2d_init(tmp);
			mx2d_add_scale(&tmp, sx, sy);
			mx2d_add_matrix(&tmp, mat);
			mx2d_copy(*mat, tmp);
			while(str[i] != 0 && str[i] == ' ') i++;
		} else if (strstr(str+i, "translate")==str+i) {
			Float tx, ty;
			SVG_Matrix tmp;
			i += 9;
			while(str[i] != 0 && str[i] == ' ') i++;
			if (str[i] == '(') {
				i++;
				i+=svg_parse_coordinate(&(str[i]), &tx);
				if (str[i] == ')') {
					ty = 0;
				} else {
					i+=svg_parse_coordinate(&(str[i]), &ty);
				}
				i++;
			}
			mx2d_init(tmp);
			mx2d_add_translation(&tmp, tx, ty);
			mx2d_add_matrix(&tmp, mat);
			mx2d_copy(*mat, tmp);
			while(str[i] != 0 && str[i] == ' ') i++;
		} else if (strstr(str+i, "rotate")==str+i) {
			Float angle;
			SVG_Matrix tmp;
			i += 6;
			while(str[i] != 0 && str[i] == ' ') i++;
			if (str[i] == '(') {
				i++;
				i+=svg_parse_coordinate(&(str[i]), &angle);
				angle /= (Float)180;
				angle *= (Float)3.14159;
				i++;
			}
			mx2d_init(tmp);
			mx2d_add_rotation(&tmp, 0, 0, angle);
			mx2d_add_matrix(&tmp, mat);
			mx2d_copy(*mat, tmp);
			while(str[i] != 0 && str[i] == ' ') i++;
		} else if (strstr(str+i, "skewX")==str+i) {
			Float angle;
			SVG_Matrix tmp;
			i += 5;
			while(str[i] != 0 && str[i] == ' ') i++;
			if (str[i] == '(') {
				i++;
				i+=svg_parse_coordinate(&(str[i]), &angle);
				angle /= 180;
				angle *= 3.14159f;
				i++;
			}
			mx2d_init(tmp);
			mx2d_add_skewX(&tmp, angle);
			mx2d_add_matrix(&tmp, mat);
			mx2d_copy(*mat, tmp);
			while(str[i] != 0 && str[i] == ' ') i++;
		} else if (strstr(str+i, "skewY")==str+i) {
			Float angle;
			SVG_Matrix tmp;
			i += 5;
			while(str[i] != 0 && str[i] == ' ') i++;
			if (str[i] == '(') {
				i++;
				i+=svg_parse_coordinate(&(str[i]), &angle);
				angle /= 180;
				angle *= 3.14159f;
				i++;
			}
			mx2d_init(tmp);
			mx2d_add_skewY(&tmp, angle);
			mx2d_add_matrix(&tmp, mat);
			mx2d_copy(*mat, tmp);
			while(str[i] != 0 && str[i] == ' ') i++;
		} else if (strstr(str+i, "matrix")==str+i) {
			SVG_Matrix tmp;
			i+=6;
			while(str[i] != 0 && str[i] == ' ') i++;
			if (str[i] == '(') {
				i++;
				i+=svg_parse_coordinate(&(str[i]), &(tmp.m[0]));
				i+=svg_parse_coordinate(&(str[i]), &(tmp.m[1]));
				i+=svg_parse_coordinate(&(str[i]), &(tmp.m[2]));
				i+=svg_parse_coordinate(&(str[i]), &(tmp.m[3]));
				i+=svg_parse_coordinate(&(str[i]), &(tmp.m[4]));
				i+=svg_parse_coordinate(&(str[i]), &(tmp.m[5]));
				i++;
			}
			mx2d_add_matrix(&tmp, mat);
			mx2d_copy(*mat, tmp);
			while(str[i] != 0 && str[i] == ' ') i++;
		}
	}
}


void svg_parse_path(SVGParser *parser, SVG_PathData *d_attribute, xmlChar *attribute_content) 
{
	Chain *d_commands = d_attribute->path_commands;
	Chain *d_points = d_attribute->path_points;
	char *d = attribute_content;
	Bool first_command = 1;
	if (strlen(d)) {
		Bool pt0_inited, subpath_closed;
		SVG_Point *pt, pt0, cur_pt;
		u8 *command;
		u32 i, k;
		char c, prev_c;
		i = 0;
		cur_pt.x = cur_pt.y = 0;
		pt0_inited = 0;
		subpath_closed = 0;
		while(1) {
			while ( (d[i]==' ') || (d[i] =='\t') ) i++;			
			c = d[i];
			if (c == 0) break;
next_command:
			switch (c) {
			case 'm':
				if (first_command) c = 'M'; 
			case 'M':
			case 'L':
			case 'l':
				i++;
				SAFEALLOC(command, sizeof(u8))
				ChainAddEntry(d_commands, command);
				if (c == 'M' || c == 'm') *command = 0;
				if (c == 'L' || c == 'l') {
					*command = 1;
					subpath_closed = 0;	
				}
				
				SAFEALLOC(pt, sizeof(SVG_Point))
				ChainAddEntry(d_points, pt);
				i += svg_parse_coordinate(&(d[i]), &(pt->x));
				i += svg_parse_coordinate(&(d[i]), &(pt->y));				
				if (c == 'm') {
					if (!subpath_closed) {
						pt->x += cur_pt.x;
						pt->y += cur_pt.y;
					} else {
						pt->x += pt0.x;
						pt->y += pt0.y;
					}
				} else if (c == 'l') {
					pt->x += cur_pt.x;
					pt->y += cur_pt.y;
				}
				cur_pt.x = pt->x;
				cur_pt.y = pt->y;
				if (!pt0_inited) {
					pt0.x = cur_pt.x;
					pt0.y = cur_pt.y;
					pt0_inited = 1;
				}
				prev_c = c;
				break;
			case 'H':
			case 'h':
				i++;				
				SAFEALLOC(command, sizeof(u8))
				ChainAddEntry(d_commands, command);
				*command = 1;
				subpath_closed = 0;

				SAFEALLOC(pt, sizeof(SVG_Point))
				ChainAddEntry(d_points, pt);
				i += svg_parse_coordinate(&(d[i]), &(pt->x));
				if (c == 'h') {
					pt->x += cur_pt.x;
				}
				pt->y = cur_pt.y;
				cur_pt.x = pt->x;
				prev_c = c;
				break;
			case 'V':
			case 'v':
				i++;				
				SAFEALLOC(command, sizeof(u8))
				ChainAddEntry(d_commands, command);
				*command = 1;
				subpath_closed = 0;

				SAFEALLOC(pt, sizeof(SVG_Point))
				ChainAddEntry(d_points, pt);
				i += svg_parse_coordinate(&(d[i]), &(pt->y));
				if (c == 'v') {
					pt->y += cur_pt.y;
				}
				pt->x = cur_pt.x;
				cur_pt.y = pt->y;
				prev_c = c;
				break;
			case 'C':
			case 'c':
				i++;				
				SAFEALLOC(command, sizeof(u8))
				ChainAddEntry(d_commands, command);
				*command = 2;
				subpath_closed = 0;
				
				for (k=0; k<3; k++) {
					SAFEALLOC(pt, sizeof(SVG_Point))
					ChainAddEntry(d_points, pt);
					i += svg_parse_coordinate(&(d[i]), &(pt->x));
					i += svg_parse_coordinate(&(d[i]), &(pt->y));				
					if (c == 'c') {
						pt->x += cur_pt.x;
						pt->y += cur_pt.y;
					}
				}				
				cur_pt.x = pt->x;
				cur_pt.y = pt->y;
				prev_c = c;
				break;
			case 'S':
			case 's':
				i++;				
				SAFEALLOC(command, sizeof(u8))
				ChainAddEntry(d_commands, command);
				*command = 3;
				subpath_closed = 0;
				
				for (k=0; k<2; k++) {
					SAFEALLOC(pt, sizeof(SVG_Point))
					ChainAddEntry(d_points, pt);
					i += svg_parse_coordinate(&(d[i]), &(pt->x));
					i += svg_parse_coordinate(&(d[i]), &(pt->y));				
					if (c == 's') {
						pt->x += cur_pt.x;
						pt->y += cur_pt.y;
					}
				}				
				cur_pt.x = pt->x;
				cur_pt.y = pt->y;
				prev_c = c;
				break;
			case 'Q':
			case 'q':
				i++;				
				SAFEALLOC(command, sizeof(u8))
				ChainAddEntry(d_commands, command);
				*command = 4;
				subpath_closed = 0;
				
				for (k=0; k<2; k++) {
					SAFEALLOC(pt, sizeof(SVG_Point))
					ChainAddEntry(d_points, pt);
					i += svg_parse_coordinate(&(d[i]), &(pt->x));
					i += svg_parse_coordinate(&(d[i]), &(pt->y));				
					if (c == 'q') {
						pt->x += cur_pt.x;
						pt->y += cur_pt.y;
					}
				}				
				cur_pt.x = pt->x;
				cur_pt.y = pt->y;
				prev_c = c;
				break;
			case 'T':
			case 't':
				i++;				
				SAFEALLOC(command, sizeof(u8))
				ChainAddEntry(d_commands, command);
				*command = 5;
				subpath_closed = 0;
				
				SAFEALLOC(pt, sizeof(SVG_Point))
				ChainAddEntry(d_points, pt);
				i += svg_parse_coordinate(&(d[i]), &(pt->x));
				i += svg_parse_coordinate(&(d[i]), &(pt->y));				
				if (c == 't') {
					pt->x += cur_pt.x;
					pt->y += cur_pt.y;
				}
				cur_pt.x = pt->x;
				cur_pt.y = pt->y;
				prev_c = c;
				break;
			case 'A':
			case 'a':
				{
					Float tmp;
					i++;				
					SAFEALLOC(command, sizeof(u8))
					ChainAddEntry(d_commands, command);
					*command = 7;
					subpath_closed = 0;
	
					SAFEALLOC(pt, sizeof(SVG_Point))
					ChainAddEntry(d_points, pt);
					i += svg_parse_coordinate(&(d[i]), &(pt->x));	
					i += svg_parse_coordinate(&(d[i]), &(pt->y));				

					i += svg_parse_coordinate(&(d[i]), &(tmp));	
					i += svg_parse_coordinate(&(d[i]), &(tmp));				
					i += svg_parse_coordinate(&(d[i]), &(tmp));	
					
					SAFEALLOC(pt, sizeof(SVG_Point))
					ChainAddEntry(d_points, pt);
					i += svg_parse_coordinate(&(d[i]), &(pt->x));	
					i += svg_parse_coordinate(&(d[i]), &(pt->y));				
					if (c == 'a') {
						pt->x += cur_pt.x;
						pt->y += cur_pt.y;
					}
					cur_pt.x = pt->x;
					cur_pt.y = pt->y;
				}
				prev_c = c;
				break;
			case 'Z':
			case 'z':
				i++;				
				SAFEALLOC(command, sizeof(u8))
				ChainAddEntry(d_commands, command);
				*command = 6;
				subpath_closed = 1;
				prev_c = c;
				break;
			default:
				i--;
				c = prev_c;
				goto next_command;
			}
			first_command = 0;
		}
	}
}

void svg_parse_paint(SVGParser *parser, const char * name, SVG_Paint *paint, char *attribute_content)
{
	if (!strcmp(attribute_content, "none")) {
		paint->paintType = SVG_PAINTTYPE_NONE;
	} else if (!strcmp(attribute_content, "inherit")) {
		paint->paintType = SVG_PAINTTYPE_INHERIT;
	} else {
		paint->paintType = SVG_PAINTTYPE_RGBCOLOR;
		svg_parse_color(parser, name, &(paint->color), attribute_content);
	}
}

void *svg_parse_one_anim_value(SVGParser *parser, const char *attribute_name, u8 anim_datatype, char *single_value_string)
{
	switch (anim_datatype) {
	case SVG_Paint_datatype:
		{
			SVG_Paint *paint;				
			SAFEALLOC(paint, sizeof(SVG_Paint))
			svg_parse_paint(parser, attribute_name, paint, single_value_string);
			return paint;
		}
		break;
	case SVG_StrokeWidthValue_datatype:
	case SVG_Length_datatype:
	case SVG_Coordinate_datatype:
		{
			SVG_Length *length;
			SAFEALLOC(length, sizeof(SVG_Length))
			if (strstr(single_value_string, "%")) length->unitType = SVG_LENGTHTYPE_PERCENTAGE;
			else length->unitType = SVG_LENGTHTYPE_PX;
			sscanf(single_value_string, "%f", &(length->number)); 
			return length;
		}
		break;
	} 
	return NULL;
}

void svg_parse_attribute(SVGParser *parser, FieldInfo *info, SVGElement *n, xmlChar *attribute_content, u8 anim_datatype)
{
	switch (info->fieldType) {
	case SVG_URI_datatype:
		{
			((SVG_URI*)info->far_ptr)->uri.string = strdup(attribute_content);
			if (attribute_content[0] == '#') {
				((SVG_URI*)info->far_ptr)->target_element = (SVGElement *)SG_FindNodeByName(parser->inline_scene->graph, &(attribute_content[1]));
			}
		}
		break;
	case SMIL_AttributeName_datatype:
		{
			FieldInfo xlink_href;
			SVGElement *targetElement = NULL;

			FieldInfo targetAttribute;
			FieldInfo *attributeName = (FieldInfo *)info->far_ptr;

			if (!Node_GetFieldByName((SFNode *)n, "xlink:href", &xlink_href)) {
				targetElement = ((SVG_URI *)xlink_href.far_ptr)->target_element;

				Node_GetFieldByName((SFNode *)targetElement, attribute_content, &targetAttribute);
				memcpy(attributeName, &targetAttribute, sizeof(FieldInfo));
			}
		}
		break;
	case SMIL_BeginOrEndValues_datatype:
		{
			// to be fixed
			u32 i;
			char *str;
			SMIL_BeginOrEndValues *beginOrEnd = (SMIL_BeginOrEndValues *)(info->far_ptr);
			u32 len;

			i = 0;
			str = attribute_content;
			len = strlen(str);
			while (i < len) {
				SMIL_BeginOrEndValue *v;
				SAFEALLOC(v, sizeof(SMIL_BeginOrEndValue))
				i+=svg_parse_time(&(str[i]), v);
				ChainAddEntry(*beginOrEnd, v);
			}
		}
		break;
	case SMIL_MinMaxDurRepeatDurValue_datatype:
		// To be fixed to align with the clock value
		{
			Float f;
			((SMIL_MinMaxDurRepeatDurValue *)info->far_ptr)->type = SMILMinMaxDurRepeatDur_clock_value;
			sscanf(attribute_content, "%f", &f);
			((SMIL_MinMaxDurRepeatDurValue *)info->far_ptr)->clock_value = f;
		}
		break;
	case SMIL_RepeatCountValue_datatype:
		if (!strcmp(attribute_content, "indefinite")) {
			*(SMIL_RepeatCountValue *)info->far_ptr = -1;
		} else {
			sscanf(attribute_content, "%f", (SMIL_RepeatCountValue *)info->far_ptr);
		}
		break;
	case SMIL_FreezeValue_datatype:
		if (!strcmp(attribute_content, "freeze")) {
			*((SMIL_FreezeValue *)info->far_ptr) = SMILFill_freeze;
		} else if (!strcmp(attribute_content, "remove")) {
			*((SMIL_FreezeValue *)info->far_ptr) = SMILFill_remove;
		}
		break;
	case SMIL_RestartValue_datatype:
		if (!strcmp(attribute_content, "always")) {
			*((SMIL_RestartValue *)info->far_ptr) = SMILRestart_always;
		} else if (!strcmp(attribute_content, "whenNotActive")) {
			*((SMIL_RestartValue *)info->far_ptr) = SMILRestart_whenNotActive;
		} else if (!strcmp(attribute_content, "never")) {
			*((SMIL_RestartValue *)info->far_ptr) = SMILRestart_never;
		}
		break;
	case SMIL_CalcModeValue_datatype:
		if (!strcmp(attribute_content, "discrete")) {
			*((SMIL_CalcModeValue *)info->far_ptr) = SMILCalcMode_discrete;
		} else if (!strcmp(attribute_content, "linear")) {
			*((SMIL_CalcModeValue *)info->far_ptr) = SMILCalcMode_linear;
		} else if (!strcmp(attribute_content, "paced")) {
			*((SMIL_CalcModeValue *)info->far_ptr) = SMILCalcMode_paced;
		} else if (!strcmp(attribute_content, "spline")) {
			*((SMIL_CalcModeValue *)info->far_ptr) = SMILCalcMode_spline;
		} 
		break;
	case SMIL_AdditiveValue_datatype:
		if (!strcmp(attribute_content, "replace")) {
			*((SMIL_AdditiveValue *)info->far_ptr) = SMILAdditiveValue_replace;
		} else if (!strcmp(attribute_content, "sum")) {
			*((SMIL_AdditiveValue *)info->far_ptr) = SMILAdditiveValue_sum;
		} 
		break;
	case SMIL_AccumulateValue_datatype:
		if (!strcmp(attribute_content, "none")) {
			*((SMIL_AccumulateValue *)info->far_ptr) = SMILAccumulateValue_none;
		} else if (!strcmp(attribute_content, "sum")) {
			*((SMIL_AccumulateValue *)info->far_ptr) = SMILAccumulateValue_sum;
		} 
		break;
	case SMIL_AnimateValue_datatype:
		{
			SMIL_AnimateValue *anim_value = (SMIL_AnimateValue *)info->far_ptr;
			anim_value->datatype = anim_datatype;
			anim_value->value = svg_parse_one_anim_value(parser, info->name, anim_value->datatype, attribute_content);
		}
		break;
	case SMIL_AnimateValues_datatype:
		{
			u32 len, i = 0;
			char *str;
			char *value_string;
			void *single_value;
			s32 psemi = -1;

			SMIL_AnimateValues *anim_values = (SMIL_AnimateValues *)info->far_ptr;
			anim_values->datatype = anim_datatype;
			
			len = strlen(attribute_content);
			str = attribute_content;
			for (i = 0; i < len+1; i++) {
				if (str[i] == ';' || str[i] == 0) {
					u32 single_value_len = 0;
					single_value_len = i - (psemi+1);
					SAFEALLOC(value_string, single_value_len+1);
					memcpy(value_string, str + (psemi+1), single_value_len);
					value_string[single_value_len] = 0;
					psemi = i;
					single_value = svg_parse_one_anim_value(parser, info->name, anim_values->datatype, value_string);
					if (single_value) ChainAddEntry(anim_values->values, single_value);
					free(value_string);
				}
			}
		}
		break;
	case SMIL_KeyTimesValues_datatype:
		{
			u32 i;
			char *str;
			SMIL_KeyTimesValues *keyTimes = (SMIL_KeyTimesValues *)(info->far_ptr);
			u32 len;

			i = 0;
			str = attribute_content;
			len = strlen(str);
			while (i < len) {
				Float *f;
				SAFEALLOC(f, sizeof(Float));
				i+=svg_parse_coordinate(&(str[i]), f);
				ChainAddEntry(*keyTimes, f);
			}
		}
		break;
	case SMIL_KeySplinesValues_datatype:
		{
			u32 i;
			char *str;
			SMIL_KeySplinesValues *keySplines = (SMIL_KeySplinesValues *)(info->far_ptr);
			u32 len;

			i = 0;
			str = attribute_content;
			len = strlen(str);
			while (i < len) {
				Float *f;
				SAFEALLOC(f, sizeof(Float));
				i+=svg_parse_coordinate(&(str[i]), f);
				ChainAddEntry(*keySplines, f);
			}
		}
		break;
	case SVG_PathData_datatype:
		{
			SVG_PathData *d = (SVG_PathData *)(info->far_ptr);
			svg_parse_path(parser, d, attribute_content);
		}
		break;
	case SVG_Points_datatype:
		{
			u32 i;
			char *str;
			SVG_Points *points = (SVG_Points *)(info->far_ptr);
			u32 len;

			i = 0;
			str = attribute_content;
			len = strlen(str);
			while (i < len) {
				SVG_Point *p;
				SAFEALLOC(p, sizeof(SVG_Point))
				i+=svg_parse_coordinate(&(str[i]), &(p->x));
				i+=svg_parse_coordinate(&(str[i]), &(p->y));
				ChainAddEntry(*points, p);
			}
		}
		break;
	case SVG_Coordinates_datatype:
		{
			u32 i;
			char *str;
			SVG_Coordinates *coords = (SVG_Coordinates *)(info->far_ptr);
			u32 len;

			i = 0;
			str = attribute_content;
			len = strlen(str);
			while (i < len) {
				Float *f;
				SAFEALLOC(f, sizeof(Float));
				i+=svg_parse_coordinate(&(str[i]), f);
				ChainAddEntry(*coords, f);
			}
		}
		break;
	case SVG_ViewBoxSpec_datatype:
		{
			char *str;
			u32 i;
			SVG_ViewBoxSpec *viewbox = (SVG_ViewBoxSpec *)info->far_ptr;
			str = attribute_content;
			i = 0;
			i+=svg_parse_coordinate(&(str[i]), &(viewbox->x));
			i+=svg_parse_coordinate(&(str[i]), &(viewbox->y));
			i+=svg_parse_coordinate(&(str[i]), &(viewbox->width));
			i+=svg_parse_coordinate(&(str[i]), &(viewbox->height));
		}
		break;
	case SVG_Boolean_datatype:
		if (!stricmp(attribute_content, "1") || !stricmp(attribute_content, "true"))
		  *(SVG_Boolean *)info->far_ptr = 1;
		else
		  *(SVG_Boolean *)info->far_ptr = 0;
	    break;
	case SVG_SVGColor_datatype:
		svg_parse_color(parser, info->name, (SVG_SVGColor *)info->far_ptr, attribute_content);
	    break;
	case SVG_Paint_datatype:
		svg_parse_paint(parser, info->name, info->far_ptr, attribute_content);
		break;
	case SVG_ClipFillRule_datatype:
		if (!strcmp(attribute_content, "inherit")) {
			*((SVG_ClipFillRule *)info->far_ptr) = SVGFillRule_inherit;
		} else if (!strcmp(attribute_content, "nonzero")) {
			*((SVG_ClipFillRule *)info->far_ptr) = SVGFillRule_nonzero;
		} else if (!strcmp(attribute_content, "evenodd")) {
			*((SVG_ClipFillRule *)info->far_ptr) = SVGFillRule_evenodd;
		} 
		break;
	case SVG_StrokeLineJoinValue_datatype:
		if (!strcmp(attribute_content, "inherit")) {
			*((SVG_StrokeLineJoinValue *)info->far_ptr) = SVGStrokeLineJoin_inherit;
		} else if (!strcmp(attribute_content, "miter")) {
			*((SVG_StrokeLineJoinValue *)info->far_ptr) = SVGStrokeLineJoin_miter;
		} else if (!strcmp(attribute_content, "round")) {
			*((SVG_StrokeLineJoinValue *)info->far_ptr) = SVGStrokeLineJoin_round;
		} else if (!strcmp(attribute_content, "bevel")) {
			*((SVG_StrokeLineJoinValue *)info->far_ptr) = SVGStrokeLineJoin_bevel;
		} 
		break;
	case SVG_StrokeLineCapValue_datatype:
		if (!strcmp(attribute_content, "inherit")) {
			*((SVG_StrokeLineCapValue *)info->far_ptr) = SVGStrokeLineCap_inherit;
		} else if (!strcmp(attribute_content, "butt")) {
			*((SVG_StrokeLineCapValue *)info->far_ptr) = SVGStrokeLineCap_butt;
		} else if (!strcmp(attribute_content, "round")) {
			*((SVG_StrokeLineCapValue *)info->far_ptr) = SVGStrokeLineCap_round;
		} else if (!strcmp(attribute_content, "square")) {
			*((SVG_StrokeLineCapValue *)info->far_ptr) = SVGStrokeLineCap_square;
		} 
		break;
	case SVG_FontFamilyValue_datatype:
		if (!strcmp(attribute_content, "inherit")) {
			((SVG_FontFamilyValue *)info->far_ptr)->type = SVGFontFamily_inherit;
		} else {
			((SVG_FontFamilyValue *)info->far_ptr)->type = SVGFontFamily_string;
			((SVG_FontFamilyValue *)info->far_ptr)->value.string = strdup(attribute_content);
			((SVG_FontFamilyValue *)info->far_ptr)->value.length = strlen(attribute_content);
		}
		break;
	case SVG_FontStyleValue_datatype:
		if (!strcmp(attribute_content, "inherit")) {
			*((SVG_FontStyleValue *)info->far_ptr) = SVGFontStyle_inherit;
		} else if (!strcmp(attribute_content, "normal")) {
			*((SVG_FontStyleValue *)info->far_ptr) = SVGFontStyle_normal;
		} else if (!strcmp(attribute_content, "italic")) {
			*((SVG_FontStyleValue *)info->far_ptr) = SVGFontStyle_italic;
		} else if (!strcmp(attribute_content, "oblique")) {
			*((SVG_FontStyleValue *)info->far_ptr) = SVGFontStyle_oblique;
		} 
		break;
	case SVG_OpacityValue_datatype:
	case SVG_FontSizeValue_datatype:
	case SVG_StrokeMiterLimitValue_datatype:
		if (!strcmp(attribute_content, "inherit")) {
			((SVGInheritableFloat *)info->far_ptr)->type = SVGFLOAT_INHERIT;
		} else {
			((SVGInheritableFloat *)info->far_ptr)->type = SVGFLOAT_VALUE;
			sscanf(attribute_content, "%f", &(((SVGInheritableFloat *)info->far_ptr)->value)); 
		}
		break;
	case SVG_StrokeWidthValue_datatype:
	case SVG_Length_datatype:
	case SVG_Coordinate_datatype:
		if (strstr(attribute_content, "%")) ((SVG_Length *)info->far_ptr)->unitType = SVG_LENGTHTYPE_PERCENTAGE;
		else ((SVG_Length *)info->far_ptr)->unitType = SVG_LENGTHTYPE_PX;
		sscanf(attribute_content, "%f", &(((SVG_Length *)info->far_ptr)->number)); 
		break;
	case SVG_TransformList_datatype:
		{
			SVG_Matrix *m;
			SVG_TransformList *tr = (SVG_TransformList *)info->far_ptr;
			SAFEALLOC(m, sizeof(SVG_Matrix));
			ChainAddEntry(*tr, m);
			svg_parse_transform(parser, m, attribute_content);
		}
		break;
	default:
		parser->last_error = M4NotSupported;
		break;    
	}
}

SVGElement *svg_parse_element(SVGParser *parser, xmlNodePtr node, SVGElement *parent)
{
	u32 tag;
	u32 i;
	u8 anim_datatype = 0;
	u32 ID = 0;
	Bool register_id;  
	SVGElement *elt, *unided_elt;
	xmlNodePtr children;
	xmlAttrPtr attributes;

	tag = SVG_GetTagByName(node->name);
	if (tag == TAG_UndefinedNode) {
		return NULL;
	}

	elt = SVG_NewNode(parser->inline_scene->graph, tag);
	if (!elt) {
		parser->last_error = M4UnknownNode;
		return NULL;
	}
	Node_Register((SFNode *)elt, (SFNode *)parent);

	if (tag == TAG_SVG_set ||
		tag == TAG_SVG_animate ||
		tag == TAG_SVG_animateColor ||
		tag == TAG_SVG_animateTransform ||
		tag == TAG_SVG_animateMotion) {
		u32 found = 0;
		FieldInfo xlink_href;
		if (!Node_GetFieldByName((SFNode *)elt, "xlink:href", &xlink_href)) {
			((SVG_URI *)xlink_href.far_ptr)->type = SVGUri_element_id;
			((SVG_URI *)xlink_href.far_ptr)->target_element = parent;
		}

		attributes = node->properties;
		while (attributes && !found) {
			if (attributes->type == XML_ATTRIBUTE_NODE) {
				if (!stricmp(attributes->name, "attributeName")) {
					for (i = 0; i<SVG_GetAttributeCount((SFNode *)elt); i++) {
						SVGAttributeInfo info;
						info.fieldIndex = i;
						SVG_GetAttributeInfo((SFNode *)elt, &info);
						if (!strcmp(info.name, attributes->name)) {
							svg_parse_attribute(parser, &info, elt, attributes->children->content, 0);
							anim_datatype = ((FieldInfo *)info.far_ptr)->fieldType;
							found = 1;
							break;
						}
					}
				}
			}
			attributes = attributes->next;
		}		
	}

	register_id = 0;
	attributes = node->properties;
	while (attributes) {
		if (attributes->type == XML_ATTRIBUTE_NODE) {
			char qname[100];
			if (attributes->ns) {
				sprintf(qname,"%s:%s", attributes->ns->prefix, attributes->name);
			} else {
				sprintf(qname,"%s", attributes->name);
			}
			if (!stricmp(qname, "id")) {
				xmlChar *nodename = attributes->children->content;//xmlGetProp(attributes, "id");
				register_id = 1;
				unided_elt = (SVGElement *)SG_FindNodeByName(parser->inline_scene->graph, nodename);
				if (unided_elt) {
					ID = Node_GetID( (SFNode *) unided_elt);
					if (SVG_hasBeenIDed(parser, nodename)) unided_elt = NULL;
				} else {
					ID = svg_get_node_id(parser, nodename);
				}
				Node_SetDEF((SFNode *)elt, ID, nodename);
				if (unided_elt) Node_ReplaceAllInstances((SFNode *)unided_elt, (SFNode *)elt, 0);
			} else if (!stricmp(qname, "attributeName")) {
				/* already dealt with above */
			} else {
				for (i = 0; i<SVG_GetAttributeCount((SFNode *)elt); i++) {
					SVGAttributeInfo info;
					info.fieldIndex = i;
					SVG_GetAttributeInfo((SFNode *)elt, &info);
					if (!strcmp(info.name, qname)) {
						svg_parse_attribute(parser, &info, elt, attributes->children->content, anim_datatype);
						break;
					}
				}
			}
		} 
		attributes = attributes->next;
	}
	if (register_id) ChainAddEntry(parser->ided_nodes, elt);

	children = node->xmlChildrenNode;
	while(children) {
		SVGElement *child;
		if (children->type == XML_ELEMENT_NODE) {
			child = svg_parse_element(parser, children, elt);
			if (child) ChainAddEntry(elt->children, child);
		} else if (children->type == XML_TEXT_NODE && tag == TAG_SVG_text) {
			SVGtextElement *text = (SVGtextElement *)elt;
			text->string.string = strdup(children->content);
			text->string.length = strlen(children->content);
		}
		children = children->next;
	}
	if (elt) Node_Init((SFNode *)elt);
	return elt;
}

void SVG_ParserTerminate(SVGParser *parser)
{
	if (!parser) return;
	DeleteChain(parser->ided_nodes);
	free(parser->fileName);
	free(parser);
	parser = NULL;
}

M4Err SVG_ParserInit(SVGParser *parser)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;

	M4Err e;

	SVGElement *n;

	if (!parser->ctx || !parser->fileName) return M4BadParam;

    xmlInitParser();
	LIBXML_TEST_VERSION

	doc = xmlParseFile(parser->fileName);
	if (doc == NULL) {
		e = M4BadParam;
		goto exit;
	}

	parser->xpathCtx = xmlXPathNewContext(doc);
    if(parser->xpathCtx == NULL) {
        //fprintf(stderr,"Error: unable to create new XPath context\n");
        xmlFreeDoc(doc); 
        return M4BadParam;
    }

	root = xmlDocGetRootElement(doc);

	parser->ided_nodes = NewChain();

	/*create one empty Scene Description Stream*/
	parser->svg_es = M4SM_NewStream(parser->ctx, 0, M4ST_SCENE, 0);
	parser->svg_au = M4SM_NewAU(parser->svg_es, 0, 0, 1);

	parser->od_es = M4SM_NewStream(parser->ctx, 1, M4ST_OD, 0);
	parser->od_au = M4SM_NewAU(parser->od_es, 0, 0, 1);

	n = svg_parse_element(parser, root, NULL);
	if (n) {
		SVGsvgElement *root_svg = (SVGsvgElement *)n;
		if (root_svg->width.unitType == SVG_LENGTHTYPE_PX && 
			root_svg->height.unitType == SVG_LENGTHTYPE_PX) {
			parser->ctx->scene_width = (u32)root_svg->width.number;
			parser->ctx->scene_height = (u32)root_svg->height.number;
		}
		SG_SetRootNode(parser->inline_scene->graph, (SFNode *)n);
	}

	return M4OK;
exit:
	e = parser->last_error;
	if (e) SVG_ParserTerminate(parser);
	return e;

}

/*************************************************************************
 *																	 	 *
 * Functions implementing the Basic Decoder and Scene Decoder interfaces *
 *																		 *
 *************************************************************************/
static M4Err SVG_ProcessData(SceneDecoder *plug, unsigned char *inBuffer, u32 inBufferLength, 
								u16 ES_ID, u32 stream_time, u32 mmlevel)
{
	M4Err e = M4OK;
	SVGParser *parser = plug->privateStack;

	if (parser->status == 0) {
		parser->status = 1;
		parser->ctx = NewSceneManager(parser->inline_scene->graph);
		e = SVG_ParserInit(parser);
		if (!e) {
			SG_SetSizeInfo(parser->inline_scene->graph, parser->ctx->scene_width, parser->ctx->scene_height, 1);
			/*attach graph to renderer*/
			IS_AttachGraphToRender(parser->inline_scene);
		} else {
			parser->status = 0;
			return e;
		}
	}
	return M4EOF;
}

static M4Err SVG_AttachScene(SceneDecoder *plug, InlineScene *scene, Bool is_scene_decoder)
{
	SVGParser *parser = plug->privateStack;
	parser->inline_scene = scene;
	parser->app = scene->root_od->term;
	return M4OK;
}

static M4Err SVG_ReleaseScene(SceneDecoder *plug)
{
	SVGParser *parser = plug->privateStack;
	if (parser->ctx) M4SM_Delete(parser->ctx);
	parser->ctx = NULL;
	return M4OK;
}


static M4Err SVG_AttachStream(BaseDecoder *plug, 
									 u16 ES_ID, 
									 unsigned char *decSpecInfo, 
									 u32 decSpecInfoSize, 
									 u16 DependsOnES_ID,
									 u32 objectTypeIndication, 
									 Bool Upstream)
{
	SVGParser *parser = plug->privateStack;
	if (Upstream) return M4NotSupported;

	/*main dummy stream we need a dsi*/
	if (!decSpecInfo) return M4NonCompliantBitStream;

	parser->fileName = strdup(decSpecInfo);
	return M4OK;
}

static M4Err SVG_DetachStream(BaseDecoder *plug, u16 ES_ID)
{
	return M4OK;
}

const char *SVG_GetName(struct _basedecoder *plug)
{
	return "GPAC SVG Parser";
}

Bool SVG_CanHandleStream(BaseDecoder *ifce, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL)
{
	if ((StreamType==M4ST_PRIVATE_SCENE) && (ObjectType==2)) return 1;
	return 0;
}

static M4Err SVG_GetCapabilities(BaseDecoder *plug, CapObject *cap)
{
	cap->cap.valueINT = 0;
	return M4NotSupported;
}

static M4Err SVG_SetCapabilities(BaseDecoder *plug, const CapObject capability)
{
	return M4OK;
}

/*interface create*/
void *LoadInterface(u32 InterfaceType)
{
	SVGParser *parser;
	SceneDecoder *sdec;
	if (InterfaceType != M4SCENEDECODERINTERFACE) return NULL;
	
	SAFEALLOC(sdec, sizeof(SceneDecoder))
	M4_REG_PLUG(sdec, M4SCENEDECODERINTERFACE, "GPAC SVG Parser", "gpac distribution", 0);

	SAFEALLOC(parser, sizeof(SVGParser))

	sdec->privateStack = parser;
	sdec->AttachStream = SVG_AttachStream;
	sdec->CanHandleStream = SVG_CanHandleStream;
	sdec->DetachStream = SVG_DetachStream;
	sdec->AttachScene = SVG_AttachScene;
	sdec->ReleaseScene = SVG_ReleaseScene;
	sdec->ProcessData = SVG_ProcessData;
	sdec->GetName = SVG_GetName;
	sdec->SetCapabilities = SVG_SetCapabilities;
	sdec->GetCapabilities = SVG_GetCapabilities;
	return sdec;
}


/*interface destroy*/
void ShutdownInterface(void *ifce)
{
	SceneDecoder *sdec = (SceneDecoder *)ifce;
	if (sdec->InterfaceType != M4SCENEDECODERINTERFACE) return;
	SVG_ParserTerminate(sdec->privateStack);
	sdec->privateStack = NULL;
	free(sdec);
}

/*interface query*/
Bool QueryInterface(u32 InterfaceType)
{
	if (InterfaceType == M4SCENEDECODERINTERFACE) return 1;
	return 0;
}
#else


/*interface create*/
void *LoadInterface(u32 InterfaceType)
{
	return NULL;
}


/*interface destroy*/
void ShutdownInterface(void *ifce)
{
}

/*interface query*/
Bool QueryInterface(u32 InterfaceType)
{
	return 0;
}
#endif
