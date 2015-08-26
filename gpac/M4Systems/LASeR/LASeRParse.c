#include "m4_laser_dev.h"

#ifdef M4_USE_LASeR

#include <gpac/intern/m4_scenegraph_dev.h>

static Float normalizer[] = {1.0f, 1.0f, 3.0f, 7.0f, 15.0f, 31.0f, 63.0f, 127.0f, 255.0f, 511.0f, 1023.0f, 2047.0f};

static s32 scaleTab[] = {2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576,
                         2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728};

static s32 tab1[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768,
			   		 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216};
static s32 tab[] = {0, 0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767,
					65535, 131071, 262143, 524287, 1048575, 2097151, 4194303, 8388607};

/*
 Translates n read from the stream to a Float value.
 The stream contains a 12-bits, 2's complement integer. 
 When read to an int, the first operation is to restore the sign. (if ...)
 Then the correction for the resolution is applied. 
*/
Float LASeR_s12ToFloat(LPLASERDEC codec, u32 u)
{
	s32 n = u;
    if (n > 2047) n -= 4096;
    return n/codec->config->resolution;
}

/* Translates n read from the stream to a Float value, when the number is a scale factor.
  The stream contains a 12-bits, 2's complement integer. 
  When read to an u32, the first operation is to restore the sign. 
  Then the correction for the resolution is applied. 
  For a scale factor, the resolution is constant and equal to 1/256.
*/
Float LASeR_scaleToFloat(LPLASERDEC codec, u32 u) {
	s32 n = u;
	if (n >= scaleTab[codec->config->scaleBits]) n -= scaleTab[codec->config->scaleBits + 1];
	return n/256.0f;
}

/* Translates n read from the stream to a Float value, when the type of the number is not known, ie
 * in a replace field update.
 * The stream contains a 9 to 24 bits 2's complement integer (n).
 * When n is read to an u32, the first operation is to restore the sign. 
 * Then the correction for the resolution is applied. 
 * For a generic number, the resolution is constant and equal to 1/256.
*/
Float LASeR_genericNbBitsToFloat(LPLASERDEC codec, u32 u, u32 nbBits) {
    s32 n = u;
	if (n > tab[nbBits]) n -= tab1[nbBits];
    return n/256.0f;
}

/* Translates n read from the stream to a Float value.
 The stream contains a "bits"-bits, 2's complement integer. 
 When read to an u32, the first operation is to restore the sign. 
 If n is over 2^(bits-1)-1, it is translated to 2^(bits)-n.
 Then the correction for the resolution is applied. 
*/
Float LASeR_sNbBitsToFloat(LPLASERDEC codec, u32 u, u32 bits)
{
	s32 n = u;
	if (n > tab[bits]) n -= tab1[bits];
    return n / codec->config->resolution;
}

static void *LASeR_ParseNode(LPLASERDEC codec, BitStream *bs, LASeRParent *parent);
static void LASeR_ParseUpdates(LPLASERDEC codec, BitStream *bs, void *conditional);

static void LASeR_ParseMatrix(LPLASERDEC codec, BitStream *bs, LASeRMatrix *m) 
{
	u8 xx_yy_present, xy_yx_present, xz_zy_present;

	m->xx = m->yy = 1;
	m->xy = m->yx = m->xz = m->yz = 0;

	xx_yy_present = BS_ReadBit(bs);
	if (xx_yy_present) {
		m->xx = LASeR_scaleToFloat(codec, BS_ReadInt(bs, codec->config->scaleBits+12));
		m->yy = LASeR_scaleToFloat(codec, BS_ReadInt(bs, codec->config->scaleBits+12));
	}
	xy_yx_present = BS_ReadBit(bs);
	if (xy_yx_present) {
		m->xy = LASeR_scaleToFloat(codec, BS_ReadInt(bs, codec->config->scaleBits+12));
		m->yx = LASeR_scaleToFloat(codec, BS_ReadInt(bs, codec->config->scaleBits+12));
	}
	xz_zy_present = BS_ReadBit(bs);
	if (xz_zy_present) {
		m->xz = LASeR_sNbBitsToFloat(codec, BS_ReadInt(bs, codec->config->scaleBits+12), codec->config->scaleBits+12);
		m->yz = LASeR_sNbBitsToFloat(codec, BS_ReadInt(bs, codec->config->scaleBits+12), codec->config->scaleBits+12);
	}
}

static void LASeR_ParseString(BitStream *bs, LASeRString *string)
{
	u8 i;
	u8 len = BS_ReadInt(bs, 5);
	string->length = BS_ReadInt(bs, len);
	SAFEALLOC(string->string, string->length + 1)
	for (i=0; i<string->length; i++) string->string[i] = BS_ReadInt(bs, 8);
	string->string[string->length] = '\0';
}

static void LASeR_ParsePointSequence(LPLASERDEC codec, BitStream *bs, LASeRPointSequence *pointseq)
{
	u32 i;

	pointseq->nbPoints = BS_ReadInt(bs, codec->config->lenBits);

	SAFEALLOC(pointseq->x, pointseq->nbPoints * sizeof(Float))
	SAFEALLOC(pointseq->y, pointseq->nbPoints * sizeof(Float))

	if (pointseq->nbPoints <3) {
		u8 bits = BS_ReadInt(bs, 5);
		for (i = 0; i<pointseq->nbPoints; i++) {
			pointseq->x[i] = LASeR_sNbBitsToFloat(codec, BS_ReadInt(bs, bits), bits);
			pointseq->y[i] = LASeR_sNbBitsToFloat(codec, BS_ReadInt(bs, bits), bits);
		} 
	} else {
		Float dx, dy;
		u8 bitsx, bitsy, bits = BS_ReadInt(bs, 5);
		pointseq->x[0] = LASeR_sNbBitsToFloat(codec, BS_ReadInt(bs, bits), bits);
		pointseq->y[0] = LASeR_sNbBitsToFloat(codec, BS_ReadInt(bs, bits), bits);

		bitsx = BS_ReadInt(bs, 5);
		bitsy = BS_ReadInt(bs, 5);
		for (i = 1; i<pointseq->nbPoints; i++) {
			dx = LASeR_sNbBitsToFloat(codec, BS_ReadInt(bs, bitsx), bitsx);
			pointseq->x[i] = dx + pointseq->x[i-1];
			dy = LASeR_sNbBitsToFloat(codec, BS_ReadInt(bs, bitsy), bitsy);
			pointseq->y[i] = dy + pointseq->y[i-1];
		} 
	}
}

