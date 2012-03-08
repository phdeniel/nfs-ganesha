/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright CEA/DAM/DIF  (2008)
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
 *
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
 *
 * ---------------------------------------
 */

/**
 * \file    pnfs_layoutget.c
 * \author  $Author: deniel $
 * \date    $Date: 2005/11/28 17:02:50 $
 * \version $Revision: 1.8 $
 * \brief   Routines used for managing the NFS4 COMPOUND functions.
 *
 * pnfs_lock.c : Routines used for managing the NFS4 COMPOUND functions.
 *
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _SOLARIS
#include "solaris_port.h"
#endif

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/file.h>           /* for having FNDELAY */
#include <stdint.h>
#include "HashData.h"
#include "HashTable.h"
#include "rpc.h"
#include "log.h"
#include "stuff_alloc.h"
#include "nfs23.h"
#include "nfs4.h"
#include "pnfs.h"
#include "mount.h"
#include "nfs_core.h"
#include "cache_inode.h"
#include "cache_content.h"
#include "nfs_exports.h"
#include "nfs_creds.h"
#include "nfs_proto_functions.h"
#include "nfs_proto_tools.h"
#include "nfs_file_handle.h"
#include "nfs_tools.h"
#ifdef _USE_FSALMDS
#include "fsal.h"
#include "fsal_pnfs.h"
#include "sal_data.h"
#include "sal_functions.h"
#endif                                          /* _USE_FSALMDS */

#include "pnfs_internal.h"

#ifdef _USE_FSALMDS
static nfsstat4 acquire_layout_state(compound_data_t *data,
                                     stateid4 *supplied_stateid,
                                     layouttype4 layout_type,
                                     state_t **layout_state,
                                     const char *tag);

static nfsstat4 one_segment(fsal_handle_t *handle,
                            fsal_op_context_t *context,
                            state_t *layout_state,
                            const struct fsal_layoutget_arg *arg,
                            struct fsal_layoutget_res *res,
                            layout4 *current);
#endif /* _USE_FSALMDS */

/**
 *
 * \brief The NFS4_OP_LAYOUTGET operation.
 *
 * This function implements the NFS4_OP_LAYOUTGET operation.
 *
 * \param op    [IN]    pointer to nfs4_op arguments
 * \param data  [INOUT] Pointer to the compound request's data
 * \param resp  [IN]    Pointer to nfs4_op results
 *
 * \return NFS4_OK if successfull, other values show an error.
 *
 * \see all the pnfs_<*> function
 * \see nfs4_Compound
 *
 */

#define arg_LAYOUTGET4 op->nfs_argop4_u.oplayoutget
#define res_LAYOUTGET4 resp->nfs_resop4_u.oplayoutget

/* In the future, put soemthing in the FSAL's static info allowing it
   to specify how big this buffer should be. */

