/* main.c
 * ----------------------------------------------------------
 *  CS350
 *  Midterm Programming Assignment
 *
 *  Purpose:  - Use Linux programming environment.
 *            - Review process creation and management
 * ----------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>	

#define MAX_CMD_LEN 512
#define MAX_SH_ARG 64

#define DELIMITER " \t\n"
void parse_input(char **sh_argv, FILE *line){
	char cmdline[MAX_CMD_LEN];
	fgets(cmdline, MAX_CMD_LEN, line);
	// system(cmdline);

	int sh_argc = 0;
	char *token = strtok(cmdline, DELIMITER);
	while (token != NULL){
		sh_argv[sh_argc++] = token;
		token = strtok(NULL, DELIMITER);
	}
	sh_argv[sh_argc] = NULL;

	// printf("DEB debug\n");
	// for (int i=0; i<sh_argc; i++){
	// 	printf("DEB sh_arvg[%d]: %s\n", i, sh_argv[i]);
	// }
}

void doexec(char **sh_argv){
	execvp(sh_argv[0], sh_argv);
	// perror("execvp error");				// should never reach here
	exit(1);
}

int main( int argc, char ** argv ){
	char *sh_argv[MAX_SH_ARG];

	for (;;){
		printf("> ");
		parse_input(sh_argv, stdin);

		pid_t pid = fork();
		switch(pid){
			case -1:
				// perror("fork");
				break;
			case 0:
				doexec(sh_argv);
			default:
				waitpid(pid, NULL, 0); break;
		}

	}

	return 0;
}