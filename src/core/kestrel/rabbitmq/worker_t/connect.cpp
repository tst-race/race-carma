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

#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>

#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <kestrel/catalog/KESTREL_WITH_OPENSSL_SSL.h>
#include <kestrel/rabbitmq/link_addrinfo_t.hpp>
#include <kestrel/tracing_event_t.hpp>

#if KESTREL_WITH_OPENSSL_SSL
#include <amqp_ssl_socket.h>
#endif

namespace kestrel {
namespace rabbitmq {

// TODO: Support server.host() values that begin with "amqp://" or
//       "amqps://". Perhaps default to only trying "amqps://" if
//       neither prefix is given instead of trying both.

void worker_t::connect(tracing_event_t tev,
                       link_addrinfo_t const & server) {
  SST_TEV_ADD(tev);
  try {
    bool flip_ssl_on_failure = false;
    try {
      if (connection_ == nullptr) {
        connection_ = amqp_new_connection();
        if (connection_ == nullptr) {
          throw std::runtime_error("amqp_new_connection() failed");
        }
      }
      flip_ssl_on_failure = true;
      if (socket_ == nullptr) {
        if (ssl_ && KESTREL_WITH_OPENSSL_SSL) {
#if KESTREL_WITH_OPENSSL_SSL
          amqp_set_initialize_ssl_library(0);
          socket_ = amqp_ssl_socket_new(connection_);
          if (socket_ == nullptr) {
            throw std::runtime_error("amqp_ssl_socket_new() failed");
          }
          amqp_ssl_socket_set_verify_peer(socket_, 0);
          amqp_ssl_socket_set_verify_hostname(socket_, 0);
#endif
        } else {
          socket_ = amqp_tcp_socket_new(connection_);
          if (socket_ == nullptr) {
            throw std::runtime_error("amqp_tcp_socket_new() failed");
          }
        }
      }
      if (!connected_) {
        {
          timeval timeout{};
          timeout.tv_usec = connect_timeout_us_;
          int const s =
              amqp_socket_open_noblock(socket_,
                                       server.host().c_str(),
                                       static_cast<int>(server.port()),
                                       &timeout);
          if (s != AMQP_STATUS_OK) {
            throw std::runtime_error(
                "amqp_socket_open_noblock() failed");
          }
        }
        {
          amqp_rpc_reply_t const r =
              amqp_login(connection_,
                         server.vhost().c_str(),
                         channel_max_,
                         frame_max_,
                         heartbeat_,
                         sasl_method_,
                         server.username().c_str(),
                         server.password().c_str());
          if (r.reply_type != AMQP_RESPONSE_NORMAL) {
            throw std::runtime_error(
                "amqp_login() failed (SSL "
                + std::string(ssl_ && KESTREL_WITH_OPENSSL_SSL ?
                                  "enabled" :
                                  "disabled")
                + ")");
          }
        }
        {
          (void)amqp_channel_open(connection_, channel_);
          amqp_rpc_reply_t const r = amqp_get_rpc_reply(connection_);
          if (r.reply_type != AMQP_RESPONSE_NORMAL) {
            throw std::runtime_error("amqp_channel_open() failed");
          }
        }
        connected_ = true;
      }
    } catch (...) {
      disconnect();
      if (flip_ssl_on_failure) {
        ssl_ = !ssl_;
      }
      throw;
    }
  }
  SST_TEV_RETHROW(tev);
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