static void LASeR_ParseUrlParam(LPLASERDEC codec, BitStream *bs, LASeRUrlParam *urlParam)
{
	LASeR_ParseString(bs, &(urlParam->name));
	urlParam->source = BS_ReadInt(bs, codec->config->idBits);
}

static void LASeR_ParseActionKey(LPLASERDEC codec, BitStream *bs, LASeRActionKey *actionKey)
{
	actionKey->hasHideObject = BS_ReadBit(bs);
	if (actionKey->hasHideObject) actionKey->hideObject = BS_ReadInt(bs, codec->config->idBits);
	actionKey->hasShowObject = BS_ReadBit(bs);
	if (actionKey->hasShowObject) actionKey->showObject = BS_ReadInt(bs, codec->config->idBits);
	actionKey->hasAnimObject = BS_ReadBit(bs);
	if (actionKey->hasAnimObject) actionKey->animObject = BS_ReadInt(bs, codec->config->idBits);
	actionKey->hasMoveObject = BS_ReadBit(bs);
	if (actionKey->hasMoveObject) {
		LASeR_ParseMatrix(codec, bs, &(actionKey->moveObject));
		actionKey->absolute = BS_ReadBit(bs);
	}
	actionKey->hasPointer = BS_ReadBit(bs);
	if (actionKey->hasPointer) {		
		actionKey->pointerCenterX = LASeR_s12ToFloat(codec, BS_ReadInt(bs, 12));
		actionKey->pointerCenterY = LASeR_s12ToFloat(codec, BS_ReadInt(bs, 12));
		actionKey->pointerSizeX = LASeR_s12ToFloat(codec, BS_ReadInt(bs, 12));
		actionKey->pointerSizeY = LASeR_s12ToFloat(codec, BS_ReadInt(bs, 12));
	}
	actionKey->hasTextToChange = BS_ReadBit(bs);
	if (actionKey->hasTextToChange) {
		actionKey->textToChange = BS_ReadInt(bs, codec->config->idBits);
		LASeR_ParseString(bs, &(actionKey->newText));
	}
	actionKey->hasConcat = BS_ReadBit(bs);
	if (actionKey->hasConcat) {
		actionKey->concatReadID = BS_ReadInt(bs, codec->config->idBits);
		actionKey->concatWriteID = BS_ReadInt(bs, codec->config->idBits);
	}
}

static void LASeR_ParseBase(LPLASERDEC codec, BitStream *bs, LASeRNode *node) 
{
	u8 hasID = BS_ReadBit(bs);
	u32 nodeID = 0;
	if (hasID) nodeID = BS_ReadInt(bs, codec->config->idBits);
	Node_SetDEF((SFNode *)node, nodeID, NULL);
	node->sgprivate->active = BS_ReadBit(bs);
}

static void LASeR_ParseTransform(LPLASERDEC codec, BitStream *bs, LASeRTransform *t) 
{
	u32 i, nbChildren;
	LASeR_ParseBase(codec, bs, (LASeRNode *)t);
	LASeR_ParseMatrix(codec, bs, &(t->m));
	t->hasChoice = BS_ReadBit(bs);
	if (t->hasChoice) t->choice = BS_ReadInt(bs, codec->config->lenBits);
	t->hasDelta = BS_ReadBit(bs);
	if (t->hasDelta) {
		t->deltax = LASeR_s12ToFloat(codec, BS_ReadInt(bs, 12));
		t->deltay = LASeR_s12ToFloat(codec, BS_ReadInt(bs, 12));
	}
	nbChildren = BS_ReadInt(bs, codec->config->lenBits);
	if (nbChildren > 0) {
		for (i=0; i<nbChildren; i++) ChainAddEntry(t->children, LASeR_ParseNode(codec, bs, (LASeRParent *)t));
	} 
}

static void LASeR_ParseShape(LPLASERDEC codec, BitStream *bs, LASeRShape *shape)
{
	u8 morePoints, hasWidth;
	u32 i, colIndex, lcolIndex;

	LASeR_ParseBase(codec, bs, (LASeRNode *)shape);
	
	shape->filled = BS_ReadBit(bs);
	
	colIndex = BS_ReadInt(bs, codec->config->colorIndexBits); 
	if (colIndex) {
		memcpy(&(shape->fill), &(codec->config->colors[colIndex-1]), sizeof(LASeRColor));
	} else {
		shape->filled = 0;
	}

	lcolIndex = BS_ReadInt(bs, codec->config->colorIndexBits);
	if (lcolIndex) {
		memcpy(&(shape->linecolor), &(codec->config->colors[colIndex-1]), sizeof(LASeRColor));
	} else {
		shape->stroked = 0;
	}

	LASeR_ParsePointSequence(codec, bs, &(shape->initpointseq));
	hasWidth = BS_ReadBit(bs);
	if (hasWidth) shape->width = 0.125f * BS_ReadInt(bs, 8);
	morePoints = BS_ReadBit(bs);
	if (morePoints) {		
		shape->nbAdditionalSeqs = BS_ReadInt(bs, codec->config->lenBits);
		SAFEALLOC(shape->morepointseqs, shape->nbAdditionalSeqs * sizeof(LASeRPointSequence))
		for (i =0; i<shape->nbAdditionalSeqs; i++) LASeR_ParsePointSequence(codec, bs, &(shape->morepointseqs[i]));
	} else {
		shape->nbAdditionalSeqs = 0;
		shape->morepointseqs = NULL;
	}
}

