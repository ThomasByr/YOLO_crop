#pragma once
#include "lib.h"

#include "image.h"

class App {
private:
  // input folder containing the files to be processed
  std::string _path_to_input_folder;
  // output folder for the processed files
  std::string _path_to_output_folder;

  // path to the config file folder if different from the input folder
  std::string _path_to_config_folder;
  bool _config_folder_is_input_folder = false;

  // image file extention
  std::string _image_ext = ".png";

  // max number of threads to use for processing
  unsigned _max_threads = 8;

  // minimum size of the object to be processed
  int _min_object_size = EOF;
  // maximum size of the object to be processed
  int _max_object_size = EOF;
  // target width of the generated cropped image
  int _target_width = EOF;
  // target height of the generated cropped image
  int _target_height = EOF;

public:
  /**
   * @brief Construct a new App object
   *
   * @param argc number of arguments
   * @param argv command line arguments
   */
  App(int argc, char *argv[]);
  /**
   * @brief Destroy the App object
   *
   */
  ~App();

  /**
   * @brief check all the arguments against a set of rules
   *
   */
  void check_args();
  /**
   * @brief display private members
   *
   */
  friend std::ostream &operator<<(std::ostream &os, const App &app);

  int run();
};
