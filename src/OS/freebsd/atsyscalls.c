/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright Panasas, Inc (2012)
 * contributeur : Brent Welch welch@panasas.com
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
 * ------------- 
 */

/**
 * \file    atsyscalls.c
 * \author  $Author: welch $
 * \date    $Date: 2006/01/17 14:20:07 $
 * \version $Revision: 1.26 $
 * \brief   syscall stubs for the "at" syscall family added to FreeBSD to support Ganesha
 *
 * These stubs are ordinarily automatically generated and built into libc,
 * but to resolve build order issues between Ganesha and our customized FreeBSD,
 * it is easier to just duplicate them here.  The one fine point is that the
 * kernel source area src/sys/sys must be on the -I compile line for Ganesha so
 * the new SYS_ macros that define the system call numbers are picked up.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/syscall.h>

/* "*at" syscalls
 * These syscalls behave the same as their non-at equivalences of yore
 * except the following:
 *
 * If the pathname given is relative, then it is interpreted
 * relative to the file descriptor 'dirfd' rather than the current
 * working directory for its non-at equivalent.
 * If the pathname is relative and the 'dirfd' is the special value
 * AT_FDCWD, the pathname is interpreted relative to the current working
 * directory.  For example,
 *
 *	fd = openat(AT_FDCWD, "some/path", O_RW|O_CREAT, 0755);
 *
 * is equivalent to
 *
 *	fd = open("some/path", O_RW|O_CREAT, 0755);
 *
 * If the pathname given is absolute, the 'dirfd' argument is ignored.
 */

int openat(int dir_fd, char *file, int oflag, mode_t mode)
{
  return syscall(SYS_openat, dir_fd, file, oflag, mode);
}
int mkdirat (int dir_fd, char *file, mode_t mode)
{
  return syscall(SYS_mkdirat, dir_fd, file, mode);
}
int mknodat (int dir_fd, char *file, mode_t mode, dev_t *dev)
{
  return syscall(SYS_mknodat, dir_fd, file, mode, dev);
}
int fchownat (int dir_fd, char *file, uid_t owner, gid_t group, int flag)
{
  return syscall(SYS_fchownat, dir_fd, file, owner, group, flag);
}
int futimesat(int dir_fd, char *filename, struct timeval *utimes)
{
  return syscall(SYS_futimesat, dir_fd, filename, utimes);
}
int fstatat (int dir_fd, char *file, struct stat *st, int flag)
{
  return syscall(SYS_fstatat, dir_fd, file, st, flag);
}
int unlinkat (int dir_fd, char *file, int flag)
{
  return syscall(SYS_unlinkat, dir_fd, file, flag);
}
int renameat (int oldfd, char *old, int newfd, char *new)
{
  return syscall(SYS_renameat, oldfd, old, newfd, new);
}
int linkat (int fromfd, char *from, int tofd, char *to, int flags)
{
  return syscall(SYS_linkat, fromfd, from, tofd, to, flags);
}
int symlinkat (char *from, int tofd, char *to)
{
  return syscall(SYS_symlinkat, from, tofd, to);
}
int readlinkat (int fd, char *path, char *buf, size_t len)
{
  return syscall(SYS_readlinkat, fd, path, buf, len);
}
int fchmodat(int dir_fd, char *filename, mode_t mode, int flags)
{
  return syscall(SYS_fchmodat, dir_fd, filename, mode, flags);
}
int faccessat(int dir_fd, char *filename, int mode, int flags)
{
  return syscall(SYS_faccessat, dir_fd, filename, mode, flags);
}
int getfhat(int dir_fd, char *fname, struct fhandle *fhp)
{
  return syscall(SYS_getfhat, dir_fd, fname, fhp);
}
int fhopenat(int dir_fd, const struct fhandle *u_fhp, int flags)
{
  return syscall(SYS_fhopenat, dir_fd, u_fhp, flags);
}

/* library calls that use mknodat */

int mkfifoat (int dir_fd, char *file,mode_t  mode)
{
  return ENOSYS;
}
int utimensat (int dir_fd, char *path, struct timespec *times, int flags)
{
  return ENOSYS;
}
