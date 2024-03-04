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

#ifndef KESTREL_CARMA_PHONEBOOK_PAIR_LT_T_HPP
#define KESTREL_CARMA_PHONEBOOK_PAIR_LT_T_HPP

#include <sst/catalog/SST_ASSERT.h>

#include <kestrel/carma/phonebook_pair_t.hpp>

namespace kestrel {
namespace carma {

class phonebook_pair_lt_t final {

public:

  bool operator()(phonebook_pair_t const & a,
                  phonebook_pair_t const & b) const {
    return a.first < b.first;
  }

  bool operator()(phonebook_pair_t const * const & a,
                  phonebook_pair_t const * const & b) const {
    SST_ASSERT((a));
    SST_ASSERT((b));
    return operator()(*a, *b);
  }

}; //

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_PHONEBOOK_PAIR_LT_T_HPP
