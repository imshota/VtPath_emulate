#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]){
  static struct {
    char pas[3];
    int mode;
  } input;
  input.mode = 0;

  if(argc != 2){
    printf("Usage: %s\n", argv[0]);
    return 1;
  }
  strcpy(input.pas, argv[1]);

  if (!strcmp(input.pas, "VP")) {
    printf("Secret mode\n");
    input.mode = 1;
  } else {
    printf("Normal mode\n");
  }
  if (input.mode != 0) {
    printf("You are so cute\n");
  } else {
    printf("You are so nice\n");
  }
  return 0;
}