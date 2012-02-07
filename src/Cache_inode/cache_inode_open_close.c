/**
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
 * \file    cache_inode_open_close.c
 * \author  $Author: deniel $
 * \date    $Date: 2005/11/28 17:02:27 $
 * \version $Revision: 1.20 $
 * \brief   Removes an entry of any type.
 *
 * cache_inode_rdwr.c : performs an IO on a REGULAR_FILE.
 *
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _SOLARIS
#include "solaris_port.h"
#endif                          /* _SOLARIS */

#include "fsal.h"

#include "LRU_List.h"
#include "log.h"
#include "HashData.h"
#include "HashTable.h"
#include "cache_inode.h"
#include "cache_content.h"
#include "stuff_alloc.h"

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <time.h>
#include <pthread.h>
#include <strings.h>

fsal_file_t * cache_inode_fd(cache_entry_t * pentry)
{
  if(pentry == NULL)
    return NULL;

  if(pentry->internal_md.type != REGULAR_FILE)
      return NULL;

/*   if(((pentry->object.file.open_fd.openflags == FSAL_O_RDONLY) || */
/*       (pentry->object.file.open_fd.openflags == FSAL_O_RDWR) || */
/*       (pentry->object.file.open_fd.openflags == FSAL_O_WRONLY)) && */
/*      (pentry->object.file.open_fd.fileno != 0)) */
/*     { */
/*       return &pentry->object.file.open_fd.fd; */
/*     } */

  return NULL;
}

/**
 *
 * cache_content_open: opens the local fd on  the cache.
 *
 * Opens the fd on  the FSAL
 *
 * @param pentry    [IN]  entry in file content layer whose content is to be accessed.
 * @param pclient   [IN]  ressource allocated by the client for the nfs management.
 * @param openflags [IN]  flags to be used to open the file
 * @param pcontent  [IN]  FSAL operation context
 * @pstatus         [OUT] returned status.
 *
 * @return CACHE_CONTENT_SUCCESS is successful .
 *
 */

cache_inode_status_t cache_inode_open(cache_entry_t * pentry,
                                      cache_inode_client_t * pclient,
                                      fsal_openflags_t openflags,
                                      struct user_cred *creds,
                                      cache_inode_status_t * pstatus)
{
  fsal_status_t fsal_status;
  fsal_accessflags_t access_type;

  if((pentry == NULL) || (pclient == NULL) || (pstatus == NULL))
    return CACHE_INODE_INVALID_ARGUMENT;

  if(pentry->internal_md.type != REGULAR_FILE)
    {
      *pstatus = CACHE_INODE_BAD_TYPE;
      return *pstatus;
    }

  /* access check but based on fsal_open_flags_t, not fsal_access_flags_t
   * this may be checked above but here is a last stop check.
   * Execute access not considered here.  Could fail execute opens.
   * FIXME: sort out access checks in callers.
   */
  access_type = (openflags == FSAL_O_RDWR) ? FSAL_R_OK : FSAL_W_OK;
  fsal_status = pentry->obj_handle->ops->test_access(pentry->obj_handle, creds, access_type);
  if(FSAL_IS_ERROR(fsal_status))
    {
      *pstatus = cache_inode_error_convert(fsal_status);

      LogDebug(COMPONENT_CACHE_INODE,
	       "cache_inode_open: returning %d(%s) from access check",
	       *pstatus, cache_inode_err_str(*pstatus));

      return *pstatus;
    }

      fsal_status = pentry->obj_handle->ops->open(pentry->obj_handle, openflags);

      if(FSAL_IS_ERROR(fsal_status))
        {
          *pstatus = cache_inode_error_convert(fsal_status);

          LogDebug(COMPONENT_CACHE_INODE,
                   "cache_inode_open: returning %d(%s) from FSAL_open",
                   *pstatus, cache_inode_err_str(*pstatus));

          return *pstatus;
        }

      LogFullDebug(COMPONENT_CACHE_INODE,
               "cache_inode_open: pentry %p: lastop=0, openflags = %d",
               pentry, (int) openflags);
  *pstatus = CACHE_INODE_SUCCESS;
  return *pstatus;

}                               /* cache_inode_open */

/**
 *
 * cache_content_open: opens the local fd on  the cache.
 *
 * Opens the fd on  the FSAL
 *
 * @param pentry_dir  [IN]  parent entry for the file
 * @param pname       [IN]  name of the file to be opened in the parent directory
 * @param pentry_file [IN]  file entry to be opened
 * @param pclient     [IN]  ressource allocated by the client for the nfs management.
 * @param openflags   [IN]  flags to be used to open the file
 * @param pcontent    [IN]  FSAL operation context
 * @pstatus           [OUT] returned status.
 *
 * @return CACHE_CONTENT_SUCCESS is successful .
 *
 */

/* FIXME: deprecate this
 */
