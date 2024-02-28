#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <fcntl.h> 
#include <ctype.h>
#include "builtin.h"
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define DefaultPermissions (0644)  // default file permissions for cp

//Prototypes
static void exitProgram(char** args, int argcp);
static void cd(char** args, int argpcp);
static void pwd(char** args, int argcp);
static void ls(char** args, int argcp);
static void env(char** args, int argcp);
static void cp(char** args, int argcp);

/* builtIn
 ** built in checks each built in command the given command, if the given command
 * matches one of the built in commands, that command is called and builtin returns 1.
 *If none of the built in commands match the wanted command, builtin returns 0;
  */
int builtIn(char** args, int argcp) {          // error handling to check proper num args and command names
  if (strcmp(args[0], "ls") == 0 && argcp < 3) {          // also calls proper functions to implement commands
    ls(args, argcp);
  } else if (strcmp(args[0], "env") == 0 && argcp < 3) {
    env(args, argcp);
  } else if (strcmp(args[0], "cp") == 0 && argcp == 3) {
    cp(args, argcp);
  } else if (strcmp(args[0], "exit") == 0 && argcp < 3) {
    exitProgram(args, argcp);
  } else if (strcmp(args[0], "pwd") == 0 && argcp == 1) {
    pwd(args, argcp);
  } else if (strcmp(args[0], "cd") == 0 && argcp < 3) {
    cd(args, argcp);
  } else {
    return 0;  // return 0 when command not found
  }
  return 1;    // return 1 on success
}

// prints out all info from stat needed
// num links, group name, etc...
void printP(char *fd, int maxByte) {

  struct stat *buf = calloc(sizeof(struct stat), sizeof(struct stat));
  if (stat(fd, buf) == -1) {   // error check to make sure file exists
    perror("file does not exist");
  }
  int m = buf->st_mode;

  struct passwd *p;
  p = getpwuid(buf->st_uid);
  if (p == NULL) {  // error check for reading password struct
    perror("error or entry not found");
  }

  struct group *g;
  g = getgrgid(buf->st_gid);
  if (g == NULL) {  // error check for reading group struct
    perror("error or entry not found");
  }
                        // array of possible permissions
  int mode [9] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};

  if (S_ISDIR(m)) {  // determine if file is directory or other
    printf("d");
  } else if(S_ISREG(m)) {
    printf("-");
  } else if(S_ISCHR(m)) {
    printf("c");
  } else if(S_ISBLK(m)) {
    printf("b");
  } else if(S_ISFIFO(m)){
    printf("p");
  } else if(S_ISLNK(m)) {
    printf("l");
  } else if(S_ISSOCK(m)) {
    printf("s");
  }

  for (int i = 0 ; i < 9; i++) {   // print proper file permissions
    if (m & mode[i] && (i == 0 || i == 3 || i == 6)) {
      printf("r");
    } else if (m & mode[i] && (i == 1 || i == 4 || i == 7)) {
      printf("w"); 
    } else if (m & mode[i] && (i == 2 || i == 5 || i == 8)) {
        printf("x");
    } else {
        printf("-");
    }
  }

  printf(" %ld ", buf->st_nlink);
  printf("%s ", p->pw_name);
  printf("%s ", g->gr_name);
  printf("%*ld ", maxByte, buf->st_size);

  struct tm *t;
  t = gmtime(&buf->st_mtim.tv_sec);
  if (t == NULL) {  // error check to make sure tm struct is read properly
    perror("error or entry not found");
  }

  if (t->tm_hour - 8 < 0) {    // simple calc to convert to pacific northwest time
    t->tm_hour = 24 + (t->tm_hour - 8);
    t->tm_mday = t->tm_mday - 1;
  } else {
    t->tm_hour = t->tm_hour - 8;
  }
 
  char buff[100];   // buff won't exceed this size
  strftime(buff, sizeof(buff), "%b %d %H:%M", t);  // format time
  printf("%s ", buff);
  free(buf);
}

