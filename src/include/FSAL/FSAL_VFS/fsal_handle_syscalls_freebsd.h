/*
 *   Copyright (C) Panasas, Inc. 2011
 *   Author(s): Brent Welch <welch@panasas.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library can be distributed with a BSD license as well, just ask.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef HANDLE_FREEBSD_H
#define HANDLE_FREEBSD_H

#ifdef FREEBSD

/*
 * FreeBSD has had getfh() and fhopen() since version 4.
 * This defines struct fhandle
 */
#include <sys/mount.h>

#ifndef O_PATH
#define O_PATH 0
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif

#ifndef O_NOACCESS
#define O_NOACCESS 0
#endif

/*
 * The "at" calls are custom to this version of BSD
 * Duplicating things here eliminates a build dependency on finding
 * the right version of the fcntl.h header file
 */

/* Should come from <sys/fcntl.h> */
#ifndef AT_EMPTY_PATH
# define AT_FDCWD               -100    /* Special value used to indicate
                                           the *at functions should use the
                                           current working directory. */
# define AT_SYMLINK_NOFOLLOW    0x100   /* Do not follow symbolic links.  */
# define AT_REMOVEDIR           0x200   /* Remove directory instead of
                                           unlinking file.  */
# define AT_SYMLINK_FOLLOW      0x400   /* Follow symbolic links.  */
# define AT_NO_AUTOMOUNT        0x800   /* Suppress terminal automount
                                           traversal.  */
# define AT_EMPTY_PATH          0x1000  /* Allow empty relative pathname.  */
# define AT_EACCESS             0x200   /* Test access permitted for
                                           effective IDs, not real IDs.  */
#endif

/* Should come from <sys/syscall.h> */
int openat(int dir_fd, char *file, int oflag, mode_t mode);
int fchownat (int dir_fd, char *file, uid_t owner, gid_t group, int flag);
int futimesat(int dir_fd, char *filename, struct timeval *utimes);
int fstatat (int dir_fd, char *file, struct stat *st, int flag);
int getfhat(int dir_fd, char *fname, struct fhandle *fhp);
int fhopenat(int dir_fd, const struct fhandle *u_fhp, int flags);
int fchmodat(int dir_fd, char *filename, mode_t mode, int flags);
int faccessat(int dir_fd, char *filename, int mode, int flags);
int linkat (int fromfd, char *from, int tofd, char *to,int flags);
int mkdirat (int dir_fd, char *file, mode_t mode);
int mkfifoat (int dir_fd, char *file,mode_t  mode);
int mknodat (int dir_fd, char *file, mode_t mode, dev_t *dev);
int unlinkat (int dir_fd, char *file, int flag);
int readlinkat (int fd, char *path, char *buf, size_t len);
int symlinkat (char *from, int tofd, char *to);
int renameat (int oldfd, char *old, int newfd, char *new);
int utimensat (int dir_fd, char *path, struct timespec *times, int flags);

#else /* ifdef FREEBSD */
#error "Not FreeBSD, no by handle syscalls defined."
#endif

/*
 * vfs_file_handle_t has a leading type and byte count that is
 * not aligned the same as the BSD struct fhandle, which begins
 * instead with a struct fsid, which is two 32-bit ints, then
 * a 2-byte length, 2-bytes of pad, and finally the array of handle bytes.
 * PanFS doesn't fill in the length, oh by the way.
 * So we copy the struct fhandle into the handle array instead of
 * overlaying the whole type like the Linux code does it.
 */
#if VFS_HANDLE_LEN < 48
#error "VFS_HANDLE_LEN must be >= 48"
#endif
#define PANFS_HANDLE_SIZE 40
#define VFS_BSD_HANDLE_INIT(_fh, _handle) \
    _fh->handle_bytes = sizeof(struct fsid) + PANFS_HANDLE_SIZE; \
    _fh->handle_type = 0; \
    memcpy((void *)&_fh->handle[0], (void *)&_handle, _fh->handle_bytes);

static inline int vfs_fd_to_handle(int fd, vfs_file_handle_t * fh, int *mnt_id)
{
  int error;
  struct fhandle handle;
  error = getfhat(fd, NULL, &handle);
  if (error == 0) {
    VFS_BSD_HANDLE_INIT(fh, handle);
  }
  return error;
}

static inline int vfs_name_by_handle_at(int atfd, const char *name, vfs_file_handle_t *fh)
{
  int error;
  struct fhandle handle;
  error = getfhat(atfd, (char *)name, &handle);
  if (error == 0) {
    VFS_BSD_HANDLE_INIT(fh, handle);
  }
  return error;
}

static inline int vfs_open_by_handle(int mountfd, vfs_file_handle_t * fh, int flags)
{
  return fhopen((struct fhandle *)fh->handle, flags);
}

static inline int vfs_stat_by_handle(int mountfd, vfs_file_handle_t *fh, struct stat *buf)
{
        int fd, ret;
        fd = vfs_open_by_handle(mountfd, fh, (O_PATH|O_NOACCESS));
        if (fd < 0)
                return fd;
        /* BSD doesn't (yet) have AT_EMPTY_PATH support, so just use fstat() */
        ret = fstat(fd, buf);
        close(fd);
        return ret;
}

static inline int vfs_chown_by_handle(int mountfd, vfs_file_handle_t *fh, uid_t owner, gid_t group)
{
        int fd, ret;
        fd = vfs_open_by_handle(mountfd, fh, (O_PATH|O_NOACCESS));
        if (fd < 0)
                return fd;
        /* BSD doesn't (yet) have AT_EMPTY_PATH support, so just use fchown() */
        ret = fchown(fd, owner, group);
        close(fd);
        return ret;
}

#endif
