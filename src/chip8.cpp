#include <SDL2/SDL.h>

#include <iostream>
#include <iomanip>
#include <ctime>
#include <string>
#include <array>

#include "chip8.h"
#include "utils.h"

static constexpr std::array<uint8_t, 80> fonts =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
  0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
  0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
  0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
  0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
  0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
  0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
  0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
  0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
  0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
  0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
  0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
  0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
  0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
  0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
  0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

Chip8::Chip8() {
  std::copy(fonts.begin(), fonts.end(), memory.begin() + FONT_START);
  std::fill(video_memory.begin(), video_memory.end(), 0);
  std::fill(stack.begin(), stack.end(), 0);
}

Chip8::Chip8(const std::vector<uint8_t> &mem) : Chip8()
{
  std::copy(mem.begin(), mem.end(), memory.begin() + ROM_START);
}

void Chip8::step() {
  uint16_t opcode = fetch_opcode();
  decode_and_execute(opcode);
}

void Chip8::decode_and_execute(int opcode) {
  uint8_t nibble = (opcode & 0xf000) >> 12;

  if (nibble > 16) {
    std::cerr << std::hex << pc << ": Invalid opcode\n";
    return ;
  }

  opcode_func function = opcodes[nibble];
  (this->*function)(opcode);

}

bool Chip8::draw_screen(uint32_t *pixels) {
  if (!refresh_display) return false;

  for (size_t i = 0; i < video_memory.size(); i++) {
    pixels[i] = video_memory[i] * 0xffffffff;
  }

  refresh_display = false;
  return true;
}

bool Chip8::set_input(int8_t key, bool released) {
  if (key < 0 || key > 16)
    return waiting_for_key_press;

  keys[key] = !released;
  if (released) {
    return waiting_for_key_press;
  }

  if (!waiting_for_key_press) return false;

  waiting_for_key_press = false;
  regs[wait_key] = key;
  wait_key = -1;
  return false;
}

void Chip8::update_timer() {
  if (delay_timer == 0) return ;
  delay_timer --;
}

uint16_t Chip8::fetch_opcode() {
  if (pc >= memory.size()) {
    std::cerr << "PC out of bounds\n";
    exit(-1);
  }

  uint16_t opcode;
  opcode = (memory[pc] << 8) | memory[pc + 1];
  pc += 2;
  return opcode;
}


/**
 * 0x00e0: clear display
 * 0x00ee: return
 */
void Chip8::op0xxx(int opcode) {

  if (opcode == 0x00e0) {
    std::fill(video_memory.begin(), video_memory.end(), 0);
    refresh_display = true;
  } else if (opcode == 0x00ee) {
    sp -= 1;
    uint16_t ret = stack[sp];
    pc = ret;
  } else {
    std::cerr << "pc: " << std::hex << pc - 2;
    std::cerr << " opcode: " << opcode << " unimplemented\n";
  }
}

/**
 * 0x1nnn: Goto nnn
 */
void Chip8::op1xxx(int opcode) {
  uint16_t nnn = opcode & 0xfff;
  pc = nnn;
}

/**
 * 0x2nnn: Call nnn
 */
void Chip8::op2xxx(int opcode) {
  uint16_t nnn = opcode & 0xfff;
  stack[sp++] = pc;
  pc = nnn;
}

/**
 * 0x3xnn: Vx == nn
 */
void Chip8::op3xxx(int opcode) {
  uint8_t nn = opcode & 0x00ff;
  uint8_t x = (opcode & 0x0f00) >> 8;
  pc = (regs[x] == nn) ? pc + 2 : pc;
}

/**
 * 0x4xnn: Vx != nn
 */
void Chip8::op4xxx(int opcode) {
  uint8_t nn = opcode & 0x00ff;
  uint8_t x = (opcode & 0x0f00) >> 8;
  pc = (regs[x] != nn) ? pc + 2 : pc;
}

/**
 * 0x5xy0: Vx == Vy
 */
void Chip8::op5xxx(int opcode) {
  uint8_t x = (opcode & 0x0f00) >> 8;
  uint8_t y = (opcode & 0x00f0) >> 4;
  pc = (regs[x] == regs[y]) ? pc + 2 : pc;
}

/**
 * 0x6xnn: Vx = nn
 */
void Chip8::op6xxx(int opcode) {
  uint8_t nn = opcode & 0x00ff;
  uint8_t x = (opcode & 0x0f00) >> 8;
  regs[x] = nn;
}

/**
 * 0x7xnn: Vx += nn
 */
void Chip8::op7xxx(int opcode) {
  uint8_t nn = opcode & 0x00ff;
  uint8_t x = (opcode & 0x0f00) >> 8;
  regs[x] += nn;
}

/**
 * 0x8xy0: Vx = nn
 * 0x8xy1: Vx |= nn
 * 0x8xy2: Vx &= nn
 * 0x8xy3: Vx ^= nn
 * 0x8xy4: Vx += nn
 * 0x8xy5: Vx -= Vy
 * 0x8xy6: Vx <<= 1
 * 0x8xy7: Vx = Vy - Vx
 * 0x8xye: Vx <<= 1
 */
