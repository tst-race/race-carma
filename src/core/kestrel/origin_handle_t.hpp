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

#ifndef KESTREL_ORIGIN_HANDLE_T_HPP
#define KESTREL_ORIGIN_HANDLE_T_HPP

#include <kestrel/race_handle_t.hpp>
#include <utility>

namespace kestrel {

class origin_handle_t {
  race_handle_t handle_;

protected:
  void set_origin_handle(race_handle_t const & span);

public:
  template<class... Args>
  origin_handle_t(Args &&... args)
      : handle_(std::forward<Args>(args)...) {
  }

  race_handle_t const & origin_handle();
};

} // namespace kestrel

#endif // #ifndef KESTREL_ORIGIN_HANDLE_T_HPP
