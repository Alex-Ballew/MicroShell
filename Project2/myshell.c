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

#define TRUE (1)

/* PROTOTYPES */

void processline (char *line);
ssize_t getinput(char** line, size_t* size);

/* main
 * This function is the main entry point to the program.  This is essentially
 * the primary read-eval-print loop of the command interpreter.
 */

int main () {

  while (TRUE) {   // loop until exit

    char** line = calloc(3,3); 
    line[0] = calloc(10, 10);
    getinput(line, (size_t*) 10);  // retrieves initial line
    processline(*line);      // adjusts line to align with arguments and calls builtin
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

  ssize_t len = 0;
  printf("%c ", 37);
                    // reads character by character mallocing and freeing when needed
  while (TRUE) {

    char c = fgetc(stdin);

    if (c == EOF) {
      perror("fgetc error");
    }

    if (c == '\n') {   // loops until end of line
      break;
    } else {

      if(strlen(*line) == (intptr_t) size) {   // need to malloc for more space
            // create temp copy over and free after more space is made
        char temp[(intptr_t) size];
        strcpy(temp, *line);
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

  if (strlen(line) == 0) {   // check for empty line
    return;
  }

  pid_t cpid;
  int   status;
  int argCount;
  char** arguments = argparse(line, &argCount);
                             // make sure isn't command I implemented 
  if (builtIn(arguments, argCount) == 1) { // dont fork
    // calls in builtin 
  } else { 

    cpid = fork();

    if (cpid == 0) {
                      // create space for non implemented calls
      char* defaultCommand = calloc(strlen(line) + 1, strlen(line) + 1);
      strcpy(defaultCommand, "/bin/");
      strcat(defaultCommand, arguments[0]); 

      if (execv(defaultCommand, arguments) == -1) {
        perror("not valid command");
        free(defaultCommand);
        exit(1);
      } else {    // iterate over all arguments to call proper command
        for (int i = 1; i < argCount; i++) { 
          if (execvp(arguments[i], arguments) == -1) {
            perror("not valid command");
            free(defaultCommand);
            exit(1);   // error exit with status 1 to stop child process
          }
        }      
      }
    } else if(cpid > 0) {
      if (wait(&status) == -1) {   // wait for child to finish to prevent zombies
        perror("error waiting for child to finish on fork");
      }
    } else {
      perror("fork failed");
    }
  }

  for (int i = 0; i < argCount; i++) {  // free up last used space before restarting
    free(arguments[i]);
  }
  free(arguments);
  free(line);
}