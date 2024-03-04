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

#ifndef KESTREL_CARMA_DEFINE_INNER_E_FOO_HPP
#define KESTREL_CARMA_DEFINE_INNER_E_FOO_HPP

#include <exception>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <sst/catalog/enable_if_t.hpp>
#include <sst/catalog/to_string.hpp>

#include <SdkResponse.h>

#include <kestrel/sdk_status_t.hpp>

#define CARMA_DEFINE_INNER_E_FOO(Sdk, foo)                             \
                                                                       \
  template<class... Args>                                              \
  auto inner_e_##foo(Sdk & sdk, Args &&... args)                       \
      ->::sst::enable_if_t<                                            \
          ::std::is_same<decltype(sdk.foo(                             \
                             ::std::forward<Args>(args)...)),          \
                         SdkResponse>::value,                          \
          SdkResponse> {                                               \
    try {                                                              \
      SdkResponse const r = sdk.foo(::std::forward<Args>(args)...);    \
      if (r.status != SDK_OK) {                                        \
        throw ::std::runtime_error(                                    \
            #foo "() failed: Error code "                              \
            + ::kestrel::sdk_status_t(r.status).name() + " = "           \
            + ::sst::to_string(r.status) + ".");                       \
      }                                                                \
      return r;                                                        \
    } catch (::std::exception const &) {                               \
      throw;                                                           \
    } catch (...) {                                                    \
      throw ::std::runtime_error(#foo "() failed: Unknown error.");    \
    }                                                                  \
  }                                                                    \
                                                                       \
  template<class... Args>                                              \
  auto inner_e_##foo(Sdk & sdk, Args &&... args)                       \
      ->::sst::enable_if_t<                                            \
          !::std::is_same<decltype(sdk.foo(                            \
                              ::std::forward<Args>(args)...)),         \
                          SdkResponse>::value,                         \
          decltype(sdk.foo(::std::forward<Args>(args)...))> {          \
    try {                                                              \
      return sdk.foo(::std::forward<Args>(args)...);                   \
    } catch (::std::exception const &) {                               \
      throw;                                                           \
    } catch (...) {                                                    \
      throw ::std::runtime_error(#foo "() failed: Unknown error.");    \
    }                                                                  \
  }

#endif // #ifndef KESTREL_CARMA_DEFINE_INNER_E_FOO_HPP
