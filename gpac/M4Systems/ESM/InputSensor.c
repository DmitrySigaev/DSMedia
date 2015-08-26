/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Stream Management sub-project
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


#include <gpac/m4_render.h>
#include <gpac/intern/m4_esm_dev.h>
#include <gpac/m4_x3d_nodes.h>

#include "InputSensor.h"

#if M4_HTK_DEMO

void HTK_Init(const char *htk_path);
void HTK_Close(void);
void HTK_SetDictionary(char *dicofile);
void HTK_DoDetection(void);
const char *HTK_GetWord();
int HTK_GetWordIndex();
float HTK_GetWordScore();
void StartHTK(ISPriv *is_dec);

static u32 htk_num_users = 0;
#endif


/*
				input sensor decoder(s) handling
*/

M4Err IS_Configure(BaseDecoder *plug, InlineScene *scene, Bool is_remote)
{
	ISPriv *is = plug->privateStack;
	/*we can only deal with encoded content (for now)*/
	if (!scene->scene_codec) return M4NotSupported;
	is->scene = scene;
	is->is_local = !is_remote;
	return M4OK;
}


static void add_field(ISPriv *priv, u32 fieldType, const char *fieldName)
{
	FieldInfo *field = (FieldInfo *) malloc(sizeof(FieldInfo));
	memset(field, 0, sizeof(FieldInfo));
	field->fieldType = fieldType;
	field->far_ptr = VRML_NewFieldPointer(fieldType);
	field->name = (const char *) fieldName;
	field->fieldIndex = ChainGetCount(priv->ddf);
	ChainAddEntry(priv->ddf, field);
}

static M4Err IS_AttachStream(BaseDecoder *plug, 
									 u16 ES_ID, 
									 unsigned char *decSpecInfo, 
									 u32 decSpecInfoSize, 
									 u16 DependsOnES_ID,
									 u32 objectTypeIndication, 
									 Bool Upstream)
{
	BitStream *bs;
	u32 len, size, i;
	char devName[255];
	s16 termSeq[20];

	ISPriv *is = plug->privateStack;
	if (Upstream) return M4NotSupported;
	if (!decSpecInfo) return M4NonCompliantBitStream;

	/*no more than one UI stream per object*/
	if (is->ES_ID) return M4NotSupported;
	is->ES_ID = ES_ID;
	/*parse config*/
	bs = NewBitStream(decSpecInfo, decSpecInfoSize, BS_READ);
	len = BS_ReadInt(bs, 8);
	for (i=0; i<len; i++) {
		devName[i] = BS_ReadInt(bs, 8);
	}
	devName[i] = 0;
	size = len + 1;

	is->type = 0;
	if (!stricmp(devName, "KeySensor")) {
		is->type = IS_KeySensor;
		add_field(is, FT_SFInt32, "keyPressed");
		add_field(is, FT_SFInt32, "keyReleased");
		add_field(is, FT_SFInt32, "actionKeyPressed");
		add_field(is, FT_SFInt32, "actionKeyReleased");
		add_field(is, FT_SFBool, "shiftKeyPressed");
		add_field(is, FT_SFBool, "controlKeyPressed");
		add_field(is, FT_SFBool, "altKeyPressed");

	} else if (!stricmp(devName, "StringSensor")) {
		is->type = IS_StringSensor;
		add_field(is, FT_SFString, "enteredText");
		add_field(is, FT_SFString, "finalText");

		is->termChar = '\r';
		is->delChar = '\b';

		/*get escape chars if any specified*/
		if (size<decSpecInfoSize) {
			const char *src = decSpecInfo + size;
			utf8_mbstowcs(termSeq, decSpecInfoSize - size, &src);
			is->termChar = termSeq[0];
			is->delChar = termSeq[1];
		}
	} else if (!stricmp(devName, "Mouse")) {
		is->type = IS_Mouse;
		add_field(is, FT_SFVec2f, "position");
		add_field(is, FT_SFBool, "leftButtonDown");
		add_field(is, FT_SFBool, "middleButtonDown");
		add_field(is, FT_SFBool, "rightButtonDown");
		add_field(is, FT_SFFloat, "wheel");

#if M4_HTK_DEMO
	} else if (!stricmp(devName, "HTKSensor")) {
		FILE *f;
		u32 nb_word, nbPhone, c, j;
		char szPh[3];
		char szName[1024];
		char *szPath = IF_GetKey(is->scene->root_od->term->user->config, "HTK", "HTKDirectory");
		if (!szPath) szPath = IF_GetKey(is->scene->root_od->term->user->config, "General", "PluginsDirectory");
		strcpy(is->szHTKPath, szPath);
		if (szPath[strlen(szPath)-1] != M4_PATH_SEPARATOR) is->szHTKPath[strlen(szPath)] = M4_PATH_SEPARATOR;

		add_field(is, FT_SFString, "word");
		add_field(is, FT_SFInt32, "wordIndex");
		add_field(is, FT_SFFloat, "wordScore");
	
		if (!htk_num_users) {
			fprintf(stdout, "loading HTK...");
			HTK_Init(is->szHTKPath);
			htk_num_users++;
			fprintf(stdout, "done\n");
		}
		
		sprintf(szName, "HTKD_%d", (u32) is);
		strcat(is->szHTKPath, szName);

		f = fopen(is->szHTKPath, "wt");
		szPh[2] = 0;
		nb_word = BS_ReadInt(bs, 8);
		for (i=0; i<nb_word; i++) {
			nbPhone = BS_ReadInt(bs, 8);
			while ((c=BS_ReadInt(bs, 8))) fprintf(f, "%c", c);
			fprintf(f, " ");
			for (j=0; j<nbPhone; j++) {
				BS_ReadData(bs, szPh, 2);
				if (j) fprintf(f, " ");
				if (!stricmp(szPh, "vc")) fprintf(f, "vcl");
				else fprintf(f, "%s", szPh);
			}
			fprintf(f, "\n");
		}
		fprintf(f, "RIEN sp\nSENT-END [] endsil\nSENT-START [] inisil\n");
		fclose(f);
		is->type = IS_HTKSensor;

		StartHTK(is);
#endif
	
	}
	DeleteBitStream(bs);
	return M4OK;
}

