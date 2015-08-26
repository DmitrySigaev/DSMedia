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

#ifndef _M4_SG_SVG_H
#define _M4_SG_SVG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <m4_scenegraph.h>

#include <m4_graphics.h>

/* SVG attribute types */
enum {
	SVG_Unknown_datatype = 0,
	SVG_TextContent_datatype,
	SVG_Boolean_datatype,
	SVG_SVGColor_datatype,
	SVG_Paint_datatype,
	SVG_ClipFillRule_datatype,
	SVG_OpacityValue_datatype,
	SVG_StrokeWidthValue_datatype,
	SVG_StrokeLineJoinValue_datatype,
	SVG_StrokeLineCapValue_datatype,
	SVG_StrokeMiterLimitValue_datatype,
	SVG_TransformList_datatype,
	SVG_PathData_datatype,
	SVG_Points_datatype,
	SVG_FontFamilyValue_datatype,
	SVG_FontSizeValue_datatype,
	SVG_FontStyleValue_datatype,

	SMIL_CalcModeValue_datatype,
	SMIL_AdditiveValue_datatype,
	SMIL_AccumulateValue_datatype,
	SMIL_AnimateValue_datatype,
	SMIL_AnimateValues_datatype,
	SMIL_KeyTimesValues_datatype,
	SMIL_KeySplinesValues_datatype,
	SMIL_BeginOrEndValues_datatype,
	SMIL_MinMaxDurRepeatDurValue_datatype,
	SMIL_RepeatCountValue_datatype,
	SMIL_RestartValue_datatype,
	SMIL_AttributeName_datatype,
	SMIL_FreezeValue_datatype,

	SVG_FontVariantValue_datatype,
	SVG_FontWeightValue_datatype,
	SVG_DisplayValue_datatype, 
	SVG_VisibilityValue_datatype,

	SVG_FeatureList_datatype,
	SVG_ExtensionList_datatype,
	SVG_FormatList_datatype,
	SVG_String_datatype,
	SVG_URI_datatype,
	SVG_ContentType_datatype,
	SVG_LinkTarget_datatype,
	SVG_Text_datatype,
	SVG_LanguageCode_datatype,
	SVG_LanguageCodes_datatype,
	SVG_Number_datatype,
	SVG_Numbers_datatype,
	SVG_NumberOrPercentage_datatype,
	SVG_SVG_overflow_values_datatype,
	SVG_StrokeDashArrayValue_datatype,
	SVG_StrokeDashOffsetValue_datatype,
	SVG_Length_datatype,
	SVG_Coordinate_datatype,
	SVG_PreserveAspectRatioSpec_datatype,
	SVG_Coordinates_datatype,
	SVG_ViewBoxSpec_datatype,
	SVG_Color_datatype,

	SVG_VectorEffectAttrib_datatype,
	SVG_GradientOffset_datatype
};

/* 
	Reusing BIFS structure:
	fieldType: attribute data type
	far_ptr: same meaning
	NDTType: unused in SVG
	eventType: unused in SVG
	on_event_in: unused in SVG
	name: attribute name
	fieldIndex: index of the attribute
*/
typedef FieldInfo SVGAttributeInfo;

/* Definition of SVG base data types */
typedef struct {
	u32 length;
	u8 *string;
} DOM_String;
typedef DOM_String SVG_Text;
typedef DOM_String SVG_TextContent;
typedef DOM_String SVG_String;
typedef DOM_String SVG_ContentType;
typedef DOM_String SVG_LanguageCode;
typedef DOM_String SVG_LanguageCodes;
typedef DOM_String SVG_StrokeDashArrayValue;
typedef DOM_String SVG_StrokeDashOffsetValue;
typedef DOM_String SVG_Color; /* CSS Color definition ? */
typedef DOM_String SVG_Number;
typedef DOM_String SVG_Numbers;
typedef DOM_String SVG_NumberOrPercentage;
typedef DOM_String SVG_LinkTarget;
typedef DOM_String SVG_PreserveAspectRatioSpec;
typedef DOM_String SVG_FeatureList;
typedef DOM_String SVG_ExtensionList;
typedef DOM_String SVG_FormatList;

