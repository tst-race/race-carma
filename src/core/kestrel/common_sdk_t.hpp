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

#ifndef KESTREL_COMMON_SDK_T_HPP
#define KESTREL_COMMON_SDK_T_HPP

#include <atomic>
#include <cstdint>
#include <exception>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/enable_if_t.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/mono_time_ns.hpp>
#include <sst/catalog/moved_from.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unique_ptr.hpp>
#include <sst/catalog/unix_time_ns.hpp>

#include <ChannelProperties.h>
#include <EncPkg.h>
#include <IRaceSdkCommon.h>
#include <PluginResponse.h>
#include <RaceLog.h>
#include <SdkResponse.h>

#include <kestrel/CARMA_DEFINE_FINAL_WRAPPER.hpp>
#include <kestrel/channel_id_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/logging.hpp>
#include <kestrel/plugin_response_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/race_to_json.hpp>
#include <kestrel/sdk_status_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/tracing_exception_t.hpp>

namespace kestrel {

class common_sdk_t {

  //--------------------------------------------------------------------
  // Moved-from detection
  //--------------------------------------------------------------------

  // TODO: This can be removed, as common_sdk_t is not movable.

protected:

  sst::moved_from moved_from_;

  //--------------------------------------------------------------------
  // Underlying SDK
  //--------------------------------------------------------------------

private:

  IRaceSdkCommon * sdk_ = nullptr;

  IRaceSdkCommon & sdk() noexcept {
    SST_ASSERT(!moved_from_);
    SST_ASSERT(sdk_ != nullptr);
    return *sdk_;
  }

  //--------------------------------------------------------------------
  // plugin_name
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<std::string> plugin_name_;

public:

  std::string const & plugin_name() const noexcept {
    SST_ASSERT(!moved_from_);
    SST_ASSERT(plugin_name_ != nullptr);
    return *plugin_name_;
  }

  //--------------------------------------------------------------------
  // SDK call numbering
  //--------------------------------------------------------------------
  //
  // Technically this doesn't need to be static, but doing so will make
  // all SDK calls have unique IDs across all plugins that use this
  // class, which is nice.
  //

protected:

  static std::atomic_uintmax_t sdk_function_call_id_;

  //--------------------------------------------------------------------
  // Logging
  //--------------------------------------------------------------------
  //
  // log_level_ is initialized to the lowest possible level so that all
  // log messages pass through to sdk_ by default. This is important for
  // messages that occur before log_level_ is set from any config, as if
  // the initial level were any larger, any such messages that are lower
  // than the initial level would never be output.
  //

private:

  sst::unique_ptr<std::atomic<log_level_t>> log_level_{sst::in_place,
                                                       log_level_trace};

protected:

  static std::string
  describe_blob(std::vector<std::uint8_t> const & blob);

public:

  log_level_t log_level() const noexcept {
    SST_ASSERT(!moved_from_);
    SST_ASSERT(log_level_ != nullptr);
    return log_level_->load();
  }

  void log_level(log_level_t const x) noexcept {
    SST_ASSERT(!moved_from_);
    SST_ASSERT(log_level_ != nullptr);
    log_level_->store(x);
  }

  std::pair<bool, RaceLog::LogLevel>
  should_log(log_level_t level, log_level_t sublevel) const noexcept;

  template<class Line>
  bool log(char const * const file,
           char const * const func,
           Line const line,
           RaceLog::LogLevel const level,
           std::string const & message) const noexcept {
    try {
      SST_ASSERT((file != nullptr));
      SST_ASSERT((func != nullptr));
      SST_ASSERT((line > 0));
      std::string trace;
      trace += file != nullptr ? file : "?";
      trace += ":";
      trace += func != nullptr ? func : "?";
      trace += ":";
      sst::to_string(line, std::back_inserter(trace));
      RaceLog::log(level, plugin_name(), message, trace);
      return true;
    } catch (...) {
      return false;
    }
  }

