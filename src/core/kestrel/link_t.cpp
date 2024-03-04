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
#include <kestrel/link_t.hpp>
// Include twice to test idempotence.
#include <kestrel/link_t.hpp>

#include <LinkProperties.h>
#include <SdkResponse.h>
#include <kestrel/old_config_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/connection_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/race_to_json.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/tracing_exception_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <cassert>
#include <map>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <set>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <string>
#include <utility>
#include <vector>

namespace kestrel {

link_t::link_t(tracing_event_t tev,
               old_config_t const & config,
               sdk_wrapper_t & sdk,
               link_id_t const & id,
               std::vector<std::string> const & personas)
    : sdk_((SST_TEV_ADD(tev), sdk)),
      old_config_(config),
      id_(id) {
  try {
    for (auto const & persona : personas) {
      personas_.emplace(persona);
    }
    properties_ = sdk_.getLinkProperties(
        SST_TEV_ARG(tev, "link", nlohmann::json({{"id", id_}})),
        id_.string());
  }
  SST_TEV_RETHROW(tev);
}

link_t::link_t(link_t && other) noexcept
    : sdk_((assert(!other.moved_from_), other.sdk_)),
      old_config_(other.old_config_),
      id_(std::move(other.id_)),
      personas_(std::move(other.personas_)),
      properties_(std::move(other.properties_)) {
  other.moved_from_ = true;
}

link_id_t const & link_t::id() const noexcept {
  assert(!moved_from_);
  return id_;
}

void link_t::add_personas(std::vector<std::string> const & personas) {
  for (auto const & persona : personas) {
    personas_.emplace(persona);
  }
}

std::set<psn_t> const & link_t::personas() const noexcept {
  assert(!moved_from_);
  return personas_;
}

LinkProperties const & link_t::properties() const noexcept {
  assert(!moved_from_);
  return properties_;
}

LinkProperties const &
link_t::properties(LinkProperties const & properties) {
  assert(!moved_from_);
  properties_ = properties;
  return properties_;
}

std::map<connection_id_t, connection_t> &
link_t::connections() noexcept {
  assert(!moved_from_);
  return connections_;
}

std::map<connection_id_t, connection_t> const &
link_t::connections() const noexcept {
  assert(!moved_from_);
  return connections_;
}

nlohmann::json link_t::to_json() const {
  assert(!moved_from_);
  auto j = nlohmann::json::object();
  j["id"] = id_;
  j["properties"] = race_to_json(properties_);
  return j;
}

} // namespace kestrel
