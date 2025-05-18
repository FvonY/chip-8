#define BUFFER_SIZE 8
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "stack.h"

int main() {
	printf("%s\n", "Chip-8 Emulator");
	
	// 4096 bytes of RAM
	unsigned char* mem = malloc(4096 * sizeof(char));
	
	// Program Counter
	unsigned int pc = 0;
	
	// index register
	uint16_t I = 0;

	// stack for 16 bit adresses
	Stack *stack = malloc(sizeof(Stack));
	// 8bit delay timer
	uint8_t delay_timer = 0;

	// 8bit sound timer
	uint8_t sound_timer = 0;

	// 16x8bit general purpose variable register V0-VF
	unsigned char v0 = 0x00;
	unsigned char v1 = 0x00;
	unsigned char v2 = 0x00;
	unsigned char v3 = 0x00;
	unsigned char v4 = 0x00;
	unsigned char v5 = 0x00;
	unsigned char v6 = 0x00;
	unsigned char v7 = 0x00;
	unsigned char v8 = 0x00;
	unsigned char v9 = 0x00;
	unsigned char vA = 0x00;
	unsigned char vB = 0x00;
	unsigned char vC = 0x00;
	unsigned char vD = 0x00;
	unsigned char vE = 0x00;
	unsigned char vF = 0x00;
}

