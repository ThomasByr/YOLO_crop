#pragma once

#include "lib.h"

class Image {
private:
  int _width, _height, _channels;
  size_t _size;
  unsigned char *_data = nullptr;

public:
  Image();
  Image(const std::string &path, int channels_force = 0);
  Image(int width, int height, int channels = 3);
  Image(const Image &other);
  ~Image();

  const int &width() const;
  int &width();
  void width(const int &width);

  const int &height() const;
  int &height();
  void height(const int &height);

  const int &channels() const;
  int &channels();
  void channels(const int &channels);

  const size_t &size() const;
  size_t &size();
  void size(const size_t &size);

  const unsigned char *data() const;
  unsigned char *data();
  void data(const unsigned char *data);

  bool read(const std::string &path, int channels_force = 0);
  bool write(const std::string &path) const;

  /**
   * @brief crop the image according to the rectangle and return a new image
   *
   * @param x top-left x coordinate
   * @param y top-left y coordinate
   * @param width width of the cropped image
   * @param height height of the cropped image
   * @param bg background image to fill the cropped area with
   * @param bw background image width
   * @param bh background image height
   * @return Image* - the cropped image
   */
  Image *crop_rect(int x, int y, int width, int height, Image *bg = nullptr,
                   int bw = EOF, int bh = EOF) const;

  /**
   * @brief crop the image accorging to the ellipse and return a new image
   *
   * @param x top-left x coordinate
   * @param y top-left y coordinate
   * @param width width of the cropped image
   * @param height height of the cropped image
   * @param bg background image to fill the cropped area with
   * @param bw background image width
   * @param bh background image height
   * @return Image* - the cropped image
   */
  Image *crop_ellipse(int x, int y, int width, int height, Image *bg = nullptr,
                      int bw = EOF, int bh = EOF) const;
};
