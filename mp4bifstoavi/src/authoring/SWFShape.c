/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Authoring Tools sub-project
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

#include <m4_mpeg4_nodes.h>
#include <intern/m4_swf_dev.h>

SFColor get_bifs_col(u32 ARGB)
{
	SFColor val;
	val.red = ((ARGB>>16)&0xFF) / 255.0f;
	val.green = ((ARGB>>8)&0xFF) / 255.0f;
	val.blue = ((ARGB)&0xFF) / 255.0f;
	return val;
}
Float get_bifs_alpha(u32 ARGB)
{
	Float val;
	val = ((ARGB>>24)&0xFF) / 255.0f;
	return val;
}

void SWF_InsertAppearance(SWFReader *read, SFNode *app)
{
	M_Shape *s = (M_Shape *) SWF_NewNode(read, TAG_MPEG4_Shape);
	s->appearance = app;
	Node_Register(app, (SFNode *) s);

	SWF_InsertNode(read, (SFNode *)s);
}

Bool col_equal(SFColor c1, SFColor c2)
{
	if (c1.red != c2.red) return 0;
	if (c1.green != c2.green) return 0;
	if (c1.blue != c2.blue) return 0;
	return 1;
}

SFNode *SWF_GetAppearance(SWFReader *read, SFNode *parent, u32 fill_col, Float line_width, u32 l_col)
{
	char szDEF[1024];
	u32 ID, i;
	SFColor fc, lc;
	Float fill_transp, line_transp;
	M_Appearance *app;
	M_Material2D *mat;

	fc = get_bifs_col(fill_col);
	fill_transp = 1.0f - get_bifs_alpha(fill_col);
	if (fill_transp<0) fill_transp=0;
	lc = get_bifs_col(l_col);
	line_transp = 1.0f - get_bifs_alpha(l_col);
	if (line_transp<0) line_transp=0;

	for (i=0; i<ChainGetCount(read->apps); i++) {
		app = ChainGetEntry(read->apps, i);
		mat = (M_Material2D *)app->material;
		if (!line_width) {
			if (mat->lineProps || !mat->filled) continue;
		} else {
			if (!mat->lineProps) continue;
			if (!col_equal(((M_LineProperties *)mat->lineProps)->lineColor, lc)) continue;
			if (((M_LineProperties *)mat->lineProps)->width != line_width) continue;
		}
		if (!mat->filled && fill_col) continue;
		if (mat->filled) {
			if (!fill_col) continue;
			if (mat->transparency != fill_transp) continue;
			if (!col_equal(mat->emissiveColor, fc)) continue;
		}
		/*OK same appearance let's go*/
		Node_Register((SFNode *)app, parent);
		return (SFNode *)app;
	}

	app = (M_Appearance *) SWF_NewNode(read, TAG_MPEG4_Appearance);
	app->material = SWF_NewNode(read, TAG_MPEG4_Material2D);
	Node_Register(app->material, (SFNode *)app);
	((M_Material2D *)app->material)->filled = 0;

	if (fill_col) {
		((M_Material2D *)app->material)->filled = 1;
		((M_Material2D *)app->material)->emissiveColor = fc;
		((M_Material2D *)app->material)->transparency = fill_transp;
	}
	if (line_width && l_col) {
		if (read->flags & M4SWF_UseXLineProps) {
			M_XLineProperties *lp = (M_XLineProperties *) SWF_NewNode(read, TAG_MPEG4_XLineProperties);
			((M_Material2D *)app->material)->lineProps = (SFNode *) lp;
			lp->width = line_width;
			lp->lineColor = lc;
			lp->isScalable = 1;
			lp->transparency = line_transp;
			Node_Register((SFNode *)lp, app->material);
		} else {
			M_LineProperties *lp = (M_LineProperties *) SWF_NewNode(read, TAG_MPEG4_LineProperties);
			((M_Material2D *)app->material)->lineProps = (SFNode *) lp;
			lp->width = line_width;
			lp->lineColor = lc;
			Node_Register((SFNode *)lp, app->material);
		}
	}

	sprintf(szDEF, "FILLAPP_%d", ChainGetCount(read->apps));

	read->load->ctx->max_node_id++;
	ID = read->load->ctx->max_node_id;

	Node_SetDEF((SFNode *)app, ID, szDEF);
	SWF_InsertAppearance(read, (SFNode *)app);
	Node_Register((SFNode *)app, parent);
	
	ChainAddEntry(read->apps, app);
	
	return (SFNode *) app;
}



