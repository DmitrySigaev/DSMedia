/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Scene Graph sub-project
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

/*math.h is not included in main config (C++ clash on win32)*/
#include <math.h>
#include <intern/m4_scenegraph_dev.h>

/*MPEG4 & X3D tags (for node tables & script handling)*/
#include <m4_mpeg4_nodes.h>
#include <m4_x3d_nodes.h>

/*use available tools for vec & rotation ops*/
#include <intern/m4_render_tools.h>

#ifdef M4_USE_SPIDERMONKEY

static JSClass globalClass;
static JSClass browserClass;
static JSClass SFNodeClass;
static JSClass SFVec2fClass;
static JSClass SFVec3fClass;
static JSClass SFRotationClass;
static JSClass SFColorClass;
static JSClass SFImageClass;
static JSClass MFInt32Class;
static JSClass MFBoolClass;
static JSClass MFFloatClass;
static JSClass MFTimeClass;
static JSClass MFVec2fClass;
static JSClass MFVec3fClass;
static JSClass MFRotationClass;
static JSClass MFRotationClass;
static JSClass MFColorClass;
static JSClass MFStringClass;
static JSClass MFUrlClass;
static JSClass MFNodeClass;


static M4INLINE JSInterface *JS_GetInterface(JSContext *c)
{
	SFNode *n = JS_GetContextPrivate(c);
	return n->sgprivate->scenegraph->js_ifce;
}

static M4INLINE JSField *NewJSField()
{
	JSField *ptr;
	SAFEALLOC(ptr, sizeof(JSField));
	return ptr;
}

static M4INLINE M_Script *JS_GetScript(JSContext *c)
{
	return (M_Script *) JS_GetContextPrivate(c);
}
static M4INLINE ScriptPriv *JS_GetScriptStack(JSContext *c)
{
	M_Script *script = (M_Script *) JS_GetContextPrivate(c);
	return script->sgprivate->privateStack;
}

static void script_error(JSContext *c, const char *msg, JSErrorReport *jserr)
{
	JSInterface *ifce = JS_GetInterface(c);
	if (ifce)
		ifce->Error(ifce->callback, msg);
}

static JSBool JSPrint(JSContext *c, JSObject *p, uintN argc, jsval *argv, jsval *rval)
{
	u32 i;
	char buf[5000];
	JSInterface *ifce = JS_GetInterface(c);
	if (!ifce) return JS_FALSE;

	strcpy(buf, "");
	for (i = 0; i < argc; i++) {
		JSString *str = JS_ValueToString(c, argv[i]);
		if (!str) return JS_FALSE;
		if (i) strcat(buf, " ");
		strcat(buf, JS_GetStringBytes(str));
	}
	ifce->Print(ifce->callback, buf);
	return JS_TRUE;
}

static JSFunctionSpec globalFunctions[] = {
    {"print",           JSPrint,          0},
    { 0 }
};

static JSBool getName(JSContext *c, JSObject *obj, uintN n, jsval *v, jsval *rval)
{
	*rval = STRING_TO_JSVAL(JS_InternString(c, "GPAC Browser"));
	return JS_TRUE;
}
static JSBool getVersion(JSContext*c, JSObject*obj, uintN n, jsval *v, jsval *rval)
{
	*rval = STRING_TO_JSVAL(JS_InternString( c, M4_VERSION));
	return JS_TRUE;
}
static JSBool getCurrentSpeed(JSContext *c, JSObject *o, uintN n, jsval *v, jsval *rval)
{
	*rval = DOUBLE_TO_JSVAL(JS_NewDouble( c, 30.0 ));
	return JS_TRUE;
}
static JSBool getCurrentFrameRate(JSContext *c, JSObject*o, uintN n, jsval *v, jsval*rval)
{
	*rval = DOUBLE_TO_JSVAL(JS_NewDouble(c, 30.0 ));
	return JS_TRUE;
}
static JSBool getWorldURL(JSContext*c, JSObject*obj, uintN n, jsval *v, jsval *rval)
{
	const char *name;
	JSInterface *ifce = JS_GetInterface(c);
	if (!ifce) return JS_FALSE;
	name = ifce->GetOption(ifce->callback, "WorldURL");
	*rval = STRING_TO_JSVAL(JS_InternString( c, name ));
	return JS_TRUE;
}
static JSBool replaceWorld(JSContext*c, JSObject*o, uintN n, jsval *v, jsval *rv)
{
	return JS_TRUE;
}
static JSBool addRoute(JSContext*c, JSObject*o, uintN argc, jsval *argv, jsval *rv)
{
	JSField *ptr;
	SFNode *n1, *n2;
	char *f1, *f2;
	FieldInfo info;
	LPROUTE r;
	u32 f_id1, f_id2;
	if (argc!=4) return JS_FALSE;
	if (!JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFNodeClass, NULL) ) return JS_FALSE;
	if (!JSVAL_IS_OBJECT(argv[2]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[2]), &SFNodeClass, NULL) ) return JS_FALSE;
	if (!JSVAL_IS_STRING(argv[1]) || !JSVAL_IS_STRING(argv[3])) return JS_FALSE;

	ptr = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0]));
	assert(ptr->field.fieldType==FT_SFNode);
	n1 = * ((SFNode **)ptr->field.far_ptr);
	ptr = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[2]));
	assert(ptr->field.fieldType==FT_SFNode);
	n2 = * ((SFNode **)ptr->field.far_ptr);
		
	if (!n1 || !n2) return JS_FALSE;
	f1 = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
	f2 = JS_GetStringBytes(JSVAL_TO_STRING(argv[3]));
	if (!f1 || !f2) return JS_FALSE;
	
	if (!strnicmp(f1, "_field", 6)) {
		f_id1 = atoi(f1+6);
		if (Node_GetField(n1, f_id1, &info) != M4OK) return JS_FALSE;
	} else {
		if (Node_GetFieldByName(n1, f1, &info) != M4OK) return JS_FALSE;
		f_id1 = info.fieldIndex;
	}
	if (!strnicmp(f2, "_field", 6)) {
		f_id2 = atoi(f2+6);
		if (Node_GetField(n2, f_id2, &info) != M4OK) return JS_FALSE;
	} else {
		if (Node_GetFieldByName(n2, f2, &info) != M4OK) return JS_FALSE;
		f_id2 = info.fieldIndex;
	}
	r = SG_NewRoute(n1->sgprivate->scenegraph, n1, f_id1, n2, f_id2);
	if (!r) return JS_FALSE;
	return JS_TRUE;
}
static JSBool deleteRoute(JSContext*c, JSObject*o, uintN argc, jsval *argv, jsval *rv)
{
	JSField *ptr;
	SFNode *n1, *n2;
	char *f1, *f2;
	FieldInfo info;
	LPROUTE r;
	u32 f_id1, f_id2, i;
	if (argc!=4) return JS_FALSE;
	if (!JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFNodeClass, NULL) ) return JS_FALSE;
	if (!JSVAL_IS_OBJECT(argv[2]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[2]), &SFNodeClass, NULL) ) return JS_FALSE;
	if (!JSVAL_IS_STRING(argv[1]) || !JSVAL_IS_STRING(argv[3])) return JS_FALSE;

	ptr = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0]));
	assert(ptr->field.fieldType==FT_SFNode);
	n1 = * ((SFNode **)ptr->field.far_ptr);
	ptr = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[2]));
	assert(ptr->field.fieldType==FT_SFNode);
	n2 = * ((SFNode **)ptr->field.far_ptr);
		
	if (!n1 || !n2) return JS_FALSE;
	f1 = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
	f2 = JS_GetStringBytes(JSVAL_TO_STRING(argv[3]));
	if (!f1 || !f2) return JS_FALSE;
	
	if (!strnicmp(f1, "_field", 6)) {
		f_id1 = atoi(f1+6);
		if (Node_GetField(n1, f_id1, &info) != M4OK) return JS_FALSE;
	} else {
		if (Node_GetFieldByName(n1, f1, &info) != M4OK) return JS_FALSE;
		f_id1 = info.fieldIndex;
	}
	if (!strnicmp(f2, "_field", 6)) {
		f_id2 = atoi(f2+6);
		if (Node_GetField(n2, f_id2, &info) != M4OK) return JS_FALSE;
	} else {
		if (Node_GetFieldByName(n2, f2, &info) != M4OK) return JS_FALSE;
		f_id2 = info.fieldIndex;
	}
	for (i=0; i<ChainGetCount(n1->sgprivate->outRoutes); i++) {
		r = ChainGetEntry(n1->sgprivate->outRoutes, i);
		if (r->FromFieldIndex != f_id1) continue;
		if (r->ToNode != n2) continue;
		if (r->ToFieldIndex != f_id2) continue;
		SG_DeleteRoute(r);
		return JS_TRUE;
	}
	return JS_TRUE;
}

static JSBool loadURL(JSContext*c, JSObject*obj, uintN argc, jsval *argv, jsval *rval)
{
	u32 i;
	jsval item;
	jsuint len;
	JSObject *p;
	JSField *f;
	JSInterface *ifce = JS_GetInterface(c);
	if (!ifce) return JS_FALSE;
	if (argc < 1) return JS_FALSE;

	if (JSVAL_IS_STRING(argv[0])) {
		JSString *str = JSVAL_TO_STRING(argv[0]);
		if (ifce->LoadURL(ifce->callback, JS_GetStringBytes(str), NULL, 0))
			return JS_TRUE;
		return JS_FALSE;
	}
	if (!JSVAL_IS_OBJECT(argv[0])) return JS_FALSE;
	
	JS_ValueToObject(c, argv[0], &p);

	f = (JSField *) JS_GetPrivate(c, p);
	if (!f || !f->js_list) return JS_FALSE;
	JS_GetArrayLength(c, f->js_list, &len);

	for (i=0; i<len; i++) {
		JS_GetElement(c, f->js_list, (jsint) i, &item);

		if (JSVAL_IS_STRING(item)) {
			JSString *str = JSVAL_TO_STRING(item);
			if (ifce->LoadURL(ifce->callback, JS_GetStringBytes(str), NULL, 0) )
				return JS_TRUE;
		}
	}
	return JS_TRUE;
}

static JSBool setDescription(JSContext*c, JSObject*o, uintN n, jsval *v, jsval *rv)
{
	return JS_TRUE;
}

void js_on_message(void *cbk, char *msg, M4Err e)
{
	JSInterface *ifce = (JSInterface *) cbk;
	if (e)
		ifce->Error(ifce->callback, msg);
	else
		ifce->Print(ifce->callback, msg);
}

static JSBool createVrmlFromString(JSContext*c, JSObject*obj, uintN argc, jsval *argv, jsval *rval)
{
	ScriptPriv *priv;
	FieldInfo field;
	/*BT/VRML from string*/
	Chain *M4CTX_BTLoadFromString(LPSCENEGRAPH in_scene, char *node_str, void (*OnMessage)(void *cbk, char *msg, M4Err e), void *cbk);
	JSString *js_str;
	char *str;
	Chain *nlist;
	SFNode *sc_node = JS_GetContextPrivate(c);
	JSInterface *ifce = JS_GetInterface(c);
	if (!ifce) return JS_FALSE;
	if (argc < 1) return JS_FALSE;

	if (!JSVAL_IS_STRING(argv[0])) return JS_FALSE;
	js_str = JSVAL_TO_STRING(argv[0]);
	str = JS_GetStringBytes(js_str);
	nlist = M4CTX_BTLoadFromString(sc_node->sgprivate->scenegraph, str, js_on_message, ifce );
	if (!nlist) return JSVAL_NULL;

	priv = JS_GetScriptStack(c);
	memset(&field, 0, sizeof(FieldInfo));
	field.fieldType = FT_MFNode;
	field.far_ptr = &nlist;
	*rval = JS_ToJSField(priv, &field, NULL);

	/*don't forget to unregister all this stuff*/
	while (ChainGetCount(nlist)) {
		SFNode *n = ChainGetEntry(nlist, 0);
		ChainDeleteEntry(nlist, 0);
		Node_Unregister(n, NULL);
	}
	DeleteChain(nlist);
	return JS_TRUE;
}

static JSFunctionSpec browserFunctions[] = {
  {"getName", getName, 0},
  {"getVersion", getVersion, 0},
  {"getCurrentSpeed", getCurrentSpeed, 0},
  {"getCurrentFrameRate", getCurrentFrameRate, 0},
  {"getWorldURL", getWorldURL, 0},
  {"replaceWorld", replaceWorld, 0},
  {"addRoute", addRoute, 0},
  {"deleteRoute", deleteRoute, 0},
  {"loadURL", loadURL, 0},
  {"createVrmlFromString", createVrmlFromString, 0},
  {"setDescription", setDescription, 0},
  {0}
};

void Script_FieldChanged(SFNode *parent, JSField *parent_owner, FieldInfo *field)
{
	ScriptPriv *priv;
	u32 i;
	ScriptField *sf;


	if (!parent) {
		parent = parent_owner->owner;
		field = &parent_owner->field;
	}
	if (!parent) return;

	if ((parent->sgprivate->tag != TAG_MPEG4_Script) && (parent->sgprivate->tag != TAG_X3D_Script) ) {
		if (field->on_event_in) field->on_event_in(parent);
		/*field has changed, set routes...*/
		Node_OnEventOut(parent, field->fieldIndex);
		SG_NodeChanged(parent, field);
		return;
	}
	/*otherwise mark field if eventOut*/
	if (parent_owner) {
		priv = parent_owner->owner->sgprivate->privateStack;
		for (i=0; i<ChainGetCount(priv->fields); i++) {
			sf = ChainGetEntry(priv->fields, i);
			if (sf->ALL_index == field->fieldIndex) {
				/*queue eventOut*/
				if (sf->eventType == ET_EventOut) {
					sf->activate_event_out = 1;
				}
			}
		}
	}
}

JSBool eventOut_setProperty(JSContext *c, JSObject *obj, jsval id, jsval *val)
{
	u32 i;
	const char *eventName;
	ScriptPriv *script;
	SFNode *n;
	FieldInfo info;
	JSString *str = JS_ValueToString(c, id);
	if (!str) return JS_FALSE;
	
	eventName = JS_GetStringBytes(str);

	script = JS_GetScriptStack(c);
	if (!script) return JS_FALSE;
	n = (SFNode *) JS_GetScript(c);

	for (i=0; i<ChainGetCount(script->fields); i++) {
		ScriptField *sf = ChainGetEntry(script->fields, i);
		if (!stricmp(sf->name, eventName)) {
			Node_GetField(n, sf->ALL_index, &info);
			JS_ToNodeField(c, *val, &info, n, NULL);
			sf->activate_event_out = 1;
			return JS_TRUE;
		}
	}
	return JS_FALSE;
}


