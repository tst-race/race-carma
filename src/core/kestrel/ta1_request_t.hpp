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

#ifndef KESTREL_TA1_REQUEST_T_HPP
#define KESTREL_TA1_REQUEST_T_HPP

#include <initializer_list>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <utility>

#include <sst/catalog/enable_if.hpp>

#include <kestrel/channel_list_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {

class ta1_request_t final {

public:
  ta1_request_t() = default;
  ta1_request_t(ta1_request_t const &) = default;
  ta1_request_t(ta1_request_t &&) = default;
  ta1_request_t & operator=(ta1_request_t const &) = default;
  ta1_request_t & operator=(ta1_request_t &&) = default;
  ~ta1_request_t() = default;

  //--------------------------------------------------------------------

  bool contains(psn_t const & sender,
                std::string const & channel,
                std::set<psn_t> const & recipients) const;

  //--------------------------------------------------------------------

  bool connects(
      psn_t const & sender,
      psn_t const & recipient,
      channel_list_t const & channel_list,
      std::initializer_list<connection_type_t> connection_types) const;

  //--------------------------------------------------------------------

  void add(psn_t const & sender,
           std::string const & channel,
           std::set<psn_t> const & recipients,
           ta1_request_t const * const filter = nullptr);

  //--------------------------------------------------------------------

  void add(psn_t const & sender,
           std::string const & channel,
           std::set<psn_t> && recipients,
           ta1_request_t const * const filter = nullptr);

  //--------------------------------------------------------------------

  template<class PersonaIterator1,
           class PersonaIterator2,
           sst::enable_if_t<std::is_base_of<
               std::input_iterator_tag,
               typename std::iterator_traits<
                   PersonaIterator1>::iterator_category>::value> = 0>
  void add(psn_t const & sender,
           std::string const & channel,
           PersonaIterator1 const recipients_first,
           PersonaIterator2 const recipients_last,
           ta1_request_t const * const filter = nullptr) {
    add(sender,
        channel,
        std::set<psn_t>(recipients_first, recipients_last),
        filter);
  }

  //--------------------------------------------------------------------

  template<class PersonaContainer,
           sst::enable_if_t<
               std::is_same<typename PersonaContainer::value_type,
                            psn_t>::value> = 0>
  void add(psn_t const & sender,
           std::string const & channel,
           PersonaContainer const & recipients,
           ta1_request_t const * const filter = nullptr) {
    add(sender, channel, recipients.begin(), recipients.end(), filter);
  }

  //--------------------------------------------------------------------

  void add(psn_t const & sender,
           std::string const & channel,
           psn_t const & recipient,
           ta1_request_t const * const filter = nullptr);

  //--------------------------------------------------------------------

  void add(psn_t const & sender,
           std::set<psn_t> const & recipients,
           channel_list_t const & channel_list,
           std::initializer_list<connection_type_t> connection_types,
           ta1_request_t const * const filter = nullptr);

  //--------------------------------------------------------------------

  template<class PersonaIterator1,
           class PersonaIterator2,
           sst::enable_if_t<std::is_base_of<
               std::input_iterator_tag,
               typename std::iterator_traits<
                   PersonaIterator1>::iterator_category>::value> = 0>
  void add(psn_t const & sender,
           PersonaIterator1 const recipients_first,
           PersonaIterator2 const recipients_last,
           channel_list_t const & channel_list,
           std::initializer_list<connection_type_t> connection_types,
           ta1_request_t const * const filter = nullptr) {
    add(sender,
        std::set<psn_t>(recipients_first, recipients_last),
        channel_list,
        connection_types,
        filter);
  }

  //--------------------------------------------------------------------

  template<class PersonaContainer,
           sst::enable_if_t<
               std::is_same<typename PersonaContainer::value_type,
                            psn_t>::value> = 0>
  void add(psn_t const & sender,
           PersonaContainer const & recipients,
           channel_list_t const & channel_list,
           std::initializer_list<connection_type_t> connection_types,
           ta1_request_t const * const filter = nullptr) {
    add(sender,
        recipients.begin(),
        recipients.end(),
        channel_list,
        connection_types,
        filter);
  }

  //--------------------------------------------------------------------

  void add(psn_t const & sender,
           psn_t const & recipient,
           channel_list_t const & channel_list,
           std::initializer_list<connection_type_t> connection_types,
           ta1_request_t const * const filter = nullptr);

  //--------------------------------------------------------------------
  // entries
  //--------------------------------------------------------------------
  //
  // Each entries_[sender][channel] is a set of recipient sets. A
  // recipient set with multiple entries is a multicast link request.
  //

public:

  using entries_t =
      std::map<psn_t,
               std::map<std::string, std::set<std::set<psn_t>>>>;

private:

  entries_t entries_;

public:

  entries_t const & entries() const noexcept {
    return entries_;
  }

  //--------------------------------------------------------------------

  static ta1_request_t from_json(nlohmann::json && src);
  static ta1_request_t from_json(nlohmann::json const & src);
  friend void from_json(nlohmann::json const & src,
                        ta1_request_t & dst);

  friend void to_json(nlohmann::json & dst, ta1_request_t const & src);
};

} // namespace kestrel

#endif // #ifndef KESTREL_TA1_REQUEST_T_HPP
