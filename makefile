CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -g
CPPFLAGS = -D_POSIX_C_SOURCE=200809L

# Folders
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = build
INC_DIR = include

# Targets
TARGET = $(BIN_DIR)/unix-shell

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

OBJS_NO_MAIN = $(patsubst $(OBJ_DIR)/main.o,,$(OBJS))

# Default target
all: $(TARGET)

# Linking stage
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INC_DIR) -o $@ $^

# Compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INC_DIR) -c $< -o $@

# Create directories if absent
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Housekeeping
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
