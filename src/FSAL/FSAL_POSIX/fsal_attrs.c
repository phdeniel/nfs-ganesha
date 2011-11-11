/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 */

/**
 *
 * \file    fsal_attrs.c
 * \author  $Author: leibovic $
 * \date    $Date: 2005/09/09 15:22:49 $
 * \version $Revision: 1.19 $
 * \brief   Attributes functions.
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "fsal.h"
#include "fsal_internal.h"
#include "fsal_convert.h"
#include <utime.h>

/**
 * FSAL_getattrs:
 * Get attributes for the object specified by its filehandle.
 *
 * \param filehandle (input):
 *        The handle of the object to get parameters.
 * \param cred (input):
 *        Authentication context for the operation (user,...).
 * \param object_attributes (mandatory input/output):
 *        The retrieved attributes for the object.
 *        As input, it defines the attributes that the caller
 *        wants to retrieve (by positioning flags into this structure)
 *        and the output is built considering this input
 *        (it fills the structure according to the flags it contains).
 *
 * \return Major error codes :
 *        - ERR_FSAL_NO_ERROR     (no error)
 *        - Another error code if an error occured.
 */
fsal_status_t POSIXFSAL_getattrs(fsal_handle_t * filehandle,     /* IN */
                                 fsal_op_context_t * context,    /* IN */
                                 fsal_attrib_list_t * p_object_attributes       /* IN/OUT */
    )
{
  posixfsal_handle_t * p_filehandle = (posixfsal_handle_t *) filehandle;
  posixfsal_op_context_t * p_context = (posixfsal_op_context_t *) context;
  fsal_status_t status;

  fsal_path_t fsalpath;
  struct stat buffstat;

  /* sanity checks.
   * note : object_attributes is mandatory in FSAL_getattrs.
   */
  if(!p_filehandle || !p_context || !p_object_attributes)
    Return(ERR_FSAL_FAULT, 0, INDEX_FSAL_getattrs);

  status =
      fsal_internal_getPathFromHandle(p_context, p_filehandle, 0, &fsalpath, &buffstat);
  if(FSAL_IS_ERROR(status))
    Return(status.major, status.minor, INDEX_FSAL_getattrs);

  /* convert attributes */

  status = posix2fsal_attributes(&buffstat, p_object_attributes);
  if(FSAL_IS_ERROR(status))
    {
      FSAL_CLEAR_MASK(p_object_attributes->asked_attributes);
      FSAL_SET_MASK(p_object_attributes->asked_attributes, FSAL_ATTR_RDATTR_ERR);
      Return(status.major, status.minor, INDEX_FSAL_getattrs);
    }

  Return(ERR_FSAL_NO_ERROR, 0, INDEX_FSAL_getattrs);

}

/**
 * FSAL_setattrs:
 * Set attributes for the object specified by its filehandle.
 *
 * \param filehandle (input):
 *        The handle of the object to get parameters.
 * \param cred (input):
 *        Authentication context for the operation (user,...).
 * \param attrib_set (mandatory input):
 *        The attributes to be set for the object.
 *        It defines the attributes that the caller
 *        wants to set and their values.
 * \param object_attributes (optionnal input/output):
 *        The post operation attributes for the object.
 *        As input, it defines the attributes that the caller
 *        wants to retrieve (by positioning flags into this structure)
 *        and the output is built considering this input
 *        (it fills the structure according to the flags it contains).
 *        May be NULL.
 *
 * \return Major error codes :
 *        - ERR_FSAL_NO_ERROR     (no error)
 *        - Another error code if an error occured.
 */
