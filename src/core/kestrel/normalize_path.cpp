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

// Include first to test independence.
#include <kestrel/normalize_path.hpp>
// Include twice to test idempotence.
#include <kestrel/normalize_path.hpp>
//

#include <iostream>
#include <stdexcept>
#include <string>

#include <sst/catalog/c_quote.hpp>

namespace kestrel {

std::string normalize_path(std::string x) {

  // Step 1
  if (x.empty()) {
    throw std::runtime_error("Path is empty.");
  }
  for (char const c : x) {
    if (c != '-' && c != '%' && c != '.' && c != '/'
        && (c < '0' || c > '9') && (c < 'A' || c > 'Z') && c != '_'
        && (c < 'a' || c > 'z')) {
      throw std::runtime_error("Path " + sst::c_quote(x)
                               + " contains a " + sst::c_quote(c)
                               + " character, which is not accepted.");
    }
  }

  // Step 2
  if (x.front() == '/') {
    x.insert(x.begin(), '.');
  }

  // Step 3
  if (x.back() == '/') {
    x.insert(x.end(), '.');
  }

  // Step 4
  {
    std::string::size_type i;
    std::string::size_type j = 0;
    while (true) {
      i = j;
      while (i < x.size() && x[i] != '/') {
        ++i;
      }
      j = i;
      while (j < x.size() && x[j] == '/') {
        ++j;
      }
      if (i == j) {
        break;
      }
      x.erase(i, j - i - 1);
      j = i + 1;
    }
  }

  // Step 5
  {
    std::string::size_type i;
    std::string::size_type j = 0;
    while (true) {
      i = j;
      while (i < x.size() && x[i] == '/') {
        ++i;
      }
      j = i;
      while (j < x.size() && x[j] != '/') {
        ++j;
      }
      if (i == j) {
        break;
      }
      if (j - i == 1 && x[i] == '.') {
        if (i > 0) {
          x.erase(i - 1, 2);
          j = i - 1;
        } else if (j < x.size()) {
          x.erase(i, 2);
          j = i;
        }
      }
    }
  }

  // Step 6
  {
    std::string::size_type i;
    std::string::size_type j = 0;
    while (true) {
      i = j;
      while (i < x.size() && x[i] == '/') {
        ++i;
      }
      j = i;
      while (j < x.size() && x[j] != '/') {
        ++j;
      }
      if (i == j) {
        break;
      }
      if (j - i == 2 && x[i] == '.' && x[i + 1] == '.') {
        if (i > 0) {
          --i;
          while (i > 0 && x[i - 1] != '/') {
            --i;
          }
          if (i > 0) {
            x.erase(i - 1, j - i + 1);
            j = i - 1;
          } else if (j < x.size()) {
            x.erase(i, j - i + 1);
            j = i;
          } else {
            x = ".";
          }
        } else if (j < x.size()) {
          x.erase(i, j - i + 1);
          j = i;
        } else {
          x = ".";
        }
      }
    }
  }

  return x;

} //

} // namespace kestrel