void Chip8::op8xxx(int opcode) {
  uint8_t last_nibble = opcode & 0xf;
  uint8_t x = (opcode & 0x0f00) >> 8;
  uint8_t y = (opcode & 0x00f0) >> 4;

  if (last_nibble == 0x0) {
    regs[x] = regs[y];
  } else if (last_nibble == 0x1) {
    regs[x] |= regs[y];
  } else if (last_nibble == 0x2) {
    regs[x] &= regs[y];
  } else if (last_nibble == 0x3) {
    regs[x] ^= regs[y];
  } else if (last_nibble == 0x4) {
    uint16_t sum = regs[x] + regs[y];
    regs[0xf] = 0;

    if (sum > 0xffu) regs[0xf] = 1;
    regs[x] = sum;
  } else if (last_nibble == 0x5) {
    regs[0xf] = 0;
    if (regs[x] > regs[y]) regs[0xf] = 1;
    regs[x] -= regs[y];
  } else if (last_nibble == 0x6) {
    regs[0xf] = regs[x] & 0x1;
    regs[x] >>= 1;
  } else if (last_nibble == 0x7) {
    regs[0xf] = 0;

    if (regs[y] > regs[x]) regs[0xf] = 1;
    regs[x] = regs[y] - regs[x];
  } else if (last_nibble == 0xe) {
    regs[0xf] = (regs[x] & 0x80) >> 7;
    regs[x] <<= 1;
  } else {
    unimplemented_opcode(opcode);
  }

}

/**
 * 0x9xy0: JMP Vx != Vy
 */
void Chip8::op9xxx(int opcode) {
  uint8_t x = (opcode & 0x0f00) >> 8;
  uint8_t y = (opcode & 0x00f0) >> 4;

  pc = (regs[x] != regs[y]) ? pc + 2 : pc;
}

/**
 * 0xAnnn: I = nnn
 */
void Chip8::opAxxx(int opcode) {
  uint16_t nnn = opcode & 0x0fff;
  reg_i = nnn;
}

/**
 * 0xBnnn: PC = V0 + nnn
 */
void Chip8::opBxxx(int opcode) {
  uint16_t nnn = opcode & 0x0fff;
  pc = regs[0] + nnn;
}

/**
 * 0xCxnn: Vx = rand() & nnn
 */
void Chip8::opCxxx(int opcode) {
  uint8_t nn = opcode & 0x00ff;
  uint8_t x = (opcode & 0x0f00) >> 8;

  regs[x] = rand() & nn;
}

/**
 * 0xDxyn: draw(x, y, n)
 */
void Chip8::opDxxx(int opcode) {
  uint8_t x = (opcode & 0x0f00) >> 8;
  uint8_t y = (opcode & 0x00f0) >> 4;
  uint8_t n = (opcode & 0x000f);

  int rx = regs[x] % 64;
  int ry = regs[y] % 32;

  regs[0xf] = 0;

  for (size_t row = 0; row < n; row++) {
    uint8_t sprite_pixel = memory[reg_i + row];

    for (size_t col = 0; col < 8; col++) {

      int bit = (sprite_pixel & (0x80 >> col));
      int location = (row + ry) * 64 + rx + col;

      if (bit) {
        if (video_memory[location]) regs[0xf] = 1;
        video_memory[location] ^= 1;
      }
    }
  }
  refresh_display = true;
}

/**
 * 0xEx9E: SKP key() == Vx
 * 0xExA1: SKP key() != Vx
 */
void Chip8::opExxx(int opcode) {
  uint8_t last_byte = opcode & 0xff;
  uint8_t x = (opcode & 0x0f00) >> 8;

  if (last_byte == 0x9e) {
    if (keys[regs[x]]) pc += 2;
  } else if (last_byte == 0xa1) {
    if (!keys[regs[x]]) pc += 2;
  } else {
    unimplemented_opcode(opcode);
  }
}

/**
 * 0xFx07: Vx = get_delay()
 * 0xFx0A: Vx = get_key()
 * 0xFx15: Set delay_timer(Vx)
 * 0xFx18: Set sound_timer(Vx)
 * 0xFx1E: I += Vx
 * 0xFx29: I = sprite_addr[Vx]
 * 0xFx33: set_BCD(Vx)
 * 0xFx55: reg_dump(Vx, &I)
 * 0xFx65: reg_load(Vx, &I)
 */
void Chip8::opFxxx(int opcode) {
  uint8_t last_byte = opcode & 0xff;
  uint8_t x = (opcode & 0x0f00) >> 8;

  if (last_byte == 0x07) {
    regs[x] = delay_timer;
  } else if (last_byte == 0x0a) {
    waiting_for_key_press = true;
    wait_key = x;
  } else if (last_byte == 0x15) {
    delay_timer = regs[x];
  } else if (last_byte == 0x18) {
    sound_timer = regs[x];
  } else if (last_byte == 0x1e) {
    reg_i += regs[x];
  } else if (last_byte == 0x29) {
    uint8_t digit = regs[x];
    reg_i = (FONT_START + (5 * digit));
  } else if (last_byte == 0x33) {
    uint8_t digit = regs[x];
    memory[reg_i] = digit / 100;
    memory[reg_i + 1] = (digit / 10) % 10;
    memory[reg_i + 2] = digit % 10;
  } else if (last_byte == 0x55) {
    for (int i = 0; i <= x; i++)  memory[reg_i + i] = regs[i];
  } else if (last_byte == 0x65) {
    for (int i = 0; i <= x; i++)  regs[i] = memory[reg_i + i];
  } else {
    unimplemented_opcode(opcode);
  }
}

void Chip8::unimplemented_opcode(int opcode) {
  std::cerr << "Unimplemented opcode\n";
  std::cerr << "pc: " << std::hex << pc - 2
    << std::setfill('0') << std::setw(4)
    << " - " << std::hex << opcode << "\n";
}
