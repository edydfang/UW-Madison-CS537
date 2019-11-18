
// Copyright [2019] <Yidong Fang>

/**
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <string.h>

/**
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef MAP_H
#define MAP_H
#define MAP_VERSION "0.1.0"

struct map_node_t;
typedef struct map_node_t map_node_t;

typedef struct {
  map_node_t **buckets;
  unsigned nbuckets, nnodes;
} map_base_t;

typedef struct {
  unsigned bucketidx;
  map_node_t *node;
} map_iter_t;

#define map_t(T)     \
  struct {           \
    map_base_t base; \
    T *ref;          \
    T tmp;           \
  }

#define map_init(m) memset(m, 0, sizeof(*(m)))

#define map_deinit(m) map_deinit_(&(m)->base)

#define map_get(m, key) ((m)->ref = map_get_(&(m)->base, key))

#define map_set(m, key, value) \
  ((m)->tmp = (value), map_set_(&(m)->base, key, &(m)->tmp, sizeof((m)->tmp)))

#define map_remove(m, key) map_remove_(&(m)->base, key)

#define map_iter(m) map_iter_()

#define map_next(m, iter) map_next_(&(m)->base, iter)

void map_deinit_(map_base_t *m);
void *map_get_(map_base_t *m, const char *key);
int map_set_(map_base_t *m, const char *key, void *value, int vsize);
void map_remove_(map_base_t *m, const char *key);
map_iter_t map_iter_(void);
const char *map_next_(map_base_t *m, map_iter_t *iter);

typedef map_t(void *) map_void_t;
typedef map_t(char *) map_str_t;
typedef map_t(int) map_int_t;
typedef map_t(char) map_char_t;
typedef map_t(float) map_float_t;
typedef map_t(double) map_double_t;

#endif

struct map_node_t {
  unsigned hash;
  void *value;
  map_node_t *next;
  /* char key[]; */
  /* char value[]; */
};

static unsigned map_hash(const char *str) {
  unsigned hash = 5381;
  while (*str) {
    hash = ((hash << 5) + hash) ^ *str++;
  }
  return hash;
}

static map_node_t *map_newnode(const char *key, void *value, int vsize) {
  map_node_t *node;
  int ksize = strlen(key) + 1;
  int voffset = ksize + ((sizeof(void *) - ksize) % sizeof(void *));
  node = malloc(sizeof(*node) + voffset + vsize);
  if (!node) return NULL;
  memcpy(node + 1, key, ksize);
  node->hash = map_hash(key);
  node->value = ((char *)(node + 1)) + voffset;
  memcpy(node->value, value, vsize);
  return node;
}

static int map_bucketidx(map_base_t *m, unsigned hash) {
  /* If the implementation is changed to allow a non-power-of-2 bucket count,
   * the line below should be changed to use mod instead of AND */
  return hash & (m->nbuckets - 1);
}

static void map_addnode(map_base_t *m, map_node_t *node) {
  int n = map_bucketidx(m, node->hash);
  node->next = m->buckets[n];
  m->buckets[n] = node;
}

static int map_resize(map_base_t *m, int nbuckets) {
  map_node_t *nodes, *node, *next;
  map_node_t **buckets;
  int i;
  /* Chain all nodes together */
  nodes = NULL;
  i = m->nbuckets;
  while (i--) {
    node = (m->buckets)[i];
    while (node) {
      next = node->next;
      node->next = nodes;
      nodes = node;
      node = next;
    }
  }
  /* Reset buckets */
  buckets = realloc(m->buckets, sizeof(*m->buckets) * nbuckets);
  if (buckets != NULL) {
    m->buckets = buckets;
    m->nbuckets = nbuckets;
  }
  if (m->buckets) {
    memset(m->buckets, 0, sizeof(*m->buckets) * m->nbuckets);
    /* Re-add nodes to buckets */
    node = nodes;
    while (node) {
      next = node->next;
      map_addnode(m, node);
      node = next;
    }
  }
  /* Return error code if realloc() failed */
  return (buckets == NULL) ? -1 : 0;
}

static map_node_t **map_getref(map_base_t *m, const char *key) {
  unsigned hash = map_hash(key);
  map_node_t **next;
  if (m->nbuckets > 0) {
    next = &m->buckets[map_bucketidx(m, hash)];
    while (*next) {
      if ((*next)->hash == hash && !strcmp((char *)(*next + 1), key)) {
        return next;
      }
      next = &(*next)->next;
    }
  }
  return NULL;
}

void map_deinit_(map_base_t *m) {
  map_node_t *next, *node;
  int i;
  i = m->nbuckets;
  while (i--) {
    node = m->buckets[i];
    while (node) {
      next = node->next;
      free(node);
      node = next;
    }
  }
  free(m->buckets);
}

