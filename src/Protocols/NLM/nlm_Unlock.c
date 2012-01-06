/*
 * Copyright IBM Corporation, 2010
 *  Contributor: Aneesh Kumar K.v  <aneesh.kumar@linux.vnet.ibm.com>
 *
 * --------------------------
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
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _SOLARIS
#include "solaris_port.h"
#endif

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "rpc.h"
#include "log_macros.h"
#include "stuff_alloc.h"
#include "nlm4.h"
#include "sal_functions.h"
#include "nlm_util.h"
#include "nlm_async.h"

/**
 * nlm4_Unlock: Set a range lock
 *
 *  @param parg        [IN]
 *  @param pexportlist [IN]
 *  @param pcontextp   [IN]
 *  @param pclient     [INOUT]
 *  @param ht          [INOUT]
 *  @param preq        [IN]
 *  @param pres        [OUT]
 *
 */

int nlm4_Unlock(nfs_arg_t * parg /* IN     */ ,
                exportlist_t * pexport /* IN     */ ,
                fsal_op_context_t * pcontext /* IN     */ ,
                cache_inode_client_t * pclient /* INOUT  */ ,
                hash_table_t * ht /* INOUT  */ ,
                struct svc_req *preq /* IN     */ ,
                nfs_res_t * pres /* OUT    */ )
{
  nlm4_unlockargs    * arg = &parg->arg_nlm4_unlock;
  cache_entry_t      * pentry;
  state_status_t       state_status = CACHE_INODE_SUCCESS;
  char                 buffer[MAXNETOBJ_SZ * 2];
  state_nsm_client_t * nsm_client;
  state_nlm_client_t * nlm_client;
  state_owner_t      * nlm_owner;
  fsal_lock_param_t    lock;
  int                  rc;

  netobj_to_string(&arg->cookie, buffer, sizeof(buffer));
  LogDebug(COMPONENT_NLM,
           "REQUEST PROCESSING: Calling nlm4_Unlock svid=%d off=%llx len=%llx cookie=%s",
           (int) arg->alock.svid,
           (unsigned long long) arg->alock.l_offset,
           (unsigned long long) arg->alock.l_len,
           buffer);

  if(!copy_netobj(&pres->res_nlm4test.cookie, &arg->cookie))
    {
      pres->res_nlm4.stat.stat = NLM4_FAILED;
      LogDebug(COMPONENT_NLM, "REQUEST RESULT: nlm4_Unlock %s",
               lock_result_str(pres->res_nlm4.stat.stat));
      return NFS_REQ_OK;
    }


  if(in_nlm_grace_period())
    {
      pres->res_nlm4.stat.stat = NLM4_DENIED_GRACE_PERIOD;
      LogDebug(COMPONENT_NLM, "REQUEST RESULT: nlm4_Unlock %s",
               lock_result_str(pres->res_nlm4.stat.stat));
      return NFS_REQ_OK;
    }

  rc = nlm_process_parameters(preq,
                              FALSE, /* exlcusive doesn't matter */
                              &arg->alock,
                              &lock,
                              ht,
                              &pentry,
                              pcontext,
                              pclient,
                              CARE_NOT, /* unlock doesn't care if owner is found */
                              &nsm_client,
                              &nlm_client,
                              &nlm_owner,
                              NULL);

  if(rc >= 0)
    {
      /* Present the error back to the client */
      pres->res_nlm4.stat.stat = (nlm4_stats)rc;
      LogDebug(COMPONENT_NLM, "REQUEST RESULT: nlm4_Unlock %s",
               lock_result_str(pres->res_nlm4.stat.stat));
      return NFS_REQ_OK;
    }

  if(state_unlock(pentry,
                  pcontext,
                  nlm_owner,
                  NULL,
                  &lock,
                  pclient,
                  &state_status) != STATE_SUCCESS)
    {
      /* Unlock could fail in the FSAL and make a bit of a mess, especially if
       * we are in out of memory situation. Such an error is logged by
       * Cache Inode.
       */
      pres->res_nlm4test.test_stat.stat = nlm_convert_state_error(state_status);
    }
  else
    {
      pres->res_nlm4.stat.stat = NLM4_GRANTED;
    }

  /* Release the NLM Client and NLM Owner references we have */
  dec_nsm_client_ref(nsm_client);
  dec_nlm_client_ref(nlm_client);
  dec_state_owner_ref(nlm_owner, pclient);

  LogDebug(COMPONENT_NLM, "REQUEST RESULT: nlm4_Unlock %s",
           lock_result_str(pres->res_nlm4.stat.stat));
  return NFS_REQ_OK;
}

