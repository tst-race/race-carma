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
#include <kestrel/channel_t.hpp>
// Include twice to test idempotence.
#include <kestrel/channel_t.hpp>
//

#include <stdexcept>

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/c_quote.hpp>

#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

race_handle_t channel_t::activate(tracing_event_t tev) {
  SST_TEV_ADD(tev);
  try {
    if (properties().roles.empty()) {
      throw std::runtime_error("Channel " + sst::c_quote(id().string())
                               + " does not have any roles.");
    }
    return race_handle_t(
        sdk_.activateChannel(SST_TEV_ARG(tev),
                             id().string(),
                             properties().roles.front().roleName,
                             0)
            .handle);
  }
  SST_TEV_RETHROW(tev);
}

} // namespace kestrel
