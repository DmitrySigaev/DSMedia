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


#include <intern/m4_scenegraph_dev.h>
/*MPEG4 tags (for internal nodes)*/
#include <m4_mpeg4_nodes.h>

void format_sftime_string(Float val, char *str)
{
	u32 h, m, s;
	Bool neg = 0;
	if (val<0) {
		val = -val;
		neg = 1;
	}
	h = (u32) val/3600;
	m = (u32) val/60-h*60;
	s = (u32) val - h*3600 - m*60;
	sprintf(str, "%s%02d:%02d:%02d", neg ? "-" : "", h, m, s);
}

void SetValuatorOutput(M_Valuator *p, SFVec4f *inSFField, GenMFField *inMFField, u32 inType)
{
	char str[500];
	u32 i;
	LPROUTE r;
	SFVec4f output, sf_out;
	u32 count, num_out;

	if (!p->sgprivate->NodeID && !p->sgprivate->scenegraph->pOwningProto) return;

	num_out = 1;

	if (!inMFField) {
		count = 1;
		output.x = p->Factor1 * inSFField->x + p->Offset1;
		output.y = p->Factor2 * inSFField->y + p->Offset2;
		output.z = p->Factor3 * inSFField->z + p->Offset3;
		output.q = p->Factor4 * inSFField->q + p->Offset4;

		if (p->Sum) {
			output.x = output.x + output.y + output.z + output.q;
			output.y = output.z = output.q = output.x;
		}

		switch (inType) {
		case FT_SFVec2f:
			num_out = 2;
			break;
		case FT_SFVec3f:
		case FT_SFColor:
			num_out = 3;
			break;
		case FT_SFVec4f:
		case FT_SFRotation:
			num_out = 4;
			break;
		}
	} else {
		count = inMFField->count;
	}
	/*reallocate all MF fields*/
	VRML_MF_Reset(&p->outMFColor, FT_MFColor);
	VRML_MF_Reset(&p->outMFFloat, FT_MFFloat);
	VRML_MF_Reset(&p->outMFInt32, FT_MFInt32);
	VRML_MF_Reset(&p->outMFRotation, FT_MFRotation);
	VRML_MF_Reset(&p->outMFString, FT_MFString);
	VRML_MF_Reset(&p->outMFVec2f, FT_MFVec2f);
	VRML_MF_Reset(&p->outMFVec3f, FT_MFVec3f);

	VRML_MF_Alloc(&p->outMFColor, FT_MFColor, count);
	VRML_MF_Alloc(&p->outMFFloat, FT_MFFloat, count);
	VRML_MF_Alloc(&p->outMFInt32, FT_MFInt32, count);
	VRML_MF_Alloc(&p->outMFRotation, FT_MFRotation, count);
	VRML_MF_Alloc(&p->outMFString, FT_MFString, count);
	VRML_MF_Alloc(&p->outMFVec2f, FT_MFVec2f, count);
	VRML_MF_Alloc(&p->outMFVec3f, FT_MFVec3f, count);

	/*set all MF outputs*/
	assert(count);
	for (i=0; i<count; i++) {
		if (inType) {
			switch (inType) {
			case FT_MFInt32:
				output.x = p->Factor1 * ((MFInt32 *)inMFField)->vals[i] + p->Offset1;
				output.y = p->Factor2 * ((MFInt32 *)inMFField)->vals[i] + p->Offset2;
				output.z = p->Factor3 * ((MFInt32 *)inMFField)->vals[i] + p->Offset3;
				output.q = p->Factor4 * ((MFInt32 *)inMFField)->vals[i] + p->Offset4;
				break;
			case FT_MFFloat:
				output.x = p->Factor1 * ((MFFloat *)inMFField)->vals[i] + p->Offset1;
				output.y = p->Factor2 * ((MFFloat *)inMFField)->vals[i] + p->Offset2;
				output.z = p->Factor3 * ((MFFloat *)inMFField)->vals[i] + p->Offset3;
				output.q = p->Factor4 * ((MFFloat *)inMFField)->vals[i] + p->Offset4;
				break;
			case FT_MFColor:
				output.x = p->Factor1 * ((MFColor *)inMFField)->vals[i].red + p->Offset1;
				output.y = p->Factor2 * ((MFColor *)inMFField)->vals[i].green + p->Offset2;
				output.z = p->Factor3 * ((MFColor *)inMFField)->vals[i].blue + p->Offset3;
				output.q = p->Offset4;
				num_out = 3;
				break;
			case FT_MFVec2f:
				output.x = p->Factor1 * ((MFVec2f *)inMFField)->vals[i].x + p->Offset1;
				output.y = p->Factor2 * ((MFVec2f *)inMFField)->vals[i].y + p->Offset2;
				output.z = p->Offset3;
				output.q = p->Offset4;
				num_out = 2;
				break;
			case FT_MFVec3f:
				output.x = p->Factor1 * ((MFVec3f *)inMFField)->vals[i].x + p->Offset1;
				output.y = p->Factor2 * ((MFVec3f *)inMFField)->vals[i].y + p->Offset2;
				output.z = p->Factor3 * ((MFVec3f *)inMFField)->vals[i].z + p->Offset3;
				output.q = p->Offset4;
				num_out = 3;
				break;
			case FT_MFRotation:
				output.x = p->Factor1 * ((MFRotation *)inMFField)->vals[i].xAxis + p->Offset1;
				output.y = p->Factor2 * ((MFRotation *)inMFField)->vals[i].yAxis + p->Offset2;
				output.z = p->Factor3 * ((MFRotation *)inMFField)->vals[i].zAxis + p->Offset3;
				output.q = p->Factor4 * ((MFRotation *)inMFField)->vals[i].angle + p->Offset4;
				num_out = 4;
				break;
			case FT_MFString:
				/*cf below*/
				output.x = output.y = output.z = output.q = 0;
				if (((MFString *)inMFField)->vals[i]) {
					if (stricmp(((MFString *)inMFField)->vals[i], "true")) {
						output.x = output.y = output.z = output.q = 1;
					} else if (!strstr(((MFString *)inMFField)->vals[i], ".")) {
						output.x = (Float) atoi(((MFString *)inMFField)->vals[i]);
						output.y = output.z = output.q = output.x;
					} else {
						output.x = (Float) atof(((MFString *)inMFField)->vals[i]);
						output.y = output.z = output.q = output.x;
					}
				}

				output.x = p->Factor1 * output.x + p->Offset1;
				output.y = p->Factor2 * output.y + p->Offset2;
				output.z = p->Factor3 * output.z + p->Offset3;
				output.q = p->Factor4 * output.q + p->Offset4;
				break;

			}
			if (p->Sum) {
				output.x = output.x + output.y + output.z + output.q;
				output.y = output.z = output.q = output.x;
			}
		}
		
		p->outMFFloat.vals[i] = output.x;
		
		p->outMFInt32.vals[i] = (s32) output.x;

		p->outMFColor.vals[i].red = output.x;
		p->outMFColor.vals[i].green = output.y;
		p->outMFColor.vals[i].blue = output.z;

		p->outMFVec2f.vals[i].x = output.x;
		p->outMFVec2f.vals[i].y = output.y;

		p->outMFVec3f.vals[i].x = output.x;
		p->outMFVec3f.vals[i].y = output.y;
		p->outMFVec3f.vals[i].z = output.z;

		p->outMFRotation.vals[i].xAxis = output.x;
		p->outMFRotation.vals[i].yAxis = output.y;
		p->outMFRotation.vals[i].zAxis = output.z;
		p->outMFRotation.vals[i].angle = output.q;


		if (num_out==1) {
			if (inType==FT_SFTime) {
				format_sftime_string(output.x, str);
			} else {
				sprintf(str, "%.6f", output.x);
			}
		} else if (num_out==2) {
			sprintf(str, "%.4f %.4f", output.x, output.y);
		} else if (num_out==3) {
			sprintf(str, "%.3f %.3f %.3f", output.x, output.y, output.z);
		} else if (num_out==4) {
			sprintf(str, "%.2f %.2f %.2f %.2f", output.x, output.y, output.z, output.q);
		}

		if (p->outMFString.vals[i]) free(p->outMFString.vals[i]);
		p->outMFString.vals[i] = strdup(str);
		
		if (!i) sf_out = output;

	}

	p->outSFBool = (Bool) (sf_out.x ? 1 : 0);
	p->outSFFloat = sf_out.x;
	p->outSFInt32 = (s32) sf_out.x;
	p->outSFTime = (SFTime) sf_out.x;
	p->outSFRotation.xAxis = sf_out.x;
	p->outSFRotation.yAxis = sf_out.y;
	p->outSFRotation.zAxis = sf_out.z;
	p->outSFRotation.angle = sf_out.q;
	p->outSFColor.red = sf_out.x;
	p->outSFColor.green = sf_out.y;
	p->outSFColor.blue = sf_out.z;
	p->outSFVec2f.x = sf_out.x;
	p->outSFVec2f.y = sf_out.y;
	p->outSFVec3f.x = sf_out.x;
	p->outSFVec3f.y = sf_out.y;
	p->outSFVec3f.z = sf_out.z;

	if (num_out==1) {
		if (inType==FT_SFTime) {
			format_sftime_string(output.x, str);
		} else {
			sprintf(str, "%.6f", sf_out.x);
		}
	} else if (num_out==2) {
		sprintf(str, "%.4f %.4f", sf_out.x, sf_out.y);
	} else if (num_out==3) {
		sprintf(str, "%.3f %.3f %.3f", sf_out.x, sf_out.y, sf_out.z);
	} else if (num_out==4) {
		sprintf(str, "%.2f %.2f %.2f %.2f", sf_out.x, sf_out.y, sf_out.z, sf_out.q);
	}
	if (p->outSFString.buffer ) free(p->outSFString.buffer);
	p->outSFString.buffer = strdup(str);

	/*valuator is a special case, all routes are triggered*/
	for (i=0; i<ChainGetCount(p->sgprivate->outRoutes); i++) {
		r = ChainGetEntry(p->sgprivate->outRoutes, i);
		if (r->FromNode != (SFNode *)p) continue;

		if (r->IS_route) {
			ActivateRoute(r);
		} else {
			QueueRoute(p->sgprivate->scenegraph, r);
		}
	}
}


