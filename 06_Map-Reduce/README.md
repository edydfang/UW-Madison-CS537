# Map-Reduce

In 2004, engineers at Google introduced a new paradigm for large-scale parallel data processing known as MapReduce (the original paper is [here](https://static.googleusercontent.com/media/research.google.com/en//archive/mapreduce-osdi04.pdf); as a bonus, see that authors of the citations at the end). MapReduce makes developers efficiently and easily program large-scale clusters, especially for tasks with a lot of data processing. With Map Reduce, the developer can focus on writing their task as a set of Map and Reduce functions, and let the underlying infrastructure handle parallelism/concurrency, machine crashes, and other complexities common within clusters of machines.

In this project, we built a simplified version of MapReduce for just a single machine. During that, we tried to build efficient and correct concurrency support. 

Three specific objectives to this assignment:

- To gain an understanding of the fundamentals of the MapReduce paradigm.
- To implement a correct and efficient MapReduce framework using threads and related functions.
- To gain experience designing and developing concurrent code.

## Implementation Details

The API of our MapReduce Library can be found in `MapReduce.h`.

The MapReduce program can be divided into three phases.

### Running Phase

- Mapping: In the mapping phase, it feeds the input arguments into mappers and stores the emitted key-value pairs into our storage data structure. The storage data structure has multiple partitions, and it's partition function's responsibility to figure out which partition a key should belong to.
- Sorting: In the sorting phase, it uses multi-thread sorting threads to sort the keys inside each partition.
- Reducing: In reducing phase, it invokes one reducer per partition to process the key-value pairs in the data structure. 

### Data Structures

In this project, we only use an array-based data structure for fast data retrieval. How can we handle the unknown number of key-value
pairs? We achieve it by expending the size of the array dynamically, and each time we multiply the size by two, which makes our program
runs efficiently.

More about data structure:

- All data: Array of partition;
- In each partition: Array of key-value pair; Parameters of the partitions;
- In each key-value pair: pointer to key; pointer to value;


### Some Important Points
- Although I don't know what the purpose of doing so (still no ordering guarantee), we ensure that partition `i` is sent to a reducer before partition `i+1`. 
- As the signature of the Getter function only accepts a string as key arguments, we use data structure from [https://github.com/rxi/map](https://github.com/rxi/map) to get O(1) access of index of that key.
- The good point of array-based implementation is that we can use `qsort` during the sorting phase. 


## How to build or test
```shell
# build wordcount
make
make test-wordcount
make gdb-wordcount
```

## TODO

- use reduce threads to do sorting inside partition
