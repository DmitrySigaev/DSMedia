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



#include <intern/m4_author_dev.h>


#define TTXT_DEFAULT_WIDTH	400
#define TTXT_DEFAULT_HEIGHT	60
#define TTXT_DEFAULT_FONT_SIZE	18
#ifndef M4_READ_ONLY

static void get_max_video_size(M4File *dest, u32 *width, u32 *height)
{
	u32 w, h, i;
	(*width) = (*height) = 0;
	for (i=0; i<M4_GetTrackCount(dest); i++) {
		switch (M4_GetMediaType(dest, i+1)) {
		case M4_BIFSMediaType:
		case M4_VisualMediaType:
			M4_GetVisualEntrySize(dest, i+1, 1, &w, &h);
			if (w > (*width)) (*width) = w;
			if (h > (*height)) (*height) = h;
			break;
		}
	}
}

M4Err MP4T_ImportSRT_Text(M4TrackImporter *import)
{
	FILE *srt_in;
	u32 track, timescale, i, idx;
	TextConfigDescriptor*cfg;
	M4Err e;
	StyleRecord rec;
	M4TXTSAMPLE samp;
	u32 sh, sm, ss, sms, eh, em, es, ems, start, end, txt_line, char_len, char_line, nb_samp, j, duration;
	Bool set_start_char, set_end_char, first_samp;
	u32 state, curLine, line, len, ID;
	char szLine[2048], szText[2048], *ptr;
	unsigned short uniLine[5000];

	srt_in = fopen(import->in_name, "rt");
	if (!srt_in) return import_message(import, M4URLNotFound, "Cannot open input SRT %s\n", import->in_name);

	cfg = NULL;

	if (import->esd) {
		if (!import->esd->slConfig) {
			import->esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
			import->esd->slConfig->predefined = 2;
			import->esd->slConfig->timestampResolution = 1000;
		}
		timescale = import->esd->slConfig->timestampResolution;
		if (!timescale) timescale = 1000;

		/*explicit text config*/
		if (import->esd->decoderConfig && import->esd->decoderConfig->decoderSpecificInfo->tag == TextConfig_Tag) {
			cfg = (TextConfigDescriptor *) import->esd->decoderConfig->decoderSpecificInfo;
			import->esd->decoderConfig->decoderSpecificInfo = NULL;
		}
		ID = import->esd->ESID;
	} else {
		timescale = 1000;
		ID = 0;
	}
	
	if (cfg && cfg->timescale) timescale = cfg->timescale;
	track = M4_NewTrack(import->dest, ID, M4_TimedTextMediaType, timescale);
	if (!track) {
		fclose(srt_in);
		return import_message(import, M4_GetLastError(import->dest), "Error creating text track");
	}
	M4_SetTrackEnabled(import->dest, track, 1);
	if (import->esd && !import->esd->ESID) import->esd->ESID = M4_GetTrackID(import->dest, track);

	/*setup track*/
	if (cfg) {
		char *firstFont = NULL;
		/*set track info*/
		M4_SetTrackVideoInfo(import->dest, track, cfg->text_width<<16, cfg->text_height<<16, 0, 0, cfg->layer);

		/*and set sample descriptions*/
		for (i=0; i<ChainGetCount(cfg->sample_descriptions); i++) {
			TextSampleDescriptor *sd= ChainGetEntry(cfg->sample_descriptions, i);
			if (!sd->font_count) {
				sd->fonts = malloc(sizeof(FontRecord));
				sd->font_count = 1;
				sd->fonts[0].fontID = 1;
				sd->fonts[0].fontName = strdup("Serif");
			}
			if (!sd->default_style.fontID) sd->default_style.fontID = sd->fonts[0].fontID;
			if (!sd->default_style.font_size) sd->default_style.font_size = 16;
			if (!sd->default_style.text_color) sd->default_style.text_color = 0xFF000000;
			/*store attribs*/
			if (!i) rec = sd->default_style;

			M4_NewTextDescription(import->dest, track, sd, NULL, NULL, &idx);
			if (!firstFont) firstFont = sd->fonts[0].fontName;
		}
		import_message(import, M4OK, "SRT->3GPP import - text track %d x %d, font %s (size %d)", cfg->text_width, cfg->text_height, firstFont, rec.font_size);

		OD_DeleteDescriptor((Descriptor **)&cfg);
	} else {
		u32 w, h;
		TextSampleDescriptor *sd;
		get_max_video_size(import->dest, &w, &h);
		if (!w) w = TTXT_DEFAULT_WIDTH;
		if (!h) h = TTXT_DEFAULT_HEIGHT;

		/*have to work with default - use max size (if only one video, this means the text region is the
		entire display, and with bottom alignment things should be fine...*/
		M4_SetTrackVideoInfo(import->dest, track, w<<16, h<<16, 0, 0, 0);
		sd = (TextSampleDescriptor*)OD_NewDescriptor(TextSampleDescriptor_Tag);
		sd->fonts = malloc(sizeof(FontRecord));
		sd->font_count = 1;
		sd->fonts[0].fontID = 1;
		sd->fonts[0].fontName = strdup("Serif");
		sd->back_color = 0x00000000;	/*transparent*/
		sd->default_style.fontID = 1;
		sd->default_style.font_size = TTXT_DEFAULT_FONT_SIZE;
		sd->default_style.text_color = 0xFFFFFFFF;	/*white*/
		sd->horiz_justif = 1; /*center of scene*/
		sd->vert_justif = -1;	/*bottom of scene*/
		/*store attribs*/
		rec = sd->default_style;
		M4_NewTextDescription(import->dest, track, sd, NULL, NULL, &idx);

		import_message(import, M4OK, "SRT->3GPP import - text track %d x %d, font %s (size %d)", w, h, sd->fonts[0].fontName, rec.font_size);
		OD_DeleteDescriptor((Descriptor **)&sd);
	}

	duration = (u32) (((Double) import->duration)*timescale/1000.0);

	e = M4OK;
	state = end = 0;
	curLine = 0;
	txt_line = 0;
	set_start_char = set_end_char = 0;
	char_len = 0;
	start = 0;
	nb_samp = 0;
	samp = M4_NewTextSample();

	first_samp = 1;
	while (!feof(srt_in)) {
		fgets(szLine, 2048, srt_in);

		while ( (szLine[strlen(szLine)-1]=='\n') || (szLine[strlen(szLine)-1]=='\r') || (szLine[strlen(szLine)-1]=='\t') )
			szLine[strlen(szLine)-1] = 0;

		if (!strlen(szLine)) {
			state = 0;
			rec.style_flags = 0;
			rec.startChar = rec.endChar = 0;

			if (txt_line) {
				M4Sample *s = M4_TxtToSample(samp);
				if (s) {
					Double ts = start;
					ts *= timescale;
					ts /= 1000;
					s->DTS = (u32) ts;
					s->IsRAP = 1;
					M4_AddSample(import->dest, track, 1, s);
					M4_DeleteSample(&s);
					nb_samp++;
				}
				M4_TxtReset(samp);
				s = M4_TxtToSample(samp);
				if (s) {
					Double ts = end;
					ts *= timescale;
					ts /= 1000;
					s->DTS = (u32) ts;
					s->IsRAP = 1;
					M4_AddSample(import->dest, track, 1, s);
					M4_DeleteSample(&s);
					nb_samp++;
				}
				txt_line = 0;
				char_len = 0;
				set_start_char = set_end_char = 0;
				rec.startChar = rec.endChar = 0;
				import_progress(import, nb_samp, nb_samp+1);
				if (duration && (end >= duration)) break;
			}
			continue;
		}

		switch (state) {
		case 0:
			if (sscanf(szLine, "%d", &line) != 1) {
				fprintf(stdout, "Bad SRT format\n");
				e = M4CorruptedData;
				goto exit;
			}
			if (line != curLine + 1) {
				fprintf(stdout, "Error importing SRT frame (previous %d, current %d)\n", curLine, line);
				e = M4CorruptedData;
				goto exit;
			}
			curLine = line;
			state = 1;
			break;
		case 1:
			if (sscanf(szLine, "%d:%d:%d,%d --> %d:%d:%d,%d", &sh, &sm, &ss, &sms, &eh, &em, &es, &ems) != 8) {
				fprintf(stdout, "Error importing SRT frame %d\n", curLine);
				e = M4CorruptedData;
				goto exit;
			}
			start = (3600*sh + 60*sm + ss)*1000 + sms;
			if (start<end) {
				import_message(import, M4OK, "WARNING: corrupted SRT frame %d - starts (at %d ms) before end of previous one (%d ms) - adjusting time stamps", curLine, start, end);
				start = end;
			}
			end = (3600*eh + 60*em + es)*1000 + ems;
			/*make stream start at 0 by inserting a fake AU*/
			if (first_samp && (start>0)) {
				M4Sample *s = M4_TxtToSample(samp);
				s->DTS = 0;
				M4_AddSample(import->dest, track, 1, s);
				M4_DeleteSample(&s);
				nb_samp++;
				import_progress(import, nb_samp, nb_samp+1);
			}
			rec.style_flags = 0;
			state = 2;			
			break;

		default:
			/*reset only when text is present*/
			first_samp = 0;
			ptr = szLine;
			i = j = 0;
			len = 0;
			while (ptr[i]) {
				if (!strnicmp(&ptr[i], "<i>", 3)) {
					rec.style_flags |= STF_Italic;
					i += 3;
					set_start_char = 1;
				}
				else if (!strnicmp(&ptr[i], "<u>", 3)) {
					rec.style_flags |= STF_Underlined;
					i += 3;
					set_start_char = 1;
				}
				else if (!strnicmp(&ptr[i], "<b>", 3)) {
					rec.style_flags |= STF_Bold;
					i += 3;
					set_start_char = 1;
				}
				else if (!strnicmp(&ptr[i], "- ? ", 4)) i += 4;
				else if (!strnicmp(&ptr[i], "? ", 2)) i += 2;
				else if (!strnicmp(&ptr[i], "</i>", 4) || !strnicmp(&ptr[i], "</b>", 4) || !strnicmp(&ptr[i], "</u>", 4)) {
					i+=4;
					set_end_char = 1;
				}
				else {
					if (ptr[i] & 0x80) {
						szText[len] = 0xc0 | ( (ptr[i] >> 6) & 0x3 );
						len++;
						ptr[i] &= 0xbf;
					}
					szText[len] = ptr[i];
					len++;
					i++;
				}
			}
			szText[len] = 0;
			ptr = (char *) szText;
			char_line = utf8_mbstowcs(uniLine, 5000, (const char **) &ptr);
			if (set_start_char) {
				rec.startChar = char_len;
				set_start_char = 0;
			}

			/*go to line*/
			if (txt_line) {
				M4_TxtAddText(samp, "\n", 1);
				char_len += 1;
			}

			if (set_end_char) {
				rec.endChar = char_line + char_len;
				set_end_char = 0;
				assert(rec.style_flags);
				M4_TxtAddStyle(samp, &rec);
				rec.endChar = rec.startChar = 0;
				rec.style_flags = 0;
			}
			char_len += char_line;

			M4_TxtAddText(samp, szText, len);
			txt_line ++;
			break;
		}
		if (duration && (start >= duration)) break;
	}
	M4_DeleteTextSample(samp);
	
	import_progress(import, nb_samp, nb_samp);

exit:
	if (e) M4_RemoveTrack(import->dest, track);
	fclose(srt_in);
	return e;
}

