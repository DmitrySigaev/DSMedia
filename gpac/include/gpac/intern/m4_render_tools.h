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

#ifndef _M4_RENDER_TOOLS_H_
#define _M4_RENDER_TOOLS_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <gpac/m4_graphics.h>
#include <gpac/m4_scenegraph_vrml.h>

/*math.h is not included in main config (C++ clash on win32)*/
#include <math.h>

/*default resolution for N-bezier curves*/
#define M4_DEFAULT_RESOLUTION		64

/*subpath object (one subpath per moveTo)*/
typedef struct
{
	Bool closed;
	u32 pointlen;
	u32 pointmax;
	M4Point2D *point;
} M4SubPath;

/*macro used to keep track of bounds*/
#define PATH_CHECKBOUNDS(_this, x, y)	\
{	\
	if (x<_this->min_x) _this->min_x = x;	\
	if (x>_this->max_x)_this->max_x = x;	\
	if (y<_this->min_y) _this->min_y = y;	\
	if (y>_this->max_y)_this->max_y = y;	\
}	\

/*path object*/
typedef struct _path
{
	u32 fill_mode;
	/*2nd and 3rd order bezier fineness (>3rd: fineness not supported, arbitrary subdivide)*/
	Float fineness;
	/*beziers / ellipses res*/
	u32 resolution;

	/*subpaths info*/
	u32 subpathlen;
	u32 subpathmax;
	M4SubPath **subpath;

	Float bx, by;	/* beginning of subpath */
	Float cx, cy;	/* current point */

	/*bounds info*/
	Float min_x, min_y, max_x, max_y;
} M4Path;

M4SubPath *new_subpath2D();
void subpath2D_free(M4SubPath *subpath);
void subpath2D_reset(M4SubPath *subpath);
M4Err subpath2D_addpoint(M4SubPath *subpath, Float x, Float y);
void subpath2D_close(M4SubPath *subpath);
M4Err m4_path_newsubpath(M4Path *path);
M4Err m4_path_addsubpath(M4Path *path, M4SubPath *src, Bool join);


/*iteration info*/	
typedef struct 
{
	Float len;
	Float dx, dy;
	Float start_x, start_y;
} IterInfo;

typedef struct
{
	u32 num_seg;
	IterInfo *seg;
	Float length;
} M4PathIterator;

/*inits path iteration*/
M4PathIterator *m4_path_new_iterator(LPM4PATH _this);
void m4_path_delete_iterator(M4PathIterator *it);

/*gets transformation matrix at given point (offset) 
the transform is so that a local system is translated to the given point, its x-axis tangent to the path and in 
the same direction (path direction is from first point to last point)
@offset: length on the path in local system unit
@follow_tangent: indicates if transformation shall be computed if @offset indicates a point outside 
the path (<0 or >path_length). In which case the path shall be virtually extended by the tangent at 
origin (@offset <0) or at end (@offset>path_length). Otherwise the transformation is not computed 
and 0 is returned
@mat: matrix to be transformed (transformation shall be appended) - the matrix shall not be initialized
@smooth_edges: indicates if discontinuities shall be smoothed. 
	If not set, the rotation angle THETA is
the slope (DX/DY) of the current segment found.
	if set, the amount of the object that lies on next segment shall be computed according to 
@length_after_point . let:
	len_last: length of current checked segment
	len1: length of all previous segments so that len1 + len_last >= offset
then if (offset + length_after_point > len1 + len_last) {
		ratio = (len1 + len_last - offset) / length_after_point;

	then THETA = ratio * slope(L1) + (1-ratio) * slope(L2)
(of course care must be taken for PI/2 angles & the like)

returns 1 if matrix has been updated, 0 otherwise (fail or point out of path without tangent extension)
*/
Bool m4_pathiterator_get_transform_at_offset(M4PathIterator *it, Float offset, Bool follow_tangent, M4Matrix2D *mat, Bool smooth_edges, Float length_after_point);


/*YUV->RGB routines*/
void yuv2rgb_555(unsigned char *dst, s32 dst_stride,
				 unsigned char *y_src, unsigned char * u_src, unsigned char * v_src,
				 s32 y_stride, s32 uv_stride, s32 width, s32 height);
