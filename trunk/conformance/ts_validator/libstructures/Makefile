VERSION = 1.0.0

SRCS = $(wildcard *.c)

OBJS = $(SRCS:%.c=%.o)

INCLUDES = -I.
LIBS =  -L. -lm

CC = gcc
CFLAGS += -std=c99 -g -O3 -fexpensive-optimizations -funroll-loops -fomit-frame-pointer -Wall -W -Wshadow -Wwrite-strings
LD = gcc
LDFLAGS += $(CFLAGS)

LDFLAGS += $(LIBS)
CFLAGS  += $(INCLUDES)

AR = ar
ARFLAGS = rc

RANLIB = ranlib

.PHONY: depend clean

all: depend libdatastruct.a

#fib_heap_test not checked in?
test: binheap_test hashtable_test varray_test vqarray_test hash_leak_test
	./binheap_test
	./hashtable_test
	./varray_test
	./vqarray_test

libdatastruct.a: varray.o vqarray.o binheap.o hashtable.o hashtable_itr.o hashtable_str.o
	$(AR) $(ARFLAGS) libdatastruct.a varray.o vqarray.o binheap.o hashtable.o hashtable_itr.o hashtable_str.o
	$(RANLIB) libdatastruct.a

binheap_test: binheap_test.o libdatastruct.a
	$(LD) -o binheap_test binheap_test.o libdatastruct.a $(LDFLAGS)

hashtable_test: hashtable_test.o libdatastruct.a
	$(LD) -o hashtable_test hashtable_test.o libdatastruct.a $(LDFLAGS)

hash_leak_test: hash_leak_test.o libdatastruct.a
	$(LD) -o hash_leak_test hash_leak_test.o libdatastruct.a $(LDFLAGS)

varray_test: varray_test.o libdatastruct.a
	$(LD) -o varray_test varray_test.o libdatastruct.a $(LDFLAGS)

vqarray_test: vqarray_test.o libdatastruct.a
	$(LD) -o vqarray_test vqarray_test.o libdatastruct.a $(LDFLAGS)

.depend: 
	rm -f .depend
	$(foreach SRC, $(SRCS), $(CC) $(CFLAGS) $(SRC) -MM 1>> .depend ;)

depend: .depend

ifneq ($(wildcard .depend),)
include .depend
endif

clean:
	rm -f $(OBJS)
	rm -f .depend
	rm -f libdatastruct.a

dist: clean
	mkdir libstructures-$(VERSION)
	tar c --files-from=MANIFEST -f tmp.tar ; cd libstructures-$(VERSION) ; tar xf ../tmp.tar ; rm -f ../tmp.tar
	tar czf ../libstructures-$(VERSION).tar.gz libstructures-$(VERSION)
	rm -rf libstructures-$(VERSION)
