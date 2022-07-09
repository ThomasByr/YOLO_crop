#include "lib.h"

class Image {
private:
  int _width, _height, _channels;
  size_t _size;
  unsigned char *_data;
  ImageType _type;

public:
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

  const ImageType &type() const;
  ImageType &type();
  void type(const ImageType &type);

  bool read(const std::string &path, int channels_force = 0);
  bool write(const std::string &path);

  /**
   * @brief crop the image and return a new image
   *
   * @param x top-left x coordinate
   * @param y top-left y coordinate
   * @param width width of the cropped image
   * @param height height of the cropped image
   * @return Image& - reference to the cropped image
   */
  const Image &crop(int x, int y, int width, int height) const;
};
