#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "freelist.h"
#include "balloc.h"


int testsRun = 0;
int testsFailed = 0;

static void testCheck(const char *testName, int passed) {
    testsRun++;
    if (!passed) {
        testsFailed++;
        printf("Test failed: %s\n", testName);
    }   
}

static void testSummary() {
    printf("Tests run: %d, Tests failed: %d\n", testsRun, testsFailed);
    if (testsFailed == 0) {
        printf("All tests passed!\n");
    }
}

//---------------------------------------
// Test functions for utils.c
//---------------------------------------
static void testDivUp(void){
    testCheck("divup(10, 3) == 4", divup(10, 3) == 4);
    testCheck("divup(9, 3) == 3", divup(9, 3) == 3);
    testCheck("divup(0, 1) == 0", divup(0, 1) == 0);
    testCheck("divup(1, 1) == 1", divup(1, 1) == 1);
}

static void testBits2Bytes(void){
    testCheck("bits2bytes(8) == 1", bits2bytes(8) == 1);
    testCheck("bits2bytes(9) == 2", bits2bytes(9) == 2);
    testCheck("bits2bytes(0) == 0", bits2bytes(0) == 0);
    testCheck("bits2bytes(7) == 1", bits2bytes(7) == 1);
}

static void testE2Size(void){
    testCheck("e2size(0) == 1", e2size(0) == 1);
    testCheck("e2size(1) == 2", e2size(1) == 2);
    testCheck("e2size(10) == 1024", e2size(10) == 1024);
}

static void testSize2E(void){
    testCheck("size2e(1) == 0", size2e(1) == 0);
    testCheck("size2e(2) == 1", size2e(2) == 1);
    testCheck("size2e(1024) == 10", size2e(1024) == 10);
}

static void bitManipulationTests(void) {
    unsigned char byte = 0;

    bitset(&byte, 0);
    testCheck("bitset(&byte, 0)", byte == 1);

    bitset(&byte, 1);
    testCheck("bitset(&byte, 1)", byte == 3);

    bitclr(&byte, 0);
    testCheck("bitclr(&byte, 0)", byte == 2);

    bitinv(&byte, 1);
    testCheck("bitinv(&byte, 1)", byte == 0);

    bitset(&byte, 7);
    testCheck("bitset(&byte, 7)", byte == 128);

    testCheck("bittst(&byte, 7)", bittst(&byte, 7) == 1);
    testCheck("bittst(&byte, 0)", bittst(&byte, 0) == 0);
}

static void testmmallocAndmmfree(void) {
    size_t size = 1024; // 1 KB
    void *ptr = mmalloc(size);
    testCheck("mmalloc should not return NULL", ptr != NULL);

    // Optionally, you can write to the allocated memory to ensure it's writable
    unsigned char *bytePtr = (unsigned char *)ptr;
    for (size_t i = 0; i < size; i++) {
        bytePtr[i] = (unsigned char)(i % 256);
    }

    mmfree(ptr, size);
}

//---------------------------------------
// Test functions for freelist.c
//---------------------------------------

static void testFreelistCreateDelete(void) {
    size_t size = 256;
    int l = 4;
    int u = 8;

    FreeList f = freelistcreate(size, l, u);

    testCheck("freelistcreate should not return NULL", f != NULL);

    if (f != NULL) {
        freelistdelete(f, l, u);
    }
}


static void testFreelistFreeAndAlloc(void) {
    size_t size = 256;
    int l = 4;
    int u = 8;

    // Create a region of memory to manage.
    void *base = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    testCheck("mmap should succeed", base != MAP_FAILED);

    if (base == MAP_FAILED) {
        return;
    }

    FreeList f = freelistcreate(size, l, u);
    testCheck("freelistcreate should succeed", f != NULL);

    if (f == NULL) {
        munmap(base, size);
        return;
    }

    // Put a 16-byte block onto the free list.
    void *block = base;

    freelistfree(f, base, block, 4, l);

    void *result = freelistalloc(f, base, 4, l);

    testCheck("freelistalloc should return freed block",
              result == block);

    freelistdelete(f, l, u);
    munmap(base, size);
}


