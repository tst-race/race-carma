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

config_h_get_string() {

  local config
  local macro
  local main

  case $# in
    2)
    ;;
    *)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  config=$1
  readonly config
  expect_safe_path "$config"

  macro=$2
  readonly macro
  sst_expect_basic_identifier "$macro"

  main=config_h_get_string-$macro-$$.c
  readonly main

  cat >$main <<EOF
#include "$config"
#include <stdio.h>
#include <stdlib.h>
int (main)(
  void
) {
  static char const * const x = $macro;
  if (printf("%s\\n", x) >= 0) {
    if (fflush(stdout) == 0) {
      return EXIT_SUCCESS;
    }
  }
  return EXIT_FAILURE;
}
EOF

  cc -o $main.exe $main

  ./$main.exe

  rm $main.exe $main

}; readonly -f config_h_get_string
