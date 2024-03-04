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

#ifndef KESTREL_NULL_CONFIG_T_HPP
#define KESTREL_NULL_CONFIG_T_HPP

#include <string>

#include <sst/catalog/SST_NOEXCEPT.hpp>

#include <kestrel/common_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

class null_config_t {

public:

  explicit null_config_t(tracing_event_t,
                         std::string,
                         common_sdk_t * = nullptr) noexcept {
  }

  null_config_t(null_config_t const &) SST_NOEXCEPT(true) = default;

  null_config_t & operator=(null_config_t const &)
      SST_NOEXCEPT(true) = default;

  null_config_t(null_config_t &&) SST_NOEXCEPT(true) = default;

  null_config_t & operator=(null_config_t &&)
      SST_NOEXCEPT(true) = default;

  ~null_config_t() SST_NOEXCEPT(true) = default;

  void flush(tracing_event_t) noexcept {
  }

}; //

} // namespace kestrel

#endif // #ifndef KESTREL_NULL_CONFIG_T_HPP
