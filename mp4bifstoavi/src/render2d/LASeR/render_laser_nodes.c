#include "../visualsurface2d.h"

#ifdef M4_USE_LASeR

#include "laser_stacks.h"

static void LASeR_DestroyTransform(SFNode *n)
{
	Transform2DStack *ptr = (Transform2DStack *)Node_GetPrivate(n);
	DeleteGroupingNode2D((GroupingNode2D *)ptr);
	free(ptr);
}

static void LASeR_RenderTransform(SFNode *node, void *rs)
{
	M4Matrix2D bckup;
	LASeRTransform *tr = (LASeRTransform *)node;
	Transform2DStack *ptr = (Transform2DStack *)Node_GetPrivate(node);
	RenderEffect2D *eff;

	eff = (RenderEffect2D *) rs;

	if (!Node_GetActive(node)) return;

	if (Node_GetDirty(node) & SG_NODE_DIRTY) {
		mx2d_init(ptr->mat);
		ptr->is_identity = 1;
		if ((tr->m.xx != 1.0) || (tr->m.yy != 1.0)) {
			mx2d_add_scale(&ptr->mat, tr->m.xx, tr->m.yy);
			ptr->is_identity = 0;
		}
		if (tr->m.xy || tr->m.yx) {
			mx2d_add_skew(&ptr->mat, tr->m.xy, tr->m.yx);
			ptr->is_identity = 0;
		}
		if (tr->m.xz || tr->m.yz) {
			ptr->is_identity = 0;
			mx2d_add_translation(&ptr->mat, tr->m.xz, tr->m.yz);
		}
	}

	/*note we don't clear dirty flag, this is done in traversing*/
	if (ptr->is_identity) {
		group2d_traverse((GroupingNode2D *)ptr, tr->children, eff);
	} else {
		mx2d_copy(bckup, eff->transform);
		mx2d_copy(eff->transform, ptr->mat);
		mx2d_add_matrix(&eff->transform, &bckup);
		group2d_traverse((GroupingNode2D *)ptr, tr->children, eff);
		mx2d_copy(eff->transform, bckup);
	}
}

void LASeR_InitTransform(Render2D *sr, SFNode *node)
{
	Transform2DStack *stack = malloc(sizeof(Transform2DStack));
	SetupGroupingNode2D((GroupingNode2D *)stack, sr, node);
	mx2d_init(stack->mat);
	stack->is_identity = 1;
	Node_SetPrivate(node, stack);
	Node_SetPreDestroyFunction(node, LASeR_DestroyTransform);
	Node_SetRenderFunction(node, LASeR_RenderTransform);
}

static void LASeR_RenderShape(SFNode *node, void *rs) 
{
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;
	LASeRShape *shape = (LASeRShape *)node;
	u32 i;

	if (!Node_GetActive(node)) return;

	if (Node_GetDirty(node)) {
		drawable_reset_path(cs);
		if (shape->initpointseq.nbPoints) {
			m4_path_add_move_to(cs->path, shape->initpointseq.x[0], shape->initpointseq.y[0]);
			if (shape->initpointseq.x[0] == shape->initpointseq.x[shape->initpointseq.nbPoints-1] &&
				shape->initpointseq.y[0] == shape->initpointseq.y[shape->initpointseq.nbPoints-1]) {
				for (i = 1; i < shape->initpointseq.nbPoints - 1; i++) {
					m4_path_add_line_to(cs->path, shape->initpointseq.x[i], shape->initpointseq.y[i]);
				}
				m4_path_close(cs->path);
			} else {
				for (i = 1; i < shape->initpointseq.nbPoints; i++) {
					m4_path_add_line_to(cs->path, shape->initpointseq.x[i], shape->initpointseq.y[i]);
				}
			}
			if (shape->nbAdditionalSeqs) {
				LASeRPointSequence *pts;
				u32 j;
				for (j = 0; j< shape->nbAdditionalSeqs; j++) {
					pts = &(shape->morepointseqs[j]);
					m4_path_add_move_to(cs->path, pts->x[0], pts->y[0]);
					if (pts->x[0] == pts->x[pts->nbPoints-1] &&
						pts->y[0] == pts->y[pts->nbPoints-1]) {
						for (i = 1; i < pts->nbPoints - 1; i++) {
							m4_path_add_line_to(cs->path, pts->x[i], pts->y[i]);
						}
						m4_path_close(cs->path);
					} else {
						for (i = 1; i < pts->nbPoints; i++) {
							m4_path_add_line_to(cs->path, pts->x[i], pts->y[i]);
						}
					}
				}
			}
		}
		Node_ClearDirty(node);
		cs->node_changed = 1;
	}
	ctx = LASeR_drawable_init_context(cs, eff);
	if (!ctx) return;
	
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);
}