/*generic ToString method*/
static M4INLINE void sffield_toString(char *str, void *f_ptr, u32 fieldType)
{
	char temp[1000];

	switch (fieldType) {
	case FT_SFVec2f:
	{
		SFVec2f val = * ((SFVec2f *) f_ptr);
		sprintf(temp, "%f %f", val.x, val.y);
		strcat(str, temp);
		break;
	}
	case FT_SFVec3f:
	{
		SFVec3f val = * ((SFVec3f *) f_ptr);
		sprintf(temp, "%f %f %f", val.x, val.y, val.z);
		strcat(str, temp);
		break;
	}
	case FT_SFVec4f:
	{
		SFVec4f val = * ((SFVec4f *) f_ptr);
		sprintf(temp, "%f %f %f %f", val.x, val.y, val.z, val.q);
		strcat(str, temp);
		break;
	}
	case FT_SFRotation:
	{
		SFRotation val = * ((SFRotation *) f_ptr);
		sprintf(temp, "%f %f %f %f", val.xAxis, val.xAxis, val.zAxis, val.angle);
		strcat(str, temp);
		break;
	}
	case FT_SFColor:
	{
		SFColor val = * ((SFColor *) f_ptr);
		sprintf(temp, "%f %f %f", val.red, val.green, val.blue);
		strcat(str, temp);
		break;
	}
	case FT_SFImage:
	{
		SFImage *val = ((SFImage *)f_ptr);
		sprintf(temp, "%dx%dx%d", val->width, val->height, val->numComponents);
		strcat(str, temp);
		break;
	}
	
	}
}

static void JS_ObjectDestroyed(JSContext *c, JSObject *obj)
{
	ScriptPriv *priv = JS_GetScriptStack(c);
	JS_SetPrivate(c, obj, 0);
	ChainDeleteItem(priv->obj_bank, obj);
}


static JSBool field_toString(JSContext *c, JSObject *obj, uintN n, jsval *v, jsval *rval)
{
	u32 i, len;
	jsdouble d;
	char temp[1000];
	char str[5000];
	JSString *s;
	jsval item;
	JSField *f = (JSField *) JS_GetPrivate(c, obj);
	if (!f) return JS_FALSE;

	strcpy(str, "");

	if (VRML_IsSFField(f->field.fieldType)) {
		sffield_toString(str, f->field.far_ptr, f->field.fieldType);
	} else {
		if (f->field.fieldType == FT_MFNode) return JS_TRUE;

		strcat(str, "[");
		JS_GetArrayLength(c, f->js_list, &len);
		for (i=0; i<len; i++) {
			JS_GetElement(c, f->js_list, (jsint) i, &item);
			switch (f->field.fieldType) {
			case FT_MFBool:
				sprintf(temp, "%s", JSVAL_TO_BOOLEAN(item) ? "TRUE" : "FALSE");
				strcat(str, temp);
				break;
			case FT_MFInt32:
				sprintf(temp, "%d", JSVAL_TO_INT(item));
				strcat(str, temp);
				break;
			case FT_MFFloat:
			case FT_MFTime:
				JS_ValueToNumber(c, item, &d);
				sprintf(temp, "%g", d);
				strcat(str, temp);
				break;
			case FT_MFString:
			case FT_MFURL:
			{
				JSString *j_str = JSVAL_TO_STRING(item);
				char *str_val = JS_GetStringBytes(j_str);
				strcat(str, str_val);
			}
				break;
			default:
				if (JSVAL_IS_OBJECT(item)) {
					JSField *sf = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(item));
					sffield_toString(str, sf->field.far_ptr, sf->field.fieldType);
				}
				break;
			}
			if (i<len-1) strcat(str, ", ");
		}
		strcat(str, "]");
	}
	s = JS_NewStringCopyZ(c, str);
	if (!s) return JS_FALSE;
	*rval = STRING_TO_JSVAL(s); 
	return JS_TRUE; 
}



static JSBool SFNodeConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	u32 tag, ID;
	SFNode *new_node;
	JSField *field;
	JSString *str;
	LPPROTO proto;
	LPSCENEGRAPH sg;
	char *node_name;
	M_Script *sc = JS_GetScript(c);
	if (!argc) {
		field = NewJSField();
		field->field.fieldType = FT_SFNode;
		field->temp_node = NULL;
		field->field.far_ptr = &field->temp_node;
		JS_SetPrivate(c, obj, field);
		*rval = OBJECT_TO_JSVAL(obj);
		return JS_TRUE;
	}
	if (!JS_InstanceOf(c, obj, &SFNodeClass, NULL) ) return JS_FALSE;


	str = JS_ValueToString(c, argv[0]);
	if (!str) return JS_FALSE;

	node_name = JS_GetStringBytes(str);
	if (!strnicmp(node_name, "_proto", 6)) {
		ID = atoi(node_name+6);

		/*locate proto in current graph and all parents*/
		sg = sc->sgprivate->scenegraph;
		while (1) {
			proto = SG_FindProto(sg, ID, NULL);
			if (proto) break;
			if (!sg->parent_scene) break;
			sg = sg->parent_scene;
		}
		if (!proto) return JS_FALSE;
		/* create interface and load code in current graph*/
		new_node = Proto_CreateInstance(sc->sgprivate->scenegraph, proto);
		if (!new_node) return JS_FALSE;
		/*OK, instanciate proto code*/
		if (Proto_LoadCode(new_node) != M4OK) {
			Node_Unregister(new_node, NULL);
			return JS_FALSE;
		}
	} else {
		if (sc->sgprivate->tag==TAG_MPEG4_Script) {
			tag = MPEG4_GetTagByName(node_name);
		} else {
			tag = X3D_GetTagByName(node_name);
		}
		if (!tag) return JS_FALSE;
		new_node = SG_NewNode(sc->sgprivate->scenegraph, tag);
		if (!new_node) return JS_FALSE;
		Node_Init(new_node);
	}

	Node_Register(new_node, NULL);
	field = NewJSField();
	field->field.fieldType = FT_SFNode;
	field->temp_node = new_node;
	field->field.far_ptr = &field->temp_node;
	JS_SetPrivate(c, obj, field);
	*rval = OBJECT_TO_JSVAL(obj);

	return JS_TRUE;
}
static void node_finalize(JSContext *c, JSObject *obj)
{
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);
	if (ptr) {
		if (ptr->temp_node) Node_Unregister(ptr->temp_node, ptr->owner);
		free(ptr);
	}
	JS_ObjectDestroyed(c, obj);
}
static JSBool node_getProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	SFNode *n;
	u32 index;
	JSString *str;
	FieldInfo info;
	JSField *ptr;
	if (! JS_InstanceOf(c, obj, &SFNodeClass, NULL) ) return JS_FALSE;
	ptr = (JSField *) JS_GetPrivate(c, obj);
	assert(ptr->field.fieldType==FT_SFNode);
	n = * ((SFNode **)ptr->field.far_ptr);

	if (n && JSVAL_IS_STRING(id) && ( (str = JSVAL_TO_STRING(id)) != 0) ) {
		char *fieldName = JS_GetStringBytes(str);
		if (!strnicmp(fieldName, "toString", 8)) {
			return JS_TRUE;
		}
		/*fieldID indexing*/
		if (!strnicmp(fieldName, "_field", 6)) {
			index = atoi(fieldName+6);
			if ( Node_GetField(n, index, &info) != M4OK) return JS_FALSE;
		} else {
			if ( Node_GetFieldByName(n, fieldName, &info) != M4OK) return JS_FALSE;
		}
		*vp = JS_ToJSField(JS_GetScriptStack(c), &info, n);
		return (*vp == JSVAL_NULL) ? JS_FALSE : JS_TRUE;
    }
	return JS_FALSE;
}

static JSBool node_setProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	SFNode *n;
	FieldInfo info;
	u32 index;
	char *fieldname;
	JSField *ptr;
	if (! JS_InstanceOf(c, obj, &SFNodeClass, NULL) ) return JS_FALSE;
	ptr = (JSField *) JS_GetPrivate(c, obj);
	assert(ptr->field.fieldType==FT_SFNode);
	n = * ((SFNode **)ptr->field.far_ptr);

	if (n && JSVAL_IS_STRING(id)) {
		JSString *str = JSVAL_TO_STRING(id);
		fieldname = JS_GetStringBytes(str);
		
		/*fieldID indexing*/
		if (!strnicmp(fieldname, "_field", 6)) {
			index = atoi(fieldname+6);
			if ( Node_GetField(n, index, &info) != M4OK) return JS_FALSE;
		} else {
			if (Node_GetFieldByName(n, fieldname, &info) != M4OK) {
				/*VRML style*/
				if (!strnicmp(fieldname, "set_", 4)) {
					fieldname+=4;
					if (Node_GetFieldByName(n, fieldname, &info) != M4OK) return JS_FALSE;
				} else return JS_FALSE;
			}
		}
		JS_ToNodeField(c, *vp, &info, n, ptr);
	}
	return JS_TRUE;
}
static JSBool node_toString(JSContext *c, JSObject *obj, uintN i, jsval *v, jsval *rval)
{
	char str[1000];
	SFNode *n;
	JSString *s;
	JSField *f;
	if (! JS_InstanceOf(c, obj, &SFNodeClass, NULL) ) return JS_FALSE;
	f = (JSField *) JS_GetPrivate(c, obj);
	if (!f) return JS_FALSE;

	str[0] = 0;
	n = * ((SFNode **)f->field.far_ptr);
	if (n->sgprivate->NodeID) {
		if (n->sgprivate->NodeName) {
			sprintf(str , "DEF %s ", n->sgprivate->NodeName);
		} else {
			sprintf(str , "DEF %d ", n->sgprivate->NodeID - 1);
		}
	}
	strcat(str, Node_GetName(n));
	s = JS_NewStringCopyZ(c, (const char *) str);
	if (!s) return JS_FALSE;
	*rval = STRING_TO_JSVAL(s); 
	return JS_TRUE; 
}
static JSFunctionSpec SFNodeMethods[] = {
	{"toString", node_toString, 0},
	{0}
};
static JSPropertySpec SFNodeProps[] = {
	{"__dummy",       0,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{0}
};

/* Generic field destructor */
static void field_finalize(JSContext *c, JSObject *obj)
{
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);
	JS_ObjectDestroyed(c, obj);
	if (!ptr) return;

	if (ptr->field_ptr) VRML_DeleteFieldPointer(ptr->field_ptr, ptr->field.fieldType);
	free(ptr);
}





/*SFImage class functions */
static M4INLINE JSField *SFImage_Create(JSContext *c, JSObject *obj, u32 w, u32 h, u32 nbComp, MFInt32 *pixels)
{
	u32 i, len;
	JSField *field;
	SFImage *v;
	field = NewJSField();
	v = VRML_NewFieldPointer(FT_SFImage);
	field->field_ptr = field->field.far_ptr = v;
	field->field.fieldType = FT_SFImage;
	v->width = w;
	v->height = h;
	v->numComponents = nbComp;
	SAFEALLOC(v->pixels, sizeof(u8) * nbComp * w * h);
	len = MIN(nbComp * w * h, pixels->count);
	for (i=0; i<len; i++) v->pixels[i] = (u8) pixels->vals[i];
	JS_SetPrivate(c, obj, field);
	return field;
}
static JSBool SFImageConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rv)
{
	u32 w, h, nbComp;
	MFInt32 *pixels;
	if (argc<4) return 0;
	if (!JSVAL_IS_INT(argv[0]) || !JSVAL_IS_INT(argv[1]) || !JSVAL_IS_INT(argv[2])) return JS_FALSE;
	if (!JSVAL_IS_OBJECT(argv[3]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[3]), &MFInt32Class, NULL)) return JS_FALSE;
	w = JSVAL_TO_INT(argv[0]);
	h = JSVAL_TO_INT(argv[1]);
	nbComp = JSVAL_TO_INT(argv[2]);
	pixels = (MFInt32 *) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[3])))->field.far_ptr;
	SFImage_Create(c, obj, w, h, nbComp, pixels);
	return JS_TRUE;
}
static JSBool image_getProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	ScriptPriv *priv = JS_GetScriptStack(c);
	JSField *val = (JSField *) JS_GetPrivate(c, obj);
	SFImage *sfi = (SFImage*)val->field.far_ptr;
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0: *vp = INT_TO_JSVAL( sfi->width ); break;
		case 1: *vp = INT_TO_JSVAL( sfi->height); break;
		case 2: *vp = INT_TO_JSVAL( sfi->numComponents ); break;
		case 3: 
		{
			u32 i, len;
			JSObject *an_obj = JS_ConstructObject(priv->js_ctx, &MFInt32Class, 0, priv->js_obj);
			len = sfi->width*sfi->height*sfi->numComponents;
			for (i=0; i<len; i++) {
				jsval newVal = INT_TO_JSVAL(sfi->pixels[i]);
				JS_SetElement(priv->js_ctx, an_obj, (jsint) i, &newVal);
			}
		}
			break;
		default: return JS_FALSE;
		}
	}
	return JS_TRUE;
}

static JSBool image_setProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	u32 ival;
	Bool changed = 0;
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);
	SFImage *sfi = (SFImage*)ptr->field.far_ptr;

	if (JSVAL_IS_INT(id) && JSVAL_TO_INT(id) >= 0 && JSVAL_TO_INT(id) < 4) {
		switch (JSVAL_TO_INT(id)) {
		case 0: 
			ival = JSVAL_TO_INT(vp);
			changed = ! (sfi->width == (Float) ival);
			sfi->width = ival;
			if (changed && sfi->pixels) { free(sfi->pixels); sfi->pixels = NULL; }
			break;
		case 1: 
			ival = JSVAL_TO_INT(vp);
			changed = ! (sfi->height == (Float) ival);
			sfi->height = ival;
			if (changed && sfi->pixels) { free(sfi->pixels); sfi->pixels = NULL; }
			break;
		case 2: 
			ival = JSVAL_TO_INT(vp);
			changed = ! (sfi->numComponents == (Float) ival);
			sfi->numComponents = ival;
			if (changed && sfi->pixels) { free(sfi->pixels); sfi->pixels = NULL; }
			break;
		case 3:
		{
			MFInt32 *pixels;
			u32 len, i;
			if (!JSVAL_IS_OBJECT(*vp) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(*vp), &MFInt32Class, NULL)) return JS_FALSE;
			pixels = (MFInt32 *) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(*vp)))->field.far_ptr;
			if (sfi->pixels) free(sfi->pixels);
			len = sfi->width*sfi->height*sfi->numComponents;
			SAFEALLOC(sfi->pixels, sizeof(char)*len);
			len = MAX(len, pixels->count);
			for (i=0; i<len; i++) sfi->pixels[i] = (u8) pixels->vals[i];
			changed = 1;
			break;
		}
		default: return JS_FALSE;
		}
		if (changed) Script_FieldChanged(NULL, ptr, NULL);
		return JS_TRUE;
    }
	return JS_FALSE;
}
static JSPropertySpec SFImageProps[] = {
	{"x",       0,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{"y",       1,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{"comp",    2,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{"array",   3,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{0}
};


/*SFVec2f class functions */
static M4INLINE JSField *SFVec2f_Create(JSContext *c, JSObject *obj, Float x, Float y)
{
	JSField *field;
	SFVec2f *v;
	field = NewJSField();
	v = VRML_NewFieldPointer(FT_SFVec2f);
	field->field_ptr = field->field.far_ptr = v;
	field->field.fieldType = FT_SFVec2f;
	v->x = x;
	v->y = y;
	JS_SetPrivate(c, obj, field);
	return field;
}
static JSBool SFVec2fConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rv)
{
	jsdouble x = 0.0, y = 0.0;
	if (argc > 0) JS_ValueToNumber(c, argv[0], &x);
	if (argc > 1) JS_ValueToNumber(c, argv[1], &y);
	SFVec2f_Create(c, obj, (Float) x, (Float) y);
	return JS_TRUE;
}
static JSBool vec2f_getProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	JSField *val = (JSField *) JS_GetPrivate(c, obj);
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0: *vp = DOUBLE_TO_JSVAL(JS_NewDouble(c, ((SFVec2f*)val->field.far_ptr)->x)); break;
		case 1: *vp = DOUBLE_TO_JSVAL(JS_NewDouble(c, ((SFVec2f*)val->field.far_ptr)->y)); break;
		default: return JS_FALSE;
		}
	}
	return JS_TRUE;
}

