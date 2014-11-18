/*
 * vim:noexpandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright (C) Panasas Inc., 2014
 * Author: Jim Lieb jlieb@panasas.com
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * -------------
 */

/**
 * @defgroup IP Address management
 * @{
 */

/**
 * @file addr_set.c
 * @author Jim Lieb <jlieb@panasas.com>
 * @brief export manager
 */

#include "config.h"
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include "abstract_mem.h"
#include "addr_set.h"
#include "log.h"

#define DEF_CIDR_SIZE 23
#define MAX_CIDR_SIZE 16

static inline bool test_bit(uint32_t *bitmap, int bitnum)
{
	return !!(bitmap[bitnum / 32] & (1 << (bitnum % 32)));
}

static inline void set_bit(uint32_t *bitmap, int low, int high)
{
	int bit;

	for (bit = low; bit <= high; bit++)
		bitmap[bit / 32] |= (1 << (bit % 32));
}

void foreach_addr_in_set(struct ip_addr_set *set,
			 int (*cb)(struct sockaddr_storage *sock,
				   void *state))
{
}

/**
 * @brief Test if this address is in the set
 *
 * @param set  [IN] The set in question
 * @param sock [IN] The address to test
 *
 * @return true if in set, false otherwise
 */

bool is_in_addr_set(struct ip_addr_set *set,
		    struct sockaddr_storage *sock)
{
	CIDR *cidr_tmp;
	bool found = false;

	if (sock->ss_family == AF_INET) {
		struct in_addr *sk
			= &((struct sockaddr_in *)sock)->sin_addr;
		cidr_tmp = cidr_from_inaddr(sk);
	} else if (sock->ss_family == AF_INET6) {
		struct in6_addr *sk6
			= &((struct sockaddr_in6 *)sock)->sin6_addr;
		cidr_tmp = cidr_from_in6addr(sk6);
	} else {
		LogCrit(COMPONENT_CONFIG,
			 "This address is neither v4 or v6");
		return false;
	}
	if (cidr_contains(set->subnet, cidr_tmp) == 0) {
		uint32_t addr, mask;

		if (set->bitmap != NULL) {
			memcpy(&addr, &set->subnet->addr, sizeof(uint32_t));
			memcpy(&mask, &set->subnet->mask, sizeof(uint32_t));
			addr &= ~mask;
			found =  test_bit(set->bitmap, addr);
		} else
			found = true;
	}
	cidr_free(cidr_tmp);
	return found;
}

/**
 * @brief Merge this CIDR into the address set
 *
 * @param cidr [IN] - the CIDR to be merged
 * @param set  [IN/OUT] - call by reference to the set in question.
 *
 * @return  0 if added, < 0 (-errno) on failure.
 */

int cidr_to_addr_set(CIDR *cidr, struct ip_addr_set **set)
{
	struct ip_addr_set *my_set = *set;
	uint32_t cidr_mask;

	if (my_set == NULL) {
		int cidr_size;
		uint32_t cidr_addr;
		int mapsize;

		my_set = gsh_calloc(1, sizeof(struct ip_addr_set));
		if (my_set == NULL) {
			LogCrit(COMPONENT_CONFIG,
				"Cannot allocate an address set");
			return -ENOMEM;
		}
		cidr_size = cidr_get_pflen(cidr);
		if (cidr_size < MAX_CIDR_SIZE)
			return -EINVAL; /* too big */
		if (cidr_size > DEF_CIDR_SIZE)
			cidr_size = DEF_CIDR_SIZE;
		cidr_mask = ~(0xffffffff >> cidr_size);
		errno = 0;
		my_set->subnet = cidr_dup(cidr);
		if (my_set->subnet == NULL) {
			gsh_free(my_set);
			return -errno;
		}
		mapsize = (((1 << (32 - cidr_size)) + 31) / 32) * 4;
		my_set->bitmap = gsh_calloc(1, mapsize);
		if (my_set->bitmap == NULL) {
			LogCrit(COMPONENT_CONFIG,
				"Could not allocate ip addr set bitmap");
			cidr_free(my_set->subnet);
			gsh_free(my_set);
			return -ENOMEM;
		}
		memcpy(&cidr_addr, &cidr->addr, sizeof(uint32_t));
		cidr_addr &= ~cidr_mask;
		memcpy(&my_set->subnet->addr, &cidr_addr, sizeof(uint32_t));
		memcpy(&cidr_mask, &cidr->mask, sizeof(uint32_t));
		cidr_mask &= cidr_mask;
		memcpy(&my_set->subnet->mask, &cidr_mask, sizeof(uint32_t));
		*set = my_set;
	}
	if (cidr_contains(my_set->subnet, cidr) == 0) {
		CIDR *first, *last;
		uint32_t low, high, subnet;

		memcpy(&cidr_mask, &cidr->mask, sizeof(uint32_t));
		memcpy(&subnet, &my_set->subnet->mask, sizeof(uint32_t));
		if (~cidr_mask != 0) { /* subnet, set range */
			first = cidr_addr_hostmin(cidr);
			memcpy(&low, &first->addr, sizeof(uint32_t));
			last = cidr_addr_hostmax(cidr);
			memcpy(&high, &last->addr, sizeof(uint32_t));
			cidr_free(first);
			cidr_free(last);
		} else { /* unicast, set one bit */
			memcpy(&low, &cidr->addr, sizeof(uint32_t));
			high = low;
		}
		low &= ~subnet;
		high &= ~subnet;
		set_bit(my_set->bitmap, low, high);
		return 0;
	}
	return -EINVAL; /* no match or fit */
}

