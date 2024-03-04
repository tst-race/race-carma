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

#ifndef KESTREL_CARMA_RANGEGEN_ROW_HPP
#define KESTREL_CARMA_RANGEGEN_ROW_HPP

#include <sst/catalog/SST_STATIC_ASSERT.h>
#include <sst/catalog/str_cmp.hpp>

#include <kestrel/carma/rangegen/nonrigid_correctness_only.hpp>
#include <kestrel/carma/rangegen/nonrigid_default.hpp>
#include <kestrel/carma/rangegen/rigid_correctness_only.hpp>
#include <kestrel/carma/rangegen/rigid_default.hpp>

SST_STATIC_ASSERT((sst::str_cmp(KESTREL_CARMA_RANGEGEN_NONRIGID_CORRECTNESS_ONLY_HPP_ROW, KESTREL_CARMA_RANGEGEN_NONRIGID_DEFAULT_HPP_ROW) == 0));
SST_STATIC_ASSERT((sst::str_cmp(KESTREL_CARMA_RANGEGEN_NONRIGID_CORRECTNESS_ONLY_HPP_ROW, KESTREL_CARMA_RANGEGEN_RIGID_CORRECTNESS_ONLY_HPP_ROW) == 0));
SST_STATIC_ASSERT((sst::str_cmp(KESTREL_CARMA_RANGEGEN_NONRIGID_CORRECTNESS_ONLY_HPP_ROW, KESTREL_CARMA_RANGEGEN_RIGID_DEFAULT_HPP_ROW) == 0));

#endif // #ifndef KESTREL_CARMA_RANGEGEN_ROW_HPP
