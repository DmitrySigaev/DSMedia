#include "m4_laser_dev.h"
#include <gpac/intern/m4_scenegraph_dev.h>

static void *LASeRMem_DecNode(BitStream *bs, LASeRSceneHeader *sh);
static void LASeRMem_DecUpdates(BitStream *bs, LASeRSceneHeader *sh, LASeRUpdate **updates, u32 *nbUpdates);

static void LASeRMem_DecMatrix(BitStream *bs, LASeRSceneHeader *sh, LASeRMatrix *m) 
{
	u8 xx_yy_present, xy_yx_present, xz_zy_present;

	m->xx = m->yy = 1;
	m->xy = m->yx = m->xz = m->yz = 0;

	xx_yy_present = BS_ReadBit(bs);
	if (xx_yy_present) {
		m->xx = BS_ReadInt(bs, sh->scaleBits+12);
		m->yy = BS_ReadInt(bs, sh->scaleBits+12);
	}
	xy_yx_present = BS_ReadBit(bs);
	if (xy_yx_present) {
		m->xy = BS_ReadInt(bs, sh->scaleBits+12);
		m->yx = BS_ReadInt(bs, sh->scaleBits+12);
	}
	xz_zy_present = BS_ReadBit(bs);
	if (xz_zy_present) {
		m->xz = BS_ReadInt(bs, sh->scaleBits+12);
		m->yz = BS_ReadInt(bs, sh->scaleBits+12);
	}
}

static void LASeRMem_DecString(BitStream *bs, LASeRSceneHeader *sh, LASeRString *string)
{
	u8 i;
	u8 len = BS_ReadInt(bs, 5);
	string->length = BS_ReadInt(bs, len);
	string->string = malloc(string->length);
	memset(string->string, 0, string->length);
	for (i=0; i<string->length; i++) string->string[i] = BS_ReadInt(bs, 8);
	string->string[string->length] = '\0';
}

static void LASeRMem_DecPointSequence(BitStream *bs, LASeRSceneHeader *sh, LASeRPointSequence *pointseq)
{
	u32 i;

	pointseq->nbPoints = BS_ReadInt(bs, sh->lenBits);

	pointseq->x = malloc(pointseq->nbPoints * sizeof(u32));
	memset(pointseq->x, 0, pointseq->nbPoints * sizeof(u32));
	pointseq->y = malloc(pointseq->nbPoints * sizeof(u32));
	memset(pointseq->y, 0, pointseq->nbPoints * sizeof(u32));
	if (pointseq->nbPoints <3) {
		u8 bits = BS_ReadInt(bs, 5);
		for (i = 0; i<pointseq->nbPoints; i++) {
			pointseq->x[i] = BS_ReadInt(bs, bits);
			pointseq->y[i] = BS_ReadInt(bs, bits);
		} 
	} else {
		u32 dx, dy;
		u8 bitsx, bitsy, bits = BS_ReadInt(bs, 5);
		pointseq->x[0] = BS_ReadInt(bs, bits);
		pointseq->y[0] = BS_ReadInt(bs, bits);

		bitsx = BS_ReadInt(bs, 5);
		bitsy = BS_ReadInt(bs, 5);
		for (i = 1; i<pointseq->nbPoints; i++) {
			dx = BS_ReadInt(bs, bitsx);
			pointseq->x[i] = dx + pointseq->x[i-1];
			dy = BS_ReadInt(bs, bitsy);
			pointseq->y[i] = dy + pointseq->y[i-1];
		} 
	}
}

static void LASeRMem_DecUrlParam(BitStream *bs, LASeRSceneHeader *sh, LASeRUrlParam *urlParam)
{
	LASeRMem_DecString(bs, sh, &(urlParam->name));
	urlParam->source = BS_ReadInt(bs, sh->idBits);
}

