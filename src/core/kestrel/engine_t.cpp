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
#include <kestrel/engine_t.hpp>
// Include twice to test idempotence.
#include <kestrel/engine_t.hpp>
//

#include <chrono>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_THROW_UNIMPLEMENTED.hpp>
#include <sst/catalog/c_quote.hpp>
#include <sst/catalog/checked_resize.hpp>
#include <sst/catalog/crypto_rng.hpp>
#include <sst/catalog/dir_it.hpp>
#include <sst/catalog/is_positive.hpp>
#include <sst/catalog/json/set_from_file.hpp>
#include <sst/catalog/mkdir_p.hpp>
#include <sst/catalog/read_whole_file.hpp>
#include <sst/catalog/rm_f_r.hpp>
#include <sst/catalog/test_d.hpp>
#include <sst/catalog/test_f.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unix_time_us.hpp>
#include <sst/catalog/what.hpp>
#include <sst/catalog/write_whole_file.hpp>

#include <EncPkg.h>
#include <RaceLog.h>

#include <kestrel/json_t.hpp>
#include <kestrel/normalize_path.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/slugify.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// Handle generation
//----------------------------------------------------------------------

std::atomic_uintmax_t engine_t::next_handle_{0U};

RaceHandle engine_t::create_RaceHandle() {
  return static_cast<RaceHandle>(next_handle_++);
}

//----------------------------------------------------------------------
// IRaceSdkCommon functions
//----------------------------------------------------------------------

SdkResponse
engine_t::appendFile(std::string const & filepath,
                     std::vector<std::uint8_t> const & data) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse engine_t::asyncError(RaceHandle handle,
                                 PluginResponse status) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

std::string engine_t::getActivePersona() {
  auto const primary_lock = this->primary_lock();
  return this->config_.psn()->value();
}

std::vector<ChannelProperties> engine_t::getAllChannelProperties() {
  auto const primary_lock = this->primary_lock();
  return {rabbitmq_management_.properties()};
}

ChannelProperties
engine_t::getChannelProperties(std::string channelGid) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

RawData engine_t::getEntropy(std::uint32_t const numBytes) {
  return sst::crypto_rng(numBytes);
}

//----------------------------------------------------------------------
// activateChannel
//----------------------------------------------------------------------

SdkResponse engine_t::activateChannel(std::string channelGid,
                                      std::string roleName,
                                      std::int32_t const timeout) {
  auto const primary_lock = this->primary_lock();
  this->expect_asynchronous(timeout);
  auto & ta2_plugin = this->expect_ta2_plugin(channelGid);
  RaceHandle const handle = this->create_RaceHandle();
  this->activateChannel_tasks_.push_back(
      {ta2_plugin, handle, std::move(channelGid), std::move(roleName)});
  this->increment_task_count();
  return this->create_SdkResponse(handle);
}

//----------------------------------------------------------------------
// createLinkFromAddress
//----------------------------------------------------------------------

SdkResponse
engine_t::createLinkFromAddress(std::string channelGid,
                                std::string linkAddress,
                                std::vector<std::string> personas,
                                std::int32_t const timeout) {
  auto const primary_lock = this->primary_lock();
  this->expect_asynchronous(timeout);
  auto & plugin = this->expect_ta2_plugin(channelGid);
  RaceHandle const handle = this->create_RaceHandle();
  this->createLinkFromAddress_tasks_.push_back({plugin,
                                                handle,
                                                std::move(channelGid),
                                                std::move(linkAddress),
                                                std::move(personas)});
  this->increment_task_count();
  return this->create_SdkResponse(handle);
}

//----------------------------------------------------------------------
// generateConnectionId
//----------------------------------------------------------------------

ConnectionID engine_t::generateConnectionId(LinkID linkId) {
  ConnectionID x = linkId;
  x += ':';
  sst::to_string(this->connection_id_count_++, std::back_inserter(x));
  this->connection_id_to_link_id_[x] = std::move(linkId);
  return x;
}

