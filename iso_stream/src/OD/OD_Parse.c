/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Object Descriptor sub-project
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


#include <m4_descriptors.h>
#include <m4_author.h>


/* to complete...*/

u32 OD_DescriptorFieldType(Descriptor *desc, char *fieldName)
{
	switch (desc->tag) {
	case InitialObjectDescriptor_Tag:
	case ObjectDescriptor_Tag:
		if (!stricmp(fieldName, "esDescr")) return 2;
		if (!stricmp(fieldName, "ociDescr")) return 2;
		if (!stricmp(fieldName, "ipmpDescrPtr")) return 2;
		if (!stricmp(fieldName, "extDescr")) return 2;
		return 0;
	case DecoderConfigDescriptor_Tag:
		if (!stricmp(fieldName, "decSpecificInfo")) return 1;
		if (!stricmp(fieldName, "profileLevelIndicationIndexDescr")) return 2;
		return 0;
	case ESDescriptor_Tag:
		if (!stricmp(fieldName, "decConfigDescr")) return 1;
		if (!stricmp(fieldName, "muxInfo")) return 1;
		if (!stricmp(fieldName, "StreamSource")) return 1;
		if (!stricmp(fieldName, "slConfigDescr")) return 1;
		if (!stricmp(fieldName, "ipiPtr")) return 1;
		if (!stricmp(fieldName, "qosDescr")) return 1;
		if (!stricmp(fieldName, "regDescr")) return 1;
		if (!stricmp(fieldName, "ipIDS")) return 2;
		if (!stricmp(fieldName, "langDescr")) return 2;
		if (!stricmp(fieldName, "ipmpDescPtr")) return 2;
		if (!stricmp(fieldName, "extDescr")) return 2;
		return 0;
	case TextConfig_Tag:
		if (!stricmp(fieldName, "SampleDescriptions")) return 2;
		return 0;
	}
	return 0;
}

u32 OD_GetDescriptorTag(char *descName)
{
	if (!stricmp(descName, "ObjectDescriptor")) return ObjectDescriptor_Tag;
	if (!stricmp(descName, "InitialObjectDescriptor")) return InitialObjectDescriptor_Tag;
	if (!stricmp(descName, "ES_Descriptor")) return ESDescriptor_Tag;
	if (!stricmp(descName, "DecoderConfigDescriptor")) return DecoderConfigDescriptor_Tag;
	if (!stricmp(descName, "DecoderSpecificInfo")) return DecoderSpecificInfo_Tag;
	if (!stricmp(descName, "DecoderSpecificInfoString")) return DecoderSpecificInfo_Tag;
	if (!stricmp(descName, "SLConfigDescriptor")) return SLConfigDescriptor_Tag;
	if (!stricmp(descName, "ContentIdentification")) return ContentIdentification_Tag;
	if (!stricmp(descName, "SuppContentIdentification")) return SuppContentIdentification_Tag;
	if (!stricmp(descName, "IPIPtr")) return IPIPtr_Tag;
	if (!stricmp(descName, "IPMPPtr")) return IPMPPtr_Tag;
	if (!stricmp(descName, "IPMP")) return IPMP_Tag;
	if (!stricmp(descName, "QoS")) return QoS_Tag;
	if (!stricmp(descName, "RegistrationDescriptor")) return RegistrationDescriptor_Tag;
	if (!stricmp(descName, "ExtensionPL")) return ExtensionPL_Tag;
	if (!stricmp(descName, "PL_IndicationIndex")) return PL_IndicationIndex_Tag;
	if (!stricmp(descName, "ContentClassification")) return ContentClassification_Tag;
	if (!stricmp(descName, "KeyWordDescriptor")) return KeyWordDescriptor_Tag;
	if (!stricmp(descName, "RatingDescriptor")) return RatingDescriptor_Tag;
	if (!stricmp(descName, "LanguageDescriptor")) return LanguageDescriptor_Tag;
	if (!stricmp(descName, "ShortTextualDescriptor")) return ShortTextualDescriptor_Tag;
	if (!stricmp(descName, "ExpandedTextualDescriptor")) return ExpandedTextualDescriptor_Tag;
	if (!stricmp(descName, "ContentCreatorName")) return ContentCreatorName_Tag;
	if (!stricmp(descName, "ContentCreationDate")) return ContentCreationDate_Tag;
	if (!stricmp(descName, "OCI_CreatorName")) return OCI_CreatorName_Tag;
	if (!stricmp(descName, "OCI_CreationDate")) return OCI_CreationDate_Tag;
	if (!stricmp(descName, "SmpteCameraPosition")) return SmpteCameraPosition_Tag;
	if (!stricmp(descName, "SegmentDescriptor")) return SegmentDescriptor_Tag;
	if (!stricmp(descName, "MediaTimeDescriptor")) return MediaTimeDescriptor_Tag;
	if (!stricmp(descName, "MuxInfo")) return MuxInfoDescriptor_Tag;
	if (!stricmp(descName, "StreamSource")) return MuxInfoDescriptor_Tag;
	if (!stricmp(descName, "BIFSConfig") || !stricmp(descName, "BIFSv2Config")) return BIFSConfig_Tag;
	if (!stricmp(descName, "TextConfig")) return TextConfig_Tag;
	if (!stricmp(descName, "TextSampleDescriptor")) return TextSampleDescriptor_Tag;
	if (!stricmp(descName, "UIConfig")) return UIConfig_Tag;
	if (!stricmp(descName, "ES_ID_Ref")) return ES_ID_RefTag;
	if (!stricmp(descName, "ES_ID_Inc")) return ES_ID_IncTag;
	if (!stricmp(descName, "DefaultDescriptor")) return DecoderSpecificInfo_Tag;
	return ForbiddenZero_Tag;
}

