/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004 
 *					All rights reserved
 *
 *  This file is part of GPAC / Scene Rendering sub-project
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

#include <intern/m4_render_tools.h>

void mx2d_add_matrix(M4Matrix2D *_this, M4Matrix2D *from)
{
	M4Matrix2D bck;
	if (!_this || !from) return;
	mx2d_copy(bck, *_this);
	_this->m[0] = from->m[0]*bck.m[0] + from->m[1]*bck.m[3];
	_this->m[1] = from->m[0]*bck.m[1] + from->m[1]*bck.m[4];
	_this->m[2] = from->m[0]*bck.m[2] + from->m[1]*bck.m[5] + from->m[2];
	_this->m[3] = from->m[3]*bck.m[0] + from->m[4]*bck.m[3];
	_this->m[4] = from->m[3]*bck.m[1] + from->m[4]*bck.m[4];
	_this->m[5] = from->m[3]*bck.m[2] + from->m[4]*bck.m[5] + from->m[5];
}

void mx2d_add_translation(M4Matrix2D *_this, Float cx, Float cy)
{
	M4Matrix2D tmp;
	if (!_this || (!cx && !cy) ) return;
	mx2d_init(tmp);
	tmp.m[2] = cx;
	tmp.m[5] = cy;
	mx2d_add_matrix(_this, &tmp);
}


void mx2d_add_rotation(M4Matrix2D *_this, Float cx, Float cy, Float angle)
{
	M4Matrix2D tmp;
	if (!_this) return;
	mx2d_init(tmp);

	mx2d_add_translation(_this, -cx, -cy);
	
	tmp.m[0] = (Float) cos(angle);
	tmp.m[4] = tmp.m[0];
	tmp.m[3] = (Float) sin(angle);
	tmp.m[1] = -1 * tmp.m[3];
	mx2d_add_matrix(_this, &tmp);

	mx2d_add_translation(_this, cx, cy);
}

void mx2d_add_scale(M4Matrix2D *_this, Float scale_x, Float scale_y)
{
	M4Matrix2D tmp;
	if (!_this || ((scale_x==1.0) && (scale_y==1.0)) ) return;
	mx2d_init(tmp);
	tmp.m[0] = scale_x;
	tmp.m[4] = scale_y;
	mx2d_add_matrix(_this, &tmp);
}

void mx2d_add_scale_at(M4Matrix2D *_this, Float scale_x, Float scale_y, Float cx, Float cy, Float angle)
{
	M4Matrix2D tmp;
	if (!_this) return;
	mx2d_init(tmp);
	if (angle) {
		mx2d_add_rotation(_this, cx, cy, -angle);
	}
	tmp.m[0] = scale_x;
	tmp.m[4] = scale_y;
	mx2d_add_matrix(_this, &tmp);
	if (angle) mx2d_add_rotation(_this, cx, cy, angle);
}

void mx2d_add_skew(M4Matrix2D *_this, Float skew_x, Float skew_y)
{
	M4Matrix2D tmp;
	if (!_this || (!skew_x && !skew_y) ) return;
	mx2d_init(tmp);
	tmp.m[1] = skew_x;
	tmp.m[3] = skew_y;
	mx2d_add_matrix(_this, &tmp);
}

void mx2d_add_skewX(M4Matrix2D *_this, Float angle)
{
	M4Matrix2D tmp;
	Float sina = (Float) sin(angle);
	if (!_this || !sina) return;
	mx2d_init(tmp);
	if (!sina) return;
	tmp.m[1] = (Float) cos(angle)/sina;
	tmp.m[3] = 0;
	mx2d_add_matrix(_this, &tmp);
}

void mx2d_add_skewY(M4Matrix2D *_this, Float angle)
{
	M4Matrix2D tmp;
	Float sina = (Float) sin(angle);
	if (!_this || !sina) return;
	mx2d_init(tmp);
	tmp.m[1] = 0;
	tmp.m[3] = (Float) cos(angle)/sina;
	mx2d_add_matrix(_this, &tmp);
}

Float mx2d_get_determinent(M4Matrix2D *_this)
{
	if (_this)
		return _this->m[0]*_this->m[4] - _this->m[1]*_this->m[3];
	return 0;
}

void mx2d_inverse(M4Matrix2D *_this)
{
	Float res;
	M4Matrix tmp;
	if(!_this) return;
	res = mx2d_get_determinent(_this);
	if (! res) {
		mx2d_init(*_this);
		return;
	}
	tmp.m[0] = _this->m[4]/res;
	tmp.m[1] = -1 *  _this->m[1]/res;
	tmp.m[2] =  ( _this->m[1]*_this->m[5] - _this->m[4]*_this->m[2]) / res;
	tmp.m[3] = -1 * _this->m[3]/res;
	tmp.m[4] =  _this->m[0]/res;
	tmp.m[5] = -1 * ( _this->m[0]*_this->m[5] - _this->m[3]*_this->m[2]) / res;
	mx2d_copy(*_this, tmp);
}


void mx2d_apply_coords(M4Matrix2D *_this, Float *x, Float *y)
{
	Float _x, _y;
	if (!_this || !x || !y) return;

	_x = *x * _this->m[0] + *y * _this->m[1] + _this->m[2];
	_y = *x * _this->m[3] + *y * _this->m[4] + _this->m[5];
	*x = _x;
	*y = _y;
}

void mx2d_apply_point(M4Matrix2D *_this, SFVec2f *pt)
{
	mx2d_apply_coords(_this, &pt->x, &pt->y);
}

void mx2d_apply_rect(M4Matrix2D *_this, M4Rect *rc)
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

	mx2d_apply_point(_this, &tl);
	mx2d_apply_point(_this, &tr);
	mx2d_apply_point(_this, &bl);
	mx2d_apply_point(_this, &br);

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

void mx2d_from_mx(M4Matrix2D *mat2D, M4Matrix *mat)
{
	mx2d_init(*mat2D);
	mat2D->m[0] = mat->m[0];
	mat2D->m[1] = mat->m[4];
	mat2D->m[2] = mat->m[12];
	mat2D->m[3] = mat->m[1];
	mat2D->m[4] = mat->m[5];
	mat2D->m[5] = mat->m[13];
}

void mx_apply_rect(M4Matrix *mat, M4Rect *rc)
{
	M4Matrix2D mat2D;
	mx2d_from_mx(&mat2D, mat);
	mx2d_apply_rect(&mat2D, rc);
}