static JSBool vec2f_setProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	jsdouble d;
	Bool changed = 0;
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);

	if (JSVAL_IS_INT(id) && JSVAL_TO_INT(id) >= 0 && JSVAL_TO_INT(id) < 2 && JS_ValueToNumber(c, *vp, &d)) {
		switch (JSVAL_TO_INT(id)) {
		case 0: 
			changed = ! ( ((SFVec2f*)ptr->field.far_ptr)->x == (Float) d);
			((SFVec2f*)ptr->field.far_ptr)->x = (Float) d;
			break;
		case 1: 
			changed = ! ( ((SFVec2f*)ptr->field.far_ptr)->y == (Float) d);
			((SFVec2f*)ptr->field.far_ptr)->y = (Float) d;
			break;
		default: return JS_FALSE;
		}
		if (changed) Script_FieldChanged(NULL, ptr, NULL);
		return JS_TRUE;
    }
	return JS_FALSE;
}
static JSPropertySpec SFVec2fProps[] = {
	{"x",       0,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{"y",       1,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{0}
};
static JSBool vec2f_add(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec2f *v1, *v2;
	JSObject *pNew;
	if (argc<=0 || !JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFVec2fClass, NULL))
		return JS_FALSE;

	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
    v2 = ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0])))->field.far_ptr;
	pNew = JS_NewObject(c, &SFVec2fClass, 0, JS_GetParent(c, obj));  
	SFVec2f_Create(c, pNew, v1->x + v2->x, v1->y + v2->y);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool vec2f_subtract(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec2f *v1, *v2;
	JSObject *pNew;
	if (argc<=0 || !JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFVec2fClass, NULL))
		return JS_FALSE;

	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
    v2 = ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0])))->field.far_ptr;
	pNew = JS_NewObject(c, &SFVec2fClass, 0, JS_GetParent(c, obj));  
	SFVec2f_Create(c, pNew, v1->x - v2->x, v1->y - v2->y);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool vec2f_negate(JSContext *c, JSObject *obj, uintN n, jsval *v, jsval *rval)
{
	SFVec2f *v1;
	JSObject *pNew;
	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	pNew = JS_NewObject(c, &SFVec2fClass, 0, JS_GetParent(c, obj));  
	SFVec2f_Create(c, pNew, -v1->x , -v1->y );
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool vec2f_multiply(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec2f *v1;
	JSObject *pNew;
	jsdouble d;
	if (argc<=0) return JS_FALSE;
	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	pNew = JS_NewObject(c, &SFVec2fClass, 0, JS_GetParent(c, obj));  
	JS_ValueToNumber(c, argv[0], &d );
	SFVec2f_Create(c, pNew, v1->x * (Float) d, v1->y * (Float) d);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool vec2f_divide(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec2f *v1;
	JSObject *pNew;
	jsdouble d;
	if (argc<=0) return JS_FALSE;
	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	pNew = JS_NewObject(c, &SFVec2fClass, 0, JS_GetParent(c, obj));  
	JS_ValueToNumber(c, argv[0], &d );
	SFVec2f_Create(c, pNew, v1->x / (Float) d, v1->y / (Float) d);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool vec2f_length(JSContext *c, JSObject *obj, uintN n, jsval *val, jsval *rval)
{
	Double res;
	SFVec2f *v1;
	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	res = sqrt(v1->x*v1->x + v1->y*v1->y);
	*rval = DOUBLE_TO_JSVAL(JS_NewDouble(c, res) );
	return JS_TRUE;
}
static JSBool vec2f_normalize(JSContext *c, JSObject *obj, uintN n, jsval *val, jsval *rval)
{
	SFVec2f *v1;
	Double res;
	JSObject *pNew;
	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	res = sqrt(v1->x*v1->x + v1->y*v1->y);
	pNew = JS_NewObject(c, &SFVec2fClass, 0, JS_GetParent(c, obj));  
	SFVec2f_Create(c, pNew, v1->x / (Float) res, v1->y / (Float) res);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool vec2f_dot(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec2f *v1, *v2;
	if (argc<=0 || !JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFVec2fClass, NULL))
		return JS_FALSE;

	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
    v2 = ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0])))->field.far_ptr;
	*rval = DOUBLE_TO_JSVAL(JS_NewDouble(c, v1->x * v2->x + v1->y * v2->y) );
	return JS_TRUE;
}

static JSFunctionSpec SFVec2fMethods[] = {
    {"add",             vec2f_add,      1},
    {"divide",          vec2f_divide,   1},
    {"dot",             vec2f_dot,      1},
    {"length",          vec2f_length,   0},
    {"multiply",        vec2f_multiply, 1},
    {"normalize",       vec2f_normalize,0},
    {"subtract",        vec2f_subtract, 1},
    {"negate",          vec2f_negate,   0},
	{"toString",        field_toString,       0},
	{0}
};


/*SFVec3f class functions */
static M4INLINE JSField *SFVec3f_Create(JSContext *c, JSObject *obj, Float x, Float y, Float z)
{
	JSField *field;
	SFVec3f *v;
	field = NewJSField();
	v = VRML_NewFieldPointer(FT_SFVec3f);
	field->field_ptr = field->field.far_ptr = v;
	field->field.fieldType = FT_SFVec3f;
	v->x = x;
	v->y = y;
	v->z = z;
	JS_SetPrivate(c, obj, field);
	return field;
}
static JSBool SFVec3fConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rv)
{
	jsdouble x = 0.0, y = 0.0, z = 0.0;
	if (argc > 0) JS_ValueToNumber(c, argv[0], &x);
	if (argc > 1) JS_ValueToNumber(c, argv[1], &y);
	if (argc > 2) JS_ValueToNumber(c, argv[2], &z);
	SFVec3f_Create(c, obj, (Float) x, (Float) y, (Float) z);
	return JS_TRUE;
}
static JSBool vec3f_getProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	JSField *val = (JSField *) JS_GetPrivate(c, obj);
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0: *vp = DOUBLE_TO_JSVAL(JS_NewDouble(c, ((SFVec3f*)val->field.far_ptr)->x)); break;
		case 1: *vp = DOUBLE_TO_JSVAL(JS_NewDouble(c, ((SFVec3f*)val->field.far_ptr)->y)); break;
		case 2: *vp = DOUBLE_TO_JSVAL(JS_NewDouble(c, ((SFVec3f*)val->field.far_ptr)->z)); break;
		default: return JS_FALSE;
		}
	}
	return JS_TRUE;
}
static JSBool vec3f_setProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	jsdouble d;
	Bool changed = 0;
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);

	if (JSVAL_IS_INT(id) && JSVAL_TO_INT(id) >= 0 && JSVAL_TO_INT(id) < 3 && JS_ValueToNumber(c, *vp, &d)) {
		switch (JSVAL_TO_INT(id)) {
		case 0: 
			changed = ! ( ((SFVec3f*)ptr->field.far_ptr)->x == (Float) d);
			((SFVec3f*)ptr->field.far_ptr)->x = (Float) d;
			break;
		case 1: 
			changed = ! ( ((SFVec3f*)ptr->field.far_ptr)->y == (Float) d);
			((SFVec3f*)ptr->field.far_ptr)->y = (Float) d;
			break;
		case 2: 
			changed = ! ( ((SFVec3f*)ptr->field.far_ptr)->z == (Float) d);
			((SFVec3f*)ptr->field.far_ptr)->z = (Float) d;
			break;
		default: return JS_FALSE;
		}
		if (changed) Script_FieldChanged(NULL, ptr, NULL);
		return JS_TRUE;
    }
	return JS_FALSE;
}
static JSPropertySpec SFVec3fProps[] = {
	{"x",       0,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{"y",       1,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{"z",       2,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{0}
};
static JSBool vec3f_add(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec3f *v1, *v2;
	JSObject *pNew;
	if (argc<=0 || !JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFVec3fClass, NULL))
		return JS_FALSE;

	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
    v2 = ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0])))->field.far_ptr;
	pNew = JS_NewObject(c, &SFVec3fClass, 0, JS_GetParent(c, obj));  
	SFVec3f_Create(c, pNew, v1->x + v2->x, v1->y + v2->y, v1->z + v2->z);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool vec3f_subtract(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec3f *v1, *v2;
	JSObject *pNew;
	if (argc<=0 || !JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFVec3fClass, NULL))
		return JS_FALSE;

	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
    v2 = ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0])))->field.far_ptr;
	pNew = JS_NewObject(c, &SFVec3fClass, 0, JS_GetParent(c, obj));  
	SFVec3f_Create(c, pNew, v1->x - v2->x, v1->y - v2->y, v1->z - v2->z);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool vec3f_negate(JSContext *c, JSObject *obj, uintN n, jsval *v, jsval *rval)
{
	SFVec3f *v1;
	JSObject *pNew;
	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	pNew = JS_NewObject(c, &SFVec3fClass, 0, JS_GetParent(c, obj));  
	SFVec3f_Create(c, pNew, -v1->x , -v1->y , -v1->z );
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool vec3f_multiply(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec3f *v1;
	JSObject *pNew;
	jsdouble d;
	if (argc<=0) return JS_FALSE;

	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	pNew = JS_NewObject(c, &SFVec3fClass, 0, JS_GetParent(c, obj));  
	JS_ValueToNumber(c, argv[0], &d );
	SFVec3f_Create(c, pNew, v1->x * (Float) d, v1->y * (Float) d, v1->z * (Float) d);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool vec3f_divide(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec3f *v1;
	JSObject *pNew;
	jsdouble d;
	if (argc<=0) return JS_FALSE;
	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	pNew = JS_NewObject(c, &SFVec3fClass, 0, JS_GetParent(c, obj));  
	JS_ValueToNumber(c, argv[0], &d );
	SFVec3f_Create(c, pNew, v1->x / (Float) d, v1->y / (Float) d, v1->z / (Float) d);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool vec3f_length(JSContext *c, JSObject *obj, uintN n, jsval *val, jsval *rval)
{
	Double res;
	SFVec3f *v1;
	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	res = sqrt(v1->x*v1->x + v1->y*v1->y + v1->z*v1->z);
	*rval = DOUBLE_TO_JSVAL(JS_NewDouble(c, res) );
	return JS_TRUE;
}
static JSBool vec3f_normalize(JSContext *c, JSObject *obj, uintN n, jsval *val, jsval *rval)
{
	SFVec3f v1;
	JSObject *pNew;
	v1 = * (SFVec3f *) ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	vec_norm(v1);
	pNew = JS_NewObject(c, &SFVec3fClass, 0, JS_GetParent(c, obj));  
	SFVec3f_Create(c, pNew, v1.x, v1.y, v1.z);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool vec3f_dot(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec3f v1, v2;
	if (argc<=0 || !JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFVec3fClass, NULL))
		return JS_FALSE;

	v1 = *(SFVec3f *) ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
    v2 = *(SFVec3f *) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0])))->field.far_ptr;
	*rval = DOUBLE_TO_JSVAL(JS_NewDouble(c, vec_dot(v1, v2)) );
	return JS_TRUE;
}
static JSBool vec3f_cross(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec3f v1, v2, v3;
	JSObject *pNew;

	if (argc<=0 || !JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFVec3fClass, NULL))
		return JS_FALSE;

	v1 = * (SFVec3f *) ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
    v2 = * (SFVec3f *) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0])))->field.far_ptr;
	pNew = JS_NewObject(c, &SFVec3fClass, 0, JS_GetParent(c, obj));  
	vec_cross(v3, v1, v2);
	SFVec3f_Create(c, pNew, v3.x, v3.y, v3.z);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}

static JSFunctionSpec SFVec3fMethods[] = {
    {"add",             vec3f_add,      1},
    {"divide",          vec3f_divide,   1},
    {"dot",             vec3f_dot,      1},
    {"length",          vec3f_length,   0},
    {"multiply",        vec3f_multiply, 1},
    {"normalize",       vec3f_normalize,0},
    {"subtract",        vec3f_subtract, 1},
    {"cross",			vec3f_cross,	1},
    {"negate",          vec3f_negate,   0},
	{"toString",        field_toString,	0},
	{0}
};



/*SFRotation class*/
static M4INLINE JSField *SFRotation_Create(JSContext *c, JSObject *obj, Float x, Float y, Float z, Float angle)
{
	JSField *field;
	SFRotation *v;
	field = NewJSField();
	v = VRML_NewFieldPointer(FT_SFRotation);
	field->field_ptr = field->field.far_ptr = v;
	field->field.fieldType = FT_SFRotation;
	v->xAxis = x;
	v->yAxis = y;
	v->zAxis = z;
	v->angle = angle;
	JS_SetPrivate(c, obj, field);
	return field;
}
static JSBool SFRotationConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rv)
{
	JSObject *an_obj;
	SFVec3f v1, v2;
	Float l1, l2, dot;
	jsdouble x = 0.0, y = 0.0, z = 0.0, a = 0.0;
	if (argc == 0) {
		SFRotation_Create(c, obj, (Float) x, (Float) y, (Float) z, (Float) a);
		return JS_TRUE;
	}
	if ((argc>0) && JSVAL_IS_DOUBLE(argv[0])) {
		if (argc > 0) JS_ValueToNumber(c, argv[0], &x);
		if (argc > 1) JS_ValueToNumber(c, argv[1], &y);
		if (argc > 2) JS_ValueToNumber(c, argv[2], &z);
		if (argc > 3) JS_ValueToNumber(c, argv[2], &a);
		SFRotation_Create(c, obj, (Float) x, (Float) y, (Float) z, (Float) a);
		return JS_TRUE;
	}
	if (argc!=2) return JS_FALSE;
	if (!JSVAL_IS_OBJECT(argv[0])) return JS_FALSE;
	an_obj = JSVAL_TO_OBJECT(argv[0]);
	if (! JS_InstanceOf(c, an_obj, &SFVec3fClass, NULL)) return JS_FALSE;
	v1 = * (SFVec3f *) ((JSField *) JS_GetPrivate(c, an_obj))->field.far_ptr;
	if (JSVAL_IS_DOUBLE(argv[1])) {
		JS_ValueToNumber(c, argv[1], &a);
		SFRotation_Create(c, obj, v1.x, v1.y, v1.z, (Float) a);
		return JS_TRUE;
	}

	if (!JSVAL_IS_OBJECT(argv[1])) return JS_FALSE;
	an_obj = JSVAL_TO_OBJECT(argv[1]);
	if (!JS_InstanceOf(c, an_obj, &SFVec3fClass, NULL)) return JS_FALSE;
	v2 = * (SFVec3f *) ((JSField *) JS_GetPrivate(c, an_obj))->field.far_ptr;
	l1 = vec_len(v1);
	l2 = vec_len(v2);
	dot = vec_dot(v1, v2) / (l1*l2);
	a = atan2(sqrt(1 - dot * dot), dot);
	SFRotation_Create(c, obj, v1.y * v2.z - v2.y * v1.z,
								v1.z * v2.x - v2.z * v1.x,
								v1.x * v2.y - v2.x * v1.y,
								(Float) a);
	return JS_TRUE;
}

