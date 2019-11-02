# xv6 Memory Safe Allocation
Programming Assignment 5 for CS 537

In this assignemtn, we modified the memory management part of xv6 so that it can
prevent the memory from Rowhammer attack. We modified the memory free function 
so that the free list is kept in a decending order w.r.t memory address. Also, 
we created a system call `dump_physmem`, which enable user program to get information
about the page allocation statistics.

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

## Some Comments

Current implementation may be still not able to prevent from Rowhammer attack 
because user program page can be allocated next to UKNOWN process page, which
may belongs to kernel process. Thus, user programs may corrupte kernel and do 
some malicious things.