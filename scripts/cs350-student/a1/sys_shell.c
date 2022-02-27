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


int main(){
    for (;;){
        // prompt $
        // printf("$");
        char *line_sys = NULL;
        size_t len = 0;
        getline(&line_sys, &len, stdin);
        system(line_sys);
        free(line_sys);
    }

}
