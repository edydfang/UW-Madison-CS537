// Copyright [2019] <Yidong Fang>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_COMMAND_LEN 512

static const char INVALID_ARG[] = "Usage: mysh [batchFile]\n";
static const char PROPMT[] = "mysh> ";


int main(int argc, char** argv) {
  FILE* batch_fd = NULL;
  if (argc > 2) {
    write(STDERR_FILENO, INVALID_ARG, sizeof(INVALID_ARG));
    return 1;
  } else if (argc == 1) {
    // interactive mode
    while (1) {
      char command[MAX_COMMAND_LEN];
      write(STDERR_FILENO, PROPMT, sizeof(PROPMT));
      fgets(command, MAX_COMMAND_LEN, stdin);
      printf("%s", command);
    }

  } else {
    // batch mode
    batch_fd = fopen(argv[1], "r");
    if (batch_fd == NULL) {
      // failed to open batch file
      // const int msg_size = 30 + strlen(argv[1]);
      // char* err_msg = (char*)malloc(msg_size);
      // snprintf(err_msg, msg_size, "Error: Cannot open file %s\n", argv[1]);
      // write(STDERR_FILENO, err_msg, strlen(err_msg));
      // free(err_msg);
      fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
      fflush(stderr);
      return 1;
    }
  }

  if (batch_fd) {
    fclose(batch_fd);
  }

  return 0;
}
