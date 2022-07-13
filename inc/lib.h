#pragma once

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include <dirent.h>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>

#define __AUTHOR__ "ThomasByr"

#define __VERSION_MAJOR__ 1
#define __VERSION_MINOR__ 1
#define __VERSION_PATCH__ 0

#define TIMEOUT 3000

#define RST "\x1b[m\x1b[0m"

#define FG_RED "\x1b[0;31m"
#define FG_GRN "\x1b[0;32m"
#define FG_YEL "\x1b[0;33m"
#define FG_BLU "\x1b[0;34m"
#define FG_MAG "\x1b[0;35m"
#define FG_CYN "\x1b[0;36m"
#define FG_WHT "\x1b[0;37m"

#define BG_BLU "\x1b[40m"
#define BG_RED "\x1b[41m"
#define BG_GRN "\x1b[42m"
#define BG_ORA "\x1b[43m"
#define BG_CYN1 "\x1b[44m"
#define BG_YEL "\x1b[45m"
#define BG_CYN2 "\x1b[46m"
#define BG_WHT "\x1b[47m"

#define OPT_RCTG 1000 + 1
#define OPT_SQUR 1000 + 2
#define OPT_CRCL 1000 + 3
#define OPT_LLPS 1000 + 4

#ifndef DEBUG

#define std_debug(msg) (void)msg;

#else

#define std_debug(msg) log(msg, 0);

#endif

#define chk(op)       \
  do {                \
    if ((op) == -1) { \
      panic(#op);     \
    }                 \
  } while (0)

#define chk_p(op)          \
  do {                     \
    if ((op) == nullptr) { \
      panic(#op);          \
    }                      \
  } while (0)

void panic [[noreturn]] (const std::string &msg);

enum struct ImageType { png, jpg, bmp, unknown };

ImageType get_img_type(const std::string &path);

enum struct ImageShape { square, rectangle, circle, ellipse, undefined };

std::ostream &operator<<(std::ostream &os, const ImageShape &shape);

/**
 * @brief linear interpolation
 *
 * @param a left bound
 * @param b right bound
 * @param t value to interpolate
 * @return double - interpolated value
 */
double lerp(double a, double b, double t);

/**
 * @brief round to nearest integer
 *
 * @param d double to round
 * @return int - rounded value
 */
int round_to_int(double d);

/**
 * @brief implements repeat for a string
 *
 * @param str string
 * @param n unsigned
 * @return std::string - new string
 */
std::string repeat(std::string str, const unsigned n);

/**
 * @brief * operator for a string and an unsigned
 *
 * @param str string
 * @param n unsigned
 * @return std::string - new string
 */
std::string operator*(std::string str, const unsigned n);

enum struct LogLevel { Debug = 0, Info = 1, Warn = 2, Error = 3 };

/**
 * @brief outputs a string to stdout on a given level
 *
 * @param msg message to output
 * @param level level of output (0 = debug, 1 = info, 2 = warn, 3 = error)
 */
void log(const std::string &msg, const unsigned level = 0);
/**
 * @brief outputs a string to stdout on a given level
 *
 * @param msg message to output
 * @param level level of output
 */
void log(const std::string &msg, const LogLevel level);

/**
 * @brief displays a progress bar
 *
 * @param progress progress in total number of steps
 * @param total total number of steps
 */
void display_progress(const unsigned progress, const unsigned total,
                      const std::string &desc = "",
                      const std::string &more = "");

/**
 * @brief get all files in a directory
 *
 * @param path path to the directory
 * @param files vector of files
 * @param fileext optional file extension
 */
void get_files_in_folder(const std::string &path,
                         std::vector<std::string> &files,
                         const std::string &fileext = "");
