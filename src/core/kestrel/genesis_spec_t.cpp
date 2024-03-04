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
#include <kestrel/genesis_spec_t.hpp>
// Include twice to test idempotence.
#include <kestrel/genesis_spec_t.hpp>
//

#include <exception>
#include <utility>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/json/unknown_key.hpp>
#include <sst/catalog/perfect_lt.hpp>
#include <sst/catalog/to_string.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// num_clients
//----------------------------------------------------------------------

void genesis_spec_t::parse_num_clients(json_t & src,
                                       parse_state_t & state) {
  if (state.called_parse_num_clients_) {
    return;
  }
  state.called_parse_num_clients_ = true;
  sst::json::remove_to(src, num_clients_, "num_clients");
  try {
    if (!num_clients_) {
      throw sst::json::exception::missing_value();
    }
    if (sst::perfect_lt(*num_clients_, 1)) {
      throw sst::json::exception("Value must be at least 1");
    }
  } catch (sst::json::exception const & e) {
    std::throw_with_nested(e.add_key("num_clients"));
  }
}

void genesis_spec_t::unparse_num_clients(json_t & dst) const {
  SST_ASSERT((num_clients_));
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("num_clients")));
  dst["num_clients"] = sst::to_string(*num_clients_);
}

long genesis_spec_t::num_clients() const noexcept {
  return *num_clients_;
}

//----------------------------------------------------------------------
// num_servers
//----------------------------------------------------------------------

void genesis_spec_t::parse_num_servers(json_t & src,
                                       parse_state_t & state) {
  if (state.called_parse_num_servers_) {
    return;
  }
  state.called_parse_num_servers_ = true;
  sst::json::remove_to(src, num_servers_, "num_servers");
  try {
    if (!num_servers_) {
      throw sst::json::exception::missing_value();
    }
    if (sst::perfect_lt(*num_servers_, 7)) {
      throw sst::json::exception("Value must be at least 7");
    }
  } catch (sst::json::exception const & e) {
    std::throw_with_nested(e.add_key("num_servers"));
  }
}

void genesis_spec_t::unparse_num_servers(json_t & dst) const {
  SST_ASSERT((num_servers_));
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("num_servers")));
  dst["num_servers"] = sst::to_string(*num_servers_);
}

long genesis_spec_t::num_servers() const noexcept {
  return *num_servers_;
}

//----------------------------------------------------------------------
// JSON conversions
//----------------------------------------------------------------------

void from_json(json_t src, genesis_spec_t & dst) {

  genesis_spec_t tmp;
  genesis_spec_t::parse_state_t state;

  tmp.parse_num_clients(src, state);
  tmp.parse_num_servers(src, state);

  SST_ASSERT((state.called_parse_num_clients_));
  SST_ASSERT((state.called_parse_num_servers_));

  sst::json::unknown_key(src);

  dst = std::move(tmp);

} //

void to_json(json_t & dst, genesis_spec_t const & src) {

  json_t tmp = json_t::object();

  src.unparse_num_clients(tmp);
  src.unparse_num_servers(tmp);

  dst = std::move(tmp);

} //

//----------------------------------------------------------------------

} // namespace kestrel
