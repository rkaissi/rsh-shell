#include "rsh.h"
#include "utils.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>

#include <readline/readline.h>
#include <readline/history.h>

// Potentially free aliases and use custom hashmap instead
// Add support for multiline commands with backslash at end (newline starts with "> ")

char HISTORY_PATH[HISTORY_PATH_BUFFERSIZE];
char RSHRC_PATH[RSHRC_PATH_BUFFERSIZE];

Alias aliases[MAX_ALIAS_COUNT];
int alias_count = 0;

char **tokenize(char *line, size_t *tokenCount) {
    *tokenCount = 0;
    size_t lineLen = strlen(line);

    int bufferSize = MIN_LISTSIZE;
    
    char **tokens = Malloc(bufferSize * sizeof(char *));

    char tokenBuffer[TOKEN_BUFFERSIZE];
    int tokenBufferLen = 0;

    bool withinQuotes = false;
    char lastQuote = 0;

    for (size_t i = 0; i < lineLen; i++) {
        char c = line[i];

        if (c == '\'' || c == '"') {
            if (withinQuotes && c == lastQuote) {
                withinQuotes = false;
                tokenBuffer[tokenBufferLen] = '\0';
                tokens[(*tokenCount)++] = strdup(tokenBuffer);
                if ((*tokenCount) >= bufferSize) {
                    bufferSize *= 2;
                    tokens = Realloc(tokens, bufferSize * sizeof(char *));
                }
                tokenBufferLen = 0;
                continue;
            }

            if (!withinQuotes) {
                withinQuotes = true;
                lastQuote = c;
                continue;
            }
        }

        if (isspace(c) && !withinQuotes) {
            if (tokenBufferLen > 0) {
                tokenBuffer[tokenBufferLen] = '\0';
                tokens[(*tokenCount)++] = strdup(tokenBuffer);
                if (*tokenCount >= bufferSize) {
                    bufferSize *= 2;
                    tokens = Realloc(tokens, bufferSize * sizeof(char *));
                }
                tokenBufferLen = 0;
            }
            continue;
        }

        tokenBuffer[tokenBufferLen++] = c;
    }

    if (tokenBufferLen > 0) {
        tokenBuffer[tokenBufferLen] = '\0';
        tokens[(*tokenCount)++] = strdup(tokenBuffer);
    }

    return tokens;
}

bool check_builtins(char**argv) {
    if (argv[0] == NULL) {
        return true;
    }

    if (strcmp(argv[0], "cd") == 0) {
        if (argv[1] == NULL) {
            argv[1] = getenv("HOME");
        }

        if (chdir(argv[1]) != 0) {
            err(argv[1]);
        }
        return true;
    }
    
    if (strcmp(argv[0], "exit") == 0) {
        printf(RED"[Exit]\n"RST);
        write_history(HISTORY_PATH);
        exit(EXIT_SUCCESS);
        return true;
    }
    
    if (strcmp(argv[0], "alias") == 0 && argv[1] != NULL) {
        if (alias_count >= 256) {
            printf(RED"Exceeded max alias count of %d\n"RST, MAX_ALIAS_COUNT);
            return true;
        }

        char *delimPtr = strchr(argv[1], '=');

        if (delimPtr == NULL) {
            printf(RED"Alias not assigned correctly\n"RST);
            return true;
        }
        *delimPtr = '\0';
        char *key = argv[1];
        char *val = delimPtr + 1;

        for (int i = 0; i < alias_count; i++) {
            if (aliases[i].key && strcmp(aliases[i].key, argv[1]) == 0) {
                free(aliases[i].key);
                free(aliases[i].value);
                aliases[i] = (Alias){strdup(key), strdup(val)};
                return true;
            }
        }

        aliases[alias_count++] = (Alias){strdup(key), strdup(val)};
        return true;
    }

    if (strcmp(argv[0], "unalias") == 0 && argv[1] != NULL) {
        for (int i = 0; i < alias_count; i++) {
            if (strcmp(argv[1], "-a") == 0) {
                free(aliases[i].key);
                free(aliases[i].value);
            }

            if (aliases[i].key && strcmp(aliases[i].key, argv[1]) == 0) {
                for (int j = i; j < alias_count - 1; j++) {
                    aliases[j] = aliases[j + 1];
                }
                alias_count--;
                return true;
            }
        }

        if (strcmp(argv[1], "-a") == 0)
            alias_count = 0;
        
        return true;
    }

    return false;
}

char ***split_pipes(char **tokens, size_t tokenCount, size_t *commandCount) {
    *commandCount = 0;
    int bufferSize = MIN_LISTSIZE;
    char ***commands = Malloc(bufferSize * sizeof(char **));
    
    for (size_t i = 0; i < tokenCount; i++) {
        char **argv = Malloc(TOKEN_BUFFERSIZE * sizeof(char *));
        size_t j = 0;
        for (; i < tokenCount && strcmp(tokens[i], "|") != 0; j++, i++) {
            argv[j] = tokens[i];
        }
        argv[j] = NULL;
        commands[(*commandCount)++] = argv;

        if (*commandCount >= bufferSize) {
            bufferSize *= 2;
            commands = Realloc(commands, bufferSize * sizeof(char **));
        }
    }
    return commands;
}

