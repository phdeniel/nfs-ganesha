/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright CEA/DAM/DIF  (2008)
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
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
 * ---------------------------------------
 */

/**
 * \file    nfs_stats_thread.c
 * \author  $Author: deniel $
 * \date    $Date: 2006/02/22 12:01:58 $
 * \version $Revision: 1.6 $
 * \brief   The file that contain the 'stats_thread' routine for the nfsd.
 *
 * nfs_stats_thread.c : The file that contain the 'stats_thread' routine for the nfsd.
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
#include <sys/stat.h>
#include <time.h>
#include "nfs_core.h"
#include "nfs_stat.h"
#include "nfs_exports.h"
#include "log.h"

extern hash_table_t *ht_ip_stats[NB_MAX_WORKER_THREAD];

#ifndef _NO_BUDDY_SYSTEM
extern buddy_stats_t global_tcp_dispatcher_buddy_stat;
#endif

void set_min_latency(nfs_request_stat_item_t *cur_stat, unsigned int val)
{
  if(val > 0)
    {
      if(val < cur_stat->min_latency)
        {
          cur_stat->min_latency = val;
        }
    }
}

void set_max_latency(nfs_request_stat_item_t *cur_stat, unsigned int val)
{
  if(val > cur_stat->max_latency)
    {
      cur_stat->max_latency = val;
    }
}

/*
 * This function collects statistics from all Ganesha system modules so that they can then
 * be pushed into various users (e.g.: a statistics file, a network mgmt serviece, ...)
 * That is why collection of statistics is separated into its own function so that any user
 * can call for it.
 */
