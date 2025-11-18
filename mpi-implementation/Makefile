
CC = mpicc
CFLAGS = -Iinclude -I../include

BUILD_DIR = build
EXEC_DIR = bin
EXEC = $(EXEC_DIR)/main

MPI_SRCS = src/main.c src/tree_reduce.c
COMMON_SRCS = ../src/qcore.c ../src/memory_utils.c ../src/dynamic_array.c ../src/queue.c

MPI_OBJS = $(patsubst src/%.c,$(BUILD_DIR)/mpi_%.o,$(MPI_SRCS))
COMMON_OBJS = $(patsubst ../src/%.c,$(BUILD_DIR)/common_%.o,$(COMMON_SRCS))
OBJS = $(MPI_OBJS) $(COMMON_OBJS)

.PHONY: test clean run-local-test

main-mpi: $(EXEC)

$(EXEC): $(OBJS)| $(EXEC_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/mpi_%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/common_%.o: ../src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

$(EXEC_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(EXEC_DIR)

run-local-test: $(EXEC)
	mpirun -n 10 $(EXEC)
