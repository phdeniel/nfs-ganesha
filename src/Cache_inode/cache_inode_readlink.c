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
 * \file    cache_inode_readlink.c
 * \author  $Author: deniel $
 * \date    $Date: 2005/12/05 09:02:36 $
 * \version $Revision: 1.16 $
 * \brief   Reads a symlink.
 *
 * cache_inode_readlink.c : Reads a symlink.
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>

cache_inode_status_t cache_inode_readlink(cache_entry_t * pentry,
					  fsal_path_t * plink_content,
					  hash_table_t * ht,       /* Unused, kept for
								    * protototype's homogeneity */
                                          cache_inode_client_t * pclient,
                                          struct user_cred *creds,
                                          cache_inode_status_t * pstatus)
{
  fsal_status_t fsal_status;
  fsal_accessflags_t access_mask = 0;

  /* Set the return default to CACHE_INODE_SUCCESS */
  *pstatus = CACHE_INODE_SUCCESS;

  /* stats */
  (pclient->stat.nb_call_total)++;
  (pclient->stat.func_stats.nb_call[CACHE_INODE_READLINK])++;

  /* Lock the entry */
  P_w(&pentry->lock);
  if(cache_inode_renew_entry(pentry, NULL, ht, pclient, pstatus) !=
     CACHE_INODE_SUCCESS)
    {
      (pclient->stat.func_stats.nb_err_retryable[CACHE_INODE_READLINK])++;
      V_w(&pentry->lock);
      return *pstatus;
    }
  if( !pentry->obj_handle->ops->handle_is(pentry->obj_handle,
					  FSAL_TYPE_LNK))
    {
      V_w(&pentry->lock);
      *pstatus = CACHE_INODE_BAD_TYPE;
      /* stats */
      pclient->stat.func_stats.nb_err_unrecover[CACHE_INODE_READLINK] += 1;

      return *pstatus;
    }

  /* RW_Lock obtained as writer turns to reader */
  rw_lock_downgrade(&pentry->lock);

  /* Check is user (as specified by the credentials) is authorized to read
   * the symlink.  Is this an 'attribute?' */
  access_mask = FSAL_MODE_MASK_SET(FSAL_R_OK) |
                FSAL_ACE4_MASK_SET(FSAL_ACE_PERM_READ_ATTR);
  if(cache_inode_access_no_mutex(pentry,
                                 access_mask,
                                 ht, pclient,
				 creds,
				 pstatus) != CACHE_INODE_SUCCESS)
    {
      V_r(&pentry->lock);

      (pclient->stat.func_stats.nb_err_retryable[CACHE_INODE_READLINK])++;
      return *pstatus;
    }
  /* Content is not cached, call FSAL_readlink here */
  fsal_status = pentry->obj_handle->ops->readlink(pentry->obj_handle,
						  plink_content->path,
						  FSAL_MAX_PATH_LEN) ; 
  if(FSAL_IS_ERROR(fsal_status))
    {
          *pstatus = cache_inode_error_convert(fsal_status);
          V_r(&pentry->lock);

          if(fsal_status.major == ERR_FSAL_STALE)
            {
              cache_inode_status_t kill_status;

              LogEvent(COMPONENT_CACHE_INODE,
                       "cache_inode_readlink: Stale FSAL File Handle detected for pentry = %p, fsal_status=(%u,%u)",
                       pentry, fsal_status.major, fsal_status.minor);

              if(cache_inode_kill_entry(pentry, NO_LOCK, ht, pclient, &kill_status) !=
                 CACHE_INODE_SUCCESS)
                LogCrit(COMPONENT_CACHE_INODE,
                        "cache_inode_readlink: Could not kill entry %p, status = %u",
                        pentry, kill_status);

              *pstatus = CACHE_INODE_FSAL_ESTALE;
            }
          /* stats */
          pclient->stat.func_stats.nb_err_unrecover[CACHE_INODE_READLINK] += 1;

          return *pstatus;
    }


  /* Release the entry */
  *pstatus = cache_inode_valid(pentry, CACHE_INODE_OP_GET, pclient);
  V_r(&pentry->lock);

  /* stat */
  if(*pstatus != CACHE_INODE_SUCCESS)
    pclient->stat.func_stats.nb_err_retryable[CACHE_INODE_READLINK] += 1;
  else
    pclient->stat.func_stats.nb_success[CACHE_INODE_READLINK] += 1;

  return *pstatus;
}                               /* cache_inode_readlink */
