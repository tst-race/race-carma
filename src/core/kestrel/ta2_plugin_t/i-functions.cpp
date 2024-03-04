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
#include <kestrel/ta2_plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/ta2_plugin_t.hpp>
//

#include <cstdint>
#include <string>
#include <vector>

#include <sst/catalog/to_string.hpp>

#include <ChannelProperties.h>
#include <EncPkg.h>
#include <LinkProperties.h>
#include <LinkType.h>
#include <SdkResponse.h>

#include <kestrel/json_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/race_to_json.hpp>

namespace kestrel {

json_t ta2_plugin_t::i_activateChannel(RaceHandle const handle,
                                       std::string const & channelGid,
                                       std::string const & roleName) {
  return {
      {"handle", race_handle_t(handle)},
      {"channelGid", channelGid},
      {"roleName", roleName},
  };
}

json_t
ta2_plugin_t::i_closeConnection(RaceHandle const handle,
                                ConnectionID const & connectionId) {
  return {
      {"handle", race_handle_t(handle)},
      {"connectionId", connectionId},
  };
}

json_t
ta2_plugin_t::i_createBootstrapLink(RaceHandle const handle,
                                    std::string const & channelGid,
                                    std::string const & passphrase) {
  return {
      {"handle", race_handle_t(handle)},
      {"channelGid", channelGid},
      {"passphrase", passphrase},
  };
}

json_t ta2_plugin_t::i_createLink(RaceHandle const handle,
                                  std::string const & channelGid) {
  return {
      {"handle", race_handle_t(handle)},
      {"channelGid", channelGid},
  };
}

json_t
ta2_plugin_t::i_createLinkFromAddress(RaceHandle const handle,
                                      std::string const & channelGid,
                                      std::string const & linkAddress) {
  return {
      {"handle", race_handle_t(handle)},
      {"channelGid", channelGid},
      {"linkAddress", linkAddress},
  };
}

json_t
ta2_plugin_t::i_deactivateChannel(RaceHandle const handle,
                                  std::string const & channelGid) {
  return {
      {"handle", race_handle_t(handle)},
      {"channelGid", channelGid},
  };
}

json_t ta2_plugin_t::i_destroyLink(RaceHandle const handle,
                                   LinkID const & linkId) {
  return {
      {"handle", race_handle_t(handle)},
      {"linkId", linkId},
  };
}

json_t ta2_plugin_t::i_flushChannel(RaceHandle const handle,
                                    std::string const & channelGid,
                                    std::uint64_t const batchId) {
  return {
      {"handle", race_handle_t(handle)},
      {"channelGid", channelGid},
      {"batchId", sst::to_string(batchId)},
  };
}

json_t
ta2_plugin_t::i_loadLinkAddress(RaceHandle const handle,
                                std::string const & channelGid,
                                std::string const & linkAddress) {
  return {
      {"handle", race_handle_t(handle)},
      {"channelGid", channelGid},
      {"linkAddress", linkAddress},
  };
}

json_t ta2_plugin_t::i_loadLinkAddresses(
    RaceHandle const handle,
    std::string const & channelGid,
    std::vector<std::string> const & linkAddresses) {
  return {
      {"handle", race_handle_t(handle)},
      {"channelGid", channelGid},
      {"linkAddresses", linkAddresses},
  };
}

json_t
ta2_plugin_t::i_onUserAcknowledgementReceived(RaceHandle const handle) {
  return {
      {"handle", race_handle_t(handle)},
  };
}

json_t
ta2_plugin_t::i_onUserInputReceived(RaceHandle const handle,
                                    bool const answered,
                                    std::string const & response) {
  return {
      {"handle", race_handle_t(handle)},
      {"answered", answered},
      {"response", response},
  };
}

json_t ta2_plugin_t::i_openConnection(RaceHandle const handle,
                                      LinkType const linkType,
                                      LinkID const & linkId,
                                      std::string const & linkHints,
                                      std::int32_t const sendTimeout) {
  return {
      {"handle", race_handle_t(handle)},
      {"linkType", link_type_t(linkType)},
      {"linkId", linkId},
      {"linkHints", linkHints},
      {"sendTimeout", sst::to_string(sendTimeout)},
  };
}

json_t ta2_plugin_t::i_sendPackage(RaceHandle const handle,
                                   ConnectionID const & connectionId,
                                   EncPkg const & pkg,
                                   double const timeoutTimestamp,
                                   std::uint64_t const batchId) {
  return {
      {"handle", race_handle_t(handle)},
      {"connectionId", connectionId},
      {"pkg", race_to_json(pkg)},
      {"timeoutTimestamp", timeoutTimestamp},
      {"batchId", sst::to_string(batchId)},
  };
}

json_t ta2_plugin_t::i_serveFiles(LinkID const & linkId,
                                  std::string const & path) {
  return {
      {"linkId", linkId},
      {"path", path},
  };
}

json_t
ta2_plugin_t::i_xGetChannelGids(std::vector<std::string> const &) {
  return {};
}

json_t
ta2_plugin_t::i_xGetChannelProperties(std::string const & channelGid,
                                      ChannelProperties const &) {
  return {
      {"channelGid", channelGid},
  };
}

} // namespace kestrel
