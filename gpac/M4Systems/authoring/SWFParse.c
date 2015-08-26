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


#include <gpac/m4_mpeg4_nodes.h>
#include <math.h>
#include <gpac/intern/m4_swf_dev.h>
#include <gpac/intern/m4_render_tools.h>

const char *swf_get_tag(u32 tag);


u16 swf_get_od_id(SWFReader *read)
{
	return ++read->prev_od_id;
}
u16 swf_get_es_id(SWFReader *read)
{
	return ++read->prev_es_id;
}

void swf_init_decompress(SWFReader *read)
{
	read->compressed = (read->sig[0] == 'C') ? 1 : 0; 
	if (!read->compressed) return;
}
void swf_done_decompress(SWFReader *read)
{
}


M4Err swf_seek_file_to(SWFReader *read, u32 size)
{
	if (!read->compressed) return BS_Seek(read->bs, size);
	return M4NotSupported;
}

u32 swf_get_file_pos(SWFReader *read)
{
	if (!read->compressed) return (u32) BS_GetPosition(read->bs);
	return 0;
}

u32 swf_read_data(SWFReader *read, char *data, u32 data_size)
{
	if (!read->compressed) return BS_ReadData(read->bs, data, data_size);
	return 0;
}

u32 swf_read_int(SWFReader *read, u32 nbBits)
{
	if (!read->compressed) return BS_ReadInt(read->bs, nbBits);
	return 0;
}

s32 swf_read_sint(SWFReader *read, u32 nbBits)
{
	s32 r = 0;
	u32 i;
	if (!nbBits)return 0;
	r = -1 * (s32) swf_read_int(read, 1);
	for (i=1; i<nbBits; i++){
		r <<= 1;
		r |= swf_read_int(read, 1);
	}
	return r;
}


u32 swf_align(SWFReader *read)
{
	if (!read->compressed) return BS_Align(read->bs);
	return 0;
}
void swf_skip_data(SWFReader *read, u32 size)
{
	while (size && !read->ioerr) {
		swf_read_int(read, 8);
		size --;
	}
}
void swf_get_rec(SWFReader *read, SWFRec *rc)
{
	u32 nbbits;
	swf_align(read);
	nbbits = swf_read_int(read, 5);
	rc->x = swf_read_sint(read, nbbits) * SWF_TWIP_SCALE;
	rc->w = swf_read_sint(read, nbbits) * SWF_TWIP_SCALE;
	rc->w -= rc->x;
	rc->y = swf_read_sint(read, nbbits) * SWF_TWIP_SCALE;
	rc->h = swf_read_sint(read, nbbits) * SWF_TWIP_SCALE;
	rc->h -= rc->y;
}
u32 swf_get_32(SWFReader *read)
{
	u32 val, res;
	val = swf_read_int(read, 32);
	res = (val&0xFF); res <<=8;
	res |= ((val>>8)&0xFF); res<<=8;
	res |= ((val>>16)&0xFF); res<<=8;
	res|= ((val>>24)&0xFF);
	return res;
}
u16 swf_get_16(SWFReader *read)
{
	u16 val, res;
	val = swf_read_int(read, 16);
	res = (val&0xFF); res <<=8;
	res |= ((val>>8)&0xFF);
	return res;
}
s16 swf_get_s16(SWFReader *read)
{
	s16 val;
	u8 v1;
	v1 = swf_read_int(read, 8);
	val = swf_read_sint(read, 8);
	val = (val<<8)&0xFF00; 
	val |= (v1&0xFF);
	return val;
}
u32 swf_get_color(SWFReader *read)
{
	u32 res;
	res = 0xFF00;
	res |= swf_read_int(read, 8); res<<=8;
	res |= swf_read_int(read, 8); res<<=8;
	res |= swf_read_int(read, 8);
	return res;
}
u32 swf_get_argb(SWFReader *read)
{
	u32 res, al;
	res = swf_read_int(read, 8); res<<=8;
	res |= swf_read_int(read, 8); res<<=8;
	res |= swf_read_int(read, 8);
	al = swf_read_int(read, 8);
	return ((al<<24) | res);
}
u32 swf_get_matrix(SWFReader *read, M4Matrix2D *mat, Bool rescale)
{
	u32 bits_read;
	u32 flag, nb_bits;

	memset(mat, 0, sizeof(M4Matrix2D));
	mat->m[0] = mat->m[4] = 1;

	bits_read = swf_align(read);
	
	flag = swf_read_int(read, 1);
	bits_read += 1;
	if (flag) {
		nb_bits = swf_read_int(read, 5);
		mat->m[0] = (Float) swf_read_sint(read, nb_bits);
		mat->m[0] /= 0x10000;
		mat->m[4] = (Float) swf_read_sint(read, nb_bits);
		mat->m[4] /= 0x10000;
		bits_read += 5 + 2*nb_bits;
	}
	flag = swf_read_int(read, 1);
	bits_read += 1;
	if (flag) {
		nb_bits = swf_read_int(read, 5);
		/*WATCHOUT FOR ORDER*/
		mat->m[3] = (Float) swf_read_sint(read, nb_bits);
		mat->m[3] /= 0x10000;
		mat->m[1] = (Float) swf_read_sint(read, nb_bits);
		mat->m[1] /= 0x10000;
		bits_read += 5 + 2*nb_bits;
	}
	nb_bits = swf_read_int(read, 5);
	bits_read += 5 + 2*nb_bits;
	if (nb_bits) {
		mat->m[2] = swf_read_sint(read, nb_bits) * SWF_TWIP_SCALE;
		mat->m[5] = swf_read_sint(read, nb_bits) * SWF_TWIP_SCALE;
	}

	/*for gradients and bitmap texture transforms*/
	if (rescale) {
		mat->m[0] *= SWF_TWIP_SCALE;
		mat->m[1] *= SWF_TWIP_SCALE;
		mat->m[3] *= SWF_TWIP_SCALE;
		mat->m[4] *= SWF_TWIP_SCALE;
	}
	return bits_read;
}

void swf_get_colormatrix(SWFReader *read, M4ColorMatrix *cmat)
{
	Bool has_add, has_mul;
	u32 nbbits;
	memset(cmat, 0, sizeof(M4ColorMatrix));
	cmat->m[0] = cmat->m[6] = cmat->m[12] = cmat->m[18] = 1;

	swf_align(read);
	has_add = swf_read_int(read, 1);
	has_mul = swf_read_int(read, 1);
	nbbits = swf_read_int(read, 4);
	if (has_mul) {
		cmat->m[0] = swf_read_int(read, nbbits) * SWF_COLOR_SCALE;
		cmat->m[6] = swf_read_int(read, nbbits) * SWF_COLOR_SCALE;
		cmat->m[12] = swf_read_int(read, nbbits) * SWF_COLOR_SCALE;
		cmat->m[18] = swf_read_int(read, nbbits) * SWF_COLOR_SCALE;
	}
	if (has_add) {
		cmat->m[4] = swf_read_int(read, nbbits) * SWF_COLOR_SCALE;
		cmat->m[9] = swf_read_int(read, nbbits) * SWF_COLOR_SCALE;
		cmat->m[14] = swf_read_int(read, nbbits) * SWF_COLOR_SCALE;
		cmat->m[19] = swf_read_int(read, nbbits) * SWF_COLOR_SCALE;
	}
	cmat->identity = 0;
	if ((cmat->m[0] == cmat->m[6] == cmat->m[12] == cmat->m[18] == 1)
		&& (cmat->m[4] == cmat->m[9] == cmat->m[14] == cmat->m[19] == 0))
		cmat->identity = 1;

}

char *swf_get_string(SWFReader *read)
{
	char szName[1024];
	u32 i = 0;
	while (1) {
		szName[i] = swf_read_int(read, 8);
		if (!szName[i]) break;
		i++;
	}
	return strdup(szName);
}

SFNode *SWF_NewNode(SWFReader *read, u32 tag)
{
	SFNode *n = SG_NewNode(read->load->scene_graph, tag);
	if (n) Node_Init(n);
	return n;
}

Bool SWF_CheckDepth(SWFReader *read, u32 depth)
{
	SFNode *disp, *empty;
	if (read->max_depth > depth) return 1;
	/*modify display list*/
	disp = SG_FindNodeByName(read->load->scene_graph, "DISPLAYLIST");

	empty = SG_FindNodeByName(read->load->scene_graph, "EMPTYSHAPE");
	while (read->max_depth<=depth) {
		Node_InsertChild(disp, empty, -1);
		Node_Register(empty, disp);
		read->max_depth++;
	}
	return 0;
}


SWFShapeRec *swf_new_shape_rec()
{
	SWFShapeRec *style = malloc(sizeof(SWFShapeRec));
	memset(style, 0, sizeof(SWFShapeRec));
	style->path = malloc(sizeof(SWFPath));
	memset(style->path, 0, sizeof(SWFPath));
	return style;
}
SWFShapeRec *swf_clone_shape_rec(SWFShapeRec *old_sr)
{
	SWFShapeRec *new_sr = malloc(sizeof(SWFShapeRec));
	memcpy(new_sr, old_sr, sizeof(SWFShapeRec));
	new_sr->path = malloc(sizeof(SWFPath));
	memset(new_sr->path, 0, sizeof(SWFPath));

	if (old_sr->nbGrad) {
		new_sr->grad_col = malloc(sizeof(u32) * old_sr->nbGrad);
		memcpy(new_sr->grad_col, old_sr->grad_col, sizeof(u32) * old_sr->nbGrad);
		new_sr->grad_ratio = malloc(sizeof(u8) * old_sr->nbGrad);
		memcpy(new_sr->grad_ratio, old_sr->grad_ratio, sizeof(u8) * old_sr->nbGrad);
	}
	return new_sr;
}

/*parse/append fill and line styles*/
void swf_parse_styles(SWFReader *read, u32 revision, SWFShape *shape, u32 *bits_fill, u32 *bits_line)
{
	u32 i, j, count;
	SWFShapeRec *style;
	
	swf_align(read);

	/*get fill styles*/
	count = swf_read_int(read, 8);
	if (revision && (count== 0xFF)) count = swf_get_16(read);
	if (count) {
		for (i=0; i<count; i++) {
			style = swf_new_shape_rec();

			style->solid_col = 0xFF00FF00;
			style->type = swf_read_int(read, 8);

			/*gradient fill*/
			if (style->type & 0x10) {
				swf_get_matrix(read, &style->mat, 1);
				swf_align(read);
				style->nbGrad = swf_read_int(read, 8);
				if (style->nbGrad) {
					SAFEALLOC(style->grad_col, sizeof(u32) * style->nbGrad);
					SAFEALLOC(style->grad_ratio, sizeof(u8) * style->nbGrad);
					for (j=0; j<style->nbGrad; j++) {
						style->grad_ratio[j] = swf_read_int(read, 8);
						if (revision==2) style->grad_col[j] = swf_get_argb(read);
						else style->grad_col[j] = swf_get_color(read);
					}
					style->solid_col = style->grad_col[0];

					/*make sure we have keys between 0 and 1.0 for BIFS (0 and 255 in swf)*/
					if (style->grad_ratio[0] != 0) {
						u32 i;
						u32 *grad_col;
						u8 *grad_ratio;
						SAFEALLOC(grad_ratio, sizeof(u8) * (style->nbGrad+1));
						SAFEALLOC(grad_col, sizeof(u32) * (style->nbGrad+1));
						grad_col[0] = style->grad_col[0];
						grad_ratio[0] = 0;
						for (i=0; i<style->nbGrad; i++) {
							grad_col[i+1] = style->grad_col[i];
							grad_ratio[i+1] = style->grad_ratio[i];
						}
						free(style->grad_col);
						style->grad_col = grad_col;
						free(style->grad_ratio);
						style->grad_ratio = grad_ratio;
						style->nbGrad++;
					}
					if (style->grad_ratio[style->nbGrad-1] != 255) {
						u32 *grad_col = malloc(sizeof(u32) * (style->nbGrad+1));
						u8 *grad_ratio = malloc(sizeof(u8) * (style->nbGrad+1));
						memcpy(grad_col, style->grad_col, sizeof(u32) * style->nbGrad);
						memcpy(grad_ratio, style->grad_ratio, sizeof(u8) * style->nbGrad);
						grad_col[style->nbGrad] = style->grad_col[style->nbGrad-1];
						grad_ratio[style->nbGrad] = 255;
						free(style->grad_col);
						style->grad_col = grad_col;
						free(style->grad_ratio);
						style->grad_ratio = grad_ratio;
						style->nbGrad++;
					}

				} else {
					style->solid_col = 0xFF;
				}
			} 
			/*bitmap fill*/
			else if (style->type & 0x40) {
				style->img_id = swf_get_16(read);
				if (style->img_id == 65535) {
					style->img_id = 0;
					style->type = 0;
					style->solid_col = 0xFF00FFFF;
				}
				swf_get_matrix(read, &style->mat, 1);
			} 
			/*solid fill*/
			else {
				if (revision==2) style->solid_col = swf_get_argb(read);
				else style->solid_col = swf_get_color(read);
			}
			ChainAddEntry(shape->fill_right, style);
			style = swf_clone_shape_rec(style);
			ChainAddEntry(shape->fill_left, style);
		}
	}

	swf_align(read);
	/*get line styles*/
	count = swf_read_int(read, 8);
	if (revision && (count==0xFF)) count = swf_get_16(read);
	if (count) {
		for (i=0; i<count; i++) {
			style = swf_new_shape_rec();
			ChainAddEntry(shape->lines, style);
			style->width = swf_get_16(read) * SWF_TWIP_SCALE;
			if (revision==2) style->solid_col = swf_get_argb(read);
			else style->solid_col = swf_get_color(read);
		}
	}

	swf_align(read);
	*bits_fill = swf_read_int(read, 4);
	*bits_line = swf_read_int(read, 4);
}

