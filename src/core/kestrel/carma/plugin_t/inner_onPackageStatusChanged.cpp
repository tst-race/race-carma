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

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>

#include <kestrel/goodbox_entry_t.hpp>
#include <kestrel/package_status_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

void plugin_t::inner_onPackageStatusChanged(
    tracing_event_t tev,
    race_handle_t const & handle,
    package_status_t const & status) {
  SST_TEV_TOP(tev);

  // TODO: "Sent" isn't as strong as "received". The former only means
  // the package (probably) made it onto the wire, whereas the latter
  // means the package (probably) made it over the wire to the other
  // side. We need a probabilistic scoring system.
  if (status == package_status_t::sent()
      || status == package_status_t::received()) {
    goodbox_.emplace(handle, goodbox_entry_t());
  }

  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
