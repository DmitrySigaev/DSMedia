/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004
 *					All rights reserved
 *
 *  This file is part of GPAC / 3GPP/MPEG4 timed text plugin
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

#include <gpac/intern/m4_esm_dev.h>
#include <gpac/intern/m4_isomedia_dev.h>


/*
	this decoder is simply a scene decoder generating its own scene graph based on input data, 
	this scene graph is then used as an extra graph by the renderer, and manipulated by the decoder
	for any time animation handling.
	Translation from text to MPEG-4 scene graph:
		* all modifiers (styles, hilight, etc) are unrolled into chunks forming a unique, linear 
	sequence of text data (startChar, endChar) with associated styles & modifs
		* chunks are mapped to classic MPEG-4/VRML text
		* these chunks are then gathered in a Form node (supported by 2D and 3D renderers), with 
	text truncation at each newline char.
		* the Form then performs all alignment of the chunks

	It could be possible to use Layout instead of form, BUT layout cannot handle new lines at the time being...

	Currently supported for 3GP text streams:
		* text box positioning & filling, dynamic text box
		* text color
		* proper alignment (H and V) with horizontal text. Vertical text may not be properly layed out (not fully tested)
		* style Records (font, size, fontstyles, and colors change) - any nb per sample supported
		* hilighting (static only) with color or reverse video - any nb per sample supported
		* hypertext links - any nb per sample supported
		* blinking - any nb per sample supported
		* complete scrolling: in, out, in+out, scroll delay, up, down, right and left directions. All other
		modifiers are supported when scrolling

	It does NOT support:
		* dynamic hilighting (karaoke)
		* wrap

	The decoder only accepts complete timed text units TTU(1). In band reconfig (TTU(5) is not supported, 
	nor fragmented TTUs (2, 3, 4).
	UTF16 support should workbut MP4Box does not support it at encoding time.
*/

typedef struct 
{
	InlineScene *inlineScene;
	struct _m4_client *app;
	u32 PL, nb_streams;

	TextConfigDescriptor *cfg;

	LPSCENEGRAPH sg;

	/*avoid searching the graph for things we know...*/
	M_Transform2D *tr_track, *tr_box, *tr_scroll;
	M_Material2D *mat_track, *mat_box;
	M_Layer2D *dlist;
	M_Rectangle *rec_box;

	M_TimeSensor *ts_blink, *ts_scroll;
	M_ScalarInterpolator *process_blink, *process_scroll;
	LPROUTE time_route;
	Chain *blink_nodes;
	u32 scroll_type, scroll_mode;
	Float scroll_time;
	Bool is_active;
} TTDPriv;


static void ttd_set_blink_fraction(SFNode *node);
static void ttd_set_scroll_fraction(SFNode *node);
static void TTD_ResetDisplay(TTDPriv *priv);

/*the WORST thing about 3GP in MPEG4 is positioning of the text track...*/
static void TTD_UpdateSizeInfo(TTDPriv *priv)
{
	u32 w, h;
	Bool has_size;
	s32 offset, thw, thh, vw, vh;

	has_size = SG_GetSizeInfo(priv->inlineScene->graph, &w, &h);
	/*no size info is given in main scene, override by associated video size if any, or by text track size*/
	if (!has_size) {
		if (priv->cfg->has_vid_info && priv->cfg->video_width && priv->cfg->video_height) {
			SG_SetSizeInfo(priv->sg, priv->cfg->video_width, priv->cfg->video_height, 1);
		} else {
			SG_SetSizeInfo(priv->sg, priv->cfg->text_width, priv->cfg->text_height, 1);
		}
		SG_GetSizeInfo(priv->sg, &w, &h);
		if (!w || !h) return;
		IS_ForceSceneSize(priv->inlineScene, w, h);
	}

	if (!w || !h) return;
	/*apply*/
	SG_SetSizeInfo(priv->sg, w, h, 1);
	/*make sure the scene size is big enough to contain the text track after positioning. RESULTS ARE UNDEFINED
	if offsets are negative: since MPEG-4 uses centered coord system, we must assume video is aligned to top-left*/
	if (priv->cfg->has_vid_info) {
		vw = priv->cfg->horiz_offset; if (vw<0) vw = 0;
		vh = priv->cfg->vert_offset; if (vh<0) vh = 0;
		if ((priv->cfg->text_width + (u32) vw > w) || (priv->cfg->text_height + (u32) vh > h)) {
			w = priv->cfg->text_width+vw;
			h = priv->cfg->text_height+vh;
			SG_SetSizeInfo(priv->sg, w, h, 1);
			IS_ForceSceneSize(priv->inlineScene, w, h);
		}
	} else {
		/*otherwise override (mainly used for SRT & TTXT file direct loading*/
		priv->cfg->text_width = w;
		priv->cfg->text_height = h;
	}

	/*ok override video size with main scene size*/
	priv->cfg->video_width = w;
	priv->cfg->video_height = h;

	vw = (s32) w;
	vh = (s32) h;
	thw = priv->cfg->text_width / 2;
	thh = priv->cfg->text_height / 2;
	/*check translation, we must not get out of scene size - not supported in GPAC*/
	offset = priv->cfg->horiz_offset - vw/2 + thw;
	/*safety checks ?
	if (offset + thw < - vw/2) offset = - vw/2 + thw;
	else if (offset - thw > vw/2) offset = vw/2 - thw;
	*/
	priv->tr_track->translation.x = (Float) offset;
	
	offset = vh/2 - priv->cfg->vert_offset - thh;
	/*safety checks ?
	if (offset + thh > vh/2) offset = vh/2 - thh;
	else if (offset - thh < -vh/2) offset = -vh/2 + thh;
	*/
	priv->tr_track->translation.y = (Float) offset;

	SG_NodeChanged((SFNode *)priv->tr_track, NULL);
}

