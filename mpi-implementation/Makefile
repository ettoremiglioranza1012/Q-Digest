CC = mpicc
CFLAGS = -Iinclude -I../include

BUILD_DIR = build
EXEC_DIR = bin
EXEC = $(EXEC_DIR)/main

# Sources
MPI_SRCS = src/main.c src/tree_reduce.c
COMMON_SRCS = ../src/qcore.c ../src/memory_utils.c ../src/dynamic_array.c ../src/queue.c
TEST_SRCS = src/test_mpi.c src/tree_reduce.c

MPI_OBJS = $(patsubst src/%.c,$(BUILD_DIR)/mpi_%.o,$(MPI_SRCS))
COMMON_OBJS = $(patsubst ../src/%.c,$(BUILD_DIR)/common_%.o,$(COMMON_SRCS))
TEST_OBJS = $(patsubst src/%.c,$(BUILD_DIR)/test_%.o,$(TEST_SRCS))
OBJS = $(MPI_OBJS) $(COMMON_OBJS)
TEST_EXEC = $(EXEC_DIR)/test_mpi

.PHONY: main-mpi test-mpi run-local-main run-local-test clean

# Main binary
main-mpi: $(EXEC)

$(EXEC): $(OBJS) | $(EXEC_DIR)
	$(CC) $(CFLAGS) $^ -o $@

run-local-main: $(EXEC)
	mpirun -n 4 $(EXEC)

# Test binary
test-mpi: $(TEST_EXEC)

$(TEST_EXEC): $(TEST_OBJS) $(COMMON_OBJS) | $(EXEC_DIR)
	$(CC) $(CFLAGS) $^ -o $@

run-local-test: $(TEST_EXEC)
	mpirun -n 1 $(TEST_EXEC)

# Pattern rules for objects
$(BUILD_DIR)/mpi_%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/test_%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/common_%.o: ../src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

$(EXEC_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(EXEC_DIR)