void mx_add_matrix(M4Matrix *mat, M4Matrix *mul)
{
    M4Matrix tmp;
	mx_init(tmp);

    tmp.m[0] = mat->m[0]*mul->m[0] + mat->m[4]*mul->m[1] + mat->m[8]*mul->m[2];
    tmp.m[4] = mat->m[0]*mul->m[4] + mat->m[4]*mul->m[5] + mat->m[8]*mul->m[6];
    tmp.m[8] = mat->m[0]*mul->m[8] + mat->m[4]*mul->m[9] + mat->m[8]*mul->m[10];
    tmp.m[12]= mat->m[0]*mul->m[12] + mat->m[4]*mul->m[13] + mat->m[8]*mul->m[14] + mat->m[12];
    tmp.m[1] = mat->m[1]*mul->m[0] + mat->m[5]*mul->m[1] + mat->m[9]*mul->m[2];
    tmp.m[5] = mat->m[1]*mul->m[4] + mat->m[5]*mul->m[5] + mat->m[9]*mul->m[6];
    tmp.m[9] = mat->m[1]*mul->m[8] + mat->m[5]*mul->m[9] + mat->m[9]*mul->m[10];
    tmp.m[13]= mat->m[1]*mul->m[12] + mat->m[5]*mul->m[13] + mat->m[9]*mul->m[14] + mat->m[13];
    tmp.m[2] = mat->m[2]*mul->m[0] + mat->m[6]*mul->m[1] + mat->m[10]*mul->m[2];
    tmp.m[6] = mat->m[2]*mul->m[4] + mat->m[6]*mul->m[5] + mat->m[10]*mul->m[6];
    tmp.m[10]= mat->m[2]*mul->m[8] + mat->m[6]*mul->m[9] + mat->m[10]*mul->m[10];
    tmp.m[14]= mat->m[2]*mul->m[12] + mat->m[6]*mul->m[13] + mat->m[10]*mul->m[14] + mat->m[14];
	memcpy(mat->m, tmp.m, sizeof(Float)*16);
}

void mx_add_translation(M4Matrix *mat, Float tx, Float ty, Float tz)
{
	Float tmp[3];
	u32 i;
	tmp[0] = mat->m[12];
	tmp[1] = mat->m[13];
	tmp[2] = mat->m[14];
	for (i=0; i<3; i++)
		tmp[i] += (tx*mat->m[i] + ty * mat->m[i+4] + tz * mat->m[i + 8]);
	mat->m[12] = tmp[0];
	mat->m[13] = tmp[1];
	mat->m[14] = tmp[2];
}

void mx_add_scale(M4Matrix *mat, Float sx, Float sy, Float sz)
{
	Float tmp[3];
	u32 i, j;

	tmp[0] = sx; 
	tmp[1] = sy; 
	tmp[2] = sz; 

	for (i=0; i<3; i++) {
		for (j=0; j<3; j++) {
			mat->m[i*4 + j] = mat->m[j+4 * i] * tmp[i];
		}
	}
}

void mx_add_rotation(M4Matrix *mat, Float angle, Float x, Float y, Float z)
{
	M4Matrix tmp;
	Float xx, yy, zz, xy, xz, yz;
	Float nor = (Float) sqrt(x*x + y*y + z*z);
	Float cos_a = (Float) cos(angle);
	Float sin_a = (Float) sin(angle);
	Float icos_a = 1.0f - cos_a;
	if (nor) { x /= nor; y /= nor; z /= nor; }
	xx = x*x; yy = y*y; zz = z*z; xy = x*y; xz = x*z; yz = y*z;
	mx_init(tmp);
    tmp.m[0] = icos_a*xx + cos_a;
    tmp.m[1] = xy*icos_a + z*sin_a;
    tmp.m[2] = xz*icos_a - y*sin_a;
    
	tmp.m[4] = xy*icos_a - z*sin_a;
    tmp.m[5] = icos_a*yy + cos_a;
    tmp.m[6] = yz*icos_a + x*sin_a;

	tmp.m[8] = xz*icos_a + y*sin_a;
    tmp.m[9] = yz*icos_a - x*sin_a;
    tmp.m[10]= icos_a*zz + cos_a;

	mx_add_matrix(mat, &tmp);
}

void mx_from_mx2d(M4Matrix *mat, M4Matrix2D *mat2D)
{
	mx_init(*mat);
	mat->m[0] = mat2D->m[0];
	mat->m[4] = mat2D->m[1];
	mat->m[12] = mat2D->m[2];
	mat->m[1] = mat2D->m[3];
	mat->m[5] = mat2D->m[4];
	mat->m[13] = mat2D->m[5];
}

Bool mx_equal(M4Matrix *mx1, M4Matrix *mx2)
{
	if (mx1->m[0] != mx2->m[0]) return 0;
	if (mx1->m[1] != mx2->m[1]) return 0;
	if (mx1->m[2] != mx2->m[2]) return 0;
	if (mx1->m[4] != mx2->m[4]) return 0;
	if (mx1->m[5] != mx2->m[5]) return 0;
	if (mx1->m[6] != mx2->m[6]) return 0;
	if (mx1->m[8] != mx2->m[8]) return 0;
	if (mx1->m[9] != mx2->m[9]) return 0;
	if (mx1->m[10] != mx2->m[10]) return 0;
	if (mx1->m[12] != mx2->m[12]) return 0;
	if (mx1->m[13] != mx2->m[13]) return 0;
	if (mx1->m[14] != mx2->m[14]) return 0;
	return 1;
}


void mx_inverse(M4Matrix *mx)
{
    Float det;
	M4Matrix rev;
	mx_init(rev);

	assert(! ((mx->m[3] != 0.0) || (mx->m[7] != 0.0) || (mx->m[11] != 0.0) || (mx->m[15] != 1.0)) );

	det = mx->m[0] * mx->m[5] * mx->m[10] + mx->m[1] * mx->m[6] * mx->m[8] + mx->m[2] * mx->m[4] * mx->m[9]
		- mx->m[2] * mx->m[5] * mx->m[8] - mx->m[1] * mx->m[4] * mx->m[10] - mx->m[0] * mx->m[6] * mx->m[9];

//	if (det * det < M4_EPSILON_FLOAT) return;

	/* Calculate inverse(A) = adj(A) / det(A) */
	det = 1.0f / det;
	rev.m[0] = (Float)  ((mx->m[5] * mx->m[10] - mx->m[6] * mx->m[9]) * det);
	rev.m[4] = (Float) -((mx->m[4] * mx->m[10] - mx->m[6] * mx->m[8]) * det);
	rev.m[8] = (Float)  ((mx->m[4] * mx->m[9] - mx->m[5] * mx->m[8]) * det);
	rev.m[1] = (Float) -((mx->m[1] * mx->m[10] - mx->m[2] * mx->m[9]) * det);
	rev.m[5] = (Float)  ((mx->m[0] * mx->m[10] - mx->m[2] * mx->m[8]) * det);
	rev.m[9] = (Float) -((mx->m[0] * mx->m[9] - mx->m[1] * mx->m[8]) * det);
	rev.m[2] = (Float)  ((mx->m[1] * mx->m[6] - mx->m[2] * mx->m[5]) * det);
	rev.m[6] = (Float) -((mx->m[0] * mx->m[6] - mx->m[2] * mx->m[4]) * det);
	rev.m[10] = (Float) ((mx->m[0] * mx->m[5] - mx->m[1] * mx->m[4]) * det);

	/* do translation part*/
	rev.m[12] = -( mx->m[12] * rev.m[0] + mx->m[13] * rev.m[4] + mx->m[14] * rev.m[8] );
	rev.m[13] = -( mx->m[12] * rev.m[1] + mx->m[13] * rev.m[5] + mx->m[14] * rev.m[9] );
	rev.m[14] = -( mx->m[12] * rev.m[2] + mx->m[13] * rev.m[6] + mx->m[14] * rev.m[10] );
	mx_copy(*mx, rev);
}


void mx_apply_vec(M4Matrix *mx, SFVec3f *pt)
{
	SFVec3f res;
	res.x = pt->x * mx->m[0] + pt->y * mx->m[4] + pt->z * mx->m[8] + mx->m[12];
	res.y = pt->x * mx->m[1] + pt->y * mx->m[5] + pt->z * mx->m[9] + mx->m[13];
	res.z = pt->x * mx->m[2] + pt->y * mx->m[6] + pt->z * mx->m[10] + mx->m[14];
	*pt = res;
}