//----------------------------------------------------------------------
// generateLinkId
//----------------------------------------------------------------------

LinkID engine_t::generateLinkId(std::string channelGid) {
  channelGid += ':';
  sst::to_string(this->link_id_count_++,
                 std::back_inserter(channelGid));
  return channelGid;
}

//----------------------------------------------------------------------
// getLinkProperties
//----------------------------------------------------------------------

LinkProperties engine_t::getLinkProperties(LinkID const linkId) {
  auto const primary_lock = this->primary_lock();
  tracing_event_t SST_TEV_DEF(tev);
  return this->link_id_to_link_properties_.at(linkId);
}

//----------------------------------------------------------------------
// getLinksForPersonas
//----------------------------------------------------------------------

std::vector<LinkID>
engine_t::getLinksForPersonas(std::vector<std::string>, LinkType) {
  return {};
}

//----------------------------------------------------------------------
// listDir
//----------------------------------------------------------------------

std::vector<std::string>
engine_t::listDir(std::string const & directoryPath) {
  auto const primary_lock = this->primary_lock();
  std::string const x = fs_dir_ + "/" + normalize_path(directoryPath);
  std::vector<std::string> ys;
  if (sst::test_d(x)) {
    sst::dir_it it(x);
    for (; it != it.end(); ++it) {
      ys.emplace_back(it->str().c_str() + it.prefix());
    }
  }
  return ys;
}

//----------------------------------------------------------------------
// loadLinkAddress
//----------------------------------------------------------------------

SdkResponse engine_t::loadLinkAddress(std::string channelGid,
                                      std::string linkAddress,
                                      std::vector<std::string> personas,
                                      std::int32_t const timeout) {
  auto const primary_lock = this->primary_lock();
  this->expect_asynchronous(timeout);
  auto & plugin = this->expect_ta2_plugin(channelGid);
  RaceHandle const handle = this->create_RaceHandle();
  this->loadLinkAddress_tasks_.push_back({plugin,
                                          handle,
                                          std::move(channelGid),
                                          std::move(linkAddress),
                                          std::move(personas)});
  this->increment_task_count();
  return this->create_SdkResponse(handle);
}

//----------------------------------------------------------------------
// makeDir
//----------------------------------------------------------------------

SdkResponse engine_t::makeDir(std::string const & directoryPath) {
  auto const primary_lock = this->primary_lock();
  std::string const x = fs_dir_ + "/" + normalize_path(directoryPath);
  sst::mkdir_p(x);
  return this->create_SdkResponse();
}

//----------------------------------------------------------------------
// onChannelStatusChanged
//----------------------------------------------------------------------

SdkResponse
engine_t::onChannelStatusChanged(RaceHandle handle,
                                 std::string channelGid,
                                 ChannelStatus status,
                                 ChannelProperties properties,
                                 int32_t const timeout) {
  auto const primary_lock = this->primary_lock();
  this->expect_asynchronous(timeout);
  auto & plugin = this->carma_;
  this->onChannelStatusChanged_tasks_.push_back(
      {plugin,
       std::move(handle),
       std::move(channelGid),
       std::move(status),
       std::move(properties)});
  this->increment_task_count();
  return this->create_SdkResponse();
}

//----------------------------------------------------------------------
// onConnectionStatusChanged
//----------------------------------------------------------------------

SdkResponse
engine_t::onConnectionStatusChanged(RaceHandle handle,
                                    ConnectionID connId,
                                    ConnectionStatus status,
                                    LinkProperties properties,
                                    int32_t const timeout) {
  auto const primary_lock = this->primary_lock();
  this->expect_asynchronous(timeout);
  auto & plugin = this->carma_;
  LinkID linkId = this->connection_id_to_link_id_.at(connId);
  this->onConnectionStatusChanged_tasks_.push_back(
      {plugin,
       std::move(handle),
       std::move(connId),
       std::move(status),
       std::move(linkId),
       std::move(properties)});
  this->increment_task_count();
  return this->create_SdkResponse();
}

