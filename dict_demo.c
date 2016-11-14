// Demonstrate loading of a dictionary, retrieval of a word.

#include <stdio.h>
#include <stdlib.h>
#include "crack.h"

int main(int argc, char **argv){
  if(argc < 3){
    printf("usage: %s <dictfile> [wordnum1] [wordnum2]...\n",argv[0]);
    printf("  <dictfile> : file name of a dictionary (newline separated words)\n");
    printf("  [wordnum1] : integers, words to print from the dictionary\n");
    return 0;
  }

  dict_t *dict = dict_load(argv[1]);
  printf("%d words loaded\n",dict_get_word_count(dict));
  for(int i=2; i<argc; i++){
    int word_num = atoi(argv[i]);
    char *word = dict_get_word(dict,word_num);
    printf("word %d: %s\n",word_num,word);
  }

  dict_free(dict);
  return 0;
}
                          
