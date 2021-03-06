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
	DO NOT MOFIFY - File generated on GMT Fri Nov 12 16:25:28 2004

	BY SVGGen for GPAC Version 0.2.3-DEV
*/

#ifndef _M4_SVG_NODES_H
#define _M4_SVG_NODES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <m4_scenegraph_svg.h>


/* Definition of SVG element internal tags */
/* TAG names are made of "TAG_SVG" + SVG element name (with - replaced by _) */
enum {
	TAG_SVG_svg = TAG_RANGE_FIRST_SVG,
	TAG_SVG_g,
	TAG_SVG_defs,
	TAG_SVG_desc,
	TAG_SVG_title,
	TAG_SVG_metadata,
	TAG_SVG_use,
	TAG_SVG_switch,
	TAG_SVG_image,
	TAG_SVG_path,
	TAG_SVG_rect,
	TAG_SVG_circle,
	TAG_SVG_line,
	TAG_SVG_ellipse,
	TAG_SVG_polyline,
	TAG_SVG_polygon,
	TAG_SVG_text,
	TAG_SVG_tspan,
	TAG_SVG_a,
	TAG_SVG_script,
	TAG_SVG_animate,
	TAG_SVG_set,
	TAG_SVG_animateMotion,
	TAG_SVG_animateColor,
	TAG_SVG_animateTransform,
	TAG_SVG_mpath,
	TAG_SVG_font,
	TAG_SVG_font_face,
	TAG_SVG_glyph,
	TAG_SVG_missing_glyph,
	TAG_SVG_hkern,
	TAG_SVG_font_face_src,
	TAG_SVG_font_face_uri,
	TAG_SVG_font_face_name,
	TAG_SVG_foreignObject,
	TAG_SVG_linearGradient,
	TAG_SVG_radialGradient,
	TAG_SVG_stop,
	TAG_SVG_solidColor,
	TAG_SVG_audio,
	TAG_SVG_video,
	TAG_SVG_pageSet,
	TAG_SVG_page,
	TAG_SVG_flowRoot,
	TAG_SVG_flowRegion,
	TAG_SVG_flowPara,
	TAG_SVG_flowSpan,
	TAG_SVG_handler,
	TAG_SVG_prefetch,
	/*undefined elements (when parsing) use this tag*/
	TAG_SVG_UndefinedElement
};

/******************************************
*   SVG Elements structure definitions    *
*******************************************/
typedef struct _tagSVGsvgElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_SVG_overflow_values overflow; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Paint background_fill; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue background_fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate x; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate y; /* optional, animatable: no, inheritable: false */
	SVG_Length width; /* optional, animatable: no, inheritable: false */
	SVG_Length height; /* optional, animatable: no, inheritable: false */
	SVG_ViewBoxSpec viewBox; /* optional, animatable: no, inheritable: false */
	SVG_PreserveAspectRatioSpec preserveAspectRatio; /* optional, animatable: no, inheritable: false */
	SVG_String zoomAndPan; /* optional, animatable: no, inheritable: false */
	SVG_String version; /* optional, animatable: no, inheritable: false */
	SVG_Text baseProfile; /* optional, animatable: no, inheritable: false */
	SVG_ContentType contentScriptType; /* optional, animatable: no, inheritable: false */
} SVGsvgElement;


typedef struct _tagSVGgElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
	SVG_Boolean transform_host; /* optional, animatable: no, inheritable: false */
} SVGgElement;


typedef struct _tagSVGdefsElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
	SVG_Boolean transform_host; /* optional, animatable: no, inheritable: false */
} SVGdefsElement;


typedef struct _tagSVGdescElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
} SVGdescElement;


typedef struct _tagSVGtitleElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
} SVGtitleElement;


typedef struct _tagSVGmetadataElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
} SVGmetadataElement;


typedef struct _tagSVGuseElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Coordinate x; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate y; /* optional, animatable: no, inheritable: false */
	SVG_Length width; /* optional, animatable: no, inheritable: false */
	SVG_Length height; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* required, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
	SVG_Boolean transform_host; /* optional, animatable: no, inheritable: false */
} SVGuseElement;