static JSBool rot_getProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	JSField *val = (JSField *) JS_GetPrivate(c, obj);
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0: *vp = DOUBLE_TO_JSVAL(JS_NewDouble(c, ((SFRotation*)val->field.far_ptr)->xAxis)); break;
		case 1: *vp = DOUBLE_TO_JSVAL(JS_NewDouble(c, ((SFRotation*)val->field.far_ptr)->yAxis)); break;
		case 2: *vp = DOUBLE_TO_JSVAL(JS_NewDouble(c, ((SFRotation*)val->field.far_ptr)->zAxis)); break;
		case 3: *vp = DOUBLE_TO_JSVAL(JS_NewDouble(c, ((SFRotation*)val->field.far_ptr)->angle)); break;
		default: return JS_FALSE;
		}
	}
	return JS_TRUE;
}
static JSBool rot_setProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	jsdouble d;
	Bool changed = 0;
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);

	if (JSVAL_IS_INT(id) && JSVAL_TO_INT(id) >= 0 && JSVAL_TO_INT(id) < 4 && JS_ValueToNumber(c, *vp, &d)) {
		switch (JSVAL_TO_INT(id)) {
		case 0: 
			changed = ! ( ((SFRotation*)ptr->field.far_ptr)->xAxis == (Float) d);
			((SFRotation*)ptr->field.far_ptr)->xAxis = (Float) d;
			break;
		case 1: 
			changed = ! ( ((SFRotation*)ptr->field.far_ptr)->yAxis == (Float) d);
			((SFRotation*)ptr->field.far_ptr)->yAxis = (Float) d;
			break;
		case 2: 
			changed = ! ( ((SFRotation*)ptr->field.far_ptr)->zAxis == (Float) d);
			((SFRotation*)ptr->field.far_ptr)->zAxis = (Float) d;
			break;
		case 3: 
			changed = ! ( ((SFRotation*)ptr->field.far_ptr)->angle == (Float) d);
			((SFRotation*)ptr->field.far_ptr)->angle = (Float) d;
			break;
		default: return JS_FALSE;
		}
		if (changed) Script_FieldChanged(NULL, ptr, NULL);
		return JS_TRUE;
    }
	return JS_FALSE;
}
static JSPropertySpec SFRotationProps[] = {
	{"x",       0,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{"y",       1,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{"z",       2,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{"angle",   3,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{0}
};
static JSBool rot_getAxis(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFRotation r;
	JSObject *pNew;
	r = * (SFRotation *) ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	pNew = JS_NewObject(c, &SFVec3fClass, 0, JS_GetParent(c, obj));  
	SFVec3f_Create(c, pNew, r.xAxis, r.yAxis, r.zAxis);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool rot_inverse(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFRotation r;
	JSObject *pNew;
	r = * (SFRotation *) ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	pNew = JS_NewObject(c, &SFRotationClass, 0, JS_GetParent(c, obj));  
	SFRotation_Create(c, pNew, r.xAxis, r.yAxis, r.zAxis, r.angle-M_PI);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}

static JSBool rot_multiply(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFRotation r1, r2;
	SFVec4f q1, q2;
	JSObject *pNew;

	if (argc<=0 || !JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFRotationClass, NULL))
		return JS_FALSE;
	
	r1 = * (SFRotation *) ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	r2 = * (SFRotation *) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0])))->field.far_ptr;
	q1 = quat_from_rotation(r1);
	q2 = quat_from_rotation(r2);
	q1 = quat_multiply(&q1, &q2);
	r1 = quat_to_rotation(&q1);

	pNew = JS_NewObject(c, &SFRotationClass, 0, JS_GetParent(c, obj)); 
	SFRotation_Create(c, pNew, r1.xAxis, r1.yAxis, r1.zAxis, r1.angle);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool rot_multVec(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec3f v;
	SFRotation r;
	M4Matrix mx;
	JSObject *pNew;
	if (argc<=0) return JS_FALSE;

	if (argc<=0 || !JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFVec3fClass, NULL))
		return JS_FALSE;

	r = *(SFRotation *) ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	v = *(SFVec3f *) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0])))->field.far_ptr;
	mx_init(mx);
	mx_add_rotation(&mx, r.angle, r.xAxis, r.yAxis, r.zAxis);
	mx_apply_vec(&mx, &v);
	pNew = JS_NewObject(c, &SFVec3fClass, 0, JS_GetParent(c, obj));  
	SFVec3f_Create(c, pNew, v.x, v.y, v.z);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}
static JSBool rot_setAxis(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFVec3f v;
	SFRotation r;
	if (argc<=0) return JS_FALSE;

	if (argc<=0 || !JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFVec3fClass, NULL))
		return JS_FALSE;

	r = *(SFRotation *) ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	v = *(SFVec3f *) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0])))->field.far_ptr;
	r.xAxis = v.x;
	r.yAxis = v.y;
	r.zAxis = v.z;
	return JS_TRUE;
}
static JSBool rot_slerp(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	SFRotation v1, v2, res;
	SFVec4f q1, q2;
	JSObject *pNew;
	jsdouble d;
	if (argc<=1) return JS_FALSE;

	if (!JSVAL_IS_DOUBLE(argv[1]) || !JSVAL_IS_OBJECT(argv[0]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[0]), &SFRotationClass, NULL)) return JS_FALSE;

	v1 = *(SFRotation *) ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	v2 = *(SFRotation *) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0])))->field.far_ptr;
	JS_ValueToNumber(c, argv[1], &d );
	q1 = quat_from_rotation(v1);
	q2 = quat_from_rotation(v2);
	q1 = quat_slerp(q1, q2, (Float) d);
	res = quat_to_rotation(&q1);
	pNew = JS_NewObject(c, &SFRotationClass, 0, JS_GetParent(c, obj));  
	SFRotation_Create(c, pNew, res.xAxis, res.yAxis, res.zAxis, res.angle);
	*rval = OBJECT_TO_JSVAL(pNew);
	return JS_TRUE;
}

static JSFunctionSpec SFRotationMethods[] = {
    {"getAxis",         rot_getAxis,      1},
    {"inverse",         rot_inverse,   1},
    {"multiply",        rot_multiply,      1},
    {"multVec",         rot_multVec,   0},
    {"setAxis",			rot_setAxis, 1},
    {"slerp",			rot_slerp,0},
	{"toString",        field_toString,	0},
	{0}
};




/* SFColor class functions */
static M4INLINE JSField *SFColor_Create(JSContext *c, JSObject *obj, Float r, Float g, Float b)
{
	JSField *field;
	SFColor *v;
	field = NewJSField();
	v = VRML_NewFieldPointer(FT_SFColor);
	field->field_ptr = field->field.far_ptr = v;
	field->field.fieldType = FT_SFColor;
	v->red = r;
	v->green = g;
	v->blue = b;
	JS_SetPrivate(c, obj, field);
	return field;
}
static JSBool SFColorConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rv)
{
	jsdouble r = 0.0, g = 0.0, b = 0.0;
	if (argc > 0) JS_ValueToNumber(c, argv[0], &r);
	if (argc > 1) JS_ValueToNumber(c, argv[1], &g);
	if (argc > 2) JS_ValueToNumber(c, argv[2], &b);
	SFColor_Create(c, obj, (Float) r, (Float) g, (Float) b);
	return JS_TRUE;
}
static JSBool color_getProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	JSField *val = (JSField *) JS_GetPrivate(c, obj);
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0: *vp = DOUBLE_TO_JSVAL(JS_NewDouble(c, ((SFColor*)val->field.far_ptr)->red)); break;
		case 1: *vp = DOUBLE_TO_JSVAL(JS_NewDouble(c, ((SFColor*)val->field.far_ptr)->green)); break;
		case 2: *vp = DOUBLE_TO_JSVAL(JS_NewDouble(c, ((SFColor*)val->field.far_ptr)->blue)); break;
		default: return JS_FALSE;
		}
	}
	return JS_TRUE;
}

static JSBool color_setProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	jsdouble d;
	Bool changed = 0;
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);

	if (JSVAL_IS_INT(id) && JSVAL_TO_INT(id) >= 0 && JSVAL_TO_INT(id) < 3 && JS_ValueToNumber(c, *vp, &d)) {
		switch (JSVAL_TO_INT(id)) {
		case 0: 
			changed = ! ( ((SFColor*)ptr->field.far_ptr)->red == (Float) d);
			((SFColor*)ptr->field.far_ptr)->red = (Float) d;
			break;
		case 1: 
			changed = ! ( ((SFColor*)ptr->field.far_ptr)->green == (Float) d);
			((SFColor*)ptr->field.far_ptr)->green = (Float) d;
			break;
		case 2: 
			changed = ! ( ((SFColor*)ptr->field.far_ptr)->blue == (Float) d);
			((SFColor*)ptr->field.far_ptr)->blue = (Float) d;
			break;
		default: return JS_FALSE;
		}
		if (changed) Script_FieldChanged(NULL, ptr, NULL);
		return JS_TRUE;
    }
	return JS_FALSE;
}
static JSPropertySpec SFColorProps[] = {
	{"r",       0,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{"g",       1,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{"b",       2,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
	{0}
};
static JSBool color_setHSV(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rv)
{
	SFColor *v1, hsv;
	jsdouble h, s, v;
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);
	if (argc != 3) return JS_FALSE;
	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	JS_ValueToNumber( c, argv[0], &h);
	JS_ValueToNumber( c, argv[1], &s);
	JS_ValueToNumber( c, argv[2], &v);
	hsv.red = (Float) h;
	hsv.green = (Float) s;
	hsv.blue = (Float) v;
	SFColor_fromHSV(&hsv);
	VRML_FieldCopy(v1, &hsv, FT_SFColor);
	Script_FieldChanged(NULL, ptr, NULL);
	return JS_TRUE;
}

static JSBool color_getHSV(JSContext *c, JSObject *obj, uintN n, jsval *va, jsval *rval)
{
	SFColor *v1, hsv;
	jsval vec[3];
	JSObject *arr;

	v1 = ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
	hsv = *v1;
	SFColor_toHSV(&hsv);
	vec[0] = DOUBLE_TO_JSVAL(JS_NewDouble(c, hsv.red));
	vec[1] = DOUBLE_TO_JSVAL(JS_NewDouble(c, hsv.green));
	vec[2] = DOUBLE_TO_JSVAL(JS_NewDouble(c, hsv.blue));
	arr = JS_NewArrayObject(c, 3, vec);
	*rval = OBJECT_TO_JSVAL(arr);
	return JS_TRUE;
}
static JSFunctionSpec SFColorMethods[] = {
    {"setHSV",          color_setHSV,   3, 0, 0},
    {"getHSV",          color_getHSV,   0, 0, 0},
	{"toString",        field_toString,       0, 0, 0},
	{0, 0, 0, 0, 0}
};



static JSBool MFArrayConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval, u32 fieldType)
{
	JSField *ptr = NewJSField();
	ptr->field.fieldType = fieldType;
	ptr->js_list = JS_NewArrayObject(c, (jsint) argc, argv);
	JS_SetPrivate(c, obj, ptr);
	*rval = OBJECT_TO_JSVAL(obj);
	return obj == 0 ? JS_FALSE : JS_TRUE;
}

static JSBool MFBoolConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	return MFArrayConstructor(c, obj, argc, argv, rval, FT_MFBool);
}
static JSBool MFInt32Constructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	return MFArrayConstructor(c, obj, argc, argv, rval, FT_MFInt32);
}
static JSBool MFFloatConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	return MFArrayConstructor(c, obj, argc, argv, rval, FT_MFFloat);
}
static JSBool MFTimeConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	return MFArrayConstructor(c, obj, argc, argv, rval, FT_MFTime);
}
static JSBool MFStringConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	return MFArrayConstructor(c, obj, argc, argv, rval, FT_MFString);
}
static JSBool MFURLConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	return MFArrayConstructor(c, obj, argc, argv, rval, FT_MFURL);
}
static JSBool MFNodeConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	return MFArrayConstructor(c, obj, argc, argv, rval, FT_MFNode);
}


static void array_finalize(JSContext *c, JSObject *obj)
{
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);
	JS_ObjectDestroyed(c, obj);
	if (!ptr) return;
	
	/*MFNode*/
	if (ptr->temp_list) {
		while (ChainGetCount(ptr->temp_list)) {
			SFNode *n = ChainGetEntry(ptr->temp_list, 0);
			ChainDeleteEntry(ptr->temp_list, 0);
			Node_Unregister(n, ptr->owner);
		}
		DeleteChain(ptr->temp_list);
	} 
	if (ptr->field_ptr) {
		VRML_DeleteFieldPointer(ptr->field_ptr, ptr->field.fieldType);
	}
	free(ptr);
}

JSBool array_getElement(JSContext *c, JSObject *obj, jsval id, jsval *rval)
{
	u32 i;
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);
	if (JSVAL_IS_INT(id)) {
		i = JSVAL_TO_INT(id);
		JS_GetElement(c, ptr->js_list, (jsint) i, rval);
	}
	return JS_TRUE;
}


