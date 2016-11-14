// Header file for passcrack functions

#ifndef CRACK_H
#define CRACK_H

// Uncomment to print out every password as it is checked
// #define DEBUG 

// dict.c
typedef struct {
  char *data;
  int *offsets;
  int word_count;
  int total_length;
  int longest_word_length;
} dict_t;

void file_sizes(FILE* file, int *total_chars, int *total_lines, int *longest_line_len);
dict_t *dict_load(char *fname);
void dict_free(dict_t *dict);
int dict_word_count(dict_t *dict);
int dict_get_word_count(dict_t *dict);
int dict_get_longest_word_length(dict_t *dict);
char *dict_get_word(dict_t *dict, int i);
dict_t **dict_load_dicts(char **fnames, int dicts_len);
void dict_free_dicts(dict_t **dicts, int dicts_len);

// md5crypt_r.c
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/des.h>

#define MD5CRYPT_SIZE (6 + 9 + 24 + 2)

int md5crypt_r(const char *passwd, const char *magic, const char *salt, char *out_buf);

// crack_funcs.c
int check_password(char *target, char *plain);

int try_crack(char *target,
              dict_t **dicts, int dicts_len, int dict_pos,
              char *buf, int buflen, int bufpos);

int omp_try_crack(char *target,
              dict_t **dicts, int dicts_len, int dict_pos,
		  char *buf, int buflen, int bufpos);


// parallel_funcs.c

#endif
