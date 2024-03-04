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
#include <kestrel/kestrel_cli_args_t.hpp>
// Include twice to test idempotence.
#include <kestrel/kestrel_cli_args_t.hpp>
//

#include <string>
#include <utility>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/json/unknown_key.hpp>

#include <kestrel/json_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// work_dir
//----------------------------------------------------------------------

void kestrel_cli_args_t::parse_work_dir(json_t & src,
                                        parse_state_t & state) {
  if (state.called_parse_work_dir_) {
    return;
  }
  state.called_parse_work_dir_ = true;
  sst::json::remove_to(src,
                       *this->work_dir_,
                       "work_dir",
                       default_work_dir());
}

void kestrel_cli_args_t::unparse_work_dir(json_t & dst) const {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((!dst.contains("work_dir")));
  if (this->work_dir() != default_work_dir()) {
    dst["work_dir"] = this->work_dir();
  }
}

std::string const & kestrel_cli_args_t::work_dir() const {
  return *this->work_dir_;
}

kestrel_cli_args_t &
kestrel_cli_args_t::work_dir(std::string const & src) {
  *this->work_dir_ = src;
  return *this;
}

//----------------------------------------------------------------------
// JSON conversions
//----------------------------------------------------------------------

void from_json(json_t src, kestrel_cli_args_t & dst) {

  kestrel_cli_args_t tmp;
  kestrel_cli_args_t::parse_state_t state;

  tmp.parse_work_dir(src, state);

  SST_ASSERT((state.called_parse_work_dir_));

  sst::json::unknown_key(src);

  dst = std::move(tmp);

} //

void to_json(json_t & dst, kestrel_cli_args_t const & src) {

  json_t tmp = json_t::object();

  src.unparse_work_dir(tmp);

  dst = std::move(tmp);

} //

//----------------------------------------------------------------------

} // namespace kestrel
