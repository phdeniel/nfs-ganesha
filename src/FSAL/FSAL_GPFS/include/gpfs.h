/*                                                                              */
/* Copyright (C) 2001 International Business Machines                           */
/* All rights reserved.                                                         */
/*                                                                              */
/* This file is part of the GPFS user library.                                  */
/*                                                                              */
/* Redistribution and use in source and binary forms, with or without           */
/* modification, are permitted provided that the following conditions           */
/* are met:                                                                     */
/*                                                                              */
/*  1. Redistributions of source code must retain the above copyright notice,   */
/*     this list of conditions and the following disclaimer.                    */
/*  2. Redistributions in binary form must reproduce the above copyright        */
/*     notice, this list of conditions and the following disclaimer in the      */
/*     documentation and/or other materials provided with the distribution.     */
/*  3. The name of the author may not be used to endorse or promote products    */
/*     derived from this software without specific prior written                */
/*     permission.                                                              */
/*                                                                              */
/* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR         */
/* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES    */
/* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.      */
/* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, */
/* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;  */
/* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,     */
/* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR      */
/* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF       */
/* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                   */
/*                                                                              */
/* @(#)42       1.1.2.122  src/avs/fs/mmfs/ts/util/gpfs.h, mmfs, avs_rhrz 1/23/12 00:54:07 */
/*
 *  Library calls for GPFS interfaces
 */
#ifndef H_GPFS
#define H_GPFS

#include <stddef.h>

/* Define GPFS_64BIT_INODES to map the default interface definitions
   to 64-bit interfaces. Without this define, the 32-bit interface
   is the default. Both interfaces are always present, but the
   define sets the default. The actual mapping can be found near the
   end of this header. */
/* #define GPFS_64BIT_INODES 1 */


#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) && (defined(GPFSDLL) || defined(GWIN))

  /* The following errno values either are missing from Windows errno.h or
     have a conflicting value. Other errno values (e.g. EPERM) are okay. */
  #define GPFS_EALREADY     37      /* Operation already in progress        */
  #define GPFS_EOPNOTSUPP   45      /* Operation not supported              */
  #define GPFS_EDQUOT       69      /* Disk quota exceeded                  */
  #define GPFS_ESTALE       9       /* No filesystem (mapped to EBADF)      */
  #define GPFS_EFORMAT      19      /* Unformatted media (mapped to ENODEV) */

  /* specify the library calling convention */
  #define GPFS_API __stdcall

  /* On Windows, this is a HANDLE as returned by CreateFile() */
  typedef void* gpfs_file_t;

#else /* not WIN32 */

  #define GPFS_API
  /* On UNIX (and Windows SUA) systems, this is a file descriptor as
     returned by open() */
  typedef int gpfs_file_t;

#endif


typedef unsigned int gpfs_uid_t;
typedef long long gpfs_off64_t;
typedef unsigned long long gpfs_uid64_t;

typedef struct gpfs_timestruc
{
  unsigned int tv_sec;
  unsigned int tv_nsec;
} gpfs_timestruc_t;

typedef struct gpfs_timestruc64
{
  long long    tv_sec;
  unsigned int tv_nsec;
} gpfs_timestruc64_t;

#define GPFS_SLITE_SIZE_BIT     0x00000001
#define GPFS_SLITE_BLKSIZE_BIT  0x00000002
#define GPFS_SLITE_BLOCKS_BIT   0x00000004
#define GPFS_SLITE_ATIME_BIT    0x00000010
#define GPFS_SLITE_MTIME_BIT    0x00000020
#define GPFS_SLITE_CTIME_BIT    0x00000040
#define GPFS_SLITE_EXACT_BITS   0x00000077

/* Returns "1" if the attribute is requested to be accurate.
   (On output, indicates the value returned in statbufP is accurate). */
#define GPFS_SLITE(m)         (0 == (m))
#define GPFS_SLITE_SIZET(m)   (0 != ((m) & GPFS_SLITE_SIZE_BIT))
#define GPFS_SLITE_BLKSIZE(m) (0 != ((m) & GPFS_SLITE_BLKSIZE_BIT))
#define GPFS_SLITE_BLOCKS(m)  (0 != ((m) & GPFS_SLITE_BLOCKS_BIT))
#define GPFS_SLITE_ATIME(m)   (0 != ((m) & GPFS_SLITE_ATIME_BIT))
#define GPFS_SLITE_MTIME(m)   (0 != ((m) & GPFS_SLITE_MTIME_BIT))
#define GPFS_SLITE_CTIME(m)   (0 != ((m) & GPFS_SLITE_CTIME_BIT))
#define GPFS_SLITE_EXACT(m)   (GPFS_SLITE_EXACT_BITS == (m))

/* Sets the litemask bit indicating that the attribute should be accurate */
#define GPFS_S_SLITE(m)         (m) = 0
#define GPFS_S_SLITE_SIZET(m)   (m) |= GPFS_SLITE_SIZE_BIT
#define GPFS_S_SLITE_BLKSIZE(m) (m) |= GPFS_SLITE_BLKSIZE_BIT
#define GPFS_S_SLITE_BLOCKS(m)  (m) |= GPFS_SLITE_BLOCKS_BIT
#define GPFS_S_SLITE_ATIME(m)   (m) |= GPFS_SLITE_ATIME_BIT
#define GPFS_S_SLITE_MTIME(m)   (m) |= GPFS_SLITE_MTIME_BIT
#define GPFS_S_SLITE_CTIME(m)   (m) |= GPFS_SLITE_CTIME_BIT
#define GPFS_S_SLITE_EXACT(m)   (m) |= GPFS_SLITE_EXACT_BITS

#define GPFS_STATLITE 0
#define GPFS_NOFOLLOW 1

/* Mapping of buffer for gpfs_getacl, gpfs_putacl. */
typedef struct gpfs_opaque_acl
{
  int            acl_buffer_len;  /* INPUT:  Total size of buffer (including this field).
                                     OUTPUT: Actual size of the ACL information.  */
  unsigned short acl_version;     /* INPUT:  Set to zero.
                                     OUTPUT: Current version of the returned ACL. */
  unsigned char  acl_type;        /* INPUT:  Type of ACL: access (1) or default (2). */
  char           acl_var_data[1]; /* OUTPUT: Remainder of the ACL information. */
} gpfs_opaque_acl_t;

/* ACL types (acl_type field in gpfs_opaque_acl_t or gpfs_acl_t) */
#define GPFS_ACL_TYPE_ACCESS  1
#define GPFS_ACL_TYPE_DEFAULT 2
#define GPFS_ACL_TYPE_NFS4    3

/* gpfs_getacl, gpfs_putacl flag indicating structures instead of the
   opaque style data normally used.  */
#define GPFS_GETACL_STRUCT 0x00000020
#define GPFS_PUTACL_STRUCT 0x00000020

/* gpfs_getacl, gpfs_putacl flag indicating smbd is the caller */
#define GPFS_ACL_SAMBA     0x00000040

/* Defined values for gpfs_aclVersion_t */
#define GPFS_ACL_VERSION_POSIX   1
#define GPFS_ACL_VERSION_NFS4    4

/* Values for gpfs_aclLevel_t  */
#define GPFS_ACL_LEVEL_BASE    0 /* compatible with all acl_version values */
#define GPFS_ACL_LEVEL_V4FLAGS 1 /* requires GPFS_ACL_VERSION_NFS4 */

/* Values for gpfs_aceType_t (ACL_VERSION_POSIX) */
#define GPFS_ACL_USER_OBJ  1
#define GPFS_ACL_GROUP_OBJ 2
#define GPFS_ACL_OTHER     3
#define GPFS_ACL_MASK      4
#define GPFS_ACL_USER      5
#define GPFS_ACL_GROUP     6

/* Values for gpfs_acePerm_t (ACL_VERSION_POSIX) */
#define ACL_PERM_EXECUTE 001
#define ACL_PERM_WRITE   002
#define ACL_PERM_READ    004
#define ACL_PERM_CONTROL 010

/* Values for gpfs_aceType_t (ACL_VERSION_NFS4) */
#define ACE4_TYPE_ALLOW 0
#define ACE4_TYPE_DENY  1
#define ACE4_TYPE_AUDIT 2
#define ACE4_TYPE_ALARM 3

/* Values for gpfs_aceFlags_t (ACL_VERSION_NFS4) */
#define ACE4_FLAG_FILE_INHERIT    0x00000001
#define ACE4_FLAG_DIR_INHERIT     0x00000002
#define ACE4_FLAG_NO_PROPAGATE    0x00000004
#define ACE4_FLAG_INHERIT_ONLY    0x00000008
#define ACE4_FLAG_SUCCESSFUL      0x00000010
#define ACE4_FLAG_FAILED          0x00000020
#define ACE4_FLAG_GROUP_ID        0x00000040
#define ACE4_FLAG_INHERITED       0x00000080

/* GPFS-defined flags.  Placed in a seperate ACL field to avoid
   ever running into newly defined NFSv4 flags. */
#define ACE4_IFLAG_SPECIAL_ID     0x80000000

/* Values for gpfs_aceMask_t (ACL_VERSION_NFS4) */
#define ACE4_MASK_READ         0x00000001
#define ACE4_MASK_LIST_DIR     0x00000001
#define ACE4_MASK_WRITE        0x00000002
#define ACE4_MASK_ADD_FILE     0x00000002
#define ACE4_MASK_APPEND       0x00000004
#define ACE4_MASK_ADD_SUBDIR   0x00000004
#define ACE4_MASK_READ_NAMED   0x00000008
#define ACE4_MASK_WRITE_NAMED  0x00000010
#define ACE4_MASK_EXECUTE      0x00000020

/* The rfc doesn't provide a mask equivalent to "search" ("x" on a
 * directory in posix), but it also doesn't say that its EXECUTE
 * is to have this dual use (even though it does so for other dual
 * use permissions such as read/list.  Going to make the assumption
 * here that the EXECUTE bit has this dual meaning... otherwise
 * we're left with no control over search.
 */
#define ACE4_MASK_SEARCH       0x00000020

#define ACE4_MASK_DELETE_CHILD 0x00000040
#define ACE4_MASK_READ_ATTR    0x00000080
#define ACE4_MASK_WRITE_ATTR   0x00000100
#define ACE4_MASK_DELETE       0x00010000
#define ACE4_MASK_READ_ACL     0x00020000
#define ACE4_MASK_WRITE_ACL    0x00040000
#define ACE4_MASK_WRITE_OWNER  0x00080000
#define ACE4_MASK_SYNCHRONIZE  0x00100000
#define ACE4_MASK_ALL          0x001f01ff

/* Values for gpfs_uid_t (ACL_VERSION_NFS4) */
#define ACE4_SPECIAL_OWNER              1
#define ACE4_SPECIAL_GROUP              2
#define ACE4_SPECIAL_EVERYONE           3

/* per-ACL flags imported from a Windows security descriptor object */
#define ACL4_FLAG_OWNER_DEFAULTED               0x00000100
#define ACL4_FLAG_GROUP_DEFAULTED               0x00000200
#define ACL4_FLAG_DACL_PRESENT                  0x00000400
#define ACL4_FLAG_DACL_DEFAULTED                0x00000800
#define ACL4_FLAG_SACL_PRESENT                  0x00001000
#define ACL4_FLAG_SACL_DEFAULTED                0x00002000
#define ACL4_FLAG_DACL_UNTRUSTED                0x00004000
#define ACL4_FLAG_SERVER_SECURITY               0x00008000
#define ACL4_FLAG_DACL_AUTO_INHERIT_REQ         0x00010000
#define ACL4_FLAG_SACL_AUTO_INHERIT_REQ         0x00020000
#define ACL4_FLAG_DACL_AUTO_INHERITED           0x00040000
#define ACL4_FLAG_SACL_AUTO_INHERITED           0x00080000
#define ACL4_FLAG_DACL_PROTECTED                0x00100000
#define ACL4_FLAG_SACL_PROTECTED                0x00200000
#define ACL4_FLAG_RM_CONTROL_VALID              0x00400000
#define ACL4_FLAG_NULL_DACL                     0x00800000
#define ACL4_FLAG_NULL_SACL                     0x01000000
#define ACL4_FLAG_VALID_FLAGS                   0x01ffff00


/* Externalized ACL defintions */
typedef unsigned int gpfs_aclType_t;
typedef unsigned int gpfs_aclLen_t;
typedef unsigned int gpfs_aclLevel_t;
typedef unsigned int gpfs_aclVersion_t;
typedef unsigned int gpfs_aclCount_t;
typedef unsigned int gpfs_aclFlag_t;

typedef unsigned int gpfs_aceType_t;
typedef unsigned int gpfs_aceFlags_t;
typedef unsigned int gpfs_acePerm_t;
typedef unsigned int gpfs_aceMask_t;

/* A POSIX ACL Entry */
typedef struct gpfs_ace_v1
{
  gpfs_aceType_t  ace_type; /* POSIX ACE type */
  gpfs_uid_t      ace_who;  /* uid/gid */
  gpfs_acePerm_t  ace_perm; /* POSIX permissions */
} gpfs_ace_v1_t;

/* A NFSv4 ACL Entry */
typedef struct gpfs_ace_v4
{
  gpfs_aceType_t  aceType;   /* Allow or Deny */
  gpfs_aceFlags_t aceFlags;  /* Inherit specifications, etc. */
  gpfs_aceFlags_t aceIFlags; /* GPFS Internal flags */
  gpfs_aceMask_t  aceMask;   /* NFSv4 mask specification */
  gpfs_uid_t      aceWho;    /* User/Group identification */
} gpfs_ace_v4_t;

/* when GPFS_ACL_VERSION_NFS4, and GPFS_ACL_LEVEL_V4FLAGS */
typedef struct v4Level1_ext /* ACL extension */
{ 
  gpfs_aclFlag_t acl_flags; /* per-ACL flags */
  gpfs_ace_v4_t ace_v4[1];
} v4Level1_t;

/* The GPFS ACL */
typedef struct gpfs_acl
{
  gpfs_aclLen_t     acl_len;     /* Total length of this ACL in bytes */
  gpfs_aclLevel_t   acl_level;   /* Reserved (must be zero) */
  gpfs_aclVersion_t acl_version; /* POSIX or NFS4 ACL */
  gpfs_aclType_t    acl_type;    /* Access, Default, or NFS4 */
  gpfs_aclCount_t   acl_nace;    /* Number of Entries that follow */
  union
  {
    gpfs_ace_v1_t  ace_v1[1]; /* when GPFS_ACL_VERSION_POSIX */
    gpfs_ace_v4_t  ace_v4[1]; /* when GPFS_ACL_VERSION_NFS4  */
    v4Level1_t     v4Level1;  /* when GPFS_ACL_LEVEL_V4FLAGS */
  };
} gpfs_acl_t;


