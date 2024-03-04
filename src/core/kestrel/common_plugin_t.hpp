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

#ifndef KESTREL_COMMON_PLUGIN_T_HPP
#define KESTREL_COMMON_PLUGIN_T_HPP

#include <atomic>
#include <exception>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/SST_THROW_UNIMPLEMENTED.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/str_cmp.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unique_ptr.hpp>
#include <sst/catalog/what.hpp>

#include <PluginConfig.h>
#include <PluginResponse.h>

#include <kestrel/CARMA_XLOG_ERROR.hpp>
#include <kestrel/CARMA_XLOG_FATAL.hpp>
#include <kestrel/CARMA_XLOG_TRACE.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/plugin_response_t.hpp>
#include <kestrel/plugin_state_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_to_json.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/tracing_exception_t.hpp>

namespace kestrel {

template<class Sdk>
class common_plugin_t {

  //--------------------------------------------------------------------
  // Underlying SDK
  //--------------------------------------------------------------------
  //
  // TODO: Make sdk_ private. This hits a lot of spots that need to be
  //       changed to use sdk() instead of sdk_. Do it gradually.
  //

protected:

  Sdk sdk_;

protected:

  Sdk & sdk() noexcept {
    return sdk_;
  }

  Sdk const & sdk() const noexcept {
    return sdk_;
  }

  //--------------------------------------------------------------------

protected:

  sst::unique_ptr<psn_t> persona_;

  psn_t const & persona() const noexcept {
    SST_ASSERT((persona_ != nullptr));
    return *persona_;
  }

  //--------------------------------------------------------------------
  // Plugin call numbering
  //--------------------------------------------------------------------
  //
  // Technically this doesn't need to be static, but doing so will make
  // all plugin calls have unique IDs across all plugins that use this
  // class, which is nice.
  //

protected:

  static std::atomic_uintmax_t plugin_call_id_;

  //--------------------------------------------------------------------
  // Primary mutexing
  //--------------------------------------------------------------------
  //
  // primary_mutex_ is the mutex under which all "primary calls" into
  // the plugin are synchronized. Primary calls should finish quickly,
  // performing any long-running work in other threads. Primary calls
  // have unrestricted access to all "primary data".
  //
  // All plugin calls and data are primary unless otherwise noted. In
  // particular, all calls into the plugin through the IRacePluginTa*
  // functions are primary.
  //
  // TODO: Use sst::mutex once it's available. It should have recursive
  //       lock detection when SST_DEBUG is on.
  //

protected:

  std::shared_ptr<std::recursive_mutex> primary_mutex_ =
      std::make_shared<std::recursive_mutex>();
  bool primary_mutex_entered_ = false;
  using primary_lock_t = std::lock_guard<std::recursive_mutex>;

public:

  std::shared_ptr<std::recursive_mutex> const & primary_mutex() const {
    return this->primary_mutex_;
  }

  //--------------------------------------------------------------------
  // Plugin state
  //--------------------------------------------------------------------

protected:

  plugin_state_t plugin_state_ = plugin_state_t::uninitialized;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

protected:

  explicit common_plugin_t(typename Sdk::race_sdk_t & sdk,
                           std::string const & plugin_name)
      : sdk_(sdk, plugin_name) {
  }

  common_plugin_t(common_plugin_t const &) = delete;

  common_plugin_t & operator=(common_plugin_t const &) = delete;

  common_plugin_t(common_plugin_t &&) = delete;

  common_plugin_t & operator=(common_plugin_t &&) = delete;

  virtual ~common_plugin_t() noexcept {
    try {
      if (plugin_state_ != plugin_state_t::uninitialized) {
        inner_shutdown(SST_TEV_DEF(tracing_event_t));
      }
    } catch (...) {
    }
  }

  //--------------------------------------------------------------------
  // Input description functions
  //--------------------------------------------------------------------

protected:

  static json_t i_init(PluginConfig const & pluginConfig) {
    return {
        {"pluginConfig", race_to_json(pluginConfig)},
    };
  }

  static json_t i_shutdown() {
    return {};
  }

  //--------------------------------------------------------------------
  // Output description functions
  //--------------------------------------------------------------------

protected:

  static json_t o_init(PluginConfig const &) {
    return {};
  }

  static json_t o_shutdown() {
    return {};
  }

  //--------------------------------------------------------------------
  // Common plugin function behavior
  //--------------------------------------------------------------------
  //
  // For each function common_foo, the foo function in both ta1_plugin_t
  // and ta2_plugin_t should begin by calling common_foo.
  //

protected:

  virtual void common_init(tracing_event_t tev,
                           PluginConfig const & pluginConfig);

  virtual void common_shutdown(tracing_event_t tev) noexcept;

  //--------------------------------------------------------------------
  // Inner plugin functions
  //--------------------------------------------------------------------
  //
  // If you're indirectly inheriting from this class to implement a TA1
  // or TA2 plugin, these are the functions you need to implement.
  //
  // Each of these functions has a default implementation that simply
  // throws an exception.
  //

protected:

  virtual void inner_init(tracing_event_t tev,
                          PluginConfig const & pluginConfig);

  virtual void inner_shutdown(tracing_event_t tev) noexcept;

  //--------------------------------------------------------------------
  // inner_call
  //--------------------------------------------------------------------

protected:

