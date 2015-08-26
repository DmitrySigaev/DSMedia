/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / common tools sub-project
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

#include <gpac/m4_tools.h>

/*use linked-list or use array */
#if 0


typedef struct tagIS
{
	struct tagIS *next;
	void *data;
} ItemSlot;

struct tagChain
{
	struct tagIS *head;
	struct tagIS *tail;
	u32 entryCount;
	s32 foundEntryNumber;
	struct tagIS *foundEntry;
};


Chain * NewChain()
{
	Chain *newChain;

	newChain = (Chain *) malloc(sizeof(Chain));
	if (! newChain) return NULL;
	
	newChain->head = newChain->foundEntry = NULL;
	newChain->tail = NULL;
	newChain->foundEntryNumber = -1;
	newChain->entryCount = 0;
	return newChain;
}

void DeleteChain(Chain *ptr)
{
	if (!ptr) return;
	if (ptr->tail) assert(ptr->tail->next==NULL);
	while (ptr->entryCount) {
		ChainDeleteEntry(ptr, 0);
	}
	free(ptr);
}

void ChainReset(Chain *ptr)
{
	if (ptr) 
		while (ptr->entryCount) ChainDeleteEntry(ptr, 0);
}


M4Err ChainAddEntry(Chain *ptr, void* item)
{
	ItemSlot *entry;

    if (! ptr) return M4BadParam;
	entry = (ItemSlot *) malloc(sizeof(ItemSlot));
	if (!entry) return M4OutOfMem;
	if (ptr->tail) assert(ptr->tail->next==NULL);
	entry->data = item;
	entry->next = NULL;

	if (! ptr->head) {
		ptr->head = entry;
		ptr->entryCount = 1;
	} else {
		ptr->entryCount += 1;
		ptr->tail->next = entry;
	}
	ptr->tail = entry;
	ptr->foundEntryNumber = ptr->entryCount - 1;
	ptr->foundEntry = entry;
	return M4OK;
}


u32 ChainGetCount(Chain *ptr)
{
	if (! ptr) return 0;
	if (ptr->tail) assert(ptr->tail->next==NULL);
	return ptr->entryCount;
}

void *ChainGetEntry(Chain *ptr, u32 itemNumber)
{
	ItemSlot *entry;
	u32 i;

	if ((! ptr) || (itemNumber >= ptr->entryCount) ) return NULL;

	if (ptr->tail) assert(ptr->tail->next==NULL);

	if ( itemNumber < (u32) ptr->foundEntryNumber ) {
		ptr->foundEntryNumber = 0;
		ptr->foundEntry = ptr->head;
	}
	entry = ptr->foundEntry;
	for (i = ptr->foundEntryNumber; i < itemNumber; i++ ) {
		/*our list is broken...*/
		if (! entry->next) {
			return NULL;
		}
		entry = entry->next;
	}
	ptr->foundEntryNumber = itemNumber;
	ptr->foundEntry = entry;
	return (void *) entry->data;
}




/*WARNING: itemNumber is from 0 to entryCount - 1*/
M4Err ChainDeleteEntry(Chain *ptr, u32 itemNumber)
{
	ItemSlot *tmp, *tmp2;
	u32 i;

	/* !! if head is null (empty list)*/
	if ( (! ptr) || (! ptr->head) || (ptr->head && !ptr->entryCount) || (itemNumber >= ptr->entryCount) ) 
		return M4BadParam;

	if (ptr->tail) assert(ptr->tail->next==NULL);
	/*we delete the head*/
	if (! itemNumber) {
		tmp = ptr->head;
		ptr->head = ptr->head->next;
		ptr->entryCount --;
		ptr->foundEntry = ptr->head;
		ptr->foundEntryNumber = 0;
		free(tmp);
		/*that was the last entry, reset the tail*/
		if (!ptr->entryCount) {
			ptr->tail = ptr->head = ptr->foundEntry = NULL;
			ptr->foundEntryNumber = -1;
		}
		return M4OK;
	}

	tmp = ptr->head;
	i = 0;
	while (i < itemNumber - 1) {
		tmp = tmp->next;
		i++;
	}
	tmp2 = tmp->next;
	tmp->next = tmp2->next;
	/*if we deleted the last entry, update the tail !!!*/
	if (! tmp->next || (ptr->tail == tmp2) ) {
		ptr->tail = tmp;
		tmp->next = NULL;
	}

	free(tmp2);
	ptr->entryCount --;
	ptr->foundEntry = ptr->head;
	ptr->foundEntryNumber = 0;

	return M4OK;
}