#define mat_init(_obj) { memset((_obj).m, 0, sizeof(Float)*6); (_obj).m[0] = (_obj).m[4] = 1.0; }
#define mat_copy(_obj, from) memcpy((_obj).m, (from).m, sizeof(Float)*6);

void mat_inverse(M4Matrix2D *_this)
{
	Float res;
	M4Matrix tmp;
	if(!_this) return;
	res = _this->m[0]*_this->m[4] - _this->m[1]*_this->m[3];
	if (!res) {
		mat_init(*_this);
		return;
	}
	tmp.m[0] = _this->m[4]/res;
	tmp.m[1] = -1 *  _this->m[1]/res;
	tmp.m[2] =  ( _this->m[1]*_this->m[5] - _this->m[4]*_this->m[2]) / res;
	tmp.m[3] = -1 * _this->m[3]/res;
	tmp.m[4] =  _this->m[0]/res;
	tmp.m[5] = -1 * ( _this->m[0]*_this->m[5] - _this->m[3]*_this->m[2]) / res;
	mat_copy(*_this, tmp);
}

void mat_add_matrix(M4Matrix2D *_this, M4Matrix2D *from)
{
	M4Matrix2D bck;
	if (!_this || !from) return;
	mat_copy(bck, *_this);
	_this->m[0] = from->m[0]*bck.m[0] + from->m[1]*bck.m[3];
	_this->m[1] = from->m[0]*bck.m[1] + from->m[1]*bck.m[4];
	_this->m[2] = from->m[0]*bck.m[2] + from->m[1]*bck.m[5] + from->m[2];
	_this->m[3] = from->m[3]*bck.m[0] + from->m[4]*bck.m[3];
	_this->m[4] = from->m[3]*bck.m[1] + from->m[4]*bck.m[4];
	_this->m[5] = from->m[3]*bck.m[2] + from->m[4]*bck.m[5] + from->m[5];
}

void mat_add_scale(M4Matrix2D *_this, Float scale_x, Float scale_y)
{
	M4Matrix2D tmp;
	if (!_this || ((scale_x==1.0) && (scale_y==1.0)) ) return;
	mat_init(tmp);
	tmp.m[0] = scale_x;
	tmp.m[4] = scale_y;
	mat_add_matrix(_this, &tmp);
}

void mat_add_translation(M4Matrix2D *_this, Float cx, Float cy)
{
	M4Matrix2D tmp;
	if (!_this || (!cx && !cy) ) return;
	mat_init(tmp);
	tmp.m[2] = cx;
	tmp.m[5] = cy;
	mat_add_matrix(_this, &tmp);
}

void mat_apply_coords(M4Matrix2D *_this, Float *x, Float *y)
{
	Float _x, _y;
	if (!_this || !x || !y) return;

	_x = *x * _this->m[0] + *y * _this->m[1] + _this->m[2];
	_y = *x * _this->m[3] + *y * _this->m[4] + _this->m[5];
	*x = _x;
	*y = _y;
}

void mat_apply_point(M4Matrix2D *_this, SFVec2f *pt)
{
	mat_apply_coords(_this, &pt->x, &pt->y);
}

void mat_apply_rect(M4Matrix2D *_this, M4Rect *rc)
{
	SFVec2f tl, tr, bl, br;
	tl.x = rc->x;
	tl.y = rc->y;
	tr.x = rc->x+rc->width;
	tr.y = rc->y;
	bl.x = rc->x;
	bl.y = rc->y - rc->height;
	br.x = rc->x+rc->width;
	br.y = rc->y - rc->height;

	mat_apply_point(_this, &tl);
	mat_apply_point(_this, &tr);
	mat_apply_point(_this, &bl);
	mat_apply_point(_this, &br);

	rc->x = tl.x;
	if (rc->x>tr.x) rc->x=tr.x;
	if (rc->x>bl.x) rc->x=bl.x;
	if (rc->x>br.x) rc->x=br.x;

	rc->y = tl.y;
	if (rc->y<tr.y) rc->y=tr.y;
	if (rc->y<bl.y) rc->y=bl.y;
	if (rc->y<br.y) rc->y=br.y;
	
	rc->width = tl.x;
	if (rc->width<tr.x) rc->width=tr.x;
	if (rc->width<bl.x) rc->width=bl.x;
	if (rc->width<br.x) rc->width=br.x;

	rc->height = tl.y;
	if (rc->height>tr.y) rc->height=tr.y;
	if (rc->height>bl.y) rc->height=bl.y;
	if (rc->height>br.y) rc->height=br.y;

	rc->height = rc->y - rc->height;
	rc->width -= rc->x;
	assert(rc->height>=0);
	assert(rc->width>=0);
}