typedef struct _tagSVGswitchElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
	SVG_Boolean transform_host; /* optional, animatable: no, inheritable: false */
} SVGswitchElement;


typedef struct _tagSVGimageElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SMIL_BeginOrEndValues begin; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues end; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate x; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate y; /* optional, animatable: no, inheritable: false */
	SVG_Length width; /* required, animatable: yes, inheritable: false */
	SVG_Length height; /* required, animatable: yes, inheritable: false */
	SVG_ContentType type; /* optional, animatable: no, inheritable: false */
	SVG_PreserveAspectRatioSpec preserveAspectRatio; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_SVG_overflow_values overflow; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* required, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_Paint background_fill; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue background_fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
} SVGimageElement;


typedef struct _tagSVGpathElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_PathData d; /* required, animatable: yes, inheritable: false */
	SVG_Number pathLength; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
} SVGpathElement;


typedef struct _tagSVGrectElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Coordinate x; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate y; /* optional, animatable: no, inheritable: false */
	SVG_Length width; /* required, animatable: yes, inheritable: false */
	SVG_Length height; /* required, animatable: yes, inheritable: false */
	SVG_Length rx; /* optional, animatable: no, inheritable: false */
	SVG_Length ry; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
} SVGrectElement;


typedef struct _tagSVGcircleElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Coordinate cx; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate cy; /* optional, animatable: no, inheritable: false */
	SVG_Length r; /* required, animatable: yes, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
} SVGcircleElement;


typedef struct _tagSVGlineElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Coordinate x1; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate y1; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate x2; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate y2; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
} SVGlineElement;


typedef struct _tagSVGellipseElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Coordinate cx; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate cy; /* optional, animatable: no, inheritable: false */
	SVG_Length rx; /* required, animatable: yes, inheritable: false */
	SVG_Length ry; /* required, animatable: yes, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
} SVGellipseElement;


typedef struct _tagSVGpolylineElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Points points; /* required, animatable: yes, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
} SVGpolylineElement;


typedef struct _tagSVGpolygonElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Points points; /* required, animatable: yes, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
} SVGpolygonElement;


typedef struct _tagSVGtextElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Coordinates x; /* optional, animatable: no, inheritable: false */
	SVG_Coordinates y; /* optional, animatable: no, inheritable: false */
	SVG_Numbers rotate; /* optional, animatable: no, inheritable: false */
	SVG_TextContent string; /* required, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Boolean editable; /* optional, animatable: no, inheritable: false */
	SVG_FontFamilyValue font_family; /* optional, animatable: no, inheritable: false */
	SVG_FontSizeValue font_size; /* optional, animatable: no, inheritable: false */
	SVG_FontStyleValue font_style; /* optional, animatable: no, inheritable: false */
	SVG_String font_weight; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
	SVG_Boolean transform_host; /* optional, animatable: no, inheritable: false */
} SVGtextElement;


typedef struct _tagSVGtspanElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_FontFamilyValue font_family; /* optional, animatable: no, inheritable: false */
	SVG_FontSizeValue font_size; /* optional, animatable: no, inheritable: false */
	SVG_FontStyleValue font_style; /* optional, animatable: no, inheritable: false */
	SVG_String font_weight; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
} SVGtspanElement;


typedef struct _tagSVGaElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_LinkTarget target; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* required, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
	SVG_Boolean transform_host; /* optional, animatable: no, inheritable: false */
} SVGaElement;


typedef struct _tagSVGscriptElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_ContentType type; /* required, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
} SVGscriptElement;


typedef struct _tagSVGanimateElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
	SMIL_AttributeName attributeName; /* required, animatable: no, inheritable: false */
	SVG_String attributeType; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues begin; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue dur; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues end; /* optional, animatable: no, inheritable: false */
	SMIL_RepeatCountValue repeatCount; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue repeatDur; /* optional, animatable: no, inheritable: false */
	SMIL_RestartValue restart; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue min; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue max; /* optional, animatable: no, inheritable: false */
	SMIL_FreezeValue freeze; /* optional, animatable: no, inheritable: false */
	SMIL_CalcModeValue calcMode; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValues values; /* optional, animatable: no, inheritable: false */
	SMIL_KeyTimesValues keyTimes; /* optional, animatable: no, inheritable: false */
	SMIL_KeySplinesValues keySplines; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValue from; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValue to; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValue by; /* optional, animatable: no, inheritable: false */
	SMIL_AdditiveValue additive; /* optional, animatable: no, inheritable: false */
	SMIL_AccumulateValue accumulate; /* optional, animatable: no, inheritable: false */
} SVGanimateElement;


