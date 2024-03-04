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

#ifndef KESTREL_LOGGING_HPP
#define KESTREL_LOGGING_HPP

#include <sst/catalog/SST_DISPATCH.h>

//----------------------------------------------------------------------

namespace kestrel {

using log_level_t = decltype(60000);

enum : log_level_t {
  log_level_limit = 60000,
  log_level_fatal = 50000,
  log_level_error = 40000,
  log_level_warn = 30000,
  log_level_info = 20000,
  log_level_debug = 10000,
  log_level_trace = 0
};

} // namespace kestrel

//----------------------------------------------------------------------

#define KESTREL_LOG(sdk, level, sublevel, message)                     \
  ([&]() -> bool {                                                     \
    try {                                                              \
      auto const & KESTREL_sdk = (sdk);                                \
      auto const KESTREL_pair =                                        \
          KESTREL_sdk.should_log(level, sublevel);                     \
      if (KESTREL_pair.first) {                                        \
        return KESTREL_sdk.log(__FILE__,                               \
                               __func__,                               \
                               __LINE__,                               \
                               KESTREL_pair.second,                    \
                               message);                               \
      }                                                                \
      return true;                                                     \
    } catch (...) {                                                    \
      return false;                                                    \
    }                                                                  \
  }())

//----------------------------------------------------------------------

#define KESTREL_TRACE_2(sdk, message)                                  \
  (KESTREL_LOG(sdk, ::kestrel::log_level_trace, 0, message))

#define KESTREL_TRACE(...)                                             \
  SST_DISPATCH(KESTREL_TRACE_, __VA_ARGS__)(__VA_ARGS__)

//----------------------------------------------------------------------

#define KESTREL_DEBUG_2(sdk, message)                                  \
  (KESTREL_LOG(sdk, ::kestrel::log_level_debug, 0, message))

#define KESTREL_DEBUG(...)                                             \
  SST_DISPATCH(KESTREL_DEBUG_, __VA_ARGS__)(__VA_ARGS__)

//----------------------------------------------------------------------

#define KESTREL_INFO_2(sdk, message)                                   \
  (KESTREL_LOG(sdk, ::kestrel::log_level_info, 0, message))

#define KESTREL_INFO(...)                                              \
  SST_DISPATCH(KESTREL_INFO_, __VA_ARGS__)(__VA_ARGS__)

//----------------------------------------------------------------------

#define KESTREL_WARN_2(sdk, message)                                   \
  (KESTREL_LOG(sdk, ::kestrel::log_level_warn, 0, message))

#define KESTREL_WARN(...)                                              \
  SST_DISPATCH(KESTREL_WARN_, __VA_ARGS__)(__VA_ARGS__)

//----------------------------------------------------------------------

#define KESTREL_ERROR_2(sdk, message)                                  \
  (KESTREL_LOG(sdk, ::kestrel::log_level_error, 0, message))

#define KESTREL_ERROR(...)                                             \
  SST_DISPATCH(KESTREL_ERROR_, __VA_ARGS__)(__VA_ARGS__)

//----------------------------------------------------------------------

#define KESTREL_FATAL_2(sdk, message)                                  \
  (KESTREL_LOG(sdk, ::kestrel::log_level_fatal, 0, message))

#define KESTREL_FATAL(...)                                             \
  SST_DISPATCH(KESTREL_FATAL_, __VA_ARGS__)(__VA_ARGS__)

//----------------------------------------------------------------------

#endif // #ifndef KESTREL_LOGGING_HPP
