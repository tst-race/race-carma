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
#include <kestrel/ta1_plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/ta1_plugin_t.hpp>
//

namespace kestrel {

#define F(x) char const ta1_plugin_t::func_##x[] = #x

F(init);
F(notifyEpoch);
F(onBootstrapPkgReceived);
F(onChannelStatusChanged);
F(onConnectionStatusChanged);
F(onLinkPropertiesChanged);
F(onLinkStatusChanged);
F(onPackageStatusChanged);
F(onUserAcknowledgementReceived);
F(prepareToBootstrap);
F(processClrMsg);
F(shutdown);

#undef F

} // namespace kestrel
