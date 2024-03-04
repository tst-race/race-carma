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

#ifndef KESTREL_PLUGIN_STATUS_T_HPP
#define KESTREL_PLUGIN_STATUS_T_HPP

#include <PluginStatus.h>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <sst/catalog/boxed.hpp>
#include <type_traits>

namespace kestrel {

#define CARMA_PLUGIN_STATUS_LIST                                       \
  CARMA_ITEM(undef, PLUGIN_UNDEF)                                      \
  CARMA_ITEM(not_ready, PLUGIN_NOT_READY)                              \
  CARMA_ITEM(ready, PLUGIN_READY)

class plugin_status_t final
    : sst::boxed<PluginStatus, plugin_status_t> {
  using boxed = sst::boxed<PluginStatus, plugin_status_t>;
  friend class sst::boxed<PluginStatus, plugin_status_t>;

  enum class enum_t : std::underlying_type<PluginStatus>::type {
#define CARMA_ITEM(a, b) a = b,
    CARMA_PLUGIN_STATUS_LIST
#undef CARMA_ITEM
  };

public:
  using boxed::boxed;
  using boxed::value;

  constexpr operator enum_t() const noexcept {
    return static_cast<enum_t>(value());
  }

#define CARMA_ITEM(a, b)                                               \
  static constexpr plugin_status_t a() noexcept {                      \
    return plugin_status_t(b);                                         \
  }
  CARMA_PLUGIN_STATUS_LIST
#undef CARMA_ITEM

  friend void to_json(nlohmann::json & dst,
                      plugin_status_t const & src);
};

} // namespace kestrel

#endif // #ifndef KESTREL_PLUGIN_STATUS_T_HPP
