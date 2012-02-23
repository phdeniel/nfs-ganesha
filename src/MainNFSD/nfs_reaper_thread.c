/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
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
 * nfs_reaper_thread.c : check for expired clients and whack them.
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _SOLARIS
#include "solaris_port.h"
#endif

#include <pthread.h>
#include <unistd.h>
#include "log_macros.h"
#include "stuff_alloc.h"
#include "nfs4.h"
#include "sal_functions.h"
#include "nfs_proto_functions.h"
#include "nfs_core.h"

unsigned int reaper_delay = 10;

void *reaper_thread(void *unused)
{
        hash_table_t *ht = ht_client_id;
        struct rbt_head *head_rbt;
        hash_data_t *pdata = NULL;
        int i;
        struct rbt_node *pn;
        nfs_client_id_t *clientp;

#ifndef _NO_BUDDY_SYSTEM
        if((i = BuddyInit(&nfs_param.buddy_param_admin)) != BUDDY_SUCCESS) {
        /* Failed init */
                LogFatal(COMPONENT_MAIN,
                    "Memory manager could not be initialized");
        }
        LogInfo(COMPONENT_MAIN, "Memory manager successfully initialized");
#endif

        SetNameFunction("reaper_thr");

        while(1) {
                /* Initial wait */
                /* TODO: should this be configurable? */
                /* sleep(nfs_param.core_param.reaper_delay); */
                sleep(reaper_delay);
                LogFullDebug(COMPONENT_MAIN,
                    "NFS reaper : now checking clients");

                /* For each bucket of the hashtable */
                for(i = 0; i < ht->parameter.index_size; i++) {
                        head_rbt = &(ht->array_rbt[i]);

restart:
                        /* acquire mutex */
                        P_w(&(ht->array_lock[i]));

                        /* go through all entries in the red-black-tree*/
                        RBT_LOOP(head_rbt, pn) {
                                pdata = RBT_OPAQ(pn);

                                clientp =
                                    (nfs_client_id_t *)pdata->buffval.pdata;
                                if (clientp->confirmed != EXPIRED_CLIENT_ID &&
                                    nfs4_is_lease_expired(clientp)) {
                                        V_w(&(ht->array_lock[i]));
                                        LogDebug(COMPONENT_MAIN,
                                            "NFS reaper: expire client %s",
                                            clientp->client_name);
                                        nfs_client_id_expire(clientp);
                                        goto restart;
                                }

                                if (clientp->confirmed == EXPIRED_CLIENT_ID) {
                                        LogDebug(COMPONENT_MAIN,
                                            "reaper: client %s already expired",
                                            clientp->client_name);
                                }

                                RBT_INCREMENT(pn);
                        }
                        V_w(&(ht->array_lock[i]));
                }

        }                           /* while ( 1 ) */

        return NULL;
}
