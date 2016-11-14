// Demonstrate use of md5crypt_r function to encrypt all plaintext
// password in a given file named on the command line.
#include <stdlib.h>
#include <stdio.h>
#include <crack.h>

int main(int argc, char **argv){
  if(argc < 2){
    printf("usage: %s <passwdfile>\n",argv[0]);
    printf("  <passwdfile> : file containing passwords separated by lines\n");
    return 0;
  }

  char crypt [MD5CRYPT_SIZE];
  dict_t *passwords = dict_load(argv[1]);
  for(int i=0; i<dict_get_word_count(passwords); i++){
    char *plaintext = dict_get_word(passwords,i);
    md5crypt_r(plaintext, "1", "",crypt);
    printf("%s\n",crypt);
    // printf("%3d: %s <-- %s\n",i,crypt,plaintext);
  }

  dict_free(passwords);

  return 0;
    
}
