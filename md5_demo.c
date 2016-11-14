// Demonstrate the use of md5crypt_r() function to securely hash a
// plaintext given on the command line and display its results.

#include <stdio.h>
#include <crack.h>

int main(int argc, char *argv[]) {
  if(argc < 2) {
    printf("Usage: %s <pass>\n",argv[0]);
    exit(1);
  }

  char *plaintext = argv[1];
  char crypt [MD5CRYPT_SIZE];
  md5crypt_r(plaintext, "1", "",crypt);
  printf("%3d: %s <-- %s\n",0,crypt,plaintext);
  return 0;
}