/* NAME:        gpfs_getacl()
 *
 * FUNCTION:    Retrieves the ACL information for a file.
 *
 *              The aclP parameter must point to a buffer mapped by either:
 *                - gpfs_opaque_acl_t (when flags are zero).  In this case,
 *                  the opaque data that is intended to be used by a backup
 *                  program (restoreed by passing this data back on a subsequent
 *                  call to gpfs_putacl).
 *                - gpfs_acl_t (when GPFS_GETACL_STRUCT is specified).  In this
 *                  case, the data can be interpreted by the calling application
 *                  (and may be modified and applied to the file by passing it
 *                  to gpfs_putacl...along with the GPFS_PUTACL_STRUCT flag).
 *
 *              On input, the first four bytes of the buffer must contain its
 *              total size.
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  function not available
 *              ENOSPC  buffer too small to return the entire ACL.
 *                      Needed size is returned in the first four
 *                      bytes of the buffer pointed to by aclP.
 *              EINVAL  Invalid arguments
 *              ENOTDIR Not on directory
 *              ENOMEM  Out of memory
 */
int GPFS_API
gpfs_getacl(const char *pathname,
            int flags,
            void *acl);


/* NAME:        gpfs_putacl()
 *
 * FUNCTION:    Sets the ACL information for a file.
 *              The buffer passed in should contain the ACL data
 *              that was obtained by a previous call to gpfs_getacl.
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  function not available
 *              EINVAL  Invalid arguments
 *              ENOTDIR Not on directory
 *              ENOMEM  Out of memory
 */
int GPFS_API
gpfs_putacl(const char *pathname,
            int flags,
            void *acl);


/* NAME:        gpfs_prealloc()
 * FUNCTION:    Preallocate disk storage for the file handle that has
 *              already been opened for writing, starting at the specified
 *              starting offset and covering at least the number of bytes
 *              requested. Allocations are rounded to block boundaries
 *              (block size can be found using fstat() in st_blksize.)
 *              Any existing data already in the file will not be modified.
 *              Any read of the preallocated blocks will return zeros.
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  No prealloc service available
 *              EBADF   Bad file desc
 *              EINVAL  Not a GPFS file
 *              EINVAL  Not a regular file
 *              EINVAL  StartOffset or BytesToPrealloc < 0
 *              EACCES  File not opened for writing
 *              EDQUOT  Quota exceeded
 *              ENOSPC  Not enough space on disk
 */
int GPFS_API
gpfs_prealloc(gpfs_file_t fileDesc,
              gpfs_off64_t startOffset,
              gpfs_off64_t bytesToPrealloc);


typedef struct gpfs_winattr
{
  gpfs_timestruc_t creationTime;
  unsigned int winAttrs; /* values as defined below */
} gpfs_winattr_t;

/* winAttrs values */
#define GPFS_WINATTR_ARCHIVE              0x0001
#define GPFS_WINATTR_COMPRESSED           0x0002
#define GPFS_WINATTR_DEVICE               0x0004
#define GPFS_WINATTR_DIRECTORY            0x0008
#define GPFS_WINATTR_ENCRYPTED            0x0010
#define GPFS_WINATTR_HIDDEN               0x0020
#define GPFS_WINATTR_NORMAL               0x0040
#define GPFS_WINATTR_NOT_CONTENT_INDEXED  0x0080
#define GPFS_WINATTR_OFFLINE              0x0100
#define GPFS_WINATTR_READONLY             0x0200
#define GPFS_WINATTR_REPARSE_POINT        0x0400
#define GPFS_WINATTR_SPARSE_FILE          0x0800
#define GPFS_WINATTR_SYSTEM               0x1000
#define GPFS_WINATTR_TEMPORARY            0x2000
#define GPFS_WINATTR_HAS_STREAMS          0x4000


/* NAME:        gpfs_get_winattrs()
 *              gpfs_get_winattrs_path()
 *
 * FUNCTION:    Returns gpfs_winattr_t attributes
 *
 * Returns:      0      Success
 *              -1      Failure
 *
 * Errno:       Specific error indication
 *              EINVAL
 */
int GPFS_API
gpfs_get_winattrs(gpfs_file_t fileDesc, gpfs_winattr_t *attrP);

int GPFS_API
gpfs_get_winattrs_path(const char *pathname, gpfs_winattr_t *attrP);


/* NAME:        gpfs_set_winattrs()
 *              gpfs_set_winattrs_path()
 *
 * FUNCTION:    Sets gpfs_winattr_t attributes (as specified by
 *              the flags).
 *
 * Returns:      0      Success
 *              -1      Failure
 *
 * Errno:       Specific error indication
 *              EINVAL
 */
int GPFS_API
gpfs_set_winattrs(gpfs_file_t fileDesc, int flags, gpfs_winattr_t *attrP);

int GPFS_API
gpfs_set_winattrs_path(const char *pathname, int flags, gpfs_winattr_t *attrP);

/* gpfs_set_winattr flag values */
#define GPFS_WINATTR_SET_CREATION_TIME 0x08
#define GPFS_WINATTR_SET_ATTRS         0x10

/*
 * NAME:        gpfs_set_times(), gpfs_set_times_path()
 *
 * FUNCTION:    Sets sets file access time, modefied time, change time,
 *              and/or creation time (as specified by the flags).
 *
 * Input:       flagsfileDesc : file descriptor of the object to set
 *              pathname      : path to a file or directory
 *              flag          : define time value to set
 *              GPFS_SET_ATIME - set access time
 *              GPFS_SET_MTIME - set mod. time
 *              GPFS_SET_CTIME - set change time
 *              GPFS_SET_CREATION_TIME - set creation time
 *              times         : array to times
 *
 * Returns:      0      Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  function not available
 *              EBADF   Not a GPFS File
 *              EINVAL  invalid argument
 *              EACCES  Permission denied
 *              EROFS   Filesystem is read only
 *              ENOENT  No such file or directory
 */
typedef gpfs_timestruc_t gpfs_times_vector_t[4];

int GPFS_API
gpfs_set_times(gpfs_file_t fileDesc, int flags, gpfs_times_vector_t times);

int GPFS_API
gpfs_set_times_path(char *pathname, int flags, gpfs_times_vector_t times);

/* gpfs_set_times flag values */
#define GPFS_SET_ATIME         0x01
#define GPFS_SET_MTIME         0x02
#define GPFS_SET_CTIME         0x04
#define GPFS_SET_CREATION_TIME 0x08


/* NAME:        gpfs_set_share()
 *
 * FUNCTION:    Acquire shares for Samba
 *
 * Input:       fileDesc : file descriptor
 *              allow    : share type being requested
 *                         GPFS_SHARE_NONE, GPFS_SHARE_READ,
 *                         GPFS_SHARE_WRITE, GPFS_SHARE_BOTH
 *              deny     : share type to deny to others
 *                         GPFS_DENY_NONE, GPFS_DENY_READ,
 *                         GPFS_DENY_WRITE, GPFS_DENY_BOTH
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       Specific error indication
 *              EACCES  share mode not available
 */

/* allow/deny specifications */
#define GPFS_SHARE_NONE   0
#define GPFS_SHARE_READ   1
#define GPFS_SHARE_WRITE  2
#define GPFS_SHARE_BOTH   3
#define GPFS_SHARE_ALL    3
#define GPFS_DENY_NONE    0
#define GPFS_DENY_READ    1
#define GPFS_DENY_WRITE   2
#define GPFS_DENY_BOTH    3
#define GPFS_DENY_DELETE  4
#define GPFS_DENY_ALL     7

int GPFS_API
gpfs_set_share(gpfs_file_t fileDesc,
               unsigned int share,
               unsigned int deny);


/* NAME:        gpfs_set_lease()
 *
 * FUNCTION:    Acquire leases for Samba
 *
 * Input:       fileDesc  : file descriptor
 *              leaseType : lease type being requested
 *                          GPFS_LEASE_NONE GPFS_LEASE_READ,
 *                          GPFS_LEASE_WRITE
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       Specific error indication
 *              EACCES  lease not available
 */

/* leaseType specifications */
#define GPFS_LEASE_NONE    0
#define GPFS_LEASE_READ    1
#define GPFS_LEASE_WRITE   2

int GPFS_API
gpfs_set_lease(gpfs_file_t fileDesc,
               unsigned int leaseType);


/* NAME:        gpfs_get_lease()
 *
 * FUNCTION:    Returns the type of lease currently held
 *
 * Returns:     GPFS_LEASE_READ
 *              GPFS_LEASE_WRITE
 *              GPFS_LEASE_NONE
 *              -1      Failure
 *
 * Errno:       Specific error indication
 *              EINVAL
 */
int GPFS_API
gpfs_get_lease(gpfs_file_t fileDesc);


 /* NAME:        gpfs_get_realfilename(), gpfs_get_realfilename_path()
  *
  * FUNCTION:    Interface to get real name of a file.
  *
  * INPUT:       File descriptor, pathname, buffer, bufferlength
  * OUTPUT:      Real file name stored in filesystem
  * Returns:     0       Successful
  *              -1      Failure
  *
  * Errno:       ENOSYS  function not available
  *              EBADF   Bad file handle
  *              EBADF   Not a GPFS file
  *              EINVAL  Not a regular file
  *              ENOSPC  buffer too small to return the real file name.
  *                      Needed size is returned in buflen parameter.
  *              ENOENT  File does not exist
  *              ENOMEM  Memory allocation failed
  *              EACCESS Permission denied
  */
int GPFS_API
gpfs_get_realfilename(gpfs_file_t fileDesc,
                      char *fileNameP,
                      int *buflen);

int GPFS_API
gpfs_get_realfilename_path(const char *pathname,
                           char *fileNameP,
                           int *buflen);

 /* NAME:        gpfs_ftruncate()
  *
  * FUNCTION:    Interface to truncate a file.
  *
  * INPUT:       File descriptor
  *              length
  * Returns:     0       Successful
  *              -1      Failure
  *
  * Errno:       ENOSYS  function not available
  *              EBADF   Bad file handle
  *              EBADF   Not a GPFS file
  *              EINVAL  Not a regular file
  *              ENOENT  File does not exist
  *              ENOMEM  Memory allocation failed
  *              EINVAL  length < 0
  *              EACCESS  Permission denied
  */
int GPFS_API
gpfs_ftruncate(gpfs_file_t fileDesc, gpfs_off64_t length);

#define GPFS_WIN_CIFS_REGISTERED   0x02000000
typedef struct cifsThreadData_t
{
  unsigned int dataLength; /* Total buffer length */
  unsigned int share;      /* gpfs_set_share declaration */
  unsigned int deny;       /* gpfs_set_share specification */
  unsigned int lease;      /* gpfs_set_lease lease type */
  unsigned int secInfoFlags; /* Future use.  Must be zero */
  gpfs_uid_t   sdUID;      /* Owning user */
  gpfs_uid_t   sdGID;      /* Owning group */
  unsigned int aclLength ; /* Length of the following ACL */
  gpfs_acl_t   acl;        /* The initial ACL for create/mkdir */
} cifsThreadData_t;

 /* NAME:        gpfs_register_cifs_export()
  *
  * FUNCTION:    Register a CIFS export process.
  *
  * INPUT:       implicit use of the process ids
  *              
  * Returns:     0       Successful
  *              ENOSYS  function not available
  *              EACCES  cannot establish credentials
  *              ENOMEM  temporary shortage of memory
  *              EINVAL  prior process/thread registrations exist
  *              EBADF   unable to allocate a file descriptor
  */
int GPFS_API
gpfs_register_cifs_export();

 /* NAME:        gpfs_unregister_cifs_export()
  *
  * FUNCTION:    remove a registration for a CIFS export 
  *
  * INPUT:       implicit use of the process ids
  *              
  * Returns:     0       Successful
  *              ENOSYS  function not available
  *              EACCES  cannot establish credentials
  *              ENOMEM  temporary shortage of memory
  */
int GPFS_API
gpfs_unregister_cifs_export();

 /* NAME:        gpfs_register_cifs_buffer()
  *
  * FUNCTION:    Register a CIFS thread/buffer combination
  *
  * INPUT:       implicit use of the process and thread ids
  *              Address of a cifsThreadData_t structure that will include
  *              a GPFS ACL (GPFS_ACL_VERSION_NFS4/GPFS_ACL_LEVEL_V4FLAGS)              
  *              that can be applied at file/dir creation.              
  *              
  * Returns:     0       Successful
  *              ENOSYS  function not available
  *              EACCES  cannot establish credentials
  *              ENOMEM  unable to allocate required memory
  *              EINVAL  no associated process registrion exists 
  *                      bad dataLength in buffer.
  */
int GPFS_API
gpfs_register_cifs_buffer(cifsThreadData_t *bufP);

 /* NAME:        gpfs_unregister_cifs_buffer()
  *
  * FUNCTION:    remove a CIFS thread/buffer registration
  *
  * INPUT:       implicit use of the process and thread ids
  *              
  * Returns:     0       Successful
  *              ENOSYS  function not available
  *              EACCES  cannot establish credentials
  *              ENOMEM  unable to allocate required memory
  *              EINVAL  no associated process registrion exists
  */
int GPFS_API
gpfs_unregister_cifs_buffer();

/* NAME:        gpfs_lib_init()
 *
 * FUNCTION:    Open GPFS main module device file
 *
 * INPUT:       Flags
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  Function not available
 */
int GPFS_API
gpfs_lib_init(int flags);

/* NAME:        gpfs_lib_term()
 *
 * FUNCTION:    Close GPFS main module device file
 *
 * INPUT:       Flags
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  Function not available
 */
int GPFS_API
gpfs_lib_term(int flags);

/* Define maximum length of the name for a GPFS named object, such
   as a snapshot, storage pool or fileset. The name is a null-terminated
   character string, which is not include in the max length */
#define GPFS_MAXNAMLEN       255

/* Define maximum length of the path to a GPFS named object
   such as a snapshot or fileset. If the absolute path name exceeds
   this limit, then use a relative path name. The path is a null-terminated
   character string, which is not included in the max length */
#define GPFS_MAXPATHLEN     1023

/* ASCII code for "GPFS" in the struct statfs f_type field */
#define GPFS_SUPER_MAGIC     0x47504653

/* GPFS inode attributes
   gpfs_uid_t - defined above
   gpfs_uid64_t - defined above
   gpfs_off64_t - defined above */
typedef unsigned int gpfs_mode_t;
typedef unsigned int gpfs_gid_t;
typedef unsigned long long gpfs_gid64_t;
typedef unsigned int gpfs_ino_t;
typedef unsigned long long gpfs_ino64_t;
typedef unsigned int gpfs_gen_t;
typedef unsigned long long gpfs_gen64_t;
typedef unsigned int gpfs_dev_t;
typedef unsigned int gpfs_mask_t;
typedef unsigned int gpfs_pool_t;
typedef unsigned int gpfs_snapid_t;
typedef unsigned long long gpfs_snapid64_t;
typedef unsigned long long gpfs_fsid64_t[2];
typedef short gpfs_nlink_t;
typedef long long gpfs_nlink64_t;


