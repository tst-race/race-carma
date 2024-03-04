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

#ifndef KESTREL_CARMA_CONTAINS_HPP
#define KESTREL_CARMA_CONTAINS_HPP

#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_set_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {
namespace carma {

bool contains(phonebook_set_t const & set,
              phonebook_pair_t const & pair);

bool contains(phonebook_set_t const & set, psn_t const & psn);

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_CONTAINS_HPP