static void LASeR_ParseVideo(LPLASERDEC codec, BitStream *bs, LASeRVideo *video)
{
	LASeR_ParseBase(codec, bs, (LASeRNode *)video);
	video->repeat = BS_ReadBit(bs);
	video->hasStartTime = BS_ReadBit(bs);
	if (video->hasStartTime) video->startTime = BS_ReadInt(bs, 32);
	video->streamID = BS_ReadInt(bs, 8);
	video->width = BS_ReadInt(bs, 16);
	video->height = BS_ReadInt(bs, 12);
}

static void LASeR_ParseAudio(LPLASERDEC codec, BitStream *bs, LASeRAudio *audio)
{
	LASeR_ParseBase(codec, bs, (LASeRNode *)audio);
	audio->repeat = BS_ReadBit(bs);
	audio->hasStartTime = BS_ReadBit(bs);
	if (audio->hasStartTime) audio->startTime = BS_ReadInt(bs, 32);
	audio->streamID = BS_ReadInt(bs, 8);
}

static void LASeR_ParseUse(LPLASERDEC codec, BitStream *bs, LASeRUse *use)
{
	u32 target; 

	LASeR_ParseBase(codec, bs, (LASeRNode *)use);
	target = BS_ReadInt(bs, codec->config->idBits);
	if (target == (u32)((1<<codec->config->idBits) - 1)) {
		use->used_node = (LASeRNode *)SG_GetRootNode(codec->current_graph);
	} else {
		use->used_node = (LASeRNode *)SG_FindNode(codec->current_graph, target);
	}
}

static void LASeR_ParseText(LPLASERDEC codec, BitStream *bs, LASeRText *text)
{
	u32 colIndex, face;
	LASeR_ParseBase(codec, bs, (LASeRNode *)text);
	LASeR_ParseString(bs, &(text->string));
	
	colIndex = BS_ReadInt(bs, codec->config->colorIndexBits);
	if (colIndex) {
		memcpy(&(text->color), &(codec->config->colors[colIndex-1]), sizeof(LASeRColor));
	} else {
		text->filled = 0;
	}

	text->horizAlign = BS_ReadInt(bs, 2);
	text->vertAlign = BS_ReadInt(bs, 2);
	text->wrap = BS_ReadBit(bs);
	text->displayRectX = LASeR_s12ToFloat(codec, BS_ReadInt(bs, 12));
	text->hasDisplayRect = BS_ReadBit(bs);
	if (text->hasDisplayRect) text->displayRectY = LASeR_s12ToFloat(codec, BS_ReadInt(bs, 12));
	face = BS_ReadInt(bs, codec->config->fontIndexBits);
	text->face = &(codec->config->fontNames[face]);
	text->style = BS_ReadInt(bs, 4);
	text->size = BS_ReadInt(bs, 12) / 16.0;
}

static void LASeR_ParseBitmap(LPLASERDEC codec, BitStream *bs, LASeRBitmap *bitmap)
{
	LASeR_ParseBase(codec, bs, (LASeRNode *)bitmap);
	bitmap->streamID = BS_ReadInt(bs, 8);
}

static void LASeR_ParseConditional(LPLASERDEC codec, BitStream *bs, LASeRConditional *conditional)
{
	u16 frameLength;	
	LASeR_ParseBase(codec, bs, (LASeRNode *)conditional);
	conditional->hasStartTime = BS_ReadBit(bs);
	if (conditional->hasStartTime) conditional->startTime = BS_ReadInt(bs, 32);
	BS_Align(bs);
	frameLength = BS_ReadInt(bs, 16);
	LASeR_ParseUpdates(codec, bs, conditional);
	BS_Align(bs);
}

static void LASeR_ParseAction(LPLASERDEC codec, BitStream *bs, LASeRAction *action)
{
	u8 moreKeys;
	u32 i;
	LASeR_ParseBase(codec, bs, (LASeRNode *)action);
	action->hasStartTime = BS_ReadBit(bs);
	if (action->hasStartTime) action->startTime = BS_ReadInt(bs, 32);

	// TODO: add the keycodes to the structure
	do {
		u8 keyCode;
		keyCode = BS_ReadInt(bs, 8);
		moreKeys = BS_ReadBit(bs);
	} while(moreKeys);

	action->hasUrl = BS_ReadBit(bs);
	if (action->hasUrl) LASeR_ParseString(bs, &(action->url));

	action->nbUrlParams = BS_ReadInt(bs, codec->config->lenBits);
	SAFEALLOC(action->urlParams, sizeof(LASeRUrlParam) * action->nbUrlParams)
	for (i = 0; i < action->nbUrlParams; i++) LASeR_ParseUrlParam(codec, bs, &(action->urlParams[i]));

	action->nbActionKeys = BS_ReadInt(bs, codec->config->lenBits);
	SAFEALLOC(action->actionKeys, sizeof(LASeRActionKey) * action->nbActionKeys)
	for (i = 0; i < action->nbActionKeys; i++) LASeR_ParseActionKey(codec, bs, &(action->actionKeys[i]));
}