void swf_path_realloc_pts(SWFPath *path, u32 nbPts)
{
	if (path->nbPts) {
		path->pts = realloc(path->pts, sizeof(SFVec2f) * (path->nbPts + nbPts));
	} else {
		path->pts = malloc(sizeof(SFVec2f) * nbPts);
	}
}
void swf_path_add_com(SWFShapeRec *sr, SFVec2f pt, SFVec2f ctr, u32 type)
{
	/*not an error*/
	if (!sr) return;

	if (sr->path->nbType) {
		sr->path->types = realloc(sr->path->types, sizeof(u32) * (sr->path->nbType+1));
	} else {
		sr->path->types = malloc(sizeof(u32));
	}
	sr->path->types[sr->path->nbType] = type;
	switch (type) {
	case 2:
		swf_path_realloc_pts(sr->path, 2);
		sr->path->pts[sr->path->nbPts] = ctr;
		sr->path->pts[sr->path->nbPts+1] = pt;
		sr->path->nbPts+=2;
		break;
	case 1:
	default:
		swf_path_realloc_pts(sr->path, 1);
		sr->path->pts[sr->path->nbPts] = pt;
		sr->path->nbPts++;
		break;
	}
	sr->path->nbType++;
}

void swf_referse_path(SWFPath *path)
{
	u32 i, j, pti, ptj;
	u32 *types;
	SFVec2f *pts;

	if (path->nbType<=1) return;

	types = (u32 *) malloc(sizeof(u32) * path->nbType);
	pts = (SFVec2f *) malloc(sizeof(SFVec2f) * path->nbPts);


	/*need first moveTo*/
	types[0] = 0;
	pts[0] = path->pts[path->nbPts - 1];
	pti = path->nbPts - 2;
	ptj = 1;
	j=1;

	for (i=0; i<path->nbType-1; i++) {
		types[j] = path->types[path->nbType - i - 1];
		switch (types[j]) {
		case 2:
			assert(ptj<=path->nbPts-2);
			pts[ptj] = path->pts[pti];
			pts[ptj+1] = path->pts[pti-1];
			pti-=2;
			ptj+=2;
			break;
		case 1:
			assert(ptj<=path->nbPts-1);
			pts[ptj] = path->pts[pti];
			pti--;
			ptj++;
			break;
		case 0:
			assert(ptj<=path->nbPts-1);
			pts[ptj] = path->pts[pti];
			pti--;
			ptj++;
			break;
		}
		j++;
	}
	free(path->pts);
	path->pts = pts;
	free(path->types);
	path->types = types;
}

void swf_free_shape_rec(SWFShapeRec *ptr)
{
	if (ptr->grad_col) free(ptr->grad_col);
	if (ptr->grad_ratio) free(ptr->grad_ratio);
	if (ptr->path) {
		if (ptr->path->pts) free(ptr->path->pts);
		if (ptr->path->types) free(ptr->path->types);
		free(ptr->path);
	}
	free(ptr);
}

void swf_free_rec_list(Chain *recs)
{
	while (ChainGetCount(recs)) {
		SWFShapeRec *tmp = ChainGetEntry(recs, 0);
		ChainDeleteEntry(recs, 0);
		swf_free_shape_rec(tmp);
	}
	DeleteChain(recs);
}

void swf_append_path(SWFPath *a, SWFPath *b)
{
	if (b->nbType<=1) return;

	a->pts = realloc(a->pts, sizeof(SFVec2f) * (a->nbPts + b->nbPts));
	memcpy(&a->pts[a->nbPts], b->pts, sizeof(SFVec2f)*b->nbPts);
	a->nbPts += b->nbPts;

	a->types = realloc(a->types, sizeof(u32)*(a->nbType+ b->nbType));
	memcpy(&a->types[a->nbType], b->types, sizeof(u32)*b->nbType);
	a->nbType += b->nbType;
}

void swf_path_add_type(SWFPath *path, u32 val)
{
	if (path->nbType) {
		path->types = realloc(path->types, sizeof(u32) * (path->nbType + 1));
	} else {
		path->types = malloc(sizeof(u32));
	}
	path->types[path->nbType] = val;
	path->nbType++;
}

void swf_resort_path(SWFPath *a, SWFReader *read)
{
	u32 idx, i, j;
	Chain *paths;
	SWFPath *sorted;

	if (!a->nbType) return;

	paths = NewChain();
	sorted = malloc(sizeof(SWFPath));
	memset(sorted, 0, sizeof(SWFPath));
	swf_path_realloc_pts(sorted, 1);
	sorted->pts[sorted->nbPts] = a->pts[0];
	sorted->nbPts++;
	swf_path_add_type(sorted, 0);
	ChainAddEntry(paths, sorted);

	/*1- split all paths*/
	idx = 1;
	for (i=1; i<a->nbType; i++) {
		switch (a->types[i]) {
		case 2:
			swf_path_realloc_pts(sorted, 2);
			sorted->pts[sorted->nbPts] = a->pts[idx];
			sorted->pts[sorted->nbPts+1] = a->pts[idx+1];
			sorted->nbPts+=2;
			swf_path_add_type(sorted, 2);
			idx += 2;
			break;
		case 1:
			swf_path_realloc_pts(sorted, 1);
			sorted->pts[sorted->nbPts] = a->pts[idx];
			sorted->nbPts+=1;
			swf_path_add_type(sorted, 1);
			idx += 1;
			break;
		case 0:
			sorted = malloc(sizeof(SWFPath));
			memset(sorted, 0, sizeof(SWFPath));
			swf_path_realloc_pts(sorted, 1);
			sorted->pts[sorted->nbPts] = a->pts[idx];
			sorted->nbPts++;
			swf_path_add_type(sorted, 0);
			ChainAddEntry(paths, sorted);
			idx += 1;
			break;
		}
	}

restart:
	for (i=0; i<ChainGetCount(paths); i++) {
		SWFPath *p = ChainGetEntry(paths, i);

		for (j=i+1; j<ChainGetCount(paths); j++) {
			SWFPath *np = ChainGetEntry(paths, j);
	
			/*check if any next subpath ends at the same place we're starting*/
			if ((np->pts[np->nbPts-1].x == p->pts[0].x) && (np->pts[np->nbPts-1].y == p->pts[0].y)) {
				u32 k;
				idx = 1;
				for (k=1; k<p->nbType; k++) {
					switch (p->types[k]) {
					case 2:
						swf_path_realloc_pts(np, 2);
						np->pts[np->nbPts] = p->pts[idx];
						np->pts[np->nbPts+1] = p->pts[idx+1];
						np->nbPts+=2;
						swf_path_add_type(np, 2);
						idx += 2;
						break;
					case 1:
						swf_path_realloc_pts(np, 1);
						np->pts[np->nbPts] = p->pts[idx];
						np->nbPts+=1;
						swf_path_add_type(np, 1);
						idx += 1;
						break;
					default:
						assert(0);
						break;
					}
				}
				free(p->pts);
				free(p->types);
				free(p);
				ChainDeleteEntry(paths, i);
				goto restart;
			}
			/*check if any next subpath starts at the same place we're ending*/
			else if ((p->pts[p->nbPts-1].x == np->pts[0].x) && (p->pts[p->nbPts-1].y == np->pts[0].y)) {
				u32 k;
				idx = 1;
				for (k=1; k<np->nbType; k++) {
					switch (np->types[k]) {
					case 2:
						swf_path_realloc_pts(p, 2);
						p->pts[p->nbPts] = np->pts[idx];
						p->pts[p->nbPts+1] = np->pts[idx+1];
						p->nbPts+=2;
						swf_path_add_type(p, 2);
						idx += 2;
						break;
					case 1:
						swf_path_realloc_pts(p, 1);
						p->pts[p->nbPts] = np->pts[idx];
						p->nbPts+=1;
						swf_path_add_type(p, 1);
						idx += 1;
						break;
					default:
						assert(0);
						break;
					}
				}
				free(np->pts);
				free(np->types);
				free(np);
				ChainDeleteEntry(paths, j);
				j--;
			}
		}
	}

	/*reassemble path*/
	free(a->pts);
	free(a->types);
	memset(a, 0, sizeof(SWFPath));

	while (ChainGetCount(paths)) {
		sorted = ChainGetEntry(paths, 0);
		if (read->flat_limit==0.0f) {
			swf_append_path(a, sorted);
		} else {
			Bool prev_is_line_to = 0;
			idx = 0;
			for (i=0; i<sorted->nbType; i++) {
				switch (sorted->types[i]) {
				case 2:
					swf_path_realloc_pts(a, 2);
					a->pts[a->nbPts] = sorted->pts[idx];
					a->pts[a->nbPts+1] = sorted->pts[idx+1];
					a->nbPts+=2;
					swf_path_add_type(a, 2);
					idx += 2;
					prev_is_line_to = 0;
					break;
				case 1:
					if (prev_is_line_to) {
						Float angle;
						Bool flatten = 0;
						SFVec2f v1, v2;
						v1.x = a->pts[a->nbPts-1].x - a->pts[a->nbPts-2].x;
						v1.y = a->pts[a->nbPts-1].y - a->pts[a->nbPts-2].y;
						v2.x = a->pts[a->nbPts-1].x - sorted->pts[idx].x;
						v2.y = a->pts[a->nbPts-1].y - sorted->pts[idx].y;

						angle = v1.x*v2.x + v1.y*v2.y;
						/*get magnitudes*/
						v1.x = (Float) sqrt(v1.x*v1.x + v1.y*v1.y);
						v2.x = (Float) sqrt(v2.x*v2.x + v2.y*v2.y);
						if (!v1.x || !v2.x) flatten = 1;
						else {
							Float h_pi = M_PI / 2;
							angle /= (v1.x * v2.x);
							if (angle + M4_EPSILON_FLOAT >= 1.0f) angle = 0.0;
							else if (angle - M4_EPSILON_FLOAT <= -1.0f) angle = M_PI;
							else angle = (Float) acos(angle);

							if (angle<0) angle += h_pi;
							angle = ABSDIFF(angle, h_pi);
							if (angle < read->flat_limit) flatten = 1;
						}
						if (flatten) {
							a->pts[a->nbPts-1] = sorted->pts[idx];
							idx++;
							read->flatten_points++;
							break;
						}
					}
					swf_path_realloc_pts(a, 1);
					a->pts[a->nbPts] = sorted->pts[idx];
					a->nbPts+=1;
					swf_path_add_type(a, 1);
					idx += 1;
					prev_is_line_to = 1;
					break;
				case 0:
					swf_path_realloc_pts(a, 1);
					a->pts[a->nbPts] = sorted->pts[idx];
					a->nbPts+=1;
					swf_path_add_type(a, 0);
					idx += 1;
					prev_is_line_to = 0;
					break;
				}
			}
		}
		free(sorted->pts);
		free(sorted->types);
		free(sorted);
		ChainDeleteEntry(paths, 0);
	}
	DeleteChain(paths);
}

