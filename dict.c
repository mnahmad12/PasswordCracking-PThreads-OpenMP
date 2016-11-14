// Dictionary of words utilities. Allows easy reading of an entire
// file of words. Used for dictionaries of possibilities and for
// password file contents.

#include <stdlib.h>
#include <stdio.h>
#include <crack.h>


// Calculate the total characters, total lines, and longest line
// length in the given file. Presumes the file is open and position at
// a desired position.  Reads to the end of the file and does not
// reset position after scanning.
void file_sizes(FILE* file, int *total_chars, int *total_lines, int *longest_line_len){
  int tc=0, tl=0, ll=0;
  int cur_len=0;
  for(int c=fgetc(file); c != EOF; c=fgetc(file)){
    tc++;
    cur_len++;
    if(c == '\n'){
      tl++;
      if(cur_len > ll){
        ll = cur_len;
      }
    }
  }
  *total_chars = tc;
  *total_lines = tl;
  *longest_line_len = ll;
  
  return;
}

// Load a dictionary from a named file
dict_t *dict_load(char *fname){
  FILE* file = fopen(fname,"r");
  if(file==NULL){
    perror(fname);
    exit(1);
  }
  int total_chars, total_lines, longest_line_len;
  file_sizes(file, &total_chars, &total_lines, &longest_line_len);
  rewind(file);

  dict_t *dict = malloc(sizeof(dict_t));
  dict->word_count = total_lines;
  dict->total_length = total_chars;
  dict->longest_word_length = longest_line_len;
  dict->data = (char *) malloc(dict->total_length * sizeof(char));
  dict->offsets = (int *) malloc(dict->word_count * sizeof(int));

  int word_idx = 0;
  dict->offsets[word_idx] = 0;
  for(int c=fgetc(file),i=0; c != EOF; c=fgetc(file),i++){
    dict->data[i] = c;
    if(c == '\n'){
      dict->data[i] = '\0';
      word_idx++;
      if(word_idx < dict->word_count){
        dict->offsets[word_idx] = i+1;
      }
    }
  }
  fclose(file);
  return dict;
}

// dict_t *dict_load(char *fname){
//   FILE* file = fopen(fname,"r");
//   dict_t *dict = malloc(sizeof(dict_t));
//   dict->word_count = 0;
//   dict->total_length = 0;
//   for(int c=fgetc(file); c != EOF; c=fgetc(file)){
//     dict->total_length++;
//     if(c == '\n'){
//       dict->word_count++;
//     }
//   }
//   rewind(file);
//   dict->data = (char *) malloc(dict->total_length * sizeof(char));
//   dict->offsets = (int *) malloc(dict->word_count * sizeof(int));
//   int word_idx = 0;
//   dict->offsets[word_idx] = 0;
//   for(int c=fgetc(file),i=0; c != EOF; c=fgetc(file),i++){
//     dict->data[i] = c;
//     if(c == '\n'){
//       dict->data[i] = '\0';
//       word_idx++;
//       if(word_idx < dict->word_count){
//         dict->offsets[word_idx] = i+1;
//       }
//     }
//   }
//   fclose(file);
//   return dict;
// }

void dict_free(dict_t *dict){
  free(dict->data);
  free(dict->offsets);
  free(dict);
}

// Return the number of words in the dictionary
int dict_get_word_count(dict_t *dict){
  return dict->word_count;
}
  
// Return the length of the longest word
int dict_get_longest_word_length(dict_t *dict){
  return dict->longest_word_length;
}

// Return the ith word in the dictionary
char *dict_get_word(dict_t *dict, int i){
  if(i >= dict->word_count){
    fprintf(stderr,"WARNING: %d out of bounds in dict with %d words\n",
            i,dict->word_count);
  }
  return dict->data + dict->offsets[i];
}


// Load an array of dicts based on an array of file names
// given. Efficiently handles repeated files by creating shallow
// references.
dict_t **dict_load_dicts(char **fnames, int dicts_len){
  dict_t **dicts = malloc(dicts_len * sizeof(dict_t*));
  for(int i=0; i<dicts_len; i++){
    dicts[i] = NULL;

    // search back to see if dict is previously loaded
    for(int j=0; j<i; j++){
      if(strcmp(fnames[i],fnames[j])==0){
        dicts[i] = dicts[j];
        break;
      }
    }

    // New dictionary, load it
    if(dicts[i] == NULL){
      dicts[i] = dict_load(fnames[i]);
    }
  }
  return dicts;
}

// Free up an array of dict points; may contain shallow references so
// check and avoid double-frees
void dict_free_dicts(dict_t **dicts, int dicts_len){
  for(int i=0; i<dicts_len; i++){
    for(int j=i+1; j<dicts_len; j++){
      if(dicts[i] == dicts[j]){
        // Free later
        dicts[i] = NULL;
        break;
      }
    }
    // Unique reference, free it
    if(dicts[i] != NULL){
      dict_free(dicts[i]);
    }
  }
  free(dicts);
  return;
}
