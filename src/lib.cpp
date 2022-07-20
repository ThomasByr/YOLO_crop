#include "lib.h"

void panic(const std::string &msg) {
  std::stringstream ss;
  ss << FG_RED << "[panic] " << msg << " (" << strerror(errno) << ")" << RST
     << std::endl;
  throw std::runtime_error(ss.str());
}

ImageType get_img_type(const std::string &path) {
  const char *ext = strrchr(path.c_str(), '.');
  if (ext != nullptr) {
    if (strcmp(ext, ".png") == 0) {
      return ImageType::png;
    } else if (strcmp(ext, ".jpg") == 0) {
      return ImageType::jpg;
    } else if (strcmp(ext, ".bmp") == 0) {
      return ImageType::bmp;
    }
  }

  return ImageType::unknown;
}

std::ostream &operator<<(std::ostream &os, const ImageShape &shape) {
  return os << shape_to_string(shape);
}

std::string shape_to_string(const ImageShape &shape) {
  switch (shape) {
  case ImageShape::square:
    return "square";
  case ImageShape::rectangle:
    return "rectangle";
  case ImageShape::circle:
    return "circle";
  case ImageShape::ellipse:
    return "ellipse";
  default:
    return "undefined";
  }
}

double lerp(double a, double b, double t) { return a + (b - a) * t; }

int round_to_int(double d) { return (int)(d + (d < 0 ? -0.5 : 0.5)); }

std::string repeat(std::string str, const unsigned n) {
  if (n == 0) {
    str.clear();
    str.shrink_to_fit();
    return str;
  } else if (n == 1 || str.empty()) {
    return str;
  }
  const auto period = str.size();
  if (period == 1) {
    str.append(n - 1, str.front());
    return str;
  }
  str.reserve(period * n);
  unsigned m = 2;
  for (; m < n; m *= 2)
    str += str;
  str.append(str.c_str(), (n - (m / 2)) * period);
  return str;
}

std::string operator*(std::string str, const unsigned n) {
  return repeat(std::move(str), n);
}

void log(const std::string &msg, const LogLevel level) {
  // save if the last message ended with a newline
  static bool last_msg_ended_with_newline = true;
  chk(write(STDOUT_FILENO, "\033[2K\r", 5));

  // if the last message does not end with a newline, print the current message
  // without the level info
  if (last_msg_ended_with_newline) {
    switch (level) {
    case LogLevel::debug:
      std::cout << FG_GRN << "  debug: " << RST;
      break;
    case LogLevel::info:
      std::cout << FG_CYN << "   info: " << RST;
      break;
    case LogLevel::warning:
      std::cout << FG_YEL << "   warn: " << RST;
      break;
    case LogLevel::error:
    default:
      std::cout << FG_RED << "  error: " << RST;
      break;
    }
  }

  std::cout << msg << std::flush;
  last_msg_ended_with_newline = (msg.empty() || msg.back() == '\n');
}

void display_progress(const unsigned progress, const unsigned total,
                      const std::string &desc, const std::string &more) {
  // calculate the progress bar length
  static const unsigned bar_len = 50;
  const unsigned bar_len_p = bar_len * progress / total;
  const unsigned bar_len_m = bar_len - bar_len_p;

  // calculate the progress bar percentage
  const unsigned bar_pct = 100 * progress / total;

  // print the progress bar
  std::stringstream ss;
  ss << desc << ' ' << FG_GRN << std::string(bar_len_p, '-') << FG_RED
     << std::string(bar_len_m, '-') << RST << ' ' << FG_WHT << bar_pct << '%'
     << RST << ' ' << '(' << progress << '/' << total << ')' << more;

  std::cout << '\r' << RST << ss.str() << std::flush;
}

void get_files_in_folder(const std::string &path,
                         std::vector<std::string> &files,
                         const std::string &fileext) {
  size_t pos;
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(path.c_str())) != nullptr) {
    while ((ent = readdir(dir)) != nullptr) {
      std::string filename(ent->d_name);
      // if the file is not a directory
      if (filename != "." && filename != "..") {
        // if the file is a file
        if (ent->d_type == DT_REG) {
          // if the file matches *.fileext and fileext is at the end
          if (fileext.empty() ||
              ((pos = filename.find(fileext)) != std::string::npos &&
               filename.size() == fileext.size() + pos)) {
            files.push_back(filename);
          }
        }
      }
    }
    chk(closedir(dir));
  } else {
    panic("could not open directory '" + path + (char)047);
  }
}

unsigned count_files_in_folder(const std::string &path,
                               const std::string &fileext) {
  unsigned count = 0;
  size_t pos;
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(path.c_str())) != nullptr) {
    while ((ent = readdir(dir)) != nullptr) {
      std::string filename(ent->d_name);
      // if the file is not a directory
      if (filename != "." && filename != "..") {
        // if the file is a file
        if (ent->d_type == DT_REG) {
          // if the file matches *.fileext and fileext is at the end
          if (fileext.empty() ||
              ((pos = filename.find(fileext)) != std::string::npos &&
               filename.size() == fileext.size() + pos)) {
            count++;
          }
        }
      }
    }
    chk(closedir(dir));
  } else {
    panic("could not open directory '" + path + (char)047);
  }
  return count;
}
