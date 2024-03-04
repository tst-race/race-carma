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
#include <kestrel/carma/plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/plugin_t.hpp>
//

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <exception>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/checked_t.hpp>
#include <sst/catalog/copy_bytes.hpp>

#include <ChannelProperties.h>
#include <ChannelStatus.h>
#include <ClrMsg.h>
#include <ConnectionStatus.h>
#include <EncPkg.h>
#include <LinkProperties.h>
#include <LinkStatus.h>
#include <PluginConfig.h>
#include <PluginResponse.h>

#include <kestrel/carma/contains.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/channel_id_t.hpp>
#include <kestrel/channel_status_t.hpp>
#include <kestrel/channel_t.hpp>
#include <kestrel/config.h>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/connection_status_t.hpp>
#include <kestrel/connection_t.hpp>
#include <kestrel/encpkg_t.hpp>
#include <kestrel/goodbox_entry_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_address_packet_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/link_profile_t.hpp>
#include <kestrel/link_status_t.hpp>
#include <kestrel/package_status_t.hpp>
#include <kestrel/packet_packet_t.hpp>
#include <kestrel/plugin_state_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/tracing_exception_t.hpp>

namespace kestrel {
namespace carma {

void plugin_t::inner_onLinkStatusChanged(
    tracing_event_t tev,
    race_handle_t const & handle,
    link_id_t const & link_id,
    link_status_t const & link_status,
    LinkProperties const & link_properties) {
  SST_TEV_ADD(tev);
  try {

    process_message_context_t pmc(sdk(), *this);

    if (bootstrappee_step_3(SST_TEV_ARG(tev),
                            handle,
                            link_id,
                            link_status,
                            link_properties)) {
      return;
    }

    if (bootstrappee_step_6(SST_TEV_ARG(tev),
                            handle,
                            link_id,
                            link_status,
                            link_properties)) {
      return;
    }

    switch (link_status) {

      case link_status_t::created():
      case link_status_t::loaded(): {

        {
          auto const it = create_link_calls_.find(handle);
          if (it != create_link_calls_.end()) {
            mb_server_bootstrappee_link_created(
                SST_TEV_ARG(tev),
                pmc,
                handle,
                link_address_t(link_properties.linkAddress));
            auto const & call = it->second;
            add_link(SST_TEV_ARG(tev),
                     link_id,
                     link_properties,
                     call.personas());
            create_link_calls_.erase(it);
          }
        }

        {
          auto const it = create_link_from_address_calls_.find(handle);
          if (it != create_link_from_address_calls_.end()) {
            auto const & call = it->second;
            add_link(SST_TEV_ARG(tev),
                     link_id,
                     link_properties,
                     call.personas());
            create_link_from_address_calls_.erase(it);
          }
        }

        {
          auto const it = load_link_calls_.find(handle);
          if (it != load_link_calls_.end()) {
            auto const & call = it->second;
            add_link(SST_TEV_ARG(tev),
                     link_id,
                     link_properties,
                     call.personas());
            load_link_calls_.erase(it);
          }
        }

      } break;

      default: {
      } break;
    }
  }
  SST_TEV_RETHROW(tev);
}

} // namespace carma
} // namespace kestrel
