/**
 *    Copyright (C) 2018-present MongoDB, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the Server Side Public License, version 1,
 *    as published by MongoDB, Inc.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    Server Side Public License for more details.
 *
 *    You should have received a copy of the Server Side Public License
 *    along with this program. If not, see
 *    <http://www.mongodb.com/licensing/server-side-public-license>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the Server Side Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#pragma once

#include <boost/optional.hpp>
#include <string>
#include <vector>

#include "mongo/s/catalog/type_chunk.h"
#include "mongo/s/chunk_version.h"
#include "mongo/s/client/shard.h"

namespace mongo {

class NamespaceString;
class OperationContext;
class ShardKeyPattern;
class ShardRegistry;
template <typename T>
class StatusWith;

/**
 * Set of functions used to introspect and manipulate the state of individual shards.
 */
namespace shardutil {

static constexpr size_t kMaxSplitPoints = 8192;

/**
 * Executes the listDatabases command against the specified shard and obtains the total data
 * size across all databases in bytes (essentially, the totalSize field).
 *
 * Returns OK with the total size or an error. Known errors are:
 *  ShardNotFound if shard by that id is not available on the registry
 *  NoSuchKey if the total shard size could not be retrieved
 */
StatusWith<long long> retrieveTotalShardSize(OperationContext* opCtx, const ShardId& shardId);

/**
 * Executes the dataSize command against the specified shard and obtains the total data
 * size for the collection in bytes (essentially, the dataSize field).
 *
 * Returns OK with the total size in bytes or an error. Known errors are:
 *  ShardNotFound if shard by that id is not available on the registry
 *  NoSuchKey if the total shard size could not be retrieved
 */
StatusWith<long long> retrieveCollectionShardSize(OperationContext* opCtx,
                                                  const ShardId& shardId,
                                                  NamespaceString const& ns,
                                                  bool estimate = true);

/**
 * Ask the specified shard to figure out the split points for a given chunk.
 *
 * - shardId: the shard id to query.
 * - nss: namespace, which owns the chunk.
 * - shardKeyPattern: the shard key which corresponds to this sharded namespace.
 * - chunkRange: bounds of the chunk to search for split points on.
 * - chunkSizeBytes: chunk size to target in bytes.
 * - limit: limits the number of split points to search. Unspecified means no limit
 */
StatusWith<std::vector<BSONObj>> selectChunkSplitPoints(OperationContext* opCtx,
                                                        const ShardId& shardId,
                                                        const NamespaceString& nss,
                                                        const ShardKeyPattern& shardKeyPattern,
                                                        const ChunkRange& chunkRange,
                                                        long long chunkSizeBytes,
                                                        boost::optional<int> limit = boost::none);

/**
 * Asks the specified shard to split the chunk described by min/maxKey into the respective split
 * points. If split was successful and the shard indicated that one of the resulting chunks should
 * be moved off the currently owning shard, the return value will contain the bounds of this chunk.
 *
 * shardId The shard, which currently owns the chunk.
 * nss Namespace, which owns the chunk.
 * shardKeyPattern The shard key which corresponds to this sharded namespace.
 * collectionVersion The expected collection version when doing the split.
 * chunkRange Bounds of the chunk to be split.
 * splitPoints The set of points at which the chunk should be split.
 */
StatusWith<boost::optional<ChunkRange>> splitChunkAtMultiplePoints(
    OperationContext* opCtx,
    const ShardId& shardId,
    const NamespaceString& nss,
    const ShardKeyPattern& shardKeyPattern,
    const OID& epoch,
    const Timestamp& timestamp,
    ChunkVersion shardVersion,
    const ChunkRange& chunkRange,
    const std::vector<BSONObj>& splitPoints);

}  // namespace shardutil
}  // namespace mongo