#endif


M4Err M4SM_ImportSRT_BIFS(M4SceneManager *ctx, ESDescriptor *src, MuxInfoDescriptor *mux)
{
	M4Err e;
	SFNode *text, *font;
	M4StreamContext *srt;
	FILE *srt_in;
	FieldInfo string, style;
	u32 sh, sm, ss, sms, eh, em, es, ems, start, end;
	M4AUContext *au;
	SGCommand *com;
	SFString *sfstr;
	CommandFieldInfo *inf;
	Bool italic, underlined, bold;
	u32 state, curLine, line, i, len;
	char szLine[2048], szText[2048], *ptr;
	M4StreamContext *sc = NULL;

	if (!ctx->scene_graph) {
		fprintf(stdout, "Error importing SRT: base scene not assigned\n");
		return M4BadParam;
	}
	for (i=0; i<ChainGetCount(ctx->streams); i++) {
		sc = ChainGetEntry(ctx->streams, i);
		if (sc->streamType==M4ST_SCENE) break;
		sc = NULL;
	}

	if (!sc) {
		fprintf(stdout, "Error importing SRT: Cannot locate base scene\n");
		return M4BadParam;
	}
	if (!mux->textNode) {
		fprintf(stdout, "Error importing SRT: Target text node unspecified\n");
		return M4BadParam;
	}
	text = SG_FindNodeByName(ctx->scene_graph, mux->textNode);
	if (!text) {
		fprintf(stdout, "Error importing SRT: Cannot find target text node %s\n", mux->textNode);
		return M4BadParam;
	}
	if (Node_GetFieldByName(text, "string", &string) != M4OK) {
		fprintf(stdout, "Error importing SRT: Target text node %s doesn't look like text\n", mux->textNode);
		return M4BadParam;
	}

	font = NULL;
	if (mux->fontNode) {
		font = SG_FindNodeByName(ctx->scene_graph, mux->fontNode);
		if (!font) {
			fprintf(stdout, "Error importing SRT: Cannot find target font node %s\n", mux->fontNode);
			return M4BadParam;
		}
		if (Node_GetFieldByName(font, "style", &style) != M4OK) {
			fprintf(stdout, "Error importing SRT: Target font node %s doesn't look like font\n", mux->fontNode);
			return M4BadParam;
		}
	}

	srt_in = fopen(mux->file_name, "rt");
	if (!srt_in) {
		fprintf(stdout, "Cannot open input SRT %s\n", mux->file_name);
		return M4URLNotFound;
	}

	srt = M4SM_NewStream(ctx, src->ESID, M4ST_SCENE, 1);
	if (!srt) return M4OutOfMem;

	if (!src->slConfig) src->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
	src->slConfig->timestampResolution = 1000;
	if (!src->decoderConfig) src->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
	src->decoderConfig->streamType = M4ST_SCENE;
	src->decoderConfig->objectTypeIndication = 1;

	e = M4OK;
	state = end = 0;
	curLine = 0;
	au = NULL;
	com = NULL;
	italic = underlined = bold = 0;
	inf = NULL;

	while (!feof(srt_in)) {
		fgets(szLine, 2048, srt_in);

		while ( (szLine[strlen(szLine)-1]=='\n') || (szLine[strlen(szLine)-1]=='\r') || (szLine[strlen(szLine)-1]=='\t') )
			szLine[strlen(szLine)-1] = 0;

		if (!strlen(szLine)) {
			state = 0;
			if (au) {
				/*if italic or underscore do it*/
				if (font && (italic || underlined || bold)) {
					com = SG_NewCommand(ctx->scene_graph, SG_FieldReplace);
					com->node = font;
					Node_Register(font, NULL);
					inf = SG_NewFieldCommand(com);
					inf->fieldIndex = style.fieldIndex;
					inf->fieldType = style.fieldType;
					sfstr = inf->field_ptr = VRML_NewFieldPointer(style.fieldType);
					if (bold && italic && underlined) sfstr->buffer = strdup("BOLDITALIC UNDERLINED");
					else if (italic && underlined) sfstr->buffer = strdup("ITALIC UNDERLINED");
					else if (bold && underlined) sfstr->buffer = strdup("BOLD UNDERLINED");
					else if (underlined) sfstr->buffer = strdup("UNDERLINED");
					else if (bold && italic) sfstr->buffer = strdup("BOLDITALIC");
					else if (bold) sfstr->buffer = strdup("BOLD");
					else sfstr->buffer = strdup("ITALIC");
					ChainAddEntry(au->commands, com);
				}

				au = M4SM_NewAU(srt, end, 0, 1);
				com = SG_NewCommand(ctx->scene_graph, SG_FieldReplace);
				com->node = text;
				Node_Register(text, NULL);
				inf = SG_NewFieldCommand(com);
				inf->fieldIndex = string.fieldIndex;
				inf->fieldType = string.fieldType;
				inf->field_ptr = VRML_NewFieldPointer(string.fieldType);
				ChainAddEntry(au->commands, com);
				/*reset font styles so that all AUs are true random access*/
				if (font) {
					com = SG_NewCommand(ctx->scene_graph, SG_FieldReplace);
					com->node = font;
					Node_Register(font, NULL);
					inf = SG_NewFieldCommand(com);
					inf->fieldIndex = style.fieldIndex;
					inf->fieldType = style.fieldType;
					inf->field_ptr = VRML_NewFieldPointer(style.fieldType);
					ChainAddEntry(au->commands, com);
				}
				au = NULL;
			}
			inf = NULL;
			continue;
		}

		switch (state) {
		case 0:
			if (sscanf(szLine, "%d", &line) != 1) {
				fprintf(stdout, "Bad SRT format\n");
				e = M4CorruptedData;
				goto exit;
			}
			if (line != curLine + 1) {
				fprintf(stdout, "Error importing SRT frame (previous %d, current %d)\n", curLine, line);
				e = M4CorruptedData;
				goto exit;
			}
			curLine = line;
			state = 1;
			break;
		case 1:
			if (sscanf(szLine, "%d:%d:%d,%d --> %d:%d:%d,%d", &sh, &sm, &ss, &sms, &eh, &em, &es, &ems) != 8) {
				fprintf(stdout, "Error importing SRT frame %d\n", curLine);
				e = M4CorruptedData;
				goto exit;
			}
			start = (3600*sh + 60*sm + ss)*1000 + sms;
			if (start<end) {
				fprintf(stdout, "WARNING: corrupted SRT frame starts before end of previous one (SRT Frame %d) - adjusting time stamps\n", curLine);
				start = end;
			}
			end = (3600*eh + 60*em + es)*1000 + ems;
			/*make stream start at 0 by inserting a fake AU*/
			if ((curLine==1) && start>0) {
				au = M4SM_NewAU(srt, 0, 0, 1);
				com = SG_NewCommand(ctx->scene_graph, SG_FieldReplace);
				com->node = text;
				Node_Register(text, NULL);
				inf = SG_NewFieldCommand(com);
				inf->fieldIndex = string.fieldIndex;
				inf->fieldType = string.fieldType;
				inf->field_ptr = VRML_NewFieldPointer(string.fieldType);
				ChainAddEntry(au->commands, com);
			}

			au = M4SM_NewAU(srt, start, 0, 1);
			com = NULL;
			state = 2;			
			italic = underlined = bold = 0;
			break;

		default:
			ptr = szLine;
			/*FIXME - other styles posssibles ??*/
			while (1) {
				if (!strnicmp(ptr, "<i>", 3)) {
					italic = 1;
					ptr += 3;
				}
				else if (!strnicmp(ptr, "<u>", 3)) {
					underlined = 1;
					ptr += 3;
				}
				else if (!strnicmp(ptr, "<b>", 3)) {
					bold = 1;
					ptr += 3;
				}
				else
					break;
			}
			/*if style remove end markers*/
			while ((strlen(ptr)>4) && (ptr[strlen(ptr) - 4] == '<') && (ptr[strlen(ptr) - 1] == '>')) {
				ptr[strlen(ptr) - 4] = 0;
			}

			if (!com) {
				com = SG_NewCommand(ctx->scene_graph, SG_FieldReplace);
				com->node = text;
				Node_Register(text, NULL);
				inf = SG_NewFieldCommand(com);
				inf->fieldIndex = string.fieldIndex;
				inf->fieldType = string.fieldType;
				inf->field_ptr = VRML_NewFieldPointer(string.fieldType);
				ChainAddEntry(au->commands, com);
			}
			assert(inf);
			VRML_MF_Append(inf->field_ptr, FT_MFString, (void **) &sfstr);
			len = 0;
			for (i=0; i<strlen(ptr); i++) {
				if (ptr[i] & 0x80) {
					szText[len] = 0xc0 | ( (ptr[i] >> 6) & 0x3 );
					len++;
					ptr[i] &= 0xbf;
				}
				szText[len] = ptr[i];
				len++;
			}
			szText[len] = 0;
			sfstr->buffer = strdup(szText);
			break;
		}
	}

exit:
	if (e) M4SM_RemoveStream(ctx, srt);
	fclose(srt_in);
	return e;
}

