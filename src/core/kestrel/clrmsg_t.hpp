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

#ifndef KESTREL_CLRMSG_T_HPP
#define KESTREL_CLRMSG_T_HPP

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_STATIC_ASSERT.h>
#include <sst/catalog/buffer_overrun.hpp>
#include <sst/catalog/buffer_underrun.hpp>
#include <sst/catalog/char_bit_v.hpp>
#include <sst/catalog/checked_t.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/checked_resize.hpp>
#include <sst/catalog/copy_bytes.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/old/from_bytes.hpp>
#include <sst/catalog/old/to_bytes.hpp>
#include <sst/catalog/perfect_ge.hpp>
#include <sst/catalog/perfect_gt.hpp>
#include <sst/catalog/perfect_le.hpp>
#include <sst/catalog/remove_cvref_t.hpp>
#include <sst/catalog/unsigned_gt.hpp>

#include <ClrMsg.h>

#include <kestrel/generic_hash_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {

//
// This class can be pooled. After default constructing or
// acquiring an instance, call to_bytes_init() or from_bytes_init()
// depending on whether you're going to be calling to_bytes() or
// from_bytes().
//
// An instance of this class should never be accessed by multiple
// threads at the same time, even if the accesses are conceptually
// const.
//

//
// TODO: Use varint for prefix sizing. This will allow any sizes to be
//       used, but will still save bytes for small sizes, which is the
//       common case.
//

class clrmsg_t final {

  std::size_t msg_size_size_;
  std::size_t psn_size_size_;

  using msg_type = decltype(std::declval<ClrMsg>().getMsg());
  using from_type = decltype(std::declval<ClrMsg>().getFrom());
  using to_type = decltype(std::declval<ClrMsg>().getTo());
  using time_type = decltype(std::declval<ClrMsg>().getTime());
  using nonce_type = decltype(std::declval<ClrMsg>().getNonce());
  using amp_index_type = decltype(std::declval<ClrMsg>().getAmpIndex());
  using trace_id_type = decltype(std::declval<ClrMsg>().getTraceId());
  using span_id_type = decltype(std::declval<ClrMsg>().getSpanId());

  SST_STATIC_ASSERT((std::is_same<msg_type, std::string>::value));
  SST_STATIC_ASSERT((std::is_same<from_type, std::string>::value));
  SST_STATIC_ASSERT((std::is_same<to_type, std::string>::value));
  SST_STATIC_ASSERT((std::is_integral<time_type>::value));
  SST_STATIC_ASSERT((std::is_integral<nonce_type>::value));
  SST_STATIC_ASSERT((std::is_integral<amp_index_type>::value));
  SST_STATIC_ASSERT((std::is_integral<trace_id_type>::value));
  SST_STATIC_ASSERT((std::is_integral<span_id_type>::value));

  std::vector<unsigned char> msg_;
  std::vector<unsigned char> from_;
  std::vector<unsigned char> to_;
  time_type time_;
  nonce_type nonce_;
  amp_index_type amp_index_;
  trace_id_type trace_id_;
  span_id_type span_id_;

  mutable std::string msg_str_;
  mutable std::string from_str_;
  mutable std::string to_str_;

  template<class ByteIt, class Size2>
  static ByteIt buffer_to_bytes(std::vector<unsigned char> const & src,
                                ByteIt dst,
                                Size2 const size_size) {
    SST_ASSERT((sst::perfect_gt(size_size, 0)));
    dst = sst::old::to_bytes(src.size(),
                             dst,
                             size_size,
                             sst::integer_rep::pure_unsigned());
    dst = std::copy_n(src.data(), src.size(), dst);
    return dst;
  }

  template<class ByteIt, class Size2, class Avail>
  static ByteIt buffer_from_bytes(ByteIt src,
                                  Size2 const size_size,
                                  Avail & avail,
                                  std::vector<unsigned char> & dst) {
    SST_ASSERT((sst::perfect_gt(size_size, 0)));
    SST_ASSERT((sst::perfect_ge(avail, 0)));
    auto const dst_size = sst::old::from_bytes<decltype(dst.size())>(
        &src,
        size_size,
        avail,
        sst::integer_rep::pure_unsigned());
    dst.resize(dst_size);
    sst::copy_bytes(&src, dst.size(), avail, dst.data());
    return src;
  }

