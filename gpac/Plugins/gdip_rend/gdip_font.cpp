/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / GDIplus rasterizer plugin
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

#include "gdip_priv.h"


#ifndef GDIP_MAX_STRING_SIZE
#define GDIP_MAX_STRING_SIZE	5000
#endif


M4Err gdip_init_font_engine(FontRaster *dr)
{
	const char *sOpt;
	FontPriv *ctx = (FontPriv *)dr->priv;

	sOpt = PMI_GetOpt(dr, "FontEngine", "FontSerif");
	strcpy(ctx->font_serif, sOpt ? sOpt : "Times New Roman");
	sOpt = PMI_GetOpt(dr, "FontEngine", "FontSans");
	strcpy(ctx->font_sans, sOpt ? sOpt : "Arial");
	sOpt = PMI_GetOpt(dr, "FontEngine", "FontFixed");
	strcpy(ctx->font_fixed, sOpt ? sOpt : "Courier New");

	return M4OK;
}
M4Err gdip_shutdown_font_engine(FontRaster *dr)
{
	FontPriv *ctx = (FontPriv *)dr->priv;

	if (ctx->font) GdipDeleteFontFamily(ctx->font);
	ctx->font = NULL;
	
	/*nothing to do*/
	return M4OK;
}


static void adjust_white_space(const unsigned short *string, float *width, float whiteSpaceWidth)
{
	u32 len , i=0;
	while (string[i] == (unsigned short) ' ') {
		*width += whiteSpaceWidth;
		i++;
	}
	if (whiteSpaceWidth<0) return;
	len = utf8_wcslen(string);
	if (i != len) {
		i = len - 1;
		while (string[i] == (unsigned short) ' ') {
			*width += whiteSpaceWidth;
			i--;
		}
	}
}

static M4Err gdip_get_font_metrics(FontRaster *dr, Float *ascent, Float *descent, Float *lineSpacing)
{
	UINT16 val, em;
	FontPriv *ctx = (FontPriv *)dr->priv;

	*ascent = *descent = *lineSpacing = 0.0;
	if (!ctx->font) return M4BadParam;


	GdipGetEmHeight(ctx->font, ctx->font_style, &em);
	GdipGetCellAscent(ctx->font, ctx->font_style, &val);
	*ascent = ctx->font_size * val;
	*ascent /= em;

	GdipGetCellDescent(ctx->font, ctx->font_style, &val);
	*descent = ctx->font_size * val;
	*descent /= em;

	GdipGetLineSpacing(ctx->font, ctx->font_style, &val);
	*lineSpacing = ctx->font_size * val;
	*lineSpacing /= em;
	return M4OK;
}

static M4Err gdip_get_text_size(FontRaster *dr, const unsigned short *string, Float *width, Float *height)
{
	GpPath *path_tmp;
	GpStringFormat *fmt;
	FontPriv *ctx = (FontPriv *)dr->priv;
	*width = *height = 0.0;
	if (!ctx->font) return M4BadParam;

	GdipCreateStringFormat(StringFormatFlagsNoWrap, LANG_NEUTRAL, &fmt);
	GdipCreatePath(FillModeAlternate, &path_tmp);
	RectF rc;
	rc.X = rc.Y = 0;
	rc.Width = rc.Height = 0;
	GdipAddPathString(path_tmp, string, -1, ctx->font, ctx->font_style, ctx->font_size, &rc, fmt);

	GdipGetPathWorldBounds(path_tmp, &rc, NULL, NULL);
	*width = rc.Width;
	*height = rc.Height;
	adjust_white_space(string, width, ctx->whitespace_width );

	GdipDeleteStringFormat(fmt);
	GdipDeletePath(path_tmp);
	
	return M4OK;
}


