#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mapreduce.h"
#include <pthread.h>

void Map(char *file_name) {
    FILE *fp = fopen(file_name, "r");
    assert(fp != NULL);

    char* line = malloc(100);
    while (fgets(line, 100, fp)) {
        line[strlen(line) - 1] = '\0';
        MR_Emit(line, "dummy");
    }
    free(line);
    fclose(fp);
}

void Reduce(char *key, Getter get_next, int partition_number) {
    int count = 0;
    char *value;
    while ((value = get_next(key, partition_number)) != NULL)
        count += atoi(value);
    printf("%s\n", key);
}

int main(int argc, char *argv[]) {
    int argc_alt = argc - 3;
    int num_mapper = atoi(argv[argc_alt]);
    int num_reducer = atoi(argv[argc_alt + 1]);
    int num_partition = atoi(argv[argc_alt + 2]);
    argv[argc_alt] = "\0";
    MR_Run(argc_alt, argv, Map, num_mapper, Reduce, num_reducer, MR_SortedPartition, num_partition);
}