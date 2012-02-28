/*
 *
 * Copyright (C) 2012 Panasas, Inc.
 * Author: Brent Welch
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * ---------------------------------------
 */

/**
 * \file    fsal_mntent.h
 * \brief   OS-specific API to manage mount tables.
 *
 * fsal_mntent_setup allocates and initializes a handle.
 *
 * fsal_mntent_next returns the next mount table entry.  The len parameter is the size of the
 * caller's buffers that they must pass in to take a copy of the mntdir (where it is mounted)
 * the type of the mount, and the fsname (what is being mounted)
 *
 * fsal_mntent_done deallocates the handle and internal buffers.
 */

#ifndef _FSAL_MNTENT_H
#define _FSAL_MNTENT_H

typedef void *fsal_mnt_iter_t;

fsal_status_t fsal_mntent_setup(fsal_mnt_iter_t *mnt_handle);
int fsal_mntent_next(fsal_mnt_iter_t mnt_handle, int len, char *mntdir, char *type, char *fsname);
int fsal_mntent_done(fsal_mnt_iter_t mnt_handle);

#endif /* _FSAL_MNTENT_H */
