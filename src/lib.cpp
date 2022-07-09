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

void log(const std::string &msg, const unsigned level) {
  // save the last message
  static std::string last_msg;

  // if the last message does not end with a newline, print the current message
  // without the level info
  if (last_msg.empty() || (!last_msg.empty() && last_msg.back() == '\n')) {
    switch (level) {
    case 0:
      std::cout << FG_GRN << "  debug: " << RST;
      break;
    case 1:
      std::cout << FG_CYN << "   info: " << RST;
      break;
    case 2:
      std::cout << FG_YEL << "   warn: " << RST;
      break;
    case 3:
    default:
      std::cout << FG_RED << "  error: " << RST;
      break;
    }
  }

  std::cout << msg << std::flush;
  last_msg = msg;
}

void log(const std::string &msg, const LogLevel level) {
  log(msg, static_cast<unsigned>(level));
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
