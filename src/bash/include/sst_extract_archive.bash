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

sst_extract_archive() {

  sst_expect_argument_count $# 1

  local -r x="$1"
  local y

  case $x in
    *.tar)
      tar xf "$x"
    ;;
    *.tar.gz)
      tar xzf "$x"
    ;;
    *.tar.xz)
      tar xJf "$x"
    ;;
    *.zip)
      unzip "$x"
    ;;
    *)
      y=$(sst_quote "$x" | sst_csf)
      sst_csf y
      sst_barf 'unknown archive file extension: %s' "$y"
    ;;
  esac

}; readonly -f sst_extract_archive
