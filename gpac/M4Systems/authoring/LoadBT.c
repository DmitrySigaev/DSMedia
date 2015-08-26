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

#include <gpac/intern/m4_author_dev.h>
#include <gpac/intern/m4_bifs_dev.h>
#include <gpac/intern/m4_scenegraph_dev.h>

#include <gpac/m4_x3d_nodes.h>

/*since 0.2.2, we use zlib for bt reading to handle wrl.gz files*/
#include <zlib.h>

#define BT_LINE_SIZE	4000

typedef struct
{
	M4ContextLoader *load;
	gzFile gz_in;
	
	/*create from string only*/
	Chain *top_nodes;

	M4Err last_error;
	u32 line;

	Bool done, is_wrl;
	/*0: no unicode, 1: UTF-16BE, 2: UTF-16LE*/
	u32 unicode_type;

	/*routes are not created in the graph when parsing, so we need to track insert and delete/replace*/
	Chain *unresolved_routes, *inserted_routes;
	Chain *undef_nodes, *def_nodes;

	char *line_buffer;
	char cur_buffer[500];
	s32 line_size, line_pos, line_start_pos;

	/*set when parsing proto*/
	LPPROTO parsing_proto;
	Bool is_extern_proto_field;

	/*current stream ID, AU time and RAP flag*/
	u32 stream_id;
	u32 au_time;
	Bool au_is_rap;

	/*current BIFS stream & AU*/
	M4StreamContext *bifs_es;
	M4AUContext *bifs_au;
	u32 base_bifs_id;
	SGCommand *cur_com;

	/*current OD stream & AU*/
	M4StreamContext *od_es;
	M4AUContext *od_au;
	u32 base_od_id;
} BTParser;

M4Err bt_parse_bifs_command(BTParser *parser, char *name, Chain *cmdList);
LPROUTE bt_parse_route(BTParser *parser, Bool skip_def, Bool is_insert, SGCommand *com);
void bt_resolve_routes(BTParser *parser, Bool clean);

static M4Err bt_report(BTParser *parser, M4Err e, char *format, ...)
{
	va_list args;
	va_start(args, format);
	if (parser->load->OnMessage) {
		char szMsg[2048];
		char szMsgFull[2048];
		vsprintf(szMsg, format, args);
		sprintf(szMsgFull, "(line %d) %s", parser->line, szMsg);
		parser->load->OnMessage(parser->load->cbk, szMsgFull, e);
	} else {
		fprintf(stdout, "(line %d) ", parser->line);
		vfprintf(stdout, format, args);
		fprintf(stdout, "\n");
	}
	va_end(args);
	if (e) parser->last_error = e;
	return e;
}


SFNode *BTNewNode(BTParser *parser, u32 tag) 
{
	SFNode *n = SG_NewNode(parser->load->scene_graph, tag);
	return n;
}

void bt_check_line(BTParser *parser)
{
	while (
		(parser->line_buffer[parser->line_pos]==' ') 
		|| (parser->line_buffer[parser->line_pos]=='\t') 
		) parser->line_pos++;

	if (parser->line_buffer[parser->line_pos]=='#') parser->line_size = parser->line_pos;
	else if ((parser->line_buffer[parser->line_pos]=='/') && (parser->line_buffer[parser->line_pos+1]=='/') ) parser->line_size = parser->line_pos;

	if (parser->line_size == parser->line_pos) {
		/*string based input - done*/
		if (!parser->gz_in) {
			parser->done = 1;
			return;
		}

next_line:
		parser->line_start_pos = gztell(parser->gz_in);
		parser->line_buffer[0] = 0;
		if (parser->unicode_type) {
			u8 c1, c2;
			unsigned short wchar;
			unsigned short l[BT_LINE_SIZE];
			unsigned short *dst = l;
			Bool is_ret = 0;
			u32 last_space_pos, last_space_pos_stream;
			u32 go = BT_LINE_SIZE - 1;
			last_space_pos = last_space_pos_stream = 0;
			while (go && !gzeof(parser->gz_in) ) {
				c1 = gzgetc(parser->gz_in);
				c2 = gzgetc(parser->gz_in);
				/*Little-endian order*/
				if (parser->unicode_type==2) {
					if (c2) { wchar = c2; wchar <<=8; wchar |= c1; }
					else wchar = c1;
				} else {
					wchar = c1;
					if (c2) { wchar <<= 8; wchar |= c2;}
				}
				*dst = wchar;
				if (wchar=='\r') is_ret = 1;
				else if (wchar=='\n') {
					dst++;
					break;
				}
				else if (is_ret && wchar!='\n') {
					u32 fpos = gztell(parser->gz_in);
					gzseek(parser->gz_in, fpos-2, SEEK_SET);
					is_ret = 1;
					break;
				}
				if (wchar==' ') {
					last_space_pos_stream = gztell(parser->gz_in);
					last_space_pos = dst - l;
				}
				dst++;
				go--;

			}
			*dst = 0;
			/*long line, rewind stream to last space*/
			if (!go) {
				u32 rew_pos = gztell(parser->gz_in) - 2*(dst - &l[last_space_pos]);
				gzseek(parser->gz_in, rew_pos, SEEK_SET);
				l[last_space_pos+1] = 0;
			}
			/*check eof*/
			if (l[0]==0xFFFF) {
				parser->done = 1;
				return;
			}
			/*convert to mbc string*/
			dst = l;
			utf8_wcstombs(parser->line_buffer, BT_LINE_SIZE, (const unsigned short **) &dst);

			if (!strlen(parser->line_buffer) && gzeof(parser->gz_in)) {
				parser->done = 1;
				return;
			}
		} else {
			if ((gzgets(parser->gz_in, parser->line_buffer, BT_LINE_SIZE) == NULL) 
				|| (!strlen(parser->line_buffer) && gzeof(parser->gz_in))) {
				parser->done = 1;
				return;
			}
			/*watchout for long lines*/
			if (1 + strlen(parser->line_buffer) == BT_LINE_SIZE) {
				u32 rew, pos, go;
				rew = 0;
				go = 1;
				while (go) {
					switch (parser->line_buffer[strlen(parser->line_buffer)-1]) {
					case ' ':
					case ',':
					case '[':
					case ']':
						go = 0;
						break;
					default:
						parser->line_buffer[strlen(parser->line_buffer)-1] = 0;
						rew++;
						break;
					}
				}
				pos = gztell(parser->gz_in);
				gzseek(parser->gz_in, pos-rew, SEEK_SET);
			}
		}


		while (
			(parser->line_buffer[strlen(parser->line_buffer)-1]=='\n')
			|| (parser->line_buffer[strlen(parser->line_buffer)-1]=='\r')
			|| (parser->line_buffer[strlen(parser->line_buffer)-1]=='\t')
			)
			parser->line_buffer[strlen(parser->line_buffer)-1] = 0;

		
		parser->line_size = strlen(parser->line_buffer);
		parser->line_pos = 0;
		parser->line++;

		while ((parser->line_buffer[parser->line_pos]==' ') || (parser->line_buffer[parser->line_pos]=='\t'))
			parser->line_pos++;
		if ( (parser->line_buffer[parser->line_pos]=='#') 
			|| ( (parser->line_buffer[parser->line_pos]=='/') && (parser->line_buffer[parser->line_pos+1]=='/')) ) {

			if (parser->line==1) {
				if (strstr(parser->line_buffer, "VRML")) {
					if (strstr(parser->line_buffer, "VRML V2.0")) parser->is_wrl = 1;
					/*although not std, many files use this*/
					else if (strstr(parser->line_buffer, "VRML2.0")) parser->is_wrl = 1;
					else {
						bt_report(parser, M4NotSupported, "%s: VRML Version Not Supported", parser->line_buffer);
						return;
					}
				}
				else if (strstr(parser->line_buffer, "X3D")) {
					if (strstr(parser->line_buffer, "X3D V3.0")) parser->is_wrl = 2;
					else {
						bt_report(parser, M4NotSupported, "%s: X3D Version Not Supported", parser->line_buffer);
						return;
					}
				}
			}
			goto next_line;
		}
	}
	if (!parser->line_size) {
		if (!gzeof(parser->gz_in)) bt_check_line(parser);
		else parser->done = 1;
	}
	else if (!parser->done && (parser->line_size == parser->line_pos)) bt_check_line(parser);
}

void bt_force_line(BTParser *parser)
{
	parser->line_pos = parser->line_size;
}

Bool bt_check_code(BTParser *parser, char code)
{
	bt_check_line(parser);
	if (parser->line_buffer[parser->line_pos]==code) {
		parser->line_pos++;
		return 1;
	}
	return 0;
}

char *bt_get_next(BTParser *parser, Bool point_break)
{
	u32 has_quote;
	s32 i;
	bt_check_line(parser);
	i=0;
	has_quote = 0;
	while (1) {
		if (parser->line_buffer[parser->line_pos + i] == '\"') {
			if (!has_quote) has_quote = 1;
			else has_quote = 0;
			parser->line_pos += 1;

			if (parser->line_pos+i==parser->line_size) break;
			continue;
		}
		if (!has_quote) {
			if (!parser->line_buffer[parser->line_pos + i]) break;
			else if (parser->line_buffer[parser->line_pos + i] == ' ') break;
			else if (parser->line_buffer[parser->line_pos + i] == '\t') break;
			else if (parser->line_buffer[parser->line_pos + i] == '{') break;
			else if (parser->line_buffer[parser->line_pos + i] == '}') break;
			else if (parser->line_buffer[parser->line_pos + i] == ']') break;
			else if (parser->line_buffer[parser->line_pos + i] == '[') break;
			else if (parser->line_buffer[parser->line_pos + i] == ',') break;
			else if (point_break && parser->line_buffer[parser->line_pos + i] == '.') break;
		}		
		parser->cur_buffer[i] = parser->line_buffer[parser->line_pos + i];
		i++;
		if (parser->line_pos+i==parser->line_size) break;
	}
	parser->cur_buffer[i] = 0;
	parser->line_pos += i;
	return parser->cur_buffer;
}

char *bt_get_string(BTParser *parser)
{
	char *res;
	s32 i, size;

	res = malloc(sizeof(char) * 500);
	size = 500;
	while (parser->line_buffer[parser->line_pos]==' ') parser->line_pos++;
	
	if (parser->line_pos==parser->line_size) {
		if (gzeof(parser->gz_in)) return NULL;
		bt_check_line(parser);
	}

	i=0;
	while (1) {
		if (parser->line_buffer[parser->line_pos] == '\"') 
			if (parser->line_buffer[parser->line_pos-1] != '\\') break;
		if (i==size) {
			res = realloc(res, sizeof(char) * (size+500));
			size += 500;
		}

		if ((parser->line_buffer[parser->line_pos]=='/') && (parser->line_buffer[parser->line_pos+1]=='/') ) {
			/*this looks like a comment*/
			if (!strstr(&parser->line_buffer[parser->line_pos], "\"")) {
				bt_check_line(parser);
				continue;
			}
		}
		if ((parser->line_buffer[parser->line_pos] != '\\') || (parser->line_buffer[parser->line_pos+1] != '"')) {
			/*handle UTF-8 - WARNING: if parser is in unicode string is already utf8 multibyte chars*/
			if (!parser->unicode_type && parser->line_buffer[parser->line_pos] & 0x80) {
				res[i] = 0xc0 | ( (parser->line_buffer[parser->line_pos] >> 6) & 0x3 );
				i++;
				parser->line_buffer[parser->line_pos] &= 0xbf;
			}
			res[i] = parser->line_buffer[parser->line_pos];
			i++;
		}
		parser->line_pos++;
		if (parser->line_pos==parser->line_size) {
			bt_check_line(parser);
		}

	}
	res[i] = 0;
	parser->line_pos += 1;
	return res;
}

Bool bt_check_externproto_field(BTParser *parser, char *str)
{
	if (!parser->is_extern_proto_field) return 0;
	if (!strcmp(str, "field") || !strcmp(str, "eventIn") || !strcmp(str, "eventOut") || !strcmp(str, "exposedField")) {
		parser->last_error = M4FieldNotQuantized;
		return 1;
	} 
	return 0;
}