static M4Err TTD_GetCapabilities(BaseDecoder *plug, CapObject *capability)
{
	TTDPriv *priv = plug->privateStack;
	switch (capability->CapCode) {
	case CAP_WIDTH:
		capability->cap.valueINT = priv->cfg->text_width;
		return M4OK;
	case CAP_HEIGHT:
		capability->cap.valueINT = priv->cfg->text_height;
		return M4OK;
	case CAP_MEDIA_NOT_OVER:
		capability->cap.valueINT = priv->is_active;
		return M4OK;
	default:
		capability->cap.valueINT = 0;
		return M4OK;
	}
}

static M4Err TTD_SetCapabilities(BaseDecoder *plug, const CapObject capability)
{
	TTDPriv *priv = plug->privateStack;
	if (capability.CapCode==CAP_SHOW_EXTRASCENE) {
		if (capability.cap.valueINT) {
			TTD_ResetDisplay(priv);
			TTD_UpdateSizeInfo(priv);
			IS_RegisterExtraScene(priv->inlineScene, priv->sg, 0);
		} else {
			IS_RegisterExtraScene(priv->inlineScene, priv->sg, 1);
		}			
	}
	return M4OK;
}

M4Err TTD_AttachScene(SceneDecoder *plug, InlineScene *scene, Bool is_scene_decoder)
{
	TTDPriv *priv = plug->privateStack;
	if (priv->nb_streams) return M4BadParam;
	/*timedtext cannot be a root scene object*/
	if (is_scene_decoder) return M4BadParam;
	priv->inlineScene = scene;
	priv->app = scene->root_od->term;
	return M4OK;
}

M4Err TTD_ReleaseScene(SceneDecoder *plug)
{
	TTDPriv *priv = plug->privateStack;
	if (priv->nb_streams) return M4BadParam;
	return M4OK;
}

static M4INLINE void add_child(SFNode *n1, SFNode *par)
{
	ChainAddEntry(((SFParent *)par)->children, n1);
	Node_Register(n1, par);
}


static M4INLINE SFNode *ttd_create_node(TTDPriv *ttd, u32 tag, const char *def_name)
{
	SFNode *n = SG_NewNode(ttd->sg, tag);
	if (n) {
		if (def_name) Node_SetDEF(n, SG_GetNextAvailableNodeID(ttd->sg), def_name);
		Node_Init(n);
	}
	return n;
}

static M4Err TTD_AttachStream(BaseDecoder *plug, 
									 u16 ES_ID, 
									 unsigned char *decSpecInfo, 
									 u32 decSpecInfoSize, 
									 u16 DependsOnES_ID,
									 u32 objectTypeIndication, 
									 Bool Upstream)
{
	TTDPriv *priv = plug->privateStack;
	M4Err e;
	DefaultDescriptor dsi;
	SFNode *root, *n1, *n2;
	/*no scalable, no upstream*/
	if (priv->nb_streams || Upstream) return M4NotSupported;
	if (!decSpecInfo || !decSpecInfoSize) return M4NonCompliantBitStream;

	priv->cfg = (TextConfigDescriptor *) OD_NewDescriptor(TextConfig_Tag);
	dsi.tag = DecoderSpecificInfo_Tag;
	dsi.data = decSpecInfo;
	dsi.dataLength = decSpecInfoSize;
	e = OD_GetTextConfig(&dsi, (u8) objectTypeIndication, priv->cfg);
	if (e) {
		OD_DeleteDescriptor((Descriptor **) &priv->cfg);
		return e;
	}
	priv->nb_streams++;
	if (!priv->cfg->timescale) priv->cfg->timescale = 1000;

	priv->sg = SG_NewSubScene(priv->inlineScene->graph);

	root = ttd_create_node(priv, TAG_MPEG4_OrderedGroup, NULL);
	SG_SetRootNode(priv->sg, root);
	Node_Register(root, NULL);
	/*root transform*/
	priv->tr_track = (M_Transform2D *)ttd_create_node(priv, TAG_MPEG4_Transform2D, NULL);
	add_child((SFNode *) priv->tr_track, root);

	/*txt track background*/
	n1 = ttd_create_node(priv, TAG_MPEG4_Shape, NULL);
	add_child(n1, (SFNode *) priv->tr_track);
	((M_Shape *)n1)->appearance = ttd_create_node(priv, TAG_MPEG4_Appearance, NULL);
	Node_Register(((M_Shape *)n1)->appearance, n1);
	priv->mat_track = (M_Material2D *) ttd_create_node(priv, TAG_MPEG4_Material2D, NULL);
	priv->mat_track->filled = 1;
	priv->mat_track->transparency = 1;
	((M_Appearance *) ((M_Shape *)n1)->appearance)->material = (SFNode *) priv->mat_track;
	Node_Register((SFNode *) priv->mat_track, ((M_Shape *)n1)->appearance);
	n2 = ttd_create_node(priv, TAG_MPEG4_Rectangle, NULL);
	((M_Rectangle *)n2)->size.x = priv->cfg->text_width;
	((M_Rectangle *)n2)->size.y = priv->cfg->text_height;
	((M_Shape *)n1)->geometry = n2;
	Node_Register(n2, n1);

	/*txt box background*/
	priv->tr_box = (M_Transform2D *) ttd_create_node(priv, TAG_MPEG4_Transform2D, NULL);
	add_child((SFNode*) priv->tr_box, (SFNode*)priv->tr_track);
	n1 = ttd_create_node(priv, TAG_MPEG4_Shape, NULL);
	add_child(n1, (SFNode*)priv->tr_box);
	((M_Shape *)n1)->appearance = ttd_create_node(priv, TAG_MPEG4_Appearance, NULL);
	Node_Register(((M_Shape *)n1)->appearance, n1);
	priv->mat_box = (M_Material2D *) ttd_create_node(priv, TAG_MPEG4_Material2D, NULL);
	priv->mat_box->filled = 1;
	priv->mat_box->transparency = 1;
	((M_Appearance *) ((M_Shape *)n1)->appearance)->material = (SFNode *)priv->mat_box;
	Node_Register((SFNode *)priv->mat_box, ((M_Shape *)n1)->appearance);
	priv->rec_box = (M_Rectangle *) ttd_create_node(priv, TAG_MPEG4_Rectangle, NULL);
	priv->rec_box->size.x = priv->cfg->text_width;
	priv->rec_box->size.y = priv->cfg->text_height;
	((M_Shape *)n1)->geometry = (SFNode *) priv->rec_box;
	Node_Register((SFNode *) priv->rec_box, n1);

	priv->dlist = (M_Layer2D *) ttd_create_node(priv, TAG_MPEG4_Layer2D, NULL);
	priv->dlist->size.x = priv->cfg->text_width;
	priv->dlist->size.y = priv->cfg->text_height;
	add_child((SFNode *)priv->dlist, (SFNode *)priv->tr_box);

	priv->blink_nodes = NewChain();
	priv->ts_blink = (M_TimeSensor *) ttd_create_node(priv, TAG_MPEG4_TimeSensor, "TimerBlink");
	priv->ts_blink->cycleInterval = 0.25;
	priv->ts_blink->startTime = 0.0;
	priv->ts_blink->loop = 1;
	priv->process_blink = (M_ScalarInterpolator *) ttd_create_node(priv, TAG_MPEG4_ScalarInterpolator, NULL);
	/*override set_fraction*/
	priv->process_blink->on_set_fraction = ttd_set_blink_fraction;
	Node_SetPrivate((SFNode *) priv->process_blink, priv);
	/*route from fraction_changed to set_fraction*/
	SG_NewRoute(priv->sg, (SFNode *) priv->ts_blink, 6, (SFNode *) priv->process_blink, 0);
	
	priv->ts_scroll = (M_TimeSensor *) ttd_create_node(priv, TAG_MPEG4_TimeSensor, "TimerScroll");
	priv->ts_scroll->cycleInterval = 0;
	priv->ts_scroll->startTime = -1;
	priv->ts_scroll->loop = 0;
	priv->process_scroll = (M_ScalarInterpolator *) ttd_create_node(priv, TAG_MPEG4_ScalarInterpolator, NULL);
	/*override set_fraction*/
	priv->process_scroll->on_set_fraction = ttd_set_scroll_fraction;
	Node_SetPrivate((SFNode *) priv->process_scroll, priv);
	/*route from fraction_changed to set_fraction*/
	SG_NewRoute(priv->sg, (SFNode *) priv->ts_scroll, 6, (SFNode *) priv->process_scroll, 0);

	Node_Register((SFNode *) priv->ts_blink, NULL);
	Node_Register((SFNode *) priv->process_blink, NULL);
	Node_Register((SFNode *) priv->ts_scroll, NULL);
	Node_Register((SFNode *) priv->process_scroll, NULL);
	return e;
}

