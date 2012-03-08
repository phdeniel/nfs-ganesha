/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* avs_rhrz src/avs/fs/mmfs/ts/util/gpfs_nfs.h 1.23                       */
/*                                                                        */
/* Licensed Materials - Property of IBM                                   */
/*                                                                        */
/* Restricted Materials of IBM                                            */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2011,2012              */
/* All Rights Reserved                                                    */
/*                                                                        */
/* US Government Users Restricted Rights - Use, duplication or            */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.      */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
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
/* @(#)83       1.23  src/avs/fs/mmfs/ts/util/gpfs_nfs.h, mmfs, avs_rhrz 1/19/12 12:19:24 */
/*
 *  Library calls for GPFS interfaces
 */
#ifndef H_GPFS_NFS
#define H_GPFS_NFS

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
struct flock
{};
#endif

//#define GPFS_PRINTK

/* GANESHA common information */

#define OPENHANDLE_NAME_TO_HANDLE 101
#define OPENHANDLE_OPEN_BY_HANDLE 102
#define OPENHANDLE_LINK_BY_FD     103
#define OPENHANDLE_READLINK_BY_FD 104
#define OPENHANDLE_STAT_BY_HANDLE 105
#define OPENHANDLE_LAYOUT_TYPE    106
#define OPENHANDLE_GET_DEVICEINFO 107
#define OPENHANDLE_GET_DEVICELIST 108
#define OPENHANDLE_LAYOUT_GET     109
#define OPENHANDLE_LAYOUT_RETURN  110
#define OPENHANDLE_INODE_UPDATE   111
#define OPENHANDLE_GET_XSTAT      112
#define OPENHANDLE_SET_XSTAT      113
#define OPENHANDLE_CHECK_ACCESS   114
#define OPENHANDLE_OPEN_SHARE_BY_HANDLE 115
#define OPENHANDLE_GET_LOCK       116
#define OPENHANDLE_SET_LOCK       117
#define OPENHANDLE_THREAD_UPDATE  118
#define OPENHANDLE_LAYOUT_COMMIT  119
#define OPENHANDLE_DS_READ        120
#define OPENHANDLE_DS_WRITE       121
#define OPENHANDLE_GET_VERIFIER   122
#define OPENHANDLE_FSYNC          123

int gpfs_ganesha(int op, void *oarg);

#define OPENHANDLE_HANDLE_LEN 40
#define OPENHANDLE_KEY_LEN 28

struct xstat_cred_t
{
  u_int32_t principal;          /* user id */
  u_int32_t group;              /* primary group id */
  u_int16_t num_groups;         /* number secondary groups for this user */
#define XSTAT_CRED_NGROUPS 32
  u_int32_t eGroups[XSTAT_CRED_NGROUPS];/* array of secondary groups */
};

struct gpfs_time_t
{
  u_int32_t t_sec;
  u_int32_t t_nsec;
};

struct gpfs_file_handle
{
  u_int32_t handle_size;
  u_int32_t handle_type;
  u_int16_t handle_version;
  u_int16_t handle_key_size;
  /* file identifier */
  unsigned char f_handle[OPENHANDLE_HANDLE_LEN];
};

struct name_handle_arg
{
  int dfd;
  int flag;
  char *name;
  struct gpfs_file_handle *handle;
};

struct open_arg
{
  int mountdirfd;
  int flags;
  int openfd;
  struct gpfs_file_handle *handle;
};

struct glock
{
  int cmd;
  int lfd;
  void *lock_owner;
  struct flock flock;
};
#define GPFS_F_CANCELLK (1024 + 5)   /* Maps to Linux F_CANCELLK */

struct set_get_lock_arg
{
  int mountdirfd;
  struct glock *lock;
};

struct open_share_arg
{
  int mountdirfd;
  int flags;
  int openfd;
  struct gpfs_file_handle *handle;
  int share_access;
  int share_deny;
};

struct link_arg
{
  int file_fd;
  int dir_fd;
  char *name;
};

struct readlink_arg
{
  int fd;
  char *buffer;
  int size;
};

struct nfsd4_pnfs_deviceid {
	unsigned long	sbid;		/* per-superblock unique ID */
	unsigned long	devid;		/* filesystem-wide unique device ID */
};

struct gpfs_exp_xdr_stream {
  int *p;
  int *end;
};

