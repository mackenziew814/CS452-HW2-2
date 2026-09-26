include ../GNUmakefile

.DEFAULT_GOAL := all

.PHONY: all
all: main wraptest

main: main.o balloc.o freelist.o bm.o bbm.o utils.o
	$(ld) -o $@ $^ $(ldflags)

wraptest: wraptest.o wrapper.o deq.o balloc.o freelist.o bm.o bbm.o utils.o
	$(ld) -o $@ $^ $(ldflags)

.PHONY: run-main run-wraptest
run-main:     main     ; ./main
run-wraptest: wraptest ; ./wraptest


clean:: ; rm -f main wraptest
