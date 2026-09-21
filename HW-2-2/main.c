#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include "utils.h"


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

int main(void){
    testDivUp();
    testBits2Bytes();
    testE2Size();
    testSize2E();
    bitManipulationTests();
    testmmallocAndmmfree();
    testSummary();
    return testsFailed == 0 ? 0 : 1;
}