M4Err bt_parse_float(BTParser *parser, const char *name, Float *val)
{
	u32 i;
	char *str = bt_get_next(parser, 0);

	if (!str) return parser->last_error = M4IOErr;
	if (bt_check_externproto_field(parser, str)) return M4OK;


	for (i=0; i<strlen(str); i++) {
		if (!isdigit(str[i]) && (str[i] != '.') && (str[i] != 'E') && (str[i] != 'e') && (str[i] != '-') && (str[i] != '+')) {
			return bt_report(parser, M4BadParam, "%s: Number expected", name);
		}
	}
	if (!i) {
		return bt_report(parser, M4BadParam, "%s: Number expected", name);
	}
	*val = (Float) atof(str);
	return M4OK;
}
M4Err bt_parse_double(BTParser *parser, const char *name, SFDouble *val)
{
	u32 i;
	char *str = bt_get_next(parser, 0);
	if (!str) return parser->last_error = M4IOErr;
	if (bt_check_externproto_field(parser, str)) return M4OK;

	for (i=0; i<strlen(str); i++) {
		if (!isdigit(str[i]) && (str[i] != '.') && (str[i] != 'E') && (str[i] != 'e') && (str[i] != '-') && (str[i] != '+')) {
			return bt_report(parser, M4BadParam, "%s: Number expected", name);
		}
	}
	if (!i) {
		return bt_report(parser, M4BadParam, "%s: Number expected", name);
	}
	*val = atof(str);
	return M4OK;
}
M4Err bt_parse_int(BTParser *parser, const char *name, SFInt32 *val)
{
	u32 i;
	char *str = bt_get_next(parser, 0);
	if (!str) return parser->last_error = M4IOErr;
	if (bt_check_externproto_field(parser, str)) return M4OK;
	/*URL ODID*/
	if (!strnicmp(str, "od:", 3)) str += 3;

	for (i=0; i<strlen(str); i++) {
		if (!isdigit(str[i]) && (str[i] != 'E') && (str[i] != 'e') && (str[i] != '-')) {
			return bt_report(parser, M4BadParam, "%s: Number expected", name);
		}
	}
	if (!i) {
		return bt_report(parser, M4BadParam, "%s: Number expected", name);
	}
	*val = atoi(str);
	return M4OK;
}
M4Err bt_parse_bool(BTParser *parser, const char *name, SFBool *val)
{
	char *str = bt_get_next(parser, 0);
	if (!str) return parser->last_error = M4IOErr;
	if (bt_check_externproto_field(parser, str)) return M4OK;

	if (!stricmp(str, "true") || !strcmp(str, "1") ) {
		*val = 1;
	}
	else if (!stricmp(str, "false") || !strcmp(str, "0") ) {
		*val = 0;
	} else {
		return bt_report(parser, M4BadParam, "%s: Boolean expected", name);
	}
	return M4OK;
}

M4Err bt_parse_color(BTParser *parser, const char *name, SFColor *col)
{
	u32 i;
	u32 val;
	char *str = bt_get_next(parser, 0);
	if (!str) return parser->last_error = M4IOErr;
	if (bt_check_externproto_field(parser, str)) return M4OK;

	/*HTML code*/
	if (str[0]=='$') {
		sscanf(str, "%x", &val);
		col->red = (Float) ((val>>16) & 0xFF) / 255.0f;
		col->green = (Float) ((val>>8) & 0xFF) / 255.0f;
		col->blue = (Float) (val & 0xFF) / 255.0f;
		return parser->last_error;
	} 

	for (i=0; i<strlen(str); i++) {
		if (!isdigit(str[i]) && (str[i] != '.') && (str[i] != 'E') && (str[i] != 'e') && (str[i] != '-') && (str[i] != '+')) {
			return bt_report(parser, M4BadParam, "%s: Number expected", name);
		}
	}
	col->red = (Float) atof(str);
	/*many VRML files use ',' separator*/
	bt_check_code(parser, ',');
	bt_parse_float(parser, name, & col->green);
	bt_check_code(parser, ',');
	bt_parse_float(parser, name, & col->blue);
	return parser->last_error;
}

M4Err bt_parse_colorRGBA(BTParser *parser, const char *name, SFColorRGBA *col)
{
	u32 i;
	u32 val;
	char *str = bt_get_next(parser, 0);
	if (!str) return parser->last_error = M4IOErr;
	if (bt_check_externproto_field(parser, str)) return M4OK;

	/*HTML code*/
	if (str[0]=='$') {
		sscanf(str, "%x", &val);
		col->red = (Float) ((val>>24) & 0xFF) / 255.0f;
		col->green = (Float) ((val>>16) & 0xFF) / 255.0f;
		col->blue = (Float) ((val>>8) & 0xFF) / 255.0f;
		col->alpha = (Float) (val & 0xFF) / 255.0f;
		return parser->last_error;
	} 

	for (i=0; i<strlen(str); i++) {
		if (!isdigit(str[i]) && (str[i] != '.') && (str[i] != 'E') && (str[i] != 'e') && (str[i] != '-') && (str[i] != '+')) {
			return bt_report(parser, M4BadParam, "%s: Number expected", name);
		}
	}
	col->red = (Float) atof(str);
	bt_parse_float(parser, name, & col->green);
	bt_parse_float(parser, name, & col->blue);
	bt_parse_float(parser, name, & col->alpha);
	return parser->last_error;
}

static void bt_offset_time(BTParser *parser, Double *time)
{
	Double res;
	res = parser->au_time;
	res /= parser->bifs_es->timeScale;
	*time += res;
}

static void bt_check_time_offset(BTParser *parser, SFNode *n, FieldInfo *info)
{
	if (!(parser->load->flags & M4CL_FOR_PLAYBACK)) return;
	if (Node_GetTag(n) != TAG_ProtoNode) {
		if (!stricmp(info->name, "startTime") || !stricmp(info->name, "stopTime")) 
			bt_offset_time(parser, (Double *)info->far_ptr);
	} else if (Proto_FieldIsSFTimeOffset(n, info)) {
		bt_offset_time(parser, (Double *)info->far_ptr);
	}
}
static void bt_update_timenode(BTParser *parser, SFNode *node)
{
	if (!(parser->load->flags & M4CL_FOR_PLAYBACK)) return;

	switch (Node_GetTag(node)) {
	case TAG_MPEG4_AnimationStream:
		bt_offset_time(parser, & ((M_AnimationStream*)node)->startTime);
		bt_offset_time(parser, & ((M_AnimationStream*)node)->stopTime);
		break;
	case TAG_MPEG4_AudioBuffer:
		bt_offset_time(parser, & ((M_AudioBuffer*)node)->startTime);
		bt_offset_time(parser, & ((M_AudioBuffer*)node)->stopTime);
		break;
	case TAG_MPEG4_AudioClip:
		bt_offset_time(parser, & ((M_AudioClip*)node)->startTime);
		bt_offset_time(parser, & ((M_AudioClip*)node)->stopTime);
		break;
	case TAG_MPEG4_AudioSource:
		bt_offset_time(parser, & ((M_AudioSource*)node)->startTime);
		bt_offset_time(parser, & ((M_AudioSource*)node)->stopTime);
		break;
	case TAG_MPEG4_MovieTexture:
		bt_offset_time(parser, & ((M_MovieTexture*)node)->startTime);
		bt_offset_time(parser, & ((M_MovieTexture*)node)->stopTime);
		break;
	case TAG_MPEG4_TimeSensor:
		bt_offset_time(parser, & ((M_TimeSensor*)node)->startTime);
		bt_offset_time(parser, & ((M_TimeSensor*)node)->stopTime);
		break;
	case TAG_ProtoNode:
	{
		u32 i, nbFields;
		FieldInfo inf;
		nbFields = Node_GetNumFields(node, FCM_ALL);
		for (i=0; i<nbFields; i++) {
			Node_GetField(node, i, &inf);
			if (inf.fieldType != FT_SFTime) continue;
			bt_check_time_offset(parser, node, &inf);
		}
	}
		break;
	}
}

void bt_sffield(BTParser *parser, FieldInfo *info, SFNode *n)
{
	switch (info->fieldType) {
	case FT_SFInt32:
		bt_parse_int(parser, info->name, (SFInt32 *)info->far_ptr);
		if (parser->last_error) return;
		break;
	case FT_SFBool:
		bt_parse_bool(parser, info->name, (SFBool *)info->far_ptr);
		if (parser->last_error) return;
		break;
	case FT_SFFloat:
		bt_parse_float(parser, info->name, (SFFloat *)info->far_ptr);
		if (parser->last_error) return;
		break;
	case FT_SFDouble:
		bt_parse_double(parser, info->name, (SFDouble *)info->far_ptr);
		if (parser->last_error) return;
		break;
	case FT_SFTime:
		bt_parse_double(parser, info->name, (SFDouble *)info->far_ptr);
		if (parser->last_error) return;
		bt_check_time_offset(parser, n, info);
		break;
	case FT_SFColor:
		bt_parse_color(parser, info->name, (SFColor *)info->far_ptr);
		break;
	case FT_SFColorRGBA:
		bt_parse_colorRGBA(parser, info->name, (SFColorRGBA *)info->far_ptr);
		break;
	case FT_SFVec2f:
		bt_parse_float(parser, info->name, & ((SFVec2f *)info->far_ptr)->x);
		if (parser->last_error) return;
		/*many VRML files use ',' separator*/
		bt_check_code(parser, ',');
		bt_parse_float(parser, info->name, & ((SFVec2f *)info->far_ptr)->y);
		if (parser->last_error) return;
		break;
	case FT_SFVec2d:
		bt_parse_double(parser, info->name, & ((SFVec2d *)info->far_ptr)->x);
		if (parser->last_error) return;
		/*many VRML files use ',' separator*/
		bt_check_code(parser, ',');
		bt_parse_double(parser, info->name, & ((SFVec2d *)info->far_ptr)->y);
		if (parser->last_error) return;
		break;
	case FT_SFVec3f:
		bt_parse_float(parser, info->name, & ((SFVec3f *)info->far_ptr)->x);
		if (parser->last_error) return;
		/*many VRML files use ',' separator*/
		bt_check_code(parser, ',');
		bt_parse_float(parser, info->name, & ((SFVec3f *)info->far_ptr)->y);
		if (parser->last_error) return;
		/*many VRML files use ',' separator*/
		bt_check_code(parser, ',');
		bt_parse_float(parser, info->name, & ((SFVec3f *)info->far_ptr)->z);
		if (parser->last_error) return;
		break;
	case FT_SFVec3d:
		bt_parse_double(parser, info->name, & ((SFVec3d *)info->far_ptr)->x);
		if (parser->last_error) return;
		/*many VRML files use ',' separator*/
		bt_check_code(parser, ',');
		bt_parse_double(parser, info->name, & ((SFVec3d *)info->far_ptr)->y);
		if (parser->last_error) return;
		/*many VRML files use ',' separator*/
		bt_check_code(parser, ',');
		bt_parse_double(parser, info->name, & ((SFVec3d *)info->far_ptr)->z);
		if (parser->last_error) return;
		break;
	case FT_SFRotation:
		bt_parse_float(parser, info->name, & ((SFRotation *)info->far_ptr)->xAxis);
		if (parser->last_error) return;
		bt_parse_float(parser, info->name, & ((SFRotation *)info->far_ptr)->yAxis);
		if (parser->last_error) return;
		bt_parse_float(parser, info->name, & ((SFRotation *)info->far_ptr)->zAxis);
		if (parser->last_error) return;
		bt_parse_float(parser, info->name, & ((SFRotation *)info->far_ptr)->angle);
		if (parser->last_error) return;
		break;
	case FT_SFString:
		if (bt_check_code(parser, '\"') || bt_check_code(parser, '\'')) {
			char *str = bt_get_string(parser); 
			if (!str) 
				goto err;
			if (((SFString *)info->far_ptr)->buffer) free(((SFString *)info->far_ptr)->buffer);
			((SFString *)info->far_ptr)->buffer = NULL;
			if (strlen(str))
				((SFString *)info->far_ptr)->buffer = str;
			else
				free(str);
		} else {
			goto err;
		}
		break;
	case FT_SFURL:
		if (bt_check_code(parser, '\"') || bt_check_code(parser, '\'')) {
			SFURL *url = (SFURL *)info->far_ptr;
			char *str = bt_get_string(parser);
			if (!str) goto err;
			if (url->url) free(url->url);
			url->url = NULL;
			url->OD_ID = 0;
			if (strchr(str, '#')) {
				url->url = str;
			} else {
				u32 id = 0;
				char *odstr = str;
				if (!strnicmp(str, "od:", 3)) odstr += 3;
				/*be carefull, an url like "11-regression-test.mp4" will return 1 on sscanf :)*/
				if (sscanf(odstr, "%d", &id) == 1) {
					char szURL[20];
					sprintf(szURL, "%d", id);
					if (strcmp(szURL, odstr)) id=0;
				}
				if (id) {
					url->OD_ID = id;
					free(str);
				} else {
					url->url = str;
				}
			}
		} else {
			s32 val;
			bt_parse_int(parser, info->name, & val );
			if (parser->last_error) return;
			((SFURL *)info->far_ptr)->OD_ID = val;
		}
		break;
	case FT_SFCommandBuffer:
	{
		SFCommandBuffer *cb = (SFCommandBuffer *)info->far_ptr;
		if (bt_check_code(parser, '{')) {
			SGCommand *prev_com = parser->cur_com;
			while (!parser->last_error) {
				if (bt_check_code(parser, '}')) break;
				parser->last_error = bt_parse_bifs_command(parser, NULL, cb->commandList);
			}
			parser->cur_com = prev_com;
		}
	}
		break;
	case FT_SFImage:
	{
		u32 i, size, v;
		char *str;
		SFImage *img = (SFImage *)info->far_ptr;
		bt_parse_int(parser, "width", &img->width);
		if (parser->last_error) return;
		bt_parse_int(parser, "height", &img->height);
		if (parser->last_error) return;
		bt_parse_int(parser, "nbComp", &v);
		if (parser->last_error) return;
		img->numComponents = v;
		size = img->width * img->height * img->numComponents;
		if (img->pixels) free(img->pixels);
		img->pixels = malloc(sizeof(char) * size);
		for (i=0; i<size; i++) {
			str = bt_get_next(parser, 0);
			if (strstr(str, "0x")) sscanf(str, "%x", &v);
			else sscanf(str, "%d", &v);
			switch (img->numComponents) {
			case 1:
				img->pixels[i] = (char) v;
				break;
			case 2:
				img->pixels[i] = (char) (v>>8)&0xFF;
				img->pixels[i+1] = (char) (v)&0xFF;
				i++;
				break;
			case 3:
				img->pixels[i] = (char) (v>>16)&0xFF;
				img->pixels[i+1] = (char) (v>>8)&0xFF;
				img->pixels[i+2] = (char) (v)&0xFF;
				i+=2;
				break;
			case 4:
				img->pixels[i] = (char) (v>>24)&0xFF;
				img->pixels[i+1] = (char) (v>>16)&0xFF;
				img->pixels[i+2] = (char) (v>>8)&0xFF;
				img->pixels[i+3] = (char) (v)&0xFF;
				i+=3;
				break;
			}
		}
	}
		break;
	case FT_SFScript:
	{
		SFScript *sc = (SFScript *) info->far_ptr;
		if (!bt_check_code(parser, '\"')) {
			bt_report(parser, M4BadParam, "\" expected in Script");
		}
		sc->script_text = bt_get_string(parser);
	}
		break;
	default:
		parser->last_error = M4NotSupported;
		break;

	}
	bt_check_code(parser, ','); 
	return;
err:
	bt_report(parser, M4BadParam, "%s: Invalid field syntax", info->name);
}

