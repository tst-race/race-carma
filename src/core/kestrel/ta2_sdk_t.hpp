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

#ifndef KESTREL_TA2_SDK_T_HPP
#define KESTREL_TA2_SDK_T_HPP

#include <cstdint>
#include <string>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>

#include <ChannelProperties.h>
#include <ChannelStatus.h>
#include <ConnectionStatus.h>
#include <EncPkg.h>
#include <IRaceSdkComms.h>
#include <LinkProperties.h>
#include <LinkStatus.h>
#include <PackageStatus.h>
#include <RaceEnums.h>
#include <SdkResponse.h>

#include <kestrel/CARMA_DEFINE_FINAL_WRAPPER.hpp>
#include <kestrel/common_sdk_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {

//
// The ta2_sdk_t class is a wrapper class around the IRaceSdkComms class
// from the RACE SDK that provides enhanced logging.
//

class ta2_sdk_t : public common_sdk_t {

  //--------------------------------------------------------------------
  // Underlying SDK
  //--------------------------------------------------------------------

public:

  using race_sdk_t = IRaceSdkComms;

private:

  IRaceSdkComms * sdk_ = nullptr;

  IRaceSdkComms & sdk() const noexcept {
    SST_ASSERT(!moved_from_);
    SST_ASSERT(sdk_ != nullptr);
    return *sdk_;
  }

  //--------------------------------------------------------------------

public:

  explicit ta2_sdk_t(IRaceSdkComms & sdk, std::string const & plugin_name)
      : common_sdk_t(sdk, plugin_name),
        sdk_(&sdk) {
  }

  ta2_sdk_t(ta2_sdk_t const &) = delete;
  ta2_sdk_t & operator=(ta2_sdk_t const &) = delete;
  ta2_sdk_t(ta2_sdk_t &&) = delete;
  ta2_sdk_t & operator=(ta2_sdk_t &&) = delete;
  ~ta2_sdk_t() noexcept = default;

  //--------------------------------------------------------------------
  // Exception wrapper functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define an exception wrapper function
  // e_foo that translates all errors reported by foo into exceptions.
  // Some errors may already be exceptions, in which case we either
  // rethrow them or translate them to different exceptions.
  //

private:

  SdkResponse e_displayBootstrapInfoToUser(
      std::string const & data,
      RaceEnums::UserDisplayType displayType,
      RaceEnums::BootstrapActionType actionType);

  SdkResponse
  e_displayInfoToUser(std::string const & data,
                      RaceEnums::UserDisplayType displayType);

  ConnectionID e_generateConnectionId(LinkID const & linkId);

  LinkID e_generateLinkId(std::string const & channelGid);

  SdkResponse
  e_onChannelStatusChanged(RaceHandle handle,
                           std::string const & channelGid,
                           ChannelStatus status,
                           ChannelProperties const & properties,
                           std::int32_t timeout);

  SdkResponse
  e_onConnectionStatusChanged(RaceHandle handle,
                              ConnectionID const & connId,
                              ConnectionStatus status,
                              LinkProperties const & properties,
                              std::int32_t timeout);

  SdkResponse e_onLinkStatusChanged(RaceHandle handle,
                                    LinkID const & linkId,
                                    LinkStatus status,
                                    LinkProperties const & properties,
                                    std::int32_t timeout);

  SdkResponse e_onPackageStatusChanged(RaceHandle handle,
                                       PackageStatus status,
                                       std::int32_t timeout);

  SdkResponse e_receiveEncPkg(EncPkg const & pkg,
                              std::vector<ConnectionID> const & connIDs,
                              std::int32_t timeout);

  SdkResponse e_requestCommonUserInput(std::string const & key);

  SdkResponse e_requestPluginUserInput(std::string const & key,
                                       std::string const & prompt,
                                       bool cache);

  SdkResponse e_updateLinkProperties(LinkID const & linkId,
                                     LinkProperties const & properties,
                                     std::int32_t timeout);

  //--------------------------------------------------------------------
  // Input description functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define an input description function
  // i_foo that returns a JSON object that describes the input to foo.
  //

private:

  static json_t i_displayBootstrapInfoToUser(
      std::string const & data,
      RaceEnums::UserDisplayType displayType,
      RaceEnums::BootstrapActionType actionType);

  static json_t
  i_displayInfoToUser(std::string const & data,
                      RaceEnums::UserDisplayType displayType);

  static json_t i_generateConnectionId(LinkID const & linkId);

  static json_t i_generateLinkId(std::string const & channelGid);

  static json_t
  i_onChannelStatusChanged(RaceHandle handle,
                           std::string const & channelGid,
                           ChannelStatus status,
                           ChannelProperties const & properties,
                           std::int32_t timeout);

