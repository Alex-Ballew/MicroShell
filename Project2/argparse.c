#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "argparse.h"
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define FALSE (0)
#define TRUE  (1)

/*
* argCount is a helper function that takes in a String and returns the number of "words" in the string assuming that whitespace is the only possible delimeter.
*/
static int argCount(char*line) {
 //write your code
  int args = 1;

  for(int i = 0; i < strlen(line); i++){
    if(isspace(line[i])){  // prob should error handle for ending in white space and multiple space like "mv  file1" (wouldn't work for this)
      args++;
    }
  }
  return args;
}



/*
*
* Argparse takes in a String and returns an array of strings from the input.
* The arguments in the String are broken up by whitespaces in the string.
* The count of how many arguments there are is saved in the argcp pointer
*/
char** argparse(char* line, int* argcp) {
  *argcp = argCount(line);
  char** args = calloc(strlen(line) + 1, strlen(line) + 1);  // need to malloc fpr line not (*argcp) not sure why
  //args[0] = calloc(strlen(line), strlen(line));   // proably shouldnt do this, change later to reduce calloced space(prob just make a methd which finds length until white space)
  int argloc = 0;

  for(int i = 0; i < *argcp; i++){
    args[i] = calloc(strlen(line), strlen(line));
  }

  for(int i = 0; i < strlen(line); i++){
    char c = line[i];
    if(isspace(line[i])){
      argloc++;
    } else {
      strncat(args[argloc], &c, 1);
    }
  }
  return args;
}

