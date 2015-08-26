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

#include <gpac/intern/m4_od_dev.h>


//
//		CONSTRUCTORS
//
Descriptor *CreateDescriptor(u8 tag)
{
	Descriptor *desc;

	switch (tag) {
	case InitialObjectDescriptor_Tag:
		return NewInitialObjectDescriptor();
	case ObjectDescriptor_Tag:
		return NewObjectDescriptor();
	case ESDescriptor_Tag:
		return NewESDescriptor();
	case DecoderConfigDescriptor_Tag:
		return NewDecoderConfigDescriptor();
	case SLConfigDescriptor_Tag:
		//default : we create it without any predefinition...
		return NewSLConfigDescriptor(0);
	case ContentIdentification_Tag:
		return NewContentIdentificationDescriptor();
	case SuppContentIdentification_Tag:
		return NewSupplementaryContentIdentificationDescriptor();
	case IPIPtr_Tag:
		return NewIPIDescriptorPointer();
	//special case for the file format
	case IPI_DescPtr_Tag:
		desc = NewIPIDescriptorPointer();
		if (!desc) return desc;
		desc->tag = IPI_DescPtr_Tag;
		return desc;

	case IPMPPtr_Tag:
		return NewIPMPDescriptorPointer();
	case IPMP_Tag:
		return NewIPMPDescriptor();
	case QoS_Tag:
		return NewQoSDescriptor();
	case RegistrationDescriptor_Tag:
		return NewRegistrationDescriptor();
	case ContentClassification_Tag:
		return NewContentClassificationDescriptor();
	case KeyWordDescriptor_Tag:
		return NewKeyWordDescriptor();
	case RatingDescriptor_Tag:
		return NewRatingDescriptor();
	case LanguageDescriptor_Tag:
		return NewLanguageDescriptor();
	case ShortTextualDescriptor_Tag:
		return NewShortTextualDescriptor();
	case ExpandedTextualDescriptor_Tag:
		return NewExpandedTextualDescriptor();
	case ContentCreatorName_Tag:
		return NewContentCreatorNameDescriptor();
	case ContentCreationDate_Tag:
		return NewContentCreatorDateDescriptor();
	case OCI_CreatorName_Tag:
		return NewOCICreatorNameDescriptor();
	case OCI_CreationDate_Tag:
		return NewOCICreationDateDescriptor();
	case SmpteCameraPosition_Tag:
		return NewSmpteCameraPositionDescriptor();
	case ExtensionPL_Tag:
		return NewExtensionProfileLevelDescriptor();
	case PL_IndicationIndex_Tag:
		return NewProfileLevelIndicationIndexDescriptor();

	//File Format Specific
	case MP4_IOD_Tag:
		return NewM4FInitialObjectDescriptor();
	case MP4_OD_Tag:
		return NewM4FObjectDescriptor();
	case ES_ID_IncTag:
		return NewES_ID_Inc();
	case ES_ID_RefTag:
		return NewES_ID_Ref();

	case ForbiddenZero_Tag:
	case ForbiddenFF_Tag:
		return NULL;

	case SegmentDescriptor_Tag:
		return NewSegmentDescriptor();
	case MediaTimeDescriptor_Tag:
		return NewMediaTimeDescriptor();

	case MuxInfoDescriptor_Tag:
		return NewMuxInfoDescriptor();
	case BIFSConfig_Tag:
		return NewBIFSConfigDescriptor();
	case UIConfig_Tag:
		return NewUIConfigDescriptor();
	case TextConfig_Tag:
		return NewTextConfigDescriptor();
	case TextSampleDescriptor_Tag:
		return NewTextSampleDescriptor();

	//default. The DecSpecInfo is handled as default
	//the appropriate decoder (audio, video, bifs...) has to decode the DecSpecInfo alone !
	case DecoderSpecificInfo_Tag:
		desc = NewDefaultDescriptor();
		if (!desc) return desc;
		desc->tag = DecoderSpecificInfo_Tag;
		return desc;

	default:
		//ISO Reserved
		if ( (tag >= ISO_RESERVED_RANGE_BEGIN) &&
			(tag <= ISO_RESERVED_RANGE_END) ) {
			return NULL;
		}
		desc = NewDefaultDescriptor();
		if (!desc) return desc;
		desc->tag = tag;
		return desc;
	}
}