static void LASeRMem_DecActionKey(BitStream *bs, LASeRSceneHeader *sh, LASeRActionKey *actionKey)
{
	actionKey->hasHideObject = BS_ReadBit(bs);
	if (actionKey->hasHideObject) actionKey->hideObject = BS_ReadInt(bs, sh->idBits);
	actionKey->hasShowObject = BS_ReadBit(bs);
	if (actionKey->hasShowObject) actionKey->showObject = BS_ReadInt(bs, sh->idBits);
	actionKey->hasAnimObject = BS_ReadBit(bs);
	if (actionKey->hasAnimObject) actionKey->animObject = BS_ReadInt(bs, sh->idBits);
	actionKey->hasMoveObject = BS_ReadBit(bs);
	if (actionKey->hasMoveObject) {
		LASeRMem_DecMatrix(bs, sh, &(actionKey->moveObject));
		actionKey->absolute = BS_ReadBit(bs);
	}
	actionKey->hasPointer = BS_ReadBit(bs);
	if (actionKey->hasPointer) {		
		actionKey->pointerCenterX = BS_ReadInt(bs, 12);
		actionKey->pointerCenterY = BS_ReadInt(bs, 12);
		actionKey->pointerSizeX = BS_ReadInt(bs, 12);
		actionKey->pointerSizeY = BS_ReadInt(bs, 12);
	}
	actionKey->hasTextToChange = BS_ReadBit(bs);
	if (actionKey->hasTextToChange) {
		actionKey->textToChange = BS_ReadInt(bs, sh->idBits);
		LASeRMem_DecString(bs, sh, &(actionKey->newText));
	}
	actionKey->hasConcat = BS_ReadBit(bs);
	if (actionKey->hasConcat) {
		actionKey->concatReadID = BS_ReadInt(bs, sh->idBits);
		actionKey->concatWriteID = BS_ReadInt(bs, sh->idBits);
	}
}

static void LASeRMem_DecBase(BitStream *bs, LASeRSceneHeader *sh, LASeRNode *node) 
{
	u8 hasID = BS_ReadBit(bs);
	u32 nodeID = 0;
	if (hasID) nodeID = BS_ReadInt(bs, sh->idBits);
	Node_SetDEF((SFNode *)node, nodeID, NULL);
	node->sgprivate->active = BS_ReadBit(bs);
}

static void LASeRMem_DecTransform(BitStream *bs, LASeRSceneHeader *sh, LASeRTransform *t) 
{
	u32 i, nbChildren;
	LASeRMem_DecBase(bs, sh, (LASeRNode *)t);
	LASeRMem_DecMatrix(bs, sh, &(t->m));
	t->hasChoice = BS_ReadBit(bs);
	if (t->hasChoice) t->choice = BS_ReadInt(bs, sh->lenBits);
	t->hasDelta = BS_ReadBit(bs);
	if (t->hasDelta) {
		t->deltax = BS_ReadInt(bs, 12);
		t->deltay = BS_ReadInt(bs, 12);
	}
	nbChildren = BS_ReadInt(bs, sh->lenBits);
	if (nbChildren > 0) {
		for (i=0; i<nbChildren; i++) ChainAddEntry(t->children, LASeRMem_DecNode(bs, sh));
	}
}

static void LASeRMem_DecShape(BitStream *bs, LASeRSceneHeader *sh, LASeRShape *shape)
{
	u8 morePoints;
	u32 i;
	LASeRMem_DecBase(bs, sh, (LASeRNode *)shape);
	shape->filled = BS_ReadBit(bs);
	shape->colIndex = BS_ReadInt(bs, sh->colorIndexBits); 
	shape->lcolIndex = BS_ReadInt(bs, sh->colorIndexBits);
	LASeRMem_DecPointSequence(bs, sh, &(shape->initpointseq));
	shape->hasWidth = BS_ReadBit(bs);
	if (shape->hasWidth) shape->encodedWidth = BS_ReadInt(bs, 8);
	morePoints = BS_ReadBit(bs);
	if (morePoints) {		
		shape->nbAdditionalSeqs = BS_ReadInt(bs, sh->lenBits);
		shape->morepointseqs = malloc(shape->nbAdditionalSeqs * sizeof(LASeRPointSequence));
		for (i =0; i<shape->nbAdditionalSeqs; i++) LASeRMem_DecPointSequence(bs, sh, &(shape->morepointseqs[i]));
	} else {
		shape->nbAdditionalSeqs = 0;
		shape->morepointseqs = NULL;
	}
}

