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
#include <kestrel/race_handle_t.hpp>
// Include twice to test idempotence.
#include <kestrel/race_handle_t.hpp>

// Standard C++ headers
#include <cstddef>
#include <functional>

#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>

#include <SdkResponse.h>

// SST headers
#include <sst/catalog/to_string.hpp>

namespace kestrel {

race_handle_t::race_handle_t(SdkResponse const & response)
    : race_handle_t(response.handle) {
}

void to_json(nlohmann::json & dst, race_handle_t const & src) {
  dst = sst::to_string(src.value());
}

} // namespace kestrel

namespace std {

std::size_t hash<kestrel::race_handle_t>::operator()(
    kestrel::race_handle_t const & src) const {
  return static_cast<std::size_t>(src.value());
}

} // namespace std
