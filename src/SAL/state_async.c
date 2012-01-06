/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright CEA/DAM/DIF  (2008)
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * ---------------------------------------
 *
 * \File    state_async.c
 * \author  $Author: deniel $
 * \date    $Date: 2006/01/05 15:14:51 $
 * \version $Revision: 1.63 $
 * \brief   Some routines for management of state manager asynchronous processing.
 *
 * state_async.c : Some routines for management of state manager asynchronous processing.
 *
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _SOLARIS
#include "solaris_port.h"
#endif                          /* _SOLARIS */

#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#include "log_macros.h"
#include "HashData.h"
#include "HashTable.h"
#include "fsal.h"
#include "sal_functions.h"
#include "nfs_tcb.h"

#ifdef _USE_BLOCKING_LOCKS
static pthread_t               state_async_thread_id;
static struct glist_head       state_async_queue;
cache_inode_client_parameter_t state_async_cache_inode_client_param;
cache_inode_client_t           state_async_cache_inode_client;
nfs_tcb_t                      state_async_tcb;

/* Execute a func from the async queue */
void *state_async_thread(void *argp)
{
#ifndef _NO_BUDDY_SYSTEM
  int rc;
#endif
  state_async_queue_t * entry;
  struct timeval        now;
  struct timespec       timeout;
  state_block_data_t  * pblock;

  SetNameFunction("state_async_thread");

#ifndef _NO_BUDDY_SYSTEM
  if((rc = BuddyInit(NULL)) != BUDDY_SUCCESS)
    {
      /* Failed init */
      LogFatal(COMPONENT_STATE,
               "State Async Thread: Memory manager could not be initialized");
    }
  LogInfo(COMPONENT_STATE,
          "State Async Thread: Memory manager successfully initialized");
#endif
  if(mark_thread_existing(&state_async_tcb) == PAUSE_EXIT)
    {
      /* Oops, that didn't last long... exit. */
      mark_thread_done(&state_async_tcb);
      LogDebug(COMPONENT_STATE,
               "State Async Thread: exiting before initialization");
      return NULL;
    }
  LogFullDebug(COMPONENT_STATE,
               "State Async Thread: my pthread id is %p",
               (caddr_t) pthread_self());

  while(1)
    {
      /* Check without tcb lock*/
      if((state_async_tcb.tcb_state != STATE_AWAKE) ||
          (glist_empty(&state_async_queue) &&
           glist_empty(&state_notified_locks)))
        {
          while(1)
            {
              P(state_async_tcb.tcb_mutex);
              if((state_async_tcb.tcb_state == STATE_AWAKE) &&
                  (!glist_empty(&state_async_queue) ||
                   !glist_empty(&state_notified_locks)))
                {
                  V(state_async_tcb.tcb_mutex);
                  break;
                }
              switch(thread_sm_locked(&state_async_tcb))
                {
                  case THREAD_SM_RECHECK:
                    V(state_async_tcb.tcb_mutex);
                    continue;

                  case THREAD_SM_BREAK:
                    if(glist_empty(&state_async_queue))
                      {
                        gettimeofday(&now, NULL);
                        timeout.tv_sec = 10 + now.tv_sec;
                        timeout.tv_nsec = 0;
                        pthread_cond_timedwait(&state_async_tcb.tcb_condvar,
                                               &state_async_tcb.tcb_mutex,
                                               &timeout);
                      }
                    V(state_async_tcb.tcb_mutex);
                    continue;

                  case THREAD_SM_EXIT:
                    V(state_async_tcb.tcb_mutex);
                    return NULL;
                }
            }
        }

      /* Check for async blocking lock notifications first */
      P(blocked_locks_mutex);

      /* Handle one async blocking lock notification and loop back */
      pblock = glist_first_entry(&state_notified_locks,
                                 state_block_data_t,
                                 sbd_list);

      if(pblock != NULL)
        {
          /* Pull block off list */
          glist_del(&pblock->sbd_list);

          /* Block is off list, no need to hold mutex any more */
          V(blocked_locks_mutex);

          /*
          process_blocked_lock_upcall(pblock,
                                      &state_async_cache_inode_client);
          */

          continue;
        }

      V(blocked_locks_mutex);

      /* Process one request if available */
      P(state_async_tcb.tcb_mutex);

      entry = glist_first_entry(&state_async_queue,
                                state_async_queue_t,
                                state_async_glist);
      if(entry != NULL)
        {
          /* Pull entry off list */
          glist_del(&entry->state_async_glist);

          /* Entry is off list, no need to hold mutex any more */
          V(state_async_tcb.tcb_mutex);

          /* Process async queue entry */
          entry->state_async_func(entry);

          continue;
        }

      V(state_async_tcb.tcb_mutex);
    }
  tcb_remove(&state_async_tcb);
}