void yuv2rgb_565(unsigned char * dst, s32 dst_stride,
				 unsigned char* y_src, unsigned char* u_src, unsigned char* v_src,
				 s32 y_stride, s32 uv_stride, s32 width, s32 height);
void yuv2bgr_24(unsigned char *dst, s32 dststride, 
				unsigned char *y_src, unsigned char *u_src, unsigned char * v_src, 
				s32 y_stride, s32 uv_stride, s32 width, s32 height);
void yuv2rgb_24(unsigned char *dst, s32 dststride, unsigned char *y_src, unsigned char *u_src, unsigned char * v_src, 
				 s32 y_stride, s32 uv_stride, s32 width, s32 height);
void yuv2rgb_32(unsigned char *dst, s32 dststride, 
				unsigned char *y_src, unsigned char *v_src, unsigned char * u_src,
				s32 y_stride, s32 uv_stride, s32 width, s32 height);
void yuva2rgb_32(unsigned char *dst, s32 dststride, 
				 unsigned char *y_src, unsigned char *v_src, unsigned char * u_src, unsigned char *a_src,
				 s32 y_stride, s32 uv_stride, s32 width, s32 height);
/*for openGL texturing*/
void yuv2rgb_24_flip(unsigned char *dst, s32 dststride, unsigned char *y_src, unsigned char *u_src, unsigned char * v_src, 
				 s32 y_stride, s32 uv_stride, s32 width, s32 height);

/*bounds object used by the renderer*/
typedef struct
{
	/*min_x, min_y, min_z*/
	SFVec3f min_edge;
	/*max_x, max_y, max_z*/
	SFVec3f max_edge;

	/*center of bbox*/
	SFVec3f center;
	/*radius bbox bounding sphere*/
	Float radius;

	Bool is_set;
} M4BBox;

/*matrix2D tools*/
#define mx2d_init(_obj) { memset((_obj).m, 0, sizeof(Float)*6); (_obj).m[0] = (_obj).m[4] = 1.0; }
#define mx2d_copy(_obj, from) memcpy((_obj).m, (from).m, sizeof(Float)*6);
void mx2d_add_matrix(M4Matrix2D *_this, M4Matrix2D *from);
void mx2d_add_translation(M4Matrix2D *_this, Float cx, Float cy);
void mx2d_add_rotation(M4Matrix2D *_this, Float cx, Float cy, Float angle);
void mx2d_add_scale(M4Matrix2D *_this, Float scale_x, Float scale_y);
void mx2d_add_scale_at(M4Matrix2D *_this, Float scale_x, Float scale_y, Float cx, Float cy, Float angle);
void mx2d_add_skew(M4Matrix2D *_this, Float skew_x, Float skew_y);
void mx2d_add_skewX(M4Matrix2D *_this, Float angle);
void mx2d_add_skewY(M4Matrix2D *_this, Float angle);
void mx2d_inverse(M4Matrix2D *_this);
void mx2d_apply_coords(M4Matrix2D *_this, Float *x, Float *y);
void mx2d_apply_point(M4Matrix2D *_this, SFVec2f *pt);
/*gets enclosing rect of rect after transformed*/
void mx2d_apply_rect(M4Matrix2D *_this, M4Rect *rc);
/*trash all z info and gets a 2D matrix*/
void mx2d_from_mx(M4Matrix2D *mat2D, M4Matrix *mat);

