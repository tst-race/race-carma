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

#ifndef KESTREL_OPEN_CONNECTION_CALL_T_HPP
#define KESTREL_OPEN_CONNECTION_CALL_T_HPP

// CARMA headers
#include <kestrel/basic_call_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/race_handle_t.hpp>

namespace kestrel {

class open_connection_call_t : public basic_call_t {

  //--------------------------------------------------------------------
  // link_id
  //--------------------------------------------------------------------

private:
  link_id_t link_id_;

public:
  link_id_t const & link_id() const;

  //--------------------------------------------------------------------

public:
  explicit open_connection_call_t(race_handle_t const & handle,
                                  link_id_t const & link_id);

  ~open_connection_call_t() = default;

  open_connection_call_t(open_connection_call_t const &) = delete;
  open_connection_call_t(open_connection_call_t &&) = delete;
  open_connection_call_t &
  operator=(open_connection_call_t const &) = delete;
  open_connection_call_t &
  operator=(open_connection_call_t &&) = delete;

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_OPEN_CONNECTION_CALL_T_HPP