static M4Err TTD_DetachStream(BaseDecoder *plug, u16 ES_ID)
{
	TTDPriv *priv = plug->privateStack;
	if (!priv->nb_streams) return M4BadParam;

	Node_Unregister((SFNode *) priv->ts_blink, NULL);
	Node_Unregister((SFNode *) priv->process_blink, NULL);
	Node_Unregister((SFNode *) priv->ts_scroll, NULL);
	Node_Unregister((SFNode *) priv->process_scroll, NULL);

	SG_Delete(priv->sg);
	priv->sg = NULL;
	OD_DeleteDescriptor((Descriptor **) &priv->cfg);
	priv->nb_streams = 0;
	DeleteChain(priv->blink_nodes);
	return M4OK;
}

static void ttd_set_blink_fraction(SFNode *node)
{
	u32 i;
	TTDPriv *priv = (TTDPriv *)Node_GetPrivate(node);
	
	Bool blink_on = 1;
	if (priv->process_blink->set_fraction>0.5f) blink_on = 0;
	for (i=0; i<ChainGetCount(priv->blink_nodes); i++) {
		M_Material2D *m = ChainGetEntry(priv->blink_nodes, i);
		if (m->filled != blink_on) {
			m->filled = blink_on;
			SG_NodeChanged((SFNode *) m, NULL);
		}
	}
}

static void ttd_set_scroll_fraction(SFNode *node)
{
	Float frac;
	TTDPriv *priv = (TTDPriv *)Node_GetPrivate(node);
	
	frac = priv->process_scroll->set_fraction;
	if (frac==1.0f) priv->is_active = 0;
	if (!priv->tr_scroll) return;

	switch (priv->scroll_type - 1) {
	case STF_ScrollCredits:
	case STF_ScrollDown:
		priv->tr_scroll->translation.x = 0;
		if (priv->scroll_mode & STF_ScrollIn) {
			if (frac>priv->scroll_time) {
				priv->scroll_mode &= ~STF_ScrollIn;
				priv->tr_scroll->translation.y = 0;
			} else {
				priv->tr_scroll->translation.y = (frac/priv->scroll_time - 1) * priv->dlist->size.y;
			}
		} else if (priv->scroll_mode & STF_ScrollOut) {
			if (frac<1-priv->scroll_time) return;
			frac -= 1-priv->scroll_time;
			priv->tr_scroll->translation.y = (frac/priv->scroll_time) * priv->dlist->size.y;
		}
		if (priv->scroll_type - 1 == STF_ScrollDown) priv->tr_scroll->translation.y *= -1;
		break;
	case STF_ScrollMarquee:
	case STF_ScrollRight:
		priv->tr_scroll->translation.y = 0;
		if (priv->scroll_mode & STF_ScrollIn) {
			if (frac>priv->scroll_time) {
				priv->scroll_mode &= ~STF_ScrollIn;
				priv->tr_scroll->translation.x = 0;
			} else {
				priv->tr_scroll->translation.x = (frac/priv->scroll_time - 1) * priv->dlist->size.x;
			}
		} else if (priv->scroll_mode & STF_ScrollOut) {
			if (frac<1-priv->scroll_time) return;
			frac -= 1-priv->scroll_time;
			priv->tr_scroll->translation.x = (frac/priv->scroll_time) * priv->dlist->size.x;
		}
		if (priv->scroll_type - 1 == STF_ScrollMarquee) priv->tr_scroll->translation.x *= -1;
		break;
	default:
		break;
	}
	SG_NodeChanged((SFNode *)priv->tr_scroll, NULL);
}

