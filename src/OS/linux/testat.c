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
 * \file    testat.c
 * \author  $Author: welch $
 * \date    $Date: 2006/01/17 14:20:07 $
 * \version $Revision: 1.26 $
 * \brief   Test program for Linux "at" syscalls
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/mount.h>
#include <sys/errno.h>

#include "FSAL/FSAL_VFS/fsal_handle_syscalls.h"

#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif
#ifndef O_PATH
#define O_PATH 0
#endif
#ifndef O_NOACCESS
#define O_NOACCESS O_ACCMODE
#endif

void print_handle(struct file_handle *fhp);

#ifdef notdef
static inline int name_to_handle_at(int mdirfd, const char *name,
				    struct file_handle * handle, int *mnt_id, int flags);
extern int open_by_handle_at (int __mountdirfd, struct file_handle *__handle,
			      int __flags);
#endif

int
main(int argc, char *argv[])
{
  int dir_fd, fd1;
  int len, n;
  int mnt_id;
  char *main_dir = NULL;
  char *path;
  struct file_handle *fhp = NULL;
  char buf[1024];

  if (argv[1] != NULL) {
    main_dir = argv[1];
  } else {
    main_dir = "/tmp";
  }

  dir_fd = open(main_dir, O_RDONLY|O_DIRECTORY);
  if (dir_fd < 0) {
    perror("open");
    fprintf(stderr, "Cannot open directory %s\n", main_dir);
    exit(1);
  }
  printf("Opened %s successfully\n", main_dir);

  path = "hello";
  fd1 = openat(dir_fd, path, O_RDWR|O_CREAT, 0666);
  if (fd1 < 0) {
    perror("openat");
    fprintf(stderr, "Cannot create %s file via openat\n", path);
    exit(1);
  }
  printf("Created %s successfully relative to %s\n", path, main_dir);

#define HELLO "Hello, World!\n"
  snprintf(buf, sizeof(buf), HELLO);
  len = strlen(buf);
  n = write(fd1, buf, len);
  if (n < 0) {
    perror("write");
    fprintf(stderr, "Cannot write %s file after openat\n", path);
    exit(1);
  } else if (n < len) {
    fprintf(stderr, "Short write %s file %d not %d bytes\n", path, n, len);
    exit(1);
  }
  close(fd1);

  path = "home";
  fd1 = openat(dir_fd, path, O_RDONLY|O_NOFOLLOW, 0600);
  if (fd1 < 0) {
    if (errno != ENOENT) {
      perror("openat");
      fprintf(stderr, "Cannot open %s file via openat\n", path);
      exit(1);
    } else {
      fprintf(stderr, "Cannot open %s file via openat (ENOENT)\n", path);
    }
  } else {
    printf("Opened %s successfully relative to %s\n", path, main_dir);
    close(fd1);
  }

  fhp = malloc(sizeof(struct vfs_file_handle));
  fhp->handle_bytes = sizeof(struct vfs_file_handle);

  if (name_to_handle_at(dir_fd, "", fhp, &mnt_id, AT_EMPTY_PATH) < 0) {
    perror("name_to_handle_at");
    fprintf(stderr, "Cannot name_to_handle_at %s directory\n", main_dir);
    exit(1);
  }
  printf("Obtained file handle for %s\n", main_dir);
  print_handle(fhp);

  fd1 = open_by_handle_at( dir_fd, fhp, (O_PATH|O_RDONLY));
  if (fd1 < 0) {
    perror("open_by_handle_at");
    fprintf(stderr, "Cannot open_by_handle_at handle for %s\n", main_dir);
    exit(1);
  }
  printf("Opened %s from its handle\n", main_dir);

  path = "hello";
  if (name_to_handle_at(dir_fd, path, fhp, &mnt_id, 0) < 0) {
    perror("name_to_handle_at");
    fprintf(stderr, "Cannot name_to_handle_at %s file relative to %s\n", path, main_dir);
    exit(1);
  }
  printf("Obtained file handle for %s relative to %s\n", path, main_dir);
  print_handle(fhp);

  fd1 = open_by_handle_at( dir_fd, fhp, (O_RDONLY));
  if (fd1 < 0) {
    perror("open_by_handle_at");
    fprintf(stderr, "Cannot open_by_handle_at handle for %s\n", path);
    exit(1);
  }
  printf("Opened %s from its handle\n", path);

  n = read(fd1, buf, sizeof(buf));
  if (n < 0) {
    perror("read");
    fprintf(stderr, "Cannot read %s file after open_by_handle_at\n", path);
    exit(1);
  }
  printf("Read %d bytes from %s\n", n, path);
  close(fd1);

  if (name_to_handle_at(dir_fd, "", fhp, &mnt_id, AT_EMPTY_PATH) < 0) {
    perror("name_to_handle_at");
    fprintf(stderr, "Cannot name_to_handle_at from fd for %s w/ NULL path\n", main_dir);
    exit(1);
  }
  printf("Obtained file handle from fd for %s w/ NULL path\n", main_dir);
  print_handle(fhp);

  path = "hello";
  n = unlinkat(dir_fd, path, /* flag */ 0);
  if (n < 0) {
    perror("unlinkat");
    fprintf(stderr, "Cannot delete %s file via unlinkat\n", path);
    exit(1);
  }
  printf("Deleted %s successfully relative to %s\n", path, main_dir);

  return 0;
}

void
print_handle(struct file_handle *fhp)
{
  int i;
  printf("handle type %d len %d ",
        fhp->handle_type,
	fhp->handle_bytes);
  for (i=0 ; i<fhp->handle_bytes ; i++) {
    printf("%02x", fhp->f_handle[i]);
  }
  printf("\n");
}

