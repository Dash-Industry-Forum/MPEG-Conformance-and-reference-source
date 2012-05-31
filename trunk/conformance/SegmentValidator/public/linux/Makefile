#
#	<Insert proper header here>
#

VPATH = . ../src

CFLAGS = -g -DLITTLEENDIAN -Wno-multichar

CC = gcc

HEADERS = \
ValidateMP4.h \
EndianMP4.h

SOURCES = \
ValidateAtomList.c \
ValidateAtoms.c \
ValidateBitStreams.c \
ValidateBits.c \
ValidateFileIO.c \
ValidateHints.c \
ValidateMP4.c

OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

ValidateMP4:	$(OBJECTS) $(HEADERS)
	$(CC) -g -o $@ $(CFLAGS) $(OBJECTS)
	
clean:
	-rm $(OBJECTS) $(SOURCES:.c=.d) ValidateMP4


%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s/$*.o/& $@/g'\'' > $@'

TAGS: $(SOURCES)
	etags $^

#
# Include all dependency files
#
-include $(SOURCES:.c=.d)
