//
//  main.c
//  Forks
//
//  Created by Bradley Juma on 2/15/19.
//  Copyright © 2019 Bradley Juma. All rights reserved.
//

#include "definitions.h"

int main(int argc, char **argv){
    // If arguemnt is greater than 1
    if(argc > 1)
        //Batch file being the only argument taken is set to true
        batch=1;
    //Bread & Butter
    environ_init(argv[1]);
    //Shell loop
    shell_loop();
    
    return EXIT_SUCCESS;
}


/*---------------------------------------------------------------*
 *        Initialzing the enviroment for the shell                 *
 *  If the batch file is the value taken in then the BatchFile name. *
 *------------------------------------------------------------------*/
void environ_init(char* batchfile){
    // Detecting batchfile
    if(batch)
        //open batchfile for input
        batchfp = fopen(batchfile,"r");
    /*Allocating (calloc), sets it back to 0 for less memory leaks*/
    environ.cwd = (char*)calloc(255,1);
    getcwd(environ.cwd,255);
    
    environ.dir = (char*)calloc(255,1);
    getcwd(environ.dir,255);
    
    environ.shell = (char*)calloc(255,1);
    getcwd(environ.shell,255);
    
    strcat(environ.shell,"/myshell");
    
    cmd_prompt = (char *) malloc(sizeof(char)*LINE_SIZE);
    strcpy(cmd_prompt,environ.cwd);
}
/*---------------------------------------------------------------------------------------*
 *        Finding full length for eveything                                                 *
 *  Adding space to every which would the concatenate the double arrays into a single array *
 *----------------------------------------------------------------------------------------*/
char* join_lines(char** lines, int size)
{
    int full_len = 0;
    int i = 0;
    char* retval;
    
    for (i=0;i<size;++i)
        if (lines[i] != NULL)
            full_len += strlen(lines[i]);
    
    retval = (char*)calloc(full_len + size,1);
    
    for (i=0;i<size;++i)
    {
        if (lines[i] == NULL)
            break;
        
        strcat(retval, lines[i]);
        if (i < size-1)
            strcat(retval, " ");
    }
    
    return retval;
}

/*---------------------------------------------------------------*
 *        Initialzing the enviroment for the shell                 *
 *  If the batch file is the value taken in then the BatchFile name. *
 *------------------------------------------------------------------*/
void shell_loop(){
    
    char line[LINE_SIZE];
    char **argv;
    int status = 0;
    pid_t pid;
    
    do{
        //Set Globals Cmd flags to 0 by default
        background=0;
        redi=0;
        std_in=0;
        std_out=0;
        std_out_append=0;
        flag_pipe=0;
        
        if(batch){
            //input from batch
            fgets(line,LINE_SIZE,batchfp);
            _pause(argv);
            //feof returns a nonzero if EOF is set
            if(feof(batchfp))
                break;
        }
        else{
            //prints the command prompt
            printf("%s",cmd_prompt);
            printf("/myshell> ");
            //Takes the input from the user
            fgets(line,LINE_SIZE,stdin);
            if(feof(stdin)){
                printf("\n");
                break;
            }
        }
        //Splits the the user input using split line/parser function.
        argv = split_line(line);
        //Lines split, now evaluation of the command flags.
        cmdflags(argv);
        //If theirs & at the in the input for background exec
        if(background){
            //fork the process
            pid = fork();
            // error checking the fork
            if(pid == -1)
                perror("FORK ERROR");
            //Child process
            if(pid == 0){
                //Execute argv in background
                status = _exe(argv);
                //Exit
                exit(1);
            }
        }else
            //execute argv
            status = _exe(argv);
        free(argv);
        //while status != 0 done.
    }while (status);
}


/*---------------------------------------------------------------*
 *        SPLITS LINE USING TOKEN DELIMS INIT GLOBALS              *
 *  If the batch file is the value taken in then the BatchFile name. *
 *------------------------------------------------------------------*/
char **split_line(char *line){
    //Holder to count num of args
    argv_count=0;
    char *arg;
    //Allocating space for the args
    char **argv = (char**) malloc(sizeof(char*)*TOK_SIZE);
    // if space isn't made we should be informed.
    if(argv == NULL){
        perror("Error Malloc");
        exit(EXIT_FAILURE);
    }
    //begin tokenizing
    arg = strtok(line, TOK_DELIMS);
    while (arg){
        //send token to argv
        argv[argv_count++] = arg;
        //check buffer overflow
        if (argv_count >= TOK_SIZE) {
            // Double tok_sze and place it into size
            int size = TOK_SIZE*2;
            // Reallocate the size of the token, and place into the args
            argv = realloc(argv, sizeof(char*)*size);
            //Error checkign for reallocation
            if(argv==NULL){
                perror("Error Realloc");
                //exit
                exit(EXIT_FAILURE);
            }
        }
        //get next token
        arg = strtok(NULL, TOK_DELIMS);
    }
    //End of argv and num_args++(# of argvs) for NULL
    argv[argv_count++] = NULL;
    return argv;
}

