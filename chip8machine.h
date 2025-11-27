#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include "stack.h"
#define RAM_SIZE 4096

typedef struct {
    // Display Buffer
    unsigned char display_buffer[256];
    // RAM
    unsigned char mem[RAM_SIZE];
    // Program Counter
    unsigned int pc;
    // Instruction
    uint16_t I;
    Stack stack;
    uint8_t delay_timer;
    uint8_t sound_timer;
    unsigned char v[16];
} Chip8;

#endif
