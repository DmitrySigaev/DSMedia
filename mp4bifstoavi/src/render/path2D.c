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
 *
 *	C code rewrite of the charcoal lib (mupdf)
 *	written by Tor Andersson <tor.andersson@dsek.lth.se> published under the GPL license
 *	and libart outliner (Raph Levien / libart / art_svp_vpath_stroke)
 *		
 */


#include <intern/m4_render_tools.h>

#define M4PATH()	M4Path *path = (M4Path *)_this;

M4SubPath *new_subpath2D()
{
	M4SubPath *subpath;

	SAFEALLOC(subpath, sizeof (M4SubPath));
	if (subpath == NULL) return NULL;
	subpath->pointmax = 16;
	subpath->point = malloc(sizeof (M4Point2D) * subpath->pointmax);
	if (subpath->point == NULL) {
		free(subpath);
		return NULL;
	}
	return subpath;
}

void subpath2D_free(M4SubPath *subpath)
{
	free(subpath->point);
	free(subpath);
}

void subpath2D_reset(M4SubPath *subpath)
{
	subpath->closed = 0;
	subpath->pointlen = 0;
}

M4Err subpath2D_addpoint(M4SubPath *subpath, Float x, Float y)
{
	M4Point2D *newpoints;
	s32 newlen;

	if (subpath->pointlen + 1 >= subpath->pointmax) {
		newlen = subpath->pointmax * 2;
		newpoints = realloc(subpath->point, sizeof (M4Point2D) * newlen);
		if (newpoints == NULL) return M4OutOfMem;
		subpath->pointmax = newlen;
		subpath->point = newpoints;
	}
	subpath->point[subpath->pointlen].x = x;
	subpath->point[subpath->pointlen].y = y;
	subpath->pointlen ++;
	return M4OK;
}

void subpath2D_close(M4SubPath *subpath)
{
	subpath->closed = 1;
}

M4Err m4_path_newsubpath(M4Path *path)
{
	M4SubPath **newsubpaths;
	u32 newlen;
	u32 i;
	
	if (path->subpathlen + 1 >= path->subpathmax) {
		newlen = path->subpathmax * 2;
		newsubpaths = realloc(path->subpath, sizeof (M4SubPath*) * newlen);
		if (newsubpaths == NULL) return M4OutOfMem;
		path->subpathmax = newlen;
		path->subpath = newsubpaths;
		for (i = path->subpathlen; i < path->subpathmax; i++) {
			path->subpath[i] = NULL;
		}
	}
	
	if (path->subpath[path->subpathlen] == NULL) {
		path->subpath[path->subpathlen] = new_subpath2D();
		if (path->subpath[path->subpathlen] == NULL) return M4OutOfMem;
	}
	path->subpathlen ++;
	return M4OK;
}


M4Path *m4_new_path()
{	
	M4Path *path;
	SAFEALLOC(path , sizeof(M4Path));
	if (path == NULL) return NULL;
	path->subpathmax = 8;
	SAFEALLOC(path->subpath , sizeof (M4SubPath*) * path->subpathmax);
	if (path->subpath == NULL) {
		free(path);
		return NULL;
	}
	path->min_x = FLT_MAX;
	path->min_y = FLT_MAX;
	path->max_x = -1 * FLT_MAX;
	path->max_y = -1 * FLT_MAX;
	path->fineness = 0.5;
	path->resolution = M4_DEFAULT_RESOLUTION;
	path->fill_mode = M4PathFillOddEven;
	return path;
}

void m4_path_delete(M4Path *_this)
{
	u32 i;
	M4PATH();
	for (i = 0; i < path->subpathmax; i++) {
		if (path->subpath[i] != NULL) subpath2D_free(path->subpath[i]);
	}
	free(path->subpath);
	free(path);
}

M4Err m4_path_reset(M4Path *_this)
{
	M4PATH();
	path->subpathlen = 0;
	path->min_x = FLT_MAX;
	path->min_y = FLT_MAX;
	path->max_x = -1 * FLT_MAX;
	path->max_y = -1 * FLT_MAX;
	return M4OK;
}

void m4_path_refresh_bounds(M4Path *_this)
{
	u32 i, j;
	for (i=0; i<_this->subpathlen; i++) {
		for (j=0; j<_this->subpath[i]->pointlen; j++) {
			M4Point2D *pt = &_this->subpath[i]->point[j];
			if (pt->x>_this->max_x) _this->max_x = pt->x;
			if (pt->x<_this->min_x) _this->min_x = pt->x;
			if (pt->y>_this->max_y) _this->max_y = pt->y;
			if (pt->y<_this->min_y) _this->min_y = pt->y;
		}
	}
}

