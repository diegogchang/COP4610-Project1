# ---- COP4610 Project: minish (linprog friendly) -----------------------------
CC      := gcc
CSTD    := -std=c11
CFLAGS  := $(CSTD) -Wall -Wextra -Wpedantic -O2 -g -Iinclude -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE
LDFLAGS :=
LDLIBS  :=          # no extra libs (glob() is in libc on Linux)

TARGET_NAME := minish
BIN_DIR     := bin
OBJ_DIR     := obj
SRC_DIR     := src
INC_DIR     := include
TARGET      := $(BIN_DIR)/$(TARGET_NAME)

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all dirs run clean debug memcheck format

all: dirs $(TARGET)

dirs:
	mkdir -p $(BIN_DIR) $(OBJ_DIR)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/*.h
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

debug: CFLAGS := $(CSTD) -Wall -Wextra -O0 -g3 -Iinclude -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE
debug: clean all

memcheck: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

format:
	@command -v clang-format >/dev/null 2>&1 && clang-format -i $(SRC_DIR)/*.c $(INC_DIR)/*.h || echo "clang-format not found; skipping."
