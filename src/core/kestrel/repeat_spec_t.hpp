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

#ifndef KESTREL_REPEAT_SPEC_T_HPP
#define KESTREL_REPEAT_SPEC_T_HPP

#include <sst/catalog/SST_NODISCARD.hpp>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/optional.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/time_spec_t.hpp>

namespace kestrel {

class repeat_spec_t final {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  repeat_spec_t() SST_NOEXCEPT(true) = default;

  ~repeat_spec_t() SST_NOEXCEPT(true) = default;

  repeat_spec_t(repeat_spec_t const &) = default;

  repeat_spec_t & operator=(repeat_spec_t const &) = default;

  repeat_spec_t(repeat_spec_t &&) SST_NOEXCEPT(true) = default;

  repeat_spec_t & operator=(repeat_spec_t &&)
      SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // parse_state_t
  //--------------------------------------------------------------------
  //
  // Keeps track of which parse_*() functions have already been called
  // during a from_json() call.
  //

private:

  struct parse_state_t {
    bool called_parse_count_ = false;
    bool called_parse_delay_ = false;
  };

  //--------------------------------------------------------------------
  // count
  //--------------------------------------------------------------------

private:

  static constexpr long default_count() noexcept {
    return 0L;
  }

  long count_ = default_count();

  void parse_count(json_t & src, parse_state_t & state);

  void unparse_count(json_t & dst) const;

public:

  SST_NODISCARD() long & count();

  SST_NODISCARD() long const & count() const;

  //--------------------------------------------------------------------
  // delay
  //--------------------------------------------------------------------

private:

  sst::optional<time_spec_t> delay_;

  void parse_delay(json_t & src, parse_state_t & state);

  void unparse_delay(json_t & dst) const;

public:

  SST_NODISCARD() time_spec_t const & delay() const;

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, repeat_spec_t & dst);

  friend void to_json(json_t & dst, repeat_spec_t const & src);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_REPEAT_SPEC_T_HPP