typedef struct _tagSVGsetElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SMIL_AnimateValue to; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
	SMIL_AttributeName attributeName; /* required, animatable: no, inheritable: false */
	SVG_String attributeType; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues begin; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue dur; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues end; /* optional, animatable: no, inheritable: false */
	SMIL_RepeatCountValue repeatCount; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue repeatDur; /* optional, animatable: no, inheritable: false */
	SMIL_RestartValue restart; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue min; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue max; /* optional, animatable: no, inheritable: false */
	SMIL_FreezeValue freeze; /* optional, animatable: no, inheritable: false */
} SVGsetElement;


typedef struct _tagSVGanimateMotionElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SMIL_CalcModeValue calcMode; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValues values; /* optional, animatable: no, inheritable: false */
	SMIL_KeyTimesValues keyTimes; /* optional, animatable: no, inheritable: false */
	SMIL_KeySplinesValues keySplines; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValue from; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValue to; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValue by; /* optional, animatable: no, inheritable: false */
	SVG_String path; /* optional, animatable: no, inheritable: false */
	SVG_String keyPoints; /* optional, animatable: no, inheritable: false */
	SVG_String rotate; /* optional, animatable: no, inheritable: false */
	SVG_String origin; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues begin; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue dur; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues end; /* optional, animatable: no, inheritable: false */
	SMIL_RepeatCountValue repeatCount; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue repeatDur; /* optional, animatable: no, inheritable: false */
	SMIL_RestartValue restart; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue min; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue max; /* optional, animatable: no, inheritable: false */
	SMIL_FreezeValue freeze; /* optional, animatable: no, inheritable: false */
	SMIL_AdditiveValue additive; /* optional, animatable: no, inheritable: false */
	SMIL_AccumulateValue accumulate; /* optional, animatable: no, inheritable: false */
} SVGanimateMotionElement;


typedef struct _tagSVGanimateColorElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
	SMIL_AttributeName attributeName; /* required, animatable: no, inheritable: false */
	SVG_String attributeType; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues begin; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue dur; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues end; /* optional, animatable: no, inheritable: false */
	SMIL_RepeatCountValue repeatCount; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue repeatDur; /* optional, animatable: no, inheritable: false */
	SMIL_RestartValue restart; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue min; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue max; /* optional, animatable: no, inheritable: false */
	SMIL_FreezeValue freeze; /* optional, animatable: no, inheritable: false */
	SMIL_CalcModeValue calcMode; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValues values; /* optional, animatable: no, inheritable: false */
	SMIL_KeyTimesValues keyTimes; /* optional, animatable: no, inheritable: false */
	SMIL_KeySplinesValues keySplines; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValue from; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValue to; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValue by; /* optional, animatable: no, inheritable: false */
	SMIL_AdditiveValue additive; /* optional, animatable: no, inheritable: false */
	SMIL_AccumulateValue accumulate; /* optional, animatable: no, inheritable: false */
} SVGanimateColorElement;


typedef struct _tagSVGanimateTransformElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String type; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
	SMIL_AttributeName attributeName; /* required, animatable: no, inheritable: false */
	SVG_String attributeType; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues begin; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue dur; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues end; /* optional, animatable: no, inheritable: false */
	SMIL_RepeatCountValue repeatCount; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue repeatDur; /* optional, animatable: no, inheritable: false */
	SMIL_RestartValue restart; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue min; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue max; /* optional, animatable: no, inheritable: false */
	SMIL_FreezeValue freeze; /* optional, animatable: no, inheritable: false */
	SMIL_CalcModeValue calcMode; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValues values; /* optional, animatable: no, inheritable: false */
	SMIL_KeyTimesValues keyTimes; /* optional, animatable: no, inheritable: false */
	SMIL_KeySplinesValues keySplines; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValue from; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValue to; /* optional, animatable: no, inheritable: false */
	SMIL_AnimateValue by; /* optional, animatable: no, inheritable: false */
	SMIL_AdditiveValue additive; /* optional, animatable: no, inheritable: false */
	SMIL_AccumulateValue accumulate; /* optional, animatable: no, inheritable: false */
} SVGanimateTransformElement;