static M4Err IS_DetachStream(BaseDecoder *plug, u16 ES_ID)
{
	ISPriv *is = plug->privateStack;
	is->ES_ID = 0;
#if M4_HTK_DEMO
	if (htk_num_users) {
		htk_num_users--;
		if (!htk_num_users) {
			while (is->htk_running) Sleep(10);
			HTK_Close();
		}
	}
#endif
	return M4OK;
}

static M4Err IS_GetCapabilities(BaseDecoder *plug, CapObject *capability)
{
	capability->cap.valueINT = 0;
	return M4OK;
}

static M4Err IS_SetCapabilities(BaseDecoder *plug, const CapObject capability)
{
	return M4OK;
}

static M4Err IS_ProcessData(SceneDecoder *plug, unsigned char *inBuffer, u32 inBufferLength, 
								u16 ES_ID, u32 AU_time, u32 mmlevel)
{
	u32 i, j, count;
	Double scene_time;
	BitStream *bs;
	ISPriv *priv = plug->privateStack;
	M4Err e = M4OK;

	/*decode data frame except if local stringSensor*/
	bs = NewBitStream(inBuffer, inBufferLength, BS_READ);
	for (i=0; i<ChainGetCount(priv->ddf); i++) {
		FieldInfo *field = ChainGetEntry(priv->ddf, i);
		/*store present flag in eventIn for command skip - this is an ugly hack but it works since DDF don't have event types*/
		field->eventType = BS_ReadInt(bs, 1);
		/*parse val ourselves (we don't want to depend on bifs codec)*/
		if (field->eventType) {
			switch (field->fieldType) {
			case FT_SFBool: * ((SFBool *) field->far_ptr) = (SFBool) BS_ReadInt(bs, 1); break;
			case FT_SFFloat: *((SFFloat *)field->far_ptr) = BS_ReadFloat(bs); break;
			case FT_SFInt32: *((SFInt32 *)field->far_ptr) = (s32) BS_ReadInt(bs, 32); break;
			case FT_SFTime: *((SFTime *)field->far_ptr) = BS_ReadDouble(bs); break;
			case FT_SFVec2f:
				((SFVec2f *)field->far_ptr)->x = BS_ReadFloat(bs);
				((SFVec2f *)field->far_ptr)->y = BS_ReadFloat(bs);
				break;
			case FT_SFVec3f:
				((SFVec3f *)field->far_ptr)->x = BS_ReadFloat(bs);
				((SFVec3f *)field->far_ptr)->y = BS_ReadFloat(bs);
				((SFVec3f *)field->far_ptr)->z = BS_ReadFloat(bs);
				break;
			case FT_SFColor:
				((SFColor *)field->far_ptr)->red = BS_ReadFloat(bs);;
				((SFColor *)field->far_ptr)->green = BS_ReadFloat(bs);
				((SFColor *)field->far_ptr)->blue = BS_ReadFloat(bs);
				break;
			case FT_SFRotation:
				((SFRotation *)field->far_ptr)->xAxis = BS_ReadFloat(bs);
				((SFRotation *)field->far_ptr)->yAxis = BS_ReadFloat(bs);
				((SFRotation *)field->far_ptr)->zAxis = BS_ReadFloat(bs);
				((SFRotation *)field->far_ptr)->angle = BS_ReadFloat(bs);
				break;

			case FT_SFString:
			{
				u32 size, length;
				size = BS_ReadInt(bs, 5);
				length = BS_ReadInt(bs, size);
				if (BS_Available(bs) < length) return M4NonCompliantBitStream;

				if ( ((SFString *)field->far_ptr)->buffer ) free( ((SFString *)field->far_ptr)->buffer);
				((SFString *)field->far_ptr)->buffer = malloc(sizeof(char)*(length+1));
				memset(((SFString *)field->far_ptr)->buffer , 0, length+1);
				for (j=0; j<length; j++) {
					 ((SFString *)field->far_ptr)->buffer[j] = BS_ReadInt(bs, 8);
				}
			}
				break;
			}
		}
	}
	DeleteBitStream(bs);

	/*special case for StringSensor in local mode: lookup for special chars*/
	if ((priv->type == IS_StringSensor) && priv->is_local) {
		char tmp_utf8[5000];
		const unsigned short *ptr;
		u32 len;
		FieldInfo *field1 = ChainGetEntry(priv->ddf, 0);
		FieldInfo *field2 = ChainGetEntry(priv->ddf, 1);
		SFString *inText = (SFString *) field1->far_ptr;
		SFString *outText = (SFString *) field2->far_ptr;

		field1->eventType = field2->eventType = 0;
		priv->enteredText[priv->text_len] = (short) '\0';

		len = utf8_wcslen(priv->enteredText);
		if (len && (priv->enteredText[len-1] == priv->termChar)) {
			ptr = priv->enteredText;
			len = utf8_wcstombs(tmp_utf8, 5000, &ptr);
			if (outText->buffer) free(outText->buffer);
			outText->buffer = malloc(sizeof(char) * (len+1));
			memcpy(outText->buffer, tmp_utf8, sizeof(char) * len);
			outText->buffer[len] = 0;
			if (inText->buffer) free(inText->buffer);
			inText->buffer = NULL;
			priv->text_len = 0;

			field1->eventType = field2->eventType = 1;
		} else {
			if (priv->delChar) {
				/*remove chars*/
				if (len && (priv->enteredText[len-1] == priv->delChar)) {
					priv->enteredText[len-1] = (short) '\0';
					len--;
					if (len) {
						priv->enteredText[len-1] = (short) '\0';
						len--;
					}
				}
			}
			priv->text_len = len;
			ptr = priv->enteredText;
			len = utf8_wcstombs(tmp_utf8, 5000, &ptr);
			if (inText->buffer) free(inText->buffer);
			inText->buffer = malloc(sizeof(char) * (len+1));
			memcpy(inText->buffer, tmp_utf8, sizeof(char) * len);
			inText->buffer[len] = 0;
			field1->eventType = 1;
		}
	}

	/*apply it*/
	for (i=0; i<ChainGetCount(priv->is_nodes); i++) {
		ISStack *st = ChainGetEntry(priv->is_nodes, i);
		assert(st->is);
		assert(st->is_dec);
		if (!st->is->enabled) continue;

		count = ChainGetCount(st->is->buffer.commandList);
		scene_time = IS_GetSceneTime(priv->scene);
		for (j=0; j<count; j++) {
			SGCommand *com = ChainGetEntry(st->is->buffer.commandList, j);
			FieldInfo *field = ChainGetEntry(priv->ddf, j);
			CommandFieldInfo *info = ChainGetEntry(com->command_fields, 0);
			if (info && field && field->eventType) {
				VRML_FieldCopy(info->field_ptr, field->far_ptr, field->fieldType);
				SG_ApplyCommand(priv->scene->graph, com, scene_time);
			}
		}
	}
	return e;
}

