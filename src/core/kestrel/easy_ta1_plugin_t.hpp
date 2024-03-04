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

#ifndef KESTREL_EASY_TA1_PLUGIN_T_HPP
#define KESTREL_EASY_TA1_PLUGIN_T_HPP

#include <string>

#include <sst/catalog/SST_NOEXCEPT.hpp>

#include <IRaceSdkNM.h>

#include <kestrel/common_plugin_t.hpp>
#include <kestrel/easy_common_plugin_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/link_type_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/ta1_plugin_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

//
// TODO: This should eventually use ta1_sdk_t. Right now it's using
//       sdk_wrapper_t, which is essentially the old version of
//       ta1_sdk_t.
//

template<class Config>
class easy_ta1_plugin_t
    : public easy_common_plugin_t<sdk_wrapper_t, Config>,
      public ta1_plugin_t {

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

protected:

  easy_ta1_plugin_t() = delete;

  easy_ta1_plugin_t(easy_ta1_plugin_t const &) = delete;

  easy_ta1_plugin_t & operator=(easy_ta1_plugin_t const &) = delete;

  easy_ta1_plugin_t(easy_ta1_plugin_t &&) = delete;

  easy_ta1_plugin_t & operator=(easy_ta1_plugin_t &&) = delete;

  virtual ~easy_ta1_plugin_t() SST_NOEXCEPT(true) = default;

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

protected:

  explicit easy_ta1_plugin_t(IRaceSdkNM & sdk,
                             std::string const & plugin_name)
      : easy_ta1_plugin_t::common_plugin_t(sdk, plugin_name),
        easy_ta1_plugin_t::easy_common_plugin_t(sdk, plugin_name),
        easy_ta1_plugin_t::ta1_plugin_t(sdk, plugin_name) {
  }

  //--------------------------------------------------------------------

public:

  race_handle_t openConnection(tracing_event_t tev,
                               link_id_t const & link_id,
                               link_type_t link_type);

  //--------------------------------------------------------------------

}; //

} // namespace kestrel

#include <kestrel/easy_ta1_plugin_t/openConnection/id-type.hpp>

#endif // #ifndef KESTREL_EASY_TA1_PLUGIN_T_HPP