void bt_mffield(BTParser *parser, FieldInfo *info, SFNode *n)
{
	FieldInfo sfInfo;
	Bool force_single = 0;

	if (!bt_check_code(parser, '[')) {
		if (parser->is_extern_proto_field) return;
		force_single = 1;
	}

	sfInfo.fieldType = VRML_GetSFType(info->fieldType);
	sfInfo.name = info->name;
	VRML_MF_Reset(info->far_ptr, info->fieldType);

	while (!bt_check_code(parser, ']')) {
		VRML_MF_Append(info->far_ptr, info->fieldType, &sfInfo.far_ptr);
		bt_sffield(parser, &sfInfo, n);
		if (parser->last_error) return;

		bt_check_code(parser, ','); 
		if (force_single) break;
	}
}

Bool BTCheckNDT(BTParser *parser, FieldInfo *info, SFNode *node, SFNode *parent)
{
	if (parent->sgprivate->tag == TAG_MPEG4_Script) return 1;
	if (parent->sgprivate->tag == TAG_X3D_Script) return 1;

	/*this handles undefined nodes*/
	if (Node_IsInTable(node, info->NDTtype)) return 1;
	/*not found*/
	bt_report(parser, M4InvalidNode, "node %s not valid in field %s\n", Node_GetName(node), info->name);
	Node_Unregister(node, parent);
	return 0;
}

u32 bt_get_next_node_id(BTParser *parser)
{
	u32 ID;
	LPSCENEGRAPH sc = parser->load->scene_graph;
	if (parser->parsing_proto) sc = Proto_GetSceneGraph(parser->parsing_proto);
	ID = SG_GetNextAvailableNodeID(sc);
	if (parser->load->ctx && (ID>parser->load->ctx->max_node_id)) 
		parser->load->ctx->max_node_id = ID;
	return ID;
}
u32 bt_get_next_route_id(BTParser *parser)
{
	u32 ID;
	LPSCENEGRAPH sc = parser->load->scene_graph;
	if (parser->parsing_proto) sc = Proto_GetSceneGraph(parser->parsing_proto);

	ID = SG_GetNextAvailableRouteID(sc);
	if (parser->load->ctx && (ID>parser->load->ctx->max_route_id)) 
		parser->load->ctx->max_route_id = ID;
	return ID;
}
u32 bt_get_next_proto_id(BTParser *parser)
{
	u32 ID;
	LPSCENEGRAPH sc = parser->load->scene_graph;
	if (parser->parsing_proto) sc = Proto_GetSceneGraph(parser->parsing_proto);
	ID = SG_GetNextAvailableProtoID(sc);
	if (parser->load->ctx && (ID>parser->load->ctx->max_node_id)) 
		parser->load->ctx->max_proto_id = ID;
	return ID;
}

u32 bt_GetDEFID(BTParser *parser, char *defName)
{
	SFNode *n;
	u32 ID;
	if (sscanf(defName, "N%d", &ID) == 1) {
		ID ++;
		n = SG_FindNode(parser->load->scene_graph, ID);
		/*if an existing node use*/
		if (n) {
			u32 nID = bt_get_next_node_id(parser);
			bt_report(parser, M4OK, "WARNING: changing node \"%s\" ID from %d to %d", n->sgprivate->NodeName, n->sgprivate->NodeID-1, nID-1);
			Node_SetDEF(n, nID, n->sgprivate->NodeName);
		}
		if (parser->load->ctx && (parser->load->ctx->max_node_id<ID)) parser->load->ctx->max_node_id=ID;
	} else {
		ID = bt_get_next_node_id(parser);
	}
	return ID;
}

Bool bt_IS_field(BTParser *parser, FieldInfo *info, SFNode *n)
{
	u32 i;
	LPPROTOFIELD pfield;
	FieldInfo pinfo;
	char *str;
	bt_check_line(parser);
	i=0;
	while ((parser->line_buffer[parser->line_pos + i] == ' ') || (parser->line_buffer[parser->line_pos + i] == '\t')) i++;
	if (strnicmp(&parser->line_buffer[parser->line_pos + i] , "IS", 2)) return 0;

	str = bt_get_next(parser, 0);
	str = bt_get_next(parser, 0);

	/*that's an ISed field*/
	pfield = Proto_FindFieldByName(parser->parsing_proto, str);
	if (!pfield) {
		bt_report(parser, M4BadParam, "%s: Unknown proto field", str);
		return 1;
	}
	ProtoField_GetField(pfield, &pinfo);
	Proto_SetISField(parser->parsing_proto, pinfo.fieldIndex, n, info->fieldIndex);
	return 1;
}

void BT_CheckUnresolvedNodes(BTParser *parser)
{
	u32 i, count;
	count = ChainGetCount(parser->undef_nodes); 
	if (!count) return;
	for (i=0; i<count; i++) {
		SFNode *n = ChainGetEntry(parser->undef_nodes, i);
		assert(n->sgprivate->NodeName);
		fprintf(stdout, "Cannot find node %s\n", n->sgprivate->NodeName);
	}
	parser->last_error = M4BadParam;
}

Bool BT_HasBeenDEF(BTParser *parser, char *node_name)
{
	u32 i, count;
	count = ChainGetCount(parser->def_nodes);
	for (i=0; i<count; i++) {
		SFNode *n = ChainGetEntry(parser->def_nodes, i);
		if (!strcmp(n->sgprivate->NodeName, node_name)) return 1;
	}
	return 0;
}

u32 bt_get_node_tag(BTParser *parser, char *node_name) 
{
	u32 tag;
	/*if VRML and allowing non MPEG4 nodes, use X3D*/
	if (parser->is_wrl && !(parser->load->flags & M4CL_MPEG4_STRICT)) {
		tag = X3D_GetTagByName(node_name);
		if (!tag) tag = MPEG4_GetTagByName(node_name);
		if (tag) return tag;
		if (!strcmp(node_name, "Rectangle")) return TAG_X3D_Rectangle2D;
		if (!strcmp(node_name, "Circle")) return TAG_X3D_Circle2D;
	} else {
		tag = MPEG4_GetTagByName(node_name);
		if (!tag) {
			if (!strcmp(node_name, "Rectangle2D")) return TAG_MPEG4_Rectangle;
			if (!strcmp(node_name, "Circle2D")) return TAG_MPEG4_Circle;
			if (!(parser->load->flags & M4CL_MPEG4_STRICT)) return X3D_GetTagByName(node_name);
		}
	}
	return tag;
}

