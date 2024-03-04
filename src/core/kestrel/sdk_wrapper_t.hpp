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

#ifndef KESTREL_SDK_WRAPPER_T_HPP
#define KESTREL_SDK_WRAPPER_T_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/json/get_to.hpp>
#include <sst/integer.h>
#include <sst/language.h>
#include <sst/time.h>

#include <kestrel/CARMA_XLOG_DEBUG.hpp>
#include <kestrel/CARMA_XLOG_ERROR.hpp>
#include <kestrel/CARMA_XLOG_FATAL.hpp>
#include <kestrel/CARMA_XLOG_INFO.hpp>
#include <kestrel/CARMA_XLOG_TRACE.hpp>
#include <kestrel/CARMA_XLOG_WARN.hpp>
#include <kestrel/config.h>
#include <kestrel/old_config_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/logging.hpp>
#include <kestrel/message_status_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/plugin_status_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/race_to_json.hpp>
#include <kestrel/ta1_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/tracing_exception_t.hpp>

// RACE SDK
#include <ChannelProperties.h>
#include <ClrMsg.h>
#include <EncPkg.h>
#include <IRaceSdkCommon.h>
#include <IRaceSdkNM.h>
#include <LinkProperties.h>
#include <PluginResponse.h>
#include <PluginStatus.h>
#include <RaceLog.h>
#include <SdkResponse.h>

#if !CARMA_WITH_MOCK_SDK
#include <OpenTracingHelpers.h>

#include <opentracing/tracer.h>
#endif

namespace kestrel {

// TODO: This class (sdk_wrapper_t) will eventually be subsumed by
//       ta1_sdk_t.

// TODO: set_config() should go away completely. This class should never
//       be using a plugin config.

class sdk_wrapper_t final : public ta1_sdk_t {

  //--------------------------------------------------------------------
  // Logging
  //--------------------------------------------------------------------

private:

#define CARMA_LOG_FATAL_SDK(sdk, sublevel, message)                    \
  CARMA_XLOG_FATAL(sdk, sublevel, message)

#define CARMA_LOG_ERROR_SDK(sdk, sublevel, message)                    \
  CARMA_XLOG_ERROR(sdk, sublevel, message)

#define CARMA_LOG_WARN_SDK(sdk, sublevel, message)                     \
  CARMA_XLOG_WARN(sdk, sublevel, message)

#define CARMA_LOG_INFO_SDK(sdk, sublevel, message)                     \
  CARMA_XLOG_INFO(sdk, sublevel, message)

#define CARMA_LOG_DEBUG_SDK(sdk, sublevel, message)                    \
  CARMA_XLOG_DEBUG(sdk, sublevel, message)

#define CARMA_LOG_TRACE_SDK(sdk, sublevel, message)                    \
  CARMA_XLOG_TRACE(sdk, sublevel, message)

  //--------------------------------------------------------------------

private:

  IRaceSdkNM * sdk_;