#if defined(WIN32) || defined(_MS_SUA_)
  typedef struct gpfs_stat64
  {
    gpfs_dev_t         st_dev;        /* id of device containing file */
    gpfs_ino64_t       st_ino;        /* file inode number */
    gpfs_mode_t        st_mode;       /* access mode */
    gpfs_nlink64_t     st_nlink;      /* number of links */
    unsigned int       st_flags;      /* flag word */
    gpfs_uid64_t       st_uid;        /* owner uid */
    gpfs_gid64_t       st_gid;        /* owner gid */
    gpfs_dev_t         st_rdev;       /* device id (if special file) */
    gpfs_off64_t       st_size;       /* file size in bytes */
    gpfs_timestruc64_t st_atime;      /* time of last access */
    gpfs_timestruc64_t st_mtime;      /* time of last data modification */
    gpfs_timestruc64_t st_ctime;      /* time of last status change */
    int                st_blksize;    /* preferred block size for io */
    gpfs_off64_t       st_blocks;     /* 512 byte blocks of disk held by file */
    long long          st_fsid;       /* file system id */
    unsigned int       st_type;       /* file type */
    gpfs_gen64_t       st_gen;        /* inode generation number */
    gpfs_timestruc64_t st_createtime; /* time of creation */
    unsigned int       st_attrs;      /* Window's flags */
  } gpfs_stat64_t;
#else
  typedef struct stat64 gpfs_stat64_t;
#endif

#if defined(WIN32) || defined(_MS_SUA_)
  typedef struct gpfs_statfs64
  {
    gpfs_off64_t       f_blocks;      /* total data blocks in file system */
    gpfs_off64_t       f_bfree;       /* free block in fs */
    gpfs_off64_t       f_bavail;      /* free blocks avail to non-superuser */
    int                f_bsize;       /* optimal file system block size */
    gpfs_ino64_t       f_files;       /* total file nodes in file system */
    gpfs_ino64_t       f_ffree;       /* free file nodes in fs */
    gpfs_fsid64_t      f_fsid;        /* file system id */
    int                f_fsize;       /* fundamental file system block size */
    int                f_sector_size; /* logical disk sector size */
    char               f_fname[32];   /* file system name (usually mount pt.) */
    char               f_fpack[32];   /* file system pack name */
    int                f_name_max;    /* maximum component name length for posix */
  } gpfs_statfs64_t;
#else
  typedef struct statfs64 gpfs_statfs64_t;
#endif

/* Declarations for backwards compatibility. */
typedef gpfs_stat64_t stat64_t;
typedef gpfs_statfs64_t statfs64_t;


/* Define a version number for the directory entry data to allow
   future changes in this structure. Careful callers should also use
   the d_reclen field for the size of the structure rather than sizeof,
   to allow some degree of forward compatibility */
#define GPFS_D_VERSION 1

typedef struct gpfs_direntx
{
  int            d_version;     /* this struct's version */
  unsigned short d_reclen;      /* actual size of this struct including
                                   null terminated variable length d_name */
  unsigned short d_type;        /* Types are defined below */
  gpfs_ino_t     d_ino;         /* File inode number */
  gpfs_gen_t     d_gen;         /* Generation number for the inode */
  char           d_name[256];   /* null terminated variable length name */
} gpfs_direntx_t;


#define GPFS_D64_VERSION 2

typedef struct gpfs_direntx64
{
  int            d_version;     /* this struct's version */
  unsigned short d_reclen;      /* actual size of this struct including
                                   null terminated variable length d_name */
  unsigned short d_type;        /* Types are defined below */
  gpfs_ino64_t   d_ino;         /* File inode number */
  gpfs_gen64_t   d_gen;         /* Generation number for the inode */
  unsigned int   d_flags;       /* Flags are defined below */
  char           d_name[1028];  /* null terminated variable length name */
                                /* (1020+null+7 byte pad to double word) */
                                /* to handle up to 255 UTF-8 chars */
} gpfs_direntx64_t;

/* File types for d_type field in gpfs_direntx_t */
#define GPFS_DE_OTHER    0
#define GPFS_DE_DIR      4
#define GPFS_DE_REG      8
#define GPFS_DE_LNK     10
#define GPFS_DE_DEL     16

/* Define flags for gpfs_direntx64_t */
#define GPFS_DEFLAG_NONE      0x0000 /* Default value, no flags set */
#define GPFS_DEFLAG_JUNCTION  0x0001 /* DirEnt is a fileset junction */
#define GPFS_DEFLAG_IJUNCTION 0x0002 /* DirEnt is a inode space junction */
#define GPFS_DEFLAG_ORPHAN    0x0004 /* DirEnt is an orphan (pcache) */


/* Define a version number for the iattr data to allow future changes
   in this structure. Careful callers should also use the ia_reclen field
   for the size of the structure rather than sizeof, to allow some degree
   of forward compatibility */
#define GPFS_IA_VERSION 1
#define GPFS_IA64_VERSION 2
#define GPFS_IA64_RESERVED 4
#define GPFS_IA64_UNUSED 10

typedef struct gpfs_iattr
{
  int              ia_version;    /* this struct version */
  int              ia_reclen;     /* sizeof this structure */
  int              ia_checksum;   /* validity check on iattr struct */
  gpfs_mode_t      ia_mode;       /* access mode */
  gpfs_uid_t       ia_uid;        /* owner uid */
  gpfs_gid_t       ia_gid;        /* owner gid */
  gpfs_ino_t       ia_inode;      /* file inode number */
  gpfs_gen_t       ia_gen;        /* inode generation number */
  gpfs_nlink_t     ia_nlink;      /* number of links */
  short            ia_flags;      /* Flags (defined below) */
  int              ia_blocksize;  /* preferred block size for io */
  gpfs_mask_t      ia_mask;       /* Initial attribute mask (not used) */
  unsigned int     ia_pad1;       /* reserved space */
  gpfs_off64_t     ia_size;       /* file size in bytes */
  gpfs_off64_t     ia_blocks;     /* 512 byte blocks of disk held by file */
  gpfs_timestruc_t ia_atime;      /* time of last access */
  gpfs_timestruc_t ia_mtime;      /* time of last data modification */
  gpfs_timestruc_t ia_ctime;      /* time of last status change */
  gpfs_dev_t       ia_rdev;       /* id of device */
  unsigned int     ia_xperm;      /* extended attributes (defined below) */
  unsigned int     ia_modsnapid;  /* snapshot id of last modification */
  unsigned int     ia_filesetid;  /* fileset ID */
  unsigned int     ia_datapoolid; /* storage pool ID for data */
  unsigned int     ia_pad2;       /* reserved space */

} gpfs_iattr_t;


typedef struct gpfs_iattr64
{
  int                ia_version;    /* this struct version */
  int                ia_reclen;     /* sizeof this structure */
  int                ia_checksum;   /* validity check on iattr struct */
  gpfs_mode_t        ia_mode;       /* access mode */
  gpfs_uid64_t       ia_uid;        /* owner uid */
  gpfs_gid64_t       ia_gid;        /* owner gid */
  gpfs_ino64_t       ia_inode;      /* file inode number */
  gpfs_gen64_t       ia_gen;        /* inode generation number */
  gpfs_nlink64_t     ia_nlink;      /* number of links */
  gpfs_off64_t       ia_size;       /* file size in bytes */
  gpfs_off64_t       ia_blocks;     /* 512 byte blocks of disk held by file */
  gpfs_timestruc64_t ia_atime;      /* time of last access */
  unsigned int       ia_winflags;   /* window's flags (defined below) */
  unsigned int       ia_pad1;       /* reserved space */
  gpfs_timestruc64_t ia_mtime;      /* time of last data modification */
  unsigned int       ia_flags;      /* flags (defined below) */
  int                ia_pad2;       /* reserved space */
  gpfs_timestruc64_t ia_ctime;      /* time of last status change */
  int                ia_blocksize;  /* preferred block size for io */
  unsigned int       ia_pad3;       /* reserved space */
  gpfs_timestruc64_t ia_createtime; /* creation time */
  gpfs_mask_t        ia_mask;       /* initial attribute mask (not used) */
  int                ia_pad4;       /* reserved space */
  unsigned int       ia_reserved[GPFS_IA64_RESERVED]; /* reserved space */
  unsigned int       ia_xperm;      /* extended attributes (defined below) */
  gpfs_dev_t         ia_dev;        /* id of device containing file */
  gpfs_dev_t         ia_rdev;       /* device id (if special file) */
  unsigned int       ia_pcacheflags; /* pcache inode bits */
  gpfs_snapid64_t    ia_modsnapid;  /* snapshot id of last modification */
  unsigned int       ia_filesetid;  /* fileset ID */
  unsigned int       ia_datapoolid; /* storage pool ID for data */
  gpfs_ino64_t       ia_inode_space_mask; /* inode space mask of this file system */
                                          /* This value is saved in the iattr structure
                                             during backup and used during restore */
  unsigned int       ia_unused[GPFS_IA64_UNUSED];  /* reserved space */
} gpfs_iattr64_t;

/* Define flags for inode attributes */
#define GPFS_IAFLAG_SNAPDIR         0x0001 /* (obsolete) */
#define GPFS_IAFLAG_USRQUOTA        0x0002 /* inode is a user quota file */
#define GPFS_IAFLAG_GRPQUOTA        0x0004 /* inode is a group quota file */
#define GPFS_IAFLAG_ERROR           0x0008 /* error reading inode */
/* Define flags for inode replication attributes */
#define GPFS_IAFLAG_FILESET_ROOT    0x0010 /* root dir of a fileset */
#define GPFS_IAFLAG_NO_SNAP_RESTORE 0x0020 /* don't restore from snapshots */
#define GPFS_IAFLAG_FILESETQUOTA    0x0040 /* inode is a fileset quota file */
#define GPFS_IAFLAG_COMANAGED       0x0080 /* file data is co-managed */
#define GPFS_IAFLAG_ILLPLACED       0x0100 /* may not be properly placed */
#define GPFS_IAFLAG_REPLMETA        0x0200 /* metadata replication set */
#define GPFS_IAFLAG_REPLDATA        0x0400 /* data replication set */
#define GPFS_IAFLAG_EXPOSED         0x0800 /* may have data on suspended disks */
#define GPFS_IAFLAG_ILLREPLICATED   0x1000 /* may not be properly replicated */
#define GPFS_IAFLAG_UNBALANCED      0x2000 /* may not be properly balanced */
#define GPFS_IAFLAG_DATAUPDATEMISS  0x4000 /* has stale data blocks on
                                              unavailable disk */
#define GPFS_IAFLAG_METAUPDATEMISS  0x8000 /* has stale metadata on
                                              unavailable disk */

#define GPFS_IAFLAG_IMMUTABLE       0x00010000 /* Immutability */
#define GPFS_IAFLAG_INDEFRETENT     0x00020000 /* Indefinite retention */
#define GPFS_IAFLAG_SECUREDELETE    0x00040000 /* Secure deletion */

#define GPFS_IAFLAG_TRUNCMANAGED    0x00080000 /* dmapi truncate event enabled */
#define GPFS_IAFLAG_READMANAGED     0x00100000 /* dmapi read event enabled */
#define GPFS_IAFLAG_WRITEMANAGED    0x00200000 /* dmapi write event enabled */

#define GPFS_IAFLAG_APPENDONLY      0x00400000 /* AppendOnly only */

/* Define flags for window's attributes */
#define GPFS_IWINFLAG_ARCHIVE       0x0001 /* Archive */
#define GPFS_IWINFLAG_HIDDEN        0x0002 /* Hidden */
#define GPFS_IWINFLAG_NOTINDEXED    0x0004 /* Not content indexed */
#define GPFS_IWINFLAG_OFFLINE       0x0008 /* Off-line */
#define GPFS_IWINFLAG_READONLY      0x0010 /* Read-only */
#define GPFS_IWINFLAG_REPARSE       0x0020 /* Reparse point */
#define GPFS_IWINFLAG_SYSTEM        0x0040 /* System */
#define GPFS_IWINFLAG_TEMPORARY     0x0080 /* Temporary */
#define GPFS_IWINFLAG_COMPRESSED    0x0100 /* Compressed */
#define GPFS_IWINFLAG_ENCRYPTED     0x0200 /* Encrypted */
#define GPFS_IWINFLAG_SPARSE        0x0400 /* Sparse file */
#define GPFS_IWINFLAG_HASSTREAMS    0x0800 /* Has streams */

/* Define flags for extended attributes */
#define GPFS_IAXPERM_ACL            0x0001 /* file has acls */
#define GPFS_IAXPERM_XATTR          0x0002 /* file has extended attributes */
#define GPFS_IAXPERM_DMATTR         0x0004 /* file has dm attributes */
#define GPFS_IAXPERM_DOSATTR        0x0008 /* file has non-default dos attrs */
#define GPFS_IAXPERM_RPATTR         0x0010 /* file has restore policy attrs */

/* Define flags for pcache bits defined in the inode */
#define GPFS_ICAFLAG_CACHED   0x0001  // "cached complete"
#define GPFS_ICAFLAG_CREATE   0x0002  // "created"
#define GPFS_ICAFLAG_DIRTY    0x0004  // "data dirty"
#define GPFS_ICAFLAG_LINK     0x0008  // "hard linked"
#define GPFS_ICAFLAG_SETATTR  0x0010  // "attr changed"
#define GPFS_ICAFLAG_LOCAL    0x0020  // "local"
#define GPFS_ICAFLAG_APPEND   0x0040  // "append"
#define GPFS_ICAFLAG_STATE    0x0080  // "has remote state"

/* Define pointers to interface types */
typedef struct gpfs_fssnap_handle gpfs_fssnap_handle_t;
typedef struct gpfs_iscan gpfs_iscan_t;
typedef struct gpfs_ifile gpfs_ifile_t;
typedef struct gpfs_restore gpfs_restore_t;

typedef struct gpfs_fssnap_id
{
  char opaque[48];
} gpfs_fssnap_id_t;


/* Define extended return codes for gpfs backup & restore
   calls without an explicit return code will return the value in errno */
#define GPFS_ERRNO_BASE  190
#define GPFS_E_INVAL_FSSNAPID       (GPFS_ERRNO_BASE+0) /* invalid fssnap id */
#define GPFS_E_INVAL_ISCAN          (GPFS_ERRNO_BASE+1) /* invalid iscan pointer */
#define GPFS_E_INVAL_IFILE          (GPFS_ERRNO_BASE+2) /* invalid ifile pointer */
#define GPFS_E_INVAL_IATTR          (GPFS_ERRNO_BASE+3) /* invalid iattr structure */
#define GPFS_E_INVAL_RESTORE        (GPFS_ERRNO_BASE+4) /* invalid restore pointer */
#define GPFS_E_INVAL_FSSNAPHANDLE   (GPFS_ERRNO_BASE+5) /* invalid fssnap handle */
#define GPFS_E_INVAL_SNAPNAME       (GPFS_ERRNO_BASE+6) /* invalid snapshot name */
#define GPFS_E_FS_NOT_RESTORABLE    (GPFS_ERRNO_BASE+7) /* FS is not clean */
#define GPFS_E_RESTORE_NOT_ENABLED  (GPFS_ERRNO_BASE+8) /* Restore was not enabled */
#define GPFS_E_RESTORE_STARTED      (GPFS_ERRNO_BASE+9) /* Restore is running */
#define GPFS_E_INVAL_XATTR          (GPFS_ERRNO_BASE+10) /* invalid extended
                                                            attribute pointer */

