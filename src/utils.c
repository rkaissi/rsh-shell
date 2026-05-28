#include "utils.h"

#include <string.h>
#include <errno.h>

void *Malloc(size_t size) {
    void *ptr;

    if (size == 0) return NULL;
    ptr = malloc(size);
    if (ptr == NULL) {
        err("Malloc Failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *Realloc(void *block, size_t size) {
    void *ptr;

    if (size == 0) return NULL;
    ptr = realloc(block, size);
    if (ptr == NULL) {
        err("Realloc Failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void err(const char *msg) {
    if (msg && *msg)
        fprintf(stderr, RED "%s: %s\n" RST, msg, strerror(errno));
    else
        fprintf(stderr, RED "%s\n" RST, strerror(errno));
}
