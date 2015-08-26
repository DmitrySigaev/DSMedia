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


#ifndef _M4_OD_DEV_H
#define _M4_OD_DEV_H

#include <gpac/m4_descriptors.h>

/*read-write OD formatted strings*/
M4Err OD_ReadString(BitStream *bs, char **string, u32 isUTF8);
M4Err OD_WriteString(BitStream *bs, char *string, char isUTF8);

/*descriptors base functions*/
Descriptor *CreateDescriptor(u8 tag);
M4Err DeleteDescriptorList(Chain *descList);
M4Err DelDesc(Descriptor *desc);
M4Err ParseDescriptor(BitStream *bs, Descriptor **desc, u32 *size);
M4Err ReadDesc(BitStream *bs, Descriptor *desc, u32 DescSize);
M4Err writeBaseDescriptor(BitStream *bs, u8 tag, u32 size);
M4Err writeDescList(BitStream *bs, Chain *descList);
M4Err WriteDesc(BitStream *bs, Descriptor *desc);
M4Err CalcSize(Descriptor *desc, u32 *outSize);
M4Err calcDescListSize(Chain *descList, u32 *outSize);

/*handle lazy bitstreams where SizeOfInstance is always encoded on 4 bytes*/
s32 GetSizeFieldSize(u32 size_desc);

/*QoS qualifiers base functions*/
M4Err ParseQoSDescriptor(BitStream *bs, QoS_Default **qos_qual, u32 *qos_size);
void DelQoS(QoS_Default *qos);
M4Err SetQoSSize(QoS_Default *qos);
M4Err WriteQoS(BitStream *bs, QoS_Default *qos);

Descriptor *NewInitialObjectDescriptor();
Descriptor *NewESDescriptor();
Descriptor *NewDecoderConfigDescriptor();
Descriptor *NewSLConfigDescriptor(u8 predef);
Descriptor *NewContentClassificationDescriptor();
Descriptor *NewContentCreatorDateDescriptor();
Descriptor *NewContentCreatorNameDescriptor();
Descriptor *NewContentIdentificationDescriptor();
Descriptor *NewDefaultDescriptor();
Descriptor *NewES_ID_Inc();
Descriptor *NewES_ID_Ref();
Descriptor *NewExpandedTextualDescriptor();
Descriptor *NewExtensionProfileLevelDescriptor();
Descriptor *NewIPIDescriptorPointer();
Descriptor *NewIPMPDescriptor();
Descriptor *NewIPMPDescriptorPointer();
Descriptor *NewKeyWordDescriptor();
Descriptor *NewLanguageDescriptor();
Descriptor *NewM4FInitialObjectDescriptor();
Descriptor *NewM4FObjectDescriptor();
Descriptor *NewObjectDescriptor();
Descriptor *NewOCICreationDateDescriptor();
Descriptor *NewOCICreatorNameDescriptor();
Descriptor *NewProfileLevelIndicationIndexDescriptor();
Descriptor *NewQoSDescriptor();
Descriptor *NewRatingDescriptor();
Descriptor *NewRegistrationDescriptor();
Descriptor *NewShortTextualDescriptor();
Descriptor *NewSmpteCameraPositionDescriptor();
Descriptor *NewSupplementaryContentIdentificationDescriptor();
Descriptor *NewSegmentDescriptor();
Descriptor *NewMediaTimeDescriptor();
Descriptor *NewStreamPLDescriptor();
Descriptor *NewMuxInfoDescriptor();
Descriptor *NewBIFSConfigDescriptor();
Descriptor *NewUIConfigDescriptor();

