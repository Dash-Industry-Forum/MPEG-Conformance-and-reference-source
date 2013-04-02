SHELL = /bin/sh

CC = gcc
CFLAGS = -std=c99 -O0 -g -pedantic -pipe -Wall -Wextra
#CFLAGS = -std=c99 -O2 -ffast-math -g -pedantic -pipe -Wall -Wextra

LD = gcc
LDFLAGS += -g -static

AR = ar
ARFLAGS = rcls

RANLIB = ranlib
RM = rm -f

SRCS = $(wildcard *.c) $(wildcard apps/*.c)
OBJS = $(SRCS:%.c=%.o)

INCLUDES = -I . -I../common -I../libstructures/ -I../h264bitstream/ 
LIBS = -L .  -L../libstructures/ -L../h264bitstream -ldatastruct -ltslib -lh264bitstream

CFLAGS  += $(INCLUDES)
LDFLAGS += $(LIBS)

BINARIES = apps/dash_mpeg2ts_validate

all: libtslib.a $(BINARIES)

libtslib.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $^ 
	$(RANLIB) $@

apps/dash_mpeg2ts_validate: apps/dash_mpeg2ts_validate.c libtslib.a
	$(CC) $(CFLAGS) -o apps/dash_mpeg2ts_validate apps/dash_mpeg2ts_validate.c $(LIBS)
#	$(CC) $(CFLAGS) -o apps/dash_mpeg2ts_validate apps/dash_mpeg2ts_validate.c apps/avc_es_handler.o -L. -ltslib -L../libstructures/  -ldatastruct -lm -lh264bitstream

clean:
	rm -f $(OBJS) *.a $(BINARIES)
