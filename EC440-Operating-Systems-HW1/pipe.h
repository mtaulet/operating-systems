/*
* Marta Taulet Sanchez
* EC440 Project 1
* pipe.h
*/

struct Pipe {

   pid_t pid1, pid2;
      int pipe_array[2];
      int status1, status2;
      char **argv1;
      char **argv2;
      int input_redir, output_redir;
      char* file_in;
      char* file_out;
      int backg1, backg2;

};

int create_pipe (struct Pipe * mypipe, char** sector);
    /* 
      Inputs: 
          mypipe: Pointer to a Pipe struct that will be fully formed 
                  at completion of subroutine
          sector: array that contains 2 strings representing the two 
                  sides of the pipe
      Returns:
          int: success 1, error 0
    */
int exec_pipe (struct Pipe mypipe);
    /* 
      Inputs: 
          mypipe: Pipe to be executed
      Returns:
          int: failed children 1, parent 0
    */