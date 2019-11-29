#include <iostream>
#include <unistd.h>
#include <unordered_map>
#include <string.h>
#include <sys/mman.h>
#include <math.h>
#define GET_PAGESIZE() sysconf(_SC_PAGESIZE)
using namespace std;

enum SlabType { SMALL, LARGE};
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
    void * bitvec;
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

    unordered_map< void*, struct mem_bufctl *> btobctl;
    unordered_map< void *, pair<struct mem_slab *, unsigned int> > btoslab;
    //create hash table
    //for small objects this hash gives the slab address

    SlabType slabtype;



};


struct mem_slab * mem_allocate_small_slab ( unsigned int objsize,
        unsigned int align,
        unsigned int color,
        unsigned int objs_per_slab,
        void (*constructor) (void *,size_t),
        struct mem_cache * cache) {

    //create a slab object (using malloc, later use mem_cache), initialize, set free_buffctls to null
    struct mem_slab * slab = new mem_slab();
    slab->next_slab= NULL;
    slab->prev_slab = NULL;
    slab->refcount = 0;
    slab->free_buffctls = NULL;
    slab-> align = align;
    slab->color = color;
    unsigned int pagesize = GET_PAGESIZE();


    //mmap objsize bytes, aligned 0 (defaults to “get a page”) store in mem ptr
    slab->mem = mmap(NULL, objsize , PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    //jump to mem + pagesize – sizeof(mem_slab)  (maybe create an inline func to get this)
    void * slabptr = slab->mem + pagesize + slab->color - sizeof(mem_slab);
    memcpy(slabptr, slab, sizeof(struct mem_slab) );


    //cache->objs_per_slab = (int) floor  [ ( pagesize – sizeof(mem_slab) – color ) / objsize ]
    cache->objs_per_slab =  (pagesize - sizeof(mem_slab) - color )/ objsize ;

    //skip color bytes and construct cache->objs_per_slab objects.
    //Keep adding  <buffptr, pair<slab_ptr, buffindex >> to cache.btobctl (hashtable)

    //create dummy object
    void * dummy;
    constructor(dummy, objsize);

    void * tmp = slab->mem + color;
    for ( unsigned int i = 0 ; i < cache->objs_per_slab; i++) {
        memcpy (tmp, dummy, objsize);
        tmp += objsize;

        cache->btoslab[tmp] = make_pair ( (struct mem_slab *)slabptr, i) ;
    }


    //initialize bitvec
    unsigned int bytes_required = (int) ceil ( cache->objs_per_slab / 8 );
    slab->bitvec = malloc (bytes_required);
    memset(slab->bitvec, 0, bytes_required);


    return slab;




}





struct mem_cache *mem_cache_create (
    char * name,
    size_t objsize,
    unsigned int objs_per_slab,
    unsigned int align,
    void (*constructor)(void *, size_t),
    void (*destructor)(void *, size_t)
) {


    //initialize cache object
    struct mem_cache * cache = new mem_cache ();
    cache->name = name;
    cache->objsize = objsize;
    cache->objs_per_slab = objs_per_slab;
    cache->align = align;
    cache->constructor = constructor;
    cache->destructor = destructor;
    cache->lastcolor = 0;

    //check which slab type to create
    unsigned int pagesize = GET_PAGESIZE();
    struct mem_slab * newslab;

    if (objsize > pagesize / 8) {
        cache->slabtype = LARGE;
        //If large, create large slab, update slabtype

    } else {


        cache->slabtype = SMALL;
        //If small, create small slab

        //calculate color
        unsigned int color = 0;


        newslab =   mem_allocate_small_slab ( objsize, align, color, objs_per_slab, constructor, cache);



    }

    //initialize free_slabs and slabs, lastcolor
    cache->free_slabs = cache->slabs = cache->lastslab = newslab;
    unsigned int lastcolor;
    lastcolor = (lastcolor + 8) % 32;
    cache->lastcolor = lastcolor;


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

int main() {
    cout << GET_PAGESIZE() << endl;
    return 0;
}

