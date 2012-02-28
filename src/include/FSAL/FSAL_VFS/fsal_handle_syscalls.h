/*
 *   Copyright (C) International Business Machines  Corp., 2010
 *   Author(s): Aneesh Kumar K.V <aneesh.kumar@linux.vnet.ibm.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
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

#ifndef HANDLE_H
#define HANDLE_H

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

/*
 * The vfs_file_handle_t is similar to the Linux struct file_handle,
 * except the handle[] array is fixed size in this definition.
 * The BSD struct fhandle is a bit different (of course).
 * So the Linux code will typecast all of vfs_file_handle_t to
 * a struct file_handle, while the BSD code will cast the
 * handle subfield to struct fhandle.
 */

/* This is large enough for PanFS file handles embedded in a BSD fhandle */
#define VFS_HANDLE_LEN 48

typedef struct vfs_file_handle {
        unsigned int handle_bytes;
        int handle_type;
        unsigned char handle[VFS_HANDLE_LEN];
} vfs_file_handle_t ;

#ifdef LINUX
#include "FSAL/FSAL_VFS/fsal_handle_syscalls_linux.h"
#elif FREEBSD
#include "FSAL/FSAL_VFS/fsal_handle_syscalls_freebsd.h"
#else
#error "No by handle syscalls defined."
#endif

static inline ssize_t vfs_readlink_by_handle(int mountfd, vfs_file_handle_t *fh, char *buf, size_t bufsize)
{
        int fd, ret;

        fd = vfs_open_by_handle(mountfd, fh, (O_PATH|O_NOACCESS));
        if (fd < 0)
                return fd;
        ret = readlinkat(fd, "", buf, bufsize);
        close(fd);
        return ret;
}

static inline int vfs_link_by_handle(int mountfd, vfs_file_handle_t *fh, int newdirfd, char *newname)
{
        int fd, ret;
        fd = vfs_open_by_handle(mountfd, fh, (O_PATH|O_NOACCESS));
        if (fd < 0)
                return fd;
        ret = linkat(fd, "", newdirfd, newname, AT_EMPTY_PATH);
        close(fd);
        return ret;
}

#endif