static void TTD_ResetDisplay(TTDPriv *priv)
{
	ChainReset(priv->blink_nodes);
	while (ChainGetCount(priv->dlist->children)) {
		SFNode *n = ChainGetEntry(priv->dlist->children, 0);
		ChainDeleteEntry(priv->dlist->children, 0);
		Node_Unregister(n, (SFNode *)priv->dlist);
	}
	SG_NodeChanged((SFNode *) priv->dlist, NULL);
	priv->tr_scroll = NULL;
}

char *TTD_FindFont(TextSampleDescriptor *tsd, u32 ID)
{
	u32 i;
	for (i=0; i<tsd->font_count; i++) {
		if (tsd->fonts[i].fontID==ID) return tsd->fonts[i].fontName;
	}
	return "SERIF";
}

static void ttd_add_item(M_Form *form)
{
	s32 *new_gr;
	VRML_MF_Append(&form->groups, FT_MFInt32, (void **) &new_gr);
	(*new_gr) = ChainGetCount(form->children);
	VRML_MF_Append(&form->groups, FT_MFInt32, (void **) &new_gr);
	(*new_gr) = -1;
	/*store line info*/
	VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &new_gr);
	(*new_gr) = ChainGetCount(form->children);
}

static void ttd_add_line(M_Form *form)
{
	s32 *new_gr;
	VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &new_gr);
	(*new_gr) = -1;
}

typedef struct
{
	u32 start_char, end_char;
	StyleRecord *srec;
	Bool is_hilight;
	u32 hilight_col;	/*0 means RV*/
	TextHyperTextAtom *hlink;
	Bool has_blink;
	/*karaoke not done yet*/
	/*text wrapping is not supported - we will need to move to Layout (rather than form), and modify
	layout to handle new lines and proper scrolling*/
} TTDTextChunk;

