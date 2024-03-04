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

#ifndef KESTREL_RANGE_CONFIG_T_HPP
#define KESTREL_RANGE_CONFIG_T_HPP

#include <unordered_set>

#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {

struct range_config_t final {
  std::unordered_set<psn_t> clients;
  std::unordered_set<psn_t> servers;
};

void from_json(json_t const & src, range_config_t & dst);

} // namespace kestrel

#endif // #ifndef KESTREL_RANGE_CONFIG_T_HPP