/*matrix tools - all tools assume affine matrices, except when _4x4 is specified*/
#define mx_init(_obj) { memset((_obj).m, 0, sizeof(Float)*16); (_obj).m[0] = (_obj).m[5] = (_obj).m[10] = (_obj).m[15] = 1.0; }
#define mx_copy(_obj, from) memcpy((_obj).m, (from).m, sizeof(Float)*16);
/*converts 2D matrix to 3D matrix*/
void mx_from_mx2d(M4Matrix *mat, M4Matrix2D *mat2D);
/*returns 1 if matrices are same*/
Bool mx_equal(M4Matrix *mx1, M4Matrix *mx2);
/*translates matrix*/
void mx_add_translation(M4Matrix *mat, Float tx, Float ty, Float tz);
/*scales matrix*/
void mx_add_scale(M4Matrix *mat, Float sx, Float sy, Float sz);
/*rotates matrix*/
void mx_add_rotation(M4Matrix *mat, Float angle, Float x, Float y, Float z);
/*multiply: mat = mat*mul*/
void mx_add_matrix(M4Matrix *mat, M4Matrix *mul);
/*affine matrix inversion*/
void mx_inverse(M4Matrix *mx);
void mx_apply_vec(M4Matrix *mx, SFVec3f *pt);
/*gets enclosing rect of rect after transformed (matrix is considered as 2D)*/
void mx_apply_rect(M4Matrix *_this, M4Rect *rc);
/*creates ortho matrix*/
void mx_ortho(M4Matrix *mx, Float left, Float right, Float bottom, Float top, Float z_near, Float z_far);
/*creates perspective matrix*/
void mx_perspective(M4Matrix *mx, Float fieldOfView, Float aspectRatio, Float z_ear, Float z_far);
/*creates look matrix*/
void mx_lookat(M4Matrix *mx, SFVec3f position, SFVec3f target, SFVec3f upVector);
/*gets enclosing box of box after transformed*/
void mx_apply_bbox(M4Matrix *mx, M4BBox *b);
/*multiply: mat = mat*mul as full 4x4 matrices*/
void mx_add_matrix_4x4(M4Matrix *mat, M4Matrix *mul);
/*generic 4x4 matrix inversion - returns 0 if failure*/
Bool mx_inverse_4x4(M4Matrix *mx);
/*apply 4th dim vector*/
void mx_apply_vec_4x4(M4Matrix *mx, SFVec4f *vec);
/*decomposes matrix into translation, scale, shear and rotate - only use with affine matrix */
void mx_decompose(M4Matrix *mx, SFVec3f *translate, SFVec3f *scale, SFRotation *rotate, SFVec3f *shear);
/*only rotates the vector*/
void mx_rotate_vector(M4Matrix *mx, SFVec3f *pt);
/*rotation matrix transforming local axis in given norm vectors*/
void mx_rotation_matrix_from_vectors(M4Matrix *mx, SFVec3f x_axis, SFVec3f y_axis, SFVec3f z_axis);


/*color matrix tools*/
void cmat_init(M4ColorMatrix *_this);
void cmat_set(M4ColorMatrix *_this, 
				 Float c1, Float c2, Float c3, Float c4, Float c5,
				 Float c6, Float c7, Float c8, Float c9, Float c10,
				 Float c11, Float c12, Float c13, Float c14, Float c15,
				 Float c16, Float c17, Float c18, Float c19, Float c20);
void cmat_copy(M4ColorMatrix *_this, M4ColorMatrix *from);
void cmat_multiply(M4ColorMatrix *_this, M4ColorMatrix *w);
M4Color cmat_apply(M4ColorMatrix *_this, M4Color col);
void cmat_apply_sf(M4ColorMatrix *_this, SFColor *col, Float *alpha);
		
/*color tools*/
#define M4C_A(c) (u8) ((c)>>24)
#define M4C_R(c) (u8) ( ((c)>>16) & 0xFF)
#define M4C_G(c) (u8) ( ((c)>>8) & 0xFF)
#define M4C_B(c) (u8) ( (c) & 0xFF)
#define M4C_ARGB(a, r, g, b) ( ( (a) << 24) | ((r) << 16) | ((g) << 8) | (b) )
#define M4C_565(r, g, b) (u16) (((r & 248)<<8) + ((g & 252)<<3)  + (b>>3))
#define M4C_555(r, g, b) (u16) (((r & 248)<<7) + ((g & 248)<<2)  + (b>>3))
#define M4C_AG(a, g) (u16) ( (a << 8) | g)
#define M4COL_565(c) (((M4C_R(c) & 248)<<8) + ((M4C_G(c) & 252)<<3)  + (M4C_B(c)>>3))
#define M4COL_555(c) (((M4C_R(c) & 248)<<7) + ((M4C_G(c) & 248)<<2)  + (M4C_B(c)>>3))
#define M4COL_AG(c) ( (M4C_A(c) << 8) | M4C_R(c))