SFNode *bt_SFNode(BTParser *parser, char *node_name, SFNode *parent, char *szDEFName)
{
	u32 tag, ID;
	Bool is_script, replace_prev, register_def;
	LPPROTO p;
	SFNode *node, *newnode, *undef_node;
	FieldInfo info;
	Bool init_node;
	char *name;
	char * str;

	init_node = 0;

	if (node_name) {
		str = node_name;
	} else {
		str = bt_get_next(parser, 0);
	}
	name = NULL;
	if (!strcmp(str, "NULL")) return NULL;

	ID = 0;
	register_def = 0;
	replace_prev = 0;
	undef_node = NULL;
	if (!strcmp(str, "DEF")) {
		register_def = 1;
		str = bt_get_next(parser, 0);
		name = strdup(str);
		str = bt_get_next(parser, 0);
	} else if (szDEFName) {
		name = strdup(szDEFName);
		register_def = 1;
	}
	if (name) {
		undef_node = SG_FindNodeByName(parser->load->scene_graph, name);
		if (undef_node) {
			ID = undef_node->sgprivate->NodeID;
			/*if we see twice a DEF N1 then force creation of a new node*/
			if (BT_HasBeenDEF(parser, name)) {
				undef_node = NULL;
				ID = bt_GetDEFID(parser, name);
				bt_report(parser, M4OK, "Warning: Node %s has been DEFed several times, IDs may get corrupted", name);
			}
		} else {
			ID = bt_GetDEFID(parser, name);
		}
	}
	else if (!strcmp(str, "USE")) {
		str = bt_get_next(parser, 0);
		node = SG_FindNodeByName(parser->load->scene_graph, str);
		if (!node) {
			/*create a temp node (undefined)*/
			node = BTNewNode(parser, TAG_UndefinedNode);
			ID = bt_GetDEFID(parser, str);
			Node_SetDEF(node, ID, str);
			ChainAddEntry(parser->undef_nodes, node);
		} 
		Node_Register(node, parent);
		return node;
	}
	p = NULL;
	tag = bt_get_node_tag(parser, str);
	if (!tag) {
		LPSCENEGRAPH sg = parser->load->scene_graph;
		while (1) {
			p = SG_FindProto(sg, 0, str);
			if (p) break;
			sg = sg->parent_scene;
			if (!sg) break;
		}
		if (!p) {
			/*locate proto*/
			bt_report(parser, M4InvalidNode, "%s: not a valid/supported node", str);
			return NULL;
		}
		tag = TAG_ProtoNode;
	}
	if (undef_node && (undef_node->sgprivate->tag == tag)) {
		node = undef_node;
	} else {
		if (undef_node) replace_prev = 1;
		if (p) {
			node = Proto_CreateInstance(parser->load->scene_graph, p);
		} else {
			node = BTNewNode(parser, tag);
		}
		if (!parser->parsing_proto) init_node = 1;
	}
	is_script = 0;
	if ((tag==TAG_MPEG4_Script) || (tag==TAG_X3D_Script))
		is_script = 1;

	if (!node) {
		parser->last_error = M4UnknownNode;
		return NULL;
	}
	if (register_def) ChainAddEntry(parser->def_nodes, node);

	Node_Register(node, parent);

	/*VRML: "The transformation hierarchy shall be a directed acyclic graph; results are undefined if a node 
	in the transformation hierarchy is its own ancestor"
	that's good, because the scene graph can't handle cyclic graphs (destroy will never be called).
	However we still have to register the node before parsing it, to update node registry and get correct IDs*/
	if (name) {
		if (!undef_node || replace_prev) {
			Node_SetDEF(node, ID, name);
		}
		free(name);
		name = NULL;
	}
	if (!parser->parsing_proto) bt_update_timenode(parser, node);

	if (bt_check_code(parser, '{')) {

		while (1) {
			if (bt_check_code(parser, '}')) 
				break;

			str = bt_get_next(parser, 0);
			if (!str) {
				bt_report(parser, M4InvalidNode, "Invalid node syntax");
				goto err;
			}

			/*we ignore bboxCenter and bboxSize*/
			if (parser->is_wrl && (!strcmp(str, "bboxCenter") || !strcmp(str, "bboxSize"))) {
				Float f;
				bt_parse_float(parser, "x", &f);
				bt_parse_float(parser, "y", &f);
				bt_parse_float(parser, "z", &f);
				continue;
			}
			
			parser->last_error = Node_GetFieldByName(node, str, &info);

			/*check common VRML fields removed in MPEG4*/
			if (parser->last_error) {
				if (!parser->is_wrl) {
					/*we ignore 'solid' for MPEG4 box/cone/etc*/
					if (!strcmp(str, "solid")) {
						Bool b;
						bt_parse_bool(parser, "solid", &b);
						parser->last_error = M4OK;
						continue;
					}
					/*we ignore 'description' for MPEG4 sensors*/
					else if (!strcmp(str, "description")) {
						char *str = bt_get_string(parser); 
						free(str);
						parser->last_error = M4OK;
						continue;
					}
					/*remaps X3D to old VRML/MPEG4*/
					else if ((tag==TAG_MPEG4_LOD) && !strcmp(str, "children")) {
						str = "level";
						parser->last_error = Node_GetFieldByName(node, str, &info);
					}
					else if ((tag==TAG_MPEG4_Switch) && !strcmp(str, "children")) {
						str = "choice";
						parser->last_error = Node_GetFieldByName(node, str, &info);
					}

				} else {
					/*remaps old VRML/MPEG4 to X3D if possible*/
					if ((tag==TAG_X3D_LOD) && !strcmp(str, "level")) {
						str = "children";
						parser->last_error = Node_GetFieldByName(node, str, &info);
					}
					else if ((tag==TAG_X3D_Switch) && !strcmp(str, "choice")) {
						str = "children";
						parser->last_error = Node_GetFieldByName(node, str, &info);
					}
				}
			}

			if (is_script && parser->last_error) {
				u32 eType, fType;
				LPSCRIPTFIELD sf;
				eType = 0;
				if (!strcmp(str, "eventIn") || !strcmp(str, "inputOnly")) eType = SFET_EventIn;
				else if (!strcmp(str, "eventOut") || !strcmp(str, "outputOnly")) eType = SFET_EventOut;
				else if (!strcmp(str, "field") || !strcmp(str, "initializeOnly")) eType = SFET_Field;
				else {
					bt_report(parser, M4InvalidNode, "%s: Unknown script event type", str);
					goto err;
				}
				str = bt_get_next(parser, 0);
				fType = GetFieldTypeByName(str);
				if (fType==FT_Unknown) {
					bt_report(parser, M4InvalidNode, "%s: Unknown script field type", str);
					goto err;
				}
				parser->last_error = 0;
				str = bt_get_next(parser, 0);
				sf = SG_NewScriptField(node, eType, fType, str);
				parser->last_error = Node_GetFieldByName(node, str, &info);

				if (parser->parsing_proto && bt_IS_field(parser, &info, node)) continue;
				if ((eType == SFET_EventIn) || (eType == SFET_EventOut)) continue;
			}
			
			if (parser->last_error) {
				bt_report(parser, M4OK, "%s: Unknown field", str);
				goto err;
			}
			
			if (parser->parsing_proto && bt_IS_field(parser, &info, node)) continue;

			switch (info.fieldType) {
			case FT_SFNode:
				/*if redefining node reset it - this happens with CreateVrmlFromString*/
				if (undef_node==node) {
					SFNode *tmp = * (SFNode **)info.far_ptr;
					if (tmp) Node_Unregister(tmp, node);
				}
				newnode = bt_SFNode(parser, NULL, node, NULL);
				if (!newnode && parser->last_error) goto err;
				if (newnode) {
					if (!BTCheckNDT(parser, &info, newnode, node)) goto err;
					* ((SFNode **)info.far_ptr) = newnode;
				}
				break;
			case FT_MFNode:
			{
				Bool single_child = 0;
				if (!bt_check_code(parser, '[')) {
					if (parser->is_wrl) single_child = 1;
					else break;
				}

				/*if redefining node reset it - this happens with CreateVrmlFromString*/
				if (undef_node==node) {
					Chain *l = *(Chain **)info.far_ptr;
					while (ChainGetCount(l)) {
						SFNode *tmp = ChainGetEntry(l, 0);
						Node_Unregister(tmp, node);
						ChainDeleteEntry(l, 0);
					}
				}

				while (single_child || !bt_check_code(parser, ']')) {
					newnode = bt_SFNode(parser, NULL, node, NULL);
					if (!newnode && parser->last_error) goto err;
					if (newnode) {
						if (!BTCheckNDT(parser, &info, newnode, node)) goto err;
						ChainAddEntry(*(Chain **)info.far_ptr, newnode);
					}
					if (single_child) break;
				}
			}
				break;
			default:
				if (VRML_IsSFField(info.fieldType)) {
					bt_sffield(parser, &info, node);
				} else {
					bt_mffield(parser, &info, node);
				}
				if (parser->last_error) goto err;
				break;
			}
			/*VRML seems to allow that*/
			bt_check_code(parser, ',');
		}
	}
	/*VRML seems to allow that*/
	bt_check_code(parser, ',');

	/*we must init the node once ID is set in case we're creating rendering stacks*/
	if (init_node && (Node_GetTag(node)!=TAG_ProtoNode) ) Node_Init(node);

	/*remove temp node*/
	if (replace_prev) {
		Node_ReplaceAllInstances(undef_node, node, 0);
		ChainDeleteItem(parser->undef_nodes, undef_node);
	}

	if (!parser->parsing_proto && is_script && (parser->load->flags & M4CL_FOR_PLAYBACK) ) {
		if (parser->cur_com) {
			if (!parser->cur_com->scripts_to_load) parser->cur_com->scripts_to_load = NewChain();
			ChainAddEntry(parser->cur_com->scripts_to_load, node);
		} else {
			Script_Load(node);
		}
	}
	return node;

err:
	Node_Unregister(node, parent);
	if (name) free(name);
	return NULL;
}
/*
	locate node, if not defined yet parse ahead in current AU
*/
SFNode *bt_peek_node(BTParser *parser, char *defID)
{
	SFNode *n;
	u32 tag, ID;
	char *str, *ret;
	char nName[1000];
	u32 pos, line, line_pos;
	
	n = SG_FindNodeByName(parser->load->scene_graph, defID);
	if (n) {
		assert(!parser->load->ctx || (n->sgprivate->NodeID <= parser->load->ctx->max_node_id));
		return n;
	}

	pos = parser->line_start_pos;
	line_pos = parser->line_pos;
	line = parser->line;
	strcpy(nName, defID);

	n = NULL;
	while (!parser->done) {
		str = bt_get_next(parser, 0);
		bt_check_code(parser, '[');
		bt_check_code(parser, ']');
		bt_check_code(parser, '{');
		bt_check_code(parser, '}');
		bt_check_code(parser, ',');
		bt_check_code(parser, '.');

		if (!strcmp(str, "AT")) {
			/*only check in current command (but be aware of conditionals..)*/
			if (ChainFindEntry(parser->bifs_au->commands, parser->cur_com)) {
				bt_report(parser, M4BadParam, "Cannot find node %s\n", nName);
				break;
			}
			continue;
		}
		if (strcmp(str, "DEF")) continue;
		str = bt_get_next(parser, 0);
		ret = strdup(str);
		str = bt_get_next(parser, 0);
		if (!strcmp(str, "ROUTE") || strcmp(ret, nName)) {
			free(ret);
			continue;
		}
		tag = bt_get_node_tag(parser, str);
		if (!tag) {
			LPPROTO p;
			LPSCENEGRAPH sg = parser->load->scene_graph;
			while (1) {
				p = SG_FindProto(sg, 0, str);
				if (p) break;
				sg = sg->parent_scene;
				if (!sg) break;
			}
			if (!p) {
				/*locate proto*/
				bt_report(parser, M4InvalidNode, "%s: not a valid/supported node", str);
				return NULL;
			}
			n = Proto_CreateInstance(parser->load->scene_graph, p);
		} else {
			n = BTNewNode(parser, tag);
		}
		ID = bt_GetDEFID(parser, ret);
		Node_SetDEF(n, ID, ret);
		free(ret);

		if (n && !parser->parsing_proto) Node_Init(n);

		/*NO REGISTER on peek (both scene graph or DEF list) because peek is only used to get node type
		and fields, never to insert in the graph*/
		break;
	}
	/*restore context*/
	parser->done = 0;
	gzrewind(parser->gz_in);
	gzseek(parser->gz_in, pos, SEEK_SET);
	parser->line_pos = parser->line_size;
	bt_check_line(parser);
	parser->line = line;
	parser->line_pos = line_pos;
	
	return n;
}

u32 bt_get_route(BTParser *parser, char *name) 
{
	u32 i;
	LPROUTE r = SG_FindRouteByName(parser->load->scene_graph, name);
	if (r) return r->ID;
	for (i=0; i<ChainGetCount(parser->inserted_routes); i++) {
		SGCommand *com = ChainGetEntry(parser->inserted_routes, i);
		if (com->def_name && !strcmp(com->def_name, name)) return com->RouteID;
	}
	return 0;
}

Bool bt_route_id_used(BTParser *parser, u32 ID) 
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

static u32 get_evt_type(char *eventName)
{
	if (!strcmp(eventName, "eventIn") || !strcmp(eventName, "inputOnly")) return ET_EventIn;
	else if (!strcmp(eventName, "eventOut") || !strcmp(eventName, "outputOnly")) return ET_EventOut;
	else if (!strcmp(eventName, "field") || !strcmp(eventName, "initializeOnly")) return ET_Field;
	else if (!strcmp(eventName, "exposedField") || !strcmp(eventName, "inputOutput")) return ET_ExposedField;
	else return ET_Unknown;
}

