#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uint16_t *data;
  int top;
  int capacity;
} Stack;

void stack_init(Stack *s, unsigned int capacity) {
  s->data = malloc(capacity * sizeof(uint16_t));
  if (!s->data) {
    printf("%s\n", "Failed to allocate memory for stack. Exiting.");
    exit(-1);
  }
  s->capacity = capacity;
  s->top = 0;
}

void stack_free(Stack *s) {
  free(s->data);
  s->data = NULL;
}

void stack_push(Stack *s, uint16_t element) {
  if (s->top == s->capacity) {
    printf("%s\n", "Stack is full. Exiting.");
    exit(-1);
  }
  s->data[s->top + 1] = element;
  s->top++;
}

uint16_t stack_peak(Stack *s) {
  if (s->top < 0) {
    printf("%s\n", "Stack is empty. Exiting.");
    exit(-1);
  }
  return s->data[s->top];
}

uint16_t stack_pop(Stack *s) {
  if (s->top < 0) {
    printf("%s\n", "Stack is empty. Exiting.");
    exit(-1);
  }
  uint16_t element = s->data[s->top];
  s->top--;
  return element;
}
