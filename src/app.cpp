#include "ctpl.hpp"

#include "app.h"

static void sig_handler(int signal) {
  static int64_t ms = 0;
  static std::chrono::milliseconds elapsed;
  static auto base = std::chrono::high_resolution_clock::now();
  static auto now = base;

  switch (signal) {
  case SIGINT:
    // count elapsed time since last SIGINT
    now = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - base);
    ms = elapsed.count();

    if (ms <= 1) {
    } else if (ms <= TIMEOUT) {
      // force exit
      chk(write(STDOUT_FILENO, "\033[2K\r", 5));
      std::cout << FG_RED << "Ctrl-C (user interrupt)." << RST << '\n';
      std::exit(EXIT_FAILURE);
    } else {
      // reset timer
      ms = 0;
      base = now;
    }

    chk(write(STDOUT_FILENO, "\033[2K\r", 5));
    std::cout << FG_YEL << "Ctrl-C received (send again shortly to exit)."
              << RST << '\n';

    break;
  }
}

static void print_help [[noreturn]] (const std::string &msg = "") {
  int status = msg.empty() ? EXIT_SUCCESS : EXIT_FAILURE;
  if (!msg.empty()) log(msg, LogLevel::error);

  std::stringstream ss;
  ss << "YOLO_crop\n"
     << "version: " << __VERSION_MAJOR__ << "." << __VERSION_MINOR__ << "."
     << __VERSION_PATCH__ << "\n"
     << " author: " << __AUTHOR__ << "\n\n"
     << "  usage: YOLO_crop [OPTION]...\n"
     << "-h, --help\t\t\tdisplay this help and exit\n"
     << "-v, --version\t\t\tdisplay version and exit\n"
     << "-l, --license\t\t\tdisplay license and exit\n"
     << "-i, --in\t\t\tinput folder\n"
     << "-o, --out\t\t\toutput folder\n"
     << "-c, --cfg\t\t\tconfig folder (defaults to the input folder)\n"
     << "-e, --ext\t\t\timage file extension (defaults to .png)\n"
     << "-t, --thrds\t\t\tmax number of threads (defaults to 8)\n"
     << "-s, --siz\t\t\tspecified size from \"min, max, w, h\" "
        "(defaults to no size restriction)\n"
     << "  , --rect\t\t\tuse rectangle as an inside crop shape\n"
     << "  , --squr\t\t\tuse square as an inside crop shape\n"
     << "  , --crcl\t\t\tuse circle as an inside crop shape\n"
     << "  , --llps\t\t\tuse ellipse as an inside crop shape\n"
     << "-b, --bg\t\t\tbackground image (defaults to none)\n"
     << "  , --clss\t\t\tonly look for the specified class (defaults to all)\n"
     << "  , --cnfd\t\t\tspecify a minimum confidence threshold "
        "(defaults to .5)\n";

  std::cout << ss.str() << std::flush;
  std::exit(status);
}

static void print_version [[noreturn]] () {
  std::stringstream ss;
  ss << "YOLO_crop\n"
     << "version: " << __VERSION_MAJOR__ << "." << __VERSION_MINOR__ << "."
     << __VERSION_PATCH__ << "\n";
  ss << " author: " << __AUTHOR__ << "\n";

  std::cout << ss.str() << std::flush;
  std::exit(EXIT_SUCCESS);
}