//
//		DESTRUCTORS
//
M4Err DelDesc(Descriptor *desc)
{
	switch (desc->tag) {
	case InitialObjectDescriptor_Tag :
		return DelIOD((InitialObjectDescriptor *)desc);
	case ESDescriptor_Tag :
		return DelESD((ESDescriptor *)desc);
	case DecoderConfigDescriptor_Tag :
		return DelDCD((DecoderConfigDescriptor *)desc);
	case SLConfigDescriptor_Tag:
		return DelSL((SLConfigDescriptor *)desc);
	case ContentClassification_Tag:
		return DelCC((ContentClassificationDescriptor *)desc);
	case ContentCreationDate_Tag:
		return DelCCD((ContentCreationDateDescriptor *)desc);
	case ContentCreatorName_Tag:
		return DelCCN((ContentCreatorNameDescriptor *)desc);
	case ContentIdentification_Tag:
		return DelCI((ContentIdentificationDescriptor *)desc);
	case ES_ID_IncTag:
		return DelES_Inc((ES_ID_Inc *)desc);
	case ES_ID_RefTag:
		return DelES_Ref((ES_ID_Ref *)desc);
	case ExpandedTextualDescriptor_Tag:
		return DelET((ExpandedTextualDescriptor *)desc);
	case ExtensionPL_Tag:
		return DelEPL((ExtensionProfileLevelDescriptor *)desc);
	case IPIPtr_Tag:
	case IPI_DescPtr_Tag:
		return DelIPIP((IPI_DescrPointer *)desc);
	case IPMP_Tag:
		return DelIPMP((IPMP_Descriptor *)desc);
	case IPMPPtr_Tag:
		return DelIPMPD((IPMP_DescrPointer *)desc);
	case KeyWordDescriptor_Tag:
		return DelKW((KeyWordDescriptor *)desc);
	case LanguageDescriptor_Tag:
		return DelLang((LanguageDescriptor *)desc);
	case MP4_IOD_Tag:
		return DelM4_IOD((M4F_InitialObjectDescriptor *)desc);
	case MP4_OD_Tag:
		return DelM4_OD((M4F_ObjectDescriptor *)desc);
	case ObjectDescriptor_Tag:
		return DelOD((ObjectDescriptor *)desc);
	case OCI_CreationDate_Tag:
		return DelOCD((OCICreationDateDescriptor *)desc);
	case OCI_CreatorName_Tag:
		return DelOCN((OCICreatorNameDescriptor *)desc);
	case PL_IndicationIndex_Tag:
		return DelPLII((ProfileLevelIndicationIndexDescriptor *)desc);
	case QoS_Tag:
		return DelQ((QoS_Descriptor *)desc);
	case RatingDescriptor_Tag:
		return DelRat((RatingDescriptor *)desc);
	case RegistrationDescriptor_Tag:
		return DelReg((RegistrationDescriptor *)desc);
	case ShortTextualDescriptor_Tag:
		return DelST((ShortTextualDescriptor *)desc);
	case SmpteCameraPosition_Tag:
		return DelSCP((SmpteCameraPositionDescriptor *)desc);
	case SuppContentIdentification_Tag:
		return DelSCI((SupplementaryContentIdentificationDescriptor *)desc);


	case SegmentDescriptor_Tag:
		return DelSegDesc((SegmentDescriptor *) desc);
	case MediaTimeDescriptor_Tag:
		return DelMediaTimeDesc((MediaTimeDescriptor *) desc);

	case MuxInfoDescriptor_Tag:
		return DelMuxInfo((MuxInfoDescriptor *)desc);
	case BIFSConfig_Tag:
		return DelBIFSConfig((BIFSConfigDescriptor *)desc);
	case UIConfig_Tag:
		return DelUIConfig((UIConfigDescriptor *)desc);
	case TextConfig_Tag:
		return DelTextConfig((TextConfigDescriptor *)desc);
	case TextSampleDescriptor_Tag:
		return DelTextSampleDescriptor((TextSampleDescriptor*)desc);

	default:
		return DelDD((DefaultDescriptor *)desc);
	}
	return M4OK;
}




