#include "./types.h"
#include "./user.h"


int
main(int argc, char *argv[])
{
    while(1){
        sleep(10);
        printf(1, "%d\n", getpid());
    }
    exit();
}