M4Path *m4_path_clone(M4Path *_this)
{	
	u32 i;
	M4Path *tmp;
	M4PATH();
	SAFEALLOC(tmp, sizeof(M4Path));
	if (!tmp) return tmp;

	memcpy(tmp, path, sizeof(M4Path));
	tmp->subpath = malloc(sizeof(M4SubPath*) * path->subpathmax);
	for (i=0; i<path->subpathlen; i++) {
		SAFEALLOC(tmp->subpath[i] , sizeof(M4SubPath));
		memcpy(tmp->subpath, path->subpath, sizeof(M4SubPath));
		tmp->subpath[i]->point = malloc(sizeof(M4Point2D) * tmp->subpath[i]->pointmax);
		memcpy(tmp->subpath[i]->point, path->subpath[i]->point, sizeof(M4Point2D) * tmp->subpath[i]->pointlen);
	}
	return tmp;
}

M4Err m4_path_set_resolution(M4Path *_this, u32 resol, u32 *prev_res)
{
	M4PATH();
	if (prev_res) *prev_res = path->resolution;

#ifdef _WIN32_WCE
	path->resolution = M4_DEFAULT_RESOLUTION;
#else
	path->resolution = resol;
#endif
	return M4OK;
}


M4Err m4_path_add_line_to(M4Path *_this, Float x, Float y)
{
	M4Err e;
	M4PATH();

	if ((path->cx == x) && (path->cy == y)) return M4OK;

	e = subpath2D_addpoint(path->subpath[path->subpathlen-1], x, y);
	if (e) return e;
	path->cx = x;
	path->cy = y;
	PATH_CHECKBOUNDS(path, x, y);
	return M4OK;
}

M4Err m4_path_add_move_to(M4Path *_this, Float x, Float y)
{
	M4Err e;
	M4PATH();

	e = m4_path_newsubpath(path);
	if (e) return e;
	subpath2D_reset(path->subpath[path->subpathlen-1]);
	e = subpath2D_addpoint(path->subpath[path->subpathlen-1], x, y);
	if (e) return e;
	path->bx = x;
	path->by = y;
	path->cx = x;
	path->cy = y;
	PATH_CHECKBOUNDS(path, x, y);
	return M4OK;
}

M4Err m4_path_close(M4Path *_this)
{
	M4Err e;
	M4PATH();
	if (path->cx != path->bx || path->cy != path->by) {
		e = subpath2D_addpoint(path->subpath[path->subpathlen-1], path->bx, path->by);
		if (e) return e;
	}
	subpath2D_close(path->subpath[path->subpathlen-1]);
	path->cx = path->bx;
	path->cy = path->by;
	return M4OK;
}

M4Err m4_path_add_ellipse(M4Path *_this, Float a_axis, Float b_axis)
{
	M4Err e;
	Float _vx, _vy;
	Float step, cur, end, offset;
	u32 i;
	M4PATH();

	end = 2*M_PI;
	step = end / (path->resolution);
	a_axis /= 2;
	b_axis /= 2;
	offset = M_PI / 2;

	i=0;
	e = m4_path_add_move_to(path, 0, b_axis);
	if (e) return e;
	for (cur=step; cur<end; cur += step) {
		_vx = (Float) (a_axis * cos(cur + offset) );
		_vy = (Float) (b_axis * sin(cur + offset) );
		e = m4_path_add_line_to(_this, _vx, _vy);
		if (e) return e;
		i++;
	}
	e = m4_path_add_line_to(_this, 0, b_axis);
	if (e) return e;
	return m4_path_close(_this);
}

M4Err m4_path_add_rectangle(M4Path *_this, Float x, Float y, Float w, Float h)
{
	M4Err e;
	e = m4_path_add_move_to(_this, x - w/2, y - h/2);
	if (e) return e;
	e = m4_path_add_line_to(_this, x + w/2, y - h/2);
	if (e) return e;
	e = m4_path_add_line_to(_this, x + w/2, y + h/2);
	if (e) return e;
	e = m4_path_add_line_to(_this, x - w/2, y + h/2);
	if (e) return e;
	return m4_path_close(_this);
}