/*-------------------------------------------------------------*
 *        cmdFlags - Loops through to check I/O Flags           *
 *-------------------------------------------------------------*/
void cmdflags(char **argv){
    
    int pos = 0;
    if(argv_count>1){
        //Test background execution fisrt
        if (strcmp(argv[argv_count-2],"&")==0){
            //remove '&' symbol
            rmv_arg(argv,argv_count-2);
            //Set background execution to 1 --- 0 by default.
            background=1;
        }
        //Test for I/O redirection
        while (argv[pos] != NULL){
            
            //I/O redirection in '<'
            if(strcmp(argv[pos], "<")==0){
                //Global states Flags
                redi = 1;
                std_in = 1;
                std_in_index = pos;
            }
            
            //I/O redirection out '>'
            else if(strcmp(argv[pos], ">")==0){
                //Global state flags
                redi = 1;
                std_out =1;
                std_out_index = pos;
            }
            
            //I/O redirection out (appending)
            else if(strcmp(argv[pos], ">>")==0){
                redi = 1;
                //since append is still IO out flagged
                std_out = 1 ;
                std_out_append =1;
                std_out_index = pos;
            }
            
            //Piping '|'
            else if(strcmp(argv[pos], "|")==0){
                //Globals
                flag_pipe = 1;
                flag_pipe_index = pos+1;
                //to separate programs
                argv[pos]=NULL;
            }
            pos++;
        }
    }
}


/*-------------------------------------------------------------*
 *                  Remove Argv                                 *
 *-------------------------------------------------------------*/
void rmv_arg(char **argv, int index){
    
    while(index < argv_count-2){
        argv[index] = argv[index+1];
        index++;
    }
    argv[index]=NULL;
    argv_count--;
}

/*-------------------------------------------------------------*
 *                  Exce                                        *
 *-------------------------------------------------------------*/
int _exe(char **argv){
    int i;
    pid_t pid;
    int status = 0;
    //Flag if command is found
    int command_found = 0;
    
    if(flag_pipe){//if pipe flagged
        flag_pipe = 0;
        return(_pipe(argv));//execute with pipes
    }
    
    //Is CMD NULL
    if(argv[0]==NULL){
        puts("Type 'help' for more info\n");
        return 1;
    }
    
    // If argv isn't NULL then go through the internals to execute
    for (i=0;i<CMD_CNT;i++){
        if (strcmp(argv[0], internals[i]) == 0){
            command_found = 1;
            return (*internals_funcs[i])(argv);
        }
    }
    
    // If argv isn't NULL then go through the internals to execute
    for (i=0;i<CMD_IO_CNT;i++)
    {
        if (strcmp(argv[0], internalsIO[i]) == 0)
        {
            command_found = 1;
            if((pid = fork()) == -1)
                perror("I/O Redirection");
            if(pid == 0)
            {
                if(redi)
                {
                    //I/O out is flagged
                    if(std_out)
                    {
                        close(1);
                        printf("cd %s\n", argv[std_out_index+1]);
                        fopen(argv[std_out_index+1], "w");
                        //argv[io_out_index];
                        //execute command
                        (*internalsIO_funcs[i])(argv);
                        //return 1;
                    }
                    if(std_in)
                    {
                        close(0);
                        printf("cd %s\n", argv[std_out_index+1]);
                        fopen(argv[std_in_index+1], "w");
                        //argv[io_in_index];
                        //Execute command
                        (*internalsIO_funcs[i])(argv);
                        //return 1;
                    }
                    exit(1);
                }else
                    //Execute commands
                    return (*internalsIO_funcs[i])(argv);
                
            } else
                //Parent
                wait(&status);
            
            
        }
    }
     return prg_exe(argv);
    //Only allows built in function with this if state
//    if (command_found)
//    {
//        return prg_exe(argv);
//    }
//    else
//    {
//        printf("No such command\n");
//        return 1;
//    }
    
}
/*-------------------------------------------------------------*
 *                  Program Execute                             *
 *-------------------------------------------------------------*/
