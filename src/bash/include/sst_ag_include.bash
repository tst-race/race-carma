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

sst_ag_include() {

  local file
  # Bash >=4.2: declare -g sst_ag_include_seen

  if ((SST_DEBUG)); then
    sst_expect_errexit
  fi

  for file; do
    sst_expect_source_path "$file"
    if [[ "${sst_ag_include_seen= }" != *" $file "* ]]; then
      sst_info "Including: $file"
      sst_ag_include_seen+="$file "
      sst_am_distribute $file
      . $file
    fi
  done

}; readonly -f sst_ag_include
