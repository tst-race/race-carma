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

#ifndef KESTREL_PKC_HPP
#define KESTREL_PKC_HPP

#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_STATIC_ASSERT.h>
#include <sst/catalog/checked_t.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/conditional_t.hpp>
#include <sst/catalog/crypto_rng.hpp>
#include <sst/catalog/enable_if_t.hpp>
#include <sst/catalog/is_byte.hpp>
#include <sst/catalog/is_negative.hpp>
#include <sst/catalog/remove_cv_t.hpp>
#include <sst/catalog/remove_reference_t.hpp>
#include <sst/catalog/to_string.hpp>

#include <kestrel/pkc/invalid_ciphertext.hpp>
#include <kestrel/sodium.hpp>

//
// TODO
//
// This whole file will eventually be replaced by pkc_shell_t and
// friends. The same way vrf_core_t and vrf_shell_t are structured.
//
// Here's the plan to support multiple PKC schemes:
//
//    1. Implement a pure virtual class pkc_core_t that provides the
//       fundamental operations of a particular PKC scheme. It must
//       include a pure virtual function named clone() that a derived
//       class implements to make a deep copy of itself but return the
//       copy as sst::unique_ptr<pkc_core_t>. A derived class can have
//       any kind of extra parameters in its constructor. The derived
//       class need not be copiable nor movable.
//
//    2. Implement a class pkc_shell_t that wraps a
//       std::unique_ptr<pkc_core_t> and provides all the convenient
//       calls. In particular, it must be copiable and movable, using
//       pkc_core_t::clone to do a copy. This class should have a
//       from_json friend function that translates an arbitrary JSON
//       value into a std::unique_ptr<pkc_core_t>. It should also be
//       default constructible, which just stores a null pointer and
//       assertion fails on any calls. You must (indirectly) call
//       from_json to initialize it.
//
//    3. Now the PKC scheme can be stored in global.json and
//       global_config_t can store a pkc_shell_t object. To perform any
//       PKC operation foo, operations, all calling code should call
//       global.pkc().foo().
//

