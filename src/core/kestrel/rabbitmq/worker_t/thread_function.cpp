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

#include <array>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/is_positive.hpp>
#include <sst/catalog/is_zero.hpp>
#include <sst/catalog/mono_time_ms.hpp>
#include <sst/catalog/mono_time_ms_t.hpp>
#include <sst/catalog/unique_ptr.hpp>
#include <sst/catalog/what.hpp>

#include <EncPkg.h>

#include <amqp.h>
#include <kestrel/CARMA_XLOG_ERROR.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_role_t.hpp>
#include <kestrel/rabbitmq/link_addrinfo_t.hpp>
#include <kestrel/rabbitmq/plugin_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

void worker_t::thread_function(
    tracing_event_t tev,
    link_addrinfo_t const & addrinfo) noexcept {

  SST_TEV_ADD(tev,
              "addrinfo",
              json_t{
                  {"role", addrinfo.role()},
                  {"host", addrinfo.host()},
                  {"port", addrinfo.port()},
                  {"username", addrinfo.username()},
                  {"password", addrinfo.password()},
                  {"vhost", addrinfo.vhost()},
              });

  bool const sender = addrinfo.role() == link_role_t::loader();
  bool const receiver = !sender;

  struct action_t final {
    queue_t * queue;
    queue_t::status_t status;
  };
  using actions_t = std::array<action_t, 10>;
  actions_t actions_array;
  action_t * const actions = actions_array.data();
  actions_t::size_type const max_actions = actions_array.size();
  actions_t::size_type num_actions;

  while (!stop_->load()) {

    try {

      sst::mono_time_ms_t const now_ms = sst::mono_time_ms();

      //----------------------------------------------------------------
      // Update all queue statuses if we're not connected
      //----------------------------------------------------------------

      if (!connected_) {
        std::lock_guard<std::mutex> const lock(queues_mutex());
        auto it = queues().begin();
        while (it != queues().end()) {
          queue_t & queue = it->second;
          if (queue.garbage
              || queue.status == queue_t::status_t::destroy) {
            it = queues().erase(it);
          } else {
            queue.status = queue_t::status_t::create;
            ++it;
          }
        }
      }

      //----------------------------------------------------------------
      // Connect to the server
      //----------------------------------------------------------------

      connect(SST_TEV_ARG(tev), addrinfo);

      //----------------------------------------------------------------
      // Let RabbitMQ-C release some memory
      //----------------------------------------------------------------

      amqp_maybe_release_buffers(connection_);

      //----------------------------------------------------------------
      // Scan the queues and cache some actions to take
      //----------------------------------------------------------------

      num_actions = 0;
      if (sst::is_positive(queue_actions_.load())
          || now_ms - last_scan_ms_ > queue_ttl_ms_ / 4) {
        std::lock_guard<std::mutex> const lock(queues_mutex());
        auto it = queues().begin();
        while (it != queues().end() && num_actions < max_actions) {
          queue_t & queue = it->second;
          if (queue.garbage) {
            it = queues().erase(it);
            --queue_actions_;
          } else {
            if (sender && queue.status == queue_t::status_t::active
                && now_ms - queue.last_create_ms > queue_ttl_ms_ / 2) {
              queue.status = queue_t::status_t::create;
              ++queue_actions_;
            }
            if (queue.status != queue_t::status_t::active) {
              action_t & action = actions[num_actions];
              action.queue = &queue;
              action.status = queue.status;
              ++num_actions;
            }
            ++it;
          }
        }
        last_scan_ms_ = now_ms;
      }

      //----------------------------------------------------------------
      // Perform the cached actions
      //----------------------------------------------------------------

      for (actions_t::size_type i = 0; i < num_actions; ++i) {
        action_t const & action = actions[i];
        queue_t & queue = *action.queue;
        tracing_event_t tev2 =
            SST_TEV_ARG(tev,
                        "addrinfo",
                        json_t{{"queue", *queue.name}});
        if (action.status == queue_t::status_t::create) {
          declare(SST_TEV_ARG(tev2), queue);
          if (receiver) {
            subscribe(SST_TEV_ARG(tev2), queue);
          }
          queue.last_create_ms = sst::mono_time_ms();
        } else {
          SST_ASSERT(action.status == queue_t::status_t::destroy);
          if (receiver) {
            unsubscribe(SST_TEV_ARG(tev2), queue);
          }
        }
      }

      //----------------------------------------------------------------
      // Merge the cached actions into the queues
      //----------------------------------------------------------------

      if (sst::is_positive(num_actions)) {
        std::lock_guard<std::mutex> const lock(queues_mutex());
        for (actions_t::size_type i = 0; i < num_actions; ++i) {
          action_t const & action = actions[i];
          queue_t & queue = *action.queue;
          if (action.status == queue_t::status_t::create) {
            if (queue.garbage) {
              queue.status = queue_t::status_t::destroy;
              queue.garbage = false;
            } else {
              SST_ASSERT(queue.status == queue_t::status_t::create);
              queue.status = queue_t::status_t::active;
              --queue_actions_;
            }
          } else {
            SST_ASSERT(action.status == queue_t::status_t::destroy);
            SST_ASSERT(!queue.garbage);
            SST_ASSERT(queue.status != queue_t::status_t::create);
            if (queue.status == queue_t::status_t::active) {
              queue.status = queue_t::status_t::create;
              ++queue_actions_;
            } else {
              SST_ASSERT(queue.status == queue_t::status_t::destroy);
              queue.garbage = true;
            }
          }
        }
      }

      //----------------------------------------------------------------
      // If we're a sender, send all the packages we can
      //----------------------------------------------------------------

      if (sender) {
        while (true) {

          std::shared_ptr<outbox_entry_t> & entry = outbox_entry_;

          if (entry == nullptr) {
            std::lock_guard<std::mutex> const lock(outbox_mutex());
            while (!outbox().empty()) {
              auto p = outbox().front().lock();
              if (p != nullptr) {
                entry = std::move(p);
                outbox().pop();
                break;
              }
              outbox().pop();
            }
            if (entry == nullptr) {
              break;
            }
          }
          SST_ASSERT(entry != nullptr);

          if (!entry->sent()) {
            send(SST_TEV_ARG(tev),
                 entry->package(),
                 *entry->connection().queue().name);
            entry->sent() = true;
          }
          SST_ASSERT(entry->sent());

          {
            auto & x = entry->fired();
            auto a = outbox_entry_t::fired_t::not_yet;
            auto const b = outbox_entry_t::fired_t::by_worker_thread;
            if (x.compare_exchange_strong(a, b) || a == b) {
              plugin().sdk().onPackageStatusChanged(
                  SST_TEV_DEF(tracing_event_t),
                  entry->handle().value(),
                  PACKAGE_SENT,
                  0);
            }
          }

          entry = nullptr;

        } //
      }

      //----------------------------------------------------------------
      // If we're a receiver, receive all the packages we can
      //----------------------------------------------------------------

      if (receiver) {
        while (true) {
          if (have_envelope_) {
            RawData buffer;
            {
              amqp_bytes_t const & data = envelope_.message.body;
              if (sst::is_positive(data.len)) {
                SST_ASSERT(data.bytes != nullptr);
                unsigned char const * const p =
                    static_cast<unsigned char const *>(data.bytes);
                buffer.insert(buffer.end(), p, p + data.len);
              }
            }
            EncPkg encpkg(std::move(buffer));
            std::string name;
            {
              amqp_bytes_t const & data = envelope_.routing_key;
              if (sst::is_positive(data.len)) {
                SST_ASSERT(data.bytes != nullptr);
                unsigned char const * const p =
                    static_cast<unsigned char const *>(data.bytes);
                name.insert(name.end(), p, p + data.len);
              }
            }
            auto const it = queues().find(name);
            SST_ASSERT(it != queues().end());
            queue_t const & queue = it->second;
            plugin().sdk().receiveEncPkg(SST_TEV_DEF(tracing_event_t),
                                         std::move(encpkg),
                                         *queue.connection_ids,
                                         0);
          }
          receive(SST_TEV_ARG(tev));
          if (!have_envelope_) {
            break;
          }
        }
      }

      // This loop seems to run kind of hot. I'm not sure why the call
      // to receive() above isn't relieving the CPU, as that call does
      // make a non-blocking call to RabbitMQ-C with a decent timeout
      // parameter. We'll just do a little sleep here, I guess.
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      //----------------------------------------------------------------
    } catch (tracing_exception_t const & e) {
      CARMA_XLOG_ERROR(plugin().sdk(),
                       0,
                       SST_TEV_ARG(e.tev(), "exception", sst::what(e)));
      error_sleep();
    } catch (...) {
      CARMA_XLOG_ERROR(plugin().sdk(),
                       0,
                       SST_TEV_ARG(tev, "exception", sst::what()));
      error_sleep();
    }
  }

  disconnect();

} //

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
