#ifndef __CACHE_H__
#define __CACHE_H__

#include "csapp.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

typedef struct {
    unsigned obj_size;
    int age;
    char * name;
    char * buf;
    struct cache_obj* next;
} cache_obj;


cache_obj* in_cache(char* name, int num_entries, cache_obj* cache);
cache_obj* cache_init(cache_obj* cache, char * name, char buf[]);
cache_obj* add_obj(cache_obj* cache, int num_entries, char* name, char buf[]);
cache_obj* free_spot(cache_obj* entry);
void cache_deinit(cache_obj* cache, int num_entries);
int size(cache_obj* cache, int num_entries);
cache_obj* cache_write(char* name, char buf[], int num_entries, cache_obj* cache);
cache_obj* cache_evict(cache_obj* cache, int num_entries, size_t alloc_size);

#endif /* __CACHE_H__ */
