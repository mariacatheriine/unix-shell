#include <stdio.h>
#include <stdlib.h>

#define USH_BUFF_SIZE 1024
#define USH_TOK_BUFF_SIZE 64
#define DELIM " \t\r\n\a"

int main(int argc, char **argv) {
    // looping function
    ush_loop();

    // exit 
    return EXIT_SUCCESS;
}

// function to loop and execute commands
void ush_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = ush_read_line();
        args = ush_split_line(line);
        status = ush_execute(args);

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