static void LASeRMem_DecVideo(BitStream *bs, LASeRSceneHeader *sh, LASeRVideo *video)
{
	LASeRMem_DecBase(bs, sh, (LASeRNode *)video);
	video->repeat = BS_ReadBit(bs);
	video->hasStartTime = BS_ReadBit(bs);
	if (video->hasStartTime) video->startTime = BS_ReadInt(bs, 32);
	video->streamID = BS_ReadInt(bs, 8);
	video->width = BS_ReadInt(bs, 16);
	video->height = BS_ReadInt(bs, 12);
}

static void LASeRMem_DecAudio(BitStream *bs, LASeRSceneHeader *sh, LASeRAudio *audio)
{
	LASeRMem_DecBase(bs, sh, (LASeRNode *)audio);
	audio->repeat = BS_ReadBit(bs);
	audio->hasStartTime = BS_ReadBit(bs);
	if (audio->hasStartTime) audio->startTime = BS_ReadInt(bs, 32);
	audio->streamID = BS_ReadInt(bs, 8);
}

static void LASeRMem_DecUse(BitStream *bs, LASeRSceneHeader *sh, LASeRUse *use)
{
	LASeRMem_DecBase(bs, sh, (LASeRNode *)use);
	use->target = BS_ReadInt(bs, sh->idBits);
}

static void LASeRMem_DecText(BitStream *bs, LASeRSceneHeader *sh, LASeRText *text)
{
	LASeRMem_DecBase(bs, sh, (LASeRNode *)text);
	LASeRMem_DecString(bs, sh, &(text->string));
	text->color = BS_ReadInt(bs, sh->colorIndexBits);
	text->horizAlign = BS_ReadInt(bs, 2);
	text->vertAlign = BS_ReadInt(bs, 2);
	text->wrap = BS_ReadBit(bs);
	text->displayRectX = BS_ReadInt(bs, 12);
	text->hasDisplayRect = BS_ReadBit(bs);
	if (text->hasDisplayRect) text->displayRectY = BS_ReadInt(bs, 12);
	text->face = BS_ReadInt(bs, sh->fontIndexBits);
	text->style = BS_ReadInt(bs, 4);
	text->size = BS_ReadInt(bs, 12);
}

static void LASeRMem_DecBitmap(BitStream *bs, LASeRSceneHeader *sh, LASeRBitmap *bitmap)
{
	LASeRMem_DecBase(bs, sh, (LASeRNode *)bitmap);
	bitmap->streamID = BS_ReadInt(bs, 8);
}

static void LASeRMem_DecConditional(BitStream *bs, LASeRSceneHeader *sh, LASeRConditional *conditional)
{
	u16 frameLength;	
	LASeRMem_DecBase(bs, sh, (LASeRNode *)conditional);
	conditional->startTimePresent = BS_ReadBit(bs);
	if (conditional->startTimePresent) conditional->startTime = BS_ReadInt(bs, 32);
	BS_Align(bs);
	frameLength = BS_ReadInt(bs, 16);
	LASeRMem_DecUpdates(bs, sh, &(conditional->updates), &(conditional->nbUpdates));
	BS_Align(bs);
}

static void LASeRMem_DecAction(BitStream *bs, LASeRSceneHeader *sh, LASeRAction *action)
{
	u8 moreKeys;
	u32 i;
	LASeRMem_DecBase(bs, sh, (LASeRNode *)action);
	action->hasStartTime = BS_ReadBit(bs);
	if (action->hasStartTime) action->startTime = BS_ReadInt(bs, 32);

	// TODO: add the keycodes to the structure
	do {
		u8 keyCode;
		keyCode = BS_ReadInt(bs, 8);
		moreKeys = BS_ReadBit(bs);
	} while(moreKeys);

	action->hasUrl = BS_ReadBit(bs);
	if (action->hasUrl) LASeRMem_DecString(bs, sh, &(action->url));

	action->nbUrlParams = BS_ReadInt(bs, sh->lenBits);
	action->urlParams = malloc(sizeof(LASeRUrlParam) * action->nbUrlParams);
	for (i = 0; i < action->nbUrlParams; i++) LASeRMem_DecUrlParam(bs, sh, &(action->urlParams[i]));

	action->nbActionKeys = BS_ReadInt(bs, sh->lenBits);
	action->actionKeys = malloc(sizeof(LASeRActionKey) * action->nbActionKeys);
	for (i = 0; i < action->nbActionKeys; i++) LASeRMem_DecActionKey(bs,sh, &(action->actionKeys[i]));
}