typedef DOM_String SVG_VectorEffectAttrib;
typedef DOM_String SVG_GradientOffset;

/* SMIL Anim types */
typedef FieldInfo SMIL_AttributeName;

enum {
	SMILBeginOrEnd_offset_value = 0,
	SMILBeginOrEnd_syncbase_value,
	SMILBeginOrEnd_event_value,
	SMILBeginOrEnd_repeat_value,
	SMILBeginOrEnd_accessKey_value,
	SMILBeginOrEnd_wallclock_sync_value,
	SMILBeginOrEnd_indefinite
};

typedef struct {
	u8 type;
	Double clock_value;
} SMIL_BeginOrEndValue;

typedef Chain * SMIL_BeginOrEndValues;

enum {
	SMILMinMaxDurRepeatDur_clock_indefinite_value = 0,
	SMILMinMaxDurRepeatDur_clock_value,
	SMILMinMaxDurRepeatDur_clock_media_value
};
typedef struct {
	u8 type;
	Double clock_value;
} SMIL_MinMaxDurRepeatDurValue;


enum {
	SMILRestart_always = 0,
	SMILRestart_whenNotActive,
	SMILRestart_never
};
typedef u8 SMIL_RestartValue;

enum {
	SMILFill_remove = 0,
	SMILFill_freeze
};
typedef u8 SMIL_FreezeValue;

typedef Float SMIL_RepeatCountValue;

typedef struct {
	u8 datatype;
	void *value;
} SMIL_AnimateValue;

typedef struct {
	u8 datatype;
	Chain *values;
} SMIL_AnimateValues;

typedef Chain * SMIL_KeyTimesValues;

/* Float between 0 and 1 */
typedef Chain * SMIL_KeySplinesValues;

enum {
	SMILAdditiveValue_replace = 0,
	SMILAdditiveValue_sum
}; 
typedef u8 SMIL_AdditiveValue;

enum {
	SMILAccumulateValue_none = 0,
	SMILAccumulateValue_sum
}; 
typedef u8 SMIL_AccumulateValue;

enum {
	SMILCalcMode_linear = 0,
	SMILCalcMode_discrete,
	SMILCalcMode_paced,
	SMILCalcMode_spline
};
typedef u8 SMIL_CalcModeValue;
/* end of SMIL Anim types */


enum {
	SVGUri_element_id,
	SVGUri_uri
};
typedef struct {
	u8 type;
	DOM_String uri;
	struct _svg_element *target_element;
} SVG_URI;

enum {
	SVGFontFamily_inherit,
	SVGFontFamily_string
};
typedef struct {
	u8 type;
	SVG_String value;
} SVG_FontFamilyValue;

enum {
	SVGFontStyle_normal,
	SVGFontStyle_italic,  
	SVGFontStyle_oblique,
	SVGFontStyle_inherit
}; 
typedef u8 SVG_FontStyleValue;

typedef struct {
	Chain *path_commands;
	Chain *path_points;
} SVG_PathData;

typedef struct {
	Float x, y;
} SVG_Point;
typedef Chain * SVG_Points;
typedef Chain * SVG_Coordinates;

typedef struct {
	Float x, y, width, height;
} SVG_Rect;

typedef SVG_Rect SVG_ViewBoxSpec;

typedef Bool SVG_Boolean;

enum {
	SVGFillRule_evenodd = M4PathFillOddEven,
	SVGFillRule_nonzero = M4PathFillZeroNonZero,
	SVGFillRule_inherit
};
typedef u8 SVG_ClipFillRule;
	
enum {
	SVGStrokeLineJoin_miter = M4LineJoinMiter,
	SVGStrokeLineJoin_round = M4LineJoinRound,
	SVGStrokeLineJoin_bevel = M4LineJoinBevel,
	SVGStrokeLineJoin_inherit = 100
};
typedef u8 SVG_StrokeLineJoinValue;

/* Warning: GPAC naming is not the same as SVG naming for line cap Flat = butt and Butt = square*/
enum {
	SVGStrokeLineCap_butt = M4LineCapFlat,
	SVGStrokeLineCap_round = M4LineCapRound,
	SVGStrokeLineCap_square = M4LineCapButt,
	SVGStrokeLineCap_inherit = 100
};
typedef u8 SVG_StrokeLineCapValue;

