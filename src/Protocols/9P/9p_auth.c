/*
 * vim:noexpandtab:shiftwidth=8:tabstop=8:
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
 * Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * ---------------------------------------
 */

/**
 * \file    9p_attach.c
 * \brief   9P version
 *
 * 9p_attach.c : _9P_interpretor, request ATTACH
 *
 *
 */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "nfs_core.h"
#include "export_mgr.h"
#include "log.h"
#include "cache_inode.h"
#include "fsal.h"
#include "9p.h"

int _9p_auth(struct _9p_request_data *req9p, void *worker_data,
	     u32 *plenout, char *preply)
{
	char *cursor = req9p->_9pmsg + _9P_HDR_SIZE + _9P_TYPE_SIZE;
	u16 *msgtag = NULL;
	u32 *afid = NULL;
	u16 *uname_len = NULL;
	char *uname_str = NULL;
	u16 *aname_len = NULL;
	char *aname_str = NULL;
	u32 *n_aname = NULL;

	/* Get data */
	_9p_getptr(cursor, msgtag, u16);
	_9p_getptr(cursor, afid, u32);
	_9p_getstr(cursor, uname_len, uname_str);
	_9p_getstr(cursor, aname_len, aname_str);
	_9p_getptr(cursor, n_aname, u32);

	LogDebug(COMPONENT_9P,
		 "TAUTH: tag=%u afid=%d uname='%.*s' aname='%.*s' n_uname=%d",
		 (u32) *msgtag, *afid, (int) *uname_len, uname_str,
		 (int) *aname_len, aname_str, *n_aname);

	if (*afid >= _9P_FID_PER_CONN)
		return _9p_rerror(req9p, worker_data, msgtag, ERANGE, plenout,
				  preply);
#ifdef USE_SELINUX
	struct _9p_fid *pfid = NULL;

	u32 err = 0;

	/* Set export and fid id in fid */
	pfid = gsh_calloc(1, sizeof(struct _9p_fid));
	if (pfid == NULL) {
		err = ENOMEM;
		goto errout;
	}

	pfid->selinux.scon = gsh_malloc(*uname_len);
	if (pfid->selinux.scon == NULL) {
		err = ENOMEM;
		goto errout;
	}
	memcpy(pfid->selinux.scon, uname_str, (size_t)(*uname_len));
	pfid->selinux.scon_size = *uname_len;

	pfid->fid = *afid;
	req9p->pconn->fids[*afid] = pfid;

	/* Set type of fid */
	pfid->fid_type = _9P_FID_AUTH_SELINUX;

	/* Compute the qid */
	pfid->qid.type = _9P_QTDIR;
	pfid->qid.version = 0;	/* No cache, we want the client
				 * to stay synchronous with the server */

	/** @todo do the actual fileid computation here */
	pfid->qid.path = 12;

	/* Build the reply */
	_9p_setinitptr(cursor, preply, _9P_RAUTH);
	_9p_setptr(cursor, msgtag, u16);

	_9p_setqid(cursor, pfid->qid);

	_9p_setendptr(cursor, preply);
	_9p_checkbound(cursor, preply, plenout);

	LogDebug(COMPONENT_9P,
		 "RAUTH: tag=%u afid=%u qid=(type=%u,version=%u,path=%llu)",
		 *msgtag, *afid, (u32) pfid->qid.type, pfid->qid.version,
		 (unsigned long long)pfid->qid.path);

	/* Everything is OK */
	return 1;

errout:
	if (pfid->selinux.scon)
		gsh_free(pfid->selinux.scon);

	if (pfid)
		gsh_free(pfid);

	return _9p_rerror(req9p, worker_data, msgtag, err, plenout, preply);
#else
	return _9p_rerror(req9p, worker_data, msgtag, ENOTSUP,
			  plenout, preply);
#endif

}
