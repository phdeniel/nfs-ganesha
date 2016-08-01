/*
 * vim:noexpandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright (C) Panasas Inc., 2011
 * Author: Jim Lieb jlieb@panasas.com
 *
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * -------------
 */

/* file.c
 * File I/O methods for ZFS module
 */

#include "config.h"

#include <assert.h>
#include "fsal.h"
#include "fsal_internal.h"
#include "FSAL/access_check.h"
#include "fsal_convert.h"
#include <unistd.h>
#include <fcntl.h>
#include "FSAL/fsal_commonlib.h"
#include "kvsfs_methods.h"
#include <stdbool.h>

/** kvsfs_open
 * called with appropriate locks taken at the cache inode level
 */

fsal_status_t kvsfs_open(struct fsal_obj_handle *obj_hdl,
			fsal_openflags_t openflags)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/* kvsfs_status
 * Let the caller peek into the file's open/close state.
 */

fsal_openflags_t kvsfs_status(struct fsal_obj_handle *obj_hdl)
{
	struct kvsfs_fsal_obj_handle *myself;

	myself = container_of(obj_hdl,
			      struct kvsfs_fsal_obj_handle,
			      obj_handle);
	return myself->u.file.openflags;
}

/* kvsfs_read
 * concurrency (locks) is managed in cache_inode_*
 */

fsal_status_t kvsfs_read(struct fsal_obj_handle *obj_hdl,
			uint64_t offset,
			size_t buffer_size, void *buffer, size_t *read_amount,
			bool *end_of_file)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/* kvsfs_write
 * concurrency (locks) is managed in cache_inode_*
 */

fsal_status_t kvsfs_write(struct fsal_obj_handle *obj_hdl,
			 uint64_t offset,
			 size_t buffer_size, void *buffer,
			 size_t *write_amount, bool *fsal_stable)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/* kvsfs_commit
 * Commit a file range to storage.
 * for right now, fsync will have to do.
 */

fsal_status_t kvsfs_commit(struct fsal_obj_handle *obj_hdl,	/* sync */
			  off_t offset, size_t len)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/* kvsfs_close
 * Close the file if it is still open.
 * Yes, we ignor lock status.  Closing a file in POSIX
 * releases all locks but that is state and cache inode's problem.
 */

fsal_status_t kvsfs_close(struct fsal_obj_handle *obj_hdl)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/* kvsfs_lru_cleanup
 * free non-essential resources at the request of cache inode's
 * LRU processing identifying this handle as stale enough for resource
 * trimming.
 */

fsal_status_t kvsfs_lru_cleanup(struct fsal_obj_handle *obj_hdl,
			       lru_actions_t requests)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

fsal_status_t kvsfs_lock_op(struct fsal_obj_handle *obj_hdl,
			   void *p_owner,
			   fsal_lock_op_t lock_op,
			   fsal_lock_param_t *request_lock,
			   fsal_lock_param_t *conflicting_lock)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}