/**
 * @brief Add this IP address to the address set.
 *
 * The address set is a CIDR subnet with a bitmap for the host
 * range. The set param is call by reference so it can return
 * a new set if the reference == NULL.  The sock must be within the
 * subnet of the set in order to be added.  If it is outside the
 * range and the subnet cannot be expanded to handle it, return NULL.
 * A NULL return can then be handled by a second call with *set == NULL.
 *
 * @param sock [IN] - The v4 or v6 address to add
 * @param set  [IN/OUT] - call by reference to the set in question.
 *
 * @return 0 if added, < 0 (-errno) on failure.
 *
 */

int addr_to_addr_set(struct sockaddr_storage *sock,
		      struct ip_addr_set **set)
{
	CIDR *cidr_tmp;
	bool ret = false;

	errno = 0;
	if (sock->ss_family == AF_INET) {
		struct in_addr *sk
			= &((struct sockaddr_in *)sock)->sin_addr;
		cidr_tmp = cidr_from_inaddr(sk);
	} else if (sock->ss_family == AF_INET6) {
		struct in6_addr *sk6
			= &((struct sockaddr_in6 *)sock)->sin6_addr;
		cidr_tmp = cidr_from_in6addr(sk6);
	} else {
		LogCrit(COMPONENT_CONFIG,
			 "This address is neither v4 or v6");
		return -EINVAL;
	}
	if (cidr_tmp == NULL)
		return -errno;
	ret = cidr_to_addr_set(cidr_tmp, set);
	cidr_free(cidr_tmp);
	return ret;
}

/**
 * @brief Resolves hostname and adds IP address to the address set.
 *
 * The address set is a CIDR subnet with a bitmap for the host
 * range. The set param is call by reference so it can return
 * a new set if the reference == NULL.  The sock must be within the
 * subnet of the set in order to be added.  If it is outside the
 * range and the subnet cannot be expanded to handle it, return NULL.
 * A NULL return can then be handled by a second call with *set == NULL.
 *
 * @param hostname [IN] - The hostname to be converted to IPv4
 * @param set  [IN/OUT] - call by reference to the set in question.
 *
 * @return 0 if added, < 0 (-errno) on failure.
 *
 */

int hostnamev4_to_addr_set(char *hostname,
			struct ip_addr_set **set)
{
	struct sockaddr_storage addr;
	struct hostent *hp = NULL;

	hp = gethostbyname(hostname);
	if (hp == NULL)
		return -EINVAL;

	memcpy((char *)&addr, hp->h_addr, hp->h_length);

	return addr_to_addr_set(&addr, set);
}

/**
 * @brief Free an address set
 *
 * @param set - the set to be freed
 */

void free_ip_addr_set(struct ip_addr_set *set)
{
	if (set->subnet != NULL)
		cidr_free(set->subnet);
	if (set->bitmap != NULL)
		gsh_free(set->bitmap);
	gsh_free(set);
}


/**
 * @brief Allocates an address set
 *
 * @param nothing (void function)
 *
 * @return the allocated ip_addr_set or NULL is an error occurred
 *
 */

struct ip_addr_set *alloc_ip_addr_set(void)
{
	struct ip_addr_set *set = NULL;

	set = gsh_malloc(sizeof(struct ip_addr_set));
	if (set == NULL)
		return NULL;

	set->subnet = gsh_malloc(sizeof(CIDR));
	if (set->subnet	== NULL) {
		gsh_free(set);
		return NULL;
	}

	set->bitmap = gsh_malloc(sizeof(uint32_t));
	if (set->bitmap == NULL) {
		gsh_free(set->subnet);
		gsh_free(set);
		return NULL;
	}

	return set;
}
/** @} */
