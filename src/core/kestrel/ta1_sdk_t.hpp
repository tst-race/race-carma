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

#ifndef KESTREL_TA1_SDK_T_HPP
#define KESTREL_TA1_SDK_T_HPP

#include <cstdint>
#include <string>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>

#include <EncPkg.h>
#include <IRaceSdkNM.h>
#include <LinkProperties.h>
#include <RaceEnums.h>
#include <SdkResponse.h>

#include <kestrel/CARMA_DEFINE_FINAL_WRAPPER.hpp>
#include <kestrel/common_sdk_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {

//
// The ta1_sdk_t class is a wrapper class around the IRaceSdkNM class
// from the RACE SDK that provides enhanced logging.
//

class ta1_sdk_t : public common_sdk_t {

  //--------------------------------------------------------------------
  // Underlying SDK
  //--------------------------------------------------------------------

public:

  using race_sdk_t = IRaceSdkNM;

private:

  IRaceSdkNM * sdk_ = nullptr;

  IRaceSdkNM & sdk() const noexcept {
    SST_ASSERT(!moved_from_);
    SST_ASSERT(sdk_ != nullptr);
    return *sdk_;
  }

  //--------------------------------------------------------------------

public:

  explicit ta1_sdk_t(IRaceSdkNM & sdk, std::string const & plugin_name)
      : common_sdk_t(sdk, plugin_name),
        sdk_(&sdk) {
  }

  ta1_sdk_t(ta1_sdk_t const &) = delete;
  ta1_sdk_t & operator=(ta1_sdk_t const &) = delete;
  ta1_sdk_t(ta1_sdk_t &&) = delete;
  ta1_sdk_t & operator=(ta1_sdk_t &&) = delete;
  ~ta1_sdk_t() noexcept = default;

  //--------------------------------------------------------------------
  // Exception wrapper functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define an exception wrapper function
  // e_foo that translates all errors reported by foo into exceptions.
  // Some errors may already be exceptions, in which case we either
  // rethrow them or translate them to different exceptions.
  //

private:

  SdkResponse
  e_bootstrapDevice(RaceHandle handle,
                    std::vector<std::string> const & ta2Channels);

  SdkResponse
  e_displayInfoToUser(std::string const & data,
                      RaceEnums::UserDisplayType displayType);

  SdkResponse e_sendBootstrapPkg(ConnectionID const & connectionId,
                                 std::string const & persona,
                                 RawData const & pkg,
                                 std::int32_t timeout);

  //--------------------------------------------------------------------
  // Input description functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define an input description function
  // i_foo that returns a JSON object that describes the input to foo.
  //

private:

  static json_t
  i_bootstrapDevice(RaceHandle handle,
                    std::vector<std::string> const & ta2Channels);

  static json_t
  i_displayInfoToUser(std::string const & data,
                      RaceEnums::UserDisplayType displayType);

  static json_t i_sendBootstrapPkg(ConnectionID const & connectionId,
                                   std::string const & persona,
                                   RawData const & pkg,
                                   std::int32_t timeout);

  //--------------------------------------------------------------------
  // Output description functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define an output description function
  // o_foo that returns a JSON object that describes the output of foo.
  //

private:

  static json_t
  o_bootstrapDevice(SdkResponse const & return_value,
                    RaceHandle handle,
                    std::vector<std::string> const & ta2Channels);

  static json_t
  o_displayInfoToUser(SdkResponse const & return_value,
                      std::string const & data,
                      RaceEnums::UserDisplayType displayType);

  static json_t o_sendBootstrapPkg(SdkResponse const & return_value,
                                   ConnectionID const & connectionId,
                                   std::string const & persona,
                                   RawData const & pkg,
                                   std::int32_t timeout);

  //--------------------------------------------------------------------
  // Final wrapper functions
  //--------------------------------------------------------------------
  //
  // For each SDK function foo, we define a final wrapper function foo
  // that additionally takes a tracing_event_t parameter, forwards the
  // call to e_foo, and uses i_foo and o_foo to log information about
  // the call and the result.
  //

public:

  CARMA_DEFINE_FINAL_WRAPPER(bootstrapDevice)
  CARMA_DEFINE_FINAL_WRAPPER(displayInfoToUser)
  CARMA_DEFINE_FINAL_WRAPPER(sendBootstrapPkg)

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_TA1_SDK_T_HPP
