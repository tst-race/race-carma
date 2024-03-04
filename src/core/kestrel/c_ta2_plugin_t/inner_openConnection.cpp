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

#include <cstdint>
#include <stdexcept>
#include <string>

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>

#include <kestrel/kestrel_char.h>
#include <kestrel/kestrel_ta2_plugin.h>
#include <kestrel/link_id_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/plugin_response_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

void c_ta2_plugin_t::inner_openConnection(tracing_event_t tev,
                                          race_handle_t const & handle,
                                          link_type_t const & linkType,
                                          link_id_t const & linkId,
                                          std::string const & linkHints,
                                          std::int32_t sendTimeout) {
  SST_TEV_ADD(tev);
  try {
    plugin_response_t const r(plugin().openConnection(
        plugin().plugin,
        handle.value(),
        static_cast<kestrel_LinkType>(linkType.value()),
        reinterpret_cast<kestrel_char const *>(linkId.value().c_str()),
        reinterpret_cast<kestrel_char const *>(linkHints.c_str()),
        sendTimeout));
    if (r != plugin_response_t::ok()) {
      throw std::runtime_error("openConnection() failed");
    }
  }
  SST_TEV_RETHROW(tev);
}

} // namespace kestrel
