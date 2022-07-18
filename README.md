# Fast Crop for Images around YOLO bounding boxes

[![Linux](https://svgshare.com/i/Zhy.svg)](https://docs.microsoft.com/en-us/windows/wsl/tutorials/gui-apps)
[![GitHub license](https://img.shields.io/github/license/ThomasByr/YOLO_crop)](https://github.com/ThomasByr/YOLO_crop/blob/master/LICENSE)
[![GitHub commits](https://badgen.net/github/commits/ThomasByr/YOLO_crop)](https://GitHub.com/ThomasByr/YOLO_crop/commit/)
[![GitHub latest commit](https://badgen.net/github/last-commit/ThomasByr/YOLO_crop)](https://gitHub.com/ThomasByr/YOLO_crop/commit/)
[![Maintenance](https://img.shields.io/badge/maintained%3F-yes-green.svg)](https://GitHub.com/ThomasByr/YOLO_crop/graphs/commit-activity)

[![C/C++ CI](https://github.com/ThomasByr/YOLO_crop/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/ThomasByr/YOLO_crop/actions/workflows/c-cpp.yml)
[![CodeQL](https://github.com/ThomasByr/YOLO_crop/actions/workflows/codeql.yml/badge.svg)](https://github.com/ThomasByr/YOLO_crop/actions/workflows/codeql.yml)
[![Publish](https://github.com/ThomasByr/YOLO_crop/actions/workflows/publish.yml/badge.svg)](https://github.com/ThomasByr/YOLO_crop/actions/workflows/publish.yml)
[![GitHub version](https://badge.fury.io/gh/ThomasByr%2FYOLO_crop.svg)](https://github.com/ThomasByr/YOLO_crop)
[![Author](https://img.shields.io/badge/author-@ThomasByr-blue)](https://github.com/ThomasByr)

1. [✏️ Setup](#️-setup)
2. [💁 More infos and Usage](#-more-infos-and-usage)
3. [🧪 Testing](#-testing)
4. [⚖️ License](#️-license)
5. [🔄 Changelog](#-changelog)
6. [🐛 Bugs & TODO](#-bugs--todo)

## ✏️ Setup

> **Note**
> This repository contains very specific instructions for some un-related codebase. That being said, the present code should be as generic as possible for you to tweak it to your likings without any issues, _hopefully_.

Please make sure you do run a recent enough version of Linux, `g++ >= 4.8.5 w/ CentOS 7` should be enough though, with possible non-broken links to **posix threads** and all **gnu standard** extensions. This program uses the flag `-std=gnu++11` to compile.

This program takes images as input, as well as a config file, which are basically rectangles outputs from YOLO (which is a shape detection neural network). It then loop through all objects in the config file, optionally ignoring those whose size isn't in a specific range, and create new images cropping the original one.

Compile a release version of the program with :

```bash
make release
```

The produced executable binary is to be found inside of the `bin` folder.

## 💁 More infos and Usage

The program takes command line arguments from (`..` indicating no short option) :

- `-h, --help` : display this help and exit
- `-v, --version` : display version and exit
- `-l, --license` : display license and exit
- `-i, --in` : input folder
- `-o, --out` : output folder
- `-c, --cfg` : config folder (defaults to the input folder)
- `-e, --ext` : image file extension (defaults to .png)
- `-t, --thrds` : max number of threads (defaults to 8)
- `-s, --size` : specific size of the objects (defaults to no size restriction)
- `-p, --padd` : add a little padding to the bounding box (defaults to 0)
- `.., --rect` : use rectangle a an insides crop shape
- `.., --squr` : use square as an inside crop shape
- `.., --crcl` : use circle as an inside crop shape
- `.., --llps` : use ellipse as an inside crop shape
- `-b, --bg` : background image (defaults to none)
- `.., --clss` : only look for the specified class (defaults to all)
- `.., --cnfd` : specify a minimum confidence threshold (defaults to .5)

The specific size input should match the following pattern : `"min, max, w, h"`, which will result in the following behavior. The program will only crop around objects whose minimum size (the minimum between the width and the height of the rectangle defined by YOLO) is greater than or equal to `min`, and maximum size (same thing) is less than or equal to `max`. It will then crop the objects around their center with a new rectangle of width `w` and height `h`. If both `w` and `h` are unspecified, the new rectangle's dimensions will match the one defined by YOLO. If there is only one value specified (let's say that only `w` is specified), the program will crop according to the square of width `w`. To force only one of the two dimensions, please set one to zero ; setting values to your system's `EOF` will let them undefined.

Additionally, since v2, you can crop in a variety of new ways. At the time of writing, you can choose between `rectangle`, `square`, `circle` and `ellipse`. All previous four shapes only apply to the bounding box defined by YOLO. It works as follow : if you do not specify any shape and force the cropped size, the program will crop the original image with that size, around the center point defined by the bounding box. Then if you do specify any shape, it will crop according to that shape whose dimensions are defined by the **outer rectangle** bounding box. It is up to you to force the dimension of the final image, which, if you choose from either circle or ellipse, is guarantied to have rounded black corners. This you can avoid by specifying a path to a background default image (this argument will only eliminate dark edges when cropping outside of the original image when used with no specific shape). Note that the background image locks the number of channels used for image processing. The program will first crop the background image to the desired size (either the one you chose or the one defined by the bounding box) at the center of the background image, and then copy the YOLO-recognized subject above it, according to the shape. No checks are performed regarding the size of the background image, you might want to supply one large enough.

So, a legal launching instruction could be :

```bash
./bin/YOLO_crop -i orig -o dest -e .jpg -t 4 -s 30,60,64 --llps -b orig/bg.png
```

Just in case, this will tell the program to take all files with the `.jpg` extension in the folder named `orig`, where there is also matching named `.txt` files for the config, look only for objects whose minimum size is bigger than `30` and maximum size is less than `60`, and crop according to the `ellipse` defined by that bounding box. The program will then crop the `orig/bg.png` at the center point to create a new `64x64` image for it to then paste the cropped ellipse in the middle. It will then save it inside the folder named `dest` using the `.jpg` format.

> **Warning**
> Please be thoughtfull when naming/generating images/config files. We assume each image in the input folder named `x.ext` will have a matching twin text file named `x.txt` in the config folder.

Here is the current structure of the config text file that we assume you are using (**without** the first heading line) :

```txt
class_id center_x center_y width    height    confidence_score
0        0.085143 0.38912  0.002182 0.0027172 0.9954308
```

Note that only `class_id` field is parsed as an int, and all others are doubles. A friendly reminder that here we use relative coordinates (meaning all doubles are in the range `[0, 1]`).

## 🧪 Testing

Oh god... please don't.

Still, make sure you have `valgrind` up and ready and then run :

```bash
cd tests && make check
```

## ⚖️ License

This project is licensed under the GPL-3.0 new or revised license. Please read the [LICENSE](LICENSE) file.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
- Neither the name of the YOLO_crop authors nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## 🔄 Changelog

> For obvious reasons, some feature were not reported here and have been remove from git entirely.

`v0` - basic crop

- we cropped images using a fixed sized rectangle
- use of OpenCV

`v1` - home-made image processing wizardry

- grabbed stb_image code headers
- cpp thread pool implementation using basic thread, future and mutex
- implemented basic size selection

`v2` - cropping shapes

- choosing from square, rectangle, circle and ellipse
- static type cast
- shared pointers for default background and improved performances
- class and confidence extra selection

`v3` - final revision

- option to add a little padding horizontally or vertically
- command line issue fix

## 🐛 Bugs & TODO

**known bugs** (final correction patch version)

- ~~`-s, --siz` signed comparison~~ (v1.1.2)
- ~~possible int overflow detected by security analysis~~ (v2)
- ~~bad offset when cropping as circle or ellipse~~ (v2.1)

**todo** (first implementation version)

- [x] integrate YOLO basic object detection (v1)
- [x] thread pool (v1)
- [x] basic size selection (v1)
- [x] alter original crop with shape selection (v2)