/*
valuator spec (9.4.2.116.2)
"In the special case of a scalar input type (e.g. SFBool, SFInt32) that is cast to a vectorial output type (e.g.
SFVec2f), for all components i of output.i, input.i shall take the value of the scalar input type, after appropriate type
conversion"
*/

void Valuator_SetInSFBool(SFNode *n)
{
	SFVec4f val;
	M_Valuator *_this = (M_Valuator *) n;
	val.x = val.y = val.z = val.q = (Float) _this->inSFBool;
	SetValuatorOutput(_this, &val, NULL, FT_SFBool);
}
void Valuator_SetInSFFloat(SFNode *n)
{
	SFVec4f val;
	M_Valuator *_this = (M_Valuator *) n;
	val.x = val.y = val.z = val.q = _this->inSFFloat;
	SetValuatorOutput(_this, &val, NULL, FT_SFFloat);
}
void Valuator_SetInSFInt32(SFNode *n)
{
	SFVec4f val;
	M_Valuator *_this = (M_Valuator *) n;
	val.x = val.y = val.z = val.q = (Float) _this->inSFInt32;
	SetValuatorOutput(_this, &val, NULL, FT_SFInt32);
}
void Valuator_SetInSFTime(SFNode *n)
{
	SFVec4f val;
	M_Valuator *_this = (M_Valuator *) n;
	val.x = val.y = val.z = val.q = (Float) _this->inSFTime;
	SetValuatorOutput(_this, &val, NULL, FT_SFTime);
}
void Valuator_SetInSFColor(SFNode *n)
{
	SFVec4f val;
	M_Valuator *_this = (M_Valuator *) n;
	val.x = (Float) _this->inSFColor.red;
	val.y = (Float) _this->inSFColor.green;
	val.z = (Float) _this->inSFColor.blue;
	val.q = 0;
	SetValuatorOutput(_this, &val, NULL, FT_SFColor);
}
void Valuator_SetInSFVec2f(SFNode *n)
{
	SFVec4f val;
	M_Valuator *_this = (M_Valuator *) n;
	val.x = (Float) _this->inSFVec2f.x;
	val.y = (Float) _this->inSFVec2f.y;
	val.z = val.q = 0;
	SetValuatorOutput(_this, &val, NULL, FT_SFVec2f);
}
void Valuator_SetInSFVec3f(SFNode *n)
{
	SFVec4f val;
	M_Valuator *_this = (M_Valuator *) n;
	val.x = (Float) _this->inSFVec3f.x;
	val.y = (Float) _this->inSFVec3f.y;
	val.z = (Float) _this->inSFVec3f.z;
	val.q = 0;
	SetValuatorOutput(_this, &val, NULL, FT_SFVec3f);
}
void Valuator_SetInSFRotation(SFNode *n)
{
	SFVec4f val;
	M_Valuator *_this = (M_Valuator *) n;
	val.x = (Float) _this->inSFRotation.xAxis;
	val.y = (Float) _this->inSFRotation.yAxis;
	val.z = (Float) _this->inSFRotation.zAxis;
	val.q = (Float) _this->inSFRotation.angle;
	SetValuatorOutput(_this, &val, NULL, FT_SFRotation);
}