/* Define flags parameter for get/put file attributes.
   Used by gpfs_fgetattr, gpfs_fputattr, gpfs_fputattrwithpath
   and gpfs_igetattrsx, gpfs_iputattrsx
*/
#define GPFS_ATTRFLAG_DEFAULT        0x0000  /* default behavior */
#define GPFS_ATTRFLAG_NO_PLACEMENT   0x0001  /* exclude file placement attributes */
#define GPFS_ATTRFLAG_IGNORE_POOL    0x0002  /* saved poolid is not valid */
#define GPFS_ATTRFLAG_USE_POLICY     0x0004  /* use restore policy rules to
                                                determine poolid */
#define GPFS_ATTRFLAG_INCL_DMAPI     0x0008  /* Include dmapi attributes */
#define GPFS_ATTRFLAG_FINALIZE_ATTRS 0x0010  /* Finalize immutability attributes */
#define GPFS_ATTRFLAG_SKIP_IMMUTABLE 0x0020  /* Skip immutable attributes */

/* Define structure used by gpfs_statfspool */
typedef struct gpfs_statfspool_s
{
  gpfs_off64_t f_blocks;     /* total data blocks in pool */
  gpfs_off64_t f_bfree;      /* free blocks in pool */
  gpfs_off64_t f_bavail;     /* free blocks avail to non-superuser */
  gpfs_off64_t f_mblocks;    /* total metadata blocks in pool */
  gpfs_off64_t f_mfree;      /* free blocks avail for system metadata */
  int          f_bsize;      /* optimal storage pool block size */
  int          f_files;      /* total file nodes assigned to pool */
  gpfs_pool_t  f_poolid;     /* storage pool id */
  int          f_fsize;      /* fundamental file system block size */
  unsigned int f_usage;      /* data and/or metadata stored in pool */
  int          f_reserved[7];/* Current unused and set to  zero */
} gpfs_statfspool_t;

#define STATFSPOOL_USAGE_DATA      0x0001 /* Pool stores user data */
#define STATFSPOOL_USAGE_METADATA  0x0002 /* Pool stores system metadata */


/* NAME:        gpfs_fstat(), gpfs_stat()
 *
 * FUNCTION:    Get exact stat information for a file descriptor (or filename).
 *              Forces all other nodes to flush dirty data and metadata to disk.
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  function not available
 *              EBADF   Bad file desc
 *              EINVAL  Not a GPFS file
 *              ESTALE  cached fs information was invalid
 */
int GPFS_API
gpfs_fstat(gpfs_file_t fileDesc,
           gpfs_stat64_t *buffer);

int GPFS_API
gpfs_stat(const char *pathname, /* File pathname */
          gpfs_stat64_t *buffer);

/* NAME:        gpfs_statfs64()
 *
 * FUNCTION:    Get information about the file system.
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  function not available
 *              EBADF   Bad file desc
 *              EINVAL  Not a GPFS file
 *              ESTALE  cached fs information was invalid
 */
int GPFS_API
gpfs_statfs64(const char *pathname, /* File pathname */
              gpfs_statfs64_t *buffer);

/* NAME:        gpfs_statlite()
 *              gpfs_lstatlite() - do not follow a symlink at the end of the path
 *
 * FUNCTION:    Returns stat() information with specified accuracy
 *
 * Input:       pathname    : path to a file or directory
 *
 * In/Out:      st_litemaskP: bitmask specification of required accuracy
 *              statbufP    : buffer for returned stat information
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       Specific error indication
 *              EINVAL
 *
 */
int GPFS_API
gpfs_statlite(const char *pathname,
              unsigned int *st_litemaskP,
              gpfs_stat64_t *statbufP);

int GPFS_API
gpfs_lstatlite(const char *pathname,
               unsigned int *st_litemaskP,
               gpfs_stat64_t *statbufP);


/* NAME:        gpfs_fgetattrs()
 *
 * FUNCTION:    Retrieves all extended file attributes in opaque format.
 *              This function together with gpfs_fputattrs is intended for
 *              use by a backup program to save (gpfs_fgetattrs) and
 *              restore (gpfs_fputattrs) all extended file attributes
 *              (ACLs, user attributes, ...) in one call.
 *
 *              NOTE: This call does not return extended attributes used for
 *                    the Data Storage Management (XDSM) API (aka DMAPI).
 *
 * Input:       flags   Define behavior of get attributes
 *                GPFS_ATTRFLAG_NO_PLACEMENT - file attributes for placement
 *                      are not saved, neither is the current storage pool.
 *                GPFS_ATTRFLAG_IGNORE_POOL - file attributes for placement
 *                      are saved, but the current storage pool is not.
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  function not available
 *              EINVAL  Not a GPFS file
 *              EINVAL  invalid flags provided
 *              ENOSPC  buffer too small to return all attributes
 *                      *attrSizeP will be set to the size necessary
 */
int GPFS_API
gpfs_fgetattrs(gpfs_file_t fileDesc,
               int flags,
               void *bufferP,
               int bufferSize,
               int *attrSizeP);


/* NAME:        gpfs_fputattrs()
 *
 * FUNCTION:    Sets all extended file attributes of a file
 *              and sets the file's storage pool and data replication
 *              to the values saved in the extended attributes.
 *
 *              If the saved storage pool is not valid or if the IGNORE_POOL
 *              flag is set, then it will select the storage pool by matching
 *              a PLACEMENT rule using the saved file attributes.
 *              If it fails to match a placement rule or if there are
 *              no placement rules installed it will assign the file
 *              to the "system" storage pool.
 *
 *              The buffer passed in should contain extended attribute data
 *              that was obtained by a previous call to gpfs_fgetattrs.
 *
 * Input:       flags   Define behavior of put attributes
 *                GPFS_ATTRFLAG_NO_PLACEMENT - file attributes are restored
 *                      but the storage pool and data replication are unchanged
 *                GPFS_ATTRFLAG_IGNORE_POOL - file attributes are restored
 *                      but the storage pool and data replication are selected
 *                      by matching the saved attributes to a placement rule
 *                      instead of restoring the saved storage pool.
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  function not available
 *              EINVAL  Not a GPFS file
 *              EINVAL  the buffer does not contain valid attribute data
 *              EINVAL  invalid flags provided
 */
int GPFS_API
gpfs_fputattrs(gpfs_file_t fileDesc,
               int flags,
               void *bufferP);


/* NAME:        gpfs_fputattrswithpathname()
 *
 * FUNCTION:    Sets all extended file attributes of a file and invokes
 *              the policy engine to match a RESTORE rule using the file's
 *              attributes saved in the extended attributes to set the
 *              file's storage pool and data replication. The caller should
 *              include the full path to the file, including the file name,
 *              to allow rule selection based on file name or path.
 *
 *              If the file fails to match a RESTORE rule, or if there are
 *              no RESTORE rules installed, then the storage pool and data
 *              replication are selected as when calling gpfs_fputattrs().
 *
 *              The buffer passed in should contain extended attribute data
 *              that was obtained by a previous call to gpfs_fgetattrs.
 *
 *              pathName is a UTF-8 encoded string. On Windows, applications
 *              can convert UTF-16 ("Unicode") to UTF-8 using the platforms
 *              WideCharToMultiByte function.
 *
 *
 * Input:       flags   Define behavior of put attributes
 *                GPFS_ATTRFLAG_NO_PLACEMENT - file attributes are restored
 *                      but the storage pool and data replication are unchanged
 *                GPFS_ATTRFLAG_IGNORE_POOL - file attributes are restored
 *                      but if the file fails to match a RESTORE rule, it
 *                      ignore the saved storage pool and select a pool
 *                      by matching the saved attributes to a PLACEMENT rule.
 *                GPFS_ATTRFLAG_SKIP_IMMUTABLE - Skip immutable/appendOnly flags
 *                      before restoring file data. Then use GPFS_ATTRFLAG_FINALIZE_ATTRS
 *                      to restore immutable/appendOnly flags after data is restored.
 *                GPFS_ATTRFLAG_FINALIZE_ATTRS - file attributes that are restored
 *                      after data is retored. If file is immutable/appendOnly
 *                      call without this flag before restoring data
 *                      then call with this flag after restoring data
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  function not available
 *              EINVAL  Not a GPFS file
 *              EINVAL  the buffer does not contain valid attribute data
 *              ENOENT  invalid pathname
 *              EINVAL  invalid flags provided
 */
int GPFS_API
gpfs_fputattrswithpathname(gpfs_file_t fileDesc,
                           int flags,
                           void *bufferP,
                           const char *pathName);


/* NAME:        gpfs_get_fssnaphandle_by_path()
 *
 * FUNCTION:    Get a volatile handle to uniquely identify a filesystem
 *              and snapshot by the path to the file system and snapshot
 *
 * Input:       pathName: path to a file or directory in a gpfs file system
 *                        or to one of its snapshots
 *
 * Returns:     pointer to gpfs_fssnap_handle_t (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EINVAL  Not a GPFS file
 *              ENOENT invalid pathname
 *              see system calls open(), fstatfs(), and malloc() ERRORS
 */
gpfs_fssnap_handle_t * GPFS_API
gpfs_get_fssnaphandle_by_path(const char *pathName);


/* NAME:        gpfs_get_fssnaphandle_by_name()
 *
 * FUNCTION:    Get a volatile handle to uniquely identify a filesystem
 *              and snapshot by the file system name and snapshot name.
 *
 * Input:       fsName: unique name for gpfs file system (may be specified
 *                      as fsName or /dev/fsName)
 *              snapName: name for snapshot within that file system
 *                        or NULL to access the active file system rather
 *                        than a snapshot within the file system.
 *
 * Returns:     pointer to gpfs_fssnap_handle_t (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              ENOENT invalid file system name
 *              GPFS_E_INVAL_SNAPNAME invalid snapshot name
 *              see system calls open(), fstatfs(), and malloc() ERRORS
 */
gpfs_fssnap_handle_t * GPFS_API
gpfs_get_fssnaphandle_by_name(const char *fsName,
                              const char *snapName);


/* NAME:        gpfs_get_fssnaphandle_by_fssnapid()
 *
 * FUNCTION:    Get a volatile handle to uniquely identify a filesystem
 *              and snapshot by a fssnapId created from a previous handle.
 *
 * Input:       fssnapId: unique id for a file system and snapshot
 *
 * Returns:     pointer to gpfs_fssnap_handle_t (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              GPFS_E_INVAL_FSSNAPID invalid snapshot id
 *              see system calls open(), fstatfs(), and malloc() ERRORS
 */
gpfs_fssnap_handle_t * GPFS_API
gpfs_get_fssnaphandle_by_fssnapid(const gpfs_fssnap_id_t *fssnapId);

/* NAME:        gpfs_get_fset_snaphandle_by_path()
 *
 * FUNCTION:    Get a volatile handle to uniquely identify an inode space within a
 *              filesyetsm and snapshot by the path to the file system and snapshot.
 *
 * Input:       pathName: path to a file or directory in a gpfs file system
 *                        or to one of its snapshots
 *
 * Returns:     pointer to gpfs_fssnap_handle_t (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EINVAL  Not a GPFS file
 *              ENOENT invalid pathname
 *              see system calls open(), fstatfs(), and malloc() ERRORS
 */
gpfs_fssnap_handle_t * GPFS_API
gpfs_get_fset_snaphandle_by_path(const char *pathName);

/* NAME:        gpfs_get_fset_snaphandle_by_name()
 *
 * FUNCTION:    Get a volatile handle to uniquely identify an inode space within a
 *              filesystem and snapshot by the independent fileset name, file system
 *              name and snapshot name.
 *
 * Input:       fsName: unique name for gpfs file system (may be specified
 *                      as fsName or /dev/fsName)
 *              fsetName name of the independent fileset that owns the inode space
 *              snapName: name for snapshot within that file system
 *                        or NULL to access the active file system rather
 *                        than a snapshot within the file system.
 *
 * Returns:     pointer to gpfs_fssnap_handle_t (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              ENOENT invalid file system name
 *              GPFS_E_INVAL_FSETNAME invalid fset nsmae
 *              GPFS_E_INVAL_SNAPNAME invalid snapshot name
 *              see system calls open(), fstatfs(), and malloc() ERRORS
 */
gpfs_fssnap_handle_t * GPFS_API
gpfs_get_fset_snaphandle_by_name(const char *fsName,
                                 const char *fsetName,
                                 const char *snapName);

/* NAME:        gpfs_get_fset_snaphandle_by_fset_snapid()
 *
 * FUNCTION:    Get a volatile handle to uniquely identify a filesystem
 *              and snapshot by a fssnapId created from a previous handle.
 *
 * Input:       fssnapId: unique id for a file system and snapshot
 *
 * Returns:     pointer to gpfs_fssnap_handle_t (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              GPFS_E_INVAL_FSSNAPID invalid snapshot id
 *              see system calls open(), fstatfs(), and malloc() ERRORS
 */
gpfs_fssnap_handle_t * GPFS_API
gpfs_get_fset_snaphandle_by_fset_snapid(const gpfs_fssnap_id_t *fsetsnapId);

/* NAME:        gpfs_get_pathname_from_fssnaphandle()
 *
 * FUNCTION:    Get the mountpoint and path to a file system
 *              and snapshot identified by a fssnapHandle
 *
 * Input:       fssnapHandle: ptr to file system & snapshot handle
 *
 * Returns:     ptr to path name to the file system  (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              GPFS_E_INVAL_FSSNAPHANDLE invalid fssnapHandle
 */
const char * GPFS_API
gpfs_get_pathname_from_fssnaphandle(gpfs_fssnap_handle_t *fssnapHandle);


/* NAME:        gpfs_get_fsname_from_fssnaphandle()
 *
 * FUNCTION:    Get the unique name for the file system
 *              identified by a fssnapHandle
 *
 * Input:       fssnapHandle: ptr to file system & snapshot handle
 *
 * Returns:     ptr to name of the file system  (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              GPFS_E_INVAL_FSSNAPHANDLE invalid fssnapHandle
 */
const char * GPFS_API
gpfs_get_fsname_from_fssnaphandle(gpfs_fssnap_handle_t *fssnapHandle);


/* NAME:        gpfs_get_snapname_from_fssnaphandle()
 *
 * FUNCTION:    Get the name for the snapshot
 *              uniquely identified by a fssnapHandle
 *
 * Input:       fssnapHandle: ptr to file system & snapshot handle
 *
 * Returns:     ptr to name assigned to the snapshot (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              GPFS_E_INVAL_FSSNAPHANDLE invalid fssnaphandle
 *              GPFS_E_INVAL_SNAPNAME snapshot has been deleted
 *
 * Notes:       If the snapshot has been deleted from the file system
 *              the snapId may still be valid, but the call will fail
 *              with errno set to GPFS_E_INVAL_SNAPNAME.
 */