M4Err bt_parse_proto(BTParser *parser, char *proto_code, Chain *proto_list)
{
	FieldInfo info;
	u32 fType, eType, QPType, pID;
	Bool externProto;
	LPPROTO proto, prevproto;
	LPPROTOFIELD pfield;
	LPSCENEGRAPH sg;
	char *str, *name;
	char szDefName[1024];
	Bool isDEF;

	if (proto_code) 
		str = proto_code;
	else
		str = bt_get_next(parser, 0);
	
	externProto = !strcmp(str, "EXTERNPROTO") ? 1 : 0;
	str = bt_get_next(parser, 0);
	name = strdup(str);
	if (!bt_check_code(parser, '[')) {
		return bt_report(parser, M4BadParam, "[ expected in proto declare");
	}
	pID = bt_get_next_proto_id(parser);
	proto = SG_NewProto(parser->load->scene_graph, pID, name, proto_list ? 1 : 0);
	if (proto_list) ChainAddEntry(proto_list, proto);
	if (parser->load->ctx && (parser->load->ctx->max_proto_id<pID)) parser->load->ctx->max_proto_id = pID;

	/*hack for VRML, where externProto default field values are not mandatory*/
	parser->is_extern_proto_field = externProto;

	free(name);
	/*get all fields*/
	while (!parser->last_error && !bt_check_code(parser, ']')) {
		str = bt_get_next(parser, 0);

next_field:
		if (bt_check_code(parser, ']')) break;

		eType = get_evt_type(str);
		if (eType==ET_Unknown) {
			bt_report(parser, M4BadParam, "%s: Unknown event type", str);
			goto err;
		}
		str = bt_get_next(parser, 0);
		fType = GetFieldTypeByName(str);
		if (fType==FT_Unknown) {
			bt_report(parser, M4BadParam, "%s: Unknown field type", str);
			goto err;
		}
		str = bt_get_next(parser, 0);
		pfield = Proto_NewField(proto, fType, eType, str);
		if ((eType==ET_EventIn) || (eType==ET_EventOut)) continue;

		ProtoField_GetField(pfield, &info);
		if (fType==FT_SFNode) {
			str = bt_get_next(parser, 0);
			if (strcmp(str, "NULL")) {
				if ( (!strlen(str) || (get_evt_type(str)!=ET_Unknown)) && parser->is_extern_proto_field) goto next_field;
				bt_report(parser, M4BadParam, "SFNode protofield must use NULL default value");
				goto err;
			}
		} else if (fType==FT_MFNode) {
			if (bt_check_code(parser, '[')) {
				if (!bt_check_code(parser, ']')) {
					bt_report(parser, M4BadParam, "MFNode protofield must use empty default value");
					goto err;
				}
			}
		} else if (VRML_IsSFField(fType)) {
			bt_sffield(parser, &info, NULL);
			if (parser->last_error==M4FieldNotQuantized) {
				parser->last_error=M4OK;
				goto next_field;
			}
		} else {
			bt_mffield(parser, &info, NULL);
		}
		/*check QP info*/
		if (!bt_check_code(parser, '{')) continue;
		if (bt_check_code(parser, '}')) continue;
		str = bt_get_next(parser, 0);
		if (!strcmp(str, "QP")) {
			u32 nbBits, hasMin;
			Float ftMin, ftMax;
			bt_parse_int(parser, "QPType", &QPType);

			nbBits = 0;
			str = bt_get_next(parser, 0);
			if (!strcmp(str, "nbBits")) {
				bt_parse_int(parser, "nbBits", &nbBits);
				str = bt_get_next(parser, 0);
			}
			hasMin = 0;
			eType = 0;
			if (!strcmp(str, "b")) {
				hasMin = 1;
				if (!bt_check_code(parser, '{')) {
					bt_report(parser, M4BadParam, "%s: Invalid proto coding parameter declare", str);
					goto err;
				}
				bt_parse_float(parser, "min", &ftMin);
				bt_parse_float(parser, "max", &ftMax);
				if (!bt_check_code(parser, '}')) {
					bt_report(parser, M4BadParam, "Invalid proto coding parameter declare");
					goto err;
				}
				if (VRML_GetSFType(fType) == FT_SFInt32) {
					eType = FT_SFInt32;
				} else {
					eType = FT_SFFloat;
				}
			}
			ProtoField_SetQuantizationInfo(pfield, QPType, hasMin, eType, &ftMin, &ftMax, nbBits);
			if (!bt_check_code(parser, '}')) {
				bt_report(parser, M4BadParam, "Invalid proto coding parameter declare");
				goto err;
			}
		}
	}
	parser->is_extern_proto_field = 0;

	if (externProto) {
		SFURL *url;
		u32 nb_urls;
		Bool has_urls = 0;
		if (bt_check_code(parser, '[')) has_urls = 1;

		VRML_MF_Reset(&proto->ExternProto, FT_MFURL);
		nb_urls = 0;
		do {
			str = bt_get_next(parser, 0);
			VRML_MF_Append(&proto->ExternProto, FT_MFURL, (void **) &url);
			if (!strnicmp(str, "od:", 3)) {
				sscanf(str, "od:%d", &url->OD_ID);
			} else {
				if (!sscanf(str, "%d", &url->OD_ID)) {
					url->url = strdup(str);
				} else {
					char szURL[20];
					sprintf(szURL, "%d", url->OD_ID);
					if (strcmp(szURL, str)) {
						url->OD_ID = 0;
						url->url = strdup(str);
					}
				}
			}
			if (has_urls) {
				bt_check_code(parser, ',');
				if (bt_check_code(parser, ']')) has_urls = 0;
			}
		} while (has_urls);
		return M4OK;
	} 

	/*parse proto code */
	if (!bt_check_code(parser, '{')) {
		bt_report(parser, M4OK, "Warning: empty proto body");
		return M4OK;
	}

	prevproto = parser->parsing_proto;
	sg = parser->load->scene_graph;
	parser->parsing_proto = proto;
	parser->load->scene_graph = Proto_GetSceneGraph(proto);

	isDEF = 0;
	while (!bt_check_code(parser, '}')) {
		str = bt_get_next(parser, 0);
		if (!strcmp(str, "PROTO") || !strcmp(str, "EXTERNPROTO")) {
			bt_parse_proto(parser, str, NULL);
		} else if (!strcmp(str, "DEF")) {
			isDEF = 1;
			str = bt_get_next(parser, 0);
			strcpy(szDefName, str);
		} else if (!strcmp(str, "ROUTE")) {
			LPROUTE r = bt_parse_route(parser, 1, 0, NULL);
			if (isDEF) {
				u32 rID = bt_get_route(parser, szDefName);
				if (!rID) {
					rID = bt_get_next_route_id(parser);
					if (parser->load->cbk && (parser->load->ctx->max_route_id<rID)) parser->load->ctx->max_route_id = rID;
				}
				SG_SetRouteID(r, rID);
				SG_SetRouteName(r, szDefName);
				isDEF = 0;
			}
		} else {
			SFNode *n = bt_SFNode(parser, str, NULL, NULL);
			if (!n) goto err;
			if (isDEF) {
				u32 ID = bt_GetDEFID(parser, szDefName);
				isDEF = 0;
				Node_SetDEF(n, ID, szDefName);
			}
			Proto_AddNodeCode(proto, n);
		}
	}
	bt_resolve_routes(parser, 1);
	BT_CheckUnresolvedNodes(parser);
	parser->load->scene_graph = sg;
	parser->parsing_proto = prevproto;
	return parser->last_error;

err:
	if (proto_list) ChainDeleteItem(proto_list, proto);
	SG_DeleteProto(proto);
	return parser->last_error;
}


LPROUTE bt_parse_route(BTParser *parser, Bool skip_def, Bool is_insert, SGCommand *com)
{
	LPROUTE r;
	char *str, nstr[1000], rName[1000];
	u32 rID;
	SFNode *orig, *dest;
	FieldInfo orig_field, dest_field;
	M4Err e;

	rID = 0;
	strcpy(nstr, bt_get_next(parser, 1));
	if (!skip_def && !strcmp(nstr, "DEF")) {
		str = bt_get_next(parser, 0);
		strcpy(rName, str);
		rID = bt_get_route(parser, rName);
		if (!rID && (str[0]=='R') ) {
			rID = atoi(&str[1]);
			if (rID) {
				rID++;
				if (bt_route_id_used(parser, rID)) rID = 0;
			}
		}
		if (!rID) rID = bt_get_next_route_id(parser);
		strcpy(nstr, bt_get_next(parser, 1));
	}
	orig = bt_peek_node(parser, nstr);
	if (!orig) {
		bt_report(parser, M4BadParam, "cannot find node %s", nstr);
		return NULL;
	}
	if (!bt_check_code(parser, '.')) {
		bt_report(parser, M4BadParam, ". expected in route decl");
		return NULL;
	}
	str = bt_get_next(parser, 0);
	e = Node_GetFieldByName(orig, str, &orig_field);
	/*VRML loosy syntax*/
	if ((e != M4OK) && parser->is_wrl && !strnicmp(str, "set_", 4)) 
		e = Node_GetFieldByName(orig, &str[4], &orig_field);

	if ((e != M4OK) && parser->is_wrl && strstr(str, "_changed")) {
		char *s = strstr(str, "_changed");
		s[0] = 0;
		e = Node_GetFieldByName(orig, str, &orig_field);
	}

	if (e != M4OK) {
		bt_report(parser, M4BadParam, "%s not a field of node %s (%s)", str, orig->sgprivate->NodeName, Node_GetName(orig));
		return NULL;
	}
	str = bt_get_next(parser, 0);
	if (strcmp(str, "TO")) {
		bt_report(parser, M4BadParam, "TO expected in route declaration");
		return NULL;
	}

	strcpy(nstr, bt_get_next(parser, 1));
	dest = bt_peek_node(parser, nstr);
	if (!dest) {
		bt_report(parser, M4BadParam, "cannot find node %s", nstr);
		return NULL;
	}
	if (!bt_check_code(parser, '.')) {
		bt_report(parser, M4BadParam, ". expected in route decl");
		return NULL;
	}
	str = bt_get_next(parser, 0);
	e = Node_GetFieldByName(dest, str, &dest_field);
	/*VRML loosy syntax*/
	if ((e != M4OK) && parser->is_wrl && !strnicmp(str, "set_", 4)) 
		e = Node_GetFieldByName(dest, &str[4], &dest_field);
	
	if ((e != M4OK) && parser->is_wrl && strstr(str, "_changed")) {
		char *s = strstr(str, "_changed");
		s[0] = 0;
		e = Node_GetFieldByName(dest, str, &dest_field);
	}

	if (e != M4OK) {
		bt_report(parser, M4BadParam, "%s not a field of node %s (%s)", str, dest->sgprivate->NodeName, Node_GetName(dest));
		return NULL;
	}
	if (com) {
		com->fromNodeID = orig->sgprivate->NodeID;
		com->fromFieldIndex = orig_field.fieldIndex;
		com->toNodeID = dest->sgprivate->NodeID;
		com->toFieldIndex = dest_field.fieldIndex;
		if (rID) {
			com->RouteID = rID;
			com->def_name = strdup(rName);
			/*whenever inserting routes, keep track of max defined ID*/
			if (is_insert) SG_SetMaxDefinedRouteID(parser->load->scene_graph, rID);
		}
		return NULL;
	}
	r = SG_NewRoute(parser->load->scene_graph, orig, orig_field.fieldIndex, dest, dest_field.fieldIndex);
	if (r && rID) {
		SG_SetRouteID(r, rID);
		SG_SetRouteName(r, rName);
	}
	return r;
}

void bt_resolve_routes(BTParser *parser, Bool clean)
{
	SGCommand *com;
	/*resolve all commands*/
	while(ChainGetCount(parser->unresolved_routes) ) {
		com = ChainGetEntry(parser->unresolved_routes, 0);
		ChainDeleteEntry(parser->unresolved_routes, 0);
		switch (com->tag) {
		case SG_RouteDelete:
		case SG_RouteReplace:
			com->RouteID = bt_get_route(parser, com->unres_name);
			if (!com->RouteID) bt_report(parser, M4BadParam, "Cannot resolve Route DEF %s", com->unres_name);
			free(com->unres_name);
			com->unres_name = NULL;
			com->unresolved = 0;
			break;
		}
	}

	if (!clean) return;
	while (ChainGetCount(parser->inserted_routes)) ChainDeleteEntry(parser->inserted_routes, 0);
}


static void bd_set_com_node(SGCommand *com, SFNode *node)
{
	com->node = node;
	Node_Register(com->node, NULL);
}

