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

void DelQoS(QoS_Default *qos)
{
	switch (qos->tag) {
	case QoSMaxDelayTag :
		free((QoS_MaxDelay *)qos);
		return;

	case QoSPrefMaxDelayTag:
		free((QoS_PrefMaxDelay *)qos);
		return;

	case QoSLossProbTag:
		free((QoS_LossProb *)qos);
		return;

	case QoSMaxGapLossTag:
		free((QoS_MaxGapLoss *)qos);
		return;

	case QoSMaxAUSizeTag:
		free((QoS_MaxAUSize *)qos);
		return;
		
	case QoSAvgAUSizeTag:
		free((QoS_AvgAUSize *)qos);
		return;

	case QoSMaxAURateTag:
		free((QoS_MaxAURate *)qos);
		return;

	default:
		if ( ((QoS_Private *)qos)->DataLength)
			free(((QoS_Private *)qos)->Data);
		free( (QoS_Private *) qos);
		return;
	}
}


M4Err SetQoSSize(QoS_Default *qos)
{
	if (! qos) return M4BadParam;
	qos->size = 0;

	switch (qos->tag) {
	case QoSMaxDelayTag:
	case QoSPrefMaxDelayTag:
	case QoSLossProbTag:
	case QoSMaxGapLossTag:
	case QoSMaxAUSizeTag:
	case QoSAvgAUSizeTag:
	case QoSMaxAURateTag:
		qos->size += 4;
		return M4OK;

	case 0x00:
	case 0xFF:
		return M4ISOForbiddenQoS;

	default :
		qos->size += ((QoS_Private *)qos)->DataLength;
	}
	return M4OK;
}

M4Err WriteQoS(BitStream *bs, QoS_Default *qos)
{
	M4Err e;
	if (!bs || !qos) return M4BadParam;
	
	e = SetQoSSize(qos);
	if (e) return e;
	e = writeBaseDescriptor(bs, qos->tag, qos->size);
	if (e) return e;

	switch (qos->tag) {
	case QoSMaxDelayTag:
		BS_WriteInt(bs, ((QoS_MaxDelay *)qos)->MaxDelay, 32);
		break;

	case QoSPrefMaxDelayTag:
		BS_WriteInt(bs, ((QoS_PrefMaxDelay *)qos)->PrefMaxDelay, 32);
		break;

	case QoSLossProbTag:
		//FLOAT (double on 4 bytes)
		BS_WriteFloat(bs, ((QoS_LossProb *)qos)->LossProb);
		break;

	case QoSMaxGapLossTag:
		BS_WriteInt(bs, ((QoS_MaxGapLoss *)qos)->MaxGapLoss, 32);
		break;

	case QoSMaxAUSizeTag:
		BS_WriteInt(bs, ((QoS_MaxAUSize *)qos)->MaxAUSize, 32);
		break;

	case QoSAvgAUSizeTag:
		BS_WriteInt(bs, ((QoS_AvgAUSize *)qos)->AvgAUSize, 32);
		break;

	case QoSMaxAURateTag:
		BS_WriteInt(bs, ((QoS_MaxAURate *)qos)->MaxAURate, 32);
		break;

	case 0x00:
	case 0xFF:
		return M4ISOForbiddenQoS;

	default:
		//we defined the private qos...
		BS_WriteData(bs, ((QoS_Private *)qos)->Data, ((QoS_Private *)qos)->DataLength);
		break;
	}
	return M4OK;
}