#define GET_U8(field) { if (strstr(val, "0x")) { ret += sscanf(val, "%x", &d); if (ret) field = (u8) d; } else { ret += sscanf(val, "%d", &d); if (ret) field = (u8) d; }	}	
#define GET_U32(field) { if (strstr(val, "0x")) { ret += sscanf(val, "%x", &d); if (ret) field = (u32) d; } else { ret += sscanf(val, "%d", &d); if (ret) field = (u32) d; }	}	
#define GET_S32(field) { if (strstr(val, "0x")) { ret += sscanf(val, "%x", &d); if (ret) field = (s32) d; } else { ret += sscanf(val, "%d", &d); if (ret) field = (s32) d; }	}	
#define GET_BOOL(field) { ret = 1; field = (!stricmp(val, "true") || !stricmp(val, "1")) ? 1 : 0; }

#define GET_DOUBLE(field) { Float v; ret = 1; sscanf(val, "%f", &v); field = (Double) v;}
#define GET_STRING(field) { ret = 1; field = strdup(val); if (val[0] == '"') strcpy(field, val+1); if (field[strlen(field)-1] == '"') field[strlen(field)-1] = 0; }

void OD_ParseBinData(char *val, char **out_data, u32 *out_data_size)
{
	u32 i, c;
	char s[3];
	u32 len = strlen(val) / 3;
	if (*out_data) free(*out_data);
	*out_data_size = len;
	*out_data = malloc(sizeof(char) * len);
	s[2] = 0;
	for (i=0; i<len; i++) {
		s[0] = val[3*i+1];
		s[1] = val[3*i+2];
		sscanf(s, "%02X", &c);
		(*out_data)[i] = (unsigned char) c;
	}
}

void OD_ParseFileData(char *fileName, char **out_data, u32 *out_data_size)
{
	FILE *f;
	u32 size;
	if (*out_data) free(*out_data);
	*out_data = NULL;
	*out_data_size = 0;
	f = fopen(fileName, "rb");
	if (!f) {
		fprintf(stdout, "WARNING: cannot open data file %s - ignoring config\n", fileName);
		return;
	}
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	*out_data_size = size;
	*out_data = malloc(sizeof(char) * size);
	fread(*out_data, sizeof(char) * size, 1, f);
	fclose(f);
}