  template<class Line>
  bool log(char const * const file,
           char const * const func,
           Line const line,
           RaceLog::LogLevel const level,
           tracing_event_t tev) const noexcept {
    try {
      SST_ASSERT((file != nullptr));
      SST_ASSERT((func != nullptr));
      SST_ASSERT((line > 0));
      SST_TEV_ADD(tev,
                  "mono_time_ns",
                  sst::to_string(sst::mono_time_ns()));
      SST_TEV_ADD(tev,
                  "unix_time_ns",
                  sst::to_string(sst::unix_time_ns()));
      std::string message = "carma_tracing_event: ";
      json_t const * const json = tev.json();
      if (json == nullptr) {
        message += "{}";
      } else {
        message +=
            json->dump(-1, ' ', true, json_t::error_handler_t::replace);
      }
      message += " js\\on";
      std::string const trace;
      RaceLog::log(level, plugin_name(), message, trace);
      return true;
    } catch (...) {
      return false;
    }
  }

  //--------------------------------------------------------------------

protected:

  explicit common_sdk_t(IRaceSdkCommon & sdk,
                        std::string const & plugin_name)
      : sdk_(&sdk),
        plugin_name_(sst::in_place, plugin_name) {
  }

  common_sdk_t(common_sdk_t const &) = delete;
  common_sdk_t & operator=(common_sdk_t const &) = delete;
  common_sdk_t(common_sdk_t &&) = delete;
  common_sdk_t & operator=(common_sdk_t &&) = delete;
  ~common_sdk_t() noexcept = default;

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

  SdkResponse e_appendFile(std::string const & filepath,
                           std::vector<std::uint8_t> const & data);

  SdkResponse e_asyncError(RaceHandle handle, PluginResponse status);

  std::string e_getActivePersona();

  std::vector<ChannelProperties> e_getAllChannelProperties();

  ChannelProperties
  e_getChannelProperties(std::string const & channelGid);

  RawData e_getEntropy(std::uint32_t numBytes);

  std::vector<std::string> e_listDir(std::string const & directoryPath);

  SdkResponse e_makeDir(std::string const & directoryPath);

  std::vector<std::uint8_t> e_readFile(std::string const & filepath);

  SdkResponse e_removeDir(std::string const & directoryPath);

  SdkResponse e_writeFile(std::string const & filepath,
                          std::vector<std::uint8_t> const & data);

  std::vector<std::string> e_xListDirRecursive(std::string const & dir);

  void e_xMakeDirs(std::string const & path);

  void e_xMakeParentDirs(std::string const & path);

  // TODO: This should be eliminated long term. Don't write any new code
  //       that uses it.
  bool e_xPathExists(std::string const & path);

  // TODO: This should be eliminated long term. Don't write any new code
  //       that uses it.
  bool e_xPathIsDir(std::string const & path);

  // TODO: This should be eliminated long term. Don't write any new code
  //       that uses it.
  void e_xWriteJsonFile(std::string const & filepath,
                        json_t const & json);

  //--------------------------------------------------------------------
  // Input description functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define an input description function
  // i_foo that returns a JSON object that describes the input to foo.
  //

private:

  static json_t i_appendFile(std::string const & filepath,
                             std::vector<std::uint8_t> const & data);

  static json_t i_asyncError(RaceHandle handle, PluginResponse status);

  static json_t i_getActivePersona();

  static json_t i_getAllChannelProperties();

  static json_t i_getChannelProperties(std::string const & channelGid);

  static json_t i_getEntropy(std::uint32_t numBytes);

  static json_t i_listDir(std::string const & directoryPath);

  static json_t i_makeDir(std::string const & directoryPath);

  static json_t i_readFile(std::string const & filepath);

  static json_t i_removeDir(std::string const & directoryPath);

  static json_t i_writeFile(std::string const & filepath,
                            std::vector<std::uint8_t> const & data);

  static json_t i_xListDirRecursive(std::string const & dir);

  static json_t i_xMakeDirs(std::string const & path);

  static json_t i_xMakeParentDirs(std::string const & path);

  static json_t i_xPathExists(std::string const & path);

  static json_t i_xPathIsDir(std::string const & path);

  static json_t i_xWriteJsonFile(std::string const & filepath,
                                 json_t const & json);

  //--------------------------------------------------------------------
  // Output description functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define an output description function
  // o_foo that returns a JSON object that describes the output of foo.
  //

private:

