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
#include <kestrel/carma/clrmsg_store_entry_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/clrmsg_store_entry_t.hpp>
//

#include <utility>

namespace kestrel {
namespace carma {

clrmsg_store_entry_t::clrmsg_store_entry_t(
    clrmsg_store_entry_t && other) noexcept
    : clrmsg_(std::move(other.clrmsg_)),
      clrmsg_done_(std::move(other.clrmsg_done_)),
      dirty_(true),
      moved_from_(std::move(other.moved_from_)) {
}

} // namespace carma
} // namespace kestrel