  //--------------------------------------------------------------------
  // construct-empty
  //--------------------------------------------------------------------
  //
  // TODO: This constructor should go away after we remove pooling.
  //

public:

  clrmsg_t() {
  }

  //--------------------------------------------------------------------
  // construct-from-ClrMsg
  //--------------------------------------------------------------------

public:

  clrmsg_t(ClrMsg const & src) {
    to_bytes_init(src);
  }

  //--------------------------------------------------------------------
  // recver
  //--------------------------------------------------------------------

private:

  psn_t recver_;

public:

  psn_t const & recver() const {
    return recver_;
  }

  //--------------------------------------------------------------------
  // sender
  //--------------------------------------------------------------------

private:

  psn_t sender_;

public:

  psn_t const & sender() const {
    return sender_;
  }

  //--------------------------------------------------------------------

public:
  bool operator==(clrmsg_t const & other) const {
    bool b = true;
    b = b && msg_ == other.msg_;
    b = b && from_ == other.from_;
    b = b && to_ == other.to_;
    b = b && time_ == other.time_;
    b = b && nonce_ == other.nonce_;
    b = b && amp_index_ == other.amp_index_;
    b = b && trace_id_ == other.trace_id_;
    b = b && span_id_ == other.span_id_;
    return b;
  }

  bool operator!=(clrmsg_t const & other) const {
    return !(*this == other);
  }

  //--------------------------------------------------------------------

  void to_bytes_init(ClrMsg const & src) {
    from_bytes_init();
    {
      auto const & msg = src.getMsg();
      sst::checked_resize(msg_, msg.size());
      std::copy_n(reinterpret_cast<unsigned char const *>(msg.data()),
                  msg.size(),
                  msg_.begin());
    }
    {
      auto const & from = src.getFrom();
      sst::checked_resize(from_, from.size());
      std::copy_n(reinterpret_cast<unsigned char const *>(from.data()),
                  from.size(),
                  from_.begin());
    }
    {
      auto const & to = src.getTo();
      sst::checked_resize(to_, to.size());
      std::copy_n(reinterpret_cast<unsigned char const *>(to.data()),
                  to.size(),
                  to_.begin());
    }
    time_ = src.getTime();
    nonce_ = src.getNonce();
    amp_index_ = src.getAmpIndex();
    trace_id_ = src.getTraceId();
    span_id_ = src.getSpanId();
    sender_ = from_;
    recver_ = to_;
  }

  template<class Size>
  Size to_bytes_size() const {
    sst::checked_t<Size> n = 0;
    n += msg_size_size_;
    n += msg_.size();
    n += psn_size_size_;
    n += from_.size();
    n += psn_size_size_;
    n += to_.size();
    n += sizeof(time_type);
    n += sizeof(nonce_type);
    n += sizeof(amp_index_type);
    n += sizeof(trace_id_type);
    n += sizeof(span_id_type);
    return n.value();
  }

  template<class ByteIt>
  ByteIt to_bytes(ByteIt dst) const {
    dst = buffer_to_bytes(msg_, dst, msg_size_size_);
    dst = buffer_to_bytes(from_, dst, psn_size_size_);
    dst = buffer_to_bytes(to_, dst, psn_size_size_);
    dst = sst::old::to_bytes(time_,
                             dst,
                             sizeof(time_type),
                             std::is_signed<time_type>::value ?
                                 sst::integer_rep::twos_complement() :
                                 sst::integer_rep::pure_unsigned());
    dst = sst::old::to_bytes(nonce_,
                             dst,
                             sizeof(nonce_type),
                             std::is_signed<nonce_type>::value ?
                                 sst::integer_rep::twos_complement() :
                                 sst::integer_rep::pure_unsigned());
    dst = sst::old::to_bytes(amp_index_,
                             dst,
                             sizeof(amp_index_type),
                             std::is_signed<amp_index_type>::value ?
                                 sst::integer_rep::twos_complement() :
                                 sst::integer_rep::pure_unsigned());
    dst = sst::old::to_bytes(trace_id_,
                             dst,
                             sizeof(trace_id_type),
                             std::is_signed<trace_id_type>::value ?
                                 sst::integer_rep::twos_complement() :
                                 sst::integer_rep::pure_unsigned());
    dst = sst::old::to_bytes(span_id_,
                             dst,
                             sizeof(span_id_type),
                             std::is_signed<span_id_type>::value ?
                                 sst::integer_rep::twos_complement() :
                                 sst::integer_rep::pure_unsigned());
    return dst;
  }

