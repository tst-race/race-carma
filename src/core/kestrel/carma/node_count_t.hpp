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

#ifndef KESTREL_CARMA_NODE_COUNT_T_HPP
#define KESTREL_CARMA_NODE_COUNT_T_HPP

#include <cstdint>
#include <limits>
#include <type_traits>

#include <sst/catalog/SST_STATIC_ASSERT.h>
#include <sst/catalog/promote_unsigned_t.hpp>
#include <sst/catalog/promotes.hpp>

namespace kestrel {
namespace carma {

using node_count_t = sst::promote_unsigned_t<std::uint_least32_t>;

SST_STATIC_ASSERT(
    (std::is_integral<node_count_t>::value
     && std::is_unsigned<node_count_t>::value
     && !std::is_same<node_count_t, bool>::value),
    "node_count_t must be an unsigned fundamental integer type");

SST_STATIC_ASSERT(
    (!sst::promotes<node_count_t>::value),
    "node_count_t must be unaffected by the integer promotions");

inline constexpr node_count_t node_count_max() noexcept {
  return std::numeric_limits<node_count_t>::max();
}

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_NODE_COUNT_T_HPP
