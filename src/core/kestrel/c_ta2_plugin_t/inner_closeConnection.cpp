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
#include <kestrel/c_ta2_plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/c_ta2_plugin_t.hpp>
//

#include <stdexcept>

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>

#include <kestrel/connection_id_t.hpp>
#include <kestrel/kestrel_char.h>
#include <kestrel/kestrel_ta2_plugin.h>
#include <kestrel/plugin_response_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

void c_ta2_plugin_t::inner_closeConnection(
    tracing_event_t tev,
    race_handle_t const & handle,
    connection_id_t const & connectionId) {
  SST_TEV_ADD(tev);
  try {
    plugin_response_t const r(
        plugin().closeConnection(plugin().plugin,
                                 handle.value(),
                                 reinterpret_cast<kestrel_char const *>(
                                     connectionId.value().c_str())));
    if (r != plugin_response_t::ok()) {
      throw std::runtime_error("closeConnection() failed");
    }
  }
  SST_TEV_RETHROW(tev);
}

} // namespace kestrel