namespace kestrel {
namespace pkc {

//----------------------------------------------------------------------
// generate_keypair_sizes
//----------------------------------------------------------------------
//
// Determines how large the public and secret keys of a keypair will be.
//

template<class PublicKeySize, class SecretKeySize = PublicKeySize>
std::pair<PublicKeySize, SecretKeySize> generate_keypair_sizes() {
  return std::make_pair(
      sst::checked_cast<PublicKeySize>(crypto_box_PUBLICKEYBYTES),
      sst::checked_cast<SecretKeySize>(crypto_box_SECRETKEYBYTES));
}

//----------------------------------------------------------------------
// generate_keypair
//----------------------------------------------------------------------
//
// Generates a keypair. public_key and secret_key should have the sizes
// returned by generate_keypair_sizes.
//

inline void generate_keypair(unsigned char * const public_key,
                             unsigned char * const secret_key) {
  SST_ASSERT((public_key != nullptr));
  SST_ASSERT((secret_key != nullptr));
  int const s = sodium::crypto_box_keypair(public_key, secret_key);
  if (s != 0) {
    throw std::runtime_error(
        "crypto_box_keypair() failed (libsodium error code "
        + sst::to_string(s) + ").");
  }
}

template<class PublicKey = std::vector<unsigned char>,
         class SecretKey = PublicKey>
std::pair<PublicKey, SecretKey> generate_keypair() {
  auto const sizes =
      generate_keypair_sizes<typename PublicKey::size_type,
                             typename SecretKey::size_type>();
  std::pair<PublicKey, SecretKey> keypair;
  keypair.first.resize(sizes.first);
  keypair.second.resize(sizes.second);
  generate_keypair(keypair.first.data(), keypair.second.data());
  return keypair;
}

//----------------------------------------------------------------------
// anon_header_size
//----------------------------------------------------------------------
//
// The first byte of the header is used to indicate anon vs. auth. Zero
// means anon, and nonzero means auth.
//

template<class Size>
Size anon_header_size() {
  sst::checked_t<Size> header_size = 1;
  header_size += crypto_box_SEALBYTES;
  return header_size.value();
}

//----------------------------------------------------------------------
// auth_header_size
//----------------------------------------------------------------------
//
// The first byte of the header is used to indicate anon vs. auth. Zero
// means anon, and nonzero means auth.
//

template<class Size>
Size auth_header_size() {
  sst::checked_t<Size> header_size = 1;
  header_size += crypto_box_NONCEBYTES;
  header_size += crypto_box_MACBYTES;
  return header_size.value();
}

//----------------------------------------------------------------------
// anon_ciphertext_size
//----------------------------------------------------------------------

template<class CiphertextSize = void, class PlaintextSize>
sst::conditional_t<std::is_void<CiphertextSize>::value,
                   PlaintextSize,
                   CiphertextSize>
anon_ciphertext_size(PlaintextSize const plaintext_size) {
  using R = sst::conditional_t<std::is_void<CiphertextSize>::value,
                               PlaintextSize,
                               CiphertextSize>;
  SST_ASSERT((!sst::is_negative(plaintext_size)));
  sst::checked_t<R> ciphertext_size = 0;
  ciphertext_size += anon_header_size<R>();
  ciphertext_size += plaintext_size;
  return ciphertext_size.value();
}

//----------------------------------------------------------------------
// auth_ciphertext_size
//----------------------------------------------------------------------

template<class CiphertextSize = void, class PlaintextSize>
sst::conditional_t<std::is_void<CiphertextSize>::value,
                   PlaintextSize,
                   CiphertextSize>
auth_ciphertext_size(PlaintextSize const plaintext_size) {
  using R = sst::conditional_t<std::is_void<CiphertextSize>::value,
                               PlaintextSize,
                               CiphertextSize>;
  SST_ASSERT((!sst::is_negative(plaintext_size)));
  sst::checked_t<R> ciphertext_size = 0;
  ciphertext_size += auth_header_size<R>();
  ciphertext_size += plaintext_size;
  return ciphertext_size.value();
}

//----------------------------------------------------------------------
// anon_plaintext_size
//----------------------------------------------------------------------

template<class PlaintextSize = void, class CiphertextSize>
sst::conditional_t<std::is_void<PlaintextSize>::value,
                   CiphertextSize,
                   PlaintextSize>
anon_plaintext_size(CiphertextSize const ciphertext_size) {
  using R = sst::conditional_t<std::is_void<PlaintextSize>::value,
                               CiphertextSize,
                               PlaintextSize>;
  SST_ASSERT((!sst::is_negative(ciphertext_size)));
  CiphertextSize const header_size = anon_header_size<CiphertextSize>();
  if (ciphertext_size < header_size) {
    throw invalid_ciphertext();
  }
  return sst::checked_cast<R>(ciphertext_size - header_size);
}

//----------------------------------------------------------------------
// auth_plaintext_size
//----------------------------------------------------------------------

template<class PlaintextSize = void, class CiphertextSize>
sst::conditional_t<std::is_void<PlaintextSize>::value,
                   CiphertextSize,
                   PlaintextSize>
auth_plaintext_size(CiphertextSize const ciphertext_size) {
  using R = sst::conditional_t<std::is_void<PlaintextSize>::value,
                               CiphertextSize,
                               PlaintextSize>;
  SST_ASSERT((!sst::is_negative(ciphertext_size)));
  CiphertextSize const header_size = auth_header_size<CiphertextSize>();
  if (ciphertext_size < header_size) {
    throw invalid_ciphertext();
  }
  return sst::checked_cast<R>(ciphertext_size - header_size);
}

//----------------------------------------------------------------------
// anon_id_size
//----------------------------------------------------------------------

template<class Size>
Size anon_id_size() {
  return sst::checked_cast<Size>(crypto_box_SEALBYTES);
}

//----------------------------------------------------------------------
// auth_id_size
//----------------------------------------------------------------------

template<class Size>
Size auth_id_size() {
  return sst::checked_cast<Size>(crypto_box_NONCEBYTES);
}

//----------------------------------------------------------------------
// anon_encrypt
//----------------------------------------------------------------------
//
// Encrypts a plaintext without sender authentication.
//
// The return value points to a portion of the ciphertext that can be
// used as an ID. The size of this ID is returned by anon_id_size.
//

template<class PlaintextSize>
unsigned char *
anon_encrypt(unsigned char const * const recver_public_key,
             unsigned char const * const plaintext,
             PlaintextSize const plaintext_size,
             unsigned char * const ciphertext) {
  SST_ASSERT((recver_public_key != nullptr));
  SST_ASSERT((plaintext != nullptr));
  SST_ASSERT((!sst::is_negative(plaintext_size)));
  SST_ASSERT((ciphertext != nullptr));
  ciphertext[0] = 0;
  int const s = sodium::crypto_box_seal(
      ciphertext + 1,
      plaintext,
      sst::checked_cast<unsigned long long>(plaintext_size),
      recver_public_key);
  if (s != 0) {
    throw std::runtime_error(
        "crypto_box_seal() failed (libsodium error code "
        + sst::to_string(s) + ").");
  }
  return ciphertext + 1;
}

template<class RecverPublicKey, class Plaintext, class Ciphertext>
unsigned char * anon_encrypt(RecverPublicKey const & recver_public_key,
                             Plaintext const & plaintext,
                             Ciphertext & ciphertext) {
  SST_STATIC_ASSERT(
      (sst::is_byte<typename RecverPublicKey::value_type>::value));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename Plaintext::value_type>::value));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename Ciphertext::value_type>::value));
  SST_ASSERT(
      (recver_public_key.size()
       == generate_keypair_sizes<typename RecverPublicKey::size_type>()
              .first));
  ciphertext.resize(
      anon_ciphertext_size<typename Ciphertext::size_type>(
          plaintext.size()));
  return anon_encrypt(
      reinterpret_cast<unsigned char const *>(recver_public_key.data()),
      reinterpret_cast<unsigned char const *>(plaintext.data()),
      plaintext.size(),
      reinterpret_cast<unsigned char *>(ciphertext.data()));
}

