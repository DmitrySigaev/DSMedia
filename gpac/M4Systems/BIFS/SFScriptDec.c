/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / BIFS codec sub-project
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


#include "SFScript.h"

#define BINOP_MINVAL ET_EQ

static char* binaryOp[] =
{
	"==", "!=","<", "<=",
	">", ">=", "+", "-",
	"*", "/", "%",
	"&&", "||",
	"&", "|", "^",
	"<<", ">>", ">>>"
};

static char* assignmentOp[] =
{
	"=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", ">>>=",
	"&=", "^=", "|="
};

typedef struct 
{
	SFNode *script;
	BifsDecoder *codec;
	BitStream *bs;
	char *string;
	u32 length;
	Chain *identifiers;
	char *new_line;
	u32 indent;
} ScriptParser;


void SFS_Identifier(ScriptParser *parser);
void SFS_Arguments(ScriptParser *parser);
void SFS_StatementBlock(ScriptParser *parser, Bool functBody);
void SFS_Statement(ScriptParser *parser);
void SFS_IfStatement(ScriptParser *parser);
void SFS_SwitchStatement(ScriptParser *parser);
void SFS_ForStatement(ScriptParser *parser);
void SFS_WhileStatement(ScriptParser *parser);
void SFS_ReturnStatement(ScriptParser *parser);
void SFS_CompoundExpression(ScriptParser *parser);
void SFS_OptionalExpression(ScriptParser *parser);
void SFS_Expression(ScriptParser *parser);
void SFS_NewObject(ScriptParser *parser);
void SFS_ArrayDeref(ScriptParser *parser);
void SFS_FunctionCall(ScriptParser *parser);
void SFS_ObjectMemberAccess(ScriptParser *parser);
void SFS_ObjectMethodCall(ScriptParser *parser);
void SFS_Params(ScriptParser *parser);
void SFS_GetNumber(ScriptParser *parser);
void SFS_GetString(ScriptParser *parser);
void SFS_GetBoolean(ScriptParser *parser);
#define PARSER_STEP_ALLOC	500

static void SFS_AddString(ScriptParser *parser, char *str)
{
	char *new_str;
	if (!str) return;
	if (strlen(parser->string) + strlen(str) >= parser->length) {
		parser->length += PARSER_STEP_ALLOC;
		new_str = malloc(sizeof(char)*parser->length);
		strcpy(new_str, parser->string);
		free(parser->string);
		parser->string = new_str;
	}
	strcat(parser->string, str);
}

static void SFS_AddInt(ScriptParser *parser, s32 val)
{
	char msg[500];
	sprintf(msg, "%d", val);
	SFS_AddString(parser, msg);
}
static void SFS_AddChar(ScriptParser *parser, char c)
{
	char msg[2];
	sprintf(msg, "%c", c);
	SFS_AddString(parser, msg);
}


M4Err ParseScriptField(ScriptParser *parser)
{
	LPSCRIPTFIELD field;
	M4Err e;
	u32 eventType, fieldType;
	char name[1000];
	FieldInfo info;

	eventType = BS_ReadInt(parser->bs, 2);
	fieldType = BS_ReadInt(parser->bs, 6);
	BD_GetName(parser->bs, name);
	field = SG_NewScriptField(parser->script, eventType, fieldType, name);
	if (!field) return M4NonCompliantBitStream;

	//save the name in the list of identifiers
	ChainAddEntry(parser->identifiers, strdup(name));

	if (parser->codec->pCurrentProto) {
		Bool isISfield = BS_ReadInt(parser->bs, 1);
		if (isISfield) {
			u32 numProtoField = Proto_GetFieldCount(parser->codec->pCurrentProto);
			u32 numBits = GetNumBits(numProtoField - 1);
			u32 field_all = BS_ReadInt(parser->bs, numBits);
			e = ScriptField_GetInfo(field, &info);
			if (e) return e;
			e = Proto_SetISField(parser->codec->pCurrentProto, field_all, parser->script, info.fieldIndex);
			return e;
		}
	}
	/*get default value*/
	if (eventType == SFET_Field) {
		if (BS_ReadInt(parser->bs, 1)) {
			e = ScriptField_GetInfo(field, &info);
			if (e) return e;
			BD_DecField(parser->codec, parser->bs, parser->script, &info);
		}
	}

	return parser->codec->LastError;
}

