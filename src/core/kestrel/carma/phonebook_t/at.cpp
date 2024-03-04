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

#include <atomic>
#include <exception>
#include <memory>
#include <string>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_DEBUG.h>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/get_from_file.hpp>
#include <sst/catalog/json/get_from_string.hpp>
#include <sst/catalog/json/get_to.hpp>
#include <sst/catalog/optional.hpp>

#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

std::shared_ptr<phonebook_entry_t const>
phonebook_t::at(tracing_event_t tev,
                phonebook_pair_t const & entry) const {
  SST_TEV_TOP(tev);
  if (SST_DEBUG) {
    auto const it = entries_.find(entry.first);
    SST_ASSERT((it != entries_.end()));
    SST_ASSERT((&*it == &entry));
  }
  std::shared_ptr<phonebook_entry_t const> p =
      std::atomic_load(&entry.second);
  if (!p) {
    std::string const file = file_for(entry.first);
    std::shared_ptr<phonebook_entry_t const> pbe_ptr =
        std::make_shared<phonebook_entry_t>();
    phonebook_entry_t & pbe = const_cast<phonebook_entry_t &>(*pbe_ptr);
    // TODO: Maybe don't want const_cast here? Yeah, I think the
    //       phonebook back pointer should probably be const everywhere.
    pbe.set_phonebook(const_cast<phonebook_t &>(*this));
    if (sdk_ == nullptr) {
      sst::json::get_to(sst::json::get_from_file<json_t>(file), pbe);
    } else {
      try {
        sst::json::get_to(sst::json::get_from_string<json_t>(
                              sdk_->readFile(SST_TEV_ARG(tev), file)),
                          pbe);
      } catch (sst::json::exception const & e) {
        std::throw_with_nested(e.add_file(file));
      }
    }
    if (std::atomic_compare_exchange_strong(
            &const_cast<phonebook_pair_t &>(entry).second,
            &p,
            pbe_ptr)) {
      p = pbe_ptr;
    }
  }
  return p;
  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
