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
#include <stdexcept>
#include <string>

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <ConnectionStatus.h>

#include <kestrel/link_id_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/rabbitmq/connection_t.hpp>
#include <kestrel/rabbitmq/link_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

void plugin_t::inner_openConnection(tracing_event_t tev,
                                    race_handle_t const & handle,
                                    link_type_t const & linkType,
                                    link_id_t const & linkId,
                                    std::string const &,
                                    std::int32_t const sendTimeout) {
  SST_TEV_ADD(tev);
  try {
    collect_garbage(SST_TEV_ARG(tev));
    auto const link_kv = links().find(linkId);
    if (link_kv == links().cend()) {
      throw sdk().unknown_link_id(linkId);
    }
    link_t & link = link_kv->second;
    if (linkType.value() != link.properties().linkType) {
      throw std::runtime_error("The requested link type does not match "
                               "the actual link type.");
    }
    connection_t & connection =
        link.open_connection(SST_TEV_ARG(tev), sendTimeout);
    sdk().onConnectionStatusChanged(SST_TEV_ARG(tev),
                                    handle.value(),
                                    connection.id().value(),
                                    CONNECTION_OPEN,
                                    link.properties(),
                                    0);
    connection.commit();
  }
  SST_TEV_RETHROW(tev);
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