void ISDec_Delete(BaseDecoder *plug)
{
	ISPriv *priv = plug->privateStack;
	DeleteChain(priv->is_nodes);

	while (ChainGetCount(priv->ddf)) {
		FieldInfo *fi = ChainGetEntry(priv->ddf, 0);
		ChainDeleteEntry(priv->ddf, 0);
		VRML_DeleteFieldPointer(fi->far_ptr, fi->fieldType);
		free(fi);
	}
	DeleteChain(priv->ddf);
#if M4_HTK_DEMO
	TH_Delete(priv->th);
#endif
	free(priv);
	free(plug);
}


BaseDecoder *NewISCodec(u32 PL)
{
	ISPriv *priv;
	SceneDecoder *tmp;
	
	tmp = (SceneDecoder*) malloc(sizeof(SceneDecoder));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(SceneDecoder));

	priv = (ISPriv *) malloc(sizeof(ISPriv));
	memset(priv, 0, sizeof(ISPriv));

	priv->is_nodes = NewChain();
	priv->ddf = NewChain();

	tmp->privateStack = priv;

	tmp->AttachStream = IS_AttachStream;
	tmp->DetachStream = IS_DetachStream;
	tmp->GetCapabilities = IS_GetCapabilities;
	tmp->SetCapabilities = IS_SetCapabilities;
	tmp->ProcessData = IS_ProcessData;
	/*we don't use this...*/
	tmp->AttachScene = NULL;

	M4_REG_PLUG(tmp, M4SCENEDECODERINTERFACE, "GPAC InputSensor Decoder", "gpac distribution", 0)