void stats_collect (ganesha_stats_t                 *ganesha_stats)
{
    cache_inode_stat_t     *global_cache_inode_stat = &ganesha_stats->global_cache_inode;
    hash_stat_t            *cache_inode_stat = &ganesha_stats->cache_inode_hstat;
    nfs_worker_stat_t      *global_worker_stat = &ganesha_stats->global_worker_stat;
    fsal_statistics_t      *global_fsal_stat = &ganesha_stats->global_fsal;
#ifndef _NO_BUDDY_SYSTEM
    buddy_stats_t *global_buddy_stat = &ganesha_stats->global_buddy;
#endif
    unsigned int           i, j;


    /* Zeroing the cache_stats */
    global_cache_inode_stat->nb_gc_lru_active = 0;
    global_cache_inode_stat->nb_gc_lru_total = 0;
    global_cache_inode_stat->nb_call_total = 0;

    memset(global_cache_inode_stat->func_stats.nb_err_unrecover, 0,
             sizeof(unsigned int) * CACHE_INODE_NB_COMMAND);

    /* Merging the cache inode stats for every thread */
    for (i = 0; i < nfs_param.core_param.nb_worker; i++) {
        global_cache_inode_stat->nb_gc_lru_active +=
            workers_data[i].cache_inode_client.stat.nb_gc_lru_active;
        global_cache_inode_stat->nb_gc_lru_total +=
            workers_data[i].cache_inode_client.stat.nb_gc_lru_total;
        global_cache_inode_stat->nb_call_total +=
            workers_data[i].cache_inode_client.stat.nb_call_total;

          for (j = 0; j < CACHE_INODE_NB_COMMAND; j++) {
              if (i == 0) {
                  global_cache_inode_stat->func_stats.nb_success[j] =
                      workers_data[i].cache_inode_client.stat.func_stats.nb_success[j];
                  global_cache_inode_stat->func_stats.nb_call[j] =
                      workers_data[i].cache_inode_client.stat.func_stats.nb_call[j];
                  global_cache_inode_stat->func_stats.nb_err_retryable[j] =
                      workers_data[i].cache_inode_client.stat.func_stats.
                      nb_err_retryable[j];
                  global_cache_inode_stat->func_stats.nb_err_unrecover[j] =
                      workers_data[i].cache_inode_client.stat.func_stats.
                      nb_err_unrecover[j];
                } else {
                  global_cache_inode_stat->func_stats.nb_success[j] +=
                      workers_data[i].cache_inode_client.stat.func_stats.nb_success[j];
                  global_cache_inode_stat->func_stats.nb_call[j] +=
                      workers_data[i].cache_inode_client.stat.func_stats.nb_call[j];
                  global_cache_inode_stat->func_stats.nb_err_retryable[j] +=
                      workers_data[i].cache_inode_client.stat.func_stats.
                      nb_err_retryable[j];
                  global_cache_inode_stat->func_stats.nb_err_unrecover[j] +=
                      workers_data[i].cache_inode_client.stat.func_stats.
                      nb_err_unrecover[j];
              }
          }
    }

    /* This is done only on worker[0]: the hashtable is shared and worker 0 always exists */
    HashTable_GetStats(workers_data[0].ht, cache_inode_stat);

    /* Merging the NFS protocols stats together */
    global_worker_stat->nb_total_req = 0;
    global_worker_stat->nb_udp_req = 0;
    global_worker_stat->nb_tcp_req = 0;
    global_worker_stat->stat_req.nb_mnt1_req = 0;
    global_worker_stat->stat_req.nb_mnt3_req = 0;
    global_worker_stat->stat_req.nb_nfs2_req = 0;
    global_worker_stat->stat_req.nb_nfs3_req = 0;
    global_worker_stat->stat_req.nb_nfs4_req = 0;
    global_worker_stat->stat_req.nb_nlm4_req = 0;
    global_worker_stat->stat_req.nb_nfs40_op = 0;
    global_worker_stat->stat_req.nb_nfs41_op = 0;
    global_worker_stat->stat_req.nb_rquota1_req = 0;
    global_worker_stat->stat_req.nb_rquota2_req = 0;

    /* prepare for computing pending request stats */
    ganesha_stats->min_pending_request = 10000000;
    ganesha_stats->max_pending_request = 0;
    ganesha_stats->total_pending_request = 0;
    ganesha_stats->average_pending_request = 0;
    ganesha_stats->len_pending_request = 0;

    for (i = 0; i < nfs_param.core_param.nb_worker; i++) {
        global_worker_stat->nb_total_req += workers_data[i].stats.nb_total_req;
        global_worker_stat->nb_udp_req += workers_data[i].stats.nb_udp_req;
        global_worker_stat->nb_tcp_req += workers_data[i].stats.nb_tcp_req;
        global_worker_stat->stat_req.nb_mnt1_req +=
            workers_data[i].stats.stat_req.nb_mnt1_req;
        global_worker_stat->stat_req.nb_mnt3_req +=
            workers_data[i].stats.stat_req.nb_mnt3_req;
        global_worker_stat->stat_req.nb_nfs2_req +=
            workers_data[i].stats.stat_req.nb_nfs2_req;
        global_worker_stat->stat_req.nb_nfs3_req +=
            workers_data[i].stats.stat_req.nb_nfs3_req;
        global_worker_stat->stat_req.nb_nfs4_req +=
            workers_data[i].stats.stat_req.nb_nfs4_req;
        global_worker_stat->stat_req.nb_nfs40_op +=
            workers_data[i].stats.stat_req.nb_nfs40_op;
        global_worker_stat->stat_req.nb_nfs41_op +=
            workers_data[i].stats.stat_req.nb_nfs41_op;

        global_worker_stat->stat_req.nb_nlm4_req +=
            workers_data[i].stats.stat_req.nb_nlm4_req;

        global_worker_stat->stat_req.nb_rquota1_req +=
            workers_data[i].stats.stat_req.nb_nlm4_req;

        global_worker_stat->stat_req.nb_rquota2_req +=
            workers_data[i].stats.stat_req.nb_nlm4_req;

        for (j = 0; j < MNT_V1_NB_COMMAND; j++) {
            if (i == 0) {
                global_worker_stat->stat_req.stat_req_mnt1[j].total =
                    workers_data[i].stats.stat_req.stat_req_mnt1[j].total;
                global_worker_stat->stat_req.stat_req_mnt1[j].success =
                    workers_data[i].stats.stat_req.stat_req_mnt1[j].success;
                global_worker_stat->stat_req.stat_req_mnt1[j].dropped =
                    workers_data[i].stats.stat_req.stat_req_mnt1[j].dropped;
            } else {
                global_worker_stat->stat_req.stat_req_mnt1[j].total +=
                    workers_data[i].stats.stat_req.stat_req_mnt1[j].total;
                global_worker_stat->stat_req.stat_req_mnt1[j].success +=
                    workers_data[i].stats.stat_req.stat_req_mnt1[j].success;
                global_worker_stat->stat_req.stat_req_mnt1[j].dropped +=
                    workers_data[i].stats.stat_req.stat_req_mnt1[j].dropped;
            }
        }

        for (j = 0; j < MNT_V3_NB_COMMAND; j++) {
            if (i == 0) {
                global_worker_stat->stat_req.stat_req_mnt3[j].total =
                    workers_data[i].stats.stat_req.stat_req_mnt3[j].total;
                global_worker_stat->stat_req.stat_req_mnt3[j].success =
                    workers_data[i].stats.stat_req.stat_req_mnt3[j].success;
                global_worker_stat->stat_req.stat_req_mnt3[j].dropped =
                    workers_data[i].stats.stat_req.stat_req_mnt3[j].dropped;
            } else {
                global_worker_stat->stat_req.stat_req_mnt3[j].total +=
                    workers_data[i].stats.stat_req.stat_req_mnt3[j].total;
                global_worker_stat->stat_req.stat_req_mnt3[j].success +=
                    workers_data[i].stats.stat_req.stat_req_mnt3[j].success;
                global_worker_stat->stat_req.stat_req_mnt3[j].dropped +=
                    workers_data[i].stats.stat_req.stat_req_mnt3[j].dropped;
            }
        }

        for (j = 0; j < NFS_V2_NB_COMMAND; j++) {
            if (i == 0) {
                global_worker_stat->stat_req.stat_req_nfs2[j].total =
                    workers_data[i].stats.stat_req.stat_req_nfs2[j].total;
                global_worker_stat->stat_req.stat_req_nfs2[j].success =
                    workers_data[i].stats.stat_req.stat_req_nfs2[j].success;
                global_worker_stat->stat_req.stat_req_nfs2[j].dropped =
                    workers_data[i].stats.stat_req.stat_req_nfs2[j].dropped;
            } else {
                global_worker_stat->stat_req.stat_req_nfs2[j].total +=
                    workers_data[i].stats.stat_req.stat_req_nfs2[j].total;
                global_worker_stat->stat_req.stat_req_nfs2[j].success +=
                    workers_data[i].stats.stat_req.stat_req_nfs2[j].success;
                global_worker_stat->stat_req.stat_req_nfs2[j].dropped +=
                    workers_data[i].stats.stat_req.stat_req_nfs2[j].dropped;
            }
        }

        for (j = 0; j < NFS_V3_NB_COMMAND; j++) {
            if (i == 0) {
                global_worker_stat->stat_req.stat_req_nfs3[j].total =
                    workers_data[i].stats.stat_req.stat_req_nfs3[j].total;
                global_worker_stat->stat_req.stat_req_nfs3[j].success =
                    workers_data[i].stats.stat_req.stat_req_nfs3[j].success;
                global_worker_stat->stat_req.stat_req_nfs3[j].dropped =
                    workers_data[i].stats.stat_req.stat_req_nfs3[j].dropped;
                global_worker_stat->stat_req.stat_req_nfs3[j].tot_latency =
                    workers_data[i].stats.stat_req.stat_req_nfs3[j].tot_latency;
                global_worker_stat->stat_req.stat_req_nfs3[j].min_latency =
                    workers_data[i].stats.stat_req.stat_req_nfs3[j].min_latency;
                global_worker_stat->stat_req.stat_req_nfs3[j].max_latency =
                    workers_data[i].stats.stat_req.stat_req_nfs3[j].max_latency;
            } else {
                global_worker_stat->stat_req.stat_req_nfs3[j].total +=
                    workers_data[i].stats.stat_req.stat_req_nfs3[j].total;
                global_worker_stat->stat_req.stat_req_nfs3[j].success +=
                    workers_data[i].stats.stat_req.stat_req_nfs3[j].success;
                global_worker_stat->stat_req.stat_req_nfs3[j].dropped +=
                    workers_data[i].stats.stat_req.stat_req_nfs3[j].dropped;
                global_worker_stat->stat_req.stat_req_nfs3[j].tot_latency +=
                    workers_data[i].stats.stat_req.stat_req_nfs3[j].tot_latency;
                set_min_latency(&(global_worker_stat->stat_req.stat_req_nfs3[j]),
                                workers_data[i].stats.stat_req.stat_req_nfs3[j].min_latency);
                set_max_latency(&(global_worker_stat->stat_req.stat_req_nfs3[j]),
                                workers_data[i].stats.stat_req.stat_req_nfs3[j].max_latency);
            }
        }

        for (j = 0; j < NFS_V4_NB_COMMAND; j++) {
            if (i == 0) {
                global_worker_stat->stat_req.stat_req_nfs4[j].total =
                    workers_data[i].stats.stat_req.stat_req_nfs4[j].total;
                global_worker_stat->stat_req.stat_req_nfs4[j].success =
                    workers_data[i].stats.stat_req.stat_req_nfs4[j].success;
                global_worker_stat->stat_req.stat_req_nfs4[j].dropped =
                    workers_data[i].stats.stat_req.stat_req_nfs4[j].dropped;
            } else {
                global_worker_stat->stat_req.stat_req_nfs4[j].total +=
                    workers_data[i].stats.stat_req.stat_req_nfs4[j].total;
                global_worker_stat->stat_req.stat_req_nfs4[j].success +=
                    workers_data[i].stats.stat_req.stat_req_nfs4[j].success;
                global_worker_stat->stat_req.stat_req_nfs4[j].dropped +=
                    workers_data[i].stats.stat_req.stat_req_nfs4[j].dropped;
            }
        }

        for (j = 0; j < NFS_V40_NB_OPERATION; j++) {
            if (i == 0) {
                global_worker_stat->stat_req.stat_op_nfs40[j].total =
                    workers_data[i].stats.stat_req.stat_op_nfs40[j].total;
                global_worker_stat->stat_req.stat_op_nfs40[j].success =
                    workers_data[i].stats.stat_req.stat_op_nfs40[j].success;
                global_worker_stat->stat_req.stat_op_nfs40[j].failed =
                    workers_data[i].stats.stat_req.stat_op_nfs40[j].failed;
            } else {
                global_worker_stat->stat_req.stat_op_nfs40[j].total +=
                    workers_data[i].stats.stat_req.stat_op_nfs40[j].total;
                global_worker_stat->stat_req.stat_op_nfs40[j].success +=
                    workers_data[i].stats.stat_req.stat_op_nfs40[j].success;
                global_worker_stat->stat_req.stat_op_nfs40[j].failed +=
                    workers_data[i].stats.stat_req.stat_op_nfs40[j].failed;
            }
        }

        for (j = 0; j < NFS_V41_NB_OPERATION; j++) {
            if (i == 0) {
                global_worker_stat->stat_req.stat_op_nfs41[j].total =
                    workers_data[i].stats.stat_req.stat_op_nfs41[j].total;
                global_worker_stat->stat_req.stat_op_nfs41[j].success =
                    workers_data[i].stats.stat_req.stat_op_nfs41[j].success;
                global_worker_stat->stat_req.stat_op_nfs41[j].failed =
                    workers_data[i].stats.stat_req.stat_op_nfs41[j].failed;
            } else {
                global_worker_stat->stat_req.stat_op_nfs41[j].total +=
                    workers_data[i].stats.stat_req.stat_op_nfs41[j].total;
                global_worker_stat->stat_req.stat_op_nfs41[j].success +=
                    workers_data[i].stats.stat_req.stat_op_nfs41[j].success;
                global_worker_stat->stat_req.stat_op_nfs41[j].failed +=
                    workers_data[i].stats.stat_req.stat_op_nfs41[j].failed;
            }
        }

        for (j = 0; j < NLM_V4_NB_OPERATION; j++) {
            if (i == 0) {
                global_worker_stat->stat_req.stat_req_nlm4[j].total =
                    workers_data[i].stats.stat_req.stat_req_nlm4[j].total;
                global_worker_stat->stat_req.stat_req_nlm4[j].success =
                    workers_data[i].stats.stat_req.stat_req_nlm4[j].success;
                global_worker_stat->stat_req.stat_req_nlm4[j].dropped =
                    workers_data[i].stats.stat_req.stat_req_nlm4[j].dropped;
            } else {
                global_worker_stat->stat_req.stat_req_nlm4[j].total +=
                    workers_data[i].stats.stat_req.stat_req_nlm4[j].total;
                global_worker_stat->stat_req.stat_req_nlm4[j].success +=
                    workers_data[i].stats.stat_req.stat_req_nlm4[j].success;
                global_worker_stat->stat_req.stat_req_nlm4[j].dropped +=
                    workers_data[i].stats.stat_req.stat_req_nlm4[j].dropped;
            }
        }

        for (j = 0; j < RQUOTA_NB_COMMAND; j++) {
            if (i == 0) {
                global_worker_stat->stat_req.stat_req_rquota1[j].total =
                    workers_data[i].stats.stat_req.stat_req_rquota1[j].total;
                global_worker_stat->stat_req.stat_req_rquota1[j].success =
                    workers_data[i].stats.stat_req.stat_req_rquota1[j].success;
                global_worker_stat->stat_req.stat_req_rquota1[j].dropped =
                    workers_data[i].stats.stat_req.stat_req_rquota1[j].dropped;

                global_worker_stat->stat_req.stat_req_rquota2[j].total =
                    workers_data[i].stats.stat_req.stat_req_rquota2[j].total;
                global_worker_stat->stat_req.stat_req_rquota2[j].success =
                    workers_data[i].stats.stat_req.stat_req_rquota2[j].success;
                global_worker_stat->stat_req.stat_req_rquota2[j].dropped =
                    workers_data[i].stats.stat_req.stat_req_rquota2[j].dropped;
            } else {
                global_worker_stat->stat_req.stat_req_rquota1[j].total +=
                    workers_data[i].stats.stat_req.stat_req_rquota1[j].total;
                global_worker_stat->stat_req.stat_req_rquota1[j].success +=
                    workers_data[i].stats.stat_req.stat_req_rquota1[j].success;
                global_worker_stat->stat_req.stat_req_rquota1[j].dropped +=
                    workers_data[i].stats.stat_req.stat_req_rquota1[j].dropped;

                global_worker_stat->stat_req.stat_req_rquota2[j].total +=
                    workers_data[i].stats.stat_req.stat_req_rquota2[j].total;
                global_worker_stat->stat_req.stat_req_rquota2[j].success +=
                    workers_data[i].stats.stat_req.stat_req_rquota2[j].success;
                global_worker_stat->stat_req.stat_req_rquota2[j].dropped +=
                    workers_data[i].stats.stat_req.stat_req_rquota2[j].dropped;
            }
        }

        /* Computing the pending request stats */
        ganesha_stats->len_pending_request =
            workers_data[i].pending_request->nb_entry -
            workers_data[i].pending_request->nb_invalid;

        if (ganesha_stats->len_pending_request < ganesha_stats->min_pending_request)
            ganesha_stats->min_pending_request = ganesha_stats->len_pending_request;

        if (ganesha_stats->len_pending_request > ganesha_stats->max_pending_request)
            ganesha_stats->max_pending_request = ganesha_stats->len_pending_request;

        ganesha_stats->total_pending_request += ganesha_stats->len_pending_request;
    }                       /* for( i = 0 ; i < nfs_param.core_param.nb_worker ; i++ ) */

    /* Compute average pending request */
    ganesha_stats->average_pending_request = ganesha_stats->total_pending_request / nfs_param.core_param.nb_worker;

    for (j = 0; j < NFS_V3_NB_COMMAND; j++) {
        if (global_worker_stat->stat_req.stat_req_nfs3[j].total > 0) {
            ganesha_stats->avg_latency = (global_worker_stat->stat_req.stat_req_nfs3[j].tot_latency /
                                         global_worker_stat->stat_req.stat_req_nfs3[j].total);
        } else {
            ganesha_stats->avg_latency = 0;
        }
    }

    /* Printing the cache inode hash stat */
    nfs_dupreq_get_stats(&ganesha_stats->drc_udp, &ganesha_stats->drc_tcp);

    /* Printing the UIDMAP_TYPE hash table stats */
    idmap_get_stats(UIDMAP_TYPE, &ganesha_stats->uid_map, &ganesha_stats->uid_reverse);
    /* Printing the GIDMAP_TYPE hash table stats */
    idmap_get_stats(GIDMAP_TYPE, &ganesha_stats->gid_map, &ganesha_stats->gid_reverse);
    /* Stats for the IP/Name hashtable */
    nfs_ip_name_get_stats(&ganesha_stats->ip_name_map);

    /* fsal statistics */
    memset(global_fsal_stat, 0, sizeof(fsal_statistics_t));
    ganesha_stats->total_fsal_calls = 0;

    for (i = 0; i < nfs_param.core_param.nb_worker; i++) {
        for (j = 0; j < FSAL_NB_FUNC; j++) {
            ganesha_stats->total_fsal_calls += workers_data[i].stats.fsal_stats.func_stats.nb_call[j];

            global_fsal_stat->func_stats.nb_call[j] +=
                workers_data[i].stats.fsal_stats.func_stats.nb_call[j];
            global_fsal_stat->func_stats.nb_success[j] +=
                workers_data[i].stats.fsal_stats.func_stats.nb_success[j];
            global_fsal_stat->func_stats.nb_err_retryable[j] +=
                workers_data[i].stats.fsal_stats.func_stats.nb_err_retryable[j];
            global_fsal_stat->func_stats.nb_err_unrecover[j] +=
                workers_data[i].stats.fsal_stats.func_stats.nb_err_unrecover[j];
        }
    }

#ifndef _NO_BUDDY_SYSTEM
    /* buddy memory */
    memset(global_buddy_stat, 0, sizeof(buddy_stats_t));
    for (i = 0; i < nfs_param.core_param.nb_worker; i++) {
        global_buddy_stat->TotalMemSpace +=
            workers_data[i].stats.buddy_stats.TotalMemSpace;
        global_buddy_stat->ExtraMemSpace +=
            workers_data[i].stats.buddy_stats.ExtraMemSpace;

        global_buddy_stat->StdMemSpace += workers_data[i].stats.buddy_stats.StdMemSpace;
        global_buddy_stat->StdUsedSpace +=
            workers_data[i].stats.buddy_stats.StdUsedSpace;

        if(workers_data[i].stats.buddy_stats.StdUsedSpace > global_buddy_stat->WM_StdUsedSpace)
            global_buddy_stat->WM_StdUsedSpace =
                workers_data[i].stats.buddy_stats.StdUsedSpace;

        global_buddy_stat->NbStdPages += workers_data[i].stats.buddy_stats.NbStdPages;
        global_buddy_stat->NbStdUsed += workers_data[i].stats.buddy_stats.NbStdUsed;

        if(workers_data[i].stats.buddy_stats.NbStdUsed > global_buddy_stat->WM_NbStdUsed)
            global_buddy_stat->WM_NbStdUsed = workers_data[i].stats.buddy_stats.NbStdUsed;
    }

    /* Add aggregated Tcp Dispatcher buddy malloc */
    global_buddy_stat->TotalMemSpace +=
        global_tcp_dispatcher_buddy_stat.TotalMemSpace;
    global_buddy_stat->ExtraMemSpace +=
        global_tcp_dispatcher_buddy_stat.ExtraMemSpace;

    global_buddy_stat->StdMemSpace += global_tcp_dispatcher_buddy_stat.StdMemSpace;
    global_buddy_stat->StdUsedSpace +=
        global_tcp_dispatcher_buddy_stat.StdUsedSpace;

    if (global_tcp_dispatcher_buddy_stat.StdUsedSpace >
        global_tcp_dispatcher_buddy_stat.WM_StdUsedSpace)
        global_buddy_stat->WM_StdUsedSpace =
            global_tcp_dispatcher_buddy_stat.StdUsedSpace;

    global_buddy_stat->NbStdPages += global_tcp_dispatcher_buddy_stat.NbStdPages;
    global_buddy_stat->NbStdUsed += global_tcp_dispatcher_buddy_stat.NbStdUsed;

    if (global_tcp_dispatcher_buddy_stat.NbStdUsed > global_buddy_stat->WM_NbStdUsed)
        global_buddy_stat->WM_NbStdUsed = global_tcp_dispatcher_buddy_stat.NbStdUsed;
#endif
}