void TTD_NewTextChunk(TTDPriv *priv, TextSampleDescriptor *tsd, M_Form *form, s16 *utf16_txt, TTDTextChunk *tc)
{
	SFNode *txt_model, *n2, *txt_material;
	M_Text *text;
	M_FontStyle *fs;
	char *fontName;
	char szStyle[1024];
	u32 fontSize, styleFlags, color, i, start_char;

	if (!tc->srec) {
		fontName = TTD_FindFont(tsd, tsd->default_style.fontID);
		fontSize = tsd->default_style.font_size;
		styleFlags = tsd->default_style.style_flags;
		color = tsd->default_style.text_color;
	} else {
		fontName = TTD_FindFont(tsd, tc->srec->fontID);
		fontSize = tc->srec->font_size;
		styleFlags = tc->srec->style_flags;
		color = tc->srec->text_color;
	}

	/*create base model for text node. It will then be cloned for each text item*/
	txt_model = ttd_create_node(priv, TAG_MPEG4_Shape, NULL);
	Node_Register(txt_model, NULL);
	n2 = ttd_create_node(priv, TAG_MPEG4_Appearance, NULL);
	((M_Shape *)txt_model)->appearance = n2;
	Node_Register(n2, txt_model);
	txt_material = ttd_create_node(priv, TAG_MPEG4_Material2D, NULL);
	((M_Appearance *)n2)->material = txt_material;
	Node_Register(txt_material, n2);

	((M_Material2D *)txt_material)->filled = 1;
	((M_Material2D *)txt_material)->transparency = 1.0f - ((Float) ( (color>>24) & 0xFF ) ) / 255.0f;;
	((M_Material2D *)txt_material)->emissiveColor.red = ((Float) ( (color>>16) & 0xFF) ) / 255.0f;
	((M_Material2D *)txt_material)->emissiveColor.green = ((Float) ( (color>>8) & 0xFF) ) / 255.0f;
	((M_Material2D *)txt_material)->emissiveColor.blue = ((Float) ( (color) & 0xFF) ) / 255.0f;
	/*force 0 lineWidth if blinking (the stupid MPEG-4 default values once again..)*/
	if (tc->has_blink) {
		((M_Material2D *)txt_material)->lineProps = ttd_create_node(priv, TAG_MPEG4_LineProperties, NULL);
		((M_LineProperties *)((M_Material2D *)txt_material)->lineProps)->width = 0;
		Node_Register(((M_Material2D *)txt_material)->lineProps, txt_material);
	}

	n2 = ttd_create_node(priv, TAG_MPEG4_Text, NULL);
	((M_Shape *)txt_model)->geometry = n2;
	Node_Register(n2, txt_model);
	text = (M_Text *) n2;
	fs = (M_FontStyle *) ttd_create_node(priv, TAG_MPEG4_FontStyle, NULL);
	free(fs->family.vals[0]);
	fs->family.vals[0] = strdup(fontName);
	fs->size = (Float) fontSize;
	free(fs->style.buffer);
	strcpy(szStyle, "");
	if (styleFlags & STF_Bold) {
		if (styleFlags & STF_Italic) strcpy(szStyle, "BOLDITALIC");
		else strcpy(szStyle, "BOLD");
	} else if (styleFlags & STF_Italic) strcat(szStyle, "ITALIC");
	if (!strlen(szStyle)) strcpy(szStyle, "PLAIN");
	/*also underline for URLs*/
	if ((styleFlags & STF_Underlined) || (tc->hlink && tc->hlink->URL)) strcat(szStyle, " UNDERLINED");

	if (tc->is_hilight) {
		if (tc->hilight_col) {
			char szTxt[50];
			sprintf(szTxt, " HIGHLIGHT#%x", tc->hilight_col);
			strcat(szStyle, szTxt);
		} else {
			strcat(szStyle, " HIGHLIGHT#RV");
		}
	}
	/*a better way would be to draw the entire text box in a composite texture & bitmap but we can't really rely 
	on text box size (in MP4Box, it actually defaults to the entire video area) and drawing a too large texture
	& bitmap could slow down rendering*/
	strcat(szStyle, " TEXTURED");

	fs->style.buffer = strdup(szStyle);
	fs->horizontal = (tsd->displayFlags & STF_VerticalText) ? 0 : 1;
	text->fontStyle = (SFNode *) fs;
	Node_Register((SFNode *)fs, (SFNode *)text);
	VRML_MF_Reset(&text->string, FT_MFString);

	if (tc->hlink && tc->hlink->URL) {
		SFURL *s;
		M_Anchor *anc = (M_Anchor *) ttd_create_node(priv, TAG_MPEG4_Anchor, NULL);
		VRML_MF_Append(&anc->url, FT_MFURL, (void **) &s);
		s->OD_ID = 0; 
		s->url = strdup(tc->hlink->URL);
		if (tc->hlink->URL_hint) anc->description.buffer = strdup(tc->hlink->URL_hint);
		ChainAddEntry(anc->children, txt_model);
		Node_Register(txt_model, (SFNode *)anc);
		txt_model = (SFNode *)anc;
		Node_Register((SFNode *)anc, NULL);
	}

	start_char = tc->start_char;
	for (i=tc->start_char; i<tc->end_char; i++) {
		Bool new_line = 0;
		if ((utf16_txt[i] == '\n') || (utf16_txt[i] == '\r') || (utf16_txt[i] == 0x85) || (utf16_txt[i] == 0x2028) || (utf16_txt[i] == 0x2029)) new_line = 1;
		
		if (new_line || (i+1==tc->end_char) ) {
			SFString *st;

			if (i+1==tc->end_char) i++;

			if (i!=start_char) {
				char szLine[5000];
				u32 len;
				s16 wsChunk[5000], *sp;

				/*spliting lines, duplicate node*/

				n2 = SG_CloneNode(priv->sg, txt_model, NULL);
				if (tc->hlink && tc->hlink->URL) {
					SFNode *t = ChainGetEntry(((M_Anchor *)n2)->children, 0);
					text = (M_Text *) ((M_Shape *)t)->geometry;
					txt_material = ((M_Appearance *) ((M_Shape *)t)->appearance)->material;
				} else {
					text = (M_Text *) ((M_Shape *)n2)->geometry;
					txt_material = ((M_Appearance *) ((M_Shape *)n2)->appearance)->material;
				}
				VRML_MF_Reset(&text->string, FT_MFString);
				ChainAddEntry(form->children, n2);
				Node_Register(n2, (SFNode *) form);
				ttd_add_item(form);
				/*clone node always register by default*/
				Node_Unregister(n2, NULL);
				
				if (tc->has_blink && txt_material) ChainAddEntry(priv->blink_nodes, txt_material);

				
				memcpy(wsChunk, &utf16_txt[start_char], sizeof(s16)*(i-start_char));
				wsChunk[i-start_char] = 0;
				sp = &wsChunk[0];
				len = utf8_wcstombs(szLine, 5000, (const unsigned short **) &sp);
				szLine[len] = 0;

				VRML_MF_Append(&text->string, FT_MFString, (void **) &st);
				st->buffer = strdup(szLine);
			}
			start_char = i+1;
			if (new_line) {
				ttd_add_line(form);
				if ((utf16_txt[i]=='\r') && (utf16_txt[i+1]=='\n')) i++;
			}
		}
	}
	Node_Unregister(txt_model, NULL);
	return;
}


/*mod can be any of TextHighlightAtom, TextKaraokeAtom, TextHyperTextAtom, TextBlinkAtom*/
void TTD_SplitChunks(TextSample *txt, u32 nb_chars, Chain *chunks, Atom *mod)
{
	u32 start_char, end_char;
	u32 i;
	switch (mod->type) {
	/*these 3 can be safelly typecasted to the same struct for start/end char*/
	case TextHighlightAtomType:
	case TextHyperTextAtomType:
	case TextBlinkAtomType:
		start_char = ((TextHighlightAtom *)mod)->startcharoffset;
		end_char = ((TextHighlightAtom *)mod)->endcharoffset;
		break;
	case TextKaraokeAtomType:
	default:
		return;
	}

	if (end_char>nb_chars) end_char = nb_chars;

	for (i=0; i<ChainGetCount(chunks); i++) {
		TTDTextChunk *tc = ChainGetEntry(chunks, i);
		if (tc->end_char<=start_char) continue;
		/*need to split chunk at begin*/
		if (tc->start_char<start_char) {
			TTDTextChunk *tc2;
			SAFEALLOC(tc2, sizeof(TTDTextChunk));
			memcpy(tc2, tc, sizeof(TTDTextChunk));
			tc2->start_char = start_char;
			tc2->end_char = tc->end_char;
			tc->end_char = start_char;
			ChainInsertEntry(chunks, tc2, i+1);
			i++;
			tc = tc2;
		}
		/*need to split chunks at end*/
		if (tc->end_char>end_char) {
			TTDTextChunk *tc2;
			SAFEALLOC(tc2, sizeof(TTDTextChunk));
			memcpy(tc2, tc, sizeof(TTDTextChunk));
			tc2->start_char = tc->start_char;
			tc2->end_char = end_char;
			tc->start_char = end_char;
			ChainInsertEntry(chunks, tc2, i);
			i++;
			tc = tc2;
		}
		/*assign mod*/
		switch (mod->type) {
		case TextHighlightAtomType:
			tc->is_hilight = 1;
			if (txt->highlight_color) tc->hilight_col = txt->highlight_color->hil_color;
			break;
		case TextHyperTextAtomType:
			tc->hlink = (TextHyperTextAtom *) mod;
			break;
		case TextBlinkAtomType:
			tc->has_blink = 1;
			break;
		}
		/*done*/
		if (tc->end_char==end_char) return;
	}
}


