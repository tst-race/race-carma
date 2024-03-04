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

#ifndef KESTREL_NPR_WPRF_H
#define KESTREL_NPR_WPRF_H

#include <cassert>
#include <sst/bn_ctx.h>

namespace kestrel {
namespace npr_wprf {

//
// p should be a "big enough" prime, but we can't really assert that
// since it's vague. Instead, we just assert that p > 1 and p is odd.
// The same goes for the safe prime s = 2p + 1.
//

//
// Sets dst to a random number mod p, excluding zero.
//

inline BIGNUM & KeyGen(BIGNUM & dst, BIGNUM const & p, sst::bn_ctx bn) {
  assert(bn.cmp(p, bn.one()) > 0);
  assert(bn.is_odd(p));
  do {
    bn.rand_range(dst, p);
  } while (bn.is_zero(dst));
  return dst;
}

//
// Samples a random x mod s, excluding zero, and sets dst to x^2 mod s.
//

inline BIGNUM &
RandInput(BIGNUM & dst, BIGNUM const & s, sst::bn_ctx bn) {
  assert(bn.cmp(s, bn.one()) > 0);
  assert(bn.is_odd(s));
  assert(&dst != &s);
  do {
    bn.rand_range(dst, s);
  } while (bn.is_zero(dst));
  bn.mod_sqr(dst, dst, s);
  return dst;
}

//
// Sets dst to x^k mod s.
//

inline BIGNUM & Eval(
    BIGNUM & dst,
    BIGNUM const & k,
    BIGNUM const & x,
    BIGNUM const & s,
    sst::bn_ctx bn) {
  assert(bn.cmp(k, bn.zero()) > 0);
  assert(bn.cmp(x, bn.zero()) > 0);
  assert(bn.cmp(s, bn.one()) > 0);
  assert(bn.is_odd(s));
  assert(bn.cmp(k, bn.div(bn.sub(s, bn.one()), bn.set_word(2))) < 0);
  assert(bn.cmp(x, s) < 0);
  bn.mod_exp(dst, x, k, s);
  return dst;
}

} // namespace npr_wprf
} // namespace kestrel

#endif // #ifndef KESTREL_NPR_WPRF_H
