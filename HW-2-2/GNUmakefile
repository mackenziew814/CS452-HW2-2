include ../GNUmakefile

.DEFAULT_GOAL := all

.PHONY: all
all: main wrappertest

main: main.o balloc.o freelist.o bm.o bbm.o utils.o
	$(ld) -o $@ $^ $(ldflags)

wrappertest: wrappertest.o wrapper.o deq.o balloc.o freelist.o bm.o bbm.o utils.o
	$(ld) -o $@ $^ $(ldflags)

clean:: ; rm -f main wrappertest
