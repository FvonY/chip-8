#ifndef chip8
#define chip8

#include "stack.h"
#include <cstdint>
#include <stdint.h>
#define RAM_SIZE 4096

typedef struct {
  unsigned char display_buffer[256];
  unsigned char mem[RAM_SIZE];
  unsigned int pc;
  uint16_t I;
  Stack stack;
  uint8_t delay_timer;
  uint8_t sound_timer;
  unsigned char v[16];
} Chip8;

#endif
