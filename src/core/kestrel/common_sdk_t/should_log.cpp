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
#include <kestrel/common_sdk_t.hpp>
// Include twice to test idempotence.
#include <kestrel/common_sdk_t.hpp>
//

#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/is_negative.hpp>

#include <RaceLog.h>

#include <kestrel/logging.hpp>

namespace kestrel {

std::pair<bool, RaceLog::LogLevel>
common_sdk_t::should_log(log_level_t const level,
                         log_level_t const sublevel) const noexcept {
  log_level_t this_level;
  log_level_t next_level;
  RaceLog::LogLevel race_level;
  if (level >= log_level_fatal) {
    this_level = log_level_fatal;
    next_level = log_level_limit;
    race_level = RaceLog::LL_ERROR;
  } else if (level >= log_level_error) {
    this_level = log_level_error;
    next_level = log_level_fatal;
    race_level = RaceLog::LL_ERROR;
  } else if (level >= log_level_warn) {
    this_level = log_level_warn;
    next_level = log_level_error;
    race_level = RaceLog::LL_WARNING;
  } else if (level >= log_level_info) {
    this_level = log_level_info;
    next_level = log_level_warn;
    race_level = RaceLog::LL_INFO;
  } else if (level >= log_level_debug) {
    this_level = log_level_debug;
    next_level = log_level_info;
    race_level = RaceLog::LL_DEBUG;
  } else {
    this_level = log_level_trace;
    next_level = log_level_debug;
    race_level = RaceLog::LL_DEBUG;
  }
  SST_ASSERT((level == this_level));
  SST_ASSERT((!sst::is_negative(sublevel)));
  SST_ASSERT((sublevel < next_level - this_level));
  log_level_t const current_level = log_level();
  return std::make_pair(this_level >= current_level
                            || sublevel >= current_level - this_level,
                        race_level);
}

} // namespace kestrel