static void LASeR_ParseAnimateTransform(LPLASERDEC codec, BitStream *bs, LASeRAnimateTransform *animt) 
{
	u32 i;

	LASeR_ParseBase(codec, bs, (LASeRNode *)animt);
	animt->target = BS_ReadInt(bs, codec->config->idBits);
	animt->hasStartTime = BS_ReadBit(bs);
	if (animt->hasStartTime) animt->startTime = BS_ReadInt(bs, 32);

	animt->repeat = BS_ReadBit(bs);
	animt->nbKeys = BS_ReadInt(bs, codec->config->lenBits);
	SAFEALLOC(animt->keys, sizeof(LASeRTransformKey)*animt->nbKeys)
	SAFEALLOC(animt->durations, sizeof(u32)*animt->nbKeys)
	for (i = 0; i <animt->nbKeys; i++) {
		animt->durations[i] = BS_ReadInt(bs, 32);
		animt->keys[i].hasMatrix = BS_ReadBit(bs);
		if (animt->keys[i].hasMatrix) {
			LASeR_ParseMatrix(codec, bs, &(animt->keys[i].m));
		} else {
			animt->keys[i].deltax = LASeR_s12ToFloat(codec, BS_ReadInt(bs, 12));
			animt->keys[i].deltay = LASeR_s12ToFloat(codec, BS_ReadInt(bs, 12));
		}
	}
}

static void LASeR_ParseAnimateColor(LPLASERDEC codec, BitStream *bs, LASeRAnimateColor *animc) 
{
	u32 i;
	LASeR_ParseBase(codec, bs, (LASeRNode *)animc);
	animc->target = BS_ReadInt(bs, codec->config->idBits);
	animc->hasStartTime = BS_ReadBit(bs);
	if (animc->hasStartTime) animc->startTime = BS_ReadInt(bs, 32);
	animc->repeat = BS_ReadBit(bs);
	animc->nbKeys = BS_ReadInt(bs, codec->config->lenBits);
	SAFEALLOC(animc->keys, sizeof(LASeRColorKey) * animc->nbKeys)
	SAFEALLOC(animc->durations, sizeof(u32)*animc->nbKeys)
	for (i = 0; i <animc->nbKeys; i++) {
		u32 colIndex;
		animc->durations[i] = BS_ReadInt(bs, 32);
		colIndex = BS_ReadInt(bs, codec->config->colorIndexBits);
		memcpy(&(animc->keys[i].color), &(codec->config->colors[colIndex-1]), sizeof(LASeRColor));
	}
}

static void LASeR_ParseAnimateActivate(LPLASERDEC codec, BitStream *bs, LASeRAnimateActivate *anima) 
{
	u32 i;
	LASeR_ParseBase(codec, bs, (LASeRNode *)anima);
	anima->target = BS_ReadInt(bs, codec->config->idBits);
	anima->hasStartTime = BS_ReadBit(bs);
	if (anima->hasStartTime) anima->startTime = BS_ReadInt(bs, 32);
	anima->repeat = BS_ReadBit(bs);
	anima->nbKeys = BS_ReadInt(bs, codec->config->lenBits);
	SAFEALLOC(anima->keys, sizeof(LASeRActivateKey)*anima->nbKeys)
	SAFEALLOC(anima->durations, sizeof(u32)*anima->nbKeys)
	for (i = 0; i <anima->nbKeys; i++) {
		anima->durations[i] = BS_ReadInt(bs, 32);
		anima->keys[i].animObject = BS_ReadInt(bs, codec->config->idBits);
	}
}

static void LASeR_ParseCursor(LPLASERDEC codec, BitStream *bs, LASeRCursor *cursor) 
{
	u32 i;
	LASeR_ParseBase(codec, bs, (LASeRNode *)cursor);
	cursor->target = BS_ReadInt(bs, codec->config->idBits);
	cursor->nbKeys = BS_ReadInt(bs, codec->config->lenBits);
	SAFEALLOC(cursor->keys, sizeof(LASeRKey) * cursor->nbKeys)
	for (i = 0; i <cursor->nbKeys; i++) {
		cursor->keys[i].keyCode = BS_ReadInt(bs, 8);
		cursor->keys[i].MoveX = LASeR_s12ToFloat(codec, BS_ReadInt(bs, 12));
		cursor->keys[i].MoveY = LASeR_s12ToFloat(codec, BS_ReadInt(bs, 12));
	}
}

static void LASeR_ParseTextInput(LPLASERDEC codec, BitStream *bs, LASeRTextInput *textinput) 
{
	LASeR_ParseBase(codec, bs, (LASeRNode *)textinput);
	textinput->text = BS_ReadInt(bs, codec->config->idBits);
	textinput->hasTitle = BS_ReadBit(bs);
	if (textinput->hasTitle) {
		LASeR_ParseString(bs, &(textinput->title));
	}
}

