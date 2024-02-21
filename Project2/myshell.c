/* CS 347 -- Mini Shell!
 * Original author Phil Nelson 2000
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include "argparse.h"
#include "builtin.h"


/* PROTOTYPES */

void processline (char *line);
ssize_t getinput(char** line, size_t* size);

/* main
 * This function is the main entry point to the program.  This is essentially
 * the primary read-eval-print loop of the command interpreter.
 */

int main () {

  while(1){

    char** line = calloc(10,10);  // what is this doing 10 x 10, necessary??
    line[0] = calloc(10, 10);
    getinput(line, (size_t*) 10);
    processline(*line);
    free(line[0]);
    free(line);

  }
  return EXIT_SUCCESS;
}


/* getinput
* line     A pointer to a char* that points at a buffer of size *size or NULL.
* size     The size of the buffer *line or 0 if *line is NULL.
* returns  The length of the string stored in *line.
*
* This function prompts the user for input, e.g., %myshell%.  If the input fits in the buffer
* pointed to by *line, the input is placed in *line.  However, if there is not
* enough room in *line, *line is freed and a new buffer of adequate space is
* allocated.  The number of bytes allocated is stored in *size. 
* Hint: There is a standard i/o function that can make getinput easier than it sounds.
*/
ssize_t getinput(char** line, size_t* size) {
  //write your code

  ssize_t len = 0;
  printf("%c ", 37);

  while(1){

    char c = fgetc(stdin);

    if(c == EOF){
      perror("fgetc error");
    }

    if (c == '\n'){
      break;
    } else {

      if(strlen(*line) == (intptr_t) size){

        char temp[(intptr_t) size];
        strcpy(temp, *line);
        free(*line);
        *line = (char*) calloc((intptr_t) size * 2, (intptr_t) size * 2);
        strcpy(*line, temp);

      }

      len++;
      strncat(*line, &c, 1);

    }
  }
  return len;
}


/* processline
 * The parameter line is interpreted as a command name.  This function creates a
 * new process that executes that command.
 * Note the three cases of the switch: fork failed, fork succeeded and this is
 * the child, fork succeeded and this is the parent (see fork(2)).
 * processline only forks when the line is not empty, and the line is not trying to run a built in command
 */
void processline (char *line) {

  if (strlen(line) == 0){
    return;
  }

  pid_t cpid;
  int   status;
  int argCount;
  char** arguments = argparse(line, &argCount);

  if(strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "pwd") == 0 || strcmp(arguments[0], "cd") == 0
  || strcmp(arguments[0], "ls") == 0 || strcmp(arguments[0], "cp") == 0 || strcmp(arguments[0], "env") == 0){ // dont fork

    if(builtIn(arguments, argCount) == 0){
      printf("No command found");
    }

  } else { 

    cpid = fork();

    if(cpid == 0) {
      
      char* test = calloc(strlen(line) + 1, strlen(line) + 1);
      strcpy(test, "/bin/");
      strcat(test, arguments[0]); 

      if(execv(test, arguments) == -1){
        perror("not valid command");
      }

      for(int i = 1; i < argCount; i++){
        if(execvp(arguments[i], arguments) == -1){
          perror("not valid command");
        }
      }       
    } else if(cpid > 0) {
      wait(&status);  // is status actually doing anything?
    } else {
      perror("fork failed");
    }
  }

  free(arguments[0]);
  free(arguments);
}