M4Err ParseQoSDescriptor(BitStream *bs, QoS_Default **qos_qual, u32 *qual_size)
{
	u32 tag, qos_size, val, bytesParsed, sizeHeader;
	QoS_Default *newQoS;

	//tag
	tag = BS_ReadInt(bs, 8);
	bytesParsed = 1;
	//size of instance
	qos_size = 0;
	sizeHeader = 0;
	do {
		val = BS_ReadInt(bs, 8);
		sizeHeader++;
		qos_size <<= 7;
		qos_size |= val & 0x7F;
	} while ( val & 0x80 );
	bytesParsed += sizeHeader;

	//Payload
	switch (tag) {
	case QoSMaxDelayTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_MaxDelay));
		((QoS_MaxDelay *)newQoS)->MaxDelay = BS_ReadInt(bs, 32);
		bytesParsed += 4;
		break;

	case QoSPrefMaxDelayTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_PrefMaxDelay));
		((QoS_PrefMaxDelay *)newQoS)->PrefMaxDelay = BS_ReadInt(bs, 32);
		bytesParsed += 4;
		break;

	case QoSLossProbTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_LossProb));
		((QoS_LossProb *)newQoS)->LossProb = BS_ReadFloat(bs);
		bytesParsed += 4;
		break;

	case QoSMaxGapLossTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_MaxGapLoss));
		((QoS_MaxGapLoss *)newQoS)->MaxGapLoss = BS_ReadInt(bs, 32);
		bytesParsed += 4;
		break;

	case QoSMaxAUSizeTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_MaxAUSize));
		((QoS_MaxAUSize *)newQoS)->MaxAUSize = BS_ReadInt(bs, 32);
		bytesParsed += 4;
		break;

	case QoSAvgAUSizeTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_AvgAUSize));
		((QoS_AvgAUSize *)newQoS)->AvgAUSize = BS_ReadInt(bs, 32);
		bytesParsed += 4;
		break;

	case QoSMaxAURateTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_MaxAURate));
		((QoS_MaxAURate *)newQoS)->MaxAURate = BS_ReadInt(bs, 32);
		bytesParsed += 4;
		break;

	case 0x00:
	case 0xFF:
		return M4ISOForbiddenQoS;

	default:
		//we defined the private qos...
		newQoS = (QoS_Default *) malloc(sizeof(QoS_Private));
		((QoS_Private *)newQoS)->DataLength = qos_size;
		BS_ReadData(bs, ((QoS_Private *)newQoS)->Data, ((QoS_Private *)newQoS)->DataLength);
		bytesParsed += ((QoS_Private *)newQoS)->DataLength;
		break;
	}
	newQoS->size = qos_size;
	newQoS->tag = tag;
	if (bytesParsed != 1 + qos_size + sizeHeader) {
		DelQoS(newQoS);
		return M4ReadDescriptorFailed;
	}
	*qos_qual = newQoS;
	*qual_size = bytesParsed;
	return M4OK;
}


QoS_Default *NewQoS(u8 tag)
{
	QoS_Default *newQoS;

	switch (tag) {
	case QoSMaxDelayTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_MaxDelay));
		((QoS_MaxDelay *)newQoS)->MaxDelay = 0;
		((QoS_MaxDelay *)newQoS)->size = 4;
		break;

	case QoSPrefMaxDelayTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_PrefMaxDelay));
		((QoS_PrefMaxDelay *)newQoS)->PrefMaxDelay = 0;
		((QoS_PrefMaxDelay *)newQoS)->size = 4;
		break;

	case QoSLossProbTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_LossProb));
		((QoS_LossProb *)newQoS)->LossProb = 0;
		((QoS_LossProb *)newQoS)->size = 4;
		break;

	case QoSMaxGapLossTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_MaxGapLoss));
		((QoS_MaxGapLoss *)newQoS)->MaxGapLoss = 0;
		((QoS_MaxGapLoss *)newQoS)->size = 4;
		break;

	case QoSMaxAUSizeTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_MaxAUSize));
		((QoS_MaxAUSize *)newQoS)->MaxAUSize = 0;
		((QoS_MaxAUSize *)newQoS)->size = 0;
		break;

	case QoSAvgAUSizeTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_AvgAUSize));
		((QoS_AvgAUSize *)newQoS)->AvgAUSize = 0;
		((QoS_AvgAUSize *)newQoS)->size = 4;
		break;

	case QoSMaxAURateTag:
		newQoS = (QoS_Default *) malloc(sizeof(QoS_MaxAURate));
		((QoS_MaxAURate *)newQoS)->MaxAURate = 0;
		((QoS_MaxAURate *)newQoS)->size = 4;
		break;

	case 0x00:
	case 0xFF:
		return NULL;

	default:
		//we defined the private qos...
		newQoS = (QoS_Default *) malloc(sizeof(QoS_Private));
		((QoS_Private *)newQoS)->DataLength = 0;
		((QoS_Private *)newQoS)->Data = NULL;
		break;
	}
	newQoS->tag = tag;
	return newQoS;
}

