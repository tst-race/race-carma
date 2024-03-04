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

#ifndef KESTREL_RABBITMQ_LINK_ADDRINFO_TEMPLATE_T_HPP
#define KESTREL_RABBITMQ_LINK_ADDRINFO_TEMPLATE_T_HPP

#include <string>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/SST_STATIC_ASSERT.h>
#include <sst/catalog/c_quote.hpp>
#include <sst/catalog/crypto_rng.hpp>
#include <sst/catalog/in_place.hpp>
#include <sst/catalog/moved_from.hpp>
#include <sst/catalog/to_hex.hpp>
#include <sst/catalog/to_integer.hpp>
#include <sst/catalog/to_integer_exception.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/link_address_t.hpp>
#include <kestrel/link_role_t.hpp>
#include <kestrel/rabbitmq/invalid_link_address.hpp>
#include <kestrel/simple_kv_reader.hpp>
#include <kestrel/simple_kv_writer.hpp>

namespace kestrel {
namespace rabbitmq {

template<char const * DefaultHost, unsigned int DefaultPort>
class link_addrinfo_template_t final {

  SST_STATIC_ASSERT((DefaultHost));
  SST_STATIC_ASSERT((DefaultPort >= 1U));
  SST_STATIC_ASSERT((DefaultPort <= 65535U));

  friend class worker_key_cmp_t;

  //--------------------------------------------------------------------
  // Moved-from detection
  //--------------------------------------------------------------------

private:

  sst::moved_from moved_from_;

  //--------------------------------------------------------------------
  // role
  //--------------------------------------------------------------------

private:

  link_role_t role_ = link_role_t::creator();

public:

  SST_NODISCARD()
  link_role_t role() const noexcept {
    SST_ASSERT((!moved_from_));
    return role_;
  }

  link_addrinfo_template_t & role(link_role_t const r) {
    SST_ASSERT((!moved_from_));
    role_ = r;
    return *this;
  }

  //--------------------------------------------------------------------
  // host
  //--------------------------------------------------------------------

private:

  SST_NODISCARD()
  static constexpr char const * default_host() noexcept {
    return DefaultHost;
  }

  sst::unique_ptr<std::string> host_{sst::in_place, default_host()};

  SST_NODISCARD()
  static bool validate_host(std::string const & h) {
    return !h.empty() && h.find('\0') == h.npos;
  }

public:

  SST_NODISCARD()
  std::string const & host() const noexcept {
    SST_ASSERT((!moved_from_));
    return *host_.get();
  }

  link_addrinfo_template_t & host(std::string const & h) {
    SST_ASSERT((!moved_from_));
    SST_ASSERT((validate_host(h)));
    *host_ = h;
    return *this;
  }

  //--------------------------------------------------------------------
  // port
  //--------------------------------------------------------------------

private:

  SST_NODISCARD()
  static constexpr unsigned int default_port() noexcept {
    return DefaultPort;
  }

  unsigned int port_ = default_port();

  SST_NODISCARD()
  static constexpr bool validate_port(unsigned int const p) noexcept {
    return p >= 1U && p <= 65535U;
  }

public:

  SST_NODISCARD()
  unsigned int port() const noexcept {
    SST_ASSERT((!moved_from_));
    return port_;
  }

  link_addrinfo_template_t & port(unsigned int const p) {
    SST_ASSERT((!moved_from_));
    SST_ASSERT((validate_port(p)));
    port_ = p;
    return *this;
  }

  //--------------------------------------------------------------------
  // username
  //--------------------------------------------------------------------

private:

  SST_NODISCARD()
  static constexpr char const * default_username() noexcept {
    return "stealth";
  }

  sst::unique_ptr<std::string> username_{sst::in_place,
                                         default_username()};

  SST_NODISCARD()
  static bool validate_username(std::string const & u) {
    return !u.empty() && u.find('\0') == u.npos;
  }

public:

  SST_NODISCARD()
  std::string const & username() const noexcept {
    SST_ASSERT((!moved_from_));
    return *username_.get();
  }

  link_addrinfo_template_t & username(std::string const & u) {
    SST_ASSERT((!moved_from_));
    SST_ASSERT((validate_username(u)));
    *username_ = u;
    return *this;
  }

  //--------------------------------------------------------------------
  // password
  //--------------------------------------------------------------------

private:

  SST_NODISCARD()
  static constexpr char const * default_password() noexcept {
    return "SCV7YznrPqju";
  }

  sst::unique_ptr<std::string> password_{sst::in_place,
                                         default_password()};

