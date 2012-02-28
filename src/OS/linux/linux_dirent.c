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
 * \file    linux_dirent.c
 * \author  $Author: welch $
 * \date    $Date: 2006/01/17 14:20:07 $
 * \version $Revision: 1.26 $
 * \brief   API to mask differences between dirent on different systems
 *
 * Because there is no "opendirat", we use openat and getdents.  For linux getdents,
 * need to define your own version of linux_dirent based on the syscall manpage
 */

/* Cut/paste directly from the man page */

struct linux_dirent {
   unsigned long  d_ino;     /* Inode number */
   unsigned long  d_off;     /* Offset to next linux_dirent */
   unsigned short d_reclen;  /* Length of this linux_dirent */
   char           d_name[];  /* Filename (null-terminated) */
                       /* length is actually (d_reclen - 2 -
                          offsetof(struct linux_dirent, d_name) */
   /*
   char           pad;       // Zero padding byte
   char           d_type;    // File type (only since Linux 2.6.4;
                             // offset is (d_reclen - 1))
   */

};


#include "fsal_types.h"
 
#ifndef offsetof
#define offsetof(type, field) ((off_t)(&((type *)0)->field))
#endif

void
vfsfsal_get_dirent(size_t dir_offset, char *addr, vfsfsal_dirent_t *dp)
{
    struct linux_dirent *direntp = (struct linux_dirent *)addr;
    char type;

    dp->d_ino = direntp->d_ino;
    dp->d_off = direntp->d_off;	         /* Should equal dir_offset */
    dp->d_reclen = direntp->d_reclen;
    type = addr[direntp->d_reclen - 1];
    dp->d_type = type;
    strncpy(dp->d_name, direntp->d_name, direntp->d_reclen -2 -offsetof(struct linux_dirent, d_name));
}
