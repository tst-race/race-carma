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
#include <kestrel/carma/local_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/local_config_t.hpp>
//

#include <kestrel/carma/phonebook_entry_t.hpp>

namespace kestrel {
namespace carma {

void local_config_t::clear_deducible() const {
  SST_ASSERT((!moved_from_));

  phonebook_entry_t::clear_deducible();

  if (global().is_rigid()) {
    mb_servers_.reset();
  }

  clear_mc_groups();
  clear_mc_leader();
  clear_next_layer();

  if (global().is_rigid()) {
    prev_layer_.reset();
  }

  rx_nodes_.reset();

  ticket_.reset();

  tx_nodes_.reset();

} //

} // namespace carma
} // namespace kestrel
