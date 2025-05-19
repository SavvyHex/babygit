# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -g
LDFLAGS = -lcrypto

# Source files
SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)

# Output directory and executable
BIN_DIR = bin
EXECUTABLE = $(BIN_DIR)/babygit  # Output: bin/babygit

# Default target
all: $(EXECUTABLE)

# Rule to create the executable
$(EXECUTABLE): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to create the bin/ directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $@

# Compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	rmdir $(BIN_DIR) 2>/dev/null || true

# Install system-wide (optional)
install: $(EXECUTABLE)
	sudo cp $(EXECUTABLE) /usr/local/bin/

.PHONY: all clean install