  //--------------------------------------------------------------------
  // Exception wrapper functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define an exception wrapper function
  // e_foo that translates all errors reported by foo into exceptions.
  // Some errors may already be exceptions, in which case we may
  // translate them to different exceptions.
  //
  // If foo returns void, we'll make e_foo return sst::monostate, which
  // helps simplify the next layer of wrapper functions.
  //

#define CARMA_E_FOO_1(f)                                               \
  template<class... Args>                                              \
  SdkResponse e_##f(Args &&... args) {                                 \
    try {                                                              \
      SdkResponse const r = sdk_->f(std::forward<Args>(args)...);       \
      switch (r.status) {                                              \
        case SDK_OK: {                                                 \
          return r;                                                    \
        } break;                                                       \
        default: {                                                     \
          throw std::runtime_error(#f " failed");                      \
        } break;                                                       \
      }                                                                \
    } catch (std::exception const &) {                                 \
      throw;                                                           \
    } catch (...) {                                                    \
      throw std::runtime_error(#f " failed");                          \
    }                                                                  \
  }

private:
  CARMA_E_FOO_1(activateChannel)

  SdkResponse e_closeConnection(ConnectionID const & connectionId,
                                int32_t const timeout);

  CARMA_E_FOO_1(createLink)
  CARMA_E_FOO_1(createLinkFromAddress)

#if !CARMA_WITH_MOCK_SDK
  template<class... Args>
  std::shared_ptr<opentracing::Tracer> e_createTracer(Args &&... args) {
    try {
      return ::createTracer(std::forward<Args>(args)...);
    } catch (std::exception const &) {
      throw;
    } catch (...) {
      throw std::runtime_error("createTracer failed");
    }
  }
#endif

  template<class T_connectionId>
  LinkID e_getLinkForConnection(T_connectionId && connectionId) {
    try {
      return sdk_->getLinkForConnection(
          std::forward<T_connectionId>(connectionId));
    } catch (std::exception const &) {
      throw;
    } catch (...) {
      throw std::runtime_error("getLinkForConnection failed");
    }
  }

  template<class T_linkId>
  LinkProperties e_getLinkProperties(T_linkId && linkId) {
    try {
      return sdk_->getLinkProperties(std::forward<T_linkId>(linkId));
    } catch (std::exception const &) {
      throw;
    } catch (...) {
      throw std::runtime_error("getLinkProperties failed");
    }
  }

  template<class T_recipientPersonas>
  std::vector<LinkID>
  e_getLinksForPersonas(T_recipientPersonas && recipientPersonas,
                        LinkType const linkType) {
    try {
      return sdk_->getLinksForPersonas(
          std::forward<T_recipientPersonas>(recipientPersonas),
          linkType);
    } catch (std::exception const &) {
      throw;
    } catch (...) {
      throw std::runtime_error("getLinksForPersonas failed");
    }
  }

  std::vector<std::string>
  e_getPersonasForLink(std::string const & linkId);

  std::map<std::string, ChannelProperties> e_getSupportedChannels() {
    try {
      return sdk_->getSupportedChannels();
    } catch (std::exception const &) {
      throw;
    } catch (...) {
      throw std::runtime_error("getSupportedChannels failed");
    }
  }

  CARMA_E_FOO_1(loadLinkAddress)

  CARMA_E_FOO_1(onMessageStatusChanged)
  CARMA_E_FOO_1(onPluginStatusChanged)

  template<class T_linkId, class T_linkHints>
  SdkResponse e_openConnection(LinkType const linkType,
                               T_linkId && linkId,
                               T_linkHints linkHints,
                               int32_t const priority,
                               int32_t const sendTimeout,
                               int32_t const timeout) {
    try {
      SdkResponse const r =
          sdk_->openConnection(linkType,
                              std::forward<T_linkId>(linkId),
                              std::forward<T_linkHints>(linkHints),
                              priority,
                              sendTimeout,
                              timeout);
      switch (r.status) {
        case SDK_OK: {
          return r;
        } break;
        default: {
          throw std::runtime_error("openConnection failed");
        } break;
      }
    } catch (std::exception const &) {
      throw;
    } catch (...) {
      throw std::runtime_error("openConnection failed");
    }
  }

  template<class T_msg>
  SdkResponse e_presentCleartextMessage(T_msg && msg) {
    try {
      SdkResponse const r =
          sdk_->presentCleartextMessage(std::forward<T_msg>(msg));
      switch (r.status) {
        case SDK_OK: {
          return r;
        } break;
        default: {
          throw std::runtime_error("presentCleartextMessage failed");
        } break;
      }
    } catch (std::exception const &) {
      throw;
    } catch (...) {
      throw std::runtime_error("presentCleartextMessage failed");
    }
  }

  template<class T_ePkg, class T_connectionId>
  SdkResponse e_sendEncryptedPackage(T_ePkg && ePkg,
                                     T_connectionId && connectionId,
                                     int32_t const timeout) {
    try {
      SdkResponse const r = sdk_->sendEncryptedPackage(
          std::forward<T_ePkg>(ePkg),
          std::forward<T_connectionId>(connectionId),
          RACE_BATCH_ID_NULL,
          timeout);
      switch (r.status) {
        case SDK_OK: {
          return r;
        } break;
        default: {
          throw std::runtime_error("sendEncryptedPackage failed");
        } break;
      }
    } catch (std::exception const &) {
      throw;
    } catch (...) {
      throw std::runtime_error("sendEncryptedPackage failed");
    }
  }

#undef CARMA_E_FOO_1

  //--------------------------------------------------------------------
  // Input description functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define an input description function
  // i_foo that accepts the same parameters as foo and returns a JSON
  // object that describes the input to foo.
  //

#define CARMA_DEFUN(foo)                                               \
  template<class... Args>                                              \
  nlohmann::json i_##foo(Args &&...) {                                 \
    return nlohmann::json::object();                                   \
  }

  CARMA_DEFUN(closeConnection)
  CARMA_DEFUN(getLinkForConnection)
  CARMA_DEFUN(getLinkProperties)
  CARMA_DEFUN(presentCleartextMessage)
  CARMA_DEFUN(sendEncryptedPackage)

#undef CARMA_DEFUN

