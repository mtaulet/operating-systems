/*
*	Marta Taulet Sanchez
*	EC440 Project 1
*	myshell.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "pipe.h"
#include "functions.h"

char *get_input(void)
{
  char *line = NULL;
  size_t buffer = 0;
  getline(&line, &buffer, stdin);
  return line;
}

int execute(char **args) {  // Note: Still needs to handle I/O redirection for non-piped inputs

  pid_t pid, wpid;
  int status;

  int bkg = background(args);

  pid = fork();
  if (pid == 0) {   // Child process
    
    if (execvp(args[0], args) == -1) {
      perror("ERROR");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {

    perror("ERROR: Error forking");
  } else {    // Parent process
    do {
      if (!bkg) { wpid = waitpid(pid, &status, 0);}
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}


int main(int argc, char **argv) {

  int print_promt = 1;
  char * no_prompt = "n\n";
  char *line;
  int status;

  if ((argc > 1) && compare(argv[1], no_prompt)) {
    print_promt = 0;
  }

  do {

    if (print_promt) { printf("> ");}

    line = get_input();
    char * line_copy = line;


    int* num_pipe_sectors_ptr = malloc(sizeof(int));

    char** pipe_sectors = split_input(line, num_pipe_sectors_ptr, "|");

  	int pipe_sectors_int = *num_pipe_sectors_ptr;

    int pipes_num = pipe_sectors_int - 1;

    if (pipes_num > 0) {                      // With pipes

    	struct Pipe ** all_pipes = malloc(sizeof(pipes_num));      // Array with all pipe objects
      char* sector_pair [2];

	    for (int i = 0; i < pipes_num; i++) {   // Set up all pipes
        struct Pipe onepipe;

        onepipe.argv1 = malloc(64 * sizeof(char*));
        onepipe.argv2 = malloc(64 * sizeof(char*));
        all_pipes[i] = &onepipe;

        sector_pair[0] = pipe_sectors[i];
        sector_pair[1] = pipe_sectors[i+1];
	    	create_pipe(all_pipes[i], sector_pair);
	    }

      for (int i = 0; i < pipes_num; i++)  {   // Execute pipes
        exec_pipe(*(all_pipes[i]));
      }

	  }

    if (pipes_num == 0) {                   // No pipes
      int * x = malloc(sizeof(int));
      char ** arguments = split_input(line_copy, x, " \n");
      execute(arguments);

    }

    free(line);

  } while (status);

  return 1;
}
