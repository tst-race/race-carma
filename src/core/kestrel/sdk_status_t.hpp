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

#ifndef KESTREL_SDK_STATUS_T_HPP
#define KESTREL_SDK_STATUS_T_HPP

#include <string>
#include <type_traits>

#include <sst/catalog/boxed.hpp>

#include <kestrel/json_t.hpp>

#include <SdkResponse.h>
#include <kestrel/json_t.hpp>

namespace kestrel {

#define CARMA_SDK_STATUS_LIST                                          \
  CARMA_ITEM(invalid, SDK_INVALID)                                     \
  CARMA_ITEM(invalid_argument, SDK_INVALID_ARGUMENT)                   \
  CARMA_ITEM(ok, SDK_OK)                                               \
  CARMA_ITEM(plugin_missing, SDK_PLUGIN_MISSING)                       \
  CARMA_ITEM(queue_full, SDK_QUEUE_FULL)                               \
  CARMA_ITEM(shutting_down, SDK_SHUTTING_DOWN)

class sdk_status_t final : sst::boxed<SdkStatus, sdk_status_t> {
  using boxed = sst::boxed<SdkStatus, sdk_status_t>;
  friend class sst::boxed<SdkStatus, sdk_status_t>;

  enum class enum_t : std::underlying_type<SdkStatus>::type {
#define CARMA_ITEM(a, b) a = b,
    CARMA_SDK_STATUS_LIST
#undef CARMA_ITEM
  };

public:
  using boxed::boxed;
  using boxed::value;

  constexpr operator enum_t() const noexcept {
    return static_cast<enum_t>(value());
  }

#define CARMA_ITEM(a, b)                                               \
  static constexpr sdk_status_t a() noexcept {                         \
    return sdk_status_t(b);                                            \
  }
  CARMA_SDK_STATUS_LIST
#undef CARMA_ITEM

  std::string name() const;

  friend void to_json(nlohmann::json & dst, sdk_status_t const & src);
};

} // namespace kestrel

#endif // #ifndef KESTREL_SDK_STATUS_T_HPP
