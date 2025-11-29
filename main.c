#include "stack.h"
#define RAM_SIZE 4096
#define STACK_SIZE 32

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "chip8machine.h"

#define TRUE (1 == 1)
#define FALSE (1 != 1)

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

void load_rom(Chip8* chip8) {
    unsigned int const addr = 0x200;

    FILE* f = fopen("ibm_logo.ch8", "rb");

    if (!f) {
        printf("%s\n", "ROM file could not be opened. Quitting.");
        exit(-1);
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    fread(chip8->mem + addr, 1, size, f);
    chip8->pc = addr;

    fclose(f);
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

void clear_screen(Chip8* chip8) {
    for (unsigned int i = 0; i < DISPLAY_SIZE; i++) {
        chip8->display_buffer[i] = 0;
    }
}

void display(Chip8* chip8) {
    system("clear");
    printf("%s", "  +");
    for (int i = 0; i < DISPLAY_X; i++)
        putchar('-');
    printf("%s\n", "+");
    for (unsigned int row = 0; row < DISPLAY_Y; row++) {
        printf("%2d|", row);
        for (unsigned int col = 0; col < DISPLAY_X; col++) {
            putchar(chip8->display_buffer[row * DISPLAY_X + col] ? '#' : ' ');
        }

        printf("%s\n", "|");
    }

    printf("%s", "  +");
    for (int i = 0; i < DISPLAY_X; i++)
        putchar('-');
    printf("%s\n", "+");
}

unsigned char set_pixel(Chip8* chip8,
                        uint16_t display_offset,
                        unsigned char sprite_pixel) {
    unsigned char current_pixel = chip8->display_buffer[display_offset];
    unsigned char updated_pixel = 0;

    if (current_pixel == 1 && sprite_pixel == 1) {
        updated_pixel = 0;
        set_register(chip8, 0xF, 1);
    } else if (current_pixel == 0 && sprite_pixel == 1) {
        updated_pixel = 1;
    }

    chip8->display_buffer[display_offset] = updated_pixel;

    return TRUE;
}

void draw_sprite(Chip8* chip8, uint8_t x, uint8_t y, uint8_t n) {
    uint8_t loc_x = chip8->v[x];
    uint8_t loc_y = chip8->v[y];

    loc_x = loc_x % DISPLAY_X;
    loc_y = loc_y % DISPLAY_Y;

    set_register(chip8, 0xF, 0);

    for (unsigned int row = 0; row < n; row++) {
        if (loc_y + row >= DISPLAY_Y)
            break;
        unsigned char sprite_byte = read_memory(chip8, chip8->I + row);
        for (unsigned int b = 0; b < 8; b++) {
            // each bit in a byte is a pixel of the row in the sprite
            uint16_t display_offset = (loc_y + row) * DISPLAY_X + loc_x + b;
            if (loc_x + b >= DISPLAY_X)
                break;
            unsigned char sprite_pixel = (sprite_byte >> (7 - b)) & 1;

            set_pixel(chip8, display_offset, sprite_pixel);
        }
    }
}

void decode(uint16_t instruction, Chip8* chip8) {
    uint8_t w = (instruction & 0xF000) >> 12;
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;
    uint8_t n = instruction & 0x000F;
    unsigned char nn = instruction & 0x00FF;
    uint16_t nnn = instruction & 0x0FFF;

    if (instruction == 0x00E0) {
        // Clear Screen
        clear_screen(chip8);
    }

    switch (w) {
        case 0x0:
            // 0NNN: Skip
            break;
        case 0x1:
            // 1NNN: Unconditional Jump to NNN
            chip8->pc = nnn;
            break;
        case 0x2:
            // 2NNN: Call Subroutine at NNN
            stack_push(&(chip8->stack), chip8->pc);
            chip8->pc = nnn;
            break;
        case 0x3:
            // 3XNN: Conditional Skip if VX==NN
            if (read_register(chip8, x) == nn) {
                chip8->pc++;
                chip8->pc++;
            }
            break;
        case 0x4:
            // 4XNN: Conditional Skip if VX!=NN
            if (read_register(chip8, x) != nn) {
                chip8->pc++;
                chip8->pc++;
            }
            break;
        case 0x5:
            // 5XY0: Contional Skip if VX==VY
            if (read_register(chip8, x) == read_register(chip8, y)) {
                chip8->pc++;
                chip8->pc++;
            }
            break;
        case 0x6:
            // set vx
            set_register(chip8, x, nn);
            break;
        case 0x7:
            // add nn to x
            add_to_register(chip8, x, nn);
            break;
        case 0x9:
            // 9XY0: Contional Skip if VX!=VY
            if (read_register(chip8, x) != read_register(chip8, y)) {
                chip8->pc++;
                chip8->pc++;
            }
            break;
        case 0xA:
            // set index register
            chip8->I = nnn;
            break;
        case 0xD:
            // draw DXYN
            draw_sprite(chip8, x, y, n);
            break;
        default:
            printf("Unhandled instruction: %x.\n", instruction);
            getchar();
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
        0xF0, 0x90, 0xF0, 0x90, 0x90,  // Atrue
        0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
        0xF0, 0x80, 0xF0, 0x80, 0x80   // F
    };
    unsigned char* font = fontset;
    write_memory(chip8, addr, font, 80);
}

int main() {
    printf("%s\n", "Chip-8 Emulator");
    // init
    Chip8* chip8 = malloc(sizeof(Chip8));
    stack_init(&(chip8->stack), STACK_SIZE);
    store_font(chip8, 0x50);

    load_rom(chip8);
    int counter = 0;

    while (TRUE) {
        unsigned int instr = fetch(chip8);
        printf("%3d Instruction: %4x\n", counter++, instr);
        getchar();
        decode(instr, chip8);
        display(chip8);
    }

    free(chip8);
}
