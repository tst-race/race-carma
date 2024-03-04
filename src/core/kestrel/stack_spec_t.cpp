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
#include <kestrel/stack_spec_t.hpp>
// Include twice to test idempotence.
#include <kestrel/stack_spec_t.hpp>
//

#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/json/unknown_key.hpp>
#include <sst/catalog/optional.hpp>

#include <kestrel/cluster_spec_t.hpp>
#include <kestrel/cluster_vector_t.hpp>
#include <kestrel/genesis_spec_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// clusters
//----------------------------------------------------------------------

void stack_spec_t::parse_clusters(json_t & src, parse_state_t & state) {
  if (state.called_parse_clusters_) {
    return;
  }
  state.called_parse_clusters_ = true;
  sst::json::remove_to(src, clusters_, "clusters", cluster_vector_t());
}

void stack_spec_t::unparse_clusters(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("clusters")));
  dst["clusters"] = clusters_;
}

std::vector<cluster_spec_t> & stack_spec_t::clusters() {
  return clusters_.vector();
}

std::vector<cluster_spec_t> const & stack_spec_t::clusters() const {
  return clusters_.vector();
}

//----------------------------------------------------------------------
// genesis
//----------------------------------------------------------------------

void stack_spec_t::parse_genesis(json_t & src, parse_state_t & state) {
  if (state.called_parse_genesis_) {
    return;
  }
  state.called_parse_genesis_ = true;
  sst::json::remove_to(src, genesis_, "genesis");
}

void stack_spec_t::unparse_genesis(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("genesis")));
  if (genesis()) {
    dst["genesis"] = *genesis();
  }
}

sst::optional<genesis_spec_t> & stack_spec_t::genesis() {
  return genesis_;
}

sst::optional<genesis_spec_t> const & stack_spec_t::genesis() const {
  return genesis_;
}

//----------------------------------------------------------------------
// JSON conversions
//----------------------------------------------------------------------

void from_json(json_t src, stack_spec_t & dst) {

  stack_spec_t tmp;
  stack_spec_t::parse_state_t state;

  tmp.parse_clusters(src, state);
  tmp.parse_genesis(src, state);

  SST_ASSERT((state.called_parse_clusters_));
  SST_ASSERT((state.called_parse_genesis_));

  sst::json::unknown_key(src);

  dst = std::move(tmp);

} //

void to_json(json_t & dst, stack_spec_t const & src) {

  json_t tmp = json_t::object();

  src.unparse_clusters(tmp);
  src.unparse_genesis(tmp);

  dst = std::move(tmp);

} //

//----------------------------------------------------------------------

} // namespace kestrel
