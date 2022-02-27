#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <wait.h>

// #define _GNU_SOURCE
#include <stdio.h>

#define MAXSIZE 1000
#define MAXARGS 100

int parse_input(char *line, char **av){
    ssize_t len = 0;
    int retval = getline(&line, &len, stdin);
    if (retval == -1){
        if (!feof(stdin)){
            perror("getline error");
            return -1;
        }
    }

    // printf("len: %d\nline: %s\n", retval, line);
    // if (retval == 1){
    //     perror("here!!\n");
    //     return 0;
    // }

    int nargs = 0;
    const char *delim = " ";
    char* token;

    for (token = strtok(line, delim); token!=NULL; nargs++){
        av[nargs] = token;
        token = strtok(NULL, delim);
    }
    // printf("nargs is %d\n", nargs);
    av[nargs] = NULL;

    return 1;
}

int doexec(char **av){

    // if (av[0]==NULL){
    //     return 1;
    // }
    
    execvp(av[0], av);        // after fork
    
    // should never execute, only returns here if failed execvp
    perror("failed execvp");
    return -1;
}


int main(){
    char line[MAXSIZE];        // char *line
    char *av[MAXARGS];         // char **av
    pid_t pid;
    int status = 1;

    for (;;){
        // prompt $
        printf("$ ");
        
        status = parse_input(line, av);
        
        // if (status <= 0){
        //     if (status == -1) { perror("error parse_input"); }
        //     // 0 is if empty input
        //     continue;
        // }
        
        // sanity check
        // printf("DEBUG: sanity check, print av");
        // for (int i=0; av[i]; i++){
        //     printf("\nav[%d]: %s", i, av[i]);
        // }

        switch(pid = fork()){
        case -1:
            perror("failed fork"); break;
            
        case 0:     // child
            // perror("currently doing child exec\n");
            status = doexec(av);

        default:    // parent
            waitpid(pid, NULL, 0); break;
        }

        if (status != 1){
            break;
        }

        // char *line_sys = NULL;
        // size_t len = 0;
        // getline(&line_sys, &len, stdin);
        // system(line_sys);
        // free(line_sys);
    }

}
