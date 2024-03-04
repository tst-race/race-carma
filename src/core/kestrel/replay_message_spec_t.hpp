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

#ifndef KESTREL_REPLAY_MESSAGE_SPEC_T_HPP
#define KESTREL_REPLAY_MESSAGE_SPEC_T_HPP

#include <string>

#include <sst/catalog/SST_DEFINE_COMPARISONS.hpp>
#include <sst/catalog/SST_NODISCARD.hpp>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/repeat_spec_t.hpp>
#include <kestrel/time_spec_t.hpp>

namespace kestrel {

class replay_message_spec_t final {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  replay_message_spec_t() SST_NOEXCEPT(true) = default;

  ~replay_message_spec_t() SST_NOEXCEPT(true) = default;

  replay_message_spec_t(replay_message_spec_t const &) = default;

  replay_message_spec_t &
  operator=(replay_message_spec_t const &) = default;

  replay_message_spec_t(replay_message_spec_t &&)
      SST_NOEXCEPT(true) = default;

  replay_message_spec_t & operator=(replay_message_spec_t &&)
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
    bool called_parse_content_ = false;
    bool called_parse_receiver_ = false;
    bool called_parse_repeat_ = false;
    bool called_parse_sender_ = false;
    bool called_parse_time_ = false;
  };

  //--------------------------------------------------------------------
  // content
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<std::string> content_;

  void parse_content(json_t & src, parse_state_t & state);

  void unparse_content(json_t & dst) const;

public:

  SST_NODISCARD() std::string const & content() const;

  //--------------------------------------------------------------------
  // receiver
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<std::string> receiver_slug_;

  sst::unique_ptr<psn_t> receiver_;

  void parse_receiver(json_t & src, parse_state_t & state);

  void unparse_receiver(json_t & dst) const;

public:

  SST_NODISCARD() std::string const & receiver_slug() const;

  SST_NODISCARD() psn_t const & receiver() const;

  //--------------------------------------------------------------------
  // repeat
  //--------------------------------------------------------------------

private:

  sst::optional<repeat_spec_t> repeat_;

  void parse_repeat(json_t & src, parse_state_t & state);

  void unparse_repeat(json_t & dst) const;

public:

  SST_NODISCARD() sst::optional<repeat_spec_t> & repeat();

  SST_NODISCARD() sst::optional<repeat_spec_t> const & repeat() const;

  //--------------------------------------------------------------------
  // sender
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<std::string> sender_slug_;

  sst::unique_ptr<psn_t> sender_;

  void parse_sender(json_t & src, parse_state_t & state);

  void unparse_sender(json_t & dst) const;

public:

  SST_NODISCARD() std::string const & sender_slug() const;

  SST_NODISCARD() psn_t const & sender() const;

  //--------------------------------------------------------------------
  // time
  //--------------------------------------------------------------------

private:

  sst::optional<time_spec_t> time_;

  void parse_time(json_t & src, parse_state_t & state);

  void unparse_time(json_t & dst) const;

public:

  SST_NODISCARD() time_spec_t & time();

  SST_NODISCARD() time_spec_t const & time() const;

  //--------------------------------------------------------------------
  // Comparisons
  //--------------------------------------------------------------------

public:

  int compare(replay_message_spec_t const & other) const {
    return this->time().compare(other.time());
  }

  SST_DEFINE_COMPARISONS(replay_message_spec_t);

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, replay_message_spec_t & dst);

  friend void to_json(json_t & dst, replay_message_spec_t const & src);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_REPLAY_MESSAGE_SPEC_T_HPP
