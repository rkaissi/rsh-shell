CC      = gcc
LIBS    = -lreadline
TARGET  = build/rsh
SRCS    = src/rsh.c src/utils.c src/builtins.c
OBJS    = $(patsubst src/%.c, build/%.o, $(SRCS))
DEPS    = $(OBJS:.o=.d)

ifdef DEBUG
    CFLAGS = -Wall -Wextra -g -Iinclude
else
    CFLAGS = -O2 -DNDEBUG -Iinclude
endif

CFLAGS += -MMD -MP

.PHONY: all clean run install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	sudo ln -sf "$(abspath $(TARGET))" /usr/bin/rsh

uninstall:
	sudo rm -f /usr/bin/rsh

clean:
	rm -rf build

-include $(DEPS)