static void testFreelistMultipleBlocks(void) {
    size_t size = 256;
    int l = 4;
    int u = 8;

    void *base = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    testCheck("mmap should succeed", base != MAP_FAILED);

    if (base == MAP_FAILED) {
        return;
    }

    FreeList f = freelistcreate(size, l, u);
    testCheck("freelistcreate should succeed", f != NULL);

    if (f == NULL) {
        munmap(base, size);
        return;
    }

    void *block1 = base;
    void *block2 = (char *)base + 16;

    freelistfree(f, base, block1, 4, l);
    freelistfree(f, base, block2, 4, l);

    void *result1 = freelistalloc(f, base, 4, l);
    void *result2 = freelistalloc(f, base, 4, l);

    testCheck("first allocation after coalescing returns the low half (block1)", result1 == block1);

    testCheck("second allocation after coalescing returns the high half (block2)", result2 == block2);

    freelistdelete(f, l, u);
    munmap(base, size);
}


static void testFreelistReuse(void) {
    size_t size = 256;
    int l = 4;
    int u = 8;

    void *base = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    testCheck("mmap should succeed", base != MAP_FAILED);

    if (base == MAP_FAILED) {
        return;
    }

    FreeList f = freelistcreate(size, l, u);

    testCheck("freelistcreate should succeed", f != NULL);

    if (f == NULL) {
        munmap(base, size);
        return;
    }

    void *block = base;

    // Free a block.
    freelistfree(f, base, block, 4, l);

    // Allocate it.
    void *allocated = freelistalloc(f, base, 4, l);

    testCheck("allocated block should be the freed block", allocated == block);

    // Free it again.
    freelistfree(f, base, allocated, 4, l);

    // It should be reusable.
    void *allocatedAgain = freelistalloc(f, base, 4, l);

    testCheck("freed block should be reusable",
              allocatedAgain == block);

    freelistdelete(f, l, u);
    munmap(base, size);
}


static void testFreelistSize(void) {
    size_t size = 256;
    int l = 4;
    int u = 8;

    void *base = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    testCheck("mmap should succeed", base != MAP_FAILED);

    if (base == MAP_FAILED) {
        return;
    }

    FreeList f = freelistcreate(size, l, u);

    testCheck("freelistcreate should succeed", f != NULL);

    if (f == NULL) {
        munmap(base, size);
        return;
    }

    freelistfree(f, base, base, u, l);
    void *block = freelistalloc(f, base, 4, l);
    testCheck("freelistalloc should produce a 16-byte block", block == base);

    int result = freelistsize(f, base, block, l, u);

    testCheck("freelistsize should identify 16-byte block",
              result == 4);

    freelistdelete(f, l, u);
    munmap(base, size);
}

//---------------------------------------
// Test functions for balloc.c
//---------------------------------------
 
