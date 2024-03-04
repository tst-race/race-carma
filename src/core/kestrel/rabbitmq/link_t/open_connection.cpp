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

#include <tuple>
#include <utility>

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/in_place.hpp>

#include <kestrel/connection_id_t.hpp>
#include <kestrel/rabbitmq/connection_t.hpp>
#include <kestrel/rabbitmq/plugin_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

connection_t & link_t::open_connection(tracing_event_t tev,
                                       long const send_timeout) {
  SST_TEV_ADD(tev);
  try {
    while (true) {
      connection_id_t const connection_id(
          plugin().sdk().generateConnectionId(SST_TEV_ARG(tev),
                                              id().value()));
      auto const r1 =
          plugin().connections().emplace(connection_id, nullptr);
      if (r1.second) {
        ++plugin().connections_garbage_;
        auto const lookup_it = r1.first;
        auto const r2 = connections().emplace(
            std::piecewise_construct,
            std::forward_as_tuple(connection_id),
            std::forward_as_tuple(sst::in_place,
                                  SST_TEV_ARG(tev),
                                  plugin(),
                                  lookup_it,
                                  *this,
                                  connection_id,
                                  send_timeout));
        if (r2.second) {
          connection_t & connection = *r2.first->second;
          return connection;
        }
      }
    }
  }
  SST_TEV_RETHROW(tev);
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
