#include <gpac/intern/m4_scenegraph_dev.h>

#ifdef M4_USE_LASeR

#include "m4_laser_dev.h"

LASeRNode *LASERSG_NewNode(LPSCENEGRAPH inScene, u32 tag)
{
	SFNode *node;
	if (!inScene) return NULL;
	node = LASeR_CreateNode(tag);
	if (node) {
		node->sgprivate->scenegraph = inScene;
		Node_Init(node);
	}
	return (LASeRNode *)node;
}


#endif	//M4_USE_LASeR
