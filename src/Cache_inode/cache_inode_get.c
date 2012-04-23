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
 */

/**
 * \file    cache_inode_get.c
 * \author  $Author: deniel $
 * \date    $Date: 2005/11/28 17:02:26 $
 * \version $Revision: 1.26 $
 * \brief   Get and eventually cache an entry.
 *
 * cache_inode_get.c : Get and eventually cache an entry.
 *
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _SOLARIS
#include "solaris_port.h"
#endif                          /* _SOLARIS */

#include "LRU_List.h"
#include "log.h"
#include "HashData.h"
#include "HashTable.h"
#include "fsal.h"
#include "cache_inode.h"
#include "stuff_alloc.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <time.h>
#include <pthread.h>

/**
 *
 * cache_inode_get: Gets an entry by using its fsdata as a key and caches it if needed.
 * 
 * Gets an entry by using its fsdata as a key and caches it if needed.
 *
 * @param fsdata [IN] file system data
 * @param pattr [OUT] pointer to the attributes for the result. 
 * @param ht [IN] hash table used for the cache, unused in this call.
 * @param pclient [INOUT] ressource allocated by the client for the nfs management.
 * @param pstatus [OUT] returned status.
 * 
 * @return the pointer to the entry is successfull, NULL otherwise.
 *
 */
cache_entry_t *cache_inode_get( cache_inode_fsal_data_t * pfsdata,
                                cache_inode_policy_t policy,
                                fsal_attrib_list_t * pattr,
                                hash_table_t * ht,
                                cache_inode_client_t * pclient,
                                cache_inode_status_t * pstatus )
{
  return cache_inode_get_located( pfsdata, NULL, policy, pattr, ht, pclient, pstatus ) ;
} /* cache_inode_get */

/**
 *
 * cache_inode_geti_located: Gets an entry by using its fsdata as a key and caches it if needed, with origin information.
 * 
 * Gets an entry by using its fsdata as a key and caches it if needed, with origin/location information.
 * The reason to this call is cross-junction management : you can go through a directory that it its own parent from a 
 * FSAL point of view. This could lead to hang (same P_w taken twice on the same entry). To deal this, a check feature is 
 * added through the plocation argument.
 *
 * @param fsdata [IN] file system data
 * @param plocation [IN] pentry used as "location form where the call is done". Usually a son of a parent entry
 * @param pattr [OUT] pointer to the attributes for the result. 
 * @param ht [IN] hash table used for the cache, unused in this call.
 * @param pclient [INOUT] ressource allocated by the client for the nfs management.
 * @param pstatus [OUT] returned status.
 * 
 * @return the pointer to the entry is successfull, NULL otherwise.
 *
 */

cache_entry_t *cache_inode_get_located(cache_inode_fsal_data_t * pfsdata,
                                       cache_entry_t * plocation, 
                                       cache_inode_policy_t policy,
                                       fsal_attrib_list_t * pattr, /* deprecate this */
                                       hash_table_t * ht,
                                       cache_inode_client_t * pclient,
                                       cache_inode_status_t * pstatus)
{
  hash_buffer_t key, value;
  cache_entry_t *pentry = NULL;
  fsal_status_t fsal_status;
  int hrc = 0;
  struct fsal_export *exp_hdl = NULL;  /** @TODO find one */
  struct fsal_obj_handle *new_hdl;

  /* Set the return default to CACHE_INODE_SUCCESS */
  *pstatus = CACHE_INODE_SUCCESS;

  /* stats */
  /* cache_invalidate calls this with no context or client */
  if (pclient) {
    pclient->stat.nb_call_total += 1;
    pclient->stat.func_stats.nb_call[CACHE_INODE_GET] += 1;
  }

  /* Turn the input to a hash key on our own.
   */
  key.pdata = pfsdata->fh_desc.start;
  key.len = pfsdata->fh_desc.len;

  hrc = HashTable_Get(ht, &key, &value);
  switch (hrc)
    {
    case HASHTABLE_SUCCESS:
      /* Entry exists in the cache and was found */
      pentry = (cache_entry_t *) value.pdata;

      /* return attributes additionally */
      *pattr = pentry->obj_handle->attributes;

      if ( !pclient ) {
	/* invalidate. Just return it to mark it stale and go on. */
	return( pentry );
      }

      break;

    case HASHTABLE_ERROR_NO_SUCH_KEY:
      if ( !pclient ) {
	/* invalidate. Just return */
	return( NULL );
      }
      /* Cache miss, allocate a new entry */
      exp_hdl = pfsdata->export;
      fsal_status = exp_hdl->ops->create_handle(exp_hdl, &pfsdata->fh_desc, &new_hdl);
      if( FSAL_IS_ERROR( fsal_status ) )
        {
	  *pstatus = cache_inode_error_convert(fsal_status);
	  LogDebug(COMPONENT_CACHE_INODE,
		   "could not get create_handle object");
	  return NULL;
	}

      /* Add the entry to the cache */
      if((pentry = cache_inode_new_entry(new_hdl,
					 policy, 
					 ht, 
					 pclient, 
					 FALSE,  /* This is a population, not a creation */
					 pstatus ) ) == NULL )
        {
          /* stats */
          pclient->stat.func_stats.nb_err_unrecover[CACHE_INODE_GET] += 1;

          return NULL;
        }

      /* Set the returned attributes */
/** @TODO bad code, bad code. for now, NULL kills this. later, zip from args */
      if(pattr != NULL)
	      *pattr = pentry->obj_handle->attributes;

      /* Now, exit the switch/case and returns */
      break;

    default:
      /* This should not happened */
      *pstatus = CACHE_INODE_INVALID_ARGUMENT;
      LogCrit(COMPONENT_CACHE_INODE,
              "cache_inode_get returning CACHE_INODE_INVALID_ARGUMENT - this should not have happened");

      if ( !pclient ) {
        /* invalidate. Just return */
        return( NULL );
      }

      /* stats */
      pclient->stat.func_stats.nb_err_unrecover[CACHE_INODE_GET] += 1;

      return NULL;
      break;
    }  /* end switch */

  /* valid the found entry, if this is not feasable, returns nothing to the client */
  if( plocation != NULL )
   {
     if( plocation != pentry )
      {
        P_w(&pentry->lock);
        if((*pstatus =
           cache_inode_valid(pentry, CACHE_INODE_OP_GET, pclient)) != CACHE_INODE_SUCCESS)
          {
            V_w(&pentry->lock);
            pentry = NULL;
          }
        V_w(&pentry->lock);
      }
   }

  /* stats */
  pclient->stat.func_stats.nb_success[CACHE_INODE_GET] += 1;

  return pentry;
}  /* cache_inode_get_located */