static void nlm4_unlock_message_resp(state_async_queue_t *arg)
{
  state_nlm_async_data_t * nlm_arg = &arg->state_async_data.state_nlm_async_data;

  if(isFullDebug(COMPONENT_NLM))
    {
      char buffer[1024];
      netobj_to_string(&nlm_arg->nlm_async_args.nlm_async_res.res_nlm4test.cookie, buffer, 1024);
      LogFullDebug(COMPONENT_NLM,
                   "Calling nlm_send_async cookie=%s status=%s",
                   buffer, lock_result_str(nlm_arg->nlm_async_args.nlm_async_res.res_nlm4.stat.stat));
    }
  nlm_send_async(NLMPROC4_UNLOCK_RES,
                 nlm_arg->nlm_async_host,
                 &(nlm_arg->nlm_async_args.nlm_async_res),
                 NULL);
  nlm4_Unlock_Free(&nlm_arg->nlm_async_args.nlm_async_res);
  dec_nsm_client_ref(nlm_arg->nlm_async_host->slc_nsm_client);
  dec_nlm_client_ref(nlm_arg->nlm_async_host);
  Mem_Free(arg);
}

/**
 * nlm4_Unlock_Message: Unlock Message
 *
 *  @param parg        [IN]
 *  @param pexportlist [IN]
 *  @param pcontextp   [IN]
 *  @param pclient     [INOUT]
 *  @param ht          [INOUT]
 *  @param preq        [IN]
 *  @param pres        [OUT]
 *
 */
int nlm4_Unlock_Message(nfs_arg_t * parg /* IN     */ ,
                        exportlist_t * pexport /* IN     */ ,
                        fsal_op_context_t * pcontext /* IN     */ ,
                        cache_inode_client_t * pclient /* INOUT  */ ,
                        hash_table_t * ht /* INOUT  */ ,
                        struct svc_req *preq /* IN     */ ,
                        nfs_res_t * pres /* OUT    */ )
{
  state_nlm_client_t * nlm_client = NULL;
  state_nsm_client_t * nsm_client;
  nlm4_unlockargs    * arg = &parg->arg_nlm4_unlock;
  int                  rc = NFS_REQ_OK;

  LogDebug(COMPONENT_NLM, "REQUEST PROCESSING: Calling nlm_Unlock_Message");

  nsm_client = get_nsm_client(CARE_NO_MONITOR, preq->rq_xprt, arg->alock.caller_name);

  if(nsm_client != NULL)
    nlm_client = get_nlm_client(CARE_NO_MONITOR, preq->rq_xprt, nsm_client, arg->alock.caller_name);

  if(nlm_client == NULL)
    rc = NFS_REQ_DROP;
  else
    rc = nlm4_Unlock(parg, pexport, pcontext, pclient, ht, preq, pres);

  if(rc == NFS_REQ_OK)
    rc = nlm_send_async_res_nlm4(nlm_client, nlm4_unlock_message_resp, pres);

  if(rc == NFS_REQ_DROP)
    {
      if(nsm_client != NULL)
        dec_nsm_client_ref(nsm_client);
      if(nlm_client != NULL)
        dec_nlm_client_ref(nlm_client);
      LogCrit(COMPONENT_NLM,
            "Could not send async response for nlm_Unlock_Message");
    }

  return NFS_REQ_DROP;
}

/**
 * nlm4_Unlock_Free: Frees the result structure allocated for nlm4_Unlock
 *
 * Frees the result structure allocated for nlm4_Lock. Does Nothing in fact.
 *
 * @param pres        [INOUT]   Pointer to the result structure.
 *
 */
void nlm4_Unlock_Free(nfs_res_t * pres)
{
  netobj_free(&pres->res_nlm4test.cookie);
  return;
}
