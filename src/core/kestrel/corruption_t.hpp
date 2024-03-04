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

#ifndef KESTREL_CORRUPTION_T_HPP
#define KESTREL_CORRUPTION_T_HPP

#include <stdexcept>

namespace kestrel {

class corruption_t : public std::runtime_error {

public:

  corruption_t() : std::runtime_error("Corruption detected") {
  }

}; //

} // namespace kestrel

#endif // #ifndef KESTREL_CORRUPTION_T_HPP
