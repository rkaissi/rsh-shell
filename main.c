#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void *Malloc(size_t size) {
    void *ptr;

    if (size == 0) return NULL;
    ptr = malloc(size);
    if (ptr == NULL) {
        perror("Malloc Failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *Realloc(void *block, size_t size) {
    void *ptr;

    if (size == 0) return NULL;
    ptr = realloc(block, size);
    if (ptr == NULL) {
        perror("Realloc Failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
} 

#define DELIM " \t\r\n\a"
#define BUFFERSIZE 64

char **split_line(char *line) {
    int bufferSize = BUFFERSIZE;
    char **tokens = Malloc(bufferSize * sizeof(char *));
    char *token;
    int pos = 0;

    token = strtok(line, DELIM);

    while (token) {
        tokens[pos++] = token;
        token = strtok(NULL, DELIM);

        if (pos >= bufferSize) {
            bufferSize += BUFFERSIZE;
            tokens = Realloc(tokens, bufferSize * sizeof(char *));
        }
    }

    tokens[pos] = NULL;
    return tokens;
}

void execute(char**argv) {
    if (argv[0] == NULL) {
        return;
    }
    
    if (strcmp(argv[0], "cd") == 0 && argv[1] != NULL) {
        if (chdir(argv[1]) != 0) {
            perror("Failed to change directory");
        }
        return;
    } else if (strcmp(argv[0], "exit") == 0) {
        printf("\nexit");
        exit(EXIT_SUCCESS);
    }

    pid_t pid = fork();
    int status;

    if (pid == 0) { // child
        execvp(argv[0], argv);
        perror(argv[0]);
        exit(EXIT_FAILURE);
    } else if (pid == -1) { // fork failed
        perror("Failed to run process");
    } else { // parent, pid = child's pid
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status)) {
            perror("Process termination error");
        }
    }
}

int main(void) {
    char *line = NULL;
    size_t len = 0;
    int ret = 0;

    while (1) {
        printf("$ ");
        fflush(stdout);
        ret = getline(&line, &len, stdin);
        if (ret == -1) {
            if (ferror(stdin)) {
                perror("Error!");
                exit(EXIT_FAILURE);
            } else if (feof(stdin)) {
                printf("\nlogout\n");
                break;
            }
        }

        char **argv = split_line(line);
        execute(argv);
        free(argv);
    }
    
    free(line);
    return EXIT_SUCCESS;
}