const char * GPFS_API
gpfs_get_snapname_from_fssnaphandle(gpfs_fssnap_handle_t *fssnapHandle);


/* NAME:        gpfs_get_snapid_from_fssnaphandle()
 *
 * FUNCTION:    Get the numeric id for the snapshot identified
 *              by a fssnapHandle. The snapshots define an ordered
 *              sequence of changes to each file. The file's iattr
 *              structure defines the snapshot id in which the file
 *              was last modified (ia_modsnapid). This numeric value
 *              can be compared to the numeric snapid from a fssnaphandle
 *              to determine if the file changed before or after the
 *              snapshot identified by the fssnaphandle.
 *
 * Input:       fssnapHandle: ptr to file system & snapshot handle
 *
 * Returns:     Numeric id for the snapshot referred to by the fssnaphandle
 *              0 if the fssnaphandle does not refer to a snapshot
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              GPFS_E_INVAL_FSSNAPHANDLE invalid fssnaphandle
 *
 * Notes:       The snapshot need not be on-line to determine the
 *              snapshot's numeric id.
 */
gpfs_snapid_t GPFS_API
gpfs_get_snapid_from_fssnaphandle(gpfs_fssnap_handle_t *fssnapHandle);

gpfs_snapid64_t GPFS_API
gpfs_get_snapid_from_fssnaphandle64(gpfs_fssnap_handle_t *fssnapHandle);


/* NAME:        gpfs_get_fssnapid_from_fssnaphandle()
 *
 * FUNCTION:    Get a unique, non-volatile file system and snapshot id
 *              for the file system and snapshot identified by a
 *              volatile fssnap handle.
 *
 * Input:       fssnapHandle: ptr to file system & snapshot handle
 *              fssnapId: returned fssnapId uniquely identifying the
 *                        file system and snapshot being scanned
 *
 * Returns:     0 and fssnapId is set with id (Successful)
 *              -1 and errno is set (Failure)
 *
 * Errno:       GPFS_E_INVAL_FSSNAPHANDLE invalid fssnaphandle
 *              EINVAL null ptr given for returned fssnapId
 *              EFAULT size mismatch for fssnapId
 */
int GPFS_API
gpfs_get_fssnapid_from_fssnaphandle(gpfs_fssnap_handle_t *fssnapHandle,
                                    gpfs_fssnap_id_t *fssnapId);


/* NAME:        gpfs_get_restore_fssnapid_from_fssnaphandle()
 *
 * FUNCTION:    Get the unique, non-volatile file system and snapshot id
 *              used for the last complete restore of a mirrored file
 *              system. The file system must been a previous restore
 *              target and ready for additional incremental restore.
 *
 * Input:       fssnapHandle: ptr to file system & snapshot handle
 *              fssnapId: returned fssnapId uniquely identifying the
 *                        last complete restored file system.
 *
 * Returns:     0 and fssnapId is set with id (Successful)
 *              -1 and errno is set (Failure)
 *
 * Errno:       GPFS_E_INVAL_FSSNAPHANDLE invalid fssnaphandle
 *              EINVAL null ptr given for returned fssnapId
 *              EFAULT size mismatch for fssnapId
 *              EPERM caller must have superuser privilege
 *              ENOMEM unable to allocate memory for request
 *              GPFS_E_FS_NOT_RESTORABLE fs is not clean for restore
 */
int GPFS_API
gpfs_get_restore_fssnapid_from_fssnaphandle(gpfs_fssnap_handle_t *fssnapHandle,
                                            gpfs_fssnap_id_t *fssnapId);

/* NAME:        gpfs_free_fssnaphandle()
 *
 * FUNCTION:    Free a fssnapHandle
 *
 * Input:       fssnapHandle: ptr to file system & snapshot handle
 *
 * Returns:     void
 *
 * Errno:       None
 */
void GPFS_API
gpfs_free_fssnaphandle(gpfs_fssnap_handle_t *fssnapHandle);

/* NAME:        gpfs_get_snapdirname()
 *
 * FUNCTION:    Get the name of the directory containing snapshots.
 *
 * Input:       fssnapHandle: handle for the file system
 *              snapdirName: buffer into which the name of the snapshot
 *                directory will be copied
 *              bufLen: the size of the provided buffer
 *
 * Returns:     0 (Successful)
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              ENOMEM unable to allocate memory for request
 *              GPFS_E_INVAL_FSSNAPHANDLE fssnapHandle is invalid
 *              E2BIG buffer too small to return the snapshot directory name
 */
int GPFS_API
gpfs_get_snapdirname(gpfs_fssnap_handle_t *fssnapHandle,
                     char *snapdirName,
                     int bufLen);


/* NAME:        gpfs_open_inodescan()
 *
 * FUNCTION:    Open inode file for inode scan.
 *
 * Input:       fssnapHandle: handle for file system and snapshot
 *                            to be scanned
 *              prev_fssnapId: if NULL, all inodes of existing file will
 *                be returned; if non-null, only returns inodes of files
 *                that have changed since the specified previous snapshot
 *              maxIno: if non-null, returns the maximum inode number
 *                available in the inode file being scanned.
 *
 * Returns:     pointer to gpfs_iscan_t (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EINVAL bad parameters
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              ENOMEM unable to allocate memory for request
 *              GPFS_E_INVAL_FSSNAPHANDLE fssnapHandle is invalid
 *              GPFS_E_INVAL_FSSNAPID prev_fssnapId is invalid
 *              EDOM prev_fssnapId is from a different fs
 *              ERANGE prev_fssnapId is same as or more recent than snapId
 *                     being scanned
 *              see system calls dup() and malloc() ERRORS
 */
gpfs_iscan_t * GPFS_API
gpfs_open_inodescan(gpfs_fssnap_handle_t *fssnapHandle,
                    const gpfs_fssnap_id_t *prev_fssnapId,
                    gpfs_ino_t *maxIno);

gpfs_iscan_t * GPFS_API
gpfs_open_inodescan64(gpfs_fssnap_handle_t *fssnapHandle,
                      const gpfs_fssnap_id_t *prev_fssnapId,
                      gpfs_ino64_t *maxIno);


/* NAME:        gpfs_open_inodescan_with_xattrs()
 *
 * FUNCTION:    Open inode file and extended attributes for an inode scan
 *
 * Input:       fssnapHandle: handle for file system and snapshot
 *                            to be scanned
 *              prev_fssnapId: if NULL, all inodes of existing file will
 *                be returned; if non-null, only returns inodes of files
 *                that have changed since the specified previous snapshot
 *              nxAttrs: count of extended attributes to be returned.
 *                if nxAttrs is set to 0, call returns no extended
 *                attributes, like gpfs_open_inodescan.
 *                if nxAttrs is set to -1, call returns all extended attributes
 *              xAttrList: pointer to array of pointers to names of extended
 *                attribute to be returned. nxAttrList may be null if nxAttrs
 *                is set to 0 or -1.
 *              maxIno: if non-null, returns the maximum inode number
 *                available in the inode file being scanned.
 *
 * Returns:     pointer to gpfs_iscan_t (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EINVAL bad parameters
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              ENOMEM unable to allocate memory for request
 *              GPFS_E_INVAL_FSSNAPHANDLE fssnapHandle is invalid
 *              GPFS_E_INVAL_FSSNAPID prev_fssnapId is invalid
 *              EDOM prev_fssnapId is from a different fs
 *              ERANGE prev_fssnapId is same as or more recent than snapId
 *                     being scanned
 *              see system calls dup() and malloc() ERRORS
 */
gpfs_iscan_t * GPFS_API
gpfs_open_inodescan_with_xattrs(gpfs_fssnap_handle_t *fssnapHandle,
                                const gpfs_fssnap_id_t *prev_fssnapId,
                                int nxAttrs,
                                const char *xattrsList[],
                                gpfs_ino_t *maxIno);

gpfs_iscan_t * GPFS_API
gpfs_open_inodescan_with_xattrs64(gpfs_fssnap_handle_t *fssnapHandle,
                                  const gpfs_fssnap_id_t *prev_fssnapId,
                                  int nxAttrs,
                                  const char *xattrList[],
                                  gpfs_ino64_t *maxIno);


/* NAME:        gpfs_next_inode()
 *
 * FUNCTION:    Get next inode from inode scan. Scan terminates before
 *              the last inode specified or the last inode in the
 *              inode file being scanned.
 *
 * Input:       iscan: ptr to inode scan descriptor
 *              termIno: scan terminates before this inode number
 *                caller may specify maxIno from gpfs_open_inodescan()
 *                or 0 to scan the entire inode file.
 *              iattr: pointer to returned pointer to file's iattr.
 *
 * Returns:     0 and *iattr set to point to gpfs_iattr_t (Successful)
 *              0 and *iattr set to NULL for no more inodes before termIno
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              ENOMEM buffer too small
 *              GPFS_E_INVAL_ISCAN bad parameters
 *
 * Notes:       The data returned by gpfs_next_inode() is overwritten by
 *              subsequent calls to gpfs_next_inode() or gpfs_seek_inode().
 *
 *              The termIno parameter provides a means to partition an
 *              inode scan such that it may be executed on more than one node.
 */
int GPFS_API
gpfs_next_inode(gpfs_iscan_t *iscan,
                gpfs_ino_t termIno,
                const gpfs_iattr_t **iattr);

int GPFS_API
gpfs_next_inode64(gpfs_iscan_t *iscan,
                  gpfs_ino64_t termIno,
                  const gpfs_iattr64_t **iattr);


/* NAME:        gpfs_next_inode_with_xattrs()
 *
 * FUNCTION:    Get next inode and its extended attributes from the inode scan.
 *              The set of extended attributes returned were defined when
 *              the inode scan was opened. The scan terminates before the last
 *              inode specified or the last inode in the inode file being
 *              scanned.
 *
 * Input:       iscan: ptr to inode scan descriptor
 *              termIno: scan terminates before this inode number
 *                caller may specify maxIno from gpfs_open_inodescan()
 *                or 0 to scan the entire inode file.
 *              iattr: pointer to returned pointer to file's iattr.
 *              xattrBuf: pointer to returned pointer to xattr buffer
 *              xattrBufLen: returned length of xattr buffer
 *
 *
 * Returns:     0 and *iattr set to point to gpfs_iattr_t (Successful)
 *              0 and *iattr set to NULL for no more inodes before termIno
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              EFAULT buffer data was overwritten
 *              ENOMEM buffer too small
 *              GPFS_E_INVAL_ISCAN bad parameters
 *              GPFS_E_INVAL_XATTR bad parameters
 *
 * Notes:       The data returned by gpfs_next_inode() is overwritten by
 *              subsequent calls to gpfs_next_inode(), gpfs_seek_inode()
 *              or gpfs_stat_inode().
 *
 *              The termIno parameter provides a means to partition an
 *              inode scan such that it may be executed on more than one node.
 *
 *              The returned values for xattrBuf and xattrBufLen must be
 *              provided to gpfs_next_xattr() to obtain the extended attribute
 *              names and values. The buffer used for the extended attributes
 *              is overwritten by subsequent calls to gpfs_next_inode(),
 *              gpfs_seek_inode() or gpfs_stat_inode();
 *
 *              The returned pointers to the extended attribute name and value
 *              will be aligned to a double-word boundary.
 */
int GPFS_API
gpfs_next_inode_with_xattrs(gpfs_iscan_t *iscan,
                            gpfs_ino_t termIno,
                            const gpfs_iattr_t **iattr,
                            const char **xattrBuf,
                            unsigned int *xattrBufLen);

int GPFS_API
gpfs_next_inode_with_xattrs64(gpfs_iscan_t *iscan,
                              gpfs_ino64_t termIno,
                              const gpfs_iattr64_t **iattr,
                              const char **xattrBuf,
                              unsigned int *xattrBufLen);


/* NAME:        gpfs_next_xattr()
 *
 * FUNCTION:    Iterate over the extended attributes buffer returned
 *              by get_next_inode_with_xattrs to return the individual
 *              attributes and their values. Note that the attribute names
 *              are null-terminated strings, whereas the atttribute value
 *              contains binary data.
 *
 * Input:       iscan: ptr to inode scan descriptor
 *              xattrBufLen: ptr to attribute buffer length
 *              xattrBuf: ptr to the ptr to the attribute buffer
 *
 * Returns:     0 and *name set to point attribue name (Successful)
 *                also sets: *valueLen to length of attribute value
 *                           *value to point to attribute value
 *                           *xattrBufLen to remaining length of buffer
 *                           **xattrBuf to index next attribute in buffer
 *              0 and *name set to NULL for no more attributes in buffer
 *                also sets: *valueLen to 0
 *                           *value to NULL
 *                           *xattrBufLen to 0
 *                           **xattrBuf to NULL
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              GPFS_E_INVAL_ISCAN invalid iscan parameter
 *              GPFS_E_INVAL_XATTR invalid xattr parameters
 *
 * Notes:       The caller is not allowed to modify the returned attribute
 *              names or values.  The data returned by gpfs_next_attribute()
 *              may be overwritten by subsequent calls to gpfs_next_attribute()
 *              or other gpfs library calls.
 */
int GPFS_API
gpfs_next_xattr(gpfs_iscan_t *iscan,
                const char **xattrBuf,
                unsigned int *xattrBufLen,
                const char **name,
                unsigned int *valueLen,
                const char **value);



/* NAME:        gpfs_seek_inode()
 *
 * FUNCTION:    Seek to a given inode number.
 *
 * Input:       iscan: ptr to inode scan descriptor
 *              ino: next inode number to be scanned
 *
 * Returns:     0       Successful
 *              -1      Failure and errno is set
 *
 * Errno:       ENOSYS function not available
 *              GPFS_E_INVAL_ISCAN bad parameters
 */
int GPFS_API
gpfs_seek_inode(gpfs_iscan_t *iscan,
                gpfs_ino_t ino);

int GPFS_API
gpfs_seek_inode64(gpfs_iscan_t *iscan,
                  gpfs_ino64_t ino);


/* NAME:        gpfs_stat_inode()
 * NAME:        gpfs_stat_inode_with_xattrs()
 *
 * FUNCTION:    Seek to the specified inode and get that inode and
 *              its extended attributes from the inode scan. This is
 *              simply a combination of gpfs_seek_inode and get_next_inode
 *              but will only return the specified inode.
 *
 * Input:       iscan: ptr to inode scan descriptor
 *              ino: inode number to be returned
 *              termIno: prefetch inodes up to this inode
 *                caller may specify maxIno from gpfs_open_inodescan()
 *                or 0 to allow prefetching over the entire inode file.
 *              iattr: pointer to returned pointer to file's iattr.
 *              xattrBuf: pointer to returned pointer to xattr buffer
 *              xattrBufLen: returned length of xattr buffer
 *
 * Returns:     0 and *iattr set to point to gpfs_iattr_t (Successful)
 *              0 and *iattr set to NULL for no more inodes before termIno
 *                or if requested inode does not exist.
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              ENOMEM buffer too small
 *              GPFS_E_INVAL_ISCAN bad parameters
 *
 * Notes:       The data returned by gpfs_next_inode() is overwritten by
 *              subsequent calls to gpfs_next_inode(), gpfs_seek_inode()
 *              or gpfs_stat_inode().
 *
 *              The termIno parameter provides a means to partition an
 *              inode scan such that it may be executed on more than one node.
 *              It is only used by this call to control prefetching.
 *
 *              The returned values for xattrBuf and xattrBufLen must be
 *              provided to gpfs_next_xattr() to obtain the extended attribute
 *              names and values. The buffer used for the extended attributes
 *              is overwritten by subsequent calls to gpfs_next_inode(),
 *              gpfs_seek_inode() or gpfs_stat_inode();
 */
