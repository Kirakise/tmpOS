#pragma once


#define PS2_PORT_COMMAND 0x64
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE
#define PS2_KEYBOARD_KEY_RELEASED 0x80
#define ISR_KEYBOARD_INTERRUPT 0x21
#define KEYBOARD_INPUT_PORT 0x60

struct keyboard *PS2_init();
void PS2_keyboard_handle_interrupt();
