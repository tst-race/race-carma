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

#ifndef KESTREL_C_TA2_PLUGIN_T_HPP
#define KESTREL_C_TA2_PLUGIN_T_HPP

#include <cstdint>
#include <string>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>

#include <IRaceSdkComms.h>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/kestrel_ta2_plugin.h>
#include <kestrel/link_address_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/ta2_plugin_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

class c_ta2_plugin_t : public ta2_plugin_t {

  //--------------------------------------------------------------------
  // plugin
  //--------------------------------------------------------------------

private:

  kestrel_ta2_plugin plugin_;

protected:

  kestrel_ta2_plugin const & plugin() const noexcept {
    return plugin_;
  }

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  explicit c_ta2_plugin_t(IRaceSdkComms & sdk,
                          kestrel_ta2_plugin const & plugin)
      : common_plugin_t(sdk,
                        (SST_ASSERT((plugin.name != nullptr)),
                         reinterpret_cast<char const *>(plugin.name))),
        ta2_plugin_t(sdk,
                     (SST_ASSERT((plugin.name != nullptr)),
                      reinterpret_cast<char const *>(plugin.name))),
        plugin_(plugin) {
    SST_ASSERT((plugin.plugin != nullptr));
    SST_ASSERT((plugin.activateChannel != nullptr));
  }

  c_ta2_plugin_t(c_ta2_plugin_t const &) = delete;

  c_ta2_plugin_t & operator=(c_ta2_plugin_t const &) = delete;

  c_ta2_plugin_t(c_ta2_plugin_t &&) = delete;

  c_ta2_plugin_t & operator=(c_ta2_plugin_t &&) = delete;

  ~c_ta2_plugin_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Inner plugin functions
  //--------------------------------------------------------------------

protected:

  void inner_activateChannel(tracing_event_t tev,
                             race_handle_t const & handle,
                             channel_id_t const & channelGid,
                             std::string const & roleName) override;

  void
  inner_closeConnection(tracing_event_t tev,
                        race_handle_t const & handle,
                        connection_id_t const & connectionId) override;

  void inner_createLink(tracing_event_t tev,
                        race_handle_t const & handle,
                        channel_id_t const & channelGid) override;

  void
  inner_deactivateChannel(tracing_event_t tev,
                          race_handle_t const & handle,
                          channel_id_t const & channelGid) override;

  void inner_destroyLink(tracing_event_t tev,
                         race_handle_t const & handle,
                         link_id_t const & linkId) override;

  void inner_flushChannel(tracing_event_t tev,
                          race_handle_t const & handle,
                          channel_id_t const & channelGid,
                          std::uint64_t batchId) override;

  void
  inner_loadLinkAddress(tracing_event_t tev,
                        race_handle_t const & handle,
                        channel_id_t const & channelGid,
                        link_address_t const & linkAddress) override;

  void inner_onUserAcknowledgementReceived(
      tracing_event_t tev,
      race_handle_t const & handle) override;

  void inner_onUserInputReceived(tracing_event_t tev,
                                 race_handle_t const & handle,
                                 bool answered,
                                 std::string const & response) override;

  void inner_openConnection(tracing_event_t tev,
                            race_handle_t const & handle,
                            link_type_t const & linkType,
                            link_id_t const & linkId,
                            std::string const & linkHints,
                            std::int32_t sendTimeout) override;

  void inner_shutdown(tracing_event_t tev) noexcept override;

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_C_TA2_PLUGIN_T_HPP
