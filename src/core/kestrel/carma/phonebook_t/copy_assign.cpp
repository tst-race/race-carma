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

#include <memory>
#include <utility>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>

#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

phonebook_t & phonebook_t::copy_assign(tracing_event_t tev,
                                       phonebook_t const & other) {
  SST_TEV_TOP(tev);
  clear();
  for (phonebook_pair_t const & pair : other) {
    add_fast(SST_TEV_ARG(tev), *other.at(SST_TEV_ARG(tev), pair));
  }
  return *this;
  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
