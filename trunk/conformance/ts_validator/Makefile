SHELL = /bin/sh

SUBDIRS = tslib libstructures h264bitstream

tslib: libstructures h264bitstream

$(SUBDIRS):
	$(MAKE) -C $@

subdirs: $(SUBDIRS)

subdirs-clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

all: subdirs
default: all

clean: subdirs-clean

.PHONY: subdirs $(SUBDIRS) subdirs-clean
