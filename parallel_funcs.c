#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crack.h>
#include <omp.h>
#include <pthread.h>

// Add parallel functions here


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


struct PThreadParams{
  int id;
  int start;
  int stop;
  dict_t* cur_dict;
  int bufpos;
  int buflen;
  int* finalSuccess;
  int dict_lens;
  int dict_pos;
  char* buf;
  char *target;
  dict_t **dicts;
};

void *pthreadCrackPass(void *params)
{
  //getting all values:
  struct PThreadParams *pthreadparams=(struct PThreadParams*)params;
  int *finalSuccess=(*pthreadparams).finalSuccess;
  dict_t* cur_dict=(*pthreadparams).cur_dict;
  int bufpos=(*pthreadparams).bufpos;;
  int buflen=(*pthreadparams).buflen;
  char *target=(*pthreadparams).target;
  dict_t **dicts=(*pthreadparams).dicts;
  int dicts_len=(*pthreadparams).dict_lens;
  int dict_pos=(*pthreadparams).dict_pos;
  char* buf=(*pthreadparams).buf;
  int id=(*pthreadparams).id;
  int start=(*pthreadparams).start;
  int stop=(*pthreadparams).stop;
  //how to split the for loop

 for(int i=start; i<stop; i++){
   // printf("%d\n",i);
   if(*finalSuccess==1)             // Check for success      
	 {
	   continue;
	 }
  
   //printf("I am thread: %d, and I am inspecting iteration: %d\n",id,i);
       char *mybuf= malloc(buflen * sizeof(char));
       
       // Append a new word to the end of buf
       
       char *word = dict_get_word(cur_dict,i);
       //printf("I am thread: %d and the word I am looking at is: %s\n",omp_get_thread_num(),word);
       int bp = bufpos + strlen(word);
       if(bp > buflen){
	 fprintf(stderr,"WARNING: Buffer capacity exceeded: buflen= %d buflim= %d\n",
		 buflen, bp);
       }
       strncpy((mybuf)+bufpos, word, (buflen-bufpos));
       
       //printf("I am thread: %d, %s\n",id,buf);
       // Descend another layer
       int success = try_crack(target,
			       dicts, dicts_len, dict_pos+1,
			       mybuf, buflen, bp);
       if(success==1)
	 {
	   strncpy(buf,mybuf,buflen);
	   *finalSuccess=1;
	 }
    
 }
  

 return NULL;
}


int pthread_try_crack(char *target,
              dict_t **dicts, int dicts_len, int dict_pos,
              char *buf, int buflen, int bufpos)
{

  // Check environment variable for number of threads
  int nthreads = 4;
  char *nthreads_str = getenv("PASSCRACK_NUMTHREADS");
  if(nthreads_str != NULL){
    nthreads = atoi(nthreads_str);
  }


  
  
  // Base case: buf contains a word from each dictionary in the
  // array. Check the current possibility for success.
  if(dict_pos == dicts_len){     
    int success = check_password(target, buf);
    return success;
  }

  // Not at base case yet, loop through another layer of the
  // dictionaries appending each word to the end of buf the descending
  // to the next dictionary.
  
  //setting up parameters:
  int finSucc=0;
  int *finalSuccess=&finSucc;  
  dict_t *cur_dict = dicts[dict_pos];
  struct PThreadParams *pThreadParams;
  pThreadParams=malloc(nthreads*sizeof(struct PThreadParams));
  
  int length=dict_get_word_count(cur_dict);
  int threadPortion=length/nthreads;
  //printf("Launching Threads\n");
  int i;
  pthread_t threads[nthreads];
  for(i=0;i<nthreads;i++)
    {
      int start=i*threadPortion;
      int stop=(i==nthreads-1) ? length : (start+threadPortion);
      pThreadParams[i].start=start;
      pThreadParams[i].stop=stop;
      pThreadParams[i].id=i;
      pThreadParams[i].cur_dict=cur_dict;
      pThreadParams[i].finalSuccess=finalSuccess;
      pThreadParams[i].target=target;
      pThreadParams[i].bufpos=bufpos;
      pThreadParams[i].buf=buf;
      pThreadParams[i].dicts=dicts;
      pThreadParams[i].buflen=buflen;
      pThreadParams[i].dict_lens=dicts_len;
      pThreadParams[i].dict_pos=dict_pos;
      pthread_create(&threads[i],NULL,pthreadCrackPass,(void*)(&pThreadParams[i]));
    }

  for(i=0;i<nthreads;i++)
    {
      pthread_join(threads[i],(void **) NULL);
    }


  
  
  // No match, next iteration overwrites previous word


  // Tried all dictionary words at this level with no luck
  return *finalSuccess;
}










int omp_try_crack(char *target,
              dict_t **dicts, int dicts_len, int dict_pos,
              char *buf, int buflen, int bufpos)
{

  // Check environment variable for number of threads
  int nthreads = 4;
  char *nthreads_str = getenv("PASSCRACK_NUMTHREADS");
  if(nthreads_str != NULL){
    nthreads = atoi(nthreads_str);
  }
  omp_set_num_threads(nthreads);

  //printf("IN OMP_TRY_CRACK\n");
  int finalSuccess=0;
  // Base case: buf contains a word from each dictionary in the
  // array. Check the current possibility for success.
  if(dict_pos == dicts_len){     
    int success = check_password(target, buf);
    return success;
  }

  // Not at base case yet, loop through another layer of the
  // dictionaries appending each word to the end of buf the descending
  // to the next dictionary.
  
  dict_t *cur_dict = dicts[dict_pos];
  #pragma omp parallel for
  for(int i=0; i<dict_get_word_count(cur_dict); i++){
    //printf("I am thread: %d and I am doing iteration: %d\n",omp_get_thread_num(),i);
    char *mybuf= malloc(buflen * sizeof(char));
    if(finalSuccess==1)             // Check for success      
      {
	continue;
      }
    // Append a new word to the end of buf
    char *word = dict_get_word(cur_dict, i);
    //printf("I am thread: %d and the word I am looking at is: %s\n",omp_get_thread_num(),word);
    int bp = bufpos + strlen(word);
    if(bp > buflen){
      fprintf(stderr,"WARNING: Buffer capacity exceeded: buflen= %d buflim= %d\n",
              buflen, bp);
    }
    strncpy((mybuf)+bufpos, word, (buflen-bufpos));
    
    //printf("%s\n",buf);
    // Descend another layer
    int success = try_crack(target,
                            dicts, dicts_len, dict_pos+1,
                            mybuf, buflen, bp);
    if(success==1)
      {
	strncpy(buf,mybuf,buflen);
	//printf("FOUND\n");
	finalSuccess=1;
      }
    
  }
    // No match, next iteration overwrites previous word


  // Tried all dictionary words at this level with no luck
  return finalSuccess;
}

