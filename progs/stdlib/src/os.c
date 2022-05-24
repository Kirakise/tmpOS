#include "os.h"
#include "stdio.h"
#include <stdbool.h>

int getkey_block(){
  int val;
  do
    val = os_getkey();
  while (val == 0);
  return val;
}

void terminal_readline(char *out, int max, bool out_while_type){
  int i = 0;
  for (i = 0; i < max - 1; i++){
    char key = getkey_block();
    if (key == 13)
      break;
    if (out_while_type)
      os_putchar(key);
    if (key == 0x08 && i >= 1){
      out[i-1] = 0;
      i -= 2;
      continue;
    }
    out[i] = key;
  }
  out[i] = 0;
}
