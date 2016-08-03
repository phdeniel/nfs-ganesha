/*
 * vim:noexpandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright (C) Panasas Inc., 2011
 *
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * -------------
 */

/* xattrs.c
 * KVSFS object (file|dir) handle object extended attributes
 */

#include "config.h"

#include <assert.h>

#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <mntent.h>
#include <kvsns.h>
#include <ctype.h>
#include "gsh_list.h"
#include "fsal_internal.h"
#include "fsal_convert.h"
#include "FSAL/fsal_config.h"
#include "FSAL/fsal_commonlib.h"
#include <stdbool.h>

typedef int (*xattr_getfunc_t) (struct fsal_obj_handle *, /* object handle */
				caddr_t,	/* output buff */
				size_t,	/* output buff size */
				size_t *,	/* output size */
				void *arg);	/* optional argument */

typedef int (*xattr_setfunc_t) (struct fsal_obj_handle *, /* object handle */
				caddr_t,	/* input buff */
				size_t,	/* input size */
				int,	/* creation flag */
				void *arg);	/* optional argument */

typedef struct fsal_xattr_def__ {
	char xattr_name[MAXNAMLEN + 1];
	xattr_getfunc_t get_func;
	xattr_setfunc_t set_func;
	int flags;
	void *arg;
} fsal_xattr_def_t;

/*
 * DEFINE GET/SET FUNCTIONS
 */

int print_vfshandle(struct fsal_obj_handle *obj_hdl, caddr_t buffer_addr,
		    size_t buffer_size, size_t *p_output_size, void *arg)
{
	*p_output_size = snprintf(buffer_addr, buffer_size,
				  "(not yet implemented)");

	return 0;
}				/* print_fid */

/* DEFINE HERE YOUR ATTRIBUTES LIST */

#if 0
static fsal_xattr_def_t xattr_list[] = {
	{"vfshandle", print_vfshandle, NULL, XATTR_FOR_ALL | XATTR_RO, NULL},
};

#define XATTR_COUNT 1

/* we assume that this number is < 254 */
#if (XATTR_COUNT > 254)
#error "ERROR: xattr count > 254"
#endif
/* test if an object has a given attribute */
static int do_match_type(int xattr_flag, object_file_type_t obj_type)
{
	switch (obj_type) {
	case REGULAR_FILE:
		return ((xattr_flag & XATTR_FOR_FILE) == XATTR_FOR_FILE);

	case DIRECTORY:
		return ((xattr_flag & XATTR_FOR_DIR) == XATTR_FOR_DIR);

	case SYMBOLIC_LINK:
		return ((xattr_flag & XATTR_FOR_SYMLINK) == XATTR_FOR_SYMLINK);

	default:
		return ((xattr_flag & XATTR_FOR_ALL) == XATTR_FOR_ALL);
	}
}

static int attr_is_read_only(unsigned int attr_index)
{
	if (attr_index < XATTR_COUNT) {
		if (xattr_list[attr_index].flags & XATTR_RO)
			return true;
	}
	/* else : standard xattr */
	return false;
}

static void chomp_attr_value(char *str, size_t size)
{
	int len;

	if (str == NULL)
		return;

	/* security: set last char to '\0' */
	str[size - 1] = '\0';

	len = strnlen(str, size);
	if ((len > 0) && (str[len - 1] == '\n'))
		str[len - 1] = '\0';
}

