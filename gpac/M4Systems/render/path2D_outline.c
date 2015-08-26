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

#include <gpac/intern/m4_render_tools.h>

#ifndef M_SQRT2
#define M_SQRT2		1.4142135623730950
#endif

#ifndef EPSILON
#define EPSILON		1e-6
#endif

#ifndef EPSILON_2
#define EPSILON_2	1e-12
#endif

/*
Render an arc segment starting at (xc + x0, yc + y0) to (xc + x1, yc + y1),
centered at (xc, yc), and with given radius.
Both x0^2 + y0^2 and x1^2 + y1^2 should be equal to radius^2.
A positive value of radius means curve to the left, negative means curve to the right.
*/
static M4Err m4_path_stroke_arc(M4SubPath *result,
	Float xc, Float yc, Float x0, Float y0, Float x1, Float y1,
	Float radius, Float flatness)
{
	Float theta;
	Float th0, th1;
	Float aradius;
	u32 npts;
	u32 i;
	M4Err e;
	
	aradius = (Float) fabs(radius);
	theta = 2 * (Float) M_SQRT2 * (Float) sqrt(flatness / aradius);
	th0 = (Float) atan2(y0, x0);
	th1 = (Float) atan2 (y1, x1);
	
	if (radius > 0) {
		if (th0 < th1) th0 += (Float) M_PI * 2;
		npts = (u32) ceil((th0 - th1) / theta);
	}
	else {
		if (th1 < th0) th1 += (Float) M_PI * 2;
		npts = (u32) ceil((th1 - th0) / theta);
	}
	
	e = subpath2D_addpoint(result, xc + x0, yc + y0);
	if (e) return e;
	for (i = 1; i < npts; i++) {
		theta = th0 + (th1 - th0) * i / npts;
		e = subpath2D_addpoint(result,
				xc + (Float) cos(theta) * aradius,
				yc + (Float) sin(theta) * aradius);
		if (e) return e;
	}
	return subpath2D_addpoint(result, xc + x1, yc + y1);
}