/*
valuator spec (9.4.2.116.2)
Convert if the content of the string represents an int, float or
double value. ‘Boolean’ string values 'true' and 'false' are
converted to 1.0 and 0.0 respectively. Any other string is converted to 0.0
*/
void Valuator_SetInSFString(SFNode *n)
{
	SFVec4f val;
	M_Valuator *_this = (M_Valuator *) n;
	val.x = val.y = val.z = val.q = 0;
	if (! _this->inSFString.buffer) return;
	if (!stricmp(_this->inSFString.buffer, "true")) {
		val.x = val.y = val.z = val.q = 1;
	} else if (!strstr(_this->inSFString.buffer, ".")) {
		val.x = (Float) atoi(_this->inSFString.buffer);
		val.y = val.z = val.q = val.x;
	} else {
		val.x = (Float) atof(_this->inSFString.buffer);
		val.y = val.z = val.q = val.x;
	}
	SetValuatorOutput(_this, &val, NULL, FT_SFString);
}

void Valuator_SetInMFColor(SFNode *n)
{
	M_Valuator *_this = (M_Valuator *) n;
	SetValuatorOutput(_this, NULL, (GenMFField *) &_this->inMFColor, FT_MFColor);
}

void Valuator_SetInMFFloat(SFNode *n)
{
	M_Valuator *_this = (M_Valuator *) n;
	SetValuatorOutput(_this, NULL, (GenMFField *) &_this->inMFFloat, FT_MFFloat);
}
void Valuator_SetInMFInt32(SFNode *n)
{
	M_Valuator *_this = (M_Valuator *) n;
	SetValuatorOutput(_this, NULL, (GenMFField *) &_this->inMFInt32, FT_MFInt32);
} 
void Valuator_SetInMFVec2f(SFNode *n)
{
	M_Valuator *_this = (M_Valuator *) n;
	SetValuatorOutput(_this, NULL, (GenMFField *) &_this->inMFVec2f, FT_MFVec2f);
}
void Valuator_SetInMFVec3f(SFNode *n)
{
	M_Valuator *_this = (M_Valuator *) n;
	SetValuatorOutput(_this, NULL, (GenMFField *) &_this->inMFVec3f, FT_MFVec3f);
}
void Valuator_SetInMFRotation(SFNode *n)
{
	M_Valuator *_this = (M_Valuator *) n;
	SetValuatorOutput(_this, NULL, (GenMFField *) &_this->inMFRotation, FT_MFRotation);
}
void Valuator_SetInMFString(SFNode *n)
{
	M_Valuator *_this = (M_Valuator *) n;
	SetValuatorOutput(_this, NULL, (GenMFField *) &_this->inMFString, FT_MFString);
}
