#pragma once


#define PS2_PORT_COMMAND 0x64
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE

struct keyboard *PS2_init();