typedef struct _tagSVGmpathElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* required, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
} SVGmpathElement;


typedef struct _tagSVGfontElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Number horiz_origin_x; /* optional, animatable: no, inheritable: false */
	SVG_Number horiz_origin_y; /* optional, animatable: no, inheritable: false */
	SVG_Number horiz_adv_x; /* required, animatable: no, inheritable: false */
	SVG_Number vert_origin_x; /* optional, animatable: no, inheritable: false */
	SVG_Number vert_origin_y; /* optional, animatable: no, inheritable: false */
	SVG_Number vert_adv_y; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
} SVGfontElement;


typedef struct _tagSVGfont_faceElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String font_family; /* optional, animatable: no, inheritable: false */
	SVG_FontStyleValue font_style; /* optional, animatable: no, inheritable: false */
	SVG_String font_variant; /* optional, animatable: no, inheritable: false */
	SVG_String font_weight; /* optional, animatable: no, inheritable: false */
	SVG_String font_stretch; /* optional, animatable: no, inheritable: false */
	SVG_String font_size; /* optional, animatable: no, inheritable: false */
	SVG_String unicode_range; /* optional, animatable: no, inheritable: false */
	SVG_String panose_1; /* optional, animatable: no, inheritable: false */
	SVG_String widths; /* optional, animatable: no, inheritable: false */
	SVG_String bbox; /* optional, animatable: no, inheritable: false */
	SVG_Number units_per_em; /* optional, animatable: no, inheritable: false */
	SVG_Number stemv; /* optional, animatable: no, inheritable: false */
	SVG_Number stemh; /* optional, animatable: no, inheritable: false */
	SVG_Number slope; /* optional, animatable: no, inheritable: false */
	SVG_Number cap_height; /* optional, animatable: no, inheritable: false */
	SVG_Number x_height; /* optional, animatable: no, inheritable: false */
	SVG_Number accent_height; /* optional, animatable: no, inheritable: false */
	SVG_Number ascent; /* optional, animatable: no, inheritable: false */
	SVG_Number descent; /* optional, animatable: no, inheritable: false */
	SVG_Number ideographic; /* optional, animatable: no, inheritable: false */
	SVG_Number alphabetic; /* optional, animatable: no, inheritable: false */
	SVG_Number mathematical; /* optional, animatable: no, inheritable: false */
	SVG_Number hanging; /* optional, animatable: no, inheritable: false */
	SVG_Number v_ideographic; /* optional, animatable: no, inheritable: false */
	SVG_Number v_alphabetic; /* optional, animatable: no, inheritable: false */
	SVG_Number v_mathematical; /* optional, animatable: no, inheritable: false */
	SVG_Number v_hanging; /* optional, animatable: no, inheritable: false */
	SVG_Number underline_position; /* optional, animatable: no, inheritable: false */
	SVG_Number underline_thickness; /* optional, animatable: no, inheritable: false */
	SVG_Number strikethrough_position; /* optional, animatable: no, inheritable: false */
	SVG_Number strikethrough_thickness; /* optional, animatable: no, inheritable: false */
	SVG_Number overline_position; /* optional, animatable: no, inheritable: false */
	SVG_Number overline_thickness; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
} SVGfont_faceElement;


typedef struct _tagSVGglyphElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String unicode; /* optional, animatable: no, inheritable: false */
	SVG_String glyph_name; /* optional, animatable: no, inheritable: false */
	SVG_String orientation; /* optional, animatable: no, inheritable: false */
	SVG_String arabic_form; /* optional, animatable: no, inheritable: false */
	SVG_PathData d; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes lang; /* optional, animatable: no, inheritable: false */
	SVG_Number horiz_adv_x; /* optional, animatable: no, inheritable: false */
	SVG_Number vert_origin_x; /* optional, animatable: no, inheritable: false */
	SVG_Number vert_origin_y; /* optional, animatable: no, inheritable: false */
	SVG_Number vert_adv_y; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
} SVGglyphElement;


