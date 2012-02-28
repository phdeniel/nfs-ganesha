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
 *
 * \file    linux_mntent.c
 * \author  $Author: welch $
 * \date    $Date: 2011/10/21 13:45:36 $
 * \version $Revision: 1.0 $
 * \brief   read the mount table
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#ifdef LINUX
#include <mntent.h>             /* for handling mntent */
#else
#error "No mount table API on this platform"
#endif

/**
 * @defgroup FSALMount Mount table reader
 *
 * These functions read the mount table
 *
 * @{
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

#include <mntent.h>

/*
 * The opqaue type fsal_mnt_iter_t is cast to this type on Linux
 */
typedef struct
{
  FILE *fp;
} linux_mnt_iter_t;

/**
 * Initialize a handle used to read the mount table
 */
fsal_status_t fsal_mntent_setup(fsal_mnt_iter_t *opaque_handle)
{
  linux_mnt_iter_t *mnt_handle;

  *opaque_handle = NULL;
  mnt_handle = malloc(sizeof(*mnt_handle));
  if (mnt_handle == NULL)
    {
      LogCrit(COMPONENT_OS, "Malloc failure 1 in fsal_mntent_setup");
      Return(posix2fsal_error(ENOMEM), 0, INDEX_FSAL_BuildExportContext);
    }

  mnt_handle->fp = setmntent(MOUNTED, "r");

  if(mnt_handle->fp == NULL)
    {
      LogCrit(COMPONENT_FSAL, "Error %d in setmntent(%s): %s", errno, MOUNTED,
                      strerror(errno));
      Return(posix2fsal_error(errno), 0, INDEX_FSAL_BuildExportContext);
    }
  *opaque_handle = (fsal_mnt_iter_t)mnt_handle;
  Return(ERR_FSAL_NO_ERROR, 0, INDEX_FSAL_BuildExportContext);
}

/**
 * Return the next mount table entry
 */
int fsal_mntent_next(fsal_mnt_iter_t opaque_handle, int len, char *mntdir, char *type, char *fsname)
{
  linux_mnt_iter_t *mnt_handle = (linux_mnt_iter_t *)opaque_handle;
  struct mntent *p_mnt;

  if (mnt_handle == NULL || mnt_handle->fp == NULL)
  {
    return 0;
  }
  p_mnt = getmntent(mnt_handle->fp);
  if (p_mnt == NULL)
  {
    return 0;
  }
  strncpy(mntdir, p_mnt->mnt_dir, len);
  strncpy(type, p_mnt->mnt_type, len);
  strncpy(fsname, p_mnt->mnt_fsname, len);

  return 1;
}

/**
 * Clean up the mount table iterator
 */
int fsal_mntent_done(fsal_mnt_iter_t opaque_handle)
{
  linux_mnt_iter_t *mnt_handle = (linux_mnt_iter_t *)opaque_handle;

  if (mnt_handle == NULL) return 0;
  if (mnt_handle->fp != NULL)
  {
    endmntent(mnt_handle->fp);
  }
  free(mnt_handle);
  return 0;
}

/* @} */