/*makes ARGB from unsigned char vals (color range 0-255)*/
#define MAKE_ARGB(a, r, g, b) M4C_ARGB(a, r, g, b)
/*makes ARGB from float vals (color range 0-1)*/
#define MAKE_ARGB_FLOAT(a, r, g, b) M4C_ARGB( (u8) (a*255), (u8) (r*255), (u8) (g*255), (u8) (b*255))
/*makes ARGB from unsigned int vals (color range 0-255)*/
#define MAKE_ARGB_32(a, r, g, b) M4C_ARGB(a&0xFF, r&0xFF, g&0xFF, b&0xFF)

/*adds @rc2 to @rc1 - the new @rc1 contains the old @rc1 and @rc2*/
static M4INLINE void m4_rect_union(M4Rect *rc1, M4Rect *rc2) 
{
	if (!rc1->width || !rc1->height) {*rc1=*rc2; return;}

	if (rc2->x < rc1->x) {
		rc1->width += rc1->x - rc2->x;
		rc1->x = rc2->x;
	}
	if (rc2->x + rc2->width > rc1->x+rc1->width) rc1->width = rc2->x + rc2->width - rc1->x;
	if (rc2->y > rc1->y) {
		rc1->height += rc2->y - rc1->y;
		rc1->y = rc2->y;
	}
	if (rc2->y - rc2->height < rc1->y - rc1->height) rc1->height = rc1->y - rc2->y + rc2->height;
}

static M4INLINE void m4_rect_center(M4Rect *rc, Float w, Float h)
{
	rc->x=-w/2; rc->y=h/2; rc->width=w; rc->height=h;
}

static M4INLINE Bool m4_rect_overlaps(M4Rect rc1, M4Rect rc2)
{
	if (! rc2.height || !rc2.width || !rc1.height || !rc1.width) return 0;
	if (rc2.x+rc2.width<=rc1.x) return 0;
	if (rc2.x>=rc1.x+rc1.width) return 0;
	if (rc2.y-rc2.height>=rc1.y) return 0;
	if (rc2.y<=rc1.y-rc1.height) return 0;
	return 1;
}

static M4INLINE Bool m4_rect_equal(M4Rect rc1, M4Rect rc2) 
{ 
	if ( (rc1.x == rc2.x)  && (rc1.y == rc2.y) && (rc1.width == rc2.width) && (rc1.height == rc2.height) )
		return 1;
	return 0;
}

#define vec_equal(v1, v2) (((v1).x == (v2).x) && ((v1).y == (v2).y) && ((v1).z == (v2).z))
#define vec_diff(res, v1, v2) { (res).x = (v1).x - (v2).x; (res).y = (v1).y - (v2).y; (res).z = (v1).z - (v2).z; }
#define vec_add(res, v1, v2) { (res).x = (v1).x + (v2).x; (res).y = (v1).y + (v2).y; (res).z = (v1).z + (v2).z; }
#define vec_len(v) ((Float) sqrt((v).x*(v).x + (v).y*(v).y + (v).z*(v).z) )
#define vec_lensq(v) ((Float) ((v).x*(v).x + (v).y*(v).y + (v).z*(v).z) )
#define vec_norm(v) { Float __res = vec_len(v); (v).x /= __res; (v).y /= __res; (v).z /= __res; }
#define vec_dot(v1, v2) ( ((v1).x*(v2).x + (v1).y*(v2).y + (v1).z*(v2).z) )
#define vec_scale(res, v, f) { (res).x = (v).x*(f); (res).y = (v).y*(f); (res).z = (v).z*(f); }
/*WARNING: DO NOT DO vec_cross(v1, v1, v2)!!!*/
#define vec_cross(res, v1, v2) { \
	(res).x = (v1).y*(v2).z - (v2).y*(v1).z;	\
	(res).y = (v2).x*(v1).z - (v1).x*(v2).z;		\
	(res).z = (v1).x*(v2).y - (v2).x*(v1).y;		\
}	\

/*update center & radius & is_set flag*/
static M4INLINE void m4_bbox_refresh(M4BBox *b)
{
	SFVec3f v;
	vec_add(v, b->min_edge, b->max_edge);
	vec_scale(b->center, v, 0.5f);
	vec_diff(v, b->max_edge, b->min_edge);
	b->radius = vec_len(v) / 2.0f;
	b->is_set = 1;
}

