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
#include <kestrel/carma/config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/config_t.hpp>
//

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>

#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/delete_atomic_file.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/write_atomic_json_file.hpp>

namespace kestrel {
namespace carma {

void config_t::flush(tracing_event_t tev, bool const pack) {
  SST_TEV_ADD(tev);
  try {

    if (bootstrap_.exists()) {
      write_atomic_json_file(SST_TEV_ARG(tev),
                             sdk_,
                             bootstrap_file_,
                             bootstrap_);
    } else {
      delete_atomic_file(SST_TEV_ARG(tev), sdk_, bootstrap_file_);
    }

    clrmsgs().flush(SST_TEV_ARG(tev));

    write_atomic_json_file(SST_TEV_ARG(tev),
                           sdk_,
                           global_file_,
                           global_);

    write_atomic_json_file(SST_TEV_ARG(tev), sdk_, local_file_, local_);

    phonebook().flush(SST_TEV_ARG(tev), pack);

    chunk_joiner().flush(SST_TEV_ARG(tev));

    if (detached_clrmsg_store_) {
      detached_clrmsg_store_->flush(SST_TEV_ARG(tev));
    }

  } //
  SST_TEV_RETHROW(tev);
}

} // namespace carma
} // namespace kestrel
