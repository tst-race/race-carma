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
#include <kestrel/ta2_plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/ta2_plugin_t.hpp>
//

#include <cstdint>
#include <string>
#include <utility>

#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>

#include <PluginResponse.h>
#include <SdkResponse.h>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

PluginResponse ta2_plugin_t::flushChannel(RaceHandle handle,
                                          std::string channelGid,
                                          std::uint64_t batchId) {
  tracing_event_t SST_TEV_DEF(tev);
  SST_TEV_ADD(tev,
              "plugin_function_input",
              this->i_flushChannel(handle, channelGid, batchId));
  return this->template inner_call<func_flushChannel, PLUGIN_ERROR>(
      tev,
      [&](tracing_event_t & tev) {
        this->inner_flushChannel(SST_TEV_ARG(tev),
                                 race_handle_t(std::move(handle)),
                                 channel_id_t(std::move(channelGid)),
                                 batchId);
        SST_TEV_ADD(tev,
                    "plugin_function_output",
                    this->o_flushChannel(handle, channelGid, batchId));
      });
}

} // namespace kestrel