//
//		READERS
//
M4Err ReadDesc(BitStream *bs, Descriptor *desc, u32 DescSize)
{
	switch (desc->tag) {
	case InitialObjectDescriptor_Tag :
		return ReadIOD(bs, (InitialObjectDescriptor *)desc, DescSize);
	case ESDescriptor_Tag :
		return ReadESD(bs, (ESDescriptor *)desc, DescSize);
	case DecoderConfigDescriptor_Tag :
		return ReadDCD(bs, (DecoderConfigDescriptor *)desc, DescSize);
	case SLConfigDescriptor_Tag :
		return ReadSL(bs, (SLConfigDescriptor *)desc, DescSize);
	case ContentClassification_Tag:
		return ReadCC(bs, (ContentClassificationDescriptor *)desc, DescSize);
	case ContentCreationDate_Tag:
		return ReadCCD(bs, (ContentCreationDateDescriptor *)desc, DescSize);
	case ContentCreatorName_Tag:
		return ReadCCN(bs, (ContentCreatorNameDescriptor *)desc, DescSize);
	case ContentIdentification_Tag:
		return ReadCI(bs, (ContentIdentificationDescriptor *)desc, DescSize);
	case ExpandedTextualDescriptor_Tag:
		return ReadET(bs, (ExpandedTextualDescriptor *)desc, DescSize);
	case ExtensionPL_Tag:
		return ReadEPL(bs, (ExtensionProfileLevelDescriptor *)desc, DescSize);
	case IPIPtr_Tag:
	case IPI_DescPtr_Tag:
		return ReadIPIP(bs, (IPI_DescrPointer *)desc, DescSize);
	case IPMP_Tag:
		return ReadIPMP(bs, (IPMP_Descriptor *)desc, DescSize);
	case IPMPPtr_Tag:
		return ReadIPMPD(bs, (IPMP_DescrPointer *)desc, DescSize);
	case KeyWordDescriptor_Tag:
		return ReadKW(bs, (KeyWordDescriptor *)desc, DescSize);
	case LanguageDescriptor_Tag:
		return ReadLang(bs, (LanguageDescriptor *)desc, DescSize);
	case ObjectDescriptor_Tag:
		return ReadOD(bs, (ObjectDescriptor *)desc, DescSize);
	case OCI_CreationDate_Tag:
		return ReadOCD(bs, (OCICreationDateDescriptor *)desc, DescSize);
	case OCI_CreatorName_Tag:
		return ReadOCN(bs, (OCICreatorNameDescriptor *)desc, DescSize);
	case PL_IndicationIndex_Tag:
		return ReadPLII(bs, (ProfileLevelIndicationIndexDescriptor *)desc, DescSize);
	case QoS_Tag:
		return ReadQ(bs, (QoS_Descriptor *)desc, DescSize);
	case RatingDescriptor_Tag:
		return ReadRat(bs, (RatingDescriptor *)desc, DescSize);
	case RegistrationDescriptor_Tag:
		return ReadReg(bs, (RegistrationDescriptor *)desc, DescSize);
	case ShortTextualDescriptor_Tag:
		return ReadST(bs, (ShortTextualDescriptor *)desc, DescSize);
	case SmpteCameraPosition_Tag:
		return ReadSCP(bs, (SmpteCameraPositionDescriptor *)desc, DescSize);
	case SuppContentIdentification_Tag:
		return ReadSCI(bs, (SupplementaryContentIdentificationDescriptor *)desc, DescSize);
	//MP4 File Format
	case MP4_IOD_Tag:
		return ReadM4_IOD(bs, (M4F_InitialObjectDescriptor *)desc, DescSize);
	case MP4_OD_Tag:
		return ReadM4_OD(bs, (M4F_ObjectDescriptor *)desc, DescSize);
	case ES_ID_IncTag:
		return ReadES_Inc(bs, (ES_ID_Inc *)desc, DescSize);
	case ES_ID_RefTag:
		return ReadES_Ref(bs, (ES_ID_Ref *)desc, DescSize);

	case SegmentDescriptor_Tag:
		return ReadSegDesc(bs, (SegmentDescriptor *) desc, DescSize);
	case MediaTimeDescriptor_Tag:
		return ReadMediaTimeDesc(bs, (MediaTimeDescriptor *) desc, DescSize);
		
	case MuxInfoDescriptor_Tag:
		return ReadMI(bs, (MuxInfoDescriptor *) desc, DescSize);

	//default:
	case DecoderSpecificInfo_Tag:
	default:
		return ReadDD(bs, (DefaultDescriptor *)desc, DescSize);
	
	}
	return M4OK;
}