  SST_NODISCARD()
  static bool validate_password(std::string const & a) {
    return !a.empty() && a.find('\0') == a.npos;
  }

public:

  SST_NODISCARD()
  std::string const & password() const noexcept {
    SST_ASSERT((!moved_from_));
    return *password_.get();
  }

  link_addrinfo_template_t & password(std::string const & a) {
    SST_ASSERT((!moved_from_));
    SST_ASSERT((validate_password(a)));
    *password_ = a;
    return *this;
  }

  //--------------------------------------------------------------------
  // vhost
  //--------------------------------------------------------------------

private:

  SST_NODISCARD()
  static constexpr char const * default_vhost() noexcept {
    return "/";
  }

  sst::unique_ptr<std::string> vhost_{sst::in_place, default_vhost()};

  SST_NODISCARD()
  static bool validate_vhost(std::string const & v) {
    return !v.empty() && v.find('\0') == v.npos;
  }

public:

  SST_NODISCARD()
  std::string const & vhost() const noexcept {
    SST_ASSERT((!moved_from_));
    return *vhost_.get();
  }

  link_addrinfo_template_t & vhost(std::string const & v) {
    SST_ASSERT((!moved_from_));
    SST_ASSERT((validate_vhost(v)));
    *vhost_ = v;
    return *this;
  }

  //--------------------------------------------------------------------
  // queue
  //--------------------------------------------------------------------

private:

  sst::unique_ptr<std::string> queue_{sst::in_place};

  SST_NODISCARD()
  static bool validate_queue(std::string const & q) {
    return !q.empty() && q.find('\0') == q.npos;
  }

public:

  SST_NODISCARD()
  std::string const & queue() const noexcept {
    SST_ASSERT((!moved_from_));
    return *queue_.get();
  }

  link_addrinfo_template_t & queue(std::string const & q) {
    SST_ASSERT((!moved_from_));
    SST_ASSERT((validate_queue(q)));
    *queue_ = q;
    return *this;
  }

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  link_addrinfo_template_t()
      : queue_(sst::in_place, "q" + sst::to_hex(sst::crypto_rng(16))) {
    SST_ASSERT((validate_queue(*queue_)));
  }

  ~link_addrinfo_template_t() noexcept = default;

  link_addrinfo_template_t(link_addrinfo_template_t const &) = default;

  link_addrinfo_template_t &
  operator=(link_addrinfo_template_t const &) = default;

  link_addrinfo_template_t(link_addrinfo_template_t &&) noexcept =
      default;

  link_addrinfo_template_t &
  operator=(link_addrinfo_template_t &&) noexcept = default;

  //--------------------------------------------------------------------
  // link_address_t conversions
  //--------------------------------------------------------------------

public:

  SST_NODISCARD()
  link_address_t to_link_address() const {
    SST_ASSERT((!moved_from_));
    simple_kv_writer w;
    w.write("R", role() == link_role_t::creator() ? "C" : "L");
    if (host() != default_host()) {
      w.write("H", host());
    }
    if (port() != default_port()) {
      w.write("P", port());
    }
    if (username() != default_username()) {
      w.write("U", username());
    }
    if (password() != default_password()) {
      w.write("A", password());
    }
    if (vhost() != default_vhost()) {
      w.write("V", vhost());
    }
    w.write("Q", queue());
    return link_address_t(std::move(w.get()));
  }