static M4INLINE void m4_bbox_from_rect(M4BBox *box, M4Rect *rc)
{
	box->min_edge.x = rc->x;
	box->min_edge.y = rc->y - rc->height;
	box->min_edge.z = 0;
	box->max_edge.x = rc->x + rc->width;
	box->max_edge.y = rc->y;
	box->max_edge.z = 0;
	m4_bbox_refresh(box);
}

static M4INLINE void m4_rect_from_bbox(M4Rect *rc, M4BBox *box)
{
	rc->x = box->min_edge.x;
	rc->y = box->max_edge.y;
	rc->width = box->max_edge.x - box->min_edge.x;
	rc->height = box->max_edge.y - box->min_edge.y;
}

static M4INLINE void m4_bbox_grow_point(M4BBox *box, SFVec3f pt)
{
	if (pt.x > box->max_edge.x) box->max_edge.x = pt.x;
	if (pt.y > box->max_edge.y) box->max_edge.y = pt.y;
	if (pt.z > box->max_edge.z) box->max_edge.z = pt.z;
	if (pt.x < box->min_edge.x) box->min_edge.x = pt.x;
	if (pt.y < box->min_edge.y) box->min_edge.y = pt.y;
	if (pt.z < box->min_edge.z) box->min_edge.z = pt.z;
}

static M4INLINE void m4_bbox_union(M4BBox *b1, M4BBox *b2)
{
	if (b2->is_set) {
		if (!b1->is_set) {
			*b1 = *b2;
		} else {
			m4_bbox_grow_point(b1, b2->min_edge);
			m4_bbox_grow_point(b1, b2->max_edge);
			m4_bbox_refresh(b1);
		}
	}
}

static M4INLINE Bool m4_bbox_equal(M4BBox *b1, M4BBox *b2)
{
	return (vec_equal(b1->min_edge, b2->min_edge) && vec_equal(b1->max_edge, b2->max_edge));
}

static M4INLINE Bool m4_bbox_point_inside(M4BBox *box, SFVec3f *p)
{
	return (p->x >= box->min_edge.x && p->x <= box->max_edge.x &&
			p->y >= box->min_edge.y && p->y <= box->max_edge.y &&
			p->z >= box->min_edge.z && p->z <= box->max_edge.z);
}


typedef struct
{
	SFVec3f normal;
	Float d;
} M4Plane;

static M4INLINE void mx_apply_plane(M4Matrix *mx, M4Plane *plane)
{
	SFVec3f pt, end;
	/*get pt*/
	vec_scale(pt, plane->normal, -plane->d);
	vec_add(end, pt, plane->normal);
	mx_apply_vec(mx, &pt);
	mx_apply_vec(mx, &end);
	vec_diff(plane->normal, end, pt);
	vec_norm(plane->normal);
	plane->d = - vec_dot(pt, plane->normal);
}

Bool m4_plane_intersect_line(M4Plane *plane, SFVec3f *linepoint, SFVec3f *linevec, SFVec3f *outPoint);
static M4INLINE Float m4_plane_get_distance(M4Plane *plane, SFVec3f *p)
{
	return vec_dot(*p, plane->normal) + plane->d;
}

/*returns closest point on line from a given point in space*/
SFVec3f m4_closest_point_to_line(SFVec3f line_pt, SFVec3f line_vec, SFVec3f pt);

/*vertices are ordered to respect p vertex indexes (vertex from bbox closer to plane)
and so that n-vertex (vertex from bbox farther from plane) is 7-p_vx_idx*/
static M4INLINE void bbox_get_vertices(SFVec3f bmin, SFVec3f bmax, SFVec3f *vecs)
{
	vecs[0].x = vecs[1].x = vecs[2].x = vecs[3].x = bmax.x;
	vecs[4].x = vecs[5].x = vecs[6].x = vecs[7].x = bmin.x;
	vecs[0].y = vecs[1].y = vecs[4].y = vecs[5].y = bmax.y;
	vecs[2].y = vecs[3].y = vecs[6].y = vecs[7].y = bmin.y;
	vecs[0].z = vecs[2].z = vecs[4].z = vecs[6].z = bmax.z;
	vecs[1].z = vecs[3].z = vecs[5].z = vecs[7].z = bmin.z;
}

