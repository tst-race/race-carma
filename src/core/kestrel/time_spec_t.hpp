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

#ifndef KESTREL_TIME_SPEC_T_HPP
#define KESTREL_TIME_SPEC_T_HPP

#include <sst/catalog/SST_DEFINE_COMPARISONS.hpp>
#include <sst/catalog/SST_NODISCARD.hpp>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/unix_time_ns_t.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

class time_spec_t final {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  time_spec_t() SST_NOEXCEPT(true) = default;

  ~time_spec_t() SST_NOEXCEPT(true) = default;

  time_spec_t(time_spec_t const &) = default;

  time_spec_t & operator=(time_spec_t const &) = default;

  time_spec_t(time_spec_t &&) SST_NOEXCEPT(true) = default;

  time_spec_t & operator=(time_spec_t &&) SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Constructors
  //--------------------------------------------------------------------

public:

  time_spec_t(sst::unix_time_ns_t a) SST_NOEXCEPT(true);

  time_spec_t(sst::unix_time_ns_t a, sst::unix_time_ns_t b)
      SST_NOEXCEPT(true);

  //--------------------------------------------------------------------
  // a
  //--------------------------------------------------------------------

private:

  sst::unix_time_ns_t a_{};

public:

  SST_NODISCARD() sst::unix_time_ns_t a() const;

  //--------------------------------------------------------------------
  // b
  //--------------------------------------------------------------------

private:

  sst::optional<sst::unix_time_ns_t> b_;

public:

  SST_NODISCARD() sst::optional<sst::unix_time_ns_t> const & b() const;

  //--------------------------------------------------------------------
  // eval
  //--------------------------------------------------------------------

public:

  SST_NODISCARD() time_spec_t eval() const;

  //--------------------------------------------------------------------
  // Comparisons
  //--------------------------------------------------------------------

public:

  int compare(time_spec_t const & other) const SST_NOEXCEPT(true) {
    return this->a_ < other.a_ ? -1 : this->a_ == other.a_ ? 0 : 1;
  }

  SST_DEFINE_COMPARISONS(time_spec_t);

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t const & src, time_spec_t & dst);

  friend void to_json(json_t & dst, time_spec_t const & src);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_TIME_SPEC_T_HPP