enum x_nfsd_fsid {
	x_FSID_DEV = 0,
	x_FSID_NUM,
	x_FSID_MAJOR_MINOR,
	x_FSID_ENCODE_DEV,
	x_FSID_UUID4_INUM,
	x_FSID_UUID8,
	x_FSID_UUID16,
	x_FSID_UUID16_INUM,
	x_FSID_MAX
};

//#if !defined(NFS4_DEVICEID4_SIZE)
#ifdef P_NFS4
enum pnfs_layouttype {
	LAYOUT_NFSV4_1_FILES  = 1,
	LAYOUT_OSD2_OBJECTS = 2,
	LAYOUT_BLOCK_VOLUME = 3,

	NFS4_PNFS_PRIVATE_LAYOUT = 0x80000000
};

/* used for both layout return and recall */
enum pnfs_layoutreturn_type {
	RETURN_FILE = 1,
	RETURN_FSID = 2,
	RETURN_ALL  = 3
};

enum pnfs_iomode {
	IOMODE_READ = 1,
	IOMODE_RW = 2,
	IOMODE_ANY = 3,
};
#endif

enum stable_nfs
{
  x_UNSTABLE4 = 0,
  x_DATA_SYNC4 = 1,
  x_FILE_SYNC4 = 2
};

struct pnfstime4 {
	u_int64_t	seconds;
	u_int32_t	nseconds;
};

struct nfsd4_pnfs_dev_iter_res {
	u_int64_t		gd_cookie;	/* request/repsonse */
	u_int64_t		gd_verf;	/* request/repsonse */
	u_int64_t		gd_devid;	/* response */
	u_int32_t		gd_eof;		/* response */
};

/* Arguments for set_device_notify */
struct pnfs_devnotify_arg {
	struct nfsd4_pnfs_deviceid dn_devid;	/* request */
	u_int32_t dn_layout_type;		/* request */
	u_int32_t dn_notify_types;		/* request/response */
};

struct nfsd4_layout_seg {
	u_int64_t	clientid;
	u_int32_t	layout_type;
	u_int32_t	iomode;
	u_int64_t	offset;
	u_int64_t	length;
};

struct nfsd4_pnfs_layoutget_arg {
	u_int64_t		lg_minlength;
	u_int64_t		lg_sbid;
	struct gpfs_file_handle	*lg_fh;
};

struct nfsd4_pnfs_layoutget_res {
	struct nfsd4_layout_seg	lg_seg;	/* request/resopnse */
	u_int32_t		lg_return_on_close;
};

struct nfsd4_pnfs_layoutcommit_arg {
	struct nfsd4_layout_seg	lc_seg;		/* request */
	u_int32_t		lc_reclaim;	/* request */
	u_int32_t		lc_newoffset;	/* request */
	u_int64_t		lc_last_wr;	/* request */
	struct pnfstime4		lc_mtime;	/* request */
	u_int32_t		lc_up_len;	/* layout length */
	void			*lc_up_layout;	/* decoded by callback */
};

struct nfsd4_pnfs_layoutcommit_res {
	u_int32_t		lc_size_chg;	/* boolean for response */
	u_int64_t		lc_newsize;	/* response */
};

struct nfsd4_pnfs_layoutreturn_arg {
	u_int32_t		lr_return_type;	/* request */
	struct nfsd4_layout_seg	lr_seg;		/* request */
	u_int32_t		lr_reclaim;	/* request */
	u_int32_t		lrf_body_len;	/* request */
	void			*lrf_body;	/* request */
	void			*lr_cookie;	/* fs private */
};

struct x_xdr_netobj {
	unsigned int	len;
	unsigned char	*data;
};
struct pnfs_filelayout_devaddr {
	struct x_xdr_netobj	r_netid;
	struct x_xdr_netobj	r_addr;
};

/* list of multipath servers */
struct pnfs_filelayout_multipath {
	u_int32_t			fl_multipath_length;
	struct pnfs_filelayout_devaddr 	*fl_multipath_list;
};

struct pnfs_filelayout_device {
	u_int32_t			fl_stripeindices_length;
	u_int32_t			*fl_stripeindices_list;
	u_int32_t			fl_device_length;
	struct pnfs_filelayout_multipath *fl_device_list;
};

struct pnfs_filelayout_layout {
	u_int32_t                        lg_layout_type; /* response */
	u_int32_t                        lg_stripe_type; /* response */
	u_int32_t                        lg_commit_through_mds; /* response */
	u_int64_t                        lg_stripe_unit; /* response */
	u_int64_t                        lg_pattern_offset; /* response */
	u_int32_t                        lg_first_stripe_index;	/* response */
	struct nfsd4_pnfs_deviceid	device_id;		/* response */
	u_int32_t                        lg_fh_length;		/* response */
	struct gpfs_file_handle          *lg_fh_list;		/* response */
};