static M4Err gdip_add_text_to_path(FontRaster *dr, LPM4PATH path, Bool flipText,
					const unsigned short* string, Float left, Float top, Float x_scaling, Float y_scaling, 
					Float ascent, M4Rect *bounds)
{
	GpPath *path_tmp;
	GpMatrix *mat;
	GpStringFormat *fmt;
	Float real_start;
	unsigned short str[4];
	FontPriv *ctx = (FontPriv *)dr->priv;

	if (!ctx->font) return M4BadParam;

	
	RectF rc;
	rc.X = rc.Y = 0;
	rc.Width = rc.Height = 0;

	/*find first non-space char and estimate its glyph pos since GDIplus doesn't give this info*/
	s32 len = utf8_wcslen(string);
	s32 i=0;
	for (; i<len; i++) {
		if (string[i] != (unsigned short) ' ') break;
	}
	if (i>=len) return M4OK;

	GdipCreateStringFormat(StringFormatFlagsNoWrap | StringFormatFlagsNoFitBlackBox | StringFormatFlagsMeasureTrailingSpaces, LANG_NEUTRAL, &fmt);
	GdipSetStringFormatAlign(fmt, StringAlignmentNear);
	GdipCreatePath(FillModeAlternate, &path_tmp);

	/*to compute first glyph alignment (say 'x', we figure out its bounding full box by using the '_' char as wrapper (eg, "_x_")
	then the bounding box starting from xMin of the glyph ('x_'). The difference between both will give us a good approx 
	of the glyph alignment*/
	str[0] = (unsigned short) '_';
	str[1] = string[i];
	str[2] = (unsigned short) '_';
	str[3] = (unsigned short) 0;
	GdipAddPathString(path_tmp, str, -1, ctx->font, ctx->font_style, ctx->font_size, &rc, fmt);
	GdipGetPathWorldBounds(path_tmp, &rc, NULL, NULL);
	Float w1 = rc.Width - 2 * ctx->underscore_width;
	
	GdipResetPath(path_tmp);

	str[0] = string[i];
	str[1] = (unsigned short) '_';
	str[2] = (unsigned short) 0;
	rc.X = rc.Y = 0;
	rc.Width = rc.Height = 0;
	GdipAddPathString(path_tmp, str, -1, ctx->font, ctx->font_style, ctx->font_size, &rc, fmt);
	GdipGetPathWorldBounds(path_tmp, &rc, NULL, NULL);
	real_start = w1 - (rc.Width - ctx->underscore_width);

	GdipResetPath(path_tmp);

	rc.X = rc.Y = 0;
	rc.Width = rc.Height = 0;
	GdipAddPathString(path_tmp, string, -1, ctx->font, ctx->font_style, ctx->font_size, &rc, fmt);
	GdipGetPathWorldBounds(path_tmp, &rc, NULL, NULL);


	GdipCreateMatrix(&mat);
	Float off_left = rc.X;
	Float off_left_real = rc.X;

	/*adjust all white space at begin*/
	adjust_white_space(string, &off_left, -1*ctx->whitespace_width);

	if (flipText) {
		/*first translate in local system*/
		GdipTranslateMatrix(mat, left - off_left + real_start, -top, MatrixOrderAppend);
		/*then scale as specified*/
		GdipScaleMatrix(mat, x_scaling, -y_scaling, MatrixOrderAppend);
	} else {
		/*first translate in local system*/
		GdipTranslateMatrix(mat, left - off_left + real_start, top-ascent, MatrixOrderAppend);
		/*then scale as specified*/
		GdipScaleMatrix(mat, x_scaling, y_scaling, MatrixOrderAppend);
	}
	GdipTransformPath(path_tmp, mat);

	/*start enum*/
	s32 count;
	GdipGetPointCount(path_tmp, &count);
	GpPointF *pts = new GpPointF[count];
	BYTE *types = new BYTE[count];
	GdipGetPathTypes(path_tmp, types, count);
	GdipGetPathPoints(path_tmp, pts, count);

	for (i=0; i<count; ) {
		BOOL closed = 0;
		s32 sub_type;
		
		sub_type = types[i] & PathPointTypePathTypeMask;

		if (sub_type == PathPointTypeStart) {
			m4_path_add_move_to(path, pts[i].X, pts[i].Y);
			i++;
		}
		else if (sub_type == PathPointTypeLine) {
			m4_path_add_line_to(path, pts[i].X, pts[i].Y);
		
			if (types[i] & PathPointTypeCloseSubpath) m4_path_close(path);

			i++;
		}
		else if (sub_type == PathPointTypeBezier) {
			assert(i+2<=count);
			m4_path_add_cubic_to(path, pts[i].X, pts[i].Y, pts[i+1].X, pts[i+1].Y, pts[i+2].X, pts[i+2].Y);

			if (types[i+2] & PathPointTypeCloseSubpath) m4_path_close(path);

			i += 3;
		} else {
			assert(0);
			break;
		}
	}
	
	delete [] pts;
	delete [] types;
	
	GdipResetPath(path_tmp);
	adjust_white_space(string, &rc.Width, ctx->whitespace_width);
	rc.X = off_left_real;
	rc.X = off_left;
	/*special case where string is just space*/
	if (!rc.Height) rc.Height = 1;

	GdipAddPathRectangles(path_tmp, &rc, 1);
	GdipGetPathWorldBounds(path_tmp, &rc, mat, NULL);
	bounds->x = rc.X;
	bounds->y = rc.Y;
	bounds->width = rc.Width;
	bounds->height = rc.Height;

	GdipDeleteStringFormat(fmt);
	GdipDeletePath(path_tmp);
	GdipDeleteMatrix(mat);
	return M4OK;
}

