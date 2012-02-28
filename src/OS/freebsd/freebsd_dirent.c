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
 * \file    free_dirent.c
 * \author  $Author: welch $
 * \date    $Date: 2006/01/17 14:20:07 $
 * \version $Revision: 1.26 $
 * \brief   API to mask differences between dirent on different systems
 *
 * Because there is no "opendirat", we use openat and getdents.  The BSD dirent
 * doesn't have an offset field, so the API passes it in based on context from
 * our caller's iteration.  However, it isn't safe to lseek() to the offset
 * and attempt getdents on a BSD system because you won't necessarily get
 * the record you're are looking for - lseek offsets are only valid between calls
 * to getdents.
 */

#include <dirent.h>
#include "fsal.h"
#include "FSAL/FSAL_VFS/fsal_types.h"

void
vfsfsal_get_dirent(size_t dir_offset, char *addr, vfsfsal_dirent_t *dp)
{
    struct dirent *direntp = (struct dirent *)addr;     /* FreeBSD type */

    dp->d_ino = direntp->d_fileno;
    dp->d_off = dir_offset;
    dp->d_reclen = direntp->d_reclen;
    dp->d_type = direntp->d_type;
    strncpy(dp->d_name, direntp->d_name, sizeof(dp->d_name));
}
