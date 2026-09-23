#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include "utils.h"
#include "freelist.h"


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
    void *base = mmap(NULL, size,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1, 0);

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

    // The next allocation of a 2^4 = 16 byte block
    // should return the block we just freed.
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

    void *base = mmap(NULL, size,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1, 0);

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

    // Create two separate 16-byte blocks.
    void *block1 = base;
    void *block2 = (char *)base + 16;

    freelistfree(f, base, block1, 4, l);
    freelistfree(f, base, block2, 4, l);

    // Since the free list behaves like a stack, block2
    // should be returned first.
    void *result1 = freelistalloc(f, base, 4, l);
    void *result2 = freelistalloc(f, base, 4, l);

    testCheck("first allocation should return most recently freed block",
              result1 == block2);

    testCheck("second allocation should return first freed block",
              result2 == block1);

    freelistdelete(f, l, u);
    munmap(base, size);
}


static void testFreelistReuse(void) {
    size_t size = 256;
    int l = 4;
    int u = 8;

    void *base = mmap(NULL, size,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1, 0);

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

    testCheck("allocated block should be the freed block",
              allocated == block);

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

    void *base = mmap(NULL, size,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1, 0);

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

    /*
     * Free a block at level 4 (2^4 = 16 bytes).
     */
    void *block = base;

    freelistfree(f, base, block, 4, l);

    int result = freelistsize(f, base, block, l, u);

    testCheck("freelistsize should identify 16-byte block",
              result == 4);

    freelistdelete(f, l, u);
    munmap(base, size);
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
    testSummary();
    return testsFailed == 0 ? 0 : 1;
}