/*
	Notes on SWF->BIFS conversion - some ideas taken from libswfdec 
	A single fillStyle has 2 associated path, one used for left fill, one for right fill
	This is then a 4 step process:
	1- blindly parse swf shape, and add point/lines to the proper left/right path
	2- for each fillStyles, revert the right path so that it becomes a left path
	3- concatenate left and right paths
	4- resort all subelements of the final path, making sure moveTo introduced by the SWF coding (due to style changes)
	are removed. 
		Ex: if path is 
			A->C, B->A, C->B = moveTo(A), lineTo(C), moveTo(B), lineTo (A), moveTo(C), lineTo(B)
		we restort and remove unneeded moves to get
			A->C->B = moveTo(A), lineTo(C), lineTo(B), lineTo(A)
*/

SFNode *swf_parse_shape_def(SWFReader *read, Bool has_styles, u32 revision)
{
	u32 ID, nbBits, comType, i;
	s32 x, y;
	SFVec2f orig, ctrl, end;
	Bool flag;
	u32 fill0, fill1, strike;
	SWFRec rc;
	u32 bits_fill, bits_line;
	SWFShape shape;
	Bool is_empty;
	SFNode *n;
	u32 fill_base, line_base;
	SWFShapeRec *sf0, *sf1, *sl;

	memset(&shape, 0, sizeof(SWFShape));
	shape.fill_left = NewChain();
	shape.fill_right = NewChain();
	shape.lines = NewChain();

	swf_align(read);
	ID = 0;

	/*get initial styles*/
	if (has_styles) {
		ID = swf_get_16(read);
		/*don't care about that...*/
		swf_get_rec(read, &rc);
		swf_parse_styles(read, revision, &shape, &bits_fill, &bits_line);
	} else {
		bits_fill = swf_read_int(read, 4);
		bits_line = swf_read_int(read, 4);

		/*fonts are usually defined without styles*/
		if ((read->tag == SWF_DEFINEFONT) || (read->tag==SWF_DEFINEFONT2)) {
			sf0 = swf_new_shape_rec();
			ChainAddEntry(shape.fill_right, sf0);
			sf0 = swf_new_shape_rec();
			ChainAddEntry(shape.fill_left, sf0);
			sf0->solid_col = 0xFF000000;
			sf0->type = 0;
		}
	}
	fill_base = line_base = 0;
	
	is_empty = 1;
	comType = 0;
	/*parse all points*/
	fill0 = fill1 = strike = 0;
	sf0 = sf1 = sl = NULL;
	x = y = 0;
	while (1) {
		flag = swf_read_int(read, 1);
		if (!flag) {
			Bool new_style = swf_read_int(read, 1);
			Bool set_strike = swf_read_int(read, 1);
			Bool set_fill1 = swf_read_int(read, 1);
			Bool set_fill0 = swf_read_int(read, 1);
			Bool move_to = swf_read_int(read, 1);
			/*end of shape*/
			if (!new_style && !set_strike && !set_fill0 && !set_fill1 && !move_to) break;

			is_empty = 0;

			if (move_to) {
				nbBits = swf_read_int(read, 5);
				x = swf_read_sint(read, nbBits);
				y = swf_read_sint(read, nbBits);
			}
			if (set_fill0) fill0 = fill_base + swf_read_int(read, bits_fill);
			if (set_fill1) fill1 = fill_base + swf_read_int(read, bits_fill);
			if (set_strike) strike = line_base + swf_read_int(read, bits_line);
			/*looks like newStyle does not append styles but define a new set - old styles can no 
			longer be referenced*/
			if (new_style) {
				fill_base += ChainGetCount(shape.fill_left);
				line_base += ChainGetCount(shape.lines);
				swf_parse_styles(read, revision, &shape, &bits_fill, &bits_line);
			}

			if (read->flags & M4SWF_NoStrike) strike = 0;

			/*moveto*/
			comType = 0;
			orig.x = x * SWF_TWIP_SCALE;
			orig.y = y * SWF_TWIP_SCALE;
			end = orig;

			sf0 = fill0 ? ChainGetEntry(shape.fill_left, fill0 - 1) : NULL;
			sf1 = fill1 ? ChainGetEntry(shape.fill_right, fill1 - 1) : NULL;
			sl = strike ? ChainGetEntry(shape.lines, strike - 1) : NULL;

			if (move_to) {
				swf_path_add_com(sf0, end, ctrl, 0);
				swf_path_add_com(sf1, end, ctrl, 0);
				swf_path_add_com(sl, end, ctrl, 0);
			} else {
				if (set_fill0) swf_path_add_com(sf0, end, ctrl, 0);
				if (set_fill1) swf_path_add_com(sf1, end, ctrl, 0);
				if (set_strike) swf_path_add_com(sl, end, ctrl, 0);
			}

		} else {
			flag = swf_read_int(read, 1);
			/*quadratic curve*/
			if (!flag) {
				nbBits = 2 + swf_read_int(read, 4);
				x += swf_read_sint(read, nbBits);
				y += swf_read_sint(read, nbBits);
				ctrl.x = x * SWF_TWIP_SCALE;
				ctrl.y = y * SWF_TWIP_SCALE;
				x += swf_read_sint(read, nbBits);
				y += swf_read_sint(read, nbBits);
				end.x = x * SWF_TWIP_SCALE;
				end.y = y * SWF_TWIP_SCALE;
				/*curveTo*/
				comType = 2;
			} 
			/*straight line*/
			else {
				nbBits = 2 + swf_read_int(read, 4);
				flag = swf_read_int(read, 1);
				if (flag) {
					x += swf_read_sint(read, nbBits);
					y += swf_read_sint(read, nbBits);
				} else {
					flag = swf_read_int(read, 1);
					if (flag) {
						y += swf_read_sint(read, nbBits);
					} else {
						x += swf_read_sint(read, nbBits);
					}
				}
				/*lineTo*/
				comType = 1;
				end.x = x * SWF_TWIP_SCALE;
				end.y = y * SWF_TWIP_SCALE;
			}
			swf_path_add_com(sf0, end, ctrl, comType);
			swf_path_add_com(sf1, end, ctrl, comType);
			swf_path_add_com(sl, end, ctrl, comType);
		}
	}

	if (is_empty) {
		swf_free_rec_list(shape.fill_left);
		swf_free_rec_list(shape.fill_right);
		swf_free_rec_list(shape.lines);
		return NULL;
	}

	swf_align(read);

	for (i=0; i<ChainGetCount(shape.fill_left); i++) {
		sf0 = ChainGetEntry(shape.fill_left, i);
		sf1 = ChainGetEntry(shape.fill_right, i);
		/*reverse right path*/
		swf_referse_path(sf1->path);
		/*concatenate with left path*/
		swf_append_path(sf0->path, sf1->path);
		/*resort all path curves*/
		swf_resort_path(sf0->path, read);
	}
	/*remove dummy fill_left*/
	for (i=0; i<ChainGetCount(shape.fill_left); i++) {
		sf0 = ChainGetEntry(shape.fill_left, i);
		if (sf0->path->nbType<=1) {
			ChainDeleteEntry(shape.fill_left, i);
			swf_free_shape_rec(sf0);
			i--;
		}
	}
	/*remove dummy lines*/
	for (i=0; i<ChainGetCount(shape.lines); i++) {
		sl = ChainGetEntry(shape.lines, i);
		if (sl->path->nbType<1) {
			ChainDeleteEntry(shape.lines, i);
			swf_free_shape_rec(sl);
			i--;
		} else {
			swf_resort_path(sl->path, read);
		}
	}

	/*now translate a flash shape record into BIFS*/
	shape.ID = ID;
	n = SWFShapeToBIFS(read, &shape);

	/*delete shape*/
	swf_free_rec_list(shape.fill_left);
	swf_free_rec_list(shape.fill_right);
	swf_free_rec_list(shape.lines);

	if (n && has_styles) {
		char szDEF[1024];
		sprintf(szDEF, "Shape%d", ID);
		read->load->ctx->max_node_id++;
		ID = read->load->ctx->max_node_id;
		Node_SetDEF(n, ID, szDEF);
	}
	return n;
}

SWFFont *SWF_FindFont(SWFReader *read, u32 ID)
{
	u32 i, count;
	count = ChainGetCount(read->fonts);
	for (i=0; i<count; i++) {
		SWFFont *ft = ChainGetEntry(read->fonts, i);
		if (ft->fontID==ID) return ft;
	}
	return NULL;
}

SFNode *SWF_GetNode(SWFReader *read, u32 ID)
{
	SFNode *n;
	char szDEF[1024];
	sprintf(szDEF, "Shape%d", ID);
	n = SG_FindNodeByName(read->load->scene_graph, szDEF);
	if (n) return n;
	sprintf(szDEF, "Text%d", ID);
	n = SG_FindNodeByName(read->load->scene_graph, szDEF);
	if (n) return n;
	return NULL;
}
DispShape *SWF_GetDepthEntry(SWFReader *read, u32 Depth, Bool create)
{
	u32 i;
	DispShape *tmp;
	for (i=0; i<ChainGetCount(read->display_list); i++) {
		tmp = ChainGetEntry(read->display_list, i);
		if (tmp->depth == Depth) return tmp;
	}
	if (!create) return NULL;
	tmp = malloc(sizeof(DispShape));
	tmp->depth = Depth;
	tmp->n = NULL;
	ChainAddEntry(read->display_list, tmp);

	memset(&tmp->mat, 0, sizeof(M4Matrix2D));
	tmp->mat.m[0] = tmp->mat.m[4] = 1;
	
	memset(&tmp->cmat, 0, sizeof(M4ColorMatrix));
	tmp->cmat.m[0] = tmp->cmat.m[6] = tmp->cmat.m[12] = tmp->cmat.m[18] = 1;
	tmp->cmat.identity = 1;
	return tmp;
}



M4Err swf_func_skip(SWFReader *read)
{
	swf_skip_data(read, read->size);
	return read->ioerr;
}
M4Err swf_unknown_tag(SWFReader *read)
{
	swf_report(read, M4NotSupported, "Tag not implemented - skipping");
	return swf_func_skip(read);
}
M4Err swf_set_backcol(SWFReader *read)
{
	u32 col;
	SGCommand *com;
	CommandFieldInfo *f;
	com = SG_NewCommand(read->load->scene_graph, SG_FieldReplace);
	com->node = SG_FindNodeByName(read->load->scene_graph, "BACKGROUND");
	Node_Register(com->node, NULL);
	f = SG_NewFieldCommand(com);
	f->field_ptr = VRML_NewFieldPointer(FT_SFColor);
	f->fieldType = FT_SFColor;
	f->fieldIndex = 1;	/*backColor index*/
	col = swf_get_color(read);
	((SFColor *)f->field_ptr)->red = (Float) ((col>>16) & 0xFF) / 255.0f;
	((SFColor *)f->field_ptr)->green = (Float) ((col>>8) & 0xFF) / 255.0f;
	((SFColor *)f->field_ptr)->blue = (Float) ((col) & 0xFF) / 255.0f;
	ChainAddEntry(read->bifs_au->commands, com);
	return M4OK;
}


M4Err SWF_InsertNode(SWFReader *read, SFNode *n)
{
	SGCommand *com;
	CommandFieldInfo *f;

	if (read->flags & M4SWF_StaticDictionary) {
		M_Switch *par = (M_Switch *)SG_FindNodeByName(read->load->scene_graph, "DICTIONARY");
		ChainAddEntry(par->choice, n);
		Node_Register((SFNode *)n, (SFNode *)par);
	} else {
		com = SG_NewCommand(read->load->scene_graph, SG_IndexedInsert);
		com->node = SG_FindNodeByName(read->load->scene_graph, "DICTIONARY");
		Node_Register(com->node, NULL);
		f = SG_NewFieldCommand(com);
		f->field_ptr = &f->new_node;
		f->fieldType = FT_SFNode;
		f->fieldIndex = 0;	/*choice index*/
		f->pos = -1;
		f->new_node = n;
		Node_Register(f->new_node, NULL);
		ChainAddEntry(read->bifs_au->commands, com);
	}
	return M4OK;
}

