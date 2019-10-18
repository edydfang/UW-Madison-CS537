# xv6 MLQ Scheduler
Programming Assignment 4 for CS 537

In this assignment, we changed the scheduler of xv6 from simple Round Robin to a Multi-level Queue one. The most important thing to do in this assignment is to figure out the control flow of the scheduler, and then use the right data structure to store the related queue information. Also, remember to test all the manipulations on the data structures before integrated it into the scheduler as the kernel panic is really hard to debug even with GDB. In my modification, I used statically allocated queue node to simulate the node in the queue (or linked list), as we already has a limit of total number of the processes in the system. Also, for scheduling part, there should exist two strategies. One is to update the queues before every scheduling, the other one is to update the queue whenever the related events happen, i.e. sleep, wakeup, fork, etc. I choose the first strategy which is much easier to implement.

## How to build

```shell
# normal make
make
# debug make
make qemu-nox-debug
# debug test make
make test-debug
```

## How to run

```shell
# normal build and run without X
make qemu-nox
# build test code together and run without X
make test
```

## Important Things during Development

- kernel debugging is really hard even equipped with GDB, so always do `defensive programming` and panic for unexpected value.
- test cases from TAs can be weird, just try to test them on different machines.