M4Err m4_path_addsubpath(M4Path *path, M4SubPath *src, Bool join)
{
	M4Err e;
	M4SubPath *dst;
	u32 i;
	
	if (!join) {
		e = m4_path_newsubpath(path);
		if (e) return e;
	}
	
	dst = path->subpath[path->subpathlen-1];
	subpath2D_reset(dst);
	
	for (i=0; i<src->pointlen; i++) {
		e = subpath2D_addpoint(dst, src->point[i].x, src->point[i].y);
		if (e) return e;
		PATH_CHECKBOUNDS(path, src->point[i].x, src->point[i].y);
	}
	if (src->closed) subpath2D_close(dst);
	return M4OK;
}

M4Err m4_path_add_path(M4Path *_this, M4Path *_with_path, u32 joinPath)
{
	u32 i;
	M4Err e;
	M4Path *with_path;
	M4PATH();
	with_path = (M4Path *) _with_path;

	if (!with_path || !path || !with_path->subpathlen) return M4OK;

	for (i=0; i<with_path->subpathlen; i++) {
		e = m4_path_addsubpath(path, with_path->subpath[i], (joinPath && i==0) ? 1 : 0);
		if (e) return e;
	}
	return M4OK;
}

M4Err m4_path_get_bounds(M4Path *_this, M4Rect *rc)
{
	M4Rect rec;
	M4PATH();
	if (!rc) return M4BadParam;

	if (!path->subpathlen) {
		rc->x = rc->y = rc->width = rc->height = 0;
		return M4OK;
	}
	rec.x = path->min_x;
	rec.y = path->max_y;
	rec.width = path->max_x - path->min_x;
	rec.height = path->max_y - path->min_y;

	/*take care of straight line path by adding a default width if height and vice-versa*/
	if (rec.height && !rec.width) {
		rec.width = 2;
		rec.x -= 1;
	}
	else if (!rec.height && rec.width) {
		rec.height = 2;
		rec.y += 1;
	}
	*rc = rec;
	return M4OK;
}

M4Path *m4_path_get_outline(M4Path *_this, M4PenSettings pen)
{
	M4Err e;
	M4Path *outline;
	M4Path *dash;
	Float w, dash_off;
	M4Err m4_path_dash(M4Path *path, M4Path *result, M4PenSettings *pen);
	M4Err m4_path_outline(M4Path *path, M4Path *result, M4PenSettings *pen);
	M4PATH();

	if (!_this || !path->subpathlen) return NULL;

	outline = m4_new_path();
	if (!outline) return NULL;

	w = pen.width;
	dash_off = pen.dash_offset;
	pen.dash_offset *= pen.width;

	if (pen.dash != M4StrikePlain) {
		dash = m4_new_path();
		if (!dash) {
			m4_path_delete(outline);
			return NULL;
		}
		e = m4_path_dash(_this, dash, &pen);
		if (!e) e = m4_path_outline(dash, outline, &pen);
		m4_path_delete(dash);
	} else {
		e = m4_path_outline(_this, outline, &pen);
	}
	if (e) {
		m4_path_delete(outline);
		return NULL;
	}

	pen.width = w;
	pen.dash_offset = dash_off;
	outline->fill_mode = M4PathFillZeroNonZero;
	m4_path_refresh_bounds(outline);
	return outline;
}


/*generic N-bezier*/
static void NBezier(M4Point2D *points, s32 n, double mu, M4Point2D *out)
{
	s32 k,kn,nn,nkn;
	double blend,muk,munk;
	out->x = out->y = 0;

	muk = 1;
	munk = pow(1-mu,(double)n);
	for (k=0;k<=n;k++) {
		nn = n;
		kn = k;
		nkn = n - k;
		blend = muk * munk;
		muk *= mu;
		munk /= (1-mu);
		while (nn >= 1) {
			blend *= nn;
			nn--;
			if (kn > 1) {
				blend /= (double)kn;
				kn--;
			}
			if (nkn > 1) {
				blend /= (double)nkn;
				nkn--;
			}
		}
		out->x += points[k].x * (Float) blend;
		out->y += points[k].y * (Float) blend;
	}
}