M4Err swf_def_shape(SWFReader *read, u32 revision)
{
	SFNode *new_node;
	new_node = swf_parse_shape_def(read, 1, revision);
	if (!new_node) return M4OK;
	return SWF_InsertNode(read, new_node);
}

Bool swf_mat_is_identity(M4Matrix2D *mat)
{
	if (mat->m[0] != 1.0) return 0;
	if (mat->m[4] != 1.0) return 0;
	if (mat->m[1] != 0.0) return 0;
	if (mat->m[2] != 0.0) return 0;
	if (mat->m[3] != 0.0) return 0;
	if (mat->m[5] != 0.0) return 0;
	return 1;
}

enum
{
	SWF_PLACE,
	SWF_REPLACE,
	SWF_MOVE,
};
M4Err swf_place_obj(SWFReader *read, u32 revision)
{
	SGCommand *com;
	CommandFieldInfo *f;
	u32 ID, bitsize, ratio;
	u32 clip_depth;
	M4Matrix2D mat;
	M4ColorMatrix cmat;
	SFNode *shape, *par;
	DispShape *ds;
	char *name;
	char szDEF[100];
	u32 depth, type;
	Bool had_depth, is_sprite;
	/*SWF flags*/
	Bool has_clip, has_name, has_ratio, has_cmat, has_mat, has_id, has_move;
	
	name = NULL;
	clip_depth = 0;
	ID = 0;
	depth = 0;
	has_clip = has_name = has_ratio = has_cmat = has_mat = has_id = has_move = 0;

	cmat_init(&cmat);
	mx2d_init(mat);
	/*place*/
	type = SWF_PLACE;

	/*SWF 1.0*/
	if (revision==0) {
		ID = swf_get_16(read);
		has_id = 1;
		depth = swf_get_16(read);
		bitsize = 32;
		bitsize += swf_get_matrix(read, &mat, 0);
		has_mat = 1;
		/*size exceeds matrix, parse col mat*/
		if (bitsize < read->size*8) {
			swf_get_colormatrix(read, &cmat);
			has_cmat = 1;
			swf_align(read);
		}
	}
	/*SWF 3.0*/
	else if (revision==1) {
		/*reserved*/
		swf_read_int(read, 1);
		has_clip = swf_read_int(read, 1);
		has_name = swf_read_int(read, 1);
		has_ratio = swf_read_int(read, 1);
		has_cmat = swf_read_int(read, 1);
		has_mat = swf_read_int(read, 1);
		has_id = swf_read_int(read, 1);
		has_move = swf_read_int(read, 1);

		depth = swf_get_16(read);
		if (has_id) ID = swf_get_16(read);
		if (has_mat) {
			swf_get_matrix(read, &mat, 0);
			swf_align(read);
		}
		if (has_cmat) {
			swf_get_colormatrix(read, &cmat);
			swf_align(read);
		}
		if (has_ratio) ratio = swf_get_16(read);
		if (has_clip) clip_depth = swf_get_16(read);

		if (has_name) {
			name = swf_get_string(read);
			free(name);
		}
		/*replace*/
		if (has_id && has_move) type = SWF_REPLACE;
		/*move*/
		else if (!has_id && has_move) type = SWF_MOVE;
		/*place*/
		else type = SWF_PLACE;
	}

	if (clip_depth) {
		swf_report(read, M4NotSupported, "Clipping not supported - ignoring");
		return M4OK;
	}

	/*1: check depth of display list*/
	had_depth = SWF_CheckDepth(read, depth);
	/*check validity*/
	if ((type==SWF_MOVE) && !had_depth) swf_report(read, M4BadParam, "Accessing empty depth level %d", depth);

	ds = NULL;
	shape = NULL;
	/*usual case: (re)place depth level*/
	switch (type) {
	case SWF_MOVE:
		ds = SWF_GetDepthEntry(read, depth, 0);
		shape = ds ? ds->n : NULL;
		break;
	case SWF_REPLACE:
	case SWF_PLACE:
	default:
		assert(has_id);
		shape = SWF_GetNode(read, ID);
		break;
	}

	is_sprite = 0;
	if (!shape) {
		/*this may be a sprite*/
		if (type != SWF_MOVE) {
			sprintf(szDEF, "Sprite%d_root", ID);
			shape = SG_FindNodeByName(read->load->scene_graph, szDEF);
			if (shape) is_sprite = 1;
		}
		if (!shape) {
			swf_report(read, M4BadParam, "%s unfound object (ID %d)", (type==SWF_MOVE) ? "Moving" : ((type==SWF_PLACE) ? "Placing" : "Replacing"), ID);
			return M4OK;
		}
	}
	/*restore prev matrix if needed*/
	if (type==SWF_REPLACE) {
		if (!ds) ds = SWF_GetDepthEntry(read, depth, 0);
		if (ds) {
			if (!has_mat) {
				memcpy(&mat, &ds->mat, sizeof(M4Matrix2D));
				has_mat = 1;
			}
			if (!has_cmat) {
				memcpy(&cmat, &ds->cmat, sizeof(M4ColorMatrix));
				has_cmat = 1;
			}
		}
	}

	/*check for identity matrices*/
	if (has_cmat && cmat.identity) has_cmat = 0;
	if (has_mat && swf_mat_is_identity(&mat)) has_mat = 0;


	/*then add cmat/mat and node*/
	par = NULL;
	if (!has_mat && !has_cmat) {
		par = shape;
	} else {
		if (has_mat) par = SWF_GetBIFSMatrix(read, &mat);
		if (has_cmat) {
			SFNode *cm = SWF_GetBIFSColorMatrix(read, &cmat);
			if (!par) {
				par = cm;
				Node_InsertChild(par, shape, -1);
				Node_Register(shape, par);
			} else {
				Node_InsertChild(par, cm, -1);
				Node_Register(cm, par);
				Node_InsertChild(cm, shape, -1);
				Node_Register(shape, cm);
			}
		} else {
			Node_InsertChild(par, shape, -1);
			Node_Register(shape, par);
		}
	}
	/*store in display list*/
	ds = SWF_GetDepthEntry(read, depth, 1);
	ds->n = shape;
	/*remember matrices*/
	memcpy(&ds->mat, &mat, sizeof(M4Matrix2D));
	memcpy(&ds->cmat, &cmat, sizeof(M4ColorMatrix));

	/*and write command*/
	com = SG_NewCommand(read->load->scene_graph, SG_IndexedReplace);
	/*in sprite definiton, modify at sprite root level*/
	if (read->current_sprite_id) {
		sprintf(szDEF, "Sprite%d_root", read->current_sprite_id);
		com->node = SG_FindNodeByName(read->load->scene_graph, szDEF);
		depth = 0;
	} else {
		com->node = SG_FindNodeByName(read->load->scene_graph, "DISPLAYLIST");
	}
	Node_Register(com->node, NULL);
	f = SG_NewFieldCommand(com);
	f->field_ptr = &f->new_node;
	f->fieldType = FT_SFNode;
	f->pos = depth;
	f->fieldIndex = 2;	/*children index*/
	f->new_node = par;
	Node_Register(f->new_node, com->node);
	ChainAddEntry(read->bifs_au->commands, com);

	/*starts anim*/
	if (is_sprite) {
		sprintf(szDEF, "Sprite%d_ctrl", ID);
		com = SG_NewCommand(read->load->scene_graph, SG_FieldReplace);
		com->node = SG_FindNodeByName(read->load->scene_graph, szDEF);
		Node_Register(com->node, NULL);
		f = SG_NewFieldCommand(com);
		f->field_ptr = VRML_NewFieldPointer(FT_SFTime);
		*(SFTime *)f->field_ptr = ((Double)read->bifs_au->timing) / read->bifs_es->timeScale;
		f->fieldType = FT_SFTime;
		f->fieldIndex = 2;	/*startTime index*/
		ChainAddEntry(read->bifs_au->commands, com);
	}
	return M4OK;
}

M4Err swf_remove_obj(SWFReader *read, u32 revision)
{
	SGCommand *com;
	CommandFieldInfo *f;
	DispShape *ds;
	u32 depth;
	if (revision==0) swf_get_16(read);
	depth = swf_get_16(read);
	ds = SWF_GetDepthEntry(read, depth, 0);
	/*this happens if a placeObject has failed*/
	if (!ds) return M4OK;
	ds->n = NULL;

	com = SG_NewCommand(read->load->scene_graph, SG_IndexedReplace);
	com->node = SG_FindNodeByName(read->load->scene_graph, "DISPLAYLIST");
	Node_Register(com->node, NULL);
	f = SG_NewFieldCommand(com);
	f->field_ptr = &f->new_node;
	f->fieldType = FT_SFNode;
	f->pos = depth;
	f->fieldIndex = 2;	/*children index*/
	f->new_node = SG_FindNodeByName(read->load->scene_graph, "EMPTYSHAPE");
	Node_Register(f->new_node, com->node);
	ChainAddEntry(read->bifs_au->commands, com);
	return M4OK;
}

M4Err swf_show_frame(SWFReader *read)
{
	u32 ts;
	Bool is_rap;

	/*hack to allow for empty BIFS AU to be encoded in order to keep the frame-rate (this reduces MP4 table size...)*/
	if (0 && !ChainGetCount(read->bifs_au->commands)) {
		SGCommand *com;
		CommandFieldInfo *f;
		com = SG_NewCommand(read->load->scene_graph, SG_FieldReplace);
		com->node = SG_FindNodeByName(read->load->scene_graph, "DICTIONARY");
		Node_Register(com->node, NULL);
		f = SG_NewFieldCommand(com);
		f->field_ptr = VRML_NewFieldPointer(FT_SFInt32);
		f->fieldType = FT_SFInt32;
		f->fieldIndex = 1;	/*whichCoice index*/
		/*replace by same value*/
		*((SFInt32 *)f->field_ptr) = -1;
		ChainAddEntry(read->bifs_au->commands, com);
	}

	/*flush AU*/
	read->current_frame ++;
	ts = read->current_frame * 100;
	is_rap = read->current_sprite_id ? 1 : 0;
	/*if we use ctrl stream, same thing*/
	read->bifs_au = M4SM_NewAU(read->bifs_es, ts, 0, 
				/*all frames in sprites are RAP (define is not allowed in sprites)
				if we use a ctrl stream, all AUs are RAP (defines are placed in a static dictionary)
				*/
				(read->current_sprite_id || (read->flags & M4SWF_SplitTimeline)) ? 1 : 0);
	return M4OK;
}

