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

#include <iterator>
#include <stdexcept>

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/rand_range.hpp>

#include <LinkStatus.h>

#include <kestrel/rabbitmq/link_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

void plugin_t::inner_createLink(tracing_event_t tev,
                                race_handle_t const & handle) {
  SST_TEV_ADD(tev);
  try {
    collect_garbage(SST_TEV_ARG(tev));
    if (servers().empty()) {
      throw std::runtime_error("No known RabbitMQ servers.");
    }
    auto server = servers().cbegin();
    std::advance(server, sst::rand_range(servers().size() - 1));
    links_iterator_t const it =
        open_link(SST_TEV_ARG(tev), server->generate_link());
    link_t & link = it->second;
    try {
      sdk().onLinkStatusChanged(SST_TEV_ARG(tev),
                                handle.value(),
                                link.id().value(),
                                LINK_CREATED,
                                link.properties(),
                                0);
    } catch (...) {
      this->links().erase(it);
      throw;
    }
  }
  SST_TEV_RETHROW(tev);
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
