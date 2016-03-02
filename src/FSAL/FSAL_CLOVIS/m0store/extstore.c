/* -*- C -*- */
/*
 * COPYRIGHT 2014 SEAGATE LLC
 *
 * THIS DRAWING/DOCUMENT, ITS SPECIFICATIONS, AND THE DATA CONTAINED
 * HEREIN, ARE THE EXCLUSIVE PROPERTY OF SEAGATE LLC,
 * ISSUED IN STRICT CONFIDENCE AND SHALL NOT, WITHOUT
 * THE PRIOR WRITTEN PERMISSION OF SEAGATE TECHNOLOGY LIMITED,
 * BE REPRODUCED, COPIED, OR DISCLOSED TO A THIRD PARTY, OR
 * USED FOR ANY PURPOSE WHATSOEVER, OR STORED IN A RETRIEVAL SYSTEM
 * EXCEPT AS ALLOWED BY THE TERMS OF SEAGATE LICENSES AND AGREEMENTS.
 *
 * YOU SHOULD HAVE RECEIVED A COPY OF SEAGATE'S LICENSE ALONG WITH
 * THIS RELEASE. IF NOT PLEASE CONTACT A SEAGATE REPRESENTATIVE
 * http://www.xyratex.com/contact
 *
 * Original author:  Juan   Gonzalez <juan.gonzalez@seagate.com>
 *                   James  Morse    <james.s.morse@seagate.com>
 *                   Sining Wu       <sining.wu@seagate.com>
 * Original creation date: 30-Oct-2014
 */
/* This is a sample Clovis application. It will read data from
 * a file and write it to an object.
 * In General, steps are:
 * 1. Create a Clovis instance from the configuration data provided.
 * 2. Create an object.
 * 3. Read data from a file and fill it into Clovis buffers.
 * 4. Submit the write request.
 * 5. Wait for the write request to finish.
 * 6. Finalise the Clovis instance.
 */
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <assert.h>

#include "clovis/clovis.h"
#include "clovis/clovis_idx.h"

/* Currently Clovis can write at max 200 blocks in
 * a single request. This will change in future. */
#define CLOVIS_MAX_BLOCK_COUNT (200)

/* Clovis parameters */

/* Object id */
//static int   clovis_oid;

/* Blocksize in which data is to be written 
 * Clovis supports only 4K blocksize.
 * This will change in future.
 */
//static int   clovis_block_size;

/* Number of blocks to be copied */
//static int   clovis_block_count;

/* Clovis Instance */
static struct m0_clovis          *clovis_instance = NULL;

/* Clovis container */
static struct m0_clovis_container clovis_container;
static struct m0_clovis_realm     clovis_uber_realm;

/* Clovis Configuration */
static struct m0_clovis_config    clovis_conf;


/* 
 * This function initialises Clovis and Mero.
 * Creates a Clovis instance and initializes the 
 * realm to uber realm.
 */
int init_clovis(char *clovis_local_addr,
	        char *clovis_ha_addr,
		char *clovis_confd_addr,
		char *clovis_prof,
		char *clovis_index_dir)
{
	int rc;

	/* Initialize Clovis configuration */
	clovis_conf.cc_is_oostore            = false;
	clovis_conf.cc_is_read_verify        = false;
	clovis_conf.cc_local_addr            = clovis_local_addr;
	clovis_conf.cc_ha_addr               = clovis_ha_addr;
	clovis_conf.cc_confd                 = clovis_confd_addr;
	clovis_conf.cc_profile               = clovis_prof;
	clovis_conf.cc_tm_recv_queue_min_len = M0_NET_TM_RECV_QUEUE_DEF_LEN;
	clovis_conf.cc_max_rpc_msg_size      = M0_RPC_DEF_MAX_RPC_MSG_SIZE;

	/* Index service parameters */
	clovis_conf.cc_idx_service_id        = M0_CLOVIS_IDX_MOCK;
        clovis_conf.cc_idx_service_conf      = clovis_index_dir;

	/* Create Clovis instance */
	rc = m0_clovis_init(&clovis_instance, &clovis_conf, true);
	if (rc != 0) {
		printf("Failed to initilise Clovis\n");
		goto err_exit;
	}

	/* Container is where Entities (object) resides.
 	 * Currently, this feature is not implemented in Clovis.
 	 * We have only single realm: UBER REALM. In future with multiple realms
 	 * multiple applications can run in different containers. */
	m0_clovis_container_init(&clovis_container, 
				 NULL, &M0_CLOVIS_UBER_REALM,
				 clovis_instance);
	rc = clovis_container.co_realm.re_entity.en_sm.sm_rc;

	if (rc != 0) {
		printf("Failed to open uber realm\n");
		goto err_exit;
	}
	
	clovis_uber_realm = clovis_container.co_realm;
	return 0;

err_exit:
	return rc;
}

void fini_clovis(void)
{
	/* Finalize Clovis instance */
	m0_clovis_fini(&clovis_instance, true);
}

int create_object(struct m0_uint128 id)
{
	int                  rc;

	/* Clovis object */
	struct m0_clovis_obj obj;

	/* Clovis operation */
	struct m0_clovis_op *ops[1] = {NULL};

	memset(&obj, 0, sizeof(struct m0_clovis_obj));
 
 	/* Initialize obj structures 
 	 * Note: This api doesnot create an object. It simply fills
 	 * obj structure with require data. */
	m0_clovis_obj_init(&obj, &clovis_uber_realm, &id);

	/* Create object-create request */
	m0_clovis_entity_create(&obj.ob_entity, &ops[0]);

       /* Launch the request. This is a asynchronous call.
 	* This will actually create an object */
	m0_clovis_op_launch(ops, ARRAY_SIZE(ops));

	/* Wait for the object creation to finish */
	rc = m0_clovis_op_wait(
		ops[0], M0_BITS(M0_CLOVIS_OS_FAILED, M0_CLOVIS_OS_STABLE),
		m0_time_from_now(3,0));
	
	m0_clovis_op_fini(ops[0]);
	m0_clovis_op_free(ops[0]);
	m0_clovis_entity_fini(&obj.ob_entity);

	return rc;
}

int write_data_to_object(struct m0_uint128 id,
				struct m0_indexvec *ext,
				struct m0_bufvec *data,
				struct m0_bufvec *attr)
{
	int                  rc;  
	struct m0_clovis_obj obj;
	struct m0_clovis_op *ops[1] = {NULL};

	memset(&obj, 0, sizeof(struct m0_clovis_obj));

	/* Set the object entity we want to write */
	m0_clovis_obj_init(&obj, &clovis_uber_realm, &id);

	/* Create the write request */
	m0_clovis_obj_op(&obj, M0_CLOVIS_OC_WRITE, 
			 ext, data, attr, 0, &ops[0]);

	/* Launch the write request*/
	m0_clovis_op_launch(ops, 1);

	/* wait */
	rc = m0_clovis_op_wait(ops[0],
			M0_BITS(M0_CLOVIS_OS_FAILED,
				M0_CLOVIS_OS_STABLE),
			M0_TIME_NEVER);

	/* fini and release */
	m0_clovis_op_fini(ops[0]);
	m0_clovis_op_free(ops[0]);
	m0_clovis_entity_fini(&obj.ob_entity);

	return rc;
}

/*
 *  Local variables:
 *  c-indentation-style: "K&R"
 *  c-basic-offset: 8
 *  tab-width: 8
 *  fill-column: 80
 *  scroll-step: 1
 *  End:
 */