M4Err swf_def_font(SWFReader *read, u32 revision)
{
	u32 i, count;
	M4Err e;
	SFNode *glyph;
	SWFFont *ft;
	u32 *offset_table;
	u32 start;
	ft = malloc(sizeof(SWFFont));
	memset(ft, 0, sizeof(SWFFont));
	ft->glyphs = NewChain();
	ft->fontID = swf_get_16(read);
	e = M4OK;


	if (revision==0) {
		u32 count;

		start = swf_get_file_pos(read);

		count = swf_get_16(read);
		ft->nbGlyphs = count / 2;
		offset_table = malloc(sizeof(u32) * ft->nbGlyphs);
	    offset_table[0] = 0;
		for (i=1; i<ft->nbGlyphs; i++) offset_table[i] = swf_get_16(read);

		for (i=0; i<ft->nbGlyphs; i++) {
			swf_align(read);
			e = swf_seek_file_to(read, start + offset_table[i]);
			if (e) break;
			while (1) {
				glyph = swf_parse_shape_def(read, 0, 0);
				/*not a mistake, that's likelly space char*/
				if (!glyph) glyph = SWF_NewNode(read, TAG_MPEG4_Shape);
				ChainAddEntry(ft->glyphs, glyph);
				Node_Register(glyph, NULL);
				break;
			}
		}
		free(offset_table);
		if (e) return e;
	} else if (revision==1) {
		SWFRec rc;
		Bool wide_offset, wide_codes;
		ft->has_layout = swf_read_int(read, 1);
		ft->has_shiftJIS = swf_read_int(read, 1);
		ft->is_unicode = swf_read_int(read, 1);
		ft->is_ansi = swf_read_int(read, 1);
		wide_offset = swf_read_int(read, 1);
		wide_codes = swf_read_int(read, 1);
		ft->is_italic = swf_read_int(read, 1);
		ft->is_bold = swf_read_int(read, 1);
		swf_read_int(read, 8);
		count = swf_read_int(read, 8);
		ft->fontName = malloc(sizeof(u8)*count+1);
		ft->fontName[count] = 0;
		for (i=0; i<count; i++) ft->fontName[i] = swf_read_int(read, 8);

		ft->nbGlyphs = swf_get_16(read);
		start = swf_get_file_pos(read);

		if (ft->nbGlyphs) {
			u32 code_offset, checkpos;

			offset_table = malloc(sizeof(u32) * ft->nbGlyphs);
			for (i=0; i<ft->nbGlyphs; i++) {
				if (wide_offset) offset_table[i] = swf_get_32(read);
				else offset_table[i] = swf_get_16(read);
			}
			
			if (wide_offset) {
				code_offset = swf_get_32(read);
			} else {
				code_offset = swf_get_16(read);
			}

			for (i=0; i<ft->nbGlyphs; i++) {
				swf_align(read);
				e = swf_seek_file_to(read, start + offset_table[i]);
				if (e) break;
				while (1) {
					glyph = swf_parse_shape_def(read, 0, 0);
					if (!glyph) continue;
					ChainAddEntry(ft->glyphs, glyph);
					Node_Register(glyph, NULL);
					break;
				}
			}
			free(offset_table);
			if (e) return e;

			checkpos = swf_get_file_pos(read);
			if (checkpos != start + code_offset) {
				fprintf(stdout, "ERROR: BAD CODE OFFSET\n");
				return M4NonCompliantBitStream;
			}

			ft->glyph_codes = malloc(sizeof(u16) * ft->nbGlyphs);
			for (i=0; i<ft->nbGlyphs; i++) {
				if (wide_codes) ft->glyph_codes[i] = swf_get_16(read);
				else ft->glyph_codes[i] = swf_read_int(read, 8);
			}
		}
		if (ft->has_layout) {
			ft->ascent = swf_get_s16(read);
			ft->descent = swf_get_s16(read);
			ft->leading = swf_get_s16(read);
			if (ft->nbGlyphs) {
				ft->glyph_adv = malloc(sizeof(s16) * ft->nbGlyphs);
				for (i=0; i<ft->nbGlyphs; i++) ft->glyph_adv[i] = swf_get_s16(read);
				for (i=0; i<ft->nbGlyphs; i++) swf_get_rec(read, &rc);
			}
			/*kerning info*/
			count = swf_get_16(read);
			for (i=0; i<count; i++) {
				if (wide_codes) {
					swf_get_16(read);
					swf_get_16(read);
				} else {
					swf_read_int(read, 8);
					swf_read_int(read, 8);
				}
				swf_get_s16(read);
			}
		}
	}

	ChainAddEntry(read->fonts, ft);
	return M4OK;
}

M4Err swf_def_font_info(SWFReader *read)
{
	SWFFont *ft;
	Bool wide_chars;
	u32 i, count;
	
	i = swf_get_16(read);
	ft = SWF_FindFont(read, i);
	if (!ft) {
		swf_report(read, M4BadParam, "Cannot locate font ID %d", i);
		return M4BadParam;
	}
	/*overwrite font info*/
	if (ft->fontName) free(ft->fontName);
	count = swf_read_int(read, 8);
	ft->fontName = malloc(sizeof(char) * (count+1));
	ft->fontName[count] = 0;
	for (i=0; i<count; i++) ft->fontName[i] = swf_read_int(read, 8);
	swf_read_int(read, 2);
	ft->is_unicode = swf_read_int(read, 1);
	ft->has_shiftJIS = swf_read_int(read, 1);
	ft->is_ansi = swf_read_int(read, 1);
	ft->is_italic = swf_read_int(read, 1);
	ft->is_bold = swf_read_int(read, 1);
	/*TODO - this should be remapped to a font data stream, we currently only assume the glyph code
	table is the same as the original font file...*/
	wide_chars = swf_read_int(read, 1);
	if (ft->glyph_codes) free(ft->glyph_codes);
	ft->glyph_codes = malloc(sizeof(u16) * ft->nbGlyphs);

	for (i=0; i<ft->nbGlyphs; i++) {
		if (wide_chars) ft->glyph_codes[i] = swf_get_16(read);
		else ft->glyph_codes[i] = swf_read_int(read, 8);
	}
	return M4OK;
}

M4Err swf_def_text(SWFReader *read, u32 revision)
{
	SWFRec rc;
	SWFText txt;
	Bool flag;
	SFNode *n;
	u32 ID, nbits_adv, nbits_glyph, i, col, fontID, count;
	Float offX, offY, fontHeight;
	M4Err e;

	ID = swf_get_16(read);
	swf_get_rec(read, &rc);
	swf_get_matrix(read, &txt.mat, 0);
	txt.text = NewChain();

	swf_align(read);
	nbits_glyph = swf_read_int(read, 8);
	nbits_adv = swf_read_int(read, 8);
	fontID = 0;
	offX = offY = fontHeight = 0;
	col = 0xFF000000;
	e = M4OK;

	while (1) {
		flag = swf_read_int(read, 1);
		/*regular glyph record*/
		if (!flag) {
			SWFGlyphRec *gr;
			count = swf_read_int(read, 7);
			if (!count) break;

			if (!fontID) {
				e = M4BadParam;
				swf_report(read, M4BadParam, "Defining text %d without assigning font", fontID);
				break;
			}

			gr = malloc(sizeof(SWFGlyphRec));
			memset(gr, 0, sizeof(SWFGlyphRec));
			ChainAddEntry(txt.text, gr);
			gr->fontID = fontID;
			gr->fontHeight = fontHeight;
			gr->col = col;
			gr->orig_x = offX;
			gr->orig_y = offY;
			gr->nbGlyphs = count;
			gr->indexes = malloc(sizeof(u32) * gr->nbGlyphs);
			gr->dx = malloc(sizeof(Float) * gr->nbGlyphs);
			for (i=0; i<gr->nbGlyphs; i++) {
				gr->indexes[i] = swf_read_int(read, nbits_glyph);
				gr->dx[i] = swf_read_int(read, nbits_adv) * SWF_TWIP_SCALE;
			}
			swf_align(read);
		}
		/*text state change*/
		else {
			Bool has_font, has_col, has_y_off, has_x_off;
			/*reserved*/
			swf_read_int(read, 3);
			has_font = swf_read_int(read, 1);
			has_col = swf_read_int(read, 1);
			has_y_off = swf_read_int(read, 1);
			has_x_off = swf_read_int(read, 1);
			
			/*end of rec*/
			if (!has_font && !has_col && !has_y_off && !has_x_off) break;
			if (has_font) fontID = swf_get_16(read);
			if (has_col) {
				if (revision==0) col = swf_get_color(read);
				else col = swf_get_argb(read);
			}
			/*openSWF spec seems to have wrong order here*/
			if (has_x_off) offX = swf_get_s16(read) * SWF_TWIP_SCALE;
			if (has_y_off) offY = swf_get_s16(read) * SWF_TWIP_SCALE;
			if (has_font) fontHeight = swf_get_16(read) * SWF_TEXT_SCALE;
		}
	}

	if (e) goto exit;

	if (! (read->flags & M4SWF_NoText) ) {
		n = SWFTextToBIFS(read, &txt);
		if (n) {
			char szDEF[1024];
			sprintf(szDEF, "Text%d", ID);
			read->load->ctx->max_node_id++;
			ID = read->load->ctx->max_node_id;
			Node_SetDEF(n, ID, szDEF);
			SWF_InsertNode(read, n);
		}
	}

exit:
	while (ChainGetCount(txt.text)) {
		SWFGlyphRec *gr = ChainGetEntry(txt.text, 0);
		ChainDeleteEntry(txt.text, 0);
		if (gr->indexes) free(gr->indexes);
		if (gr->dx) free(gr->dx);
		free(gr);
	}
	DeleteChain(txt.text);

	return e;
}

M4Err swf_init_od(SWFReader *read)
{
	ESDescriptor *esd;
	if (read->od_es) return M4OK;
	read->od_es = M4SM_NewStream(read->load->ctx, 2, 1, 1);
	if (!read->od_es) return M4OutOfMem;
	if (!read->load->ctx->root_od) {
		BIFSConfigDescriptor *bc;
		read->load->ctx->root_od = (ObjectDescriptor *) OD_NewDescriptor(InitialObjectDescriptor_Tag);
		/*add BIFS stream*/
		esd = (ESDescriptor *) OD_NewESDescriptor(0);
		if (!esd) return M4OutOfMem;
		esd->decoderConfig->streamType = M4ST_SCENE;
		esd->decoderConfig->objectTypeIndication = 1;
		esd->slConfig->timestampResolution = read->bifs_es->timeScale;
		esd->ESID = 1;
		ChainAddEntry(read->load->ctx->root_od->ESDescriptors, esd);
		read->load->ctx->root_od->objectDescriptorID = 1;
		OD_DeleteDescriptor((Descriptor **) &esd->decoderConfig->decoderSpecificInfo);
		bc = (BIFSConfigDescriptor *) OD_NewDescriptor(BIFSConfig_Tag);
		bc->isCommandStream = 1;
		bc->pixelMetrics = 1;
		bc->pixelWidth = (u16) read->width;
		bc->pixelHeight = (u16) read->height;
		esd->decoderConfig->decoderSpecificInfo = (DefaultDescriptor *) bc;
	}
	if (!read->load->ctx->root_od) return M4OutOfMem;
	esd = (ESDescriptor *) OD_NewESDescriptor(0);
	if (!esd) return M4OutOfMem;
	esd->decoderConfig->streamType = M4ST_OD;
	esd->decoderConfig->objectTypeIndication = 1;
	esd->slConfig->timestampResolution = read->od_es->timeScale = read->bifs_es->timeScale;
	esd->ESID = 2;
	esd->OCRESID = 1;
	OD_DeleteDescriptor((Descriptor **) &esd->decoderConfig->decoderSpecificInfo);
	return ChainAddEntry(read->load->ctx->root_od->ESDescriptors, esd);
}

M4Err swf_insert_od(SWFReader *read, u32 at_time, ObjectDescriptor *od)
{
	u32 i;
	ObjectDescriptorUpdate *com;
	read->od_au = M4SM_NewAU(read->od_es, at_time, 0, 1);
	if (!read->od_au) return M4OutOfMem;

	for (i=0; i<ChainGetCount(read->od_au->commands); i++) {
		com = ChainGetEntry(read->od_au->commands, i);
		if (com->tag == ODUpdate_Tag) {
			ChainAddEntry(com->objectDescriptors, od);
			return M4OK;
		}
	}
	com = (ObjectDescriptorUpdate *) OD_NewCommand(ODUpdate_Tag);
	ChainAddEntry(com->objectDescriptors, od);
	return ChainAddEntry(read->od_au->commands, com);
}


void swf_delete_sound_stream(SWFReader *read)
{
	if (!read->sound_stream) return;
	if (read->sound_stream->output) fclose(read->sound_stream->output);
	if (read->sound_stream->szFileName) free(read->sound_stream->szFileName);
	free(read->sound_stream);
	read->sound_stream = NULL;
}