  explicit link_addrinfo_template_t(link_address_t const & address) {
    bool have_role = false;
    bool have_queue = false;
    simple_kv_reader kv(&address.value());
    while (kv.read()) {
      if (kv.key() == "R") {
        if (kv.value() == "C") {
          role(link_role_t::creator());
        } else if (kv.value() == "L") {
          role(link_role_t::loader());
        } else {
          throw invalid_link_address(
              "Invalid role: " + sst::c_quote(kv.value()) + ".");
        }
        have_role = true;
      } else if (kv.key() == "H") {
        if (!validate_host(kv.value())) {
          throw invalid_link_address(
              "Invalid host: " + sst::c_quote(kv.value()) + ".");
        }
        host(kv.value());
      } else if (kv.key() == "P") {
        try {
          unsigned int const p =
              sst::to_integer<unsigned int>(kv.value());
          if (!validate_port(p)) {
            throw invalid_link_address(
                "Invalid port: " + sst::c_quote(kv.value()) + ".");
          }
          port(p);
        } catch (sst::to_integer_exception const & e) {
          std::throw_with_nested(invalid_link_address(
              "Invalid port: " + sst::c_quote(kv.value()) + "."));
        }
      } else if (kv.key() == "U") {
        if (!validate_username(kv.value())) {
          throw invalid_link_address(
              "Invalid username: " + sst::c_quote(kv.value()) + ".");
        }
        username(kv.value());
      } else if (kv.key() == "A") {
        if (!validate_password(kv.value())) {
          throw invalid_link_address(
              "Invalid password: " + sst::c_quote(kv.value()) + ".");
        }
        password(kv.value());
      } else if (kv.key() == "V") {
        if (!validate_vhost(kv.value())) {
          throw invalid_link_address(
              "Invalid vhost: " + sst::c_quote(kv.value()) + ".");
        }
        vhost(kv.value());
      } else if (kv.key() == "Q") {
        if (!validate_queue(kv.value())) {
          throw invalid_link_address(
              "Invalid queue: " + sst::c_quote(kv.value()) + ".");
        }
        queue(kv.value());
        have_queue = true;
      } else {
        throw invalid_link_address(
            "Unknown key: " + sst::c_quote(kv.key()) + ".");
      }
    }
    if (!have_role) {
      throw invalid_link_address("Missing host.");
    }
    if (!have_queue) {
      throw invalid_link_address("Missing queue.");
    }
  }

  //--------------------------------------------------------------------
  // Comparisons
  //--------------------------------------------------------------------

public:

  SST_NODISCARD()
  int compare(link_addrinfo_template_t const & b) const noexcept {
    link_addrinfo_template_t const & a = *this;
    SST_ASSERT((!a.moved_from_));
    SST_ASSERT((!b.moved_from_));
    {
      if (a.role() < b.role()) {
        return -1;
      }
      if (a.role() > b.role()) {
        return 1;
      }
    }
    {
      int const x = a.host().compare(b.host());
      if (x != 0) {
        return x;
      }
    }
    {
      if (a.port() < b.port()) {
        return -1;
      }
      if (a.port() > b.port()) {
        return 1;
      }
    }
    {
      int const x = a.username().compare(b.username());
      if (x != 0) {
        return x;
      }
    }
    {
      int const x = a.password().compare(b.password());
      if (x != 0) {
        return x;
      }
    }
    {
      int const x = a.vhost().compare(b.vhost());
      if (x != 0) {
        return x;
      }
    }
    {
      int const x = a.queue().compare(b.queue());
      if (x != 0) {
        return x;
      }
    }
    return 0;
  }

  SST_NODISCARD()
  friend bool operator<(link_addrinfo_template_t const & a,
                        link_addrinfo_template_t const & b) noexcept {
    SST_ASSERT((!a.moved_from_));
    SST_ASSERT((!b.moved_from_));
    return a.compare(b) < 0;
  }

  SST_NODISCARD()
  friend bool operator>(link_addrinfo_template_t const & a,
                        link_addrinfo_template_t const & b) noexcept {
    SST_ASSERT((!a.moved_from_));
    SST_ASSERT((!b.moved_from_));
    return a.compare(b) > 0;
  }

  SST_NODISCARD()
  friend bool operator<=(link_addrinfo_template_t const & a,
                         link_addrinfo_template_t const & b) noexcept {
    SST_ASSERT((!a.moved_from_));
    SST_ASSERT((!b.moved_from_));
    return a.compare(b) <= 0;
  }

  SST_NODISCARD()
  friend bool operator>=(link_addrinfo_template_t const & a,
                         link_addrinfo_template_t const & b) noexcept {
    SST_ASSERT((!a.moved_from_));
    SST_ASSERT((!b.moved_from_));
    return a.compare(b) >= 0;
  }

  SST_NODISCARD()
  friend bool operator==(link_addrinfo_template_t const & a,
                         link_addrinfo_template_t const & b) noexcept {
    SST_ASSERT((!a.moved_from_));
    SST_ASSERT((!b.moved_from_));
    return a.compare(b) == 0;
  }

  SST_NODISCARD()
  friend bool operator!=(link_addrinfo_template_t const & a,
                         link_addrinfo_template_t const & b) noexcept {
    SST_ASSERT((!a.moved_from_));
    SST_ASSERT((!b.moved_from_));
    return a.compare(b) != 0;
  }

  //--------------------------------------------------------------------
};

} // namespace rabbitmq
} // namespace kestrel

#endif // #ifndef KESTREL_RABBITMQ_LINK_ADDRINFO_TEMPLATE_T_HPP
