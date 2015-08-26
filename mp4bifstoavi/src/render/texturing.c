/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
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

#include <intern/m4_render_dev.h>

void texture_setup(TextureHandler *txh, SceneRenderer *sr, SFNode *owner)
{
	memset(txh, 0, sizeof(TextureHandler));
	txh->owner = owner;
	txh->compositor = sr;
	if (ChainFindEntry(sr->textures, txh)<0) ChainAddEntry(sr->textures, txh);
}


void texture_destroy(TextureHandler *txh)
{
	if (txh->hwtx) {
		txh->compositor->visual_renderer->ReleaseTexture(txh);
		txh->hwtx = NULL;
	}
	if (txh->is_open) texture_stop(txh);
	ChainDeleteItem(txh->compositor->textures, txh);
}

Bool texture_check_url_change(TextureHandler *txh, MFURL *url)
{
	if (!txh->stream) return url->count;
	return MO_UrlChanged(txh->stream, url);
}

M4Err texture_play(TextureHandler *txh, MFURL *url)
{
	if (txh->is_open) return M4BadParam;

	/*if existing texture in cache destroy it - we don't destroy it on stop to handle MovieTexture*/
	if (txh->hwtx) {
		txh->compositor->visual_renderer->ReleaseTexture(txh);
		txh->hwtx = NULL;
	}

	/*store url*/
	VRML_FieldCopy(&txh->current_url, url, FT_MFURL);

	/*get media object*/
	txh->stream = MO_FindObject(txh->owner, url);
	/*bad/Empty URL*/
	if (!txh->stream) return M4UnsupportedURL;

	/*request play*/
	MO_Play(txh->stream);
	txh->last_frame_time = (u32) (-1);
	SR_Invalidate(txh->compositor, NULL);
	txh->is_open = 1;
	return M4OK;
}

void texture_stop(TextureHandler *txh)
{
	if (!txh->is_open) return;
	texture_release_stream(txh);
	SR_Invalidate(txh->compositor, NULL);
	MO_Stop(txh->stream);
	VRML_MF_Reset(&txh->current_url, FT_MFURL);
	txh->is_open = 0;

	/*and deassociate object*/
	txh->stream = NULL;
}

void texture_restart(TextureHandler *txh)
{
	if (!txh->is_open) return;
	texture_release_stream(txh);
	txh->stream_finished = 0;
	MO_Restart(txh->stream);
}

void texture_update_frame(TextureHandler *txh, Bool disable_resync)
{
	if (!txh->stream) return;
	/*already refreshed*/
	if (txh->needs_refresh) return;

	/*this is in case the texture was not release at previous cycle if the node hasn't been drawn
	we need a better way to do this since it locks video memory*/
	if (txh->needs_release) {
		MO_ReleaseFrame(txh->stream, txh->stream->current_size, 0);
	}
	txh->needs_release = MO_FetchFrame(txh->stream, !disable_resync, &txh->stream_finished);

	/*check init flag*/
	if (!(txh->stream->mo_flags & MO_IS_INIT)) {
		/*if we had a texture this means the object has changed - delete texture and force next frame 
		rendering (this will take care of OD reuse)*/
		if (txh->hwtx) {
			txh->compositor->visual_renderer->ReleaseTexture(txh);
			txh->hwtx = NULL;
			txh->needs_refresh = 1;
			SR_Invalidate(txh->compositor, NULL);
			return;
		}
	}

	/*if no frame or muted don't draw*/
	if (!txh->stream->current_frame || !txh->stream->current_size) return;

	/*if setup and same frame return*/
	if (txh->hwtx && (txh->stream_finished || (txh->last_frame_time==txh->stream->current_ts)) ) {
		if (txh->needs_release) {
			MO_ReleaseFrame(txh->stream, txh->stream->current_size, 0);
			txh->needs_release = 0;
		}
		return;
	}
	txh->last_frame_time = txh->stream->current_ts;
	if (MO_IsMuted(txh->stream)) return;

	if (!txh->hwtx) {
		txh->compositor->visual_renderer->AllocTexture(txh);
		if (!txh->hwtx) return;

		/*for now the video object is the entire frame*/
		txh->active_window.x = txh->active_window.y = 0;
		txh->active_window.width = (Float) txh->stream->width;
		txh->active_window.height = (Float) txh->stream->height;

		txh->transparent = 0;
		switch (txh->stream->pixelFormat) {
		case M4PF_ALPHAGREY:
		case M4PF_ARGB:
		case M4PF_RGBA:
		case M4PF_YUVA:
			txh->transparent = 1;
			break;
		}

		txh->stream->mo_flags |= MO_IS_INIT;
	}

	txh->width = txh->stream->width;
	txh->height = txh->stream->height;
	txh->pixelformat = txh->stream->pixelFormat;
	txh->stride = txh->stream->stride;
	txh->frame_size = txh->stream->current_size;
	txh->data = txh->stream->current_frame;

	/*try to push texture on graphics but don't complain if failure*/
	txh->compositor->visual_renderer->SetTextureData(txh);

	txh->needs_refresh = 1;
	SR_Invalidate(txh->compositor, NULL);
}

void texture_release_stream(TextureHandler *txh)
{
	if (txh->needs_release) {
		assert(txh->stream);
		MO_ReleaseFrame(txh->stream, txh->stream->current_size, 0);
		txh->needs_release = 0;
	}
	txh->needs_refresh = 0;
}