M4Err swf_def_sprite(SWFReader *read)
{
	M4Err SWF_ParseTag(SWFReader *read);
	M4Err e;
	ObjectDescriptor *od;
	ESDescriptor *esd;
	u32 spriteID, ID;
	u32 frame_count;
	Bool prev_sprite;
	u32 prev_frame;
	SFNode *n, *par;
	FieldInfo info;
	char szDEF[100];
	SWFSound *snd;

	M4StreamContext *prev_sc;
	M4AUContext *prev_au;

	spriteID = swf_get_16(read);
	frame_count = swf_get_16(read);

	/*init OD*/
	e = swf_init_od(read);
	if (e) return e;

	/*create animationStream object*/
	od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
	if (!od) return M4OutOfMem;

	od->objectDescriptorID = swf_get_od_id(read);
	esd = (ESDescriptor *) OD_NewESDescriptor(0);
	if (!esd) return M4OutOfMem;
	esd->ESID = swf_get_es_id(read);
	/*sprite runs on its own timeline*/
	esd->OCRESID = esd->ESID;
	/*always depends on main scene*/
	esd->dependsOnESID = 1;
	esd->decoderConfig->streamType = M4ST_SCENE;
	esd->decoderConfig->objectTypeIndication = 1;
	esd->slConfig->timestampResolution = read->bifs_es->timeScale;
	OD_DeleteDescriptor((Descriptor **) &esd->decoderConfig->decoderSpecificInfo);
	ChainAddEntry(od->ESDescriptors, esd);

	/*by default insert OD at begining*/
	e = swf_insert_od(read, 0, od);
	if (e) {
		OD_DeleteDescriptor((Descriptor **) &od);
		return e;
	}

	/*create AS for sprite - all AS are created in initial scene replace*/
	n = SWF_NewNode(read, TAG_MPEG4_AnimationStream);
	sprintf(szDEF, "Sprite%d_ctrl", spriteID);

	read->load->ctx->max_node_id++;
	ID = read->load->ctx->max_node_id;
	Node_SetDEF(n, ID, szDEF);

	Node_InsertChild(read->root, n, 0);
	Node_Register(n, read->root);
	/*assign URL*/
	Node_GetFieldByName(n, "url", &info);
	VRML_MF_Alloc(info.far_ptr, info.fieldType, 1);
	((MFURL*)info.far_ptr)->vals[0].OD_ID = od->objectDescriptorID;
	/*inactive by default (until inserted)*/
	((M_AnimationStream *)n)->startTime = -1;
	/*loop by default - not 100% sure from SWF spec, I believe a sprite loops until removed from DList*/
	((M_AnimationStream *)n)->loop = 1;

	/*create sprite grouping node*/
	n = SWF_NewNode(read, TAG_MPEG4_Group);
	sprintf(szDEF, "Sprite%d_root", spriteID);

	read->load->ctx->max_node_id++;
	ID = read->load->ctx->max_node_id;
	Node_SetDEF(n, ID, szDEF);
	par = SG_FindNodeByName(read->load->scene_graph, "DICTIONARY");
	assert(par);
	ChainAddEntry(((M_Switch *)par)->choice, n);
	Node_Register(n, par);
	par = SG_FindNodeByName(read->load->scene_graph, "EMPTYSHAPE");
	Node_InsertChild(n, par, -1);
	Node_Register(par, n);

	/*store BIFS context*/
	prev_frame = read->current_frame;
	prev_sc = read->bifs_es;
	prev_au = read->bifs_au;
	prev_sprite = read->current_sprite_id;
	/*create new BIFS stream*/
	read->bifs_es = M4SM_NewStream(read->load->ctx, esd->ESID, M4ST_SCENE, 1);
	read->bifs_es->timeScale = prev_sc->timeScale;
	read->current_frame = 0;
	/*create first AU*/
	read->bifs_au = M4SM_NewAU(read->bifs_es, 0, 0, 1);
	read->current_sprite_id = spriteID;
	/*store soundStream*/
	snd = read->sound_stream;
	read->sound_stream = NULL;

	/*and parse*/
	while (1) {
		e = SWF_ParseTag(read);
		if (e<0) return e;
		/*done with sprite*/
		if (read->tag==SWF_END) break;
	}
	/*restore BIFS context*/
	read->current_frame = prev_frame;
	read->bifs_es = prev_sc;
	read->bifs_au = prev_au;
	read->current_sprite_id = prev_sprite;

	/*close sprite soundStream*/
	swf_delete_sound_stream(read);
	/*restore sound stream*/
	read->sound_stream = snd;

	read->tag = SWF_DEFINESPRITE;
	return M4OK;
}

M4Err swf_def_sound(SWFReader *read)
{
	SWFSound *snd;
	snd = malloc(sizeof(SWFSound));
	memset(snd, 0, sizeof(SWFSound));
	snd->ID = swf_get_16(read);
	snd->format = swf_read_int(read, 4);
	snd->sound_rate = swf_read_int(read, 2);
	snd->bits_per_sample = swf_read_int(read, 1) ? 16 : 8;
	snd->stereo = swf_read_int(read, 1);
	snd->sample_count = swf_get_32(read);

	switch (snd->format) {
	/*raw PCM*/
	case 0:
		swf_report(read, M4NotSupported, "Raw PCM Audio not supported");
		free(snd);
		break;
	/*ADPCM*/
	case 1:
		swf_report(read, M4NotSupported, "AD-PCM Audio not supported");
		free(snd);
		break;
	/*MP3*/
	case 2:
	{
		unsigned char bytes[4];
		char szName[1024];
		u32 hdr, alloc_size, size, tot_size;
		char *frame;

		sprintf(szName, "swf_sound_%d.mp3", snd->ID);
		if (read->localPath) {
			snd->szFileName = malloc(sizeof(char)*M4_MAX_PATH);
			strcpy(snd->szFileName, read->localPath);
			strcat(snd->szFileName, szName);
		} else {
			snd->szFileName = strdup(szName);
		}
		snd->output = fopen(snd->szFileName, "wb");

		alloc_size = 1;
		frame = malloc(sizeof(char));
		snd->frame_delay_ms = swf_get_16(read);
		snd->frame_delay_ms = 0;
		tot_size = 9;
		/*parse all frames*/
		while (1) {
			bytes[0] = swf_read_int(read, 8);
			bytes[1] = swf_read_int(read, 8);
			bytes[2] = swf_read_int(read, 8);
			bytes[3] = swf_read_int(read, 8);
			hdr = FOUR_CHAR_INT(bytes[0], bytes[1], bytes[2], bytes[3]);
			size = MP3_GetFrameSize(hdr);
			if (alloc_size<size-4) {
				frame = realloc(frame, sizeof(char)*(size-4));
				alloc_size = size-4;
			}
			/*watchout for truncated framesif */
			if (tot_size + size >= read->size) size = read->size - tot_size;

			swf_read_data(read, frame, size-4);
			fwrite(bytes, sizeof(char)*4, 1, snd->output);
			fwrite(frame, sizeof(char)*(size-4), 1, snd->output);
			if (tot_size + size >= read->size) break;
			tot_size += size;
		}
		free(frame);
		return ChainAddEntry(read->sounds, snd);
	}
	case 3:
		swf_report(read, M4NotSupported, "Unrecognized sound format");
		free(snd);
		break;
	}
	return M4OK;
}


typedef struct
{
	u32 sync_flags;
	u32 in_point, out_point;
	u32 nb_loops;
} SoundInfo; 

SoundInfo swf_skip_soundinfo(SWFReader *read)
{
	SoundInfo si;
	u32 sync_flags = swf_read_int(read, 4);
	Bool has_env = swf_read_int(read, 1);
	Bool has_loops = swf_read_int(read, 1);
	Bool has_out_pt = swf_read_int(read, 1);
	Bool has_in_pt = swf_read_int(read, 1);

	memset(&si, 0, sizeof(SoundInfo));
	si.sync_flags = sync_flags;
	if (has_in_pt) si.in_point = swf_get_32(read);
	if (has_out_pt) si.out_point = swf_get_32(read);
	if (has_loops) si.nb_loops = swf_get_16(read);
	/*we ignore the envelope*/
	if (has_env) {
		u32 i;
		u32 nb_ctrl = swf_read_int(read, 8);
		for (i=0; i<nb_ctrl; i++) {
			swf_read_int(read, 32);	/*mark44*/
			swf_read_int(read, 16);	/*l0*/
			swf_read_int(read, 16);	/*l1*/
		}
	}
	return si;
}

SWFSound *sndswf_get_sound(SWFReader *read, u32 ID)
{
	u32 i;
	for (i=0; i<ChainGetCount(read->sounds); i++) {
		SWFSound *snd = ChainGetEntry(read->sounds, i);
		if (snd->ID==ID) return snd;
	}
	return NULL;
}


M4Err swf_setup_sound(SWFReader *read, SWFSound *snd)
{
	M4Err e;
	ObjectDescriptor *od;
	ESDescriptor *esd;
	MuxInfoDescriptor *mux;
	SFNode *n, *par;
	FieldInfo info;
	u32 ID;
	char szDEF[100];

	e = swf_init_od(read);
	if (e) return e;

	/*create audio object*/
	od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
	if (!od) return M4OutOfMem;
	od->objectDescriptorID = swf_get_od_id(read);
	esd = (ESDescriptor *) OD_NewDescriptor(ESDescriptor_Tag);
	if (!esd) return M4OutOfMem;
	esd->ESID = swf_get_es_id(read);
	if (snd->ID) {
		/*sound runs on its own timeline*/
		esd->OCRESID = esd->ESID;
	} else {
		/*soundstream runs on movie/sprite timeline*/
		esd->OCRESID = read->bifs_es->ESID;
	}
	ChainAddEntry(od->ESDescriptors, esd);

	/*setup mux info*/
	mux = (MuxInfoDescriptor*)OD_NewDescriptor(MuxInfoDescriptor_Tag);
	mux->file_name = strdup(snd->szFileName);
	mux->startTime = snd->frame_delay_ms;
	/*MP3 in, destroy file once done*/
	if (snd->format==2) mux->delete_file = 1;
	ChainAddEntry(esd->extensionDescriptors, mux);


	/*by default insert OD at begining*/
	e = swf_insert_od(read, 0, od);
	if (e) {
		OD_DeleteDescriptor((Descriptor **) &od);
		return e;
	}
	/*create sound & audio clip*/
	n = SWF_NewNode(read, TAG_MPEG4_Sound2D);
	Node_InsertChild(read->root, n, 0);
	Node_Register(n, read->root);
	par = n;
	n = SWF_NewNode(read, TAG_MPEG4_AudioClip);
	/*soundStream doesn't have ID and doesn't need to be accessed*/
	if (snd->ID) {
		sprintf(szDEF, "Sound%d", snd->ID);
		read->load->ctx->max_node_id++;
		ID = read->load->ctx->max_node_id;
		Node_SetDEF(n, ID, szDEF);
	}
	((M_Sound2D *)par)->source = n;
	Node_Register(n, par);
	/*assign URL*/
	Node_GetFieldByName(n, "url", &info);
	VRML_MF_Alloc(info.far_ptr, info.fieldType, 1);
	((MFURL *)info.far_ptr)->vals[0].OD_ID = od->objectDescriptorID;

	snd->is_setup = 1;
	return M4OK;
}

M4Err swf_start_sound(SWFReader *read)
{
	char szDEF[100];
	SGCommand *com;
	FieldInfo info;
	SFNode *sound2D;
	CommandFieldInfo *f;
	SWFSound *snd;
	u32 ID = swf_get_16(read);
	SoundInfo si = swf_skip_soundinfo(read);

	snd = sndswf_get_sound(read, ID);
	if (!snd) {
		swf_report(read, M4BadParam, "Cannot find sound with ID %d", ID);
		return M4OK;
	}
	if (!snd->is_setup) {
		M4Err e = swf_setup_sound(read, snd);
		if (e) return e;
	}

	sprintf(szDEF, "Sound%d", snd->ID);
	sound2D = SG_FindNodeByName(read->load->scene_graph, szDEF);
	/*check flags*/
	if (si.sync_flags & 0x2) {
		/*need a STOP*/
		com = SG_NewCommand(read->load->scene_graph, SG_FieldReplace);
		com->node = sound2D;
		Node_Register(com->node, NULL);
		Node_GetFieldByName(sound2D, "stopTime", &info);
		f = SG_NewFieldCommand(com);
		f->field_ptr = VRML_NewFieldPointer(FT_SFTime);
		f->fieldType = FT_SFTime;
		f->fieldIndex = info.fieldIndex;
		/*replace by "now"*/
		*(SFTime *)f->field_ptr = ((Double)read->bifs_au->timing) / read->bifs_es->timeScale;
		*(SFTime *)f->field_ptr = 0;
		ChainAddEntry(read->bifs_au->commands, com);
	}

	com = SG_NewCommand(read->load->scene_graph, SG_FieldReplace);
	com->node = sound2D;
	Node_Register(com->node, NULL);
	Node_GetFieldByName(sound2D, "startTime", &info);
	f = SG_NewFieldCommand(com);
	f->field_ptr = VRML_NewFieldPointer(FT_SFTime);
	f->fieldType = FT_SFTime;
	f->fieldIndex = info.fieldIndex;
	/*replace by "now"*/
	*(SFTime *)f->field_ptr = ((Double)read->bifs_au->timing) / read->bifs_es->timeScale;
	*(SFTime *)f->field_ptr = 0;
	ChainAddEntry(read->bifs_au->commands, com);

	return M4OK;
}