enum {
	SVGOverflow_inherit,
	SVGOverflow_visible,
	SVGOverflow_hidden,
	SVGOverflow_scroll,
	SVGOverflow_auto
};
typedef u8 SVG_SVG_overflow_values;

enum {
	SVG_COLORTYPE_UNKNOWN = 0,
	SVG_COLORTYPE_RGBCOLOR = 1,
	SVG_COLORTYPE_RGBCOLOR_ICCCOLOR = 2,
	SVG_COLORTYPE_CURRENTCOLOR = 3
};

typedef struct {
	u8 colorType;
	Float red, green, blue;
} SVG_SVGColor;

enum {
	SVG_PAINTTYPE_UNKNOWN = 0,
	SVG_PAINTTYPE_RGBCOLOR = 1,
	SVG_PAINTTYPE_RGBCOLOR_ICCCOLOR = 2,
	SVG_PAINTTYPE_INHERIT = 100,
	SVG_PAINTTYPE_NONE = 101,
	SVG_PAINTTYPE_CURRENTCOLOR = 102,
	SVG_PAINTTYPE_URI_NONE = 103,
	SVG_PAINTTYPE_URI_CURRENTCOLOR = 104,
	SVG_PAINTTYPE_URI_RGBCOLOR = 105,
	SVG_PAINTTYPE_URI_RGBCOLOR_ICCCOLOR = 106,
	SVG_PAINTTYPE_URI = 107
};

typedef struct {
	u8 paintType;
	SVG_SVGColor color;
	DOM_String uri;
} SVG_Paint;

enum {
	SVGFLOAT_INHERIT,
	SVGFLOAT_VALUE
};

typedef struct {
	u8 type;
	Float value;
} SVGInheritableFloat, SVG_OpacityValue, SVG_StrokeMiterLimitValue, SVG_FontSizeValue;

enum {
	SVG_LENGTHTYPE_UNKNOWN = 0,
	SVG_LENGTHTYPE_NUMBER = 1,
	SVG_LENGTHTYPE_PERCENTAGE = 2,
	SVG_LENGTHTYPE_EMS = 3,
	SVG_LENGTHTYPE_EXS = 4,
	SVG_LENGTHTYPE_PX = 5,
	SVG_LENGTHTYPE_CM = 6,
	SVG_LENGTHTYPE_MM = 7,
	SVG_LENGTHTYPE_IN = 8,
	SVG_LENGTHTYPE_PT = 9,
	SVG_LENGTHTYPE_PC = 10,
	SVG_LENGTHTYPE_INHERIT = 11
};

typedef struct {
	u8 unitType;
	Float number;
} SVG_Length, SVG_Coordinate, SVG_StrokeWidthValue;

typedef M4Matrix2D SVG_Matrix;

enum {
	SVG_TRANSFORM_UNKNOWN = 0,
	SVG_TRANSFORM_MATRIX = 1,
	SVG_TRANSFORM_TRANSLATE = 2,
	SVG_TRANSFORM_SCALE = 3,
	SVG_TRANSFORM_ROTATE = 4,
	SVG_TRANSFORM_SKEWX = 5,
	SVG_TRANSFORM_SKEWY = 6
};

typedef struct {
	u8 type;
	SVG_Matrix matrix;
	Float angle;
} SVG_Transform;

typedef Chain * SVG_TransformList;

enum {
	SVGFontVariant_normal,
	SVGFontVariant_small_caps
};
typedef u8 SVG_FontVariantValue;

enum {
	SVGFontWeight_normal,
	SVGFontWeight_bold, 
	SVGFontWeight_bolder, 
	SVGFontWeight_lighter, 
	SVGFontWeight_100, 
	SVGFontWeight_200,
	SVGFontWeight_300, 
	SVGFontWeight_400,
	SVGFontWeight_500,
	SVGFontWeight_600,
	SVGFontWeight_700,
	SVGFontWeight_800,
	SVGFontWeight_900
};
typedef u8 SVG_FontWeightValue;