#ifndef M4_READ_ONLY



#define CHECK_STR(__str)	\
	if (!__str) { \
		e = import_message(import, M4BadParam, "Invalid XML formatting (line %d)", parser.line);	\
		goto exit;	\
	}	\


u32 ttxt_get_color(M4TrackImporter *import, XMLParser *parser)
{
	u32 r, g, b, a, res;
	r = g = b = a = 0;
	if (sscanf(parser->value_buffer, "%x %x %x %x", &r, &g, &b, &a) != 4) {
		import_message(import, M4OK, "Warning (line %d): color badly formatted", parser->line);
	}
	res = (a&0xFF); res<<=8;
	res |= (r&0xFF); res<<=8;
	res |= (g&0xFF); res<<=8;
	res |= (b&0xFF);
	return res;
}

void ttxt_parse_text_box(M4TrackImporter *import, XMLParser *parser, BoxRecord *box)
{
	memset(box, 0, sizeof(BoxRecord));
	while (xml_has_attributes(parser)) {
		char *str = xml_get_attribute(parser);
		if (!stricmp(str, "top")) box->top = atoi(parser->value_buffer);
		else if (!stricmp(str, "bottom")) box->bottom = atoi(parser->value_buffer);
		else if (!stricmp(str, "left")) box->left = atoi(parser->value_buffer);
		else if (!stricmp(str, "right")) box->right = atoi(parser->value_buffer);
	}
	xml_skip_element(parser, "TextBox");
}

