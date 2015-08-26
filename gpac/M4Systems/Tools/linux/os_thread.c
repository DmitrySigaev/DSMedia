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

#include <features.h>
#include <sched.h>
#include <pthread.h>
#include <semaphore.h>

typedef pthread_t TH_HANDLE ;


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

void *RunThread(void *ptr)
{
	u32 ret;
	M4Thread *t = (M4Thread *)ptr;

	/* Signal the caller */
	if (! t->_signal) goto exit;

	t->status = THREAD_STATUS_PLAY;
	
	SEM_Notify(t->_signal, 1);
	/* Run our thread */
	ret = t->Run(t->args);

exit:
	/* kill the pthread*/
	t->status = THREAD_STATUS_DEAD;
	t->Run = NULL;
	pthread_exit((void *)0);
	return (void *)0;
}

M4Err TH_Run(M4Thread *t, u32 (*Run)(void *param), void *param)
{
	pthread_attr_t att;

	if (!t || t->Run || t->_signal) return M4BadParam;
	t->Run = Run;
	t->args = param;
	t->_signal = NewSemaphore(1, 0);

	if ( pthread_attr_init(&att) != 0 ) return M4IOErr;
	pthread_attr_setdetachstate(&att, PTHREAD_CREATE_JOINABLE);
	if ( pthread_create(&t->threadH, &att, RunThread, t) != 0 ) {
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
			pthread_cancel(t->threadH);
			t->threadH = 0;
		} else {
			/*gracefully wait for Run to finish*/
			pthread_join(t->threadH, NULL);			
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
	free(t);
}


void TH_SetPriority(M4Thread *t, s32 priority)
{
	struct sched_param s_par;
	if (!t) return;

	/* consider this as real-time priority */
	if (priority > 200) {
		s_par.sched_priority = priority - 200;
		pthread_setschedparam(t->threadH, SCHED_RR, &s_par);
	} else {
		s_par.sched_priority = priority;
		pthread_setschedparam(t->threadH, SCHED_OTHER, &s_par);
	}
}

u32 TH_GetStatus(M4Thread *t)
{
	if (!t) return 0;
	return t->status;
}


u32 TH_GetID()
{
	return ((u32) pthread_self());
}


/*********************************************************************
						OS-Specific Mutex Object
**********************************************************************/
struct tagM4Mutex
{
	pthread_mutex_t hMutex;
	/* We filter recursive calls (1 thread calling Lock several times in a row only locks
	ONCE the mutex. Holder is the current ThreadID of the mutex holder*/
	u32 Holder, HolderCount;
};


M4Mutex *NewMutex()
{
	pthread_mutexattr_t attr;
	M4Mutex *tmp = malloc(sizeof(M4Mutex));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(M4Mutex));
	
	pthread_mutexattr_init(&attr);
	if ( pthread_mutex_init(&tmp->hMutex, &attr) != 0 ) {
		free(tmp);
		return NULL;
	}
	return tmp;
}

void MX_Delete(M4Mutex *mx)
{
	pthread_mutex_destroy(&mx->hMutex);
	free(mx);
}

void MX_V(M4Mutex *mx)
{
	u32 caller;
	if (!mx) return;
	caller = TH_GetID();
	/*only if we own*/
	if (caller != mx->Holder) return;

	if (mx->HolderCount) {
	  mx->HolderCount -= 1;
	} else {
		mx->Holder = 0;
		pthread_mutex_unlock(&mx->hMutex);
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

	if (pthread_mutex_lock(&mx->hMutex) == 0 ) {
		mx->Holder = caller;
		mx->HolderCount = 0;
		return 1;
	}
	assert(0);
	mx->Holder = mx->HolderCount = 0;
	return 0;
}



/*********************************************************************
						OS-Specific Semaphore Object
**********************************************************************/
struct tagM4Semaphore
{
	sem_t *hSemaphore;
	sem_t SemaData;
};


M4Sema *NewSemaphore(u32 MaxCount, u32 InitCount)
{
	M4Sema *tmp = malloc(sizeof(M4Sema));



	if (!tmp) return NULL;
	if (sem_init(&tmp->SemaData, 0, InitCount) < 0 ) {
		free(tmp);
		return NULL;
	}
	tmp->hSemaphore = &tmp->SemaData;
	return tmp;
}

void SEM_Delete(M4Sema *sm)
{
	sem_destroy(sm->hSemaphore);
	free(sm);
}

u32 SEM_Notify(M4Sema *sm, u32 NbRelease)
{
	u32 prevCount;
	
	if (!sm) return 0;

	sem_getvalue(sm->hSemaphore, (s32 *) &prevCount);
	while (NbRelease) {
		if (sem_post(sm->hSemaphore) < 0) return 0;
		NbRelease -= 1;
	}
	return prevCount;
}

void SEM_Wait(M4Sema *sm)
{
	sem_wait(sm->hSemaphore);
}

Bool SEM_WaitFor(M4Sema *sm, u32 TimeOut)
{
	if (!TimeOut) {
		if (!sem_trywait(sm->hSemaphore)) return 1;
		return 0;
	}
	TimeOut += M4_GetSysClock();
	do {
		if (!sem_trywait(sm->hSemaphore)) return 1;
		Sleep(1);	
	} while (M4_GetSysClock() < TimeOut);
	return 0;
}

