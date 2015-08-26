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


typedef HANDLE TH_HANDLE ;


/*********************************************************************
						OS-Specific Thread Object
**********************************************************************/
struct _tagM4Thread
{

	u32 status;
	TH_HANDLE threadH;
	u32 stackSize;
	/* the thread procedure */
	u32 (*Run)(void *param);
	void *args;
	/* lock for signal */
	M4Sema *_signal;
};

M4Thread *NewThread()
{
	M4Thread *tmp = malloc(sizeof(M4Thread));
	memset(tmp, 0, sizeof(M4Thread));
	tmp->status = THREAD_STATUS_STOP;
	return tmp;
}

DWORD WINAPI RunThread(void *ptr)
{
	DWORD ret = 0;
	M4Thread *t = (M4Thread *)ptr;

	/* Signal the caller */
	if (! t->_signal) goto exit;

	t->status = THREAD_STATUS_PLAY;
	
	SEM_Notify(t->_signal, 1);
	/* Run our thread */
	ret = t->Run(t->args);

exit:
	t->status = THREAD_STATUS_DEAD;
	t->Run = NULL;
	CloseHandle(t->threadH);
	t->threadH = NULL;
	return ret;
}

M4Err TH_Run(M4Thread *t, u32 (*Run)(void *param), void *param)
{
	DWORD id;
	if (!t || t->Run || t->_signal) return M4BadParam;
	t->Run = Run;
	t->args = param;
	t->_signal = NewSemaphore(1, 0);

	t->threadH = CreateThread(NULL,  t->stackSize, &(RunThread), (void *)t, 0, &id);
	if (t->threadH == NULL) {
		t->status = THREAD_STATUS_DEAD;
		return M4IOErr;
	}

	/*wait for the child function to call us - do NOT return bedfore, otherwise the thread status would
	be unknown*/ 	
	SEM_Wait(t->_signal);
	SEM_Delete(t->_signal);
	t->_signal = NULL;
	return M4OK;
}


/* Stops a thread. If Destroy is not 0, thread is destroyed DANGEROUS as no cleanup */
void Thread_Stop(M4Thread *t, Bool Destroy)
{
	if (TH_GetStatus(t) == THREAD_STATUS_PLAY) {
		if (Destroy) {
			DWORD dw = 1;
			TerminateThread(t->threadH, dw);
			t->threadH = NULL;
		} else {
			WaitForSingleObject(t->threadH, INFINITE);			
		}
	}
	t->status = THREAD_STATUS_DEAD;
}

void TH_Stop(M4Thread *t)
{
	Thread_Stop(t, 0);
}

void TH_Delete(M4Thread *t)
{
	Thread_Stop(t, 0);
	if (t->threadH) CloseHandle(t->threadH);
	free(t);
}


void TH_SetPriority(M4Thread *t, s32 priority)
{
	SetThreadPriority(t ? t->threadH : GetCurrentThread(), priority);
}

u32 TH_GetStatus(M4Thread *t)
{
	if (!t) return 0;
	return t->status;
}


u32 TH_GetID()
{
	return ((u32) GetCurrentThreadId());
}


/*********************************************************************
						OS-Specific Mutex Object
**********************************************************************/
struct tagM4Mutex
{
	HANDLE hMutex;
	/* We filter recursive calls (1 thread calling Lock several times in a row only locks
	ONCE the mutex. Holder is the current ThreadID of the mutex holder*/
	u32 Holder, HolderCount;
};


M4Mutex *NewMutex()
{
	M4Mutex *tmp = malloc(sizeof(M4Mutex));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(M4Mutex));
	
	tmp->hMutex = CreateMutex(NULL, FALSE, NULL);
	if (!tmp->hMutex) {
		free(tmp);
		return NULL;
	}
	return tmp;
}

void MX_Delete(M4Mutex *mx)
{
	CloseHandle(mx->hMutex);
	free(mx);
}

void MX_V(M4Mutex *mx)
{
	u32 caller;
	if (!mx) return;
	caller = TH_GetID();

	/*only if we own*/
	assert(caller == mx->Holder);
	if (caller != mx->Holder) return;

	assert(mx->HolderCount > 0);
	mx->HolderCount -= 1;

	if (mx->HolderCount == 0) {
		mx->Holder = 0;
		caller = ReleaseMutex(mx->hMutex);
		assert(caller);
	}
}

u32 MX_P(M4Mutex *mx)
{
	u32 caller;
	if (!mx) return 0;
	caller = TH_GetID();
	if (caller == mx->Holder) {
		mx->HolderCount += 1;
		return 1;
	}

	switch (WaitForSingleObject(mx->hMutex, INFINITE)) {
	case WAIT_ABANDONED:
	case WAIT_TIMEOUT:
		mx->Holder = mx->HolderCount = 0;
		return 0;
	default:
		mx->HolderCount = 1;
		mx->Holder = caller;
		return 1;
	}
}



/*********************************************************************
						OS-Specific Semaphore Object
**********************************************************************/
struct tagM4Semaphore
{
	HANDLE hSemaphore;
};


M4Sema *NewSemaphore(u32 MaxCount, u32 InitCount)
{
	M4Sema *tmp = malloc(sizeof(M4Sema));

	if (!tmp) return NULL;
	tmp->hSemaphore = CreateSemaphore(NULL, InitCount, MaxCount, NULL);
	if (!tmp->hSemaphore) {
		free(tmp);
		return NULL;
	}
	return tmp;
}

void SEM_Delete(M4Sema *sm)
{
	CloseHandle(sm->hSemaphore);
	free(sm);
}

u32 SEM_Notify(M4Sema *sm, u32 NbRelease)
{
	LONG prevCount;
	
	if (!sm) return 0;

	ReleaseSemaphore(sm->hSemaphore, NbRelease, &prevCount);
	return (u32) prevCount;
}

void SEM_Wait(M4Sema *sm)
{
	WaitForSingleObject(sm->hSemaphore, INFINITE);
}

Bool SEM_WaitFor(M4Sema *sm, u32 TimeOut)
{
	if (WaitForSingleObject(sm->hSemaphore, TimeOut) == WAIT_TIMEOUT) return 0;
	return 1;
}