#if M4_HTK_DEMO
	priv->th = NewThread();
#endif
	return (BaseDecoder *) tmp;
}

/*
				input sensor node handling
*/
static void IS_Unregister(ISStack *st)
{
	u32 i;
	ODManager *odm;
	ISPriv *is_dec;
	odm = st->is_dec->odm;
	if (!odm) return;

	assert(odm->codec && (odm->codec->type == M4ST_INTERACT));

	/*get IS dec*/
	is_dec = odm->codec->decio->privateStack;
	for (i=0; i<ChainGetCount(is_dec->is_nodes); i++) {
		ISStack *tmp = ChainGetEntry(is_dec->is_nodes, i);
		if (tmp == st) {
			ChainDeleteEntry(is_dec->is_nodes, i);
			i--;
		}
	}
	/*stop stream*/
	MO_Stop(st->is_dec);
	st->is_dec = NULL;
	st->registered = 0;
}

static void IS_Register(ISStack *st)
{
	ODManager *odm;
	ISPriv *is_dec;
	odm = st->is_dec->odm;
	if (!odm) return;

	assert(odm->codec && (odm->codec->type == M4ST_INTERACT));

	/*get IS dec*/
	is_dec = odm->codec->decio->privateStack;
	ChainAddEntry(is_dec->is_nodes, st);
	st->registered = 1;
#if M4_HTK_DEMO
	StartHTK(is_dec);
#endif
	/*start stream*/
	MO_Play(st->is_dec);
}


static void RenderInputSensor(SFNode *node, void *rs)
{
	ISStack *st = Node_GetPrivate(node);
	M_InputSensor *is = (M_InputSensor *)node;

	/*get decoder object */
	if (!st->is_dec) st->is_dec = MO_FindObject(node, &is->url);
	/*register with decoder*/
	if (st->is_dec && !st->registered) IS_Register(st);
}

void DestroyInputSensor(SFNode *node)
{
	ISStack *st = Node_GetPrivate(node);
	if (st->registered) IS_Unregister(st);
	VRML_MF_Reset(&st->url, FT_MFURL);
	free(st);
}

void InitInputSensor(InlineScene *is, SFNode *node)
{
	ISStack *stack;
	stack = malloc(sizeof(ISStack));
	memset(stack, 0, sizeof(ISStack));
	stack->is = (M_InputSensor *) node;
	Node_SetPrivate(node, stack);
	Node_SetRenderFunction(node, RenderInputSensor);
	Node_SetPreDestroyFunction(node, DestroyInputSensor);
}

