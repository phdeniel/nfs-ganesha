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
 * \file    freebsd_mntent.c
 * \author  $Author: welch $
 * \date    $Date: 2006/01/17 14:20:07 $
 * \version $Revision: 1.26 $
 * \brief   read the mount table on FreeBSD
 *
 * The BSD api is getfststat, which is used to find out how many mount table entries there are,
 * and then a second time to read the mount table into a buffer we have to allocate.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "fsal.h"
#include "../../FSAL/FSAL_VFS/fsal_internal.h"
#include "fsal_mntent.h"
#include "fsal_convert.h"
#include "log_macros.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <sys/param.h>
#include <sys/mount.h>

/*
 * The opaque type fsal_mnt_iter_t is cast to this type on FreeBSD
 */

typedef struct
{
  int num_fs;
  int cur_fs;
  struct statfs *buf;
} freebsd_mnt_iter_t;

/**
 * @defgroup OSMount Mount table reader
 *
 * These functions read the mount table
 *
 * @{
 */

/**
 * Initialize a handle used to read the mount table (BSD)
 */
fsal_status_t fsal_mntent_setup(fsal_mnt_iter_t *opaque_handle)
{
  freebsd_mnt_iter_t *mnt_handle;
  
  *opaque_handle = NULL;
  mnt_handle = malloc(sizeof(*mnt_handle));
  if (mnt_handle == NULL)
    {
      LogCrit(COMPONENT_OS, "Malloc failure 1 in fsal_mntent_setup");
      Return(posix2fsal_error(ENOMEM), 0, INDEX_FSAL_BuildExportContext);
    }

  /* Find out how many mount table entries there are */
  mnt_handle->cur_fs = 0;
  mnt_handle->buf = NULL;
  mnt_handle->num_fs = getfsstat(NULL, 0, MNT_NOWAIT);

  if(mnt_handle->num_fs < 0)
    {
      free(mnt_handle);
      LogCrit(COMPONENT_OS, "Error %d in getfsstat(): %s", errno,
                      strerror(errno));
      Return(posix2fsal_error(errno), 0, INDEX_FSAL_BuildExportContext);
    }
  /*
   * Allocate the buffer and read in the mount table
   */
  mnt_handle->buf = (struct statfs *)calloc(mnt_handle->num_fs, sizeof(struct statfs));
  if (mnt_handle->buf == NULL)
    {
      free(mnt_handle);
      LogCrit(COMPONENT_OS, "Malloc failure 2 in fsal_mntent_setup");
      Return(posix2fsal_error(ENOMEM), 0, INDEX_FSAL_BuildExportContext);
    }
  if (getfsstat(mnt_handle->buf, mnt_handle->num_fs * sizeof(struct statfs), MNT_NOWAIT) < 0) 
    {
      fsal_mntent_done((fsal_mnt_iter_t)mnt_handle);    /* free memory */
      LogCrit(COMPONENT_OS, "getfsstat failed");
      Return(posix2fsal_error(errno), 0, INDEX_FSAL_BuildExportContext);
    }
  *opaque_handle = (fsal_mnt_iter_t)mnt_handle;
  Return(ERR_FSAL_NO_ERROR, 0, INDEX_FSAL_BuildExportContext);
}

/**
 * Return the next mount table entry (BSD)
 */
int fsal_mntent_next(fsal_mnt_iter_t opaque_handle, int len, char *mntdir, char *type, char *fsname)
{
  freebsd_mnt_iter_t *mnt_handle = (freebsd_mnt_iter_t *)opaque_handle;
  struct statfs *s;

  if (mnt_handle == NULL ||
      mnt_handle->cur_fs >= mnt_handle->num_fs)
  {
    return 0;
  }
  s = &mnt_handle->buf[mnt_handle->cur_fs];
  strncpy(mntdir, s->f_mntonname, len);
  strncpy(type, s->f_fstypename, len);
  strncpy(fsname, s->f_mntfromname, len);

  mnt_handle->cur_fs++;

  return 1;
}

/**
 * Clean up the mount table iterator (BSD)
 */
int fsal_mntent_done(fsal_mnt_iter_t opaque_handle)
{
  freebsd_mnt_iter_t *mnt_handle = (freebsd_mnt_iter_t *)opaque_handle;

  if (mnt_handle == NULL) return 0;
  if (mnt_handle->buf != NULL)
  {
    free(mnt_handle->buf);
  }
  free(mnt_handle);
  return 0;
}

/* @} */
