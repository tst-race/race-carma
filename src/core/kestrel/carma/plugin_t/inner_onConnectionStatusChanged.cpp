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

#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>

#include <LinkProperties.h>

#include <kestrel/CARMA_XLOG_WARN.hpp>
#include <kestrel/channel_id_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/connection_status_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

void plugin_t::inner_onConnectionStatusChanged(
    tracing_event_t tev,
    race_handle_t const & handle,
    connection_id_t const & connection_id,
    connection_status_t const status,
    link_id_t const & link_id,
    LinkProperties const & link_properties) {
  SST_TEV_TOP(tev);

  if (bootstrappee_step_4(SST_TEV_ARG(tev),
                          handle,
                          connection_id,
                          status,
                          link_id,
                          link_properties)) {
    return;
  }

  switch (status) {

    case connection_status_t::open(): {

      auto const call_it = open_connection_calls_.find(handle);
      auto const link_it = links_.find(link_id);
      if (link_it != links_.end()) {
        auto & link = link_it->second;
        link.properties(link_properties);
        if (link.connections().find(connection_id)
            == link.connections().end()) {
          link.connections().emplace(
              connection_id,
              connection_t(SST_TEV_ARG(tev), sdk_, connection_id));
        }
      } else {
        try {
          sdk_.closeConnection(SST_TEV_ARG(tev),
                               connection_id.string(),
                               0);
        } catch (...) {
        }
      }
      if (call_it != open_connection_calls_.end()) {
        open_connection_calls_.erase(call_it);
      }

      //----------------------------------------------------------------

      {
        auto const it = prepareToBootstrap_infos_1_.find(handle);
        if (it != prepareToBootstrap_infos_1_.end()) {
          prepareToBootstrap_info_1_t & info1 = it->second;
          if (!info1.garbage) {
            auto const r = prepareToBootstrap_infos_2_.emplace(
                std::piecewise_construct,
                std::make_tuple(guid_t::generate()),
                std::make_tuple());
            if (r.second) {
              prepareToBootstrap_info_2_t & info2 = r.first->second;
              info2.connection_id = connection_id;
              config().prepare_to_bootstrap(
                  SST_TEV_ARG(tev),
                  info1.configPath,
                  channel_id_t(link_properties.channelGid),
                  link_address_t(link_properties.linkAddress));
              // TODO: Remove this copying of config.json once
              //       old_config is gone. The void statement is here
              //       just to trigger a compilation error once it's
              //       gone.
              (void)this->old_config_;
              sdk().writeFile(
                  SST_TEV_ARG(tev),
                  info1.configPath + "/config.json",
                  sdk().readFile(SST_TEV_ARG(tev), "config.json"));
              std::vector<std::string> ta2Channels;
              for (auto const & kv : channels_) {
                ta2Channels.emplace_back(kv.first.string());
              }
              sdk().bootstrapDevice(SST_TEV_ARG(tev),
                                    info1.handle.value(),
                                    std::move(ta2Channels));
              info2.garbage = false;
              // TODO: No longer leaking the connection id at this
              //       point, as info2 is locked in. No need to close
              //       the connection during error cleanup for this
              //       function once we reach this point.
            } else {
              CARMA_XLOG_WARN(
                  sdk(),
                  0,
                  SST_TEV_ARG(tev, "event", "duplicate_guid"));
            }
            info1.garbage = true;
          }
          prepareToBootstrap_infos_1_.erase(it);
        }
      }

      //----------------------------------------------------------------

    } break;

    case connection_status_t::closed():
    case connection_status_t::init_failed(): {

      auto const link_it = links_.find(link_id);
      if (link_it != links_.end()) {
        auto & link = link_it->second;
        link.connections().erase(connection_id);
        ensure_connection(SST_TEV_ARG(tev), link);
      }

    } break;

    default: {
    } break;
  }
  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
