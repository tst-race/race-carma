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
#include <kestrel/common_sdk_t.hpp>
// Include twice to test idempotence.
#include <kestrel/common_sdk_t.hpp>
//

#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <sst/catalog/c_quote.hpp>
#include <sst/catalog/enable_if_t.hpp>
#include <sst/catalog/to_string.hpp>

#include <ChannelProperties.h>
#include <EncPkg.h>
#include <IRaceSdkCommon.h>
#include <PluginResponse.h>
#include <SdkResponse.h>

#include <kestrel/CARMA_DEFINE_INNER_E_FOO.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/plugin_response_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/race_to_json.hpp>
#include <kestrel/sdk_status_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------

std::string
common_sdk_t::describe_blob(std::vector<std::uint8_t> const & blob) {
  decltype(blob.size()) constexpr hard_max = 96;
  decltype(blob.size()) constexpr soft_max = 72;
  std::string s;
  if (blob.size() <= hard_max) {
    s.insert(s.end(), blob.begin(), blob.end());
  } else {
    s.insert(s.end(), blob.begin(), blob.begin() + soft_max / 2);
    auto const k = blob.size() - soft_max;
    s += "...[";
    s += sst::to_string(k);
    s += k > 1 ? "bytes" : "byte";
    s += " omitted]...";
    s.insert(s.end(),
             blob.end() - (soft_max - soft_max / 2),
             blob.end());
  }
  return s;
}

//----------------------------------------------------------------------
// Exception wrapper functions
//----------------------------------------------------------------------

namespace {

using namespace kestrel;

CARMA_DEFINE_INNER_E_FOO(IRaceSdkCommon, asyncError)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkCommon, getActivePersona)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkCommon, getAllChannelProperties)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkCommon, getChannelProperties)
CARMA_DEFINE_INNER_E_FOO(IRaceSdkCommon, getEntropy)

#undef CARMA_DEFINE_INNER_E_FOO

} // namespace

SdkResponse common_sdk_t::e_asyncError(RaceHandle const handle,
                                       PluginResponse const status) {
  return inner_e_asyncError(sdk(), handle, status);
}

std::string common_sdk_t::e_getActivePersona() {
  return inner_e_getActivePersona(sdk());
}

std::vector<ChannelProperties>
common_sdk_t::e_getAllChannelProperties() {
  return inner_e_getAllChannelProperties(sdk());
}

ChannelProperties
common_sdk_t::e_getChannelProperties(std::string const & channelGid) {
  return inner_e_getChannelProperties(sdk(), channelGid);
}

RawData common_sdk_t::e_getEntropy(std::uint32_t const numBytes) {
  return inner_e_getEntropy(sdk(), numBytes);
}

//----------------------------------------------------------------------
// Input description functions
//----------------------------------------------------------------------

nlohmann::json common_sdk_t::i_asyncError(RaceHandle const handle,
                                          PluginResponse const status) {
  return {
      {"handle", race_handle_t(handle)},
      {"status", plugin_response_t(status)},
  };
}

nlohmann::json common_sdk_t::i_getActivePersona() {
  return {};
}

nlohmann::json common_sdk_t::i_getAllChannelProperties() {
  return {};
}

nlohmann::json
common_sdk_t::i_getChannelProperties(std::string const & channelGid) {
  return {
      {"channelGid", channelGid},
  };
}

nlohmann::json
common_sdk_t::i_getEntropy(std::uint32_t const numBytes) {
  return {
      {"numBytes", numBytes},
  };
}

//----------------------------------------------------------------------
// Unknown ID exceptions
//----------------------------------------------------------------------

namespace {

std::runtime_error unknown_id(std::string const & type,
                              std::string const & id) {
  return std::runtime_error("Unknown " + type
                            + " ID: " + sst::c_quote(id));
}

} // namespace

std::runtime_error
common_sdk_t::unknown_channel_id(std::string const & id) {
  return unknown_id("channel", id);
}

std::runtime_error
common_sdk_t::unknown_channel_id(channel_id_t const & id) {
  return unknown_channel_id(id.value());
}

std::runtime_error
common_sdk_t::unknown_link_id(std::string const & id) {
  return unknown_id("link", id);
}

std::runtime_error common_sdk_t::unknown_link_id(link_id_t const & id) {
  return unknown_link_id(id.value());
}

std::runtime_error
common_sdk_t::unknown_connection_id(std::string const & id) {
  return unknown_id("connection", id);
}

std::runtime_error
common_sdk_t::unknown_connection_id(connection_id_t const & id) {
  return unknown_connection_id(id.value());
}
//----------------------------------------------------------------------

} // namespace kestrel