  nlohmann::json i_activateChannel(std::string const & channelGid,
                                   std::string const & roleName,
                                   std::int32_t const timeout) {
    return {
        {"channelGid", channelGid},
        {"roleName", roleName},
        {"timeout", timeout},
    };
  }

  nlohmann::json i_createLink(std::string const & channelGid,
                              std::vector<std::string> const & personas,
                              std::int32_t const timeout) {
    return {
        {"channelGid", channelGid},
        {"personas", personas},
        {"timeout", timeout},
    };
  }

  nlohmann::json
  i_createLinkFromAddress(std::string const & channelGid,
                          std::string const & linkAddress,
                          std::vector<std::string> const & personas,
                          std::int32_t const timeout) {
    return {
        {"channelGid", channelGid},
        {"linkAddress", linkAddress},
        {"personas", personas},
        {"timeout", timeout},
    };
  }

#if !CARMA_WITH_MOCK_SDK
  nlohmann::json i_createTracer(std::string const & jaegerConfigPath,
                                std::string const & persona) {
    return {
        {"jaegerConfigPath", jaegerConfigPath},
        {"persona", persona},
    };
  }
#endif

  nlohmann::json i_getLinksForPersonas(
      std::vector<std::string> const & recipientPersonas,
      LinkType const linkType) {
    return {
        {"recipientPersonas", recipientPersonas},
        {"linkType", link_type_t(linkType).to_json()},
    };
  }

  nlohmann::json i_getPersonasForLink(std::string const & linkId);

  nlohmann::json
  i_loadLinkAddress(std::string const & channelGid,
                    std::string const & linkAddress,
                    std::vector<std::string> const & personas,
                    std::int32_t const & timeout) {
    return {
        {"channelGid", channelGid},
        {"linkAddress", linkAddress},
        {"personas", personas},
        {"timeout", timeout},
    };
  }

  nlohmann::json i_getSupportedChannels();

  nlohmann::json i_onMessageStatusChanged(RaceHandle const handle,
                                          MessageStatus const status) {
    return {
        {"handle", race_handle_t(handle)},
        {"status", message_status_t(status)},
    };
  }

  nlohmann::json
  i_onPluginStatusChanged(PluginStatus const pluginStatus) {
    return {
        {"pluginStatus", plugin_status_t(pluginStatus)},
    };
  }

  nlohmann::json i_openConnection(LinkType linkType,
                                  LinkID const & linkId,
                                  std::string const & linkHints,
                                  int32_t priority,
                                  int32_t sendTimeout,
                                  int32_t timeout);

  //--------------------------------------------------------------------
  // Output description functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define an output description function
  // o_foo that accepts the value returned by e_foo and returns a JSON
  // object that describes the output of foo.
  //

#define CARMA_O_FOO_1(f)                                               \
  nlohmann::json o_##f(SdkResponse const & rv) {                       \
    return race_to_json(rv);                                           \
  }

#define CARMA_DEFUN(foo)                                               \
  template<class... Args>                                              \
  nlohmann::json o_##foo(Args &&...) {                                 \
    return nlohmann::json::object();                                   \
  }

  CARMA_O_FOO_1(activateChannel)
  CARMA_O_FOO_1(closeConnection)
  CARMA_O_FOO_1(createLink)
  CARMA_O_FOO_1(createLinkFromAddress)

  CARMA_DEFUN(getLinkForConnection)
  CARMA_DEFUN(getLinkProperties)

  nlohmann::json o_getSupportedChannels(
      std::map<std::string, ChannelProperties> const & rv);

  CARMA_O_FOO_1(loadLinkAddress)
  CARMA_O_FOO_1(onMessageStatusChanged)
  CARMA_O_FOO_1(onPluginStatusChanged)
  CARMA_O_FOO_1(openConnection)
  CARMA_O_FOO_1(presentCleartextMessage)
  CARMA_O_FOO_1(sendEncryptedPackage)

#if !CARMA_WITH_MOCK_SDK
  CARMA_DEFUN(createTracer)
#endif

#undef CARMA_DEFUN

  nlohmann::json o_getLinksForPersonas(std::vector<LinkID> const & rv) {
    return {
        {"return_value", rv},
    };
  }

  nlohmann::json
  o_getPersonasForLink(std::vector<std::string> const & rv);

  //--------------------------------------------------------------------
  // Tracing wrapper functions
  //--------------------------------------------------------------------
  //
  // For each exception wrapper function e_foo, we define a tracing
  // wrapper function foo that additionally takes a tracing_event_t
  // parameter and outputs logging events for every call and return.
  //

