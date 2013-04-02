VERSION = 0.2.0

INCLUDES = 
LIBS = 

CC = gcc
CFLAGS = -O0 -g -std=c99 -pedantic -Wall -Wextra $(INCLUDES)

LD = gcc
LDFLAGS += $(LIBS)

AR = ar
ARFLAGS = rsc

BINARIES = h264_analyze

all: libh264bitstream.a $(BINARIES)

h264_analyze: h264_analyze.o libh264bitstream.a
	$(LD) $(LDFLAGS) -o h264_analyze h264_analyze.o -L. -lh264bitstream -lm

libh264bitstream.a: h264_stream.c h264_nal.c h264_slice_data.c h264_sei.c  
	$(CC) $(CFLAGS) -c -o h264_nal.o h264_nal.c
	$(CC) $(CFLAGS) -c -o h264_stream.o h264_stream.c
	$(CC) $(CFLAGS) -c -o h264_slice_data.o h264_slice_data.c
	$(CC) $(CFLAGS) -c -o h264_sei.o h264_sei.c
	$(AR) $(ARFLAGS) libh264bitstream.a h264_stream.o h264_nal.o h264_slice_data.o h264_sei.o  

clean:
	rm -rf *.o libh264bitstream.a $(BINARIES) *.lo *.la .libs .deps autom4te.cache

dox: h264_stream.c h264_stream.h bs.h Doxyfile
	doxygen Doxyfile

dist: clean
	mkdir h264bitstream-$(VERSION)
	tar c --files-from=MANIFEST -f tmp.tar ; cd h264bitstream-$(VERSION) ; tar xf ../tmp.tar ; rm -f ../tmp.tar
	tar czf ../h264bitstream-$(VERSION).tar.gz h264bitstream-$(VERSION)
	rm -rf h264bitstream-$(VERSION)

