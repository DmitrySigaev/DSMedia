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

#include <intern/m4_od_dev.h>
#include <m4_author.h>

#define OD_MAX_TREE		100

#define OD_FORMAT_INDENT( ind_buf, indent ) \
	{ \
		u32 z;	\
		assert(OD_MAX_TREE>indent);	\
		for (z=0; z<indent; z++) ind_buf[z] = ' '; \
		ind_buf[z] = 0; \
	} \

M4Err OD_DumpCommand(void *p, FILE *trace, u32 indent, Bool XMTDump)
{
	ODCommand *com = (ODCommand *)p;

	switch (com->tag) {
	case ODUpdate_Tag:
		return DumpODUpdate((ObjectDescriptorUpdate *)com, trace, indent, XMTDump);
	case ODRemove_Tag:
		return DumpODRemove((ObjectDescriptorRemove *)com, trace, indent, XMTDump);
	case ESDUpdate_Tag:
		return DumpESDUpdate((ESDescriptorUpdate *)com, trace, indent, XMTDump);
	case ESDRemove_Tag:
		return DumpESDRemove((ESDescriptorRemove *)com, trace, indent, XMTDump);
	case IPMPDUpdate_Tag:
		return DumpIPMPDUpdate((IPMPDescriptorUpdate *)com, trace, indent, XMTDump);
	case IPMPDRemove_Tag:
		return DumpIPMPDRemove((IPMPDescriptorRemove *)com, trace, indent, XMTDump);
	default:
		return DumpBaseCom((BaseODCommand *) com, trace, indent, XMTDump);
	}
}


M4Err OD_DumpAU(char *data, u32 dataLength, FILE *trace, u32 indent, Bool XMTDump)
{
	ODCommand *com;
	LPODCODEC odread = OD_NewCodec(OD_READ);
	OD_SetBuffer(odread, data, dataLength);
	OD_DecodeAU(odread);

	while (1) {
		com = OD_GetCommand(odread);
		if (!com) break;

		OD_DumpCommand(com, trace, indent, XMTDump);
		OD_DeleteCommand(&com);
	}
	OD_DeleteCodec(odread);
	return M4OK;
}

M4Err OD_DumpCommandList(Chain *commandList, FILE *trace, u32 indent, Bool XMTDump)
{
	ODCommand *com;
	u32 i;
	for (i=0; i<ChainGetCount(commandList); i++) {
		com = ChainGetEntry(commandList, i);
		OD_DumpCommand(com, trace, indent, XMTDump);
	}
	return M4OK;
}


M4Err OD_DumpDescriptor(void *ptr, FILE *trace, u32 indent, Bool XMTDump)
{
	Descriptor *desc = (Descriptor *)ptr;

	switch (desc->tag) {
	case InitialObjectDescriptor_Tag :
		return DumpIOD((InitialObjectDescriptor *)desc, trace, indent, XMTDump);
	case ESDescriptor_Tag :
		return DumpESD((ESDescriptor *)desc, trace, indent, XMTDump);
	case DecoderConfigDescriptor_Tag :
		return DumpDCD((DecoderConfigDescriptor *)desc, trace, indent, XMTDump);
	case SLConfigDescriptor_Tag:
		return DumpSL((SLConfigDescriptor *)desc, trace, indent, XMTDump);
	case ContentClassification_Tag:
		return DumpCC((ContentClassificationDescriptor *)desc, trace, indent, XMTDump);
	case ContentCreationDate_Tag:
		return DumpCCD((ContentCreationDateDescriptor *)desc, trace, indent, XMTDump);
	case ContentCreatorName_Tag:
		return DumpCCN((ContentCreatorNameDescriptor *)desc, trace, indent, XMTDump);
	case ContentIdentification_Tag:
		return DumpCI((ContentIdentificationDescriptor *)desc, trace, indent, XMTDump);
	case ES_ID_IncTag:
		return DumpES_Inc((ES_ID_Inc *)desc, trace, indent, XMTDump);
	case ES_ID_RefTag:
		return DumpES_Ref((ES_ID_Ref *)desc, trace, indent, XMTDump);
	case ExpandedTextualDescriptor_Tag:
		return DumpET((ExpandedTextualDescriptor *)desc, trace, indent, XMTDump);
	case ExtensionPL_Tag:
		return DumpEPL((ExtensionProfileLevelDescriptor *)desc, trace, indent, XMTDump);
	case IPIPtr_Tag:
	case IPI_DescPtr_Tag:
		return DumpIPIP((IPI_DescrPointer *)desc, trace, indent, XMTDump);
	case IPMP_Tag:
		return DumpIPMP((IPMP_Descriptor *)desc, trace, indent, XMTDump);
	case IPMPPtr_Tag:
		return DumpIPMPD((IPMP_DescrPointer *)desc, trace, indent, XMTDump);
	case KeyWordDescriptor_Tag:
		return DumpKW((KeyWordDescriptor *)desc, trace, indent, XMTDump);
	case LanguageDescriptor_Tag:
		return DumpLang((LanguageDescriptor *)desc, trace, indent, XMTDump);
	case MP4_IOD_Tag:
		return DumpM4_IOD((M4F_InitialObjectDescriptor *)desc, trace, indent, XMTDump);
	case MP4_OD_Tag:
		return DumpM4_OD((M4F_ObjectDescriptor *)desc, trace, indent, XMTDump);
	case ObjectDescriptor_Tag:
		return DumpOD((ObjectDescriptor *)desc, trace, indent, XMTDump);
	case OCI_CreationDate_Tag:
		return DumpOCD((OCICreationDateDescriptor *)desc, trace, indent, XMTDump);
	case OCI_CreatorName_Tag:
		return DumpOCN((OCICreatorNameDescriptor *)desc, trace, indent, XMTDump);
	case PL_IndicationIndex_Tag:
		return DumpPLII((ProfileLevelIndicationIndexDescriptor *)desc, trace, indent, XMTDump);
	case QoS_Tag:
		return DumpQ((QoS_Descriptor *)desc, trace, indent, XMTDump);
	case RatingDescriptor_Tag:
		return DumpRat((RatingDescriptor *)desc, trace, indent, XMTDump);
	case RegistrationDescriptor_Tag:
		return DumpReg((RegistrationDescriptor *)desc, trace, indent, XMTDump);
	case ShortTextualDescriptor_Tag:
		return DumpST((ShortTextualDescriptor *)desc, trace, indent, XMTDump);
	case SmpteCameraPosition_Tag:
		return DumpSCP((SmpteCameraPositionDescriptor *)desc, trace, indent, XMTDump);
	case SuppContentIdentification_Tag:
		return DumpSCI((SupplementaryContentIdentificationDescriptor *)desc, trace, indent, XMTDump);

	case SegmentDescriptor_Tag:
		return DumpSegDesc((SegmentDescriptor *)desc, trace, indent, XMTDump);
	case MediaTimeDescriptor_Tag:
		return DumpMediaTimeDesc((MediaTimeDescriptor *)desc, trace, indent, XMTDump);

	case MuxInfoDescriptor_Tag:
		return DumpMuxInfo((MuxInfoDescriptor *)desc, trace, indent, XMTDump);
	case BIFSConfig_Tag:
		return DumpBIFSConfig((BIFSConfigDescriptor *)desc, trace, indent, XMTDump);
	case UIConfig_Tag:
		return DumpUIConfig((UIConfigDescriptor *)desc, trace, indent, XMTDump);
	default:
		return DumpDD((DefaultDescriptor *)desc, trace, indent, XMTDump);
	}
	return M4OK;
}



static void StartDescDump(FILE *trace, char *descName, u32 indent, Bool XMTDump)
{
	char ind_buf[OD_MAX_TREE];
	OD_FORMAT_INDENT(ind_buf, indent);

	if (!XMTDump) {
		fprintf(trace, "%s {\n", descName);
	} else {
		fprintf(trace, "%s<%s ", ind_buf, descName);
	}
}

static void EndDescDump(FILE *trace, char *descName, u32 indent, Bool XMTDump)
{
	char ind_buf[OD_MAX_TREE];
	OD_FORMAT_INDENT(ind_buf, indent);

	if (!XMTDump) {
		fprintf(trace, "%s}\n", ind_buf);
	} else {
		fprintf(trace, "%s</%s>\n", ind_buf, descName);
	}
}

static void StartAttribute(FILE *trace, char *attName, u32 indent, Bool XMTDump)
{
	char ind_buf[OD_MAX_TREE];
	OD_FORMAT_INDENT(ind_buf, indent);
	if (XMTDump) {
		fprintf(trace, "%s<%s ", ind_buf, attName);
	}
}

static void EndAttribute(FILE *trace, u32 indent, Bool XMTDump)
{
	if (XMTDump) {
		fprintf(trace, ">\n");
	}
}

static void EndLeafAttribute(FILE *trace, u32 indent, Bool XMTDump)
{
	if (!XMTDump) {
	} else {
		fprintf(trace, "/>\n");
	}
}

