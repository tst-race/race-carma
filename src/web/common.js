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

// window.onerror = function(msg, url, line) {
//   alert(msg);
// };

var native = undefined;

let my_vh = undefined;
function update_my_vh() {
  my_vh = window.innerHeight * 0.01;
  document.documentElement.style.setProperty('--my_vh', `${my_vh}px`);
}
window.addEventListener("resize", update_my_vh);
update_my_vh();

// TODO: Put the engine dir under idbfs_dir eventually.
var $body = $("body");
var idbfs_dir = "/idbfs";
var engine_dir = "/engine";
var plugins_dir = engine_dir + "/plugins";
var carma_dir = plugins_dir + "/carma";
var carma_fs_dir = carma_dir + "/fs";

// Flushes any pending filesystem changes to the browser.
function flush() {
  FS.syncfs(false, function() {});
}

// Recursively ensures that a directory exists.
function mkdir_p(path) {
  path = path.replace(/\/*$/, "");
  if (path.includes("/")) {
    mkdir_p(path.replace(/[^\/]*$/, ""));
  }
  if (path != "") {
    const info = FS.analyzePath(path);
    if (!info.exists) {
      FS.mkdir(path);
    } else if (!FS.isDir(info.object.mode)) {
      throw new Error("Path already exists as a non-directory: "
                      + JSON.stringify(path));
    }
  }
}

// Recursively deletes path, if it exists.
function rm_f_r(path) {
  const info = FS.analyzePath(path);
  path = info.path;
  if (info.exists) {
    if (FS.isDir(info.object.mode)) {
      path = path.replace(/\/*$/, "/");
      for (const x of FS.readdir(path)) {
        if (x != "." && x != "..") {
          rm_f_r(path + x);
        }
      }
      FS.rmdir(path);
    } else {
      FS.unlink(path);
    }
  }
}

// Asynchronously unzips a zip file src into an existing directory dst,
// then calls callback.
function unzip(src, dst, callback) {
  dst = dst.replace(/\/*$/, "/");
  JSZip.loadAsync(src).then(function(zip) {
    let n_walked = 0;
    let n_output = 0;
    zip.forEach(function(path, z) {
      ++n_walked;
      path = dst + path;
      if (path.endsWith("/")) {
        mkdir_p(path);
        ++n_output;
      } else {
        const info = FS.analyzePath(path);
        if (info.exists && !FS.isFile(info.object.mode)) {
          throw new Error("Path already exists as a non-file: "
                          + JSON.stringify(path));
        }
        z.async("uint8array").then(function(data) {
          FS.writeFile(path, data);
          ++n_output;
          if (n_output == n_walked) {
            // This case means all directories and files have now been
            // written out, and there was at least one file.
            callback();
          }
        });
      }
    });
    if (n_output == n_walked) {
      // This case means all directories and files have now been written
      // out, and there were no files.
      callback();
    }
  });
}

// Encodes a Uint8Array to a String. Same as kestrel::slugify().
function slugify(src) {
  if (src.length == 6 && src[0] == "s".charCodeAt(0)
      && src[1] == "h".charCodeAt(0) && src[2] == "a".charCodeAt(0)
      && src[3] == "r".charCodeAt(0) && src[4] == "e".charCodeAt(0)
      && src[5] == "d".charCodeAt(0)) {
    return "share%64";
  }
  let dst = "";
  for (let i = 0; i < src.length; ++i) {
    const c = src[i];
    if ((c >= "0".charCodeAt(0) && c <= "9".charCodeAt(0))
        || c == "_".charCodeAt(0)
        || (c >= "a".charCodeAt(0) && c <= "z".charCodeAt(0))
        || (i > 0 && i < src.length - 1
            && (c == "-".charCodeAt(0) || c == ".".charCodeAt(0)))) {
      dst += String.fromCharCode(c);
    } else if (c == "%".charCodeAt(0)) {
      dst += "%";
      dst += "%";
    } else {
      const x = c;
      const a = (x >> 4) & 0xF;
      const b = (x >> 0) & 0xF;
      dst += "%";
      dst += "0123456789ABCDEF"[a];
      dst += "0123456789ABCDEF"[b];
    }
  }
  return dst;
}

// Decodes a String to a Uint8Array. Same as kestrel::unslugify().
function unslugify(src) {
  const dst = new Uint8Array(src.length);
  let n = 0;
  let i = 0;
  let w = 0;
  let v = 0;
  for (const c of src) {
    if (i == 0) {
      if (c == "%") {
        ++i;
      } else {
        dst[n++] = c.charCodeAt(0);
      }
    } else if (i == 1) {
      if (c == "%") {
        dst[n++] = "%".charCodeAt(0);
        i = 0;
      } else if (c >= "0" && c <= "9") {
        w = c.charCodeAt(0);
        v = (c.charCodeAt(0) - "0".charCodeAt(0)) << 4;
        ++i;
      } else if (c >= "A" && c <= "F") {
        w = c.charCodeAt(0);
        v = (10 + (c.charCodeAt(0) - "A".charCodeAt(0))) << 4;
        ++i;
      } else if (c >= "a" && c <= "f") {
        w = c.charCodeAt(0);
        v = (10 + (c.charCodeAt(0) - "a".charCodeAt(0))) << 4;
        ++i;
      } else {
        dst[n++] = "%".charCodeAt(0);
        dst[n++] = c.charCodeAt(0);
        i = 0;
      }
    } else if (i == 2) {
      if (c >= "0" && c <= "9") {
        v |= (c.charCodeAt(0) - "0".charCodeAt(0));
        dst[n++] = v;
      } else if (c >= "A" && c <= "F") {
        v |= 10 + (c.charCodeAt(0) - "A".charCodeAt(0));
        dst[n++] = v;
      } else if (c >= "a" && c <= "f") {
        v |= 10 + (c.charCodeAt(0) - "a".charCodeAt(0));
        dst[n++] = v;
      } else {
        dst[n++] = "%".charCodeAt(0);
        dst[n++] = w;
        dst[n++] = c;
      }
      i = 0;
    }
  }
  if (i == 1) {
    dst[n++] = "%".charCodeAt(0);
  } else if (i == 2) {
    dst[n++] = "%".charCodeAt(0);
    dst[n++] = w;
  }
  return dst.slice(0, n);
}

// Decodes a Uint8Array to a String via UTF-8 with U+FFFD replacement.
// Typically used for display purposes.
function unicodify(src) {
  return new TextDecoder("utf-8", {fatal: false}).decode(src);
}

const $body_container = $("#body_container");

const $main_loading_containers = $(`
  <div class="main_loading_container">
    <div class="spinner-border" role="status">
      <span class="visually-hidden">Loading...</span>
    </div>
  </div>
  <div class="main_loading_container">
    <h1>Loading...</h1>
  </div>
`);
$main_loading_containers.hide();
$main_loading_containers.appendTo($body);

const $select_config = $(`
  <div class="select_config container text-center">
    <div class="row mb-2">
      <div class="col">
        <h1 class="d-flex justify-content-center align-items-center">
          <img class="me-2" src="kestrel.svg" style="height: 1em">Kestrel Client
        </h1>
      </div>
    </div>
    <div class="row">
      <div class="col">
        <label class="btn btn-primary">
          Load config...
          <input id="upload_picker" type="file" class="d-none">
        </label>
      </div>
    </div>
  </div>
`);
$select_config.hide();
$select_config.appendTo($body);

function show_body() {
  $main_loading_containers.hide();
  $select_config.hide();
  $body_container.show();
}

function show_loading() {
  $body_container.hide();
  $select_config.hide();
  $main_loading_containers.show();
}
show_loading();

function show_select_config() {
  $body_container.hide();
  $main_loading_containers.hide();
  $select_config.show();
}

function idbfs_done() {
  main();
}

function idbfs_init() {
  FS.mkdir(idbfs_dir);
  FS.mount(IDBFS, {}, idbfs_dir);
  FS.syncfs(true, function(error) {
    if (error) {
      const m = "Error: Error initializing IDBFS.";
      console.error(m);
      alert(m);
      return;
    }
    idbfs_done();
  });
}

var Module = {
  onRuntimeInitialized: function() {
    native = Module;
    //idbfs_init();
    idbfs_done();
  }
};