static void LASeRMem_DecAnimateTransform(BitStream *bs, LASeRSceneHeader *sh, LASeRAnimateTransform *animt) 
{
	u32 i;

	LASeRMem_DecBase(bs, sh, (LASeRNode *)animt);
	animt->target = BS_ReadInt(bs, sh->idBits);
	animt->hasStartTime = BS_ReadBit(bs);
	if (animt->hasStartTime) animt->startTime = BS_ReadInt(bs, 32);

	animt->repeat = BS_ReadBit(bs);
	animt->nbKeys = BS_ReadInt(bs, sh->lenBits);
	animt->keys = malloc(sizeof(LASeRTransformKey)*animt->nbKeys);
	for (i = 0; i <animt->nbKeys; i++) {
		animt->keys[i].duration = BS_ReadInt(bs, 32);
		animt->keys[i].hasMatrix = BS_ReadBit(bs);
		if (animt->keys[i].hasMatrix) {
			LASeRMem_DecMatrix(bs, sh, &(animt->keys[i].m));
		} else {
			animt->keys[i].deltax = BS_ReadInt(bs, 12);
			animt->keys[i].deltay = BS_ReadInt(bs, 12);
		}
	}
}

static void LASeRMem_DecAnimateColor(BitStream *bs, LASeRSceneHeader *sh, LASeRAnimateColor *animc) 
{
	u32 i;
	LASeRMem_DecBase(bs, sh, (LASeRNode *)animc);
	animc->target = BS_ReadInt(bs, sh->idBits);
	animc->hasStartTime = BS_ReadBit(bs);
	if (animc->hasStartTime) animc->startTime = BS_ReadInt(bs, 32);
	animc->repeat = BS_ReadBit(bs);
	animc->nbKeys = BS_ReadInt(bs, sh->lenBits);
	animc->keys = malloc(sizeof(LASeRColorKey) * animc->nbKeys);
	for (i = 0; i <animc->nbKeys; i++) {
		animc->keys[i].duration = BS_ReadInt(bs, 32);
		animc->keys[i].colorIndex = BS_ReadInt(bs, sh->colorIndexBits);
	}
}

static void LASeRMem_DecAnimateActivate(BitStream *bs, LASeRSceneHeader *sh, LASeRAnimateActivate *anima) 
{
	u32 i;
	LASeRMem_DecBase(bs, sh, (LASeRNode *)anima);
	anima->target = BS_ReadInt(bs, sh->idBits);
	anima->hasStartTime = BS_ReadBit(bs);
	if (anima->hasStartTime) anima->startTime = BS_ReadInt(bs, 32);
	anima->repeat = BS_ReadBit(bs);
	anima->nbKeys = BS_ReadInt(bs, sh->lenBits);
	anima->keys = malloc(sizeof(LASeRActivateKey)*anima->nbKeys);
	for (i = 0; i <anima->nbKeys; i++) {
		anima->keys[i].duration = BS_ReadInt(bs, 32);
		anima->keys[i].animObject = BS_ReadInt(bs, sh->idBits);
	}
}

static void LASeRMem_DecCursor(BitStream *bs, LASeRSceneHeader *sh, LASeRCursor *cursor) 
{
	u32 i;
	LASeRMem_DecBase(bs, sh, (LASeRNode *)cursor);
	cursor->target = BS_ReadInt(bs, sh->idBits);
	cursor->nbKeys = BS_ReadInt(bs, sh->lenBits);
	cursor->keys = malloc(sizeof(LASeRKey) * cursor->nbKeys);
	for (i = 0; i <cursor->nbKeys; i++) {
		cursor->keys[i].keyCode = BS_ReadInt(bs, 8);
		cursor->keys[i].encodedMoveX = BS_ReadInt(bs, 12);
		cursor->keys[i].encodedMoveY = BS_ReadInt(bs, 12);
	}
}

static void LASeRMem_DecTextInput(BitStream *bs, LASeRSceneHeader *sh, LASeRTextInput *textinput) 
{
	LASeRMem_DecBase(bs, sh, (LASeRNode *)textinput);
	textinput->text = BS_ReadInt(bs, sh->idBits);
	textinput->hasTitle = BS_ReadBit(bs);
	if (textinput->hasTitle) {
		LASeRMem_DecString(bs, sh, &(textinput->title));
	}
}

