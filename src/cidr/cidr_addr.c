/*
 * Functions to generate various addresses based on a CIDR
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <string.h>

#include "stuff_alloc.h"
#include "../include/cidr.h"


/* Create a network address */
CIDR *
cidr_addr_network(const CIDR *addr)
{
	int i, j;
	CIDR *toret;

	/* Quick check */
	if(addr==NULL)
	{
		errno = EFAULT;
		return(NULL);
	}
	
	toret = cidr_alloc();
	if(toret==NULL)
		return(NULL); /* Preserve errno */
	toret->proto = addr->proto;
	
	/* The netmask is the same */
	memcpy(toret->mask, addr->mask, (16 * sizeof(toret->mask[0])) );

	/* Now just figure out the network address and spit it out */
	for(i=0 ; i<=15 ; i++)
	{
		for(j=7 ; j>=0 ; j--)
		{
			/* If we're into host bits, hop out */
			if( (addr->mask[i] & 1<<j) == 0)
				return(toret);

			/* Else, copy this network bit */
			toret->addr[i] |= (addr->addr[i] & 1<<j);
		}
	}

	/*
	 * We only get here on host (/32 or /128) addresses; shorter masks
	 * return earlier.  But it's as correct as can be to just say the
	 * same here, so...
	 */
	return(toret);
}


/* And a broadcast */
CIDR *
cidr_addr_broadcast(const CIDR *addr)
{
	int i, j;
	CIDR *toret;

	/* Quick check */
	if(addr==NULL)
	{
		errno = EFAULT;
		return(NULL);
	}
	
	toret = cidr_alloc();
	if(toret==NULL)
		return(NULL); /* Preserve errno */
	toret->proto = addr->proto;
	
	/* The netmask is the same */
	memcpy(toret->mask, addr->mask, (16 * sizeof(toret->mask[0])) );

	/* Copy all the network bits */
	for(i=0 ; i<=15 ; i++)
	{
		for(j=7 ; j>=0 ; j--)
		{
			/* If we're into host bits, hop out */
			if( (addr->mask[i] & 1<<j) == 0)
				goto post;

			/* Else, copy this network bit */
			toret->addr[i] |= (addr->addr[i] & 1<<j);

		}
	}

post:
	/* Now set the remaining bits to 1 */
	for( /* i */ ; i<=15 ; i++)
	{
		for( /* j */ ; j>=0 ; j--)
			toret->addr[i] |= (1<<j);

		j=7;
	}

	/* And send it back */
	return(toret);
}


/* Get the first host in a CIDR block */
CIDR *
cidr_addr_hostmin(const CIDR *addr)
{
	CIDR *toret;

	toret = cidr_addr_network(addr);
	if(toret==NULL)
		return(NULL); /* Preserve errno */
	
	toret->addr[15] |= 1;

	return(toret);
}


/* Get the last host in a CIDR block */
CIDR *
cidr_addr_hostmax(const CIDR *addr)
{
	CIDR *toret;

	toret = cidr_addr_broadcast(addr);
	if(toret==NULL)
		return(NULL); /* Preserve errno */
	
	toret->addr[15] &= 0xfe;

	return(toret);
}