static int file_attributes_to_xattr_attrs(struct attrlist *file_attrs,
					  struct attrlist *xattr_attrs,
					  unsigned int attr_index)
{
	/* supported attributes are:
	 * - owner (same as the objet)
	 * - group (same as the objet)
	 * - type FSAL_TYPE_XATTR
	 * - fileid (attr index ? or (fileid^((index+1)<<24)) )
	 * - mode (config & file)
	 * - atime, mtime, ctime = these of the object ?
	 * - size=1block, used=1block
	 * - rdev=0
	 * - nlink=1
	 */
	attrmask_t supported =
		(ATTR_MODE | ATTR_FILEID | ATTR_TYPE |
		 ATTR_OWNER | ATTR_GROUP |
		 ATTR_ATIME | ATTR_MTIME | ATTR_CTIME |
		 ATTR_CREATION | ATTR_CHGTIME
		 | ATTR_SIZE | ATTR_SPACEUSED |
		 ATTR_NUMLINKS | ATTR_RAWDEV |
		 ATTR_FSID);
	attrmask_t unsupp;

	if (xattr_attrs->mask == 0) {
		xattr_attrs->mask = supported;

		LogCrit(COMPONENT_FSAL,
			"Error: xattr_attrs->mask was 0");
	}

	unsupp = xattr_attrs->mask & (~supported);

	if (unsupp) {
		LogDebug(COMPONENT_FSAL,
			 "Asking for unsupported attributes: 0x%"PRIx64
			 " removing it from asked attributes",
			 unsupp);

		xattr_attrs->mask &= (~unsupp);
	}

	if (xattr_attrs->mask & ATTR_MODE) {
		xattr_attrs->mode = file_attrs->mode;

		if (attr_is_read_only(attr_index))
			xattr_attrs->mode &= ~(0222);
	}

	if (xattr_attrs->mask & ATTR_FILEID) {
		unsigned int i;
		unsigned long hash = attr_index + 1;
		char *str = (char *)&file_attrs->fileid;

		for (i = 0; i < sizeof(xattr_attrs->fileid); i++, str++)
			hash = (hash << 5) - hash + (unsigned long)(*str);
		xattr_attrs->fileid = hash;
	}

	if (xattr_attrs->mask & ATTR_TYPE)
		xattr_attrs->type = EXTENDED_ATTR;

	if (xattr_attrs->mask & ATTR_OWNER)
		xattr_attrs->owner = file_attrs->owner;

	if (xattr_attrs->mask & ATTR_GROUP)
		xattr_attrs->group = file_attrs->group;

	if (xattr_attrs->mask & ATTR_ATIME)
		xattr_attrs->atime = file_attrs->atime;

	if (xattr_attrs->mask & ATTR_MTIME)
		xattr_attrs->mtime = file_attrs->mtime;

	if (xattr_attrs->mask & ATTR_CTIME)
		xattr_attrs->ctime = file_attrs->ctime;

	if (xattr_attrs->mask & ATTR_CREATION)
		xattr_attrs->creation = file_attrs->creation;

	if (xattr_attrs->mask & ATTR_CHGTIME) {
		xattr_attrs->chgtime = file_attrs->chgtime;
		xattr_attrs->change = xattr_attrs->chgtime.tv_sec;
	}

	if (xattr_attrs->mask & ATTR_SIZE)
		xattr_attrs->filesize = DEV_BSIZE;

	if (xattr_attrs->mask & ATTR_SPACEUSED)
		xattr_attrs->spaceused = DEV_BSIZE;

	if (xattr_attrs->mask & ATTR_NUMLINKS)
		xattr_attrs->numlinks = 1;

	if (xattr_attrs->mask & ATTR_RAWDEV) {
		xattr_attrs->rawdev.major = 0;
		xattr_attrs->rawdev.minor = 0;
	}

	if (xattr_attrs->mask & ATTR_FSID)
		xattr_attrs->fsid = file_attrs->fsid;

	/* if mode==0, then owner is set to root and mode is set to 0600 */
	if ((xattr_attrs->mask & ATTR_OWNER)
	    && (xattr_attrs->mask & ATTR_MODE) && (xattr_attrs->mode == 0)) {
		xattr_attrs->owner = 0;
		xattr_attrs->mode = 0600;
		if (attr_is_read_only(attr_index))
			xattr_attrs->mode &= ~(0200);
	}

	return 0;

}

