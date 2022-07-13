#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "image.h"

Image::Image() {
  _width = 0;
  _height = 0;
  _channels = 0;
  _size = 0;
  _data = nullptr;
}

Image::Image(const std::string &path, int channels_force) {
  if (!read(path, channels_force)) panic("failed to read image from " + path);
}

Image::Image(int width, int height, int channels)
    : _width(width), _height(height), _channels(channels) {
  _size = static_cast<size_t>(_width) * _height * _channels;
  // we use malloc here because stb_image uses free() to free the memory
  _data = (unsigned char *)malloc(sizeof(unsigned char) * _size);

  if (_data == nullptr) panic("failed to allocate memory for image");

  chk_p(memset(_data, (unsigned char)0, _size));
}

Image::Image(const Image &other)
    : Image(other._width, other._height, other._channels) {
  void *dest = memcpy(_data, other._data, _size);
  if (dest != _data) panic("failed to copy image");
}

Image::~Image() {
  if (_data != nullptr) stbi_image_free(_data);
}

const int &Image::width() const { return _width; }
int &Image::width() { return _width; }
void Image::width(const int &width) { _width = width; }

const int &Image::height() const { return _height; }
int &Image::height() { return _height; }
void Image::height(const int &height) { _height = height; }

const int &Image::channels() const { return _channels; }
int &Image::channels() { return _channels; }
void Image::channels(const int &channels) { _channels = channels; }

const size_t &Image::size() const { return _size; }
size_t &Image::size() { return _size; }
void Image::size(const size_t &size) { _size = size; }

const unsigned char *Image::data() const { return _data; }
unsigned char *Image::data() { return _data; }
void Image::data(const unsigned char *data) { memcpy(_data, data, _size); }

bool Image::read(const std::string &path, int channels_force) {
  _data =
      stbi_load(path.c_str(), &_width, &_height, &_channels, channels_force);
  channels() = channels_force == 0 ? channels() : channels_force;
  return data() != nullptr;
}

bool Image::write(const std::string &path) const {
  bool success;
  const ImageType type = get_img_type(path);

  switch (type) {
  case ImageType::png:
    success = stbi_write_png(path.c_str(), width(), height(), channels(),
                             data(), width() * channels());
    break;
  case ImageType::jpg:
    success = stbi_write_jpg(path.c_str(), width(), height(), channels(),
                             data(), 100);
    break;
  case ImageType::bmp:
    success =
        stbi_write_bmp(path.c_str(), width(), height(), channels(), data());
    break;
  default:
    log("unknown image type from " + path + " - image not saved", 3);
    success = false;
    break;
  }
  return success;
}

Image *Image::crop(int x, int y, int width, int height) const {
  Image *cropped = new Image(width, height, channels());

  for (int i = 0; i < height; i++) {
    if (i + y >= _height || i + y < 0) continue;
    for (int j = 0; j < width; j++) {
      if (j + x >= _width || j + x < 0) continue;
      chk_p(memcpy(cropped->data() + (i * width + j) * channels(),
                   data() + ((y + i) * _width + x + j) * channels(),
                   channels()));
    }
  }

  return cropped;
}
