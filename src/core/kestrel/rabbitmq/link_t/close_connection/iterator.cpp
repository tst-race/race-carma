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
#include <kestrel/rabbitmq/link_t.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq/link_t.hpp>
//

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <ConnectionStatus.h>

#include <kestrel/rabbitmq/connection_t.hpp>
#include <kestrel/rabbitmq/plugin_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

link_t::connections_t::iterator
link_t::close_connection(tracing_event_t tev,
                         connections_t::iterator it,
                         race_handle_t const & handle) {
  SST_TEV_ADD(tev);
  try {
    SST_ASSERT(it != connections().end());
    sst::unique_ptr<connection_t> & connection = it->second;
    SST_ASSERT(connection != nullptr);
    plugin().sdk().onConnectionStatusChanged(SST_TEV_ARG(tev),
                                             handle.value(),
                                             connection->id().value(),
                                             CONNECTION_CLOSED,
                                             properties(),
                                             0);
    connection = nullptr;
    ++plugin().link_connections_garbage_;
    try {
      it = connections().erase(it);
      --plugin().link_connections_garbage_;
    } catch (...) {
      ++it;
    }
    return it;
  }
  SST_TEV_RETHROW(tev);
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
