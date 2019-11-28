#include <iostream>
#include <unistd.h>
#include <unordered_map>
#define GET_PAGESIZE() sysconf(_SC_PAGESIZE)
using namespace std;

enum SlabType { SMALL , LARGE};
struct mem_slab;

struct mem_bufctl {
    struct mem_bufctl * next_bufctl; //also freelist linkage
    struct mem_bufctl * prev_bufctl; //used in mem_cahce_destroy
    void * buff;
    struct mem_slab * parent_slab;

};


struct mem_slab {
    struct mem_slab * next_slab, *prev_slab;
    int refcount;
    struct mem_bufctl * free_buffctls;
    void * mem;
    unsigned int align;
    unsigned int color;
    char * bitvec;
    // int max_relevant_bit = cache->objs_per_slab;

};

struct mem_cache {
    char * name;
    size_t objsize;
    unsigned int align;

    unsigned int objs_per_slab;
    void (*constructor)(void *, size_t);
    void (*destructor)(void *, size_t);


    struct mem_slab * free_slabs; //first non-empty slab LL
    struct mem_slab * slabs; //doubly linked list of slabs /(not circ)
    struct mem_slab * lastslab;

    unsigned int lastcolor;

    unordered_map< void* , void*> btobctl;
    //create hash table
    //for small objects this hash gives the slab address

    SlabType slabtype;



};

struct mem_cache *mem_cache_create (
        char * name,
        size_t objsize,
        unsigned int objs_per_slab,
        unsigned int align,
        void (*constructor)(void *, size_t),
        void (*destructor)(void *, size_t)
    ) {

    struct mem_cache * cache = new mem_cache ();
    cache->name = name;
    cache->objsize = objsize;
    cache->objs_per_slab = objs_per_slab;
    cache->align = align;
    cache->constructor = constructor;
    cache->destructor = destructor;
    cache->co






    return cache;

}


//flags not supported by mmap
void * mem_cache_alloc (struct mem_cache * cache) {


}

void mem_cache_free ( struct mem_cache * cache, void * buff) {


}

void mem_cache_destroy (struct mem_cache * cache) {
//TODO: remember to munmap!
}

void mem_cache_grow (struct mem_cache * cache) {

}

void mem_cache_reap (struct mem_cache * cache) {

}

int main()
{
    cout << GET_PAGESIZE() << endl;
    return 0;
}

