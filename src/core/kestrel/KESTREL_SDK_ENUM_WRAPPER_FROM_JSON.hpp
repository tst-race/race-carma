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

#ifndef KESTREL_KESTREL_SDK_ENUM_WRAPPER_FROM_JSON_HPP
#define KESTREL_KESTREL_SDK_ENUM_WRAPPER_FROM_JSON_HPP

#include <string>

#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/get_as.hpp>
#include <sst/catalog/remove_cvref_t.hpp>

#include <kestrel/json_t.hpp>

#define KESTREL_SDK_ENUM_WRAPPER_FROM_JSON_ITEM(A, B)                  \
  if (x == #B) {                                                       \
    dst = sst::remove_cvref_t<decltype(dst)>::A();                     \
    return;                                                            \
  }

#define KESTREL_SDK_ENUM_WRAPPER_FROM_JSON(NAME, ITEMS)                \
  void from_json(json_t const & src, NAME & dst) {                     \
    std::string const x = sst::json::get_as<std::string>(src);         \
    ITEMS(KESTREL_SDK_ENUM_WRAPPER_FROM_JSON_ITEM)                     \
    throw sst::json::exception::invalid_value(src);                    \
  }

#endif // #ifndef KESTREL_KESTREL_SDK_ENUM_WRAPPER_FROM_JSON_HPP