//
//		SIZE FUNCTION
//
M4Err CalcSize(Descriptor *desc, u32 *outSize)
{
	switch(desc->tag) {
	case InitialObjectDescriptor_Tag : 
		return SizeIOD((InitialObjectDescriptor *)desc, outSize);
	case ESDescriptor_Tag : 
		return SizeESD((ESDescriptor *)desc, outSize);
	case DecoderConfigDescriptor_Tag : 
		return SizeDCD((DecoderConfigDescriptor *)desc, outSize);
	case SLConfigDescriptor_Tag : 
		return SizeSL((SLConfigDescriptor *)desc, outSize);
	case ContentClassification_Tag:
		return SizeCC((ContentClassificationDescriptor *)desc, outSize);
	case ContentCreationDate_Tag:
		return SizeCCD((ContentCreationDateDescriptor *)desc, outSize);
	case ContentCreatorName_Tag:
		return SizeCCN((ContentCreatorNameDescriptor *)desc, outSize);
	case ContentIdentification_Tag:
		return SizeCI((ContentIdentificationDescriptor *)desc, outSize);
	case ExpandedTextualDescriptor_Tag:
		return SizeET((ExpandedTextualDescriptor *)desc, outSize);
	case ExtensionPL_Tag:
		return SizeEPL((ExtensionProfileLevelDescriptor *)desc, outSize);
	case IPIPtr_Tag:
	case IPI_DescPtr_Tag:
		return SizeIPIP((IPI_DescrPointer *)desc, outSize);
	case IPMP_Tag:
		return SizeIPMP((IPMP_Descriptor *)desc, outSize);
	case IPMPPtr_Tag:
		return SizeIPMPD((IPMP_DescrPointer *)desc, outSize);
	case KeyWordDescriptor_Tag:
		return SizeKW((KeyWordDescriptor *)desc, outSize);
	case LanguageDescriptor_Tag:
		return SizeLang((LanguageDescriptor *)desc, outSize);
	case ObjectDescriptor_Tag:
		return SizeOD((ObjectDescriptor *)desc, outSize);
	case OCI_CreationDate_Tag:
		return SizeOCD((OCICreationDateDescriptor *)desc, outSize);
	case OCI_CreatorName_Tag:
		return SizeOCN((OCICreatorNameDescriptor *)desc, outSize);
	case PL_IndicationIndex_Tag:
		return SizePLII((ProfileLevelIndicationIndexDescriptor *)desc, outSize);
	case QoS_Tag:
		return SizeQ((QoS_Descriptor *)desc, outSize);
	case RatingDescriptor_Tag:
		return SizeRat((RatingDescriptor *)desc, outSize);
	case RegistrationDescriptor_Tag:
		return SizeReg((RegistrationDescriptor *)desc, outSize);
	case ShortTextualDescriptor_Tag:
		return SizeST((ShortTextualDescriptor *)desc, outSize);
	case SmpteCameraPosition_Tag:
		return SizeSCP((SmpteCameraPositionDescriptor *)desc, outSize);
	case SuppContentIdentification_Tag:
		return SizeSCI((SupplementaryContentIdentificationDescriptor *)desc, outSize);
	//MP4File
	case MP4_IOD_Tag:
		return SizeM4_IOD((M4F_InitialObjectDescriptor *)desc, outSize);
	case MP4_OD_Tag:
		return SizeM4_OD((M4F_ObjectDescriptor *)desc, outSize);
	case ES_ID_IncTag:
		return SizeES_Inc((ES_ID_Inc *)desc, outSize);
	case ES_ID_RefTag:
		return SizeES_Ref((ES_ID_Ref *)desc, outSize);

	case SegmentDescriptor_Tag:
		return SizeSegDesc((SegmentDescriptor *) desc, outSize);
	case MediaTimeDescriptor_Tag:
		return SizeMediaTimeDesc((MediaTimeDescriptor *) desc, outSize);

	case MuxInfoDescriptor_Tag:
		return SizeMI((MuxInfoDescriptor *) desc, outSize);
		
	default:
		return SizeDD((DefaultDescriptor *)desc, outSize);
	}
	return M4OK;
}


