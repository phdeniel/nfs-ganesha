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
 * \file    nfs4_op_openattr.c
 * \author  $Author: deniel $
 * \date    $Date: 2005/11/28 17:02:51 $
 * \version $Revision: 1.8 $
 * \brief   Routines used for managing the NFS4 COMPOUND functions.
 *
 * nfs4_op_openattr.c : Routines used for managing the NFS4 COMPOUND functions.
 *
 * $Header: /cea/home/cvs/cvs/SHERPA/BaseCvs/GANESHA/src/NFS_Protocols/nfs4_op_openattr.c,v 1.8 2005/11/28 17:02:51 deniel Exp $
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
#include "rpc.h"
#include "log.h"
#include "stuff_alloc.h"
#include "nfs23.h"
#include "nfs4.h"
#include "mount.h"
#include "nfs_core.h"
#include "cache_inode.h"
#include "cache_content.h"
#include "nfs_exports.h"
#include "nfs_creds.h"
#include "nfs_proto_functions.h"
#include "nfs_tools.h"

/**
 *
 * nfs4_op_openattr: Implemtation of NFS4_OP_OPENATTR
 * 
 * Implementation of NFS4_OP_OPENATTRR.
 *
 * @param op    [IN]    pointer to nfs4_op arguments
 * @param data  [INOUT] Pointer to the compound request's data
 * @param resp  [IN]    Pointer to nfs4_op results
 * 
 * @return NFS4_OK 
 * 
 */
#define arg_OPENATTR4 op->nfs_argop4_u.opopenattr
#define res_OPENATTR4 resp->nfs_resop4_u.opopenattr

int nfs4_op_openattr(struct nfs_argop4 *op,
                     compound_data_t * data, struct nfs_resop4 *resp)
{
  char __attribute__ ((__unused__)) funcname[] = "nfs4_op_openattr";

  resp->resop = NFS4_OP_OPENATTR;
  res_OPENATTR4.status = NFS4_OK;

  res_OPENATTR4.status = nfs4_fh_to_xattrfh(&(data->currentFH), &(data->currentFH));

  return res_OPENATTR4.status;
}                               /* nfs4_op_openattr */

/**
 * nfs4_op_openattr_Free: frees what was allocared to handle nfs4_op_openattr.
 * 
 * Frees what was allocared to handle nfs4_op_openattr.
 *
 * @param resp  [INOUT]    Pointer to nfs4_op results
 *
 * @return nothing (void function )
 * 
 */
void nfs4_op_openattr_Free(OPENATTR4res * resp)
{
  /* Nothing to be done */
  return;
}                               /* nfs4_op_openattr_Free */