M4Err swf_soundstream_hdr(SWFReader *read)
{
	u8 rec_mix;
	u32 samplesperframe;
	SWFSound *snd;

	if (read->sound_stream) {
		swf_report(read, M4BadParam, "More than one sound stream for current timeline!!");
		return swf_func_skip(read);
	}
	
	snd = malloc(sizeof(SWFSound));
	memset(snd, 0, sizeof(SWFSound));

	rec_mix = swf_read_int(read, 8);
	/*0: uncompressed, 1: ADPCM, 2: MP3*/
	snd->format = swf_read_int(read, 4);
	/*0: 5.5k, 1: 11k, 2: 2: 22k, 3: 44k*/
	snd->sound_rate = swf_read_int(read, 2);
	/*0: 8 bit, 1: 16 bit*/
	snd->bits_per_sample = swf_read_int(read, 1) ? 16 : 8;
	/*0: mono, 8 1: stereo*/
	snd->stereo = swf_read_int(read, 1);
	/*samplesperframe hint*/
	samplesperframe = swf_read_int(read, 16);

	switch (snd->format) {
	/*raw PCM*/
	case 0:
		swf_report(read, M4NotSupported, "Raw PCM Audio not supported");
		free(snd);
		break;
	/*ADPCM*/
	case 1:
		swf_report(read, M4NotSupported, "AD-PCM Audio not supported");
		free(snd);
		break;
	/*MP3*/
	case 2:
		read->sound_stream = snd;
		break;
	case 3:
		swf_report(read, M4NotSupported, "Unrecognized sound format");
		free(snd);
		break;
	}
	return M4OK;
}

M4Err swf_soundstream_block(SWFReader *read)
{
	unsigned char bytes[4];
	u32 hdr, alloc_size, size, tot_size, samplesPerFrame, delay;
	char *frame;

	/*note we're doing only MP3*/
	if (!read->sound_stream) return swf_func_skip(read);

	samplesPerFrame = swf_get_16(read);
	delay = swf_get_16(read);

	if (!read->sound_stream->is_setup) {
		if (!read->sound_stream->szFileName) {
			char szName[1024];
			sprintf(szName, "swf_soundstream_%d.mp3", (u32) read->sound_stream);
			if (read->localPath) {
				read->sound_stream->szFileName = malloc(sizeof(char)*M4_MAX_PATH);
				strcpy(read->sound_stream->szFileName, read->localPath);
				strcat(read->sound_stream->szFileName, szName);
			} else {
				read->sound_stream->szFileName = strdup(szName);
			}
			read->sound_stream->output = fopen(read->sound_stream->szFileName, "wb");
		}
		/*error at setup*/
		if (!read->sound_stream->output) return swf_func_skip(read);
		/*store TS of first AU*/
		read->sound_stream->frame_delay_ms = read->current_frame*1000;
		read->sound_stream->frame_delay_ms /= read->frame_rate;
		read->sound_stream->frame_delay_ms = delay;
		swf_setup_sound(read, read->sound_stream);
	}

	if (!samplesPerFrame) return M4OK;

	alloc_size = 1;
	frame = malloc(sizeof(char));
	tot_size = 4;
	/*parse all frames*/
	while (1) {
		bytes[0] = swf_read_int(read, 8);
		bytes[1] = swf_read_int(read, 8);
		bytes[2] = swf_read_int(read, 8);
		bytes[3] = swf_read_int(read, 8);
		hdr = FOUR_CHAR_INT(bytes[0], bytes[1], bytes[2], bytes[3]);
		size = MP3_GetFrameSize(hdr);
		if (alloc_size<size-4) {
			frame = realloc(frame, sizeof(char)*(size-4));
			alloc_size = size-4;
		}
		/*watchout for truncated framesif */
		if (tot_size + size >= read->size) size = read->size - tot_size;

		swf_read_data(read, frame, size-4);
		fwrite(bytes, sizeof(char)*4, 1, read->sound_stream->output);
		fwrite(frame, sizeof(char)*(size-4), 1, read->sound_stream->output);
		if (tot_size + size >= read->size) break;
		tot_size += size;
	}
	free(frame);
	return M4OK;
}

M4Err swf_process_tag(SWFReader *read)
{
	switch (read->tag) {
	case SWF_END: return M4OK; /*void*/
	case SWF_PROTECT: return M4OK;
	case SWF_SETBACKGROUNDCOLOR: return swf_set_backcol(read);
	case SWF_DEFINESHAPE: return swf_def_shape(read, 0);
	case SWF_DEFINESHAPE2: return swf_def_shape(read, 1);
	case SWF_DEFINESHAPE3: return swf_def_shape(read, 2);
	case SWF_PLACEOBJECT: return swf_place_obj(read, 0);
	case SWF_PLACEOBJECT2: return swf_place_obj(read, 1);
	case SWF_SHOWFRAME: return swf_show_frame(read);
	case SWF_REMOVEOBJECT: return swf_remove_obj(read, 0);
	case SWF_REMOVEOBJECT2: return swf_remove_obj(read, 1);
	case SWF_DEFINEFONT: return swf_def_font(read, 0);
	case SWF_DEFINEFONT2: return swf_def_font(read, 1);
	case SWF_DEFINEFONTINFO: return swf_def_font_info(read);
	case SWF_DEFINETEXT: return swf_def_text(read, 0);
	case SWF_DEFINETEXT2: return swf_def_text(read, 1);
	case SWF_DEFINESPRITE: return swf_def_sprite(read);
	/*no revision needed*/
	case SWF_SOUNDSTREAMHEAD: 
	case SWF_SOUNDSTREAMHEAD2:
		return swf_soundstream_hdr(read);
	case SWF_DEFINESOUND: return swf_def_sound(read);
	case SWF_STARTSOUND: return swf_start_sound(read);
	case SWF_SOUNDSTREAMBLOCK: return swf_soundstream_block(read);

	case SWF_DEFINEBUTTONSOUND:
	case SWF_DEFINEBUTTON:
	case SWF_DEFINEBUTTON2:
	case SWF_DOACTION:
		read->has_interact = 1;
		return swf_func_skip(read);

/*	case SWF_DEFINEBITSJPEG: return swf_def_bits_jpeg(read);
	case SWF_JPEGTABLES: return swf_def_hdr_jpeg(read);
	case SWF_SOUNDSTREAMHEAD: return swf_sound_hdr(read);
	case SWF_SOUNDSTREAMBLOCK: return swf_sound_block(read);
	case SWF_DEFINEBITSLOSSLESS: return swf_def_bits_lossless(read);
	case SWF_DEFINEBITSJPEG2: return swf_def_bits_jpegV2(read);
	case SWF_DEFINEBITSJPEG3: return swf_def_bits_jpegV3(read);
	case SWF_DEFINEBITSLOSSLESS2: return swf_def_bits_losslessV2(read);
	case SWF_FRAMELABEL: return swf_def_frame_label(read);
*/	default: return swf_unknown_tag(read);
	}
}

M4Err SWF_ParseTag(SWFReader *read)
{
	M4Err e;
	s32 diff;
	u16 hdr;


	hdr = swf_get_16(read);
	read->tag = hdr>>6;
	read->size = hdr & 0x3f;
	if (read->size == 0x3f) {
		swf_align(read);
		read->size = swf_get_32(read);
	}
	diff = swf_get_file_pos(read) + read->size;
	
	e = swf_process_tag(read);
	swf_align(read);

	diff -= swf_get_file_pos(read);
	if (diff<0) {
		swf_report(read, M4IOErr, "tag over-read of %d bytes (size %d)", -1*diff, read->size);
		return M4IOErr;
	} else {
		swf_read_int(read, diff*8);
	}


	if (!e && !read->tag) return M4EOF;
	if (read->ioerr) {
		swf_report(read, M4IOErr, "bitstream IO err (tag size %d)", read->size);
		return read->ioerr;
	}
	return e;
}


void swf_report(SWFReader *read, M4Err e, char *format, ...)
{
	va_list args;
	va_start(args, format);
	if (read->load->OnMessage) {
		char szMsg[2048];
		char szMsgFull[2048];
		vsprintf(szMsg, format, args);
		if (e) {
			sprintf(szMsgFull, "(Frame %d TAG %s) %s", read->current_frame+1, swf_get_tag(read->tag), szMsg);
			read->load->OnMessage(read->load->cbk, szMsgFull, e);
		} else {
			read->load->OnMessage(read->load->cbk, szMsg, e);
		}
	} else {
		if (e) fprintf(stdout, "(Frame %d TAG %s) ", read->current_frame+1, swf_get_tag(read->tag));
		vfprintf(stdout, format, args);
		fprintf(stdout, "\n");
	}
	va_end(args);
}



const char *swf_get_tag(u32 tag)
{
	switch (tag) {
	case SWF_END: return "End";
	case SWF_SHOWFRAME: return "ShowFrame";
	case SWF_DEFINESHAPE: return "DefineShape";
	case SWF_FREECHARACTER: return "FreeCharacter";
	case SWF_PLACEOBJECT: return "PlaceObject";
	case SWF_REMOVEOBJECT: return "RemoveObject";
	case SWF_DEFINEBITSJPEG: return "DefineBitsJPEG";
	case SWF_DEFINEBUTTON: return "DefineButton";
	case SWF_JPEGTABLES: return "JPEGTables";
	case SWF_SETBACKGROUNDCOLOR: return "SetBackgroundColor";
	case SWF_DEFINEFONT: return "DefineFont";
	case SWF_DEFINETEXT: return "DefineText";
	case SWF_DOACTION: return "DoAction";
	case SWF_DEFINEFONTINFO: return "DefineFontInfo";
	case SWF_DEFINESOUND: return "DefineSound";
	case SWF_STARTSOUND: return "StartSound";
	case SWF_DEFINEBUTTONSOUND: return "DefineButtonSound";
	case SWF_SOUNDSTREAMHEAD: return "SoundStreamHead";
	case SWF_SOUNDSTREAMBLOCK: return "SoundStreamBlock";
	case SWF_DEFINEBITSLOSSLESS: return "DefineBitsLossless";
	case SWF_DEFINEBITSJPEG2: return "DefineBitsJPEG2";
	case SWF_DEFINESHAPE2: return "DefineShape2";
	case SWF_DEFINEBUTTONCXFORM: return "DefineButtonCXForm";
	case SWF_PROTECT: return "Protect";
	case SWF_PLACEOBJECT2: return "PlaceObject2";
	case SWF_REMOVEOBJECT2: return "RemoveObject2";
	case SWF_DEFINESHAPE3: return "DefineShape3";
	case SWF_DEFINETEXT2: return "DefineText2";
	case SWF_DEFINEBUTTON2: return "DefineButton2";
	case SWF_DEFINEBITSJPEG3: return "DefineBitsJPEG3";
	case SWF_DEFINEBITSLOSSLESS2: return "DefineBitsLossless2";
	case SWF_DEFINEEDITTEXT: return "DefineEditText";
	case SWF_DEFINEMOVIE: return "DefineMovie";
	case SWF_DEFINESPRITE: return "DefineSprite";
	case SWF_NAMECHARACTER: return "NameCharacter";
	case SWF_SERIALNUMBER: return "SerialNumber";
	case SWF_GENERATORTEXT: return "GeneratorText";
	case SWF_FRAMELABEL: return "FrameLabel";
	case SWF_SOUNDSTREAMHEAD2: return "SoundStreamHead2";
	case SWF_DEFINEMORPHSHAPE: return "DefineMorphShape";
	case SWF_DEFINEFONT2: return "DefineFont2";
	case SWF_TEMPLATECOMMAND: return "TemplateCommand";
	case SWF_GENERATOR3: return "Generator3";
	case SWF_EXTERNALFONT: return "ExternalFont";
	case SWF_EXPORTASSETS: return "ExportAssets";
	case SWF_IMPORTASSETS: return "ImportAssets";
	case SWF_ENABLEDEBUGGER: return "EnableDebugger";
	case SWF_MX0: return "MX0";
	case SWF_MX1: return "MX1";
	case SWF_MX2: return "MX2";
	case SWF_MX3: return "MX3";
	case SWF_MX4: return "MX4";
	default: return "UnknownTag";
	}
}