M4Err bt_parse_bifs_command(BTParser *parser, char *name, Chain *cmdList)
{
	s32 pos;
	LPROUTE r;
	SFNode *n, *newnode;
	SGCommand *com;
	CommandFieldInfo *inf;
	FieldInfo info;
	char *str, field[1000];
	if (!name) {
		str = bt_get_next(parser, 0);
	} else {
		str = name;
	}
	com = NULL;
	pos = -2;
	/*REPLACE commands*/
 	if (!strcmp(str, "REPLACE")) {
		str = bt_get_next(parser, 1);
		if (!strcmp(str, "ROUTE")) {
			str = bt_get_next(parser, 0);
			r = SG_FindRouteByName(parser->load->scene_graph, str);
			if (!r) strcpy(field, str);
			str = bt_get_next(parser, 0);
			if (strcmp(str, "BY")) {
				return bt_report(parser, M4BadParam, "BY expected got %s", str);
			}
			com = SG_NewCommand(parser->load->scene_graph, SG_RouteReplace);
			if (r) {
				com->RouteID = r->ID;
			} else {
				com->unres_name = strdup(field);
				com->unresolved = 1;
				ChainAddEntry(parser->unresolved_routes, com);
			}
			bt_parse_route(parser, 1, 0, com);
			ChainAddEntry(cmdList, com);
			return parser->last_error;
		} 
		/*scene replace*/
		if (!strcmp(str, "SCENE")) {
			str = bt_get_next(parser, 0);
			if (strcmp(str, "BY")) {
				return bt_report(parser, M4BadParam, "BY expected got %s", str);
			}
			bt_resolve_routes(parser, 1);
			com = SG_NewCommand(parser->load->scene_graph, SG_SceneReplace);
			while (ChainGetCount(parser->def_nodes)) ChainDeleteEntry(parser->def_nodes, 0);

			/*note we're extremely lucky in BT, REPLACE SCENE can't use protos, it's just a top scene*/
			n = bt_SFNode(parser, NULL, NULL, NULL);
			if (parser->last_error) goto err;
			com->node = n;
			ChainAddEntry(cmdList, com);
			parser->cur_com = com;
			return M4OK;
		}
		if (!strcmp(str, "LAST")) pos = -1;
		else if (!strcmp(str, "BEGIN")) pos = 0;

		bt_check_code(parser, '.');
		strcpy(field, str);
		n = bt_peek_node(parser, str);
		if (!n) return bt_report(parser, M4BadParam, "%s: unknown node", field);

		str = bt_get_next(parser, 0);
		strcpy(field, str);
		if (bt_check_code(parser, '[')) {
			if ( (parser->last_error = bt_parse_int(parser, "index", &pos)) ) return parser->last_error;
			if (!bt_check_code(parser, ']')) 
				return bt_report(parser, M4BadParam, "] expected");
		}
		/*node replace*/
		if (!strcmp(field, "BY")) {
			com = SG_NewCommand(parser->load->scene_graph, SG_NodeReplace);
			bd_set_com_node(com, n);
			inf = SG_NewFieldCommand(com);
			inf->new_node = bt_SFNode(parser, NULL, n, NULL);
			inf->fieldType = FT_SFNode;
			inf->field_ptr = &inf->new_node;
			ChainAddEntry(cmdList, com);
			parser->cur_com = com;
			return parser->last_error;
		}
		str = bt_get_next(parser, 0);
		if (strcmp(str, "BY")) return bt_report(parser, M4BadParam, "BY expected got %s", str);

		parser->last_error = Node_GetFieldByName(n, field, &info);
		if (parser->last_error) 
			return bt_report(parser, parser->last_error, "%s: Unknown node field", field);

		/*field replace*/
		if (pos==-2) {
			com = SG_NewCommand(parser->load->scene_graph, SG_FieldReplace);
			bd_set_com_node(com, n);
			inf = SG_NewFieldCommand(com);
			inf->fieldIndex = info.fieldIndex;
			inf->fieldType = info.fieldType;

			switch (info.fieldType) {
			case FT_SFNode:
				inf->new_node = bt_SFNode(parser, NULL, n, NULL);
				inf->field_ptr = &inf->new_node;
				if (!BTCheckNDT(parser, &info, inf->new_node, n)) goto err;
				break;
			case FT_MFNode:
				if (!bt_check_code(parser, '[')) break;
				inf->node_list = NewChain();
				inf->field_ptr = &inf->node_list;
				while (!bt_check_code(parser, ']')) {
					newnode = bt_SFNode(parser, NULL, n, NULL);
					if (!newnode) goto err;
					if (parser->last_error!=M4OK) goto err;
					if (!BTCheckNDT(parser, &info, newnode, n)) goto err;
					ChainAddEntry(inf->node_list, newnode);
				}
				break;
			default:
				inf->field_ptr = VRML_NewFieldPointer(info.fieldType);
				info.far_ptr = inf->field_ptr;
				if (VRML_IsSFField(info.fieldType)) {
					bt_sffield(parser, &info, n);
				} else {
					bt_mffield(parser, &info, n);
				}
				if (parser->last_error) goto err;
				break;
			}

			ChainAddEntry(cmdList, com);
			parser->cur_com = com;
			return parser->last_error;
		}
		/*indexed field replace*/
		com = SG_NewCommand(parser->load->scene_graph, SG_IndexedReplace);
		bd_set_com_node(com, n);
		inf = SG_NewFieldCommand(com);
		inf->pos = pos;
		inf->fieldIndex = info.fieldIndex;
		if (VRML_IsSFField(info.fieldType)) {
			bt_report(parser, M4BadParam, "%s: MF type field expected", info.name);
			goto err;
		}
		inf->fieldType = VRML_GetSFType(info.fieldType);
		switch (info.fieldType) {
		case FT_MFNode:
			inf->new_node = bt_SFNode(parser, NULL, n, NULL);
			inf->field_ptr = &inf->new_node;
 			break;
		default:
			info.fieldType = inf->fieldType;
			info.far_ptr = inf->field_ptr = VRML_NewFieldPointer(inf->fieldType);
			bt_sffield(parser, &info, n);
			break;
		}
		if (parser->last_error) goto err;
		ChainAddEntry(cmdList, com);
		parser->cur_com = com;
		return parser->last_error;
	}
	/*INSERT commands*/
	if (!strcmp(str, "INSERT") || !strcmp(str, "APPEND")) {
		Bool is_append = !strcmp(str, "APPEND") ? 1 : 0;
		str = bt_get_next(parser, 0);
		if (!strcmp(str, "ROUTE")) {
			com = SG_NewCommand(parser->load->scene_graph, SG_RouteInsert);
			bt_parse_route(parser, 0, 1, com);
			if (parser->last_error) goto err;
			ChainAddEntry(cmdList, com);
			ChainAddEntry(parser->inserted_routes, com);
			parser->cur_com = com;
			return M4OK;
		}
		if (strcmp(str, "AT") && strcmp(str, "TO")) {
			return bt_report(parser, M4BadParam, is_append ? "TO expected got %s" : "AT expected got %s", str);
		}
		str = bt_get_next(parser, 1);
		strcpy(field, str);
		n = bt_peek_node(parser, str);
		if (!n) {
			return bt_report(parser, M4BadParam, "%s: Unknown node", field);
		}
		if (!bt_check_code(parser, '.')) {
			return bt_report(parser, M4BadParam, ". expected");
		}
		str = bt_get_next(parser, 1);
		strcpy(field, str);
		if (!is_append) {
			if (!bt_check_code(parser, '[')) {
				return bt_report(parser, M4BadParam, "[ expected");
			}
			bt_parse_int(parser, "index", &pos);
			if (!bt_check_code(parser, ']')) {
				return bt_report(parser, M4BadParam, "] expected");
			}
		} else {
			if (bt_check_code(parser, '[')) {
				return bt_report(parser, M4BadParam, "[ unexpected in Append command");
			}
			pos = -1;
		}
		if (!strcmp(field, "children")) {
			com = SG_NewCommand(parser->load->scene_graph, SG_NodeInsert);
			bd_set_com_node(com, n);
			inf = SG_NewFieldCommand(com);
			inf->pos = pos;
			inf->new_node = bt_SFNode(parser, NULL, n, NULL);
			inf->fieldType = FT_SFNode;
			inf->field_ptr = &inf->new_node;
			if (parser->last_error) goto err;
			parser->cur_com = com;
			return ChainAddEntry(cmdList, com);
		}
		Node_GetFieldByName(n, field, &info);
		if (VRML_IsSFField(info.fieldType)) {
			bt_report(parser, M4BadParam, "%s: MF type field expected", info.name);
			goto err;
		}
		com = SG_NewCommand(parser->load->scene_graph, SG_IndexedInsert);
		bd_set_com_node(com, n);
		inf = SG_NewFieldCommand(com);
		inf->pos = pos;
		inf->fieldIndex = info.fieldIndex;
		inf->fieldType = VRML_GetSFType(info.fieldType);
		switch (info.fieldType) {
		case FT_MFNode:
			inf->new_node = bt_SFNode(parser, NULL, n, NULL);
			inf->field_ptr = &inf->new_node;
			break;
		default:
			info.fieldType = inf->fieldType;
			inf->field_ptr = VRML_NewFieldPointer(inf->fieldType);
			info.far_ptr = inf->field_ptr;
			bt_sffield(parser, &info, n);
			break;
		}
		if (parser->last_error) goto err;
		ChainAddEntry(cmdList, com);
		parser->cur_com = com;
		return parser->last_error;
	}
	/*DELETE commands*/
	if (!strcmp(str, "DELETE")) {
		str = bt_get_next(parser, 1);
		if (!strcmp(str, "ROUTE")) {
			str = bt_get_next(parser, 0);
			com = SG_NewCommand(parser->load->scene_graph, SG_RouteDelete);
			com->RouteID = bt_get_route(parser, str);
			if (!com->RouteID) {
				com->unres_name = strdup(str);
				com->unresolved = 1;
				ChainAddEntry(parser->unresolved_routes, com);
			}
			/*for bt<->xmt conversions*/
			com->def_name = strdup(str);
			return ChainAddEntry(cmdList, com);
		}
		strcpy(field, str);
		n = bt_peek_node(parser, str);
		if (!n) {
			return bt_report(parser, M4BadParam, "DELETE %s: Unknown Node", field);
		}
		if (!bt_check_code(parser, '.')) {
			com = SG_NewCommand(parser->load->scene_graph, SG_NodeDelete);
			bd_set_com_node(com, n);
			return ChainAddEntry(cmdList, com);
		}
		str = bt_get_next(parser, 0);
		Node_GetFieldByName(n, str, &info);
		if (!bt_check_code(parser, '[')) {
			return bt_report(parser, M4BadParam, "[ expected");
		}
		bt_parse_int(parser, "index", &pos);
		if (!bt_check_code(parser, ']')) {
			return bt_report(parser, M4BadParam, "[ expected");
		}
		com = SG_NewCommand(parser->load->scene_graph, SG_IndexedDelete);
		bd_set_com_node(com, n);
		inf = SG_NewFieldCommand(com);
		inf->fieldIndex = info.fieldIndex;
		inf->pos = pos;
		return ChainAddEntry(cmdList, com);
	}
	/*Extended BIFS commands*/

	/*GlobalQP commands*/
	if (!strcmp(str, "GLOBALQP")) {
		newnode = bt_SFNode(parser, NULL, NULL, NULL);
		if (newnode && (newnode->sgprivate->tag != TAG_MPEG4_QuantizationParameter)) {
			bt_report(parser, M4BadParam, "Only QuantizationParameter node allowed in GLOBALQP");
			Node_Unregister(newnode, NULL);
			return parser->last_error;
		}
		com = SG_NewCommand(parser->load->scene_graph, SG_GlobalQuantizer);
		com->node = NULL;
		inf = SG_NewFieldCommand(com);
		inf->new_node = newnode;
		inf->field_ptr = &inf->new_node;
		inf->fieldType = FT_SFNode;
		return ChainAddEntry(cmdList, com);
	}

	/*MultipleReplace commands*/
	if (!strcmp(str, "MULTIPLEREPLACE")) {
		str = bt_get_next(parser, 0);
		strcpy(field, str);
		n = bt_peek_node(parser, str);
		if (!n) {
			return bt_report(parser, M4BadParam, "%s: Unknown node", field);
		}
		if (!bt_check_code(parser, '{')) {
			return bt_report(parser, M4BadParam, "{ expected");
		}
		com = SG_NewCommand(parser->load->scene_graph, SG_MultipleReplace);
		bd_set_com_node(com, n);

		while (!bt_check_code(parser, '}')) {
			str = bt_get_next(parser, 0);
			parser->last_error = Node_GetFieldByName(n, str, &info);
			if (parser->last_error) {
				bt_report(parser, M4BadParam, "%s: Unknown node field", str);
				goto err;
			}
			inf = SG_NewFieldCommand(com);
			inf->fieldIndex = info.fieldIndex;
			inf->fieldType = info.fieldType;
			inf->pos = -1;

			switch (info.fieldType) {
			case FT_SFNode:
				inf->new_node = bt_SFNode(parser, NULL, n, NULL);
				if (parser->last_error) goto err;
				if (!BTCheckNDT(parser, &info, inf->new_node, n)) goto err;
				inf->field_ptr = &inf->new_node;
				break;
			case FT_MFNode:
				if (!bt_check_code(parser, '[')) {
					bt_report(parser, M4BadParam, "[ expected");
					goto err;
				}
				inf->node_list = NewChain();
				info.far_ptr = inf->field_ptr = &inf->node_list;
				while (!bt_check_code(parser, ']')) {
					newnode = bt_SFNode(parser, NULL, n, NULL);
					if (parser->last_error!=M4OK) goto err;
					if (!BTCheckNDT(parser, &info, newnode, n)) goto err;
					ChainAddEntry(inf->node_list, newnode);
				}
				break;
			default:
				info.far_ptr = inf->field_ptr = VRML_NewFieldPointer(inf->fieldType);
				if (VRML_IsSFField(info.fieldType)) {
					bt_sffield(parser, &info, n);
				} else {
					bt_mffield(parser, &info, n);
				}
				if (parser->last_error) goto err;
				break;
			}
		}
		parser->cur_com = com;
		return ChainAddEntry(cmdList, com);
	}

	/*MultipleIndexReplace commands*/
	if (!strcmp(str, "MULTIPLEINDREPLACE")) {
		str = bt_get_next(parser, 1);
		strcpy(field, str);
		n = bt_peek_node(parser, str);
		if (!n) {
			return bt_report(parser, M4BadParam, "%s: Unknown node", field);
		}
		if (!bt_check_code(parser, '.')) {
			return bt_report(parser, M4BadParam, ". expected");
		}
		str = bt_get_next(parser, 0);
		parser->last_error = Node_GetFieldByName(n, str, &info);
		if (parser->last_error) {
			return bt_report(parser, M4BadParam, "%s: Unknown field", info.name);
		}
		if (VRML_IsSFField(info.fieldType)) {
			return bt_report(parser, M4BadParam, "Only MF field allowed");
		}
		if (!bt_check_code(parser, '[')) {
			return bt_report(parser, M4BadParam, "[ expected");
		}

		com = SG_NewCommand(parser->load->scene_graph, SG_MultipleIndexedReplace);
		bd_set_com_node(com, n);
		info.fieldType = VRML_GetSFType(info.fieldType);

		while (!bt_check_code(parser, ']')) {
			u32 pos;
			if (bt_parse_int(parser, "position", &pos)) goto err;
			str = bt_get_next(parser, 0);
			if (strcmp(str, "BY")) {
				bt_report(parser, M4BadParam, "BY expected");
				goto err;
			}
			inf = SG_NewFieldCommand(com);
			inf->fieldIndex = info.fieldIndex;
			inf->fieldType = info.fieldType;
			inf->pos = pos;
			if (inf->fieldType==FT_SFNode) {
				info.far_ptr = inf->field_ptr = &inf->new_node;
				inf->new_node = bt_SFNode(parser, NULL, n, NULL);
				if (parser->last_error) goto err;
				if (!BTCheckNDT(parser, &info, inf->new_node, n)) goto err;
				inf->field_ptr = &inf->new_node;
			} else {
				info.far_ptr = inf->field_ptr = VRML_NewFieldPointer(inf->fieldType);
				bt_sffield(parser, &info, n);
				if (parser->last_error) goto err;
			}
		}
		parser->cur_com = com;
		return ChainAddEntry(cmdList, com);
	}

	if (!strcmp(str, "XDELETE")) {
		str = bt_get_next(parser, 1);
		strcpy(field, str);
		n = bt_peek_node(parser, str);
		if (!n) {
			return bt_report(parser, M4BadParam, "%s: Unknown Node", field);
		}
		com = SG_NewCommand(parser->load->scene_graph, SG_NodeDeleteEx);
		bd_set_com_node(com, n);
		return ChainAddEntry(cmdList, com);
	}

	if (!strcmp(str, "INSERTPROTO")) {
		if (!bt_check_code(parser, '[')) {
			return bt_report(parser, M4BadParam, "[ expected");
		}
		com = SG_NewCommand(parser->load->scene_graph, SG_ProtoInsert);
		while (!bt_check_code(parser, ']')) {
			parser->last_error = bt_parse_proto(parser, NULL, com->new_proto_list);
			if (parser->last_error) goto err;
		}
		ChainAddEntry(cmdList, com);
		return M4OK;
	}
	if (!strcmp(str, "DELETEPROTO")) {
		if (!bt_check_code(parser, '[')) {
			com = SG_NewCommand(parser->load->scene_graph, SG_ProtoDeleteAll);
			str = bt_get_next(parser, 0);
			if (strcmp(str, "ALL")) {
				bt_report(parser, M4BadParam, "ALL expected");
				goto err;
			}
			return ChainAddEntry(cmdList, com);
		}
		com = SG_NewCommand(parser->load->scene_graph, SG_ProtoDelete);
		while (!bt_check_code(parser, ']')) {
			LPPROTO proto;
			str = bt_get_next(parser, 0);
			proto = SG_FindProto(parser->load->scene_graph, 0, str);
			if (!proto) {
				bt_report(parser, M4BadParam, "%s: Unknown proto", str);
				goto err;
			}
			com->del_proto_list = realloc(com->del_proto_list, sizeof(u32)*(com->del_proto_list_size+1));
			com->del_proto_list[com->del_proto_list_size] = proto->ID;
			com->del_proto_list_size++;
		}
		ChainAddEntry(cmdList, com);
		return M4OK;
	}
	return bt_report(parser, M4BadParam, "%s: Unknown command syntax, str");

err:
	if (com) SG_DeleteCommand(com);
	return parser->last_error;
}

