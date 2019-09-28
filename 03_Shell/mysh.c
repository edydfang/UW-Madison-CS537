// Copyright [2019] <Yidong Fang>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_COMMAND_LEN 512
#define MAX_PARSE_NUM 256
#define WAIT_CMD "wait"
#define JOBS_CMD "jobs"
#define EXIT_CMD "exit"
#define PROPMT "mysh> "

static const char INVALID_ARG[] = "Usage: mysh [batchFile]\n";

void func_wait(int pid) {
  printf("wait invoked.\n");
  return;
}

void func_jobs() {
  // print all background jobs
}

void func_exit() {
  // exit the shell
  exit(0);
}

int func_general_cmd(const char* program_path, char** argv, int background) {
  /*
  background 1: run int the backgrond
  background 0: wait the child
  */
  pid_t child_pid, tpid;
  int child_status;

  child_pid = fork();
  if (child_pid == 0) {
    // child process
    execvp(program_path, argv);
    fprintf(stderr, "%s: Command not found\n", program_path);
    fflush(stderr);
  } else {
    // parent process
    if (!background) {
      // wait the child process to end
      do {
        tpid = wait(&child_status);
      } while (tpid != child_pid);
    }
    // add child process into the data structure
    return 0;
  }
}

int check_built_in(char** arg_arr, int num_arg) {
  /*
  0 - no built-in keyword
  1 - built-in keywrod activated
  */
  if (num_arg == 1) {
    if (strcmp(EXIT_CMD, arg_arr[0]) == 0) {
      func_exit();
      return 1;
    } else if (strcmp(JOBS_CMD, arg_arr[0]) == 0) {
      func_jobs();
      return 1;
    }
  } else if (num_arg == 2 && strcmp(WAIT_CMD, arg_arr[0]) == 0) {
    // wait command
    // check whether the 2nd argument is a number first
    return 1;
  }
  return 0;
}

int parse_command(char* command) {
  /*
  built-in shell command: exit, jobs, wait
  return 0: normal
  return 1: exit
  */
  char* arg_arr[MAX_PARSE_NUM];
  char* token = strtok_r(command, " ", &command);
  int num_parse = 0, bg_flag = 0;
  // store all arguments into the arg_arr
  while (token) {
    arg_arr[num_parse++] = token;
    // printf("%s\n", token);
    // fflush(stdout);
    token = strtok_r(NULL, " ", &command);
  }
  if (num_parse == 0) {
    // do nothing
    return 1;
  }
  // remove newline char from the last token
  int len = strlen(arg_arr[num_parse - 1]);
  arg_arr[num_parse - 1][len - 1] = '\0';
  // check bg flag
  if (strcmp("&", arg_arr[0]) == 0) {
    bg_flag = 1;
  }
  // char* argv_new[num_parse - bg_flag];
  // for (size_t i = 0; i < num_parse - bg_flag; i++) {
  //   argv_new[i] = arg_arr[i];
  // }
  arg_arr[num_parse - bg_flag] = NULL;

  // check keyword function
  if (!check_built_in(arg_arr, num_parse - bg_flag)) {
    func_general_cmd(arg_arr[0], arg_arr, bg_flag);
  }

  return 0;
}

int main(int argc, char** argv) {
  FILE* batch_fd = NULL;
  int eof_indicator = 0;
  if (argc > 2) {
    write(STDERR_FILENO, INVALID_ARG, sizeof(INVALID_ARG));
    return 1;
  } else if (argc == 1) {
    // interactive mode
    while (1) {
      char command[MAX_COMMAND_LEN];
      write(STDERR_FILENO, PROPMT, sizeof(PROPMT));
      fgets(command, MAX_COMMAND_LEN, stdin);
      eof_indicator = feof(stdin);
      if (eof_indicator) {
        printf("\n");
        fflush(stdout);
        break;
      }
      parse_command(command);
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