/*defines internal structure of the scene*/
M4Err SWF_InitContext(SWFReader *read)
{
	M4Err e;
	char szMsg[1000];
	ObjectDescriptor *od;
	ESDescriptor *esd;
	u32 ID;
	FieldInfo info;
	M4StreamContext *prev_sc;
	SGCommand *com;
	SFNode *n, *n2;

	/*create BIFS stream*/
	read->bifs_es = M4SM_NewStream(read->load->ctx, 1, M4ST_SCENE, 0x01);
	read->bifs_es->timeScale = read->frame_rate*100;

	read->bifs_au = M4SM_NewAU(read->bifs_es, 0, 0.0, 1);
	/*create scene replace command*/
	com = SG_NewCommand(read->load->scene_graph, SG_SceneReplace);
	read->load->ctx->scene_width = (u32) read->width;
	read->load->ctx->scene_height = (u32) read->height;
	read->load->ctx->is_pixel_metrics = 1;

	ChainAddEntry(read->bifs_au->commands, com);
	read->load->scene_graph = read->load->scene_graph;

	/*create base tree*/
	com->node = read->root = SWF_NewNode(read, TAG_MPEG4_OrderedGroup);
	Node_Register(read->root, NULL);

	/*hehehe*/
	n = SWF_NewNode(read, TAG_MPEG4_WorldInfo);
	Node_InsertChild(read->root, n, -1);
	Node_Register(n, read->root);
	((M_WorldInfo *)n)->title.buffer = strdup("GPAC SWF CONVERTION DISCLAIMER");
	VRML_MF_Alloc( & ((M_WorldInfo *)n)->info, FT_MFString, 3);

	sprintf(szMsg, "%s file converted to MPEG-4 Systems", read->load->fileName);
	((M_WorldInfo *)n)->info.vals[0] = strdup(szMsg);
	((M_WorldInfo *)n)->info.vals[1] = strdup("Conversion done using GPAC version " M4_VERSION " - (C) 2000-2004 GPAC");
	((M_WorldInfo *)n)->info.vals[2] = strdup("Macromedia SWF to MPEG-4 Conversion mapping released under GPL license");

	/*background*/
	n = SWF_NewNode(read, TAG_MPEG4_Background2D);
	Node_SetDEF(n, 1, "BACKGROUND");
	Node_InsertChild(read->root, n, -1);
	Node_Register(n, read->root);
	
	/*dictionary*/
	n = SWF_NewNode(read, TAG_MPEG4_Switch);
	Node_SetDEF(n, 2, "DICTIONARY");
	Node_InsertChild(read->root, n, -1);
	Node_Register(n, read->root);
	/*empty shape to fill depth levels & sprites roots*/
	n2 = SWF_NewNode(read, TAG_MPEG4_Shape);
	Node_SetDEF(n2, 3, "EMPTYSHAPE");
	ChainAddEntry( ((M_Switch *)n)->choice, n2);
	Node_Register(n2, n);

	/*display list*/
	n = SWF_NewNode(read, TAG_MPEG4_Transform2D);
	Node_SetDEF(n, 4, "DISPLAYLIST");
	Node_InsertChild(read->root, n, -1);
	Node_Register(n, read->root);
	/*update w/h transform*/
	((M_Transform2D *)n)->scale.y = -1;
	((M_Transform2D *)n)->translation.x = -read->width/2;
	((M_Transform2D *)n)->translation.y = read->height/2;

	read->load->ctx->max_node_id = 5;

	/*always reserve OD_ID=1 for main ctrl stream if any*/
	read->prev_od_id = 1;
	/*always reserve ES_ID=2 for OD stream, 3 for main ctrl stream if any*/
	read->prev_es_id = 3;

	/*no control stream*/
	if (!(read->flags & M4SWF_SplitTimeline)) return M4OK;

	/*init OD*/
	e = swf_init_od(read);
	if (e) return e;

	/*create animationStream object*/
	od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
	if (!od) return M4OutOfMem;
	od->objectDescriptorID = 1;
	esd = (ESDescriptor *) OD_NewESDescriptor(0);
	if (!esd) return M4OutOfMem;
	esd->ESID = esd->OCRESID = 3;
	esd->dependsOnESID = 1;
	esd->decoderConfig->streamType = M4ST_SCENE;
	esd->decoderConfig->objectTypeIndication = 1;
	esd->slConfig->timestampResolution = read->bifs_es->timeScale;
	OD_DeleteDescriptor((Descriptor **) &esd->decoderConfig->decoderSpecificInfo);
	ChainAddEntry(od->ESDescriptors, esd);
	e = swf_insert_od(read, 0, od);
	if (e) {
		OD_DeleteDescriptor((Descriptor **) &od);
		return e;
	}

	/*setup a new BIFS context*/
	prev_sc = read->bifs_es;
	read->bifs_es = M4SM_NewStream(read->load->ctx, esd->ESID, M4ST_SCENE, 1);
	read->bifs_es->timeScale = prev_sc->timeScale;
	/*create first AU*/
	read->bifs_au = M4SM_NewAU(read->bifs_es, 0, 0, 1);

	if (read->flags & M4SWF_NoAnimationStream) return M4OK;

	/*setup the animationStream node*/
	n = SWF_NewNode(read, TAG_MPEG4_AnimationStream);
	read->load->ctx->max_node_id++;
	ID = read->load->ctx->max_node_id;
	Node_SetDEF(n, ID, "MovieControl");

	Node_InsertChild(read->root, n, 0);
	Node_Register(n, read->root);
	/*assign URL*/
	Node_GetFieldByName(n, "url", &info);
	VRML_MF_Alloc(info.far_ptr, info.fieldType, 1);
	((MFURL*)info.far_ptr)->vals[0].OD_ID = 1;
	/*run from start*/
	((M_AnimationStream *)n)->startTime = 0;
	/*and always loop*/
	((M_AnimationStream *)n)->loop = 1;

	return M4OK;
}


void SWF_IOErr(void *par)
{
	SWFReader *read = (SWFReader *)par;
	read->ioerr = M4IOErr;
}

M4Err M4SM_LoaderRun_SWF(M4ContextLoader *load)
{
	M4Err e;
	SWFReader *read = (SWFReader *)load->loader_priv;
	if (!read) return M4BadParam;

	/*parse all tags*/
	e = M4OK;
	while (e == M4OK) {
		e = SWF_ParseTag(read);
		if (load->OnProgress) {
			load->OnProgress(load->cbk, read->current_frame, read->frame_count);
		} else {
			fprintf(stdout, "Importing SWF Frame %d / %d (%.2f %%)\r", read->current_frame, read->frame_count, (Float) (100*read->current_frame) / read->frame_count);
		}
	}
	if (!load->OnProgress) fprintf(stdout, "\n");
	if (e==M4EOF) e = M4OK;

	if (!e) {
		if (read->flat_limit != 0.0f) 
			swf_report(read, M4OK, "%d points removed while parsing shapes (Flattening limit %.4f)", read->flatten_points, read->flat_limit);

		if (read->has_interact) swf_report(read, M4OK, "Buttons and ActionScripts are not supported and have been removed");
	}
	return e;
}

void M4SM_LoaderDone_SWF(M4ContextLoader *load)
{
	SWFReader *read = (SWFReader *) load->loader_priv;
	if (!read) return;

	if (read->compressed) swf_done_decompress(read);
	DeleteBitStream(read->bs);
	while (ChainGetCount(read->display_list)) {
		DispShape *s = ChainGetEntry(read->display_list, 0);
		ChainDeleteEntry(read->display_list, 0);
		free(s);
	}
	DeleteChain(read->display_list);
	while (ChainGetCount(read->fonts)) {
		SWFFont *ft = ChainGetEntry(read->fonts, 0);
		ChainDeleteEntry(read->fonts, 0);
		if (ft->glyph_adv) free(ft->glyph_adv);
		if (ft->glyph_codes) free(ft->glyph_codes);
		if (ft->fontName) free(ft->fontName);
		while (ChainGetCount(ft->glyphs)) {
			SFNode *gl = ChainGetEntry(ft->glyphs, 0);
			ChainDeleteEntry(ft->glyphs, 0);
			Node_Unregister(gl, NULL);
		}
		DeleteChain(ft->glyphs);
		free(ft);
	}
	DeleteChain(read->fonts);
	DeleteChain(read->apps);

	while (ChainGetCount(read->sounds)) {
		SWFSound *snd = ChainGetEntry(read->sounds, 0);
		ChainDeleteEntry(read->sounds, 0);
		if (snd->output) fclose(snd->output);
		if (snd->szFileName) free(snd->szFileName);
		free(snd);
	}
	DeleteChain(read->sounds);
	swf_delete_sound_stream(read);
	if (read->localPath) free(read->localPath);
	fclose(read->input);
	free(read);
	load->loader_priv = NULL;
}

M4Err M4SM_LoaderInit_SWF(M4ContextLoader *load)
{
	SWFReader *read;
	SWFRec rc;
	M4Err e;
	FILE *input;

	if (!load->ctx || !load->scene_graph || !load->fileName) return M4BadParam;
	input = fopen(load->fileName, "rb");
	if (!input) return M4URLNotFound;

	SAFEALLOC(read, sizeof(SWFReader));
	read->load = load;
	
	e = M4OK;

	read->input = input;
	read->bs = NewBitStreamFromFile(input, BS_FILE_READ);
	BS_SetEOSCallback(read->bs, SWF_IOErr, &read);
	read->display_list = NewChain();
	read->fonts = NewChain();
	read->apps = NewChain();
	read->sounds = NewChain();

	read->flags = load->swf_import_flags;
	read->flat_limit = load->swf_flatten_limit;
	if (load->localPath) read->localPath = strdup(load->localPath);
	else {
		char *c;
		read->localPath = strdup(load->fileName);
		c = strrchr(read->localPath, M4_PATH_SEPARATOR);
		if (c) c[1] = 0;
		else {
			free(read->localPath);
			read->localPath = NULL;
		}
	}

	load->loader_priv = read;

	/*get signature*/
	read->sig[0] = BS_ReadInt(read->bs, 8);
	read->sig[1] = BS_ReadInt(read->bs, 8);
	read->sig[2] = BS_ReadInt(read->bs, 8);
	/*"FWS" or "CWS"*/
	if ( ((read->sig[0] != 'F') && (read->sig[0] != 'C')) || (read->sig[1] != 'W') || (read->sig[2] != 'S') ) {
		e = M4InvalidURL;
		goto exit;
	}
	read->version = BS_ReadInt(read->bs, 8);
	read->length = swf_get_32(read);

	/*if compressed decompress the whole file*/
	swf_init_decompress(read);
	
	swf_get_rec(read, &rc);
	read->width = rc.w;
	read->height = rc.h;
	load->ctx->scene_width = (u32) read->width;
	load->ctx->scene_height = (u32) read->height;
	load->ctx->is_pixel_metrics = 1;
	
	swf_align(read);
	read->frame_rate = swf_get_16(read)>>8;
	read->frame_count = swf_get_16(read);
	
	swf_report(read, M4OK, "SWF Import - Scene Size %dx%d - %d frames @ %d FPS", load->ctx->scene_width, load->ctx->scene_height, read->frame_count, read->frame_rate);

	/*init scene*/
	if (read->flags & M4SWF_SplitTimeline) read->flags |= M4SWF_StaticDictionary;
	
	e = SWF_InitContext(read);

	/*parse all tags*/
	while (e == M4OK) {
		e = SWF_ParseTag(read);
		if (read->current_frame==1) break;
	}
	if (e==M4EOF) e = M4OK;

exit:
	if (e) M4SM_LoaderDone_SWF(load);
	return e;
}