typedef struct _tagSVGmissing_glyphElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_PathData d; /* optional, animatable: no, inheritable: false */
	SVG_Number horiz_adv_x; /* optional, animatable: no, inheritable: false */
	SVG_Number vert_origin_x; /* optional, animatable: no, inheritable: false */
	SVG_Number vert_origin_y; /* optional, animatable: no, inheritable: false */
	SVG_Number vert_adv_y; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
} SVGmissing_glyphElement;


typedef struct _tagSVGhkernElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String u1; /* optional, animatable: no, inheritable: false */
	SVG_String g1; /* optional, animatable: no, inheritable: false */
	SVG_String u2; /* optional, animatable: no, inheritable: false */
	SVG_String g2; /* optional, animatable: no, inheritable: false */
	SVG_Number k; /* required, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
} SVGhkernElement;


typedef struct _tagSVGfont_face_srcElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
} SVGfont_face_srcElement;


typedef struct _tagSVGfont_face_uriElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* required, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
} SVGfont_face_uriElement;


typedef struct _tagSVGfont_face_nameElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String name; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
} SVGfont_face_nameElement;


typedef struct _tagSVGforeignObjectElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Coordinate x; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate y; /* optional, animatable: no, inheritable: false */
	SVG_Length width; /* required, animatable: yes, inheritable: false */
	SVG_Length height; /* required, animatable: yes, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SVG_Paint background_fill; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue background_fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* required, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
	SVG_Boolean transform_host; /* optional, animatable: no, inheritable: false */
} SVGforeignObjectElement;


typedef struct _tagSVGlinearGradientElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Coordinate x1; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate y1; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate x2; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate y2; /* optional, animatable: no, inheritable: false */
	SVG_String gradientUnits; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_SVGColor stop_color; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stop_opacity; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
} SVGlinearGradientElement;


typedef struct _tagSVGradialGradientElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Coordinate cx; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate cy; /* optional, animatable: no, inheritable: false */
	SVG_Length r; /* optional, animatable: no, inheritable: false */
	SVG_String gradientUnits; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_SVGColor stop_color; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stop_opacity; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
} SVGradialGradientElement;


typedef struct _tagSVGstopElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Number offset; /* required, animatable: yes, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_SVGColor stop_color; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stop_opacity; /* optional, animatable: no, inheritable: false */
} SVGstopElement;


typedef struct _tagSVGsolidColorElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_SVGColor solid_color; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue solid_opacity; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
} SVGsolidColorElement;


typedef struct _tagSVGaudioElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* required, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues begin; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue dur; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues end; /* optional, animatable: no, inheritable: false */
	SMIL_RepeatCountValue repeatCount; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue repeatDur; /* optional, animatable: no, inheritable: false */
	SMIL_RestartValue restart; /* optional, animatable: no, inheritable: false */
	SVG_String audio_level; /* optional, animatable: no, inheritable: false */
} SVGaudioElement;


typedef struct _tagSVGvideoElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_Coordinate x; /* optional, animatable: no, inheritable: false */
	SVG_Coordinate y; /* optional, animatable: no, inheritable: false */
	SVG_Length width; /* required, animatable: yes, inheritable: false */
	SVG_Length height; /* required, animatable: yes, inheritable: false */
	SVG_PreserveAspectRatioSpec preserveAspectRatio; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_FeatureList requiredFeatures; /* optional, animatable: no, inheritable: false */
	SVG_ExtensionList requiredExtensions; /* optional, animatable: no, inheritable: false */
	SVG_FormatList requiredFormats; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCodes systemLanguage; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* required, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
	SVG_Boolean externalResourcesRequired; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues begin; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue dur; /* optional, animatable: no, inheritable: false */
	SMIL_BeginOrEndValues end; /* optional, animatable: no, inheritable: false */
	SMIL_RepeatCountValue repeatCount; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue repeatDur; /* optional, animatable: no, inheritable: false */
	SMIL_RestartValue restart; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue min; /* optional, animatable: no, inheritable: false */
	SMIL_MinMaxDurRepeatDurValue max; /* optional, animatable: no, inheritable: false */
	SMIL_FreezeValue freeze; /* optional, animatable: no, inheritable: false */
	SVG_String audio_level; /* optional, animatable: no, inheritable: false */
	SVG_Paint background_fill; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue background_fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_TransformList transform; /* optional, animatable: no, inheritable: false */
} SVGvideoElement;