static void StartElement(FILE *trace, char *attName, u32 indent, Bool XMTDump, Bool IsList)
{
	char ind_buf[OD_MAX_TREE];
	OD_FORMAT_INDENT(ind_buf, indent);
	if (!XMTDump) {
		if (IsList) 
			fprintf(trace, "%s%s [\n", ind_buf, attName);
		else
			fprintf(trace, "%s%s ", ind_buf, attName);
	} else {
		fprintf(trace, "%s<%s>\n", ind_buf, attName);
	}
}

static void EndElement(FILE *trace, char *attName, u32 indent, Bool XMTDump, Bool IsList)
{
	char ind_buf[OD_MAX_TREE];
	OD_FORMAT_INDENT(ind_buf, indent);
	if (!XMTDump) {
		if (IsList) fprintf(trace, "%s]\n", ind_buf);
	} else {
		fprintf(trace, "%s</%s>\n", ind_buf, attName);
	}
}

static void StartAttItem(FILE *trace, char *attName, u32 indent, Bool XMTDump)
{
	char ind_buf[OD_MAX_TREE];
	OD_FORMAT_INDENT(ind_buf, indent);
	if (!XMTDump) {
		fprintf(trace, "%s%s ", ind_buf, attName);
	} else {
		fprintf(trace, "%s=\"", attName);
	}
}
static void EndAttItem(FILE *trace, u32 indent, Bool XMTDump)
{
	if (!XMTDump) {
		fprintf(trace, "\n");
	} else {
		fprintf(trace, "\" ");
	}
}

static void DumpInt(FILE *trace, char *attName, u32  val, u32 indent, Bool XMTDump)
{
	if (!val) return;

	StartAttItem(trace, attName, indent, XMTDump);
	fprintf(trace, "%d", val);
	EndAttItem(trace, indent, XMTDump);
}

static void DumpFloat(FILE *trace, char *attName, Float val, u32 indent, Bool XMTDump)
{
	StartAttItem(trace, attName, indent, XMTDump);
	fprintf(trace, "%g", val);
	EndAttItem(trace, indent, XMTDump);
}

static void DumpDouble(FILE *trace, char *attName, Double val, u32 indent, Bool XMTDump)
{
	StartAttItem(trace, attName, indent, XMTDump);
	fprintf(trace, "%g", val);
	EndAttItem(trace, indent, XMTDump);
}

static void DumpBool(FILE *trace, char *attName, u32  val, u32 indent, Bool XMTDump)
{
	if (!val) return;

	StartAttItem(trace, attName, indent, XMTDump);
	fprintf(trace, "%s", val ? "true" : "false");
	EndAttItem(trace, indent, XMTDump);
}

static void DumpString(FILE *trace, char *attName, char *val, u32 indent, Bool XMTDump)
{
	if (!val) return;
	StartAttItem(trace, attName, indent, XMTDump);
	if (!XMTDump) fprintf(trace, "\"");
	fprintf(trace, "%s", val);
	if (!XMTDump) fprintf(trace, "\"");
	EndAttItem(trace, indent, XMTDump);
}

static void DumpData(FILE *trace, char *name, char *data, u32 dataLength, u32 indent, Bool XMTDump)
{
	u32 i;

	if (!name ||!data) return;

	StartAttItem(trace, name, indent, XMTDump);
	if (XMTDump) fprintf(trace, "data:application/octet-string,");

	for (i=0; i<dataLength; i++) {
		fprintf(trace, "%%");
		fprintf(trace, "%02X", (unsigned char) data[i]);
	}
	EndAttItem(trace, indent, XMTDump);
}


M4Err DumpDescList(Chain *list, FILE *trace, u32 indent, char *ListName, Bool XMTDump)
{
	u32 i, count;
	Descriptor *desc;
	char ind_buf[OD_MAX_TREE];

	if (!list) return M4OK;

	count = ChainGetCount(list);
	if (!count) return M4OK;

	StartElement(trace, ListName, indent, XMTDump, 1);
	indent++;
	OD_FORMAT_INDENT(ind_buf, indent);
	for (i=0; i<count; i++) {
		desc = ChainGetEntry(list, i);
		//add offset if not XMT
		if (!XMTDump) fprintf(trace, "%s", ind_buf);
		OD_DumpDescriptor(desc, trace, indent, XMTDump);
	}
	indent--;
	EndElement(trace, ListName, indent, XMTDump, 1);
	return M4OK;
}

