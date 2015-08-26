/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Cyril Concolato 2004
 *					All rights reserved
 *
 *  This file is part of GPAC / SVG Scene Graph sub-project
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
	DO NOT MOFIFY - File generated on GMT Fri Nov 12 16:25:29 2004

	BY SVGGen for GPAC Version 0.2.3-DEV
*/

#include <gpac/m4_svg_nodes.h>

#ifndef M4_DISABLE_SVG

#include <gpac/intern/m4_scenegraph_dev.h>

static void SVG_svg_Del(SFNode *node)
{
	SVGsvgElement *p = (SVGsvgElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_svg_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->requiredFeatures;
			return M4OK;
		case 5:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->requiredExtensions;
			return M4OK;
		case 6:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->requiredFormats;
			return M4OK;
		case 7:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->systemLanguage;
			return M4OK;
		case 8:
			info->name = "overflow";
			info->fieldType = SVG_SVG_overflow_values_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->overflow;
			return M4OK;
		case 9:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->fill;
			return M4OK;
		case 10:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->fill_rule;
			return M4OK;
		case 11:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->stroke;
			return M4OK;
		case 12:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->stroke_dasharray;
			return M4OK;
		case 13:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->stroke_dashoffset;
			return M4OK;
		case 14:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->stroke_linecap;
			return M4OK;
		case 15:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->stroke_linejoin;
			return M4OK;
		case 16:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->stroke_miterlimit;
			return M4OK;
		case 17:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->stroke_width;
			return M4OK;
		case 18:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->vector_effect;
			return M4OK;
		case 19:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->color;
			return M4OK;
		case 20:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->color_rendering;
			return M4OK;
		case 21:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->fill_opacity;
			return M4OK;
		case 22:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->stroke_opacity;
			return M4OK;
		case 23:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->display;
			return M4OK;
		case 24:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->visibility;
			return M4OK;
		case 25:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->image_rendering;
			return M4OK;
		case 26:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->pointer_events;
			return M4OK;
		case 27:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->shape_rendering;
			return M4OK;
		case 28:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->text_rendering;
			return M4OK;
		case 29:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->focusable;
			return M4OK;
		case 30:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->nav_index;
			return M4OK;
		case 31:
			info->name = "background-fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->background_fill;
			return M4OK;
		case 32:
			info->name = "background-fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->background_fill_opacity;
			return M4OK;
		case 33:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->externalResourcesRequired;
			return M4OK;
		case 34:
			info->name = "x";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->x;
			return M4OK;
		case 35:
			info->name = "y";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->y;
			return M4OK;
		case 36:
			info->name = "width";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->width;
			return M4OK;
		case 37:
			info->name = "height";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->height;
			return M4OK;
		case 38:
			info->name = "viewBox";
			info->fieldType = SVG_ViewBoxSpec_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->viewBox;
			return M4OK;
		case 39:
			info->name = "preserveAspectRatio";
			info->fieldType = SVG_PreserveAspectRatioSpec_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->preserveAspectRatio;
			return M4OK;
		case 40:
			info->name = "zoomAndPan";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->zoomAndPan;
			return M4OK;
		case 41:
			info->name = "version";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->version;
			return M4OK;
		case 42:
			info->name = "baseProfile";
			info->fieldType = SVG_Text_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->baseProfile;
			return M4OK;
		case 43:
			info->name = "contentScriptType";
			info->fieldType = SVG_ContentType_datatype;
			info->far_ptr = & ((SVGsvgElement *)node)->contentScriptType;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_svg()
{
	SVGsvgElement *p;
	SAFEALLOC(p, sizeof(SVGsvgElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_svg);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "svg";
	((SFNode *p)->sgprivate->node_del = SVG_svg_Del;
	((SFNode *p)->sgprivate->get_field = SVG_svg_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	return p;
}

static void SVG_g_Del(SFNode *node)
{
	SVGgElement *p = (SVGgElement *)node;
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_g_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGgElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGgElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGgElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGgElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGgElement *)node)->requiredFeatures;
			return M4OK;
		case 5:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGgElement *)node)->requiredExtensions;
			return M4OK;
		case 6:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGgElement *)node)->requiredFormats;
			return M4OK;
		case 7:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGgElement *)node)->systemLanguage;
			return M4OK;
		case 8:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGgElement *)node)->focusable;
			return M4OK;
		case 9:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGgElement *)node)->nav_index;
			return M4OK;
		case 10:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGgElement *)node)->fill;
			return M4OK;
		case 11:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGgElement *)node)->fill_rule;
			return M4OK;
		case 12:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGgElement *)node)->stroke;
			return M4OK;
		case 13:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGgElement *)node)->stroke_dasharray;
			return M4OK;
		case 14:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGgElement *)node)->stroke_dashoffset;
			return M4OK;
		case 15:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGgElement *)node)->stroke_linecap;
			return M4OK;
		case 16:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGgElement *)node)->stroke_linejoin;
			return M4OK;
		case 17:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGgElement *)node)->stroke_miterlimit;
			return M4OK;
		case 18:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGgElement *)node)->stroke_width;
			return M4OK;
		case 19:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGgElement *)node)->vector_effect;
			return M4OK;
		case 20:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGgElement *)node)->color;
			return M4OK;
		case 21:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGgElement *)node)->color_rendering;
			return M4OK;
		case 22:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGgElement *)node)->fill_opacity;
			return M4OK;
		case 23:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGgElement *)node)->stroke_opacity;
			return M4OK;
		case 24:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGgElement *)node)->display;
			return M4OK;
		case 25:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGgElement *)node)->visibility;
			return M4OK;
		case 26:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGgElement *)node)->image_rendering;
			return M4OK;
		case 27:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGgElement *)node)->pointer_events;
			return M4OK;
		case 28:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGgElement *)node)->shape_rendering;
			return M4OK;
		case 29:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGgElement *)node)->text_rendering;
			return M4OK;
		case 30:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGgElement *)node)->externalResourcesRequired;
			return M4OK;
		case 31:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGgElement *)node)->transform;
			return M4OK;
		case 32:
			info->name = "transform-host";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGgElement *)node)->transform_host;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_g()
{
	SVGgElement *p;
	SAFEALLOC(p, sizeof(SVGgElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_g);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "g";
	((SFNode *p)->sgprivate->node_del = SVG_g_Del;
	((SFNode *p)->sgprivate->get_field = SVG_g_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_defs_Del(SFNode *node)
{
	SVGdefsElement *p = (SVGdefsElement *)node;
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_defs_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->requiredFeatures;
			return M4OK;
		case 5:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->requiredExtensions;
			return M4OK;
		case 6:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->requiredFormats;
			return M4OK;
		case 7:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->systemLanguage;
			return M4OK;
		case 8:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->fill;
			return M4OK;
		case 9:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->fill_rule;
			return M4OK;
		case 10:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->stroke;
			return M4OK;
		case 11:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->stroke_dasharray;
			return M4OK;
		case 12:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->stroke_dashoffset;
			return M4OK;
		case 13:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->stroke_linecap;
			return M4OK;
		case 14:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->stroke_linejoin;
			return M4OK;
		case 15:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->stroke_miterlimit;
			return M4OK;
		case 16:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->stroke_width;
			return M4OK;
		case 17:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->vector_effect;
			return M4OK;
		case 18:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->color;
			return M4OK;
		case 19:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->color_rendering;
			return M4OK;
		case 20:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->fill_opacity;
			return M4OK;
		case 21:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->stroke_opacity;
			return M4OK;
		case 22:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->display;
			return M4OK;
		case 23:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->visibility;
			return M4OK;
		case 24:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->image_rendering;
			return M4OK;
		case 25:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->pointer_events;
			return M4OK;
		case 26:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->shape_rendering;
			return M4OK;
		case 27:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->text_rendering;
			return M4OK;
		case 28:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->externalResourcesRequired;
			return M4OK;
		case 29:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->transform;
			return M4OK;
		case 30:
			info->name = "transform-host";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGdefsElement *)node)->transform_host;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_defs()
{
	SVGdefsElement *p;
	SAFEALLOC(p, sizeof(SVGdefsElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_defs);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "defs";
	((SFNode *p)->sgprivate->node_del = SVG_defs_Del;
	((SFNode *p)->sgprivate->get_field = SVG_defs_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_desc_Del(SFNode *node)
{
	SVGdescElement *p = (SVGdescElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_desc_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGdescElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGdescElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGdescElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGdescElement *)node)->xml_space;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_desc()
{
	SVGdescElement *p;
	SAFEALLOC(p, sizeof(SVGdescElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_desc);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "desc";
	((SFNode *p)->sgprivate->node_del = SVG_desc_Del;
	((SFNode *p)->sgprivate->get_field = SVG_desc_get_attribute;
#endif
	return p;
}

static void SVG_title_Del(SFNode *node)
{
	SVGtitleElement *p = (SVGtitleElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_title_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtitleElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGtitleElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGtitleElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtitleElement *)node)->xml_space;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_title()
{
	SVGtitleElement *p;
	SAFEALLOC(p, sizeof(SVGtitleElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_title);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "title";
	((SFNode *p)->sgprivate->node_del = SVG_title_Del;
	((SFNode *p)->sgprivate->get_field = SVG_title_get_attribute;
#endif
	return p;
}

static void SVG_metadata_Del(SFNode *node)
{
	SVGmetadataElement *p = (SVGmetadataElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_metadata_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmetadataElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGmetadataElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGmetadataElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmetadataElement *)node)->xml_space;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_metadata()
{
	SVGmetadataElement *p;
	SAFEALLOC(p, sizeof(SVGmetadataElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_metadata);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "metadata";
	((SFNode *p)->sgprivate->node_del = SVG_metadata_Del;
	((SFNode *p)->sgprivate->get_field = SVG_metadata_get_attribute;
#endif
	return p;
}

static void SVG_use_Del(SFNode *node)
{
	SVGuseElement *p = (SVGuseElement *)node;
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_use_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "x";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->x;
			return M4OK;
		case 1:
			info->name = "y";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->y;
			return M4OK;
		case 2:
			info->name = "width";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->width;
			return M4OK;
		case 3:
			info->name = "height";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->height;
			return M4OK;
		case 4:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->id;
			return M4OK;
		case 5:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->xml_base;
			return M4OK;
		case 6:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->xml_lang;
			return M4OK;
		case 7:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->xml_space;
			return M4OK;
		case 8:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->requiredFeatures;
			return M4OK;
		case 9:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->requiredExtensions;
			return M4OK;
		case 10:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->requiredFormats;
			return M4OK;
		case 11:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->systemLanguage;
			return M4OK;
		case 12:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->fill;
			return M4OK;
		case 13:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->fill_rule;
			return M4OK;
		case 14:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->stroke;
			return M4OK;
		case 15:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->stroke_dasharray;
			return M4OK;
		case 16:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->stroke_dashoffset;
			return M4OK;
		case 17:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->stroke_linecap;
			return M4OK;
		case 18:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->stroke_linejoin;
			return M4OK;
		case 19:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->stroke_miterlimit;
			return M4OK;
		case 20:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->stroke_width;
			return M4OK;
		case 21:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->vector_effect;
			return M4OK;
		case 22:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->color;
			return M4OK;
		case 23:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->color_rendering;
			return M4OK;
		case 24:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->fill_opacity;
			return M4OK;
		case 25:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->stroke_opacity;
			return M4OK;
		case 26:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->display;
			return M4OK;
		case 27:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->visibility;
			return M4OK;
		case 28:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->image_rendering;
			return M4OK;
		case 29:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->pointer_events;
			return M4OK;
		case 30:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->shape_rendering;
			return M4OK;
		case 31:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->text_rendering;
			return M4OK;
		case 32:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->focusable;
			return M4OK;
		case 33:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->nav_index;
			return M4OK;
		case 34:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->xlink_type;
			return M4OK;
		case 35:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->xlink_role;
			return M4OK;
		case 36:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->xlink_arcrole;
			return M4OK;
		case 37:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->xlink_title;
			return M4OK;
		case 38:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->xlink_href;
			return M4OK;
		case 39:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->xlink_show;
			return M4OK;
		case 40:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->xlink_actuate;
			return M4OK;
		case 41:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->externalResourcesRequired;
			return M4OK;
		case 42:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->transform;
			return M4OK;
		case 43:
			info->name = "transform-host";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGuseElement *)node)->transform_host;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_use()
{
	SVGuseElement *p;
	SAFEALLOC(p, sizeof(SVGuseElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_use);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "use";
	((SFNode *p)->sgprivate->node_del = SVG_use_Del;
	((SFNode *p)->sgprivate->get_field = SVG_use_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_switch_Del(SFNode *node)
{
	SVGswitchElement *p = (SVGswitchElement *)node;
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_switch_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->requiredFeatures;
			return M4OK;
		case 5:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->requiredExtensions;
			return M4OK;
		case 6:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->requiredFormats;
			return M4OK;
		case 7:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->systemLanguage;
			return M4OK;
		case 8:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->fill;
			return M4OK;
		case 9:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->fill_rule;
			return M4OK;
		case 10:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->stroke;
			return M4OK;
		case 11:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->stroke_dasharray;
			return M4OK;
		case 12:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->stroke_dashoffset;
			return M4OK;
		case 13:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->stroke_linecap;
			return M4OK;
		case 14:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->stroke_linejoin;
			return M4OK;
		case 15:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->stroke_miterlimit;
			return M4OK;
		case 16:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->stroke_width;
			return M4OK;
		case 17:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->vector_effect;
			return M4OK;
		case 18:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->color;
			return M4OK;
		case 19:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->color_rendering;
			return M4OK;
		case 20:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->fill_opacity;
			return M4OK;
		case 21:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->stroke_opacity;
			return M4OK;
		case 22:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->display;
			return M4OK;
		case 23:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->visibility;
			return M4OK;
		case 24:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->image_rendering;
			return M4OK;
		case 25:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->pointer_events;
			return M4OK;
		case 26:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->shape_rendering;
			return M4OK;
		case 27:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->text_rendering;
			return M4OK;
		case 28:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->externalResourcesRequired;
			return M4OK;
		case 29:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->transform;
			return M4OK;
		case 30:
			info->name = "transform-host";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGswitchElement *)node)->transform_host;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_switch()
{
	SVGswitchElement *p;
	SAFEALLOC(p, sizeof(SVGswitchElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_switch);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "switch";
	((SFNode *p)->sgprivate->node_del = SVG_switch_Del;
	((SFNode *p)->sgprivate->get_field = SVG_switch_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_image_Del(SFNode *node)
{
	SVGimageElement *p = (SVGimageElement *)node;
	DeleteChain(p->begin);
	DeleteChain(p->end);
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_image_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "begin";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->begin;
			return M4OK;
		case 1:
			info->name = "end";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->end;
			return M4OK;
		case 2:
			info->name = "x";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->x;
			return M4OK;
		case 3:
			info->name = "y";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->y;
			return M4OK;
		case 4:
			info->name = "width";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->width;
			return M4OK;
		case 5:
			info->name = "height";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->height;
			return M4OK;
		case 6:
			info->name = "type";
			info->fieldType = SVG_ContentType_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->type;
			return M4OK;
		case 7:
			info->name = "preserveAspectRatio";
			info->fieldType = SVG_PreserveAspectRatioSpec_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->preserveAspectRatio;
			return M4OK;
		case 8:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->id;
			return M4OK;
		case 9:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->xml_base;
			return M4OK;
		case 10:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->xml_lang;
			return M4OK;
		case 11:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->xml_space;
			return M4OK;
		case 12:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->requiredFeatures;
			return M4OK;
		case 13:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->requiredExtensions;
			return M4OK;
		case 14:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->requiredFormats;
			return M4OK;
		case 15:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->systemLanguage;
			return M4OK;
		case 16:
			info->name = "overflow";
			info->fieldType = SVG_SVG_overflow_values_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->overflow;
			return M4OK;
		case 17:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->color;
			return M4OK;
		case 18:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->color_rendering;
			return M4OK;
		case 19:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->fill_opacity;
			return M4OK;
		case 20:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->stroke_opacity;
			return M4OK;
		case 21:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->display;
			return M4OK;
		case 22:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->visibility;
			return M4OK;
		case 23:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->image_rendering;
			return M4OK;
		case 24:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->pointer_events;
			return M4OK;
		case 25:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->shape_rendering;
			return M4OK;
		case 26:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->text_rendering;
			return M4OK;
		case 27:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->focusable;
			return M4OK;
		case 28:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->nav_index;
			return M4OK;
		case 29:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->xlink_type;
			return M4OK;
		case 30:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->xlink_role;
			return M4OK;
		case 31:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->xlink_arcrole;
			return M4OK;
		case 32:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->xlink_title;
			return M4OK;
		case 33:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->xlink_href;
			return M4OK;
		case 34:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->xlink_show;
			return M4OK;
		case 35:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->xlink_actuate;
			return M4OK;
		case 36:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->externalResourcesRequired;
			return M4OK;
		case 37:
			info->name = "background-fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->background_fill;
			return M4OK;
		case 38:
			info->name = "background-fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->background_fill_opacity;
			return M4OK;
		case 39:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGimageElement *)node)->transform;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_image()
{
	SVGimageElement *p;
	SAFEALLOC(p, sizeof(SVGimageElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_image);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "image";
	((SFNode *p)->sgprivate->node_del = SVG_image_Del;
	((SFNode *p)->sgprivate->get_field = SVG_image_get_attribute;
#endif
	p->begin = NewChain();
	p->end = NewChain();
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_path_Del(SFNode *node)
{
	SVGpathElement *p = (SVGpathElement *)node;
	DeleteChain(p->d.path_commands);
	DeleteChain(p->d.path_points);
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_path_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "d";
			info->fieldType = SVG_PathData_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->d;
			return M4OK;
		case 1:
			info->name = "pathLength";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->pathLength;
			return M4OK;
		case 2:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->id;
			return M4OK;
		case 3:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->xml_base;
			return M4OK;
		case 4:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->xml_lang;
			return M4OK;
		case 5:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->xml_space;
			return M4OK;
		case 6:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->requiredFeatures;
			return M4OK;
		case 7:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->requiredExtensions;
			return M4OK;
		case 8:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->requiredFormats;
			return M4OK;
		case 9:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->systemLanguage;
			return M4OK;
		case 10:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->fill;
			return M4OK;
		case 11:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->fill_rule;
			return M4OK;
		case 12:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->stroke;
			return M4OK;
		case 13:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->stroke_dasharray;
			return M4OK;
		case 14:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->stroke_dashoffset;
			return M4OK;
		case 15:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->stroke_linecap;
			return M4OK;
		case 16:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->stroke_linejoin;
			return M4OK;
		case 17:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->stroke_miterlimit;
			return M4OK;
		case 18:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->stroke_width;
			return M4OK;
		case 19:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->vector_effect;
			return M4OK;
		case 20:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->fill_opacity;
			return M4OK;
		case 21:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->stroke_opacity;
			return M4OK;
		case 22:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->color;
			return M4OK;
		case 23:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->color_rendering;
			return M4OK;
		case 24:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->display;
			return M4OK;
		case 25:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->visibility;
			return M4OK;
		case 26:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->image_rendering;
			return M4OK;
		case 27:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->pointer_events;
			return M4OK;
		case 28:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->shape_rendering;
			return M4OK;
		case 29:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->text_rendering;
			return M4OK;
		case 30:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->focusable;
			return M4OK;
		case 31:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->nav_index;
			return M4OK;
		case 32:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->externalResourcesRequired;
			return M4OK;
		case 33:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGpathElement *)node)->transform;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_path()
{
	SVGpathElement *p;
	SAFEALLOC(p, sizeof(SVGpathElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_path);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "path";
	((SFNode *p)->sgprivate->node_del = SVG_path_Del;
	((SFNode *p)->sgprivate->get_field = SVG_path_get_attribute;
#endif
	p->d.path_commands = NewChain();
	p->d.path_points = NewChain();
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_rect_Del(SFNode *node)
{
	SVGrectElement *p = (SVGrectElement *)node;
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_rect_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "x";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->x;
			return M4OK;
		case 1:
			info->name = "y";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->y;
			return M4OK;
		case 2:
			info->name = "width";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->width;
			return M4OK;
		case 3:
			info->name = "height";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->height;
			return M4OK;
		case 4:
			info->name = "rx";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->rx;
			return M4OK;
		case 5:
			info->name = "ry";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->ry;
			return M4OK;
		case 6:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->id;
			return M4OK;
		case 7:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->xml_base;
			return M4OK;
		case 8:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->xml_lang;
			return M4OK;
		case 9:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->xml_space;
			return M4OK;
		case 10:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->requiredFeatures;
			return M4OK;
		case 11:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->requiredExtensions;
			return M4OK;
		case 12:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->requiredFormats;
			return M4OK;
		case 13:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->systemLanguage;
			return M4OK;
		case 14:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->fill;
			return M4OK;
		case 15:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->fill_rule;
			return M4OK;
		case 16:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->stroke;
			return M4OK;
		case 17:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->stroke_dasharray;
			return M4OK;
		case 18:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->stroke_dashoffset;
			return M4OK;
		case 19:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->stroke_linecap;
			return M4OK;
		case 20:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->stroke_linejoin;
			return M4OK;
		case 21:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->stroke_miterlimit;
			return M4OK;
		case 22:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->stroke_width;
			return M4OK;
		case 23:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->vector_effect;
			return M4OK;
		case 24:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->fill_opacity;
			return M4OK;
		case 25:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->stroke_opacity;
			return M4OK;
		case 26:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->color;
			return M4OK;
		case 27:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->color_rendering;
			return M4OK;
		case 28:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->display;
			return M4OK;
		case 29:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->visibility;
			return M4OK;
		case 30:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->image_rendering;
			return M4OK;
		case 31:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->pointer_events;
			return M4OK;
		case 32:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->shape_rendering;
			return M4OK;
		case 33:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->text_rendering;
			return M4OK;
		case 34:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->focusable;
			return M4OK;
		case 35:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->nav_index;
			return M4OK;
		case 36:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->externalResourcesRequired;
			return M4OK;
		case 37:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGrectElement *)node)->transform;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_rect()
{
	SVGrectElement *p;
	SAFEALLOC(p, sizeof(SVGrectElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_rect);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "rect";
	((SFNode *p)->sgprivate->node_del = SVG_rect_Del;
	((SFNode *p)->sgprivate->get_field = SVG_rect_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_circle_Del(SFNode *node)
{
	SVGcircleElement *p = (SVGcircleElement *)node;
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_circle_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "cx";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->cx;
			return M4OK;
		case 1:
			info->name = "cy";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->cy;
			return M4OK;
		case 2:
			info->name = "r";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->r;
			return M4OK;
		case 3:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->id;
			return M4OK;
		case 4:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->xml_base;
			return M4OK;
		case 5:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->xml_lang;
			return M4OK;
		case 6:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->xml_space;
			return M4OK;
		case 7:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->requiredFeatures;
			return M4OK;
		case 8:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->requiredExtensions;
			return M4OK;
		case 9:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->requiredFormats;
			return M4OK;
		case 10:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->systemLanguage;
			return M4OK;
		case 11:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->focusable;
			return M4OK;
		case 12:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->nav_index;
			return M4OK;
		case 13:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->fill;
			return M4OK;
		case 14:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->fill_rule;
			return M4OK;
		case 15:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->stroke;
			return M4OK;
		case 16:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->stroke_dasharray;
			return M4OK;
		case 17:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->stroke_dashoffset;
			return M4OK;
		case 18:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->stroke_linecap;
			return M4OK;
		case 19:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->stroke_linejoin;
			return M4OK;
		case 20:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->stroke_miterlimit;
			return M4OK;
		case 21:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->stroke_width;
			return M4OK;
		case 22:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->vector_effect;
			return M4OK;
		case 23:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->fill_opacity;
			return M4OK;
		case 24:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->stroke_opacity;
			return M4OK;
		case 25:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->color;
			return M4OK;
		case 26:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->color_rendering;
			return M4OK;
		case 27:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->display;
			return M4OK;
		case 28:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->visibility;
			return M4OK;
		case 29:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->image_rendering;
			return M4OK;
		case 30:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->pointer_events;
			return M4OK;
		case 31:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->shape_rendering;
			return M4OK;
		case 32:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->text_rendering;
			return M4OK;
		case 33:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->externalResourcesRequired;
			return M4OK;
		case 34:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGcircleElement *)node)->transform;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_circle()
{
	SVGcircleElement *p;
	SAFEALLOC(p, sizeof(SVGcircleElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_circle);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "circle";
	((SFNode *p)->sgprivate->node_del = SVG_circle_Del;
	((SFNode *p)->sgprivate->get_field = SVG_circle_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_line_Del(SFNode *node)
{
	SVGlineElement *p = (SVGlineElement *)node;
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_line_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "x1";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->x1;
			return M4OK;
		case 1:
			info->name = "y1";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->y1;
			return M4OK;
		case 2:
			info->name = "x2";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->x2;
			return M4OK;
		case 3:
			info->name = "y2";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->y2;
			return M4OK;
		case 4:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->id;
			return M4OK;
		case 5:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->xml_base;
			return M4OK;
		case 6:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->xml_lang;
			return M4OK;
		case 7:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->xml_space;
			return M4OK;
		case 8:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->requiredFeatures;
			return M4OK;
		case 9:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->requiredExtensions;
			return M4OK;
		case 10:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->requiredFormats;
			return M4OK;
		case 11:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->systemLanguage;
			return M4OK;
		case 12:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->fill;
			return M4OK;
		case 13:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->fill_rule;
			return M4OK;
		case 14:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->stroke;
			return M4OK;
		case 15:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->stroke_dasharray;
			return M4OK;
		case 16:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->stroke_dashoffset;
			return M4OK;
		case 17:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->stroke_linecap;
			return M4OK;
		case 18:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->stroke_linejoin;
			return M4OK;
		case 19:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->stroke_miterlimit;
			return M4OK;
		case 20:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->stroke_width;
			return M4OK;
		case 21:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->vector_effect;
			return M4OK;
		case 22:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->color;
			return M4OK;
		case 23:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->color_rendering;
			return M4OK;
		case 24:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->fill_opacity;
			return M4OK;
		case 25:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->stroke_opacity;
			return M4OK;
		case 26:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->display;
			return M4OK;
		case 27:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->visibility;
			return M4OK;
		case 28:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->image_rendering;
			return M4OK;
		case 29:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->pointer_events;
			return M4OK;
		case 30:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->shape_rendering;
			return M4OK;
		case 31:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->text_rendering;
			return M4OK;
		case 32:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->focusable;
			return M4OK;
		case 33:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->nav_index;
			return M4OK;
		case 34:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->externalResourcesRequired;
			return M4OK;
		case 35:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGlineElement *)node)->transform;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_line()
{
	SVGlineElement *p;
	SAFEALLOC(p, sizeof(SVGlineElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_line);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "line";
	((SFNode *p)->sgprivate->node_del = SVG_line_Del;
	((SFNode *p)->sgprivate->get_field = SVG_line_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_ellipse_Del(SFNode *node)
{
	SVGellipseElement *p = (SVGellipseElement *)node;
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_ellipse_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "cx";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->cx;
			return M4OK;
		case 1:
			info->name = "cy";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->cy;
			return M4OK;
		case 2:
			info->name = "rx";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->rx;
			return M4OK;
		case 3:
			info->name = "ry";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->ry;
			return M4OK;
		case 4:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->id;
			return M4OK;
		case 5:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->xml_base;
			return M4OK;
		case 6:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->xml_lang;
			return M4OK;
		case 7:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->xml_space;
			return M4OK;
		case 8:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->requiredFeatures;
			return M4OK;
		case 9:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->requiredExtensions;
			return M4OK;
		case 10:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->requiredFormats;
			return M4OK;
		case 11:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->systemLanguage;
			return M4OK;
		case 12:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->fill;
			return M4OK;
		case 13:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->fill_rule;
			return M4OK;
		case 14:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->stroke;
			return M4OK;
		case 15:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->stroke_dasharray;
			return M4OK;
		case 16:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->stroke_dashoffset;
			return M4OK;
		case 17:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->stroke_linecap;
			return M4OK;
		case 18:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->stroke_linejoin;
			return M4OK;
		case 19:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->stroke_miterlimit;
			return M4OK;
		case 20:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->stroke_width;
			return M4OK;
		case 21:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->vector_effect;
			return M4OK;
		case 22:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->color;
			return M4OK;
		case 23:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->color_rendering;
			return M4OK;
		case 24:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->fill_opacity;
			return M4OK;
		case 25:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->stroke_opacity;
			return M4OK;
		case 26:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->display;
			return M4OK;
		case 27:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->visibility;
			return M4OK;
		case 28:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->image_rendering;
			return M4OK;
		case 29:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->pointer_events;
			return M4OK;
		case 30:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->shape_rendering;
			return M4OK;
		case 31:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->text_rendering;
			return M4OK;
		case 32:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->focusable;
			return M4OK;
		case 33:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->nav_index;
			return M4OK;
		case 34:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->externalResourcesRequired;
			return M4OK;
		case 35:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGellipseElement *)node)->transform;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_ellipse()
{
	SVGellipseElement *p;
	SAFEALLOC(p, sizeof(SVGellipseElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_ellipse);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "ellipse";
	((SFNode *p)->sgprivate->node_del = SVG_ellipse_Del;
	((SFNode *p)->sgprivate->get_field = SVG_ellipse_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_polyline_Del(SFNode *node)
{
	SVGpolylineElement *p = (SVGpolylineElement *)node;
	DeleteChain(p->points);
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_polyline_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "points";
			info->fieldType = SVG_Points_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->points;
			return M4OK;
		case 1:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->id;
			return M4OK;
		case 2:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->xml_base;
			return M4OK;
		case 3:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->xml_lang;
			return M4OK;
		case 4:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->xml_space;
			return M4OK;
		case 5:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->requiredFeatures;
			return M4OK;
		case 6:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->requiredExtensions;
			return M4OK;
		case 7:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->requiredFormats;
			return M4OK;
		case 8:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->systemLanguage;
			return M4OK;
		case 9:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->fill;
			return M4OK;
		case 10:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->fill_rule;
			return M4OK;
		case 11:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->stroke;
			return M4OK;
		case 12:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->stroke_dasharray;
			return M4OK;
		case 13:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->stroke_dashoffset;
			return M4OK;
		case 14:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->stroke_linecap;
			return M4OK;
		case 15:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->stroke_linejoin;
			return M4OK;
		case 16:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->stroke_miterlimit;
			return M4OK;
		case 17:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->stroke_width;
			return M4OK;
		case 18:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->vector_effect;
			return M4OK;
		case 19:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->color;
			return M4OK;
		case 20:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->color_rendering;
			return M4OK;
		case 21:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->fill_opacity;
			return M4OK;
		case 22:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->stroke_opacity;
			return M4OK;
		case 23:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->display;
			return M4OK;
		case 24:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->visibility;
			return M4OK;
		case 25:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->image_rendering;
			return M4OK;
		case 26:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->pointer_events;
			return M4OK;
		case 27:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->shape_rendering;
			return M4OK;
		case 28:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->text_rendering;
			return M4OK;
		case 29:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->focusable;
			return M4OK;
		case 30:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->nav_index;
			return M4OK;
		case 31:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->externalResourcesRequired;
			return M4OK;
		case 32:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGpolylineElement *)node)->transform;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_polyline()
{
	SVGpolylineElement *p;
	SAFEALLOC(p, sizeof(SVGpolylineElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_polyline);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "polyline";
	((SFNode *p)->sgprivate->node_del = SVG_polyline_Del;
	((SFNode *p)->sgprivate->get_field = SVG_polyline_get_attribute;
#endif
	p->points = NewChain();
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_polygon_Del(SFNode *node)
{
	SVGpolygonElement *p = (SVGpolygonElement *)node;
	DeleteChain(p->points);
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_polygon_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "points";
			info->fieldType = SVG_Points_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->points;
			return M4OK;
		case 1:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->id;
			return M4OK;
		case 2:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->xml_base;
			return M4OK;
		case 3:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->xml_lang;
			return M4OK;
		case 4:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->xml_space;
			return M4OK;
		case 5:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->requiredFeatures;
			return M4OK;
		case 6:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->requiredExtensions;
			return M4OK;
		case 7:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->requiredFormats;
			return M4OK;
		case 8:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->systemLanguage;
			return M4OK;
		case 9:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->fill;
			return M4OK;
		case 10:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->fill_rule;
			return M4OK;
		case 11:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->stroke;
			return M4OK;
		case 12:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->stroke_dasharray;
			return M4OK;
		case 13:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->stroke_dashoffset;
			return M4OK;
		case 14:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->stroke_linecap;
			return M4OK;
		case 15:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->stroke_linejoin;
			return M4OK;
		case 16:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->stroke_miterlimit;
			return M4OK;
		case 17:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->stroke_width;
			return M4OK;
		case 18:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->vector_effect;
			return M4OK;
		case 19:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->color;
			return M4OK;
		case 20:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->color_rendering;
			return M4OK;
		case 21:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->fill_opacity;
			return M4OK;
		case 22:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->stroke_opacity;
			return M4OK;
		case 23:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->display;
			return M4OK;
		case 24:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->visibility;
			return M4OK;
		case 25:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->image_rendering;
			return M4OK;
		case 26:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->pointer_events;
			return M4OK;
		case 27:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->shape_rendering;
			return M4OK;
		case 28:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->text_rendering;
			return M4OK;
		case 29:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->focusable;
			return M4OK;
		case 30:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->nav_index;
			return M4OK;
		case 31:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->externalResourcesRequired;
			return M4OK;
		case 32:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGpolygonElement *)node)->transform;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_polygon()
{
	SVGpolygonElement *p;
	SAFEALLOC(p, sizeof(SVGpolygonElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_polygon);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "polygon";
	((SFNode *p)->sgprivate->node_del = SVG_polygon_Del;
	((SFNode *p)->sgprivate->get_field = SVG_polygon_get_attribute;
#endif
	p->points = NewChain();
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_text_Del(SFNode *node)
{
	SVGtextElement *p = (SVGtextElement *)node;
	DeleteChain(p->x);
	DeleteChain(p->y);
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_text_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "x";
			info->fieldType = SVG_Coordinates_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->x;
			return M4OK;
		case 1:
			info->name = "y";
			info->fieldType = SVG_Coordinates_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->y;
			return M4OK;
		case 2:
			info->name = "rotate";
			info->fieldType = SVG_Numbers_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->rotate;
			return M4OK;
		case 3:
			info->name = "string";
			info->fieldType = SVG_TextContent_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->string;
			return M4OK;
		case 4:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->id;
			return M4OK;
		case 5:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->xml_base;
			return M4OK;
		case 6:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->xml_lang;
			return M4OK;
		case 7:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->xml_space;
			return M4OK;
		case 8:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->requiredFeatures;
			return M4OK;
		case 9:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->requiredExtensions;
			return M4OK;
		case 10:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->requiredFormats;
			return M4OK;
		case 11:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->systemLanguage;
			return M4OK;
		case 12:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->focusable;
			return M4OK;
		case 13:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->nav_index;
			return M4OK;
		case 14:
			info->name = "editable";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->editable;
			return M4OK;
		case 15:
			info->name = "font-family";
			info->fieldType = SVG_FontFamilyValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->font_family;
			return M4OK;
		case 16:
			info->name = "font-size";
			info->fieldType = SVG_FontSizeValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->font_size;
			return M4OK;
		case 17:
			info->name = "font-style";
			info->fieldType = SVG_FontStyleValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->font_style;
			return M4OK;
		case 18:
			info->name = "font-weight";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->font_weight;
			return M4OK;
		case 19:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->fill;
			return M4OK;
		case 20:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->fill_rule;
			return M4OK;
		case 21:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->stroke;
			return M4OK;
		case 22:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->stroke_dasharray;
			return M4OK;
		case 23:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->stroke_dashoffset;
			return M4OK;
		case 24:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->stroke_linecap;
			return M4OK;
		case 25:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->stroke_linejoin;
			return M4OK;
		case 26:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->stroke_miterlimit;
			return M4OK;
		case 27:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->stroke_width;
			return M4OK;
		case 28:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->vector_effect;
			return M4OK;
		case 29:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->color;
			return M4OK;
		case 30:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->color_rendering;
			return M4OK;
		case 31:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->fill_opacity;
			return M4OK;
		case 32:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->stroke_opacity;
			return M4OK;
		case 33:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->display;
			return M4OK;
		case 34:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->visibility;
			return M4OK;
		case 35:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->image_rendering;
			return M4OK;
		case 36:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->pointer_events;
			return M4OK;
		case 37:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->shape_rendering;
			return M4OK;
		case 38:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->text_rendering;
			return M4OK;
		case 39:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->externalResourcesRequired;
			return M4OK;
		case 40:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->transform;
			return M4OK;
		case 41:
			info->name = "transform-host";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGtextElement *)node)->transform_host;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_text()
{
	SVGtextElement *p;
	SAFEALLOC(p, sizeof(SVGtextElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_text);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "text";
	((SFNode *p)->sgprivate->node_del = SVG_text_Del;
	((SFNode *p)->sgprivate->get_field = SVG_text_get_attribute;
#endif
	p->x = NewChain();
	p->y = NewChain();
	p->properties.font_family = &(p->font_family);
	p->font_size.type = SVGFLOAT_INHERIT;
	p->properties.font_size = &(p->font_size);
	p->properties.font_style = &(p->font_style);
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_tspan_Del(SFNode *node)
{
	SVGtspanElement *p = (SVGtspanElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_tspan_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->requiredFeatures;
			return M4OK;
		case 5:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->requiredExtensions;
			return M4OK;
		case 6:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->requiredFormats;
			return M4OK;
		case 7:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->systemLanguage;
			return M4OK;
		case 8:
			info->name = "font-family";
			info->fieldType = SVG_FontFamilyValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->font_family;
			return M4OK;
		case 9:
			info->name = "font-size";
			info->fieldType = SVG_FontSizeValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->font_size;
			return M4OK;
		case 10:
			info->name = "font-style";
			info->fieldType = SVG_FontStyleValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->font_style;
			return M4OK;
		case 11:
			info->name = "font-weight";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->font_weight;
			return M4OK;
		case 12:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->fill;
			return M4OK;
		case 13:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->fill_rule;
			return M4OK;
		case 14:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->stroke;
			return M4OK;
		case 15:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->stroke_dasharray;
			return M4OK;
		case 16:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->stroke_dashoffset;
			return M4OK;
		case 17:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->stroke_linecap;
			return M4OK;
		case 18:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->stroke_linejoin;
			return M4OK;
		case 19:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->stroke_miterlimit;
			return M4OK;
		case 20:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->stroke_width;
			return M4OK;
		case 21:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->vector_effect;
			return M4OK;
		case 22:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->color;
			return M4OK;
		case 23:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->color_rendering;
			return M4OK;
		case 24:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->fill_opacity;
			return M4OK;
		case 25:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->stroke_opacity;
			return M4OK;
		case 26:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->display;
			return M4OK;
		case 27:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->visibility;
			return M4OK;
		case 28:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->image_rendering;
			return M4OK;
		case 29:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->pointer_events;
			return M4OK;
		case 30:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->shape_rendering;
			return M4OK;
		case 31:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGtspanElement *)node)->text_rendering;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_tspan()
{
	SVGtspanElement *p;
	SAFEALLOC(p, sizeof(SVGtspanElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_tspan);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "tspan";
	((SFNode *p)->sgprivate->node_del = SVG_tspan_Del;
	((SFNode *p)->sgprivate->get_field = SVG_tspan_get_attribute;
#endif
	p->properties.font_family = &(p->font_family);
	p->font_size.type = SVGFLOAT_INHERIT;
	p->properties.font_size = &(p->font_size);
	p->properties.font_style = &(p->font_style);
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	return p;
}

static void SVG_a_Del(SFNode *node)
{
	SVGaElement *p = (SVGaElement *)node;
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_a_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "target";
			info->fieldType = SVG_LinkTarget_datatype;
			info->far_ptr = & ((SVGaElement *)node)->target;
			return M4OK;
		case 1:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->id;
			return M4OK;
		case 2:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGaElement *)node)->xml_base;
			return M4OK;
		case 3:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGaElement *)node)->xml_lang;
			return M4OK;
		case 4:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->xml_space;
			return M4OK;
		case 5:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGaElement *)node)->requiredFeatures;
			return M4OK;
		case 6:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGaElement *)node)->requiredExtensions;
			return M4OK;
		case 7:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGaElement *)node)->requiredFormats;
			return M4OK;
		case 8:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGaElement *)node)->systemLanguage;
			return M4OK;
		case 9:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->focusable;
			return M4OK;
		case 10:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->nav_index;
			return M4OK;
		case 11:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGaElement *)node)->fill;
			return M4OK;
		case 12:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGaElement *)node)->fill_rule;
			return M4OK;
		case 13:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGaElement *)node)->stroke;
			return M4OK;
		case 14:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGaElement *)node)->stroke_dasharray;
			return M4OK;
		case 15:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGaElement *)node)->stroke_dashoffset;
			return M4OK;
		case 16:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGaElement *)node)->stroke_linecap;
			return M4OK;
		case 17:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGaElement *)node)->stroke_linejoin;
			return M4OK;
		case 18:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGaElement *)node)->stroke_miterlimit;
			return M4OK;
		case 19:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGaElement *)node)->stroke_width;
			return M4OK;
		case 20:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->vector_effect;
			return M4OK;
		case 21:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGaElement *)node)->color;
			return M4OK;
		case 22:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->color_rendering;
			return M4OK;
		case 23:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGaElement *)node)->fill_opacity;
			return M4OK;
		case 24:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGaElement *)node)->stroke_opacity;
			return M4OK;
		case 25:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGaElement *)node)->display;
			return M4OK;
		case 26:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGaElement *)node)->visibility;
			return M4OK;
		case 27:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->image_rendering;
			return M4OK;
		case 28:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->pointer_events;
			return M4OK;
		case 29:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->shape_rendering;
			return M4OK;
		case 30:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->text_rendering;
			return M4OK;
		case 31:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->xlink_type;
			return M4OK;
		case 32:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGaElement *)node)->xlink_role;
			return M4OK;
		case 33:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGaElement *)node)->xlink_arcrole;
			return M4OK;
		case 34:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->xlink_title;
			return M4OK;
		case 35:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGaElement *)node)->xlink_href;
			return M4OK;
		case 36:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->xlink_show;
			return M4OK;
		case 37:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaElement *)node)->xlink_actuate;
			return M4OK;
		case 38:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGaElement *)node)->externalResourcesRequired;
			return M4OK;
		case 39:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGaElement *)node)->transform;
			return M4OK;
		case 40:
			info->name = "transform-host";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGaElement *)node)->transform_host;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_a()
{
	SVGaElement *p;
	SAFEALLOC(p, sizeof(SVGaElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_a);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "a";
	((SFNode *p)->sgprivate->node_del = SVG_a_Del;
	((SFNode *p)->sgprivate->get_field = SVG_a_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_script_Del(SFNode *node)
{
	SVGscriptElement *p = (SVGscriptElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_script_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "type";
			info->fieldType = SVG_ContentType_datatype;
			info->far_ptr = & ((SVGscriptElement *)node)->type;
			return M4OK;
		case 1:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGscriptElement *)node)->id;
			return M4OK;
		case 2:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGscriptElement *)node)->xml_base;
			return M4OK;
		case 3:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGscriptElement *)node)->xml_lang;
			return M4OK;
		case 4:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGscriptElement *)node)->xml_space;
			return M4OK;
		case 5:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGscriptElement *)node)->externalResourcesRequired;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_script()
{
	SVGscriptElement *p;
	SAFEALLOC(p, sizeof(SVGscriptElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_script);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "script";
	((SFNode *p)->sgprivate->node_del = SVG_script_Del;
	((SFNode *p)->sgprivate->get_field = SVG_script_get_attribute;
#endif
	return p;
}

static void SVG_animate_Del(SFNode *node)
{
	SVGanimateElement *p = (SVGanimateElement *)node;
	DeleteChain(p->begin);
	DeleteChain(p->end);
	DeleteChain(p->values.values);
	DeleteChain(p->keyTimes);
	DeleteChain(p->keySplines);
	free(p->from.value);
	free(p->to.value);
	free(p->by.value);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_animate_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->requiredFeatures;
			return M4OK;
		case 5:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->requiredExtensions;
			return M4OK;
		case 6:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->requiredFormats;
			return M4OK;
		case 7:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->systemLanguage;
			return M4OK;
		case 8:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->externalResourcesRequired;
			return M4OK;
		case 9:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->xlink_type;
			return M4OK;
		case 10:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->xlink_role;
			return M4OK;
		case 11:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->xlink_arcrole;
			return M4OK;
		case 12:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->xlink_title;
			return M4OK;
		case 13:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->xlink_href;
			return M4OK;
		case 14:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->xlink_show;
			return M4OK;
		case 15:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->xlink_actuate;
			return M4OK;
		case 16:
			info->name = "attributeName";
			info->fieldType = SMIL_AttributeName_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->attributeName;
			return M4OK;
		case 17:
			info->name = "attributeType";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->attributeType;
			return M4OK;
		case 18:
			info->name = "begin";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->begin;
			return M4OK;
		case 19:
			info->name = "dur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->dur;
			return M4OK;
		case 20:
			info->name = "end";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->end;
			return M4OK;
		case 21:
			info->name = "repeatCount";
			info->fieldType = SMIL_RepeatCountValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->repeatCount;
			return M4OK;
		case 22:
			info->name = "repeatDur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->repeatDur;
			return M4OK;
		case 23:
			info->name = "restart";
			info->fieldType = SMIL_RestartValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->restart;
			return M4OK;
		case 24:
			info->name = "min";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->min;
			return M4OK;
		case 25:
			info->name = "max";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->max;
			return M4OK;
		case 26:
			info->name = "fill";
			info->fieldType = SMIL_FreezeValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->freeze;
			return M4OK;
		case 27:
			info->name = "calcMode";
			info->fieldType = SMIL_CalcModeValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->calcMode;
			return M4OK;
		case 28:
			info->name = "values";
			info->fieldType = SMIL_AnimateValues_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->values;
			return M4OK;
		case 29:
			info->name = "keyTimes";
			info->fieldType = SMIL_KeyTimesValues_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->keyTimes;
			return M4OK;
		case 30:
			info->name = "keySplines";
			info->fieldType = SMIL_KeySplinesValues_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->keySplines;
			return M4OK;
		case 31:
			info->name = "from";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->from;
			return M4OK;
		case 32:
			info->name = "to";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->to;
			return M4OK;
		case 33:
			info->name = "by";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->by;
			return M4OK;
		case 34:
			info->name = "additive";
			info->fieldType = SMIL_AdditiveValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->additive;
			return M4OK;
		case 35:
			info->name = "accumulate";
			info->fieldType = SMIL_AccumulateValue_datatype;
			info->far_ptr = & ((SVGanimateElement *)node)->accumulate;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_animate()
{
	SVGanimateElement *p;
	SAFEALLOC(p, sizeof(SVGanimateElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_animate);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "animate";
	((SFNode *p)->sgprivate->node_del = SVG_animate_Del;
	((SFNode *p)->sgprivate->get_field = SVG_animate_get_attribute;
#endif
	p->begin = NewChain();
	p->end = NewChain();
	p->repeatCount = 1;
	p->repeatDur.clock_value = -1;
	p->min.type = SMILMinMaxDurRepeatDur_clock_value;
	p->values.values = NewChain();
	p->keyTimes = NewChain();
	p->keySplines = NewChain();
	return p;
}

static void SVG_set_Del(SFNode *node)
{
	SVGsetElement *p = (SVGsetElement *)node;
	free(p->to.value);
	DeleteChain(p->begin);
	DeleteChain(p->end);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_set_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "to";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->to;
			return M4OK;
		case 1:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->id;
			return M4OK;
		case 2:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->xml_base;
			return M4OK;
		case 3:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->xml_lang;
			return M4OK;
		case 4:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->xml_space;
			return M4OK;
		case 5:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->requiredFeatures;
			return M4OK;
		case 6:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->requiredExtensions;
			return M4OK;
		case 7:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->requiredFormats;
			return M4OK;
		case 8:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->systemLanguage;
			return M4OK;
		case 9:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->externalResourcesRequired;
			return M4OK;
		case 10:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->xlink_type;
			return M4OK;
		case 11:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->xlink_role;
			return M4OK;
		case 12:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->xlink_arcrole;
			return M4OK;
		case 13:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->xlink_title;
			return M4OK;
		case 14:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->xlink_href;
			return M4OK;
		case 15:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->xlink_show;
			return M4OK;
		case 16:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->xlink_actuate;
			return M4OK;
		case 17:
			info->name = "attributeName";
			info->fieldType = SMIL_AttributeName_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->attributeName;
			return M4OK;
		case 18:
			info->name = "attributeType";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->attributeType;
			return M4OK;
		case 19:
			info->name = "begin";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->begin;
			return M4OK;
		case 20:
			info->name = "dur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->dur;
			return M4OK;
		case 21:
			info->name = "end";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->end;
			return M4OK;
		case 22:
			info->name = "repeatCount";
			info->fieldType = SMIL_RepeatCountValue_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->repeatCount;
			return M4OK;
		case 23:
			info->name = "repeatDur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->repeatDur;
			return M4OK;
		case 24:
			info->name = "restart";
			info->fieldType = SMIL_RestartValue_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->restart;
			return M4OK;
		case 25:
			info->name = "min";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->min;
			return M4OK;
		case 26:
			info->name = "max";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->max;
			return M4OK;
		case 27:
			info->name = "fill";
			info->fieldType = SMIL_FreezeValue_datatype;
			info->far_ptr = & ((SVGsetElement *)node)->freeze;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_set()
{
	SVGsetElement *p;
	SAFEALLOC(p, sizeof(SVGsetElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_set);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "set";
	((SFNode *p)->sgprivate->node_del = SVG_set_Del;
	((SFNode *p)->sgprivate->get_field = SVG_set_get_attribute;
#endif
	p->begin = NewChain();
	p->end = NewChain();
	p->repeatCount = 1;
	p->repeatDur.clock_value = -1;
	p->min.type = SMILMinMaxDurRepeatDur_clock_value;
	return p;
}

static void SVG_animateMotion_Del(SFNode *node)
{
	SVGanimateMotionElement *p = (SVGanimateMotionElement *)node;
	DeleteChain(p->values.values);
	DeleteChain(p->keyTimes);
	DeleteChain(p->keySplines);
	free(p->from.value);
	free(p->to.value);
	free(p->by.value);
	DeleteChain(p->begin);
	DeleteChain(p->end);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_animateMotion_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "calcMode";
			info->fieldType = SMIL_CalcModeValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->calcMode;
			return M4OK;
		case 1:
			info->name = "values";
			info->fieldType = SMIL_AnimateValues_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->values;
			return M4OK;
		case 2:
			info->name = "keyTimes";
			info->fieldType = SMIL_KeyTimesValues_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->keyTimes;
			return M4OK;
		case 3:
			info->name = "keySplines";
			info->fieldType = SMIL_KeySplinesValues_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->keySplines;
			return M4OK;
		case 4:
			info->name = "from";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->from;
			return M4OK;
		case 5:
			info->name = "to";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->to;
			return M4OK;
		case 6:
			info->name = "by";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->by;
			return M4OK;
		case 7:
			info->name = "path";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->path;
			return M4OK;
		case 8:
			info->name = "keyPoints";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->keyPoints;
			return M4OK;
		case 9:
			info->name = "rotate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->rotate;
			return M4OK;
		case 10:
			info->name = "origin";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->origin;
			return M4OK;
		case 11:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->id;
			return M4OK;
		case 12:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->xml_base;
			return M4OK;
		case 13:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->xml_lang;
			return M4OK;
		case 14:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->xml_space;
			return M4OK;
		case 15:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->requiredFeatures;
			return M4OK;
		case 16:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->requiredExtensions;
			return M4OK;
		case 17:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->requiredFormats;
			return M4OK;
		case 18:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->systemLanguage;
			return M4OK;
		case 19:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->externalResourcesRequired;
			return M4OK;
		case 20:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->xlink_type;
			return M4OK;
		case 21:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->xlink_role;
			return M4OK;
		case 22:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->xlink_arcrole;
			return M4OK;
		case 23:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->xlink_title;
			return M4OK;
		case 24:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->xlink_href;
			return M4OK;
		case 25:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->xlink_show;
			return M4OK;
		case 26:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->xlink_actuate;
			return M4OK;
		case 27:
			info->name = "begin";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->begin;
			return M4OK;
		case 28:
			info->name = "dur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->dur;
			return M4OK;
		case 29:
			info->name = "end";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->end;
			return M4OK;
		case 30:
			info->name = "repeatCount";
			info->fieldType = SMIL_RepeatCountValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->repeatCount;
			return M4OK;
		case 31:
			info->name = "repeatDur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->repeatDur;
			return M4OK;
		case 32:
			info->name = "restart";
			info->fieldType = SMIL_RestartValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->restart;
			return M4OK;
		case 33:
			info->name = "min";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->min;
			return M4OK;
		case 34:
			info->name = "max";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->max;
			return M4OK;
		case 35:
			info->name = "fill";
			info->fieldType = SMIL_FreezeValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->freeze;
			return M4OK;
		case 36:
			info->name = "additive";
			info->fieldType = SMIL_AdditiveValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->additive;
			return M4OK;
		case 37:
			info->name = "accumulate";
			info->fieldType = SMIL_AccumulateValue_datatype;
			info->far_ptr = & ((SVGanimateMotionElement *)node)->accumulate;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_animateMotion()
{
	SVGanimateMotionElement *p;
	SAFEALLOC(p, sizeof(SVGanimateMotionElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_animateMotion);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "animateMotion";
	((SFNode *p)->sgprivate->node_del = SVG_animateMotion_Del;
	((SFNode *p)->sgprivate->get_field = SVG_animateMotion_get_attribute;
#endif
	p->calcMode = SMILCalcMode_paced;
	p->values.values = NewChain();
	p->keyTimes = NewChain();
	p->keySplines = NewChain();
	p->begin = NewChain();
	p->end = NewChain();
	p->repeatCount = 1;
	p->repeatDur.clock_value = -1;
	p->min.type = SMILMinMaxDurRepeatDur_clock_value;
	return p;
}

static void SVG_animateColor_Del(SFNode *node)
{
	SVGanimateColorElement *p = (SVGanimateColorElement *)node;
	DeleteChain(p->begin);
	DeleteChain(p->end);
	DeleteChain(p->values.values);
	DeleteChain(p->keyTimes);
	DeleteChain(p->keySplines);
	free(p->from.value);
	free(p->to.value);
	free(p->by.value);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_animateColor_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->requiredFeatures;
			return M4OK;
		case 5:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->requiredExtensions;
			return M4OK;
		case 6:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->requiredFormats;
			return M4OK;
		case 7:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->systemLanguage;
			return M4OK;
		case 8:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->externalResourcesRequired;
			return M4OK;
		case 9:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->xlink_type;
			return M4OK;
		case 10:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->xlink_role;
			return M4OK;
		case 11:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->xlink_arcrole;
			return M4OK;
		case 12:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->xlink_title;
			return M4OK;
		case 13:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->xlink_href;
			return M4OK;
		case 14:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->xlink_show;
			return M4OK;
		case 15:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->xlink_actuate;
			return M4OK;
		case 16:
			info->name = "attributeName";
			info->fieldType = SMIL_AttributeName_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->attributeName;
			return M4OK;
		case 17:
			info->name = "attributeType";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->attributeType;
			return M4OK;
		case 18:
			info->name = "begin";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->begin;
			return M4OK;
		case 19:
			info->name = "dur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->dur;
			return M4OK;
		case 20:
			info->name = "end";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->end;
			return M4OK;
		case 21:
			info->name = "repeatCount";
			info->fieldType = SMIL_RepeatCountValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->repeatCount;
			return M4OK;
		case 22:
			info->name = "repeatDur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->repeatDur;
			return M4OK;
		case 23:
			info->name = "restart";
			info->fieldType = SMIL_RestartValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->restart;
			return M4OK;
		case 24:
			info->name = "min";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->min;
			return M4OK;
		case 25:
			info->name = "max";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->max;
			return M4OK;
		case 26:
			info->name = "fill";
			info->fieldType = SMIL_FreezeValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->freeze;
			return M4OK;
		case 27:
			info->name = "calcMode";
			info->fieldType = SMIL_CalcModeValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->calcMode;
			return M4OK;
		case 28:
			info->name = "values";
			info->fieldType = SMIL_AnimateValues_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->values;
			return M4OK;
		case 29:
			info->name = "keyTimes";
			info->fieldType = SMIL_KeyTimesValues_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->keyTimes;
			return M4OK;
		case 30:
			info->name = "keySplines";
			info->fieldType = SMIL_KeySplinesValues_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->keySplines;
			return M4OK;
		case 31:
			info->name = "from";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->from;
			return M4OK;
		case 32:
			info->name = "to";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->to;
			return M4OK;
		case 33:
			info->name = "by";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->by;
			return M4OK;
		case 34:
			info->name = "additive";
			info->fieldType = SMIL_AdditiveValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->additive;
			return M4OK;
		case 35:
			info->name = "accumulate";
			info->fieldType = SMIL_AccumulateValue_datatype;
			info->far_ptr = & ((SVGanimateColorElement *)node)->accumulate;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_animateColor()
{
	SVGanimateColorElement *p;
	SAFEALLOC(p, sizeof(SVGanimateColorElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_animateColor);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "animateColor";
	((SFNode *p)->sgprivate->node_del = SVG_animateColor_Del;
	((SFNode *p)->sgprivate->get_field = SVG_animateColor_get_attribute;
#endif
	p->begin = NewChain();
	p->end = NewChain();
	p->repeatCount = 1;
	p->repeatDur.clock_value = -1;
	p->min.type = SMILMinMaxDurRepeatDur_clock_value;
	p->values.values = NewChain();
	p->keyTimes = NewChain();
	p->keySplines = NewChain();
	return p;
}

static void SVG_animateTransform_Del(SFNode *node)
{
	SVGanimateTransformElement *p = (SVGanimateTransformElement *)node;
	DeleteChain(p->begin);
	DeleteChain(p->end);
	DeleteChain(p->values.values);
	DeleteChain(p->keyTimes);
	DeleteChain(p->keySplines);
	free(p->from.value);
	free(p->to.value);
	free(p->by.value);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_animateTransform_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->type;
			return M4OK;
		case 1:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->id;
			return M4OK;
		case 2:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->xml_base;
			return M4OK;
		case 3:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->xml_lang;
			return M4OK;
		case 4:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->xml_space;
			return M4OK;
		case 5:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->requiredFeatures;
			return M4OK;
		case 6:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->requiredExtensions;
			return M4OK;
		case 7:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->requiredFormats;
			return M4OK;
		case 8:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->systemLanguage;
			return M4OK;
		case 9:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->externalResourcesRequired;
			return M4OK;
		case 10:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->xlink_type;
			return M4OK;
		case 11:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->xlink_role;
			return M4OK;
		case 12:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->xlink_arcrole;
			return M4OK;
		case 13:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->xlink_title;
			return M4OK;
		case 14:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->xlink_href;
			return M4OK;
		case 15:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->xlink_show;
			return M4OK;
		case 16:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->xlink_actuate;
			return M4OK;
		case 17:
			info->name = "attributeName";
			info->fieldType = SMIL_AttributeName_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->attributeName;
			return M4OK;
		case 18:
			info->name = "attributeType";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->attributeType;
			return M4OK;
		case 19:
			info->name = "begin";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->begin;
			return M4OK;
		case 20:
			info->name = "dur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->dur;
			return M4OK;
		case 21:
			info->name = "end";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->end;
			return M4OK;
		case 22:
			info->name = "repeatCount";
			info->fieldType = SMIL_RepeatCountValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->repeatCount;
			return M4OK;
		case 23:
			info->name = "repeatDur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->repeatDur;
			return M4OK;
		case 24:
			info->name = "restart";
			info->fieldType = SMIL_RestartValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->restart;
			return M4OK;
		case 25:
			info->name = "min";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->min;
			return M4OK;
		case 26:
			info->name = "max";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->max;
			return M4OK;
		case 27:
			info->name = "fill";
			info->fieldType = SMIL_FreezeValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->freeze;
			return M4OK;
		case 28:
			info->name = "calcMode";
			info->fieldType = SMIL_CalcModeValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->calcMode;
			return M4OK;
		case 29:
			info->name = "values";
			info->fieldType = SMIL_AnimateValues_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->values;
			return M4OK;
		case 30:
			info->name = "keyTimes";
			info->fieldType = SMIL_KeyTimesValues_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->keyTimes;
			return M4OK;
		case 31:
			info->name = "keySplines";
			info->fieldType = SMIL_KeySplinesValues_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->keySplines;
			return M4OK;
		case 32:
			info->name = "from";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->from;
			return M4OK;
		case 33:
			info->name = "to";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->to;
			return M4OK;
		case 34:
			info->name = "by";
			info->fieldType = SMIL_AnimateValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->by;
			return M4OK;
		case 35:
			info->name = "additive";
			info->fieldType = SMIL_AdditiveValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->additive;
			return M4OK;
		case 36:
			info->name = "accumulate";
			info->fieldType = SMIL_AccumulateValue_datatype;
			info->far_ptr = & ((SVGanimateTransformElement *)node)->accumulate;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_animateTransform()
{
	SVGanimateTransformElement *p;
	SAFEALLOC(p, sizeof(SVGanimateTransformElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_animateTransform);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "animateTransform";
	((SFNode *p)->sgprivate->node_del = SVG_animateTransform_Del;
	((SFNode *p)->sgprivate->get_field = SVG_animateTransform_get_attribute;
#endif
	p->begin = NewChain();
	p->end = NewChain();
	p->repeatCount = 1;
	p->repeatDur.clock_value = -1;
	p->min.type = SMILMinMaxDurRepeatDur_clock_value;
	p->values.values = NewChain();
	p->keyTimes = NewChain();
	p->keySplines = NewChain();
	return p;
}

static void SVG_mpath_Del(SFNode *node)
{
	SVGmpathElement *p = (SVGmpathElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_mpath_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmpathElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGmpathElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGmpathElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmpathElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmpathElement *)node)->xlink_type;
			return M4OK;
		case 5:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGmpathElement *)node)->xlink_role;
			return M4OK;
		case 6:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGmpathElement *)node)->xlink_arcrole;
			return M4OK;
		case 7:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmpathElement *)node)->xlink_title;
			return M4OK;
		case 8:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGmpathElement *)node)->xlink_href;
			return M4OK;
		case 9:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmpathElement *)node)->xlink_show;
			return M4OK;
		case 10:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmpathElement *)node)->xlink_actuate;
			return M4OK;
		case 11:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGmpathElement *)node)->externalResourcesRequired;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_mpath()
{
	SVGmpathElement *p;
	SAFEALLOC(p, sizeof(SVGmpathElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_mpath);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "mpath";
	((SFNode *p)->sgprivate->node_del = SVG_mpath_Del;
	((SFNode *p)->sgprivate->get_field = SVG_mpath_get_attribute;
#endif
	return p;
}

static void SVG_font_Del(SFNode *node)
{
	SVGfontElement *p = (SVGfontElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_font_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "horiz-origin-x";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->horiz_origin_x;
			return M4OK;
		case 1:
			info->name = "horiz-origin-y";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->horiz_origin_y;
			return M4OK;
		case 2:
			info->name = "horiz-adv-x";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->horiz_adv_x;
			return M4OK;
		case 3:
			info->name = "vert-origin-x";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->vert_origin_x;
			return M4OK;
		case 4:
			info->name = "vert-origin-y";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->vert_origin_y;
			return M4OK;
		case 5:
			info->name = "vert-adv-y";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->vert_adv_y;
			return M4OK;
		case 6:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->id;
			return M4OK;
		case 7:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->xml_base;
			return M4OK;
		case 8:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->xml_lang;
			return M4OK;
		case 9:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->xml_space;
			return M4OK;
		case 10:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->fill;
			return M4OK;
		case 11:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->fill_rule;
			return M4OK;
		case 12:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->stroke;
			return M4OK;
		case 13:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->stroke_dasharray;
			return M4OK;
		case 14:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->stroke_dashoffset;
			return M4OK;
		case 15:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->stroke_linecap;
			return M4OK;
		case 16:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->stroke_linejoin;
			return M4OK;
		case 17:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->stroke_miterlimit;
			return M4OK;
		case 18:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->stroke_width;
			return M4OK;
		case 19:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->vector_effect;
			return M4OK;
		case 20:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->color;
			return M4OK;
		case 21:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->color_rendering;
			return M4OK;
		case 22:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->fill_opacity;
			return M4OK;
		case 23:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->stroke_opacity;
			return M4OK;
		case 24:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->display;
			return M4OK;
		case 25:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->visibility;
			return M4OK;
		case 26:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->image_rendering;
			return M4OK;
		case 27:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->pointer_events;
			return M4OK;
		case 28:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->shape_rendering;
			return M4OK;
		case 29:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->text_rendering;
			return M4OK;
		case 30:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGfontElement *)node)->externalResourcesRequired;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_font()
{
	SVGfontElement *p;
	SAFEALLOC(p, sizeof(SVGfontElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_font);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "font";
	((SFNode *p)->sgprivate->node_del = SVG_font_Del;
	((SFNode *p)->sgprivate->get_field = SVG_font_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	return p;
}

static void SVG_font_face_Del(SFNode *node)
{
	SVGfont_faceElement *p = (SVGfont_faceElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_font_face_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "font-family";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->font_family;
			return M4OK;
		case 1:
			info->name = "font-style";
			info->fieldType = SVG_FontStyleValue_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->font_style;
			return M4OK;
		case 2:
			info->name = "font-variant";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->font_variant;
			return M4OK;
		case 3:
			info->name = "font-weight";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->font_weight;
			return M4OK;
		case 4:
			info->name = "font-stretch";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->font_stretch;
			return M4OK;
		case 5:
			info->name = "font-size";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->font_size;
			return M4OK;
		case 6:
			info->name = "unicode-range";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->unicode_range;
			return M4OK;
		case 7:
			info->name = "panose-1";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->panose_1;
			return M4OK;
		case 8:
			info->name = "widths";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->widths;
			return M4OK;
		case 9:
			info->name = "bbox";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->bbox;
			return M4OK;
		case 10:
			info->name = "units-per-em";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->units_per_em;
			return M4OK;
		case 11:
			info->name = "stemv";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->stemv;
			return M4OK;
		case 12:
			info->name = "stemh";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->stemh;
			return M4OK;
		case 13:
			info->name = "slope";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->slope;
			return M4OK;
		case 14:
			info->name = "cap-height";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->cap_height;
			return M4OK;
		case 15:
			info->name = "x-height";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->x_height;
			return M4OK;
		case 16:
			info->name = "accent-height";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->accent_height;
			return M4OK;
		case 17:
			info->name = "ascent";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->ascent;
			return M4OK;
		case 18:
			info->name = "descent";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->descent;
			return M4OK;
		case 19:
			info->name = "ideographic";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->ideographic;
			return M4OK;
		case 20:
			info->name = "alphabetic";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->alphabetic;
			return M4OK;
		case 21:
			info->name = "mathematical";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->mathematical;
			return M4OK;
		case 22:
			info->name = "hanging";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->hanging;
			return M4OK;
		case 23:
			info->name = "v-ideographic";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->v_ideographic;
			return M4OK;
		case 24:
			info->name = "v-alphabetic";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->v_alphabetic;
			return M4OK;
		case 25:
			info->name = "v-mathematical";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->v_mathematical;
			return M4OK;
		case 26:
			info->name = "v-hanging";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->v_hanging;
			return M4OK;
		case 27:
			info->name = "underline-position";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->underline_position;
			return M4OK;
		case 28:
			info->name = "underline-thickness";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->underline_thickness;
			return M4OK;
		case 29:
			info->name = "strikethrough-position";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->strikethrough_position;
			return M4OK;
		case 30:
			info->name = "strikethrough-thickness";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->strikethrough_thickness;
			return M4OK;
		case 31:
			info->name = "overline-position";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->overline_position;
			return M4OK;
		case 32:
			info->name = "overline-thickness";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->overline_thickness;
			return M4OK;
		case 33:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->id;
			return M4OK;
		case 34:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->xml_base;
			return M4OK;
		case 35:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->xml_lang;
			return M4OK;
		case 36:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_faceElement *)node)->xml_space;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_font_face()
{
	SVGfont_faceElement *p;
	SAFEALLOC(p, sizeof(SVGfont_faceElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_font_face);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "font_face";
	((SFNode *p)->sgprivate->node_del = SVG_font_face_Del;
	((SFNode *p)->sgprivate->get_field = SVG_font_face_get_attribute;
#endif
	return p;
}

static void SVG_glyph_Del(SFNode *node)
{
	SVGglyphElement *p = (SVGglyphElement *)node;
	DeleteChain(p->d.path_commands);
	DeleteChain(p->d.path_points);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_glyph_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "unicode";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->unicode;
			return M4OK;
		case 1:
			info->name = "glyph-name";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->glyph_name;
			return M4OK;
		case 2:
			info->name = "orientation";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->orientation;
			return M4OK;
		case 3:
			info->name = "arabic-form";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->arabic_form;
			return M4OK;
		case 4:
			info->name = "d";
			info->fieldType = SVG_PathData_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->d;
			return M4OK;
		case 5:
			info->name = "lang";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->lang;
			return M4OK;
		case 6:
			info->name = "horiz-adv-x";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->horiz_adv_x;
			return M4OK;
		case 7:
			info->name = "vert-origin-x";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->vert_origin_x;
			return M4OK;
		case 8:
			info->name = "vert-origin-y";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->vert_origin_y;
			return M4OK;
		case 9:
			info->name = "vert-adv-y";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->vert_adv_y;
			return M4OK;
		case 10:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->id;
			return M4OK;
		case 11:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->xml_base;
			return M4OK;
		case 12:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->xml_lang;
			return M4OK;
		case 13:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->xml_space;
			return M4OK;
		case 14:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->fill;
			return M4OK;
		case 15:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->fill_rule;
			return M4OK;
		case 16:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->stroke;
			return M4OK;
		case 17:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->stroke_dasharray;
			return M4OK;
		case 18:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->stroke_dashoffset;
			return M4OK;
		case 19:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->stroke_linecap;
			return M4OK;
		case 20:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->stroke_linejoin;
			return M4OK;
		case 21:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->stroke_miterlimit;
			return M4OK;
		case 22:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->stroke_width;
			return M4OK;
		case 23:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->vector_effect;
			return M4OK;
		case 24:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->color;
			return M4OK;
		case 25:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->color_rendering;
			return M4OK;
		case 26:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->fill_opacity;
			return M4OK;
		case 27:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->stroke_opacity;
			return M4OK;
		case 28:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->display;
			return M4OK;
		case 29:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->visibility;
			return M4OK;
		case 30:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->image_rendering;
			return M4OK;
		case 31:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->pointer_events;
			return M4OK;
		case 32:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->shape_rendering;
			return M4OK;
		case 33:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGglyphElement *)node)->text_rendering;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_glyph()
{
	SVGglyphElement *p;
	SAFEALLOC(p, sizeof(SVGglyphElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_glyph);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "glyph";
	((SFNode *p)->sgprivate->node_del = SVG_glyph_Del;
	((SFNode *p)->sgprivate->get_field = SVG_glyph_get_attribute;
#endif
	p->d.path_commands = NewChain();
	p->d.path_points = NewChain();
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	return p;
}

static void SVG_missing_glyph_Del(SFNode *node)
{
	SVGmissing_glyphElement *p = (SVGmissing_glyphElement *)node;
	DeleteChain(p->d.path_commands);
	DeleteChain(p->d.path_points);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_missing_glyph_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "d";
			info->fieldType = SVG_PathData_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->d;
			return M4OK;
		case 1:
			info->name = "horiz-adv-x";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->horiz_adv_x;
			return M4OK;
		case 2:
			info->name = "vert-origin-x";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->vert_origin_x;
			return M4OK;
		case 3:
			info->name = "vert-origin-y";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->vert_origin_y;
			return M4OK;
		case 4:
			info->name = "vert-adv-y";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->vert_adv_y;
			return M4OK;
		case 5:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->id;
			return M4OK;
		case 6:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->xml_base;
			return M4OK;
		case 7:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->xml_lang;
			return M4OK;
		case 8:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->xml_space;
			return M4OK;
		case 9:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->fill;
			return M4OK;
		case 10:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->fill_rule;
			return M4OK;
		case 11:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->stroke;
			return M4OK;
		case 12:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->stroke_dasharray;
			return M4OK;
		case 13:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->stroke_dashoffset;
			return M4OK;
		case 14:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->stroke_linecap;
			return M4OK;
		case 15:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->stroke_linejoin;
			return M4OK;
		case 16:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->stroke_miterlimit;
			return M4OK;
		case 17:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->stroke_width;
			return M4OK;
		case 18:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->vector_effect;
			return M4OK;
		case 19:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->color;
			return M4OK;
		case 20:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->color_rendering;
			return M4OK;
		case 21:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->fill_opacity;
			return M4OK;
		case 22:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->stroke_opacity;
			return M4OK;
		case 23:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->display;
			return M4OK;
		case 24:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->visibility;
			return M4OK;
		case 25:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->image_rendering;
			return M4OK;
		case 26:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->pointer_events;
			return M4OK;
		case 27:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->shape_rendering;
			return M4OK;
		case 28:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGmissing_glyphElement *)node)->text_rendering;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_missing_glyph()
{
	SVGmissing_glyphElement *p;
	SAFEALLOC(p, sizeof(SVGmissing_glyphElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_missing_glyph);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "missing_glyph";
	((SFNode *p)->sgprivate->node_del = SVG_missing_glyph_Del;
	((SFNode *p)->sgprivate->get_field = SVG_missing_glyph_get_attribute;
#endif
	p->d.path_commands = NewChain();
	p->d.path_points = NewChain();
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	return p;
}

static void SVG_hkern_Del(SFNode *node)
{
	SVGhkernElement *p = (SVGhkernElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_hkern_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "u1";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGhkernElement *)node)->u1;
			return M4OK;
		case 1:
			info->name = "g1";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGhkernElement *)node)->g1;
			return M4OK;
		case 2:
			info->name = "u2";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGhkernElement *)node)->u2;
			return M4OK;
		case 3:
			info->name = "g2";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGhkernElement *)node)->g2;
			return M4OK;
		case 4:
			info->name = "k";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGhkernElement *)node)->k;
			return M4OK;
		case 5:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGhkernElement *)node)->id;
			return M4OK;
		case 6:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGhkernElement *)node)->xml_base;
			return M4OK;
		case 7:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGhkernElement *)node)->xml_lang;
			return M4OK;
		case 8:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGhkernElement *)node)->xml_space;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_hkern()
{
	SVGhkernElement *p;
	SAFEALLOC(p, sizeof(SVGhkernElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_hkern);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "hkern";
	((SFNode *p)->sgprivate->node_del = SVG_hkern_Del;
	((SFNode *p)->sgprivate->get_field = SVG_hkern_get_attribute;
#endif
	return p;
}

static void SVG_font_face_src_Del(SFNode *node)
{
	SVGfont_face_srcElement *p = (SVGfont_face_srcElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_font_face_src_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_face_srcElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGfont_face_srcElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGfont_face_srcElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_face_srcElement *)node)->xml_space;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_font_face_src()
{
	SVGfont_face_srcElement *p;
	SAFEALLOC(p, sizeof(SVGfont_face_srcElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_font_face_src);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "font_face_src";
	((SFNode *p)->sgprivate->node_del = SVG_font_face_src_Del;
	((SFNode *p)->sgprivate->get_field = SVG_font_face_src_get_attribute;
#endif
	return p;
}

static void SVG_font_face_uri_Del(SFNode *node)
{
	SVGfont_face_uriElement *p = (SVGfont_face_uriElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_font_face_uri_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_face_uriElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGfont_face_uriElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGfont_face_uriElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_face_uriElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_face_uriElement *)node)->xlink_type;
			return M4OK;
		case 5:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGfont_face_uriElement *)node)->xlink_role;
			return M4OK;
		case 6:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGfont_face_uriElement *)node)->xlink_arcrole;
			return M4OK;
		case 7:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_face_uriElement *)node)->xlink_title;
			return M4OK;
		case 8:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGfont_face_uriElement *)node)->xlink_href;
			return M4OK;
		case 9:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_face_uriElement *)node)->xlink_show;
			return M4OK;
		case 10:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_face_uriElement *)node)->xlink_actuate;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_font_face_uri()
{
	SVGfont_face_uriElement *p;
	SAFEALLOC(p, sizeof(SVGfont_face_uriElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_font_face_uri);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "font_face_uri";
	((SFNode *p)->sgprivate->node_del = SVG_font_face_uri_Del;
	((SFNode *p)->sgprivate->get_field = SVG_font_face_uri_get_attribute;
#endif
	return p;
}

static void SVG_font_face_name_Del(SFNode *node)
{
	SVGfont_face_nameElement *p = (SVGfont_face_nameElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_font_face_name_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "name";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_face_nameElement *)node)->name;
			return M4OK;
		case 1:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_face_nameElement *)node)->id;
			return M4OK;
		case 2:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGfont_face_nameElement *)node)->xml_base;
			return M4OK;
		case 3:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGfont_face_nameElement *)node)->xml_lang;
			return M4OK;
		case 4:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGfont_face_nameElement *)node)->xml_space;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_font_face_name()
{
	SVGfont_face_nameElement *p;
	SAFEALLOC(p, sizeof(SVGfont_face_nameElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_font_face_name);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "font_face_name";
	((SFNode *p)->sgprivate->node_del = SVG_font_face_name_Del;
	((SFNode *p)->sgprivate->get_field = SVG_font_face_name_get_attribute;
#endif
	return p;
}

static void SVG_foreignObject_Del(SFNode *node)
{
	SVGforeignObjectElement *p = (SVGforeignObjectElement *)node;
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_foreignObject_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "x";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->x;
			return M4OK;
		case 1:
			info->name = "y";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->y;
			return M4OK;
		case 2:
			info->name = "width";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->width;
			return M4OK;
		case 3:
			info->name = "height";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->height;
			return M4OK;
		case 4:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->id;
			return M4OK;
		case 5:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->xml_base;
			return M4OK;
		case 6:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->xml_lang;
			return M4OK;
		case 7:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->xml_space;
			return M4OK;
		case 8:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->requiredFeatures;
			return M4OK;
		case 9:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->requiredExtensions;
			return M4OK;
		case 10:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->requiredFormats;
			return M4OK;
		case 11:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->systemLanguage;
			return M4OK;
		case 12:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->fill;
			return M4OK;
		case 13:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->fill_rule;
			return M4OK;
		case 14:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->stroke;
			return M4OK;
		case 15:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->stroke_dasharray;
			return M4OK;
		case 16:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->stroke_dashoffset;
			return M4OK;
		case 17:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->stroke_linecap;
			return M4OK;
		case 18:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->stroke_linejoin;
			return M4OK;
		case 19:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->stroke_miterlimit;
			return M4OK;
		case 20:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->stroke_width;
			return M4OK;
		case 21:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->vector_effect;
			return M4OK;
		case 22:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->color;
			return M4OK;
		case 23:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->color_rendering;
			return M4OK;
		case 24:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->fill_opacity;
			return M4OK;
		case 25:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->stroke_opacity;
			return M4OK;
		case 26:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->display;
			return M4OK;
		case 27:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->visibility;
			return M4OK;
		case 28:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->image_rendering;
			return M4OK;
		case 29:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->pointer_events;
			return M4OK;
		case 30:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->shape_rendering;
			return M4OK;
		case 31:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->text_rendering;
			return M4OK;
		case 32:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->focusable;
			return M4OK;
		case 33:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->nav_index;
			return M4OK;
		case 34:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->externalResourcesRequired;
			return M4OK;
		case 35:
			info->name = "background-fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->background_fill;
			return M4OK;
		case 36:
			info->name = "background-fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->background_fill_opacity;
			return M4OK;
		case 37:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->xlink_type;
			return M4OK;
		case 38:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->xlink_role;
			return M4OK;
		case 39:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->xlink_arcrole;
			return M4OK;
		case 40:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->xlink_title;
			return M4OK;
		case 41:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->xlink_href;
			return M4OK;
		case 42:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->xlink_show;
			return M4OK;
		case 43:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->xlink_actuate;
			return M4OK;
		case 44:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->transform;
			return M4OK;
		case 45:
			info->name = "transform-host";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGforeignObjectElement *)node)->transform_host;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_foreignObject()
{
	SVGforeignObjectElement *p;
	SAFEALLOC(p, sizeof(SVGforeignObjectElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_foreignObject);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "foreignObject";
	((SFNode *p)->sgprivate->node_del = SVG_foreignObject_Del;
	((SFNode *p)->sgprivate->get_field = SVG_foreignObject_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	p->transform = NewChain();
	return p;
}

static void SVG_linearGradient_Del(SFNode *node)
{
	SVGlinearGradientElement *p = (SVGlinearGradientElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_linearGradient_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "x1";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->x1;
			return M4OK;
		case 1:
			info->name = "y1";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->y1;
			return M4OK;
		case 2:
			info->name = "x2";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->x2;
			return M4OK;
		case 3:
			info->name = "y2";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->y2;
			return M4OK;
		case 4:
			info->name = "gradientUnits";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->gradientUnits;
			return M4OK;
		case 5:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->id;
			return M4OK;
		case 6:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->xml_base;
			return M4OK;
		case 7:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->xml_lang;
			return M4OK;
		case 8:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->xml_space;
			return M4OK;
		case 9:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->color;
			return M4OK;
		case 10:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->color_rendering;
			return M4OK;
		case 11:
			info->name = "stop-color";
			info->fieldType = SVG_SVGColor_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->stop_color;
			return M4OK;
		case 12:
			info->name = "stop-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->stop_opacity;
			return M4OK;
		case 13:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGlinearGradientElement *)node)->externalResourcesRequired;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_linearGradient()
{
	SVGlinearGradientElement *p;
	SAFEALLOC(p, sizeof(SVGlinearGradientElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_linearGradient);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "linearGradient";
	((SFNode *p)->sgprivate->node_del = SVG_linearGradient_Del;
	((SFNode *p)->sgprivate->get_field = SVG_linearGradient_get_attribute;
#endif
	return p;
}

static void SVG_radialGradient_Del(SFNode *node)
{
	SVGradialGradientElement *p = (SVGradialGradientElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_radialGradient_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "cx";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->cx;
			return M4OK;
		case 1:
			info->name = "cy";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->cy;
			return M4OK;
		case 2:
			info->name = "r";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->r;
			return M4OK;
		case 3:
			info->name = "gradientUnits";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->gradientUnits;
			return M4OK;
		case 4:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->id;
			return M4OK;
		case 5:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->xml_base;
			return M4OK;
		case 6:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->xml_lang;
			return M4OK;
		case 7:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->xml_space;
			return M4OK;
		case 8:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->color;
			return M4OK;
		case 9:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->color_rendering;
			return M4OK;
		case 10:
			info->name = "stop-color";
			info->fieldType = SVG_SVGColor_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->stop_color;
			return M4OK;
		case 11:
			info->name = "stop-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->stop_opacity;
			return M4OK;
		case 12:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGradialGradientElement *)node)->externalResourcesRequired;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_radialGradient()
{
	SVGradialGradientElement *p;
	SAFEALLOC(p, sizeof(SVGradialGradientElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_radialGradient);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "radialGradient";
	((SFNode *p)->sgprivate->node_del = SVG_radialGradient_Del;
	((SFNode *p)->sgprivate->get_field = SVG_radialGradient_get_attribute;
#endif
	return p;
}

static void SVG_stop_Del(SFNode *node)
{
	SVGstopElement *p = (SVGstopElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_stop_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "offset";
			info->fieldType = SVG_Number_datatype;
			info->far_ptr = & ((SVGstopElement *)node)->offset;
			return M4OK;
		case 1:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGstopElement *)node)->id;
			return M4OK;
		case 2:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGstopElement *)node)->xml_base;
			return M4OK;
		case 3:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGstopElement *)node)->xml_lang;
			return M4OK;
		case 4:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGstopElement *)node)->xml_space;
			return M4OK;
		case 5:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGstopElement *)node)->color;
			return M4OK;
		case 6:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGstopElement *)node)->color_rendering;
			return M4OK;
		case 7:
			info->name = "stop-color";
			info->fieldType = SVG_SVGColor_datatype;
			info->far_ptr = & ((SVGstopElement *)node)->stop_color;
			return M4OK;
		case 8:
			info->name = "stop-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGstopElement *)node)->stop_opacity;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_stop()
{
	SVGstopElement *p;
	SAFEALLOC(p, sizeof(SVGstopElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_stop);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "stop";
	((SFNode *p)->sgprivate->node_del = SVG_stop_Del;
	((SFNode *p)->sgprivate->get_field = SVG_stop_get_attribute;
#endif
	return p;
}

static void SVG_solidColor_Del(SFNode *node)
{
	SVGsolidColorElement *p = (SVGsolidColorElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_solidColor_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsolidColorElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGsolidColorElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGsolidColorElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsolidColorElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGsolidColorElement *)node)->color;
			return M4OK;
		case 5:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGsolidColorElement *)node)->color_rendering;
			return M4OK;
		case 6:
			info->name = "solid-color";
			info->fieldType = SVG_SVGColor_datatype;
			info->far_ptr = & ((SVGsolidColorElement *)node)->solid_color;
			return M4OK;
		case 7:
			info->name = "solid-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGsolidColorElement *)node)->solid_opacity;
			return M4OK;
		case 8:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGsolidColorElement *)node)->externalResourcesRequired;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_solidColor()
{
	SVGsolidColorElement *p;
	SAFEALLOC(p, sizeof(SVGsolidColorElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_solidColor);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "solidColor";
	((SFNode *p)->sgprivate->node_del = SVG_solidColor_Del;
	((SFNode *p)->sgprivate->get_field = SVG_solidColor_get_attribute;
#endif
	return p;
}

static void SVG_audio_Del(SFNode *node)
{
	SVGaudioElement *p = (SVGaudioElement *)node;
	DeleteChain(p->begin);
	DeleteChain(p->end);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_audio_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->requiredFeatures;
			return M4OK;
		case 5:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->requiredExtensions;
			return M4OK;
		case 6:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->requiredFormats;
			return M4OK;
		case 7:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->systemLanguage;
			return M4OK;
		case 8:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->xlink_type;
			return M4OK;
		case 9:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->xlink_role;
			return M4OK;
		case 10:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->xlink_arcrole;
			return M4OK;
		case 11:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->xlink_title;
			return M4OK;
		case 12:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->xlink_href;
			return M4OK;
		case 13:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->xlink_show;
			return M4OK;
		case 14:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->xlink_actuate;
			return M4OK;
		case 15:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->externalResourcesRequired;
			return M4OK;
		case 16:
			info->name = "begin";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->begin;
			return M4OK;
		case 17:
			info->name = "dur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->dur;
			return M4OK;
		case 18:
			info->name = "end";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->end;
			return M4OK;
		case 19:
			info->name = "repeatCount";
			info->fieldType = SMIL_RepeatCountValue_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->repeatCount;
			return M4OK;
		case 20:
			info->name = "repeatDur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->repeatDur;
			return M4OK;
		case 21:
			info->name = "restart";
			info->fieldType = SMIL_RestartValue_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->restart;
			return M4OK;
		case 22:
			info->name = "audio-level";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGaudioElement *)node)->audio_level;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_audio()
{
	SVGaudioElement *p;
	SAFEALLOC(p, sizeof(SVGaudioElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_audio);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "audio";
	((SFNode *p)->sgprivate->node_del = SVG_audio_Del;
	((SFNode *p)->sgprivate->get_field = SVG_audio_get_attribute;
#endif
	p->begin = NewChain();
	p->end = NewChain();
	p->repeatCount = 1;
	p->repeatDur.clock_value = -1;
	return p;
}

static void SVG_video_Del(SFNode *node)
{
	SVGvideoElement *p = (SVGvideoElement *)node;
	DeleteChain(p->begin);
	DeleteChain(p->end);
	DeleteChain(p->transform);
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_video_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "x";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->x;
			return M4OK;
		case 1:
			info->name = "y";
			info->fieldType = SVG_Coordinate_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->y;
			return M4OK;
		case 2:
			info->name = "width";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->width;
			return M4OK;
		case 3:
			info->name = "height";
			info->fieldType = SVG_Length_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->height;
			return M4OK;
		case 4:
			info->name = "preserveAspectRatio";
			info->fieldType = SVG_PreserveAspectRatioSpec_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->preserveAspectRatio;
			return M4OK;
		case 5:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->id;
			return M4OK;
		case 6:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->xml_base;
			return M4OK;
		case 7:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->xml_lang;
			return M4OK;
		case 8:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->xml_space;
			return M4OK;
		case 9:
			info->name = "requiredFeatures";
			info->fieldType = SVG_FeatureList_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->requiredFeatures;
			return M4OK;
		case 10:
			info->name = "requiredExtensions";
			info->fieldType = SVG_ExtensionList_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->requiredExtensions;
			return M4OK;
		case 11:
			info->name = "requiredFormats";
			info->fieldType = SVG_FormatList_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->requiredFormats;
			return M4OK;
		case 12:
			info->name = "systemLanguage";
			info->fieldType = SVG_LanguageCodes_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->systemLanguage;
			return M4OK;
		case 13:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->fill_opacity;
			return M4OK;
		case 14:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->stroke_opacity;
			return M4OK;
		case 15:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->display;
			return M4OK;
		case 16:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->visibility;
			return M4OK;
		case 17:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->image_rendering;
			return M4OK;
		case 18:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->pointer_events;
			return M4OK;
		case 19:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->shape_rendering;
			return M4OK;
		case 20:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->text_rendering;
			return M4OK;
		case 21:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->focusable;
			return M4OK;
		case 22:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->nav_index;
			return M4OK;
		case 23:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->xlink_type;
			return M4OK;
		case 24:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->xlink_role;
			return M4OK;
		case 25:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->xlink_arcrole;
			return M4OK;
		case 26:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->xlink_title;
			return M4OK;
		case 27:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->xlink_href;
			return M4OK;
		case 28:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->xlink_show;
			return M4OK;
		case 29:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->xlink_actuate;
			return M4OK;
		case 30:
			info->name = "externalResourcesRequired";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->externalResourcesRequired;
			return M4OK;
		case 31:
			info->name = "begin";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->begin;
			return M4OK;
		case 32:
			info->name = "dur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->dur;
			return M4OK;
		case 33:
			info->name = "end";
			info->fieldType = SMIL_BeginOrEndValues_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->end;
			return M4OK;
		case 34:
			info->name = "repeatCount";
			info->fieldType = SMIL_RepeatCountValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->repeatCount;
			return M4OK;
		case 35:
			info->name = "repeatDur";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->repeatDur;
			return M4OK;
		case 36:
			info->name = "restart";
			info->fieldType = SMIL_RestartValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->restart;
			return M4OK;
		case 37:
			info->name = "min";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->min;
			return M4OK;
		case 38:
			info->name = "max";
			info->fieldType = SMIL_MinMaxDurRepeatDurValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->max;
			return M4OK;
		case 39:
			info->name = "fill";
			info->fieldType = SMIL_FreezeValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->freeze;
			return M4OK;
		case 40:
			info->name = "audio-level";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->audio_level;
			return M4OK;
		case 41:
			info->name = "background-fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->background_fill;
			return M4OK;
		case 42:
			info->name = "background-fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->background_fill_opacity;
			return M4OK;
		case 43:
			info->name = "fill";
			info->fieldType = SMIL_FreezeValue_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->freeze;
			return M4OK;
		case 44:
			info->name = "transform";
			info->fieldType = SVG_TransformList_datatype;
			info->far_ptr = & ((SVGvideoElement *)node)->transform;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_video()
{
	SVGvideoElement *p;
	SAFEALLOC(p, sizeof(SVGvideoElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_video);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "video";
	((SFNode *p)->sgprivate->node_del = SVG_video_Del;
	((SFNode *p)->sgprivate->get_field = SVG_video_get_attribute;
#endif
	return p;
}

static void SVG_pageSet_Del(SFNode *node)
{
	SVGpageSetElement *p = (SVGpageSetElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_pageSet_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->fill;
			return M4OK;
		case 5:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->fill_rule;
			return M4OK;
		case 6:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->stroke;
			return M4OK;
		case 7:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->stroke_dasharray;
			return M4OK;
		case 8:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->stroke_dashoffset;
			return M4OK;
		case 9:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->stroke_linecap;
			return M4OK;
		case 10:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->stroke_linejoin;
			return M4OK;
		case 11:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->stroke_miterlimit;
			return M4OK;
		case 12:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->stroke_width;
			return M4OK;
		case 13:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->vector_effect;
			return M4OK;
		case 14:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->color;
			return M4OK;
		case 15:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->color_rendering;
			return M4OK;
		case 16:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->fill_opacity;
			return M4OK;
		case 17:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->stroke_opacity;
			return M4OK;
		case 18:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->display;
			return M4OK;
		case 19:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->visibility;
			return M4OK;
		case 20:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->image_rendering;
			return M4OK;
		case 21:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->pointer_events;
			return M4OK;
		case 22:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->shape_rendering;
			return M4OK;
		case 23:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageSetElement *)node)->text_rendering;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_pageSet()
{
	SVGpageSetElement *p;
	SAFEALLOC(p, sizeof(SVGpageSetElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_pageSet);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "pageSet";
	((SFNode *p)->sgprivate->node_del = SVG_pageSet_Del;
	((SFNode *p)->sgprivate->get_field = SVG_pageSet_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	return p;
}

static void SVG_page_Del(SFNode *node)
{
	SVGpageElement *p = (SVGpageElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_page_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "page-orientation";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->page_orientation;
			return M4OK;
		case 1:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->id;
			return M4OK;
		case 2:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->xml_base;
			return M4OK;
		case 3:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->xml_lang;
			return M4OK;
		case 4:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->xml_space;
			return M4OK;
		case 5:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->fill;
			return M4OK;
		case 6:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->fill_rule;
			return M4OK;
		case 7:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->stroke;
			return M4OK;
		case 8:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->stroke_dasharray;
			return M4OK;
		case 9:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->stroke_dashoffset;
			return M4OK;
		case 10:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->stroke_linecap;
			return M4OK;
		case 11:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->stroke_linejoin;
			return M4OK;
		case 12:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->stroke_miterlimit;
			return M4OK;
		case 13:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->stroke_width;
			return M4OK;
		case 14:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->vector_effect;
			return M4OK;
		case 15:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->color;
			return M4OK;
		case 16:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->color_rendering;
			return M4OK;
		case 17:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->fill_opacity;
			return M4OK;
		case 18:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->stroke_opacity;
			return M4OK;
		case 19:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->display;
			return M4OK;
		case 20:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->visibility;
			return M4OK;
		case 21:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->image_rendering;
			return M4OK;
		case 22:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->pointer_events;
			return M4OK;
		case 23:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->shape_rendering;
			return M4OK;
		case 24:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->text_rendering;
			return M4OK;
		case 25:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->focusable;
			return M4OK;
		case 26:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGpageElement *)node)->nav_index;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_page()
{
	SVGpageElement *p;
	SAFEALLOC(p, sizeof(SVGpageElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_page);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "page";
	((SFNode *p)->sgprivate->node_del = SVG_page_Del;
	((SFNode *p)->sgprivate->get_field = SVG_page_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	return p;
}

static void SVG_flowRoot_Del(SFNode *node)
{
	SVGflowRootElement *p = (SVGflowRootElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_flowRoot_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->fill;
			return M4OK;
		case 5:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->fill_rule;
			return M4OK;
		case 6:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->stroke;
			return M4OK;
		case 7:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->stroke_dasharray;
			return M4OK;
		case 8:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->stroke_dashoffset;
			return M4OK;
		case 9:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->stroke_linecap;
			return M4OK;
		case 10:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->stroke_linejoin;
			return M4OK;
		case 11:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->stroke_miterlimit;
			return M4OK;
		case 12:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->stroke_width;
			return M4OK;
		case 13:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->vector_effect;
			return M4OK;
		case 14:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->color;
			return M4OK;
		case 15:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->color_rendering;
			return M4OK;
		case 16:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->fill_opacity;
			return M4OK;
		case 17:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->stroke_opacity;
			return M4OK;
		case 18:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->display;
			return M4OK;
		case 19:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->visibility;
			return M4OK;
		case 20:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->image_rendering;
			return M4OK;
		case 21:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->pointer_events;
			return M4OK;
		case 22:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->shape_rendering;
			return M4OK;
		case 23:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->text_rendering;
			return M4OK;
		case 24:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->focusable;
			return M4OK;
		case 25:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowRootElement *)node)->nav_index;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_flowRoot()
{
	SVGflowRootElement *p;
	SAFEALLOC(p, sizeof(SVGflowRootElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_flowRoot);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "flowRoot";
	((SFNode *p)->sgprivate->node_del = SVG_flowRoot_Del;
	((SFNode *p)->sgprivate->get_field = SVG_flowRoot_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	return p;
}

static void SVG_flowRegion_Del(SFNode *node)
{
	SVGflowRegionElement *p = (SVGflowRegionElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_flowRegion_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowRegionElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGflowRegionElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGflowRegionElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowRegionElement *)node)->xml_space;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_flowRegion()
{
	SVGflowRegionElement *p;
	SAFEALLOC(p, sizeof(SVGflowRegionElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_flowRegion);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "flowRegion";
	((SFNode *p)->sgprivate->node_del = SVG_flowRegion_Del;
	((SFNode *p)->sgprivate->get_field = SVG_flowRegion_get_attribute;
#endif
	return p;
}

static void SVG_flowPara_Del(SFNode *node)
{
	SVGflowParaElement *p = (SVGflowParaElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_flowPara_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->fill;
			return M4OK;
		case 5:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->fill_rule;
			return M4OK;
		case 6:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->stroke;
			return M4OK;
		case 7:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->stroke_dasharray;
			return M4OK;
		case 8:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->stroke_dashoffset;
			return M4OK;
		case 9:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->stroke_linecap;
			return M4OK;
		case 10:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->stroke_linejoin;
			return M4OK;
		case 11:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->stroke_miterlimit;
			return M4OK;
		case 12:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->stroke_width;
			return M4OK;
		case 13:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->vector_effect;
			return M4OK;
		case 14:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->color;
			return M4OK;
		case 15:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->color_rendering;
			return M4OK;
		case 16:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->fill_opacity;
			return M4OK;
		case 17:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->stroke_opacity;
			return M4OK;
		case 18:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->display;
			return M4OK;
		case 19:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->visibility;
			return M4OK;
		case 20:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->image_rendering;
			return M4OK;
		case 21:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->pointer_events;
			return M4OK;
		case 22:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->shape_rendering;
			return M4OK;
		case 23:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->text_rendering;
			return M4OK;
		case 24:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->focusable;
			return M4OK;
		case 25:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->nav_index;
			return M4OK;
		case 26:
			info->name = "editable";
			info->fieldType = SVG_Boolean_datatype;
			info->far_ptr = & ((SVGflowParaElement *)node)->editable;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_flowPara()
{
	SVGflowParaElement *p;
	SAFEALLOC(p, sizeof(SVGflowParaElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_flowPara);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "flowPara";
	((SFNode *p)->sgprivate->node_del = SVG_flowPara_Del;
	((SFNode *p)->sgprivate->get_field = SVG_flowPara_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	return p;
}

static void SVG_flowSpan_Del(SFNode *node)
{
	SVGflowSpanElement *p = (SVGflowSpanElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_flowSpan_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->id;
			return M4OK;
		case 1:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->xml_base;
			return M4OK;
		case 2:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->xml_lang;
			return M4OK;
		case 3:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->xml_space;
			return M4OK;
		case 4:
			info->name = "fill";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->fill;
			return M4OK;
		case 5:
			info->name = "fill-rule";
			info->fieldType = SVG_ClipFillRule_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->fill_rule;
			return M4OK;
		case 6:
			info->name = "stroke";
			info->fieldType = SVG_Paint_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->stroke;
			return M4OK;
		case 7:
			info->name = "stroke-dasharray";
			info->fieldType = SVG_StrokeDashArrayValue_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->stroke_dasharray;
			return M4OK;
		case 8:
			info->name = "stroke-dashoffset";
			info->fieldType = SVG_StrokeDashOffsetValue_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->stroke_dashoffset;
			return M4OK;
		case 9:
			info->name = "stroke-linecap";
			info->fieldType = SVG_StrokeLineCapValue_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->stroke_linecap;
			return M4OK;
		case 10:
			info->name = "stroke-linejoin";
			info->fieldType = SVG_StrokeLineJoinValue_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->stroke_linejoin;
			return M4OK;
		case 11:
			info->name = "stroke-miterlimit";
			info->fieldType = SVG_StrokeMiterLimitValue_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->stroke_miterlimit;
			return M4OK;
		case 12:
			info->name = "stroke-width";
			info->fieldType = SVG_StrokeWidthValue_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->stroke_width;
			return M4OK;
		case 13:
			info->name = "vector-effect";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->vector_effect;
			return M4OK;
		case 14:
			info->name = "color";
			info->fieldType = SVG_Color_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->color;
			return M4OK;
		case 15:
			info->name = "color-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->color_rendering;
			return M4OK;
		case 16:
			info->name = "fill-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->fill_opacity;
			return M4OK;
		case 17:
			info->name = "stroke-opacity";
			info->fieldType = SVG_OpacityValue_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->stroke_opacity;
			return M4OK;
		case 18:
			info->name = "display";
			info->fieldType = SVG_DisplayValue_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->display;
			return M4OK;
		case 19:
			info->name = "visibility";
			info->fieldType = SVG_VisibilityValue_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->visibility;
			return M4OK;
		case 20:
			info->name = "image-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->image_rendering;
			return M4OK;
		case 21:
			info->name = "pointer-events";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->pointer_events;
			return M4OK;
		case 22:
			info->name = "shape-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->shape_rendering;
			return M4OK;
		case 23:
			info->name = "text-rendering";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->text_rendering;
			return M4OK;
		case 24:
			info->name = "focusable";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->focusable;
			return M4OK;
		case 25:
			info->name = "nav-index";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGflowSpanElement *)node)->nav_index;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_flowSpan()
{
	SVGflowSpanElement *p;
	SAFEALLOC(p, sizeof(SVGflowSpanElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_flowSpan);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "flowSpan";
	((SFNode *p)->sgprivate->node_del = SVG_flowSpan_Del;
	((SFNode *p)->sgprivate->get_field = SVG_flowSpan_get_attribute;
#endif
	p->fill.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.fill = &(p->fill);
	p->fill_rule = SVGFillRule_inherit;
	p->properties.fill_rule = &(p->fill_rule);
	p->stroke.paintType = SVG_PAINTTYPE_INHERIT;
	p->properties.stroke = &(p->stroke);
	p->stroke_linecap = SVGStrokeLineCap_inherit;
	p->properties.stroke_linecap = &(p->stroke_linecap);
	p->stroke_linejoin = SVGStrokeLineJoin_inherit;
	p->properties.stroke_linejoin = &(p->stroke_linejoin);
	p->stroke_miterlimit.type = SVGFLOAT_INHERIT;
	p->properties.stroke_miterlimit = &(p->stroke_miterlimit);
	p->stroke_width.unitType = SVG_LENGTHTYPE_INHERIT;
	p->properties.stroke_width = &(p->stroke_width);
	p->fill_opacity.type = SVGFLOAT_INHERIT;
	p->properties.fill_opacity = &(p->fill_opacity);
	p->stroke_opacity.type = SVGFLOAT_INHERIT;
	p->properties.stroke_opacity = &(p->stroke_opacity);
	return p;
}

static void SVG_handler_Del(SFNode *node)
{
	SVGhandlerElement *p = (SVGhandlerElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_handler_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "ev:event";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGhandlerElement *)node)->ev_event;
			return M4OK;
		case 1:
			info->name = "type";
			info->fieldType = SVG_ContentType_datatype;
			info->far_ptr = & ((SVGhandlerElement *)node)->type;
			return M4OK;
		case 2:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGhandlerElement *)node)->id;
			return M4OK;
		case 3:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGhandlerElement *)node)->xml_base;
			return M4OK;
		case 4:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGhandlerElement *)node)->xml_lang;
			return M4OK;
		case 5:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGhandlerElement *)node)->xml_space;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_handler()
{
	SVGhandlerElement *p;
	SAFEALLOC(p, sizeof(SVGhandlerElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_handler);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "handler";
	((SFNode *p)->sgprivate->node_del = SVG_handler_Del;
	((SFNode *p)->sgprivate->get_field = SVG_handler_get_attribute;
#endif
	return p;
}

static void SVG_prefetch_Del(SFNode *node)
{
	SVGprefetchElement *p = (SVGprefetchElement *)node;
	DestroyChildrenNode((SFNode *) p);
	SFNode_Delete((SFNode *)p);
}

static M4Err SVG_prefetch_get_attribute(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
		case 0:
			info->name = "mediaSize";
			info->fieldType = SVG_NumberOrPercentage_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->mediaSize;
			return M4OK;
		case 1:
			info->name = "mediaTime";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->mediaTime;
			return M4OK;
		case 2:
			info->name = "mediaEncodings";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->mediaEncodings;
			return M4OK;
		case 3:
			info->name = "bandwidth";
			info->fieldType = SVG_NumberOrPercentage_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->bandwidth;
			return M4OK;
		case 4:
			info->name = "id";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->id;
			return M4OK;
		case 5:
			info->name = "xml:base";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->xml_base;
			return M4OK;
		case 6:
			info->name = "xml:lang";
			info->fieldType = SVG_LanguageCode_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->xml_lang;
			return M4OK;
		case 7:
			info->name = "xml:space";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->xml_space;
			return M4OK;
		case 8:
			info->name = "xlink:type";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->xlink_type;
			return M4OK;
		case 9:
			info->name = "xlink:role";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->xlink_role;
			return M4OK;
		case 10:
			info->name = "xlink:arcrole";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->xlink_arcrole;
			return M4OK;
		case 11:
			info->name = "xlink:title";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->xlink_title;
			return M4OK;
		case 12:
			info->name = "xlink:href";
			info->fieldType = SVG_URI_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->xlink_href;
			return M4OK;
		case 13:
			info->name = "xlink:show";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->xlink_show;
			return M4OK;
		case 14:
			info->name = "xlink:actuate";
			info->fieldType = SVG_String_datatype;
			info->far_ptr = & ((SVGprefetchElement *)node)->xlink_actuate;
			return M4OK;
		default: return M4BadParam;
	}
}

void *SVG_New_prefetch()
{
	SVGprefetchElement *p;
	SAFEALLOC(p, sizeof(SVGprefetchElement));
	if (!p) return NULL;
	Node_Setup((SFNode *)p, TAG_SVG_prefetch);
	SetupChildrenNode((SFNode *) p);
#ifdef NODE_USE_POINTERS
	((SFNode *p)->sgprivate->name = "prefetch";
	((SFNode *p)->sgprivate->node_del = SVG_prefetch_Del;
	((SFNode *p)->sgprivate->get_field = SVG_prefetch_get_attribute;
#endif
	return p;
}

SVGElement *SVG_CreateNode(u32 ElementTag)
{
	switch (ElementTag) {
		case TAG_SVG_svg: return SVG_New_svg();
		case TAG_SVG_g: return SVG_New_g();
		case TAG_SVG_defs: return SVG_New_defs();
		case TAG_SVG_desc: return SVG_New_desc();
		case TAG_SVG_title: return SVG_New_title();
		case TAG_SVG_metadata: return SVG_New_metadata();
		case TAG_SVG_use: return SVG_New_use();
		case TAG_SVG_switch: return SVG_New_switch();
		case TAG_SVG_image: return SVG_New_image();
		case TAG_SVG_path: return SVG_New_path();
		case TAG_SVG_rect: return SVG_New_rect();
		case TAG_SVG_circle: return SVG_New_circle();
		case TAG_SVG_line: return SVG_New_line();
		case TAG_SVG_ellipse: return SVG_New_ellipse();
		case TAG_SVG_polyline: return SVG_New_polyline();
		case TAG_SVG_polygon: return SVG_New_polygon();
		case TAG_SVG_text: return SVG_New_text();
		case TAG_SVG_tspan: return SVG_New_tspan();
		case TAG_SVG_a: return SVG_New_a();
		case TAG_SVG_script: return SVG_New_script();
		case TAG_SVG_animate: return SVG_New_animate();
		case TAG_SVG_set: return SVG_New_set();
		case TAG_SVG_animateMotion: return SVG_New_animateMotion();
		case TAG_SVG_animateColor: return SVG_New_animateColor();
		case TAG_SVG_animateTransform: return SVG_New_animateTransform();
		case TAG_SVG_mpath: return SVG_New_mpath();
		case TAG_SVG_font: return SVG_New_font();
		case TAG_SVG_font_face: return SVG_New_font_face();
		case TAG_SVG_glyph: return SVG_New_glyph();
		case TAG_SVG_missing_glyph: return SVG_New_missing_glyph();
		case TAG_SVG_hkern: return SVG_New_hkern();
		case TAG_SVG_font_face_src: return SVG_New_font_face_src();
		case TAG_SVG_font_face_uri: return SVG_New_font_face_uri();
		case TAG_SVG_font_face_name: return SVG_New_font_face_name();
		case TAG_SVG_foreignObject: return SVG_New_foreignObject();
		case TAG_SVG_linearGradient: return SVG_New_linearGradient();
		case TAG_SVG_radialGradient: return SVG_New_radialGradient();
		case TAG_SVG_stop: return SVG_New_stop();
		case TAG_SVG_solidColor: return SVG_New_solidColor();
		case TAG_SVG_audio: return SVG_New_audio();
		case TAG_SVG_video: return SVG_New_video();
		case TAG_SVG_pageSet: return SVG_New_pageSet();
		case TAG_SVG_page: return SVG_New_page();
		case TAG_SVG_flowRoot: return SVG_New_flowRoot();
		case TAG_SVG_flowRegion: return SVG_New_flowRegion();
		case TAG_SVG_flowPara: return SVG_New_flowPara();
		case TAG_SVG_flowSpan: return SVG_New_flowSpan();
		case TAG_SVG_handler: return SVG_New_handler();
		case TAG_SVG_prefetch: return SVG_New_prefetch();
		default: return NULL;
	}
}

void SVGElement_Del(SVGElement *elt)
{
	SFNode *node = (SFNode *)elt;
	switch (node->sgprivate->tag) {
		case TAG_SVG_svg: SVG_svg_Del(node); return;
		case TAG_SVG_g: SVG_g_Del(node); return;
		case TAG_SVG_defs: SVG_defs_Del(node); return;
		case TAG_SVG_desc: SVG_desc_Del(node); return;
		case TAG_SVG_title: SVG_title_Del(node); return;
		case TAG_SVG_metadata: SVG_metadata_Del(node); return;
		case TAG_SVG_use: SVG_use_Del(node); return;
		case TAG_SVG_switch: SVG_switch_Del(node); return;
		case TAG_SVG_image: SVG_image_Del(node); return;
		case TAG_SVG_path: SVG_path_Del(node); return;
		case TAG_SVG_rect: SVG_rect_Del(node); return;
		case TAG_SVG_circle: SVG_circle_Del(node); return;
		case TAG_SVG_line: SVG_line_Del(node); return;
		case TAG_SVG_ellipse: SVG_ellipse_Del(node); return;
		case TAG_SVG_polyline: SVG_polyline_Del(node); return;
		case TAG_SVG_polygon: SVG_polygon_Del(node); return;
		case TAG_SVG_text: SVG_text_Del(node); return;
		case TAG_SVG_tspan: SVG_tspan_Del(node); return;
		case TAG_SVG_a: SVG_a_Del(node); return;
		case TAG_SVG_script: SVG_script_Del(node); return;
		case TAG_SVG_animate: SVG_animate_Del(node); return;
		case TAG_SVG_set: SVG_set_Del(node); return;
		case TAG_SVG_animateMotion: SVG_animateMotion_Del(node); return;
		case TAG_SVG_animateColor: SVG_animateColor_Del(node); return;
		case TAG_SVG_animateTransform: SVG_animateTransform_Del(node); return;
		case TAG_SVG_mpath: SVG_mpath_Del(node); return;
		case TAG_SVG_font: SVG_font_Del(node); return;
		case TAG_SVG_font_face: SVG_font_face_Del(node); return;
		case TAG_SVG_glyph: SVG_glyph_Del(node); return;
		case TAG_SVG_missing_glyph: SVG_missing_glyph_Del(node); return;
		case TAG_SVG_hkern: SVG_hkern_Del(node); return;
		case TAG_SVG_font_face_src: SVG_font_face_src_Del(node); return;
		case TAG_SVG_font_face_uri: SVG_font_face_uri_Del(node); return;
		case TAG_SVG_font_face_name: SVG_font_face_name_Del(node); return;
		case TAG_SVG_foreignObject: SVG_foreignObject_Del(node); return;
		case TAG_SVG_linearGradient: SVG_linearGradient_Del(node); return;
		case TAG_SVG_radialGradient: SVG_radialGradient_Del(node); return;
		case TAG_SVG_stop: SVG_stop_Del(node); return;
		case TAG_SVG_solidColor: SVG_solidColor_Del(node); return;
		case TAG_SVG_audio: SVG_audio_Del(node); return;
		case TAG_SVG_video: SVG_video_Del(node); return;
		case TAG_SVG_pageSet: SVG_pageSet_Del(node); return;
		case TAG_SVG_page: SVG_page_Del(node); return;
		case TAG_SVG_flowRoot: SVG_flowRoot_Del(node); return;
		case TAG_SVG_flowRegion: SVG_flowRegion_Del(node); return;
		case TAG_SVG_flowPara: SVG_flowPara_Del(node); return;
		case TAG_SVG_flowSpan: SVG_flowSpan_Del(node); return;
		case TAG_SVG_handler: SVG_handler_Del(node); return;
		case TAG_SVG_prefetch: SVG_prefetch_Del(node); return;
		default: return;
	}
}

u32 SVG_GetAttributeCount(SFNode *node)
{
	switch (node->sgprivate->tag) {
		case TAG_SVG_svg: return 44;
		case TAG_SVG_g: return 33;
		case TAG_SVG_defs: return 31;
		case TAG_SVG_desc: return 4;
		case TAG_SVG_title: return 4;
		case TAG_SVG_metadata: return 4;
		case TAG_SVG_use: return 44;
		case TAG_SVG_switch: return 31;
		case TAG_SVG_image: return 40;
		case TAG_SVG_path: return 34;
		case TAG_SVG_rect: return 38;
		case TAG_SVG_circle: return 35;
		case TAG_SVG_line: return 36;
		case TAG_SVG_ellipse: return 36;
		case TAG_SVG_polyline: return 33;
		case TAG_SVG_polygon: return 33;
		case TAG_SVG_text: return 42;
		case TAG_SVG_tspan: return 32;
		case TAG_SVG_a: return 41;
		case TAG_SVG_script: return 6;
		case TAG_SVG_animate: return 36;
		case TAG_SVG_set: return 28;
		case TAG_SVG_animateMotion: return 38;
		case TAG_SVG_animateColor: return 36;
		case TAG_SVG_animateTransform: return 37;
		case TAG_SVG_mpath: return 12;
		case TAG_SVG_font: return 31;
		case TAG_SVG_font_face: return 37;
		case TAG_SVG_glyph: return 34;
		case TAG_SVG_missing_glyph: return 29;
		case TAG_SVG_hkern: return 9;
		case TAG_SVG_font_face_src: return 4;
		case TAG_SVG_font_face_uri: return 11;
		case TAG_SVG_font_face_name: return 5;
		case TAG_SVG_foreignObject: return 46;
		case TAG_SVG_linearGradient: return 14;
		case TAG_SVG_radialGradient: return 13;
		case TAG_SVG_stop: return 9;
		case TAG_SVG_solidColor: return 9;
		case TAG_SVG_audio: return 23;
		case TAG_SVG_video: return 45;
		case TAG_SVG_pageSet: return 24;
		case TAG_SVG_page: return 27;
		case TAG_SVG_flowRoot: return 26;
		case TAG_SVG_flowRegion: return 4;
		case TAG_SVG_flowPara: return 27;
		case TAG_SVG_flowSpan: return 26;
		case TAG_SVG_handler: return 6;
		case TAG_SVG_prefetch: return 15;
		default: return 0;
	}
}

M4Err SVG_GetAttributeInfo(SFNode *node, FieldInfo *info)
{
	switch (node->sgprivate->tag) {
		case TAG_SVG_svg: return SVG_svg_get_attribute(node, info);
		case TAG_SVG_g: return SVG_g_get_attribute(node, info);
		case TAG_SVG_defs: return SVG_defs_get_attribute(node, info);
		case TAG_SVG_desc: return SVG_desc_get_attribute(node, info);
		case TAG_SVG_title: return SVG_title_get_attribute(node, info);
		case TAG_SVG_metadata: return SVG_metadata_get_attribute(node, info);
		case TAG_SVG_use: return SVG_use_get_attribute(node, info);
		case TAG_SVG_switch: return SVG_switch_get_attribute(node, info);
		case TAG_SVG_image: return SVG_image_get_attribute(node, info);
		case TAG_SVG_path: return SVG_path_get_attribute(node, info);
		case TAG_SVG_rect: return SVG_rect_get_attribute(node, info);
		case TAG_SVG_circle: return SVG_circle_get_attribute(node, info);
		case TAG_SVG_line: return SVG_line_get_attribute(node, info);
		case TAG_SVG_ellipse: return SVG_ellipse_get_attribute(node, info);
		case TAG_SVG_polyline: return SVG_polyline_get_attribute(node, info);
		case TAG_SVG_polygon: return SVG_polygon_get_attribute(node, info);
		case TAG_SVG_text: return SVG_text_get_attribute(node, info);
		case TAG_SVG_tspan: return SVG_tspan_get_attribute(node, info);
		case TAG_SVG_a: return SVG_a_get_attribute(node, info);
		case TAG_SVG_script: return SVG_script_get_attribute(node, info);
		case TAG_SVG_animate: return SVG_animate_get_attribute(node, info);
		case TAG_SVG_set: return SVG_set_get_attribute(node, info);
		case TAG_SVG_animateMotion: return SVG_animateMotion_get_attribute(node, info);
		case TAG_SVG_animateColor: return SVG_animateColor_get_attribute(node, info);
		case TAG_SVG_animateTransform: return SVG_animateTransform_get_attribute(node, info);
		case TAG_SVG_mpath: return SVG_mpath_get_attribute(node, info);
		case TAG_SVG_font: return SVG_font_get_attribute(node, info);
		case TAG_SVG_font_face: return SVG_font_face_get_attribute(node, info);
		case TAG_SVG_glyph: return SVG_glyph_get_attribute(node, info);
		case TAG_SVG_missing_glyph: return SVG_missing_glyph_get_attribute(node, info);
		case TAG_SVG_hkern: return SVG_hkern_get_attribute(node, info);
		case TAG_SVG_font_face_src: return SVG_font_face_src_get_attribute(node, info);
		case TAG_SVG_font_face_uri: return SVG_font_face_uri_get_attribute(node, info);
		case TAG_SVG_font_face_name: return SVG_font_face_name_get_attribute(node, info);
		case TAG_SVG_foreignObject: return SVG_foreignObject_get_attribute(node, info);
		case TAG_SVG_linearGradient: return SVG_linearGradient_get_attribute(node, info);
		case TAG_SVG_radialGradient: return SVG_radialGradient_get_attribute(node, info);
		case TAG_SVG_stop: return SVG_stop_get_attribute(node, info);
		case TAG_SVG_solidColor: return SVG_solidColor_get_attribute(node, info);
		case TAG_SVG_audio: return SVG_audio_get_attribute(node, info);
		case TAG_SVG_video: return SVG_video_get_attribute(node, info);
		case TAG_SVG_pageSet: return SVG_pageSet_get_attribute(node, info);
		case TAG_SVG_page: return SVG_page_get_attribute(node, info);
		case TAG_SVG_flowRoot: return SVG_flowRoot_get_attribute(node, info);
		case TAG_SVG_flowRegion: return SVG_flowRegion_get_attribute(node, info);
		case TAG_SVG_flowPara: return SVG_flowPara_get_attribute(node, info);
		case TAG_SVG_flowSpan: return SVG_flowSpan_get_attribute(node, info);
		case TAG_SVG_handler: return SVG_handler_get_attribute(node, info);
		case TAG_SVG_prefetch: return SVG_prefetch_get_attribute(node, info);
		default: return M4BadParam;
	}
}

u32 SVG_GetTagByName(const char *element_name)
{
	if (!element_name) return TAG_UndefinedNode;
	if (!stricmp(element_name, "svg")) return TAG_SVG_svg;
	if (!stricmp(element_name, "g")) return TAG_SVG_g;
	if (!stricmp(element_name, "defs")) return TAG_SVG_defs;
	if (!stricmp(element_name, "desc")) return TAG_SVG_desc;
	if (!stricmp(element_name, "title")) return TAG_SVG_title;
	if (!stricmp(element_name, "metadata")) return TAG_SVG_metadata;
	if (!stricmp(element_name, "use")) return TAG_SVG_use;
	if (!stricmp(element_name, "switch")) return TAG_SVG_switch;
	if (!stricmp(element_name, "image")) return TAG_SVG_image;
	if (!stricmp(element_name, "path")) return TAG_SVG_path;
	if (!stricmp(element_name, "rect")) return TAG_SVG_rect;
	if (!stricmp(element_name, "circle")) return TAG_SVG_circle;
	if (!stricmp(element_name, "line")) return TAG_SVG_line;
	if (!stricmp(element_name, "ellipse")) return TAG_SVG_ellipse;
	if (!stricmp(element_name, "polyline")) return TAG_SVG_polyline;
	if (!stricmp(element_name, "polygon")) return TAG_SVG_polygon;
	if (!stricmp(element_name, "text")) return TAG_SVG_text;
	if (!stricmp(element_name, "tspan")) return TAG_SVG_tspan;
	if (!stricmp(element_name, "a")) return TAG_SVG_a;
	if (!stricmp(element_name, "script")) return TAG_SVG_script;
	if (!stricmp(element_name, "animate")) return TAG_SVG_animate;
	if (!stricmp(element_name, "set")) return TAG_SVG_set;
	if (!stricmp(element_name, "animateMotion")) return TAG_SVG_animateMotion;
	if (!stricmp(element_name, "animateColor")) return TAG_SVG_animateColor;
	if (!stricmp(element_name, "animateTransform")) return TAG_SVG_animateTransform;
	if (!stricmp(element_name, "mpath")) return TAG_SVG_mpath;
	if (!stricmp(element_name, "font")) return TAG_SVG_font;
	if (!stricmp(element_name, "font-face")) return TAG_SVG_font_face;
	if (!stricmp(element_name, "glyph")) return TAG_SVG_glyph;
	if (!stricmp(element_name, "missing-glyph")) return TAG_SVG_missing_glyph;
	if (!stricmp(element_name, "hkern")) return TAG_SVG_hkern;
	if (!stricmp(element_name, "font-face-src")) return TAG_SVG_font_face_src;
	if (!stricmp(element_name, "font-face-uri")) return TAG_SVG_font_face_uri;
	if (!stricmp(element_name, "font-face-name")) return TAG_SVG_font_face_name;
	if (!stricmp(element_name, "foreignObject")) return TAG_SVG_foreignObject;
	if (!stricmp(element_name, "linearGradient")) return TAG_SVG_linearGradient;
	if (!stricmp(element_name, "radialGradient")) return TAG_SVG_radialGradient;
	if (!stricmp(element_name, "stop")) return TAG_SVG_stop;
	if (!stricmp(element_name, "solidColor")) return TAG_SVG_solidColor;
	if (!stricmp(element_name, "audio")) return TAG_SVG_audio;
	if (!stricmp(element_name, "video")) return TAG_SVG_video;
	if (!stricmp(element_name, "pageSet")) return TAG_SVG_pageSet;
	if (!stricmp(element_name, "page")) return TAG_SVG_page;
	if (!stricmp(element_name, "flowRoot")) return TAG_SVG_flowRoot;
	if (!stricmp(element_name, "flowRegion")) return TAG_SVG_flowRegion;
	if (!stricmp(element_name, "flowPara")) return TAG_SVG_flowPara;
	if (!stricmp(element_name, "flowSpan")) return TAG_SVG_flowSpan;
	if (!stricmp(element_name, "handler")) return TAG_SVG_handler;
	if (!stricmp(element_name, "prefetch")) return TAG_SVG_prefetch;
	return TAG_UndefinedNode;
}

const char *SVG_GetElementName(u32 tag)
{
	switch(tag) {
	case TAG_SVG_svg: return "svg";
	case TAG_SVG_g: return "g";
	case TAG_SVG_defs: return "defs";
	case TAG_SVG_desc: return "desc";
	case TAG_SVG_title: return "title";
	case TAG_SVG_metadata: return "metadata";
	case TAG_SVG_use: return "use";
	case TAG_SVG_switch: return "switch";
	case TAG_SVG_image: return "image";
	case TAG_SVG_path: return "path";
	case TAG_SVG_rect: return "rect";
	case TAG_SVG_circle: return "circle";
	case TAG_SVG_line: return "line";
	case TAG_SVG_ellipse: return "ellipse";
	case TAG_SVG_polyline: return "polyline";
	case TAG_SVG_polygon: return "polygon";
	case TAG_SVG_text: return "text";
	case TAG_SVG_tspan: return "tspan";
	case TAG_SVG_a: return "a";
	case TAG_SVG_script: return "script";
	case TAG_SVG_animate: return "animate";
	case TAG_SVG_set: return "set";
	case TAG_SVG_animateMotion: return "animateMotion";
	case TAG_SVG_animateColor: return "animateColor";
	case TAG_SVG_animateTransform: return "animateTransform";
	case TAG_SVG_mpath: return "mpath";
	case TAG_SVG_font: return "font";
	case TAG_SVG_font_face: return "font-face";
	case TAG_SVG_glyph: return "glyph";
	case TAG_SVG_missing_glyph: return "missing-glyph";
	case TAG_SVG_hkern: return "hkern";
	case TAG_SVG_font_face_src: return "font-face-src";
	case TAG_SVG_font_face_uri: return "font-face-uri";
	case TAG_SVG_font_face_name: return "font-face-name";
	case TAG_SVG_foreignObject: return "foreignObject";
	case TAG_SVG_linearGradient: return "linearGradient";
	case TAG_SVG_radialGradient: return "radialGradient";
	case TAG_SVG_stop: return "stop";
	case TAG_SVG_solidColor: return "solidColor";
	case TAG_SVG_audio: return "audio";
	case TAG_SVG_video: return "video";
	case TAG_SVG_pageSet: return "pageSet";
	case TAG_SVG_page: return "page";
	case TAG_SVG_flowRoot: return "flowRoot";
	case TAG_SVG_flowRegion: return "flowRegion";
	case TAG_SVG_flowPara: return "flowPara";
	case TAG_SVG_flowSpan: return "flowSpan";
	case TAG_SVG_handler: return "handler";
	case TAG_SVG_prefetch: return "prefetch";
	default: return "UndefinedNode";
	}
}

#endif /*M4_DISABLE_SVG*/

