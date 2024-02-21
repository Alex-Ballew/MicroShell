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
#include <pwd.h> // I added
#include <grp.h> // I added
#include <time.h> // I added

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
int builtIn(char** args, int argcp)                 // NO ERROR handling yet, have mercy 
{
  if(strcmp(args[0], "ls") == 0){
    ls(args, argcp);
  } else if (strcmp(args[0], "env") == 0){
    env(args, argcp);
  } else if (argcp == 3 && strcmp(args[0], "cp") == 0){   // this is proably supposed to work with file paths
    cp(args, argcp);
  } else if (strcmp(args[0], "exit") == 0){
    exitProgram(args, argcp);
  } else if (strcmp(args[0], "pwd") == 0){
    pwd(args, argcp);
  } else if (strcmp(args[0], "cd") == 0){
    cd(args, argcp);
  } else {
    return 0;
  }
  return 1;
}

// prints out all info from stat needed
// num links, group name, etc...
void printP(char *fd, int maxByte){

  struct stat *buf = malloc(sizeof(struct stat));
  if(stat(fd, buf) == -1){
    perror("file does not exist");
  }
  int m = buf->st_mode;

  struct passwd *p;
  p = getpwuid(buf->st_uid);
  if(p == NULL){
    perror("error or entry not found");
  }

  struct group *g;
  g = getgrgid(buf->st_gid);
  if(g == NULL){
    perror("error or entry not found");
  }

  int mode [9] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};

  if(S_ISDIR(m)){  // determine if file is directory, need to add other options
    printf("d");
  } else if(S_ISREG(m)){
    printf("-");
  } else if(S_ISCHR(m)){
    printf("c");  // check later
  } else if(S_ISBLK(m)){
    printf("b");
  } else if(S_ISFIFO(m)){
    printf("p");
  } else if(S_ISLNK(m)){
    printf("l");
  } else if(S_ISSOCK(m)){
    printf("s");
  }

  for(int i = 0 ; i < 9; i++){   // deals with file permissions, proably better way to do this
    if(m & mode[i] && (i == 0 || i == 3 || i == 6)){
      printf("r");
    } else if (m & mode[i] && (i == 1 || i == 4 || i == 7)){
      printf("w"); 
    } else if (m & mode[i] && (i == 2 || i == 5 || i == 8)){
        printf("x");
    } else {
        printf("-");
    }
  }

  printf(" %ld ", buf->st_nlink);
  printf("%s ", p->pw_name);
  printf("%s ", g->gr_name);
  printf("%*ld ", maxByte, buf->st_size); // thing after permissions needs formatting aswell

  struct tm *t;
  t = gmtime(&buf->st_mtim.tv_sec);
  if(t == NULL){
    perror("error or entry not found");
  }

  if(t->tm_hour - 8 < 0){
    t->tm_hour = 24 + (t->tm_hour - 8);
    t->tm_mday = t->tm_mday - 1;
  } else {
    t->tm_hour = t->tm_hour - 8;
  }
 
  char buff[100];   // gets formated time for ls -l
  strftime(buff, sizeof(buff), "%b %d %H:%M", t);  // works idk why % is red
  printf("%s ", buff);
  free(buf);
}

static void ls(char** args, int argcp){

  DIR *dir;
  struct dirent *entry;
  dir = opendir(".");
  if(dir == NULL){
    perror("error or entry not found");
  }
  int maxByte = 1;
  int curByte = 1;
  int total = 0;

  if(argcp == 1 && strcmp(args[0], "ls") == 0){

    while ((entry = readdir(dir)) != NULL){
      if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && entry->d_name[0] != '.'){
        printf("%s\n", entry->d_name);
      }
    }

  } else if(argcp == 2 && strcmp(args[0], "ls") == 0 && strcmp(args[1], "-l") == 0){

    struct dirent *entry2;

    while((entry2 = readdir(dir)) != NULL){

      if(strcmp(entry2->d_name, ".") != 0 && strcmp(entry2->d_name, "..") != 0 && entry2->d_name[0] != '.'){
        
        struct stat *buf = malloc(sizeof(struct stat));

        if(stat(entry2->d_name, buf) == -1){
          perror("file does not exist");
        }

        total += buf->st_blocks/2;

        while(buf->st_size > 9){ 
          curByte++; 
          buf->st_size/=10; 
        }

        if(curByte > maxByte){
          maxByte = curByte;
        }

        curByte = 1;
        free(buf);
      }
    }

    rewinddir(dir);
    printf("total: %d\n", total);

    while ((entry = readdir(dir)) != NULL){
      
      if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && entry->d_name[0] != '.'){
        printP(entry->d_name, maxByte);
        printf("%s\n", entry->d_name);
      }
    }
  }
  closedir(dir);
}

static void env(char** args, int argcp){

  if (argcp == 2) { 

    char* change = malloc(sizeof(args[1]));
    strcpy(change, args[1]);
    char* name = malloc(sizeof(args[1]));
    char* value = malloc(sizeof(args[1]));
    int tracker = 0;
    int other = 0;
                // proably gonna need some error handling, deal with later
    for(int i = 0; i < strlen(args[1]); i++){
      if(*change != '=' && other == 0){
        name[tracker] = *change;
        tracker++;
        change++;
      } else if (*change != '=' && other == 1){
        value[tracker] = *change;
        change++;
        tracker++;
      } else {
        tracker = 0;
        other = 1;
        change++;
      }
    }

    setenv(name, value, 1);  // overwrites if name exists
  }

  extern char **environ;
  for(int i = 0; environ[i] != NULL; i++){
    printf("%s\n", environ[i]);
  }     
  //exit(3);
}

static void cp(char** args, int argcp) {

  char c;                                               // will deal with later
  int fd = open(args[1], O_RDONLY);
  int fd1 = open(args[2], O_WRONLY | O_CREAT, 0644);  // magic number, deal with later

  while(read(fd, &c, 1) != 0){
      write(fd1, &c, 1);
  }

  close(fd);
  close(fd1);
  //exit(3);
}

static void exitProgram(char** args, int argcp) {
 //write your code
 // prob should error hanlde for if user enters something like exit a
  if(args[1] != 0){
    exit(*args[1]);// later
  } else {
    exit(3);
  }
}

static void pwd(char** args, int argpc) {
  
  char cwd[256];
  if (getcwd(cwd, sizeof(cwd)) == NULL){
    perror("getting file path failed");
  }
  printf("%s\n", cwd);
}

static void cd(char** args, int argcp) {
 
  if(argcp == 1){
    chdir("/home");
  } else if(strcmp(args[0], ".") == 0){
    // ask in class
  } else if(strcmp(args[0], "..") == 0){
    chdir("..");
  } else {
    if(chdir(args[1]) == -1){
      perror("Incorrect file path");
    }
  }
}