#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define USH_BUFF_SIZE 1024
#define USH_TOK_BUFF_SIZE 64
#define DELIM " \t\r\n\a"

typedef struct {
    char **args1;
    char **args2;
} ush_pipe_args;

void ush_loop(void);
char *ush_read_line(void);
char **ush_split_line(char *line);
int ush_launch(char **args);
int ush_execute(char **args);
int ush_execute_piped(char **args1, char **args2);
int ush_cd(char **args);
int ush_help(char **args);
int ush_exit(char **args);
int ush_has_pipe(char **args);
ush_pipe_args ush_split_pipe(char **args);

int main(int argc, char **argv) {
    // looping function
    ush_loop();

    // exit 
    return EXIT_SUCCESS;
}

// function to loop commands
void ush_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = ush_read_line();
        args = ush_split_line(line);
        if (ush_has_pipe(args)) {
            ush_pipe_args pa = ush_split_pipe(args);
            status = ush_execute_piped(pa.args1, pa.args2);
        }
        else {
            status = ush_execute(args);
        }

        free(line);
        free(args);
    } while (status);
}

// function to read a line of input
char *ush_read_line(void) {
    int bufsize = USH_BUFF_SIZE;
    char *buffer = malloc(sizeof(char) * bufsize);
    int position = 0;
    int c;
    if (!buffer) {
        fprintf(stderr, "ush: allocation error\n");
        exit(EXIT_FAILURE);
    }
    while (1) {
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        }
        else {
            buffer[position] = c;
        }
        position++;
        if (position >= bufsize) {
            bufsize += USH_BUFF_SIZE;
            char *new_buffer = realloc(buffer, bufsize * sizeof(char));
            if (!new_buffer) {
                free(buffer);  
                exit(EXIT_FAILURE);
            }
            buffer = new_buffer;
        }
    }
}

// function to parse a line and split it into tokens
char **ush_split_line(char *line) {
    int bufsize = USH_TOK_BUFF_SIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    if (!tokens) {
        fprintf(stderr, "ush: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        if (position >= bufsize) {
            bufsize += USH_TOK_BUFF_SIZE;
            char **new_tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!new_tokens) {
                free(tokens);
                exit(EXIT_FAILURE);
            }
            tokens = new_tokens;
        }
        token = strtok(NULL, DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

// function to start a program
int ush_launch(char **args) {
    pid_t pid, wpid;
    int status;
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0;

    // check for input/output redirection
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
            output_file = args[i + 1];
            args[i] = NULL;
        }
        else if (strcmp(args[i], ">>") == 0) {
            output_file = args[i + 1];
            append = 1;
            args[i] = NULL;
        }
        else if (strcmp(args[i], "<") == 0) {
            input_file = args[i + 1];
            args[i] = NULL;
        }
        i++;
    }

    pid = fork();
    if (pid == 0) {
        if (output_file != NULL) {
            int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
            int fd = open(output_file, flags, 0644);
            if (fd == -1) {
                perror("ush");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (input_file != NULL) {
            int fd = open(input_file, O_RDONLY);
            if (fd == -1) {
                perror("ush");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (execvp(args[0], args) == -1) {
            perror("ush");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        perror("ush");
    }
    else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == -1) {
                perror("ush");
                exit(EXIT_FAILURE);
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

// list of built-in commands and their corresponding functions
char *builtin_str[] = {"cd", "help", "exit"};
int (*builtin_func[]) (char**) = {&ush_cd, &ush_help, &ush_exit};

// function to return the number of built-in commands
int ush_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

// cd implementation
int ush_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "ush: expected argument to \"cd\"\n");
    }
    else {
        if (chdir(args[1]) != 0) {
            perror("ush");
        }
    }
    return 1;
}

// help implementation
int ush_help(char **args) {
    int i;
    printf("Unix Shell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");
    for (i = 0; i < ush_num_builtins(); i++) {
        printf("%s\n", builtin_str[i]);
    }
    return 1;
}

// exit implementation
int ush_exit(char **args) {
    return 0;
}

// function to execute a command
int ush_execute(char **args) {
    if (args[0] == NULL) {
        return 1;
    }
    
    int i;
    for (i = 0; i < ush_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return ush_launch(args);
}

// function to check if a command has a pipe
int ush_has_pipe(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0) {
            return 1;
        }
        i++;
    }
    return 0;
}

// function to execute a piped command
int ush_execute_piped(char **args1, char **args2) {
    int fds[2];
    pid_t pid1, pid2;
    if (pipe(fds) == -1) {
        perror("ush");
        exit(EXIT_FAILURE);
    }
    pid1 = fork();
    if (pid1 == 0) {
        dup2(fds[1], STDOUT_FILENO);
        close(fds[0]);
        close(fds[1]);
        execvp(args1[0], args1);
        perror("ush");
        exit(EXIT_FAILURE);
    }
    pid2 = fork();
    if (pid2 == 0) {
        dup2(fds[0], STDIN_FILENO);
        close(fds[0]);
        close(fds[1]);
        execvp(args2[0], args2);
        perror("ush");
        exit(EXIT_FAILURE);
    }
    close(fds[0]);
    close(fds[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 1;
}

// function to split a piped command
ush_pipe_args ush_split_pipe(char **args) {
    ush_pipe_args result;
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;
            break;
        }
        i++;
    }
    result.args1 = args;
    result.args2 = &args[i + 1];
    return result;
}

