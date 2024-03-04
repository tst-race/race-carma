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

#ifndef KESTREL_KESTREL_CLI_ARGS_T_HPP
#define KESTREL_KESTREL_CLI_ARGS_T_HPP

#include <string>

#include <sst/catalog/SST_NODISCARD.hpp>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

class kestrel_cli_args_t final {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  kestrel_cli_args_t() = default;

  ~kestrel_cli_args_t() SST_NOEXCEPT(true) = default;

  kestrel_cli_args_t(kestrel_cli_args_t const &) = default;

  kestrel_cli_args_t & operator=(kestrel_cli_args_t const &) = default;

  kestrel_cli_args_t(kestrel_cli_args_t &&)
      SST_NOEXCEPT(true) = default;

  kestrel_cli_args_t & operator=(kestrel_cli_args_t &&)
      SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // parse_state_t
  //--------------------------------------------------------------------
  //
  // Keeps track of which parse_*() functions have already been called
  // during a from_json() call.
  //

private:

  struct parse_state_t {
    bool called_parse_work_dir_ = false;
  };

  //--------------------------------------------------------------------
  // work_dir
  //--------------------------------------------------------------------

private:

  static std::string default_work_dir() {
    return ".";
  }

  sst::unique_ptr<std::string> work_dir_{sst::in_place,
                                         default_work_dir()};

  void parse_work_dir(json_t & src, parse_state_t & state);

  void unparse_work_dir(json_t & dst) const;

public:

  SST_NODISCARD() std::string const & work_dir() const;

  kestrel_cli_args_t & work_dir(std::string const & src);

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

public:

  friend void from_json(json_t src, kestrel_cli_args_t & dst);

  friend void to_json(json_t & dst, kestrel_cli_args_t const & src);

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_KESTREL_CLI_ARGS_T_HPP