nfsstat4 FSAL_pnfs_layoutget( LAYOUTGET4args   * pargs,
                              compound_data_t  * data,
                              LAYOUTGET4res    * pres )
{
     char __attribute__ ((__unused__)) funcname[] = "pnfs_layoutget";
#ifdef _USE_FSALMDS
     /* Return code from state functions */
     state_status_t state_status = 0;
     /* NFSv4.1 status code */
     nfsstat4 nfs_status = 0;
     /* Status from Cache_inode */
     cache_inode_status_t cache_status = 0;
     /* Pointer to state governing layouts */
     state_t *layout_state = NULL;
     /* Pointer to the array of layouts */
     layout4 *layouts = NULL;
     /* Total number of layout segments returned by the FSAL */
     uint32_t numlayouts = 0;
     /* Pointer to the file handle */
     fsal_handle_t *handle = NULL;
     /* Tag supplied to SAL functions for debugging messages */
     const char *tag = "LAYOUTGET";
     /* Input arguments of layoutget */
     struct fsal_layoutget_arg arg;
     /* Input/output and output arguments of layoutget */
     struct fsal_layoutget_res res;
     /* Maximum number of segments this FSAL will ever return for a
        single LAYOUTGET */
     int max_segment_count = 0;
#endif /* _USE_FSALMDS */

#ifdef _USE_FSALMDS

     if ((nfs_status = nfs4_sanity_check_FH(data,
                                         REGULAR_FILE))
         != NFS4_OK) {
          goto out;
     }

     if (!nfs4_pnfs_supported(data->pexport)) {
          nfs_status = NFS4ERR_LAYOUTUNAVAILABLE;
          goto out;
     }

     if ((nfs_status = acquire_layout_state(data,
                                            &pargs->loga_stateid,
                                            pargs->loga_layout_type,
                                            &layout_state,
                                            tag))
         != NFS4_OK) {
          goto out;
     }

     /*
      * Blank out argument structures and get the filehandle.
      */

     memset(&arg, 0, sizeof(struct fsal_layoutget_arg));
     memset(&res, 0, sizeof(struct fsal_layoutget_res));

     handle = cache_inode_get_fsal_handle(data->current_entry,
                                          &cache_status);

     if (cache_status != CACHE_INODE_SUCCESS) {
          nfs_status = nfs4_Errno(cache_status);
          goto out;
     }

     /*
      * Initialize segment array and fill out input-only arguments
      */

     max_segment_count = (data->pcontext->export_context->
                          fe_static_fs_info->max_segment_count);

     if (max_segment_count == 0) {
          LogCrit(COMPONENT_PNFS,
                  "The FSAL must specify a non-zero max_segment_count "
                  "in its fsal_staticfsinfo_t");
          nfs_status = NFS4ERR_SERVERFAULT;
          goto out;
     }

     if ((layouts = (layout4*) Mem_Alloc(sizeof(layout4) *
                                         max_segment_count))
         == NULL) {
          nfs_status = NFS4ERR_SERVERFAULT;
          goto out;
     }

     memset(layouts, 0, sizeof(layout4) * max_segment_count);

     arg.type = pargs->loga_layout_type;
     arg.minlength = pargs->loga_minlength;
     arg.export_id = data->pexport->id;
     arg.maxcount = pargs->loga_maxcount;

     /* Guaranteed on the first call */
     res.context = NULL;

     /* XXX Currently we have no callbacks, so it makes no sense to
        pass the client-supplied value to the FSAL.  When we get
        callbacks, it will. */
     res.signal_available = FALSE;

     do {
          /* Since the FSAL writes to tis structure with every call, we
             re-initialize it with the operation's arguments */
          res.segment.io_mode = pargs->loga_iomode;
          res.segment.offset = pargs->loga_offset;
          res.segment.length = pargs->loga_length;

          /* Clear anything from a previous segment */
          res.fsal_seg_data = NULL;

          if ((nfs_status = one_segment(handle,
                                        data->pcontext,
                                        layout_state,
                                        &arg,
                                        &res,
                                        layouts + numlayouts))
              != NFS4_OK) {
               goto out;
          }

          numlayouts++;

          if ((numlayouts == max_segment_count) && !res.last_segment) {
               nfs_status = NFS4ERR_SERVERFAULT;
               goto out;
          }
     } while (!res.last_segment);

     /* Update stateid.seqid and copy to current */
     update_stateid(layout_state,
                    &pres->LAYOUTGET4res_u.logr_resok4.logr_stateid,
                    data,
                    tag);

     pres->LAYOUTGET4res_u.logr_resok4.logr_return_on_close =
          layout_state->state_data.layout.state_return_on_close;

     /* Now the layout specific information */
     pres->LAYOUTGET4res_u.logr_resok4.logr_layout.logr_layout_len
          = numlayouts;
     pres->LAYOUTGET4res_u.logr_resok4.logr_layout.logr_layout_val
          = layouts;

     nfs_status = NFS4_OK;

out:

     if (pres->logr_status != NFS4_OK) {
          if (layouts) {
               size_t i;
               for (i = 0; i < numlayouts; i++) {
                    if (layouts[i].lo_content.loc_body.loc_body_val) {
                         Mem_Free(layouts[i].lo_content.loc_body.loc_body_val);
                    }
               }
               Mem_Free(layouts);
          }

          if ((layout_state) && (layout_state->state_seqid == 0)) {
               state_del(layout_state,
                         data->pclient,
                         &state_status);
               layout_state = NULL;
          }
     }

     pres->logr_status = nfs_status;

#else
     pres->logr_status = NFS4ERR_NOTSUPP;
#endif /* _USE_FSALMDS*/
     return pres->logr_status;
}                               /* pnfs_layoutget */