  template<char const * PluginFunction,
           PluginResponse PluginFailure,
           class Lambda>
  PluginResponse inner_call(tracing_event_t & tev, Lambda && lambda) {
    SST_TEV_ADD(tev);
    primary_lock_t const primary_lock(*this->primary_mutex());
    PluginResponse plugin_response = PluginFailure;
    SST_TEV_ADD(tev,
                "local_psn",
                persona_ == nullptr ?
                    sdk().getActivePersona(SST_TEV_ARG(tev)) :
                    persona().value());
    SST_TEV_ADD(tev,
                "plugin",
                sdk().plugin_name(),
                "plugin_function",
                PluginFunction,
                "plugin_call_id",
                sst::to_string(plugin_call_id_++));
    CARMA_XLOG_TRACE(
        sdk(),
        0,
        SST_TEV_ARG(tev, "event", "plugin_function_called"));
    try {
      if (primary_mutex_entered_) {
        plugin_response = PLUGIN_ERROR;
        try {
          throw std::runtime_error("Plugin called recursively.");
        }
        SST_TEV_RETHROW(tev);
      }
      if (plugin_state_ == plugin_state_t::uninitialized) {
        if (sst::str_cmp(PluginFunction, "init") != 0) {
          plugin_response = PLUGIN_ERROR;
          try {
            throw std::runtime_error("Plugin is not initialized.");
          }
          SST_TEV_RETHROW(tev);
        }
      } else if (plugin_state_ == plugin_state_t::initialized) {
        if (sst::str_cmp(PluginFunction, "init") == 0) {
          plugin_response = PLUGIN_ERROR;
          try {
            throw std::runtime_error("Plugin is already initialized.");
          }
          SST_TEV_RETHROW(tev);
        }
      } else if (sst::str_cmp(PluginFunction, "shutdown") != 0) {
        plugin_response = PLUGIN_FATAL;
        try {
          throw std::runtime_error("Plugin is broken.");
        }
        SST_TEV_RETHROW(tev);
      }
      try {
        primary_mutex_entered_ = true;
        lambda(tev);
        primary_mutex_entered_ = false;
      } catch (...) {
        primary_mutex_entered_ = false;
        throw;
      }
      if (sst::str_cmp(PluginFunction, "init") == 0) {
        plugin_state_ = plugin_state_t::initialized;
      } else if (sst::str_cmp(PluginFunction, "shutdown") == 0) {
        plugin_state_ = plugin_state_t::uninitialized;
      }
      plugin_response = PLUGIN_OK;
      SST_TEV_ADD(tev,
                  "plugin_function_output",
                  json_t({{"return_value",
                           plugin_response_t(plugin_response)}}));
      CARMA_XLOG_TRACE(
          sdk(),
          0,
          SST_TEV_ARG(tev, "event", "plugin_function_succeeded"));
      return plugin_response;
    } catch (tracing_exception_t const & e) {
      SST_TEV_ADD(tev,
                  "plugin_function_output",
                  json_t({{"return_value",
                           plugin_response_t(plugin_response)}}));
      if (plugin_response == PLUGIN_FATAL) {
        inner_shutdown(SST_TEV_ARG(tev));
        plugin_state_ = plugin_state_t::broken;
        CARMA_XLOG_FATAL(sdk(),
                         0,
                         SST_TEV_ARG(e.tev(),
                                     "event",
                                     "plugin_function_failed",
                                     "exception",
                                     sst::what()));
      } else {
        SST_ASSERT((plugin_response == PLUGIN_ERROR));
        CARMA_XLOG_ERROR(sdk(),
                         0,
                         SST_TEV_ARG(e.tev(),
                                     "event",
                                     "plugin_function_failed",
                                     "exception",
                                     sst::what()));
      }
      return plugin_response;
    } catch (...) {
      SST_TEV_ADD(tev,
                  "plugin_function_output",
                  json_t({{"return_value",
                           plugin_response_t(plugin_response)}}));
      if (plugin_response == PLUGIN_FATAL) {
        inner_shutdown(SST_TEV_ARG(tev));
        plugin_state_ = plugin_state_t::broken;
        CARMA_XLOG_FATAL(sdk(),
                         0,
                         SST_TEV_ARG(tev,
                                     "event",
                                     "plugin_function_failed",
                                     "exception",
                                     sst::what()));
      } else {
        SST_ASSERT((plugin_response == PLUGIN_ERROR));
        CARMA_XLOG_ERROR(sdk(),
                         0,
                         SST_TEV_ARG(tev,
                                     "event",
                                     "plugin_function_failed",
                                     "exception",
                                     sst::what()));
      }
      return plugin_response;
    }
  }

  //--------------------------------------------------------------------
  // Miscellaneous functions
  //--------------------------------------------------------------------

protected:

  std::runtime_error
  unsupported_function(char const * const function) const {
    SST_ASSERT((function != nullptr));
    return std::runtime_error("This plugin (" + sdk().plugin_name()
                              + ") does not support the " + function
                              + " function.");
  }

  //--------------------------------------------------------------------
};

//----------------------------------------------------------------------
// plugin_call_id_
//----------------------------------------------------------------------

template<class Sdk>
std::atomic_uintmax_t common_plugin_t<Sdk>::plugin_call_id_{0};

//----------------------------------------------------------------------

} // namespace kestrel

//----------------------------------------------------------------------

#include <kestrel/common_plugin_t/common_init.hpp>
#include <kestrel/common_plugin_t/common_shutdown.hpp>
#include <kestrel/common_plugin_t/inner_init.hpp>
#include <kestrel/common_plugin_t/inner_shutdown.hpp>

//----------------------------------------------------------------------

#endif // #ifndef KESTREL_COMMON_PLUGIN_T_HPP