void mx_ortho(M4Matrix *mx, Float left, Float right, Float bottom, Float top, Float z_near, Float z_far)
{
	mx_init(*mx);
	mx->m[0] = 2/(right-left);
	mx->m[5] = 2/(top-bottom);
	mx->m[10] = -2/(z_far-z_near);
	mx->m[12] = (right+left)/(right-left);
	mx->m[13] = (top+bottom)/(top-bottom);
	mx->m[14] = (z_far+z_near)/(z_far-z_near);
	mx->m[15] = 1.0;
}

void mx_perspective(M4Matrix *mx, Float fieldOfView, Float aspectRatio, Float z_near, Float z_far)
{
	Float f = (Float) (cos(fieldOfView/2) / sin(fieldOfView/2));
	mx_init(*mx);
	mx->m[0] = f/aspectRatio;
	mx->m[5] = f;
	mx->m[10] = (z_far+z_near)/(z_near-z_far);
	mx->m[11] = -1;
	mx->m[14] = 2*z_near*z_far/(z_near-z_far);
	mx->m[15] = 0;
}

void mx_lookat(M4Matrix *mx, SFVec3f eye, SFVec3f center, SFVec3f upVector)
{
	SFVec3f f, s, u;
	
	vec_diff(f, center, eye);
	vec_norm(f);
	vec_norm(upVector);

	vec_cross(s, f, upVector);
	vec_cross(u, s, f);
	mx_init(*mx);
	
	mx->m[0] = s.x;
	mx->m[1] = u.x;
	mx->m[2] = -f.x;
	mx->m[4] = s.y;
	mx->m[5] = u.y;
	mx->m[6] = -f.y;
	mx->m[8] = s.z;
	mx->m[9] = u.z;
	mx->m[10] = -f.z;

	mx_add_translation(mx, -eye.x, -eye.y, -eye.z);
}

void mx_decompose(M4Matrix *mx, SFVec3f *translate, SFVec3f *scale, SFRotation *rotate, SFVec3f *shear)
{
	u32 i, j;
	SFVec4f quat;
	Float locmat[16];
	M4Matrix tmp;
	SFVec3f row0, row1, row2;
	Float shear_xy, shear_xz, shear_yz; 
	assert(mx->m[15]);

	memcpy(locmat, mx->m, sizeof(Float)*16);
	/*no perspective*/
    locmat[3] = locmat[7] = locmat[11] = 0.0f;
	/*normalize*/
    for (i=0; i<4; i++) {
        for (j=0; j<4; j++) {
            locmat[4*i+j] /= locmat[15];
        }
    }
    translate->x = locmat[12];
    translate->y = locmat[13];
    translate->z = locmat[14];
    locmat[12] = locmat[13] = locmat[14] = 0.0;
    row0.x = locmat[0]; row0.y = locmat[1]; row0.z = locmat[2];
    row1.x = locmat[4]; row1.y = locmat[5]; row1.z = locmat[6];
    row2.x = locmat[8]; row2.y = locmat[9]; row2.z = locmat[10];

    scale->x = vec_len(row0);
    vec_norm(row0);
    shear_xy = vec_dot(row0, row1);
    row1.x -= row0.x * shear_xy;
    row1.y -= row0.y * shear_xy;
    row1.z -= row0.z * shear_xy;

    scale->y = vec_len(row1);
    vec_norm(row1);
    shear->x = shear_xy / scale->y;

    shear_xz = vec_dot(row0, row2);
    row2.x -= row0.x * shear_xz;
    row2.y -= row0.y * shear_xz;
    row2.z -= row0.z * shear_xz;
    shear_yz = vec_dot(row1, row2);
    row2.x -= row1.x * shear_yz;
    row2.y -= row1.y * shear_yz;
    row2.z -= row1.z * shear_yz;

    scale->z = vec_len(row2);
    vec_norm(row2);
    shear->y = shear_xz / scale->z;
    shear->z = shear_yz / scale->z;

	locmat[0] = row0.x; locmat[4] = row1.x; locmat[8] = row2.x;
	locmat[1] = row0.y; locmat[5] = row1.y; locmat[9] = row2.y;
	locmat[2] = row0.z; locmat[6] = row1.z; locmat[10] = row2.z;

	memcpy(tmp.m, locmat, sizeof(Float)*16);
	quat = quat_from_matrix(&tmp);
	*rotate = quat_to_rotation(&quat);
}

void mx_apply_bbox(M4Matrix *mx, M4BBox *b)
{
	Float var;
	mx_apply_vec(mx, &b->min_edge);
	mx_apply_vec(mx, &b->max_edge);

	if (b->min_edge.x > b->max_edge.x) 
	{
		var = b->min_edge.x; b->min_edge.x = b->max_edge.x; b->max_edge.x = var;
	}
	if (b->min_edge.y > b->max_edge.y) 
	{
		var = b->min_edge.y; b->min_edge.y = b->max_edge.y; b->max_edge.y = var;
	}
	if (b->min_edge.z > b->max_edge.z) 
	{
		var = b->min_edge.z; b->min_edge.z = b->max_edge.z; b->max_edge.z = var;
	}
	m4_bbox_refresh(b);
}


// Apply the rotation portion of a matrix to a vector.
void mx_rotate_vector(M4Matrix *mx, SFVec3f *pt)
{
	SFVec3f res;
	Float den;
	res.x = pt->x * mx->m[0] + pt->y * mx->m[4] + pt->z * mx->m[8];
	res.y = pt->x * mx->m[1] + pt->y * mx->m[5] + pt->z * mx->m[9];
	res.z = pt->x * mx->m[2] + pt->y * mx->m[6] + pt->z * mx->m[10];
	den = pt->x * mx->m[3] + pt->y * mx->m[7] + pt->z * mx->m[11] + mx->m[15];
	if (den == 0.0f) return;
	den = 1/den;
	vec_scale(res, res, den);
	*pt = res;
}

void mx_rotation_matrix_from_vectors(M4Matrix *mx, SFVec3f x, SFVec3f y, SFVec3f z)
{
    mx->m[0] = x.x; mx->m[1] = y.x; mx->m[2] = z.x; mx->m[3] = 0;
    mx->m[4] = x.y; mx->m[5] = y.y; mx->m[6] = z.y; mx->m[7] = 0;
    mx->m[8] = x.z; mx->m[9] = y.z; mx->m[10] = z.z; mx->m[11] = 0;
    mx->m[12] = 0; mx->m[13] = 0; mx->m[14] = 0; mx->m[15] = 1;
}

/*color matrix stuff*/

static void cmat_identity(M4ColorMatrix *_this)
{
	M4ColorMatrix mat;
	cmat_init(&mat);
	_this->identity = memcmp(_this->m, mat.m, sizeof(Float)*20) ? 0 : 1;
}

