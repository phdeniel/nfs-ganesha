/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright CEA/DAM/DIF  (2008)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * ---------------------------------------
 */

/**
 * \file    test_cache_inode_lookup.c
 * \author  $Author: deniel $
 * \date    $Date: 2005/11/28 17:02:28 $
 * \version $Revision: 1.8 $
 * \brief   Program for testing cache_inode / lookup functions.
 *
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "fsal.h"
#include "cache_inode.h"
#include "LRU_List.h"
#include "log_macros.h"
#include "err_fsal.h"
#include "err_cache_inode.h"
#include "stuff_alloc.h"
#include <unistd.h>             /* for using gethostname */
#include <stdlib.h>             /* for using exit */
#include <strings.h>
#include <sys/types.h>

#define HPSS_SSM "hpss_ssm"
#define HPSS_KEYTAB "/krb5/hpssserver.keytab"

int lru_entry_to_str(LRU_data_t data, char *str)
{
  cache_entry_t *pentry = NULL;

  pentry = (cache_entry_t *) data.pdata;

  return sprintf(str, "Pentry: Addr %p, state=%d", pentry,
                 pentry->internal_md.valid_state);
}                               /* lru_entry_to_str */

int lru_clean_entry(LRU_entry_t * entry, void *adddata)
{
  return 0;
}                               /* lru_clean_entry */

/**
 *
 * main: test main function.
 *
 * test main function.
 *
 * @param argc [IN] number of command line arguments.
 * @param argv [IN] arguments set on the command line.
 *
 * @return an integer sent back to the calling shell.
 *
 */

main(int argc, char *argv[])
{
  char localmachine[256];

  cache_inode_client_t client;
  LRU_parameter_t lru_param;
  LRU_status_t lru_status;
  cache_inode_fsal_data_t fsdata;

  fsal_status_t status;
  fsal_parameter_t init_param;
  fsal_name_t name;
  fsal_path_t path;
  fsal_attrib_mask_t mask;
  fsal_path_t pathroot;
  fsal_attrib_list_t attribs;
  fsal_handle_t root_handle;

  cache_inode_endofdir_t eod_met;
  cache_inode_dir_entry_t dirent_array[100];
  cache_inode_dir_entry_t dirent_array_loop[5];
  unsigned int nbfound;

  unsigned int begin_cookie = 0;
  hash_buffer_t key, value;

  uid_t uid;
  fsal_cred_t cred;

  cache_inode_status_t cache_status;
  cache_inode_parameter_t cache_param;
  cache_inode_client_parameter_t cache_client_param;

  hash_table_t *ht = NULL;
  fsal_attrib_list_t attrlookup;
  cache_entry_t *cache_entry_root = NULL;
  cache_entry_t *cache_entry_lookup = NULL;
  cache_entry_t *cache_entry_lookup2 = NULL;
  cache_entry_t *cache_entry_lookup3 = NULL;
  cache_entry_t *cache_entry_lookup4 = NULL;
  cache_entry_t *cache_entry_lookup5 = NULL;
  cache_entry_t *cache_entry_lookup6 = NULL;
  cache_entry_t *cache_entry_dircont = NULL;

  cache_inode_gc_policy_t gcpol;

  char *configfile = argv[1];
  int i = 0;
  int rc = 0;

  /* Init the Buddy System allocation */
  if((rc = BuddyInit(NULL)) != BUDDY_SUCCESS)
    {
      LogTest("Error while initializing Buddy system allocator");
      exit(1);
    }

  /* init debug */

  SetNamePgm("test_cache_inode");
  SetDefaultLogging("TEST");
  SetNameFunction("main");
  InitLogging();

#if defined( _USE_GHOSTFS )
  if(argc != 2)
    {
      LogTest("Please set the configuration file as parameter");
      exit(1);
    }
#endif

  /* Obtention du nom de la machine */
  if(gethostname(localmachine, sizeof(localmachine)) != 0)
    {
      LogError(COMPONENT_STDOUT, ERR_SYS, ERR_GETHOSTNAME, errno);
      exit(1);
    }
  else
    SetNameHost(localmachine);

  AddFamilyError(ERR_FSAL, "FSAL related Errors", tab_errstatus_FSAL);
  AddFamilyError(ERR_CACHE_INODE, "FSAL related Errors", tab_errstatus_cache_inode);

  LogTest( "Starting the test");
  LogTest( "-----------------");

#if defined( _USE_GHOSTFS )
  if(FSAL_IS_ERROR(status = FSAL_str2path(configfile,
                                          strlen(configfile) + 1,
                                          &(init_param.fs_specific_info.
                                            definition_file))))
    {
      LogError(COMPONENT_STDOUT, ERR_FSAL, status.major, status.minor);
    }
#elif defined( _USE_HPSS )

  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, Flags);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, DebugValue);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, TransferType);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, NumRetries);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, BusyDelay);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, BusyRetries);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, TotalDelay);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, GKTotalDelay);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, LimitedRetries);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, MaxConnections);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, ReuseDataConnections);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, UsePortRange);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, RetryStageInp);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, DMAPWriteUpdates);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, ServerName);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, DescName);

  init_param.fs_specific_info.behaviors.PrincipalName = FSAL_INIT_FORCE_VALUE;
  strncpy(init_param.fs_specific_info.hpss_config.PrincipalName,
          HPSS_SSM, HPSS_MAX_PRINCIPAL_NAME);

  init_param.fs_specific_info.behaviors.KeytabPath = FSAL_INIT_FORCE_VALUE;
  strncpy(init_param.fs_specific_info.hpss_config.KeytabPath,
          HPSS_KEYTAB, HPSS_MAX_PATH_NAME);

  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, DebugPath);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, HostName);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, RegistrySiteName);