void LASeR_InitShape(Render2D *sr, SFNode *node) 
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, LASeR_RenderShape);
}

static void LASeR_RenderText(SFNode *node, void *rs) 
{
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;
	LASeRText *txt = (LASeRText *) node;
	unsigned char face[100];
	unsigned short wcTemp[5000];
	char *str;

	M4Rect rc;
	FontRaster *ft_dr = eff->surface->render->compositor->font_engine;
	Float fontSize, start_y, font_height, line_spacing;
	Float ascent, descent;

	if (!Node_GetActive(node)) return;
	if (!ft_dr) return;
	if (!txt->string.length || !txt->string.string || !strcmp(txt->string.string, "")) return;


	fontSize = txt->size;
	line_spacing = fontSize;

	if (!strcmp(txt->face->string, "PROPORTIONAL")) {
		strcpy(face,"Courier New");
	} else if (!strcmp(txt->face->string, "SYSTEM")) {
		strcpy(face,"Times New Roman");
	} else if (!strcmp(txt->face->string, "MONOSPACE")) {
		strcpy(face,"Times New Roman");
	} else {
		strcpy(face,txt->face->string);
	}

	str = txt->string.string;
	utf8_mbstowcs(wcTemp, 5000, (const char **) &str);
	if (ft_dr->set_font(ft_dr, face, "PLAIN") != M4OK) {
		if (ft_dr->set_font(ft_dr, NULL, "PLAIN") != M4OK) {
			return;
		}
	}

	ft_dr->set_font_size(ft_dr, fontSize);
	ft_dr->get_font_metrics(ft_dr, &ascent, &descent, &font_height);
	/*adjust size*/
	fontSize *= fontSize / (font_height);
	ft_dr->set_font_size(ft_dr, fontSize);
	ft_dr->get_font_metrics(ft_dr, &ascent, &descent, &font_height);
	start_y = descent;

	if (Node_GetDirty(node) && txt->string.length > 0) {
		drawable_reset_path(cs);
		ft_dr->add_text_to_path(ft_dr, cs->path, 1, wcTemp, 0, start_y, 1, 1, ascent, &rc);			
		Node_ClearDirty(node);
		cs->node_changed = 1;
	}
	ctx = LASeR_drawable_init_context(cs, eff);
	if (!ctx) return;
	
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);
}

void LASeR_InitText(Render2D *sr, SFNode *node) 
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, LASeR_RenderText);
}

static void LASeR_RenderUse(SFNode *node, void *rs) 
{
	if (!Node_GetActive(node)) return;
	Node_Render((SFNode *)((LASeRUse *)node)->used_node, rs);
}

void LASeR_InitUse(Render2D *sr, SFNode *node) 
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, LASeR_RenderUse);
}

static void LASeR_RenderBackground(SFNode *node, void *rs) 
{
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;
	LASeRBackground *bck = (LASeRText *) node;

	if (Node_GetDirty(node)) {
		ctx = LASeR_drawable_init_context(cs, eff);
		if (!ctx) return;
		ctx->aspect.filled = 1;
		ctx->is_background = 1;
		ctx->aspect.fill_color = MAKE_ARGB_FLOAT(1, bck->color->r, bck->color->g, bck->color->b);

		VS2D_Clear(ctx->surface, NULL, ctx->aspect.fill_color);
		ctx->redraw_flags = 0;
		Node_ClearDirty(node);
		cs->node_changed = 1;
	}
	
}

void LASeR_InitBackground(Render2D *sr, SFNode *node) 
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, LASeR_RenderBackground);
}

static void LASeR_DestroyVideo(SFNode *n)
{
}

static void LASeR_RenderVideo(SFNode *node, void *rs) 
{
}

void LASeR_InitVideo(Render2D *sr, SFNode *node) 
{
	Node_SetPreDestroyFunction(node, LASeR_DestroyVideo);
	Node_SetRenderFunction(node, LASeR_RenderVideo);
}

static void LASeR_DestroyBitmap(SFNode *n)
{
}

static void LASeR_RenderBitmap(SFNode *node, void *rs) 
{
}

void LASeR_InitBitmap(Render2D *sr, SFNode *node) 
{
	Node_SetPreDestroyFunction(node, LASeR_DestroyBitmap);
	Node_SetRenderFunction(node, LASeR_RenderBitmap);
}


static void LASeR_DestroyAudio(SFNode *n)
{
}

static void LASeR_RenderAudio(SFNode *node, void *rs) 
{
}

void LASeR_InitAudio(Render2D *sr, SFNode *node) 
{
	Node_SetPreDestroyFunction(node, LASeR_DestroyAudio);
	Node_SetRenderFunction(node, LASeR_RenderAudio);
}

#endif