M4Err OD_SetDescriptorField(Descriptor *desc, char *fieldName, char *val)
{
	Bool OD_ParseUIConfig(char *val, char **out_data, u32 *out_data_size);
	u32 d, ret = 0;

	if (!stricmp(val, "auto")) return M4OK;

	switch (desc->tag) {
	case InitialObjectDescriptor_Tag:
	{
		InitialObjectDescriptor *iod = (InitialObjectDescriptor *)desc;
		if (!stricmp(fieldName, "objectDescriptorID") || !stricmp(fieldName, "binaryID")) ret += sscanf(val, "%hd", &iod->objectDescriptorID);
		else if (!stricmp(fieldName, "URLString")) {
			iod->URLString = strdup(val);
			ret = 1;
		}
		else if (!stricmp(fieldName, "includeInlineProfileLevelFlag")) {
			GET_BOOL(iod->inlineProfileFlag)
			if (!ret) { iod->inlineProfileFlag = 0; ret = 1; }
		}
		else if (!stricmp(fieldName, "ODProfileLevelIndication")) {
			GET_U8(iod->OD_profileAndLevel)
			if (!ret) { iod->OD_profileAndLevel = 0xFE; ret = 1; }
		}
		else if (!stricmp(fieldName, "sceneProfileLevelIndication")) {
			GET_U8(iod->scene_profileAndLevel)
			if (!ret) { iod->scene_profileAndLevel = 0xFE; ret = 1; }
		}
		else if (!stricmp(fieldName, "audioProfileLevelIndication")) {
			GET_U8(iod->audio_profileAndLevel)
			if (!ret) { iod->audio_profileAndLevel = 0xFE; ret = 1; }
		}
		else if (!stricmp(fieldName, "visualProfileLevelIndication")) {
			GET_U8(iod->visual_profileAndLevel)
			if (!ret) { iod->visual_profileAndLevel = 0xFE; ret = 1; }
		}
		else if (!stricmp(fieldName, "graphicsProfileLevelIndication")) {
			GET_U8(iod->graphics_profileAndLevel)
			if (!ret) { iod->graphics_profileAndLevel = 0xFE; ret = 1; }
		}
	}
		break;
	case ObjectDescriptor_Tag:
	{
		ObjectDescriptor *od = (ObjectDescriptor *) desc;
		if (!stricmp(fieldName, "objectDescriptorID") || !stricmp(fieldName, "binaryID")) ret += sscanf(val, "%hd", &od->objectDescriptorID);
		else if (!stricmp(fieldName, "URLString")) {
			od->URLString = strdup(val);
			ret = 1;
		}
	}
		break;
	case DecoderConfigDescriptor_Tag:
	{
		DecoderConfigDescriptor *dcd = (DecoderConfigDescriptor *)desc;
		if (!stricmp(fieldName, "objectTypeIndication")) {
			GET_U8(dcd->objectTypeIndication)
			/*XMT may use string*/
			if (!ret) {
				if (!stricmp(val, "MPEG4Systems1")) { dcd->objectTypeIndication = 0x01; ret = 1; }
				else if (!stricmp(val, "MPEG4Systems2")) { dcd->objectTypeIndication = 0x02; ret = 1; }
				else if (!stricmp(val, "MPEG4Visual")) { dcd->objectTypeIndication = 0x20; ret = 1; }
				else if (!stricmp(val, "MPEG4Audio")) { dcd->objectTypeIndication = 0x40; ret = 1; }
				else if (!stricmp(val, "MPEG2VisualSimple")) { dcd->objectTypeIndication = 0x60; ret = 1; }
				else if (!stricmp(val, "MPEG2VisualMain")) { dcd->objectTypeIndication = 0x61; ret = 1; }
				else if (!stricmp(val, "MPEG2VisualSNR")) { dcd->objectTypeIndication = 0x62; ret = 1; }
				else if (!stricmp(val, "MPEG2VisualSpatial")) { dcd->objectTypeIndication = 0x63; ret = 1; }
				else if (!stricmp(val, "MPEG2VisualHigh")) { dcd->objectTypeIndication = 0x64; ret = 1; }
				else if (!stricmp(val, "MPEG2Visual422")) { dcd->objectTypeIndication = 0x65; ret = 1; }
				else if (!stricmp(val, "MPEG2AudioMain")) { dcd->objectTypeIndication = 0x66; ret = 1; }
				else if (!stricmp(val, "MPEG2AudioLowComplexity")) { dcd->objectTypeIndication = 0x67; ret = 1; }
				else if (!stricmp(val, "MPEG2AudioScaleableSamplingRate")) { dcd->objectTypeIndication = 0x68; ret = 1; }
				else if (!stricmp(val, "MPEG2AudioPart3")) { dcd->objectTypeIndication = 0x69; ret = 1; }
				else if (!stricmp(val, "MPEG1Visual")) { dcd->objectTypeIndication = 0x6A; ret = 1; }
				else if (!stricmp(val, "MPEG1Audio")) { dcd->objectTypeIndication = 0x6B; ret = 1; }
				else if (!stricmp(val, "JPEG")) { dcd->objectTypeIndication = 0x6C; ret = 1; }
				else if (!stricmp(val, "PNG")) { dcd->objectTypeIndication = 0x6D; ret = 1; }
			}
		}
		else if (!stricmp(fieldName, "streamType")) {
			GET_U8(dcd->streamType)
			/*XMT may use string*/
			if (!ret) {
				if (!stricmp(val, "ObjectDescriptor")) { dcd->streamType = M4ST_OD; ret = 1; } 
				else if (!stricmp(val, "ClockReference")) { dcd->streamType = M4ST_OCR; ret = 1; }
				else if (!stricmp(val, "SceneDescription")) { dcd->streamType = M4ST_SCENE; ret = 1; }
				else if (!stricmp(val, "Visual")) { dcd->streamType = M4ST_VISUAL; ret = 1; }
				else if (!stricmp(val, "Audio")) { dcd->streamType = M4ST_AUDIO; ret = 1; }
				else if (!stricmp(val, "MPEG7")) { dcd->streamType = M4ST_MPEG7; ret = 1; }
				else if (!stricmp(val, "IPMP")) { dcd->streamType = M4ST_IPMP; ret = 1; }
				else if (!stricmp(val, "OCI")) { dcd->streamType = M4ST_OCI; ret = 1; }
				else if (!stricmp(val, "MPEGJ")) { dcd->streamType = M4ST_MPEGJ; ret = 1; }
			}
		}
		else if (!stricmp(fieldName, "upStream")) GET_BOOL(dcd->upstream)
		else if (!stricmp(fieldName, "bufferSizeDB")) ret += sscanf(val, "%d", &dcd->bufferSizeDB);
		else if (!stricmp(fieldName, "maxBitRate")) ret += sscanf(val, "%d", &dcd->maxBitrate);
		else if (!stricmp(fieldName, "avgBitRate")) ret += sscanf(val, "%d", &dcd->avgBitrate);
	}
		break;
	case ESDescriptor_Tag:
	{
		ESDescriptor *esd = (ESDescriptor *)desc;
		if (!stricmp(fieldName, "ES_ID") || !stricmp(fieldName, "binaryID")) {
			ret += sscanf(val, "%hd", &esd->ESID);
		}
		else if (!stricmp(fieldName, "streamPriority")) GET_U8(esd->streamPriority)
		else if (!stricmp(fieldName, "dependsOn_ES_ID")) ret += sscanf(val, "%hd", &esd->dependsOnESID);
		else if (!stricmp(fieldName, "OCR_ES_ID")) ret += sscanf(val, "%hd", &esd->OCRESID);
		else if (!stricmp(fieldName, "URLstring")) {
			esd->URLString = strdup(val);
			ret = 1;
		}
	}
		break;
	case SLConfigDescriptor_Tag:
	{
		u32 ts;
		SLConfigDescriptor *slc = (SLConfigDescriptor*)desc;
		if (!stricmp(fieldName, "predefined")) GET_U8(slc->predefined)
		else if (!stricmp(fieldName, "useAccessUnitStartFlag")) GET_BOOL(slc->useAccessUnitStartFlag)
		else if (!stricmp(fieldName, "useAccessUnitEndFlag")) GET_BOOL(slc->useAccessUnitEndFlag)
		else if (!stricmp(fieldName, "useRandomAccessPointFlag")) GET_BOOL(slc->useRandomAccessPointFlag)
		else if (!stricmp(fieldName, "hasRandomAccessUnitsOnlyFlag")) GET_BOOL(slc->useRandomAccessUnitsOnlyFlag)
		else if (!stricmp(fieldName, "usePaddingFlag")) GET_BOOL(slc->usePaddingFlag)
		else if (!stricmp(fieldName, "useTimeStampsFlag")) GET_BOOL(slc->useTimestampsFlag)
		else if (!stricmp(fieldName, "useIdleFlag")) GET_BOOL(slc->useIdleFlag)
		else if (!stricmp(fieldName, "timeStampResolution")) ret += sscanf(val, "%d", &slc->timestampResolution);
		else if (!stricmp(fieldName, "OCRResolution")) ret += sscanf(val, "%d", &slc->OCRResolution);
		else if (!stricmp(fieldName, "timeStampLength")) GET_U8(slc->timestampLength)
		else if (!stricmp(fieldName, "OCRLength")) GET_U8(slc->OCRLength)
		else if (!stricmp(fieldName, "AU_Length")) GET_U8(slc->AULength)
		else if (!stricmp(fieldName, "instantBitrateLength")) GET_U8(slc->instantBitrateLength)
		else if (!stricmp(fieldName, "degradationPriorityLength")) GET_U8(slc->degradationPriorityLength)
		else if (!stricmp(fieldName, "AU_seqNumLength")) GET_U8(slc->AUSeqNumLength)
		else if (!stricmp(fieldName, "packetSeqNumLength")) GET_U8(slc->packetSeqNumLength)
		else if (!stricmp(fieldName, "timeScale")) ret += sscanf(val, "%d", &slc->timeScale);
		else if (!stricmp(fieldName, "accessUnitDuration")) ret += sscanf(val, "%hd", &slc->AUDuration);
		else if (!stricmp(fieldName, "compositionUnitDuration")) ret += sscanf(val, "%hd", &slc->CUDuration);
		else if (!stricmp(fieldName, "startDecodingTimeStamp")) {
			ret += sscanf(val, "%d", &ts);
			slc->startDTS = (u32) ts;
		}
		else if (!stricmp(fieldName, "startCompositionTimeStamp")) {
			ret += sscanf(val, "%d", &ts);
			slc->startCTS = (u32) ts;
		}
	}	
		break;
	case BIFSConfig_Tag:
	{
		s32 notused;
		BIFSConfigDescriptor *bcd = (BIFSConfigDescriptor*)desc;
		if (!stricmp(val, "auto")) return M4OK;
		if (!stricmp(fieldName, "nodeIDbits")) ret += sscanf(val, "%hd", &bcd->nodeIDbits);
		else if (!stricmp(fieldName, "nodeIDbits")) ret += sscanf(val, "%hd", &bcd->nodeIDbits);
		else if (!stricmp(fieldName, "routeIDbits")) ret += sscanf(val, "%hd", &bcd->routeIDbits);
		else if (!stricmp(fieldName, "protoIDbits")) ret += sscanf(val, "%hd", &bcd->protoIDbits);
		else if (!stricmp(fieldName, "isCommandStream")) GET_BOOL(bcd->isCommandStream)
		else if (!stricmp(fieldName, "pixelMetric") || !stricmp(fieldName, "pixelMetrics")) GET_BOOL(bcd->pixelMetrics)
		else if (!stricmp(fieldName, "pixelWidth")) ret += sscanf(val, "%hd", &bcd->pixelWidth);
		else if (!stricmp(fieldName, "pixelHeight")) ret += sscanf(val, "%hd", &bcd->pixelHeight);
		else if (!stricmp(fieldName, "use3DMeshCoding")) GET_BOOL(notused)
		else if (!stricmp(fieldName, "usePredictiveMFField")) GET_BOOL(notused)
	}
		break;
	case MuxInfoDescriptor_Tag:
	{
		MuxInfoDescriptor *mi = (MuxInfoDescriptor *)desc;
		if (!stricmp(fieldName, "fileName")) GET_STRING(mi->file_name)
		else if (!stricmp(fieldName, "url")) GET_STRING(mi->file_name)
		else if (!stricmp(fieldName, "GroupID")) ret += sscanf(val, "%d", &mi->GroupID);
		else if (!stricmp(fieldName, "streamFormat")) GET_STRING(mi->streamFormat)
		else if (!stricmp(fieldName, "startTime")) ret += sscanf(val, "%d", &mi->startTime);
		else if (!stricmp(fieldName, "duration")) ret += sscanf(val, "%d", &mi->duration);
		else if (!stricmp(fieldName, "useDataReference"))
		{ ret = 1; if (!stricmp(val, "true") || !stricmp(val, "1")) mi->import_flags |= M4TI_USE_DATAREF; }
		else if (!stricmp(fieldName, "noFrameDrop"))
		{ ret = 1; if (!stricmp(val, "true") || !stricmp(val, "1")) mi->import_flags |= M4TI_NO_FRAME_DROP; }
		else if (!stricmp(fieldName, "forceSBR"))
		{ ret = 1; if (!stricmp(val, "true") || !stricmp(val, "1")) mi->import_flags |= M4TI_FORCE_SBR; }

		else if (!stricmp(fieldName, "textNode")) GET_STRING(mi->textNode)
		else if (!stricmp(fieldName, "fontNode")) GET_STRING(mi->fontNode)
	}
		break;
	case DecoderSpecificInfo_Tag:
	{
		DefaultDescriptor *dsi = (DefaultDescriptor*)desc;
		if (!stricmp(fieldName, "info")) {
			/*only parse true hexa strings*/
			if (val[0] == '%') {
				OD_ParseBinData(val, &dsi->data, &dsi->dataLength);
				ret = 1;
			} else if (!strnicmp(val, "file:", 5)) {
				OD_ParseFileData(val+5, &dsi->data, &dsi->dataLength);
				ret = 1;
			}
		}
		if (!stricmp(fieldName, "src")) {
			u32 len = strlen("data:application/octet-string,");
			if (strnicmp(val, "data:application/octet-string,", len)) break;
			val += len;
			/*only parse true hexa strings*/
			if (val[0] == '%') {
				OD_ParseBinData(val, &dsi->data, &dsi->dataLength);
				ret = 1;
			} else if (!strnicmp(val, "file:", 5)) {
				OD_ParseFileData(val+5, &dsi->data, &dsi->dataLength);
				ret = 1;
			}
		}
	}
		break;
	case SegmentDescriptor_Tag:
	{
		SegmentDescriptor *sd = (SegmentDescriptor*)desc;
		if (!stricmp(fieldName, "start") || !stricmp(fieldName, "startTime")) GET_DOUBLE(sd->startTime)
		else if (!stricmp(fieldName, "duration")) GET_DOUBLE(sd->Duration)
		else if (!stricmp(fieldName, "name") || !stricmp(fieldName, "segmentName")) GET_STRING(sd->SegmentName)
	}
		break;
	case UIConfig_Tag:
	{
		UIConfigDescriptor *uic = (UIConfigDescriptor*)desc;
		if (!stricmp(fieldName, "deviceName")) GET_STRING(uic->deviceName)
		else if (!stricmp(fieldName, "termChar")) GET_U8(uic->termChar)
		else if (!stricmp(fieldName, "delChar")) GET_U8(uic->delChar)
		else if (!stricmp(fieldName, "uiData")) {
			/*only parse true hexa strings*/
			if (val[0] == '%') {
				OD_ParseBinData(val, &uic->ui_data, &uic->ui_data_length);
				ret = 1;
			} else if (!strnicmp(val, "file:", 5)) {
				OD_ParseFileData(val+5, &uic->ui_data, &uic->ui_data_length);
				ret = 1;
			} else {
				ret = OD_ParseUIConfig(val, &uic->ui_data, &uic->ui_data_length);
			}
		}
	}
		break;
	case ES_ID_IncTag:
	{
		ES_ID_Inc *inc = (ES_ID_Inc *)desc;
		if (!stricmp(fieldName, "trackID")) ret += sscanf(val, "%d", &inc->trackID);
	}
		break;
	case ES_ID_RefTag:
	{
		ES_ID_Ref *inc = (ES_ID_Ref *)desc;
		if (!stricmp(fieldName, "trackID")) ret += sscanf(val, "%hd", &inc->trackRef);
	}
		break;
	case TextConfig_Tag:
	{
		TextConfigDescriptor *txt = (TextConfigDescriptor*)desc;
		if (!stricmp(fieldName, "3GPPBaseFormat")) GET_U8(txt->Base3GPPFormat)
		else if (!stricmp(fieldName, "MPEGExtendedFormat")) GET_U8(txt->MPEGExtendedFormat)
		else if (!stricmp(fieldName, "profileLevel")) GET_U8(txt->profileLevel)
		else if (!stricmp(fieldName, "durationClock") || !stricmp(fieldName, "timescale") ) GET_U32(txt->timescale)
		else if (!stricmp(fieldName, "layer")) GET_U32(txt->layer)
		else if (!stricmp(fieldName, "text_width")) GET_U32(txt->text_width)
		else if (!stricmp(fieldName, "text_height")) GET_U32(txt->text_height)
		else if (!stricmp(fieldName, "video_width")) {
			GET_U32(txt->video_width)
			txt->has_vid_info = 1;
		}
		else if (!stricmp(fieldName, "video_height")) {
			GET_U32(txt->video_height)
			txt->has_vid_info = 1;
		}
		else if (!stricmp(fieldName, "horizontal_offset")) {
			GET_S32(txt->horiz_offset)
			txt->has_vid_info = 1;
		}
		else if (!stricmp(fieldName, "vertical_offset")) {
			GET_S32(txt->vert_offset)
			txt->has_vid_info = 1;
		}
	}
		break;
	case TextSampleDescriptor_Tag:
	{
		TextSampleDescriptor *sd = (TextSampleDescriptor*)desc;
		if (!stricmp(fieldName, "displayFlags")) GET_U32(sd->displayFlags)
		else if (!stricmp(fieldName, "horiz_justif")) GET_S32(sd->horiz_justif)
		else if (!stricmp(fieldName, "vert_justif")) GET_S32(sd->vert_justif)
		else if (!stricmp(fieldName, "back_color")) GET_S32(sd->back_color)
		else if (!stricmp(fieldName, "top")) GET_S32(sd->default_pos.top)
		else if (!stricmp(fieldName, "bottom")) GET_S32(sd->default_pos.bottom)
		else if (!stricmp(fieldName, "left")) GET_S32(sd->default_pos.left)
		else if (!stricmp(fieldName, "right")) GET_S32(sd->default_pos.right)
		else if (!stricmp(fieldName, "style_font_ID")) GET_S32(sd->default_style.fontID)
		else if (!stricmp(fieldName, "style_font_size")) GET_S32(sd->default_style.font_size)
		else if (!stricmp(fieldName, "style_text_color")) GET_U32(sd->default_style.text_color)
		else if (!stricmp(fieldName, "style_flags")) {
			char szStyles[1024];
			strcpy(szStyles, val);
			strlwr(szStyles);
			if (strstr(szStyles, "bold")) sd->default_style.style_flags |= STF_Bold;
			if (strstr(szStyles, "italic")) sd->default_style.style_flags |= STF_Italic;
			if (strstr(szStyles, "underlined")) sd->default_style.style_flags |= STF_Underlined;
			ret = 1;
		}
		else if (!stricmp(fieldName, "fontID") || !stricmp(fieldName, "fontName")) {
			/*check if we need a new entry*/
			if (!sd->font_count) {
				sd->fonts = malloc(sizeof(FontRecord));
				sd->font_count = 1;
				sd->fonts[0].fontID = 0;
				sd->fonts[0].fontName = NULL;
			} else {
				Bool realloc_fonts = 0;
				if (!stricmp(fieldName, "fontID") && sd->fonts[sd->font_count-1].fontID) realloc_fonts = 1;
				else if (!stricmp(fieldName, "fontName") && sd->fonts[sd->font_count-1].fontName) realloc_fonts = 1;
				if (realloc_fonts) {
					sd->font_count += 1;
					sd->fonts = realloc(sd->fonts, sizeof(FontRecord)*sd->font_count);
					sd->fonts[sd->font_count-1].fontID = 0;
					sd->fonts[sd->font_count-1].fontName = NULL;
				}
			}
			if (!stricmp(fieldName, "fontID")) GET_U32(sd->fonts[sd->font_count-1].fontID)
			if (!stricmp(fieldName, "fontName")) GET_STRING(sd->fonts[sd->font_count-1].fontName)
			ret = 1;
		}
	}
		break;
	}

	return ret ? M4OK : M4BadParam;
}