enum {
	SVGVisibility_inherit,
	SVGVisibility_visible,
	SVGVisibility_hidden,
	SVGVisibility_collapse
};
typedef u8 SVG_VisibilityValue;

enum {
	SVGDisplay_inherit,
	SVGDisplay_none,
	SVGDisplay_inline,
	SVGDisplay_block,
	SVGDisplay_list_item,
	SVGDisplay_run_in,
	SVGDisplay_compact,
	SVGDisplay_marker,
	SVGDisplay_table,
	SVGDisplay_inline_table,
	SVGDisplay_table_row_group,
	SVGDisplay_table_header_group,
	SVGDisplay_table_footer_group,
	SVGDisplay_table_row,
	SVGDisplay_table_column_group,
	SVGDisplay_table_column,
	SVGDisplay_table_cell,
	SVGDisplay_table_caption
};
typedef u8 SVG_DisplayValue;

/**************************************************
 *  SVG's styling properties (see 6.1 in REC 1.1) *
 *************************************************/
/* general assumption: 
   if a property can be inherited then the value inherit 
   in the XML document is equivalent to no value specified.
   Thus the value inherit, if present in enumerations, is useless. */

typedef struct _svg_styling_properties {
	SVG_Paint *fill;
	SVG_ClipFillRule *fill_rule; 
	SVG_OpacityValue *fill_opacity;
	SVG_Paint *stroke;
	SVG_OpacityValue *stroke_opacity;
	SVG_StrokeWidthValue *stroke_width;
	SVG_StrokeLineCapValue *stroke_linecap; 
	SVG_StrokeLineJoinValue *stroke_linejoin; 
	SVG_StrokeMiterLimitValue *stroke_miterlimit; 
	SVG_FontSizeValue *font_size;
	SVG_FontStyleValue *font_style; 
	SVG_FontFamilyValue *font_family;

/*
	SVG_FontVariantValue *font_variant; 
	SVG_FontWeightValue *font_weight; 
	SVG_DisplayValue *display; 
	SVG_OpacityValue *opacity;
	SVG_VisibilityValue *visibility;
	u8 *overflow; 
	SVG_String *font;
	SVG_String *font_size_adjust;
	SVG_String *font_stretch;
	SVG_String *direction;
	SVG_String *letter_spacing;
	SVG_String *text_decoration;
	SVG_String *unicode_bidi;
	SVG_String *word_spacing;
	SVG_String *clip; 
	SVG_String *color;
	SVG_String *cursor;
	SVG_String *clip_path;
	SVG_String *clip_rule;
	SVG_String *mask;
	SVG_String *enable_background;
	SVG_String *filter;
	SVG_String *flood_color;
	SVG_String *flood_opacity;
	SVG_String *lighting_color;
	SVG_String *stop_color;
	SVG_String *stop_opacity;
	SVG_String *pointer_events;
	SVG_String *color_interpolation;
	SVG_String *color_interpolation_filters;
	SVG_String *color_profile;
	SVG_String *color_rendering;
	SVG_String *image_rendering;
	SVG_String *marker;
	SVG_String *marker_end;
	SVG_String *marker_mid;
	SVG_String *marker_start;
	SVG_String *shape_rendering;
	SVG_String *stroke_dasharray;
	SVG_String *stroke_dashoffset;
	SVG_String *text_rendering;
	SVG_String *alignment_baseline;
	SVG_String *baseline_shift;
	SVG_String *dominant_baseline;
	SVG_String *glyph_orientation_horizontal;
	SVG_String *glyph_orientation_vertical;
	SVG_String *kerning;
	SVG_String *text_anchor;
	SVG_String *writing_mode;
*/
} SVGStylingProperties;

#define BASE_SVG_ELEMENT \
	struct _tagSVGsvgElement *ownerSVGElement; \
	struct _svg_element *viewportElement; \
	SVGStylingProperties properties;

typedef struct _svg_element {
	BASE_NODE
	CHILDREN
	BASE_SVG_ELEMENT
} SVGElement;


