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

#ifndef KESTREL_CATALOG_KESTREL_WITH_KESTREL_RABBITMQ_MANAGEMENT_H
#define KESTREL_CATALOG_KESTREL_WITH_KESTREL_RABBITMQ_MANAGEMENT_H

// clang-format off

// TODO: This should eventually become part of ./configure as
//       --with-kestrel-rabbitmq_management with the usual build system
//       config (BSC) and preprocessor config (PPC) split.

#define KESTREL_WITH_KESTREL_RABBITMQ_MANAGEMENT (defined(__EMSCRIPTEN__))

// clang-format off

#endif // #ifndef KESTREL_CATALOG_KESTREL_WITH_KESTREL_RABBITMQ_MANAGEMENT_H
