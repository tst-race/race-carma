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

#ifndef KESTREL_LOAD_LINK_CALL_T_HPP
#define KESTREL_LOAD_LINK_CALL_T_HPP

// Standard C++ headers
#include <unordered_set>

// SST headers
#include <sst/catalog/mono_time_ns.hpp>

// CARMA headers
#include <kestrel/basic_call_t.hpp>
#include <kestrel/channel_id_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_handle_t.hpp>

namespace kestrel {

class load_link_call_t final : public basic_call_t {
  channel_id_t channel_id_;
  std::unordered_set<psn_t> personas_;

public:
  load_link_call_t(load_link_call_t const &) = delete;
  load_link_call_t(load_link_call_t &&) = delete;
  load_link_call_t & operator=(load_link_call_t const &) = delete;
  load_link_call_t & operator=(load_link_call_t &&) = delete;
  ~load_link_call_t() = default;

  explicit load_link_call_t(
      race_handle_t const & handle,
      channel_id_t const & channel,
      std::unordered_set<psn_t> const & personas);

  explicit load_link_call_t(race_handle_t const & handle,
                            channel_id_t const & channel,
                            psn_t const & persona);

  channel_id_t const & channel_id() const;

  std::unordered_set<psn_t> const & personas() const;
};

} // namespace kestrel

#endif // #ifndef KESTREL_LOAD_LINK_CALL_T_HPP