  static json_t
  i_onConnectionStatusChanged(RaceHandle handle,
                              ConnectionID const & connId,
                              ConnectionStatus status,
                              LinkProperties const & properties,
                              std::int32_t timeout);

  static json_t i_onLinkStatusChanged(RaceHandle handle,
                                      LinkID const & linkId,
                                      LinkStatus status,
                                      LinkProperties const & properties,
                                      std::int32_t timeout);

  static json_t i_onPackageStatusChanged(RaceHandle handle,
                                         PackageStatus status,
                                         std::int32_t timeout);

  static json_t
  i_receiveEncPkg(EncPkg const & pkg,
                  std::vector<ConnectionID> const & connIDs,
                  std::int32_t timeout);

  static json_t i_requestCommonUserInput(std::string const & key);

  static json_t i_requestPluginUserInput(std::string const & key,
                                         std::string const & prompt,
                                         bool cache);

  static json_t
  i_updateLinkProperties(LinkID const & linkId,
                         LinkProperties const & properties,
                         std::int32_t timeout);

  //--------------------------------------------------------------------
  // Output description functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define an output description function
  // o_foo that returns a JSON object that describes the output of foo.
  //

private:

  static json_t o_displayBootstrapInfoToUser(
      SdkResponse const & return_value,
      std::string const & data,
      RaceEnums::UserDisplayType displayType,
      RaceEnums::BootstrapActionType actionType);

  static json_t
  o_displayInfoToUser(SdkResponse const & return_value,
                      std::string const & data,
                      RaceEnums::UserDisplayType displayType);

  static json_t
  o_generateConnectionId(ConnectionID const & return_value,
                         LinkID const & linkId);

  static json_t o_generateLinkId(LinkID const & return_value,
                                 std::string const & channelGid);

  static json_t
  o_onChannelStatusChanged(SdkResponse const & return_value,
                           RaceHandle handle,
                           std::string const & channelGid,
                           ChannelStatus status,
                           ChannelProperties const & properties,
                           std::int32_t timeout);

  static json_t
  o_onConnectionStatusChanged(SdkResponse const & return_value,
                              RaceHandle handle,
                              ConnectionID const & connId,
                              ConnectionStatus status,
                              LinkProperties const & properties,
                              std::int32_t timeout);

  static json_t o_onLinkStatusChanged(SdkResponse const & return_value,
                                      RaceHandle handle,
                                      LinkID const & linkId,
                                      LinkStatus status,
                                      LinkProperties const & properties,
                                      std::int32_t timeout);

  static json_t
  o_onPackageStatusChanged(SdkResponse const & return_value,
                           RaceHandle handle,
                           PackageStatus status,
                           std::int32_t timeout);

  static json_t
  o_receiveEncPkg(SdkResponse const & return_value,
                  EncPkg const & pkg,
                  std::vector<ConnectionID> const & connIDs,
                  std::int32_t timeout);

  static json_t
  o_requestCommonUserInput(SdkResponse const & return_value,
                           std::string const & key);

  static json_t
  o_requestPluginUserInput(SdkResponse const & return_value,
                           std::string const & key,
                           std::string const & prompt,
                           bool cache);

  static json_t
  o_updateLinkProperties(SdkResponse const & return_value,
                         LinkID const & linkId,
                         LinkProperties const & properties,
                         std::int32_t timeout);

  //--------------------------------------------------------------------
  // Final wrapper functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define a final wrapper function foo
  // that additionally takes a tracing_event_t parameter, forwards the
  // call to e_foo, and uses i_foo and o_foo to log information about
  // the call and the result.
  //

public:

  CARMA_DEFINE_FINAL_WRAPPER(displayBootstrapInfoToUser)
  CARMA_DEFINE_FINAL_WRAPPER(displayInfoToUser)
  CARMA_DEFINE_FINAL_WRAPPER(generateConnectionId)
  CARMA_DEFINE_FINAL_WRAPPER(generateLinkId)
  CARMA_DEFINE_FINAL_WRAPPER(onChannelStatusChanged)
  CARMA_DEFINE_FINAL_WRAPPER(onConnectionStatusChanged)
  CARMA_DEFINE_FINAL_WRAPPER(onLinkStatusChanged)
  CARMA_DEFINE_FINAL_WRAPPER(onPackageStatusChanged)
  CARMA_DEFINE_FINAL_WRAPPER(receiveEncPkg)
  CARMA_DEFINE_FINAL_WRAPPER(requestCommonUserInput)
  CARMA_DEFINE_FINAL_WRAPPER(requestPluginUserInput)
  CARMA_DEFINE_FINAL_WRAPPER(updateLinkProperties)

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_TA2_SDK_T_HPP
