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

int throw_err(const char *errmsg){
	// perror(errmsg);		// uncomment to see written printf error msgs
	const char error_message[30] = "An error has occured \n";
	write(STDERR_FILENO, error_message, strlen(error_message));

	// choose one
	exit(1);
	return -1;
}

// parses cmdline into tokens in sh_argv
int tokenize_input(char *cmdline, char **sh_argv){

	int sh_argc = 0;
	char *token = strtok(cmdline, DELIMITER);
	while (token != NULL){
		sh_argv[sh_argc++] = token;
		token = strtok(NULL, DELIMITER);
	}
	sh_argv[sh_argc] = NULL;
	return sh_argc;
}

// Section for built-in commands

char *sh_builtin[]={
	"cd",
	"pwd",
	"wait",
	"exit",
	"help"
};

int sh_num_builtins(){
	return (sizeof(sh_builtin) / sizeof(char *));
}

int sh_cd(char **argv){
	if (argv[1] == NULL){
		chdir(getenv("HOME"));
		return 0;
	}
	if (chdir(argv[1]) != 0){
		return throw_err("chdir");
	}
	return 0;
}

int sh_pwd(){
	char cwd[1024];
    getcwd(cwd, sizeof(cwd));
	write(STDOUT_FILENO, cwd, 1024);
	return 0;
}

int sh_wait(){
	// wait for any child with same process group to terminate
	waitpid(-1, NULL, 0);
	return 0;
}

int sh_exit(){
	exit(0);
}

int sh_help(){
	for (int i=0; i<sh_num_builtins(); i++){
		printf("%s\n", sh_builtin[i]);
	}
	return 0;
}

int (*sh_builtin_func[]) (char **) = {
	&sh_cd,
	&sh_pwd,
	&sh_wait,
	&sh_exit,
	&sh_help
};

// End section

int doexec(int sh_argc, char **sh_argv){
	for (int i=0; i<sh_num_builtins(); i++){
		if (strcmp(sh_argv[0], sh_builtin[i]) == 0){
			return (*sh_builtin_func[i])(sh_argv);
			}
	}

	int is_background = 0;
	if (strcmp(sh_argv[sh_argc-1], "&") == 0){
		is_background = 1;
		sh_argv[sh_argc-1] = NULL;
	}

	// if not built-in
	pid_t pid = fork();
	switch(pid){
		case -1:
			return throw_err("fork");
			break;
		case 0:
			if (execvp(sh_argv[0], sh_argv) == -1){
				write(STDOUT_FILENO, "execvp\n", 10);
				return throw_err("execvp, invalid cmd");
			}
		default:
			if (!is_background){
				waitpid(pid, NULL, 0); break;
			}
	}

	return 0;
}


int main( int argc, char ** argv ){
	char cmdline[MAX_CMD_LEN];
	char *sh_argv[MAX_SH_ARG];

	switch(argc){
		case 1:
			printf("Interactive mode\n");
			for (int status = 0; status != -1;){
				write(STDOUT_FILENO, "> ", 3);
				fgets(cmdline, MAX_CMD_LEN, stdin);
				int sh_argc = tokenize_input(cmdline, sh_argv);
				status = doexec(sh_argc, sh_argv);
				// printf("main status = %d\n", status);
			}
			break;
		case 2:
			;
			printf("Batch mode\n");
			FILE *file_ptr = fopen(argv[1], "r");
			while (fgets(cmdline, MAX_CMD_LEN, file_ptr)){
				// write(STDOUT_FILENO, cmdline, strlen(cmdline));
				int sh_argc = tokenize_input(cmdline, sh_argv);
				
				if (doexec(sh_argc, sh_argv) == -1){
					// return throw_err("batch status");
					break;
				}
			}
			break;
		default:
			return throw_err("Usage: ./myshell [batchFile]");
			break;
	}

	throw_err("exit status");
	return 0;
}