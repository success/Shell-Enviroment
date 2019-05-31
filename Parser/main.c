//
//  main.c
//  Splits
//
//  Created by Bradley Juma on 2/14/19.
//  Copyright © 2019 Bradley Juma. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>



#define TOK_BUFSIZE 128
#define TOK_DELIMS " \t\n"
#define TOK_SIZE 32
#define LINE_SIZE 1024


char **split_line(char *);

char **split_lines(char *);

char **line(char *);

int argv_count;

int main(){
    
    //char out [100] = "amazon prime hahahhaha\t\t asd 2 da \n\0";
    char *out = "amazon prime hahahhaha\t\t asd 2 da\n";
    char **results  = split_line(out);
    char **result  = split_lines(out);
    char **resultz  = line(out);
    int i = 0;
    
    while(i <= 6){
        
        printf("%s\n", results[i]);
        i++;
    }
    
    
}

char **split_line(char *line) {
    char **tokens;
    char *token;
    int pos = 0;
    
    char out [100];
    int i = 0;
    
    while (1) {
        out[i] = line[i];
        if (out[i] == '\0' || out[i] == '\n') {
            out[i] = '\0';
            break;
        }
        i++;
    }

    if ((tokens = malloc(sizeof(char*) * TOK_BUFSIZE)) == NULL)
        exit(EXIT_FAILURE);

    token = strtok(out, TOK_DELIMS);
    //printf("Bye\n");
    while (token != NULL) {
        //printf("%s %d \n",token, pos);
        tokens[pos++] = token;
        
        token = strtok(NULL, TOK_DELIMS);
    }

    return tokens;
}


char **split_lines(char *line){
    char *arg;
    char **argv = (char**) malloc(sizeof(char*)*TOK_SIZE);
    if(argv == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    arg = strtok(line, TOK_DELIMS);//begin token
    while (arg){
        argv[argv_count++] = arg;//send token to argv
        if (argv_count >= TOK_SIZE) {//check buffer overflow
            int size = TOK_SIZE*2;
            argv = realloc(argv, sizeof(char*)*size);
            if(argv==NULL){
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }
        arg = strtok(NULL, TOK_DELIMS);//get next token
    }
    argv[argv_count++] = NULL;//End of argv and num_args++ for NULL
    return argv;
}