Descriptor *bt_parse_descriptor(BTParser *parser, char *name)
{
	char *str, field[500];
	Descriptor *desc, *subdesc;
	u32 type;
	u8 tag;
	if (name) {
		str = name;
	} else {
		str = bt_get_next(parser, 0);
	}
	tag = OD_GetDescriptorTag(str);
	if (!tag) {
		bt_report(parser, M4BadParam, "%s: Unknown descriptor", str);
		return NULL;
	}
	desc = OD_NewDescriptor(tag);

	if (!desc) return NULL;
	if (!bt_check_code(parser, '{')) return desc;

	while (1) {
		/*done*/
		if (bt_check_code(parser, '}')) break;
		str = bt_get_next(parser, 0);
		strcpy(field, str);
		type = OD_DescriptorFieldType(desc, str);
		switch (type) {
		case 2:
			if (bt_check_code(parser, '[')) {
				while (!bt_check_code(parser, ']')) {
					subdesc = bt_parse_descriptor(parser, NULL);
					if (!subdesc) {
						OD_DeleteDescriptor(&desc);
						parser->last_error = M4BadParam;
						return NULL;
					}
					OD_AddDescToDesc(desc, subdesc);
				}
			}
			break;
		case 1:
			str = bt_get_next(parser, 0);
			subdesc = bt_parse_descriptor(parser, str);
			if (!subdesc) {
				bt_report(parser, M4BadParam, "Unknown desc %s in field %s", str, field);
				OD_DeleteDescriptor(&desc);
				return NULL;
			}
			OD_AddDescToDesc(desc, subdesc);
			break;
		default:
			str = bt_get_next(parser, 0);
			parser->last_error = OD_SetDescriptorField(desc, field, str);

			if (parser->last_error) {
				bt_report(parser, M4BadParam, "Unknown desc %s in field %s", str, field);
				OD_DeleteDescriptor(&desc);
				return NULL;
			}
			break;
		}
	}
	if (desc->tag == BIFSConfig_Tag) {
		BIFSConfigDescriptor *bcfg = (BIFSConfigDescriptor *)desc;
		parser->load->ctx->scene_width = bcfg->pixelWidth;
		parser->load->ctx->scene_height = bcfg->pixelHeight;
		parser->load->ctx->is_pixel_metrics = bcfg->pixelMetrics;

		/*for bt->xmt*/
		if (!bcfg->isCommandStream) bcfg->isCommandStream = 1;
		if (!bcfg->version) bcfg->version = 1;
	}
	else if (desc->tag==ESDescriptor_Tag) {
		ESDescriptor *esd  =(ESDescriptor*)desc;
		if (esd->decoderConfig) {
			/*watchout for default BIFS stream*/
			if (parser->bifs_es && !parser->base_bifs_id && (esd->decoderConfig->streamType==M4ST_SCENE)) {
				parser->bifs_es->ESID = parser->base_bifs_id = esd->ESID;
				parser->bifs_es->timeScale = esd->slConfig ? esd->slConfig->timestampResolution : 1000;
			} else {
				M4StreamContext *sc = M4SM_NewStream(parser->load->ctx, esd->ESID, esd->decoderConfig->streamType, esd->decoderConfig->objectTypeIndication);
				/*set default timescale for systems tracks (ignored for other)*/
				if (sc) sc->timeScale = esd->slConfig ? esd->slConfig->timestampResolution : 1000;
				/*assign base OD*/
				if (!parser->base_od_id && (esd->decoderConfig->streamType==M4ST_OD)) parser->base_od_id = esd->ESID;
			}
		}
	}
	return desc;
}

void bt_parse_od_command(BTParser *parser, char *name)
{
	u32 val;
	char *str;
	Descriptor *desc;
	
	if (!strcmp(name, "UPDATE")) {
		str = bt_get_next(parser, 0);
		/*OD update*/
		if (!strcmp(str, "OD")) {
			ObjectDescriptorUpdate *odU;
			if (!bt_check_code(parser, '[')) {
				bt_report(parser, M4BadParam, "[ expected");
 				return;
			}
			odU = (ObjectDescriptorUpdate *) OD_NewCommand(ODUpdate_Tag);
			ChainAddEntry(parser->od_au->commands, odU);
			while (!parser->done) {
				str = bt_get_next(parser, 0);
				if (bt_check_code(parser, ']')) break;
				if (strcmp(str, "ObjectDescriptor") && strcmp(str, "InitialObjectDescriptor")) {
					bt_report(parser, M4BadParam, "Object Descriptor expected got %s", str);
					break;
				}
				desc = bt_parse_descriptor(parser, str);
				if (!desc) break;
				ChainAddEntry(odU->objectDescriptors, desc);
			}
			return;
		}
		/*ESD update*/
		if (!strcmp(str, "ESD")) {
			ESDescriptorUpdate *esdU;
			str = bt_get_next(parser, 0);
			if (strcmp(str, "IN")) {
				bt_report(parser, M4BadParam, "IN expected got %s", str);
 				return;
			}
			esdU = (ESDescriptorUpdate *) OD_NewCommand(ESDUpdate_Tag);
			parser->last_error = bt_parse_int(parser, "OD_ID", &val);
			if (parser->last_error) return;
			esdU->ODID = val;
			ChainAddEntry(parser->od_au->commands, esdU);

			if (!bt_check_code(parser, '[')) {
				str = bt_get_next(parser, 0);
				if (strcmp(str, "esDescr")) {
					bt_report(parser, M4BadParam, "esDescr expected got %s", str);
 					return;
				}
				if (!bt_check_code(parser, '[')) {
					bt_report(parser, M4BadParam, "[ expected");
 					return;
				}
			}

			while (!parser->done) {
				str = bt_get_next(parser, 0);
				if (bt_check_code(parser, ']')) break;
				if (strcmp(str, "ESDescriptor")) {
					bt_report(parser, M4BadParam, "ESDescriptor expected got %s", str);
					break;
				}
				desc = bt_parse_descriptor(parser, str);
				if (!desc) break;
				ChainAddEntry(esdU->ESDescriptors, desc);
			}
			return;
		}
		bt_report(parser, M4BadParam, "unknown OD command", str);
		return;
	}
	if (!strcmp(name, "REMOVE")) {
		str = bt_get_next(parser, 0);
		/*OD remove*/
		if (!strcmp(str, "OD")) {
			ObjectDescriptorRemove *odR;
			if (!bt_check_code(parser, '[')) {
				bt_report(parser, M4BadParam, "[ expected");
 				return;
			}
			odR = (ObjectDescriptorRemove *) OD_NewCommand(ODRemove_Tag);
			ChainAddEntry(parser->od_au->commands, odR);
			while (!parser->done) {
				u32 id;
				if (bt_check_code(parser, ']')) break;
				bt_parse_int(parser, "ODID", &id);
				if (parser->last_error) return;
				odR->OD_ID = realloc(odR->OD_ID, sizeof(u16) * (odR->NbODs+1));
				odR->OD_ID[odR->NbODs] = id;
				odR->NbODs++;
			}
			return;
		}
		/*ESD remove*/
		if (!strcmp(str, "ESD")) {
			u32 odid;
			ESDescriptorRemove *esdR;
			str = bt_get_next(parser, 0);
			if (strcmp(str, "FROM")) {
				bt_report(parser, M4BadParam, "FROM expected got %s", str);
 				return;
			}
			bt_parse_int(parser, "ODID", &odid);
			if (parser->last_error) return;

			if (!bt_check_code(parser, '[')) {
				bt_report(parser, M4BadParam, "[ expected");
 				return;
			}
			esdR = (ESDescriptorRemove *) OD_NewCommand(ESDRemove_Tag);
			esdR->ODID = odid;
			ChainAddEntry(parser->od_au->commands, esdR);
			while (!parser->done) {
				u32 id;
				if (bt_check_code(parser, ']')) break;
				bt_parse_int(parser, "ES_ID", &id);
				if (parser->last_error) return;
				esdR->ES_ID = realloc(esdR->ES_ID, sizeof(u16) * (esdR->NbESDs+1));
				esdR->ES_ID[esdR->NbESDs] = id;
				esdR->NbESDs++;
			}
			return;
		}
		bt_report(parser, M4BadParam, "%s: Unknown OD command", str);
		return;
	}
}


