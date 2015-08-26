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


//should never be called!!!
void  Atom_Delete(Atom *ptr)
{
	free(ptr);
}

//should never be called!!!
Atom *Atom_New()
{
	Atom *tmp;
	tmp = (Atom *) malloc(sizeof(Atom));
	return tmp;
}

//should never be called!!!
void  FullAtom_Delete(Atom *a)
{
	FullAtom *ptr = (FullAtom *)a;
	free(ptr);
}

//should never be called!!!
Atom *FullAtom_New()
{
	FullAtom *tmp;
	tmp = (FullAtom *) malloc(sizeof(FullAtom));
	return (Atom *)tmp;
}

//Add this funct to handle incomplete files...
//bytesExpected is 0 most of the time. If the file is incomplete, bytesExpected
//is the number of bytes missing to parse the atom...
M4Err ParseRootAtom(Atom **outAtom, BitStream *bs, u64 *bytesExpected)
{
	M4Err ret;
	u64 read;
	//first make sure we can at least get the atom size and type...
	//18 = size (int32) + type (int32)
	if (BS_Available(bs) < 8) {
		*bytesExpected = 8;
		return M4UncompleteFile;
	}
	ret = ParseAtom(outAtom, bs, &read);
	if (ret == M4UncompleteFile) {
		*bytesExpected = (*outAtom)->size - read - BS_Available(bs);
		//rewind our BitStream for next parsing
		BS_Rewind(bs, read);
		DelAtom(*outAtom);
		*outAtom = NULL;
	}
	return ret;
}


M4Err ParseAtom(Atom **outAtom, BitStream *bs, u64 *read)
{
	u32 type;
	u64 size;
	u8 uuid[16];
	M4Err e;
	Atom *newAtom;
	char name[5];
	name[4] = 0;
	e = M4OK;
	if ((bs == NULL) || (outAtom == NULL) ) return M4BadParam;
	*outAtom = NULL;

	//read atom header
	size = (u64) BS_ReadInt(bs, 32);
	*read = 4;
	/*fix for some atoms found in some old hinted files*/
	if ((size >= 2) && (size <= 4)) {
		size = 4;
		type = VoidAtomType;
	} else {
		type = BS_ReadInt(bs, 32);
		*read += 4;
		/*no size means till end of file - EXCEPT FOR some old QuickTime atoms...*/
		if (type == totlAtomType)
			size = 12;
		if (!size) 
			size = BS_Available(bs) + 4;
	}
	/*handle uuid*/
	memset(uuid, 0, 16);
	if (type == ExtendedAtomType ) {
		BS_ReadData(bs, (unsigned char *) uuid, 16);
		*read += 16;
	}
	
	//handle large atom
	if (size == 1) {
		size = BS_ReadInt(bs, 64);
		*read += 8;
	}

	if ( size - *read < 0 ) return M4InvalidMP4File;
	//OK, create the atom based on the type
	newAtom = CreateAtom(type);
	if (! newAtom) return M4OutOfMem;

	//OK, init and read this atom
	newAtom->size = size;
	memcpy(newAtom->uuid, uuid, 16);
	if (!newAtom->type) newAtom->type = type; 

	if (newAtom->size - *read > BS_Available(bs) ) {
		*outAtom = newAtom;
		return M4UncompleteFile;
	}
	//we need a special reading for these atoms...
	if (newAtom->type == DegradationPriorityAtomType) {
		*outAtom = newAtom;
		return M4OK;
	}

#if 0
	/*perf test*/
	switch (type) {
	case SampleTableAtomType:
	case TimeToSampleAtomType:
	case SampleToChunkAtomType:
	case CompactSampleSizeAtomType:
	case SampleSizeAtomType:
	case ChunkOffsetAtomType:
	case ChunkLargeOffsetAtomType:
		now = M4_GetSysClock();
		e = ReadAtom(newAtom, bs);
		M4_MP4TypeToString(newAtom->type, name);
		printf("Read atom %s in %d ms\n", name, M4_GetSysClock() - now);
		break;
	default:
		e = ReadAtom(newAtom, bs, read);
		break;
	}
#else
	e = ReadAtom(newAtom, bs, read);	
#endif

	//DON'T DELETE THE MDAT ATOM IF UNCOMPLETE...
	if (e < 0 && e != M4UncompleteFile) {
		DelAtom(newAtom);
		*outAtom = NULL;
		return e;
	}
	*outAtom = newAtom;
	return e;
}



M4Err FullAtom_Read(Atom *ptr, BitStream *bs, u64 *read)
{
	FullAtom *self;
	self = (FullAtom *) ptr;
	self->version = BS_ReadInt(bs, 8);
	self->flags = BS_ReadInt(bs, 24);
	*read += 4;
	return M4OK;
}

void InitFullAtom(Atom *a)
{
	FullAtom *ptr = (FullAtom *)a;
	if (! ptr) return;
	ptr->flags = 0;
	ptr->version = 0;
}


void DeleteAtomList(Chain *atomList)
{
	u32 count, i;
	Atom *a;
	if (!atomList) return;
	count = ChainGetCount(atomList); 
	for (i = 0; i < count; i++) {
		a = (Atom *)ChainGetEntry(atomList, i);
		if (a) DelAtom(a);
	}
	DeleteChain(atomList);
}



//from here, for write/edit versions
#ifndef M4_READ_ONLY

M4Err Atom_Size(Atom *ptr)
{
	ptr->size = 8;

	if (ptr->type == ExtendedAtomType) {
		ptr->size += 16;
	}
	//the large size is handled during write, cause at this stage we don't know the size
	return M4OK;
}

M4Err FullAtom_Size(Atom *ptr)
{
	M4Err e;
	e = Atom_Size(ptr);
	if (e) return e;
	ptr->size += 4;
	return M4OK;
}


M4Err Atom_Write(Atom *ptr, BitStream *bs)
{
	if (! bs || !ptr) return M4BadParam;
	if (!ptr->size) return M4InvalidAtom;

	if (ptr->size > 0xFFFFFFFF) {
		BS_WriteInt(bs, 1, 32);
	} else {
		BS_WriteInt(bs, (u32) ptr->size, 32);
	}
	BS_WriteInt(bs, ptr->type, 32);
	if (ptr->type == ExtendedAtomType) BS_WriteData(bs, (unsigned char *)ptr->uuid, 16);
	if (ptr->size > 0xFFFFFFFF) BS_WriteLongInt(bs, ptr->size, 64);
	return M4OK;
}

M4Err FullAtom_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	FullAtom *ptr = (FullAtom *)s;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->version, 8);
	BS_WriteInt(bs, ptr->flags, 24);
	return M4OK;
}


M4Err WriteAtomList(Atom *parent, Chain *list, BitStream *bs)
{
	u32 count, i;
	Atom *a;
	M4Err e;
	if (!list) return M4BadParam;
	count = ChainGetCount(list);
	for (i = 0; i < count; i++) {
		a = (Atom *)ChainGetEntry(list, i);
		if (a) {
			e = WriteAtom(a, bs);
			if (e) return e;
		}
	}
	return M4OK;
}

M4Err SizeAtomList(Atom *parent, Chain *list)
{
	M4Err e;
	u32 count, i;
	Atom *a;
	if (! list) return M4BadParam;

	count = ChainGetCount(list);
	for (i = 0; i < count; i++) {
		a = (Atom *)ChainGetEntry(list, i);
		if (a) {
			e = SizeAtom(a);
			if (e) return e;
			parent->size += a->size;
		}
	}
	return M4OK;
}

#endif //M4_READ_ONLY
