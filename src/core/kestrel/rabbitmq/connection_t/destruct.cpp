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
#include <kestrel/rabbitmq/connection_t.hpp>
// Include twice to test idempotence.
#include <kestrel/rabbitmq/connection_t.hpp>
//

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_DEF.hpp>

#include <PackageStatus.h>

#include <kestrel/rabbitmq/plugin_t.hpp>
#include <kestrel/rabbitmq/worker_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace rabbitmq {

connection_t::~connection_t() noexcept {
  if (!garbage()) {
    lookup_ptr() = nullptr;
    ++plugin().connections_garbage_;
    try {
      plugin().connections().erase(lookup_it_);
      --plugin().connections_garbage_;
    } catch (...) {
    }
  }

  //--------------------------------------------------------------------
  // Detach from the RabbitMQ queue
  //--------------------------------------------------------------------

  {
    std::lock_guard<std::mutex> const lock(worker().queues_mutex());
    SST_ASSERT(!queue().garbage);
    SST_ASSERT(queue().connection_count > 0U);
    SST_ASSERT(queue().connection_ids != nullptr);
    SST_ASSERT(!queue().connection_ids->empty());
    if (--queue().connection_count == 0U) {
      using status_t = worker_t::queue_t::status_t;
      SST_ASSERT(queue().status != status_t::destroy);
      if (queue().status == status_t::create) {
        queue().garbage = true;
      } else {
        SST_ASSERT(queue().status == status_t::active);
        queue().status = status_t::destroy;
        ++worker().queue_actions_;
      }
    }
    try {
      std::string * p = &queue().connection_ids->front();
      std::string & back = queue().connection_ids->back();
      while (p != &back && *p != id().string()) {
        ++p;
      }
      if (p != &back) {
        std::string a = std::move(*p);
        std::string b = std::move(back);
        *p = std::move(b);
        back = std::move(a);
      }
      queue().connection_ids->pop_back();
    } catch (...) {
    }
  }

  //--------------------------------------------------------------------
  // Fail all pending packages
  //--------------------------------------------------------------------
  //
  // If we're processing an entry at the same time as the worker thread,
  // whichever one of us modifies entry->fired() first gets to fire the
  // onPackageStatusChanged event. It's possible that we'll be scooping
  // a package out from under the worker thread and firing a fail event
  // for it while the worker thread is actively sending it, but this is
  // acceptable. The alternative would be to use a mutex instead of an
  // atomic, but then network issues could cause the worker thread to
  // block us for a long time, which would be unacceptable.
  //

  try {
    for (auto & entry : outbox()) {
      if (entry != nullptr) {
        auto a = outbox_entry_t::fired_t::not_yet;
        auto const b = outbox_entry_t::fired_t::by_main_thread;
        if (entry->fired().compare_exchange_strong(a, b)) {
          try {
            plugin().sdk().onPackageStatusChanged(
                SST_TEV_DEF(tracing_event_t),
                entry->handle().value(),
                entry->overdue() ? PACKAGE_FAILED_TIMEOUT :
                                   PACKAGE_FAILED_GENERIC,
                0);
          } catch (...) {
          }
        }
        entry = nullptr;
      }
    }
  } catch (...) {
  }

  //--------------------------------------------------------------------

  if (worker_.use_count() == 1) {
    ++plugin().workers_garbage_;
    try {
      auto const p = plugin().workers().find(link().addrinfo());
      SST_ASSERT(p != plugin().workers().end());
      std::weak_ptr<worker_t> & w = p->second;
      SST_ASSERT(!w.owner_before(worker_));
      SST_ASSERT(!worker_.owner_before(w));
      plugin().workers().erase(p);
      --plugin().workers_garbage_;
    } catch (...) {
    }
  }
}

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ
