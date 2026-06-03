#ifndef RSH_H
#define RSH_H

#define DELIM " \t\r\n\a"
#define MIN_LISTSIZE 4
#define TOKEN_BUFFERSIZE 1024

#define DEFAULT_BUFFERSIZE 256

#define HISTSIZE 1000
#define HISTORY_FILE "/.rsh_history"

#define RSHRC_FILE "/.rshrc"

#define MAX_ALIAS_COUNT 256

typedef struct {
    char *key;
    char *value;
} Alias;

int execute_script(char *path);

#endif
