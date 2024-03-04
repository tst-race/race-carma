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

#ifndef KESTREL_RABBITMQ_OUTBOX_ENTRY_T_HPP
#define KESTREL_RABBITMQ_OUTBOX_ENTRY_T_HPP

#include <kestrel/catalog/KESTREL_WITH_KESTREL_RABBITMQ.h>

#if KESTREL_WITH_KESTREL_RABBITMQ

#include <atomic>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/unix_time_s.hpp>

#include <EncPkg.h>

#include <kestrel/race_handle_t.hpp>

namespace kestrel {
namespace rabbitmq {

class connection_t;

class outbox_entry_t final {

  //--------------------------------------------------------------------
  // Containing connection
  //--------------------------------------------------------------------
  //
  // This is a back pointer to our containing connection. Our lifetime
  // is always contained by the lifetime of our containing connection,
  // so this should never be null.
  //

private:

  connection_t * connection_;

public:

  SST_NODISCARD() connection_t & connection() noexcept {
    SST_ASSERT(connection_ != nullptr);
    return *connection_;
  }

  //--------------------------------------------------------------------

private:

  RawData package_;

public:

  SST_NODISCARD() RawData const & package() const noexcept {
    return package_;
  }

  //--------------------------------------------------------------------

private:

  bool sent_ = false;

public:

  SST_NODISCARD() bool & sent() noexcept {
    return sent_;
  }

  //--------------------------------------------------------------------

public:

  enum class fired_t { not_yet, by_main_thread, by_worker_thread };

private:

  std::atomic<fired_t> fired_{fired_t::not_yet};

public:

  SST_NODISCARD() std::atomic<fired_t> & fired() noexcept {
    return fired_;
  }

  //--------------------------------------------------------------------

private:

  race_handle_t handle_;

public:

  SST_NODISCARD() race_handle_t const & handle() const noexcept {
    return handle_;
  }

  //--------------------------------------------------------------------

private:

  double deadline_;

public:

  double deadline() const noexcept {
    return deadline_;
  }

  bool overdue() const {
    return sst::unix_time_s() > deadline();
  }

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  explicit outbox_entry_t(connection_t & connection,
                          race_handle_t const & handle,
                          RawData && package,
                          double const deadline)
      : connection_(&connection),
        package_(std::move(package)),
        handle_(handle),
        deadline_(deadline) {
  }

  outbox_entry_t(outbox_entry_t const &) = delete;
  outbox_entry_t & operator=(outbox_entry_t const &) = delete;
  outbox_entry_t(outbox_entry_t &&) = delete;
  outbox_entry_t & operator=(outbox_entry_t &&) = delete;
  ~outbox_entry_t() noexcept = default;

  //--------------------------------------------------------------------
};

} // namespace rabbitmq
} // namespace kestrel

#endif // #if KESTREL_WITH_KESTREL_RABBITMQ

#endif // #ifndef KESTREL_RABBITMQ_OUTBOX_ENTRY_T_HPP
