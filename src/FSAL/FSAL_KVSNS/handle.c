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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * -------------
 */

/* handle.c
 * ZFS object (file|dir) handle object
 */

#include "config.h"

#include <libgen.h>		/* used for 'dirname' */
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <mntent.h>
#include "gsh_list.h"
#include "fsal.h"
#include "fsal_internal.h"
#include "fsal_convert.h"
#include "FSAL/fsal_config.h"
#include "FSAL/fsal_commonlib.h"
#include "kvsfs_methods.h"
#include <stdbool.h>

/* helpers
 */

/* alloc_handle
 * allocate and fill in a handle
 * this uses malloc/free for the time being.
 */
#if 0
static struct kvsfs_fsal_obj_handle *alloc_handle(struct kvsfs_file_handle *fh,
						struct stat *stat,
						const char *link_content,
						struct fsal_export *exp_hdl)
{
	struct kvsfs_fsal_obj_handle *hdl;

	hdl = gsh_malloc(sizeof(struct kvsfs_fsal_obj_handle) +
			 sizeof(struct kvsfs_file_handle));

	memset(hdl, 0,
	       (sizeof(struct kvsfs_fsal_obj_handle) +
		sizeof(struct kvsfs_file_handle)));
	hdl->handle = (struct kvsfs_file_handle *)&hdl[1];
	memcpy(hdl->handle, fh, sizeof(struct kvsfs_file_handle));

	hdl->obj_handle.attrs = &hdl->attributes;
	hdl->obj_handle.type = posix2fsal_type(stat->st_mode);

	if ((hdl->obj_handle.type == SYMBOLIC_LINK) &&
	    (link_content != NULL)) {
		size_t len = strlen(link_content) + 1;

		hdl->u.symlink.link_content = gsh_malloc(len);
		memcpy(hdl->u.symlink.link_content, link_content, len);
		hdl->u.symlink.link_size = len;
	}

	hdl->attributes.mask = exp_hdl->exp_ops.fs_supported_attrs(exp_hdl);

	posix2fsal_attributes(stat, &hdl->attributes);

	fsal_obj_handle_init(&hdl->obj_handle,
			     exp_hdl,
			     posix2fsal_type(stat->st_mode));
	kvsfs_handle_ops_init(&hdl->obj_handle.obj_ops);
	return hdl;
}
#endif

/* handle methods
 */

/* lookup
 * deprecated NULL parent && NULL path implies root handle
 */

static fsal_status_t kvsfs_lookup(struct fsal_obj_handle *parent,
				 const char *path,
				 struct fsal_obj_handle **handle)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/* lookup_path
 * should not be used for "/" only is exported */

