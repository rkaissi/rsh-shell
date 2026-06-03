#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <readline/history.h>

#include "builtins.h"
#include "utils.h"

const Builtin builtins[] = {
    {.name = "cd", .func = builtin_cd, .description = "Change the shell working directory", .usage = "cd [dir]"},
    {.name = "exit", .func = builtin_exit, .description = "Exit the shell", .usage = "exit [n]"},
    {.name = "alias", .func = builtin_alias, .description = "Define or display aliases", .usage = "alias [name[=value] ... ]"},
    {.name = "unalias", .func = builtin_unalias, .description = "Remove from the list of defined aliases", .usage = "unalias [-a] name [name ...]"},
    {.name = "source", .func = builtin_source, .description = "Execute commands from a file in the current shell", .usage = "source filename"},
};

const int builtin_count = ARRAY_LEN(builtins);

const char *get_usage(const char *builtin_name) {
    for (int i = 0; i < ARRAY_LEN(builtins); i++) {
        if (strcmp(builtins[i].name, builtin_name) == 0) {
            return builtins[i].usage;
        }
    }

    return NULL;
}

void print_usage_error(const char *name) {
    const char *usage = get_usage(name);
    if (usage)
        fprintf(stderr, RED"%s: usage: %s\n"RST, name, usage);
}

int builtin_cd(char **argv)  {
    char *path = argv[1] ? argv[1] : getenv("HOME");
    
    if (path == NULL) {
        fprintf(stderr, RED"%s: HOME not set\n"RST, argv[0]);
        return 1;
    }

    if (chdir(path) != 0) {
        err(argv[0]);
        return 1;
    }
    return 0;
}

int builtin_exit(char **argv) {
    char *end;
    long exit_code = argv[1] ? strtol(argv[1], &end, 10) : EXIT_SUCCESS;
    if (argv[1] && *end != '\0') {
        fprintf(stderr, RED"%s: %s: numeric argument required\n"RST, argv[0], argv[1]);
        print_usage_error(argv[0]);
    }
    write_history(history_path);
    printf(RED"[Exit]\n"RST);
    exit(exit_code % (ERROR_CODES + 1));
    return exit_code;
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
        print_usage_error(argv[0]);
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
        print_usage_error(argv[0]);
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
        print_usage_error(argv[0]);
        return 2;
    }

    return execute_script(argv[1]);
}