/*
Assume that forw and rev are at point i0.
Bring them to i1, joining with the vector i1 - i2.
Forw is to the line's right and rev is to its left.
Precondition: no zero-length vector, or else divide by zero.
*/
static M4Err m4_path_render_seg(M4SubPath *forw, M4SubPath *rev,
	M4Point2D *vpath, s32 i0, s32 i1, s32 i2,
	u32 lineJoin, Float lineWidth, Float miter, Float flatness)
{
	Float dx0, dy0;
	Float dx1, dy1;
	Float dlx0, dly0;
	Float dlx1, dly1;
	Float dmx, dmy;
	Float dmr2;
	Float scale;
	Float cross;
	Float sqwidth;
	M4Err e;
	
	/* Vectors of lines from i0 to i1 and i1 to i2 */
	dx0 = vpath[i1].x - vpath[i0].x;
	dy0 = vpath[i1].y - vpath[i0].y;

	dx1 = vpath[i2].x - vpath[i1].x;
	dy1 = vpath[i2].y - vpath[i1].y;
	
	/* Set dl[xy]0 to vector from i0 to i1,
		rotated ccw 90 degrees scaled to length of linewidth */
	scale = lineWidth / (Float) sqrt(dx0 * dx0 + dy0 * dy0);
	dlx0 = dy0 * scale;
	dly0 = -dx0 * scale;
	
	/* Set dl[xy]1 to vector from i1 to i2,
		rotated ccw 90 degrees scaled to length of linewidth */
	scale = lineWidth / (Float) sqrt(dx1 * dx1 + dy1 * dy1);
	dlx1 = dy1 * scale;
	dly1 = -dx1 * scale;

	/* forw's last point is expected to be colinear along d[xy]0 to point i0 - dl[xy]0,
		and rev with i0 + dl[xy]0 */
	
	/* positive for positive area (ie left turn) */
	cross = dx1 * dy0 - dx0 * dy1;
	dmx = (dlx0 + dlx1) * 0.5f;
	dmy = (dly0 + dly1) * 0.5f;
	dmr2 = dmx * dmx + dmy * dmy;
	
	sqwidth = lineWidth * lineWidth;
	if (lineJoin == M4LineJoinMiter) {
		if (dmr2 * miter * miter < sqwidth) {
			lineJoin = M4LineJoinBevel;
		}
	}
	
	/* the case when dmr2 is zero or very small bothers me (raph) 
		(i.e. near a 180 degree angle) */
	scale = sqwidth / dmr2;
	dmx *= scale;
	dmy *= scale;
	
	/* going straight */
	if (cross * cross < EPSILON_2 && dx0 * dx1 + dy0 * dy1 >= 0) {
		e = subpath2D_addpoint(forw, vpath[i1].x - dlx0, vpath[i1].y - dly0);
		if (e) return e;
		e = subpath2D_addpoint(rev, vpath[i1].x + dlx0, vpath[i1].y + dly0);
		if (e) return e;
	}
	
	/* left turn, forw is outside and rev is inside */
	else if (cross > 0) {
		/* can safely add single intersection point */
		if (	/* check that i1 + dm[xy] is inside i0-i1 rectangle */
			(dx0 + dmx) * dx0 + (dy0 + dmy) * dy0 > 0 &&
			/* and that i1 + dm[xy] is inside i1-i2 rectangle */
			((dx1 - dmx) * dx1 + (dy1 - dmy) * dy1 > 0)
			)
		{
			e = subpath2D_addpoint(rev, vpath[i1].x + dmx, vpath[i1].y + dmy);
			if (e) return e;
		}
		
		/* need to loop-de-loop the inside */
		else {
			e = subpath2D_addpoint(rev, vpath[i1].x + dlx0, vpath[i1].y + dly0);
			if (e) return e;
			e = subpath2D_addpoint(rev, vpath[i1].x, vpath[i1].y);
			if (e) return e;
			e = subpath2D_addpoint(rev, vpath[i1].x + dlx1, vpath[i1].y + dly1);
			if (e) return e;
		}
		
		if (lineJoin == M4LineJoinBevel) {
			e = subpath2D_addpoint(forw, vpath[i1].x - dlx0, vpath[i1].y - dly0);
			if (e) return e;
			e = subpath2D_addpoint(forw, vpath[i1].x - dlx1, vpath[i1].y - dly1);
			if (e) return e;
		}
		else if (lineJoin == M4LineJoinMiter) {
			e = subpath2D_addpoint(forw, vpath[i1].x - dmx, vpath[i1].y - dmy);
			if (e) return e;
		}
		else if (lineJoin == M4LineJoinRound) {
			e = m4_path_stroke_arc(forw, vpath[i1].x, vpath[i1].y,
					-dlx0, -dly0, -dlx1, -dly1, lineWidth, flatness);
			if (e) return e;
		}
	}
	
	/* right turn, rev is outside and forw is inside */
	else {
		/* can safely add single intersection point */
		if (	/* check that i1 - dm[xy] is inside i0-i1 rectangle */
			(dx0 - dmx) * dx0 + (dy0 - dmy) * dy0 > 0 &&
			/* and that i1 - dm[xy] is inside i1-i2 rectangle */
			((dx1 + dmx) * dx1 + (dy1 + dmy) * dy1 > 0)
			)
		{
			e = subpath2D_addpoint(forw, vpath[i1].x - dmx, vpath[i1].y - dmy);
			if (e) return e;
		}
		
		/* need to loop-de-loop the inside */
		else {
			e = subpath2D_addpoint(forw, vpath[i1].x - dlx0, vpath[i1].y - dly0);
			if (e) return e;
			e = subpath2D_addpoint(forw, vpath[i1].x, vpath[i1].y);
			if (e) return e;
			e = subpath2D_addpoint(forw, vpath[i1].x - dlx1, vpath[i1].y - dly1);
			if (e) return e;
		}
		
		if (lineJoin == M4LineJoinBevel) {
			e = subpath2D_addpoint(rev, vpath[i1].x + dlx0, vpath[i1].y + dly0);
			if (e) return e;
			e = subpath2D_addpoint(rev, vpath[i1].x + dlx1, vpath[i1].y + dly1);
			if (e) return e;
		}
		else if (lineJoin == M4LineJoinMiter) {
			e = subpath2D_addpoint(rev, vpath[i1].x + dmx, vpath[i1].y + dmy);
			if (e) return e;
		}
		else if (lineJoin == M4LineJoinRound) {
			e = m4_path_stroke_arc(rev, vpath[i1].x, vpath[i1].y,
					dlx0, dly0, dlx1, dly1, -lineWidth, flatness);
			if (e) return e;
		}
	}
	return 0;
}