static void *LASeR_ParseNode(LPLASERDEC codec, BitStream *bs, LASeRParent *parent) 
{
	void *current_node;
	u32 node_type = BS_ReadInt(bs, 5);
	/*translate to internal TAG namespace*/
	node_type += TAG_RANGE_FIRST_LASeR;

	current_node = LASERSG_NewNode(codec->current_graph, node_type);

	switch (node_type) {
	case TAG_LASeRTransform:
		LASeR_ParseTransform(codec, bs, (LASeRTransform *)current_node);
		break;
	case TAG_LASeRShape: 
		LASeR_ParseShape(codec, bs, (LASeRShape *)current_node);
		break;
	case TAG_LASeRAction:
 		LASeR_ParseAction(codec, bs, (LASeRAction *)current_node);
		break;
	case TAG_LASeRVideo:
		LASeR_ParseVideo(codec, bs, (LASeRVideo *)current_node);
		break;
	case TAG_LASeRAudio:
		LASeR_ParseAudio(codec, bs, (LASeRAudio *)current_node);
		break;
	case TAG_LASeRBitmap:
		LASeR_ParseBitmap(codec, bs, (LASeRBitmap *)current_node);
		break;
	case TAG_LASeRText:
		LASeR_ParseText(codec, bs, (LASeRText *)current_node);
		break;
	case TAG_LASeRAnimateTransform:
		LASeR_ParseAnimateTransform(codec, bs, (LASeRAnimateTransform *)current_node);
		break;
	case TAG_LASeRAnimateColor:
		LASeR_ParseAnimateColor(codec, bs, (LASeRAnimateColor *)current_node);
		break;
	case TAG_LASeRAnimateActivate:
		LASeR_ParseAnimateActivate(codec, bs, (LASeRAnimateActivate *)current_node);
		break;
	case TAG_LASeRUse:
		LASeR_ParseUse(codec, bs, (LASeRUse *)current_node);
		break;
	case TAG_LASeRConditional:
		LASeR_ParseConditional(codec, bs, (LASeRConditional *)current_node);
		break;
	case TAG_LASeRTextInput:
		LASeR_ParseTextInput(codec, bs, (LASeRTextInput *)current_node);
		break;
	case TAG_LASeRCursor:
		LASeR_ParseCursor(codec, bs, (LASeRCursor *)current_node);
		break;
	}
	Node_Register(current_node, (SFNode *)parent);
	return current_node;
}

M4Err LASeRNode_InsertChild(SFNode *parent, SFNode *new_child, s32 Position)
{
	SFParent *node = (SFParent *) parent;

	if (Position == -1) {
		return ChainAddEntry(node->children, new_child);
	} else {
		return ChainInsertEntry(node->children, new_child, Position);
	}
}

static M4Err LASeR_ParseInsert(LPLASERDEC codec, BitStream *bs) 
{
	SFNode *node;
	SFNode *targetnode;
	u32 target = BS_ReadInt(bs, codec->config->idBits);
	if (target == (u32)((1<<codec->config->idBits) - 1)) {
		targetnode = SG_GetRootNode(codec->current_graph);
	} else {
		targetnode = SG_FindNode(codec->current_graph, target);
	}

	if (!targetnode) return M4NonCompliantBitStream;

	node = LASeR_ParseNode(codec, bs, (LASeRParent *)targetnode);
	LASeRNode_InsertChild(targetnode, (SFNode *) node, -1);
	return 0;
}

static M4Err LASeR_ParseDelete(LPLASERDEC codec, BitStream *bs) 
{
	SFNode *targetnode;
	u32 target = BS_ReadInt(bs, codec->config->idBits);
	if (target == (u32)((1<<codec->config->idBits) - 1)) {
		targetnode = SG_GetRootNode(codec->current_graph);
	} else {
		targetnode = SG_FindNode(codec->current_graph, target);
	}

	if (!targetnode) return M4NonCompliantBitStream;

	return M4OK;
}

static void LASeR_ParseReplace(LPLASERDEC codec, BitStream *bs) 
{
	LASeRNode *node;
	SFNode *targetnode;
	u32 target = BS_ReadInt(bs, codec->config->idBits);
	if (target == (u32)((1<<codec->config->idBits) - 1)) {
		targetnode = SG_GetRootNode(codec->current_graph);
	} else {
		targetnode = SG_FindNode(codec->current_graph, target);
	}

	if (!targetnode) return;

	node = LASeR_ParseNode(codec, bs, (LASeRParent *)targetnode);
	/* TO DO: To be removed when the replace node is implemented */
	LASeRNode_Del((SFNode *)node);
}

static void LASeR_ParseReplaceIndexed(LPLASERDEC codec, BitStream *bs) 
{
	LASeRNode *node;
	SFNode *targetnode;
	u32 index = BS_ReadInt(bs, codec->config->lenBits);
	u32 target = BS_ReadInt(bs, codec->config->idBits);
	if (target == (u32)((1<<codec->config->idBits) - 1)) {
		targetnode = SG_GetRootNode(codec->current_graph);
	} else {
		targetnode = SG_FindNode(codec->current_graph, target);
	}

	if (!targetnode) return;

	node = (LASeRNode *)LASeR_ParseNode(codec, bs, (LASeRParent *)targetnode);
	/* TO DO: To be removed when the replace node is implemented */
	LASeRNode_Del((SFNode *)node);
}

static void LASeR_ParseDeleteIndexed(LPLASERDEC codec, BitStream *bs) 
{
	SFNode *targetnode;

	u32 index = BS_ReadInt(bs, codec->config->lenBits);
	u32 target = BS_ReadInt(bs, codec->config->idBits);
	if (target == (u32)((1<<codec->config->idBits) - 1)) {
		targetnode = SG_GetRootNode(codec->current_graph);
	} else {
		targetnode = SG_FindNode(codec->current_graph, target);
	}

	if (!targetnode) return;
	Node_ReplaceChild(targetnode, ((SFParent *)targetnode)->children, index, NULL);
	SG_NodeChanged(targetnode, NULL);

	if (codec->current_graph->NodeModified) codec->current_graph->NodeModified(codec->current_graph->ModifCallback, targetnode);
}

