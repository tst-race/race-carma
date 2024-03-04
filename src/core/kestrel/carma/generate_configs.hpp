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

#ifndef KESTREL_CARMA_GENERATE_CONFIGS_HPP
#define KESTREL_CARMA_GENERATE_CONFIGS_HPP

#include <list>
#include <string>

namespace kestrel {
namespace carma {

void generate_configs(std::list<std::string> argv);

void generate_configs(int const argc, char ** argv);

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_GENERATE_CONFIGS_HPP