static void array_rewriteMFField(JSContext *c, JSField *ptr)
{
	jsval item;
	u32 i, len;
	jsdouble d;
	JSField *from;

	JS_GetArrayLength(c, ptr->js_list, &len);

	if (ptr->field.fieldType == FT_MFNode) {
		SFNode *child;
		Chain *list = * (Chain **) ptr->field.far_ptr;
		while (ChainGetCount(list)) {
			child = ChainGetEntry(list, 0);
			ChainDeleteEntry(list, 0);
			Node_Unregister(child, ptr->owner);
		}
	
		for (i=0; i<len; i++) {
			JSObject *node_obj;
			JS_GetElement(c, ptr->js_list, (jsint) i, &item);
			if (JSVAL_IS_NULL(item)) break;
			node_obj = JSVAL_TO_OBJECT(item);
			if (!JS_InstanceOf(c, node_obj, &SFNodeClass, NULL) ) break;
			child = * ((SFNode**) ((JSField *) JS_GetPrivate(c, node_obj))->field.far_ptr);
			if (!child) continue;	/*not an error, arrays may be allocated before items are set*/
			ChainAddEntry(list, child);
			Node_Register(child, ptr->owner);
		}
		return;
	}

	/*realloc*/
	if (len != ((GenMFField *)ptr->field.far_ptr)->count) {
		VRML_MF_Reset(ptr->field.far_ptr, ptr->field.fieldType);
		VRML_MF_Alloc(ptr->field.far_ptr, ptr->field.fieldType, len);
	}
	/*assign each slot*/
	for (i=0; i<len; i++) {
		JS_GetElement(c, ptr->js_list, (jsint) i, &item);

		switch (ptr->field.fieldType) {
		case FT_MFBool:
			if (JSVAL_IS_BOOLEAN(item)) 
				((MFBool*)ptr->field.far_ptr)->vals[i] = (Bool) JSVAL_TO_BOOLEAN(item);
			break;
		case FT_MFInt32:
			if (JSVAL_IS_INT(item))
				((MFInt32 *)ptr->field.far_ptr)->vals[i] = (s32) JSVAL_TO_INT(item);
			break;
		case FT_MFFloat:
			if (JSVAL_IS_NUMBER(item)) {
				JS_ValueToNumber(c, item, &d);
				((MFFloat *)ptr->field.far_ptr)->vals[i] = (Float) d;
			}
			break;
		case FT_MFTime:
			if (JSVAL_IS_NUMBER(item)) {
				JS_ValueToNumber(c, item, &d);
				((MFTime *)ptr->field.far_ptr)->vals[i] = d;
			}
			break;
		case FT_MFString:
			if (JSVAL_IS_STRING(item)) {
				MFString *mfs = (MFString *) ptr->field.far_ptr;
				JSString *str = JSVAL_TO_STRING(item);
				char *str_val = JS_GetStringBytes(str);
				if (mfs->vals[i]) free(mfs->vals[i]);
				mfs->vals[i] = strdup(str_val);
			}
			break;
		case FT_MFURL:
			if (JSVAL_IS_STRING(item)) {
				MFURL *mfu = (MFURL *) ptr->field.far_ptr;
				JSString *str = JSVAL_TO_STRING(item);
				if (mfu->vals[i].url) free(mfu->vals[i].url);
				mfu->vals[i].url = strdup(JS_GetStringBytes(str));
				mfu->vals[i].OD_ID = 0;
			}
			break;

		case FT_MFVec2f:
			if ( JSVAL_IS_OBJECT(item) && JS_InstanceOf(c, JSVAL_TO_OBJECT(item), &SFVec2fClass, NULL) ) {
				from = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(item));
				VRML_FieldCopy(& ((MFVec2f*)ptr->field.far_ptr)->vals[i], from->field.far_ptr, FT_SFVec2f);
			}
			break;
		case FT_MFVec3f:
			if ( JSVAL_IS_OBJECT(item) && JS_InstanceOf(c, JSVAL_TO_OBJECT(item), &SFVec3fClass, NULL) ) {
				from = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(item));
				VRML_FieldCopy(& ((MFVec3f*)ptr->field.far_ptr)->vals[i], from->field.far_ptr, FT_SFVec3f);
			}
			break;
		case FT_MFRotation:
			if ( JSVAL_IS_OBJECT(item) && JS_InstanceOf(c, JSVAL_TO_OBJECT(item), &SFRotationClass, NULL) ) {
				from = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(item));
				VRML_FieldCopy(& ((MFRotation*)ptr->field.far_ptr)->vals[i], from->field.far_ptr, FT_SFRotation);
			}
			break;
		case FT_MFColor:
			if ( JSVAL_IS_OBJECT(item) && JS_InstanceOf(c, JSVAL_TO_OBJECT(item), &SFColorClass, NULL) ) {
				SFColor *col;
				from = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(item));
				col = (SFColor *)from->field.far_ptr;
				VRML_FieldCopy(& ((MFColor*)ptr->field.far_ptr)->vals[i], from->field.far_ptr, FT_SFColor);
			}
			break;

		default:
			return;
		}
	}
}
//this could be overloaded for each MF type...
JSBool array_setElement(JSContext *c, JSObject *obj, jsval id, jsval *rval)
{
	u32 ind, len;
	jsval elt_val;
	JSField *elt_ptr;
	JSBool ret;
	Bool val_changed = 1;
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);
	ret = JS_GetArrayLength(c, ptr->js_list, &len);
	if (ret==JS_FALSE) return JS_FALSE;

	ind = JSVAL_TO_INT(id);
	/*dynamic expend*/
	if (ind>=len) {
		u32 sftype;
		JSClass *the_sf_class = NULL;
		ret = JS_SetArrayLength(c, ptr->js_list, len+1);
		if (ret==JS_FALSE) return JS_FALSE;
		ret = JS_GetArrayLength(c, ptr->js_list, &len);
		switch (ptr->field.fieldType) {
		case FT_MFVec2f: the_sf_class = &SFVec2fClass; break;
		case FT_MFVec3f: the_sf_class = &SFVec3fClass; break;
		case FT_MFColor: the_sf_class = &SFColorClass; break;
		case FT_MFRotation: the_sf_class = &SFRotationClass; break;
		}
		sftype = VRML_GetSFType(ptr->field.fieldType);
		while (len<ind) {
			jsval a_val;
			if (the_sf_class) {
				JSObject *an_obj = JS_ConstructObject(c, the_sf_class, 0, obj);
				a_val = OBJECT_TO_JSVAL(an_obj );
			} else {
				switch (sftype) {
				case FT_SFBool: a_val = BOOLEAN_TO_JSVAL(0); break;
				case FT_SFInt32: a_val = INT_TO_JSVAL(0); break;
				case FT_SFFloat: 
				case FT_SFTime: 
					a_val = DOUBLE_TO_JSVAL( JS_NewDouble(c, 0) );
					break;
				case FT_SFString:
				case FT_SFURL:
					a_val = STRING_TO_JSVAL( JS_NewStringCopyZ(c, "") );
					break;
				default: a_val = INT_TO_JSVAL(0); break;
				}
			}
			JS_SetElement(c, ptr->js_list, len, &a_val);
			len++;
		}
		/*and assign*/
		if (ptr->field.fieldType==FT_MFNode) {
			JSObject *an_obj;
			SFNode *item;
			if (! JS_InstanceOf(c, JSVAL_TO_OBJECT(*rval), &SFNodeClass, NULL) ) return JS_FALSE;
			
			an_obj = JS_NewObject(c, &SFNodeClass, 0, obj);
			item = * (SFNode **) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(*rval) ))->field.far_ptr;
			elt_ptr = NewJSField();
			elt_ptr->owner = ptr->owner;
			elt_ptr->field.fieldType = FT_SFNode;
			JS_SetPrivate(c, an_obj, elt_ptr);
			elt_ptr->temp_node = item;
			elt_ptr->field.far_ptr = &elt_ptr->temp_node;
			Node_Register(item, ptr->owner);
			elt_val = OBJECT_TO_JSVAL(an_obj);
			ret = JS_SetElement(c, ptr->js_list, ind, &elt_val);
			if (ret==JS_FALSE) return JS_FALSE;
		} else {
			if (!the_sf_class || JS_InstanceOf(c, JSVAL_TO_OBJECT(*rval), the_sf_class, NULL) ) 
				JS_SetElement(c, ptr->js_list, ind, rval);
		}	
		if (ptr->owner) {
			array_rewriteMFField(c, ptr);
			Script_FieldChanged(NULL, ptr, NULL);
		}
		return JS_TRUE;
	} 

	ret = JS_GetElement(c, ptr->js_list, (jsint) ind, &elt_val);
	if (ret==JS_FALSE) return JS_FALSE;

	switch (ptr->field.fieldType) {
	case FT_MFBool:
	case FT_MFInt32:
	case FT_MFFloat:
	case FT_MFTime:
	case FT_MFURL:
		JS_SetElement(c, ptr->js_list, ind, rval);
		break;
	case FT_MFString:
		if (JSVAL_IS_STRING(*rval) && JSVAL_IS_STRING(elt_val)) {
			JSString *n_str = JSVAL_TO_STRING(*rval);
			JSString *o_str = JSVAL_TO_STRING(elt_val);
			char *str_new = JS_GetStringBytes(n_str);
			char *str_old = JS_GetStringBytes(o_str);
			if (!strcmp(str_new, str_old) ) {
				val_changed = 0;
			} else {
				JS_SetElement(c, ptr->js_list, ind, rval);
			}
		} else {
			JS_SetElement(c, ptr->js_list, ind, rval);
		}
		break;
	case FT_MFVec2f:
		if ( JSVAL_IS_OBJECT(elt_val) && JS_InstanceOf(c, JSVAL_TO_OBJECT(elt_val), &SFVec2fClass, NULL) ) {
			SFVec2f *item = (SFVec2f *) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(*rval) ))->field.far_ptr;
			elt_ptr = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(elt_val));
			VRML_FieldCopy(elt_ptr->field.far_ptr, item, FT_SFVec2f);
		}
		break;
	case FT_MFVec3f:
		if ( JSVAL_IS_OBJECT(elt_val) && JS_InstanceOf(c, JSVAL_TO_OBJECT(elt_val), &SFVec3fClass, NULL) ) {
			SFVec3f *item = (SFVec3f *) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(*rval) ))->field.far_ptr;
			elt_ptr = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(elt_val));
			VRML_FieldCopy(elt_ptr->field.far_ptr, item, FT_SFVec3f);
		}
		break;
	case FT_MFRotation:
		if ( JSVAL_IS_OBJECT(elt_val) && JS_InstanceOf(c, JSVAL_TO_OBJECT(elt_val), &SFRotationClass, NULL) ) {
			SFRotation *item = (SFRotation *) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(*rval) ))->field.far_ptr;
			elt_ptr = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(elt_val));
			VRML_FieldCopy(elt_ptr->field.far_ptr, item, FT_SFRotation);
		}
		break;
	case FT_MFColor:
		if ( JSVAL_IS_OBJECT(elt_val) && JS_InstanceOf(c, JSVAL_TO_OBJECT(elt_val), &SFColorClass, NULL) ) {
			SFColor *item = (SFColor *) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(*rval) ))->field.far_ptr;
			elt_ptr = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(elt_val));
			VRML_FieldCopy(elt_ptr->field.far_ptr, item, FT_SFColor);
		}
		break;
	case FT_MFNode:
		if (!ptr->owner) return JS_FALSE;
		if ( JSVAL_IS_OBJECT(elt_val) && JS_InstanceOf(c, JSVAL_TO_OBJECT(elt_val), &SFNodeClass, NULL) ) {
			SFNode *item = * (SFNode **) ((JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(*rval) ))->field.far_ptr;
			elt_ptr = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(elt_val));
			/*first assignment*/
			if (!elt_ptr->owner) elt_ptr->owner = ptr->owner;
			assert(elt_ptr->owner==ptr->owner);

			Node_Register(item, ptr->owner);
			if (elt_ptr->field.far_ptr) {
				Node_Unregister( *(SFNode **)elt_ptr->field.far_ptr, elt_ptr->owner);
			}
			elt_ptr->temp_node = item;
			elt_ptr->field.far_ptr = &elt_ptr->temp_node;
		}
		break;

	default:
		return JS_FALSE;
	}

	if (!val_changed || !ptr->owner) return JS_TRUE;
	/*ok switch back to node MFField*/
	array_rewriteMFField(c, ptr);
	Script_FieldChanged(NULL, ptr, NULL);
	return JS_TRUE;
}

JSBool array_setLength(JSContext *c, JSObject *obj, jsval v, jsval *val)
{
	u32 len, i, sftype;
	JSBool ret;
	JSClass *the_sf_class;
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);
	if (!JSVAL_IS_INT(*val) || JSVAL_TO_INT(*val) < 0) return JS_FALSE;
	len = JSVAL_TO_INT(*val);
	ret = JS_SetArrayLength(c, ptr->js_list, len);
	if (ret==JS_FALSE) return ret;

	
#if 0
	/*insert till index if needed*/
	if (ptr->field.fieldType != FT_MFNode) {
		if (!ptr->field.far_ptr) ptr->field_ptr = ptr->field.far_ptr = VRML_NewFieldPointer(ptr->field.fieldType);
		VRML_MF_Reset(ptr->field.far_ptr, ptr->field.fieldType);
		VRML_MF_Alloc(ptr->field.far_ptr, ptr->field.fieldType, len);
		if (ptr->field_ptr) ptr->field_ptr = ptr->field.far_ptr;
	}
#endif

	the_sf_class = NULL;
	switch (ptr->field.fieldType) {
	case FT_MFVec2f: the_sf_class = &SFVec2fClass; break;
	case FT_MFVec3f: the_sf_class = &SFVec3fClass; break;
	case FT_MFColor: the_sf_class = &SFColorClass; break;
	case FT_MFRotation: the_sf_class = &SFRotationClass; break;
	case FT_MFNode: the_sf_class = &SFNodeClass; break;
	}
	sftype = VRML_GetSFType(ptr->field.fieldType);
	for (i=0; i<len; i++) {
		jsval a_val;
		if (the_sf_class) {
			JSObject *an_obj = JS_ConstructObject(c, the_sf_class, 0, obj);
			a_val = OBJECT_TO_JSVAL(an_obj );
		} else {
			switch (sftype) {
			case FT_SFBool: a_val = BOOLEAN_TO_JSVAL(0); break;
			case FT_SFInt32: a_val = INT_TO_JSVAL(0); break;
			case FT_SFFloat: 
			case FT_SFTime: 
				a_val = DOUBLE_TO_JSVAL( JS_NewDouble(c, 0) );
				break;
			case FT_SFString:
			case FT_SFURL:
				a_val = STRING_TO_JSVAL( JS_NewStringCopyZ(c, "") );
				break;
			default: a_val = INT_TO_JSVAL(0); break;
			}
		}
		JS_SetElement(c, ptr->js_list, i, &a_val);
	}
	return JS_TRUE;
}

JSBool array_getLength(JSContext *c, JSObject *obj, jsval v, jsval *val)
{
	jsuint len;
	JSField *ptr = (JSField *) JS_GetPrivate(c, obj);
	JSBool ret = JS_GetArrayLength(c, ptr->js_list, &len);
	*val = INT_TO_JSVAL(len);
	return ret;
}
static JSPropertySpec MFArrayProp[] = {
	{ "length", 0, JSPROP_PERMANENT, array_getLength, array_setLength },
	{ "assign", 0, JSPROP_PERMANENT, array_getElement, array_setElement},
	{ 0, 0, 0, 0, 0 } 
};
static JSFunctionSpec MFArrayMethods[] = {
	{"toString",        field_toString,       0},
	{0}
};


