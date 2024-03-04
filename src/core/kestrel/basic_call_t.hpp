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

#ifndef KESTREL_BASIC_CALL_T_HPP
#define KESTREL_BASIC_CALL_T_HPP

#include <sst/catalog/mono_time_ns.hpp>

#include <kestrel/race_handle_t.hpp>

namespace kestrel {

class basic_call_t {

  //--------------------------------------------------------------------
  // handle
  //--------------------------------------------------------------------

private:
  race_handle_t handle_;

public:
  race_handle_t const & handle() const;

  //--------------------------------------------------------------------
  // time
  //--------------------------------------------------------------------

private:
  decltype(sst::mono_time_ns()) time_;

public:
  decltype(sst::mono_time_ns()) time() const;

  //--------------------------------------------------------------------

public:
  explicit basic_call_t(race_handle_t const & handle);

  basic_call_t(basic_call_t const &) = delete;
  basic_call_t(basic_call_t &&) = delete;
  basic_call_t & operator=(basic_call_t const &) = delete;
  basic_call_t & operator=(basic_call_t &&) = delete;
  ~basic_call_t() = default;

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_BASIC_CALL_T_HPP
