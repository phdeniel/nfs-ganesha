/* -*- C -*- */
#ifndef EXTSTORE_H
#define EXTSTORE_H 
int init_clovis(char *clovis_local_addr,
	        char *clovis_ha_addr,
		char *clovis_confd_addr,
		char *clovis_prof,
		char *clovis_index_dir);

void fini_clovis(void);
/*
 *  Local variables:
 *  c-indentation-style: "K&R"
 *  c-basic-offset: 8
 *  tab-width: 8
 *  fill-column: 80
 *  scroll-step: 1
 *  End:
 */
#endif
