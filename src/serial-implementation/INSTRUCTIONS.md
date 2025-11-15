# Instructions for testing

To test the implementation use the dedicated test module.

```bash
# build the test
make test-all

# run the binary executable
bin/test

# clean the directory
make clean

# clean files generated on cluster
make clean-files-cluster
```

## Results of testing on QCore

The following results have been obtained by running the dedicated test suite.

```bash
# clean directory and prepare for new build
make clean

# build executable test suite
make test-core

# run the test
bin/test_core

# or if using mpicc as the compiler
mpirun -n 1 bin/test_core
```

```text
==== Testing log_2_ceil ====
log_2_ceil tests passed

==== Testing create_node and delete_node ====
Node creation/deletion passed

==== Testing QDigest create/delete ====
QDigest create/delete passed

==== Testing insert and percentile ====
Approx 50th percentile: 3

==== Testing insert_node and traversal ====
2 2 1
4 4 1

==== Testing expand_tree ====
1 1 1
3 3 1
7 7 1

==== Testing compress_if_needed ====
[TREE] num_nodes: 5, (N, K): (10, 1)

==== Testing merge ====
After merge, q1 N = 4
1 1 1
2 2 1
3 3 1
4 4 1

==== Testing swap_q ====

All tests completed successfully.
Process 51965 is not debuggable. Due to security restrictions, leaks can only show or save contents of readonly memory of restricted processes.

Process:         test_core [51965]
Path:            /Users/USER/Desktop/*/test_core
Load Address:    0x104274000
Identifier:      test_core
Version:         0
Code Type:       ARM64
Platform:        macOS
Parent Process:  zsh [50026]
Target Type:     corpse

Date/Time:       2025-11-12 22:08:50.930 +0100
Launch Time:     2025-11-12 22:08:50.406 +0100
OS Version:      macOS 26.0.1 (25A362)
Report Version:  7
Analysis Tool:   /usr/bin/leaks

Physical footprint:         912K
Physical footprint (peak):  912K
Idle exit:                  untracked
----

leaks Report Version: 4.0
Process 51965: 191 nodes malloced for 14 KB
Process 51965: 7 leaks for 336 total leaked bytes.

    7 (336 bytes) << TOTAL >>

      4 (192 bytes) ROOT CYCLE: 0x84d0001e0 [48]
         3 (144 bytes) ROOT CYCLE: 0x84d000210 [48]
            CYCLE BACK TO 0x84d0001e0 [48]
            2 (96 bytes) ROOT CYCLE: 0x84d000240 [48]
               1 (48 bytes) ROOT CYCLE: 0x84d000270 [48]

      3 (144 bytes) ROOT CYCLE: 0x84d000000 [48]
         2 (96 bytes) ROOT CYCLE: 0x84d000030 [48]
            CYCLE BACK TO 0x84d000000 [48]
            1 (48 bytes) ROOT CYCLE: 0x84d000060 [48]
```

## Acknowledgments

The current implementation uses portions of code that have been ported in C from a
C++ implementation. The main repo that inspired the current work is available at:
`https://github.com/dhruvbird/q-digest/tree/master`
