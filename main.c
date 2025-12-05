#include "stack.h"
#define RAM_SIZE 4096
#define STACK_SIZE 32

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

void load_rom(Chip8* chip8,
              const char* rom_file_name,
              const unsigned int addr) {
    FILE* f = NULL;
    if (rom_file_name) {
        f = fopen(rom_file_name, "rb");
    } else {
        f = fopen("ibm_logo.ch8", "rb");
    }

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
    unsigned char value = read_register(chip8, x);
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

void instruction8_handler(uint8_t x, uint8_t y, uint8_t n, Chip8* chip8) {
    unsigned char vx = read_register(chip8, x);
    unsigned char vy = read_register(chip8, y);
    uint16_t result;

    switch (n) {
        case 0x0:
            // Set
            set_register(chip8, x, vy);
            break;
        case 0x1:
            // Binary OR
            set_register(chip8, x, vx | vy);
            break;
        case 0x2:
            // Binary AND
            set_register(chip8, x, vx & vy);
            break;
        case 0x3:
            // Logical XOR
            set_register(chip8, x, vx ^ vy);
            break;
        case 0x4:
            // Add
            result = vx + vy;
            if (result > 255) {
                set_register(chip8, 0xF, 1);
            } else {
                set_register(chip8, 0xF, 0);
            }
            set_register(chip8, 0xF, result > 255 ? 1 : 0);
            set_register(chip8, x, result);
            break;
        case 0x5:
            // Subtract VX-VY
            result = vx - vy;
            set_register(chip8, 0xF, result >= 0 ? 1 : 0);
            set_register(chip8, x, result);
            break;
        case 0x7:
            // Subtract VY-VX
            result = vy - vx;
            set_register(chip8, 0xF, result >= 0 ? 1 : 0);
            set_register(chip8, x, result);
            break;
        case 0x6:
            // VX = (VY >> 1) Right Shift
            {
                const unsigned char shifted_bit = (0x01 & vy);
                set_register(chip8, 0xF, shifted_bit);
                set_register(chip8, x, (vy >> 1));
            }
            break;
        case 0xE:
            // VX = (VY << 1) Left Shift
            {
                const unsigned char shifted_bit = (0x80 & vy) >> 7;
                set_register(chip8, 0xF, shifted_bit);
                set_register(chip8, x, (vy << 1));
            }
            break;
        default:
            printf("Unhandled instruction: 0x8%x%x%x.\n", x, y, n);
            break;
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
    } else if (instruction == 0x00EE) {
        // Return from Subroutine
        chip8->pc = stack_pop(&(chip8->stack));
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
            // 5XY0: Conditional Skip if VX==VY
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
        case 0x8:
            // logic and arithmetic
            instruction8_handler(x, y, n, chip8);
            break;
        case 0x9:
            // 9XY0: Conditional Skip if VX!=VY
            if (read_register(chip8, x) != read_register(chip8, y)) {
                chip8->pc++;
                chip8->pc++;
            }
            break;
        case 0xA:
            // set index register
            chip8->I = nnn;
            break;
        case 0xB:
            // Jump with offset
            {
                const unsigned char v0 = read_register(chip8, 0x0);
                chip8->I = nnn + v0;
            }
            break;
        case 0xD:
            // draw DXYN
            draw_sprite(chip8, x, y, n);
            break;
        default:
            printf("Unhandled instruction: %x.\n", instruction);
            // getchar();
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

Chip8* init_machine() {
    Chip8* chip8 = malloc(sizeof(Chip8));
    stack_init(&(chip8->stack), STACK_SIZE);
    store_font(chip8, 0x50);

    return chip8;
}

unsigned char detect_stuck(unsigned int current_pc) {
    static unsigned int prev_pc = 0xDEADBEEF;

    if (prev_pc == current_pc) {
        // Gibt doch sicher instructions, bei denen man nur den Timer abwartet?
        printf("%s\n", "Program execution stuck.\n");
        return TRUE;
    } else {
        prev_pc = current_pc;

        return FALSE;
    }
}

int main(int argc, char** argv) {
    printf("%s\n", "Chip-8 Emulator");

    // init
    Chip8* chip8 = init_machine();

    char* rom_file_name = NULL;
    if (argc == 2) {
        rom_file_name = argv[1];
    }

    load_rom(chip8, rom_file_name, 0x200);
    int counter = 0;

    while (!detect_stuck(chip8->pc)) {
        unsigned int instr = fetch(chip8);
        printf("%3d Instruction: %4x\n", counter++, instr);
        // getchar();
        decode(instr, chip8);
        display(chip8);
        usleep(500);
    }

    free(chip8);
}
