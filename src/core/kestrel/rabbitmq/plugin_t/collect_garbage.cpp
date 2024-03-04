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

#include <memory>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/is_positive.hpp>
#include <sst/catalog/is_zero.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/rabbitmq/connection_t.hpp>
#include <kestrel/rabbitmq/link_t.hpp>
#include <kestrel/rabbitmq/worker_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

void plugin_t::collect_garbage(tracing_event_t tev) {
  SST_TEV_ADD(tev);
  try {

    if (sst::is_positive(link_connections_garbage_)) {
      for (auto & kv : links()) {
        link_t & link = kv.second;
        auto it = link.connections().begin();
        while (it != link.connections().end()) {
          sst::unique_ptr<connection_t> & connection = it->second;
          if (connection == nullptr || connection->garbage()) {
            connection = nullptr;
            it = link.connections().erase(it);
            SST_ASSERT(sst::is_positive(link_connections_garbage_));
            --link_connections_garbage_;
          } else {
            ++it;
          }
        }
      }
    }
    SST_ASSERT(sst::is_zero(link_connections_garbage_));

    if (sst::is_positive(connections_garbage_)) {
      auto it = connections().begin();
      while (it != connections().end()) {
        connection_t *& connection = it->second;
        if (connection == nullptr) {
          it = connections().erase(it);
          SST_ASSERT(sst::is_positive(connections_garbage_));
          --connections_garbage_;
        } else {
          ++it;
        }
      }
    }
    SST_ASSERT(sst::is_zero(connections_garbage_));

    if (sst::is_positive(workers_garbage_)) {
      auto it = workers().begin();
      while (it != workers().end()) {
        std::weak_ptr<worker_t> & worker = it->second;
        if (worker.expired()) {
          it = workers().erase(it);
          SST_ASSERT(sst::is_positive(workers_garbage_));
          --workers_garbage_;
        } else {
          ++it;
        }
      }
    }
    SST_ASSERT(sst::is_zero(workers_garbage_));

  } //
  SST_TEV_RETHROW(tev);
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
