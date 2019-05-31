//
//  definitions.h
//  Forks
//
//  Created by Bradley Juma on 3/15/19.
//  Copyright © 2019 Bradley Juma. All rights reserved.
//

#ifndef definitions_h
#define definitions_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>


//Global Variables
FILE *batchfp;
int argv_count;
//Batch file Flag
int batch;
//Bacground Exec Flag
int background;
// Redirection Flag
int redi;
// I/O stdin "<"
int std_in;
// I/O stdin ">"
int std_out;
// I/O stdin ">>"
int std_out_append;
int std_in_index;
int std_out_index;
// I/O stdin "|"
int flag_pipe;
int flag_pipe_index;
char *cmd_prompt;

//Internal Commands
int _cd(char **argv);
int _clr(char **dir);
int _dir(char **argv);
int _echo(char **argv);
int _environ(char **argv);
int _help(char **argv);
int _pause(char **argv);
int _quit(char **argv);
int _mkdir(char **argv);

//Shell Functions
void environ_init(char* batchfile);
int _exe(char **argv);
void cmdflags(char **argv);
void shell_loop(void);
int _pipe(char **argv);
int prg_exe(char **argv);
void rmv_arg(char **argv, int index);
char* join_lines(char** lines, int size);
char **split_line(char *line);

//Struct for environment variable
typedef struct{
    //current working directory
    char *cwd;
    //directory myshell is runing from
    char *dir;
    //shell executable path
    char *shell;
} environment;
environment environ;

//Array of internal commands fucntions without IO
char *internals[]={"cd","clr","quit","pause"};
//List of command functions
int (*internals_funcs[])(char **)={&_cd,&_clr,&_quit,&_pause};
//List of internal command functions with IO Redirection
char *internalsIO[]={"help","dir","environ","echo","mkdir"};
//List of command functions
int (*internalsIO_funcs[])(char **)={&_help,&_dir,&_environ,&_echo,&_mkdir};

// COMMAND COUNT FOR HOW MANY INTERNALS AVAILABLE
const int CMD_CNT = 4;
// COMMAND COUNT FOR HOW MANY INTERNALS AVAILABLE
const int CMD_IO_CNT = 5;

#define LINE_SIZE 1024
#define TOK_SIZE 32
#define TOK_DELIMS " \t\n"


#endif /* definitions_h */
