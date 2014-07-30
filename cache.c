/* I removed the Null checks for Malloc because that check is redundant.
 * Malloc with a capitol M is a wrapper defined in csapp.c that performs
 * a null check before returing. If malloc fails then the wrapper calls
 * unix_error and the program exits with error status.
 */
#include "cache.h"

static size_t cache_size = 0;
static cache_obj* last = NULL;

cache_obj* in_cache(char* name, int num_entries, cache_obj* cache){
    int i = 0;
    //printf("in_cache called\n");
    //printf("num_entries is %d\n", num_entries);
    while(i < num_entries){
        //printf("cache is %lx\n", (long)cache);
        if(!strcmp(name, cache->name)){
            //return to be read

/* I think we may need to reevaluate this, or I need to have this explained to me
 * Why are we aging a cache line when we successfully find it? Usually when a cache
 * hit occurs the entires age is reset because it has been recently referenced.
 * A cache entry should be aged every time it is checked and reset if it is a hit.
 */

            cache->age++;
            //printf("in_cache returned positive\n");
            return cache;
        }

        if(cache->next == NULL){
            break;
        }
        cache = cache->next;
    }


    //printf("in_cache returned negative\n");
    return NULL;
}

cache_obj* cache_init(cache_obj* cache, char* name, char buf[]){
    //printf("cache_init called\n");    
    size_t alloc_size = (strlen(name)+1) + (strlen(buf)+1) + 2*sizeof(int) + sizeof(cache_obj*);
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
    //printf("num is %d, cache is %lx\n", num, (long)cache);
    //check if zeroing is necessary
    //printf("cache_init returned\n");  
    return cache;
}

/* I could use some documentation here please
 */
int set_next(cache_obj* entry){
    //printf("in_cache called\n");
    //printf("num_entries is %d\n", num_entries);
    ///while(i < num_entries){
        //printf("cache is %lx\n", (long)cache);
    if(last->next == NULL){
            //return to be read
        last->next = (struct cache_obj*) entry;
        last = entry;
        //printf("in_cache returned positive\n");
        return 0;
    }

    //printf("in_cache returned negative\n");
    return 1;
}

cache_obj* add_obj(cache_obj* cache, int num_entries, char* name, char buf[]){
    cache_obj* spot;
    int next_set;
    //printf("add_obj called\n");
    size_t alloc_size = (strlen(name)+1) + (strlen(buf)+1) + 2*sizeof(int) + sizeof(cache_obj*);

    if(cache_size == MAX_CACHE_SIZE) {
        spot = cache_evict(cache, num_entries, alloc_size);
        free_spot(spot);
    }
          else {
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

    if(next_set == 1){
        return NULL;
    }
    //printf("add_obj ret\n");
    return spot;
}

void free_spot(cache_obj* entry){
    //printf("free_spot called\n");
    cache_size -= entry->obj_size;

    Free(entry->name);
    Free(entry->buf);
    //Free(entry);

     //printf("free_spot ret\n");
}

void cache_deinit(cache_obj* cache, int num_entries){
    int i;
    cache_obj* next;
    for(i = 0; i < num_entries; i++){
        next = cache->next;
        free_spot(cache);
        Free(cache);
        if(next == NULL){
            break;
        }
        else {
            cache = next;
        }
    }
    cache_size = 0;
}

int size(cache_obj* cache, int num_entries){
    int size = 0;
    int i = 0;
    // sum up obj_size of cache_obj or keep a global variable

    //check that whatever is after the last entry in a full
    //cache is NULL
    while(i < num_entries){

        size += cache[i].obj_size;

        if(cache[i].next == NULL){
            break;
        }
        i++;
    }

    return size;
}

cache_obj* cache_write(char* name, char buf[], int num_entries, cache_obj* cache){
    //printf("cache_write called\n");
    cache_obj* spot;
    // check that num_entries does what it should

    //printf("start of write. name is %s, buf is %s, num_entries is %d, cache is %lx\n", name, buf, num_entries, (long)cache);
    if(cache == NULL){
        spot = cache_init(cache, name, buf);
        //printf("init successful, spot is %lx\n", (long)spot);
        return spot;
    }
    else {
        spot = add_obj(cache, num_entries, name, buf);
    }
    //printf("name is %s, buf is %s, obj_size is %d, age is %d\n", spot->name, spot->buf, spot->obj_size, spot->age);   
    //printf("cache_write ret, cache here is %lx\n", (long)cache);
    return cache;
}

cache_obj* cache_evict(cache_obj* cache, int num_entries, size_t alloc_size){
    //printf("cache_evict called\n");
    int i = 0;
    cache_obj* eldest = cache;

    while(i < num_entries){

        if(eldest->age < cache->age && alloc_size <= cache->obj_size){
            eldest = cache;
        }
        else if (cache->next == NULL){
            break;
        }
        cache = cache->next;

        i++;
    }

    //printf("cache_evict ret\n");
    return eldest;
}                                                                                                                                                              
