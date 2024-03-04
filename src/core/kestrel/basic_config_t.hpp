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

#ifndef KESTREL_BASIC_CONFIG_T_HPP
#define KESTREL_BASIC_CONFIG_T_HPP

#include <memory>
#include <string>

#include <sst/catalog/moved_from.hpp>

#include <kestrel/common_sdk_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

//
// The basic_config_t class manages a TA1 plugin or TA2 channel config
// that consists of a single JSON file named config.json that's small
// enough to always keep in memory.
//

class basic_config_t {

  //--------------------------------------------------------------------
  // copy-assign
  //--------------------------------------------------------------------

public:

  basic_config_t & operator=(basic_config_t const & other) = delete;

  //--------------------------------------------------------------------
  // copy-construct
  //--------------------------------------------------------------------

public:

  basic_config_t(basic_config_t const & other) = delete;

  //--------------------------------------------------------------------
  // construct
  //--------------------------------------------------------------------
  //
  // If sdk is null, the system's filesystem will be used instead of the
  // SDK's filesystem.
  //

public:

  explicit basic_config_t(tracing_event_t tev,
                          std::string dir,
                          common_sdk_t * sdk = nullptr);

  //--------------------------------------------------------------------
  // destruct
  //--------------------------------------------------------------------

public:

  ~basic_config_t() noexcept;

  //--------------------------------------------------------------------
  // dir_
  //--------------------------------------------------------------------

private:

  std::string dir_;

  //--------------------------------------------------------------------
  // flush
  //--------------------------------------------------------------------
  //
  // Flushes the config to disk.
  //
  // Redundant flushes may be ignored. In other words, it is assumed
  // that the disk is not modified by external means between flushes.
  //

public:

  void flush(tracing_event_t tev);

  //--------------------------------------------------------------------
  // json
  //--------------------------------------------------------------------

private:

  std::unique_ptr<json_t> json_;
  std::string json_file_;

public:

  json_t & json(tracing_event_t tev);

  json_t const & json(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // move-assign
  //--------------------------------------------------------------------

public:

  basic_config_t & operator=(basic_config_t && other) = delete;

  //--------------------------------------------------------------------
  // move-construct
  //--------------------------------------------------------------------

public:

  basic_config_t(basic_config_t && other) noexcept;

  //--------------------------------------------------------------------
  // moved_from_
  //--------------------------------------------------------------------

private:

  sst::moved_from moved_from_;

  //--------------------------------------------------------------------
  // sdk_
  //--------------------------------------------------------------------

private:

  common_sdk_t * sdk_;

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_BASIC_CONFIG_T_HPP
