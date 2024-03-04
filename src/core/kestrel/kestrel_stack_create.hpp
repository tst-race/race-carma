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

#ifndef KESTREL_KESTREL_STACK_CREATE_HPP
#define KESTREL_KESTREL_STACK_CREATE_HPP

#include <list>
#include <ostream>
#include <string>

#include <kestrel/kestrel_cli_args_t.hpp>

namespace kestrel {

void kestrel_stack_create(kestrel_cli_args_t const & kestrel_cli_args,
                          std::list<std::string> argv,
                          std::ostream & cout);

} // namespace kestrel

#endif // #ifndef KESTREL_KESTREL_STACK_CREATE_HPP
