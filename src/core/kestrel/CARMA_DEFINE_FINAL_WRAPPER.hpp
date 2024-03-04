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

#ifndef KESTREL_CARMA_DEFINE_FINAL_WRAPPER_HPP
#define KESTREL_CARMA_DEFINE_FINAL_WRAPPER_HPP

#include <exception>
#include <type_traits>
#include <utility>

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/enable_if_t.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/what.hpp>

#include <kestrel/CARMA_XLOG_ERROR.hpp>
#include <kestrel/CARMA_XLOG_TRACE.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/tracing_exception_t.hpp>

//
// Note that when o_foo is called to describe the output of foo, some of
// the parameters passed to o_foo may be moved-from, as e_foo is called
// as e_foo(std::forward<Args>(args)...). However, this is OK because
// o_foo should only inspect output parameters, which will never be
// moved-from because e_foo will never take them by value.
//

#define CARMA_DEFINE_FINAL_WRAPPER_A(foo)                              \
  SST_TEV_ADD(tev,                                                     \
              "sdk_function",                                          \
              #foo,                                                    \
              "sdk_function_call_id",                                  \
              sst::to_string(sdk_function_call_id_++));                \
  SST_TEV_ADD(tev, "sdk_function_input", i_##foo(args...));            \
  CARMA_XLOG_TRACE(*this,                                              \
                   0,                                                  \
                   SST_TEV_ARG(tev, "event", "sdk_function_called"));

#define CARMA_DEFINE_FINAL_WRAPPER_r(foo)                              \
  (e_##foo(std::forward<Args>(args)...))

#define CARMA_DEFINE_FINAL_WRAPPER_B()                                 \
  CARMA_XLOG_TRACE(                                                    \
      *this,                                                           \
      0,                                                               \
      SST_TEV_ARG(tev, "event", "sdk_function_succeeded"));

#define CARMA_DEFINE_FINAL_WRAPPER_C(foo)                              \
  catch (...) {                                                        \
    CARMA_XLOG_ERROR(*this,                                            \
                     0,                                                \
                     SST_TEV_ARG(tev,                                  \
                                 "event",                              \
                                 "sdk_function_failed",                \
                                 "exception",                          \
                                 sst::what()));                        \
    std::throw_with_nested(tracing_exception_t(std::move(tev)));       \
  }

#define CARMA_DEFINE_FINAL_WRAPPER(foo)                                \
                                                                       \
  template<class... Args>                                              \
  auto foo(tracing_event_t tev, Args &&... args)                       \
      ->sst::enable_if_t<                                              \
          std::is_void<decltype(CARMA_DEFINE_FINAL_WRAPPER_r(          \
              foo))>::value,                                           \
          decltype(CARMA_DEFINE_FINAL_WRAPPER_r(foo))> {               \
    try {                                                              \
      CARMA_DEFINE_FINAL_WRAPPER_A(foo)                                \
      CARMA_DEFINE_FINAL_WRAPPER_r(foo);                               \
      SST_TEV_ADD(tev, "sdk_function_output", o_##foo(args...));       \
      CARMA_DEFINE_FINAL_WRAPPER_B()                                   \
    }                                                                  \
    CARMA_DEFINE_FINAL_WRAPPER_C(foo)                                  \
  }                                                                    \
                                                                       \
  template<class... Args>                                              \
  auto foo(tracing_event_t tev, Args &&... args)                       \
      ->sst::enable_if_t<                                              \
          !std::is_void<decltype(CARMA_DEFINE_FINAL_WRAPPER_r(         \
              foo))>::value,                                           \
          decltype(CARMA_DEFINE_FINAL_WRAPPER_r(foo))> {               \
    try {                                                              \
      CARMA_DEFINE_FINAL_WRAPPER_A(foo)                                \
      auto const r = CARMA_DEFINE_FINAL_WRAPPER_r(foo);                \
      SST_TEV_ADD(tev, "sdk_function_output", o_##foo(r, args...));    \
      CARMA_DEFINE_FINAL_WRAPPER_B()                                   \
      return r;                                                        \
    }                                                                  \
    CARMA_DEFINE_FINAL_WRAPPER_C(foo)                                  \
  }

#endif // #ifndef KESTREL_CARMA_DEFINE_FINAL_WRAPPER_HPP