/*
Cap i1, under the assumption of a vector from i0
*/
static M4Err m4_path_render_cap(M4SubPath *result, M4Point2D *vpath, s32 i0, s32 i1, u32 lineCap, Float lineWidth, Float flatness)
{
	Float dx0, dy0;
	Float dlx0, dly0;
	Float scale;
	s32 npts;
	s32 i;
	M4Err e;
	
	dx0 = vpath[i1].x - vpath[i0].x;
	dy0 = vpath[i1].y - vpath[i0].y;

	/* Set dl[xy]0 to the vector from i0 to i1, rotated ccw 90 degrees,
		and scaled to the length of linewidth */
	scale = lineWidth / (Float) sqrt(dx0 * dx0 + dy0 * dy0);
	dlx0 = dy0 * scale;
	dly0 = -dx0 * scale;
	
	switch (lineCap) {
	case M4LineCapFlat:
		e = subpath2D_addpoint(result, vpath[i1].x - dlx0, vpath[i1].y - dly0);
		if (e) return e;
		e = subpath2D_addpoint(result, vpath[i1].x + dlx0, vpath[i1].y + dly0);
		if (e) return e;
		break;
		
	case M4LineCapRound:
		npts = (s32) ceil(M_PI / (2.0 * M_SQRT2 * (Float) sqrt(flatness / lineWidth)));
		e = subpath2D_addpoint(result, vpath[i1].x - dlx0, vpath[i1].y - dly0);
		if (e) return e;
		for (i = 1; i < npts; i++) {
			Float theta, cth, sth;
			theta = (Float) M_PI * i / npts;
			cth = (Float) cos(theta);
			sth = (Float) sin(theta);
			e = subpath2D_addpoint(result,
					vpath[i1].x - dlx0 * cth - dly0 * sth,
					vpath[i1].y - dly0 * cth + dlx0 * sth);
			if (e) return e;
		}
		e = subpath2D_addpoint(result, vpath[i1].x + dlx0, vpath[i1].y + dly0);
		if (e) return e;
		break;

	case M4LineCapTriangle:
		e = subpath2D_addpoint(result, vpath[i1].x - dlx0, vpath[i1].y - dly0);
		if (e) return e;
		e = subpath2D_addpoint(result, vpath[i1].x - dly0, vpath[i1].y + dlx0);
		if (e) return e;
		e = subpath2D_addpoint(result, vpath[i1].x + dlx0, vpath[i1].y + dly0);
		if (e) return e;
		break;
	
	case M4LineCapButt:
		e = subpath2D_addpoint(result,
				vpath[i1].x - dlx0 - dly0,
				vpath[i1].y - dly0 + dlx0);
		if (e) return e;
		e = subpath2D_addpoint(result,
				vpath[i1].x + dlx0 - dly0,
				vpath[i1].y + dly0 + dlx0);
		if (e) return e;
		break;
	}
	return M4OK;
}