int GPFS_API
gpfs_stat_inode(gpfs_iscan_t *iscan,
                gpfs_ino_t ino,
                gpfs_ino_t termIno,
                const gpfs_iattr_t **iattr);

int GPFS_API
gpfs_stat_inode64(gpfs_iscan_t *iscan,
                  gpfs_ino64_t ino,
                  gpfs_ino64_t termIno,
                  const gpfs_iattr64_t **iattr);

int GPFS_API
gpfs_stat_inode_with_xattrs(gpfs_iscan_t *iscan,
                            gpfs_ino_t ino,
                            gpfs_ino_t termIno,
                            const gpfs_iattr_t **iattr,
                            const char **xattrBuf,
                            unsigned int *xattrBufLen);

int GPFS_API
gpfs_stat_inode_with_xattrs64(gpfs_iscan_t *iscan,
                              gpfs_ino64_t ino,
                              gpfs_ino64_t termIno,
                              const gpfs_iattr64_t **iattr,
                              const char **xattrBuf,
                              unsigned int *xattrBufLen);


/* NAME:        gpfs_close_inodescan()
 *
 * FUNCTION:    Close inode file.
 *
 * Input:       iscan: ptr to inode scan descriptor
 *
 * Returns:     void
 *
 * Errno:       None
 */
void GPFS_API
gpfs_close_inodescan(gpfs_iscan_t *iscan);


/* NAME:        gpfs_cmp_fssnapid()
 *
 * FUNCTION:    Compare two fssnapIds for the same file system to
 *              determine the order in which the two snapshots were taken.
 *              The 'result' variable will be set as follows:
 *                *result < 0:  snapshot 1 was taken before snapshot 2
 *                *result == 0: snapshot 1 and 2 are the same
 *                *result > 0:  snapshot 1 was taken after snapshot 2
 *
 * Input:      fssnapId1: ptr to fssnapId 1
 *             fssnapId2: ptr to fssnapId id 2
 *             result: ptr to returned results
 *
 * Returns:     0 and *result is set as described above (Successful)
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              GPFS_E_INVAL_FSSNAPID fssnapid1 or fssnapid2 is not a
 *                valid snapshot id
 *              EDOM the two snapshots cannot be compared because
 *                they were taken from two different file systems.
 */
int GPFS_API
gpfs_cmp_fssnapid(const gpfs_fssnap_id_t *fssnapId1,
                  const gpfs_fssnap_id_t *fssnapId2,
                  int *result);


/* NAME:        gpfs_iopen()
 *
 * FUNCTION:    Open a file or directory by inode number.
 *
 * Input: fssnapHandle: handle for file system and snapshot
 *                      being scanned
 *        ino: inode number
 *        open_flags: O_RDONLY for gpfs_iread()
 *                    O_WRONLY for gpfs_iwrite()
 *                    O_CREAT create the file if it doesn't exist
 *                    O_TRUNC if the inode already exists delete it
 *           caller may use GPFS_O_BACKUP to read files for backup
 *                      and GPFS_O_RESTORE to write files for restore
 *        statxbuf: used only with O_CREAT/GPFS_O_BACKUP
 *                  all other cases set to NULL
 *        symLink: used only with O_CREAT/GPFS_O_BACKUP for a symbolic link
 *                 all other cases set to NULL
 *
 * Returns:     pointer to gpfs_ifile_t (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EINVAL missing or bad parameter
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              ENOMEM unable to allocate memory for request
 *              EFORMAT invalid fs version number
 *              EIO error reading original inode
 *              ERANGE error ino is out of range, should use gpfs_iopen64
 *              GPFS_E_INVAL_IATTR iattr structure was corrupted
 *              see dup() and malloc() ERRORS
 */
gpfs_ifile_t * GPFS_API
gpfs_iopen(gpfs_fssnap_handle_t *fssnapHandle,
           gpfs_ino_t ino,
           int open_flags,
           const gpfs_iattr_t *statxbuf,
           const char *symLink);

gpfs_ifile_t * GPFS_API
gpfs_iopen64(gpfs_fssnap_handle_t *fssnapHandle,
             gpfs_ino64_t ino,
             int open_flags,
             const gpfs_iattr64_t *statxbuf,
             const char *symLink);


/* Define gpfs_iopen flags as used by the backup & restore by inode.
   The backup code will only read the source files.
   The restore code writes the target files & creates them if they
   don't already exist. The file length is set by the inode attributes.
   Consequently, to restore a user file it is unnecessary to include
   the O_TRUNC flag. */
#define GPFS_O_BACKUP  (O_RDONLY)
#define GPFS_O_RESTORE (O_WRONLY | O_CREAT)


/* NAME:        gpfs_iread()
 *
 * FUNCTION:    Read file opened by gpfs_iopen.
 *
 * Input:       ifile:      pointer to gpfs_ifile_t from gpfs_iopen
 *              buffer:     buffer for data to be read
 *              bufferSize: size of buffer (ie amount of data to be read)
 * In/Out       offset:     offset of where within the file to read
 *                          if successful, offset will be updated to the
 *                          next byte after the last one that was read
 *
 * Returns:     number of bytes read (Successful)
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EISDIR file is a directory
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              GPFS_E_INVAL_IFILE bad ifile parameters
 *              see system call read() ERRORS
 */
int GPFS_API
gpfs_iread(gpfs_ifile_t *ifile,
           void *buffer,
           int bufferSize,
           gpfs_off64_t *offset);


/* NAME:        gpfs_iwrite()
 *
 * FUNCTION:    Write file opened by gpfs_iopen.
 *
 * Input:       ifile:    pointer to gpfs_ifile_t from gpfs_iopen
 *              buffer:   the data to be written
 *              writeLen: how much to write
 * In/Out       offset:   offset of where within the file to write
 *                        if successful, offset will be updated to the
 *                        next byte after the last one that was written
 *
 * Returns:     number of bytes written (Successful)
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EISDIR file is a directory
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              GPFS_E_INVAL_IFILE bad ifile parameters
 *              see system call write() ERRORS
 */
int GPFS_API
gpfs_iwrite(gpfs_ifile_t *ifile,
            void *buffer,
            int writeLen,
            gpfs_off64_t *offset);


/* NAME:        gpfs_ireaddir()
 *
 * FUNCTION:    Get next directory entry.
 *
 * Input:       idir:   pointer to gpfs_ifile_t from gpfs_iopen
 *              dirent: pointer to returned pointer to directory entry
 *
 * Returns:     0 and pointer to gpfs_direntx set (Successful)
 *              0 and pointer to gpfs_direntx set to NULL (End of directory)
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              ENOTDIR file is not a directory
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              GPFS_E_INVAL_IFILE bad ifile parameter
 *              ENOMEM unable to allocate memory for request
 *
 * Notes:       The data returned by gpfs_ireaddir() is overwritten by
 *              subsequent calls to gpfs_ireaddir().
 */
int GPFS_API
gpfs_ireaddir(gpfs_ifile_t *idir,
              const gpfs_direntx_t **dirent);

int GPFS_API
gpfs_ireaddir64(gpfs_ifile_t *idir,
                const gpfs_direntx64_t **dirent);


int GPFS_API
gpfs_ireaddirx(gpfs_ifile_t *idir,
               gpfs_iscan_t *iscan,      /* in only  */
               const gpfs_direntx_t **dirent);

int GPFS_API
gpfs_ireaddirx64(gpfs_ifile_t *idir,
                 gpfs_iscan_t *iscan,      /* in only  */
                 const gpfs_direntx64_t **dirent);


/* NAME:        gpfs_iwritedir()
 *
 * FUNCTION:    Create a directory entry in a directory opened by gpfs_iopen.
 *
 * Input:       idir:   pointer to gpfs_ifile_t from gpfs_iopen
 *              dirent: directory entry to be written
 *
 * Returns:     0 (Successful)
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              GPFS_E_INVAL_IFILE bad file pointer
 *              ENOTDIR file is not a directory
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              ENOMEM unable to allocate memory for request
 *              EFORMAT invalid dirent version number
 *              see system call write() ERRORS
 */
int GPFS_API
gpfs_iwritedir(gpfs_ifile_t *idir,
               const gpfs_direntx_t *dirent);

int GPFS_API
gpfs_iwritedir64(gpfs_ifile_t *idir,
                 const gpfs_direntx64_t *dirent);


/* NAME:        gpfs_igetattrs()
 *
 * FUNCTION:    Retrieves all extended file attributes in opaque format.
 *              This function together with gpfs_iputattrs is intended for
 *              use by a backup program to save (gpfs_igetattrs) and
 *              restore (gpfs_iputattrs) all extended file attributes
 *              (ACLs, user attributes, ...) in one call.
 *
 *              NOTE: This call does not return extended attributes used for
 *                    the Data Storage Management (XDSM) API (aka DMAPI).
 *
 * Input:       ifile:      pointer to gpfs_ifile_t from gpfs_iopen
 *              buffer:     pointer to buffer for returned attributes
 *              bufferSize: size of buffer
 *              attrSize:   ptr to returned size of attributes
 *
 * Returns:     0       Successful
 *              -1      Failure and errno is set
 *
 * Errno:       ENOSYS  function not available
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              ENOSPC  buffer too small to return all attributes
 *                      *attrSizeP will be set to the size necessary
 *              GPFS_E_INVAL_IFILE bad ifile parameters
 */
int GPFS_API
gpfs_igetattrs(gpfs_ifile_t *ifile,
               void *buffer,
               int bufferSize,
               int *attrSize);

/* NAME:        gpfs_igetattrsx()
 *
 * FUNCTION:    Retrieves all extended file attributes in opaque format.
 *              This function together with gpfs_iputattrsx is intended for
 *              use by a backup program to save (gpfs_igetattrsx) and
 *              restore (gpfs_iputattrsx) all extended file attributes
 *              (ACLs, user attributes, ...) in one call.
 *
 *              NOTE: This call can optionally return extended attributes
 *                    used for the Data Storage Management (XDSM) API
 *                    (aka DMAPI).
 *
 * Input:       ifile:      pointer to gpfs_ifile_t from gpfs_iopen
 *              flags   Define behavior of get attributes
 *                GPFS_ATTRFLAG_NO_PLACEMENT - file attributes for placement
 *                      are not saved, neither is the current storage pool.
 *                GPFS_ATTRFLAG_IGNORE_POOL - file attributes for placement
 *                      are saved, but the current storage pool is not.
 *                GPFS_ATTRFLAG_INCL_DMAPI - file attributes for dmapi are
 *                      included in the returned buffer
 *
 *              buffer:     pointer to buffer for returned attributes
 *              bufferSize: size of buffer
 *              attrSize:   ptr to returned size of attributes
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  function not available
 *              EINVAL  Not a GPFS file
 *              EINVAL  invalid flags provided
 *              ENOSPC  buffer too small to return all attributes
 *                      *attrSizeP will be set to the size necessary
 */
int GPFS_API
gpfs_igetattrsx(gpfs_ifile_t *ifile,
                int flags,
                void *buffer,
                int bufferSize,
                int *attrSize);


/* NAME:        gpfs_iputattrs()
 *
 * FUNCTION:    Sets all extended file attributes of a file.
 *              The buffer passed in should contain extended attribute data
 *              that was obtained by a previous call to gpfs_igetattrs.
 *
 *              NOTE: This call will not restore extended attributes
 *                    used for the Data Storage Management (XDSM) API
 *                    (aka DMAPI). They will be silently ignored.
 *
 * Input:       ifile:  pointer to gpfs_ifile_t from gpfs_iopen
 *              buffer: pointer to buffer for returned attributes
 *
 * Returns:     0       Successful
 *              -1      Failure and errno is set
 *
 * Errno:       ENOSYS  function not available
 *              EINVAL  the buffer does not contain valid attribute data
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              GPFS_E_INVAL_IFILE bad ifile parameters
 */
int GPFS_API
gpfs_iputattrs(gpfs_ifile_t *ifile,
               void *buffer);


/* NAME:        gpfs_iputattrsx()
 *
 * FUNCTION:    Sets all extended file attributes of a file.
 *
 *              This routine can optionally invoke the policy engine
 *              to match a RESTORE rule using the file's attributes saved
 *              in the extended attributes to set the file's storage pool and
 *              data replication as when calling gpfs_fputattrswithpathname.
 *              When used with the policy the caller should include the
 *              full path to the file, including the file name, to allow
 *              rule selection based on file name or path.
 *
 *              By default, the routine will not use RESTORE policy rules
 *              for data placement. The pathName parameter will be ignored
 *              and may be set to NULL.
 *
 *              If the call does not use RESTORE policy rules, or if the
 *              file fails to match a RESTORE rule, or if there are no
 *              RESTORE rules installed, then the storage pool and data
 *              replication are selected as when calling gpfs_fputattrs().
 *
 *              The buffer passed in should contain extended attribute data
 *              that was obtained by a previous call to gpfs_fgetattrs.
 *
 *              pathName is a UTF-8 encoded string. On Windows, applications
 *              can convert UTF-16 ("Unicode") to UTF-8 using the platforms
 *              WideCharToMultiByte function.
 *
 *              NOTE: This call will restore extended attributes
 *                    used for the Data Storage Management (XDSM) API
 *                    (aka DMAPI) if they are present in the buffer.
 *
 * Input:       ifile:  pointer to gpfs_ifile_t from gpfs_iopen
 *              flags   Define behavior of put attributes
 *                GPFS_ATTRFLAG_NO_PLACEMENT - file attributes are restored
 *                      but the storage pool and data replication are unchanged
 *                GPFS_ATTRFLAG_IGNORE_POOL - file attributes are restored
 *                      but the storage pool and data replication are selected
 *                      by matching the saved attributes to a placement rule
 *                      instead of restoring the saved storage pool.
 *                GPFS_ATTRFLAG_USE_POLICY - file attributes are restored
 *                      but the storage pool and data replication are selected
 *                      by matching the saved attributes to a RESTORE rule
 *                      instead of restoring the saved storage pool.
 *                GPFS_ATTRFLAG_FINALIZE_ATTRS - file attributes that are restored
 *                      after data is retored. If file is immutable/appendOnly
 *                      call without this flag before restoring data
 *                      then call with this flag after restoring data
 *
 *              buffer: pointer to buffer for returned attributes
 *              pathName: pointer to file path and file name for file
 *                        May be set to NULL.
 *
 * Returns:     0       Successful
 *              -1      Failure and errno is set
 *
 * Errno:       ENOSYS  function not available
 *              EINVAL  the buffer does not contain valid attribute data
 *              EINVAL  invalid flags provided
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              GPFS_E_INVAL_IFILE bad ifile parameters
 */
