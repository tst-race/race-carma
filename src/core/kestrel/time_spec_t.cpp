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
#include <kestrel/time_spec_t.hpp>
// Include twice to test idempotence.
#include <kestrel/time_spec_t.hpp>
//

#include <utility>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/get_to.hpp>
#include <sst/catalog/json/types.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/perfect_eq.hpp>
#include <sst/catalog/rand_range.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unix_time_ns_t.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------

time_spec_t::time_spec_t(sst::unix_time_ns_t const a) SST_NOEXCEPT(true)
    : a_(a) {
}

time_spec_t::time_spec_t(sst::unix_time_ns_t const a,
                         sst::unix_time_ns_t const b) SST_NOEXCEPT(true)
    : a_(a),
      b_(b) {
}

//----------------------------------------------------------------------
// a
//----------------------------------------------------------------------

sst::unix_time_ns_t time_spec_t::a() const {
  return a_;
}

//----------------------------------------------------------------------
// b
//----------------------------------------------------------------------

sst::optional<sst::unix_time_ns_t> const & time_spec_t::b() const {
  return b_;
}

//----------------------------------------------------------------------
// eval
//----------------------------------------------------------------------

time_spec_t time_spec_t::eval() const {
  if (!b_) {
    return a_;
  }
  SST_ASSERT((a_ <= *b_));
  return sst::rand_range(a_, *b_);
}

//----------------------------------------------------------------------
// JSON conversions
//----------------------------------------------------------------------

void from_json(json_t const & src, time_spec_t & dst) {

  time_spec_t tmp;

  if (src.is_number_integer() || src.is_number_unsigned()
      || src.is_string()) {
    sst::json::get_to(src, tmp.a_);
    tmp.b_.reset();
  } else if (src.is_array()) {
    if (!sst::perfect_eq(src.size(), 2)) {
      throw sst::json::exception("Array must have size 2");
    }
    tmp.b_.emplace();
    sst::json::get_to(src[0], tmp.a_);
    sst::json::get_to(src[1], *tmp.b_);
    if (tmp.a_ > *tmp.b_) {
      throw sst::json::exception(
          ".[0] must be greater than or equal to .[1]");
    }
  } else {
    throw sst::json::exception::expected(sst::json::types::number()
                                         | sst::json::types::string()
                                         | sst::json::types::array());
  }

  dst = std::move(tmp);

} //

void to_json(json_t & dst, time_spec_t const & src) {

  if (!src.b_ || *src.b_ == src.a_) {
    dst = sst::to_string(src.a_);
  } else {
    dst = {sst::to_string(src.a_), sst::to_string(*src.b_)};
  }

} //

//----------------------------------------------------------------------

} // namespace kestrel
