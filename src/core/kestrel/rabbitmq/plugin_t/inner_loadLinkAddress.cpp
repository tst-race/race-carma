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

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>

#include <kestrel/link_address_t.hpp>
#include <kestrel/link_status_t.hpp>
#include <kestrel/rabbitmq/link_addrinfo_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

void plugin_t::inner_loadLinkAddress(tracing_event_t tev,
                                     race_handle_t const & handle,
                                     link_address_t const & linkAddress,
                                     link_status_t const & status) {
  SST_TEV_TOP(tev);
  easy_ta2_plugin_t::inner_loadLinkAddress(SST_TEV_ARG(tev),
                                           handle,
                                           linkAddress,
                                           status);
  servers().emplace(link_addrinfo_t(linkAddress));
  SST_TEV_BOT(tev);
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
