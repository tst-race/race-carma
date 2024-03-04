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
#include <sst/catalog/crypto_rng.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/json/get_from_string.hpp>
#include <sst/catalog/json/get_to.hpp>
#include <sst/catalog/min.hpp>
#include <sst/catalog/str_cmp.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/type_max.hpp>
#include <sst/time.h>

#include <ChannelProperties.h>
#include <ChannelStatus.h>
#include <ClrMsg.h>
#include <ConnectionStatus.h>
#include <EncPkg.h>
#include <LinkProperties.h>
#include <LinkStatus.h>
#include <PluginConfig.h>
#include <PluginResponse.h>

#include <kestrel/CARMA_XLOG_WARN.hpp>
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
#include <kestrel/message_status_t.hpp>
#include <kestrel/open_connection_call_t.hpp>
#include <kestrel/outbox_entry_t.hpp>
#include <kestrel/package_status_t.hpp>
#include <kestrel/packet_packet_t.hpp>
#include <kestrel/pkc.hpp>
#include <kestrel/plugin_state_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/tracing_exception_t.hpp>

namespace kestrel {
namespace carma {

void plugin_t::inner_onChannelStatusChanged(
    tracing_event_t tev,
    race_handle_t const &,
    channel_id_t const & channelGid,
    channel_status_t const & status,
    ChannelProperties const & properties) {
  SST_TEV_ADD(tev);
  try {

    ChannelProperties props(properties);

    if (props.channelStatus != status.value()) {
      CARMA_XLOG_WARN(
          sdk(),
          0,
          SST_TEV_ARG(tev, "event", "inconsistent_channel_status"));
      props.channelStatus = status.value();
    }

    channel_t & channel = [&]() -> channel_t & {
      auto const it = channels_.find(channelGid);
      if (it == channels_.end()) {
        return channels_
            .emplace(
                std::piecewise_construct,
                std::forward_as_tuple(channelGid),
                std::forward_as_tuple(SST_TEV_ARG(tev), sdk_, props))
            .first->second;
      }
      channel_t & channel = it->second;
      channel.properties(props);
      return channel;
    }();

    if (bootstrappee_step_2(SST_TEV_ARG(tev), channel)) {
      return;
    }

    if (bootstrappee_step_5(SST_TEV_ARG(tev), channel)) {
      return;
    }

    switch (status) {
      case channel_status_t::available(): {
        for (auto const & link_profile : link_profiles_[channelGid]) {
          switch (link_profile.role()) {
            case link_role_t::creator(): {
              create_link_from_address(SST_TEV_ARG(tev),
                                       channel,
                                       link_profile);
            } break;
            case link_role_t::loader(): {
              load_link(SST_TEV_ARG(tev), channel.id(), link_profile);
            } break;
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
