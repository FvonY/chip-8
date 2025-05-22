#define RAM_SIZE 4096

#include "chip8machine.h"
#include "stack.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

unsigned char read_memory(Chip8 *chip8, unsigned int addr) {
  if (addr >= RAM_SIZE) {
    printf("%s\n", "Memory access out of bounds. Exiting.");
    exit(-1);
  }
  return chip8->mem[addr];
}

void write_memory(Chip8 *chip8, unsigned int addr, unsigned char *bytes,
                  unsigned int num_bytes) {
  if (addr + num_bytes >= RAM_SIZE) {
    printf("%s\n", "Trying to write outside of RAM. Exiting.");
    exit(-1);
  }

  for (unsigned int i = 0; i < num_bytes; i++) {
    chip8->mem[addr + i] = bytes[i];
  }
}

void set_register(Chip8* chip8, uint8_t x, uint16_t nn) { chip8->v[x] = nn; }

unsigned char read_register(Chip8* chip8, uint8_t x) { return chip8->v[x]; }

void add_to_register(Chip8* chip8, uint8_t x, uint16_t nn) {
  uint16_t value = read_register(chip8, x);
  set_register(chip8, x, value + nn);
}

uint16_t fetch(Chip8 *chip8) {
  unsigned char first_byte = read_memory(chip8, chip8->pc);
  chip8->pc++;
  unsigned char second_byte = read_memory(chip8, chip8->pc);
  chip8->pc++;
  uint16_t instruction = ((uint16_t)first_byte << 8 | second_byte);
  return instruction;
}

void decode(uint16_t instruction, Chip8* chip8) {
  uint8_t w = instruction & 0xF000;
  uint8_t x = instruction & 0x0F00;
  uint8_t y = instruction & 0x00F0;
  uint8_t n = instruction & 0x000F;
  unsigned char nn = instruction & 0x00FF;
  uint16_t nnn = instruction & 0x0FFF;

  unsigned char loc_x;
  unsigned char loc_y;

  if (instruction == 0x00E0) {
    // Clear Screen
  }

  switch (w) {
  case 0x0:
    printf("0x0000\n");
    break;
  case 0x1:
    // JUMP
    chip8->pc = nnn;
    break;
  case 0x6:
    // set vx
    set_register(chip8, x, nn);
    break;
  case 0x7:
    // add nn to x
    add_to_register(chip8, x, nn);
    break;
  case 0xA:
    // set index register
    chip8->I = nnn;
    break;
  case 0xD:
    // draw DXYN
    loc_x = read_memory(chip8, x);
    loc_y = read_memory(chip8, y);
    break;
  default:
    printf("Unhandled instruction: %x.\n", instruction);
    break;
  }
};

void store_font(Chip8* chip8, unsigned int addr) {
  unsigned char fontset[] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };
  unsigned char *font = fontset;
  write_memory(chip8, addr, font, 80);
}

void display_update() {}

void draw_sprite() {}

void draw_line() {}

int main() {
  printf("%s\n", "Chip-8 Emulator");

  Chip8 *chip8 = malloc(sizeof(Chip8));

  // // Display buffer 64 bit x 32 lines (2048bit)
  // // 8 bytes x 32 lines
  // unsigned char *display_buffer = calloc(256, sizeof(unsigned char));
  //
  // // 4096 bytes of RAM
  // unsigned char *mem = malloc(RAM_SIZE * sizeof(unsigned char));
  //
  // // Program Counter
  // unsigned int *pc = malloc(sizeof(unsigned int));
  //
  // // index register
  // uint16_t *I = malloc(sizeof(uint16_t));
  //
  // // stack for 16 bit adresses
  // Stack *stack = malloc(sizeof(Stack));
  //
  // // 8bit delay timer
  // uint8_t delay_timer = 0;
  //
  // // 8bit sound timer
  // uint8_t sound_timer = 0;
  //
  // // 16x8bit general purpose variable register V0-VF
  // unsigned char v0 = 0x00;
  // unsigned char v1 = 0x00;
  // unsigned char v2 = 0x00;
  // unsigned char v3 = 0x00;
  // unsigned char v4 = 0x00;
  // unsigned char v5 = 0x00;
  // unsigned char v6 = 0x00;
  // unsigned char v7 = 0x00;
  // unsigned char v8 = 0x00;
  // unsigned char v9 = 0x00;
  // unsigned char vA = 0x00;
  // unsigned char vB = 0x00;
  // unsigned char vC = 0x00;
  // unsigned char vD = 0x00;
  // unsigned char vE = 0x00;
  // unsigned char vF = 0x00;
  //
  // // as array mayhaps?
  // unsigned char *v = calloc(16, sizeof(unsigned char));

  decode(0x0000, chip8);
  decode(0x0100, chip8);
  decode(0x1023, chip8);
  decode(0x2002, chip8);

  store_font(chip8, 0x50);
  unsigned char test = read_memory(chip8, 0x50);
  printf("%X\n", test);

  free(chip8);
  // free(display_buffer);
  // free(mem);
  // free(pc);
  // free(I);
  // free(stack);
}
