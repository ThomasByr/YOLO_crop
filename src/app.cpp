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
  if (!msg.empty()) log(msg + '\n', LogLevel::error);

  std::stringstream ss;
  ss << "YOLO_crop\n"
     << "version: " << __VERSION_MAJOR__ << "." << __VERSION_MINOR__ << "."
     << __VERSION_PATCH__ << "\n"
     << " author: " << __AUTHOR__ << "\n\n"
     << "  usage: YOLO_crop [OPTION]...\n"
     << "-h, --help\t\tdisplay this help and exit\n"
     << "-v, --version\t\tdisplay version and exit\n"
     << "-l, --license\t\tdisplay license and exit\n"
     << "-i, --in <>\t\tinput folder\n"
     << "-o, --out <>\t\toutput folder\n"
     << "-c, --cfg <>\t\tconfig folder (defaults to the input folder)\n"
     << "-e, --ext <>\t\timage file extension (defaults to .png)\n"
     << "-t, --thrds <>\t\tmax number of threads (defaults to 8)\n"
     << "-s, --size <>\t\tspecified size from \"min, max, w, h\" "
        "(defaults to no size restriction)\n"
     << "-p, --padd <>\t\tadd a little padding to the bounding box "
        "(defaults to 0)\n"
     << "  , --lock\t\tdo not allow cropping outside of the original image\n"
     << "  , --squr\t\tuse square as an inside crop shape\n"
     << "  , --rect\t\tuse rectangle as an inside crop shape\n"
     << "  , --crcl\t\tuse circle as an inside crop shape\n"
     << "  , --llps\t\tuse ellipse as an inside crop shape\n"
     << "-b, --bg <>\t\tbackground image (defaults to none)\n"
     << "  , --clss <>\t\tonly look for the specified class (defaults to "
        "all)\n"
     << "  , --cnfd <>\t\tspecify a minimum confidence threshold "
        "(defaults to .5)\n"
     << "  ,--trgt <>\t\ttarget minimum number of images to generate "
        "(defaults to no restriction)\n";

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
        {"size", required_argument, nullptr, 's'},
        {"padd", required_argument, nullptr, 'p'},
        {"lock", no_argument, nullptr, OPT_LOCK},
        {"rect", no_argument, nullptr, OPT_RECT},
        {"squr", no_argument, nullptr, OPT_SQUR},
        {"crcl", no_argument, nullptr, OPT_CRCL},
        {"llps", no_argument, nullptr, OPT_LLPS},
        {"bg", required_argument, nullptr, 'b'},
        {"clss", required_argument, nullptr, OPT_CLSS},
        {"cnfd", required_argument, nullptr, OPT_CNFD},
        {"trgt", required_argument, nullptr, OPT_TRGT},
        {"help", no_argument, nullptr, 'h'},
        {"version", no_argument, nullptr, 'v'},
        {"license", no_argument, nullptr, 'l'}, {nullptr, 0, nullptr, 0},
  };
  static const char *short_options = "i:o:c:e:t:s:b:p:hvl";

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
    case 'p':
      err = sscanf(optarg, "%d, %d", &_horizontal_padding, &_vertical_padding);
      if (err == EOF) {
        panic("invalid argument for --padd from " + std::string(optarg));
      }
      break;
    case OPT_LOCK:
      _lock = true;
      break;
    case OPT_RECT:
      _image_shape = ImageShape::rectangle;
      _set_shape_count++;
      break;
    case OPT_SQUR:
      _image_shape = ImageShape::square;
      _set_shape_count++;
      break;
    case OPT_CRCL:
      _image_shape = ImageShape::circle;
      _set_shape_count++;
      break;
    case OPT_LLPS:
      _image_shape = ImageShape::ellipse;
      _set_shape_count++;
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
    case OPT_TRGT:
      _min_target_images = std::stol(optarg);
      _min_target_images_is_set = true;
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
  if (_min_object_size > 0 && _max_object_size > 0 &&
      _min_object_size > _max_object_size) {
    print_help("minimum object size must be <= maximum object size\n");
  }
  if (_target_width != EOF && _target_width < 0) {
    print_help("target width must be >= 0\n");
  }
  if (_target_height != EOF && _target_height < 0) {
    print_help("target height must be >= 0\n");
  }
  if (_horizontal_padding > 0 && _vertical_padding == EOF) {
    _vertical_padding = _horizontal_padding;
  }
  if (_horizontal_padding == EOF && _vertical_padding > 0) {
    _horizontal_padding = _vertical_padding;
  }
  if (_horizontal_padding != EOF && _horizontal_padding < 0) {
    print_help("horizontal padding must be >= 0\n");
  }
  if (_vertical_padding != EOF && _vertical_padding < 0) {
    print_help("vertical padding must be >= 0\n");
  }
  if (_min_confidence < 0 || _min_confidence > 1) {
    print_help("minimum confidence must be between 0 and 1\n");
  }
  if (_class_id_is_set && _class_id < 0) {
    print_help("please let class id be EOF by not setting --clss manually\n");
  }
  if (_min_target_images_is_set && _min_target_images < 0) {
    print_help("please let target id be EOF by not setting --trgt manually\n");
  }

  if (_lock && _image_shape == ImageShape::undefined &&
      !_path_to_background_image.empty()) {
    print_help("locking cropping feature without any specific shape "
               "will result in the background image not being used\n"
               "(--bg is useless here)\n");
  }
  if (_image_shape == ImageShape::rectangle && _target_width <= 0 &&
      _target_height <= 0) {
    print_help("target width and height should be > 0 "
               "when using rectangle shape\n(--rect is useless here)\n");
  }
  switch (_image_shape) {
  case ImageShape::undefined:
  case ImageShape::rectangle:
  case ImageShape::square:
    if (_target_width <= 0 && _target_height <= 0 && _horizontal_padding <= 0 &&
        _vertical_padding <= 0 && !_path_to_background_image.empty()) {
      print_help("cropping without altering the image size will result in "
                 "the background image not being used\n"
                 "(--bg is useless here)\n");
    }
  default:
    break;
  }
  if (_set_shape_count > 1) {
    print_help("specifying more than one crop shape is not allowed\n");
  }

  switch (get_img_type(_image_ext)) {
  case ImageType::unknown:
    print_help("unrecognized image type extention\n");
  default:
    break;
  }
  if (!_path_to_background_image.empty()) {
    switch (get_img_type(_path_to_background_image)) {
    case ImageType::unknown:
      print_help("unrecognized background image type\n");
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
  int min_object_size, max_object_size, target_width, target_height,
      horizontal_padding, vertical_padding, class_id;
  bool lock;
  unsigned img_num;
  double min_confidence;
  ImageShape image_shape;
  Image *background_image;

  process_args()
      : img_path(""), cfg_path(""), out_path(""), img_name(""), img_ext(""),
        min_object_size(EOF), max_object_size(EOF), target_width(EOF),
        target_height(EOF), horizontal_padding(EOF), vertical_padding(EOF),
        class_id(EOF), lock(false), img_num(0), min_confidence(0.5),
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
  const int horizontal_padding = p_args.horizontal_padding;
  const int vertical_padding = p_args.vertical_padding;
  const int class_id = p_args.class_id;
  const bool lock = p_args.lock;
  const ImageShape image_shape = p_args.image_shape;
  const Image *background_image = p_args.background_image;
  const double min_confidence = p_args.min_confidence;
  const unsigned img_num = p_args.img_num;

  const int min_padding = // minimum padding if padding is set, otherwise 0
      std::min((horizontal_padding == EOF) ? 0 : horizontal_padding,
               (vertical_padding == EOF) ? 0 : vertical_padding);
  const int max_padding = // maximum padding if padding is set, otherwise 0
      std::max((horizontal_padding == EOF) ? 0 : horizontal_padding,
               (vertical_padding == EOF) ? 0 : vertical_padding);
  const int min_size = // minimum object size if object size is set, otherwise 0
      (min_object_size == EOF) ? 0 : min_object_size + 2 * min_padding;
  const int max_size = // maximum object size if object size is set, otherwise 0
      (max_object_size == EOF) ? 0 : max_object_size + 2 * max_padding;

  volatile ssize_t count = 0; // number correctly generated images
  int status = EXIT_SUCCESS;  // status return code
  int err = 0;                // error on sscanf
  int channel_force =         // force channel to be set to this value
      background_image == nullptr ? 0 : background_image->channels();
  const Image source = Image(img_path, channel_force);

  std::ifstream cfg_file;
  cfg_file.open(cfg_path + img_name + ".txt", std::ios::out);
  if (!cfg_file.is_open()) {
    log("could not open config file '" + cfg_path + img_name + ".txt'\n",
        LogLevel::error);
    status = EXIT_FAILURE;
  }

  // "class, x, y, width, height, confidence"
  static const char pattern[] = "%d %lf %lf %lf %lf %lf";
  std::string line; // one line of the config file

  const int w = source.width();  // width of the source image
  const int h = source.height(); // height of the source image

  int bg_w = -1, bg_h = -1;
  if (background_image != nullptr) {
    bg_w = background_image->width();  // width of the background image
    bg_h = background_image->height(); // height of the background image
  }

  int _cls;      // the class id of the object
  double _cx;    // the center x coordinate, in the range [0, 1]
  double _cy;    // the center y coordinate, in the range [0, 1]
  double _w;     // the width, in the range [0, 1]
  double _h;     // the height, in the range [0, 1]
  double _score; // the confidence of the object

  int center_x; // the center x coordinate, in the range [0, w]
  int center_y; // the center y coordinate, in the range [0, w]
  int i;        // the top-left x coordinate, in the range [0, w]
  int j;        // the top-right x coordinate, in the range [0, w]
  int width;    // width (the desired or the one of the object)
  int height;   // height (the desired or the one of the object)
  int _width;   // the width of the object, in the range [0, w]
  int _height;  // he height of the object, in the range [0, h]
  int _r;       // minimum radius of the object, in the range [0, w]

  // read cfg_file line by line
  while (std::getline(cfg_file, line) /* boolean on conversion */) {

    err = sscanf(line.c_str(), pattern, &_cls, &_cx, &_cy, &_w, &_h, &_score);
    if (err == EOF) break;

    if (class_id != EOF && _cls != class_id) continue;
    if (_score < min_confidence) continue;

    _width = round_to_int(lerp(0, w, _w)) +
             (horizontal_padding == EOF ? 0 : horizontal_padding * 2);
    _height = round_to_int(lerp(0, h, _h)) +
              (vertical_padding == EOF ? 0 : vertical_padding * 2);
    _r = std::min(_width, _height);

    if (min_object_size > 0 && min_size > std::min(_width, _height)) {
      continue;
    }
    if (max_object_size > 0 && max_size < std::max(_width, _height)) {
      continue;
    }

    width = target_width <= 0 ? _width : target_width;
    height = target_height <= 0 ? _height : target_height;
    center_x = round_to_int(lerp(0, w, _cx));
    center_y = round_to_int(lerp(0, h, _cy));

    // continue if locking blocks cropping feature
    if (lock) {
      if (center_x - width / 2 < 0 || center_x + width / 2 > w ||
          center_y - height / 2 < 0 || center_y + height / 2 > h) {
        continue;
      }
    }

    // the base image (either blank or background image)
    Image *dest = nullptr;
    if (background_image != nullptr) {
      dest = background_image->crop_rect(bg_w / 2 - width / 2,
                                         bg_h / 2 - height / 2, width, height);
    }

    // the cropped image (can use dest as a base)
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
        std::to_string(center_x) + '_' + std::to_string(center_y) + '_' +
        std::to_string(count) + '_' + std::to_string(img_num) + img_ext;
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

  if (cfg_file.is_open()) {
    cfg_file.close();
    if (cfg_file.fail() && !cfg_file.eof()) {
      status = EXIT_FAILURE;
      log("could not close config file '" + cfg_path + img_name + ".txt'\n",
          LogLevel::error);
    } // could not close the file
  }   // if the file was open, close it

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

  // figure out if we need a 's' at "image(s)"
  const char sf = n > 1u ? 's' : ' ';

  log("found " + std::to_string(n) + " image" + sf + '\n', LogLevel::info);

  // thread pool
  thread_pool tp(_max_threads);
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
  p_args.horizontal_padding = _horizontal_padding;
  p_args.vertical_padding = _vertical_padding;
  p_args.lock = _lock;
  p_args.class_id = _class_id;
  p_args.min_confidence = _min_confidence;

  if (_path_to_background_image.empty()) {
    // if no background image is provided, use a blank image
    // the blank image will be created in the crop method
    p_args.background_image = nullptr;
  } else {
    // otherwise, load the background image
    // create a common image for all images to use
    p_args.background_image = new Image(_path_to_background_image);
  }

  // process each image one at a time (in parallel)
  for (const auto &img_name : imgs_files) {
    std::string img_name_no_ext =
        img_name.substr(0, img_name.find_last_of('.'));

    // some image specific parameters

    p_args.img_num = idx;

    p_args.img_name = img_name_no_ext;
    p_args.img_path = _path_to_input_folder + '/' + img_name;
    p_args.cfg_path = _path_to_config_folder + '/';

    futures[idx++] = /* register future trait */
        tp.push(std::move([p_args](ssize_t) { return process(p_args); }));
  }

  // wait for all threads to finish

  idx = 0; // don't forget to reset the index
  volatile unsigned progress = 0, last_progress = 0;
  const std::string desc = "Cutting Images" FG_WHT " \u2702 " RST;
  const std::string more = '[' + std::to_string(tp.size()) + ']';

  volatile ssize_t count = 0;              // number of images processed
  const ssize_t trgt = _min_target_images; // target number of images
  for (auto &f : futures) {
    count += f.get();
    if (trgt != EOF && count > 0 && count >= trgt) break;

    progress = (++idx * 100) / n;
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

  // figure out if we need a 's' at "image(s)"
  const char sc = count > 1l ? 's' : ' ';
  log("created " + std::to_string(count) + " image" + sc + '\n',
      LogLevel::info);

  // if the number of generated images is less than the target number
  if (count < trgt) {
    log("could not create enough images\n", LogLevel::warning);
  }

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
     << "horizontal padding: " << app._horizontal_padding << '\n'
     << "vertical padding: " << app._vertical_padding << '\n'
     << "locking crop feature: " << app._lock << '\n'
     << "custom crop shape: " << app._image_shape << '\n'
     << "path to background image: " << app._path_to_background_image << '\n'
     << "selected class id: " << app._class_id << '\n'
     << "minimum confidence score: " << app._min_confidence << '\n'
     << "target minimum number of images: " << app._min_target_images << '\n';

  return os;
}
