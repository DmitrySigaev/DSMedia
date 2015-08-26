#include "m4_laser_dev.h"

#ifdef M4_USE_LASeR

#include <gpac/intern/m4_scenegraph_dev.h>

void LASeR_DumpNode(FILE *xml, void *current_node);
void dumpUpdates(FILE *xml, LASeRUpdate *updates, u32 nbUpdates);

void dumpMatrix(FILE *xml, LASeRMatrix m) 
{
	fprintf(xml, "matrix=\"%f %f %f %f %f %f\" ", m.xx, m.xy, m.yx, m.yy, m.xz, m.yz);		
}

void dumpPointSequence(FILE *xml, LASeRPointSequence pointseq)
{
	u32 i;
	fprintf(xml, "points=\"");
	for (i = 0; i<pointseq.nbPoints; i++) fprintf(xml, "%f %f ", pointseq.x[i], pointseq.y[i]);
	fprintf(xml, "\" ");
}

void dumpUrlParam(FILE *xml, LASeRUrlParam urlParam)
{
	fprintf(xml, "<UrlParam ");
	fprintf(xml, "name=\"%s\" ", urlParam.name.string);
	fprintf(xml, "source=\"%d\" ", urlParam.source);
	fprintf(xml, "/>\n");
}

void dumpActionKey(FILE *xml, LASeRActionKey actionKey)
{
	fprintf(xml, "<ActionKey ");
	if (actionKey.hasHideObject) {
		fprintf(xml, "hideObject=\"%d\" ", actionKey.hideObject);
	}
	if (actionKey.hasShowObject) {
		fprintf(xml, "showObject=\"%d\" ", actionKey.showObject);
	}
	if (actionKey.hasAnimObject) {
		fprintf(xml, "animObject=\"%d\" ", actionKey.animObject);
	}
	if (actionKey.hasMoveObject) {
		dumpMatrix(xml, actionKey.moveObject);
		fprintf(xml, "absolute=\"%s\" ", (actionKey.absolute ? "true":"false"));
	}
	if (actionKey.hasPointer) {		
		fprintf(xml, "pointerCenterX=\"%f\" ", actionKey.pointerCenterX);
		fprintf(xml, "pointerCenterY=\"%f\" ", actionKey.pointerCenterY);
		fprintf(xml, "pointerSizeX=\"%f\" ", actionKey.pointerSizeX);
		fprintf(xml, "pointerSizeY=\"%f\" ", actionKey.pointerSizeY);
	}
	if (actionKey.hasTextToChange) {
		fprintf(xml, "textToChange=\"%d\" ", actionKey.textToChange);
		fprintf(xml, "newText=\"%s\" ", actionKey.newText.string);
	}
	if (actionKey.hasConcat) {
		fprintf(xml, "concatReadID=\"%d\" ", actionKey.concatReadID);
		fprintf(xml, "concatWriteID=\"%d\" ", actionKey.concatWriteID);
	}
	fprintf(xml, "/>\n");
}

void dumpBase(FILE *xml, LASeRNode node) 
{
	if (node.sgprivate->NodeID) fprintf(xml, "id=\"%d\" ", node.sgprivate->NodeID);
	if (!node.sgprivate->active) fprintf(xml, "active=\"false\" ");
}

void dumpTransform(FILE *xml, LASeRTransform t) 
{
	u32 i;

	fprintf(xml, "<Transform ");
	dumpBase(xml, *(LASeRNode *)&t);
	dumpMatrix(xml, t.m);
	if (t.hasChoice) fprintf(xml, "choice=\"%d\" ", t.choice);
	if (t.hasDelta) fprintf(xml, "delta=\"%f %f\" ", t.deltax, t.deltay);
	if (ChainGetCount(t.children) > 0) {
		fprintf(xml, ">\n");
		for (i=0; i<ChainGetCount(t.children); i++) LASeR_DumpNode(xml, ChainGetEntry(t.children,i));
		fprintf(xml, "</Transform>\n");
	} else {
		fprintf(xml, "/>\n");
	}
}