void cmat_init(M4ColorMatrix *_this)
{
	if (!_this) return;
	memset(_this->m, 0, sizeof(Float)*20);
	_this->m[0] = _this->m[6] = _this->m[12] = _this->m[18] = 1;
	_this->identity = 1;
}
void cmat_set_all(M4ColorMatrix *_this, Float *coefs)
{
	if (!_this || !coefs) return;
}
void cmat_set(M4ColorMatrix *_this, 
				 Float c1, Float c2, Float c3, Float c4, Float c5,
				 Float c6, Float c7, Float c8, Float c9, Float c10,
				 Float c11, Float c12, Float c13, Float c14, Float c15,
				 Float c16, Float c17, Float c18, Float c19, Float c20)
{
	if (!_this) return;
	_this->m[0] = c1; _this->m[1] = c2; _this->m[2] = c3; _this->m[3] = c4; _this->m[4] = c5;
	_this->m[5] = c6; _this->m[6] = c7; _this->m[7] = c8; _this->m[8] = c9; _this->m[9] = c10;
	_this->m[10] = c11; _this->m[11] = c12; _this->m[12] = c13; _this->m[13] = c14; _this->m[14] = c15;
	_this->m[15] = c16; _this->m[16] = c17; _this->m[17] = c18; _this->m[18] = c19; _this->m[19] = c20;
	cmat_identity(_this);
}

void cmat_copy(M4ColorMatrix *_this, M4ColorMatrix *from)
{
	if (!_this || !from) return;
	memcpy(_this->m, from->m, sizeof(Float)*20);
	cmat_identity(_this);
}


void cmat_multiply(M4ColorMatrix *_this, M4ColorMatrix *w)
{
	Float res[20];
	if (!_this || !w || w->identity) return;
	if (_this->identity) {
		cmat_copy(_this, w);
		return;
	}

	res[0] = _this->m[0]*w->m[0] + _this->m[1]*w->m[5] + _this->m[2]*w->m[10] + _this->m[3]*w->m[15];
	res[1] = _this->m[0]*w->m[1] + _this->m[1]*w->m[6] + _this->m[2]*w->m[11] + _this->m[3]*w->m[16];
	res[2] = _this->m[0]*w->m[2] + _this->m[1]*w->m[7] + _this->m[2]*w->m[12] + _this->m[3]*w->m[17];
	res[3] = _this->m[0]*w->m[3] + _this->m[1]*w->m[8] + _this->m[2]*w->m[13] + _this->m[3]*w->m[18];
	res[4] = _this->m[0]*w->m[4] + _this->m[1]*w->m[9] + _this->m[2]*w->m[14] + _this->m[3]*w->m[19] + _this->m[4];
	
	res[5] = _this->m[5]*w->m[0] + _this->m[6]*w->m[5] + _this->m[7]*w->m[10] + _this->m[8]*w->m[15];
	res[6] = _this->m[5]*w->m[1] + _this->m[6]*w->m[6] + _this->m[7]*w->m[11] + _this->m[8]*w->m[16];
	res[7] = _this->m[5]*w->m[2] + _this->m[6]*w->m[7] + _this->m[7]*w->m[12] + _this->m[8]*w->m[17];
	res[8] = _this->m[5]*w->m[3] + _this->m[6]*w->m[8] + _this->m[7]*w->m[13] + _this->m[8]*w->m[18];
	res[9] = _this->m[5]*w->m[4] + _this->m[6]*w->m[9] + _this->m[7]*w->m[14] + _this->m[8]*w->m[19] + _this->m[9];
	
	res[10] = _this->m[10]*w->m[0] + _this->m[11]*w->m[5] + _this->m[12]*w->m[10] + _this->m[13]*w->m[15];
	res[11] = _this->m[10]*w->m[1] + _this->m[11]*w->m[6] + _this->m[12]*w->m[11] + _this->m[13]*w->m[16];
	res[12] = _this->m[10]*w->m[2] + _this->m[11]*w->m[7] + _this->m[12]*w->m[12] + _this->m[13]*w->m[17];
	res[13] = _this->m[10]*w->m[3] + _this->m[11]*w->m[8] + _this->m[12]*w->m[13] + _this->m[13]*w->m[18];
	res[14] = _this->m[10]*w->m[4] + _this->m[11]*w->m[9] + _this->m[12]*w->m[14] + _this->m[13]*w->m[19] + _this->m[14];
	
	res[15] = _this->m[15]*w->m[0] + _this->m[16]*w->m[5] + _this->m[17]*w->m[10] + _this->m[18]*w->m[15];
	res[16] = _this->m[15]*w->m[1] + _this->m[16]*w->m[6] + _this->m[17]*w->m[11] + _this->m[18]*w->m[16];
	res[17] = _this->m[15]*w->m[2] + _this->m[16]*w->m[7] + _this->m[17]*w->m[12] + _this->m[18]*w->m[17];
	res[18] = _this->m[15]*w->m[3] + _this->m[16]*w->m[8] + _this->m[17]*w->m[13] + _this->m[18]*w->m[18];
	res[19] = _this->m[15]*w->m[4] + _this->m[16]*w->m[9] + _this->m[17]*w->m[14] + _this->m[18]*w->m[19] + _this->m[19];
	
	memcpy(_this->m, res, sizeof(Float)*20);
	cmat_identity(_this);
}

#define CLIP_COLOR(val)	if (val<0.0) { val=0; } else if (val>1.0) { val=1.0; }

void cmat_apply_sf(M4ColorMatrix *_this, SFColor *col, Float *alpha)
{
	Float _a, _r, _g, _b, a;
	a = *alpha;
	if (!_this || _this->identity) return;
	_r = col->red * _this->m[0] + col->green * _this->m[1] + col->blue * _this->m[2] + a * _this->m[3] + _this->m[4];
	_g = col->red * _this->m[5] + col->green * _this->m[6] + col->blue * _this->m[7] + a * _this->m[8] + _this->m[9];
	_b = col->red * _this->m[10] + col->green * _this->m[11] + col->blue * _this->m[12] + a * _this->m[13] + _this->m[14];
	_a = col->red * _this->m[15] + col->green * _this->m[16] + col->blue * _this->m[17] + a * _this->m[18] + _this->m[19];
	CLIP_COLOR(_a);
	CLIP_COLOR(_r);
	CLIP_COLOR(_g);
	CLIP_COLOR(_b);
	col->red = _r;
	col->green = _g;
	col->blue = _b;
	*alpha = _a;
}


M4Color cmat_apply(M4ColorMatrix *_this, M4Color col)
{
	Float _a, _r, _g, _b;
	Float a, r, g, b;
	if (!_this || _this->identity) return col;
	a = M4C_A(col); a /= 255;
	r = M4C_R(col); r /= 255;
	g = M4C_G(col); g /= 255;
	b = M4C_B(col); b /= 255;
	_r = r * _this->m[0] + g * _this->m[1] + b * _this->m[2] + a * _this->m[3] + _this->m[4];
	_g = r * _this->m[5] + g * _this->m[6] + b * _this->m[7] + a * _this->m[8] + _this->m[9];
	_b = r * _this->m[10] + g * _this->m[11] + b * _this->m[12] + a * _this->m[13] + _this->m[14];
	_a = r * _this->m[15] + g * _this->m[16] + b * _this->m[17] + a * _this->m[18] + _this->m[19];
	CLIP_COLOR(_a);
	CLIP_COLOR(_r);
	CLIP_COLOR(_g);
	CLIP_COLOR(_b);
	return MAKE_ARGB_FLOAT(_a, _r, _g, _b);
}


#define ConvexCompare(delta)	\
    ( (delta.x > 0) ? -1 :		\
      (delta.x < 0) ?	1 :		\
      (delta.y > 0) ? -1 :		\
      (delta.y < 0) ?	1 :	\
      0 )