//----------------------------------------------------------------------
// onLinkStatusChanged
//----------------------------------------------------------------------

SdkResponse engine_t::onLinkStatusChanged(RaceHandle handle,
                                          LinkID linkId,
                                          LinkStatus status,
                                          LinkProperties properties,
                                          int32_t const timeout) {
  auto const primary_lock = this->primary_lock();
  this->expect_asynchronous(timeout);
  auto & plugin = this->carma_;
  this->link_id_to_link_properties_[linkId] = properties;
  this->onLinkStatusChanged_tasks_.push_back({plugin,
                                              std::move(handle),
                                              std::move(linkId),
                                              std::move(status),
                                              std::move(properties)});
  this->increment_task_count();
  return this->create_SdkResponse();
}

//----------------------------------------------------------------------
// onMessageStatusChanged
//----------------------------------------------------------------------

SdkResponse engine_t::onMessageStatusChanged(RaceHandle,
                                             MessageStatus) {
  return this->create_SdkResponse();
}

//----------------------------------------------------------------------
// onPackageStatusChanged
//----------------------------------------------------------------------

SdkResponse engine_t::onPackageStatusChanged(RaceHandle handle,
                                             PackageStatus status,
                                             int32_t const timeout) {
  auto const primary_lock = this->primary_lock();
  this->expect_asynchronous(timeout);
  auto & plugin = this->carma_;
  this->onPackageStatusChanged_tasks_.push_back(
      {plugin, std::move(handle), std::move(status)});
  this->increment_task_count();
  return this->create_SdkResponse();
}

//----------------------------------------------------------------------
// onPluginStatusChanged
//----------------------------------------------------------------------

SdkResponse engine_t::onPluginStatusChanged(PluginStatus) {
  return this->create_SdkResponse();
}

//----------------------------------------------------------------------
// openConnection
//----------------------------------------------------------------------

SdkResponse engine_t::openConnection(LinkType linkType,
                                     LinkID linkId,
                                     std::string linkHints,
                                     int32_t priority,
                                     int32_t sendTimeout,
                                     int32_t const timeout) {
  auto const primary_lock = this->primary_lock();
  this->expect_asynchronous(timeout);
  auto & plugin = this->rabbitmq_management_;
  RaceHandle const handle = this->create_RaceHandle();
  this->openConnection_tasks_.push_back({plugin,
                                         handle,
                                         std::move(linkType),
                                         std::move(linkId),
                                         std::move(linkHints),
                                         std::move(priority),
                                         std::move(sendTimeout)});
  this->increment_task_count();
  return this->create_SdkResponse(handle);
}

//----------------------------------------------------------------------
// presentCleartextMessage
//----------------------------------------------------------------------

SdkResponse engine_t::presentCleartextMessage(ClrMsg msg) {
  auto const primary_lock = this->primary_lock();
  if (on_recv_clrmsg_) {
    // TODO: Can we populate this under the lock, then do the callback
    //       outside the lock?
    on_recv_clrmsg_info_t info;
    info.psn = msg.getFrom();
    info.msg = msg.getMsg();
    info.unix_time_us =
        static_cast<decltype(info.unix_time_us)>(msg.getTime());
    on_recv_clrmsg_(info);
  }
  return this->create_SdkResponse();
}

//----------------------------------------------------------------------
// processClrMsg
//----------------------------------------------------------------------

void engine_t::processClrMsg(std::string psn, std::string msg) {
  auto const primary_lock = this->primary_lock();
  auto & plugin = this->carma_;
  RaceHandle const handle = this->create_RaceHandle();
  this->processClrMsg_tasks_.push_back({plugin,
                                        std::move(handle),
                                        std::move(psn),
                                        std::move(msg),
                                        sst::unix_time_us()});
  this->increment_task_count();
}

//----------------------------------------------------------------------
// readFile
//----------------------------------------------------------------------

