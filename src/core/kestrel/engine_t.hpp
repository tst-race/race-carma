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

#ifndef KESTREL_ENGINE_T_HPP
#define KESTREL_ENGINE_T_HPP

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/is_negative.hpp>
#include <sst/catalog/is_zero.hpp>
#include <sst/catalog/unix_time_us_t.hpp>

#include <ClrMsg.h>
#include <EncPkg.h>
#include <IRaceSdkNM.h>
#include <IRaceSdkComms.h>
#include <SdkResponse.h>

#include <kestrel/carma/plugin_t.hpp>
#include <kestrel/engine_config_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/rabbitmq/plugin_t.hpp>
#include <kestrel/rabbitmq_management/plugin_t.hpp>
#include <kestrel/race_handle_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
//
// Some functions are declared identically in both IRaceSdkNM and
// IRaceSdkComms (they should have probably been in IRaceSdkCommon). When
// we're implementing both of these interfaces in one class, this is OK
// as long as we're OK with having one implementation that will be used
// for both functions. This is probably the case, but we'll work around
// it anyway to keep our options open. We'll define a few intermediate
// classes that will split each colliding function into two functions:
// one with "Ta1" at the end of its name, and the other with "Ta2" at
// the end of its name. We can always just call one function from the
// other if we want identical behavior.
//

namespace guts {
namespace engine_t {

class my_IRaceSdkNM : public IRaceSdkNM {

public:

  virtual ~my_IRaceSdkNM() = default;

  virtual SdkResponse
  displayInfoToUserTa1(std::string const & data,
                       RaceEnums::UserDisplayType displayType) = 0;

  virtual SdkResponse
  requestCommonUserInputTa1(std::string const & key) = 0;

  virtual SdkResponse
  requestPluginUserInputTa1(std::string const & key,
                            std::string const & prompt,
                            bool cache) = 0;

  SdkResponse displayInfoToUser(
      std::string const & data,
      RaceEnums::UserDisplayType const displayType) final {
    return displayInfoToUserTa1(data, displayType);
  }

  SdkResponse requestCommonUserInput(std::string const & key) final {
    return requestCommonUserInputTa1(key);
  }

  SdkResponse requestPluginUserInput(std::string const & key,
                                     std::string const & prompt,
                                     bool const cache) final {
    return requestPluginUserInputTa1(key, prompt, cache);
  }

}; //

class my_IRaceSdkComms : public IRaceSdkComms {

public:

  virtual ~my_IRaceSdkComms() = default;

  virtual SdkResponse
  displayInfoToUserTa2(std::string const & data,
                       RaceEnums::UserDisplayType displayType) = 0;

  virtual SdkResponse
  requestCommonUserInputTa2(std::string const & key) = 0;

  virtual SdkResponse
  requestPluginUserInputTa2(std::string const & key,
                            std::string const & prompt,
                            bool cache) = 0;

  SdkResponse displayInfoToUser(
      std::string const & data,
      RaceEnums::UserDisplayType const displayType) final {
    return displayInfoToUserTa2(data, displayType);
  }

  SdkResponse requestCommonUserInput(std::string const & key) final {
    return requestCommonUserInputTa2(key);
  }

  SdkResponse requestPluginUserInput(std::string const & key,
                                     std::string const & prompt,
                                     bool const cache) final {
    return requestPluginUserInputTa2(key, prompt, cache);
  }

}; //

} // namespace engine_t
} // namespace guts

//----------------------------------------------------------------------
// engine_t
//----------------------------------------------------------------------