template<class Ciphertext = std::vector<unsigned char>,
         class RecverPublicKey,
         class Plaintext>
std::pair<Ciphertext, unsigned char *>
anon_encrypt(RecverPublicKey const & recver_public_key,
             Plaintext const & plaintext) {
  SST_STATIC_ASSERT(
      (sst::is_byte<typename RecverPublicKey::value_type>::value));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename Plaintext::value_type>::value));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename Ciphertext::value_type>::value));
  SST_ASSERT(
      (recver_public_key.size()
       == generate_keypair_sizes<typename RecverPublicKey::size_type>()
              .first));
  Ciphertext ciphertext;
  unsigned char * const id =
      anon_encrypt(recver_public_key, plaintext, ciphertext);
  return std::pair<Ciphertext, unsigned char *>(std::move(ciphertext),
                                                id);
}

//----------------------------------------------------------------------
// auth_encrypt
//----------------------------------------------------------------------
//
// Encrypts a plaintext with sender authentication.
//
// The return value points to a portion of the ciphertext that can be
// used as an ID. The size of this ID is returned by auth_id_size.
//

template<class PlaintextSize>
unsigned char *
auth_encrypt(unsigned char const * const sender_secret_key,
             unsigned char const * const recver_public_key,
             unsigned char const * const plaintext,
             PlaintextSize const plaintext_size,
             unsigned char * const ciphertext) {
  SST_ASSERT((sender_secret_key != nullptr));
  SST_ASSERT((recver_public_key != nullptr));
  SST_ASSERT((plaintext != nullptr));
  SST_ASSERT((!sst::is_negative(plaintext_size)));
  SST_ASSERT((ciphertext != nullptr));
  ciphertext[0] = 1;
  sst::crypto_rng(ciphertext + 1, crypto_box_NONCEBYTES);
  int const s = sodium::crypto_box_easy(
      ciphertext + 1 + crypto_box_NONCEBYTES,
      plaintext,
      sst::checked_cast<unsigned long long>(plaintext_size),
      ciphertext + 1,
      recver_public_key,
      sender_secret_key);
  if (s != 0) {
    throw std::runtime_error(
        "crypto_box_easy() failed (libsodium error code "
        + sst::to_string(s) + ").");
  }
  return ciphertext + 1;
}

template<class SenderSecretKey,
         class RecverPublicKey,
         class Plaintext,
         class Ciphertext>
