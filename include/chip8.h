#ifndef __CHIP8_H
#define __CHIP8_H
#include <vector>
#include <array>
#include <random>

static constexpr uint32_t CHIP8_HEIGHT = 32;
static constexpr uint32_t CHIP8_WIDTH = 64;
static constexpr uint32_t FONT_END = 0x50;
static constexpr uint32_t ROM_START= 0x200;

class Chip8 {
public:
  Chip8();
  Chip8(const std::vector<uint8_t> &mem);
  void step();

private:
  uint16_t pc;
  uint16_t sp;
  uint16_t reg_i;
  uint16_t status_reg;
  uint16_t delay_timer;
  uint16_t sound_timer;
  std::array<uint8_t, 16> regs;
  std::array<bool, 16> keys;
  std::array<uint8_t, 4086> memory;
  std::array<uint16_t, 16> stack;
  std::array<uint8_t, CHIP8_WIDTH * CHIP8_HEIGHT> video_memory;
  std::mt19937 random_generator;
};

#endif