class engine_t final : public guts::engine_t::my_IRaceSdkNM,
                       public guts::engine_t::my_IRaceSdkComms {

  //--------------------------------------------------------------------
  // Primary mutexing
  //--------------------------------------------------------------------
  //
  // primary_mutex_ is the mutex under which all "primary calls" into
  // the engine are synchronized. Primary calls should finish quickly,
  // performing any long-running work in other threads. Primary calls
  // have unrestricted access to all "primary data".
  //
  // All engine calls and data are primary unless otherwise noted. In
  // particular, most calls into the engine through the IRaceSdkTa*
  // functions are primary.
  //

private:

  std::recursive_mutex primary_mutex_;

  std::unique_lock<std::recursive_mutex> primary_lock() {
    return std::unique_lock<std::recursive_mutex>(primary_mutex_);
  }

  //--------------------------------------------------------------------
  // Handle generation
  //--------------------------------------------------------------------
  //
  // Technically this doesn't need to be static, but doing so will make
  // all handles unique across all instances of this class, which is
  // nice.
  //

private:

  static std::atomic_uintmax_t next_handle_;

  static RaceHandle create_RaceHandle();

  //--------------------------------------------------------------------

private:

  std::mutex task_mutex_;
  std::atomic<long long> task_count_{0};
  std::condition_variable task_condition_;

  void increment_task_count() noexcept {
    ++this->task_count_;
    this->task_condition_.notify_one();
  }

  void decrement_task_count() noexcept {
    auto const n = --this->task_count_;
    SST_ASSERT((!sst::is_negative(n)));
  }

  //--------------------------------------------------------------------
  //
  // We always throw exceptions for errors, so for our functions that
  // must return an SdkResponse, we always set the status to SDK_OK. We
  // also don't have a notion of queueUtilization, so we always set that
  // to 0. It's only the handle that we're sometimes interested in.
  //

private:

  static SdkResponse create_SdkResponse(
      RaceHandle const handle = NULL_RACE_HANDLE) noexcept {
    return {SDK_OK, 0, handle};
  }

  //--------------------------------------------------------------------

  template<class Timeout>
  void expect_asynchronous(Timeout && timeout) {
    if (!sst::is_zero(timeout)) {
      throw std::runtime_error(
          "This engine only supports asynchronous calls");
    }
  }

  //--------------------------------------------------------------------

private:

  std::string const data_dir_;
  std::string const config_file_;
  std::string const fs_dir_;

  engine_config_t config_;

#ifdef __EMSCRIPTEN__
  using rmq_plugin_t = rabbitmq_management::plugin_t;
#else
  using rmq_plugin_t = rabbitmq::plugin_t;
#endif

  carma::plugin_t carma_;
  rmq_plugin_t rabbitmq_management_;

  //--------------------------------------------------------------------

  // TODO: We might need to define non-templated interfaces for our
  //       plugin classes that has all the features we want. E.g.,
  //       getting channel properties from a channel is not possible
  //       with IRacePluginComms, which is a problem. Maybe we can just
  //       wrap those interfaces and add a few functions. The issue here
  //       is that we want to keep our plugins in a map, but we can't
  //       because the current types are template-type-heterogeneous.

  // TODO: Eventually this will return a reference to the interface type
  //       discussed above.
  rmq_plugin_t & expect_ta2_plugin(std::string const & id) {
    /*
    if (id != "kestrel-rabbitmq_management") {
      throw common_sdk_t::unknown_channel_id(id);
    }
    */
    return rabbitmq_management_;
  }

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

public:

  explicit engine_t(std::string const & data_dir);

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  engine_t() = delete;

  ~engine_t() SST_NOEXCEPT(true) = default;

  engine_t(engine_t const &) = delete;

  engine_t & operator=(engine_t const &) = delete;

  engine_t(engine_t &&) = delete;

  engine_t & operator=(engine_t &&) = delete;

  //--------------------------------------------------------------------
  // IRaceSdkCommon functions
  //--------------------------------------------------------------------

public:

  SdkResponse appendFile(std::string const & filepath,
                         std::vector<std::uint8_t> const & data) final;

  SdkResponse asyncError(RaceHandle handle,
                         PluginResponse status) final;

  std::string getActivePersona() final;

  std::vector<ChannelProperties> getAllChannelProperties() final;

  ChannelProperties getChannelProperties(std::string channelGid) final;

  RawData getEntropy(std::uint32_t numBytes) final;

  //--------------------------------------------------------------------
  // activateChannel
  //--------------------------------------------------------------------

private:

  struct activateChannel_task_t {
    rmq_plugin_t & ta2_plugin;
    RaceHandle handle;
    std::string channelGid;
    std::string roleName;
  };

  std::list<activateChannel_task_t> activateChannel_tasks_;

public:

  SdkResponse activateChannel(std::string channelGid,
                              std::string roleName,
                              std::int32_t timeout) final;

  //--------------------------------------------------------------------
  // config
  //--------------------------------------------------------------------

public:

  engine_config_t const & config() const SST_NOEXCEPT(true) {
    return this->config_;
  }

  //--------------------------------------------------------------------
  // createLinkFromAddress
  //--------------------------------------------------------------------

private:

  struct createLinkFromAddress_task_t {
    rmq_plugin_t & plugin;
    RaceHandle handle;
    std::string channelGid;
    std::string linkAddress;
    std::vector<std::string> personas;
  };

  std::list<createLinkFromAddress_task_t> createLinkFromAddress_tasks_;

public:

  SdkResponse createLinkFromAddress(std::string channelGid,
                                    std::string linkAddress,
                                    std::vector<std::string> personas,
                                    std::int32_t timeout) final;

  //--------------------------------------------------------------------
  // generateConnectionId
  //--------------------------------------------------------------------

private:

  std::atomic<unsigned long> connection_id_count_{0};

  // TODO: We need some kind of pruning process for this map, otherwise
  //       it will only ever increase in size.
  std::map<ConnectionID, LinkID> connection_id_to_link_id_;

public:

  ConnectionID generateConnectionId(LinkID linkId) final;

  //--------------------------------------------------------------------
  // generateLinkId
  //--------------------------------------------------------------------

private:

  std::atomic<unsigned long> link_id_count_{0};

public:

  LinkID generateLinkId(std::string channelGid) final;

  //--------------------------------------------------------------------
  // getLinkProperties
  //--------------------------------------------------------------------

public:

  LinkProperties getLinkProperties(LinkID linkId) final;

  //--------------------------------------------------------------------
  // getLinksForPersonas
  //--------------------------------------------------------------------

public:

  std::vector<LinkID>
  getLinksForPersonas(std::vector<std::string> recipientPersonas,
                      LinkType linkType) final;

  //--------------------------------------------------------------------
  // listDir
  //--------------------------------------------------------------------
  //
  // Returns the list of children of directoryPath if it exists as a
  // directory, or an empty vector if not (including if directoryPath
  // exists as a non-directory). Throws an exception upon error.
  //

public:

  std::vector<std::string>
  listDir(std::string const & directoryPath) final;

  //--------------------------------------------------------------------
  // loadLinkAddress
  //--------------------------------------------------------------------

private:

  struct loadLinkAddress_task_t {
    rmq_plugin_t & plugin;
    RaceHandle handle;
    std::string channelGid;
    std::string linkAddress;
    std::vector<std::string> personas;
  };

  std::list<loadLinkAddress_task_t> loadLinkAddress_tasks_;

public:

  SdkResponse loadLinkAddress(std::string channelGid,
                              std::string linkAddress,
                              std::vector<std::string> personas,
                              std::int32_t timeout) final;

  //--------------------------------------------------------------------
  // makeDir
  //--------------------------------------------------------------------
  //
  // Ensures that directoryPath exists as a directory by creating any
  // necessary components. Throws an exception upon error.
  //

public:

  SdkResponse makeDir(std::string const & directoryPath) final;

  //--------------------------------------------------------------------
  // onChannelStatusChanged
  //--------------------------------------------------------------------

private:

  struct onChannelStatusChanged_task_t {
    carma::plugin_t & plugin;
    RaceHandle handle;
    std::string channelGid;
    ChannelStatus status;
    ChannelProperties properties;
  };

  std::list<onChannelStatusChanged_task_t>
      onChannelStatusChanged_tasks_;

public:

  SdkResponse onChannelStatusChanged(RaceHandle handle,
                                     std::string channelGid,
                                     ChannelStatus status,
                                     ChannelProperties properties,
                                     int32_t timeout) final;

  //--------------------------------------------------------------------
  // onConnectionStatusChanged
  //--------------------------------------------------------------------

private:

  struct onConnectionStatusChanged_task_t {
    carma::plugin_t & plugin;
    RaceHandle handle;
    ConnectionID connId;
    ConnectionStatus status;
    LinkID linkId;
    LinkProperties properties;
  };

  std::list<onConnectionStatusChanged_task_t>
      onConnectionStatusChanged_tasks_;

public:

  SdkResponse onConnectionStatusChanged(RaceHandle handle,
                                        ConnectionID connId,
                                        ConnectionStatus status,
                                        LinkProperties properties,
                                        int32_t timeout) final;

  //--------------------------------------------------------------------
  // onLinkStatusChanged
  //--------------------------------------------------------------------

private:

  struct onLinkStatusChanged_task_t {
    carma::plugin_t & plugin;
    RaceHandle handle;
    LinkID linkId;
    LinkStatus status;
    LinkProperties properties;
  };

  std::list<onLinkStatusChanged_task_t> onLinkStatusChanged_tasks_;

  // TODO: We need some kind of pruning process for this map, otherwise
  //       it will only ever increase in size.
  std::map<LinkID, LinkProperties> link_id_to_link_properties_;

public:

  SdkResponse onLinkStatusChanged(RaceHandle handle,
                                  LinkID linkId,
                                  LinkStatus status,
                                  LinkProperties properties,
                                  int32_t timeout) final;

  //--------------------------------------------------------------------
  // onMessageStatusChanged
  //--------------------------------------------------------------------

public:

  SdkResponse onMessageStatusChanged(RaceHandle handle,
                                     MessageStatus status) final;

  //--------------------------------------------------------------------
  // onPackageStatusChanged
  //--------------------------------------------------------------------

private:

  struct onPackageStatusChanged_task_t {
    carma::plugin_t & plugin;
    RaceHandle handle;
    PackageStatus status;
  };

  std::list<onPackageStatusChanged_task_t>
      onPackageStatusChanged_tasks_;

public:

  SdkResponse onPackageStatusChanged(RaceHandle handle,
                                     PackageStatus status,
                                     int32_t timeout) final;

  //--------------------------------------------------------------------
  // onPluginStatusChanged
  //--------------------------------------------------------------------

public:

  SdkResponse onPluginStatusChanged(PluginStatus pluginStatus) final;

  //--------------------------------------------------------------------
  // openConnection
  //--------------------------------------------------------------------

private:

  struct openConnection_task_t {
    rmq_plugin_t & plugin;
    RaceHandle handle;
    LinkType linkType;
    LinkID linkId;
    std::string linkHints;
    int32_t priority;
    int32_t sendTimeout;
  };

  std::list<openConnection_task_t> openConnection_tasks_;

public:

  SdkResponse openConnection(LinkType linkType,
                             LinkID linkId,
                             std::string linkHints,
                             int32_t priority,
                             int32_t sendTimeout,
                             int32_t timeout) final;

  //--------------------------------------------------------------------
  // presentCleartextMessage
  //--------------------------------------------------------------------

public:

  SdkResponse presentCleartextMessage(ClrMsg msg) final;

  //--------------------------------------------------------------------
  // readFile
  //--------------------------------------------------------------------
  //
  // Returns the content of filepath if it exists as a file, or an empty
  // vector if not (including if filepath exists as a non-file). Throws
  // an exception upon error.
  //

public:

  std::vector<std::uint8_t>
  readFile(std::string const & filepath) final;

  //--------------------------------------------------------------------
  // receiveEncPkg
  //--------------------------------------------------------------------

private:

  struct receiveEncPkg_task_t {
    carma::plugin_t & plugin;
    RaceHandle handle;
    EncPkg pkg;
    std::vector<ConnectionID> connIDs;
  };

  std::list<receiveEncPkg_task_t> receiveEncPkg_tasks_;

public:

  SdkResponse receiveEncPkg(EncPkg const & pkg,
                            std::vector<ConnectionID> const & connIDs,
                            int32_t timeout) final;

  //--------------------------------------------------------------------
  // removeDir
  //--------------------------------------------------------------------
  //
  // Recursively deletes directoryPath if it exists as a directory, or
  // does nothing if not (including if it exists as a non-directory).
  // Throws an exception upon error.
  //

public:

  SdkResponse removeDir(std::string const & directoryPath) final;

  //--------------------------------------------------------------------
  // sendEncryptedPackage
  //--------------------------------------------------------------------

private:

  struct sendEncryptedPackage_task_t {
    rmq_plugin_t & plugin;
    RaceHandle handle;
    EncPkg ePkg;
    ConnectionID connectionId;
    uint64_t batchId;
  };

  std::list<sendEncryptedPackage_task_t> sendEncryptedPackage_tasks_;

public:

  SdkResponse sendEncryptedPackage(EncPkg ePkg,
                                   ConnectionID connectionId,
                                   uint64_t batchId,
                                   int32_t timeout) final;

  //--------------------------------------------------------------------
  // writeFile
  //--------------------------------------------------------------------
  //
  // Writes data to filepath, overwriting any existing file. Throws an
  // exception upon error.
  //

public:

  SdkResponse writeFile(std::string const & filepath,
                        std::vector<std::uint8_t> const & data) final;

  //--------------------------------------------------------------------

public:

  SdkResponse
  bootstrapDevice(RaceHandle handle,
                  std::vector<std::string> ta2Channels) final;

  SdkResponse bootstrapFailed(RaceHandle handle) final;

  SdkResponse closeConnection(ConnectionID connectionId,
                              int32_t timeout) final;

  SdkResponse createLink(std::string channelGid,
                         std::vector<std::string> personas,
                         std::int32_t timeout) final;

  SdkResponse deactivateChannel(std::string channelGid,
                                std::int32_t timeout) final;

  SdkResponse destroyLink(LinkID linkId, std::int32_t timeout) final;

  SdkResponse
  displayInfoToUserTa1(std::string const & data,
                       RaceEnums::UserDisplayType displayType) final;

  SdkResponse flushChannel(std::string channelGid,
                           uint64_t batchId,
                           int32_t timeout) final;

  LinkID getLinkForConnection(ConnectionID connectionId) final;

  std::vector<LinkID> getLinksForChannel(std::string channelGid) final;

  std::vector<std::string> getPersonasForLink(std::string linkId) final;

  std::map<std::string, ChannelProperties> getSupportedChannels() final;

  SdkResponse loadLinkAddresses(std::string channelGid,
                                std::vector<std::string> linkAddresses,
                                std::vector<std::string> personas,
                                std::int32_t timeout) final;

  SdkResponse requestCommonUserInputTa1(std::string const & key) final;

  SdkResponse requestPluginUserInputTa1(std::string const & key,
                                        std::string const & prompt,
                                        bool cache) final;

  SdkResponse sendBootstrapPkg(ConnectionID connectionId,
                               std::string persona,
                               RawData pkg,
                               int32_t timeout) final;

  SdkResponse
  setPersonasForLink(std::string linkId,
                     std::vector<std::string> personas) final;

  //--------------------------------------------------------------------
  // IRaceSdkComms functions
  //--------------------------------------------------------------------

public:

  SdkResponse displayBootstrapInfoToUser(
      std::string const & data,
      RaceEnums::UserDisplayType displayType,
      RaceEnums::BootstrapActionType actionType) final;

  SdkResponse
  displayInfoToUserTa2(std::string const & data,
                       RaceEnums::UserDisplayType displayType) final;

  SdkResponse requestCommonUserInputTa2(std::string const & key) final;

  SdkResponse requestPluginUserInputTa2(std::string const & key,
                                        std::string const & prompt,
                                        bool cache) final;

  SdkResponse unblockQueue(ConnectionID connId) final;

  SdkResponse updateLinkProperties(LinkID linkId,
                                   LinkProperties properties,
                                   int32_t timeout) final;

  //--------------------------------------------------------------------
  //
  // start() signals the engine to start in a new thread. It will only
  // throw an exception if it was called twice without an intervening
  // call to stop(), in which case it has no effect. This is a caller
  // error.
  //
  // stop() signals the engine to stop. It will only throw an exception
  // if it was called twice without an intervening call to start(), or
  // if it was called before the very first call to start(), in which
  // case it has no effect. This is a caller error.
  //
  // It is safe to call start() and stop() from multiple threads at the
  // same time, although calling them like this should be a last resort.
  //
  // Before calling start(), you should call on_stop() to register a
  // callback that will be called when the engine stops. The callback
  // will be called exactly once for each successful start() call. It
  // will be called either immediately by start() itself in the current
  // thread, or at a later time by the engine thread. The callback will
  // be passed a pointer to a C string. If the pointer is null, it means
  // stop() was called and the engine stopped gracefully. Otherwise, it
  // means the engine stopped because of an error, and the string holds
  // the error message. You can only register one callback. If you call
  // on_stop() multiple times, it will overwrite the callback. You can
  // also call on_stop({}) to unregister the callback.
  //
  // It is safe to call start() inside your on_stop() callback, as the
  // engine will be in a startable state. However, it is possible that
  // this can cause infinite recursion if there is an error with the
  // threading system. In this case, start() may immediately call the
  // callback again, which may then call start() again, and so on. To
  // resolve this, start() will always begin these error messages with
  // "Critical threading error". You can check for this and handle it
  // accordingly.
  //

private:

  std::atomic<int> running_{0};
  std::future<void> future_{};
  std::function<void(char const *)> on_stop_{};

  void tick();
  void run();
  void fire_on_stop(char const * error) noexcept;

public:

  void start();
  void stop();
  void on_stop(std::function<void(char const *)> const & f);

  //--------------------------------------------------------------------
  // wait()
  //--------------------------------------------------------------------
  //
  // Waits for the engine to stop.
  //
  // If any call to this function overlaps any call to the start()
  // function, the behavior is undefined.
  //
  // If any call to this function overlaps any other call to this
  // function, the behavior is undefined.
  //
  // If the engine is not running, this function will return
  // immediately.
  //
  // If the engine was running and stopped because of an exception, this
  // function will rethrow that exception.
  //

  void wait();

  //--------------------------------------------------------------------
  // processClrMsg
  //--------------------------------------------------------------------

private:

  struct processClrMsg_task_t {
    carma::plugin_t & plugin;
    RaceHandle handle;
    std::string psn;
    std::string msg;
    sst::unix_time_us_t unix_time_us;
  };

  std::list<processClrMsg_task_t> processClrMsg_tasks_;

public:

  void processClrMsg(std::string psn, std::string message);

  //--------------------------------------------------------------------
  // on_recv_clrmsg
  //--------------------------------------------------------------------

public:

  struct on_recv_clrmsg_info_t {
    std::string psn;
    std::string msg;
    sst::unix_time_us_t unix_time_us;
  };

  using on_recv_clrmsg_t =
      std::function<void(on_recv_clrmsg_info_t const &)>;

  void on_recv_clrmsg(on_recv_clrmsg_t const & f) {
    on_recv_clrmsg_ = f;
  }

private:

  on_recv_clrmsg_t on_recv_clrmsg_;

  //--------------------------------------------------------------------
  // on_start
  //--------------------------------------------------------------------

public:

  struct on_start_info_t {
    std::string psn_slug;
  };

  using on_start_t = std::function<void(on_start_info_t const &)>;

  void on_start(on_start_t const & f) {
    on_start_ = f;
  }

private:

  on_start_t on_start_;

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_ENGINE_T_HPP
