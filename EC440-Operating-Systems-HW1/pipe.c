/*
* Marta Taulet Sanchez
* EC440 Project 1
* pipe.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <fcntl.h> 

#include "pipe.h"
#include "functions.h"

#define OUTPUT_MODE 0644
#define TOK_BUFSIZE 512

int create_pipe (struct Pipe * mypipe, char** sector) {

  char ** argv1 = malloc(TOK_BUFSIZE * sizeof(char*));
  char ** argv2 = malloc(TOK_BUFSIZE * sizeof(char*));
  int * argc1 = malloc(sizeof(int));
  int * argc2 = malloc(sizeof(int));

  int * p = malloc(sizeof(int));

  argv1 = split_input(sector[0], argc1, "<");

  if (*argc1 == 1) // No input redirection for sector 1
  {
    mypipe->argv1 = split_input(argv1[0], p, " \n");
    mypipe->backg1 = background(mypipe->argv1);
    mypipe->input_redir = 0;
    mypipe->file_in = NULL;
  }
  if (*argc1 == 2) // Input redirection for sector 1
  {
    mypipe->argv1 = split_input(argv1[0], p, " \n");
    mypipe->backg1 = background(mypipe->argv1);
    mypipe->input_redir = 1;
    mypipe->file_in = argv1[1];
  }
  if (*argc1 > 2) {
    printf("ERROR: Invalid input. More than one input redirection\n");
    return 0;
  }

  argv2 = split_input(sector[1], argc2, ">");

  if (*argc2 == 1) // No OUTPUT redirection for sector 2
  {
    mypipe->argv2 = split_input(argv2[0], p, " \n");
    mypipe->backg2 = background(mypipe->argv2);
    mypipe->output_redir = 0;
    mypipe->file_out = NULL;
  }

  if (*argc2 == 2)  // Output redirection for sector 2
  {
    mypipe->argv2 = split_input(argv2[0], p, " \n");
    mypipe->backg2 = background(mypipe->argv2);
    mypipe->output_redir = 1;
    mypipe->file_out = (split_input(argv2[1], p, " \n"))[0];
  }
  if (*argc2 > 2) {
    printf("ERROR: Invalid input. More than one output redirection\n");
    return 0;
  }

    free(argv1);
    free(argv2);
    free(argc1);
    free(argc2);
    return 1;

}


int exec_pipe (struct Pipe mypipe) 
{
   pipe(mypipe.pipe_array);                        // Create a pipe.

   mypipe.pid1 = fork();                           // Create our first process to the left of pipe
   if (mypipe.pid1 == 0) 
   {

      if(mypipe.input_redir) {
         int fd1 = open(mypipe.file_in, O_RDONLY);
         dup2(fd1, STDIN_FILENO);
         close(fd1);
      }
      dup2(mypipe.pipe_array[1], STDOUT_FILENO);
      close(mypipe.pipe_array[0]);

      execvp(mypipe.argv1[0], mypipe.argv1);
      perror("exec error in the child");
      return 1;
   }

   
   mypipe.pid2 = fork();                           // Create our second process to the right of pipe
   if (mypipe.pid2 == 0) 
   {
      if(mypipe.output_redir) {
         int fd2 = creat(mypipe.file_out, OUTPUT_MODE);
         dup2(fd2, STDOUT_FILENO);
         close(fd2);
      }
      dup2(mypipe.pipe_array[0], STDIN_FILENO);
      close(mypipe.pipe_array[1]);

      execvp(mypipe.argv2[0], mypipe.argv2);
      perror("exec");
      return 1;
   }

   close(mypipe.pipe_array[0]);
   close(mypipe.pipe_array[1]);

   if (!mypipe.backg1) { waitpid(mypipe.pid1, &mypipe.status1, 0);}
   if (!mypipe.backg2) { waitpid(mypipe.pid2, &mypipe.status2, 0);}
   return 0;
}
