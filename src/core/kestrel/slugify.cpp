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
#include <kestrel/slugify.hpp>
// Include twice to test idempotence.
#include <kestrel/slugify.hpp>
//

#include <string>

namespace kestrel {

std::string slugify(std::string const & src) {
  if (src == "shared") {
    return "share%64";
  }
  std::string dst;
  using Size = std::string::size_type;
  for (Size i = Size(0); i < src.size(); ++i) {
    char const c = src[i];
    if ((c >= '0' && c <= '9') || c == '_' || (c >= 'a' && c <= 'z')
        || (i > Size(0) && i < src.size() - Size(1)
            && (c == '-' || c == '.'))) {
      dst += c;
    } else if (c == '%') {
      dst += '%';
      dst += '%';
    } else {
      unsigned int const x = static_cast<unsigned int>(c);
      unsigned int const a = (x >> 4) & 0xFU;
      unsigned int const b = (x >> 0) & 0xFU;
      dst += '%';
      dst += "0123456789ABCDEF"[a];
      dst += "0123456789ABCDEF"[b];
    }
  }
  return dst;
}

std::string unslugify(std::string const & src) {
  std::string dst;
  int i = 0;
  char w = '\0';
  unsigned int v = 0U;
  for (char const c : src) {
    if (i == 0) {
      if (c == '%') {
        ++i;
      } else {
        dst += c;
      }
    } else if (i == 1) {
      if (c == '%') {
        dst += '%';
        i = 0;
      } else if (c >= '0' && c <= '9') {
        w = c;
        v = static_cast<unsigned int>(c - '0') << 4;
        ++i;
      } else if (c >= 'A' && c <= 'F') {
        w = c;
        v = (10U + static_cast<unsigned int>(c - 'A')) << 4;
        ++i;
      } else if (c >= 'a' && c <= 'f') {
        w = c;
        v = (10U + static_cast<unsigned int>(c - 'a')) << 4;
        ++i;
      } else {
        dst += '%';
        dst += c;
        i = 0;
      }
    } else if (i == 2) {
      if (c >= '0' && c <= '9') {
        v |= static_cast<unsigned int>(c - '0');
        dst += static_cast<char>(v);
      } else if (c >= 'A' && c <= 'F') {
        v |= 10U + static_cast<unsigned int>(c - 'A');
        dst += static_cast<char>(v);
      } else if (c >= 'a' && c <= 'f') {
        v |= 10U + static_cast<unsigned int>(c - 'a');
        dst += static_cast<char>(v);
      } else {
        dst += '%';
        dst += w;
        dst += c;
      }
      i = 0;
    }
  }
  if (i == 1) {
    dst += '%';
  } else if (i == 2) {
    dst += '%';
    dst += w;
  }
  return dst;
}

} // namespace kestrel
