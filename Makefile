CC      = gcc
LIBS    = -lreadline
TARGET  = build/rsh
SRCS    = src/rsh.c src/utils.c
OBJS    = $(patsubst src/%.c, build/%.o, $(SRCS))

ifdef DEBUG
    CFLAGS = -Wall -Wextra -g -Iinclude
else
    CFLAGS = -O2 -Iinclude
endif

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build

.PHONY: clean run