static s32 subpath_stroke(M4SubPath *subpath, M4Path *result, M4SubPath *forw, M4SubPath *rev, M4PenSettings *pen, Float flatness)
{
	Float dx, dy;
	Float halflw;
	u32 this, next, second, last;
	M4Err e;
	u32 i;
	
	subpath2D_reset(forw);
	subpath2D_reset(rev);
	
	halflw = 0.5f * pen->width;
	
	/* skip coincident points at the beginning of the subpath */
	this = 0;
	for (i = this + 1; i < subpath->pointlen; i++) {
		dx = subpath->point[i].x - subpath->point[this].x;
		dy = subpath->point[i].y - subpath->point[this].y;
		if (dx * dx + dy * dy > EPSILON_2) {
			break;
		}
	}
	next = i;
	second = next;
	
	/* invariant:  this does not coincide with next */
	while (next < subpath->pointlen) {
		last = this;
		this = next;
		
		/* skip coincident points */
		for (i = this + 1; i < subpath->pointlen; i++) {
			dx = subpath->point[i].x - subpath->point[this].x;
			dy = subpath->point[i].y - subpath->point[this].y;
			if (dx * dx + dy * dy > EPSILON_2) {
				break;
			}
		}
		next = i;
		
		/* reached end of path */
		if (next == subpath->pointlen)
		{
			/* path is closed */
			if (subpath->closed) {
				s32 k;
								
				/* closed: render join to beginning */
				e = m4_path_render_seg(forw, rev, subpath->point,
						last, this, second, pen->join, halflw, pen->miterLimit, flatness);
				if (e) return e;
				
				/* forward path */
				e = m4_path_add_move_to(result,
						forw->point[forw->pointlen-1].x,
						forw->point[forw->pointlen-1].y);
				if (e) return e;
				for (k = 0; k < (s32) forw->pointlen; k++) {
					e = m4_path_add_line_to(result, forw->point[k].x, forw->point[k].y);
					if (e) return e;
				}
				
				/* reverse path, reversed */
				e = m4_path_add_move_to(result, rev->point[0].x, rev->point[0].y);
				if (e) return e;
				for (k = rev->pointlen - 1; k >= 0; k--) {
					e = m4_path_add_line_to(result, rev->point[k].x, rev->point[k].y);
					if (e) return e;
				}
			}
			
			/* path is open */
			else {
				s32 k;
				
				/* add to forw rather than result to ensure forw has at least one point */
				e = m4_path_render_cap(forw, subpath->point,
						last, this, pen->cap, halflw, flatness);
				if (e) return e;

				/* forward path */
				e = m4_path_add_move_to(result, forw->point[0].x, forw->point[0].y);
				if (e) return e;
				for (k = 1; k < (s32) forw->pointlen; k++) {
					e = m4_path_add_line_to(result, forw->point[k].x, forw->point[k].y);
					if (e) return e;
				}
				
				/* reverse path, reversed */
				for (k = rev->pointlen - 1; k >= 0; k--) {
					e = m4_path_add_line_to(result, rev->point[k].x, rev->point[k].y);
					if (e) return e;
				}
				
				/* add second cap (XXX: this messes up cx,cy of result) */
				e = m4_path_render_cap(result->subpath[result->subpathlen-1],
						subpath->point, second, 0, pen->cap, halflw, flatness);
				if (e) return e;

				e = m4_path_add_line_to(result, forw->point[0].x, forw->point[0].y);
				if (e) return e;
			}
		}
		
		/* inside path */
		else {
			e = m4_path_render_seg(forw, rev, subpath->point,
					last, this, next, pen->join, halflw, pen->miterLimit, flatness);
			if (e) return e;
		}
	}
	
	return 0;
}

M4Err m4_path_outline(M4Path *path, M4Path *result, M4PenSettings *pen)
{
	M4SubPath *forw;
	M4SubPath *rev;
	M4Err e;
	u32 i;
	
	m4_path_reset(result);
	
	forw = new_subpath2D();
	if (!forw) return M4OutOfMem;
	
	rev = new_subpath2D();
	if (!rev) {
		subpath2D_free(forw);
		return M4OutOfMem;
	}
	
	for (i=0; i<path->subpathlen; i++) {
		e = subpath_stroke(path->subpath[i], result, forw, rev, pen, 0.25);
		if (e) return e;
	}
	subpath2D_free(rev);
	subpath2D_free(forw);
	return M4OK;
}


#define M4_DOT_LEN		1
#define M4_DOT_SPACE	2
#define M4_DASH_LEN		3

