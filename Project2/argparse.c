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
  int args = 1;
  int longSpace = 0;
        // account for extra spaces in between
  for (int i = 0; i < strlen(line); i++) { 
    if (isspace(line[i]) == 0) {
      longSpace = 0;
    } 
    if (isspace(line[i]) != 0 && longSpace == 0) {
      args++;
      longSpace = 1;
    }
  }

  if (isspace(line[0]) != 0) {  // accounts for extra spaces at begining
    args--;
  }
  if (isspace(line[strlen(line) - 1]) != 0) {  // accounts for extra spaces at end
    args--;
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
  *argcp = argCount(line);  // assign pointer to argcount
  char** args = calloc(strlen(line) + 1, strlen(line) + 1);
  int argloc = 0;

  for (int i = 0; i < *argcp; i++) {   // allocate space for 2d array
    args[i] = calloc(strlen(line), strlen(line));
  }
   // loop through acounting for spaces
  for (int i = 0; i < strlen(line); i++) {
    char c = line[i];
    if (isspace(line[i]) == 0) {
      strncat(args[argloc], &c, 1);
      if (i + 1 < strlen(line) && isspace(line[i + 1]) != 0) {
        argloc++;
      }
    }
  }
  return args;
}