#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;
int ret;

int f(int arg){
  int mode = arg;    // modeの変数がオーバーフローされ、書き換わる
  char input[5+1];
  char username[5+1];
  char password[5+1];
  FILE *fp;
  
  if ((fp = fopen("passwd.txt", "rb")) == NULL) {
    printf("file open error\n");
    return 0;
  }

  printf("befor input : mode = %d\n", mode);
  // ここでオーバーフローさせ、modeの値を書き換える
  scanf("%s", input);
  printf("after input : mode = %d\n", mode);

  int fcheck;
  if( mode == 0 ) {        // check username
    if ((fcheck = fread(username, 1, 5, fp)) < 5) {
      printf("file read error\n");
      printf("size : %d", fcheck);
      fclose(fp);
      return 0;
    }
    username[5] = '\0';
    fclose(fp);
    if( strcmp(input, username) == 0 ) {
      ret = 1; //有効なユーザである。
    } else {
      ret = 0;
    }
  } else if( mode == 1 ){ // check password
    if ((fcheck = fread(password, 1, 5, fp)) < 5) {
      printf("file read error\n");
      printf("size : %d", fcheck);
      fclose(fp);
      return 0;
    }
    password[5] = '\0';
    //reverse(begin(password), end(password));
    fclose(fp);
    if( strcmp(input, password) == 0 ) {
      ret = 1; //パスワードが一致。
    } else {
      ret = 0;
    }
  }
  return ret; 
}

int main(){ 
  printf( "attack test\n" ); //sys_write()
  printf( "check user\n" );
  ret=f(0);         //(I), read/check username
  if( ret ) {       
    printf( "check pass\n");
    ret = f(1);     //(Ⅱ), read/check password if username was correct
  }

  if( ret ) {
    printf( "Authenticated\n" ); // 認証
  } else {
    printf( "Regreted\n" );  
  }
  return 0;
}