M4Err M4SM_LoaderRun_BT_intern(BTParser *parser, SGCommand *init_com)
{
	char *str;
	SFNode *node, *vrml_root_node;
	Bool in_com;
	LPROUTE r;
	Bool has_id;
	char szDEFName[1000];

	vrml_root_node = NULL;
	has_id = 0;
	in_com = init_com ? 0 : 1;
	parser->cur_com = init_com;

	/*parse all top-level items*/
	while (!parser->last_error) {
		str = bt_get_next(parser, 0);
		if (parser->done) break;
		
		/*create a default root node for all VRML nodes*/
		if ((parser->is_wrl && !parser->top_nodes) && init_com && !vrml_root_node) {
			vrml_root_node = SG_NewNode(parser->load->scene_graph, (parser->load->flags & M4CL_MPEG4_STRICT) ? TAG_MPEG4_Group : TAG_X3D_Group);
			Node_Register(vrml_root_node, NULL);
			Node_Init(vrml_root_node);
			init_com->node = vrml_root_node;
		}
		/*X3D specific things (ignored for now)*/
		if (!strcmp(str, "PROFILE")) bt_force_line(parser);
		else if (!strcmp(str, "COMPONENT")) bt_force_line(parser);
		else if (!strcmp(str, "META")) bt_force_line(parser);
		else if (!strcmp(str, "IMPORT") || !strcmp(str, "EXPORT")) {
			bt_report(parser, M4NotSupported, "X3D IMPORT/EXPORT not implemented");
			break;
		}

		/*IOD*/
		if (!strcmp(str, "InitialObjectDescriptor") || !strcmp(str, "ObjectDescriptor")) {
			parser->load->ctx->root_od = (ObjectDescriptor *) bt_parse_descriptor(parser, str);
		}
		/*explicit command*/
		else if (!strcmp(str, "AT") || !strcmp(str, "RAP")) {
			parser->au_is_rap = 0;
			if (!strcmp(str, "RAP")) {
				parser->au_is_rap = 1;
				str = bt_get_next(parser, 0);
				if (strcmp(str, "AT")) {
					bt_report(parser, M4BadParam, "AT expected got %s", str);
					parser->last_error = M4BadParam;
					break;
				}
			}
			str = bt_get_next(parser, 0);
			if (str[0] == 'D') {
				parser->au_time += atoi(&str[1]);
			} else {
				u32 i;
				for (i=0; i<strlen(str); i++) {
					if (!isdigit(str[i]) && (str[i] != 'E') && (str[i] != 'e') && (str[i] != '-')) {
						bt_report(parser, M4BadParam, "Number expected got %s", str);
						break;
					}
				}
				if (!i) {
					bt_report(parser, M4BadParam, "Number expected got %s", str);
					break;
				}
				parser->au_time = atoi(str);
			}
			if (parser->last_error) break;
			/*reset all contexts*/
			if (parser->od_au && (parser->od_au->timing != parser->au_time)) parser->od_au = NULL;
			if (parser->bifs_au && (parser->bifs_au->timing != parser->au_time)) {
				BT_CheckUnresolvedNodes(parser);
				parser->bifs_au = NULL;
			} 

			parser->stream_id = 0;
			/*fix for mp4tool bt which doesn't support RAP signaling: assume the first AU
			is always RAP*/
			if (!parser->au_time) parser->au_is_rap = 1;
	
			in_com = 1;

			if (!bt_check_code(parser, '{')) {
				str = bt_get_next(parser, 0);
				if (!strcmp(str, "IN")) {
					bt_parse_int(parser, "IN", &parser->stream_id);
					if (parser->last_error) break;
				}
				if (!bt_check_code(parser, '{')) {
					bt_report(parser, M4BadParam, "{ expected");
				}
			}
			/*done loading init frame*/
			if (init_com) break;
		}
		else if (!strcmp(str, "PROTO") || !strcmp(str, "EXTERNPROTO")) {
			bt_parse_proto(parser, str, init_com ? init_com->new_proto_list : NULL);
		}
		/*compatibility for old bt (mp4tool) in ProtoLibs*/
		else if (!strcmp(str, "NULL")) {
		}
		else if (!strcmp(str, "DEF")) {
			str = bt_get_next(parser, 0);
			strcpy(szDEFName, str);
			has_id = 1;
		}
		else if (!strcmp(str, "ROUTE")) {
			SGCommand *com = NULL;
			if (!parser->top_nodes) {
				/*if doing a scene replace, we need route insert stuff*/
				com = SG_NewCommand(parser->load->scene_graph, SG_RouteInsert);
				ChainAddEntry(parser->bifs_au->commands, com);
				ChainAddEntry(parser->inserted_routes, com);
			}

			r = bt_parse_route(parser, 1, 0, com);
			if (has_id) {
				u32 rID = bt_get_next_route_id(parser);
				if (com) {
					com->RouteID = rID;
					com->def_name = strdup(szDEFName);
					SG_SetMaxDefinedRouteID(parser->load->scene_graph, rID);
				} else if (r) {
					SG_SetRouteID(r, rID);
					SG_SetRouteName(r, szDEFName);
				}
				has_id = 0;
			}
		}
		/*OD commands*/
		else if (!strcmp(str, "UPDATE") || !strcmp(str, "REMOVE")) {
			if (!parser->stream_id || parser->stream_id==parser->bifs_es->ESID) parser->stream_id = parser->base_od_id;

			if (parser->od_es && (parser->od_es->ESID != parser->stream_id)) {
				M4StreamContext *prev = parser->od_es;
				parser->od_es = M4SM_NewStream(parser->load->ctx, (u16) parser->stream_id, M4ST_OD, 0);
				/*force new AU if stream changed*/
				if (parser->od_es != prev) parser->bifs_au = NULL;
			}
			if (!parser->od_es) parser->od_es = M4SM_NewStream(parser->load->ctx, (u16) parser->stream_id, M4ST_OD, 0);
			if (!parser->od_au) parser->od_au = M4SM_NewAU(parser->od_es, parser->au_time, 0, parser->au_is_rap);
			bt_parse_od_command(parser, str);
		}
		/*BIFS commands*/
		else if (!strcmp(str, "REPLACE") || !strcmp(str, "INSERT") || !strcmp(str, "APPEND") || !strcmp(str, "DELETE")
			/*BIFS extended commands*/
			|| !strcmp(str, "GLOBALQP") || !strcmp(str, "MULTIPLEREPLACE") || !strcmp(str, "MULTIPLEINDREPLACE") || !strcmp(str, "XDELETE") || !strcmp(str, "DELETEPROTO") || !strcmp(str, "INSERTPROTO") ) {

			if (!parser->stream_id) parser->stream_id = parser->base_bifs_id;
			if (!parser->stream_id || (parser->od_es && (parser->stream_id==parser->od_es->ESID)) ) parser->stream_id = parser->base_bifs_id;

			if (parser->bifs_es->ESID != parser->stream_id) {
				M4StreamContext *prev = parser->bifs_es;
				parser->bifs_es = M4SM_NewStream(parser->load->ctx, (u16) parser->stream_id, M4ST_SCENE, 0);
				/*force new AU if stream changed*/
				if (parser->bifs_es != prev) {
					BT_CheckUnresolvedNodes(parser);
					parser->bifs_au = NULL;
				}
			}
			if (!parser->bifs_au) parser->bifs_au = M4SM_NewAU(parser->bifs_es, parser->au_time, 0, parser->au_is_rap);
			bt_parse_bifs_command(parser, str, parser->bifs_au->commands);
		}
		/*implicit BIFS command on SFTopNodes only*/
		else if (!strcmp(str, "OrderedGroup") 
			|| !strcmp(str, "Group") 
			|| !strcmp(str, "Layer2D") 
			|| !strcmp(str, "Layer3D")
			 /* VRML parsing: all nodes are allowed*/
			|| parser->is_wrl 
			) 
		{
			node = bt_SFNode(parser, str, vrml_root_node, has_id ? szDEFName : NULL);
			has_id = 0;
			if (!node) break;
			if (parser->top_nodes) {
				ChainAddEntry(parser->top_nodes, node);
			} else if (!vrml_root_node) {
				init_com->node = node;
			} else {
				Node_InsertChild(vrml_root_node, node, -1);
			}
		} 

		/*if in command, check command end*/
		else {
			/*check command end*/
			if (in_com && bt_check_code(parser, '}')) in_com = 0;
			else if (strlen(str)) {
				bt_report(parser, M4BadParam, "%s: Unknown top-level element", str);
			}
			parser->au_is_rap = 0;
		}
	}
	bt_resolve_routes(parser, 0);
	BT_CheckUnresolvedNodes(parser);
	return parser->last_error;
}

M4Err M4SM_LoaderInit_BT(M4ContextLoader *load)
{
	gzFile gzInput;
	M4Err e;
	BTParser *parser;
	SGCommand *com;
	unsigned char BOM[5];

	if (!load->ctx || !load->fileName) return M4BadParam;
	gzInput = gzopen(load->fileName, "rb");
	if (!gzInput) return M4URLNotFound;

	SAFEALLOC(parser, sizeof(BTParser));
	parser->load = load;
	SAFEALLOC(parser->line_buffer, sizeof(char)*BT_LINE_SIZE);

	gzgets(gzInput, BOM, 5);
	gzseek(gzInput, 0, SEEK_SET);

	/*0: no unicode, 1: UTF-16BE, 2: UTF-16LE*/
	if ((BOM[0]==0xFF) && (BOM[1]==0xFE)) {
		if (!BOM[2] && !BOM[3]) {
			bt_report(parser, M4NotSupported, "UTF-32 Text Files not supported");
			gzclose(gzInput);
			free(parser);
			return M4NotSupported;
		} else {
			parser->unicode_type = 2;
			gzseek(gzInput, 2, SEEK_CUR);
		}
	} else if ((BOM[0]==0xFE) && (BOM[1]==0xFF)) {
		if (!BOM[2] && !BOM[3]) {
			bt_report(parser, M4NotSupported, "UTF-32 Text Files not supported");
			gzclose(gzInput);
			free(parser);
			return M4NotSupported;
		} else {
			parser->unicode_type = 1;
			gzseek(gzInput, 2, SEEK_CUR);
		}
	} else if ((BOM[0]==0xEF) && (BOM[1]==0xBB) && (BOM[2]==0xBF)) {
		/*we handle UTF8 as asci*/
		parser->unicode_type = 0;
		gzseek(gzInput, 3, SEEK_CUR);
	}
	parser->gz_in = gzInput;
	load->loader_priv = parser;

	parser->unresolved_routes = NewChain();
	parser->inserted_routes = NewChain();
	parser->undef_nodes = NewChain();
	parser->def_nodes = NewChain();

	/*create at least one empty BIFS stream*/
	parser->bifs_es = M4SM_NewStream(load->ctx, 0, M4ST_SCENE, 0);
	parser->bifs_au = M4SM_NewAU(parser->bifs_es, 0, 0, 1);

	/*default scene replace - we create it no matter what since it is used to store BIFS config
	when parsing IOD.*/
	com = SG_NewCommand(parser->load->scene_graph, SG_SceneReplace);
	ChainAddEntry(parser->bifs_au->commands, com);
	e = M4SM_LoaderRun_BT_intern(parser, com);
	if (e) M4SM_LoaderDone_BT(load);
	return e;
}

void M4SM_LoaderDone_BT(M4ContextLoader *load)
{
	BTParser *parser = (BTParser *)load->loader_priv;
	if (!parser) return;
	DeleteChain(parser->unresolved_routes);
	DeleteChain(parser->inserted_routes);
	DeleteChain(parser->undef_nodes);
	DeleteChain(parser->def_nodes);
	gzclose(parser->gz_in);
	free(parser->line_buffer);
	free(parser);
	load->loader_priv = NULL;
	return;
}



M4Err M4SM_LoaderRun_BT(M4ContextLoader *load)
{
	BTParser *parser = (BTParser *)load->loader_priv;
	if (!parser) return M4BadParam;
	return M4SM_LoaderRun_BT_intern(parser, NULL);
}


Chain *M4CTX_BTLoadFromString(LPSCENEGRAPH in_scene, char *node_str, void (*OnMessage)(void *cbk, char *msg, M4Err e), void *cbk)
{
	M4ContextLoader ctx;
	BTParser parser;
	memset(&ctx, 0, sizeof(M4ContextLoader));
	ctx.OnMessage = OnMessage;
	ctx.cbk = cbk;
	ctx.scene_graph = in_scene;
	memset(&parser, 0, sizeof(BTParser));
	parser.line_buffer = node_str;
	parser.line_size = strlen(node_str);
	parser.load = &ctx;
	parser.top_nodes = NewChain();
	parser.is_wrl = 1;
	M4SM_LoaderRun_BT_intern(&parser, NULL);
	return parser.top_nodes;
}