/*check only URL changes*/
void InputSensorModified(SFNode *node)
{
#if M4_HTK_DEMO
	ODManager *odm;
	ISPriv *is_dec;
#endif
	ISStack *st = Node_GetPrivate(node);
	if (!st->is_dec) return;

#if M4_HTK_DEMO
	/*turn audio analyse on/off*/
	if (!st->is_dec || !st->is_dec->od_man) return;
	odm = st->is_dec->od_man;
	assert(odm->codec && (odm->codec->type == M4ST_INTERACT));
	/*get IS dec*/
	is_dec = odm->codec->decio->privateStack;
	StartHTK(is_dec);
#endif

	if (! MO_UrlChanged(st->is_dec, &st->url)) return;
	/*unregister*/
	IS_Unregister(st);
}



/*
				input sensor DDF generations (user interface)
*/

void M4T_MouseInput(MPEG4CLIENT term, M4EventMouse *event)
{
	s32 X, Y;
	u32 left_but_down, middle_but_down, right_but_down;
	SFFloat wheel_pos;
	u32 i;
	BitStream *bs;
	SLHeader slh;
	unsigned char *buf;
	u32 buf_size;
	Float bX, bY;

	if (!term || !ChainGetCount(term->input_streams)) return;

	X = event->x;
	Y = event->y;
	left_but_down = middle_but_down = right_but_down = 0;
	wheel_pos = 0;
	switch (event->type) {
	case M4E_LEFTDOWN: left_but_down = 2; break;
	case M4E_LEFTUP: left_but_down = 1; break;
	case M4E_RIGHTDOWN: right_but_down = 2; break;
	case M4E_RIGHTUP: right_but_down = 1; break;
	case M4E_MIDDLEDOWN: middle_but_down = 2; break;
	case M4E_MIDDLEUP: middle_but_down = 1; break;
	case M4E_MOUSEWHEEL: wheel_pos = event->wheel_pos; break;
	case M4E_MOUSEMOVE: break;
	default: return;
	}

	/*get BIFS coordinates*/
	SR_MapCoordinates(term->renderer, X, Y, &bX, &bY);

	bs = NewBitStream(NULL, 0, BS_WRITE);

	/*If wheel is specified disable X and Y (bug from MS wheel handling)*/
	if (wheel_pos) {
		BS_WriteInt(bs, 0, 1);
	} else {
		BS_WriteInt(bs, 1, 1);
		BS_WriteFloat(bs, bX);
		BS_WriteFloat(bs, bY);
	}
	BS_WriteInt(bs, left_but_down ? 1 : 0, 1);
	if (left_but_down) BS_WriteInt(bs, left_but_down-1, 1);
	BS_WriteInt(bs, middle_but_down ? 1 : 0, 1);
	if (middle_but_down) BS_WriteInt(bs, middle_but_down-1, 1);
	BS_WriteInt(bs, right_but_down ? 1 : 0, 1);
	if (right_but_down) BS_WriteInt(bs, right_but_down-1, 1);
	if (wheel_pos==0) {
		BS_WriteInt(bs, 0, 1);
	} else {
		BS_WriteInt(bs, 1, 1);
		BS_WriteFloat(bs, wheel_pos);
	}

	BS_Align(bs);
	BS_GetContent(bs, &buf, &buf_size);
	DeleteBitStream(bs);

	memset(&slh, 0, sizeof(SLHeader));
	slh.accessUnitStartFlag = slh.accessUnitEndFlag = 1;
	slh.compositionTimeStampFlag = 1;
	/*note we could use an exact TS but it's not needed: since the input is generated locally
	we want it to be decoded as soon as possible, thus using 0 emulates permanent seeking on 
	InputSensor stream, hence forces input frame resync*/
	slh.compositionTimeStamp = 0;

	/*get all IS Mouse decoders and send frame*/
	for (i=0; i<ChainGetCount(term->input_streams); i++) {
		GenericCodec *cod = ChainGetEntry(term->input_streams, i);
		ISPriv *is = cod->decio->privateStack;
		if (is->type==IS_Mouse) {
			Channel *ch = ChainGetEntry(cod->inChannels, 0);
			Channel_RecieveSLP(ch->service, ch, buf, buf_size, &slh, M4OK);
		}
	}
	free(buf);
}