void dumpShape(FILE *xml, LASeRShape shape)
{
	u32 i;

	fprintf(xml, "<Shape ");
	dumpBase(xml, *(LASeRNode *)&shape);
	fprintf(xml, "filled=\"%s\" ",(shape.filled?"true":"false"));
	if (shape.filled) fprintf(xml, "fill=\"%f %f %f\" ",shape.fill.r, shape.fill.g, shape.fill.b);
	if (shape.stroked) fprintf(xml, "linecolor=\"%f %f %f\" ",shape.linecolor.r, shape.linecolor.g, shape.linecolor.b);
	dumpPointSequence(xml, shape.initpointseq);
	if (shape.width != 1) fprintf(xml, "width=\"%f\" ",shape.width);
	if (shape.nbAdditionalSeqs) {		
		fprintf(xml, ">\n");
		for (i =0; i<shape.nbAdditionalSeqs; i++) {
			fprintf(xml, "<seq ");
			dumpPointSequence(xml, shape.morepointseqs[i]);
			fprintf(xml, "/>\n");
		}
		fprintf(xml, "</Shape>\n");
	} else {
		fprintf(xml, "/>\n");
	}
}

void dumpVideo(FILE *xml, LASeRVideo video)
{
	fprintf(xml, "<Video ");
	dumpBase(xml, *(LASeRNode *)&video);
	fprintf(xml, "repeat=\"%d\" ", video.repeat);
	if (video.hasStartTime) fprintf(xml, "startTime=\"%d\" ", video.startTime);
	fprintf(xml, "streamID=\"%d\" ", video.streamID);
	fprintf(xml, "width=\"%f\" ", video.width );
	fprintf(xml, "height=\"%f\" ", video.height );
	fprintf(xml, "/>\n");
}

void dumpAudio(FILE *xml, LASeRAudio audio)
{
	fprintf(xml, "<Audio ");
	dumpBase(xml, *(LASeRNode *)&audio);
	fprintf(xml, "repeat=\"%d\" ", audio.repeat);
	if (audio.hasStartTime) fprintf(xml, "startTime=\"%d\" ", audio.startTime);
	fprintf(xml, "streamID=\"%d\"/>\n", audio.streamID);
}

void dumpUse(FILE *xml, LASeRUse use)
{
	fprintf(xml, "<Use ");
	dumpBase(xml, *(LASeRNode *)&use);
	fprintf(xml, "target=\"%d\"/>\n", use.used_node->sgprivate->NodeID-1);
}

void dumpText(FILE *xml, LASeRText text)
{
	fprintf(xml, "<Text ");
	dumpBase(xml, *(LASeRNode *)&text);
	fprintf(xml, "string=\"%s\" ", text.string.string);
	fprintf(xml, "color=\"%f %f %f\" ", text.color.r, text.color.g, text.color.b);
	fprintf(xml, "horizAlign=\"%d\" ", text.horizAlign);
	fprintf(xml, "vertAlign=\"%d\" ", text.vertAlign);
	fprintf(xml, "wrap=\"%s\" ", (text.wrap?"true":"false"));
	fprintf(xml, "displayRectX=\"%f\" ", text.displayRectX);
	if (text.hasDisplayRect) fprintf(xml, "displayRectY=\"%f\" ", text.displayRectY);
	fprintf(xml, "face=\"%s\" ", text.face->string);
	fprintf(xml, "style=\"%d\" ", text.style);
	fprintf(xml, "size=\"%f\" ", text.size);
	fprintf(xml, "/>\n");
}

void dumpBitmap(FILE *xml, LASeRBitmap bitmap)
{
	fprintf(xml, "<Bitmap ");
	dumpBase(xml, *(LASeRNode *)&bitmap);
	fprintf(xml, "streamID=\"%d\" ", bitmap.streamID);
	fprintf(xml, "/>\n");
}

void dumpConditional(FILE *xml, LASeRConditional conditional)
{	
	fprintf(xml, "<Conditional ");
	dumpBase(xml, *(LASeRNode *)&conditional);
	if (conditional.hasStartTime) fprintf(xml, "startTime=\"%d\" ", conditional.startTime);
	fprintf(xml, ">\n");
	dumpUpdates(xml, conditional.updates, conditional.nbUpdates);
	fprintf(xml, "</Conditional>\n");
}