/*return p-vertex index (vertex from bbox closer to plane) - index range from 0 to 8*/
static M4INLINE u32 m4_plane_get_p_vertex_idx(M4Plane *p)
{
	if (p->normal.x>=0) {
		if (p->normal.y>=0) return (p->normal.z>=0) ? 0 : 1;
		return (p->normal.z>=0) ? 2 : 3;
	} else {
		if (p->normal.y>=0) return (p->normal.z>=0) ? 4 : 5;
		return (p->normal.z>=0) ? 6 : 7;
	}
}

/*classify box/plane position. Retturns one of the following*/
enum 
{	
	M4_BOX_FRONT, /*box is in front of the plane*/
	M4_BOX_INTER, /*box intersects the plane*/
	M4_BOX_BACK /*box is back of the plane*/
};

static M4INLINE u32 m4_box_plane_relation(M4BBox *box, M4Plane *p)
{
	SFVec3f nearv, farv;
	nearv = box->max_edge;
	farv = box->min_edge;
	if (p->normal.x > 0) {
		nearv.x = box->min_edge.x;
		farv.x = box->max_edge.x;
	}
	if (p->normal.y > 0) {
		nearv.y = box->min_edge.y;
		farv.y = box->max_edge.y;
	}
	if (p->normal.z > 0) {
		nearv.z = box->min_edge.z;
		farv.z = box->max_edge.z;
	}
	if (vec_dot(p->normal, nearv) + p->d > 0) return M4_BOX_FRONT;
	if (vec_dot(p->normal, farv) + p->d > 0) return M4_BOX_INTER;
	return M4_BOX_BACK;
}


typedef struct
{
	SFVec3f orig;
	SFVec3f dir;
} M4Ray;

static M4INLINE M4Ray m4_ray(SFVec3f *start, SFVec3f *end)
{
	M4Ray r;
	r.orig = *start;
	vec_diff(r.dir, *end, *start);
	vec_norm(r.dir);
	return r;
}

static M4INLINE void mx_apply_ray(M4Matrix *mx, M4Ray *r)
{
	vec_add(r->dir, r->orig, r->dir);
	mx_apply_vec(mx, &r->orig);
	mx_apply_vec(mx, &r->dir);
	vec_diff(r->dir, r->dir, r->orig);
	vec_norm(r->dir);
}

/*retuns 1 if intersection and stores value in outPoint*/
Bool m4_ray_hit_box(M4Ray *ray, SFVec3f min_edge, SFVec3f max_edge, SFVec3f *outPoint);
/*retuns 1 if intersection and stores value in outPoint - if @center is NULL, assumes center is origin(0, 0, 0)*/
Bool m4_ray_hit_sphere(M4Ray *ray, SFVec3f *center, Float radius, SFVec3f *outPoint);
/*retuns 1 if intersection with triangle and stores distance on ray value */
Bool m4_ray_hit_triangle(M4Ray *ray, SFVec3f *v0, SFVec3f *v1, SFVec3f *v2, Float *dist);
/*same as above and performs backface cull (solid meshes)*/
Bool m4_ray_hit_triangle_backcull(M4Ray *ray, SFVec3f *v0, SFVec3f *v1, SFVec3f *v2, Float *dist);


/*quaternions tools*/
#define quat_norm(v) { \
	Float __mag = (Float) sqrt((v).q*(v).q + (v).x*(v).x + (v).y*(v).y + (v).z*(v).z);	\
	(v).x /= __mag; (v).y /= __mag; (v).z /= __mag; (v).q /= __mag;	\
	}	\

#define quat_len(v) ((Float) sqrt((v).q*(v).q + (v).x*(v).x + (v).y*(v).y + (v).z*(v).z))

SFRotation quat_to_rotation(SFVec4f *quat);
SFVec4f quat_from_matrix(M4Matrix *mx);
SFVec4f quat_from_rotation(SFRotation rot);
SFVec4f quat_get_inv(SFVec4f *quat);
SFVec4f quat_multiply(SFVec4f *q1, SFVec4f *q2);
SFVec3f quat_rotate(SFVec4f *quat, SFVec3f *vec);
SFVec4f quat_from_axis_cos(SFVec3f axis, Float cos_a);
SFVec4f quat_slerp(SFVec4f q1, SFVec4f q2, Float frac);


#ifdef __cplusplus
}
#endif

#endif
