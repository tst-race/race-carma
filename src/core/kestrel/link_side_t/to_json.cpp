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
#include <kestrel/link_side_t.hpp>
// Include twice to test idempotence.
#include <kestrel/link_side_t.hpp>
//

#include <sst/catalog/SST_UNREACHABLE.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

void to_json(json_t & dst, link_side_t const & src) {

#define KESTREL_ITEM(a, b)                                             \
  case b:                                                              \
    dst = #b;                                                          \
    return;

  switch (src.value()) { KESTREL_LINK_SIDE_LIST }

#undef KESTREL_ITEM

  SST_UNREACHABLE();

} //

} // namespace kestrel