void ttxt_parse_text_style(M4TrackImporter *import, XMLParser *parser, StyleRecord *style)
{
	memset(style, 0, sizeof(StyleRecord));
	style->fontID = 1;
	style->font_size = TTXT_DEFAULT_FONT_SIZE;
	style->text_color = 0xFFFFFFFF;

	while (xml_has_attributes(parser)) {
		char *str = xml_get_attribute(parser);
		if (!stricmp(str, "fromChar")) style->startChar = atoi(parser->value_buffer);
		else if (!stricmp(str, "toChar")) style->endChar = atoi(parser->value_buffer) + 1;
		else if (!stricmp(str, "fontID")) style->fontID = atoi(parser->value_buffer);
		else if (!stricmp(str, "fontSize")) style->font_size = atoi(parser->value_buffer);
		else if (!stricmp(str, "color")) style->text_color = ttxt_get_color(import, parser);
		else if (!stricmp(str, "styles")) {
			if (strstr(parser->value_buffer, "Bold")) style->style_flags |= STF_Bold;
			else if (strstr(parser->value_buffer, "Italic")) style->style_flags |= STF_Italic;
			else if (strstr(parser->value_buffer, "Underlined")) style->style_flags |= STF_Underlined;
		}
	}
	xml_skip_element(parser, "Style");
}

