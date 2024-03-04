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

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>

#include <kestrel/common_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/tracing_exception_t.hpp>

namespace kestrel {
namespace carma {

void plugin_t::inner_shutdown(tracing_event_t tev) noexcept {
  SST_TEV_ADD(tev);

  //--------------------------------------------------------------------
  // Stop and join the dispatcher
  //--------------------------------------------------------------------

  if (network_maintenance_thread_.joinable()) {
    CARMA_LOG_INFO(sdk_,
                   0,
                   SST_TEV_ARG(tev,
                               "event",
                               "stopping_network_maintenance_thread"));
    network_maintenance_stop_ = true;
    network_maintenance_cond_.notify_one();
    try {
      network_maintenance_thread_.join();
    } catch (...) {
    }
    CARMA_LOG_INFO(sdk_,
                   0,
                   SST_TEV_ARG(tev,
                               "event",
                               "stopped_network_maintenance_thread"));
  }

  //--------------------------------------------------------------------
  // Join any outstanding futures
  //--------------------------------------------------------------------
  //
  // Note that we need to have joined the dispatcher before joining
  // any outstanding futures, as the dispatcher itself may join the
  // futures. Doing otherwise would be undefined behavior (only one
  // thread may call get() on a std::future).
  //

  for (auto * const future : {
           &channel_update_cooldown_future_,
       }) {
    try {
      if (future->valid()) {
        static_cast<void>(future->get());
      }
    } catch (std::exception const & e) {
      CARMA_LOG_WARN(sdk_,
                     0,
                     SST_TEV_ARG(tev,
                                 "event",
                                 "future_joining_failed",
                                 "exception",
                                 e.what()));
    } catch (...) {
      CARMA_LOG_WARN(sdk_,
                     0,
                     SST_TEV_ARG(tev,
                                 "event",
                                 "future_joining_failed",
                                 "exception",
                                 nullptr));
    }
  }

  //--------------------------------------------------------------------
  // Close all connections
  //--------------------------------------------------------------------

  links_.clear();

  //--------------------------------------------------------------------
}

} // namespace carma
} // namespace kestrel
