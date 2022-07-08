# Fast Crop for Images

[![Linux](https://svgshare.com/i/Zhy.svg)](https://docs.microsoft.com/en-us/windows/wsl/tutorials/gui-apps)
[![GitHub license](https://img.shields.io/github/license/ThomasByr/crop_imgs)](https://github.com/ThomasByr/crop_imgs/blob/master/LICENSE)
[![GitHub commits](https://badgen.net/github/commits/ThomasByr/crop_imgs)](https://GitHub.com/ThomasByr/crop_imgs/commit/)
[![GitHub latest commit](https://badgen.net/github/last-commit/ThomasByr/crop_imgs)](https://gitHub.com/ThomasByr/crop_imgs/commit/)
[![Maintenance](https://img.shields.io/badge/maintained%3F-yes-green.svg)](https://GitHub.com/ThomasByr/crop_imgs/graphs/commit-activity)

[![Python application](https://github.com/ThomasByr/crop_imgs/actions/workflows/python-app.yml/badge.svg)](https://github.com/ThomasByr/crop_imgs/actions/workflows/python-app.yml)
[![Pylint](https://github.com/ThomasByr/crop_imgs/actions/workflows/pylint.yml/badge.svg)](https://github.com/ThomasByr/crop_imgs/actions/workflows/pylint.yml)
[![GitHub version](https://badge.fury.io/gh/ThomasByr%2Fcrop_imgs.svg)](https://github.com/ThomasByr/crop_imgs)
[![Author](https://img.shields.io/badge/author-@ThomasByr-blue)](https://github.com/ThomasByr)

1. [✏️ Setup](#️-setup)
2. [💁 More infos](#-more-infos)
3. [🧪 Testing](#-testing)
4. [⚖️ License](#️-license)
5. [🐛 Bugs & TODO](#-bugs--todo)

## ✏️ Setup

> **Note**
> This repository contains very specific instructions for some un-related codebase. That being said, the present code should be as generic as possible for you to tweak it to your likings without any issues, hopefully.

Please make sure you do run a recent enough version of Linux, `g++ >= 8.4` should be enough though, with possible non-broken links to posix threads and all gnu standard extensions. This program uses the flag `-std=gnu++11` to compile.

This program takes images as input, as well as a config file, which are basically rectangles outputs from YOLO (which is a shape detection neural network). It then loop through all objects in the config file, optionally ignoring those whose size isn't in a specific range, and create new images cropping the original one.

Compile a release version of the program with :

```bash
make release
```

The produced executable binary is to be found inside of the `bin` folder.

## 💁 More infos

The program takes command line arguments from :

- `-h, --help` : display this help and exit
- `-v, --version` : display version and exit
- `-i, --in` : input folder
- `-o, --out` : output folder
- `-c, --cfg` : config folder (defaults to the input folder)
- `-e, --ext` : image file extension (defaults to .png)
- `-t, --thrds` : max number of threads (defaults to 8)
- `-s, --siz` : specific size of the objects (defaults to all objects)

The specific size input should match the following pattern : `"min, max, w, h"`, which will result in the following behavior. The program will only crop around objects whose minimum size (the minimum between the width and the height of the rectangle defined by YOLO) is greater than or equal to `min`, and maximum size (same thing) is less than or equal to `max`. It will then crop the objects around their center with a new rectangle of width `w` and height `h`. If both `w` and `h` are unspecified, the new rectangle's dimensions will match the one defined by YOLO. If there is only one value specified (let's say that only `w` is specified), the program will crop according to the square of width `w`.

So, a legal launching instruction could be :

```bash
./bin/crop_img -i orig -o dest -e .jpg -t 4 -s 30,60,64
```

Just in case, this will tell the program to take all files with the `.jpg` extension in the folder named `orig`, where there is also matching named `.txt` files for the config, look only for objects whose minimum size is bigger than `30` and maximum size is less than `60`, and create a new `64x64` image for each one of them then save it inside the folder named `dest`.

Please be thoughtfull when naming/generating images/config files. We assume each image in the input folder named `x.ext` will have a matching twin text file named `x.txt`.

## 🧪 Testing

Oh god... please don't.

## ⚖️ License

This project is licensed under the GPL-3.0 new or revised license. Please read the [LICENSE](LICENSE) file.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
- Neither the name of the crop_imgs authors nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## 🐛 Bugs & TODO

**bugs** (final correction patch version)

This section has been deleted and removed from git history.

**todo** (first implementation version)

This section has been deleted and removed from git history.
