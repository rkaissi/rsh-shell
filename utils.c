#include "utils.h"

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