M4Rect SWF_GetCenteredBounds(SWFShapeRec *srec)
{
	M4Rect rc;
	u32 i;
	Float xm, ym, xM, yM;
	xM = yM = M4_MIN_FLOAT;
	xm = ym = M4_MAX_FLOAT;

	for (i=0; i<srec->path->nbPts; i++) {
		if (srec->path->pts[i].x<=xm) xm = srec->path->pts[i].x;
		if (srec->path->pts[i].x>=xM) xM = srec->path->pts[i].x;
		if (srec->path->pts[i].y<=ym) ym = srec->path->pts[i].y;
		if (srec->path->pts[i].y>=yM) yM = srec->path->pts[i].y;
	}
	rc.width = xM-xm;
	rc.height = yM-ym;
	rc.x = xm + rc.width/2;
	rc.y = ym + rc.height/2;
	return rc;
}

SFNode *SWF_GetGradient(SWFReader *read, SFNode *parent, SWFShapeRec *srec)
{
	Bool is_radial, has_alpha;
	M4Rect rc;
	M4Matrix2D mx;
	u32 i;
	MFFloat *keys;
	MFColor *values;
	FieldInfo info;
	M_Appearance *app = (M_Appearance *) SWF_NewNode(read, TAG_MPEG4_Appearance);
	Node_Register((SFNode *)app, parent);
	app->material = SWF_NewNode(read, TAG_MPEG4_Material2D);
	Node_Register(app->material, (SFNode *)app);
	((M_Material2D *)app->material)->filled = 1;

	is_radial = (srec->type==0x12) ? 1 : 0;
	app->texture = SWF_NewNode(read, is_radial ? TAG_MPEG4_RadialGradient : TAG_MPEG4_LinearGradient);
	Node_Register((SFNode *) app->texture, (SFNode *) app);

	/*set keys*/
	Node_GetFieldByName(app->texture, "key", &info);
	VRML_MF_Alloc(info.far_ptr, info.fieldType, srec->nbGrad);
	keys = (MFFloat *)info.far_ptr;
	for (i=0; i<srec->nbGrad; i++) {
		keys->vals[i] = srec->grad_ratio[i];
		keys->vals[i] /= 255;
	}

	/*set colors*/
	Node_GetFieldByName(app->texture, "keyValue", &info);
	VRML_MF_Alloc(info.far_ptr, info.fieldType, srec->nbGrad);
	values = (MFColor *)info.far_ptr;
	has_alpha = 0;
	for (i=0; i<srec->nbGrad; i++) {
		values->vals[i] = get_bifs_col(srec->grad_col[i]);
		if (get_bifs_alpha(srec->grad_col[i]) != 1.0f) has_alpha = 1;
	}
	/*set opacity*/
	if (has_alpha) {
		Node_GetFieldByName(app->texture, "opacity", &info);
		VRML_MF_Alloc(info.far_ptr, info.fieldType, srec->nbGrad);
		keys = (MFFloat *)info.far_ptr;
		for (i=0; i<srec->nbGrad; i++) {
			keys->vals[i] = get_bifs_alpha(srec->grad_col[i]);
		}
		/*and remove material !!*/
		((M_Material2D *)app->material)->filled = 0;
		((M_Material2D *)app->material)->lineProps = SWF_NewNode(read, TAG_MPEG4_LineProperties);;
		((M_LineProperties *)((M_Material2D *)app->material)->lineProps)->width = 0;
		Node_Register(((M_Material2D *)app->material)->lineProps, app->material);
	}

	/*
		FIXME - THIS IS WRONG, don't have time to investigate how to map gradients into (u, v) space
	*/

	/*get bounds in local coord system*/
	rc = SWF_GetCenteredBounds(srec);

	/*remove positioning*/
	srec->mat.m[2] -= rc.x;
	srec->mat.m[5] -= rc.y;
	/*set positioning in TEX coords (0,1) and not shape coords*/
	srec->mat.m[2] /= rc.width;
	srec->mat.m[5] /= rc.height;

	/*remove gradient square to local shape scaling*/
	mat_init(mx);
	mat_add_scale(&mx, 32768.0f/rc.width, 32768.0f/rc.height);
	mat_add_matrix(&mx, &srec->mat);

	/*adjust center for radial gradient*/
	if (rc.width > rc.height) {
		Float ar = rc.width/rc.height;
		mx.m[5] += (1-ar)*0.5f;
	} else {
		Float ar = rc.height/rc.width;
		mx.m[2] += (1-ar)*0.5f;
	}

	Node_GetFieldByName(app->texture, "spreadMethod", &info);
	*((SFInt32*)info.far_ptr) = 1;

	Node_GetFieldByName(app->texture, "transform", &info);
	*((SFNode **)info.far_ptr) = SWF_GetBIFSMatrix(read, &mx);
	Node_Register(*((SFNode **)info.far_ptr), app->texture);
	return (SFNode *) app;
}

