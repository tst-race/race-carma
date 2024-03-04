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

#ifndef KESTREL_CREATE_LINK_FROM_ADDRESS_CALL_T_HPP
#define KESTREL_CREATE_LINK_FROM_ADDRESS_CALL_T_HPP

#include <unordered_set>

#include <sst/catalog/mono_time_ns.hpp>

#include <kestrel/basic_call_t.hpp>
#include <kestrel/channel_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_handle_t.hpp>

namespace kestrel {

class create_link_from_address_call_t final : public basic_call_t {
  channel_t & channel_;
  std::unordered_set<psn_t> personas_;

public:
  create_link_from_address_call_t(
      create_link_from_address_call_t const &) = delete;
  create_link_from_address_call_t(create_link_from_address_call_t &&) =
      delete;
  create_link_from_address_call_t &
  operator=(create_link_from_address_call_t const &) = delete;
  create_link_from_address_call_t &
  operator=(create_link_from_address_call_t &&) = delete;
  ~create_link_from_address_call_t() = default;

  explicit create_link_from_address_call_t(
      race_handle_t const & handle,
      channel_t & channel,
      std::unordered_set<psn_t> const & personas);

  explicit create_link_from_address_call_t(race_handle_t const & handle,
                                           channel_t & channel,
                                           psn_t const & persona);

  channel_t & channel() const;

  std::unordered_set<psn_t> const & personas() const;
};

} // namespace kestrel

#endif // #ifndef KESTREL_CREATE_LINK_FROM_ADDRESS_CALL_T_HPP
