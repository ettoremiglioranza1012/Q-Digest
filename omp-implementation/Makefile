CC = gcc-15
CFLAGS = -Iinclude -I../include -Wall -fopenmp -std=gnu99
# CFLAGS_DEBUG = $(CFLAGS) -g -O0

BUILD_DIR = build
EXEC_DIR = bin
EXEC = $(EXEC_DIR)/main

# Sources
OMP_SRCS = src/main.c src/ompcore.c
COMMON_SRCS = ../src/qcore.c ../src/memory_utils.c ../src/dynamic_array.c ../src/queue.c
# TEST_SRCS = src/test_omp.c src/ompcore.c

OMP_OBJS = $(patsubst src/%.c,$(BUILD_DIR)/omp_%.o,$(OMP_SRCS))
COMMON_OBJS = $(patsubst ../src/%.c,$(BUILD_DIR)/common_%.o,$(COMMON_SRCS))
# TEST_OBJS = $(patsubst src/%.c,$(BUILD_DIR)/test_%.o,$(TEST_SRCS))
OBJS = $(OMP_OBJS) $(COMMON_OBJS)
# TEST_EXEC = $(EXEC_DIR)/test_omp

.PHONY: main-omp run-local-main clean

# Main binary
main-omp: $(EXEC)

$(EXEC): $(OBJS) | $(EXEC_DIR)
	$(CC) $(CFLAGS) $^ -o $@

run-local-main: $(EXEC)
	./$(EXEC) 4

# Test binary (uncomment when needed)
# test-omp: $(TEST_EXEC)

# $(TEST_EXEC): $(TEST_OBJS) $(COMMON_OBJS) | $(EXEC_DIR)
# 	$(CC) $(CFLAGS) $^ -o $@

# run-local-test: $(TEST_EXEC)
# 	./$(TEST_EXEC) 4

# Pattern rules for objects
$(BUILD_DIR)/omp_%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# $(BUILD_DIR)/test_%.o: src/%.c | $(BUILD_DIR)
# 	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/common_%.o: ../src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

$(EXEC_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(EXEC_DIR) test_try test_try.dSYM
