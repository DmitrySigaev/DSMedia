/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / IETF RTP/RTSP/SDP sub-project
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

#include <gpac/intern/m4_ietf_dev.h>

//check for 30 packets
#define SN_CHECK_OFFSET		0x0A


#define REORDER_DEBUG	0

RTPReorder *NewPckReoreder(u32 MaxCount, u32 MaxDelay)
{
	RTPReorder *tmp;
	
	if (MaxCount <= 1 || !MaxDelay) return NULL;

	tmp = malloc(sizeof(RTPReorder));
	memset(tmp, 0, sizeof(RTPReorder));
	tmp->MaxCount = MaxCount;
	tmp->MaxDelay = MaxDelay;
	return tmp;
}

static void DelItem(PRO_item *it)
{
	if (it) {
		if (it->next) DelItem(it->next);
		free(it->pck);
		free(it);
	}
}


void PO_Delete(RTPReorder *po)
{
	if (po->in) DelItem(po->in);
	free(po);
}

void PO_Reset(RTPReorder *po)
{
	if (!po) return;

	if (po->in) DelItem(po->in);
	po->head_seqnum = 0;
	po->Count = 0;
	po->IsInit = 0;
	po->in = NULL;
}

M4Err PO_AddPacket(RTPReorder *po, void *pck, u32 pck_size, u32 pck_seqnum)
{
	PRO_item *it, *cur;
	u32 bounds;

	if (!po) return M4BadParam;

	it = malloc(sizeof(PRO_item));
	it->pck_seq_num = pck_seqnum;
	it->next = NULL;
	it->size = pck_size;
	it->pck = malloc(pck_size);
	memcpy(it->pck, pck, pck_size);
	/*reset timeout*/
	po->LastTime = 0;

	//no input, this packet will be the input
	if (!po->in) {
		//the seq num was not initialized
		if (!po->head_seqnum) {
			po->head_seqnum = pck_seqnum;
		} else if (!po->IsInit) {
			//this is not in our current range for init
			if (ABSDIFF(po->head_seqnum, pck_seqnum) > SN_CHECK_OFFSET) goto discard;
			po->IsInit = 1;
		}

		po->in = it;
		po->Count += 1;
		return M4OK;
	}

	//this is 16 bitr seq num, as we work with RTP only for now
	bounds = 0;
	if ( (po->head_seqnum >= 0xf000 ) || (po->head_seqnum <= 0x1000) ) bounds = 0x2000;

	//first check the head of the list
	//same seq num, we drop
	if (po->in->pck_seq_num == pck_seqnum) goto discard;

	if ( ( (u16) (pck_seqnum + bounds) <= (u16) (po->in->pck_seq_num + bounds) )
		&& ( (u16) (pck_seqnum + bounds) >= (u16) (po->head_seqnum + bounds)) ) {

		it->next = po->in;
		po->in = it;
		po->Count += 1;
#if REORDER_DEBUG
		fprintf(stdout, "INSERTING AT HEAD\n");
#endif
		return M4OK;
	}

	//no, insert at the right place
	cur = po->in;

	while (1) {
		//same seq num, we drop
		if (cur->pck_seq_num == pck_seqnum) goto discard;

		//end of list
		if (!cur->next) {
			cur->next = it;
			po->Count += 1;
#if REORDER_DEBUG
			if (cur->pck_seq_num +1 != it->pck_seq_num) 
				fprintf(stdout, "!! Got %d expected %d\n", cur->pck_seq_num+1, it->pck_seq_num);
#endif
			//done
			return M4OK;
		}

		//are we in the bounds ??
		if ( ( (u16) (cur->pck_seq_num + bounds) < (u16) (pck_seqnum + bounds) )
			&& ( (u16) (pck_seqnum + bounds) < (u16) (cur->next->pck_seq_num + bounds)) ) {

			//insert
			it->next = cur->next;
			cur->next = it;
			po->Count += 1;
#if REORDER_DEBUG
			fprintf(stdout, "INSERTING\n");
#endif
			//done
			return M4OK;
		}
		cur = cur->next;
	}
	

discard:
	free(it->pck);
	free(it);
#if REORDER_DEBUG
	fprintf(stdout, "DROP");
#endif
	return M4OK;
}

//retrieve the first available packet. Note that the behavior will be undefined if the first
//ever recieved packet if its SeqNum was unknown
//the BUFFER is yours, you must delete it
void *PO_GetPacket(RTPReorder *po, u32 *pck_size)
{
	PRO_item *t;
	u32 bounds;
	void *ret;

	if (!po || !pck_size) return NULL;

	*pck_size = 0;

	//empty queue
	if (!po->in) return NULL;

	//check we have recieved the first packet
	if ( po->head_seqnum && po->MaxCount
		&& (po->MaxCount > po->Count) 
		&& (po->in->pck_seq_num != po->head_seqnum)) 
		return NULL;

	//no entry
	if (!po->in->next) goto check_timeout;

	bounds = 0;
	if ( (po->head_seqnum >= 0xf000 ) || (po->head_seqnum <= 0x1000) ) bounds = 0x2000;

	//release the output if SN in order or maxCount reached
	if (( (u16) (po->in->pck_seq_num + bounds + 1) == (u16) (po->in->next->pck_seq_num + bounds)) 
		|| (po->MaxCount && po->Count == po->MaxCount) ) {

#if REORDER_DEBUG
		if (po->in->pck_seq_num + 1 != po->in->next->pck_seq_num) 
			fprintf(stdout, "Fetched %d expected %d\n", po->in->pck_seq_num + 1, po->in->next->pck_seq_num);
#endif
		goto send_it;
	}
	//update timing
	else {
check_timeout:
		if (!po->LastTime) {
			po->LastTime = M4_GetSysClock();
#if REORDER_DEBUG
			fprintf(stdout, "starting reorder timeout\n");
#endif
			return NULL;
		}
		//if exceeding the delay send the head
		if (M4_GetSysClock() - po->LastTime >= po->MaxDelay) {
#if REORDER_DEBUG
			fprintf(stdout, "RTP Forcing output after %d ms wait (max allowed %d)\n", M4_GetSysClock() - po->LastTime, po->MaxDelay);
#endif
			goto send_it;
		}
	}
	return NULL;


send_it:
	*pck_size = po->in->size;
	t = po->in;
	po->in = po->in->next;
	//no other output. reset the head seqnum
	po->head_seqnum = po->in ? po->in->pck_seq_num : 0;
	po->Count -= 1;
	//release the item
	ret = t->pck;
	free(t);
	return ret;
}