void M4T_KeyboardInput(MPEG4CLIENT term, s32 keyPressed, s32 keyReleased, s32 actionKeyPressed, s32 actionKeyReleased, u32 shiftKeyDown, u32 controlKeyDown, u32 altKeyDown)
{
	u32 i;
	BitStream *bs;
	SLHeader slh;
	unsigned char *buf;
	u32 buf_size;

	if (!term || (!ChainGetCount(term->input_streams) && !ChainGetCount(term->x3d_sensors)) ) return;

	memset(&slh, 0, sizeof(SLHeader));
	slh.accessUnitStartFlag = slh.accessUnitEndFlag = 1;
	slh.compositionTimeStampFlag = 1;
	/*cf above*/
	slh.compositionTimeStamp = 0;

	bs = NewBitStream(NULL, 0, BS_WRITE);

	BS_WriteInt(bs, keyPressed ? 1 : 0, 1); 
	if (keyPressed) BS_WriteInt(bs, keyPressed, 32);
	BS_WriteInt(bs, keyReleased ? 1 : 0, 1); 
	if (keyReleased) BS_WriteInt(bs, keyReleased, 32);
	BS_WriteInt(bs, actionKeyPressed ? 1 : 0, 1); 
	if (actionKeyPressed) BS_WriteInt(bs, actionKeyPressed, 32);
	BS_WriteInt(bs, actionKeyReleased ? 1 : 0, 1); 
	if (actionKeyReleased) BS_WriteInt(bs, actionKeyReleased, 32);
	BS_WriteInt(bs, shiftKeyDown ? 1 : 0 , 1); 
	if (shiftKeyDown) BS_WriteInt(bs, shiftKeyDown-1, 1);
	BS_WriteInt(bs, controlKeyDown ? 1 : 0 , 1); 
	if (controlKeyDown) BS_WriteInt(bs, controlKeyDown-1, 1);
	BS_WriteInt(bs, altKeyDown ? 1 : 0 , 1); 
	if (altKeyDown) BS_WriteInt(bs, altKeyDown, 1);

	BS_Align(bs);
	BS_GetContent(bs, &buf, &buf_size);
	DeleteBitStream(bs);

	/*get all IS keySensor decoders and send frame*/
	for (i=0; i<ChainGetCount(term->input_streams); i++) {
		GenericCodec *cod = ChainGetEntry(term->input_streams, i);
		ISPriv *is = cod->decio->privateStack;
		if (is->type==IS_KeySensor) {
			Channel *ch = ChainGetEntry(cod->inChannels, 0);
			Channel_RecieveSLP(ch->service, ch, buf, buf_size, &slh, M4OK);
		}
	}
	free(buf);
	
	for (i=0; i<ChainGetCount(term->x3d_sensors); i++) {
		u16 tc[2];
		u32 len;
		char szStr[10];
		const unsigned short *ptr;
		X_KeySensor *n = ChainGetEntry(term->x3d_sensors, i);
		if (Node_GetTag((SFNode *)n) != TAG_X3D_KeySensor) continue;
		if (!n->enabled) return;

		if (keyPressed) {
			if (n->keyPress.buffer) free(n->keyPress.buffer);
			tc[0] = keyPressed; tc[1] = 0;
			ptr = tc;
			len = utf8_wcstombs(szStr, 10, &ptr);
			n->keyPress.buffer = malloc(sizeof(char) * (len+1));
			memcpy(n->keyPress.buffer, szStr, sizeof(char) * len);
			n->keyPress.buffer[len] = 0;
			Node_OnEventOutSTR((SFNode *)n, "keyPress");
		}
		if (keyReleased) {
			if (n->keyRelease.buffer) free(n->keyRelease.buffer);
			tc[0] = keyReleased; tc[1] = 0;
			ptr = tc;
			len = utf8_wcstombs(szStr, 10, &ptr);
			n->keyRelease.buffer = malloc(sizeof(char) * (len+1));
			memcpy(n->keyRelease.buffer, szStr, sizeof(char) * len);
			n->keyRelease.buffer[len] = 0;
			Node_OnEventOutSTR((SFNode *)n, "keyRelease");
		}
		if (actionKeyPressed) {
			n->actionKeyPress = actionKeyPressed;
			Node_OnEventOutSTR((SFNode *)n, "actionKeyPress");
		}
		if (actionKeyReleased) {
			n->actionKeyRelease = actionKeyReleased;
			Node_OnEventOutSTR((SFNode *)n, "actionKeyRelease");
		}
		if (shiftKeyDown) {
			n->shiftKey = (shiftKeyDown-1) ? 1 : 0;
			Node_OnEventOutSTR((SFNode *)n, "shiftKey");
		}
		if (controlKeyDown) {
			n->controlKey = (controlKeyDown-1) ? 1 : 0;
			Node_OnEventOutSTR((SFNode *)n, "controlKey");
		}
		if (altKeyDown) {
			n->altKey= (altKeyDown-1) ? 1 : 0;
			Node_OnEventOutSTR((SFNode *)n, "altKey");
		}
		if (keyPressed || actionKeyPressed || (shiftKeyDown-1) || (controlKeyDown-1) || (altKeyDown-1)) {
			if (!n->isActive) {
				n->isActive = 1;
				Node_OnEventOutSTR((SFNode *)n, "isActive");
			}
		} else if (n->isActive) {
			n->isActive = 0;
			Node_OnEventOutSTR((SFNode *)n, "isActive");
		}
	}
}

