#include <iostream>
#include <fstream>

#include "utils.h"

namespace fs = std::filesystem;

void load_rom(const fs::path &fname, std::vector<uint8_t> &rom) {
  std::fstream f(fname.c_str(), std::ios::binary | std::ios::in);
  if (!f) {
    std::cerr << "Error opening file\n";
    exit(-1);
  }

  /* Get the size of the file */
  size_t size = fs::file_size(fname);

  rom.resize(size);

  f.read((char *)rom.data(), size);
}

