#include "../stdlib/src/os.h"
#include "../stdlib/src/stdlib.h"

int main(int argc, char **argv){
  print("HENLO\n");
  char *str = malloc(1024);
  while(1){
    terminal_readline(str, 1024, true);
    str[3] = '#';
    print(str);
  } 
  free(str);
  return 0;
}
