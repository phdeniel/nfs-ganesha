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
 * \file    testdir.c
 * \author  $Author: welch $
 * \date    $Date: 2006/01/17 14:20:07 $
 * \version $Revision: 1.26 $
 * \brief   Test program for FreeBSD getdents and lseek behavior
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/dirent.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/syscall.h>

#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif
#ifndef O_PATH
#define O_PATH 0
#endif
#ifndef O_NOACCESS
#define O_NOACCESS O_ACCMODE
#endif

int
main(int argc, char *argv[])
{
  int dir_fd;
  int n, reclen, buf_offset;
  size_t offset, seek_offset;
  struct dirent *p_dirent;
  char *main_dir = NULL;
  char buf[256];

  if (argv[1] != NULL) {
    main_dir = argv[1];
  } else {
    fprintf(stderr, "Usage: %s /path/to/directory (seek_offset)\n", argv[0]);
    exit(1);
  }

  dir_fd = open(main_dir, O_RDONLY|O_DIRECTORY);
  if (dir_fd < 0) {
    perror("open");
    fprintf(stderr, "Cannot open directory %s\n", main_dir);
    exit(1);
  }
  printf("Opened %s successfully\n", main_dir);
  if (argv[2] != NULL) {
    seek_offset = atoi(argv[2]);
    if (lseek(dir_fd, seek_offset, SEEK_SET) < 0) {
      perror("lseek");
      exit(1);
    }
  }

  offset = 0;
  while (1) {
    n = getdents(dir_fd, buf, sizeof(buf));
    if (n < 0) {
      perror("getdents");
      exit(1);
    } else if (n == 0) {
      break;
    }
    buf_offset = 0;
    printf("---\n");
    while (buf_offset < n) {
      p_dirent = (struct dirent *)&buf[buf_offset];
      reclen = p_dirent->d_reclen;
      printf("%ld | fileno %d reclen %d \"%s\"\n", 
	offset, p_dirent->d_fileno, reclen, p_dirent->d_name);
      offset += reclen;
      buf_offset += reclen;
    }
    printf("seek offset %ld\n", lseek(dir_fd, 0, SEEK_CUR));
  }

  return 0;
}

