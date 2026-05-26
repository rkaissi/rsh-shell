#include <stdlib.h>
#include <stdio.h>

int main(void) {
    char *line = NULL;
    size_t len = 0;

    while (1) {
        printf("$ ");
        if (!getline(&line, &len, stdin)) break;

        printf("%s", line);
    }
    
    free(line);
    return EXIT_SUCCESS;
}