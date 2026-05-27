#include "rsh.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>

#include <readline/readline.h>
#include <readline/history.h>

#define DELIM " \t\r\n\a"
#define BUFFERSIZE 64

char HISTORY_PATH[256];

char **split_line(char *line) {
    int bufferSize = BUFFERSIZE;
    char **tokens = Malloc(bufferSize * sizeof(char *));
    char *token;
    int pos = 0;

    token = strtok(line, DELIM);

    while (token) {
        tokens[pos++] = token;
        token = strtok(NULL, DELIM);

        if (pos >= bufferSize) {
            bufferSize += BUFFERSIZE;
            tokens = Realloc(tokens, bufferSize * sizeof(char *));
        }
    }

    tokens[pos] = NULL;
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
    } else if (strcmp(argv[0], "exit") == 0) {
        printf(RED"[Exit]\n"RST);
        write_history(HISTORY_PATH);
        exit(EXIT_SUCCESS);
    }

    return false;
}

char ***split_pipes(char **tokens, size_t *count) {
    int bufferSize = BUFFERSIZE;
    char ***commands = Malloc(bufferSize * sizeof(char **));
    size_t tokenPos = 0;
    size_t commandPos = 0;
    
    while (tokens[tokenPos] != NULL) {
        char **argv = Malloc(16 * sizeof(char *));
        size_t i = 0;
        for (; tokens[tokenPos] != NULL && strcmp(tokens[tokenPos], "|") != 0; i++, tokenPos++) {
            argv[i] = tokens[tokenPos];
        }
        if (tokens[tokenPos] != NULL && strcmp(tokens[tokenPos], "|") == 0) tokenPos++;
        argv[i] = NULL;
        commands[commandPos++] = argv;

        if (commandPos >= bufferSize) {
            bufferSize += BUFFERSIZE;
            commands = Realloc(commands, bufferSize * sizeof(char **));
        }
    }
    commands[commandPos] = NULL;
    *count = commandPos;
    return commands;
}

void close_pipes(int pipes[][2], int pipeCount) {
    for (int i = 0; i < pipeCount; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

void execute_pipeline(char ***commands, int commandCount) {
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

int main(void) {
    print_banner();

    // Configure readline to auto-complete paths when the tab key is hit
    rl_bind_key('\t', rl_complete);

    // Enable history and read from file
    snprintf(HISTORY_PATH, sizeof(HISTORY_PATH), "%s" HISTORY_FILE, getenv("HOME"));

    using_history();
    stifle_history(1000);
    read_history(HISTORY_PATH);

    while (1) {
        char *pwd = getcwd(NULL, 0);
        char prompt[strlen(pwd) + 32];
        snprintf(prompt, sizeof(prompt), GREEN"%s $ "RST, pwd);
        free(pwd);

        char *input = readline(prompt);

        // Check for EOF
        if (!input) {
            printf(RED "[Exit]\n" RST);
            break;
        }

        add_history(input);

        char **tokens = split_line(input);

        size_t commandCount;
        char ***commands = split_pipes(tokens, &commandCount);

        bool handled = false;

        if (commandCount == 1)
            handled = check_builtins(commands[0]);

        if (!handled)
            execute_pipeline(commands, commandCount);

        for (int i = 0; i < commandCount; i++) {
            free(commands[i]);
        }
        
        free(tokens);
        free(commands);
        free(input);
    }

    write_history(HISTORY_PATH);
    return EXIT_SUCCESS;
}