M4Err DelIOD(InitialObjectDescriptor *iod);
M4Err DelESD(ESDescriptor *esd);
M4Err DelDCD(DecoderConfigDescriptor *dcd);
M4Err DelSL(SLConfigDescriptor *sl);
M4Err DelCC(ContentClassificationDescriptor *ccd);
M4Err DelCCD(ContentCreationDateDescriptor *cdd);
M4Err DelCCN(ContentCreatorNameDescriptor *cnd);
M4Err DelCI(ContentIdentificationDescriptor *cid);
M4Err DelDD(DefaultDescriptor *dd);
M4Err DelES_Inc(ES_ID_Inc *esd_inc);
M4Err DelES_Ref(ES_ID_Ref *esd_ref);
M4Err DelET(ExpandedTextualDescriptor *etd);
M4Err DelEPL(ExtensionProfileLevelDescriptor *pld);
M4Err DelIPIP(IPI_DescrPointer *ipid);
M4Err DelIPMP(IPMP_Descriptor *ipmp);
M4Err DelIPMPD(IPMP_DescrPointer *ipmpd);
M4Err DelKW(KeyWordDescriptor *kwd);
M4Err DelLang(LanguageDescriptor *ld);
M4Err DelM4_IOD(M4F_InitialObjectDescriptor *iod);
M4Err DelM4_OD(M4F_ObjectDescriptor *od);
M4Err DelOD(ObjectDescriptor *od);
M4Err DelOCD(OCICreationDateDescriptor *ocd);
M4Err DelOCN(OCICreatorNameDescriptor *ocn);
M4Err DelPLII(ProfileLevelIndicationIndexDescriptor *plid);
M4Err DelQ(QoS_Descriptor *qos);
M4Err DelRat(RatingDescriptor *rd);
M4Err DelReg(RegistrationDescriptor *reg);
M4Err DelST(ShortTextualDescriptor *std);
M4Err DelSCP(SmpteCameraPositionDescriptor *cpd);
M4Err DelSCI(SupplementaryContentIdentificationDescriptor *scid);
M4Err DelSegDesc(SegmentDescriptor *sd);
M4Err DelMediaTimeDesc(MediaTimeDescriptor *mt);
M4Err DelMuxInfo(MuxInfoDescriptor *mi);
M4Err DelBIFSConfig(BIFSConfigDescriptor *desc);
M4Err DelUIConfig(UIConfigDescriptor *desc);

M4Err ReadIOD(BitStream *bs, InitialObjectDescriptor *iod, u32 DescSize);
M4Err ReadESD(BitStream *bs, ESDescriptor *esd, u32 DescSize);
M4Err ReadDCD(BitStream *bs, DecoderConfigDescriptor *dcd, u32 DescSize);
M4Err ReadSL(BitStream *bs, SLConfigDescriptor *sl, u32 DescSize);
M4Err ReadCC(BitStream *bs, ContentClassificationDescriptor *ccd, u32 DescSize);
M4Err ReadCCD(BitStream *bs, ContentCreationDateDescriptor *cdd, u32 DescSize);
M4Err ReadCCN(BitStream *bs, ContentCreatorNameDescriptor *cnd, u32 DescSize);
M4Err ReadCI(BitStream *bs, ContentIdentificationDescriptor *cid, u32 DescSize);
M4Err ReadDD(BitStream *bs, DefaultDescriptor *dd, u32 DescSize);
M4Err ReadES_Inc(BitStream *bs, ES_ID_Inc *esd_inc, u32 DescSize);
M4Err ReadES_Ref(BitStream *bs, ES_ID_Ref *esd_ref, u32 DescSize);
M4Err ReadET(BitStream *bs, ExpandedTextualDescriptor *etd, u32 DescSize);
M4Err ReadEPL(BitStream *bs, ExtensionProfileLevelDescriptor *pld, u32 DescSize);
M4Err ReadIPIP(BitStream *bs, IPI_DescrPointer *ipid, u32 DescSize);
M4Err ReadIPMP(BitStream *bs, IPMP_Descriptor *ipmp, u32 DescSize);
M4Err ReadIPMPD(BitStream *bs, IPMP_DescrPointer *ipmpd, u32 DescSize);
M4Err ReadKW(BitStream *bs, KeyWordDescriptor *kwd, u32 DescSize);
M4Err ReadLang(BitStream *bs, LanguageDescriptor *ld, u32 DescSize);
M4Err ReadM4_IOD(BitStream *bs, M4F_InitialObjectDescriptor *iod, u32 DescSize);
M4Err ReadM4_OD(BitStream *bs, M4F_ObjectDescriptor *od, u32 DescSize);
M4Err ReadOD(BitStream *bs, ObjectDescriptor *od, u32 DescSize);
M4Err ReadOCD(BitStream *bs, OCICreationDateDescriptor *ocd, u32 DescSize);
M4Err ReadOCN(BitStream *bs, OCICreatorNameDescriptor *ocn, u32 DescSize);
M4Err ReadPLII(BitStream *bs, ProfileLevelIndicationIndexDescriptor *plid, u32 DescSize);
M4Err ReadQ(BitStream *bs, QoS_Descriptor *qos, u32 DescSize);
M4Err ReadRat(BitStream *bs, RatingDescriptor *rd, u32 DescSize);
M4Err ReadReg(BitStream *bs, RegistrationDescriptor *reg, u32 DescSize);
M4Err ReadST(BitStream *bs, ShortTextualDescriptor *std, u32 DescSize);
M4Err ReadSCP(BitStream *bs, SmpteCameraPositionDescriptor *cpd, u32 DescSize);
M4Err ReadSCI(BitStream *bs, SupplementaryContentIdentificationDescriptor *scid, u32 DescSize);
M4Err ReadSegDesc(BitStream *bs, SegmentDescriptor *sd, u32 DescSize);
M4Err ReadMediaTimeDesc(BitStream *bs, MediaTimeDescriptor *mt, u32 DescSize);
M4Err ReadMI(BitStream *bs, MuxInfoDescriptor *mi, u32 DescSize);

