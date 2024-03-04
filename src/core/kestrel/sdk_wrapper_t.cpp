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
#include <kestrel/sdk_wrapper_t.hpp>
// Include twice to test idempotence.
#include <kestrel/sdk_wrapper_t.hpp>
//

#include <exception>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// RACE headers
#include <ChannelProperties.h>
#include <EncPkg.h>
#include <IRaceSdkNM.h>
#include <LinkProperties.h>
#include <RaceLog.h>
#include <SdkResponse.h>

#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>

// CARMA headers
#include <kestrel/config.h>
#include <kestrel/old_config_t.hpp>
#include <kestrel/race_to_json.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

void sdk_wrapper_t::set_config(tracing_event_t,
                               old_config_t const & config) {
  log_level(config.loglevel);
}

//----------------------------------------------------------------------
// e_closeConnection
//----------------------------------------------------------------------

SdkResponse
sdk_wrapper_t::e_closeConnection(ConnectionID const & connectionId,
                                 int32_t const timeout) {
  try {
    SdkResponse const r = sdk_->closeConnection(connectionId, timeout);
    if (r.status != SDK_OK) {
      throw std::runtime_error("closeConnection failed");
    }
    return r;
  } catch (std::exception const &) {
    throw;
  } catch (...) {
    throw std::runtime_error("closeConnection failed");
  }
}

//----------------------------------------------------------------------
// e_getPersonasForLink
//----------------------------------------------------------------------

std::vector<std::string>
sdk_wrapper_t::e_getPersonasForLink(std::string const & linkId) {
  try {
    return sdk_->getPersonasForLink(linkId);
  } catch (std::exception const &) {
    throw;
  } catch (...) {
    throw std::runtime_error("getPersonasForLink failed");
  }
}

//----------------------------------------------------------------------
// i_getPersonasForLink
//----------------------------------------------------------------------

nlohmann::json
sdk_wrapper_t::i_getPersonasForLink(std::string const & linkId) {
  return {
      {"linkId", linkId},
  };
}

//----------------------------------------------------------------------
// i_getSupportedChannels
//----------------------------------------------------------------------

nlohmann::json sdk_wrapper_t::i_getSupportedChannels() {
  return nlohmann::json::object();
}

//----------------------------------------------------------------------
// i_openConnection
//----------------------------------------------------------------------

nlohmann::json
sdk_wrapper_t::i_openConnection(LinkType const linkType,
                                LinkID const & linkId,
                                std::string const & linkHints,
                                int32_t const priority,
                                int32_t const sendTimeout,
                                int32_t const timeout) {
  return {
      {"linkType", link_type_t(linkType).to_json()},
      {"linkID", linkId},
      {"linkHints", linkHints},
      {"priority", priority},
      {"sendTimeout", sendTimeout},
      {"timeout", timeout},
  };
}

//----------------------------------------------------------------------
// o_getPersonasForLink
//----------------------------------------------------------------------

nlohmann::json sdk_wrapper_t::o_getPersonasForLink(
    std::vector<std::string> const & rv) {
  return {
      {"return_value", rv},
  };
}

//----------------------------------------------------------------------
// o_getSupportedChannels
//----------------------------------------------------------------------

nlohmann::json sdk_wrapper_t::o_getSupportedChannels(
    std::map<std::string, ChannelProperties> const & rv) {
  nlohmann::json rv_json = nlohmann::json::object();
  for (auto const & kv : rv) {
    rv_json.emplace(kv.first, race_to_json(kv.second));
  }
  return {
      {"return_value", rv_json},
  };
}

//----------------------------------------------------------------------

} // namespace kestrel
