CC        ?= gcc
CFLAGS    := -Wall -Wextra -Werror -pedantic -std=c11 -g -Iinclude
LDFLAGS   :=

TARGET    := dns_cli
SRC_DIR   := src
PYTHON    := python3
BUILD_DIR := build

SRCS      := $(wildcard $(SRC_DIR)/*.c)
OBJS      := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run valgrind mock-server compiledb

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET) google.com

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes ./$(TARGET) google.com

mock-server:
	$(PYTHON) tests/mock_server.py $(ARGS)

compiledb: clean
	bear -- make all

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