static void SFS_IncIndent(ScriptParser *pars) { pars->indent++; }
static void SFS_DecIndent(ScriptParser *pars) { pars->indent--; }
static void SFS_Space(ScriptParser *pars) { if (pars->new_line) SFS_AddString(pars, " ");}
static void SFS_Indent(ScriptParser *pars) 
{
	u32 i;
	if (pars->new_line) {
		for (i=0; i<pars->indent; i++) SFS_AddString(pars, " ");
	}
}
static M4INLINE void SFS_Line(ScriptParser *parser) 
{
	if (parser->new_line) {
		SFS_AddString(parser, parser->new_line);
	}
}



M4Err SFScript_Parse(BifsDecoder *codec, BitStream *bs, SFNode *n)
{
	M4Err e;
	u32 i, count, nbBits;
	char *ptr;
	ScriptParser parser;
	M_Script *pNode = (M_Script *)n;
	e = M4OK;
	if (Node_GetTag(n) != TAG_MPEG4_Script) return M4NonCompliantBitStream;

	parser.codec = codec;
	parser.script = n;
	parser.bs = bs;
	parser.length = 500;
	parser.string = (char *) malloc(sizeof(char)* parser.length);
	parser.string[0] = 0;
	parser.identifiers = NewChain();
	parser.new_line = codec->dec_memory_mode ? "\n" : NULL;
	parser.indent = 0;
	//reset URL
	VRML_MF_Reset(&pNode->url, FT_MFScript);

	//first parse fields

	if (BS_ReadInt(bs, 1)) {
		//endFlag
		while (!BS_ReadInt(bs, 1)){
			e = ParseScriptField(&parser);
			if (e) goto exit;
		}
	} else {
		nbBits = BS_ReadInt(bs, 4);
		count = BS_ReadInt(bs, nbBits);
		for (i=0; i<count; i++) {
			e = ParseScriptField(&parser);
			if (e) goto exit;
		}
	}
	//reserevd
	BS_ReadInt(bs, 1);
	//then parse
	SFS_AddString(&parser, "javascript:");
	SFS_AddString(&parser, parser.new_line);

	//hasFunction
	while (BS_ReadInt(bs, 1)) {
		SFS_AddString(&parser, "function ");
		SFS_Identifier(&parser);
		SFS_Arguments(&parser);
		SFS_Space(&parser);
		SFS_StatementBlock(&parser, 1);
		SFS_Line(&parser);
	}

	SFS_Line(&parser);

	VRML_MF_Alloc(&pNode->url, FT_MFScript, 1);
	pNode->url.count = 1;
	pNode->url.vals[0].script_text = strdup(parser.string);

exit:
	//clean up
	while (ChainGetCount(parser.identifiers)) {
		ptr = ChainGetEntry(parser.identifiers, 0);
		free(ptr);
		ChainDeleteEntry(parser.identifiers, 0);
	}
	DeleteChain(parser.identifiers);
	if (parser.string) free(parser.string);
	return e;
}



void SFS_Identifier(ScriptParser *parser)
{
	u32 index;
	char name[500];

	if (parser->codec->LastError) return;

	//recieved
	if (BS_ReadInt(parser->bs, 1)) {
		index = BS_ReadInt(parser->bs, GetNumBits(ChainGetCount(parser->identifiers) - 1));
		SFS_AddString(parser, ChainGetEntry(parser->identifiers, index));
	}
	//parse
	else{
		BD_GetName(parser->bs, name);
		ChainAddEntry(parser->identifiers, strdup(name));
		SFS_AddString(parser, name);
	}
}

void SFS_Arguments(ScriptParser *parser)
{
	u32 val;
	if (parser->codec->LastError) return;
	SFS_AddString(parser, "(");
	
	val = BS_ReadInt(parser->bs, 1);
	while (val) {
		SFS_Identifier(parser);
		val = BS_ReadInt(parser->bs, 1);
		if (val) SFS_AddString(parser, ",");
	}
	SFS_AddString(parser, ")");
}

