#include "freelist.h"
#include "utils.h"
#include "bbm.c"

#include <stdint.h>
#include <stdlib.h>

typedef struct Level {
    void *addr;
    BBM bmap;
} Level;

//Returns pointer to level struct for e in array
static Level *level(Freelist f, int e, int l){
    return &((Level *)f)[(e - l)];
}

extern FreeList freelistcreate(size_t size, int l, int u) {
    int n = u - l + 1;
    Level *lv = mmalloc(n * sizeof *lv);
    if(lv == -1){
        return 0;
    }

    for(int e = l; e <= u; e++){
        lv[e-l].addr = 0;
        lv[e-l].bmap = bbmcreate(size, e);
    }

    //Top level of bbmap should only be a single bit
    lv[u-l].addr = 0;
    lv[u-l].bmap = 0;
    return lv;
}

extern void freelistdelete(FreeList f, int l, int u) {
    Level *lv = level(f, e, l);
    for(int e = l; e <= u; e++){
        bbmdelete(lv[e-l].bmap);
    }
    mmfree(lv, (u - l + 1) * sizeof *lv);
}

extern void *freelistalloc(FreeList f, void *base, int e, int l) {
    Level *lv = level(f, e, l);

    void *block = lv->addr;
    if(block){
        lv->addr = *(void **)block;
        return block;
    }

    if(!lv->bmap){
        return 0;
    }

    void *parent = freelistalloc(f, base, e + 1, l);
    if(!parent){
        return 0;
    }

    void *right = baddrset(base, parent, e);
    lv->addr = right;

    bbmset(lv->bmap, base, parent, e);
    return parent;
}

extern void freelistfree(FreeList f, void *base, void *mem, int e, int l) {
    Level *lv = level(f, e, l);

    if(lv->bmap){
        void *buddy = baddrinv(base, mem, e);
        void **link = &lv->addr;
        //Walk through list to find buddy
        while(*link && *link != buddy){
            link = (void **)*link;
        }

        if(*link == buddy){
            *link = *(void **)buddy;
            bbmclear(lv->bmap, base, mem, e); //clear mergeed half
            void *merged = baddrclr(base, mem, e); //Merged block of buddies
            freelistfree(f, base, merged, e + 1, l);
            return;
        }
    }

    //Push mem onto the free list at this level
    *(void **)mem = lv->addr;
    lv->addr = mem;
}

extern int freelistsize(FreeList f, void *base, void *mem, int l, int u) {
    for(int e = l; e <= u; e++){
        Level *lv = level(f, e, l);
        if(bbmtst(lv->bmap, base, mem, e)){
            return e;
        }
    }
    return u;
}

extern void freelistprint(FreeList f, int l, int u) {
    for(int e = l; e <= u; e++){
        Level *lv = level(f, e, l);
        printf("2^%d (%zu bytes):", e, e2size(e));
        for(void *block = lv->addr; block; block = *(void **)block){
            printf(" %p", block);
        }
        printf("\n");
    }
}
