#include "lib.h"

#include "app.h"
#include "image.h"

#include "m.h"

#define N 1 << 5
unsigned long _no_asserts = 0;

void dummy_test(void) {
  assert_eq(1, 1);
  assert_neq(1, 2);
  assert_lt(1, 2);
  assert_gt(2, 1);
  assert_leq(1, 1);
  assert_geq(1, 1);
}

void memory_test_0(void) {
  const Image image = Image(1920, 1080);
  assert_neq(image.data(), nullptr);
}

void memory_test_1(void) {
  const Image image = Image(1920, 1080);
  assert_neq(image.data(), nullptr);
  assert_eq(image.width(), 1920);
  assert_eq(image.height(), 1080);
  assert_eq(image.channels(), 3);
  assert_eq(image.size(), 1920 * 1080 * 3);
}

void memory_test_2(void) {
  const Image image = Image(1920, 1080);
  for (int i = 0; i < N; i++) {
    const Image *c = image.crop(0, 0, 64, 64);
    assert_neq(c, nullptr);
    assert_eq(c->width(), 64);
    assert_eq(c->height(), 64);
    assert_eq(c->channels(), 3);
    assert_eq(c->size(), 64 * 64 * 3);
    delete c;
  }
}

void memory_test_3(void) {
  const Image image = Image(1920, 1080);
  std::vector<Image *> v;
  for (int i = 0; i < N; i++) {
    v.push_back(image.crop(0, 0, 64, 64));
  }
  for (int i = 0; i < N; i++) {
    assert_neq(v[i], nullptr);
    assert_neq(v[i]->data(), nullptr);
    for (int j = 0; j < N; j++) {
      if (i != j) {
        assert_neq(v[i]->data(), v[j]->data());
      }
    }
  }
  for (int i = 0; i < N; i++) {
    delete v[i];
  }
}

void crop_test_0(void) {
  Image image = Image(0xff, 0xff, 1);
  const int w = image.width();
  assert_leq(image.width(), 0xff);
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < w; j++) {
      image.data()[i * w + j] = (unsigned char)i;
    }
  }
  for (int i = 0; i < w; i++) {
    const Image *c = image.crop(0, i, w, 1);
    assert_neq(c, nullptr);
    assert_eq(c->width(), w);
    assert_eq(c->height(), 1);
    assert_eq(c->channels(), 1);
    assert_eq(c->size(), w);
    for (int j = 0; j < w; j++) {
      assert_eq(c->data()[j], i);
    }
    delete c;
  }
}

void app_test_0(void) {
  char *argv[] = {(char *)"app",  (char *)"-i",     (char *)"../in",
                  (char *)"-o",   (char *)"../out", (char *)"-e",
                  (char *)".jpg", (char *)"-s",     (char *)"30,60,64",
                  (char *)nullptr};
  App app = App(9, argv);
  app.check_args();
  assert(app.path_to_input_folder().compare("../in") == 0);
  assert(app.path_to_output_folder().compare("../out") == 0);
  assert(app.image_ext().compare(".jpg") == 0);
  assert(app.path_to_config_folder().compare("../in") == 0);
  assert_eq(app.min_object_size(), 30);
  assert_eq(app.max_object_size(), 60);
  assert_eq(app.target_width(), 64);
  assert_eq(app.target_height(), 64);
}

void app_test_1(void) {
  char *argv[] = {(char *)"app", (char *)"-i",     (char *)"../in",
                  (char *)"-o",  (char *)"../out", (char *)nullptr};
  App app = App(5, argv);
  app.check_args();

  std::vector<std::string> files;
  get_files_in_folder("../lib", files);
  assert_eq(files.size(), 3);
}

void app_test_2(void) {
  char *argv[] = {(char *)"app", (char *)"-i",     (char *)"../in",
                  (char *)"-o",  (char *)"../out", (char *)nullptr};
  App app = App(5, argv);
  app.check_args();

  std::vector<std::string> files;
  get_files_in_folder("../lib", files, ".hpp");
  assert_eq(files.size(), 1);

  files.clear();
  get_files_in_folder("../lib", files, ".h");
  assert_eq(files.size(), 2);
}

int main(void) {
  test_case(dummy_test);

  test_case(memory_test_0);
  test_case(memory_test_1);
  test_case(memory_test_2);
  test_case(memory_test_3);

  test_case(crop_test_0);

  test_case(app_test_0);
  test_case(app_test_1);
  test_case(app_test_2);

  return EXIT_SUCCESS;
}
