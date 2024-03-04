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

#ifndef KESTREL_SPAN_PARENTS_T_HPP
#define KESTREL_SPAN_PARENTS_T_HPP

#include <kestrel/config.h>

#if CARMA_WITH_MOCK_SDK

namespace kestrel {

struct span_parents_t final {
  template<class... Args>
  span_parents_t(Args &&...) {
  }

  template<class... Args>
  void push_back(Args &&...) {
  }
};

} // namespace kestrel

#else // !CARMA_WITH_MOCK_SDK

#include <kestrel/sdk_span_t.hpp>
#include <functional>
#include <vector>

namespace kestrel {

using span_parents_t =
    std::vector<std::reference_wrapper<sdk_span_t const>>;

} // namespace kestrel

#endif // !CARMA_WITH_MOCK_SDK

#endif // #ifndef KESTREL_SPAN_PARENTS_T_HPP
