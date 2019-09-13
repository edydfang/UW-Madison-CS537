# UNIX Utilities
Programming Assignment 1 for CS 537

In this assignment, three simple unix utilities are created, which are `my-look`, `across` and `my-diff`, respectively.

## How to build
```shell
# Take my-look as an example
gcc -o my-look my-look.c -Wall -Werror
```

## How to run
```shell
# my-look: similar to look
./my-look prefix [word-dict-file]

# across: find words with targeted lengths with a substring at a specific position from a word dict file
./across substring start_index target_length [word-dict-file]

# my-diff: find difference between two text file
./diff fileA fileB
```

## Important Things during Development

- Memory Leak Checking using [valgring](http://www.valgrind.org/)
- Code style checking tools, such as Google's [cpplint](https://github.com/cpplint/cpplint)
- Potential problem, If size of some line read by `my-diff` is larger than RAM, the program can failed because of `getline`.