enum stripetype4 {
	STRIPE_SPARSE = 1,
	STRIPE_DENSE = 2
};

struct deviceinfo_arg
{
  int mountdirfd;
  int type;
  struct nfsd4_pnfs_deviceid devid;
  struct gpfs_exp_xdr_stream xdr;
};

struct layoutget_arg
{
  int mountdirfd;
  struct gpfs_file_handle *handle;
  struct nfsd4_pnfs_layoutget_arg args;
  struct pnfs_filelayout_layout *file_layout;
  struct gpfs_exp_xdr_stream *xdr;
};

struct layoutreturn_arg
{
  int mountdirfd;
  struct gpfs_file_handle *handle;
  struct nfsd4_pnfs_layoutreturn_arg args;
};

struct dsread_arg
{
  int mountdirfd;
  struct gpfs_file_handle *handle;
  char *bufP;
  u_int64_t offset;
  u_int64_t length;
};

struct dswrite_arg
{
  int mountdirfd;
  struct gpfs_file_handle *handle;
  char *bufP;
  u_int64_t offset;
  u_int64_t length;
  u_int32_t stability_wanted;
  u_int32_t *stability_got;
  u_int32_t *verifier4;
};

struct layoutcommit_arg
{
  int mountdirfd;
  struct gpfs_file_handle *handle;
  u_int64_t offset;
  u_int64_t length;
  u_int32_t reclaim;      /* True if this is a reclaim commit */
  u_int32_t new_offset;   /* True if the client has suggested a new offset */
  u_int64_t last_write;   /* The offset of the last byte written, if
                               new_offset if set, otherwise undefined. */
  u_int32_t time_changed; /* True if the client provided a new value for mtime */
  struct gpfs_time_t new_time;  /* If time_changed is true, the client-supplied
                               modification tiem for the file.  otherwise, undefined. */
  struct gpfs_exp_xdr_stream *xdr;
};

struct fsync_arg
{
  int mountdirfd;
  struct gpfs_file_handle *handle;
  u_int64_t offset;
  u_int64_t length;
  u_int32_t *verifier4;
};

struct stat_arg
{
    int mountdirfd;
    struct gpfs_file_handle *handle;
#if BITS_PER_LONG != 64
    struct stat64 *buf;
#else
    struct stat *buf;
#endif
};

struct callback_arg
{
    int mountdirfd;
    int *reason;
    struct gpfs_file_handle *handle;
    struct glock *fl;
#if BITS_PER_LONG != 64
    struct stat64 *buf;
#else
    struct stat *buf;
#endif
};
/* reason list */
#define INODE_INVALIDATE 1
#define INODE_UPDATE     2
#define INODE_LOCK_GRANTED 3
#define INODE_LOCK_AGAIN   4
#define THREAD_STOP        5
#define THREAD_PAUSE       6

/* define flags for attr_valid */
#define XATTR_STAT      (1 << 0)
#define XATTR_ACL       (1 << 1)

/* define flags for attr_chaged */
#define XATTR_MODE      (1 << 0) //  01
#define XATTR_UID       (1 << 1) //  02
#define XATTR_GID       (1 << 2) //  04
#define XATTR_SIZE      (1 << 3) //  08
#define XATTR_ATIME     (1 << 4) //  10
#define XATTR_MTIME     (1 << 5) //  20
#define XATTR_CTIME     (1 << 6) //  40
#define XATTR_ATIME_SET (1 << 7) //  80
#define XATTR_MTIME_SET (1 << 8) // 100

struct xstat_arg
{
    int attr_valid;
    int mountdirfd;
    struct gpfs_file_handle *handle;
    struct gpfs_acl *acl;
    int attr_changed;
#if BITS_PER_LONG != 64
    struct stat64 *buf;
#else
    struct stat *buf;
#endif
};

struct xstat_access_arg
{
    int mountdirfd;
    struct gpfs_file_handle *handle;
    struct gpfs_acl *acl;
    struct xstat_cred_t *cred;
    unsigned int posix_mode;
    unsigned int access;       /* v4maske */
    unsigned int *supported;	
};

#ifdef __cplusplus
}
#endif

#endif /* H_GPFS_NFS */
