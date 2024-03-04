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

#ifndef KESTREL_CONFIG_COMPILE_HPP
#define KESTREL_CONFIG_COMPILE_HPP

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <kestrel/carma/config_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/range_config_t.hpp>
#include <kestrel/stack_config_t.hpp>

namespace kestrel {

struct genesis_rejection {};

std::map<std::string, json_t> config_compile(
    std::string const & stack_config_file,
    carma::config_t & unified,
    std::string const & range_config_file,
    stack_config_t const & stack_config,
    range_config_t const & range_config,
    std::set<std::string> const & dynamic_only_nodes,
    std::map<psn_t, carma::config_t> & configs2,
    std::string const & dir,
    unsigned int num_threads,
    std::unordered_map<psn_t, std::unordered_set<psn_t>> & tx_nodes);

} // namespace kestrel

#endif // #ifndef KESTREL_CONFIG_COMPILE_HPP