#define ConvexGetPointDelta(delta, pprev, pcur )			\
    /* Given a previous point 'pprev', read a new point into 'pcur' */	\
    /* and return delta in 'delta'.				    */	\
    pcur = pts[iread++];						\
    delta.x = pcur.x - pprev.x;					\
    delta.y = pcur.y - pprev.y;					\

#define ConvexCross(p, q) p.x*q.y - p.y*q.x;

#define ConvexCheckTriple						\
    if ( (thisDir = ConvexCompare(dcur)) == -curDir ) {			\
	  ++dirChanges;							\
	  /* if ( dirChanges > 2 ) return NotConvex;		     */ \
    }									\
    curDir = thisDir;							\
    cross = ConvexCross(dprev, dcur);					\
    if ( cross > 0 ) { \
		if ( angleSign == -1 ) return M4_PolyComplex;		\
		angleSign = 1;					\
	}							\
    else if (cross < 0) {	\
		if (angleSign == 1) return M4_PolyComplex;		\
		angleSign = -1;				\
	}						\
    pSecond = pThird;		\
    dprev.x = dcur.x;		\
    dprev.y = dcur.y;							\

u32 polygon2D_check_convexity(M4Point2D *pts, u32 len)
{
	s32 curDir, thisDir = 0, dirChanges = 0, angleSign = 0;
	u32 iread;
    Float cross;
	M4Point2D pSecond, pThird, pSaveSecond;
	M4Point2D dprev, dcur;

    /* Get different point, return if less than 3 diff points. */
    if (len < 3 ) return M4_PolyConvexLine;
    iread = 1;
	ConvexGetPointDelta(dprev, (pts[0]), pSecond);
    pSaveSecond = pSecond;
	/*initial direction */
    curDir = ConvexCompare(dprev);
    while ( iread < len) {
		/* Get different point, break if no more points */
		ConvexGetPointDelta(dcur, pSecond, pThird );
		if ( (dcur.x == 0.0f) && (dcur.y == 0.0f) ) continue;
		/* Check current three points */
		ConvexCheckTriple;
    }

    /* Must check for direction changes from last vertex back to first */
	/* Prepare for 'ConvexCheckTriple' */
    pThird = pts[0];
    dcur.x = pThird.x - pSecond.x;
    dcur.y = pThird.y - pSecond.y;
    if ( ConvexCompare(dcur) ) ConvexCheckTriple;

    /* and check for direction changes back to second vertex */
    dcur.x = pSaveSecond.x - pSecond.x;
    dcur.y = pSaveSecond.y - pSecond.y;
	/* Don't care about 'pThird' now */
    ConvexCheckTriple;			

    /* Decide on polygon type given accumulated status */
    if ( dirChanges > 2 ) return M4_PolyComplex;
    if ( angleSign > 0 ) return M4_PolyConvexCCW;
    if ( angleSign < 0 ) return M4_PolyConvexCW;
    return M4_PolyConvexLine;
}


Bool m4_plane_exists_intersection(M4Plane *plane, M4Plane *with)
{
	SFVec3f cross;
	vec_cross(cross, with->normal, plane->normal);
	return vec_lensq(cross) > M4_EPSILON_FLOAT;
}

Bool m4_plane_intersect_line(M4Plane *plane, SFVec3f *linepoint, SFVec3f *linevec, SFVec3f *outPoint)
{
	Float t, t2;
	t2 = vec_dot(plane->normal, *linevec);
	if (t2 == 0) return 0;
	t = - (vec_dot(plane->normal, *linepoint) + plane->d) / t2;
	if (t<0) return 0;
	vec_scale(*outPoint, *linevec, t);
	vec_add(*outPoint, *linepoint, *outPoint);
	return 1;
}

Bool m4_plane_intersect_plane(M4Plane *plane, M4Plane *with, SFVec3f *linepoint, SFVec3f *linevec)
{
	Float fn00 = vec_len(plane->normal);
	Float fn01 = vec_dot(plane->normal, with->normal);
	Float fn11 = vec_len(with->normal);
	Float det = fn00*fn11 - fn01*fn01;
	if (fabs(det) > M4_EPSILON_FLOAT) {
		Float fc0, fc1;
		SFVec3f v1, v2;
		det = 1.0f / det;
		fc0 = (fn11*-plane->d + fn01*with->d) * det;
		fc1 = (fn00*-with->d + fn01*plane->d) * det;
		vec_cross(*linevec, plane->normal, with->normal);
		vec_scale(v1, plane->normal, fc0);
		vec_scale(v2, with->normal, fc1);
		vec_add(*linepoint, v1, v2);
		return 1;
	}
	return 0;
}

Bool m4_plane_intersect_planes(M4Plane *plane, M4Plane *p1, M4Plane *p2, SFVec3f *outPoint)
{
	SFVec3f lp, lv;
	if (m4_plane_intersect_plane(plane, p1, &lp, &lv))
		return m4_plane_intersect_line(p2, &lp, &lv, outPoint);
	return 0;
}



#define XPLANE 0
#define YPLANE 1
#define ZPLANE 2

Bool m4_ray_hit_box(M4Ray *ray, SFVec3f box_min, SFVec3f box_max, SFVec3f *outPoint)
{
	Double t1, t2, tNEAR=-1e100, tFAR=1e100;
	Double temp;
	s8 xyorz, sign;

	
	if (ray->dir.x == 0) {
		if ((ray->orig.x < box_min.x) || (ray->orig.x > box_max.x))
			return 0;
	} else {
		t1 = (box_min.x - ray->orig.x) / ray->dir.x;
		t2 = (box_max.x - ray->orig.x) / ray->dir.x;
		if (t1 > t2) {
			temp = t1;
			t1 = t2;
			t2 = temp;
		}
		if (t1 > tNEAR) {
			tNEAR = t1;
			xyorz = XPLANE;
			sign = (ray->dir.x < 0) ? 1 : -1;
		}
		if (t2 < tFAR) tFAR = t2;
		if (tNEAR > tFAR) return 0; // box missed
		if (tFAR < 0) return 0; // box behind the ray
	}

	// Check the Y plane
	if (ray->dir.y == 0) {
		if ((ray->orig.y < box_min.y) || (ray->orig.y > box_max.y)) 
			return 0;
	} else {
		t1 = (box_min.y - ray->orig.y) / ray->dir.y;
		t2 = (box_max.y - ray->orig.y) / ray->dir.y;
		if (t1 > t2) {
			temp = t1;
			t1 = t2;
			t2 = temp;
		}
		if (t1 > tNEAR) {
			tNEAR = t1;
			xyorz = YPLANE;
			sign = (ray->dir.y < 0) ? 1 : -1;
		}
		if (t2 < tFAR) tFAR = t2;
		if (tNEAR > tFAR) return 0; // box missed
		if (tFAR < 0) return 0; // box behind the ray
	}

	// Check the Z plane
	if (ray->dir.z == 0) {
		if ((ray->orig.z < box_min.z) || (ray->orig.z > box_max.z))
			return 0;
	} else {
		t1 = (box_min.z - ray->orig.z) / ray->dir.z;
		t2 = (box_max.z - ray->orig.z) / ray->dir.z;
		if (t1 > t2) {
			temp = t1;
			t1 = t2;
			t2 = temp;
		}
		if (t1 > tNEAR) {
			tNEAR = t1;
			xyorz = ZPLANE;
			sign = (ray->dir.z < 0) ? 1 : -1;
		}
		if (t2 < tFAR) tFAR = t2;
		if (tNEAR>tFAR) return 0; // box missed
		if (tFAR < 0) return 0;  // box behind the ray
	}
	if (outPoint) {
		vec_scale(*outPoint, ray->dir, (Float) tNEAR);
		vec_add(*outPoint, *outPoint, ray->orig);
	}
	return 1;
}


