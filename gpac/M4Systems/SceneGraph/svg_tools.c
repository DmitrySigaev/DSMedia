/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Cyril Concolato 2004
 *					All rights reserved
 *
 *  This file is part of GPAC / SVG Scene Graph sub-project
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
#include <gpac/intern/m4_scenegraph_dev.h>

#ifndef M4_DISABLE_SVG

#include <gpac/m4_scenegraph_svg.h>

SVGElement *SVG_NewNode(LPSCENEGRAPH inScene, u32 tag)
{
	SVGElement *node;
	if (!inScene) return NULL;
	node = SVG_CreateNode(tag);
	if (node) {
		node->sgprivate->scenegraph = inScene;
	}
	return (SVGElement *)node;
}

void SVG_DelPathData(SVG_PathData *d_attribute) 
{
	u32 i;

	for (i = 0; i < ChainGetCount(d_attribute->path_commands); i++)
	{
		u8 *type = ChainGetEntry(d_attribute->path_commands, i);
		free(type);
	}
	DeleteChain(d_attribute->path_commands);
	d_attribute->path_commands = NULL;
	for (i = 0; i < ChainGetCount(d_attribute->path_points); i++)
	{
		SVG_Point *p = ChainGetEntry(d_attribute->path_points, i);
		free(p);
	}
	DeleteChain(d_attribute->path_points);
	d_attribute->path_points = NULL;
}

#else
/*these ones are only needed for W32 M4Systems_dll build in order not to modify export def file*/
u32 SVG_GetTagByName(const char *element_name)
{
	return 0;
}
u32 SVG_GetAttributeCount(SFNode *n)
{
	return 0;
}
M4Err SVG_GetAttributeInfo(SFNode *node, FieldInfo *info)
{
	return M4NotSupported;
}

SFNode *SVG_NewNode(LPSCENEGRAPH inScene, u32 tag)
{
	return NULL;
}
SFNode *SVG_CreateNode(LPSCENEGRAPH inScene, u32 tag)
{
	return NULL;
}
const char *SVG_GetElementName(u32 tag)
{
	return "Unsupported";
}

#endif	//M4_DISABLE_SVG
