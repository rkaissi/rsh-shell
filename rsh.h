#ifndef RSH_H
#define RSH_H

#define DELIM " \t\r\n\a"
#define BUFFERSIZE 64
#define HISTORY_PATH_BUFFERSIZE 256
#define HISTORY_FILE "/.rsh_history"

#define MAX_ALIAS_COUNT 256

typedef struct {
    char *key;
    char *value;
} Alias;

#endif
