/**
 *
 * \file    fsal_internal.h
 * \date    $Date: 2006/01/24 13:45:37 $
 * \brief   Extern definitions for variables that are
 *          defined in fsal_internal.c.
 *
 */

#ifndef _FSAL_INTERNAL_H
#define _FSAL_INTERNAL_H

#include  "fsal.h"
#include <kvsns/kvsns.h>

/* linkage to the exports and handle ops initializers
 */

void kvsfs_export_ops_init(struct export_ops *ops);
void kvsfs_handle_ops_init(struct fsal_obj_ops *ops);

typedef struct kvsfs_file_handle {
	kvsns_ino_t kvsfs_handle;
} kvsfs_file_handle_t;

struct kvsfs_ds {
	struct fsal_ds_handle ds; /*< Public DS handle */
	struct kvsfs_file_handle wire; /*< Wire data */
	struct kvsfs_filesystem *kvsfs_fs; /*< Related kvsfs filesystem */
	bool connected; /*< True if the handle has been connected */
};

/* defined the set of attributes supported with POSIX */
#define KVSFS_SUPPORTED_ATTRIBUTES (				       \
		ATTR_TYPE     | ATTR_SIZE     |		   	       \
		ATTR_FSID     | ATTR_FILEID   |		   		\
		ATTR_MODE     | ATTR_NUMLINKS | ATTR_OWNER     |	\
		ATTR_GROUP    | ATTR_ATIME    | ATTR_RAWDEV    |	\
		ATTR_CTIME    | ATTR_MTIME    | ATTR_SPACEUSED |	\
		ATTR_CHGTIME)

static inline size_t kvsfs_sizeof_handle(struct kvsfs_file_handle *hdl)
{
	return (size_t) sizeof(struct kvsfs_file_handle);
}

/* the following variables must not be defined in fsal_internal.c */
#ifndef FSAL_INTERNAL_C

/* static filesystem info.
 * read access only.
 */
extern struct fsal_staticfsinfo_t global_fs_info;

/* KVSFS methods for pnfs
 */

nfsstat4 kvsfs_getdeviceinfo(struct fsal_module *fsal_hdl,
			      XDR *da_addr_body,
			      const layouttype4 type,
			      const struct pnfs_deviceid *deviceid);

size_t kvsfs_fs_da_addr_size(struct fsal_module *fsal_hdl);
void export_ops_pnfs(struct export_ops *ops);
void handle_ops_pnfs(struct fsal_obj_ops *ops);
void kvsfs_pnfs_ds_ops_init(struct fsal_pnfs_ds_ops *ops);

#endif
#endif
