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

#include <stdexcept>

#include <sys/time.h>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>

#include <amqp.h>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

void worker_t::receive(tracing_event_t tev) {
  SST_TEV_ADD(tev);
  try {
    try {

      SST_ASSERT(connection_ != nullptr);
      SST_ASSERT(socket_ != nullptr);
      SST_ASSERT(connected_);

      if (have_envelope_) {
        amqp_destroy_envelope(&envelope_);
        have_envelope_ = false;
      }

      timeval timeout{};
      timeout.tv_usec = receive_timeout_us_;

      int const flags = 0;

      amqp_rpc_reply_t const r = amqp_consume_message(connection_,
                                                      &envelope_,
                                                      &timeout,
                                                      flags);
      if (r.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION
          && r.library_error == AMQP_STATUS_TIMEOUT) {
        return;
      }
      if (r.reply_type != AMQP_RESPONSE_NORMAL) {
        throw std::runtime_error("amqp_consume_message() failed.");
      }

      have_envelope_ = true;

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