M4Err SizeIOD(InitialObjectDescriptor *iod, u32 *outSize);
M4Err SizeESD(ESDescriptor *esd, u32 *outSize);
M4Err SizeDCD(DecoderConfigDescriptor *dcd, u32 *outSize);
M4Err SizeSL(SLConfigDescriptor *sl, u32 *outSize);
M4Err SizeCC(ContentClassificationDescriptor *ccd, u32 *outSize);
M4Err SizeCCD(ContentCreationDateDescriptor *cdd, u32 *outSize);
M4Err SizeCCN(ContentCreatorNameDescriptor *cnd, u32 *outSize);
M4Err SizeCI(ContentIdentificationDescriptor *cid, u32 *outSize);
M4Err SizeDD(DefaultDescriptor *dd, u32 *outSize);
M4Err SizeES_Inc(ES_ID_Inc *esd_inc, u32 *outSize);
M4Err SizeES_Ref(ES_ID_Ref *esd_ref, u32 *outSize);
M4Err SizeET(ExpandedTextualDescriptor *etd, u32 *outSize);
M4Err SizeEPL(ExtensionProfileLevelDescriptor *pld, u32 *outSize);
M4Err SizeIPIP(IPI_DescrPointer *ipid, u32 *outSize);
M4Err SizeIPMP(IPMP_Descriptor *ipmp, u32 *outSize);
M4Err SizeIPMPD(IPMP_DescrPointer *ipmpd, u32 *outSize);
M4Err SizeKW(KeyWordDescriptor *kwd, u32 *outSize);
M4Err SizeLang(LanguageDescriptor *ld, u32 *outSize);
M4Err SizeM4_IOD(M4F_InitialObjectDescriptor *iod, u32 *outSize);
M4Err SizeM4_OD(M4F_ObjectDescriptor *od, u32 *outSize);
M4Err SizeOD(ObjectDescriptor *od, u32 *outSize);
M4Err SizeOCD(OCICreationDateDescriptor *ocd, u32 *outSize);
M4Err SizeOCN(OCICreatorNameDescriptor *ocn, u32 *outSize);
M4Err SizePLII(ProfileLevelIndicationIndexDescriptor *plid, u32 *outSize);
M4Err SizeQ(QoS_Descriptor *qos, u32 *outSize);
M4Err SizeRat(RatingDescriptor *rd, u32 *outSize);
M4Err SizeReg(RegistrationDescriptor *reg, u32 *outSize);
M4Err SizeST(ShortTextualDescriptor *std, u32 *outSize);
M4Err SizeSCP(SmpteCameraPositionDescriptor *cpd, u32 *outSize);
M4Err SizeSCI(SupplementaryContentIdentificationDescriptor *scid, u32 *outSize);
M4Err SizeSegDesc(SegmentDescriptor *sd, u32 *outSize);
M4Err SizeMediaTimeDesc(MediaTimeDescriptor *mt, u32 *outSize);
M4Err SizeMI(MuxInfoDescriptor *mi, u32 *outSize);

