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
#include <kestrel/rabbitmq/worker_t.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq/worker_t.hpp>
//

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <cstddef>
#include <stdexcept>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/checked_cast.hpp>

#include <amqp.h>
#include <kestrel/CARMA_XLOG_INFO.hpp>
#include <kestrel/rabbitmq/plugin_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

void worker_t::unsubscribe(tracing_event_t tev, queue_t const & queue) {
  SST_TEV_ADD(tev);
  try {
    try {

      SST_ASSERT(connection_ != nullptr);
      SST_ASSERT(socket_ != nullptr);
      SST_ASSERT(connected_);
      SST_ASSERT(queue.tag != nullptr);

      amqp_bytes_t tag;
      tag.len = sst::checked_cast<std::size_t>(queue.tag->size());
      tag.bytes = const_cast<unsigned char *>(queue.tag->data());

      (void)amqp_basic_cancel(connection_, channel_, tag);
      amqp_rpc_reply_t const r = amqp_get_rpc_reply(connection_);
      if (r.reply_type != AMQP_RESPONSE_NORMAL) {
        throw std::runtime_error("amqp_basic_cancel() failed.");
      }

      CARMA_XLOG_INFO(
          plugin().sdk(),
          0,
          SST_TEV_ARG(tev, "event", "unsubscribed_from_queue"));

    } catch (...) {
      disconnect();
      throw;
    }
  }
  SST_TEV_RETHROW(tev);
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