void SFS_StatementBlock(ScriptParser *parser, Bool funcBody)
{
	if (parser->codec->LastError) return;

    if (BS_ReadInt(parser->bs, 1)) {
		SFS_AddString(parser, "{");
		SFS_IncIndent(parser);

		while (BS_ReadInt(parser->bs, 1)) {
			SFS_Line(parser);
			SFS_Indent(parser);
			SFS_Statement(parser);
		}
		SFS_DecIndent(parser);
		SFS_Line(parser);
		SFS_Indent(parser);
		SFS_AddString(parser, "}");
	} else if (funcBody) {
		SFS_AddString(parser, "{");
		SFS_Statement(parser);
		SFS_AddString(parser, "}");
	} else {
		SFS_Statement(parser);
	}
}


void SFS_Statement(ScriptParser *parser)
{
	u32 val;
	if (parser->codec->LastError) return;

	val = BS_ReadInt(parser->bs, NUMBITS_STATEMENT);
	switch (val) {
	case ST_IF:
		SFS_IfStatement(parser);
		break;
	case ST_FOR:
		SFS_ForStatement(parser);
		break;
	case ST_WHILE:
		SFS_WhileStatement(parser);
		break;
	case ST_RETURN:
		SFS_ReturnStatement(parser);
		break;
	case ST_BREAK:
		SFS_AddString(parser, "break;");
		break;
	case ST_CONTINUE:
		SFS_AddString(parser, "continue;");
		break;
	case ST_COMPOUND_EXPR:
		SFS_CompoundExpression(parser);
		SFS_AddString(parser, ";");
		break;
	case ST_SWITCH:
		SFS_SwitchStatement(parser);
		break;
	}
}

void SFS_IfStatement(ScriptParser *parser)
{
	if (parser->codec->LastError) return;
	SFS_AddString(parser, "if (");
	SFS_CompoundExpression(parser);
	SFS_AddString(parser, ") ");
	SFS_StatementBlock(parser, 0);
	//has else
	if (BS_ReadInt(parser->bs, 1)) {
		SFS_Line(parser);
		SFS_Indent(parser);
		SFS_AddString(parser, "else ");
		SFS_StatementBlock(parser, 0);
	}
}

void SFS_SwitchStatement(ScriptParser *parser)
{
	u32 numBits, caseValue;

	if (parser->codec->LastError) return;
	SFS_AddString(parser, "switch (");
	SFS_CompoundExpression(parser);
	SFS_AddString(parser, ")");
	SFS_AddString(parser, "{");
	SFS_Line(parser);

	numBits = BS_ReadInt(parser->bs, 5);
	do {
		SFS_Indent(parser);
		SFS_AddString(parser, "case ");
		caseValue = BS_ReadInt(parser->bs, numBits);
		SFS_AddInt(parser, caseValue);
		SFS_AddString(parser, ":");
		SFS_Line(parser);
		SFS_Indent(parser);
		SFS_StatementBlock(parser, 0);
		SFS_Line(parser);
	}
	while (BS_ReadInt(parser->bs, 1));

	//default
	if (BS_ReadInt(parser->bs, 1)) {
		SFS_AddString(parser, "default:");
		SFS_Line(parser);
		SFS_StatementBlock(parser, 0);
	}
	SFS_AddString(parser, "}");
}

void SFS_ForStatement(ScriptParser *parser)
{
	if (parser->codec->LastError) return;
	SFS_AddString(parser, "for (");
	SFS_OptionalExpression(parser);
	SFS_AddString(parser, ";");
	SFS_OptionalExpression(parser);
	SFS_AddString(parser, ";");
	SFS_OptionalExpression(parser);
	SFS_AddString(parser, ")");

	SFS_StatementBlock(parser, 0);
}

void SFS_WhileStatement(ScriptParser *parser)
{
	if (parser->codec->LastError) return;
	SFS_AddString(parser, "while (");
	SFS_CompoundExpression(parser);
	SFS_AddString(parser, ")");

	SFS_StatementBlock(parser, 0);
}

void SFS_ReturnStatement(ScriptParser *parser)
{
	if (parser->codec->LastError) return;
	SFS_AddString(parser, "return");
	if (BS_ReadInt(parser->bs, 1)) {
		SFS_AddString(parser, " ");
		SFS_CompoundExpression(parser);
	}
	SFS_AddString(parser, ";");
	SFS_Line(parser);
}

void SFS_CompoundExpression(ScriptParser *parser)
{
	if (parser->codec->LastError) return;
	SFS_Expression(parser);
	if (! BS_ReadInt(parser->bs, 1)) return;
	SFS_AddString(parser, ",");
	SFS_CompoundExpression(parser);
}