static Float m4_path_get_dash(M4PenSettings *pen, u32 dash_slot, u32 *next_slot)
{
	Float ret = 0;
	switch (pen->dash) {
	case M4StrikeDot:
		if (dash_slot==0) ret = M4_DOT_LEN;
		else if (dash_slot==1) ret = M4_DOT_SPACE;
		*next_slot = (dash_slot + 1) % 2;
		return ret * pen->width ;
	case M4StrikeDash:
		if (dash_slot==0) ret = M4_DASH_LEN;
		else if (dash_slot==1) ret = M4_DOT_SPACE;
		*next_slot = (dash_slot + 1) % 2;
		return ret * pen->width ;
	case M4StrikeDashDot:
		if (dash_slot==0) ret = M4_DASH_LEN;
		else if (dash_slot==1) ret = M4_DOT_SPACE;
		else if (dash_slot==2) ret = M4_DOT_LEN;
		else if (dash_slot==3) ret = M4_DOT_SPACE;
		*next_slot = (dash_slot + 1) % 4;
		return ret * pen->width ;
	case M4StrikeDashDashDot:
		if (dash_slot==0) ret = M4_DASH_LEN;
		else if (dash_slot==1) ret = M4_DOT_SPACE;
		else if (dash_slot==2) ret = M4_DASH_LEN;
		else if (dash_slot==3) ret = M4_DOT_SPACE;
		else if (dash_slot==4) ret = M4_DOT_LEN;
		else if (dash_slot==5) ret = M4_DOT_SPACE;
		*next_slot = (dash_slot + 1) % 6;
		return ret * pen->width ;
	case M4StrikeDashDotDot:
		if (dash_slot==0) ret = M4_DASH_LEN;
		else if (dash_slot==1) ret = M4_DOT_SPACE;
		else if (dash_slot==2) ret = M4_DOT_LEN;
		else if (dash_slot==3) ret = M4_DOT_SPACE;
		else if (dash_slot==4) ret = M4_DOT_LEN;
		else if (dash_slot==5) ret = M4_DOT_SPACE;
		*next_slot = (dash_slot + 1) % 6;
		return ret * pen->width ;
	case M4StrikeCustom:
		if (!pen->dash_set || !pen->dash_set->num_dash) return 0;
		if (dash_slot>=pen->dash_set->num_dash) dash_slot = 0;
		ret = pen->dash_set->dashes[dash_slot];
		*next_slot = (1 + dash_slot) % pen->dash_set->num_dash;
		return ret * pen->width;

	default:
	case M4StrikePlain:
		*next_slot = 0;
		return 0;
	}
}


/* Credits go to Raph Levien for libart / art_vpath_dash */

/* Merge first and last subpaths when first and last dash segment are joined a closepath. */
static M4Err m4_path_mergedashes(M4Path *path, u32 firstindex)
{
	M4SubPath *first;
	M4SubPath *last;
	M4SubPath *temp;
	M4Err e;
	u32 i;
	
	assert(firstindex < path->subpathlen - 1 && "mergedashes");

	/* create new temp subpath */
	e = m4_path_newsubpath(path);
	if (e) return e;

	first = path->subpath[firstindex];
	last = path->subpath[path->subpathlen-2];
	temp = path->subpath[path->subpathlen-1];
	
	subpath2D_reset(temp);

	for (i=0; i<last->pointlen; i++) {
		e = subpath2D_addpoint(temp, last->point[i].x, last->point[i].y);
		if (e) return e;
	}
	
	/* last point of 'last' is same as first point of 'first' */
	for (i=1; i<first->pointlen; i++) {
		e = subpath2D_addpoint(temp, first->point[i].x, first->point[i].y);
		if (e) return e;
	}
	
	/* swap temp with first */
	path->subpath[firstindex] = temp;
	path->subpath[path->subpathlen-1] = first;
	path->subpathlen -= 2;
	return 0;
}