#endif
  /* 2-common info (default) */
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, maxfilesize);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, maxlink);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, maxnamelen);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, maxpathlen);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, no_trunc);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, chown_restricted);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, case_insensitive);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, case_preserving);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, fh_expire_type);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, link_support);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, symlink_support);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, named_attr);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, unique_handles);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, lease_time);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, acl_support);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, cansettime);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, homogenous);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, maxread);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, maxwrite);

  /* Init */
  if(FSAL_IS_ERROR(status = FSAL_Init(&init_param)))
    {
      LogError(COMPONENT_STDOUT, ERR_FSAL, status.major, status.minor);
    }

  /* getting creds */
  uid = getuid();

  if(FSAL_IS_ERROR(status = FSAL_GetUserCred(uid, NULL, &cred)))
    {
      LogError(COMPONENT_STDOUT, ERR_FSAL, status.major, status.minor);
    }

  /* Init of the cache inode module */
  cache_param.hparam.index_size = 31;
  cache_param.hparam.alphabet_length = 10;      /* Buffer seen as a decimal polynom */
  cache_param.hparam.nb_node_prealloc = 100;
  cache_param.hparam.hash_func_key = cache_inode_fsal_hash_func;
  cache_param.hparam.hash_func_rbt = cache_inode_fsal_rbt_func;
  cache_param.hparam.hash_func_both = NULL ; /* BUGAZOMEU */
  cache_param.hparam.compare_key = cache_inode_compare_key_fsal;
  cache_param.hparam.key_to_str = display_key;
  cache_param.hparam.val_to_str = display_value;

  if((ht = cache_inode_init(cache_param, &cache_status)) == NULL)
    {
      LogTest( "Error %d while init hash ", cache_status);
    }
  else
    LogTest( "Hash Table address = %p", ht);

  /* We need a cache_client to acces the cache */
  cache_client_param.attrmask =
      FSAL_ATTRS_MANDATORY | FSAL_ATTR_MTIME | FSAL_ATTR_CTIME | FSAL_ATTR_ATIME;
  cache_client_param.nb_prealloc_entry = 1000;
  cache_client_param.nb_pre_dir_data = 200;
  cache_client_param.nb_pre_parent = 1200;
  cache_client_param.nb_pre_state_v4 = 100;

  cache_client_param.lru_param.nb_entry_prealloc = 1000;
  cache_client_param.lru_param.entry_to_str = lru_entry_to_str;
  cache_client_param.lru_param.clean_entry = lru_clean_entry;

  cache_client_param.grace_period_attr   = 0;
  cache_client_param.grace_period_link   = 0;
  cache_client_param.grace_period_dirent = 0;
  cache_client_param.expire_type_attr    = CACHE_INODE_EXPIRE_NEVER;
  cache_client_param.expire_type_link    = CACHE_INODE_EXPIRE_NEVER;
  cache_client_param.expire_type_dirent  = CACHE_INODE_EXPIRE_NEVER;

  /* Init the cache_inode client */
  if(cache_inode_client_init(&client, &cache_client_param, 0) != 0)
    exit(1);

  /* Getting the root of the FS */
  if((FSAL_IS_ERROR(status = FSAL_str2path("/", 2, &pathroot))))
    {
      LogError(COMPONENT_STDOUT, ERR_FSAL, status.major, status.minor);
      exit(1);
    }

  attribs.asked_attributes = cache_client_param.attrmask;
  if((FSAL_IS_ERROR(status = FSAL_lookupPath(pathroot, &cred, &root_handle, &attribs))))
    {
      LogError(COMPONENT_STDOUT, ERR_FSAL, status.major, status.minor);
      exit(1);
    }
  fsdata.cookie = 0;
  fsdata.handle = root_handle;

  /* Cache the root of the FS */
  if((cache_entry_root =
      cache_inode_make_root(&fsdata, 1, ht, &client, &cred, &cache_status)) == NULL)
    {
      LogTest( "Error: can't init fs's root");
      exit(1);
    }

  /* A lookup in the root fsal */
  if((FSAL_IS_ERROR(status = FSAL_str2name("cea", 10, &name))))
    {
      LogError(COMPONENT_STDOUT, ERR_FSAL, status.major, status.minor);
      exit(1);
    }

  if((cache_entry_lookup = cache_inode_lookup(cache_entry_root,
                                              name,
                                              &attrlookup,
                                              ht, &client, &cred, &cache_status)) == NULL)
    {
      LogTest( "Error: can't lookup");
      exit(1);
    }

  /* Lookup a second time (entry should now be cached) */
  if((cache_entry_lookup2 = cache_inode_lookup(cache_entry_root,
                                               name,
                                               &attrlookup,
                                               ht,
                                               &client, &cred, &cache_status)) == NULL)
    {
      LogTest( "Error: can't lookup");
      exit(1);
    }

  if(cache_entry_lookup2 != cache_entry_lookup)
    {
      LogTest("Error: lookup results should be the same");
      exit(1);
    }

  /* A lookup in the root fsal */
  if((FSAL_IS_ERROR(status = FSAL_str2name("log", 10, &name))))
    {
      LogError(COMPONENT_STDOUT, ERR_FSAL, status.major, status.minor);
      exit(1);
    }

  if((cache_entry_lookup3 = cache_inode_lookup(cache_entry_root,
                                               name,
                                               &attrlookup,
                                               ht,
                                               &client, &cred, &cache_status)) == NULL)
    {
      LogTest( "Error: can't lookup");
      exit(1);
    }

  if((cache_entry_lookup4 = cache_inode_lookup(cache_entry_root,
                                               name,
                                               &attrlookup,
                                               ht,
                                               &client, &cred, &cache_status)) == NULL)
    {
      LogTest( "Error: can't lookup");
      exit(1);
    }

  if(cache_entry_lookup3 != cache_entry_lookup4)
    {
      LogTest("Error: lookup results should be the same");
      exit(1);
    }

  /* A lookup in the root fsal */
  if((FSAL_IS_ERROR(status = FSAL_str2name("cea", 10, &name))))
    {
      LogError(COMPONENT_STDOUT, ERR_FSAL, status.major, status.minor);
      exit(1);
    }

  if((cache_entry_lookup = cache_inode_lookup(cache_entry_root,
                                              name,
                                              &attrlookup,
                                              ht, &client, &cred, &cache_status)) == NULL)
    {
      LogTest( "Error: can't lookup");
      exit(1);
    }

  /* A lookup in the root fsal */
  if((FSAL_IS_ERROR(status = FSAL_str2name("log", 10, &name))))
    {
      LogError(COMPONENT_STDOUT, ERR_FSAL, status.major, status.minor);
      exit(1);
    }

  if((cache_entry_lookup = cache_inode_lookup(cache_entry_root,
                                              name,
                                              &attrlookup,
                                              ht, &client, &cred, &cache_status)) == NULL)
    {
      LogTest( "Error: can't lookup");
      exit(1);
    }

  LogTest( "---------------------------------");

  /* The end of all the tests */
  LogTest( "All tests exited successfully");

  exit(0);
}                               /* main */
