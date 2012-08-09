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
 * \file    9p_xattrwalk.c
 * \brief   9P version
 *
 * 9p_xattrwalk.c : _9P_interpretor, request XATTRWALK
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
#include <sys/stat.h>
#include <sys/types.h>
#include <attr/xattr.h>
#include "nfs_core.h"
#include "log.h"
#include "cache_inode.h"
#include "fsal.h"
#include "9p.h"


int _9p_xattrwalk( _9p_request_data_t * preq9p, 
                   void  * pworker_data,
                   u32 * plenout, 
                   char * preply)
{
  char * cursor = preq9p->_9pmsg + _9P_HDR_SIZE + _9P_TYPE_SIZE ;
  u8   * pmsgtype =  preq9p->_9pmsg + _9P_HDR_SIZE ;
  nfs_worker_data_t * pwkrdata = (nfs_worker_data_t *)pworker_data ;

  u16 * msgtag = NULL ;
  u32 * fid    = NULL ;
  u32 * attrfid = NULL ;
  u16 * name_len ;
  char * name_str ;
  u64 attrsize = 0LL ;

  fsal_status_t fsal_status ; 
  fsal_name_t name;
  fsal_xattrent_t xattrs_tab[255];
  int eod_met = FALSE;
  unsigned int nb_xattrs_read = 0;
  unsigned int i = 0 ; 
  char * xattr_cursor = NULL ;
  unsigned int tmplen = 0 ;

  _9p_fid_t * pfid = NULL ;
  _9p_fid_t * pxattrfid = NULL ;

  if ( !preq9p || !pworker_data || !plenout || !preply )
   return -1 ;

  /* Get data */
  _9p_getptr( cursor, msgtag, u16 ) ; 
  _9p_getptr( cursor, fid,    u32 ) ; 
  _9p_getptr( cursor, attrfid, u32 ) ; 

  LogDebug( COMPONENT_9P, "TXATTRWALK: tag=%u fid=%u attrfid=%u" ,
            (u32)*msgtag, *fid, *attrfid ) ;

  _9p_getstr( cursor, name_len, name_str ) ;
  LogDebug( COMPONENT_9P, "TXATTRWALK (component): tag=%u fid=%u attrfid=%u name=%.*s",
            (u32)*msgtag, *fid, *attrfid, *name_len, name_str ) ;

  if( *fid >= _9P_FID_PER_CONN )
    return  _9p_rerror( preq9p, pworker_data,  msgtag, ERANGE, plenout, preply ) ;

  if( *attrfid >= _9P_FID_PER_CONN )
    return  _9p_rerror( preq9p, pworker_data,  msgtag, ERANGE, plenout, preply ) ;
 
  pfid = &preq9p->pconn->fids[*fid] ;
  /* Check that it is a valid fid */
  if (pfid->pentry == NULL) 
  {
    LogDebug( COMPONENT_9P, "request on invalid fid=%u", *fid ) ;
    return  _9p_rerror( preq9p, pworker_data,  msgtag, EIO, plenout, preply ) ;
  }

  pxattrfid = &preq9p->pconn->fids[*attrfid] ;
  /* Check that it is a free fid */
  if (pxattrfid->pentry != NULL) 
  {
    LogDebug( COMPONENT_9P, "request on invalid in-use fid=%u", *attrfid ) ;
    return  _9p_rerror( preq9p, pworker_data,  msgtag, EIO, plenout, preply ) ;
  }

  /* Initiate xattr's fid by copying file's fid in it */
  memcpy( (char *)pxattrfid, (char *)pfid, sizeof( _9p_fid_t ) ) ;

  snprintf( name.name, FSAL_MAX_NAME_LEN, "%.*s", *name_len, name_str ) ;
  name.len = *name_len + 1 ;

  if( ( pxattrfid->specdata.xattr.xattr_content = gsh_malloc( XATTR_BUFFERSIZE ) ) == NULL ) 
    return  _9p_rerror( preq9p, pworker_data,  msgtag, ENOMEM, plenout, preply ) ;

  if( *name_len == 0 ) 
   {
      /* xattrwalk is used with an empty name, this is a listxattr request */
      fsal_status = FSAL_ListXAttrs( &pxattrfid->pentry->handle,
                                     FSAL_XATTR_RW_COOKIE, /* Start with RW cookie, hiding RO ones */
                                     &pxattrfid->fsal_op_context,
                                     xattrs_tab,
                                     100, /* for wanting of something smarter */  
                                     &nb_xattrs_read, 
                                     &eod_met);

      if(FSAL_IS_ERROR(fsal_status))
        return  _9p_rerror( preq9p, pworker_data,  msgtag, _9p_tools_errno( cache_inode_error_convert(fsal_status) ), plenout, preply ) ;

      /* if all xattrent are not read, returns ERANGE as listxattr does */
      if( eod_met != TRUE )
        return  _9p_rerror( preq9p, pworker_data,  msgtag, ERANGE, plenout, preply ) ;
     
      xattr_cursor = pxattrfid->specdata.xattr.xattr_content ; 
      attrsize = 0LL ; 
      for( i = 0 ; i < nb_xattrs_read ; i++ )
       {
         tmplen = snprintf( xattr_cursor, MAXNAMLEN, "%s", xattrs_tab[i].xattr_name.name ) ; 
         xattr_cursor[tmplen] = '\0' ; /* Just to be sure */
         xattr_cursor += tmplen+1 ; /* Do not forget to take in account the '\0' at the end */
         attrsize += tmplen+1 ;

         /* Make sure not to go beyond the buffer */
         if( attrsize > XATTR_BUFFERSIZE ) 
           return  _9p_rerror( preq9p, pworker_data,  msgtag, ERANGE, plenout, preply ) ;
       }
   }
  else
   {
      /* xattrwalk has a non-empty name, use regular setxattr */
      fsal_status = FSAL_GetXAttrIdByName( &pxattrfid->pentry->handle,
                                           &name, 
                                           &pxattrfid->fsal_op_context,
                                           &pxattrfid->specdata.xattr.xattr_id);
      if(FSAL_IS_ERROR(fsal_status))
       {
         if( fsal_status.major == ERR_FSAL_NOENT ) /* ENOENT for xattr is ENOATTR (set setxattr's manpage) */
           return  _9p_rerror( preq9p, pworker_data,  msgtag, ENOATTR, plenout, preply ) ;
         else 
           return  _9p_rerror( preq9p, pworker_data,  msgtag, _9p_tools_errno( cache_inode_error_convert(fsal_status) ), plenout, preply ) ;
       }

      fsal_status = FSAL_GetXAttrValueByName( &pxattrfid->pentry->handle,
                                              &name, 
                                              &pxattrfid->fsal_op_context,
                                              pxattrfid->specdata.xattr.xattr_content, 
                                              XATTR_BUFFERSIZE, 
                                              &attrsize );

      if(FSAL_IS_ERROR(fsal_status))
        return  _9p_rerror( preq9p, pworker_data,  msgtag, _9p_tools_errno( cache_inode_error_convert(fsal_status) ), plenout, preply ) ;

      _9p_chomp_attr_value( pxattrfid->specdata.xattr.xattr_content, strlen(  pxattrfid->specdata.xattr.xattr_content) ) ;

      attrsize = strlen( pxattrfid->specdata.xattr.xattr_content ) ;
   }

  /* Build the reply */
  _9p_setinitptr( cursor, preply, _9P_RXATTRWALK ) ;
  _9p_setptr( cursor, msgtag, u16 ) ;

  _9p_setvalue( cursor, attrsize, u64 ) ; /* No xattr for now */

  _9p_setendptr( cursor, preply ) ;
  _9p_checkbound( cursor, preply, plenout ) ;

  LogDebug( COMPONENT_9P, "RXATTRWALK: tag=%u fid=%u attrfid=%u name=%.*s size=%llu",
            (u32)*msgtag, *fid, *attrfid,  *name_len, name_str, (unsigned long long)attrsize ) ;

  _9p_stat_update( *pmsgtype, TRUE, &pwkrdata->stats._9p_stat_req ) ;
  return 1 ;
} /* _9p_xattrwalk */