unsigned char * auth_encrypt(SenderSecretKey const & sender_secret_key,
                             RecverPublicKey const & recver_public_key,
                             Plaintext const & plaintext,
                             Ciphertext & ciphertext) {
  SST_STATIC_ASSERT(
      (sst::is_byte<typename SenderSecretKey::value_type>::value));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename RecverPublicKey::value_type>::value));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename Plaintext::value_type>::value));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename Ciphertext::value_type>::value));
  SST_ASSERT(
      (sender_secret_key.size()
       == generate_keypair_sizes<typename SenderSecretKey::size_type>()
              .second));
  SST_ASSERT(
      (recver_public_key.size()
       == generate_keypair_sizes<typename RecverPublicKey::size_type>()
              .first));
  ciphertext.resize(
      auth_ciphertext_size<typename Ciphertext::size_type>(
          plaintext.size()));
  return auth_encrypt(
      reinterpret_cast<unsigned char const *>(sender_secret_key.data()),
      reinterpret_cast<unsigned char const *>(recver_public_key.data()),
      reinterpret_cast<unsigned char const *>(plaintext.data()),
      plaintext.size(),
      reinterpret_cast<unsigned char *>(ciphertext.data()));
}

template<class Ciphertext = std::vector<unsigned char>,
         class SenderSecretKey,
         class RecverPublicKey,
         class Plaintext>
std::pair<Ciphertext, unsigned char *>
auth_encrypt(SenderSecretKey const & sender_secret_key,
             RecverPublicKey const & recver_public_key,
             Plaintext const & plaintext) {
  SST_STATIC_ASSERT(
      (sst::is_byte<typename SenderSecretKey::value_type>::value));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename RecverPublicKey::value_type>::value));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename Plaintext::value_type>::value));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename Ciphertext::value_type>::value));
  SST_ASSERT(
      (sender_secret_key.size()
       == generate_keypair_sizes<typename SenderSecretKey::size_type>()
              .second));
  SST_ASSERT(
      (recver_public_key.size()
       == generate_keypair_sizes<typename RecverPublicKey::size_type>()
              .first));
  Ciphertext ciphertext;
  unsigned char * const id = auth_encrypt(sender_secret_key,
                                          recver_public_key,
                                          plaintext,
                                          ciphertext);
  return std::pair<Ciphertext, unsigned char *>(std::move(ciphertext),
                                                id);
}

//----------------------------------------------------------------------
// ciphertext_is_anon
//----------------------------------------------------------------------
//
// Returns true if the ciphertext was encrypted without authentication.
//

template<class CiphertextSize>
bool ciphertext_is_anon(unsigned char const * const ciphertext,
                        CiphertextSize const ciphertext_size) {
  SST_ASSERT((ciphertext != nullptr));
  SST_ASSERT((!sst::is_negative(ciphertext_size)));
  if (ciphertext_size == 0) {
    throw invalid_ciphertext();
  }
  return ciphertext[0] == 0;
}

//----------------------------------------------------------------------
// ciphertext_is_auth
//----------------------------------------------------------------------
//
// Returns true if the ciphertext was encrypted with authentication.
//

template<class CiphertextSize>
bool ciphertext_is_auth(unsigned char const * const ciphertext,
                        CiphertextSize const ciphertext_size) {
  SST_ASSERT((ciphertext != nullptr));
  SST_ASSERT((!sst::is_negative(ciphertext_size)));
  if (ciphertext_size == 0) {
    throw invalid_ciphertext();
  }
  return ciphertext[0] != 0;
}

//----------------------------------------------------------------------
// anon_decrypt
//----------------------------------------------------------------------

template<class CiphertextByte,
         class CiphertextSize,
         sst::enable_if_t<std::is_same<sst::remove_cv_t<CiphertextByte>,
                                       unsigned char>::value> = 0>
CiphertextByte *
anon_decrypt(unsigned char const * const recver_public_key,
             unsigned char const * const recver_secret_key,
             CiphertextByte * const ciphertext,
             CiphertextSize const ciphertext_size,
             unsigned char * const plaintext) {
  SST_ASSERT((recver_public_key != nullptr));
  SST_ASSERT((recver_secret_key != nullptr));
  SST_ASSERT((ciphertext != nullptr));
  SST_ASSERT((!sst::is_negative(ciphertext_size)));
  SST_ASSERT((plaintext != nullptr));
  if (!ciphertext_is_anon(ciphertext, ciphertext_size)) {
    throw invalid_ciphertext();
  }
  static_cast<void>(anon_plaintext_size(ciphertext_size));
  int const s = sodium::crypto_box_seal_open(
      plaintext,
      ciphertext + 1,
      sst::checked_cast<unsigned long long>(
          ciphertext_size - static_cast<CiphertextSize>(1)),
      recver_public_key,
      recver_secret_key);
  if (s == -1) {
    throw invalid_ciphertext();
  }
  if (s != 0) {
    throw std::runtime_error(
        "crypto_box_seal_open() failed (libsodium error code "
        + sst::to_string(s) + ").");
  }
  return ciphertext + 1;
}

