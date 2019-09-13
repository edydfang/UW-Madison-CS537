// Copyright [2019] <Yidong Fang>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void remove_newline(char *line) {
  int str_len = strlen(line);
  if (line[str_len - 1] == '\n') {
    line[str_len - 1] = 0;
  }
}

int main(int argc, char **argv) {
  // check arguments count
  if (argc != 3) {
    printf("my-diff: invalid number of arguments\n");
    return 1;
  }
  // assign the argument to the vars
  char *fileAname, *fileBname;
  fileAname = argv[1];
  fileBname = argv[2];

  // open the dictionary file
  FILE *fpA = fopen(fileAname, "r");
  FILE *fpB = fopen(fileBname, "r");
  if (fpA == NULL || fpB == NULL) {
    printf("my-diff: cannot open file\n");
    return 1;
  }

  int lineno = 0;
  char *lineA = NULL, *lineB = NULL;
  size_t read_len = 0;
  int read_returnA, read_returnB;
  int last_diff = -1;
  while (1) {
    lineno += 1;
    // read line from both files
    read_returnA = getline(&lineA, &read_len, fpA);
    read_returnB = getline(&lineB, &read_len, fpB);
    if (read_returnA == -1 || read_returnB == -1) {
      break;
    }
    // before comparison remove the last newline char
    remove_newline(lineA);
    remove_newline(lineB);
    if (strcmp(lineA, lineB) != 0) {
      // two line are different
      if (last_diff + 1 != lineno) {
        printf("%d\n", lineno);
      }
      printf("< %s\n", lineA);
      printf("> %s\n", lineB);
      last_diff = lineno;
    }
  }
  // for a longer file, print it
  if (read_returnA == -1 && read_returnB != -1) {
    if (last_diff + 1 != lineno) {
      printf("%d\n", lineno);
    }
    do {
      remove_newline(lineB);
      printf("> %s\n", lineB);
    } while (getline(&lineB, &read_len, fpB) != -1);
  } else if (read_returnA != -1 && read_returnB == -1) {
    if (last_diff + 1 != lineno) {
      printf("%d\n", lineno);
    }
    do {
      remove_newline(lineA);
      printf("< %s\n", lineA);
    } while (getline(&lineA, &read_len, fpA) != -1);
  }
  // free the mem for lines
  free(lineA);
  free(lineB);

  // close two files
  fclose(fpA);
  fclose(fpB);
  return 0;
}
