#include <netinet/in.h>
#include "cidr.h"


/* this becomes a client set. */
struct ip_addr_set {
	CIDR *subnet;
	uint32_t *bitmap;
};

bool is_in_addr_set(struct ip_addr_set *set,
		    struct sockaddr_storage *sock);
int addr_to_addr_set(struct sockaddr_storage *sock,
		      struct ip_addr_set **set);
int hostnamev4_to_addr_set(char *hostname,
			struct ip_addr_set **set);
int cidr_to_addr_set(CIDR *cidr, struct ip_addr_set **set);
void free_ip_addr_set(struct ip_addr_set *set);
struct ip_addr_set *alloc_ip_addr_set(void);
struct ip_addr_set *alloc_ip_addr_set(void);
int nl_name_to_addr_set(char *hostname,
			void *other);
