/*
MIT License

Copyright (c) 2020 Niteesh

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef __CHIP8_H
#define __CHIP8_H
#include <vector>
#include <array>
#include <ctime>
#include <random>

static constexpr uint32_t CHIP8_HEIGHT = 32;
static constexpr uint32_t CHIP8_WIDTH = 64;
static constexpr uint32_t FONT_START = 0x50;
static constexpr uint32_t ROM_START= 0x200;

class Chip8;

typedef void (Chip8::*opcode_func)(int opcode);

class Chip8 {
public:
  Chip8();
  Chip8(const std::vector<uint8_t> &mem);
  void step();
  bool draw_screen(uint32_t *pixels);
  bool set_input(int8_t key, bool released);
  void update_timer();

private:
  bool     refresh_display{true};
  bool     waiting_for_key_press{false};
  int8_t   wait_key{-1};
  uint16_t pc{0x200};
  uint16_t sp{0x0};
  uint16_t reg_i{0x0};
  uint16_t delay_timer{0x0};
  uint16_t sound_timer{0x0};
  std::array<uint8_t, 16> regs{};
  std::array<bool, 16> keys{};
  std::array<uint8_t, 4086> memory{};
  std::array<uint16_t, 16> stack{};
  std::array<uint8_t, CHIP8_WIDTH * CHIP8_HEIGHT> video_memory{};
  std::mt19937 random_generator{(long unsigned int)time(nullptr)};

  uint16_t fetch_opcode();
  void decode_and_execute(int opcode);
  void unimplemented_opcode(int opcode);

  /* Opcodes */
  void op0xxx(int opcode);
  void op1xxx(int opcode);
  void op2xxx(int opcode);
  void op3xxx(int opcode);
  void op4xxx(int opcode);
  void op5xxx(int opcode);
  void op6xxx(int opcode);
  void op7xxx(int opcode);
  void op8xxx(int opcode);
  void op9xxx(int opcode);
  void opAxxx(int opcode);
  void opBxxx(int opcode);
  void opCxxx(int opcode);
  void opDxxx(int opcode);
  void opExxx(int opcode);
  void opFxxx(int opcode);

  static constexpr opcode_func opcodes[0x10] =
  {
    &Chip8::op0xxx,
    &Chip8::op1xxx,
    &Chip8::op2xxx,
    &Chip8::op3xxx,
    &Chip8::op4xxx,
    &Chip8::op5xxx,
    &Chip8::op6xxx,
    &Chip8::op7xxx,
    &Chip8::op8xxx,
    &Chip8::op9xxx,
    &Chip8::opAxxx,
    &Chip8::opBxxx,
    &Chip8::opCxxx,
    &Chip8::opDxxx,
    &Chip8::opExxx,
    &Chip8::opFxxx
  };
};

#endif

