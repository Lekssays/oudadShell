#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAX_BUFFER_SIZE 1024
#define MAX_TOKEN_BUFFER_SIZE 64
#define TOKEN_DELIMITERS " \t\r\n\a"

int cd(char **args);
int help(char **args);
int du(char **args);
int vi(char **args);
int nano(char **args);
int bye(char **args);

char *builtinsCommands[] = {
  "cd",
  "help",
  "vi",
  "du",
  "nano",
  "bye"
};

int (*builtinFunctions[]) (char **) = {
  &cd,
  &help,
  &vi,
  &du,
  &nano,
  &bye
};

int numBuiltins(void) {
  return sizeof(builtinsCommands) / sizeof(char *);
}

int cd(char **args) {
	if (args[1] == NULL) {
		fprintf(stderr, "oudad: expected argument to \"cd\"\n");
	} else {
		if (chdir(args[1]) != 0) {
			perror("oudad");
		}
	}
	return 1;
}

int bye(char **args) {
	return EXIT_SUCCESS;
}

int help(char **args) {
	int i;
	printf("Welcome to oudadShell\n");
	printf("USAGE: command argument\n");
	printf("Here are the available commands:\n");
	for(i = 0; i < numBuiltins(); i++) {
		printf("  %s\n", builtinsCommands[i]);
	}
	return 1;
}

int du(char **args) {
	if(args[1] == NULL) {
		char *exec;
		char dir[20] = "*", cmd[20] = "du -h ";
		if((exec = malloc(strlen(cmd) + strlen(dir) + 1) ) != NULL) {
	    		exec[0] = '\0';  
	    		strcat(exec, cmd);
	    		strcat(exec, dir);
	    		int state = system(exec);
	    		return state;
		} else {
			fprintf(stderr,"oudad: malloc failed!\n");
			return -1;
		}
	} else {
		char *exec;
		char cmd[20] = "du -h ";
		if((exec = malloc(strlen(cmd) + strlen(args[1]) + 1) ) != NULL) {
	    		exec[0] = '\0'; 
	    		strcat(exec, cmd);
	    		strcat(exec, args[1]);
	    		int state = system(exec);
	    		return state;
		} else {
	    		fprintf(stderr,"oudad: malloc failed!\n");
	    		return -1;
		}
	}
}

int vi(char **args) {
	int state = system("vi");
	return state;
}

int nano(char **args) {
	int state = system("nano");
	return state;
}

char *oudadRead(void) {
	char *command = NULL;
	ssize_t bufferSize = 0;
	getline(&command, &bufferSize, stdin);
	return command;
}

char **oudadParse(char* command) {
	int bufferSize = MAX_TOKEN_BUFFER_SIZE, pos = 0;
	char** tokens = malloc(bufferSize * sizeof(char*));
	char* token;
	if(!tokens) {
		fprintf(stderr, "oudad: I could not allocate memory for you :(\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(command, TOKEN_DELIMITERS);
	while(token != NULL) {
		tokens[pos] = token;
		pos++;
		if(pos >= bufferSize) {
			bufferSize += MAX_TOKEN_BUFFER_SIZE;
			tokens = realloc(tokens, bufferSize * sizeof(char*));
			if(!tokens) {
				fprintf(stderr, "oudad: I could not allocate memory for you :(\n");
				exit(EXIT_FAILURE);
			}		
		}
		token = strtok(NULL, TOKEN_DELIMITERS);
	}
	tokens[pos] = NULL;
	return tokens;
}

int oudadStart(char** args) {
	int state;
	pid_t pid, wpid;
	pid = fork();
	if (pid == 0) {
		if (execvp(args[0], args) == -1) {
		  perror("oudad");
		}
    	exit(EXIT_FAILURE);
  	} else if (pid < 0) {
    	perror("oudad");
  	} else {
	    do {
	      wpid = waitpid(pid, &state, WUNTRACED);
	    } while (!WIFEXITED(state) && !WIFSIGNALED(state));
  	}
  	return 1;
}

int oudadExecute(char **args) {
    int i;
        if (args[0] == NULL) {
            return 1;
        }
    for (i = 0; i < numBuiltins(); i++) {
        if (strcmp(args[0], builtinsCommands[i]) == 0) {
            return (*builtinFunctions[i])(args);
        }
    }
    return oudadStart(args);
}

void oudad(void) {
	char* command;
	char** args;
	int state = 1;
	while(state) {
		printf("oudad$ > ");
		command = oudadRead();
		args = oudadParse(command);
		state = oudadExecute(args);

		free(command);
		free(args);
	}
}

int main(int argc, char const *argv[]) {
	oudad();
	return EXIT_SUCCESS;
}