static void TTD_ApplySample(TTDPriv *priv, TextSample *txt, u32 sdi, Bool is_utf_16, u32 sample_duration)
{
	u32 i, nb_lines, start_idx;
	s32 *id, thw, thh, tw, th, offset;
	Bool vertical;
	MFInt32 idx;
	SFString *s;
	BoxRecord br;
	M_Material2D *n;
	M_Form *form;
	s16 utf16_text[5000];
	u32 char_offset, char_count;
	Chain *chunks;
	TTDTextChunk *tc;
	TextSampleDescriptor *td = NULL;
	
	TTD_ResetDisplay(priv);
	if (!sdi || !txt || !txt->len) return;

	for (i=0; i<ChainGetCount(priv->cfg->sample_descriptions); i++) {
		td = ChainGetEntry(priv->cfg->sample_descriptions, i);
		if (td->sample_index==sdi) break;
		td = NULL;
	}
	if (!td) return;
		
	vertical = (td->displayFlags & STF_VerticalText) ? 1 : 0;

	/*set back color*/
	/*do we fill the text box or the entire text track region*/
	if (td->displayFlags & STF_FillRegion) {
		priv->mat_box->transparency = 1.0f;
		n = priv->mat_track;
	} else {
		priv->mat_track->transparency = 1.0f;
		n = priv->mat_box;
	}
	n->transparency = 1.0f - ((Float) ( (td->back_color>>24) & 0xFF ) ) / 255.0f;
	n->emissiveColor.red = ((Float) ( (td->back_color>>16) & 0xFF) ) / 255.0f;
	n->emissiveColor.green = ((Float) ( (td->back_color>>8) & 0xFF) ) / 255.0f;
	n->emissiveColor.blue = ((Float) ( (td->back_color) & 0xFF) ) / 255.0f;
	SG_NodeChanged((SFNode *) n, NULL);
	
	if (txt->box) {
		br = txt->box->box;
	} else {
		br = td->default_pos;
	}
	if (!br.right || !br.bottom) {
		br.top = br.left = 0;
		br.right = priv->cfg->text_width;
		br.bottom = priv->cfg->text_height;
	}
	thw = br.right - br.left;
	thh = br.bottom - br.top;
	if (!thw || !thh) {
		br.top = br.left = 0;
		thw = priv->cfg->text_width;
		thh = priv->cfg->text_height;
	}

	priv->dlist->size.x = priv->rec_box->size.x = (Float) (thw);
	priv->dlist->size.y = priv->rec_box->size.y = (Float) (thh);
	SG_NodeChanged((SFNode *) priv->rec_box, NULL);

	form = (M_Form *) ttd_create_node(priv, TAG_MPEG4_Form, NULL);
	form->size.x = (Float) (thw);
	form->size.y = (Float) (thh);

	thw /= 2;
	thh /= 2;
	tw = priv->cfg->text_width;
	th = priv->cfg->text_height;

	/*check translation, we must not get out of scene size - not supported in GPAC*/
	offset = br.left - tw/2 + thw;
	if (offset + thw < - tw/2) offset = - tw/2 + thw;
	else if (offset - thw > tw/2) offset = tw/2 - thw;
	priv->tr_box->translation.x = (Float) offset;
	
	offset = th/2 - br.top - thh;
	if (offset + thh > th/2) offset = th/2 - thh;
	else if (offset - thh < -th/2) offset = -th/2 + thh;
	priv->tr_box->translation.y = (Float) offset;
	
	Node_SetDirty((SFNode *)priv->tr_box, 1);


	if (priv->scroll_type) {
		priv->ts_scroll->stopTime = Node_GetSceneTime((SFNode *) priv->ts_scroll);
		SG_NodeChanged((SFNode *) priv->ts_scroll, NULL);
	}
	priv->scroll_mode = 0;
	if (td->displayFlags & STF_ScrollIn) priv->scroll_mode |= STF_ScrollIn;
	if (td->displayFlags & STF_ScrollOut) priv->scroll_mode |= STF_ScrollOut;

	priv->scroll_type = 0;
	if (priv->scroll_mode) {
		priv->scroll_type = (td->displayFlags & STF_ScrollDirection)>>7;
		priv->scroll_type ++;
	}
	/*no sample duration, cannot determine scroll rate, so just show*/
	if (!sample_duration) priv->scroll_type = 0;
	/*no scroll*/
	if (!priv->scroll_mode) priv->scroll_type = 0;

	if (priv->scroll_type) {
		priv->tr_scroll = (M_Transform2D *) ttd_create_node(priv, TAG_MPEG4_Transform2D, NULL);
		ChainAddEntry(priv->dlist->children, priv->tr_scroll);
		Node_Register((SFNode *) priv->tr_scroll, (SFNode *) priv->dlist);
		ChainAddEntry(priv->tr_scroll->children, form);
		Node_Register((SFNode *) form, (SFNode *) priv->tr_scroll);
		priv->tr_scroll->translation.x = priv->tr_scroll->translation.y = (priv->scroll_mode & STF_ScrollIn) ? -1000.0f : 0;
		priv->scroll_time = 1.0f;
		if (txt->scroll_delay) priv->scroll_time = 1.0f - ((Float) txt->scroll_delay->scroll_delay) / priv->cfg->timescale;
		priv->scroll_time = 0.5f;
		if ((priv->scroll_mode & STF_ScrollIn) && (priv->scroll_mode & STF_ScrollOut)) priv->scroll_time /= 2;
	} else {
		ChainAddEntry(priv->dlist->children, form);
		Node_Register((SFNode *) form, (SFNode *) priv->dlist);
		priv->tr_scroll = NULL;
	}

	if (is_utf_16) {
		memcpy((char *) utf16_text, txt->text, sizeof(char) * txt->len);
		((char *) utf16_text)[txt->len] = 0;
		((char *) utf16_text)[txt->len+1] = 0;
		char_count = txt->len / 2;
	} else {
		char *p = txt->text;
		char_count = utf8_mbstowcs(utf16_text, 2500, (const char **) &p);
	}

	chunks = NewChain();
	/*flatten all modifiers*/
	if (!txt->styles || !txt->styles->entry_count) {
		SAFEALLOC(tc, sizeof(TTDTextChunk));
		tc->end_char = char_count;
		ChainAddEntry(chunks, tc);
	} else {
		StyleRecord *srec = NULL;
		char_offset = 0;
		for (i=0; i<txt->styles->entry_count; i++) {
			TTDTextChunk *tc;
			srec = &txt->styles->styles[i];
			/*handle not continuous modifiers*/
			if (char_offset < srec->startChar) {
				SAFEALLOC(tc, sizeof(TTDTextChunk));
				tc->start_char = char_offset;
				tc->end_char = srec->startChar;
				ChainAddEntry(chunks, tc);
			}
			SAFEALLOC(tc, sizeof(TTDTextChunk));
			tc->start_char = srec->startChar;
			tc->end_char = srec->endChar;
			tc->srec = srec;
			ChainAddEntry(chunks, tc);
			char_offset = srec->endChar;
		}

		if (srec->endChar<char_count) {
			SAFEALLOC(tc, sizeof(TTDTextChunk));
			tc->start_char = char_offset;
			tc->end_char = char_count;
			ChainAddEntry(chunks, tc);
		}
	}
	/*apply all other modifiers*/
	for (i=0; i<ChainGetCount(txt->others); i++) {
		Atom *a = ChainGetEntry(txt->others, i);
		TTD_SplitChunks(txt, char_count, chunks, a);
	}

	while (ChainGetCount(chunks)) {
		tc = ChainGetEntry(chunks, 0);
		ChainDeleteEntry(chunks, 0);
		TTD_NewTextChunk(priv, td, form, utf16_text, tc);
		free(tc);
	}
	DeleteChain(chunks);

	if (form->groupsIndex.vals[form->groupsIndex.count-1] != -1)
		ttd_add_line(form);

	/*rewrite form groupIndex - group is fine (eg one child per group)*/
	idx.count = form->groupsIndex.count;
	idx.vals = form->groupsIndex.vals;
	form->groupsIndex.vals = NULL;
	form->groupsIndex.count = 0;
	
	nb_lines = 0;
	start_idx = 0;
	for (i=0; i<idx.count; i++) {
		if (idx.vals[i] == -1) {
			s32 *id;
			u32 j;

			/*only one item in line, no need for alignment, but still add a group (we could use the
			item as a group but that would complicate the alignment generation)*/
			if (start_idx==i-1) {
				VRML_MF_Append(&form->groups, FT_MFInt32, (void **) &id); (*id) = idx.vals[start_idx];
				VRML_MF_Append(&form->groups, FT_MFInt32, (void **) &id); (*id) = -1;
			} else {
				/*spread horizontal 0 pixels (eg align) all items in line*/
				VRML_MF_Append(&form->constraints, FT_MFString, (void **) &s); s->buffer = strdup(vertical ? "SV 0" : "SH 0");
				for (j=start_idx; j<i; j++) {
					VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &id); (*id) = idx.vals[j];
					/*also add a group for the line, for final justif*/
					VRML_MF_Append(&form->groups, FT_MFInt32, (void **) &id); (*id) = idx.vals[j];
				}
				VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &id); (*id) = -1;
				/*mark end of group*/
				VRML_MF_Append(&form->groups, FT_MFInt32, (void **) &id); (*id) = -1;
			}
			start_idx = i+1;
			nb_lines ++;
		}
	}
	free(idx.vals);

	/*finally add constraints on lines*/
	start_idx = ChainGetCount(form->children) + 1;
	/*horizontal alignment*/
	VRML_MF_Append(&form->constraints, FT_MFString, (void **) &s);
	if (vertical) {
		switch (td->vert_justif) {
		case 1: s->buffer = strdup("AV"); break;/*center*/
		case -1: s->buffer = strdup("AB"); break;/*bottom*/
		default: s->buffer = strdup("AT"); break;/*top*/
		}
	} else {
		switch (td->horiz_justif) {
		case 1: s->buffer = strdup("AH"); break;/*center*/
		case -1: s->buffer = strdup("AR"); break;/*right*/
		default: s->buffer = strdup("AL"); break;/*left*/
		}
	}
	VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &id); (*id) = 0;
	for (i=0; i<nb_lines; i++) {
		VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &id); (*id) = i+start_idx;
	}
	VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &id); (*id) = -1;


	/*vertical alignment: first align all items vertically, 0 pixel */
	VRML_MF_Append(&form->constraints, FT_MFString, (void **) &s); s->buffer = strdup(vertical ? "SH 0" : "SV 0"); 
	VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &id); (*id) = 0;
	for (i=0; i<nb_lines; i++) {
		VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &id); (*id) = i+start_idx;
	}
	VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &id); (*id) = -1;

	/*define a group with every item drawn*/
	for (i=0; i<ChainGetCount(form->children); i++) {
		VRML_MF_Append(&form->groups, FT_MFInt32, (void **) &id); (*id) = i+1;
	}
	VRML_MF_Append(&form->groups, FT_MFInt32, (void **) &id); (*id) = -1;

	VRML_MF_Append(&form->constraints, FT_MFString, (void **) &s);
	if (vertical) {
		switch (td->horiz_justif) {
		case 1: s->buffer = strdup("AH"); break;/*center*/
		case -1: s->buffer = strdup("AR"); break;/*right*/
		default: s->buffer = strdup("AL"); break;/*left*/
		}
	} else {
		switch (td->vert_justif) {
		case 1: s->buffer = strdup("AV"); break;/*center*/
		case -1: s->buffer = strdup("AB"); break;/*bottom*/
		default: s->buffer = strdup("AT"); break;/*top*/
		}
	}
	VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &id); (*id) = 0;
	VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &id); (*id) = start_idx + nb_lines;
	VRML_MF_Append(&form->groupsIndex, FT_MFInt32, (void **) &id); (*id) = -1;


	Node_SetDirty((SFNode *)form, 1);
	SG_NodeChanged((SFNode *)form, NULL);
	SG_NodeChanged((SFNode *) priv->dlist, NULL);

	/*stop timer sensor*/
	priv->ts_blink->stopTime = Node_GetSceneTime((SFNode *) priv->ts_blink);
	SG_NodeChanged((SFNode *) priv->ts_blink, NULL);

	if (ChainGetCount(priv->blink_nodes)) {
		/*restart time sensor*/
		priv->ts_blink->startTime = Node_GetSceneTime((SFNode *) priv->ts_blink);
		SG_NodeChanged((SFNode *) priv->ts_blink, NULL);
	}

	priv->is_active = 1;
	priv->ts_scroll->stopTime = Node_GetSceneTime((SFNode *) priv->ts_scroll);
	SG_NodeChanged((SFNode *) priv->ts_scroll, NULL);
	/*scroll timer also acts as AU timer*/
	priv->ts_scroll->startTime = Node_GetSceneTime((SFNode *) priv->ts_scroll);
	priv->ts_scroll->cycleInterval = sample_duration;
	priv->ts_scroll->cycleInterval /= priv->cfg->timescale;
	priv->ts_scroll->cycleInterval -= 0.1;
	SG_NodeChanged((SFNode *) priv->ts_scroll, NULL);
}

