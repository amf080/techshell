// Name: Aaron Fore
// Description: C Shell


#include <stdio.h> // printf, perror, getline,
#include <unistd.h> // getcwd, 
#include <stdlib.h> // free, 
#include <sys/wait.h> // wait,
#include <string.h> // strtok, strcmp,strdup,strerror
#include <errno.h> // errno
#include <fcntl.h> // for file opening and certain options


// A function that causes the prompt to display in the terminal
void prompt() {
	char *cwd = getcwd(NULL, 0); // allocates memory for the directory
	if (!cwd) {
		perror("getcwd() failed!\n");
		exit(1);
	}
	printf("%s$ ", cwd); // prints the prompt with the $ and space
	free(cwd); // frees the memory reserved for the directory
}

char* get() {
	char *input = NULL; // NULL so getline() allocates the memory itself
	size_t buffer = 0; // buffer size doesn't matter
	ssize_t charror; // to see if getline() didn't work, it retuns a ssize_t?
	charror = getline(&input, &buffer, stdin); // dynamically assigns the line to input and returns possible error
	if (charror == -1) {
		perror("getline() failed!\n");
		exit(1);
	}
	if (input[charror - 1] == '\n') { // remove the newline from the getline()
		input[charror - 1] = '\0';
	}
	return input;
}

// A function that parses user input
char** parse(char *input) {
	if (input[0] == '\0') { // return an empty array if theres nothing
		char **array = malloc(sizeof(char*));
		array[0] = NULL;
		return array;
	}
	int args = 0; // counts the amount of arguments
	char *temp = strdup(input); // copy to count the arguments without modifying
	char *arg = strtok(temp, " "); // start at beginning 
	while (arg != NULL) { // go until no more tokens
		args += 1;
		arg = strtok(NULL, " "); // start at next token
	}
	free(temp); // free the copy
	char **array = malloc((args + 1) * sizeof(char*)); // +1 for the NULL for execvp
	int i = 0;
    arg = strtok(input, " "); 
    while (arg != NULL) {
        array[i++] = arg;
        arg = strtok(NULL, " ");
    }
    array[i] = NULL; // set the NULL for execvp
	return array;
}

// A function that executes the command
void execute(char **parsed) {
	int error;
	if (parsed[0] == NULL) { // no command
		return;
	}
	else if (strcmp(parsed[0], "cd") == 0) { // ls command
		if (parsed[1] == NULL) { // make sure a directory is targeted
			perror("No directory argument");
			return;
		}
		error = chdir(parsed[1]); // changes directory to the argument, returns an int for error
		if (error == -1) {
			printf("Error %d (%s)\n", errno, strerror(errno));
		}
	}
	else if (strcmp(parsed[0], "exit") == 0) { // exit command
		exit(0);
	}
	else {
		int pid = fork(); // fork because execvp takes over the process
		if (pid == -1) {
			perror("fork failed!");
			return;
		}
		if (pid == 0) { // do any other command with any arguments in the child
			// child
			int i = 0;
			while (parsed[i] != NULL) {
				if (strcmp(parsed[i], ">") == 0) {
					int fd = open(parsed[i+1], O_CREAT | O_WRONLY | O_TRUNC, 0644); // if standard out open/create the next file in r/w only and truncate with octal 644 perms
					dup2(fd, STDOUT_FILENO); // dup2 copies any standard output that happens to the opened file
					close(fd); // close the file
					parsed[i] = NULL; // get rid of redirect for execvp
				}
				else if (strcmp(parsed[i], "<") == 0) {
					int fd = open(parsed[i+1], O_RDONLY); // if standard in open the next file in read only
					dup2(fd, STDIN_FILENO); // dup2 copies any standard input that happens to the opened file
					close(fd); // close the file
					parsed[i] = NULL; // get rid of the redirect for execvp
				}
				i++;
			}
			error = execvp(parsed[0], parsed);
			if (error == -1) {
				printf("Error %d (%s)\n", errno, strerror(errno));
			}
		}
		else {
			// parent
			wait(NULL); // doesn't do anything just lets the child execute the command
		}
	return;
	}
}

int main() {
	while (1) {
		prompt();
		char *input = get();
		char **parsed = parse(input);
		execute(parsed);
		free(input); // free input from when get() was called
		free(parsed); // free parsed from when parse() was called
	}
	return 0;
}