//
//	Constructor
//
Descriptor *NewQoSDescriptor()
{
	QoS_Descriptor *newDesc = (QoS_Descriptor *) malloc(sizeof(QoS_Descriptor));
	if (!newDesc) return NULL;
	newDesc->QoS_Qualifiers = NewChain();
	newDesc->predefined = 0;
	newDesc->tag = QoS_Tag;
	return (Descriptor *) newDesc;
}

//
//	Desctructor
//
M4Err DelQ(QoS_Descriptor *qos)
{
	if (!qos) return M4BadParam;

	while (ChainGetCount(qos->QoS_Qualifiers)) {
		QoS_Default *tmp = (QoS_Default*)ChainGetEntry(qos->QoS_Qualifiers, 0);
		DelQoS(tmp);
		ChainDeleteEntry(qos->QoS_Qualifiers, 0);
	}
	DeleteChain(qos->QoS_Qualifiers);
	return M4OK;
}


//
//		Reader
//
M4Err ReadQ(BitStream *bs, QoS_Descriptor *qos, u32 DescSize)
{
	M4Err e;
	QoS_Default *tmp;
	u32 tmp_size, nbBytes = 0;
	if (!qos) return M4BadParam;

	qos->predefined = BS_ReadInt(bs, 8);
	nbBytes += 1;

	if (qos->predefined) {
		if (nbBytes != DescSize) return M4ReadDescriptorFailed;		
		return M4OK;
	}

	while (nbBytes < DescSize) {
		tmp = NULL;
		e = ParseQoSDescriptor(bs, &tmp, &tmp_size);
		if (!tmp) return M4ReadDescriptorFailed;
		e = ChainAddEntry(qos->QoS_Qualifiers, tmp);
		if (e) return e;
		nbBytes += tmp_size;
	}
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}




//
//		Size
//
M4Err SizeQ(QoS_Descriptor *qos, u32 *outSize)
{
	M4Err e;
	u32 i;
	QoS_Default *tmp;

	if (!qos) return M4BadParam;
	
	*outSize = 1;
	
	for ( i = 0; i < ChainGetCount(qos->QoS_Qualifiers); i++ ) {
		tmp = (QoS_Default*)ChainGetEntry(qos->QoS_Qualifiers, i);
		if (tmp) {

			e = SetQoSSize(tmp);
			if (e) return e;
			*outSize += tmp->size + GetSizeFieldSize(tmp->size);
		}
	}
	return M4OK;
}

//
//		Writer
//
M4Err WriteQ(BitStream *bs, QoS_Descriptor *qos)
{
	M4Err e;
	u32 size, i;
	QoS_Default *tmp;
	if (!qos) return M4BadParam;

	e = CalcSize((Descriptor *)qos, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, qos->tag, size);
	if (e) return e;

	BS_WriteInt(bs, qos->predefined, 8);

	if (! qos->predefined) {
		for ( i = 0; i < ChainGetCount(qos->QoS_Qualifiers); i++ ) {
			tmp = (QoS_Default*)ChainGetEntry(qos->QoS_Qualifiers, i);
			if (tmp) {
				e = WriteQoS(bs, tmp);
				if (e) return e;
			} 
		}
	}
	return M4OK;
}