int prg_exe(char **argv){
    // Declate process
    pid_t pid;
    // Status holder
    int status;
    int newstdout;
    int newstdin = 0;
    
    //printf("DIRECTOUT INDEX %d\n", io_out_index);
    if((pid = fork()) < 0)
        //If forking failed
        perror("MyShell");
    //Child process
    if(pid == 0){
        //Get flags
        if(redi){//I/O redirection
            if(std_in){//I/O redirection in
                close(0);
                //Open file for reading
                fopen(argv[std_in_index+1], "r");
                argv[std_in_index] = NULL;
                //changed the input as file
                if( newstdin == -1) {
                    perror(argv[std_in_index]);
                    return 1;
                }
                else
                    //init input as file
                    dup2(newstdin,0);
            }
            if(std_out){
                close(1);
                //Truncate to zero length or create file for writing.
                fopen(argv[std_out_index+1], "w");
                argv[std_out_index] = NULL;
                if(std_out_append)//change output to file
                    newstdout = open(argv[std_out_index+1], O_WRONLY|O_CREAT|O_APPEND, S_IRWXU|S_IRWXG|S_IRWXO);
                else
                    newstdout = open(argv[std_out_index+1], O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
                
                if(newstdout == -1)
                    //Error Redirecting
                    perror("I/O Redirection Out");
                else
                    dup2(newstdout,1);//set output to file
            }
        }
        
        if (execvp(argv[0], argv) == -1){
            perror(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    else
        //PARENT
        wait(&status);
    return 1;
}

/*-------------------------------------------------------------*
 *                  Piping                                      *
 *-------------------------------------------------------------*/
int _pipe(char **argv) {
    /*
     pid_t pid1;//process 1
     pid_t pid2;//process 2
     // file des [ stdin, stdout, stderr ]
     int fd[2];
     // file des 2
     int fd_2[2];
     int temp_count = 0;
     int j,double_array_ind=0;
     char tmp_buf[10];
     pipe(fd);
     pipe(fd_2);
     //If foek is -1 error
     if((pid1 = fork()) == -1) {
     perror("fork1");
     return 1;
     }
     if((pid2 = fork()) == -1) {
     perror("fork2");
     return 1;
     }
     //process 1
     if(pid1 == 0) {
     //open write end and make it the new write fd
     dup2(fd[1],1);
     //char* data = join_lines(argv,argv_count);
     //write into fd
     //write(fd[1],data,strlen(data));
     //close the fd
     _exe(argv+flag_pipe_index);
     close(fd[0]);
     //free data in the fd,
     free(data);
     
     exit(1);
     }
     // else if (pid1 > 0)
     //{
     //   char read_buf[100] = {0};
     //char** proc1_argv;
     //debug
     // printf("Parent proc");
     
     // wait(NULL);
     
     // read(fd[0],read_buf,100);
     
     // proc1_argv = split_line(read_buf);
     
     //   _exe(pargv);
     //printf(str);
     //}
     
     if(pid2 == 0) {
     //open read,and apply it as the new read file descryptor
     dup2(fd_2[1], 2);
     char* data = join_lines(argv+(flag_pipe_index),argv_count-flag_pipe_index);
     //   write(fd_2[1],data,strlen(data));
     close(fd_2[1]);
     free(data);
     //close(fd[1]);//close write end
     //_exe(argv+flag_pipe_index);//execute program to the right of |
     exit(1);
     }
     else if (pid2 > 0)
     {
     //parent proc
     char read_buf[100] = {0};
     char** proc2_argv;
     //debug
     printf("Parent proc");
     
     wait(NULL);
     
     read(fd_2[0],read_buf,100);
     proc2_argv = split_line(read_buf);
     
     _exe(proc2_argv);
     }
     
     close(fd[0]);//Close stdin
     close(fd[1]);//Close stdout
     close(fd_2[0]);
     close(fd_2[1]);
     
     return 1;
     */
    pid_t pid1;//process 1
    pid_t pid2;//process 2
    
    int fd[2];
    
    pipe(fd);
    
    if((pid1 = fork()) == -1) {
        perror("fork1");
        return 1;
    }
    //For process 1
    if(pid1 == 0) {
        //dup2 opens, and writes end and makes in the new write fd
        dup2(fd[1],1);
        //closes read
        close(fd[0]);
        //execute program to the left of the pipe
        _exe(argv);
        exit(1);
    }
    
    if((pid2 = fork()) == -1) {
        perror("fork2");
        return 1;
    }
    
    if(pid2 == 0) {
        //Opens the read and makes it into the new fd read
        dup2(fd[0], 0);
        close(fd[1]);
        //execute program to the right half fo the pipe
        _exe(argv+flag_pipe_index);
        exit(1);
    }
    //Close fd read
    close(fd[0]);
    //Close fd write proc
    close(fd[1]);
    
    return 1;
}
/*-------------------------------------------------------------*
 *                         CD                                   *
 *-------------------------------------------------------------*/
int _cd(char **argv){
    //check for argument first
    if(argv[1]==NULL)
        printf("MyShell: too few arguments\n");
    else{
        //Change directory
        if(chdir(argv[1])==0){
            //change cwd env
            environ.cwd = getcwd(NULL,0);
            cmd_prompt = (char *) malloc(sizeof(char)*strlen(environ.cwd)+10);
            strcpy(cmd_prompt,environ.cwd);
            free(cmd_prompt);
        }
        else
            perror(argv[1]);
    }
    return 1;
}

/*-------------------------------------------------------------*
 *                      Clear                                   *
 *-------------------------------------------------------------*/
int _clr(char **argv){
    printf("\033[2J\033[H");
    return 1;
}

/*-------------------------------------------------------------*
 *         Make Directory                                       *
 *-------------------------------------------------------------*/
int _mkdir(char** argv){
    if (argv[1] == NULL)
    {
        printf("MyShell: too few arguments\n");
    }
    else
    {
        struct stat st = {0};
        //Make sure the directory does not already exist
        if (stat(argv[1],&st) == -1)
        {
            mkdir(argv[1], 0777);
            return 1;
        }
        else
        {
            printf("MyShell: directory already exists");
            return 1;
        }
    }
    return 1;
}

/*-------------------------------------------------------------*
 *         List Directory                                       *
 *-------------------------------------------------------------*/
int _dir(char **argv){
    
    //Declare a pointer to the dir
    DIR *dp = NULL;
    //pointer to dirent struct found within readdir()
    struct dirent *dir;
    //lists currrent working directory
    if(argv[1]==NULL){

        dp = opendir(environ.cwd);
        if(dp==NULL)
            perror("MyShell");
        else{
            while((dir=readdir(dp))){
                //skip it
                if(strcmp(dir->d_name,".")==0 || strcmp(dir->d_name,"..")==0);
                //continue;
                else
                    //Display the cwd
                    printf("%s\n",dir->d_name);
            }
        }
    }
    closedir(dp);
    return 1;
    /*
     
     // Declare a pointer to the dir
     DIR *dp = NULL;
     //pointer to dirent struct found within readdir()
     struct dirent *dir;
     
     char *in = argv[1];
     
     if (dp = opendir(target) == NULL) {
     fprintf(stderr, "MyShell %s\n", env
     }
     if (dp != NULL) {
     while ((dir = readdir(dp))) {
     if (strcmp(dir->d_name,".")==0 || strcmp(dir->d_name,"..")==0)
     continue;
     printf("%s \n", dir->d_name);
     
     }
     closedir(dp);
     return 1;
     }
     */
}

/*-------------------------------------------------------------*
 *         Echo Directory                                       *
 *-------------------------------------------------------------*/
int _echo(char **argv){
    //If nothing is Entered print a new line
    if (argv[1] == NULL)
        fprintf(stdout, "\n");
    else {
        // If something is entered print what is entered
        for (int i = 1; argv[i] != NULL; i++)
            printf("%s ", argv[i]);
        //new line at the end
        printf("\n");
    }
    return 1;
}

/*-------------------------------------------------------------*
 *          Environ Directory                                   *
 *-------------------------------------------------------------*/
int _environ(char **argv){
    
    printf("PWD=%s\n", environ.cwd);
    printf("DIR=%s\n", environ.dir);
    printf("SHELL=%s\n", environ.shell);
    
    return 1;
}

/*-------------------------------------------------------------*
 *                       Help                                   *
 *-------------------------------------------------------------*/
int _help(char **argv){
    // File pointer for the steam
    FILE *fp;
    int c;
    char sys_call[1024];
    // Grabs the readme in read only mode.
    fp = fopen(strcat(environ.dir,"/readme"), "r");
    // Reads the characters until End of File
    while(( c = fgetc(fp) ) != EOF) {
        fputc(c, stdout);
    }
    strcpy(sys_call,"more -d ");
    strcat(sys_call,environ.dir);
    strcat(sys_call,"/readme");
    system(sys_call);
    return 1;
}

/*-------------------------------------------------------------*
 *                  Pause Shell                                 *
 *-------------------------------------------------------------*/
int _pause(char **argv){
    //Lets the user know to press enter due the pause
    printf("Press 'Enter' to continue");
    //While character entered isn't a new line.
    while (getchar() != '\n');
    
    return 1;
}

/*-------------------------------------------------------------*
 *                      Quit                                    *
 *-------------------------------------------------------------*/
int _quit(char **argv){
    //Exit program return 0;
    exit(EXIT_FAILURE);
}