/* Schedule Async Work */
state_status_t state_async_schedule(state_async_queue_t *arg)
{
  int rc;

  LogFullDebug(COMPONENT_STATE, "Schedule %p", arg);

  P(state_async_tcb.tcb_mutex);

  glist_add_tail(&state_async_queue, &arg->state_async_glist);

  rc = pthread_cond_signal(&state_async_tcb.tcb_condvar);

  if(rc == -1)
    {
      LogFullDebug(COMPONENT_STATE,
                   "Unable to signal State Async Thread");
      glist_del(&arg->state_async_glist);
    }

  V(state_async_tcb.tcb_mutex);

  return rc != -1 ? STATE_SUCCESS : STATE_SIGNAL_ERROR;
}

/* Signal Async Work */
void signal_async_work()
{
  int rc;

  P(state_async_tcb.tcb_mutex);

  rc = pthread_cond_signal(&state_async_tcb.tcb_condvar);

  if(rc == -1)
    LogFatal(COMPONENT_STATE,
             "Unable to signal State Async Thread");

  V(state_async_tcb.tcb_mutex);
}

static int local_lru_inode_entry_to_str(LRU_data_t data, char *str)
{
  return sprintf(str, "N/A ");
}                               /* local_lru_inode_entry_to_str */

static int local_lru_inode_clean_entry(LRU_entry_t * entry, void *adddata)
{
  return 0;
}                               /* lru_clean_entry */
#endif

state_status_t state_async_init()
{
#ifdef _USE_BLOCKING_LOCKS
  init_glist(&state_async_queue);

  /* setting the 'state_async_cache_inode_client_param' structure */
  state_async_cache_inode_client_param.lru_param.nb_entry_prealloc = 10;
  state_async_cache_inode_client_param.lru_param.entry_to_str      = local_lru_inode_entry_to_str;
  state_async_cache_inode_client_param.lru_param.clean_entry       = local_lru_inode_clean_entry;
  state_async_cache_inode_client_param.nb_prealloc_entry           = 0;
  state_async_cache_inode_client_param.nb_pre_parent               = 0;
  state_async_cache_inode_client_param.nb_pre_state_v4             = 0;
  state_async_cache_inode_client_param.grace_period_link           = 0;
  state_async_cache_inode_client_param.grace_period_attr           = 0;
  state_async_cache_inode_client_param.grace_period_dirent         = 0;
  state_async_cache_inode_client_param.grace_period_attr           = 0;
  state_async_cache_inode_client_param.grace_period_link           = 0;
  state_async_cache_inode_client_param.grace_period_dirent         = 0;
  state_async_cache_inode_client_param.expire_type_attr            = CACHE_INODE_EXPIRE_NEVER;
  state_async_cache_inode_client_param.expire_type_link            = CACHE_INODE_EXPIRE_NEVER;
  state_async_cache_inode_client_param.expire_type_dirent          = CACHE_INODE_EXPIRE_NEVER;
  state_async_cache_inode_client_param.use_test_access             = 1;
  state_async_cache_inode_client_param.attrmask                    = 0;

  if(cache_inode_client_init(&state_async_cache_inode_client,
                             state_async_cache_inode_client_param,
                             NLM_THREAD_INDEX, NULL))
    {
      LogCrit(COMPONENT_STATE,
              "Could not initialize cache inode client for State Async Thread");
      return STATE_INIT_ENTRY_FAILED;
    }

  tcb_new(&state_async_tcb, "State Async Thread");
#endif
  return STATE_SUCCESS;
}

void state_async_thread_start()
{
#ifdef _USE_BLOCKING_LOCKS
  if(pthread_create(&state_async_thread_id, NULL, state_async_thread, NULL) != 0)
    LogFatal(COMPONENT_STATE, "Could not start State Async Thread");
#else
  return;
#endif
}
