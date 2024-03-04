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

#ifndef KESTREL_CHUNK_JOINER_T_HPP
#define KESTREL_CHUNK_JOINER_T_HPP

#include <map>
#include <string>

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/optional.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/common_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>

// TODO: We'll eventually need timestamping and a way to delete
//       incomplete groups that haven't seen updates in a long time.

namespace kestrel {

class chunk_joiner_t final {

  std::string dir_{};
  common_sdk_t * sdk_{};

  struct group_t {
    bytes_t::size_type count;
    std::map<bytes_t::size_type, bytes_t> chunks;
  };

  std::map<bytes_t, group_t> groups_{};

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  chunk_joiner_t() = delete;

  chunk_joiner_t(chunk_joiner_t const &) = delete;

  chunk_joiner_t & operator=(chunk_joiner_t const &) = delete;

  chunk_joiner_t(chunk_joiner_t const &&) = delete;

  chunk_joiner_t && operator=(chunk_joiner_t const &&) = delete;

  ~chunk_joiner_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

public:

  explicit chunk_joiner_t(tracing_event_t tev,
                          std::string dir,
                          common_sdk_t * sdk = nullptr);

  //--------------------------------------------------------------------
  // add
  //--------------------------------------------------------------------

public:

  sst::optional<bytes_t> add(tracing_event_t tev,
                             bytes_t const & id,
                             bytes_t::size_type index,
                             bytes_t::size_type count,
                             bytes_t chunk);

  //--------------------------------------------------------------------
  // flush
  //--------------------------------------------------------------------

public:

  void flush(tracing_event_t tev);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_CHUNK_JOINER_T_HPP
