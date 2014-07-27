#ifndef __CACHE_H__
#define __CACHE_H__

#include "csapp.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

typedef struct {
	char * name;
	char * buf;
	int obj_size;
	int age;
} cache_obj;

cache_obj* in_cache(char* name, int num_entries, cache_obj* cache);
cache_obj* cache_init(cache_obj* cache);
void cache_deinit(cache_obj* cache);
int size(cache_obj* cache, int num_entries);
int cache_write(char* name, char buf[], int buf_bytes, int num_entries, cache_obj* cache);
cache_obj* cache_evict(cache_obj* cache, int num_entries);

#endif /* __CACHE_H__ */
