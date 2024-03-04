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

#ifndef KESTREL_DETACHED_CLRMSG_STORE_T_HPP
#define KESTREL_DETACHED_CLRMSG_STORE_T_HPP

#include <list>
#include <map>
#include <string>

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/clrmsg_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

class common_sdk_t;

class detached_clrmsg_store_t final {

public:

  struct entry_t {
    race_handle_t handle;
    clrmsg_t clrmsg;
  };

private:

  std::string dir_{};
  common_sdk_t * sdk_{};

  // TODO: Maybe instead of std::list, we should use std::priority_queue
  //       sorted descendingly by clrmsg timestamp. Then the top element
  //       will be the oldest message.
  std::map<psn_t, std::list<sst::unique_ptr<entry_t>>> map_;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  detached_clrmsg_store_t() = delete;

  ~detached_clrmsg_store_t() SST_NOEXCEPT(true) = default;

  detached_clrmsg_store_t(detached_clrmsg_store_t const &) = delete;

  detached_clrmsg_store_t &
  operator=(detached_clrmsg_store_t const &) = delete;

  detached_clrmsg_store_t(detached_clrmsg_store_t &&) = delete;

  detached_clrmsg_store_t &
  operator=(detached_clrmsg_store_t &&) = delete;

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

public:

  explicit detached_clrmsg_store_t(tracing_event_t tev,
                                   std::string dir,
                                   common_sdk_t * sdk = nullptr);

  //--------------------------------------------------------------------
  // add
  //--------------------------------------------------------------------
  //
  // If this function throws an exception, the entry pointer will not
  // have been added to the store and will not have been moved from.
  //

public:

  void add(tracing_event_t tev, sst::unique_ptr<entry_t> && entry);

  //--------------------------------------------------------------------
  // get
  //--------------------------------------------------------------------

public:

  sst::unique_ptr<entry_t> * get(tracing_event_t tev,
                                 psn_t const & psn);

  //--------------------------------------------------------------------
  // flush
  //--------------------------------------------------------------------

public:

  void flush(tracing_event_t tev);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_DETACHED_CLRMSG_STORE_T_HPP