  void from_bytes_init() {
    msg_size_size_ = 4;
    psn_size_size_ = 1;
  }

  template<class ByteIt, class Avail>
  ByteIt from_bytes(ByteIt src, Avail & avail) {
    SST_ASSERT(sst::perfect_ge(avail, 0));
    src = buffer_from_bytes(src, msg_size_size_, avail, msg_);
    src = buffer_from_bytes(src, psn_size_size_, avail, from_);
    src = buffer_from_bytes(src, psn_size_size_, avail, to_);
    time_ = sst::old::from_bytes<time_type>(
        &src,
        sizeof(time_type),
        avail,
        std::is_signed<time_type>::value ?
            sst::integer_rep::twos_complement() :
            sst::integer_rep::pure_unsigned());
    nonce_ = sst::old::from_bytes<nonce_type>(
        &src,
        sizeof(nonce_type),
        avail,
        std::is_signed<nonce_type>::value ?
            sst::integer_rep::twos_complement() :
            sst::integer_rep::pure_unsigned());
    amp_index_ = sst::old::from_bytes<amp_index_type>(
        &src,
        sizeof(amp_index_type),
        avail,
        std::is_signed<amp_index_type>::value ?
            sst::integer_rep::twos_complement() :
            sst::integer_rep::pure_unsigned());
    trace_id_ = sst::old::from_bytes<trace_id_type>(
        &src,
        sizeof(trace_id_type),
        avail,
        std::is_signed<trace_id_type>::value ?
            sst::integer_rep::twos_complement() :
            sst::integer_rep::pure_unsigned());
    span_id_ = sst::old::from_bytes<span_id_type>(
        &src,
        sizeof(span_id_type),
        avail,
        std::is_signed<span_id_type>::value ?
            sst::integer_rep::twos_complement() :
            sst::integer_rep::pure_unsigned());
    sender_ = from_;
    recver_ = to_;
    return src;
  }

  //--------------------------------------------------------------------

  ClrMsg to_ClrMsg() const {
    {
      msg_str_.clear();
      char const * const p =
          reinterpret_cast<char const *>(msg_.data());
      msg_str_.insert(msg_str_.end(), p, p + msg_.size());
    }
    {
      from_str_.clear();
      char const * const p =
          reinterpret_cast<char const *>(from_.data());
      from_str_.insert(from_str_.end(), p, p + from_.size());
    }
    {
      to_str_.clear();
      char const * const p = reinterpret_cast<char const *>(to_.data());
      to_str_.insert(to_str_.end(), p, p + to_.size());
    }
    return ClrMsg(msg_str_,
                  from_str_,
                  to_str_,
                  time_,
                  nonce_,
                  amp_index_,
                  trace_id_,
                  span_id_);
  }

  //--------------------------------------------------------------------
  // JSON
  //--------------------------------------------------------------------
  //
  // This is only for logging purposes, so convenience is more important
  // than performance.
  //

  nlohmann::json to_json() const;

  //--------------------------------------------------------------------
  // content_hash
  //--------------------------------------------------------------------

  sst::remove_cvref_t<decltype(std::declval<generic_hash_t>().finish())>
  content_hash() const;

  //--------------------------------------------------------------------
};

extern template std::size_t clrmsg_t::to_bytes_size() const;

} // namespace kestrel

#endif // #ifndef KESTREL_CLRMSG_T_HPP
