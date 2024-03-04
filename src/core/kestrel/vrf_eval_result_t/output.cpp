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
#include <kestrel/vrf_eval_result_t.hpp>
// Include twice to test idempotence.
#include <kestrel/vrf_eval_result_t.hpp>
//

#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/json/remove_to.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

void vrf_eval_result_t::parse_output(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (!done_output_) {
    sst::json::remove_to(src, *output_, "output");
    done_output_ = true;
  }
}

std::vector<unsigned char> const & vrf_eval_result_t::output() const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((done_output_));
  return *output_;
}

vrf_eval_result_t &
vrf_eval_result_t::output(std::vector<unsigned char> src) {
  SST_ASSERT((!moved_from_));
  *output_ = std::move(src);
  done_output_ = true;
  return *this;
}

} // namespace kestrel
