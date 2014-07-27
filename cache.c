#include "cache.h"

cache_obj* in_cache(char* name, int num_entries, cache_obj* cache){
	int i = 0;

	while(i < num_entries){
		if(!strcmp(name, cache[i].name)){
			cache[i].age++;
			return &cache[i];
		}
	}

	return NULL;
}

cache_obj* cache_init(cache_obj* cache){
	return (cache_obj*) Calloc(MAX_CACHE_SIZE/sizeof(cache_obj), sizeof(cache_obj));
}

void cache_deinit(cache_obj* cache){
	Free(cache);
}

int size(cache_obj* cache, int num_entries){
	int size;
	int i = 0;

	while(i < num_entries){
		size += cache[i].obj_size;
		i++;
	}

	return size;
}

int cache_write(char* name, char buf[], int buf_bytes, int num_entries, cache_obj* cache){
	cache_obj* spot;
	// check that num_entries does what it should
	int cache_size = size(cache, num_entries);

	if(cache == NULL){
		spot = cache_init(cache);
	}
	if(cache_size == MAX_CACHE_SIZE){
		//evict last recentely used or some variant of that method
		spot = cache_evict(cache, num_entries);
	}
    else {
		spot = &cache[num_entries]; // check for off by one error	
	}

	spot->name = name;
	spot->buf = buf;
	spot->obj_size = strlen(name) + buf_bytes + 2*sizeof(int);
	spot->age = 0;
	
	return 0;
}

cache_obj* cache_evict(cache_obj* cache, int num_entries){
	int i = 0;
	cache_obj* eldest = cache;

	while(i < num_entries){
	
		if(eldest->age < cache[i].age){
			eldest = &cache[i];
		}

		i++;
	}

	return eldest;
}
