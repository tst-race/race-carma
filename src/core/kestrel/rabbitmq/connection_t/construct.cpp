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
#include <kestrel/rabbitmq/connection_t.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq/connection_t.hpp>
//

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <memory>

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/in_place.hpp>

#include <kestrel/CARMA_XLOG_DEBUG.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/rabbitmq/link_t.hpp>
#include <kestrel/rabbitmq/plugin_t.hpp>
#include <kestrel/rabbitmq/worker_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

connection_t::connection_t(tracing_event_t tev,
                           plugin_t & plugin,
                           lookup_it_t const lookup_it,
                           link_t & link,
                           connection_id_t const & id,
                           long const send_timeout)
    : plugin_(&plugin),
      lookup_it_(lookup_it),
      lookup_ptr_(&lookup_it->second),
      link_(&link),
      id_(sst::in_place, id),
      send_timeout_(send_timeout) {
  SST_TEV_ADD(tev);
  try {
    std::weak_ptr<worker_t> & w = plugin.workers()[link.addrinfo()];
    worker_ = w.lock();
    bool const create = worker_ == nullptr;
    if (create) {
      worker_ = std::make_shared<worker_t>(SST_TEV_ARG(tev), link);
      CARMA_XLOG_DEBUG(plugin.sdk(),
                       0,
                       SST_TEV_ARG(tev, "event", "worker_created"));
    } else {
      CARMA_XLOG_DEBUG(plugin.sdk(),
                       0,
                       SST_TEV_ARG(tev, "event", "worker_located"));
    }
    queue_ = &worker().add_connection(SST_TEV_ARG(tev), *this);
    if (create) {
      w = worker_;
    }
    ++plugin.link_connections_garbage_;
  }
  SST_TEV_RETHROW(tev);
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