static M4Err gdip_set_font(FontRaster *dr, const char *fontName, const char *styles)
{
	WCHAR wcFontName[GDIP_MAX_STRING_SIZE];
	FontPriv *ctx = (FontPriv *)dr->priv;

	if (ctx->font) GdipDeleteFontFamily(ctx->font);
	ctx->font = NULL;

	if (fontName && strlen(fontName) >= GDIP_MAX_STRING_SIZE) fontName = NULL;

	if (!fontName || !strlen(fontName) ) fontName = ctx->font_serif;
	else if (!strcmp(fontName, "SANS")) fontName = ctx->font_sans;
	else if (!strcmp(fontName, "SERIF")) fontName = ctx->font_serif;
	else if (!strcmp(fontName, "TYPEWRITER")) fontName = ctx->font_fixed;

	MultiByteToWideChar(CP_ACP, 0, fontName, strlen(fontName)+1, 
						wcFontName, sizeof(wcFontName)/sizeof(wcFontName[0]) );


	GdipCreateFontFamilyFromName(wcFontName, NULL, &ctx->font);
	if (!ctx->font) return M4NotSupported;

	//setup styles
	ctx->font_style = 0;
	if (styles) {
		if (strstr(styles, "BOLDITALIC")) ctx->font_style |= FontStyleBoldItalic;
		else if (strstr(styles, "BOLD")) ctx->font_style |= FontStyleBold;
		else if (strstr(styles, "ITALIC")) ctx->font_style |= FontStyleItalic;
		
		if (strstr(styles, "UNDERLINED")) {
			ctx->font_style |= FontStyleUnderline;
		}
		if (strstr(styles, "STRIKEOUT")) {
			ctx->font_style |= FontStyleStrikeout;
		}
	}	

	return M4OK;
}


static M4Err gdip_set_font_size(FontRaster *dr, Float pixel_size)
{
	unsigned short test_str[4];
	FontPriv *ctx = (FontPriv *)dr->priv;
	if (!ctx->font) return M4BadParam;

	ctx->font_size = pixel_size;

	/*GDI+ won't return begin/end whitespace info through GetBoundingRect, so compute a default value for space...*/
	ctx->whitespace_width = 0;
	Float w, h, w2;
	test_str[0] = (unsigned short) '_';
	test_str[1] = (unsigned short) '\0';
	gdip_get_text_size(dr, test_str, &w, &h);
	ctx->underscore_width = w;

	test_str[0] = (unsigned short) '_';
	test_str[1] = (unsigned short) ' ';
	test_str[2] = (unsigned short) '_';
	test_str[3] = (unsigned short) '\0';
	gdip_get_text_size(dr, test_str, &w2, &h);
	ctx->whitespace_width = w2 - 2*w;
	return M4OK;
}


FontRaster *gdip_new_font_driver()
{
	GdiplusStartupInput startupInput;
	FontRaster *dr;
	FontPriv *ctx;

	GP_SAFEALLOC(ctx, FontPriv);
	GP_SAFEALLOC(dr, FontRaster);
	GdiplusStartup(&ctx->gdiToken, &startupInput, NULL);

	M4_REG_PLUG(dr, M4_FONT_RASTER_INTERFACE, "GDIplus Font Engine", "gpac distribution", 0)
	dr->add_text_to_path = gdip_add_text_to_path;
	dr->get_font_metrics = gdip_get_font_metrics;
	dr->get_text_size = gdip_get_text_size;
	dr->init_font_engine = gdip_init_font_engine;
	dr->set_font = gdip_set_font;
	dr->set_font_size = gdip_set_font_size;
	dr->shutdown_font_engine = gdip_shutdown_font_engine;
	dr->priv = ctx;
	return dr;
}

void gdip_delete_font_driver(FontRaster *dr)
{
	FontPriv *ctx = (FontPriv *)dr->priv;
	GdiplusShutdown(ctx->gdiToken);

	if (ctx->font) GdipDeleteFontFamily(ctx->font);
	ctx->font = NULL;

	free(dr->priv);
	free(dr);
}

