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
#include <kestrel/replay_spec_t.hpp>
// Include twice to test idempotence.
#include <kestrel/replay_spec_t.hpp>
//

#include <set>
#include <utility>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/json/unknown_key.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unix_time_ns_t.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/replay_message_spec_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// dawn
//----------------------------------------------------------------------

void replay_spec_t::parse_dawn(json_t & src, parse_state_t & state) {
  if (state.called_parse_dawn_) {
    return;
  }
  state.called_parse_dawn_ = true;
  sst::json::remove_to(src, dawn_, "dawn", default_dawn());
}

void replay_spec_t::unparse_dawn(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("dawn")));
  dst["dawn"] = sst::to_string(dawn());
}

sst::unix_time_ns_t replay_spec_t::dawn() const {
  return dawn_;
}

//----------------------------------------------------------------------
// messages
//----------------------------------------------------------------------

void replay_spec_t::parse_messages(json_t & src,
                                   parse_state_t & state) {
  if (state.called_parse_messages_) {
    return;
  }
  state.called_parse_messages_ = true;
  sst::json::remove_to(src,
                       messages_.ensure(),
                       "messages",
                       std::multiset<replay_message_spec_t>());
}

void replay_spec_t::unparse_messages(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("messages")));
  dst["messages"] = messages();
}

std::multiset<replay_message_spec_t> & replay_spec_t::messages() {
  SST_ASSERT((messages_));
  return *messages_;
}

std::multiset<replay_message_spec_t> const &
replay_spec_t::messages() const {
  SST_ASSERT((messages_));
  return *messages_;
}

//----------------------------------------------------------------------
// offset
//----------------------------------------------------------------------

void replay_spec_t::parse_offset(json_t & src, parse_state_t & state) {
  if (state.called_parse_offset_) {
    return;
  }
  state.called_parse_offset_ = true;
  sst::json::remove_to(src, offset_, "offset", default_offset());
}

void replay_spec_t::unparse_offset(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("offset")));
  dst["offset"] = sst::to_string(offset());
}

sst::unix_time_ns_t replay_spec_t::offset() const {
  return offset_;
}

//----------------------------------------------------------------------
// JSON conversions
//----------------------------------------------------------------------

void from_json(json_t src, replay_spec_t & dst) {

  replay_spec_t tmp;
  replay_spec_t::parse_state_t state;

  tmp.parse_dawn(src, state);
  tmp.parse_messages(src, state);
  tmp.parse_offset(src, state);

  SST_ASSERT((state.called_parse_dawn_));
  SST_ASSERT((state.called_parse_messages_));
  SST_ASSERT((state.called_parse_offset_));

  sst::json::unknown_key(src);

  dst = std::move(tmp);

} //

void to_json(json_t & dst, replay_spec_t const & src) {

  json_t tmp = json_t::object();

  src.unparse_dawn(tmp);
  src.unparse_messages(tmp);
  src.unparse_offset(tmp);

  dst = std::move(tmp);

} //

//----------------------------------------------------------------------

} // namespace kestrel
