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

#ifndef KESTREL_EASY_COMMON_PLUGIN_T_HPP
#define KESTREL_EASY_COMMON_PLUGIN_T_HPP

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
#include <kestrel/common_plugin_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/null_config_t.hpp>
#include <kestrel/plugin_response_t.hpp>
#include <kestrel/plugin_state_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_to_json.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/ta2_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/tracing_exception_t.hpp>

namespace kestrel {

template<class Sdk, class Config = null_config_t>
class easy_common_plugin_t : public virtual common_plugin_t<Sdk> {

  //--------------------------------------------------------------------
  // config
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<Config> config_;

protected:

  Config & config() noexcept {
    SST_ASSERT((config_));
    return *config_;
  }

  Config const & config() const noexcept {
    SST_ASSERT((config_));
    return *config_;
  }

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

protected:

  explicit easy_common_plugin_t(typename Sdk::race_sdk_t & sdk,
                                std::string const & plugin_name)
      : common_plugin_t<Sdk>(sdk, plugin_name) {
  }

  easy_common_plugin_t(easy_common_plugin_t const &) = delete;

  easy_common_plugin_t &
  operator=(easy_common_plugin_t const &) = delete;

  easy_common_plugin_t(easy_common_plugin_t &&) = delete;

  easy_common_plugin_t & operator=(easy_common_plugin_t &&) = delete;

  virtual ~easy_common_plugin_t() noexcept {
  }

  //--------------------------------------------------------------------
  // Common plugin function behavior
  //--------------------------------------------------------------------

protected:

  void common_init(tracing_event_t tev,
                   PluginConfig const & pluginConfig) override;

  void common_shutdown(tracing_event_t tev) noexcept override;

  //--------------------------------------------------------------------
};

} // namespace kestrel

//----------------------------------------------------------------------

#include <kestrel/easy_common_plugin_t/common_init.hpp>
#include <kestrel/easy_common_plugin_t/common_shutdown.hpp>

//----------------------------------------------------------------------

#endif // #ifndef KESTREL_EASY_COMMON_PLUGIN_T_HPP
