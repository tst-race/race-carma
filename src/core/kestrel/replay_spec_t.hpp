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

#ifndef KESTREL_REPLAY_SPEC_T_HPP
#define KESTREL_REPLAY_SPEC_T_HPP

#include <set>

#include <sst/catalog/SST_NODISCARD.hpp>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/unique_ptr.hpp>
#include <sst/catalog/unix_time_ns_t.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/replay_message_spec_t.hpp>

namespace kestrel {

class replay_spec_t final {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  replay_spec_t() SST_NOEXCEPT(true) = default;

  ~replay_spec_t() SST_NOEXCEPT(true) = default;

  replay_spec_t(replay_spec_t const &) = default;

  replay_spec_t & operator=(replay_spec_t const &) = default;

  replay_spec_t(replay_spec_t &&) SST_NOEXCEPT(true) = default;

  replay_spec_t & operator=(replay_spec_t &&)
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
    bool called_parse_dawn_ = false;
    bool called_parse_messages_ = false;
    bool called_parse_offset_ = false;
  };

  //--------------------------------------------------------------------
  // dawn
  //--------------------------------------------------------------------

private:

  static constexpr sst::unix_time_ns_t default_dawn()
      SST_NOEXCEPT(true) {
    return static_cast<sst::unix_time_ns_t>(0);
  }

  sst::unix_time_ns_t dawn_ = default_dawn();

  void parse_dawn(json_t & src, parse_state_t & state);

  void unparse_dawn(json_t & dst) const;

public:

  SST_NODISCARD() sst::unix_time_ns_t dawn() const;

  //--------------------------------------------------------------------
  // messages
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<std::multiset<replay_message_spec_t>> messages_;

  void parse_messages(json_t & src, parse_state_t & state);

  void unparse_messages(json_t & dst) const;

public:

  SST_NODISCARD() std::multiset<replay_message_spec_t> & messages();

  SST_NODISCARD()
  std::multiset<replay_message_spec_t> const & messages() const;

  //--------------------------------------------------------------------
  // offset
  //--------------------------------------------------------------------

private:

  static constexpr sst::unix_time_ns_t default_offset()
      SST_NOEXCEPT(true) {
    return static_cast<sst::unix_time_ns_t>(0);
  }

  sst::unix_time_ns_t offset_ = default_offset();

  void parse_offset(json_t & src, parse_state_t & state);

  void unparse_offset(json_t & dst) const;

public:

  SST_NODISCARD() sst::unix_time_ns_t offset() const;

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, replay_spec_t & dst);

  friend void to_json(json_t & dst, replay_spec_t const & src);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_REPLAY_SPEC_T_HPP
