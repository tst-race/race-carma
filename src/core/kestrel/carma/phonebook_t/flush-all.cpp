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

#include <string>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/dir_it.hpp>
#include <sst/catalog/json/get_as_file.hpp>
#include <sst/catalog/mkdir_p_only.hpp>
#include <sst/catalog/rm_f_r.hpp>
#include <sst/catalog/test_e.hpp>

#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

void phonebook_t::flush(tracing_event_t tev, bool const pack) {
  SST_TEV_ADD(tev);
  try {

    if (pack) {

      //----------------------------------------------------------------
      // Write out the packed file
      //----------------------------------------------------------------

      {
        json_t json = json_t::object();
        for (auto const & psn_pbe : entries_) {
          json.emplace(psn_pbe.first.to_path_slug(),
                       *at(SST_TEV_ARG(tev), psn_pbe));
        }
        if (sdk_ == nullptr) {
          sst::mkdir_p_only(packed_file_);
          sst::json::get_as_file(json, packed_file_);
        } else {
          sdk_->xMakeParentDirs(SST_TEV_ARG(tev), packed_file_);
          sdk_->xWriteJsonFile(SST_TEV_ARG(tev), packed_file_, json);
        }
      }

      //----------------------------------------------------------------
      // Delete any unpacked files
      //----------------------------------------------------------------

      if (sdk_ == nullptr) {
        if (sst::test_e(entries_dir_)) {
          sst::rm_f_r(entries_dir_);
        }
      } else {
        if (sdk_->xPathExists(SST_TEV_ARG(tev), entries_dir_)) {
          sdk_->removeDir(SST_TEV_ARG(tev), entries_dir_);
        }
      }

      //----------------------------------------------------------------

    } else {

      //----------------------------------------------------------------
      // Write out the unpacked files
      //----------------------------------------------------------------

      for (auto & kv : entries_) {
        if (kv.second) {
          flush(SST_TEV_ARG(tev), *kv.second, file_for(kv.first));
        }
      }

      //----------------------------------------------------------------
      // Delete any dangling unpacked files
      //----------------------------------------------------------------

      if (sdk_ == nullptr) {
        if (sst::test_e(entries_dir_)) {
          std::string x = entries_dir_;
          for (auto it = sst::dir_it(entries_dir_); it != it.end();
               ++it) {
            psn_t const psn =
                psn_t::from_path_slug(it->str().substr(it.prefix()));
            if (entries_.find(psn) == entries_.end()) {
              x.resize(entries_dir_.size());
              x += '/';
              x += it->str();
              sst::rm_f_r(x);
            }
          }
        }
      } else {
        if (sdk_->xPathExists(SST_TEV_ARG(tev), entries_dir_)) {
          std::string x = entries_dir_;
          for (auto const & slug :
               sdk_->listDir(SST_TEV_ARG(tev), entries_dir_)) {
            psn_t const psn = psn_t::from_path_slug(slug);
            if (entries_.find(psn) == entries_.end()) {
              x.resize(entries_dir_.size());
              x += '/';
              x += slug;
              sdk_->removeDir(SST_TEV_ARG(tev), x);
            }
          }
        }
      }

      //----------------------------------------------------------------
      // Delete any packed file
      //----------------------------------------------------------------

      if (sdk_ == nullptr) {
        if (sst::test_e(packed_dir_)) {
          sst::rm_f_r(packed_dir_);
        }
      } else {
        if (sdk_->xPathExists(SST_TEV_ARG(tev), packed_dir_)) {
          sdk_->removeDir(SST_TEV_ARG(tev), packed_dir_);
        }
      }

      //----------------------------------------------------------------

    } //
  }
  SST_TEV_RETHROW(tev);
}

} // namespace carma
} // namespace kestrel
