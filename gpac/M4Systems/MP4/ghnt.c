/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / ISO Media File Format sub-project
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

#include <gpac/intern/m4_isomedia_dev.h>

Atom *ghnt_New()
{
	HintSampleEntryAtom *tmp = (HintSampleEntryAtom *) malloc(sizeof(HintSampleEntryAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(HintSampleEntryAtom));
	tmp->HintDataTable = NewChain();
	if (!tmp->HintDataTable) {
		free(tmp);
		return NULL;
	}
	//this type is used internally for protocols that share the same base entry
	//currently only RTP uses this, but a flexMux could use this entry too...
	tmp->type = GenericHintSampleEntryAtomType;
	tmp->HintTrackVersion = 1;
	tmp->LastCompatibleVersion = 1;
	return (Atom *)tmp;
}

void ghnt_del(Atom *s)
{
	HintSampleEntryAtom *ptr;
	
	ptr = (HintSampleEntryAtom *)s;
	DeleteAtomList(ptr->HintDataTable);
	if (ptr->w_sample) Del_HintSample(ptr->w_sample);
	free(ptr);
}

M4Err ghnt_Read(Atom *s, BitStream *bs, u64 *read)
{
	Atom *a;
	u64 sr;
	M4Err e;
	HintSampleEntryAtom *ptr = (HintSampleEntryAtom *)s;
	if (ptr == NULL) return M4BadParam;

	BS_ReadData(bs, ptr->reserved, 6);
	ptr->dataReferenceIndex = BS_ReadInt(bs, 16);
	*read += 8;
	ptr->HintTrackVersion = BS_ReadInt(bs, 16);
	ptr->LastCompatibleVersion = BS_ReadInt(bs, 16);
	ptr->MaxPacketSize = BS_ReadInt(bs, 32);
	*read += 8;

	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		e = ChainAddEntry(ptr->HintDataTable, a);
		if (e) return e;
		*read += a->size;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY

M4Err ghnt_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	HintSampleEntryAtom *ptr = (HintSampleEntryAtom *)s;

	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteData(bs, ptr->reserved, 6);
	BS_WriteInt(bs, ptr->dataReferenceIndex, 16);
	BS_WriteInt(bs, ptr->HintTrackVersion, 16);
	BS_WriteInt(bs, ptr->LastCompatibleVersion, 16);
	BS_WriteInt(bs, ptr->MaxPacketSize, 32);
	return WriteAtomList(s, ptr->HintDataTable, bs);
}

M4Err ghnt_Size(Atom *s)
{
	M4Err e;
	HintSampleEntryAtom *ptr = (HintSampleEntryAtom *)s;
	
	e = Atom_Size(s);
	if (e) return e;
	ptr->size += 16;
	e = SizeAtomList(s, ptr->HintDataTable);
	if (e) return e;
	return M4OK;
}


#endif	//M4_READ_ONLY
