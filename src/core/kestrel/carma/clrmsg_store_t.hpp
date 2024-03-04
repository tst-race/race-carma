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

#ifndef KESTREL_CARMA_CLRMSG_STORE_T_HPP
#define KESTREL_CARMA_CLRMSG_STORE_T_HPP

#include <memory>
#include <string>
#include <vector>

#include <sst/catalog/moved_from.hpp>

#include <kestrel/carma/clrmsg_store_entry_t.hpp>
#include <kestrel/common_sdk_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

class clrmsg_store_t final {

  //--------------------------------------------------------------------
  // add
  //--------------------------------------------------------------------

public:

  clrmsg_store_entry_t & add(tracing_event_t tev,
                             clrmsg_store_entry_t entry);

  //--------------------------------------------------------------------
  // construct
  //--------------------------------------------------------------------
  //
  // If sdk is null, the system's filesystem will be used instead of the
  // SDK's filesystem.
  //

public:

  explicit clrmsg_store_t(tracing_event_t tev,
                          std::string dir,
                          common_sdk_t * sdk = nullptr);

  //--------------------------------------------------------------------
  // copy-assign
  //--------------------------------------------------------------------

public:

  clrmsg_store_t & operator=(clrmsg_store_t const & other) = delete;

  //--------------------------------------------------------------------
  // copy-construct
  //--------------------------------------------------------------------

public:

  clrmsg_store_t(clrmsg_store_t const & other) = delete;

  //--------------------------------------------------------------------
  // destruct
  //--------------------------------------------------------------------

public:

  ~clrmsg_store_t() noexcept;

  //--------------------------------------------------------------------
  // dir_
  //--------------------------------------------------------------------

private:

  std::string dir_;

  //--------------------------------------------------------------------
  // entries_
  //--------------------------------------------------------------------
  //
  // TODO: This data structure is the simplest possible thing that lets
  //       us implement add(). It will have to change as we implement
  //       more functionality.
  //

private:

  std::unique_ptr<std::vector<clrmsg_store_entry_t>> entries_;

  //--------------------------------------------------------------------
  // flush
  //--------------------------------------------------------------------

public:

  void flush(tracing_event_t tev);

  //--------------------------------------------------------------------
  // move-assign
  //--------------------------------------------------------------------

public:

  clrmsg_store_t & operator=(clrmsg_store_t && other) = delete;

  //--------------------------------------------------------------------
  // move-construct
  //--------------------------------------------------------------------

public:

  clrmsg_store_t(clrmsg_store_t && other) noexcept;

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

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_CLRMSG_STORE_T_HPP
