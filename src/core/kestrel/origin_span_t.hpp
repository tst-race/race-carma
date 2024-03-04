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

#ifndef KESTREL_ORIGIN_SPAN_T_HPP
#define KESTREL_ORIGIN_SPAN_T_HPP

#include <kestrel/sdk_span_t.hpp>
#include <utility>

namespace kestrel {

//
// The origin_span_t class holds an OpenTracing span inherited from a
// ClrMsg or EncPkg. This is used by packet classes to remember where
// they came from for further OpenTracing purposes.
//

class origin_span_t {
  sdk_span_t span_;

protected:
  void set_origin_span(sdk_span_t const & span);

public:
  template<class... Args>
  origin_span_t(Args &&... args) : span_(std::forward<Args>(args)...) {
  }

  sdk_span_t const & origin_span();
};

} // namespace kestrel

#endif // #ifndef KESTREL_ORIGIN_SPAN_T_HPP
