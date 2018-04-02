/**
 * (C) Copyright 2016 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * GOVERNMENT LICENSE RIGHTS-OPEN SOURCE SOFTWARE
 * The Government's rights to use, modify, reproduce, release, perform, display,
 * or disclose this software are subject to the terms of the Apache License as
 * provided in Contract No. B609815.
 * Any reproduction of computer software, computer software documentation, or
 * portions thereof marked with this legend must also reproduce the markings.
 */
/**
 * ds_pool: Pool Server API
 */

#ifndef __DAOS_SRV_POOL_H__
#define __DAOS_SRV_POOL_H__

#include <abt.h>
#include <daos/common.h>
#include <daos/lru.h>
#include <daos/pool_map.h>
#include <daos/rpc.h>

/*
 * Pool object
 *
 * Caches per-pool information, such as the pool map.
 */
struct ds_pool {
	struct daos_llink	sp_entry;
	uuid_t			sp_uuid;
	ABT_rwlock		sp_lock;
	struct pool_map	       *sp_map;
	uint32_t		sp_map_version;	/* temporary */
	crt_group_t	       *sp_group;
	struct ds_iv_ns		*sp_iv_ns;
};

struct ds_pool_create_arg {
	struct pool_map	       *pca_map;
	uint32_t		pca_map_version;
	bool			pca_need_group;
};

int ds_pool_lookup_create(const uuid_t uuid, struct ds_pool_create_arg *arg,
			  struct ds_pool **pool);
struct ds_pool *ds_pool_lookup(const uuid_t uuid);
void ds_pool_put(struct ds_pool *pool);

/*
 * Pool handle object
 *
 * Stores per-handle information, such as the capabilities. References the pool
 * object.
 */
struct ds_pool_hdl {
	d_list_t		sph_entry;
	uuid_t			sph_uuid;	/* of the pool handle */
	uint64_t		sph_capas;
	struct ds_pool	       *sph_pool;
	int			sph_ref;
};

struct ds_pool_hdl *ds_pool_hdl_lookup(const uuid_t uuid);
void ds_pool_hdl_put(struct ds_pool_hdl *hdl);

/*
 * Per-thread pool object
 *
 * Stores per-thread, per-pool information, such as the vos pool handle. And,
 * caches per-pool information, such as the pool map version, so that DAOS
 * object I/Os do not need to access global, parent ds_pool objects.
 */
struct ds_pool_child {
	d_list_t	spc_list;
	daos_handle_t	spc_hdl;
	uuid_t		spc_uuid;
	uint32_t	spc_map_version;
	int		spc_ref;
};

struct ds_pool_child *ds_pool_child_lookup(const uuid_t uuid);
void ds_pool_child_put(struct ds_pool_child *child);

int
ds_pool_bcast_create(crt_context_t ctx, struct ds_pool *pool,
		     enum daos_module_id module, crt_opcode_t opcode,
		     crt_rpc_t **rpc, crt_bulk_t bulk_hdl,
		     d_rank_list_t *excluded_list);
int
ds_pool_map_buf_get(const uuid_t uuid, d_iov_t *iov, uint32_t *map_ver);

int ds_pool_tgt_exclude_out(uuid_t pool_uuid, d_rank_list_t *tgts,
			    d_rank_list_t *tgts_out);
int ds_pool_tgt_exclude(uuid_t pool_uuid, d_rank_list_t *tgts,
			d_rank_list_t *tgts_out);

int ds_pool_tgt_map_update(struct ds_pool *pool, struct pool_buf *buf,
			   unsigned int map_version);

/*
 * TODO: Make the following internal functions of ds_pool after merging in
 * mgmt.
 */

int ds_pool_create(const uuid_t pool_uuid, const char *path,
		   uuid_t target_uuid);

int ds_pool_svc_create(const uuid_t pool_uuid, unsigned int uid,
		       unsigned int gid, unsigned int mode, int ntargets,
		       uuid_t target_uuids[], const char *group,
		       const d_rank_list_t *target_addrs, int ndomains,
		       const int *domains, d_rank_list_t *svc_addrs);
int ds_pool_svc_destroy(const uuid_t pool_uuid);

/*
 * Called by dmg on the pool service leader to list all pool handles of a pool.
 * Upon successful completion, "buf" returns an array of handle UUIDs if its
 * large enough, while "size" returns the size of all the handle UUIDs assuming
 * "buf" is large enough.
 */
int ds_pool_hdl_list(const uuid_t pool_uuid, uuid_t buf, size_t *size);

/*
 * Called by dmg on the pool service leader to evict one or all pool handles of
 * a pool. If "handle_uuid" is NULL, all pool handles of the pool are evicted.
 */
int ds_pool_hdl_evict(const uuid_t pool_uuid, const uuid_t handle_uuid);

typedef int (*obj_iter_cb_t)(uuid_t cont_uuid, daos_unit_oid_t oid, void *arg);
int ds_pool_obj_iter(uuid_t pool_uuid, obj_iter_cb_t callback, void *arg);

char *ds_pool_rdb_path(const uuid_t uuid, const uuid_t pool_uuid);
int ds_pool_svc_start(const uuid_t uuid);
void ds_pool_svc_stop(const uuid_t uuid);

struct cont_svc;
struct rsvc_hint;
int ds_pool_cont_svc_lookup_leader(const uuid_t pool_uuid,
				   struct cont_svc ***svcpp,
				   struct rsvc_hint *hint);
void ds_pool_cont_svc_put_leader(struct cont_svc **svcp);
uint64_t ds_pool_cont_svc_term(struct cont_svc **svcp);

struct rdb;
void ds_pool_set_hint(struct rdb *db, struct rsvc_hint *hint);

int
ds_pool_iv_ns_try_create(struct ds_pool *pool, unsigned int master_rank,
			 d_iov_t *iv_iov, unsigned int iv_ns_id);
#endif /* __DAOS_SRV_POOL_H__ */