template<class RecverPublicKey,
         class RecverSecretKey,
         class Ciphertext,
         class Plaintext>
auto anon_decrypt(RecverPublicKey const & recver_public_key,
                  RecverSecretKey const & recver_secret_key,
                  Ciphertext && ciphertext,
                  Plaintext & plaintext)
    -> sst::conditional_t<std::is_const<sst::remove_reference_t<
                              decltype(*ciphertext.data())>>::value,
                          unsigned char const *,
                          unsigned char *> {
  using R =
      sst::conditional_t<std::is_const<sst::remove_reference_t<
                             decltype(*ciphertext.data())>>::value,
                         unsigned char const *,
                         unsigned char *>;
  SST_STATIC_ASSERT(
      (sst::is_byte<typename RecverPublicKey::value_type>::value));
  SST_ASSERT(
      (recver_public_key.size()
       == generate_keypair_sizes<typename RecverPublicKey::size_type>()
              .first));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename RecverSecretKey::value_type>::value));
  SST_ASSERT(
      (recver_secret_key.size()
       == generate_keypair_sizes<typename RecverSecretKey::size_type>()
              .second));
  SST_STATIC_ASSERT((sst::is_byte<typename sst::remove_reference_t<
                         Ciphertext>::value_type>::value));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename Plaintext::value_type>::value));
  plaintext.resize(anon_plaintext_size<typename Plaintext::size_type>(
      ciphertext.size()));
  return anon_decrypt(
      reinterpret_cast<unsigned char const *>(recver_public_key.data()),
      reinterpret_cast<unsigned char const *>(recver_secret_key.data()),
      reinterpret_cast<R>(ciphertext.data()),
      ciphertext.size(),
      reinterpret_cast<unsigned char *>(plaintext.data()));
}

template<class Plaintext = std::vector<unsigned char>,
         class RecverPublicKey,
         class RecverSecretKey,
         class Ciphertext>
auto anon_decrypt(RecverPublicKey const & recver_public_key,
                  RecverSecretKey const & recver_secret_key,
                  Ciphertext && ciphertext)
    -> std::pair<
        Plaintext,
        sst::conditional_t<std::is_const<sst::remove_reference_t<
                               decltype(*ciphertext.data())>>::value,
                           unsigned char const *,
                           unsigned char *>> {
  using R = std::pair<
      Plaintext,
      sst::conditional_t<std::is_const<sst::remove_reference_t<
                             decltype(*ciphertext.data())>>::value,
                         unsigned char const *,
                         unsigned char *>>;
  SST_STATIC_ASSERT(
      (sst::is_byte<typename RecverPublicKey::value_type>::value));
  SST_ASSERT(
      (recver_public_key.size()
       == generate_keypair_sizes<typename RecverPublicKey::size_type>()
              .first));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename RecverSecretKey::value_type>::value));
  SST_ASSERT(
      (recver_secret_key.size()
       == generate_keypair_sizes<typename RecverSecretKey::size_type>()
              .second));
  SST_STATIC_ASSERT((sst::is_byte<typename sst::remove_reference_t<
                         Ciphertext>::value_type>::value));
  Plaintext plaintext;
  auto * const id = anon_decrypt(recver_public_key,
                                 recver_secret_key,
                                 ciphertext,
                                 plaintext);
  return R(std::move(plaintext), id);
}

//----------------------------------------------------------------------
// auth_decrypt
//----------------------------------------------------------------------

template<class CiphertextByte,
         class CiphertextSize,
         sst::enable_if_t<std::is_same<sst::remove_cv_t<CiphertextByte>,
                                       unsigned char>::value> = 0>