static void *LASeRMem_DecNode(BitStream *bs, LASeRSceneHeader *sh) 
{
	void *current_node;
	u8 node_type = BS_ReadInt(bs, 5);
	switch (node_type) {
	case 0:
		current_node = malloc(sizeof(LASeRTransform));
		memset(current_node, 0 , sizeof(LASeRTransform));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		((LASeRTransform *)current_node)->children = NewChain();
		LASeRMem_DecTransform(bs, sh, (LASeRTransform *)current_node);
		break;
	case 1: 
		current_node = malloc(sizeof(LASeRShape));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		LASeRMem_DecShape(bs, sh, (LASeRShape *)current_node);
		break;
	case 2:
		current_node = malloc(sizeof(LASeRAction));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
 		LASeRMem_DecAction(bs, sh, (LASeRAction *)current_node);
		break;
	case 3:
		current_node = malloc(sizeof(LASeRVideo));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		LASeRMem_DecVideo(bs, sh, (LASeRVideo *)current_node);
		break;
	case 4:
		current_node = malloc(sizeof(LASeRAudio));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		LASeRMem_DecAudio(bs, sh, (LASeRAudio *)current_node);
		break;
	case 5:
		current_node = malloc(sizeof(LASeRBitmap));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		LASeRMem_DecBitmap(bs, sh, (LASeRBitmap *)current_node);
		break;
	case 6:
		current_node = malloc(sizeof(LASeRText));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		LASeRMem_DecText(bs, sh, (LASeRText *)current_node);
		break;
	case 7:
		current_node = malloc(sizeof(LASeRAnimateTransform));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		LASeRMem_DecAnimateTransform(bs, sh, (LASeRAnimateTransform *)current_node);
		break;
	case 8:
		current_node = malloc(sizeof(LASeRAnimateColor));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		LASeRMem_DecAnimateColor(bs, sh, (LASeRAnimateColor *)current_node);
		break;
	case 9:
		current_node = malloc(sizeof(LASeRAnimateActivate));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		LASeRMem_DecAnimateActivate(bs, sh, (LASeRAnimateActivate *)current_node);
		break;
	case 10:
		current_node = malloc(sizeof(LASeRUse));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		LASeRMem_DecUse(bs, sh, (LASeRUse *)current_node);
		break;
	case 11:
		current_node = malloc(sizeof(LASeRConditional));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		LASeRMem_DecConditional(bs, sh, (LASeRConditional *)current_node);
		break;
	case 12:
		current_node = malloc(sizeof(LASeRTextInput));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		LASeRMem_DecTextInput(bs, sh, (LASeRTextInput *)current_node);
		break;
	case 13:
		current_node = malloc(sizeof(LASeRCursor));
		((LASeRNode *)current_node)->sgprivate->tag = node_type;
		LASeRMem_DecCursor(bs, sh, (LASeRCursor *)current_node);
		break;
	}
	return current_node;
}

static void LASeRMem_DecInsert(BitStream *bs, LASeRSceneHeader *sh, LASeRInsert *insert) 
{
	insert->target = BS_ReadInt(bs, sh->idBits);
	insert->node = LASeRMem_DecNode(bs, sh);
}

static void LASeRMem_DecDelete(BitStream *bs, LASeRSceneHeader *sh, LASeRDelete *d) 
{
	d->target = BS_ReadInt(bs, sh->idBits);
}

static void LASeRMem_DecReplace(BitStream *bs, LASeRSceneHeader *sh, LASeRReplace *r) 
{
	r->target = BS_ReadInt(bs, sh->idBits);
	r->node = LASeRMem_DecNode(bs, sh);
}

static void LASeRMem_DecReplaceIndexed(BitStream *bs, LASeRSceneHeader *sh, LASeRReplaceIndexed *ri) 
{
	ri->index = BS_ReadInt(bs, sh->lenBits);
	ri->target = BS_ReadInt(bs, sh->idBits);
	ri->node = LASeRMem_DecNode(bs, sh);
}

static void LASeRMem_DecDeleteIndexed(BitStream *bs, LASeRSceneHeader *sh, LASeRDeleteIndexed *di) 
{
	di->index = BS_ReadInt(bs, sh->lenBits);
	di->target = BS_ReadInt(bs, sh->idBits);
}