void dumpAction(FILE *xml, LASeRAction action)
{
	u32 i;

	fprintf(xml, "<Action ");
	dumpBase(xml, *(LASeRNode *)&action);
	if (action.hasStartTime) fprintf(xml, "startTime=\"%d\" ", action.startTime);
	fprintf(xml, ">\n");
	if (action.hasUrl) fprintf(xml, "<Url>%s</Url>\n", action.url.string);
	for (i = 0; i < action.nbUrlParams; i++) dumpUrlParam(xml, action.urlParams[i]);
	for (i = 0; i < action.nbActionKeys; i++) dumpActionKey(xml, action.actionKeys[i]);
	fprintf(xml, "</Action>\n");
}

void dumpAnimateTransform(FILE *xml, LASeRAnimateTransform animt) 
{
	u32 i;

	fprintf(xml, "<AnimateTransform ");
	dumpBase(xml, *(LASeRNode *)&animt);
	fprintf(xml, "target=\"%d\" ", animt.target);
	if (animt.hasStartTime) fprintf(xml, "startTime=\"%d\" ", animt.startTime);
	fprintf(xml, "repeat=\"%d\" ", animt.repeat);
	fprintf(xml, ">\n");
	for (i = 0; i <animt.nbKeys; i++) {
		fprintf(xml, "<Key ");
		fprintf(xml, "duration=\"%d\" ", animt.durations[i]);
		if (animt.keys[i].hasMatrix) {
			dumpMatrix(xml, animt.keys[i].m);
		} else {
			fprintf(xml, "delta=\"%d %d\" ", animt.keys[i].deltax, animt.keys[i].deltay);
		}
		fprintf(xml, "/>\n");
	}
	fprintf(xml, "</AnimateTransform>\n");
}

void dumpAnimateColor(FILE *xml, LASeRAnimateColor animc) 
{
	u32 i;

	fprintf(xml, "<AnimateColor ");
	dumpBase(xml, *(LASeRNode *)&animc);
	fprintf(xml, "target=\"%d\" ", animc.target);
	if (animc.hasStartTime) fprintf(xml, "startTime=\"%d\" ", animc.startTime);
	fprintf(xml, "repeat=\"%d\" ", animc.repeat);
	fprintf(xml, ">\n");
	for (i = 0; i <animc.nbKeys; i++) {
		fprintf(xml, "<Key ");
		fprintf(xml, "duration=\"%d\" ", animc.durations[i]);
		fprintf(xml, "color=\"%f %f %f \" ", animc.keys[i].color.r, animc.keys[i].color.g, animc.keys[i].color.b);
		fprintf(xml, "/>\n");
	}
	fprintf(xml, "</AnimateColor>\n");
}

void dumpAnimateActivate(FILE *xml, LASeRAnimateActivate anima) 
{
	u32 i;

	fprintf(xml, "<AnimateActivate ");
	dumpBase(xml, *(LASeRNode *)&anima);
	fprintf(xml, "target=\"%d\" ", anima.target);
	if (anima.hasStartTime) fprintf(xml, "startTime=\"%d\" ", anima.startTime);
	fprintf(xml, "repeat=\"%d\" ", anima.repeat);
	fprintf(xml, ">\n");
	for (i = 0; i <anima.nbKeys; i++) {
		fprintf(xml, "<Key ");
		fprintf(xml, "duration=\"%d\" ", anima.durations[i]);
		fprintf(xml, "animObject=\"%d\" ", anima.keys[i].animObject);
		fprintf(xml, "/>\n");
	}
	fprintf(xml, "</AnimateActivate>\n");
}

void dumpCursor(FILE *xml, LASeRCursor cursor) 
{
	u32 i;

	fprintf(xml, "<Cursor ");
	dumpBase(xml, *(LASeRNode *)&cursor);
	fprintf(xml, "target=\"%d\" ", cursor.target);
	fprintf(xml, ">\n");
	for (i = 0; i <cursor.nbKeys; i++) {
		fprintf(xml, "<Key ");
		fprintf(xml, "keyCode=\"%d\" ", cursor.keys[i].keyCode);
		fprintf(xml, "MoveX=\"%f\" ", cursor.keys[i].MoveX);
		fprintf(xml, "MoveY=\"%f\" ", cursor.keys[i].MoveY);
		fprintf(xml, "/>\n");
	}
	fprintf(xml, "</Cursor>\n");
}

void dumpTextInput(FILE *xml, LASeRTextInput textinput) 
{
	fprintf(xml, "<TextInput ");
	dumpBase(xml, *(LASeRNode *)&textinput);
	fprintf(xml, "target=\"%d\" ", textinput.text);
	if (textinput.hasTitle) fprintf(xml, "title=\"%s\" ", textinput.title.string);
	fprintf(xml, "/>\n");
}