void M4T_StringInput(MPEG4CLIENT term, u32 character)
{
	u32 i;
	BitStream *bs;
	SLHeader slh;
	unsigned char *buf;
	u32 buf_size;

	if (!character || !term) return;
	if (!ChainGetCount(term->input_streams) && !ChainGetCount(term->x3d_sensors)) return;

	memset(&slh, 0, sizeof(SLHeader));
	slh.accessUnitStartFlag = slh.accessUnitEndFlag = 1;
	slh.compositionTimeStampFlag = 1;
	/*cf above*/
	slh.compositionTimeStamp = 0;

	/*get all IS StringSensor decoders and send frame*/
	for (i=0; i<ChainGetCount(term->input_streams); i++) {
		GenericCodec *cod = ChainGetEntry(term->input_streams, i);
		ISPriv *is = cod->decio->privateStack;
		if (is->type==IS_StringSensor) {

			Channel *ch = ChainGetEntry(cod->inChannels, 0);
			is->enteredText[is->text_len] = character;
			is->text_len += 1;

			/*write empty DDF*/
			bs = NewBitStream(NULL, 0, BS_WRITE);
			BS_WriteInt(bs, 0, 1);
			BS_WriteInt(bs, 0, 1);
			BS_Align(bs);
			BS_GetContent(bs, &buf, &buf_size);
			DeleteBitStream(bs);
			
			Channel_RecieveSLP(ch->service, ch, buf, buf_size, &slh, M4OK);
			
			free(buf);
		}
	}


	/*get all X3D StringSensors*/
	for (i=0; i<ChainGetCount(term->x3d_sensors); i++) {
		StringSensorStack *st;
		char szStr[5000];
		const unsigned short *ptr;
		u32 len;
		X_StringSensor *n = ChainGetEntry(term->x3d_sensors, i);
		if (Node_GetTag((SFNode *)n) != TAG_X3D_StringSensor) continue;
		if (!n->enabled) continue;

		st = (StringSensorStack *) Node_GetPrivate((SFNode *)n);

		if (character=='\b') {
			if (n->deletionAllowed && st->text_len) {
				st->text_len -= 1;
				st->enteredText[st->text_len] = 0;
				ptr = st->enteredText;
				len = utf8_wcstombs(szStr, 10, &ptr);
				if (n->enteredText.buffer) free(n->enteredText.buffer);
				szStr[len] = 0;
				n->enteredText.buffer = strdup(szStr);
				Node_OnEventOutSTR((SFNode *)n, "enteredText");
			}
		} else if (character=='\r') {
			if (n->finalText.buffer) free(n->finalText.buffer);
			n->finalText.buffer = n->enteredText.buffer;
			n->enteredText.buffer = strdup("");
			st->text_len = 0;
			Node_OnEventOutSTR((SFNode *)n, "enteredText");
			Node_OnEventOutSTR((SFNode *)n, "finalText");
		} else {
			st->enteredText[st->text_len] = character;
			st->text_len += 1;
			st->enteredText[st->text_len] = 0;
			ptr = st->enteredText;
			len = utf8_wcstombs(szStr, 10, &ptr);
			if (n->enteredText.buffer) free(n->enteredText.buffer);
			szStr[len] = 0;
			n->enteredText.buffer = strdup(szStr);
			Node_OnEventOutSTR((SFNode *)n, "enteredText");
		}
	}
}