/*
enum {
	SVG_ANGLETYPE_UNKNOWN = 0,
	SVG_ANGLETYPE_UNSPECIFIED = 1,
	SVG_ANGLETYPE_DEG = 2,
	SVG_ANGLETYPE_RAD = 3,
	SVG_ANGLETYPE_GRAD = 4
};

typedef struct {
	u8 angleType;
	Float angle;
} SVGAngle;

enum {
	SVG_UNIT_TYPE_UNKNOWN = 0,
	SVG_UNIT_TYPE_USERSPACEONUSE = 1,
	SVG_UNIT_TYPE_OBJECTBOUNDINGBOX = 2
};

enum {
	// Alignment Types
	SVG_PRESERVEASPECTRATIO_UNKNOWN = 0,
	SVG_PRESERVEASPECTRATIO_NONE = 1,
	SVG_PRESERVEASPECTRATIO_XMINYMIN = 2,
	SVG_PRESERVEASPECTRATIO_XMIDYMIN = 3,
	SVG_PRESERVEASPECTRATIO_XMAXYMIN = 4,
	SVG_PRESERVEASPECTRATIO_XMINYMID = 5,
	SVG_PRESERVEASPECTRATIO_XMIDYMID = 6,
	SVG_PRESERVEASPECTRATIO_XMAXYMID = 7,
	SVG_PRESERVEASPECTRATIO_XMINYMAX = 8,
	SVG_PRESERVEASPECTRATIO_XMIDYMAX = 9,
	SVG_PRESERVEASPECTRATIO_XMAXYMAX = 10,
};

enum {
	// Meet_or_slice Types
	SVG_MEETORSLICE_UNKNOWN = 0,
	SVG_MEETORSLICE_MEET = 1,
	SVG_MEETORSLICE_SLICE = 2
};

typedef struct {
	u16 align;
	u16 meetOrSlice;
} SVGPreserveAspectRatio; SVG_PreserveAspectRatioSpec_datatype

enum {
	SVG_ZOOMANDPAN_UNKNOWN = 0,
	SVG_ZOOMANDPAN_DISABLE = 1,
	SVG_ZOOMANDPAN_MAGNIFY = 2
};

u8 zoomAndPan;

enum {
	LENGTHADJUST_UNKNOWN   = 0,
	LENGTHADJUST_SPACING     = 1,
	LENGTHADJUST_SPACINGANDGLYPHS     = 2
};

enum {
    // textPath Method Types
	TEXTPATH_METHODTYPE_UNKNOWN   = 0,
	TEXTPATH_METHODTYPE_ALIGN     = 1,
	TEXTPATH_METHODTYPE_STRETCH     = 2
};
enum {
    // textPath Spacing Types
	TEXTPATH_SPACINGTYPE_UNKNOWN   = 0,
	TEXTPATH_SPACINGTYPE_AUTO     = 1,
	TEXTPATH_SPACINGTYPE_EXACT     = 2
};

enum {
    // Marker Unit Types
	SVG_MARKERUNITS_UNKNOWN        = 0,
	SVG_MARKERUNITS_USERSPACEONUSE = 1,
	SVG_MARKERUNITS_STROKEWIDTH    = 2
};
enum {
    // Marker Orientation Types
	SVG_MARKER_ORIENT_UNKNOWN      = 0,
	SVG_MARKER_ORIENT_AUTO         = 1,
	SVG_MARKER_ORIENT_ANGLE        = 2
};

enum {
    // Spread Method Types
	SVG_SPREADMETHOD_UNKNOWN = 0,
	SVG_SPREADMETHOD_PAD     = 1,
	SVG_SPREADMETHOD_REFLECT = 2,
	SVG_SPREADMETHOD_REPEAT  = 3
};

*/

/*shortcut to SG_NewNode + Node_Init*/
SVGElement *SVG_NewNode(LPSCENEGRAPH inScene, u32 tag);
void SVG_DelPathData(SVG_PathData *);


/*the exported functions used by the scene graph*/
u32 SVG_GetTagByName(const char *element_name);
u32 SVG_GetAttributeCount(SFNode *);
M4Err SVG_GetAttributeInfo(SFNode *node, FieldInfo *info);
SVGElement *SVG_CreateNode(u32 ElementTag);
const char *SVG_GetElementName(u32 tag);
void SVGElement_Del(SVGElement *elt);

#ifdef __cplusplus
}
#endif

#endif	//_M4_SG_SVG_H

