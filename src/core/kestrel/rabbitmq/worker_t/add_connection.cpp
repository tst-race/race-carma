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
#include <kestrel/rabbitmq/worker_t.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq/worker_t.hpp>
//

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <future>
#include <mutex>
#include <tuple>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/to_string.hpp>

#include <kestrel/CARMA_XLOG_DEBUG.hpp>
#include <kestrel/rabbitmq/connection_t.hpp>
#include <kestrel/rabbitmq/link_addrinfo_t.hpp>
#include <kestrel/rabbitmq/link_t.hpp>
#include <kestrel/rabbitmq/plugin_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

worker_t::queue_t &
worker_t::add_connection(tracing_event_t tev,
                         connection_t const & connection) {
  SST_TEV_ADD(tev);
  try {
    std::lock_guard<std::mutex> const lock(queues_mutex());
    link_addrinfo_t const & addrinfo = connection.link().addrinfo();
    auto const r = queues().emplace(
        std::piecewise_construct,
        std::forward_as_tuple(addrinfo.queue()),
        std::forward_as_tuple(connection.id(), addrinfo.queue()));
    if (r.second) {
      ++queue_actions_;
    }
    queue_t & queue = r.first->second;
    if (!r.second) {
      if (queue.garbage) {
        queue = queue_t(connection.id(), addrinfo.queue());
      } else {
        queue.connection_ids->emplace_back(connection.id().string());
      }
    }
    if (!thread().valid()) {
      thread() = std::async(
          std::launch::async,
          [this, addrinfo](tracing_event_t tev) -> void {
            auto const thread_id = ++thread_id_;
            SST_TEV_ADD(tev,
                        "worker_thread_id",
                        sst::to_string(thread_id));
            CARMA_XLOG_DEBUG(
                plugin().sdk(),
                0,
                SST_TEV_ARG(tev, "event", "worker_thread_started"));
            thread_function(SST_TEV_ARG(tev), addrinfo);
            CARMA_XLOG_DEBUG(
                plugin().sdk(),
                0,
                SST_TEV_ARG(tev, "event", "worker_thread_stopped"));
          },
          SST_TEV_ARG(tev));
    }
    queue.garbage = false;
    ++queue.connection_count;
    if (queue.status == queue_t::status_t::destroy) {
      queue.status = queue_t::status_t::active;
      --queue_actions_;
    }
    return queue;
  }
  SST_TEV_RETHROW(tev);
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