CiphertextByte *
auth_decrypt(unsigned char const * const sender_public_key,
             unsigned char const * const recver_secret_key,
             CiphertextByte * const ciphertext,
             CiphertextSize const ciphertext_size,
             unsigned char * const plaintext) {
  SST_ASSERT((sender_public_key != nullptr));
  SST_ASSERT((recver_secret_key != nullptr));
  SST_ASSERT((ciphertext != nullptr));
  SST_ASSERT((!sst::is_negative(ciphertext_size)));
  SST_ASSERT((plaintext != nullptr));
  if (!ciphertext_is_auth(ciphertext, ciphertext_size)) {
    throw invalid_ciphertext();
  }
  static_cast<void>(auth_plaintext_size(ciphertext_size));
  int const s = sodium::crypto_box_open_easy(
      plaintext,
      ciphertext + 1 + crypto_box_NONCEBYTES,
      sst::checked_cast<unsigned long long>(
          ciphertext_size - static_cast<CiphertextSize>(1)
          - static_cast<CiphertextSize>(crypto_box_NONCEBYTES)),
      ciphertext + 1,
      sender_public_key,
      recver_secret_key);
  if (s == -1) {
    throw invalid_ciphertext();
  }
  if (s != 0) {
    throw std::runtime_error(
        "crypto_box_open_easy() failed (libsodium error code "
        + sst::to_string(s) + ").");
  }
  return ciphertext + 1;
}

template<class SenderPublicKey,
         class RecverSecretKey,
         class Ciphertext,
         class Plaintext>
auto auth_decrypt(SenderPublicKey const & sender_public_key,
                  RecverSecretKey const & recver_secret_key,
                  Ciphertext && ciphertext,
                  Plaintext & plaintext)
    -> sst::conditional_t<std::is_const<sst::remove_reference_t<
                              decltype(*ciphertext.data())>>::value,
                          unsigned char const *,
                          unsigned char *> {
  using R =
      sst::conditional_t<std::is_const<sst::remove_reference_t<
                             decltype(*ciphertext.data())>>::value,
                         unsigned char const *,
                         unsigned char *>;
  SST_STATIC_ASSERT(
      (sst::is_byte<typename SenderPublicKey::value_type>::value));
  SST_ASSERT(
      (sender_public_key.size()
       == generate_keypair_sizes<typename SenderPublicKey::size_type>()
              .first));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename RecverSecretKey::value_type>::value));
  SST_ASSERT(
      (recver_secret_key.size()
       == generate_keypair_sizes<typename RecverSecretKey::size_type>()
              .second));
  SST_STATIC_ASSERT((sst::is_byte<typename sst::remove_reference_t<
                         Ciphertext>::value_type>::value));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename Plaintext::value_type>::value));
  plaintext.resize(auth_plaintext_size<typename Plaintext::size_type>(
      ciphertext.size()));
  return auth_decrypt(
      reinterpret_cast<unsigned char const *>(sender_public_key.data()),
      reinterpret_cast<unsigned char const *>(recver_secret_key.data()),
      reinterpret_cast<R>(ciphertext.data()),
      ciphertext.size(),
      reinterpret_cast<unsigned char *>(plaintext.data()));
}

template<class Plaintext = std::vector<unsigned char>,
         class SenderPublicKey,
         class RecverSecretKey,
         class Ciphertext>
auto auth_decrypt(SenderPublicKey const & sender_public_key,
                  RecverSecretKey const & recver_secret_key,
                  Ciphertext && ciphertext)
    -> std::pair<
        Plaintext,
        sst::conditional_t<std::is_const<sst::remove_reference_t<
                               decltype(*ciphertext.data())>>::value,
                           unsigned char const *,
                           unsigned char *>> {
  using R = std::pair<
      Plaintext,
      sst::conditional_t<std::is_const<sst::remove_reference_t<
                             decltype(*ciphertext.data())>>::value,
                         unsigned char const *,
                         unsigned char *>>;
  SST_STATIC_ASSERT(
      (sst::is_byte<typename SenderPublicKey::value_type>::value));
  SST_ASSERT(
      (sender_public_key.size()
       == generate_keypair_sizes<typename SenderPublicKey::size_type>()
              .first));
  SST_STATIC_ASSERT(
      (sst::is_byte<typename RecverSecretKey::value_type>::value));
  SST_ASSERT(
      (recver_secret_key.size()
       == generate_keypair_sizes<typename RecverSecretKey::size_type>()
              .second));
  SST_STATIC_ASSERT((sst::is_byte<typename sst::remove_reference_t<
                         Ciphertext>::value_type>::value));
  Plaintext plaintext;
  auto * const id = auth_decrypt(sender_public_key,
                                 recver_secret_key,
                                 ciphertext,
                                 plaintext);
  return R(std::move(plaintext), id);
}

//----------------------------------------------------------------------

} // namespace pkc
} // namespace kestrel

#endif // #ifndef KESTREL_PKC_HPP
