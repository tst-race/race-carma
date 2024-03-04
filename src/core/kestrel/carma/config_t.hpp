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

#ifndef KESTREL_CARMA_CONFIG_T_HPP
#define KESTREL_CARMA_CONFIG_T_HPP

#include <memory>
#include <mutex>
#include <set>
#include <string>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/carma/bootstrap_config_t.hpp>
#include <kestrel/carma/clrmsg_store_t.hpp>
#include <kestrel/carma/global_config_t.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_t.hpp>
#include <kestrel/channel_id_t.hpp>
#include <kestrel/chunk_joiner_t.hpp>
#include <kestrel/common_sdk_t.hpp>
#include <kestrel/detached_clrmsg_store_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

// TODO: This class will eventually completely subsume old_config_t.

class config_t {

  std::string dir_{};
  common_sdk_t * sdk_{};

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  config_t() = delete;

  config_t(config_t const &) = delete;

  config_t & operator=(config_t const &) = delete;

  config_t(config_t &&) = delete;

  config_t & operator=(config_t &&) = delete;

  ~config_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // bootstrap
  //--------------------------------------------------------------------

private:

  std::string bootstrap_file_;

  bootstrap_config_t bootstrap_;

public:

  bootstrap_config_t & bootstrap() noexcept {
    return bootstrap_;
  }

  bootstrap_config_t const & bootstrap() const noexcept {
    return bootstrap_;
  }

  //--------------------------------------------------------------------
  // clear_deducible
  //--------------------------------------------------------------------

public:

  void clear_deducible(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // clrmsgs
  //--------------------------------------------------------------------

private:

  clrmsg_store_t clrmsgs_;

public:

  clrmsg_store_t & clrmsgs() {
    return clrmsgs_;
  }

  clrmsg_store_t const & clrmsgs() const {
    return clrmsgs_;
  }

  //--------------------------------------------------------------------
  // construct
  //--------------------------------------------------------------------
  //
  // If sdk is null, the system's filesystem will be used instead of the
  // SDK's filesystem.
  //

public:

  explicit config_t(tracing_event_t tev,
                    std::string dir,
                    common_sdk_t * sdk = nullptr);

  //--------------------------------------------------------------------
  // flush
  //--------------------------------------------------------------------
  //
  // Flushes the config to disk. Redundant flushes may be ignored.
  //

public:

  void flush(tracing_event_t tev, bool const pack = false);

  //--------------------------------------------------------------------
  // global
  //--------------------------------------------------------------------

private:

  std::string global_file_;

  global_config_t global_;

public:

  global_config_t & global() noexcept {
    return global_;
  }

  global_config_t const & global() const noexcept {
    return global_;
  }

  //--------------------------------------------------------------------
  // local
  //--------------------------------------------------------------------

private:

  local_config_t local_;

  std::string local_file_;

public:

  local_config_t & local() noexcept {
    return local_;
  }

  local_config_t const & local() const noexcept {
    return local_;
  }

  //--------------------------------------------------------------------
  // phonebook
  //--------------------------------------------------------------------

private:

  phonebook_t phonebook_;

public:

  phonebook_t & phonebook();

  phonebook_t const & phonebook() const;

  //--------------------------------------------------------------------
  // chunk_joiner
  //--------------------------------------------------------------------

private:

  chunk_joiner_t chunk_joiner_;

public:

  chunk_joiner_t & chunk_joiner() noexcept {
    return chunk_joiner_;
  }

  chunk_joiner_t const & chunk_joiner() const noexcept {
    return chunk_joiner_;
  }

  //--------------------------------------------------------------------
  // detached_clrmsg_store
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<detached_clrmsg_store_t> detached_clrmsg_store_{};

public:

  detached_clrmsg_store_t & detached_clrmsg_store(tracing_event_t tev);

  detached_clrmsg_store_t const &
  detached_clrmsg_store(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // prepare_to_bootstrap
  //--------------------------------------------------------------------

public:

  void prepare_to_bootstrap(tracing_event_t tev,
                            std::string dir,
                            channel_id_t channel_id,
                            link_address_t link_address);

  //--------------------------------------------------------------------
};

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_CONFIG_T_HPP
