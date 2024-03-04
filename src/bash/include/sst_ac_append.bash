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

# TODO: After autogen_ac_append is eventually removed, the
# quoted-by-default convention will be gone and we can remove the
# printing of "[" and "]" in this function, as well as removing the
# opening "[" and the trailing "]" in the start and finish functions.

sst_ac_append() {

  if ((SST_DEBUG)); then
    sst_expect_errexit
  fi

  if [[ ! "${sst_ac_start_has_been_called+x}" ]]; then
    sst_barf 'sst_ac_start has not been called'
  fi

  if [[ "${sst_ac_finish_has_been_called+x}" ]]; then
    sst_barf 'sst_ac_finish has been called'
  fi

  if ((SST_DEBUG)); then
    if (($# != 0)); then
      sst_expect_argument_count $# 0
    fi
  fi

  printf ']\n' >>$autogen_ac_file
  cat >>$autogen_ac_file
  printf '[\n' >>$autogen_ac_file

}; readonly -f sst_ac_append
