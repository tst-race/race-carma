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
#include <kestrel/carma/local_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/local_config_t.hpp>
//

#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/json/remove_to.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {
namespace carma {

void local_config_t::parse_sk(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (!have_sk_) {
    sst::json::remove_to(src, *sk_, "sk");
    have_sk_ = true;
  }
}

std::vector<unsigned char> const & local_config_t::sk() const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((have_sk_));
  return *sk_;
}

local_config_t &
local_config_t::sk(std::vector<unsigned char> const & src) {
  SST_ASSERT((!moved_from_));
  *sk_ = src;
  have_sk_ = true;
  return *this;
}

} // namespace carma
} // namespace kestrel