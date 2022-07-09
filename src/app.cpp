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

void print_help [[noreturn]] (const std::string &msg = "") {
  int status = msg.empty() ? EXIT_SUCCESS : EXIT_FAILURE;
  if (!msg.empty()) log(msg, 3);

  std::stringstream ss;
  ss << "crop_img\n"
     << "version: " << __VERSION_MAJOR__ << "." << __VERSION_MINOR__ << "."
     << __VERSION_PATCH__ << "\n";
  ss << " author: " << __AUTHOR__ << "\n\n";
  ss << "  usage: crop_img [OPTION]...\n";
  ss << "-h, --help\t\t\tdisplay this help and exit\n";
  ss << "-v, --version\t\t\tdisplay version and exit\n";
  ss << "-i, --in\t\t\tinput folder\n";
  ss << "-o, --out\t\t\toutput folder\n";
  ss << "-c, --cfg\t\t\tconfig folder (defaults to the input folder)\n";
  ss << "-e, --ext\t\t\timage file extension (defaults to .png)\n";
  ss << "-t, --thrds\t\t\tmax number of threads (defaults to 8)\n";
  ss << "-s, --siz\t\t\tspecified size from \"min, max, w, h\" "
        "(defaults to no size restriction)\n";

  std::cout << ss.str() << std::flush;
  std::exit(status);
}

void print_version [[noreturn]] () {
  std::stringstream ss;
  ss << "crop_img\n"
     << "version: " << __VERSION_MAJOR__ << "." << __VERSION_MINOR__ << "."
     << __VERSION_PATCH__ << "\n";
  ss << " author: " << __AUTHOR__ << "\n";

  std::cout << ss.str() << std::flush;
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
        {"help", no_argument, nullptr, 'h'},
        {"version", no_argument, nullptr, 'v'}, {nullptr, 0, nullptr, 0},
  };
  static const char *short_options = "i:o:c:e:t:s:hv";

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
    case 'h':
      print_help();
      panic("unreachable");
    case 'v':
      print_version();
      panic("unreachable");
    default:
      bad_opt = std::string(argv[optind - 1]);
      ss.clear();
      ss << "unrecognized option '" << bad_opt << "'\n";
      print_help(ss.str());
      panic("unreachable");
    }
  }
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
  if (_target_width != EOF && _target_height == EOF) {
    _target_height = _target_width;
  }
  if (_target_width == EOF && _target_height != EOF) {
    _target_width = _target_height;
  }
  if (_min_object_size != EOF && _min_object_size < 0) {
    print_help("minimum object size must be >= 0\n");
  }
  if (_max_object_size != EOF && _max_object_size < 0) {
    print_help("maximum object size must be >= 0\n");
  }
  if (_min_object_size != EOF && _max_object_size != EOF &&
      _min_object_size > _max_object_size) {
    print_help("minimum object size must be <= maximum object size\n");
  }
  if (_target_width != EOF && _target_width < 0) {
    print_help("target width must be >= 0\n");
  }
  if (_target_height != EOF && _target_height < 0) {
    print_help("target height must be >= 0\n");
  }
  switch (get_img_type(_image_ext)) {
  case ImageType::unknown:
    print_help("unrecognized image type\n");
    break;
  default:
    break;
  }
}

void get_files_in_folder(const std::string &path,
                         std::vector<std::string> &files,
                         const std::string &fileext = "") {
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(path.c_str())) != nullptr) {
    while ((ent = readdir(dir)) != nullptr) {
      std::string filename(ent->d_name);
      if (fileext.empty() || filename.find(fileext) != std::string::npos) {
        files.push_back(filename);
      }
    }
    chk(closedir(dir));
  } else {
    panic("could not open directory '" + path + (char)047);
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
  int min_object_size, max_object_size, target_width, target_height;

  process_args()
      : img_path(""), cfg_path(""), out_path(""), img_name(""), img_ext(""),
        min_object_size(EOF), max_object_size(EOF), target_width(EOF),
        target_height(EOF) {}
};

