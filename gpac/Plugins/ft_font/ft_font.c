/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / FreeType font engine plugin
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

#include "ft_font.h"



static M4Err ft_init_font_engine(FontRaster *dr)
{
	const char *sOpt;
	FTBuilder *ftpriv = (FTBuilder *)dr->priv;

	sOpt = PMI_GetOpt(dr, "FontEngine", "FontDirectory");
	if (!sOpt) return M4BadParam;

	/*inits freetype*/
	if (FT_Init_FreeType(&ftpriv->library) ) return M4IOErr;

	/*remove the final delimiter*/
    ftpriv->font_dir = strdup(sOpt);
	while ( (ftpriv->font_dir[strlen(ftpriv->font_dir)-1] == '\n') || (ftpriv->font_dir[strlen(ftpriv->font_dir)-1] == '\r') )
		ftpriv->font_dir[strlen(ftpriv->font_dir)-1] = 0;

	/*store font path*/
	if (ftpriv->font_dir[strlen(ftpriv->font_dir)-1] != M4_PATH_SEPARATOR) {
		char ext[2], *temp;
		ext[0] = M4_PATH_SEPARATOR;
		ext[1] = 0;
		temp = malloc(sizeof(char) * (strlen(ftpriv->font_dir) + 2));
		strcpy(temp, ftpriv->font_dir);
		strcat(temp, ext);
		free(ftpriv->font_dir);
		ftpriv->font_dir = temp;
	}
	sOpt = PMI_GetOpt(dr, "FontEngine", "FontSerif");
	strcpy(ftpriv->font_serif, sOpt ? sOpt : "Times New Roman");
	sOpt = PMI_GetOpt(dr, "FontEngine", "FontSans");
	strcpy(ftpriv->font_sans, sOpt ? sOpt : "Arial");
	sOpt = PMI_GetOpt(dr, "FontEngine", "FontFixed");
	strcpy(ftpriv->font_fixed, sOpt ? sOpt : "Courier New");
	return M4OK;
}

static M4Err ft_shutdown_font_engine(FontRaster *dr)
{
	FTBuilder *ftpriv = (FTBuilder *)dr->priv;

	ftpriv->active_face = NULL;
	/*reset loaded fonts*/
	while (ChainGetCount(ftpriv->loaded_fonts)) {
		FT_Face face = ChainGetEntry(ftpriv->loaded_fonts, 0);
		ChainDeleteEntry(ftpriv->loaded_fonts, 0);
		FT_Done_Face(face);
	}

	/*exit FT*/
	if (ftpriv->library) FT_Done_FreeType(ftpriv->library);
	ftpriv->library = NULL;
	return M4OK;
}


static Bool ft_check_face(FT_Face font, const char *fontName, const char *styles)
{
	Bool ret;
	char *ft_name;
	char *ft_style;

	if (fontName && stricmp(font->family_name, fontName)) return 0;
	ft_style = strdup(font->style_name);
	strupr(ft_style);
	if (!styles) {
		ret = 1;
		if (strstr(ft_style, "BOLD") || strstr(ft_style, "ITALIC") ) ret = 0;
		free(ft_style);
		return ret;
	}
	ft_name = strdup(font->family_name);
	strupr(ft_name);
	if (strstr(styles, "BOLDITALIC") ) {
		if (!strstr(ft_name, "BOLD") && !strstr(ft_style, "BOLD") ) {
			free(ft_name);
			free(ft_style);
			return 0;
		}
		if (!strstr(ft_name, "ITALIC") && !strstr(ft_style, "ITALIC") ) {
			free(ft_name);
			free(ft_style);
			return 0;
		}
	}
	else if (strstr(styles, "BOLD")) {
		if (!strstr(ft_name, "BOLD") && !strstr(ft_style, "BOLD") ) {
			free(ft_name);
			free(ft_style);
			return 0;
		}
		if (strstr(ft_style, "ITALIC")) {
			free(ft_name);
			free(ft_style);
			return 0;
		}
	}
	else if (strstr(styles, "ITALIC")) {
		if (!strstr(ft_name, "ITALIC") && !strstr(ft_style, "ITALIC")) {
			free(ft_name);
			free(ft_style);
			return 0;
		}
		if (strstr(ft_style, "BOLD")) {
			free(ft_name);
			free(ft_style);
			return 0;
		}
	}
	/*looks good, let's use this one*/
	free(ft_name);
	free(ft_style);
	return 1;
}

