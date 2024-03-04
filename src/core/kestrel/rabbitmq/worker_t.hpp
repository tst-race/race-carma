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

#ifndef KESTREL_RABBITMQ_WORKER_T_HPP
#define KESTREL_RABBITMQ_WORKER_T_HPP

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include <sys/time.h>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/mono_time_ms_t.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <EncPkg.h>
#include <LinkProperties.h>

#include <amqp.h>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/rabbitmq/link_addrinfo_t.hpp>
#include <kestrel/rabbitmq/outbox_entry_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

class plugin_t;
class link_t;
class connection_t;

class worker_t final {

  friend class plugin_t;
  friend class link_t;
  friend class connection_t;

  //--------------------------------------------------------------------
  // Parent plugin
  //--------------------------------------------------------------------
  //
  // plugin_ is a back pointer to the parent plugin. The lifetime of a
  // worker is a subset of its parent plugin, so plugin_ will never be
  // null.
  //

private:

  plugin_t * plugin_;

public:

  plugin_t & plugin() SST_NOEXCEPT(true) {
    SST_ASSERT((plugin_ != nullptr));
    return *plugin_;
  }

  plugin_t const & plugin() const SST_NOEXCEPT(true) {
    SST_ASSERT((plugin_ != nullptr));
    return *plugin_;
  }

  //--------------------------------------------------------------------
  // Parent link
  //--------------------------------------------------------------------
  //
  // link_ is a back pointer to the parent link. The lifetime of a
  // worker is a subset of its parent link, so link_ will never be null.
  //

private:

  link_t * link_;

public:

  link_t & link() SST_NOEXCEPT(true) {
    SST_ASSERT((link_ != nullptr));
    return *link_;
  }

  link_t const & link() const SST_NOEXCEPT(true) {
    SST_ASSERT((link_ != nullptr));
    return *link_;
  }

  //--------------------------------------------------------------------
  // Mutexing
  //--------------------------------------------------------------------

  sst::unique_ptr<std::mutex> queues_mutex_{sst::in_place};

  std::mutex & queues_mutex() noexcept {
    SST_ASSERT(queues_mutex_ != nullptr);
    return *queues_mutex_;
  }

  //--------------------------------------------------------------------
  // RabbitMQ queue tracking
  //--------------------------------------------------------------------

  struct queue_t;

  struct queue_t final {

    bool garbage = true;
    unsigned long long connection_count = 0U;
    sst::unique_ptr<std::vector<ConnectionID>> connection_ids;
    sst::unique_ptr<std::string> name;
    enum class status_t {
      create,
      active,
      destroy
    } status = status_t::create;
    sst::mono_time_ms_t last_create_ms = 0;
    sst::unique_ptr<std::vector<unsigned char>> tag;

    explicit queue_t(connection_id_t const & connection_id,
                     std::string const & name)
        : connection_ids(sst::in_place, 1, connection_id.value()),
          name(sst::in_place, name) {
    }

    queue_t(queue_t const &) = delete;
    queue_t & operator=(queue_t const &) = delete;
    queue_t(queue_t &&) = delete;
    queue_t & operator=(queue_t &&) noexcept = default;
    ~queue_t() noexcept = default;

  }; //

  using queues_t = std::map<std::string, queue_t>;

  sst::unique_ptr<queues_t> queues_{sst::in_place};

  // The number of entries in queues_ that are not in active status. In
  // other words, the number of entries q for which q.garbage is true or
  // q.status is not queue_t::status_t::active.
  std::atomic<unsigned long long> queue_actions_{0U};

  sst::mono_time_ms_t last_scan_ms_ = 0;

  SST_NODISCARD() queues_t & queues() noexcept {
    SST_ASSERT(queues_ != nullptr);
    return *queues_;
  }

  //--------------------------------------------------------------------

  sst::unique_ptr<std::future<void>> thread_{sst::in_place};

  SST_NODISCARD() std::future<void> & thread() noexcept {
    SST_ASSERT(thread_ != nullptr);
    return *thread_;
  }

  sst::unique_ptr<std::atomic<bool>> stop_{sst::in_place, false};

  //--------------------------------------------------------------------

  void error_sleep() noexcept;

  //--------------------------------------------------------------------

  using outbox_t = std::queue<std::weak_ptr<outbox_entry_t>>;

  sst::unique_ptr<outbox_t> outbox_{sst::in_place};

  SST_NODISCARD() outbox_t & outbox() noexcept {
    SST_ASSERT(outbox_ != nullptr);
    return *outbox_;
  }

  sst::unique_ptr<std::mutex> outbox_mutex_{sst::in_place};

  SST_NODISCARD() std::mutex & outbox_mutex() noexcept {
    SST_ASSERT(outbox_mutex_ != nullptr);
    return *outbox_mutex_;
  }

  //--------------------------------------------------------------------
  // The current outbox entry we're sending
  //--------------------------------------------------------------------

private:

  std::shared_ptr<outbox_entry_t> outbox_entry_;

  //--------------------------------------------------------------------
  // RabbitMQ connection
  //--------------------------------------------------------------------

private:

  decltype(timeval{}.tv_usec) connect_timeout_us_ = 5000000;
  int channel_max_ = AMQP_DEFAULT_MAX_CHANNELS;
  int frame_max_ = AMQP_DEFAULT_FRAME_SIZE;
  amqp_sasl_method_enum sasl_method_ = AMQP_SASL_METHOD_PLAIN;
  int heartbeat_ = 0;
  amqp_channel_t channel_ = 1;
  std::int64_t queue_ttl_ms_ = 3600000;
  decltype(timeval{}.tv_usec) receive_timeout_us_ = 1000000;

  amqp_connection_state_t connection_ = nullptr;
  amqp_socket_t * socket_ = nullptr;
  bool ssl_ = true;
  bool connected_ = false;
  amqp_envelope_t envelope_{};
  bool have_envelope_ = false;

  template<class T>
  static amqp_bytes_t wrap_as_amqp_bytes(T const & x) {
    amqp_bytes_t y{};
    y.len = sst::checked_cast<std::size_t>(x.size());
    y.bytes = const_cast<void *>(static_cast<void const *>(x.data()));
    return y;
  }

  void connect(tracing_event_t tev, link_addrinfo_t const & server);

  void declare(tracing_event_t tev, queue_t & queue);

  void subscribe(tracing_event_t tev, queue_t & queue);

  void unsubscribe(tracing_event_t tev, queue_t const & queue);

  void send(tracing_event_t tev,
            RawData const & package,
            std::string const & queue);

  void receive(tracing_event_t tev);

  void disconnect() noexcept;

  //--------------------------------------------------------------------

  static std::atomic<std::uintmax_t> thread_id_;

  void thread_function(tracing_event_t tev,
                       link_addrinfo_t const & addrinfo) noexcept;

  //--------------------------------------------------------------------

  SST_NODISCARD()
  queue_t & add_connection(tracing_event_t tev, connection_t const &);

  //--------------------------------------------------------------------

public:

  explicit worker_t(tracing_event_t tev, link_t & link);

  worker_t(worker_t const &) = delete;
  worker_t & operator=(worker_t const &) = delete;
  worker_t(worker_t &&) = delete;
  worker_t & operator=(worker_t &&) = delete;

  ~worker_t() noexcept;

  //--------------------------------------------------------------------
};

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ

#endif // #ifndef KESTREL_RABBITMQ_WORKER_T_HPP
