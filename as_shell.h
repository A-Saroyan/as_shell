#ifndef AS_SHELL_H
#define AS_SHELL_H

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>

#define LINE_SIZE 200
#define TOKEN_SIZE 50 
#define KEY_SIZE 50
#define VALUE_SIZE 50

struct Pair
{
    char key [KEY_SIZE];
    char value [VALUE_SIZE];
};


void as_print_help ();
void as_print_tokens (char** tokens,int token_count);
void as_get_token_count (char* input,int*  token_count);
void as_make_tokens(char** tokens,char* input);
void as_free(char** tokens,int token_count);
void as_cd (char* PWD);
void as_exit();
void as_echo(char** tokens,int token_count);
void as_set_initial_PWD ();
void as_history_write (char* input);
void as_history_print();
void as_check_variables (char** token,int token_count);
void as_set (char* token);
void as_unset (char * token);
void as_print_pwd ();
void as_parser(char **tokens,int token_count);
void start ();


#endif