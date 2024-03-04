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

sst_ac_config_file() {

  # Bash >=4.2: declare -g -A sst_ac_config_file_srcs
  # Bash >=4.2: declare -g -A sst_am_targets

  declare    dst
  declare    src
  declare    x

  for src; do

    sst_expect_source_path "$src"

    if [[ "${sst_ac_config_file_srcs[$src]+x}" ]]; then
      sst_warn "'$src' was redundantly processed."
      continue
    fi

    if [[ $src == *@(.im.in|.in|.im) ]]; then
      if [[ ! -f $src ]]; then
        sst_barf "Unable to process '$src' because it does not exist."
      fi
      dst=${src%%?(.im)?(.in)}
    elif [[ -f $src.im.in ]]; then
      dst=$src
      src=$src.im.in
    elif [[ -f $src.in ]]; then
      dst=$src
      src=$src.in
    elif [[ -f $src.im ]]; then
      dst=$src
      src=$src.im
    else
      sst_barf \
        "Unable to process '$src' because it does not have a" \
        ".im.in, .in, or .im extension, nor does adding such" \
        "an extension yield an existent file." \
      ;
    fi

    for x in $dst{,.im.in,.in,.im}; do
      if [[ $x != $src && -f $x ]]; then
        sst_barf "Unable to process '$src' because '$x' exists."
      fi
    done

    sst_ac_config_file_srcs[$src]=$dst

    if [[ "${sst_am_targets[$dst]+x}" ]]; then
      sst_barf \
        "Unable to produce '$dst' from '$src' because it has" \
        "already been produced from '${sst_am_targets[$dst]}'." \
      ;
    fi

    sst_am_targets[$dst]=$src

    if [[ $src == *.im.in ]]; then
      sst_ac_append <<<"GATBPS_CONFIG_FILE([$dst.im])"
      sst_ac_append <<<"GATBPS_CONFIG_LATER([$dst])"
    elif [[ $src == *.in ]]; then
      sst_ac_append <<<"GATBPS_CONFIG_FILE([$dst])"
    elif [[ $src == *.im ]]; then
      sst_ac_append <<<"GATBPS_CONFIG_LATER([$dst])"
    fi

  done

}; readonly -f sst_ac_config_file
