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

#include "TEST_EXIT.h"
#include <kestrel/npr_wprf.h>
#include <exception>
#include <iostream>
#include <sst/bn_ctx.h>

using namespace kestrel;

int main() {
  int exit_status = TEST_EXIT_PASS;

  try {
    sst::bn_ctx bn;
    BIGNUM & p = bn.set_word(53);
    BIGNUM & s = bn.set_word(107);
    BIGNUM & x = bn.get();
    BIGNUM & k = bn.get();
    BIGNUM & t1 = bn.get();
    BIGNUM & t2 = bn.get();

    for (int i = 0; i != 1000; ++i) {
      npr_wprf::KeyGen(x, p, bn.arg());
      if (bn.is_negative(x)) {
        std::cerr << "KeyGen returned " << bn.bn2dec(x)
                  << ", which is negative" << std::endl;
        exit_status = TEST_EXIT_FAIL;
        break;
      }
      if (bn.cmp(x, p) >= 0) {
        std::cerr << "KeyGen returned " << bn.bn2dec(x)
                  << ", which is not smaller than p = " << bn.bn2dec(p)
                  << std::endl;
        exit_status = TEST_EXIT_FAIL;
        break;
      }
    }

    for (int i = 0; i != 1000; ++i) {
      npr_wprf::RandInput(x, s, bn.arg());
      if (bn.is_negative(x)) {
        std::cerr << "RandInput returned " << bn.bn2dec(x)
                  << ", which is negative" << std::endl;
        exit_status = TEST_EXIT_FAIL;
        break;
      }
      if (bn.cmp(x, s) >= 0) {
        std::cerr << "RandInput returned " << bn.bn2dec(x)
                  << ", which is not smaller than s = " << bn.bn2dec(s)
                  << std::endl;
        exit_status = TEST_EXIT_FAIL;
        break;
      }
      // x is a square mod s iff x^p = 1 mod s
      if (!bn.is_one(bn.mod_exp(t1, x, p, s))) {
        std::cerr << "RandInput returned " << bn.bn2dec(x)
                  << ", which is not a square mod s = " << bn.bn2dec(s)
                  << std::endl;
        exit_status = TEST_EXIT_FAIL;
        break;
      }
    }

    for (int i = 0; i != 1000; ++i) {
      npr_wprf::RandInput(x, s, bn.arg());
      npr_wprf::KeyGen(k, p, bn.arg());
      npr_wprf::Eval(t1, k, x, s, bn.arg());
      bn.mod_exp(t2, x, k, s);
      if (bn.cmp(t1, t2) != 0) {
        std::cerr << "Eval returned " << bn.bn2dec(t1) << " for "
                  << bn.bn2dec(x) << "^" << bn.bn2dec(k) << " mod "
                  << bn.bn2dec(s) << ", but the expected result was "
                  << bn.bn2dec(t2) << std::endl;
        exit_status = TEST_EXIT_FAIL;
        break;
      }
    }
  } catch (std::exception const & e) {
    try {
      std::cerr << "Error: " << e.what() << std::endl;
    } catch (...) {
    }
    exit_status = TEST_EXIT_ERROR;
  } catch (...) {
    try {
      std::cerr << "Unknown error" << std::endl;
    } catch (...) {
    }
    exit_status = TEST_EXIT_ERROR;
  }

  return exit_status;
}