void LASeR_DumpNode(FILE *xml, void *current_node) 
{
	switch (((LASeRNode *)current_node)->sgprivate->tag) {
	case 0:
		dumpTransform(xml, *(LASeRTransform *)current_node);
		break;
	case 1: 
		dumpShape(xml, *(LASeRShape *)current_node);
		break;
	case 2:
 		dumpAction(xml, *(LASeRAction *)current_node);
		break;
	case 3:
		dumpVideo(xml, *(LASeRVideo *)current_node);
		break;
	case 4:
		dumpAudio(xml, *(LASeRAudio *)current_node);
		break;
	case 5:
		dumpBitmap(xml, *(LASeRBitmap *)current_node);
		break;
	case 6:
		dumpText(xml, *(LASeRText *)current_node);
		break;
	case 7:
		dumpAnimateTransform(xml, *(LASeRAnimateTransform *)current_node);
		break;
	case 8:
		dumpAnimateColor(xml, *(LASeRAnimateColor *)current_node);
		break;
	case 9:
		dumpAnimateActivate(xml, *(LASeRAnimateActivate *)current_node);
		break;
	case 10:
		dumpUse(xml, *(LASeRUse *)current_node);
		break;
	case 11:
		dumpConditional(xml, *(LASeRConditional *)current_node);
		break;
	case 12:
		dumpTextInput(xml, *(LASeRTextInput *)current_node);
		break;
	case 13:
		dumpCursor(xml, *(LASeRCursor *)current_node);
		break;
	}
}

void dumpInsert(FILE *xml, LASeRInsert insert) 
{
	fprintf(xml, "<Insert target=\"%d\">\n", insert.target);
	LASeR_DumpNode(xml, insert.node);
	fprintf(xml, "</Insert>\n");
}

void dumpDelete(FILE *xml, LASeRDelete d) 
{
	fprintf(xml, "<Delete target=\"%d\"/>\n", d.target);	
}

void dumpReplace(FILE *xml, LASeRReplace r) 
{
	fprintf(xml, "<Replace target=\"%d\">\n", r.target);
	LASeR_DumpNode(xml, r.node);
	fprintf(xml, "</Replace>\n");
}

void dumpReplaceIndexed(FILE *xml, LASeRReplaceIndexed ri) 
{
	fprintf(xml, "<Replace target=\"%d\" index=\"%d\">\n", ri.target, ri.index);
	LASeR_DumpNode(xml, ri.node);
	fprintf(xml, "</Replace>\n");
}

void dumpDeleteIndexed(FILE *xml, LASeRDeleteIndexed di) 
{
	fprintf(xml, "<Delete target=\"%d\" index=\"%d\"/>\n", di.target, di.index);
}

void dumpReplaceField(FILE *xml, LASeRReplaceField rf) 
{
	fprintf(xml, "<Replace target=\"%d\" field=\"%d\" ", rf.target, rf.fieldIndex);
	if (rf.isIndexed) fprintf(xml, "index=\"%d\" ", rf.index);
	switch (rf.fieldIndex) {
	case 0:
    case 1:
		fprintf(xml, "floatvalue=\"%d\"/>\n", rf.floatEncodedValue1);
		break;
    case 2:
    case 3:
    case 4:
		fprintf(xml, "floatvalue1=\"%d\" ", rf.floatEncodedValue1);
		fprintf(xml, "floatvalue2=\"%d\"/>\n", rf.floatEncodedValue2);
		break;
    case 5:
		dumpMatrix(xml, rf.matrixvalue);
		fprintf(xml, "/>\n");
		break;
    case 6:
		fprintf(xml, ">\n");
		LASeR_DumpNode(xml, rf.nodevalue);
		fprintf(xml, "</Replace>\n");
		break;
    case 7:
    case 8:
    case 9:
		fprintf(xml, "id=\"%d\"/>\n", rf.idvalue);
		break;
    case 10:
	    fprintf(xml, "stringvalue=\"%s\"/>\n", rf.stringvalue.string);
		break;
    case 11:
    case 12:
		fprintf(xml, "colorIndex=\"%d\"/>\n", rf.colIndex);
		break;
    case 13:
    case 14:
    case 17:
		fprintf(xml, "booleanvalue=\"%d\"/>\n", rf.booleanvalue);
		break;
    case 15:
		fprintf(xml, "timevalue=\"%d\"/>\n", rf.timevalue);
		break;
    case 16:
		fprintf(xml, "intvalue=\"%d\"/>\n", rf.intvalue);
		break;
	}
}

