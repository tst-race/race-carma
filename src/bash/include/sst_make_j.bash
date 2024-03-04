#
# Copyright (C) 2019-2024 Stealth Software Technologies, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an "AS
# IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language
# governing permissions and limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
#

sst_make_j() {

  # Bash >=4.2: declare -g sst_have_make_j

  sst_get_have_make_j
  if ((sst_have_make_j)); then
    sst_get_max_procs
    sst_make -j $sst_max_procs "$@"
  else
    sst_make "$@"
  fi

}; readonly -f sst_make_j
