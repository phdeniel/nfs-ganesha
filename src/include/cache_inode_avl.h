/*
 * Copyright (C) 2010, Linux Box Corporation
 * All Rights Reserved
 *
 * Contributor: Matt Benjamin
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
 * -------------
 */


/**
 *
 * \file cache_inode_avl.h
 * \author Matt Benjamin
 * \brief Definitions supporting AVL dirent representation
 *
 * \section DESCRIPTION
 *
 * Definitions supporting AVL dirent representation.  The current design
 * represents dirents as a single AVL tree ordered by a collision-resistent
 * hash function (currently, Murmur3, which appears to be several times
 * faster than lookup3 on x86_64 architecture).  Quadratic probing is used
 * to emulate perfect hashing.  Worst case behavior is challenging to
 * reproduce.  Heuristic methods are used to detect worst-case scenarios and
 * fall back to tractable (e.g., lookup) algorthims.
 *
 */

#ifndef _CACHE_INODE_AVL_H
#define _CACHE_INODE_AVL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif                          /* HAVE_CONFIG_H */

#ifdef _SOLARIS
#include "solaris_port.h"
#endif                          /* _SOLARIS */

#include "log.h"
#include "cache_inode.h"
#include "avltree.h"

static inline int
avl_dirent_hk_cmpf(const struct avltree_node *lhs,
                   const struct avltree_node *rhs)
{
    cache_inode_dir_entry_t *lk, *rk;

    lk = avltree_container_of(lhs, cache_inode_dir_entry_t, node_hk);
    rk = avltree_container_of(rhs, cache_inode_dir_entry_t, node_hk);

    if (lk->hk.k < rk->hk.k)
	return (-1);

    if (lk->hk.k == rk->hk.k)
	return (0);

    return (1);
}

extern void cache_inode_avl_init(cache_entry_t *entry);
extern int cache_inode_avl_qp_insert(cache_entry_t *entry,
                                     cache_inode_dir_entry_t *v);
extern cache_inode_dir_entry_t *cache_inode_avl_lookup_k(
    cache_entry_t *entry,
    cache_inode_dir_entry_t *v);
extern cache_inode_dir_entry_t *cache_inode_avl_qp_lookup_s(
    cache_entry_t *entry,
    cache_inode_dir_entry_t *v,
    int maxj);
static inline void
cache_inode_avl_remove(cache_entry_t *entry,
                       cache_inode_dir_entry_t *v)
{
    avltree_remove(&v->node_hk, &entry->object.dir.avl);
}

#endif /* _CACHE_INODE_AVL_H */