static M4Err TTD_ProcessData(SceneDecoder*plug, unsigned char *inBuffer, u32 inBufferLength, 
								u16 ES_ID, u32 AU_time, u32 mmlevel)
{
	BitStream *bs;
	M4Err e = M4OK;
	TTDPriv *priv = plug->privateStack;

	bs = NewBitStream(inBuffer, inBufferLength, BS_READ);
	while (BS_Available(bs)) {
		TextSample *txt;
		Bool is_utf_16;
		u32 type, length, sample_index, sample_duration;
		is_utf_16 = BS_ReadInt(bs, 1);
		BS_ReadInt(bs, 4);
		type = BS_ReadInt(bs, 3);
		length = BS_ReadInt(bs, 16);
		
		/*currently only full text samples are supported*/
		if (type != 1) {
			DeleteBitStream(bs);
			return M4NotSupported;
		}
		sample_index = BS_ReadInt(bs, 8);
		/*duration*/
		sample_duration = BS_ReadInt(bs, 24);
		length -= 8;
		/*txt length is parsed with the sample*/
		txt = M4_ParseTextSample(bs);
		TTD_ApplySample(priv, txt, sample_index, is_utf_16, sample_duration);
		M4_DeleteTextSample(txt);
		/*since we support only TTU(1), no need to go on*/
		break;
	}
	DeleteBitStream(bs);
	return e;
}