static void m4_add_n_bezier(M4Path *path, M4Point2D *pts, u32 nbPoints)
{
	Double mu;
	u32 numPoints, i;
	M4Point2D start;
	M4Point2D end;
	numPoints = (u32) (path->resolution * path->fineness);
	mu = 0.0;
	if (numPoints) mu = 1/(Double)numPoints;
	start = pts[0];
	for (i=1; i<numPoints; i++) {
		NBezier(pts, nbPoints - 1, i*mu, &end);
		m4_path_add_line_to(path, end.x, end.y);
		start = end;
	}
	m4_path_add_line_to(path, pts[nbPoints-1].x, pts[nbPoints-1].y);
}

M4Err m4_path_add_bezier(M4Path *_this, M4Point2D *pts, u32 nbPoints)
{
	M4Point2D *newpts;
	M4PATH();
	newpts = (M4Point2D*) malloc(sizeof(M4Point2D) * (nbPoints+1));
	newpts[0].x = path->cx;
	newpts[0].y = path->cy;
	memcpy(&newpts[1], pts, sizeof(M4Point2D) * nbPoints);
	m4_add_n_bezier(path, newpts, nbPoints + 1);
	free(newpts);
	return M4OK;
}

//#ifndef USE_SUBDIV
#if 0

M4Err m4_path_add_quadratic_to(M4Path *_this, Float c_x, Float c_y, Float x, Float y)
{
	M4Point2D ctr[3];
	M4PATH();
	ctr[0].x = path->cx;
	ctr[0].y = path->cy;
	ctr[1].x = c_x;
	ctr[1].y = c_y;
	ctr[2].x = x;
	ctr[2].y = y;

	m4_add_n_bezier(path, ctr, 3);
	return M4OK;
}

M4Err m4_path_add_cubic_to(M4Path *_this, Float c1_x, Float c1_y, Float c2_x, Float c2_y, Float x, Float y)
{
	M4Point2D ctr[4];
	M4PATH();

	ctr[0].x = path->cx;
	ctr[0].y = path->cy;
	ctr[1].x = c1_x;
	ctr[1].y = c1_y;
	ctr[2].x = c2_x;
	ctr[2].y = c2_y;
	ctr[3].x = x;
	ctr[3].y = y;

	m4_add_n_bezier(path, ctr, 4);
	return M4OK;
}

#else

M4Err m4_subpath_addcurve(M4SubPath *path,
	Float x0, Float y0, Float x1, Float y1, Float x2, Float y2, Float x3, Float y3,
	M4Path *par)
{
	Float x3_0, y3_0;
	Float z3_0_dot;
	Float z1_dot, z2_dot;
	Float z1_perp, z2_perp;
	Float max_perp_sq;

	Float x_m, y_m;
	Float xa1, ya1;
	Float xa2, ya2;
	Float xb1, yb1;
	Float xb2, yb2;
	
	s32 err;

	x3_0 = x3 - x0;
	y3_0 = y3 - y0;

	/* z3_0_dot is dist z0-z3 squared */
	z3_0_dot = x3_0 * x3_0 + y3_0 * y3_0;

	/* todo: this test is far from satisfactory. */
	if (z3_0_dot < 0.001)
		goto nosubdivide;

	/* perp is distance from line, multiplied by dist z0-z3 */
	max_perp_sq = par->fineness * par->fineness * z3_0_dot;

	z1_perp = (y1 - y0) * x3_0 - (x1 - x0) * y3_0;
	if (z1_perp * z1_perp > max_perp_sq)
		goto subdivide;

	z2_perp = (y3 - y2) * x3_0 - (x3 - x2) * y3_0;
	if (z2_perp * z2_perp > max_perp_sq)
		goto subdivide;

	z1_dot = (x1 - x0) * x3_0 + (y1 - y0) * y3_0;
	if (z1_dot < 0 && z1_dot * z1_dot > max_perp_sq)
		goto subdivide;

	z2_dot = (x3 - x2) * x3_0 + (y3 - y2) * y3_0;
	if (z2_dot < 0 && z2_dot * z2_dot > max_perp_sq)
		goto subdivide;

	if (z1_dot + z1_dot > z3_0_dot)
		goto subdivide;

	if (z2_dot + z2_dot > z3_0_dot)
		goto subdivide;

nosubdivide:
	/* don't subdivide */
	err = subpath2D_addpoint(path, x3, y3);
	PATH_CHECKBOUNDS(par, x3, y3);
	if (err) return err;
	return 0;

subdivide:
	xa1 = (x0 + x1) * 0.5f;
	ya1 = (y0 + y1) * 0.5f;
	xa2 = (x0 + 2 * x1 + x2) * 0.25f;
	ya2 = (y0 + 2 * y1 + y2) * 0.25f;
	xb1 = (x1 + 2 * x2 + x3) * 0.25f;
	yb1 = (y1 + 2 * y2 + y3) * 0.25f;
	xb2 = (x2 + x3) * 0.5f;
	yb2 = (y2 + y3) * 0.5f;
	x_m = (xa2 + xb1) * 0.5f;
	y_m = (ya2 + yb1) * 0.5f;

	err = m4_subpath_addcurve(path,
			x0, y0, xa1, ya1, xa2, ya2, x_m, y_m, par);
	if (err) return err;
	err = m4_subpath_addcurve(path,
			x_m, y_m, xb1, yb1, xb2, yb2, x3, y3, par);
	if (err) return err;

	return 0;
}