static FT_Face ft_font_in_cache(FTBuilder *ft, const char *fontName, const char *styles)
{
	u32 i;
	FT_Face font;

	for (i=0; i<ChainGetCount(ft->loaded_fonts); i++) {
		font = ChainGetEntry(ft->loaded_fonts, i);
		if (ft_check_face(font, fontName, styles)) return font;
	}
	return NULL;
}

static Bool ft_enum_fonts(void *cbck, char *file_name, char *file_path)
{
	FT_Face face;
	u32 num_faces, i;
	FontRaster *dr = cbck;
	FTBuilder *ftpriv = dr->priv;

	
	/*only trueType fonts/collections are handled*/
	if (!strstr(file_name, ".ttf") 
		&& !strstr(file_name, ".TTF")
		&& !strstr(file_name, ".ttc")
		&& !strstr(file_name, ".TTC") )
	
		return 0;
	
	if (FT_New_Face(ftpriv->library, file_path, 0, & face )) return 0;
	if (!face) return 0;

	num_faces = face->num_faces;
	/*locate right font in collection if several*/
	for (i=0; i<num_faces; i++) {
		if (ft_check_face(face, ftpriv->tmp_font_name, ftpriv->tmp_font_style)) 
			break;
		
		FT_Done_Face(face);
		if (i+1==num_faces) return 0;

		/*load next font in collection*/
		if (FT_New_Face(ftpriv->library, file_path, i+1, & face )) return 0;
		if (!face) return 0;
	}

	/*reject font if not scalablebitmap glyphs*/
	if (! (face->face_flags & FT_FACE_FLAG_SCALABLE)) {
		FT_Done_Face(face);
		return 0;
	}
	/*OK store in cache*/
	ChainAddEntry(ftpriv->loaded_fonts, face);
	ftpriv->active_face = face;

	/*and store entry in cfg file*/
	if (ftpriv->register_font) {
		char szFont[M4_MAX_PATH];
		strcpy(szFont, face->family_name);
		if (ftpriv->tmp_font_style && strstr(ftpriv->tmp_font_style, "BOLD") && strstr(ftpriv->tmp_font_style, "ITALIC")) {
			strcat(szFont, " Bold Italic");
		} else if (ftpriv->tmp_font_style && strstr(ftpriv->tmp_font_style, "BOLD") ) {
			strcat(szFont, " Bold");
		} else if (ftpriv->tmp_font_style && strstr(ftpriv->tmp_font_style, "ITALIC") ) {
			strcat(szFont, " Italic");
		}
		PMI_SetOpt(dr, "FontEngine", szFont, file_name);
	}
	return 1;
}

static M4Err ft_set_font(FontRaster *dr, const char *fontName, const char *styles)
{
	char *file_name, fname[1024];
	M4Err e;
	FTBuilder *ftpriv = (FTBuilder *)dr->priv;

	ftpriv->active_face = NULL;
	ftpriv->strike_style = 0;
	if (styles && strstr(styles, "UNDERLINE")) ftpriv->strike_style = 1;
	else if (styles && strstr(styles, "STRIKE")) ftpriv->strike_style = 2;

	if (fontName ) {
		if (!strlen(fontName) || !stricmp(fontName, "SERIF")) fontName = ftpriv->font_serif;
		else if (!stricmp(fontName, "SANS")) fontName = ftpriv->font_sans;
		else if (!stricmp(fontName, "TYPEWRITER")) fontName = ftpriv->font_fixed;
	}

	if (styles && (!stricmp(styles, "PLAIN") || !stricmp(styles, "REGULAR"))) styles = NULL;

	/*first look in loaded fonts*/
	ftpriv->active_face = ft_font_in_cache(ftpriv, fontName, styles);
	if (ftpriv->active_face) {
		return M4OK;
	}

	ftpriv->tmp_font_name = fontName;
	ftpriv->tmp_font_style = styles;
	ftpriv->register_font = 0;

	/*check cfg file - freetype is slow at loading fonts so we keep the (font name + styles)=fontfile associations
	in the cfg file*/
	if (fontName) {
		char file_path[M4_MAX_PATH];
		strcpy(fname, fontName);
		if (styles && strstr(styles, "BOLD") && strstr(styles, "ITALIC")) {
			strcat(fname, " Bold Italic");
		}
		else if (styles && strstr(styles, "BOLD")) {
			strcat(fname, " Bold");
		}
		else if (styles && strstr(styles, "ITALIC")) {
			strcat(fname, " Italic");
		}
		file_name = PMI_GetOpt(dr, "FontEngine", fname);
		if (file_name) {
			strcpy(file_path, ftpriv->font_dir);
			strcat(file_path, file_name);
			if (ft_enum_fonts(dr, file_name, file_path)) return M4OK;
		}
	}

	/*not found, browse all fonts*/
	ftpriv->register_font = 1;
	DIR_Enum(ftpriv->font_dir, 0, ft_enum_fonts, dr);
	ftpriv->register_font = 0;

	if (ftpriv->active_face) return M4OK;

	/*try load the first font that has the desired styles*/
	if (fontName) {
		e = ft_set_font(dr, NULL, styles);
		if (e==M4OK) return e;
		/*otherwise try without styles*/
		if (styles) return ft_set_font(dr, fontName, NULL);
	}
	return M4NotSupported;
}

