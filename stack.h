#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint16_t* data;
    int top;
    int capacity;
} Stack;

void stack_init(Stack* s, unsigned int capacity);
void stack_free(Stack* s);
void stack_push(Stack* s, uint16_t element);
uint16_t stack_peak(Stack* s);
uint16_t stack_pop(Stack* s);

#endif
