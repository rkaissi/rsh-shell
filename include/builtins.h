#ifndef BUILTINS_H
#define BUILTINS_H

#include "rsh.h"

#define ERROR_CODES 255

typedef struct {
    const char *name;
    const char *description;
    const char *usage;
    int (*func)(char **argv);
} Builtin;

extern const Builtin builtins[];
extern const int builtin_count;

int builtin_cd(char **argv);
int builtin_exit(char **argv);
int builtin_alias(char **argv);
int builtin_unalias(char **argv);
int builtin_source(char **argv);

extern char history_path[HISTORY_PATH_BUFFERSIZE];
extern Alias aliases[MAX_ALIAS_COUNT];
extern int alias_count;

#endif
