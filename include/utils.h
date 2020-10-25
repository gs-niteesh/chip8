#ifndef __UTILS_H
#define __UTILS_H
#include <filesystem>
#include <vector>

void load_rom(const std::filesystem::path &fname, std::vector<uint8_t> &rom);

#endif

