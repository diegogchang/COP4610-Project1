BIN_DIR := bin
TARGET  := proj2

.PHONY: all run clean
all:
	cargo build --release
	mkdir -p $(BIN_DIR)
	# Copy platform-appropriate artifact name
	cp target/release/$(TARGET) $(BIN_DIR)/$(TARGET) || true

run: all
	./$(BIN_DIR)/$(TARGET)

clean:
	cargo clean
	rm -rf $(BIN_DIR)