void SWFShape_SetAppearance(SWFReader *read, SWFShape *shape, M_Shape *n, SWFShapeRec *srec, Bool is_fill)
{
	/*get regular appearance reuse*/
	if (is_fill) {
		switch (srec->type) {
		/*solid/alpha fill*/
		case 0x00:
			n->appearance = SWF_GetAppearance(read, (SFNode *) n, srec->solid_col, 0, 0);
			break;
		case 0x10:
		case 0x12:
			if (read->flags & M4SWF_NoGradient) {
				u32 col = srec->grad_col[srec->nbGrad/2];
				col |= 0xFF000000;
				n->appearance = SWF_GetAppearance(read, (SFNode *) n, col, 0, 0);
			} else {
				n->appearance = SWF_GetGradient(read, (SFNode *) n, srec);
			}
			break;
		default:
			swf_report(read, M4NotSupported, "Bitmap fill_style not supported");
			break;
		}
	} else {
		n->appearance = SWF_GetAppearance(read, (SFNode *) n, 0, srec->width, srec->solid_col);
	}
}

/*translate a flash sub shape with only one path (eg one looking style) to a BIFS Shape node*/
SFNode *SWFShapeToCurve2D(SWFReader *read, SWFShape *shape, SWFShapeRec *srec, Bool is_fill)
{
	u32 pt_idx, i;
	Bool use_xcurve;
	void *fptr;
	SFVec2f ct1, ct2, ct, pt, move_orig;
	M_Curve2D *curve;
	M_Coordinate2D *points;
	M_Shape *n = (M_Shape *) SWF_NewNode(read, TAG_MPEG4_Shape);

	SWFShape_SetAppearance(read, shape, n, srec, is_fill);

	use_xcurve = (read->flags & M4SWF_UseXCurve) ? 1 : 0;
	if (use_xcurve) {
		curve = (M_Curve2D *) SWF_NewNode(read, TAG_MPEG4_XCurve2D);
	} else {
		curve = (M_Curve2D *) SWF_NewNode(read, TAG_MPEG4_Curve2D);
	}
	points = (M_Coordinate2D *) SWF_NewNode(read, TAG_MPEG4_Coordinate2D);
	n->geometry = (SFNode *) curve;
	Node_Register((SFNode *) curve, (SFNode *)n);
	curve->point = (SFNode *) points;
	Node_Register((SFNode *) points, (SFNode *) curve);
	curve->fineness = 1.0;

	assert(srec->path->nbType);

	pt_idx = 0;
	for (i=0; i<srec->path->nbType; i++) {
		switch (srec->path->types[i]) {
		/*moveTo*/
		case 0:
			/*first moveTo implicit in BIFS*/
			if (i) {
				VRML_MF_Append(&curve->type, FT_MFInt32, &fptr);
				*((SFInt32 *)fptr) = 0;
			}
			VRML_MF_Append(&points->point, FT_MFVec2f, &fptr);
			((SFVec2f *)fptr)->x = srec->path->pts[pt_idx].x;
			((SFVec2f *)fptr)->y = srec->path->pts[pt_idx].y;
			move_orig = srec->path->pts[pt_idx];
			pt_idx++;
			break;
		/*lineTo*/
		case 1:
			VRML_MF_Append(&curve->type, FT_MFInt32, &fptr);
			*((SFInt32 *)fptr) = 1;
			VRML_MF_Append(&points->point, FT_MFVec2f, &fptr);
			((SFVec2f *)fptr)->x = srec->path->pts[pt_idx].x;
			((SFVec2f *)fptr)->y = srec->path->pts[pt_idx].y;
			pt_idx++;
			break;
		/*curveTo*/
		case 2:
			/*XCurve2D has quad arcs*/
			if (use_xcurve) {
				VRML_MF_Append(&curve->type, FT_MFInt32, &fptr);
				*((SFInt32 *)fptr) = 7;
				VRML_MF_Append(&points->point, FT_MFVec2f, &fptr);
				((SFVec2f *)fptr)->x = srec->path->pts[pt_idx].x;
				((SFVec2f *)fptr)->y = srec->path->pts[pt_idx].y;
				VRML_MF_Append(&points->point, FT_MFVec2f, &fptr);
				((SFVec2f *)fptr)->x = srec->path->pts[pt_idx+1].x;
				((SFVec2f *)fptr)->y = srec->path->pts[pt_idx+1].y;
				pt_idx+=2;
			} else {
				VRML_MF_Append(&curve->type, FT_MFInt32, &fptr);
				*((SFInt32 *)fptr) = 2;
				/*recompute cubic from quad*/
				ct.x = srec->path->pts[pt_idx].x;
				ct.y = srec->path->pts[pt_idx].y;
				pt.x = srec->path->pts[pt_idx-1].x;
				pt.y = srec->path->pts[pt_idx-1].y;
				ct1.x = pt.x + 2*(ct.x - pt.x)/3;
				ct1.y = pt.y + 2*(ct.y - pt.y)/3;
				ct.x = srec->path->pts[pt_idx+1].x;
				ct.y = srec->path->pts[pt_idx+1].y;
				ct2.x = ct1.x + (ct.x - pt.x) / 3;
				ct2.y = ct1.y + (ct.y - pt.y) / 3;

				VRML_MF_Append(&points->point, FT_MFVec2f, &fptr);
				((SFVec2f *)fptr)->x = ct1.x;
				((SFVec2f *)fptr)->y = ct1.y;
				VRML_MF_Append(&points->point, FT_MFVec2f, &fptr);
				((SFVec2f *)fptr)->x = ct2.x;
				((SFVec2f *)fptr)->y = ct2.y;
				VRML_MF_Append(&points->point, FT_MFVec2f, &fptr);
				((SFVec2f *)fptr)->x = ct.x;
				((SFVec2f *)fptr)->y = ct.y;
				pt_idx+=2;
			}
			break;
		}
	}
	return (SFNode *) n;
}