static M4Err ft_set_font_size(FontRaster *dr, Float pixel_size)
{
	FTBuilder *ftpriv = (FTBuilder *)dr->priv;
	if (!ftpriv->active_face || !pixel_size) return M4BadParam;

	ftpriv->pixel_size = pixel_size;

	return M4OK;
}

static M4Err ft_get_font_metrics(FontRaster *dr, Float *ascent, Float *descent, Float *lineSpacing)
{
	FTBuilder *ftpriv = (FTBuilder *)dr->priv;
	if (!ftpriv->active_face || !ftpriv->pixel_size) return M4BadParam;

	*ascent = ftpriv->active_face->ascender * ftpriv->pixel_size;
	*ascent /= ftpriv->active_face->units_per_EM;

	*descent = -1 * ftpriv->active_face->descender * ftpriv->pixel_size;
	*descent /= ftpriv->active_face->units_per_EM;

	*lineSpacing = ftpriv->active_face->height * ftpriv->pixel_size;
	*lineSpacing /= ftpriv->active_face->units_per_EM;

	return M4OK;
}


/*small hack on charmap selection: by default use UNICODE, otherwise use first available charmap*/
static void ft_set_charmap(FT_Face face)
{
	if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0) {
		FT_CharMap *cur = face->charmaps;
		assert(cur);
		face->charmap = cur[0];
	}
}

static M4Err ft_get_text_size(FontRaster *dr, const unsigned short *string, Float *width, Float *height)
{
	u32 i, count, glyph_idx, w, h;
	FT_Glyph glyph;
	FT_BBox bbox;
	FTBuilder *ftpriv = (FTBuilder *)dr->priv;
	if (!ftpriv->active_face || !ftpriv->pixel_size) return M4BadParam;

	ft_set_charmap(ftpriv->active_face);

	count = utf8_wcslen(string);
	if (count == (size_t) (-1)) return M4BadParam;

	w = h = 0;
	for (i=0; i<count; i++) {
		glyph_idx = FT_Get_Char_Index(ftpriv->active_face, string[i]);
		/*missing glyph*/
		if (!glyph_idx) continue;

		/*work in design units*/
		FT_Load_Glyph(ftpriv->active_face, glyph_idx, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP);
		FT_Get_Glyph(ftpriv->active_face->glyph, &glyph);

		FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_unscaled, &bbox);		
		if (h < (u32) (bbox.yMax - bbox.yMin)) h = (bbox.yMax - bbox.yMin);

		if (!i) w = -1 * bbox.xMin;
		if (i+1<count) {
			w += ftpriv->active_face->glyph->metrics.horiAdvance;
		} else {
			w += bbox.xMax;
		}
		FT_Done_Glyph(glyph);
	}

	/*convert to font size*/
	*width = (Float) w;
	*width *= ftpriv->pixel_size;
	*width /= ftpriv->active_face->units_per_EM;

	*height = (Float) h;
	*height *= ftpriv->pixel_size;
	*height /= ftpriv->active_face->units_per_EM;

	return M4OK;
}




typedef struct
{
	Float font_scale;
	Float top;
	Float pos_x;
	FTBuilder *ftpriv;
	LPM4PATH path;

	Float x_scale, y_scale;
	Float last_x, last_y;
} ft_outliner;


