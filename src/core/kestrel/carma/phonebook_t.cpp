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
#include <kestrel/carma/phonebook_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/phonebook_t.hpp>
//

#include <functional>
#include <memory>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>

#include <kestrel/carma/config_t.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

//----------------------------------------------------------------------
// add_fast
//----------------------------------------------------------------------

std::shared_ptr<phonebook_entry_t const> &
phonebook_t::add_fast(tracing_event_t tev, psn_t psn) {
  SST_TEV_TOP(tev);
  auto const entries_r = entries_.emplace(std::move(psn), nullptr);
  SST_ASSERT((entries_r.second));
  psn_t const & k = entries_r.first->first;
  phonebook_pair_t * const v = &*entries_r.first;
  try {
    auto const psn_hashes_1_r = psn_hashes_1_.emplace(k, v);
    SST_ASSERT((psn_hashes_1_r.second));
  } catch (...) {
    entries_.erase(entries_r.first);
    throw;
  }
  return entries_r.first->second;
  SST_TEV_BOT(tev);
}

std::shared_ptr<phonebook_entry_t const>
phonebook_t::add_fast(tracing_event_t tev, phonebook_entry_t entry) {
  SST_TEV_TOP(tev);
  entry.set_phonebook(*this);
  std::shared_ptr<phonebook_entry_t const> & p =
      add_fast(SST_TEV_ARG(tev), entry.psn());
  p = std::make_shared<phonebook_entry_t>(std::move(entry));
  return p;
  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------
// add_slow
//----------------------------------------------------------------------

std::shared_ptr<phonebook_entry_t const>
phonebook_t::add_slow(tracing_event_t tev, phonebook_entry_t entry) {
  SST_TEV_TOP(tev);

  entry.set_phonebook(*this);
  entry.clear_deducible();

  role_t const entry_role = entry.role();
  bool const entry_is_client = entry_role == role_t::client();
  bool const entry_is_mb_server = entry_role == role_t::mb_server();
  if (entry_is_client || entry_is_mb_server) {
    for (phonebook_pair_t const * const & p :
         entry.bucket_clients(SST_TEV_ARG(tev))) {
      std::shared_ptr<phonebook_entry_t const> const q =
          at(SST_TEV_ARG(tev), *p);
      phonebook_entry_t & x = const_cast<phonebook_entry_t &>(*q);
      if (entry_is_client) {
        x.clear_bucket_clients();
      } else {
        x.clear_bucket_mb_servers();
      }
    }
    for (phonebook_pair_t const * const & p :
         entry.bucket_mb_servers(SST_TEV_ARG(tev))) {
      std::shared_ptr<phonebook_entry_t const> const q =
          at(SST_TEV_ARG(tev), *p);
      phonebook_entry_t & x = const_cast<phonebook_entry_t &>(*q);
      if (entry_is_client) {
        x.clear_bucket_clients();
      } else {
        x.clear_bucket_mb_servers();
      }
    }
    role_t const local_role = config().local().role();
    if ((local_role == role_t::client()
         || local_role == role_t::mb_server())
        && config().local().bucket(SST_TEV_ARG(tev))
               == entry.bucket(SST_TEV_ARG(tev))) {
      if (entry_is_client) {
        config().local().clear_bucket_clients();
      } else {
        config().local().clear_bucket_mb_servers();
      }
    }
  }

  std::shared_ptr<phonebook_entry_t const> p =
      add_fast(SST_TEV_ARG(tev), std::move(entry));

  // TODO: Unload the least recently used entry (or entries) from memory
  //       if we have too many currently loaded.

  return p;

  SST_TEV_BOT(tev);
}

//----------------------------------------------------------------------

} // namespace carma
} // namespace kestrel
