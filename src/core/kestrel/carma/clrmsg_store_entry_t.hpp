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

#ifndef KESTREL_CARMA_CLRMSG_STORE_ENTRY_T_HPP
#define KESTREL_CARMA_CLRMSG_STORE_ENTRY_T_HPP

#include <sst/catalog/moved_from.hpp>

#include <kestrel/clrmsg_t.hpp>

namespace kestrel {
namespace carma {

class clrmsg_store_entry_t final {

  //--------------------------------------------------------------------
  // clrmsg
  //--------------------------------------------------------------------

private:

  clrmsg_t clrmsg_;
  bool clrmsg_done_ = false;

public:

  clrmsg_t const & clrmsg() const;

  //--------------------------------------------------------------------
  // construct
  //--------------------------------------------------------------------

public:

  explicit clrmsg_store_entry_t(clrmsg_t clrmsg);

  //--------------------------------------------------------------------
  // copy-assign
  //--------------------------------------------------------------------

public:

  clrmsg_store_entry_t & operator=(clrmsg_store_entry_t const & other);

  //--------------------------------------------------------------------
  // copy-construct
  //--------------------------------------------------------------------

public:

  clrmsg_store_entry_t(clrmsg_store_entry_t const & other);

  //--------------------------------------------------------------------
  // dirty
  //--------------------------------------------------------------------

private:

  bool dirty_ = true;

public:

  bool dirty() const;

  clrmsg_store_entry_t & dirty(bool src);

  //--------------------------------------------------------------------
  // destruct
  //--------------------------------------------------------------------

public:

  ~clrmsg_store_entry_t() noexcept;

  //--------------------------------------------------------------------
  // move-assign
  //--------------------------------------------------------------------

public:

  clrmsg_store_entry_t &
  operator=(clrmsg_store_entry_t && other) noexcept;

  //--------------------------------------------------------------------
  // move-construct
  //--------------------------------------------------------------------

public:

  clrmsg_store_entry_t(clrmsg_store_entry_t && other) noexcept;

  //--------------------------------------------------------------------
  // moved_from_
  //--------------------------------------------------------------------

private:

  sst::moved_from moved_from_;

  //--------------------------------------------------------------------
};

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_CLRMSG_STORE_ENTRY_T_HPP
