#include "m4_laser_dev.h"

#ifdef M4_USE_LASeR

LPLASERDEC LASeR_NewDecoder(LPSCENEGRAPH scenegraph)
{
	LASeRDecoder * tmp;
	SAFEALLOC(tmp, sizeof(LASeRDecoder));

	tmp->scenegraph = scenegraph;
	if (!scenegraph) {
		tmp->dec_memory_mode = 1;
		tmp->conditionals = NewChain();
	}
	tmp->current_graph = NULL;
	tmp->mx = NewMutex();
	return tmp;
}

void LASeR_DeleteDecoder(LPLASERDEC codec)
{	
	if (!codec) return;
	if (codec->dec_memory_mode) {
		assert(ChainGetCount(codec->conditionals) == 0);
		DeleteChain(codec->conditionals);
	}
	LASeR_DeleteSceneHeader(codec->config);
	MX_Delete(codec->mx);
	free(codec);
}

void LASeR_SetClock(LPLASERDEC codec, Double (*GetSceneTime)(void *st_cbk), void *st_cbk )
{
	codec->GetSceneTime = GetSceneTime;
	codec->st_cbk = st_cbk;
}

M4Err LASeR_ConfigureStream(LASeRDecoder * codec, char *DecoderSpecificInfo, u32 DecoderSpecificInfoLength)
{
	BitStream *bs;	
	if (!DecoderSpecificInfo) return M4BadParam;
	MX_P(codec->mx);
	
	bs = NewBitStream(DecoderSpecificInfo, DecoderSpecificInfoLength, BS_READ);
	codec->config = LASeR_ParseSceneHeader(bs);
	DeleteBitStream(bs);

	SG_SetSizeInfo(codec->scenegraph, codec->config->width, codec->config->height, 1);

	MX_V(codec->mx);
	return M4OK;
}

SFNode *LASeR_FindNode(LASeRDecoder * codec, u32 NodeID)
{	
	assert(codec->current_graph);
	return SG_FindNode(codec->current_graph, NodeID);
}

#endif	//M4_USE_LASeR


