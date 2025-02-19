/**
 *    Copyright (C) 2020-present MongoDB, Inc.
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

#include "mongo/db/storage/storage_engine.h"

namespace mongo {
namespace startup_recovery {

/**
 * Recovers or repairs all databases from a previous shutdown. May throw a MustDowngrade error
 * if data files are incompatible with the current binary version.
 * The optional parameter `startupTimeElapsedBuilder` is for adding time elapsed of tasks done in
 * this function into one single builder that records the time elapsed during startup. Its default
 * value is nullptr because we only want to time this function when it is called during startup.
 */
void repairAndRecoverDatabases(OperationContext* opCtx,
                               StorageEngine::LastShutdownState lastShutdownState,
                               BSONObjBuilder* startupTimeElapsedBuilder = nullptr);

/**
 * Runs startup recovery after system startup, specifying whether to recover as a replica set
 * being started in standalone mode (no index build resumption).
 */
enum class StartupRecoveryMode { kAuto, kReplicaSetMember, kReplicaSetMemberInStandalone };

void runStartupRecoveryInMode(OperationContext* opCtx,
                              StorageEngine::LastShutdownState lastShutdownState,
                              StartupRecoveryMode mode);

}  // namespace startup_recovery
}  // namespace mongo
