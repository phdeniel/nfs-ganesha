#include "config.h"

#include <libgen.h>		/* used for 'dirname' */
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <mntent.h>
#include "gsh_list.h"
#include "fsal.h"
#include "fsal_internal.h"
#include "fsal_convert.h"
#include "FSAL/fsal_config.h"
#include "FSAL/fsal_commonlib.h"
#include "zfs_methods.h"
#include <stdbool.h>

/* a f***ing ugly define, to be removed later as things are OK */
#define EXTERNAL_STORE "/btrfs/store"

static int build_external_path(inogen_t object,
			       char *external_path,
			       size_t pathlen)
{
	if (!external_path)
		return -1;

	return snprintf(external_path, pathlen, "%s/inum=%llu_gen=%llu",
			EXTERNAL_STORE,
			(unsigned long long)object.inode,
			(unsigned long long)object.generation);
}

int external_read(struct fsal_obj_handle *obj_hdl,
		  uint64_t offset,
		  size_t buffer_size,
		  void *buffer,
		  size_t *read_amount,
		  bool *end_of_file)
{
	char storepath[MAXPATHLEN];
	int rc;
	int fd;
	ssize_t read_bytes;
	struct zfs_fsal_obj_handle *myself;

	myself = container_of(obj_hdl, struct zfs_fsal_obj_handle, obj_handle);

	rc = build_external_path(myself->handle->zfs_handle,
				 storepath, MAXPATHLEN);
	if (rc < 0)
		return rc;

	printf("READ: I got external path=%s\n", storepath);

	fd = open(storepath, O_CREAT|O_RDONLY|O_SYNC);
	if (fd < 0)
		return -errno;

	read_bytes = pread(fd, buffer, buffer_size, offset);
	if (read_bytes < 0) {
		close(fd);
		return -errno;
	}

	*read_amount = read_bytes;
	return read_bytes;
}

int external_write(struct fsal_obj_handle *obj_hdl,
		   uint64_t offset,
		   size_t buffer_size,
		   void *buffer,
		   size_t *write_amount,
		   bool *fsal_stable,
		   struct stat *stat)
{
	char storepath[MAXPATHLEN];
	int rc;
	int fd;
	ssize_t written_bytes;
	struct zfs_fsal_obj_handle *myself;
	struct stat storestat;

	myself = container_of(obj_hdl, struct zfs_fsal_obj_handle, obj_handle);

	rc = build_external_path(myself->handle->zfs_handle,
				 storepath, MAXPATHLEN);
	if (rc < 0)
		return rc;

	printf("WRITE: I got external path=%s\n", storepath);

	fd = open(storepath, O_CREAT|O_WRONLY|O_SYNC);
	if (fd < 0)
		return -errno;

	written_bytes = pwrite(fd, buffer, buffer_size, offset);
	if (written_bytes < 0) {
		close(fd);
		return -errno;
	}

	*write_amount = written_bytes;

	rc = fstat(fd, &storestat);
	if (rc < 0)
		return -errno;

	stat->st_mtime = storestat.st_mtime;
	stat->st_size = storestat.st_size;
	stat->st_blocks = storestat.st_blocks;
	stat->st_blksize = storestat.st_blksize;

	rc = close(fd);
	if (rc < 0)
		return -errno;

	*fsal_stable = true;
	return written_bytes;
}

int external_consolidate_attrs(struct fsal_obj_handle *obj_hdl,
			       struct stat *zfsstat)
{
	struct zfs_fsal_obj_handle *myself;
	struct stat extstat;
	char storepath[MAXPATHLEN];
	int rc;

	if (!obj_hdl || !zfsstat)
		return -1;

	myself = container_of(obj_hdl, struct zfs_fsal_obj_handle, obj_handle);

	if (myself->attributes.type != REGULAR_FILE)
		return 0;

	rc = build_external_path(myself->handle->zfs_handle,
				 storepath, MAXPATHLEN);
	if (rc < 0)
		return rc;

	printf("=======> external_stat: %s\n", storepath);

	rc = stat(storepath, &extstat);
	if (rc < 0) {
		printf("===> external_stat: errno=%u\n", errno);
		if (errno == ENOENT)
			return 0; /* No data written yet */
		else
			return rc;
	}

	zfsstat->st_mtime = extstat.st_mtime;
	zfsstat->st_atime = extstat.st_atime;
	zfsstat->st_size = extstat.st_size;
	zfsstat->st_blksize = extstat.st_blksize;
	zfsstat->st_blocks = extstat.st_blocks;

	return 0;
}

int external_unlink(struct fsal_obj_handle *dir_hdl,
		    const char *name)
{
	int rc;
	char storepath[MAXPATHLEN];
	creden_t cred;
	inogen_t object;
	int type = 0;
	struct stat stat;
	struct zfs_fsal_obj_handle *myself;

	myself = container_of(dir_hdl, struct zfs_fsal_obj_handle, obj_handle);

	cred.uid = op_ctx->creds->caller_uid;
	cred.gid = op_ctx->creds->caller_gid;

	rc = libzfswrap_lookup(tank_get_root_pvfs(op_ctx->fsal_export),
			       &cred,
			       myself->handle->zfs_handle,
			       name, &object, &type);
	if (rc)
		return errno;

	if (type == S_IFDIR)
		return 0;

	rc = libzfswrap_getattr(tank_get_root_pvfs(op_ctx->fsal_export),
				&cred,
				object,
				&stat,
				&type);
	if (rc)
		return errno;

	if (stat.st_nlink > 1)
		return 0;

	rc = build_external_path(object, storepath, MAXPATHLEN);
	if (rc < 0)
		return rc;

	/* file may not exist */
	if (unlink(storepath) < 0) {
		/* Store obj may not exist if file was
		 * never written */
		if (errno == ENOENT)
			return 0;
		else
			return errno;
	}

	/* Should not be reach, but needed for compiler's happiness */
	return 0;
}

int external_truncate(struct fsal_obj_handle *obj_hdl,
		      off_t filesize)
{
	int rc;
	char storepath[MAXPATHLEN];
	struct zfs_fsal_obj_handle *myself;

	myself = container_of(obj_hdl, struct zfs_fsal_obj_handle, obj_handle);

	rc = build_external_path(myself->handle->zfs_handle,
				 storepath, MAXPATHLEN);
	if (rc < 0)
		return rc;

	rc = truncate(storepath, filesize);
	if (rc == -1) {
		if (errno == ENOENT)
			return 0;
		else
			return -errno;
	}

	return 0;
}

