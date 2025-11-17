# =======================
# Q-Digest Build System
# =======================

CC = mpicc
AR = ar
CFLAGS = -I include -Wall -std=c99 -g
TESTFLAGS = -I include -std=gnu99 -g -Wall
DEBUG_FLAGS = -g -O0

BUILD_DIR = build
LIB_DIR = lib
BIN_DIR = bin

# Core library sources (NO src/ prefix - just filenames)
CORE_SOURCES = qcore.c queue.c memory_utils.c dynamic_array.c
CORE_OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(CORE_SOURCES))
LIB_NAME = libqdigest.a
LIB_PATH = $(LIB_DIR)/$(LIB_NAME)

# MPI implementation
MPI_MAIN = mpi-implementation/src/main.c
MPI_OBJ = $(BUILD_DIR)/main.o
MPI_BIN = $(BIN_DIR)/main

# Tests
TEST_MAIN = tests/test_main.c
TEST_OBJ = $(BUILD_DIR)/test_main.o
TEST_BIN = $(BIN_DIR)/test

.PHONY: all library mpi test clean help docs

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

docs:
	doxygen Doxyfile
help:
	@echo "Q-Digest Build System"
	@echo "===================="
	@echo "make library   - Build core library only"
	@echo "make mpi       - Build MPI implementation"
	@echo "make test      - Build tests"
	@echo "make all       - Build everything (library, mpi, test)"
	@echo "make docs      - Builds documentation for the project"
	@echo "make clean     - Remove build artifacts"
	@echo "make help      - Show this help message"
