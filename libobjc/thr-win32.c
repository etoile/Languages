/* GNU Objective C Runtime Thread Interface - Win32 Implementation
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   Contributed by Galen C. Hunt (gchunt@cs.rochester.edu)

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2, or (at your option) any later version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License along with
GCC; see the file COPYING.  If not, write to the Free Software
Foundation, 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.  */

/* As a special exception, if you link this library with files compiled with
   GCC to produce an executable, this does not cause the resulting executable
   to be covered by the GNU General Public License. This exception does not
   however invalidate any other reasons why the executable file might be
   covered by the GNU General Public License.  */

#include "objc/thr.h"
#include "objc/runtime.h"

#ifndef __OBJC__
#define __OBJC__
#endif
#include <windows.h>
#include <process.h>

/* Key structure for maintaining thread specific storage */
static DWORD	__objc_data_tls = (DWORD)-1;

/* Backend initialization functions */

/* Initialize the threads subsystem. */
int
__objc_init_thread_system(void)
{
  /* Initialize the thread storage key */
  if ((__objc_data_tls = TlsAlloc()) != (DWORD)-1)
    return 0;
  else
    return -1;
}

/* Close the threads subsystem. */
int
__objc_close_thread_system(void)
{
  if (__objc_data_tls != (DWORD)-1)
    TlsFree(__objc_data_tls);
  return 0;
}

/* Backend thread functions */

/* Create a new thread of execution. */
objc_thread_t
__objc_thread_detach(void (*func)(void *arg), void *arg)
{
  DWORD	thread_id = 0;
  HANDLE win32_handle;

#ifndef __MINGW32__
  if (!(win32_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func,
                                   arg, 0, &thread_id)))
#else 
  // According to MSDN documentation threads which use libc functions should call _beginthreadex not CreateThread
  if ((HANDLE)-1 == (win32_handle = (HANDLE)_beginthreadex(NULL, 0, (void*)func, arg, 0, (int*)&thread_id)))
#endif
    thread_id = 0;
    
  CloseHandle((HANDLE)win32_handle);
  return (objc_thread_t)thread_id;
}

/* Set the current thread's priority. */
int
__objc_thread_set_priority(int priority)
{
  int sys_priority = 0;

  switch (priority)
    {
    case OBJC_THREAD_INTERACTIVE_PRIORITY:
      sys_priority = THREAD_PRIORITY_NORMAL;
      break;
    default:
    case OBJC_THREAD_BACKGROUND_PRIORITY:
      sys_priority = THREAD_PRIORITY_BELOW_NORMAL;
      break;
    case OBJC_THREAD_LOW_PRIORITY:
      sys_priority = THREAD_PRIORITY_LOWEST;
      break;
    }

  /* Change priority */
  if (SetThreadPriority(GetCurrentThread(), sys_priority))
    return 0;
  else
    return -1;
}

/* Return the current thread's priority. */
int
__objc_thread_get_priority(void)
{
  int sys_priority;

  sys_priority = GetThreadPriority(GetCurrentThread());
  
  switch (sys_priority)
    {
    case THREAD_PRIORITY_HIGHEST:
    case THREAD_PRIORITY_TIME_CRITICAL:
    case THREAD_PRIORITY_ABOVE_NORMAL:
    case THREAD_PRIORITY_NORMAL:
      return OBJC_THREAD_INTERACTIVE_PRIORITY;

    default:
    case THREAD_PRIORITY_BELOW_NORMAL:
      return OBJC_THREAD_BACKGROUND_PRIORITY;
    
    case THREAD_PRIORITY_IDLE:
    case THREAD_PRIORITY_LOWEST:
      return OBJC_THREAD_LOW_PRIORITY;
    }

  /* Couldn't get priority. */
  return -1;
}

/* Yield our process time to another thread. */
void
__objc_thread_yield(void)
{
  Sleep(0);
}

/* Terminate the current thread. */
int
__objc_thread_exit(void)
{
  /* exit the thread */
#ifndef __MINGW32__
  ExitThread(__objc_thread_exit_status);
#else
  _endthreadex(__objc_thread_exit_status);
#endif

  /* Failed if we reached here */
  return -1;
}

/* Returns an integer value which uniquely describes a thread. */
objc_thread_t
__objc_thread_id(void)
{
  return (objc_thread_t)GetCurrentThreadId();
}

/* Sets the thread's local storage pointer. */
int
__objc_thread_set_data(void *value)
{
  if (TlsSetValue(__objc_data_tls, value))
    return 0;
  else
    return -1;
}