/* MFVec2f class constructor */
static JSBool MFVec2fConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	jsval val;
	JSObject *item;
	u32 i;
	JSField *ptr = NewJSField();
	ptr->js_list = JS_NewArrayObject(c, 0, 0);
	JS_SetArrayLength(c, ptr->js_list, argc);
	JS_SetPrivate(c, obj, ptr);

	for (i=0; i<argc; i++) {
		if (!JSVAL_IS_OBJECT(argv[i]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[i]), &SFVec2fClass, NULL) ) {
			item = JS_ConstructObject(c, &SFVec2fClass, 0, obj);
			val = OBJECT_TO_JSVAL(item);
			JS_SetElement(c, ptr->js_list, i, &val);
		} else {
			JS_SetElement(c, ptr->js_list, i, &argv[i]);
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return obj == 0 ? JS_FALSE : JS_TRUE;
}

/* MFVec3f class constructor */
static JSBool MFVec3fConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	jsval val;
	JSObject *item;
	u32 i;
	JSField *ptr = NewJSField();
	ptr->field.fieldType = FT_MFVec3f;
	ptr->js_list = JS_NewArrayObject(c, (jsint) argc, argv);
	JS_SetArrayLength(c, ptr->js_list, argc);
	JS_SetPrivate(c, obj, ptr);

	for (i=0; i<argc; i++) {
		if (!JSVAL_IS_OBJECT(argv[i]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[i]), &SFVec3fClass, NULL) ) {
			item = JS_ConstructObject(c, &SFVec3fClass, 0, obj);
			val = OBJECT_TO_JSVAL(item);
			JS_SetElement(c, ptr->js_list, i, &val);
		} else {
			JS_SetElement(c, ptr->js_list, i, &argv[i]);
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return obj == 0 ? JS_FALSE : JS_TRUE;
}

/* MFRotation class constructor */
static JSBool MFRotationConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	jsval val;
	JSObject *item;
	u32 i;
	JSField *ptr = NewJSField();
	ptr->field.fieldType = FT_MFRotation;
	ptr->js_list = JS_NewArrayObject(c, 0, 0);
	JS_SetArrayLength(c, ptr->js_list, argc);
	JS_SetPrivate(c, obj, ptr);

	for (i=0; i<argc; i++) {
		if (!JSVAL_IS_OBJECT(argv[i]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[i]), &SFRotationClass, NULL) ) {
			item = JS_ConstructObject(c, &SFVec3fClass, 0, obj);
			val = OBJECT_TO_JSVAL(item);
			JS_SetElement(c, ptr->js_list, i, &val);
		} else {
			JS_SetElement(c, ptr->js_list, i, &argv[i]);
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return obj == 0 ? JS_FALSE : JS_TRUE;
}

/*MFColor class constructor */
static JSBool MFColorConstructor(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	jsval val;
	JSObject *item;
	u32 i;
	JSField *ptr = NewJSField();
	ptr->field.fieldType = FT_MFColor;
	ptr->js_list = JS_NewArrayObject(c, 0, 0);
	JS_SetArrayLength(c, ptr->js_list, argc);
	JS_SetPrivate(c, obj, ptr);

	for (i=0; i<argc; i++) {
		if (!JSVAL_IS_OBJECT(argv[i]) || !JS_InstanceOf(c, JSVAL_TO_OBJECT(argv[i]), &SFColorClass, NULL) ) {
			item = JS_ConstructObject(c, &SFColorClass, 0, obj);
			val = OBJECT_TO_JSVAL(item);
			JS_SetElement(c, ptr->js_list, i, &val);
		} else {
			JS_SetElement(c, ptr->js_list, i, &argv[i]);
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return obj == 0 ? JS_FALSE : JS_TRUE;
}



#define M4_SETUP_JS(vrmlclass, cname, flag, addp, delp, getp, setp, enump, resp, conv, fin)	\
	vrmlclass.name = cname;	\
	vrmlclass.flags = flag;	\
	vrmlclass.addProperty = addp;	\
	vrmlclass.delProperty = delp;	\
	vrmlclass.getProperty = getp;	\
	vrmlclass.setProperty = setp;	\
	vrmlclass.enumerate = enump;	\
	vrmlclass.resolve = resp;		\
	vrmlclass.convert = conv;		\
	vrmlclass.finalize = fin;	

void init_spidermonkey_api(ScriptPriv *sc, SFNode *script)
{

	/*GCC port: classes are declared within code since JS_PropertyStub and co are exported symbols
	from JS runtime lib, so with non-constant addresses*/
	M4_SETUP_JS(globalClass, "global", 0, 
		JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, 
		JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub);

	M4_SETUP_JS(browserClass , "Browser", 0,
		JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,
		JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  JS_FinalizeStub);

	M4_SETUP_JS(SFNodeClass, "SFNode", JSCLASS_HAS_PRIVATE,
		JS_PropertyStub,  JS_PropertyStub,  node_getProperty, node_setProperty,
		JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  node_finalize);

	M4_SETUP_JS(SFVec2fClass , "SFVec2f", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub,  vec2f_getProperty, vec2f_setProperty,
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  field_finalize);

	M4_SETUP_JS(SFVec3fClass , "SFVec3f", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub,  vec3f_getProperty, vec3f_setProperty,
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  field_finalize);

	M4_SETUP_JS(SFRotationClass , "SFRotation", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub,  rot_getProperty, rot_setProperty,
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  field_finalize);

	M4_SETUP_JS(SFColorClass , "SFColor", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub,  color_getProperty, color_setProperty,
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  field_finalize);

	M4_SETUP_JS(SFImageClass , "SFImage", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub,  image_getProperty, image_setProperty,
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  field_finalize);

	M4_SETUP_JS(MFInt32Class , "MFInt32", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub, array_getElement,  array_setElement,  
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  array_finalize);
	
	M4_SETUP_JS(MFBoolClass , "MFBool", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub, array_getElement,  array_setElement,  
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  array_finalize);

	M4_SETUP_JS(MFTimeClass , "MFTime", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub, array_getElement,  array_setElement,  
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  array_finalize);

	M4_SETUP_JS(MFFloatClass , "MFFloat", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub, array_getElement,  array_setElement,  
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  array_finalize);

	M4_SETUP_JS(MFUrlClass , "MFUrl", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub, array_getElement,  array_setElement,  
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  array_finalize);

	M4_SETUP_JS(MFStringClass , "MFString", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub, array_getElement,  array_setElement,  
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  array_finalize);

	M4_SETUP_JS(MFNodeClass , "MFNode", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub, array_getElement,  array_setElement,  
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  array_finalize);

	M4_SETUP_JS(MFVec2fClass , "MFVec2f", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub, array_getElement,  array_setElement,  
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  array_finalize);

	M4_SETUP_JS(MFVec3fClass , "MFVec3f", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub, array_getElement,  array_setElement,  
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  array_finalize);

	M4_SETUP_JS(MFRotationClass , "MFRotation", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub, array_getElement,  array_setElement,  
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  array_finalize);

	M4_SETUP_JS(MFColorClass , "MFColor", JSCLASS_HAS_PRIVATE,
	  JS_PropertyStub,  JS_PropertyStub, array_getElement,  array_setElement,  
	  JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,  array_finalize);

	JS_SetErrorReporter(sc->js_ctx, script_error);

	sc->js_obj = JS_NewObject(sc->js_ctx, &globalClass, 0, 0 );
	JS_InitStandardClasses(sc->js_ctx, sc->js_obj);
	JS_DefineFunctions(sc->js_ctx, sc->js_obj, globalFunctions );

	JS_DefineProperty(sc->js_ctx, sc->js_obj, "FALSE", BOOLEAN_TO_JSVAL(0), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(sc->js_ctx, sc->js_obj, "TRUE", BOOLEAN_TO_JSVAL(1), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(sc->js_ctx, sc->js_obj, "_this", PRIVATE_TO_JSVAL(script), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT );

	sc->js_browser = JS_DefineObject(sc->js_ctx, sc->js_obj, "Browser", &browserClass, 0, 0 );
	JS_DefineProperty(sc->js_ctx, sc->js_browser, "_this", PRIVATE_TO_JSVAL(script), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineFunctions(sc->js_ctx, sc->js_browser, browserFunctions);

	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &SFNodeClass, SFNodeConstructor, 1, SFNodeProps, SFNodeMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &SFVec2fClass, SFVec2fConstructor, 0, SFVec2fProps, SFVec2fMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &SFVec3fClass, SFVec3fConstructor, 0, SFVec3fProps, SFVec3fMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &SFRotationClass, SFRotationConstructor, 0, SFRotationProps, SFRotationMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &SFColorClass, SFColorConstructor, 0, SFColorProps, SFColorMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &SFImageClass, SFImageConstructor, 0, SFImageProps, 0, 0, 0);

	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &MFInt32Class, MFInt32Constructor, 0, MFArrayProp, MFArrayMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &MFBoolClass, MFBoolConstructor, 0, MFArrayProp, MFArrayMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &MFFloatClass, MFFloatConstructor, 0, MFArrayProp, MFArrayMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &MFTimeClass, MFTimeConstructor, 0, MFArrayProp, MFArrayMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &MFStringClass, MFStringConstructor, 0, MFArrayProp, MFArrayMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &MFUrlClass, MFURLConstructor, 0, MFArrayProp, MFArrayMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &MFVec2fClass, MFVec2fConstructor, 0, MFArrayProp, MFArrayMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &MFVec3fClass, MFVec3fConstructor, 0, MFArrayProp, MFArrayMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &MFRotationClass, MFRotationConstructor, 0, MFArrayProp, MFArrayMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &MFColorClass, MFColorConstructor, 0, MFArrayProp, MFArrayMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &MFNodeClass, MFNodeConstructor, 0, MFArrayProp, MFArrayMethods, 0, 0);
/*
	cant get any doc specifying if these are supposed to be supported in MPEG4Script...
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &SFVec4fClass, SFVec4fConstructor, 0, SFVec4fProps, SFVec4fMethods, 0, 0);
	JS_InitClass(sc->js_ctx, sc->js_obj, 0, &MFVec4fClass, MFVec4fCons, 0, MFArrayProp, 0, 0, 0);
*/

}



void JS_ToNodeField(JSContext *c, jsval val, FieldInfo *field, SFNode *owner, JSField *parent)
{
	jsdouble d;
	Bool changed;
	JSObject *obj;
	JSField *p, *from;
	jsuint len;
	jsval item;
	u32 i;

	if (JSVAL_IS_VOID(val)) return;
	if ((field->fieldType != FT_SFNode) && JSVAL_IS_NULL(val)) return;


	switch (field->fieldType) {
	case FT_SFBool:
	{
		if (JSVAL_IS_BOOLEAN(val)) {
			*((SFBool *) field->far_ptr) = JSVAL_TO_BOOLEAN(val);
			Script_FieldChanged(owner, parent, field);
		}
		return;
	}
	case FT_SFInt32:
	{
		if (JSVAL_IS_INT(val) ) {
			* ((SFInt32 *) field->far_ptr) = JSVAL_TO_INT(val);
			Script_FieldChanged(owner, parent, field);
		} else if (JSVAL_IS_NUMBER(val) ) {
			JS_ValueToNumber(c, val, &d );
			*((SFInt32 *) field->far_ptr) = (s32) d;
			Script_FieldChanged(owner, parent, field);
		}
		return;
	}
	case FT_SFFloat:
	{
		if (JSVAL_IS_NUMBER(val) ) {
			JS_ValueToNumber(c, val, &d );
			*((SFFloat *) field->far_ptr) = (Float) d;
			Script_FieldChanged(owner, parent, field);
		}
		return;
    }
	case FT_SFTime:
	{
		if (JSVAL_IS_NUMBER(val) ) {
			JS_ValueToNumber(c, val, &d );
			*((SFTime *) field->far_ptr) = (Double) d;
			Script_FieldChanged(owner, parent, field);
		}
		return;
    }
	case FT_SFString:
	{
		if (JSVAL_IS_STRING(val)) {
			SFString *s = (SFString*)field->far_ptr;
			JSString *str = JSVAL_TO_STRING(val);
			char *str_val = JS_GetStringBytes(str);
			/*we do filter strings since rebuilding a text is quite slow, so let's avoid killing the renderers*/
			if (!s->buffer || strcmp(str_val, s->buffer)) {
				if ( s->buffer) free(s->buffer);
				s->buffer = strdup(str_val);
				Script_FieldChanged(owner, parent, field);
			}
		}
		return;
	}
	case FT_SFURL:
	{
		if (JSVAL_IS_STRING(val)) {
			JSString *str = JSVAL_TO_STRING(val);
			if (((SFURL*)field->far_ptr)->url) free(((SFURL*)field->far_ptr)->url);
			((SFURL*)field->far_ptr)->url = strdup(JS_GetStringBytes(str));
			((SFURL*)field->far_ptr)->OD_ID = 0;
			Script_FieldChanged(owner, parent, field);
		}
		return;
	}

	default:
		break;
	}

	//from here we must have an object
	if (! JSVAL_IS_OBJECT(val)) return;
	obj = JSVAL_TO_OBJECT(val) ;

	switch (field->fieldType) {
	case FT_SFVec2f:
	{
		if (JS_InstanceOf(c, obj, &SFVec2fClass, NULL) ) {
			p = (JSField *) JS_GetPrivate(c, obj);
			VRML_FieldCopy(field->far_ptr, p->field.far_ptr, FT_SFVec2f);
			Script_FieldChanged(owner, parent, field);
		}
		return;
	}
	case FT_SFVec3f:
	{
		if (JS_InstanceOf(c, obj, &SFVec3fClass, NULL) ) {
			p = (JSField *) JS_GetPrivate(c, obj);
			VRML_FieldCopy(field->far_ptr, p->field.far_ptr, FT_SFVec3f);
			Script_FieldChanged(owner, parent, field);
		}
		return;
	}
	case FT_SFRotation:
	{
		if ( JS_InstanceOf(c, obj, &SFRotationClass, NULL) ) {
			p = (JSField *) JS_GetPrivate(c, obj);
			VRML_FieldCopy(field->far_ptr, p->field.far_ptr, FT_SFRotation);
			Script_FieldChanged(owner, parent, field);
		}
		return;
	}
	case FT_SFColor:
	{
		if (JS_InstanceOf(c, obj, &SFColorClass, NULL) ) {
			p = (JSField *) JS_GetPrivate(c, obj);
			VRML_FieldCopy(field->far_ptr, p->field.far_ptr, FT_SFColor);
			Script_FieldChanged(owner, parent, field);
		}
		return;
	}
	case FT_SFNode:
	{
		/*replace object*/
		if (*((SFNode**)field->far_ptr)) 
			Node_Unregister(*((SFNode**)field->far_ptr), owner);

		if (JSVAL_IS_NULL(val)) {
			field->far_ptr = NULL;
			Script_FieldChanged(owner, parent, field);
		} else if (JS_InstanceOf(c, obj, &SFNodeClass, NULL) ) {
			SFNode *n = * (SFNode**) ((JSField *) JS_GetPrivate(c, obj))->field.far_ptr;
			* ((SFNode **)field->far_ptr) = n;
			Node_Register(n, owner);
			Script_FieldChanged(owner, parent, field);
		}
		return;
	}
	case FT_SFImage:
	{
		if ( JS_InstanceOf(c, obj, &SFImageClass, NULL) ) {
			p = (JSField *) JS_GetPrivate(c, obj);
			VRML_FieldCopy(field->far_ptr, p->field.far_ptr, FT_SFImage);
			Script_FieldChanged(owner, parent, field);
		}
		return;
	}
	default:
		break;
	}

	//from here we handle only MF fields 
	if ( !JS_InstanceOf(c, obj, &MFBoolClass, NULL)
		&& !JS_InstanceOf(c, obj, &MFInt32Class, NULL)
		&& !JS_InstanceOf(c, obj, &MFFloatClass, NULL)
		&& !JS_InstanceOf(c, obj, &MFTimeClass, NULL)
		&& !JS_InstanceOf(c, obj, &MFStringClass, NULL)
		&& !JS_InstanceOf(c, obj, &MFUrlClass, NULL)
		&& !JS_InstanceOf(c, obj, &MFVec2fClass, NULL)
		&& !JS_InstanceOf(c, obj, &MFVec3fClass, NULL)
		&& !JS_InstanceOf(c, obj, &MFRotationClass, NULL)
		&& !JS_InstanceOf(c, obj, &MFColorClass, NULL)
		&& !JS_InstanceOf(c, obj, &MFNodeClass, NULL)
/*
		&& !JS_InstanceOf(c, obj, &MFVec4fClass, NULL)
*/
		) return;


	p = (JSField *) JS_GetPrivate(c, obj);
	JS_GetArrayLength(c, p->js_list, &len);

	/*special handling for MF node, reset list first*/
	if (JS_InstanceOf(c, obj, &MFNodeClass, NULL)) {
		SFNode *child;
		Chain *list = * (Chain **) field->far_ptr;
		while (ChainGetCount(list)) {
			child = ChainGetEntry(list, 0);
			ChainDeleteEntry(list, 0);
			Node_Unregister(child, owner);
		}
	
		for (i=0; i<len; i++) {
			JSObject *node_obj;
			JS_GetElement(c, p->js_list, (jsint) i, &item);
			if (JSVAL_IS_NULL(item)) break;
			node_obj = JSVAL_TO_OBJECT(item);
			if ( !JS_InstanceOf(c, node_obj, &SFNodeClass, NULL)) break;
			from = (JSField *) JS_GetPrivate(c, node_obj);

			child = * ((SFNode**)from->field.far_ptr);

			ChainAddEntry(list, child);
			Node_Register(child, owner);
		}
		Script_FieldChanged(owner, parent, field);
		return;
	}
	
	/*again, check text changes*/
	changed = (field->fieldType == FT_MFString) ? 0 : 1;
	/*realloc*/
	if (len != ((GenMFField *)field->far_ptr)->count) {
		VRML_MF_Reset(field->far_ptr, field->fieldType);
		VRML_MF_Alloc(field->far_ptr, field->fieldType, len);
		changed = 1;
	}
	/*assign each slot*/
	for (i=0; i<len; i++) {
		JS_GetElement(c, p->js_list, (jsint) i, &item);

		switch (field->fieldType) {
		case FT_MFBool:
			if (JSVAL_IS_BOOLEAN(item)) {
				((MFBool*)field->far_ptr)->vals[i] = (Bool) JSVAL_TO_BOOLEAN(item);
			}
			break;
		case FT_MFInt32:
			if (JSVAL_IS_INT(item)) {
				((MFInt32 *)field->far_ptr)->vals[i] = (s32) JSVAL_TO_INT(item);
			}
			break;
		case FT_MFFloat:
			if (JSVAL_IS_NUMBER(item)) {
				JS_ValueToNumber(c, item, &d);
				((MFFloat *)field->far_ptr)->vals[i] = (Float) d;
			}
			break;
		case FT_MFTime:
			if (JSVAL_IS_NUMBER(item)) {
				JS_ValueToNumber(c, item, &d);
				((MFTime *)field->far_ptr)->vals[i] = d;
			}
			break;
		case FT_MFString:
			if (JSVAL_IS_STRING(item)) {
				MFString *mfs = (MFString *) field->far_ptr;
				JSString *str = JSVAL_TO_STRING(item);
				char *str_val = JS_GetStringBytes(str);
				if (!mfs->vals[i] || strcmp(str_val, mfs->vals[i]) ) {
					if (mfs->vals[i]) free(mfs->vals[i]);
					mfs->vals[i] = strdup(str_val);
					changed = 1;
				}
			}
			break;
		case FT_MFURL:
			if (JSVAL_IS_STRING(item)) {
				MFURL *mfu = (MFURL *) field->far_ptr;
				JSString *str = JSVAL_TO_STRING(item);
				if (mfu->vals[i].url) free(mfu->vals[i].url);
				mfu->vals[i].url = strdup(JS_GetStringBytes(str));
				mfu->vals[i].OD_ID = 0;
			}
			break;

		case FT_MFVec2f:
			if ( JSVAL_IS_OBJECT(item) && JS_InstanceOf(c, JSVAL_TO_OBJECT(item), &SFVec2fClass, NULL) ) {
				from = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(item));
				VRML_FieldCopy(& ((MFVec2f*)field->far_ptr)->vals[i], from->field.far_ptr, FT_SFVec2f);
			}
			break;
		case FT_MFVec3f:
			if ( JSVAL_IS_OBJECT(item) && JS_InstanceOf(c, JSVAL_TO_OBJECT(item), &SFVec3fClass, NULL) ) {
				from = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(item));
				VRML_FieldCopy(& ((MFVec3f*)field->far_ptr)->vals[i], from->field.far_ptr, FT_SFVec3f);
			}
			break;
		case FT_MFRotation:
			if ( JSVAL_IS_OBJECT(item) && JS_InstanceOf(c, JSVAL_TO_OBJECT(item), &SFRotationClass, NULL) ) {
				from = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(item));
				VRML_FieldCopy(& ((MFRotation*)field->far_ptr)->vals[i], from->field.far_ptr, FT_SFRotation);
			}
			break;
		case FT_MFColor:
			if ( JSVAL_IS_OBJECT(item) && JS_InstanceOf(c, JSVAL_TO_OBJECT(item), &SFColorClass, NULL) ) {
				SFColor *col;
				from = (JSField *) JS_GetPrivate(c, JSVAL_TO_OBJECT(item));
				col = (SFColor *)from->field.far_ptr;
				VRML_FieldCopy(& ((MFColor*)field->far_ptr)->vals[i], from->field.far_ptr, FT_SFColor);
			}
			break;

		default:
			return;
		}
	}
	if (changed) Script_FieldChanged(owner, parent, field);
}