void SWF_MergeCurve2D(M_Curve2D *s, M_Curve2D *tomerge)
{
	u32 i, pt_idx, j;
	SFVec2f pt;
	void *ptr;
	M_Coordinate2D *dest, *orig;
	dest = (M_Coordinate2D *) s->point;
	orig = (M_Coordinate2D *) tomerge->point;

	if (!tomerge->type.count) return;
	if (!orig->point.count) return;
	pt = orig->point.vals[0];

	if (s->type.vals[s->type.count - 1] == 0) {
		dest->point.vals[dest->point.count - 1] = pt;
	} else {
		VRML_MF_Append(&s->type, FT_MFInt32, &ptr);
		*((SFInt32 *)ptr) = 0;
		VRML_MF_Append(&dest->point, FT_MFVec2f, &ptr);
		*((SFVec2f *)ptr) = pt;
	}
	
	i = 0;
	if (tomerge->type.vals[0] == 0) i=1;
	pt_idx = 1;

	for (; i<tomerge->type.count; i++) {
		switch (tomerge->type.vals[i]) {
		case 0:
			if (s->type.vals[s->type.count - 1] == 0) {
				dest->point.vals[dest->point.count - 1] = pt;
			} else {
				VRML_MF_Append(&s->type, FT_MFInt32, &ptr);
				*((SFInt32 *)ptr) = 0;
				VRML_MF_Append(&dest->point, FT_MFVec2f, &ptr);
				*((SFVec2f *)ptr) = orig->point.vals[pt_idx];
			}
			pt_idx++;
			break;
		case 1:
			VRML_MF_Append(&s->type, FT_MFInt32, &ptr);
			*((SFInt32 *)ptr) = 1;
			VRML_MF_Append(&dest->point, FT_MFVec2f, &ptr);
			*((SFVec2f *)ptr) = orig->point.vals[pt_idx];
			pt_idx++;
			break;
		case 2:
			VRML_MF_Append(&s->type, FT_MFInt32, &ptr);
			*((SFInt32 *)ptr) = 2;
			for (j=0; j<3; j++) {
				VRML_MF_Append(&dest->point, FT_MFVec2f, &ptr);
				*((SFVec2f *)ptr) = orig->point.vals[pt_idx];
				pt_idx++;
			}
			break;
		case 7:
			VRML_MF_Append(&s->type, FT_MFInt32, &ptr);
			*((SFInt32 *)ptr) = 7;
			for (j=0; j<2; j++) {
				VRML_MF_Append(&dest->point, FT_MFVec2f, &ptr);
				*((SFVec2f *)ptr) = orig->point.vals[pt_idx];
				pt_idx++;
			}
			break;
		}
	}
}