void dumpBackground(FILE *xml, LASeRBackground b) 
{
	if (b.isImage) {
		fprintf(xml, "<Background imagenumber=\"%d\"/>\n", b.streamID);
	} else {
		if (b.color) fprintf(xml, "<Background color=\"%f %f %f\"/>\n", b.color->r, b.color->g, b.color->b);
	}
}

void dumpClean(FILE *xml, LASeRClean clean) 
{
	fprintf(xml, "<Clean serviceID=\"%d\" groupID=\"%d\"/>\n", clean.serviceID, clean.groupID);
}

void dumpSave(FILE *xml, LASeRSave save) 
{
	u32 i;
	fprintf(xml, "<Save serviceID=\"%d\" groupID=\"%d\">\n", save.serviceID, save.groupID);
	for (i =0; i<save.nbIds; i++) fprintf(xml, "%d ", save.ids[i]);
	fprintf(xml, "</Save>\n");
}

void dumpRestore(FILE *xml, LASeRRestore restore) 
{
	fprintf(xml, "<Restore serviceID=\"%d\" groupID=\"%d\"/>\n", restore.serviceID, restore.groupID);
}

void dumpUpdates(FILE *xml, LASeRUpdate *updates, u32 nbUpdates) 
{
	u32 i;

	for (i = 0; i < nbUpdates; i++) {
		switch (updates[i].type) {
		case 0:
			dumpInsert(xml, *(LASeRInsert *)updates[i].update);
			break;
		case 1:
			dumpDelete(xml, *(LASeRDelete *)updates[i].update);
			break;
		case 2:
			dumpReplace(xml, *(LASeRReplace *)updates[i].update);
			break;
		case 3:
			dumpReplaceIndexed(xml, *(LASeRReplaceIndexed *)updates[i].update);
			break;
		case 4:
			break;
		case 5:
			dumpDeleteIndexed(xml, *(LASeRDeleteIndexed *)updates[i].update);
			break;
		case 6:
			dumpReplaceField(xml, *(LASeRReplaceField *)updates[i].update);
			break;
		case 7:
			dumpBackground(xml, *(LASeRBackground *)updates[i].update);
			break;
		case 8:
			dumpClean(xml, *(LASeRClean *)updates[i].update);
			break;
		case 9:
			dumpSave(xml, *(LASeRSave *)updates[i].update);
			break;
		case 10:
			dumpRestore(xml, *(LASeRRestore *)updates[i].update);
			break;
		}
		fflush(xml);
	}
}

void dumpSceneUnit(FILE *xml, LASeRSceneUnit sceneunit) 
{

	fprintf(xml, "<SceneUnit time=\"%d\">\n", sceneunit.time);
/*
	fprintf(xml, "<colors>\n");
	for (i = 0; i < xml.nbColors; i++) {
		fprintf(xml, "<color i=\"%d\" value=\"%d,%d,%d\"/>\n", i, xml.colors[i].r, xml.colors[i].b, xml.colors[i].b);
	}
	fprintf(xml, "</colors>\n");
*/

/*
	fprintf(xml, "<fonts>\n");
	for (i= 0; i<xml.nbFonts; i++) {
		fprintf(xml, "<font i=\"%d\" name=\"%s\"/>\n", i, xml.fontNames[i].string);
	}
	fprintf(xml, "</fonts>\n");
*/
	dumpUpdates(xml, sceneunit.updates, sceneunit.nbUpdates);
	fprintf(xml, "</SceneUnit>\n");
}

void dumpSceneHeader(FILE *xml, LASeRSceneHeader sh) 
{
	fprintf(xml, "<SceneHeader idBits=\"%d\" lenBits=\"%d\" colorBits=\"%d\" resolution=\"%f\" scaleBits=\"%d\" ",sh.idBits, sh.lenBits, sh.colorBits, sh.resolution, sh.scaleBits);
	if (!sh.append)	fprintf(xml, "width=\"%d\" height=\"%d\" ", sh.width, sh.height);
	fprintf(xml, "/>\n");
}

#endif	//M4_USE_LASeR
