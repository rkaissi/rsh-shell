# rsh-shell

A shell written in C for Linux and Unix systems. Covers process forking, pipe creation, signal handling, and terminal I/O.

## Features

- Command execution via `fork` + `execvp`
- Pipelines (`ls | grep foo | wc -l`)
- Builtins: `cd`, `exit`, `alias`, `unalias`
- Aliases with multi-token expansion (`alias ll="ls -l"`)
- Quoted arguments (`cd "My Folder"`)
- Command history with persistence (`~/.rsh_history`)
- History expansion (`!!`, `!n`)
- Tab completion

## Requirements

- GCC
- GNU Readline (`sudo apt install libreadline-dev` on Debian/Ubuntu)

## Build

Standard build:

```bash
make
```

Debug build (enables `-Wall -Wextra` warnings and `-g` debug symbols):

```bash
make DEBUG=1
```

Clean build artifacts:

```bash
make clean
```

## Usage

```bash
make        # build
make run    # build and run
```

Or run directly:

```bash
./build/rsh
```