void SWFShape_InsertBIFSShape(M_OrderedGroup *og, M_Shape *n)
{
#if 1
	u32 i;
	for (i=0; i<ChainGetCount(og->children); i++) {
		M_Shape *prev = ChainGetEntry(og->children, i);
		if (prev && (prev->appearance == n->appearance) ) {
			SWF_MergeCurve2D( (M_Curve2D *)prev->geometry, (M_Curve2D *)n->geometry);
			Node_Register((SFNode *)n, NULL);
			Node_Unregister((SFNode *)n, NULL);
			return;
		}
	}
#endif
	Node_InsertChild((SFNode *)og, (SFNode *)n, -1);
	Node_Register((SFNode *) n, (SFNode *) og);
}

/*this is the core of the parser, translates flash to BIFS shapes*/
SFNode *SWFShapeToBIFS(SWFReader *read, SWFShape *shape)
{
	SFNode *n;
	SFNode *og;
	u32 i, count;
	SWFShapeRec *srec;

	count = ChainGetCount(shape->fill_left);
	count += ChainGetCount(shape->lines);
	/*empty, return empty shape*/
	if (!count) {
		M_Shape *s = (M_Shape *) SWF_NewNode(read, TAG_MPEG4_Shape);
		s->geometry = SWF_NewNode(read, TAG_MPEG4_Curve2D);
		Node_Register(s->geometry, (SFNode *)s);
		return (SFNode *)s;
	}

	/*direct match, no top group*/
	if (count == 1) {
		Bool is_fill = 1;
		srec = ChainGetEntry(shape->fill_left, 0);
		if (!srec) {
			srec = ChainGetEntry(shape->lines, 0);
			is_fill = 0;
		}
		return SWFShapeToCurve2D(read, shape, srec, is_fill);
	}

	/*we need a grouping node*/
	og = SWF_NewNode(read, TAG_MPEG4_OrderedGroup);
	for (i=0; i<ChainGetCount(shape->fill_left); i++) {
		srec = ChainGetEntry(shape->fill_left, i);
		n = SWFShapeToCurve2D(read, shape, srec, 1);
		if (n) SWFShape_InsertBIFSShape((M_OrderedGroup*)og, (M_Shape *)n);
	}
	for (i=0; i<ChainGetCount(shape->lines); i++) {
		srec = ChainGetEntry(shape->lines, i);
		n = SWFShapeToCurve2D(read, shape, srec, 0);
		if (n) SWFShape_InsertBIFSShape((M_OrderedGroup*)og, (M_Shape *)n);
	}

	return og;
}



SFNode *SWF_GetBIFSMatrix(SWFReader *read, M4Matrix2D *mat)
{
	M_TransformMatrix2D *tm = (M_TransformMatrix2D *)SWF_NewNode(read, TAG_MPEG4_TransformMatrix2D);
	tm->mxx = mat->m[0];
	tm->mxy = mat->m[1];
	tm->tx = mat->m[2];
	tm->myx = mat->m[3];
	tm->myy = mat->m[4];
	tm->ty = mat->m[5];
	return (SFNode *) tm;
}

