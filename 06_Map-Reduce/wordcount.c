#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mapreduce.h"
#include <pthread.h>

void Map(char *file_name) {
    FILE* fp = fopen(file_name, "r");
    assert(fp != NULL);

    char* line = malloc(100);
    while (fgets(line, 100, fp)) {
        line[strlen(line) - 1] = '\0';
        MR_Emit(line, "1");
    }
    printf("Mapper %u %s\n", (uint32_t) pthread_self(), file_name);
    free(line);
    fclose(fp);
}

void Reduce(char *key, Getter get_next, int partition_number) {
    // Do some dummy things about files so that this thread is guaranteed to yield
    FILE* dummy = fopen("dummy.txt", "r");
    if (dummy) {
        int d = 0;
        for (int i = 0; i < 64; ++i) {
            int c = getc(dummy);
            d += c;
        }
        fclose(dummy);
    }

    int count = 0;
    char *value;
    while ((value = get_next(key, partition_number)) != NULL)
        count += atoi(value);
    printf("Reducer %s %d %d %u\n", key, count, partition_number, (uint32_t) pthread_self());
}

int main(int argc, char *argv[]) {
    // To ease testing, we read numbers from the commandline, and forge argc and argv passed into MR_Run
    // as if they only contains input file names

    int argc_alt = argc - 3;
    int num_mapper = atoi(argv[argc_alt]);
    int num_reducer = atoi(argv[argc_alt + 1]);
    int num_partition = atoi(argv[argc_alt + 2]);
    argv[argc_alt] = "\0";
    MR_Run(argc_alt, argv, Map, num_mapper, Reduce, num_reducer, MR_DefaultHashPartition, num_partition);
}