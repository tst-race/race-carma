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

#ifndef KESTREL_EASY_TA2_PLUGIN_T_ACTIVATECHANNEL_HPP
#define KESTREL_EASY_TA2_PLUGIN_T_ACTIVATECHANNEL_HPP

#include <string>
#include <utility>

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_DEF.hpp>

#include <ChannelStatus.h>
#include <PluginResponse.h>
#include <SdkResponse.h>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/easy_ta2_plugin_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

template<class Plugin, class Types>
PluginResponse easy_ta2_plugin_t<Plugin, Types>::activateChannel(
    RaceHandle const handle,
    std::string const channelGid,
    std::string const roleName) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev,
              "plugin_function_input",
              this->i_activateChannel(handle, channelGid, roleName));
  return this->template inner_call<func_activateChannel, PLUGIN_ERROR>(
      tev,
      [&](tracing_event_t & tev) {
        this->expect_active(false);
        this->verify_channel_id(channelGid);
        this->inner_activateChannel(SST_TEV_ARG(tev));
        this->mutable_properties().channelStatus = CHANNEL_AVAILABLE;
        try {
          this->sdk().onChannelStatusChanged(SST_TEV_ARG(tev),
                                             handle,
                                             channelGid,
                                             properties().channelStatus,
                                             properties(),
                                             0);
        } catch (...) {
          this->mutable_properties().channelStatus = CHANNEL_ENABLED;
          throw;
        }
        SST_TEV_ADD(
            tev,
            "plugin_function_output",
            this->o_activateChannel(handle, channelGid, roleName));
      });
}

} // namespace kestrel

#endif // #ifndef KESTREL_EASY_TA2_PLUGIN_T_ACTIVATECHANNEL_HPP
