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

#include <intern/m4_isomedia_dev.h>

void co64_del(Atom *s)
{
	ChunkLargeOffsetAtom *ptr;
	ptr = (ChunkLargeOffsetAtom *) s;
	if (ptr == NULL) return;
	if (ptr->offsets) free(ptr->offsets);
	free(ptr);
}

M4Err co64_Read(Atom *s,BitStream *bs, u64 *read)
{
	M4Err e;
	u32 entries;
	ChunkLargeOffsetAtom *ptr = (ChunkLargeOffsetAtom *) s;
	if ( ptr == NULL ) return M4BadParam;
	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	ptr->entryCount = BS_ReadInt(bs, 32);
	*read += 4;
	ptr->offsets = (u64 *) malloc(ptr->entryCount * sizeof(u64) );
	if (ptr->offsets == NULL) return M4OutOfMem;
	for (entries = 0; entries < ptr->entryCount; entries++) {
		ptr->offsets[entries] = BS_ReadInt(bs, 64);
		*read += 8;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *co64_New()
{
	ChunkLargeOffsetAtom *tmp;
	
	tmp = (ChunkLargeOffsetAtom *) malloc(sizeof(ChunkLargeOffsetAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(ChunkLargeOffsetAtom));
	InitFullAtom((Atom *)tmp);
	tmp->type = ChunkLargeOffsetAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err co64_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	ChunkLargeOffsetAtom *ptr = (ChunkLargeOffsetAtom *) s;
	
	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->entryCount, 32);
	for (i = 0; i < ptr->entryCount; i++ ) {
		BS_WriteLongInt(bs, ptr->offsets[i], 64);
	}
	return M4OK;
}

M4Err co64_Size(Atom *s)
{
	M4Err e;
	ChunkLargeOffsetAtom *ptr = (ChunkLargeOffsetAtom *) s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 4 + (8 * ptr->entryCount);
	return M4OK;
}

#endif //M4_READ_ONLY

void cprt_del(Atom *s)
{
	CopyrightAtom *ptr = (CopyrightAtom *) s;
	if (ptr == NULL) return;
	if (ptr->notice)
		free(ptr->notice);
	free(ptr);
}


M4Err cprt_Read(Atom *s,BitStream *bs, u64 *read)
{
	M4Err e;
	u32 bytesToRead;

	CopyrightAtom *ptr = (CopyrightAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	BS_ReadInt(bs, 1);
	//the spec is unclear here, just says "the value 0 is interpreted as undetermined"
	ptr->packedLanguageCode[0] = BS_ReadInt(bs, 5);
	ptr->packedLanguageCode[1] = BS_ReadInt(bs, 5);
	ptr->packedLanguageCode[2] = BS_ReadInt(bs, 5);
	//but before or after compaction ?? We assume before
	if (ptr->packedLanguageCode[0] || ptr->packedLanguageCode[1] || ptr->packedLanguageCode[2]) {
		ptr->packedLanguageCode[0] += 0x60;
		ptr->packedLanguageCode[1] += 0x60;
		ptr->packedLanguageCode[2] += 0x60;
	} else {
		ptr->packedLanguageCode[0] = 'u';
		ptr->packedLanguageCode[1] = 'n';
		ptr->packedLanguageCode[2] = 'd';
	}

	*read += 2;
	bytesToRead = (u32) (ptr->size - *read);
	if ( bytesToRead < 0 ) return M4InvalidAtom;
	if ( bytesToRead > 0 ) {
		ptr->notice = (char*)malloc(bytesToRead * sizeof(char));
		if (ptr->notice == NULL) return M4OutOfMem;
		BS_ReadData(bs, (unsigned char *)ptr->notice, bytesToRead);
		*read += bytesToRead;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *cprt_New()
{
	CopyrightAtom *tmp;
	
	tmp = (CopyrightAtom *) malloc(sizeof(CopyrightAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(CopyrightAtom));

	InitFullAtom((Atom *)tmp);
	tmp->type = CopyrightAtomType;
	tmp->packedLanguageCode[0] = 'u';
	tmp->packedLanguageCode[1] = 'n';
	tmp->packedLanguageCode[2] = 'd';

	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err cprt_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	CopyrightAtom *ptr = (CopyrightAtom *) s;

	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, 0, 1);
	if (ptr->packedLanguageCode[0]) {
		BS_WriteInt(bs, ptr->packedLanguageCode[0] - 0x60, 5);
		BS_WriteInt(bs, ptr->packedLanguageCode[1] - 0x60, 5);
		BS_WriteInt(bs, ptr->packedLanguageCode[2] - 0x60, 5);
	} else {
		BS_WriteInt(bs, 0, 15);
	}
	if (ptr->notice) {
		BS_WriteData(bs, (unsigned char*)ptr->notice, (unsigned long)strlen(ptr->notice) + 1);
	}
	return M4OK;
}

M4Err cprt_Size(Atom *s)
{
	M4Err e;
	CopyrightAtom *ptr = (CopyrightAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 2;
	if (ptr->notice)
		ptr->size += strlen(ptr->notice) + 1;
	return M4OK;
}

#endif //M4_READ_ONLY

void ctts_del(Atom *s)
{
	u32 entryCount;
	u32 i;
	dttsEntry *pe;
	CompositionOffsetAtom *ptr = (CompositionOffsetAtom *)s;
	if (ptr == NULL) return;
	
	if (ptr->entryList) {
		entryCount = ChainGetCount(ptr->entryList);
		for ( i = 0; i < entryCount; i++ ) {
			pe = (dttsEntry*)ChainGetEntry(ptr->entryList, i);
			if (pe) free(pe);	
		}
		DeleteChain(ptr->entryList);
	}
	free(ptr);
}



M4Err ctts_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 entries;
	u32 entryCount;
	u32 count, sampleCount;
	s32 decodingOffset;
	dttsEntry *p;
	CompositionOffsetAtom *ptr = (CompositionOffsetAtom *)s;
	
	p = NULL;
	if (!ptr) return M4BadParam;
	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	entryCount = BS_ReadInt(bs, 32);
	*read += 4;
	sampleCount = 0;
	for ( entries = 0; entries < entryCount; entries++ ) {
		p = (dttsEntry *) malloc(sizeof(dttsEntry));
		if (!p) return M4OutOfMem;
		count = BS_ReadInt(bs, 32);
		decodingOffset = BS_ReadInt(bs, 32);
		*read += 8;
		p->sampleCount = count;
		sampleCount += count;
		p->decodingOffset = decodingOffset;
		ChainAddEntry(ptr->entryList, p);
	}
#ifndef M4_READ_ONLY
	ptr->w_currentEntry = p;
	ptr->w_LastSampleNumber = sampleCount;
#endif
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *ctts_New()
{
	CompositionOffsetAtom *tmp;
	
	tmp = (CompositionOffsetAtom *) malloc(sizeof(CompositionOffsetAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(CompositionOffsetAtom));

	InitFullAtom((Atom *) tmp);
	tmp->entryList = NewChain();
	if (! tmp->entryList) {
		free(tmp);
		return NULL;
	}
	tmp->type = CompositionOffsetAtomType;
	return (Atom *) tmp;
}



//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err ctts_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	u32 entryCount;
	dttsEntry *p;
	CompositionOffsetAtom *ptr = (CompositionOffsetAtom *)s;
	
	e = FullAtom_Write(s, bs);
	if (e) return e;
    entryCount = ChainGetCount(ptr->entryList);
	BS_WriteInt(bs, entryCount, 32);
	for ( i = 0; i < entryCount; i++ ) {
		p = (dttsEntry*)ChainGetEntry(ptr->entryList, i);
		if (p) {
			BS_WriteInt(bs, p->sampleCount, 32);
			BS_WriteInt(bs, p->decodingOffset, 32);
		}
	}
	return M4OK;
}

M4Err ctts_Size(Atom *s)
{
	M4Err e;
	u32 entryCount;
	CompositionOffsetAtom *ptr = (CompositionOffsetAtom *) s;

	e = FullAtom_Size(s);
	if (e) return e;
	entryCount = ChainGetCount(ptr->entryList);
	ptr->size += 4 + (8 * entryCount);
	return M4OK;
}

#endif //M4_READ_ONLY

void url_del(Atom *s)
{
	DataEntryURLAtom *ptr = (DataEntryURLAtom *)s;
	if (ptr == NULL) return;
	if (ptr->location) free(ptr->location);
	free(ptr);
	return;
}


M4Err url_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 bytesToRead;
	DataEntryURLAtom *ptr = (DataEntryURLAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	bytesToRead = (u32) (ptr->size - *read);
	if (bytesToRead > 0) {
		ptr->location = (char*)malloc(bytesToRead);
		if (! ptr->location) return M4OutOfMem;
		BS_ReadData(bs, (unsigned char*)ptr->location, bytesToRead);
		*read += bytesToRead;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *url_New()
{
	DataEntryURLAtom *tmp = (DataEntryURLAtom *) malloc(sizeof(DataEntryURLAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(DataEntryURLAtom));

	InitFullAtom((Atom *)tmp);
	tmp->type = DataEntryURLAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err url_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	DataEntryURLAtom *ptr = (DataEntryURLAtom *)s;

	e = FullAtom_Write(s, bs);
	if (e) return e;
	//the flag set indicates we have a string (WE HAVE TO for URLs)
    if ( !(ptr->flags & 1)) {
		if (ptr->location) {
			BS_WriteData(bs, (unsigned char*)ptr->location, (u32)strlen(ptr->location) + 1);
		}
	}
	return M4OK;
}

M4Err url_Size(Atom *s)
{
	M4Err e;
	DataEntryURLAtom *ptr = (DataEntryURLAtom *)s;
	
	e = FullAtom_Size(s);
	if (e) return e;
	if ( !(ptr->flags & 1)) {
		if (ptr->location) ptr->size += 1 + strlen(ptr->location);
	}
	return M4OK;
}

#endif //M4_READ_ONLY

void urn_del(Atom *s)
{
	DataEntryURNAtom *ptr = (DataEntryURNAtom *)s;
	if (ptr == NULL) return;
	if (ptr->location) free(ptr->location);
	if (ptr->nameURN) free(ptr->nameURN);
	free(ptr);
}


M4Err urn_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 bytesToRead;
	u32 i;
	char *tmpName;
	DataEntryURNAtom *ptr = (DataEntryURNAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	if (ptr->size < *read) return M4InvalidAtom;
	bytesToRead = (u32) (ptr->size - *read);
	//if nothing to do, do nothing ;)
	if (! bytesToRead) return M4OK;

	//here we have to handle that in a clever way
	tmpName = (char*)malloc(sizeof(char) * bytesToRead);
	if (!tmpName) return M4OutOfMem;
	//get the data
	BS_ReadData(bs, (unsigned char*)tmpName, bytesToRead);
	*read += bytesToRead;

	//then get the break
	i = 0;
	while ( (tmpName[i] != 0) && (i < bytesToRead) ) {
		i++;
	}
	//check the data is consistent
	if (i == bytesToRead) {
		free(tmpName);
		return M4ReadAtomFailed;
	}
	//no NULL char, URL is not specified
	if (i == bytesToRead - 1) {
		ptr->nameURN = tmpName;
		ptr->location = NULL;
		return M4OK;
	}
	//OK, this has both URN and URL
	ptr->nameURN = (char*)malloc(sizeof(char) * (i+1));
	if (!ptr->nameURN) {
		free(tmpName);
		return M4OutOfMem;
	}
	ptr->location = (char*)malloc(sizeof(char) * (bytesToRead - i - 1));
	if (!ptr->location) {
		free(tmpName);
		free(ptr->nameURN);
		ptr->nameURN = NULL;
		return M4OutOfMem;
	}
	memcpy(ptr->nameURN, tmpName, i + 1);
	memcpy(ptr->location, tmpName + i + 1, (bytesToRead - i - 1));
	free(tmpName);
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *urn_New()
{
	DataEntryURNAtom *tmp = (DataEntryURNAtom *) malloc(sizeof(DataEntryURNAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(DataEntryURNAtom));
	InitFullAtom((Atom *)tmp);
	tmp->type = DataEntryURNAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY


M4Err urn_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	DataEntryURNAtom *ptr = (DataEntryURNAtom *)s;

	e = FullAtom_Write(s, bs);
	if (e) return e;
	//the flag set indicates we have a string (WE HAVE TO for URLs)
    if ( !(ptr->flags & 1)) {
		//to check, the spec says: First name, then location
		if (ptr->nameURN) {
			BS_WriteData(bs, (unsigned char*)ptr->nameURN, (u32)strlen(ptr->nameURN) + 1);
		}
		if (ptr->location) {
			BS_WriteData(bs, (unsigned char*)ptr->location, (u32)strlen(ptr->location) + 1);
		}
	}
	return M4OK;
}

M4Err urn_Size(Atom *s)
{
	M4Err e;
	DataEntryURNAtom *ptr = (DataEntryURNAtom *)s;

	e = FullAtom_Size(s);
	if (e) return e;
	if ( !(ptr->flags & 1)) {
		if (ptr->nameURN) ptr->size += 1 + strlen(ptr->nameURN);
		if (ptr->location) ptr->size += 1 + strlen(ptr->location);
	}
	return M4OK;
}

#endif //M4_READ_ONLY

void defa_del(Atom *s)
{
	UnknownAtom *ptr = (UnknownAtom *) s;
	if (!s) return;
	if (ptr->data) free(ptr->data);
	free(ptr);
}


M4Err defa_Read(Atom *s, BitStream *bs, u64 *read)
{
	u32 bytesToRead;
	UnknownAtom *ptr = (UnknownAtom *)s;
	
	if (ptr == NULL) return M4BadParam;
	if (ptr->size - *read > 0xFFFFFFFF) return M4ReadAtomFailed;
	bytesToRead = (u32) (ptr->size - *read);
	if (bytesToRead < 0) return M4InvalidAtom;
	
	if (bytesToRead > 0) {
		ptr->data = (char*)malloc(bytesToRead);
		if (ptr->data == NULL ) return M4OutOfMem;
		ptr->dataSize = bytesToRead;
		BS_ReadData(bs, (unsigned char*)ptr->data, ptr->dataSize);
		*read += ptr->dataSize;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

//warning: we don't have any atomType, trick has to be done while creating..
Atom *defa_New()
{
	UnknownAtom *tmp = (UnknownAtom *) malloc(sizeof(UnknownAtom));
	memset(tmp, 0, sizeof(UnknownAtom));
	return (Atom *) tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err defa_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	UnknownAtom *ptr = (UnknownAtom *)s;
	if (!s) return M4BadParam;

	e = Atom_Write(s, bs);
	if (e) return e;
    if (ptr->data) {
		BS_WriteData(bs, (unsigned char*)ptr->data, ptr->dataSize);
	}
	return M4OK;
}

M4Err defa_Size(Atom *s)
{
	M4Err e;
	UnknownAtom *ptr = (UnknownAtom *)s;
	e = Atom_Size(s);
	if (e) return e;
	ptr->size += ptr->dataSize;
	return M4OK;
}

#endif //M4_READ_ONLY

void dinf_del(Atom *s)
{
	DataInformationAtom *ptr = (DataInformationAtom *)s;
	if (ptr == NULL) return;
	DeleteAtomList(ptr->atomList);
	free(ptr);
}



M4Err dinf_AddAtom(DataInformationAtom *ptr, Atom *a)
{
	if (!a) return M4OK;
	switch(a->type) {
		//check if we already have a DREF
		case DataReferenceAtomType:
			if (ptr->dref) return M4BadParam;
			ptr->dref = (DataReferenceAtom *)a;
			break;		
	}
	return ChainAddEntry(ptr->atomList, a);
}

M4Err dinf_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	Atom *a;
	u64 sub_read;
	DataInformationAtom *ptr = (DataInformationAtom *)s;
	if (!ptr) return M4BadParam;

	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sub_read);
		if (e) return e;
		e = dinf_AddAtom(ptr, a);
		if (e) return e;
		*read += a->size;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *dinf_New()
{
	DataInformationAtom *tmp = (DataInformationAtom *) malloc(sizeof(DataInformationAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(DataInformationAtom));
	tmp->atomList = NewChain();
	if (tmp->atomList == NULL) {
		free(tmp);
		return NULL;
	}
	tmp->type = DataInformationAtomType;

	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err dinf_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	DataInformationAtom *ptr = (DataInformationAtom *)s;

	e = Atom_Write(s, bs);
	if (e) return e;
	e = WriteAtomList(s, ptr->atomList, bs);
	if (e) return e;
	return M4OK;
}

M4Err dinf_Size(Atom *s)
{
	M4Err e;
	DataInformationAtom *ptr = (DataInformationAtom *)s;
	e = Atom_Size(s);
	return SizeAtomList(s, ptr->atomList);
}

#endif //M4_READ_ONLY

void dref_del(Atom *s)
{
	DataReferenceAtom *ptr = (DataReferenceAtom *) s;
	if (ptr == NULL) return;
	DeleteAtomList(ptr->atomList);
	free(ptr);
}


M4Err dref_AddDataEntry(DataReferenceAtom *ptr, Atom *entry)
{
	return ChainAddEntry(ptr->atomList, entry);
}

M4Err dref_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u64 subread;
	u32 count, i;
	Atom *a;
	DataReferenceAtom *ptr = (DataReferenceAtom *)s;
	
	if (ptr == NULL) return M4BadParam;
	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	count = BS_ReadInt(bs, 32);
	*read += 4;
	for ( i = 0; i < count; i++ ) {
		e = ParseAtom(&a, bs, &subread);
		if (e) return e;
		e = ChainAddEntry(ptr->atomList, a);
		if (e) return e;
		*read += a->size;
	}
	if (*read != ptr->size ) return M4ReadAtomFailed;
	return M4OK;
}

Atom *dref_New()
{
	DataReferenceAtom *tmp = (DataReferenceAtom *) malloc(sizeof(DataReferenceAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(DataReferenceAtom));

	InitFullAtom((Atom *) tmp);
	tmp->atomList = NewChain();
	if (!tmp->atomList) {
		free(tmp);
		return NULL;
	}
	tmp->type = DataReferenceAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err dref_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 count;
	DataReferenceAtom *ptr = (DataReferenceAtom *)s;
	if (!s) return M4BadParam;

	e = FullAtom_Write(s, bs);
	if (e) return e;
	count = ChainGetCount(ptr->atomList);
	BS_WriteInt(bs, count, 32);
	return WriteAtomList(s, ptr->atomList, bs);
}

M4Err dref_Size(Atom *s)
{
	M4Err e;
	DataReferenceAtom *ptr = (DataReferenceAtom *)s;
	if (!s) return M4BadParam;

	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 4;
	e = SizeAtomList(s, ptr->atomList);
	if (e) return e;
	return M4OK;
}

#endif //M4_READ_ONLY

void edts_del(Atom *s)
{
	EditAtom *ptr = (EditAtom *) s;
	if (ptr == NULL) return;
	DeleteAtomList(ptr->atomList);
	free(ptr);
}


M4Err edts_AddAtom(EditAtom *ptr, Atom *a)
{
	if (!a) return M4OK;
	if (a->type == EditListAtomType) {
		if (ptr->editList) return M4BadParam;
		ptr->editList = (EditListAtom *)a;
	}
	return ChainAddEntry(ptr->atomList, a);
}


M4Err edts_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	Atom *a;
	u64 sr;
	EditAtom *ptr = (EditAtom *)s;
	if (ptr == NULL) return M4BadParam;
	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		e = edts_AddAtom(ptr, a); 
		if (e) return e;
		*read += a->size;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;

}

Atom *edts_New()
{
	EditAtom *tmp;
	
	tmp = (EditAtom *) malloc(sizeof(EditAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(EditAtom));

	tmp->atomList = NewChain();
	if (!tmp->atomList) {
		free(tmp);
		return NULL;
	}
	tmp->type = EditAtomType;

	return (Atom *) tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err edts_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	EditAtom *ptr = (EditAtom *)s;

	//here we have a trick: if editList is empty, skip the atom
	if (ChainGetCount(ptr->editList->entryList)) {
		e = Atom_Write(s, bs);
		if (e) return e;
		e = WriteAtomList(s, ptr->atomList, bs);
		if (e) return e;
	}
	return M4OK;
}

M4Err edts_Size(Atom *s)
{
	M4Err e;
	EditAtom *ptr = (EditAtom *)s;

	//here we have a trick: if editList is empty, skip the atom
	if (! ChainGetCount(ptr->editList->entryList)) {
		ptr->size = 0;
	} else {
		e = Atom_Size(s);
		if (e) return e;
		e = SizeAtomList(s, ptr->atomList);
		if (e) return e;
	}
	return M4OK;
}

#endif //M4_READ_ONLY

void elst_del(Atom *s)
{
	EditListAtom *ptr;
	edtsEntry *p;
	u32 entryCount;
	u32 i;

	ptr = (EditListAtom *)s;
	if (ptr == NULL) return;
	entryCount = ChainGetCount(ptr->entryList);
	for (i = 0; i < entryCount; i++) {
		p = (edtsEntry*)ChainGetEntry(ptr->entryList, i);
		if (p) free(p);
	}
	DeleteChain(ptr->entryList);
	free(ptr);
}




M4Err elst_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 entries;
	s32 tr;
	u32 entryCount;
	edtsEntry *p;

	EditListAtom *ptr = (EditListAtom *)s;
	
	if (ptr == NULL) return M4BadParam;
	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	entryCount = BS_ReadInt(bs, 32);
	*read += 4;
	for (entries = 0; entries < entryCount; entries++ ) {
		p = (edtsEntry *) malloc(sizeof(edtsEntry));
		if (!p) return M4OutOfMem;
		if (ptr->version == 1) {
			p->segmentDuration = BS_ReadLongInt(bs, 64);
			p->mediaTime = (s64) BS_ReadLongInt(bs, 64);
			*read += 16;
		} else {
			p->segmentDuration = BS_ReadInt(bs, 32);
			tr = BS_ReadInt(bs, 32);
			p->mediaTime = (s64) tr;
			*read += 8;
		}
		p->mediaRate = BS_ReadInt(bs, 32);
		*read += 4;
		ChainAddEntry(ptr->entryList, p);
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *elst_New()
{
	EditListAtom *tmp;
	
	tmp = (EditListAtom *) malloc(sizeof(EditListAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(EditListAtom));

	InitFullAtom((Atom *)tmp);
	tmp->entryList = NewChain();
	if (!tmp->entryList) {
		free(tmp);
		return NULL;
	}
	tmp->type = EditListAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err elst_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	u32 entryCount;
	edtsEntry *p;
	EditListAtom *ptr = (EditListAtom *)s;
	if (!ptr) return M4BadParam;

	entryCount = ChainGetCount(ptr->entryList);
	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, entryCount, 32);
	for (i = 0; i < entryCount; i++ ) {
		p = (edtsEntry*)ChainGetEntry(ptr->entryList, i);
		if (ptr->version == 1) {
			BS_WriteLongInt(bs, p->segmentDuration, 64);
			BS_WriteLongInt(bs, p->mediaTime, 64);
		} else {
			BS_WriteInt(bs, (u32) p->segmentDuration, 32);
			BS_WriteInt(bs, (s32) p->mediaTime, 32);
		}
		BS_WriteInt(bs, p->mediaRate, 32);
	}
	return M4OK;
}

M4Err elst_Size(Atom *s)
{
	M4Err e;
	u32 durtimebytes;
	u32 entryCount;
	EditListAtom *ptr = (EditListAtom *)s;

	e = FullAtom_Size(s);
	if (e) return e;
	//entry count
	ptr->size += 4;
	durtimebytes = (ptr->version == 1 ? 16 : 8) + 4;
	entryCount = ChainGetCount(ptr->entryList);
	ptr->size += (entryCount * durtimebytes);
	return M4OK;
}


#endif //M4_READ_ONLY

void esds_del(Atom *s)
{
	ESDAtom *ptr = (ESDAtom *)s;
	if (ptr == NULL)	return;
	if (ptr->desc) OD_DeleteDescriptor((Descriptor **)&ptr->desc);
	free(ptr);
}


M4Err esds_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 descSize;
	char *enc_desc;

	u32 SLIsPredefined(SLConfigDescriptor *sl);

	ESDAtom *ptr = (ESDAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	descSize = (u32) (ptr->size - *read);
	if (descSize) {
		enc_desc = (char*)malloc(sizeof(char) * descSize);
		if (!enc_desc) return M4OutOfMem;
		//get the payload
		BS_ReadData(bs, (unsigned char*)enc_desc, descSize);
		//send it to the OD Codec
		e = OD_ReadDesc(enc_desc, descSize, (Descriptor **) &ptr->desc);
		//OK, free our desc
		free(enc_desc);
		//we do not abbort on error, but skip the descritpor
		if (e) {
			ptr->desc = NULL;
		} else {
			/*fix broken files*/
			if (!ptr->desc->URLString) {
				if (!ptr->desc->slConfig) {
					ptr->desc->slConfig = (SLConfigDescriptor *) OD_NewDescriptor(SLConfigDescriptor_Tag);
					ptr->desc->slConfig->predefined = SLPredef_MP4;
				} else if (ptr->desc->slConfig->predefined != SLPredef_MP4) {
					M4Err SLSetPredefined(SLConfigDescriptor *sl);
					ptr->desc->slConfig->predefined = SLPredef_MP4;
					SLSetPredefined(ptr->desc->slConfig);
				}
			}
		}
		//but the bytes parsed may be different!
		*read += descSize;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *esds_New()
{
	ESDAtom *tmp = (ESDAtom *) malloc(sizeof(ESDAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(ESDAtom));

	InitFullAtom((Atom *) tmp);
	tmp->type = ESDAtomType;

	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err esds_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	char *enc_desc;
	u32 descSize = 0;
	ESDAtom *ptr = (ESDAtom *)s;
	
	e = FullAtom_Write(s, bs);
	if (e) return e;
	e = OD_EncDesc((Descriptor *)ptr->desc, &enc_desc, &descSize);
	if (e) return e;
	BS_WriteData(bs, (unsigned char*)enc_desc, descSize);
	//free our buffer
	free(enc_desc);
	return M4OK;
}

M4Err esds_Size(Atom *s)
{
	M4Err e;
	u32 descSize = 0;
	ESDAtom *ptr = (ESDAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	descSize = OD_GetDescSize((Descriptor *)ptr->desc);
	ptr->size += descSize;
	return M4OK;
}

#endif //M4_READ_ONLY

void free_del(Atom *s)
{
	FreeSpaceAtom *ptr = (FreeSpaceAtom *)s;
	if (ptr->data) free(ptr->data);
	free(ptr);
}


M4Err free_Read(Atom *s, BitStream *bs, u64 *read)
{
	u32 bytesToRead;
	FreeSpaceAtom *ptr = (FreeSpaceAtom *)s;
	if (ptr == NULL) return M4BadParam;

	if (ptr->size - *read > 0xFFFFFFFF) return M4IOErr;
	
	bytesToRead = (u32) (ptr->size - *read);
	if (bytesToRead < 0) return M4InvalidAtom;
	
	if (bytesToRead > 0) {
		ptr->data = (char*)malloc(bytesToRead * sizeof(char));
		BS_ReadData(bs, (unsigned char*)ptr->data, bytesToRead);
		*read += bytesToRead;
		ptr->dataSize = bytesToRead;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *free_New()
{
	FreeSpaceAtom *tmp = (FreeSpaceAtom *) malloc(sizeof(FreeSpaceAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(FreeSpaceAtom));
	tmp->type = FreeSpaceAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err free_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	FreeSpaceAtom *ptr = (FreeSpaceAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	if (ptr->dataSize)	BS_WriteData(bs, (unsigned char*)ptr->data, ptr->dataSize);
	return M4OK;
}

M4Err free_Size(Atom *s)
{
	M4Err e;
	FreeSpaceAtom *ptr = (FreeSpaceAtom *)s;
	e = Atom_Size(s);
	if (e) return e;
	ptr->size += ptr->dataSize;
	return M4OK;
}

#endif //M4_READ_ONLY

void ftyp_del(Atom *s)
{
	FileTypeAtom *ptr = (FileTypeAtom *) s;
	if (ptr->altBrand) free(ptr->altBrand);
	free(ptr);
}

Atom *ftyp_New()
{
	FileTypeAtom *tmp;
	
	tmp = (FileTypeAtom *) malloc(sizeof(FileTypeAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(FileTypeAtom));

	tmp->type = FileTypeAtomType;
	return (Atom *)tmp;
}

M4Err ftyp_Read(Atom *s,BitStream *bs, u64 *read)
{
	u32 i;

	FileTypeAtom *ptr = (FileTypeAtom *)s;
	if (ptr == NULL) return M4BadParam;

	ptr->majorBrand = BS_ReadInt(bs, 32);
	ptr->minorVersion = BS_ReadInt(bs, 32);
	*read += 8;

	ptr->altCount = ( (u32) (ptr->size - *read)) / 4;
	if (!ptr->altCount) return M4OK;
	if (ptr->altCount * 4 != (u32) (ptr->size - *read)) return M4InvalidAtom;

	ptr->altBrand = (u32*)malloc(sizeof(u32)*ptr->altCount);
	for (i = 0; i<ptr->altCount; i++) {
		ptr->altBrand[i] = BS_ReadInt(bs, 32);
		*read += 4;
	}
	return M4OK;
}



//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err ftyp_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	FileTypeAtom *ptr = (FileTypeAtom *) s;

	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->majorBrand, 32);
	BS_WriteInt(bs, ptr->minorVersion, 32);
	for (i=0; i<ptr->altCount; i++) {
		BS_WriteInt(bs, ptr->altBrand[i], 32);
	}
	return M4OK;
}

M4Err ftyp_Size(Atom *s)
{
	M4Err e;
	FileTypeAtom *ptr = (FileTypeAtom *)s;
	
	e = Atom_Size(s);
	if (e) return e;
	ptr->size += 8 + ptr->altCount * 4;
	return M4OK;
}

#endif //M4_READ_ONLY



void gnrm_del(Atom *s)
{
	GenericMediaSampleEntryAtom *ptr = (GenericMediaSampleEntryAtom *)s;
	if (ptr->data) free(ptr->data);
	free(ptr);
}

Atom *gnrm_New()
{
	GenericMediaSampleEntryAtom *tmp = (GenericMediaSampleEntryAtom *) malloc(sizeof(GenericMediaSampleEntryAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(GenericMediaSampleEntryAtom));
	tmp->type = GenericMediaSampleEntryAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err gnrm_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	GenericMediaSampleEntryAtom *ptr = (GenericMediaSampleEntryAtom *)s;
	
	//carefull we are not writing the atom type but the entry type so switch for write
	ptr->type = ptr->EntryType;
	e = Atom_Write(s, bs);
	if (e) return e;
	ptr->type = GenericMediaSampleEntryAtomType;
	BS_WriteData(bs, (unsigned char*)ptr->reserved, 6);
	BS_WriteInt(bs, ptr->dataReferenceIndex, 16);
	BS_WriteData(bs, (unsigned char*) ptr->data, ptr->data_size);
	return M4OK;
}

M4Err gnrm_Size(Atom *s)
{
	M4Err e;
	GenericMediaSampleEntryAtom *ptr = (GenericMediaSampleEntryAtom *)s;
	s->type = ptr->EntryType;
	e = Atom_Size(s);
	s->type = GenericMediaSampleEntryAtomType;
	if (e) return e;
	ptr->size += 8+ptr->data_size;
	return M4OK;
}

#endif //M4_READ_ONLY


void gnrv_del(Atom *s)
{
	GenericVisualSampleEntryAtom *ptr = (GenericVisualSampleEntryAtom *)s;
	if (ptr->data) free(ptr->data);
	free(ptr);
}

Atom *gnrv_New()
{
	GenericVisualSampleEntryAtom *tmp = (GenericVisualSampleEntryAtom *) malloc(sizeof(GenericVisualSampleEntryAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(GenericVisualSampleEntryAtom));
	tmp->type = GenericVisualSampleEntryAtomType;
	InitVideoSampleEntry((VisualSampleEntryAtom*) tmp);
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err gnrv_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	GenericVisualSampleEntryAtom *ptr = (GenericVisualSampleEntryAtom *)s;
	
	//carefull we are not writing the atom type but the entry type so switch for write
	ptr->type = ptr->EntryType;
	e = Atom_Write(s, bs);
	if (e) return e;
	ptr->type = GenericVisualSampleEntryAtomType;

	WriteVideoSampleEntry((VisualSampleEntryAtom *)ptr, bs);
	BS_WriteData(bs, (unsigned char*) ptr->data, ptr->data_size);
	return M4OK;
}

M4Err gnrv_Size(Atom *s)
{
	M4Err e;
	GenericVisualSampleEntryAtom *ptr = (GenericVisualSampleEntryAtom *)s;
	s->type = ptr->EntryType;
	e = Atom_Size(s);
	s->type = GenericVisualSampleEntryAtomType;
	if (e) return e;
	SizeVideoSampleEntry((VisualSampleEntryAtom *)s);
	ptr->size += ptr->data_size;
	return M4OK;
}

#endif //M4_READ_ONLY



void gnra_del(Atom *s)
{
	GenericAudioSampleEntryAtom *ptr = (GenericAudioSampleEntryAtom *)s;
	if (ptr->data) free(ptr->data);
	free(ptr);
}

Atom *gnra_New()
{
	GenericAudioSampleEntryAtom *tmp = (GenericAudioSampleEntryAtom *) malloc(sizeof(GenericAudioSampleEntryAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(GenericAudioSampleEntryAtom));
	tmp->type = GenericAudioSampleEntryAtomType;
	InitAudioSampleEntry((AudioSampleEntryAtom*) tmp);
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err gnra_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	GenericAudioSampleEntryAtom *ptr = (GenericAudioSampleEntryAtom *)s;
	
	//carefull we are not writing the atom type but the entry type so switch for write
	ptr->type = ptr->EntryType;
	e = Atom_Write(s, bs);
	if (e) return e;
	ptr->type = GenericAudioSampleEntryAtomType;

	WriteAudioSampleEntry((AudioSampleEntryAtom *)ptr, bs);
	BS_WriteData(bs, (unsigned char*) ptr->data, ptr->data_size);
	return M4OK;
}

M4Err gnra_Size(Atom *s)
{
	M4Err e;
	GenericAudioSampleEntryAtom *ptr = (GenericAudioSampleEntryAtom *)s;
	s->type = ptr->EntryType;
	e = Atom_Size(s);
	s->type = GenericAudioSampleEntryAtomType;
	if (e) return e;
	SizeAudioSampleEntry((AudioSampleEntryAtom *)s);
	ptr->size += ptr->data_size;
	return M4OK;
}

#endif //M4_READ_ONLY

void hdlr_del(Atom *s)
{
	HandlerAtom *ptr = (HandlerAtom *)s;
	if (ptr == NULL) return;
	if (ptr->nameUTF8) free(ptr->nameUTF8);
	free(ptr);
}


M4Err hdlr_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	HandlerAtom *ptr = (HandlerAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	ptr->reserved1 = BS_ReadInt(bs, 32);
	ptr->handlerType = BS_ReadInt(bs, 32);
	BS_ReadData(bs, ptr->reserved2, 12);
	*read += 20;
	ptr->nameLength = (u32) (ptr->size - *read);
	if (ptr->nameLength < 0) return M4InvalidAtom;
	ptr->nameUTF8 = (char*)malloc(ptr->nameLength);
	memset(ptr->nameUTF8, 0, sizeof(char)*ptr->nameLength);
	if (ptr->nameUTF8 == NULL) return M4OutOfMem;
	BS_ReadData(bs, (unsigned char*)ptr->nameUTF8, ptr->nameLength);
	*read += ptr->nameLength;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *hdlr_New()
{
	HandlerAtom *tmp = (HandlerAtom *) malloc(sizeof(HandlerAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(HandlerAtom));

	InitFullAtom((Atom *)tmp);
	tmp->type = HandlerAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err hdlr_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	HandlerAtom *ptr = (HandlerAtom *)s;
	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->reserved1, 32);
	BS_WriteInt(bs, ptr->handlerType, 32);
	BS_WriteData(bs, ptr->reserved2, 12);
	BS_WriteData(bs, (unsigned char*)ptr->nameUTF8, ptr->nameLength);
	return M4OK;
}

M4Err hdlr_Size(Atom *s)
{
	M4Err e;
	HandlerAtom *ptr = (HandlerAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 20 + ptr->nameLength;
	return M4OK;
}

#endif //M4_READ_ONLY


void hinf_del(Atom *s)
{
	HintInfoAtom *hinf = (HintInfoAtom *)s;
	DeleteAtomList(hinf->atomList);
	DeleteChain(hinf->dataRates);
	free(hinf);
}

Atom *hinf_New()
{
	HintInfoAtom *tmp = (HintInfoAtom *)malloc(sizeof(HintInfoAtom));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(HintInfoAtom));

	tmp->atomList = NewChain();
	if (!tmp->atomList) {
		free(tmp);
		return NULL;
	}
	tmp->dataRates = NewChain();
	if (!tmp->dataRates) {
		DeleteChain(tmp->atomList);
		free(tmp);
		return NULL;
	}
	tmp->type = HintInfoAtomType;
	return (Atom *)tmp;
}

M4Err hinf_AddAtom(HintInfoAtom *hinf, Atom *a)
{
	MaxrAtom *maxR;
	u32 i;
	if (!hinf || !a) return M4BadParam;

	switch (a->type) {
	case totlAtomType:
	case trpyAtomType:
		if (hinf->numBytes) return M4BadParam;
		hinf->numBytes = a;
		break;

	case numpAtomType:
	case npckAtomType:
		if (hinf->numPack) return M4BadParam;
		hinf->numPack = a;
		break;

	case tpylAtomType:
		if (hinf->nbDataBytes) return M4BadParam;
		hinf->nbDataBytes = (TpylAtom *)a;
		break;

	case maxrAtomType:
		for (i = 0; i < ChainGetCount(hinf->dataRates); i++) {
			maxR = ChainGetEntry(hinf->dataRates, i);
			if (maxR->granularity == ((MaxrAtom *)a)->granularity) return M4BadParam;
		}
		ChainAddEntry(hinf->dataRates, a);
		break;

	case dmedAtomType:
		if (hinf->nbMediaBytes) return M4BadParam;
		hinf->nbMediaBytes = (DmedAtom *)a;
		break;

	case dimmAtomType:
		if (hinf->nbImmediateBytes) return M4BadParam;
		hinf->nbImmediateBytes = (DimmAtom *)a;
		break;

	case drepAtomType:
		if (hinf->nbRepeatedBytes) return M4BadParam;
		hinf->nbRepeatedBytes = (DrepAtom *)a;
		break;
	
	case tminAtomType:
		if (hinf->minTransTime) return M4BadParam;
		hinf->minTransTime = (TminAtom *)a;
		break;

	case tmaxAtomType:
		if (hinf->maxTransTime) return M4BadParam;
		hinf->maxTransTime = (TmaxAtom *)a;
		break;

	case pmaxAtomType:
		if (hinf->maxPackSize) return M4BadParam;
		hinf->maxPackSize = (PmaxAtom *)a;
		break;

	case dmaxAtomType:
		if (hinf->maxPackDur) return M4BadParam;
		hinf->maxPackDur = (DmaxAtom *)a;
		break;
	
	case paytAtomType:
		if (hinf->payload) return M4BadParam;
		hinf->payload = (PaytAtom *)a;
		break;

		//what to do with default ...
	default:
		break;
	}
	return ChainAddEntry(hinf->atomList, a);
}


M4Err hinf_Read(Atom *s, BitStream *bs, u64 *read)
{
	Atom *a;
	u64 sr;
	M4Err e;
	HintInfoAtom *ptr = (HintInfoAtom *)s;
	if (ptr == NULL) return M4BadParam;

	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		e = hinf_AddAtom(ptr, a);
		*read += a->size;
		/*don't complain if error*/
		if (e) DelAtom(a);
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY

M4Err hinf_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	HintInfoAtom *ptr = (HintInfoAtom *)s;
	if (!s) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	return WriteAtomList(s, ptr->atomList, bs);
}

M4Err hinf_Size(Atom *s)
{
	M4Err e;
	HintInfoAtom *ptr = (HintInfoAtom *)s;
	e = Atom_Size(s);
	if (e) return e;
	return SizeAtomList(s, ptr->atomList);
}
#endif	

void hmhd_del(Atom *s)
{
	HintMediaHeaderAtom *ptr = (HintMediaHeaderAtom *)s;
	if (ptr == NULL) return;
	free(ptr);
}


M4Err hmhd_Read(Atom *s,BitStream *bs, u64 *read)
{
	M4Err e;
	HintMediaHeaderAtom *ptr = (HintMediaHeaderAtom *)s;
	if (ptr == NULL ) return M4BadParam;
	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	ptr->maxPDUSize = BS_ReadInt(bs, 16);
	ptr->avgPDUSize = BS_ReadInt(bs, 16);
	ptr->maxBitrate = BS_ReadInt(bs, 32);
	ptr->avgBitrate = BS_ReadInt(bs, 32);
	ptr->slidingAverageBitrate = BS_ReadInt(bs, 32);
	*read +=16;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *hmhd_New()
{
	HintMediaHeaderAtom *tmp = (HintMediaHeaderAtom *) malloc(sizeof(HintMediaHeaderAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(HintMediaHeaderAtom));
	
	InitFullAtom((Atom *)tmp);
	tmp->type = HintMediaHeaderAtomType;

	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err hmhd_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	HintMediaHeaderAtom *ptr = (HintMediaHeaderAtom *)s;
	
	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->maxPDUSize, 16);
	BS_WriteInt(bs, ptr->avgPDUSize, 16);
	BS_WriteInt(bs, ptr->maxBitrate, 32);
	BS_WriteInt(bs, ptr->avgBitrate, 32);
	BS_WriteInt(bs, ptr->slidingAverageBitrate, 32);
	return M4OK;
}

M4Err hmhd_Size(Atom *s)
{
	M4Err e;
	HintMediaHeaderAtom *ptr = (HintMediaHeaderAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 16;
	return M4OK;
}

#endif //M4_READ_ONLY

Atom *hnti_New()
{
	HintTrackInfoAtom *tmp = (HintTrackInfoAtom *)malloc(sizeof(HintTrackInfoAtom));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(HintTrackInfoAtom));
	tmp->atomList = NewChain();
	if (!tmp->atomList) {
		free(tmp);
		return NULL;
	}
	tmp->type = HintTrackInfoAtomType;
	return (Atom *)tmp;
}

void hnti_del(Atom *a)
{
	Atom *t;
	RTPAtom *rtp;
	HintTrackInfoAtom *ptr = (HintTrackInfoAtom *)a;
	while (ChainGetCount(ptr->atomList)) {
		t = ChainGetEntry(ptr->atomList, 0);
		if (t->type != RTPAtomType) {
			DelAtom(t);
		} else {
			rtp = (RTPAtom *)t;
			if (rtp->sdpText) free(rtp->sdpText);
			free(rtp);
		}
		ChainDeleteEntry(ptr->atomList, 0);
	}
	DeleteChain(ptr->atomList);
	free(ptr);
}

M4Err hnti_AddAtom(HintTrackInfoAtom *hnti, Atom *a)
{
	if (!hnti || !a) return M4BadParam;

	switch (a->type) {
	//this is the value for RTPAtom - same as HintSampleEntry for RTP !!!
	case RTPAtomType:
	case SDPAtomType:
		if (hnti->SDP) return M4BadParam;
		hnti->SDP = a;
		break;
	default:
		break;
	}
	return ChainAddEntry(hnti->atomList, a);
}

M4Err hnti_Read(Atom *s, BitStream *bs, u64 *read)
{
	u32 type;
	u32 length;
	u64 sr;
	M4Err e;
	Atom *a;
	RTPAtom *rtp;

	HintTrackInfoAtom *ptr = (HintTrackInfoAtom *)s;
	if (ptr == NULL) return M4BadParam;

	//WARNING: because of the HNTI at movie level, we cannot use the generic parsing scheme!
	//this because the child SDP atom at the movie level has a type of RTP, used for
	//the HintSampleEntry !
	while (*read < ptr->size) {
		//get the type of the atom (4 bytes after our current position in the bitstream)
		//before parsing...
		type = BS_PeekBits(bs, 32, 4);
		if (type != RTPAtomType) {
			e = ParseAtom(&a, bs, &sr);
			if (e) return e;
			e = hnti_AddAtom(ptr, a);
			if (e) return e;
			*read += a->size;
		} else {
			rtp = malloc(sizeof(RTPAtom));
			if (!rtp) return M4OutOfMem;
			rtp->size = BS_ReadInt(bs, 32);
			rtp->type = BS_ReadInt(bs, 32);
			sr = 8;
			//"ITS LENGTH IS CALCULATED BY SUBSTRACTING 8 (or 12) from the atom size" - QT specs
			//this means that we dn't have any NULL char as a delimiter in QT ...
			if (rtp->size == 1) return M4BadParam;
			rtp->subType = BS_ReadInt(bs, 32);
			sr += 4;
			if (rtp->subType != SDPAtomType) return M4NotSupported;
			if (rtp->size < sr) return M4InvalidAtom;
			length = (u32) (rtp->size - sr);
			rtp->sdpText = malloc(sizeof(char) * (length + 1));
			if (!rtp->sdpText) {
				free(rtp);
				return M4OutOfMem;
			}
			BS_ReadData(bs, rtp->sdpText, length);
			rtp->sdpText[length] = 0;
			sr += length;
			e = hnti_AddAtom(ptr, (Atom *)rtp);
			if (e) return e;
			*read += rtp->size;
		}
	}

	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

#ifndef M4_READ_ONLY
M4Err hnti_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i, count;
	Atom *a;
	RTPAtom *rtp;

	HintTrackInfoAtom *ptr = (HintTrackInfoAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = Atom_Write(s, bs);
	if (e) return e;

	count = ChainGetCount(ptr->atomList);
	for (i = 0; i < count; i ++) {
		a = ChainGetEntry(ptr->atomList, i);
		if (a->type != RTPAtomType) {
			e = WriteAtom(a, bs);
			if (e) return e;
		} else {
			//write the RTPAtom by hand
			rtp = (RTPAtom *)a;
			e = Atom_Write(a, bs);
			if (e) return e;
			BS_WriteInt(bs, rtp->subType, 32);
			//don't write the NULL char
			BS_WriteData(bs, rtp->sdpText, strlen(rtp->sdpText));
		}
	}
	return M4OK;
}


M4Err hnti_Size(Atom *s)
{
	M4Err e;
	u32 i, count;
	Atom *a;
	RTPAtom *rtp;

	HintTrackInfoAtom *ptr = (HintTrackInfoAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = Atom_Size(s);
	if (e) return e;

	count = ChainGetCount(ptr->atomList);
	for (i = 0; i < count; i ++) {
		a = ChainGetEntry(ptr->atomList, i);
		if (a->type != RTPAtomType) {
			e = SizeAtom(a);
			if (e) return e;
		} else {
			//get the RTPAtom size by hand
			rtp = (RTPAtom *)a;
			e = Atom_Size(a);
			if (e) return e;
			//don't count the NULL char...
			rtp->size += 4 + strlen(rtp->sdpText);
		}
		ptr->size += a->size;
	}
	return M4OK;
}
#endif

/**********************************************************
		SDPAtom
**********************************************************/

void sdp_del(Atom *s)
{
	SDPAtom *ptr = (SDPAtom *)s;
	if (ptr->sdpText) free(ptr->sdpText);
	free(ptr);

}
M4Err sdp_Read(Atom *s, BitStream *bs, u64 *read)
{
	u32 length;
	SDPAtom *ptr = (SDPAtom *)s;
	if (ptr == NULL) return M4BadParam;

	if (ptr->size < *read) return M4InvalidAtom;
	length = (u32) (ptr->size - *read);
	//sdp text has no delimiter !!!
	ptr->sdpText = malloc(sizeof(char) * (length+1));
	if (!ptr->sdpText) return M4OutOfMem;
	
	BS_ReadData(bs, ptr->sdpText, length);
	ptr->sdpText[length] = 0;

	*read += length;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *sdp_New()
{
	SDPAtom *tmp = (SDPAtom *) malloc(sizeof(SDPAtom));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(SDPAtom));
	tmp->type = SDPAtomType;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err sdp_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	SDPAtom *ptr = (SDPAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	//don't write the NULL char!!!
	BS_WriteData(bs, ptr->sdpText, strlen(ptr->sdpText));
	return M4OK;
}
M4Err sdp_Size(Atom *s)
{
	M4Err e;
	SDPAtom *ptr = (SDPAtom *)s;
	e = Atom_Size(s);
	if (e) return e;
	//don't count the NULL char!!!
	ptr->size += strlen(ptr->sdpText);
	return M4OK;
}

#endif


/**********************************************************
		TRPY Atom
**********************************************************/

void trpy_del(Atom *s)
{
	free((TrpyAtom *)s);
}
M4Err trpy_Read(Atom *s, BitStream *bs, u64 *read)
{
	TrpyAtom *ptr = (TrpyAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->nbBytes = BS_ReadInt(bs, 64);
	*read += 8;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *trpy_New()
{
	TrpyAtom *tmp = (TrpyAtom *) malloc(sizeof(TrpyAtom));
	if (!tmp) return NULL;
	tmp->type = trpyAtomType;
	tmp->nbBytes = 0;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY

M4Err trpy_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TrpyAtom *ptr = (TrpyAtom *)s;
	if (ptr == NULL) return M4BadParam;
	
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteLongInt(bs, ptr->nbBytes, 64);
	return M4OK;
}
M4Err trpy_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 8;
	return M4OK;
}
#endif

/**********************************************************
		TOTL Atom
**********************************************************/

void totl_del(Atom *s)
{
	free((TrpyAtom *)s);
}
M4Err totl_Read(Atom *s, BitStream *bs, u64 *read)
{
	TotlAtom *ptr = (TotlAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->nbBytes = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *totl_New()
{
	TotlAtom *tmp = (TotlAtom *) malloc(sizeof(TotlAtom));
	if (!tmp) return NULL;
	tmp->type = totlAtomType;
	tmp->nbBytes = 0;
	return (Atom *)tmp;
}

#ifndef M4_READ_ONLY

M4Err totl_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TotlAtom *ptr = (TotlAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->nbBytes, 32);
	return M4OK;
}
M4Err totl_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}
#endif


/**********************************************************
		NUMP Atom
**********************************************************/

void nump_del(Atom *s)
{
	free((NumpAtom *)s);
}
M4Err nump_Read(Atom *s, BitStream *bs, u64 *read)
{
	NumpAtom *ptr = (NumpAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->nbPackets = BS_ReadInt(bs, 64);
	*read += 8;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *nump_New()
{
	NumpAtom *tmp = (NumpAtom *) malloc(sizeof(NumpAtom));
	if (!tmp) return NULL;
	tmp->type = numpAtomType;
	tmp->nbPackets = 0;
	return (Atom *)tmp;
}

#ifndef M4_READ_ONLY
M4Err nump_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	NumpAtom *ptr = (NumpAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteLongInt(bs, ptr->nbPackets, 64);
	return M4OK;
}
M4Err nump_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 8;
	return M4OK;
}
#endif


/**********************************************************
		NPCK Atom
**********************************************************/

void npck_del(Atom *s)
{
	free((NpckAtom *)s);
}
M4Err npck_Read(Atom *s, BitStream *bs, u64 *read)
{
	NpckAtom *ptr = (NpckAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->nbPackets = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *npck_New()
{
	NpckAtom *tmp = (NpckAtom *) malloc(sizeof(NpckAtom));
	if (!tmp) return NULL;
	tmp->type = npckAtomType;
	tmp->nbPackets = 0;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err npck_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	NpckAtom *ptr = (NpckAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->nbPackets, 32);
	return M4OK;
}
M4Err npck_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}
#endif


/**********************************************************
		TPYL Atom
**********************************************************/

void tpyl_del(Atom *s)
{
	free((TpylAtom *)s);
}
M4Err tpyl_Read(Atom *s, BitStream *bs, u64 *read)
{
	TpylAtom *ptr = (TpylAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->nbBytes = BS_ReadInt(bs, 64);
	*read += 8;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *tpyl_New()
{
	TpylAtom *tmp = (TpylAtom *) malloc(sizeof(TpylAtom));
	if (!tmp) return NULL;
	tmp->type = tpylAtomType;
	tmp->nbBytes = 0;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err tpyl_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TpylAtom *ptr = (TpylAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteLongInt(bs, ptr->nbBytes, 64);
	return M4OK;
}
M4Err tpyl_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 8;
	return M4OK;
}
#endif

/**********************************************************
		TPAY Atom
**********************************************************/

void tpay_del(Atom *s)
{
	free((TpayAtom *)s);
}
M4Err tpay_Read(Atom *s, BitStream *bs, u64 *read)
{
	TpayAtom *ptr = (TpayAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->nbBytes = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *tpay_New()
{
	TpayAtom *tmp = (TpayAtom *) malloc(sizeof(TpayAtom));
	if (!tmp) return NULL;
	tmp->type = tpayAtomType;
	tmp->nbBytes = 0;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err tpay_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TpayAtom *ptr = (TpayAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->nbBytes, 32);
	return M4OK;
}
M4Err tpay_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}
#endif


/**********************************************************
		MAXR Atom
**********************************************************/

void maxr_del(Atom *s)
{
	free((MaxrAtom *)s);
}
M4Err maxr_Read(Atom *s, BitStream *bs, u64 *read)
{
	MaxrAtom *ptr = (MaxrAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->granularity = BS_ReadInt(bs, 32);
	ptr->maxDataRate = BS_ReadInt(bs, 32);
	*read += 8;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *maxr_New()
{
	MaxrAtom *tmp = (MaxrAtom *) malloc(sizeof(MaxrAtom));
	if (!tmp) return NULL;
	tmp->type = maxrAtomType;
	tmp->granularity = tmp->maxDataRate = 0;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err maxr_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MaxrAtom *ptr = (MaxrAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->granularity, 32);
	BS_WriteInt(bs, ptr->maxDataRate, 32);
	return M4OK;
}
M4Err maxr_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 8;
	return M4OK;
}
#endif


/**********************************************************
		DMED Atom
**********************************************************/

void dmed_del(Atom *s)
{
	free((DmedAtom *)s);
}
M4Err dmed_Read(Atom *s, BitStream *bs, u64 *read)
{
	DmedAtom *ptr = (DmedAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->nbBytes = BS_ReadInt(bs, 64);
	*read += 8;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *dmed_New()
{
	DmedAtom *tmp = (DmedAtom *) malloc(sizeof(DmedAtom));
	if (!tmp) return NULL;
	tmp->type = dmedAtomType;
	tmp->nbBytes = 0;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err dmed_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	DmedAtom *ptr = (DmedAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteLongInt(bs, ptr->nbBytes, 64);
	return M4OK;
}
M4Err dmed_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 8;
	return M4OK;
}
#endif

/**********************************************************
		DIMM Atom
**********************************************************/

void dimm_del(Atom *s)
{
	free((DimmAtom *)s);
}
M4Err dimm_Read(Atom *s, BitStream *bs, u64 *read)
{
	DimmAtom *ptr = (DimmAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->nbBytes = BS_ReadInt(bs, 64);
	*read += 8;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *dimm_New()
{
	DimmAtom *tmp = (DimmAtom *) malloc(sizeof(DimmAtom));
	if (!tmp) return NULL;
	tmp->type = dimmAtomType;
	tmp->nbBytes = 0;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err dimm_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	DimmAtom *ptr = (DimmAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteLongInt(bs, ptr->nbBytes, 64);
	return M4OK;
}
M4Err dimm_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 8;
	return M4OK;
}
#endif

/**********************************************************
		DREP Atom
**********************************************************/

void drep_del(Atom *s)
{
	free((DrepAtom *)s);
}
M4Err drep_Read(Atom *s, BitStream *bs, u64 *read)
{
	DrepAtom *ptr = (DrepAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->nbBytes = BS_ReadInt(bs, 64);
	*read += 8;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *drep_New()
{
	DrepAtom *tmp = (DrepAtom *) malloc(sizeof(DrepAtom));
	if (!tmp) return NULL;
	tmp->type = drepAtomType;
	tmp->nbBytes = 0;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err drep_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	DrepAtom *ptr = (DrepAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteLongInt(bs, ptr->nbBytes, 64);
	return M4OK;
}
M4Err drep_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 8;
	return M4OK;
}
#endif



/**********************************************************
		TMIN Atom
**********************************************************/

void tmin_del(Atom *s)
{
	free((TminAtom *)s);
}
M4Err tmin_Read(Atom *s, BitStream *bs, u64 *read)
{
	TminAtom *ptr = (TminAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->minTime = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *tmin_New()
{
	TminAtom *tmp = (TminAtom *) malloc(sizeof(TminAtom));
	if (!tmp) return NULL;
	tmp->type = tminAtomType;
	tmp->minTime = 0;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err tmin_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TminAtom *ptr = (TminAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->minTime, 32);
	return M4OK;
}
M4Err tmin_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}
#endif


/**********************************************************
		TMAX Atom
**********************************************************/

void tmax_del(Atom *s)
{
	free((TmaxAtom *)s);
}
M4Err tmax_Read(Atom *s, BitStream *bs, u64 *read)
{
	TmaxAtom *ptr = (TmaxAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->maxTime = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *tmax_New()
{
	TmaxAtom *tmp = (TmaxAtom *) malloc(sizeof(TmaxAtom));
	if (!tmp) return NULL;
	tmp->type = tmaxAtomType;
	tmp->maxTime = 0;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err tmax_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TmaxAtom *ptr = (TmaxAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->maxTime, 32);
	return M4OK;
}
M4Err tmax_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}
#endif


/**********************************************************
		PMAX Atom
**********************************************************/

void pmax_del(Atom *s)
{
	free((PmaxAtom *)s);
}
M4Err pmax_Read(Atom *s, BitStream *bs, u64 *read)
{
	PmaxAtom *ptr = (PmaxAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->maxSize = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *pmax_New()
{
	PmaxAtom *tmp = (PmaxAtom *) malloc(sizeof(PmaxAtom));
	if (!tmp) return NULL;
	tmp->type = pmaxAtomType;
	tmp->maxSize = 0;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err pmax_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	PmaxAtom *ptr = (PmaxAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->maxSize, 32);
	return M4OK;
}
M4Err pmax_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}
#endif


/**********************************************************
		DMAX Atom
**********************************************************/

void dmax_del(Atom *s)
{
	free((DmaxAtom *)s);
}
M4Err dmax_Read(Atom *s, BitStream *bs, u64 *read)
{
	DmaxAtom *ptr = (DmaxAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->maxDur = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *dmax_New()
{
	DmaxAtom *tmp = (DmaxAtom *) malloc(sizeof(DmaxAtom));
	if (!tmp) return NULL;
	tmp->type = dmaxAtomType;
	tmp->maxDur = 0;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err dmax_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	DmaxAtom *ptr = (DmaxAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->maxDur, 32);
	return M4OK;
}
M4Err dmax_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}
#endif


/**********************************************************
		PAYT Atom
**********************************************************/

void payt_del(Atom *s)
{
	PaytAtom *payt = (PaytAtom *)s;
	if (payt->payloadString) free(payt->payloadString);
	free(payt);
}
M4Err payt_Read(Atom *s, BitStream *bs, u64 *read)
{
	u32 length;
	PaytAtom *ptr = (PaytAtom *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->payloadCode = BS_ReadInt(bs, 32);
	*read += 4;

	if (ptr->size < *read) return M4InvalidAtom;
	length = (u32) (ptr->size - *read);
	ptr->payloadString = malloc(sizeof(char) * length);
	if (! ptr->payloadString) return M4OutOfMem;

	BS_ReadData(bs, ptr->payloadString, length);
	*read += length;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *payt_New()
{
	PaytAtom *tmp = (PaytAtom *) malloc(sizeof(PaytAtom));
	if (!tmp) return NULL;
	tmp->type = paytAtomType;
	tmp->payloadCode = 0;
	tmp->payloadString = NULL;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err payt_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	PaytAtom *ptr = (PaytAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->payloadCode, 32);
	if (ptr->payloadString) {
		BS_WriteData(bs, ptr->payloadString, strlen(ptr->payloadString) + 1);
	}
	return M4OK;
}
M4Err payt_Size(Atom *s)
{
	M4Err e;
	PaytAtom *ptr = (PaytAtom *)s;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	if (ptr->payloadString) ptr->size += strlen(ptr->payloadString) + 1;
	return M4OK;
}
#endif


/**********************************************************
		PAYT Atom
**********************************************************/

void name_del(Atom *s)
{
	NameAtom *name = (NameAtom *)s;
	if (name->string) free(name->string);
	free(name);
}
M4Err name_Read(Atom *s, BitStream *bs, u64 *read)
{
	u32 length;
	NameAtom *ptr = (NameAtom *)s;
	if (ptr == NULL) return M4BadParam;

	if (ptr->size < *read) return M4InvalidAtom;
	length = (u32) (ptr->size - *read);
	ptr->string = malloc(sizeof(char) * length);
	if (! ptr->string) return M4OutOfMem;

	BS_ReadData(bs, ptr->string, length);
	*read += length;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}
Atom *name_New()
{
	NameAtom *tmp = (NameAtom *) malloc(sizeof(NameAtom));
	if (!tmp) return NULL;
	tmp->type = nameAtomType;
	tmp->string = NULL;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err name_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	NameAtom *ptr = (NameAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	if (ptr->string) {
		BS_WriteData(bs, ptr->string, strlen(ptr->string) + 1);
	}
	return M4OK;
}
M4Err name_Size(Atom *s)
{
	M4Err e;
	NameAtom *ptr = (NameAtom *)s;
	e = Atom_Size(s);
	if (e) return e;
	if (ptr->string) ptr->size += strlen(ptr->string) + 1;
	return M4OK;
}
#endif

void iods_del(Atom *s)
{
	ObjectDescriptorAtom *ptr = (ObjectDescriptorAtom *)s;
	if (ptr == NULL) return;
	if (ptr->descriptor) OD_DeleteDescriptor(& ptr->descriptor);
	free(ptr);
}


M4Err iods_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 descSize;
	char *desc;
	ObjectDescriptorAtom *ptr = (ObjectDescriptorAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	//use the OD codec...
	descSize = (u32) (ptr->size - *read);
	desc = (char*)malloc(sizeof(char) * descSize);
	BS_ReadData(bs, (unsigned char*)desc, descSize);
	e = OD_ReadDesc(desc, descSize, &ptr->descriptor);
	//OK, free our desc
	free(desc);

	//but the bytes parsed may be different
	*read += descSize;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *iods_New()
{
	ObjectDescriptorAtom *tmp = (ObjectDescriptorAtom *) malloc(sizeof(ObjectDescriptorAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(ObjectDescriptorAtom));
	InitFullAtom((Atom *)tmp);
	tmp->type = ObjectDescriptorAtomType;
	return (Atom *)tmp;
}



//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err iods_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 descSize;
	char *desc;
	ObjectDescriptorAtom *ptr = (ObjectDescriptorAtom *)s;
	e = FullAtom_Write(s, bs);
	if (e) return e;
	//call our OD codec
	e = OD_EncDesc(ptr->descriptor, &desc, &descSize);
	if (e) return e;
	BS_WriteData(bs, (unsigned char*)desc, descSize);
	//and free our stuff maybe!!
	free(desc);
	return M4OK;
}

M4Err iods_Size(Atom *s)
{
	M4Err e;
	ObjectDescriptorAtom *ptr = (ObjectDescriptorAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += OD_GetDescSize(ptr->descriptor);
	return M4OK;
}

#endif //M4_READ_ONLY

void mdat_del(Atom *s)
{
	MediaDataAtom *ptr = (MediaDataAtom *)s;
	if (!s) return;
	
	if (ptr->data) free(ptr->data);
	free(ptr);
}


M4Err mdat_Read(Atom *s, BitStream *bs, u64 *read)
{
	MediaDataAtom *ptr = (MediaDataAtom *)s;
	if (ptr == NULL) return M4BadParam;

	ptr->dataSize = s->size - *read;

	//then skip these bytes
	BS_SkipBytes(bs, ptr->dataSize);
	
	*read += ptr->dataSize;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *mdat_New()
{
	MediaDataAtom *tmp = (MediaDataAtom *) malloc(sizeof(MediaDataAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MediaDataAtom));
	tmp->type = MediaDataAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err mdat_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MediaDataAtom *ptr = (MediaDataAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;

	//make sure we have some data ...
	//if not, we handle that independantly (edit files)
	if (ptr->data) {
		BS_WriteData(bs, (unsigned char*)ptr->data, (u32) ptr->dataSize);
	}
	return M4OK;
}

M4Err mdat_Size(Atom *s)
{
	M4Err e;
	MediaDataAtom *ptr = (MediaDataAtom *)s;
	e = Atom_Size(s);
	if (e) return e;
	
	ptr->size += ptr->dataSize;
	return M4OK;
}

#endif //M4_READ_ONLY

void mdhd_del(Atom *s)
{
	MediaHeaderAtom *ptr = (MediaHeaderAtom *)s;
	if (ptr == NULL) return;
	free(ptr);
}

M4Err mdhd_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	MediaHeaderAtom *ptr = (MediaHeaderAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	if (ptr->version == 1) {
		ptr->creationTime = BS_ReadInt(bs, 64);
		ptr->modificationTime = BS_ReadInt(bs, 64);
		ptr->timeScale = BS_ReadInt(bs, 32);
		ptr->duration = BS_ReadInt(bs, 64);
		*read += 28;
	} else {
		ptr->creationTime = BS_ReadInt(bs, 32);
		ptr->modificationTime = BS_ReadInt(bs, 32);
		ptr->timeScale = BS_ReadInt(bs, 32);
		ptr->duration = BS_ReadInt(bs, 32);
		*read += 16;
	}
	//our padding bit
	BS_ReadInt(bs, 1);
	//the spec is unclear here, just says "the value 0 is interpreted as undetermined"
	ptr->packedLanguage[0] = BS_ReadInt(bs, 5);
	ptr->packedLanguage[1] = BS_ReadInt(bs, 5);
	ptr->packedLanguage[2] = BS_ReadInt(bs, 5);
	//but before or after compaction ?? We assume before
	if (ptr->packedLanguage[0] || ptr->packedLanguage[1] || ptr->packedLanguage[2]) {
		ptr->packedLanguage[0] += 0x60;
		ptr->packedLanguage[1] += 0x60;
		ptr->packedLanguage[2] += 0x60;
	} else {
		ptr->packedLanguage[0] = 'u';
		ptr->packedLanguage[1] = 'n';
		ptr->packedLanguage[2] = 'd';
	}
	ptr->reserved = BS_ReadInt(bs, 16);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *mdhd_New()
{
	MediaHeaderAtom *tmp = (MediaHeaderAtom *) malloc(sizeof(MediaHeaderAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MediaHeaderAtom));

	InitFullAtom((Atom *) tmp);
	tmp->type = MediaHeaderAtomType;

	tmp->packedLanguage[0] = 'u';
	tmp->packedLanguage[1] = 'n';
	tmp->packedLanguage[2] = 'd';
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err mdhd_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MediaHeaderAtom *ptr = (MediaHeaderAtom *)s;
	e = FullAtom_Write(s, bs);
	if (e) return e;
    if (ptr->version == 1) {
		BS_WriteLongInt(bs, ptr->creationTime, 64);
		BS_WriteLongInt(bs, ptr->modificationTime, 64);
		BS_WriteInt(bs, ptr->timeScale, 32);
		BS_WriteLongInt(bs, ptr->duration, 64);
	} else {
		BS_WriteInt(bs, (u32) ptr->creationTime, 32);
		BS_WriteInt(bs, (u32) ptr->modificationTime, 32);
		BS_WriteInt(bs, ptr->timeScale, 32);
		BS_WriteInt(bs, (u32) ptr->duration, 32);
	}
	//SPECS: BIT(1) of padding
	BS_WriteInt(bs, 0, 1);
	BS_WriteInt(bs, ptr->packedLanguage[0] - 0x60, 5);
	BS_WriteInt(bs, ptr->packedLanguage[1] - 0x60, 5);
	BS_WriteInt(bs, ptr->packedLanguage[2] - 0x60, 5);
	BS_WriteInt(bs, ptr->reserved, 16);
	return M4OK;
}

M4Err mdhd_Size(Atom *s)
{
	M4Err e;
	MediaHeaderAtom *ptr = (MediaHeaderAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 4;
	ptr->size += (ptr->version == 1) ? 28 : 16;
	return M4OK;
}

#endif //M4_READ_ONLY


void mdia_del(Atom *s)
{
	MediaAtom *ptr = (MediaAtom *)s;
	if (ptr == NULL) return;
	if (ptr->mediaHeader) DelAtom((Atom *)ptr->mediaHeader);
	if (ptr->handler) DelAtom((Atom *)ptr->handler);
	DeleteAtomList(ptr->atomList);
	free(ptr);
}


M4Err mdia_AddAtom(MediaAtom *ptr, Atom *a)
{
	if (!a) return M4OK;
	switch(a->type) {
	case MediaHeaderAtomType:
	   if (ptr->mediaHeader) return M4InvalidAtom;
	   ptr->mediaHeader = (MediaHeaderAtom *)a;
	   return M4OK;
   
	case HandlerAtomType:
	   if (ptr->handler) return M4InvalidAtom;
	   ptr->handler = (HandlerAtom *)a;
	   return M4OK;
   
	case MediaInformationAtomType:
	   if (ptr->information) return M4InvalidAtom;
	   ptr->information = (MediaInformationAtom *)a;
	   break;
	}
	return ChainAddEntry(ptr->atomList, a);
}


M4Err mdia_Read(Atom *s, BitStream *bs, u64 *read)
{	
	M4Err e;
	Atom *a;
	u64 sr;
	MediaAtom *ptr = (MediaAtom *)s;
	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		e = mdia_AddAtom(ptr, a);
		if (e) return e;
		*read += a->size;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *mdia_New()
{   
	MediaAtom *tmp = (MediaAtom *) malloc(sizeof(MediaAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MediaAtom));

	tmp->atomList = NewChain();
	if (!tmp->atomList) {
		free(tmp);
		return NULL;
	}
	tmp->type = MediaAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err mdia_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MediaAtom *ptr = (MediaAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	//Header first
	if (ptr->mediaHeader) {
		e = WriteAtom((Atom *) ptr->mediaHeader, bs);
		if (e) return e;
	}
	//then handler
	if (ptr->handler) {
		e = WriteAtom((Atom *) ptr->handler, bs);
		if (e) return e;
	}
	return WriteAtomList(s, ptr->atomList, bs);
}

M4Err mdia_Size(Atom *s)
{
	M4Err e;
	MediaAtom *ptr = (MediaAtom *)s;
	e = Atom_Size(s);
	if (e) return e;

	if (ptr->mediaHeader) {
		e = SizeAtom((Atom *) ptr->mediaHeader);
		if (e) return e;
		ptr->size += ptr->mediaHeader->size;
	}	
	if (ptr->handler) {
		e = SizeAtom((Atom *) ptr->handler);
		if (e) return e;
		ptr->size += ptr->handler->size;
	}	
	return SizeAtomList(s, ptr->atomList);
}

#endif //M4_READ_ONLY


#ifndef	M4_ISO_NO_FRAGMENTS

void mfhd_del(Atom *s)
{
	MovieFragmentHeaderAtom *ptr = (MovieFragmentHeaderAtom *)s;
	if (ptr == NULL) return;
	free(ptr);
}

M4Err mfhd_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	MovieFragmentHeaderAtom *ptr = (MovieFragmentHeaderAtom *)s;
	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	ptr->sequence_number = BS_ReadInt(bs, 32);
	if (!ptr->sequence_number) return M4InvalidMP4File;
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *mfhd_New()
{
	MovieFragmentHeaderAtom *tmp = (MovieFragmentHeaderAtom *) malloc(sizeof(MovieFragmentHeaderAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MovieFragmentHeaderAtom));
	tmp->type = MovieFragmentHeaderAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY


M4Err mfhd_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MovieFragmentHeaderAtom *ptr = (MovieFragmentHeaderAtom *) s;
	if (!s) return M4BadParam;

	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->sequence_number, 32);
	return M4OK;
}

M4Err mfhd_Size(Atom *s)
{
	M4Err e;
	MovieFragmentHeaderAtom *ptr = (MovieFragmentHeaderAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 4;
	return M4OK;
}



#endif /*M4_READ_ONLY*/

#endif /*M4_ISO_NO_FRAGMENTS*/


void minf_del(Atom *s)
{
	MediaInformationAtom *ptr = (MediaInformationAtom *)s;
	if (ptr == NULL) return;

	//if we have a Handler not self-contained, delete it (the self-contained belongs to the movie)
	if (ptr->dataHandler) {
		DataMap_Close(ptr);
	}
	if (ptr->InfoHeader) DelAtom((Atom *)ptr->InfoHeader);
	if (ptr->dataInformation) DelAtom((Atom *)ptr->dataInformation);

	DeleteAtomList(ptr->atomList);
	free(ptr);
}

M4Err minf_AddAtom(MediaInformationAtom *ptr, Atom *a)
{
	if (!a) return M4OK;
	switch (a->type) {
	case MPEGMediaHeaderAtomType:
	case VideoMediaHeaderAtomType:
	case SoundMediaHeaderAtomType:
	case HintMediaHeaderAtomType:
		if (ptr->InfoHeader) return M4InvalidAtom;
		ptr->InfoHeader = a;
		return M4OK;
	
	case DataInformationAtomType:
		if (ptr->dataInformation ) return M4InvalidAtom;
		ptr->dataInformation = (DataInformationAtom *)a;
		return M4OK;
		
	case SampleTableAtomType:
		if (ptr->sampleTable ) return M4InvalidAtom;
		ptr->sampleTable = (SampleTableAtom *)a;
		break;
	}
	return ChainAddEntry(ptr->atomList, a);
}


M4Err minf_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	Atom *a;
	u64 sr;
	MediaInformationAtom *ptr = (MediaInformationAtom *)s;
	
	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		e = minf_AddAtom(ptr, a);
		if (e) return e;
		*read += a->size;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *minf_New()
{
	MediaInformationAtom *tmp = (MediaInformationAtom *) malloc(sizeof(MediaInformationAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MediaInformationAtom));
	tmp->atomList = NewChain();
	if (! tmp->atomList) {
		free(tmp);
		return NULL;
	}
	tmp->type = MediaInformationAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err minf_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MediaInformationAtom *ptr = (MediaInformationAtom *)s;
	if (!s) return M4BadParam;
	
	e = Atom_Write(s, bs);
	if (e) return e;

	//Header first
	if (ptr->InfoHeader) {
		e = WriteAtom((Atom *) ptr->InfoHeader, bs);
		if (e) return e;
	}	
	//then dataInfo
	if (ptr->dataInformation) {
		e = WriteAtom((Atom *) ptr->dataInformation, bs);
		if (e) return e;
	}	
	return WriteAtomList(s, ptr->atomList, bs);
}

M4Err minf_Size(Atom *s)
{
	M4Err e;
	MediaInformationAtom *ptr = (MediaInformationAtom *)s;
	
	e = Atom_Size(s);
	if (e) return e;
	if (ptr->InfoHeader) {
		e = SizeAtom((Atom *) ptr->InfoHeader);
		if (e) return e;
		ptr->size += ptr->InfoHeader->size;
	}	
	if (ptr->dataInformation) {
		e = SizeAtom((Atom *) ptr->dataInformation);
		if (e) return e;
		ptr->size += ptr->dataInformation->size;
	}	
	e = SizeAtomList(s, ptr->atomList);
	if (e) return e;
	return M4OK;
}

#endif //M4_READ_ONLY

#ifndef	M4_ISO_NO_FRAGMENTS

void moof_del(Atom *s)
{
	MovieFragmentAtom *ptr = (MovieFragmentAtom *)s;
	if (ptr == NULL) return;

	if (ptr->mfhd) DelAtom((Atom *) ptr->mfhd);
	DeleteAtomList(ptr->TrackList);
	free(ptr);
}

M4Err moof_AddAtom(MovieFragmentAtom *ptr, Atom *a)
{
	if (!a) return M4OK;
	if (!ptr) return M4BadParam;

	switch (a->type) {
	case MovieFragmentHeaderAtomType:
		if (ptr->mfhd) return M4InvalidMP4File;
		ptr->mfhd = (MovieFragmentHeaderAtom *) a;
		return M4OK;
	case TrackFragmentAtomType:
		return ChainAddEntry(ptr->TrackList, a);
	default:
		return M4InvalidAtom;
	}
}

M4Err moof_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	Atom *a;
	u64 sr;
	MovieFragmentAtom *ptr = (MovieFragmentAtom *)s;

	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		*read += a->size;
		e = moof_AddAtom(ptr, a);
		if (e) return e;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *moof_New()
{
	MovieFragmentAtom *tmp = (MovieFragmentAtom *) malloc(sizeof(MovieFragmentAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MovieFragmentAtom));
	tmp->type = MovieFragmentAtomType;
	tmp->TrackList = NewChain();
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err moof_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MovieFragmentAtom *ptr = (MovieFragmentAtom *) s;
	if (!s) return M4BadParam;
	
	e = Atom_Write(s, bs);
	if (e) return e;
	//Header First
	if (ptr->mfhd) {
		e = WriteAtom((Atom *) ptr->mfhd, bs);
		if (e) return e;
	}
	//then the track list
	return WriteAtomList(s, ptr->TrackList, bs);
}

M4Err moof_Size(Atom *s)
{
	M4Err e;
	MovieFragmentAtom *ptr = (MovieFragmentAtom *)s;
	e = Atom_Size(s);
	if (e) return e;

	
	if (ptr->mfhd) {
		e = SizeAtom((Atom *) ptr->mfhd);
		if (e) return e;
		ptr->size += ptr->mfhd->size;
	}
	return SizeAtomList(s, ptr->TrackList);
}



#endif //M4_READ_ONLY

#endif 

void moov_del(Atom *s)
{
	MovieAtom *ptr = (MovieAtom *)s;
	if (ptr == NULL) return;

	if (ptr->mvhd) DelAtom((Atom *)ptr->mvhd);
	if (ptr->iods) DelAtom((Atom *)ptr->iods);
	if (ptr->udta) DelAtom((Atom *)ptr->udta);
#ifndef	M4_ISO_NO_FRAGMENTS
	if (ptr->mvex) DelAtom((Atom *)ptr->mvex);
#endif

	DeleteAtomList(ptr->atomList);
	DeleteChain(ptr->trackList);
	free(ptr);
}


M4Err moov_AddAtom(MovieAtom *ptr, Atom *a)
{
	M4Err e;
	if (!a) return M4OK;
	if (!ptr) return M4BadParam;
	switch (a->type ) {
	case ObjectDescriptorAtomType:
		if (ptr->iods) return M4InvalidAtom;
		ptr->iods = (ObjectDescriptorAtom *)a;
		//if no IOD, delete the atom
		if (!ptr->iods->descriptor) {
			ptr->iods = NULL;
			DelAtom(a);
		}
		return M4OK;
		
	case MovieHeaderAtomType:
		if (ptr->mvhd) return M4InvalidAtom;
		ptr->mvhd = (MovieHeaderAtom *)a;
		return M4OK;
		
	case UserDataAtomType:
		if (ptr->udta) return M4InvalidAtom;
		ptr->udta = (UserDataAtom *)a;
		return M4OK;

#ifndef	M4_ISO_NO_FRAGMENTS
	case MovieExtendsAtomType:
		if (ptr->mvex) return M4InvalidAtom;
		ptr->mvex = (MovieExtendsAtom *)a;
		((MovieExtendsAtom *)a)->mov = ptr->mov;
		return M4OK;
#endif
		
	case TrackAtomType:
		//set our pointer to this obj
		((TrackAtom *)a)->moov = ptr;
		e = ChainAddEntry(ptr->trackList, a);
		if (e) return e;
		break;
	}
	return ChainAddEntry(ptr->atomList, a);
}



M4Err moov_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	Atom *a;
	u64 sr;
	MovieAtom *ptr = (MovieAtom *)s;
	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		*read += a->size;
		e = moov_AddAtom(ptr, a);
		if (e) return e;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *moov_New()
{
	MovieAtom *tmp = (MovieAtom *) malloc(sizeof(MovieAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MovieAtom));
	tmp->atomList = NewChain();
	if (!tmp->atomList) {
		free(tmp);
		return NULL;
	}
	tmp->trackList = NewChain();
	if (!tmp->trackList) {
		DeleteChain(tmp->atomList);
		free(tmp);
		return NULL;
	}
	tmp->type = MovieAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY


M4Err moov_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MovieAtom *ptr = (MovieAtom *)s;
	if (!s) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;

	//header SHOULD be placed first
	if (ptr->mvhd) {
		e = WriteAtom((Atom *) ptr->mvhd, bs);
		if (e) return e;
	}
	//iods SHOULD be placed second IMO ;)
	if (ptr->iods) {
		e = WriteAtom((Atom *) ptr->iods, bs);
		if (e) return e;
	}
#ifndef	M4_ISO_NO_FRAGMENTS
	//mvex SHOULD be placed third IMO ;)
	if (ptr->mvex) {
		e = WriteAtom((Atom *) ptr->mvex, bs);
		if (e) return e;
	}
#endif 


	e = WriteAtomList(s, ptr->atomList, bs);
	if (e) return e;

	//udta SHOULD be placed last
	if (ptr->udta) {
		e = WriteAtom((Atom *) ptr->udta, bs);
		if (e) return e;
	}
	return M4OK;
}

M4Err moov_Size(Atom *s)
{
	M4Err e;
	MovieAtom *ptr = (MovieAtom *)s;
	e = Atom_Size(s);
	if (e) return e;

	if (ptr->mvhd) {
		e = SizeAtom((Atom *) ptr->mvhd);
		if (e) return e;
		ptr->size += ptr->mvhd->size;
	}
	if (ptr->iods) {
		e = SizeAtom((Atom *) ptr->iods);
		if (e) return e;
		ptr->size += ptr->iods->size;
	}
	if (ptr->udta) {
		e = SizeAtom((Atom *) ptr->udta);
		if (e) return e;
		ptr->size += ptr->udta->size;
	}
#ifndef	M4_ISO_NO_FRAGMENTS
	if (ptr->mvex) {
		e = SizeAtom((Atom *) ptr->mvex);
		if (e) return e;
		ptr->size += ptr->mvex->size;
	}
#endif

	return SizeAtomList(s, ptr->atomList);
}

#endif //M4_READ_ONLY

void mp4a_del(Atom *s)
{
	MPEGAudioSampleEntryAtom *ptr = (MPEGAudioSampleEntryAtom *)s;
	if (ptr == NULL) return;
	if (ptr->esd) DelAtom((Atom *)ptr->esd);
	if (ptr->slc) OD_DeleteDescriptor((Descriptor **)&ptr->slc);
	free(ptr);
}

M4Err mp4a_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u64 sr, pos;
	MPEGAudioSampleEntryAtom *ptr = (MPEGAudioSampleEntryAtom *)s;
	if (ptr == NULL) return M4BadParam;

	ReadAudioSampleEntry((AudioSampleEntryAtom*)s, bs, read);
	pos = BS_GetPosition(bs);
	e = ParseAtom((Atom **)&ptr->esd, bs, &sr);
	if (e) return e;

	/*once again, iPod fix*/
	if (*read + sr > s->size) {
		unsigned char *data;
		u32 size, i;
		if (ptr->esd) DelAtom((Atom *) ptr->esd);
		size = (u32) (s->size - *read);
		*read = s->size;

		BS_Seek(bs, pos);
		data = malloc(sizeof(char) * size);
		BS_ReadData(bs, data, size);
		for (i=0; i<size-8; i++) {
			if (FOUR_CHAR_INT(data[i+4], data[i+5], data[i+6], data[i+7]) == ESDAtomType) {
				BitStream *mybs = NewBitStream(data + i, size - i, BS_READ);
				e = ParseAtom((Atom **)&ptr->esd, mybs, &sr);
				DeleteBitStream(mybs);
				break;
			}
		}
		free(data);
		return M4OK;
	}
	*read += sr;
	return M4OK;
}

Atom *mp4a_New()
{
	MPEGAudioSampleEntryAtom *tmp = malloc(sizeof(MPEGAudioSampleEntryAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MPEGAudioSampleEntryAtom));
	tmp->type = MPEGAudioSampleEntryAtomType;

	InitAudioSampleEntry((AudioSampleEntryAtom*)tmp);
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err mp4a_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MPEGAudioSampleEntryAtom *ptr = (MPEGAudioSampleEntryAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	WriteAudioSampleEntry((AudioSampleEntryAtom*)s, bs);
	return WriteAtom((Atom *)ptr->esd, bs);
}

M4Err mp4a_Size(Atom *s)
{
	M4Err e;
	MPEGAudioSampleEntryAtom *ptr = (MPEGAudioSampleEntryAtom *)s;
	e = Atom_Size(s);
	if (e) return e;

	SizeAudioSampleEntry((AudioSampleEntryAtom*)s);

	e = SizeAtom((Atom *)ptr->esd);
	if (e) return e;
	ptr->size += ptr->esd->size;
	return M4OK;
}

#endif //M4_READ_ONLY


void mp4s_del(Atom *s)
{
	MPEGSampleEntryAtom *ptr = (MPEGSampleEntryAtom *)s;
	if (ptr == NULL) return;
	if (ptr->esd) DelAtom((Atom *)ptr->esd);
	if (ptr->slc) OD_DeleteDescriptor((Descriptor **)&ptr->slc);
	free(ptr);
}


M4Err mp4s_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u64 sr;
	MPEGSampleEntryAtom *ptr = (MPEGSampleEntryAtom *)s;
	if (ptr == NULL) return M4BadParam;

	BS_ReadData(bs, (unsigned char*)ptr->reserved, 6);
	ptr->dataReferenceIndex = BS_ReadInt(bs, 16);
	*read += 8;
	e = ParseAtom((Atom **)&ptr->esd, bs, &sr);
	if (e) return e;
	*read += sr;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *mp4s_New()
{
	MPEGSampleEntryAtom *tmp = (MPEGSampleEntryAtom *) malloc(sizeof(MPEGSampleEntryAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MPEGSampleEntryAtom));

	tmp->type = MPEGSampleEntryAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err mp4s_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MPEGSampleEntryAtom *ptr = (MPEGSampleEntryAtom *)s;

	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteData(bs, (unsigned char*)ptr->reserved, 6);
	BS_WriteInt(bs, ptr->dataReferenceIndex, 16);
	return WriteAtom((Atom *)ptr->esd, bs);
}

M4Err mp4s_Size(Atom *s)
{
	M4Err e;
	MPEGSampleEntryAtom *ptr = (MPEGSampleEntryAtom *)s;
	
	e = Atom_Size(s);
	if (e) return e;
	ptr->size += 8;
	e = SizeAtom((Atom *)ptr->esd);
	if (e) return e;
	ptr->size += ptr->esd->size;
	return M4OK;
}

#endif //M4_READ_ONLY

void mp4v_del(Atom *s)
{
	MPEGVisualSampleEntryAtom *ptr = (MPEGVisualSampleEntryAtom *)s;
	if (ptr == NULL) return;
	if (ptr->esd) DelAtom((Atom *)ptr->esd);
	if (ptr->slc) OD_DeleteDescriptor((Descriptor **)&ptr->slc);
	free(ptr);
}


M4Err mp4v_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u64 sr;
	MPEGVisualSampleEntryAtom *ptr = (MPEGVisualSampleEntryAtom *)s;
	if (ptr == NULL) return M4BadParam;

	ReadVideoSampleEntry((VisualSampleEntryAtom *)ptr, bs, read);
	e = ParseAtom((Atom **)&ptr->esd, bs, &sr);
	if (e) return e;
	*read += sr;
	if (*read > ptr->size) return M4ReadAtomFailed;
	else BS_SkipBytes(bs, (u32) (ptr->size - *read));
	return M4OK;
}

Atom *mp4v_New()
{
	MPEGVisualSampleEntryAtom *tmp = (MPEGVisualSampleEntryAtom *) malloc(sizeof(MPEGVisualSampleEntryAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MPEGVisualSampleEntryAtom));

	InitVideoSampleEntry((VisualSampleEntryAtom *)tmp);
	tmp->type = MPEGVisualSampleEntryAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err mp4v_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MPEGVisualSampleEntryAtom *ptr = (MPEGVisualSampleEntryAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	WriteVideoSampleEntry((VisualSampleEntryAtom *)s, bs);
	return WriteAtom((Atom *)ptr->esd, bs);
}

M4Err mp4v_Size(Atom *s)
{
	M4Err e;
	MPEGVisualSampleEntryAtom *ptr = (MPEGVisualSampleEntryAtom *)s;
	e = Atom_Size(s);
	if (e) return e;

	SizeVideoSampleEntry((VisualSampleEntryAtom *)s);

	e = SizeAtom((Atom *)ptr->esd);
	if (e) return e;
	ptr->size += ptr->esd->size;
	return M4OK;
}

#endif //M4_READ_ONLY



#ifndef	M4_ISO_NO_FRAGMENTS

void mvex_del(Atom *s)
{
	MovieExtendsAtom *ptr = (MovieExtendsAtom *)s;
	if (ptr == NULL) return;
	DeleteAtomList(ptr->TrackExList);
	free(ptr);
}


M4Err mvex_AddAtom(MovieExtendsAtom *ptr, Atom *a)
{
	if (!a) return M4OK;
	if (!ptr) return M4BadParam;
	
	if (a->type != TrackExtendsAtomType) return M4BadParam;
	return ChainAddEntry(ptr->TrackExList, a);
}



M4Err mvex_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	Atom *a;
	u64 sr;
	MovieExtendsAtom *ptr = (MovieExtendsAtom *)s;

	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		*read += a->size;
		e = mvex_AddAtom(ptr, a);
		if (e) return e;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *mvex_New()
{
	MovieExtendsAtom *tmp = (MovieExtendsAtom *) malloc(sizeof(MovieExtendsAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MovieExtendsAtom));
	tmp->TrackExList = NewChain();
	if (!tmp->TrackExList) {
		free(tmp);
		return NULL;
	}
	tmp->type = MovieExtendsAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY


M4Err mvex_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MovieExtendsAtom *ptr = (MovieExtendsAtom *) s;
	if (!s) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	return WriteAtomList(s, ptr->TrackExList, bs);
}

M4Err mvex_Size(Atom *s)
{
	M4Err e;
	MovieExtendsAtom *ptr = (MovieExtendsAtom *)s;
	e = Atom_Size(s);
	if (e) return e;
	return SizeAtomList(s, ptr->TrackExList);
}



#endif //M4_READ_ONLY

#endif 


void mvhd_del(Atom *s)
{
	MovieHeaderAtom *ptr = (MovieHeaderAtom *)s;
	if (ptr == NULL) return;
	free(ptr);
}


M4Err mvhd_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	MovieHeaderAtom *ptr = (MovieHeaderAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	if (ptr->version == 1) {
		ptr->creationTime = BS_ReadInt(bs, 64);
		ptr->modificationTime = BS_ReadInt(bs, 64);
		ptr->timeScale = BS_ReadInt(bs, 32);
		ptr->duration = BS_ReadInt(bs, 64);
		*read += 28;
	} else {
		ptr->creationTime = BS_ReadInt(bs, 32);
		ptr->modificationTime = BS_ReadInt(bs, 32);
		ptr->timeScale = BS_ReadInt(bs, 32);
		ptr->duration = BS_ReadInt(bs, 32);
		*read += 16;
	}
	ptr->preferredRate = BS_ReadInt(bs, 32);
	ptr->preferredVolume = BS_ReadInt(bs, 16);
	BS_ReadData(bs, (unsigned char*)ptr->reserved, 10);
	ptr->matrixA = BS_ReadInt(bs, 32);
	ptr->matrixB = BS_ReadInt(bs, 32);
	ptr->matrixU = BS_ReadInt(bs, 32);
	ptr->matrixC = BS_ReadInt(bs, 32);
	ptr->matrixD = BS_ReadInt(bs, 32);
	ptr->matrixV = BS_ReadInt(bs, 32);
	ptr->matrixX = BS_ReadInt(bs, 32);
	ptr->matrixY = BS_ReadInt(bs, 32);
	ptr->matrixW = BS_ReadInt(bs, 32);
	ptr->previewTime = BS_ReadInt(bs, 32);
	ptr->previewDuration = BS_ReadInt(bs, 32);
	ptr->posterTime = BS_ReadInt(bs, 32);
	ptr->selectionTime = BS_ReadInt(bs, 32);
	ptr->selectionDuration = BS_ReadInt(bs, 32);
	ptr->currentTime = BS_ReadInt(bs, 32);
	ptr->nextTrackID = BS_ReadInt(bs, 32);
	*read += 80;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *mvhd_New()
{
	MovieHeaderAtom *tmp = (MovieHeaderAtom *) malloc(sizeof(MovieHeaderAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MovieHeaderAtom));

	InitFullAtom((Atom *)tmp);
	tmp->type = MovieHeaderAtomType;
	tmp->preferredRate = (1<<16);
	tmp->preferredVolume = (1<<8);

	tmp->matrixA = (1<<16);
	tmp->matrixD = (1<<16);
	tmp->matrixW = (1<<30);

	tmp->nextTrackID = 1;

	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err mvhd_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MovieHeaderAtom *ptr = (MovieHeaderAtom *)s;
	e = FullAtom_Write(s, bs);
	if (e) return e;
	if (ptr->version == 1) {
		BS_WriteLongInt(bs, ptr->creationTime, 64);
		BS_WriteLongInt(bs, ptr->modificationTime, 64);
		BS_WriteInt(bs, ptr->timeScale, 32);
		BS_WriteLongInt(bs, ptr->duration, 64);
	} else {
		BS_WriteInt(bs, (u32) ptr->creationTime, 32);
		BS_WriteInt(bs, (u32) ptr->modificationTime, 32);
		BS_WriteInt(bs, ptr->timeScale, 32);
		BS_WriteInt(bs, (u32) ptr->duration, 32);
	}
	BS_WriteInt(bs, ptr->preferredRate, 32);
	BS_WriteInt(bs, ptr->preferredVolume, 16);
	BS_WriteData(bs, (unsigned char*)ptr->reserved, 10);
	BS_WriteInt(bs, ptr->matrixA, 32);
	BS_WriteInt(bs, ptr->matrixB, 32);
	BS_WriteInt(bs, ptr->matrixU, 32);
	BS_WriteInt(bs, ptr->matrixC, 32);
	BS_WriteInt(bs, ptr->matrixD, 32);
	BS_WriteInt(bs, ptr->matrixV, 32);
	BS_WriteInt(bs, ptr->matrixX, 32);
	BS_WriteInt(bs, ptr->matrixY, 32);
	BS_WriteInt(bs, ptr->matrixW, 32);
	BS_WriteInt(bs, ptr->previewTime, 32);
	BS_WriteInt(bs, ptr->previewDuration, 32);
	BS_WriteInt(bs, ptr->posterTime, 32);
	BS_WriteInt(bs, ptr->selectionTime, 32);
	BS_WriteInt(bs, ptr->selectionDuration, 32);
	BS_WriteInt(bs, ptr->currentTime, 32);
	BS_WriteInt(bs, ptr->nextTrackID, 32);
	return M4OK;
}

M4Err mvhd_Size(Atom *s)
{
	M4Err e;
	MovieHeaderAtom *ptr = (MovieHeaderAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += (ptr->version == 1) ? 28 : 16;
	ptr->size += 80;
	return M4OK;
}

#endif //M4_READ_ONLY


void nmhd_del(Atom *s)
{
	MPEGMediaHeaderAtom *ptr = (MPEGMediaHeaderAtom *)s;
	if (ptr == NULL) return;
	free(ptr);
}



M4Err nmhd_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	MPEGMediaHeaderAtom *ptr = (MPEGMediaHeaderAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *nmhd_New()
{
	MPEGMediaHeaderAtom *tmp = (MPEGMediaHeaderAtom *) malloc(sizeof(MPEGMediaHeaderAtom));
	if (tmp == NULL) return NULL;
	InitFullAtom((Atom *) tmp);
	tmp->type = MPEGMediaHeaderAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err nmhd_Write(Atom *s, BitStream *bs)
{
	return FullAtom_Write(s, bs);
}

M4Err nmhd_Size(Atom *s)
{
	return FullAtom_Size(s);
}

#endif //M4_READ_ONLY



void padb_del(Atom *s)
{
	PaddingBitsAtom *ptr = (PaddingBitsAtom *) s;
	if (ptr == NULL) return;
	if (ptr->padbits) free(ptr->padbits);
	free(ptr);
}


M4Err padb_Read(Atom *s,BitStream *bs, u64 *read)
{
	M4Err e;
	u32 i;

	PaddingBitsAtom *ptr = (PaddingBitsAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read( s, bs, read);
	if (e) return e;

	ptr->SampleCount = BS_ReadInt(bs, 32);
	*read += 4;

	ptr->padbits = malloc(sizeof(u8)*ptr->SampleCount);
	for (i=0; i<ptr->SampleCount; i += 2) {
		BS_ReadInt(bs, 1);
		if (i+1 < ptr->SampleCount) {
			ptr->padbits[i+1] = BS_ReadInt(bs, 3);
		} else {
			BS_ReadInt(bs, 3);
		}
		BS_ReadInt(bs, 1);
		ptr->padbits[i] = BS_ReadInt(bs, 3);
		*read += 1;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *padb_New()
{
	PaddingBitsAtom *tmp;
	
	tmp = (PaddingBitsAtom *) malloc(sizeof(PaddingBitsAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(PaddingBitsAtom));

	InitFullAtom((Atom *)tmp);
	tmp->type = PaddingBitsAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err padb_Write(Atom *s, BitStream *bs)
{
	u32 i;
	M4Err e;
	PaddingBitsAtom *ptr = (PaddingBitsAtom *) s;

	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->SampleCount, 32);
	
	for (i=0 ; i<ptr->SampleCount; i += 2) {
		BS_WriteInt(bs, 0, 1);
		if (i+1 < ptr->SampleCount) {
			BS_WriteInt(bs, ptr->padbits[i+1], 3);
		} else {
			BS_WriteInt(bs, 0, 3);
		}
		BS_WriteInt(bs, 0, 1);
		BS_WriteInt(bs, ptr->padbits[i], 3);
	}
	return M4OK;
}

M4Err padb_Size(Atom *s)
{
	M4Err e;
	PaddingBitsAtom *ptr = (PaddingBitsAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 4;
	if (ptr->SampleCount) ptr->size += (ptr->SampleCount + 1) / 2;


	return M4OK;
}

#endif //M4_READ_ONLY


void rely_del(Atom *s)
{
	RelyHintEntry *rely = (RelyHintEntry *)s;
	free(rely);
}

M4Err rely_Read(Atom *s, BitStream *bs, u64 *read)
{
	RelyHintEntry *ptr = (RelyHintEntry *)s;
	if (ptr == NULL) return M4BadParam;

	ptr->reserved = BS_ReadInt(bs, 6);
	ptr->prefered = BS_ReadInt(bs, 1);
	ptr->required = BS_ReadInt(bs, 1);
	*read += 1;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *rely_New()
{
	RelyHintEntry *tmp = (RelyHintEntry *)malloc(sizeof(RelyHintEntry));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(RelyHintEntry));
	tmp->type = relyHintEntryType;

	return (Atom *)tmp;
}


#ifndef M4_READ_ONLY
M4Err rely_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	RelyHintEntry *ptr = (RelyHintEntry *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->reserved, 6);
	BS_WriteInt(bs, ptr->prefered, 1);
	BS_WriteInt(bs, ptr->required, 1);
	return M4OK;
}

M4Err rely_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 1;
	return M4OK;
}
#endif


void rtpo_del(Atom *s)
{
	RtpoAtom *rtpo = (RtpoAtom *)s;
	free(rtpo);
}

M4Err rtpo_Read(Atom *s, BitStream *bs, u64 *read)
{
	RtpoAtom *ptr = (RtpoAtom *)s;
	if (ptr == NULL) return M4BadParam;

	ptr->timeOffset = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *rtpo_New()
{
	RtpoAtom *tmp = (RtpoAtom *) malloc(sizeof(RtpoAtom));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(RtpoAtom));
	tmp->type = rtpoAtomType;
	return (Atom *)tmp;
}
#ifndef M4_READ_ONLY
M4Err rtpo_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	RtpoAtom *ptr = (RtpoAtom *)s;
	if (ptr == NULL) return M4BadParam;

	//here we have no pb, just remembed that some entries will have to
	//be 4-bytes aligned ...
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->timeOffset, 32);
	return M4OK;
}

M4Err rtpo_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}
#endif

void smhd_del(Atom *s)
{
	SoundMediaHeaderAtom *ptr = (SoundMediaHeaderAtom *)s;
	if (ptr == NULL ) return;
	free(ptr);
}


M4Err smhd_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	SoundMediaHeaderAtom *ptr = (SoundMediaHeaderAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	ptr->reserved = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *smhd_New()
{
	SoundMediaHeaderAtom *tmp = (SoundMediaHeaderAtom *) malloc(sizeof(SoundMediaHeaderAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(SoundMediaHeaderAtom));
	InitFullAtom((Atom *) tmp);
	tmp->type = SoundMediaHeaderAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err smhd_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	SoundMediaHeaderAtom *ptr = (SoundMediaHeaderAtom *)s;
	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->reserved, 32);
	return M4OK;
}

M4Err smhd_Size(Atom *s)
{
	M4Err e;
	SoundMediaHeaderAtom *ptr = (SoundMediaHeaderAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->reserved = 0;
	ptr->size += 4;
	return M4OK;
}

#endif //M4_READ_ONLY



void snro_del(Atom *s)
{
	SeqOffHintEntry *snro = (SeqOffHintEntry *)s;
	free(snro);
}

M4Err snro_Read(Atom *s, BitStream *bs, u64 *read)
{
	SeqOffHintEntry *ptr = (SeqOffHintEntry *)s;
	if (ptr == NULL) return M4BadParam;
	ptr->SeqOffset = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *snro_New()
{
	SeqOffHintEntry *tmp = (SeqOffHintEntry *) malloc(sizeof(SeqOffHintEntry));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(SeqOffHintEntry));
	tmp->type = snroHintEntryType;
	return (Atom *)tmp;
}


#ifndef M4_READ_ONLY
M4Err snro_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	SeqOffHintEntry *ptr = (SeqOffHintEntry *)s;
	if (ptr == NULL) return M4BadParam;

	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->SeqOffset, 32);
	return M4OK;
}

M4Err snro_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}
#endif


#define WRITE_SAMPLE_FRAGMENTS		1

void stbl_del(Atom *s)
{
	SampleTableAtom *ptr = (SampleTableAtom *)s;
	if (ptr == NULL) return;

	if (ptr->ChunkOffset) DelAtom(ptr->ChunkOffset);
	if (ptr->CompositionOffset) DelAtom((Atom *) ptr->CompositionOffset);
	if (ptr->DegradationPriority) DelAtom((Atom *) ptr->DegradationPriority);
	if (ptr->SampleDescription) DelAtom((Atom *) ptr->SampleDescription);
	if (ptr->SampleSize) DelAtom((Atom *) ptr->SampleSize);
	if (ptr->SampleToChunk) DelAtom((Atom *) ptr->SampleToChunk);
	if (ptr->ShadowSync) DelAtom((Atom *) ptr->ShadowSync);
	if (ptr->SyncSample) DelAtom((Atom *) ptr->SyncSample);
	if (ptr->TimeToSample) DelAtom((Atom *) ptr->TimeToSample);
	if (ptr->PaddingBits) DelAtom((Atom *) ptr->PaddingBits);
	if (ptr->Fragments) DelAtom((Atom *) ptr->Fragments);

	free(ptr);
}

M4Err stbl_AddAtom(SampleTableAtom *ptr, Atom *a)
{
	if (!a) return M4OK;
	switch (a->type) {
	case TimeToSampleAtomType:
		if (ptr->TimeToSample) return M4InvalidAtom;
		ptr->TimeToSample = (TimeToSampleAtom *)a;
		break;
	case CompositionOffsetAtomType:
		if (ptr->CompositionOffset) return M4InvalidAtom;
		ptr->CompositionOffset = (CompositionOffsetAtom *)a;
		break;
	case SyncSampleAtomType:
		if (ptr->SyncSample) return M4InvalidAtom;
		ptr->SyncSample = (SyncSampleAtom *)a;
		break;
	case SampleDescriptionAtomType:
		if (ptr->SampleDescription) return M4InvalidAtom;
		ptr->SampleDescription  =(SampleDescriptionAtom *)a;
		break;
	case CompactSampleSizeAtomType:
	case SampleSizeAtomType:
		if (ptr->SampleSize) return M4InvalidAtom;
		ptr->SampleSize = (SampleSizeAtom *)a;
		break;
	case SampleToChunkAtomType:
		if (ptr->SampleToChunk) return M4InvalidAtom;
		ptr->SampleToChunk = (SampleToChunkAtom *)a;
		break;
	case PaddingBitsAtomType:
		if (ptr->PaddingBits) return M4InvalidAtom;
		ptr->PaddingBits = (PaddingBitsAtom *) a;
		break;

	//WARNING: AS THIS MAY CHANGE DYNAMICALLY DURING EDIT, 
	case ChunkLargeOffsetAtomType:
	case ChunkOffsetAtomType:
		if (ptr->ChunkOffset) {
			DelAtom(ptr->ChunkOffset);
		}
		ptr->ChunkOffset = a;
		return M4OK;
	case ShadowSyncAtomType:
		if (ptr->ShadowSync) return M4InvalidAtom;
		ptr->ShadowSync = (ShadowSyncAtom *)a;
		break;
	case DegradationPriorityAtomType:
		if (ptr->DegradationPriority) return M4InvalidAtom;
		ptr->DegradationPriority = (DegradationPriorityAtom *)a;
		break;

	case SampleFragmentAtomType:
		if (ptr->Fragments) return M4InvalidAtom;
		ptr->Fragments = (SampleFragmentAtom *)a;
		break;

	//what's this atom ??? delete it
	default:
		DelAtom(a);
	}
	return M4OK;
}




M4Err stbl_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	Atom *a;
	u64 sr;
	//we need to parse DegPrior in a special way
	M4Err stdp_Read(Atom *s, BitStream *bs, u64 *read);

	SampleTableAtom *ptr = (SampleTableAtom *)s;
	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		//we need to read the DegPriority in a different way...
		if (a->type == DegradationPriorityAtomType) {
			if (!ptr->SampleSize) {
				DelAtom(a);
				return M4InvalidMP4File;
			}
			((DegradationPriorityAtom *)a)->entryCount = ptr->SampleSize->sampleCount;
			e = stdp_Read(a, bs, &sr);
			if (e) {
				DelAtom(a);
				return e;
			}
		}
		*read += a->size;
		e = stbl_AddAtom(ptr, a);
		if (e) return e;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *stbl_New()
{
	SampleTableAtom *tmp = (SampleTableAtom *) malloc(sizeof(SampleTableAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(SampleTableAtom));

	tmp->type = SampleTableAtomType;
	//maxSamplePer chunk is 10 by default
	tmp->MaxSamplePerChunk = 10;
	tmp->groupID = 1;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err stbl_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	SampleTableAtom *ptr = (SampleTableAtom *)s;
	if (!s) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;

	//stsd
	e = WriteAtom((Atom *) ptr->SampleDescription, bs);
	if (e) return e;
	//stts
	e = WriteAtom((Atom *) ptr->TimeToSample, bs);
	if (e) return e;
	//ctts
	if (ptr->CompositionOffset)	{
		e = WriteAtom((Atom *) ptr->CompositionOffset, bs);
		if (e) return e;
	}
	//stss
	if (ptr->SyncSample) {
		e = WriteAtom((Atom *) ptr->SyncSample, bs);
		if (e) return e;
	}
	//stsh
	if (ptr->ShadowSync) {
		e = WriteAtom((Atom *) ptr->ShadowSync, bs);
		if (e) return e;
	}
	//stsc
	e = WriteAtom((Atom *) ptr->SampleToChunk, bs);
	if (e) return e;
	//stsz
	e = WriteAtom((Atom *) ptr->SampleSize, bs);
	if (e) return e;
	//stco
	e = WriteAtom(ptr->ChunkOffset, bs);
	if (e) return e;
	//stdp
	if (ptr->DegradationPriority) {
		e = WriteAtom((Atom *) ptr->DegradationPriority, bs);
		if (e) return e;
	}
	//padb
	if (ptr->PaddingBits) {
		e = WriteAtom((Atom *) ptr->PaddingBits, bs);
		if (e) return e;
	}

#if WRITE_SAMPLE_FRAGMENTS
	//sampleFragments
	if (ptr->Fragments) {
		e = WriteAtom((Atom *) ptr->Fragments, bs);
		if (e) return e;
	}
#endif
	return M4OK;
}

M4Err stbl_Size(Atom *s)
{
	M4Err e;
	SampleTableAtom *ptr = (SampleTableAtom *)s;
	e = Atom_Size(s);
	if (e) return e;

	//Mandatory atoms
	e = SizeAtom((Atom *) ptr->SampleDescription);
	ptr->size += ptr->SampleDescription->size;
	e = SizeAtom((Atom *) ptr->SampleSize);
	if (e) return e;
	ptr->size += ptr->SampleSize->size;
	e = SizeAtom((Atom *) ptr->SampleToChunk);
	if (e) return e;
	ptr->size += ptr->SampleToChunk->size;
	e = SizeAtom((Atom *) ptr->TimeToSample);
	if (e) return e;
	ptr->size += ptr->TimeToSample->size;
	e = SizeAtom(ptr->ChunkOffset);
	if (e) return e;
	ptr->size += ptr->ChunkOffset->size;

	//optional atoms
	if (ptr->CompositionOffset)	{
		e = SizeAtom((Atom *) ptr->CompositionOffset);
		if (e) return e;
		ptr->size += ptr->CompositionOffset->size;
	}
	if (ptr->DegradationPriority) {
		e = SizeAtom((Atom *) ptr->DegradationPriority);
		if (e) return e;
		ptr->size += ptr->DegradationPriority->size;
	}
	if (ptr->ShadowSync) {
		e = SizeAtom((Atom *) ptr->ShadowSync);
		if (e) return e;
		ptr->size += ptr->ShadowSync->size;
	}
	if (ptr->SyncSample) {
		e = SizeAtom((Atom *) ptr->SyncSample);
		if (e) return e;
		ptr->size += ptr->SyncSample->size;
	}
	//padb
	if (ptr->PaddingBits) {
		e = SizeAtom((Atom *) ptr->PaddingBits);
		if (e) return e;
		ptr->size += ptr->PaddingBits->size;
	}
#if WRITE_SAMPLE_FRAGMENTS
	//sample fragments
	if (ptr->Fragments) {
		e = SizeAtom((Atom *) ptr->Fragments);
		if (e) return e;
		ptr->size += ptr->Fragments->size;
	}
#endif
	
	return M4OK;
}

#endif //M4_READ_ONLY


void stco_del(Atom *s)
{
	ChunkOffsetAtom *ptr = (ChunkOffsetAtom *)s;
	if (ptr == NULL) return;
	if (ptr->offsets) free(ptr->offsets);
	free(ptr);
}


M4Err stco_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 entries;
	ChunkOffsetAtom *ptr = (ChunkOffsetAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	ptr->entryCount = BS_ReadInt(bs, 32);
	*read += 4;
	if (ptr->entryCount) {
		ptr->offsets = (u32 *) malloc(ptr->entryCount * sizeof(u32) );
		if (ptr->offsets == NULL) return M4OutOfMem;

		for (entries = 0; entries < ptr->entryCount; entries++) {
			ptr->offsets[entries] = BS_ReadInt(bs, 32);
			*read += 4;
		}
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *stco_New()
{
	ChunkOffsetAtom *tmp = (ChunkOffsetAtom *) malloc(sizeof(ChunkOffsetAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(ChunkOffsetAtom));
	InitFullAtom((Atom *)tmp);
	tmp->type = ChunkOffsetAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err stco_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	ChunkOffsetAtom *ptr = (ChunkOffsetAtom *)s;
	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->entryCount, 32);
	for (i = 0; i < ptr->entryCount; i++) {
		BS_WriteInt(bs, ptr->offsets[i], 32);
	}
	return M4OK;
}


M4Err stco_Size(Atom *s)
{
	M4Err e;
	ChunkOffsetAtom *ptr = (ChunkOffsetAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 4 + (4 * ptr->entryCount);
	return M4OK;
}

#endif //M4_READ_ONLY



void stdp_del(Atom *s)
{
	DegradationPriorityAtom *ptr = (DegradationPriorityAtom *)s;
	if (ptr == NULL ) return;
	if (ptr->priorities) free(ptr->priorities);
	free(ptr);
}

//this is called through stbl_read...
M4Err stdp_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 entry;
	DegradationPriorityAtom *ptr = (DegradationPriorityAtom *)s;
	if (ptr == NULL) return M4BadParam;
	e = FullAtom_Read(s, bs, read);
	if (e) return e;

	ptr->priorities = (u16 *) malloc(ptr->entryCount * sizeof(u16));
	if (ptr->priorities == NULL) return M4OutOfMem;
	for (entry = 0; entry < ptr->entryCount; entry++) {
		//we have a bit for padding
		BS_ReadInt(bs, 1);
		ptr->priorities[entry] = BS_ReadInt(bs, 15);
		*read += 2;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *stdp_New()
{
	DegradationPriorityAtom *tmp = (DegradationPriorityAtom *) malloc(sizeof(DegradationPriorityAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(DegradationPriorityAtom));
	InitFullAtom((Atom *)tmp);
	tmp->type = DegradationPriorityAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err stdp_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	DegradationPriorityAtom *ptr = (DegradationPriorityAtom *)s;
	e = FullAtom_Write(s, bs);
	if (e) return e;

	for (i = 0; i < ptr->entryCount; i++) {
		BS_WriteInt(bs, 0, 1);
		BS_WriteInt(bs, ptr->priorities[i], 15);
	}
	return M4OK;
}

M4Err stdp_Size(Atom *s)
{
	M4Err e;
	DegradationPriorityAtom *ptr = (DegradationPriorityAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += (2 * ptr->entryCount);
	return M4OK;
}

#endif //M4_READ_ONLY


void stsc_del(Atom *s)
{
	u32 entryCount;
	u32 i;
	stscEntry *ent;
	SampleToChunkAtom *ptr = (SampleToChunkAtom *)s;
	if (ptr == NULL) return;

	entryCount = ChainGetCount(ptr->entryList);
	if (entryCount) {
		for (i = 0; i < entryCount; i++) {
			ent = (stscEntry*)ChainGetEntry(ptr->entryList, i);
			if (ent) free(ent);
		}
	}
	DeleteChain(ptr->entryList);
	free(ptr);
}


M4Err stsc_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 i;
	u32 entryCount;
	stscEntry *ent, *firstEnt;
	SampleToChunkAtom *ptr = (SampleToChunkAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	entryCount = BS_ReadInt(bs, 32);
	*read += 4;
	
	firstEnt = NULL;
	for (i = 0; i < entryCount; i++) {
		ent = (stscEntry *)malloc(sizeof(stscEntry));
		if (!ent) return M4OutOfMem;
		ent->firstChunk = BS_ReadInt(bs, 32);
		ent->samplesPerChunk = BS_ReadInt(bs, 32);
		ent->sampleDescriptionIndex = BS_ReadInt(bs, 32);
		ent->isEdited = 0;
		ent->nextChunk = 0;

		//create our cache at first load
		if (! ptr->currentEntry) {
			firstEnt = ent;
		} else {
			//update the next chunk
			ptr->currentEntry->nextChunk = ent->firstChunk;
		}
		ptr->currentEntry = ent;
		e = ChainAddEntry(ptr->entryList, ent);
		if (e) return e;
		*read += 12;
	}
	//create our cache
	if (firstEnt) {
		ptr->currentEntry = firstEnt;
		ptr->currentIndex = 0;
		ptr->firstSampleInCurrentChunk = 0;
		ptr->currentChunk = 0;
		ptr->ghostNumber = 0;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *stsc_New()
{
	SampleToChunkAtom *tmp = (SampleToChunkAtom *) malloc(sizeof(SampleToChunkAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(SampleToChunkAtom));
	InitFullAtom((Atom *)tmp);
	tmp->entryList = NewChain();
	if (tmp->entryList == NULL) {
		free(tmp);
		return NULL;
	}
	tmp->type = SampleToChunkAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err stsc_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;	
	u32 entryCount;
	stscEntry *ent;
	SampleToChunkAtom *ptr = (SampleToChunkAtom *)s;

	e = FullAtom_Write(s, bs);
	if (e) return e;
    entryCount = ChainGetCount(ptr->entryList);
	BS_WriteInt(bs, entryCount, 32);
	for (i = 0; i < entryCount; i++) {
		ent = (stscEntry*)ChainGetEntry(ptr->entryList, i);
		BS_WriteInt(bs, ent->firstChunk, 32);
		BS_WriteInt(bs, ent->samplesPerChunk, 32);
		BS_WriteInt(bs, ent->sampleDescriptionIndex, 32);
	}
	return M4OK;
}

M4Err stsc_Size(Atom *s)
{
	M4Err e;
	SampleToChunkAtom *ptr = (SampleToChunkAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 4 + (12 * ChainGetCount(ptr->entryList));
	return M4OK;
}

#endif //M4_READ_ONLY

void stsd_del(Atom *s)
{
	SampleDescriptionAtom *ptr = (SampleDescriptionAtom *)s;
	if (ptr == NULL) return;
	DeleteAtomList(ptr->atomList);
	free(ptr);
}

M4Err stsd_AddAtom(SampleDescriptionAtom *ptr, Atom *a)
{
	UnknownAtom *def;
	if (!a) return M4OK;

	switch (a->type) {
	case MPEGSampleEntryAtomType:
	case MPEGAudioSampleEntryAtomType:
	case MPEGVisualSampleEntryAtomType:
	case AMRSampleEntryAtomType:
	case WB_AMRSampleEntryAtomType:
	case H263SampleEntryAtomType:
	case GenericHintSampleEntryAtomType:
	case RTPHintSampleEntryAtomType:
	case AVCSampleEntryAtomType:
	case TextSampleEntryAtomType:
		return ChainAddEntry(ptr->atomList, a);

	//unknown sample description: we need a specific atom to handle the data ref index
	//rather than a default atom ...
	default:
		def = (UnknownAtom *)a;
		/*we need at least 8 bytes for unknown sample entries*/
		if (def->dataSize < 8) {
			DelAtom(a);
			return M4OK;
		}
		return ChainAddEntry(ptr->atomList, a);
	}
}


M4Err stsd_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 entryCount;
	u32 i;
	u64 sr;
	Atom *a;
	SampleDescriptionAtom *ptr = (SampleDescriptionAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	entryCount = BS_ReadInt(bs, 32);
	*read += 4;
	for (i = 0; i < entryCount; i++) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		*read += a->size;
		e = stsd_AddAtom(ptr, a);
		if (e) return e;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *stsd_New()
{
	SampleDescriptionAtom *tmp = (SampleDescriptionAtom *) malloc(sizeof(SampleDescriptionAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(SampleDescriptionAtom));
	InitFullAtom((Atom *)tmp);
	tmp->atomList = NewChain();
	if (! tmp->atomList) {
		free(tmp);
		return NULL;
	}
	tmp->type = SampleDescriptionAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err stsd_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 entryCount;
	SampleDescriptionAtom *ptr = (SampleDescriptionAtom *)s;

	e = FullAtom_Write(s, bs);
	if (e) return e;
	entryCount = ChainGetCount(ptr->atomList);
	BS_WriteInt(bs, entryCount, 32);
	return WriteAtomList(s, ptr->atomList, bs);
}

M4Err stsd_Size(Atom *s)
{
	M4Err e;
	SampleDescriptionAtom *ptr = (SampleDescriptionAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 4;
	return SizeAtomList(s, ptr->atomList);
}

#endif //M4_READ_ONLY

void stsf_del(Atom *s)
{
	u32 entryCount;
	u32 i;
	stsfEntry *pe;
	SampleFragmentAtom *ptr = (SampleFragmentAtom *)s;
	if (ptr == NULL) return;
	
	if (ptr->entryList) {
		entryCount = ChainGetCount(ptr->entryList);
		for ( i = 0; i < entryCount; i++ ) {
			pe = (stsfEntry*)ChainGetEntry(ptr->entryList, i);
			if (pe->fragmentSizes) free(pe->fragmentSizes);
			free(pe);	
		}
		DeleteChain(ptr->entryList);
	}
	free(ptr);
}



M4Err stsf_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 entries, i;
	u32 entryCount;
	stsfEntry *p;
	SampleFragmentAtom *ptr = (SampleFragmentAtom *)s;
	
	p = NULL;
	if (!ptr) return M4BadParam;
	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	entryCount = BS_ReadInt(bs, 32);
	*read += 4;

	p = NULL;
	for ( entries = 0; entries < entryCount; entries++ ) {
		p = (stsfEntry *) malloc(sizeof(stsfEntry));
		if (!p) return M4OutOfMem;
		p->SampleNumber = BS_ReadInt(bs, 32);
		p->fragmentCount = BS_ReadInt(bs, 32);
		*read += 8;
		p->fragmentSizes = malloc(sizeof(stsfEntry) * p->fragmentCount);
		for (i=0; i<p->fragmentCount; i++) {
			p->fragmentSizes[i] = BS_ReadInt(bs, 16);
			*read += 2;
		}
		ChainAddEntry(ptr->entryList, p);
	}
#ifndef M4_READ_ONLY
	ptr->w_currentEntry = p;
	ptr->w_currentEntryIndex = entryCount-1;
#endif
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *stsf_New()
{
	SampleFragmentAtom *tmp;
	
	tmp = (SampleFragmentAtom *) malloc(sizeof(SampleFragmentAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(SampleFragmentAtom));

	InitFullAtom((Atom *) tmp);
	tmp->entryList = NewChain();
	if (! tmp->entryList) {
		free(tmp);
		return NULL;
	}
	tmp->type = SampleFragmentAtomType;
	return (Atom *) tmp;
}



//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err stsf_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i, j;
	u32 entryCount;
	stsfEntry *p;
	SampleFragmentAtom *ptr = (SampleFragmentAtom *)s;
	
	e = FullAtom_Write(s, bs);
	if (e) return e;
    entryCount = ChainGetCount(ptr->entryList);
	BS_WriteInt(bs, entryCount, 32);
	for ( i = 0; i < entryCount; i++ ) {
		p = (stsfEntry*)ChainGetEntry(ptr->entryList, i);
		BS_WriteInt(bs, p->SampleNumber, 32);
		BS_WriteInt(bs, p->fragmentCount, 32);
		for (j=0;j<p->fragmentCount;j++) {
			BS_WriteInt(bs, p->fragmentSizes[j], 16);
		}
	}
	return M4OK;
}

M4Err stsf_Size(Atom *s)
{
	M4Err e;
	stsfEntry *p;
	u32 entryCount, i;
	SampleFragmentAtom *ptr = (SampleFragmentAtom *) s;

	e = FullAtom_Size(s);
	if (e) return e;
	entryCount = ChainGetCount(ptr->entryList);
	ptr->size += 4;
	for (i=0;i<entryCount; i++) {
		p = ChainGetEntry(ptr->entryList, i);
		ptr->size += 8 + 2*p->fragmentCount;
	}
	return M4OK;
}

#endif //M4_READ_ONLY

void stsh_del(Atom *s)
{
	ShadowSyncAtom *ptr = (ShadowSyncAtom *)s;
	if (ptr == NULL) return;
	DeleteChain(ptr->entries);
	free(ptr);
}



M4Err stsh_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 count, i;
	stshEntry *ent;
	ShadowSyncAtom *ptr = (ShadowSyncAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	count = BS_ReadInt(bs, 32);
	*read += 4;
	for (i = 0; i < count; i++) {
		ent = (stshEntry *) malloc(sizeof(stshEntry));
		if (!ent) return M4OutOfMem;
		ent->shadowedSampleNumber = BS_ReadInt(bs, 32);
		ent->syncSampleNumber = BS_ReadInt(bs, 32);
		*read += 8;
		e = ChainAddEntry(ptr->entries, ent);
		if (e) return e;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *stsh_New()
{
	ShadowSyncAtom *tmp = (ShadowSyncAtom *) malloc(sizeof(ShadowSyncAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(ShadowSyncAtom));
	InitFullAtom((Atom *)tmp);
	tmp->entries = NewChain();
	if (!tmp->entries) {
		free(tmp);
		return NULL;
	}
	tmp->type = ShadowSyncAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err stsh_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	stshEntry *ent;
	ShadowSyncAtom *ptr = (ShadowSyncAtom *)s;

	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ChainGetCount(ptr->entries), 32);
	for (i = 0; i < ChainGetCount(ptr->entries); i++) {
		ent = (stshEntry*)ChainGetEntry(ptr->entries, i);
		BS_WriteInt(bs, ent->shadowedSampleNumber, 32);
		BS_WriteInt(bs, ent->syncSampleNumber, 32);
	}
	return M4OK;
}

M4Err stsh_Size(Atom *s)
{
	M4Err e;
	ShadowSyncAtom *ptr = (ShadowSyncAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 4 + (8 * ChainGetCount(ptr->entries));
	return M4OK;
}

#endif //M4_READ_ONLY



void stss_del(Atom *s)
{
	SyncSampleAtom *ptr = (SyncSampleAtom *)s;
	if (ptr == NULL) return;
	if (ptr->sampleNumbers) free(ptr->sampleNumbers);
	free(ptr);
}

M4Err stss_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 i;
	SyncSampleAtom *ptr = (SyncSampleAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	ptr->entryCount = BS_ReadInt(bs, 32);
	*read += 4;
	ptr->sampleNumbers = (u32 *) malloc( (ptr->entryCount + 1) * sizeof(u32));
	if (ptr->sampleNumbers == NULL) return M4OutOfMem;

	for (i = 0; i < ptr->entryCount; i++) {
		ptr->sampleNumbers[i] = BS_ReadInt(bs, 32);
		*read += 4;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *stss_New()
{
	SyncSampleAtom *tmp = (SyncSampleAtom *) malloc(sizeof(SyncSampleAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(SyncSampleAtom));
	InitFullAtom((Atom *)tmp);
	tmp->type = SyncSampleAtomType;
	return (Atom*)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err stss_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	SyncSampleAtom *ptr = (SyncSampleAtom *)s;

	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->entryCount, 32);
	for (i = 0; i < ptr->entryCount; i++) {
		BS_WriteInt(bs, ptr->sampleNumbers[i], 32);
	}
	return M4OK;
}

M4Err stss_Size(Atom *s)
{
	M4Err e;
	SyncSampleAtom *ptr = (SyncSampleAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 4 + (4 * ptr->entryCount);
	return M4OK;
}

#endif //M4_READ_ONLY


void stsz_del(Atom *s)
{
	SampleSizeAtom *ptr = (SampleSizeAtom *)s;
	if (ptr == NULL) return;
	if (ptr->sizes) free(ptr->sizes);
	free(ptr);
}


M4Err stsz_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 i, estSize;
	SampleSizeAtom *ptr = (SampleSizeAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	//support for CompactSizes
	if (s->type == SampleSizeAtomType) {
		ptr->sampleSize = BS_ReadInt(bs, 32);
		ptr->sampleCount = BS_ReadInt(bs, 32);
		*read += 8;
	} else {
		//24-reserved
		BS_ReadInt(bs, 24);
		i = BS_ReadInt(bs, 8);
		ptr->sampleCount = BS_ReadInt(bs, 32);
		*read += 8;
		switch (i) {
		case 4:
		case 8:
		case 16:
			ptr->sampleSize = i;
			break;
		default:
			//try to fix the file
			//no samples, no parsing pb
			if (!ptr->sampleCount) {
				ptr->sampleSize = 16;
				return M4OK;
			}			
			estSize = (u32) (ptr->size - *read) / ptr->sampleCount;
			if (!estSize && ((ptr->sampleCount+1)/2 == (ptr->size - *read)) ) {
				ptr->sampleSize = 4;
				break;
			} else if (estSize == 1 || estSize == 2) {
				ptr->sampleSize = 8 * estSize;
			} else {
				return M4InvalidMP4File;
			}
		}
	}
	if (s->type == SampleSizeAtomType) {
		if (! ptr->sampleSize && ptr->sampleCount) {
			ptr->sizes = (u32 *) malloc(ptr->sampleCount * sizeof(u32));
			if (! ptr->sizes) return M4OutOfMem;
			for (i = 0; i < ptr->sampleCount; i++) {
				ptr->sizes[i] = BS_ReadInt(bs, 32);
				*read += 4;
			}
		}
	} else {
		//note we could optimize the mem usage by keeping the table compact
		//in memory. But that would complicate both caching and editing
		//we therefore keep all sizes as u32 and uncompress the table
		ptr->sizes = (u32 *) malloc(ptr->sampleCount * sizeof(u32));
		if (! ptr->sizes) return M4OutOfMem;

		for (i = 0; i < ptr->sampleCount; ) {
			switch (ptr->sampleSize) {
			case 4:
				ptr->sizes[i] = BS_ReadInt(bs, 4);
				if (i+1 < ptr->sampleCount) {
					ptr->sizes[i+1] = BS_ReadInt(bs, 4);
				} else {
					//0 padding in odd sample count
					BS_ReadInt(bs, 4);
				}
				*read += 1;
				i += 2;
				break;
			default:
				ptr->sizes[i] = BS_ReadInt(bs, ptr->sampleSize);
				*read += ptr->sampleSize/8;
				i += 1;
				break;
			}
		}
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *stsz_New()
{
	SampleSizeAtom *tmp = (SampleSizeAtom *) malloc(sizeof(SampleSizeAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(SampleSizeAtom));

	InitFullAtom((Atom *)tmp);
	//type is unknown here, can be regular or compact table
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err stsz_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	SampleSizeAtom *ptr = (SampleSizeAtom *)s;
	
	e = FullAtom_Write(s, bs);
	if (e) return e;
	//in both versions this is still valid
	if (ptr->type == SampleSizeAtomType) {
		BS_WriteInt(bs, ptr->sampleSize, 32);
	} else {
		BS_WriteInt(bs, 0, 24);
		BS_WriteInt(bs, ptr->sampleSize, 8);
	}
	BS_WriteInt(bs, ptr->sampleCount, 32);

	if (ptr->type == SampleSizeAtomType) {
		if (! ptr->sampleSize) {
			for (i = 0; i < ptr->sampleCount; i++) {
				BS_WriteInt(bs, ptr->sizes[i], 32);
			}
		}
	} else {
		for (i = 0; i < ptr->sampleCount; ) {
			switch (ptr->sampleSize) {
			case 4:
				BS_WriteInt(bs, ptr->sizes[i], 4);
				if (i+1 < ptr->sampleCount) {
					BS_WriteInt(bs, ptr->sizes[i+1], 4);
				} else {
					//0 padding in odd sample count
					BS_WriteInt(bs, 0, 4);
				}
				i += 2;
				break;
			default:
				BS_WriteInt(bs, ptr->sizes[i], ptr->sampleSize);
				i += 1;
				break;
			}
		}
	}
	return M4OK;
}

M4Err stsz_Size(Atom *s)
{
	M4Err e;
	u32 i, fieldSize, size;

	SampleSizeAtom *ptr = (SampleSizeAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;

	ptr->size += 8;
	if (!ptr->sampleCount) return M4OK;

	//regular table
	if (ptr->type == SampleSizeAtomType) {
		if (ptr->sampleSize) return M4OK;
		ptr->size += (4 * ptr->sampleCount);
		return M4OK;
	}

	//we could let the user decide what to use but we'd rather keep it simple
	//in the API. Therefore we only recompute the size field and automatically
	//switch the ATOM TYPE
	//start with min field size and increment
	fieldSize = 4;
	size = ptr->sizes[0];

	for (i=0; i < ptr->sampleCount; i++) {
		if (ptr->sizes[i] <= 0xF) continue;
		//switch to 8-bit table
		else if (ptr->sizes[i] <= 0xFF) {
			fieldSize = 8;
		}
		//switch to 16-bit table
		else if (ptr->sizes[i] <= 0xFFFF) {
			fieldSize = 16;
		}
		//switch to 32-bit table
		else {
			fieldSize = 32;
		}

		//check the size
		if (size != ptr->sizes[i]) size = 0;
	}
	//if all samples are of the same size, switch to regular (more compact)
	if (size) {
		ptr->type = SampleSizeAtomType;
		ptr->sampleSize = size;
		free(ptr->sizes);
		ptr->sizes = NULL;
	}

	if (fieldSize == 32) {
		//oops, doesn't fit in a compact table
		ptr->type = SampleSizeAtomType;
		ptr->size += (4 * ptr->sampleCount);
		return M4OK;
	}
	
	//make sure we are a compact table (no need to change the mem representation)
	ptr->type = CompactSampleSizeAtomType;
	ptr->sampleSize = fieldSize;
	if (fieldSize == 4) {
		//do not forget the 0 padding field for odd count
		ptr->size += (ptr->sampleCount + 1) / 2;
	} else {
		ptr->size += (ptr->sampleCount) * (fieldSize/8);
	}
	return M4OK;
}

#endif //M4_READ_ONLY


void stts_del(Atom *s)
{
	u32 i;
	sttsEntry *ent;
	TimeToSampleAtom *ptr = (TimeToSampleAtom *)s;
	if (ptr == NULL) return;
	if (ptr->entryList) {
		for (i = 0; i < ChainGetCount(ptr->entryList); i++) {
			ent = (sttsEntry*)ChainGetEntry(ptr->entryList, i);
			if (ent) free(ent);
		}
		DeleteChain(ptr->entryList);
	}
	free(ptr);
}


M4Err stts_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 i;
	u32 entryCount;
	sttsEntry *ent;
	TimeToSampleAtom *ptr = (TimeToSampleAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;

	ent = NULL;
#ifndef M4_READ_ONLY
	ptr->w_LastDTS = 0;
#endif
	entryCount = BS_ReadInt(bs, 32);
	*read += 4;
	for (i = 0; i < entryCount; i++) {
		ent = (sttsEntry *) malloc(sizeof(sttsEntry));
		if (! ent) return M4OutOfMem;
		ent->sampleCount = BS_ReadInt(bs, 32);
		ent->sampleDelta = BS_ReadInt(bs, 32);
		e = ChainAddEntry(ptr->entryList, ent);
		if (e) return e;
		*read += 8;
#ifndef M4_READ_ONLY
		ptr->w_currentEntry = ent;
		ptr->w_currentSampleNum += ent->sampleCount;
		ptr->w_LastDTS += ent->sampleCount * ent->sampleDelta;
#endif
	}
	//remove the last sample delta.
#ifndef M4_READ_ONLY
	if (ent) ptr->w_LastDTS -= ent->sampleDelta;
#endif
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *stts_New()
{
	TimeToSampleAtom *tmp = (TimeToSampleAtom *) malloc(sizeof(TimeToSampleAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(TimeToSampleAtom));
	InitFullAtom((Atom *)tmp);
	tmp->entryList = NewChain();
	if (!tmp->entryList) {
		free(tmp);
		return NULL;
	}
	tmp->type = TimeToSampleAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err stts_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	sttsEntry *ent;
	TimeToSampleAtom *ptr = (TimeToSampleAtom *)s;
	
	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ChainGetCount(ptr->entryList), 32);
	for (i = 0; i < ChainGetCount(ptr->entryList); i++) {
		ent = (sttsEntry*)ChainGetEntry(ptr->entryList, i);
		BS_WriteInt(bs, ent->sampleCount, 32);
		BS_WriteInt(bs, ent->sampleDelta, 32);
	}
	return M4OK;
}

M4Err stts_Size(Atom *s)
{
	M4Err e;
	TimeToSampleAtom *ptr = (TimeToSampleAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 4 + (8 * ChainGetCount(ptr->entryList));
	return M4OK;
}


#endif //M4_READ_ONLY


#ifndef	M4_ISO_NO_FRAGMENTS

void tfhd_del(Atom *s)
{
	TrackFragmentHeaderAtom *ptr = (TrackFragmentHeaderAtom *)s;
	if (ptr == NULL) return;
	free(ptr);
}

M4Err tfhd_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	TrackFragmentHeaderAtom *ptr = (TrackFragmentHeaderAtom *)s;
	
	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	
	ptr->trackID = BS_ReadInt(bs, 32);
	*read += 4;

	//The rest depends on the flags
	if (ptr->flags & TF_BASE_OFFSET) {
		ptr->base_data_offset = BS_ReadInt(bs, 64);
		*read += 8;
	}
	if (ptr->flags & TF_SAMPLE_DESC) {
		ptr->sample_desc_index = BS_ReadInt(bs, 32);
		*read += 4;
	}
	if (ptr->flags & TF_SAMPLE_DUR) {
		ptr->def_sample_duration = BS_ReadInt(bs, 32);
		*read += 4;
	}
	if (ptr->flags & TF_SAMPLE_SIZE) {
		ptr->def_sample_size = BS_ReadInt(bs, 32);
		*read += 4;
	}
	if (ptr->flags & TF_SAMPLE_FLAGS) {
		ptr->def_sample_flags = BS_ReadInt(bs, 32);
		*read += 4;
	}
	
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *tfhd_New()
{
	TrackFragmentHeaderAtom *tmp = (TrackFragmentHeaderAtom *) malloc(sizeof(TrackFragmentHeaderAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(TrackFragmentHeaderAtom));
	tmp->type = TrackFragmentHeaderAtomType;
	//NO FLAGS SET BY DEFAULT
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY


M4Err tfhd_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TrackFragmentHeaderAtom *ptr = (TrackFragmentHeaderAtom *) s;
	if (!s) return M4BadParam;

	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->trackID, 32);

	//The rest depends on the flags
	if (ptr->flags & TF_BASE_OFFSET) {
		BS_WriteLongInt(bs, ptr->base_data_offset, 64);
	}
	if (ptr->flags & TF_SAMPLE_DESC) {
		BS_WriteInt(bs, ptr->sample_desc_index, 32);
	}
	if (ptr->flags & TF_SAMPLE_DUR) {
		BS_WriteInt(bs, ptr->def_sample_duration, 32);
	}
	if (ptr->flags & TF_SAMPLE_SIZE) {
		BS_WriteInt(bs, ptr->def_sample_size, 32);
	}
	if (ptr->flags & TF_SAMPLE_FLAGS) {
		BS_WriteInt(bs, ptr->def_sample_flags, 32);
	}
	return M4OK;
}

M4Err tfhd_Size(Atom *s)
{
	M4Err e;
	TrackFragmentHeaderAtom *ptr = (TrackFragmentHeaderAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 4;

	//The rest depends on the flags
	if (ptr->flags & TF_BASE_OFFSET) ptr->size += 8;
	if (ptr->flags & TF_SAMPLE_DESC) ptr->size += 4;
	if (ptr->flags & TF_SAMPLE_DUR) ptr->size += 4;
	if (ptr->flags & TF_SAMPLE_SIZE) ptr->size += 4;
	if (ptr->flags & TF_SAMPLE_FLAGS) ptr->size += 4;
	return M4OK;
}



#endif //M4_READ_ONLY

#endif 

void tims_del(Atom *s)
{
	TSHintEntry *tims = (TSHintEntry *)s;
	free(tims);
}

M4Err tims_Read(Atom *s, BitStream *bs, u64 *read)
{
	TSHintEntry *ptr = (TSHintEntry *)s;
	if (ptr == NULL) return M4BadParam;

	ptr->timeScale = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *tims_New()
{
	TSHintEntry *tmp = (TSHintEntry *) malloc(sizeof(TSHintEntry));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(TSHintEntry));
	tmp->type = timsHintEntryType;
	return (Atom *)tmp;
}

#ifndef M4_READ_ONLY

M4Err tims_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TSHintEntry *ptr = (TSHintEntry *)s;
	if (ptr == NULL) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->timeScale, 32);
	return M4OK;
}

M4Err tims_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}

#endif	


void tkhd_del(Atom *s)
{
	TrackHeaderAtom *ptr = (TrackHeaderAtom *)s;
	if (ptr == NULL) return;
	free(ptr);
	return;
}


M4Err tkhd_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	TrackHeaderAtom *ptr = (TrackHeaderAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	if (ptr->version == 1) {
		ptr->creationTime = BS_ReadInt(bs, 64);
		ptr->modificationTime = BS_ReadInt(bs, 64);
		ptr->trackID = BS_ReadInt(bs, 32);
		ptr->reserved1 = BS_ReadInt(bs, 32);
		ptr->duration = BS_ReadInt(bs, 64);
		*read += 32;
	} else {
		ptr->creationTime = BS_ReadInt(bs, 32);
		ptr->modificationTime = BS_ReadInt(bs, 32);
		ptr->trackID = BS_ReadInt(bs, 32);
		ptr->reserved1 = BS_ReadInt(bs, 32);
		ptr->duration = BS_ReadInt(bs, 32);
		*read += 20;
	}
	ptr->reserved2[0] = BS_ReadInt(bs, 32);
	ptr->reserved2[1] = BS_ReadInt(bs, 32);
	ptr->layer = BS_ReadInt(bs, 16);
	ptr->alternate_group = BS_ReadInt(bs, 16);
	ptr->volume = BS_ReadInt(bs, 16);
	ptr->reserved3 = BS_ReadInt(bs, 16);
	ptr->matrix[0] = BS_ReadInt(bs, 32);
	ptr->matrix[1] = BS_ReadInt(bs, 32);
	ptr->matrix[2] = BS_ReadInt(bs, 32);
	ptr->matrix[3] = BS_ReadInt(bs, 32);
	ptr->matrix[4] = BS_ReadInt(bs, 32);
	ptr->matrix[5] = BS_ReadInt(bs, 32);
	ptr->matrix[6] = BS_ReadInt(bs, 32);
	ptr->matrix[7] = BS_ReadInt(bs, 32);
	ptr->matrix[8] = BS_ReadInt(bs, 32);
	ptr->width = BS_ReadInt(bs, 32);
	ptr->height = BS_ReadInt(bs, 32);
	*read += 60;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *tkhd_New()
{
	TrackHeaderAtom *tmp = (TrackHeaderAtom *) malloc(sizeof(TrackHeaderAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(TrackHeaderAtom));
	InitFullAtom((Atom *)tmp);
	tmp->type = TrackHeaderAtomType;
	tmp->matrix[0] = 0x00010000;
	tmp->matrix[4] = 0x00010000;
	tmp->matrix[8] = 0x40000000;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err tkhd_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TrackHeaderAtom *ptr = (TrackHeaderAtom *)s;

	e = FullAtom_Write(s, bs);
	if (e) return e;
	if (ptr->version == 1) {
		BS_WriteLongInt(bs, ptr->creationTime, 64);
		BS_WriteLongInt(bs, ptr->modificationTime, 64);
		BS_WriteInt(bs, ptr->trackID, 32);
		BS_WriteInt(bs, ptr->reserved1, 32);
		BS_WriteLongInt(bs, ptr->duration, 64);
	} else {
		BS_WriteInt(bs, (u32) ptr->creationTime, 32);
		BS_WriteInt(bs, (u32) ptr->modificationTime, 32);
		BS_WriteInt(bs, ptr->trackID, 32);
		BS_WriteInt(bs, ptr->reserved1, 32);
		BS_WriteInt(bs, (u32) ptr->duration, 32);
	}
	BS_WriteInt(bs, ptr->reserved2[0], 32);
	BS_WriteInt(bs, ptr->reserved2[1], 32);
	BS_WriteInt(bs, ptr->layer, 16);
	BS_WriteInt(bs, ptr->alternate_group, 16);
	BS_WriteInt(bs, ptr->volume, 16);
	BS_WriteInt(bs, ptr->reserved3, 16);
	BS_WriteInt(bs, ptr->matrix[0], 32);
	BS_WriteInt(bs, ptr->matrix[1], 32);
	BS_WriteInt(bs, ptr->matrix[2], 32);
	BS_WriteInt(bs, ptr->matrix[3], 32);
	BS_WriteInt(bs, ptr->matrix[4], 32);
	BS_WriteInt(bs, ptr->matrix[5], 32);
	BS_WriteInt(bs, ptr->matrix[6], 32);
	BS_WriteInt(bs, ptr->matrix[7], 32);
	BS_WriteInt(bs, ptr->matrix[8], 32);
	BS_WriteInt(bs, ptr->width, 32);
	BS_WriteInt(bs, ptr->height, 32);
	return M4OK;
}

M4Err tkhd_Size(Atom *s)
{
	M4Err e;
	TrackHeaderAtom *ptr = (TrackHeaderAtom *)s;
	
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += (ptr->version == 1) ? 32 : 20;
	ptr->size += 60;
	return M4OK;
}

#endif //M4_READ_ONLY



#ifndef	M4_ISO_NO_FRAGMENTS

void traf_del(Atom *s)
{
	TrackFragmentAtom *ptr = (TrackFragmentAtom *)s;
	if (ptr == NULL) return;
	if (ptr->tfhd) DelAtom((Atom *) ptr->tfhd);
	DeleteAtomList(ptr->TrackRuns);
	free(ptr);
}

M4Err traf_AddAtom(TrackFragmentAtom *ptr, Atom *a)
{
	if (!a) return M4OK;
	if (!ptr) return M4BadParam;
	
	switch (a->type) {
	case TrackFragmentHeaderAtomType:
		if (ptr->tfhd) return M4InvalidMP4File;
		ptr->tfhd = (TrackFragmentHeaderAtom *) a;
		return M4OK;
	case TrackFragmentRunAtomType:
		return ChainAddEntry(ptr->TrackRuns, a);

	default:
		return M4InvalidAtom;
	}
}


M4Err traf_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	Atom *a;
	u64 sr;
	TrackFragmentAtom *ptr = (TrackFragmentAtom *)s;

	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		*read += a->size;
		e = traf_AddAtom(ptr, a);
		if (e) return e;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *traf_New()
{
	TrackFragmentAtom *tmp = (TrackFragmentAtom *) malloc(sizeof(TrackFragmentAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(TrackFragmentAtom));
	tmp->type = TrackFragmentAtomType;
	tmp->TrackRuns = NewChain();
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY


M4Err traf_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TrackFragmentAtom *ptr = (TrackFragmentAtom *) s;
	if (!s) return M4BadParam;

	e = Atom_Write(s, bs);
	if (e) return e;

	//Header first
	if (ptr->tfhd) {
		e = WriteAtom((Atom *) ptr->tfhd, bs);
		if (e) return e;
	}
	return WriteAtomList(s, ptr->TrackRuns, bs);
}

M4Err traf_Size(Atom *s)
{
	M4Err e;
	TrackFragmentAtom *ptr = (TrackFragmentAtom *)s;

	e = Atom_Size(s);
	if (e) return e;
	if (ptr->tfhd) {
		e = SizeAtom((Atom *) ptr->tfhd);
		if (e) return e;
		ptr->size += ptr->tfhd->size;
	}
	return SizeAtomList(s, ptr->TrackRuns);
}



#endif //M4_READ_ONLY

#endif 

void trak_del(Atom *s)
{
	TrackAtom *ptr = (TrackAtom *) s;
	if (ptr == NULL) return;

	if (ptr->Header) DelAtom((Atom *)ptr->Header);
	if (ptr->udta) DelAtom((Atom *)ptr->udta);
	if (ptr->References) DelAtom((Atom *)ptr->References);
	if (ptr->EditAtom) DelAtom((Atom *)ptr->EditAtom);
	DeleteAtomList(ptr->atomList);
	if (ptr->name) free(ptr->name); 
	free(ptr);
}


M4Err trak_AddAtom(TrackAtom *ptr, Atom *a)
{
	if (!a) return M4OK;
	switch(a->type) {
	case TrackHeaderAtomType:
		if (ptr->Header) return M4InvalidAtom;
		ptr->Header = (TrackHeaderAtom *)a;
		return M4OK;
	//not added to the atomList for edition purposes
	case EditAtomType:
		if (ptr->EditAtom) return M4InvalidAtom;
		ptr->EditAtom = (EditAtom *)a;
		return M4OK;
	case UserDataAtomType:
		if (ptr->udta) return M4InvalidAtom;
		ptr->udta = (UserDataAtom *)a;
		return M4OK;

	//not added to the atomList for edition purposes
	case TrackReferenceAtomType:
		if (ptr->References) return M4InvalidAtom;
		ptr->References = (TrackReferenceAtom *)a;
		return M4OK;
	case MediaAtomType:
		if (ptr->Media) return M4InvalidAtom;
		ptr->Media = (MediaAtom *)a;
		((MediaAtom *)a)->mediaTrack = ptr;
		break;
	}
	return ChainAddEntry(ptr->atomList, a);
}

void M4_CheckUnknownDescription(TrackAtom *trak)
{
	BitStream *bs;
	GenericVisualSampleEntryAtom *genv;
	GenericAudioSampleEntryAtom *gena;
	GenericMediaSampleEntryAtom *genm;
	u32 i;
	u64 read;

	for (i=0; i<ChainGetCount(trak->Media->information->sampleTable->SampleDescription->atomList); i++) {
		UnknownAtom *a = ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, i);
		switch (a->type) {
		case MPEGSampleEntryAtomType:
		case MPEGAudioSampleEntryAtomType:
		case MPEGVisualSampleEntryAtomType:
		case AMRSampleEntryAtomType:
		case WB_AMRSampleEntryAtomType:
		case H263SampleEntryAtomType:
		case GenericHintSampleEntryAtomType:
		case RTPHintSampleEntryAtomType:
		case AVCSampleEntryAtomType:
		case TextSampleEntryAtomType:
			continue;
		default:
			break;
		}
		/*only process visual or audio*/
		switch (trak->Media->handler->handlerType) {
		case M4_VisualMediaType:
			/*remove entry*/
			ChainDeleteEntry(trak->Media->information->sampleTable->SampleDescription->atomList, i);
			genv = (GenericVisualSampleEntryAtom *) CreateAtom(GenericVisualSampleEntryAtomType);
			bs = NewBitStream(a->data, a->size, BS_READ);
			read = 0;
			ReadVideoSampleEntry((VisualSampleEntryAtom *) genv, bs, &read);
			genv->data_size = (u32) BS_Available(bs);
			if (genv->data_size) {
				genv->data = malloc(sizeof(char) * genv->data_size);
				BS_ReadData(bs, genv->data, genv->data_size);
			}
			DeleteBitStream(bs);
			genv->size = a->size;
			genv->EntryType = a->type;
			DelAtom((Atom *)a);
			ChainInsertEntry(trak->Media->information->sampleTable->SampleDescription->atomList, genv, i);
			break;
		case M4_AudioMediaType:
			/*remove entry*/
			ChainDeleteEntry(trak->Media->information->sampleTable->SampleDescription->atomList, i);
			gena = (GenericAudioSampleEntryAtom *) CreateAtom(GenericAudioSampleEntryAtomType);
			bs = NewBitStream(a->data, a->size, BS_READ);
			read = 0;
			ReadAudioSampleEntry((AudioSampleEntryAtom *) gena, bs, &read);
			gena->data_size = (u32) BS_Available(bs);
			if (gena->data_size) {
				gena->data = malloc(sizeof(char) * gena->data_size);
				BS_ReadData(bs, gena->data, gena->data_size);
			}
			DeleteBitStream(bs);
			gena->size = a->size;
			gena->EntryType = a->type;
			DelAtom((Atom *)a);
			ChainInsertEntry(trak->Media->information->sampleTable->SampleDescription->atomList, gena, i);
			break;

		default:
			/*remove entry*/
			ChainDeleteEntry(trak->Media->information->sampleTable->SampleDescription->atomList, i);
			genm = (GenericMediaSampleEntryAtom *) CreateAtom(GenericMediaSampleEntryAtomType);
			bs = NewBitStream(a->data, a->size, BS_READ);
			read = 0;
			BS_ReadData(bs, genm->reserved, 6);
			genm->dataReferenceIndex = BS_ReadInt(bs, 16);
			genm->data_size = (u32) BS_Available(bs);
			if (genm->data_size) {
				genm->data = malloc(sizeof(char) * genm->data_size);
				BS_ReadData(bs, genm->data, genm->data_size);
			}
			DeleteBitStream(bs);
			genm->size = a->size;
			genm->EntryType = a->type;
			DelAtom((Atom *)a);
			ChainInsertEntry(trak->Media->information->sampleTable->SampleDescription->atomList, genm, i);
			break;
		}

	}
}

M4Err trak_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u64 sr;
	Atom *a;
	TrackAtom *ptr = (TrackAtom *)s;
	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		e = trak_AddAtom(ptr, a);
		if (e) return e;
		*read += a->size;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	M4_CheckUnknownDescription(ptr);
	return M4OK;
}

Atom *trak_New()
{
	TrackAtom *tmp = (TrackAtom *) malloc(sizeof(TrackAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(TrackAtom));
	tmp->atomList = NewChain();
	if (!tmp->atomList) {
		free(tmp);
		return NULL;
	}
	tmp->type = TrackAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err trak_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TrackAtom *ptr = (TrackAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;

	//Header first
	if (ptr->Header) {
		e = WriteAtom((Atom *) ptr->Header, bs);
		if (e) return e;
	}
	//then references
	if (ptr->References) {
		e = WriteAtom((Atom *) ptr->References, bs);
		if (e) return e;
	}
	//then EditAtom
	if (ptr->EditAtom) {
		e = WriteAtom((Atom *) ptr->EditAtom, bs);
		if (e) return e;
	}
	//then the rest
	e = WriteAtomList(s, ptr->atomList, bs);
	if (e) return e;

	//then udta
	if (ptr->udta) {
		e = WriteAtom((Atom *) ptr->udta, bs);
		if (e) return e;
	}
	return M4OK;
}

M4Err trak_Size(Atom *s)
{
	M4Err e;
	TrackAtom *ptr = (TrackAtom *)s;
	
	e = Atom_Size(s);
	if (e) return e;

	if (ptr->Header) {
		e = SizeAtom((Atom *) ptr->Header);
		if (e) return e;
		ptr->size += ptr->Header->size;
	}
	if (ptr->udta) {
		e = SizeAtom((Atom *) ptr->udta);
		if (e) return e;
		ptr->size += ptr->udta->size;
	}
	if (ptr->References) {
		e = SizeAtom((Atom *) ptr->References);
		if (e) return e;
		ptr->size += ptr->References->size;
	}
	if (ptr->EditAtom) {
		e = SizeAtom((Atom *) ptr->EditAtom);
		if (e) return e;
		ptr->size += ptr->EditAtom->size;
	}
	return SizeAtomList(s, ptr->atomList);
}

#endif //M4_READ_ONLY


void tref_del(Atom *s)
{
	TrackReferenceAtom *ptr = (TrackReferenceAtom *)s;
	if (ptr == NULL) return;
	DeleteAtomList(ptr->atomList);
	free(ptr);
}


M4Err tref_AddAtom(TrackReferenceAtom *ptr, Atom *a)
{
	if (!a) return M4OK;
	if (ptr == NULL) return M4BadParam;
	return ChainAddEntry(ptr->atomList, a);
}

M4Err tref_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	Atom *a;
	u64 sr;
	TrackReferenceAtom *ptr = (TrackReferenceAtom *)s;
	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		e = tref_AddAtom(ptr, a);
		if (e) return e;
		*read += a->size;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *tref_New()
{
	TrackReferenceAtom *tmp = (TrackReferenceAtom *) malloc(sizeof(TrackReferenceAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(TrackReferenceAtom));
	tmp->atomList = NewChain();
	if (!tmp->atomList) {
		free(tmp);
		return NULL;
	}
	tmp->type = TrackReferenceAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err tref_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TrackReferenceAtom *ptr = (TrackReferenceAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	return WriteAtomList(s, ptr->atomList, bs);
}

M4Err tref_Size(Atom *s)
{
	M4Err e;
	TrackReferenceAtom *ptr = (TrackReferenceAtom *)s;
	e = Atom_Size(s);
	if (e) return e;
	return SizeAtomList(s, ptr->atomList);
}

#endif //M4_READ_ONLY

void reftype_del(Atom *s)
{
	TrackReferenceTypeAtom *ptr = (TrackReferenceTypeAtom *)s;
	if (!ptr) return;
	if (ptr->trackIDs) free(ptr->trackIDs);
	free(ptr);
}


M4Err reftype_Read(Atom *s, BitStream *bs, u64 *read)
{
	u32 bytesToRead;
	u32 i;
	TrackReferenceTypeAtom *ptr = (TrackReferenceTypeAtom *)s;
	if (ptr == NULL) return M4BadParam;

	bytesToRead = (u32) (ptr->size - *read);
	if (bytesToRead < 0) return M4InvalidAtom;
	//empty TREF
	if (!bytesToRead) return M4OK;

	ptr->trackIDCount = (u32) (ptr->size - *read) / sizeof(u32);
	ptr->trackIDs = (u32 *) malloc(ptr->trackIDCount * sizeof(u32));
	if (!ptr->trackIDs) return M4OutOfMem;
	
	for (i = 0; i < ptr->trackIDCount; i++) {
		ptr->trackIDs[i] = BS_ReadInt(bs, 32);
		*read += 4;
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *reftype_New()
{
	TrackReferenceTypeAtom *tmp = (TrackReferenceTypeAtom *) malloc(sizeof(TrackReferenceTypeAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(TrackReferenceTypeAtom));
	return (Atom *)tmp;
}


M4Err reftype_AddRefTrack(TrackReferenceTypeAtom *ref, u32 trackID, u16 *outRefIndex)
{
	u32 i;
	if (!ref || !trackID) return M4BadParam;

	if (outRefIndex) *outRefIndex = 0;
	//don't add a dep if already here !!
	for (i = 0; i < ref->trackIDCount; i++) {
		if (ref->trackIDs[i] == trackID) {
			if (outRefIndex) *outRefIndex = i+1;
			return M4OK;
		}
	}

	ref->trackIDs = (u32 *) realloc(ref->trackIDs, (ref->trackIDCount + 1) * sizeof(u32) );
	if (!ref->trackIDs) return M4OutOfMem;
	ref->trackIDs[ref->trackIDCount] = trackID;
	ref->trackIDCount++;
	if (outRefIndex) *outRefIndex = ref->trackIDCount;
	return M4OK;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err reftype_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	TrackReferenceTypeAtom *ptr = (TrackReferenceTypeAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	for (i = 0; i < ptr->trackIDCount; i++) {
		BS_WriteInt(bs, ptr->trackIDs[i], 32);
	}
	return M4OK;
}


M4Err reftype_Size(Atom *s)
{
	M4Err e;
	TrackReferenceTypeAtom *ptr = (TrackReferenceTypeAtom *)s;
	e = Atom_Size(s);
	if (e) return e;
	ptr->size += (ptr->trackIDCount * sizeof(u32));
	return M4OK;
}

#endif //M4_READ_ONLY



#ifndef	M4_ISO_NO_FRAGMENTS

void trex_del(Atom *s)
{
	TrackExtendsAtom *ptr = (TrackExtendsAtom *)s;
	if (ptr == NULL) return;
	free(ptr);
}


M4Err trex_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	TrackExtendsAtom *ptr = (TrackExtendsAtom *)s;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;

	ptr->trackID = BS_ReadInt(bs, 32);
	ptr->def_sample_desc_index = BS_ReadInt(bs, 32);
	ptr->def_sample_duration = BS_ReadInt(bs, 32);
	ptr->def_sample_size = BS_ReadInt(bs, 32);
	ptr->def_sample_flags = BS_ReadInt(bs, 32);
	
	*read += 20;

	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *trex_New()
{
	TrackExtendsAtom *tmp = (TrackExtendsAtom *) malloc(sizeof(TrackExtendsAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(TrackExtendsAtom));
	tmp->type = TrackExtendsAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY


M4Err trex_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TrackExtendsAtom *ptr = (TrackExtendsAtom *) s;
	if (!s) return M4BadParam;
	e = FullAtom_Write(s, bs);
	if (e) return e;

	BS_WriteInt(bs, ptr->trackID, 32);
	BS_WriteInt(bs, ptr->def_sample_desc_index, 32);
	BS_WriteInt(bs, ptr->def_sample_duration, 32);
	BS_WriteInt(bs, ptr->def_sample_size, 32);
	BS_WriteInt(bs, ptr->def_sample_flags, 32);
	return M4OK;
}

M4Err trex_Size(Atom *s)
{
	M4Err e;
	TrackExtendsAtom *ptr = (TrackExtendsAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 20;
	return M4OK;
}



#endif //M4_READ_ONLY

#endif


#ifndef	M4_ISO_NO_FRAGMENTS

void trun_del(Atom *s)
{
	TrunEntry *p;
	TrackFragmentRunAtom *ptr = (TrackFragmentRunAtom *)s;
	if (ptr == NULL) return;
	
	while (ChainGetCount(ptr->entries)) {
		p = ChainGetEntry(ptr->entries, 0);
		ChainDeleteEntry(ptr->entries, 0);
		free(p);
	}
	DeleteChain(ptr->entries);
	if (ptr->cache) DeleteBitStream(ptr->cache);
	free(ptr);
}

M4Err trun_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u32 i;
	TrunEntry *p;
	TrackFragmentRunAtom *ptr = (TrackFragmentRunAtom *)s;
	
	e = FullAtom_Read(s, bs, read);
	if (e) return e;

	//check this is a good file
	if ((ptr->flags & TR_FIRST_FLAG) && (ptr->flags & TR_FLAGS)) 
		return M4InvalidMP4File;

	ptr->sample_count = BS_ReadInt(bs, 32);
	*read += 4;

	//The rest depends on the flags
	if (ptr->flags & TR_DATA_OFFSET) {
		ptr->data_offset = BS_ReadInt(bs, 32);
		*read += 4;
	}
	if (ptr->flags & TR_FIRST_FLAG) {
		ptr->first_sample_flags = BS_ReadInt(bs, 32);
		*read += 4;
	}

	//read each entry (even though nothing may be written)
	for (i=0; i<ptr->sample_count; i++) {
		p = (TrunEntry *) malloc(sizeof(TrunEntry));
		memset(p, 0, sizeof(TrunEntry));

		if (ptr->flags & TR_DURATION) {
			p->Duration = BS_ReadInt(bs, 32);
			*read += 4;
		}
		if (ptr->flags & TR_SIZE) {
			p->size = BS_ReadInt(bs, 32);
			*read += 4;
		}
		//SHOULDN'T BE USED IF TR_FIRST_FLAG IS DEFINED
		if (ptr->flags & TR_FLAGS) {
			p->flags = BS_ReadInt(bs, 32);
			*read += 4;
		}
		if (ptr->flags & TR_CTS_OFFSET) {
			p->CTS_Offset = BS_ReadInt(bs, 32);
			*read += 4;
		}
		ChainAddEntry(ptr->entries, p);
		//make sure about that...
		if (*read > ptr->size) break;
	}	
	if (*read != ptr->size) return M4ReadAtomFailed;

	if (ChainGetCount(ptr->entries) != ptr->sample_count) return M4InvalidAtom;

	return M4OK;
}

Atom *trun_New()
{
	TrackFragmentRunAtom *tmp = (TrackFragmentRunAtom *) malloc(sizeof(TrackFragmentRunAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(TrackFragmentRunAtom));
	tmp->type = TrackFragmentRunAtomType;
	tmp->entries = NewChain();
	//NO FLAGS SET BY DEFAULT
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY


M4Err trun_Write(Atom *s, BitStream *bs)
{
	TrunEntry *p;
	M4Err e;
	u32 i, count;
	TrackFragmentRunAtom *ptr = (TrackFragmentRunAtom *) s;
	if (!s) return M4BadParam;

	e = FullAtom_Write(s, bs);
	if (e) return e;

	BS_WriteInt(bs, ptr->sample_count, 32);

	//The rest depends on the flags
	if (ptr->flags & TR_DATA_OFFSET) {
		BS_WriteInt(bs, ptr->data_offset, 32);
	}
	if (ptr->flags & TR_FIRST_FLAG) {
		BS_WriteInt(bs, ptr->first_sample_flags, 32);
	}

	//if nothing to do, this will be skipped automatically
	count = ChainGetCount(ptr->entries);
	for (i=0; i<count; i++) {
		p = ChainGetEntry(ptr->entries, i);

		if (ptr->flags & TR_DURATION) {
			BS_WriteInt(bs, p->Duration, 32);
		}
		if (ptr->flags & TR_SIZE) {
			BS_WriteInt(bs, p->size, 32);
		}
		//SHOULDN'T BE USED IF TR_FIRST_FLAG IS DEFINED
		if (ptr->flags & TR_FLAGS) {
			BS_WriteInt(bs, p->flags, 32);
		}
		if (ptr->flags & TR_CTS_OFFSET) {
			BS_WriteInt(bs, p->CTS_Offset, 32);
		}
	}	
	return M4OK;
}

M4Err trun_Size(Atom *s)
{
	M4Err e;
	u32 i, count;
	TrunEntry *p;
	TrackFragmentRunAtom *ptr = (TrackFragmentRunAtom *)s;

	e = FullAtom_Size(s);
	if (e) return e;
	
	ptr->size += 4;
	//The rest depends on the flags
	if (ptr->flags & TR_DATA_OFFSET) ptr->size += 4;
	if (ptr->flags & TR_FIRST_FLAG) ptr->size += 4;

	//if nothing to do, this will be skipped automatically
	count = ChainGetCount(ptr->entries);
	for (i=0; i<count; i++) {
		p = ChainGetEntry(ptr->entries, i);
		if (ptr->flags & TR_DURATION) ptr->size += 4;
		if (ptr->flags & TR_SIZE) ptr->size += 4;
		//SHOULDN'T BE USED IF TR_FIRST_FLAG IS DEFINED
		if (ptr->flags & TR_FLAGS) ptr->size += 4;
		if (ptr->flags & TR_CTS_OFFSET) ptr->size += 4;
	}
	
	return M4OK;
}



#endif //M4_READ_ONLY

#endif 

void tsro_del(Atom *s)
{
	TimeOffHintEntry *tsro = (TimeOffHintEntry *)s;
	free(tsro);
}

M4Err tsro_Read(Atom *s, BitStream *bs, u64 *read)
{
	TimeOffHintEntry *ptr = (TimeOffHintEntry *)s;
	if (ptr == NULL) return M4BadParam;

	ptr->TimeOffset = BS_ReadInt(bs, 32);
	*read += 4;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *tsro_New()
{
	TimeOffHintEntry *tmp = (TimeOffHintEntry *) malloc(sizeof(TimeOffHintEntry));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(TimeOffHintEntry));
	tmp->type = tsroHintEntryType;
	return (Atom *)tmp;
}


#ifndef M4_READ_ONLY
M4Err tsro_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	TimeOffHintEntry *ptr = (TimeOffHintEntry *)s;
	if (ptr == NULL) return M4BadParam;

	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->TimeOffset, 32);
	return M4OK;
}

M4Err tsro_Size(Atom *s)
{
	M4Err e;
	e = Atom_Size(s);
	if (e) return e;
	s->size += 4;
	return M4OK;
}
#endif


void udta_del(Atom *s)
{
	u32 i;
	UserDataMap *map;
	UserDataAtom *ptr = (UserDataAtom *)s;
	if (ptr == NULL) return;
	for (i = 0; i < ChainGetCount(ptr->recordList); i++) {
		map = (UserDataMap*)ChainGetEntry(ptr->recordList, i);
		if (map) {
			DeleteAtomList(map->atomList);
			free(map);
		}
	}
	DeleteChain(ptr->recordList);
	if (ptr->voidAtom) DelAtom(ptr->voidAtom);
	free(ptr);
}

UserDataMap *udta_getEntry(UserDataAtom *ptr, u32 atomType, bin128 UUID)
{
	u32 i;
	UserDataMap *map;
	for (i = 0; i < ChainGetCount(ptr->recordList); i++) {
		map = (UserDataMap *) ChainGetEntry(ptr->recordList, i);
		if (map->atomType == atomType) {
			if (atomType != ExtendedAtomType) return map;
			if (!memcmp(map->uuid, UUID, 16)) return map;
		}
	}
	return NULL;
}

M4Err udta_AddAtom(UserDataAtom *ptr, Atom *a)
{
	M4Err e;
	UserDataMap *map;
	if (!ptr) return M4BadParam;
	if (!a) return M4OK;

	if (a->type == VoidAtomType) {
		if (ptr->voidAtom) return M4BadParam;
		ptr->voidAtom = a;
		return M4OK;
	}

	map = udta_getEntry(ptr, a->type, a->uuid);

	if (map == NULL) {
		map = (UserDataMap *) malloc(sizeof(UserDataMap));
		if (map == NULL) return M4OutOfMem;
		memset(map, 0, sizeof(UserDataMap));
		
		map->atomType = a->type;
		if (a->type == ExtendedAtomType) memcpy(map->uuid, a->uuid, 16);
		map->atomList = NewChain();
		if (!map->atomList) {
			free(map);
			return M4OutOfMem;
		}
		e = ChainAddEntry(ptr->recordList, map);
		if (e) return e;
	}
	return ChainAddEntry(map->atomList, a);
}


M4Err udta_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u64 start, sr;
	u32 sub_type;
	Atom *a;
	UserDataAtom *ptr = (UserDataAtom *)s;
	while (*read < ptr->size) {
		start = BS_GetPosition(bs);
		/*if no udta type coded, break*/
		sub_type = BS_PeekBits(bs, 32, 0);
		if (sub_type) {
			e = ParseAtom(&a, bs, &sr);
			//udta is of no importance: if we have a pb, skip till end of atom
			if (e) {
				if (a) DelAtom(a);
				start = BS_GetPosition(bs) - start;
				while (*read + start < ptr->size) {
					BS_ReadInt(bs, 8);
					(*read) ++;
				}
				return M4OK;
			}
			e = udta_AddAtom(ptr, a);
			if (e) return e;
			*read += a->size;
		} else {
			BS_ReadInt(bs, 32);
			*read += 4;
		}
	}
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *udta_New()
{
	UserDataAtom *tmp = (UserDataAtom *) malloc(sizeof(UserDataAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(UserDataAtom));
	tmp->recordList = NewChain();
	if (!tmp->recordList) {
		free(tmp);
		return NULL;
	}
	tmp->type = UserDataAtomType;
	return (Atom *)tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err udta_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	u32 i;
	UserDataMap *map;
	UserDataAtom *ptr = (UserDataAtom *)s;

	e = Atom_Write(s, bs);
	if (e) return e;
	for (i = 0; i < ChainGetCount(ptr->recordList); i++) {
		map = (UserDataMap*)ChainGetEntry(ptr->recordList, i);
		//warning: here we are not passing the actual "parent" of the list
		//but the UDTA atom. The parent itself is not an atom, we don't care about it
		e = WriteAtomList(s, map->atomList, bs);
		if (e) return e;
    }
	if (ptr->voidAtom) {
		e = WriteAtom(ptr->voidAtom, bs);
		if (e) return e;
	}
	return M4OK;
}

M4Err udta_Size(Atom *s)
{
	M4Err e;
	u32 i;
	UserDataMap *map;
	UserDataAtom *ptr = (UserDataAtom *)s;
	
	e = Atom_Size(s);
	if (e) return e;
	for (i = 0; i < ChainGetCount(ptr->recordList); i++) {
		map = (UserDataMap*)ChainGetEntry(ptr->recordList, i);
		//warning: here we are not passing the actual "parent" of the list
		//but the UDTA atom. The parent itself is not an atom, we don't care about it
		e = SizeAtomList(s, map->atomList);
		if (e) return e;
	}
	if (ptr->voidAtom) {
		ptr->size += 4;
	}
	return M4OK;
}

#endif //M4_READ_ONLY


void vmhd_del(Atom *s)
{
	VideoMediaHeaderAtom *ptr = (VideoMediaHeaderAtom *)s;
	if (ptr == NULL) return;
	free(ptr);
}


M4Err vmhd_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	VideoMediaHeaderAtom *ptr = (VideoMediaHeaderAtom *)s;
	if (ptr == NULL) return M4BadParam;

	e = FullAtom_Read(s, bs, read);
	if (e) return e;
	ptr->reserved = BS_ReadLongInt(bs, 64);
	*read += 8;
	if (*read != ptr->size) return M4ReadAtomFailed;
	return M4OK;
}

Atom *vmhd_New()
{
	VideoMediaHeaderAtom *tmp = (VideoMediaHeaderAtom *) malloc(sizeof(VideoMediaHeaderAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(VideoMediaHeaderAtom));
	InitFullAtom((Atom *)tmp);
	tmp->flags = 1;
	tmp->type = VideoMediaHeaderAtomType;
	return (Atom *)tmp;
}


//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err vmhd_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	VideoMediaHeaderAtom *ptr = (VideoMediaHeaderAtom *)s;
	
	e = FullAtom_Write(s, bs);
	if (e) return e;
	BS_WriteLongInt(bs, ptr->reserved, 64);
	return M4OK;
}

M4Err vmhd_Size(Atom *s)
{
	M4Err e;
	VideoMediaHeaderAtom *ptr = (VideoMediaHeaderAtom *)s;
	e = FullAtom_Size(s);
	if (e) return e;
	ptr->size += 8;
	return M4OK;
}

#endif //M4_READ_ONLY


void void_del(Atom *s)
{
	free(s);
}


M4Err void_Read(Atom *s, BitStream *bs, u64 *read)
{
	if (*read != 4) return M4ReadAtomFailed;
	return M4OK;
}

Atom *void_New()
{
	Atom *tmp = (Atom *) malloc(sizeof(Atom));
	if (!tmp) return NULL;
	tmp->type = VoidAtomType;
	return tmp;
}

//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err void_Write(Atom *s, BitStream *bs)
{
	BS_WriteInt(bs, 0, 32);
	return M4OK;
}

M4Err void_Size(Atom *s)
{
	s->size = 4;
	return M4OK;
}

#endif //M4_READ_ONLY