fsal_status_t kvsfs_lookup_path(struct fsal_export *exp_hdl,
			       const char *path,
			       struct fsal_obj_handle **handle)
{
	if (strcmp(path, "/"))
		return fsalstat(ERR_FSAL_NOTSUPP, 0);

	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/* create
 * create a regular file and set its attributes
 */

static fsal_status_t kvsfs_create(struct fsal_obj_handle *dir_hdl,
				 const char *name, struct attrlist *attrib,
				 struct fsal_obj_handle **handle)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

static fsal_status_t kvsfs_mkdir(struct fsal_obj_handle *dir_hdl,
				const char *name, struct attrlist *attrib,
				struct fsal_obj_handle **handle)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

static fsal_status_t kvsfs_makenode(struct fsal_obj_handle *dir_hdl,
				   const char *name,
				   object_file_type_t nodetype,	/* IN */
				   fsal_dev_t *dev,	/* IN */
				   struct attrlist *attrib,
				   struct fsal_obj_handle **handle)
{
	return fsalstat(ERR_FSAL_NOTSUPP, 0);
}

/** makesymlink
 *  Note that we do not set mode bits on symlinks for Linux/POSIX
 *  They are not really settable in the kernel and are not checked
 *  anyway (default is 0777) because open uses that target's mode
 */

static fsal_status_t kvsfs_makesymlink(struct fsal_obj_handle *dir_hdl,
				      const char *name, const char *link_path,
				      struct attrlist *attrib,
				      struct fsal_obj_handle **handle)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

static fsal_status_t kvsfs_readsymlink(struct fsal_obj_handle *obj_hdl,
				      struct gsh_buffdesc *link_content,
				      bool refresh)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

static fsal_status_t kvsfs_linkfile(struct fsal_obj_handle *obj_hdl,
				   struct fsal_obj_handle *destdir_hdl,
				   const char *name)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

#define MAX_ENTRIES 256
/**
 * read_dirents
 * read the directory and call through the callback function for
 * each entry.
 * @param dir_hdl [IN] the directory to read
 * @param entry_cnt [IN] limit of entries. 0 implies no limit
 * @param whence [IN] where to start (next)
 * @param dir_state [IN] pass thru of state to callback
 * @param cb [IN] callback function
 * @param eof [OUT] eof marker true == end of dir
 */
static fsal_status_t kvsfs_readdir(struct fsal_obj_handle *dir_hdl,
				  fsal_cookie_t *whence, void *dir_state,
				  fsal_readdir_cb cb, bool *eof)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

static fsal_status_t kvsfs_rename(struct fsal_obj_handle *obj_hdl,
				 struct fsal_obj_handle *olddir_hdl,
				 const char *old_name,
				 struct fsal_obj_handle *newdir_hdl,
				 const char *new_name)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/* FIXME: attributes are now merged into fsal_obj_handle.  This
 * spreads everywhere these methods are used.  eventually deprecate
 * everywhere except where we explicitly want to to refresh them.
 * NOTE: this is done under protection of the attributes rwlock in the
 * cache entry.
 */

static fsal_status_t kvsfs_getattrs(struct fsal_obj_handle *obj_hdl)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/*
 * NOTE: this is done under protection of the attributes rwlock
 * in the cache entry.
 */

static fsal_status_t kvsfs_setattrs(struct fsal_obj_handle *obj_hdl,
				   struct attrlist *attrs)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/* file_unlink
 * unlink the named file in the directory
 */
static fsal_status_t kvsfs_unlink(struct fsal_obj_handle *dir_hdl,
				 const char *name)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/* handle_digest
 * fill in the opaque f/s file handle part.
 * we zero the buffer to length first.  This MAY already be done above
 * at which point, remove memset here because the caller is zeroing
 * the whole struct.
 */

static fsal_status_t kvsfs_handle_digest(const struct fsal_obj_handle *obj_hdl,
					fsal_digesttype_t output_type,
					struct gsh_buffdesc *fh_desc)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

void kvsfs_handle_ops_init(struct fsal_obj_ops *ops)
{
	/** ops->release = release; @todo */
	ops->lookup = kvsfs_lookup;
	ops->readdir = kvsfs_readdir;
	ops->create = kvsfs_create;
	ops->mkdir = kvsfs_mkdir;
	ops->mknode = kvsfs_makenode;
	ops->symlink = kvsfs_makesymlink;
	ops->readlink = kvsfs_readsymlink;
	ops->test_access = fsal_test_access;
	ops->getattrs = kvsfs_getattrs;
	ops->setattrs = kvsfs_setattrs;
	ops->link = kvsfs_linkfile;
	ops->rename = kvsfs_rename;
	ops->unlink = kvsfs_unlink;
	ops->open = kvsfs_open;
	ops->status = kvsfs_status;
	ops->read = kvsfs_read;
	ops->write = kvsfs_write;
	ops->commit = kvsfs_commit;
	ops->lock_op = kvsfs_lock_op;
	ops->close = kvsfs_close;
	ops->lru_cleanup = kvsfs_lru_cleanup;
	ops->handle_digest = kvsfs_handle_digest;
	/** ops->handle_to_key = kvsfs_handle_to_key; @todo */

	/* xattr related functions */
	ops->list_ext_attrs = kvsfs_list_ext_attrs;
	ops->getextattr_id_by_name = kvsfs_getextattr_id_by_name;
	ops->getextattr_value_by_name = kvsfs_getextattr_value_by_name;
	ops->getextattr_value_by_id = kvsfs_getextattr_value_by_id;
	ops->setextattr_value = kvsfs_setextattr_value;
	ops->setextattr_value_by_id = kvsfs_setextattr_value_by_id;
	ops->getextattr_attrs = kvsfs_getextattr_attrs;
	ops->remove_extattr_by_id = kvsfs_remove_extattr_by_id;
	ops->remove_extattr_by_name = kvsfs_remove_extattr_by_name;
}

/* export methods that create object handles
 */

/* create_handle
 * Does what original FSAL_ExpandHandle did (sort of)
 * returns a ref counted handle to be later used in cache_inode etc.
 * NOTE! you must release this thing when done with it!
 * BEWARE! Thanks to some holes in the *AT syscalls implementation,
 * we cannot get an fd on an AF_UNIX socket.  Sorry, it just doesn't...
 * we could if we had the handle of the dir it is in, but this method
 * is for getting handles off the wire for cache entries that have LRU'd.
 * Ideas and/or clever hacks are welcome...
 */

fsal_status_t kvsfs_create_handle(struct fsal_export *exp_hdl,
				 struct gsh_buffdesc *hdl_desc,
				 struct fsal_obj_handle **handle)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}
