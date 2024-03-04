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
#include <kestrel/repeat_spec_t.hpp>
// Include twice to test idempotence.
#include <kestrel/repeat_spec_t.hpp>
//

#include <utility>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/is_positive.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/json/unknown_key.hpp>
#include <sst/catalog/to_string.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/time_spec_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// count
//----------------------------------------------------------------------

void repeat_spec_t::parse_count(json_t & src, parse_state_t & state) {
  if (state.called_parse_count_) {
    return;
  }
  state.called_parse_count_ = true;
  sst::json::remove_to(src, count_, "count", default_count());
}

void repeat_spec_t::unparse_count(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("count")));
  if (sst::is_positive(count_)) {
    dst["count"] = sst::to_string(count_);
  }
}

long & repeat_spec_t::count() {
  return count_;
}

long const & repeat_spec_t::count() const {
  return count_;
}

//----------------------------------------------------------------------
// delay
//----------------------------------------------------------------------

void repeat_spec_t::parse_delay(json_t & src, parse_state_t & state) {
  if (state.called_parse_delay_) {
    return;
  }
  state.called_parse_delay_ = true;
  sst::json::remove_to(src, delay_, "delay", nullptr);
}

void repeat_spec_t::unparse_delay(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("delay")));
  dst["delay"] = delay();
}

time_spec_t const & repeat_spec_t::delay() const {
  SST_ASSERT((delay_));
  return *delay_;
}

//----------------------------------------------------------------------
// JSON conversions
//----------------------------------------------------------------------

void from_json(json_t src, repeat_spec_t & dst) {

  repeat_spec_t tmp;
  repeat_spec_t::parse_state_t state;

  tmp.parse_count(src, state);
  tmp.parse_delay(src, state);

  SST_ASSERT((state.called_parse_count_));
  SST_ASSERT((state.called_parse_delay_));

  sst::json::unknown_key(src);

  dst = std::move(tmp);

} //

void to_json(json_t & dst, repeat_spec_t const & src) {

  json_t tmp = json_t::object();

  src.unparse_count(tmp);
  src.unparse_delay(tmp);

  dst = std::move(tmp);

} //

//----------------------------------------------------------------------

} // namespace kestrel
