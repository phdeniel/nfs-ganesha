/*
 * Copyright CEA/DAM/DIF  2010
 *  Author: Philippe Deniel (philippe.deniel@cea.fr)
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
#include <fcntl.h>
#include <sys/file.h>           /* for having FNDELAY */
#include <sys/quota.h>          /* For USRQUOTA */
#include "HashData.h"
#include "HashTable.h"
#include "rpc.h"
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
#include "rquota.h"
#include "nfs_proto_functions.h"

/**
 * rquota_setquota: The Rquota setquota function, for all versions.
 *
 * The RQUOTA setquota function, for all versions.
 *
 *  @param parg        [IN]    ignored
 *  @param pexportlist [IN]    ignored
 *  @param pcontextp   [IN]    ignored
 *  @param pclient     [INOUT] ignored
 *  @param ht          [INOUT] ignored
 *  @param preq        [IN]    ignored
 *  @param pres        [OUT]   ignored
 *
 */
int rquota_setquota(nfs_arg_t * parg /* IN     */ ,
                    exportlist_t * pexport /* IN     */ ,
                    fsal_op_context_t * pcontext /* IN     */ ,
                    cache_inode_client_t * pclient /* INOUT  */ ,
                    hash_table_t * ht /* INOUT  */ ,
                    struct svc_req *preq /* IN     */ ,
                    nfs_res_t * pres /* OUT    */ )
{
  fsal_status_t fsal_status;
  fsal_quota_t fsal_quota_in;
  fsal_quota_t fsal_quota_out;
  fsal_path_t fsal_path;
  int quota_type = USRQUOTA;
  int quota_id;
  char work[MAXPATHLEN];

  LogFullDebug(COMPONENT_NFSPROTO,
               "REQUEST PROCESSING: Calling rquota_setquota");

  if(preq->rq_vers == EXT_RQUOTAVERS)
    {
      quota_type = parg->arg_ext_rquota_getquota.gqa_type;
      quota_id = parg->arg_ext_rquota_getquota.gqa_id;
    }
  else
    {
      quota_type = USRQUOTA;
      quota_id = parg->arg_rquota_getquota.gqa_uid;
    }

  if(parg->arg_rquota_getquota.gqa_pathp[0] == '/')
    strncpy(work, parg->arg_rquota_getquota.gqa_pathp, MAXPATHLEN);
  else
    {
      if(nfs_export_tag2path(nfs_param.pexportlist,
                             parg->arg_rquota_getquota.gqa_pathp,
                             strnlen(parg->arg_rquota_getquota.gqa_pathp, MAXPATHLEN),
                             work, MAXPATHLEN) == -1)

        {
          pres->res_rquota_getquota.status = Q_EPERM;
          return NFS_REQ_OK;
        }
    }

  if(FSAL_IS_ERROR((fsal_status = FSAL_str2path(work, MAXPATHLEN, &fsal_path))))
    {
      pres->res_rquota_setquota.status = Q_EPERM;
      return NFS_REQ_OK;
    }

  memset((char *)&fsal_quota_in, 0, sizeof(fsal_quota_t));
  memset((char *)&fsal_quota_out, 0, sizeof(fsal_quota_t));

  fsal_quota_in.bhardlimit = parg->arg_rquota_setquota.sqa_dqblk.rq_bhardlimit;
  fsal_quota_in.bsoftlimit = parg->arg_rquota_setquota.sqa_dqblk.rq_bsoftlimit;
  fsal_quota_in.curblocks = parg->arg_rquota_setquota.sqa_dqblk.rq_curblocks;
  fsal_quota_in.fhardlimit = parg->arg_rquota_setquota.sqa_dqblk.rq_fhardlimit;
  fsal_quota_in.fsoftlimit = parg->arg_rquota_setquota.sqa_dqblk.rq_fsoftlimit;
  fsal_quota_in.btimeleft = parg->arg_rquota_setquota.sqa_dqblk.rq_btimeleft;
  fsal_quota_in.ftimeleft = parg->arg_rquota_setquota.sqa_dqblk.rq_ftimeleft;

  fsal_status = FSAL_set_quota(&fsal_path,
                               quota_type, quota_id, &fsal_quota_in, &fsal_quota_out);
  if(FSAL_IS_ERROR(fsal_status))
    {
      if(fsal_status.major == ERR_FSAL_NO_QUOTA)
        pres->res_rquota_setquota.status = Q_NOQUOTA;
      else
        pres->res_rquota_setquota.status = Q_EPERM;
      return NFS_REQ_OK;
    }

  /* is success */
  pres->res_rquota_getquota.status = Q_OK;

  pres->res_rquota_setquota.setquota_rslt_u.sqr_rquota.rq_active = TRUE;
  pres->res_rquota_setquota.setquota_rslt_u.sqr_rquota.rq_bhardlimit =
      fsal_quota_out.bhardlimit;
  pres->res_rquota_setquota.setquota_rslt_u.sqr_rquota.rq_bsoftlimit =
      fsal_quota_out.bsoftlimit;
  pres->res_rquota_setquota.setquota_rslt_u.sqr_rquota.rq_curblocks =
      fsal_quota_out.curblocks;
  pres->res_rquota_setquota.setquota_rslt_u.sqr_rquota.rq_fhardlimit =
      fsal_quota_out.fhardlimit;
  pres->res_rquota_setquota.setquota_rslt_u.sqr_rquota.rq_fsoftlimit =
      fsal_quota_out.fsoftlimit;
  pres->res_rquota_setquota.setquota_rslt_u.sqr_rquota.rq_btimeleft =
      fsal_quota_out.btimeleft;
  pres->res_rquota_setquota.setquota_rslt_u.sqr_rquota.rq_ftimeleft =
      fsal_quota_out.ftimeleft;

  return NFS_REQ_OK;

  return 0;
}                               /* rquota_setquota */

/**
 * rquota_setquota_Free: Frees the result structure allocated for rquota_setquota
 *
 * Frees the result structure allocated for rquota_setquota. Does Nothing in fact.
 *
 * @param pres        [INOUT]   Pointer to the result structure.
 *
 */
void rquota_setquota_Free(nfs_res_t * pres)
{
  return;
}
