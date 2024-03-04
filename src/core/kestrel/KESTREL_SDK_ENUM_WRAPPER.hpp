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

#ifndef KESTREL_KESTREL_SDK_ENUM_WRAPPER_HPP
#define KESTREL_KESTREL_SDK_ENUM_WRAPPER_HPP

#include <stdexcept>
#include <string>

#include <sst/catalog/SST_UNREACHABLE.hpp>
#include <sst/catalog/boxed.hpp>
#include <sst/catalog/enable_if_t.hpp>
#include <sst/catalog/is_integer.hpp>
#include <sst/catalog/perfect_eq.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/underlying_type_t.hpp>

// TODO: Can we use SST_STRONG_ENUM_CLASS here, or does it lack some
//       features we need? It would eliminate our *_FROM_JSON and
//       *_TO_JSON generators too, since it defines those inline.

#define KESTREL_SDK_ENUM_WRAPPER_TO_STRING_ITEM(A, B)                  \
  case B:                                                              \
    return #B;

#include <kestrel/json_t.hpp>

#define KESTREL_SDK_ENUM_WRAPPER_ITEM_1(A, B)                          \
  static constexpr KESTREL_SDK_ENUM_WRAPPER_NAME A() noexcept {        \
    return KESTREL_SDK_ENUM_WRAPPER_NAME(B);                           \
  }

#define KESTREL_SDK_ENUM_WRAPPER_ITEM_2(A, B) A = B,

#define KESTREL_SDK_ENUM_WRAPPER_ITEM_3(A, B)                          \
  ::sst::perfect_eq(x, B) ? B:

#define KESTREL_SDK_ENUM_WRAPPER(NAME, ITEMS, TYPE, ZERO)              \
                                                                       \
  class NAME : sst::boxed<TYPE, NAME> {                                \
                                                                       \
    using KESTREL_SDK_ENUM_WRAPPER_NAME = NAME;                        \
    using boxed = sst::boxed<TYPE, NAME>;                              \
    friend class sst::boxed<TYPE, NAME>;                               \
                                                                       \
    /*------------------------------------------------------------*/   \
    /* Inherited operations                                       */   \
    /*------------------------------------------------------------*/   \
                                                                       \
  private:                                                             \
                                                                       \
    using boxed::boxed;                                                \
                                                                       \
  public:                                                              \
                                                                       \
    using boxed::operator!=;                                           \
    using boxed::operator==;                                           \
    using boxed::value;                                                \
                                                                       \
    /*------------------------------------------------------------*/   \
    /* Default operations                                         */   \
    /*------------------------------------------------------------*/   \
                                                                       \
  public:                                                              \
                                                                       \
    NAME() noexcept : boxed(ZERO) {}                                   \
                                                                       \
    NAME(NAME const &) noexcept = default;                             \
                                                                       \
    NAME & operator=(NAME const &) noexcept = default;                 \
                                                                       \
    NAME(NAME &&) noexcept = default;                                  \
                                                                       \
    NAME & operator=(NAME &&) noexcept = default;                      \
                                                                       \
    ~NAME() noexcept = default;                                        \
                                                                       \
    /*------------------------------------------------------------*/   \
    /* Integer parsing                                            */   \
    /*------------------------------------------------------------*/   \
                                                                       \
  private:                                                             \
                                                                       \
    template<class T>                                                  \
    static constexpr TYPE parse_integer(T const x) {                   \
      return ITEMS(KESTREL_SDK_ENUM_WRAPPER_ITEM_3) throw ::std::      \
          runtime_error("Invalid " #TYPE " value: "                    \
                        + ::sst::to_string(x));                        \
    }                                                                  \
                                                                       \
  public:                                                              \
                                                                       \
    template<class T,                                                  \
             ::sst::enable_if_t<::sst::is_integer<T>::value> = 0>      \
    explicit NAME(T const x) : boxed(parse_integer(x)) {}              \
                                                                       \
    /*------------------------------------------------------------*/   \
    /* Named values                                               */   \
    /*------------------------------------------------------------*/   \
                                                                       \
  public:                                                              \
                                                                       \
    ITEMS(KESTREL_SDK_ENUM_WRAPPER_ITEM_1)                             \
                                                                       \
    /*------------------------------------------------------------*/   \
    /* Switch statement support                                   */   \
    /*------------------------------------------------------------*/   \
                                                                       \
  private:                                                             \
                                                                       \
    enum class enum_t : sst::underlying_type_t<TYPE> {                 \
      ITEMS(KESTREL_SDK_ENUM_WRAPPER_ITEM_2)                           \
    };                                                                 \
                                                                       \
  public:                                                              \
                                                                       \
    constexpr operator enum_t() const noexcept {                       \
      return static_cast<enum_t>(value());                             \
    }                                                                  \
                                                                       \
    /*------------------------------------------------------------*/   \
    /* String conversions                                         */   \
    /*------------------------------------------------------------*/   \
                                                                       \
  public:                                                              \
                                                                       \
    ::std::string to_string() const {                                  \
      switch (value()) {                                               \
        ITEMS(KESTREL_SDK_ENUM_WRAPPER_TO_STRING_ITEM)                 \
      }                                                                \
      SST_UNREACHABLE();                                               \
    }                                                                  \
                                                                       \
    /*------------------------------------------------------------*/   \
    /* JSON conversions                                           */   \
    /*------------------------------------------------------------*/   \
                                                                       \
  public:                                                              \
                                                                       \
    friend void to_json(json_t & dst, NAME const & src);               \
                                                                       \
    friend void from_json(json_t const & src, NAME & dst);             \
                                                                       \
    /*------------------------------------------------------------*/   \
  };

#endif // #ifndef KESTREL_KESTREL_SDK_ENUM_WRAPPER_HPP