static void LASeR_ParseReplaceField(LPLASERDEC codec, BitStream *bs) 
{
	SFNode *targetnode;
	u8 floatLen;
	u32 index;
	Float floatEncodedValue1, floatEncodedValue2;
	u32 colIndex;
	LASeRNode *nodevalue = NULL;

	u8 fieldIndex = BS_ReadInt(bs, 5);
	u32 target = BS_ReadInt(bs, codec->config->idBits);
	u8 isIndexed = BS_ReadBit(bs);

	if (isIndexed) index = BS_ReadInt(bs, codec->config->lenBits);
	else index = 0;


	if (target == (u32)((1<<codec->config->idBits) - 1)) {
		targetnode = SG_GetRootNode(codec->current_graph);
	} else {
		targetnode = SG_FindNode(codec->current_graph, target);
	}
	
	switch (fieldIndex) {
	case 0:
		{
			//type 0 is float: Audio.volume, Shape.width
			floatLen = BS_ReadInt(bs, 4);
			if (targetnode && targetnode->sgprivate->tag == TAG_LASeRShape) {
				((LASeRShape *)targetnode)->width = LASeR_genericNbBitsToFloat(codec, BS_ReadInt(bs, floatLen + 9), floatLen + 9);
			} 			
		}
		break;
    case 1:
     //type 1 is float:
		floatLen = BS_ReadInt(bs, 4);
		LASeR_genericNbBitsToFloat(codec, BS_ReadInt(bs, floatLen + 9), floatLen + 9);
		break;
    case 2:
     //type 2 is vec2f: Shape.points, Transform.translation
		floatLen = BS_ReadInt(bs, 4);
		floatEncodedValue1 = LASeR_genericNbBitsToFloat(codec, BS_ReadInt(bs, floatLen + 9), floatLen + 9);
		floatLen = BS_ReadInt(bs, 4);
		floatEncodedValue2 = LASeR_genericNbBitsToFloat(codec, BS_ReadInt(bs, floatLen + 9), floatLen + 9);
		if (targetnode && targetnode->sgprivate->tag == TAG_LASeRShape) {
			((LASeRShape *)targetnode)->initpointseq.x[index] = floatEncodedValue1;
			((LASeRShape *)targetnode)->initpointseq.y[index] = floatEncodedValue2;
		} else if (targetnode && targetnode->sgprivate->tag == TAG_LASeRTransform) {
			((LASeRTransform *)targetnode)->m.xz = floatEncodedValue1;
			((LASeRTransform *)targetnode)->m.yz = floatEncodedValue2;
			SG_NodeChanged(targetnode, NULL);
		}
		break;
    case 3:
     //type 3 is vec2f: Transform.scale
		floatLen = BS_ReadInt(bs, 4);
		floatEncodedValue1 = LASeR_genericNbBitsToFloat(codec, BS_ReadInt(bs, floatLen + 9), floatLen + 9);
		floatLen = BS_ReadInt(bs, 4);
		floatEncodedValue2 = LASeR_genericNbBitsToFloat(codec, BS_ReadInt(bs, floatLen + 9), floatLen + 9);
		if (targetnode && targetnode->sgprivate->tag == TAG_LASeRTransform) {
			((LASeRTransform *)targetnode)->m.xx = floatEncodedValue1;
			((LASeRTransform *)targetnode)->m.yy = floatEncodedValue2;
			SG_NodeChanged(targetnode, NULL);
		}
		break;
    case 4:
     //type 4 is vec2f: Transform.delta
		floatLen = BS_ReadInt(bs, 4);
		floatEncodedValue1 = LASeR_genericNbBitsToFloat(codec, BS_ReadInt(bs, floatLen + 9), floatLen + 9);
		floatLen = BS_ReadInt(bs, 4);
		floatEncodedValue2 = LASeR_genericNbBitsToFloat(codec, BS_ReadInt(bs, floatLen + 9), floatLen + 9);
		if (targetnode && targetnode->sgprivate->tag == TAG_LASeRTransform) {
			((LASeRTransform *)targetnode)->deltax = floatEncodedValue1;
			((LASeRTransform *)targetnode)->deltay = floatEncodedValue2;
			SG_NodeChanged(targetnode, NULL);
		}
		break;
    case 5:
     //type 5 is matrix: Transform.matrix
		LASeR_ParseMatrix(codec, bs, &(((LASeRTransform *)targetnode)->m));
		SG_NodeChanged(targetnode, NULL);
		break;
    case 6:
     //type 6 is node: Transform.children
		nodevalue = (LASeRNode *)LASeR_ParseNode(codec, bs, NULL);
		/* TO DO: To be removed when the replace node is implemented */
		LASeRNode_Del((SFNode *)nodevalue);
		SG_NodeChanged(targetnode, NULL);
		break;
    case 7:
     //type 7 is ID: Action.animObject
		((LASeRAction *)targetnode)->actionKeys[index].animObject = BS_ReadInt(bs, codec->config->idBits);
		break;
    case 8:
     //type 8 is ID: Action.hideObject
		((LASeRAction *)targetnode)->actionKeys[index].hideObject = BS_ReadInt(bs, codec->config->idBits);
		break;
    case 9:
     //type 9 is ID: Action.showObject
		((LASeRAction *)targetnode)->actionKeys[index].showObject = BS_ReadInt(bs, codec->config->idBits);
		break;
    case 10:
     //type 10 is string: Text.string, Action.url
		if (targetnode && targetnode->sgprivate->tag == TAG_LASeRText) {
			LASeRString_Del(((LASeRText *)targetnode)->string);
			LASeR_ParseString(bs, &(((LASeRText *)targetnode)->string));			
		} else if (targetnode && targetnode->sgprivate->tag == TAG_LASeRAction){
			LASeRString_Del(((LASeRAction *)targetnode)->url);
			LASeR_ParseString(bs, &(((LASeRAction *)targetnode)->url));
		}
		break;
    case 11:
     //type 11 is color: Text.color, Shape.color
		colIndex = BS_ReadInt(bs, codec->config->colorIndexBits);
		if (targetnode && targetnode->sgprivate->tag == TAG_LASeRText) {
			if (colIndex) {
				memcpy(&(((LASeRText *)targetnode)->color), &(codec->config->colors[colIndex - 1]), sizeof(LASeRColor));
				((LASeRText *)targetnode)->filled = 1;
			} else ((LASeRText *)targetnode)->filled = 0;
		} else if (targetnode && targetnode->sgprivate->tag == TAG_LASeRShape) {
			if (colIndex) {
				memcpy(&(((LASeRShape *)targetnode)->fill), &(codec->config->colors[colIndex - 1]), sizeof(LASeRColor));
				((LASeRShape *)targetnode)->filled = 1;
			} else ((LASeRShape *)targetnode)->filled = 0;
		}
		break;
    case 12:
     //type 12 is color: Shape.linecolor
		colIndex = BS_ReadInt(bs, codec->config->colorIndexBits);
		if (targetnode && targetnode->sgprivate->tag == TAG_LASeRShape) {
			if (colIndex) {
				memcpy(&(((LASeRShape *)targetnode)->linecolor), &(codec->config->colors[colIndex - 1]), sizeof(LASeRColor));
				((LASeRShape *)targetnode)->stroked = 1;
			} else ((LASeRShape *)targetnode)->stroked = 0;
		}
		break;
    case 13:
     //type 13 is boolean: Shape.filled
		if (targetnode && targetnode->sgprivate->tag == TAG_LASeRShape) {
			((LASeRShape *)targetnode)->filled = BS_ReadBit(bs);
		}
		break;
    case 14:
     //type 14 is boolean: Shape.dotted
		if (targetnode && targetnode->sgprivate->tag == TAG_LASeRShape) {
		}
		break;
    case 17:
     //type 17 is boolean: *.active
		targetnode->sgprivate->active = BS_ReadBit(bs);
		break;
    case 15:
     //type 15 is time: *.startTime
		((LASeRTimedNode *)targetnode)->startTime = BS_ReadInt(bs, 32);
		break;
    case 16:
     //type 16 is integer: Transform.choice
		if (targetnode && targetnode->sgprivate->tag == TAG_LASeRTransform) {
			((LASeRTransform *)targetnode)->choice = BS_ReadInt(bs, codec->config->lenBits);
			SG_NodeChanged(targetnode, NULL);
		}
		break;
	}
	if (codec->current_graph->NodeModified) codec->current_graph->NodeModified(codec->current_graph->ModifCallback, targetnode);
}