void *map_get_(map_base_t *m, const char *key) {
  map_node_t **next = map_getref(m, key);
  return next ? (*next)->value : NULL;
}

int map_set_(map_base_t *m, const char *key, void *value, int vsize) {
  int n, err;
  map_node_t **next, *node;
  /* Find & replace existing node */
  next = map_getref(m, key);
  if (next) {
    memcpy((*next)->value, value, vsize);
    return 0;
  }
  /* Add new node */
  node = map_newnode(key, value, vsize);
  if (node == NULL) goto fail;
  if (m->nnodes >= m->nbuckets) {
    n = (m->nbuckets > 0) ? (m->nbuckets << 1) : 1;
    err = map_resize(m, n);
    if (err) goto fail;
  }
  map_addnode(m, node);
  m->nnodes++;
  return 0;
fail:
  if (node) free(node);
  return -1;
}

void map_remove_(map_base_t *m, const char *key) {
  map_node_t *node;
  map_node_t **next = map_getref(m, key);
  if (next) {
    node = *next;
    *next = (*next)->next;
    free(node);
    m->nnodes--;
  }
}

map_iter_t map_iter_(void) {
  map_iter_t iter;
  iter.bucketidx = -1;
  iter.node = NULL;
  return iter;
}

const char *map_next_(map_base_t *m, map_iter_t *iter) {
  if (iter->node) {
    iter->node = iter->node->next;
    if (iter->node == NULL) goto nextBucket;
  } else {
  nextBucket:
    do {
      if (++iter->bucketidx >= m->nbuckets) {
        return NULL;
      }
      iter->node = m->buckets[iter->bucketidx];
    } while (iter->node == NULL);
  }
  return (char *)(iter->node + 1);
}

// =================== Defination of map above ================================
// =================== Defination of Mapreduce below ==========================

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#include "./mapreduce.h"

#define INIT_MAX_KV_PAIR_PARTITION 2
typedef u_int64_t ulong;

// data structure for intermediate values
typedef struct __kv_pair {
  char *key;
  char *value;
} kv_pair;

typedef struct __partition_data {
  // store data here (dynamicall expend)
  kv_pair **kv_pair_list;
  ulong size_of_list;
  ulong next_to_fill;
  ulong num_keys;
  ulong *start_idxs;  // range [start_idx, end_idx)
  ulong *end_idxs;
  ulong *cur_idxs;
  ulong cur_key_idx;
  map_int_t m;
  sem_t sent_flag;
  pthread_mutex_t lock;
} partition_data;

typedef struct __arg_next {
  int idx;
  pthread_mutex_t lock;  // each file can only be modified by one mapper
} arg_next;

// static var (assumption: only one main thread)
Partitioner my_partitionar = NULL;
Reducer my_reduer = NULL;
Mapper my_mapper = NULL;
int my_num_partitions = 0;
int cur_reduce_partition;
int input_count = 0;
char **input_args = NULL;
arg_next my_arg_next;
partition_data **partition_list = NULL;
pthread_t *my_mapper_threads = NULL;
pthread_t *my_reducer_threads = NULL;
pthread_t *my_sort_threads = NULL;
pthread_mutex_t cur_partition_lock;
// sem_t sem_all_finished;

ulong MR_DefaultHashPartition(char *key, int num_partitions) {
  ulong hash = 5381;
  int c;
  while ((c = *key++) != '\0') hash = hash * 33 + c;
  return hash % num_partitions;
}

ulong MR_SortedPartition(char *key, int num_partitions) {
  // key assumption: 32-bit unsigned integer
  // num_partitions assumption: power of 2
  u_int64_t key_num = strtoul(key, NULL, 0);
  ulong assigned_partition = (key_num * num_partitions) >> 32;
  return assigned_partition;
}

void MR_Emit(char *key, char *value) {
  ulong partition_num = my_partitionar(key, my_num_partitions);
  // put kv pair in to partition
  partition_data *partition = partition_list[partition_num];
  pthread_mutex_lock(&partition->lock);
  if (partition->next_to_fill == partition->size_of_list) {
    // we need to expend the array
    ulong original_size = partition->size_of_list;
    partition->size_of_list = original_size * 2;

    kv_pair **new_kv_pair_list =
        (kv_pair **)malloc(sizeof(kv_pair *) * partition->size_of_list);
    // copy the original values to new address
    memcpy(new_kv_pair_list, partition->kv_pair_list,
           sizeof(kv_pair *) * original_size);
    // free the original one
    // printf("%lu %d free\n", partition_num, original_size);
    free(partition->kv_pair_list);
    // update the structure
    partition->kv_pair_list = new_kv_pair_list;
  }
  // add new pair
  kv_pair *new_pair = (kv_pair *)malloc(sizeof(kv_pair));
  char *key_copy = (char *)malloc(strlen(key) + 1);
  char *value_copy = (char *)malloc(strlen(value) + 1);

  snprintf(key_copy, strlen(key) + 1, "%s", key);
  snprintf(value_copy, strlen(value) + 1, "%s", value);
  // strcpy(key_copy, key);
  // strcpy(value_copy, value);
  new_pair->key = key_copy;
  new_pair->value = value_copy;
  partition->kv_pair_list[partition->next_to_fill++] = new_pair;
  pthread_mutex_unlock(&partition->lock);
}

