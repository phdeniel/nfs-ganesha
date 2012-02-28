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
 * \brief   Test program for FreeBSD "at" syscalls
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <sys/uio.h>

#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif
#ifndef O_PATH
#define O_PATH 0
#endif
#ifndef O_NOACCESS
#define O_NOACCESS O_ACCMODE
#endif

void print_handle(struct fhandle *fhp);

int
main(int argc, char *argv[])
{
  struct fhandle fh;
  int dir_fd, fd1;
  int len, n;
  char *main_dir = NULL;
  char *a_file = NULL;
  char *path;
  char buf[1024];

  if (argv[1] != NULL) {
    main_dir = argv[1];
  } else {
    main_dir = "/tmp";
  }
  if (argv[2] != NULL) {
    a_file = argv[2];
    if (getfh(a_file, &fh) < 0) {
      perror("getfh");
      fprintf(stderr, "Cannot get handle for file \"%s\"\n", a_file);
      exit(1);
    }
    printf("Handle for \"%s\" ", a_file);
    print_handle(&fh);
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
    perror("openat");
    fprintf(stderr, "Cannot open %s file via openat\n", path);
    if (errno != ENOENT) {
      exit(1);
    }
  } else {
    printf("Opened %s successfully relative to %s\n", path, main_dir);
    close(fd1);
  }

  if (getfh(main_dir, &fh) < 0) {
    perror("getfh");
    fprintf(stderr, "Cannot getfh %s directory\n", main_dir);
    exit(1);
  }
  printf("Obtained file handle for %s\n", main_dir);
  print_handle(&fh);

  fd1 = fhopen(&fh, (O_PATH|O_RDONLY));
  if (fd1 < 0) {
    perror("fhopen");
    fprintf(stderr, "Cannot fhopen handle for %s\n", main_dir);
    exit(1);
  }
  printf("Opened %s from its handle\n", main_dir);

  path = "hello";
  if (getfhat(dir_fd, path, &fh) < 0) {
    perror("getfhat");
    fprintf(stderr, "Cannot getfhat %s file relative to %s\n", path, main_dir);
    exit(1);
  }
  printf("Obtained file handle for %s relative to %s\n", path, main_dir);
  print_handle(&fh);

  fd1 = fhopen(&fh, (O_PATH|O_RDONLY));
  if (fd1 < 0) {
    perror("fhopen");
    fprintf(stderr, "Cannot fhopen handle for %s\n", path);
    exit(1);
  }
  printf("Opened %s from its handle\n", path);

  n = read(fd1, buf, sizeof(buf));
  if (n < 0) {
    perror("read");
    fprintf(stderr, "Cannot read %s file after fhopen\n", path);
    exit(1);
  }
  printf("Read %d bytes from %s\n", n, path);
  close(fd1);

  if (getfhat(dir_fd, NULL, &fh) < 0) {
    perror("getfhat");
    fprintf(stderr, "Cannot getfhat from fd for %s w/ NULL path\n", main_dir);
    exit(1);
  }
  printf("Obtained file handle from fd for %s w/ NULL path\n", main_dir);
  print_handle(&fh);

  path = "hello";
  n = unlinkat(dir_fd, path);
  if (n < 0) {
    perror("unlinkat");
    fprintf(stderr, "Cannot delete %s file via unlinkat\n", path);
    exit(1);
  }
  printf("Deleted %s successfully relative to %s\n", path, main_dir);

  return 0;
}

/* This is redefined in freebsd-c, so let's just hard-wire it here to be sure */
#define MAXFIDSZ        36

#ifdef notdef_for_docs

struct fid {
        u_short         fid_len;                /* length of data in bytes */
        u_short         fid_reserved;           /* force longword alignment */
        char            fid_data[MAXFIDSZ];     /* data (variable length) */
};

struct fhandle {
        fsid_t  fh_fsid;        /* Filesystem id of mount point */
        struct  fid fh_fid;     /* Filesys specific id */
};
#endif

/*
 * Panfs doesn't put a len at the front of its handles, but
 * it still casts the storage for "struct fid" to
 * the pan_kernel_fs_client_fid_t type, which has 4 reserved
 * bytes in the front.  But, it doesn't fill in the fid_len correctly
 */
void
print_handle(struct fhandle *fhp)
{
  int i;
  printf("fsid %08x.%08x ",
        fhp->fh_fsid.val[0], fhp->fh_fsid.val[1]);
  printf("%02x%02x", fhp->fh_fid.fid_len & 0xff,
                     fhp->fh_fid.fid_reserved & 0xff);
  for (i=0 ; i < MAXFIDSZ ; i++) {
    printf("%02x", fhp->fh_fid.fid_data[i] & 0xff);
  }
  printf("\n");
}