static s32 m4_subpath_dash(M4SubPath *subpath, M4Path *result, M4PenSettings *pen)
{
	Float *dists;
	Float totaldist;
	Float dash;
	Float dist;
	s32 offsetinit, next_offset;
	s32 toggleinit;
	s32 firstindex;
	M4Err e;
	u32 i, start_ind;
	
	dists = malloc(sizeof (Float) * subpath->pointlen);
	if (dists == NULL) return 1;
	
	/* initial values */
	toggleinit = 1;
	offsetinit = 0;

	dash = m4_path_get_dash(pen, offsetinit, &next_offset);
	firstindex = -1;
	
	start_ind = 0;
	dist = 0;

	/* calculate line lengths and update offset*/
	totaldist = 0;
	for (i = 0; i < subpath->pointlen - 1; i++) {
		Float dx = subpath->point[i+1].x - subpath->point[i].x;
		Float dy = subpath->point[i+1].y - subpath->point[i].y;
		dists[i] = (Float) sqrt(dx * dx + dy * dy);
		if (pen->dash_offset > dists[i]) {
			pen->dash_offset -= dists[i];
			dists[i] = 0;
		} else if (pen->dash_offset) {
			Float a;
			Float x, y;
			Float dx, dy;

			a = pen->dash_offset / dists[i];
			dx = subpath->point[i + 1].x - subpath->point[i].x;
			dy = subpath->point[i + 1].y - subpath->point[i].y;
			x = subpath->point[i].x + a * dx;
			y = subpath->point[i].y + a * dy;
			e = m4_path_add_move_to(result, x, y);
			if (e) return e;
			totaldist += dists[i];
			dist = pen->dash_offset;
			pen->dash_offset = 0;
			start_ind = i;
		} else {
			totaldist += dists[i];
		}
	}
	/* subpath fits within first dash and no offset*/
	if (!dist && totaldist <= dash) {
		if (toggleinit) {
			e = m4_path_addsubpath(result, subpath, 0);
			if (e) return e;
		}
	}
	
	/* subpath is composed of at least one dash */
	else {
		Float phase = 0;
		s32 offset = offsetinit;
		s32 toggle = toggleinit;
		i = start_ind;
		
		if (toggle && !dist) {
			e = m4_path_add_move_to(result, subpath->point[i].x, subpath->point[i].y);
			if (e)  return e;
			firstindex = result->subpathlen - 1;
		}
		
		while (i < subpath->pointlen - 1) {
			/* dash boundary is next */
			if (dists[i] - dist > dash - phase) {
				Float a;
				Float x, y;
				Float dx, dy;
				
				dist += dash - phase;
				a = dist / dists[i];
				dx = subpath->point[i + 1].x - subpath->point[i].x;
				dy = subpath->point[i + 1].y - subpath->point[i].y;
				x = subpath->point[i].x + a * dx;
				y = subpath->point[i].y + a * dy;
				
				if (toggle) {
					e = m4_path_add_line_to(result, x, y);
					if (e) return e;
				}
				else {
					e = m4_path_add_move_to(result, x, y);
					if (e) return e;
				}
				
				/* advance to next dash */
				toggle = !toggle;
				phase = 0;

				offset = next_offset;
				dash = m4_path_get_dash(pen, offset, &next_offset);
			}
			/* end of line in subpath is next */
			else {
				phase += dists[i] - dist;
				i ++;
				dist = 0;
				if (toggle) {
					e = m4_path_add_line_to(result, subpath->point[i].x, subpath->point[i].y);
					if (e) return e;
					
					if (i == (subpath->pointlen - 1) && subpath->closed && (firstindex + 1) != (s32) start_ind ) {
						e = m4_path_mergedashes(result, firstindex);
						if (e) return e;
					}
				}
			}
		}
	}
	free(dists);
	return 0;
}

M4Err m4_path_dash(M4Path *path, M4Path *result, M4PenSettings *pen)
{
	Float dash_off;
	M4Err e;
	u32 i;
	m4_path_reset(result);
	dash_off = pen->dash_offset;
	for (i=0; i < path->subpathlen; i++) {
		e = m4_subpath_dash(path->subpath[i], result, pen);
		if (e) return e;
	}
	pen->dash_offset = dash_off;
	return M4OK;
}