Bool TTD_CanHandleStream(BaseDecoder *ifce, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL)
{
	TTDPriv *priv = ifce->privateStack;
	if (StreamType!=M4ST_TEXT) return 0;
	if (ObjectType!=0x08) return 0;
	priv->PL = PL;
	return 1;
}


void DeleteTimedTextDec(BaseDecoder *plug)
{
	TTDPriv *priv = plug->privateStack;
	/*in case something went wrong*/
	if (priv->cfg) OD_DeleteDescriptor((Descriptor **) &priv->cfg);
	free(priv);
	free(plug);
}

BaseDecoder *NewTimedTextDec()
{
	TTDPriv *priv;
	SceneDecoder *tmp;
	
	SAFEALLOC(tmp, sizeof(SceneDecoder));
	if (!tmp) return NULL;
	SAFEALLOC(priv, sizeof(TTDPriv));

	tmp->privateStack = priv;
	tmp->AttachStream = TTD_AttachStream;
	tmp->DetachStream = TTD_DetachStream;
	tmp->GetCapabilities = TTD_GetCapabilities;
	tmp->SetCapabilities = TTD_SetCapabilities;
	tmp->ProcessData = TTD_ProcessData;
	tmp->AttachScene = TTD_AttachScene;
	tmp->CanHandleStream = TTD_CanHandleStream;
	tmp->ReleaseScene = TTD_ReleaseScene;
	M4_REG_PLUG(tmp, M4SCENEDECODERINTERFACE, "GPAC TimedText Decoder", "gpac distribution", 0)
	return (BaseDecoder *) tmp;
}

Bool QueryInterface(u32 InterfaceType)
{
	switch (InterfaceType) {
	case M4SCENEDECODERINTERFACE: return 1;
	case M4STREAMINGCLIENT: return 1;
	default: return 0;
	}
}


#ifndef M4_READ_ONLY
void DeleteTTReader(void *ifce);
void *NewTTReader();
#endif

void *LoadInterface(u32 InterfaceType)
{
	switch (InterfaceType) {
	case M4SCENEDECODERINTERFACE: return NewTimedTextDec();
#ifndef M4_READ_ONLY
	case M4STREAMINGCLIENT: return NewTTReader();
#endif
	default: return NULL;
	}
}

void ShutdownInterface(void *ifce)
{
	BaseDecoder *ifcd = (BaseDecoder *)ifce;
	switch (ifcd->InterfaceType) {
	case M4SCENEDECODERINTERFACE:
		DeleteTimedTextDec(ifcd);
		break;
#ifndef M4_READ_ONLY
	case M4STREAMINGCLIENT:
		DeleteTTReader(ifcd);
		break;
#endif
	}
}
