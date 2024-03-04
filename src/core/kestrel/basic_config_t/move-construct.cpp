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
#include <kestrel/basic_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/basic_config_t.hpp>
//

#include <utility>

namespace kestrel {

basic_config_t::basic_config_t(basic_config_t && other) noexcept
    : dir_(std::move(other.dir_)),
      json_(std::move(other.json_)),
      json_file_(std::move(other.json_file_)),
      moved_from_(std::move(other.moved_from_)),
      sdk_(std::move(other.sdk_)) {
}

} // namespace kestrel