#ifdef _USE_FSALMDS

/**
 *
 * \brief Get or make a layout state
 *
 * If the stateid supplied by the client refers to a layout state,
 * that state is returned.  Otherwise, if it is a share, lock, or
 * delegation, a new state is created.  Any layout state matching
 * clientid, file, and type is freed.
 *
 * \param data             [IN]  Pointer to the compound request's data
 * \param supplied_stateid [IN]  Pointer to the stateid included in
 *                               the arguments to layoutget
 * \param layout_type      [IN]  The type of layout being requested.
 * \param layout_state     [OUT] The layout state.
 *
 * \return NFS4_OK if successfull, other values show an error.
 */

static nfsstat4
acquire_layout_state(compound_data_t *data,
                     stateid4 *supplied_stateid,
                     layouttype4 layout_type,
                     state_t **layout_state,
                     const char *tag)
{
     /* State associated with the client-supplied stateid */
     state_t *supplied_state = NULL;
     /* State owner for per-clientid states */
     state_owner_t *clientid_owner = NULL;
     /* Return from this function */
     nfsstat4 nfs_status = 0;
     /* Return from state functions */
     state_status_t state_status = 0;
     /* Layout state, forgotten about by caller */
     state_t *condemned_state = NULL;

     if ((state_status = get_clientid_owner(data->psession->clientid,
                                            &clientid_owner))
         != STATE_SUCCESS) {
          nfs_status = nfs4_Errno_state(state_status);
     }

     /* Retrieve state corresponding to supplied ID, inspect it and, if
        necessary, create a new layout state */

     if ((nfs_status = nfs4_Check_Stateid(supplied_stateid,
                                          data->current_entry,
                                          0LL,
                                          &supplied_state,
                                          data,
                                          STATEID_SPECIAL_CURRENT,
                                          tag)) != NFS4_OK) {
          goto out;
     }

     if (supplied_state->state_type == STATE_TYPE_LAYOUT) {
          /* If the state supplied is a layout state, we can simply
           * use it */
          *layout_state = supplied_state;
     } else if ((supplied_state->state_type == STATE_TYPE_SHARE) ||
                (supplied_state->state_type == STATE_TYPE_DELEG) ||
                (supplied_state->state_type == STATE_TYPE_LOCK)) {
          /* For share, delegation, and lock states, create a new
             layout state. */
          state_data_t layout_data;
          memset(&layout_data, 0, sizeof(state_data_t));
          /* See if a layout state already exists */
          state_status = state_lookup_layout_state(data->current_entry,
                                                   clientid_owner,
                                                   layout_type,
                                                   &condemned_state);
          /* If it does, we assume that the client is using the
             forgetful model and has forgotten it had any layouts.
             Free all layouts associated with the state and delete
             it. */
          if (state_status == STATE_SUCCESS) {
               /* Flag indicating whether all layouts were returned
                  and the state was deleted */
               bool_t deleted = FALSE;
               struct pnfs_segment entire = {
                    .io_mode = LAYOUTIOMODE4_ANY,
                    .offset = 0,
                    .length = NFS4_UINT64_MAX
               };
               if ((nfs_status
                    = nfs4_return_one_state(data->current_entry,
                                            data->pclient,
                                            data->pcontext,
                                            TRUE,
                                            FALSE,
                                            0,
                                            condemned_state,
                                            entire,
                                            0,
                                            NULL,
                                            &deleted)) != NFS4_OK) {
                    goto out;
               }
               if (!deleted) {
                    nfs_status = NFS4ERR_SERVERFAULT;
                    goto out;
               }
               condemned_state = NULL;
          } else if (state_status != STATE_NOT_FOUND) {
               nfs_status = nfs4_Errno_state(state_status);
               goto out;
          }

          layout_data.layout.state_layout_type = layout_type;
          layout_data.layout.state_return_on_close = FALSE;

          if (state_add(data->current_entry,
                        STATE_TYPE_LAYOUT,
                        &layout_data,
                        clientid_owner,
                        data->pclient,
                        data->pcontext,
                        layout_state,
                        &state_status) != STATE_SUCCESS) {
               nfs_status = nfs4_Errno_state(state_status);
               goto out;
          }

          init_glist(&(*layout_state)->state_data.layout.state_segments);
     } else {
          /* A state eixsts but is of an invalid type. */
          nfs_status = NFS4ERR_BAD_STATEID;
          goto out;
     }

out:

     return nfs_status;
}