static void print_license [[noreturn]] () {
  static const char l[] =
      "This project is licensed under the [GPL-3.0](LICENSE) license. "
      "Please see the [license](LICENSE) file for more "
      "details.\n\nRedistribution and use in source and binary forms, with "
      "or without\nmodification, are permitted provided that the following "
      "conditions are met:\n\n- Redistributions of source code must retain "
      "the above copyright notice,\n  this list of conditions and the "
      "following disclaimer.\n\n- Redistributions in binary form must "
      "reproduce the above copyright notice,\n  this list of conditions "
      "and the following disclaimer in the documentation\n  and/or other "
      "materials provided with the distribution.\n\n- Neither the name of "
      "the YOLO_crop authors nor the names of its\n  contributors may be "
      "used to endorse or promote products derived from\n  this software "
      "without specific prior written permission.\n\nTHIS SOFTWARE IS "
      "PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\""
      "\nAND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED "
      "TO, THE\nIMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A "
      "PARTICULAR PURPOSE\nARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT "
      "HOLDER OR CONTRIBUTORS BE\nLIABLE FOR ANY DIRECT, INDIRECT, "
      "INCIDENTAL, SPECIAL, EXEMPLARY, OR\nCONSEQUENTIAL DAMAGES "
      "(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF\nSUBSTITUTE GOODS OR "
      "SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS\nINTERRUPTION) "
      "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER "
      "IN\nCONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR "
      "OTHERWISE)\nARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, "
      "EVEN IF ADVISED OF THE\nPOSSIBILITY OF SUCH DAMAGE.\n";
  std::cout << l << std::flush;
  std::exit(EXIT_SUCCESS);
}

App::~App() {}

App::App(int argc, char *argv[]) {
  int opt;
  static const struct option long_options[] {
    {"thrds", required_argument, nullptr, 't'},
        {"in", required_argument, nullptr, 'i'},
        {"out", required_argument, nullptr, 'o'},
        {"cfg", required_argument, nullptr, 'c'},
        {"ext", required_argument, nullptr, 'e'},
        {"thrds", required_argument, nullptr, 't'},
        {"siz", required_argument, nullptr, 's'},
        {"rect", no_argument, nullptr, OPT_RECT},
        {"squr", no_argument, nullptr, OPT_SQUR},
        {"crcl", no_argument, nullptr, OPT_CRCL},
        {"llps", no_argument, nullptr, OPT_LLPS},
        {"bg", required_argument, nullptr, 'b'},
        {"clss", required_argument, nullptr, OPT_CLSS},
        {"cnfd", required_argument, nullptr, OPT_CNFD},
        {"version", no_argument, nullptr, 'v'},
        {"license", no_argument, nullptr, 'l'}, {nullptr, 0, nullptr, 0},
  };
  static const char *short_options = "i:o:c:e:t:s:b:hvl";

  std::string bad_opt;
  std::stringstream ss;
  int err;

  while ((opt = getopt_long(argc, argv, short_options, long_options,
                            nullptr)) != -1) {
    switch (opt) {
    case 'i':
      _path_to_input_folder = optarg;
      break;
    case 'o':
      _path_to_output_folder = optarg;
      break;
    case 'c':
      _path_to_config_folder = optarg;
      break;
    case 'e':
      _image_ext = optarg;
      break;
    case 't':
      _max_threads = std::stoul(optarg);
      break;
    case 's':
      err = sscanf(optarg, "%d, %d, %d, %d", &_min_object_size,
                   &_max_object_size, &_target_width, &_target_height);
      if (err == EOF) {
        panic("invalid argument for --siz from " + std::string(optarg));
      }
      break;
    case OPT_RECT:
      _image_shape = ImageShape::rectangle;
      break;
    case OPT_SQUR:
      _image_shape = ImageShape::square;
      break;
    case OPT_CRCL:
      _image_shape = ImageShape::circle;
      break;
    case OPT_LLPS:
      _image_shape = ImageShape::ellipse;
      break;
    case 'b':
      _path_to_background_image = optarg;
      break;
    case OPT_CLSS:
      _class_id = std::stoi(optarg);
      _class_id_is_set = true;
      break;
    case OPT_CNFD:
      _min_confidence = std::stod(optarg);
      break;
    case 'h':
      print_help();
      panic("unreachable");
    case 'v':
      print_version();
      panic("unreachable");
    case 'l':
      print_license();
      panic("unreachable");
    default:
      bad_opt = std::string(argv[optind - 1]);
      ss.clear();
      ss << "unrecognized option '" << bad_opt << "'\n";
      print_help(ss.str());
      panic("unreachable");
    }
  }

  if (optind < argc) {
    ss.clear();
    ss << "unrecognized argument '" << argv[optind] << "'\n";
    print_help(ss.str());
    panic("unreachable");
  }
  optind = 0; // reinitialize optind to 0 to make getopt_long() work again
}

