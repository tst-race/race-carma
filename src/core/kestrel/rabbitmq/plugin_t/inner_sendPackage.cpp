//
// Copyright (C) 2019-2024 Stealth Software Technologies, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an "AS
// IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied. See the License for the specific language
// governing permissions and limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
//

// Include first to test independence.
#include <kestrel/rabbitmq/plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq/plugin_t.hpp>
//

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <cstdint>
#include <memory>
#include <mutex>
#include <utility>

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>

#include <EncPkg.h>

#include <kestrel/connection_id_t.hpp>
#include <kestrel/rabbitmq/connection_t.hpp>
#include <kestrel/rabbitmq/outbox_entry_t.hpp>
#include <kestrel/rabbitmq/worker_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

void plugin_t::inner_sendPackage(tracing_event_t tev,
                                 race_handle_t const & handle,
                                 connection_id_t const & connectionId,
                                 RawData && pkg,
                                 double const timeoutTimestamp,
                                 std::uint64_t) {
  SST_TEV_ADD(tev);
  try {
    collect_garbage(SST_TEV_ARG(tev));
    connection_t & connection = expect_connection(connectionId);
    worker_t & worker = connection.worker();
    connection.outbox().emplace_back(nullptr);
    std::shared_ptr<outbox_entry_t> & x = connection.outbox().back();
    std::shared_ptr<outbox_entry_t> y =
        std::make_shared<outbox_entry_t>(connection,
                                         handle,
                                         std::move(pkg),
                                         timeoutTimestamp);
    {
      std::lock_guard<std::mutex> const lock(worker.outbox_mutex());
      worker.outbox().emplace(y);
    }
    x = std::move(y);
  }
  SST_TEV_RETHROW(tev);
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
