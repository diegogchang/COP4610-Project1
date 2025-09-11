# ---- COP4610-compliant Makefile (linprog friendly) --------------------------
# Build outputs go to bin/, objects to obj/, headers in include/, sources in src/

# Toolchain
CC      := gcc
CSTD    := -std=c11
CFLAGS  := $(CSTD) -Wall -Wextra -Wpedantic -O2 -g -Iinclude
LDFLAGS :=
LDLIBS  :=

# Project metadata
TARGET_NAME := proj1
BIN_DIR     := bin
OBJ_DIR     := obj
SRC_DIR     := src
INC_DIR     := include

TARGET := $(BIN_DIR)/$(TARGET_NAME)

# Sources and objects
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Default rule
.PHONY: all
all: $(TARGET)

# Link step
$(TARGET): $(BIN_DIR) $(OBJ_DIR) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# Compile step (pattern rule)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/*.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure dirs exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Convenience targets
.PHONY: run clean debug memcheck format
run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Build with debug symbols (already included via -g); kept for clarity
debug: CFLAGS := $(CSTD) -Wall -Wextra -O0 -g3 -Iinclude
debug: clean all

# Optional: valgrind memory check (available on linprog)
memcheck: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# Optional: clang-format (only if present)
format:
	@command -v clang-format >/dev/null 2>&1 && clang-format -i $(SRC_DIR)/*.c $(INC_DIR)/*.h || echo "clang-format not found; skipping."