//
//		WRITERS
//
M4Err WriteDesc(BitStream *bs, Descriptor *desc)
{
	switch(desc->tag) {
	case InitialObjectDescriptor_Tag : 
		return WriteIOD(bs, (InitialObjectDescriptor *)desc);
	case ESDescriptor_Tag : 
		return WriteESD(bs, (ESDescriptor *)desc);
	case DecoderConfigDescriptor_Tag : 
		return WriteDCD(bs, (DecoderConfigDescriptor *)desc);
	case SLConfigDescriptor_Tag : 
		return WriteSL(bs, (SLConfigDescriptor *)desc);
	case ContentClassification_Tag:
		return WriteCC(bs, (ContentClassificationDescriptor *)desc);
	case ContentCreationDate_Tag:
		return WriteCCD(bs, (ContentCreationDateDescriptor *)desc);
	case ContentCreatorName_Tag:
		return WriteCCN(bs, (ContentCreatorNameDescriptor *)desc);
	case ContentIdentification_Tag:
		return WriteCI(bs, (ContentIdentificationDescriptor *)desc);

	case ES_ID_IncTag:
		return WriteES_Inc(bs, (ES_ID_Inc *)desc);
	case ES_ID_RefTag:
		return WriteES_Ref(bs, (ES_ID_Ref *)desc);
	case ExpandedTextualDescriptor_Tag:
		return WriteET(bs, (ExpandedTextualDescriptor *)desc);
	case ExtensionPL_Tag:
		return WriteEPL(bs, (ExtensionProfileLevelDescriptor *)desc);
	case IPIPtr_Tag:
	case IPI_DescPtr_Tag:
		return WriteIPIP(bs, (IPI_DescrPointer *)desc);
	case IPMP_Tag:
		return WriteIPMP(bs, (IPMP_Descriptor *)desc);
	case IPMPPtr_Tag:
		return WriteIPMPD(bs, (IPMP_DescrPointer *)desc);
	case KeyWordDescriptor_Tag:
		return WriteKW(bs, (KeyWordDescriptor *)desc);
	case LanguageDescriptor_Tag:
		return WriteLang(bs, (LanguageDescriptor *)desc);
	case MP4_IOD_Tag:
		return WriteM4_IOD(bs, (M4F_InitialObjectDescriptor *)desc);
	case MP4_OD_Tag:
		return WriteM4_OD(bs, (M4F_ObjectDescriptor *)desc);
	case ObjectDescriptor_Tag:
		return WriteOD(bs, (ObjectDescriptor *)desc);
	case OCI_CreationDate_Tag:
		return WriteOCD(bs, (OCICreationDateDescriptor *)desc);
	case OCI_CreatorName_Tag:
		return WriteOCN(bs, (OCICreatorNameDescriptor *)desc);
	case PL_IndicationIndex_Tag:
		return WritePLII(bs, (ProfileLevelIndicationIndexDescriptor *)desc);
	case QoS_Tag:
		return WriteQ(bs, (QoS_Descriptor *)desc);
	case RatingDescriptor_Tag:
		return WriteRat(bs, (RatingDescriptor *)desc);
	case RegistrationDescriptor_Tag:
		return WriteReg(bs, (RegistrationDescriptor *)desc);
	case ShortTextualDescriptor_Tag:
		return WriteST(bs, (ShortTextualDescriptor *)desc);
	case SmpteCameraPosition_Tag:
		return WriteSCP(bs, (SmpteCameraPositionDescriptor *)desc);
	case SuppContentIdentification_Tag:
		return WriteSCI(bs, (SupplementaryContentIdentificationDescriptor *)desc);

	case SegmentDescriptor_Tag:
		return WriteSegDesc(bs, (SegmentDescriptor *) desc);
	case MediaTimeDescriptor_Tag:
		return WriteMediaTimeDesc(bs, (MediaTimeDescriptor *) desc);

	case MuxInfoDescriptor_Tag:
		return WriteMI(bs, (MuxInfoDescriptor *) desc);

	default:
		return WriteDD(bs, (DefaultDescriptor *)desc);
	}
	return M4OK;
}
