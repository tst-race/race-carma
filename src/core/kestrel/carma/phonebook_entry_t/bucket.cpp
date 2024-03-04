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
#include <kestrel/carma/phonebook_entry_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/phonebook_entry_t.hpp>
//

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/atomic.hpp>
#include <sst/catalog/bigint.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/sha512_t.hpp>

#include <kestrel/carma/config_t.hpp>
#include <kestrel/carma/global_config_t.hpp>
#include <kestrel/carma/node_count_t.hpp>
#include <kestrel/carma/phonebook_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {
namespace carma {

void phonebook_entry_t::clear_bucket() const {
  SST_ASSERT((!moved_from_));
  bucket_.store(node_count_max());
}

void phonebook_entry_t::parse_bucket(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (bucket_.load() != node_count_max()) {
    return;
  }
  parse_role(src);
  if (role() != role_t::client() && role() != role_t::mb_server()) {
    return;
  }
  sst::optional<node_count_t> val;
  sst::json::remove_to(src, val, "bucket");
  if (val) {
    bucket_.store(*val);
  }
}

void phonebook_entry_t::unparse_bucket(json_t & dst) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((dst.is_object()));
  node_count_t const val = bucket_.load();
  if (val != node_count_max()) {
    dst["bucket"] = sst::to_string(val);
  }
}

node_count_t phonebook_entry_t::bucket(tracing_event_t tev) const {
  SST_TEV_TOP(tev);
  SST_ASSERT((!moved_from_));
  SST_ASSERT(
      (role() == role_t::client() || role() == role_t::mb_server()));
  node_count_t val = bucket_.load();
  if (val == node_count_max()) {
    sst::sha512_t f;
    f.init();
    f.update(global().epoch_nonce().data(),
             global().epoch_nonce().size());
    f.update(psn().value().data(), psn().value().size());
    f.finish();
    sst::bigint n;
    n.set_from_bytes(f.output().data(),
                     f.output().size(),
                     sst::integer_rep::pure_unsigned());
    node_count_t const new_val =
        static_cast<node_count_t>(n % global().num_buckets());
    if (bucket_.compare_exchange_strong(val, new_val)) {
      val = new_val;
    }
  }
  return val;
  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