void App::check_args() {
  if (_path_to_input_folder.empty()) {
    print_help("missing input folder\n");
  }
  if (_path_to_output_folder.empty()) {
    print_help("missing output folder\n");
  }
  if (_path_to_config_folder.empty()) {
    _path_to_config_folder = _path_to_input_folder;
  }
  if (_path_to_input_folder.compare(_path_to_config_folder) == 0) {
    _config_folder_is_input_folder = true;
  }
  if (_target_width > 0 && _target_height == EOF) {
    _target_height = _target_width;
  }
  if (_target_width == EOF && _target_height > 0) {
    _target_width = _target_height;
  }
  if (_min_object_size != EOF && _min_object_size < 0) {
    print_help("minimum object size must be >= 0\n");
  }
  if (_max_object_size != EOF && _max_object_size < 0) {
    print_help("maximum object size must be >= 0\n");
  }
  if (_min_object_size < 0 && _max_object_size < 0 &&
      _min_object_size > _max_object_size) {
    print_help("minimum object size must be <= maximum object size\n");
  }
  if (_target_width != EOF && _target_width < 0) {
    print_help("target width must be >= 0\n");
  }
  if (_target_height != EOF && _target_height < 0) {
    print_help("target height must be >= 0\n");
  }
  if (_min_confidence < 0 || _min_confidence > 1) {
    print_help("minimum confidence must be between 0 and 1\n");
  }
  if (_class_id_is_set && _class_id < 0) {
    print_help("please let class id be EOF by not setting --clss manually\n");
  }

  switch (get_img_type(_image_ext)) {
  case ImageType::unknown:
    print_help("unrecognized image type extention\n");
    break;
  default:
    break;
  }
  if (!_path_to_background_image.empty()) {
    switch (get_img_type(_path_to_background_image)) {
    case ImageType::unknown:
      print_help("unrecognized background image type\n");
      break;
    default:
      break;
    }
  }
}

void create_dir(const std::string &path) {
  if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
    panic("could not create directory '" + path + (char)047);
  }
}

/// @brief holds the necessary information for a single image
struct process_args {
  std::string img_path, cfg_path, out_path, img_name, img_ext;
  int min_object_size, max_object_size, target_width, target_height, class_id;
  double min_confidence;
  ImageShape image_shape;
  Image *background_image;

  process_args()
      : img_path(""), cfg_path(""), out_path(""), img_name(""), img_ext(""),
        min_object_size(EOF), max_object_size(EOF), target_width(EOF),
        target_height(EOF), class_id(EOF), min_confidence(0.5),
        image_shape(ImageShape::undefined), background_image(nullptr) {}
};

