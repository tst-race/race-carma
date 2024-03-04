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
#include <kestrel/sodium.hpp>
// Include twice to test idempotence.
#include <kestrel/sodium.hpp>

#include <mutex>
#include <sodium.h>
#include <sst/catalog/to_string.hpp>
#include <stdexcept>

namespace kestrel {
namespace sodium {

void init_once() {
  static std::once_flag f;
  std::call_once(f, []() {
    auto const s = sodium_init();
    if (s != 0 && s != 1) {
      throw std::runtime_error("sodium_init returned "
                               + sst::to_string(s));
    }
  });
}

} // namespace sodium
} // namespace kestrel