static void LASeR_ParseBackground(LPLASERDEC codec, BitStream *bs) 
{
	SFNode *root = SG_GetRootNode(codec->current_graph);
	LASeRBackground *bck = LASERSG_NewNode(codec->current_graph, TAG_LASeRBackground);
	Node_Register(bck, root);

	bck->sgprivate->active = 1;

	bck->isImage = BS_ReadBit(bs);
	if (bck->isImage) {
		bck->streamID = BS_ReadInt(bs, 8);
	} else {
		u32 colIndex = BS_ReadInt(bs, codec->config->colorIndexBits);
		bck->color = &(codec->config->colors[colIndex - 1]);
	}

	if (!root) {
		MPEG4_Node_Del((SFNode *)bck);
		return;
	}
	LASeRNode_InsertChild(root, (SFNode *) bck, -1);
}

static void LASeR_ParseClean(LPLASERDEC codec, BitStream *bs) 
{
	u16 serviceID = BS_ReadInt(bs, 12);
	u8 groupID = BS_ReadInt(bs, 8);
}

static void LASeR_ParseSave(LPLASERDEC codec, BitStream *bs) 
{
	u32 i;
	u32 *ids;
	
	u16 serviceID = BS_ReadInt(bs, 12);
	u8 groupID = BS_ReadInt(bs, 8);
	u32 nbIds = BS_ReadInt(bs, codec->config->lenBits);
	SAFEALLOC(ids, sizeof(u32)*nbIds)
	for (i =0; i<nbIds; i++) ids[i] = BS_ReadInt(bs, codec->config->idBits);

	free(ids);
	ids = NULL;
}

static void LASeR_ParseRestore(LPLASERDEC codec, BitStream *bs) 
{
	u16 serviceID = BS_ReadInt(bs, 12);
	u8 groupID = BS_ReadInt(bs, 8);
}

static void LASeR_ParseUpdates(LPLASERDEC codec, BitStream *bs, void *conditional) 
{
	u32 i;

	u32 nbUpdates = BS_ReadInt(bs, codec->config->lenBits);
	for (i = 0; i < nbUpdates; i++) {
		u8 type = BS_ReadInt(bs, 5);
		switch (type) {
		case 0:
			LASeR_ParseInsert(codec, bs);
			break;
		case 1:
			LASeR_ParseDelete(codec, bs);
			break;
		case 2:
			LASeR_ParseReplace(codec, bs);
			break;
		case 3:
			LASeR_ParseReplaceIndexed(codec, bs);
			break;
		case 4:
			break;
		case 5:
			LASeR_ParseDeleteIndexed(codec, bs);
			break;
		case 6:
			LASeR_ParseReplaceField(codec, bs);
			break;
		case 7:
			LASeR_ParseBackground(codec, bs);
			break;
		case 8:
			LASeR_ParseClean(codec, bs);
			break;
		case 9:
			LASeR_ParseSave(codec, bs);
			break;
		case 10:
			LASeR_ParseRestore(codec, bs);
			break;
		}
	}
}

void LASeR_EndOfStream(void *codec)
{
	((LASeRDecoder *) codec)->LastError = M4IOErr;
}

void BS_SetEOSCallback(BitStream *bs, void (*EndOfStream)(void *par), void *par);

