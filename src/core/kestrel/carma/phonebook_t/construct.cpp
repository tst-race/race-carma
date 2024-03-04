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
#include <stdexcept>
#include <string>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/c_quote.hpp>
#include <sst/catalog/dir_it.hpp>
#include <sst/catalog/json/get_from_file.hpp>
#include <sst/catalog/json/get_from_string.hpp>
#include <sst/catalog/json/get_to.hpp>
#include <sst/catalog/path.hpp>
#include <sst/catalog/rm_f_r.hpp>
#include <sst/catalog/test_e.hpp>

#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/common_sdk_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

// TODO: Paging. We don't want to pull all entries into memory if
//       there's too many.

//
// Note that this function may fill the entries_ map past the memory
// limit depending on how many packed entries there are. We can't unpack
// them all to disk at once, as that can cause performance problems, so
// we'll need to somehow slowly unload them from memory onto disk until
// we get down to the memory limit.
//

phonebook_t::phonebook_t(tracing_event_t tev,
                         config_t & config,
                         std::string dir,
                         common_sdk_t * const sdk)
    : config_(&config),
      dir_(std::move(dir)),
      entries_dir_(dir_ + "/unpacked"),
      packed_dir_(dir_ + "/packed"),
      packed_file_(packed_dir_ + "/entries.json"),
      sdk_(sdk) {
  SST_TEV_TOP(tev);

  //--------------------------------------------------------------------
  // Load any packed entries
  //--------------------------------------------------------------------
  //
  // Note that we don't overwrite any existing unpacked entries, as
  // unpacked entries have higher priority than packed entries by
  // convention.
  //

  if (sdk_ == nullptr) {
    bool fully_subsumed = true;
    if (sst::test_e(packed_file_)) {
      json_t const json =
          sst::json::get_from_file<json_t>(packed_file_);
      for (auto const & kv : json.items()) {
        psn_t const psn = psn_t::from_path_slug(kv.key());
        std::string const file = file_for(psn);
        if (!sst::test_e(file) && kv.key() == psn.to_path_slug()) {
          fully_subsumed = false;
          phonebook_entry_t pbe;
          pbe.set_phonebook(*this);
          sst::json::get_to(kv.value(), pbe);
          // TODO: Double-check PSN?
          this->add_fast(SST_TEV_ARG(tev), std::move(pbe));
        }
      }
      if (fully_subsumed) {
        sst::rm_f_r(packed_dir_);
      }
    }
  } else {
    bool fully_subsumed = true;
    if (sdk_->xPathExists(SST_TEV_ARG(tev), packed_file_)) {
      json_t const json = sst::json::get_from_string<json_t>(
          sdk_->readFile(SST_TEV_ARG(tev), packed_file_));
      for (auto const & kv : json.items()) {
        psn_t const psn = psn_t::from_path_slug(kv.key());
        std::string const file = file_for(psn);
        if (!sdk_->xPathExists(SST_TEV_ARG(tev), file)
            && kv.key() == psn.to_path_slug()) {
          fully_subsumed = false;
          phonebook_entry_t pbe;
          pbe.set_phonebook(*this);
          sst::json::get_to(kv.value(), pbe);
          // TODO: Double-check PSN?
          this->add_fast(SST_TEV_ARG(tev), std::move(pbe));
        }
      }
      if (fully_subsumed) {
        sdk_->removeDir(SST_TEV_ARG(tev), packed_dir_);
      }
    }
  }

  //--------------------------------------------------------------------
  // Load any unpacked-only PSNs
  //--------------------------------------------------------------------

  if (sdk_ == nullptr) {
    if (sst::test_e(entries_dir_)) {
      auto it = sst::dir_it(entries_dir_);
      auto const end = it.end();
      for (; it != end; ++it) {
        sst::path & src = *it;
        std::string const & psn_slug = src.str().substr(it.prefix());
        psn_t psn = psn_t::from_path_slug(psn_slug);
        if (psn_slug == psn.to_path_slug()) {
          add_fast(SST_TEV_ARG(tev), std::move(psn));
        } else {
          // TODO: Delete it? It's not slugified properly.
        }
      }
    }
  } else {
    if (sdk_->xPathExists(SST_TEV_ARG(tev), entries_dir_)) {
      for (auto const & psn_slug :
           sdk_->listDir(SST_TEV_ARG(tev), entries_dir_)) {
        psn_t psn = psn_t::from_path_slug(psn_slug);
        if (psn_slug == psn.to_path_slug()) {
          add_fast(SST_TEV_ARG(tev), std::move(psn));
        } else {
          // TODO: Delete it? It's not slugified properly.
        }
      }
    }
  }

  //--------------------------------------------------------------------

  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
