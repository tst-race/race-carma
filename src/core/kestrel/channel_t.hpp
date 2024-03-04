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

#ifndef KESTREL_CHANNEL_T_HPP
#define KESTREL_CHANNEL_T_HPP

#include <ChannelProperties.h>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/channel_status_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

class channel_t final {

  //--------------------------------------------------------------------
  // activate
  //--------------------------------------------------------------------

public:

  race_handle_t activate(tracing_event_t tev);

  //--------------------------------------------------------------------
  // id
  //--------------------------------------------------------------------

private:
  channel_id_t id_;

public:
  channel_id_t const & id() const;

  //--------------------------------------------------------------------
  // properties
  //--------------------------------------------------------------------

private:
  ChannelProperties properties_;

public:
  ChannelProperties const & properties() const;
  ChannelProperties const &
  properties(ChannelProperties const & properties);

  //--------------------------------------------------------------------
  // sdk
  //--------------------------------------------------------------------

private:
  sdk_wrapper_t & sdk_;

public:
  sdk_wrapper_t & sdk() const;

  //--------------------------------------------------------------------

public:

  channel_status_t status() const {
    return channel_status_t(properties().channelStatus);
  }

  //--------------------------------------------------------------------

public:
  channel_t(channel_t const &) = delete;
  channel_t(channel_t &&) = delete;
  channel_t & operator=(channel_t const &) = delete;
  channel_t & operator=(channel_t &&) = delete;
  ~channel_t() = default;

  explicit channel_t(tracing_event_t tev,
                     sdk_wrapper_t & sdk,
                     ChannelProperties const & properties);
};

} // namespace kestrel

#endif // #ifndef KESTREL_CHANNEL_T_HPP
