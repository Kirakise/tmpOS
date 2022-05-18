#pragma once


typedef int (*KEYBOARD_INIT_FUNCTION)();

#define KEYBOARD_BUFFER_SIZE 1024

struct keyboard{
  KEYBOARD_INIT_FUNCTION init;
  char name[20];
  struct keyboard *next;
};


void keyboard_init();
char keyboard_pop();
void keyboard_push(char c);
void keyboard_backspace();
