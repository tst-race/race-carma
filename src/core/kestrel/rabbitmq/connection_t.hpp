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

#ifndef KESTREL_RABBITMQ_CONNECTION_T_HPP
#define KESTREL_RABBITMQ_CONNECTION_T_HPP

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <deque>
#include <map>
#include <memory>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <LinkProperties.h>

#include <kestrel/connection_id_t.hpp>
#include <kestrel/rabbitmq/outbox_entry_t.hpp>
#include <kestrel/rabbitmq/worker_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

class plugin_t;
class link_t;

class connection_t final {

  friend class plugin_t;
  friend class link_t;
  friend class worker_t;

  //--------------------------------------------------------------------

  bool garbage_ = true;

  SST_NODISCARD() bool garbage() const noexcept {
    return garbage_;
  }

  void commit() noexcept;

  //--------------------------------------------------------------------

  plugin_t * plugin_;

  SST_NODISCARD() plugin_t & plugin() noexcept {
    SST_ASSERT(plugin_ != nullptr);
    return *plugin_;
  }

  SST_NODISCARD() plugin_t const & plugin() const noexcept {
    SST_ASSERT(plugin_ != nullptr);
    return *plugin_;
  }

  //--------------------------------------------------------------------

  using lookup_it_t =
      std::map<connection_id_t, connection_t *>::iterator;

  lookup_it_t lookup_it_;

  connection_t ** lookup_ptr_;

  SST_NODISCARD() connection_t *& lookup_ptr() noexcept {
    SST_ASSERT(lookup_ptr_ != nullptr);
    return *lookup_ptr_;
  }

  //--------------------------------------------------------------------

  link_t * link_;

  SST_NODISCARD() link_t & link() noexcept {
    SST_ASSERT(link_ != nullptr);
    return *link_;
  }

  SST_NODISCARD() link_t const & link() const noexcept {
    SST_ASSERT(link_ != nullptr);
    return *link_;
  }

  //--------------------------------------------------------------------

  sst::unique_ptr<connection_id_t> id_;

  SST_NODISCARD() connection_id_t const & id() const noexcept {
    SST_ASSERT(id_ != nullptr);
    return *id_;
  }

  //--------------------------------------------------------------------

  long send_timeout_;

  SST_NODISCARD() long send_timeout() const noexcept {
    return send_timeout_;
  }

  //--------------------------------------------------------------------

  std::shared_ptr<worker_t> worker_;

  SST_NODISCARD() worker_t & worker() noexcept {
    SST_ASSERT(worker_ != nullptr);
    return *worker_;
  }

  SST_NODISCARD() worker_t const & worker() const noexcept {
    SST_ASSERT(worker_ != nullptr);
    return *worker_;
  }

  //--------------------------------------------------------------------

  worker_t::queue_t * queue_;

  SST_NODISCARD() worker_t::queue_t const & queue() const noexcept {
    SST_ASSERT(queue_ != nullptr);
    return *queue_;
  }

  SST_NODISCARD() worker_t::queue_t & queue() noexcept {
    SST_ASSERT(queue_ != nullptr);
    return *queue_;
  }

  //--------------------------------------------------------------------

  using outbox_t = std::deque<std::shared_ptr<outbox_entry_t>>;

  sst::unique_ptr<outbox_t> outbox_{sst::in_place};

  SST_NODISCARD() outbox_t & outbox() noexcept {
    SST_ASSERT(outbox_ != nullptr);
    return *outbox_;
  }

  SST_NODISCARD() outbox_t const & outbox() const noexcept {
    SST_ASSERT(outbox_ != nullptr);
    return *outbox_;
  }

  //--------------------------------------------------------------------

public:

  explicit connection_t(tracing_event_t tev,
                        plugin_t & plugin,
                        lookup_it_t lookup_it,
                        link_t & link,
                        connection_id_t const & id,
                        long send_timeout);

  connection_t(connection_t const &) = delete;
  connection_t & operator=(connection_t const &) = delete;
  connection_t(connection_t &&) = delete;
  connection_t & operator=(connection_t &&) = delete;

  ~connection_t() noexcept;

  //--------------------------------------------------------------------
};

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ

#endif // #ifndef KESTREL_RABBITMQ_CONNECTION_T_HPP