public:
#define CARMA_DEFUN(f)                                                 \
  template<class... Args>                                              \
  auto f(tracing_event_t tev, Args &&... args)                         \
      ->decltype(e_##f(std::forward<Args>(args)...)) {                 \
    try {                                                              \
      SST_TEV_ADD(tev, "sdk_function", #f);                            \
      SST_TEV_ADD(tev, "sdk_function_input", i_##f(args...));          \
      CARMA_LOG_TRACE_SDK(                                             \
          *this,                                                       \
          0,                                                           \
          SST_TEV_ARG(tev, "event", "sdk_function_called"));           \
      auto const r = e_##f(std::forward<Args>(args)...);               \
      SST_TEV_ADD(tev, "sdk_function_output", o_##f(r));               \
      CARMA_LOG_TRACE_SDK(                                             \
          *this,                                                       \
          0,                                                           \
          SST_TEV_ARG(tev, "event", "sdk_function_succeeded"));        \
      return r;                                                        \
    } catch (std::exception const & e) {                               \
      CARMA_LOG_ERROR_SDK(*this,                                       \
                          0,                                           \
                          SST_TEV_ARG(tev,                             \
                                      "event",                         \
                                      "sdk_function_failed",           \
                                      "exception",                     \
                                      std::string(e.what())));         \
      std::throw_with_nested(tracing_exception_t(std::move(tev)));     \
    } catch (...) {                                                    \
      CARMA_LOG_ERROR_SDK(*this,                                       \
                          0,                                           \
                          SST_TEV_ARG(tev,                             \
                                      "event",                         \
                                      "sdk_function_failed",           \
                                      "exception",                     \
                                      nullptr));                       \
      std::throw_with_nested(tracing_exception_t(std::move(tev)));     \
    }                                                                  \
  }

  CARMA_DEFUN(activateChannel)
  CARMA_DEFUN(closeConnection)
  CARMA_DEFUN(createLink)
  CARMA_DEFUN(createLinkFromAddress)
  CARMA_DEFUN(getLinkForConnection)
  CARMA_DEFUN(getLinkProperties)
  CARMA_DEFUN(getLinksForPersonas)
  CARMA_DEFUN(getPersonasForLink)
  CARMA_DEFUN(getSupportedChannels)
  CARMA_DEFUN(loadLinkAddress)
  CARMA_DEFUN(onMessageStatusChanged)
  CARMA_DEFUN(onPluginStatusChanged)
  CARMA_DEFUN(openConnection)
  CARMA_DEFUN(presentCleartextMessage)
  CARMA_DEFUN(sendEncryptedPackage)

#if !CARMA_WITH_MOCK_SDK
  CARMA_DEFUN(createTracer)
#endif

#undef CARMA_DEFUN

  //--------------------------------------------------------------------

public:

  sdk_wrapper_t(IRaceSdkNM & sdk, std::string const & plugin_name)
      : ta1_sdk_t(sdk, plugin_name),
        sdk_(&sdk) {
  }

  sdk_wrapper_t(sdk_wrapper_t const &) = delete;
  sdk_wrapper_t & operator=(sdk_wrapper_t const &) = delete;
  sdk_wrapper_t(sdk_wrapper_t &&) = delete;
  sdk_wrapper_t & operator=(sdk_wrapper_t &&) = delete;
  ~sdk_wrapper_t() noexcept = default;

  void set_config(tracing_event_t tev, old_config_t const & config);

  void present(tracing_event_t tev, ClrMsg const & message) {
    presentCleartextMessage(SST_TEV_ARG(tev), message);
  }

#define CARMA_LOG_FATAL(sdk, sublevel, message)                        \
  CARMA_XLOG_FATAL(sdk, sublevel, message)

#define CARMA_LOG_ERROR(sdk, sublevel, message)                        \
  CARMA_XLOG_ERROR(sdk, sublevel, message)

#define CARMA_LOG_WARN(sdk, sublevel, message)                         \
  CARMA_XLOG_WARN(sdk, sublevel, message)

#define CARMA_LOG_INFO(sdk, sublevel, message)                         \
  CARMA_XLOG_INFO(sdk, sublevel, message)

#define CARMA_LOG_DEBUG(sdk, sublevel, message)                        \
  CARMA_XLOG_DEBUG(sdk, sublevel, message)

#define CARMA_LOG_TRACE(sdk, sublevel, message)                        \
  CARMA_XLOG_TRACE(sdk, sublevel, message)
};

} // namespace kestrel

#endif // #ifndef KESTREL_SDK_WRAPPER_T_HPP