static ssize_t process(const struct process_args p_args /* copy */) {
  const std::string img_path = p_args.img_path;
  const std::string cfg_path = p_args.cfg_path;
  const std::string out_path = p_args.out_path;
  const std::string img_name = p_args.img_name;
  const std::string img_ext = p_args.img_ext;
  const int min_object_size = p_args.min_object_size;
  const int max_object_size = p_args.max_object_size;
  const int target_width = p_args.target_width;
  const int target_height = p_args.target_height;
  const int class_id = p_args.class_id;
  const ImageShape image_shape = p_args.image_shape;
  const Image *background_image = p_args.background_image;
  double min_confidence = p_args.min_confidence;

  // img_path = "./data/test/test.png"
  // cfg_path = "./data/test/test.json"
  // out_path = "./data/test/"
  // img_name = "test"
  // img_ext  = ".png"

  ssize_t count = 0;
  int status = EXIT_SUCCESS, err = 0;
  int channel_force =
      background_image != nullptr ? background_image->channels() : 0;
  const Image source = Image(img_path, channel_force);

  std::ifstream cfg_file;
  try {
    cfg_file.open(cfg_path + img_name + ".txt", std::ios::out);
  } catch (const std::exception &e) {
    log("could not open config file '" + cfg_path + img_name + ".txt'",
        LogLevel::error);
    return EXIT_FAILURE;
  }
  // "class, x, y, width, height, confidence"
  static const char pattern[] = "%d %lf %lf %lf %lf %lf";
  std::string line;

  const int w = source.width();
  const int h = source.height();

  int bg_w = -1, bg_h = -1;
  if (background_image != nullptr) {
    bg_w = background_image->width();
    bg_h = background_image->height();
  }

  int _cls;
  double _cx, _cy, _w, _h, _score;
  int center_x, center_y, i, j, width, height, _width, _height, _r;

  // _cls is the class id
  // _cx is the center x coordinate, in the range [0, 1]
  // _cy is the center y coordinate, in the range [0, 1]
  // _w is the width, in the range [0, 1]
  // _h is the height, in the range [0, 1]

  // center_x is the center x coordinate, in the range [0, w]
  // center_y is the center y coordinate, in the range [0, h]
  // i is the top-left x coordinate, in the range [0, w]
  // j is the top-left y coordinate, in the range [0, h]
  // width (either the desired width or the width of the object)
  // height (same thing)
  // _width is the width of the object, in the range [0, w]
  // _height is the height of the object, in the range [0, h]

  // read cfg_file line by line
  while (std::getline(cfg_file, line)) { // boolean on conversion

    err = sscanf(line.c_str(), pattern, &_cls, &_cx, &_cy, &_w, &_h, &_score);
    if (err == EOF) break;

    if (class_id != EOF && _cls != class_id) continue;
    if (_score < min_confidence) continue;

    _width = round_to_int(lerp(0, w, _w));
    _height = round_to_int(lerp(0, h, _h));
    _r = std::min(_width, _height);

    if (min_object_size > 0 && min_object_size > std::min(_width, _height)) {
      continue;
    }
    if (max_object_size > 0 && max_object_size < std::max(_width, _height)) {
      continue;
    }

    width = target_width <= 0 ? _width : target_width;
    height = target_height <= 0 ? _height : target_height;
    center_x = round_to_int(lerp(0, w, _cx));
    center_y = round_to_int(lerp(0, h, _cy));

    Image *dest = nullptr;
    if (background_image != nullptr) {
      dest = background_image->crop_rect(bg_w / 2 - width / 2,
                                         bg_h / 2 - height / 2, width, height);
    }

    Image *subject = nullptr;
    // there might be a better way to do this...
    switch (image_shape) {
    case ImageShape::undefined: // we do not crop according to the bounding box
      i = center_x - width / 2;
      j = center_y - height / 2;
      subject = source.crop_rect(i, j, width, height, dest);
      break;
    case ImageShape::square: // square inside the bounding box
      i = center_x - _r / 2;
      j = center_y - _r / 2;
      subject = source.crop_rect(i, j, _r, _r, dest, width, height);
      break;
    case ImageShape::rectangle: // the bounding box itself
      i = center_x - _width / 2;
      j = center_y - _height / 2;
      subject = source.crop_rect(i, j, _width, _height, dest, width, height);
      break;
    case ImageShape::circle: // circle inside the bounding box
      i = center_x - _r / 2;
      j = center_y - _r / 2;
      subject = source.crop_ellipse(i, j, _r, _r, dest, width, height);
      break;
    case ImageShape::ellipse: // ellipse inside the bounding box
      i = center_x - _width / 2;
      j = center_y - _height / 2;
      subject = source.crop_ellipse(i, j, _width, _height, dest, width, height);
      break;
    }

    if (subject == nullptr) {
      log("could not crop image '" + img_path + "' to " +
              shape_to_string(image_shape) + '\n',
          LogLevel::error);
      status = EXIT_FAILURE;
      if (dest != nullptr) delete dest;
      continue;
    } // big oops

    // save the image
    const std::string subject_name =
        out_path + img_name + '_' + std::to_string(_cls) + '_' +
        std::to_string(center_x) + '_' + std::to_string(center_y) + '[' +
        std::to_string(count) + ']' + img_ext;
    if (!subject->write(subject_name)) {
      status = EXIT_FAILURE;
      log("could not write image '" + subject_name + "'\n", LogLevel::error);
    } else {
      count++; // saving was successful, increment the counter
    }
    delete subject; // which will delete dest if it was not nullptr
  }

  if (err == EOF) {
    status = EXIT_FAILURE;
    log("could not parse config file for image '" + img_path + "'\n",
        LogLevel::error);
  } // sscanf failed, break fallthrough

  try {
    cfg_file.close();
  } catch (const std::exception &e) {
    status = EXIT_FAILURE;
    log("could not close config file '" + cfg_path + img_name + ".txt'\n" +
            e.what() + '\n',
        LogLevel::error);
  }

  if (status == EXIT_FAILURE) {
    // instead of returning the status and then loging the error
    // we acknowledge errors and return the number of correctly saved images
    log("error(s) processing image '" + img_name + img_ext + "'\n",
        LogLevel::error);
  }

  return count;
}