void DestroyKeySensor(SFNode *node)
{
	M4Client *term = (M4Client *) Node_GetPrivate(node);
	ChainDeleteItem(term->x3d_sensors, node);
}
void InitKeySensor(InlineScene *is, SFNode *node)
{
	Node_SetPrivate(node, is->root_od->term);
	Node_SetPreDestroyFunction(node, DestroyKeySensor);
	ChainAddEntry(is->root_od->term->x3d_sensors, node);
}

void DestroyStringSensor(SFNode *node)
{
	StringSensorStack *st = (StringSensorStack *) Node_GetPrivate(node);
	ChainDeleteItem(st->term->x3d_sensors, node);
	free(st);
}
void InitStringSensor(InlineScene *is, SFNode *node)
{
	StringSensorStack*st;
	SAFEALLOC(st, sizeof(StringSensorStack));
	st->term = is->root_od->term;
	Node_SetPrivate(node, st);
	Node_SetPreDestroyFunction(node, DestroyStringSensor);
	ChainAddEntry(is->root_od->term->x3d_sensors, node);
}

#if M4_HTK_DEMO
u32 RunHTKDec(void *par)
{
	BitStream *bs;
	char *szWord;
	s32 word_index;
	u32 len, val, i;
	Float word_score;
	SLHeader slh;
	unsigned char *buf;
	u32 buf_size;


	ISPriv *is_dec = (ISPriv *)par;
//	while (is_dec->htk_running)

	HTK_DoDetection();
	szWord = HTK_GetWord();
	word_index = HTK_GetWordIndex();
	word_score = HTK_GetWordScore();

	fprintf(stdout, "HTK result: %s %d %f\n", szWord, word_index, word_score);

	bs = NewBitStream(NULL, 0, BS_WRITE);
	
	/*HTK sensor buffer format: SFString - SFInt32 - SFFloat*/
	BS_WriteInt(bs, 1, 1); 
	len = strlen(szWord);
	val = GetNumBits(len);
	BS_WriteInt(bs, val, 5);
	BS_WriteInt(bs, len, val);
	for (i=0; i<len; i++) BS_WriteInt(bs, szWord[i], 8);

	BS_WriteInt(bs, 1, 1); 
	BS_WriteInt(bs, word_index, 32);
	BS_WriteInt(bs, 1, 1); 
	BS_WriteFloat(bs, word_score);

	BS_Align(bs);
	BS_GetContent(bs, &buf, &buf_size);
	DeleteBitStream(bs);

	memset(&slh, 0, sizeof(SLHeader));
	slh.accessUnitStartFlag = slh.accessUnitEndFlag = 1;
	slh.compositionTimeStamp = 0;

	/*get all IS keySensor decoders and send frame*/
	for (i=0; i<ChainGetCount(is_dec->scene->root_od->term->input_streams); i++) {
		GenericCodec *cod = ChainGetEntry(is_dec->scene->root_od->term->input_streams, i);
		ISPriv *is = cod->decio->privateStack;
		if (is != is_dec) continue;
		if (is->type==IS_HTKSensor) {
			Channel *ch = ChainGetEntry(cod->inChannels, 0);
			Channel_RecieveSLP(ch->service, ch, buf, buf_size, &slh, M4OK);
		}
	}
	free(buf);

	is_dec->htk_running = 0;
	return 0;
}

void StartHTK(ISPriv *is_dec)
{
	u32 j;
	Bool run;

	run = 0;
	for (j=0; j<ChainGetCount(is_dec->is_nodes); j++) {
		ISStack *st = ChainGetEntry(is_dec->is_nodes, j);
		if (st->is->enabled) {
			run = 1;
			break;
		}
	}
	if (is_dec->htk_running && run) return;
	if (!is_dec->htk_running && !run) return;
	
	fprintf(stdout, "Starting HTK recognition\n");

	is_dec->htk_running = run;
	if (run) {
		HTK_SetDictionary(is_dec->szHTKPath);
		TH_Run(is_dec->th, RunHTKDec, is_dec);
	}
}
#endif