void init_partition_list() {
  partition_list =
      (partition_data **)malloc(sizeof(partition_data *) * my_num_partitions);
  for (int i = 0; i < my_num_partitions; i++) {
    partition_list[i] = malloc(sizeof(partition_data));
    pthread_mutex_init(&(partition_list[i]->lock), NULL);
    partition_list[i]->size_of_list = INIT_MAX_KV_PAIR_PARTITION;
    partition_list[i]->kv_pair_list =
        (kv_pair **)malloc(sizeof(kv_pair *) * INIT_MAX_KV_PAIR_PARTITION);
    memset(partition_list[i]->kv_pair_list, 0,
           sizeof(kv_pair *) * INIT_MAX_KV_PAIR_PARTITION);
    partition_list[i]->next_to_fill = 0;
    sem_init(&partition_list[i]->sent_flag, 0, 0);
  }
}

void init_mapper_concurrency() {
  my_arg_next.idx = 0;
  pthread_mutex_init(&(my_arg_next.lock), NULL);
}

void init_reducer_concurrency() {
  cur_reduce_partition = 0;
  pthread_mutex_init(&cur_partition_lock, NULL);
}

void map_control() {
  // each mapper take one of arguments, mutual exclusion here
  int next_idx;
  while (1) {
    pthread_mutex_lock(&my_arg_next.lock);
    next_idx = my_arg_next.idx;
    my_arg_next.idx += 1;
    if (next_idx >= input_count) {
      pthread_mutex_unlock(&my_arg_next.lock);
      break;
    }
    pthread_mutex_unlock(&my_arg_next.lock);
    my_mapper(input_args[next_idx]);
  }
  // multi-thread sorting for each partition in mapping threads
}

int kv_pair_comparator(const void *pair1, const void *pair2) {
  kv_pair **p1 = (kv_pair **)pair1;
  kv_pair **p2 = (kv_pair **)pair2;
  // printf("comparing %s, %s\n", (*p1)->key, (*p2)->key);
  return strcmp((*p1)->key, (*p2)->key);
}

void sort_partition(int partition_idx) {
  // sort kv pair inside each partitions according to keys
  partition_data *partition = partition_list[partition_idx];
  // printf("sorting %d, size %d\n", partition_idx, partition->next_to_fill);
  qsort(partition->kv_pair_list, partition->next_to_fill, sizeof(kv_pair *),
        kv_pair_comparator);
  // get start index and end index of each key
  // count how many distinct keys
  ulong num_keys = 0;
  char *key_tmp = 0;
  char *key_cmp;
  for (size_t i = 0; i < partition->next_to_fill; i++) {
    key_cmp = partition->kv_pair_list[i]->key;
    if (key_tmp == 0 || strcmp(key_cmp, key_tmp) != 0) {
      num_keys++;
      key_tmp = key_cmp;
      //   printf("partition: %d  key %s\n", partition_idx, key_tmp);
      //   fflush(stdout);
    }
  }
  // get start and end idx
  partition->num_keys = num_keys;
  partition->start_idxs = (ulong *)malloc(sizeof(ulong) * num_keys);
  partition->end_idxs = (ulong *)malloc(sizeof(ulong) * num_keys);
  partition->cur_idxs = (ulong *)malloc(sizeof(ulong) * num_keys);
  // init those idxs
  // printf("bbb %d\n", partition_idx);
  map_init(&partition->m);
  int cur_key_idx = -1;
  for (size_t i = 0; i < partition->next_to_fill; i++) {
    key_cmp = partition->kv_pair_list[i]->key;
    if (strcmp(key_cmp, key_tmp) != 0) {
      num_keys++;
      if (cur_key_idx > -1) {
        partition->end_idxs[cur_key_idx] = i;
      }
      cur_key_idx++;
      partition->start_idxs[cur_key_idx] = i;
      map_set(&partition->m, key_cmp, cur_key_idx);
      // printf("setting %d %s %d\n", partition_idx, key_cmp, cur_key_idx);
      partition->cur_idxs[cur_key_idx] = i;
      key_tmp = key_cmp;
    }
  }
  if (cur_key_idx != -1) {
    // if no data
    partition->end_idxs[cur_key_idx] = partition->next_to_fill;
  }
  partition->cur_key_idx = 0;
  return;
}