int GPFS_API
gpfs_iputattrsx(gpfs_ifile_t *ifile,
                int flags,
                void *buffer,
                const char *pathName);


/* NAME:        gpfs_igetfilesetname()
 *
 * FUNCTION:    Retrieves the name of the fileset which contains this file.
 *              The fileset name is a null-terminated string, with a
 *              a maximum length of GPFS_MAXNAMLEN.
 *
 * Input:       iscan:      ptr to gpfs_iscan_t from gpfs_open_inodescan()
 *              filesetId:  ia_filesetId returned in an iattr from the iscan
 *              buffer:     pointer to buffer for returned fileset name
 *              bufferSize: size of buffer
 *
 * Returns:     0       Successful
 *              -1      Failure and errno is set
 *
 * Errno:       ENOSYS  function not available
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              ENOSPC  buffer too small to return fileset name
 *              GPFS_E_INVAL_ISCAN bad iscan parameter
 */
int GPFS_API
gpfs_igetfilesetname(gpfs_iscan_t *iscan,
                     unsigned int filesetId,
                     void *buffer,
                     int bufferSize);


/* NAME:        gpfs_igetstoragepool()
 *
 * FUNCTION:    Retrieves the name of the storage pool assigned for
 *              this file's data. The storage pool name is a null-terminated
 *              string, with a maximum length of GPFS_MAXNAMLEN.
 *
 * Input:       iscan:      ptr to gpfs_iscan_t from gpfs_open_inodescan()
 *              dataPoolId: ia_dataPoolId returned in an iattr from the iscan
 *              buffer:     pointer to buffer for returned attributes
 *              bufferSize: size of buffer
 *
 * Returns:     0       Successful
 *              -1      Failure and errno is set
 *
 * Errno:       ENOSYS  function not available
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              ENOSPC  buffer too small to return all storage pool name
 *              GPFS_E_INVAL_ISCAN bad iscan parameters
 */
int GPFS_API
gpfs_igetstoragepool(gpfs_iscan_t *iscan,
                     unsigned int dataPoolId,
                     void *buffer,
                     int bufferSize);


/* NAME:        gpfs_iclose()
 *
 * FUNCTION:    Close file opened by inode and update dates.
 *
 * Input:       ifile:   pointer to gpfs_ifile_t from gpfs_iopen
 *
 * Returns:     void
 *
 * Errno:       ENOSYS function not available
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 */
void GPFS_API
gpfs_iclose(gpfs_ifile_t *ifile);


/* NAME:        gpfs_ireadlink()
 *
 * FUNCTION:    Read symbolic link by inode number.
 *
 * Input:       fssnapHandle: handle for file system & snapshot being scanned
 *              ino:        inode number of link file to read
 *              buffer:     pointer to buffer for returned link data
 *              bufferSize: size of the buffer
 *
 * Returns:     number of bytes read (Successful)
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              GPFS_E_INVAL_FSSNAPHANDLE invalid fssnap handle
 *              see system call readlink() ERRORS
 */
int GPFS_API
gpfs_ireadlink(gpfs_fssnap_handle_t *fssnapHandle,
               gpfs_ino_t ino,
               char *buffer,
               int bufferSize);

int GPFS_API
gpfs_ireadlink64(gpfs_fssnap_handle_t *fssnapHandle,
               gpfs_ino64_t ino,
               char *buffer,
               int bufferSize);


/* NAME:        gpfs_sync_fs()
 *
 * FUNCTION:    sync file system.
 *
 * Input:       fssnapHandle: handle for file system being restored
 *
 * Returns:      0 all data flushed to disk (Successful)
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS  function not available
 *              ENOMEM unable to allocate memory for request
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              GPFS_E_INVAL_FSSNAPHANDLE invalid fssnapHandle
 */
int GPFS_API
gpfs_sync_fs(gpfs_fssnap_handle_t *fssnapHandle);


/* NAME:        gpfs_enable_restore()
 *
 * FUNCTION:    Mark file system as enabled for restore on/off
 *
 * Input:       fssnapHandle: handle for file system to be enabled
 *                            or disabled for restore
 *              on_off:   flag set to 1 to enable restore
 *                                    0 to disable restore
 *
 * Returns:      0 (Successful)
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EINVAL bad parameters
 *              GPFS_E_INVAL_FSSNAPHANDLE invalid fssnapHandle
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              ENOMEM unable to allocate memory for request
 *              E_FS_NOT_RESTORABLE fs is not clean
 *              EALREADY fs already marked as requested
 *              E_RESTORE_STARTED restore in progress
 *
 * Notes: EALREADY indicates enable/disable restore was already called
 * for this fs. The caller must decide if EALREADY represents an
 * error condition.
 */
int GPFS_API
gpfs_enable_restore(gpfs_fssnap_handle_t *fssnapHandle,
                    int on_off);


/* NAME:        gpfs_start_restore()
 *
 * FUNCTION:    Start a restore session.
 *
 * Input:       fssnapHandle: handle for file system to be restored
 *              restore_flags: Flag to indicate the restore should be started
 *                             even if a prior restore has not completed.
 *              old_fssnapId: fssnapId of last restored snapshot
 *              new_fssnapId: fssnapId of snapshot being restored
 *
 * Returns:     pointer to gpfs_restore_t (Successful)
 *              NULL and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              ENOMEM unable to allocate memory for request
 *              EINVAL missing parameter
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              EDOM restore fs does not match existing fs
 *              ERANGE restore is missing updates
 *              EFORMAT invalid fs version number
 *              GPFS_E_INVAL_FSSNAPHANDLE invalid fssnaphandle
 *              GPFS_E_INVAL_FSSNAPID bad fssnapId parameter
 *              E_FS_NOT_RESTORABLE fs is not clean for restore
 *              E_RESTORE_NOT_ENABLED fs is not enabled for restore
 *              EALREADY Restore already in progress
 *
 * Note: EALREADY indicates start restore was already called for
 * this fs. This could be due to a prior restore process that failed
 * or it could be due to a concurrent restore process still running.
 * The caller must decide if EALREADY represents an error condition.
 */
gpfs_restore_t * GPFS_API
gpfs_start_restore(gpfs_fssnap_handle_t *fssnapHandle,
                   int restore_flags,
                   const gpfs_fssnap_id_t *old_fssnapId,
                   const gpfs_fssnap_id_t *new_fssnapId);

#define GPFS_RESTORE_NORMAL 0   /* Restore not started if prior restore
                                   has not completed. */
#define GPFS_RESTORE_FORCED 1   /* Restore starts even if prior restore
                                   has not completed. */


/* NAME:        gpfs_end_restore()
 *
 * FUNCTION:    End a restore session.
 *
 * Input:       restoreId: ptr to gpfs_restore_t
 *
 * Returns:     0 (Successful)
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EINVAL bad parameters
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              GPFS_E_INVAL_RESTORE bad restoreId parameter
 *              GPFS_E_FS_NOT_RESTORABLE fs is not clean for restore
 *              GPFS_E_RESTORE_NOT_ENABLED fs is not enabled for restore
 *              EALREADY Restore already ended
 *
 * Note: EALREADY indicates end restore was already called for
 * this fs. This could be due to a concurrent restore process that
 * already completed. The caller must decide if EALREADY represents
 * an error condition.
 */
int GPFS_API
gpfs_end_restore(gpfs_restore_t *restoreId);


/* NAME:        gpfs_ireadx()
 *
 * FUNCTION:    Block level incremental read on a file opened by gpfs_iopen
 *              with a given incremental scan opened via gpfs_open_inodescan.
 *
 * Input:       ifile:      ptr to gpfs_ifile_t returned from gpfs_iopen()
 *              iscan:      ptr to gpfs_iscan_t from gpfs_open_inodescan()
 *              buffer:     ptr to buffer for returned data
 *              bufferSize: size of buffer for returned data
 *              offset:     ptr to offset value
 *              termOffset: read terminates before reading this offset
 *                          caller may specify ia_size for the file's
 *                          gpfs_iattr_t or 0 to scan the entire file.
 *              hole:       ptr to returned flag to indicate a hole in the file
 *
 * Returns:     number of bytes read and returned in buffer
 *              or size of hole encountered in the file. (Success)
 *              -1 and errno is set (Failure)
 *
 *              On input, *offset contains the offset in the file
 *              at which to begin reading to find a difference same file
 *              in a previous snapshot specified when the inodescan was opened.
 *              On return, *offset contains the offset of the first
 *              difference.
 *
 *              On return, *hole indicates if the change in the file
 *              was data (*hole == 0) and the data is returned in the
 *              buffer provided. The function's value is the amount of data
 *              returned. If the change is a hole in the file,
 *              *hole != 0 and the size of the changed hole is returned
 *              as the function value.
 *
 *              A call with a NULL buffer pointer will query the next increment
 *              to be read from the current offset. The *offset, *hole and
 *              returned length will be set for the next increment to be read,
 *              but no data will be returned. The bufferSize parameter is
 *              ignored, but the termOffset parameter will limit the
 *              increment returned.
 *
 * Errno:       ENOSYS function not available
 *              EINVAL missing or bad parameter
 *              EISDIR file is a directory
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              ENOMEM unable to allocate memory for request
 *              EDOM fs snapId does match local fs
 *              ERANGE previous snapId is more recent than scanned snapId
 *              GPFS_E_INVAL_IFILE bad ifile parameter
 *              GPFS_E_INVAL_ISCAN bad iscan parameter
 *              see system call read() ERRORS
 *
 * Notes:       The termOffset parameter provides a means to partition a
 *              file's data such that it may be read on more than one node.
 */
gpfs_off64_t GPFS_API
gpfs_ireadx(gpfs_ifile_t *ifile,      /* in only  */
            gpfs_iscan_t *iscan,      /* in only  */
            void *buffer,             /* in only  */
            int bufferSize,           /* in only  */
            gpfs_off64_t *offset,     /* in/out   */
            gpfs_off64_t termOffset,  /* in only */
            int *hole);               /* out only */


/* NAME:        gpfs_iwritex()
 *
 * FUNCTION:    Write file opened by gpfs_iopen.
 *              If parameter hole == 0, then write data
 *              addressed by buffer to the given offset for the
 *              given length. If hole != 0, then write
 *              a hole at the given offset for the given length.
 *
 * Input:       ifile :   ptr to gpfs_ifile_t returned from gpfs_iopen()
 *              buffer:   ptr to data buffer
 *              writeLen: length of data to write
 *              offset:   offset in file to write data
 *              hole:     flag =1 to write a "hole"
 *                             =0 to write data
 *
 * Returns:     number of bytes/size of hole written (Success)
 *              -1 and errno is set (Failure)
 *
 * Errno:       ENOSYS function not available
 *              EINVAL missing or bad parameter
 *              EISDIR file is a directory
 *              EPERM caller must have superuser privilege
 *              ESTALE cached fs information was invalid
 *              GPFS_E_INVAL_IFILE bad ifile parameter
 *              see system call write() ERRORS
 */
gpfs_off64_t GPFS_API
gpfs_iwritex(gpfs_ifile_t *ifile,    /* in only */
             void *buffer,           /* in only */
             gpfs_off64_t writeLen,  /* in only */
             gpfs_off64_t offset,    /* in only */
             int hole);              /* in only */


/* NAME:        gpfs_statfspool()
 *
 * FUNCTION:    Obtain status information about the storage pools
 *
 * Input:       pathname   : path to any file in the file system
 *              poolId     : id of first pool to return
 *                           on return set to next poolId or -1
 *                           to indicate there are no more pools.
 *              options    : option flags (currently not used)
 *              nPools     : number of stat structs requested or 0
 *                           on return number of stat structs in buffer
 *                           or if nPools was 0 its value is the max number
 *                           of storage pools currently defined
 *              buffer     :  ptr to return stat structures
 *              bufferSize : sizeof stat buffer
 *
 *              The user is expected to issue two or more calls. On the first
 *              call the user should pass nPools set to 0 and gpfs will
 *              return in nPools the total number of storage pools currently
 *              defined for the file system indicated by the pathname
 *              and it returns in poolId the id of the first storage pool.
 *              The buffer parameter may be set to NULL for this call.
 *
 *              The user may then allocate a buffer large enough to contain
 *              a gpfs_statfspool_t structure for each of the pools and issue
 *              a second call to obtain stat information about each pool.
 *              Parameter nPools should be set the number of pools requested.
 *              On return, nPools will be set to the number of stat structs
 *              contained in the buffer, and poolId will be set to the id
 *              of the next storage pool or -1 to indicate there are no
 *              additional storage pools defined.
 *
 *              Alternatively, if the user has a valid poolId from a previous
 *              call, the user may provide that poolId and a buffer large
 *              enough for a single gpfs_statfspool_t structure, and the call
 *              will return the status for a single storage pool.
 *
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       Specific error indication
 *              EINVAL
 */
int GPFS_API
gpfs_statfspool(const char *pathname, /* in only: path to file system*/
                gpfs_pool_t *poolId,  /* in out: id of first pool to return
                                         on return set to next poolId
                                         or -1 when there are no more pools */
                unsigned int options, /* in only: option flags */
                int *nPools,          /* in out: number of pool stats requested
                                         on return number of stat structs
                                         returned in buffer or if nPools was
                                         set to 0, the return value is the
                                         number of pools currently defined */
                void *buffer,         /* ptr to return stat structures */
                int bufferSize);      /* sizeof stat buffer or 0 */


/* NAME:        gpfs_getpoolname()
 *
 * FUNCTION:    Retrieves the name of the storage pool assigned for
 *              this file's data. The storage pool name is a null-terminated
 *              string, with a maximum length of GPFS_MAXNAMLEN.
 *
 * Input:       pathname:   path to any file in the file system
 *              poolId:     f_poolid returned in gpfs_statfspool_t
 *              buffer:     pointer to buffer for returned name
 *              bufferSize: size of buffer
 *
 * Returns:     0       Successful
 *              -1      Failure and errno is set
 *
 * Errno:       ENOSYS function not available
 *              ESTALE file system was unmounted
 *              E_FORMAT_INCOMPAT file system does not support pools
 *              E2BIG  buffer too small to return storage pool name
 */
int GPFS_API
gpfs_getpoolname(const char *pathname,
                 gpfs_pool_t poolId,
                 void *buffer,
                 int bufferSize);


/* /usr/src/linux/include/linux/fs.h includes /usr/src/linux/include/linux/quota.h
   which has conflicting definitions. */
#ifdef _LINUX_QUOTA_
  #undef Q_SYNC
  #undef Q_GETQUOTA
  #undef Q_SETQUOTA
  #undef Q_QUOTAON
  #undef Q_QUOTAOFF
#endif


/* GPFS QUOTACTL */

