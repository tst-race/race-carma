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

#ifndef KESTREL_CARMA_BOOTSTRAP_CONFIG_T_HPP
#define KESTREL_CARMA_BOOTSTRAP_CONFIG_T_HPP

#include <string>
#include <vector>

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/moved_from.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/common_sdk_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

class bootstrap_config_t final {

  sst::moved_from moved_from_;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  bootstrap_config_t() = default;

  bootstrap_config_t(bootstrap_config_t const &) = default;

  bootstrap_config_t & operator=(bootstrap_config_t const &) = default;

  bootstrap_config_t(bootstrap_config_t &&)
      SST_NOEXCEPT(true) = default;

  bootstrap_config_t & operator=(bootstrap_config_t &&)
      SST_NOEXCEPT(true) = default;

  ~bootstrap_config_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Construction from a file
  //--------------------------------------------------------------------

public:

  explicit bootstrap_config_t(tracing_event_t tev,
                              common_sdk_t * sdk,
                              std::string const & file);

  //--------------------------------------------------------------------
  // bootstrapper
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<psn_t> bootstrapper_;

  void parse_bootstrapper(json_t & src);

  void unparse_bootstrapper(json_t & dst) const;

public:

  psn_t const & bootstrapper() const;

  void set_bootstrapper(psn_t src);

  //--------------------------------------------------------------------
  // channel_id
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<channel_id_t> channel_id_;

  void parse_channel_id(json_t & src);

  void unparse_channel_id(json_t & dst) const;

public:

  channel_id_t const & channel_id() const;

  void set_channel_id(channel_id_t src);

  //--------------------------------------------------------------------
  // exists
  //--------------------------------------------------------------------

private:

  bool exists_ = false;

public:

  bool exists() const noexcept {
    SST_ASSERT((!moved_from_));
    return exists_;
  }

  void exists(bool const src) noexcept {
    SST_ASSERT((!moved_from_));
    exists_ = src;
  }

  //--------------------------------------------------------------------
  // link_address
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<link_address_t> link_address_;

  void parse_link_address(json_t & src);

  void unparse_link_address(json_t & dst) const;

public:

  link_address_t const & link_address() const;

  void set_link_address(link_address_t src);

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, bootstrap_config_t & dst);

  friend void to_json(json_t & dst, bootstrap_config_t const & src);

  //--------------------------------------------------------------------
};

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_BOOTSTRAP_CONFIG_T_HPP
