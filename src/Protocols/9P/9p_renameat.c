/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright CEA/DAM/DIF  (2011)
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
 * \file    9p_renameat.c
 * \brief   9P version
 *
 * 9p_renameat.c : _9P_interpretor, request ATTACH
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
#include "nfs_core.h"
#include "log.h"
#include "cache_inode.h"
#include "fsal.h"
#include "9p.h"

int _9p_renameat( _9p_request_data_t * preq9p, 
                void  * pworker_data,
                u32 * plenout, 
                char * preply)
{
  char * cursor = preq9p->_9pmsg + _9P_HDR_SIZE + _9P_TYPE_SIZE ;
  u8   * pmsgtype =  preq9p->_9pmsg + _9P_HDR_SIZE ;
  nfs_worker_data_t * pwkrdata = (nfs_worker_data_t *)pworker_data ;

  u16  * msgtag      = NULL ;
  u32  * oldfid      = NULL ;
  u16  * oldname_len = NULL ;
  char * oldname_str = NULL ;
  u32  * newfid      = NULL ;
  u16  * newname_len = NULL ;
  char * newname_str = NULL ;

  _9p_fid_t * poldfid = NULL ;
  _9p_fid_t * pnewfid = NULL ;

  fsal_attrib_list_t    oldfsalattr ;
  fsal_attrib_list_t    newfsalattr ;

  cache_inode_status_t  cache_status ;

  fsal_name_t           oldname ;
  fsal_name_t           newname ;

  if ( !preq9p || !pworker_data || !plenout || !preply )
   return -1 ;

  /* Get data */
  _9p_getptr( cursor, msgtag, u16 ) ; 

  _9p_getptr( cursor, oldfid,   u32 ) ; 
  _9p_getstr( cursor, oldname_len, oldname_str ) ;
  _9p_getptr( cursor, newfid,   u32 ) ; 
  _9p_getstr( cursor, newname_len, newname_str ) ;

  LogDebug( COMPONENT_9P, "TRENAMEAT: tag=%u oldfid=%u oldname=%.*s newfid=%u newname=%.*s",
            (u32)*msgtag, *oldfid, *oldname_len, oldname_str, *newfid, *newname_len, newname_str ) ;

  if( *oldfid >= _9P_FID_PER_CONN )
   return  _9p_rerror( preq9p, pworker_data,  msgtag, ERANGE, plenout, preply ) ;

  poldfid = &preq9p->pconn->fids[*oldfid] ;

  /* Check that it is a valid fid */
  if (poldfid->pentry == NULL) 
  {
    LogDebug( COMPONENT_9P, "request on invalid fid=%u", *oldfid ) ;
    return  _9p_rerror( preq9p, pworker_data,  msgtag, EIO, plenout, preply ) ;
  }

  if( *newfid >= _9P_FID_PER_CONN )
   return  _9p_rerror( preq9p, pworker_data,  msgtag, ERANGE, plenout, preply ) ;

  pnewfid = &preq9p->pconn->fids[*newfid] ;

  /* Check that it is a valid fid */
  if (pnewfid->pentry == NULL) 
  {
    LogDebug( COMPONENT_9P, "request on invalid fid=%u", *newfid ) ;
    return  _9p_rerror( preq9p, pworker_data,  msgtag, EIO, plenout, preply ) ;
  }

  /* Let's do the job */
  snprintf( oldname.name, FSAL_MAX_NAME_LEN, "%.*s", *oldname_len, oldname_str ) ;
  oldname.len = *oldname_len + 1 ;
  snprintf( newname.name, FSAL_MAX_NAME_LEN, "%.*s", *newname_len, newname_str ) ;
  newname.len = *newname_len + 1 ;

  if( cache_inode_rename( poldfid->pentry,
                          &oldname,
                          pnewfid->pentry,
                          &newname,
                          &oldfsalattr,
                          &newfsalattr,
                          &poldfid->fsal_op_context, 
                          &cache_status) != CACHE_INODE_SUCCESS )
    return  _9p_rerror( preq9p, pworker_data,  msgtag, _9p_tools_errno( cache_status ), plenout, preply ) ;

  /* Build the reply */
  _9p_setinitptr( cursor, preply, _9P_RRENAMEAT ) ;
  _9p_setptr( cursor, msgtag, u16 ) ;

  _9p_setendptr( cursor, preply ) ;
  _9p_checkbound( cursor, preply, plenout ) ;

  LogDebug( COMPONENT_9P, "RRENAMEAT: tag=%u oldfid=%u oldname=%.*s newfid=%u newname=%.*s",
            (u32)*msgtag, *oldfid, *oldname_len, oldname_str, *newfid, *newname_len, newname_str ) ;

  _9p_stat_update( *pmsgtype, TRUE, &pwkrdata->stats._9p_stat_req ) ;
  return 1 ;
}