char *Get(char *key, int num_partition) {
  // func to get next value for specific key and partition
  partition_data *partition = partition_list[num_partition];
  int key_idx = *map_get(&partition->m, key);
  ulong cur_kv_pair_idx = partition->cur_idxs[key_idx];
  ulong end_idx = partition->end_idxs[key_idx];
  if (cur_kv_pair_idx >= end_idx) {
    return NULL;
  } else {
    partition->cur_idxs[key_idx]++;
    return partition->kv_pair_list[cur_kv_pair_idx]->value;
  }
}

void reduce_controller() {
  /*detailed spec https://piazza.com/class/jyivrc1wvcv7dh?cid=1137
  Partitions cannot be subdivided across reducers; each partition goes to
  exactly one reducer. Partition i must be *sent* to a reducer before partition
  i+1 is started. It doesn't have to be completed.
  */
  int partition_to_reduce = -1;

  while (1) {
    // first assign a partition to a thread
    pthread_mutex_lock(&cur_partition_lock);
    partition_to_reduce = cur_reduce_partition;
    if (partition_to_reduce >= my_num_partitions) {
      // all partitions are done
      pthread_mutex_unlock(&cur_partition_lock);
      break;
    }
    cur_reduce_partition++;
    pthread_mutex_unlock(&cur_partition_lock);

    // do sort first 
    partition_data *partition = partition_list[partition_to_reduce];
    sort_partition(partition_to_reduce);
    if (partition_to_reduce > 0) {
      // ensure partition sent in order here
      sem_wait(&partition_list[partition_to_reduce - 1]->sent_flag);
    }

    for (size_t i = 0; i < partition->num_keys; i++) {
      // reduce for each key
      ulong kv_pair_idx = partition->start_idxs[i];
      char *key = partition->kv_pair_list[kv_pair_idx]->key;
      // printf("reduceing partition %d, key %s\n", partition_to_reduce, key);
      my_reduer(key, Get, partition_to_reduce);
      sem_post(&partition->sent_flag);
    }
    // if no key in the partition, we still need post semaphone
    if (partition->num_keys == 0) {
      sem_post(&partition->sent_flag);
    }
  }
}

void destruct_partition_list() {
  for (size_t i = 0; i < my_num_partitions; i++) {
    // printf("freeing\n");
    partition_data *partition = partition_list[i];
    // for each partition
    for (size_t j = 0; j < partition->next_to_fill; j++) {
      // free kv pair
      free(partition->kv_pair_list[j]->key);
      free(partition->kv_pair_list[j]->value);
      free(partition->kv_pair_list[j]);
    }
    free(partition->kv_pair_list);
    free(partition->start_idxs);
    free(partition->end_idxs);
    free(partition->cur_idxs);
    map_deinit(&partition->m);
    free(partition);
  }
  free(partition_list);
}

void MR_Run(int argc, char *argv[], Mapper map, int num_mappers, Reducer reduce,
            int num_reducers, Partitioner partition, int num_partitions) {
  // set shared func or params
  /*
  int num_sort_threads = 0;
  if (num_partitions < num_reducers || num_partitions < num_mappers) {
    num_sort_threads = num_partitions;
  } else {
    num_sort_threads = num_mappers > num_reducers ? num_mappers : num_reducers;
  }
  */
  my_partitionar = partition;
  my_num_partitions = num_partitions;
  my_mapper = map;
  my_reduer = reduce;
  // the first params is useless
  input_count = argc - 1;
  input_args = argv + sizeof(char);
  // init partitions
  init_partition_list();
  // init threads
  my_mapper_threads = (pthread_t *)malloc(num_mappers * sizeof(pthread_t));
  my_reducer_threads = (pthread_t *)malloc(num_reducers * sizeof(pthread_t));
  init_mapper_concurrency();
  // run mappers
  for (int i = 0; i < num_mappers; i++) {
    pthread_create(&my_mapper_threads[i], NULL, (void *)&map_control, NULL);
  }
  for (int i = 0; i < num_mappers; i++) {
    pthread_join(my_mapper_threads[i], NULL);
  }

  // run reducers; sort keys inside each partition in those threads
  for (int i = 0; i < num_reducers; i++) {
    pthread_create(&my_reducer_threads[i], NULL, (void *)&reduce_controller,
                   NULL);
  }
  for (int i = 0; i < num_reducers; i++) {
    pthread_join(my_reducer_threads[i], NULL);
  }
  // destrcut everything here
  destruct_partition_list();
  free(my_mapper_threads);
  free(my_sort_threads);
  free(my_reducer_threads);
  return;
}