M4Err m4_path_add_cubic_to(M4Path *_this, Float c1_x, Float c1_y, Float c2_x, Float c2_y, Float x, Float y)
{
	M4Err e;
	Float fine;
	M4PATH();
	
	fine = path->fineness;
	/*don't allow 0.0f, this would generate way too many points for nothing, which can kill tesselators
	later on*/
	path->fineness = MAX(1.0f - fine, 0.01f);
	e = m4_subpath_addcurve(path->subpath[path->subpathlen-1],
			path->cx, path->cy, c1_x, c1_y, c2_x, c2_y, x, y, path);
	if (e) return e;
	
	path->fineness = fine;
	e = subpath2D_addpoint(path->subpath[path->subpathlen-1], x, y);
	if (e) return e;
	
	path->cx = x;
	path->cy = y;
	PATH_CHECKBOUNDS(path, x, y);
	return M4OK;
}

M4Err m4_path_add_quadratic_to(M4Path *_this, Float c_x, Float c_y, Float x, Float y)
{
	M4Point2D c1, c2;
	M4PATH();

	c1.x = path->cx + 2*(c_x - path->cx)/3;
	c1.y = path->cy + 2*(c_y - path->cy)/3;

	c2.x = c1.x + (x-path->cx) / 3;
	c2.y = c1.y + (y-path->cy) / 3;

	return m4_path_add_cubic_to(_this, c1.x, c1.y, c2.x, c2.y, x, y);
}

#endif


M4Err m4_path_add_arc_to(M4Path *_this, Float end_x, Float end_y, Float fa_x, Float fa_y, Float fb_x, Float fb_y, Bool cw)
{
	M4Matrix2D mat, inv;
	Float angle, start_angle, end_angle, sweep, axis_w, axis_h, tmp, cx, cy, _vx, _vy, start_x, start_y;
	u32 i, num_steps;

	start_x = _this->cx;
	start_y = _this->cy;

	cx = (fb_x + fa_x)/2;
	cy = (fb_y + fa_y)/2;

	angle = (Float) atan2(fb_y - fa_y, fb_x-fa_x);
	mx2d_init(mat);
	mx2d_add_rotation(&mat, 0, 0, angle);
	mx2d_add_translation(&mat, cx, cy);

	mx2d_copy(inv, mat);
	mx2d_inverse(&inv);
	mx2d_apply_coords(&inv, &start_x, &start_y);
	mx2d_apply_coords(&inv, &end_x, &end_y);
	mx2d_apply_coords(&inv, &fa_x, &fa_y);
	mx2d_apply_coords(&inv, &fb_x, &fb_y);

	//start angle and end angle
	start_angle = (Float) atan2(start_y, start_x);
	end_angle = (Float) atan2(end_y, end_x);
	tmp = (start_x - fa_x) * (start_x - fa_x) + (start_y - fa_y) * (start_y - fa_y); 
	axis_w = (Float) sqrt(tmp);
	tmp = (start_x - fb_x) * (start_x - fb_x) + (start_y - fb_y) * (start_y - fb_y); 
	axis_w += (Float) sqrt(tmp);
	axis_w /= 2;
	axis_h = (Float) sqrt(axis_w*axis_w - fa_x*fa_x);
	sweep = end_angle - start_angle;

	if (cw) {
		if (sweep>0) sweep -= 2*M_PI;
	} else {
		if (sweep<0) sweep += 2*M_PI;
	}


	num_steps = _this->resolution/2;
	for (i=0; i<=num_steps; i++) {
		angle = start_angle + sweep*i/num_steps;
		_vx = (Float) (axis_w * cos(angle) );
		_vy = (Float) (axis_h * sin(angle) );
		/*re-invert*/
		mx2d_apply_coords(&mat, &_vx, &_vy);
		m4_path_add_line_to(_this, _vx, _vy);
	}
	return M4OK;
}

