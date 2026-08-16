CC      ?= gcc
CFLAGS  := -Wall -Wextra -Werror -pedantic -std=c11 -g
LDFLAGS :=

TARGET  := dns_cli
SRCS    := main.c
OBJS    := $(SRCS:.c=.o)

.PHONY: all clean run valgrind

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) google.com

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes ./$(TARGET) google.com

clean:
	rm -f $(OBJS) $(TARGET)
