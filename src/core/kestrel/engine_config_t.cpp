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
#include <kestrel/engine_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/engine_config_t.hpp>
//

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/json/remove_as.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/json/unknown_key.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// clients
//----------------------------------------------------------------------

void engine_config_t::parse_clients(json_t & src,
                                    parse_state_t & state) {
  if (state.called_parse_clients_) {
    return;
  }
  state.called_parse_clients_ = true;
  this->set_clients(
      sst::json::remove_as<std::vector<psn_t>>(src, "clients"));
}

void engine_config_t::unparse_clients(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("clients")));
  dst["clients"] = *this->clients();
}

std::shared_ptr<std::vector<psn_t> const>
engine_config_t::clients() const {
  std::shared_ptr<std::vector<psn_t> const> p = this->clients_.load();
  SST_ASSERT((p != nullptr));
  return p;
}

void engine_config_t::set_clients(std::vector<psn_t> src) {
  std::sort(src.begin(), src.end());
  src.erase(std::unique(src.begin(), src.end()), src.end());
  this->clients_.store(
      std::make_shared<std::vector<psn_t> const>(std::move(src)));
}

//----------------------------------------------------------------------
// psn
//----------------------------------------------------------------------

void engine_config_t::parse_psn(json_t & src, parse_state_t & state) {
  if (state.called_parse_psn_) {
    return;
  }
  state.called_parse_psn_ = true;
  this->set_psn(sst::json::remove_as<psn_t>(src, "psn"));
}

void engine_config_t::unparse_psn(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("psn")));
  dst["psn"] = *this->psn();
}

std::shared_ptr<psn_t const> engine_config_t::psn() const {
  std::shared_ptr<psn_t const> p = this->psn_.load();
  SST_ASSERT((p != nullptr));
  return p;
}

void engine_config_t::set_psn(psn_t src) {
  this->psn_.store(std::make_shared<psn_t const>(std::move(src)));
}

//----------------------------------------------------------------------
// role
//----------------------------------------------------------------------

void engine_config_t::parse_role(json_t & src, parse_state_t & state) {
  if (state.called_parse_role_) {
    return;
  }
  state.called_parse_role_ = true;
  this->set_role(sst::json::remove_as<role_t>(src, "role"));
}

void engine_config_t::unparse_role(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("role")));
  dst["role"] = *this->role();
}

std::shared_ptr<role_t const> engine_config_t::role() const {
  std::shared_ptr<role_t const> p = this->role_.load();
  SST_ASSERT((p != nullptr));
  return p;
}

void engine_config_t::set_role(role_t src) {
  this->role_.store(std::make_shared<role_t const>(std::move(src)));
}

//----------------------------------------------------------------------
// JSON conversions
//----------------------------------------------------------------------

void from_json(json_t src, engine_config_t & dst) {

  engine_config_t tmp;
  engine_config_t::parse_state_t state;

  tmp.parse_clients(src, state);
  tmp.parse_psn(src, state);
  tmp.parse_role(src, state);

  SST_ASSERT((state.called_parse_clients_));
  SST_ASSERT((state.called_parse_psn_));
  SST_ASSERT((state.called_parse_role_));

  sst::json::unknown_key(src);

  dst = std::move(tmp);

} //

void to_json(json_t & dst, engine_config_t const & src) {

  json_t tmp = json_t::object();

  src.unparse_clients(tmp);
  src.unparse_psn(tmp);
  src.unparse_role(tmp);

  dst = std::move(tmp);

} //

//----------------------------------------------------------------------

} // namespace kestrel
