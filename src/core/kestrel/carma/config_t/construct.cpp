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

#include <exception>
#include <string>
#include <utility>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/set_from_file.hpp>
#include <sst/catalog/json/set_from_string.hpp>
#include <sst/catalog/test_e.hpp>

#include <kestrel/carma/global_config_t.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/common_sdk_t.hpp>
#include <kestrel/read_atomic_file.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

config_t::config_t(tracing_event_t tev,
                   std::string dir,
                   common_sdk_t * const sdk)
    : dir_(std::move(dir)),
      sdk_(sdk),
      bootstrap_file_(dir_ + "/bootstrap.json"),
      bootstrap_(SST_TEV_ARG(tev), sdk, bootstrap_file_),
      clrmsgs_(SST_TEV_ARG(tev), dir_ + "/clrmsgs", sdk),
      global_file_(dir_ + "/global.json"),
      global_(SST_TEV_ARG(tev), sdk, global_file_),
      local_file_(dir_ + "/local.json"),
      phonebook_(SST_TEV_ARG(tev), *this, dir_ + "/phonebook", sdk),
      chunk_joiner_(SST_TEV_ARG(tev), dir_ + "/chunks", sdk) {
  SST_TEV_TOP(tev);

  //--------------------------------------------------------------------
  // Load local.json
  //--------------------------------------------------------------------

  local_.set_phonebook(phonebook_);
  {
    std::vector<unsigned char> const data =
        read_atomic_file(SST_TEV_ARG(tev), sdk_, local_file_);
    if (!data.empty()) {
      try {
        sst::json::set_from_string<json_t>(data, local_);
      } catch (sst::json::exception const & e) {
        std::throw_with_nested(e.add_file(local_file_));
      }
    }
  }

  //--------------------------------------------------------------------
  // Prevent a bootstrapper from sneaking in bad deduced data
  //--------------------------------------------------------------------

  if (bootstrap_.exists()) {
    clear_deducible(SST_TEV_ARG(tev));
  }

  //--------------------------------------------------------------------

  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
