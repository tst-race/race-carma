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
#include <kestrel/ta1_request_t.hpp>
// Include twice to test idempotence.
#include <kestrel/ta1_request_t.hpp>

#include <algorithm>
#include <kestrel/channel_list_entry_t.hpp>
#include <kestrel/connection_type_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/ta1_request_link_t.hpp>
#include <exception>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <set>
#include <sst/catalog/json/remove_as.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace kestrel {

bool ta1_request_t::contains(
    psn_t const & sender,
    std::string const & channel,
    std::set<psn_t> const & recipients) const {
  auto const it1 = entries_.find(sender);
  if (it1 != entries_.end()) {
    auto const it2 = it1->second.find(channel);
    if (it2 != it1->second.end()) {
      auto const it3 = it2->second.find(recipients);
      if (it3 != it2->second.end()) {
        return true;
      }
    }
  }
  return false;
}

bool ta1_request_t::connects(
    psn_t const & sender,
    psn_t const & recipient,
    channel_list_t const & channel_list,
    std::initializer_list<connection_type_t> connection_types) const {
  auto const it1 = entries_.find(sender);
  if (it1 != entries_.end()) {
    for (auto const & it2 : it1->second) {
      auto const it3 = channel_list.channels().find(it2.first);
      if (it3 != channel_list.channels().end()
          && std::find(connection_types.begin(),
                       connection_types.end(),
                       it3->second.connection_type())
                 != connection_types.end()) {
        for (auto const & recipients : it2.second) {
          if (recipients.find(recipient) != recipients.end()) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

void ta1_request_t::add(psn_t const & sender,
                        std::string const & channel,
                        std::set<psn_t> const & recipients,
                        ta1_request_t const * const filter) {
  if (filter == nullptr
      || filter->contains(sender, channel, recipients)) {
    entries_[sender][channel].insert(recipients);
  }
}

void ta1_request_t::add(psn_t const & sender,
                        std::string const & channel,
                        std::set<psn_t> && recipients,
                        ta1_request_t const * const filter) {
  if (filter == nullptr
      || filter->contains(sender, channel, recipients)) {
    entries_[sender][channel].insert(std::move(recipients));
  }
}

void ta1_request_t::add(psn_t const & sender,
                        std::string const & channel,
                        psn_t const & recipient,
                        ta1_request_t const * const filter) {
  add(sender, channel, &recipient, &recipient + 1, filter);
}

void ta1_request_t::add(
    psn_t const & sender,
    std::set<psn_t> const & recipients,
    channel_list_t const & channel_list,
    std::initializer_list<connection_type_t> connection_types,
    ta1_request_t const * const filter) {
  for (auto const & kv : channel_list.channels()) {
    auto const & channel = kv.second;
    if (std::find(connection_types.begin(),
                  connection_types.end(),
                  channel.connection_type())
        != connection_types.end()) {
      add(sender, channel.channel_id(), recipients, filter);
    }
  }
}

void ta1_request_t::add(
    psn_t const & sender,
    psn_t const & recipient,
    channel_list_t const & channel_list,
    std::initializer_list<connection_type_t> connection_types,
    ta1_request_t const * const filter) {
  add(sender,
      &recipient,
      &recipient + 1,
      channel_list,
      connection_types,
      filter);
}

ta1_request_t ta1_request_t::from_json(nlohmann::json && src) {
  ta1_request_t dst;
  for (auto && link :
       sst::json::remove_as<std::vector<ta1_request_link_t>>(src,
                                                             "links")) {
    for (auto const & channel : link.channels()) {
      dst.add(link.sender(), channel, link.recipients());
    }
  }
  return dst;
}

ta1_request_t ta1_request_t::from_json(nlohmann::json const & src) {
  return from_json(nlohmann::json(src));
}

void to_json(nlohmann::json & dst, ta1_request_t const & src) {
  dst = nlohmann::json::object();
  auto & links = dst["links"] = nlohmann::json::array();
  for (auto const & it1 : src.entries_) {
    auto const & sender = it1.first;
    for (auto const & it2 : it1.second) {
      auto const & channel = it2.first;
      for (auto const & recipients : it2.second) {
        links.push_back({
            {"channels", {channel}},
            {"details", {}},
            {"groupId", nullptr},
            {"recipients", recipients},
            {"sender", sender},
        });
      }
    }
  }
}

} // namespace kestrel