M4Err WriteIOD(BitStream *bs, InitialObjectDescriptor *iod);
M4Err WriteESD(BitStream *bs, ESDescriptor *esd);
M4Err WriteDCD(BitStream *bs, DecoderConfigDescriptor *dcd);
M4Err WriteSL(BitStream *bs, SLConfigDescriptor *sl);
M4Err WriteCC(BitStream *bs, ContentClassificationDescriptor *ccd);
M4Err WriteCCD(BitStream *bs, ContentCreationDateDescriptor *cdd);
M4Err WriteCCN(BitStream *bs, ContentCreatorNameDescriptor *cnd);
M4Err WriteCI(BitStream *bs, ContentIdentificationDescriptor *cid);
M4Err WriteDD(BitStream *bs, DefaultDescriptor *dd);
M4Err WriteES_Inc(BitStream *bs, ES_ID_Inc *esd_inc);
M4Err WriteES_Ref(BitStream *bs, ES_ID_Ref *esd_ref);
M4Err WriteET(BitStream *bs, ExpandedTextualDescriptor *etd);
M4Err WriteEPL(BitStream *bs, ExtensionProfileLevelDescriptor *pld);
M4Err WriteIPIP(BitStream *bs, IPI_DescrPointer *ipid);
M4Err WriteIPMP(BitStream *bs, IPMP_Descriptor *ipmp);
M4Err WriteIPMPD(BitStream *bs, IPMP_DescrPointer *ipmpd);
M4Err WriteKW(BitStream *bs, KeyWordDescriptor *kwd);
M4Err WriteLang(BitStream *bs, LanguageDescriptor *ld);
M4Err WriteM4_IOD(BitStream *bs, M4F_InitialObjectDescriptor *iod);
M4Err WriteM4_OD(BitStream *bs, M4F_ObjectDescriptor *od);
M4Err WriteOD(BitStream *bs, ObjectDescriptor *od);
M4Err WriteOCD(BitStream *bs, OCICreationDateDescriptor *ocd);
M4Err WriteOCN(BitStream *bs, OCICreatorNameDescriptor *ocn);
M4Err WritePLII(BitStream *bs, ProfileLevelIndicationIndexDescriptor *plid);
M4Err WriteQ(BitStream *bs, QoS_Descriptor *qos);
M4Err WriteRat(BitStream *bs, RatingDescriptor *rd);
M4Err WriteReg(BitStream *bs, RegistrationDescriptor *reg);
M4Err WriteST(BitStream *bs, ShortTextualDescriptor *std);
M4Err WriteSCP(BitStream *bs, SmpteCameraPositionDescriptor *cpd);
M4Err WriteSCI(BitStream *bs, SupplementaryContentIdentificationDescriptor *scid);
M4Err WriteSegDesc(BitStream *bs, SegmentDescriptor *sd);
M4Err WriteMediaTimeDesc(BitStream *bs, MediaTimeDescriptor *mt);
M4Err WriteMI(BitStream *bs, MuxInfoDescriptor *mi);

Descriptor *NewTextConfigDescriptor();
M4Err DelTextConfig(TextConfigDescriptor *desc);
Descriptor *NewTextSampleDescriptor();
M4Err DelTextSampleDescriptor(TextSampleDescriptor *sd);

/*our commands base functions*/
ODCommand *CreateCom(u8 tag);
M4Err DelCom(ODCommand *com);
M4Err ParseCommand(BitStream *bs, ODCommand **com, u32 *com_size);
M4Err ReadCom(BitStream *bs, ODCommand *com, u32 ComSize);
M4Err ComSize(ODCommand *com, u32 *outSize);
M4Err WriteCom(BitStream *bs, ODCommand *com);

ODCommand *NewODRemove();
ODCommand *NewODUpdate();
ODCommand *NewESDUpdate();
ODCommand *NewESDRemove();
ODCommand *NewIPMPDUpdate();
ODCommand *NewIPMPDRemove();
ODCommand *NewODExecute();
ODCommand *NewIPMPToolDescUpdate();
ODCommand *NewIPMPToolDescRemove();
ODCommand *NewIPMPDescLink(Bool IsLink);
ODCommand *NewBaseCom();

M4Err DelODRemove(ObjectDescriptorRemove *ODRemove);
M4Err DelODUpdate(ObjectDescriptorUpdate *ODUpdate);
M4Err DelESDUpdate(ESDescriptorUpdate *ESDUpdate);
M4Err DelESDRemove(ESDescriptorRemove *ESDRemove);
M4Err DelIPMPDUpdate(IPMPDescriptorUpdate *IPMPDUpdate);
M4Err DelIPMPDRemove(IPMPDescriptorRemove *IPMPDRemove);
M4Err DelODExecute(ObjectDescriptorExecute *ODExec);
M4Err DelBaseCom(BaseODCommand *bcRemove);

M4Err ReadODRemove(BitStream *bs, ObjectDescriptorRemove *odRem, u32 ComSize);
M4Err ReadODUpdate(BitStream *bs, ObjectDescriptorUpdate *odUp, u32 ComSize);
M4Err ReadESDUpdate(BitStream *bs, ESDescriptorUpdate *esdUp, u32 ComSize);
M4Err ReadESDRemove(BitStream *bs, ESDescriptorRemove *esdRem, u32 ComSize);
M4Err ReadIPMPDUpdate(BitStream *bs, IPMPDescriptorUpdate *ipmpUp, u32 ComSize);
M4Err ReadIPMPDRemove(BitStream *bs, IPMPDescriptorRemove *ipmpRem, u32 ComSize);
M4Err ReadODExecute(BitStream *bs, ObjectDescriptorExecute *ODExec, u32 ComSize);
M4Err ReadBaseCom(BitStream *bs, BaseODCommand *bcRem, u32 ComSize);

