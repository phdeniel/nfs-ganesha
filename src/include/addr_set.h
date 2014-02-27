#include <netinet/in.h>
#include "cidr.h"


/* this becomes a client set. */
struct ip_addr_set {
	CIDR *subnet;
	uint32_t *bitmap;
};

bool in_addr_set(struct ip_addr_set *set,
		    struct sockaddr_storage *sock);
int addr_to_addr_set(struct sockaddr_storage *sock,
		      struct ip_addr_set **set);
int cidr_to_addr_set(CIDR *cidr, struct ip_addr_set **set);
void free_ip_addr_set(struct ip_addr_set *set);
