vpath %.c src
vpath %.h include

CC = mpicc
CFLAGS = -I include
TESTFLAGS = -I include -std=gnu99 -g -Wall
TESTCOREFLAGS = -I include -DTESTCORE -std=gnu99 -g -Wall
TESTALLFLAGS = -I include -DTESTALL -std=gnu99 -g -Wall

BUILD_DIR = build
EXEC_DIR = bin

all: $(EXEC_DIR)/main
test-queue: $(EXEC_DIR)/queue
test-all: $(EXEC_DIR)/test
test-core: $(EXEC_DIR)/test_core
test-serialization: $(EXEC_DIR)/test_serialization

.PHONY: clean clean-files-cluster run-local-test

$(EXEC_DIR)/test: test.c qcore.c dynamic_array.c memory_utils.c queue.c| $(EXEC_DIR)
	$(CC) $(TESTALLFLAGS) $^ -o $@

$(EXEC_DIR)/queue: queue.c memory_utils.c | $(EXEC_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_DIR)/test_core: test_qcore.c qcore.c dynamic_array.c memory_utils.c queue.c| $(EXEC_DIR)
	$(CC) $(TESTFLAGS) $^ -o $@

$(EXEC_DIR)/test_serialization: qcore.c dynamic_array.c memory_utils.c queue.c| $(EXEC_DIR)
	$(CC) $(TESTCOREFLAGS) $^ -o $@


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

# NOTE: ONLY FOR LOCAL DEVELOPMENT, DO NOT USE ON CLUSTER
run-local-test:
	mpirun -n 1 $(EXEC_DIR)/test_core