char *ttxt_parse_string(M4TrackImporter *import, XMLParser *parser)
{
	char value[XML_LINE_SIZE];
	u32 i=0;
	u32 k=0;
	char *str = parser->value_buffer;

	strcpy(value, "");
	
	while (strchr(str, '\'')) {
		while (str[0] != '\'')  str += 1;
		str+=1;
		while (str[i] && (str[i] != '\'')) {
			/*handle UTF-8 - WARNING: if parser is in unicode string is already utf8 multibyte chars*/
			if (!parser->unicode_type && (str[i] & 0x80)) {
				value[k] = 0xc0 | ( (str[i] >> 6) & 0x3 );
				k++;
				str[i] &= 0xbf;
			}
			value[k] = str[i];
			i++;
			k++;
		}
		if (!str[i+1]) break;
		str = str + i + 1;
		i=0;
		value[k] = '\n';
		k++;
	} 
	value[k] = 0;
	if (!strlen(value)) return strdup("");
	return xml_translate_xml_string(value);
}


M4Err MP4T_ImportTimedText(M4TrackImporter *import)
{
	M4Err e;
	u32 track, ID, nb_samples, nb_descs;
	XMLParser parser;
	char *str;

	import_message(import, M4OK, "3GPP Timed Text import");

	e = xml_init_parser(&parser, import->in_name);
	if (e) return import_message(import, e, "Cannot open file %s", import->in_name);

	str = xml_get_element(&parser);
	if (!str) { e = M4IOErr; goto exit; }
	if (strcmp(str, "TextStream")) {
		e = import_message(import, M4BadParam, "Invalid Timed Text file - expecting %s got %s", "TextStream", str);
		goto exit;
	}
	xml_skip_attributes(&parser);

	/*setup track in 3GP format directly (no ES desc)*/
	ID = (import->esd) ? import->esd->ESID : 0;
	track = M4_NewTrack(import->dest, ID, M4_TimedTextMediaType, 1000);
	if (!track) {
		e = M4_GetLastError(import->dest);
		goto exit;
	}
	M4_SetTrackEnabled(import->dest, track, 1);
	/*some MPEG-4 setup*/
	if (import->esd) {
		if (!import->esd->ESID) import->esd->ESID = M4_GetTrackID(import->dest, track);
		if (!import->esd->decoderConfig) import->esd->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
		if (!import->esd->slConfig) import->esd->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
		import->esd->slConfig->timestampResolution = 1000;
		import->esd->decoderConfig->streamType = M4ST_TEXT;
		import->esd->decoderConfig->objectTypeIndication = 0x08;
		if (import->esd->OCRESID) M4_SetTrackReference(import->dest, track, M4_OCR_Ref, import->esd->OCRESID);
	}

	nb_descs = 0;
	nb_samples = 0;
	while (!xml_element_done(&parser, "TextStream") && !parser.done) {
		str = xml_get_element(&parser);
		CHECK_STR(str)

		if (!strcmp(str, "TextStreamHeader")) {
			s32 w, h, tx, ty, layer;
			w = TTXT_DEFAULT_WIDTH;
			h = TTXT_DEFAULT_HEIGHT;
			tx = ty = layer = 0;

			while (xml_has_attributes(&parser)) {
				str = xml_get_attribute(&parser);
				if (!strcmp(str, "width")) w = atoi(parser.value_buffer);
				else if (!strcmp(str, "height")) h = atoi(parser.value_buffer);
				else if (!strcmp(str, "layer")) layer = atoi(parser.value_buffer);
				else if (!strcmp(str, "translation_x")) tx = atoi(parser.value_buffer);
				else if (!strcmp(str, "translation_y")) ty = atoi(parser.value_buffer);
			}
			M4_SetTrackVideoInfo(import->dest, track, w<<16, h<<16, tx<<16, ty<<16, (s16) layer);

			while (!xml_element_done(&parser, "TextStreamHeader")) {
				str = xml_get_element(&parser);
				CHECK_STR(str)

				if (!strcmp(str, "TextSampleDescription")) {
					TextSampleDescriptor td;
					u32 idx;
					memset(&td, 0, sizeof(TextSampleDescriptor));
					td.tag = TextConfig_Tag;
					td.vert_justif = -1;
					td.default_style.fontID = 1;
					td.default_style.font_size = TTXT_DEFAULT_FONT_SIZE;

					while (xml_has_attributes(&parser)) {
						str = xml_get_attribute(&parser);
						if (!strcmp(str, "horizontalJustification")) {
							if (!stricmp(parser.value_buffer, "center")) td.horiz_justif = 1;
							else if (!stricmp(parser.value_buffer, "right")) td.horiz_justif = -1;
							else if (!stricmp(parser.value_buffer, "left")) td.horiz_justif = 0;
						}
						else if (!strcmp(str, "verticalJustification")) {
							if (!stricmp(parser.value_buffer, "center")) td.vert_justif = 1;
							else if (!stricmp(parser.value_buffer, "bottom")) td.vert_justif = -1;
							else if (!stricmp(parser.value_buffer, "top")) td.vert_justif = 0;
						}
						else if (!strcmp(str, "backColor")) td.back_color = ttxt_get_color(import, &parser);
						else if (!strcmp(str, "verticalText") && !stricmp(parser.value_buffer, "yes") ) td.displayFlags |= STF_VerticalText;
						else if (!strcmp(str, "fillTextRegion") && !stricmp(parser.value_buffer, "yes") ) td.displayFlags |= STF_FillRegion;
						else if (!strcmp(str, "continuousKaraoke") && !stricmp(parser.value_buffer, "yes") ) td.displayFlags |= STF_Karaoke;
						else if (!strcmp(str, "scroll")) {
							if (!stricmp(parser.value_buffer, "inout")) td.displayFlags |= STF_ScrollIn | STF_ScrollOut;
							else if (!stricmp(parser.value_buffer, "in")) td.displayFlags |= STF_ScrollIn;
							else if (!stricmp(parser.value_buffer, "out")) td.displayFlags |= STF_ScrollOut;
						}
						else if (!strcmp(str, "scrollMode")) {
							u32 scroll_mode = STF_ScrollCredits;
							td.displayFlags |= STF_ScrollDirection;
							if (!stricmp(parser.value_buffer, "Credits")) scroll_mode = STF_ScrollCredits;
							else if (!stricmp(parser.value_buffer, "Marquee")) scroll_mode = STF_ScrollMarquee;
							else if (!stricmp(parser.value_buffer, "Right")) scroll_mode = STF_ScrollRight;
							else if (!stricmp(parser.value_buffer, "Down")) scroll_mode = STF_ScrollDown;
							
							td.displayFlags |= ((scroll_mode<<7) & STF_ScrollDirection);
						}
					}
					while (!xml_element_done(&parser, "TextSampleDescription")) {
						str = xml_get_element(&parser);
						if (!strcmp(str, "TextBox")) ttxt_parse_text_box(import, &parser, &td.default_pos);
						else if (!strcmp(str, "Style")) ttxt_parse_text_style(import, &parser, &td.default_style);
						else if (!strcmp(str, "FontTable")) {
							xml_skip_attributes(&parser);
							while (!xml_element_done(&parser, "FontTable")) {
								str = xml_get_element(&parser);
								if (!strcmp(str, "FontTableEntry")) {
									if (!td.fonts) {
										td.fonts = malloc(sizeof(FontRecord));
										td.font_count = 1;
									} else {
										td.font_count += 1;
										td.fonts = realloc(td.fonts, sizeof(FontRecord)*td.font_count);
									}
									while (xml_has_attributes(&parser)) {
										str = xml_get_attribute(&parser);
										if (!stricmp(str, "fontID")) td.fonts[td.font_count-1].fontID = atoi(parser.value_buffer);
										else if (!stricmp(str, "fontName")) td.fonts[td.font_count-1].fontName = strdup(parser.value_buffer);
									}
									xml_skip_element(&parser, "FontTableEntry");
								}
								else xml_skip_element(&parser, str);
							}
							xml_skip_element(&parser, "FontTable");
						}
						else {
							xml_skip_element(&parser, str);
						}

					}
					if ((td.default_pos.bottom==td.default_pos.top) || (td.default_pos.right==td.default_pos.left)) {
						td.default_pos.top = td.default_pos.left = 0;
						td.default_pos.right = w;
						td.default_pos.bottom = h;
					}
					if (!td.fonts) {
						td.font_count = 1;
						td.fonts = malloc(sizeof(FontRecord));
						td.fonts[0].fontID = 1;
						td.fonts[0].fontName = strdup("Serif");
					}
					M4_NewTextDescription(import->dest, track, &td, NULL, NULL, &idx);
					free(td.fonts);
					nb_descs ++;
				}
			}
		} else if (!strcmp(str, "TextSample")) {
			M4Sample *s;
			M4TXTSAMPLE samp;
			u32 ts, descIndex;
			
			if (!nb_descs) {
				e = import_message(import, M4BadParam, "Invalid Timed Text file - text stream header not found or empty");
				goto exit;
			}
			samp = M4_NewTextSample();
			ts = 0;
			descIndex = 1;

			while (xml_has_attributes(&parser)) {
				str = xml_get_attribute(&parser);
				if (!strcmp(str, "sampleTime")) {
					u32 h, m, s, ms;
					if (sscanf(parser.value_buffer, "%d:%d:%d.%d", &h, &m, &s, &ms) == 4) {
						ts = (h*3600 + m*60 + s)*1000 + ms;
					} else {
						ts = (u32) (atof(parser.value_buffer) * 1000);
					}
				}
				else if (!strcmp(str, "sampleDescriptionIndex")) descIndex = atoi(parser.value_buffer);
				else if (!strcmp(str, "text")) {
					u32 len;
					char *str = ttxt_parse_string(import, &parser);
					len = strlen(str);
					M4_TxtAddText(samp, str, len);
					free(str);
				}
				else if (!strcmp(str, "scrollDelay")) M4_TxtSetScrollDelay(samp, (u32) (1000*atoi(parser.value_buffer)));
				else if (!strcmp(str, "highlightColor")) M4_TxtSetHighlightColorARGB(samp, ttxt_get_color(import, &parser));
				else if (!strcmp(str, "wrap") && !strcmp(parser.value_buffer, "Automatic")) M4_TxtSetWrap(samp, 0x01);
			}
			/*get all modifiers*/
			while (!xml_element_done(&parser, "TextSample")) {
				str = xml_get_element(&parser);
				if (!stricmp(str, "Style")) {
					StyleRecord r;
					ttxt_parse_text_style(import, &parser, &r);
					M4_TxtAddStyle(samp, &r);
				}
				else if (!stricmp(str, "TextBox")) {
					BoxRecord r;
					ttxt_parse_text_box(import, &parser, &r);
					M4_TxtSetBox(samp, r.top, r.left, r.bottom, r.right);
				}
				else if (!stricmp(str, "Highlight")) {
					u16 start, end;
					start = end = 0;
					while (xml_has_attributes(&parser)) {
						str = xml_get_attribute(&parser);
						if (!strcmp(str, "fromChar")) start = atoi(parser.value_buffer);
						else if (!strcmp(str, "toChar")) end = 1 + atoi(parser.value_buffer);
					}
					xml_skip_element(&parser, "Highlight");
					M4_TxtAddHighlight(samp, start, end);
				}
				else if (!stricmp(str, "Blinking")) {
					u16 start, end;
					start = end = 0;
					while (xml_has_attributes(&parser)) {
						str = xml_get_attribute(&parser);
						if (!strcmp(str, "fromChar")) start = atoi(parser.value_buffer);
						else if (!strcmp(str, "toChar")) end = 1 + atoi(parser.value_buffer);
					}
					xml_skip_element(&parser, "Blinking");
					M4_TxtAddBlink(samp, start, end);
				}
				else if (!stricmp(str, "HyperLink")) {
					u16 start, end;
					char *url, *url_tt;
					start = end = 0;
					url = url_tt = NULL;
					while (xml_has_attributes(&parser)) {
						str = xml_get_attribute(&parser);
						if (!strcmp(str, "fromChar")) start = atoi(parser.value_buffer);
						else if (!strcmp(str, "toChar")) end = 1 + atoi(parser.value_buffer);
						else if (!strcmp(str, "URL")) url = strdup(parser.value_buffer);
						else if (!strcmp(str, "URLToolTip")) url_tt = strdup(parser.value_buffer);
					}
					xml_skip_element(&parser, "Blinking");
					M4_TxtAddHypertext(samp, url, url_tt, start, end);
					if (url) free(url);
					if (url_tt) free(url_tt);
				}
				else if (!stricmp(str, "Karaoke")) {
					u32 startTime;
					startTime = 0;
					while (xml_has_attributes(&parser)) {
						str = xml_get_attribute(&parser);
						if (!strcmp(str, "startTime")) startTime = (u32) (1000*atof(parser.value_buffer));
					}
					M4_TxtAddKaraoke(samp, startTime);
					while (!xml_element_done(&parser, "Karaoke")) {
						str = xml_get_element(&parser);
						if (!strcmp(str, "KaraokeRange")) {
							u16 start, end;
							u32 endTime;
							start = end = 0;
							endTime = 0;
							while (xml_has_attributes(&parser)) {
								str = xml_get_attribute(&parser);
								if (!strcmp(str, "fromChar")) start = atoi(parser.value_buffer);
								else if (!strcmp(str, "toChar")) end = 1 + atoi(parser.value_buffer);
								else if (!strcmp(str, "endTime")) endTime = (u32) (1000*atof(parser.value_buffer));
							}
							xml_skip_element(&parser, "KaraokeRange");
							M4_TxtSetKaraokeSeg(samp, endTime, start, end);
						} else {
							xml_skip_element(&parser, str);
						}
					}
				}
			}

			/*in MP4 we must start at T=0, so add an empty sample*/
			if (ts && !nb_samples) {
				M4TXTSAMPLE firstsamp = M4_NewTextSample();
				s = M4_TxtToSample(firstsamp);
				s->DTS = 0;
				M4_AddSample(import->dest, track, 1, s);
				nb_samples++;
				M4_DeleteTextSample(firstsamp);
				M4_DeleteSample(&s);
			}

			s = M4_TxtToSample(samp);
			M4_DeleteTextSample(samp);
			s->DTS = ts;


			M4_AddSample(import->dest, track, descIndex, s);
			M4_DeleteSample(&s);
			nb_samples++;

			import_progress(import, nb_samples, nb_samples+1);
			if (import->duration && (ts>import->duration)) break;
		} else {
			import_message(import, M4OK, "Unknown element %s - skipping", str);
			xml_skip_element(&parser, str);
		}
	}
	import_progress(import, nb_samples, nb_samples);

exit:
	xml_reset_parser(&parser);
	return e;
}


#endif

