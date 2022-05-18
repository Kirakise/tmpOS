#include "keyboard.h"
#include "PS2.h"
#include <stdint.h>
#include <stddef.h>
#include "../io/io.h"

int PS2_keyboard_init();


static uint8_t keyboard_scan_table[] = {
    0x00, 0x1B, '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    0x08, '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    0x0d, 0x00, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`', 
    0x00, '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.', '/', 0x00, '*',
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.'
};

struct keyboard PS2_keyboard = {
  .name = {"PS2"},
  .init = PS2_keyboard_init
};

int PS2_keyboard_init(){
  outb(PS2_PORT_COMMAND, PS2_COMMAND_ENABLE_FIRST_PORT); //enable the first PS2 port
  return 0;
}

uint8_t PS2_keyboard_scan_to_char(uint8_t scan){
  size_t size_of_keyboard_scan_table = sizeof(keyboard_scan_table)/sizeof(uint8_t);
  if (scan > size_of_keyboard_scan_table)
    return 0;
  char c = keyboard_scan_table[scan];
  //Probably some caps and shift shit needed
  return c;
}

void PS2_keyboard_handle_interrupt(){

}

struct keyboard *PS2_init(){
  return &PS2_keyboard;
}