cache_inode_status_t cache_inode_open_by_name(cache_entry_t * pentry_dir,
                                              fsal_name_t * pname,
                                              cache_entry_t * pentry_file,
                                              cache_inode_client_t * pclient,
                                              fsal_openflags_t openflags,
					      struct user_cred *creds,
                                              cache_inode_status_t * pstatus)
{
  fsal_status_t fsal_status;
  fsal_accessflags_t access_type;
  fsal_size_t save_filesize = 0;
  fsal_size_t save_spaceused = 0;
  fsal_time_t save_mtime = {
    .seconds = 0,
    .nseconds = 0
  };

  if((pentry_dir == NULL) || (pname == NULL) || (pentry_file == NULL) ||
     (pclient == NULL) || (pstatus == NULL))
    return CACHE_INODE_INVALID_ARGUMENT;

  if((pentry_dir->internal_md.type != DIRECTORY))
    {
      *pstatus = CACHE_INODE_BAD_TYPE;
      return *pstatus;
    }

  if(pentry_file->internal_md.type != REGULAR_FILE)
    {
      *pstatus = CACHE_INODE_BAD_TYPE;
      return *pstatus;
    }

  /* access check but based on fsal_open_flags_t, not fsal_access_flags_t
   * this may be checked above but here is a last stop check.
   * Execute access not considered here.  Could fail execute opens.
   * FIXME: sort out access checks in callers.
   */
  access_type = (openflags == FSAL_O_RDWR) ? FSAL_R_OK : FSAL_W_OK;
  fsal_status = pentry_file->obj_handle->ops->test_access(pentry_file->obj_handle,
						      creds, access_type);
  if(FSAL_IS_ERROR(fsal_status))
    {
      *pstatus = cache_inode_error_convert(fsal_status);

      LogDebug(COMPONENT_CACHE_INODE,
	       "cache_inode_open: returning %d(%s) from access check",
	       *pstatus, cache_inode_err_str(*pstatus));

      return *pstatus;
    }
      LogFullDebug(COMPONENT_FSAL,
               "cache_inode_open_by_name: pentry %p: lastop=0", pentry_file);

      /* Keep coherency with the cache_content */
      if(pentry_file->object.file.pentry_content != NULL)
        {
          save_filesize = pentry_file->obj_handle->attributes.filesize;
          save_spaceused = pentry_file->obj_handle->attributes.spaceused;
          save_mtime = pentry_file->obj_handle->attributes.mtime;
        }

      /* opened file is not preserved yet */
/* FIXME: I'm not sure why this is here.  The caller does a lookup to get the handle,
 * passing pname into the method.  Therefore, we *should* have the same thing!
 * for now, short circuit this with a simple open and refactor this whole function out
 * of nfs4_op_open and nfs41_op_open. also, let attributes bit slide for now...
 */
      fsal_status = pentry_dir->obj_handle->ops->open(pentry_file->obj_handle, openflags);
      if( !FSAL_IS_ERROR(fsal_status))
	 fsal_status = pentry_dir->obj_handle->ops->getattrs(pentry_file->obj_handle,
							     &(pentry_file->obj_handle->attributes));

      if(FSAL_IS_ERROR(fsal_status))
        {
          *pstatus = cache_inode_error_convert(fsal_status);

          LogDebug(COMPONENT_CACHE_INODE,
                   "cache_inode_open_by_name: returning %d(%s) from FSAL_open_by_name",
                   *pstatus, cache_inode_err_str(*pstatus));

          return *pstatus;
        }

      /* Keep coherency with the cache_content */
      if(pentry_file->object.file.pentry_content != NULL)
        {
          pentry_file->obj_handle->attributes.filesize = save_filesize;
          pentry_file->obj_handle->attributes.spaceused = save_spaceused;
          pentry_file->obj_handle->attributes.mtime = save_mtime;
        }

      LogFullDebug(COMPONENT_FSAL,
               "cache_inode_open_by_name: pentry %p",
               pentry_file);


  *pstatus = CACHE_INODE_SUCCESS;
  return *pstatus;

}                               /* cache_inode_open_by_name */

/**
 *
 * cache_inode_close: closes the local fd in the FSAL.
 *
 * Closes the local fd in the FSAL.
 *
 * No lock management is done in this layer: the related pentry in the cache inode layer is
 * locked and will prevent from concurent accesses.
 *
 * @param pentry  [IN] entry in file content layer whose content is to be accessed.
 * @param pclient [IN]  ressource allocated by the client for the nfs management.
 * @pstatus       [OUT] returned status.
 *
 * @return CACHE_CONTENT_SUCCESS is successful .
 *
 */
cache_inode_status_t cache_inode_close(cache_entry_t * pentry,
                                       cache_inode_client_t * pclient,
                                       cache_inode_status_t * pstatus)
{
  fsal_status_t fsal_status;

  if((pentry == NULL) || (pclient == NULL) || (pstatus == NULL))
    return CACHE_CONTENT_INVALID_ARGUMENT;

  if(pentry->internal_md.type != REGULAR_FILE)
    {
      *pstatus = CACHE_INODE_BAD_TYPE;
      return *pstatus;
    }

  /* if locks are held in the file, do not close */
  if( cache_inode_file_holds_state( pentry ) )
    {
      *pstatus = CACHE_INODE_SUCCESS; /** @todo : PhD : May be CACHE_INODE_STATE_CONFLICTS would be better ? */
      return *pstatus;
    }
  /** @TODO used to be file aging.  that is in the fsal now so close unconditionally for now..*/
  if(pclient->use_fd_cache == 0)
    {

      LogFullDebug(COMPONENT_CACHE_INODE,
               "cache_inode_close: pentry %p",
               pentry);

      fsal_status = pentry->obj_handle->ops->close(pentry->obj_handle);

      if(FSAL_IS_ERROR(fsal_status) && (fsal_status.major != ERR_FSAL_NOT_OPENED))
        {
          *pstatus = cache_inode_error_convert(fsal_status);

          LogDebug(COMPONENT_CACHE_INODE,
                   "cache_inode_close: returning %d(%s) from FSAL_close",
                   *pstatus, cache_inode_err_str(*pstatus));

          return *pstatus;
        }
    }
  *pstatus = CACHE_CONTENT_SUCCESS;

  return *pstatus;
}                               /* cache_content_close */