/*WARNING: position is from 0 to entryCount - 1*/
M4Err ChainInsertEntry(Chain *ptr, void *item, u32 position)
{
	u32 i;
	ItemSlot *tmp, *tmp2;

	if (!ptr || !item) return M4BadParam;
	/*if last entry or first of an empty chain...*/
	if (position >= ptr->entryCount) return ChainAddEntry(ptr, item);
	assert(ptr->tail->next==NULL);

	tmp2 = (ItemSlot *) malloc(sizeof(ItemSlot));
	tmp2->data = item;
	tmp2->next = NULL;
	/*special case for the head*/
	if (position == 0) {
		tmp2->next = ptr->head;
		ptr->head = tmp2;
		ptr->entryCount ++;
		ptr->foundEntry = tmp2;
		ptr->foundEntryNumber = 0;
		return M4OK;
	}
	tmp = ptr->head;
	for (i = 1; i < position; i++) {
		tmp = tmp->next;
		if (!tmp)
			break;
	}
	tmp2->next = tmp->next;
	tmp->next = tmp2;
	ptr->entryCount ++;
	ptr->foundEntry = tmp2;
	ptr->foundEntryNumber = i;
	return M4OK;
}

#else

#ifdef _WIN32_WCE
#define CHAIN_STEP_ALLOC	10
#else
#define CHAIN_STEP_ALLOC	50
#endif

struct tagChain
{
	void **slots;
	u32 entryCount;
	u32 allocSize;
};


Chain * NewChain()
{
	Chain *newChain;

	newChain = (Chain *) malloc(sizeof(Chain));
	if (! newChain) return NULL;
	
	newChain->slots = NULL;
	newChain->entryCount = 0;
	newChain->allocSize = 0;
	return newChain;
}

void DeleteChain(Chain *ptr)
{
	if (!ptr) return;
	free(ptr->slots);
	free(ptr);
}

static void realloc_chain(Chain *ptr)
{
	ptr->allocSize += CHAIN_STEP_ALLOC;
	if (ptr->slots) {
		ptr->slots = realloc(ptr->slots, ptr->allocSize*sizeof(void*));
	} else {
		ptr->slots = malloc(sizeof(void*)*ptr->allocSize);
	}
}

M4Err ChainAddEntry(Chain *ptr, void* item)
{
    if (! ptr) return M4BadParam;
	if (ptr->allocSize==ptr->entryCount) realloc_chain(ptr);
	if (!ptr->slots) return M4OutOfMem;

	ptr->slots[ptr->entryCount] = item;
	ptr->entryCount ++;
	return M4OK;
}


u32 ChainGetCount(Chain *ptr)
{
	if (!ptr) return 0;
	return ptr->entryCount;
}

void *ChainGetEntry(Chain *ptr, u32 itemNumber)
{
	if(!ptr || (itemNumber >= ptr->entryCount)) return NULL;
	return ptr->slots[itemNumber];
}


/*WARNING: itemNumber is from 0 to entryCount - 1*/
M4Err ChainDeleteEntry(Chain *ptr, u32 itemNumber)
{
	u32 i;
	if ( !ptr || !ptr->slots || !ptr->entryCount) return M4BadParam;
	i = ptr->entryCount - itemNumber - 1;
	if (i) memmove(&ptr->slots[itemNumber], & ptr->slots[itemNumber +1], sizeof(void *)*i);
	ptr->slots[ptr->entryCount-1] = NULL;
	ptr->entryCount -= 1;
	return M4OK;
}


/*WARNING: position is from 0 to entryCount - 1*/
M4Err ChainInsertEntry(Chain *ptr, void *item, u32 position)
{
	u32 i;
	if (!ptr || !item) return M4BadParam;
	/*if last entry or first of an empty chain...*/
	if (position >= ptr->entryCount) return ChainAddEntry(ptr, item);
	if (ptr->allocSize==ptr->entryCount) realloc_chain(ptr);

	i = ptr->entryCount - position;
	memmove(&ptr->slots[position + 1], &ptr->slots[position], sizeof(void *)*i);
	ptr->entryCount++;
	ptr->slots[position] = item;
	return M4OK;
}

void ChainReset(Chain *ptr)
{
	if (ptr) ptr->entryCount = 0;
}

#endif

s32 ChainFindEntry(Chain *ptr, void *item)
{
	u32 i;
	for (i=0; i<ChainGetCount(ptr); i++) {
		if (ChainGetEntry(ptr, i) == item) return (s32) i;
	}
	return -1;
}

s32 ChainDeleteItem(Chain *ptr, void *item)
{
	s32 i = ChainFindEntry(ptr, item);
	if (i>=0) ChainDeleteEntry(ptr, (u32) i);
	return i;
}

