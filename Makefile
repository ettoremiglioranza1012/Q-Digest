vpath %.c src
vpath %.h include

CC = mpicc
CFLAGS = -I include

BUILD_DIR = build
EXEC_DIR = bin

all: $(EXEC_DIR)/main

.PHONY: clean clean-files-cluster

$(EXEC_DIR)/%: $(BUILD_DIR)/%.o | $(EXEC_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: main.c test.h

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXEC_DIR):
	mkdir -p $(EXEC_DIR)

clean:
	rm -rf $(BUILD_DIR) $(EXEC_DIR)

clean-files-cluster:
	rm -rf *sh.*