#define SETUP_FIELD	\
		jsf = NewJSField();	\
		jsf->owner = parent;	\
		if(parent) Node_GetField(parent, field->fieldIndex, &jsf->field);	\

#define SETUP_MF_FIELD	\
		jsf = (JSField *) JS_GetPrivate(priv->js_ctx, obj);	\
		jsf->owner = parent;		\
		if (parent) Node_GetField(parent, field->fieldIndex, &jsf->field);	\



jsval JS_ToJSField(ScriptPriv *priv, FieldInfo *field, SFNode *parent)
{
	u32 i;
	JSObject *obj = NULL;
	JSField *jsf = NULL;
	JSField *slot = NULL;
	SFNode *n;
	jsdouble *d;
	jsval newVal;
	JSString *s;


	/*look into object bank in case we already have this object*/
	if (parent) {
		for (i=0; i<ChainGetCount(priv->obj_bank); i++) {
			obj = ChainGetEntry(priv->obj_bank, i);
			jsf = (JSField *) JS_GetPrivate(priv->js_ctx, obj);
			if (jsf && (jsf->owner==parent) && (jsf->field.fieldIndex == field->fieldIndex)) {
				/*we need to rebuild MF types where SF is a native type.*/
				switch (jsf->field.fieldType) {
				case FT_MFBool:
				{
					MFBool *f = (MFBool *) field->far_ptr;
					JS_SetArrayLength(priv->js_ctx, jsf->js_list, 0);
					JS_SetArrayLength(priv->js_ctx, jsf->js_list, f->count);
					for (i=0; i<f->count; i++) {
						newVal = BOOLEAN_TO_JSVAL(f->vals[i]);
						JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
					}
				}
					break;
				case FT_MFInt32:
				{
					MFInt32 *f = (MFInt32 *) field->far_ptr;
					JS_SetArrayLength(priv->js_ctx, jsf->js_list, 0);
					JS_SetArrayLength(priv->js_ctx, jsf->js_list, f->count);
					for (i=0; i<f->count; i++) {
						newVal = INT_TO_JSVAL(f->vals[i]);
						JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
					}
				}
					break;
				case FT_MFFloat:
				{
					MFFloat *f = (MFFloat *) field->far_ptr;
					JS_SetArrayLength(priv->js_ctx, jsf->js_list, 0);
					JS_SetArrayLength(priv->js_ctx, jsf->js_list, f->count);
					for (i=0; i<f->count; i++) {
						newVal = DOUBLE_TO_JSVAL(JS_NewDouble(priv->js_ctx, f->vals[i]));
						JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
					}
				}
					break;
				case FT_MFTime:
				{
					MFTime *f = (MFTime *) field->far_ptr;
					JS_SetArrayLength(priv->js_ctx, jsf->js_list, 0);
					JS_SetArrayLength(priv->js_ctx, jsf->js_list, f->count);
					for (i=0; i<f->count; i++) {
						newVal = DOUBLE_TO_JSVAL(JS_NewDouble(priv->js_ctx, f->vals[i]));
						JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
					}
				}
					break;
				case FT_MFString:
				{
					MFString *f = (MFString *) field->far_ptr;
					JS_SetArrayLength(priv->js_ctx, jsf->js_list, 0);
					JS_SetArrayLength(priv->js_ctx, jsf->js_list, f->count);
					for (i=0; i<f->count; i++) {
						s = JS_NewStringCopyZ(priv->js_ctx, f->vals[i]);
						newVal = STRING_TO_JSVAL( s );
						JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
					}
				}
					break;
				case FT_MFURL:
				{
					MFURL *f = (MFURL *) field->far_ptr;
					JS_SetArrayLength(priv->js_ctx, jsf->js_list, 0);
					JS_SetArrayLength(priv->js_ctx, jsf->js_list, f->count);
					for (i=0; i<f->count; i++) {
						if (f->vals[i].OD_ID > 0) {
							char msg[30];
							sprintf(msg, "od:%d", f->vals[i].OD_ID);
							s = JS_NewStringCopyZ(priv->js_ctx, (const char *) msg);
						} else {
							s = JS_NewStringCopyZ(priv->js_ctx, f->vals[i].url);
						}
						newVal = STRING_TO_JSVAL(s);
						JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
					}
				}
					break;
				}
				return OBJECT_TO_JSVAL(obj);
			}
		}
	}

	switch (field->fieldType) {
    case FT_SFBool:
		return BOOLEAN_TO_JSVAL( * ((SFBool *) field->far_ptr) );
	case FT_SFInt32:
		return INT_TO_JSVAL(  * ((SFInt32 *) field->far_ptr));
    case FT_SFFloat:
		d = JS_NewDouble(priv->js_ctx, * ((SFFloat *) field->far_ptr));
		return DOUBLE_TO_JSVAL(d);
	case FT_SFTime:
		d = JS_NewDouble(priv->js_ctx, * ((SFTime *) field->far_ptr));
		return DOUBLE_TO_JSVAL(d);
	case FT_SFString:
    {
		s = JS_NewStringCopyZ(priv->js_ctx, ((SFString *) field->far_ptr)->buffer);
		return STRING_TO_JSVAL( s );
    }
	case FT_SFURL:
    {
		SFURL *url = (SFURL *)field->far_ptr;
		if (url->OD_ID > 0) {
			char msg[30];
			sprintf(msg, "od:%d", url->OD_ID);
			s = JS_NewStringCopyZ(priv->js_ctx, (const char *) msg);
		} else {
			s = JS_NewStringCopyZ(priv->js_ctx, (const char *) url->url);
		}
		return STRING_TO_JSVAL( s );
    }
    case FT_SFVec2f:
		SETUP_FIELD
		obj = JS_NewObject(priv->js_ctx, &SFVec2fClass, 0, priv->js_obj);
		break;
    case FT_SFVec3f:
		SETUP_FIELD
		obj = JS_NewObject(priv->js_ctx, &SFVec3fClass, 0, priv->js_obj);
		break;
    case FT_SFRotation:
		SETUP_FIELD
		obj = JS_NewObject(priv->js_ctx, &SFRotationClass, 0, priv->js_obj);
		break;
    case FT_SFColor:
		SETUP_FIELD
		obj = JS_NewObject(priv->js_ctx, &SFColorClass, 0, priv->js_obj);
		break;
    case FT_SFImage:
		SETUP_FIELD
		obj = JS_NewObject(priv->js_ctx, &SFImageClass, 0, priv->js_obj);
		break;
	case FT_SFNode:
		SETUP_FIELD
		obj = JS_NewObject(priv->js_ctx, &SFNodeClass, 0, priv->js_obj);
		break;


	case FT_MFBool:
	{
		MFBool *f = (MFBool *) field->far_ptr;
		obj = JS_ConstructObject(priv->js_ctx, &MFBoolClass, 0, priv->js_obj);
		SETUP_MF_FIELD
		for (i = 0; i<f->count; i++) {
			jsval newVal = BOOLEAN_TO_JSVAL(f->vals[i]);
			JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
		}
		break;
	}
	case FT_MFInt32:
	{
		MFInt32 *f = (MFInt32 *) field->far_ptr;
		obj = JS_ConstructObject(priv->js_ctx, &MFInt32Class, 0, priv->js_obj);
		SETUP_MF_FIELD
		for (i=0; i<f->count; i++) {
			jsval newVal = INT_TO_JSVAL(f->vals[i]);
			JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
		}
		break;
	}
	case FT_MFFloat:
	{
		MFFloat *f = (MFFloat *) field->far_ptr;
		obj = JS_ConstructObject(priv->js_ctx, &MFFloatClass, 0, priv->js_obj);
		SETUP_MF_FIELD
		for (i=0; i<f->count; i++) {
			jsval newVal = DOUBLE_TO_JSVAL(JS_NewDouble(priv->js_ctx, f->vals[i]));
			JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
		}
		break;
	}
	case FT_MFTime:
	{
		MFTime *f = (MFTime *) field->far_ptr;
		obj = JS_ConstructObject(priv->js_ctx, &MFTimeClass, 0, priv->js_obj);
		SETUP_MF_FIELD
		for (i=0; i<f->count; i++) {
			jsval newVal = DOUBLE_TO_JSVAL( JS_NewDouble(priv->js_ctx, f->vals[i]) );
			JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
		}
		break;
	}
	case FT_MFString:
	{
		MFString *f = (MFString *) field->far_ptr;
		obj = JS_ConstructObject(priv->js_ctx, &MFStringClass, 0, priv->js_obj);
		SETUP_MF_FIELD
		for (i=0; i<f->count; i++) {
			s = JS_NewStringCopyZ(priv->js_ctx, f->vals[i]);
			newVal = STRING_TO_JSVAL( s );
			JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
		}
		break;
	}
	case FT_MFURL:
	{
		MFURL *f = (MFURL *) field->far_ptr;
		obj = JS_ConstructObject(priv->js_ctx, &MFUrlClass, 0, priv->js_obj);
		SETUP_MF_FIELD
		for (i=0; i<f->count; i++) {
			if (f->vals[i].OD_ID > 0) {
				char msg[30];
				sprintf(msg, "od:%d", f->vals[i].OD_ID);
				s = JS_NewStringCopyZ(priv->js_ctx, (const char *) msg);
			} else {
				s = JS_NewStringCopyZ(priv->js_ctx, f->vals[i].url);
			}
			newVal = STRING_TO_JSVAL( s );
			JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
		}
		break;
	}

	case FT_MFVec2f:
	{
		MFVec2f *f = (MFVec2f *) field->far_ptr;
		obj = JS_ConstructObject(priv->js_ctx, &MFVec2fClass, 0, priv->js_obj);
		SETUP_MF_FIELD
		for (i=0; i<f->count; i++) {
			JSObject *pf = JS_NewObject(priv->js_ctx, &SFVec2fClass, 0, obj);
			newVal = OBJECT_TO_JSVAL(pf);
			slot = SFVec2f_Create(priv->js_ctx, pf, f->vals[i].x, f->vals[i].y);
			slot->owner = parent;
			JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
		}
		break;
	}
	case FT_MFVec3f:
	{
		MFVec3f *f = (MFVec3f *) field->far_ptr;
		obj = JS_ConstructObject(priv->js_ctx, &MFVec3fClass, 0, priv->js_obj);
		SETUP_MF_FIELD
		for (i=0; i<f->count; i++) {
			JSObject *pf = JS_NewObject(priv->js_ctx, &SFVec3fClass, 0, obj);
			newVal = OBJECT_TO_JSVAL(pf);
			slot = SFVec3f_Create(priv->js_ctx, pf, f->vals[i].x, f->vals[i].y, f->vals[i].z);
			slot->owner = parent;
			JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
		}
		break;
	}
	case FT_MFRotation:
	{
		MFRotation *f = (MFRotation*) field->far_ptr;
		obj = JS_ConstructObject(priv->js_ctx, &MFRotationClass, 0, priv->js_obj);
		SETUP_MF_FIELD
		for (i=0; i<f->count; i++) {
			JSObject *pf = JS_NewObject(priv->js_ctx, &SFRotationClass, 0, obj);
			newVal = OBJECT_TO_JSVAL(pf);
			slot = SFRotation_Create(priv->js_ctx, pf, f->vals[i].xAxis, f->vals[i].yAxis, f->vals[i].zAxis, f->vals[i].angle);
			slot->owner = parent;
			JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
		}
		break;
	}
	case FT_MFColor:
	{
		MFColor *f = (MFColor *) field->far_ptr;
		obj = JS_ConstructObject(priv->js_ctx, &MFColorClass, 0, priv->js_obj);
		SETUP_MF_FIELD
		for (i=0; i<f->count; i++) {
			JSObject *pf = JS_NewObject(priv->js_ctx, &SFColorClass, 0, obj);
			newVal = OBJECT_TO_JSVAL(pf);
			slot = SFColor_Create(priv->js_ctx, pf, f->vals[i].red, f->vals[i].green, f->vals[i].blue);
			slot->owner = parent;
			JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
		}
		break;
	}

	case FT_MFNode:
	{
		u32 size;
		Chain *f = * ((Chain**)field->far_ptr);
		obj = JS_ConstructObject(priv->js_ctx, &MFNodeClass, 0, priv->js_obj);
		SETUP_MF_FIELD
		size = ChainGetCount(f);
		if (JS_SetArrayLength(priv->js_ctx, jsf->js_list, size) != JS_TRUE) return JSVAL_NULL;
		for (i=0; i<size; i++) {
			JSObject *pf = JS_NewObject(priv->js_ctx, &SFNodeClass, 0, obj);
			n = ChainGetEntry(f, i);
			if (parent) {
				slot = NewJSField();
				slot->owner = parent;
			} else {
				slot = NewJSField();
			}
			slot->temp_node = n;
			slot->field.far_ptr = & slot->temp_node;
			slot->field.fieldType = FT_SFNode;
			Node_Register(n, parent);
			JS_SetPrivate(priv->js_ctx, pf, slot);

			newVal = OBJECT_TO_JSVAL(pf);
			JS_SetElement(priv->js_ctx, jsf->js_list, (jsint) i, &newVal);
		}
		break;
	}

	//not supported
    default:
		return JSVAL_NULL;
    }

	if (!obj) return JSVAL_NULL;
	//store field associated with object if needed
	if (jsf) {
		JS_SetPrivate(priv->js_ctx, obj, jsf);
		/*if this is the obj corresponding to an existing node, store it*/
		if (parent) ChainAddEntry(priv->obj_bank, obj);
	}
	return OBJECT_TO_JSVAL(obj);

}



