/*
 * vim:expandtab:shiftwidth=4:tabstop=4:
 */

/**
 *
 * \file    fsal_init.c
 * \author  $Author: leibovic $
 * \date    $Date: 2006/01/24 13:45:37 $
 * \version $Revision: 1.20 $
 * \brief   Initialization functions.
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "fsal.h"
#include "fsal_internal.h"

/**
 * FSAL_Init : Initializes the FileSystem Abstraction Layer.
 *
 * \param init_info (input, fsal_parameter_t *) :
 *        Pointer to a structure that contains
 *        all initialization parameters for the FSAL.
 *        Specifically, it contains settings about
 *        the filesystem on which the FSAL is based,
 *        security settings, logging policy and outputs,
 *        and other general FSAL options.
 *
 * \return Major error codes :
 *         ERR_FSAL_NO_ERROR     (initialisation OK)
 *         ERR_FSAL_FAULT        (init_info pointer is null)
 *         ERR_FSAL_SERVERFAULT  (misc FSAL error)
 *         ERR_FSAL_ALREADY_INIT (The FS is already initialized)
 *         ERR_FSAL_BAD_INIT     (FS specific init error,
 *                                minor error code gives the reason
 *                                for this error.)
 *         ERR_FSAL_SEC_INIT     (Security context init error).
 */
fsal_status_t LUSTREFSAL_Init(fsal_parameter_t * init_info      /* IN */
    )
{

  fsal_status_t status;

  /* sanity check.  */
  if(!init_info)
    Return(ERR_FSAL_FAULT, 0, INDEX_FSAL_Init);

  /* proceeds FSAL internal initialization */

  status = fsal_internal_init_global(&(init_info->fsal_info),
                                     &(init_info->fs_common_info),
                                     &(init_info->fs_specific_info));

  if(FSAL_IS_ERROR(status))
    Return(status.major, status.minor, INDEX_FSAL_Init);

  /* No FS Specific initialization. */

  Return(ERR_FSAL_NO_ERROR, 0, INDEX_FSAL_Init);

}
