// Copyright [2019] <Yidong Fang>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int check_digits(char *input_str) {
  /*
  check whether all the chars in an char array are digits
  if ture return 0
  if false, return 1
  */
  int i = 0;
  while (input_str[i]) {
    if (!isdigit(input_str[i])) {
      return 1;
    }
    i += 1;
  }
  return 0;
}

int check_letters(char *input_str) {
  /*
  check whether all the chars in an char array are letters
  if ture return 0
  if false, return 1
  */
  int i = 0;
  while (input_str[i]) {
    if ((input_str[i] < 'a' || input_str[i] > 'z') && input_str[i] != '\n') {
      return 1;
    }
    i += 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  // check arguments count
  if (argc < 4 || argc > 5) {
    printf("across: invalid number of arguments\n");
    return 1;
  }
  // assign the argument to the vars
  char *str, *filename;
  int start_idx, target_len;
  str = argv[1];
  if (check_digits(argv[2]) || check_digits(argv[3])) {
    printf("across: invalid arguments for start index or word length\n");
    return 1;
  }
  start_idx = strtol(argv[2], NULL, 10);
  target_len = strtol(argv[3], NULL, 10);

  if (argc == 5) {
    filename = argv[4];
  } else {
    filename = "/usr/share/dict/words";
  }

  // open the dictionary file
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("across: cannot open file\n");
    return 1;
  }

  if (strlen(str) + start_idx > target_len) {
    printf("across: invalid position\n");
    fclose(fp);
    return 1;
  }

  // size 100 is engouth for assmumed condition
  char line[100];
  int result, str_len = strlen(str);
  int line_len_without_n, line_len;
  // read file
  while (fgets(line, 100, fp)) {
    line_len = strlen(line);
    if (line[strlen(line) - 1] == '\n') {
      line_len_without_n = line_len - 1;
    } else {
      line_len_without_n = line_len;
    }

    if (line_len_without_n != target_len) {
      continue;
    }
    // compare input string and line beginning at start index
    result = strncmp(str, line + start_idx, str_len);
    if (result == 0) {
      if (check_letters(line)) {
        // if it contains char that is not a letter
        continue;
      }

      printf("%s", line);
    }
  }
  fclose(fp);

  return 0;
}