/*all spidermonkey specific stuff*/
static JSRuntime *js_runtime = 0;
static u32 nb_inst = 0;
const long MAX_HEAP_BYTES = 4L * 1024L * 1024L;
const long STACK_CHUNK_BYTES = 4024L;

static void JS_PreDestroy(SFNode *node)
{
	jsval fval, rval;
	ScriptPriv *priv = node->sgprivate->privateStack;
	if (!priv) return;
	
	if (JS_LookupProperty(priv->js_ctx, priv->js_obj, "shutdown", &fval))
		if (! JSVAL_IS_VOID(fval))
			JS_CallFunctionValue(priv->js_ctx, priv->js_obj, fval, 0, NULL, &rval);

	JS_DestroyContext(priv->js_ctx);
	nb_inst --;
	if (nb_inst == 0) {
		JS_DestroyRuntime(js_runtime);
		JS_ShutDown();
		js_runtime = 0;
	}
	if (priv->obj_bank) DeleteChain(priv->obj_bank);
}

static void JS_InitScriptFields(ScriptPriv *priv, SFNode *sc)
{
	u32 i;
	ScriptField *sf;
	FieldInfo info;
	jsval val;

    for (i=0; i < ChainGetCount(priv->fields); i++) { 
		sf = ChainGetEntry(priv->fields, i);

		switch (sf->eventType) {
		case ET_EventIn:
			break;
		case ET_EventOut:
			Node_GetField(sc, sf->ALL_index, &info);
			val = JS_ToJSField(priv, &info, sc);
			/*for native types directly modified*/
			JS_DefineProperty(priv->js_ctx, priv->js_obj, (const char *) sf->name, val, 0, eventOut_setProperty, JSPROP_PERMANENT );
			break;
		default:
			Node_GetField(sc, sf->ALL_index, &info);
			val = JS_ToJSField(priv, &info, sc);
			JS_DefineProperty(priv->js_ctx, priv->js_obj, (const char *) sf->name, val, 0, 0, JSPROP_PERMANENT);
			break;
		}
    }
}

static void JS_EventIn(SFNode *node, FieldInfo *in_field)
{
	jsval fval, rval;
	Double time;
	jsval argv[2];
	ScriptField *sf;
	FieldInfo t_info;
	ScriptPriv *priv;
	u32 i;	
	priv = node->sgprivate->privateStack;

	/*no support for change of static fields*/
	if (in_field->fieldIndex<3) return;
	
	i = (node->sgprivate->tag==TAG_MPEG4_Script) ? 3 : 4;
	sf = ChainGetEntry(priv->fields, in_field->fieldIndex - i);
	time = Node_GetSceneTime(node);

	/*
	if (sf->last_route_time == time) return; 
	*/
	sf->last_route_time = time;

	//locate function
	if (! JS_LookupProperty(priv->js_ctx, priv->js_obj, sf->name, &fval)) return;
	if (JSVAL_IS_VOID(fval)) return;

	argv[0] = JS_ToJSField(priv, in_field, node);

	memset(&t_info, 0, sizeof(FieldInfo));
	t_info.far_ptr = &sf->last_route_time;
	t_info.fieldType = FT_SFTime;
	t_info.fieldIndex = -1;
	argv[1] = JS_ToJSField(priv, &t_info, node);

	/*protect args*/
	if (JSVAL_IS_GCTHING(argv[0])) JS_AddRoot(priv->js_ctx, &argv[0]);
	if (JSVAL_IS_GCTHING(argv[1])) JS_AddRoot(priv->js_ctx, &argv[1]);

	JS_CallFunctionValue(priv->js_ctx, priv->js_obj, fval, 2, argv, &rval);

	/*release args*/
	if (JSVAL_IS_GCTHING(argv[0])) JS_RemoveRoot(priv->js_ctx, &argv[0]);
	if (JSVAL_IS_GCTHING(argv[1])) JS_RemoveRoot(priv->js_ctx, &argv[1]);

	/*flush event out*/
	for (i=0; i<ChainGetCount(priv->fields); i++) {
		sf = ChainGetEntry(priv->fields, i);
		if (sf->activate_event_out) {
			sf->activate_event_out = 0;
			Node_OnEventOut(node, sf->ALL_index);
		}
	}
}

void JSScriptFromFile(SFNode *node);

static void JSFileFetched(void *cbck, Bool success, const char *file_cached)
{
	FILE *jsf;
	M_Script *script = (M_Script *)cbck;

	
	/*OK got the file*/
	if (success) {
		jsf = fopen(file_cached, "rt");
		if (!jsf) success = 0;
		else {
			char *jsscript;
			u32 fsize;
			JSBool ret;
			jsval rval, fval;
			ScriptPriv *priv = (ScriptPriv *) script->sgprivate->privateStack;


			fseek(jsf, 0, SEEK_END);
			fsize = ftell(jsf);
			fseek(jsf, 0, SEEK_SET);
			jsscript = malloc(sizeof(char)*fsize);
			fread(jsscript, sizeof(char)*fsize, 1, jsf);
			fclose(jsf);

			ret = JS_EvaluateScript(priv->js_ctx, priv->js_obj, jsscript, sizeof(char)*fsize, 0, 0, &rval);
			if (ret==JS_FALSE) success = 0;

			if (success && JS_LookupProperty(priv->js_ctx, priv->js_obj, "initialize", &fval)) {
				if (! JSVAL_IS_VOID(fval)) {
					JS_CallFunctionValue(priv->js_ctx, priv->js_obj, fval, 0, NULL, &rval);
				}
			}
			free(jsscript);
			if (success) return;
		}
	}
	
	if (!success) {
		u32 i;
		if (script->url.count<=1) return;
		free(script->url.vals[0].script_text);
		for (i=0; i<script->url.count-1; i++) 
			script->url.vals[i].script_text = script->url.vals[i+1].script_text;
		script->url.vals[script->url.count-1].script_text = NULL;
		script->url.count -= 1;
		JSScriptFromFile((SFNode *)script);
		return;
	}

}

void JSScriptFromFile(SFNode *node)
{
	u32 i;
	JSInterface *ifce;
	char szExt[50], *ext;
	M_Script *script = (M_Script *)node;
	Bool can_dnload = 0;

	for (i=0; i<script->url.count; i++) {
		ext = strrchr(script->url.vals[i].script_text, '.');
		if (!ext) break;
		strcpy(szExt, ext);
		strlwr(szExt);
		if (strcmp(szExt, ".js")) continue;
		can_dnload = 1;
		break;
	}
	if (!can_dnload) return;
	
	ifce = node->sgprivate->scenegraph->js_ifce;
	ifce->GetScriptFile(ifce->callback, node->sgprivate->scenegraph, script->url.vals[0].script_text, JSFileFetched, node);
}

static void JSScript_Load(SFNode *node)
{
	char *str;
	JSBool ret;
	u32 i;
	Bool local_script;
	jsval rval, fval;
	M_Script *script = (M_Script *)node;
	ScriptPriv *priv = (ScriptPriv *) node->sgprivate->privateStack;

	if (!priv || priv->is_loaded) return;
	if (!script->url.count) return;
	priv->is_loaded = 1;

	str = NULL;
	for (i=0; i<script->url.count; i++) {
		str = script->url.vals[i].script_text;
		if (!strnicmp(str, "javascript:", 11)) str += 11;
		else if (!strnicmp(str, "vrmlscript:", 11)) str += 11;
		else if (!strnicmp(str, "ecmascript:", 11)) str += 11;
		else if (!strnicmp(str, "mpeg4script:", 12)) str += 12;
		else str = NULL;
		if (str) break;
	}
	local_script = str ? 1 : 0;

	/*JS load*/
	if (!js_runtime) {
		js_runtime = JS_NewRuntime(MAX_HEAP_BYTES);
		assert(js_runtime);
	}
	priv->js_ctx = JS_NewContext(js_runtime, STACK_CHUNK_BYTES);
	assert(priv->js_ctx);
	nb_inst++;
	JS_SetContextPrivate(priv->js_ctx, node);
	init_spidermonkey_api(priv, node);

	priv->obj_bank = NewChain();

	/*setup fields interfaces*/
	JS_InitScriptFields(priv, node);

	priv->JS_PreDestroy = JS_PreDestroy;
	priv->JS_EventIn = JS_EventIn;

	if (!local_script) {
		JSScriptFromFile(node);
		return;
	}

	ret = JS_EvaluateScript(priv->js_ctx, priv->js_obj, str, strlen(str), 0, 0, &rval);
	if (ret==JS_FALSE) return;

	/*call initialize if present*/
	if (! JS_LookupProperty(priv->js_ctx, priv->js_obj, "initialize", &fval)) return;
	if (JSVAL_IS_VOID(fval)) return;
	JS_CallFunctionValue(priv->js_ctx, priv->js_obj, fval, 0, NULL, &rval);
}

#endif


/*set JavaScript interface*/
void SG_SetJavaScriptAPI(LPSCENEGRAPH scene, JSInterface *ifce)
{
	if (!scene) return;
	scene->js_ifce = ifce;

#ifdef M4_USE_SPIDERMONKEY
	scene->Script_Load = JSScript_Load;
#endif

}

