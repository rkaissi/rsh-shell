CC      = gcc
CFLAGS  = -Wall -Wextra
LIBS    = -lreadline
TARGET  = rsh

ifdef DEBUG
    CFLAGS += -g
else
    CFLAGS  = -O2
endif

SRCS    = rsh.c utils.c
OBJS    = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

rsh.o: rsh.c rsh.h utils.h
	$(CC) $(CFLAGS) -c rsh.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
