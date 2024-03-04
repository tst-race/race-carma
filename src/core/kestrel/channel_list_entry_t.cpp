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
#include <kestrel/channel_list_entry_t.hpp>
// Include twice to test idempotence.
#include <kestrel/channel_list_entry_t.hpp>

#include <kestrel/connection_type_t.hpp>
#include <kestrel/transmission_type_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/json/remove_to.hpp>
#include <string>

namespace kestrel {

//----------------------------------------------------------------------
// channel_id
//----------------------------------------------------------------------

std::string const &
channel_list_entry_t::channel_id(nlohmann::json & src) {
  if (!done_channel_id_) {
    sst::json::remove_to(src, channel_id_, "channelGid");
    done_channel_id_ = true;
  }
  return channel_id_;
}

std::string const & channel_list_entry_t::channel_id() const {
  SST_ASSERT((done_channel_id_));
  return channel_id_;
}

//----------------------------------------------------------------------
// connection_type
//----------------------------------------------------------------------

connection_type_t
channel_list_entry_t::connection_type(nlohmann::json & src) {
  if (!done_connection_type_) {
    sst::json::remove_to(src, connection_type_, "connectionType");
    done_connection_type_ = true;
  }
  return connection_type_;
}

connection_type_t channel_list_entry_t::connection_type() const {
  SST_ASSERT((done_connection_type_));
  return connection_type_;
}

//----------------------------------------------------------------------
// description
//----------------------------------------------------------------------

std::string const &
channel_list_entry_t::description(nlohmann::json & src) {
  if (!done_description_) {
    sst::json::remove_to(src, description_, "description");
    done_description_ = true;
  }
  return description_;
}

std::string const & channel_list_entry_t::description() const {
  SST_ASSERT((done_description_));
  return description_;
}

//----------------------------------------------------------------------
// reliable
//----------------------------------------------------------------------

bool channel_list_entry_t::reliable(nlohmann::json & src) {
  sst::json::remove_to(src, reliable_, "reliable");
  if (!done_reliable_) {
    done_reliable_ = true;
  }
  return reliable_;
}

bool channel_list_entry_t::reliable() const {
  SST_ASSERT((done_reliable_));
  return reliable_;
}

//----------------------------------------------------------------------
// transmission_type
//----------------------------------------------------------------------

transmission_type_t
channel_list_entry_t::transmission_type(nlohmann::json & src) {
  if (!done_transmission_type_) {
    sst::json::remove_to(src, transmission_type_, "transmissionType");
    done_transmission_type_ = true;
  }
  return transmission_type_;
}

transmission_type_t channel_list_entry_t::transmission_type() const {
  SST_ASSERT((done_transmission_type_));
  return transmission_type_;
}

//----------------------------------------------------------------------

channel_list_entry_t
channel_list_entry_t::from_json(nlohmann::json && src) {
  channel_list_entry_t dst;
  dst.channel_id(src);
  dst.connection_type(src);
  dst.description(src);
  dst.reliable(src);
  dst.transmission_type(src);
  return dst;
}

channel_list_entry_t
channel_list_entry_t::from_json(nlohmann::json const & src) {
  return from_json(nlohmann::json(src));
}

void to_json(nlohmann::json & dst, channel_list_entry_t const & src) {
  dst = {
      {"channelGid", src.channel_id_},
      {"connectionType", src.connection_type_},
      {"description", src.description_},
      {"reliable", src.reliable_},
      {"transmissionType", src.transmission_type_},
  };
}

nlohmann::json channel_list_entry_t::to_json() const {
  nlohmann::json dst;
  kestrel::to_json(dst, *this);
  return dst;
}

} // namespace kestrel