SFNode *SWF_GetBIFSColorMatrix(SWFReader *read, M4ColorMatrix *cmat)
{
	M_ColorTransform *ct = (M_ColorTransform*)SWF_NewNode(read, TAG_MPEG4_ColorTransform);
	ct->mrr = cmat->m[0];
	ct->mrg = cmat->m[1];
	ct->mrb = cmat->m[2];
	ct->mra = cmat->m[3];
	ct->tr = cmat->m[4];
	ct->mgr = cmat->m[5];
	ct->mgg = cmat->m[6];
	ct->mgb = cmat->m[7];
	ct->mga = cmat->m[8];
	ct->tg = cmat->m[9];
	ct->mbr = cmat->m[10];
	ct->mbg = cmat->m[11];
	ct->mbb = cmat->m[12];
	ct->mba = cmat->m[13];
	ct->tb = cmat->m[14];
	ct->mar = cmat->m[15];
	ct->mag = cmat->m[16];
	ct->mab = cmat->m[17];
	ct->maa = cmat->m[18];
	ct->ta = cmat->m[19];
	return (SFNode *) ct;
}


SFNode *SWF_GetGlyph(SWFReader *read, u32 fontID, u32 gl_index, SFNode *par)
{
	char szDEF[1024];
	u32 ID;
	SFNode *n, *glyph;
	SWFFont *ft;

	sprintf(szDEF, "FT%d_GL%d", fontID, gl_index);
	n = SG_FindNodeByName(read->load->scene_graph, szDEF);
	if (n) {
		Node_Register(n, par);
		return n;
	}

	/*first use of glyph in file*/
	ft = SWF_FindFont(read, fontID);
	if (!ft) {
		swf_report(read, M4BadParam, "Cannot find font %d - skipping glyph", fontID);
		return NULL;
	}
	if (ft->nbGlyphs <= gl_index) {
		swf_report(read, M4BadParam, "Glyph #%d not found in font %d - skipping", gl_index, fontID);
		return NULL;
	}
	n = ChainGetEntry(ft->glyphs, gl_index);
	if (Node_GetTag(n) != TAG_MPEG4_Shape) {
		swf_report(read, M4BadParam, "Glyph #%d in font %d not a shape (translated in %s) - skipping", gl_index, fontID, Node_GetName(n));
		return NULL;
	}
	glyph = ((M_Shape *)n)->geometry;
	/*space*/
	if (!glyph) return NULL;

	read->load->ctx->max_node_id++;
	ID = read->load->ctx->max_node_id;
	Node_SetDEF(glyph, ID, szDEF);
	Node_Register(glyph, par);

	/*also insert glyph*/
	SWF_InsertNode(read, n);

	return glyph;
}