/**
 *
 * \brief Grant and add one layout segment
 *
 * This is a wrapper around the FSAL call that populates one entry in
 * the logr_layout array and adds one segment to the state list.
 *
 * \param handle  [IN]     Pointer to the file handle
 * \param context [IN]     Pointer to the FSAL operaton context
 * \param arg     [IN]     The input arguments to the FSAL
 * \param res     [IN,OUT] The input/output and output arguments to
 *                         the FSAL
 * \param current [OUT]    The current entry in the logr_layout array.
 *
 * \return NFS4_OK if successfull, other values show an error.
 */

static nfsstat4
one_segment(fsal_handle_t *handle,
            fsal_op_context_t *context,
            state_t *layout_state,
            const struct fsal_layoutget_arg *arg,
            struct fsal_layoutget_res *res,
            layout4 *current)
{
     /* The initial position of the XDR stream after creation, so we
        can find the total length of encoded data. */
     size_t start_position = 0;
     /* XDR stream to encode loc_body of the current segment */
     XDR loc_body;
     /* Return code from this function */
     nfsstat4 nfs_status = 0;
     /* Return from state calls */
     state_status_t state_status = 0;
     /* Size of a loc_body buffer */
     size_t loc_body_size =
          context->export_context->fe_static_fs_info->loc_buffer_size;

     if (loc_body_size == 0) {
          LogCrit(COMPONENT_PNFS,
                  "The FSAL must specify a non-zero loc_buffer_size "
                  "in its fsal_staticfsinfo_t");
          nfs_status = NFS4ERR_SERVERFAULT;
          goto out;
     }

     /* Initialize the layout_content4 structure, allocate a buffer,
     and create an XDR stream for the FSAL to encode to. */

     current->lo_content.loc_type
          = arg->type;
     current->lo_content.loc_body.loc_body_val
          = Mem_Alloc(loc_body_size);

     xdrmem_create(&loc_body,
                   current->lo_content.loc_body.loc_body_val,
                   loc_body_size,
                   XDR_ENCODE);

     start_position = xdr_getpos(&loc_body);

     /*
      * XXX This assumes a single FSAL and must be changed after the
      * XXX Lieb Rearchitecture.  The MDS function structure
      * XXX associated with the current filehandle should be used.
      */

     nfs_status
          = fsal_mdsfunctions.layoutget(handle,
                                        context,
                                        &loc_body,
                                        arg,
                                        res);

     current->lo_content.loc_body.loc_body_len
          = xdr_getpos(&loc_body) - start_position;
     xdr_destroy(&loc_body);

     if (nfs_status != NFS4_OK) {
          goto out;
     }

     current->lo_offset = res->segment.offset;
     current->lo_length = res->segment.length;
     current->lo_iomode = res->segment.io_mode;

     if ((state_status = state_add_segment(layout_state,
                                           &res->segment,
                                           res->fsal_seg_data,
                                           res->return_on_close))
         != STATE_SUCCESS) {
          nfs_status = nfs4_Errno_state(state_status);
          goto out;
     }

out:

     if (nfs_status != NFS4_OK) {
          if (current->lo_content.loc_body.loc_body_val)
               Mem_Free(current->lo_content.loc_body.loc_body_val);
     }

     return nfs_status;
}
#endif /* _USE_FSALMDS */
