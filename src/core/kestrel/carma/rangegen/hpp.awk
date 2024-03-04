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

BEGIN {

  gsub(/\\n/, "\n", copyright_notice);

  print copyright_notice;
  print "";
  print "#ifndef " hpp_guard;
  print "#define " hpp_guard;
  print "";
  print "#include <cstdint>";
  print "#include <utility>";
  print "";

  FS = ",";

}

{
  if (NR == 1) {
    print "#define " hpp_guard "_ROW \"" $0 "\"";
    print "";
    print "namespace kestrel {";
    print "namespace carma {";
    print "namespace rangegen {";
    print "";
    print "#ifndef KESTREL_CARMA_RANGEGEN_ROW";
    print "#define KESTREL_CARMA_RANGEGEN_ROW";
    print "";
    print "struct row {";
    for (i = 0; i++ < NF;) {
      printf "  int %s;\n", $i;
    }
    print "};";
    print "";
    print "#endif // #ifndef KESTREL_CARMA_RANGEGEN_ROW";
    print "";
    print "std::pair<row const *, std::size_t> " array_name "();";
  } else {
    num_servers = $1;
  }
}

END {
  print "";
  print "} // namespace rangegen";
  print "} // namespace carma";
  print "} // namespace kestrel";
  print "";
  print "#endif // #ifndef " hpp_guard;
}
