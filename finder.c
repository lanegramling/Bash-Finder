#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256

#define BASH_EXEC  "/bin/bash"
#define FIND_EXEC  "/usr/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC  "/bin/grep"
#define SORT_EXEC  "/usr/bin/sort"
#define HEAD_EXEC  "/usr/bin/head"

int main(int argc, char *argv[])
{
  int status;
  pid_t pid_1, pid_2, pid_3, pid_4;


  if (argc != 4) {
    printf(" usage: finder DIR STR NUM_FILES\n");
    exit(0);
  }

  int fd1[2], fd2[2], fd3[2]; //File Descriptors ---  0 : Read End | 1 : Write End

  pipe(fd1); pipe(fd2); pipe(fd3); //Pipes on the file descriptors.

  pid_1 = fork();
  if (pid_1 == 0) {
    /* First Child */// 'find' execution:
    char buf[BSIZE];   //Buffer for chars
    bzero(buf, BSIZE); //Initialize buffer with zeroes
    sprintf(buf, "%s %s -name \'*\'.[ch]", FIND_EXEC, argv[1]); //Build given 'find' command and store in buffer
    printf("%s", buf);

    dup2(fd1[1], STDOUT_FILENO);  //Start pumping into the write end of pipe 1
    close(fd1[0]);                //Close unused end

    char* find_args[] = {BASH_EXEC, "-c", buf, (char*) 0}; //Finish building arguments to execute with bash
    execv(BASH_EXEC, find_args);

    exit(0);
  }

  close(fd1[1]); //Close end whose use has expended

  pid_2 = fork();
  if (pid_2 == 0) {
    /* Second Child */// 'grep' execution:
    char buf[BSIZE];   //Buffer for chars
    bzero(buf, BSIZE); //Initialize buffer with zeroes
    sprintf(buf, "%s %s -c %s", XARGS_EXEC, GREP_EXEC, argv[2]); //Build given 'xargs grep' command and store in buffer

    dup2(fd1[0], STDIN_FILENO);  //Listen to the read end of pipe 1
    dup2(fd2[1], STDOUT_FILENO);  //Pump the output into pipe 2

    char* grep_args[] = {BASH_EXEC, "-c", buf, (char*) 0}; //Finish building arguments to execute with bash
    execv(BASH_EXEC, grep_args);

    exit(0);
  }

  close(fd1[0]);    //Close ends whose use has expended
  close(fd2[1]);

  pid_3 = fork();
  if (pid_3 == 0) {
    /* Third Child */// 'sort' execution:
    char buf[BSIZE];   //Buffer for chars
    bzero(buf, BSIZE); //Initialize buffer with zeroes
    sprintf(buf, "%s -t : +1.0 -2.0 --numeric --reverse", SORT_EXEC); //Build given 'sort' command and store in buffer

    dup2(fd2[0], STDIN_FILENO);  //Listen to the read end of pipe 2
    dup2(fd3[1], STDOUT_FILENO);  //Pump the output into pipe 3

    char* sort_args[] = {BASH_EXEC, "-c", buf, (char*) 0}; //Finish building arguments to execute with bash
    execv(BASH_EXEC, sort_args);

    exit(0);
  }

  close(fd2[0]);    //Close ends whose use has expended
  close(fd3[1]);

  pid_4 = fork();
  if (pid_4 == 0) {
    /* Fourth Child */// 'head' execution:
    char buf[BSIZE];   //Buffer for chars
    bzero(buf, BSIZE); //Initialize buffer with zeroes
    sprintf(buf, "%s --lines=%s", HEAD_EXEC, argv[3]); //Build given 'head' command and store in buffer

    dup2(fd3[0], STDIN_FILENO);  //Listen to the read end of pipe 3

    char* head_args[] = {BASH_EXEC, "-c", buf, (char*) 0}; //Finish building arguments to execute with bash
    execv(BASH_EXEC, head_args);

    exit(0);
  }

  close(fd3[0]); //Close end whose use has expended


  if ((waitpid(pid_1, &status, 0)) == -1) {
    fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_2, &status, 0)) == -1) {
    fprintf(stderr, "Process 2 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_3, &status, 0)) == -1) {
    fprintf(stderr, "Process 3 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_4, &status, 0)) == -1) {
    fprintf(stderr, "Process 4 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }


  return 0;
}