/*
 * Command definitions for the 'gpfs_quotactl' system call.
 * The commands are broken into a main command defined below
 * and a subcommand that is used to convey the type of
 * quota that is being manipulated (see above).
 */

#define SUBCMDMASK      0x00ff
#define SUBCMDSHIFT     8
#define GPFS_QCMD(cmd, type) (((cmd) << SUBCMDSHIFT) | ((type) & SUBCMDMASK))

#define Q_QUOTAON       0x0100  /* enable quotas */
#define Q_QUOTAOFF      0x0200  /* disable quotas */
#define Q_GETQUOTA      0x0300  /* get limits and usage */
#ifndef _LINUX_SOURCE_COMPAT
  /* Standard AIX definitions of quota commands */
  #define Q_SETQUOTA    0x0400  /* set limits */
  #define Q_SETQLIM     Q_SETQUOTA
#else
  /* Alternate definitions, for Linux Affinity */
  #define Q_SETQLIM     0x0400  /* set limits */
  #define Q_SETQUOTA    0x0700  /* set limits and usage */
#endif
#define Q_SETUSE        0x0500  /* set usage */
#define Q_SYNC          0x0600  /* sync disk copy of a filesystems quotas */

/* gpfs quota types */
#define GPFS_USRQUOTA     0
#define GPFS_GRPQUOTA     1
#define GPFS_FILESETQUOTA 2

/* define GPFS generated errno */
#define GPFS_E_NO_QUOTA_INST  237 /* file system does not support quotas */

typedef struct gpfs_quotaInfo
{
  gpfs_off64_t blockUsage;      /* current block count in 1 KB units*/
  gpfs_off64_t blockHardLimit;  /* absolute limit on disk blks alloc */
  gpfs_off64_t blockSoftLimit;  /* preferred limit on disk blks */
  gpfs_off64_t blockInDoubt;    /* distributed shares + "lost" usage for blks */
  int          inodeUsage;      /* current # allocated inodes */
  int          inodeHardLimit;  /* absolute limit on allocated inodes */
  int          inodeSoftLimit;  /* preferred inode limit */
  int          inodeInDoubt;    /* distributed shares + "lost" usage for inodes */
  gpfs_uid_t   quoId;           /* uid, gid or fileset id */
  int          entryType;       /* entry type, not used */
  unsigned int blockGraceTime;  /* time limit for excessive disk use */
  unsigned int inodeGraceTime;  /* time limit for excessive inode use */
} gpfs_quotaInfo_t;


/* NAME:        gpfs_quotactl()
 *
 * FUNCTION:    Manipulate disk quotas
 * INPUT:       pathname: specifies the pathname of any file within the
 *                        mounted file system to which the command is to
 *                        be applied
 *              cmd: specifies a quota control command to be applied
 *                   to UID/GID/FILESETID id. The cmd parameter can be
 *                   constructed using GPFS_QCMD(cmd, type) macro defined
 *                   in gpfs.h
 *              id:  UID or GID or FILESETID that command applied to.
 *              bufferP: points to the address of an optional, command
 *                       specific, data structure that is copied in or out of
 *                       the system.
 *
 * OUTPUT:      bufferP, if applicable.
 *
 * Returns:     0 success
 *              -1 failure
 *
 * Errno:       EACCESS
 *              EFAULT        An invalid bufferP parameter is supplied;
 *                            the associated structure could not be copied
 *                            in or out of the kernel
 *              EINVAL
 *              ENOENT        No such file or directory
 *              EPERM         The quota control command is privileged and
 *                            the caller did not have root user authority
 *              EOPNOTSUPP
 *              GPFS_E_NO_QUOTA_INST The file system does not support quotas
 */
int GPFS_API
gpfs_quotactl(const char *pathname,
              int cmd,
              int id,
              void *bufferP);


/* NAME:        gpfs_getfilesetid()
 *
 * FUNCTION:    Translate FilesetName to FilesetID
 *
 * INPUT:       pathname: specifies the pathname of any file within the
 *                        mounted file system to which the command is to
 *                        be applied
 *              name: name of the fileset
 *
 * OUTPUT:      idP:  points to the address of an integer that receives the ID
 *
 * Returns:     0 success
 *              -1 failure
 *
 * Errno:       EACCESS
 *              EFAULT        An invalid pointer is supplied; the associated
 *                            data could not be copied in or out of the kernel
 *              EINVAL
 *              ENOENT        No such file, directory or fileset
 */
int GPFS_API
gpfs_getfilesetid(const char *pathname,
                  const char *name,
                  int *idP);


/* NAME:        gpfs_clone_snap()
 *
 * FUNCTION:    Create an immutable clone parent from a source file
 *
 * Input:       sourcePathP:  path to source file, which will be cloned
 *              destPathP:    path to destination file, to be created
 *
 *              If destPathP is NULL, then the source file will be changed
 *              in place into an immutable clone parent.
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  Function not available
 *              ENOENT  File does not exist
 *              EACCESS Write access to target or source search permission denied
 *              EINVAL  Not a regular file or not a GPFS filesystem
 *              EFAULT  Input argument points outside accessible address space
 *              ENAMETOOLONG  Source or destination path name too long
 *              ENOSPC  Not enough space on disk
 *              EISDIR  Destination is a directory
 *              EXDEV   Source and destination aren't in the same filesystem
 */
int GPFS_API
gpfs_clone_snap(const char *sourcePathP, const char *destPathP);

/* NAME:        gpfs_clone_copy()
 *
 * FUNCTION:    Create a clone copy of an immutable clone parent file
 *
 * Input:       sourcePathP:  path to immutable source file, to be cloned
 *              destPathP:    path to destination file, to be created
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  Function not available
 *              ENOENT  File does not exist
 *              EACCESS Write access to target or source search permission denied
 *              EINVAL  Not a regular file or not a GPFS filesystem
 *              EFAULT  Input argument points outside accessible address space
 *              ENAMETOOLONG  Source or destination path name too long
 *              ENOSPC  Not enough space on disk
 *              EISDIR  Destination is a directory
 *              EXDEV   Source and destination aren't in the same filesystem
 */
int GPFS_API
gpfs_clone_copy(const char *sourcePathP, const char *destPathP);


/* NAME:        gpfs_declone()
 *
 * FUNCTION:    Copy blocks from clone parent(s) to child so that the
 *              parent blocks are no longer referenced by the child.
 *
 * Input:       fileDesc:  File descriptor for file to be de-cloned
 *              ancLimit:  Ancestor limit (immediate parent only, or all)
 *              nBlocks:   Maximum number of GPFS blocks to copy
 * In/Out:      offsetP:   Pointer to starting offset within file (will be
 *                         updated to offset of next block to process or
 *                         -1 if no more blocks)
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  Function not available
 *              EINVAL  Invalid argument to function
 *              EBADF   Bad file descriptor or not a GPFS file
 *              EPERM   Not a regular file
 *              EACCESS Write access to target file not permitted
 *              EFAULT  Input argument points outside accessible address space
 *              ENOSPC  Not enough space on disk
 */

/* Values for ancLimit */
#define GPFS_CLONE_ALL         0
#define GPFS_CLONE_PARENT_ONLY 1

int GPFS_API
gpfs_declone(gpfs_file_t fileDesc, int ancLimit, gpfs_off64_t nBlocks,
             gpfs_off64_t *offsetP);

/* NAME:        gpfs_clone_split()
 *
 * FUNCTION:    Split a clone child file from its parent.  Must call
 *              gpfs_declone first, to remove all references.
 *
 * Input:       fileDesc:  File descriptor for file to be split
 *              ancLimit:  Ancestor limit (immediate parent only, or all)
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  Function not available
 *              EINVAL  Invalid argument to function
 *              EBADF   Bad file descriptor or not a GPFS file
 *              EPERM   Not a regular file or not a clone child
 *              EACCESS Write access to target file not permitted
 */
int GPFS_API
gpfs_clone_split(gpfs_file_t fileDesc, int ancLimit);

/* NAME:        gpfs_clone_unsnap()
 *
 * FUNCTION:    Change a clone parent with no children back into a
 *              normal file.
 *
 * Input:       fileDesc:  File descriptor for file to be un-snapped
 *
 * Returns:     0       Successful
 *              -1      Failure
 *
 * Errno:       ENOSYS  Function not available
 *              EINVAL  Invalid argument to function
 *              EBADF   Bad file descriptor or not a GPFS file
 *              EPERM   Not a regular file or not a clone parent
 *              EACCESS Write access to target file not permitted
 */
int GPFS_API
gpfs_clone_unsnap(gpfs_file_t fileDesc);

/* NAME:       gpfs_get_fset_masks()
 *
 * FUNCTION:   return bit masks governing "external" inode and inode-space numbering
 *
 * Input:      fset_snaphandle: ptr to an fset snaphandle
 * Output:     the bit masks and inodes per block factor.
 *
 * Returns:    0       Success
 *            -1       Failure
 *
 * Errno:       ENOSYS function not available
 *              GPFS_E_INVAL_FSSNAPHANDLE invalid fssnapHandle
 */
int GPFS_API
gpfs_get_fset_masks(gpfs_fssnap_handle_t* fset_snapHandle,
                    gpfs_ino64_t* inodeSpaceMask,
                    gpfs_ino64_t* inodeBlockMask,
                    int* inodesPerInodeBlock);


/*
   API functions for Light Weight Event
*/
#include <gpfs_lweTypes.h>

/* NAME:       gpfs_lweCreateSession()
 *
 * FUNCTION:   create a light weight event session
 *
 * Input:      oldsid: existing session id, 
                       INVALID_SID for create a new session,
                       valid sid for resume an existing session
	       sessinfop: session string, unique for each session
 *	       
 * Output:     newsidp: session id for new session
 *
 * Returns:    0       Success
 *            -1       Failure
 *
 * Errno:       EINVAL oldsid and sessinfop mismatch
 *              EEXIST session with the same name already exists, can not create.
 */
int GPFS_API 
gpfs_lweCreateSession(lwe_sessid_t   oldsid,        /* IN */
                      char          *sessinfop,     /* IN */
                      lwe_sessid_t  *newsidp        /* OUT */
                      );


/* NAME:       gpfs_lweDestroySession()
 *
 * FUNCTION:   destroy a light weight event session
 *
 * Input:      sid: id of the session to be destroyed
 *
 * Returns:    0       Success
 *             EINVAL sid invalid
 *             EBUSY  session is busy 
 */
int GPFS_API 
gpfs_lweDestroySession(lwe_sessid_t sid);         /* IN */

/* NAME:       gpfs_lweGetAllSessions()
 *
 * FUNCTION:   fetch all lwe sessions
 *
 * Input:      nelem:   max number of elements
               sidbufp: array of session id
               nelemp:  number of session returned in sidbufp
 *
 * Returns:    0       Success
 *             EINVAL pass in args invalid
 *             E2BIG  information is too large
 */
int GPFS_API 
gpfs_lweGetAllSessions(unsigned int   nelem,      /* IN */
                       lwe_sessid_t  *sidbufp,    /* OUT */
		       unsigned int  *nelemp);    /* OUT */

/* NAME:       gpfs_lweQuerySession()
 *
 * FUNCTION:   query session string by id 
 *
 * Input:      sid:    id of session to be queryed
               buflen: length of buffer
               bufp:   buffer to store sessions string
               rlenp:  returned length of bufp
 *
 * Returns:    0       Success
 *             EINVAL  pass in args invalid
 *             E2BIG   information is too large
 */
int GPFS_API 
gpfs_lweQuerySession(lwe_sessid_t   sid,     /* IN */
                     size_t         buflen,  /* IN */
		     void          *bufp,    /* OUT */
		     size_t        *rlenP);  /* OUT */

/* NAME:       gpfs_lweGetEvents()
 *
 * FUNCTION:   get events from a light weight session
 *
 * Input:      sid:     id of the session
 *             maxmsgs: max number of event to fetch, 
 *                      0 to fetch all possible
 *             flags:   LWE_EV_WAIT: waiting for new events if event
 *                      queue is empty
 *             buflen:  length of the buffer
 *             bufp:    buffer to hold events
 *             rlenp:   returned length of bufp
 *
 * Returns:    0        Success
 *             E2BIG    information is too large
 *             EINVAL   pass in args invalid
 */
int GPFS_API
gpfs_lweGetEvents(lwe_sessid_t  sid,     /* IN  */
                  unsigned int  maxmsgs, /* IN  */
                  unsigned int  flags,   /* IN  */
		  size_t        buflen,  /* IN  */
		  void         *bufp,    /* OUT */
		  size_t       *rlenp);  /* OUT */

/* NAME:      gpfs_lweRespondEvent()
 *
 * FUNCTION:  response to a light weight event
 *
 * Input:     sid:      id of the session
 *            token:    token of the event
 *            response: response to the event
 *            reterror: return error to event callers
 *
 * Returns:   0         Success
 *            EINVAL    pass in args invalid
 *
 */
int GPFS_API
gpfs_lweRespondEvent(lwe_sessid_t   sid,       /* IN */
                     lwe_token_t    token,     /* IN */
		     lwe_resp_t     response,  /* IN */
		     int            reterror); /* IN */


/* 
 * When GPFS_64BIT_INODES is defined, use the 64-bit interface definitions as
 * the default.
 */

#ifdef GPFS_64BIT_INODES
  #undef  GPFS_D_VERSION
  #define GPFS_D_VERSION GPFS_D64_VERSION
  #undef  GPFS_IA_VERSION
  #define GPFS_IA_VERSION GPFS_IA64_VERSION

  #define gpfs_ino_t gpfs_ino64_t
  #define gpfs_gen_t gpfs_gen64_t
  #define gpfs_uid_t gpfs_uid64_t
  #define gpfs_gid_t gpfs_gid64_t
  #define gpfs_snapid_t gpfs_snapid64_t
  #define gpfs_nlink_t gpfs_nlink64_t
  #define gpfs_timestruc_t gpfs_timestruc64_t
  #define gpfs_direntx_t gpfs_direntx64_t
  #define gpfs_direntx gpfs_direntx64
  #define gpfs_iattr_t gpfs_iattr64_t

  #define gpfs_get_snapid_from_fssnaphandle gpfs_get_snapid_from_fssnaphandle64
  #define gpfs_open_inodescan gpfs_open_inodescan64
  #define gpfs_open_inodescan_with_xattrs gpfs_open_inodescan_with_xattrs64
  #define gpfs_next_inode gpfs_next_inode64
  #define gpfs_next_inode_with_xattrs gpfs_next_inode_with_xattrs64
  #define gpfs_seek_inode gpfs_seek_inode64
  #define gpfs_stat_inode gpfs_stat_inode64
  #define gpfs_stat_inode_with_xattrs gpfs_stat_inode_with_xattrs64
  #define gpfs_iopen gpfs_iopen64
  #define gpfs_ireaddir gpfs_ireaddir64
  #define gpfs_ireaddirx gpfs_ireaddirx64
  #define gpfs_iwritedir gpfs_iwritedir64
  #define gpfs_ireadlink gpfs_ireadlink64
#endif


#ifdef __cplusplus
}
#endif

#endif /* H_GPFS */