Bool OD_ParseUIConfig(char *val, char **out_data, u32 *out_data_size)
{
	BitStream *bs;
	if (!strnicmp(val, "HTK:", 4)) {
		char szItem[100];
		s32 pos, bs_start, bs_cur;
		Bool has_word;
		u32 nb_phonems, nbWords = 0;
		bs_start = 0;
		nb_phonems = 0;
		bs = NewBitStream(NULL, 0, BS_WRITE);
		/*we'll write the nb of words later on*/
		BS_WriteInt(bs, 0, 8);
		has_word = 0;
		/*parse all words*/
		val += 4;
		while (1) {
			pos = SP_GetComponent(val, 0, " ;", szItem, 100);
			if (pos>0) val += pos;
			if (!has_word) {
				has_word = 1;
				nbWords++;
				nb_phonems = 0;
				bs_start = (u32) BS_GetPosition(bs);
				/*nb phonems*/
				BS_WriteInt(bs, 0, 8);
				BS_WriteData(bs, szItem, strlen(szItem));
				BS_WriteInt(bs, 0, 8);
				continue;
			}
			if (pos>0) {
		
				nb_phonems ++;
				/*would be nicer with a phone book & use indexes*/
				if (!stricmp(szItem, "vcl")) {
					BS_WriteData(bs, "vc", 2);
				} else {
					BS_WriteData(bs, szItem, 2);
				}

				while (val[0] && (val[0]==' ')) val += 1;
			}

			if ((pos<0) || !val[0] || val[0]==';') {
				if (has_word) {
					has_word = 0;
					bs_cur = (u32) BS_GetPosition(bs);
					BS_Seek(bs, bs_start);
					BS_WriteInt(bs, nb_phonems, 8);
					BS_Seek(bs, bs_cur);
				}
				if ((pos<0) || !val[0]) break;
				val += 1;
				while (val[0] && (val[0]==' ')) val += 1;
			}
		}
		if (nbWords) {
			bs_cur = (u32) BS_GetPosition(bs);
			BS_Seek(bs, 0);
			BS_WriteInt(bs, nbWords, 8);
			BS_Seek(bs, bs_cur);
			BS_GetContent(bs, (unsigned char **)out_data, out_data_size);
		}
		DeleteBitStream(bs);
		return 1;
	}
	return 0;
}