void close_pipes(int pipes[][2], int pipeCount) {
    for (int i = 0; i < pipeCount; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

void execute_pipeline(char ***commands, size_t commandCount) {
    int pipeCount = commandCount - 1;
    int pipes[pipeCount][2];
    pid_t pids[commandCount];

    for (int i = 0; i < pipeCount; i++) {
        pipe(pipes[i]);
    }

    for (int i = 0; i < commandCount; i++)
    {
        pid_t pid = fork();
        if (pid == 0) { // child
            if (i != 0) // if not first command
                dup2(pipes[i-1][0], STDIN_FILENO);  // make stdin point to the read end

            if (i != commandCount - 1) // if not last command
                dup2(pipes[i][1], STDOUT_FILENO); // make stdout point to the write end

            close_pipes(pipes, pipeCount);

            char **argv = commands[i];
            execvp(argv[0], argv);
            err(argv[0]);
            write_history(HISTORY_PATH);
            exit(EXIT_FAILURE);
        } else if (pid == -1) { // fork failed
            err("Failed to run process");
        } else { // parent, pid = child's pid
            pids[i] = pid;
            continue;
        }
    }

    close_pipes(pipes, pipeCount);

    int status;
    for (int i = 0; i < commandCount; i++) {
        waitpid(pids[i], &status, 0);
        if (!WIFEXITED(status)) {
            err("Process termination error");
        }
    }
}

void print_banner() {
    printf(BLUE"\n"
           "██████╗ ███████╗██╗  ██╗\n"
           "██╔══██╗██╔════╝██║  ██║\n"
           "██████╔╝███████╗███████║\n"
           "██╔══██╗╚════██║██╔══██║\n"
           "██║  ██║███████║██║  ██║\n"
           "╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝\n"
           "\n"RST);
}

void execute_line(char *line) {
    size_t baseTokenCount = 0;
    char **baseTokens = tokenize(line, &baseTokenCount);

    char **tokens = baseTokens;
    int tokenCount = baseTokenCount;

    for (int i = 0; i < alias_count; i++) {
        if (aliases[i].key && strcmp(baseTokens[0], aliases[i].key) == 0) {
            size_t aliasTokenCount;
            char **aliasTokens = tokenize(aliases[i].value, &aliasTokenCount);
            tokenCount = baseTokenCount + aliasTokenCount - 1;
            tokens = Malloc(tokenCount * sizeof(char *));

            for (int j = 0; j < aliasTokenCount; j++) {
                tokens[j] = aliasTokens[j];
            }

            for (int j = 1; j < baseTokenCount; j++) { // skip alias in tokens[0]
                tokens[aliasTokenCount + j - 1] = baseTokens[j];
            }

            free(baseTokens);
            free(aliasTokens);
            break;
        }
    }

    size_t commandCount = 0;
    char ***commands = split_pipes(tokens, tokenCount, &commandCount);

    bool handled = false;

    if (commandCount == 1)
        handled = check_builtins(commands[0]);

    if (!handled)
        execute_pipeline(commands, commandCount);

    for (int i = 0; i < commandCount; i++) {
        free(commands[i]);
    }

    for (int i = 0; i < tokenCount; i++) {
        free(tokens[i]);
    }
    
    free(tokens);
    free(commands);
}

int main(void) {
    int interactive = isatty(STDIN_FILENO);

    if (interactive) {
        print_banner();

        // Configure readline to auto-complete paths when the tab key is hit
        rl_bind_key('\t', rl_complete);

        // Enable history and read from file
        snprintf(HISTORY_PATH, sizeof(HISTORY_PATH), "%s" HISTORY_FILE, getenv("HOME"));
        snprintf(RSHRC_PATH, sizeof(RSHRC_PATH), "%s" RSHRC_FILE, getenv("HOME"));

        using_history();
        stifle_history(1000);
        read_history(HISTORY_PATH);
    }

    while (1) {
        char *input = NULL;

        if (interactive) {
            char *pwd = getcwd(NULL, 0);
            char prompt[strlen(pwd) + 32];
            snprintf(prompt, sizeof(prompt), GREEN"%s $ "RST, pwd);
            free(pwd);
            input = readline(prompt);
        } else {
            size_t len = 0;
            if (getline(&input, &len, stdin) == -1) {
                if (ferror(stdin)) {
                    err("Error");
                    exit(EXIT_FAILURE);
                } else if (feof(stdin)) {
                    break;
                }
            }
        }

        // Check for EOF
        if (!input) {
            if (interactive) printf(RED "[Exit]\n" RST);
            break;
        }

        // Empty line (only space)
        if (strspn(input, DELIM) == strlen(input)) {
            free(input);
            continue;
        }

        char *expanded = input;

        if (interactive) {
            int result = history_expand(input, &expanded);
            free(input);

            // Expanded
            if (result == 1) {
                printf(YELLOW"%s\n"RST, expanded);
                fflush(stdout);
            }

            // Error expanding
            if (result == -1) {
                err(expanded);  // expanded contains the error message in this case
                free(expanded);
                continue;
            }

            // Display only (e.g. user typed :p suffix), don't execute
            if (result == 2) {
                printf("%s\n", expanded);
                free(expanded);
                continue;
            }

            add_history(expanded);
        }

        execute_line(expanded);

        free(expanded);
    }

    if (interactive)
        write_history(HISTORY_PATH);

    return EXIT_SUCCESS;
}