M4Err DumpIOD(InitialObjectDescriptor *iod, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "InitialObjectDescriptor", indent, XMTDump);
	indent++;

	StartAttItem(trace, "objectDescriptorID", indent, XMTDump);
	if (XMTDump) {
		fprintf(trace, "od%d", iod->objectDescriptorID);
		EndAttItem(trace, indent, XMTDump);
		DumpInt(trace, "binaryID", iod->objectDescriptorID, indent, XMTDump);
	} else {
		fprintf(trace, "%d", iod->objectDescriptorID);
		EndAttItem(trace, indent, XMTDump);
	}

	EndAttribute(trace, indent, XMTDump);

	if (XMTDump) {
		StartAttribute(trace, "Profiles", indent, 1);
	}
	DumpInt(trace, "audioProfileLevelIndication", iod->audio_profileAndLevel, indent, XMTDump);
	DumpInt(trace, "visualProfileLevelIndication", iod->visual_profileAndLevel, indent, XMTDump);
	DumpInt(trace, "sceneProfileLevelIndication", iod->scene_profileAndLevel, indent, XMTDump);
	DumpInt(trace, "graphicsProfileLevelIndication", iod->graphics_profileAndLevel, indent, XMTDump);
	DumpInt(trace, "ODProfileLevelIndication", iod->OD_profileAndLevel, indent, XMTDump);
	DumpBool(trace, "includeInlineProfileLevelFlag", iod->inlineProfileFlag, indent, XMTDump);

	if (XMTDump) {
		EndLeafAttribute(trace, indent, XMTDump);
	}
	if (iod->URLString) {
		if (XMTDump) {
			fprintf(trace, "<URL URLstring=\"%s\"/>\n", iod->URLString);
		} else {
			StartAttribute(trace, "URL", indent, XMTDump);
			DumpString(trace, "URLstring", iod->URLString, indent, XMTDump);
			EndAttribute(trace, indent, XMTDump);
		}
	}
		
	if (XMTDump) {
		StartElement(trace, "Descr", indent, XMTDump, 1);
		indent++;
	}
	//ESDescr
	DumpDescList(iod->ESDescriptors, trace, indent, "esDescr", XMTDump);
	DumpDescList(iod->OCIDescriptors, trace, indent, "ociDescr", XMTDump);
	DumpDescList(iod->IPMPDescriptorPointers, trace, indent, "ipmpDescrPtr", XMTDump);

	DumpDescList(iod->extensionDescriptors, trace, indent, "extDescr", XMTDump);
	
	if (XMTDump) {
		indent--;
		EndElement(trace, "Descr", indent, XMTDump, 1);
	}
	indent--;
	EndDescDump(trace, "InitialObjectDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpESD(ESDescriptor *esd, FILE *trace, u32 indent, Bool XMTDump)
{
	MuxInfoDescriptor *mi;
	u32 i;
	StartDescDump(trace, "ES_Descriptor", indent, XMTDump);
	indent++;

	StartAttItem(trace, "ES_ID", indent, XMTDump);
	if (XMTDump) {
		fprintf(trace, "es%d", esd->ESID);
		EndAttItem(trace, indent, XMTDump);
		DumpInt(trace, "binaryID", esd->ESID, indent, XMTDump);
	} else {
		fprintf(trace, "%d", esd->ESID);
		EndAttItem(trace, indent, XMTDump);
	}
	DumpInt(trace, "streamPriority", esd->streamPriority, indent, XMTDump);

	if (XMTDump) {
		if (esd->dependsOnESID) {
			StartAttItem(trace, "dependsOn_ES_ID", indent, XMTDump);
			fprintf(trace, "es%d", esd->dependsOnESID);
			EndAttItem(trace, indent, XMTDump);
		}
	
		if (esd->OCRESID) {
			StartAttItem(trace, "OCR_ES_ID", indent, XMTDump);
			fprintf(trace, "es%d", esd->OCRESID);
			EndAttItem(trace, indent, XMTDump);
		}
	} else {
		if (esd->dependsOnESID) DumpInt(trace, "dependsOn_ES_ID", esd->dependsOnESID, indent, XMTDump);
		if (esd->OCRESID) DumpInt(trace, "OCR_ES_ID", esd->OCRESID, indent, XMTDump);
	}

	EndAttribute(trace, indent, XMTDump);

	if (esd->URLString) {
		StartAttribute(trace, "URL", indent, XMTDump);
		DumpString(trace, "URLstring", esd->URLString, indent, XMTDump);
		EndAttribute(trace, indent, XMTDump);
	}
	if (esd->decoderConfig) {
		StartElement(trace, "decConfigDescr" , indent, XMTDump, 0);
		OD_DumpDescriptor(esd->decoderConfig, trace, indent + (XMTDump ? 1 : 0), XMTDump);
		EndElement(trace, "decConfigDescr" , indent, XMTDump, 0);
	}
	if (esd->slConfig) {
		StartElement(trace, "slConfigDescr" , indent, XMTDump, 0);
		OD_DumpDescriptor(esd->slConfig, trace, indent + (XMTDump ? 1 : 0), XMTDump);
		EndElement(trace, "slConfigDescr" , indent, XMTDump, 0);
	}
	if (esd->ipiPtr) {
		StartElement(trace, "ipiPtr" , indent, XMTDump, 0);
		OD_DumpDescriptor(esd->ipiPtr, trace, indent + (XMTDump ? 1 : 0), XMTDump);
		EndElement(trace, "ipiPtr" , indent, XMTDump, 0);
	}

	DumpDescList(esd->IPIDataSet, trace, indent, "ipIDS", XMTDump);
	DumpDescList(esd->IPMPDescriptorPointers, trace, indent, "ipmpDescPtr", XMTDump);
	DumpDescList(esd->langDesc, trace, indent, "langDescr", XMTDump);

	if (esd->qos) {
		StartElement(trace, "qosDescr" , indent, XMTDump, 0);
		OD_DumpDescriptor(esd->qos, trace, indent + (XMTDump ? 1 : 0), XMTDump);
		EndElement(trace, "qosDescr" , indent, XMTDump, 0);
	}
	if (esd->RegDescriptor) {
		StartElement(trace, "regDescr" , indent, XMTDump, 0);
		OD_DumpDescriptor(esd->RegDescriptor, trace, indent + (XMTDump ? 1 : 0), XMTDump);
		EndElement(trace, "regDescr" , indent, XMTDump, 0);
	}

	mi = NULL;
	for (i=0; i<ChainGetCount(esd->extensionDescriptors); i++) {
		mi = ChainGetEntry(esd->extensionDescriptors, i);
		if (mi->tag == MuxInfoDescriptor_Tag) {
			ChainDeleteEntry(esd->extensionDescriptors, i);
			break;
		}
		mi = NULL;
	}

	DumpDescList(esd->extensionDescriptors, trace, indent, "extDescr", XMTDump);

	if (mi) {
		ChainInsertEntry(esd->extensionDescriptors, mi, i);
		if (XMTDump) {
			OD_DumpDescriptor(mi, trace, indent, 1);
		} else {
			StartElement(trace, "muxInfo" , indent, 0, 0);
			OD_DumpDescriptor(mi, trace, indent, 0);
			EndElement(trace, "muxInfo" , indent, 0, 0);
		}
	}

	indent--;
	EndDescDump(trace, "ES_Descriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpBIFSConfig(BIFSConfigDescriptor *dsi, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, (dsi->version==1) ? "BIFSConfig" : "BIFSv2Config", indent, XMTDump);
	indent++;
	
	if (dsi->version==2) {
		DumpBool(trace, "use3DMeshCoding", 0, indent, XMTDump);
		DumpBool(trace, "usePredictiveMFField", 0, indent, XMTDump);
	}
	DumpInt(trace, "nodeIDbits", dsi->nodeIDbits, indent, XMTDump);
	DumpInt(trace, "routeIDbits", dsi->routeIDbits, indent, XMTDump);
	if (dsi->version==2) DumpInt(trace, "protoIDbits", dsi->protoIDbits, indent, XMTDump);
	if (!dsi->isCommandStream) {
		EndAttribute(trace, indent, XMTDump);
		indent--;
		EndDescDump(trace, (dsi->version==1) ? "BIFSConfig" : "BIFSv2Config", indent, XMTDump);
		return M4NotSupported;
	}
	if (XMTDump) {
		EndAttribute(trace, indent, XMTDump);
		indent++;
		StartDescDump(trace, "commandStream" , indent, XMTDump);
		DumpBool(trace, "pixelMetric", dsi->pixelMetrics, indent, XMTDump);
		if (XMTDump) EndAttribute(trace, indent, XMTDump);
	} else {
		DumpBool(trace, "isCommandStream", 1, indent, XMTDump);
		DumpBool(trace, "pixelMetric", dsi->pixelMetrics, indent, XMTDump);
	}
	if (dsi->pixelWidth && dsi->pixelHeight) {
		if (XMTDump) {
			indent++;
			StartDescDump(trace, "size" , indent, XMTDump);
		}

		DumpInt(trace, "pixelWidth", dsi->pixelWidth, indent, XMTDump);
		DumpInt(trace, "pixelHeight", dsi->pixelHeight, indent, XMTDump);
		if (XMTDump) {
			EndLeafAttribute(trace, indent, XMTDump);
			indent--;
		}
	}

	if (XMTDump) {
		EndDescDump(trace, "commandStream", indent, XMTDump);
		indent--;
	}
	indent--;
	EndDescDump(trace, (dsi->version==1) ? "BIFSConfig" : "BIFSv2Config", indent, XMTDump);
	return M4OK;
}

M4Err DumpRawBIFSConfig(DefaultDescriptor *dsi, FILE *trace, u32 indent, Bool XMTDump, u32 oti)
{
	BitStream *bs;
	u32 flag;

	bs = NewBitStream(dsi->data, dsi->dataLength, BS_READ);

	StartDescDump(trace, (oti==1) ? "BIFSConfig" : "BIFSv2Config", indent, XMTDump);
	indent++;
	
	if (oti==2) {
		DumpBool(trace, "use3DMeshCoding", BS_ReadInt(bs, 1), indent, XMTDump);
		DumpBool(trace, "usePredictiveMFField", BS_ReadInt(bs, 1), indent, XMTDump);
	}
	DumpInt(trace, "nodeIDbits", BS_ReadInt(bs, 5), indent, XMTDump);
	DumpInt(trace, "routeIDbits", BS_ReadInt(bs, 5), indent, XMTDump);

	if (oti==2) 
		DumpInt(trace, "protoIDbits", BS_ReadInt(bs, 5), indent, XMTDump);

	flag = BS_ReadInt(bs, 1);
	if (!flag) {
		DeleteBitStream(bs);
		return M4NotSupported;
	}

	if (XMTDump) {
		EndAttribute(trace, indent, XMTDump);
		indent++;
		StartDescDump(trace, "commandStream" , indent, XMTDump);
		DumpBool(trace, "pixelMetric", BS_ReadInt(bs, 1), indent, XMTDump);
		if (XMTDump) EndAttribute(trace, indent, XMTDump);
	} else {
		DumpBool(trace, "isCommandStream", 1, indent, XMTDump);
		DumpBool(trace, "pixelMetric", BS_ReadInt(bs, 1), indent, XMTDump);
	}

	if (BS_ReadInt(bs, 1)) {

		if (XMTDump) {
			indent++;
			StartDescDump(trace, "size" , indent, XMTDump);
		}

		DumpInt(trace, "pixelWidth", BS_ReadInt(bs, 16), indent, XMTDump);
		DumpInt(trace, "pixelHeight", BS_ReadInt(bs, 16), indent, XMTDump);
		if (XMTDump) {
			EndLeafAttribute(trace, indent, XMTDump);
			indent--;
		}

	}

	if (XMTDump) {
		EndDescDump(trace, "commandStream", indent, XMTDump);
		indent--;
	}
	indent--;
	EndDescDump(trace, (oti==1) ? "BIFSConfig" : "BIFSv2Config", indent, XMTDump);

	DeleteBitStream(bs);
	return M4OK;
}

M4Err DumpUIConfig(UIConfigDescriptor *uid, FILE *trace, u32 indent, Bool XMTDump)
{
	char devName[255];
	u32 i;

	StartDescDump(trace, "UIConfig" , indent, XMTDump);
	indent++;
	DumpString(trace, "deviceName", uid->deviceName, indent, XMTDump);
	
	if (!stricmp(devName, "StringSensor") && uid->termChar) {
		devName[0] = uid->termChar;
		devName[1] = 0;
		DumpString(trace, "termChar", devName, indent, XMTDump);
		devName[0] = uid->delChar;
		DumpString(trace, "delChar", devName, indent, XMTDump);
	}
	if (uid->ui_data_length) {
		if (!stricmp(uid->deviceName, "HTKSensor")) {
			u32 nb_word, nbPhone, c, j;
			BitStream *bs = NewBitStream(uid->ui_data, uid->ui_data_length, BS_READ);
			char szPh[3];
			StartAttItem(trace, "uiData", indent, XMTDump);
			if (!XMTDump) fprintf(trace, "\"");
			fprintf(trace, "HTK:");
			szPh[2] = 0;
			nb_word = BS_ReadInt(bs, 8);
			for (i=0; i<nb_word; i++) {
				nbPhone = BS_ReadInt(bs, 8);
				if (i) fprintf(trace, ";");
				while ((c=BS_ReadInt(bs, 8))) fprintf(trace, "%c", c);
				fprintf(trace, " ");
				for (j=0; j<nbPhone; j++) {
					BS_ReadData(bs, szPh, 2);
					if (j) fprintf(trace, " ");
					if (!stricmp(szPh, "vc")) fprintf(trace, "vcl");
					else fprintf(trace, "%s", szPh);
				}
			}
			if (!XMTDump) fprintf(trace, "\"");
			EndAttItem(trace, indent, XMTDump);
			DeleteBitStream(bs);
		} else {
			DumpData(trace, "uiData", uid->ui_data, uid->ui_data_length, indent, XMTDump);
		}
	}

	indent--;
	EndAttribute(trace, indent, XMTDump);
	EndDescDump(trace, "UIConfig", indent, XMTDump);
	return M4OK;
}

M4Err DumpRawUIConfig(DefaultDescriptor *dsi, FILE *trace, u32 indent, Bool XMTDump, u32 oti)
{
	char devName[255];
	u32 i, len;
	BitStream *bs;

	bs = NewBitStream(dsi->data, dsi->dataLength, BS_READ);

	StartDescDump(trace, "UIConfig" , indent, XMTDump);
	indent++;
	len = BS_ReadInt(bs, 8);
	for (i=0; i<len; i++) devName[i] = BS_ReadInt(bs, 8);
	devName[i] = 0;
	DumpString(trace, "deviceName", devName, indent, XMTDump);
	
	if (!stricmp(devName, "StringSensor") && BS_Available(bs)) {
		devName[0] = BS_ReadInt(bs, 8);
		devName[1] = 0;
		DumpString(trace, "termChar", devName, indent, XMTDump);
		devName[0] = BS_ReadInt(bs, 8);
		DumpString(trace, "delChar", devName, indent, XMTDump);
	}
	len = (u32) BS_Available(bs);
	if (len) {
		if (!stricmp(devName, "HTKSensor")) {
			u32 nb_word, nbPhone, c, j;
			char szPh[3];
			StartAttItem(trace, "uiData", indent, XMTDump);
			if (!XMTDump) fprintf(trace, "\"");
			fprintf(trace, "HTK:");
			szPh[2] = 0;
			nb_word = BS_ReadInt(bs, 8);
			for (i=0; i<nb_word; i++) {
				nbPhone = BS_ReadInt(bs, 8);
				if (i) fprintf(trace, ";");
				while ((c=BS_ReadInt(bs, 8))) fprintf(trace, "%c", c);
				fprintf(trace, " ");
				for (j=0; j<nbPhone; j++) {
					BS_ReadData(bs, szPh, 2);
					if (j) fprintf(trace, " ");
					if (!stricmp(szPh, "vc")) fprintf(trace, "vcl");
					else fprintf(trace, "%s", szPh);
				}
			}
			if (!XMTDump) fprintf(trace, "\"");
			EndAttItem(trace, indent, XMTDump);
		} else {
			char *data = dsi->data;
			data += (u32) BS_GetPosition(bs);
			DumpData(trace, "uiData", data, len, indent, XMTDump);
		}
	}

	indent--;
	EndAttribute(trace, indent, XMTDump);
	EndDescDump(trace, "UIConfig", indent, XMTDump);
	DeleteBitStream(bs);
	return M4OK;
}

M4Err OD_DumpDSI(DefaultDescriptor *dsi, FILE *trace, u32 indent, Bool XMTDump, u32 streamType, u32 oti)
{
	switch (streamType) {
	case M4ST_SCENE:
		if (oti<=2) return DumpRawBIFSConfig(dsi, trace, indent, XMTDump, oti);
		return M4OK;
	case M4ST_INTERACT:
		return DumpRawUIConfig(dsi, trace, indent, XMTDump, oti);
	default:
		return OD_DumpDescriptor(dsi, trace, indent, XMTDump);
	}
}

M4Err DumpDCD(DecoderConfigDescriptor *dcd, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "DecoderConfigDescriptor", indent, XMTDump);
	indent++;

	DumpInt(trace, "objectTypeIndication", dcd->objectTypeIndication, indent, XMTDump);
	DumpInt(trace, "streamType", dcd->streamType, indent, XMTDump);
	DumpInt(trace, "upStream", dcd->upstream, indent, XMTDump);
	DumpInt(trace, "bufferSizeDB", dcd->bufferSizeDB, indent, XMTDump);
	DumpInt(trace, "maxBitrate", dcd->maxBitrate, indent, XMTDump);
	DumpInt(trace, "avgBitrate", dcd->avgBitrate, indent, XMTDump);
	EndAttribute(trace, indent, XMTDump);

	if (dcd->decoderSpecificInfo) {
		if (dcd->decoderSpecificInfo->tag==DecoderSpecificInfo_Tag) {
			if (dcd->decoderSpecificInfo->dataLength) {
				StartElement(trace, "decSpecificInfo" , indent, XMTDump, 0);
				OD_DumpDSI(dcd->decoderSpecificInfo, trace, indent + (XMTDump ? 1 : 0), XMTDump, dcd->streamType, dcd->objectTypeIndication);
				EndElement(trace, "decSpecificInfo" , indent, XMTDump, 0);
			}
		} else {
			StartElement(trace, "decSpecificInfo" , indent, XMTDump, 0);
			OD_DumpDescriptor(dcd->decoderSpecificInfo, trace, indent + (XMTDump ? 1 : 0), XMTDump);
			EndElement(trace, "decSpecificInfo" , indent, XMTDump, 0);
		}
	}
	DumpDescList(dcd->profileLevelIndicationIndexDescriptor, trace, indent, "profileLevelIndicationIndexDescr", XMTDump);
	indent--;
	EndDescDump(trace, "DecoderConfigDescriptor", indent, XMTDump);

	return M4OK;
}

M4Err DumpSL(SLConfigDescriptor *sl, FILE *trace, u32 indent, Bool XMTDump)
{

	StartDescDump(trace, "SLConfigDescriptor", indent, XMTDump);
	EndAttribute(trace, indent, XMTDump);
	indent++;

	if (sl->predefined) {
		StartAttribute(trace, "predefined" , indent, XMTDump);
		DumpInt(trace, "value", sl->predefined, indent, XMTDump);
		EndLeafAttribute(trace, indent, XMTDump);
	}
	if (XMTDump) StartAttribute(trace, "custom" , indent, XMTDump);
	if (!sl->predefined) {
		DumpBool(trace, "useAccessUnitStartFlag", sl->useAccessUnitStartFlag, indent, XMTDump);
		DumpBool(trace, "useAccessUnitEndFlag", sl->useAccessUnitEndFlag, indent, XMTDump);
		DumpBool(trace, "useRandomAccessPointFlag", sl->useRandomAccessPointFlag, indent, XMTDump);
		DumpBool(trace, "usePaddingFlag", sl->usePaddingFlag, indent, XMTDump);
		if (!XMTDump) DumpBool(trace, "useTimeStampsFlag", sl->useTimestampsFlag, indent, XMTDump);
		DumpBool(trace, "useIdleFlag", sl->useIdleFlag, indent, XMTDump);
		if (!XMTDump) DumpBool(trace, "durationFlag", sl->durationFlag, indent, XMTDump);
		DumpInt(trace, "timeStampResolution", sl->timestampResolution, indent, XMTDump);
		DumpInt(trace, "OCRResolution", sl->OCRResolution, indent, XMTDump);
		DumpInt(trace, "timeStampLength", sl->timestampLength, indent, XMTDump);
		DumpInt(trace, "OCRLength", sl->OCRLength, indent, XMTDump);
		DumpInt(trace, "AU_Length", sl->AULength, indent, XMTDump);
		DumpInt(trace, "instantBitrateLength", sl->instantBitrateLength, indent, XMTDump);
		DumpInt(trace, "degradationPriorityLength", sl->degradationPriorityLength, indent, XMTDump);
		DumpInt(trace, "AU_SeqNumLength", sl->AUSeqNumLength, indent, XMTDump);
		DumpInt(trace, "packetSeqNumLength", sl->packetSeqNumLength, indent, XMTDump);
	}
	EndAttribute(trace, indent, XMTDump);

	indent++;
	if (sl->durationFlag) {
		StartAttribute(trace, "Duration" , indent, XMTDump);
		DumpInt(trace, "timescale", sl->timeScale, indent, XMTDump);
		DumpInt(trace, "accessUnitDuration", sl->AUDuration, indent, XMTDump);
		DumpInt(trace, "compositionUnitDuration", sl->CUDuration, indent, XMTDump);
		EndAttribute(trace, indent, XMTDump);
	}
	if (! sl->useTimestampsFlag) {
		StartAttribute(trace, "noUseTimeStamps" , indent, XMTDump);
		DumpInt(trace, "startDecodingTimeStamp", (u32) sl->startDTS, indent, XMTDump);
		DumpInt(trace, "startCompositionTimeStamp", (u32) sl->startCTS, indent, XMTDump);
		EndLeafAttribute(trace, indent, XMTDump);
	}
	indent--;
	if (XMTDump) EndElement(trace, "custom" , indent, XMTDump, 1);

	indent--;
	EndDescDump(trace, "SLConfigDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpCC(ContentClassificationDescriptor *ccd, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "ContentClassificationDescriptor", indent, XMTDump);
	DumpInt(trace, "classificationEntity", ccd->classificationEntity, indent, XMTDump);
	DumpInt(trace, "classificationTable", ccd->classificationTable, indent, XMTDump);
	DumpData(trace, "ccd->contentClassificationData", ccd->contentClassificationData, ccd->dataLength, indent, XMTDump);
	EndAttribute(trace, indent, XMTDump);
	EndDescDump(trace, "ContentClassificationDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpCCD(ContentCreationDateDescriptor *cdd, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "ContentClassificationDescriptor", indent, XMTDump);
	DumpString(trace, "creationDate", cdd->contentCreationDate, indent, XMTDump);
	EndAttribute(trace, indent, XMTDump);
	EndDescDump(trace, "ContentClassificationDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpCCN(ContentCreatorNameDescriptor *cnd, FILE *trace, u32 indent, Bool XMTDump)
{
	u32 i;
	contentCreatorInfo *p;

	StartDescDump(trace, "ContentCreatorNameDescriptor", indent, XMTDump);
	EndAttribute(trace, indent, XMTDump);
	indent++;
	for (i=0; i<ChainGetCount(cnd->ContentCreators); i++) {
		p = ChainGetEntry(cnd->ContentCreators, i);
		StartAttribute(trace, "Creator", indent, XMTDump);
		DumpInt(trace, "languageCode", p->langCode, indent, XMTDump);
		DumpBool(trace, "isUTF8", p->isUTF8, indent, XMTDump);
		DumpString(trace, "Name", p->contentCreatorName, indent, XMTDump);
	}
	indent--;
	EndDescDump(trace, "ContentClassificationDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpCI(ContentIdentificationDescriptor *cid, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "ContentIdentificationDescriptor", indent, XMTDump);
	DumpBool(trace, "protectedContent", cid->protectedContent, indent, XMTDump);
	EndAttribute(trace, indent, XMTDump);
	indent++;
	if (cid->contentTypeFlag) {
		StartAttribute(trace, "contentType", indent, XMTDump);
		DumpInt(trace, "contentType", cid->contentType, indent, XMTDump);
		EndAttribute(trace, indent, XMTDump);
	}
	if (cid->contentIdentifierFlag) {
		StartAttribute(trace, "contentIdentifierType", indent, XMTDump);
		DumpInt(trace, "contentIdentifierType", cid->contentIdentifierType, indent, XMTDump);
		DumpString(trace, "contentIdentifier", cid->contentIdentifier, indent, XMTDump);
		EndAttribute(trace, indent, XMTDump);
	}

	indent--;
	EndDescDump(trace, "ContentIdentificationDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpDD(DefaultDescriptor *dd, FILE *trace, u32 indent, Bool XMTDump)
{
	if (dd->tag == DecoderSpecificInfo_Tag) {
		StartDescDump(trace, "DecoderSpecificInfo", indent, XMTDump);
		indent++;
		if (XMTDump) {
			DumpString(trace, "type", "auto", indent, XMTDump);
			DumpData(trace, "src", dd->data, dd->dataLength, indent, XMTDump);
		} else {
			DumpData(trace, "info", dd->data, dd->dataLength, indent, XMTDump);
		}
		indent--;
		if (XMTDump) {
			EndLeafAttribute(trace, indent, 1);
		} else {
			EndDescDump(trace, "", indent, 0);
		}
	} else {
		StartDescDump(trace, "DefaultDescriptor", indent, XMTDump);
		indent++;
		DumpData(trace, "data", dd->data, dd->dataLength, indent, XMTDump);
		indent--;
		EndLeafAttribute(trace, indent, XMTDump);
	}
	return M4OK;
}

M4Err DumpES_Inc(ES_ID_Inc *esd_inc, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "ES_ID_Inc", indent, XMTDump);
	indent++;
	DumpInt(trace, "trackID", esd_inc->trackID, indent, XMTDump);
	indent--;
	EndAttribute(trace, indent, XMTDump);
	EndDescDump(trace, "ES_ID_Inc", indent, XMTDump);
	return M4OK;
}

M4Err DumpES_Ref(ES_ID_Ref *esd_ref, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "ES_ID_Ref", indent, XMTDump);
	indent++;
	DumpInt(trace, "trackRef", esd_ref->trackRef, indent, XMTDump);
	indent--;
	EndAttribute(trace, indent, XMTDump);
	EndDescDump(trace, "ES_ID_Ref", indent, XMTDump);
	return M4OK;
}

M4Err DumpET(ExpandedTextualDescriptor *etd, FILE *trace, u32 indent, Bool XMTDump)
{
	ETD_ItemText *it1, *it2;
	u32 i;

	StartDescDump(trace, "ExpandedTextualDescriptor", indent, XMTDump);
	indent++;
	DumpInt(trace, "languageCode", etd->langCode, indent, XMTDump);
	DumpBool(trace, "isUTF8", etd->isUTF8, indent, XMTDump);
	DumpString(trace, "nonItemText", etd->NonItemText, indent, XMTDump);
	EndAttribute(trace, indent, XMTDump);
	
	for (i=0; i<ChainGetCount(etd->itemDescriptionList); i++) {
		it1 = ChainGetEntry(etd->itemDescriptionList, i);
		it2 = ChainGetEntry(etd->itemTextList, i);
		StartAttribute(trace, "item", indent, XMTDump);	
		DumpString(trace, "description", it1->text, indent, XMTDump);
		DumpString(trace, "text", it2->text, indent, XMTDump);
		EndLeafAttribute(trace, indent, XMTDump);	
	}
	indent--;
	EndDescDump(trace, "ExpandedTextualDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpEPL(ExtensionProfileLevelDescriptor *pld, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "ExtensionProfileLevelDescriptor", indent, XMTDump);
	indent++;
	DumpInt(trace, "profileLevelIndicationIndex", pld->profileLevelIndicationIndex, indent, XMTDump);
	DumpInt(trace, "ODProfileLevelIndication", pld->ODProfileLevelIndication, indent, XMTDump);
	DumpInt(trace, "sceneProfileLevelIndication", pld->SceneProfileLevelIndication, indent, XMTDump);
	DumpInt(trace, "audioProfileLevelIndication", pld->AudioProfileLevelIndication, indent, XMTDump);
	DumpInt(trace, "visualProfileLevelIndication", pld->VisualProfileLevelIndication, indent, XMTDump);
	DumpInt(trace, "graphicsProfileLevelIndication", pld->GraphicsProfileLevelIndication, indent, XMTDump);
	DumpInt(trace, "MPEGJProfileLevelIndication", pld->MPEGJProfileLevelIndication, indent, XMTDump);
	indent--;
	EndLeafAttribute(trace, indent, XMTDump);
	return M4OK;
}

M4Err DumpIPIP(IPI_DescrPointer *ipid, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "IPI_DescrPointer", indent, XMTDump);
	indent++;
	DumpInt(trace, "IPI_ES_Id", ipid->IPI_ES_Id, indent, XMTDump);
	indent--;
	EndLeafAttribute(trace, indent, XMTDump);
	return M4OK;
}

M4Err DumpIPMP(IPMP_Descriptor *ipmp, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "IPMP_Descriptor", indent, XMTDump);
	indent++;
	StartAttItem(trace, "IPMP_DescriptorID", indent, XMTDump);
	if (XMTDump) {
		fprintf(trace, "ipmpd%d", ipmp->IPMP_DescID);
		EndAttItem(trace, indent, XMTDump);
		DumpInt(trace, "binaryID", ipmp->IPMP_DescID, indent, XMTDump);
	} else {
		fprintf(trace, "%d", ipmp->IPMP_DescID);
		EndAttItem(trace, indent, XMTDump);
	}
	EndAttribute(trace, indent, XMTDump);

	if (ipmp->URLstring) {
		StartAttribute(trace, "IPMPS_URL", indent, XMTDump);
		DumpString(trace, "URLString", ipmp->URLstring, indent, XMTDump);
		EndLeafAttribute(trace, indent, XMTDump);
	} else if (ipmp->IPMP_Data) {
		StartAttribute(trace, "IPMPS_data", indent, XMTDump);
		DumpInt(trace, "type", ipmp->IPMPSType, indent, XMTDump);
		DumpData(trace, "IPMP_data", ipmp->IPMP_Data, ipmp->IPMP_DataLength, indent, XMTDump);
		EndLeafAttribute(trace, indent, XMTDump);
	}
	indent--;
	EndDescDump(trace, "IPMP_Descriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpIPMPD(IPMP_DescrPointer *ipmpd, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "IPMP_DescriptorPointer", indent, XMTDump);
	indent++;
	DumpInt(trace, "IPMP_DescriptorID", ipmpd->IPMPDescrID, indent, XMTDump);
	indent--;
	EndLeafAttribute(trace, indent, XMTDump);	
	return M4OK;
}

M4Err DumpKW(KeyWordDescriptor *kwd, FILE *trace, u32 indent, Bool XMTDump)
{
	KeyWordItem *p;
	u32 i;

	StartDescDump(trace, "KeyWordDescriptor", indent, XMTDump);
	indent++;
	DumpInt(trace, "languageCode", kwd->languageCode, indent, XMTDump);
	DumpBool(trace, "isUTF8", kwd->isUTF8, indent, XMTDump);
	EndAttribute(trace, indent, XMTDump);

	for (i=0; i<ChainGetCount(kwd->keyWordsList); i++) {
		p = ChainGetEntry(kwd->keyWordsList, i);
		StartAttribute(trace, "keyWord", indent, XMTDump);
		DumpString(trace, "value", p->keyWord, indent, XMTDump);
		EndLeafAttribute(trace, indent, XMTDump);
	}
	indent--;
	EndDescDump(trace, "KeyWordDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpLang(LanguageDescriptor *ld, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "LanguageDescriptor", indent, XMTDump);
	indent++;
	DumpInt(trace, "languageCode", ld->langCode, indent, XMTDump);
	indent--;
	EndLeafAttribute(trace, indent, XMTDump);	
	return M4OK;
}

M4Err DumpM4_IOD(M4F_InitialObjectDescriptor *iod, FILE *trace, u32 indent, Bool XMTDump)
{

	
	StartDescDump(trace, "MP4InitialObjectDescriptor", indent, XMTDump);
	indent++;

	StartAttItem(trace, "objectDescriptorID", indent, XMTDump);


	if (XMTDump) {
		fprintf(trace, "od%d", iod->objectDescriptorID);
		EndAttItem(trace, indent, XMTDump);
		DumpInt(trace, "binaryID", iod->objectDescriptorID, indent, XMTDump);
	} else {
		fprintf(trace, "%d", iod->objectDescriptorID);
		EndAttItem(trace, indent, XMTDump);
	}

	EndAttribute(trace, indent, XMTDump);


	if (XMTDump) {
		StartAttribute(trace, "Profile", indent, 1);
	}
	DumpInt(trace, "audioProfileLevelIndication", iod->audio_profileAndLevel, indent, XMTDump);
	DumpInt(trace, "visualProfileLevelIndication", iod->visual_profileAndLevel, indent, XMTDump);
	DumpInt(trace, "sceneProfileLevelIndication", iod->scene_profileAndLevel, indent, XMTDump);
	DumpInt(trace, "graphicsProfileLevelIndication", iod->graphics_profileAndLevel, indent, XMTDump);
	DumpInt(trace, "ODProfileLevelIndication", iod->OD_profileAndLevel, indent, XMTDump);
	DumpBool(trace, "includeInlineProfileLevelFlag", iod->inlineProfileFlag, indent, XMTDump);

	if (XMTDump) {
		EndLeafAttribute(trace, indent, XMTDump);
	}
	if (iod->URLString) {
		StartAttribute(trace, "URL", indent, XMTDump);
		DumpString(trace, "URLstring", iod->URLString, indent, XMTDump);
		EndAttribute(trace, indent, XMTDump);
	}
		
	if (XMTDump) {
		StartElement(trace, "Descr", indent, XMTDump, 1);
		indent++;
	}
	//ESDescr
	if (ChainGetCount(iod->ES_ID_IncDescriptors)) {
		DumpDescList(iod->ES_ID_IncDescriptors, trace, indent, "esDescrInc", XMTDump);
	} else {
		DumpDescList(iod->ES_ID_RefDescriptors, trace, indent, "esDescrRef", XMTDump);
	}
	DumpDescList(iod->OCIDescriptors, trace, indent, "ociDescr", XMTDump);
	DumpDescList(iod->IPMPDescriptorPointers, trace, indent, "ipmpDescrPtr", XMTDump);

	DumpDescList(iod->extensionDescriptors, trace, indent, "extDescr", XMTDump);
	
	if (XMTDump) {
		indent--;
		EndElement(trace, "Descr", indent, XMTDump, 1);
	}
	indent--;
	EndDescDump(trace, "MP4InitialObjectDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpOD(ObjectDescriptor *od, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "ObjectDescriptor", indent, XMTDump);
	indent++;
	StartAttItem(trace, "objectDescriptorID", indent, XMTDump);
	if (XMTDump) {
		fprintf(trace, "od%d", od->objectDescriptorID);
		EndAttItem(trace, indent, XMTDump);
		DumpInt(trace, "binaryID", od->objectDescriptorID, indent, XMTDump);
	} else {
		fprintf(trace, "%d", od->objectDescriptorID);
		EndAttItem(trace, indent, XMTDump);
	}
	EndAttribute(trace, indent, XMTDump);
	if (od->URLString) {
		if (XMTDump) {
			fprintf(trace, "<URL URLstring=\"%s\"/>\n", od->URLString);
		} else {
			StartAttribute(trace, "URL", indent, XMTDump);
			DumpString(trace, "URLstring", od->URLString, indent, XMTDump);
			EndAttribute(trace, indent, XMTDump);
		}
	}
		
	if (XMTDump) {
		StartElement(trace, "Descr", indent, XMTDump, 1);
		indent++;
	}
	//ESDescr
	DumpDescList(od->ESDescriptors, trace, indent, "esDescr", XMTDump);
	DumpDescList(od->OCIDescriptors, trace, indent, "ociDescr", XMTDump);
	DumpDescList(od->IPMPDescriptorPointers, trace, indent, "ipmpDescrPtr", XMTDump);

	DumpDescList(od->extensionDescriptors, trace, indent, "extDescr", XMTDump);
	
	if (XMTDump) {
		indent--;
		EndElement(trace, "Descr", indent, XMTDump, 1);
	}
	indent--;
	EndDescDump(trace, "ObjectDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpM4_OD(M4F_ObjectDescriptor *od, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "MP4ObjectDescriptor", indent, XMTDump);
	indent++;
	StartAttItem(trace, "objectDescriptorID", indent, XMTDump);
	if (XMTDump) {
		fprintf(trace, "od%d", od->objectDescriptorID);
		EndAttItem(trace, indent, XMTDump);
		DumpInt(trace, "binaryID", od->objectDescriptorID, indent, XMTDump);
	} else {
		fprintf(trace, "%d", od->objectDescriptorID);
		EndAttItem(trace, indent, XMTDump);
	}
	EndAttribute(trace, indent, XMTDump);
	if (od->URLString) {
		StartAttribute(trace, "URL", indent, XMTDump);
		DumpString(trace, "URLstring", od->URLString, indent, XMTDump);
		EndAttribute(trace, indent, XMTDump);
	}
		
	if (XMTDump) {
		StartElement(trace, "Descr", indent, XMTDump, 1);
		indent++;
	}
	//ESDescr
	if (ChainGetCount(od->ES_ID_IncDescriptors)) {
		DumpDescList(od->ES_ID_IncDescriptors, trace, indent, "esDescrInc", XMTDump);
	} else {
		DumpDescList(od->ES_ID_RefDescriptors, trace, indent, "esDescrRef", XMTDump);
	}
	DumpDescList(od->OCIDescriptors, trace, indent, "ociDescr", XMTDump);
	DumpDescList(od->IPMPDescriptorPointers, trace, indent, "ipmpDescrPtr", XMTDump);
	DumpDescList(od->extensionDescriptors, trace, indent, "extDescr", XMTDump);
	
	if (XMTDump) {
		indent--;
		EndElement(trace, "Descr", indent, XMTDump, 1);
	}
	indent--;
	EndDescDump(trace, "MP4ObjectDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpOCD(OCICreationDateDescriptor *ocd, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "OCICreationDateDescriptor", indent, XMTDump);
	indent++;
	DumpString(trace, "OCICreationDate", ocd->OCICreationDate, indent, XMTDump);
	indent--;
	EndLeafAttribute(trace, indent, XMTDump);
	return M4OK;
}

M4Err DumpOCN(OCICreatorNameDescriptor *ocn, FILE *trace, u32 indent, Bool XMTDump)
{
	OCICreator *p;
	u32 i;

	StartDescDump(trace, "OCICreatorNameDescriptor", indent, XMTDump);
	indent++;
	for (i=0; i<ChainGetCount(ocn->OCICreators); i++) {
		p = ChainGetEntry(ocn->OCICreators, i);
		StartAttribute(trace, "Creator", indent, XMTDump);
		DumpInt(trace, "languageCode", p->langCode, indent, XMTDump);
		DumpBool(trace, "isUTF8", p->isUTF8, indent, XMTDump);
		DumpString(trace, "name", p->OCICreatorName, indent, XMTDump);
		EndLeafAttribute(trace, indent, XMTDump);
	}
	indent--;
	EndDescDump(trace, "OCICreatorNameDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpPLII(ProfileLevelIndicationIndexDescriptor *plid, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "ProfileLevelIndicationIndexDescriptor", indent, XMTDump);
	indent++;
	DumpInt(trace, "profileLevelIndicationIndex", plid->profileLevelIndicationIndex, indent, XMTDump);
	indent--;
	EndLeafAttribute(trace, indent, XMTDump);
	return M4OK;
}

M4Err DumpQ(QoS_Descriptor *qos, FILE *trace, u32 indent, Bool XMTDump)
{
	QoS_Default *p;
	u32 i;

	StartDescDump(trace, "QoS_Descriptor", indent, XMTDump);
	indent++;

	if (qos->predefined) {
		StartAttribute(trace, "predefined", indent, XMTDump);
		DumpInt(trace, "value", qos->predefined, indent, XMTDump);
		EndLeafAttribute(trace, indent, XMTDump);
	} else {
		for ( i = 0; i < ChainGetCount(qos->QoS_Qualifiers); i++ ) {
			p = (QoS_Default*)ChainGetEntry(qos->QoS_Qualifiers, i);
			switch (p->tag) {
			case QoSMaxDelayTag:
				StartAttribute(trace, "QoSMaxDelay", indent, XMTDump);
				DumpInt(trace, "value", ((QoS_MaxDelay *)p)->MaxDelay, indent, XMTDump);
				EndLeafAttribute(trace, indent, XMTDump);
				break;
			case QoSPrefMaxDelayTag:
				StartAttribute(trace, "QoSPrefMaxDelay", indent, XMTDump);
				DumpInt(trace, "value", ((QoS_PrefMaxDelay *)p)->PrefMaxDelay, indent, XMTDump);
				EndLeafAttribute(trace, indent, XMTDump);
				break;
			case QoSLossProbTag:
				StartAttribute(trace, "QoSLossProb", indent, XMTDump);
				DumpFloat(trace, "value", ((QoS_LossProb *)p)->LossProb, indent, XMTDump);
				EndLeafAttribute(trace, indent, XMTDump);
				break;
			case QoSMaxGapLossTag:
				StartAttribute(trace, "QoSMaxGapLoss", indent, XMTDump);
				DumpInt(trace, "value", ((QoS_MaxGapLoss *)p)->MaxGapLoss, indent, XMTDump);
				EndLeafAttribute(trace, indent, XMTDump);
				break;
			case QoSMaxAUSizeTag:
				StartAttribute(trace, "QoSMaxAUSize", indent, XMTDump);
				DumpInt(trace, "value", ((QoS_MaxAUSize *)p)->MaxAUSize, indent, XMTDump);
				EndLeafAttribute(trace, indent, XMTDump);
				break;
			case QoSAvgAUSizeTag:
				StartAttribute(trace, "QoSAvgAUSize", indent, XMTDump);
				DumpInt(trace, "value", ((QoS_AvgAUSize *)p)->AvgAUSize, indent, XMTDump);
				EndLeafAttribute(trace, indent, XMTDump);
				break;
			case QoSMaxAURateTag:
				StartAttribute(trace, "QoSMaxAURate", indent, XMTDump);
				DumpInt(trace, "value", ((QoS_MaxAURate *)p)->MaxAURate, indent, XMTDump);
				EndLeafAttribute(trace, indent, XMTDump);
				break;
			default:
				StartAttribute(trace, "QoSCustom", indent, XMTDump);
				DumpInt(trace, "tag", p->tag, indent, XMTDump);
				DumpData(trace, "customData", ((QoS_Private *)p)->Data, ((QoS_Private *)p)->DataLength, indent, XMTDump);
				EndLeafAttribute(trace, indent, XMTDump);
				break;
			}
		}
	}
	return M4OK;
}

M4Err DumpRat(RatingDescriptor *rd, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "RatingDescriptor", indent, XMTDump);
	indent++;
	DumpInt(trace, "ratingEntity", rd->ratingEntity, indent, XMTDump);
	DumpInt(trace, "ratingCriteria", rd->ratingCriteria, indent, XMTDump);
	DumpData(trace, "ratingInfo", rd->ratingInfo, rd->infoLength, indent, XMTDump);
	indent--;
	EndLeafAttribute(trace, indent, XMTDump);
	return M4OK;
}

M4Err DumpReg(RegistrationDescriptor *reg, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "RegistrationDescriptor", indent, XMTDump);
	indent++;
	DumpInt(trace, "formatIdentifier", reg->formatIdentifier, indent, XMTDump);
	DumpData(trace, "additionalIdentificationInfo", reg->additionalIdentificationInfo, reg->dataLength, indent, XMTDump);
	indent--;
	EndLeafAttribute(trace, indent, XMTDump);
	return M4OK;
}

M4Err DumpST(ShortTextualDescriptor *std, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "ShortTextualDescriptor", indent, XMTDump);
	indent++;
	DumpInt(trace, "languageCode", std->langCode, indent, XMTDump);
	DumpBool(trace, "isUTF8", std->isUTF8, indent, XMTDump);
	EndAttribute(trace, indent, XMTDump);
	StartAttribute(trace, "event", indent, XMTDump);
	DumpString(trace, "name", std->eventName, indent, XMTDump);
	DumpString(trace, "text", std->eventText, indent, XMTDump);
	EndLeafAttribute(trace, indent, XMTDump);
	indent--;
	EndDescDump(trace, "ShortTextualDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpSCP(SmpteCameraPositionDescriptor *cpd, FILE *trace, u32 indent, Bool XMTDump)
{
	SmpteParam *p;
	u32 i;

	StartDescDump(trace, "SMPTECameraPositionDescriptor", indent, XMTDump);
	indent++;
	DumpInt(trace, "cameraID", cpd->cameraID, indent, XMTDump);
	EndAttribute(trace, indent, XMTDump);
	
	for (i=0; i<ChainGetCount(cpd->ParamList); i++) {
		p = ChainGetEntry(cpd->ParamList, i);
		StartAttribute(trace, "parameter", indent, XMTDump);
		DumpInt(trace, "id", p->paramID, indent, XMTDump);
		DumpInt(trace, "value", p->param, indent, XMTDump);
		EndLeafAttribute(trace, indent, XMTDump);
	}
	indent--;
	EndDescDump(trace, "SMPTECameraPositionDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpSCI(SupplementaryContentIdentificationDescriptor *scid, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "SupplementaryContentIdentification", indent, XMTDump);
	indent++;
	DumpInt(trace, "languageCode", scid->languageCode, indent, XMTDump);
	DumpString(trace, "supplContentIdentiferTitle", scid->supplContentIdentifierTitle, indent, XMTDump);
	DumpString(trace, "supplContentIdentiferValue", scid->supplContentIdentifierValue, indent, XMTDump);
	indent--;
	EndLeafAttribute(trace, indent, XMTDump);
	return M4OK;
}


M4Err DumpSegDesc(SegmentDescriptor *sd, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "SegmentDescriptor", indent, XMTDump);
	indent++;
	DumpDouble(trace, "startTime", sd->startTime, indent, XMTDump);
	DumpDouble(trace, "duration", sd->Duration, indent, XMTDump);
	DumpString(trace, "name", sd->SegmentName, indent, XMTDump);
	indent--;
	if (XMTDump) 
		EndLeafAttribute(trace, indent, XMTDump);
	else
		EndDescDump(trace, "SegmentDescriptor", indent, XMTDump);
	return M4OK;
}

M4Err DumpMediaTimeDesc(MediaTimeDescriptor *mt, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "MediaTimeDescriptor", indent, XMTDump);
	indent++;
	DumpDouble(trace, "mediaTimestamp ", mt->mediaTimeStamp, indent, XMTDump);
	indent--;
	EndLeafAttribute(trace, indent, XMTDump);
	return M4OK;
}


M4Err DumpMuxInfo(MuxInfoDescriptor *mi, FILE *trace, u32 indent, Bool XMTDump)
{
	if (!XMTDump) {
		StartDescDump(trace, "MuxInfo", indent, 0);
		indent++;
		if (mi->file_name) DumpString(trace, "fileName", mi->file_name, indent, 0);
		if (mi->streamFormat) DumpString(trace, "streamFormat", mi->streamFormat, indent, 0);
		if (mi->GroupID) DumpInt(trace, "GroupID", mi->GroupID, indent, 0);
		if (mi->startTime) DumpInt(trace, "startTime", mi->startTime, indent, 0);
		if (mi->duration) DumpInt(trace, "duration", mi->duration, indent, 0);
		if (mi->import_flags & M4TI_USE_DATAREF) DumpBool(trace, "useDataReference", 1, indent, 0);
		if (mi->import_flags & M4TI_NO_FRAME_DROP) DumpBool(trace, "noFrameDrop", 1, indent, 0);
		if (mi->import_flags & M4TI_FORCE_SBR) DumpBool(trace, "forceSBR", 1, indent, 0);
		if (mi->textNode) DumpString(trace, "textNode", mi->textNode, indent, 0);
		if (mi->fontNode) DumpString(trace, "fontNode", mi->fontNode, indent, 0);
		indent--;
		EndDescDump(trace, "MuxInfo", indent, 0);
		return M4OK;
	}

	StartDescDump(trace, "StreamSource", indent, 1);
	indent++;
	if (mi->file_name) DumpString(trace, "url", mi->file_name, indent, 1);
	EndAttribute(trace, indent, 1);

	StartDescDump(trace, "MP4MuxHints", indent, 1);
	if (mi->GroupID) DumpInt(trace, "GroupID", mi->GroupID, indent, 1);
	if (mi->startTime) DumpInt(trace, "startTime", mi->startTime, indent, 1);
	if (mi->duration) DumpInt(trace, "duration", mi->duration, indent, 1);
	if (mi->import_flags & M4TI_USE_DATAREF) DumpBool(trace, "useDataReference", 1, indent, 1);
	if (mi->import_flags & M4TI_NO_FRAME_DROP) DumpBool(trace, "noFrameDrop", 1, indent, 1);
	if (mi->import_flags & M4TI_FORCE_SBR) DumpBool(trace, "forceSBR", 1, indent, 1);

	if (mi->textNode) DumpString(trace, "textNode", mi->textNode, indent, 1);
	if (mi->fontNode) DumpString(trace, "fontNode", mi->fontNode, indent, 1);
	EndLeafAttribute(trace, indent, 1);

	indent--;
	EndElement(trace, "StreamSource" , indent, 1, 1);
	return M4OK;
}


M4Err DumpODUpdate(ObjectDescriptorUpdate *com, FILE *trace, u32 indent, Bool XMTDump)
{
	if (XMTDump) {
		StartDescDump(trace, "ObjectDescriptorUpdate", indent, XMTDump);
		EndAttribute(trace, indent, XMTDump);
		indent++;
		DumpDescList(com->objectDescriptors, trace, indent+1, "OD", XMTDump);
		indent--;
		EndDescDump(trace, "ObjectDescriptorUpdate", indent, XMTDump);
	} else {
		DumpDescList(com->objectDescriptors, trace, indent, "UPDATE OD", XMTDump);
	}
	return M4OK;
}

M4Err DumpODRemove(ObjectDescriptorRemove *com, FILE *trace, u32 indent, Bool XMTDump)
{
	u32 i;

	if (XMTDump) {
		StartDescDump(trace, "ObjectDescriptorRemove", indent, XMTDump);
		indent++;
		StartAttItem(trace, "objectDescriptorId", indent, XMTDump);
	} else {
		char ind_buf[OD_MAX_TREE];
		OD_FORMAT_INDENT(ind_buf, indent);
		fprintf(trace, "%sREMOVE OD [", ind_buf);
	}
	for (i=0; i<com->NbODs; i++) {
		if (i) fprintf(trace, " ");
		fprintf(trace, "%s%d", XMTDump ? "od" : "", com->OD_ID[i]);
	}
	if (XMTDump) {
		EndAttItem(trace, indent, XMTDump);
		indent--;
		EndLeafAttribute(trace, indent, XMTDump);
	} else {
		fprintf(trace, "]\n");
	}
	return M4OK;
}

M4Err DumpESDUpdate(ESDescriptorUpdate *com, FILE *trace, u32 indent, Bool XMTDump)
{
	if (XMTDump) {
		StartDescDump(trace, "ES_DescriptorUpdate", indent, XMTDump);
		StartAttItem(trace, "objectDescriptorId", indent, XMTDump);
		fprintf(trace, "od%d", com->ODID);
		EndAttItem(trace, indent, XMTDump);
		EndAttribute(trace, indent, XMTDump);
	} else {
		char ind_buf[OD_MAX_TREE];
		OD_FORMAT_INDENT(ind_buf, indent);
		fprintf(trace, "%sUPDATE ESD in %d\n", ind_buf, com->ODID);
	}
	indent++;
	DumpDescList(com->ESDescriptors, trace, indent+1, "esDescr", XMTDump);
	indent--;
	if (XMTDump) {
		EndDescDump(trace, "ES_DescriptorUpdate", indent, XMTDump);
	} else {
		fprintf(trace, "\n");
	}
	return M4OK;
}

M4Err DumpESDRemove(ESDescriptorRemove *com, FILE *trace, u32 indent, Bool XMTDump)
{
	u32 i;

	if (XMTDump) {
		StartDescDump(trace, "ES_DescriptorRemove", indent, XMTDump);
		StartAttItem(trace, "objectDescriptorId", indent, XMTDump);
		fprintf(trace, "od%d", com->ODID);
		EndAttItem(trace, indent, XMTDump);
		StartAttItem(trace, "ES_ID", indent, XMTDump);
	} else {
		char ind_buf[OD_MAX_TREE];
		OD_FORMAT_INDENT(ind_buf, indent);
		fprintf(trace, "%sREMOVE ESD FROM %d [", ind_buf, com->ODID);
	}
	for (i=0; i<com->NbESDs; i++) {
		if (i) fprintf(trace, " ");
		if (XMTDump) fprintf(trace, "es");
		fprintf(trace, "%d", com->ES_ID[i]);
	}
	if (XMTDump) {
		EndAttItem(trace, indent, XMTDump);
		indent--;
		EndLeafAttribute(trace, indent, XMTDump);
	} else {
		fprintf(trace, "]\n");
	}
	return M4OK;
}

M4Err DumpIPMPDUpdate(IPMPDescriptorUpdate *com, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "IPMP_DescriptorUpdate", indent, XMTDump);
	EndAttribute(trace, indent, XMTDump);
	indent++;
	DumpDescList(com->IPMPDescList, trace, indent+1, "ipmpDesc", XMTDump);
	indent--;
	EndDescDump(trace, "IPMP_DescriptorUpdate", indent, XMTDump);
	return M4OK;
}

M4Err DumpIPMPDRemove(IPMPDescriptorRemove *com, FILE *trace, u32 indent, Bool XMTDump)
{
	u32 i;

	StartDescDump(trace, "IPMP_DescriptorRemove", indent, XMTDump);
	indent++;

	StartAttItem(trace, "IPMP_DescriptorID", indent, XMTDump);
	for (i=0; i<com->NbIPMPDs; i++) {
		if (i) fprintf(trace, " ");
		fprintf(trace, "%d", com->IPMPDescID[i]);
	}
	EndAttItem(trace, indent, XMTDump);
	indent--;
	EndLeafAttribute(trace, indent, XMTDump);
	return M4OK;
}

M4Err DumpBaseCom(BaseODCommand *com, FILE *trace, u32 indent, Bool XMTDump)
{
	StartDescDump(trace, "BaseODCommand", indent, XMTDump);
	indent++;

	DumpData(trace, "custom", com->data, com->dataSize, indent, XMTDump);
	indent--;
	EndLeafAttribute(trace, indent, XMTDump);
	return M4OK;
}


M4Err OCI_DumpEvent(LPOCIEVENT ev, FILE *trace, u32 indent, Bool XMTDump)
{
	u8 H, M, S, hS, rien;
	u16 evID;
	u32 i;
	Descriptor *desc;

	StartDescDump(trace, "OCI_Event", indent, XMTDump);
	indent++;
	OCIEvent_GetEventID(ev, &evID);
	DumpInt(trace, "eventID", evID, indent, XMTDump);

	OCIEvent_GetStartTime(ev, &H, &M, &S, &hS, &rien);
	DumpBool(trace, "absoluteTimeFlag", rien, indent, XMTDump);
	StartAttItem(trace, "startingTime", indent, XMTDump);
	fprintf(trace, "%d:%d:%d:%d", H, M, S, hS);
	EndAttItem(trace, indent, XMTDump);

	OCIEvent_GetDuration(ev, &H, &M, &S, &hS);
	StartAttItem(trace, "duration", indent, XMTDump);
	fprintf(trace, "%d:%d:%d:%d", H, M, S, hS);
	EndAttItem(trace, indent, XMTDump);
	
	StartElement(trace, "OCIDescr", indent, XMTDump, 1);
	for (i=0; i<OCIEvent_GetDescriptorCount(ev); i++) {
		desc = OCIEvent_GetDescriptor(ev, i);
		OD_DumpDescriptor(desc, trace, indent+1, XMTDump);
	}
	EndElement(trace, "OCIDescr", indent, XMTDump, 1);
	indent--;
	EndDescDump(trace, "OCI_Event", indent, XMTDump);

	return M4OK;
}


M4Err OCI_DumpAU(u8 version, char *au, u32 au_length, FILE *trace, u32 indent, Bool XMTDump)
{
	M4Err e;
	LPOCICODEC codec = OCI_NewCodec(0, version);
	if (!codec) return M4BadParam;

	e = OCI_DecodeAU(codec, au, au_length);

	if (!e) {
		while (1) {
			LPOCIEVENT ev = OCI_GetEvent(codec);
			if (!ev) break;
			OCI_DumpEvent(ev, trace, indent, XMTDump);
		}
	}
	OCI_DeleteCodec(codec);
	return e;
}
