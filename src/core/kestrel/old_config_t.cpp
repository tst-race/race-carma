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

#include <kestrel/old_config_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <cstdint>
#include <map>
#include <set>
#include <utility>
#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/bigint.hpp>
#include <sst/catalog/floor_sqrt.hpp>
#include <sst/catalog/json/get_as.hpp>
#include <sst/catalog/json/get_to.hpp>
#include <sst/catalog/json/remove_to.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// allow_multicast
//----------------------------------------------------------------------

bool old_config_t::allow_multicast(nlohmann::json & src) {
  if (!done_allow_multicast_) {
    sst::json::remove_to(src, allow_multicast_, "allow_multicast");
    done_allow_multicast_ = true;
  }
  return allow_multicast_;
}

bool old_config_t::allow_multicast() const {
  SST_ASSERT((done_allow_multicast_));
  return allow_multicast_;
}

//----------------------------------------------------------------------
// dynamic_only
//----------------------------------------------------------------------

bool old_config_t::dynamic_only(nlohmann::json & src) {
  if (!done_dynamic_only_) {
    sst::json::remove_to(src, dynamic_only_, "dynamic_only", false);
    done_dynamic_only_ = true;
  }
  return dynamic_only_;
}

bool old_config_t::dynamic_only() const {
  SST_ASSERT((done_dynamic_only_));
  return dynamic_only_;
}

//----------------------------------------------------------------------
// link_discovery_cooldown
//----------------------------------------------------------------------

std::uintmax_t old_config_t::link_discovery_cooldown(nlohmann::json & src) {
  if (!done_link_discovery_cooldown_) {
    sst::json::remove_to(src,
                         link_discovery_cooldown_,
                         "link_discovery_cooldown",
                         60);
    done_link_discovery_cooldown_ = true;
  }
  return link_discovery_cooldown_;
}

std::uintmax_t old_config_t::link_discovery_cooldown() const {
  SST_ASSERT((done_link_discovery_cooldown_));
  return link_discovery_cooldown_;
}

//----------------------------------------------------------------------
// send_retry_count
//----------------------------------------------------------------------

sst::bigint const & old_config_t::send_retry_count(nlohmann::json & src) {
  if (!done_send_retry_count_) {
    sst::json::remove_to(src, send_retry_count_, "send_retry_count");
    done_send_retry_count_ = true;
  }
  return send_retry_count_;
}

sst::bigint const & old_config_t::send_retry_count() const {
  SST_ASSERT((done_send_retry_count_));
  return send_retry_count_;
}

//----------------------------------------------------------------------
// send_retry_window
//----------------------------------------------------------------------

sst::bigint const & old_config_t::send_retry_window(nlohmann::json & src) {
  if (!done_send_retry_window_) {
    sst::json::remove_to(src, send_retry_window_, "send_retry_window");
    done_send_retry_window_ = true;
  }
  return send_retry_window_;
}

sst::bigint const & old_config_t::send_retry_window() const {
  SST_ASSERT((done_send_retry_window_));
  return send_retry_window_;
}

//----------------------------------------------------------------------
// send_timeout
//----------------------------------------------------------------------

sst::bigint const & old_config_t::send_timeout(nlohmann::json & src) {
  if (!done_send_timeout_) {
    sst::json::remove_to(src, send_timeout_, "send_timeout");
    done_send_timeout_ = true;
  }
  return send_timeout_;
}

sst::bigint const & old_config_t::send_timeout() const {
  SST_ASSERT((done_send_timeout_));
  return send_timeout_;
}

//----------------------------------------------------------------------

void from_json(nlohmann::json src, old_config_t & dst) {

  dst.allow_multicast(src);
  dst.dynamic_only(src);
  dst.link_discovery_cooldown(src);
  dst.send_retry_count(src);
  dst.send_retry_window(src);
  dst.send_timeout(src);

  dst.threshold = src["Threshold"];
  sst::json::get_to(src, dst.prime, "Prime");

  dst.mixsize = src["MixSize"];

  dst.loglevel = 2000;
  if (src.contains("LogLevel")) {
    dst.loglevel = src["LogLevel"];
  }

  sst::json::get_to(src, dst.leader_relay_only, "leader_relay_only");
  sst::json::get_to(src, dst.inter_server_direct_only, "inter_server_direct_only");
}

} // namespace kestrel