static void testBallocCreateDelete(void) {
    Balloc ba = bcreate(4096, 4, 10);
 
    testCheck("bcreate should not return NULL", ba != NULL);
 
    if (ba != NULL) {
        bdelete(ba);
    }
}
 
 
static void testBallocAllocFree(void) {
    Balloc ba = bcreate(4096, 4, 10);
    testCheck("bcreate should succeed", ba != NULL);
 
    if (ba == NULL) {
        return;
    }
 
    void *p = balloc(ba, 20);
    testCheck("balloc(20) should not return NULL", p != NULL);
 
    // 20 bytes doesn't fit in a 16-byte (2^4) block, so it should be
    // rounded up to the next power of two, 32 (2^5).
    testCheck("bsize(20) should round up to 32", bsize(ba, p) == 32);
 
    unsigned char *bytes = (unsigned char *)p;
    for (unsigned int i = 0; i < bsize(ba, p); i++) {
        bytes[i] = (unsigned char)(i % 256);
    }
 
    bfree(ba, p);
 
    // Re-allocating the same size afterward should succeed again.
    void *p2 = balloc(ba, 20);
    testCheck("re-alloc after free should succeed", p2 != NULL);
 
    bfree(ba, p2);
    bdelete(ba);
}
 
 
static void testBallocBounds(void) {
    Balloc ba = bcreate(4096, 4, 10);
    testCheck("bcreate should succeed", ba != NULL);
 
    if (ba == NULL) {
        return;
    }
 
    void *tiny = balloc(ba, 1);
    testCheck("balloc(1) should not return NULL", tiny != NULL);
    testCheck("balloc(1) should round up to the smallest block, 2^4 = 16", bsize(ba, tiny) == 16);
 
    void *big = balloc(ba, 1024);
    testCheck("balloc at exactly 2^u should succeed", big != NULL);
    testCheck("balloc(1024) should report size exactly 1024", bsize(ba, big) == 1024);
 
    void *tooBig = balloc(ba, 2000);
    testCheck("balloc larger than 2^u should fail", tooBig == NULL);
 
    bfree(ba, tiny);
    bfree(ba, big);
    bdelete(ba);
}
 
 
static void testBallocExhaustionAndRecovery(void) {
    size_t size = 1024;
    int l = 4;
    int u = 10;
 
    Balloc ba = bcreate(size, l, u);
    testCheck("bcreate should succeed", ba != NULL);
 
    if (ba == NULL) {
        return;
    }
 
    // Allocate every possible 16-byte block in the region.
    int n = size / 16;
    void *blocks[64];
    testCheck("region fits in test array", n <= 64);
 
    int allSucceeded = 1;
    for (int i = 0; i < n; i++) {
        blocks[i] = balloc(ba, 16);
        if (blocks[i] == NULL) {
            allSucceeded = 0;
        }
    }
    testCheck("every 16-byte block in the region should be allocatable", allSucceeded);
 
    void *overflow = balloc(ba, 16);
    testCheck("region should be exhausted after allocating everything", overflow == NULL);
 
    for (int i = 0; i < n; i++) {
        bfree(ba, blocks[i]);
    }
 
    void *whole = balloc(ba, size);
    testCheck("full region should be reclaimable as one block", whole != NULL);
    testCheck("recombined block should report the full region size", bsize(ba, whole) == size);
 
    bfree(ba, whole);
    bdelete(ba);
}
 
 
static void testBallocMixedSizes(void) {
    Balloc ba = bcreate(8192, 4, 13);
    testCheck("bcreate should succeed", ba != NULL);
 
    if (ba == NULL) {
        return;
    }
 
    void *a = balloc(ba, 10);   // -> 16
    void *b = balloc(ba, 100);  // -> 128
    void *c = balloc(ba, 500);  // -> 512
    void *d = balloc(ba, 10);   // -> 16
 
    testCheck("all four mixed allocations should succeed", a != NULL && b != NULL && c != NULL && d != NULL);
    testCheck("a and d should be distinct 16-byte blocks", a != d);
 
    memset(a, 0x11, bsize(ba, a));
    memset(b, 0x22, bsize(ba, b));
    memset(c, 0x33, bsize(ba, c));
    memset(d, 0x44, bsize(ba, d));
 
    // Free out of allocation order.
    bfree(ba, c);
    bfree(ba, a);
 
    testCheck("b's contents should survive unrelated frees",
              ((unsigned char *)b)[0] == 0x22 &&
              ((unsigned char *)b)[bsize(ba, b) - 1] == 0x22);
    testCheck("d's contents should survive unrelated frees",
              ((unsigned char *)d)[0] == 0x44 &&
              ((unsigned char *)d)[bsize(ba, d) - 1] == 0x44);
 
    bfree(ba, b);
    bfree(ba, d);
    bdelete(ba);
}
 
int main(void){
    testDivUp();
    testBits2Bytes();
    testE2Size();
    testSize2E();
    bitManipulationTests();
    testmmallocAndmmfree();
    testFreelistCreateDelete();
    testFreelistFreeAndAlloc();
    testFreelistMultipleBlocks();
    testFreelistReuse();
    testFreelistSize();
    testBallocCreateDelete();
    testBallocAllocFree();
    testBallocBounds();
    testBallocExhaustionAndRecovery();
    testBallocMixedSizes();
    testSummary();
    return testsFailed == 0 ? 0 : 1;
}