static void LASeRMem_DecReplaceField(BitStream *bs, LASeRSceneHeader *sh, LASeRReplaceField *rf) 
{
	u8 floatLen;
	rf->fieldIndex = BS_ReadInt(bs, 5);
	rf->target = BS_ReadInt(bs, sh->idBits);
	rf->isIndexed = BS_ReadBit(bs);
	if (rf->isIndexed) rf->index = BS_ReadInt(bs, sh->lenBits);
	switch (rf->fieldIndex) {
	case 0:
    case 1:
		floatLen = BS_ReadInt(bs, 4);
		rf->floatEncodedValue1 = BS_ReadInt(bs, floatLen + 9);
		break;
    case 2:
    case 3:
    case 4:
		floatLen = BS_ReadInt(bs, 4);
		rf->floatEncodedValue1 = BS_ReadInt(bs, floatLen + 9);
		floatLen = BS_ReadInt(bs, 4);
		rf->floatEncodedValue2 = BS_ReadInt(bs, floatLen + 9);
		break;
    case 5:
		LASeRMem_DecMatrix(bs, sh, &(rf->matrixvalue));
		break;
    case 6:
		rf->nodevalue = LASeRMem_DecNode(bs, sh);
		break;
    case 7:
    case 8:
    case 9:
		rf->idvalue = BS_ReadInt(bs, sh->idBits);
		break;
    case 10:
		LASeRMem_DecString(bs, sh, &(rf->stringvalue));
		break;
    case 11:
    case 12:
		rf->colIndex = BS_ReadInt(bs, sh->colorIndexBits);
		break;
    case 13:
    case 14:
    case 17:
		rf->booleanvalue = BS_ReadBit(bs);
		break;
    case 15:
		rf->timevalue = BS_ReadInt(bs, 32);
		break;
    case 16:
		rf->intvalue = BS_ReadInt(bs, sh->lenBits);
		break;
	}
}

static void LASeRMem_DecBackground(BitStream *bs, LASeRSceneHeader *sh, LASeRBackground *b) 
{
	b->isImage = BS_ReadBit(bs);
	if (b->isImage) b->streamID = BS_ReadInt(bs, 8);
	else b->colIndex = BS_ReadInt(bs, sh->colorIndexBits);
}

static void LASeRMem_DecClean(BitStream *bs, LASeRSceneHeader *sh, LASeRClean *clean) 
{
	clean->serviceID = BS_ReadInt(bs, 12);
	clean->groupID = BS_ReadInt(bs, 8);
}

static void LASeRMem_DecSave(BitStream *bs, LASeRSceneHeader *sh, LASeRSave *save) 
{
	u32 i;
	save->serviceID = BS_ReadInt(bs, 12);
	save->groupID = BS_ReadInt(bs, 8);
	save->nbIds = BS_ReadInt(bs, sh->lenBits);
	save->ids = malloc(sizeof(u32)*save->nbIds);
	for (i =0; i<save->nbIds; i++) save->ids[i] = BS_ReadInt(bs, sh->idBits);
}

static void LASeRMem_DecRestore(BitStream *bs, LASeRSceneHeader *sh, LASeRRestore *restore) 
{
	restore->serviceID = BS_ReadInt(bs, 12);
	restore->groupID = BS_ReadInt(bs, 8);
}

