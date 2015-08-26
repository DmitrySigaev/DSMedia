#ifndef _LASERCODEC_H
#define _LASERCODEC_H

#include <gpac/intern/m4_esm_dev.h>
#include "m4_laser_dev.h"

typedef struct 
{
	InlineScene *pScene;
	struct _m4_client *app;
	LPLASERDEC codec;
} LASeRPriv;

BaseDecoder *NewLASeRCodec();
void LASeRDec_Delete(BaseDecoder *plug);

#endif