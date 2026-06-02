#ifndef BUILTINS_H
#define BUILTINS_H

#include "rsh.h"

int builtin_cd(char **argv);
int builtin_exit(char **argv);
int builtin_alias(char **argv);
int builtin_unalias(char **argv);
int builtin_source(char **argv);

extern char history_path[HISTORY_PATH_BUFFERSIZE];
extern Alias aliases[MAX_ALIAS_COUNT];
extern int alias_count;

#endif
