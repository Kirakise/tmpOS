#include "../stdlib/src/os.h"
#include "../stdlib/src/stdlib.h"

int main(int argc, char **argv){
  print("HENLO\n");
  char *str = malloc(12);
  free(str);
  while(1){
    if (getkey() != 0){
      print("key was pressed\n");
    }
  };
  return 0;
}
