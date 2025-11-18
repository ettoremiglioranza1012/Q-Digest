# =======================
# Q-Digest Build System
# =======================


CC = mpicc
AR = ar
CFLAGS = -I include -Wall -std=c99 -g
DEBUG_FLAGS = -g -O0
# Serial variables
SERIAL_TESTFLAGS = -I include -std=c99 -g -Wall
SERIAL_TESTCOREFLAGS = $(SERIAL_TESTFLAGS) -DTESTCORE
SERIAL_TESTALLFLAGS = $(SERIAL_TESTFLAGS) -DTESTALL
SERIAL_TESTQUEUEFLAGS = $(CFLAGS) -DTESTQUEUE

BUILD_DIR = build
LIB_DIR = lib
BIN_DIR = bin

# Core library sources (NO src/ prefix - just filenames)
CORE_SOURCES = qcore.c queue.c memory_utils.c dynamic_array.c
CORE_OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(CORE_SOURCES))
LIB_NAME = libqdigest.a
LIB_PATH = $(LIB_DIR)/$(LIB_NAME)
SERIAL_CORE_SRCS = $(addprefix src/,qcore.c queue.c memory_utils.c dynamic_array.c)
SERIAL_TEST_QCORE = serial-implementation/src/test_qcore.c 
SERIAL_TEST_MAIN = serial-implementation/src/test.c 
SERIAL_TEST_CORE_BIN = $(BIN_DIR)/serial-test_core
SERIAL_TEST_ALL_BIN  = $(BIN_DIR)/serial-test_all
SERIAL_TEST_QUEUE_BIN= $(BIN_DIR)/serial-test_queue
SERIAL_TEST_SER_BIN  = $(BIN_DIR)/serial-test_serialization

# MPI implementation
MPI_MAIN = mpi-implementation/src/main.c
MPI_OBJ = $(BUILD_DIR)/main.o
MPI_BIN = $(BIN_DIR)/main

# Tests
TEST_MAIN = tests/test_main.c
TEST_OBJ = $(BUILD_DIR)/test_main.o
TEST_BIN = $(BIN_DIR)/test


.PHONY: all library mpi test clean help docs serial-test-core serial-test-all serial-test-queue serial-test-serialization serial-run-local-test


all: library mpi test

# ===== Library =====
library: $(LIB_PATH)

$(LIB_PATH): $(CORE_OBJECTS) | $(LIB_DIR)
	$(AR) rcs $@ $^
	@echo "✓ Library built: $@"

# ===== MPI Implementation =====
mpi: $(MPI_BIN)

$(MPI_BIN): $(MPI_OBJ) $(LIB_PATH) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(MPI_OBJ) -o $@ -L$(LIB_DIR) -lqdigest
	@echo "✓ MPI executable built: $@"

# ===== Tests =====
test: $(TEST_BIN)

$(TEST_BIN): $(TEST_OBJ) $(LIB_PATH) | $(BIN_DIR)
	$(CC) $(TESTFLAGS) $(TEST_OBJ) -o $@ -L$(LIB_DIR) -lqdigest
	@echo "✓ Test executable built: $@"

# ===== Serial test ======
serial-test-core: $(SERIAL_TEST_CORE_BIN)

$(SERIAL_TEST_CORE_BIN): $(SERIAL_TEST_QCORE) $(SERIAL_CORE_SRCS) | $(BIN_DIR)
	$(CC) $(SERIAL_TESTFLAGS) $^ -o $@
	@echo "✓ Serial test_core built: $@"

serial-test-all: $(SERIAL_TEST_ALL_BIN)

$(SERIAL_TEST_ALL_BIN): $(SERIAL_TEST_MAIN) $(SERIAL_CORE_SRCS) | $(BIN_DIR)
	$(CC) $(SERIAL_TESTALLFLAGS) $^ -o $@
	@echo "✓ Serial test_all built: $@"

serial-test-queue: $(SERIAL_TEST_QUEUE_BIN)

$(SERIAL_TEST_QUEUE_BIN): src/queue.c src/memory_utils.c | $(BIN_DIR)
	$(CC) $(SERIAL_TESTQUEUEFLAGS) $^ -o $@
	@echo "✓ Serial queue test built: $@"

serial-test-serialization: $(SERIAL_TEST_SER_BIN)
	
$(SERIAL_TEST_SER_BIN): $(SERIAL_CORE_SRCS) | $(BIN_DIR)
	$(CC) $(SERIAL_TESTCOREFLAGS) $^ -o $@
	@echo "✓ Serial serialization test built: $@"

# ===== Object Files =====
# Core sources from src/
$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "  ○ Compiled: $<"

# MPI main
$(BUILD_DIR)/main.o: $(MPI_MAIN) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "  ○ Compiled: $<"

# Test main
$(BUILD_DIR)/test_main.o: $(TEST_MAIN) | $(BUILD_DIR)
	$(CC) $(TESTFLAGS) -c $< -o $@
	@echo "  ○ Compiled: $<"

# ===== Directories =====
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# ===== Utilities =====
clean:
	@rm -rf $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR)
	@echo "✓ Cleaned build artifacts"

# Right now we made a pattern rule only to run a test on the core structures
# eventually we can expand in the future. 
serial-run-local-test: serial-test-core
	mpirun -n 1 $(SERIAL_TEST_CORE_BIN)

docs:
	doxygen Doxyfile
help:
	@printf "Q-Digest Build System\n"
	@printf "=====================\n\n"
	@printf "%-32s %s\n" "make library" "Build core library only"
	@printf "%-32s %s\n" "make mpi" "Build MPI implementation"
	@printf "%-32s %s\n" "make test" "Build tests"
	@printf "%-32s %s\n" "make serial-test-core" "Build serial test_core executable"
	@printf "%-32s %s\n" "make serial-test-all" "Build serial comprehensive test executable"
	@printf "%-32s %s\n" "make serial-test-queue" "Build serial queue test executable"
	@printf "%-32s %s\n" "make serial-test-serialization" "Build serial serialization test executable"
	@printf "%-32s %s\n" "make serial-run-local-test" "Run serial test_core with mpirun"
	@printf "%-32s %s\n" "make all" "Build everything (library, mpi, test)"
	@printf "%-32s %s\n" "make docs" "Builds documentation for the project"
	@printf "%-32s %s\n" "make clean" "Remove build artifacts"
	@printf "%-32s %s\n" "make help" "Show this help message"
