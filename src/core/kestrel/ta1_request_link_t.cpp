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
#include <kestrel/ta1_request_link_t.hpp>
// Include twice to test idempotence.
#include <kestrel/ta1_request_link_t.hpp>

#include <kestrel/psn_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <set>
#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/json/remove_to.hpp>
#include <string>

namespace kestrel {

//----------------------------------------------------------------------
// channels
//----------------------------------------------------------------------

std::set<std::string> const &
ta1_request_link_t::channels(nlohmann::json & src) {
  if (!done_channels_) {
    sst::json::remove_to(src, channels_, "channels");
    done_channels_ = true;
  }
  return channels_;
}

std::set<std::string> const & ta1_request_link_t::channels() const {
  SST_ASSERT((done_channels_));
  return channels_;
}

//----------------------------------------------------------------------
// recipients
//----------------------------------------------------------------------

std::set<psn_t> const &
ta1_request_link_t::recipients(nlohmann::json & src) {
  if (!done_recipients_) {
    sst::json::remove_to(src, recipients_, "recipients");
    done_recipients_ = true;
  }
  return recipients_;
}

std::set<psn_t> const & ta1_request_link_t::recipients() const {
  SST_ASSERT((done_recipients_));
  return recipients_;
}

//----------------------------------------------------------------------
// sender
//----------------------------------------------------------------------

psn_t const & ta1_request_link_t::sender(nlohmann::json & src) {
  if (!done_sender_) {
    sst::json::remove_to(src, sender_, "sender");
    done_sender_ = true;
  }
  return sender_;
}

psn_t const & ta1_request_link_t::sender() const {
  SST_ASSERT((done_sender_));
  return sender_;
}

//----------------------------------------------------------------------

ta1_request_link_t
ta1_request_link_t::from_json(nlohmann::json && src) {
  ta1_request_link_t dst;
  dst.channels(src);
  dst.recipients(src);
  dst.sender(src);
  return dst;
}

ta1_request_link_t
ta1_request_link_t::from_json(nlohmann::json const & src) {
  return from_json(nlohmann::json(src));
}

void to_json(nlohmann::json & dst, ta1_request_link_t const & src) {
  dst = {
      {"channels", src.channels()},
      {"recipients", src.recipients()},
      {"sender", src.sender()},
  };
}

nlohmann::json ta1_request_link_t::to_json() const {
  nlohmann::json dst;
  kestrel::to_json(dst, *this);
  return dst;
}

} // namespace kestrel