void *stats_thread(void *addr)
{
  FILE *stats_file = NULL;
  struct stat statref;
  struct stat stattest;
  time_t current_time;
  struct tm *current_time_struct;
  struct tm *boot_time_struct;
  char strdate[1024];
  char strbootdate[1024];
  unsigned int j = 0;
  int reopen_stats = FALSE;

  ganesha_stats_t        ganesha_stats;
  cache_inode_stat_t     *global_cache_inode_stat = &ganesha_stats.global_cache_inode;
  nfs_worker_stat_t      *global_worker_stat = &ganesha_stats.global_worker_stat;
  hash_stat_t            *cache_inode_stat = &ganesha_stats.cache_inode_hstat;
  hash_stat_t            *uid_map_hstat = &ganesha_stats.uid_map;
  hash_stat_t            *gid_map_hstat = &ganesha_stats.gid_map;
  hash_stat_t            *ip_name_hstat = &ganesha_stats.ip_name_map;
  hash_stat_t            *hstat_uid_reverse = &ganesha_stats.uid_reverse;
  hash_stat_t            *hstat_gid_reverse = &ganesha_stats.gid_reverse;
  hash_stat_t            *hstat_drc_udp = &ganesha_stats.drc_udp;
  hash_stat_t            *hstat_drc_tcp = &ganesha_stats.drc_tcp;
  fsal_statistics_t      *global_fsal_stat = &ganesha_stats.global_fsal;


#ifndef _NO_BUDDY_SYSTEM
  int rc = 0;
  buddy_stats_t *global_buddy_stat = &ganesha_stats.global_buddy;
#endif

  SetNameFunction("stat_thr");

#ifndef _NO_BUDDY_SYSTEM
  if((rc = BuddyInit(NULL)) != BUDDY_SUCCESS)
    {
      /* Failed init */
      LogFatal(COMPONENT_MAIN,
               "NFS STATS : Memory manager could not be initialized");
    }
  LogInfo(COMPONENT_MAIN,
          "NFS STATS : Memory manager successfully initialized");
#endif

  /* Open the stats file, in append mode */
  if((stats_file = fopen(nfs_param.core_param.stats_file_path, "a")) == NULL)
    {
      LogCrit(COMPONENT_MAIN,
              "NFS STATS : Could not open stats file %s, no stats will be made...",
              nfs_param.core_param.stats_file_path);
      return NULL;
    }

  if(stat(nfs_param.core_param.stats_file_path, &statref) != 0)
    {
      LogCrit(COMPONENT_MAIN,
              "NFS STATS : Could not get inode for %s, no stats will be made...",
              nfs_param.core_param.stats_file_path);
      fclose(stats_file);
      return NULL;
    }

#ifdef _SNMP_ADM_ACTIVE
  /* start snmp library */
  if(stats_snmp(workers_data) == 0)
    LogInfo(COMPONENT_MAIN,
            "NFS STATS: SNMP stats service was started successfully");
  else
    LogCrit(COMPONENT_MAIN,
            "NFS STATS: ERROR starting SNMP stats export thread");
#endif /*_SNMP_ADM_ACTIVE*/

  while(1)
    {
      /* Initial wait */
      sleep(nfs_param.core_param.stats_update_delay);

      /* Debug trace */
      LogInfo(COMPONENT_MAIN, "NFS STATS : now dumping stats");

      /* Stats main loop */
      if(stat(nfs_param.core_param.stats_file_path, &stattest) == 0)
        {
          if(stattest.st_ino != statref.st_ino)
            reopen_stats = TRUE;
        }
      else
        {
          if(errno == ENOENT)
            reopen_stats = TRUE;
        }

      /* Check is file has changed (the inode number will be different) */
      if(reopen_stats == TRUE)
        {
          /* Stats file has changed */
          LogEvent(COMPONENT_MAIN,
                   "NFS STATS : stats file has changed or was removed, I close and reopen it");
          fflush(stats_file);
          fclose(stats_file);
          if((stats_file = fopen(nfs_param.core_param.stats_file_path, "a")) == NULL)
            {
              LogCrit(COMPONENT_MAIN,
                      "NFS STATS : Could not open stats file %s, no further stats will be made...",
                      nfs_param.core_param.stats_file_path);
              return NULL;
            }
          statref = stattest;
          reopen_stats = FALSE;
        }

      /* Get the current epoch time */
      current_time = time(NULL);
      current_time_struct = localtime(&current_time);
      snprintf(strdate, 1024, "%u, %.2d/%.2d/%.4d %.2d:%.2d:%.2d ",
               (unsigned int)current_time,
               current_time_struct->tm_mday,
               current_time_struct->tm_mon + 1,
               1900 + current_time_struct->tm_year,
               current_time_struct->tm_hour,
               current_time_struct->tm_min,
               current_time_struct->tm_sec);

      /* Printing the general Stats */
      boot_time_struct = localtime(&ServerBootTime);
      snprintf(strbootdate, 1024, "%u, %.2d/%.2d/%.4d %.2d:%.2d:%.2d ",
               (unsigned int)ServerBootTime,
               boot_time_struct->tm_mday,
               boot_time_struct->tm_mon + 1,
               1900 + boot_time_struct->tm_year,
               boot_time_struct->tm_hour,
               boot_time_struct->tm_min,
               boot_time_struct->tm_sec);

      fprintf(stats_file, "NFS_SERVER_GENERAL,%s;%s\n", strdate, strbootdate);

      /* collect statistics */
      stats_collect(&ganesha_stats);

      /* Printing the cache_inode stat */
      fprintf(stats_file, "CACHE_INODE_CALLS,%s;%u,%u,%u",
              strdate,
              global_cache_inode_stat->nb_call_total,
              global_cache_inode_stat->nb_gc_lru_total,
              global_cache_inode_stat->nb_gc_lru_active);

      for(j = 0; j < CACHE_INODE_NB_COMMAND; j++)
        fprintf(stats_file, "|%u,%u,%u,%u",
                global_cache_inode_stat->func_stats.nb_call[j],
                global_cache_inode_stat->func_stats.nb_success[j],
                global_cache_inode_stat->func_stats.nb_err_retryable[j],
                global_cache_inode_stat->func_stats.nb_err_unrecover[j]);
      fprintf(stats_file, "\n");

      /* Pinting the cache inode hash stat */
      fprintf(stats_file,
              "CACHE_INODE_HASH,%s;%u,%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u\n",
              strdate, cache_inode_stat->dynamic.nb_entries, cache_inode_stat->computed.min_rbt_num_node,
              cache_inode_stat->computed.max_rbt_num_node, cache_inode_stat->computed.average_rbt_num_node,
              cache_inode_stat->dynamic.ok.nb_set, cache_inode_stat->dynamic.notfound.nb_set,
              cache_inode_stat->dynamic.err.nb_set, cache_inode_stat->dynamic.ok.nb_test,
              cache_inode_stat->dynamic.notfound.nb_test, cache_inode_stat->dynamic.err.nb_test,
              cache_inode_stat->dynamic.ok.nb_get, cache_inode_stat->dynamic.notfound.nb_get,
              cache_inode_stat->dynamic.err.nb_get, cache_inode_stat->dynamic.ok.nb_del,
              cache_inode_stat->dynamic.notfound.nb_del, cache_inode_stat->dynamic.err.nb_del);

      fprintf(stats_file, "NFS/MOUNT STATISTICS,%s;%u,%u,%u|%u,%u,%u,%u,%u|%u,%u,%u,%u\n",
              strdate,
              global_worker_stat->nb_total_req,
              global_worker_stat->nb_udp_req,
              global_worker_stat->nb_tcp_req,
              global_worker_stat->stat_req.nb_mnt1_req,
              global_worker_stat->stat_req.nb_mnt3_req,
              global_worker_stat->stat_req.nb_nfs2_req,
              global_worker_stat->stat_req.nb_nfs3_req,
              global_worker_stat->stat_req.nb_nfs4_req,
              ganesha_stats.total_pending_request,
              ganesha_stats.min_pending_request,
              ganesha_stats.max_pending_request,
              ganesha_stats.average_pending_request);

      fprintf(stats_file, "MNT V1 REQUEST,%s;%u", strdate,
              global_worker_stat->stat_req.nb_mnt1_req);
      for(j = 0; j < MNT_V1_NB_COMMAND; j++)
        fprintf(stats_file, "|%u,%u,%u",
                global_worker_stat->stat_req.stat_req_mnt1[j].total,
                global_worker_stat->stat_req.stat_req_mnt1[j].success,
                global_worker_stat->stat_req.stat_req_mnt1[j].dropped);
      fprintf(stats_file, "\n");

      fprintf(stats_file, "MNT V3 REQUEST,%s;%u", strdate,
              global_worker_stat->stat_req.nb_mnt3_req);
      for(j = 0; j < MNT_V3_NB_COMMAND; j++)
        fprintf(stats_file, "|%u,%u,%u",
                global_worker_stat->stat_req.stat_req_mnt3[j].total,
                global_worker_stat->stat_req.stat_req_mnt3[j].success,
                global_worker_stat->stat_req.stat_req_mnt3[j].dropped);
      fprintf(stats_file, "\n");

      fprintf(stats_file, "NFS V2 REQUEST,%s;%u", strdate,
              global_worker_stat->stat_req.nb_nfs2_req);
      for(j = 0; j < NFS_V2_NB_COMMAND; j++)
        fprintf(stats_file, "|%u,%u,%u",
                global_worker_stat->stat_req.stat_req_nfs2[j].total,
                global_worker_stat->stat_req.stat_req_nfs2[j].success,
                global_worker_stat->stat_req.stat_req_nfs2[j].dropped);
      fprintf(stats_file, "\n");

      fprintf(stats_file, "NFS V3 REQUEST,%s;%u", strdate,
              global_worker_stat->stat_req.nb_nfs3_req);
      for (j = 0; j < NFS_V3_NB_COMMAND; j++)
	{
          fprintf(stats_file, "|%u,%u,%u,%u,%u,%u,%u",
                  global_worker_stat->stat_req.stat_req_nfs3[j].total,
                  global_worker_stat->stat_req.stat_req_nfs3[j].success,
                  global_worker_stat->stat_req.stat_req_nfs3[j].dropped,
                  global_worker_stat->stat_req.stat_req_nfs3[j].tot_latency,
                  ganesha_stats.avg_latency,
                  global_worker_stat->stat_req.stat_req_nfs3[j].min_latency,
                  global_worker_stat->stat_req.stat_req_nfs3[j].max_latency);
        }
      fprintf(stats_file, "\n");

      fprintf(stats_file, "NFS V4 REQUEST,%s;%u", strdate,
              global_worker_stat->stat_req.nb_nfs4_req);
      for(j = 0; j < NFS_V4_NB_COMMAND; j++)
        fprintf(stats_file, "|%u,%u,%u",
                global_worker_stat->stat_req.stat_req_nfs4[j].total,
                global_worker_stat->stat_req.stat_req_nfs4[j].success,
                global_worker_stat->stat_req.stat_req_nfs4[j].dropped);
      fprintf(stats_file, "\n");

      fprintf(stats_file, "NFS V4.0 OPERATIONS,%s;%u", strdate,
              global_worker_stat->stat_req.nb_nfs40_op);
      for(j = 0; j < NFS_V40_NB_OPERATION; j++)
        fprintf(stats_file, "|%u,%u,%u",
                global_worker_stat->stat_req.stat_op_nfs40[j].total,
                global_worker_stat->stat_req.stat_op_nfs40[j].success,
                global_worker_stat->stat_req.stat_op_nfs40[j].failed);
      fprintf(stats_file, "\n");

      fprintf(stats_file, "NFS V4.1 OPERATIONS,%s;%u", strdate,
              global_worker_stat->stat_req.nb_nfs41_op);
      for(j = 0; j < NFS_V41_NB_OPERATION; j++)
        fprintf(stats_file, "|%u,%u,%u",
                global_worker_stat->stat_req.stat_op_nfs41[j].total,
                global_worker_stat->stat_req.stat_op_nfs41[j].success,
                global_worker_stat->stat_req.stat_op_nfs41[j].failed);
      fprintf(stats_file, "\n");

      fprintf(stats_file, "NLM V4 REQUEST,%s;%u", strdate,
              global_worker_stat->stat_req.nb_nlm4_req);
      for(j = 0; j < NLM_V4_NB_OPERATION; j++)
        fprintf(stats_file, "|%u,%u,%u",
                global_worker_stat->stat_req.stat_req_nlm4[j].total,
                global_worker_stat->stat_req.stat_req_nlm4[j].success,
                global_worker_stat->stat_req.stat_req_nlm4[j].dropped);
      fprintf(stats_file, "\n");

      fprintf(stats_file, "RQUOTA V1 REQUEST,%s;%u", strdate,
              global_worker_stat->stat_req.nb_rquota1_req);
      for(j = 0; j < RQUOTA_NB_COMMAND; j++)
        fprintf(stats_file, "|%u,%u,%u",
                global_worker_stat->stat_req.stat_req_rquota1[j].total,
                global_worker_stat->stat_req.stat_req_rquota1[j].success,
                global_worker_stat->stat_req.stat_req_rquota1[j].dropped);
      fprintf(stats_file, "\n");

      fprintf(stats_file, "RQUOTA V2 REQUEST,%s;%u", strdate,
              global_worker_stat->stat_req.nb_rquota2_req);
      for(j = 0; j < RQUOTA_NB_COMMAND; j++)
        fprintf(stats_file, "|%u,%u,%u",
                global_worker_stat->stat_req.stat_req_rquota2[j].total,
                global_worker_stat->stat_req.stat_req_rquota2[j].success,
                global_worker_stat->stat_req.stat_req_rquota2[j].dropped);
      fprintf(stats_file, "\n");

      fprintf(stats_file,
              "DUP_REQ_HASH,%s;%u,%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u\n",
              strdate, 
              hstat_drc_udp->dynamic.nb_entries            + hstat_drc_tcp->dynamic.nb_entries,
              hstat_drc_udp->computed.min_rbt_num_node     + hstat_drc_tcp->computed.min_rbt_num_node,
              hstat_drc_udp->computed.max_rbt_num_node     + hstat_drc_tcp->computed.max_rbt_num_node,
              hstat_drc_udp->computed.average_rbt_num_node + hstat_drc_tcp->computed.average_rbt_num_node,
              hstat_drc_udp->dynamic.ok.nb_set             + hstat_drc_tcp->dynamic.ok.nb_set,
              hstat_drc_udp->dynamic.notfound.nb_set       + hstat_drc_tcp->dynamic.notfound.nb_set,
              hstat_drc_udp->dynamic.err.nb_set            + hstat_drc_tcp->dynamic.err.nb_set,
              hstat_drc_udp->dynamic.ok.nb_test            + hstat_drc_tcp->dynamic.ok.nb_test,
              hstat_drc_udp->dynamic.notfound.nb_test      + hstat_drc_tcp->dynamic.notfound.nb_test,
              hstat_drc_udp->dynamic.err.nb_test           + hstat_drc_tcp->dynamic.err.nb_test,
              hstat_drc_udp->dynamic.ok.nb_get             + hstat_drc_tcp->dynamic.ok.nb_get,
              hstat_drc_udp->dynamic.notfound.nb_get       + hstat_drc_tcp->dynamic.notfound.nb_get,
              hstat_drc_udp->dynamic.err.nb_get            + hstat_drc_tcp->dynamic.err.nb_get,
              hstat_drc_udp->dynamic.ok.nb_del             + hstat_drc_tcp->dynamic.ok.nb_del,
              hstat_drc_udp->dynamic.notfound.nb_del       + hstat_drc_tcp->dynamic.notfound.nb_del,
              hstat_drc_udp->dynamic.err.nb_del            + hstat_drc_tcp->dynamic.err.nb_del);

      fprintf(stats_file,
              "UIDMAP_HASH,%s;%u,%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u\n", strdate,
              uid_map_hstat->dynamic.nb_entries, uid_map_hstat->computed.min_rbt_num_node,
              uid_map_hstat->computed.max_rbt_num_node, uid_map_hstat->computed.average_rbt_num_node,
              uid_map_hstat->dynamic.ok.nb_set, uid_map_hstat->dynamic.notfound.nb_set,
              uid_map_hstat->dynamic.err.nb_set, uid_map_hstat->dynamic.ok.nb_test,
              uid_map_hstat->dynamic.notfound.nb_test, uid_map_hstat->dynamic.err.nb_test,
              uid_map_hstat->dynamic.ok.nb_get, uid_map_hstat->dynamic.notfound.nb_get,
              uid_map_hstat->dynamic.err.nb_get, uid_map_hstat->dynamic.ok.nb_del,
              uid_map_hstat->dynamic.notfound.nb_del, uid_map_hstat->dynamic.err.nb_del);
      fprintf(stats_file,
              "UNAMEMAP_HASH,%s;%u,%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u\n",
              strdate, hstat_uid_reverse->dynamic.nb_entries,
              hstat_uid_reverse->computed.min_rbt_num_node,
              hstat_uid_reverse->computed.max_rbt_num_node,
              hstat_uid_reverse->computed.average_rbt_num_node,
              hstat_uid_reverse->dynamic.ok.nb_set, hstat_uid_reverse->dynamic.notfound.nb_set,
              hstat_uid_reverse->dynamic.err.nb_set, hstat_uid_reverse->dynamic.ok.nb_test,
              hstat_uid_reverse->dynamic.notfound.nb_test, hstat_uid_reverse->dynamic.err.nb_test,
              hstat_uid_reverse->dynamic.ok.nb_get, hstat_uid_reverse->dynamic.notfound.nb_get,
              hstat_uid_reverse->dynamic.err.nb_get, hstat_uid_reverse->dynamic.ok.nb_del,
              hstat_uid_reverse->dynamic.notfound.nb_del, hstat_uid_reverse->dynamic.err.nb_del);

      fprintf(stats_file,
              "GIDMAP_HASH,%s;%u,%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u\n", strdate,
              gid_map_hstat->dynamic.nb_entries, gid_map_hstat->computed.min_rbt_num_node,
              gid_map_hstat->computed.max_rbt_num_node, gid_map_hstat->computed.average_rbt_num_node,
              gid_map_hstat->dynamic.ok.nb_set, gid_map_hstat->dynamic.notfound.nb_set,
              gid_map_hstat->dynamic.err.nb_set, gid_map_hstat->dynamic.ok.nb_test,
              gid_map_hstat->dynamic.notfound.nb_test, gid_map_hstat->dynamic.err.nb_test,
              gid_map_hstat->dynamic.ok.nb_get, gid_map_hstat->dynamic.notfound.nb_get,
              gid_map_hstat->dynamic.err.nb_get, gid_map_hstat->dynamic.ok.nb_del,
              gid_map_hstat->dynamic.notfound.nb_del, gid_map_hstat->dynamic.err.nb_del);
      fprintf(stats_file,
              "GNAMEMAP_HASH,%s;%u,%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u\n",
              strdate, hstat_gid_reverse->dynamic.nb_entries,
              hstat_gid_reverse->computed.min_rbt_num_node,
              hstat_gid_reverse->computed.max_rbt_num_node,
              hstat_gid_reverse->computed.average_rbt_num_node,
              hstat_gid_reverse->dynamic.ok.nb_set, hstat_gid_reverse->dynamic.notfound.nb_set,
              hstat_gid_reverse->dynamic.err.nb_set, hstat_gid_reverse->dynamic.ok.nb_test,
              hstat_gid_reverse->dynamic.notfound.nb_test, hstat_gid_reverse->dynamic.err.nb_test,
              hstat_gid_reverse->dynamic.ok.nb_get, hstat_gid_reverse->dynamic.notfound.nb_get,
              hstat_gid_reverse->dynamic.err.nb_get, hstat_gid_reverse->dynamic.ok.nb_del,
              hstat_gid_reverse->dynamic.notfound.nb_del, hstat_gid_reverse->dynamic.err.nb_del);

      fprintf(stats_file,
              "IP_NAME_HASH,%s;%u,%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u|%u,%u,%u\n",
              strdate, ip_name_hstat->dynamic.nb_entries,
              ip_name_hstat->computed.min_rbt_num_node,
              ip_name_hstat->computed.max_rbt_num_node,
              ip_name_hstat->computed.average_rbt_num_node,
              ip_name_hstat->dynamic.ok.nb_set, ip_name_hstat->dynamic.notfound.nb_set,
              ip_name_hstat->dynamic.err.nb_set, ip_name_hstat->dynamic.ok.nb_test,
              ip_name_hstat->dynamic.notfound.nb_test, ip_name_hstat->dynamic.err.nb_test,
              ip_name_hstat->dynamic.ok.nb_get, ip_name_hstat->dynamic.notfound.nb_get,
              ip_name_hstat->dynamic.err.nb_get, ip_name_hstat->dynamic.ok.nb_del,
              ip_name_hstat->dynamic.notfound.nb_del, ip_name_hstat->dynamic.err.nb_del);

      fprintf(stats_file, "FSAL_CALLS,%s;%llu", strdate, ganesha_stats.total_fsal_calls);
      for(j = 0; j < FSAL_NB_FUNC; j++)
        fprintf(stats_file, "|%u,%u,%u,%u",
                global_fsal_stat->func_stats.nb_call[j],
                global_fsal_stat->func_stats.nb_success[j],
                global_fsal_stat->func_stats.nb_err_retryable[j],
                global_fsal_stat->func_stats.nb_err_unrecover[j]);
      fprintf(stats_file, "\n");

#ifndef _NO_BUDDY_SYSTEM

      /* total memory space preallocated, total space preallocated for pages, total space that overflows pages */
      /* total memory, used memory, avg used memory/worker, max used memory/worker */
      /* total pages, used pages, avg used pages/worker, max used pages/worker */
      fprintf(stats_file, "BUDDY_MEMORY,%s;%lu,%lu,%lu|%lu,%lu,%lu|%u,%u,%u,%u\n",
              strdate,
              (unsigned long)global_buddy_stat->TotalMemSpace,
              (unsigned long)global_buddy_stat->StdMemSpace,
              (unsigned long)global_buddy_stat->ExtraMemSpace,
              (unsigned long)global_buddy_stat->StdUsedSpace,
              (unsigned long)(global_buddy_stat->StdUsedSpace /
                              nfs_param.core_param.nb_worker),
              (unsigned long)global_buddy_stat->WM_StdUsedSpace,
              global_buddy_stat->NbStdPages, global_buddy_stat->NbStdUsed,
              global_buddy_stat->NbStdUsed / nfs_param.core_param.nb_worker,
              global_buddy_stat->WM_NbStdUsed);

#endif

      /* Flush the data written */
      fprintf(stats_file, "END, ----- NO MORE STATS FOR THIS PASS ----\n");
      fflush(stats_file);

      /* Now managed IP stats dump */
      nfs_ip_stats_dump(ht_ip_stats,
                        nfs_param.core_param.nb_worker,
                        nfs_param.core_param.stats_per_client_directory);

    }                           /* while ( 1 ) */

  return NULL;
}                               /* stats_thread */