int process(const struct process_args p_args) {
  const std::string img_path = p_args.img_path;
  const std::string cfg_path = p_args.cfg_path;
  const std::string out_path = p_args.out_path;
  const std::string img_name = p_args.img_name;
  const std::string img_ext = p_args.img_ext;
  const int min_object_size = p_args.min_object_size;
  const int max_object_size = p_args.max_object_size;
  const int target_width = p_args.target_width;
  const int target_height = p_args.target_height;

  // img_path = "./data/test/test.png"
  // cfg_path = "./data/test/test.json"
  // out_path = "./data/test/"
  // img_name = "test"
  // img_ext  = ".png"

  // todo: load config and crop accordingly
  (void)cfg_path;
  (void)min_object_size;
  (void)max_object_size;
  (void)target_width;
  (void)target_height;

  int status = EXIT_SUCCESS;
  const Image source = Image(img_path);

  // todo: for now we just crop 64x64 and save it to the output folder
  const int w = source.width();
  const int h = source.height();

  for (int i = 0; i < w; i += 64) {
    for (int j = 0; j < h; j += 64) {
      Image dest = source.crop(i, j, 64, 64);
      std::string dest_name = out_path + img_name + '_' + std::to_string(i) +
                              '_' + std::to_string(j) + img_ext;
      if (!dest.write(dest_name)) {
        status = EXIT_FAILURE;
        log("could not write image" + dest_name + "\n", 1);
      }
    }
  }
  return status;
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

  log("found " + std::to_string(n) + " images\n", 1);

  // thread pool
  thread_pool tp(_max_threads);
  std::vector<std::future<int>> futures(n);

  struct process_args p_args;
  p_args.out_path = _path_to_output_folder + '/';
  p_args.img_ext = _image_ext;
  p_args.min_object_size = _min_object_size;
  p_args.max_object_size = _max_object_size;
  p_args.target_width = _target_width;
  p_args.target_height = _target_height;

  // process each image one at a time (in parallel)
  for (const auto &img_name : imgs_files) {
    // todo: get the config file name from the image name
    std::string img_path = _path_to_input_folder + '/' + img_name;
    std::string cfg_path = _path_to_config_folder + '/' + img_name;

    std::string img_name_no_ext =
        img_name.substr(0, img_name.find_last_of('.'));

    p_args.img_name = img_name_no_ext;
    p_args.img_path = img_path;
    p_args.cfg_path = cfg_path;

    futures[idx++] = tp.push([p_args](int) { return process(p_args); });
  }

  // wait for all the threads to finish
  idx = 0;
  volatile unsigned progress = 0, last_progress = 0;
  const std::string desc = "Cutting Images" FG_WHT " \u2702 " RST;
  const std::string more = '[' + std::to_string(_max_threads) + ']';

  for (auto &f : futures) {
    switch (f.get()) {
    case EXIT_SUCCESS:
      break;
    default:
      log("error processing image '" + imgs_files[idx] + (char)047, 3);
      break;
    }
    idx++;

    progress = (idx * 100) / n;
    if (progress > last_progress) {
      display_progress(idx, n, desc, more);
      last_progress = progress;
    }
  }

  std::cout << std::endl;

  // terminate the thread pool
  tp.stop();

  return EXIT_SUCCESS;
}

std::ostream &operator<<(std::ostream &os, const App &app) {
  os << "App..." << '\n'
     << "path_to_input_folder: " << app._path_to_input_folder << '\n'
     << "path_to_config_folder: " << app._path_to_config_folder << '\n'
     << "path_to_output_folder: " << app._path_to_output_folder << '\n'
     << "image_ext: " << app._image_ext << '\n'
     << "max_threads: " << app._max_threads << '\n'
     << "min_object_size: " << app._min_object_size << '\n'
     << "max_object_size: " << app._max_object_size << '\n'
     << "target_width: " << app._target_width << '\n'
     << "target_height: " << app._target_height << '\n'
     << "config_folder_is_input_folder: " << app._config_folder_is_input_folder;
  return os;
}
