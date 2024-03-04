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

#ifndef CARMA_MOCK_SDK_T_HPP
#define CARMA_MOCK_SDK_T_HPP

#include <atomic>
#include <chrono>
#include <cstdint>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <sst/checked.h>
#include <sst/time.h>

#include <kestrel/json_t.hpp>
#include <kestrel/post_json_t.hpp>

#include <amqp.h>
#include <amqp_tcp_socket.h>

// RACE SDK
#include <ChannelProperties.h>
#include <ClrMsg.h>
#include <EncPkg.h>
#include <IRacePluginNM.h>
#include <IRaceSdkNM.h>
#include <LinkProperties.h>
#include <PluginResponse.h>
#include <RaceEnums.h>
#include <SdkResponse.h>

namespace kestrel {

class mock_sdk_t final : public IRaceSdkNM {

  std::string channel_name_ = "carma_mock";

  std::mutex mutex_;

  RaceHandle race_handle_ = 1;

  //--------------------------------------------------------------------

  struct link_info_t {
    std::set<std::string> personas;
    std::uintmax_t connection_count = 0;
    LinkProperties link_properties;
  };

  std::map<LinkID, link_info_t> link_infos_;
  std::map<ConnectionID, LinkID> connections_;
  std::map<ConnectionID, RaceHandle> opening_connections_;
  std::set<ConnectionID> open_connections_;
  std::map<ConnectionID, RaceHandle> closing_connections_;
  std::map<std::set<std::string>, std::set<LinkID>> links_for_;
  std::map<RaceHandle, LinkID> creating_links_;

  LinkID always_create_link(std::set<std::string> const & personas);

  //--------------------------------------------------------------------

  void sdk_log(std::string const & msg);

  //--------------------------------------------------------------------

  amqp_connection_state_t rmq_connection_ = nullptr;
  amqp_socket_t * rmq_socket_ = nullptr;
  static constexpr amqp_channel_t rmq_channel_ = 1;
  bool rmq_channel_is_open_ = false;
  bool rmq_is_consuming_ = false;

  void rmq_destroy() noexcept;
  void rmq_connect();
  void rmq_queue_declare(std::string const & queue);
  void rmq_basic_publish(std::string const & queue,
                         std::vector<unsigned char> const & body);
  void rmq_basic_consume(std::string const & queue);
  bool rmq_consume_message(std::string const & queue,
                           amqp_envelope_t & envelope);

  //--------------------------------------------------------------------

public:
  std::atomic_bool const & sigint_;

  explicit mock_sdk_t(std::atomic_bool const & sigint,
                      nlohmann::json const & config);

  ~mock_sdk_t();

  //--------------------------------------------------------------------
  // IRaceSdkNM functions
  //--------------------------------------------------------------------
  //
  // How to update these function declarations:
  //
  //    1. Delete all of the function declarations.
  //
  //    2. Copy all pure virtual function declarations from the
  //       IRaceSdkNM and IRaceSdkCommon classes.
  //
  //    3. Delete "virtual" everywhere.
  //
  //    4. Replace "= 0" with "final" everywhere.
  //
  //    5. Replace "const T" with "T const" everywhere for all T.
  //

  RawData getEntropy(std::uint32_t numBytes) final;

  std::string getActivePersona() final;

  SdkResponse asyncError(RaceHandle handle,
                         PluginResponse status) final;

  SdkResponse makeDir(std::string const & directoryPath) final;

  SdkResponse removeDir(std::string const & directoryPath) final;

  std::vector<std::string>
  listDir(std::string const & directoryPath) final;

  std::vector<std::uint8_t>
  readFile(std::string const & filepath) final;

  SdkResponse appendFile(std::string const & filepath,
                         std::vector<std::uint8_t> const & data) final;

  SdkResponse writeFile(std::string const & filepath,
                        std::vector<std::uint8_t> const & data) final;

  SdkResponse onPluginStatusChanged(PluginStatus pluginStatus) final;

  SdkResponse sendEncryptedPackage(EncPkg ePkg,
                                   ConnectionID connectionId,
                                   uint64_t batchId,
                                   int32_t timeout) final;

  SdkResponse presentCleartextMessage(ClrMsg msg) final;

  SdkResponse openConnection(LinkType linkType,
                             LinkID linkId,
                             std::string linkHints,
                             int32_t priority,
                             int32_t sendTimeout,
                             int32_t timeout) final;

