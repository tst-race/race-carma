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

#include <string>

#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/get_as.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

void from_json(json_t const & src, link_side_t & dst) {

  std::string const x = sst::json::get_as<std::string>(src);

#define KESTREL_ITEM(a, b)                                             \
  if (x == #b) {                                                       \
    dst = link_side_t::a();                                            \
    return;                                                            \
  }

  KESTREL_LINK_SIDE_LIST

#undef KESTREL_ITEM

  throw sst::json::exception::invalid_value(src);

} //

} // namespace kestrel
