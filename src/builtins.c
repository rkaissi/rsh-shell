#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <readline/history.h>

#include "builtins.h"
#include "utils.h"

int builtin_cd(char **argv)  {
    if (argv[1] == NULL) {
        argv[1] = getenv("HOME");
    }

    if (chdir(argv[1]) != 0) {
        err(argv[1]);
        return 1;
    }
    return 0;
}

int builtin_exit(char **argv) {
    printf(RED"[Exit]\n"RST);
    write_history(history_path);
    exit(EXIT_SUCCESS);
    return 0;
}

int builtin_alias(char **argv) {
    if (argv[1] == NULL) {
        for (int i = 0; i < alias_count; i++) {
            printf("alias %s='%s'\n", aliases[i].key, aliases[i].value);
        }
        return 0;
    }

    if (alias_count >= 256) {
        fprintf(stderr, RED"Exceeded max alias count of %d\n"RST, MAX_ALIAS_COUNT);
        return 1;
    }

    char *delimPtr = strchr(argv[1], '=');

    if (delimPtr == NULL) {
        fprintf(stderr, RED"Alias not assigned correctly\n"RST);
        return 2;
    }
    *delimPtr = '\0';
    char *key = argv[1];
    char *val = delimPtr + 1;

    for (int i = 0; i < alias_count; i++) {
        if (aliases[i].key && strcmp(aliases[i].key, argv[1]) == 0) {
            free(aliases[i].key);
            free(aliases[i].value);
            aliases[i] = (Alias){strdup(key), strdup(val)};
            return 0;
        }
    }

    aliases[alias_count++] = (Alias){strdup(key), strdup(val)};
    return 0;
}

int builtin_unalias(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, RED"%s: usage: unalias [-a] name [name ...]\n"RST, argv[0]);
        return 2;
    }

    // Unalias all
    if (strcmp(argv[1], "-a") == 0) {
        for (int i = 0; i < alias_count; i++) {
            free(aliases[i].key);
            free(aliases[i].value);
        }
        alias_count = 0;
        return 0;
    }

    // Unalias specific
    for (int i = 0; i < alias_count; i++) {
        if (aliases[i].key && strcmp(aliases[i].key, argv[1]) == 0) {
            free(aliases[i].key);
            free(aliases[i].value);
            
            for (int j = i; j < alias_count - 1; j++) {
                aliases[j] = aliases[j + 1];
            }
            alias_count--;
            return 0;
        }
    }
    
    return 0;
}

int builtin_source(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, RED"%s: usage: source filename [arguments]\n"RST, argv[0]);
        return 2;
    }

    return execute_script(argv[1]);
}