SFNode *SWFTextToBIFS(SWFReader *read, SWFText *text)
{
	u32 i, j;
	Bool use_text;
	Float dx;
	SWFFont *ft;
	M_Transform2D *par, *gl_par;
	M_Shape *gl;
	M_TransformMatrix2D *tr;

	use_text = (read->flags & M4SWF_NoFonts) ? 1 : 0;
	tr = (M_TransformMatrix2D *) SWF_NewNode(read, TAG_MPEG4_TransformMatrix2D);
	tr->mxx = text->mat.m[0];
	tr->mxy = text->mat.m[1];
	tr->tx = text->mat.m[2];
	tr->myx = text->mat.m[3];
	tr->myy = text->mat.m[4];
	tr->ty = text->mat.m[5];


	for (i=0; i<ChainGetCount(text->text); i++) {
		SWFGlyphRec *gr = ChainGetEntry(text->text, i);
		par = (M_Transform2D *) SWF_NewNode(read, TAG_MPEG4_Transform2D);
		par->translation.x = gr->orig_x;
		par->translation.y = gr->orig_y;

		ft = NULL;
		if (use_text) {
			ft = SWF_FindFont(read, gr->fontID);
			if (!ft->glyph_codes) {
				use_text = 0;
				swf_report(read, M4BadParam, "Font glyphs are not defined, cannot reference extern font - Forcing glyph embedding");
			}
		}

		if (!use_text) {
			par->scale.x = gr->fontHeight;
			par->scale.y = gr->fontHeight;
		} else {
			/*don't forget we're flipped at top level...*/
			par->scale.y = -1;
		}
		Node_InsertChild((SFNode *)tr, (SFNode *) par, -1);
		Node_Register((SFNode *) par, (SFNode *)tr);

		if (use_text) {
			u16 *str_w, *widestr;
			u8 *str;
			void *ptr;
			M_Text *t = (M_Text *) SWF_NewNode(read, TAG_MPEG4_Text);
			M_FontStyle *f = (M_FontStyle *) SWF_NewNode(read, TAG_MPEG4_FontStyle);
			t->fontStyle = (SFNode *) f;
			Node_Register(t->fontStyle, (SFNode *) t);

			/*restore back the font height in pixels (it's currently in SWF glyph design units)*/
			f->size = gr->fontHeight * SWF_TWIP_SCALE / SWF_TEXT_SCALE;

			if (ft->fontName) {
				VRML_MF_Reset(&f->family, FT_MFString);
				VRML_MF_Append(&f->family, FT_MFString, &ptr);
				((SFString*)ptr)->buffer = strdup(ft->fontName);
			}
			VRML_MF_Reset(&f->justify, FT_MFString);
			VRML_MF_Append(&f->justify, FT_MFString, &ptr);
			((SFString*)ptr)->buffer = strdup("BEGIN");

			if (f->style.buffer) free(f->style.buffer);
			if (ft->is_italic && ft->is_bold) f->style.buffer = strdup("BOLDITALIC");
			else if (ft->is_bold) f->style.buffer = strdup("BOLD");
			else if (ft->is_italic) f->style.buffer = strdup("ITALIC");
			else f->style.buffer = strdup("PLAIN");

			/*convert to UTF-8*/
			str_w = malloc(sizeof(u16) * (gr->nbGlyphs+1));
			for (j=0; j<gr->nbGlyphs; j++) str_w[j] = ft->glyph_codes[gr->indexes[j]];
			str_w[j] = 0;
			str = malloc(sizeof(u8) * (gr->nbGlyphs+2));
			widestr = str_w;
			j = utf8_wcstombs(str, sizeof(u8) * (gr->nbGlyphs+1), (const unsigned short **) &widestr);
			if (j != -1) {
				str[j] = 0;
				VRML_MF_Reset(&t->string, FT_MFString);
				VRML_MF_Append(&t->string, FT_MFString, &ptr);
				((SFString*)ptr)->buffer = malloc(sizeof(char) * (j+1));
				memcpy(((SFString*)ptr)->buffer, str, sizeof(char) * (j+1));
			}

			free(str);
			free(str_w);

			gl = (M_Shape *) SWF_NewNode(read, TAG_MPEG4_Shape);
			gl->appearance = SWF_GetAppearance(read, (SFNode *) gl, gr->col, 0, 0);				
			gl->geometry = (SFNode *) t;
			Node_Register(gl->geometry, (SFNode *) gl);
			Node_InsertChild((SFNode *) par, (SFNode *)gl, -1);
			Node_Register((SFNode *) gl, (SFNode *) par);
		} else {

			/*convert glyphs*/
			dx = 0;
			for (j=0; j<gr->nbGlyphs; j++) {
				gl = (M_Shape *) SWF_NewNode(read, TAG_MPEG4_Shape);
				gl->geometry = SWF_GetGlyph(read, gr->fontID, gr->indexes[j], (SFNode *) gl);

				if (!gl->geometry) {
					Node_Register((SFNode *) gl, NULL);
					Node_Unregister((SFNode *) gl, NULL);
					dx += gr->dx[j];
					continue;
				}
				assert((Node_GetTag(gl->geometry)==TAG_MPEG4_Curve2D) || (Node_GetTag(gl->geometry)==TAG_MPEG4_XCurve2D));

				gl_par = (M_Transform2D *) SWF_NewNode(read, TAG_MPEG4_Transform2D);
				gl->appearance = SWF_GetAppearance(read, (SFNode *) gl, gr->col, 0, 0);

				gl_par->translation.x = dx / gr->fontHeight;
				dx += gr->dx[j];

				Node_InsertChild((SFNode *) gl_par, (SFNode *)gl, -1);
				Node_Register((SFNode *) gl, (SFNode *) gl_par);
				Node_InsertChild((SFNode *) par, (SFNode *)gl_par, -1);
				Node_Register((SFNode *) gl_par, (SFNode *) par);
			}
		}
	}

	return (SFNode *)tr;
}