#define GETX(_x) (ftol->pos_x + ftol->x_scale * ftol->font_scale * _x)
#define GETY(_y) (ftol->top + ftol->y_scale * ftol->font_scale * _y)


/*
	this is not explicit in FreeType doc, i assume each line/curve ending at last moveTo
	shall be closed
*/

int ft_move_to(FT_Vector *to, void *user)
{
	Float x, y;
	ft_outliner *ftol = (ft_outliner *)user;
	x = GETX(to->x);
	y = GETY(to->y);
	m4_path_add_move_to(ftol->path, x, y);
	ftol->last_x = x;
	ftol->last_y = y;
	return 0;
}
int ft_line_to(FT_Vector *to, void *user)
{
	Float x, y;
	ft_outliner *ftol = (ft_outliner *)user;
	x = GETX(to->x);
	y = GETY(to->y);

	if ( (ftol->last_x == x) && (ftol->last_y == y)) {
		m4_path_close(ftol->path);
	} else {
		m4_path_add_line_to(ftol->path, x, y);
	}
	return 0;
}
int ft_conic_to(FT_Vector * control, FT_Vector *to, void *user)
{
	Float x, y, cx, cy;
	ft_outliner *ftol = (ft_outliner *)user;
	cx = GETX(control->x);
	cy = GETY(control->y);
	x = GETX(to->x);
	y = GETY(to->y);
	m4_path_add_quadratic_to(ftol->path, cx, cy, x, y);

	if ( (ftol->last_x == x) && (ftol->last_y == y)) m4_path_close(ftol->path);
	return 0;
}
int ft_cubic_to(FT_Vector *control1, FT_Vector *control2, FT_Vector *to, void *user)
{
	Float x, y, c1x, c1y, c2x, c2y;
	ft_outliner *ftol = (ft_outliner *)user;
	c1x = GETX(control1->x);
	c1y = GETY(control1->y);
	c2x = GETX(control2->x);
	c2y = GETY(control2->y);
	x = GETX(to->x);
	y = GETY(to->y);
	m4_path_add_cubic_to(ftol->path, c1x, c1y, c1x, c1y, x, y);
	if ( (ftol->last_x == x) && (ftol->last_y == y)) m4_path_close(ftol->path);
	return 0;
}