/* Returns the thread's local storage pointer. */
void *
__objc_thread_get_data(void)
{
  return TlsGetValue(__objc_data_tls);          /* Return thread data.      */
}

/* Backend mutex functions */

/* Allocate a mutex. */
int
__objc_mutex_allocate(objc_mutex_t mutex)
{
  if ((mutex->backend = (void *)CreateMutex(NULL, 0, NULL)) == NULL)
    return -1;
  else
    return 0;
}

/* Deallocate a mutex. */
int
__objc_mutex_deallocate(objc_mutex_t mutex)
{
  CloseHandle((HANDLE)(mutex->backend));
  return 0;
}

/* Grab a lock on a mutex. */
int
__objc_mutex_lock(objc_mutex_t mutex)
{
  int status;

  status = WaitForSingleObject((HANDLE)(mutex->backend), INFINITE);
  if (status != WAIT_OBJECT_0 && status != WAIT_ABANDONED)
    return -1;
  else
    return 0;
}

/* Try to grab a lock on a mutex. */
int
__objc_mutex_trylock(objc_mutex_t mutex)
{
  int status;

  status = WaitForSingleObject((HANDLE)(mutex->backend), 0);
  if (status != WAIT_OBJECT_0 && status != WAIT_ABANDONED)
    return -1;
  else
    return 0;
}

/* Unlock the mutex */
int
__objc_mutex_unlock(objc_mutex_t mutex)
{
  if (ReleaseMutex((HANDLE)(mutex->backend)) == 0)
    return -1;
  else
    return 0;
}

/* Backend condition mutex functions */

#define MAX_EVENTS_TO_SET 2048

typedef struct
{
    int bottom;		/* Lower index of circular array */
    int top;		/* Upper index +1 of circular array */
    /* If top-bottom==0 there are no elements in the array.  The first
       element will increase top with 1 and all subsequent elements
       added will increase both. Each element removed from head will
       only increase bottom with 1.
       By definition eventsToSet[MAX_EVENTS_TO_SET]==eventsToSet[0].
    */
    HANDLE eventsToSet[MAX_EVENTS_TO_SET];	/* Circular array containing the events that are yet to be set */
    objc_mutex_t mutex;	/* Mutex that guards this condition struct */

    objc_thread_t broadcast_busy; /* == thread id of thread that is currently broadcasting or 0 if none is. */
} objc_condition_win32_backend;


int objc_condition_win32_backend_init(objc_condition_win32_backend *cond)
{
    cond->mutex = objc_mutex_allocate();
    if (cond->mutex == 0)
        return -1;

    if (objc_mutex_lock(cond->mutex)<1)
        return -1;
    
    cond->bottom = 0;
    cond->top = 0;
    cond->broadcast_busy = 0;

    objc_mutex_unlock(cond->mutex);

    return 0;
}


int objc_condition_win32_backend_destroy(objc_condition_win32_backend *cond)
{
    int i;
    
    if (objc_mutex_lock(cond->mutex)<1)
        return -1;

    if (cond->top<cond->bottom)
        cond->top += MAX_EVENTS_TO_SET;
    for (i=cond->bottom; i<cond->top; i++)
        {
        if (i>=MAX_EVENTS_TO_SET)
            CloseHandle(cond->eventsToSet[i-MAX_EVENTS_TO_SET]);
        else
            CloseHandle(cond->eventsToSet[i]);
        }

    objc_mutex_deallocate(cond->mutex);

    return 0;
}


int nrOfEventsInCondition(objc_condition_win32_backend *cond)
/* Calling thread must already have locked cond->mutex */
{
    int nr=cond->top-cond->bottom;
    
    if (nr<0)
        return nr+MAX_EVENTS_TO_SET;
    else
        return nr;
}


HANDLE objc_condition_win32_backend_register_at_condition(objc_condition_win32_backend *cond)
/* Return a fresh event handle that has been added at the top of the list of cond. */
{
    HANDLE event = CreateEvent (NULL,  /* no security */
                                FALSE, /* auto-reset */
                                FALSE, /* non-signaled initially */
                                NULL); /* unnamed */
    if (!event)
        return INVALID_HANDLE_VALUE;

    if (objc_mutex_lock(cond->mutex)<1)
        {
        CloseHandle(event);
        return INVALID_HANDLE_VALUE;
        }

    if (nrOfEventsInCondition(cond) >= MAX_EVENTS_TO_SET)
        {
        objc_mutex_unlock(cond->mutex);
        CloseHandle(event);
        return INVALID_HANDLE_VALUE;
        }
    
    if (cond->top == MAX_EVENTS_TO_SET)
        cond->top=0;
    
    cond->eventsToSet[cond->top] = event;
    cond->top++;

    objc_mutex_unlock(cond->mutex);

    return event;
}


