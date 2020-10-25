#include <SDL2/SDL.h>

#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

#include "chip8.h"
#include "utils.h"

/* Mapping from keyboard keys to chip8 keys */
static std::unordered_map<SDL_Keycode, int8_t> key_map = 
{
  { SDLK_x, 0 },
  { SDLK_1, 1 },
  { SDLK_2, 2 },
  { SDLK_3, 3 },
  { SDLK_q, 4 },
  { SDLK_w, 5 },
  { SDLK_e, 6 },
  { SDLK_a, 7 },
  { SDLK_s, 8 },
  { SDLK_d, 9 },
  { SDLK_z, 10 },
  { SDLK_c, 11 },
  { SDLK_4, 12 },
  { SDLK_r, 13 },
  { SDLK_f, 14 },
  { SDLK_v, 15 }
};

int main(int argc, const char *argv[]) {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Event event;
  int scale = 16;

  uint32_t win_width = scale * CHIP8_WIDTH;
  uint32_t win_height = scale * CHIP8_HEIGHT;
  std::vector<uint8_t> rom;
  uint32_t pixels[2048];
  bool run = true;
  bool released = false;
  int8_t key = -1;

  if (argc < 2) {
    std::cout << "Usage: ./chip8 ROM\n";
    return -1;
  }

  /* Load ROM */
  load_rom(std::filesystem::path{argv[1]}, rom);

  /* Initialize SDL */
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    std::cout << "SDL: Error initializing " << SDL_GetError() << std::endl;
    exit(-1);
  }

  window = SDL_CreateWindow("Chip8 Emulator", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, win_width,
                            win_height, SDL_WINDOW_SHOWN);
  if (!window) {
    std::cout << "SDL: Error creating window " << SDL_GetError() << std::endl;
    exit(-1);
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_RenderSetLogicalSize(renderer, win_width, win_height);

  // Create texture that stores frame buffer
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING,
                              CHIP8_WIDTH, CHIP8_HEIGHT);

  Chip8 chip8(rom);

  while (run) {
    released = false;
    key = -1;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          run = false;
          break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
          if (key_map.find(event.key.keysym.sym) != key_map.end())
            key = key_map.find(event.key.keysym.sym)->second;
          released = (event.type == SDL_KEYUP) ? true : false;
          break;
      }
    }

    if (chip8.set_input(key, released)) continue;

    chip8.step();

    if (chip8.draw_screen(pixels)) {
      SDL_UpdateTexture(texture, NULL, pixels, CHIP8_WIDTH * sizeof(uint32_t));
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, texture, NULL, NULL);
      SDL_RenderPresent(renderer);
    }
    
    chip8.update_timer();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyTexture(texture);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

