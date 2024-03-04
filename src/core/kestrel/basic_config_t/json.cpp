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
#include <kestrel/basic_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/basic_config_t.hpp>
//

#include <exception>
#include <memory>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/get_from_file.hpp>
#include <sst/catalog/json/get_from_string.hpp>
#include <sst/catalog/make_unique.hpp>
#include <sst/catalog/test_e.hpp>

#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

json_t & basic_config_t::json(tracing_event_t tev) {
  SST_TEV_ADD(tev);
  try {
    SST_ASSERT((!moved_from_));
    if (json_ == nullptr) {
      if (sdk_ == nullptr) {
        if (sst::test_e(json_file_)) {
          json_ = sst::make_unique<json_t>(
              sst::json::get_from_file<json_t>(json_file_));
        } else {
          json_ = sst::make_unique<json_t>(json_t::object());
        }
      } else {
        if (sdk_->xPathExists(SST_TEV_ARG(tev), json_file_)) {
          try {
            json_ = sst::make_unique<json_t>(
                sst::json::get_from_string<json_t>(
                    sdk_->readFile(SST_TEV_ARG(tev), json_file_)));
          } catch (sst::json::exception const & e) {
            std::throw_with_nested(e.add_file(json_file_));
          }
        } else {
          json_ = sst::make_unique<json_t>(json_t::object());
        }
      }
    }
    return *json_;
  }
  SST_TEV_RETHROW(tev);
}

json_t const & basic_config_t::json(tracing_event_t tev) const {
  SST_TEV_ADD(tev);
  try {
    SST_ASSERT((!moved_from_));
    return const_cast<basic_config_t &>(*this).json(SST_TEV_ARG(tev));
  }
  SST_TEV_RETHROW(tev);
}

} // namespace kestrel