  static json_t o_appendFile(SdkResponse const & return_value,
                             std::string const & filepath,
                             std::vector<std::uint8_t> const & data);

  static json_t o_asyncError(SdkResponse const & return_value,
                             RaceHandle handle,
                             PluginResponse status);

  static json_t o_getActivePersona(std::string const & return_value);

  static json_t o_getAllChannelProperties(
      std::vector<ChannelProperties> const & return_value);

  static json_t
  o_getChannelProperties(ChannelProperties const & return_value,
                         std::string const & channelGid);

  static json_t o_getEntropy(RawData const & return_value,
                             std::uint32_t numBytes);

  static json_t o_listDir(std::vector<std::string> const & return_value,
                          std::string const & directoryPath);

  static json_t o_makeDir(SdkResponse const & return_value,
                          std::string const & directoryPath);

  static json_t
  o_readFile(std::vector<std::uint8_t> const & return_value,
             std::string const & filepath);

  static json_t o_removeDir(SdkResponse const & return_value,
                            std::string const & directoryPath);

  static json_t o_writeFile(SdkResponse const & return_value,
                            std::string const & filepath,
                            std::vector<std::uint8_t> const & data);

  static json_t
  o_xListDirRecursive(std::vector<std::string> const & return_value,
                      std::string const & dir);

  static json_t o_xMakeDirs(std::string const & path);

  static json_t o_xMakeParentDirs(std::string const & path);

  static json_t o_xPathExists(bool return_value,
                              std::string const & path);

  static json_t o_xPathIsDir(bool return_value,
                             std::string const & path);

  static json_t o_xWriteJsonFile(std::string const & filepath,
                                 json_t const & json);

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

  CARMA_DEFINE_FINAL_WRAPPER(appendFile)
  CARMA_DEFINE_FINAL_WRAPPER(asyncError)
  CARMA_DEFINE_FINAL_WRAPPER(getActivePersona)
  CARMA_DEFINE_FINAL_WRAPPER(getAllChannelProperties)
  CARMA_DEFINE_FINAL_WRAPPER(getChannelProperties)
  CARMA_DEFINE_FINAL_WRAPPER(getEntropy)
  CARMA_DEFINE_FINAL_WRAPPER(listDir)
  CARMA_DEFINE_FINAL_WRAPPER(makeDir)
  CARMA_DEFINE_FINAL_WRAPPER(readFile)
  CARMA_DEFINE_FINAL_WRAPPER(removeDir)
  CARMA_DEFINE_FINAL_WRAPPER(writeFile)
  CARMA_DEFINE_FINAL_WRAPPER(xListDirRecursive)
  CARMA_DEFINE_FINAL_WRAPPER(xMakeDirs)
  CARMA_DEFINE_FINAL_WRAPPER(xMakeParentDirs)
  CARMA_DEFINE_FINAL_WRAPPER(xPathExists)
  CARMA_DEFINE_FINAL_WRAPPER(xPathIsDir)
  CARMA_DEFINE_FINAL_WRAPPER(xWriteJsonFile)

  //--------------------------------------------------------------------
  // Unknown ID exceptions
  //--------------------------------------------------------------------

  static std::runtime_error unknown_channel_id(std::string const & id);

  static std::runtime_error unknown_channel_id(channel_id_t const & id);

  static std::runtime_error unknown_link_id(std::string const & id);

  static std::runtime_error unknown_link_id(link_id_t const & id);

  static std::runtime_error
  unknown_connection_id(std::string const & id);

  static std::runtime_error
  unknown_connection_id(connection_id_t const & id);

  //--------------------------------------------------------------------
};

//----------------------------------------------------------------------

extern template bool common_sdk_t::log<decltype(__LINE__)>(
    char const * file,
    char const * func,
    decltype(__LINE__) line,
    RaceLog::LogLevel level,
    std::string const & message) const noexcept;

//----------------------------------------------------------------------

extern template bool common_sdk_t::log<decltype(__LINE__)>(
    char const * file,
    char const * func,
    decltype(__LINE__) line,
    RaceLog::LogLevel level,
    tracing_event_t tev) const noexcept;

//----------------------------------------------------------------------

} // namespace kestrel

#endif // #ifndef KESTREL_COMMON_SDK_T_HPP