static void LASeRMem_DecUpdates(BitStream *bs, LASeRSceneHeader *sh, LASeRUpdate **updates, u32 *nbUpdates) 
{
	u32 i;

	*nbUpdates = BS_ReadInt(bs, sh->lenBits);
	*updates = (LASeRUpdate *)malloc(sizeof(LASeRUpdate)*(*nbUpdates));
	for (i = 0; i < *nbUpdates; i++) {
		(*updates)[i].type = BS_ReadInt(bs, 5);
		switch ((*updates)[i].type) {
		case 0:
			(*updates)[i].update = malloc(sizeof(LASeRInsert));
			LASeRMem_DecInsert(bs, sh, (LASeRInsert *)(*updates)[i].update);
			break;
		case 1:
			(*updates)[i].update = malloc(sizeof(LASeRDelete));
			LASeRMem_DecDelete(bs, sh, (LASeRDelete *)(*updates)[i].update);
			break;
		case 2:
			(*updates)[i].update = malloc(sizeof(LASeRReplace));
			LASeRMem_DecReplace(bs, sh, (LASeRReplace *)(*updates)[i].update);
			break;
		case 3:
			(*updates)[i].update = malloc(sizeof(LASeRReplaceIndexed));
			LASeRMem_DecReplaceIndexed(bs, sh, (LASeRReplaceIndexed *)(*updates)[i].update);
			break;
		case 4:
			break;
		case 5:
			(*updates)[i].update = malloc(sizeof(LASeRDeleteIndexed));
			LASeRMem_DecDeleteIndexed(bs, sh, (LASeRDeleteIndexed *)(*updates)[i].update);
			break;
		case 6:
			(*updates)[i].update = malloc(sizeof(LASeRReplaceField));
			LASeRMem_DecReplaceField(bs, sh, (LASeRReplaceField *)(*updates)[i].update);
			break;
		case 7:
			(*updates)[i].update = malloc(sizeof(LASeRBackground));
			LASeRMem_DecBackground(bs, sh, (LASeRBackground *)(*updates)[i].update);
			break;
		case 8:
			(*updates)[i].update = malloc(sizeof(LASeRClean));
			LASeRMem_DecClean(bs, sh, (LASeRClean *)(*updates)[i].update);
			break;
		case 9:
			(*updates)[i].update = malloc(sizeof(LASeRSave));
			LASeRMem_DecSave(bs, sh, (LASeRSave *)(*updates)[i].update);
			break;
		case 10:
			(*updates)[i].update = malloc(sizeof(LASeRRestore));
			LASeRMem_DecRestore(bs, sh, (LASeRRestore *)(*updates)[i].update);
			break;
		}
	}
}


M4Err LASeRMem_DecSceneUnit(LPLASERDEC codec, char *data, u32 data_length, LASeRSceneUnit *sceneunit) 
{
	u32 i;
	u8 colorPresent, fontPresent;
	BitStream *bs;
	M4Err e = M4OK;
	LASeRSceneHeader *sh = codec->config;

	if (!codec || !data || codec->dec_memory_mode) return M4BadParam;

	bs = NewBitStream(data, data_length, BS_READ);

	/*setup current scene graph*/
	codec->current_graph = codec->scenegraph;

	colorPresent = BS_ReadBit(bs);
	if (colorPresent) {
		u8 newNbColors = BS_ReadInt(bs, 8);
		sh->nbColors += newNbColors; 
		if (sh->colors) sh->colors = realloc(sh->colors, sh->nbColors * 3);
		else {
			sh->colors = malloc(sh->nbColors * 3);
			memset(sh->colors, 0, sh->nbColors * 3);
		}
		for (i = sh->nbColors - newNbColors; i < sh->nbColors; i++) {
			sh->colors[i].r = BS_ReadInt(bs, sh->colorBits);
			sh->colors[i].g = BS_ReadInt(bs, sh->colorBits);
			sh->colors[i].b = BS_ReadInt(bs, sh->colorBits);
		}
		{
			u32 n = sh->nbColors + 1;
			sh->colorIndexBits = 0;
			while (n>0) { n = n>>1; sh->colorIndexBits++; } 
		}
	} 
	fontPresent = BS_ReadBit(bs);
	if (fontPresent) {
		u8 newNbFonts = BS_ReadInt(bs, 8);
		sh->nbFonts += newNbFonts; 
		if (sh->fontNames) sh->fontNames = realloc(sh->fontNames, sizeof(LASeRString));
		else {
			sh->fontNames = malloc(sh->nbFonts * sizeof(LASeRString));
			memset(sh->fontNames, 0, sh->nbFonts * sizeof(LASeRString));
		}
		for (i= 0; i<sh->nbFonts; i++) {
			LASeRMem_DecString(bs, sh, &(sh->fontNames[i]));
		}
		{
			u32 n = sh->nbFonts + 1;
			sh->fontIndexBits = 0;
			while (n>0) { n = n>>1; sh->fontIndexBits++; } 
		}
	}
	LASeRMem_DecUpdates(bs, sh, &(sceneunit->updates), &(sceneunit->nbUpdates));

	DeleteBitStream(bs);
	return e;

}

