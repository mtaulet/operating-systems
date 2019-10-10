/*
*	Marta Taulet Sanchez
*	EC440 Project 1
*	functions.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "functions.h"

#define MAX_SIZE_TOKEN 32
#define TOK_BUFSIZE 512

int compare(char * str1, char * str2) {

  int i = 0;
  while ((str1[i] != '\0') && (str2[i] != '\0')) {
    if (str1[i] != str2[i]) {
      return 0;
    }
    i++;
  }
  return 1;
}


int background (char ** args) {
  int i = 0;
  char * ampersant = "&";
  while ((i < sizeof(args)) && (args[i] != NULL))
   {
    if (compare(args[i], ampersant)) { 
    	args[i] = NULL; 
      return 1;
    }
    i++;
  }  

  return 0;
}


char** split_input (char* line, int* pos, char* tok_delim) {

  int bufsize = TOK_BUFSIZE;
  int position = 0;

  char **tokens = malloc(bufsize * sizeof(char*));
  char *token = malloc(MAX_SIZE_TOKEN);
  if (!tokens) {
    fprintf(stderr, "ERROR: allocation error\n");
    exit(EXIT_FAILURE);
  } 
  
  token = strtok(line, tok_delim); 

  while (token != NULL) {
    tokens[position] = token;
    position++;
    if (position >= bufsize) {
      bufsize += TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "ERROR: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, tok_delim);
  }
  tokens[position] = NULL;  // Last char is null
  *pos = position;

  return tokens;

}

