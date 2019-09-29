# Shell
Programming Assignment 3 for CS 537

In this assignment, we implmented a shell program based on Unix System. The main
functions including command line execution, background running, stdout/stderror
redirection and jobs managing, and it involves two modes, interactive mode and
batch mode.

## How to build

```shell
make
```

## How to run

```shell
# interactive mode
./mysh
# batch mode
./mysh <batch-shell-file>
# example
./mysh test.sh
```

## Important Things during Development

- For those "string" you need to store and the original variable may be changed
  later, we may need to `malloc` and put them in to `heap` space.
- Maybe this is a good idea to `refactor` the project if it is getting too large.