Bool m4_ray_hit_sphere(M4Ray *ray, SFVec3f *center, Float radius, SFVec3f *outPoint)
{
	SFVec3f radv;
	Float sqdist, center_proj, hcord;
	if (center) {
		vec_diff(radv, *center, ray->orig);
	} else {
		vec_scale(radv, ray->orig, -1);
	}
	sqdist = vec_lensq(radv);
	center_proj = vec_dot(radv, ray->dir);
	hcord = radius * radius - (sqdist - center_proj*center_proj);
    if (hcord < 0.0f) return 0;
	center_proj -= (Float) sqrt(hcord);
	if (center_proj<0) return 0;
	if (outPoint) {
		vec_scale(radv, ray->dir, center_proj);
		vec_add(*outPoint, ray->orig, radv);
	}
	return 1;
}

/*
 *		Tomas Möller and Ben Trumbore.
 *	 Fast, minimum storage ray-triangle intersection. 
 *		Journal of graphics tools, 2(1):21-28, 1997
 *
 */
Bool m4_ray_hit_triangle(M4Ray *ray, SFVec3f *v0, SFVec3f *v1, SFVec3f *v2, Float *dist)
{
	Float u, v, det, inv_det;
	SFVec3f edge1, edge2, tvec, pvec, qvec;
	/* find vectors for two edges sharing vert0 */
	vec_diff(edge1, *v1, *v0);
	vec_diff(edge2, *v2, *v0);
	/* begin calculating determinant - also used to calculate U parameter */
	vec_cross(pvec, ray->dir, edge2);
	/* if determinant is near zero, ray lies in plane of triangle */
	det = vec_dot(edge1, pvec);
	if ((det > -M4_EPSILON_FLOAT) && (det < M4_EPSILON_FLOAT)) return 0;
	inv_det = 1.0f / det;
	/* calculate distance from vert0 to ray origin */
	vec_diff(tvec, ray->orig, *v0);
	/* calculate U parameter and test bounds */
	u = vec_dot(tvec, pvec) * inv_det;
	if ((u < 0.0) || (u > 1.0)) return 0;
	/* prepare to test V parameter */
	vec_cross(qvec, tvec, edge1);
	/* calculate V parameter and test bounds */
	v = vec_dot(ray->dir, qvec) * inv_det;
	if ((v < 0.0) || (u + v > 1.0)) return 0;
	/* calculate t, ray intersects triangle */
	*dist = vec_dot(edge2, qvec) * inv_det;
	return 1;
}

Bool m4_ray_hit_triangle_backcull(M4Ray *ray, SFVec3f *v0, SFVec3f *v1, SFVec3f *v2, Float *dist)
{
	Float u, v, det;
	SFVec3f edge1, edge2, tvec, pvec, qvec;
	/* find vectors for two edges sharing vert0 */
	vec_diff(edge1, *v1, *v0);
	vec_diff(edge2, *v2, *v0);
	/* begin calculating determinant - also used to calculate U parameter */
	vec_cross(pvec, ray->dir, edge2);
	/* if determinant is near zero, ray lies in plane of triangle */
	det = vec_dot(edge1, pvec);
	if (det < M4_EPSILON_FLOAT) return 0;
	/* calculate distance from vert0 to ray origin */
	vec_diff(tvec, ray->orig, *v0);
	/* calculate U parameter and test bounds */
	u = vec_dot(tvec, pvec);
	if ((u < 0.0) || (u > det)) return 0;
	/* prepare to test V parameter */
	vec_cross(qvec, tvec, edge1);
	/* calculate V parameter and test bounds */
	v = vec_dot(ray->dir, qvec);
	if ((v < 0.0) || (u + v > det)) return 0;
	/* calculate t, scale parameters, ray intersects triangle */
	*dist = vec_dot(edge2, qvec) / det;
	return 1;
}

SFVec3f m4_closest_point_to_line(SFVec3f line_pt, SFVec3f line_vec, SFVec3f pt)
{
	SFVec3f c;
	Float t;
	vec_diff(c, pt, line_pt);
	t = vec_dot(line_vec, c);
	vec_scale(c, line_vec, t);
	vec_add(c, c, line_pt);
	return c;
}


/*we should only need a full matrix product for frustrum setup*/
void mx_add_matrix_4x4(M4Matrix *mat, M4Matrix *mul)
{
    M4Matrix tmp;
	mx_init(tmp);
    tmp.m[0] = mat->m[0]*mul->m[0] + mat->m[4]*mul->m[1] + mat->m[8]*mul->m[2] + mat->m[12]*mul->m[3];
    tmp.m[1] = mat->m[1]*mul->m[0] + mat->m[5]*mul->m[1] + mat->m[9]*mul->m[2] + mat->m[13]*mul->m[3];
    tmp.m[2] = mat->m[2]*mul->m[0] + mat->m[6]*mul->m[1] + mat->m[10]*mul->m[2] + mat->m[14]*mul->m[3];
    tmp.m[3] = mat->m[3]*mul->m[0] + mat->m[7]*mul->m[1] + mat->m[11]*mul->m[2] + mat->m[15]*mul->m[3];
    tmp.m[4] = mat->m[0]*mul->m[4] + mat->m[4]*mul->m[5] + mat->m[8]*mul->m[6] + mat->m[12]*mul->m[7];
    tmp.m[5] = mat->m[1]*mul->m[4] + mat->m[5]*mul->m[5] + mat->m[9]*mul->m[6] + mat->m[13]*mul->m[7];
    tmp.m[6] = mat->m[2]*mul->m[4] + mat->m[6]*mul->m[5] + mat->m[10]*mul->m[6] + mat->m[14]*mul->m[7];
    tmp.m[7] = mat->m[3]*mul->m[4] + mat->m[7]*mul->m[5] + mat->m[11]*mul->m[6] + mat->m[15]*mul->m[7];
    tmp.m[8] = mat->m[0]*mul->m[8] + mat->m[4]*mul->m[9] + mat->m[8]*mul->m[10] + mat->m[12]*mul->m[11];
    tmp.m[9] = mat->m[1]*mul->m[8] + mat->m[5]*mul->m[9] + mat->m[9]*mul->m[10] + mat->m[13]*mul->m[11];
    tmp.m[10] = mat->m[2]*mul->m[8] + mat->m[6]*mul->m[9] + mat->m[10]*mul->m[10] + mat->m[14]*mul->m[11];
    tmp.m[11] = mat->m[3]*mul->m[8] + mat->m[7]*mul->m[9] + mat->m[11]*mul->m[10] + mat->m[15]*mul->m[11];
    tmp.m[12] = mat->m[0]*mul->m[12] + mat->m[4]*mul->m[13] + mat->m[8]*mul->m[14] + mat->m[12]*mul->m[15];
    tmp.m[13] = mat->m[1]*mul->m[12] + mat->m[5]*mul->m[13] + mat->m[9]*mul->m[14] + mat->m[13]*mul->m[15];
    tmp.m[14] = mat->m[2]*mul->m[12] + mat->m[6]*mul->m[13] + mat->m[10]*mul->m[14] + mat->m[14]*mul->m[15];
    tmp.m[15] = mat->m[3]*mul->m[12] + mat->m[7]*mul->m[13] + mat->m[11]*mul->m[14] + mat->m[15]*mul->m[15];
	memcpy(mat->m, tmp.m, sizeof(Float)*16);
}