M4Err m4_path_add_arc(M4Path *_this, Float radius, Float start_angle, Float end_angle, u32 close_type)
{
	M4Err e;
	Float _vx, _vy;
	Float step, cur;
	u32 i, do_run;
	M4PATH();

	step = (end_angle - start_angle) / (path->resolution);
	radius *= 2;

	/*pie*/
	i=0;
	if (close_type==2) {
		m4_path_add_move_to(_this, 0, 0);
		i=1;
	}
	do_run = 1;
	cur=start_angle;
	while (do_run) {
		if (cur>=end_angle) {
			do_run = 0;
			cur = end_angle;
		}
		_vx = (Float) (radius * cos(cur) );
		_vy = (Float) (radius * sin(cur) );
		if (!i) {
			e = m4_path_add_move_to(_this, _vx, _vy);
			i = 1;
		} else {
			e = m4_path_add_line_to(_this, _vx, _vy);
		}
		if (e) return e;
		cur+=step;
	}
	if (close_type) e = m4_path_close(_this);
	return e;
}

/*sets path fineness*/
void m4_path_set_fineness(LPM4PATH _this, Float fineness)
{
	if (fineness < 0.0) fineness = 0.0;
	if (fineness > 1.0) fineness = 1.0;
	if (_this) _this->fineness = fineness;
}
/*gets path fineness*/
Float m4_path_get_fineness(LPM4PATH _this)
{
	return _this ? _this->fineness : 0;
}
/*sets path fill mode*/
void m4_path_set_fill_mode(LPM4PATH _this, u32 fillMode)
{
	if (_this) _this->fill_mode = fillMode;
}
/*returns path fill mode*/
u32 m4_path_get_fill_mode(LPM4PATH _this)
{
	return _this ? _this->fill_mode : 0;
}
/*returns number of subpath (a subpath is a set of connected lines)*/
u32 m4_path_get_subpath_count(LPM4PATH _this)
{
	return _this ? _this->subpathlen : 0;
}
/*returns number of points in subpath*/
u32 m4_subpath_get_points_count(LPM4PATH _this, u32 sub_idx)
{
	if (!_this || (_this->subpathlen<=sub_idx)) return 0;
	return _this->subpath[sub_idx]->pointlen;
}
/*returns points in subpath - do NOT touch the points*/
M4Point2D *m4_subpath_get_points(LPM4PATH _this, u32 sub_idx)
{
	if (!_this || (_this->subpathlen<=sub_idx)) return NULL;
	return _this->subpath[sub_idx]->point;
}



#define isLeft(P0, P1, P2) \
	( (P1.x - P0.x) * (P2.y - P0.y) - (P2.x - P0.x) * (P1.y - P0.y) )


Bool m4_path_point_over(LPM4PATH _this, Float x, Float y)
{
	u32 i, j;
	s32 wn;
	M4Point2D start, s, e, pt;
	M4Rect rc;
	M4PATH();

	pt.x = x;
	pt.y = y;
	rc.x = path->min_x;
	rc.y = path->min_y;
	rc.width = path->max_x - rc.x;
	rc.height = path->max_y - rc.y;

	if (pt.x < rc.x) return 0;
	if (pt.y < rc.y) return 0;
	if (pt.x > rc.x + rc.width) return 0;
	if (pt.y > rc.y + rc.height) return 0;

	wn = 0;
	for (i=0; i<path->subpathlen; i++) {
		if (!path->subpath[i]->pointlen) continue;
		start = path->subpath[i]->point[0];
		s = start;
		for (j=1; j<path->subpath[i]->pointlen; j++) {
			e = path->subpath[i]->point[j];
			if (s.y<=pt.y) {
				if (e.y>pt.y) {
					if (isLeft(s, e, pt) > 0) wn++;
				}
			}
			else if (e.y<=pt.y) {
				if (isLeft(s, e, pt) < 0) wn--;
			}
			s = e;
		}
		/*close path if needed*/
		if ((start.x != s.x) && (start.y != s.y)) {
			e = start;
			if (s.x<=pt.x) {
				if (e.y>pt.y) {
					if (isLeft(s, e, pt) > 0) wn++;
				}
			}
			else if (e.y<=pt.y) {
				if (isLeft(s, e, pt) < 0) wn--;
			}
		}
	}

	if (path->fill_mode == M4PathFillZeroNonZero) return wn ? 1 : 0;
	return wn%2 ? 1 : 0;
}


