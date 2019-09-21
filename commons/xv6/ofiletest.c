#include "fcntl.h"
#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
  // check arguments
  if (argc < 2) {
    // TODO: Check the number rest of arguments is smaller than the number of
    // opened files
    printf(1, "ofiletest: invalid number of parameters\n");
    exit();
  }
  // num of files to open or create
  int num_open = atoi(argv[1]);
  if (num_open < 1 || num_open > 13) {
    // too many files to open or invalid input Max16-3=13
    printf(1, "ofiletest: invalid parameters\n");
    exit();
  }

  // num of files to close
  int num_close = argc - 2;
  int close_vec[num_close];
  for (int i = 0; i < num_close; i++) {
    close_vec[i] = atoi(argv[i + 2]);
    if (close_vec[i] < 0 || close_vec[i] > num_open-1) {
      printf(1, "ofiletest: invalid parameters\n");
      exit();
    }
  }
  int fd[num_open];
  char filename[num_open][8];
  for (int i = 0; i < num_open; i++) {
    char* name = filename[i];
    name[0] = 'o';
    name[1] = 'f';
    name[2] = 'i';
    name[3] = 'l';
    name[4] = 'e';
    if (i < 10) {
      name[5] = '0' + i;
      name[6] = '\0';
    } else {
      name[5] = '0' + i / 10;
      name[6] = '0' + i % 10;
      name[7] = '\0';
    }
    fd[i] = open((char *)name, O_CREATE);
  }

  for (int i = 0; i < num_close; i++) {
    close(fd[close_vec[i]]);
    // delete this file
    // int unlink(const char*);
    unlink(filename[close_vec[i]]);
  }

  int ofilecnt = getofilecnt(getpid());
  int ofilenext = getofilenext(getpid());
  printf(1, "%d %d\n", ofilecnt, ofilenext);

  exit();
}
