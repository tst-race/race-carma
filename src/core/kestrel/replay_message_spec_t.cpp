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
#include <kestrel/replay_message_spec_t.hpp>
// Include twice to test idempotence.
#include <kestrel/replay_message_spec_t.hpp>
//

#include <utility>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/json/unknown_key.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/to_string.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/repeat_spec_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// content
//----------------------------------------------------------------------

void replay_message_spec_t::parse_content(json_t & src,
                                          parse_state_t & state) {
  if (state.called_parse_content_) {
    return;
  }
  state.called_parse_content_ = true;
  sst::json::remove_to(src, content_, "content", nullptr);
}

void replay_message_spec_t::unparse_content(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("content")));
  dst["content"] = content();
}

std::string const & replay_message_spec_t::content() const {
  SST_ASSERT((content_));
  return *content_;
}

//----------------------------------------------------------------------
// receiver
//----------------------------------------------------------------------

void replay_message_spec_t::parse_receiver(json_t & src,
                                           parse_state_t & state) {
  if (state.called_parse_receiver_) {
    return;
  }
  state.called_parse_receiver_ = true;
  sst::json::remove_to(src, receiver_slug_, "receiver", nullptr);
  receiver_.emplace(psn_t::from_path_slug(*receiver_slug_));
}

void replay_message_spec_t::unparse_receiver(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("receiver")));
  dst["receiver"] = receiver_slug();
}

std::string const & replay_message_spec_t::receiver_slug() const {
  SST_ASSERT((receiver_slug_));
  SST_ASSERT((receiver_));
  return *receiver_slug_;
}

psn_t const & replay_message_spec_t::receiver() const {
  SST_ASSERT((receiver_slug_));
  SST_ASSERT((receiver_));
  return *receiver_;
}

//----------------------------------------------------------------------
// repeat
//----------------------------------------------------------------------

void replay_message_spec_t::parse_repeat(json_t & src,
                                         parse_state_t & state) {
  if (state.called_parse_repeat_) {
    return;
  }
  state.called_parse_repeat_ = true;
  sst::json::remove_to(src, repeat_, "repeat");
}

void replay_message_spec_t::unparse_repeat(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("repeat")));
  if (repeat()) {
    dst["repeat"] = *repeat();
  }
}

sst::optional<repeat_spec_t> & replay_message_spec_t::repeat() {
  return repeat_;
}

sst::optional<repeat_spec_t> const &
replay_message_spec_t::repeat() const {
  return repeat_;
}

//----------------------------------------------------------------------
// sender
//----------------------------------------------------------------------

void replay_message_spec_t::parse_sender(json_t & src,
                                         parse_state_t & state) {
  if (state.called_parse_sender_) {
    return;
  }
  state.called_parse_sender_ = true;
  sst::json::remove_to(src, sender_slug_, "sender", nullptr);
  sender_.emplace(psn_t::from_path_slug(*sender_slug_));
}

void replay_message_spec_t::unparse_sender(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("sender")));
  dst["sender"] = sender_slug();
}

std::string const & replay_message_spec_t::sender_slug() const {
  SST_ASSERT((sender_slug_));
  SST_ASSERT((sender_));
  return *sender_slug_;
}

psn_t const & replay_message_spec_t::sender() const {
  SST_ASSERT((sender_slug_));
  SST_ASSERT((sender_));
  return *sender_;
}

//----------------------------------------------------------------------
// time
//----------------------------------------------------------------------

void replay_message_spec_t::parse_time(json_t & src,
                                       parse_state_t & state) {
  if (state.called_parse_time_) {
    return;
  }
  state.called_parse_time_ = true;
  sst::json::remove_to(src, time_, "time", nullptr);
}

void replay_message_spec_t::unparse_time(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("time")));
  dst["time"] = time();
}

time_spec_t & replay_message_spec_t::time() {
  SST_ASSERT((time_));
  return *time_;
}

time_spec_t const & replay_message_spec_t::time() const {
  SST_ASSERT((time_));
  return *time_;
}

//----------------------------------------------------------------------
// JSON conversions
//----------------------------------------------------------------------

void from_json(json_t src, replay_message_spec_t & dst) {

  replay_message_spec_t tmp;
  replay_message_spec_t::parse_state_t state;

  tmp.parse_content(src, state);
  tmp.parse_receiver(src, state);
  tmp.parse_repeat(src, state);
  tmp.parse_sender(src, state);
  tmp.parse_time(src, state);

  SST_ASSERT((state.called_parse_content_));
  SST_ASSERT((state.called_parse_receiver_));
  SST_ASSERT((state.called_parse_repeat_));
  SST_ASSERT((state.called_parse_sender_));
  SST_ASSERT((state.called_parse_time_));

  sst::json::unknown_key(src);

  dst = std::move(tmp);

} //

void to_json(json_t & dst, replay_message_spec_t const & src) {

  json_t tmp = json_t::object();

  src.unparse_content(tmp);
  src.unparse_receiver(tmp);
  src.unparse_repeat(tmp);
  src.unparse_sender(tmp);
  src.unparse_time(tmp);

  dst = std::move(tmp);

} //

//----------------------------------------------------------------------

} // namespace kestrel
