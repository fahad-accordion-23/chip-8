CC = gcc
CFLAGS = -Wall -Wpedantic -Werror -Wsign-conversion -Wconversion
BUILD_DIR = build

all: $(BUILD_DIR)/chip-8-run

$(BUILD_DIR)/chip-8-run: $(BUILD_DIR)/main.o $(BUILD_DIR)/chip-8.o
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/main.o: main.c chip-8.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c main.c -o $@

$(BUILD_DIR)/chip-8.o: chip-8.h chip-8.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c chip-8.c -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean: 
	rm -rf $(BUILD_DIR)