std::vector<std::uint8_t>
engine_t::readFile(std::string const & filepath) {
  auto const primary_lock = this->primary_lock();
  std::string const x = fs_dir_ + "/" + normalize_path(filepath);
  if (sst::test_f(x)) {
    return sst::read_whole_file(x);
  } else {
    return {};
  }
}

//----------------------------------------------------------------------
// receiveEncPkg
//----------------------------------------------------------------------

SdkResponse
engine_t::receiveEncPkg(EncPkg const & pkg,
                        std::vector<ConnectionID> const & connIDs,
                        int32_t const timeout) {
  auto const primary_lock = this->primary_lock();
  this->expect_asynchronous(timeout);
  auto & plugin = this->carma_;
  RaceHandle const handle = this->create_RaceHandle();
  this->receiveEncPkg_tasks_.push_back(
      {plugin, handle, std::move(pkg), std::move(connIDs)});
  this->increment_task_count();
  return this->create_SdkResponse(handle);
}

//----------------------------------------------------------------------
// removeDir
//----------------------------------------------------------------------

SdkResponse engine_t::removeDir(std::string const & directoryPath) {
  auto const primary_lock = this->primary_lock();
  std::string const x = fs_dir_ + "/" + normalize_path(directoryPath);
  if (sst::test_d(x)) {
    sst::rm_f_r(x);
  }
  return this->create_SdkResponse();
}

//----------------------------------------------------------------------
// sendEncryptedPackage
//----------------------------------------------------------------------

SdkResponse engine_t::sendEncryptedPackage(EncPkg ePkg,
                                           ConnectionID connectionId,
                                           uint64_t batchId,
                                           int32_t const timeout) {
  auto const primary_lock = this->primary_lock();
  this->expect_asynchronous(timeout);
  auto & plugin = this->rabbitmq_management_;
  RaceHandle const handle = this->create_RaceHandle();
  this->sendEncryptedPackage_tasks_.push_back({plugin,
                                               handle,
                                               std::move(ePkg),
                                               std::move(connectionId),
                                               std::move(batchId)});
  this->increment_task_count();
  return this->create_SdkResponse(handle);
}

//----------------------------------------------------------------------
// writeFile
//----------------------------------------------------------------------

SdkResponse
engine_t::writeFile(std::string const & filepath,
                    std::vector<std::uint8_t> const & data) {
  auto const primary_lock = this->primary_lock();
  std::string const x = fs_dir_ + "/" + normalize_path(filepath);
  sst::write_whole_file(data, x);
  return this->create_SdkResponse();
}

//----------------------------------------------------------------------

SdkResponse
engine_t::bootstrapDevice(RaceHandle handle,
                          std::vector<std::string> ta2Channels) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse engine_t::bootstrapFailed(RaceHandle handle) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse engine_t::closeConnection(ConnectionID connectionId,
                                      int32_t timeout) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse engine_t::createLink(std::string channelGid,
                                 std::vector<std::string> personas,
                                 std::int32_t timeout) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse engine_t::deactivateChannel(std::string channelGid,
                                        std::int32_t timeout) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse engine_t::destroyLink(LinkID linkId, std::int32_t timeout) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse
engine_t::displayInfoToUserTa1(std::string const & data,
                               RaceEnums::UserDisplayType displayType) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse engine_t::flushChannel(std::string channelGid,
                                   uint64_t batchId,
                                   int32_t timeout) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

LinkID engine_t::getLinkForConnection(ConnectionID connectionId) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

std::vector<LinkID>
engine_t::getLinksForChannel(std::string channelGid) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

std::vector<std::string>
engine_t::getPersonasForLink(std::string linkId) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

