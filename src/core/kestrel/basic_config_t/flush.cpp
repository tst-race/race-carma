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

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/json/get_as_file.hpp>
#include <sst/catalog/mkdir_p_only.hpp>

#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

void basic_config_t::flush(tracing_event_t tev) {
  SST_TEV_ADD(tev);
  try {
    SST_ASSERT((!moved_from_));
    if (sdk_ == nullptr) {
      sst::mkdir_p_only(json_file_);
      sst::json::get_as_file(json(SST_TEV_ARG(tev)), json_file_);
    } else {
      sdk_->xMakeParentDirs(SST_TEV_ARG(tev), json_file_);
      sdk_->xWriteJsonFile(SST_TEV_ARG(tev),
                           json_file_,
                           json(SST_TEV_ARG(tev)));
    }
  }
  SST_TEV_RETHROW(tev);
}

} // namespace kestrel