void SFS_OptionalExpression(ScriptParser *parser)
{
	if (parser->codec->LastError) return;
	if (BS_ReadInt(parser->bs, 1)) {
		SFS_CompoundExpression(parser);
	}
}


void SFS_Expression(ScriptParser *parser)
{
	u32 val = BS_ReadInt(parser->bs, NUMBITS_EXPR_TYPE);
	if (parser->codec->LastError) return;

	switch(val) {
	case ET_CURVED_EXPR:
		SFS_AddString(parser, "(");
		SFS_CompoundExpression(parser);
		SFS_AddString(parser, ")");
		break;
	case ET_NEGATIVE:
		SFS_AddString(parser, "-");
		SFS_Expression(parser);
		break;
	case ET_NOT:
		SFS_AddString(parser, "!");
		SFS_Expression(parser);
		break;
	case ET_ONESCOMP:
		SFS_AddString(parser, "~");
		SFS_Expression(parser);
		break;
	case ET_INCREMENT:
		SFS_AddString(parser, "++");
		SFS_Expression(parser);
		break;
	case ET_DECREMENT:
		SFS_AddString(parser, "--");
		SFS_Expression(parser);
		break;
	case ET_POST_INCREMENT:
		SFS_Expression(parser);
		SFS_AddString(parser, "++");
		break;
	case ET_POST_DECREMENT:
		SFS_Expression(parser);
		SFS_AddString(parser, "--");
		break;
	case ET_CONDTEST:
		SFS_Expression(parser);
		SFS_AddString(parser, " ? ");
		SFS_Expression(parser);
		SFS_AddString(parser, " : ");
		SFS_Expression(parser);
		break;
	case ET_STRING:
		SFS_AddString(parser, "'");
		SFS_GetString(parser);
		SFS_AddString(parser, "'");
		break;
	case ET_NUMBER:
		SFS_GetNumber(parser);
		break;
	case ET_IDENTIFIER:
		SFS_Identifier(parser);
		break;
	case ET_FUNCTION_CALL:
		SFS_FunctionCall(parser);
		break;
	case ET_NEW:
        SFS_NewObject(parser);
		break;
	case ET_OBJECT_MEMBER_ACCESS: 
   		SFS_ObjectMemberAccess(parser);
		break;
	case ET_OBJECT_METHOD_CALL:
        SFS_ObjectMethodCall(parser);
		break;
	case ET_ARRAY_DEREFERENCE:
        SFS_ArrayDeref(parser);
		break;

	case ET_MULTIPLY:
	case ET_DIVIDE:
	case ET_MOD:
	case ET_PLUS:
	case ET_MINUS:
	case ET_LSHIFT:
	case ET_RSHIFT:
	case ET_RSHIFTFILL:
	case ET_AND:
	case ET_XOR:
	case ET_OR:
		SFS_Expression(parser);
		SFS_AddString(parser, binaryOp[val - BINOP_MINVAL]);
		SFS_Expression(parser);
		break;
	case ET_LT:
	case ET_LE:
	case ET_GT:
	case ET_GE:
	case ET_EQ:
	case ET_NE:
	case ET_LAND:
	case ET_LOR:
		SFS_Expression(parser);
		SFS_AddString(parser, binaryOp[val - BINOP_MINVAL]);
		SFS_Expression(parser);
		break;
	case ET_ASSIGN:
		SFS_Expression(parser);
		SFS_AddString(parser, assignmentOp[0]);
		SFS_Expression(parser);
		break;
	case ET_PLUSEQ:
		SFS_Expression(parser);
		SFS_AddString(parser, assignmentOp[1]);
		SFS_Expression(parser);
		break;
	case ET_MINUSEQ:
		SFS_Expression(parser);
		SFS_AddString(parser, assignmentOp[2]);
		SFS_Expression(parser);
		break;
	case ET_MULTIPLYEQ:
		SFS_Expression(parser);
		SFS_AddString(parser, assignmentOp[3]);
		SFS_Expression(parser);
		break;
	case ET_DIVIDEEQ:
		SFS_Expression(parser);
		SFS_AddString(parser, assignmentOp[4]);
		SFS_Expression(parser);
		break;
	case ET_MODEQ:
		SFS_Expression(parser);
		SFS_AddString(parser, assignmentOp[5]);
		SFS_Expression(parser);
		break;
	case ET_LSHIFTEQ:
		SFS_Expression(parser);
		SFS_AddString(parser, assignmentOp[6]);
		SFS_Expression(parser);
		break;
	case ET_RSHIFTEQ:
		SFS_Expression(parser);
		SFS_AddString(parser, assignmentOp[7]);
		SFS_Expression(parser);
		break;
	case ET_RSHIFTFILLEQ:
		SFS_Expression(parser);
		SFS_AddString(parser, assignmentOp[8]);
		SFS_Expression(parser);
		break;
	case ET_ANDEQ:
		SFS_Expression(parser);
		SFS_AddString(parser, assignmentOp[9]);
		SFS_Expression(parser);
		break;
	case ET_XOREQ:
		SFS_Expression(parser);
		SFS_AddString(parser, assignmentOp[10]);
		SFS_Expression(parser);
		break;
	case ET_OREQ:
		SFS_Expression(parser);
		SFS_AddString(parser, assignmentOp[11]);
		SFS_Expression(parser);
		break;
    case ET_BOOLEAN: 
		SFS_GetBoolean(parser);
        break;
	default:
		break;
	}
}