static M4Err ft_add_text_to_path(FontRaster *dr, LPM4PATH path, Bool flipText,
					const unsigned short *string, Float left, Float top, Float x_scaling, Float y_scaling, 
					Float ascent, M4Rect *bounds)
{

	u32 i, count, glyph_idx;
	Float def_inc_x;
	s32 ymin, ymax;
	FT_BBox bbox;
	FT_OutlineGlyph outline;
	ft_outliner outl;

	FT_Outline_Funcs ft_outl_funcs;

	FTBuilder *ftpriv = (FTBuilder *)dr->priv;
	if (!ftpriv->active_face || !ftpriv->pixel_size) return M4BadParam;

	ft_set_charmap(ftpriv->active_face);

	/*setup outliner*/
	ft_outl_funcs.shift = 0;
	ft_outl_funcs.delta = 0;
	ft_outl_funcs.move_to = ft_move_to;
	ft_outl_funcs.line_to = ft_line_to;
	ft_outl_funcs.conic_to = ft_conic_to;
	ft_outl_funcs.cubic_to = ft_cubic_to;
	outl.font_scale = ftpriv->pixel_size;
	outl.font_scale /= ftpriv->active_face->units_per_EM;
	outl.path = path;
	outl.ftpriv = ftpriv;
	
	outl.x_scale = x_scaling;
	outl.y_scale = y_scaling;
	if (!flipText) outl.y_scale *= -1;
	
	bounds->x = outl.pos_x = left * x_scaling;
	if (flipText) 
		bounds->y = outl.top = (top - ascent) * y_scaling;
	else 
		bounds->y = outl.top = (top) * y_scaling;

	def_inc_x = ftpriv->active_face->max_advance_width * outl.font_scale * x_scaling;

	/*TODO: reverse layout (for arabic fonts) and glyph substitution / ligature once openType is in place in freetype*/

	bounds->height = 0;

	count = utf8_wcslen(string);
	if (count == (size_t) (-1)) return M4BadParam;
	ymin = ymax	= 0;

	for (i=0; i<count; i++) {
		glyph_idx = FT_Get_Char_Index(ftpriv->active_face, string[i]);
		/*missing glyph*/
		if (!glyph_idx) continue;

		/*work in design units*/
		FT_Load_Glyph(ftpriv->active_face, glyph_idx, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP);

		FT_Get_Glyph(ftpriv->active_face->glyph, (FT_Glyph*)&outline);
		/*oops not vectorial...*/
		if (outline->root.format != FT_GLYPH_FORMAT_OUTLINE) {
			outl.pos_x += def_inc_x;
			FT_Done_Glyph((FT_Glyph) outline);
			continue;
		}

		/*freeType is marvelous and gives back the right advance on space char !!!*/
	    FT_Outline_Decompose(&outline->outline, &ft_outl_funcs, &outl);

		FT_Glyph_Get_CBox((FT_Glyph) outline, ft_glyph_bbox_unscaled, &bbox);		
		if (ymax < bbox.yMax ) ymax = bbox.yMax ;
		if (ymin > bbox.yMin) ymin = bbox.yMin;

		/*update start of bounding box on first char*/
		if (!i) bounds->x += bbox.xMin * outl.font_scale * outl.x_scale;

		/*take care of last char (may be AFTER last horiz_advanced with certain glyphs)*/
		if ((i+1==count) && (bbox.xMax)) {
			outl.pos_x += bbox.xMax * outl.font_scale * outl.x_scale;
		} else {
			outl.pos_x += ftpriv->active_face->glyph->metrics.horiAdvance * outl.font_scale * outl.x_scale;
		}

		FT_Done_Glyph((FT_Glyph) outline);
	}


	bounds->width = outl.pos_x - bounds->x;
	bounds->height = (ymax - ymin) * outl.font_scale * outl.y_scale;
	bounds->y = ymax * outl.font_scale * outl.y_scale;
	if (!bounds->height && count) bounds->height = 0.001f;

	/*add underline/strikeout*/
	if (ftpriv->strike_style) {
		float sy;
		if (ftpriv->strike_style==1) {
			sy = top - ascent + ftpriv->active_face->underline_position * outl.font_scale;
		} else {
			sy = top - 3 * ascent / 4;
		}
		m4_path_add_rectangle(path, bounds->x + bounds->width / 2, sy, bounds->width, ftpriv->active_face->underline_thickness * outl.font_scale);
	}
	return M4OK;
}


FontRaster *FT_LoadPlugin()
{
	FontRaster *dr;
	FTBuilder *ftpriv;
	dr = malloc(sizeof(FontRaster));
	memset(dr, 0, sizeof(FontRaster));
	M4_REG_PLUG(dr, M4_FONT_RASTER_INTERFACE, "FreeType Font Engine", "gpac distribution", 0);

	ftpriv = malloc(sizeof(FTBuilder));
	memset(ftpriv, 0, sizeof(FTBuilder));

	ftpriv->loaded_fonts = NewChain();

	dr->priv = ftpriv;

	
	dr->init_font_engine = ft_init_font_engine;
	dr->shutdown_font_engine = ft_shutdown_font_engine;
	dr->set_font = ft_set_font;
	dr->set_font_size = ft_set_font_size;
	dr->get_font_metrics = ft_get_font_metrics;
	dr->get_text_size = ft_get_text_size;
	dr->add_text_to_path = ft_add_text_to_path;
	
	return dr;
}


void FT_Delete(BaseInterface *ifce)
{
	FontRaster *dr = (FontRaster *) ifce;
	FTBuilder *ftpriv = dr->priv;


	if (ftpriv->font_dir) free(ftpriv->font_dir);
	assert(!ChainGetCount(ftpriv->loaded_fonts) );

	DeleteChain(ftpriv->loaded_fonts);

	free(dr->priv);
	free(dr);
}

Bool QueryInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4_FONT_RASTER_INTERFACE) return 1;
	return 0;
}

void *LoadInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4_FONT_RASTER_INTERFACE) return FT_LoadPlugin();
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	BaseInterface *ptr = (BaseInterface *)ifce;
	switch (ptr->InterfaceType) {
	case M4_FONT_RASTER_INTERFACE:
		FT_Delete(ptr);
		break;
	}
}
