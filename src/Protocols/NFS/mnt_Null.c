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
 * \file    mnt_Null.c
 * \author  $Author: deniel $
 * \date    $Date: 2005/12/20 10:52:14 $
 * \version $Revision: 1.8 $
 * \brief   MOUNTPROC_NULL for Mount protocol v1 and v3.
 *
 * mnt_Null.c : MOUNTPROC_NULL in V1, V3.
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
#include <fcntl.h>
#include <sys/file.h>           /* for having FNDELAY */
#include "HashData.h"
#include "HashTable.h"
#include "log.h"
#include "stuff_alloc.h"
#include "nfs23.h"
#include "nfs4.h"
#include "nfs_core.h"
#include "cache_inode.h"
#include "cache_content.h"
#include "nfs_exports.h"
#include "nfs_creds.h"
#include "nfs_tools.h"
#include "mount.h"
#include "nfs_proto_functions.h"

/**
 * mnt_Null: The Mount proc null function, for all versions.
 * 
 * The MOUNT proc null function, for all versions.
 * 
 *  @param parg        [IN]    ignored
 *  @param pexportlist [IN]    ignored
 *	@param pcontextp      [IN]    ignored
 *  @param pclient     [INOUT] ignored
 *  @param ht          [INOUT] ignored
 *  @param preq        [IN]    ignored 
 *	@param pres        [OUT]   ignored
 *
 */

int mnt_Null(nfs_arg_t * parg /* IN     */ ,
             exportlist_t * pexport /* IN     */ ,
             fsal_op_context_t * pcontext /* IN     */ ,
             cache_inode_client_t * pclient /* INOUT  */ ,
             hash_table_t * ht /* INOUT  */ ,
             struct svc_req *preq /* IN     */ ,
             nfs_res_t * pres /* OUT    */ )
{
  LogDebug(COMPONENT_NFSPROTO, "REQUEST PROCESSING: Calling mnt_Null");
  return MNT3_OK;
}                               /* mnt_Null */

/**
 * mnt_Null_Free: Frees the result structure allocated for mnt_Null
 * 
 * Frees the result structure allocated for mnt_Null. Does Nothing in fact.
 * 
 * @param pres        [INOUT]   Pointer to the result structure.
 *
 */
void mnt_Null_Free(nfs_res_t * pres)
{
  return;
}                               /* mnt_Export_Free */