static int xattr_format_value(caddr_t buffer, size_t *datalen, size_t maxlen)
{
	size_t size_in = *datalen;
	size_t len = strnlen((char *)buffer, size_in);
	int i;

	if (len == size_in - 1 || len == size_in) {
		int ascii = true;
		char *str = buffer;
		int i;

		for (i = 0; i < len; i++) {
			if (!isprint(str[i]) && !isspace(str[i])) {
				ascii = false;
				break;
			}
		}

		if (ascii) {
			*datalen = size_in;
			/* add additional '\n', if missing */
			if ((size_in + 1 < maxlen) && (str[len - 1] != '\n')) {
				str[len] = '\n';
				str[len + 1] = '\0';
				(*datalen) += 2;
			}
			return ERR_FSAL_NO_ERROR;
		}
	}

	/* byte, word, 32 or 64 bits */
	if (size_in == 1) {
		unsigned char val = *((unsigned char *)buffer);
		*datalen = 1 + snprintf((char *)buffer, maxlen, "%hhu\n", val);
		return ERR_FSAL_NO_ERROR;
	} else if (size_in == 2) {
		unsigned short val = *((unsigned short *)buffer);
		*datalen = 1 + snprintf((char *)buffer, maxlen, "%hu\n", val);
		return ERR_FSAL_NO_ERROR;
	} else if (size_in == 4) {
		unsigned int val = *((unsigned int *)buffer);
		*datalen = 1 + snprintf((char *)buffer, maxlen, "%u\n", val);
		return ERR_FSAL_NO_ERROR;
	} else if (size_in == 8) {
		unsigned long long val = *((unsigned long long *)buffer);
		*datalen = 1 + snprintf((char *)buffer, maxlen, "%llu\n", val);
		return ERR_FSAL_NO_ERROR;
	} else {
		/* 2 bytes per initial byte +'0x' +\n +\0 */
		char *curr_out;
		char *tmp_buf = (char *) gsh_malloc(3 * size_in + 4);

		curr_out = tmp_buf;
		curr_out += sprintf(curr_out, "0x");
		/* hexa representation */
		for (i = 0; i < size_in; i++) {
			unsigned char *p8 = (unsigned char *)(buffer + i);

			if ((i % 4 == 3) && (i != size_in - 1))
				curr_out += sprintf(curr_out, "%02hhX.", *p8);
			else
				curr_out += sprintf(curr_out, "%02hhX", *p8);
		}
		*curr_out = '\n';
		curr_out++;
		*curr_out = '\0';
		curr_out++;
		strncpy((char *)buffer, tmp_buf, maxlen);
		*datalen = strlen(tmp_buf) + 1;
		if (*datalen > maxlen)
			*datalen = maxlen;
		gsh_free(tmp_buf);
		return ERR_FSAL_NO_ERROR;
	}
}
#endif

fsal_status_t kvsfs_list_ext_attrs(struct fsal_obj_handle *obj_hdl,
				  unsigned int argcookie,
				  fsal_xattrent_t *xattrs_tab,
				  unsigned int xattrs_tabsize,
				  unsigned int *p_nb_returned, int *end_of_list)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

fsal_status_t kvsfs_getextattr_id_by_name(struct fsal_obj_handle *obj_hdl,
					 const char *xattr_name,
					 unsigned int *pxattr_id)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

fsal_status_t kvsfs_getextattr_value_by_id(struct fsal_obj_handle *obj_hdl,
					  unsigned int xattr_id,
					  caddr_t buffer_addr,
					  size_t buffer_size,
					  size_t *p_output_size)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

fsal_status_t kvsfs_getextattr_value_by_name(struct fsal_obj_handle *obj_hdl,
					    const char *xattr_name,
					    caddr_t buffer_addr,
					    size_t buffer_size,
					    size_t *p_output_size)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

fsal_status_t kvsfs_setextattr_value(struct fsal_obj_handle *obj_hdl,
				    const char *xattr_name, caddr_t buffer_addr,
				    size_t buffer_size, int create)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

fsal_status_t kvsfs_setextattr_value_by_id(struct fsal_obj_handle *obj_hdl,
					  unsigned int xattr_id,
					  caddr_t buffer_addr,
					  size_t buffer_size)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

fsal_status_t kvsfs_getextattr_attrs(struct fsal_obj_handle *obj_hdl,
				    unsigned int xattr_id,
				    struct attrlist *p_attrs)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

fsal_status_t kvsfs_remove_extattr_by_id(struct fsal_obj_handle *obj_hdl,
					unsigned int xattr_id)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

fsal_status_t kvsfs_remove_extattr_by_name(struct fsal_obj_handle *obj_hdl,
					  const char *xattr_name)
{
	return fsalstat(ERR_FSAL_NO_ERROR, 0);
}
