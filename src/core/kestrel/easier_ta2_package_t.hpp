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

#ifndef KESTREL_EASIER_TA2_PACKAGE_T_HPP
#define KESTREL_EASIER_TA2_PACKAGE_T_HPP

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/atomic.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/easier_ta2_parent_ptr_t.hpp>
#include <kestrel/package_status_t.hpp>
#include <kestrel/ta2_sdk_t.hpp>

namespace kestrel {

template<class Plugin, class Link, class Connection, class Package>
class easier_ta2_package_t {

  //--------------------------------------------------------------------
  // Fully derived *this
  //--------------------------------------------------------------------

protected:

  Package & package() noexcept {
    return static_cast<Package &>(*this);
  }

  Package const & package() const noexcept {
    return static_cast<Package const &>(*this);
  }

  //--------------------------------------------------------------------
  // connection
  //--------------------------------------------------------------------
  //
  // The connection this package belongs to.
  //

private:

  Connection * connection_;

protected:

  SST_NODISCARD()
  Connection & connection() noexcept {
    SST_ASSERT((connection_ != nullptr));
    return *connection_;
  }

  SST_NODISCARD()
  Connection const & connection() const noexcept {
    SST_ASSERT((connection_ != nullptr));
    return *connection_;
  }

  //--------------------------------------------------------------------
  // link
  //--------------------------------------------------------------------
  //
  // The link this package belongs to.
  //

protected:

  SST_NODISCARD()
  Link & link() noexcept {
    return this->connection().link();
  }

  SST_NODISCARD()
  Link const & link() const noexcept {
    return this->connection().link();
  }

  //--------------------------------------------------------------------
  // plugin
  //--------------------------------------------------------------------
  //
  // The plugin this package belongs to.
  //

public:

  SST_NODISCARD()
  Plugin & plugin() noexcept {
    return this->link().plugin();
  }

  SST_NODISCARD()
  Plugin const & plugin() const noexcept {
    return this->link().plugin();
  }

  //--------------------------------------------------------------------
  // sdk
  //--------------------------------------------------------------------
  //
  // The SDK instance of the plugin this package belongs to.
  //

protected:

  SST_NODISCARD()
  ta2_sdk_t & sdk() noexcept {
    return this->plugin().sdk();
  }

  SST_NODISCARD()
  ta2_sdk_t const & sdk() const noexcept {
    return this->plugin().sdk();
  }

  //--------------------------------------------------------------------
  // handle
  //--------------------------------------------------------------------
  //
  // The handle this package should use for its onPackageStatusChanged
  // call.
  //

private:

  race_handle_t handle_;

public:

  race_handle_t const & handle() const noexcept {
    return handle_;
  }

  //--------------------------------------------------------------------
  // data
  //--------------------------------------------------------------------
  //
  // The content of this package.
  //

private:

  bytes_t data_;

public:

  bytes_t const & data() const noexcept {
    return data_;
  }

  //--------------------------------------------------------------------
  // lookup
  //--------------------------------------------------------------------

private:

  using lookup_t =
      typename std::map<race_handle_t,
                        std::shared_ptr<Package>>::iterator;

  sst::optional<lookup_t> lookup_;

public:

  void set_lookup(lookup_t const & lookup) noexcept {
    lookup_ = lookup;
  }

  lookup_t const & lookup() const noexcept {
    SST_ASSERT((lookup_));
    return *lookup_;
  }

  //--------------------------------------------------------------------
  // ptr
  //--------------------------------------------------------------------

public:

  easier_ta2_parent_ptr_t<Package> ptr() const noexcept {
    return this->lookup()->second;
  }

  //--------------------------------------------------------------------
  // fire_status_event
  //--------------------------------------------------------------------

private:

  bool fired_ = false;

public:

  void fire_status_event(tracing_event_t tev,
                         package_status_t const & status) noexcept {
    SST_TEV_TOP(tev);
    if (!this->fired_) {
      try {
        this->sdk().onPackageStatusChanged(SST_TEV_ARG(tev),
                                           handle().value(),
                                           status.value(),
                                           0);
      } catch (...) {
        ;
      }
      this->connection().packages().erase(this->lookup());
      this->fired_ = true;
    }
    SST_TEV_BOT(tev);
  }

  void fire_status_event(package_status_t const & status) noexcept {
    tracing_event_t SST_TEV_DEF(tev);
    fire_status_event(SST_TEV_ARG(tev), status);
  }

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

public:

  explicit easier_ta2_package_t(Connection & connection,
                                race_handle_t const & handle,
                                bytes_t const & data)
      : connection_(&connection),
        handle_(handle),
        data_(data) {
  }

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

protected:

  easier_ta2_package_t() = delete;

  virtual ~easier_ta2_package_t() SST_NOEXCEPT(true) = default;

  easier_ta2_package_t(easier_ta2_package_t const &) = delete;

  easier_ta2_package_t &
  operator=(easier_ta2_package_t const &) = delete;

  easier_ta2_package_t(easier_ta2_package_t &&) = delete;

  easier_ta2_package_t & operator=(easier_ta2_package_t &&) = delete;

  //--------------------------------------------------------------------
  // send
  //--------------------------------------------------------------------
  //
  // This is the function you must implement to initiate the sending of
  // this package. You can call this->data() to get the data to be sent,
  // and you can call this->connection(), this->link(), and
  // this->plugin() to access anything else you need. Typically you'll
  // look at this->link() to determine where to actually send the data.
  //
  // Your function should not run a blocking send. It should only
  // initiate the send, which should run asynchronously somehow. You
  // should also call this->ptr() to get a weak pointer to the package
  // and store that in your asynchronous worker. Then, once your worker
  // finishes, e.g., in a callback on an unknown thread, you can use the
  // weak pointer to safely access the package (and the entire plugin
  // state) and possibly fire the package status event:
  //
  //       auto const scope = ptr.lock();
  //       if (scope) {
  //         auto & package = scope.parent();
  //         // Here you have exclusive access to the package
  //         // and the entire plugin state through package,
  //         // package->connection(), package->link(), and
  //         // package->plugin(). When your package is done
  //         // (either successfully or not), you should call
  //         // package->fire_status_event() with the proper
  //         // package_status_t value. Below is an example.
  //         package.fire_status_event(package_status_t::sent());
  //       } else {
  //         // This means the plugin or link or connection or
  //         // package went away in the meantime. Do nothing.
  //       }
  //
  // As an example, if you need to chunk large packages because of
  // limitations in your underlying channel, you could initiate all
  // chunk sends in send(), then call fire_status_event() after all
  // chunk sends have completed.
  //

public:

  virtual void send() = 0;

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_EASIER_TA2_PACKAGE_T_HPP