int App::run() {
  using namespace ctpl;

  std::signal(SIGINT, sig_handler);

  // get the list of files in the input folder
  std::vector<std::string> imgs_files;

  get_files_in_folder(_path_to_input_folder, imgs_files, _image_ext);
  create_dir(_path_to_output_folder);

  const unsigned n = imgs_files.size();
  unsigned idx = 0;

  log("found " + std::to_string(n) + " images\n", LogLevel::info);

  // thread pool
  thread_pool tp(std::min(_max_threads, n));
  std::vector<std::future<ssize_t>> futures(n);

  // constant parameters for all images

  struct process_args p_args;
  p_args.out_path = _path_to_output_folder + '/';
  p_args.img_ext = _image_ext;
  p_args.min_object_size = _min_object_size;
  p_args.max_object_size = _max_object_size;
  p_args.target_width = _target_width;
  p_args.target_height = _target_height;
  p_args.image_shape = _image_shape;
  p_args.class_id = _class_id;
  p_args.min_confidence = _min_confidence;

  if (_path_to_background_image.empty()) {
    p_args.background_image = nullptr;
  } else {
    p_args.background_image = new Image(_path_to_background_image);
  }

  // process each image one at a time (in parallel)
  for (const auto &img_name : imgs_files) {
    std::string img_name_no_ext =
        img_name.substr(0, img_name.find_last_of('.'));

    // some image specific parameters

    p_args.img_name = img_name_no_ext;
    p_args.img_path = _path_to_input_folder + '/' + img_name;
    p_args.cfg_path = _path_to_config_folder + '/';

    futures[idx++] = /* register future trait */
        tp.push(std::move([p_args](ssize_t) { return process(p_args); }));
  }

  // wait for all threads to finish
  idx = 0;
  volatile unsigned progress = 0, last_progress = 0;
  const std::string desc = "Cutting Images" FG_WHT " \u2702 " RST;
  const std::string more = '[' + std::to_string(tp.size()) + ']';

  ssize_t count = 0; // number of images processed
  for (auto &f : futures) {
    count += f.get();

    idx++;
    progress = (idx * 100) / n;
    if (progress > last_progress) {
      display_progress(idx, n, desc, more); // need to add endl after
      last_progress = progress; // only update if progress has changed
    }
  }

  std::cout << std::endl;

  // terminate the thread pool
  tp.stop(false);

  // delete the background image if it was created
  if (p_args.background_image != nullptr) {
    delete p_args.background_image;
  }
  log("created " + std::to_string(count) + " images\n", LogLevel::info);

  return EXIT_SUCCESS;
}

std::ostream &operator<<(std::ostream &os, const App &app) {
  os << "App..." << '\n'
     << "path to input folder: " << app._path_to_input_folder << '\n'
     << "path to config folder: " << app._path_to_config_folder << '\n'
     << "path to output folder: " << app._path_to_output_folder << '\n'
     << "image extension: " << app._image_ext << '\n'
     << "maximum threads to use for processing: " << app._max_threads << '\n'
     << "minimum object size: " << app._min_object_size << '\n'
     << "maximum object size: " << app._max_object_size << '\n'
     << "target width: " << app._target_width << '\n'
     << "target height: " << app._target_height << '\n'
     << "custom crop shape: " << app._image_shape << '\n'
     << "path to background image: " << app._path_to_background_image << '\n'
     << "selected class id: " << app._class_id << '\n'
     << "minimum confidence score: " << app._min_confidence << '\n';

  return os;
}
