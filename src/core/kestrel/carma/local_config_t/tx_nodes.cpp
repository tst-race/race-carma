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

#include <mutex>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/SST_UNREACHABLE.hpp>
#include <sst/catalog/mutex.hpp>

#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_set_t.hpp>
#include <kestrel/carma/phonebook_vector_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

namespace {

void add(phonebook_set_t & dst, phonebook_pair_t const & src) {
  auto const r = dst.emplace(&src);
  SST_ASSERT((r.second));
}

void add(phonebook_set_t & dst,
         phonebook_vector_t const & src,
         phonebook_vector_t::size_type const off) {
  SST_ASSERT((off <= src.size()));
  for (phonebook_vector_t::size_type i = off; i < src.size(); ++i) {
    add(dst, *src[i]);
  }
}

void add(phonebook_set_t & dst, phonebook_set_t const & src) {
  for (phonebook_pair_t const * const & p : src) {
    add(dst, *p);
  }
}

} // namespace

void local_config_t::clear_tx_nodes() const {
  SST_ASSERT((!moved_from_));
  tx_nodes_.reset();
}

void local_config_t::parse_tx_nodes(json_t & src) {
  SST_ASSERT((!moved_from_));
  parse_phonebook_set(src, "tx_nodes", tx_nodes_);
}

void local_config_t::unparse_tx_nodes(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  unparse_phonebook_set(dst, "tx_nodes", tx_nodes_);
}

phonebook_set_t const &
local_config_t::tx_nodes(tracing_event_t tev) const {
  SST_TEV_TOP(tev);
  SST_ASSERT((!moved_from_));
  std::lock_guard<sst::mutex> const lock(tx_nodes_mutex_);
  if (!tx_nodes_) {
    phonebook_set_t xs;
    switch (role()) {
      case role_t::client(): {
        add(xs, bucket_mb_servers(SST_TEV_ARG(tev)));
      } break;
      case role_t::idle_server(): {
        add(xs, other_servers());
      } break;
      case role_t::mb_server(): {
        add(xs, mc_leaders(SST_TEV_ARG(tev)));
        add(xs, bucket_clients(SST_TEV_ARG(tev)));
        add(xs, other_servers());
      } break;
      case role_t::mc_follower(): {
        add(xs, mc_leader(SST_TEV_ARG(tev)));
        add(xs, other_servers());
      } break;
      case role_t::mc_leader(): {
        add(xs, mc_group(SST_TEV_ARG(tev)), 1U);
        add(xs, next_layer(SST_TEV_ARG(tev)));
        add(xs, other_servers());
      } break;
      case role_t::rs_server(): {
        add(xs, next_layer(SST_TEV_ARG(tev)));
        add(xs, other_servers());
      } break;
      default: {
        SST_UNREACHABLE();
      } break;
    }
    tx_nodes_.emplace(std::move(xs));
  }
  return *tx_nodes_;
  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