fsal_status_t POSIXFSAL_setattrs(fsal_handle_t * filehandle,     /* IN */
                                 fsal_op_context_t * context,    /* IN */
                                 fsal_attrib_list_t * p_attrib_set,     /* IN */
                                 fsal_attrib_list_t * p_object_attributes       /* [ IN/OUT ] */
    )
{
  posixfsal_handle_t * p_filehandle = (posixfsal_handle_t *) filehandle;
  posixfsal_op_context_t * p_context = (posixfsal_op_context_t *) context;
  int rc, errsv;
  unsigned int i;
  fsal_status_t status;
  fsal_attrib_list_t attrs;

  fsal_path_t fsalpath;
  struct stat buffstat;

  /* sanity checks.
   * note : object_attributes is optional.
   */
  if(!p_filehandle || !p_context || !p_attrib_set)
    Return(ERR_FSAL_FAULT, 0, INDEX_FSAL_setattrs);

  /* local copy of attributes */
  attrs = *p_attrib_set;

  /* First, check that FSAL attributes changes are allowed. */

  /* Is it allowed to change times ? */

  if(!global_fs_info.cansettime)
    {

      if(attrs.asked_attributes
         & (FSAL_ATTR_ATIME | FSAL_ATTR_CREATION | FSAL_ATTR_CTIME | FSAL_ATTR_MTIME))
        {
          /* handled as an unsettable attribute. */
          Return(ERR_FSAL_INVAL, 0, INDEX_FSAL_setattrs);
        }
    }

  /* apply umask, if mode attribute is to be changed */
  if(FSAL_TEST_MASK(attrs.asked_attributes, FSAL_ATTR_MODE))
    {
      attrs.mode &= (~global_fs_info.umask);
    }

  /* convert handle into path */
  status =
      fsal_internal_getPathFromHandle(p_context, p_filehandle, 0, &fsalpath, &buffstat);
  if(FSAL_IS_ERROR(status))
    Return(status.major, status.minor, INDEX_FSAL_setattrs);

  /***********
   *  CHMOD  *
   ***********/
  if(FSAL_TEST_MASK(attrs.asked_attributes, FSAL_ATTR_MODE))
    {

      /* The POSIX chmod call don't affect the symlink object, but
       * the entry it points to. So we must ignore it.
       */
      if(!S_ISLNK(buffstat.st_mode))
        {

          if((p_context->credential.user != 0)
             && (p_context->credential.user != buffstat.st_uid))
            Return(ERR_FSAL_PERM, 0, INDEX_FSAL_setattrs);

          TakeTokenFSCall();
          rc = chmod(fsalpath.path, fsal2unix_mode(attrs.mode));
          errsv = errno;
          ReleaseTokenFSCall();

          if(rc)
            Return(posix2fsal_error(errsv), errsv, INDEX_FSAL_setattrs);

        }

    }

  /***********
   *  CHOWN  *
   ***********/
  /* Only root can change uid and A normal user must be in the group he wants to set */
  if(FSAL_TEST_MASK(attrs.asked_attributes, FSAL_ATTR_OWNER))
    {
      if((p_context->credential.user != 0) &&
         ((p_context->credential.user != buffstat.st_uid) ||
          (p_context->credential.user != attrs.owner)))
        Return(ERR_FSAL_PERM, 0, INDEX_FSAL_setattrs);
    }

  if(FSAL_TEST_MASK(attrs.asked_attributes, FSAL_ATTR_GROUP))
    {

      if(p_context->credential.user != 0 && p_context->credential.user != buffstat.st_uid)
        Return(ERR_FSAL_PERM, 0, INDEX_FSAL_setattrs);
      int in_grp = 0;
      /* set in_grp */
      if(p_context->credential.group == attrs.group)
        in_grp = 1;
      else
        for(i = 0; i < p_context->credential.nbgroups; i++)
          {
            if(in_grp = (attrs.group == p_context->credential.alt_groups[i]))
              break;
          }
      if(p_context->credential.user != 0 && !in_grp)
        Return(ERR_FSAL_PERM, 0, INDEX_FSAL_setattrs);
    }

  if(FSAL_TEST_MASK(attrs.asked_attributes, FSAL_ATTR_OWNER | FSAL_ATTR_GROUP))
    {
      TakeTokenFSCall();
      rc = lchown(fsalpath.path,
                  FSAL_TEST_MASK(attrs.asked_attributes,
                                 FSAL_ATTR_OWNER) ? (int)attrs.owner : -1,
                  FSAL_TEST_MASK(attrs.asked_attributes,
                                 FSAL_ATTR_GROUP) ? (int)attrs.group : -1);
      ReleaseTokenFSCall();
      if(rc)
        Return(posix2fsal_error(errno), errno, INDEX_FSAL_setattrs);
    }

  /***********
   *  UTIME  *
   ***********/
  if(FSAL_TEST_MASK(attrs.asked_attributes, FSAL_ATTR_ATIME | FSAL_ATTR_MTIME))
    {

      struct utimbuf timebuf;

      /* user must be the owner or have read access to modify 'atime' */
      if(FSAL_TEST_MASK(attrs.asked_attributes, FSAL_ATTR_ATIME)
         && (p_context->credential.user != 0)
         && (p_context->credential.user != buffstat.st_uid)
         &&
         ((status =
           fsal_internal_testAccess(p_context, FSAL_R_OK, &buffstat,
                                    NULL)).major != ERR_FSAL_NO_ERROR))
        Return(status.major, status.minor, INDEX_FSAL_setattrs);

      /* user must be the owner or have write access to modify 'mtime' */
      if(FSAL_TEST_MASK(attrs.asked_attributes, FSAL_ATTR_MTIME)
         && (p_context->credential.user != 0)
         && (p_context->credential.user != buffstat.st_uid)
         &&
         ((status =
           fsal_internal_testAccess(p_context, FSAL_W_OK, &buffstat,
                                    NULL)).major != ERR_FSAL_NO_ERROR))
        Return(status.major, status.minor, INDEX_FSAL_setattrs);

      timebuf.actime =
          (FSAL_TEST_MASK(attrs.asked_attributes, FSAL_ATTR_ATIME) ? (time_t) attrs.atime.
           seconds : buffstat.st_atime);
      timebuf.modtime =
          (FSAL_TEST_MASK(attrs.asked_attributes, FSAL_ATTR_MTIME) ? (time_t) attrs.mtime.
           seconds : buffstat.st_mtime);

      TakeTokenFSCall();
      rc = utime(fsalpath.path, &timebuf);
      errsv = errno;
      ReleaseTokenFSCall();
      if(rc)
        Return(posix2fsal_error(errno), errno, INDEX_FSAL_setattrs);

    }

  /* Optionaly fills output attributes. */

  if(p_object_attributes)
    {
      status = POSIXFSAL_getattrs(p_filehandle, p_context, p_object_attributes);

      /* on error, we set a special bit in the mask. */
      if(FSAL_IS_ERROR(status))
        {
          FSAL_CLEAR_MASK(p_object_attributes->asked_attributes);
          FSAL_SET_MASK(p_object_attributes->asked_attributes, FSAL_ATTR_RDATTR_ERR);
        }

    }

  Return(ERR_FSAL_NO_ERROR, 0, INDEX_FSAL_setattrs);

}
