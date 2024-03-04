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
#include <kestrel/channel_t.hpp>
// Include twice to test idempotence.
#include <kestrel/channel_t.hpp>
//

#include <string>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>

#include <ChannelProperties.h>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// id
//----------------------------------------------------------------------

channel_id_t const & channel_t::id() const {
  return id_;
}

//----------------------------------------------------------------------
// properties
//----------------------------------------------------------------------

ChannelProperties const & channel_t::properties() const {
  return properties_;
}

ChannelProperties const &
channel_t::properties(ChannelProperties const & properties) {
  SST_ASSERT((properties.channelGid == id().string()));
  properties_ = properties;
  return properties_;
}

//----------------------------------------------------------------------
// sdk
//----------------------------------------------------------------------

sdk_wrapper_t & channel_t::sdk() const {
  return sdk_;
}

//----------------------------------------------------------------------

channel_t::channel_t(tracing_event_t tev,
                     sdk_wrapper_t & sdk,
                     ChannelProperties const & properties)
    : id_(properties.channelGid),
      properties_(properties),
      sdk_((SST_TEV_ADD(tev), sdk)) {
  try {
  }
  SST_TEV_RETHROW(tev);
}

} // namespace kestrel