int objc_condition_win32_backend_wait(objc_condition_win32_backend *cond, objc_mutex_t external_mutex)
{
    HANDLE event = objc_condition_win32_backend_register_at_condition(cond);
    if (event==INVALID_HANDLE_VALUE)
        return -1;

    /* We shouln't use objc_mutex_unlock here to handle the
     * external_mutex, because objc_condition_wait (which is always
     * higher up the call chain for this function) expects us _only_ to
     * handle the backend of external_mutex. Therefore use
     * __objc_mutex_unlock and __objc_mutex_lock to manipulate
     * external_mutex. */
    if (__objc_mutex_unlock(external_mutex))
        {
        CloseHandle(event);
        return -1;
        }

    /* Actually wait for the condition to be signaled */
    WaitForSingleObject(event, INFINITE);
    CloseHandle(event);
    
    if (__objc_mutex_lock(external_mutex))
        {
        return -1;
        }
    
    return 0;
}

    
int objc_condition_win32_backend_signal(objc_condition_win32_backend *cond)
{
    HANDLE event=0;
    
    /* First make sure we can manipulate cond */
    if (objc_mutex_lock(cond->mutex)<1)
        {
        return -1;
        }

    /* If there actually is a thread to signal, signal the first one */
    if (nrOfEventsInCondition(cond))
        {
        event = cond->eventsToSet[cond->bottom];

        /* Remove the event from the list */
        cond->bottom++;
        if (cond->bottom==MAX_EVENTS_TO_SET)
            {
            cond->bottom=0;
            if (cond->top==MAX_EVENTS_TO_SET)
                cond->top=0;
            }
        }

    /* Done manipulating cond */
    objc_mutex_unlock(cond->mutex);

    /* Do the actual signaling */
    if (event)
        SetEvent(event); /* It will be deallocated by _wait */

    return 0;
}


int objc_condition_win32_backend_broadcast(objc_condition_win32_backend *cond)
{
    objc_thread_t current_thread_id;

    if (objc_mutex_lock(cond->mutex)<1)
        return -1;

    current_thread_id = __objc_thread_id();

    if (cond->broadcast_busy == current_thread_id)
        {
        /* While broadcast is running, broadcast is called on the same
           thread. This shouldn't happen. Just succeed and return. */
        objc_mutex_unlock(cond->mutex);
        return 0;
        }

    /* If broadcast is called while a broadcast is running on another
       thread we choose to let the new thread take over the signaling
       of the events that are still listed. This is to prevent a
       broadcasting thread to block because some other thread keeps
       adding waiters for conditional locks and thus keeping the
       eventsToSet array filled. */
    cond->broadcast_busy = current_thread_id;

    while (nrOfEventsInCondition(cond) && cond->broadcast_busy==current_thread_id)
        {
        objc_mutex_unlock(cond->mutex);
        if (objc_condition_win32_backend_signal(cond))
            return -1;
        objc_mutex_lock(cond->mutex);
        }

    if (cond->broadcast_busy==current_thread_id)
        cond->broadcast_busy=0;
    
    objc_mutex_unlock(cond->mutex);

    return 0;
}


/* Allocate a condition. */
int
__objc_condition_allocate(objc_condition_t condition)
{
    condition->backend = objc_malloc(sizeof(objc_condition_win32_backend));
    return objc_condition_win32_backend_init((objc_condition_win32_backend *)(condition->backend));
}

/* Deallocate a condition. */
int
__objc_condition_deallocate(objc_condition_t condition)
{
    int return_value = objc_condition_win32_backend_destroy((objc_condition_win32_backend *)(condition->backend));
    if (return_value==0)
        objc_free(condition->backend);
    return return_value;
}


/* Wait on the condition */
int
__objc_condition_wait(objc_condition_t condition, objc_mutex_t external_mutex)
{
    return objc_condition_win32_backend_wait(
        (objc_condition_win32_backend *)(condition->backend),
        external_mutex);
}

/* Wake up all threads waiting on this condition. */
int
__objc_condition_broadcast(objc_condition_t condition)
{
    return objc_condition_win32_backend_broadcast(
        (objc_condition_win32_backend *)(condition->backend));
}

/* Wake up one thread waiting on this condition. */
int
__objc_condition_signal(objc_condition_t condition)
{
    return objc_condition_win32_backend_signal(
        (objc_condition_win32_backend *)(condition->backend));
}

/* End of File */
