// Copyright [2019] <Yidong Fang>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *str_to_lower(char *input_str) {
  /*
  turn a char array to lower case
  */
  char *new_str = (char *)malloc(strlen(input_str) + 1);
  // initilize the malloc mem
  memset(new_str, 0, strlen(input_str) + 1);
  int i = 0;
  while (input_str[i]) {
    new_str[i] = tolower(input_str[i]);
    i += 1;
  }
  return new_str;
}

int main(int argc, char **argv) {
  // check arguments count
  if (argc < 2 || argc > 3) {
    printf("my-look: invalid number of arguments\n");
    return 1;
  }
  // assign the argument to the vars
  char *str, *filename;
  str = str_to_lower(argv[1]);
  if (argc == 3) {
    filename = argv[2];
  } else {
    filename = "/usr/share/dict/words";
  }

  // open the dictionary file
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("my-look: cannot open file\n");
    return 1;
  }

  // size 100 is engouth for assmumed condition
  char line[100];
  char *line_lower;
  int result, str_len = strlen(str);
  // read file
  while (fgets(line, 100, fp)) {
    // or we can use strncasecmp for ignorance of the case
    // for comparison, convert the line also to lower case
    line_lower = str_to_lower(line);
    // compare two string at the beginning part
    result = strncmp(str, line_lower, str_len);
    if (result == 0) {
      printf("%s", line);
    }
    // free the sapce created by the function
    free(line_lower);
  }
  // free the lower-case input string
  free(str);
  fclose(fp);

  return 0;
}
