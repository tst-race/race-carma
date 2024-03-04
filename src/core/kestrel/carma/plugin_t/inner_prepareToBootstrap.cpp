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

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>

#include <DeviceInfo.h>

#include <kestrel/CARMA_XLOG_WARN.hpp>
#include <kestrel/carma/config_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

void plugin_t::inner_prepareToBootstrap(tracing_event_t tev,
                                        race_handle_t const & handle,
                                        link_id_t const & linkId,
                                        std::string const & configPath,
                                        DeviceInfo const & deviceInfo) {
  SST_TEV_TOP(tev);
  auto const r = prepareToBootstrap_infos_1_.emplace(
      std::piecewise_construct,
      std::make_tuple(openConnection(SST_TEV_ARG(tev),
                                     linkId,
                                     link_type_t::recv())),
      std::make_tuple());
  if (r.second) {
    prepareToBootstrap_info_1_t & v = r.first->second;
    v.handle = handle;
    v.linkId = linkId;
    v.configPath = configPath;
    v.deviceInfo = deviceInfo;
    v.openConnection_handle = r.first->first;
    v.garbage = false;
  } else {
    CARMA_XLOG_WARN(sdk(),
                    0,
                    SST_TEV_ARG(tev, "event", "duplicate_race_handle"));
  }
  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
