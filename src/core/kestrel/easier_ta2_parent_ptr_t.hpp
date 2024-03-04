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

#ifndef KESTREL_EASIER_TA2_PARENT_PTR_T_HPP
#define KESTREL_EASIER_TA2_PARENT_PTR_T_HPP

#include <memory>
#include <mutex>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>

namespace kestrel {

template<class Parent>
class easier_ta2_parent_ptr_t final {

  template<class, class, class, class>
  friend class easier_ta2_package_t;

  std::weak_ptr<std::recursive_mutex> mutex_;
  std::weak_ptr<Parent> parent_;

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

public:

  easier_ta2_parent_ptr_t(std::shared_ptr<Parent> const parent) noexcept
      : mutex_((SST_ASSERT((parent != nullptr)),
                parent->plugin().primary_mutex())),
        parent_(parent) {
  }

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  easier_ta2_parent_ptr_t() = delete;

  ~easier_ta2_parent_ptr_t() SST_NOEXCEPT(true) = default;

  easier_ta2_parent_ptr_t(easier_ta2_parent_ptr_t const &)
      SST_NOEXCEPT(true) = default;

  easier_ta2_parent_ptr_t & operator=(easier_ta2_parent_ptr_t const &)
      SST_NOEXCEPT(true) = default;

  easier_ta2_parent_ptr_t(easier_ta2_parent_ptr_t &&)
      SST_NOEXCEPT(true) = default;

  easier_ta2_parent_ptr_t & operator=(easier_ta2_parent_ptr_t &&)
      SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // lock
  //--------------------------------------------------------------------

public:

  class scope_t final {

    friend class easier_ta2_parent_ptr_t;

    std::shared_ptr<std::recursive_mutex> mutex_;
    std::unique_lock<std::recursive_mutex> lock_;
    std::shared_ptr<Parent> parent_;

    scope_t(easier_ta2_parent_ptr_t & ptr) noexcept {
      mutex_ = ptr.mutex_.lock();
      if (mutex_) {
        lock_ = std::unique_lock<std::recursive_mutex>(*mutex_);
        parent_ = ptr.parent_.lock();
        if (!parent_) {
          lock_.unlock();
          lock_.release();
          mutex_.reset();
        }
      }
    }

  public:

    operator bool() const noexcept {
      return parent_ != nullptr;
    }

    Parent & parent() noexcept {
      SST_ASSERT((parent_ != nullptr));
      return *parent_;
    }

  }; //

  scope_t lock() & noexcept {
    return *this;
  }

  void lock() const && = delete;

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_EASIER_TA2_PARENT_PTR_T_HPP
