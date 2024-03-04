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
#include <kestrel/channel_list_t.hpp>
// Include twice to test idempotence.
#include <kestrel/channel_list_t.hpp>

#include <kestrel/channel_list_entry_t.hpp>
#include <map>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/json/get_as.hpp>
#include <string>
#include <utility>
#include <vector>

namespace kestrel {

//----------------------------------------------------------------------
// channels
//----------------------------------------------------------------------

std::map<std::string, channel_list_entry_t> const &
channel_list_t::channels(nlohmann::json & src) {
  if (!done_channels_) {
    for (auto && x :
         sst::json::get_as<std::vector<channel_list_entry_t>>(src)) {
      channels_.insert({x.channel_id(), std::move(x)});
    }
    done_channels_ = true;
  }
  return channels_;
}

std::map<std::string, channel_list_entry_t> const &
channel_list_t::channels() const {
  SST_ASSERT((done_channels_));
  return channels_;
}

//----------------------------------------------------------------------

channel_list_t channel_list_t::from_json(nlohmann::json && src) {
  channel_list_t dst;
  dst.channels(src);
  return dst;
}

channel_list_t channel_list_t::from_json(nlohmann::json const & src) {
  return from_json(nlohmann::json(src));
}

void to_json(nlohmann::json & dst, channel_list_t const & src) {
  dst = nlohmann::json::array();
  for (auto const & channel : src.channels()) {
    dst.push_back(channel.second);
  }
}

nlohmann::json channel_list_t::to_json() const {
  nlohmann::json dst;
  kestrel::to_json(dst, *this);
  return dst;
}

} // namespace kestrel