void mx_apply_vec_4x4(M4Matrix *mx, SFVec4f *vec)
{
	SFVec4f res;
	res.x = mx->m[0] * vec->x + mx->m[4] * vec->y + mx->m[8] * vec->z + mx->m[12] * vec->q;
	res.y = mx->m[1] * vec->x + mx->m[5] * vec->y + mx->m[9] * vec->z + mx->m[13] * vec->q;
	res.z = mx->m[2] * vec->x + mx->m[6] * vec->y + mx->m[10] * vec->z + mx->m[14] * vec->q;
	res.q = mx->m[3] * vec->x + mx->m[7] * vec->y + mx->m[11] * vec->z + mx->m[15] * vec->q;
	*vec = res;
}

/*
 *	Taken from MESA/GLU (LGPL)
 *
 * Compute inverse of 4x4 transformation matrix.
 * Code contributed by Jacques Leroy jle@star.be
 * Return 1 for success, 0 for failure (singular matrix)
 */

Bool mx_inverse_4x4(M4Matrix *mx)
{

#define SWAP_ROWS(a, b) { Float *_tmp = a; (a)=(b); (b)=_tmp; }
	Float wtmp[4][8];
	Float m0, m1, m2, m3, s;
	Float *r0, *r1, *r2, *r3;
	M4Matrix res;
	r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];
	r0[0] = mx->m[0]; r0[1] = mx->m[4]; r0[2] = mx->m[8]; r0[3] = mx->m[12]; r0[4] = 1.0; r0[5] = r0[6] = r0[7] = 0.0;
	r1[0] = mx->m[1]; r1[1] = mx->m[5]; r1[2] = mx->m[9]; r1[3] = mx->m[13]; r1[5] = 1.0; r1[4] = r1[6] = r1[7] = 0.0;
	r2[0] = mx->m[2]; r2[1] = mx->m[6]; r2[2] = mx->m[10]; r2[3] = mx->m[14]; r2[6] = 1.0; r2[4] = r2[5] = r2[7] = 0.0;
	r3[0] = mx->m[3]; r3[1] = mx->m[7]; r3[2] = mx->m[11]; r3[3] = mx->m[15]; r3[7] = 1.0; r3[4] = r3[5] = r3[6] = 0.0;

	/* choose pivot - or die */
	if (fabs(r3[0]) > fabs(r2[0])) SWAP_ROWS(r3, r2);
	if (fabs(r2[0]) > fabs(r1[0])) SWAP_ROWS(r2, r1);
	if (fabs(r1[0]) > fabs(r0[0])) SWAP_ROWS(r1, r0);
	if (0.0 == r0[0]) return 0;
	
	/*eliminate first variable*/
	m1 = r1[0] / r0[0];
	m2 = r2[0] / r0[0];
	m3 = r3[0] / r0[0];
	s = r0[1];
	r1[1] -= m1 * s;
	r2[1] -= m2 * s;
	r3[1] -= m3 * s;
	s = r0[2];
	r1[2] -= m1 * s;
	r2[2] -= m2 * s;
	r3[2] -= m3 * s;
	s = r0[3];
	r1[3] -= m1 * s;
	r2[3] -= m2 * s;
	r3[3] -= m3 * s;
	s = r0[4];
	if (s != 0.0) {
		r1[4] -= m1 * s;
		r2[4] -= m2 * s;
		r3[4] -= m3 * s;
	}
	s = r0[5];
	if (s != 0.0) {
		r1[5] -= m1 * s;
		r2[5] -= m2 * s;
		r3[5] -= m3 * s;
	}
	s = r0[6];
	if (s != 0.0) {
		r1[6] -= m1 * s;
		r2[6] -= m2 * s;
		r3[6] -= m3 * s;
	}
	s = r0[7];
	if (s != 0.0) {
		r1[7] -= m1 * s;
		r2[7] -= m2 * s;
		r3[7] -= m3 * s;
	}

	/* choose pivot - or die */
	if (fabs(r3[1]) > fabs(r2[1])) SWAP_ROWS(r3, r2);
	if (fabs(r2[1]) > fabs(r1[1])) SWAP_ROWS(r2, r1);
	if (0.0 == r1[1]) return 0;
	
	/* eliminate second variable */
	m2 = r2[1] / r1[1];
	m3 = r3[1] / r1[1];
	r2[2] -= m2 * r1[2];
	r3[2] -= m3 * r1[2];
	r2[3] -= m2 * r1[3];
	r3[3] -= m3 * r1[3];
	s = r1[4];
	if (0.0 != s) {
		r2[4] -= m2 * s;
		r3[4] -= m3 * s;
	}
	s = r1[5];
	if (0.0 != s) {
		r2[5] -= m2 * s;
		r3[5] -= m3 * s;
	}
	s = r1[6];
	if (0.0 != s) {
		r2[6] -= m2 * s;
		r3[6] -= m3 * s;
	}
	s = r1[7];
	if (0.0 != s) {
		r2[7] -= m2 * s;
		r3[7] -= m3 * s;
	}

	/* choose pivot - or die */
	if (fabs(r3[2]) > fabs(r2[2])) SWAP_ROWS(r3, r2);
	if (0.0 == r2[2]) return 0;

	/* eliminate third variable */
	m3 = r3[2] / r2[2];
	r3[3] -= m3 * r2[3]; r3[4] -= m3 * r2[4]; r3[5] -= m3 * r2[5]; r3[6] -= m3 * r2[6]; r3[7] -= m3 * r2[7];
	/* last check */
	if (0.0 == r3[3]) return 0;

	s = 1.0f / r3[3];		/* now back substitute row 3 */
	r3[4] *= s;
	r3[5] *= s;
	r3[6] *= s;
	r3[7] *= s;
	
	m2 = r2[3];			/* now back substitute row 2 */
	s = 1.0f / r2[2];
	r2[4] = s * (r2[4] - r3[4] * m2); r2[5] = s * (r2[5] - r3[5] * m2); r2[6] = s * (r2[6] - r3[6] * m2); r2[7] = s * (r2[7] - r3[7] * m2);
	m1 = r1[3]; 
	r1[4] -= r3[4] * m1; r1[5] -= r3[5] * m1; r1[6] -= r3[6] * m1; r1[7] -= r3[7] * m1;
	m0 = r0[3];
	r0[4] -= r3[4] * m0; r0[5] -= r3[5] * m0; r0[6] -= r3[6] * m0; r0[7] -= r3[7] * m0;

	m1 = r1[2];			/* now back substitute row 1 */
	s = 1.0f / r1[1];
	r1[4] = s * (r1[4] - r2[4] * m1); r1[5] = s * (r1[5] - r2[5] * m1), r1[6] = s * (r1[6] - r2[6] * m1); r1[7] = s * (r1[7] - r2[7] * m1);
	m0 = r0[2];
	r0[4] -= r2[4] * m0; r0[5] -= r2[5] * m0; r0[6] -= r2[6] * m0; r0[7] -= r2[7] * m0;

	m0 = r0[1];			/* now back substitute row 0 */
	s = 1.0f / r0[0];
	r0[4] = s * (r0[4] - r1[4] * m0); r0[5] = s * (r0[5] - r1[5] * m0); r0[6] = s * (r0[6] - r1[6] * m0); r0[7] = s * (r0[7] - r1[7] * m0);

	mx_init(res)
	res.m[0] = r0[4]; res.m[4] = r0[5]; res.m[8] = r0[6]; res.m[12] = r0[7];
	res.m[1] = r1[4]; res.m[5] = r1[5], res.m[9] = r1[6]; res.m[13] = r1[7];
	res.m[2] = r2[4]; res.m[6] = r2[5]; res.m[10] = r2[6]; res.m[14] = r2[7];
	res.m[3] = r3[4]; res.m[7] = r3[5]; res.m[11] = r3[6]; res.m[15] = r3[7];
	mx_copy(*mx, res);
	return 1;