  SdkResponse closeConnection(ConnectionID connectionId,
                              int32_t timeout) final;

  std::vector<LinkID>
  getLinksForPersonas(std::vector<std::string> recipientPersonas,
                      LinkType linkType) final;

  LinkID getLinkForConnection(ConnectionID connectionId) final;

  LinkProperties getLinkProperties(LinkID linkId) final;

  std::map<std::string, ChannelProperties> getSupportedChannels() final;

  ChannelProperties getChannelProperties(std::string channelGid) final;

  std::vector<ChannelProperties> getAllChannelProperties() final;

  SdkResponse deactivateChannel(std::string channelGid,
                                std::int32_t timeout) final;

  SdkResponse activateChannel(std::string channelGid,
                              std::string roleName,
                              std::int32_t timeout) final;

  SdkResponse destroyLink(LinkID linkId, std::int32_t timeout) final;

  SdkResponse createLink(std::string channelGid,
                         std::vector<std::string> personas,
                         std::int32_t timeout) final;

  SdkResponse loadLinkAddress(std::string channelGid,
                              std::string linkAddress,
                              std::vector<std::string> personas,
                              std::int32_t timeout) final;

  SdkResponse loadLinkAddresses(std::string channelGid,
                                std::vector<std::string> linkAddresses,
                                std::vector<std::string> personas,
                                std::int32_t timeout) final;

  SdkResponse createLinkFromAddress(std::string channelGid,
                                    std::string linkAddress,
                                    std::vector<std::string> personas,
                                    std::int32_t timeout) final;

  SdkResponse
  bootstrapDevice(RaceHandle handle,
                  std::vector<std::string> ta2Channels) final;

  SdkResponse bootstrapFailed(RaceHandle handle) final;

  SdkResponse
  setPersonasForLink(std::string linkId,
                     std::vector<std::string> personas) final;

  std::vector<std::string> getPersonasForLink(std::string linkId) final;

  SdkResponse onMessageStatusChanged(RaceHandle handle,
                                     MessageStatus status) final;

  SdkResponse sendBootstrapPkg(ConnectionID connectionId,
                               std::string persona,
                               RawData pkg,
                               int32_t timeout) final;

  SdkResponse requestPluginUserInput(std::string const & key,
                                     std::string const & prompt,
                                     bool cache) final;

  SdkResponse requestCommonUserInput(std::string const & key) final;

  SdkResponse flushChannel(std::string channelGid,
                           uint64_t batchId,
                           int32_t timeout) final;

  std::vector<LinkID> getLinksForChannel(std::string channelGid) final;

  SdkResponse
  displayInfoToUser(std::string const & data,
                    RaceEnums::UserDisplayType displayType) final;

  //--------------------------------------------------------------------

  void run(IRacePluginNM & plugin);

  void whisper(std::string const & from,
               std::string const & to,
               std::string const & msg);

private:
  std::chrono::milliseconds work_loop_sleep{100};
  std::chrono::milliseconds rmq_error_sleep{5000};

  std::string const pseudonym;

  std::string const rmq_host;
  int const rmq_port;
  std::string const rmq_username;
  std::string const rmq_password;
  int const rmq_heartbeat;

  sst::checked_t<long long> bandwidth_bps_;
  sst::checked_t<long long> latency_ms_;

  std::string const etcDirectory;
  std::string const pluginConfigFilePath;

  post_json_t post_json;
  bool const has_received_json_post_config;
  std::string const received_json_post_addr;
  int const received_json_post_port;
  std::string const received_json_post_token;

  void postReceivedJson(ClrMsg const & msg) noexcept;

  struct parsed_tag {};
  explicit mock_sdk_t(std::atomic_bool const & sigint,
                      nlohmann::json const & config,
                      parsed_tag);

  void declare_queue(std::string const & queue);

  struct send_queue_entry_t {
    std::vector<unsigned char> blob;
    RaceHandle handle;
  };
  struct send_queue_t {
    std::list<send_queue_entry_t> entries;
    bool idle = true;
    sst::checked_t<decltype(sst::mono_time_ms())> wakeup = 0;
  };
  std::map<std::string, send_queue_t> send_queues_;
};

} // namespace kestrel

#endif // CARMA_MOCK_SDK_T_HPP