M4PathIterator *m4_path_new_iterator(M4Path *_this)
{
	M4PathIterator *it;
	u32 i, j, nb;
	M4Point2D start, end;
	M4PATH();
	
	SAFEALLOC(it, sizeof(M4PathIterator));

	nb = 0;
	for (i=0; i<path->subpathlen; i++) {
		nb += path->subpath[i]->pointlen;
	}
	it->seg = malloc(sizeof(IterInfo) * nb);
	it->num_seg = 0;
	it->length = 0;
	for (i=0; i<path->subpathlen; i++) {
		if (!path->subpath[i]->pointlen) continue;
		start = path->subpath[i]->point[0];
		for (j=1; j<path->subpath[i]->pointlen; j++) {
			end = path->subpath[i]->point[j];

			it->seg[it->num_seg].start_x = start.x;
			it->seg[it->num_seg].start_y = start.y;
			it->seg[it->num_seg].dx = end.x - start.x;
			it->seg[it->num_seg].dy = end.y - start.y;
			it->seg[it->num_seg].len = (Float) sqrt(it->seg[it->num_seg].dx*it->seg[it->num_seg].dx + it->seg[it->num_seg].dy*it->seg[it->num_seg].dy);
			it->length += it->seg[it->num_seg].len;
			start = end;
			if (it->seg[it->num_seg].len) it->num_seg++;
		}
	}
	return it;
}

Bool m4_pathiterator_get_transform_at_offset(M4PathIterator *path, Float offset, Bool follow_tangent, M4Matrix2D *mat, Bool smooth_edges, Float length_after_point)
{
	M4Matrix2D final, rot;
	Bool tang = 0;
	Float res, angle, angleNext;
	u32 i;
	Float curLen = 0;

	for (i=0; i<path->num_seg; i++) {
		if (curLen + path->seg[i].len >= offset) goto found;
		curLen += path->seg[i].len;
	}
	if (!follow_tangent) return 0;
	tang = 1;
	i--;

found:
	mx2d_init(final);

	res = (offset - curLen) / path->seg[i].len;
	if (tang) res += 1;

	/*move to current point*/
	mx2d_add_translation(&final, path->seg[i].start_x + path->seg[i].dx*res, path->seg[i].start_y + path->seg[i].dy*res);

	if (!path->seg[i].dx) {
		angle = M_PI/2;
	} else {
		angle = (Float) acos(path->seg[i].dx / path->seg[i].len);
	}
	if (path->seg[i].dy<0) angle *= -1;

	if (smooth_edges) {
		if (offset + length_after_point > curLen + path->seg[i].len) {
			float ratio = curLen + path->seg[i].len-offset;
			ratio /= length_after_point;
			if (i < path->num_seg - 1) {
				if (!path->seg[i+1].dx) {
					angleNext = M_PI/2;
				} else {
					angleNext = (Float) acos(path->seg[i+1].dx / path->seg[i+1].len);
				}
				if (path->seg[i+1].dy<0) angleNext *= -1;

				if (angle<0 && angleNext>0) {
					angle = (1-ratio) * angleNext - ratio*angle;
				} else {
					angle = ratio*angle + (1-ratio) * angleNext;
				}
			}
		}
	}
	/*handle res=0 case for rotation (point on line join)*/
	else if (res==1) {
		if (i < path->num_seg - 1) {
			if (!path->seg[i+1].dx) {
				angleNext = M_PI/2;
			} else {
				angleNext = (Float) acos(path->seg[i+1].dx / path->seg[i+1].len);
			}
			if (path->seg[i+1].dy<0) angleNext *= -1;
			angle = ( angle + angleNext) / 2;
		}
	}

	mx2d_init(rot);
	mx2d_add_rotation(&rot, 0, 0, angle);
	mx2d_add_matrix(mat, &rot);
	mx2d_add_matrix(mat, &final);
	return 1;
}

void m4_path_delete_iterator(M4PathIterator *it)
{
	if (it->seg) free(it->seg);
	free(it);
}
