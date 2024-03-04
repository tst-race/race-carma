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
#include <kestrel/create_link_from_address_call_t.hpp>
// Include twice to test idempotence.
#include <kestrel/create_link_from_address_call_t.hpp>
//

#include <unordered_set>

#include <sst/catalog/mono_time_ns.hpp>

#include <kestrel/basic_call_t.hpp>
#include <kestrel/channel_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_handle_t.hpp>

namespace kestrel {

create_link_from_address_call_t::create_link_from_address_call_t(
    race_handle_t const & handle,
    channel_t & channel,
    std::unordered_set<psn_t> const & personas)
    : basic_call_t(handle),
      channel_(channel),
      personas_(personas) {
}

create_link_from_address_call_t::create_link_from_address_call_t(
    race_handle_t const & handle,
    channel_t & channel,
    psn_t const & persona)
    : basic_call_t(handle),
      channel_(channel),
      personas_({persona}) {
}

channel_t & create_link_from_address_call_t::channel() const {
  return channel_;
}

std::unordered_set<psn_t> const &
create_link_from_address_call_t::personas() const {
  return personas_;
}

} // namespace kestrel
