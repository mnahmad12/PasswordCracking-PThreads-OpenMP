#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crack.h>

// Work horse function to try all dictionary possibilities against a
// given encrypted password.  The array of dicts are used in turn to
// generate all combinations of words from each dictionary.  The
// method recursively descends increasing the dictionary position at
// each step down. Each level of recursion runs through its own loop
// to check all possibilities in a single dictionary.  Each layer
// appends a word onto the provided buf[] array before descending
// another layer.  When the bottom layer of recursion is reached,
// buf[] contains a complete plaintext password which is checked
// agains the target encrypted password.
//
// Returns 1 on locating a plaintext password that matches which is
// left in the buf parameter (null terminated).
//
// Returns 0 if unable to locate a matching plaintext password.
int try_crack(char *target,
              dict_t **dicts, int dicts_len, int dict_pos,
              char *buf, int buflen, int bufpos)
{

  // Base case: buf contains a word from each dictionary in the
  // array. Check the current possibility for success.
  if(dict_pos == dicts_len){     
    //printf("%s\n",buf);
    int success = check_password(target, buf);
    
    return success;
  }

  // Not at base case yet, loop through another layer of the
  // dictionaries appending each word to the end of buf the descending
  // to the next dictionary.
  dict_t *cur_dict = dicts[dict_pos];
  for(int i=0; i<dict_get_word_count(cur_dict); i++){
    // Append a new word to the end of buf
    char *word = dict_get_word(cur_dict, i);
    
    int bp = bufpos + strlen(word);
    if(bp > buflen){
      fprintf(stderr,"WARNING: Buffer capacity exceeded: buflen= %d buflim= %d\n",
              buflen, bp);
    }
    strncpy((buf)+bufpos, word, (buflen-bufpos));
   
    // Descend another layer
    int success = try_crack(target,
                            dicts, dicts_len, dict_pos+1,
                            buf, buflen, bp);

    if(success==1){             // Check for success
      return 1;
    }
    // No match, next iteration overwrites previous word
  }

  // Tried all dictionary words at this level with no luck
  return 0;
}

// Check whether a plaintext password hashes to the target encrypted
// password by encrypting it and comparing strings. Uses the
// md5crypt_r function. Returns 1 if the passwords match and 0
// otherwise.
int check_password(char *target, char *plain){
  
  char crypt[MD5CRYPT_SIZE];
  md5crypt_r(plain, "1", "",crypt);
  int diff = strcmp(target, crypt);
  
  #ifdef DEBUG
  printf("Check: %4d: %s ?= %s <-- %s\n",diff,target,crypt,plain);
  #endif

  return !diff;
}
