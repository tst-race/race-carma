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

#ifndef KESTREL_CARMA_PHONEBOOK_T_HPP
#define KESTREL_CARMA_PHONEBOOK_T_HPP

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <sst/catalog/SST_NOEXCEPT.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_UNREACHABLE.hpp>
#include <sst/catalog/c_quote.hpp>
#include <sst/catalog/enable_if_t.hpp>
#include <sst/catalog/optional.hpp>

#include <kestrel/carma/global_config_t.hpp>
#include <kestrel/carma/phonebook_entries_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/common_sdk_t.hpp>
#include <kestrel/psn_any_hash_t.hpp>
#include <kestrel/psn_hash_1_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

class config_t;

class phonebook_t final {

public:

  using entries_t = phonebook_entries_t;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------
  //
  // Copy assignment needs a tracing_event_t parameter, so it's
  // implemented as a function named copy_assign instead of operator=.
  //
  // Destruction does not flush anything to disk.
  //

public:

  phonebook_t() = delete;

  phonebook_t(phonebook_t const &) = delete;

  phonebook_t & operator=(phonebook_t const &) = delete;

  phonebook_t & copy_assign(tracing_event_t tev,
                            phonebook_t const & other);

  phonebook_t(phonebook_t &&) = delete;

  phonebook_t & operator=(phonebook_t &&) = delete;

  ~phonebook_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // add_fast
  //--------------------------------------------------------------------
  //
  // Adds an entry to the phonebook.
  //
  // This function never causes any entries to be paged to disk. The new
  // entry will stay in memory and its addition will not cause any other
  // entries to be paged to disk. TODO: This is probably bad, it should
  // probably cause unloading.
  //

private:

  std::shared_ptr<phonebook_entry_t const> &
  add_fast(tracing_event_t tev, psn_t psn);

public:

  std::shared_ptr<phonebook_entry_t const>
  add_fast(tracing_event_t tev, phonebook_entry_t entry);

  //--------------------------------------------------------------------
  // add_slow
  //--------------------------------------------------------------------

public:

  std::shared_ptr<phonebook_entry_t const>
  add_slow(tracing_event_t tev, phonebook_entry_t entry);

  //--------------------------------------------------------------------
  // at
  //--------------------------------------------------------------------

public:

  std::shared_ptr<phonebook_entry_t const>
  at(tracing_event_t tev, phonebook_pair_t const & entry) const;

  template<class Key,
           sst::enable_if_t<!std::is_convertible<
               Key const &,
               phonebook_pair_t const &>::value> = 0>
  std::shared_ptr<phonebook_entry_t const> at(tracing_event_t tev,
                                              Key const & key) const {
    return at(SST_TEV_ARG(tev), expect(key));
  }

  //--------------------------------------------------------------------
  // begin
  //--------------------------------------------------------------------

public:

  entries_t::iterator begin() noexcept;

  entries_t::const_iterator begin() const noexcept;

  //--------------------------------------------------------------------
  // clear
  //--------------------------------------------------------------------
  //
  // Removes everything without flushing anything to disk.
  //

public:

  void clear();

  //--------------------------------------------------------------------
  // clear_deducible
  //--------------------------------------------------------------------

public:

  void clear_deducible(tracing_event_t tev) const;

  //--------------------------------------------------------------------
  // config
  //--------------------------------------------------------------------

private:

  config_t * config_ = nullptr;

public:

  config_t & config() noexcept {
    SST_ASSERT((config_ != nullptr));
    return *config_;
  }

  config_t const & config() const noexcept {
    SST_ASSERT((config_ != nullptr));
    return *config_;
  }

  //--------------------------------------------------------------------
  // construct
  //--------------------------------------------------------------------
  //
  // If sdk is null, the system's filesystem will be used instead of the
  // SDK's filesystem.
  //

public:

  explicit phonebook_t(tracing_event_t tev,
                       config_t & config,
                       std::string dir,
                       common_sdk_t * sdk = nullptr);

  //--------------------------------------------------------------------
  // dir_
  //--------------------------------------------------------------------

private:

  std::string dir_;

  //--------------------------------------------------------------------
  // end
  //--------------------------------------------------------------------

public:

  entries_t::iterator end() noexcept;

  entries_t::const_iterator end() const noexcept;

  //--------------------------------------------------------------------
  // entries_
  //--------------------------------------------------------------------

private:

  mutable entries_t entries_;

  //--------------------------------------------------------------------
  // entries_dir_
  //--------------------------------------------------------------------

private:

  std::string entries_dir_;

  //--------------------------------------------------------------------
  // expect
  //--------------------------------------------------------------------

public:

  template<class NotFoundException = std::runtime_error>
  phonebook_pair_t const & expect(psn_t const & psn) const {
    phonebook_pair_t const * const p = find(psn);
    if (!p) {
      throw NotFoundException("PSN not in phonebook: "
                              + sst::c_quote(psn.value()));
    }
    return *p;
  }

  template<
      class NotFoundException = std::runtime_error,
      class PsnHash,
      sst::enable_if_t<!std::is_convertible<PsnHash, psn_t>::value> = 0>
  phonebook_pair_t const & expect(PsnHash const & hash) const {
    phonebook_pair_t const * const p = find(hash);
    if (!p) {
      // TODO: Include the hash in the exception message. Hash types
      //       should probably just have .data() and .size(), then we
      //       can use sst::to_hex.
      throw NotFoundException("PSN hash not in phonebook");
    }
    return *p;
  }

  //--------------------------------------------------------------------
  // file_for
  //--------------------------------------------------------------------

private:

  std::string file_for(psn_t const & psn) const;

  //--------------------------------------------------------------------
  // find
  //--------------------------------------------------------------------

public:

  phonebook_pair_t const * find(psn_t const & psn) const {
    auto const it = entries_.find(psn);
    return it != entries_.end() ? &*it : nullptr;
  }

  phonebook_pair_t const * find(psn_hash_1_t const & hash) const {
    auto const it = psn_hashes_1_.find(hash);
    return it != psn_hashes_1_.end() ? it->second : nullptr;
  }

  phonebook_pair_t const * find(psn_any_hash_t const & hash) const {
    switch (hash.type) {
      case 1:
        return find(hash.hash_1);
    }
    SST_UNREACHABLE();
  }

  //--------------------------------------------------------------------
  // flush
  //--------------------------------------------------------------------
  //
  // Flushes one or all entries to disk.
  //

private:

  void flush(tracing_event_t tev,
             json_t const & json,
             std::string const & file);

public:

  void flush(tracing_event_t tev, bool const pack = false);

  //--------------------------------------------------------------------
  // packed_dir_
  //--------------------------------------------------------------------

private:

  std::string packed_dir_;

  //--------------------------------------------------------------------
  // packed_file_
  //--------------------------------------------------------------------

private:

  std::string packed_file_;

  //--------------------------------------------------------------------
  // PSN hash maps
  //--------------------------------------------------------------------
  //
  // These maps point into the entries_ map. There should be one of
  // these maps for each psn_hash_*_t type.
  //

private:

  std::map<psn_hash_1_t, phonebook_pair_t *> psn_hashes_1_;

  //--------------------------------------------------------------------
  // sdk_
  //--------------------------------------------------------------------

private:

  common_sdk_t * sdk_;

  //--------------------------------------------------------------------
};

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_PHONEBOOK_T_HPP
