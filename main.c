#define RAM_SIZE 4096

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "chip8machine.h"

unsigned char read_memory(Chip8* chip8, unsigned int addr) {
    if (addr >= RAM_SIZE) {
        printf("%s\n", "Memory access out of bounds. Exiting.");
        exit(-1);
    }
    return chip8->mem[addr];
}

void write_memory(Chip8* chip8,
                  unsigned int addr,
                  unsigned char* bytes,
                  unsigned int num_bytes) {
    if (addr + num_bytes >= RAM_SIZE) {
        printf("%s\n", "Trying to write outside of RAM. Exiting.");
        exit(-1);
    }

    for (unsigned int i = 0; i < num_bytes; i++) {
        chip8->mem[addr + i] = bytes[i];
    }
}

void set_register(Chip8* chip8, uint8_t x, uint16_t nn) {
    chip8->v[x] = nn;
}

unsigned char read_register(Chip8* chip8, uint8_t x) {
    return chip8->v[x];
}

void add_to_register(Chip8* chip8, uint8_t x, uint16_t nn) {
    uint16_t value = read_register(chip8, x);
    set_register(chip8, x, value + nn);
}

uint16_t fetch(Chip8* chip8) {
    unsigned char first_byte = read_memory(chip8, chip8->pc);
    chip8->pc++;
    unsigned char second_byte = read_memory(chip8, chip8->pc);
    chip8->pc++;
    uint16_t instruction = ((uint16_t)first_byte << 8 | second_byte);
    return instruction;
}

void decode(uint16_t instruction, Chip8* chip8) {
    uint8_t w = (instruction & 0xF000) >> 12;
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;
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
        0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
        0x20, 0x60, 0x20, 0x20, 0x70,  // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
        0xF0, 0x80, 0xF0, 0x80, 0x80   // F
    };
    unsigned char* font = fontset;
    write_memory(chip8, addr, font, 80);
}

void display_update() {}

void draw_sprite(Chip8* chip8, uint8_t x, uint8_t y, uint8_t n) {
    uint8_t loc_x = chip8->v[x];
    uint8_t loc_y = chip8->v[y];

    loc_x = loc_x % 64;
    loc_y = loc_y % 32;

    uint16_t sprite_start = chip8->I;

    unsigned char spriteline = read_memory(chip8, n);
}

void draw_line() {}

int main() {
    printf("%s\n", "Chip-8 Emulator");

    Chip8* chip8 = malloc(sizeof(Chip8));

    decode(0x0000, chip8);
    decode(0x0100, chip8);
    decode(0x1023, chip8);
    decode(0x2002, chip8);

    store_font(chip8, 0x50);
    unsigned char test = read_memory(chip8, 0x50);
    printf("%X\n", test);

    free(chip8);
}
