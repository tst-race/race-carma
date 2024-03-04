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

"use strict";

function for_each_file(dir, callback, prefix = dir.length + 1) {
  for (const x of FS.readdir(dir)) {
    if (x != "." && x != "..") {
      const y = dir + "/" + x;
      const st = FS.stat(y);
      if (FS.isFile(st.mode)) {
        callback(y, y.substr(prefix));
      } else if (FS.isDir(st.mode)) {
        for_each_file(y, callback, prefix);
      }
    }
  }
}

function on_done(error, result) {

  $("#all_fields").prop("disabled", false);
  $("#my_spinner").addClass("d-none");

  if (error) {
    console.error(error);
    $("#my_error").text(error);
    $("#my_error").removeClass("d-none");
    return;
  }

  const pairs = Object.entries(result).sort(
      (a, b) => a[0].localeCompare(b[0], "en", {numeric: true}));

  for (const [psn_slug, dir] of pairs) {
    const psn_text = unicodify(unslugify(psn_slug));
    const $a = $("<a>", {
      class: "list-group-item list-group-item-action",
      href: "#",
      text: psn_text,
      click: function() {
        const zip = new JSZip();
        for_each_file(dir, function(path, name) {
          zip.file(name, FS.readFile(path));
        });
        zip.generateAsync({type: "blob"}).then(function(content) {
          saveAs(content, psn_text + ".zip");
        });
      }
    });
    $a.prepend(`<i class="bi bi-download"></i>`);
    $("#my_results").append($a);
  }
}

function gogo() {

  $("#all_fields").prop("disabled", true);
  $("#my_spinner").removeClass("d-none");
  $("#my_error").addClass("d-none");
  $("#my_results").empty();

  const output_dir = "/output";
  rm_f_r(output_dir);
  native.kestrel_generate_configs({
    output_dir: output_dir,
    num_clients: $("#nc").val(),
    num_servers: $("#ns").val()
  },
                                  "on_done",
                                  "my_result");
}

function main() {
  show_body();
}