#undef SWAP_ROWS

}




SFVec4f quat_from_matrix(M4Matrix *mx)
{
	SFVec4f res;
	Float diagonal, s;
    diagonal = mx->m[0] + mx->m[5] + mx->m[10];

    if (diagonal > 0.0) {
        s = (Float) sqrt(diagonal + 1.0f);
        res.q = s / 2.0f;
        s = 0.5f / s;
        res.x = (mx->m[6] - mx->m[9]) * s;
        res.y = (mx->m[8] - mx->m[2]) * s;
        res.z = (mx->m[1] - mx->m[4]) * s;
    } else {
		Float q[4];
        u32 i, j, k;
        static const u32 next[3] = { 1, 2, 0 };
        i = 0;
        if (mx->m[5] > mx->m[0]) { i = 1; }
        if (mx->m[10] > mx->m[4*i+i]) { i = 2; }
        j = next[i];
        k = next[j];
        s = (Float) sqrt(1 + mx->m[4*i + i] - (mx->m[4*j+j] + mx->m[4*k+k]) );
        q[i] = s * 0.5f;
        if (s != 0.0f) { s = 0.5f / s; }
        q[3] = (mx->m[4*j+k] - mx->m[4*k+j]) * s;
        q[j] = (mx->m[4*i+j] + mx->m[4*j+i]) * s;
        q[k] = (mx->m[4*i+k] + mx->m[4*k+i]) * s;
		res.x = q[0]; res.y = q[1]; res.z = q[2]; res.q = q[3];
    }
	return res;
}

SFRotation quat_to_rotation(SFVec4f *quat)
{
	SFRotation r;
    Float val = (Float) acos(quat->q);
    if (val == 0.0f) {
        r.xAxis = r.yAxis = 0.0f;
        r.zAxis = 1.0f;
		r.angle = 0.0f;
    } else {
		SFVec3f axis;
        Float sin_val = (Float) sin(val);
        axis.x = quat->x / sin_val;
        axis.y = quat->y / sin_val;
        axis.z = quat->z / sin_val;
		vec_norm(axis);
		r.xAxis = axis.x;
		r.yAxis = axis.y;
		r.zAxis = axis.z;
        r.angle = 2 * val;
    }
	return r;
}

SFVec4f quat_from_rotation(SFRotation rot)
{
	SFVec4f res;
	Float s;
	Float scale = (Float) sqrt((rot.xAxis * rot.xAxis) + (rot.yAxis * rot.yAxis) + (rot.zAxis * rot.zAxis));

	/* no rotation - use (multiplication ???) identity quaternion */
	if (scale == 0.0f) {
		res.q = 1;
		res.x = 0;
		res.y = 0;
		res.z = 0;
	} else {
		s = (Float) sin(rot.angle/2);
		res.q = (Float) cos(rot.angle / 2);
		res.x = s * (rot.xAxis / scale);
		res.y = s * (rot.yAxis / scale);
		res.z = s * (rot.zAxis / scale);
		quat_norm(res);
	}
	return res;
}

SFVec4f quat_from_axis_cos(SFVec3f axis, Float cos_a)
{
	SFRotation r;
	if (cos_a < -1.0f) cos_a = -1.0f;
	else if (cos_a > 1.0f) cos_a = 1.0f;
	r.xAxis = axis.x; r.yAxis = axis.y; r.zAxis = axis.z;
	r.angle = (Float) acos(cos_a);
	return quat_from_rotation(r);
}

void quat_conjugate(SFVec4f *quat)
{
	quat->x *= -1;
	quat->y *= -1;
	quat->z *= -1;
}

SFVec4f quat_get_inv(SFVec4f *quat)
{
	SFVec4f ret = *quat;
	quat_conjugate(&ret);
	quat_norm(ret);
	return ret;
}


SFVec4f quat_multiply(SFVec4f *q1, SFVec4f *q2)
{
	SFVec4f ret;
	ret.q = (q1->q * q2->q) - (q1->x * q2->x) - (q1->y * q2->y) - (q1->z * q2->z);
	ret.x = (q1->q * q2->x) + (q1->x * q2->q) + (q1->y * q2->z) - (q1->z * q2->y);
	ret.y = (q1->q * q2->y) + (q1->y * q2->q) - (q1->x * q2->z) + (q1->z * q2->x);
	ret.z = (q1->q * q2->z) + (q1->z * q2->q) + (q1->x * q2->y) - (q1->y * q2->x);
	return ret;
}

SFVec3f quat_rotate(SFVec4f *quat, SFVec3f *vec)
{
	SFVec3f ret;
	SFVec4f q_v, q_i, q_r1, q_r2;
	q_v.q = 0.0;
	q_v.x = vec->x;
	q_v.y = vec->y;
	q_v.z = vec->z;
	q_i = quat_get_inv(quat);
	q_r1 = quat_multiply(&q_v, &q_i);
	q_r2 = quat_multiply(quat, &q_r1);
	ret.x = q_r2.x;
	ret.y = q_r2.y;
	ret.z = q_r2.z;
	return ret;
}

/*
 * Code from www.gamasutra.com/features/19980703/quaternions_01.htm,
 * Listing 5.
 *
 * SLERP(p, q, t) = [p sin((1 - t)a) + q sin(ta)] / sin(a)
 *
 * where a is the arc angle, quaternions pq = cos(q) and 0 <= t <= 1
 */
SFVec4f quat_slerp(SFVec4f q1, SFVec4f q2, Float frac)
{
	SFVec4f res;
	Float omega, cosom, sinom, scale0, scale1, q2_array[4];

	cosom = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.q * q2.q;
	if (cosom < 0.0f) {
		cosom = -cosom;
		q2_array[0] = -q2.x;
		q2_array[1] = -q2.y;
		q2_array[2] = -q2.z;
		q2_array[3] = -q2.q;
	} else {
		q2_array[0] = q2.x;
		q2_array[1] = q2.y;
		q2_array[2] = q2.z;
		q2_array[3] = q2.q;
	}

	/* calculate coefficients */
	if ((1.0f - cosom) > 0.0001f) {
		omega = (Float) acos(cosom);
		sinom = (Float) sin(omega);
		scale0 = (Float) sin((1.0 - frac) * omega) / sinom;
		scale1 = (Float) sin(frac * omega) / sinom;
	} else {
		/* q1 & q2 are very close, so do linear interpolation */
		scale0 = 1.0f - frac;
		scale1 = frac;
	}
	res.x = scale0 * q1.x + scale1 * q2_array[0];
	res.y = scale0 * q1.y + scale1 * q2_array[1];
	res.z = scale0 * q1.z + scale1 * q2_array[2];
	res.q = scale0 * q1.q + scale1 * q2_array[3];
	return res;
}