std::map<std::string, ChannelProperties>
engine_t::getSupportedChannels() {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse
engine_t::loadLinkAddresses(std::string channelGid,
                            std::vector<std::string> linkAddresses,
                            std::vector<std::string> personas,
                            std::int32_t timeout) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse
engine_t::requestCommonUserInputTa1(std::string const & key) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse
engine_t::requestPluginUserInputTa1(std::string const & key,
                                    std::string const & prompt,
                                    bool cache) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse engine_t::sendBootstrapPkg(ConnectionID connectionId,
                                       std::string persona,
                                       RawData pkg,
                                       int32_t timeout) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse
engine_t::setPersonasForLink(std::string linkId,
                             std::vector<std::string> personas) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// IRaceSdkComms functions
//----------------------------------------------------------------------

SdkResponse engine_t::displayBootstrapInfoToUser(
    std::string const & data,
    RaceEnums::UserDisplayType displayType,
    RaceEnums::BootstrapActionType actionType) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse
engine_t::displayInfoToUserTa2(std::string const & data,
                               RaceEnums::UserDisplayType displayType) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse
engine_t::requestCommonUserInputTa2(std::string const & key) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse
engine_t::requestPluginUserInputTa2(std::string const & key,
                                    std::string const & prompt,
                                    bool cache) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse engine_t::unblockQueue(ConnectionID connId) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

SdkResponse engine_t::updateLinkProperties(LinkID linkId,
                                           LinkProperties properties,
                                           int32_t timeout) {
  auto const primary_lock = this->primary_lock();
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------

engine_t::engine_t(std::string const & data_dir)
    : data_dir_(data_dir),
      config_file_(data_dir_ + "/engine.json"),
      fs_dir_(data_dir_ + "/plugins/carma/fs"),
      carma_(*this),
      rabbitmq_management_(*this) {
}

//----------------------------------------------------------------------
// tick
//----------------------------------------------------------------------

void engine_t::tick() {

  //--------------------------------------------------------------------

  SST_ASSERT((sst::is_positive(this->task_count_.load())));

  long long local_task_count;

  //--------------------------------------------------------------------
  // Move all pending tasks into local storage
  //--------------------------------------------------------------------

#define F(G)                                                           \
  G(activateChannel_tasks);                                            \
  G(createLinkFromAddress_tasks);                                      \
  G(loadLinkAddress_tasks);                                            \
  G(onChannelStatusChanged_tasks);                                     \
  G(onConnectionStatusChanged_tasks);                                  \
  G(onLinkStatusChanged_tasks);                                        \
  G(onPackageStatusChanged_tasks);                                     \
  G(openConnection_tasks);                                             \
  G(processClrMsg_tasks);                                              \
  G(receiveEncPkg_tasks);                                              \
  G(sendEncryptedPackage_tasks);                                       \
  class semicolon

#define G1(X) decltype(this->X##_) X

#define G2(X)                                                          \
  do {                                                                 \
    X = std::move(this->X##_);                                         \
    this->X##_.clear();                                                \
  } while (false)

  F(G1);
  {
    auto const primary_lock = this->primary_lock();
    F(G2);
    local_task_count = this->task_count_.load();
  }

#undef G2
#undef G1
#undef F

  //--------------------------------------------------------------------
  // activateChannel
  //--------------------------------------------------------------------

  while (!activateChannel_tasks.empty()) {
    auto & task = activateChannel_tasks.front();
    // TODO: What if this call returns an error or throws?
    task.ta2_plugin.activateChannel(std::move(task.handle),
                                    std::move(task.channelGid),
                                    std::move(task.roleName));
    activateChannel_tasks.pop_front();
    this->decrement_task_count();
    --local_task_count;
  }

  //--------------------------------------------------------------------
  // createLinkFromAddress
  //--------------------------------------------------------------------

  // TODO: We're supposed to use task.personas to keep track of a
  //       mapping from links to persona sets.

  while (!createLinkFromAddress_tasks.empty()) {
    auto & task = createLinkFromAddress_tasks.front();
    // TODO: What if this call returns an error or throws?
    task.plugin.createLinkFromAddress(std::move(task.handle),
                                      std::move(task.channelGid),
                                      std::move(task.linkAddress));
    createLinkFromAddress_tasks.pop_front();
    this->decrement_task_count();
    --local_task_count;
  }

  //--------------------------------------------------------------------
  // loadLinkAddress
  //--------------------------------------------------------------------

  // TODO: We're supposed to use task.personas to keep track of a
  //       mapping from links to persona sets.

  while (!loadLinkAddress_tasks.empty()) {
    auto & task = loadLinkAddress_tasks.front();
    // TODO: What if this call returns an error or throws?
    task.plugin.loadLinkAddress(std::move(task.handle),
                                std::move(task.channelGid),
                                std::move(task.linkAddress));
    loadLinkAddress_tasks.pop_front();
    this->decrement_task_count();
    --local_task_count;
  }

  //--------------------------------------------------------------------
  // onChannelStatusChanged
  //--------------------------------------------------------------------

  while (!onChannelStatusChanged_tasks.empty()) {
    auto & task = onChannelStatusChanged_tasks.front();
    // TODO: What if this call returns an error or throws?
    task.plugin.onChannelStatusChanged(std::move(task.handle),
                                       std::move(task.channelGid),
                                       std::move(task.status),
                                       std::move(task.properties));
    onChannelStatusChanged_tasks.pop_front();
    this->decrement_task_count();
    --local_task_count;
  }

  //--------------------------------------------------------------------
  // onConnectionStatusChanged
  //--------------------------------------------------------------------

  while (!onConnectionStatusChanged_tasks.empty()) {
    auto & task = onConnectionStatusChanged_tasks.front();
    // TODO: What if this call returns an error or throws?
    task.plugin.onConnectionStatusChanged(std::move(task.handle),
                                          std::move(task.connId),
                                          std::move(task.status),
                                          std::move(task.linkId),
                                          std::move(task.properties));
    onConnectionStatusChanged_tasks.pop_front();
    this->decrement_task_count();
    --local_task_count;
  }

  //--------------------------------------------------------------------
  // onLinkStatusChanged
  //--------------------------------------------------------------------

  while (!onLinkStatusChanged_tasks.empty()) {
    auto & task = onLinkStatusChanged_tasks.front();
    // TODO: What if this call returns an error or throws?
    task.plugin.onLinkStatusChanged(std::move(task.handle),
                                    std::move(task.linkId),
                                    std::move(task.status),
                                    std::move(task.properties));
    onLinkStatusChanged_tasks.pop_front();
    this->decrement_task_count();
    --local_task_count;
  }

  //--------------------------------------------------------------------
  // onPackageStatusChanged
  //--------------------------------------------------------------------

  while (!onPackageStatusChanged_tasks.empty()) {
    auto & task = onPackageStatusChanged_tasks.front();
    // TODO: What if this call returns an error or throws?
    task.plugin.onPackageStatusChanged(std::move(task.handle),
                                       std::move(task.status));
    onPackageStatusChanged_tasks.pop_front();
    this->decrement_task_count();
    --local_task_count;
  }

  //--------------------------------------------------------------------
  // openConnection
  //--------------------------------------------------------------------

  while (!openConnection_tasks.empty()) {
    auto & task = openConnection_tasks.front();
    // TODO: What if this call returns an error or throws?
    task.plugin.openConnection(std::move(task.handle),
                               std::move(task.linkType),
                               std::move(task.linkId),
                               std::move(task.linkHints),
                               std::move(task.sendTimeout));
    openConnection_tasks.pop_front();
    this->decrement_task_count();
    --local_task_count;
  }

  //--------------------------------------------------------------------
  // processClrMsg
  //--------------------------------------------------------------------

  while (!processClrMsg_tasks.empty()) {
    auto & task = processClrMsg_tasks.front();
    // TODO: What if this call returns an error or throws?
    task.plugin.processClrMsg(
        std::move(task.handle),
        ClrMsg(std::move(task.msg),
               this->config_.psn()->value(),
               std::move(task.psn),
               static_cast<decltype(std::declval<ClrMsg>().getTime())>(
                   task.unix_time_us),
               0,
               0,
               0,
               0));
    processClrMsg_tasks.pop_front();
    this->decrement_task_count();
    --local_task_count;
  }

  //--------------------------------------------------------------------
  // receiveEncPkg
  //--------------------------------------------------------------------

  while (!receiveEncPkg_tasks.empty()) {
    auto & task = receiveEncPkg_tasks.front();
    // TODO: What if this call returns an error or throws?
    task.plugin.processEncPkg(std::move(task.handle),
                              std::move(task.pkg),
                              std::move(task.connIDs));
    receiveEncPkg_tasks.pop_front();
    this->decrement_task_count();
    --local_task_count;
  }

  //--------------------------------------------------------------------
  // sendEncryptedPackage
  //--------------------------------------------------------------------

  while (!sendEncryptedPackage_tasks.empty()) {
    auto & task = sendEncryptedPackage_tasks.front();
    // TODO: What if this call returns an error or throws?
    task.plugin.sendPackage(std::move(task.handle),
                            std::move(task.connectionId),
                            std::move(task.ePkg),
                            9999999999999999.0,
                            std::move(task.batchId));
    sendEncryptedPackage_tasks.pop_front();
    this->decrement_task_count();
    --local_task_count;
  }

  //--------------------------------------------------------------------

  SST_ASSERT((local_task_count == 0));

  //--------------------------------------------------------------------
}

void engine_t::run() {

  sst::json::set_from_file<json_t>(config_file_, config_);

  PluginConfig plugin_config{};
  plugin_config.etcDirectory = "";
  plugin_config.loggingDirectory = "";
  plugin_config.auxDataDirectory = "";
  plugin_config.tmpDirectory = "";
  plugin_config.pluginDirectory = "";

  {
    std::string const plug = "carma";
    PluginResponse const r = carma_.init(plugin_config);
    if (r != PLUGIN_OK) {
      throw std::runtime_error("Error initializing plugin "
                               + sst::c_quote(plug));
    }
  }

  {
    std::string const plug = "kestrel-rabbitmq_management";
    PluginResponse const r = rabbitmq_management_.init(plugin_config);
    if (r != PLUGIN_OK) {
      throw std::runtime_error("Error initializing plugin "
                               + sst::c_quote(plug));
    }
  }

  if (on_start_) {
    on_start_info_t info;
    info.psn_slug = slugify(this->config_.psn()->value());
    on_start_(info);
  }

  {
    std::unique_lock<std::mutex> lock{this->task_mutex_};
    while (running_.load() == 1) {
      tick();
      this->task_condition_.wait(lock, [&] {
        return this->task_count_.load() > 0;
      });
    }
  }

} //

void engine_t::fire_on_stop(char const * const error) noexcept {
  std::function<void(char const *)> f;
  try {
    f = on_stop_;
  } catch (...) {
  }
  running_.store(0);
  try {
    if (f) {
      f(error);
    }
  } catch (...) {
  }
}

void engine_t::start() {

  // Not sure where to put this or what to do about it.
  RaceLog::setLogLevel(RaceLog::LL_ERROR);

  int x = 0;
  if (!running_.compare_exchange_strong(x, 1)) {
    throw std::logic_error("The engine is already started");
  }
  try {
    try {
      this->wait();
    } catch (...) {
    }
    future_ = std::async(std::launch::async, [&] {
      try {
        run();
        fire_on_stop(nullptr);
      } catch (...) {
        fire_on_stop(sst::what());
        throw;
      }
    });
  } catch (...) {
    try {
      std::string const s = "Critical threading error: " + sst::what();
      fire_on_stop(s.c_str());
    } catch (...) {
      fire_on_stop("Critical threading error.");
    }
  }
}

void engine_t::stop() {
  int x = 1;
  if (!running_.compare_exchange_strong(x, 2)) {
    throw std::logic_error("The engine is already stopped");
  }
  auto const primary_lock = this->primary_lock();
  this->increment_task_count();
}

void engine_t::on_stop(std::function<void(char const *)> const & f) {
  on_stop_ = f;
}

void engine_t::wait() {
  if (this->future_.valid()) {
    this->future_.get();
  }
}

//----------------------------------------------------------------------

} // namespace kestrel