M4Err SizeODRemove(ObjectDescriptorRemove *odRem, u32 *outSize);
M4Err SizeODUpdate(ObjectDescriptorUpdate *odUp, u32 *outSize);
M4Err SizeESDUpdate(ESDescriptorUpdate *esdUp, u32 *outSize);
M4Err SizeESDRemove(ESDescriptorRemove *esdRem, u32 *outSize);
M4Err SizeIPMPDUpdate(IPMPDescriptorUpdate *ipmpUp, u32 *outSize);
M4Err SizeIPMPDRemove(IPMPDescriptorRemove *ipmpRem, u32 *outSize);
M4Err SizeODExecute(ObjectDescriptorExecute *ODExec, u32 *outSize);
M4Err SizeBaseCom(BaseODCommand *bcRem, u32 *outSize);

M4Err WriteODRemove(BitStream *bs, ObjectDescriptorRemove *odRem);
M4Err WriteODUpdate(BitStream *bs, ObjectDescriptorUpdate *odUp);
M4Err WriteESDUpdate(BitStream *bs, ESDescriptorUpdate *esdUp);
M4Err WriteESDRemove(BitStream *bs, ESDescriptorRemove *esdRem);
M4Err WriteIPMPDUpdate(BitStream *bs, IPMPDescriptorUpdate *ipmpUp);
M4Err WriteIPMPDRemove(BitStream *bs, IPMPDescriptorRemove *ipmpRem);
M4Err WriteODExecute(BitStream *bs, ObjectDescriptorExecute *ODExec);
M4Err WriteBaseCom(BitStream *bs, BaseODCommand *bcRem);



/*dumping*/
M4Err DumpIOD(InitialObjectDescriptor *iod, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpESD(ESDescriptor *esd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpDCD(DecoderConfigDescriptor *dcd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpSL(SLConfigDescriptor *sl, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpCC(ContentClassificationDescriptor *ccd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpCCD(ContentCreationDateDescriptor *cdd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpCCN(ContentCreatorNameDescriptor *cnd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpCI(ContentIdentificationDescriptor *cid, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpDD(DefaultDescriptor *dd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpES_Inc(ES_ID_Inc *esd_inc, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpES_Ref(ES_ID_Ref *esd_ref, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpET(ExpandedTextualDescriptor *etd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpEPL(ExtensionProfileLevelDescriptor *pld, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpIPIP(IPI_DescrPointer *ipid, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpIPMP(IPMP_Descriptor *ipmp, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpIPMPD(IPMP_DescrPointer *ipmpd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpKW(KeyWordDescriptor *kwd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpLang(LanguageDescriptor *ld, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpM4_IOD(M4F_InitialObjectDescriptor *iod, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpM4_OD(M4F_ObjectDescriptor *od, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpOD(ObjectDescriptor *od, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpOCD(OCICreationDateDescriptor *ocd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpOCN(OCICreatorNameDescriptor *ocn, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpPLII(ProfileLevelIndicationIndexDescriptor *plid, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpQ(QoS_Descriptor *qos, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpRat(RatingDescriptor *rd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpReg(RegistrationDescriptor *reg, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpST(ShortTextualDescriptor *std, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpSCP(SmpteCameraPositionDescriptor *cpd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpSCI(SupplementaryContentIdentificationDescriptor *scid, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpSegDesc(SegmentDescriptor *sd, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpMediaTimeDesc(MediaTimeDescriptor *mt, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpMuxInfo(MuxInfoDescriptor *mi, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpBIFSConfig(BIFSConfigDescriptor *dsi, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpUIConfig(UIConfigDescriptor *dsi, FILE *trace, u32 indent, Bool XMTDump);


M4Err DumpODUpdate(ObjectDescriptorUpdate *com, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpODRemove(ObjectDescriptorRemove *com, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpESDUpdate(ESDescriptorUpdate *com, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpESDRemove(ESDescriptorRemove *com, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpIPMPDUpdate(IPMPDescriptorUpdate *com, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpIPMPDRemove(IPMPDescriptorRemove *com, FILE *trace, u32 indent, Bool XMTDump);
M4Err DumpBaseCom(BaseODCommand *com, FILE *trace, u32 indent, Bool XMTDump);

#endif