typedef struct _tagSVGpageSetElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
} SVGpageSetElement;


typedef struct _tagSVGpageElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String page_orientation; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
} SVGpageElement;


typedef struct _tagSVGflowRootElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
} SVGflowRootElement;


typedef struct _tagSVGflowRegionElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
} SVGflowRegionElement;


typedef struct _tagSVGflowParaElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
	SVG_Boolean editable; /* optional, animatable: no, inheritable: false */
} SVGflowParaElement;


typedef struct _tagSVGflowSpanElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_Paint fill; /* optional, animatable: no, inheritable: false */
	SVG_ClipFillRule fill_rule; /* optional, animatable: no, inheritable: false */
	SVG_Paint stroke; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashArrayValue stroke_dasharray; /* optional, animatable: no, inheritable: false */
	SVG_StrokeDashOffsetValue stroke_dashoffset; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineCapValue stroke_linecap; /* optional, animatable: no, inheritable: false */
	SVG_StrokeLineJoinValue stroke_linejoin; /* optional, animatable: no, inheritable: false */
	SVG_StrokeMiterLimitValue stroke_miterlimit; /* optional, animatable: no, inheritable: false */
	SVG_StrokeWidthValue stroke_width; /* optional, animatable: no, inheritable: false */
	SVG_String vector_effect; /* optional, animatable: no, inheritable: false */
	SVG_Color color; /* optional, animatable: no, inheritable: false */
	SVG_String color_rendering; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue fill_opacity; /* optional, animatable: no, inheritable: false */
	SVG_OpacityValue stroke_opacity; /* optional, animatable: no, inheritable: false */
	SVG_DisplayValue display; /* optional, animatable: no, inheritable: false */
	SVG_VisibilityValue visibility; /* optional, animatable: no, inheritable: false */
	SVG_String image_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String pointer_events; /* optional, animatable: no, inheritable: false */
	SVG_String shape_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String text_rendering; /* optional, animatable: no, inheritable: false */
	SVG_String focusable; /* optional, animatable: no, inheritable: false */
	SVG_String nav_index; /* optional, animatable: no, inheritable: false */
} SVGflowSpanElement;


typedef struct _tagSVGhandlerElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_String ev_event; /* required, animatable: no, inheritable: false */
	SVG_ContentType type; /* required, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
} SVGhandlerElement;


typedef struct _tagSVGprefetchElement
{
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
	SVG_NumberOrPercentage mediaSize; /* optional, animatable: no, inheritable: false */
	SVG_String mediaTime; /* optional, animatable: no, inheritable: false */
	SVG_String mediaEncodings; /* optional, animatable: no, inheritable: false */
	SVG_NumberOrPercentage bandwidth; /* optional, animatable: no, inheritable: false */
	SVG_String id; /* optional, animatable: no, inheritable: false */
	SVG_URI xml_base; /* optional, animatable: no, inheritable: false */
	SVG_LanguageCode xml_lang; /* optional, animatable: no, inheritable: false */
	SVG_String xml_space; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_type; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_role; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_arcrole; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_title; /* optional, animatable: no, inheritable: false */
	SVG_URI xlink_href; /* required, animatable: no, inheritable: false */
	SVG_String xlink_show; /* optional, animatable: no, inheritable: false */
	SVG_String xlink_actuate; /* optional, animatable: no, inheritable: false */
} SVGprefetchElement;


/******************************************
*  End SVG Elements structure definitions *
*******************************************/
#ifdef __cplusplus
}
#endif



#endif		/*_M4_SVG_NODES_H*/

