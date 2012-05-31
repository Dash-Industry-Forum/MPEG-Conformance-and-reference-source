#
#	<Insert proper header here>
#

#CFLAGS = -g -DLITTLEENDIAN -Wno-multichar
CFLAGS = -gdwarf-2 -Wno-multichar -DUSE_STRCASECMP -arch i386
PreprocessOptions = "-d forPublicRelease -d no3GP"

SRCDIR = ../src
PUBDIR = ../public/
#OBJDIR = ./obj
OBJDIR = .

VPATH = ../src:./obj

HEADERS = \
$(SRCDIR)/ValidateMP4.h \
$(SRCDIR)/EndianMP4.h



SOURCES = \
ValidateAtomList.c \
ValidateAtoms.c \
ValidateBitStreams.c \
ValidateBits.c \
ValidateFileIO.c \
ValidateHints.c \
ValidateMP4.c

OBJS := $(patsubst %.c,%.o,$(SOURCES))


ValidateMP4:	ValidateObjDir $(OBJS) $(HEADERS)
	$(CC) -o $@ $(CFLAGS) $(OBJS)
	
ValidateObjDir:
#	mkdir -p $(OBJDIR)
	
clean:
	-rm $(OBJS) $(SOURCES:.c=.d) ValidateMP4

	
%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s/$*.o/& $@/g'\'' > $@'

TAGS: $(SOURCES)
	etags $^


#
# Include all dependency files
#
-include $(SOURCES:.c=.d)





