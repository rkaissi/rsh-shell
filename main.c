#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void *Malloc(int size) {
    void *ptr;

    if (size == 0) return NULL;
    ptr = malloc(size);
    if (ptr == NULL) {
        perror("Malloc Failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *Realloc(void *block, int size) {
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

int main(void) {
    char *line = NULL;
    size_t len = 0;
    int ret = 0;

    while (1) {
        printf("$ ");
        fflush(stdin);
        ret = getline(&line, &len, stdin);
        if (ret == -1) exit(EXIT_FAILURE);

        char **argv = split_line(line);

        for (int i = 0; argv[i] != NULL; i++) {
            printf("%s\n", argv[i]);
        }
        free(argv);
    }
    
    free(line);
    return EXIT_SUCCESS;
}