void SFS_NewObject(ScriptParser *parser)
{
	if (parser->codec->LastError) return;
	SFS_AddString(parser, "new ");
    SFS_Identifier(parser);
	SFS_AddString(parser, "(");
	SFS_Params(parser);
	SFS_AddString(parser, ") ");
}

void SFS_ArrayDeref(ScriptParser *parser)
{
	if (parser->codec->LastError) return;
	SFS_Expression(parser);
    SFS_AddString(parser, "[");
	SFS_CompoundExpression(parser);
	SFS_AddString(parser, "]");
}

void SFS_FunctionCall(ScriptParser *parser)
{
	if (parser->codec->LastError) return;
	SFS_Identifier(parser);
	SFS_AddString(parser, "(");
	SFS_Params(parser);
	SFS_AddString(parser, ")");
}

void SFS_ObjectMemberAccess(ScriptParser *parser)
{
	if (parser->codec->LastError) return;
	SFS_Expression(parser);
	SFS_AddString(parser, ".");
    SFS_Identifier(parser);
}


void SFS_ObjectMethodCall(ScriptParser *parser)
{
	if (parser->codec->LastError) return;
	SFS_Expression(parser);
	SFS_AddString(parser, ".");
	SFS_Identifier(parser);
	SFS_AddString(parser, "(");
	SFS_Params(parser);
	SFS_AddString(parser, ")");
}

void SFS_Params(ScriptParser *parser)
{
	u32 val;
	if (parser->codec->LastError) return;
	val = BS_ReadInt(parser->bs, 1);
	while (val) {
		SFS_Expression(parser);
		val = BS_ReadInt(parser->bs, 1);
		if(val) SFS_AddString(parser, ",");
	}
}

void SFS_GetNumber(ScriptParser *parser)
{
	u32 val, nbBits;

	if (parser->codec->LastError) return;
	// integer
	if (BS_ReadInt(parser->bs, 1)) { 
		nbBits = BS_ReadInt(parser->bs, 5);
		val = BS_ReadInt(parser->bs, nbBits);
		SFS_AddInt(parser, val);
		return;
	} 
	// real number
	val = BS_ReadInt(parser->bs, 4);
	while ( val != 15) {
		if (val>=0 && val<=9) {
			SFS_AddChar(parser, (char) (val + '0') );
		} else if (val==10) {
			SFS_AddChar(parser, '.');
		} else if (val==11) {
			SFS_AddChar(parser, 'E');
		} else if (val==12){
			SFS_AddChar(parser, '-');
		}
		val = BS_ReadInt(parser->bs, 4);
	}
}

void SFS_GetString(ScriptParser *parser)
{
	char name[1000];
	if (parser->codec->LastError) return;
	BD_GetName(parser->bs, name);
	SFS_AddString(parser, name);
}

void SFS_GetBoolean(ScriptParser *parser)
{
	if (parser->codec->LastError) return;
	if (BS_ReadInt(parser->bs, 1)) {
		SFS_AddString(parser, "true");
	} else {
		SFS_AddString(parser, "false");
	}
}