M4Err LASeR_ParseSceneUnit(LPLASERDEC codec, char *data, u32 data_length) 
{
	LASeRSceneHeader *sh = codec->config;
	BitStream *bs;

	u32 i;
	u8 colorPresent, fontPresent;
	M4Err e = M4OK;

	if (!codec || !data || codec->dec_memory_mode) return M4BadParam;

	codec->au_num++;
	//if (codec->au_num > 2) return M4OK;

	bs = NewBitStream(data, data_length, BS_READ);

	/*setup current scene graph*/
	codec->current_graph = codec->scenegraph;
	
	if (SG_GetRootNode(codec->current_graph) == NULL) {
		LASeRTransform *root = (LASeRTransform *)LASERSG_NewNode(codec->current_graph,TAG_LASeRTransform);
		Node_Register((SFNode *)root, NULL);
		SG_SetRootNode(codec->current_graph, (SFNode *)root);
		root->sgprivate->active = 1;
	}

	colorPresent = BS_ReadBit(bs);
	if (colorPresent) {
		u8 newNbColors = BS_ReadInt(bs, 8);
		codec->config->nbColors += newNbColors; 
		if (codec->config->colors) {
			codec->config->colors = realloc(codec->config->colors, codec->config->nbColors * sizeof(LASeRColor));
		} else {
			SAFEALLOC(codec->config->colors, codec->config->nbColors * sizeof(LASeRColor));
		}
		for (i = codec->config->nbColors - newNbColors; i < codec->config->nbColors; i++) {
			codec->config->colors[i].r = BS_ReadInt(bs, codec->config->colorBits)/normalizer[codec->config->colorBits];
			codec->config->colors[i].g = BS_ReadInt(bs, codec->config->colorBits)/normalizer[codec->config->colorBits];
			codec->config->colors[i].b = BS_ReadInt(bs, codec->config->colorBits)/normalizer[codec->config->colorBits];
		}
		{
			u32 n = codec->config->nbColors + 1;
			codec->config->colorIndexBits = 0;
			while (n>0) { n = n>>1; codec->config->colorIndexBits++; } 
		}
	} 

	fontPresent = BS_ReadBit(bs);
	if (fontPresent) {
		u8 newNbFonts = BS_ReadInt(bs, 8);
		codec->config->nbFonts += newNbFonts; 
		if (codec->config->fontNames) {
			codec->config->fontNames = realloc(codec->config->fontNames, codec->config->nbFonts * sizeof(LASeRString));
		} else {
			SAFEALLOC(codec->config->fontNames, codec->config->nbFonts * sizeof(LASeRString))
		}
		for (i= 0; i<codec->config->nbFonts; i++) {
			LASeR_ParseString(bs, &(codec->config->fontNames[i]));
		}
		{
			u32 n = codec->config->nbFonts + 1;
			codec->config->fontIndexBits = 0;
			while (n>0) { n = n>>1; codec->config->fontIndexBits++; } 
		}
	}
	LASeR_ParseUpdates(codec, bs, NULL);

	DeleteBitStream(bs);

	codec->current_graph = NULL;

	return e;

}

void LASeR_DeleteSceneHeader(LASeRSceneHeader *config)
{
	u32 i;
	if (config->key) free(config->key);
	config->key = NULL;
	LASeRString_Del(config->urlPrefix);
	LASeRString_Del(config->name);
	if (config->colors) free(config->colors);
	config->colors = NULL;
	if (config->fontNames) {
		for (i = 0; i < config->nbFonts; i++) {
			LASeRString_Del(config->fontNames[i]);
		}
		free(config->fontNames);
		config->fontNames = NULL;
	}
	free(config);
	config = NULL;
}

LASeRSceneHeader *LASeR_ParseSceneHeader(BitStream *bs) 
{
	LASeRSceneHeader *config;

	u32 bufferingTime;
	u8 encodingType;
	s8 resolutionbits;
	u8 keyPresent, urlPrefixPresent, namePresent; 
	
	SAFEALLOC(config, sizeof(LASeRSceneHeader))
	
	bufferingTime = BS_ReadInt(bs, 24);
	encodingType = BS_ReadInt(bs, 3);
	switch (encodingType) {
	case 0:
		config->idBits = BS_ReadInt(bs, 4); config->idBits++;
		config->lenBits = BS_ReadInt(bs, 4); config->lenBits++;
		config->colorBits = BS_ReadInt(bs, 3); config->colorBits++;
		resolutionbits = BS_ReadInt(bs, 4);
		config->scaleBits = BS_ReadInt(bs, 4);
		break;
	case 1:
		config->idBits = 8;
		config->lenBits = 8;
		config->colorBits = 4;
		resolutionbits = 0;
		config->scaleBits = 0;
		break;
	case 2:
		config->idBits = 10;
		config->lenBits = 10;
		config->colorBits = 5;
		resolutionbits = 0;
		config->scaleBits = 0;
		break;
	case 3:
		config->idBits = 16;
		config->lenBits = 16;
		config->colorBits = 8;
		resolutionbits = 0;
		config->scaleBits = 0;
		break;
	}
	if (resolutionbits > 7) {
		resolutionbits -= 16;	
		config->resolution = 1.0f/(1<<(-resolutionbits));
	} else {
		config->resolution = (1<<resolutionbits)*1.0f;
	}

	config->append = BS_ReadBit(bs);
	if (!config->append) {
		config->normalized = BS_ReadBit(bs);
		config->width = BS_ReadInt(bs, 12);
		config->height = BS_ReadInt(bs, 12);
	}

	keyPresent = BS_ReadBit(bs);
	if (keyPresent) {
		config->keyLength = BS_ReadInt(bs, 7);
		SAFEALLOC(config->key, config->keyLength)
		config->forceKey = BS_ReadBit(bs);	
	}
	
	urlPrefixPresent = BS_ReadBit(bs);
	if (urlPrefixPresent) LASeR_ParseString(bs, &(config->urlPrefix));

	namePresent = BS_ReadBit(bs);
	if (namePresent) LASeR_ParseString(bs, &(config->name));

	return config;
}


#endif	//M4_USE_LASeR