static void ls(char** args, int argcp) {

  DIR *dir;
  struct dirent *entry;
  dir = opendir(".");
  if (dir == NULL) {  // make sure directory was read properly
    perror("error or entry not found");
  }
  int maxByte = 1;
  int curByte = 1;
  int total = 0;

  if (argcp == 1) {
           // prints all entry names in current directory
    while ((entry = readdir(dir)) != NULL) {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && entry->d_name[0] != '.') {
        printf("%s\n", entry->d_name);
      }
    }

  } else if (argcp == 2 && strcmp(args[1], "-l") == 0) {

    struct dirent *entry2;

    while ((entry2 = readdir(dir)) != NULL) {   // read through files in directory

      if (strcmp(entry2->d_name, ".") != 0 && strcmp(entry2->d_name, "..") != 0 && entry2->d_name[0] != '.') {
        
        struct stat *buf = malloc(sizeof(struct stat));

        if (stat(entry2->d_name, buf) == -1) {   // make sure stat read file properly
          perror("file does not exist");
        }

        total += buf->st_blocks/2;

        while (buf->st_size > 9) {   // used later for formatting byte size of file
          curByte++; 
          buf->st_size/=10; 
        }

        if (curByte > maxByte) {   // byte formatting later
          maxByte = curByte;
        }

        curByte = 1;
        free(buf);
      }
    }

    rewinddir(dir);  // rewind directory to get info again
    printf("total: %d\n", total);

    while ((entry = readdir(dir)) != NULL) {
      
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && entry->d_name[0] != '.') {
        printP(entry->d_name, maxByte);
        printf("%s\n", entry->d_name);
      }
    }
  }
  closedir(dir);
}

static void env(char** args, int argcp) {

  if (argcp == 2) { // we are setting env var
        // create space for all necessary variables
    char* change = calloc(sizeof(args[1]), sizeof(args[1]));
    char* changeReset = change;
    strcpy(change, args[1]);
    char* name = calloc(sizeof(args[1]), sizeof(args[1]));
    char* value = calloc(sizeof(args[1]), sizeof(args[1]));
    int tracker = 0;
    int other = 0;
                
    for (int i = 0; i < strlen(args[1]); i++) {  // grab global var name
      if (*change != '=' && other == 0) {
        name[tracker] = *change;
        tracker++;
        change++;
      } else if (*change != '=' && other == 1) {  // grab global var assigned var
        value[tracker] = *change;
        change++;
        tracker++;
      } else {
        tracker = 0;
        other = 1;
        change++;
      }
    }

    if (setenv(name, value, 1) == -1) {
      perror("could not set environment with value");
    }
    change = changeReset;  // reset pointer
    free(change);
    free(name);
    free(value);
  }

  extern char **environ;
  for (int i = 0; environ[i] != NULL; i++) {
    printf("%s\n", environ[i]);  // loop through environment variables and print
  }     
}

static void cp(char** args, int argcp) {

  char c;                                            
  int fd = open(args[1], O_RDONLY);
  int errorCheck;

  if (fd == -1) {  // check for open error
    perror("failed to open file");
  } else {

    int fd1 = open(args[2], O_WRONLY | O_CREAT, DefaultPermissions);
    if(fd1 == -1){
      perror("failed to open file");
    }
         // read byte at a time printing to new file
    while ((errorCheck = read(fd, &c, 1)) != 0) { 

      if (errorCheck == -1) {
        perror("fread failed");
      }
      write(fd1, &c, 1);
    }

    if (close(fd) == -1) {  // error handling
      perror("failed to close file");
    }
    
    if (close(fd1) == -1) {
      perror("failed to close file");
    }
    }
}

static void exitProgram(char** args, int argcp) {

  int status = (intptr_t) args[1];  // create int var to free args

  for (int i = 0; i < argcp; i++) {
    free(args[i]);
  }
  free(args);

  if (argcp == 2) {  // they chose status
    exit(status);
  } else {   // default exit status
    exit(0);
  }
}

static void pwd(char** args, int argpc) {
  
  char cwd[260]; // maximum file path length
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    perror("getting file path failed");
  }
  printf("%s\n", cwd);
}

static void cd(char** args, int argcp) {
 
  if (strcmp(args[0], "cd") == 0 && argcp == 1) {  // send back to /home
    if (chdir("/home") == -1) {
      perror("chdir error");
    }
  } else if (strcmp(args[0], ".") == 0 && argcp == 2) {
    // do nothing, opened current path
  } else if (strcmp(args[0], "..") == 0 && argcp == 2) {
    if (chdir("..") == -1) {  // go back one
      perror("chdir error");
    }
  } else {   // they chose file path
    if (strcmp(args[0], "cd") != 0 || chdir(args[1]) == -1 || argcp != 2) {
      perror("Incorrect file path");
    }
  }
}