#include "cache.h"

static size_t cache_size = 0;
static cache_obj* last = NULL;

cache_obj* in_cache(char* name, int num_entries, cache_obj* cache){
    int i = 0;
    while(i < num_entries){
        if(!strcmp(name, cache->name)){
            //return to be read

/* I think we may need to reevaluate this, or I need to have this explained to me
 * Why are we aging a cache line when we successfully find it? Usually when a cache
 * hit occurs the entires age is reset because it has been recently referenced.
 * A cache entry should be aged every time it is checked and reset if it is a hit.
 */

            cache->age++;
            return cache;
        }
        if(cache->next == NULL)
            break;
        cache = cache->next;
    }
    return NULL;
}

cache_obj* cache_init(cache_obj* cache, char* name, char buf[]){
    size_t alloc_size = (strlen(name)+1) + (strlen(buf)+1) + (2*sizeof(int)) + sizeof(cache_obj*);
    cache = Malloc(sizeof(cache_obj));

    cache->name = (char *)Malloc(strlen(name)+1);
    strcpy(cache->name, name);

    cache->buf = (char *)Malloc(strlen(buf)+1);
    strcpy(cache->buf, buf);

    cache->obj_size = alloc_size;
    cache->age = 0;
    cache->next = NULL;
    last = cache;
    cache_size = alloc_size;
    //TODO:check if zeroing is necessary
    return cache;
}

/* Appends entry to the end of the cache, updates the last pointer
 */
int set_next(cache_obj* entry){
    if(last->next == NULL){
        //return to be read
        last->next = entry;
        last = entry;
        return 0;
    }
    return 1;
}

cache_obj* add_obj(cache_obj* cache, int num_entries, char* name, char buf[]){
    cache_obj* spot;
    int next_set;
    size_t alloc_size = (strlen(name)+1) + (strlen(buf)+1) + (2*sizeof(int)) + sizeof(cache_obj*);

    if(cache_size == MAX_CACHE_SIZE) {
        spot = cache_evict(cache, num_entries, alloc_size);
        free_spot(spot);
    } else {
        spot = (cache_obj*)Malloc(sizeof(cache_obj));
    }

    spot->name = (char *)Malloc(strlen(name)+1);

    spot->buf = (char *)Malloc(strlen(buf)+1);

    strcpy(spot->name, name);
    strcpy(spot->buf, buf);
    cache_size += alloc_size;
    spot->obj_size = alloc_size;
    spot->age = 0;
    spot->next = NULL;
    next_set = set_next(spot);

    if(next_set == 1)
        return NULL;
    return spot;
}

void free_spot(cache_obj* entry){
    cache_size -= entry->obj_size;

    Free(entry->name);
    Free(entry->buf);
}
void cache_deinit(cache_obj* cache, int num_entries){
    int i;
    cache_obj* next;
    for(i = 0; i < num_entries; i++){
        next = cache->next;
        free_spot(cache);
        Free(cache);
        if(next == NULL)
            break;
        else cache = next;
    }
    cache_size = 0;
}

int size(cache_obj* cache, int num_entries){
    int size = 0;
    int i = 0;
    // sum up obj_size of cache_obj or keep a global variable

    //check that whatever is after the last entry in a full
    //cache is NULL
    //TODO: can we update this to use cache_size for a O(1) implementation?
    while(i < num_entries){
        size += cache[i].obj_size;
        if(cache[i].next == NULL)
            break;
        i++;
    }
    return size;
}

cache_obj* cache_write(char* name, char buf[], int num_entries, cache_obj* cache){
    cache_obj* spot;
    // check that num_entries does what it should

    if(cache == NULL){
        spot = cache_init(cache, name, buf);
        return spot;
    }
    //TODO: what's the purpose of spot here?
    spot = add_obj(cache, num_entries, name, buf);
    return cache;
}

cache_obj* cache_evict(cache_obj* cache, int num_entries, size_t alloc_size){
    int i = 0;
    cache_obj* eldest = cache;

    while(i < num_entries){
        if(eldest->age < cache->age && alloc_size <= cache->obj_size)
            eldest = cache;
        else if (cache->next == NULL)
            break;
        cache = cache->next;
        i++;
    }
    return eldest;
}                                                                                                                                                              
